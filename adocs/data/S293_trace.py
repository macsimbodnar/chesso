#!/usr/bin/env python3
"""S293. The scratch trace for mined row 171 under S291's scratch reverse
futility guard. NEVER SHIPPED: it rewrites src/search.cpp in a throwaway
worktree, never the tracked one, on top of S289_trace.py and S291_trace.py.

    git worktree add --detach ../chesso-s293t 6ebeadc
    python3 adocs/data/S289_trace.py ../chesso-s293t/src/search.cpp
    python3 adocs/data/S291_trace.py ../chesso-s293t/src/search.cpp
    python3 adocs/data/S293_trace.py ../chesso-s293t/src/search.cpp
    cmake -S ../chesso-s293t -B ../chesso-s293t/build-tune \\
        -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
    cmake --build ../chesso-s293t/build-tune -j12 --target chesso

Three additions:

  - every `S291 RFP-AGAINST-MATED` census line is followed by one `S293 AT`
    line: the iteration, the moves from the root to the node (`0000` for a
    null move) and the node's FEN (the engine's own generate_FEN), so the
    firing can be traced by S289_TRACE without reading a board.
  - $S293_GUARD_KEYS, '|'-separated FENs: S291's scratch guard (reverse
    futility not tried where the entry is a mated upper bound or exact score
    from a node that skipped no move) applies at those keys only. The
    one-node counterfactual for the guard.
  - $S293_STORE_TRACE set: every table store at a traced key (S289_TRACE)
    prints the stored type, depth and score, so what a node leaves behind is
    on the record next to what it returned.

With every variable unset the binary is the tree's own: `chesso bench` prints
the tree's total (6042933 at 6ebeadc).
"""
import sys

path = sys.argv[1]
src = open(path).read()


def sub1(old, new):
    global src
    n = src.count(old)
    if n != 1:
        raise SystemExit("pattern count %d for: %r" % (n, old[:80]))
    src = src.replace(old, new, 1)


sub1('''static bool rfp_allowed(hash_t h) { return absent(norfp, h); }''',
     '''static bool rfp_allowed(hash_t h) { return absent(norfp, h); }
static keyset_t s293_guard_keys;
static bool s293_inited = false;
static bool s293_store_trace = std::getenv("S293_STORE_TRACE") != nullptr;
static bool s293_guarded(hash_t h)
{
  if (!s293_inited) {
    s293_inited = true;
    load_keys("S293_GUARD_KEYS", &s293_guard_keys);
  }
  return !absent(s293_guard_keys, h);
}''')

sub1('''      !(s289::s291_rfp_guard && tt_entry != nullptr &&''',
     '''      !((s289::s291_rfp_guard || s289::s293_guarded(game->board.hash)) &&
        tt_entry != nullptr &&''')

sub1('''                     tt_entry_score, tt_entry_no_move_skipped ? 1 : 0);
      }''', '''                     tt_entry_score, tt_entry_no_move_skipped ? 1 : 0);
        std::string s293_path;
        for (size_t s293_i = state->root_history_size;
             s293_i < game->history.size; ++s293_i) {
          s293_path += s289::mv(game->history.entries[s293_i].move) + " ";
        }
        std::fprintf(stderr, "S293 AT it%d path %sfen %s\\n", s289::iter,
                     s293_path.c_str(), generate_FEN(&game->board).c_str());
      }''')

sub1('''    tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,
                   raw_eval, !move_skipped);''', '''    tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,
                   raw_eval, !move_skipped);
    if (s289::s293_store_trace && s289_traced) {
      std::fprintf(stderr, "%*s  store type %d depth %d score %d whole %d"
                   " move %s\\n", static_cast<int>(ply) * 2, "",
                   static_cast<int>(type), depth, best_so_far,
                   move_skipped ? 0 : 1, s289::mv(best_move).c_str());
    }''')

open(path, "w").write(src)
print("patched", path)
