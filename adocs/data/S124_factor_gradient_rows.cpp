// S124, DEC-356. The rows tests/test_tuner_gradient.cpp's "the endgame factor's
// gradient is the derivative, chain rule in" gained on S124 phase 1's rebase
// onto S122's landing, picked by the rule below and not by hand: DEC-316's answer
// in adocs/data/S101_gradient_rows.cpp's form, promoted from the scratch program
// of adocs/data/S124_factor_gradient_options.txt (option A) unchanged in what it
// computes.
//
// WHAT WAS RED. At S122's constants the case read its worst relative error
// 3.4757e-6 at parameter 726, psqt_eg[king][b6], against its 1e-6, over the
// population's 5715 rows with E1 non-zero; S124 on d83d6c0 read 3.02e-7 there.
// Numeric and analytic agree to about 6e-15 at every h from 0.04 to 0.0025, the
// relative error flat in h and Richardson's no better: the summation's rounding
// over a weight nearly stationary on these rows (|g| 1.98e-9, 1.1e-5 of the
// case's largest), the derivative right. DEC-316's class; the owner ruled its
// answer, fixture rows, the metric, h 0.01 and the 1e-6 unchanged (DEC-356).
//
// THE RULE, written 2026-11-05 before the program first ran (the options file,
// "Option A, the rule"). Nothing about the finite difference enters the pick.
//   base       the case's own rows: tests/test_endgame_positions.hpp's
//              placements(), the generator population() holds before DEC-356's
//              pair, traced, the rows whose E1 is zero at the shipped weights
//              dropped (the case's endgame_rows(true)); its labels, 1, 0.5, 0
//              cycling by row; K 1; the shipped constants with the case's forced
//              family (every cell off F, mop-up's six off zero).
//   TARGET     the case's worst over its reached parameters, 726 at S122's
//              constants; MULTIPLE 3, S101's.
//   candidates the case's own population source at twice the placements,
//              placements(80), the same generator and seed, in its order; a
//              position the base already holds, either way round, is skipped,
//              and so is one already tried.
//   eligible   S101_gradient_rows.cpp's, as written there: load_FEN accepts the
//              position and its colour-mirror, its phase is 1 to 23, each sits
//              more than a centipawn inside the lazy clamp at the shipped
//              constants (stage two with S122's curve), each scores within 800
//              cp of level there, each loads with either side to move; and the
//              case's own filter, E1 non-zero at the shipped weights for both,
//              since the case drops any other.
//   pick       for each eligible candidate in order, the pair (the position,
//              then its mirror) appended after the base with the labels
//              continuing, gradient() at the case's setting over every row; the
//              first pair putting |dE/d TARGET| at or above MULTIPLE times the
//              base's. None qualifying: the program says so and exits 1.
//   then       the case's own check over its reached parameters with the pair
//              appended, its worst and where. If the worst is still past 1e-6
//              at another parameter, that parameter is the next TARGET over the
//              extended rows, the candidates continuing in order; at most three
//              pairs in all.
//
// IT RE-DERIVES ITS OWN GOLDEN (DEC-142). The pick must be
// test_endgame_positions::factor_gradient_rows, pair for pair, and population()
// must be placements() and then those rows; otherwise the program exits 1.
//
// Built from the repository root after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -I src -I tools -I tests
//       -isystem tests/doctest/doctest -isystem tests/third_party/nlohmann
//       adocs/data/S124_factor_gradient_rows.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S124_factor_gradient_rows
//
//   build/S124_factor_gradient_rows
//
// It prints the base's reading, each round's TARGET and pick, the case's worst
// with the pair, and whether the pick reproduces the landed pair. It reads no
// file, so it runs from any directory.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <set>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "search_params.hpp"
#include "test_endgame_positions.hpp"
#include "test_helpers.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

namespace
{

using namespace tuner_layout;
using tuner_trace::dataset_t;

game_t game;

constexpr int SCORE_LIMIT = 800;
constexpr double MULTIPLE = 3.0;
constexpr int MAX_PAIRS = 3;


// Stage two at `p`, the sum the lazy clamp bounds: the clamped sites' tapered
// sum and S122's curve, each king's T taken off its own side.
double stage_two(const tuner_trace::row_trace_t& trace, const double* p)
{
  const auto& features = tuner_trace::feature_table().features;
  const double mg_weight = trace.phase / static_cast<double>(GAME_PHASE_MAX);
  const double eg_weight = 1.0 - mg_weight;
  double sum = 0.0;

  for (const tuner_trace::entry_t& e : trace.entries) {
    const tuner_trace::feature_t& f = features[e.feature];
    if (!tuner_trace::clamped_site(f.site)) { continue; }
    sum += e.coef * (p[f.mg] * mg_weight + p[f.eg] * eg_weight);
  }

  double x[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  for (const tuner_trace::side_entry_t& e : trace.side_entries) {
    if (e.group != 0) { continue; }
    x[e.side][0] += e.coef * p[KSC_MG_BASE + e.input];
    x[e.side][1] += e.coef * p[KSC_EG_BASE + e.input];
  }
  for (int side = 0; side < 2; ++side) {
    const double own = (side == 0) ? -1.0 : 1.0;
    const double m = std::max(x[side][0], 0.0);
    const double n = std::max(x[side][1], 0.0);
    sum += own * (m * m * mg_weight + n * n * eg_weight) / KS_CURVE_DIVISOR;
  }

  return sum;
}


std::string with_side_to_move(const std::string& fen, char side)
{
  std::string flipped = fen;
  flipped[fen.find(' ') + 1] = side;
  return flipped;
}


bool loads_either_side(const std::string& fen)
{
  return load_FEN(with_side_to_move(fen, 'w'), &game) &&
         load_FEN(with_side_to_move(fen, 'b'), &game);
}


bool trace_fen(const std::string& fen, tuner_trace::row_trace_t* trace)
{
  if (!load_FEN(fen, &game)) { return false; }
  std::string error;
  return tuner_trace::trace_board(&game.board, trace, &error);
}


double e1_at(const tuner_trace::row_trace_t& trace, const double* shipped)
{
  dataset_t one;
  tuner_trace::append_row(trace, 0.5f, 0, 0, &one);
  return tuner_trace::smooth(tuner_trace::engine_model(), one, 0, shipped).e1;
}


bool eligible(const std::string& fen,
              const std::string& mirror,
              const double* shipped,
              tuner_trace::row_trace_t* a,
              tuner_trace::row_trace_t* b)
{
  if (!trace_fen(fen, a) || !trace_fen(mirror, b)) { return false; }

  return a->phase >= 1 && a->phase <= GAME_PHASE_MAX - 1 &&
         std::fabs(stage_two(*a, shipped)) < LAZY_EVAL_MARGIN - 1.0 &&
         std::fabs(stage_two(*b, shipped)) < LAZY_EVAL_MARGIN - 1.0 &&
         std::abs(a->engine_white) <= SCORE_LIMIT &&
         std::abs(b->engine_white) <= SCORE_LIMIT && loads_either_side(fen) &&
         loads_either_side(mirror) && e1_at(*a, shipped) != 0.0 &&
         e1_at(*b, shipped) != 0.0;
}


// tests/test_tuner_gradient.cpp's force_endgame_family(), as the case sets it.
void force(std::vector<double>* params)
{
  static const double cells[ENDGAME_SCALE_COUNT] = {
      64, 0, 24, 28, 68, 116, 96, 127, 120, 4, 40, 76, 92};
  static const double mopup[2][MOPUP_COUNT] = {{7, 11, 5}, {13, 9, 17}};

  for (size_t c = 0; c < ENDGAME_SCALE_COUNT; ++c) {
    (*params)[EGS_BASE + c] = cells[c];
  }
  for (size_t i = 0; i < MOPUP_COUNT; ++i) {
    (*params)[MOP_MG_BASE + i] = mopup[0][i];
    (*params)[MOP_EG_BASE + i] = mopup[1][i];
  }
}


std::vector<double> labels(size_t n)
{
  std::vector<double> t(n);
  for (size_t i = 0; i < n; ++i) {
    t[i] = (i % 3 == 0) ? 1.0 : ((i % 3 == 1) ? 0.5 : 0.0);
  }
  return t;
}


std::vector<uint32_t> every(size_t n)
{
  std::vector<uint32_t> out(n);
  for (size_t i = 0; i < n; ++i) { out[i] = static_cast<uint32_t>(i); }
  return out;
}


struct case_reading_t
{
  double worst = 0.0;
  size_t at = 0;
  double second = 0.0;
  size_t second_at = 0;
  std::vector<double> analytic;
};


// The case's check over its reached parameters, as the case writes it: the
// relative error of a central difference at h 0.01 against gradient().
case_reading_t case_check(const dataset_t& data, std::vector<double> params)
{
  case_reading_t r;
  const std::vector<double> target = labels(data.size());
  const std::vector<uint32_t> index = every(data.size());
  tuner_trace::gradient(data, params.data(), 1.0, index, 0, data.size(), 1,
                        target, &r.analytic);

  std::vector<uint8_t> reached(PARAM_COUNT, 0);
  for (size_t p = 0; p < MATERIAL_COUNT; ++p) { reached[p] = 1; }
  for (const auto& f : tuner_trace::feature_table().features) {
    if (f.site == tuner_trace::SITE_POSITIONAL) { reached[f.eg] = 1; }
  }
  for (size_t p = MOP_MG_BASE; p < PARAM_COUNT; ++p) { reached[p] = 1; }

  const auto& model = tuner_trace::engine_model();
  for (size_t p = 0; p < PARAM_COUNT; ++p) {
    if (!reached[p]) { continue; }
    const double saved = params[p];
    params[p] = saved + 0.01;
    const double up = tuner_trace::error_range(
        model, data, params.data(), 1.0, index, 0, index.size(), 1, target);
    params[p] = saved - 0.01;
    const double down = tuner_trace::error_range(
        model, data, params.data(), 1.0, index, 0, index.size(), 1, target);
    params[p] = saved;
    const double n = (up - down) / 0.02;
    const double s = std::max(std::fabs(n), std::fabs(r.analytic[p]));
    const double e = (s > 1e-12) ? std::fabs(n - r.analytic[p]) / s
                                 : std::fabs(n - r.analytic[p]);
    if (e > r.worst) {
      r.second = r.worst;
      r.second_at = r.at;
      r.worst = e;
      r.at = p;
    } else if (e > r.second) {
      r.second = e;
      r.second_at = p;
    }
  }

  return r;
}

}  // namespace


TEST_CASE("S124: pick the rows DEC-356 owes the endgame factor's gradient case")
{
  initialize_game_const_data(&game);
  std::vector<double> shipped(PARAM_COUNT, 0.0);
  starting_params(shipped.data());
  std::vector<double> params = shipped;
  force(&params);

  // population() is the generator's rows and then the landed pair.
  const std::vector<std::string> base_fens = test_endgame_positions::placements();
  const std::vector<std::string> landed_population =
      test_endgame_positions::population();
  const std::vector<std::string> landed(
      std::begin(test_endgame_positions::factor_gradient_rows),
      std::end(test_endgame_positions::factor_gradient_rows));
  std::vector<std::string> expected_population = base_fens;
  expected_population.insert(expected_population.end(), landed.begin(),
                             landed.end());
  REQUIRE_MESSAGE(landed_population == expected_population,
                  "population() is not placements() and then factor_gradient_rows");

  const std::set<std::string> base_set(base_fens.begin(), base_fens.end());
  std::set<std::string> held = base_set;
  dataset_t data;
  for (const std::string& fen : base_fens) {
    tuner_trace::row_trace_t t;
    REQUIRE_MESSAGE(trace_fen(fen, &t), fen);
    if (e1_at(t, shipped.data()) == 0.0) { continue; }
    tuner_trace::append_row(t, 0.5f, 0, 0, &data);
  }

  case_reading_t reading = case_check(data, params);
  printf("base: %zu rows; the case's worst %.6g at %zu, next %.6g at %zu\n",
         data.size(), reading.worst, reading.at, reading.second,
         reading.second_at);

  const std::vector<std::string> candidates =
      test_endgame_positions::placements(80);
  size_t next = 0;
  std::set<std::string> tried;
  std::vector<std::string> picked;

  for (int round = 0; round < MAX_PAIRS && reading.worst >= 1e-6; ++round) {
    const size_t target_param = reading.at;
    const double base_g = reading.analytic[target_param];
    printf("round %d: TARGET %zu, base |g| %.6e, the rule wants %.6e\n",
           round + 1, target_param, std::fabs(base_g),
           MULTIPLE * std::fabs(base_g));

    bool found = false;
    size_t eligible_tried = 0;
    for (; next < candidates.size(); ++next) {
      const std::string& fen = candidates[next];
      const std::string mirror = mirror_fen(fen);
      if (held.count(fen) || held.count(mirror) || tried.count(fen) ||
          tried.count(mirror)) {
        continue;
      }
      tried.insert(fen);

      tuner_trace::row_trace_t a, b;
      if (!eligible(fen, mirror, shipped.data(), &a, &b)) { continue; }
      eligible_tried++;

      dataset_t with = data;
      tuner_trace::append_row(a, 0.5f, 0, 0, &with);
      tuner_trace::append_row(b, 0.5f, 0, 0, &with);
      std::vector<double> g;
      tuner_trace::gradient(with, params.data(), 1.0, every(with.size()), 0,
                            with.size(), 1, labels(with.size()), &g);

      if (std::fabs(g[target_param]) >= MULTIPLE * std::fabs(base_g)) {
        printf("  pick after %zu eligible pairs (candidate %zu of %zu): |g| "
               "%.6e\n    %s\n    %s\n",
               eligible_tried, next + 1, candidates.size(),
               std::fabs(g[target_param]), fen.c_str(), mirror.c_str());
        data = with;
        picked.push_back(fen);
        picked.push_back(mirror);
        held.insert(fen);
        held.insert(mirror);
        found = true;
        ++next;
        break;
      }
    }

    if (!found) {
      FAIL("no eligible pair of placements(80) puts the gradient at its "
           "multiple ("
           << eligible_tried << " eligible tried)");
    }

    reading = case_check(data, params);
    printf("  the case with %zu rows: worst %.6g at %zu (|g| %.3e), next %.6g "
           "at %zu\n",
           data.size(), reading.worst, reading.at,
           std::fabs(reading.analytic[reading.at]), reading.second,
           reading.second_at);
  }

  printf("picked %zu rows\n", picked.size());
  for (const std::string& f : picked) { printf("PICK %s\n", f.c_str()); }

  const bool same = (picked == landed);
  printf("reproduces the landed pair (tests/test_endgame_positions.hpp's "
         "factor_gradient_rows): %s\n",
         same ? "yes" : "NO");
  CHECK(same);
  CHECK(reading.worst < 1e-6);
}
