#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <random>
#include <string>
#include "board.hpp"
#include "evaluation.hpp"
#include "move_generator.hpp"
#include "utils.hpp"


using json = nlohmann::json;
std::random_device rd;
std::mt19937 gen(rd());


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


bool contain_move(const move_t& move, move_t moves[], size_t moves_size)
{
  for (size_t i = 0; i < moves_size; ++i) {
    if (moves[i] == move) { return true; }
  }

  return false;
}


std::string moves_to_string(move_t moves[],
                            size_t move_size,
                            const board_t& board)
{
  std::string result;

  std::vector<move_t> moves_v;
  for (size_t i = 0; i < move_size; ++i) {
    moves_v.push_back(moves[i]);
  }


  for (size_t i = 0; i < move_size; ++i) {
    const move_t& move = moves[i];

    result += index_to_string_coordinates(move.from) + " -> " +
              index_to_string_coordinates(move.to);
    result += "    " + move_to_algebraic(&move, &moves_v, &board);
    result += "\n";
  }

  return result;
}


bool contain_move_algebraic(const std::string& move,
                            const move_t moves[],
                            size_t moves_size,
                            const board_t& board)
{
  std::vector<move_t> moves_v;
  for (size_t i = 0; i < moves_size; ++i) {
    moves_v.push_back(moves[i]);
  }


  for (size_t i = 0; i < moves_size; ++i) {
    if (move == move_to_algebraic(&moves[i], &moves_v, &board)) { return true; }
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

      if (move_str == expected["move"].get<std::string>()) {
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
      if (move_str == expected["move"].get<std::string>()) {
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


move_t pick_random_move(const move_t moves[], size_t moves_size)
{
  std::uniform_int_distribution<size_t> dist(0, moves_size - 1);
  const size_t random_index = dist(gen);
  return moves[random_index];
}


static move_t moves[270];
int make_random_move(int depth, board_t* board, history_t* history)
{
  if (depth == 0) { return 0; }

  const std::string fen_before = generate_FEN(board);
  const uint64_t zobrist_before = board->game_state.zobrist_key;

  const size_t moves_count = generate_legal_moves(board, moves);

  if (moves_count == 0) { return depth; }

  const move_t move_to_make = pick_random_move(moves, moves_count);
  bool move_happened = make_move(&move_to_make, board, history);
  REQUIRE(move_happened);

  const std::string fen_after_make_move = generate_FEN(board);
  REQUIRE_NE(fen_after_make_move, fen_before);

  const uint64_t zobrist_make = board->game_state.zobrist_key;
  REQUIRE_NE(zobrist_make, zobrist_before);

  // Recursively go deeper
  int depth_reached = make_random_move(depth - 1, board, history);

  // Unmake the move
  const bool move_reverted = unmake_move(board, history);
  REQUIRE(move_reverted);

  const std::string fen_after_unmake_move = generate_FEN(board);
  REQUIRE_EQ(fen_after_unmake_move, fen_before);

  const uint64_t zobrist_unmake = board->game_state.zobrist_key;
  REQUIRE_EQ(zobrist_unmake, zobrist_before);

  return depth_reached;
}


TEST_SUITE("DEBUG TEST")
{
  TEST_CASE("DEBUG")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);
    // board.game_state.active_color = BLACK;
    move_t move(23, 55, W_PAWN);

    make_move(&move, &board, nullptr);

    int evaluation = evaluate(&board);

    std::cout << print_nice_board(&board) << std::endl;
    std::cout << "Evaluation: " << evaluation << std::endl;

    // std::cout << "debug: " << debug_evaluation(W_PAWN, 0x10) << std::endl;
    // std::cout << "debug: " << debug_evaluation(B_PAWN, 0x60) << std::endl;
  }
}


TEST_SUITE("Test utils")
{
  TEST_CASE("Test FEN")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

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
          history_t history;
          board_t board;
          init_board(expected_FEN, &board, &history);

          const std::string result_FEN = generate_FEN(&board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }

        for (const json& expected : test_case["expected"]) {
          const std::string expected_FEN = expected["fen"];
          history_t history;
          board_t board;
          init_board(expected_FEN, &board, &history);

          const std::string result_FEN = generate_FEN(&board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }
      }
    }
  }

  // TEST_CASE("Test algebraic parsing")
  // {
  //   history_t history;
  //   board_t board;
  //   init_board(DEFAULT_POSITION, &board, &history);

  //   auto moves = generate_legal_moves(&board);

  //   for (const auto& move : moves) {
  //     const std::string generated_algebraic =
  //         move_to_algebraic(&move, &moves, &board);
  //     const move_t generated_move =
  //         algebraic_to_move(generated_algebraic, &board);

  //     REQUIRE(generated_move == move);
  //   }
  // }
}


TEST_SUITE("Test pseudo legal move generator")
{
  TEST_CASE("Test pseudo legal black pawn")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    index_t index = position_to_index(0, 6);
    piece_t piece = board.board[index];

    move_t moves[30];
    size_t moves_count =
        generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 2);

    move_t pos_1 = {index, position_to_index(0, 5), piece};
    REQUIRE(contain_move(pos_1, moves, moves_count));

    move_t pos_2 = {index, position_to_index(0, 4), piece};
    pos_2.double_pawn_move = true;
    REQUIRE(contain_move(pos_2, moves, moves_count));
  }


  TEST_CASE("Test pseudo legal white pawn")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    index_t index = position_to_index(0, 1);
    piece_t piece = board.board[index];

    move_t moves[30];
    size_t moves_count =
        generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 2);

    move_t pos_1 = {index, position_to_index(0, 2), piece};
    REQUIRE(contain_move(pos_1, moves, moves_count));

    move_t pos_2 = {index, position_to_index(0, 3), piece};
    pos_2.double_pawn_move = true;
    REQUIRE(contain_move(pos_2, moves, moves_count));
  }


  TEST_CASE("Test pseudo legal rooks")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    index_t index = position_to_index(7, 7);

    move_t moves[30];
    size_t moves_count =
        generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(0, 7);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(0, 0);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(7, 0);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);
  }


  TEST_CASE("Test pseudo legal bishops")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    index_t index = position_to_index(2, 0);

    move_t moves[30];
    size_t moves_count =
        generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(5, 0);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(2, 7);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(5, 7);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);
  }


  TEST_CASE("Test pseudo legal knight")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    index_t index = position_to_index(1, 0);
    piece_t piece = board.board[index];

    move_t moves[30];
    size_t moves_count =
        generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 2);
    REQUIRE(contain_move(move_t(index, position_to_index(0, 2), piece), moves,
                         moves_count));
    REQUIRE(contain_move(move_t(index, position_to_index(2, 2), piece), moves,
                         moves_count));

    index = position_to_index(6, 0);
    piece = board.board[index];
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 2);
    REQUIRE(contain_move(move_t(index, position_to_index(5, 2), piece), moves,
                         moves_count));
    REQUIRE(contain_move(move_t(index, position_to_index(7, 2), piece), moves,
                         moves_count));

    index = position_to_index(1, 7);
    piece = board.board[index];
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 2);
    REQUIRE(contain_move(move_t(index, position_to_index(0, 5), piece), moves,
                         moves_count));
    REQUIRE(contain_move(move_t(index, position_to_index(2, 5), piece), moves,
                         moves_count));

    index = position_to_index(6, 7);
    piece = board.board[index];
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 2);
    REQUIRE(contain_move(move_t(index, position_to_index(7, 5), piece), moves,
                         moves_count));
    REQUIRE(contain_move(move_t(index, position_to_index(5, 5), piece), moves,
                         moves_count));
  }


  TEST_CASE("Test pseudo legal queen")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    index_t index = position_to_index(3, 0);

    move_t moves[30];
    size_t moves_count =
        generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(3, 7);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);
  }


  TEST_CASE("Test pseudo legal king")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    index_t index = position_to_index(4, 0);

    move_t moves[30];
    size_t moves_count =
        generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);

    index = position_to_index(4, 7);
    moves_count = generate_pseudo_legal_moves_from_index(index, &board, moves);

    REQUIRE_EQ(moves_count, 0);
  }
}


TEST_SUITE("Test legal move generator")
{
  TEST_CASE("Basic test")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    move_t moves[270];
    const size_t moves_count = generate_legal_moves(&board, moves);
    REQUIRE_EQ(moves_count, 20);
  }

  TEST_CASE("Test against generated jsons")
  {
    for (const auto& test_json_file : test_files) {
      json test_cases = load_json(test_json_file);

      for (const json& test_case : test_cases["testCases"]) {
        std::string starting_pos = test_case["start"]["fen"];
        json expected_moves = test_case["expected"];

        history_t history;
        board_t board;
        init_board(starting_pos, &board, &history);

        move_t moves[270];
        const size_t moves_count = generate_legal_moves(&board, moves);

        // Check size
        REQUIRE_MESSAGE(
            moves_count == expected_moves.size(),
            // ("\nRunning " + test_json_file + " File\n" +
            //  "Starting FEN: " + starting_pos + "\nGenerated moves:\n" +
            //  moves_to_string(moves, moves_count, board) + "Difference:\n" +
            //  difference_to_string(expected_moves, moves, board) +
            //  print_nice_board(&board)));
            "");

        // Check if move is in by Algebraic notation
        for (const json& expected : expected_moves) {
          const std::string move_str = expected["move"];
          std::string fen = expected["fen"];

          {  // Check by algebraic notation
            bool found =
                contain_move_algebraic(move_str, moves, moves_count, board);

            REQUIRE_MESSAGE(
                found,
                // ("\nStarting FEN: " + starting_pos +
                //         "\nExpect move: " + move_str + " in:\n" +
                //         moves_to_string(moves, board) + "Difference:\n" +
                //         difference_to_string(expected_moves, moves, board) +
                //         print_nice_board(&board)));
                "");
          }

          {  // Check by make_move and compare FEN
            const move_t move_to_make = algebraic_to_move(move_str, &board);

            // Make the move on a temporary board
            board_t tmp_board = board;
            bool move_happened = make_move(&move_to_make, &tmp_board, nullptr);

            REQUIRE(move_happened);

            std::string new_fen = generate_FEN(&tmp_board);

            REQUIRE_MESSAGE(
                new_fen == fen,
                // ("\nStarting FEN: " + starting_pos +
                //  "\nExpect move: " + move_str + " in:\n" +
                //  moves_to_string(moves, tmp_board) + "Difference:\n" +
                //  difference_to_string(expected_moves, moves, tmp_board) +
                //  print_nice_board(&tmp_board)));
                "");
          }
        }
      }
    }
  }
}


TEST_SUITE("Test make_move and unmake_move")
{
  TEST_CASE("Test make move with jsons")
  {
    for (const auto& test_json_file : test_files) {
      json test_cases = load_json(test_json_file);

      for (const json& test_case : test_cases["testCases"]) {
        std::string starting_pos = test_case["start"]["fen"];
        json expected_moves = test_case["expected"];

        history_t history;
        board_t board;
        init_board(starting_pos, &board, &history);

        move_t moves[270];
        const size_t moves_count = generate_legal_moves(&board, moves);

        // Apply the move
        for (size_t i = 0; i < moves_count; ++i) {
          const move_t& move = moves[i];

          const std::string fen_before_move = generate_FEN(&board);
          const uint64_t zobrist_key_before = board.game_state.zobrist_key;

          const bool result = make_move(&move, &board, &history);
          REQUIRE(result);

          // Test the fen and zobrist keys changed
          const std::string fen_after_make_move = generate_FEN(&board);
          REQUIRE_NE(fen_after_make_move, fen_before_move);

          const uint64_t zobrist_key_after_make_move =
              board.game_state.zobrist_key;
          REQUIRE_NE(zobrist_key_after_make_move, zobrist_key_before);

          // Unmake the move
          const bool un_result = unmake_move(&board, &history);
          REQUIRE(un_result);

          // Test fen and zobrist key is restored as before
          const std::string fen_after_unmake = generate_FEN(&board);
          REQUIRE_EQ(fen_after_unmake, fen_before_move);

          const uint64_t zobrist_key_after_unmake_move =
              board.game_state.zobrist_key;
          REQUIRE_EQ(zobrist_key_after_unmake_move, zobrist_key_before);
        }
      }
    }
  }

  TEST_CASE("Test random moves")
  {
    history_t history;
    board_t board;
    init_board(DEFAULT_POSITION, &board, &history);

    const int max_depth = 10000;

    const int depth_reached = make_random_move(max_depth, &board, &history);

    std::cout << "Test random moves depth reached: "
              << (max_depth - depth_reached) << std::endl;
  }
}
