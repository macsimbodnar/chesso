#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "eval_tables.hpp"
#include "evaluation.hpp"
#include "search_params.hpp"
#include "test_helpers.hpp"


static game_t game;
static game_t mirrored;


// A board as 64 characters in board-index order -- 0 is a8 and 63 is h1, which
// is the order a FEN writes its ranks -- turned into a FEN with the four
// trailing fields at their neutral settings: no castling rights, no en passant
// square, and two clocks a load will accept. '\0' is an empty square.
//
// Only the placement and the side to move are varied by the caller, because
// those are the two fields evaluate() reads.
static std::string placement_to_fen(const std::array<char, 64>& squares,
                                    char side_to_move)
{
  std::string fen;
  int empty = 0;

  for (index_t square = 0; square < 64; ++square) {
    if (squares[square] == '\0') {
      empty++;
    } else {
      if (empty > 0) {
        fen += static_cast<char>('0' + empty);
        empty = 0;
      }

      fen += squares[square];
    }

    if ((square % 8) == 7) {
      if (empty > 0) {
        fen += static_cast<char>('0' + empty);
        empty = 0;
      }

      if (square != 63) { fen += '/'; }
    }
  }

  fen += ' ';
  fen += side_to_move;
  fen += " - - 0 1";

  return fen;
}


struct eval_fixture_t
{
  eval_fixture_t()
  {
    initialize_game_const_data(&game);
    initialize_game_const_data(&mirrored);
  }
};


// NOTE ON STYLE
//
// These are written as invariants - symmetry and ordering - rather than as
// expected score values, so that they keep their meaning when evaluate() grows
// piece-square tables or is replaced by a network. The only two assertions
// pinned to today's material-only numbers are marked ANCHOR and have to be
// revisited whenever the evaluation changes.


TEST_SUITE("evaluation: score")
{
  // search.cpp's `#define MATE_MIN 48000`, which is not exported. Pinned here
  // rather than shared, the way tests/test_search.cpp pins this bound and
  // MATE_MAX beside it, so that moving it is a visible disagreement instead of
  // a silent agreement.
  static constexpr int MATE_MIN_LOCAL = 48000;

  // The strongest property an evaluation has: mirroring the board and swapping
  // both colours must negate the score exactly. A one-sided term, a wrong
  // sign, or a table indexed from the wrong side all show up here.
  TEST_CASE_FIXTURE(eval_fixture_t, "colour symmetry over every test position")
  {
    size_t checked = 0;

    for (const std::string& fen : all_test_fens()) {
      const std::string flipped = mirror_fen(fen);

      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));
      REQUIRE_MESSAGE(load_FEN(flipped, &mirrored),
                      ("mirrored FEN: " + flipped + " from " + fen));

      const int score = evaluate(&game.board);
      const int mirrored_score = evaluate(&mirrored.board);

      // evaluate() answers from the side to move's point of view, and
      // mirror_fen() swaps the side to move along with the colours. The two
      // therefore agree rather than negate: the same player, looking at the
      // same position, has to reach the same number.
      REQUIRE_MESSAGE(score == mirrored_score,
                      ("FEN: " + fen + "\nmirror: " + flipped));
      checked++;
    }

    REQUIRE(checked > 100);
  }

  // Both cases below asserted exactly 0 until S027 added the tempo term, and
  // that number stopped being the right one the moment the engine started
  // believing the move is worth something. The start position is symmetric in
  // material, in structure and in every square either side stands on -- but not
  // in whose turn it is, so a term about the turn is precisely the term that
  // does not cancel here.
  //
  // What the pair was protecting is that *nothing else* separates the two
  // sides, and that is what they assert now: each side, on move, gets the same
  // number, and that number is the tempo bonus alone. Re-targeted rather than
  // deleted or special-cased -- the assertion is still exact, and a term that
  // unbalanced a symmetric position would fail it as loudly as before.
  //
  // With a full set of pieces the phase is GAME_PHASE_MAX, so the taper is the
  // middlegame weight exactly and no arithmetic is restated here. The phase is
  // asserted rather than assumed, because if it were anything else the expected
  // value would be wrong in a way that reads as right.
  TEST_CASE_FIXTURE(eval_fixture_t, "the start position is balanced")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    REQUIRE_EQ(game_phase(&game.board), GAME_PHASE_MAX);
    REQUIRE_EQ(evaluate(&game.board), tempo_mg);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a mirrored start position is balanced")
  {
    // Same material, black to move. Still symmetric, so still worth exactly
    // what it was worth to White above: the two cases together are the
    // property, that a symmetric position is the same number to whoever is on
    // move.
    REQUIRE(load_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
                     &game));
    REQUIRE_EQ(game_phase(&game.board), GAME_PHASE_MAX);
    REQUIRE_EQ(evaluate(&game.board), tempo_mg);
  }

  TEST_CASE_FIXTURE(eval_fixture_t,
                    "score is from the side to move's point of view")
  {
    // The same position, read by each side in turn. White is a whole queen up,
    // so it is winning for White and losing for Black, and the number has to
    // change sign with the side to move rather than stay put.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 w - - 0 1", &game));
    const int white_to_move = evaluate(&game.board);
    const int phase = game_phase(&game.board);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 b - - 0 1", &game));
    const int black_to_move = evaluate(&game.board);

    REQUIRE(white_to_move > 0);
    REQUIRE(black_to_move < 0);

    // This was `white_to_move == -black_to_move` until S027 added the tempo
    // term, and that stopped being true of the evaluation rather than of this
    // position: every term that reads the board negates with the side to move,
    // and the bonus for having the move is added to whoever has it, so the two
    // scores are P + tempo and -P + tempo. Their *sum* is what the old equality
    // was really claiming, and it is what is claimed now -- everything that
    // reads the board cancels in it and twice the bonus is left.
    //
    // The queen alone is phase 4 of 24, so the taper is mostly the endgame
    // weight and neither weight can be read off this on its own.
    const int tempo =
        ((tempo_mg * phase) + (tempo_eg * (GAME_PHASE_MAX - phase))) /
        GAME_PHASE_MAX;

    REQUIRE_EQ(white_to_move + black_to_move, 2 * tempo);

    // And the mirror image, so a sign error that happens to be symmetric does
    // not slip through.
    REQUIRE(load_FEN("3qk3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE(evaluate(&game.board) < 0);

    REQUIRE(load_FEN("3qk3/8/8/8/8/8/8/4K3 b - - 0 1", &game));
    REQUIRE(evaluate(&game.board) > 0);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "removing a piece moves the score")
  {
    struct case_t
    {
      std::string with;
      std::string without;
      int material;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", "4k3/8/8/8/8/8/8/4K3 w - - 0 1", piece_value[4], "white queen"},
      {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1", "4k3/8/8/8/8/8/8/4K3 w - - 0 1", piece_value[3], "white rook"},
      {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1","4k3/8/8/8/8/8/8/4K3 w - - 0 1", piece_value[0], "white pawn"},
    };
    // clang-format on

    // Against the piece's own fitted value, not against a round number. This
    // used to require the delta to clear 100 and that was a proxy for "material
    // is priced": it passed while a pawn happened to evaluate above a hundred
    // and failed at 89 the moment S027's passed pawn term took some of it back,
    // which says nothing about whether material is priced.
    //
    // Bounding the delta on both sides is the stronger statement, and it is the
    // one the case is for. A piece has to be worth its material to within what
    // the positional terms can say about one square, so a term that quietly
    // doubled a queen fails here as loudly as one that zeroed it. The old form
    // caught neither.
    constexpr int POSITIONAL_ROOM = 150;

    for (const case_t& test : cases) {
      REQUIRE(load_FEN(test.with, &game));
      const int with = evaluate(&game.board);

      REQUIRE(load_FEN(test.without, &game));
      const int without = evaluate(&game.board);

      REQUIRE_MESSAGE(
          std::abs((with - without) - test.material) <= POSITIONAL_ROOM,
          test.title);
    }
  }

  // ANCHOR: pinned to today's numbers. Every value here is material plus what
  // the positional terms make of the one square the piece is standing on, so
  // editing any constant they read moves them and this test is what makes that
  // deliberate rather than accidental.
  //
  // It did its job at S028: the tuned tables moved all five and the suite said
  // so. It did it again at S034, when mobility became part of the score, and
  // again at S027 when the fitted king safety weights landed -- the pawn, the
  // rook and the queen moved and the knight, the bishop and the bare kings did
  // not. S027's passed pawn weights then moved the pawn alone, 104 to 89: it is
  // the only case here with a pawn on the board, that pawn is passed because
  // there are no enemy pawns anywhere, and at phase 0 it collects
  // passed_pawn_eg[0] = -15 and nothing else. The pawn structure weights then
  // moved the same case alone again, 89 to 88: that pawn is also isolated, with
  // no own pawn on the d or the f file, and it is not backward because nothing
  // attacks e3, so it collects pawn_structure_eg[isolated] = -1 once. Those
  // three are positions where king_safety_features() reports the same nine
  // counts for both colours, so the term cancels in the difference the score is
  // built from rather than being absent from it.
  //
  // The values below were recomputed by a second implementation of evaluate()
  // written for the purpose -- one that walks the rays by hand rather than
  // through the magic tables -- and not read off the engine, because an anchor
  // copied from the thing it anchors asserts nothing.
  //
  // S065's fit then moved five of the six at once, which is what a fit over 827
  // constants does. The queen is the one to read carefully: 1101 to 715 is not
  // a claim that she lost 386 centipawns of value. DEC-059 re-anchored her,
  // subtracting 432 from QUEEN and adding it back to all 128 of her squares,
  // which tuner.cpp:26-30 documents as the same evaluation -- six of these
  // seven positions came out identical either way and this one moved by one
  // centipawn, 716 to 715, on a truncation that crosses zero. A fitted material
  // value only means anything together with its own tables.
  //
  // S076's refit on the deduplicated corpus moved five of the six again -- 125,
  // 211, 279, 509, 715 to 135, 244, 325, 563, 787, the bare kings still
  // cancelling -- and every one of the five went **up** while `PAWN` fell by 1
  // and `BISHOP` rose by 2. That is the same degeneracy read from the other
  // side: what moved is the piece-square tables these pieces stand on, not the
  // material defines. Each value below was re-derived by the second
  // implementation, and matched the engine on all ten of its cases before any
  // of them was pasted here. Re-targeted, not relaxed: the assertion is the
  // same exact equality it always was.
  //
  // Symmetry and ordering say nothing about what a piece is actually worth:
  // every one of these values can be changed without moving any other
  // assertion in this file, and a wrong one costs games rather than crashes.
  //
  // GOLDEN (DEC-142): evaluate() of six one-piece positions, White to move, at
  // the shipped weights -- what one piece plus its tables is worth.
  // Re-derive: python3 adocs/data/S192_anchors.py. Moves legitimately on: a
  // refit; an evaluation term added or changed, which is a new field in every
  // case of the script and in its score(), written from src/evaluation.cpp's
  // prose and never by calling the engine.
  // Margin: exact. Property beside it: "the lazy shortcut cannot change a
  // decision", which holds over the whole corpus and does not move with a fit.
  TEST_CASE_FIXTURE(eval_fixture_t, "each piece is worth what the tables say")
  {
    struct case_t
    {
      std::string fen;
      int score;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", 135, "pawn on e2"},
      {"4k3/8/8/8/8/8/8/1N2K3 w - - 0 1", 244, "knight on b1"},
      {"4k3/8/8/8/8/8/8/2B1K3 w - - 0 1", 325, "bishop on c1"},
      {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1", 563, "rook on d1"},
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", 787, "queen on d1"},
      {"4k3/8/8/8/8/8/8/4K3 w - - 0 1",     0, "bare kings cancel"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);
      REQUIRE_MESSAGE(evaluate(&game.board) == test.score, test.title);
    }
  }

  // The lazy shortcut is sound only while the expensive terms cannot move the
  // score by more than LAZY_EVAL_MARGIN. That is a claim about every position,
  // not about the ten someone thought of, so it is asserted over the whole
  // corpus. S034.
  //
  // The margin is a bound on the sum of those terms and not on each of them,
  // which is why evaluate_expensive() clamps once over the total. Two terms
  // each bounded by the margin can correct by twice it between them, and the
  // shortcut is unsound the moment that happens.
  //
  // 150 was chosen from mobility's distribution alone -- 149084 self-play
  // positions, p99 81, maximum 143 -- before king safety existed. King safety
  // shares that budget now and its weights are fitted, so the number is no
  // longer the one it was chosen against: tools/eval_spread over all 1490839
  // positions reports the combined correction at p99 128, p99.9 178 and a
  // maximum of 330, clamped on 0.365 % of them against mobility's own 0.104 %.
  //
  // That is truncation, not unsoundness. The clamp is what makes the bound true
  // by construction, so the shortcut stays sound at any margin and what a small
  // one costs is evaluation accuracy on those positions. Moving it is its own
  // change with its own SPRT -- a larger margin truncates less and fires the
  // shortcut less often, which is two effects in opposite directions. S027.
  TEST_CASE_FIXTURE(eval_fixture_t,
                    "the lazy shortcut cannot change a decision")
  {
    size_t checked = 0;
    int worst = 0;

    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      const int full = evaluate(&game.board);
      const int cheap = evaluate_cheap(&game.board);
      const int correction = std::abs(full - cheap);

      if (correction > worst) { worst = correction; }

      // A window that contains the score: no shortcut, the exact number.
      REQUIRE_MESSAGE(
          evaluate_lazy(&game.board, full - 1000, full + 1000) == full,
          ("FEN: " + fen));

      // A beta the cheap score already clears by the margin: the shortcut
      // fires. The two things that have to hold of what comes back are that it
      // is still above beta, so the caller takes the same cutoff it would have
      // taken from the exact score, and that it is not above the exact score,
      // because fail-soft propagates it upward as a lower bound on it. Those
      // are the soundness claim; which number the shortcut picks to satisfy
      // them is not. `full >= beta` follows and is not asserted separately.
      const int beta = cheap - LAZY_EVAL_MARGIN;
      const int at_beta = evaluate_lazy(&game.board, beta - 1000, beta);

      REQUIRE_MESSAGE(at_beta >= beta,
                      ("FEN: " + fen + " returned " + std::to_string(at_beta) +
                       " at beta " + std::to_string(beta)));
      REQUIRE_MESSAGE(at_beta <= full,
                      ("FEN: " + fen + " returned " + std::to_string(at_beta) +
                       " against a true " + std::to_string(full)));

      // The same mirrored at alpha, where the number is an upper bound instead.
      const int alpha = cheap + LAZY_EVAL_MARGIN;
      const int at_alpha = evaluate_lazy(&game.board, alpha, alpha + 1000);

      REQUIRE_MESSAGE(at_alpha <= alpha,
                      ("FEN: " + fen + " returned " + std::to_string(at_alpha) +
                       " at alpha " + std::to_string(alpha)));
      REQUIRE_MESSAGE(at_alpha >= full,
                      ("FEN: " + fen + " returned " + std::to_string(at_alpha) +
                       " against a true " + std::to_string(full)));

      checked++;
    }

    REQUIRE(checked > 100);

    // Non-vacuous by construction: if the expensive terms never moved the score
    // at all, every assertion above would hold for a reason that has nothing to
    // do with the margin being right.
    REQUIRE(worst > 0);

    // The widest gap between the exact score and the cheap one over the set,
    // reported and not asserted. `correction <= LAZY_EVAL_MARGIN` used to stand
    // here as a REQUIRE and could not fail: evaluate_expensive() clamps stage
    // two to +/-LAZY_EVAL_MARGIN, so the bound is the clamp restated rather
    // than a property of the terms. The soundness claim is the four
    // evaluate_lazy() assertions above, which do read the shortcut's answers.
    // S193, 2026-09-04_test_review-F05.
    MESSAGE("widest correction over " << checked << " positions: " << worst
                                      << " (clamp " << LAZY_EVAL_MARGIN << ")");
  }

  // A caller that keeps the number instead of only comparing it has to know
  // which of the two it got, because a bound is true on one side of one window
  // and a stored number is read back from other windows. The transposition
  // entry's static evaluation is that caller. S094.
  TEST_CASE_FIXTURE(eval_fixture_t, "the shortcut says when it took one")
  {
    size_t checked = 0;
    size_t exact_runs = 0;
    size_t bound_runs = 0;

    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      const int full = evaluate(&game.board);
      const int cheap = evaluate_cheap(&game.board);

      // A window that contains the score. The flag says exact and the number
      // is the one evaluate() returns, which is what makes it worth storing.
      bool exact = false;
      const int inside =
          evaluate_lazy(&game.board, full - 1000, full + 1000, &exact);

      REQUIRE_MESSAGE(exact, ("FEN: " + fen));
      REQUIRE_MESSAGE(inside == full, ("FEN: " + fen));
      exact_runs++;

      // Both shortcut branches, and the flag has to be false on each. It is
      // initialised true here on purpose: a function that never wrote it would
      // pass on a variable that happened to start out right.
      const int beta = cheap - LAZY_EVAL_MARGIN;
      exact = true;
      evaluate_lazy(&game.board, beta - 1000, beta, &exact);
      REQUIRE_MESSAGE(!exact, ("FEN: " + fen + " at beta"));

      const int alpha = cheap + LAZY_EVAL_MARGIN;
      exact = true;
      evaluate_lazy(&game.board, alpha, alpha + 1000, &exact);
      REQUIRE_MESSAGE(!exact, ("FEN: " + fen + " at alpha"));
      bound_runs++;

      checked++;
    }

    REQUIRE(checked > 100);

    // Non-vacuous by construction: both answers have to occur, or the case
    // would pass on a flag stuck at either value.
    REQUIRE(exact_runs > 0);
    REQUIRE(bound_runs > 0);
  }

  // The case above is about the decision the caller takes. This one is about
  // the number it carries away, and a shortcut can be right about the first and
  // wrong about the second.
  //
  // The search is fail-soft: quiescence() does `if (stand_pat >= beta) { return
  // stand_pat; }`, so whatever the shortcut returned reaches the parent and the
  // transposition table as a lower bound on the true score, and a later search
  // reads it back as one. What is actually known at that branch is only
  // `full >= cheap - LAZY_EVAL_MARGIN`; the true score can be anywhere up to
  // `cheap + LAZY_EVAL_MARGIN`. Returning `cheap` therefore claims a bound up
  // to LAZY_EVAL_MARGIN centipawns better than anything the shortcut has
  // established -- 184 since S085, 150 before it, and stated symbolically here
  // so the prose cannot go stale behind the constant again, and
  // nothing downstream can tell the difference. Mirrored at alpha, where the
  // number is an upper bound and the error runs the other way. S027.
  TEST_CASE_FIXTURE(eval_fixture_t,
                    "a shortcut return is never better than the truth")
  {
    size_t checked = 0;
    size_t optimistic_at_beta = 0;
    size_t optimistic_at_alpha = 0;

    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      const int full = evaluate(&game.board);
      const int cheap = evaluate_cheap(&game.board);

      const int beta = cheap - LAZY_EVAL_MARGIN;
      const int lower = evaluate_lazy(&game.board, beta - 1000, beta);

      REQUIRE_MESSAGE(lower <= full,
                      ("FEN: " + fen + " claimed " + std::to_string(lower) +
                       " as a lower bound on " + std::to_string(full)));

      const int alpha = cheap + LAZY_EVAL_MARGIN;
      const int upper = evaluate_lazy(&game.board, alpha, alpha + 1000);

      REQUIRE_MESSAGE(upper >= full,
                      ("FEN: " + fen + " claimed " + std::to_string(upper) +
                       " as an upper bound on " + std::to_string(full)));

      // The positions where returning the cheap score was wrong rather than
      // merely unproven: the expensive terms move the score against the side of
      // the window being answered, so `cheap` sits above the truth at beta and
      // below it at alpha.
      if (full < cheap) { optimistic_at_beta++; }
      if (full > cheap) { optimistic_at_alpha++; }

      checked++;
    }

    REQUIRE(checked > 100);

    // Non-vacuous by construction, and on both sides separately. Every
    // assertion above holds trivially on a corpus where the expensive terms
    // never take the score the wrong way, and it was exactly such a corpus this
    // case was written to rule out -- the counts below are what make it fail on
    // the behaviour it exists to catch rather than merely pass on the fix.
    REQUIRE(optimistic_at_beta > 0);
    REQUIRE(optimistic_at_alpha > 0);
  }

  // evaluate_expensive_terms() is what tools/eval_spread measures the margin
  // from, and a number nobody can check is how a margin gets re-decided from
  // the wrong distribution. Clamping the pair it reports has to reproduce
  // exactly what evaluate() applied on top of evaluate_cheap(), which is the
  // one part of it the shipping path also computes. S034.
  TEST_CASE_FIXTURE(eval_fixture_t, "the unclamped terms are the engine's own")
  {
    size_t checked = 0;
    size_t clamped = 0;

    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      int mobility = 0;
      int safety = 0;
      evaluate_expensive_terms(&game.board, &mobility, &safety);

      const int applied = evaluate(&game.board) - evaluate_cheap(&game.board);
      const int raw = mobility + safety;

      if (std::abs(raw) > LAZY_EVAL_MARGIN) { clamped++; }

      REQUIRE_MESSAGE(
          std::clamp(raw, -LAZY_EVAL_MARGIN, LAZY_EVAL_MARGIN) == applied,
          ("FEN: " + fen + " raw " + std::to_string(raw) + " applied " +
           std::to_string(applied)));

      checked++;
    }

    REQUIRE(checked > 100);

    // Non-vacuous by construction. Every equality above would also hold for a
    // function that returned the clamped number, which is the one thing this
    // accessor must not do, unless some position in the corpus is outside the
    // margin. One is: the nine-bishop promotion pile-up S034 found.
    REQUIRE(clamped > 0);
  }

  // The piece-square tables have to actually prefer the squares they are meant
  // to. These are the two clearest cases and they pull in opposite directions,
  // so a table pasted in upside down fails one of them.
  TEST_CASE_FIXTURE(eval_fixture_t, "the tables prefer the right squares")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/N3K3 w - - 0 1", &game));
    const int knight_corner = evaluate(&game.board);

    REQUIRE(load_FEN("4k3/8/8/3N4/8/8/8/4K3 w - - 0 1", &game));
    const int knight_centre = evaluate(&game.board);

    REQUIRE_MESSAGE(knight_centre > knight_corner,
                    "a centralised knight must beat one in the corner");

    // With a full board the king belongs at home; with nothing left it belongs
    // in the middle. Same two squares, opposite verdicts. The middlegame case
    // needs an actual full board: with one queen each the phase is already 8 of
    // 24, which is mostly endgame and the tables correctly say so.
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const int home_mg = evaluate(&game.board);

    REQUIRE(load_FEN("rnbqkbnr/pppppppp/8/8/3K4/8/PPPPPPPP/RNBQ1BNR w kq - 0 1",
                     &game));
    const int centre_mg = evaluate(&game.board);

    REQUIRE_MESSAGE(home_mg > centre_mg,
                    "with a full board, the king is safer at home");

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    const int home_eg = evaluate(&game.board);

    REQUIRE(load_FEN("4k3/8/8/8/3K4/8/8/8 w - - 0 1", &game));
    const int centre_eg = evaluate(&game.board);

    REQUIRE_MESSAGE(centre_eg > home_eg,
                    "with the board bare, the king wants the centre");
  }

  // The king carries no material. Both sides always have exactly one in a legal
  // position, so the term could only ever cancel, and pricing it meant an
  // illegal position with an unbalanced king count scored above every mate -
  // which search() then had to guard against when deciding whether a result was
  // a mate at all.
  TEST_CASE_FIXTURE(eval_fixture_t, "a king is not worth anything")
  {
    // A bare king still moves the score, because it stands on a square the
    // positional terms have an opinion about, and since S027 they price its
    // shelter as well. What it must not do is carry material, so material is
    // what is asserted: `material` is the White-relative accumulator make_move
    // maintains and evaluate_cheap() reads, and a king given a piece value
    // would move it while no positional term can.
    //
    // The proxy this replaces was |evaluate()| under 100, standing in for a
    // pawn. It measured material plus every positional term, so it held only
    // while those were small: it failed at 102 once king safety started scoring
    // a bare king's three open files, which says nothing about what a king is
    // worth. Pinned to the accumulator, the assertion cannot be broken again by
    // a term that is not about material at all.
    // RE-PICKED by S223 (DEC-197): this was two loads, one lone black king and
    // one lone white king, and the load boundary refuses either from S223 on
    // (python-chess NO_WHITE_KING and NO_BLACK_KING). The two-king board
    // asserts the same thing about both kings at once -- a board whose only
    // pieces are kings has material 0 only if neither is priced -- and it is a
    // position a game really reaches. python-chess reports Status.VALID.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE_EQ(game.board.material, 0);

    // Non-vacuous by construction: the accumulator does move for a piece that
    // is priced, so a zero above is a king worth nothing rather than an
    // accumulator that never moves.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", &game));
    REQUIRE(game.board.material > 0);

    // The score has to stay well inside the mate band, or search() reports a
    // material imbalance as a mate. One position here; the case below is the
    // general statement.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 w - - 0 1", &game));
    REQUIRE(std::abs(evaluate(&game.board)) < MATE_MIN_LOCAL);
  }

  // `2026-09-10_adversarial-F27`, and the test the reverse futility argument
  // never had. That rule returns `static_eval - margin` as a lower bound on the
  // node, and a static score that reached the mate band would make it claim a
  // mate nothing proved. The comment at the site used to say the score
  // "provably cannot approach one, because evaluate_expensive() clamps the
  // whole king-safety correction to +/-LAZY_EVAL_MARGIN". That clamp bounds one
  // of the two stages. What bounds the other is the arithmetic of
  // evaluate_cheap(): material, the two tapered piece-square sums and the pawn
  // terms, every constant in them a two- or three-digit number and every sum
  // taken over at most sixteen men a side. The conclusion held; the reason
  // given did not establish it, and nothing asserted either. S213.
  //
  // Two populations, because they fail differently. The corpus is what ordinary
  // play looks like and would catch a term that runs away on a normal board.
  // The placements below are the other end of the range the load boundary
  // admits, where a piece value or a table entry multiplied by fifteen is the
  // thing that could reach 48000.
  TEST_CASE_FIXTURE(eval_fixture_t,
                    "the static score never reaches the mate band")
  {
    int corpus_max = 0;
    size_t corpus_checked = 0;

    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      const int score = std::abs(evaluate(&game.board));

      REQUIRE_MESSAGE(score < MATE_MIN_LOCAL, ("FEN: " + fen));

      corpus_max = std::max(corpus_max, score);
      corpus_checked++;
    }

    REQUIRE(corpus_checked > 100);

    // The pathological half. The rule is mechanical on purpose and is not a
    // chess one: squares come from a fixed seed, the piece mix is the largest
    // the refusals in load_FEN() leave standing -- sixteen men of a colour, one
    // king each, no pawn on rank 1 or rank 8 -- and whether a placement is a
    // position at all is load_FEN()'s answer rather than this loop's. Nothing
    // here is required to be reachable in a game; the point is the opposite,
    // that the bound holds on boards no game reaches.
    //
    // Family "maximal" is one side at sixteen men with fifteen of them queens
    // against a lone king, which is the largest material sum the boundary
    // permits and larger than promotions could ever deliver. Family "mixed"
    // draws both counts and every piece type, so the tables are exercised over
    // combinations the first family never produces.
    std::mt19937 rng(20260913U);

    int pathological_max = 0;
    size_t accepted = 0;
    size_t maximal_accepted = 0;
    std::string worst_fen;

    for (size_t trial = 0; trial < 80000; ++trial) {
      const bool maximal = (trial % 2) == 0;

      std::array<char, 64> squares{};
      squares.fill('\0');

      std::vector<index_t> order(64);
      for (index_t square = 0; square < 64; ++square) {
        order[square] = square;
      }
      std::shuffle(order.begin(), order.end(), rng);

      size_t next = 0;
      squares[order[next++]] = 'K';
      squares[order[next++]] = 'k';

      // The two men counts, kings included, so 16 is the cap the boundary
      // states.
      std::uniform_int_distribution<int> men(1, 16);
      const int white_men = maximal ? 16 : men(rng);
      const int black_men = maximal ? 1 : men(rng);

      // Queens only for "maximal"; the whole non-king set for "mixed". The
      // king is not in either list: a second one of a colour is refused.
      static const char white_mixed[] = {'P', 'N', 'B', 'R', 'Q'};
      std::uniform_int_distribution<size_t> pick(0, sizeof(white_mixed) - 1);

      const auto place = [&](int count, bool white) {
        for (int i = 1; i < count; ++i) {
          const index_t square = order[next++];
          char piece = maximal ? 'Q' : white_mixed[pick(rng)];

          // Rank 1 and rank 8 in board indices, where 0 is a8 and 63 is h1.
          const bool back_rank = (square / 8) == 0 || (square / 8) == 7;

          if (piece == 'P' && back_rank) { piece = 'Q'; }

          squares[square] = white ? piece
                                  : static_cast<char>(std::tolower(
                                        static_cast<unsigned char>(piece)));
        }
      };

      place(white_men, true);
      place(black_men, false);

      // Both sides to move are offered and load_FEN() takes whichever it will:
      // the boundary refuses a board where the side not to move stands in
      // check, and on a board this full that refuses most of one orientation.
      for (const char side : {'w', 'b'}) {
        const std::string fen = placement_to_fen(squares, side);

        if (!load_FEN(fen, &game)) { continue; }

        const int score = std::abs(evaluate(&game.board));

        REQUIRE_MESSAGE(score < MATE_MIN_LOCAL, ("FEN: " + fen));

        if (score > pathological_max) {
          pathological_max = score;
          worst_fen = fen;
        }

        accepted++;
        if (maximal) { maximal_accepted++; }
      }
    }

    // Non-vacuity, both halves. A boundary change that started refusing these
    // boards, or a generator that stopped producing one-sided ones, would
    // otherwise leave the case green while asserting nothing.
    REQUIRE_MESSAGE(accepted > 1000,
                    ("load_FEN accepted only " + std::to_string(accepted) +
                     " of the placements; the generator or the load boundary "
                     "moved and this case has stopped covering anything"));
    REQUIRE(maximal_accepted > 0);
    REQUIRE_MESSAGE(pathological_max > corpus_max,
                    "the placements are supposed to be the extreme end; they "
                    "are not separating from ordinary positions");

    MESSAGE("corpus: " << corpus_checked << " positions, max |evaluate()| "
                       << corpus_max << "; placements: " << accepted
                       << " accepted, max |evaluate()| " << pathological_max
                       << " on " << worst_fen << "; MATE_MIN "
                       << MATE_MIN_LOCAL);
  }
}


TEST_SUITE("evaluation: game phase")
{
  TEST_CASE_FIXTURE(eval_fixture_t, "runs from a full board down to bare kings")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    REQUIRE_EQ(game_phase(&game.board), GAME_PHASE_MAX);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE_EQ(game_phase(&game.board), 0);

    // Pawns are not part of it, so a pawn endgame is still phase 0.
    REQUIRE(load_FEN("4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", &game));
    REQUIRE_EQ(game_phase(&game.board), 0);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "weights the pieces the tables expect")
  {
    struct case_t
    {
      std::string fen;
      int phase;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/8/8/8/1N2K3 w - - 0 1", 1, "knight"},
      {"4k3/8/8/8/8/8/8/2B1K3 w - - 0 1", 1, "bishop"},
      {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1", 2, "rook"},
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", 4, "queen"},
      {"3qk3/8/8/8/8/8/8/3QK3 w - - 0 1", 8, "a queen each"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);
      REQUIRE_MESSAGE(game_phase(&game.board) == test.phase, test.title);
    }
  }

  // Promotions can put more material on the board than the opening had, and a
  // tapered term that interpolates on an out-of-range phase reads off the end
  // of its own tables.
  TEST_CASE_FIXTURE(eval_fixture_t, "never exceeds the maximum")
  {
    REQUIRE(load_FEN("qqqqkqqq/qqqqqqqq/8/8/8/8/QQQQQQQQ/QQQQKQQQ w - - 0 1",
                     &game));
    REQUIRE_EQ(game_phase(&game.board), GAME_PHASE_MAX);
  }
}


TEST_SUITE("evaluation: capture_score")
{
  // Finds the move matching from/to in the current position, so the tests can
  // name moves without hand-encoding move_t.
  static move_t find_move(game_t * g, index_t from, index_t to)
  {
    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(g, moves);

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_FROM(moves[i]) == from && MOVE_TO(moves[i]) == to) {
        return moves[i];
      }
    }

    return 0;
  }

  // One position, one victim, two attackers. Comparing two positions with
  // different victims instead would be satisfied by a score that ignores the
  // attacker entirely, which is exactly the half of MVV-LVA under test here.
  TEST_CASE_FIXTURE(eval_fixture_t, "MVV-LVA prefers a cheap attacker")
  {
    // The rook on d5 is attacked by the pawn on c4 and by the queen on d1.
    REQUIRE(load_FEN("4k3/8/8/3r4/2P5/8/8/3QK3 w - - 0 1", &game));

    const move_t pawn_takes = find_move(&game, c4, d5);
    const move_t queen_takes = find_move(&game, d1, d5);

    REQUIRE(pawn_takes != 0);
    REQUIRE(queen_takes != 0);
    REQUIRE(MOVE_PIECE(pawn_takes) == W_PAWN);
    REQUIRE(MOVE_PIECE(queen_takes) == W_QUEEN);

    REQUIRE(capture_score(&game.board, pawn_takes) >
            capture_score(&game.board, queen_takes));
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a richer victim scores higher")
  {
    // Same attacker, different victim: rook must beat knight.
    REQUIRE(load_FEN("4k3/8/8/3r4/4P3/8/8/4K3 w - - 0 1", &game));
    const int pxr = capture_score(&game.board, find_move(&game, e4, d5));

    REQUIRE(load_FEN("4k3/8/8/3n4/4P3/8/8/4K3 w - - 0 1", &game));
    const int pxn = capture_score(&game.board, find_move(&game, e4, d5));

    REQUIRE(pxr > pxn);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "en passant victim is priced as a pawn")
  {
    // The captured pawn does not sit on the target square, so capture_score
    // has a dedicated branch for it. It must agree with an ordinary pawn take.
    REQUIRE(load_FEN("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", &game));
    const move_t en_passant = find_move(&game, e5, d6);
    REQUIRE(en_passant != 0);
    REQUIRE(MOVE_EN_PASSANT(en_passant));
    const int ep_score = capture_score(&game.board, en_passant);

    REQUIRE(load_FEN("4k3/8/3p4/4P3/8/8/8/4K3 w - - 0 1", &game));
    const move_t ordinary = find_move(&game, e5, d6);
    REQUIRE(ordinary != 0);
    const int ordinary_score = capture_score(&game.board, ordinary);

    REQUIRE_EQ(ep_score, ordinary_score);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a quiet move ranks below every capture")
  {
    REQUIRE(load_FEN("4k3/8/8/3q4/4P3/8/8/4K3 w - - 0 1", &game));

    const int capture = capture_score(&game.board, find_move(&game, e4, d5));
    const int quiet = capture_score(&game.board, find_move(&game, e1, d1));

    REQUIRE(capture > quiet);
  }
}


TEST_SUITE("evaluation: score_move ordering")
{
  // The ordering bands are private to evaluation.cpp, so these assert the
  // relative order rather than the constants. That is what the search relies
  // on anyway.
  TEST_CASE_FIXTURE(eval_fixture_t, "bands are strictly ordered")
  {
    REQUIRE(
        load_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/"
                 "R3K2R w KQkq - 0 1",
                 &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);
    REQUIRE(count > 8);

    move_t a_capture = 0;
    move_t quiets[4] = {};
    size_t quiet_count = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_CAPTURE(moves[i]) && a_capture == 0) {
        a_capture = moves[i];
      } else if (!MOVE_CAPTURE(moves[i]) &&
                 MOVE_PROMOTED(moves[i]) == TO_NONE && quiet_count < 4) {
        quiets[quiet_count++] = moves[i];
      }
    }

    REQUIRE(a_capture != 0);
    REQUIRE_EQ(quiet_count, 4);

    const size_t ply = 3;

    search_state_t state = {};
    state.killer_moves[0][ply] = quiets[0];
    state.killer_moves[1][ply] = quiets[1];

    // The counter move is keyed on the previous move, so it needs one.
    const move_t prev_move = quiets[3];
    state.counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)] = quiets[2];

    // A plain quiet move only carries its history score.
    const move_t plain = quiets[3];
    state.quiet_history[game.board.active_color][MOVE_FROM(plain)]
                       [MOVE_TO(plain)] = 42;

    const move_t tt_move = a_capture;

    const int s_tt =
        score_move(&game, &state, tt_move, tt_move, ply, prev_move);
    const int s_capture =
        score_move(&game, &state, a_capture, 0, ply, prev_move);
    const int s_killer0 =
        score_move(&game, &state, quiets[0], 0, ply, prev_move);
    const int s_killer1 =
        score_move(&game, &state, quiets[1], 0, ply, prev_move);
    const int s_counter =
        score_move(&game, &state, quiets[2], 0, ply, prev_move);
    const int s_history = score_move(&game, &state, plain, 0, ply, prev_move);

    REQUIRE(s_tt > s_capture);
    REQUIRE(s_capture > s_killer0);
    REQUIRE(s_killer0 > s_killer1);
    REQUIRE(s_killer1 > s_counter);
    REQUIRE(s_counter > s_history);
    REQUIRE_EQ(s_history, 42);
  }


  // S142. The case above orders the bands at one history value; this one orders
  // them at the two extreme values the parameter table admits, which is a
  // different question and the one `QuietHistoryMax`'s upper bound exists to
  // answer. It was `OrderHistoryMax` and read 899999 until S142 -- above the
  // countermove band and above the second killer slot, and settable over UCI on
  // the tune build -- so the sentence beside it ("from ever outranking a
  // killer") held for one of the two killers and for neither countermove.
  // CLAUDE.md lists this as a one-way door: the symptom of getting it wrong is
  // a strength regression, not a wrong node count, so nothing in the tree would
  // say so out loud.
  //
  // **Both edges since S093.** The malus makes a quiet's history negative, so
  // the band is the closed interval [-max, +max] and not [0, max]. Nothing in
  // score_move() returns a value below a quiet's today, which is exactly why
  // the floor is pinned here: S025 would put losing captures under this band,
  // and the arrival has to fail loudly rather than silently overlap the
  // malused half.
  //
  // The bound is read from search_param_info() rather than written here, so the
  // case follows the declared range instead of restating it, and the clearance
  // it demands is derived from the bands in this position rather than quoted
  // from the comment that states it. 2026-08-20_plan_review-F14.
  //
  // **Two tables since S222, and the widest band the ranges admit.** The quiet
  // score is plain history plus ContHistWeight per cent of a continuation
  // entry, each bounded by its own table's bound, so the band is no longer one
  // declared maximum but a sum -- and the weight is settable over UCI on the
  // tune build, which means the clearance has to hold at the *declared
  // maximum* of the weight and not only at the value this build compiled. Both
  // are asserted: the compiled band is what the driven entries have to produce
  // exactly, which is what catches a score_move() that stopped adding the
  // second term, and the widest band is what has to clear the countermove
  // band. The arithmetic at the ceilings is 32767 + 2000 * 32767 / 100 =
  // 688107 against 700000, a clearance of 11893.
  TEST_CASE_FIXTURE(eval_fixture_t,
                    "the declared history ceiling clears the band above it")
  {
    // A rook for quiet moves and a king that can take a pawn. The cheapest
    // capture there is is the dearest attacker taking the cheapest victim, and
    // the whole band layout is spaced by the gap between that capture and the
    // first killer -- so the position has to contain one, or the 100 below is a
    // number copied out of a comment.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/4p3/R3K3 w - - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    move_t king_takes_pawn = 0;
    move_t quiets[4] = {};
    size_t quiet_count = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_CAPTURE(moves[i])) {
        if (MOVE_PIECE(moves[i]) == W_KING) { king_takes_pawn = moves[i]; }
      } else if (MOVE_PROMOTED(moves[i]) == TO_NONE && quiet_count < 4) {
        quiets[quiet_count++] = moves[i];
      }
    }

    REQUIRE(king_takes_pawn != 0);
    REQUIRE_EQ(quiet_count, 4);

    int declared_max = -1;
    int declared_weight_max = -1;
    int live_weight = -1;

    for (size_t i = 0; i < search_param_count(); ++i) {
      if (std::string("QuietHistoryMax") == search_param_info(i).name) {
        declared_max = search_param_info(i).max_value;
      }

      if (std::string("ContHistWeight") == search_param_info(i).name) {
        declared_weight_max = search_param_info(i).max_value;
        live_weight = search_param_value(i);
      }
    }

    REQUIRE(declared_max > 0);
    REQUIRE(declared_weight_max > 0);
    REQUIRE(live_weight >= 0);

    // S222. The quiet band is a sum of two tables now, each with a bound of
    // its own, and the second one is weighted on the way in. These are the two
    // widths: what the band can be at the values this build compiled, and what
    // it can be at the widest the declared ranges admit -- which is the number
    // the clearance has to hold against, because the tune build can set the
    // weight anywhere inside its range and no value there may reach the band
    // above.
    const int live_span = declared_max + (live_weight * CONT_HIST_BOUND) / 100;
    const int widest_span =
        declared_max + (declared_weight_max * CONT_HIST_BOUND) / 100;

    REQUIRE(widest_span >= live_span);

    const size_t ply = 3;

    search_state_t state = {};
    state.killer_moves[0][ply] = quiets[0];
    state.killer_moves[1][ply] = quiets[1];

    const move_t prev_move = quiets[3];
    state.counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)] = quiets[2];

    // The declared ceiling and not the shipping default. history_gravity_update
    // holds every entry inside whatever QuietHistoryMax is set to, so the two
    // worst cases the range admits are an entry sitting exactly on each bound.
    const move_t plain = quiets[3];
    int16_t& entry = state.quiet_history[game.board.active_color]
                                        [MOVE_FROM(plain)][MOVE_TO(plain)];

    // Both tables at once, which is the worst case the sum admits and the only
    // one worth pinning: the continuation cell is the self-referencing one,
    // `plain` being the previous move here as well as the move being scored,
    // and CONT_HIST_BOUND is a definition rather than a range so the entry has
    // exactly one extreme to be driven to. S222.
    int16_t& cont_entry = continuation_entry(&state, prev_move, plain);

    entry = static_cast<int16_t>(declared_max);
    cont_entry = static_cast<int16_t>(CONT_HIST_BOUND);

    const int s_capture =
        score_move(&game, &state, king_takes_pawn, 0, ply, prev_move);
    const int s_killer0 =
        score_move(&game, &state, quiets[0], 0, ply, prev_move);
    const int s_killer1 =
        score_move(&game, &state, quiets[1], 0, ply, prev_move);
    const int s_counter =
        score_move(&game, &state, quiets[2], 0, ply, prev_move);
    const int s_history = score_move(&game, &state, plain, 0, ply, prev_move);

    entry = static_cast<int16_t>(-declared_max);
    cont_entry = static_cast<int16_t>(-CONT_HIST_BOUND);
    const int s_history_malused =
        score_move(&game, &state, plain, 0, ply, prev_move);

    // Precondition 1. The clearance the bands are built on, read off this
    // position: the cheapest capture stands exactly 100 above the first killer.
    // Everything below asks history for the same 100, so if this ever stops
    // being the spacing the case fails here rather than measuring against a
    // number the code no longer uses.
    REQUIRE_EQ(s_capture - s_killer0, 100);

    // Precondition 2. The countermove band is the band immediately above
    // history -- it is the last branch score_move() takes before returning the
    // raw table entry -- and it is below both killers. Without this the
    // clearance below could be measured against the wrong neighbour.
    REQUIRE(s_killer0 > s_killer1);
    REQUIRE(s_killer1 > s_counter);

    // Precondition 3. Both bounds are what reach the score, at both edges. A
    // history entry is returned unmodified, so a case that asserted the
    // clearance without this would pass on a score_move() that quietly capped
    // the value itself -- or that clamped the malused half back to zero, which
    // would make the floor assertion below vacuous. Since S222 it is also what
    // holds the second term in the sum: without it a score_move() that stopped
    // adding the continuation term would pass every clearance below, because
    // dropping a term only ever makes the band narrower.
    REQUIRE_EQ(s_history, live_span);
    REQUIRE_EQ(s_history_malused, -live_span);

    // And the sum has to be carried in `int`. Two entries at their own bounds
    // already exceed the type they are stored in, and the weight multiplies
    // one of them by up to twenty on top -- so an accumulation in int16_t
    // would wrap negative and every clearance below would pass for the wrong
    // reason. S222.
    CHECK(widest_span > 32767);

    CHECK_MESSAGE(
        s_counter - widest_span >= 100,
        ("The widest quiet band the declared ranges admit, QuietHistoryMax's " +
         std::to_string(declared_max) + " plus ContHistWeight's " +
         std::to_string(declared_weight_max) + " per cent of " +
         std::to_string(CONT_HIST_BOUND) + ", is " +
         std::to_string(widest_span) + " against the countermove band's " +
         std::to_string(s_counter) + ", a clearance of " +
         std::to_string(s_counter - widest_span) +
         " and not the 100 the ordering bands are spaced by."));

    CHECK_MESSAGE(
        s_counter - s_history >= 100,
        ("The quiet band this build compiled scores " +
         std::to_string(s_history) + " against the countermove band's " +
         std::to_string(s_counter) + ", a clearance of " +
         std::to_string(s_counter - s_history) +
         " and not the 100 the ordering bands are spaced by."));

    // The floor. Every branch score_move() can take other than the history one
    // returns a band constant or a capture score, and the lowest of those is
    // the countermove's -- so the malused half of the quiet band has to stand
    // clear of it too, and by the same 100. This is the edge that did not exist
    // before S093 and the edge S025 would arrive at.
    CHECK_MESSAGE(
        s_counter - s_history_malused >= 100,
        ("The malused edge of the quiet band scores " +
         std::to_string(s_history_malused) +
         " against the countermove band's " + std::to_string(s_counter) +
         ", a clearance of " + std::to_string(s_counter - s_history_malused) +
         " and not the 100 the ordering bands are spaced by."));

    // And nothing occupies the malused half. Every other band this position can
    // produce stands above the whole closed interval, so a maximally malused
    // quiet is the lowest score the ordering can hand out. A band added below
    // history fails here. Measured against the widest span the ranges admit and
    // not against the compiled one, for the reason the ceiling above is.
    const int lowest_other_band =
        std::min({s_capture, s_killer0, s_killer1, s_counter});

    CHECK_MESSAGE(
        lowest_other_band - widest_span >= 100,
        ("The lowest non-history band scores " +
         std::to_string(lowest_other_band) +
         ", which does not stand 100 clear of the whole quiet band [" +
         std::to_string(-widest_span) + ", " + std::to_string(widest_span) +
         "]."));
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a promotion outranks a plain quiet move")
  {
    REQUIRE(load_FEN("6k1/4P3/8/8/8/8/8/4K3 w - - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    move_t promotion = 0;
    move_t quiet = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_PROMOTED(moves[i]) == TO_QUEEN) { promotion = moves[i]; }
      if (MOVE_PROMOTED(moves[i]) == TO_NONE && quiet == 0) {
        quiet = moves[i];
      }
    }

    REQUIRE(promotion != 0);
    REQUIRE(quiet != 0);

    const search_state_t state = {};

    REQUIRE(score_move(&game, &state, promotion, 0, 0, 0) >
            score_move(&game, &state, quiet, 0, 0, 0));
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "queen promotion outranks knight promotion")
  {
    REQUIRE(load_FEN("6k1/4P3/8/8/8/8/8/4K3 w - - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    move_t to_queen = 0;
    move_t to_knight = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_PROMOTED(moves[i]) == TO_QUEEN) { to_queen = moves[i]; }
      if (MOVE_PROMOTED(moves[i]) == TO_KNIGHT) { to_knight = moves[i]; }
    }

    REQUIRE(to_queen != 0);
    REQUIRE(to_knight != 0);

    const search_state_t state = {};

    REQUIRE(score_move(&game, &state, to_queen, 0, 0, 0) >
            score_move(&game, &state, to_knight, 0, 0, 0));
  }
}
