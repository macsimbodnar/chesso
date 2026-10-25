// S083. The two measured preconditions in tests/test_tuner_gradient.cpp,
// re-derived by S262's procedure at whatever constants the tree is built with
// (DEC-142: a measured number with no script is a finding; this is the
// script). Both are inputs the cases establish at run time, not goldens, and
// neither case's assertions are touched by what this prints.
//
//   clamp  "the clamp's derivative is exact where it binds": the push added to
//          each of the eighteen king safety weights so that the clamp binds on
//          some fixture rows and not all. S262 measured +40 at the
//          constants it shipped with (bee84d8): one row of 34 bound, at
//          263.7 against 184, the nearest other at 150; finite differences
//          1.72e-7 at h = 0.005, against 6.8e-7 at h = 0.01, "too near the
//          tolerance to survive a refit".
//          For each push from 10 to 200 in steps of 10 this prints the rows
//          bound, the largest unbound |stage two|, the smallest distance to the
//          kink, the finite-difference worst at h = 0.005 over all 823, and,
//          on the bound rows alone, the clamped block's largest gradient and
//          the rest's. THE RULE: among the pushes in that grid at which
//          (1) the clamp binds on at least one row and not on all, (2) every
//          row sits more than one centipawn from the kink, (3) the worst
//          relative error is at most 2e-7 -- S262's 1.72e-7 kept a factor of
//          five under the case's 1e-6, and 6.8e-7 was refused as too near it --
//          and (4) on the bound rows the clamped block reads exactly 0 and the
//          rest above 1e-9, the one whose nearest row to the kink is farthest
//          from it, ties to the smaller push: the case's precondition is that
//          clearance, and the most of it is what survives the next refit.
//          At S083's parent b860a8a, whose constants are the ones S262
//          measured at (S076's fit; no constant has moved since), the rule
//          answers S262's +40 (one row bound at 263.7, the nearest other at
//          150, every row 34.0 from the kink), which is the check that it
//          re-derives the input the case carries. Written first as "the
//          smallest push", which answers +30 there and so does not reproduce
//          S262's pick; changed before the paste's constants were read.
//          The case's comment then quotes the chosen push's row count and
//          the two distances.
//
//   walk   "the fit holds every gauge still and no score sees the hold": the
//          plain update's walk along the seven gauges, per optimiser, 300
//          epochs over the gauge fixture, Adam at 1 and AdaGrad at 10 -- the
//          case's own run, step for step. S262 read the smallest walk 0.32 a
//          square under Adam and 0.20 under AdaGrad (the rook's) at f57b92f's
//          constants -- this program reads 0.3179 and 0.1957 at b860a8a's --
//          and set the case's floor at 0.1, "far above the hold's 1e-9 and
//          below the walk". This prints each direction's plain and
//          held shift, the runs' largest score difference and how far the
//          plain run moved; the precondition holds where every plain shift
//          clears the case's floor, WALK_FLOOR below. The script picks no
//          input for this case: the walk is a reading of the constants, and a
//          walk under the floor is reported, not answered by a lower floor.
//          At S083's constants the walk read 0.0587 under AdaGrad
//          (psqt_eg[king]), under the 0.1 of then; the owner lowered the
//          case's floor to 0.01 (DEC-313 (5)), and WALK_FLOOR follows the
//          case so this program reports against what the case asserts.
//
// The fixture is the case's, byte for byte: test_eval_positions::positions
// with the case's cycling labels, and for the walk the four rows
// write_gauge_fixture() appends.
//
// Built from the repository root, after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -I src -I tools
//       -I tests adocs/data/S083_tuner_preconditions.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S083_tuner_preconditions
//
//   build/S083_tuner_preconditions clamp|walk|both SCRATCH_DIR
//
// SCRATCH_DIR takes the two fixture files; they are removed on exit.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "search_params.hpp"
#include "test_eval_positions.hpp"
#include "tuner_groups.hpp"
#include "tuner_layout.hpp"
#include "tuner_optimiser.hpp"
#include "tuner_trace.hpp"

namespace
{

using tuner_layout::EG_BASE;
using tuner_layout::KING_SAFETY_COUNT;
using tuner_layout::KS_EG_BASE;
using tuner_layout::KS_MG_BASE;
using tuner_layout::MATERIAL_COUNT;
using tuner_layout::MG_BASE;
using tuner_layout::MOB_MG_BASE;
using tuner_layout::PARAM_COUNT;
using test_eval_positions::positions;
using tuner_trace::dataset_t;

// tests/test_tuner_gradient.cpp's `> 0.01` on every plain shift (DEC-313 (5)).
constexpr double WALK_FLOOR = 0.01;

int engine_phase(const std::string& fen)
{
  game_t game;
  initialize_game_const_data(&game);
  if (!load_FEN(fen, &game)) {
    fprintf(stderr, "load_FEN refused %s\n", fen.c_str());
    exit(1);
  }
  return game_phase(&game.board);
}

// tests/test_tuner_gradient.cpp's write_fixture() at phase_shift 0.
bool write_fixture(const std::string& path)
{
  std::ofstream out(path);
  if (!out) { return false; }

  static const double results[3] = {1.0, 0.5, 0.0};
  int score = -280;

  for (size_t i = 0; i < positions.size(); ++i) {
    const std::string& fen = positions[i];
    out << fen << '\t' << results[i % 3] << '\t' << score << '\t'
        << engine_phase(fen) % (GAME_PHASE_MAX + 1) << '\n';
    score += 41;
    if (score > 300) { score -= 600; }
  }

  return out.good();
}

// tests/test_tuner_gradient.cpp's write_gauge_fixture().
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

bool load_fixture(const std::string& path, dataset_t* data)
{
  tuner_trace::load_report_t report;
  std::string error;
  if (!tuner_trace::load(path, data, &report, &error, 1)) {
    fprintf(stderr, "load refused %s: %s\n", path.c_str(), error.c_str());
    return false;
  }
  if (report.mismatches != 0) {
    fprintf(stderr, "load: %zu trace mismatches\n",
            static_cast<size_t>(report.mismatches));
    return false;
  }
  return true;
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
  for (size_t i = 0; i < data.size(); ++i) { target[i] = data.result[i]; }
  return target;
}

// The case file's check_differences(), every parameter.
struct difference_check_t
{
  double worst = 0.0;
  size_t worst_param = 0;
  size_t checked = 0;
};

difference_check_t check_differences(const dataset_t& data,
                                     std::vector<double> params,
                                     double k,
                                     const std::vector<uint32_t>& index,
                                     const std::vector<double>& target,
                                     const std::vector<double>& analytic,
                                     double h)
{
  const tuner_trace::model_t& model = tuner_trace::engine_model();
  difference_check_t out;

  for (size_t p = 0; p < model.param_count; ++p) {
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

// The case file's stage_two().
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

int clamp_sweep(const std::string& dir)
{
  const std::string path = dir + "/S083_tuner_preconditions_clamp.tsv";
  if (!write_fixture(path)) { return 1; }
  dataset_t data;
  const bool loaded = load_fixture(path, &data);
  std::remove(path.c_str());
  if (!loaded) { return 1; }

  const double margin = static_cast<double>(LAZY_EVAL_MARGIN);
  std::vector<double> start(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(start.data());

  double clear = 0.0;
  for (size_t row = 0; row < data.size(); ++row) {
    clear = std::max(clear, std::fabs(stage_two(data, row, start.data())));
  }
  printf("clamp: %zu fixture rows, LAZY_EVAL_MARGIN %d; at the built "
         "constants the largest |stage two| is %.1f (the case \"sits "
         "clear\" needs < %.1f)\n",
         data.size(), static_cast<int>(LAZY_EVAL_MARGIN), clear, margin - 1.0);
  printf("%5s %6s %10s %10s %11s %10s %10s %10s  %s\n", "push", "bound",
         "bound at", "next", "kink dist", "fd worst", "clamped", "rest",
         "rule");

  const std::vector<uint32_t> index = every_row(data);
  const std::vector<double> target = results_of(data);
  const double k = 1.0;
  int chosen = 0;
  double chosen_clearance = -1.0;
  std::vector<std::string> lines;
  std::vector<int> pushes;

  for (int push = 10; push <= 200; push += 10) {
    std::vector<double> params = start;
    for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
      params[KS_MG_BASE + t] += push;
      params[KS_EG_BASE + t] += push;
    }

    std::vector<uint32_t> binding;
    double bound_at = 0.0;
    double next = 0.0;
    double kink = 1e300;
    for (size_t row = 0; row < data.size(); ++row) {
      const double two = std::fabs(stage_two(data, row, params.data()));
      kink = std::min(kink, std::fabs(two - margin));
      if (two > margin) {
        binding.push_back(static_cast<uint32_t>(row));
        bound_at = std::max(bound_at, two);
      } else {
        next = std::max(next, two);
      }
    }

    std::vector<double> analytic;
    tuner_trace::gradient(data, params.data(), k, index, 0, data.size(), 1,
                          target, &analytic);
    const difference_check_t check =
        check_differences(data, params, k, index, target, analytic, 0.005);

    double clamped_block = -1.0;
    double elsewhere = -1.0;
    if (!binding.empty()) {
      std::vector<double> on_binding;
      tuner_trace::gradient(data, params.data(), k, binding, 0, binding.size(),
                            1, target, &on_binding);
      clamped_block = 0.0;
      elsewhere = 0.0;
      for (size_t p = 0; p < PARAM_COUNT; ++p) {
        if (p >= MOB_MG_BASE && p < KS_EG_BASE + KING_SAFETY_COUNT) {
          clamped_block = std::max(clamped_block, std::fabs(on_binding[p]));
        } else {
          elsewhere = std::max(elsewhere, std::fabs(on_binding[p]));
        }
      }
    }

    const bool binds = !binding.empty() && binding.size() < data.size();
    const bool clear_of_kink = kink > 1.0;
    const bool fd = check.worst <= 2e-7;
    const bool silent = binds && clamped_block == 0.0 && elsewhere > 1e-9;
    const bool ok = binds && clear_of_kink && fd && silent;
    std::string why = ok ? "ok" : "";
    if (!binds) { why += "binds on none or all; "; }
    if (!clear_of_kink) { why += "a row within 1 cp of the kink; "; }
    if (!fd) { why += "fd past 2e-7; "; }
    if (binds && !silent) { why += "bound rows not silent; "; }
    if (ok && kink > chosen_clearance) {
      chosen = push;
      chosen_clearance = kink;
    }

    char line[256];
    snprintf(line, sizeof line,
             "%+5d %6zu %10.1f %10.1f %11.2f %10.3g %10.3g %10.3g  %s", push,
             binding.size(), bound_at, next, kink, check.worst, clamped_block,
             elsewhere, why.c_str());
    lines.push_back(line);
    pushes.push_back(push);
  }

  for (size_t i = 0; i < lines.size(); ++i) {
    printf("%s%s\n", lines[i].c_str(),
           (pushes[i] == chosen) ? "  <- CHOSEN" : "");
  }

  if (chosen == 0) {
    printf("clamp: no push from +10 to +200 satisfies the rule\n");
    return 2;
  }
  printf("clamp: the rule picks +%d, every row %.2f from the kink\n", chosen,
         chosen_clearance);
  return 0;
}

int walk(const std::string& dir)
{
  const std::string path = dir + "/S083_tuner_preconditions_gauge.tsv";
  if (!write_gauge_fixture(path)) { return 1; }
  dataset_t data;
  const bool loaded = load_fixture(path, &data);
  std::remove(path.c_str());
  if (!loaded) { return 1; }

  std::vector<double> start(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(start.data());
  const std::vector<uint32_t> index = every_row(data);
  const std::vector<double> target = results_of(data);

  std::vector<uint8_t> mask;
  if (!tuner_groups::free_mask("all", &mask)) { return 1; }

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

  const double k = 1.0;
  const int epochs = 300;
  bool all_clear = true;

  printf("walk: %zu gauge-fixture rows, %d epochs, K %.1f\n", data.size(),
         epochs, k);

  for (const tuner_optimiser::kind_t kind :
       {tuner_optimiser::ADAM, tuner_optimiser::ADAGRAD}) {
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

    printf("  %s at lr %g\n", tuner_optimiser::kind_name(kind), lr);
    double smallest = 1e300;
    std::string smallest_name;
    for (const direction_t& direction : directions) {
      const double p = level_shift(direction, plain);
      const double h = level_shift(direction, held);
      printf("    %-22s plain %+9.4f  held %+10.3g  %s\n",
             direction.name.c_str(), p, h,
             std::fabs(p) > WALK_FLOOR ? "" : "<- under the case's floor");
      if (std::fabs(p) < smallest) {
        smallest = std::fabs(p);
        smallest_name = direction.name;
      }
      if (!(std::fabs(p) > WALK_FLOOR)) { all_clear = false; }
    }

    double apart = 0.0;
    double travelled = 0.0;
    for (size_t row = 0; row < data.size(); ++row) {
      const double h = tuner_trace::score(data, row, held.data());
      const double p = tuner_trace::score(data, row, plain.data());
      const double s = tuner_trace::score(data, row, start.data());
      apart = std::max(apart, std::fabs(h - p));
      travelled = std::max(travelled, std::fabs(p - s));
    }
    printf("    smallest plain walk %.4f (%s); runs apart %.3g; plain "
           "travelled %.1f\n",
           smallest, smallest_name.c_str(), apart, travelled);
  }

  printf("walk: %s %g\n",
         all_clear ? "every plain shift clears the case's floor"
                   : "a plain shift is under the case's floor",
         WALK_FLOOR);
  return all_clear ? 0 : 2;
}

}  // namespace

int main(int argc, char** argv)
{
  if (argc != 3 || (strcmp(argv[1], "clamp") != 0 &&
                    strcmp(argv[1], "walk") != 0 &&
                    strcmp(argv[1], "both") != 0)) {
    fprintf(stderr, "usage: S083_tuner_preconditions clamp|walk|both DIR\n");
    return 1;
  }

  const std::string what = argv[1];
  int rc = 0;
  if (what == "clamp" || what == "both") {
    rc = std::max(rc, clamp_sweep(argv[2]));
  }
  if (what == "walk" || what == "both") {
    rc = std::max(rc, walk(argv[2]));
  }
  return rc;
}
