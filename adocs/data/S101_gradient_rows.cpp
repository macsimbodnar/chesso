// S101, DEC-316. The fixture rows tests/test_tuner_gradient.cpp's "gradient() is
// the derivative of the error it descends" gained at S101's candidate, picked by
// the rule below and not by hand. adocs/data/S135_gradient_rows.cpp's shape. Three
// things differ from it, each said below: the threshold (WHY NOT S135'S THRESHOLD),
// eligibility (S121's, which S135's clamp clause is one clause of, THE RULE), and a
// failed read, which stops the program where S135's passed over the pair.
//
// WHAT WAS RED. At S101's candidate, the screen's 1234567 fit on the 104 rows, the
// case reads its worst relative error 2.066e-6 at parameter 984,
// `passed_king_own_mg[5]`, against its 1e-6; the next worst is 3.20e-7, at 996.
// The analytic gradient there is +7.507203e-05 and the central difference at h =
// 0.01 is +7.507218e-05, 1.55e-10 apart, and the relative error obeys O(h^2) (h
// 0.05, 0.02, 0.01, 0.005, 0.002 read 5.17e-5, 8.26e-6, 2.07e-6, 5.17e-7, 8.22e-8):
// the derivative is right, and the truncation term is large against the gradient.
// DEC-316's text names a near-zero parameter and this weight is not one, so this
// applies its answer to a case its text does not name, which DEC-343 records: fixture
// rows, the metric, h = 0.01 and the 1e-6 unchanged, every precondition that reads the
// fixture re-read, the floored metric left as the recorded alternative.
//
// WHY NOT S135'S THRESHOLD. S135's weight was all but stationary, its gradient
// under a tenth of its smallest sibling's, so "a tenth of the smallest sibling"
// separated. Here it does not: 7.5e-5 stands among its eleven siblings, 4.3e-6 to
// 5.8e-4, and that threshold is met by the base list, so it would take whichever
// pair came first. The law above gives the threshold instead: at a fixed third
// derivative the relative error goes as 1 / |gradient|.
//
// THE RULE, stated before this ran. Candidates: all_test_fens(), the JSON suites'
// positions, in that function's sorted order, skipping any position the list
// already holds, either way round. Eligible: S121's, as the coverage blocks have
// it (adocs/data/S101_coverage_rows.cpp): load_FEN accepts it and its colour-mirror,
// its phase is 1 to 23, each sits more than a centipawn inside the lazy clamp at the
// shipped constants, each scores within 800 cp of level there, and each loads with
// either side to move. For each eligible candidate, in order, the pair (the
// position, then its mirror, the list's convention) is appended after the first
// BASE_ROWS rows, with the fixture's cycling labels continuing, and gradient() is
// taken at the shipped constants over all rows. The pick is the first pair that
// puts |dE/d TARGET| at or above MULTIPLE times the base list's. MULTIPLE is 3: the
// law needs 2.07 to reach the bound, and 3 reads 6.9e-7, the factor of 1.45 a single
// next fit should not be able to take back. One pair; if none qualifies the program
// says so and exits 1. A pair `eligible` passed that then fails to load, or whose
// trace mismatches, is a defect and not a pair the rule passes over: the program
// stops on it. Nothing about the finite-difference check enters the rule: it reads the
// analytic gradient alone, so the pick is not chosen by the case passing.
//
// IT RE-DERIVES ITS OWN BLOCK (DEC-142). The base is the fixture's first BASE_ROWS =
// 104 rows, every row before this pair, asserted against the pair's marker comment in
// tests/test_eval_positions.hpp when the marker is there, read from S101_POSITIONS_HPP
// (default the source tree's, under CHESSO_SOURCE_DIR, since S324); with the pair
// landed, the pick must be the landed pair or the program exits 1.
//
// Built from the repository root after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -DCHESSO_SOURCE_DIR=\"$PWD\"
//       -I src -I tools -I tests -isystem tests/doctest/doctest -isystem
//       tests/third_party/nlohmann adocs/data/S101_gradient_rows.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S101_gradient_rows
//
// CHESSO_SOURCE_DIR, the root it is built from, is where it reads the JSON
// suites (load_json) and tests/test_eval_positions.hpp from, so it runs from any
// directory (S324; until then from build/tests, on the copy of tests/assets/ the
// build made there):
//
//   build/S101_gradient_rows
//
// It prints the base's reading, every eligible candidate it tried until the pick, the
// pick as two C++ lines for tests/test_eval_positions.hpp, and, after the pick and no
// part of the rule, the case's relative error at TARGET before and after the pair. The
// law is the estimate the multiple is set by, at a fixed third derivative, and rows
// move that too: the pair's |g| 6.16e-4 gives 2.5e-7 by the law and it reads 1.11e-7.
//
// Without the build line's CHESSO_SOURCE_DIR load_json reads the JSON suites
// relative to the working directory, and since S324 the build makes no copy of them.
#ifndef CHESSO_SOURCE_DIR
#error "build with -DCHESSO_SOURCE_DIR=\"<repository root>\", as the header says"
#endif
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

using tuner_layout::PARAM_COUNT;
using tuner_layout::PKO_MG_BASE;
using tuner_trace::dataset_t;

constexpr size_t TARGET = PKO_MG_BASE + 5;  // passed_king_own_mg[5], parameter 984
constexpr double MULTIPLE = 3.0;
constexpr size_t BASE_ROWS = 104;
constexpr int SCORE_LIMIT = 800;  // centipawns, White relative, either way
const char* const BLOCK_MARKER = "DEC-316's pair at S101's candidate";

game_t game;

int engine_phase(const std::string& fen)
{
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

// The analytic gradient over every row, as the case takes it (K = 1).
bool gradient_of(const std::vector<std::string>& fens,
                 const std::string& path,
                 std::vector<double>* analytic)
{
  dataset_t data;
  if (!write_rows(path, fens) || !load_rows(path, &data)) { return false; }

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  std::vector<uint32_t> index(data.size());
  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
    target[i] = data.result[i];
  }

  tuner_trace::gradient(data, params.data(), 1.0, index, 0, data.size(), 1, target,
                        analytic);
  return true;
}

// The case's check_differences() at TARGET alone, at its h. A reading printed after the
// pick, no part of the rule.
struct target_reading_t
{
  double analytic = 0.0;
  double numeric = 0.0;
  double relative = 0.0;
};

bool read_target(const std::vector<std::string>& fens,
                 const std::string& path,
                 target_reading_t* out)
{
  dataset_t data;
  if (!write_rows(path, fens) || !load_rows(path, &data)) { return false; }

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  std::vector<uint32_t> index(data.size());
  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
    target[i] = data.result[i];
  }

  std::vector<double> analytic;
  tuner_trace::gradient(data, params.data(), 1.0, index, 0, data.size(), 1, target,
                        &analytic);

  const tuner_trace::model_t& model = tuner_trace::engine_model();
  const double h = 0.01;
  const double saved = params[TARGET];
  params[TARGET] = saved + h;
  const double up = tuner_trace::error_range(model, data, params.data(), 1.0, index, 0,
                                             index.size(), 1, target);
  params[TARGET] = saved - h;
  const double down = tuner_trace::error_range(model, data, params.data(), 1.0, index,
                                               0, index.size(), 1, target);

  out->analytic = analytic[TARGET];
  out->numeric = (up - down) / (2.0 * h);
  const double scale = std::max(std::fabs(out->numeric), std::fabs(out->analytic));
  const double difference = std::fabs(out->numeric - out->analytic);
  out->relative = (scale > 1e-12) ? difference / scale : difference;
  return true;
}

// S121_coverage_rows.cpp's, as it is there.
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

double stage_two(const tuner_trace::row_trace_t& trace, const double* p)
{
  const std::vector<tuner_trace::feature_t>& features =
      tuner_trace::feature_table().features;
  const double mg_weight = trace.phase / static_cast<double>(GAME_PHASE_MAX);
  const double eg_weight = 1.0 - mg_weight;
  double sum = 0.0;

  for (const tuner_trace::entry_t& e : trace.entries) {
    const tuner_trace::feature_t& f = features[e.feature];
    if (!tuner_trace::clamped_site(f.site)) { continue; }
    sum += e.coef * (p[f.mg] * mg_weight + p[f.eg] * eg_weight);
  }

  // S122's curve, inside the clamp as the case's stage_two() has it since S122's
  // phase 1: each king's x per half, max(0, x)^2 / 2048 taken off its side. Until
  // S122's phase 2 this copy stopped above, which read the same while the curve
  // sat at its seed (DEC-354).
  double x[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  for (const tuner_trace::side_entry_t& e : trace.side_entries) {
    if (e.group != 0) { continue; }
    x[e.side][0] += e.coef * p[tuner_layout::KSC_MG_BASE + e.input];
    x[e.side][1] += e.coef * p[tuner_layout::KSC_EG_BASE + e.input];
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

bool eligible(const std::string& fen, const std::string& mirror, const double* params)
{
  tuner_trace::row_trace_t trace;
  tuner_trace::row_trace_t mirrored;
  if (!trace_fen(fen, &trace) || !trace_fen(mirror, &mirrored)) { return false; }

  return trace.phase >= 1 && trace.phase <= GAME_PHASE_MAX - 1 &&
         std::fabs(stage_two(trace, params)) < LAZY_EVAL_MARGIN - 1.0 &&
         std::fabs(stage_two(mirrored, params)) < LAZY_EVAL_MARGIN - 1.0 &&
         std::abs(trace.engine_white) <= SCORE_LIMIT &&
         std::abs(mirrored.engine_white) <= SCORE_LIMIT &&
         loads_either_side(fen) && loads_either_side(mirror);
}

}  // namespace


TEST_CASE("S101: pick the fixture rows DEC-316 owes passed_king_own_mg[5]")
{
  initialize_game_const_data(&game);

  const std::string path =
      (std::filesystem::temp_directory_path() / "chesso_S101_gradient_rows.tsv")
          .string();
  const std::vector<std::string>& landed = test_eval_positions::positions;
  REQUIRE(landed.size() >= BASE_ROWS);
  const std::vector<std::string> base(landed.begin(), landed.begin() + BASE_ROWS);

  // The base, asserted against the pair's marker comment where it stands.
  {
    const char* header_path = std::getenv("S101_POSITIONS_HPP");
    std::ifstream header(header_path
                             ? header_path
                             : CHESSO_SOURCE_DIR "/tests/test_eval_positions.hpp");
    REQUIRE_MESSAGE(header.good(), "tests/test_eval_positions.hpp");
    std::string line;
    size_t fens = 0;
    size_t before_marker = 0;
    bool marker = false;
    while (std::getline(header, line)) {
      if (line.find(BLOCK_MARKER) != std::string::npos) {
        REQUIRE_MESSAGE(!marker, "the pair's marker twice");
        marker = true;
        before_marker = fens;
      }
      const size_t open_quote = line.find_first_not_of(' ');
      if (open_quote != std::string::npos && line[open_quote] == '"' &&
          line.size() >= 2 && line.compare(line.size() - 2, 2, "\",") == 0) {
        fens++;
      }
    }
    printf("tests/test_eval_positions.hpp: %zu rows, marker %s", fens,
           marker ? "found" : "absent");
    if (marker) { printf(", %zu rows before it", before_marker); }
    printf("\n");
    REQUIRE(fens == landed.size());
    if (marker) { REQUIRE(before_marker == BASE_ROWS); }
  }

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  std::vector<double> g;
  REQUIRE(gradient_of(base, path, &g));

  const double wanted = MULTIPLE * std::fabs(g[TARGET]);
  printf("base, %zu rows: parameter %zu analytic %+.6e, the rule wants at least "
         "%.6e\n",
         base.size(), TARGET, g[TARGET], wanted);

  size_t tried = 0;
  for (const std::string& fen : all_test_fens()) {
    const std::string mirror = canonical_castling(mirror_fen(fen));
    bool known = false;
    for (const std::string& row : base) {
      if (row == fen || row == mirror) { known = true; }
    }
    if (known || !eligible(fen, mirror, params.data())) { continue; }

    std::vector<std::string> rows = base;
    rows.push_back(fen);
    rows.push_back(mirror);

    const bool read = gradient_of(rows, path, &g);
    REQUIRE_MESSAGE(read, "an eligible pair failed to load or to trace: " << fen);
    ++tried;

    const bool picked = std::fabs(g[TARGET]) >= wanted;
    printf("candidate %zu  |g| %.3e  %s  %s\n", tried, std::fabs(g[TARGET]),
           picked ? "PICK" : "-", fen.c_str());
    if (picked) {
      printf("%zu eligible pairs tried\n", tried);
      printf("pick, for tests/test_eval_positions.hpp:\n    \"%s\",\n    \"%s\",\n",
             fen.c_str(), mirror.c_str());

      target_reading_t before;
      target_reading_t after;
      REQUIRE(read_target(base, path, &before));
      REQUIRE(read_target(rows, path, &after));
      printf("the case's relative error at parameter %zu, h 0.01, no part of the rule:\n",
             TARGET);
      for (const target_reading_t* r : {&before, &after}) {
        printf("  %s: %.5e, analytic %+.6e, numeric %+.6e, difference %.3e\n",
               (r == &before) ? "base" : "with the pair", r->relative, r->analytic,
               r->numeric, std::fabs(r->numeric - r->analytic));
      }
      if (landed.size() > BASE_ROWS) {
        const bool same = landed.size() == BASE_ROWS + 2 &&
                          landed[BASE_ROWS] == fen && landed[BASE_ROWS + 1] == mirror;
        printf("reproduces the landed pair: %s\n", same ? "yes" : "NO");
        CHECK(same);
      }
      std::filesystem::remove(path);
      return;
    }
  }

  std::filesystem::remove(path);
  FAIL("no eligible pair of all_test_fens() puts the gradient at its multiple");
}
