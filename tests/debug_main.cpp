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

  position_t pos = index_to_position(a1);
  pos = index_to_position(a2);
  pos = index_to_position(a3);
  pos = index_to_position(a4);
  pos = index_to_position(a5);
  pos = index_to_position(a6);
  pos = index_to_position(a7);
  pos = index_to_position(a8);

  return 0;
}
