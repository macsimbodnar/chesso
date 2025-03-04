#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cassert>
#include <iostream>
#include <string>
#include "doctest.h"
#include "functions.hpp"
#include "move_generator.hpp"
#include "utils.hpp"


bool vector_contains(index_t i, std::vector<index_t>& v)
{
  for (const auto I : v) {
    if (I == i) { return true; }
  }

  return false;
}


TEST_SUITE("Test utils")
{
  TEST_CASE("Test FEN")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    std::string fen_result = generate_FEN(&board);

    REQUIRE_EQ(fen_result, std::string(DEFAULT_POSITION));
  }
}


TEST_SUITE("Test move_generator")
{
  TEST_CASE("Test pseudo legal black pawn")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(0, 6);
    auto moves = generate_pseudo_legal_moves_for_piece(index, &board);

    REQUIRE_EQ(moves.size(), 2);

    index_t pos_1 = position_to_index(0, 5);
    REQUIRE(vector_contains(pos_1, moves));

    index_t pos_2 = position_to_index(0, 4);
    REQUIRE(vector_contains(pos_2, moves));
  }
}

// int main()
// {
//   board_t board;
//   init_board(DEFAULT_POSITION, &board);

//   std::string starting_board = print_nice_board(&board);
//   std::cout << print_board(&board) << "\n";
//   std::cout << print_nice_board(&board) << "\n";

//   std::string current_fan = generate_FEN(&board);

//   assert(current_fan == DEFAULT_POSITION);

//   assert(WHITE == us(&board));

//   reset(&board);
//   assert(starting_board == print_nice_board(&board));

//   assert(us(&board) == WHITE);
//   assert(opponent(&board) == BLACK);
//   auto chess_board = get_chess_board(&board);

//   assert(king_square(WHITE, &board) == position_t(4, 0));
//   assert(king_square(BLACK, &board) == position_t(4, 7));

//   assert(get_square_color(0) == BLACK);
//   assert(get_square_color(position_to_index(2, 2)) == BLACK);
//   assert(get_square_color(position_to_index(2, 5)) == WHITE);

//   assert(has_bishop_pair(BLACK, &board));
//   assert(has_bishop_pair(WHITE, &board));


//   return 0;
// }
