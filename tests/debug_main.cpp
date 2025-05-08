#include "bitboard.cpp"
#include "data_structures.hpp"
#include "log.hpp"
#include "utils.hpp"


static bb_const_data_t bb_data;
static board_t board;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  LOG_I << "Debug" << END_I;
  LOG_S << "Debug" << END_S;
  LOG_W << "Debug" << END_W;
  LOG_E << "Debug" << END_E;

  initialize_const_data(&bb_data);

  assert(load_FEN(DEFAULT_POSITION, &board));

  // SET_BIT(board.bitboards[W_KING], a1);

  LOG_I << print_nice_board(&board) << END_I;


  // LOG_I << piece_to_str(get_piece(&board, e1)) << END_I;

  LOG_I << print_bboard(board.bitboards[B_ROOK]) << END_I;



  return 0;
}
