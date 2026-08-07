#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <random>
#include <string>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "log.hpp"
#include "utils.hpp"


using json = nlohmann::json;

// A fixed seed keeps a failing random walk reproducible. Set CHESSO_TEST_SEED
// to replay a different one.
static unsigned int test_seed()
{
  const char* env = std::getenv("CHESSO_TEST_SEED");

  if (env == nullptr) { return 20240807u; }

  return static_cast<unsigned int>(std::strtoul(env, nullptr, 10));
}

static std::mt19937 gen(test_seed());

static game_t game;

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


// bool contain_move(const move_t& move, move_t moves[], size_t moves_size)
// {
//   for (size_t i = 0; i < moves_size; ++i) {
//     if (moves[i] == move) { return true; }
//   }

//   return false;
// }


std::string moves_to_string(move_t moves[], size_t move_size, game_t* game)
{
  assert(game != nullptr);

  std::string result;

  for (size_t i = 0; i < move_size; ++i) {
    const unpacked_move_t move(moves[i]);

    result += index_to_str(move.from) + " -> " + index_to_str(move.to);
    result += "    " + move_to_algebraic(game, moves[i], moves, move_size);
    result += "\n";
  }

  return result;
}


bool contain_move_algebraic(const std::string& move,
                            const move_t moves[],
                            size_t moves_size,
                            game_t* game)
{
  for (size_t i = 0; i < moves_size; ++i) {
    if (move == move_to_algebraic(game, moves[i], moves, moves_size)) {
      return true;
    }
  }

  return false;
}


size_t test_generate_legal_moves(game_t* game, move_t moves[])
{
  size_t count = 0;
  move_t all_moves[MAX_MOVES];
  const size_t all_moves_count =
      generate_moves(&game->tables, &game->board, all_moves);

  assert(all_moves_count < MAX_MOVES);

  for (size_t i = 0; i < all_moves_count; ++i) {
    if (is_move_legal(game, all_moves[i])) { moves[count++] = all_moves[i]; }
  }

  return count;
}


std::string difference_to_string(const json& expected_moves,
                                 const move_t generated_moves[],
                                 size_t generated_moves_size,
                                 game_t* game)
{
  std::string result = "";

  std::vector<std::string> missing;
  std::vector<std::string> extra;

  // Search for missing
  for (const json& expected : expected_moves) {
    bool found = false;

    for (size_t i = 0; i < generated_moves_size; ++i) {
      std::string move_str = move_to_algebraic(
          game, generated_moves[i], generated_moves, generated_moves_size);

      if (move_str == expected["move"].get<std::string>()) {
        found = true;
        break;
      }
    }

    if (!found) { missing.push_back(expected["move"]); }
  }

  // Search for extra
  for (size_t i = 0; i < generated_moves_size; ++i) {
    bool found = false;

    std::string move_str = move_to_algebraic(
        game, generated_moves[i], generated_moves, generated_moves_size);

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
int make_random_move(int depth, game_t* g)
{
  assert(g != nullptr);

  if (depth == 0) { return 0; }

  const std::string fen_before = generate_FEN(&g->board);

  const uint64_t zobrist_before = g->board.hash;

  const size_t moves_count = test_generate_legal_moves(g, moves);

  if (moves_count == 0) { return depth; }

  const move_t move_to_make = pick_random_move(moves, moves_count);
  bool move_happened = make_move(g, move_to_make);
  REQUIRE(move_happened);

  const std::string fen_after_make_move = generate_FEN(&g->board);
  REQUIRE_NE(fen_after_make_move, fen_before);

  const uint64_t zobrist_make = g->board.hash;
  REQUIRE_NE(zobrist_make, zobrist_before);

  // Recursively go deeper
  int depth_reached = make_random_move(depth - 1, g);

  // Unmake the move
  unmake_move(g);

  const std::string fen_after_unmake_move = generate_FEN(&g->board);
  REQUIRE_EQ(fen_after_unmake_move, fen_before);

  const uint64_t zobrist_unmake = g->board.hash;
  REQUIRE_EQ(zobrist_unmake, zobrist_before);

  return depth_reached;
}


TEST_SUITE("INITIALIZATION")
{
  TEST_CASE("Test INITIALIZATION")
  {
    initialize_game_const_data(&game);
  }
}

TEST_SUITE("Test utils")
{
  TEST_CASE("Test FEN")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    std::string fen_result = generate_FEN(&game.board);

    REQUIRE_EQ(fen_result, std::string(DEFAULT_POSITION));
  }

  TEST_CASE("Test fen parsing - generation")
  {
    for (const auto& test_file : test_files) {
      const json test_cases = load_json(test_file);

      for (const json& test_case : test_cases["testCases"]) {
        {
          const std::string expected_FEN = test_case["start"]["fen"];
          REQUIRE(load_FEN(expected_FEN, &game));

          const std::string result_FEN = generate_FEN(&game.board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }

        for (const json& expected : test_case["expected"]) {
          const std::string expected_FEN = expected["fen"];
          REQUIRE(load_FEN(expected_FEN, &game));

          const std::string result_FEN = generate_FEN(&game.board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }
      }
    }
  }

  TEST_CASE("Test algebraic parsing")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    move_t moves[MAX_MOVES];
    size_t moves_count = generate_moves(&game.tables, &game.board, moves);

    for (size_t i = 0; i < moves_count; ++i) {
      const move_t move = moves[i];

      const std::string generated_algebraic =
          move_to_algebraic(&game, move, moves, moves_count);
      const move_t generated_move =
          algebraic_to_move(generated_algebraic, &game);

      REQUIRE(generated_move == move);
    }
  }
}


TEST_SUITE("Test move generator")
{
  TEST_CASE("Basic test")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    move_t moves[270];
    const size_t moves_count = generate_moves(&game.tables, &game.board, moves);

    REQUIRE_EQ(moves_count, 20);
  }

  TEST_CASE("Test against generated jsons")
  {
    for (const auto& test_json_file : test_files) {
      json test_cases = load_json(test_json_file);

      for (const json& test_case : test_cases["testCases"]) {
        std::string starting_pos = test_case["start"]["fen"];
        json expected_moves = test_case["expected"];

        REQUIRE(load_FEN(starting_pos, &game));

        move_t moves[270];
        const size_t moves_count = test_generate_legal_moves(&game, moves);
        // const size_t moves_count = generate_moves(&game.tables, &game.board,
        // moves);

        // Check size
        REQUIRE_MESSAGE(
            moves_count == expected_moves.size(),
            ("\nRunning " + test_json_file + " File\n" +
             "Starting FEN: " + starting_pos + "\nGenerated moves:\n" +
             moves_to_string(moves, moves_count, &game) + "Difference:\n" +
             difference_to_string(expected_moves, moves, moves_count, &game) +
             print_nice_board(&game.board)));

        // Check if move is in by Algebraic notation
        for (const json& expected : expected_moves) {
          const std::string move_str = expected["move"];
          std::string fen = expected["fen"];

          {  // Check by algebraic notation
            bool found =
                contain_move_algebraic(move_str, moves, moves_count, &game);

            REQUIRE_MESSAGE(found, ("\nStarting FEN: " + starting_pos +
                                    "\nExpect move: " + move_str + " in:\n" +
                                    moves_to_string(moves, moves_count, &game) +
                                    "Difference:\n" +
                                    difference_to_string(expected_moves, moves,
                                                         moves_count, &game) +
                                    print_nice_board(&game.board)));
          }

          {  // Check by make_move and compare FEN
            const move_t move_to_make = algebraic_to_move(move_str, &game);

            const bool move_happened = make_move(&game, move_to_make);

            REQUIRE(move_happened);

            std::string new_fen = generate_FEN(&game.board);

            REQUIRE_MESSAGE(
                new_fen == fen,
                ("\nStarting FEN: " + starting_pos +
                 "\nExpect move: " + move_str + "\n" +
                 "Translated into: " + print_move(move_to_make) + "\nin:\n" +
                 moves_to_string(moves, moves_count, &game) + "Difference:\n" +
                 difference_to_string(expected_moves, moves, moves_count,
                                      &game) +
                 print_nice_board(&game.board)));

            unmake_move(&game);
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

        REQUIRE(load_FEN(starting_pos, &game));

        move_t moves[270];
        const size_t moves_count = test_generate_legal_moves(&game, moves);

        // Apply the move
        for (size_t i = 0; i < moves_count; ++i) {
          const move_t& move = moves[i];

          const std::string fen_before_move = generate_FEN(&game.board);
          const uint64_t zobrist_key_before = game.board.hash;

          const bool result = make_move(&game, move);
          REQUIRE(result);

          // Test the fen and zobrist keys changed
          const std::string fen_after_make_move = generate_FEN(&game.board);
          REQUIRE_NE(fen_after_make_move, fen_before_move);

          const uint64_t zobrist_key_after_make_move = game.board.hash;
          REQUIRE_NE(zobrist_key_after_make_move, zobrist_key_before);

          // Unmake the move
          unmake_move(&game);

          // Test fen and zobrist key is restored as before
          const std::string fen_after_unmake = generate_FEN(&game.board);
          REQUIRE_EQ(fen_after_unmake, fen_before_move);

          const uint64_t zobrist_key_after_unmake_move = game.board.hash;
          REQUIRE_EQ(zobrist_key_after_unmake_move, zobrist_key_before);
        }
      }
    }
  }

  TEST_CASE("Test random moves")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    // We limit the depth to the maximum number of repetitions we can store in
    // order to avoid a crash
    const int max_depth = 500;
    // (sizeof(globals.repetitions) / sizeof(globals.repetitions[0])) - 1;

    const int depth_reached = make_random_move(max_depth, &game);

    std::cout << "Test random moves seed: " << test_seed()
              << " depth reached: " << (max_depth - depth_reached) << std::endl;

    // Without this the whole walk is satisfied by a game that ends at once.
    REQUIRE(max_depth - depth_reached > 0);
  }


  TEST_CASE("Test is in check")
  {
    struct test_case_t
    {
      std::string FEN;
      bool is_in_check;
    };

    // clang-format off
      const std::array<test_case_t, 4> test_cases = {{
        {"3k4/8/7p/Q1p3pP/1pPpPpP1/1P1PpP2/N7/2K5 b - - 0 1", true},
        {"3k4/8/7p/Q1p3pP/1pPpPpP1/1P1PpP2/N7/2K5 w - - 0 1", false},
        {"3k4/8/7p/2p3pP/1pPpPpPQ/1P1PpP2/N7/2K4r w - - 0 1", true},
        {"3k4/8/7p/2p3pP/1pPpPpPQ/1P1PpP2/N7/2K4r b - - 0 1", false},
      }};
    // clang-format on

    for (const auto& test_case : test_cases) {
      REQUIRE(load_FEN(test_case.FEN, &game));
      const bool res = is_check(&game);
      REQUIRE_EQ(res, test_case.is_in_check);
    }
  }

  TEST_CASE("Test is_attacking_king")
  {
    struct test_case_t
    {
      std::string FEN;
      move_t move;
      bool expected_result;
    };

    const std::array<test_case_t, 4> test_cases = {{
        {"3k4/8/7p/Q1p3pP/1pPpPpP1/1P1PpP2/N7/2K5 w - - 0 1",
         NEW_MOVE(a5, d8, W_QUEEN, TO_NONE, 1, 0, 0, 0), true},
        {"3k4/8/7p/Q1p3pP/1pPpPpP1/1P1PpP2/N7/2K5 w - - 0 1",
         NEW_MOVE(a5, a8, W_QUEEN, TO_NONE, 0, 0, 0, 0), false},
        {"3k4/8/7p/2p3pP/1pPpPpPQ/1P1PpP2/N7/2K4r b - - 0 1",
         NEW_MOVE(h1, h4, B_ROOK, TO_NONE, 1, 0, 0, 0), false},
        {"3k4/8/7p/2p3pP/1pPpPpPQ/1P1PpP2/N7/2K4r b - - 0 1",
         NEW_MOVE(h1, c1, B_ROOK, TO_NONE, 1, 0, 0, 0), true},
    }};

    for (const auto& test_case : test_cases) {
      REQUIRE(load_FEN(test_case.FEN, &game));
      const bool res = is_capturing_king(&game.board, test_case.move);
      REQUIRE_MESSAGE(res == test_case.expected_result,
                      ("Failed with FEN: " + test_case.FEN +
                       "\nMove: " + print_move(test_case.move)));
    }
  }


  TEST_CASE("Test file masks")
  {
    static const bb_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFEULL;
    static const bb_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL;
    static const bb_t NOT_GH_FILES = 0x3F3F3F3F3F3F3F3FULL;
    static const bb_t NOT_AB_FILES = 0xFCFCFCFCFCFCFCFCULL;

    REQUIRE_EQ(NOT_A_FILE, ~file_masks[0]);
    REQUIRE_EQ(NOT_H_FILE, ~file_masks[7]);
    REQUIRE_EQ(NOT_AB_FILES, ~(file_masks[0] | file_masks[1]));
    REQUIRE_EQ(NOT_GH_FILES, ~(file_masks[6] | file_masks[7]));
  }

}
