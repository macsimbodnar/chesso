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


// The generator's output, unfiltered. It used to run every move through
// is_move_legal(), which is membership in generate_moves()' own output, so the
// filter removed nothing and could not - and the `assert` below it was compiled
// out of the Release build the gate runs. What pins legality here is the JSON:
// each case compares this against an exact expected count and an expected move
// per entry, and "Test make move with jsons" replays every move. INV-1. Two of
// the 2026-09-04 review's mutants, M22 and M27, die on those counts.
// S193, 2026-09-04_test_review-F05.
size_t test_generate_legal_moves(game_t* game, move_t moves[])
{
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  REQUIRE_LT(count, size_t(MAX_MOVES));

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


// S042 moved en-passant to the X-FEN convention: load_FEN keeps a parsed
// en-passant square only when a pawn of the side to move could capture on it
// (src/bitboard.cpp's en_passant_is_capturable). test_files' rampart fixtures
// predate that and record a square after every double push, capturable or
// not -- 126 of the 138 non-"-" fourth fields across the eight files disagree
// with python-chess's xfen oracle (script run at S042's hand-off, not kept).
// Three examples, confirmed uncapturable with python-chess 1.11.2
// (`chess.Board(fen).has_pseudo_legal_en_passant()` is False on all three):
//   castling.json rnbq1k1r/pp1Pbppp/2p5/8/P1B5/8/1PP1N1PP/RNBQK2n b Q a3 0 8
//     -- b4, the only square a black pawn could retake from, is empty.
//   castling.json rnbqk2N/1pp1n1pp/8/p1b5/8/2P5/PP1pBPPP/RNBQ1K1R w q a6 0 9
//     -- b5, the only square a white pawn could retake from, is empty.
//   castling.json rnbq1k1r/pp1Pbppp/2p5/8/2B3P1/8/PPP1N2P/RNBQK2n b Q g3 0 8
//     -- f4 and h4, the only squares a black pawn could retake from, are
//        both empty.
// A literal string comparison against these fixtures' fourth field is
// therefore no longer defensible: load_FEN can only ever move the en-passant
// field in one direction, from a specific square to "-" (the S161 sanitiser
// only clears, per en_passant_is_capturable; it never invents a square or
// relocates one), so every field is still compared exactly except the
// en-passant one, which is checked against the only two outcomes that
// direction admits.
static bool matches_pre_xfen_fixture(const std::string& generated,
                                     const std::string& fixture)
{
  const std::vector<std::string> gen_parts = split_string(generated);
  const std::vector<std::string> fix_parts = split_string(fixture);

  if (gen_parts.size() != 6 || fix_parts.size() != 6) { return false; }

  for (const size_t field :
       {size_t{0}, size_t{1}, size_t{2}, size_t{4}, size_t{5}}) {
    if (gen_parts[field] != fix_parts[field]) { return false; }
  }

  return gen_parts[3] == fix_parts[3] || gen_parts[3] == "-";
}


// Every case in this file reaches the attack tables through generate_moves(),
// load_FEN() or make_move(). They used to be filled by a case named "Test
// INITIALIZATION" that ran first only because doctest's default order is file
// order, so `-tc=<glob>` over one case, or `--order-by=name`, ran the rest on
// zero tables: "Basic test" answered 16 moves for the start position instead of
// 20 in Release, and Debug aborted on game_tables()' own assert. The empty
// INITIALIZATION suite goes with it. S193, 2026-09-04_test_review-F09.
struct chesso_fixture_t
{
  chesso_fixture_t() { initialize_game_const_data(&game); }
};


TEST_SUITE("Test utils")
{
  TEST_CASE_FIXTURE(chesso_fixture_t, "Test FEN")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    std::string fen_result = generate_FEN(&game.board);

    REQUIRE_EQ(fen_result, std::string(DEFAULT_POSITION));
  }

  TEST_CASE_FIXTURE(chesso_fixture_t, "Test fen parsing - generation")
  {
    for (const auto& test_file : test_files) {
      const json test_cases = load_json(test_file);

      for (const json& test_case : test_cases["testCases"]) {
        {
          const std::string expected_FEN = test_case["start"]["fen"];
          REQUIRE(load_FEN(expected_FEN, &game));

          const std::string result_FEN = generate_FEN(&game.board);
          REQUIRE_MESSAGE(
              matches_pre_xfen_fixture(result_FEN, expected_FEN),
              ("Expected: " + expected_FEN + "\nGot: " + result_FEN));
        }

        for (const json& expected : test_case["expected"]) {
          const std::string expected_FEN = expected["fen"];
          REQUIRE(load_FEN(expected_FEN, &game));

          const std::string result_FEN = generate_FEN(&game.board);
          REQUIRE_MESSAGE(
              matches_pre_xfen_fixture(result_FEN, expected_FEN),
              ("Expected: " + expected_FEN + "\nGot: " + result_FEN));
        }
      }
    }
  }

  TEST_CASE_FIXTURE(chesso_fixture_t, "Test algebraic parsing")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    move_t moves[MAX_MOVES];
    size_t moves_count = generate_moves(game_tables(), &game.board, moves);

    for (size_t i = 0; i < moves_count; ++i) {
      const move_t move = moves[i];

      const std::string generated_algebraic =
          move_to_algebraic(&game, move, moves, moves_count);
      const move_t generated_move =
          algebraic_to_move(generated_algebraic, &game);

      REQUIRE(generated_move == move);
    }
  }

  // S174, 2026-09-03_adversarial-F02. The parser's three failure paths used to
  // be `assert(false)` with no return, so the Release build handed the caller a
  // move built from the partial parse -- `from` 0, a piece letter, a `to` that
  // could exceed 63 -- and make_move() applied it. Every caller tests for zero,
  // so zero is the contract; an unparseable token is input, not an invariant.
  TEST_CASE_FIXTURE(chesso_fixture_t,
                    "algebraic_to_move returns 0 for what it cannot parse")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    SUBCASE("too short to hold a destination square")
    {
      CHECK(algebraic_to_move("N", &game) == 0);
      CHECK(algebraic_to_move("e", &game) == 0);
      CHECK(algebraic_to_move("x", &game) == 0);
      CHECK(algebraic_to_move("", &game) == 0);
      // Annotations only: stripping them leaves nothing.
      CHECK(algebraic_to_move("!?", &game) == 0);
    }

    SUBCASE("a destination off the board")
    {
      CHECK(algebraic_to_move("Nz9", &game) == 0);
      CHECK(algebraic_to_move("e9", &game) == 0);
      CHECK(algebraic_to_move("i4", &game) == 0);
      CHECK(algebraic_to_move("a0", &game) == 0);
    }

    SUBCASE("a leading character that is neither a piece nor a file letter")
    {
      // S201. The piece letter was read at position 0 only, so anything in
      // front of it fell to the pawn branch and the disambiguation walk --
      // which records only a-h and 1-8 -- swallowed the letter on its way to
      // the destination square. `.Nf3` came back as the legal pawn push f2f3
      // where the token means g1f3, `make_move()` applied it and the caller
      // was told nothing. Reachable: `1 .Nf3` is legal PGN import format
      // (8.2.2.1), so make_book built a book from a wrong board.
      //
      // The knight move is asserted first: without it this case would pass on
      // a position where g1f3 was illegal anyway.
      const move_t knight = algebraic_to_move("Nf3", &game);
      REQUIRE(knight != 0);

      CHECK(algebraic_to_move(".Nf3", &game) == 0);
      CHECK(algebraic_to_move(".e4", &game) == 0);
      CHECK(algebraic_to_move("..e4", &game) == 0);
      CHECK(algebraic_to_move("-e4", &game) == 0);
      CHECK(algebraic_to_move("xd5", &game) == 0);
    }

    SUBCASE("a well-formed token matching no legal move")
    {
      CHECK(algebraic_to_move("Qxf7", &game) == 0);
      CHECK(algebraic_to_move("e5", &game) == 0);
      CHECK(algebraic_to_move("Nf6", &game) == 0);
      CHECK(algebraic_to_move("Ke2", &game) == 0);
      CHECK(algebraic_to_move("Zf3", &game) == 0);
    }
  }

  // PGN suffix annotations (`!`, `?`, `!?`, `?!`, `!!`, `??`) follow the check
  // marker and are ordinary in published PGN. They mean nothing to the board
  // and are stripped the way `+` and `#` are; before S174 `e4!?` was parsed
  // with the annotation as part of the token and fabricated a move (F02).
  TEST_CASE_FIXTURE(chesso_fixture_t,
                    "algebraic_to_move ignores suffix annotations")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    const move_t e4 = algebraic_to_move("e4", &game);
    REQUIRE(e4 != 0);

    for (const char* annotated :
         {"e4!", "e4?", "e4!?", "e4?!", "e4!!", "e4??"}) {
      CAPTURE(annotated);
      CHECK(algebraic_to_move(annotated, &game) == e4);
    }

    // With a check marker in front of the annotation, as PGN writes it. The
    // position is reached through the engine's own parser and make_move(),
    // never by hand.
    for (const char* san : {"e4", "e5", "Qh5", "Nc6", "Bc4", "Nf6"}) {
      const move_t move = algebraic_to_move(san, &game);
      REQUIRE(move != 0);
      REQUIRE(make_move(&game, move));
    }

    const move_t mate = algebraic_to_move("Qxf7#", &game);
    REQUIRE(mate != 0);
    CHECK(algebraic_to_move("Qxf7", &game) == mate);
    CHECK(algebraic_to_move("Qxf7#!", &game) == mate);
    CHECK(algebraic_to_move("Qxf7#!!", &game) == mate);
    CHECK(algebraic_to_move("Qxf7+?!", &game) == mate);
  }
}


TEST_SUITE("Test move generator")
{
  TEST_CASE_FIXTURE(chesso_fixture_t, "Basic test")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    move_t moves[270];
    const size_t moves_count =
        generate_moves(game_tables(), &game.board, moves);

    REQUIRE_EQ(moves_count, 20);
  }

  TEST_CASE_FIXTURE(chesso_fixture_t, "Test against generated jsons")
  {
    for (const auto& test_json_file : test_files) {
      json test_cases = load_json(test_json_file);

      for (const json& test_case : test_cases["testCases"]) {
        std::string starting_pos = test_case["start"]["fen"];
        json expected_moves = test_case["expected"];

        REQUIRE(load_FEN(starting_pos, &game));

        move_t moves[270];
        const size_t moves_count = test_generate_legal_moves(&game, moves);
        // const size_t moves_count = generate_moves(game_tables(), &game.board,
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

            // S042 moved en-passant to the X-FEN convention (see the comment
            // above matches_pre_xfen_fixture): `fen`, the fixture's expected
            // result, predates that and can carry a now-uncapturable square a
            // real double push still produces. Comparing `new_fen` to `fen`
            // literally would fail on exactly the positions this convention
            // change targets, so `fen` is instead loaded through the same
            // load_FEN sanitiser the played-out position went through, and the
            // two boards' own generate_FEN output is compared -- both
            // canonicalised the same way, so a real divergence in placement,
            // side to move, castling, halfmove clock or fullmove number still
            // fails this exactly as before.
            game_t expected_game = {};
            REQUIRE(load_FEN(fen, &expected_game));
            const std::string expected_fen_canonical =
                generate_FEN(&expected_game.board);

            // Unmake before asserting, not after: found while doing S042, but
            // the mechanism is orthogonal to the en-passant convention and
            // predates it. REQUIRE_MESSAGE's diagnostic argument is only
            // evaluated by doctest if the assertion fails, and that lambda
            // walks moves[] -- generated from the position before
            // move_to_make -- through move_to_algebraic(), which calls
            // make_move() internally to disambiguate. With `game.board`
            // still advanced past move_to_make, that replays a stale move
            // against the wrong side's board: silently wrong in Release (the
            // diagnostic prints a bogus difference/board), and a
            // move_belongs_to_side_to_move abort in Debug, because the
            // now-mutated board and the pre-mutation moves[] disagree on
            // whose move it is. Unmaking first restores exactly the board
            // moves[] was generated from, so the diagnostic -- if it ever
            // runs -- walks moves[] against the board it belongs to.
            unmake_move(&game);

            REQUIRE_MESSAGE(
                new_fen == expected_fen_canonical,
                ("\nStarting FEN: " + starting_pos +
                 "\nExpect move: " + move_str + "\n" +
                 "Translated into: " + print_move(move_to_make) + "\nin:\n" +
                 moves_to_string(moves, moves_count, &game) + "Difference:\n" +
                 difference_to_string(expected_moves, moves, moves_count,
                                      &game) +
                 print_nice_board(&game.board) +
                 "\nFixture FEN (pre-X-FEN): " + fen +
                 "\nFixture FEN (canonicalised): " + expected_fen_canonical +
                 "\nGot: " + new_fen));
          }
        }
      }
    }
  }
}


TEST_SUITE("Test make_move and unmake_move")
{
  TEST_CASE_FIXTURE(chesso_fixture_t, "Test make move with jsons")
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

  TEST_CASE_FIXTURE(chesso_fixture_t, "Test random moves")
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


  TEST_CASE_FIXTURE(chesso_fixture_t, "Test is in check")
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

  TEST_CASE_FIXTURE(chesso_fixture_t, "Test is_attacking_king")
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


  TEST_CASE_FIXTURE(chesso_fixture_t, "Test file masks")
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
