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

  // LOG_I << "Debug" << END_I;
  // LOG_S << "Debug" << END_S;
  // LOG_W << "Debug" << END_W;
  // LOG_E << "Debug" << END_E;

  initialize_const_data(&game.tables);
  load_FEN("8/4k3/8/8/8/8/r6r/R3K2R w KQ - 0 1", &game.board, &game.history);
  // load_FEN("8/4K3/8/8/8/8/R6R/r3k2r b KQ - 0 1", &game.board, &game.history);

  // debug‐print the three pieces of data that get_rook_attacks uses
  LOG_I << "rook mask [a1]:\n"
        << print_bboard(game.tables.rook_masks[a1]) << END_I;

  LOG_I << "rook_relevant_bits_count[a1] = " << rook_relevant_bits_count[a1]
        << END_I;

  LOG_I << "rook_magic_numbers[a1] = 0x" << std::hex << rook_magic_numbers[a1]
        << std::dec << END_I;

  LOG_I << "---------------------" << END_I;


  bb_t occ = game.board.occupancies[BOTH];
  LOG_I << print_bboard(occ) << END_I;

  bb_t attacks = get_rook_attacks(&game.tables, a1, occ);
  LOG_I << print_bboard(attacks) << END_I;


  move_count = generate_moves(&game.tables, &game.board, moves);

  for (size_t i = 0; i < move_count; ++i) {
    LOG_I << print_move(moves[i]) << END_I;
  }

  return 0;
}
