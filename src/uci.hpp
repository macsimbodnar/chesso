#pragma once
#include <optional>
#include <queue>
#include <string>
#include <vector>
#include "data_structures.hpp"

// The UCI layer lives in chesso.cpp. main() is kept in its own translation
// unit so that chesso.cpp can be linked into the test binaries without
// colliding with doctest's main.

#define MOVE_OVERHEAD_MS 50
#define DEFAULT_MOVES_TO_GO 20
#define SEARCH_SOFT_LIMIT_PERCENT 60

// Used when [go] carries no limit at all, and when it carries a time control
// that has already run out. Without it the search runs to MAX_DEPTH and the
// engine never answers.
#define FALLBACK_SEARCH_TIME_MS 1000


struct uci_move_t
{
  index_t from;
  index_t to;
  promotion_t promotion;
};


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

  // Timer time used for current search
  int search_time_ms = 0;
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


//-##########################  ENGINE ENTRY POINTS  #########################-//
void uci_init();
void uci_shutdown();
bool uci_is_running();
void uci_process_line(const std::string& input);


//-#############################  UCI INTERNALS  ############################-//
// Exposed so the test binaries can drive them directly. Not part of any
// protocol contract.
std::queue<std::string> tokenize_input(std::string string, std::string delim);
std::string trim_whitespace(const std::string& str);

bool pop_int(std::queue<std::string>& args,
             const char* name,
             int& out,
             int min,
             int max);
bool pop_u64(std::queue<std::string>& args, const char* name, uint64_t& out);

std::optional<uci_move_t> algebraic_to_uci_move(const std::string& p);
std::string uci_move_to_algebraic(const uci_move_t* move);
std::string best_move_to_string(const uci_search_result_t& result);
std::string pv_to_string(const pv_t* pv);

int compute_search_time_ms(int remaining_ms, int increment_ms, int movestogo);

bool set_position(const std::string& fen);
bool check_move_legality(move_t move);
move_t first_legal_move();
move_t validate_book_move(move_t book_move);

uci_search_result_t iterative_deepening_search(
    const uci_search_options_t& conf);

// Read-only access to the UCI layer's board and table, for assertions in
// tests, plus a way to wait for a [go] to answer instead of racing it.
const game_t* uci_game();
const transposition_table_t* uci_tt();
void uci_wait_for_search();
