// Finds the positions that make evaluate()'s taper truncate hardest. S076.
//
//   build/tools/truncation_scan --data .tuning/selfplay_v2_dedup.tsv --min 2.8
//
// `tests/test_eval_trace.cpp` pins four positions whose smooth score -- the one
// the tuner descends, every taper exact in floating point -- and evaluate()'s
// integer score part by 69/24 = 2.875, the arithmetic maximum of three
// truncating divisions, so that the bound in "the smooth score is within the
// truncation bound of the exact one" is asserted against positions that
// actually reach it. Its comment states the rule this tool serves: **a
// residual belongs to the weights, not to the position**, so the list is
// re-measured whenever the constants are refitted rather than carried forward.
// DEC-057.
//
// It had been re-measured twice by hand, at S038 and at S065, each time from a
// script in a session scratchpad that no longer exists -- the shape
// `2026-08-16_plan_review-F04` found. S076 refits again, so the third
// re-measurement is a tracked tool.
//
// Both scores are read from the engine's trace (tools/tuner_trace.hpp) since
// S276: score() is the smooth one and exact_score() the engine's own integer
// arithmetic over the same record, which is evaluate() -- checked here on
// every row, and a row on which it is not is refused, since its residual would
// not be evaluate()'s. Until S276 the smooth side was tools/eval_model.hpp, the
// hand-written mirror the tuner fitted before S262, computing the same function
// with its own sum and its own feature walks, and the exact side was
// evaluate() itself; the four pins sat in tests/test_eval_model.cpp.
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

namespace
{

struct options_t
{
  std::string data;
  double min = 2.8;
  uint64_t limit = 0;   // 0 scans everything
  uint64_t report = 0;  // 0 prints every hit
};


// The FEN of a `fen result score phase` row, or the whole line when it carries
// no tab -- so a plain list of FENs scans too.
std::string row_fen(const std::string& line)
{
  const size_t tab = line.find('\t');
  return (tab == std::string::npos) ? line : line.substr(0, tab);
}


std::string side_to_move_of(const std::string& fen)
{
  std::istringstream fields(fen);
  std::string placement, side;

  fields >> placement >> side;
  return side;
}


void usage()
{
  fprintf(stderr,
          "truncation_scan --data FILE [options]\n"
          "  --data FILE   a `fen result score phase` corpus, or one FEN per\n"
          "                line\n"
          "  --min VALUE   print positions disagreeing by more than this\n"
          "                (default 2.8; the maximum is 69/24 = 2.875)\n"
          "  --limit N     stop after N rows (default 0, everything)\n"
          "  --report N    print at most N hits (default 0, all of them)\n");
}

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "--help") {
      usage();
      return 0;
    }

    if (i + 1 >= argc) {
      usage();
      return 1;
    }

    const std::string value = argv[++i];

    if (arg == "--data") {
      opts.data = value;
    } else if (arg == "--min") {
      opts.min = atof(value.c_str());
    } else if (arg == "--limit") {
      opts.limit = strtoull(value.c_str(), nullptr, 10);
    } else if (arg == "--report") {
      opts.report = strtoull(value.c_str(), nullptr, 10);
    } else {
      usage();
      return 1;
    }
  }

  if (opts.data.empty()) {
    usage();
    return 1;
  }

  std::ifstream in(opts.data);

  if (!in) {
    fprintf(stderr, "cannot read %s\n", opts.data.c_str());
    return 1;
  }

  game_t game;
  initialize_game_const_data(&game);

  // The smooth score at the weights the engine ships, and the same weights as
  // the integers the engine was built with, for the exact one.
  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();

  std::string line;
  uint64_t scanned = 0;
  uint64_t past_two = 0;
  uint64_t past_min = 0;
  uint64_t printed = 0;
  double worst = 0.0;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }
    if (opts.limit > 0 && scanned >= opts.limit) { break; }

    const std::string fen = row_fen(line);

    if (!load_FEN(fen, &game)) {
      fprintf(stderr, "unloadable FEN: %s\n", fen.c_str());
      return 1;
    }

    tuner_trace::row_trace_t trace;
    std::string why;

    if (!tuner_trace::trace_board(&game.board, &trace, &why)) {
      fprintf(stderr, "untraceable FEN (%s): %s\n", why.c_str(), fen.c_str());
      return 1;
    }

    // One row of its own: score() and exact_score() read a dataset.
    tuner_trace::dataset_t row;
    tuner_trace::append_row(trace, 0.5f, 0, 0, &row);

    const int64_t exact = tuner_trace::exact_score(row, 0, weights.data());

    if (exact != trace.engine_white) {
      fprintf(stderr,
              "the trace disagrees with evaluate(): %" PRId64
              " against %d: %s\n",
              exact, trace.engine_white, fen.c_str());
      return 1;
    }

    const double smooth = tuner_trace::score(row, 0, params.data());
    const double difference = std::abs(smooth - static_cast<double>(exact));

    scanned++;
    worst = std::max(worst, difference);

    if (difference > 2.0) { past_two++; }

    if (difference > opts.min) {
      past_min++;

      if (opts.report == 0 || printed < opts.report) {
        printed++;
        printf("%.6f\tphase %2d\t%s\t%s\n", difference, trace.phase,
               side_to_move_of(fen).c_str(), fen.c_str());
      }
    }
  }

  fprintf(stderr,
          "%" PRIu64 " rows scanned, %" PRIu64 " past 2.0, %" PRIu64
          " past %.4f, worst %.6f\n",
          scanned, past_two, past_min, opts.min, worst);

  return 0;
}
