#pragma once
#include "data_structures.hpp"

// Positive means the side to move is better, whatever colour that is. Callers
// use the number as it comes; there is no sign to apply.
//
// The full score, every term, no shortcut. The tuner's model in
// tools/eval_model.hpp is checked against this one and not against the lazy
// form below.
int evaluate(const board_t* board);

// The terms that come straight off the accumulators make_move maintains:
// material and the tapered piece-square tables, and nothing that has to look at
// where the other pieces are standing. 1.36 ns, against 15.93 for the full
// score. Exposed because the margin below is a claim about the difference
// between the two, and a claim needs a test.
int evaluate_cheap(const board_t* board);

// The same score as evaluate(), except that the expensive terms are skipped
// when the cheap ones already put the position far enough outside the window
// that no correction they could apply would bring it back. S034.
//
// What is returned in that case is the cheap score, which is not the true
// score: it is a bound on the correct side of the window, which is all the
// caller was going to do with it.
int evaluate_lazy(const board_t* board, int alpha, int beta);

// The mobility weights, per piece type in knight, bishop, rook, queen order.
// Exposed so that the tuner's model in tools/eval_model.hpp can start from what
// the engine ships instead of from a copy of it, which would be free to drift.
extern const int mobility_mg[4];
extern const int mobility_eg[4];

// The largest correction the expensive terms are allowed to apply. The shortcut
// is only sound while that holds, so it is asserted over a position list rather
// than assumed -- see test_evaluation "the lazy shortcut cannot change a
// decision".
//
// 150 is above the largest correction observed over 149084 positions of S028
// self-play, where the tapered mobility term ran p50 19, p95 60, p99 81,
// p99.9 107 and a maximum of 143 centipawns.
#define LAZY_EVAL_MARGIN 150

// How far into the game the position is: 24 with a full set of pieces, 0 once
// only kings and pawns remain. Tapered terms interpolate on it. Pawns and kings
// do not count toward it, and promotions cannot push it above 24.
int game_phase(const board_t* board);

#define GAME_PHASE_MAX 24
int capture_score(const board_t* board, move_t move);
int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move);
