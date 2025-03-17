#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>
#include "board.hpp"
#include "data_structures.hpp"
#include "move_generator.hpp"
#include "utils.hpp"


//-##############################   DATA TYPES  #############################-//

typedef bool (*process_func)(std::queue<std::string>&);

struct uci_move_t
{
  index_t from;
  index_t to;
  promotion_t promotion;
};


class engine_handler_t
{
private:
  board_t board;
  std::string initial_position = DEFAULT_POSITION;

public:
  engine_handler_t() { set_default_position(); }

  bool set_position(const std::string& fen)
  {
    init_board(fen, &board);
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
        bool move_result = make_move(&move, &board);

        if (move_result) { return true; }

        break;
      }
    }

    return false;
  }
};


//-###########################  COMMAND DECLARATIONS  #######################-//
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

//-##############################  GLOBAL VARS  #############################-//

static engine_handler_t engine;
static bool is_debug = false;
static bool running = true;

static const std::unordered_map<std::string, process_func> commands = {
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

};


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


//-################################  COMMANDS  ##############################-//
bool command_debug(std::queue<std::string>& args)
{
  if (args.size() == 0) { return false; }

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "on") {
      is_debug = true;
    } else if (token == "off") {
      is_debug = false;
    }
  }

  return true;
}


bool command_isready(std::queue<std::string>& args)
{
  (void)args;
  std::cout << "readyok" << std::endl;

  return true;
}


bool command_setoption(std::queue<std::string>& args)
{
  if (args.size() == 0) { return false; }

  // setoption name <id> [value <x>]

  // TODO

  return true;
}


bool command_register(std::queue<std::string>& args)
{
  if (args.size() == 0) { return false; }

  // TODO

  return true;
}


bool command_ucinewgame(std::queue<std::string>& args)
{
  (void)args;
  // TODO

  return true;
}


bool command_position(std::queue<std::string>& args)
{
  if (args.size() == 0) { return false; }

  while (!args.empty()) {
    const std::string token = args.front();
    args.pop();

    if (token == "startpos") {
      // Initialize the board to the default starting position
      engine.set_default_position();

      // std::cout << print_nice_board(&board) << std::endl;
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
      engine.set_position(fen);

      // std::cout << print_nice_board(&board) << std::endl;
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
          (void)engine.try_move(move_candidate);
        }
      }
    }
  }

  return true;
}


bool command_go(std::queue<std::string>& args)
{
  if (args.size() == 0) { return false; }

  // TODO

  return true;
}


bool command_stop(std::queue<std::string>& args)
{
  (void)args;
  // TODO

  return true;
}


bool command_ponderhit(std::queue<std::string>& args)
{
  (void)args;

  // TODO

  return true;
}


bool command_quit(std::queue<std::string>& args)
{
  (void)args;
  running = false;
  return true;
}


//-##################################  MAIN  ################################-//
int main()
{
  // For now we just support UCI
  while (true) {
    std::string input;
    std::getline(std::cin, input);

    if (input == "uci") { break; }
  }

  while (running) {
    std::string input;
    std::getline(std::cin, input);

    std::queue<std::string> tokens = tokenize_input(input, " ");

    if (tokens.size() == 0) { continue; }

    // Try to find the command
    while (!tokens.empty()) {
      const std::string current_token = tokens.front();
      tokens.pop();

      if (is_command(current_token)) {
        const bool result = commands.at(current_token)(tokens);

        if (!result) { return 1; }

        // We found and executed the command for this input. So jump to next
        break;
      }
    }
  }

  return 0;
}
