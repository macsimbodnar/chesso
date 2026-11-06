// S124, DEC-316 as DEC-343 applied it. The fixture pair tests/test_tuner_gradient.cpp's
// "the clamp's derivative is exact where it binds" was offered at S124's phase 1, picked
// by the rule below and not by hand: adocs/data/S101_gradient_rows.cpp's rule in a mode
// for that case's setting. What differs from S101's is said below.
//
// **IT DID NOT LAND.** Its pick, `7K/1k6/8/8/8/8/pppppppp/NNNNNNNN w` and its mirror,
// takes TARGET's finite difference at +40 from 3.40e-7 to 9.90e-8, and +40's worst then
// moves to `passed_king_own_mg[5]` at 3.39e-7, still past the clamp rule's 2e-7, so the
// rule still answers no push; the coordinator's ruling allowed one pair, and the pair
// was taken out again (the S124 step file, 2026-11-04). The program stands as the
// reading and for the next ruling; with no pair landed it prints its pick and the
// marker check reads `absent`.
//
// **Moot since S124's rebase onto S122's landing (2026-11-05).** DEC-354 withdrew
// the clamp question, and on that tree, with the same 124 rows, the clamp rule
// answers +40 with no pair. Run there, this program finds none either: the base
// reads TARGET at +2.156e-4 at +40, the rule wants 6.47e-4, and none of the 784
// eligible pairs of all_test_fens() reaches it (the largest 5.12e-4), so it
// exits 1 by its own clause. It
// stays as the record of the reading on d83d6c0 (adocs/data/S124_rebase_goldens.txt).
//
// WHAT WAS RED. S124's coverage rows (the 18 rows of its block) put a pair 0.75 cp from
// the clamp's kink at the case's CLAMP_PUSH +30, and adocs/data/S083_tuner_preconditions.cpp
// `clamp` then answered no push from +10 to +200. Its first push by kink distance among
// the rest, +40 (six rows bound, every row 13.0 from the kink), fails one clause alone,
// the finite difference: 3.40e-7 at parameter 996, `passed_king_enemy_mg[5]`, against
// the rule's 2e-7 at h 0.005. The rows are DEC-316's answer, the coordinator's ruling of
// 2026-11-04: the metric, h, the case's 1e-6 and the rule's 2e-7 unmoved, one pair.
//
// THE MODE, S101's rule at the clamp case's setting. Where S101's reads the gradient at
// the shipped constants, this reads it where the clamp case reads its finite
// differences: the shipped constants with PUSH = 40 on each of the eighteen king safety
// weights, the case's cycling labels, K = 1. Where S101's eligibility is S121's, this
// adds the clamp case's own precondition at that push: each of the pair sits more than
// a centipawn from the clamp's kink there. MULTIPLE stays 3: by the 1 / |gradient| law
// at a fixed third derivative 3.40e-7 needs 1.70 to reach 2e-7, and 3 reads 1.13e-7.
// The rest is S101's: candidates all_test_fens() in sorted order, a position the list
// holds either way round skipped; for each eligible pair, in order, appended after the
// first BASE_ROWS rows with the labels continuing, gradient() at the mode's setting
// over every row; the pick the first pair putting |dE/d TARGET| at or above MULTIPLE
// times the base's. One pair; none qualifying, the program says so and exits 1. Nothing
// about the finite difference enters the rule; the case's relative error at TARGET is
// printed after the pick, a reading.
//
// IT RE-DERIVES ITS OWN PAIR (DEC-142). The base is the fixture's first BASE_ROWS = 124
// rows, every row before the pair, asserted against the pair's marker comment in
// tests/test_eval_positions.hpp when the marker is there, read from S124_POSITIONS_HPP
// (default the source tree's, under CHESSO_SOURCE_DIR); with the pair landed, the pick
// must be the landed pair, the two rows after the base, or the program exits 1.
//
// Built from the repository root after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -DCHESSO_SOURCE_DIR=\"$PWD\"
//       -I src -I tools -I tests -isystem tests/doctest/doctest -isystem
//       tests/third_party/nlohmann adocs/data/S124_gradient_rows.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S124_gradient_rows
//
//   build/S124_gradient_rows
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

using tuner_layout::KING_SAFETY_COUNT;
using tuner_layout::KS_EG_BASE;
using tuner_layout::KS_MG_BASE;
using tuner_layout::PARAM_COUNT;
using tuner_layout::PKE_MG_BASE;
using tuner_trace::dataset_t;

constexpr size_t TARGET = PKE_MG_BASE + 5;  // passed_king_enemy_mg[5], parameter 996
constexpr double MULTIPLE = 3.0;
constexpr size_t BASE_ROWS = 124;
constexpr double PUSH = 40.0;  // on each king safety weight, the clamp case's setting
constexpr double H = 0.005;    // the clamp rule's step
constexpr int SCORE_LIMIT = 800;  // centipawns, White relative, either way
const char* const BLOCK_MARKER = "DEC-316's pair at S124's landing";

// The mode's setting: the shipped constants with PUSH on every king safety weight.
std::vector<double> setting()
{
  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
    params[KS_MG_BASE + t] += PUSH;
    params[KS_EG_BASE + t] += PUSH;
  }
  return params;
}

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

  std::vector<double> params = setting();

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

// The clamp rule's finite difference at TARGET alone, at its h and the mode's setting. A
// reading printed after the pick, no part of the rule.
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

  std::vector<double> params = setting();

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
  const double h = H;
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
  // phase 1: each king's x per half, max(0, x)^2 / 2048 taken off its side.
  // Written on d83d6c0 this copy stopped above, which read the same while the
  // curve sat at its seed; S124's rebase onto S122's landing adds it, as S122's
  // phase 2 did for the other tools' copies (DEC-354).
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

// S121's eligibility at the shipped constants, and the clamp case's precondition at
// the mode's setting: a centipawn or more from the kink.
bool eligible(const std::string& fen,
              const std::string& mirror,
              const double* params,
              const double* pushed)
{
  tuner_trace::row_trace_t trace;
  tuner_trace::row_trace_t mirrored;
  if (!trace_fen(fen, &trace) || !trace_fen(mirror, &mirrored)) { return false; }

  const double margin = LAZY_EVAL_MARGIN;
  const auto clear = [&](const tuner_trace::row_trace_t& t) {
    return std::fabs(std::fabs(stage_two(t, pushed)) - margin) > 1.0;
  };

  return trace.phase >= 1 && trace.phase <= GAME_PHASE_MAX - 1 && clear(trace) &&
         clear(mirrored) &&
         std::fabs(stage_two(trace, params)) < LAZY_EVAL_MARGIN - 1.0 &&
         std::fabs(stage_two(mirrored, params)) < LAZY_EVAL_MARGIN - 1.0 &&
         std::abs(trace.engine_white) <= SCORE_LIMIT &&
         std::abs(mirrored.engine_white) <= SCORE_LIMIT &&
         loads_either_side(fen) && loads_either_side(mirror);
}

}  // namespace


TEST_CASE("S124: pick the fixture rows DEC-316 owes passed_king_enemy_mg[5] at +40")
{
  initialize_game_const_data(&game);

  const std::string path =
      (std::filesystem::temp_directory_path() / "chesso_S124_gradient_rows.tsv")
          .string();
  const std::vector<std::string>& landed = test_eval_positions::positions;
  REQUIRE(landed.size() >= BASE_ROWS);
  const std::vector<std::string> base(landed.begin(), landed.begin() + BASE_ROWS);

  // The base, asserted against the pair's marker comment where it stands.
  {
    const char* header_path = std::getenv("S124_POSITIONS_HPP");
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
  const std::vector<double> pushed = setting();

  std::vector<double> g;
  REQUIRE(gradient_of(base, path, &g));

  const double wanted = MULTIPLE * std::fabs(g[TARGET]);
  printf("base, %zu rows, +%.0f on the king safety weights: parameter %zu analytic "
         "%+.6e, the rule wants at least %.6e\n",
         base.size(), PUSH, TARGET, g[TARGET], wanted);

  size_t tried = 0;
  for (const std::string& fen : all_test_fens()) {
    const std::string mirror = canonical_castling(mirror_fen(fen));
    bool known = false;
    for (const std::string& row : base) {
      if (row == fen || row == mirror) { known = true; }
    }
    if (known || !eligible(fen, mirror, params.data(), pushed.data())) { continue; }

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
      printf("the clamp rule's relative error at parameter %zu, +%.0f, h %.3f, no part of "
             "the rule:\n",
             TARGET, PUSH, H);
      for (const target_reading_t* r : {&before, &after}) {
        printf("  %s: %.5e, analytic %+.6e, numeric %+.6e, difference %.3e\n",
               (r == &before) ? "base" : "with the pair", r->relative, r->analytic,
               r->numeric, std::fabs(r->numeric - r->analytic));
      }
      if (landed.size() > BASE_ROWS) {
        const bool same = landed.size() >= BASE_ROWS + 2 &&
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
