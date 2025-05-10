#include "bitboard.cpp"
#include "data_structures.hpp"
#include "log.hpp"
#include "utils.hpp"


static bb_tables_t bb_data;
static board_t board;
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

  initialize_const_data(&bb_data);
  load_FEN(DEFAULT_POSITION, &board);

  LOG_I << print_nice_board(&board) << END_I;

  move_count = generate_moves(&bb_data, &board, moves);

  for (size_t i = 0; i < move_count; ++i) {
    LOG_I << print_move(moves[i]) << END_I;
  }


  return 0;
}
