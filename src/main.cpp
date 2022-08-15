#include "gui.hpp"

int main(int argc, char** argv)
{
  gui p;

  if (!p.run()) { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}