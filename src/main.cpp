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

  // u64 a = chesso::generate_mask_king_attacks(chesso::A4);
  // chesso::print_board(a);


  return EXIT_SUCCESS;
}