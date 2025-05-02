#include <iostream>
#include "board.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "exceptions.hpp"
#include "move_generator.hpp"
#include "search.hpp"
#include "utils.hpp"


#define LOG_I std::cout  // Start log
#define END_I std::endl  // End log

#define LOG_S LOG_I << "\033[92m"  // Success green log
#define END_S "\033[37m" << END_I  // End success green log

#define LOG_W LOG_I << "\033[33m"  // Warning orange log
#define END_W "\033[37m" << END_I  // End warning orange log

#define LOG_E LOG_I << "\033[31m"  // Error red log
#define END_E "\033[37m" << END_I  // End Error red log

static board_t board = {};
static global_state_t globals = {};
static transposition_table_t tt = {};

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  LOG_I << "Debug" << END_I;
  LOG_S << "Debug" << END_S;
  LOG_W << "Debug" << END_W;
  LOG_E << "Debug" << END_E;

  init_board(KILLER_POS, &board, &globals);

  std::atomic_bool stop_search_signal = false;
  search_state_t state = {};
  state.stop = &stop_search_signal;
  state.tt = &tt;

  const search_t search_result = search_best_move(6, &board, &globals, &state);

  LOG_I << search_result.best_move << " " << search_result.score << std::endl;

  return 0;
}
