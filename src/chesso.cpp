#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "board.hpp"
#include "data_structures.hpp"
#include "move_generator.hpp"
#include "utils.hpp"

//-##############################   DATA TYPES  #############################-//

typedef bool (*process_func)(const std::vector<std::string>&);

struct uci_move_t
{
  index_t from;
  index_t to;
  promotion_t promotion;
};


//-###########################  COMMAND DECLARATIONS  #######################-//
bool command_debug(const std::vector<std::string>& args);
bool command_isready(const std::vector<std::string>& args);
bool command_setoption(const std::vector<std::string>& args);
bool command_register(const std::vector<std::string>& args);
bool command_ucinewgame(const std::vector<std::string>& args);
bool command_position(const std::vector<std::string>& args);
bool command_go(const std::vector<std::string>& args);
bool command_stop(const std::vector<std::string>& args);
bool command_ponderhit(const std::vector<std::string>& args);
bool command_quit(const std::vector<std::string>& args);

//-##############################  GLOBAL VARS  #############################-//

board_t board;
static bool is_debug = false;

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


std::vector<std::string> split_str(const std::string string,
                                   const std::string delimiter)
{
  const std::string s = trim_whitespace(string);
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;

    if (token.size() > 0) { res.push_back((token)); }
  }

  res.push_back(s.substr(pos_start));
  return res;
}


bool is_command(const std::string& command)
{
  auto it = commands.find(command);
  if (it != commands.end()) { return true; }
  return false;
}


//-################################  COMMANDS  ##############################-//
bool command_debug(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "debug");

  for (const auto& arg : args) {
    if (arg == "on") {
      is_debug = true;
    } else if (arg == "off") {
      is_debug = false;
    }
  }

  return true;
}


bool command_isready(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "isready");

  std::cout << "readyok" << std::endl;

  return true;
}


bool command_setoption(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "setoption");

  // setoption name <id> [value <x>]

  // TODO

  return true;
}


bool command_register(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "register");

  // TODO

  return true;
}


bool command_ucinewgame(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "ucinewgame");

  // TODO

  return true;
}


bool command_position(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "position");

  for (const auto& arg : args) {
    if (arg == "startpos") { init_board(DEFAULT_POSITION, &board); }
    // TODO
  }


  return true;
}


bool command_go(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "go");

  // TODO

  return true;
}


bool command_stop(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "stop");

  // TODO

  return true;
}


bool command_ponderhit(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "ponderhit");

  // TODO

  return true;
}


bool command_quit(const std::vector<std::string>& args)
{
  assert(args.size() > 0);
  assert(args[0] == "quit");

  // TODO

  return true;
}


//-##################################  MAIN  ################################-//

// int main(int argc, char* argv[])
int main()
{
  init_board(DEFAULT_POSITION, &board);

  // For now we just support UCI
  while (true) {
    std::string input;
    std::getline(std::cin, input);

    if (input == "uci") { break; }
  }

  bool running = true;

  while (running) {
    std::string input;
    std::getline(std::cin, input);

    const auto& tokens = split_str(input, " ");

    if (tokens.size() == 0) { continue; }

    // Try to find the command
    for (size_t i = 0; i < tokens.size(); ++i) {
      const auto& current_token = tokens[i];

      if (is_command(current_token)) {
        const std::vector<std::string> remaining_tokens = {tokens.begin() + i,
                                                           tokens.end()};

        const bool result = commands.at(current_token)(remaining_tokens);

        if (!result) { return 1; }

        // We found and executed the command for this input. So jump to next
        break;
      }
    }
  }

  return 0;
}