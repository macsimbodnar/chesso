#pragma once
// The curated FEN corpus the evaluation's count, trace and tuner tests run
// over. S100.
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
    //
    // RE-PICKED by S223 (DEC-197): the kings stood on c5 and c4, adjacent, so
    // whichever side was not to move was in check -- python-chess reports
    // OPPOSITE_CHECK -- and the load boundary refuses that from S223 on. They
    // move to b5 and g4, which is the same two pawn walls and the same file
    // relationship to them: three own-pawn files under each king, so every
    // pawn, passed-pawn and king-shelter count this row contributes is the one
    // it contributed before, and only the two kings' piece-square entries move.
    // python-chess reports Status.VALID on the FEN below.
    "8/PPPPPPPP/8/1k6/6K1/8/pppppppp/8 w - - 0 1",
    // Lopsided material, so a wrong sign cannot cancel itself.
    "3qk3/8/8/8/8/8/8/3QK2R w K - 0 1",
    // King safety, added at S027 because the positions above leave most of the
    // term at zero: no knight and no rook ever bore on a king zone across all
    // thirteen, and seven of the nine counts only ever pointed one way. A count
    // that is zero everywhere is a count the engine and the walk agree about
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
    // divide evenly: over the real corpus 135399 of 10795695 positions
    // disagreed with the mirror's model by more than the 2.0 that
    // test_eval_model.cpp used to allow, and none of the twenty-six above does.
    // These four were here, at S076's weights, so the bound was exercised by
    // the test rather than only by a corpus under `.tuning/` that is gitignored
    // and does not exist on every machine, and "the pinned positions reach the
    // truncation bound" in test_eval_trace.cpp said they still did, against the
    // trace's smooth score since S276 and the mirror's before. At S083's
    // weights that case reads `truncation_positions` instead (below).
    //
    // **A residual belongs to the weights, not to the position**, which is why
    // this list is re-measured whenever the constants are refitted rather than
    // carried forward. S038 pinned the four worst the 2026-08-13 audit found
    // under the constants shipping then; S065's fit moved all four to between
    // 0.25 and 1.25, and S076's fit moved *its* four to between 0.08 and 1.83,
    // each time leaving the case asserting a property of weights no longer in
    // the tree. Each of these four was at S076's weights at the arithmetic
    // maximum, 69/24 = 2.875 exactly, measured over all 10795695 rows of
    // `.tuning/selfplay_v2_dedup.tsv` by `build/tools/truncation_scan` -- 30
    // positions reach it and 69 more sit at 68/24 -- and they span the taper
    // from phase 5 to phase 23 with both sides to move so that a model error
    // confined to one end of it is still reachable. Phase 17 had no maximal
    // position under those weights, so the middle two are 11 and 19. DEC-057 is
    // the decision to re-measure; what it does not permit is lowering the
    // thresholds below to whatever came out.
    //
    // **Since S083 these four are coverage rows, no longer the bound's.**
    // S083's refit moved them to 1.875, 1.708, 2.000 and 1.500, and the four
    // it re-derived are tests/test_eval_trace.cpp's `truncation_positions`
    // alone, not rows of this list: this list is test_tuner_gradient's
    // fixture as well, and S083 measured what moving these rows does there.
    // Swapped out, they took the only middlegame row for the passed pawn's
    // bucket-1 weight with them (the gradient check red at parameter 800,
    // `passed_pawn_mg[1]`, which S100's count cases cannot see); with the new
    // four appended instead, the nonlinear group's planted recovery missed
    // its numbers at its measured step. So these stay, with the indices and
    // labels that fixture gives them.
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
    // S135's four piece placement counts, added because the thirty-four above
    // never trap a minor or hem a rook in, and give Black the larger rammed
    // count nowhere. Three positions and each one's colour-mirror, so every
    // count reads both ways round, at phases 7, 6 and 5 so each count's
    // middlegame and endgame weights are told apart and both get a gradient:
    // a knight on h8 beside an enemy h7 pawn with a rook on h1 behind its own
    // king on g1, against more rammed pawns on Black's bishop's colour; a rook
    // on g1 beside its king on f1 alone; a bishop on a7 between enemy pawns on
    // b6 and c7 alone. Each count, and that each position is valid, was
    // checked by code written from evaluation.hpp's definitions and by
    // python-chess (S135's step file), not read off the board.
    "r1b1k2N/pp5p/8/3p4/3P4/8/4BPPP/6KR w - - 0 1",
    "6kr/4bppp/8/3p4/3P4/8/PP5P/R1B1K2n b - - 0 1",
    "5rk1/5ppp/2n5/8/8/8/5PPP/2B2KR1 w - - 0 1",
    "2b2kr1/5ppp/8/8/8/2N5/5PPP/5RK1 b - - 0 1",
    "3r2k1/B1p2ppp/1p2b3/8/8/5N2/5PPP/6K1 w - - 0 1",
    "6k1/5ppp/5n2/8/8/1P2B3/b1P2PPP/3R2K1 b - - 0 1",
    // The bishop's middlegame mobility weight, added at S135 (DEC-316) for
    // the gap S100 found in another weight: at S135's piece placement fit the
    // forty rows above leave the error all but stationary in
    // `mobility_mg[1]` -- its gradient -1.3e-6 against 6e-5 to 5e-4 for the
    // other seven mobility weights -- so tests/test_tuner_gradient.cpp's
    // relative check divided an O(h^2) truncation term by almost nothing and
    // read 2.9e-6 against its 1e-6. Picked by the rule in
    // adocs/data/S135_gradient_rows.cpp, not by hand: the first position of
    // all_test_fens(), in order, whose pair (it, then its colour-mirror)
    // stays clear of the clamp and puts that gradient at or above a tenth of
    // the smallest of the other seven. It is castling.json's, candidate 47,
    // and reads 1.87e-5 there. python-chess reports both Status.VALID.
    "1r1Nk2r/ppp1n1pp/8/2b5/8/2P5/PP2BPPP/R3K2R w KQk - 2 9",
    "r3k2r/pp2bppp/2p5/8/2B5/8/PPP1N1PP/1R1nK2R b Kkq - 2 9",
    // S121's mobility table, added at S121's phase 1b and re-derived at its
    // phase 2: each position, then its colour-mirror, picked by
    // adocs/data/S121_coverage_rows.cpp's rule and not by hand. The forty-two
    // rows above leave 55 of the table's 132 cell halves with no coefficient
    // at a phase that weights them, at the identity table and again at the
    // count S121 ships (enemy pawn attacks excluded and the pin rule) and
    // fitted constants. On them the gradient case's non-vacuity
    // (test_tuner_gradient) reads red, and "the positions move every term
    // both ways" (test_eval_trace), which also reads the JSON suites, on the
    // queen's count 27. The rule: greedy over all_test_fens(), then S083's
    // corpus for what stays open; each pick and its mirror at a phase from 1
    // to 23, a centipawn clear of the lazy clamp, within 800 cp of level and
    // loadable with either side to move. Each comment names the cells its pair
    // closes, both halves unless it names one. Every position is VALID by
    // python-chess. The program starts from the 42 rows above this comment
    // (its BASE_ROWS, asserted against this comment's first line), re-checks
    // every row below, and must pick exactly these pairs in this order.
    // Re-derive by its header whenever the counts or the shipped constants
    // move what it reads (DEC-142, DEC-326): at phase 2 two of phase 1b's
    // pairs were no longer eligible at the fitted constants and the picks
    // moved, so the block was replaced (adocs/data/S121_goldens.txt).
    // bishop 13; rook 14
    "4k2r/8/8/8/3BB3/4K3/8/r7 w k - 1 2",
    "R7/8/4k3/3bb3/8/8/8/4K2R b K - 1 2",
    // rook 6; queen 10
    "4r1k1/pp4pp/4p3/2R3Q1/3n4/2q4r/P1P2PPP/5RK1 w - - 1 2",
    "5rk1/p1p2ppp/2Q4R/3N4/2r3q1/4P3/PP4PP/4R1K1 b - - 1 2",
    // knight 7; queen 13
    "5rk1/pp4pp/2n1p3/2R3Q1/8/2q4r/P1P2PPP/5RK1 w - - 1 2",
    "5rk1/p1p2ppp/2Q4R/8/2r3q1/2N1P3/PP4PP/5RK1 b - - 1 2",
    // bishop 9; queen 4 eg, 5 eg
    "rn1qk2N/ppp1n1pp/4b3/2b5/8/2P5/PP1pBPPP/RNBQ1K1R w q - 2 9",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/4B3/PPP1N1PP/RN1QK2n b Q - 2 9",
    // rook 13
    "1r2k2r/R6R/8/8/8/8/4K3/8 w - - 1 2",
    "8/4k3/8/8/8/8/r6r/1R2K2R b - - 1 2",
    // queen 25
    "4k3/r7/8/5Q2/8/8/4K3/8 w - - 0 2",
    "8/4k3/8/8/5q2/8/R7/4K3 b - - 0 2",
    // queen 15
    "5rk1/pp4pp/4p3/2R3Q1/3n4/5q1r/P1P2PPP/5RK1 w - - 1 2",
    "5rk1/p1p2ppp/5Q1R/3N4/2r3q1/4P3/PP4PP/5RK1 b - - 1 2",
    // queen 16
    "5rk1/pp4pp/4p3/2R3Q1/3n4/7r/P1q2PPP/5RK1 w - - 0 2",
    "5rk1/p1Q2ppp/7R/3N4/2r3q1/4P3/PP4PP/5RK1 b - - 0 2",
    // queen 3
    "k6K/8/8/8/8/8/1ppppppp/NqNNNNNN w - - 0 2",
    "nQnnnnnn/1PPPPPPP/8/8/8/8/8/K6k b - - 0 2",
    // queen 0
    "r1bqk2N/pppnn1pp/8/2b5/8/2P5/PP1pBPPP/RNBQ1K1R w q - 2 9",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPPNN1PP/R1BQK2n b Q - 2 9",
    // queen 2
    "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1N1PP/RNBQK2n b Q - 0 8",
    "rnbqk2N/ppp1n1pp/8/2b5/8/2P5/PP2BPPP/RNrQ1K1R w q - 0 8",
    // queen 14
    "rnb1k2N/ppp1n1pp/3q4/2b5/8/2P5/PP1pBPPP/RNBQ1K1R w q - 2 9",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/3Q4/PPP1N1PP/RNB1K2n b Q - 2 9",
    // queen 1 eg
    "rn1qk2N/pppbn1pp/8/2b5/8/2P5/PP1pBPPP/RNBQ1K1R w q - 2 9",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPPBN1PP/RN1QK2n b Q - 2 9",
    // queen 18, 19
    "1Q6/5p2/8/3k1K2/7p/P7/6PP/4q3 w - - 0 43",
    "4Q3/6pp/p7/7P/3K1k2/8/5P2/1q6 b - - 0 43",
    // bishop 12; queen 23
    "1r3k1r/p5p1/3b4/Q2B2B1/7P/P7/2q4P/4R2K w - - 0 35",
    "4r2k/2Q4p/p7/7p/q2b2b1/3B4/P5P1/1R3K1R b - - 0 35",
    // queen 21, 22
    "8/4Q3/7p/1k3K1P/8/8/8/q7 w - - 4 68",
    "Q7/8/8/8/1K3k1p/7P/4q3/8 b - - 4 68",
    // bishop 11; queen 20
    "r1b1k1nr/3p2pp/5q2/p1bQ4/4B3/1P3P2/R1P3PP/2B1K1NR b Kk - 0 17",
    "2b1k1nr/r1p3pp/1p3p2/4b3/P1Bq4/5Q2/3P2PP/R1B1K1NR w Kk - 0 17",
    // queen 24, 26
    "8/8/8/4q2p/7K/5Q2/8/2k5 b - - 5 72",
    "2K5/8/5q2/7k/4Q2P/8/8/8 w - - 5 72",
    // queen 27
    "5k2/8/p7/4q3/7p/1P1R4/3K4/8 b - - 5 70",
    "8/3k4/1p1r4/7P/4Q3/P7/8/5K2 w - - 5 70",
    // S123's passed pawn family, added at S123's phase 1: each position, then
    // its colour-mirror, picked by adocs/data/S123_coverage_rows.cpp's rule
    // and not by hand. The eighty rows above leave 11 of the family's 58
    // parameters with no coefficient at a phase that weights them -- a free
    // and safe passer on its own seventh in either half, a safe one on its
    // fifth in the middlegame, a candidate on its own second, third, fourth
    // or sixth -- and on them the gradient case's non-vacuity
    // (test_tuner_gradient) reads red, and "the positions move every term
    // both ways" (test_eval_trace) on the candidates. The rule is S121's:
    // greedy over all_test_fens(), then S083's corpus, then a seeded
    // constructive fallback for what stays open (not needed here); each pick
    // and its mirror at a phase from 1 to 23, a centipawn clear of the lazy
    // clamp, within 800 cp of level and loadable with either side to move.
    // Each comment names the parameters its pair closes, both halves unless
    // it names one. Every position is VALID by python-chess. The program
    // starts from the 80 rows above this comment (its BASE_ROWS, asserted
    // against this comment's first line), re-checks every row below, and
    // must pick exactly these pairs in this order. Re-derive by its header
    // whenever the counts or the shipped constants move what it reads
    // (DEC-142, DEC-329 (8)).
    // S123 coverage, closes passed_safe_mg[5] passed_safe_eg[5]
    "3KB2r/2PP4/8/2k5/8/8/8/8 b - - 0 1",
    "8/8/8/8/2K5/8/2pp4/3kb2R w - - 0 1",
    // S123 coverage, closes candidate_passer_mg[1] candidate_passer_eg[1]
    "r3r1k1/pp3pbp/1qp1b1p1/2BP4/2B5/Q1n2N2/P4PPP/3R1K1R b - - 0 18",
    "3r1k1r/p4ppp/q1N2n2/2b5/2bp4/1QP1B1P1/PP3PBP/R3R1K1 w - - 0 18",
    // S123 coverage, closes passed_safe_mg[3]
    "2k5/6p1/6Pp/pr1pP2P/P4K2/8/8/8 b - - 1 1",
    "8/8/8/p4k2/PR1Pp2p/6pP/6P1/2K5 w - - 1 1",
    // S123 coverage, closes candidate_passer_mg[0] candidate_passer_eg[0]
    // candidate_passer_mg[2] candidate_passer_eg[2] candidate_passer_mg[4]
    // candidate_passer_eg[4]
    "5rkr/pp1b2qp/3P3p/3P4/N1PbR3/Q5p1/P1B2P1P/6KR w - - 0 30",
    "6kr/p1b2p1p/q5P1/n1pBr3/3p4/3p3P/PP1B2QP/5RKR b - - 0 30",
    // S125's pawn structure family, added at S125's phase 1: each position,
    // then its colour-mirror, picked by adocs/data/S125_coverage_rows.cpp's
    // rule and not by hand. The eighty-eight rows above leave 15 of the
    // family's 60 parameters with no coefficient at a phase that weights
    // them -- doubled pawns on the b, c, e and g files in either half, a
    // backward pawn on its own fourth, phalanx pawns on their own fourth and
    // sixth, a supported pawn on its own seventh in the middlegame -- and on
    // them the gradient case's non-vacuity (test_tuner_gradient) reads red,
    // and "the positions move every term both ways" (test_eval_trace) on the
    // doubled and phalanx cells. The rule is S121's, as S123's block above
    // has it: greedy over all_test_fens() (which closed none), then S083's
    // corpus, then a seeded constructive fallback for what stays open (not
    // needed here); each pick and its mirror at a phase from 1 to 23, a
    // centipawn clear of the lazy clamp, within 800 cp of level and loadable
    // with either side to move. Each comment names the parameters its pair
    // closes, both halves unless it names one. Every position is VALID by
    // python-chess. The program starts from the 88 rows above this comment
    // (its BASE_ROWS, asserted against this comment's first line), re-checks
    // every row below, and must pick exactly these pairs in this order.
    // Re-derive by its header whenever the counts or the shipped constants
    // move what it reads (DEC-142, DEC-331 (11)).
    // S125 coverage, closes doubled_file_mg[0] doubled_file_eg[0]
    // doubled_file_mg[1] doubled_file_eg[1] phalanx_mg[2] phalanx_eg[2]
    // phalanx_mg[4] phalanx_eg[4] supported_mg[4]
    "6k1/1p1B1pp1/r2p3p/2rP3P/4PP2/PppP2P1/2p2RK1/2R5 w - - 0 42",
    "2r5/2P2rk1/pPPp2p1/4pp2/2Rp3p/R2P3P/1P1b1PP1/6K1 b - - 0 42",
    // S125 coverage, closes doubled_file_mg[3] doubled_file_eg[3]
    // doubled_file_mg[5] doubled_file_eg[5] backward_rank_mg[1]
    // backward_rank_eg[1]
    "6r1/kp4P1/p1p3Rp/3pP2P/1P2p1P1/4P3/1PPK4/8 w - - 1 35",
    "8/1ppk4/4p3/1p2P1p1/3Pp2p/P1P3rP/KP4p1/6R1 b - - 1 35",
};

}  // namespace test_eval_positions
