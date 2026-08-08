#include <algorithm>
#include <atomic>
#include <cassert>
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
#include "transposition_table.hpp"
#include "uci.hpp"
#include "utils.hpp"


//-##############################    GLOBALS    #############################-//
static game_t game = {};
static std::string initial_position = DEFAULT_POSITION;
static bool opening_book_loaded = false;
static bool opening_book_enabled = false;
static bool still_in_opening = true;  // Finish the opening line
static book_t opening_book;

static std::atomic_bool stop_search_signal = false;
static std::atomic_int session_id = 0;
static transposition_table_t tt = {};

static std::thread search_thread;
static std::mutex output_mutex;

static std::random_device rd;
static std::mt19937_64 gen(rd());

static bool is_debug = false;
static bool running = true;


//-#############################   DECLARATIONS  ############################-//
typedef bool (*process_func)(std::queue<std::string>&);


const game_t* uci_game()
{ return &game; }


const transposition_table_t* uci_tt()
{ return &tt; }


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
  const size_t moves_size = generate_moves(&game.tables, &game.board, moves);

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
  if (!load_FEN(fen, &game)) {
    LOG_E << "Failed to load FEN [" << fen << "]. Restoring previous position"
          << END_E;

    load_FEN(initial_position, &game);
    return false;
  }

  if (initial_position != fen) {
    initial_position = fen;

    tt_reset(&tt);
    still_in_opening = true;
  }

  return true;
}


void try_load_opening_book()
{
  opening_book_loaded = load_book_embedded(&opening_book);

  if (opening_book_loaded) {
    LOG_I << "Opening book loaded correctly! " << opening_book.num_of_positions
          << " entries." << END_I;
  } else {
    LOG_E << "Failed to load the opening book." << END_E;
  }
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

    if (session_id == session) { stop_search_signal = true; }
  });

  // Left the timer be, we return! Adios
  job.detach();
}


int compute_search_time_ms(int remaining_ms, int increment_ms, int movestogo)
{
  assert(movestogo > 0);
  assert(remaining_ms > 0);

  int budget = (remaining_ms / movestogo) + (increment_ms / 2);

  // Never budget more than is actually left
  budget = std::min(budget, remaining_ms - MOVE_OVERHEAD_MS);
  budget = std::max(budget, std::min(50, remaining_ms / 2));

  return budget;
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
  const size_t moves_count = generate_moves(&game.tables, &game.board, moves);

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
  const size_t count = generate_moves(&game.tables, &game.board, moves);

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


move_t search_random_move_in_book()
{
  move_t result = {};

  if (opening_book_loaded && opening_book_enabled && still_in_opening) {
    move_t moves[MAX_MOVES];
    const size_t moves_cout =
        get_book_moves_for_key(&opening_book, &game.board, moves);

    // A book move is reported to the GUI as the best move without ever being
    // searched or played, so nothing else would catch a bad one. A polyglot
    // key collision or a malformed book used to forfeit the game outright.
    move_t legal_moves[MAX_MOVES];
    size_t legal_count = 0;

    for (size_t i = 0; i < moves_cout; ++i) {
      const move_t validated = validate_book_move(moves[i]);

      if (validated != 0) { legal_moves[legal_count++] = validated; }
    }

    if (legal_count != moves_cout) {
      LOG_W << "Opening book returned " << (moves_cout - legal_count)
            << " illegal move(s) for this position, discarded" << END_W;
    }

    if (legal_count > 0) {
      // Extreme included
      std::uniform_int_distribution<size_t> dist(0, legal_count - 1);

      const size_t index = dist(gen);
      assert(index < legal_count);
      result = legal_moves[index];

      LOG_I << "Found position in the opening book." << END_I;
    } else {
      // We finish the move lines or move not found, disabling it
      still_in_opening = false;
    }
  }

  return result;
}


// A search cut off before it finishes even one root move still owes the GUI a
// move. Only a position with no legal move at all may answer with the UCI null
// move.
move_t first_legal_move()
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(&game.tables, &game.board, moves);

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
  assert(state.tt != nullptr);

  tt_new_search(state.tt);

  std::atomic_bool never_stop = false;
  state.stop = &never_stop;

  const auto beguine_of_the_search = std::chrono::steady_clock::now();
  std::chrono::steady_clock::duration last_iteration = {};

  // Carried across iterations so an aborted one can report the last score and
  // depth that actually mean something alongside the line it will play.
  std::string last_score = "cp 0";
  int last_complete_depth = 0;

  const int soft_percent =
      (conf.movetime_ms > 0) ? 100 : SEARCH_SOFT_LIMIT_PERCENT;
  const double soft_limit_ms =
      (static_cast<double>(conf.search_time_ms) * soft_percent) / 100.0;

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

    const auto start_time = std::chrono::steady_clock::now();

    const search_t search_result = search(current_depth, &game, &state);

    const auto end_time = std::chrono::steady_clock::now();
    last_iteration = end_time - start_time;

    const auto duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(last_iteration);

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
      last_complete_depth = current_depth;
    }

    // Reported whenever there is a line to report, aborted iteration included.
    // An aborted iteration that produced a PV supplies the move that will be
    // played, so staying silent about it leaves the GUI holding a principal
    // variation from the previous depth and a bestmove that does not start it.
    // The score and depth of an unfinished iteration mean nothing - its window
    // never closed - so the last completed ones are repeated instead, and the
    // line printed is the one that will actually be played.
    if (has_result || !state.aborted) {
      uci_reply("info score " + last_score + " time " +
                STR(duration_ms.count()) + " depth " +
                STR(last_complete_depth) + " nodes " +
                STR(search_result.explored_nodes) + " pv " +
                pv_to_string(&result.pv));
    }

    if (state.aborted) { break; }

    if (stop_search_signal) { break; }
    if (conf.nodes != 0 && result.total_node_explored >= conf.nodes) { break; }

    if (conf.search_time_ms > 0) {
      const double elapsed_ms =
          std::chrono::duration<double, std::milli>(
              std::chrono::steady_clock::now() - beguine_of_the_search)
              .count();

      if (elapsed_ms >= soft_limit_ms) { break; }
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
  uci_reply("option name Use Book type check default false");
  uci_reply("option name Hash type spin default " + STR(TT_DEFAULT_MB) +
            " min " + STR(TT_MIN_MB) + " max " + STR(TT_MAX_MB));
  uci_reply("option name Threads type spin default 1 min 1 max 1");
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
      if (!args.empty()) {
        option_value = args.front();
        args.pop();
      }
    }
  }

  // Handle specific options
  if (option_name == "Use Book" && option_value == "true") {
    opening_book_enabled = true;
    LOG_I << "Use Book ON" << END_I;
  }

  if (option_name == "Use Book" && option_value == "false") {
    opening_book_enabled = false;
    LOG_I << "Use Book OFF" << END_I;
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
      // Reading the fen string. Fen string contains 6 portions
      if (args.size() < 6) {
        // The fen string is not complete
        return false;
      }

      std::string fen;

      for (int i = 0; i < 6; ++i) {
        fen += args.front() + " ";
        args.pop();
      }

      fen = trim_whitespace(fen);

      // Initialize the board with the fen
      bool res = set_position(fen);

      if (res) {
        LOG_I << "Set fen " << fen << END_I;
      } else {
        LOG_W << "Failed to set fen " << fen << END_W;
      }
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
  search_options.movestogo = DEFAULT_MOVES_TO_GO;
  search_options.winc_ms = 0;
  search_options.binc_ms = 0;
  search_options.search_time_ms = 0;

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

  session_id++;
  stop_search_signal = false;

  // Book is searched only if the command make sense
  if (!search_options.infinite && search_options.nodes == 0) {
    const move_t book_move = search_random_move_in_book();

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
    search_options.search_time_ms = search_options.movetime_ms;
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
      search_options.search_time_ms = compute_search_time_ms(
          remaining_ms, increment_ms, search_options.movestogo);

      LOG_I << "Time budget " << search_options.search_time_ms << "ms out of "
            << remaining_ms << "ms remaining" << END_I;

    } else if (!depth_given && search_options.nodes == 0) {
      // Nothing bounds this search: no clock (or a clock already at zero), no
      // depth, no node budget. Answering late is bad, never answering is worse.
      search_options.search_time_ms = FALLBACK_SEARCH_TIME_MS;

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
  for (const auto& pair : commands) {
    const std::string& key = pair.first;
    uci_reply(key);
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
  stop_search_signal = false;

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