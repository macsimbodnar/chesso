#pragma once
#include "data_structures.hpp"
#include "search_params.hpp"  // LAZY_EVAL_MARGIN

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
// What is returned in that case is the guaranteed bound and not the cheap
// score: cheap - LAZY_EVAL_MARGIN at beta, cheap + LAZY_EVAL_MARGIN at alpha.
// Both are still on the caller's side of the window, so the cutoff is the same
// one it would have taken, and both are true statements about the real score.
//
// It returned the cheap score until S027 and that was wrong. quiesce() is fail
// soft: it returns stand_pat to the parent, which reads it as a lower bound and
// may store it. cheap is not a lower bound on anything -- the guarantee is only
// that the true score is within a margin of it, so cheap can be up to
// LAZY_EVAL_MARGIN better than the truth. Over the 2696 test positions the old
// form claimed a bound it did not have on 2613 of them.
//
// `exact`, when a caller passes one, says which of the two it got: true for the
// score, false for a bound the shortcut returned in its place. A caller that
// only compares the number against its own window does not care -- both answers
// decide the same way, which is the whole argument above. A caller that wants
// to *keep* the number does: a bound is a true statement only on the side of
// the window it was taken at, and anywhere it is read back from, the window
// will be a different one. S094.
int evaluate_lazy(const board_t* board,
                  int alpha,
                  int beta,
                  bool* exact = nullptr);

// The mobility weights, per piece type in knight, bishop, rook, queen order.
// Exposed so that the tuner's model in tools/eval_model.hpp can start from what
// the engine ships instead of from a copy of it, which would be free to drift.
extern const int mobility_mg[4];
extern const int mobility_eg[4];

// The passed pawn weights, indexed by how far the pawn has advanced: 0 on its
// own second rank, 5 on the seventh and one square from promotion. Exposed for
// the same reason as the mobility weights: the tuner's model in
// tools/eval_model.hpp starts from what the engine ships instead of from a copy
// of it that would be free to drift.
//
// Fitted and shipping non-zero since S027 -- see the weights themselves and
// the fit that produced them in evaluation.cpp. They are a residual on top of
// the pawn piece-square table and not a valuation of a passed pawn, which is
// why a bucket is allowed to be negative. The last bucket is not "nearly
// nothing": S100 read it at +22, -1 and -17 across three fits, a 39-point
// spread that changes sign twice, so it is a direction the fits do not agree
// on -- S134 folds it into the tables for that reason.
//
// Six buckets rather than one weight scaled by the rank because the value is
// not linear in the rank and the tuner fits only a linear function of its
// parameters -- see evaluation.cpp.
extern const int passed_pawn_mg[6];
extern const int passed_pawn_eg[6];

// The raw counts the term is built from, per colour, WHITE first. Each row is
// that side's own passed pawns in the buckets the weights above are indexed by;
// the White-minus-Black difference the term scores is one subtraction away.
//
// It exists for tests/test_eval_model. The weights ship non-zero, so comparing
// the engine's score against the tuner's model is a real comparison of this
// term today -- it was not while they were zero, where 0 against 0 passed
// whatever the two believed about the buckets -- but it is a comparison of the
// weighted White-minus-Black difference and nothing finer. The model has a
// test of its own against hand-computed positions, which says it matches the
// written specification; nothing said the engine reads that specification the
// same way, and a tuner fitted against a model the engine disagrees with fits
// the wrong function.
//
// Per colour and not as the difference, for the same reason
// king_safety_features() is: a miscount that hits both sides equally cancels in
// the difference, so a check that sees only the difference cannot see it -- and
// that stays true at any weight, which is why the accessor outlived the zeros.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate_cheap() calls, so there is one extraction and no second
// implementation to drift.
void passed_pawn_counts(const board_t* board, int out[2][6]);

// The pawn structure weights, indexed by feature: 0 isolated, 1 doubled, 2
// backward. Exposed for the same reason as the passed pawn weights above: the
// tuner's model in tools/eval_model.hpp starts from what the engine ships
// instead of from a copy of it that would be free to drift.
//
// Fitted and shipping non-zero since S027 -- see evaluation.cpp. A residual on
// top of the pawn piece-square table like the passed pawn buckets above, which
// is why a sign here is not a chess statement.
//
// The three definitions, stated exactly, because the tuner's model is written
// from this comment and the two have to mean the same thing. Each is about one
// side's own pawns; the term is the White count minus the Black one, feature by
// feature. "Ahead" and "behind" are that side's own directions, so ahead is
// toward rank 8 for White and toward rank 1 for Black.
//
//   isolated  own pawns with no own pawn anywhere on either neighbouring file.
//             Rank does not enter it.
//   doubled   own pawns with another own pawn ahead of them on the same file.
//             Per file that is one less than the number of own pawns on it, so
//             three pawns on a file count two.
//   backward  own pawns meeting both of: no own pawn on either neighbouring
//             file at or behind their own rank -- the same rank counts as
//             behind, so a neighbour abreast of the pawn stops it being
//             backward -- and the square directly ahead of it is attacked by an
//             enemy pawn. Whether that square is occupied is not asked, and
//             neither is whether the pawn could be defended after advancing.
//
// Counts and not booleans: three isolated pawns count three. The three features
// are not exclusive and are not meant to be -- an isolated pawn whose stop
// square an enemy pawn attacks is counted as isolated and again as backward,
// and the fit splits the shared effect between them the way DEC-044 describes
// for king safety's collinear counts.
extern const int pawn_structure_mg[3];
extern const int pawn_structure_eg[3];

// The raw counts the term is built from, per colour, WHITE first, in the
// feature order above. Exists for tests/test_eval_model and per colour rather
// than as the difference, for the reason written out over passed_pawn_counts():
// a miscount that hits both sides equally cancels in a difference, so the score
// comparison cannot see it whatever the weights are.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate_cheap() calls, so there is one extraction and no second
// implementation to drift.
void pawn_structure_counts(const board_t* board, int out[2][3]);

// The piece placement weights, indexed by feature: 0 bishop pair, 1 rook on an
// open file, 2 rook on a half-open file, 3 rook on the seventh. Exposed for the
// same reason as the weights above: the tuner's model in tools/eval_model.hpp
// starts from what the engine ships instead of from a copy of it that would be
// free to drift.
//
// **Zero in evaluation.cpp, and not because the tuner has not reached them.**
// S027 fitted piece placement (mg {2, 32, 8, -26} / eg {55, -8, 12, 11}),
// its bundled SPRT read -5.48 +/- 11.46 and H0, and the weights were then
// zeroed by hand and held there by `--freeze piece_placement` in every fit
// since S065 (DEC-057) -- held at zero, not fitted to it; evaluation.cpp says
// so beside the weights and adocs/plan_done/S100_*.md carries the ledger. The
// passed pawn, pawn structure and king safety weights above and below ship
// fitted since S027; tempo is zero for its own reason, stated at its block.
//
// The four definitions, stated exactly, because the tuner's model is written
// from this comment and the two have to mean the same thing. Each is about one
// side's own pieces; the term is the White count minus the Black one, feature
// by feature.
//
//   bishop_pair     1 when the side has two or more bishops, 0 otherwise. Not a
//                   count of bishops: it is the pair that is claimed to be
//                   worth something, so three bishops off a promotion still
//                   score 1. Square colour is not asked about -- two bishops on
//                   the same colour count as a pair here.
//   rook_open       own rooks standing on a file carrying no pawn of either
//                   colour. The rook's own rank does not enter it, and neither
//                   does anything else standing on the file: pieces are not
//                   pawns and do not close a file.
//   rook_half_open  own rooks standing on a file carrying no own pawn and at
//                   least one enemy pawn.
//   rook_seventh    own rooks standing on the rank the enemy's pawns start on
//                   -- rank 7 for White, rank 2 for Black.
//
// Counts and not booleans for the three rook features: two rooks on open files
// count two. Only rooks are asked about, so a queen on an open file is not
// counted anywhere here.
//
// rook_open and rook_half_open exclude each other, rook_seventh excludes
// neither: a rook on an open file on the seventh rank counts once in each of
// the two, and the fit splits the shared effect between them the way DEC-044
// describes for king safety's collinear counts.
//
// Two places a reader could reasonably differ, so the choice is stated rather
// than left to be inferred:
//
//   **rook_seventh asks nothing about the enemy king.** The literature usually
//   pays for a rook on the seventh only when the enemy king is on its back rank
//   or pawns are still there to be eaten. This is the rank and nothing else.
//   The condition is what the weight is fitted against, so a conditional
//   version is a different feature and would need its own fit, not a different
//   reading of this one.
//
//   **A file with an own pawn on it is neither open nor half-open**, whatever
//   the enemy has there. The two features are disjoint by construction and
//   their sum is not "rooks on files this side's pawns do not block" -- a rook
//   behind its own pawn scores zero in both, deliberately.
extern const int piece_placement_mg[4];
extern const int piece_placement_eg[4];

// The raw counts the term is built from, per colour, WHITE first, in the
// feature order above. Exists for tests/test_eval_model and per colour rather
// than as the difference, for the reasons written out over
// passed_pawn_counts(). Both of them apply here at full strength: these weights
// are still zero, so the score comparison is 0 against 0 and sees nothing at
// all, and a miscount that hits both sides equally would stay invisible in a
// difference after they are fitted.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate_cheap() calls, so there is one extraction and no second
// implementation to drift.
void piece_placement_counts(const board_t* board, int out[2][4]);

// The tempo weights: a bonus for having the move, middlegame and endgame,
// tapered like every other term. Exposed for the same reason as the weights
// above: the tuner's model in tools/eval_model.hpp starts from what the engine
// ships instead of from a copy of it that would be free to drift.
//
// **Zero, and not because the tuner has not reached them.** It has: `--only
// tempo` fitted mg 10, eg 0 at S027 and the SPRT that followed reached neither
// bound over 3000 games. The pair was put back to zero and the plumbing kept,
// so what ships is a measured "smaller than this instrument can see" and not a
// term still waiting for its first fit -- evaluation.cpp has the run and what
// S100 added to it.
//
// **The only term here that is not a function of the board**, which is what
// makes its placement the whole definition. evaluate() answers from the side to
// move's point of view and evaluate_cheap() resolves that sign by negating a
// White-relative sum; the bonus is added *after* that negation, so it pays
// whichever side is on move. Added before it, it would pay White -- a different
// term that agrees with this one on every position where White happens to be to
// move, and disagrees on every other.
//
// It has no count to expose and no per-colour row, unlike the four terms above.
// The feature is the same 1 for whoever is to move, so there is nothing an
// accessor could report that the side to move does not already say. What checks
// it instead is that the score of a position and the score of the same position
// with the other side to move sum to twice this bonus -- test_eval_model, "the
// move is worth the same to either side".
extern const int tempo_mg;
extern const int tempo_eg;

// What the king safety term counts, in the order its weights are indexed. The
// first four are attacker counts by piece type and share the order the mobility
// weights use.
enum king_safety_feature_t
{
  KS_KNIGHT_ATTACKERS,
  KS_BISHOP_ATTACKERS,
  KS_ROOK_ATTACKERS,
  KS_QUEEN_ATTACKERS,
  KS_ZONE_ATTACKS,
  KS_SHIELD_NEAR,
  KS_SHIELD_FAR,
  KS_OPEN_FILE,
  KS_HALF_OPEN_FILE,
  KS_FEATURE_COUNT
};

// The king safety weights, one per feature above. Exposed for the same reason
// as the mobility weights: the tuner's model in tools/eval_model.hpp starts
// from what the engine ships instead of from a copy of it that can drift.
//
// Fitted and shipping non-zero since S027 -- see evaluation.cpp for the fit,
// for why the model is linear, and for why a sign here is not a chess
// statement: the four attacker counts and the incidence count are collinear by
// construction, so what was fitted is their sum (DEC-044).
extern const int king_safety_mg[KS_FEATURE_COUNT];
extern const int king_safety_eg[KS_FEATURE_COUNT];

// The raw counts the term is built from, per colour, WHITE first. Each side's
// row is about its own king: the attackers are the other side's pieces bearing
// on it, the shelter is its own pawns in front of it.
//
// It exists for tests/test_eval_model. The weights ship non-zero, so the score
// comparison does now exercise this term -- while they were zero it compared 0
// against 0 and would have passed just as happily if the two disagreed about
// every one of the nine counts. What it still cannot see is the shape of the
// disagreement: the score is one weighted White-minus-Black sum over nine
// collinear counts, so a miscount in one feature offset by another, or one that
// hits both sides equally, cancels before it reaches the number. The counts are
// exposed so that each is checked on its own.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate() calls, so there is one extraction and no second implementation to
// drift.
void king_safety_features(const board_t* board, int out[2][KS_FEATURE_COUNT]);

// The two expensive terms before the clamp, in the orientation the correction
// is applied in: clamping their sum to the margin below reproduces exactly what
// evaluate() adds to evaluate_cheap().
//
// It exists because the clamp lives inside evaluate_expensive() and nothing
// outside that file can see the number it truncated, so the margin cannot be
// re-decided from data without this. tools/eval_spread reads a position corpus
// through it. Split into two terms because a large correction coming from
// mobility, from king safety or from both at once are three different answers.
//
// Off the hot path by construction rather than by measurement: it goes through
// the same collecting instantiation king_safety_features() uses, so the
// instantiation the search compiles keeps exactly one caller. Handing that one
// a second caller is what cost 21 % once -- see king_shelter_features() in
// evaluation.cpp.
void evaluate_expensive_terms(const board_t* board, int* mobility, int* safety);

// The largest correction the expensive terms are allowed to apply. The shortcut
// is only sound while that holds, so it is asserted over a position list rather
// than assumed -- see test_evaluation "the lazy shortcut cannot change a
// decision".
//
// 184 since S085's SPSA run raised it from 150. Both are above the largest
// correction observed over 149084 positions of S028 self-play, where the
// tapered mobility term ran p50 19, p95 60, p99 81, p99.9 107 and a maximum of
// 143 centipawns -- so the clamp still never binds on that corpus and the
// shortcut's soundness argument is unchanged by the move. S039 re-decides this
// number from measured spread and should expect the moved incumbent.
//
// It bounds the sum of every expensive term, not each one, so king safety
// shares the same budget -- and it has shipped at fitted, non-zero weights
// since S027, so the mobility figures above are no longer the whole correction.
// The whole correction was measured at that point: tools/eval_spread over the
// same 1490839 positions put the combined number at p50 27, p95 90, p99 128,
// p99.9 178 and a maximum of 330, clamping 0.365 % of them against 0.104 % with
// king safety at zero. That was taken at the margin of 150 that shipped then,
// so the clamped share at 184 is lower and has not been re-measured. S039
// re-decides the margin from measured data, and tools/eval_spread is what
// measures it.
//
// The value itself moved to src/search_params.hpp at S073, where it is one of
// the parameters the tune build exposes over UCI. What it means stays here,
// because it is a claim about the evaluation rather than about the search.

// How far into the game the position is: 24 with a full set of pieces, 0 once
// only kings and pawns remain. Tapered terms interpolate on it. Pawns and kings
// do not count toward it, and promotions cannot push it above 24.
int game_phase(const board_t* board);

#define GAME_PHASE_MAX 24
int capture_score(const board_t* board, move_t move);

// The raw signed history a quiet move carries -- the butterfly entry plus
// S222's weighted one-ply continuation entry plus S231's weighted two-ply one
// -- and the one path a consumer reads it through. Raw, never score_move's
// return: a killer scores 900000 there and a countermove 700000, and a consumer
// that divides one of those by a history divisor is reading a band constant,
// which is S093's own hazard. Pre-make, like every caller -- `active_color`
// indexes the table with the side that plays the move.
//
// **One production reader today, score_move's quiet band below** (a test case
// reads it too). It was factored out for
// S098's history-scaled reduction, which read the same number so that the
// ordering and the reduction could not disagree about it; that term measured
// zero at three scales and left the tree (DEC-213). What is left is the
// factoring, kept deliberately: it is behaviour-neutral, and a rule that wants
// a move's history -- S127's refits, a later reduction or pruning term --
// reads this rather than writing a second copy of the arithmetic that would be
// free to drift, the way continuation_entry is one index so a write and a read
// cannot disagree.
//
// **Defined here rather than in the .cpp, and that is a measurement.** There is
// no LTO in this build, so a definition in evaluation.cpp is a call per quiet
// scored: ten interleaved bench pairs read 3815942 nodes per second against
// 3632322 with it out of line, -4.81 %, the two groups not overlapping. Inline
// here the arithmetic folds into its call site. Behaviour-neutral by
// construction and proved by the signature: the bench total is the same number
// either way, and the removal above is what proves it again -- this tree's
// total is the reference's exactly.
inline int quiet_history_sum(const game_t* game,
                             const search_state_t* state,
                             move_t move,
                             move_t prev_move,
                             move_t prev_move2)
{
  // Signed since S093: a quiet that was tried and did not cut off carries a
  // malus, so this band runs [-QuietHistoryMax, +QuietHistoryMax] rather than
  // from zero. Gravity bounds it on the way in, so it can never reach the
  // countermove band above, and nothing sits below it -- both edges are
  // asserted in tests/test_evaluation.cpp.
  int score = state->quiet_history[game->board.active_color][MOVE_FROM(move)]
                                  [MOVE_TO(move)];

  // S222. The one-ply continuation term, on a scale of its own: its entry is
  // bounded by CONT_HIST_BOUND rather than by QuietHistoryMax, and
  // ContHistWeight is what decides how much of the quiet band it may span
  // against plain history's own.
  //
  // Guarded on there being a previous move to index at all -- the root ply and
  // the node right after a null move pass 0, and 0 is the (W_PAWN, a8) cell
  // and not an absent one.
  if (prev_move != 0) {
    score +=
        (CONT_HIST_WEIGHT * continuation_entry(state, prev_move, move)) / 100;
  }

  // S231. The two-ply term beside it, on a weight of its own, guarded on its
  // own key: ply 0, ply 1 and the node two plies after a null move all pass 0
  // here, and 0 is a real cell for this index too.
  if (prev_move2 != 0) {
    score +=
        (CONT_HIST2_WEIGHT * continuation2_entry(state, prev_move2, move)) /
        100;
  }

  // THE HEADROOM, three terms and two multiplying weights. Summed in `int`,
  // which the sum needs and which int16_t could not carry: the plain entry
  // reaches QuietHistoryMax's declared 32767 and each weighted entry reaches
  // its weight's declared 1000 per cent of CONT_HIST_BOUND, so the widest this
  // returns is 32767 + 327670 + 327670 = 688107 -- twenty-one times the type
  // the entries are stored in, and four orders inside int32.
  //
  // That number is also the one-way door CLAUDE.md names: with all three terms
  // at their extremes the band this returns is [-688107, +688107] and it has
  // to stand 100 clear of the countermove band at 700000. Asserted at every
  // declared maximum at once, not argued, in tests/test_evaluation.cpp "the
  // declared history ceiling clears the band above it"; the symptom of getting
  // it wrong is lost rating and not a wrong node count. The two weights'
  // ceilings are halves of what one weight alone could declare for exactly
  // this reason -- src/search_params.hpp's ContHistWeight block has the
  // arithmetic.
  return score;
}


int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move,
               move_t prev_move2);
