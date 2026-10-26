#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
#include "test_walked_counts.hpp"
#include "tuner_trace.hpp"

// The trace build's contract, S262. The tuner fits a sum over what evaluate()
// recorded (src/eval_trace.hpp, tools/tuner_trace.hpp), and that sum is only
// evaluate() while two things hold, which this file checks on the positions
// it can carry -- tools/tuner checks the first on every corpus row it loads:
//
//   the identity  the trace, scored with the engine's integer tapers and
//                 clamp at the weights the engine was built with, is
//                 evaluate() exactly;
//   the meaning   every recorded coefficient is the count its index names,
//                 against counts taken outside the trace -- the engine's own
//                 per-colour accessors, the board's bitboards, and for
//                 mobility, which has no accessor, the ray walk in
//                 tests/test_walked_counts.hpp (the mirror's until S276).
//
// The second is not implied by the first. A coefficient recorded against the
// wrong index is invisible to the identity wherever the two weights involved
// are equal, and tempo and piece placement's two bad bishop columns ship at
// zero, where any coefficient at all satisfies it.

namespace
{

using tuner_trace::dataset_t;
using tuner_trace::row_trace_t;

game_t game;


struct trace_fixture_t
{
  trace_fixture_t() { initialize_game_const_data(&game); }
};


// Every position the cases below run over: the curated list, each mirrored so
// both sides are to move on every feature, and every position of the JSON
// suites -- which is where the clamp binds: their promotion and tactics cases
// put more pieces on a king than a game does.
std::vector<std::string> all_positions()
{
  std::vector<std::string> out;

  for (const std::string& fen : test_eval_positions::positions) {
    out.push_back(fen);
    out.push_back(mirror_fen(fen));
  }

  for (const std::string& fen : all_test_fens()) {
    out.push_back(fen);
  }

  return out;
}


// One position traced into a one-row dataset.
dataset_t trace_of(const std::string& fen, row_trace_t* trace)
{
  REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

  std::string error;
  REQUIRE_MESSAGE(tuner_trace::trace_board(&game.board, trace, &error),
                  (fen + ": " + error));

  dataset_t data;
  tuner_trace::append_row(*trace, 0.5f, 0, 0, &data);
  return data;
}


// A trace's coefficient for one (term, index), 0 when the row has none.
int coefficient(const dataset_t& data, eval_trace::term_t term, size_t index)
{
  const uint32_t feature =
      tuner_trace::feature_table().base[term] + static_cast<uint32_t>(index);

  for (uint32_t j = data.offsets[0]; j < data.offsets[1]; ++j) {
    if (data.entries[j].feature == feature) { return data.entries[j].coef; }
  }

  return 0;
}


// The four rows pinned for the truncation bound rather than for a feature
// count. Named separately because "the pinned positions reach the truncation
// bound" has to be able to fail when *these* stop reaching it, which is a
// different statement from every position keeping inside it. S038,
// re-measured at S065 and at S076; in tests/test_eval_model.cpp against the
// mirror's score until S276 moved them here, against the smooth score the
// mirror's sum became. Until S083 they were also the four truncation rows of
// test_eval_positions, in the order listed there; S083's four live here
// alone, and the exact-reconstruction and within-bound cases below add them,
// mirrored, to their loops through positions_with_truncation_rows() (the
// within-bound case says why).
//
// GOLDEN (DEC-142): the four positions themselves, drawn from the rows that
// reach the maximum a chain of truncating divisions can produce. A residual
// belongs to the weights and not to the position (DEC-057), so a refit is
// expected to stop these four qualifying.
// Re-derive: build/tools/truncation_scan --data <corpus> --min 2.8, which
// prints `difference, phase, side to move, fen` per hit; the four are chosen
// from the hits at the maximum to span the taper with both sides to move, by
// adocs/data/S083_truncation_pick.py over the scan since S083.
// Measured 2026-09-10 on .tuning/selfplay_v2_dedup.tsv at the shipped weights,
// with the mirror's scan: 10795695 rows, 138331 past 2.0, 105 past 2.8, 33 at
// the maximum 2.875, and all four below are among them. Re-derived at S276
// (2026-10-13) with the trace's scan on the same corpus: 10795695 rows, 105
// past 2.8, 33 at 2.875, the 105 hit lines byte-identical to the mirror's, and
// all four below still among the 33 at phases 5, 11, 19 and 23 -- the four
// stand. The 2.0 column read 137108, which is ulp noise on the 60996 rows
// sitting at exactly 48/24 and asserts nothing here
// (adocs/data/S276_truncation_scan.txt). Re-derived at S083 on its corpus
// (6226944 rows, `.tuning/s083/curve/s0/cand_6226944_s1.tsv`) at its weights:
// 64 rows past 2.8, 15 at 2.875, 14 distinct positions; S076's four read
// 1.875, 1.708, 2.000 and 1.500 there and stopped qualifying, and
// adocs/data/S083_truncation_pick.py, the sentence above as a rule, took the
// four below at phases 5, 11, 17 and 23 (adocs/data/S083s_eval_only.txt, whose
// hits are adocs/data/S083r_truncation_scan.txt's line for line). Re-derived
// at S135 on the same corpus at its piece placement fit: 65 rows past 2.8, 13
// distinct positions at 2.875, phases 5 to 19; three of S083's four fell under
// 2.8 and the pick kept its phase 11 row and took three more, at phases 5, 17
// and 19 (adocs/data/S135_goldens.txt).
// Moves legitimately on: a refit, and a move of LAZY_EVAL_MARGIN -- the smooth
// score and evaluate() both clamp the tapered mobility-plus-king-safety sum at
// that margin, so while the clamp binds the two agree exactly and the residual
// this pins is not there to be measured. S085 raising it from 150 to 184 took
// the corpus from 99 rows past 2.8 to 105 at unmoved weights, which is half of
// what S206 bisected. Margin: each of the four is asserted past 2.0 and the
// worst of them past 2.8, against a maximum of 2.875 -- so the tightest of the
// two clauses has 0.075 of room, which is the point, and re-choosing four from
// a fresh scan is the re-derivation.
// Property beside it: "the smooth score is within the truncation bound of the
// exact one", which holds over every position here rather than over four
// chosen rows -- but not at any weights: its bound is a function of how many
// taperings truncate, and a fitted tempo takes it from 3 x 23/24 to
// 4 x 23/24 = 3.833, which this case's precondition reports.
const std::vector<std::string> truncation_positions = {
    "2R5/8/3r3p/6p1/1k2B1P1/4K2P/5P2/8 b - - 16 52",
    "6k1/7p/p2b2p1/5n2/8/2Q5/PP3P1P/2KN3q w - - 1 33",
    "2q3k1/1b1R2p1/2p1r3/p2pNQ1p/Pb1P2pP/4P1P1/1P4K1/2R5 b - - 5 35",
    "rn5k/ppp3bp/3qbp2/3p2pB/8/1PN2Q1P/P1PP1PP1/R4RK1 w - - 4 16",
};


// all_positions() and the four pinned rows, each mirrored as that list's rows
// are. Since S083 the four live here and not in test_eval_positions, so the
// two cases that tie the trace to evaluate() -- exactly, and within the
// truncation bound -- take them from here, and the rows at the arithmetic
// maximum stay in both.
std::vector<std::string> positions_with_truncation_rows()
{
  std::vector<std::string> out = all_positions();

  for (const std::string& fen : truncation_positions) {
    out.push_back(fen);
    out.push_back(mirror_fen(fen));
  }

  return out;
}

}  // namespace


// S278's minimized case: one position, the sink written by trace_board() from
// this translation unit and not from evaluation.cpp, which defines it. Until
// the sink was declared constinit (src/eval_trace.hpp says why) the sanitizer
// build stopped here with UBSan's "store to null pointer of type 'struct
// record_t *'" at an address that was not null. Only that build can fail it:
// no other build checks the address. Whether a write site shows it depends on
// the registers GCC picks there; trace_of() showed it on GCC 13.3, so the case
// goes through trace_of().
TEST_CASE_FIXTURE(trace_fixture_t,
                  "one position traced from outside evaluation.cpp")
{
  row_trace_t trace;
  const dataset_t data = trace_of(DEFAULT_POSITION, &trace);

  CHECK(data.offsets[1] > data.offsets[0]);
  CHECK(eval_trace::sink == nullptr);
}


// Every parameter is some feature's middlegame or endgame half, exactly once,
// and nothing else is. A parameter no feature reaches is one the fit can never
// move -- the silent failure the old per-term gradient blocks had -- and one
// two features reach is a parameter two terms think they own.
TEST_CASE("the trace's features cover the parameter vector exactly once")
{
  const std::vector<tuner_trace::feature_t>& features =
      tuner_trace::feature_table().features;

  std::vector<int> owners(tuner_layout::PARAM_COUNT, 0);
  size_t untapered = 0;

  for (const tuner_trace::feature_t& f : features) {
    REQUIRE(f.mg < tuner_layout::PARAM_COUNT);
    owners[f.mg]++;

    if (f.eg == tuner_trace::NO_PARAM) {
      untapered++;
      CHECK(f.site == tuner_trace::SITE_MATERIAL);
      continue;
    }

    REQUIRE(f.eg < tuner_layout::PARAM_COUNT);
    owners[f.eg]++;
    CHECK(f.site != tuner_trace::SITE_MATERIAL);
  }

  for (size_t p = 0; p < owners.size(); ++p) {
    CAPTURE(p);
    CHECK(owners[p] == 1);
  }

  // Only the piece values are untapered, and every term has a base.
  CHECK(untapered == tuner_layout::MATERIAL_COUNT);
  CHECK(tuner_trace::feature_table().base[eval_trace::TERM_COUNT] ==
        features.size());
}


TEST_CASE_FIXTURE(trace_fixture_t,
                  "the trace reconstructs evaluate() exactly, clamp included")
{
  const std::vector<int64_t> weights = tuner_trace::engine_weights();
  const int64_t margin = LAZY_EVAL_MARGIN;

  size_t checked = 0;
  size_t clamped_high = 0;
  size_t clamped_low = 0;

  // The pinned truncation rows with their mirrors as well, since S083: the
  // rows where the smooth and exact scores part most are where exact_score()
  // most needs to be evaluate().
  for (const std::string& fen : positions_with_truncation_rows()) {
    CAPTURE(fen);

    row_trace_t trace;
    const dataset_t data = trace_of(fen, &trace);

    const int engine = evaluate(&game.board);
    const int stm = (game.board.active_color == WHITE) ? 1 : -1;

    CHECK(tuner_trace::exact_score(data, 0, weights.data()) == stm * engine);
    CHECK(trace.engine_white == stm * engine);

    const tuner_trace::exact_parts_t parts =
        tuner_trace::exact_parts(data, 0, weights.data());
    const int64_t two = parts.mobility + parts.safety;
    if (two > margin) { clamped_high++; }
    if (two < -margin) { clamped_low++; }

    checked++;
  }

  // The precondition that makes the clamp part of the claim: it binds here,
  // from both sides. At f57b92f's weights and margin on 478 of the 2764
  // positions; a precondition and not a golden, since any count above zero
  // each way makes the point.
  CAPTURE(checked);
  CAPTURE(clamped_high);
  CAPTURE(clamped_low);
  CHECK(checked > 2000);
  CHECK(clamped_high > 0);
  CHECK(clamped_low > 0);
}


// The trace is a side channel: with the sink set, evaluate() returns what it
// returns without one, and trace_board() leaves the sink cleared.
TEST_CASE_FIXTURE(trace_fixture_t, "tracing changes nothing evaluate() returns")
{
  for (const std::string& fen : all_positions()) {
    CAPTURE(fen);
    REQUIRE(load_FEN(fen, &game));

    const int plain = evaluate(&game.board);

    row_trace_t trace;
    std::string error;
    REQUIRE(tuner_trace::trace_board(&game.board, &trace, &error));

    CHECK(eval_trace::sink == nullptr);
    CHECK(evaluate(&game.board) == plain);
    CHECK(trace.engine_white ==
          ((game.board.active_color == WHITE) ? plain : -plain));
  }
}


TEST_CASE_FIXTURE(trace_fixture_t,
                  "every coefficient is the count its index names")
{
  for (const std::string& fen : all_positions()) {
    CAPTURE(fen);

    row_trace_t trace;
    const dataset_t data = trace_of(fen, &trace);
    const board_t& board = game.board;

    // Material and the tables, from the bitboards: White's pieces minus
    // Black's, Black's squares mirrored as eval_add_piece() mirrors them.
    for (int type = 0; type < 6; ++type) {
      const bb_t white = board.bitboards[W_PAWN + type];
      const bb_t black = board.bitboards[B_PAWN + type];

      if (type < 5) {
        CAPTURE(type);
        CHECK(coefficient(data, eval_trace::TERM_MATERIAL, type) ==
              count_bits(white) - count_bits(black));
      }

      for (int square = 0; square < 64; ++square) {
        const int expected = ((white >> square) & 1) -
                             static_cast<int>((black >> (square ^ 56)) & 1);
        CAPTURE(type);
        CAPTURE(square);
        CHECK(coefficient(data, eval_trace::TERM_PSQT, type * 64 + square) ==
              expected);
      }
    }

    int passed[2][5];
    passed_pawn_counts(&board, passed);
    for (size_t b = 0; b < 5; ++b) {
      CAPTURE(b);
      CHECK(coefficient(data, eval_trace::TERM_PASSED_PAWN, b) ==
            passed[WHITE][b] - passed[BLACK][b]);
    }

    int structure[2][3];
    pawn_structure_counts(&board, structure);
    for (size_t f = 0; f < 3; ++f) {
      CAPTURE(f);
      CHECK(coefficient(data, eval_trace::TERM_PAWN_STRUCTURE, f) ==
            structure[WHITE][f] - structure[BLACK][f]);
    }

    int placement[2][walked_counts::PIECE_PLACEMENT_COUNT];
    piece_placement_counts(&board, placement);
    for (size_t f = 0; f < walked_counts::PIECE_PLACEMENT_COUNT; ++f) {
      CAPTURE(f);
      CHECK(coefficient(data, eval_trace::TERM_PIECE_PLACEMENT, f) ==
            placement[WHITE][f] - placement[BLACK][f]);
    }

    int safety[2][KS_FEATURE_COUNT];
    king_safety_features(&board, safety);
    for (size_t f = 0; f < KS_FEATURE_COUNT; ++f) {
      CAPTURE(f);
      CHECK(coefficient(data, eval_trace::TERM_KING_SAFETY, f) ==
            safety[WHITE][f] - safety[BLACK][f]);
    }

    // Mobility has no engine accessor per piece type; the hand walk, the
    // mirror's until S276, is an implementation the trace shares nothing with.
    int mobility[4] = {};
    REQUIRE(walked_counts::mobility_features(fen.substr(0, fen.find(' ')),
                                             mobility));
    for (size_t t = 0; t < 4; ++t) {
      CAPTURE(t);
      CHECK(coefficient(data, eval_trace::TERM_MOBILITY, t) == mobility[t]);
    }

    CHECK(coefficient(data, eval_trace::TERM_TEMPO, 0) ==
          ((board.active_color == WHITE) ? 1 : -1));

    CHECK(trace.phase == game_phase(&board));
  }
}


// Non-vacuity for the case above: a feature zero on every position is one the
// comparison agrees about for free. Every feature outside the tables is
// reached in both directions.
TEST_CASE_FIXTURE(trace_fixture_t, "the positions move every term both ways")
{
  const tuner_trace::feature_table_t& table = tuner_trace::feature_table();
  std::vector<int> positive(table.features.size(), 0);
  std::vector<int> negative(table.features.size(), 0);

  for (const std::string& fen : all_positions()) {
    row_trace_t trace;
    const dataset_t data = trace_of(fen, &trace);

    for (uint32_t j = data.offsets[0]; j < data.offsets[1]; ++j) {
      const tuner_trace::entry_t e = data.entries[j];
      if (e.coef > 0) { positive[e.feature]++; }
      if (e.coef < 0) { negative[e.feature]++; }
    }
  }

  for (int t = 0; t < eval_trace::TERM_COUNT; ++t) {
    if (t == eval_trace::TERM_PSQT) { continue; }

    for (uint32_t f = table.base[t]; f < table.base[t + 1]; ++f) {
      CAPTURE(t);
      CAPTURE(f - table.base[t]);
      CHECK(positive[f] > 0);
      CHECK(negative[f] > 0);
    }
  }
}


// What the fit descends against what the engine computes. The smooth score
// drops the four truncating divisions, each of which loses under one
// centipawn, so the two can part by at most 23/24 per division that rounds:
// three while tempo ships at zero, whose division then truncates 0 / 24
// exactly. Clamping is 1-Lipschitz and only narrows the gap. The bound is the
// one tests/test_eval_model.cpp held the mirror to until S276 (DEC-053), which
// is the point: the trace's smooth score is the function the mirror computed.
TEST_CASE_FIXTURE(trace_fixture_t,
                  "the smooth score is within the truncation bound of the "
                  "exact one")
{
  const bool tempo_unfitted = (tempo_mg == 0) && (tempo_eg == 0);
  const double bound = (tempo_unfitted ? 3.0 : 4.0) * 23.0 / 24.0;

  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();

  double worst = 0.0;

  // Every position of the shared list, and since S083 the four pinned rows and
  // their mirrors, which no longer live in that list (below).
  for (const std::string& fen : positions_with_truncation_rows()) {
    CAPTURE(fen);

    row_trace_t trace;
    const dataset_t data = trace_of(fen, &trace);

    const double smooth = tuner_trace::score(data, 0, params.data());
    const double exact =
        static_cast<double>(tuner_trace::exact_score(data, 0, weights.data()));
    const double gap = std::fabs(smooth - exact);

    worst = std::max(worst, gap);
    CHECK(gap <= bound + 1e-9);
  }

  // The bound is reached, not merely respected: `truncation_positions` carries
  // four rows pinned at 69/24 (DEC-057, DEC-169), and they reach it here.
  // Until S083 the pinned four were also rows of test_eval_positions, and this
  // loop reached them through all_positions(). S083's refit moved S076's four
  // off the bound, and its own four were kept out of that list, which is
  // test_tuner_gradient's fixture too: putting them there moved what that
  // file measured -- swapped in, they took the only middlegame row for the
  // passed pawn's bucket-1 weight out of the list (the gradient check red at
  // parameter 800); appended, they moved the nonlinear group's recovery off
  // its planted numbers at its measured step (S083's record). So this loop
  // takes them from positions_with_truncation_rows(), and S076's four stay in
  // the list as the coverage rows they had become.
  //
  // GOLDEN (DEC-142): those four rows, the same golden `truncation_positions`
  // and "the pinned positions reach the truncation bound" below pin, with the
  // same re-derivation -- build/tools/truncation_scan --data <corpus> --min
  // 2.8, after a refit or a move of LAZY_EVAL_MARGIN; the scan has read this
  // smooth score from the trace since S276, the mirror's before, and the two
  // are the same function. Margin: 2.8 against the maximum 2.875.
  CAPTURE(worst);
  CHECK(worst > 2.8);
}


// Non-vacuous by construction, and the reason the bound above is a bound
// rather than a number someone widened until the suite went green. Each pinned
// position has to disagree by more than 2.0, or the fixture has stopped
// exercising the bound and the tolerance is untested again.
//
// 2.0 is the threshold the arithmetic gives and not a round number. Each of
// the three divisions loses less than one unit, at most 23/24, so two of them
// together cannot reach past 46/24 = 1.9167. A residual over 2.0 = 48/24 is
// therefore proof that all three divisions truncated on that position, which
// is what "this position exercises the bound" means; a position below it may
// be exercising only two. Every position pinned sits at 69/24 = 2.875, all
// three losing their maximum at once.
//
// The precondition is the tempo weights, because the count of *effective*
// truncations depends on them: at tempo_mg == tempo_eg == 0 the tempo
// division truncates 0 / 24 exactly and three of the four divisions can round,
// bounding the gap at 3 x 23/24 = 2.875. Fit tempo to anything non-zero and
// the fourth division starts rounding too, the bound becomes 4 x 23/24 =
// 3.833, and the four pinned here have to be re-derived against it. S038; in
// tests/test_eval_model.cpp against the mirror's score until S276.
TEST_CASE_FIXTURE(trace_fixture_t,
                  "the pinned positions reach the truncation bound")
{
  // As a variable because doctest decomposes the expression it is handed and
  // refuses a `&&` inside one.
  const bool tempo_unfitted = (tempo_mg == 0) && (tempo_eg == 0);

  CHECK_MESSAGE(tempo_unfitted,
                "tempo is fitted, so all four taperings can truncate: the "
                "bound is now 4 x 23/24 = 3.833 and the four pinned positions "
                "have to be re-derived against it");

  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();

  double worst = 0.0;

  for (const std::string& fen : truncation_positions) {
    CAPTURE(fen);

    row_trace_t trace;
    const dataset_t data = trace_of(fen, &trace);

    const double smooth = tuner_trace::score(data, 0, params.data());
    const double exact =
        static_cast<double>(tuner_trace::exact_score(data, 0, weights.data()));
    const double difference = std::fabs(smooth - exact);

    worst = std::max(worst, difference);

    CHECK_MESSAGE(difference > 2.0,
                  (fen + ": difference " + std::to_string(difference) +
                   ", no longer past 2.0"));
  }

  // And one of them has to sit at the top of the range, not merely past 2.0,
  // so the bound is pinned against the arithmetic it claims rather than
  // against whatever the mildest of these four happens to be.
  CHECK_MESSAGE(worst > 2.8, ("worst pinned difference " +
                              std::to_string(worst) + ", short of 2.875"));
}


// The refusals no legal board reaches. evaluate() writes at most two entries a
// piece and load_FEN() accepts at most 32 pieces, so a record from the engine
// never comes near the 1024-entry capacity, never carries an index outside
// its term, and never a per-side entry today; sum_record() is handed synthetic
// records instead. Each refusal must leave the thread-local summing buffer
// clear: the valid record summed after it comes out exactly as it does alone.
TEST_CASE("a record the dataset cannot hold is refused and leaves nothing")
{
  static eval_trace::record_t record;

  // A small valid record, and what it sums to on its own.
  const auto valid = [&]() {
    record.clear();
    eval_trace::sink = &record;
    eval_trace::set_phase(12);
    eval_trace::add(eval_trace::TERM_MATERIAL, eval_trace::SIDE_BOTH, 1, 2);
    eval_trace::add(eval_trace::TERM_MOBILITY, eval_trace::SIDE_BOTH, 3, 5);
    eval_trace::add(eval_trace::TERM_MOBILITY, eval_trace::SIDE_BOTH, 3, -1);
    eval_trace::sink = nullptr;
  };

  valid();
  row_trace_t alone;
  std::string error;
  REQUIRE(tuner_trace::sum_record(record, &alone, &error));
  REQUIRE(alone.entries.size() == 2);
  CHECK(alone.entries[0].coef == 2);
  CHECK(alone.entries[1].coef == 4);

  // Each bad record leaves a coefficient in the buffer before it is refused,
  // so a refusal that skipped the clearing would show in the sum after it.
  const auto refused_then_clean = [&](const std::string& what) {
    CAPTURE(what);

    row_trace_t out;
    std::string why;
    CHECK_FALSE(tuner_trace::sum_record(record, &out, &why));
    CHECK(!why.empty());

    valid();
    row_trace_t after;
    REQUIRE(tuner_trace::sum_record(record, &after, &why));
    REQUIRE(after.entries.size() == alone.entries.size());
    for (size_t i = 0; i < after.entries.size(); ++i) {
      CHECK(after.entries[i].feature == alone.entries[i].feature);
      CHECK(after.entries[i].coef == alone.entries[i].coef);
    }
  };

  // One past the capacity: the record keeps 1024 entries and says it lost one.
  record.clear();
  eval_trace::sink = &record;
  eval_trace::set_phase(12);
  for (size_t i = 0; i <= eval_trace::record_t::CAPACITY; ++i) {
    eval_trace::add(eval_trace::TERM_TEMPO, eval_trace::SIDE_BOTH, 0, 1);
  }
  eval_trace::sink = nullptr;
  CHECK(record.count == eval_trace::record_t::CAPACITY);
  CHECK(record.overflow);
  refused_then_clean("overflow");

  // An index past its term's width, after a coefficient already summed.
  record.clear();
  eval_trace::sink = &record;
  eval_trace::set_phase(12);
  eval_trace::add(eval_trace::TERM_MOBILITY, eval_trace::SIDE_BOTH, 3, 7);
  eval_trace::add(eval_trace::TERM_TEMPO, eval_trace::SIDE_BOTH, 1, 1);
  eval_trace::sink = nullptr;
  refused_then_clean("index outside its term");

  // A per-side entry for a term no group is declared for.
  record.clear();
  eval_trace::sink = &record;
  eval_trace::set_phase(12);
  eval_trace::add(eval_trace::TERM_MOBILITY, eval_trace::SIDE_BOTH, 3, 7);
  eval_trace::add(eval_trace::TERM_KING_SAFETY, eval_trace::SIDE_WHITE, 0, 1);
  eval_trace::sink = nullptr;
  refused_then_clean("per-side entry without a group");

  // A summed coefficient past int16, refused while the buffer is emitted.
  record.clear();
  eval_trace::sink = &record;
  eval_trace::set_phase(12);
  eval_trace::add(eval_trace::TERM_MATERIAL, eval_trace::SIDE_BOTH, 0, 30000);
  eval_trace::add(eval_trace::TERM_MATERIAL, eval_trace::SIDE_BOTH, 0, 30000);
  eval_trace::add(eval_trace::TERM_MOBILITY, eval_trace::SIDE_BOTH, 3, 7);
  eval_trace::sink = nullptr;
  refused_then_clean("coefficient outside int16");

  // No phase recorded.
  record.clear();
  refused_then_clean("no phase");
}
