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
//
// **It is S124's factor check too** (DEC-350 (6)): exact_score() carries the
// factor's integer form, so on a build whose cells are off F a row on which
// the trace's factor and the engine's disagree -- a candidate cell recorded
// wrong, the material left out of E1, the wrong divisor, the strong side
// read from anything but E1 -- is refused here. On such a build the
// positional division is by 24 F = 3072 on every row in a cell, losing up to
// 3071/3072 where 24 lost 23/24, so the maximum on those rows is 2 x 23/24 +
// 3071/3072 = 2.916 while tempo is zero; at F, every build that ships at its
// identity, the factor is taper()'s integer and the maximum stays 2.875. A
// stale pick is invisible here -- the scan reads at the engine's own weights,
// where the stored and the current signs agree -- and the tuner's tests hold
// that (tests/test_tuner_gradient.cpp).
//
// **Each row is read off its own share of S122's curve since DEC-355
// (2026-11-06).** The curve's four truncations, T per king and per half, put
// that row's tuner_trace::curve_residual() C into smooth - exact, under 1 in
// size and zero at the curve's seed, so the number printed, counted past 2.0
// and against --min, and reported as the worst, is |smooth - exact - C|,
// tuner_trace::taperings_loss(): exactly what the taperings lost, on the 1/24
// lattice, wherever stage two's clamp does not bind, and bounded by the same
// where it does, against 2.875 at F and 2.916 off F -- the reading and the
// bounds tests/test_eval_trace.cpp holds rows to. A row counts past 2.0 or
// past --min only by more than tuner_trace::LATTICE_MARGIN, so a row exactly
// on a lattice value is not counted past it by an ulp. From S122's landing to
// 2026-11-06 the scan printed the difference itself, whose maximum sat under a
// flat 3.875 while the curve was live, and for a few hours of that day
// |smooth - exact| - |C|, only a lower bound on the taperings' loss; the
// summary still reports the largest raw difference beside the rest. And every
// row is held to its own bound, tuner_trace::truncation_bound(), the
// definition the tests read: a row past it is counted, the first named, and
// the scan exits 1.
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
          "  --min VALUE   print positions whose taperings' loss, the\n"
          "                difference less the row's signed curve residual,\n"
          "                passes this by more than 1e-6 (default 2.8; the\n"
          "                maximum is 69/24 = 2.875, or 2.916 on a row whose\n"
          "                scale factor is off F)\n"
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
  uint64_t past_bound = 0;
  double worst = 0.0;
  double worst_raw = 0.0;

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
    const double signed_raw = smooth - static_cast<double>(exact);
    const double raw = std::abs(signed_raw);
    const double difference =
        tuner_trace::taperings_loss(row, 0, weights.data(), signed_raw);

    // The same 1e-9 tests/test_eval_trace.cpp allows the floating-point sum.
    if (difference >
        tuner_trace::truncation_bound(row, 0, weights.data()) + 1e-9) {
      if (past_bound == 0) {
        fprintf(stderr, "past its own truncation bound, %.6f: %s\n", difference,
                fen.c_str());
      }
      past_bound++;
    }

    scanned++;
    worst = std::max(worst, difference);
    worst_raw = std::max(worst_raw, raw);

    if (difference > 2.0 + tuner_trace::LATTICE_MARGIN) { past_two++; }

    if (difference > opts.min + tuner_trace::LATTICE_MARGIN) {
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
          " past %.4f, worst %.6f, each row's taperings' loss; the "
          "largest difference %.6f, %" PRIu64 " past their own bound\n",
          scanned, past_two, past_min, opts.min, worst, worst_raw, past_bound);

  return (past_bound == 0) ? 0 : 1;
}
