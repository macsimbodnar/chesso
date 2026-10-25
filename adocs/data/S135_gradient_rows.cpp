// S135, DEC-316. The fixture rows tests/test_tuner_gradient.cpp's
// "gradient() is the derivative of the error it descends" gained when S135's
// piece placement fit left one parameter's gradient all but zero, picked by
// the rule below and not by hand.
//
// WHAT WAS RED. At the candidate's constants the case read its worst relative
// error 2.91e-6 at parameter 774, `mobility_mg[1]` (the bishop's), against its
// 1e-6. The analytic gradient there is -1.342036e-06 against its neighbours'
// 2.5e-4 and 3.7e-4, and the central difference at h = 0.01 is -1.342040e-06,
// 4e-12 apart: the fixture's error is nearly stationary in that weight, so the
// relative measure divides an O(h^2) truncation term by almost nothing (h 0.05,
// 0.02, 0.01, 0.005, 0.002 read 7.3e-5, 1.2e-5, 2.9e-6, 7.3e-7, 9.6e-8 there).
// The gradient is right; the fixture had stopped asking about that weight.
// The owner ruled 2026-10-26 (DEC-316): add fixture rows, S100's precedent,
// the metric, h and the bound unchanged.
//
// THE RULE, stated before this ran. Candidates: all_test_fens(), the JSON
// suites' positions, in that function's sorted order, skipping any position
// the list already holds, either way round. A candidate is eligible when the
// tuner loads it and its colour-mirror and both sit more than a centipawn
// inside the lazy clamp at the shipped constants -- the per-row condition "the
// fixture sits clear of the clamp at the shipped constants" asserts of every
// fixture row. For each eligible candidate, in order, the pair (the position,
// then its mirror, the list's convention) is appended after the list, with the
// fixture's own cycling labels and scores continuing, and gradient() is taken
// at the shipped constants over all rows. The pick is the first pair that puts
// |dE/d mobility_mg[1]| at or above one tenth of the smallest |dE/dp| over the
// other seven mobility parameters on that same extended fixture. One pair; if
// none qualifies the program says so and exits 1. Nothing about the
// finite-difference check enters the rule: it reads the analytic gradient
// alone, so the pick is not chosen by the case passing.
//
// Built from the repository root after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -I src -I tools
//       -I tests -isystem tests/doctest/doctest -isystem
//       tests/third_party/nlohmann adocs/data/S135_gradient_rows.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S135_gradient_rows
//
// and run from build/tests, where the JSON suites' relative paths resolve:
//
//   (cd build/tests && ../S135_gradient_rows)
//
// It prints the base fixture's mobility gradients, every candidate it tried
// until the pick, and the pick as two C++ lines for tests/test_eval_positions.hpp.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "search_params.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

namespace
{

using tuner_layout::MOB_MG_BASE;
using tuner_layout::PARAM_COUNT;
using tuner_trace::dataset_t;

constexpr size_t TARGET = MOB_MG_BASE + 1;  // mobility_mg[1], parameter 774

int engine_phase(const std::string& fen)
{
  game_t game;
  initialize_game_const_data(&game);
  if (!load_FEN(fen, &game)) { return -1; }
  return game_phase(&game.board);
}

// tests/test_tuner_gradient.cpp's write_fixture() at phase_shift 0, over any
// list: the labels cycle 1, 0.5, 0 by index and the scores step by 41.
bool write_rows(const std::string& path, const std::vector<std::string>& fens)
{
  std::ofstream out(path);
  if (!out) { return false; }

  static const double results[3] = {1.0, 0.5, 0.0};
  int score = -280;

  for (size_t i = 0; i < fens.size(); ++i) {
    const int phase = engine_phase(fens[i]);
    if (phase < 0) { return false; }
    out << fens[i] << '\t' << results[i % 3] << '\t' << score << '\t'
        << phase % (GAME_PHASE_MAX + 1) << '\n';
    score += 41;
    if (score > 300) { score -= 600; }
  }

  return out.good();
}

bool load_rows(const std::string& path, dataset_t* data)
{
  tuner_trace::load_report_t report;
  std::string error;
  return tuner_trace::load(path, data, &report, &error, 1) &&
         report.mismatches == 0;
}

// The tapered mobility-plus-king-safety sum the clamp is applied to:
// tests/test_tuner_gradient.cpp's stage_two(), the clamp-clear case's input.
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

// The analytic gradient over every row, as the case takes it (K = 1).
bool gradient_of(const std::vector<std::string>& fens,
                 const std::string& path,
                 std::vector<double>* analytic,
                 bool* clear)
{
  dataset_t data;
  if (!write_rows(path, fens) || !load_rows(path, &data)) { return false; }

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  *clear = true;
  for (size_t row = 0; row < data.size(); ++row) {
    if (std::fabs(stage_two(data, row, params.data())) >=
        LAZY_EVAL_MARGIN - 1.0) {
      *clear = false;
    }
  }

  std::vector<uint32_t> index(data.size());
  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
    target[i] = data.result[i];
  }

  tuner_trace::gradient(data, params.data(), 1.0, index, 0, data.size(), 1,
                        target, analytic);
  return true;
}

// mirror_fen() swaps the castling letters' case in place, so "KQk" comes back
// as "kqK"; the parser reads either, and the list writes White's rights first.
std::string canonical_castling(const std::string& fen)
{
  std::vector<std::string> parts = split_string(fen);
  if (parts.size() != 6 || parts[2] == "-") { return fen; }
  std::string rights;
  for (const char c : std::string("KQkq")) {
    if (parts[2].find(c) != std::string::npos) { rights += c; }
  }
  parts[2] = rights;
  std::string out;
  for (size_t i = 0; i < parts.size(); ++i) {
    out += (i ? " " : "") + parts[i];
  }
  return out;
}

// One tenth of the smallest |gradient| over the other seven mobility weights.
double threshold_of(const std::vector<double>& g)
{
  double smallest = INFINITY;
  for (size_t p = MOB_MG_BASE; p < MOB_MG_BASE + 8; ++p) {
    if (p != TARGET) { smallest = std::min(smallest, std::fabs(g[p])); }
  }
  return smallest / 10.0;
}

}  // namespace


TEST_CASE("S135: pick the fixture rows that move mobility_mg[1] off zero")
{
  const std::string path =
      (std::filesystem::temp_directory_path() / "chesso_S135_gradient_rows.tsv")
          .string();
  const std::vector<std::string>& base = test_eval_positions::positions;

  std::vector<double> g;
  bool clear = false;
  REQUIRE(gradient_of(base, path, &g, &clear));
  printf("base fixture, %zu rows, clear of the clamp: %s\n", base.size(),
         clear ? "yes" : "no");
  for (size_t p = MOB_MG_BASE; p < MOB_MG_BASE + 8; ++p) {
    printf("  param %zu  analytic %+.6e\n", p, g[p]);
  }
  printf("  target %zu at %.3e of the threshold %.3e\n", TARGET,
         std::fabs(g[TARGET]), threshold_of(g));

  size_t tried = 0;
  for (const std::string& fen : all_test_fens()) {
    const std::string mirror = canonical_castling(mirror_fen(fen));
    bool known = false;
    for (const std::string& row : base) {
      if (row == fen || row == mirror) { known = true; }
    }
    if (known) { continue; }

    std::vector<std::string> rows = base;
    rows.push_back(fen);
    rows.push_back(mirror);

    if (!gradient_of(rows, path, &g, &clear) || !clear) { continue; }
    ++tried;

    const double threshold = threshold_of(g);
    const bool picked = std::fabs(g[TARGET]) >= threshold;
    printf("candidate %zu  |g774| %.3e  threshold %.3e  %s  %s\n", tried,
           std::fabs(g[TARGET]), threshold, picked ? "PICK" : "-",
           fen.c_str());
    if (picked) {
      printf("pick, for tests/test_eval_positions.hpp:\n    \"%s\",\n    \"%s\",\n",
             fen.c_str(), mirror.c_str());
      std::filesystem::remove(path);
      return;
    }
  }

  std::filesystem::remove(path);
  FAIL("no eligible pair of all_test_fens() moves mobility_mg[1] off zero");
}
