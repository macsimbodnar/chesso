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

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // LOG_I << "Debug" << END_I;
  // LOG_S << "Debug" << END_S;
  // LOG_W << "Debug" << END_W;
  // LOG_E << "Debug" << END_E;

  board_t board;
  history_t history;
  init_board("r2q1r1k/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R w KQ - 1 2",
             &board, &history);

  move_t moves[MAX_MOVES];
  size_t count = generate_legal_moves(&board, moves);

  for (size_t i = 0; i < count; ++i) {
    const move_t& move = moves[i];
    int score = evaluate_move(&move);
    LOG_I << move << "    " << score << std::endl;
  }

  return 0;
}