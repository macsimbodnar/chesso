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
// Not in the set, on purpose: MATE_MAX, MATE_MIN, DRAW_SCORE, MIN and
// GAME_PHASE_MAX are definitions and not settings, and the move-ordering band
// constants are excluded because the bands clear each other by 100 points and
// tuning them inverts a capture against a killer silently (CLAUDE.md, S023).

// X(symbol, uci name, default, min, max)
//
// None of these ranges narrows a value that ships. A bound is one of three
// things and never a guess at where the good values are -- that is what the
// tuner is for:
//
//   arithmetic      a divisor cannot be zero
//   stated purpose  a history score must stay clear of the band above it
//   measured        RfpMinPly's floor of 2, the value below which the mate
//                   suite goes red
//
// The third kind is only as good as the test that produced it, so it names the
// step that measured it and the test that would go red. S142 set the first two
// of those and corrected the sentence above, which claimed only the first two
// kinds existed and named the wrong band for history.
// clang-format off
#define CHESSO_SEARCH_PARAMS(X)                                                \
  /* The gravity bound on a quiet history entry. Every score the table can     \
     hold lies in [-QuietHistoryMax, +QuietHistoryMax] by construction --      \
     `entry + b - entry*|b|/MAX` with `|b| <= MAX` maps that interval onto     \
     itself -- so this is the whole of the quiet ordering band, both edges.    \
                                                                               \
     Both bounds are arithmetic. The floor is 1 because the update divides by  \
     this value. The ceiling is INT16_MAX because the entry is an int16_t      \
     (src/data_structures.hpp), and it doubles as the overflow guard the       \
     update needs: the intermediate reaches MAX^2, and 32767^2 is 1.07e9,      \
     inside int32 with room. 8831 is **this project's own SPSA fit** and       \
     nothing else: S222's history lane of 2026-09-14, 1250 iterations over     \
     60000 games at 2+0.02 on books/UHO_4060_v3.epd, recorded in               \
     adocs/data/S222_spsa_trajectory.tsv. It replaces the 8192 this axis       \
     shipped as a power of two, which nothing had ever fitted -- S085          \
     predates S093 and named no history axis (DEC-198). S127 refits it after   \
     the block.                                                                \
                                                                               \
     The band above is the countermove's 700000 and the bands clear each other \
     by 100 (src/evaluation.cpp:33-37), so the ceiling has six orders of       \
     clearance rather than the 100 the old ORDER_HISTORY_MAX had to be trimmed \
     to. It is still asserted, in tests/test_evaluation.cpp "the declared      \
     history ceiling clears the band above it", and now at both edges: the     \
     malus makes a quiet score negative and nothing sits below quiets today,   \
     so the case pins the floor as well so that an arrival there (S025's       \
     losing captures) fails loudly. CLAUDE.md lists this as a one-way door --  \
     the symptom of getting it wrong is a strength regression, not a wrong     \
     node count. Replaces ORDER_HISTORY_MAX, whose 600000 was a saturation     \
     ceiling on an unbounded accumulator; gravity bounds the table instead.    \
     S093, 2026-08-20_plan_review-F14. */                                      \
  X(QUIET_HISTORY_MAX, "QuietHistoryMax", 8831, 1, 32767)                      \
                                                                               \
  /* The bonus a quiet move that caused a cutoff is credited with, and the     \
     malus every quiet tried before it at that node is charged, both as        \
     `QUAD*depth*depth + LIN*depth + CONST` and both clamped to the bound      \
     above before they reach the table.                                        \
                                                                               \
     Seeded as one form, `depth*depth`, which is what chesso already used and  \
     is one of the two forms CPW publishes; the other, `300*depth - 250`, is   \
     {0, 300, -250} here if the quadratic misfits. The bonus and the malus     \
     carry separate coefficients because splitting them is the published       \
     follow-up -- Weiss measured a split formula plus SPSA at +5.78 +/- 4.09   \
     LTC (PR #695), Lynx moved to x^2+x+c split (PR #1818) -- and until S222   \
     they shipped equal because nothing here had measured the split.           \
                                                                               \
     **All six values below are this project's own SPSA fit** and no other     \
     engine's numbers are behind any of them: S222's history lane of           \
     2026-09-14, 1250 iterations over 60000 games at 2+0.02 on                 \
     books/UHO_4060_v3.epd, recorded in adocs/data/S222_spsa_trajectory.tsv.   \
     The seeds they replace were {1, 0, 0} on both halves. The fit does split  \
     them, and in a shape nothing predicted: the bonus stays quadratic at      \
     6d^2 + 19d + 2 and the malus comes back **linear**, 17d + 36, with its    \
     quadratic coefficient fitted to zero. That is a hypothesis and not a      \
     result until the lane's own gainer SPRT decides the vector it sits in     \
     (DEC-019, `adocs/data/S222_sprt.sh`); the two quadratic axes were also    \
     the lane's coarsest, `c_end` at its floor of 1 over a useful region of a  \
     handful of integers, which the lane's header states rather than hides.    \
                                                                               \
     QUAD and LIN are non-negative because a negative one turns the bonus into \
     a penalty as depth grows, which inverts the mechanism rather than tuning  \
     it. Their ceilings are the overflow bound: at MAX_DEPTH 126 the three     \
     terms sum to under 1.7e7, four orders inside int32. CONST spans the       \
     entry's own type because the published linear form needs it negative. */  \
  X(HISTORY_BONUS_QUAD,  "HistoryBonusQuad",   6, 0, 1024)                     \
  X(HISTORY_BONUS_LIN,   "HistoryBonusLin",   19, 0, 4096)                     \
  X(HISTORY_BONUS_CONST, "HistoryBonusConst",  2, -32768, 32767)               \
  X(HISTORY_MALUS_QUAD,  "HistoryMalusQuad",   0, 0, 1024)                     \
  X(HISTORY_MALUS_LIN,   "HistoryMalusLin",   17, 0, 4096)                     \
  X(HISTORY_MALUS_CONST, "HistoryMalusConst", 36, -32768, 32767)               \
                                                                               \
  /* ONE-PLY CONTINUATION HISTORY, S222, and its own scale. The table is        \
     `cont_hist` in src/data_structures.hpp, keyed on the previous move's       \
     (piece, to) and this move's; it is written at every quiet cutoff beside    \
     plain history and summed into the quiet ordering score.                    \
                                                                               \
     **Three axes and not four, and that is a decision.** S024 built this table \
     on plain history's own coefficients and bound and measured H0 (DEC-194),   \
     which named two suspects: that two terms of equal weight doubled plain     \
     history's share of the quiet band, and that the shared gravity bound       \
     clipped the table. The obvious retry exposes a bonus, a malus, a bound of  \
     its own and a weight -- and those four carry an exact gauge freedom.       \
     Under `history_gravity_update` the entry is `e' = e + b - e|b|/M`, so      \
     scaling `b` and `M` together by k scales every entry by k and changes      \
     nothing the search can see; the read then multiplies by the weight, so     \
     (bonus, malus, bound, weight) enters the tree only through (bonus/bound,   \
     malus/bound, weight*bound). One of the four directions is therefore not a  \
     setting at all: SPSA would walk it and a meaningless endpoint would land   \
     in the vector an SPRT judges, which is the reason DEC-094 dropped          \
     `OrderHistoryMax` from S085 and DEC-200 dropped `TmHardPercent` from S127. \
     So the gauge is fixed instead of tuned: **the bound is a definition**,     \
     `CONT_HIST_BOUND` in src/data_structures.hpp, the int16_t ceiling the      \
     entry's own type sets, which is also the widest band the table can have    \
     and therefore the strongest available answer to the clipping suspect. The  \
     two hypotheses are then one question -- how much of the quiet band this    \
     term is allowed to span -- and `ContHistWeight` beside `QuietHistoryMax`   \
     is what the lane moves to answer it.                                       \
                                                                               \
     ContHistBonus and ContHistMalus are the graded update, in **thousandths    \
     of the table's own band at chesso's own median remaining depth**           \
     (`CONT_HIST_REF_DEPTH`, 11 -- RfpMaxDepth's comment above records that     \
     median, measured at S085's control). The published grading is quadratic in \
     the remaining depth and it stays quadratic; what the unit buys is          \
     resolution the axis would not otherwise have. A bare `QUAD * depth *       \
     depth` coefficient in the shape of HistoryBonusQuad has its whole useful   \
     region inside the integers 0 to 4, and an SPSA axis whose perturbation     \
     cannot be smaller than a quarter of its own range is an axis that cannot   \
     be fitted; expressed as a share of the band the same region spans 0 to     \
     1000.                                                                      \
                                                                               \
     Ranges by stated purpose. 0 is off for all three and is a true off value,  \
     not a range edge that merely behaves like one: at 0 the two update spans   \
     write nothing and the read term is identically 0. 1000 is where one update \
     at the reference depth already closes the whole band, past which the clamp \
     inside history_gravity_update makes every larger value the same engine.    \
     ContHistWeight's 1000 is the band-clearance ceiling, and it is the one-way \
     door CLAUDE.md names. **It was 2000 until S231 halved it, and the halving  \
     is arithmetic and not a judgement about where good values lie.** The quiet \
     band is [-(QuietHistoryMax + (w1 + w2)*CONT_HIST_BOUND/100), +the same]    \
     once the two-ply table of S231 adds a second weighted term to the same     \
     sum, and it has to stand 100 clear of the countermove band at 700000. The  \
     clearance is a property of the two weights' *total*, so with two tables    \
     sharing one band the ceiling each may declare is half of what one table    \
     could: at 1000 and 1000 the widest band the ranges admit is                \
     32767 + 327670 + 327670 = 688107, a clearance of **11893 -- the same       \
     number the single 2000 ceiling gave**, because 2000 and 1000 + 1000 span   \
     the same 655340. At 1050 each it would be 720873 --                        \
     32767 + 344053 + 344053, each weighted term through its own integer        \
     division as quiet_history_sum takes it -- and the band would swallow the   \
     countermove and both killers. **The halving moved no value**: it left      \
     S222's fitted 26 where it was and S231's lane then moved this axis to 24   \
     on its own evidence, nowhere near either edge. Asserted at both weights'   \
     declared maxima, not argued, in tests/test_evaluation.cpp "the declared    \
     history ceiling clears the band above it". Consequence worth knowing:      \
     tools/spsa_s222.json declares this axis 0 to 2000 and is the frozen record \
     of a run already taken -- `spsa_driver.py check` compares a config's       \
     bounds against the binary's and would now refuse it by name, which is      \
     correct and is not a reason to edit a finished run's input.                \
                                                                                \
     Seeds, DEC-084 as amended by DEC-105. No engine's constant is behind any   \
     of them, and S024's reuse of plain history's numbers is not evidence for   \
     these. Both shares were **(b), a derivation over chesso's own history      \
     scale**: plain history's shipped grading closed 11*11 / 8192 of its own    \
     band at the reference depth, which is 15 thousandths, so the table started \
     at the same *rate* as the table it sits beside and the lane moved it.      \
     ContHistWeight was **(b)** too: at its first setting of 25 the             \
     continuation term spanned 25 * 32767 / 100 = 8191 against plain history's  \
     shipped 8192, so the two started with equal authority -- which is exactly  \
     the equal-weight sum S024 measured and the null the first suspect is       \
     tested against.                                                            \
                                                                                \
     **The three values below are this project's own SPSA fit of 2026-09-19/20  \
     and nothing else**: S231's six-axis lane, 1250 iterations over 60000 games \
     at 2+0.02 on books/UHO_4060_v3.epd (tools/spsa_s231.json,                  \
     adocs/data/S231_spsa.sh, adocs/data/S231_spsa_trajectory.tsv), which moved \
     them 17 -> 18, 18 -> 17 and 26 -> 24. What that lane started from was      \
     S222's own fit of 2026-09-14 over 60000 further games of this project's    \
     own (DEC-198: 15 -> 17, 15 -> 18, 25 -> 26), and those three are named     \
     here as the seeds this lane replaced and not as values in force. At 24 the \
     one-ply term spans 24 * 32767 / 100 = 7864 against plain history's fitted  \
     QuietHistoryMax of 8831 and the two-ply term's own 7864, so the quiet band \
     ships as 8831 / 7864 / 7864 over its three terms. What the SPRT judges is  \
     that whole six-axis vector together with the table S231 added, and not     \
     this line -- one vector under one verdict, DEC-210's reading.              \
     */                                                                         \
  X(CONT_HIST_BONUS,   "ContHistBonus",   18, 0, 1000)                          \
  X(CONT_HIST_MALUS,   "ContHistMalus",   17, 0, 1000)                          \
  X(CONT_HIST_WEIGHT,  "ContHistWeight",  24, 0, 1000)                          \
                                                                                \
  /* TWO-PLY CONTINUATION HISTORY, S231, and its own scale again. The table is  \
     `cont_hist2` in src/data_structures.hpp, keyed on the (piece, to) of the   \
     move **two** plies back and this move's; written at every quiet cutoff     \
     beside the one-ply table and summed into the same quiet ordering score     \
     with a weight of its own. S222's H1 (DEC-210) is what it was waiting on.   \
                                                                                \
     **Three axes and not four, for DEC-209's reason unchanged.** The gauge     \
     argument is about one table and applies to this one word for word: bonus,  \
     malus, bound and weight reach the tree only through (bonus/bound,          \
     malus/bound, weight*bound), so a fourth axis would random-walk. The bound  \
     is therefore the same definition `CONT_HIST_BOUND` and **no second bound   \
     is added** -- the two tables are stored in the same type and bounded by    \
     the same ceiling, and what separates their scales is the two weights.      \
                                                                                \
     The unit is ContHistBonus's unit: thousandths of the table's own band at   \
     CONT_HIST_REF_DEPTH, so the two pairs of shares are directly comparable    \
     and the lane can move one against the other. Ranges by stated purpose and  \
     identical to the one-ply axes' for that reason: 0 is a true off value on   \
     all three (nothing is written and the ordering term is identically 0),     \
     1000 closes one whole band in a single update at the reference depth for   \
     the two shares, and 1000 on the weight is the **shared** band-clearance    \
     ceiling ContHistWeight's block above derives -- two weights over one band, \
     half the ceiling each, the same 11893 clearance as before.                 \
                                                                                \
     Seeds, DEC-084 as amended by DEC-105, all three **(b), a derivation over   \
     chesso's own numbers**: no engine's constant is behind any of them and     \
     none was read off a table, a wiki page or a release note.                  \
                                                                                \
       ContHist2Bonus's seed of 17 and ContHist2Malus's of 18 were the one-ply  \
       table's own fitted shares. The two tables are written at the same call   \
       site, over the same two spans, at the same reference depth, into bands   \
       of the same width -- so "start at the same rate as the table it sits     \
       beside" is the derivation S222 itself used against plain history, with   \
       the rate this project's own SPSA fit of 2026-09-14 rather than a shipped \
       seed. The lane then moved all six shares together.                       \
                                                                                \
       ContHist2Weight's seed of 26 was equal authority with the term beside    \
       it: at 26 the two-ply term spans 26 * 32767 / 100 = 8519, which is       \
       exactly what the one-ply term spanned at its then fitted 26, against     \
       plain history's fitted QuietHistoryMax of 8831. So the quiet band        \
       started at 8831 / 8519 / 8519 over its three terms. **Deliberately not   \
       0.** An SPSA axis that starts at a bound is a known pathology here --    \
       S085's RfpMinPly sat at one for 72.5 % of that run's iterations -- and   \
       the lane existed to fit this weight, so shipping the table switched off  \
       would have wasted the night. Whether equal authority was the right value \
       was the lane's question and the paragraph below is its answer; what is   \
       fixed here is that the seed is derived and stated.                       \
                                                                                \
     All three were first settings and **S231's own narrow lane has since       \
     fitted them** together with the one-ply table's three: 2026-09-19/20, 1250 \
     iterations over 60000 games at 2+0.02 on books/UHO_4060_v3.epd             \
     (tools/spsa_s231.json, adocs/data/S231_spsa.sh,                            \
     adocs/data/S231_spsa_trajectory.tsv), 17 -> 18, 18 -> 20 and 26 -> 24.     \
     **The values below are that fit and nothing else** and the seeds above are \
     named as the seeds they were. The weight ended near 26, which is one of    \
     the readings that lane pre-registered before a game was played: equal      \
     authority was about where the fit wanted it, the "at or under 5" row that  \
     would have owed a second attribution run is not triggered, and at 24 this  \
     term spans 24 * 32767 / 100 = 7864 -- again exactly what the one-ply term  \
     spans at its own fitted 24, against plain history's 8831. One gainer SPRT  \
     against 3a649c0, the tree before S231's first landing, decides the step -- \
     one vector under one verdict, DEC-210's reading. S127 still refits every   \
     axis after the block.                                                      \
     */                                                                         \
  X(CONT_HIST2_BONUS,  "ContHist2Bonus",  18, 0, 1000)                         \
  X(CONT_HIST2_MALUS,  "ContHist2Malus",  20, 0, 1000)                         \
  X(CONT_HIST2_WEIGHT, "ContHist2Weight", 24, 0, 1000)                         \
                                                                               \
  /* How deep quiescence may keep going on its own. Without a bound a string   \
     of checks recurses forever, since an evasion is not a capture and does    \
     not shorten the line. At 0 quiescence returns its stand-pat score and     \
     never generates, which is why the floor is 1. */                          \
  X(MAX_QSEARCH_DEPTH, "MaxQsearchDepth", 19, 1, 64)                            \
                                                                               \
  /* Reverse futility pruning. How much the opponent is assumed to be able to  \
     claw back per remaining ply, and the largest **remaining** depth the      \
     assumption is made at -- `depth <= RFP_MAX_DEPTH` in negamax_at(), so it  \
     is a distance to the leaves and not a distance from the root.             \
                                                                               \
     Both were a first setting, one pawn per ply and the last few plies, and   \
     **both are SPSA-tuned since S085**: 75 -> 63 and 6 -> 15 over 60000 games \
     at 2+0.02. The margin moved little. The depth bound moved a long way and  \
     stopped confining anything: at 15 the assumption is made at every depth   \
     this engine actually reaches -- median 11 at the tuning control -- so     \
     reverse futility is depth-unbounded in practice, and `RFP_MIN_PLY` below  \
     is the guard that is left. Not `beta < MATE_MIN`, which S145 measured     \
     inert: the static score cannot approach the mate band, so the condition   \
     never binds. **What bounds the static score is the material and table     \
     sums** -- finite sums of two- and three-digit constants over at most      \
     sixteen men a side -- and not evaluate_expensive()'s clamp, which this    \
     comment named until S213 and which bounds only the expensive stage.       \
     test_evaluation "the static score never reaches the mate band" asserts    \
     it, and negamax_at() says the same there in its own words.                \
                                                                               \
     **The declared range stays 0 to 63 (DEC-095), and that is a decision.**   \
     S145 swept the ceiling against 48 constructed forced mates with the floor \
     held at 3 and found the deep classes belong to this bound and not to the  \
     floor. S148 re-swept it over the 82-row set at every value from 0 to 15   \
     and then measured what the elbow costs. **The default stays 15, on an     \
     SPRT, and the deep mates it loses are its measured price.** At 15 the set \
     reads 39 of 82 exact, 12 of 24 mates in three, 1 of 16 in four and 0 of   \
     16 in five; at 4 -- the largest ceiling where both deep classes are       \
     non-zero -- it reads 52, 18, 7 and 1. That candidate lost: nElo -7.31     \
     +/- 5.60, Elo -5.66 +/- 4.33 over 14808 games at 8+0.08, H0 accepted      \
     against {-5, 0}. So a ceiling low enough to find the deep mates costs     \
     more than five nElo of ordinary play, and the mate counts above are what  \
     this engine pays for the pruning. S148 for the verdict, DEC-158 for the   \
     trade. The declared range is untouched by all of it and stays 0 to 63     \
     (DEC-095): the tuner keeps every value. S033 for the derivation, S085 for \
     the values, adocs/data/S145_rfp_sweep.log for the first sweep,            \
     adocs/data/S148_rfp_ceiling_sweep.log for the finer grid and              \
     adocs/data/S148_sprt.log for the run. */                                  \
  X(RFP_MARGIN,        "RfpMargin",       63,     0, 2000)                     \
  X(RFP_MAX_DEPTH,     "RfpMaxDepth",     15,     0, 63)                       \
                                                                               \
  /* The top of the tree is searched properly. Plies 0, 1 and 2 are exempt at  \
     the shipping value, because a static bound returned there is what the     \
     root compares against alpha and a mate two moves away lives exactly that  \
     far down. Buying the two plies back costs 1.7 % of the nodes the rule     \
     saves; exempting a third costs 27 %. S033.                                \
                                                                               \
     The root is not exempt because of this parameter, and that is still the   \
     caveat to read the bound with. The guard in negamax_at() is               \
     `!is_pv && ... ply >= RFP_MIN_PLY`, and search() enters negamax_at() at   \
     ply 0 with is_pv true, so `!is_pv` exempts it at every setting.           \
     **0 and 1 are therefore the same engine** -- byte-identical node counts   \
     and best moves, 329568, 260802 and 53310 at depth 8 on the three          \
     positions S085 measured (KIWIPETE_POS, BLOCKED_CENTRE_POS and             \
     KILLER_POS, the first two under the names they carried then) -- so 0 was  \
     a value no tuner could tell from its neighbour.                           \
                                                                               \
     **The declared minimum is 2: DEC-095 decided it and S145 earned it.**     \
     It stood at 0 until S142, and the evidence for raising it used to be      \
     three hand-picked mates in two, two of which were picked for a different  \
     engine. S145 replaced them with 48 constructed forced mates spanning      \
     distances two to five, each proved by exhaustive enumeration and          \
     corroborated by stockfish, asserted under the engine's own iterative      \
     deepening in tests/test_engine.cpp's "engine: mate safety". Swept there   \
     with RfpMaxDepth held at 15, and re-measured at S142 before this bound    \
     was set: at 2 and above the mate in two class is whole and immediate,     \
     **16 of 16 exact at delay 0**; at 1 and 0 it is **13 of 16 exact, with    \
     delays reaching 7 iterations**. The suite asserts both halves -- the      \
     distance and the iteration it first appears at -- so it goes red below 2, \
     which is the value S085's run spent 906 of its 1250 iterations at. How    \
     many of its assertions fail there is S154's to state; 13 of 16 is the     \
     sweep's exact count and not that number.                                  \
     adocs/data/S145_rfp_sweep.log; 2026-08-20_plan_review-F08.                \
                                                                               \
     2 and 3 are indistinguishable on that set, 24 of 48 each, and on nodes,   \
     1.0003 by sum over 6347 positions, so the choice between the tested floor \
     and the argued one was free and was the owner's. History, kept because it \
     is the reason the old text was wrong: "at ply 1 the mate cases in         \
     test_search go red" was 3 of 18, and all three are the same mate-in-two   \
     geometry, which is what S145 exists to have replaced. */                  \
  X(RFP_MIN_PLY,       "RfpMinPly",       3,      2, 63)                       \
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
  /* LATE MOVE REDUCTION BY NODE TYPE, S098 verdict 2. Four signed plies on     \
     top of the table above, each behind its own constant and each with 0 as    \
     an off value inside its range, so a failing verdict bisects by release     \
     rebuild rather than by four SPRTs (DEC-063, DEC-214). All four are         \
     properties of the **node** and not of the move, so the sum is computed     \
     once at the node and read by both consumers: the reduction a late quiet    \
     is searched with, and S109's shallow-depth gate, which is the same         \
     number since this step (`lmr_depth_of` in src/search.cpp).                 \
                                                                               \
     The published record says which direction is worth trying and seeds        \
     nothing (DEC-019, DEC-105): cutnode +1 measured +9.34 at Lynx in the       \
     3119-3138 band and a second ply on top measured -17.62 there, !improving   \
     +1 +4.64, a capturing hash move +1.87, and the PV decrement is Weiss's     \
     +3.78 from 2019 -- records, all of them above or at this engine's band,    \
     and no engine's ply count is a seed here wherever it is republished        \
     (DEC-084 as amended by DEC-105).                                           \
                                                                               \
     **Every default below is DEC-105 (c), the midpoint of the declared         \
     range**, and the range is 0 to 2 by stated purpose: 0 is off, 1 is the     \
     published class of adjustment, and 2 is where the term alone equals what   \
     the table returns for the first reducible move at the median depth --      \
     `lmr_reduction(11, 4)` is 2 -- past which the term replaces the            \
     ordering's estimate instead of adjusting it, which is a different rule.    \
     A ply count has no unit to derive from and no publication states one.      \
     S127 refits all four with LmrBase and LmrDivisor after the block.          \
                                                                               \
     LMR_CUTNODE applies where the node is predicted to fail high. The          \
     prediction is CPW's (src/search.cpp `first_child` and the three functions  \
     beside it); a wrong one costs rating and nothing else, which is what the   \
     Debug assert and the alternation cases are for.                            \
                                                                               \
     LMR_NOT_IMPROVING applies where `improving_at` is false. The asymmetry is  \
     the published shape -- reduce *more* when not improving, rather than less  \
     when improving, which failed where it was tried.                           \
                                                                               \
     LMR_TT_CAPTURE applies where the entry's own move is a capture. The move   \
     being reduced is quiet by the reduction's own eligibility, so this is a    \
     statement about the node and not about the two moves' relation.            \
                                                                               \
     LMR_PV is **subtracted** at a principal variation node. The other          \
     published form is a larger first-move bound at PV nodes; the subtraction   \
     composes with the three terms above in one integer where a second bound    \
     would not, and S098's file records the choice. It is the only negative     \
     term, and it never reaches the shallow-depth gate: a PV node is not a      \
     pruning node. */                                                          \
  X(LMR_CUTNODE,       "LmrCutNode",      1,      0, 2)                        \
  X(LMR_NOT_IMPROVING, "LmrNotImproving", 1,      0, 2)                        \
  X(LMR_TT_CAPTURE,    "LmrTtCapture",    1,      0, 2)                        \
  X(LMR_PV,            "LmrPv",           1,      0, 2)                        \
                                                                               \
  /* THE RE-SEARCH DEPTH, S098 verdict 3. A reduced late move that beat alpha   \
     is owed a zero-window repeat, and until this step that repeat always ran   \
     at `child_depth`. It now answers the reduced search instead, and **only    \
     upward**: one ply deeper where the reduced score cleared the node's        \
     fail-soft best by LMR_DEEPER_MARGIN with a reduction of at least           \
     LMR_DEEPER_MIN_REDUCTION (`lmr_research_depth` in src/search.cpp), and at  \
     `child_depth` everywhere else. The full-window re-search is untouched and  \
     still runs at `child_depth`.                                               \
                                                                               \
     UNITS. The margin is compared against search scores, so it is in chesso's  \
     material scale -- `piece_value` in src/eval_tables.hpp, where PAWN is 94.  \
     Plies have no unit.                                                        \
                                                                               \
     **LMR_DEEPER_MARGIN is DEC-105 (c), the midpoint of a range stated by      \
     purpose**: 0 to PAWN, whose top is the point past which the re-search      \
     depth would be decided on more material than a pawn of window, and whose   \
     midpoint is the integer 47. The published record says only that the        \
     direction is worth trying and seeds nothing (DEC-019, DEC-084 as amended   \
     by DEC-105): the bare form measured -7.07 and the guarded form +3.11 in    \
     the 3138-3224 band, which is above this engine's.                          \
                                                                               \
     LMR_DEEPER_MIN_REDUCTION is that guard and it is DEC-105 **(b)**, a        \
     derivation over this engine's own site: the re-search exists only where    \
     `reduction > 0`, so 1 is a guard that says nothing, and 2 is the smallest  \
     value at which it does.                                                    \
                                                                               \
     THE OFF VALUE, and it is not where section 4 of the step file expected it. \
     `LmrDeeperMargin` is **not** off at its range top -- the fail-soft best    \
     sits below alpha at every scout node, and the census measured              \
     `score > best + 94` still true at 2.86 % of re-search sites at depth 12 -- \
     so the rule's off value is `LmrDeeperMinReduction` at **its** range top,   \
     which is above every reduction the clamp to `[0, child_depth - 1]`         \
     admits (MAX_DEPTH is 126 and the reduction cannot exceed depth - 2). At    \
     that value the re-search runs at `child_depth` everywhere and the engine   \
     is the one before this verdict, bench signature included, which is what    \
     the release-rebuild bisection rested on (DEC-063, DEC-214, DEC-215).       \
                                                                               \
     **THE SHALLOWER HALF WAS MEASURED AND REMOVED, 2026-09-18.** A second      \
     path shipped beside this one until the bisection read it: a ply *off* the  \
     re-search where the score beat alpha by less than an LmrShallowerMargin of \
     its own. The pair measured `Elo -9.97 +/- 7.56` over 4496 games, the whole \
     interval below zero; leg 1 switched the shallower path off against the     \
     same reference and read H1, `Elo 5.75 +/- 4.37` and `nElo 7.44 +/- 5.65`   \
     over 14510 games. adocs/data/S098_v3_leg1_sprt.sh pre-registered that      \
     reading as the one where the path leaves, so the constant, its branch and  \
     the `reduction >= 2` guard written only for it are gone rather than left   \
     at an off value. The removal is behaviour-neutral at the shipped           \
     configuration and proved so by the signature, not asserted.                \
                                                                               \
     LmrDeeperMinReduction's floor of 1 is arithmetic -- the site already       \
     requires a reduction of at least 1 -- and its ceiling is the off value     \
     above. S127 refits both with LmrBase and LmrDivisor after the block. */    \
  X(LMR_DEEPER_MARGIN,        "LmrDeeperMargin",        47, 0, 94)             \
  X(LMR_DEEPER_MIN_REDUCTION, "LmrDeeperMinReduction",   2, 1, 126)            \
                                                                               \
  /* The shallow-depth pruning block, S109. Four rules over quiet moves, all    \
     of them gated on the **reduction-adjusted** depth                          \
     `lmr_depth = max(0, depth - lmr_adjusted_reduction(depth, move_number,     \
     node_adjustment))` and not on the node's own remaining depth: a move the   \
     ordering put late is already searched shallower than the node is deep, so  \
     the margin it is pruned against is the shallow one. **The adjustment is    \
     the node-type sum above since S098 verdict 2** -- the gate and the         \
     reduction are one number -- and at the four off values it is 0 and this    \
     is the raw table again, which is what restores S109's exact gate in one    \
     release rebuild.                                                           \
                                                                               \
     **Every cap below reads `lmr_depth < CAP`, not `<=`.** `lmr_depth` can be  \
     0, so `<= 0` would still fire and no setting of the cap would switch its   \
     rule off; with `<` the cap counts the lmr depths its rule covers and **0   \
     is an exact off value**. That is what the bisection protocol of a failing  \
     block needs (S109 section 6): one release rebuild per half, no SPRT on     \
     the tune build (S073). Each rule has a second off value in its own         \
     margin, named below.                                                       \
                                                                               \
     Under DEC-105 every default here is one of three things and says which:    \
     a value from a publication about the technique, a derivation over          \
     chesso's own data or scale, or the declared range's midpoint. No engine's  \
     shipped threshold, margin or depth cap seeds any of them, wherever it is   \
     republished (DEC-084 as amended by DEC-105, DEC-134). All of them are      \
     first settings and S127 sweeps them.                                       \
                                                                               \
     LATE MOVE PRUNING. Past a move count that grows with the reduced depth,    \
     the quiet stage is abandoned: `skip_quiets` is set and the generator is    \
     not asked for quiets at all. The count is in **hundredths of a move**, so  \
     the comparison is `100 * move_number > LMP_BASE + LMP_DEPTH_COEFF *        \
     lmr_depth`, doubled when improving -- hundredths for the reason LmrBase    \
     and LmrDivisor are hundredths, that the fit behind them is not an integer  \
     and rounding at the source throws away what was measured.                  \
                                                                               \
     Both are **(b) a derivation over chesso's own tree**, the census in        \
     adocs/data/S109_lmp_census.py over the 300-position stratified pick of     \
     adocs/data/S018_raw.tsv at depth 10 with all four caps off: one row per    \
     remaining depth, the 95th percentile of the index of the move that caused  \
     a quiet beta cutoff -- the count past which 19 of 20 quiet cutoffs have    \
     already happened, which is what "late enough to skip" means -- fitted by   \
     least squares over remaining depths 1 to 3, each row placed at the lmr     \
     depth the rule would test that count at. 904472 cutoffs.                   \
     adocs/data/S109_lmp_census.tsv is the census, _fit.txt the reading.        \
                                                                               \
     **The census does not support a count that grows with depth, and the       \
     coefficient therefore ships at its floor.** The 95th percentile *falls*    \
     with remaining depth -- 8 8 6 5 6 6 4 5 3 3 over depths 1 to 10 -- and the \
     unconstrained line is `10.00 - 2.00 * lmr_depth`. It is not an artefact of \
     the axis: the fall is there on the remaining-depth axis too, and at the    \
     99th percentile, and over depths 1 to 8. A deeper node in this tree cuts   \
     off earlier in the move order, because its table move and its killers are  \
     better. LMP_DEPTH_COEFF's range is non-negative by **stated purpose** and  \
     not by a guess at where the good values are: a negative coefficient prunes \
     harder the deeper the node, which inverts the mechanism rather than tuning \
     it. Fitted subject to that bound the line is flat at 7.33 moves. S127      \
     sweeps both axes and the quadratic form is re-tried there.                 \
                                                                               \
     LMP_MAX_LMRDEPTH was to be **(b)** from the same census -- the largest lmr \
     depth at which the fitted threshold still sits below the median number of  \
     quiet moves a node generates, 25, because a threshold past the end of the  \
     move list never binds and cannot be measured. A flat 7.33 is below 25 at   \
     every depth in range, so the criterion discriminates nothing and does not  \
     get to set the cap by accident: it takes **(c) the declared midpoint** of  \
     0 to 16 instead, which is what the other three caps are.                   \
                                                                               \
     Off: 0, or LMP_BASE at 27000 -- MAX_MOVES in hundredths, a threshold no    \
     move list reaches. */                                                      \
  X(LMP_BASE,          "LmpBase",         733,    0, 27000)                    \
  X(LMP_DEPTH_COEFF,   "LmpDepthCoeff",   0,      0, 27000)                    \
  X(LMP_MAX_LMRDEPTH,  "LmpMaxLmrDepth",  8,      0, 16)                       \
                                                                               \
  /* FUTILITY. A quiet whose node's static score plus a margin still does not   \
     reach alpha is skipped, with the remaining quiets, at a shallow reduced    \
     depth: `static + FUT_BASE + FUT_SLOPE * lmr_depth <= alpha`.               \
                                                                               \
     FUT_BASE and FUT_SLOPE are **(a) literature**, Heinz's margins as the      \
     wiki states them -- https://www.chessprogramming.org/Futility_Pruning,     \
     fetched 2026-09-05: the depth-1 margin "should not exceed the value of a   \
     minor piece" and the depth-2 margin is "more like the value of a rook".    \
     Read in **chesso's own material scale**, src/eval_tables.hpp, where a      \
     minor is `(KNIGHT + BISHOP) / 2` = 317 (the integer below 317.5) and a     \
     rook is 487: FUT_SLOPE = 487 - 317 = 170 and FUT_BASE = 317 - 170 = 147.   \
     The scale matters and an earlier pass got it wrong -- a margin compared    \
     against `evaluate()` is in `piece_value`'s units, where a pawn is 94, and  \
     not in `see_value`'s, where it is 100.                                     \
                                                                               \
     FUT_MAX_LMRDEPTH is **(c) the midpoint** of a range declared by purpose,   \
     0 to 16: 0 is off and 16 is above the median depth 11 that RfpMaxDepth's   \
     comment records, where the gate stops binding at all. Off: 0, or FUT_BASE  \
     at 48000, past every non-mate alpha the guard admits. */                   \
  X(FUT_BASE,          "FutBase",         147,    0, 48000)                    \
  X(FUT_SLOPE,         "FutSlope",        170,    0, 2000)                     \
  X(FUT_MAX_LMRDEPTH,  "FutMaxLmrDepth",  8,      0, 16)                       \
                                                                               \
  /* HISTORY PRUNING. A quiet whose butterfly history sits below a margin that  \
     scales with the reduced depth is skipped: `history < -HP_COEFF *           \
     lmr_depth`. The **raw table entry** and never score_move()'s return --     \
     a killer scores 900000 there and a countermove 700000, so reading the      \
     ordering score would silently exempt both and nothing else.                \
                                                                               \
     HP_COEFF is **this project's own SPSA fit**: S222's history lane of        \
     2026-09-14, 1250 iterations over 60000 games at 2+0.02 on                  \
     books/UHO_4060_v3.epd, adocs/data/S222_spsa_trajectory.tsv, and nothing    \
     else is behind it. It was in that lane because it reads the same scale     \
     the lane moves (DEC-205) and it still reads the raw plain entry, not the   \
     sum; S098 owns moving the rule onto the sum.                               \
                                                                               \
     What it replaces was **(c) the midpoint** of a per-lmr-depth region        \
     declared on chesso's own history scale: `M/64` to `M/8` with `M` =         \
     QuietHistoryMax, that is 128 to 1024 at the 8192 that then shipped,        \
     midpoint 576. The region is declared and not taken from anywhere: below    \
     M/64 the threshold is inside the noise a single malus writes, above M/8    \
     it reaches an eighth of the whole band in one ply. The fitted 612 is       \
     still inside that region at the fitted band -- 138 to 1104 at M = 8831 --  \
     which is a check on the fit and not the derivation of the value.           \
                                                                               \
     HP_MAX_LMRDEPTH is **(c) the midpoint** of 0 to 16, declared by the same   \
     purpose as FutMaxLmrDepth. **Off is the cap at 0, and nothing else.**      \
     The threshold is `-HP_COEFF * lmr_depth`, so at lmr_depth 0 it is 0        \
     whatever HP_COEFF holds and the rule still skips every quiet whose entry   \
     is negative. 16384 is the range top -- twice the band's own edge, so no    \
     entry sits below it at any lmr_depth of 1 or more -- and a range top is    \
     not an off value. */                                                       \
  X(HP_COEFF,          "HistPruneCoeff",  612,    0, 16384)                    \
  X(HP_MAX_LMRDEPTH,   "HistPruneMaxLmrDepth", 8, 0, 16)                       \
                                                                               \
  /* QUIET SEE PRUNING. A quiet whose exchange evaluation loses more than a     \
     margin scaled by the reduced depth squared is skipped:                     \
     `!see_ge(board, move, -(SEE_QUIET_COEFF * lmr_depth * lmr_depth))`.        \
                                                                               \
     SEE_QUIET_COEFF is **(b) chesso's own exchange scale**: the bar at         \
     lmr_depth 1 must stay under one pawn of `see_value` in src/bitboard.cpp,   \
     which is 100 there and not the 94 `piece_value` uses, so the interval the  \
     purpose declares is 0 to 100 and 50 is its midpoint. Every see_value       \
     difference is a multiple of 100, so the whole interval prunes the same     \
     set at lmr_depth 1 -- quiets that lose a pawn or more -- and the choice    \
     inside it only shows up at the depths the square scales.                   \
                                                                               \
     SEE_QUIET_MAX_LMRDEPTH is **(c) the midpoint** of 0 to 16 as above.        \
     **Off is the cap at 0, and nothing else.** The bar is                      \
     `-(SEE_QUIET_COEFF * lmr_depth * lmr_depth)`, so at lmr_depth 0 it is 0    \
     whatever the coefficient holds and the rule still skips every quiet whose  \
     exchange evaluation loses material. 10000 is the range top -- larger than  \
     any see_value, so every exchange clears the bar at any lmr_depth of 1 or   \
     more -- and a range top is not an off value. The power is a parameter of   \
     the form and not of this list: linear and squared both appear in the       \
     surveyed descriptions and S127 is where the form is re-tried. */           \
  X(SEE_QUIET_COEFF,   "SeeQuietCoeff",   50,     0, 10000)                    \
  X(SEE_QUIET_MAX_LMRDEPTH, "SeeQuietMaxLmrDepth", 8, 0, 16)                   \
                                                                               \
  /* CAPTURE SEE PRUNING, S091, and the capture half of the same technique.     \
     A capture whose exchange evaluation loses more than a margin scaled by the \
     reduced depth is skipped outright, where quiescence only declines it:      \
     `!see_ge(board, move, -(SEE_CAPT_COEFF * lmr_depth))`. It reads the same   \
     guards as the four rules above -- `may_prune` in negamax_at() -- because   \
     the hazard is the same one, with the gives-check exemption asked of the    \
     engine's own is_check() for the captures this rule wants to skip and for   \
     no others.                                                                 \
                                                                               \
     **The margin is linear in the reduced depth and the quiet one is           \
     quadratic**, which is the shape the wiki publishes for the two sides --    \
     https://www.chessprogramming.org/Static_Exchange_Evaluation, "a linear     \
     depth margin for captures, and a quadratic depth margin for quiets".       \
     Form only: the wiki publishes no number and none is taken from one.        \
                                                                               \
     SEE_CAPT_COEFF is **(b) chesso's own exchange scale**, derived exactly as  \
     SeeQuietCoeff above: the bar at lmr_depth 1 stays under one pawn of        \
     `see_value` in src/bitboard.cpp, which is 100 there and not the 94         \
     `piece_value` uses, so the interval the purpose declares is 0 to 100 and   \
     50 is its midpoint. Every see_value difference is a multiple of 100, so    \
     the whole interval skips the same set at lmr_depth 1 -- captures that lose \
     a pawn or more -- and the choice inside it shows up only at the depths the \
     margin scales.                                                            \
                                                                               \
     SEE_CAPT_MAX_LMRDEPTH is **(c) the midpoint** of 0 to 16, the range the    \
     four caps above declare by the same purpose. It is on the                  \
     reduction-adjusted depth and not on the node's own remaining depth, which  \
     is a choice against this step's research note: one engine A/B'd an         \
     lmr-depth gate for SEE pruning and measured it negative. Gating every rule \
     of the block on one axis is worth more here than a second axis nothing in  \
     this tree has measured, and S127 re-tries the axis with the power.         \
     adocs/data/S091_rule_sweep.txt is what the two rules do to this tree.      \
                                                                               \
     **Off is the cap at 0, and nothing else**, for the reason the two rules    \
     above state: the bar is `-(SEE_CAPT_COEFF * lmr_depth)`, so at lmr_depth 0 \
     it is 0 whatever the coefficient holds and the rule still skips every      \
     capture that loses material. 10000 is the range top -- larger than any     \
     see_value, so every exchange clears the bar at any lmr_depth of 1 or more  \
     -- and a range top is not an off value. */                                 \
  X(SEE_CAPT_COEFF,    "SeeCaptureCoeff", 50,     0, 10000)                    \
  X(SEE_CAPT_MAX_LMRDEPTH, "SeeCaptureMaxLmrDepth", 8, 0, 16)                  \
                                                                               \
  /* THE EXTRA REDUCTION, S091. A move the exchange evaluation says loses       \
     material -- capture or quiet, the bar is 0 and not a margin -- is searched \
     SEE_LMR_EXTRA plies shallower than late move reduction alone would have    \
     searched it, inside that rule's own eligibility: past the third legal      \
     move, at depth 3 or more, never in check, never on a move that gives       \
     check, never on a promotion. A capture is still not reduced by late move   \
     reduction itself, so on a capture this ply is the whole reduction and on a \
     quiet it is one ply on top of the table's.                                 \
                                                                               \
     **(a) literature, as a form with no number in it**: CPW's late move        \
     reduction page lists "allowing reductions of 'bad' captures (SEE < 0)"     \
     under Uncommon Conditions and Leorik 2.4's release notes describe          \
     searching "moves with a bad SEE score at a reduced depth in the main       \
     search". Neither publishes a ply count, so the default is the smallest     \
     value that is not the off value -- one ply, the narrowest form of the      \
     published idea -- and not a number read anywhere.                          \
                                                                               \
     The range is 0 to 3 by **stated purpose**: 0 is off, and this is an        \
     adjustment to the reduction the ordering already chose. At 3 it already    \
     exceeds what the table returns for the first reducible move at every depth \
     this engine reaches -- `lmr_reduction(11, 4)` is 2 at the median depth 11  \
     RfpMaxDepth's comment records -- so past it the extra replaces the         \
     ordering's own estimate instead of adjusting it, which is a different      \
     rule and not a setting of this one. The reduced search keeps at least one  \
     real ply either way: negamax_at() clamps the sum to `child_depth - 1`.     \
     S127 re-tries the ply count and the bar. */                                \
  X(SEE_LMR_EXTRA,     "SeeLmrExtra",     1,      0, 3)                        \
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
