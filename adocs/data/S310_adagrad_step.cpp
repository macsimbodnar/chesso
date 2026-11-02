// S310. tests/test_tuner_gradient.cpp's ADAGRAD_HOLD_STEP, the step "the fit
// holds every gauge still and no score sees the hold" runs AdaGrad at, re-derived
// at whatever constants and fixture the tree is built with (DEC-142).
//
// What the case reads. It runs the descent twice from one start, held and plain,
// and asserts the two score alike: `apart`, the rows' largest score difference
// after 300 epochs, under 1e-6. The hold moves only along null directions, so the
// two differ by rounding alone. Where the descent is quiet that rounding stays at
// about 1e-12. Where it is chaotic the rounding grows until it passes any bound,
// with every gauge still held, and the case then measures the chaos and not the
// hold. A larger step goes chaotic sooner, and how large is quiet moves with the
// constants and the rows: at a step of 10, S101's fit read 4.96e-6 at epoch 300
// on its 108 gauge rows and 7.39e-12 on 110, and those 110 go chaotic by epoch
// 900. So a reading at one step and one epoch is not a margin.
//
// THE RULE. Written after a first sweep of this ladder had been read, so it did
// not predict the sweep; it is what is held to from here on.
//
//   Ladder: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30. AdaGrad's default, 30,
//   is the top.
//
//   Family: two fixtures, the case's gauge fixture and the same less the last
//   pair of tests/test_eval_positions.hpp, the rows S101's fit went chaotic
//   without. On each, the shipped constants, and SEEDS draws of every parameter
//   moved uniformly by up to each of 1, 3 and 10 centipawns: 98 starts, standing
//   in for the fit that has not happened yet.
//
//   Reading: `apart` every 25 epochs through 1200, four times the case's 300, at
//   every step and start. worst(step) is the largest of them.
//
//   Onset: the smallest step whose worst(step) passes QUIET, 1e-9, three decades
//   under the case's bound.
//
//   The step: the largest on the ladder at most half the onset at which the
//   shipped constants on the case's fixture clear the case's two preconditions
//   by a margin: every plain shift at least WALK_MARGIN times its floor, and the
//   scores' travel at least TRAVEL_MARGIN times its floor. Half, because the
//   onsets of the family's own members run from 5 to 10 today, a factor of two,
//   and a later fit is one more member; the largest, because a smaller step
//   walks the gauges less and sits further from the default a real fit runs at.
//
// The script reads the step the case carries out of the case's source, so run it
// from the repository root, and exits 2 if the rule answers anything else, or if
// it cannot read one. Re-run it whenever the constants or the fixture move. It
// is the step that follows, never the case's bound. Copied by hand and compared
// with nothing: the case's bound, its floors and its epochs below, and the
// fixture in write_gauge_fixture().
//
// Built from the repository root, after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -I src -I tools
//       -I tests adocs/data/S310_adagrad_step.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S310_adagrad_step
//
//   build/S310_adagrad_step SCRATCH_DIR
//
// SCRATCH_DIR takes the fixture files; they are removed on exit. About 150 s,
// one core and its spawned worker.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <random>
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

using test_eval_positions::positions;
using tuner_layout::EG_BASE;
using tuner_layout::MATERIAL_COUNT;
using tuner_layout::MG_BASE;
using tuner_layout::PARAM_COUNT;
using tuner_trace::dataset_t;

// The case's `apart < 1e-6`, `plain shift > 0.01` and `travelled > 50`.
constexpr double CASE_BOUND = 1e-6;
constexpr double WALK_FLOOR = 0.01;
constexpr double TRAVEL_FLOOR = 50.0;
constexpr int CASE_EPOCHS = 300;

constexpr double QUIET = 1e-9;
constexpr double WALK_MARGIN = 10.0;
constexpr double TRAVEL_MARGIN = 2.0;
constexpr int EPOCHS = 1200;
constexpr int READ_EVERY = 25;
constexpr int SEEDS = 16;

const std::vector<double> LADDER = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30};
const std::vector<double> NOISE = {1, 3, 10};

// A constant the case carries, read from its source. Exits 2 if it is not there.
double case_constant(const std::string& name)
{
  const std::string path = "tests/test_tuner_gradient.cpp";
  const std::string key = "constexpr double " + name + " = ";
  std::ifstream in(path);
  std::string line;

  while (std::getline(in, line)) {
    const size_t at = line.find(key);
    if (at != std::string::npos) { return std::stod(line.substr(at + key.size())); }
  }

  fprintf(stderr, "no %s in %s: run from the repository root\n", name.c_str(),
          path.c_str());
  exit(2);
}

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

// tests/test_tuner_gradient.cpp's write_gauge_fixture(), less the last `drop`
// rows of the curated list.
bool write_gauge_fixture(const std::string& path, size_t drop)
{
  std::ofstream out(path);
  if (!out) { return false; }

  static const double results[3] = {1.0, 0.5, 0.0};
  int score = -280;

  for (size_t i = 0; i + drop < positions.size(); ++i) {
    const std::string& fen = positions[i];
    out << fen << '\t' << results[i % 3] << '\t' << score << '\t'
        << engine_phase(fen) % (GAME_PHASE_MAX + 1) << '\n';
    score += 41;
    if (score > 300) { score -= 600; }
  }

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
  const bool ok = tuner_trace::load(path, data, &report, &error, 1);
  if (!ok) { fprintf(stderr, "load refused %s: %s\n", path.c_str(), error.c_str()); }
  std::remove(path.c_str());
  return ok && report.mismatches == 0;
}

// The case's seven directions, from the layout.
std::vector<std::vector<size_t>> directions()
{
  std::vector<std::vector<size_t>> out;

  for (size_t type = 0; type < MATERIAL_COUNT; ++type) {
    std::vector<size_t> level;
    for (const size_t base : {MG_BASE, EG_BASE}) {
      for (size_t square = 0; square < 64; ++square) {
        if (type == 0 && (square < 8 || square >= 56)) { continue; }
        level.push_back(base + type * 64 + square);
      }
    }
    out.push_back(level);
  }

  for (const size_t base : {MG_BASE, EG_BASE}) {
    std::vector<size_t> level;
    for (size_t square = 0; square < 64; ++square) {
      level.push_back(base + 5 * 64 + square);
    }
    out.push_back(level);
  }

  return out;
}

struct run_t
{
  std::vector<double> by_read;  // `apart` at READ_EVERY, 2 * READ_EVERY, ...
  double worst = 0.0;
  double at_case_epochs = 0.0;
  double travelled = 0.0;      // at the case's epochs
  double smallest_walk = 0.0;  // at the case's epochs
  double largest_hold = 0.0;   // at the case's epochs
};

// The case's two runs, step for step, read as it goes.
run_t run(const dataset_t& data,
          const std::vector<double>& start,
          tuner_optimiser::kind_t kind,
          double lr,
          int epochs)
{
  std::vector<uint32_t> index(data.size());
  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
    target[i] = data.result[i];
  }

  std::vector<uint8_t> mask;
  if (!tuner_groups::free_mask("all", &mask)) { exit(1); }

  const std::vector<std::vector<size_t>> levels = directions();
  const double k = 1.0;

  std::vector<double> held = start;
  std::vector<double> plain = start;
  tuner_optimiser::state_t held_state(PARAM_COUNT, kind);
  tuner_optimiser::state_t plain_state(PARAM_COUNT, kind);
  std::vector<double> grad;

  run_t out;

  for (int epoch = 1; epoch <= epochs; ++epoch) {
    tuner_trace::gradient(data, held.data(), k, index, 0, data.size(), 1, target,
                          &grad);
    tuner_optimiser::descend(grad, mask, start.data(), lr, &held_state,
                             held.data());
    tuner_trace::gradient(data, plain.data(), k, index, 0, data.size(), 1,
                          target, &grad);
    tuner_optimiser::update(grad, mask, lr, &plain_state, plain.data());

    if (epoch % READ_EVERY != 0) { continue; }

    double apart = 0.0;
    double travelled = 0.0;
    for (size_t row = 0; row < data.size(); ++row) {
      const double h = tuner_trace::score(data, row, held.data());
      const double p = tuner_trace::score(data, row, plain.data());
      const double s = tuner_trace::score(data, row, start.data());
      apart = std::max(apart, std::fabs(h - p));
      travelled = std::max(travelled, std::fabs(p - s));
    }

    out.by_read.push_back(apart);
    out.worst = std::max(out.worst, apart);

    if (epoch == CASE_EPOCHS) {
      out.at_case_epochs = apart;
      out.travelled = travelled;
      out.smallest_walk = 1e300;

      for (const std::vector<size_t>& level : levels) {
        double plain_sum = 0.0;
        double held_sum = 0.0;
        for (const size_t column : level) {
          plain_sum += plain[column] - start[column];
          held_sum += held[column] - start[column];
        }
        const double n = static_cast<double>(level.size());
        out.smallest_walk = std::min(out.smallest_walk, std::fabs(plain_sum / n));
        out.largest_hold = std::max(out.largest_hold, std::fabs(held_sum / n));
      }
    }
  }

  return out;
}

// The draw is mt19937_64's bits, which the standard fixes, and not a
// distribution's, which it does not.
std::vector<double> drawn(const std::vector<double>& shipped,
                          double noise,
                          uint64_t seed)
{
  std::mt19937_64 bits(seed);
  std::vector<double> out = shipped;
  for (double& value : out) {
    const double unit = static_cast<double>(bits() >> 11) * 0x1p-53;
    value += (2.0 * unit - 1.0) * noise;
  }
  return out;
}

void print_table(const char* title,
                 const dataset_t& data,
                 const std::vector<double>& start)
{
  static const int shown[] = {25,  50,  100, 150, 200, 250, 300,  400,
                              500, 600, 700, 800, 900, 1000, 1100, 1200};

  printf("\n== %s, %zu rows: apart by epoch\n", title, data.size());
  printf("%6s", "step");
  for (const int epoch : shown) { printf(" %7d", epoch); }
  printf("\n");

  for (const double lr : LADDER) {
    const run_t r = run(data, start, tuner_optimiser::ADAGRAD, lr, EPOCHS);
    printf("%6g", lr);
    for (const int epoch : shown) {
      printf(" %7.0e", r.by_read[static_cast<size_t>(epoch / READ_EVERY - 1)]);
    }
    printf("\n");
  }

  const run_t adam = run(data, start, tuner_optimiser::ADAM, 1.0, EPOCHS);
  printf("%6s", "adam 1");
  for (const int epoch : shown) {
    printf(" %7.0e", adam.by_read[static_cast<size_t>(epoch / READ_EVERY - 1)]);
  }
  printf("\n");
}

}  // namespace

int main(int argc, char** argv)
{
  if (argc != 2) {
    fprintf(stderr, "usage: S310_adagrad_step SCRATCH_DIR\n");
    return 1;
  }

  const double case_step = case_constant("ADAGRAD_HOLD_STEP");

  const std::string gauge = std::string(argv[1]) + "/S310_gauge.tsv";
  const std::string less = std::string(argv[1]) + "/S310_gauge_less.tsv";
  if (!write_gauge_fixture(gauge, 0) || !write_gauge_fixture(less, 2)) {
    return 1;
  }

  dataset_t full;
  dataset_t short_of_pair;
  if (!load_fixture(gauge, &full) || !load_fixture(less, &short_of_pair)) {
    return 1;
  }

  std::vector<double> shipped(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(shipped.data());

  printf("S310: the case's fixture %zu rows, less its last pair %zu; %zu "
         "parameters; bound %g, QUIET %g\n",
         full.size(), short_of_pair.size(), static_cast<size_t>(PARAM_COUNT),
         CASE_BOUND, QUIET);

  print_table("the shipped constants, the case's fixture", full, shipped);
  print_table("the shipped constants, the fixture less its last pair",
              short_of_pair, shipped);

  printf("\n== the family, worst apart at any read through epoch %d; a, the "
         "case's fixture; b, less its last pair\n",
         EPOCHS);
  printf("%6s %8s %8s %8s %8s %8s %8s %8s %8s %9s %5s | %8s %8s %8s %9s %9s\n",
         "step", "a ship", "a +-1", "a +-3", "a +-10", "b ship", "b +-1",
         "b +-3", "b +-10", "worst", ">Q", "at 300", "walk", "travel",
         "hold", "min walk");

  double onset = 0.0;
  double member_onset[8] = {};
  std::vector<run_t> shipped_runs;
  std::vector<double> worsts;

  for (const double lr : LADDER) {
    double cell[8] = {};
    double worst = 0.0;
    double smallest_family_walk = 1e300;
    int over = 0;
    int column = 0;
    run_t case_run;

    for (const dataset_t* data : {&full, &short_of_pair}) {
      const run_t ship = run(*data, shipped, tuner_optimiser::ADAGRAD, lr, EPOCHS);
      if (data == &full) { case_run = ship; }
      cell[column] = ship.worst;
      worst = std::max(worst, ship.worst);
      smallest_family_walk = std::min(smallest_family_walk, ship.smallest_walk);
      if (ship.worst > QUIET) { over++; }
      column++;

      for (size_t n = 0; n < NOISE.size(); ++n) {
        for (int seed = 1; seed <= SEEDS; ++seed) {
          const run_t r =
              run(*data, drawn(shipped, NOISE[n], 1000 * (n + 1) + seed),
                  tuner_optimiser::ADAGRAD, lr, EPOCHS);
          cell[column] = std::max(cell[column], r.worst);
          worst = std::max(worst, r.worst);
          smallest_family_walk = std::min(smallest_family_walk, r.smallest_walk);
          if (r.worst > QUIET) { over++; }
        }
        column++;
      }
    }

    printf("%6g %8.1e %8.1e %8.1e %8.1e %8.1e %8.1e %8.1e %8.1e %9.2e %5d | "
           "%8.2e %8.4f %8.1f %9.2e %9.4f\n",
           lr, cell[0], cell[1], cell[2], cell[3], cell[4], cell[5], cell[6],
           cell[7], worst, over, case_run.at_case_epochs, case_run.smallest_walk,
           case_run.travelled, case_run.largest_hold, smallest_family_walk);
    fflush(stdout);

    shipped_runs.push_back(case_run);
    worsts.push_back(worst);
    if (onset == 0.0 && worst > QUIET) { onset = lr; }
    for (int member = 0; member < 8; ++member) {
      if (member_onset[member] == 0.0 && cell[member] > QUIET) {
        member_onset[member] = lr;
      }
    }
  }

  printf("first step past QUIET, by member: %6s", "");
  for (const double member : member_onset) { printf(" %g", member); }
  printf("\n");

  if (onset == 0.0) {
    printf("\nno step on the ladder passes QUIET: the rule has no onset\n");
    return 2;
  }

  double chosen = 0.0;
  for (size_t i = 0; i < LADDER.size(); ++i) {
    if (LADDER[i] > onset / 2.0) { break; }
    if (shipped_runs[i].smallest_walk >= WALK_MARGIN * WALK_FLOOR &&
        shipped_runs[i].travelled >= TRAVEL_MARGIN * TRAVEL_FLOOR) {
      chosen = LADDER[i];
    }
  }

  printf("\nonset %g; half %g; the rule picks %g, the case carries %g\n", onset,
         onset / 2.0, chosen, case_step);

  for (size_t i = 0; i < LADDER.size(); ++i) {
    if (LADDER[i] != chosen) { continue; }
    printf("at %g on the case's fixture: apart %.3g at epoch %d, worst %.3g "
           "through %d; every read of the family at most %.3g, %.1f decades "
           "under the bound; smallest plain walk %.4f, %.0f times its floor; "
           "travelled %.1f; holds at most %.3g\n",
           chosen, shipped_runs[i].at_case_epochs, CASE_EPOCHS,
           shipped_runs[i].worst, EPOCHS, worsts[i],
           std::log10(CASE_BOUND / worsts[i]), shipped_runs[i].smallest_walk,
           shipped_runs[i].smallest_walk / WALK_FLOOR, shipped_runs[i].travelled,
           shipped_runs[i].largest_hold);
  }

  return (chosen == case_step) ? 0 : 2;
}
