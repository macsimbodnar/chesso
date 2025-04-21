#include "experimental_search.hpp"
#include <cassert>
#include "board.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "move_generator.hpp"


int evaluate_move(const move_t* move, const board_t* board)
{
  assert(move != nullptr);
  assert(board != nullptr);

  board_t tmp_board = *board;
  const color_t color = board->game_state.active_color;

  (void)make_move(move, &tmp_board, nullptr);
  const int score = evaluate(&tmp_board);

  return (color == WHITE) ? score : -score;
}


void experimental_order_moves(move_t moves[],
                              size_t moves_size,
                              const board_t* board)
{
  assert(moves != nullptr);
  assert(moves_size <= MAX_MOVES);
  assert(board != nullptr);

  int scores[MAX_MOVES];
  for (size_t i = 0; i < moves_size; ++i) {
    scores[i] = evaluate_move(&moves[i], board);
  }

  // Insertion sort
  for (size_t i = 1; i < moves_size; ++i) {
    const int value = scores[i];
    const move_t move = moves[i];
    int j = i;

    while (j != 0 && scores[j - 1] < value) {
      scores[j] = scores[j - 1];
      moves[j] = moves[j - 1];
      --j;
    }

    scores[j] = value;
    moves[j] = move;
  }
}
