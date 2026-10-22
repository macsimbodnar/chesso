#!/usr/bin/env python3
"""S289. The scratch trace that named the node and the rule hiding the ply-2
case's mate (adocs/data/S289_trace.txt). NEVER SHIPPED: it rewrites
src/search.cpp in a throwaway worktree of the tree BEFORE S289 (4a3a0e5 or an
ancestor with the same null-move block), never the tracked one.

    git worktree add --detach ../chesso-trace 4a3a0e5
    python3 adocs/data/S289_trace.py ../chesso-trace/src/search.cpp
    cmake -S ../chesso-trace -B ../chesso-trace/build-tune -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
    cmake --build ../chesso-trace/build-tune -j12 --target chesso
    S289_TRACE='FEN0|FEN1|...' [S289_GUARD=1] <a UCI driver: position fen FEN0, go depth N>

Every negamax and quiescence node whose Zobrist key is one of the
'|'-separated FENs in $S289_TRACE prints to stderr its entry, its exit with the
rule that returned (loop, tt, null, rfp, razor, probcut, multicut, qs-leaf,
mated, stalemate, repetition), and in its move loop every child's score and
every pruned move. The FENs of a line come from python-chess applying its moves
(the tool, never a mental board). With $S289_TRACE unset the binary is the
tree's own: bench prints the tree's total. $S289_GUARD set adds S289's guard
to the null-move condition, so one binary reads both sides (bench 6042928 on
4a3a0e5, the fix's Release total).
"""
import re, sys

path = sys.argv[1]
src = open(path).read()

def sub1(old, new, count=1):
    global src
    n = src.count(old)
    if n < count or (count == 1 and n != 1):
        raise SystemExit("pattern count %d for: %r" % (n, old[:80]))
    src = src.replace(old, new) if count != 1 else src.replace(old, new, 1)

HEADER = r'''
// ---- S289 SCRATCH TRACE: never shipped ----
#include <cstdio>
#include <cstdlib>
#include <string>
namespace s289 {
static bool on = false;
static bool inited = false;
static hash_t keys[32];
static int nkeys = 0;
static thread_local const char* reason = "";
static thread_local int se = 99999;
static thread_local int extra = 99999;
static int iter = 0;
static bool guard = std::getenv("S289_GUARD") != nullptr;
static void init()
{
  if (inited) { return; }
  inited = true;
  const char* env = std::getenv("S289_TRACE");
  if (env == nullptr) { return; }
  on = env[0] != 0;
  std::string s(env);
  size_t pos = 0;
  while (pos < s.size() && nkeys < 32) {
    size_t bar = s.find('|', pos);
    if (bar == std::string::npos) { bar = s.size(); }
    static game_t g;
    g = {};
    initialize_game_const_data(&g);
    if (load_FEN(s.substr(pos, bar - pos), &g)) {
      keys[nkeys++] = g.board.hash;
    } else {
      std::fprintf(stderr, "S289 trace: bad fen %s\n", s.substr(pos, bar - pos).c_str());
    }
    pos = bar + 1;
  }
}
static int index_of(hash_t h)
{
  for (int i = 0; i < nkeys; ++i) {
    if (keys[i] == h) { return i; }
  }
  return -1;
}
static std::string mv(move_t m)
{
  if (m == 0) { return "0000"; }
  return index_to_str(MOVE_FROM(m)) + index_to_str(MOVE_TO(m));
}
}  // namespace s289
#define S289_RET(why, val)                                                     \
  do {                                                                         \
    const int s289_v = (val);                                                  \
    s289::reason = (why);                                                      \
    return s289_v;                                                             \
  } while (0)
// ---- end S289 ----
'''

sub1('#define MATE_MAX 49000\n', HEADER + '\n#define MATE_MAX 49000\n')

# --- quiescence wrapper
sub1('''int quiescence(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state)
{
  assert(game != nullptr);

  state->explored_nodes++;
''', '''static int quiescence_impl(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state);

int quiescence(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state)
{
  if (!s289::on) { return quiescence_impl(alpha, beta, ply, qply, game, state); }
  const int idx = s289::index_of(game->board.hash);
  if (idx < 0) { return quiescence_impl(alpha, beta, ply, qply, game, state); }
  const int r = quiescence_impl(alpha, beta, ply, qply, game, state);
  std::fprintf(stderr, "%*sQ N%d it%d ply%zu q%zu [%d,%d] -> %d se(raw) %d\\n",
               static_cast<int>(ply) * 2, "", idx, s289::iter, ply, qply,
               alpha, beta, r, evaluate(&game->board));
  return r;
}

static int quiescence_impl(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state)
{
  assert(game != nullptr);

  state->explored_nodes++;
''')

# --- negamax wrapper
sub1('''template <bool PROBING>
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
  assert(game != nullptr);''', '''template <bool PROBING>
static int negamax_at_impl(int alpha0,
                      int beta,
                      int depth,
                      size_t ply,
                      game_t* game,
                      search_state_t* state,
                      move_t prev_move,
                      bool is_pv,
                      bool cut_node,
                      move_t excluded_move);

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
  if (!s289::on) {
    return negamax_at_impl<PROBING>(alpha0, beta, depth, ply, game, state,
                                    prev_move, is_pv, cut_node, excluded_move);
  }
  const int idx = s289::index_of(game->board.hash);
  if (idx < 0) {
    return negamax_at_impl<PROBING>(alpha0, beta, depth, ply, game, state,
                                    prev_move, is_pv, cut_node, excluded_move);
  }
  std::fprintf(stderr, "%*s> N%d it%d ply%zu d%d [%d,%d] pv%d cut%d ex %s prev %s\\n",
               static_cast<int>(ply) * 2, "", idx, s289::iter, ply, depth,
               alpha0, beta, is_pv ? 1 : 0, cut_node ? 1 : 0,
               s289::mv(excluded_move).c_str(), s289::mv(prev_move).c_str());
  s289::reason = "?";
  s289::se = 99999;
  s289::extra = 99999;
  const int r = negamax_at_impl<PROBING>(alpha0, beta, depth, ply, game, state,
                                         prev_move, is_pv, cut_node,
                                         excluded_move);
  std::fprintf(stderr, "%*s< N%d it%d ply%zu d%d [%d,%d] -> %d by %s se %d x %d\\n",
               static_cast<int>(ply) * 2, "", idx, s289::iter, ply, depth,
               alpha0, beta, r, s289::reason, s289::se, s289::extra);
  return r;
}

template <bool PROBING>
static int negamax_at_impl(int alpha0,
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
  assert(game != nullptr);
  const bool s289_traced = s289::on && s289::index_of(game->board.hash) >= 0;''')

# return sites in negamax_at_impl
sub1('''    if (classify_repetition(&game->history, &game->board,
                            state->root_history_size) ==
        repetition_kind_t::DRAW) {
      return DRAW_SCORE;
    }''', '''    if (classify_repetition(&game->history, &game->board,
                            state->root_history_size) ==
        repetition_kind_t::DRAW) {
      S289_RET("repetition", DRAW_SCORE);
    }''')
sub1('''    if (tt_entry_answers(tt_entry, depth, ply, alpha, beta, &tt_score)) {
      return tt_score;
    }''', '''    if (tt_entry_answers(tt_entry, depth, ply, alpha, beta, &tt_score)) {
      s289::extra = tt_entry->depth * 10 + tt_entry->type;
      S289_RET("tt", tt_score);
    }''')
sub1('''  if (depth < 1) { return quiescence(alpha, beta, ply, 0, game, state); }''',
     '''  if (depth < 1) { S289_RET("qs-leaf", quiescence(alpha, beta, ply, 0, game, state)); }''')
sub1('''      return rfp_eval - margin;''', '''      s289::se = static_eval;
      s289::extra = rfp_eval;
      S289_RET("rfp", rfp_eval - margin);''')
sub1('''      return razor_score;''', '''      s289::se = static_eval;
      S289_RET("razor", razor_score);''')
sub1('''        return (null_score >= MATE_MIN) ? beta : null_score;''',
     '''        s289::se = static_eval;
        s289::extra = depth - 1 - reduction;
        S289_RET("null", (null_score >= MATE_MIN) ? beta : null_score);''')
sub1('''          return value;
        }
      }
    }
  }''', '''          s289::se = static_eval;
          S289_RET("probcut", value);
        }
      }
    }
  }''')
sub1('''        return vscore;''', '''        s289::se = static_eval;
        S289_RET("multicut", vscore);''')
sub1('''    return is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE;''',
     '''    S289_RET(is_in_check ? "mated" : "stalemate", is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE);''')
sub1('''  return best_so_far;
}''', '''  s289::se = static_eval;
  s289::extra = static_cast<int>(type);
  S289_RET("loop", best_so_far);
}''')

# per-move logs in the loop
sub1('''      unmake_move(game);
      continue;
    }

    legal_moves_counter++;''', '''      if (s289_traced) {
        std::fprintf(stderr, "%*s  pruned %s rule %d\\n", static_cast<int>(ply) * 2, "",
                     s289::mv(moves[i]).c_str(), static_cast<int>(prune_rule));
      }
      unmake_move(game);
      continue;
    }

    legal_moves_counter++;''')
sub1('''    if (ply == 0) {
      root_nodes_add(state, moves[i],''', '''    if (s289_traced) {
      std::fprintf(stderr, "%*s  move %s #%d cd %d red %d -> %d  (a %d b %d)\\n",
                   static_cast<int>(ply) * 2, "", s289::mv(moves[i]).c_str(),
                   legal_moves_counter, child_depth, reduction, score, alpha, beta);
    }

    if (ply == 0) {
      root_nodes_add(state, moves[i],''')

# search(): iteration marker
sub1('''  state->root_history_size = game->history.size;
''',
     '''  state->root_history_size = game->history.size;

  s289::init();
  s289::iter = depth;
  if (s289::on) { std::fprintf(stderr, "ITER %d [%d,%d]\\n", depth, alpha, beta); }
''')

sub1('''  if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&
      excluded_move == 0 && beta < MATE_MIN && beta > -MATE_MIN &&
      game_phase(&game->board) > 0) {''', '''  if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&
      excluded_move == 0 && beta < MATE_MIN && beta > -MATE_MIN &&
      game_phase(&game->board) > 0 &&
      !(s289::guard && tt_entry != nullptr &&
        (tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) &&
        tt_entry_score <= -MATE_MIN)) {''')

open(path, "w").write(src)
print("patched", path)
