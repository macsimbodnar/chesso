#!/usr/bin/env python3
"""S291. The scratch trace for the multicut mined row at S083's re-landed
constants (adocs/data/S291_trace.txt). NEVER SHIPPED: it rewrites
src/search.cpp in a throwaway worktree, never the tracked one.

    git worktree add --detach ../chesso-s291t 49ad271
    git -C ../chesso-s291t apply --include=src/eval_tables.hpp \\
        --include=src/evaluation.cpp --include=src/search_params.hpp \\
        .tuning/coord/S083/s083_reland.diff        # fixture 38a15f4's src/
    python3 adocs/data/S289_trace.py ../chesso-s291t/src/search.cpp
    python3 adocs/data/S291_trace.py ../chesso-s291t/src/search.cpp
    cmake -S ../chesso-s291t -B ../chesso-s291t/build-tune \\
        -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
    cmake --build ../chesso-s291t/build-tune -j12 --target chesso_engine
    S289_TRACE='FEN|...' [S291_NO_RFP='FEN|...'] \\
        ~/.venv/chess/bin/python adocs/data/S291_row.py \\
        --repo ../chesso-s291t --tune --lo 10 --hi 10 2>&1 |
        python3 adocs/data/S291_trace_filter.py 0 PLY

The shipped constants' twin (S291_trace.txt section E, S291_reach.txt section 3)
is the same patch with the three hunks left out, built as build-tune-ship.
Applied to 49ad271's src/search.cpp after S289_trace.py the result is blob
84c79c8.

On top of S289_trace.py's patch (the node trace, its env var $S289_TRACE, and
its rule names on each exit), five additions:

  - $S291_NO_RFP, '|'-separated FENs: reverse futility is skipped at a node
    whose Zobrist key is one of them, and nowhere else. The one-node
    counterfactual: does the root report the mate when that firing alone is
    removed?
  - $S291_NO_NULL, the same for the null move: no pass at those nodes.
  - at a reverse futility return on a traced node, one line with the margin,
    beta, the static score, the estimate it decided on, and the node's table
    entry as copied out before any recursion (type, depth, score), so the
    entry's claim at the moment of the cut is on the record.
  - $S291_CENSUS set: every reverse futility return anywhere in the tree
    whose node's table entry is a mated upper bound or exact score (at or
    below -MATE_MIN) prints one `S291 RFP-AGAINST-MATED` line to stderr, with
    whether the entry's node searched every legal move (S292's
    `no_move_skipped`) -- the shape S289 found for the null move, asked of
    reverse futility. The row's own cut is on a lower bound and is not one.
  - $S291_RFP_GUARD set: reverse futility is not tried where S292's null-move
    condition holds -- the entry a mated upper bound or exact score from a
    node that searched every legal move. A scratch reading of what that
    census's firings cost, not a proposal's code.

With every variable unset the binary is the tree's own: `chesso bench` prints
the tree's total (3945487 at the re-landed constants, 6042933 shipped).
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


sub1('''static int index_of(hash_t h)''', '''struct keyset_t
{
  hash_t keys[32];
  int n = 0;
};
static keyset_t norfp, nonull;
static void load_keys(const char* var, keyset_t* set)
{
  const char* env = std::getenv(var);
  if (env == nullptr) { return; }
  std::string s(env);
  size_t pos = 0;
  while (pos < s.size() && set->n < 32) {
    size_t bar = s.find('|', pos);
    if (bar == std::string::npos) { bar = s.size(); }
    static game_t g;
    g = {};
    initialize_game_const_data(&g);
    if (load_FEN(s.substr(pos, bar - pos), &g)) { set->keys[set->n++] = g.board.hash; }
    pos = bar + 1;
  }
}
static bool norfp_inited = false;
static void init_norfp()
{
  if (norfp_inited) { return; }
  norfp_inited = true;
  load_keys("S291_NO_RFP", &norfp);
  load_keys("S291_NO_NULL", &nonull);
}
static bool absent(const keyset_t& set, hash_t h)
{
  for (int i = 0; i < set.n; ++i) {
    if (set.keys[i] == h) { return false; }
  }
  return true;
}
static bool rfp_allowed(hash_t h) { return absent(norfp, h); }
static bool null_allowed(hash_t h) { return absent(nonull, h); }
static int index_of(hash_t h)''')

sub1('''      game_phase(&game->board) > 0 &&
      !(s289::guard && tt_entry != nullptr &&''', '''      game_phase(&game->board) > 0 && s289::null_allowed(game->board.hash) &&
      !(s289::guard && tt_entry != nullptr &&''')

sub1('''  s289::init();
  s289::iter = depth;''', '''  s289::init();
  s289::init_norfp();
  s289::iter = depth;''')

sub1('''  if (!is_pv && !is_in_check && excluded_move == 0 &&
      static_cast<int>(ply) >= RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&
      beta < MATE_MIN && beta > -MATE_MIN) {''', '''  if (!is_pv && !is_in_check && excluded_move == 0 &&
      static_cast<int>(ply) >= RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&
      beta < MATE_MIN && beta > -MATE_MIN &&
      s289::rfp_allowed(game->board.hash) &&
      !(s289::s291_rfp_guard && tt_entry != nullptr &&
        tt_entry_no_move_skipped &&
        (tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) &&
        tt_entry_score <= -MATE_MIN)) {''')

sub1('''      s289::se = static_eval;
      s289::extra = rfp_eval;
      S289_RET("rfp", rfp_eval - margin);''', '''      s289::se = static_eval;
      s289::extra = rfp_eval;
      if (s289_traced) {
        std::fprintf(stderr, "%*s  rfp: margin %d beta %d static %d estimate %d"
                     " entry %s type %d depth %d score %d\\n",
                     static_cast<int>(ply) * 2, "", margin, beta, static_eval,
                     rfp_eval, tt_entry != nullptr ? "yes" : "no",
                     tt_entry != nullptr ? static_cast<int>(tt_entry_type) : -1,
                     tt_entry != nullptr ? tt_entry_depth : -1,
                     tt_entry != nullptr ? tt_entry_score : 0);
      }
      if (s291_census && tt_entry != nullptr &&
          (tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) &&
          tt_entry_score <= -MATE_MIN) {
        std::fprintf(stderr, "S291 RFP-AGAINST-MATED ply %zu depth %d beta %d"
                     " estimate %d margin %d entry type %d depth %d score %d"
                     " whole %d\\n", ply, depth, beta, rfp_eval, margin,
                     static_cast<int>(tt_entry_type), tt_entry_depth,
                     tt_entry_score, tt_entry_no_move_skipped ? 1 : 0);
      }
      S289_RET("rfp", rfp_eval - margin);''')

sub1('''static int index_of(hash_t h)''', '''static const bool s291_census_on = std::getenv("S291_CENSUS") != nullptr;
static const bool s291_rfp_guard = std::getenv("S291_RFP_GUARD") != nullptr;
static int index_of(hash_t h)''')
sub1('''  const bool s289_traced = s289::on && s289::index_of(game->board.hash) >= 0;''',
     '''  const bool s289_traced = s289::on && s289::index_of(game->board.hash) >= 0;
  const bool s291_census = s289::s291_census_on;''')

open(path, "w").write(src)
print("patched", path)
