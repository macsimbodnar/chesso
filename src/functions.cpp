#include "functions.hpp"
#include <cassert>

#include <map>

#include <string>
#include <vector>
#include "data_structures.hpp"
#include "utils.hpp"

void test() {}


void init_board(const std::string& fen, board_t* board)
{
  assert(board != nullptr);

  // Cleanup
  board->board.fill(EMPTY);
  history_t empty_history = history_t();
  board->history.swap(empty_history);
  cleanup_game_state(&board->game_state);

  // Init random numbers
  init_zobrist(&board->zobrist_randoms);

  // Load FEN
  load_FEN(fen, board);

  // Init Zobrist
  board->game_state.zobrist_key = init_zobrist_key(board);

  // TODO: Init phase_value
}
