#pragma once
#include <cstddef>

// The search constants that are a setting rather than a definition, in one
// addressable place. S073, and Phase A of adocs/eval_tuning_strategy.md.
//
// Two builds come out of this file and they are deliberately different code:
//
//   default          every parameter is `inline constexpr int`, so the compiler
//                    folds it exactly as the `#define` it replaced did. This is
//                    the binary that ships and the one every SPRT measures.
//   -DCHESSO_TUNE=ON every parameter is a variable, settable over UCI with
//                    `setoption name <UciName> value <n>`, and the release
//                    binary's surface is not touched. A constant the compiler
//                    can fold is not the same code as a variable it must load,
//                    so exposing the set in the release binary would be a
//                    play-altering change for a feature that produces no Elo by
//                    itself.
//
// The defaults exist once, in the list below, and both builds are generated
// from it. `search_param_info()`'s rows are compiled the same way in both, and
// they are what tests/test_search_params.cpp holds the live values against: a
// default that drifts in one build fails there rather than shipping, and the
// two builds are then equal member by member through that common anchor.
//
// Adding a parameter here adds a UCI option to the tune build, which
// tests/test_uci_surface.cpp will require MANUAL.md to document.
//
// Not in the set, on purpose: MATE_MAX, MATE_MIN, DRAW_SCORE, MIN, MAX and
// GAME_PHASE_MAX are definitions and not settings, and the move-ordering band
// constants are excluded because the bands clear each other by 100 points and
// tuning them inverts a capture against a killer silently (CLAUDE.md, S023).

// X(symbol, uci name, default, min, max)
//
// The ranges are new metadata and none of them narrows a value that ships. Each
// bound is either arithmetic (a divisor cannot be zero) or the constant's own
// stated purpose (history must stay under a killer), never a guess at where the
// good values are -- that is what the tuner is for.
// clang-format off
#define CHESSO_SEARCH_PARAMS(X)                                                \
  /* Keeps an accumulated history score from ever outranking a killer. The     \
     upper bound is that sentence: a killer scores 900000, so a history score  \
     allowed past it would invert the ordering silently. */                    \
  X(ORDER_HISTORY_MAX, "OrderHistoryMax", 600000, 0, 899999)                   \
                                                                               \
  /* How deep quiescence may keep going on its own. Without a bound a string   \
     of checks recurses forever, since an evasion is not a capture and does    \
     not shorten the line. At 0 quiescence returns its stand-pat score and     \
     never generates, which is why the floor is 1. */                          \
  X(MAX_QSEARCH_DEPTH, "MaxQsearchDepth", 8, 1, 64)                            \
                                                                               \
  /* Reverse futility pruning. How much the opponent is assumed to be able to  \
     claw back per remaining ply, and the deepest node the assumption is made  \
     at.                                                                       \
                                                                               \
     Both are a first setting and neither is fitted: the margin is one pawn    \
     per ply, the bound keeps the assumption to the last few plies where the   \
     static score is close to what a search would return anyway. S033. */      \
  X(RFP_MARGIN,        "RfpMargin",       75,     0, 2000)                     \
  X(RFP_MAX_DEPTH,     "RfpMaxDepth",     6,      0, 63)                       \
                                                                               \
  /* The top of the tree is searched properly. The root is exempt because its  \
     answer is the one that gets played; ply 1 and ply 2 are exempt because a  \
     static bound returned there is what the root compares against alpha, and  \
     a mate two moves away lives exactly that far down. Measured, not          \
     assumed: at ply 1 the mate cases in test_search go red, and buying the    \
     two plies back costs 1.7 % of the nodes the rule saves. Exempting a       \
     third costs 27 %. S033. */                                                \
  X(RFP_MIN_PLY,       "RfpMinPly",       3,      0, 63)                       \
                                                                               \
  /* Null move pruning gives the opponent a free move and searches what is     \
     left `depth - 1 - (NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR)` deep.     \
     Deeper searches can afford to give up more, since what is left is still   \
     enough to answer the question. A divisor of zero is a division by zero,   \
     which is the floor. */                                                    \
  X(NULL_MOVE_BASE,    "NullMoveBase",    2,      0, 16)                       \
  X(NULL_MOVE_DIVISOR, "NullMoveDivisor", 6,      1, 64)                       \
                                                                               \
  /* The two coefficients of the late move reduction fit,                      \
     `r = LMR_BASE/100 + log(depth) * log(move_number) / (LMR_DIVISOR/100)`.   \
     Hundredths because a UCI spin option is an integer and the fit is not:    \
     75 and 225 are the 0.75 and 2.25 the table was built from, and both       \
     divisions are exact in binary. Same floor reason as above. */             \
  X(LMR_BASE,          "LmrBase",         75,     0, 400)                      \
  X(LMR_DIVISOR,       "LmrDivisor",      225,    1, 2000)                     \
                                                                               \
  /* The largest correction the lazy evaluation's expensive terms are allowed  \
     to apply. src/evaluation.hpp carries what the number means and what it    \
     was measured from; S039 re-decides it there. */                           \
  X(LAZY_EVAL_MARGIN,  "LazyEvalMargin",  150,    0, 2000)
// clang-format on


#ifdef CHESSO_TUNE
#define CHESSO_DECLARE_SEARCH_PARAM(sym, name, def, lo, hi) extern int sym;
#else
#define CHESSO_DECLARE_SEARCH_PARAM(sym, name, def, lo, hi) \
  inline constexpr int sym = def;
#endif

CHESSO_SEARCH_PARAMS(CHESSO_DECLARE_SEARCH_PARAM)

#undef CHESSO_DECLARE_SEARCH_PARAM


// One row per parameter, in list order. Compiled identically in both builds:
// nothing below this line is inside a `#ifdef CHESSO_TUNE`, which is what makes
// it the single source of truth the two builds are compared through.
struct search_param_t
{
  const char* name;
  int default_value;
  int min_value;
  int max_value;
};

size_t search_param_count();
const search_param_t& search_param_info(size_t index);

// The value the build actually compiled: the constant in the default build, the
// variable in the tune build. Reading it costs a pointer indirection and is not
// on any search path -- the search uses the symbols directly.
int search_param_value(size_t index);


#ifdef CHESSO_TUNE
// Sets a parameter by its UCI name. Refuses an unknown name and a value outside
// the declared range, and returns false in both cases rather than clamping: a
// tuner that asked for something impossible should hear about it.
//
// Anything derived from a parameter is rebuilt here. That is not a detail -- a
// setter that updated LMR_BASE and left the already-built reduction table alone
// would report success and change nothing.
bool search_param_set(const char* name, int value);

// Defined in search.cpp. Rebuilds the late move reduction table.
void search_params_rebuild_derived();
#endif
