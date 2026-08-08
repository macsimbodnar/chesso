#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"

// Move generation benchmark.
//
// The point of this program is to answer one question reliably: did a change
// to the move generator make it faster? Everything here exists to keep the
// number comparable between two runs on the same machine.
//
//   - The workload is fixed and compiled in. No JSON, no asset path, nothing
//     that depends on the working directory.
//   - Node counts are verified against the published perft values before any
//     timing is reported. A generator that loses moves is faster and wrong,
//     and must never read as an improvement.
//   - One warm-up sweep is thrown away, then every sweep is timed and the
//     best is reported. The minimum is the least noise-contaminated estimate
//     available on a machine that is also doing other things. The worst sweep
//     and the spread are printed next to it, so a run taken on a busy laptop
//     is recognisable as one.
//
// Two numbers come out, because a change can move them independently:
//
//   perft          generate_moves + make_move + unmake_move, which is what a
//                  search actually pays per node.
//   generate_moves the generator on its own, called repeatedly on a fixed
//                  position with nothing else in the loop.


struct bench_position_t
{
  const char* name;
  const char* fen;
  int depth;        // for the perft phase
  uint64_t nodes;   // published perft value at that depth
  int quick_depth;  // ditto, for --quick
  uint64_t quick_nodes;
};


// The six standard perft positions. Node counts come from the same published
// tables as tests/assets/perft_json.
// clang-format off
static const std::vector<bench_position_t> positions = {
  {"startpos",  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                       5,  4865609, 4, 197281},
  {"kiwipete",  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",           4,  4085603, 3,  97862},
  {"endgame",   "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                      6, 11030083, 5, 674624},
  {"promotion", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",               5, 15833292, 4, 422333},
  {"tactical",  "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",                      4,  2103487, 3,  62379},
  {"midgame",   "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",       4,  3894594, 3,  89890},
};
// clang-format on

// Calls per position in the generate_moves phase. Sized so that phase costs
// about as long as the perft phase.
static const uint64_t GENERATE_CALLS = 2000000;
static const uint64_t GENERATE_CALLS_QUICK = 50000;


static game_t game;


static double ms_since(const std::chrono::steady_clock::time_point& start)
{
  return std::chrono::duration<double, std::milli>(
             std::chrono::steady_clock::now() - start)
      .count();
}


static uint64_t perft(game_t* g, int depth)
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(&g->tables, &g->board, moves);

  if (depth == 1) {
    // Counting the legal ones here rather than recursing one more level saves
    // a whole ply of generation and changes nothing about the result.
    uint64_t nodes = 0;

    for (size_t i = 0; i < count; ++i) {
      if (!make_move(g, moves[i])) { continue; }
      nodes++;
      unmake_move(g);
    }

    return nodes;
  }

  uint64_t nodes = 0;

  for (size_t i = 0; i < count; ++i) {
    if (!make_move(g, moves[i])) { continue; }
    nodes += perft(g, depth - 1);
    unmake_move(g);
  }

  return nodes;
}


// Accumulates the move count so that nothing in the loop can be discarded.
// generate_moves lives in another translation unit, so it cannot be inlined
// away either, but the sum is what makes that guarantee independent of how
// the program is linked.
static uint64_t generate_only(game_t* g, uint64_t calls)
{
  move_t moves[MAX_MOVES];
  uint64_t produced = 0;

  for (uint64_t i = 0; i < calls; ++i) {
    produced += generate_moves(&g->tables, &g->board, moves);
  }

  return produced;
}


static uint64_t generate_captures_only(game_t* g, uint64_t calls)
{
  move_t moves[MAX_MOVES];
  uint64_t produced = 0;

  for (uint64_t i = 0; i < calls; ++i) {
    produced += generate_captures(&g->tables, &g->board, moves);
  }

  return produced;
}


// Keeps every sample rather than a running best, because the useful statistic
// here is not the average. Interference from other processes can only ever make
// a run slower, so the minimum is the estimate of the machine's actual speed
// and the mean is an estimate of how busy the machine was.
struct timing_t
{
  std::vector<double> samples;

  void add(double sample) { samples.push_back(sample); }

  double best() const
  {
    if (samples.empty()) { return 0.0; }
    return *std::min_element(samples.begin(), samples.end());
  }

  double worst() const
  {
    if (samples.empty()) { return 0.0; }
    return *std::max_element(samples.begin(), samples.end());
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
  // of this measurement: a difference between two builds that is smaller than
  // this has not been shown to exist.
  double best_of_half(bool second) const
  {
    if (samples.size() < 2) { return 0.0; }

    const size_t middle = samples.size() / 2;
    const auto from = samples.begin() + (second ? middle : 0);
    const auto to = second ? samples.end() : samples.begin() + middle;

    return *std::min_element(from, to);
  }

  // Percentage disagreement between the two halves.
  double resolution() const
  {
    const double a = best_of_half(false);
    const double b = best_of_half(true);

    if (a <= 0.0 || b <= 0.0) { return 0.0; }

    const double low = (a < b) ? a : b;
    return ((a < b ? b - a : a - b) / low) * 100.0;
  }
};


static void print_spread(const timing_t& sweep)
{
  const double best = sweep.best();
  const double spread =
      (best > 0.0) ? ((sweep.worst() - best) / best) * 100.0 : 0.0;

  printf("  best %.1f ms, median %.1f ms, worst %.1f ms, spread %.1f%%\n", best,
         sweep.median(), sweep.worst(), spread);

  const double resolution = sweep.resolution();

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
}


static void usage()
{
  printf(
      "usage: bench_movegen [-r N] [--quick]\n"
      "\n"
      "  -r N     timed sweeps, best is reported (default 5)\n"
      "  --quick  much smaller workload, for checking the tool itself\n");
}


int main(int argc, char** argv)
{
  int repetitions = 5;
  bool quick = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "--quick") {
      quick = true;
    } else if (arg == "-r" && i + 1 < argc) {
      repetitions = std::max(1, atoi(argv[++i]));
    } else if (arg == "-h" || arg == "--help") {
      usage();
      return 0;
    } else {
      printf("unknown argument: %s\n\n", arg.c_str());
      usage();
      return 1;
    }
  }

  bool assertions_on = false;
#ifndef NDEBUG
  assertions_on = true;
  // make_move and the attack tables assert on every call. Those checks cost
  // more than the work being measured, so the timings would say nothing about
  // the generator. Shrink the workload and label the output.
  quick = true;
#endif

  const uint64_t generate_calls = quick ? GENERATE_CALLS_QUICK : GENERATE_CALLS;

  initialize_game_const_data(&game);

  printf("chesso movegen benchmark\n");
  printf("build: %s  |  sweeps: %d  |  workload: %s\n\n",
         assertions_on ? "assertions ON" : "NDEBUG", repetitions,
         quick ? "quick" : "full");

  if (assertions_on) {
    printf(
        "WARNING: built with assertions enabled. These timings measure the\n"
        "         assertions, not the move generator. Configure with\n"
        "         -DCMAKE_BUILD_TYPE=Release before comparing anything.\n\n");
  }

  //-#########################  perft phase  ###############################-//

  std::vector<timing_t> per_position(positions.size());
  timing_t perft_sweep;
  uint64_t total_nodes = 0;

  // One untimed sweep first: it pages in the attack tables and lets the clock
  // settle, and its cost belongs to neither the old code nor the new.
  for (const bench_position_t& position : positions) {
    if (!load_FEN(position.fen, &game)) {
      printf("FAILED to load %s\n", position.fen);
      return 1;
    }

    const int depth = quick ? position.quick_depth : position.depth;
    const uint64_t expected = quick ? position.quick_nodes : position.nodes;
    const uint64_t nodes = perft(&game, depth);

    if (nodes != expected) {
      printf("WRONG NODE COUNT for %s at depth %d: got %llu, expected %llu\n",
             position.name, depth, (unsigned long long)nodes,
             (unsigned long long)expected);
      printf("The generator is broken. No timings are worth reading.\n");
      return 1;
    }

    total_nodes += nodes;
  }

  for (int r = 0; r < repetitions; ++r) {
    double sweep_ms = 0.0;

    for (size_t p = 0; p < positions.size(); ++p) {
      const bench_position_t& position = positions[p];
      const int depth = quick ? position.quick_depth : position.depth;
      const uint64_t expected = quick ? position.quick_nodes : position.nodes;

      load_FEN(position.fen, &game);

      const auto start = std::chrono::steady_clock::now();
      const uint64_t nodes = perft(&game, depth);
      const double elapsed = ms_since(start);

      // Checked every sweep: it costs one comparison and rules out a run
      // where the numbers drifted halfway through.
      if (nodes != expected) {
        printf("WRONG NODE COUNT for %s on sweep %d\n", position.name, r);
        return 1;
      }

      per_position[p].add(elapsed);
      sweep_ms += elapsed;
    }

    perft_sweep.add(sweep_ms);
  }

  printf("perft  (generate_moves + make_move + unmake_move)\n");
  printf("  %-12s %6s %12s %11s %10s\n", "position", "depth", "nodes",
         "best ms", "Mnps");

  double perft_total_ms = 0.0;

  for (size_t p = 0; p < positions.size(); ++p) {
    const bench_position_t& position = positions[p];
    const int depth = quick ? position.quick_depth : position.depth;
    const uint64_t nodes = quick ? position.quick_nodes : position.nodes;
    const double best = per_position[p].best();

    printf("  %-12s %6d %12llu %11.1f %10.2f\n", position.name, depth,
           (unsigned long long)nodes, best, nodes / best / 1000.0);

    perft_total_ms += best;
  }

  printf("  %-12s %6s %12llu %11.1f %10.2f\n", "total", "",
         (unsigned long long)total_nodes, perft_total_ms,
         total_nodes / perft_total_ms / 1000.0);

  print_spread(perft_sweep);

  //-####################  generate_moves phase  ###########################-//

  std::vector<timing_t> per_position_gen(positions.size());
  std::vector<uint64_t> moves_per_call(positions.size(), 0);
  timing_t generate_sweep;

  for (const bench_position_t& position : positions) {
    load_FEN(position.fen, &game);
    generate_only(&game, generate_calls / 10);
  }

  for (int r = 0; r < repetitions; ++r) {
    double sweep_ms = 0.0;

    for (size_t p = 0; p < positions.size(); ++p) {
      load_FEN(positions[p].fen, &game);

      const auto start = std::chrono::steady_clock::now();
      const uint64_t produced = generate_only(&game, generate_calls);
      const double elapsed = ms_since(start);

      per_position_gen[p].add(elapsed);
      moves_per_call[p] = produced / generate_calls;
      sweep_ms += elapsed;
    }

    generate_sweep.add(sweep_ms);
  }

  printf("\ngenerate_moves  (no make_move, no unmake_move)\n");
  printf("  %-12s %11s %12s %11s %10s %11s\n", "position", "moves/call",
         "calls", "best ms", "Mcalls/s", "Mmoves/s");

  double generate_total_ms = 0.0;
  uint64_t total_calls = 0;
  uint64_t total_moves = 0;

  for (size_t p = 0; p < positions.size(); ++p) {
    const double best = per_position_gen[p].best();
    const uint64_t moves = moves_per_call[p] * generate_calls;

    printf("  %-12s %11llu %12llu %11.1f %10.2f %11.1f\n", positions[p].name,
           (unsigned long long)moves_per_call[p],
           (unsigned long long)generate_calls, best,
           generate_calls / best / 1000.0, moves / best / 1000.0);

    generate_total_ms += best;
    total_calls += generate_calls;
    total_moves += moves;
  }

  printf("  %-12s %11s %12llu %11.1f %10.2f %11.1f\n", "total", "",
         (unsigned long long)total_calls, generate_total_ms,
         total_calls / generate_total_ms / 1000.0,
         total_moves / generate_total_ms / 1000.0);

  print_spread(generate_sweep);

  //-###################  generate_captures phase  #########################-//
  // What a quiescence node asks for, and what the first stage of a staged
  // search asks for. Most search nodes fail high on one of the first few
  // captures, so the gap between this phase and the one above is the work a
  // staged search never has to do.

  std::vector<timing_t> per_position_cap(positions.size());
  std::vector<uint64_t> caps_per_call(positions.size(), 0);
  timing_t capture_sweep;

  for (const bench_position_t& position : positions) {
    load_FEN(position.fen, &game);
    generate_captures_only(&game, generate_calls / 10);
  }

  for (int r = 0; r < repetitions; ++r) {
    double sweep_ms = 0.0;

    for (size_t p = 0; p < positions.size(); ++p) {
      load_FEN(positions[p].fen, &game);

      const auto start = std::chrono::steady_clock::now();
      const uint64_t produced = generate_captures_only(&game, generate_calls);
      const double elapsed = ms_since(start);

      per_position_cap[p].add(elapsed);
      caps_per_call[p] = produced / generate_calls;
      sweep_ms += elapsed;
    }

    capture_sweep.add(sweep_ms);
  }

  printf("\ngenerate_captures  (what a quiescence node asks for)\n");
  printf("  %-12s %11s %11s %11s %10s\n", "position", "caps/call", "moves/call",
         "best ms", "Mcalls/s");

  double capture_total_ms = 0.0;

  for (size_t p = 0; p < positions.size(); ++p) {
    const double best = per_position_cap[p].best();

    printf("  %-12s %11llu %11llu %11.1f %10.2f\n", positions[p].name,
           (unsigned long long)caps_per_call[p],
           (unsigned long long)moves_per_call[p], best,
           generate_calls / best / 1000.0);

    capture_total_ms += best;
  }

  printf("  %-12s %11s %11s %11.1f %10.2f\n", "total", "", "", capture_total_ms,
         total_calls / capture_total_ms / 1000.0);

  print_spread(capture_sweep);

  if (generate_total_ms > 0.0) {
    printf(
        "\n  captures cost %.0f%% of a full generation. A node that fails "
        "high\n"
        "  on an early capture saves the other %.0f%%.\n",
        (capture_total_ms / generate_total_ms) * 100.0,
        (1.0 - capture_total_ms / generate_total_ms) * 100.0);
  }

  printf("\nnode counts verified against the published perft values.\n");

  return 0;
}
