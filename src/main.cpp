#include "board.hpp"
#include "chesso.hpp"
#include "gui.hpp"
#include "log.hpp"


int main(int, char**)
{
  // int screen_h = 600;
  // int screen_w = 1100;
  // gui_t gui(screen_w, screen_h);

  // if (!gui.run()) { return EXIT_FAILURE; }


  chesso::init_attack_vectors();

  u64 attack_mask = chesso::generate_mask_bishop_attacks(chesso::D4);

  for (int i = 0; i < 4095; ++i) {
    u64 o = chesso::set_occupancy(i, count_bits(attack_mask), attack_mask);

    chesso::print_board(o);

    // getchar();
  }

  return EXIT_SUCCESS;
}