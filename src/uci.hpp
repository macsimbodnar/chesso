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

  // 0 means the GUI sent none, which is a sudden-death control. It is not a
  // stand-in for some number of moves: there is no boundary to divide by and
  // inventing one is what S089 removed.
  int movestogo = 0;

  // The two limits for this search, in the shape S089 gave them.
  //
  // search_time_ms is the hard one. A timer is armed at it and it stops the
  // search inside an iteration; it never exceeds what the clock has.
  // search_soft_time_ms is the unscaled soft one, which decides whether to
  // begin another iteration. 0 means "same as the hard limit".
  int search_time_ms = 0;
  int search_soft_time_ms = 0;

  // Whether the soft limit may be moved by what the search finds. Set on the
  // clock path. Clear for [go movetime], where the GUI named the time and
  // scaling it would be disobeying the command, and for the no-limit fallback.
  bool scale_time = false;
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

// Every command uci_process_line() dispatches, sorted. [help] prints this and
// the surface test compares it against MANUAL.md, so a command that is added
// or renamed cannot reach a GUI undocumented. S017, DEC-024.
std::vector<std::string> uci_command_names();


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

// The two limits a clock is turned into. S089.
struct search_time_budget_t
{
  int soft_ms;  // begin another iteration only below this
  int hard_ms;  // the armed timer; never above what the clock has
};

// movestogo of 0 is a sudden-death control and is not treated as a move count.
search_time_budget_t compute_search_time_budget(int remaining_ms,
                                                int increment_ms,
                                                int movestogo);

// The percentage the soft limit is scaled by after an iteration completes.
// Pure, so the two factors can be held against their own arithmetic; what
// proves the search uses it is uci_last_time_scale_percent() below.
int search_time_scale_percent(int best_move_stability, int score_drop_cp);

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

// Aspiration window failures in the last iterative_deepening_search(). Test
// instrumentation, and the precondition a mate case driven through that
// function needs: without it the case cannot tell a narrow window that held
// from a window schedule that never engaged. S021.
int uci_last_aspiration_failures();

// The time manager's state after the last completed iteration of the last
// iterative_deepening_search(). Test instrumentation in the same shape and for
// the same reason: search_time_scale_percent() is pure and could be right and
// never called, so the tests need the numbers the loop actually fed it and the
// scale it actually used. S089.
int uci_last_best_move_stability();
int uci_last_score_drop_cp();
int uci_last_time_scale_percent();
