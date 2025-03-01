#include "chesso_board.hpp"
#include <iostream>
#include <string>
#include <cassert>

int main()
{
  board_t board;
  board.load_FEN(DEFAULT_POSITION);
  std::cout << board.print_board() << std::endl;
  std::cout << board.print_nice_board() << std::endl;

  assert(board.generate_FEN() == DEFAULT_POSITION);
  return 0;
}