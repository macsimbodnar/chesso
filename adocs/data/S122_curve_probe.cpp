// S122 phase 1: the curve's planted recovery at a given budget and step, on
// tests/test_tuner_gradient.cpp's trace_positions() -- the curated list both ways
// round and the JSON suites, 2908 rows -- and its two plants, printing the worst
// miss a half every REPORT epochs. Built against a tree's trace library:
//
//   g++ -std=gnu++20 -O3 -march=native -DCHESSO_TRACE -DNDEBUG \
//       -DCHESSO_SOURCE_DIR=\"$TREE\" -I$TREE/src -I$TREE/tools -I$TREE/tests \
//       -I$TREE/build/generated -isystem $TREE/tests/doctest/doctest \
//       adocs/data/S122_curve_probe.cpp $TREE/build/src/libchesso_engine_trace.a \
//       -pthread -o curve_probe
//   ./curve_probe EPOCHS LR C [REPORT]     # C 30 or 0: the two plants of the case
//
// The step file's table (2026-11-02, phase 1) is this at 24000, 40000 and 80000
// epochs, lr 1 and 0.2. The same fit as check_recovery(): K 1, the curve alone
// free, Adam, four threads.
#define DOCTEST_CONFIG_DISABLE
#include <doctest.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
#include "tuner_groups.hpp"
#include "tuner_layout.hpp"
#include "tuner_optimiser.hpp"
#include "tuner_trace.hpp"
using namespace tuner_layout;
int main(int argc, char** argv)
{
  const int epochs = atoi(argv[1]);
  const double lr = atof(argv[2]);
  const double c_plant = atof(argv[3]);
  const int report = (argc > 4) ? atoi(argv[4]) : 1000;
  game_t game; initialize_game_const_data(&game);
  std::vector<std::string> fens;
  for (const auto& f : test_eval_positions::positions) { fens.push_back(f); fens.push_back(mirror_fen(f)); }
  for (const auto& f : all_test_fens()) fens.push_back(f);
  tuner_trace::dataset_t data;
  for (const auto& f : fens) {
    load_FEN(f, &game);
    tuner_trace::row_trace_t t; std::string e;
    tuner_trace::trace_board(&game.board, &t, &e);
    tuner_trace::append_row(t, 0.5f, 0, 0, &data);
  }
  const auto& model = tuner_trace::engine_model();
  std::vector<double> shipped(PARAM_COUNT); starting_params(shipped.data());
  const double mg[12] = {c_plant, 14, 11, 9, 22, 7, 16, 13, 15, 19, 10, -8};
  const double eg[12] = {c_plant == 30 ? 20.0 : c_plant, 7, 6, 5, 12, 4, 9, 7, 8, 10, 6, -5};
  std::vector<double> planted = shipped;
  for (int i = 0; i < 12; ++i) { planted[KSC_MG_BASE + i] = mg[i]; planted[KSC_EG_BASE + i] = eg[i]; }
  const double k = 1.0;
  std::vector<double> target(data.size());
  for (size_t r = 0; r < data.size(); ++r) target[r] = tuner_target::sigmoid(tuner_trace::score(model, data, r, planted.data()), k);
  std::vector<uint8_t> mask; tuner_groups::free_mask("ks_curve", &mask);
  std::vector<uint32_t> index(data.size()); for (size_t i = 0; i < index.size(); ++i) index[i] = i;
  std::vector<double> p = shipped; const std::vector<double> start = shipped;
  tuner_optimiser::state_t state(PARAM_COUNT); std::vector<double> grad;
  for (int e = 1; e <= epochs; ++e) {
    tuner_trace::gradient(model, data, p.data(), k, index, 0, data.size(), 4, target, &grad);
    tuner_optimiser::descend(grad, mask, start.data(), lr, &state, p.data());
    if (e % report == 0 || e == epochs) {
      double worst_mg = 0, worst_eg = 0; int at_mg = 0, at_eg = 0;
      for (int i = 0; i < 12; ++i) {
        double a = std::fabs(p[KSC_MG_BASE + i] - mg[i]), b = std::fabs(p[KSC_EG_BASE + i] - eg[i]);
        if (a > worst_mg) { worst_mg = a; at_mg = i; }
        if (b > worst_eg) { worst_eg = b; at_eg = i; }
      }
      const double err = tuner_trace::error_range(model, data, p.data(), k, index, 0, data.size(), 4, target);
      printf("epoch %6d error %.3e worst mg %.3f at %d, eg %.3f at %d\n", e, err, worst_mg, at_mg, worst_eg, at_eg);
    }
  }
  printf("fitted mg:"); for (int i = 0; i < 12; ++i) printf(" %.2f", p[KSC_MG_BASE + i]); printf("\n");
  printf("fitted eg:"); for (int i = 0; i < 12; ++i) printf(" %.2f", p[KSC_EG_BASE + i]); printf("\n");
  return 0;
}
