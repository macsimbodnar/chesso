// How large is the correction the lazy shortcut assumes away? S034, S027.
//
//   build-prof/tools/eval_spread --data .tuning/selfplay_v1.tsv --limit 200000
//
// evaluate_lazy() skips the expensive stage when the cheap score is already
// LAZY_EVAL_MARGIN clear of the window, and evaluate_expensive() clamps itself
// to that margin so the assumption is true by construction rather than by
// sampling. Both numbers rest on the distribution of the correction, and the
// clamp means nothing outside evaluation.cpp can see it -- which is what
// evaluate_expensive_terms() is for and what this reads.
//
// It reads what tools/datagen writes, `fen result score phase`, and uses only
// the first field: the label and the stored score say nothing about how far the
// expensive terms move a position. Mobility and king safety are reported apart
// as well as together, because a margin that binds on their sum and a margin
// that binds on one term are different problems with different fixes.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"

namespace
{

// Candidates for the margin, the current 150 first. Reported as an exceedance
// count rather than as a verdict: what a truncation costs in Elo is an SPRT
// question, and this program only says how often the truncation happens.
const std::vector<int> MARGINS = {150, 200, 250, 300, 400};


struct options_t
{
  std::string data;
  size_t limit = 0;  // 0 reads the whole file
};


// One column of the report. Values are absolute centipawns, kept unsorted until
// the end because the worst position has to be found before the order is lost.
struct series_t
{
  const char* name;
  std::vector<int> values;
  int worst = -1;
  std::string worst_fen;

  void add(int value, const std::string& fen)
  {
    values.push_back(value);

    if (value > worst) {
      worst = value;
      worst_fen = fen;
    }
  }
};


// Nearest rank over the sorted values, so every figure printed is a correction
// that some position in the corpus actually produced. No interpolation: a
// centipawn that no position reached is not evidence about the margin.
int percentile(const std::vector<int>& sorted, double fraction)
{
  if (sorted.empty()) { return 0; }

  const double rank = std::ceil(fraction * static_cast<double>(sorted.size()));
  size_t index = (rank < 1.0) ? 0 : static_cast<size_t>(rank) - 1;

  if (index >= sorted.size()) { index = sorted.size() - 1; }

  return sorted[index];
}


size_t count_above(const std::vector<int>& sorted, int margin)
{
  const auto first = std::upper_bound(sorted.begin(), sorted.end(), margin);
  return static_cast<size_t>(sorted.end() - first);
}


void usage()
{
  fprintf(stderr,
          "eval_spread --data FILE [options]\n"
          "  --limit N        stop after N positions (default: the whole "
          "file)\n");
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
    } else if (arg == "--limit") {
      opts.limit = strtoull(value.c_str(), nullptr, 10);
    } else {
      usage();
      return 1;
    }
  }

  if (opts.data.empty()) {
    usage();
    return 1;
  }

  std::ifstream file(opts.data);

  if (!file) {
    fprintf(stderr, "could not read %s\n", opts.data.c_str());
    return 1;
  }

  game_t game = {};
  initialize_game_const_data(&game);

  series_t series[3] = {{"mobility", {}, -1, ""},
                        {"king safety", {}, -1, ""},
                        {"combined", {}, -1, ""}};

  size_t rejected = 0;

  // The clamped correction is what evaluate() applies, and it is observable
  // from outside. Checking it against the clamp of the unclamped pair is what
  // says evaluate_expensive_terms() reports the same arithmetic the search runs
  // and not a second implementation that agrees with the model instead.
  size_t disagreements = 0;

  std::string line;

  while (std::getline(file, line)) {
    if (line.empty()) { continue; }

    if (opts.limit != 0 && series[2].values.size() >= opts.limit) { break; }

    const size_t tab = line.find('\t');
    const std::string fen =
        (tab == std::string::npos) ? line : line.substr(0, tab);

    if (!load_FEN(fen, &game)) {
      rejected++;
      continue;
    }

    int mobility = 0;
    int safety = 0;
    evaluate_expensive_terms(&game.board, &mobility, &safety);

    const int applied = evaluate(&game.board) - evaluate_cheap(&game.board);
    const int expected =
        std::clamp(mobility + safety, -LAZY_EVAL_MARGIN, LAZY_EVAL_MARGIN);

    if (applied != expected) { disagreements++; }

    series[0].add(std::abs(mobility), fen);
    series[1].add(std::abs(safety), fen);
    series[2].add(std::abs(mobility + safety), fen);
  }

  const size_t positions = series[2].values.size();

  if (positions == 0) {
    fprintf(stderr, "no positions read from %s\n", opts.data.c_str());
    return 1;
  }

  for (series_t& column : series) {
    std::sort(column.values.begin(), column.values.end());
  }

  printf("unclamped expensive-stage correction, absolute centipawns\n");
  printf("  data       %s\n", opts.data.c_str());
  printf("  positions  %zu\n", positions);

  if (rejected != 0) {
    printf("  rejected   %zu lines the engine would not parse\n", rejected);
  }

  printf("\n%-12s %12s %12s %12s\n", "", series[0].name, series[1].name,
         series[2].name);

  const double fractions[6] = {0.5, 0.9, 0.95, 0.99, 0.999, 1.0};
  const char* labels[6] = {"p50", "p90", "p95", "p99", "p99.9", "max"};

  for (int row = 0; row < 6; ++row) {
    printf("  %-10s %12d %12d %12d\n", labels[row],
           percentile(series[0].values, fractions[row]),
           percentile(series[1].values, fractions[row]),
           percentile(series[2].values, fractions[row]));
  }

  printf("\nabove a candidate margin\n");
  printf("%-12s %12s %12s %12s\n", "", series[0].name, series[1].name,
         series[2].name);

  for (const int margin : MARGINS) {
    printf("  %-10d", margin);

    for (const series_t& column : series) {
      const size_t above = count_above(column.values, margin);
      const double percent =
          (100.0 * static_cast<double>(above)) / static_cast<double>(positions);

      printf(" %6zu %5.3f%%", above, percent);
    }

    printf("\n");
  }

  printf("\nworst position per term\n");

  for (const series_t& column : series) {
    printf("  %-12s %4d  %s\n", column.name, column.worst,
           column.worst_fen.c_str());
  }

  // Loud rather than a footnote: every number above is wrong if this is not
  // zero, because then the tool is not reading the function the engine runs.
  if (disagreements != 0) {
    printf(
        "\nWARNING: %zu positions where clamping the reported pair did not "
        "reproduce\n"
        "         what evaluate() applied. The numbers above are not the "
        "engine's.\n",
        disagreements);
    return 1;
  }

  printf(
      "\nclamped pair reproduced evaluate() - evaluate_cheap() on every "
      "position\n");

  return 0;
}
