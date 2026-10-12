// S275. The fit's descent twice over one corpus, from the shipped constants:
// Adam alone, which is the tuner before S275 step for step, and descend(), the
// tuner since -- Adam and then the seven gauges held. Reads the vectors at the
// last epoch rather than the best reported one, so the walk along the null
// directions is visible on real labels even where held-out error never beats
// the start and `tuner` emits the start unchanged.
//
// Prints, every --report epochs, both runs' training error and the largest
// difference between them; at the end, per null direction, the level shift
// (mean change over the squares it moves) and the material shift of each run,
// the largest score difference between the two runs over every row, and how
// far the plain run moved the scores at all.
//
// Built from the repository root, after `cmake --build build`, in one line:
//
//   g++ -O3 -std=gnu++20 -march=native -I src -I tools -I build/generated
//       adocs/data/S275_walk.cpp build/src/libchesso_engine.a -lpthread
//       -o build/S275_walk
//
//   build/S275_walk SLICE.tsv K EPOCHS REPORT THREADS
//
// K is the one `tuner` fits and prints for the same corpus; the split is the
// tuner's, by game, 10 % held out at seed 1, and only the training rows are
// descended.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "eval_model.hpp"
#include "tuner_groups.hpp"
#include "tuner_model.hpp"
#include "tuner_split.hpp"

int main(int argc, char** argv)
{
  if (argc != 6) {
    fprintf(stderr, "usage: S275_walk SLICE.tsv K EPOCHS REPORT THREADS\n");
    return 1;
  }

  const double k = atof(argv[2]);
  const int epochs = atoi(argv[3]);
  const int report = atoi(argv[4]);
  const unsigned threads = static_cast<unsigned>(atoi(argv[5]));

  tuner_model::dataset_t data;
  if (!tuner_model::load(argv[1], &data)) { return 1; }

  std::vector<uint32_t> starts;
  tuner_split::game_starts(data.ply, &starts);
  std::vector<uint32_t> index;
  const size_t train = tuner_split::split(starts, data.size(), 0.1, 1, &index);

  std::vector<double> target(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    target[i] = data.result[i];
  }

  std::vector<double> start(eval_model::PARAM_COUNT, 0.0);
  eval_model::starting_params(start.data());

  std::vector<uint8_t> mask;
  tuner_groups::free_mask("all", &mask);

  std::vector<double> plain = start;
  std::vector<double> held = start;
  tuner_model::adam_t plain_adam;
  tuner_model::adam_t held_adam;
  std::vector<double> grad;

  printf("%zu rows, %zu train, K %.4f, %d epochs, %u threads\n", data.size(),
         train, k, epochs, threads);

  for (int epoch = 1; epoch <= epochs; ++epoch) {
    tuner_model::gradient(data, plain.data(), k, index, 0, train, threads,
                          target, &grad);
    tuner_model::adam_update(grad, mask, 1.0, &plain_adam, plain.data());

    tuner_model::gradient(data, held.data(), k, index, 0, train, threads,
                          target, &grad);
    tuner_model::descend(grad, mask, start.data(), 1.0, &held_adam,
                         held.data());

    if (epoch % report != 0) { continue; }

    const double e_plain = tuner_model::error_range(
        data, plain.data(), k, index, 0, train, threads, target);
    const double e_held = tuner_model::error_range(
        data, held.data(), k, index, 0, train, threads, target);

    printf("epoch %5d  train plain %.6f  held %.6f  |difference| %.3g\n",
           epoch, e_plain, e_held, std::fabs(e_plain - e_held));
  }

  for (const tuner_groups::gauge_t& gauge : tuner_groups::gauges()) {
    double plain_shift = 0.0;
    double held_shift = 0.0;
    for (const size_t column : gauge.level) {
      plain_shift += plain[column] - start[column];
      held_shift += held[column] - start[column];
    }
    plain_shift /= static_cast<double>(gauge.level.size());
    held_shift /= static_cast<double>(gauge.level.size());

    if (gauge.absorb < eval_model::PARAM_COUNT) {
      printf(
          "%-24s level shift plain %+9.4f held %+.1e   material shift plain "
          "%+8.3f held %+8.3f\n",
          gauge.name.c_str(), plain_shift, held_shift,
          plain[gauge.absorb] - start[gauge.absorb],
          held[gauge.absorb] - start[gauge.absorb]);
    } else {
      printf("%-24s level shift plain %+9.4f held %+.1e\n", gauge.name.c_str(),
             plain_shift, held_shift);
    }
  }

  double unreachable = 0.0;
  for (const size_t base : {eval_model::MG_BASE, eval_model::EG_BASE}) {
    for (size_t square = 0; square < 64; ++square) {
      if (tuner_groups::pawn_can_stand(square)) { continue; }
      unreachable = std::max(
          {unreachable, std::fabs(plain[base + square] - start[base + square]),
           std::fabs(held[base + square] - start[base + square])});
    }
  }

  double apart = 0.0;
  double travelled = 0.0;
  for (size_t row = 0; row < data.size(); ++row) {
    const double p = tuner_model::evaluate_position(data, row, plain.data());
    const double h = tuner_model::evaluate_position(data, row, held.data());
    const double s = tuner_model::evaluate_position(data, row, start.data());
    apart = std::max(apart, std::fabs(p - h));
    travelled = std::max(travelled, std::fabs(p - s));
  }

  printf("pawn ranks 1 and 8, 32 columns: largest |change| either run %.3g\n",
         unreachable);
  printf(
      "scores over all %zu rows: plain against held, largest |difference| "
      "%.3g cp; plain against the start, largest |change| %.1f cp\n",
      data.size(), apart, travelled);
  printf("S275-WALK-DONE\n");

  return 0;
}
