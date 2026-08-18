#pragma once
#include "data_structures.hpp"

// The depth a quiescence entry is stored at. Negative on purpose: negamax
// never asks the table for a depth below zero -- it recurses with depth - 1
// and only while depth >= 1, and every reduction is clamped so the child keeps
// at least one ply -- so an entry written here can never satisfy a main-search
// probe, while quiescence, which asks for this depth, accepts every entry in
// the table. The asymmetry is the whole point: a quiescence score has seen
// captures and nothing else and must not answer for a node that was going to
// search quiet moves. S094.
inline constexpr int TT_DEPTH_QS = -1;

// Allocates (or reallocates) the table. Must be called before any probe or
// store; everything else tolerates an unallocated table by doing nothing.
void tt_resize(transposition_table_t* tt, size_t megabytes);

void tt_free(transposition_table_t* tt);

void tt_reset(transposition_table_t* tt);

void tt_new_search(transposition_table_t* tt);

const tt_entry_t* tt_get_entry(const transposition_table_t* tt,
                               const board_t* board);

// best_move may be 0. Quiescence stores a node it stood pat on, and standing
// pat is not a move; a probe reads the zero back as "no move to try first",
// which is what it already does for a slot that was never written. S094.
//
// `eval` is the position's static evaluation, defaulted for the callers that
// never computed one. It has to be the score itself and not the bound the lazy
// shortcut returns in its place: a bound is only true on the side of the
// window it was taken at, and an entry outlives that window. S094.
void tt_store_entry(transposition_table_t* tt,
                    const board_t* board,
                    int depth,
                    int score,
                    node_type_t type,
                    move_t best_move,
                    int eval = TT_EVAL_NONE);
