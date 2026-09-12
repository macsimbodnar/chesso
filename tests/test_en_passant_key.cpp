// S042/DEC-187. `make_move_impl` (`src/bitboard.cpp`) used to set the
// en-passant square on every double push whether or not an enemy pawn could
// capture it, and `load_FEN`'s S161 sanitiser kept a parsed en-passant square
// on the same too-permissive test (target empty, victim present) with no
// capturer term. Two positions with the same placement, side to move,
// castling rights and halfmove clock then hashed differently depending on
// whether they were reached through a double push (which keyed the square
// unconditionally) or reached another way (which did not) -- so a position
// that recurs by the rules of chess was not recognised as recurring by
// `classify_repetition()`'s key compare. S219's first match found this firing
// live: 236 "PV continues after threefold repetition" warnings in 1500
// self-play games, all genuine threefolds, DEC-187.
//
// The rule adopted here, in `src/bitboard.cpp`'s new
// `en_passant_is_capturable`: an en-passant square is kept only when a pawn of
// the side to move stands on a square from which it attacks the target -- the
// same `pawn_attacks[opponent][en_passant]` candidate test
// `generate_moves_body` already used to decide whether to emit the capture.
// Pseudo-legal, pins not examined, matching X-FEN's "if legal" reading (the
// owner's pick, 2026-09-08).
//
// This file is new (S042's call, per its own text: a new file or new cases in
// tests/test_audit_fen_semantics.cpp). The four existing S161 cases in that
// file stay as they are -- they are unaffected by this rule, since none of
// them concern whether a pawn can *reach* the target, only whether the target
// square and the victim square are consistent with a real double push.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <atomic>
#include <string>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "search.hpp"
#include "test_helpers.hpp"
#include "transposition_table.hpp"

// Defined in bitboard.cpp, not in the header, the same way test_engine.cpp
// declares it: nothing in the engine needs it at runtime, only tests, to check
// the incremental hash against a from-scratch recomputation.
hash_t compute_full_hash(game_t* game);

namespace
{

static game_t game;
static transposition_table_t tt;

struct ep_fixture_t
{
  ep_fixture_t()
  {
    initialize_game_const_data(&game);

    if (tt.entries == nullptr) { tt_resize(&tt, 4); }
    tt_reset(&tt);
  }
};

}  // namespace


TEST_SUITE("en passant: only when capturable (S042, DEC-187)")
{
  // The step's minimized reproduction, history A: White a rook down, and the
  // only way the search can answer anything but a lost score is to find a
  // draw that is not there. python-chess 1.11.2 on the position after the
  // eighth move, replayed independently of the engine:
  // `can_claim_threefold_repetition() True` while `is_repetition(3)` is False
  // -- the position stands twice in the history and the claim looks one legal
  // move ahead to the third, which is the move the search finds; a claimable
  // threefold by the rules of chess, the case S207's two-fold-in-search rule
  // scores as the draw (the fast check over 50b1ff9 corrected this comment).
  //
  // On HEAD (en-passant keyed unconditionally on the g2g4 double push, no
  // black pawn anywhere on the board to capture it) this search returns the
  // material deficit, about -313, because the position right after g2g4
  // hashes with the ep component and the identical placement reached again
  // three shuffles later hashes without it -- classify_repetition() never
  // sees them as the same position. Observed red before the fix, quoted in
  // the step's stamp.
  TEST_CASE_FIXTURE(ep_fixture_t,
                    "a pre-root repetition through a non-capturable en "
                    "passant square scores the draw")
  {
    // Black has no pawn on the board at all, so the en-passant square g2g4
    // creates can never be capturable -- the clearest instance of the S042
    // class, and the one the step's reproduction uses.
    REQUIRE(load_FEN("r5k1/8/8/8/8/8/6PP/6K1 w - - 0 1", &game));

    REQUIRE(play_move(&game, g2, g4));
    REQUIRE(play_move(&game, a8, b8));
    REQUIRE(play_move(&game, g1, f1));
    REQUIRE(play_move(&game, b8, a8));
    REQUIRE(play_move(&game, f1, g1));
    REQUIRE(play_move(&game, a8, b8));
    REQUIRE(play_move(&game, g1, f1));
    REQUIRE(play_move(&game, b8, a8));

    REQUIRE_EQ(game.board.active_color, WHITE);
    REQUIRE_EQ(game.history.size, 8);

    // Precondition, asserted against a loose bound rather than a golden
    // (S192): White is a rook down, so anything but the repetition is a large
    // deficit. -294 at the shipped weights; the bound leaves room for a refit
    // to move it without moving what the case is about.
    REQUIRE_LT(evaluate(&game.board), -200);

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);
    tt_new_search(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    const search_t result = search(4, &game, &state);

    // The property: a position that recurs by the rules of chess is scored a
    // draw regardless of whether an intervening double push carried a
    // capturable en-passant square. `0` is DRAW_SCORE (private to
    // src/search.cpp); the material deficit this reads on the unfixed tree is
    // about a rook, so the bound is unmistakable either way.
    REQUIRE_EQ(result.score, 0);
  }

  // History B, the step's control: the same eight-ply game, replayed from one
  // ply later so the first move is never a double push at all (White's pawns
  // start already on g4/h2). No en-passant square is ever created, so nothing
  // here depends on the fix -- observed at 0 both before and after, and kept
  // to show the reproduction above isolates the double push and not the
  // shuffle.
  TEST_CASE_FIXTURE(ep_fixture_t,
                    "the control: no double push in the history already "
                    "reads the draw")
  {
    REQUIRE(load_FEN("r5k1/8/8/8/6P1/8/7P/6K1 b - - 0 1", &game));

    REQUIRE(play_move(&game, a8, b8));
    REQUIRE(play_move(&game, g1, f1));
    REQUIRE(play_move(&game, b8, a8));
    REQUIRE(play_move(&game, f1, g1));
    REQUIRE(play_move(&game, a8, b8));
    REQUIRE(play_move(&game, g1, f1));
    REQUIRE(play_move(&game, b8, a8));

    REQUIRE_EQ(game.board.active_color, WHITE);
    REQUIRE_EQ(game.history.size, 7);

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);
    tt_new_search(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    const search_t result = search(6, &game, &state);

    REQUIRE_EQ(result.score, 0);
  }

  // The incremental key make_move_impl builds and the key compute_full_hash()
  // recomputes from the board from scratch must agree after every double
  // push -- with a capturing pawn on the board and without one, and on both
  // colours, so all four combinations of the rule's two axes are covered. Not
  // red-first in the sense of the class above: compute_full_hash() has always
  // read whatever make_move_impl currently left in board->en_passant, so the
  // two never disagreed with each other on one board. What this guards is
  // that S042's rule, applied at the one site that decides the square, keeps
  // being the only place that decides it -- a future change to one and not
  // the other would show up here as a hash mismatch on a single position,
  // rather than only as a mismatch between two positions the way the bug
  // itself did.
  TEST_CASE_FIXTURE(ep_fixture_t,
                    "incremental key equals the recomputed hash after a "
                    "double push, with and without a capturing pawn")
  {
    struct case_t
    {
      std::string fen;
      index_t from;
      index_t to;
      index_t expected_ep;
      const char* why;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/3p4/8/4P3/4K3 w - - 0 1", e2, e4, e3,
       "White double push, black pawn on d4 attacks e3"},
      {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", e2, e4, INVALID_INDEX,
       "White double push, no black pawn anywhere"},
      {"4k3/4p3/8/3P4/8/8/8/4K3 b - - 0 1", e7, e5, e6,
       "Black double push, white pawn on d5 attacks e6"},
      {"4k3/4p3/8/8/8/8/8/4K3 b - - 0 1", e7, e5, INVALID_INDEX,
       "Black double push, no white pawn anywhere"},
    };
    // clang-format on

    for (const case_t& c : cases) {
      REQUIRE_MESSAGE(load_FEN(c.fen, &game), c.why);
      REQUIRE_MESSAGE(play_move(&game, c.from, c.to), c.why);

      CHECK_MESSAGE(game.board.en_passant == c.expected_ep, c.why);
      CHECK_MESSAGE(game.board.hash == compute_full_hash(&game), c.why);
    }
  }

  // The headline reproduction from "What happens now": 1.d4 d5 2.c4 and
  // 1.c4 d5 2.d4 reach the same placement, side to move, castling rights and
  // halfmove clock, and neither final en-passant square (c3 for the first
  // order, d3 for the second) has a black pawn anywhere near it -- b4/d4 for
  // c3, c4/e4 for d3 -- so under the fix both are cleared and the two orders
  // hash equal and print the same FEN. On HEAD they hash differently (the
  // step's own numbers: 11800988595472028807 against 2736264005390963272) and
  // this case would fail at the first REQUIRE_EQ below.
  TEST_CASE(
      "two orders of the same two pawn moves hash equal and print the "
      "same FEN")
  {
    game_t order1 = {};
    game_t order2 = {};

    initialize_game_const_data(&order1);
    initialize_game_const_data(&order2);

    REQUIRE(load_FEN(DEFAULT_POSITION, &order1));
    REQUIRE(play_move(&order1, d2, d4));
    REQUIRE(play_move(&order1, d7, d5));
    REQUIRE(play_move(&order1, c2, c4));

    REQUIRE(load_FEN(DEFAULT_POSITION, &order2));
    REQUIRE(play_move(&order2, c2, c4));
    REQUIRE(play_move(&order2, d7, d5));
    REQUIRE(play_move(&order2, d2, d4));

    // Neither black pawn stands where it would need to for either order's
    // final en-passant target to be capturable, so both are cleared.
    CHECK_EQ(order1.board.en_passant, INVALID_INDEX);
    CHECK_EQ(order2.board.en_passant, INVALID_INDEX);

    REQUIRE_EQ(order1.board.hash, order2.board.hash);
    REQUIRE_EQ(generate_FEN(&order1.board), generate_FEN(&order2.board));

    // Both incremental keys still equal what a from-scratch recomputation
    // gives the same board.
    CHECK_EQ(order1.board.hash, compute_full_hash(&order1));
    CHECK_EQ(order2.board.hash, compute_full_hash(&order2));
  }
}
