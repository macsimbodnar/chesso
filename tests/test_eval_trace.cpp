#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "eval_model.hpp"
#include "evaluation.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
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
//                 mobility, which has no accessor, the mirror's ray walk.
//
// The second is not implied by the first. A coefficient recorded against the
// wrong index is invisible to the identity wherever the two weights involved
// are equal, and piece placement and tempo ship at zero, where any
// coefficient at all satisfies it.

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

}  // namespace


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

  for (const std::string& fen : all_positions()) {
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

    int placement[2][3];
    piece_placement_counts(&board, placement);
    for (size_t f = 0; f < 3; ++f) {
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

    // Mobility has no engine accessor per piece type; the mirror's own ray
    // walk is an implementation the trace shares nothing with.
    int mobility[4] = {};
    REQUIRE(
        eval_model::mobility_features(fen.substr(0, fen.find(' ')), mobility));
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
// mirror's (tests/test_eval_model.cpp, DEC-053), which is the point: the
// trace's smooth score is the function the mirror computed.
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

  for (const std::string& fen : all_positions()) {
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

  // The bound is reached, not merely respected: test_eval_positions carries
  // four rows pinned at 69/24 for the mirror (DEC-057, DEC-169), and the same
  // rows reach it here.
  //
  // GOLDEN (DEC-142): those four rows, the same golden test_eval_model pins and
  // with the same re-derivation -- build/tools/truncation_scan --data <corpus>
  // --min 2.8, after a refit or a move of LAZY_EVAL_MARGIN. The mirror's
  // residual and this one are the same quantity: the smooth score here is the
  // function the mirror computes. Margin: 2.8 against the maximum 2.875.
  CAPTURE(worst);
  CHECK(worst > 2.8);
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
