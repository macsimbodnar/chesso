#include <iostream>
#include "board.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "exceptions.hpp"
#include "move_generator.hpp"
#include "search.hpp"
#include "utils.hpp"

// clang-format off
// CMK Positions
#define EMPTY_POS "8/8/8/8/8/8/8/8 b - -"
#define TRICKY_POS "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define KILLER_POS "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define CMK_POS "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"
// clang-format on

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
  init_board(KILLER_POS, &board, &history);

  search_state_t state = {};

  const search_t search_result = search_best_move(6, &board, &state);

  LOG_I << search_result.best_move << " " << search_result.score << std::endl;

  return 0;
}