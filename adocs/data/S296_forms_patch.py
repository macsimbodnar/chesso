#!/usr/bin/env python3
"""S296: the two forms of DEC-318 as throwaway builds, and the counters that
read how often each fires. Rewrites a throwaway worktree's src/search.cpp and
src/chesso.cpp (of b477459, bench 5184358); never applied to a tree that is
committed. Each form is implemented from its description in DEC-318:

  (a) "a node that skipped a move and fails low with a mated best returns a
      bound that does not claim the mate": at the move loop's end, where a
      shallow-depth rule skipped a move (S292's `move_skipped`) and the best
      searched score is at or below -MATE_MIN, the node stores and returns
      `alpha0` -- an upper bound at the window's own edge, the claim the
      skipped moves' premise makes ("not above alpha") and no more. The pawn
      correction is fed `best_so_far` as before, so it still refuses the
      mated score (one change at a time).
  (b) "the shallow-depth move-loop pruning is not applied while no searched
      move has escaped the mated band": `may_prune`, the one eligibility the
      five rules read (futility, history pruning, quiet SEE pruning, capture
      SEE pruning, late move pruning's flag and skip), also requires
      `best_so_far > -MATE_MIN`.

  ship -- neither; the counters only (the shapes as the shipped tree meets
      them; bench 5184358 like the tree it is applied to).

The counters change no node: three globals the search thread increments, and,
when $S296_COUNT is set, one `info string` line per `go` just before
`bestmove` is printed, then reset (adocs/data/S296_census.py reads it):

  info string S296 ret=<n> bindn=<n> bindm=<n>
    ret    move-loop returns of a node that skipped a move with a best at or
           below -MATE_MIN (form a's firings; the shape on ship; 0 in form b
           by construction, a check of it)
    bindn  nodes at which the five rules' old eligibility held for some
           candidate while every searched move was still mated (form b's
           firings at node level; the shape on ship and in form a)
    bindm  the candidates over those nodes (moves form b keeps eligible for
           search; each still has to be one a rule would have taken)

    python3 adocs/data/S296_forms_patch.py <worktree> ship|a|b [--after-trace]

--after-trace: the worktree's src/search.cpp has had
adocs/data/S295_trace_patch.py applied first (S296's trace of S145 root 76,
adocs/data/S296_root76.txt); the traced move-loop return then returns the
form's value and its log line gains ` s296=<0|1> bestsf=<best_so_far>`, so a
node where form (a) returned alpha0 reads `s296=1` beside the mated best it
replaced.
"""
import sys

wt, form = sys.argv[1], sys.argv[2]
after_trace = "--after-trace" in sys.argv[3:]
assert form in ("ship", "a", "b"), form
sp = wt + "/src/search.cpp"
cp = wt + "/src/chesso.cpp"
src = open(sp).read()
csrc = open(cp).read()


def rep(text, old, new, count=1):
    n = text.count(old)
    assert n == count, (old, n)
    return text.replace(old, new)


COUNTERS = r'''
#include <string>
namespace s296 {
unsigned long long ret = 0, bindn = 0, bindm = 0;
}  // namespace s296
std::string s296_count_line()
{
  const std::string line = "info string S296 ret=" + std::to_string(s296::ret) +
                           " bindn=" + std::to_string(s296::bindn) +
                           " bindm=" + std::to_string(s296::bindm);
  s296::ret = s296::bindn = s296::bindm = 0;
  return line;
}
'''

src = rep(src, '#include "utils.hpp"\n', '#include "utils.hpp"\n' + COUNTERS)

src = rep(src, "  bool move_skipped = false;\n",
          "  bool move_skipped = false;\n  bool s296_bound = false;\n")

may_old = ("    const bool may_prune = pruning_node && legal_moves_counter >= 1 &&\n"
           "                           alpha < MATE_MIN && alpha > -MATE_MIN;\n")
may_new = ("    const bool s296_may_prune_old = pruning_node && legal_moves_counter >= 1 &&\n"
           "                           alpha < MATE_MIN && alpha > -MATE_MIN;\n"
           "    if (s296_may_prune_old && best_so_far <= -MATE_MIN) {\n"
           "      ++s296::bindm;\n"
           "      s296_bound = true;\n"
           "    }\n")
if form == "b":
    may_new += ("    const bool may_prune =\n"
                "        s296_may_prune_old && best_so_far > -MATE_MIN;\n")
else:
    may_new += "    const bool may_prune = s296_may_prune_old;\n"
src = rep(src, may_old, may_new)

store_old = ("  if (excluded_move == 0) {\n"
             "    const int to_store = normalize_score(best_so_far, ply);\n")
store_new = ("  if (s296_bound) { ++s296::bindn; }\n"
             "  const bool s296_shape = move_skipped && best_so_far <= -MATE_MIN;\n"
             "  if (s296_shape) { ++s296::ret; }\n"
             "  int s296_value = best_so_far;\n")
if form == "a":
    store_new += ("  if (s296_shape) {\n"
                  "    assert(alpha0 > best_so_far && alpha0 > -MATE_MIN);\n"
                  "    s296_value = alpha0;\n"
                  "  }\n")
store_new += ("  if (excluded_move == 0) {\n"
              "    const int to_store = normalize_score(s296_value, ply);\n")
src = rep(src, store_old, store_new)
if after_trace:
    src = rep(src, '" peval=" + std::to_string(pruning_eval)), best_so_far);\n}\n',
              '" peval=" + std::to_string(pruning_eval) + " s296=" + '
              'std::to_string((int)s296_shape) + " bestsf=" + '
              'std::to_string(best_so_far)), s296_value);\n}\n')
else:
    src = rep(src, "  return best_so_far;\n}\n", "  return s296_value;\n}\n")

csrc = rep(csrc, '#include "uci.hpp"\n',
           '#include "uci.hpp"\nstd::string s296_count_line();\n')
csrc = rep(csrc, '    uci_reply("bestmove " + best_move_str + ponder_move);\n',
           '    {\n'
           '      const std::string s296_line = s296_count_line();\n'
           '      if (getenv("S296_COUNT") != nullptr) { uci_reply(s296_line); }\n'
           '    }\n'
           '    uci_reply("bestmove " + best_move_str + ponder_move);\n', 2)

open(sp, "w").write(src)
open(cp, "w").write(csrc)
print("patched", form)
