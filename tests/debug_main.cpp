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

  LOG_I << "Debug" << END_I;
  LOG_S << "Debug" << END_S;
  LOG_W << "Debug" << END_W;
  LOG_E << "Debug" << END_E;

  initialize_const_data(&game.tables);
  load_FEN(DEFAULT_POSITION, &game.board, &game.history);

  LOG_I << print_nice_board(&game.board) << END_I;

  move_count = generate_moves(&game.tables, &game.board, moves);

  for (size_t i = 0; i < move_count; ++i) {
    LOG_I << print_move(moves[i]) << END_I;
  }

  const bool happened = make_move(&game, moves[0]);

  assert(happened);

  LOG_I << print_nice_board(&game.board) << END_I;
  return 0;
}
