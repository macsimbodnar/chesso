#include "board.hpp"
#include "chesso.hpp"
#include "log.hpp"


using namespace chesso;

int main(int, char**)
{
  init_attack_vectors();

  u64 occupancy = EMPTY_BB;
  set_bit(occupancy, C5);
  set_bit(occupancy, F4);
  print_board(occupancy);

  const u64 attack = get_bishop_attacks(D4, occupancy);

  print_board(attack);

  const u64 a = get_rook_attacks(D4, occupancy);
  print_board(a);

  return EXIT_SUCCESS;
}