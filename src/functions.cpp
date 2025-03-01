#include "functions.hpp"
#include <cassert>
#include <string>
#include "data_structures.hpp"
#include "utils.hpp"


void test() {}


void init_board(const std::string& fen, board_t* board)
{
  assert(board != nullptr);

  board->board.fill(EMPTY);
  init_game_state(&board->game_state);
  history_t empty_history = history_t();
  board->history.swap(empty_history);
  init_zobrist(&board->zobrist_randoms);
}