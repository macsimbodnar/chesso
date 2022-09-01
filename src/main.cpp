#include "gui.hpp"

int main(int argc, char** argv)
{
  gui_t gui;

  if (!gui.run()) { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}