#!/usr/bin/env python3
"""S292. The scratch census behind adocs/data/S292_cost.txt: where S289's
null-move table guard fires, what the bound it reads was stored by, and what
each kind of firing costs. NEVER SHIPPED: it rewrites src/search.cpp and
src/chesso.cpp in a throwaway worktree of the reverted tree (10f1e42, whose
engine is 4a3a0e5's byte for byte), never the tracked one.

    git worktree add --detach ../chesso-s292x 10f1e42
    python3 adocs/data/S292_census_patch.py ../chesso-s292x
    cmake -S ../chesso-s292x -B ../chesso-s292x/build -DCMAKE_BUILD_TYPE=Release
    cmake --build ../chesso-s292x/build -j12 --target chesso
    S292_GUARD=<mode> ../chesso-s292x/build/src/chesso    # driven over UCI

$S292_GUARD picks the guard, one binary for every side:

    0  no guard: the reverted tree, bench 6042933 (4a3a0e5, 10f1e42)
    1  S289's guard on every bound it reads: bench 6042928 (cd50178)
    2  the guard only where the bound's node skipped no move (kind P)
    3  the guard only where the bound's node skipped a move (kind S)
    4  the guard everywhere but kind S (P and U)

The guard condition is cd50178's, byte for byte in meaning: the node's entry an
upper bound or exact score at or below -MATE_MIN, read from the copies taken
before anything recursed, tested beside the depth floor. What the patch adds
reads and counts and never decides, except through the mode:

  * provenance. At negamax_at's one store, a mated-band upper bound or exact
    score is recorded in a side table keyed by the full 64-bit hash, with the
    stored score, depth and type, how many legal moves the node searched and
    how many it skipped by a shallow-depth rule (the `continue` before
    make_move: futility, history, quiet SEE, late move pruning, capture SEE;
    a skipped move that gives check is searched and is not counted). Where the
    guard's condition holds, the record is matched against the entry's own
    score, depth and type: kind P, matched and nothing skipped; kind S,
    matched and at least one move skipped (with the rules that skipped);
    kind U, no matching record (the entry came from elsewhere, or the record
    was replaced). Cleared on `ucinewgame` with the table.
  * outcomes. negamax_at becomes a wrapper around the original body, so every
    exit of a node where the condition held is seen with its return value and
    the nodes its subtree took: where the guard stopped the pass, whether the
    node's own search failed high (the bound was not a proof at this depth),
    came back mated, or neither; where the pass was made, whether it failed
    high.
  * the report. Before every `bestmove` of a `go`, one line
    `info string S292 key=value ...` with the counters since the last report,
    then reset. The driver (adocs/data/S292_census.py) reads it.

With $S292_GUARD 0 and 1 the binary must bench 6042933 and 6042928: the
counters cannot move a node, and the driver refuses a binary that does not.
"""
import sys

root = sys.argv[1]
spath = root + "/src/search.cpp"
cpath = root + "/src/chesso.cpp"
src = open(spath).read()
csrc = open(cpath).read()


def sub1(text, old, new):
    n = text.count(old)
    if n != 1:
        raise SystemExit("pattern count %d for: %r" % (n, old[:90]))
    return text.replace(old, new, 1)


HEADER = r'''
// ---- S292 SCRATCH CENSUS: never shipped ----
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <unordered_map>
namespace s292 {
static int mode = [] {
  const char* e = std::getenv("S292_GUARD");
  return (e != nullptr) ? std::atoi(e) : 0;
}();
struct prov_t
{
  int32_t score;
  int16_t depth;
  uint8_t type;
  uint8_t mask;
  uint16_t skipped;
  uint16_t searched;
};
static std::unordered_map<hash_t, prov_t> prov;
struct fire_t
{
  int kind = 0;  // 1 P, 2 S, 3 U
  bool applied = false;
  bool pass_fh = false;
  int depth = 0;
  int own_skipped = -1;
};
static fire_t fire[MAX_PLY + 1];
static uint64_t nm = 0;
static uint64_t elig = 0;
static std::map<std::string, uint64_t> c;
static const char* K[4] = {"", "P", "S", "U"};
static void add(const std::string& k, uint64_t v = 1) { c[k] += v; }
static void outcome(const fire_t& f, int v, int beta, uint64_t nodes)
{
  const std::string k = K[f.kind];
  if (f.applied) {
    add("app_" + k);
    add("appn_" + k, nodes);
    if (v >= beta) {
      add("appfh_" + k);
    } else if (v <= -MATE_MIN) {
      add(f.own_skipped == 0 ? "appmatedfull_" + k : "appmatedskip_" + k);
    } else {
      add("appmid_" + k);
    }
  } else {
    add("pass_" + k);
    add("passn_" + k, nodes);
    if (f.pass_fh) { add("passfh_" + k); add("passfhn_" + k, nodes); }
  }
}
}  // namespace s292

std::string s292_report();
void s292_clear();

std::string s292_report()
{
  std::string s = "info string S292 nm=" + std::to_string(s292::nm) +
                  " elig=" + std::to_string(s292::elig);
  for (const auto& kv : s292::c) {
    s += " " + kv.first + "=" + std::to_string(kv.second);
  }
  s292::nm = 0;
  s292::elig = 0;
  s292::c.clear();
  return s;
}

void s292_clear() { s292::prov.clear(); }

template <bool PROBING>
static int negamax_at(int alpha0,
                      int beta,
                      int depth,
                      size_t ply,
                      game_t* game,
                      search_state_t* state,
                      move_t prev_move,
                      bool is_pv,
                      bool cut_node,
                      move_t excluded_move);
// ---- end S292 header ----

'''

# The original body becomes negamax_at_inner; the header declares the wrapper
# its recursive calls now reach.
src = sub1(src, "template <bool PROBING>\nstatic int negamax_at(int alpha0,",
           HEADER + "template <bool PROBING>\nstatic int negamax_at_inner(int alpha0,")

WRAPPER = r'''

// ---- S292 SCRATCH CENSUS: the wrapper ----
template <bool PROBING>
static int negamax_at(int alpha0,
                      int beta,
                      int depth,
                      size_t ply,
                      game_t* game,
                      search_state_t* state,
                      move_t prev_move,
                      bool is_pv,
                      bool cut_node,
                      move_t excluded_move)
{
  s292::nm++;
  const size_t p = (ply <= MAX_PLY) ? ply : MAX_PLY;
  const s292::fire_t saved = s292::fire[p];
  s292::fire[p] = s292::fire_t{};
  const uint64_t n0 = state->explored_nodes;
  const int v =
      negamax_at_inner<PROBING>(alpha0, beta, depth, ply, game, state,
                                prev_move, is_pv, cut_node, excluded_move);
  const s292::fire_t mine = s292::fire[p];
  s292::fire[p] = saved;
  if (mine.kind != 0 && !state->aborted) {
    s292::outcome(mine, v, beta, state->explored_nodes - n0);
  }
  return v;
}
// ---- end S292 wrapper ----
'''

src = sub1(src, "  return best_so_far;\n}\n\n\n// The engine's node, and the only one it ever searches.",
           "  return best_so_far;\n}\n" + WRAPPER +
           "\n\n// The engine's node, and the only one it ever searches.")

# The entry's raw stored score and the key, beside the other copies.
src = sub1(src,
           "  const int tt_entry_score =\n      (tt_entry != nullptr) ? de_normalize_score(tt_entry->score, ply) : 0;\n",
           "  const int tt_entry_score =\n      (tt_entry != nullptr) ? de_normalize_score(tt_entry->score, ply) : 0;\n"
           "  const int32_t s292_entry_raw = (tt_entry != nullptr) ? tt_entry->score : 0;\n"
           "  const hash_t s292_key = game->board.hash;\n")

# The guard, cd50178's condition, and the census beside the depth floor.
src = sub1(src, "    if (depth - 1 - null_reduction >= 1) {\n      const int reduction = null_reduction;",
           r'''    const bool table_proves_mated =
        tt_entry != nullptr &&
        (tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) &&
        tt_entry_score <= -MATE_MIN;
    bool s292_block = false;

    if (depth - 1 - null_reduction >= 1) {
      s292::elig++;

      if (table_proves_mated) {
        int kind = 3;
        uint8_t mask = 0;
        const auto it = s292::prov.find(s292_key);

        if (it != s292::prov.end() && it->second.score == s292_entry_raw &&
            it->second.depth == tt_entry_depth &&
            it->second.type == tt_entry_type) {
          kind = (it->second.skipped == 0) ? 1 : 2;
          mask = it->second.mask;
        }

        const int m = s292::mode;
        s292_block = (m == 1) || (m == 2 && kind == 1) ||
                     (m == 3 && kind == 2) || (m == 4 && kind != 2);

        const std::string k = s292::K[kind];
        s292::add("band_" + k);
        s292::add("gap_" + k, static_cast<uint64_t>(depth - tt_entry_depth));
        s292::add("bd" + std::to_string(depth < 20 ? depth : 20) + "_" + k);
        if (tt_entry_type == TT_PV_NODE) { s292::add("bandexact_" + k); }
        if (kind == 2) {
          static const char* R[6] = {"", "fut", "hist", "see", "lmp", "seec"};
          for (int r = 1; r < 6; ++r) {
            if (mask & (1u << r)) { s292::add(std::string("bS_") + R[r]); }
          }
          s292::add("bS_skipped", it->second.skipped);
          s292::add("bS_searched", it->second.searched);
        }
        if (kind == 1) { s292::add("bP_searched", it->second.searched); }

        s292::fire[ply].kind = kind;
        s292::fire[ply].applied = s292_block;
        s292::fire[ply].depth = depth;
      }
    }

    if (depth - 1 - null_reduction >= 1 && !s292_block) {
      const int reduction = null_reduction;''')

src = sub1(src, "      if (null_score >= beta) {\n        // A mate score out of a null move search",
           "      if (null_score >= beta) {\n        if (s292::fire[ply].kind != 0) { s292::fire[ply].pass_fh = true; }\n        // A mate score out of a null move search")

# Counting the moves a shallow-depth rule skipped, in both instantiations.
src = sub1(src, "  move_t best_move = 0;\n\n  for (size_t i = 0;; ++i) {",
           "  move_t best_move = 0;\n  int s292_skipped = 0;\n  unsigned s292_mask = 0;\n\n  for (size_t i = 0;; ++i) {")
src = sub1(src, "        if (!move_gives_check(&game->board, moves[i])) { continue; }",
           "        if (!move_gives_check(&game->board, moves[i])) {\n          s292_skipped++;\n          s292_mask |= 1u << prune_rule;\n          continue;\n        }")
src = sub1(src, "      unmake_move(game);\n      continue;\n    }\n\n    legal_moves_counter++;",
           "      s292_skipped++;\n      s292_mask |= 1u << prune_rule;\n      unmake_move(game);\n      continue;\n    }\n\n    legal_moves_counter++;")

# The provenance record, at the one store.
src = sub1(src, "    tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,\n                   raw_eval);\n",
           r'''    tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,
                   raw_eval);

    if ((type == TT_ALPHA_NODE || type == TT_PV_NODE) &&
        best_so_far <= -MATE_MIN) {
      s292::prov[game->board.hash] = s292::prov_t{
          to_store, static_cast<int16_t>(depth), static_cast<uint8_t>(type),
          static_cast<uint8_t>(s292_mask),
          static_cast<uint16_t>(s292_skipped),
          static_cast<uint16_t>(legal_moves_counter)};
      s292::add(std::string(s292_skipped == 0 ? "stP" : "stS") +
                (type == TT_PV_NODE ? "_exact" : "_upper"));
    }

    if (s292::fire[ply].kind != 0) {
      s292::fire[ply].own_skipped = s292_skipped;
    }
''')

# chesso.cpp: the report before the search thread's bestmove, the side table
# cleared with the transposition table.
csrc = sub1(csrc, "static void reset_for_new_game()\n{",
            "std::string s292_report();\nvoid s292_clear();\n\nstatic void reset_for_new_game()\n{")
csrc = sub1(csrc, "  tt_reset(&tt);\n  correction_reset(&pawn_correction);\n  proven_mate_line.length = 0;",
            "  tt_reset(&tt);\n  s292_clear();\n  correction_reset(&pawn_correction);\n  proven_mate_line.length = 0;")
csrc = sub1(csrc, "    uci_reply(\"bestmove \" + best_move_str + ponder_move);\n    LOG_I << \"Search time: \" << timer.duration_str() << END_I;",
            "    uci_reply(s292_report());\n    uci_reply(\"bestmove \" + best_move_str + ponder_move);\n    LOG_I << \"Search time: \" << timer.duration_str() << END_I;")

open(spath, "w").write(src)
open(cpath, "w").write(csrc)
print("S292 census patch applied to", root)
