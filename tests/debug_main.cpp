#include "bitboard.cpp"
#include "data_structures.hpp"
#include "log.hpp"
#include "utils.hpp"


static game_t game;
static move_t moves[MAX_MOVES];
static size_t move_count = 0;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  (void)move_count;
  (void)moves;
  (void)game;

  // LOG_I << "Debug" << END_I;
  // LOG_S << "Debug" << END_S;
  // LOG_W << "Debug" << END_W;
  // LOG_E << "Debug" << END_E;

  initialize_game_const_data(&game);
  load_FEN("1q1rkbnr/1b1n1pp1/2pp3p/1p1Pp3/1P2P1P1/2N1B1NP/2P1QPB1/R4RK1 b k - 0 15", &game);

  move_count = generate_moves(&game.tables, &game.board, moves);

  for (size_t i = 0; i < move_count; ++i) {
    LOG_I << print_move(moves[i]) << END_I;
  }

  return 0;
}
