#include "gui.hpp"

int main(int, char**)
{
  gui_t gui;

  if (!gui.run()) { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}