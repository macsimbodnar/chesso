// Reads SAN moves on stdin (whitespace separated) and prints, one per line:
//   ply  san  lan  fen_before
#include <cstdio>
#include <iostream>
#include <string>
#include "bitboard.hpp"
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
    const move_t move = algebraic_to_move(token, &game);

    if (move == 0) {
      fprintf(stderr, "could not parse '%s' at ply %d\n", token.c_str(), ply);
      return 1;
    }

    const std::string lan =
        index_to_str(MOVE_FROM(move)) + index_to_str(MOVE_TO(move));

    printf("%d\t%s\t%s\t%s\n", ply, token.c_str(), lan.c_str(),
           fen_before.c_str());

    if (!make_move(&game, move)) {
      fprintf(stderr, "illegal '%s' at ply %d\n", token.c_str(), ply);
      return 1;
    }
    ply++;
  }

  // The position the game ended in, so the last move can be scored too.
  printf("%d\t-\t-\t%s\n", ply, generate_FEN(&game.board).c_str());
  return 0;
}
