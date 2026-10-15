// How far does the expensive stage move the score? S034, S027.
//
//   build/tools/eval_spread --data .tuning/selfplay_v2.tsv --limit 200000
//
// The distribution of what evaluate() adds to evaluate_cheap() -- mobility,
// king safety and the two together -- read through evaluate_expensive_terms().
// Until S039 that was the number the lazy shortcut assumed away and the clamp
// truncated, both at LAZY_EVAL_MARGIN, and this tool existed to re-decide the
// margin: it printed how many positions each candidate would clamp. The
// shortcut and the clamp retired together (DEC-258), the candidate table with
// them, and what is left is the spread itself, which the steps that refit or
// rebuild the two terms read before and after (S121, S122).
//
// It reads what tools/datagen writes, `fen result score phase`, and uses only
// the first field: the label and the stored score say nothing about how far the
// expensive terms move a position. Mobility and king safety are reported apart
// as well as together, because a large correction from one term, from the
// other or from both at once are different findings.
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
// centipawn that no position reached is not evidence about the stage.
int percentile(const std::vector<int>& sorted, double fraction)
{
  if (sorted.empty()) { return 0; }

  const double rank = std::ceil(fraction * static_cast<double>(sorted.size()));
  size_t index = (rank < 1.0) ? 0 : static_cast<size_t>(rank) - 1;

  if (index >= sorted.size()) { index = sorted.size() - 1; }

  return sorted[index];
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

  // The correction is what evaluate() applies on top of evaluate_cheap(), and
  // it is observable from outside. Checking the reported pair against it is
  // what says evaluate_expensive_terms() reports the same arithmetic the search
  // runs and not a second implementation that agrees with a model instead.
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

    if (applied != mobility + safety) { disagreements++; }

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

  printf("expensive-stage correction, absolute centipawns\n");
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

  printf("\nworst position per term\n");

  for (const series_t& column : series) {
    printf("  %-12s %4d  %s\n", column.name, column.worst,
           column.worst_fen.c_str());
  }

  // Loud rather than a footnote: every number above is wrong if this is not
  // zero, because then the tool is not reading the function the engine runs.
  if (disagreements != 0) {
    printf(
        "\nWARNING: %zu positions where the reported pair did not sum to\n"
        "         what evaluate() applied. The numbers above are not the "
        "engine's.\n",
        disagreements);
    return 1;
  }

  printf(
      "\nreported pair summed to evaluate() - evaluate_cheap() on every "
      "position\n");

  return 0;
}
