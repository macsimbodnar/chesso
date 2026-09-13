// AUDIT EVIDENCE — 2026-08-22_adversarial-F01. Written by the 2026-08-22
// adversarial audit as the regression test for that finding, static-only: the
// audit ran while a time-controlled match held every core, so it registered
// and ran nothing. S161 registered it and observed it red as predicted — 4 of
// 4 cases failing on 0a274c9 — then fixed load_FEN. The audit cited its own
// finding as F02 in this header and in the suite name; F02 is fastchess.sh's
// default reference (S160), and the id is corrected here to F01.
//
// The finding: load_FEN() validates FEN syntax and not semantics. Two classes
// of well-formed-but-illegal FEN are accepted, and the moves they license
// corrupt the board in a Release build:
//
//   1. castling rights with the king or rook not on its castling square. The
//      generator's castling block (src/bitboard.cpp:395-435) tests the rights
//      bits, the empty squares and the attacked squares, and never that the
//      king stands on e1/e8 or the rook on its corner. make_move() then moves
//      a rook — castling_rook() answers from the target square, not from the
//      board (src/bitboard.cpp:645-659) — and move_piece()'s xors *create*
//      pieces on squares that were empty.
//
//   2. an en-passant square on any rank. load_FEN() checks only that the
//      square parses (src/bitboard.cpp:1755-1765); an ep square whose victim
//      square holds no enemy pawn licenses an en-passant capture whose
//      remove_piece() xor *adds* the missing pawn.
//
// The assertions below state the correct behaviour — no castling move and no
// en-passant capture may be generated from these positions — so they are
// red-first against the defect and fix-agnostic: they pass whether the fix
// refuses the FEN, sanitizes the rights/ep square, or teaches the generator to
// check placement. INV-1 (legal-only movegen) and the prime directive ("never
// plays or accepts an illegal move and never corrupts its own board state")
// are the properties under test.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <string>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "doctest.h"
#include "test_helpers.hpp"  // stdout_capture_t, for the [position fen] case
#include "uci.hpp"

namespace
{

struct fen_fixture_t
{
  game_t game = {};

  fen_fixture_t() { initialize_game_const_data(&game); }
};

size_t count_flagged(const game_t* game, bool castling, bool en_passant)
{
  move_t moves[MAX_MOVES];
  const size_t count =
      generate_moves(game_tables(), &((game_t*)game)->board, moves);

  size_t flagged = 0;

  for (size_t i = 0; i < count; ++i) {
    if (castling && MOVE_CASTLING(moves[i])) { flagged++; }
    if (en_passant && MOVE_EN_PASSANT(moves[i])) { flagged++; }
  }

  return flagged;
}

}  // namespace


TEST_SUITE("audit: FEN semantic validation (2026-08-22_adversarial-F01)")
{
  TEST_CASE_FIXTURE(fen_fixture_t,
                    "castling rights without the rook license no castle")
  {
    // White claims kingside rights with no rook on h1. If load_FEN accepts
    // this FEN, the position must still generate no castling move: applying
    // one would xor a rook onto h1 and f1 out of nothing.
    const std::string fen = "4k3/8/8/8/8/8/8/4K3 w K - 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, true, false), 0);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t,
                    "castling rights with the king displaced license no castle")
  {
    // The rook is on h1 but the king is on d1. The generator's castling block
    // hard-codes e1 as the from-square, so an emitted castle here moves a king
    // that is not there.
    const std::string fen = "3k4/8/8/8/8/8/8/3K3R w K - 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, true, false), 0);

      // S223: the clearing itself, which "a right with no king anywhere is
      // cleared" held until its position stopped being loadable. A king that is
      // not on its castling square clears both of that side's rights, and this
      // is a placement python-chess 1.11.2 loads with
      // Status.BAD_CASTLING_RIGHTS only -- none of the four king flags -- which
      // is the point: the stray right is sanitised, the position itself is not
      // refused.
      CHECK_EQ(game.board.castling, 0);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t,
                    "an en-passant square with no victim licenses no capture")
  {
    // ep = e4 is not a square any double push can produce for White to act
    // on, and e3 — the square the capture would remove a black pawn from — is
    // empty. If load_FEN accepts the FEN, no en-passant capture may be
    // generated: applying one would xor a black pawn onto e3 out of nothing.
    const std::string fen = "4k3/8/8/8/8/3P4/8/4K3 w - e4 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, false, true), 0);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t,
                    "a plausible-rank ep square with no victim licenses "
                    "no capture")
  {
    // Same defect on the rank en passant really uses: ep = e6 with no black
    // pawn on e5. Well-formed, wrong, and accepted today.
    const std::string fen = "4k3/8/8/3P4/8/8/8/4K3 w - e6 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, false, true), 0);
    }
  }

  // ------------------------------------------------------------------------
  // S161. The four cases above are negative assertions and every one of them
  // would also pass if load_FEN cleared *all* castling rights and *every* ep
  // square, which is a worse engine and a green test. These fix the fix from
  // the other side: what a legal position carries has to survive untouched,
  // and the clearing has to be per right rather than wholesale.
  // ------------------------------------------------------------------------

  TEST_CASE_FIXTURE(fen_fixture_t, "a legal position keeps all four rights")
  {
    REQUIRE(
        load_FEN("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1", &game));

    CHECK_EQ(game.board.castling, WK | WQ | BK | BQ);
    CHECK_EQ(count_flagged(&game, true, false), 2);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "a real ep square survives and is playable")
  {
    // Black has just played d7-d5. The target square is empty, the victim pawn
    // stands on d5, and White's e5 pawn may take it.
    REQUIRE(load_FEN("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", &game));

    CHECK_EQ(game.board.en_passant, d6);
    CHECK_EQ(count_flagged(&game, false, true), 1);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "only the unsupported right is cleared")
  {
    // The a1 rook is there and the h1 rook is not, so WQ survives WK.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/R3K3 w KQ - 0 1", &game));

    CHECK_EQ(game.board.castling, WQ);
    CHECK_EQ(count_flagged(&game, true, false), 1);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "the same holds for Black")
  {
    // Black claims both; only the h8 rook exists, so BK survives BQ.
    REQUIRE(load_FEN("4k2r/8/8/8/8/8/8/4K3 b kq - 0 1", &game));

    CHECK_EQ(game.board.castling, BK);
    CHECK_EQ(count_flagged(&game, true, false), 1);
  }

  // RE-PICKED by S223 (DEC-197). This case loaded
  // `4k3/8/8/8/8/8/8/R6R w KQ - 0 1` -- both white rooks in place and no white
  // king at all -- and checked that the rights were cleared. python-chess
  // reports NO_WHITE_KING on it, and the load boundary refuses such a placement
  // from S223 on, so the premise is gone and the case states the refusal.
  // The property it held is not lost: "castling rights with the king displaced
  // license no castle" above now asserts the clearing on a **legal** position
  // whose king is off e1, which is the same rule with a king on the board.
  TEST_CASE_FIXTURE(fen_fixture_t, "a right with no king anywhere is refused")
  {
    std::string reason;
    CHECK_FALSE(load_FEN("4k3/8/8/8/8/8/8/R6R w KQ - 0 1", &game, &reason));
    CHECK(reason.find("one king of each colour") != std::string::npos);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "an ep square on the mover's own rank goes")
  {
    // ep = d3 with White to move: rank 3 is the rank *Black* captures on, so
    // this is White being offered its own double push back. The victim index
    // for White would be d2, and there is a white pawn there -- which is how
    // an unsanitized load turns this into a capture of one's own pawn.
    REQUIRE(load_FEN("4k3/8/8/8/2p5/8/3P4/4K3 w - d3 0 1", &game));

    CHECK_EQ(game.board.en_passant, INVALID_INDEX);
    CHECK_EQ(count_flagged(&game, false, true), 0);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "an occupied ep target square goes")
  {
    // Victim present on e5 and a white knight standing on the target e6: the
    // capture's move_piece() would xor a pawn onto a square already occupied.
    //
    // RE-PICKED by S223 (DEC-197): the occupant was a **rook**, which from e6
    // checks the black king on e8 with White to move -- python-chess reports
    // OPPOSITE_CHECK -- and the load boundary refuses that from S223 on. A
    // knight on e6 occupies the same square and attacks d8 and f8 rather than
    // e8, so the property under test is untouched: python-chess reports
    // INVALID_EP_SQUARE and nothing else, which is the case's own subject.
    REQUIRE(load_FEN("4k3/8/4N3/3Pp3/8/8/8/4K3 w - e6 0 1", &game));

    CHECK_EQ(game.board.en_passant, INVALID_INDEX);
    CHECK_EQ(count_flagged(&game, false, true), 0);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "a cleared field is cleared in the hash")
  {
    // The rights and the ep square are Zobrist inputs, so sanitizing after the
    // fields are parsed but before compute_full_hash() is what keeps a
    // sanitized position hashing as the position it actually is. Same board,
    // one FEN carrying a right the board cannot support.
    game_t plain = {};
    initialize_game_const_data(&plain);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w K - 0 1", &game));
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &plain));

    CHECK_EQ(game.board.castling, plain.board.castling);
    CHECK_EQ(game.board.hash, plain.board.hash);
  }
}


// AUDIT EVIDENCE -- 2026-09-10_adversarial-F09 and -F10, added by S208. The
// same file because the finding is the same one S161 left open in its own
// words: *"Position legality at large -- pawn counts, two kings, the side not
// to move in check -- is deliberately not checked: only the two classes that
// corrupt."* These are a third and a fourth class that corrupt, and unlike
// S161's two they are **refused** rather than repaired, because neither can be
// repaired without inventing a position.
//
// What each did before the fix, observed on `23f926d`:
//
//   F09  `printf 'position fen QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - -
//        0 1\ngo depth 4\nquit\n' | ./build/src/chesso`
//        -> `*** stack smashing detected ***: terminated`, **exit 134**, core
//        dumped. The Release binary that ships and that every SPRT measures.
//        27 white pieces generate 277 moves into a move_t[270] on the stack.
//   F10  the same shape with `P7/8/8/8/8/8/8/K6k w - - 0 1` and
//        `K6k/8/8/8/8/8/8/p7 b - - 0 1` under `build-sanitize`
//        -> `src/evaluation.cpp:516:36: runtime error: index 6 out of bounds
//        for type 'int [6]'` and the same at `:529:36` for Black.
//
// The F09 red cannot be a ctest red: the abort takes the whole test binary
// with it. So the case below asserts the refusal, and the crash is the
// observation recorded in S208's stamp and in the two lines above. The F10
// cases are ordinary reds -- before the fix `load_FEN` returned true.
TEST_SUITE(
    "audit: FEN piece counts and back-rank pawns "
    "(2026-09-10_adversarial-F09, -F10)")
{
  TEST_CASE_FIXTURE(fen_fixture_t, "more than 16 pieces of a colour is refused")
  {
    // The F09 placement: 26 queens and a king, which loaded cleanly before
    // S208 and then overran the move buffer from the search.
    std::string reason;
    CHECK_FALSE(load_FEN("QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1",
                         &game, &reason));
    CHECK(reason.find("16 pieces") != std::string::npos);

    // The count is in the reason, so a harness reading the channel learns
    // which side was wrong and by how much.
    CHECK(reason.find("27 white") != std::string::npos);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "seventeen of one colour is the boundary")
  {
    // One over, and only on one side: the rule is per colour, not per board.
    // The start position plus a seventeenth white piece, so Black sits exactly
    // on the boundary while White is one past it -- which is what a
    // board-total bound rather than a per-colour one would let through.
    std::string reason;
    CHECK_FALSE(
        load_FEN("rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/RNBQKBNR w KQkq"
                 " - 0 1",
                 &game, &reason));
    CHECK(reason.find("17 white") != std::string::npos);
    CHECK(reason.find("16 black") != std::string::npos);
  }

  // Both ends of both back ranks, and the far corners are not decoration: the
  // rule is `square < 8 || square >= 56` over a board indexed a8 = 0, so an
  // off-by-one at either comparison leaks exactly one corner and nothing else.
  // Mutating `square < 8` to `square < 7` loads a pawn on **h8** and leaves the
  // whole fast suite green when only a8 is tested -- observed on `8aff8ac`,
  // 33/33 -- because no FEN anywhere in src/, tests/, tools/, adocs/ or books/
  // has a pawn on h8. Found by the Tier-1 fast check over that commit.
  // `tools/mutants/board.py` M37 is that mutant and this case is what kills it.
  TEST_CASE_FIXTURE(fen_fixture_t, "a pawn on either end of rank 8 is refused")
  {
    const char* cases[] = {
        "P7/8/8/8/8/8/8/K6k w - - 0 1",  // a8, white
        "7P/8/8/8/8/8/8/K6k w - - 0 1",  // h8, white -- the leaked corner
        "p7/8/8/8/8/8/8/K6k w - - 0 1",  // a8, black
        "7p/8/8/8/8/8/8/K6k w - - 0 1",  // h8, black
    };

    for (const char* fen : cases) {
      std::string reason;
      CHECK_FALSE_MESSAGE(load_FEN(fen, &game, &reason), fen);
      CHECK_MESSAGE(reason.find("rank 1 or rank 8") != std::string::npos, fen);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "a pawn on either end of rank 1 is refused")
  {
    // The mirror, because evaluate_pawns() computes the bucket separately per
    // colour and the sanitizer reported both sites. `>= 56` is the comparison
    // here and `>= 57` is its off-by-one, which leaks a1.
    const char* cases[] = {
        "K6k/8/8/8/8/8/8/p7 b - - 0 1",  // a1, black
        "K6k/8/8/8/8/8/8/7p b - - 0 1",  // h1, black
        "K6k/8/8/8/8/8/8/P7 b - - 0 1",  // a1, white
        "K6k/8/8/8/8/8/8/7P b - - 0 1",  // h1, white
    };

    for (const char* fen : cases) {
      std::string reason;
      CHECK_FALSE_MESSAGE(load_FEN(fen, &game, &reason), fen);
      CHECK_MESSAGE(reason.find("rank 1 or rank 8") != std::string::npos, fen);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "a refusal names no reason on bad syntax")
  {
    // The two classes above are the only ones that fill `reason` in; a syntax
    // failure leaves it empty, which is what lets set_position() say "does not
    // load" for everything else without a second code path.
    std::string reason = "sentinel";
    CHECK_FALSE(load_FEN("not a fen at all", &game, &reason));
    CHECK_EQ(reason, "sentinel");
  }

  // The boundary from the other side. Each of these was legal before S208 and
  // has to stay legal, or the refusal has eaten ordinary positions -- which is
  // the failure mode a bound like this actually has.
  TEST_CASE_FIXTURE(fen_fixture_t, "sixteen a side still loads")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    size_t white = 0;
    size_t black = 0;

    for (index_t square = 0; square < 64; ++square) {
      const piece_t piece = game.board.squares[square];

      if (piece == EMPTY) { continue; }

      if (piece <= W_KING) {
        white++;
      } else {
        black++;
      }
    }

    // The precondition, so this case cannot pass on a board that happens to
    // have fewer: the start position is exactly the boundary on both sides.
    CHECK_EQ(white, 16);
    CHECK_EQ(black, 16);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "pawns on ranks 2 and 7 still load")
  {
    // One rank inside the refused rank on each side. This is a control and not
    // the boundary probe it was first written as: a7 is index 8 and a2 is 48,
    // so only the `< 8` comparison is one step away from a7, and the square
    // where a `>= 56` off-by-one would show is h2 at 55 -- which
    // DEFAULT_POSITION and KIWIPETE_POS below already cover, having pawns on
    // every file of rank 2. The corners are pinned by the two cases above.
    REQUIRE(load_FEN("8/P6P/8/8/8/8/p6p/K6k w - - 0 1", &game));

    CHECK_EQ(game.board.squares[a7], W_PAWN);
    CHECK_EQ(game.board.squares[h7], W_PAWN);
    CHECK_EQ(game.board.squares[a2], B_PAWN);
    CHECK_EQ(game.board.squares[h2], B_PAWN);
  }

  // RE-STATED, not relaxed, by S223 (DEC-197). This case read "the kingless
  // debug positions still load" and required `EMPTY_POS` and
  // `8/3p4/8/8/8/8/3P4/8 w - - 0 1` -- S208's excludes kept one king a side out
  // of scope and these were the inputs that made it worth keeping out. S223
  // closes that boundary, so their premise is gone and they are refusals in the
  // suite below; `EMPTY_POS` itself is gone with the `position empty` shortcut
  // that was its only caller. What this case was actually holding is the other
  // half -- that S208's bound did not eat the debug positions it was measured
  // on -- and that half is unchanged and asserted here.
  TEST_CASE_FIXTURE(fen_fixture_t, "the debug positions still load")
  {
    REQUIRE(load_FEN(KIWIPETE_POS, &game));
    REQUIRE(load_FEN(KILLER_POS, &game));
    REQUIRE(load_FEN(BLOCKED_CENTRE_POS, &game));
    REQUIRE(load_FEN(FINE_70_POS, &game));
    REQUIRE(load_FEN(THREE_FOLD_REP_POS, &game));
    REQUIRE(load_FEN(THREE_FOLD_REP_2_POS, &game));
    REQUIRE(load_FEN(MATE_IN_2_W_POS, &game));
    REQUIRE(load_FEN(MATE_IN_2_B_POS, &game));
    REQUIRE(load_FEN(CLOSED_POSITION, &game));
  }
}


// AUDIT EVIDENCE -- 2026-09-12_adversarial-F01, added by S223. The same file
// again, and for the same reason S208 used it: this is the rest of the sentence
// S161 wrote and S208 quoted -- *"two kings, the side not to move in check --
// is deliberately not checked"*. Those are the fifth and sixth classes, and
// like S208's two they are **refused** rather than repaired.
//
// What it did before the fix, observed by hand on `ab5cd8e`'s Release binary,
// because the board it leaves is not something a ctest case can watch the UCI
// layer produce before the layer refuses it:
//
//   printf 'position fen 7k/8/8/8/8/8/8/K6R w - - 0 1 moves h1h8\nfen\nquit\n'
//        | ./build/src/chesso
//   -> 7R/8/8/8/8/8/8/K7 b - - 0 1
//
// The black king has been captured by a move the engine accepted, and no line
// was printed about it. The same binary loaded `8/3p4/8/8/8/8/3P4/8 w - - 0 1`
// and `4K3/8/8/8/8/8/8/K6k w - - 0 1` -- no kings at all, and two white kings
// -- and echoed both back from the `fen` command.
//
// **Every legality verdict below is python-chess 1.11.2's**, not a reading of
// the board (CLAUDE.md). `Board.status()` over each FEN here is quoted at its
// case, and `adocs/data/S223_fen_census.py` is the same question asked of every
// FEN literal in `tests/`, `src/` and `tools/` at once.
TEST_SUITE("audit: FEN king legality (2026-09-12_adversarial-F01)")
{
  TEST_CASE_FIXTURE(fen_fixture_t, "a board with no kings at all is refused")
  {
    // python-chess: NO_WHITE_KING,NO_BLACK_KING. The S208 control above used
    // to load this one on purpose.
    std::string reason;
    CHECK_FALSE(load_FEN("8/3p4/8/8/8/8/3P4/8 w - - 0 1", &game, &reason));
    CHECK(reason.find("one king of each colour") != std::string::npos);

    // The count is in the reason for the same purpose S208's carries one: a
    // harness reading the channel learns which side was wrong.
    CHECK(reason.find("0 white") != std::string::npos);
    CHECK(reason.find("0 black") != std::string::npos);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "one king missing on either side is refused")
  {
    // Both directions, because the count is per colour: a bound that only
    // looked at the board's total would pass both of these.
    // python-chess: NO_WHITE_KING and NO_BLACK_KING respectively.
    std::string white_missing;
    CHECK_FALSE(load_FEN("4k3/8/8/8/8/8/8/8 w - - 0 1", &game, &white_missing));
    CHECK(white_missing.find("0 white") != std::string::npos);
    CHECK(white_missing.find("1 black") != std::string::npos);

    std::string black_missing;
    CHECK_FALSE(load_FEN("8/8/8/8/8/8/8/4K3 w - - 0 1", &game, &black_missing));
    CHECK(black_missing.find("1 white") != std::string::npos);
    CHECK(black_missing.find("0 black") != std::string::npos);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "two kings of a colour are refused")
  {
    // python-chess: TOO_MANY_KINGS. Two white kings and one black one, so the
    // refusal cannot be the missing-king branch firing by accident.
    std::string reason;
    CHECK_FALSE(load_FEN("4K3/8/8/8/8/8/8/K6k w - - 0 1", &game, &reason));
    CHECK(reason.find("one king of each colour") != std::string::npos);
    CHECK(reason.find("2 white") != std::string::npos);
    CHECK(reason.find("1 black") != std::string::npos);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "the side not to move in check is refused")
  {
    // The finding's own position: White to move with the h1 rook bearing on
    // h8, so `moves h1h8` captured the black king. python-chess:
    // OPPOSITE_CHECK.
    std::string reason;
    CHECK_FALSE(load_FEN("7k/8/8/8/8/8/8/K6R w - - 0 1", &game, &reason));
    CHECK(reason.find("side not to move is in check") != std::string::npos);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "two adjacent kings are refused")
  {
    // Not a separate rule and not a separate branch: each king attacks the
    // other, so whichever side is not to move is in check and the same test
    // catches it. python-chess: OPPOSITE_CHECK.
    std::string reason;
    CHECK_FALSE(load_FEN("8/8/8/3kK3/8/8/8/8 w - - 0 1", &game, &reason));
    CHECK(reason.find("side not to move is in check") != std::string::npos);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "S208's two reasons still fire first")
  {
    // Order, and it is load-bearing twice over. The F09 placement carries one
    // king a side and would reach the check test with 26 queens on the board;
    // the back-rank-pawn fixture has no black king at all, so the king count
    // would answer for it if it ran first. Both reasons are pinned by the S208
    // cases above, and this is where the ordering that keeps them pinned is
    // stated.
    std::string queens;
    CHECK_FALSE(load_FEN("QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1",
                         &game, &queens));
    CHECK(queens.find("16 pieces") != std::string::npos);

    std::string back_rank;
    CHECK_FALSE(load_FEN("pppppppp/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                         &game, &back_rank));
    CHECK(back_rank.find("rank 1 or rank 8") != std::string::npos);
  }

  // The boundary from the other side, as S208's controls are: a bound whose
  // failure mode is eating ordinary positions.
  TEST_CASE_FIXTURE(fen_fixture_t, "the start position still loads")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    // The precondition, so the case cannot pass on a board that has no kings
    // to count: one of each is exactly what the rule requires.
    CHECK_EQ(count_bits(game.board.bitboards[W_KING]), 1);
    CHECK_EQ(count_bits(game.board.bitboards[B_KING]), 1);
  }

  TEST_CASE_FIXTURE(fen_fixture_t,
                    "a position whose side to move is in check still loads")
  {
    // The half of the check test that must not fire. White is to move and
    // White is in check from the h1 rook, which is an ordinary position every
    // GUI sends; python-chess reports Status.VALID and is_check() True.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/R3K2r w Q - 0 1", &game));

    // Non-vacuous: the position really is a check, so a test that refused
    // every checked position would fail here rather than pass on a quiet
    // board.
    CHECK(is_check(&game));
  }

  // The finding end to end, on the command it arrived through. Everything above
  // is load_FEN(); this is `position fen`, which is what a GUI or a harness
  // drives, and the S176 contract that the refusal leaves the engine exactly
  // where it was.
  TEST_CASE("[position fen] refuses the king capture and keeps the board")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position fen " + std::string(KIWIPETE_POS));
    }

    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(KIWIPETE_POS));

    std::string replies;

    {
      stdout_capture_t capture;
      uci_process_line("position fen 7k/8/8/8/8/8/8/K6R w - - 0 1 moves h1h8");
      replies = capture.str();
    }

    CHECK_MESSAGE(
        replies.find("info string refused [position fen] "
                     "7k/8/8/8/8/8/8/K6R w - - 0 1, "
                     "the side not to move is in check") != std::string::npos,
        replies);

    // The whole command is atomic (S210): the board, and with it the move that
    // would have taken the black king off it, is the one the engine had.
    CHECK_EQ(generate_FEN(&uci_game()->board), std::string(KIWIPETE_POS));

    uci_shutdown();
  }
}
