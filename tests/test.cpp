#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include "doctest.h"
#include "functions.hpp"
#include "move_generator.hpp"
#include "nlohmann_json.hpp"
#include "utils.hpp"

using json = nlohmann::json;

json load_json(const std::string& filename)
{
  std::ifstream file(filename);
  REQUIRE(file);

  json json_data;
  try {
    file >> json_data;
  } catch (const json::parse_error& e) {
    REQUIRE(false);
  }

  file.close();

  return json_data;
}


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


TEST_SUITE("Test pseudo legal move generator")
{
  TEST_CASE("Test pseudo legal black pawn")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(0, 6);
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);

    index_t pos_1 = position_to_index(0, 5);
    REQUIRE(vector_contains(pos_1, moves));

    index_t pos_2 = position_to_index(0, 4);
    REQUIRE(vector_contains(pos_2, moves));
  }


  TEST_CASE("Test pseudo legal white pawn")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(0, 1);
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);

    index_t pos_1 = position_to_index(0, 2);
    REQUIRE(vector_contains(pos_1, moves));

    index_t pos_2 = position_to_index(0, 3);
    REQUIRE(vector_contains(pos_2, moves));
  }


  TEST_CASE("Test pseudo legal rooks")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(7, 7);
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(0, 7);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(0, 0);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(7, 0);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);
  }


  TEST_CASE("Test pseudo legal bishops")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(2, 0);
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(5, 0);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(2, 7);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(5, 7);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);
  }


  TEST_CASE("Test pseudo legal knight")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(1, 0);
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(vector_contains(position_to_index(0, 2), moves));
    REQUIRE(vector_contains(position_to_index(2, 2), moves));

    index = position_to_index(6, 0);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(vector_contains(position_to_index(5, 2), moves));
    REQUIRE(vector_contains(position_to_index(7, 2), moves));

    index = position_to_index(1, 7);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(vector_contains(position_to_index(0, 5), moves));
    REQUIRE(vector_contains(position_to_index(2, 5), moves));

    index = position_to_index(6, 7);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(vector_contains(position_to_index(7, 5), moves));
    REQUIRE(vector_contains(position_to_index(5, 5), moves));
  }


  TEST_CASE("Test pseudo legal queen")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(3, 0);
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(3, 7);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);
  }


  TEST_CASE("Test pseudo legal king")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(4, 0);
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);

    index = position_to_index(4, 7);
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 0);
  }
}

TEST_SUITE("Test legal move generator")
{
  TEST_CASE("Basic test")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    auto moves = generate_legal_moves(&board);
    REQUIRE_EQ(moves.size(), 20);
  }

  TEST_CASE("Test standard.json")
  {
    json test_cases = load_json("assets/standard.json");

    for (const json& test_case : test_cases["testCases"]) {
      std::string starting_pos = test_case["start"]["fen"];

      board_t board;
      init_board(starting_pos, &board);

      auto moves = generate_legal_moves(&board);

      REQUIRE_EQ(moves.size(), test_case["expected"].size());

      // for (const json& expected : test_case["expected"]) {
      //   std::string move = expected["move"];
      //   std::string fen = expected["fen"];
      // }
    }
  }
}
