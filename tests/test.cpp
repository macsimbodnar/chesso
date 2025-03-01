#include <cassert>
#include <iostream>
#include <string>
#include "functions.hpp"
#include "utils.hpp"


int main()
{
  board_t board;
  init_board(DEFAULT_POSITION, &board);

  std::cout << print_board(&board) << "\n";
  std::cout << print_nice_board(&board) << "\n";

  return 0;
}
