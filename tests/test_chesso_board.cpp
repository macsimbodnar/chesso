#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <string>
#include "board.hpp"
#include "move_generator.hpp"
#include "utils.hpp"

using json = nlohmann::json;

// clang-format off
const static std::vector<std::string> test_files = {
  "assets/test_jsons/castling.json",
  "assets/test_jsons/checkmates.json",
  "assets/test_jsons/famous.json",
  "assets/test_jsons/pawns.json",
  "assets/test_jsons/promotions.json",
  "assets/test_jsons/stalemates.json",
  "assets/test_jsons/standard.json",
  "assets/test_jsons/taxing.json",
};
// clang-format on


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
    result += index_to_string_coordinates(move.from) + " -> " +
              index_to_string_coordinates(move.to);
    result += "    " + move_to_algebraic(&move, &moves, &board);
    result += "\n";
  }

  return result;
}


bool contain_move_algebraic(const std::string& move,
                            std::vector<move_t>& moves,
                            const board_t& board)
{
  for (const auto& I : moves) {
    if (move == move_to_algebraic(&I, &moves, &board)) { return true; }
  }

  return false;
}


std::string difference_to_string(const json& expected_moves,
                                 const std::vector<move_t>& generated_moves,
                                 const board_t& board)
{
  std::string result = "";

  std::vector<std::string> missing;
  std::vector<std::string> extra;

  // Search for missing
  for (const json& expected : expected_moves) {
    bool found = false;

    for (const move_t& move : generated_moves) {
      std::string move_str = move_to_algebraic(&move, &generated_moves, &board);

      if (move_str == expected["move"]) {
        found = true;
        break;
      }
    }

    if (!found) { missing.push_back(expected["move"]); }
  }

  // Search for extra
  for (const move_t& move : generated_moves) {
    bool found = false;

    std::string move_str = move_to_algebraic(&move, &generated_moves, &board);

    for (const json& expected : expected_moves) {
      if (move_str == expected["move"]) {
        found = true;
        break;
      }
    }

    if (!found) { extra.push_back(move_str); }
  }


  // Compose output
  result += "  Missing:\n";
  for (const auto& I : missing) {
    result += "    " + I + "\n";
  }

  result += "\n  Extra:\n";
  for (const auto& I : extra) {
    result += "    " + I + "\n";
  }

  return result;
}


TEST_SUITE("DEBUG TEST")
{
  // TEST_CASE("DEBUG") {
  //   std::string FEN = "8/4k3/8/8/8/8/r6r/R3K2R w KQ - 0 1";
  //   size_t expected_size = 8;

  //   board_t board;
  //   init_board(FEN, &board);

  //   auto moves = generate_legal_moves(&board);

  //   REQUIRE_EQ(moves.size(), expected_size);
  // }

  // TEST_CASE("test is blocking rays") {
  //   index_t from = 0x25;
  //   index_t to = 0x07;
  //   index_t point = 0x16;

  //   move_t move = {from, to, INVALID};
  //   bool result = is_blocking_ray(point, &move);

  //   REQUIRE_EQ(result, true);
  // }
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

  TEST_CASE("Test fen parsing - generation")
  {
    for (const auto& test_file : test_files) {
      const json test_cases = load_json(test_file);

      for (const json& test_case : test_cases["testCases"]) {
        {
          const std::string expected_FEN = test_case["start"]["fen"];
          board_t board;
          init_board(expected_FEN, &board);

          const std::string result_FEN = generate_FEN(&board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }

        for (const json& expected : test_case["expected"]) {
          const std::string expected_FEN = expected["fen"];
          board_t board;
          init_board(expected_FEN, &board);

          const std::string result_FEN = generate_FEN(&board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }
      }
    }
  }

  TEST_CASE("Test algebraic parsing")
  {
    board_t board;
    init_board(DEFAULT_POSITION, &board);

    auto moves = generate_legal_moves(&board);

    for (const auto& move : moves) {
      const std::string generated_algebraic =
          move_to_algebraic(&move, &moves, &board);
      const move_t generated_move =
          algebraic_to_move(generated_algebraic, &board);

      REQUIRE(generated_move == move);
    }
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
        json expected_moves = test_case["expected"];

        board_t board;
        init_board(starting_pos, &board);

        auto moves = generate_legal_moves(&board);

        // Check size
        REQUIRE_MESSAGE(
            moves.size() == expected_moves.size(),
            ("\nRunning " + test_json_file + " File\n" +
             "Starting FEN: " + starting_pos + "\nGenerated moves:\n" +
             moves_to_string(moves, board) + "Difference:\n" +
             difference_to_string(expected_moves, moves, board) +
             print_nice_board(&board)));

        // Check if move is in by Algebraic notation
        for (const json& expected : expected_moves) {
          const std::string move_str = expected["move"];
          std::string fen = expected["fen"];

          {  // Check by algebraic notation
            bool found = contain_move_algebraic(move_str, moves, board);

            REQUIRE_MESSAGE(
                found, ("\nStarting FEN: " + starting_pos +
                        "\nExpect move: " + move_str + " in:\n" +
                        moves_to_string(moves, board) + "Difference:\n" +
                        difference_to_string(expected_moves, moves, board) +
                        print_nice_board(&board)));
          }

          {  // Check by make_move and compare FEN
            const move_t move_to_make = algebraic_to_move(move_str, &board);

            // Make the move on a temporary board
            board_t tmp_board = board;
            bool move_happened = make_move(&move_to_make, &tmp_board);

            REQUIRE(move_happened);
            REQUIRE_EQ(tmp_board.history.size(), 1);

            std::string new_fen = generate_FEN(&tmp_board);

            REQUIRE_MESSAGE(
                new_fen == fen,
                ("\nStarting FEN: " + starting_pos +
                 "\nExpect move: " + move_str + " in:\n" +
                 moves_to_string(moves, tmp_board) + "Difference:\n" +
                 difference_to_string(expected_moves, moves, tmp_board) +
                 print_nice_board(&tmp_board)));
          }
        }
      }
    }
  }
}
