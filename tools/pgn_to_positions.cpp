// Reads SAN moves on stdin (whitespace separated) and prints, one per line:
//   ply  san  lan  fen_before  phase
//
// `phase` is the engine's own game_phase() for the position before the move,
// GAME_PHASE_MAX at a full board down to 0 with no pieces left but pawns and
// kings. It is emitted here rather than recomputed by the caller so that a
// finding bucketed by phase names the same quantity the tapered evaluation
// actually tapers on. S018.
//
// The column was appended, not inserted: a reader that splits on tabs and takes
// the first four fields still works.
#include <cstdio>
#include <iostream>
#include <string>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "utils.hpp"

static game_t game;

int main()
{
  initialize_game_const_data(&game);
  if (!load_FEN(DEFAULT_POSITION, &game)) { return 1; }

  std::string token;
  int ply = 0;

  while (std::cin >> token) {
    const std::string fen_before = generate_FEN(&game.board);
    const int phase = game_phase(&game.board);
    const move_t move = algebraic_to_move(token, &game);

    if (move == 0) {
      fprintf(stderr, "could not parse '%s' at ply %d\n", token.c_str(), ply);
      return 1;
    }

    const std::string lan =
        index_to_str(MOVE_FROM(move)) + index_to_str(MOVE_TO(move));

    printf("%d\t%s\t%s\t%s\t%d\n", ply, token.c_str(), lan.c_str(),
           fen_before.c_str(), phase);

    if (!make_move(&game, move)) {
      fprintf(stderr, "illegal '%s' at ply %d\n", token.c_str(), ply);
      return 1;
    }
    ply++;
  }

  // The position the game ended in, so the last move can be scored too.
  printf("%d\t-\t-\t%s\t%d\n", ply, generate_FEN(&game.board).c_str(),
         game_phase(&game.board));
  return 0;
}
