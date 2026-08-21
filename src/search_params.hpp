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
  X(MAX_QSEARCH_DEPTH, "MaxQsearchDepth", 19, 1, 64)                            \
                                                                               \
  /* Reverse futility pruning. How much the opponent is assumed to be able to  \
     claw back per remaining ply, and the deepest node the assumption is made  \
     at.                                                                       \
                                                                               \
     Both were a first setting, one pawn per ply and the last few plies, and   \
     **both are SPSA-tuned since S085**: 75 -> 63 and 6 -> 15 over 60000 games  \
     at 2+0.02. The margin moved little. The depth bound moved a long way, and  \
     it no longer means what the sentence above it meant: at 15 the assumption  \
     is made at every depth this engine actually reaches -- median 11 at the    \
     tuning control -- so reverse futility is now depth-unbounded in practice   \
     rather than confined to the last few plies. That is what the tuner chose   \
     and the mate tests still pass at it, but the bound has stopped being the   \
     guard the comment describes, which is `RFP_MIN_PLY` and `beta < MATE_MIN`  \
     below. S033 for the original derivation, S085 for the values. */           \
  X(RFP_MARGIN,        "RfpMargin",       63,     0, 2000)                     \
  X(RFP_MAX_DEPTH,     "RfpMaxDepth",     15,      0, 63)                       \
                                                                               \
  /* The top of the tree is searched properly. Ply 1 and ply 2 are exempt      \
     because a static bound returned there is what the root compares against   \
     alpha, and a mate two moves away lives exactly that far down. Buying the  \
     two plies back costs 1.7 % of the nodes the rule saves; exempting a third \
     costs 27 %. S033.                                                         \
                                                                               \
     Two claims that stood here were wrong, and S085's run found them by       \
     walking this axis down to 0. Both are corrected against the code:         \
                                                                               \
     The root is not exempt because of this parameter. The guard at            \
     src/search.cpp:521 reads `!is_pv && ... ply >= RFP_MIN_PLY`, and search()  \
     calls the root at :853 with ply 0 and is_pv true, so `!is_pv` is what     \
     exempts the root and it does so at every setting of this. **0 and 1 are   \
     therefore the same engine** -- byte-identical node counts and best moves  \
     on every position driven, TRICKY 329568, CMK 260802, KILLER 53310 at      \
     depth 8 -- so 0 is a value no tuner can tell from its neighbour.          \
                                                                               \
     And "at ply 1 the mate cases in test_search go red" was 3 of the 18, not  \
     all of them: `mate in two is found at the right distance` (:125) and the  \
     two pruning cases (:1887, :1926), each failing on its black or            \
     material-leader arm at depth 3, and each asserting mate_found and a mate  \
     distance rather than a move. The other 15 stay green at 0 and 1, as does  \
     the 123769-assertion well-formedness sweep.                               \
                                                                               \
     **The tested floor is 2, not 3.** RFP does fire at ply 2 when this is set \
     to 2 -- node counts differ from 3, TRICKY 329598 against 375687 -- and    \
     every mate case still passes there. So the ply-2 exemption argued above   \
     is one no test exercises, which src/search.cpp:517 already concedes in    \
     its own words: "A mate deeper than ply 3 can still be missed for an       \
     iteration, and no test covers that." The declared minimum stays 0 pending \
     a decision on narrowing it to the tested 2 or the argued 3; what is not   \
     in doubt is that 0 and 1 cannot ship. */                                  \
  X(RFP_MIN_PLY,       "RfpMinPly",       3,      0, 63)                       \
                                                                               \
  /* Null move pruning gives the opponent a free move and searches what is     \
     left `depth - 1 - (NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR)` deep.     \
     Deeper searches can afford to give up more, since what is left is still   \
     enough to answer the question. A divisor of zero is a division by zero,   \
     which is the floor. */                                                    \
  X(NULL_MOVE_BASE,    "NullMoveBase",    3,      0, 16)                       \
  X(NULL_MOVE_DIVISOR, "NullMoveDivisor", 6,      1, 64)                       \
                                                                               \
  /* The two coefficients of the late move reduction fit,                      \
     `r = LMR_BASE/100 + log(depth) * log(move_number) / (LMR_DIVISOR/100)`.   \
     Hundredths because a UCI spin option is an integer and the fit is not.   \
     75 and 225 were the 0.75 and 2.25 the table was built from, and both of   \
     those divisions were exact in binary. **SPSA-tuned since S085**: 52 and   \
     182, so 0.52 and 1.82, and neither is exact in binary any more. Nothing   \
     depends on the exactness -- src/search.cpp:49-50 divides by 100.0 into a  \
     double and the quotient is floored to a ply count -- but the old comment  \
     claimed a property these values do not have, so it is withdrawn rather    \
     than left standing. Same floor reason as above. */                        \
  X(LMR_BASE,          "LmrBase",         52,     0, 400)                      \
  X(LMR_DIVISOR,       "LmrDivisor",      182,    1, 2000)                     \
                                                                               \
  /* The largest correction the lazy evaluation's expensive terms are allowed  \
     to apply. src/evaluation.hpp carries what the number means and what it    \
     was measured from; S039 re-decides it there. */                           \
  X(LAZY_EVAL_MARGIN,  "LazyEvalMargin",  184,    0, 2000)                     \
                                                                               \
  /* Aspiration windows. The root of an iteration is searched in a band around \
     the previous iteration's score instead of from -inf to +inf, and the band \
     is widened and the iteration repeated when the score falls outside it.    \
                                                                               \
     ASPIRATION_MIN_DEPTH is the first depth that gets a band. Below it there  \
     is nothing worth aspirating: the iterations are microseconds and the      \
     score is still moving. The floor is 2 because depth 1 has no previous     \
     score to build a band around, so a value of 1 would be a setting the      \
     search cannot honour rather than a narrower one.                          \
                                                                               \
     ASPIRATION_DELTA is the band's half-width in centipawns at the first      \
     attempt, and ASPIRATION_MAX_DELTA is where widening stops and the full    \
     window is used instead. The upper bound on the latter is arithmetic: past \
     MATE_MIN a band is wider than every non-mate score and can never fail, so \
     a value there switches the feature off by making it inert rather than by  \
     saying so.                                                                \
                                                                               \
     **SPSA-tuned since S085**, over 60000 games at 2+0.02: the first depth   \
     5 -> 2, its arithmetic floor, where the tuner sat for 55.6 % of its       \
     iterations; the half-width 50 -> 21, which is inside the 10 to 25 the     \
     surveyed engines run and was arrived at knowing nothing about them; and   \
     the stop 400 -> 437, which S021 measured as flat from 100 to 2000 and     \
     left alone, so a 9 % move in it is consistent with flat and is not a      \
     finding. What S021 measured, on the values S085 replaced:                 \
                                                                               \
     Chosen by measurement, adocs/data/S021_aspiration_sweep.tsv: 300          \
     positions over three independent 100-position samples of                  \
     adocs/data/S018_raw.tsv, stratified by game_phase(), node counts at depth \
     11 through the tune build. Pooled, this setting costs 0.9248 of the nodes \
     the feature switched off costs and no other setting swept is below        \
     0.9439. The third number is flat from 100 to 2000 and is left where it    \
     was.                                                                      \
                                                                               \
     One sample would have chosen wrongly. On the first alone this row reads   \
     0.8728, but delta 12 reads 0.9075 there and 1.0661 on the third: the      \
     ranking between settings is a property of the sample until the samples    \
     are pooled. Nodes at a fixed depth are not Elo, so what the sweep chose   \
     is what the SPRT then measured. S021. */                                  \
  X(ASPIRATION_MIN_DEPTH, "AspirationMinDepth", 2,   2, 64)                    \
  X(ASPIRATION_DELTA,     "AspirationDelta",    21,  1, 2000)                  \
  X(ASPIRATION_MAX_DELTA, "AspirationMaxDelta", 437, 1, 48000)                 \
                                                                               \
  /* Time management. The clock is turned into an allocation for this move,    \
     and the allocation into two limits: a soft one that decides whether to    \
     begin another iteration, and a hard one a timer is armed at and which     \
     stops the search inside an iteration. S089.                               \
                                                                               \
     TM_SOFT_PERCENT and TM_HARD_PERCENT are the two limits as a percentage    \
     of that allocation. The soft one at 60 is what shipped. The hard one is   \
     new: before S089 the timer was armed at the allocation itself, so an      \
     iteration begun just under the soft limit was cut off at 1.67 times it    \
     however close it was to finishing.                                        \
                                                                               \
     Bounds are arithmetic. Above 100 the soft limit is no longer the smaller  \
     of the pair and the decision to begin an iteration is taken after the     \
     allocation is already spent; at 0 no iteration after the first ever       \
     begins. Below 100 the hard limit sits under the soft one, the soft one's  \
     clamp drags it down to meet it, and the split is inert rather than        \
     tighter. Past ten times the allocation only the clamp to the clock is     \
     left binding and the setting stops being one. */                          \
  X(TM_SOFT_PERCENT,   "TmSoftPercent",   60,  1,   100)                       \
  X(TM_HARD_PERCENT,   "TmHardPercent",   300, 100, 1000)                      \
                                                                               \
  /* The allocation itself, at a sudden-death control. There is no time        \
     control boundary to divide the clock by, and the number that used to be   \
     divided by was DEFAULT_MOVES_TO_GO -- a fabricated 20, which made the     \
     engine play as though a control sat twenty moves out however long the     \
     game had left. A percentage of what is actually there claims nothing      \
     about the move count and decays on its own: 5 % spent leaves 95 %, and    \
     5 % of that is a smaller number without anything counting moves.          \
                                                                               \
     5 and 50 are what the old formula produced at movestogo 20, deliberately: \
     this step is the soft/hard split and the scaling below, and moving the    \
     base allocation in the same commit would put two changes in one SPRT.     \
                                                                               \
     The increment is spent rather than banked because it is replenished every \
     move; the bound is that it cannot be spent twice. At 0 the sudden-death   \
     allocation is the increment alone, which at no increment leaves only the  \
     floor -- hence the floor of 1. */                                         \
  X(TM_SUDDEN_DEATH_PERCENT, "TmSuddenDeathPercent", 5,  1, 100)               \
  X(TM_INCREMENT_PERCENT,    "TmIncrementPercent",   50, 0, 100)               \
                                                                               \
  /* What the search buys back. The soft limit is scaled by a percentage that  \
     starts at 100 and is moved by two things the previous iterations said.    \
                                                                               \
     A best move that has not changed for several iterations is unlikely to    \
     change in the next one, so each unchanged iteration takes                 \
     TM_STABILITY_PERCENT off, up to TM_STABILITY_MAX of them. The count       \
     cannot exceed MAX_DEPTH iterations, which is the upper bound; 0 switches  \
     the discount off. One iteration is not allowed to take more than half the \
     allocation away by itself, and the product of the two is held off zero by \
     TM_SCALE_MIN_PERCENT rather than by a bound this list cannot express.     \
                                                                               \
     A score that has fallen since the previous iteration means the position   \
     is turning out worse than it looked, which is exactly when another        \
     iteration is worth beginning. TM_FALLING_PERCENT is the whole grant and   \
     TM_FALLING_MAX_CP is the fall that earns it, linear below and flat above. \
     The divisor cannot be zero; twenty pawns is already every fall there is.  \
     At the shipping TM_SOFT_PERCENT and TM_HARD_PERCENT the scaled soft limit \
     reaches the hard limit at 400 and the clamp takes over, so that is the    \
     top.                                                                      \
                                                                               \
     TM_SCALE_MIN_PERCENT is the floor on the result. At 0 the soft limit is   \
     zero and no iteration after the first ever begins; at 100 the scale can   \
     never fall below the unscaled allocation and the stability discount is    \
     switched off. */                                                          \
  X(TM_STABILITY_MAX,     "TmStabilityMax",     8,   0, 126)                   \
  X(TM_STABILITY_PERCENT, "TmStabilityPercent", 4,   0, 50)                    \
  X(TM_FALLING_MAX_CP,    "TmFallingMaxCp",     100, 1, 2000)                  \
  X(TM_FALLING_PERCENT,   "TmFallingPercent",   50,  0, 400)                   \
  X(TM_SCALE_MIN_PERCENT, "TmScaleMinPercent",  30,  1, 100)
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
