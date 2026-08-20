#pragma once
// The curated FEN corpus the evaluation-model tests run over. S100.
//
// Moved out of tests/test_eval_model.cpp so that tests/test_tuner_gradient.cpp
// can run over the same list. It is not an arbitrary set of positions: nearly
// every line was added because some feature count was zero in every position
// that came before it, and "the positions exercise every count" in
// test_eval_model.cpp is the case that holds the coverage there. A gradient
// check has the same non-vacuity problem in a sharper form -- a finite
// difference on a parameter whose feature is zero in every row compares 0 with
// 0 and passes for free -- so it wants exactly this list and not a new one that
// would have to re-earn the coverage.
//
// Kept as one list with one owner: test_eval_model.cpp's non-vacuity cases are
// what stop a future edit here from quietly emptying a column.
#include <string>
#include <vector>

namespace test_eval_positions
{

inline const std::vector<std::string> positions = {
    // Opening, both sides to move.
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
    "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    // Middlegame, material imbalances and castled kings.
    "r1bq1rk1/pp2ppbp/2np1np1/8/2BNP3/2N1B3/PPP2PPP/R2Q1RK1 w - - 0 9",
    "r2q1rk1/pb1nbppp/1p2pn2/2pp4/2PP4/1PN1PN2/PB2BPPP/R2Q1RK1 b - - 2 10",
    "r4rk1/1bq2ppp/p2bpn2/1p6/3P4/P1NBPN2/1P3PPP/R2Q1RK1 w - - 0 15",
    "2r3k1/5ppp/p2q4/1p1Pn3/8/2P2Q2/PP3PPP/3R2K1 b - - 0 24",
    // Endgames down to bare pawns, where the tapering weight is at the other
    // end of its range.
    "8/5pk1/6p1/7p/5P1P/6P1/5K2/8 w - - 0 1",
    "8/8/4k3/8/8/3K4/4P3/8 b - - 0 1",
    "6k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1",
    "8/2k5/8/8/3B4/2K5/8/8 b - - 0 1",
    // Promotion, which is how the phase sum can exceed a full board and where
    // game_phase() clamps.
    "8/PPPPPPPP/8/2k5/2K5/8/pppppppp/8 w - - 0 1",
    // Lopsided material, so a wrong sign cannot cancel itself.
    "3qk3/8/8/8/8/8/8/3QK2R w K - 0 1",
    // King safety, added at S027 because the positions above leave most of the
    // term at zero: no knight and no rook ever bore on a king zone across all
    // thirteen, and seven of the nine counts only ever pointed one way. A count
    // that is zero everywhere is a count the engine and the model agree about
    // for free. "the positions exercise every count" is what says so out loud;
    // each line below is here to move one of the entries it checks.
    //
    // A knight and a rook on the black king, and the same position mirrored so
    // that the White-ahead direction is covered too.
    "r1bq1rk1/ppp2ppp/2n5/3p2N1/1b1P4/2N4R/PPP2PPP/R1BQ2K1 w - - 0 1",
    "r1bq2k1/ppp2ppp/2n4r/1B1p4/3P2n1/2N5/PPP2PPP/R1BQ1RK1 b - - 0 1",
    // A bishop and a queen on the white king, from a black king with three
    // open files and no shelter of its own.
    "1k6/b7/8/8/7q/8/5PPP/6K1 w - - 0 1",
    // Shelter at both distances: White's pawns are two ranks ahead of its king
    // and Black's are one.
    "6k1/5ppp/8/8/8/5PPP/8/6K1 w - - 0 1",
    // Three half-open files in front of the white king, against a black king
    // standing behind its own pawns.
    "1k6/ppp2ppp/8/8/8/8/PP6/6K1 w - - 0 1",
    // Passed pawns, added at S027 for the reason it added the two above: the
    // thirteen original positions leave the White-minus-Black difference at
    // zero in every bucket but two, and the eight-against-eight promotion race
    // cancels exactly. These two put a passed pawn on each side at different
    // distances from promotion, so the evaluate() comparison has something to
    // disagree about the moment the weights stop being zero.
    "4k3/8/8/3pP3/8/5p2/8/4K3 w - - 0 1",
    "4k3/8/8/P3p3/4P3/8/8/4K3 w - - 0 1",
    // Five buckets at a time, per colour, added when the engine's own counts
    // were exposed and the corpus turned out to reach almost none of them: no
    // White passer existed anywhere on the third, fourth or sixth rank, no
    // Black one on the fourth or sixth, and the only position touching the last
    // bucket is the eight-against-eight promotion race, where the two sides
    // cancel exactly and the difference says nothing. A ladder of unopposed
    // pawns and its mirror claim every bucket but the first from both
    // directions, which the first already had.
    "4k3/4P3/3P4/2P5/1P6/P7/8/4K3 w - - 0 1",
    "4k3/8/p7/1p6/2p5/3p4/4p3/4K3 b - - 0 1",
    // Pawn structure, added at S027 when the engine's own counts were exposed
    // and the twenty-two positions above turned out to reach one of the three
    // features from one direction only: no side ever had a doubled pawn
    // anywhere, and only White was ever backward. A doubled f-pawn, an isolated
    // and backward d-pawn stopped by an enemy pawn on c5, and the same position
    // mirrored so that Black is the side carrying all three.
    "r2q1rk1/pp3ppp/4pn2/2p5/8/2NP1P2/PP3PPP/R2Q1RK1 w - - 0 14",
    "r2q1rk1/pp3ppp/2np1p2/8/2P5/4PN2/PP3PPP/R2Q1RK1 b - - 0 14",
    // Piece placement, added at S027 when the engine's own counts were exposed
    // and the twenty-four positions above turned out never to put a rook on the
    // seventh at all, never to give Black a rook on an open file and never to
    // give White one on a half-open file. Only the bishop pair was reached from
    // both directions. A rook lifted to the seventh beside a second one on a
    // half-open file, and the same position with the colours swapped so that
    // Black is the side holding all three.
    "r5k1/ppppRppp/8/8/8/8/PPP2PPP/3R2K1 w - - 0 1",
    "3r2k1/ppp2ppp/8/8/8/8/PPPPrPPP/R5K1 b - - 0 1",
    // The truncation bound itself, added at S038 and re-measured at S065 and
    // again at S076. Everything above is hand-picked to reach a feature, and
    // hand-picked positions are exactly the ones whose taperings happen to
    // divide evenly: over the real corpus 135399 of 10795695 positions disagree
    // with the model by more than the 2.0 this file used to allow, and none of
    // the twenty-six above does. These four are here so the bound is exercised
    // by the test rather than only by a corpus under `.tuning/` that is
    // gitignored and does not exist on every machine. "the pinned positions
    // reach the truncation bound" below is what says they still do.
    //
    // **A residual belongs to the weights, not to the position**, which is why
    // this list is re-measured whenever the constants are refitted rather than
    // carried forward. S038 pinned the four worst the 2026-08-13 audit found
    // under the constants shipping then; S065's fit moved all four to between
    // 0.25 and 1.25, and S076's fit moved *its* four to between 0.08 and 1.83,
    // each time leaving the case asserting a property of weights no longer in
    // the tree. Each of these four is at the arithmetic maximum, 69/24 = 2.875
    // exactly, measured over all 10795695 rows of
    // `.tuning/selfplay_v2_dedup.tsv` by `build/tools/truncation_scan` -- 30
    // positions reach it and 69 more sit at 68/24 -- and they span the taper
    // from phase 5 to phase 23 with both sides to move so that a model error
    // confined to one end of it is still reachable. Phase 17 has no maximal
    // position under these weights, so the middle two are 11 and 19. DEC-057 is
    // the decision to re-measure; what it does not permit is lowering the
    // thresholds below to whatever came out.
    "8/8/8/6k1/1p1pr3/1Pp4P/2P2KP1/1N1R4 b - - 1 44",
    "1n6/1p2np2/3k2p1/3P4/8/5BPP/r1b2PN1/1R2K2R w K - 6 33",
    "r1b2rk1/1p1p2p1/p2pq2p/5B2/2PppP2/P2P2Q1/1P1N2PP/2R2RK1 b - - 0 18",
    "r2qkb1r/pp1b1ppp/3p4/2p1p3/2PnP3/2NP4/PP4PP/R1BQKBNR w kq - 2 10",
    // Passed pawns at a non-zero phase, added at S100 for a gap the count-based
    // non-vacuity cases cannot see. Every position above that puts a passer in
    // bucket 2, 4 or 5 is a bare-pawn endgame at phase 0, so `mg_weight` is
    // exactly 0 on all of them and the middlegame weight for those three
    // buckets
    // has no gradient from this corpus at all -- three of the twelve passed
    // pawn
    // parameters, unclaimed, while every count column was covered.
    // tests/test_tuner_gradient.cpp is where that failed out loud, and the four
    // lines below are the fix: the same buckets from both directions with rooks
    // still on the board. A count test cannot find this because the count is
    // right; what is missing is the phase it occurs at.
    "r5k1/1P4pp/8/8/8/8/6PP/R5K1 w - - 0 1",
    "r5k1/6pp/8/8/8/8/1p4PP/R5K1 b - - 0 1",
    "4k2r/8/1P6/8/3P4/8/6pp/4K2R w - - 0 1",
    "4k2r/6PP/8/4p3/8/1p6/8/4K2R b - - 0 1",
};

}  // namespace test_eval_positions
