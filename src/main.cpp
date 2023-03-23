#include "board.hpp"
#include "chesso.hpp"
#include "gui.hpp"

int main(int, char**)
{
  // int screen_h = 600;
  // int screen_w = 1100;
  // gui_t gui(screen_w, screen_h);

  // if (!gui.run()) { return EXIT_FAILURE; }


  chesso::init_attack_vectors();

  u64 blocks = EMPTY_BB;

  set_bit(blocks, chesso::F4);
  set_bit(blocks, chesso::D2);
  set_bit(blocks, chesso::D5);
  set_bit(blocks, chesso::A4);

  chesso::print_board(blocks);


  u64 a = chesso::rook_attacks(chesso::D4, blocks);
  chesso::print_board(a);


  return EXIT_SUCCESS;
}