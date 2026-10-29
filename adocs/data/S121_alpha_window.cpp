// S121, DEC-142. The window sweep behind tests/test_search.cpp's "a node that
// skipped moves claims no mate it did not search" and its golden ALPHA, as
// adocs/data/S296_forms.txt section 6 describes it and as no tracked file
// carried until now: S296 ran it from a scratch driver. This is that sweep,
// written from the section's prose and from the case's own drive.
//
// THE DRIVE, the case's: S295's ply-5 node, "6rk/5rqp/p7/2P1p3/P7/8/1P1n2P1/
// R5K1 w - - 0 35", loaded at ply 5 on a wiped 4 MB table (the case's
// search_fixture_t), probed at ply 5, negamax_probed(alpha, alpha + 1, 3, 5,
// ..., prev 0, non-PV). For every alpha from -47999 to 0 it records the
// shape the case REQUIREs before its property -- the loop ended, at least
// one move searched, g2g3 and g2g4 both skipped, the loop's best at or below
// -MATE_MIN (48000), every searched move mated -- and whether the property
// then holds (the value is the window's alpha). S296's section 6 swept the
// shipped tree before S296's rule, where the same shape returned the mated
// best; since the rule the shape returns alpha, so the shape alone is the
// band the case's preconditions need, and the value column checks the rule.
//
// THE READING: the runs of consecutive alphas with the shape, each run's ends;
// the run holding the golden, the golden's margin to its ends. A golden
// outside every run is a red precondition: the case stops at its REQUIREs.
//
// Built from the repository root after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -I src -I tests
//       -isystem tests/doctest/doctest adocs/data/S121_alpha_window.cpp
//       build/src/libchesso_engine.a -lpthread -o build/S121_alpha_window
//
//   build/S121_alpha_window [GOLDEN]        (GOLDEN defaults to -2108)
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"

namespace
{

constexpr int MATE_MIN_LOCAL = 48000;  // the case's, MATE_MIN
constexpr int PLY = 5;
constexpr int DEPTH = 3;
const char* const FEN = "6rk/5rqp/p7/2P1p3/P7/8/1P1n2P1/R5K1 w - - 0 35";

std::string uci(move_t m)
{ return index_to_str(MOVE_FROM(m)) + index_to_str(MOVE_TO(m)); }

}  // namespace


int main(int argc, char** argv)
{
  const int golden = (argc > 1) ? std::atoi(argv[1]) : -2108;

  static game_t game;
  static transposition_table_t tt;
  initialize_game_const_data(&game);
  tt_resize(&tt, 4);

  std::atomic_bool never_stop{false};

  struct run_t
  {
    int lo;
    int hi;
  };
  std::vector<run_t> runs;
  int value_not_alpha = 0;
  bool open = false;
  int golden_shape = -1;
  int golden_value = 0;
  std::string golden_pruned;

  for (int alpha = -47999; alpha <= 0; ++alpha) {
    if (!load_FEN(FEN, &game)) {
      fprintf(stderr, "load_FEN refused %s\n", FEN);
      return 1;
    }
    tt_reset(&tt);
    tt_new_search(&tt);
    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;
    search_node_probe_t probe = {};
    probe.ply = PLY;
    state.probe = &probe;

    const int value =
        negamax_probed(alpha, alpha + 1, DEPTH, PLY, &game, &state, 0, false);

    bool g3 = false;
    bool g4 = false;
    std::string pruned;
    for (int k = 0; k < probe.pruned_count; ++k) {
      const std::string m = uci(probe.pruned_moves[k]);
      g3 = g3 || m == "g2g3";
      g4 = g4 || m == "g2g4";
      pruned += " " + m;
    }
    const bool shape = !state.aborted && probe.loop_ended &&
                       probe.move_count > 0 && g3 && g4 &&
                       probe.loop_best <= -MATE_MIN_LOCAL;

    if (shape && value != alpha) { value_not_alpha++; }
    if (shape && !open) { runs.push_back({alpha, alpha}); }
    if (shape) { runs.back().hi = alpha; }
    open = shape;

    if (alpha == golden) {
      golden_shape = shape ? 1 : 0;
      golden_value = value;
      golden_pruned = pruned;
      printf("golden %d: shape %s, value %d, searched %d, loop best %d, "
             "pruned%s\n",
             golden, shape ? "yes" : "no", value, probe.move_count,
             probe.loop_best, pruned.c_str());
    }
  }

  printf("%s at ply %d, depth %d, window [alpha, alpha + 1], non-PV, a wiped "
         "table; every alpha from -47999 to 0\n",
         FEN, PLY, DEPTH);
  for (const run_t& r : runs) {
    printf("run %d .. %d (%d alphas)%s\n", r.lo, r.hi, r.hi - r.lo + 1,
           (golden >= r.lo && golden <= r.hi) ? "  <- holds the golden" : "");
    if (golden >= r.lo && golden <= r.hi) {
      printf("margin: %d below, %d above\n", golden - r.lo, r.hi - golden);
    }
  }
  printf("shape with a value other than alpha (the rule not firing): %d\n",
         value_not_alpha);
  (void)golden_value;
  printf("%s\n", golden_shape == 1 ? "S121-ALPHA-DONE: the golden is inside a run"
                                   : "S121-ALPHA-OUTSIDE: no run holds the golden");
  return golden_shape == 1 ? 0 : 1;
}
