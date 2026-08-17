// Reduces a tuning corpus to one row per distinct position. S076.
//
//   build/tools/corpus_dedupe --in .tuning/selfplay_v2.tsv
//                             --out .tuning/selfplay_v2_dedup.tsv
//
// The pass, the key it uses and why the first row survives are all in
// tools/corpus_dedupe.hpp beside the code. This file is the command line and
// the report.
//
// Nothing here replays a game or asks the search anything: it reads the FEN
// each row already carries and hashes it with the engine's own zobrist
// randoms, which is one load_FEN per row.
#include "corpus_dedupe.hpp"
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

namespace
{

void usage()
{
  fprintf(stderr,
          "corpus_dedupe --in FILE --out FILE [--verify]\n"
          "  --in FILE    a `fen result score phase` corpus\n"
          "  --out FILE   one row per distinct position, in input order\n"
          "  --verify     also hold the four hashed FEN fields per key and\n"
          "               count key-equal rows that disagree on them, which\n"
          "               is what a 64-bit collision would look like. Roughly\n"
          "               doubles the memory the pass needs\n");
}

}  // namespace


int main(int argc, char** argv)
{
  std::string in_path;
  std::string out_path;
  bool verify = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "--help") {
      usage();
      return 0;
    }

    if (arg == "--verify") {
      verify = true;
      continue;
    }

    if (i + 1 >= argc) {
      usage();
      return 1;
    }

    const std::string value = argv[++i];

    if (arg == "--in") {
      in_path = value;
    } else if (arg == "--out") {
      out_path = value;
    } else {
      usage();
      return 1;
    }
  }

  if (in_path.empty() || out_path.empty()) {
    usage();
    return 1;
  }

  // Refused rather than truncated: the input is read a line at a time while the
  // output is written, so the same path on both sides would eat the corpus it
  // is reading.
  if (in_path == out_path) {
    fprintf(stderr, "--in and --out are the same file: %s\n", in_path.c_str());
    return 1;
  }

  std::ifstream in(in_path);

  if (!in) {
    fprintf(stderr, "cannot read %s\n", in_path.c_str());
    return 1;
  }

  std::ofstream out(out_path);

  if (!out) {
    fprintf(stderr, "cannot write %s\n", out_path.c_str());
    return 1;
  }

  corpus_dedupe::stats_t stats;
  std::string error;

  if (!corpus_dedupe::run(in, out, verify, &stats, &error)) {
    out.close();
    std::remove(out_path.c_str());
    fprintf(stderr, "%s\n", error.c_str());
    fprintf(stderr, "refused after %" PRIu64 " rows; %s removed\n",
            stats.rows_read, out_path.c_str());
    return 1;
  }

  out.close();

  if (!out) {
    fprintf(stderr, "write failed on %s\n", out_path.c_str());
    return 1;
  }

  const double dropped = (stats.rows_read > 0)
                             ? 100.0 * static_cast<double>(stats.rows_dropped) /
                                   static_cast<double>(stats.rows_read)
                             : 0.0;

  fprintf(stderr,
          "%" PRIu64 " rows read, %" PRIu64
          " distinct positions written, "
          "%" PRIu64 " dropped (%.4f%%)\n",
          stats.rows_read, stats.rows_written, stats.rows_dropped, dropped);

  // One drop count cannot tell a corpus where everything repeats twice from one
  // where a handful of positions repeat thousands of times, and those are two
  // different reasons to refit.
  fprintf(stderr,
          "repeats: %" PRIu64 " keys once, %" PRIu64 " twice, %" PRIu64
          " three times, %" PRIu64 " 4-7, %" PRIu64 " 8-15, %" PRIu64
          " 16 or more; most repeated %" PRIu64 "\n",
          stats.buckets[0], stats.buckets[1], stats.buckets[2],
          stats.buckets[3], stats.buckets[4], stats.buckets[5],
          stats.most_repeated);

  if (stats.blank_skipped > 0) {
    fprintf(stderr, "%" PRIu64 " blank line(s) skipped\n", stats.blank_skipped);
  }

  if (verify) {
    fprintf(stderr,
            "verify: %" PRIu64
            " key-equal row(s) disagreed on the four hashed FEN fields\n",
            stats.collisions);
  }

  return 0;
}
