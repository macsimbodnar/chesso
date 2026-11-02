// The clamp's census at a fit's own weights. S122, DEC-344 (4).
//
//   build/tools/margin_census --data CORPUS --vector FIT.vector [--k K]
//
// DEC-344 (4) re-derives LAZY_EVAL_MARGIN from the fit of S122's family: a
// margin passes when at most 0.5 % of rows bind on it, and the bound rows of
// the graded set hold at most 10 % of that set's gradient mass. Both are read
// here at the weights a fit emitted (tools/tuner --vector), so no build is
// owed per fit, and at each candidate margin, so the fixed point needs no
// re-run of this tool between rounds.
//
// What each figure is, exactly:
//
//   the score   exact_score()'s arithmetic over the row's trace at the
//               vector's integers -- the engine's integer tapers and the curve
//               -- with stage two clamped at the margin read: what the engine
//               would return with the fit pasted and LAZY_EVAL_MARGIN at that
//               margin. load() first holds the trace to evaluate() on every
//               row at the build's own weights and margin, and a row on which
//               it fails is refused, as the tuner refuses it.
//   bound       |mobility + king safety| past the margin, the same integers.
//   graded      either king with three or more attackers, a queen among them,
//               and a safe check under the census's reading 2: on an empty
//               square or a man of the king's (king_danger_t::checks_any),
//               the set DEC-344 (4)'s bar and the research pass's table were
//               read over. From the engine's collecting instantiation, one
//               pass over the FENs.
//   mass        sigma(1 - sigma) of the row at K, the factor the loss
//               gradient carries: at the clamped score, the one the fit and
//               the engine use, and beside it at the unclamped score, the
//               research pass's first reading. The share is the graded bound
//               rows' mass over the graded rows' mass.
//   loss        the mean squared error against the result column at K, at
//               the same integer score: a fitted vector read on rows it did
//               not see (DEC-344 (1)'s side readings).
//   x > 0       the share of kings, two a row, whose curve sum is positive in
//               each half: DEC-344 (3)'s reading at a group fit's start and
//               end.
//
//   games       with --games FILE, one line a game, `index game rows sse` at
//               --loss-margin's clamp, so two vectors' losses on one set can be
//               paired by game and their difference's error read by game
//               (adocs/data/S122_screen.py side-read). A game is a run of rows
//               with one game id and a rising ply, as tools/tuner_split.hpp
//               reconstructs games from the ply: two pools joined in one file
//               number their games from 0 alike, so an id alone is not a game
//               (S122's fast check, finding 1), and the index is the game's
//               place in the file, which pairs two files of one set.
//
// It reads, it decides nothing: the bars are printed beside each margin, and
// the smallest multiple of 25 above the build's margin that passes both, up to
// LazyEvalMargin's ceiling of 2000, is printed as `pick`. The procedure that
// acts on it is adocs/data/S122_screen.sh margin.
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "bitboard.hpp"
#include "eval_spread_margins.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_target.hpp"
#include "tuner_trace.hpp"

namespace
{

constexpr double ROW_BAR = 0.005;
constexpr double MASS_BAR = 0.10;

struct options_t
{
  std::string data;
  std::string vector;
  double k = 0.0;
  unsigned threads = 0;
  std::vector<int> also;  // margins printed beside eval_spread's candidates
  std::string games;
  int loss_margin = -1;
};


// tools/tuner --vector's file: `#` lines, one of them `# K value`, then one
// integer a line, PARAM_COUNT of them.
bool read_vector(const std::string& path,
                 std::vector<int64_t>* weights,
                 double* k,
                 std::string* error)
{
  std::ifstream in(path);

  if (!in) {
    *error = "cannot read " + path;
    return false;
  }

  weights->clear();
  std::string line;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }

    if (line[0] == '#') {
      std::istringstream fields(line.substr(1));
      std::string key;
      fields >> key;
      if (key == "K") { fields >> *k; }
      if (key == "params") {
        size_t count = 0;
        fields >> count;
        if (count != tuner_layout::PARAM_COUNT) {
          *error = path + " is " + std::to_string(count) +
                   " parameters wide, this build " +
                   std::to_string(tuner_layout::PARAM_COUNT);
          return false;
        }
      }
      continue;
    }

    char* end = nullptr;
    const long long value = strtoll(line.c_str(), &end, 10);
    if (end == line.c_str() || *end != '\0') {
      *error = "not an integer in " + path + ": " + line;
      return false;
    }
    weights->push_back(value);
  }

  if (weights->size() != tuner_layout::PARAM_COUNT) {
    *error = path + " holds " + std::to_string(weights->size()) +
             " values, not " + std::to_string(tuner_layout::PARAM_COUNT);
    return false;
  }

  return true;
}


// Each row's graded flag and game, from the FENs in load()'s order: blank
// lines skipped, as load() skips them. The game is datagen's fifth column, -1
// on a row without one.
bool graded_rows(const std::string& path,
                 size_t rows,
                 unsigned threads,
                 std::vector<uint8_t>* graded,
                 std::vector<int64_t>* game_of,
                 std::string* error)
{
  std::ifstream in(path);
  if (!in) {
    *error = "cannot read " + path;
    return false;
  }

  std::vector<std::string> fens;
  fens.reserve(rows);
  game_of->clear();
  game_of->reserve(rows);
  std::string line;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }
    const size_t tab = line.find('\t');
    fens.push_back((tab == std::string::npos) ? line : line.substr(0, tab));

    std::istringstream fields(line);
    std::string field;
    int64_t game = -1;
    for (int column = 0; std::getline(fields, field, '\t'); ++column) {
      if (column == 4) { game = strtoll(field.c_str(), nullptr, 10); }
    }
    game_of->push_back(game);
  }

  if (fens.size() != rows) {
    *error = "the corpus read twice gave two row counts";
    return false;
  }

  graded->assign(rows, 0);
  std::vector<std::thread> workers;
  std::vector<std::string> failures(threads);

  for (unsigned t = 0; t < threads; ++t) {
    workers.emplace_back([&, t]() {
      game_t game;
      initialize_game_const_data(&game);

      for (size_t i = t; i < rows; i += threads) {
        if (!load_FEN(fens[i], &game)) {
          failures[t] = "unloadable FEN: " + fens[i];
          return;
        }

        king_danger_t danger;
        king_danger_counts(&game.board, &danger);

        for (int king = 0; king < 2; ++king) {
          const int* curve = danger.curve[king];
          const int attackers =
              curve[KSC_KNIGHT_ATTACKERS] + curve[KSC_BISHOP_ATTACKERS] +
              curve[KSC_ROOK_ATTACKERS] + curve[KSC_QUEEN_ATTACKERS];
          int checks = 0;
          for (int type = 0; type < 4; ++type) {
            checks += danger.checks_any[king][type];
          }

          if (attackers >= 3 && curve[KSC_QUEEN_ATTACKERS] > 0 && checks > 0) {
            (*graded)[i] = 1;
          }
        }
      }
    });
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  for (const std::string& failure : failures) {
    if (!failure.empty()) {
      *error = failure;
      return false;
    }
  }

  return true;
}


struct tally_t
{
  uint64_t bound = 0;
  uint64_t graded_bound = 0;
  double graded_mass_bound = 0.0;
  double graded_mass = 0.0;
  double graded_mass_unclamped_bound = 0.0;
  double graded_mass_unclamped = 0.0;
  double squared_error = 0.0;
};


void usage()
{
  fprintf(stderr,
          "margin_census --data FILE [options]\n"
          "  --data FILE     a `fen result score phase ...` corpus\n"
          "  --vector FILE   a fit's vector, tools/tuner --vector (default:\n"
          "                  the weights this build ships)\n"
          "  --k VALUE       the sigmoid's scale (default: the vector's `K`\n"
          "                  line; required without one)\n"
          "  --threads N     worker threads (default: every hardware thread)\n"
          "  --also LIST     margins printed beside the candidates, comma\n"
          "                  separated\n"
          "  --games FILE    write `index game rows sse` a game at\n"
          "                  --loss-margin, a game a run of one id and a\n"
          "                  rising ply\n"
          "  --loss-margin M the clamp --games reads (default: the build's)\n");
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
    } else if (arg == "--vector") {
      opts.vector = value;
    } else if (arg == "--k") {
      opts.k = atof(value.c_str());
    } else if (arg == "--threads") {
      opts.threads = static_cast<unsigned>(atoi(value.c_str()));
    } else if (arg == "--also") {
      std::istringstream list(value);
      std::string item;
      while (std::getline(list, item, ',')) {
        const int margin = atoi(item.c_str());
        if (margin <= 0 || margin > 2000) {
          fprintf(stderr, "--also takes margins in 1 to 2000\n");
          return 1;
        }
        opts.also.push_back(margin);
      }
    } else if (arg == "--games") {
      opts.games = value;
    } else if (arg == "--loss-margin") {
      opts.loss_margin = atoi(value.c_str());
      if (opts.loss_margin <= 0 || opts.loss_margin > 2000) {
        fprintf(stderr, "--loss-margin takes a margin in 1 to 2000\n");
        return 1;
      }
    } else {
      usage();
      return 1;
    }
  }

  if (opts.data.empty()) {
    usage();
    return 1;
  }

  if (opts.threads == 0) {
    opts.threads = std::max(1u, std::thread::hardware_concurrency());
  }

  std::vector<int64_t> weights = tuner_trace::engine_weights();
  double vector_k = 0.0;
  std::string error;

  if (!opts.vector.empty() &&
      !read_vector(opts.vector, &weights, &vector_k, &error)) {
    fprintf(stderr, "%s\n", error.c_str());
    return 1;
  }

  const double k = (opts.k > 0.0) ? opts.k : vector_k;

  if (!(k > 0.0)) {
    fprintf(stderr, "no K: give --k, or a vector with a `# K` line\n");
    return 1;
  }

  tuner_trace::dataset_t data;
  tuner_trace::load_report_t report;

  if (!tuner_trace::load(opts.data, &data, &report, &error, opts.threads)) {
    fprintf(stderr, "could not load %s: %s\n", opts.data.c_str(),
            error.c_str());
    return 1;
  }

  if (report.mismatches > 0) {
    fprintf(stderr,
            "the trace disagrees with evaluate() on %" PRIu64
            " rows, the first: %s\nnothing read\n",
            report.mismatches, report.first_mismatch.c_str());
    return 1;
  }

  std::vector<uint8_t> graded;
  std::vector<int64_t> game_of;
  if (!graded_rows(opts.data, data.size(), opts.threads, &graded, &game_of,
                   &error)) {
    fprintf(stderr, "%s\n", error.c_str());
    return 1;
  }

  // The margins printed: tools/eval_spread's candidates, the build's first.
  // The pick scans every multiple of 25 up to LazyEvalMargin's ceiling.
  std::vector<int> printed = eval_spread::candidate_margins();
  for (const int margin : opts.also) {
    if (std::find(printed.begin(), printed.end(), margin) == printed.end()) {
      printed.push_back(margin);
    }
  }
  const int loss_margin = (opts.loss_margin > 0)
                              ? opts.loss_margin
                              : static_cast<int>(LAZY_EVAL_MARGIN);
  std::vector<int> margins = printed;
  if (std::find(margins.begin(), margins.end(), loss_margin) == margins.end()) {
    margins.push_back(loss_margin);
  }
  for (int m = 200; m <= 2000; m += 25) {
    if (std::find(margins.begin(), margins.end(), m) == margins.end()) {
      margins.push_back(m);
    }
  }

  const size_t width = margins.size();
  std::vector<std::vector<tally_t>> partial(opts.threads,
                                            std::vector<tally_t>(width));
  std::vector<uint64_t> graded_count(opts.threads, 0);
  std::vector<uint64_t> positive[2] = {std::vector<uint64_t>(opts.threads, 0),
                                       std::vector<uint64_t>(opts.threads, 0)};
  // Each row's squared error at the loss margin, for --games.
  std::vector<double> row_error(data.size(), 0.0);
  const size_t loss_slot = static_cast<size_t>(
      std::find(margins.begin(), margins.end(), loss_margin) - margins.begin());
  std::vector<std::thread> workers;

  const tuner_trace::model_t& model = tuner_trace::engine_model();

  for (unsigned t = 0; t < opts.threads; ++t) {
    workers.emplace_back([&, t]() {
      std::vector<double> params(weights.begin(), weights.end());

      for (size_t row = t; row < data.size(); row += opts.threads) {
        const tuner_trace::exact_parts_t parts =
            tuner_trace::exact_parts(data, row, weights.data());
        const int64_t rest = parts.material + parts.positional + parts.tempo;
        const int64_t two = parts.mobility + parts.safety;
        const double result = data.result[row];
        const bool is_graded = graded[row] != 0;

        const double free_sigma =
            tuner_target::sigmoid(static_cast<double>(rest + two), k);
        const double free_mass = free_sigma * (1.0 - free_sigma);

        if (is_graded) { graded_count[t]++; }

        for (size_t m = 0; m < width; ++m) {
          const int64_t margin = margins[m];
          const int64_t clamped = std::clamp(two, -margin, margin);
          const double sigma =
              tuner_target::sigmoid(static_cast<double>(rest + clamped), k);
          const double mass = sigma * (1.0 - sigma);
          const bool bound = (two > margin || two < -margin);
          tally_t& out = partial[t][m];

          out.squared_error += (result - sigma) * (result - sigma);
          if (m == loss_slot) {
            row_error[row] = (result - sigma) * (result - sigma);
          }
          if (bound) { out.bound++; }

          if (is_graded) {
            out.graded_mass += mass;
            out.graded_mass_unclamped += free_mass;
            if (bound) {
              out.graded_bound++;
              out.graded_mass_bound += mass;
              out.graded_mass_unclamped_bound += free_mass;
            }
          }
        }

        for (size_t g = 0; g < model.groups.size(); ++g) {
          const tuner_trace::group_sums_t x = tuner_trace::group_sums(
              model.groups[g], g, data, row, params.data());
          for (int side = 0; side < 2; ++side) {
            if (x.mg[side] > 0.0) { positive[0][t]++; }
            if (x.eg[side] > 0.0) { positive[1][t]++; }
          }
        }
      }
    });
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  std::vector<tally_t> total(width);
  uint64_t graded_rows_total = 0;
  uint64_t positive_total[2] = {0, 0};

  for (unsigned t = 0; t < opts.threads; ++t) {
    graded_rows_total += graded_count[t];
    positive_total[0] += positive[0][t];
    positive_total[1] += positive[1][t];
    for (size_t m = 0; m < width; ++m) {
      tally_t& to = total[m];
      const tally_t& from = partial[t][m];
      to.bound += from.bound;
      to.graded_bound += from.graded_bound;
      to.graded_mass_bound += from.graded_mass_bound;
      to.graded_mass += from.graded_mass;
      to.graded_mass_unclamped_bound += from.graded_mass_unclamped_bound;
      to.graded_mass_unclamped += from.graded_mass_unclamped;
      to.squared_error += from.squared_error;
    }
  }

  const double rows = static_cast<double>(data.size());
  const auto share = [](double part, double whole) {
    return (whole > 0.0) ? part / whole : 0.0;
  };
  const auto passes = [&](const tally_t& m) {
    return share(static_cast<double>(m.bound), rows) <= ROW_BAR &&
           share(m.graded_mass_bound, m.graded_mass) <= MASS_BAR;
  };

  printf("margin census, S122 DEC-344 (4)\n");
  printf("  data       %s\n", opts.data.c_str());
  printf(
      "  rows       %zu; the trace is evaluate() on every one at the "
      "build's weights and margin %d\n",
      data.size(), static_cast<int>(LAZY_EVAL_MARGIN));
  printf("  weights    %s\n",
         opts.vector.empty() ? "the build's own" : opts.vector.c_str());
  printf("  K          %.4f\n", k);
  printf("  graded     %" PRIu64
         " rows (%.3f %%): a king with >= 3 attackers, a queen among "
         "them, and a safe check, reading 2\n",
         graded_rows_total,
         100.0 * share(static_cast<double>(graded_rows_total), rows));
  printf("  x > 0      kings %.3f %% mg, %.3f %% eg, of %zu\n",
         100.0 * share(static_cast<double>(positive_total[0]), 2.0 * rows),
         100.0 * share(static_cast<double>(positive_total[1]), 2.0 * rows),
         2 * data.size());
  printf(
      "  bars       rows bound <= %.1f %%, graded mass bound <= %.0f %% at "
      "the clamped score\n\n",
      100.0 * ROW_BAR, 100.0 * MASS_BAR);

  printf("%8s %10s %8s %10s %9s %13s %15s %10s %5s\n", "margin", "bound",
         "%rows", "graded", "%graded", "%mass clamped", "%mass unclamped",
         "loss", "bars");

  for (const int margin : printed) {
    const size_t m = static_cast<size_t>(
        std::find(margins.begin(), margins.end(), margin) - margins.begin());
    const tally_t& t = total[m];
    printf(
        "%8d %10" PRIu64 " %8.3f %10" PRIu64
        " %9.2f %13.2f %15.2f %10.6f %5s\n",
        margin, t.bound, 100.0 * share(static_cast<double>(t.bound), rows),
        t.graded_bound,
        100.0 * share(static_cast<double>(t.graded_bound),
                      static_cast<double>(graded_rows_total)),
        100.0 * share(t.graded_mass_bound, t.graded_mass),
        100.0 * share(t.graded_mass_unclamped_bound, t.graded_mass_unclamped),
        t.squared_error / rows, passes(t) ? "pass" : "fail");
  }

  // The first margin is the build's; the pick is the build's own when it
  // passes, and otherwise the smallest multiple of 25 above it that does.
  int pick = -1;
  if (passes(total[0])) {
    pick = margins[0];
  } else {
    for (size_t m = 0; m < width; ++m) {
      if (margins[m] % 25 != 0 || margins[m] <= margins[0] ||
          !passes(total[m])) {
        continue;
      }
      if (pick < 0 || margins[m] < pick) { pick = margins[m]; }
    }
  }

  if (pick < 0) {
    printf("\npick none: no multiple of 25 up to 2000 passes both bars\n");
  } else {
    printf("\npick %d\n", pick);
  }

  if (!opts.games.empty()) {
    FILE* out = fopen(opts.games.c_str(), "w");
    if (out == nullptr) {
      fprintf(stderr, "cannot write %s\n", opts.games.c_str());
      return 1;
    }

    fprintf(out,
            "# margin_census --games: index game rows sse, at margin %d, K "
            "%.4f\n",
            loss_margin, k);

    // A game's rows are contiguous in datagen's files and their ply rises; a
    // new id or a ply that does not rise starts the next game, and a row
    // without a game column is refused rather than given one.
    size_t row = 0;
    size_t index = 0;
    while (row < data.size()) {
      if (game_of[row] < 0) {
        fprintf(stderr, "row %zu has no game column; --games needs one\n", row);
        fclose(out);
        return 1;
      }
      const int64_t game = game_of[row];
      size_t count = 0;
      double sse = 0.0;
      do {
        sse += row_error[row];
        count++;
        row++;
      } while (row < data.size() && game_of[row] == game &&
               data.ply[row] > data.ply[row - 1]);
      fprintf(out, "%zu %" PRId64 " %zu %.12g\n", index, game, count, sse);
      index++;
    }

    if (fclose(out) != 0) {
      fprintf(stderr, "cannot write %s\n", opts.games.c_str());
      return 1;
    }
    printf("games written to %s at margin %d: %zu games, %zu rows\n",
           opts.games.c_str(), loss_margin, index, data.size());
  }

  return 0;
}
