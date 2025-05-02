#include <algorithm>
#include <atomic>
#include <cassert>
#include <future>
#include <iostream>
#include <limits>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>
#include "board.hpp"
#include "experimental_search.hpp"
#include "log.hpp"
#include "move_generator.hpp"
#include "openings.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"


//-##############################    GLOBALS    #############################-//
static board_t board = {};
static global_state_t globals = {};
static std::string initial_position = DEFAULT_POSITION;
static bool opening_book_loaded = false;
static bool opening_book_enabled = true;  // User cna disable the book
static bool still_in_opening = true;      // Finish the opening line
static book_t opening_book;

static std::atomic_bool stop_search_signal = false;
static transposition_table_t tt = {};

static std::random_device rd;
static std::mt19937_64 gen(rd());

static bool is_debug = false;
static bool running = true;


//-##############################   DATA TYPES  #############################-//
struct uci_search_options_t
{
  // Modifiers
  std::vector<move_t> searchmoves;
  bool ponder = false;

  // Exclusive fixed Limit Options
  int depth = 0;
  uint64_t nodes = 0;
  int movetime_ms = 0;
  int mate_in_n_moves = 0;
  bool infinite = false;

  // Time control options
  int wtime_ms = 0;
  int btime_ms = 0;
  int winc_ms = 0;
  int binc_ms = 0;
  int movestogo = 1;
};

struct uci_move_t
{
  index_t from;
  index_t to;
  promotion_t promotion;
};

struct uci_search_result_t
{
  uci_move_t uci_best_move;
  bool is_ponder_move = false;
  uci_move_t ponder_move;
  move_t best_move;
  uint64_t total_node_explored;
  pv_t pv;
};


//-#############################   DECLARATIONS  ############################-//
typedef bool (*process_func)(std::queue<std::string>&);
std::string pv_to_string(const pv_t* pv);


void uci_reply(const std::string& response)
{
  std::cout << response << std::endl;
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
  result += index_to_string_coordinates(move->from);
  result += index_to_string_coordinates(move->to);

  if (move->promotion != TO_NONE) {
    result += promotion_to_string(move->promotion);
  }

  return result;
}


std::string pv_to_string(const pv_t* pv)
{
  assert(pv != nullptr);

  std::stringstream ss;

  for (size_t i = 0; i < pv->pv_length[0]; ++i) {
    const uci_move_t move = {pv->pv_table[0][i].from, pv->pv_table[0][i].to,
                             pv->pv_table[0][i].promoted_to};

    ss << uci_move_to_algebraic(&move) << " ";
  }

  return ss.str();
}


bool check_move_legality(const move_t* move)
{
  move_t moves[MAX_MOVES];
  const size_t moves_size = generate_legal_moves(&board, &globals, moves);

  if (moves_size < 1) {
    LOG_E << *move << " ILLEGAL. No move available in this position" << END_E;
    return false;
  }

  bool found = false;
  for (size_t i = 0; i < moves_size; ++i) {
    if (*move == moves[i]) {
      found = true;
      break;
    }
  }

  if (!found) {
    LOG_E << *move << " ILLEGAL. Is not in the legal move list" << END_E;
    return false;
  }

  // Attempt to make the move
  const bool res = make_move(move, &board, &globals);
  unmake_move(&board, &globals);

  if (!res) {
    LOG_E << *move << " ILLEGAL. Failed to make the move" << END_E;
    return false;
  }


  return true;
}


//-#############################    FUNCTIONS    ############################-//

bool set_position(const std::string& fen)
{
  init_board(fen, &board, &globals);

  if (initial_position != fen) {
    initial_position = fen;

    // We changed game, reset TT
    tt_reset(&tt);
  }
  still_in_opening = false;
  return true;
}


bool try_load_opening_book()
{
  opening_book_loaded = load_book_embedded(&opening_book);

  if (opening_book_loaded) {
    LOG_I << "Opening book loaded correctly! " << opening_book.num_of_positions
          << " entries." << END_I;
  } else {
    LOG_E << "Failed to load the opening book." << END_E;
  }

  return opening_book_loaded;
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
  std::thread job([ms]() {
    std::chrono::milliseconds time_to_sleep(ms);
    std::this_thread::sleep_for(time_to_sleep);

    stop_search_signal = true;
  });

  // Left the timer be, we return! Adios
  job.detach();
}


bool is_command(const std::string& command)
{
  auto it = commands.find(command);
  if (it != commands.end()) { return true; }
  return false;
}


bool try_move(move_t move_candidate)
{
  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(&board, &globals, moves);

  // Fix the possible weirdo move notation for castling
  fix_weirdo_castling(&board, &move_candidate);

  // Search the move in the list of legal moves
  for (size_t i = 0; i < moves_count; ++i) {
    const move_t& move = moves[i];

    // Set this so we can perform the comparison
    move_candidate.piece = move.piece;

    if (move == move_candidate) {
      // Apply the found move
      bool move_result = make_move(&move, &board, &globals);

      if (move_result) { return true; }

      break;
    }
  }

  return false;
}


move_t search_random_move_in_book()
{
  move_t result = {};

  if (opening_book_loaded && opening_book_enabled && still_in_opening) {
    move_t moves[MAX_MOVES];
    const size_t moves_cout =
        get_book_moves_for_key(&opening_book, &board, moves);

    if (moves_cout > 0) {
      // Extreme included
      std::uniform_int_distribution<size_t> dist(0, moves_cout - 1);

      const size_t index = dist(gen);
      assert(index < moves_cout);
      result = moves[index];

      LOG_I << "Found position in the opening book." << END_I;
    } else {
      // We finish the move lines or move not found, disabling it
      still_in_opening = false;
    }
  }

  return result;
}


uci_search_result_t iterative_deepening_search(const uci_search_options_t& conf)
{
  uci_search_result_t result = {};
  result.total_node_explored = 0;

  // If no move found in the book search by engine
  search_state_t state = {};
  state.stop = &stop_search_signal;
  state.tt = &tt;
  assert(state.tt != nullptr);
  stop_search_signal = false;

  for (int current_depth = 1; current_depth <= conf.depth; ++current_depth) {
    // Iterative deepening

    // Reset the explored nodes in the previous iteration
    state.explored_nodes = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    // const search_t search_result =
    //     search_best_move(current_depth, &board, &globals, &state);
    const search_t search_result =
        experimental_search(current_depth, &board, &globals, &state);

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                              start_time);

    // If we interupted the current search we use the previous result
    if (stop_search_signal) { break; }

    const std::string score =
        search_result.mate_found
            ? ("mate " + std::to_string(search_result.mate_in))
            : ("cp " + std::to_string(search_result.score));

    uci_reply("info score " + score + " time " +
              std::to_string(duration_ms.count()) + " depth " +
              std::to_string(current_depth) + " nodes " +
              std::to_string(search_result.explored_nodes) + " pv " +
              pv_to_string(&search_result.pv));

    result.best_move = search_result.best_move;
    result.uci_best_move = {search_result.best_move.from,
                            search_result.best_move.to,
                            search_result.best_move.promoted_to};

    result.is_ponder_move = false;
    if (search_result.pv.pv_length[0] > 1) {
      result.is_ponder_move = true;
      result.ponder_move = {search_result.pv.pv_table[0][1].from,
                            search_result.pv.pv_table[0][1].to,
                            search_result.pv.pv_table[0][1].promoted_to};
    }

    result.pv = search_result.pv;

    result.total_node_explored += search_result.explored_nodes;

    // Check if run out of nodes
    if (conf.nodes != 0 && search_result.explored_nodes > conf.nodes) { break; }
  }

  return result;
}


//-################################  COMMANDS  ##############################-//
bool command_uci(std::queue<std::string>& args)
{
  LOG_I << "Command [uci]. Args: " << args << END_I;

  uci_reply("id name Chesso");
  uci_reply("id author MazerFaker");
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

  // setoption name <id> [value <x>]

  // TODO

  return true;
}


bool command_register(std::queue<std::string>& args)
{
  LOG_I << "Command [register]. Args: " << args << END_I;
  if (args.size() == 0) { return false; }

  // TODO

  return true;
}


bool command_ucinewgame(std::queue<std::string>& args)
{
  LOG_I << "Command [ucinewgame]. Args: " << args << END_I;

  set_position(DEFAULT_POSITION);
  tt_reset(&tt);
  still_in_opening = true;

  LOG_I << print_nice_board(&board) << END_I;

  return true;
}


bool command_position(std::queue<std::string>& args)
{
  LOG_I << "Command [position]. Args: " << args << END_I;

  if (args.size() == 0) { return false; }

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

          move_t move = {};
          move.from = move_candidate.from;
          move.to = move_candidate.to;
          move.promoted_to = move_candidate.promotion;

          // Attempt the move. We ignore if move happened or not
          bool res = try_move(move);

          if (res) {
            LOG_I << "Applied move [" << move_str << "]" << END_I;
          } else {
            LOG_W << "Failed move [" << move_str << "]" << END_W;
          }
        }
      }
    }
  }

  still_in_opening = true;

  LOG_I << print_nice_board(&board) << END_I;

  return true;
}


bool command_go(std::queue<std::string>& args)
{
  LOG_I << "Command [go]. Args: " << args << END_I;

  uci_search_options_t search_options = {};
  search_options.infinite = false;
  search_options.depth = MAX_DEPTH;
  search_options.nodes = 0;
  search_options.movestogo = 20;  // Assume by default we have 10 moves to go
  search_options.winc_ms = 0;
  search_options.binc_ms = 0;

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "depth") {
      const std::string depth_token = args.front();
      args.pop();

      try {
        search_options.depth = std::stoi(depth_token);
      } catch (...) {
        LOG_W << "Depth is not a number: " << depth_token << END_W;
        return false;
      }

    } else if (token == "movetime") {
      const std::string movetime_ms_token = args.front();
      args.pop();

      try {
        search_options.movetime_ms = std::stoi(movetime_ms_token);
      } catch (...) {
        LOG_W << "Movetime is not a number: " << movetime_ms_token << END_W;
        return false;
      }
    } else if (token == "nodes") {
      const std::string nodes_token = args.front();
      args.pop();

      try {
        search_options.nodes = std::stoi(nodes_token);
      } catch (...) {
        LOG_W << "Nodes is not a number: " << nodes_token << END_W;
        return false;
      }
    } else if (token == "mate") {
      // TODO: To implement
      LOG_W << "Not implemented" << END_W;
      return false;
    } else if (token == "infinite") {
      search_options.infinite = true;
    }

    if (token == "searchmoves") {
      // TODO: To implement
      LOG_W << "Not implemented" << END_W;
      return false;
    }

    if (token == "ponder") {
      // TODO: To implement
      LOG_W << "Not implemented" << END_W;
      return false;
    }

    if (token == "wtime") {
      const std::string wtime_token = args.front();
      args.pop();

      try {
        search_options.wtime_ms = std::stoi(wtime_token);
      } catch (...) {
        LOG_W << "Wtime is not a number: " << wtime_token << END_W;
        return false;
      }
    }

    if (token == "btime") {
      const std::string btime_token = args.front();
      args.pop();

      try {
        search_options.btime_ms = std::stoi(btime_token);
      } catch (...) {
        LOG_W << "Btime is not a number: " << btime_token << END_W;
        return false;
      }
    }

    if (token == "winc") {
      const std::string winc_token = args.front();
      args.pop();

      try {
        search_options.winc_ms = std::stoi(winc_token);
      } catch (...) {
        LOG_W << "Winc is not a number: " << winc_token << END_W;
        return false;
      }
    }

    if (token == "binc") {
      const std::string binc_token = args.front();
      args.pop();

      try {
        search_options.binc_ms = std::stoi(binc_token);
      } catch (...) {
        LOG_W << "Binc is not a number: " << binc_token << END_W;
        return false;
      }
    }

    if (token == "movestogo") {
      const std::string movestogo_token = args.front();
      args.pop();

      try {
        search_options.movestogo = std::stoi(movestogo_token);
        if (search_options.movestogo == 0) { search_options.movestogo = 1; }

      } catch (...) {
        LOG_W << "Movestogo is not a number: " << movestogo_token << END_W;
        return false;
      }
    }
  }

  // Book is searched only if the command make sense
  if (!search_options.infinite && search_options.nodes == 0) {
    const move_t book_move = search_random_move_in_book();

    if (book_move) {
      // We got book move, print and return straight away
      const uci_move_t uci_book_move = {book_move.from, book_move.to,
                                        book_move.promoted_to};

      const std::string best_move_str = uci_move_to_algebraic(&uci_book_move);
      uci_reply("bestmove " + best_move_str);

      return true;
    }
  }

  // Calculate the time to play for white if set
  if (search_options.wtime_ms > 0 && board.active_color == WHITE) {
    int time_to_play = (search_options.wtime_ms / search_options.movestogo) +
                       search_options.winc_ms - 10;

    if (time_to_play < 100) { time_to_play = 100; }

    stop_search_after_ms(time_to_play);

    LOG_I << "Time to play for white calculated. Search will stop in "
          << time_to_play << "ms" << END_I;
  }

  // Calculate the time to play for black if set
  if (search_options.btime_ms > 0 && board.active_color == BLACK) {
    int time_to_play = (search_options.btime_ms / search_options.movestogo) +
                       search_options.binc_ms - 10;

    if (time_to_play < 100) { time_to_play = 100; }

    stop_search_after_ms(time_to_play);

    LOG_I << "Time to play for black calculated. Search will stop in "
          << time_to_play << "ms" << END_I;
  }

  // Start search in a thread
  std::thread search_thread([search_options]() {
    stopwatch_t timer;
    const uci_search_result_t res = iterative_deepening_search(search_options);

    const std::string best_move_str = uci_move_to_algebraic(&res.uci_best_move);

    std::string ponder_move;
    if (res.is_ponder_move) {
      ponder_move = " ponder " + uci_move_to_algebraic(&res.ponder_move);
    }

    uci_reply("bestmove " + best_move_str + ponder_move);
  });

  // Let the thread go his way
  search_thread.detach();

  // Start the move timer if necessary
  if (search_options.movetime_ms > 0) {
    stop_search_after_ms(search_options.movetime_ms);

    LOG_I << "Movetimes set. Search will stop in " << search_options.movetime_ms
          << "ms" << END_I;
  }

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

  running = false;
  return true;
}


bool command_print_board(std::queue<std::string>& args)
{
  LOG_I << "Command [print_board]. Args: " << args << END_I;
  LOG_I << print_nice_board(&board) << END_I;

  uci_reply(print_nice_board(&board));

  return true;
}


bool command_fen(std::queue<std::string>& args)
{
  LOG_I << "Command [command_fen]. Args: " << args << END_I;
  LOG_I << "position fen " << generate_FEN(&board) << END_I;

  uci_reply(generate_FEN(&board));

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
    const std::string depth_token = args.front();
    args.pop();

    try {
      search_options.depth = std::stoi(depth_token);
    } catch (...) {
      LOG_W << "Depth is not a number: " << depth_token << END_W;
    }
  }

#ifdef NDEBUG
  std::string build_type = "Release";
#else
  std::string build_type = "Debug  ";
#endif


  uci_reply("\nTESTS START ----------------------\nDepth: " +
            std::to_string(search_options.depth) +
            "\nBuild type: " + build_type + "\nDescription:");

  stopwatch_t total_timer;
  total_timer.stop();

  for (auto const& entry : entries) {
    uci_reply("");

    set_position(entry.FEN);
    uci_reply(entry.title + "\n" + generate_FEN(&board));

    uci_search_result_t res;

    total_timer.start();
    stopwatch_t timer;
    res = iterative_deepening_search(search_options);
    timer.stop();
    total_timer.stop();

    const std::string best_move_str = uci_move_to_algebraic(&res.uci_best_move);

    std::string ponder_move;
    if (res.is_ponder_move) {
      ponder_move = " ponder " + uci_move_to_algebraic(&res.ponder_move);
    }

    uci_reply("bestmove " + best_move_str + ponder_move);
    total_nodes += res.total_node_explored;

    if (!check_move_legality(&res.best_move)) {
      uci_reply("!!! ----- Best move is ILLEGAL ----- !!!");
    }

    if (!is_pv_legal(&board, &globals, &res.pv)) {
      uci_reply("!!! ----- PV move is ILLEGAL   ----- !!!");
    }
  }

  uci_reply("\nTESTS END ------------------------");
  uci_reply("Total explored nodes: " + std::to_string(total_nodes));

  return true;
}


//-##################################  MAIN  ################################-//
int main()
{
  LOG_I << "Engine started" << END_I;

  // Print the engine info
  std::queue<std::string> tokens;
  // (void)command_uci(tokens);

  // Initialization
  init_board(DEFAULT_POSITION, &board, &globals);
  (void)try_load_opening_book();
  still_in_opening = true;

  tt_reset(&tt);

  while (running) {
    std::string input;
    std::getline(std::cin, input);

    tokens = tokenize_input(input, " ");

    if (tokens.size() == 0) {
      // TODO: There is a bug. This is spammed. Understand why and how
      LOG_W << "No tokens in string" << END_W;
      continue;
    }

    // Try to find the command
    while (!tokens.empty()) {
      const std::string current_token = tokens.front();
      tokens.pop();

      if (is_command(current_token)) {
        const bool result = commands.at(current_token)(tokens);

        if (!result) {
          LOG_W << "Command [" << current_token << "] error" << END_W;
          return 1;
        }

        // We found and executed the command for this input. So jump to next
        break;
      }
    }
  }

  LOG_I << "Engine closed gracefully" << END_I;

  return 0;
}
