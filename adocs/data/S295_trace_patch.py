#!/usr/bin/env python3
"""S295 reading (2): instrument a throwaway copy of src/search.cpp so every
node of an iterative-deepening search logs its window, path, return value and
the rule it returned by. Never applied to a tree that is committed; the
instrumented engine searches the same nodes as the one it was built from
(checked: `go depth 8` on the S294 cell prints the same eight lines, nodes
61914, before and after).

Log lines, to the file named by $S295_DIAG, for the root depth $S295_DEPTH
(default 8, 0 for every depth):
  S <n> depth <d> [alpha,beta]      one aspiration search starts
  E <n> score <s> aborted <0|1>     and ends
  N <id> <parent> <ply> d<depth> [alpha,beta] pv<0|1> cut<0|1> ex=<move>
    <path> -> <score> <rule> h=<key>   a negamax node, at its return
  Q <id> <parent> <ply> q<qply> [alpha,beta] <path> -> <score> <rule> h=<key>
                                    a quiescence node
  M <id> <move> n=<k> cd=<depth> red=<r> score=<s>   a move node <id> searched
  P <id> <move> rule=<prune_rule_t>                   a move node <id> skipped
`rule` is the return the node took: tt (with the entry), rfp, razor, null,
probcut, multicut, mate, qs, loop (the move loop: the stored type, moves
searched, whether one was skipped), and so on.

With --counterfactual the build also takes, when $S295_CF is set, a node that
skipped a move and failed low on a mated best as returning its own alpha
instead -- a reading of the cause, not a proposed fix.

    python3 adocs/data/S295_trace_patch.py <worktree>/src/search.cpp [--counterfactual]
"""
import sys

path = sys.argv[1]
counterfactual = "--counterfactual" in sys.argv[2:]
src = open(path).read()

qs_start = src.index("int quiescence(int alpha,")
ng_tmpl = src.index("template <bool PROBING>\nstatic int negamax_at(")
ng_end = src.index("int negamax(int alpha0,", ng_tmpl)
head, qs, ng, tail = src[:qs_start], src[qs_start:ng_tmpl], src[ng_tmpl:ng_end], src[ng_end:]


def rep(seg, old, new, count=1, all_=False):
    n = seg.count(old)
    if all_:
        assert n >= 1, (old, n)
        return seg.replace(old, new)
    assert n == count, (old, n)
    return seg.replace(old, new)


DIAG = r'''
#include <cstdio>
#include <string>
namespace s295diag {
FILE* out = nullptr;
bool init = false;
int want_depth = 8;
int root_depth = 0;
int search_no = 0;
move_t path[MAX_PLY + 4];
unsigned long long ids[MAX_PLY + 4];
std::string reason[MAX_PLY + 4];
unsigned long long next_id = 0;
inline bool on() { return out != nullptr && (want_depth == 0 || root_depth == want_depth); }
inline std::string mv(move_t m)
{
  if (m == 0) { return "null"; }
  std::string s = index_to_str(MOVE_FROM(m)) + index_to_str(MOVE_TO(m));
  if (MOVE_PROMOTED(m)) { s += "=" + std::to_string((int)MOVE_PROMOTED(m)); }
  return s;
}
inline std::string path_str(size_t ply)
{
  std::string s;
  for (size_t i = 0; i < ply; ++i) {
    if (i) { s += ","; }
    s += mv(path[i]);
  }
  return s.empty() ? "-" : s;
}
inline std::string fmt(const char* f, int a, int b, int c, int d)
{
  char buf[160];
  snprintf(buf, sizeof buf, f, a, b, c, d);
  return buf;
}
}  // namespace s295diag
#define DR(r) (s295diag::reason[ply] = (r))
'''

head = head + DIAG

# ---- quiescence: rename the body, add a logging wrapper after it.
qs = rep(qs, "int quiescence(int alpha,", "static int quiescence_body(int alpha,")
qs = rep(qs, "if (check_limits(state)) { return 0; }",
         'if (check_limits(state)) { return (DR("limit"), 0); }')
qs = rep(qs, "      return tt_score;",
         '      return (DR(s295diag::fmt("tt t=%d s=%d m=%d x=%d", tt_entry->type, tt_score, 0, 0)), tt_score);')
qs = rep(qs, "if (ply + 1 >= MAX_PLY) { return stand_pat; }",
         'if (ply + 1 >= MAX_PLY) { return (DR("maxply"), stand_pat); }')
qs = rep(qs, "if (static_cast<int>(qply) >= MAX_QSEARCH_DEPTH) { return stand_pat; }",
         'if (static_cast<int>(qply) >= MAX_QSEARCH_DEPTH) { return (DR(in_check ? "qmax-incheck" : "qmax"), stand_pat); }')
qs = rep(qs, "if (state->aborted) { return stand_pat; }",
         'if (state->aborted) { return (DR("aborted"), stand_pat); }')
qs = rep(qs, "      return stand_pat;", '      return (DR("standpat"), stand_pat);')
qs = rep(qs, "      return score;", '      return (DR("qcut " + s295diag::mv(moves[i])), score);')
qs = rep(qs, "    return mated;", '    return (DR("mated"), mated);')
qs = rep(qs, "  return best_value;",
         '  return (DR(s295diag::fmt("qend inchk=%d legal=%d cand=%d n=%d", (int)in_check, legal_moves, (int)count, (int)n) + " best=" + s295diag::mv(best_move)), best_value);')
qs = rep(qs, "    if (!make_move(game, moves[i])) { continue; }",
         "    s295diag::path[ply] = moves[i];\n    if (!make_move(game, moves[i])) { continue; }")
qs += r'''
int quiescence(int alpha, int beta, size_t ply, size_t qply, game_t* game,
               search_state_t* state)
{
  using namespace s295diag;
  const bool log = on();
  const unsigned long long saved_id = ids[ply];
  const std::string saved_reason = reason[ply];
  unsigned long long id = 0;
  if (log) {
    id = ++next_id;
    ids[ply] = id;
    reason[ply] = "?";
  }
  const int s = quiescence_body(alpha, beta, ply, qply, game, state);
  if (log) {
    fprintf(out, "Q %llu %llu %zu q%zu [%d,%d] %s -> %d %s h=%016llx\n", id,
            ply ? ids[ply - 1] : 0ULL, ply, qply, alpha, beta,
            path_str(ply).c_str(), s, reason[ply].c_str(), (unsigned long long)game->board.hash);
  }
  ids[ply] = saved_id;
  reason[ply] = saved_reason;
  return s;
}


'''

# ---- negamax_at: rename the body, add a logging wrapper after it.
ng = rep(ng, "template <bool PROBING>\nstatic int negamax_at(",
         "template <bool PROBING>\nstatic int negamax_at(int, int, int, size_t, game_t*, search_state_t*, move_t, bool, bool, move_t);\n"
         "template <bool PROBING>\nstatic int negamax_body(")
ng = rep(ng, "if (check_limits(state)) { return 0; }", 'if (check_limits(state)) { return (DR("limit"), 0); }')
ng = rep(ng, "if (ply + 1 >= MAX_PLY) { return evaluate(&game->board); }",
         'if (ply + 1 >= MAX_PLY) { return (DR("maxply"), evaluate(&game->board)); }')
ng = rep(ng, "      return DRAW_SCORE;", '      return (DR("rep"), DRAW_SCORE);')
ng = rep(ng, "if (!is_mated) { return DRAW_SCORE; }", 'if (!is_mated) { return (DR("fifty"), DRAW_SCORE); }')
ng = rep(ng, "if (is_insufficient_material(&game->board)) { return DRAW_SCORE; }",
         'if (is_insufficient_material(&game->board)) { return (DR("insuff"), DRAW_SCORE); }')
ng = rep(ng, "      return tt_score;",
         '      return (DR(s295diag::fmt("tt d=%d t=%d s=%d nms=%d", tt_entry->depth, tt_entry->type, tt_score, tt_entry->no_move_skipped) + " m=" + s295diag::mv(tt_entry->best_move)), tt_score);')
ng = rep(ng, "if (depth < 1) { return quiescence(alpha, beta, ply, 0, game, state); }",
         'if (depth < 1) { return (DR("qs"), quiescence(alpha, beta, ply, 0, game, state)); }')
ng = rep(ng, "      return rfp_eval - margin;",
         '      return (DR(s295diag::fmt("rfp eval=%d margin=%d", rfp_eval, margin, 0, 0)), rfp_eval - margin);')
ng = rep(ng, "if (state->aborted) { return 0; }", 'if (state->aborted) { return (DR("aborted"), 0); }', all_=True)
ng = rep(ng, "      return razor_score;", '      return (DR("razor"), razor_score);')
ng = rep(ng, "        return (null_score >= MATE_MIN) ? beta : null_score;",
         '        return (DR(s295diag::fmt("null ns=%d", null_score, 0, 0, 0)), (null_score >= MATE_MIN) ? beta : null_score);')
ng = rep(ng, "          return value;", '          return (DR("probcut " + s295diag::mv(pc_moves[j])), value);')
ng = rep(ng, "        return vscore;", '        return (DR("multicut"), vscore);')
ng = rep(ng, "    if (excluded_move != 0) { return alpha0; }", '    if (excluded_move != 0) { return (DR("excl-nomove"), alpha0); }')
ng = rep(ng, "    return is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE;",
         '    return (DR(is_in_check ? "mate" : "stalemate"), is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE);')
ng = rep(ng, "  return best_so_far;",
         '  return (DR(s295diag::fmt("loop type=%d legal=%d skipped=%d se=%d", (int)type, legal_moves_counter, (int)move_skipped, se_extension) + " best=" + s295diag::mv(best_move) + " eval=" + std::to_string(static_eval) + " peval=" + std::to_string(pruning_eval)), best_so_far);')
ng = rep(ng, "    if (!make_move(game, moves[i])) { continue; }",
         "    s295diag::path[ply] = moves[i];\n    if (!make_move(game, moves[i])) { continue; }")
ng = rep(ng, "        if (!make_move(game, pc_moves[j])) { continue; }",
         "        s295diag::path[ply] = pc_moves[j];\n        if (!make_move(game, pc_moves[j])) { continue; }")
ng = rep(ng, "      make_null_move(game);", "      s295diag::path[ply] = 0;\n      make_null_move(game);")
# pruned moves, both sites
ng = rep(ng, "          move_skipped = true;\n          continue;",
         '          move_skipped = true;\n          if (s295diag::on()) { fprintf(s295diag::out, "P %llu %s rule=%d\\n", s295diag::ids[ply], s295diag::mv(moves[i]).c_str(), (int)prune_rule); }\n          continue;')
ng = rep(ng, "      move_skipped = true;\n      unmake_move(game);\n      continue;",
         '      move_skipped = true;\n      if (s295diag::on()) { fprintf(s295diag::out, "P %llu %s rule=%d chk\\n", s295diag::ids[ply], s295diag::mv(moves[i]).c_str(), (int)prune_rule); }\n      unmake_move(game);\n      continue;')
# searched moves: after unmake in the main loop
ng = rep(ng, "      }\n    }\n\n    unmake_move(game);\n\n    // S132.",
         '      }\n    }\n\n    unmake_move(game);\n    if (s295diag::on()) { fprintf(s295diag::out, "M %llu %s n=%d cd=%d red=%d score=%d\\n", s295diag::ids[ply], s295diag::mv(moves[i]).c_str(), legal_moves_counter, child_depth, reduction, score); }\n\n    // S132.')
ng += r'''
template <bool PROBING>
static int negamax_at(int alpha0, int beta, int depth, size_t ply, game_t* game,
                      search_state_t* state, move_t prev_move, bool is_pv,
                      bool cut_node, move_t excluded_move)
{
  using namespace s295diag;
  const bool log = on();
  const unsigned long long saved_id = ids[ply];
  const std::string saved_reason = reason[ply];
  unsigned long long id = 0;
  if (log) {
    id = ++next_id;
    ids[ply] = id;
    reason[ply] = "?";
  }
  const int s = negamax_body<PROBING>(alpha0, beta, depth, ply, game, state,
                                      prev_move, is_pv, cut_node, excluded_move);
  if (log) {
    fprintf(out, "N %llu %llu %zu d%d [%d,%d] pv%d cut%d ex=%s %s -> %d %s h=%016llx\n",
            id, ply ? ids[ply - 1] : 0ULL, ply, depth, alpha0, beta,
            (int)is_pv, (int)cut_node, mv(excluded_move).c_str(),
            path_str(ply).c_str(), s, reason[ply].c_str(), (unsigned long long)game->board.hash);
  }
  ids[ply] = saved_id;
  reason[ply] = saved_reason;
  return s;
}


'''

tail = rep(tail, "  int score =\n      negamax_at<false>(alpha, beta, depth, 0, game, state, 0, true, false, 0);",
           '''  if (!s295diag::init) {
    s295diag::init = true;
    if (const char* p = getenv("S295_DIAG")) { s295diag::out = fopen(p, "w"); }
    if (const char* d = getenv("S295_DEPTH")) { s295diag::want_depth = atoi(d); }
  }
  s295diag::root_depth = depth;
  ++s295diag::search_no;
  if (s295diag::on()) { fprintf(s295diag::out, "S %d depth %d [%d,%d]\\n", s295diag::search_no, depth, alpha, beta); }
  int score =
      negamax_at<false>(alpha, beta, depth, 0, game, state, 0, true, false, 0);
  if (s295diag::on()) { fprintf(s295diag::out, "E %d score %d aborted %d\\n", s295diag::search_no, score, (int)state->aborted); fflush(s295diag::out); }''')

if counterfactual:
    ng = rep(ng, "  if (excluded_move == 0) {\n    const int to_store = normalize_score(best_so_far, ply);",
             '  if (getenv("S295_CF") && move_skipped && best_so_far < alpha0 &&\n'
             '      best_so_far <= -MATE_MIN) {\n'
             '    best_so_far = alpha0;\n'
             '  }\n'
             "  if (excluded_move == 0) {\n    const int to_store = normalize_score(best_so_far, ply);")

open(path, "w").write(head + qs + ng + tail)
print("patched")
