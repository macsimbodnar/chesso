#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <random>
#include <string>
#include "bitboard.hpp"
#include "log.hpp"
#include "utils.hpp"


using json = nlohmann::json;
static std::random_device rd;
static std::mt19937 gen(rd());

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
    load_FEN(DEFAULT_POSITION, &game);

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
          load_FEN(expected_FEN, &game);

          const std::string result_FEN = generate_FEN(&game.board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }

        for (const json& expected : test_case["expected"]) {
          const std::string expected_FEN = expected["fen"];
          load_FEN(expected_FEN, &game);

          const std::string result_FEN = generate_FEN(&game.board);
          REQUIRE_EQ(result_FEN, expected_FEN);
        }
      }
    }
  }

  TEST_CASE("Test algebraic parsing")
  {
    load_FEN(DEFAULT_POSITION, &game);

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
    load_FEN(DEFAULT_POSITION, &game);

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

        load_FEN(starting_pos, &game);

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

        load_FEN(starting_pos, &game);

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
    load_FEN(DEFAULT_POSITION, &game);

    // We limit the depth to the maximum number of repetitions we can store in
    // order to avoid a crash
    const int max_depth = 500;
    // (sizeof(globals.repetitions) / sizeof(globals.repetitions[0])) - 1;

    const int depth_reached = make_random_move(max_depth, &game);

    std::cout << "Test random moves depth reached: "
              << (max_depth - depth_reached) << std::endl;
  }
}

// TEST_SUITE("Test evaluation")
// {
//   TEST_CASE("Test double pawns detection")
//   {
//     // White
//     init_board("4P3/pppppppP/1p5P/1p6/1p6/1P3P2/1P1P4/4P1K1 w - - 0 1",
//     &board,
//                &globals);

//     REQUIRE(is_double_pawn(string_coordinates_to_index("b2"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("b3"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("e1"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("e8"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("h6"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("h7"), &board));

//     REQUIRE_FALSE(is_double_pawn(string_coordinates_to_index("d2"),
//     &board));
//     REQUIRE_FALSE(is_double_pawn(string_coordinates_to_index("f3"),
//     &board));

//     // Black
//     init_board("8/pkp3pp/1p2p3/2p1p3/2p5/3PPP2/PPP3PP/6K1 b - - 0 1",
//     &board,
//                &globals);

//     REQUIRE(is_double_pawn(string_coordinates_to_index("e6"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("e5"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("c7"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("c5"), &board));
//     REQUIRE(is_double_pawn(string_coordinates_to_index("c4"), &board));

//     REQUIRE_FALSE(is_double_pawn(string_coordinates_to_index("h7"),
//     &board));
//     REQUIRE_FALSE(is_double_pawn(string_coordinates_to_index("g7"),
//     &board));
//     REQUIRE_FALSE(is_double_pawn(string_coordinates_to_index("b6"),
//     &board));
//     REQUIRE_FALSE(is_double_pawn(string_coordinates_to_index("a7"),
//     &board));
//     REQUIRE_FALSE(is_double_pawn(string_coordinates_to_index("b7"),
//     &board));
//   }


//   TEST_CASE("Test passed pawns detection")
//   {
//     init_board("4k3/8/7p/1P2Pp1P/2Pp1PP1/8/8/4K3 w - - 0 1", &board,
//     &globals);

//     REQUIRE(is_passed_pawn(string_coordinates_to_index("b5"), &board));
//     REQUIRE(is_passed_pawn(string_coordinates_to_index("c4"), &board));
//     REQUIRE(is_passed_pawn(string_coordinates_to_index("e5"), &board));
//     REQUIRE(is_passed_pawn(string_coordinates_to_index("d4"), &board));

//     REQUIRE_FALSE(is_passed_pawn(string_coordinates_to_index("f4"),
//     &board));
//     REQUIRE_FALSE(is_passed_pawn(string_coordinates_to_index("f5"),
//     &board));
//     REQUIRE_FALSE(is_passed_pawn(string_coordinates_to_index("g4"),
//     &board));
//     REQUIRE_FALSE(is_passed_pawn(string_coordinates_to_index("h5"),
//     &board));
//     REQUIRE_FALSE(is_passed_pawn(string_coordinates_to_index("h6"),
//     &board));
//     REQUIRE_FALSE(is_passed_pawn(string_coordinates_to_index("a2"),
//     &board));
//   }


//   TEST_CASE("Test isolated pawns detection")
//   {
//     init_board("4k3/pp6/7p/1P2Pp1P/3p1PP1/8/2p5/4K3 w - - 0 1", &board,
//                &globals);

//     REQUIRE(is_isolated_pawn(string_coordinates_to_index("b5"), &board));
//     REQUIRE(is_isolated_pawn(string_coordinates_to_index("f5"), &board));
//     REQUIRE(is_isolated_pawn(string_coordinates_to_index("h6"), &board));


//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("a7"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("b7"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("c2"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("d4"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("e5"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("f4"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("g4"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("h5"),
//     &board));
//     REQUIRE_FALSE(is_isolated_pawn(string_coordinates_to_index("e1"),
//     &board));
//   }

//   TEST_CASE("Test double pawns evaluation")
//   {
//     init_board("3k4/pp4pp/8/8/8/7P/PP5P/3K4 w - - 0 1", &board, &globals);

//     int score = evaluate(&board);
//     REQUIRE_EQ(score, -40);

//     init_board("3k4/pp5p/7p/8/8/8/PP4PP/3K4 w - - 0 1", &board, &globals);

//     score = evaluate(&board);
//     REQUIRE_EQ(score, 40);
//   }


//   TEST_CASE("Test isolated pawns evaluation")
//   {
//     init_board("3k4/ppp2ppp/8/4P3/8/8/PPP3PP/3K4 w - - 0 1", &board,
//     &globals);

//     int score = evaluate(&board);
//     REQUIRE_EQ(score, 10);

//     init_board("3k4/ppp3pp/8/8/4p3/8/PPP2PPP/3K4 w - - 0 1", &board,
//     &globals);

//     score = evaluate(&board);
//     REQUIRE_EQ(score, -10);

//     init_board("3k4/pp4pp/8/3p4/3P4/8/PP4PP/3K4 w - - 0 1", &board,
//     &globals);

//     score = evaluate(&board);
//     REQUIRE_EQ(score, 0);
//   }

//   TEST_CASE("Test passed pawns evaluation")
//   {
//     init_board("3k4/8/8/p4ppp/1PP3PP/8/8/3K4 w - - 0 1", &board, &globals);

//     int score = evaluate(&board);
//     REQUIRE_EQ(score, 45);

//     init_board("3k4/8/8/1pp2pp1/PPP4P/8/8/3K4 w - - 0 1", &board,
//     &globals);

//     score = evaluate(&board);
//     REQUIRE_EQ(score, -40);

//     // TODO: Check this testcase. Should be 0
//     init_board("3k4/8/8/5ppp/PPP5/8/8/3K4 w - - 0 1", &board, &globals);

//     score = evaluate(&board);
//     REQUIRE_EQ(score, 5);
//   }

//   TEST_CASE("Test count pieces on file")
//   {
//     init_board("3k4/1p4p1/2p1pp2/4p3/1Q1BBB2/B4PP1/2B2P2/3K4 w - - 0 1",
//     &board,
//                &globals);

//     piece_count_t count;

//     count = count_pieces_on_file(string_coordinates_to_index("a3"),
//     &board); REQUIRE_EQ(count.white, 0); REQUIRE_EQ(count.black, 0);

//     count = count_pieces_on_file(string_coordinates_to_index("b7"),
//     &board); REQUIRE_EQ(count.white, 1); REQUIRE_EQ(count.black, 0);

//     count = count_pieces_on_file(string_coordinates_to_index("c2"),
//     &board); REQUIRE_EQ(count.white, 0); REQUIRE_EQ(count.black, 1);

//     count = count_pieces_on_file(string_coordinates_to_index("d1"),
//     &board); REQUIRE_EQ(count.white, 1); REQUIRE_EQ(count.black, 1);

//     count = count_pieces_on_file(string_coordinates_to_index("e1"),
//     &board); REQUIRE_EQ(count.white, 1); REQUIRE_EQ(count.black, 2);

//     count = count_pieces_on_file(string_coordinates_to_index("f4"),
//     &board); REQUIRE_EQ(count.white, 2); REQUIRE_EQ(count.black, 1);

//     count = count_pieces_on_file(string_coordinates_to_index("g7"),
//     &board); REQUIRE_EQ(count.white, 1); REQUIRE_EQ(count.black, 0);

//     count = count_pieces_on_file(string_coordinates_to_index("h1"),
//     &board); REQUIRE_EQ(count.white, 0); REQUIRE_EQ(count.black, 0);
//   }

//   TEST_CASE("Test king shield")
//   {
//     init_board("2k5/1pp5/8/8/8/8/5PPP/6K1 w - - 0 1", &board, &globals);
//     bool res;

//     res = is_king_shielded(string_coordinates_to_index("g1"), &board);
//     REQUIRE(res);

//     res = is_king_shielded(string_coordinates_to_index("c8"), &board);
//     REQUIRE_FALSE(res);

//     init_board("2k5/1ppp4/8/8/8/8/5P1P/6K1 w - - 0 1", &board, &globals);

//     res = is_king_shielded(string_coordinates_to_index("g1"), &board);
//     REQUIRE_FALSE(res);

//     res = is_king_shielded(string_coordinates_to_index("c8"), &board);
//     REQUIRE(res);
//   }

//   TEST_CASE("Test is in check")
//   {
//     struct test_case_t
//     {
//       std::string FEN;
//       bool is_in_check;
//     };

//     // clang-format off
//     const std::array<test_case_t, 4> test_cases = {{
//       {"3k4/8/7p/Q1p3pP/1pPpPpP1/1P1PpP2/N7/2K5 b - - 0 1", true},
//       {"3k4/8/7p/Q1p3pP/1pPpPpP1/1P1PpP2/N7/2K5 w - - 0 1", false},
//       {"3k4/8/7p/2p3pP/1pPpPpPQ/1P1PpP2/N7/2K4r w - - 0 1", true},
//       {"3k4/8/7p/2p3pP/1pPpPpPQ/1P1PpP2/N7/2K4r b - - 0 1", false},
//     }};
//     // clang-format on

//     for (const auto& test_case : test_cases) {
//       init_board(test_case.FEN, &board, &globals);
//       const bool res = is_check(&board, &globals.zobrist_randoms);
//       REQUIRE_EQ(res, test_case.is_in_check);
//     }
//   }
// }
