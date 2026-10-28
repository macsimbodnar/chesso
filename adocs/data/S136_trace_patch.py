#!/usr/bin/env python3
"""S136 phase 2b: adocs/data/S295_trace_patch.py on the tree S296 left.

    python3 adocs/data/S136_trace_patch.py <scratch copy>/src/search.cpp

S295's patch instruments a throwaway src/search.cpp so every node logs its
window, path, return and rule (its docstring has the line format). It was
written against b477459, whose move loop ended `return best_so_far;`. S296's
form (a) ships since, and the loop now ends `return node_value;` -- `alpha0`
where a node that skipped a move found every searched move mated, its best
otherwise. This runs S295's patch unchanged except for that one anchor: the
loop's return is matched as `return node_value;`, and its rule string gains
`s296=<0|1> bestsf=<best>` as S296's own traces carried it (1 where form (a)
replaced the node's best by alpha0). Never applied to a committed tree; the
instrumented engine searches the same nodes as the one it was built from,
checked by `go depth` node counts in the step file.

    python3 adocs/data/S136_trace_patch.py <copy>/src/search.cpp --keys

`--keys` also builds a counterfactual read from the environment, a reading of
the cause and not a proposed change: at a node whose board hash is listed in
$S136_CF_KEYS (comma-separated hex, as the log's `h=` prints it), reverse
futility is not tried where $S136_CF_RULE contains `r` and the null move where
it contains `n`. Unset, the build searches the tree S295's patch does.
"""

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "S295_trace_patch.py")

OLD_ANCHOR = 'ng = rep(ng, "  return best_so_far;",'
NEW_ANCHOR = 'ng = rep(ng, "  return node_value;",'
OLD_TAIL = (' + " peval=" + std::to_string(pruning_eval)), best_so_far);\')')
NEW_TAIL = (' + " peval=" + std::to_string(pruning_eval)'
            ' + " s296=" + std::to_string((int)(node_value != best_so_far))'
            ' + " bestsf=" + std::to_string(best_so_far)), node_value);\')')


def main():
    code = open(SOURCE).read()
    for old, new in ((OLD_ANCHOR, NEW_ANCHOR), (OLD_TAIL, NEW_TAIL)):
        if code.count(old) != 1:
            raise SystemExit("S295_trace_patch.py no longer carries the line "
                             "this rewrites:\n  %s" % old)
        code = code.replace(old, new)
    keys = "--keys" in sys.argv[2:]
    sys.argv = [SOURCE] + [a for a in sys.argv[1:] if a != "--keys"]
    exec(compile(code, SOURCE, "exec"), {"__name__": "__main__"})
    if keys:
        counterfactual(sys.argv[1])
    return 0


CF = r'''
#include <cstdlib>
#include <cstring>
#include <set>
namespace s136cf {
inline bool off(unsigned long long key, char rule)
{
  static bool init = false;
  static std::set<unsigned long long> keys;
  static const char* rules = "";
  if (!init) {
    init = true;
    if (const char* r = getenv("S136_CF_RULE")) { rules = r; }
    if (const char* k = getenv("S136_CF_KEYS")) {
      std::string s(k);
      size_t at = 0;
      while (at < s.size()) {
        size_t comma = s.find(',', at);
        if (comma == std::string::npos) { comma = s.size(); }
        keys.insert(std::strtoull(s.substr(at, comma - at).c_str(), nullptr, 16));
        at = comma + 1;
      }
    }
  }
  return std::strchr(rules, rule) != nullptr && keys.count(key) != 0;
}
}  // namespace s136cf
'''

EDITS = (
    ("beta < MATE_MIN && beta > -MATE_MIN && !whole_search_mated_entry) {",
     "beta < MATE_MIN && beta > -MATE_MIN && !whole_search_mated_entry &&\n"
     "      !s136cf::off(game->board.hash, 'r')) {"),
    ("if (depth - 1 - null_reduction >= 1 && !whole_search_found_mated) {",
     "if (depth - 1 - null_reduction >= 1 && !whole_search_found_mated &&\n"
     "        !s136cf::off(game->board.hash, 'n')) {"),
)


def counterfactual(path):
    src = open(path).read()
    for old, new in EDITS:
        if src.count(old) != 1:
            raise SystemExit("src/search.cpp no longer carries the condition "
                             "--keys rewrites:\n  %s" % old)
        src = src.replace(old, new)
    anchor = "namespace s295diag {"
    if src.count(anchor) != 1:
        raise SystemExit("S295's diagnostic namespace is not where --keys "
                         "expects it")
    src = src.replace(anchor, CF + anchor)
    open(path, "w").write(src)
    print("counterfactual keys built in")


if __name__ == "__main__":
    sys.exit(main())
