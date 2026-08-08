#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <unordered_map>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "openings.hpp"


static game_t game;


TEST_SUITE("Test openings")
{
  TEST_CASE("Initialize")
  { initialize_game_const_data(&game); }

  TEST_CASE("Test key generation")
  {
    // clang-format off
    std::unordered_map<std::string, uint64_t> test_cases = {
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5060803636482931868ULL},
      {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 9384546495678726550ULL},
      {"rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", 528813709611831216ULL},
      {"rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2", 7363297126586722772ULL},
      {"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", 2496273314520498040ULL},
      {"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3", 7289745035295343297ULL},
      {"rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4", 71445182323015129ULL},
      {"rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3", 4359805404264691255ULL},
      {"rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4", 6647202560273257824ULL}
    };
    // clang-format on

    for (const auto& test_case : test_cases) {
      const std::string& pos = test_case.first;
      const uint64_t expected_key = test_case.second;

      load_FEN(pos, &game);

      const uint64_t key = get_key(&game.board);
      REQUIRE_EQ(key, expected_key);
    }
  }

  TEST_CASE("Test get moves")
  {
    load_FEN(DEFAULT_POSITION, &game);
    book_t book;
    load_book_embedded(&book);

    move_t moves[MAX_MOVES];
    const size_t moves_cout = get_book_moves_for_key(&book, &game.board, moves);
    REQUIRE(moves_cout > 0);

    REQUIRE(make_move(&game, moves[0]));
  }
}
