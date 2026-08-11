#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"

// Evaluation benchmark.
//
// One question: what does a term added to evaluate() cost per call? The search
// cannot answer it. A term that changes the score changes the tree, so two
// builds visit different nodes and a wall time at fixed depth mixes the cost
// of the term with the shape of the search it caused. This calls evaluate()
// and nothing else, over a fixed position list, so the only thing that can
// move the number is the function itself.
//
// The discipline is bench_movegen's, for the same reason: a benchmark that
// cannot say how much it is able to resolve invites a conclusion the machine
// did not support.
//
//   - Fixed workload, compiled in. No assets, no working directory.
//   - The score of every position is printed and checksummed before any
//     timing. Two builds that are supposed to evaluate alike must produce the
//     same checksum, and one that changes the score says so out loud rather
//     than quietly measuring a different function.
//   - One warm-up sweep is discarded, then the best of the timed sweeps is
//     reported. The minimum is the least noise-contaminated estimate on a
//     machine that is also doing other things.
//   - The two halves of the run are compared and their disagreement printed.
//     A difference between two builds smaller than that has not been shown to
//     exist.
//
// The positions span the phase range on purpose. Mobility, king safety and
// anything else built on occupancy costs more with more pieces on the board,
// so an average over a full board and a bare endgame is the honest figure and
// the per-position lines are there to show the spread behind it.

namespace
{

struct bench_position_t
{
  const char* name;
  const char* fen;
};

// Phase in the engine's own game_phase() terms runs 24 down to 0 across these.
const std::vector<bench_position_t> POSITIONS = {
    {"start", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
    {"kiwipete",
     "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"},
    {"midgame",
     "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
     "10"},
    {"open midgame",
     "2r2rk1/1p1qbppp/p2pbn2/4p3/4P3/1NN1BP2/PPPQ2PP/2KR3R w - - 0 15"},
    {"rooks and minors",
     "8/2p2pk1/1p1r2p1/p2n3p/P2P3P/1P3NP1/2R2PK1/8 w - - 0 30"},
    {"queen endgame", "8/5pk1/6p1/7p/7P/1Q4P1/5PK1/1q6 w - - 0 40"},
    {"rook endgame", "8/5pk1/6p1/7p/7P/6P1/R4PK1/6r1 w - - 0 40"},
    {"minor endgame", "8/5pk1/6p1/7p/7P/5NP1/5PK1/6n1 w - - 0 40"},
    {"pawn endgame", "8/5pk1/6p1/7p/7P/6P1/5PK1/8 w - - 0 40"},
    {"bare kings", "4k3/8/8/8/8/8/8/4K3 w - - 0 1"},
};

// Enough repetitions that a sweep is a decent fraction of a second. At 20000
// a sweep was 4.5 ms and the two halves of the run disagreed by 34 %, which
// resolves nothing: the workload has to be long enough that a scheduler
// hiccup is a small part of it.
//
// It is a flag rather than a constant because the right value depends on how
// expensive the evaluation being measured is. The same 400000 that gives a
// 64 ms sweep with a mobility term gives a 6 ms sweep without one, and the
// cheap build is then the one that cannot be trusted. Compare two builds at
// the same -n, and raise it until the reported resolution is small against
// the difference being claimed.
int repetitions = 400000;


struct timing_t
{
  std::vector<double> samples;

  double best() const
  {
    return samples.empty() ? 0.0
                           : *std::min_element(samples.begin(), samples.end());
  }

  double worst() const
  {
    return samples.empty() ? 0.0
                           : *std::max_element(samples.begin(), samples.end());
  }

  double median() const
  {
    if (samples.empty()) { return 0.0; }

    std::vector<double> sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    return sorted[sorted.size() / 2];
  }

  // The minimum over the first and second half of the run, as two independent
  // estimates of the same quantity. How far apart they are is the resolution
  // of this measurement.
  double best_of_half(bool second) const
  {
    if (samples.size() < 2) { return 0.0; }

    const size_t middle = samples.size() / 2;
    const auto from = samples.begin() + (second ? middle : 0);
    const auto to = second ? samples.end() : samples.begin() + middle;

    return *std::min_element(from, to);
  }

  double resolution() const
  {
    const double a = best_of_half(false);
    const double b = best_of_half(true);

    if (a <= 0.0 || b <= 0.0) { return 0.0; }

    const double low = (a < b) ? a : b;
    return ((a < b ? b - a : a - b) / low) * 100.0;
  }
};


// Returned so the compiler cannot delete the loop it came from.
int64_t sweep(const std::vector<board_t>& boards)
{
  int64_t sink = 0;

  for (int i = 0; i < repetitions; ++i) {
    for (const board_t& board : boards) {
      sink += evaluate(&board);
    }
  }

  return sink;
}


void usage()
{
  printf(
      "usage: bench_eval [-r N] [-n N]\n"
      "\n"
      "  -r N     timed sweeps, best is reported (default 7)\n"
      "  -n N     repetitions of the position list per sweep (default "
      "400000).\n"
      "           Raise it until the reported resolution is small against the\n"
      "           difference being claimed\n");
}

}  // namespace


int main(int argc, char** argv)
{
  int rounds = 7;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
      rounds = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      repetitions = atoi(argv[++i]);
    } else {
      usage();
      return (strcmp(argv[i], "-h") == 0) ? 0 : 1;
    }
  }

  if (rounds < 2) {
    printf("-r must be at least 2, or the two halves cannot be compared\n");
    return 1;
  }

  game_t game = {};
  initialize_game_const_data(&game);

  std::vector<board_t> boards;
  boards.reserve(POSITIONS.size());

  // Scores before timings, for the same reason bench_movegen verifies perft
  // before it reports anything: a build that evaluates differently is not
  // measuring the same function, and it must be impossible to miss.
  printf("positions\n");

  int64_t checksum = 0;

  for (const bench_position_t& position : POSITIONS) {
    if (!load_FEN(position.fen, &game)) {
      printf("  FAILED to load %s\n", position.name);
      return 1;
    }

    const int score = evaluate(&game.board);
    const int phase = game_phase(&game.board);

    checksum = checksum * 31 + score;
    boards.push_back(game.board);

    printf("  %-18s phase %2d  score %6d\n", position.name, phase, score);
  }

  printf("  checksum %lld over %zu positions\n\n",
         static_cast<long long>(checksum), boards.size());

  const size_t calls_per_sweep =
      static_cast<size_t>(repetitions) * boards.size();

  sweep(boards);  // warm-up, discarded

  timing_t timing;

  for (int round = 0; round < rounds; ++round) {
    const auto start = std::chrono::steady_clock::now();
    const int64_t sink = sweep(boards);
    const auto end = std::chrono::steady_clock::now();

    if (sink == 0x5EEDL) { printf(""); }  // the sink is never optimised out

    timing.samples.push_back(
        std::chrono::duration<double, std::milli>(end - start).count());
  }

  const double best = timing.best();
  const double spread =
      (best > 0.0) ? ((timing.worst() - best) / best) * 100.0 : 0.0;
  const double per_call = (best * 1e6) / static_cast<double>(calls_per_sweep);
  const double per_second =
      (best > 0.0) ? (static_cast<double>(calls_per_sweep) / (best / 1000.0))
                   : 0.0;

  printf("evaluate() over %zu calls per sweep, %d sweeps\n", calls_per_sweep,
         rounds);
  printf("  best %.1f ms, median %.1f ms, worst %.1f ms, spread %.1f%%\n", best,
         timing.median(), timing.worst(), spread);
  printf("  %.2f ns per call, %.1f M calls per second\n", per_call,
         per_second / 1e6);

  const double resolution = timing.resolution();

  printf(
      "  resolution %.1f%%  (the two halves of this run disagree by that "
      "much)\n",
      resolution);

  if (resolution > 1.0) {
    printf(
        "  NOTE: this run cannot resolve a change smaller than %.1f%%. Close\n"
        "        whatever else is using the machine, or raise -r, before\n"
        "        trusting a comparison at that scale.\n",
        resolution);
  }

  return 0;
}
