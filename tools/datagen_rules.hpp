#pragma once
// What ends a datagen self-play game by the rules. S331 moved it out of
// tools/datagen.cpp's anonymous namespace so tests/test_datagen.cpp drives the
// function the games are played by, and not a copy of it.
#include "bitboard.hpp"
#include "data_structures.hpp"


inline bool has_legal_move(game_t* game)
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (make_move(game, moves[i])) {
      unmake_move(game);
      return true;
    }
  }

  return false;
}


// 1.0, 0.5 or 0.0 from White's point of view, or -1 when the game is not over.
//
// A repetition ends the game on the third occurrence of the position on the
// board, the one FIDE 9.2 lets a player claim and the match harness ends a
// game on. It is classify_repetition()'s answer with the history's own size as
// the root, so every earlier occurrence lies at or before the root and DRAW
// means a second earlier one: the search's rule for a position from before its
// root since S207 (DEC-173). Until S331 this test was is_position_repeated(),
// a two-fold anywhere, which the search had stopped reading as a draw, so the
// engine repeated a position once when it saw no reason not to and the game
// was scored a draw the laws do not give.
//
// `recurred`, when given, says whether the position on the board occurred
// before in the game: where the rule before S331 ended the game as a draw. It
// stays false when the fifty-move rule or insufficient material decides
// first, as they decided before the repetition test then too.
inline double terminal_result(game_t* game, bool* recurred = nullptr)
{
  if (recurred != nullptr) { *recurred = false; }
  if (game->board.halfmove_clock >= 100) { return 0.5; }
  if (is_insufficient_material(&game->board)) { return 0.5; }

  const repetition_kind_t repetition =
      classify_repetition(&game->history, &game->board, game->history.size);

  if (recurred != nullptr) {
    *recurred = (repetition != repetition_kind_t::NONE);
  }

  if (repetition == repetition_kind_t::DRAW) { return 0.5; }

  if (!has_legal_move(game)) {
    // Stalemate is a draw; mate is a loss for the side that has to move.
    if (!is_check(game)) { return 0.5; }
    return (game->board.active_color == WHITE) ? 0.0 : 1.0;
  }

  return -1.0;
}
