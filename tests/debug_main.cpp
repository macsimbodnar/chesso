#include "bitboard.cpp"
#include "data_structures.hpp"
#include "log.hpp"
#include "utils.hpp"


static bb_tables_t bb_data;
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
  LOG_I << print_nice_board(&board) << END_I;

  bb_t occupancy = BB_0;
  bb_t q_attacks = get_rook_attacks(&bb_data, d4, occupancy);
  
  LOG_I << print_bboard(q_attacks) << END_I;

  return 0;
}
