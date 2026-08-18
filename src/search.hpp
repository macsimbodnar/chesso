#pragma once
#include "data_structures.hpp"


// The widest window the root can be searched with. It is what search() used
// unconditionally before S021, and it is still the default, so a caller that
// does not aspirate searches exactly the tree it searched before.
inline constexpr int SEARCH_SCORE_INF = 2000000000;

// The window is the root's, and the whole tree below it inherits the bounds.
// Narrowing it is the point of an aspiration search and also its risk: what a
// node is allowed to prune is a property of the bound its parent passed down
// (DEC-060), so a caller that narrows this owns the mate exposure too.
search_t search(int depth,
                game_t* game,
                search_state_t* state,
                int alpha = -SEARCH_SCORE_INF,
                int beta = SEARCH_SCORE_INF);

// Whether a table entry answers this node outright, and with what score.
// `depth` is what the caller needs to have been searched: negamax passes its
// own remaining depth, quiescence passes TT_DEPTH_QS, and the comparison
// against the entry's depth is the whole of the rule that keeps a quiescence
// entry out of the main search. Split out of negamax and declared here so a
// test can hold that rule directly rather than inferring it from a tree. S094.
bool tt_entry_answers(const tt_entry_t* entry,
                      int depth,
                      size_t ply,
                      int alpha,
                      int beta,
                      int* score);

// The leaf search. Declared here only so the tests can drive it directly;
// nothing outside search.cpp calls it.
int quiescence(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state);

#ifdef CHESSO_TUNE
// The reduction the built table holds for a (depth, move number) pair. Tune
// build only, and it exists for one test: LMR_BASE and LMR_DIVISOR are read
// once, when the table is built, so a setoption that moved a coefficient
// without rebuilding would be invisible from outside. S073.
int search_lmr_reduction_probe(int depth, int move_number);
#endif
