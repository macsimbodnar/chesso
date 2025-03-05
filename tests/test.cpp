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


bool contain_move(const move_t& move, const std::vector<move_t>& moves)
{
  for (const auto& I : moves) {
    if (I == move) { return true; }
  }

  return false;
}


std::string moves_to_string(const std::vector<move_t>& moves,
                            const board_t& board)
{
  std::string result;

  for (const auto& move : moves) {
    result += move_to_algebraic(&move, &board);
    result += "\n";
  }

  return result;
}


bool contain_move_algebraic(const std::string& move,
                            std::vector<move_t>& moves,
                            const board_t& board)
{
  for (const auto& I : moves) {
    if (move == move_to_algebraic(&I, &board)) { return true; }
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
    piece_t piece = board.board[index];
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);

    move_t pos_1 = {index, position_to_index(0, 5), piece};
    REQUIRE(contain_move(pos_1, moves));

    move_t pos_2 = {index, position_to_index(0, 4), piece};
    pos_2.double_pawn_move = true;
    REQUIRE(contain_move(pos_2, moves));
  }


  TEST_CASE("Test pseudo legal white pawn")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    index_t index = position_to_index(0, 1);
    piece_t piece = board.board[index];
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);

    move_t pos_1 = {index, position_to_index(0, 2), piece};
    REQUIRE(contain_move(pos_1, moves));

    move_t pos_2 = {index, position_to_index(0, 3), piece};
    pos_2.double_pawn_move = true;
    REQUIRE(contain_move(pos_2, moves));
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
    piece_t piece = board.board[index];
    auto moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(contain_move(move_t(index, position_to_index(0, 2), piece), moves));
    REQUIRE(contain_move(move_t(index, position_to_index(2, 2), piece), moves));

    index = position_to_index(6, 0);
    piece = board.board[index];
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(contain_move(move_t(index, position_to_index(5, 2), piece), moves));
    REQUIRE(contain_move(move_t(index, position_to_index(7, 2), piece), moves));

    index = position_to_index(1, 7);
    piece = board.board[index];
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(contain_move(move_t(index, position_to_index(0, 5), piece), moves));
    REQUIRE(contain_move(move_t(index, position_to_index(2, 5), piece), moves));

    index = position_to_index(6, 7);
    piece = board.board[index];
    moves = generate_pseudo_legal_moves_from_index(index, &board);

    REQUIRE_EQ(moves.size(), 2);
    REQUIRE(contain_move(move_t(index, position_to_index(7, 5), piece), moves));
    REQUIRE(contain_move(move_t(index, position_to_index(5, 5), piece), moves));
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
  // clang-format off
  const static std::vector<std::string> test_files = {
      // "assets/castling.json",
      // "assets/checkmates.json",
      // "assets/famous.json",
      // "assets/pawns.json",
      // "assets/promotions.json",
      // "assets/stalemates.json",
      "assets/standard.json",
      // "assets/taxing.json",
  };
  // clang-format on


  TEST_CASE("Basic test")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    auto moves = generate_legal_moves(&board);
    REQUIRE_EQ(moves.size(), 20);
  }

  TEST_CASE("Test against generated jsons")
  {
    for (const auto& test_json_file : test_files) {
      json test_cases = load_json(test_json_file);

      for (const json& test_case : test_cases["testCases"]) {
        std::string starting_pos = test_case["start"]["fen"];

        board_t board;
        init_board(starting_pos, &board);

        auto moves = generate_legal_moves(&board);

        // Check size
        REQUIRE_MESSAGE(moves.size() == test_case["expected"].size(),
                        ("Running " + test_json_file + " File"));

        // Check if move is in by Algebraic notation
        for (const json& expected : test_case["expected"]) {
          const std::string move = expected["move"];
          // std::string fen = expected["fen"];

          bool found = contain_move_algebraic(move, moves, board);

          REQUIRE_MESSAGE(found, ("\nStarting FEN: " + starting_pos +
                                  "\nExpect move: " + move + " in:\n" +
                                  moves_to_string(moves, board) +
                                  print_nice_board(&board)));
        }
      }
    }
  }
}
