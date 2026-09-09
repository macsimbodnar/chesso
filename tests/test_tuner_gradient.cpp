#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "test_eval_positions.hpp"
#include "test_temp_file.hpp"
#include "tuner_model.hpp"

// The two things nothing in tests/ had ever checked about the fit: that the
// columns load() packs are the columns evaluate_position() reads, and that
// gradient() is the derivative of the error it is descending. S100.
//
// Both were blind spots by construction rather than by oversight -- every one
// of dataset_t, load(), evaluate_position(), error_range() and gradient() lived
// in an anonymous namespace inside tools/tuner.cpp, so no test could name them.
// S100 moved them to tools/tuner_model.hpp for this file.
//
// Why it matters here. S100 is a diagnosis of six evaluation weights that ship
// at zero, and one of the four candidate causes is a pipeline defect: a fit
// regressing on a column that does not hold what its name says, or descending a
// gradient that is not the objective's. gradient()'s own comments record the
// same failure three times -- "the forward model was extended first and a
// 20-epoch smoke run returned the eight weights exactly as they started,
// because nothing was pushing them" -- caught each time by a human reading a
// log. A fourth occurrence would be silent, and every fit taken after it would
// be contaminated. Jon Dart's prescription for exactly this, from the thread on
// Grant's tuning paper: compare the tuner's gradient against finite
// differences.
//
// What this file does NOT check, and where that is checked instead. It does not
// say the feature *definitions* are right -- it calls the same
// eval_model::*_features() functions load() calls, so a wrong definition agrees
// with itself. tests/test_eval_model.cpp is what holds those, against the
// engine's own counts and against placements worked out by hand. What is caught
// here is everything between a correct definition and a fitted weight:
// misalignment, an off-by-one row stride, a column pair swapped, a term whose
// gradient block was never written.

namespace
{

using eval_model::KING_SAFETY_COUNT;
using eval_model::KS_EG_BASE;
using eval_model::KS_MG_BASE;
using eval_model::MATERIAL_COUNT;
using eval_model::MG_BASE;
using eval_model::MOB_EG_BASE;
using eval_model::MOB_MG_BASE;
using eval_model::MOBILITY_COUNT;
using eval_model::PARAM_COUNT;
using eval_model::PASSED_PAWN_COUNT;
using eval_model::PAWN_STRUCTURE_COUNT;
using eval_model::PIECE_PLACEMENT_COUNT;
using eval_model::PL_EG_BASE;
using eval_model::PL_MG_BASE;
using eval_model::PP_EG_BASE;
using eval_model::PP_MG_BASE;
using eval_model::PS_EG_BASE;
using eval_model::PS_MG_BASE;
using eval_model::TEMPO_EG_BASE;
using eval_model::TEMPO_MG_BASE;

using test_eval_positions::positions;


// One unique name per binary, not per call: six cases below write and then
// read the same path. S193.
std::string fixture_path()
{
  static const std::string path =
      unique_fixture_path("chesso_test_tuner_gradient.tsv");
  return path;
}


// The FEN's second field, the one field tempo reads and no other feature does.
std::string side_to_move_of(const std::string& fen)
{
  const size_t first = fen.find(' ');
  const size_t second = fen.find(' ', first + 1);

  return fen.substr(first + 1, second - first - 1);
}


// `fen result score phase`, datagen's format, over the curated corpus.
//
// The three labels cycle rather than being constant, and the scores span both
// signs. A constant result column would make (target - sigma) the same factor
// on every row, which is a residual a wrong per-row feature could still fit; a
// zero score column would make --lambda untestable for the same reason S075's
// parser refuses one.
bool write_fixture(const std::string& path)
{
  std::ofstream out(path);

  if (!out) { return false; }

  static const double results[3] = {1.0, 0.5, 0.0};
  int score = -280;

  for (size_t i = 0; i < positions.size(); ++i) {
    const std::string& fen = positions[i];
    const std::string placement = fen.substr(0, fen.find(' '));

    std::vector<uint16_t> pieces;
    if (!eval_model::parse_placement(placement, &pieces)) { return false; }

    const int phase = eval_model::phase_of(pieces.data(), pieces.size());

    out << fen << '\t' << results[i % 3] << '\t' << score << '\t' << phase
        << '\n';

    score += 41;
    if (score > 300) { score -= 600; }
  }

  return out.good();
}


// The mean squared error gradient() is the derivative of, recomputed here
// through error_range() so that the two share no arithmetic but the forward
// model. `--lambda 0`, so the target is the result column alone.
double mse(const tuner_model::dataset_t& data,
           const std::vector<double>& params,
           double k,
           const std::vector<uint32_t>& index,
           const std::vector<double>& target)
{
  return tuner_model::error_range(data, params.data(), k, index, 0, data.size(),
                                  1, target);
}

}  // namespace


TEST_CASE("the fixture corpus loads through the tuner's own parser")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));
  CHECK(data.size() == positions.size());

  std::filesystem::remove(path);
}


// The dataset path, end to end: every column load() stored, against the same
// feature recomputed from the FEN text outside it. A misaligned base, a wrong
// stride, or two columns written in the other order all show up here and in
// nothing else -- test_eval_model.cpp checks the extractors against the engine
// and never sees what the loader did with their answers.
TEST_CASE("every stored column holds the feature its index names")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));
  REQUIRE(data.size() == positions.size());

  for (size_t row = 0; row < data.size(); ++row) {
    const std::string& fen = positions[row];
    CAPTURE(fen);

    const std::string placement = fen.substr(0, fen.find(' '));

    std::vector<uint16_t> pieces;
    REQUIRE(eval_model::parse_placement(placement, &pieces));

    // The piece list and its offsets, which is what every psqt and material
    // parameter is read through.
    REQUIRE(data.offsets[row + 1] - data.offsets[row] == pieces.size());
    for (size_t i = 0; i < pieces.size(); ++i) {
      CHECK(data.pieces[data.offsets[row] + i] == pieces[i]);
    }

    CHECK(static_cast<int>(data.phase[row]) ==
          eval_model::phase_of(pieces.data(), pieces.size()));

    int mobility[MOBILITY_COUNT] = {};
    REQUIRE(eval_model::mobility_features(placement, mobility));
    for (size_t i = 0; i < MOBILITY_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.mobility[row * MOBILITY_COUNT + i] == mobility[i]);
    }

    int safety[KING_SAFETY_COUNT] = {};
    REQUIRE(eval_model::king_safety_features(placement, safety));
    for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.king_safety[row * KING_SAFETY_COUNT + i] == safety[i]);
    }

    int passed[PASSED_PAWN_COUNT] = {};
    REQUIRE(eval_model::passed_pawn_features(placement, passed));
    for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.passed_pawn[row * PASSED_PAWN_COUNT + i] == passed[i]);
    }

    int structure[PAWN_STRUCTURE_COUNT] = {};
    REQUIRE(eval_model::pawn_structure_features(placement, structure));
    for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.pawn_structure[row * PAWN_STRUCTURE_COUNT + i] ==
            structure[i]);
    }

    int placement_counts[PIECE_PLACEMENT_COUNT] = {};
    REQUIRE(eval_model::piece_placement_features(placement, placement_counts));
    for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.piece_placement[row * PIECE_PLACEMENT_COUNT + i] ==
            placement_counts[i]);
    }

    int tempo = 0;
    REQUIRE(eval_model::tempo_feature(side_to_move_of(fen), &tempo));
    CHECK(static_cast<int>(data.tempo[row]) == tempo);
  }

  std::filesystem::remove(path);
}


// Non-vacuity, and it is load bearing twice over: a finite difference on a
// parameter whose feature is zero in every row compares 0.0 with 0.0 and passes
// for free, and so does a column comparison between two zeros. Every one of the
// 54 parameters outside the piece-square tables is claimed here, in both
// directions where the feature has one.
TEST_CASE("the fixture moves every term column in both directions")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));

  struct column_t
  {
    const char* name;
    const int16_t* base;
    size_t stride;
    size_t index;
  };

  std::vector<column_t> columns;
  for (size_t i = 0; i < MOBILITY_COUNT; ++i) {
    columns.push_back({"mobility", data.mobility.data(), MOBILITY_COUNT, i});
  }
  for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
    columns.push_back(
        {"king safety", data.king_safety.data(), KING_SAFETY_COUNT, i});
  }
  for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
    columns.push_back(
        {"passed pawn", data.passed_pawn.data(), PASSED_PAWN_COUNT, i});
  }
  for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
    columns.push_back({"pawn structure", data.pawn_structure.data(),
                       PAWN_STRUCTURE_COUNT, i});
  }
  for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
    columns.push_back({"piece placement", data.piece_placement.data(),
                       PIECE_PLACEMENT_COUNT, i});
  }

  for (const column_t& column : columns) {
    CAPTURE(column.name);
    CAPTURE(column.index);

    size_t positive = 0;
    size_t negative = 0;

    for (size_t row = 0; row < data.size(); ++row) {
      const int value = column.base[row * column.stride + column.index];

      if (value > 0) { positive++; }
      if (value < 0) { negative++; }
    }

    CHECK(positive > 0);
    CHECK(negative > 0);
  }

  // Tempo is stored in its own vector and is +1 or -1 on every row, so it has
  // no zero case to guard -- what it needs is both signs present, which is the
  // corpus having positions with each side to move.
  size_t white_to_move = 0;
  size_t black_to_move = 0;
  for (size_t row = 0; row < data.size(); ++row) {
    if (data.tempo[row] > 0) { white_to_move++; }
    if (data.tempo[row] < 0) { black_to_move++; }
  }
  CHECK(white_to_move > 0);
  CHECK(black_to_move > 0);

  std::filesystem::remove(path);
}


// The clamp is the one place gradient() is deliberately not the derivative of
// error_range(): the lazy margin bounds mobility and king safety together, and
// gradient() ignores that bound because treating the boundary as flat would
// stop the fit moving a weight it should move. So the finite-difference case
// below has a precondition -- the clamp binds on no fixture row at the shipped
// constants -- and this is where it is established rather than assumed. If a
// later weight change makes it bind, this fails and names the row instead of
// the gradient case failing and looking like an arithmetic bug.
TEST_CASE("the lazy clamp binds on no fixture row at the shipped constants")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));

  std::vector<double> params(PARAM_COUNT, 0.0);
  eval_model::starting_params(params.data());

  size_t bound = 0;

  for (size_t row = 0; row < data.size(); ++row) {
    const double mg_weight = static_cast<double>(data.phase[row]) /
                             static_cast<double>(GAME_PHASE_MAX);
    const double eg_weight =
        static_cast<double>(GAME_PHASE_MAX - data.phase[row]) /
        static_cast<double>(GAME_PHASE_MAX);

    double mg = 0.0;
    double eg = 0.0;

    for (size_t t = 0; t < MOBILITY_COUNT; ++t) {
      mg += data.mobility[row * MOBILITY_COUNT + t] * params[MOB_MG_BASE + t];
      eg += data.mobility[row * MOBILITY_COUNT + t] * params[MOB_EG_BASE + t];
    }
    for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
      mg += data.king_safety[row * KING_SAFETY_COUNT + t] *
            params[KS_MG_BASE + t];
      eg += data.king_safety[row * KING_SAFETY_COUNT + t] *
            params[KS_EG_BASE + t];
    }

    const double stage_two = mg * mg_weight + eg * eg_weight;

    CAPTURE(positions[row]);
    CAPTURE(stage_two);
    CHECK(std::fabs(stage_two) < static_cast<double>(LAZY_EVAL_MARGIN));

    if (std::fabs(stage_two) >= static_cast<double>(LAZY_EVAL_MARGIN)) {
      bound++;
    }
  }

  CHECK(bound == 0);

  std::filesystem::remove(path);
}


// Jon Dart's check. gradient() against the central difference of the error it
// descends, for all 827 parameters at once.
//
// Central rather than forward because the truncation error is O(h^2) instead of
// O(h), which is what makes a 1e-6 relative tolerance meaningful rather than a
// number chosen to pass. The model is linear in the parameters and the whole
// chain is in doubles, so the only two errors left are the sigmoid's curvature
// over 2h and the cancellation in the subtraction, and they pull opposite ways
// in h.
//
// h = 0.01 is measured and not guessed. Swept over this fixture, worst relative
// error across all 827 parameters:
//
//   h        0.5       0.1       0.05      0.01      0.005     0.001
//   worst    1.2e-04   4.9e-06   1.2e-06   4.9e-08   1.5e-08   2.3e-07
//
// which is the O(h^2) truncation law exactly -- each tenth of h buys two orders
// -- down to about 0.005, where the subtraction's cancellation takes over and
// the error starts climbing again. 0.01 sits a decade inside the truncation
// side of that floor and two orders under the tolerance. A first draft used
// h = 0.5 and failed at 5.7e-04 on a mobility weight, which is the same law
// read from the other end: mobility counts reach the tens, so half a unit of
// weight is tens of centipawns of score and the sigmoid is not linear over
// that.
//
// Scale-free comparison: the absolute gradients here span many orders of
// magnitude -- an empty square's psqt column is exactly zero, tempo's is the
// whole corpus -- so the tolerance is relative to the larger of the pair, with
// an absolute floor for the ones that are legitimately zero.
TEST_CASE("gradient() is the derivative of the error it descends")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));
  REQUIRE(data.size() == positions.size());

  std::vector<double> params(PARAM_COUNT, 0.0);
  eval_model::starting_params(params.data());

  // Every row trains, none is held out: the gradient this checks is the one the
  // fit takes over its training range, and a split would only shrink it.
  std::vector<uint32_t> index(data.size());
  for (size_t i = 0; i < index.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
  }

  // The K a fit would use is fitted from the data; here it only has to be the
  // same K on both sides of the comparison, and 1.0 is the scale the sigmoid
  // was written around.
  const double k = 1.0;

  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    target[i] = data.result[i];
  }

  std::vector<double> analytic;
  tuner_model::gradient(data, params.data(), k, index, 0, data.size(), 1,
                        target, &analytic);
  REQUIRE(analytic.size() == PARAM_COUNT);

  const double h = 0.01;
  size_t checked = 0;
  size_t nonzero = 0;
  double worst = 0.0;
  size_t worst_param = 0;

  for (size_t p = 0; p < PARAM_COUNT; ++p) {
    const double saved = params[p];

    params[p] = saved + h;
    const double up = mse(data, params, k, index, target);

    params[p] = saved - h;
    const double down = mse(data, params, k, index, target);

    params[p] = saved;

    const double numeric = (up - down) / (2.0 * h);
    const double scale = std::max(std::fabs(numeric), std::fabs(analytic[p]));
    const double error = (scale > 1e-12)
                             ? std::fabs(numeric - analytic[p]) / scale
                             : std::fabs(numeric - analytic[p]);

    if (error > worst) {
      worst = error;
      worst_param = p;
    }

    if (std::fabs(analytic[p]) > 1e-12) { nonzero++; }
    checked++;
  }

  CAPTURE(worst);
  CAPTURE(worst_param);
  CHECK(checked == PARAM_COUNT);
  CHECK(worst < 1e-6);

  // Non-vacuity for the case above, and it caught something. A parameter whose
  // analytic gradient is zero is one the finite difference confirms for free,
  // so every parameter this case means to check has to have a gradient to
  // check.
  //
  // Checked here rather than left to the column case further up, because a
  // non-zero feature column is not enough: the parameter is the feature *times*
  // its taper, and a bucket that only ever occurs at phase 0 has a zero
  // middlegame gradient however well the count is covered. Three of the twelve
  // passed pawn parameters were in exactly that state -- PP_MG[2], PP_MG[4] and
  // PP_MG[5], the middlegame weights for the buckets the corpus only reached in
  // bare-pawn endgames -- which is why four positions were added to
  // tests/test_eval_positions.hpp at S100. No count test could have found it.
  for (size_t p = MOB_MG_BASE; p < PARAM_COUNT; ++p) {
    CAPTURE(p);
    CHECK(std::fabs(analytic[p]) > 1e-12);
  }

  for (size_t p = 0; p < MATERIAL_COUNT; ++p) {
    CAPTURE(p);
    CHECK(std::fabs(analytic[p]) > 1e-12);
  }

  // The piece-square side, where an empty square legitimately contributes
  // nothing and what matters is how many squares are not empty. 158 of the 768
  // columns are occupied somewhere in this corpus; the floor is under that and
  // is there to fail if the corpus is ever emptied out, not to pin the number.
  size_t psqt_nonzero = 0;
  for (size_t p = MG_BASE; p < MOB_MG_BASE; ++p) {
    if (std::fabs(analytic[p]) > 1e-12) { psqt_nonzero++; }
  }

  CAPTURE(psqt_nonzero);
  CAPTURE(nonzero);
  CHECK(psqt_nonzero > 120);

  std::filesystem::remove(path);
}


// The clamp again, from the other side. The case above passes because the
// margin does not bind, and a reader could take that to mean gradient() is the
// exact derivative everywhere. It is not, by design, and the disagreement is
// worth pinning: push the king safety weights until the clamp binds and the
// analytic gradient for a clamped row's mobility and king safety parameters
// stops matching the finite difference, while every parameter outside the clamp
// still matches. That is the documented trade and this is what would notice if
// the trade were ever moved.
TEST_CASE("the clamp is where gradient() and the error part company")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));

  std::vector<double> params(PARAM_COUNT, 0.0);
  eval_model::starting_params(params.data());

  // Enough to put the sum past the margin on a minority of rows and nothing
  // else is touched. Measured: +40 on each of the eighteen king safety weights
  // binds the clamp on 3 of the 34 rows. A much larger push binds it on 22 and
  // saturates the sigmoid everywhere, which makes the finite difference itself
  // unreliable and would be measuring the arithmetic rather than the clamp.
  for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
    params[KS_MG_BASE + t] += 40.0;
    params[KS_EG_BASE + t] += 40.0;
  }

  std::vector<uint32_t> index(data.size());
  for (size_t i = 0; i < index.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
  }

  const double k = 1.0;
  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    target[i] = data.result[i];
  }

  std::vector<double> analytic;
  tuner_model::gradient(data, params.data(), k, index, 0, data.size(), 1,
                        target, &analytic);

  const double h = 0.01;
  double worst_clamped = 0.0;
  double worst_outside = 0.0;
  size_t worst_outside_param = 0;
  size_t bound = 0;

  for (size_t row = 0; row < data.size(); ++row) {
    const double mg_weight = static_cast<double>(data.phase[row]) /
                             static_cast<double>(GAME_PHASE_MAX);
    const double eg_weight =
        static_cast<double>(GAME_PHASE_MAX - data.phase[row]) /
        static_cast<double>(GAME_PHASE_MAX);

    double mg = 0.0;
    double eg = 0.0;

    for (size_t t = 0; t < MOBILITY_COUNT; ++t) {
      mg += data.mobility[row * MOBILITY_COUNT + t] * params[MOB_MG_BASE + t];
      eg += data.mobility[row * MOBILITY_COUNT + t] * params[MOB_EG_BASE + t];
    }
    for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
      mg += data.king_safety[row * KING_SAFETY_COUNT + t] *
            params[KS_MG_BASE + t];
      eg += data.king_safety[row * KING_SAFETY_COUNT + t] *
            params[KS_EG_BASE + t];
    }

    if (std::fabs(mg * mg_weight + eg * eg_weight) >=
        static_cast<double>(LAZY_EVAL_MARGIN)) {
      bound++;
    }
  }

  for (size_t p = 0; p < PARAM_COUNT; ++p) {
    const double saved = params[p];

    params[p] = saved + h;
    const double up = mse(data, params, k, index, target);
    params[p] = saved - h;
    const double down = mse(data, params, k, index, target);
    params[p] = saved;

    const double numeric = (up - down) / (2.0 * h);
    const double scale = std::max(std::fabs(numeric), std::fabs(analytic[p]));
    const double error = (scale > 1e-12)
                             ? std::fabs(numeric - analytic[p]) / scale
                             : std::fabs(numeric - analytic[p]);

    const bool clamped =
        (p >= MOB_MG_BASE && p < KS_EG_BASE + KING_SAFETY_COUNT);

    if (clamped) {
      worst_clamped = std::max(worst_clamped, error);
    } else if (error > worst_outside) {
      worst_outside = error;
      worst_outside_param = p;
    }
  }

  CAPTURE(worst_clamped);
  CAPTURE(worst_outside);
  CAPTURE(worst_outside_param);
  CAPTURE(bound);

  // The precondition, established and not assumed: with the weights pushed the
  // clamp really does bind on some row, so the disagreement below is the clamp
  // and not noise.
  CHECK(bound > 0);
  CHECK(bound < data.size());
  CHECK(worst_clamped > 1e-6);

  // And it is confined to the clamped block. The passed pawn, pawn structure,
  // piece placement and tempo parameters -- the six symptoms S100 is
  // diagnosing -- are outside the margin by construction, and stay exact.
  CHECK(worst_outside < 1e-6);

  std::filesystem::remove(path);
}
