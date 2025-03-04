#include <cassert>
#include <iostream>
#include <string>
#include "functions.hpp"
#include "utils.hpp"


int main()
{
  board_t board;
  init_board(DEFAULT_POSITION, &board);

  std::string starting_board = print_nice_board(&board);
  std::cout << print_board(&board) << "\n";
  std::cout << print_nice_board(&board) << "\n";

  std::string current_fan = generate_FEN(&board);

  assert(current_fan == DEFAULT_POSITION);

  assert(WHITE == us(&board));

  reset(&board);
  assert(starting_board == print_nice_board(&board));

  assert(us(&board) == WHITE);
  assert(opponent(&board) == BLACK);
  auto chess_board = get_chess_board(&board);

  assert(king_square(WHITE, &board) == position_t(4, 0));
  assert(king_square(BLACK, &board) == position_t(4, 7));

  assert(get_square_color(0) == BLACK);
  assert(get_square_color(position_to_index(2, 2)) == BLACK);
  assert(get_square_color(position_to_index(2, 5)) == WHITE);

  assert(has_bishop_pair(BLACK, &board));
  assert(has_bishop_pair(WHITE, &board));


  return 0;
}
