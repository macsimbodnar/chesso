#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "datagen_rules.hpp"
#include "test_helpers.hpp"

// What ends a datagen self-play game, tools/datagen_rules.hpp's
// terminal_result(), the function tools/datagen.cpp plays every game by. S331.
//
// Until S331 it drew a game on is_position_repeated(), a two-fold anywhere,
// while the search since S207 (DEC-173) reads one earlier occurrence in the
// game as no draw. So the engine repeated a position once when it saw no
// reason not to, and datagen scored the game a draw the laws do not: FIDE 9.2
// and the match harness end a game on the third occurrence.
//
//   1. a position on the board for the second time does not end the game.
//      Precondition: it is the second occurrence, read from the engine's own
//      classify_repetition(), and is_position_repeated(), the rule before
//      S331, fires on it. Observed red before the fix: 0.5. Twice: the start
//      position, whose first occurrence is history entry 0, and the position
//      after 1.e4 e5, whose first is entry 2. The second kills the root-0
//      form, classify_repetition(..., 0), which draws any match above entry
//      0 and so is the two-fold rule again everywhere but at the start.
//   2. the third occurrence still ends it, as a draw.

namespace
{

struct datagen_fixture_t
{
  game_t game;
  datagen_fixture_t() { initialize_game_const_data(&game); }

  // Ng1-f3, Ng8-f6, Nf3-g1, Nf6-g8: the start position again, White to move,
  // with the same castling rights and no en passant square.
  void shuffle_knights()
  {
    REQUIRE(play_move(&game, g1, f3));
    REQUIRE(play_move(&game, g8, f6));
    REQUIRE(play_move(&game, f3, g1));
    REQUIRE(play_move(&game, f6, g8));
  }

  repetition_kind_t kind() const
  { return classify_repetition(&game.history, &game.board, game.history.size); }
};

}  // namespace


TEST_SUITE("datagen: what ends a game")
{
  TEST_CASE_FIXTURE(datagen_fixture_t,
                    "a position occurring a second time does not end the game")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    size_t first_entry = 0;

    SUBCASE("the start position") {}

    SUBCASE("the position after 1.e4 e5")
    {
      REQUIRE(play_move(&game, e2, e4));
      REQUIRE(play_move(&game, e7, e5));
      first_entry = 2;
    }

    REQUIRE_LT(terminal_result(&game), 0.0);
    const uint64_t first = game.board.hash;
    REQUIRE_EQ(game.history.size, first_entry);

    shuffle_knights();

    // Precondition: the position is on the board for the second time, its
    // first occurrence at history entry `first_entry`, and the two-fold rule
    // before S331 would have ended the game here.
    REQUIRE_EQ(game.board.hash, first);
    REQUIRE_EQ(game.history.entries[first_entry].hash, first);
    REQUIRE_EQ(kind(), repetition_kind_t::ONCE_PRE_ROOT);
    REQUIRE(is_position_repeated(&game.history, &game.board));

    CHECK_LT(terminal_result(&game), 0.0);
  }

  TEST_CASE_FIXTURE(datagen_fixture_t, "the third occurrence ends it, drawn")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const uint64_t start = game.board.hash;

    shuffle_knights();
    REQUIRE_EQ(kind(), repetition_kind_t::ONCE_PRE_ROOT);

    shuffle_knights();

    // Precondition: the third occurrence, the one the laws end a game on.
    REQUIRE_EQ(game.board.hash, start);
    REQUIRE_EQ(kind(), repetition_kind_t::DRAW);

    CHECK_EQ(terminal_result(&game), 0.5);
  }
}
