// S136: the truncation guard's forecast, read before tempo is fitted. For
// every row of a corpus (or a file of one FEN per line) whose three-division
// residual could pass 69/24 once tempo's division rounds too, prints that
// residual signed, in 24ths, with the row's phase, side to move and FEN:
//
//   residual<TAB>phase<TAB>side<TAB>fen
//
// The residual is tools/tuner_trace.hpp's smooth score minus its exact one,
// White relative, at the weights the engine was built with -- what
// build/tools/truncation_scan prints unsigned. While tempo ships at zero its
// division truncates 0 / 24 exactly, so this is the three other divisions'
// sum (the clamp included). An `--only tempo` fit moves no other weight, and
// tempo is tapered on its own after the side-to-move sign, so the residual
// at the fitted pair (tm, te) is this one plus tempo's own, which needs no
// engine: side * (tm * phase + te * (24 - phase)) / 24 minus its truncation.
// adocs/data/S136_screen.py `forecast` and `guard` add it. A row under
// --min cannot pass 69/24 at any tempo, since tempo's division adds at most
// 23/24; 47 is the default for that reason.
//
// Build against a tree's trace library (its build/ configured as usual), one
// command on one line:
//
//   g++ -std=c++20 -O2 -Wall -Wextra -Werror -DCHESSO_TRACE -Isrc -Itools
//       adocs/data/S136_guard_forecast.cpp build/src/libchesso_engine_trace.a
//       -lpthread -o <out>/guard_forecast
//   <out>/guard_forecast --data CORPUS [--min 47] > rows.tsv
//
// adocs/data/S136_screen.sh `forecast` runs both.
//
// A row whose trace disagrees with evaluate() is refused, as truncation_scan
// refuses it: its residual would not be evaluate()'s.
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

int main(int argc, char** argv)
{
  std::string data;
  long min = 47;

  for (int i = 1; i + 1 < argc; i += 2) {
    const std::string arg = argv[i];
    if (arg == "--data") {
      data = argv[i + 1];
    } else if (arg == "--min") {
      min = strtol(argv[i + 1], nullptr, 10);
    } else {
      fprintf(stderr, "unknown argument %s\n", argv[i]);
      return 2;
    }
  }
  if (data.empty() || (argc % 2) == 0) {
    fprintf(stderr, "usage: guard_forecast --data FILE [--min 24THS]\n");
    return 2;
  }

  std::ifstream in(data);
  if (!in) {
    fprintf(stderr, "cannot read %s\n", data.c_str());
    return 1;
  }

  game_t game;
  initialize_game_const_data(&game);

  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();

  std::string line;
  uint64_t rows = 0;
  uint64_t kept = 0;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }

    const size_t tab = line.find('\t');
    const std::string fen =
        (tab == std::string::npos) ? line : line.substr(0, tab);

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

    // A multiple of 1/24 in exact arithmetic; the double carries ulp noise,
    // which the rounding removes.
    const double smooth = tuner_trace::score(row, 0, params.data());
    const long residual =
        std::lround(24.0 * (smooth - static_cast<double>(exact)));

    rows++;

    if (std::labs(residual) >= min) {
      kept++;
      printf("%ld\t%d\t%c\t%s\n", residual, trace.phase,
             (game.board.active_color == WHITE) ? 'w' : 'b', fen.c_str());
    }
  }

  fprintf(stderr, "%" PRIu64 " rows, %" PRIu64 " with |residual| >= %ld/24\n",
          rows, kept, min);

  return 0;
}
