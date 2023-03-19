#include "gui.hpp"

int main(int, char**)
{
  int screen_h = 600;
  int screen_w = 1100;
  gui_t gui(screen_w, screen_h);

  if (!gui.run()) { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}