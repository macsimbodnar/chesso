#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>
#include "board.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "move_generator.hpp"
#include "search.hpp"
#include "utils.hpp"


static std::ofstream log_file("chesso_engine.log", std::ios::app);

#define LOG_I log_file   // Start log
#define END_I std::endl  // End log

#define LOG_S LOG_I << "\033[92m"  // Success green log
#define END_S "\033[37m" << END_I  // End success green log

#define LOG_W LOG_I << "\033[33m"  // Warning orange log
#define END_W "\033[37m" << END_I  // End warning orange log

#define LOG_E LOG_I << "\033[31m"  // Error red log
#define END_E "\033[37m" << END_I  // End Error red log


//-##############################   DATA TYPES  #############################-//

typedef bool (*process_func)(std::queue<std::string>&);

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

struct uci_move_t
{
  index_t from;
  index_t to;
  promotion_t promotion;
};


class engine_handler_t
{
private:
  history_t history;
  board_t board;
  std::string initial_position = DEFAULT_POSITION;

public:
  engine_handler_t() { set_default_position(); }

  bool set_position(const std::string& fen)
  {
    init_board(fen, &board, &history);
    initial_position = fen;
    return true;
  }

  bool set_default_position()
  {
    initial_position = DEFAULT_POSITION;
    return set_position(DEFAULT_POSITION);
  }

  bool reset_to_initial_position() { return set_position(initial_position); }

  bool try_move(const uci_move_t& move_candidate)
  {
    const std::vector<move_t> legal_moves = generate_legal_moves(&board);

    // Search the move in the list of legal moves
    for (const auto& move : legal_moves) {
      if (move.from == move_candidate.from && move.to == move_candidate.to &&
          move.promoted_to == move_candidate.promotion) {
        // Apply the found move
        bool move_result = make_move(&move, &board, &history);

        if (move_result) { return true; }

        break;
      }
    }

    return false;
  }

  std::string get_nice_board() { return print_nice_board(&board); }

  uci_move_t get_best_move(int depth)
  {
    move_t best_move = search_best_move(depth, &board);

    const uci_move_t result = {best_move.from, best_move.to,
                               best_move.promoted_to};

    return result;
  }
};


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


//-##############################  GLOBAL VARS  #############################-//

static engine_handler_t engine;
static bool is_debug = false;
static bool running = true;

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
    {"print_board", command_print_board},
};
// clang-format on


//-############################  UTILS FUNCTIONS  ###########################-//

std::string trim_whitespace(const std::string& str)
{
  // Find the first non-whitespace character
  auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
  // Find the last non-whitespace character
  auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();

  // If the string is all whitespace, return an empty string
  return (start < end) ? std::string(start, end) : std::string();
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


bool is_command(const std::string& command)
{
  auto it = commands.find(command);
  if (it != commands.end()) { return true; }
  return false;
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
    result + promotion_to_string(move->promotion);
  }

  return result;
}


void uci_reply(const std::string& response)
{
  std::cout << response << std::endl;
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

  engine.set_default_position();

  LOG_I << engine.get_nice_board() << END_I;

  return true;
}


bool command_position(std::queue<std::string>& args)
{
  LOG_I << "Command [position]. Args: " << args << END_I;

  if (args.size() == 0) { return false; }

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "startpos") {
      // Initialize the board to the default starting position
      engine.set_default_position();
      LOG_I << "Set default position" << END_I;
    }

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
      bool res = engine.set_position(fen);

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

          // Attempt the move. We ignore if move happened or not
          bool res = engine.try_move(move_candidate);

          if (res) {
            LOG_I << "Applied move [" << move_str << "]" << END_I;
          } else {
            LOG_W << "Failed move [" << move_str << "]" << END_W;
          }
        }
      }
    }
  }

  LOG_I << engine.get_nice_board() << END_I;

  return true;
}


bool command_go(std::queue<std::string>& args)
{
  LOG_I << "Command [go]. Args: " << args << END_I;

  auto start_time = std::chrono::high_resolution_clock::now();
  const int depth = 6;
  const uci_move_t best_move = engine.get_best_move(depth);

  const auto end_time = std::chrono::high_resolution_clock::now();
  const auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
      end_time - start_time);

  const auto minutes =
      std::chrono::duration_cast<std::chrono::minutes>(duration_ns);
  const auto seconds =
      std::chrono::duration_cast<std::chrono::seconds>(duration_ns - minutes);
  const auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration_ns -
                                                            minutes - seconds);

  const std::string best_move_str = uci_move_to_algebraic(&best_move);

  uci_reply("bestmove " + best_move_str);

  LOG_I << "Best move computed at depth " << depth << " in ["
        << std::to_string(minutes.count()) << " min " << seconds.count()
        << " sec " << milliseconds.count() << " msec]" << END_I;

  return true;
}


bool command_stop(std::queue<std::string>& args)
{
  LOG_I << "Command [stop]. Args: " << args << END_I;

  // TODO

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
  LOG_I << engine.get_nice_board() << END_I;

  return true;
}


//-##################################  MAIN  ################################-//
int main()
{
  LOG_I << "Engine started" << END_I;

  while (running) {
    std::string input;
    std::getline(std::cin, input);

    std::queue<std::string> tokens = tokenize_input(input, " ");

    if (tokens.size() == 0) {
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
