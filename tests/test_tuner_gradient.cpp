#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
#include "test_temp_file.hpp"
#include "tuner_groups.hpp"
#include "tuner_layout.hpp"
#include "tuner_optimiser.hpp"
#include "tuner_trace.hpp"

// The fit's own path, end to end: what load() stores, that gradient() is the
// derivative of the error it descends, that the step holds the gauges, and
// that a planted vector comes back. S100 wrote the first two over the tuner's
// mirror model (tools/tuner_model.hpp), S275 the gauges; S262 moved the tuner
// onto the engine's trace (tools/tuner_trace.hpp) and every case here with it.
// The mirror's own loader kept its cases in tests/test_eval_model.cpp for
// tools/feature_audit until S276 moved that tool to the trace and deleted the
// mirror and those cases with it.
//
// Why the gradient is checked at all. gradient()'s old per-term blocks
// recorded the same failure three times -- "the forward model was extended
// first and a 20-epoch smoke run returned the eight weights exactly as they
// started, because nothing was pushing them" -- each caught by a human reading
// a log. The trace path has one loop for every linear term, so that failure
// has nowhere left to happen for them; it has not gone for a nonlinear group,
// which needs its own derivative, and the case for the synthetic group below
// is the check for that. Jon Dart's prescription, from the thread on Grant's
// tuning paper: compare the tuner's gradient with finite differences.
//
// What this file does NOT check: that a coefficient is the count its index
// names, or that a trace scored the engine's way is evaluate().
// tests/test_eval_trace.cpp holds both, against the engine.

namespace
{

using tuner_layout::EG_BASE;
using tuner_layout::KING_SAFETY_COUNT;
using tuner_layout::KS_EG_BASE;
using tuner_layout::KS_MG_BASE;
using tuner_layout::MATERIAL_COUNT;
using tuner_layout::MG_BASE;
using tuner_layout::MOB_EG_BASE;
using tuner_layout::MOB_MG_BASE;
using tuner_layout::MOBILITY_COUNT;
using tuner_layout::PARAM_COUNT;
using tuner_layout::PASSED_PAWN_COUNT;
using tuner_layout::PIECE_PLACEMENT_COUNT;
using tuner_layout::PL_EG_BASE;
using tuner_layout::PL_MG_BASE;
using tuner_layout::PP_EG_BASE;
using tuner_layout::PP_MG_BASE;
using tuner_layout::TEMPO_EG_BASE;
using tuner_layout::TEMPO_MG_BASE;

using test_eval_positions::positions;
using tuner_trace::dataset_t;


// One unique name per binary, not per call: several cases below write and
// then read the same path. S193.
std::string fixture_path()
{
  static const std::string path =
      unique_fixture_path("chesso_test_tuner_gradient.tsv");
  return path;
}


// The phase evaluate() tapers on, from the engine.
int engine_phase(const std::string& fen)
{
  game_t game;
  initialize_game_const_data(&game);
  REQUIRE(load_FEN(fen, &game));
  return game_phase(&game.board);
}


// `fen result score phase`, datagen's format, over the curated corpus.
//
// The three labels cycle rather than being constant, and the scores span both
// signs. A constant result column would make (target - sigma) the same factor
// on every row, which is a residual a wrong per-row feature could still fit; a
// zero score column would make --lambda untestable for the same reason S075's
// parser refuses one.
//
// `phase_shift` writes a phase column that is wrong on purpose, the true phase
// plus the shift wrapped into 0..GAME_PHASE_MAX, so it still passes load()'s
// range check -- the corpus a change to phase_value would leave behind.
bool write_fixture(const std::string& path, int phase_shift = 0)
{
  std::ofstream out(path);

  if (!out) { return false; }

  static const double results[3] = {1.0, 0.5, 0.0};
  int score = -280;

  for (size_t i = 0; i < positions.size(); ++i) {
    const std::string& fen = positions[i];
    const int phase = (engine_phase(fen) + phase_shift) % (GAME_PHASE_MAX + 1);

    out << fen << '\t' << results[i % 3] << '\t' << score << '\t' << phase
        << '\n';

    score += 41;
    if (score > 300) { score -= 600; }
  }

  return out.good();
}


bool load_fixture(const std::string& path,
                  dataset_t* data,
                  unsigned threads = 1)
{
  tuner_trace::load_report_t report;
  std::string error;
  const bool ok = tuner_trace::load(path, data, &report, &error, threads);

  if (ok) { CHECK(report.mismatches == 0); }
  return ok;
}


std::vector<uint32_t> every_row(const dataset_t& data)
{
  std::vector<uint32_t> index(data.size());
  for (size_t i = 0; i < index.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
  }
  return index;
}


std::vector<double> results_of(const dataset_t& data)
{
  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    target[i] = data.result[i];
  }
  return target;
}


// Central differences of error_range() for the parameters [first, last) of
// `model` -- every one, by default -- against `analytic`. Scale-free: the
// absolute gradients span many orders of magnitude -- an empty square's column
// is exactly zero, tempo's is the whole corpus -- so the error is relative to
// the larger of the pair, with an absolute floor for the ones that are
// legitimately zero.
struct difference_check_t
{
  double worst = 0.0;
  size_t worst_param = 0;
  size_t checked = 0;
};

difference_check_t check_differences(const tuner_trace::model_t& model,
                                     const dataset_t& data,
                                     std::vector<double> params,
                                     double k,
                                     const std::vector<uint32_t>& index,
                                     const std::vector<double>& target,
                                     const std::vector<double>& analytic,
                                     double h,
                                     size_t first = 0,
                                     size_t last = SIZE_MAX)
{
  difference_check_t out;

  for (size_t p = first; p < std::min(last, model.param_count); ++p) {
    const double saved = params[p];

    params[p] = saved + h;
    const double up = tuner_trace::error_range(
        model, data, params.data(), k, index, 0, index.size(), 1, target);

    params[p] = saved - h;
    const double down = tuner_trace::error_range(
        model, data, params.data(), k, index, 0, index.size(), 1, target);

    params[p] = saved;

    const double numeric = (up - down) / (2.0 * h);
    const double scale = std::max(std::fabs(numeric), std::fabs(analytic[p]));
    const double error = (scale > 1e-12)
                             ? std::fabs(numeric - analytic[p]) / scale
                             : std::fabs(numeric - analytic[p]);

    if (error > out.worst) {
      out.worst = error;
      out.worst_param = p;
    }

    out.checked++;
  }

  return out;
}


// The tapered mobility-plus-king-safety sum the clamp is applied to, in the
// smooth model: the stage-two half of tuner_trace::smooth().
double stage_two(const dataset_t& data, size_t row, const double* p)
{
  const std::vector<tuner_trace::feature_t>& features =
      tuner_trace::feature_table().features;
  const double mg_weight =
      data.phase[row] / static_cast<double>(GAME_PHASE_MAX);
  const double eg_weight = 1.0 - mg_weight;

  double sum = 0.0;

  for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
    const tuner_trace::entry_t e = data.entries[j];
    const tuner_trace::feature_t& f = features[e.feature];

    if (!tuner_trace::clamped_site(f.site)) { continue; }

    sum += e.coef * (p[f.mg] * mg_weight + p[f.eg] * eg_weight);
  }

  return sum;
}


// The 2764 positions tests/test_eval_trace.cpp runs over -- the curated list
// both ways round, and every position of the JSON suites -- traced into one
// dataset with a neutral label. The recovery cases replace the labels.
dataset_t trace_positions()
{
  game_t game;
  initialize_game_const_data(&game);

  std::vector<std::string> fens;
  for (const std::string& fen : positions) {
    fens.push_back(fen);
    fens.push_back(mirror_fen(fen));
  }
  for (const std::string& fen : all_test_fens()) {
    fens.push_back(fen);
  }

  dataset_t data;

  for (const std::string& fen : fens) {
    REQUIRE(load_FEN(fen, &game));

    tuner_trace::row_trace_t trace;
    std::string error;
    REQUIRE(tuner_trace::trace_board(&game.board, &trace, &error));

    tuner_trace::append_row(trace, 0.5f, 0, 0, &data);
  }

  return data;
}


// Fits `mask`'s parameters from `params` to `target` with the tuner's own step
// and returns after `epochs`. Every row trains.
void fit(const tuner_trace::model_t& model,
         const dataset_t& data,
         const std::vector<double>& target,
         const std::vector<uint8_t>& mask,
         double k,
         double lr,
         int epochs,
         std::vector<double>* params)
{
  const std::vector<uint32_t> index = every_row(data);
  const std::vector<double> start = *params;
  tuner_optimiser::state_t state(model.param_count);
  std::vector<double> grad;

  for (int epoch = 0; epoch < epochs; ++epoch) {
    tuner_trace::gradient(model, data, params->data(), k, index, 0, data.size(),
                          4, target, &grad);
    tuner_optimiser::descend(grad, mask, start.data(), lr, &state,
                             params->data());
  }
}

}  // namespace


TEST_CASE("the fixture corpus loads through the tuner's trace")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  dataset_t data;
  REQUIRE(load_fixture(path, &data));
  CHECK(data.size() == positions.size());

  std::filesystem::remove(path);
}


// 2026-09-10_adversarial-F36: the mirror's load() trusted the corpus's phase
// column, range checked only; S134 made it recompute the phase from the
// placement. The trace takes it from evaluate() itself, which is the one
// phase that matters, and the stored column is read for its range alone.
TEST_CASE("load() tapers on the engine's phase, not the stored column")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path, 7));

  // Precondition, read back from the file rather than recomputed: every phase
  // column the fixture wrote disagrees with the engine's phase, or the check
  // below holds for a load() that trusts the column too.
  {
    std::ifstream in(path);
    std::string line;
    size_t rows = 0;
    size_t column_wrong = 0;

    while (std::getline(in, line)) {
      std::istringstream fields(line);
      std::string fen, result, score, phase_text;
      REQUIRE(std::getline(fields, fen, '\t'));
      REQUIRE(std::getline(fields, result, '\t'));
      REQUIRE(std::getline(fields, score, '\t'));
      REQUIRE(std::getline(fields, phase_text, '\t'));

      if (std::stoi(phase_text) != engine_phase(fen)) { column_wrong++; }
      rows++;
    }

    REQUIRE(rows == positions.size());
    REQUIRE(column_wrong == rows);
  }

  dataset_t data;
  REQUIRE(load_fixture(path, &data));
  REQUIRE(data.size() == positions.size());

  for (size_t row = 0; row < data.size(); ++row) {
    CAPTURE(positions[row]);
    CHECK(static_cast<int>(data.phase[row]) == engine_phase(positions[row]));
  }

  std::filesystem::remove(path);
}


// S275, re-targeted at S262. The two king gauges tools/tuner_groups.hpp holds
// are null only on a row with one king a side: + c on every king square
// cancels between the White king and the Black one and nowhere else. The
// trace's load() reads every row through load_FEN(), which refuses any other
// position, so a hold can never move a row's score. Each row below is appended
// alone to the legal fixture, and the fixture is refused.
TEST_CASE("load() refuses a row without one king a side")
{
  const std::string path = fixture_path();

  const auto load_with = [&](const std::string& placement) {
    REQUIRE(write_fixture(path));
    {
      std::ofstream out(path, std::ios::app);
      out << placement << " w - - 0 40\t0.5\t0\t0\n";
    }

    dataset_t data;
    tuner_trace::load_report_t report;
    std::string error;
    return tuner_trace::load(path, &data, &report, &error);
  };

  // Precondition: a legal row in the same format loads, so a refusal below is
  // the king count and not the row's shape.
  CHECK(load_with("4k3/8/8/8/8/8/8/4K3"));

  static const char* const unbalanced[] = {
      "4k3/8/8/8/8/8/8/8",      // no White king
      "8/8/8/8/8/8/8/4K3",      // no Black king
      "4k3/8/8/8/8/8/8/3KK3",   // two White kings
      "3kk3/8/8/8/8/8/8/4K3",   // two Black kings
      "3kk3/8/8/8/8/8/8/3KK3",  // two a side: balanced, still not chess
      "8/8/8/8/8/8/8/8",        // none at all
  };

  for (const std::string placement : unbalanced) {
    CAPTURE(placement);
    CHECK_FALSE(load_with(placement));
  }

  std::filesystem::remove(path);
}


// The dataset path, end to end: every row load() stored is the trace of its
// FEN as trace_board() takes it, in file order, whatever the thread count. A
// row appended to the wrong slot, an offset off by one, or blocks put back in
// the wrong order show up here -- tests/test_eval_trace.cpp checks the trace
// against the engine and never sees what the loader did with it.
TEST_CASE("every stored row is its FEN's trace, at any thread count")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  dataset_t one;
  dataset_t four;
  REQUIRE(load_fixture(path, &one, 1));
  REQUIRE(load_fixture(path, &four, 4));
  REQUIRE(one.size() == positions.size());

  CHECK(one.entries.size() == four.entries.size());
  CHECK(one.offsets == four.offsets);
  CHECK(one.phase == four.phase);

  game_t game;
  initialize_game_const_data(&game);

  for (size_t row = 0; row < one.size(); ++row) {
    const std::string& fen = positions[row];
    CAPTURE(fen);

    REQUIRE(load_FEN(fen, &game));
    tuner_trace::row_trace_t trace;
    std::string error;
    REQUIRE(tuner_trace::trace_board(&game.board, &trace, &error));

    REQUIRE(one.offsets[row + 1] - one.offsets[row] == trace.entries.size());

    for (size_t i = 0; i < trace.entries.size(); ++i) {
      const tuner_trace::entry_t a = one.entries[one.offsets[row] + i];
      const tuner_trace::entry_t b = four.entries[four.offsets[row] + i];

      CHECK(a.feature == trace.entries[i].feature);
      CHECK(a.coef == trace.entries[i].coef);
      CHECK(b.feature == a.feature);
      CHECK(b.coef == a.coef);
    }

    CHECK(static_cast<int>(one.phase[row]) == trace.phase);
    CHECK(static_cast<int>(one.stm[row]) == trace.stm);
  }

  std::filesystem::remove(path);
}


// The finite-difference case below perturbs each weight by h, and a row whose
// stage-two sum sits within that perturbation of the margin straddles the
// clamp's kink, where the two one-sided derivatives differ and no central
// difference agrees with either. So it has a precondition, established here:
// at the shipped constants no fixture row binds the clamp or comes within a
// centipawn of it -- far more than h times any coefficient moves the sum.
TEST_CASE("the fixture sits clear of the clamp at the shipped constants")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  dataset_t data;
  REQUIRE(load_fixture(path, &data));

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  for (size_t row = 0; row < data.size(); ++row) {
    CAPTURE(positions[row]);
    const double two = stage_two(data, row, params.data());
    CAPTURE(two);
    CHECK(std::fabs(two) < LAZY_EVAL_MARGIN - 1.0);
  }

  std::filesystem::remove(path);
}


// Jon Dart's check. gradient() against the central difference of the error it
// descends, for every parameter at once -- all 823.
//
// h = 0.01 is S100's measurement over this fixture, worst relative error
// across all 827 parameters of the layout before S134:
//
//   h        0.5       0.1       0.05      0.01      0.005     0.001
//   worst    1.2e-04   4.9e-06   1.2e-06   4.9e-08   1.5e-08   2.3e-07
//
// the O(h^2) truncation law down to about 0.005, where the subtraction's
// cancellation takes over. On the trace path at S262, all 823: 4.87e-08, at
// parameter 774, the bishop's middlegame mobility weight. `-s` prints it.
TEST_CASE("gradient() is the derivative of the error it descends")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  dataset_t data;
  REQUIRE(load_fixture(path, &data));
  REQUIRE(data.size() == positions.size());

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  // Every row trains, none is held out: the gradient this checks is the one the
  // fit takes over its training range.
  const std::vector<uint32_t> index = every_row(data);
  const std::vector<double> target = results_of(data);

  // The K a fit would use is fitted from the data; here it only has to be the
  // same K on both sides of the comparison.
  const double k = 1.0;

  std::vector<double> analytic;
  tuner_trace::gradient(data, params.data(), k, index, 0, data.size(), 1,
                        target, &analytic);
  REQUIRE(analytic.size() == PARAM_COUNT);

  const difference_check_t check =
      check_differences(tuner_trace::engine_model(), data, params, k, index,
                        target, analytic, 0.01);

  CAPTURE(check.worst);
  CAPTURE(check.worst_param);
  MESSAGE("worst relative error " << check.worst << " at parameter "
                                  << check.worst_param);
  CHECK(check.checked == PARAM_COUNT);
  CHECK(check.worst < 1e-6);

  // Non-vacuity, and S100 found a real gap with it: a parameter whose analytic
  // gradient is zero is one the finite difference confirms for free. Every
  // parameter outside the tables has a gradient here -- three passed pawn
  // middlegame weights had none until S100 added four positions to the list.
  for (size_t p = MOB_MG_BASE; p < PARAM_COUNT; ++p) {
    CAPTURE(p);
    CHECK(std::fabs(analytic[p]) > 1e-12);
  }

  for (size_t p = 0; p < MATERIAL_COUNT; ++p) {
    CAPTURE(p);
    CHECK(std::fabs(analytic[p]) > 1e-12);
  }

  // The piece-square side, where an empty square legitimately contributes
  // nothing. 158 of the 768 columns are occupied somewhere in this corpus; the
  // floor is under that and fails if the corpus is ever emptied out.
  size_t psqt_nonzero = 0;
  for (size_t p = MG_BASE; p < MOB_MG_BASE; ++p) {
    if (std::fabs(analytic[p]) > 1e-12) { psqt_nonzero++; }
  }

  CAPTURE(psqt_nonzero);
  CHECK(psqt_nonzero > 120);

  std::filesystem::remove(path);
}


// The clamp, where the trace path and the mirror's differ on purpose. The
// mirror's gradient ignored the clamp and pushed the mobility and king safety
// weights on rows where they could not move the score; S100 pinned that as
// "the clamp is where gradient() and the error part company". The trace path
// differentiates the clamp: a row on which it binds gives those weights
// nothing. Push the king safety weights until it binds on some rows, and the
// gradient still matches finite differences on every parameter, including
// the clamped block -- and on the binding rows alone it is exactly zero there
// while it is not zero elsewhere.
TEST_CASE("the clamp's derivative is exact where it binds")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  dataset_t data;
  REQUIRE(load_fixture(path, &data));

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  // Measured at S262: +40 on each of the eighteen king safety weights binds
  // the clamp on one row of the 34, at 263.7 against 184, and leaves the
  // nearest other at 150 -- every row at least a centipawn from the kink, which
  // the precondition below re-establishes. A
  // much larger push saturates the sigmoid everywhere, which makes the finite
  // difference itself unreliable.
  for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
    params[KS_MG_BASE + t] += 40.0;
    params[KS_EG_BASE + t] += 40.0;
  }

  std::vector<uint32_t> binding;
  for (size_t row = 0; row < data.size(); ++row) {
    const double two = std::fabs(stage_two(data, row, params.data()));
    CAPTURE(positions[row]);
    CAPTURE(two);
    CHECK(std::fabs(two - LAZY_EVAL_MARGIN) > 1.0);
    if (two > LAZY_EVAL_MARGIN) {
      binding.push_back(static_cast<uint32_t>(row));
    }
  }

  // The precondition, established and not assumed: the clamp binds on some
  // rows and not on all of them.
  CAPTURE(binding.size());
  REQUIRE(!binding.empty());
  REQUIRE(binding.size() < data.size());

  const std::vector<uint32_t> index = every_row(data);
  const std::vector<double> target = results_of(data);
  const double k = 1.0;

  std::vector<double> analytic;
  tuner_trace::gradient(data, params.data(), k, index, 0, data.size(), 1,
                        target, &analytic);

  // h = 0.005, half the case above's: the push makes some scores large, so
  // the sigmoid's curvature -- the O(h^2) term -- is larger here, and at 0.01
  // the worst read 6.8e-7, too near the tolerance to survive a refit. S100's
  // sweep puts 0.005 still above the cancellation floor.
  const difference_check_t check =
      check_differences(tuner_trace::engine_model(), data, params, k, index,
                        target, analytic, 0.005);

  CAPTURE(check.worst);
  CAPTURE(check.worst_param);
  MESSAGE("clamp case: worst relative error " << check.worst << " at parameter "
                                              << check.worst_param);
  CHECK(check.worst < 1e-6);

  // On the binding rows alone, the clamped block's gradient is exactly zero
  // and something else's is not -- the rows are in the sum, and only the
  // clamped weights are silent there.
  std::vector<double> on_binding;
  tuner_trace::gradient(data, params.data(), k, binding, 0, binding.size(), 1,
                        target, &on_binding);

  double clamped_block = 0.0;
  double elsewhere = 0.0;
  for (size_t p = 0; p < PARAM_COUNT; ++p) {
    if (p >= MOB_MG_BASE && p < KS_EG_BASE + KING_SAFETY_COUNT) {
      clamped_block = std::max(clamped_block, std::fabs(on_binding[p]));
    } else {
      elsewhere = std::max(elsewhere, std::fabs(on_binding[p]));
    }
  }

  CAPTURE(clamped_block);
  CAPTURE(elsewhere);
  CHECK(clamped_block == 0.0);
  CHECK(elsewhere > 1e-9);

  std::filesystem::remove(path);
}


namespace
{

// The fixture with four rows appended whose kings stand apart, mirrored, at a
// phase above zero. The curated list's kings mostly sit on squares that cancel
// -- Ke1 against ke8 reads one column, +1 and -1 -- and the few middlegame
// columns it leaves read the same gradient with opposite signs, which Adam
// steps in exact opposition: on that list alone the plain update does not walk
// along psqt_mg[king] at all, so the case below could not show the hold doing
// anything there. Measured, not assumed, when S275 wrote the case.
bool write_gauge_fixture(const std::string& path)
{
  if (!write_fixture(path)) { return false; }

  std::ofstream out(path, std::ios::app);

  out << "2kr1b1r/ppp2ppp/2n5/3q4/3P4/2N2N2/PP3PPP/R2Q1RK1 w - - 0 "
         "12\t1\t40\t0\n"
      << "r4rk1/pp3ppp/2n5/8/8/2N5/PP2KPPP/R6R w - - 0 20\t0.5\t-15\t0\n"
      << "8/pp3p2/3k4/8/8/5K2/PP3P2/2R5 b - - 0 35\t1\t120\t0\n"
      << "r4r2/pp3ppk/8/8/8/8/PP3PPP/1K1R3R w - - 0 25\t0\t-60\t0\n";

  return out.good();
}

}  // namespace


// S275, re-targeted at S262. Each gauge tools/tuner_groups.hpp holds is a
// direction the model cannot see: its level down by c and the piece's value up
// by c, or a king table moved whole, leaves every row's smooth score -- the one
// the fit descends -- where it was.
//
// The engine's own integer score is not quite as blind, and S262 measured it:
// a king table moved whole cancels before the taper, but a piece's c moves
// from inside the positional taper's truncation to outside it, and truncation
// towards zero is not translation invariant where the tapered sum crosses
// zero: for a non-integer 0 < y < n, trunc(y - n) = trunc(y) - n + 1, one more
// than the shift alone (y = 0.5, n = 1 gives 0, not -1). So along a material
// gauge the exact score moves by at most one centipawn, on the rows where that
// sum changes sign; DEC-059's hand re-anchor of the queen saw exactly that
// (one centipawn on one of seven pinned positions). The hold runs on the
// smooth vector, and an emitted table rounds it either way.
TEST_CASE("moving along a gauge moves no fixture row's score")
{
  const std::string path = fixture_path();
  REQUIRE(write_gauge_fixture(path));

  dataset_t data;
  REQUIRE(load_fixture(path, &data));
  REQUIRE(data.size() == positions.size() + 4);

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();

  const auto worst_move = [&](const std::vector<double>& moved) {
    double worst = 0.0;
    for (size_t row = 0; row < data.size(); ++row) {
      worst = std::max(worst,
                       std::fabs(tuner_trace::score(data, row, moved.data()) -
                                 tuner_trace::score(data, row, params.data())));
    }
    return worst;
  };

  const auto worst_exact_move = [&](const std::vector<int64_t>& moved) {
    int64_t worst = 0;
    for (size_t row = 0; row < data.size(); ++row) {
      worst = std::max(
          worst, std::abs(tuner_trace::exact_score(data, row, moved.data()) -
                          tuner_trace::exact_score(data, row, weights.data())));
    }
    return worst;
  };

  const std::vector<tuner_groups::gauge_t> gauges = tuner_groups::gauges();
  REQUIRE(gauges.size() == 7);

  const int c = 37;

  for (const tuner_groups::gauge_t& gauge : gauges) {
    CAPTURE(gauge.name);

    std::vector<double> along = params;
    std::vector<int64_t> along_exact = weights;
    for (const size_t column : gauge.level) {
      along[column] -= c;
      along_exact[column] -= c;
    }
    if (gauge.absorb < PARAM_COUNT) {
      along[gauge.absorb] += c;
      along_exact[gauge.absorb] += c;
    }

    // Non-vacuity: one of the level's columns moved alone does move a row, so
    // the fixture reads this gauge and the zero below is not a direction
    // nothing touches.
    double one_column = 0.0;
    for (const size_t column : gauge.level) {
      std::vector<double> single = params;
      single[column] -= c;
      one_column = std::max(one_column, worst_move(single));
    }

    CHECK(one_column > 1.0);
    CHECK(worst_move(along) < 1e-9);
    CHECK(worst_exact_move(along_exact) <= 1);
  }

  std::filesystem::remove(path);
}


// S275, re-targeted at S262. The fit's own descent, run twice from the shipped
// constants: once as the tuner runs it, descend(), and once as the optimiser
// alone. The plain update walks along every one of the seven gauges -- the
// precondition, established here rather than assumed, without which a hold
// that did nothing would pass -- and the tuner's step leaves all seven at the
// level it started from, while no row's score differs between the two runs.
// The 32 pawn columns no row reaches are measured rather than held: a gradient
// of exactly zero is a step of exactly zero, so they come out bit-identical
// from both runs. Under both optimisers.
TEST_CASE("the fit holds every gauge still and no score sees the hold")
{
  const std::string path = fixture_path();
  REQUIRE(write_gauge_fixture(path));

  dataset_t data;
  REQUIRE(load_fixture(path, &data));

  std::vector<double> start(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(start.data());

  const std::vector<uint32_t> index = every_row(data);
  const std::vector<double> target = results_of(data);

  std::vector<uint8_t> mask;
  REQUIRE(tuner_groups::free_mask("all", &mask));

  // The seven written out here from the layout rather than read from
  // tuner_groups::gauges(), so a direction dropped from that list is still
  // checked: this is the test's own statement of what the fit has to hold. A
  // pawn stands on squares 8..55 of its table, the model's index mirrored for
  // Black.
  struct direction_t
  {
    std::string name;
    std::vector<size_t> level;
  };

  static const char* const names[6] = {"pawn", "knight", "bishop",
                                       "rook", "queen",  "king"};
  std::vector<direction_t> directions;

  for (size_t type = 0; type < MATERIAL_COUNT; ++type) {
    direction_t d{std::string("material/table ") + names[type], {}};
    for (const size_t base : {MG_BASE, EG_BASE}) {
      for (size_t square = 0; square < 64; ++square) {
        if (type == 0 && (square < 8 || square >= 56)) { continue; }
        d.level.push_back(base + type * 64 + square);
      }
    }
    directions.push_back(d);
  }

  for (const size_t base : {MG_BASE, EG_BASE}) {
    direction_t d{(base == MG_BASE) ? "psqt_mg[king]" : "psqt_eg[king]", {}};
    for (size_t square = 0; square < 64; ++square) {
      d.level.push_back(base + 5 * 64 + square);
    }
    directions.push_back(d);
  }

  const auto level_shift = [&](const direction_t& direction,
                               const std::vector<double>& params) {
    double sum = 0.0;
    for (const size_t column : direction.level) {
      sum += params[column] - start[column];
    }
    return sum / static_cast<double>(direction.level.size());
  };

  // K at the scale the sigmoid was written around, as in the gradient case
  // above. Adam at its default step; AdaGrad at 10 and not its default 30,
  // because on these 38 rows a step of 30 makes the descent itself chaotic:
  // measured at S262, the held and plain runs' scores part by 7e-13 at epoch
  // 25, 1e-7 at 75, 5e-4 at 125 and 1.6 cp at 175 -- last-bit rounding grown a
  // hundredfold every 25 epochs while every level stays held within 1e-9 -- so
  // the comparison would measure the chaos and not the hold. At 3 and at 10
  // the two stay within 6e-13 for all 300 epochs. The full-corpus fit at 30 has
  // 9716070 training rows and descended smoothly (S262_fits.txt).
  const double k = 1.0;
  const int epochs = 300;

  for (const tuner_optimiser::kind_t kind :
       {tuner_optimiser::ADAM, tuner_optimiser::ADAGRAD}) {
    CAPTURE(tuner_optimiser::kind_name(kind));

    const double lr = (kind == tuner_optimiser::ADAM) ? 1.0 : 10.0;

    std::vector<double> held = start;
    std::vector<double> plain = start;
    tuner_optimiser::state_t held_state(PARAM_COUNT, kind);
    tuner_optimiser::state_t plain_state(PARAM_COUNT, kind);
    std::vector<double> grad;

    for (int epoch = 0; epoch < epochs; ++epoch) {
      tuner_trace::gradient(data, held.data(), k, index, 0, data.size(), 1,
                            target, &grad);
      tuner_optimiser::descend(grad, mask, start.data(), lr, &held_state,
                               held.data());

      tuner_trace::gradient(data, plain.data(), k, index, 0, data.size(), 1,
                            target, &grad);
      tuner_optimiser::update(grad, mask, lr, &plain_state, plain.data());
    }

    // The walk's floor is a precondition, not a golden: at f57b92f's shipped
    // constants the smallest of the seven plain-update shifts is 0.32 a square
    // under Adam and 0.20 under AdaGrad, both the rook's, the largest 3.8 and
    // 4.4, both the knight's (S275 read 0.22 and 3.7 on the mirror's path,
    // which ignored the clamp). Re-read with `-s` on this case if the
    // constants or the fixture move; 0.1 is far above the hold's 1e-9 and
    // below the walk.
    for (const direction_t& direction : directions) {
      CAPTURE(direction.name);
      CAPTURE(level_shift(direction, plain));
      CAPTURE(level_shift(direction, held));

      CHECK(std::fabs(level_shift(direction, plain)) > 0.1);
      CHECK(std::fabs(level_shift(direction, held)) < 1e-9);
    }

    // No score sees the hold, and the fit did move: the agreement is between
    // two runs that changed the evaluation, not two copies of the start.
    double apart = 0.0;
    double travelled = 0.0;

    for (size_t row = 0; row < data.size(); ++row) {
      const double h = tuner_trace::score(data, row, held.data());
      const double p = tuner_trace::score(data, row, plain.data());
      const double s = tuner_trace::score(data, row, start.data());

      apart = std::max(apart, std::fabs(h - p));
      travelled = std::max(travelled, std::fabs(p - s));
    }

    CAPTURE(apart);
    CAPTURE(travelled);
    CHECK(travelled > 50.0);
    CHECK(apart < 1e-6);

    // The 32 columns no pawn can stand on, both tables, both runs.
    size_t unreachable = 0;

    for (const size_t base : {MG_BASE, EG_BASE}) {
      for (size_t square = 0; square < 64; ++square) {
        if (square >= 8 && square < 56) { continue; }

        CAPTURE(square);
        CHECK(held[base + square] == start[base + square]);
        CHECK(plain[base + square] == start[base + square]);
        unreachable++;
      }
    }

    CHECK(unreachable == 32);
  }

  std::filesystem::remove(path);
}


// S100's planted-vector recovery, on the trace path and in the fast suite. The
// labels are sigmoid(K * score) at a vector that differs from the shipped one
// in one group -- feature_audit's `--plant` numbers, S100's less the two S134
// folded -- and that group alone is fitted from the shipped constants, so the
// only answer that drives the error to zero is the planted one. A frozen base
// is the shape S100 found exact for every group; the joint fit is the run the
// step file records over a corpus slice.
TEST_CASE("a planted vector comes back through the trace path")
{
  const dataset_t data = trace_positions();
  const tuner_trace::model_t& model = tuner_trace::engine_model();

  std::vector<double> shipped(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(shipped.data());

  struct plant_t
  {
    const char* group;
    std::vector<std::pair<size_t, double>> values;
  };

  const std::vector<plant_t> plants = {
      {"passed_pawns",
       {{PP_MG_BASE + 0, 7},
        {PP_MG_BASE + 1, 14},
        {PP_MG_BASE + 2, 21},
        {PP_MG_BASE + 3, 33},
        {PP_MG_BASE + 4, 48},
        {PP_EG_BASE + 0, 11},
        {PP_EG_BASE + 1, 19},
        {PP_EG_BASE + 2, 30},
        {PP_EG_BASE + 3, 45},
        {PP_EG_BASE + 4, 70}}},
      {"piece_placement",
       {{PL_MG_BASE + 0, 28},
        {PL_MG_BASE + 1, 17},
        {PL_MG_BASE + 2, 9},
        {PL_EG_BASE + 0, 41},
        {PL_EG_BASE + 1, 8},
        {PL_EG_BASE + 2, 5}}},
      {"tempo", {{TEMPO_MG_BASE, 23}, {TEMPO_EG_BASE, 12}}},
  };

  const double k = 1.0;

  for (const plant_t& plant : plants) {
    const std::string group = plant.group;
    CAPTURE(group);

    std::vector<double> planted = shipped;
    for (const auto& [param, value] : plant.values) {
      planted[param] = value;
    }

    std::vector<double> target(data.size());
    for (size_t row = 0; row < data.size(); ++row) {
      target[row] = tuner_target::sigmoid(
          tuner_trace::score(model, data, row, planted.data()), k);
    }

    std::vector<uint8_t> mask;
    REQUIRE(tuner_groups::free_mask(plant.group, &mask));

    // The tuner's own step, 1.0. Passed pawn bucket 1's middlegame weight is
    // the slowest to come back: few of these positions put a passer there at
    // a non-zero phase, and at 3000 epochs it read 13.02 against 14 planted,
    // at 6000 13.95.
    std::vector<double> fitted = shipped;
    fit(model, data, target, mask, k, 1.0, 6000, &fitted);

    // Within half a centipawn is what the emitted integer needs to be the
    // planted one.
    for (const auto& [param, value] : plant.values) {
      CAPTURE(param);
      CAPTURE(fitted[param]);
      CHECK(std::fabs(fitted[param] - value) < 0.5);
    }

    // And the frozen rest came out bit-identical.
    for (size_t p = 0; p < PARAM_COUNT; ++p) {
      if (!mask[p]) { CHECK(fitted[p] == shipped[p]); }
    }
  }
}


namespace
{

// A synthetic nonlinear group over chesso's own counts: each side's king
// danger, the four attacker counts and the zone incidences the engine already
// counts per colour (king_safety_features()), weighted and squared above zero
// -- the shape S122's curve is written for -- and subtracted from that side's
// score. evaluate() has no such term, so its per-side entries come from the
// engine's accessor and not from a trace call; everything after that is the
// path a traced group would take: the dataset's per-side entries, the model's
// group, the smooth score, the gradient and the seed.
constexpr size_t DANGER_INPUTS = 5;
constexpr double DANGER_DIVISOR = 64.0;

tuner_trace::model_t danger_model()
{
  tuner_trace::model_t model;
  model.param_count = PARAM_COUNT + 2 * DANGER_INPUTS;

  tuner_trace::group_t group;
  group.name = "king danger";
  group.divisor = DANGER_DIVISOR;
  group.sign = -1.0;

  for (size_t i = 0; i < DANGER_INPUTS; ++i) {
    group.inputs.push_back(
        {static_cast<uint32_t>(PARAM_COUNT + i),
         static_cast<uint32_t>(PARAM_COUNT + DANGER_INPUTS + i)});
  }

  model.groups.push_back(group);
  return model;
}


// trace_positions() with each row's per-side danger inputs attached.
dataset_t danger_positions()
{
  game_t game;
  initialize_game_const_data(&game);

  std::vector<std::string> fens;
  for (const std::string& fen : positions) {
    fens.push_back(fen);
    fens.push_back(mirror_fen(fen));
  }
  for (const std::string& fen : all_test_fens()) {
    fens.push_back(fen);
  }

  dataset_t data;

  for (const std::string& fen : fens) {
    REQUIRE(load_FEN(fen, &game));

    tuner_trace::row_trace_t trace;
    std::string error;
    REQUIRE(tuner_trace::trace_board(&game.board, &trace, &error));

    int counts[2][KS_FEATURE_COUNT];
    king_safety_features(&game.board, counts);

    for (int side = 0; side < 2; ++side) {
      for (size_t input = 0; input < DANGER_INPUTS; ++input) {
        if (counts[side][input] == 0) { continue; }
        trace.side_entries.push_back(
            {0, static_cast<uint8_t>(side), static_cast<uint16_t>(input),
             static_cast<int16_t>(counts[side][input])});
      }
    }

    tuner_trace::append_row(trace, 0.5f, 0, 0, &data);
  }

  return data;
}

}  // namespace


// The nonlinear group's derivative, checked as the linear terms' is: by
// central differences, over each of the group's ten parameters, at weights
// where every side the group reads sits on the transform's rising part. The
// linear parameters' derivative is the case above's; over these 2764 positions
// some table columns' gradients nearly cancel across rows, and a relative
// error there measures the sigmoid's curvature, not the derivative.
TEST_CASE("a nonlinear group's gradient is the derivative of the error")
{
  const tuner_trace::model_t model = danger_model();
  const dataset_t data = danger_positions();
  const std::vector<uint32_t> index = every_row(data);

  std::vector<double> params(model.param_count, 0.0);
  tuner_layout::starting_params(params.data());
  for (size_t i = 0; i < DANGER_INPUTS; ++i) {
    params[PARAM_COUNT + i] = 6.0 + i;
    params[PARAM_COUNT + DANGER_INPUTS + i] = 3.0 + i;
  }

  // The precondition: the group moves scores here, on both colours' kings,
  // and no side's sum is near the transform's kink at zero, where the two
  // one-sided second derivatives differ.
  size_t white_danger = 0;
  size_t black_danger = 0;
  double nearest_kink = 1e9;

  for (size_t row = 0; row < data.size(); ++row) {
    const tuner_trace::group_sums_t x =
        tuner_trace::group_sums(model.groups[0], 0, data, row, params.data());

    for (int side = 0; side < 2; ++side) {
      for (const double sum : {x.mg[side], x.eg[side]}) {
        if (sum != 0.0) { nearest_kink = std::min(nearest_kink, sum); }
      }
      if (x.mg[side] > 0.0) { (side == 0 ? white_danger : black_danger)++; }
    }
  }

  CAPTURE(white_danger);
  CAPTURE(black_danger);
  CAPTURE(nearest_kink);
  CHECK(white_danger > 100);
  CHECK(black_danger > 100);
  CHECK(nearest_kink > 1.0);

  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    target[i] = (i % 3 == 0) ? 1.0 : ((i % 3 == 1) ? 0.5 : 0.0);
  }

  const double k = 1.0;
  std::vector<double> analytic;
  tuner_trace::gradient(model, data, params.data(), k, index, 0, index.size(),
                        1, target, &analytic);
  REQUIRE(analytic.size() == model.param_count);

  const difference_check_t check =
      check_differences(model, data, params, k, index, target, analytic, 0.01,
                        PARAM_COUNT, model.param_count);

  CAPTURE(check.worst);
  CAPTURE(check.worst_param);
  MESSAGE("group: worst relative error " << check.worst << " at parameter "
                                         << check.worst_param);
  CHECK(check.checked == 2 * DANGER_INPUTS);
  CHECK(check.worst < 1e-6);

  for (size_t p = PARAM_COUNT; p < model.param_count; ++p) {
    CAPTURE(p);
    CHECK(std::fabs(analytic[p]) > 1e-12);
  }
}


// (5) of S262: max(0, x)^2 has slope exactly zero at x = 0, so a group seeded
// at zero weights has a gradient of exactly zero and a fit never moves it.
// derive_seed() puts each side's mean sum at the transform's unit over this
// data (DEC-134 form b), and from there the fit recovers a planted group --
// with the shipped linear terms frozen, so the group is the only thing that
// can move.
TEST_CASE("a nonlinear group stuck at zero is recovered from its derived seed")
{
  const tuner_trace::model_t model = danger_model();
  const dataset_t data = danger_positions();
  const std::vector<uint32_t> index = every_row(data);

  std::vector<double> base(model.param_count, 0.0);
  tuner_layout::starting_params(base.data());

  // The planted group, written here and derived from nothing outside this
  // file; its only job is to be a vector a correct fit gets back.
  static const double planted_mg[DANGER_INPUTS] = {14, 9, 11, 18, 4};
  static const double planted_eg[DANGER_INPUTS] = {6, 4, 5, 8, 2};

  std::vector<double> planted = base;
  for (size_t i = 0; i < DANGER_INPUTS; ++i) {
    planted[PARAM_COUNT + i] = planted_mg[i];
    planted[PARAM_COUNT + DANGER_INPUTS + i] = planted_eg[i];
  }

  const double k = 1.0;
  std::vector<double> target(data.size());
  for (size_t row = 0; row < data.size(); ++row) {
    target[row] = tuner_target::sigmoid(
        tuner_trace::score(model, data, row, planted.data()), k);
  }

  // The group alone is free.
  std::vector<uint8_t> mask(model.param_count, 0);
  for (size_t p = PARAM_COUNT; p < model.param_count; ++p) {
    mask[p] = 1;
  }

  // From zero: the planted labels are not met, and still the gradient over the
  // group is exactly zero and the fit does not move it.
  {
    std::vector<double> grad;
    tuner_trace::gradient(model, data, base.data(), k, index, 0, data.size(), 4,
                          target, &grad);

    const double start_error = tuner_trace::error_range(
        model, data, base.data(), k, index, 0, data.size(), 4, target);
    CAPTURE(start_error);
    CHECK(start_error > 1e-6);

    for (size_t p = PARAM_COUNT; p < model.param_count; ++p) {
      CAPTURE(p);
      CHECK(grad[p] == 0.0);
    }

    std::vector<double> stuck = base;
    fit(model, data, target, mask, k, 1.0, 50, &stuck);
    for (size_t p = PARAM_COUNT; p < model.param_count; ++p) {
      CHECK(stuck[p] == 0.0);
    }
  }

  // From the derived seed: every input seeded positive, and the fit recovers
  // the planted group.
  std::vector<double> seeded = base;
  tuner_trace::derive_seed(model, 0, data, index, 0, data.size(),
                           seeded.data());

  for (size_t p = PARAM_COUNT; p < model.param_count; ++p) {
    CAPTURE(p);
    CHECK(seeded[p] > 0.0);
  }

  // A fifth of the linear terms' step. Measured at S262 from this seed: at
  // 1.0 and at 0.3 the fit stalls at an error of 9.2e-8 and 8.8e-8, short of
  // the planted group -- on these positions without their mirrored half it
  // stalled at 7.2e-8 with two endgame weights negative, their sides pushed
  // onto the transform's flat part where nothing pulls them back -- and at 0.2
  // and 0.1 it comes back to the planted numbers. A curve's step is its own,
  // and S122 inherits that.
  std::vector<double> fitted = seeded;
  fit(model, data, target, mask, k, 0.2, 5000, &fitted);

  const double final_error = tuner_trace::error_range(
      model, data, fitted.data(), k, index, 0, data.size(), 4, target);
  CAPTURE(final_error);

  for (size_t i = 0; i < DANGER_INPUTS; ++i) {
    CAPTURE(i);
    CAPTURE(fitted[PARAM_COUNT + i]);
    CAPTURE(fitted[PARAM_COUNT + DANGER_INPUTS + i]);
    CHECK(std::fabs(fitted[PARAM_COUNT + i] - planted_mg[i]) < 0.5);
    CHECK(std::fabs(fitted[PARAM_COUNT + DANGER_INPUTS + i] - planted_eg[i]) <
          0.5);
  }
}


// AdaGrad keeps the two promises Adam's step makes: a frozen parameter comes
// out bit-identical, and so does one whose gradient has always been zero.
TEST_CASE("adagrad leaves frozen and unreached parameters bit-identical")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  dataset_t data;
  REQUIRE(load_fixture(path, &data));

  std::vector<double> start(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(start.data());

  std::vector<uint8_t> mask;
  REQUIRE(tuner_groups::free_mask("all", &mask));
  REQUIRE(tuner_groups::freeze_mask("material", &mask));

  std::vector<double> params = start;
  fit(tuner_trace::engine_model(), data, results_of(data), mask, 1.0,
      tuner_optimiser::ADAGRAD_DEFAULT_LR, 100, &params);

  size_t moved = 0;
  for (size_t p = 0; p < PARAM_COUNT; ++p) {
    if (!mask[p]) { CHECK(params[p] == start[p]); }
    if (params[p] != start[p]) { moved++; }
  }

  for (const size_t base : {MG_BASE, EG_BASE}) {
    for (size_t square = 0; square < 64; ++square) {
      if (square >= 8 && square < 56) { continue; }
      CHECK(params[base + square] == start[base + square]);
    }
  }

  // Non-vacuity: the fit did move the free parameters.
  CAPTURE(moved);
  CHECK(moved > 100);

  std::filesystem::remove(path);
}
