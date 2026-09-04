#include <algorithm>
#include <atomic>
#include <cassert>
#include <charconv>
#include <future>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "openings.hpp"
#include "search.hpp"
#include "search_params.hpp"
#include "transposition_table.hpp"
#include "uci.hpp"
#include "utils.hpp"


//-##############################    GLOBALS    #############################-//
static game_t game = {};
static std::string initial_position = DEFAULT_POSITION;
static bool opening_book_loaded = false;
static bool opening_book_enabled = false;                   // `OwnBook`
static bool opening_book_best_move = false;                 // `Best Book Move`
static std::string opening_book_file = BOOK_FILE_EMBEDDED;  // `Book File`
static bool still_in_opening = true;  // Finish the opening line
static book_t opening_book;

static std::atomic_bool stop_search_signal = false;
static std::atomic_int session_id = 0;

// stop_search_signal and session_id are one piece of state and are read and
// written as one under this. Atomics on each separately are not enough: a
// hard-limit timer that loads session_id, is preempted, and stores the stop
// flag after the next [go] has bumped the session and cleared the flag kills a
// search it was never armed for, and that search dies at its first poll -
// depth 1, an instant reply for no reason. Stale timers are the normal case,
// not the exceptional one: a move ends at its soft limit, so its hard timer is
// still sleeping when the next [go] arrives, every move of every game. Held for
// a compare and a store, never across a join or a search. S163.
static std::mutex session_mutex;
static transposition_table_t tt = {};

// The last mate line the engine was shown to deliver, kept here because it has
// to outlive a `go` and search_state_t does not: that struct is built fresh in
// iterative_deepening_search() for every search, which is exactly why a mate
// score read back from the table arrived with no line behind it. Reporting
// state only -- nothing reads it to decide, order or prune a move. S170.
static proven_mate_line_t proven_mate_line = {};

static std::thread search_thread;
static std::mutex output_mutex;

static std::random_device rd;
static std::mt19937_64 gen(rd());

static bool is_debug = false;
static bool running = true;

// How many times the last iterative_deepening_search() had to widen an
// aspiration window and repeat an iteration. Nothing in the engine reads it:
// it exists so a test can assert that the window schedule was actually in
// force, which a mate case driven through this function otherwise cannot tell
// from a search that never narrowed anything. S021.
static int last_aspiration_failures = 0;

// The time manager's state after the last completed iteration of the last
// iterative_deepening_search(). Read by nothing in the engine, for the same
// reason as above: the scale is a pure function and a loop that computed it
// and ignored it would satisfy every test written against the function alone.
// S089.
static int last_best_move_stability = 0;
static int last_score_drop_cp = 0;
static int last_time_scale_percent = 100;


//-#############################   DECLARATIONS  ############################-//
typedef bool (*process_func)(std::queue<std::string>&);


const game_t* uci_game()
{ return &game; }


const transposition_table_t* uci_tt()
{ return &tt; }


int uci_last_aspiration_failures()
{ return last_aspiration_failures; }


int uci_last_best_move_stability()
{ return last_best_move_stability; }


int uci_last_score_drop_cp()
{ return last_score_drop_cp; }


int uci_last_time_scale_percent()
{ return last_time_scale_percent; }


void uci_reply(const std::string& response)
{
  const std::lock_guard<std::mutex> lock(output_mutex);
  std::cout << (response + "\n") << std::flush;
}


// Every path that mutates the board or the transposition table, or that ends
// the process, must call this first.
void stop_and_join_search()
{
  stop_search_signal = true;

  if (search_thread.joinable()) { search_thread.join(); }
}


// Opens a new search session: a fresh id and a cleared stop flag, published as
// one. Every timer armed for an earlier search is disarmed by the id, and the
// two stores are under session_mutex so a timer's check cannot straddle them.
// Call after stop_and_join_search(), before arming a timer of your own. S163.
void begin_search_session()
{
  const std::lock_guard<std::mutex> lock(session_mutex);

  session_id++;
  stop_search_signal = false;
}


// Waits for a running [go] to finish on its own, without cutting it short. A
// GUI never needs this - it just reads bestmove off stdout - but a test has to
// know when the reply has been written.
void uci_wait_for_search()
{
  if (search_thread.joinable()) { search_thread.join(); }
}


//-###########################  COMMAND DECLARATIONS  #######################-//
bool command_uci(std::queue<std::string>& args);
bool command_debug(std::queue<std::string>& args);
bool command_isready(std::queue<std::string>& args);
bool command_setoption(std::queue<std::string>& args);
bool command_register(std::queue<std::string>& args);
bool command_ucinewgame(std::queue<std::string>& args);
bool command_position(std::queue<std::string>& args);
bool command_go(std::queue<std::string>& args);
bool command_stop(std::queue<std::string>& args);
bool command_ponderhit(std::queue<std::string>& args);
bool command_quit(std::queue<std::string>& args);

bool command_print_board(std::queue<std::string>& args);
bool command_fen(std::queue<std::string>& args);
bool command_help(std::queue<std::string>& args);
bool command_test(std::queue<std::string>& args);
bool command_clean_TT(std::queue<std::string>& args);

// clang-format off
static const std::unordered_map<std::string, process_func> commands = {
  {"uci", command_uci},
  {"debug", command_debug},
  {"isready", command_isready},
  {"setoption", command_setoption},
  {"register", command_register},
  {"ucinewgame", command_ucinewgame},
  {"position", command_position},
  {"go", command_go},
  {"stop", command_stop},
  {"ponderhit", command_ponderhit},
  {"quit", command_quit},
  // custom commands
  {"pb", command_print_board},
  {"fen", command_fen},
  {"help", command_help},
  {"test", command_test},
  {"clean-tt", command_clean_TT},
};
// clang-format on


//-############################  UTILS FUNCTIONS  ###########################-//
std::ostream& operator<<(std::ostream& os, std::queue<std::string> q)
{
  os << "[";
  while (!q.empty()) {
    os << "\"" << q.front() << "\"";
    q.pop();
    if (!q.empty()) { os << ", "; }
  }
  os << "]";
  return os;
}


std::string trim_whitespace(const std::string& str)
{
  // Find the first non-whitespace character
  auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
  // Find the last non-whitespace character
  auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();

  // If the string is all whitespace, return an empty string
  return (start < end) ? std::string(start, end) : std::string();
}


std::optional<uci_move_t> algebraic_to_uci_move(const std::string& p)
{
  if (p.length() < 4 || p.length() > 5) { return {}; }

  uint8_t from_file = p[0];
  uint8_t from_rank = p[1];
  uint8_t to_file = p[2];
  uint8_t to_rank = p[3];

  if (from_file < 'a' || from_file > 'h' || from_rank < '1' ||
      from_rank > '8') {
    return {};
  }

  if (to_file < 'a' || to_file > 'h' || to_rank < '1' || to_rank > '8') {
    return {};
  }

  from_file = from_file - 'a';
  from_rank = from_rank - '1';

  to_file = to_file - 'a';
  to_rank = to_rank - '1';

  const index_t from = position_to_index(from_file, from_rank);
  const index_t to = position_to_index(to_file, to_rank);

  uci_move_t move;
  move.from = from;
  move.to = to;
  move.promotion = TO_NONE;

  if (p.length() == 5) {
    // Handle promotion
    switch (p[4]) {
      case 'Q':
      case 'q':
        move.promotion = TO_QUEEN;
        break;
      case 'N':
      case 'n':
        move.promotion = TO_KNIGHT;
        break;
      case 'R':
      case 'r':
        move.promotion = TO_ROOK;
        break;
      case 'B':
      case 'b':
        move.promotion = TO_BISHOP;
        break;

      default:
        return {};
        break;
    }
  }

  return move;
}


std::string promotion_to_string(const promotion_t promotion)
{
  switch (promotion) {
    case TO_QUEEN:
      return "q";
      break;
    case TO_KNIGHT:
      return "n";
      break;
    case TO_ROOK:
      return "r";
      break;
    case TO_BISHOP:
      return "b";
      break;

    default:
      assert(false);
      break;
  }

  assert(false);
  return "ERROR";
}


std::string uci_move_to_algebraic(const uci_move_t* move)
{
  assert(move != nullptr);
  std::string result;
  result += index_to_str(move->from);
  result += index_to_str(move->to);

  if (move->promotion != TO_NONE) {
    result += promotion_to_string(move->promotion);
  }

  return result;
}


std::string best_move_to_string(const uci_search_result_t& result)
{
  // UCI null move is 0000
  if (result.best_move == 0) { return "0000"; }
  return uci_move_to_algebraic(&result.uci_best_move);
}


std::string pv_to_string(const pv_t* pv)
{
  assert(pv != nullptr);

  std::stringstream ss;

  for (size_t i = 0; i < pv->length; ++i) {
    const uci_move_t move = {MOVE_FROM(pv->table[i]), MOVE_TO(pv->table[i]),
                             MOVE_PROMOTED(pv->table[i])};

    ss << uci_move_to_algebraic(&move) << " ";
  }

  return ss.str();
}


bool check_move_legality(move_t move)
{
  move_t moves[MAX_MOVES];
  const size_t moves_size = generate_moves(game_tables(), &game.board, moves);

  if (moves_size < 1) {
    LOG_E << print_move(move) << " ILLEGAL. No move available in this position"
          << END_E;
    return false;
  }

  bool found = false;
  for (size_t i = 0; i < moves_size; ++i) {
    // NOTE: Here the check must be wick. Only from, to and promotion
    if (unpacked_move_t(move) == unpacked_move_t(moves[i])) {
      found = true;
      break;
    }
  }

  if (!found) {
    LOG_E << print_move(move) << " ILLEGAL. Is not in the legal move list"
          << END_E;
    return false;
  }

  // Attempt to make the move
  const bool res = make_move(&game, move);

  if (!res) {
    LOG_E << print_move(move) << " ILLEGAL. Failed to make the move" << END_E;
    return false;
  }

  unmake_move(&game);

  return true;
}


//-#############################    FUNCTIONS    ############################-//

bool set_position(const std::string& fen)
{
  // S176 (2026-09-03_adversarial-F04). A FEN that does not load leaves the
  // engine exactly where it was, moves included: the whole game -- board,
  // history, hash randoms -- is saved before the load and put back after a
  // failure, since load_FEN() may have written part of the board before it
  // rejected the rest. Reloading `initial_position` here was reloading the
  // last *FEN*, not the last *position*, so every move applied since was lost
  // and one malformed FEN after `startpos moves e2e4` put the engine on the
  // start position. The copy is about 100 KB and `position` is not on any
  // search path. The refusal goes on the UCI channel in every build: LOG_E is
  // silent in the binary that ships (the S137 pattern).
  const game_t previous = game;

  if (!load_FEN(fen, &game)) {
    game = previous;
    uci_reply("info string refused [position fen] " + fen + ", does not load");
    return false;
  }

  if (initial_position != fen) {
    initial_position = fen;

    tt_reset(&tt);
    still_in_opening = true;
  }

  return true;
}


// Loads whatever `Book File` currently names. A failure leaves the engine with
// no book rather than falling back to the built-in one: a harness that asked
// for a particular book and silently got a different one is measuring something
// nobody configured. S172.
void try_load_opening_book()
{
  opening_book = book_t{};

  const bool embedded =
      opening_book_file.empty() || opening_book_file == BOOK_FILE_EMBEDDED;

  std::string reason;

  opening_book_loaded =
      embedded ? load_book_embedded(&opening_book)
               : load_book_from_file(opening_book_file, &opening_book, &reason);

  if (opening_book_loaded) {
    LOG_I << "Opening book loaded correctly! " << opening_book.num_of_positions
          << " entries." << END_I;
    return;
  }

  if (embedded) {
    LOG_E << "Failed to load the built-in opening book." << END_E;
    return;
  }

  // On the UCI channel and not only in the log. LOG_E compiles to
  // `if (false) std::clog` under NDEBUG and the shipping binary is a Release
  // build, so a mistyped path would otherwise be answered by silence -- the
  // failure S137 removed for a refused search parameter, one option along.
  uci_reply("info string book [" + opening_book_file +
            "] not loaded: " + reason + ". Playing without a book");
}


std::queue<std::string> tokenize_input(const std::string string,
                                       const std::string delimiter)
{
  const std::string s = trim_whitespace(string);
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::queue<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;

    if (token.size() > 0) { res.push((token)); }
  }

  const std::string remaining = s.substr(pos_start);
  if (remaining.length() > 0) { res.push(remaining); }

  return res;
}


void stop_search_after_ms(uint64_t ms)
{
  std::thread job([ms, session = session_id.load()]() {
    std::chrono::milliseconds time_to_sleep(ms);
    std::this_thread::sleep_for(time_to_sleep);

    // Check and store as one decision, so no [go] can slip between them.
    const std::lock_guard<std::mutex> lock(session_mutex);

    if (session_id == session) { stop_search_signal = true; }
  });

  // Left the timer be, we return! Adios
  job.detach();
}


search_time_budget_t compute_search_time_budget(int remaining_ms,
                                                int increment_ms,
                                                int movestogo)
{
  assert(movestogo >= 0);
  assert(remaining_ms > 0);
  assert(increment_ms >= 0);

  // The allocation this move is measured against, before either limit is taken
  // from it. int64 throughout: remaining_ms is whatever the GUI sent, up to
  // INT_MAX, and TM_HARD_PERCENT multiplies it.
  //
  // With a movestogo the clock has a boundary and the share is the obvious
  // one. Without, there is no boundary, and the number that used to be divided
  // by was a fabricated 20 - so a sudden-death game was played as though a
  // control sat twenty moves out, at move 3 and at move 90 alike. A percentage
  // of what is actually left claims nothing about the move count. S089.
  const int64_t clock_part =
      (movestogo > 0)
          ? (static_cast<int64_t>(remaining_ms) / movestogo)
          : ((static_cast<int64_t>(remaining_ms) * TM_SUDDEN_DEATH_PERCENT) /
             100);

  const int64_t base_ms =
      clock_part +
      ((static_cast<int64_t>(increment_ms) * TM_INCREMENT_PERCENT) / 100);

  int64_t hard_ms = (base_ms * TM_HARD_PERCENT) / 100;
  int64_t soft_ms = (base_ms * TM_SOFT_PERCENT) / 100;

  // Never budget more than is actually left. This is the line between a late
  // move and a lost game, so it is applied to the hard limit - the one a timer
  // is armed at - and nothing downstream is allowed to raise it again: the
  // scaling in iterative_deepening_search() moves the soft limit only.
  hard_ms = std::min(hard_ms, static_cast<int64_t>(remaining_ms) -
                                  static_cast<int64_t>(MOVE_OVERHEAD_MS));

  // The floor is half the clock, capped at 50ms, and below 2 * MOVE_OVERHEAD_MS
  // it is the only thing left holding the budget up - the cap above has already
  // gone negative. At remaining_ms == 1 that halving lands on zero as well, and
  // a zero budget arms no timer, so with no depth and no node limit nothing at
  // all bounds the search and the engine never answers. One millisecond loses
  // on time; silence hangs the match. S036.
  const int64_t floor_ms = std::max(1, std::min(50, remaining_ms / 2));

  hard_ms = std::max(hard_ms, floor_ms);

  // The soft limit is the smaller of the two by construction. On a clock short
  // enough for the floor to be the whole budget the two meet, which is right:
  // there is no second iteration to decide about.
  soft_ms = std::max<int64_t>(std::min(soft_ms, hard_ms), 1);

  return {static_cast<int>(soft_ms), static_cast<int>(hard_ms)};
}


int search_time_scale_percent(int best_move_stability, int score_drop_cp)
{
  assert(best_move_stability >= 0);
  assert(score_drop_cp >= 0);

  int scale = 100;

  // A best move that has survived several iterations is unlikely to fall in
  // the next one, so each unchanged iteration takes a slice off. Capped,
  // because the confidence stops growing long before the iterations do.
  const int stability = std::min(best_move_stability, TM_STABILITY_MAX);
  scale -= stability * TM_STABILITY_PERCENT;

  // A score that fell means the position is turning out worse than the
  // previous iteration thought, which is exactly when the next iteration is
  // worth beginning. Linear in the fall up to TM_FALLING_MAX_CP, flat above.
  const int drop = std::min(score_drop_cp, TM_FALLING_MAX_CP);
  scale += (drop * TM_FALLING_PERCENT) / TM_FALLING_MAX_CP;

  // The two are independent settings and their product is not bounded by
  // either range, so the floor is what keeps the soft limit off zero. Without
  // it a stability discount large enough to go negative would end every search
  // at depth one.
  return std::max(scale, TM_SCALE_MIN_PERCENT);
}


bool is_command(const std::string& command)
{
  auto it = commands.find(command);
  if (it != commands.end()) { return true; }
  return false;
}


bool try_move(unpacked_move_t* move_candidate)
{
  assert(move_candidate != nullptr);

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_moves(game_tables(), &game.board, moves);

  // Fix the possible weirdo move notation for castling
  fix_weirdo_castling(&game.board, move_candidate);

  // Search the move in the list of legal moves
  for (size_t i = 0; i < moves_count; ++i) {
    const unpacked_move_t move(moves[i]);

    // Set this so we can perform the comparison
    move_candidate->piece = move.piece;

    if (move == *move_candidate) {
      // Apply the found move
      bool move_result = make_move(&game, moves[i]);

      if (move_result) { return true; }

      break;
    }
  }

  return false;
}


// Matches a book entry against the real move list and returns the generated
// encoding of that move, or 0 when it is not legal here. Only from, to and the
// promotion piece are compared: the book carries its own flags and they are
// not necessarily the ones generate_moves() produces.
move_t validate_book_move(move_t book_move)
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game.board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (MOVE_FROM(moves[i]) != MOVE_FROM(book_move) ||
        MOVE_TO(moves[i]) != MOVE_TO(book_move) ||
        MOVE_PROMOTED(moves[i]) != MOVE_PROMOTED(book_move)) {
      continue;
    }

    if (make_move(&game, moves[i])) {
      unmake_move(&game);
      return moves[i];
    }
  }

  return 0;
}


// Picks a book move for the current position, or 0 when the book has nothing
// playable here.
//
// Selection follows the Polyglot format's own semantics, which is what
// Stockfish did while it had a book: an entry's `weight` is how often the book
// wants that move played, so the default draws in proportion to weight and
// `Best Book Move` takes the heaviest entry instead. Until S172 this drew
// uniformly and never read the weight field at all, so a line the book gave one
// game of weight was played as often as one it gave two hundred.
move_t search_book_move()
{
  move_t result = {};

  if (!opening_book_loaded || !opening_book_enabled || !still_in_opening) {
    return result;
  }

  move_t moves[MAX_MOVES];
  uint16_t weights[MAX_MOVES];
  const size_t moves_cout =
      get_book_moves_for_key(&opening_book, &game.board, moves, weights);

  // A book move is reported to the GUI as the best move without ever being
  // searched or played, so nothing else would catch a bad one. A polyglot
  // key collision or a malformed book used to forfeit the game outright.
  move_t legal_moves[MAX_MOVES];
  uint16_t legal_weights[MAX_MOVES];
  size_t legal_count = 0;

  for (size_t i = 0; i < moves_cout; ++i) {
    const move_t validated = validate_book_move(moves[i]);

    if (validated != 0) {
      legal_weights[legal_count] = weights[i];
      legal_moves[legal_count++] = validated;
    }
  }

  if (legal_count != moves_cout) {
    LOG_W << "Opening book returned " << (moves_cout - legal_count)
          << " illegal move(s) for this position, discarded" << END_W;
  }

  if (legal_count == 0) {
    // We finish the move lines or move not found, disabling it
    still_in_opening = false;
    return result;
  }

  size_t index = 0;

  if (opening_book_best_move) {
    for (size_t i = 1; i < legal_count; ++i) {
      if (legal_weights[i] > legal_weights[index]) { index = i; }
    }
  } else {
    uint64_t total = 0;

    for (size_t i = 0; i < legal_count; ++i) {
      total += legal_weights[i];
    }

    if (total == 0) {
      // Every entry here is weightless, which a hand-made book can be. There is
      // no preference to honour, so the uniform draw is the whole distribution
      // rather than a fallback that loses information.
      std::uniform_int_distribution<size_t> dist(0, legal_count - 1);
      index = dist(gen);
    } else {
      // Extreme excluded: `total` itself must land on no entry.
      std::uniform_int_distribution<uint64_t> dist(0, total - 1);

      uint64_t ticket = dist(gen);

      for (size_t i = 0; i < legal_count; ++i) {
        if (ticket < legal_weights[i]) {
          index = i;
          break;
        }

        ticket -= legal_weights[i];
      }
    }
  }

  assert(index < legal_count);
  result = legal_moves[index];

  LOG_I << "Found position in the opening book." << END_I;

  return result;
}


// A search cut off before it finishes even one root move still owes the GUI a
// move. Only a position with no legal move at all may answer with the UCI null
// move.
move_t first_legal_move()
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game.board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (make_move(&game, moves[i])) {
      unmake_move(&game);
      return moves[i];
    }
  }

  return 0;
}


uci_search_result_t iterative_deepening_search(const uci_search_options_t& conf)
{
  uci_search_result_t result = {};
  result.total_node_explored = 0;

  // If no move found in the book search by engine
  // NOTE: stop_search_signal is cleared by the caller before the timer is
  // armed. Clearing it here would race with the timer and with "stop".
  search_state_t state = {};
  state.tt = &tt;
  state.proven_mate = &proven_mate_line;
  assert(state.tt != nullptr);

  tt_new_search(state.tt);

  std::atomic_bool never_stop = false;
  state.stop = &never_stop;

  const auto beguine_of_the_search = std::chrono::steady_clock::now();

  // Carried across iterations so an aborted one can report the last score and
  // depth that actually mean something alongside the line it will play.
  std::string last_score = "cp 0";
  int last_complete_depth = 0;

  // The mate distance behind last_score, when it is a mate. Carried because the
  // score printed and the line printed can come from different iterations, and
  // the line then has to be completed against the score it is printed beside.
  // S170.
  bool last_score_is_mate = false;
  int last_mate_in = 0;

  // The centre of the next iteration's aspiration window, and whether there is
  // one to use. Set only from an iteration that finished inside its window: an
  // aborted iteration's score is meaningless and a fail-high or fail-low one is
  // a bound, so neither is a position estimate to build a band around. S021.
  int aspiration_score = 0;
  bool aspiration_ready = false;

  last_aspiration_failures = 0;
  last_best_move_stability = 0;
  last_score_drop_cp = 0;
  last_time_scale_percent = 100;

  // The hard limit is armed as a timer by the caller, and it is what stops the
  // search inside an iteration. What is decided down here is the other half:
  // whether to begin another iteration at all. S089.
  //
  // A caller that fills in only the hard limit gets soft == hard, which is
  // what the fixed-limit paths ([go movetime], the no-limit fallback) want.
  const int64_t hard_limit_ms = conf.search_time_ms;
  const int64_t soft_base_ms = (conf.search_soft_time_ms > 0)
                                   ? conf.search_soft_time_ms
                                   : conf.search_time_ms;

  int64_t soft_limit_ms = soft_base_ms;

  // The scale's two inputs: how many completed iterations in a row returned
  // the same best move, and how far the score fell in the last one.
  move_t previous_best_move = 0;
  int best_move_stability = 0;
  int previous_score = 0;
  bool previous_score_ready = false;

  for (int current_depth = 1; current_depth <= conf.depth; ++current_depth) {
    // Iterative deepening

    // Reset the explored nodes in the previous iteration
    state.explored_nodes = 0;

    // Cap this iteration at whatever is left of the overall budget, so the
    // limit is honoured inside the search instead of only being noticed after
    // an iteration has already blown past it.
    if (conf.nodes != 0) {
      if (result.total_node_explored >= conf.nodes) { break; }
      state.node_limit = conf.nodes - result.total_node_explored;
    }

    // Aspiration windows. The previous iteration already answered "what is
    // this position worth", and the answer rarely moves much in one ply, so
    // the root is searched in a band around it instead of from -inf to +inf. A
    // narrower window at the root is a narrower window at every node below it,
    // and alpha-beta cuts off sooner the narrower the window is.
    //
    // The bet loses when the score does move. A score outside the band is only
    // a bound - the search proved "at most alpha" or "at least beta" and
    // nothing more - so the iteration is repeated with the failing side pushed
    // out. Only the failing side: a fail-low says nothing about beta, and
    // widening both would give back the cut-offs the other half is still
    // earning.
    //
    // Widening doubles, and past ASPIRATION_MAX_DELTA it stops doubling and
    // goes to the full window in one move. The schedule is 25, 50, 100, 200,
    // 400, full at the shipping defaults, so the worst case is six searches at
    // one depth and each one of them is rarer than the last.
    //
    // A mate score ends the schedule immediately. The band is centipawns wide
    // and a mate score is tens of thousands away, so doubling towards it would
    // pay several full searches to arrive where one gets to now.
    //
    // What this costs on the mate cases, which is the reason S074 put them in
    // this step's gate: every node below the root inherits these bounds, and
    // DEC-060 measured that a pruning rule's mate exposure is a property of
    // the bound the parent passes down rather than of the static score. A mate
    // appearing mid-iteration is a guaranteed fail-high here - that is what
    // the re-search is for - and the fail-high path is what
    // tests/test_engine.cpp holds the three fast-suite mate positions against.
    int delta = ASPIRATION_DELTA;
    int alpha = -SEARCH_SCORE_INF;
    int beta = SEARCH_SCORE_INF;

    if (aspiration_ready && current_depth >= ASPIRATION_MIN_DEPTH) {
      alpha = aspiration_score - delta;
      beta = aspiration_score + delta;
    }

    search_t search_result = search(current_depth, &game, &state, alpha, beta);

    while (!state.aborted &&
           (alpha > -SEARCH_SCORE_INF || beta < SEARCH_SCORE_INF) &&
           (search_result.score <= alpha || search_result.score >= beta)) {
      ++last_aspiration_failures;

      const bool fail_low = (search_result.score <= alpha);

      if (search_result.mate_found || delta > ASPIRATION_MAX_DELTA) {
        alpha = -SEARCH_SCORE_INF;
        beta = SEARCH_SCORE_INF;
      } else if (fail_low) {
        alpha = std::max(search_result.score - delta, -SEARCH_SCORE_INF);
      } else {
        beta = std::min(search_result.score + delta, SEARCH_SCORE_INF);
      }

      delta += delta;

      search_result = search(current_depth, &game, &state, alpha, beta);
    }

    // Timed from the start of the whole search, not of this iteration: it is
    // the number a GUI divides the node count below by. S037.
    const auto elapsed =
        std::chrono::steady_clock::now() - beguine_of_the_search;

    state.stop = &stop_search_signal;

    result.total_node_explored += search_result.explored_nodes;

    // An aborted iteration is still worth to evaluate. The root replaces its
    // move only when that move beats every move searched before it at this
    // depth, and the first move it searches is the previous iteration's best
    // so a non-empty PV means the partial iteration knows something the
    // completed one did not. Its *score* is meaningless, which is why nothing
    // is reported for it.
    const bool has_result = search_result.pv.length > 0;

    if (has_result) {
      result.best_move = search_result.best_move;
      result.uci_best_move = {MOVE_FROM(search_result.best_move),
                              MOVE_TO(search_result.best_move),
                              MOVE_PROMOTED(search_result.best_move)};

      result.is_ponder_move = false;
      if (search_result.pv.length > 1) {
        result.is_ponder_move = true;
        result.ponder_move = {MOVE_FROM(search_result.pv.table[1]),
                              MOVE_TO(search_result.pv.table[1]),
                              MOVE_PROMOTED(search_result.pv.table[1])};
      }

      result.pv = search_result.pv;
    }

    if (!state.aborted) {
      last_score = search_result.mate_found
                       ? ("mate " + STR(search_result.mate_in))
                       : ("cp " + STR(search_result.score));
      last_score_is_mate = search_result.mate_found;
      last_mate_in = search_result.mate_in;
      last_complete_depth = current_depth;

      // The loop above only exits without an abort once the score is inside
      // the window, so this is a score and not a bound. A mate score is not a
      // centre to build a centipawn-wide band around, so it disarms the next
      // iteration's window rather than being used as one.
      aspiration_score = search_result.score;
      aspiration_ready = !search_result.mate_found;

      // The time manager reads the iteration that just finished. An aborted
      // one supplies nothing: its score is a bound at best and the loop is
      // about to break on it anyway.
      if (search_result.best_move != 0 &&
          search_result.best_move == previous_best_move) {
        ++best_move_stability;
      } else {
        best_move_stability = 0;
      }

      previous_best_move = search_result.best_move;

      const int score_drop =
          previous_score_ready
              ? std::max(0, previous_score - search_result.score)
              : 0;

      previous_score = search_result.score;
      previous_score_ready = true;

      const int scale = conf.scale_time ? search_time_scale_percent(
                                              best_move_stability, score_drop)
                                        : 100;

      soft_limit_ms = (soft_base_ms * scale) / 100;

      // The hard limit is the ceiling on both. Scaling may bring the soft
      // limit up to it and never past it, which is what keeps every promise
      // compute_search_time_budget() made about the clock.
      if (hard_limit_ms > 0) {
        soft_limit_ms = std::min(soft_limit_ms, hard_limit_ms);
      }

      last_best_move_stability = best_move_stability;
      last_score_drop_cp = score_drop;
      last_time_scale_percent = scale;
    }

    // Reported whenever there is a line to report, aborted iteration included.
    // An aborted iteration that produced a PV supplies the move that will be
    // played, so staying silent about it leaves the GUI holding a principal
    // variation from the previous depth and a bestmove that does not start it.
    // The score and depth of an unfinished iteration mean nothing - its window
    // never closed - so the last completed ones are repeated instead, and the
    // line printed is the one that will actually be played.
    // [nodes] is the whole search's count, which is how the protocol is read
    // everywhere and what makes it non-decreasing between depths. It used to be
    // this iteration's, so a GUI watched the count fall and
    // tools/search_bench.py - which keeps the last info line and is how INV-6
    // is discharged - compared only the final iteration. The per-iteration
    // figure is the difference between two successive lines, so printing it as
    // well would add nothing. S037.
    // An aborted iteration supplies the line that will be played while the
    // score stays the last completed one's, so the two can describe different
    // trees. That costs nothing while the score is centipawns and everything
    // while it is a mate: the pair then claims a distance the line does not
    // reach, which is the third of the three ways S170 measured a mate line
    // going short. The line is completed against the score it is about to be
    // printed beside, by the same all-or-nothing rule the search itself used
    // (DEC-122), so a pair that cannot be made consistent stays visibly short
    // rather than being papered over. S170.
    if (state.aborted && has_result && last_score_is_mate) {
      complete_mate_pv(&game, &state, &result.pv, last_mate_in);
    }

    if (has_result || !state.aborted) {
      const auto elapsed_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
      const int64_t elapsed_us =
          std::chrono::duration_cast<std::chrono::microseconds>(elapsed)
              .count();

      // Floored at a microsecond so the first iteration of a trivial position
      // cannot divide by zero. A real iteration never lands under it.
      const uint64_t nps =
          (result.total_node_explored * 1000000ULL) /
          ((elapsed_us > 0) ? static_cast<uint64_t>(elapsed_us) : 1ULL);

      uci_reply("info score " + last_score + " time " +
                STR(elapsed_ms.count()) + " depth " + STR(last_complete_depth) +
                " nodes " + STR(result.total_node_explored) + " nps " +
                STR(nps) + " pv " + pv_to_string(&result.pv));
    }

    if (state.aborted) { break; }

    if (stop_search_signal) { break; }
    if (conf.nodes != 0 && result.total_node_explored >= conf.nodes) { break; }

    if (conf.search_time_ms > 0) {
      const double elapsed_ms =
          std::chrono::duration<double, std::milli>(
              std::chrono::steady_clock::now() - beguine_of_the_search)
              .count();

      if (elapsed_ms >= static_cast<double>(soft_limit_ms)) { break; }
    }
  }

  // A node or time budget small enough to abort before the first root move
  // completes used to leave this at zero and report "bestmove 0000".
  if (result.best_move == 0) {
    const move_t fallback = first_legal_move();

    if (fallback != 0) {
      result.best_move = fallback;
      result.uci_best_move = {MOVE_FROM(fallback), MOVE_TO(fallback),
                              MOVE_PROMOTED(fallback)};
      result.is_ponder_move = false;

      LOG_W << "Search returned no move, answering with "
            << print_move(fallback) << END_W;
    }
  }

  return result;
}


//-################################  COMMANDS  ##############################-//
bool command_uci(std::queue<std::string>& args)
{
  LOG_I << "Command [uci]. Args: " << args << END_I;

  uci_reply("id name Chesso");
  uci_reply("id author MazerFaker");
  uci_reply("option name OwnBook type check default false");
  uci_reply("option name Book File type string default " BOOK_FILE_EMBEDDED);
  uci_reply("option name Best Book Move type check default false");
  uci_reply("option name Hash type spin default " + STR(TT_DEFAULT_MB) +
            " min " + STR(TT_MIN_MB) + " max " + STR(TT_MAX_MB));
  uci_reply("option name Threads type spin default 1 min 1 max 1");

#ifdef CHESSO_TUNE
  // Tune build only. The release binary's surface is the three lines above and
  // S073 does not move it. tests/test_uci_surface.cpp holds both shapes.
  for (size_t i = 0; i < search_param_count(); ++i) {
    const search_param_t& param = search_param_info(i);

    uci_reply("option name " + std::string(param.name) + " type spin default " +
              STR(param.default_value) + " min " + STR(param.min_value) +
              " max " + STR(param.max_value));
  }
#endif

  uci_reply("uciok");

  return true;
}


bool command_debug(std::queue<std::string>& args)
{
  LOG_I << "Command [debug]. Args: " << args << END_I;

  if (args.size() == 0) { return false; }

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "on") {
      LOG_I << "Debug mode ON" << END_I;
      is_debug = true;
    } else if (token == "off") {
      LOG_I << "Debug mode OFF" << END_I;
      is_debug = false;
    }
  }

  return true;
}


bool command_isready(std::queue<std::string>& args)
{
  LOG_I << "Command [is_ready]. Args: " << args << END_I;

  uci_reply("readyok");

  return true;
}


bool command_setoption(std::queue<std::string>& args)
{
  LOG_I << "Command [setoption]. Args: " << args << END_I;
  if (args.size() == 0) { return false; }

  std::string option_name;
  std::string option_value;

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "name") {
      // Read the full option name (can contain spaces)
      while (!args.empty() && args.front() != "value") {
        option_name += args.front() + " ";
        args.pop();
      }
      option_name = trim_whitespace(option_name);
    }

    if (!args.empty() && args.front() == "value") {
      args.pop();  // remove "value"

      // Everything left, not the first token. UCI says a value runs to the end
      // of the line, and `Book File` is the first option here whose value can
      // contain a space: reading one token turned
      // `/Users/max/My Books/x.bin` into `/Users/max/My` and reported nothing.
      // The single space is what tokenize_input() split on, so this rebuilds
      // the line as sent for every value that does not have repeated spaces in
      // it. S172.
      while (!args.empty()) {
        if (!option_value.empty()) { option_value += " "; }

        option_value += args.front();
        args.pop();
      }
    }
  }

  // Handle specific options
  if (option_name == "OwnBook" && option_value == "true") {
    opening_book_enabled = true;
    LOG_I << "OwnBook ON" << END_I;
  }

  if (option_name == "OwnBook" && option_value == "false") {
    opening_book_enabled = false;
    LOG_I << "OwnBook OFF" << END_I;
  }

  if (option_name == "Best Book Move" && option_value == "true") {
    opening_book_best_move = true;
    LOG_I << "Best Book Move ON" << END_I;
  }

  if (option_name == "Best Book Move" && option_value == "false") {
    opening_book_best_move = false;
    LOG_I << "Best Book Move OFF" << END_I;
  }

  if (option_name == "Book File") {
    // Reloading under a live search would free the bytes that search is
    // probing, for the same reason resizing the hash would.
    stop_and_join_search();

    opening_book_file = option_value;
    try_load_opening_book();

    // A new book is a new set of lines, so a game that walked out of the old
    // one is not out of this one.
    still_in_opening = true;
  }

  if (option_name == "Hash") {
    try {
      const long long megabytes = std::stoll(option_value);

      // Reallocating under a live search would free the table it is probing.
      stop_and_join_search();
      tt_resize(&tt, static_cast<size_t>(std::max<long long>(megabytes, 0)));
    } catch (...) {
      LOG_W << "Hash value is not a number: " << option_value << END_W;
    }
  }

  if (option_name == "Threads" && option_value != "1") {
    LOG_W << "Only one search thread is supported, ignoring Threads="
          << option_value << END_W;
  }

#ifdef CHESSO_TUNE
  // Tune build only, S073. A value outside a parameter's declared range is
  // refused rather than clamped, because a tuner that asked for something
  // impossible should hear about it.
  //
  // S137 is what makes it hear. The refusal used to go to LOG_W, which is
  // `if (false) std::clog` under NDEBUG, and `build-tune` is a Release build --
  // so both ways a tuner can be wrong, an impossible value and a misspelled
  // name, were indistinguishable from success and a run could spend a night
  // against a compiled default. One `info string` per refusal, on the channel
  // that is legal UCI in every build state. DEC-093.
  bool is_search_param = false;

  for (size_t i = 0; i < search_param_count(); ++i) {
    if (option_name != search_param_info(i).name) { continue; }

    is_search_param = true;

    const search_param_t& param = search_param_info(i);
    const std::string range =
        "[" + STR(param.min_value) + ", " + STR(param.max_value) + "]";

    // Changing a parameter under a live search would move the ground that
    // search is standing on, for the same reason resizing the hash would.
    stop_and_join_search();

    // The whole token has to be consumed. std::stoi() was here and it stops at
    // the first character it cannot use without complaining, so `0x50` set 0,
    // `120.9` set 120 and `12x` set 12, each of them silently -- the same
    // failure this step exists to remove, one layer down. Found reviewing
    // S137's own diff.
    int value = 0;
    const char* const last = option_value.data() + option_value.size();
    const std::from_chars_result parsed =
        std::from_chars(option_value.data(), last, value);

    if (parsed.ec == std::errc::result_out_of_range) {
      // A well-formed integer that no int can hold is out of range, not
      // malformed, whichever end it ran off.
      uci_reply("info string refused [" + option_name + "] value " +
                option_value + ", outside " + range);
    } else if (parsed.ec != std::errc() || parsed.ptr != last) {
      uci_reply("info string refused [" + option_name + "] value " +
                option_value + ", not an integer, range " + range);
    } else if (!search_param_set(option_name.c_str(), value)) {
      // The raw token rather than the parsed value, so the line quotes back
      // exactly what was sent.
      uci_reply("info string refused [" + option_name + "] value " +
                option_value + ", outside " + range);
    }

    break;
  }

  // The other half: a name nothing above recognised. Kept as its own list
  // rather than derived, because the five options that are not parameters are
  // an if-chain with nothing to enumerate -- tests/test_uci_surface.cpp drives
  // every name the `uci` reply advertises through here and fails if one of them
  // comes back unknown.
  if (!is_search_param && option_name != "OwnBook" &&
      option_name != "Book File" && option_name != "Best Book Move" &&
      option_name != "Hash" && option_name != "Threads") {
    uci_reply("info string refused [" + option_name + "], unknown option");
  }
#endif

  return true;
}


bool command_register(std::queue<std::string>& args)
{
  LOG_I << "Command [register]. Args: " << args << END_I;
  if (args.size() == 0) { return false; }
  return true;
}


bool command_ucinewgame(std::queue<std::string>& args)
{
  LOG_I << "Command [ucinewgame]. Args: " << args << END_I;

  stop_and_join_search();

  set_position(DEFAULT_POSITION);
  tt_reset(&tt);
  proven_mate_line.length = 0;
  still_in_opening = true;

  LOG_I << print_nice_board(&game.board) << END_I;

  return true;
}


bool command_position(std::queue<std::string>& args)
{
  LOG_I << "Command [position]. Args: " << args << END_I;

  if (args.size() == 0) { return false; }

  stop_and_join_search();

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "startpos") { set_position(DEFAULT_POSITION); }
    if (token == "empty") { set_position(EMPTY_POS); }
    if (token == "mate2w") { set_position(MATE_IN_2_W_POS); }
    if (token == "mate2b") { set_position(MATE_IN_2_B_POS); }
    if (token == "3frep") { set_position(THREE_FOLD_REP_POS); }
    if (token == "tricky") { set_position(TRICKY_POS); }
    if (token == "killer") { set_position(KILLER_POS); }
    if (token == "cmk") { set_position(CMK_POS); }
    if (token == "fine70") { set_position(FINE_70_POS); }

    if (token == "fen") {
      // S176. Four to six fields. The clocks are optional in practice --
      // Stockfish, cutechess and python-chess all accept the short form -- and
      // default to `0 1`. Reading stops at `moves`, which used to be swallowed
      // as the fifth field of a four-field FEN so that the load failed and the
      // moves were never applied. Fewer than four fields is refused and, like a
      // FEN that does not load, ends the whole command with the position
      // unchanged: applying the moves that follow to the old board would put
      // the engine somewhere the GUI did not send it.
      std::vector<std::string> fields;

      while (!args.empty() && fields.size() < 6 && args.front() != "moves") {
        fields.push_back(args.front());
        args.pop();
      }

      std::string fen;

      for (const std::string& field : fields) {
        fen += field + " ";
      }

      fen = trim_whitespace(fen);

      if (fields.size() < 4) {
        uci_reply("info string refused [position fen] " +
                  (fen.empty() ? std::string("(none)") : fen) +
                  ", fewer than four fields");
        return false;
      }

      if (fields.size() == 4) { fen += " 0"; }
      if (fields.size() <= 5) { fen += " 1"; }

      // The refusal, if any, has already been reported by set_position().
      if (!set_position(fen)) { return false; }

      LOG_I << "Set fen " << fen << END_I;
    }

    if (token == "moves") {
      // Assuming all the next tokens are moves to execute. Skip the one that
      // are not valid. Doing best effort

      while (!args.empty()) {
        const std::string move_str = args.front();
        args.pop();

        const auto parsing_result = algebraic_to_uci_move(move_str);

        if (parsing_result.has_value()) {
          // Apply the move
          const uci_move_t move_candidate = parsing_result.value();

          unpacked_move_t move(0);
          move.from = move_candidate.from;
          move.to = move_candidate.to;
          move.promoted_to = move_candidate.promotion;

          // Attempt the move. We ignore if move happened or not
          bool res = try_move(&move);

          if (res) {
            LOG_I << "Applied move [" << move_str << "]" << END_I;
          } else {
            LOG_W << "Failed move [" << move_str << "]" << END_W;
          }
        }
      }
    }
  }


  LOG_I << print_nice_board(&game.board) << END_I;

  return true;
}


bool pop_int(std::queue<std::string>& args,
             const char* name,
             int& out,
             int min,
             int max)
{
  if (args.empty()) {
    LOG_W << name << " is missing its value" << END_W;
    return false;
  }

  const std::string token = args.front();
  args.pop();

  try {
    const long long value = std::stoll(token);
    out = static_cast<int>(std::clamp<long long>(value, min, max));
  } catch (...) {
    LOG_W << name << " is not a number: " << token << END_W;
    return false;
  }

  return true;
}


bool pop_u64(std::queue<std::string>& args, const char* name, uint64_t& out)
{
  if (args.empty()) {
    LOG_W << name << " is missing its value" << END_W;
    return false;
  }

  const std::string token = args.front();
  args.pop();

  try {
    // Parsed as signed on purpose: stoull silently wraps a negative literal
    const long long value = std::stoll(token);
    out = static_cast<uint64_t>(std::max<long long>(value, 0));
  } catch (...) {
    LOG_W << name << " is not a number: " << token << END_W;
    return false;
  }

  return true;
}


bool command_go(std::queue<std::string>& args)
{
  LOG_I << "Command [go]. Args: " << args << END_I;

  uci_search_options_t search_options = {};
  search_options.infinite = false;
  search_options.depth = MAX_DEPTH;
  search_options.nodes = 0;
  // Not seeded with a move count. No [movestogo] on the line means sudden
  // death, and compute_search_time_budget() allocates a share of the clock
  // instead of dividing by a number nobody sent. S089.
  search_options.movestogo = 0;
  search_options.winc_ms = 0;
  search_options.binc_ms = 0;
  search_options.search_time_ms = 0;
  search_options.search_soft_time_ms = 0;
  search_options.scale_time = false;

  const int int_max = std::numeric_limits<int>::max();

  // `depth` cannot be inspected for this, it is pre-seeded with MAX_DEPTH so
  // that an unconstrained search still terminates somewhere.
  bool depth_given = false;

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "depth") {
      depth_given = pop_int(args, "Depth", search_options.depth, 1, MAX_DEPTH);
    } else if (token == "movetime") {
      pop_int(args, "Movetime", search_options.movetime_ms, 0, int_max);
    } else if (token == "nodes") {
      pop_u64(args, "Nodes", search_options.nodes);
    } else if (token == "wtime") {
      pop_int(args, "Wtime", search_options.wtime_ms, 0, int_max);
    } else if (token == "btime") {
      pop_int(args, "Btime", search_options.btime_ms, 0, int_max);
    } else if (token == "winc") {
      pop_int(args, "Winc", search_options.winc_ms, 0, int_max);
    } else if (token == "binc") {
      pop_int(args, "Binc", search_options.binc_ms, 0, int_max);
    } else if (token == "movestogo") {
      pop_int(args, "Movestogo", search_options.movestogo, 1, int_max);
    } else if (token == "infinite") {
      search_options.infinite = true;
    } else if (token == "mate" || token == "searchmoves" || token == "ponder") {
      // Not supported. UCI says to ignore what we do not implement, and the
      // rest of the line still carries the time control we need.
      LOG_W << "[go " << token << "] not implemented, ignored" << END_W;
    }
  }

  stop_and_join_search();
  begin_search_session();

  // Book is searched only if the command make sense
  if (!search_options.infinite && search_options.nodes == 0) {
    const move_t book_move = search_book_move();

    if (book_move) {
      // We got book move, print and return straight away
      const uci_move_t uci_book_move = {
          MOVE_FROM(book_move), MOVE_TO(book_move), MOVE_PROMOTED(book_move)};

      const std::string best_move_str = uci_move_to_algebraic(&uci_book_move);

      // uci_reply("info score 0 depth 1 nodes 1 pv " + best_move_str);

      uci_reply("bestmove " + best_move_str);

      return true;
    }
  }

  // These three cases are mutually exclusive.
  if (search_options.infinite) {
    search_options.search_time_ms = 0;
    LOG_I << "Infinite search. Only [stop] ends it" << END_I;

  } else if (search_options.movetime_ms > 0) {
    // The GUI named the time. Both limits are it, and nothing scales them.
    search_options.search_time_ms = search_options.movetime_ms;
    search_options.search_soft_time_ms = search_options.movetime_ms;
    stop_search_after_ms(search_options.search_time_ms);

    LOG_I << "Movetime set. Search will stop in "
          << search_options.search_time_ms << "ms" << END_I;

  } else {
    const bool is_white = (game.board.active_color == WHITE);
    const int remaining_ms =
        is_white ? search_options.wtime_ms : search_options.btime_ms;
    const int increment_ms =
        is_white ? search_options.winc_ms : search_options.binc_ms;

    if (remaining_ms > 0) {
      const search_time_budget_t budget = compute_search_time_budget(
          remaining_ms, increment_ms, search_options.movestogo);

      search_options.search_time_ms = budget.hard_ms;
      search_options.search_soft_time_ms = budget.soft_ms;
      search_options.scale_time = true;

      LOG_I << "Time budget " << budget.soft_ms << "ms soft, " << budget.hard_ms
            << "ms hard, out of " << remaining_ms << "ms remaining" << END_I;

    } else if (!depth_given && search_options.nodes == 0) {
      // Nothing bounds this search: no clock (or a clock already at zero), no
      // depth, no node budget. Answering late is bad, never answering is worse.
      search_options.search_time_ms = FALLBACK_SEARCH_TIME_MS;

      // The same soft share the clock path takes, so this path keeps the shape
      // it had before S089 split the limits: it is a safety net, not a control
      // to react to, and it is not scaled.
      search_options.search_soft_time_ms =
          (FALLBACK_SEARCH_TIME_MS * TM_SOFT_PERCENT) / 100;

      LOG_W << "[go] carries no usable limit, falling back to "
            << search_options.search_time_ms << "ms" << END_W;
    }

    if (search_options.search_time_ms > 0) {
      stop_search_after_ms(search_options.search_time_ms);
    }
  }

  // Start search in a thread
  search_thread = std::thread([search_options]() {
    stopwatch_t timer;
    const uci_search_result_t res = iterative_deepening_search(search_options);

    const std::string best_move_str = best_move_to_string(res);

    std::string ponder_move;
    if (res.is_ponder_move) {
      ponder_move = " ponder " + uci_move_to_algebraic(&res.ponder_move);
    }

    uci_reply("bestmove " + best_move_str + ponder_move);
    LOG_I << "Search time: " << timer.duration_str() << END_I;
  });

  return true;
}


bool command_stop(std::queue<std::string>& args)
{
  LOG_I << "Command [stop]. Args: " << args << END_I;

  stop_search_signal = true;

  return true;
}


bool command_ponderhit(std::queue<std::string>& args)
{
  LOG_I << "Command [ponderhit]. Args: " << args << END_I;

  // TODO

  return true;
}


bool command_quit(std::queue<std::string>& args)
{
  LOG_I << "Command [quit]. Args: " << args << END_I;

  stop_and_join_search();

  running = false;
  return true;
}


bool command_print_board(std::queue<std::string>& args)
{
  LOG_I << "Command [print_board]. Args: " << args << END_I;

  // The search thread mutates game.board as it walks the tree. Reading it from
  // here without joining is a data race that prints a board belonging to no
  // real position.
  stop_and_join_search();

  LOG_I << print_nice_board(&game.board) << END_I;

  uci_reply(print_nice_board(&game.board));

  return true;
}


bool command_fen(std::queue<std::string>& args)
{
  LOG_I << "Command [command_fen]. Args: " << args << END_I;

  // Same race as command_print_board().
  stop_and_join_search();

  LOG_I << "position fen " << generate_FEN(&game.board) << END_I;

  uci_reply(generate_FEN(&game.board));

  return true;
}


bool command_help(std::queue<std::string>& args)
{
  LOG_I << "Command [command_help]. Args: " << args << END_I;

  uci_reply("--- Help: available commands ---");
  for (const std::string& name : uci_command_names()) {
    uci_reply(name);
  }
  uci_reply("--------------------------------");

  return true;
}


bool command_test(std::queue<std::string>& args)
{
  // LOG_I << "Command [command_help]. Args: " << args << END_I;

  struct test_entry_t
  {
    std::string FEN;
    std::string title;
  };

  // clang-format off
  std::array<test_entry_t, 7> entries = {{
      {DEFAULT_POSITION,  "DEFAULT_POSITION"},
      {TRICKY_POS,        "TRICKY_POS         bestmove e2a6 ponder b4c3"},
      {KILLER_POS,        "KILLER_POS         bestmove g7h8q ponder d8h4"},
      {CMK_POS,           "CMK_POS            bestmove h7h6 ponder c2c3"},
      {FINE_70_POS,       "FINE_70_POS        bestmove a1b2 ponder a7b7"},
      {MATE_IN_2_W_POS,   "MATE_IN_2_W_POS    bestmove e5e6 ponder e8d8"},
      {MATE_IN_2_B_POS,   "MATE_IN_2_B_POS    bestmove e5e6 ponder e8d8"}
    }};
  // clang-format on

  uint64_t total_nodes = 0;
  uci_search_options_t search_options = {};
  search_options.infinite = false;
  search_options.depth = 6;

  if (!args.empty()) {
    pop_int(args, "Depth", search_options.depth, 1, MAX_DEPTH);
  }

#ifdef NDEBUG
  std::string build_type = "Release";
#else
  std::string build_type = "Debug  ";
#endif


  uci_reply("\nTESTS START ----------------------\nDepth: " +
            STR(search_options.depth) + "\nBuild type: " + build_type +
            "\nDescription:");

  stopwatch_t total_timer;
  total_timer.stop();

  stop_and_join_search();
  begin_search_session();

  for (auto const& entry : entries) {
    uci_reply("");

    set_position(entry.FEN);
    uci_reply(entry.title + "\n" + generate_FEN(&game.board));

    uci_search_result_t res;

    total_timer.start();
    stopwatch_t timer;
    res = iterative_deepening_search(search_options);
    timer.stop();
    total_timer.stop();

    const std::string best_move_str = best_move_to_string(res);

    std::string ponder_move;
    if (res.is_ponder_move) {
      ponder_move = " ponder " + uci_move_to_algebraic(&res.ponder_move);
    }

    uci_reply("bestmove " + best_move_str + ponder_move);
    total_nodes += res.total_node_explored;

    if (!check_move_legality(res.best_move)) {
      uci_reply("!!! ----- Best move is ILLEGAL ----- !!!");
    }

    if (!is_pv_legal(&game, &res.pv)) {
      uci_reply("!!! ----- PV move is ILLEGAL   ----- !!!");
    }

    uci_reply("Search time: " + timer.duration_str());
  }

  uci_reply("\nTESTS END ------------------------");
  uci_reply("Total explored nodes: " + STR(total_nodes));
  uci_reply("Search time: " + total_timer.duration_str());

  return true;
}


bool command_clean_TT(std::queue<std::string>& args)
{
  LOG_I << "Command [command_clean_TT]. Args: " << args << END_I;
  tt_reset(&tt);

  return true;
}


//-###########################  ENGINE ENTRY POINTS  ########################-//
// main() lives in main.cpp so that this translation unit can be linked into
// the test binaries. Everything below is what main() used to do inline.

void uci_init()
{
  LOG_I << "Engine started" << END_I;

  initialize_game_const_data(&game);

  load_FEN(DEFAULT_POSITION, &game);
  try_load_opening_book();

  tt_resize(&tt, TT_DEFAULT_MB);

  running = true;
}


void uci_shutdown()
{
  stop_and_join_search();
  tt_free(&tt);

  LOG_I << "Engine closed gracefully" << END_I;
}


bool uci_is_running()
{ return running; }


// Sorted so that the order is the dispatch table's contents and not
// unordered_map's bucket layout, which changes with the key set.
std::vector<std::string> uci_command_names()
{
  std::vector<std::string> names;
  names.reserve(commands.size());

  for (const auto& pair : commands) {
    names.push_back(pair.first);
  }

  std::sort(names.begin(), names.end());

  return names;
}


void uci_process_line(const std::string& input)
{
  std::queue<std::string> tokens = tokenize_input(input, " ");

  if (tokens.size() == 0) {
    LOG_W << "No tokens in string" << END_W;
    return;
  }

  // Try to find the command
  while (!tokens.empty()) {
    const std::string current_token = tokens.front();
    tokens.pop();

    if (is_command(current_token)) {
      const bool result = commands.at(current_token)(tokens);

      // UCI requires malformed or unsupported input to be ignored.
      if (!result) {
        LOG_W << "Command [" << current_token << "] error" << END_W;
      }

      // We found and executed the command for this input. So jump to next
      return;
    }
  }
}