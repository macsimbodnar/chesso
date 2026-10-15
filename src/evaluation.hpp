#pragma once
#include <array>
#include "data_structures.hpp"
#include "search_params.hpp"  // CONT_HIST_WEIGHT

// Positive means the side to move is better, whatever colour that is. Callers
// use the number as it comes; there is no sign to apply.
//
// The full score, every term, and since S039 the only form there is: every
// caller gets the exact sum of both stages. Until then quiescence called a lazy
// form, evaluate_lazy(), which skipped the expensive stage where the cheap one
// already cleared the window by LAZY_EVAL_MARGIN and returned that bound in
// place of the score (S034), and evaluate_expensive() clamped mobility plus
// king safety to the same margin so the bound held by construction -- which
// also capped what the two terms could say in the real score. The shortcut and
// the clamp retired together under one non-regression SPRT (DEC-258). The tuner
// fits this function: since S262 through its own trace (src/eval_trace.hpp),
// and before that through the mirror.
int evaluate(const board_t* board);

// The terms that come straight off the accumulators make_move maintains:
// material and the tapered piece-square tables, and nothing that has to look at
// where the other pieces are standing. 1.36 ns, against 15.93 for the full
// score. Exposed for the tests that pin it and for tools/eval_spread, which
// reports how far the expensive stage moves the score away from it.
int evaluate_cheap(const board_t* board);

// The mobility weights, per piece type in knight, bishop, rook, queen order.
// Exposed so that the tuner (tools/tuner_layout.hpp's starting_params()) starts
// from what the engine ships instead of from a copy of it, which could drift.
extern const int mobility_mg[4];
extern const int mobility_eg[4];

// The passed pawn weights, indexed by how far the pawn has advanced: 0 on its
// own second rank, 4 on its own sixth. Exposed for the same reason as the
// mobility weights: the tuner (tools/tuner_layout.hpp) starts from what the
// engine ships instead of from a copy of it that would be free to drift.
//
// Fitted and shipping non-zero since S027 -- see the weights themselves and
// the fit that produced them in evaluation.cpp. They are a residual on top of
// the pawn piece-square table and not a valuation of a passed pawn, which is
// why a bucket is allowed to be negative.
//
// **A pawn on its own seventh is in no bucket.** It is a passer by definition,
// so a sixth bucket for it was exactly the signed sum of eight psqt[PAWN]
// columns; S100 read that bucket at +22, -1 and -17 across three fits, and S134
// folded its weight into psqt[PAWN][8..15] and deleted it (DEC-090). The table
// prices that pawn and this term does not see it.
//
// Buckets rather than one weight scaled by the rank because the value is not
// linear in the rank and the tuner fits only a linear function of its
// parameters -- see evaluation.cpp.
extern const int passed_pawn_mg[5];
extern const int passed_pawn_eg[5];

// The raw counts the term is built from, per colour, WHITE first. Each row is
// that side's own passed pawns in the buckets the weights above are indexed by;
// the White-minus-Black difference the term scores is one subtraction away.
//
// It exists so the engine's counts are checked on their own, not through a
// score: tests/test_eval_model holds them to the walk in
// tests/test_walked_counts.hpp, written from the specification above and
// sharing no code with the engine, and to hand-worked placements. The tuner
// fits evaluate()'s own trace since S262, so it fits whatever this counts,
// and no score says whether that is the specification as written.
//
// Per colour and not as the difference, for the same reason
// king_safety_features() is: a miscount that hits both sides equally cancels in
// the difference, so a check that sees only the difference cannot see it -- and
// that stays true at any weight, which is why the accessor outlived the zeros.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate_cheap() calls, so there is one extraction and no second
// implementation to drift.
void passed_pawn_counts(const board_t* board, int out[2][5]);

// The pawn structure weights, indexed by feature: 0 isolated, 1 doubled, 2
// backward. Exposed for the same reason as the passed pawn weights above: the
// tuner (tools/tuner_layout.hpp) starts from what the engine ships instead of
// from a copy of it that would be free to drift.
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
// a miscount that hits both sides equally cancels in a difference, so a check
// of the difference cannot see it whatever the weights are.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate_cheap() calls, so there is one extraction and no second
// implementation to drift.
void pawn_structure_counts(const board_t* board, int out[2][3]);

// The piece placement weights, indexed by feature: 0 bishop pair, 1 rook on an
// open file, 2 rook on a half-open file. Exposed for the same reason as the
// weights above: the tuner (tools/tuner_layout.hpp) starts from what the engine
// ships instead of from a copy of it that would be free to drift.
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
// The three definitions, stated exactly, because the tuner's model is written
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
//
// Counts and not booleans for the two rook features: two rooks on open files
// count two. Only rooks are asked about, so a queen on an open file is not
// counted anywhere here.
//
// rook_open and rook_half_open exclude each other.
//
// **Rook on the seventh is not here.** Until S134 it was a fourth feature, own
// rooks on the rank the enemy's pawns start on, and that is exactly the signed
// sum of eight psqt[ROOK] columns -- the rank is squares 8..15 for both colours
// once Black is mirrored -- so it could carry no weight the table cannot. Its
// zero weight was folded into psqt[ROOK][8..15] and the feature deleted
// (DEC-090). A conditional version, one that asks about the enemy king or the
// pawns still on that rank, is a different feature and would need its own
// identity check (tools/feature_audit) as well as its own fit.
//
// **A file with an own pawn on it is neither open nor half-open**, whatever
// the enemy has there. The two features are disjoint by construction and their
// sum is not "rooks on files this side's pawns do not block" -- a rook behind
// its own pawn scores zero in both, deliberately.
extern const int piece_placement_mg[3];
extern const int piece_placement_eg[3];

// The raw counts the term is built from, per colour, WHITE first, in the
// feature order above. Exists for tests/test_eval_model and per colour rather
// than as the difference, for the reasons written out over
// passed_pawn_counts(). Both of them apply here at full strength: these weights
// are still zero, so no score sees these counts at all, and a miscount that
// hits both sides equally would stay invisible in a difference after they are
// fitted.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate_cheap() calls, so there is one extraction and no second
// implementation to drift.
void piece_placement_counts(const board_t* board, int out[2][3]);

// The tempo weights: a bonus for having the move, middlegame and endgame,
// tapered like every other term. Exposed for the same reason as the weights
// above: the tuner (tools/tuner_layout.hpp) starts from what the engine ships
// instead of from a copy of it that would be free to drift.
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
// as the mobility weights: the tuner (tools/tuner_layout.hpp) starts from what
// the engine ships instead of from a copy of it that can drift.
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
// It exists so that each of the nine counts is checked on its own:
// tests/test_eval_model holds them to the walk in tests/test_walked_counts.hpp,
// which shares no code with the engine. A score cannot: it is one weighted
// White-minus-Black sum over nine collinear counts, so a miscount in one
// feature offset by another, or one that hits both sides equally, cancels
// before it reaches the number.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate() calls, so there is one extraction and no second implementation to
// drift.
void king_safety_features(const board_t* board, int out[2][KS_FEATURE_COUNT]);

// The two expensive terms, in the orientation evaluate() applies them in: their
// sum is exactly what evaluate() adds to evaluate_cheap(), which
// test_evaluation "the expensive stage is applied whole" asserts on every test
// position.
//
// Written so the number the clamp truncated could be seen from outside -- the
// clamp lived inside evaluate_expensive() until S039 retired it -- and kept for
// tools/eval_spread, which reads a position corpus through it. Split into two
// terms because mobility and king safety are fitted and rebuilt by different
// steps, and a large correction from one, the other or both at once are three
// different answers.
//
// Off the hot path by construction rather than by measurement: it goes through
// the same collecting instantiation king_safety_features() uses, so the
// instantiation the search compiles keeps exactly one caller. Handing that one
// a second caller is what cost 21 % once -- see king_shelter_features() in
// evaluation.cpp.
void evaluate_expensive_terms(const board_t* board, int* mobility, int* safety);

// How far into the game the position is: 24 with a full set of pieces, 0 once
// only kings and pawns remain. Tapered terms interpolate on it. Pawns and kings
// do not count toward it, and promotions cannot push it above 24.
int game_phase(const board_t* board);

#define GAME_PHASE_MAX 24

// The interpolation every tapered term goes through: the packed score's two
// halves weighted by the phase, then one division truncating towards zero. The
// halves come apart first because a packed score cannot be divided (S117).
// Each call is one truncation, so a site that tapers two terms separately
// truncates twice -- stage two does, deliberately, see
// evaluate_mobility_and_king_safety().
inline int taper(score_t score, int phase)
{
  return (mg_value(score) * phase +
          eg_value(score) * (GAME_PHASE_MAX - phase)) /
         GAME_PHASE_MAX;
}

// The packed copies the evaluation reads in place of the plain weight arrays
// above, each derived from its mg/eg pair at compile time (S117). Declared for
// the test that compares their halves with the plain arrays: a pair packed the
// wrong way round is invisible to the bench once a refit has moved the bench
// anyway, and invisible to it now for placement and tempo, whose weights are
// all zero. The piece-square tables' copy, psqt_score, is in eval_tables.hpp.
extern const std::array<score_t, 4> mobility_score;
extern const std::array<score_t, KS_FEATURE_COUNT> king_safety_score;
extern const std::array<score_t, 5> passed_pawn_score;
extern const std::array<score_t, 3> pawn_structure_score;
extern const std::array<score_t, 3> piece_placement_score;
extern const score_t tempo_score;
int capture_score(const board_t* board, move_t move);

// The raw signed history a quiet move carries -- the butterfly entry plus
// S222's weighted continuation entry -- and the one path a consumer reads it
// through. Raw, never score_move's return: a killer scores 900000 there and a
// countermove 700000, and a consumer that divides one of those by a history
// divisor is reading a band constant, which is S093's own hazard. Pre-make,
// like every caller -- `active_color` indexes the table with the side that
// plays the move.
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
// no LTO in this build -- S270 measured g++ 13's 1.6 % slower here, and left it
// off -- so a definition in evaluation.cpp is a call per quiet scored: ten
// interleaved bench pairs read 3815942 nodes per second against 3632322 with
// it out of line, -4.81 %, the two groups not overlapping. Inline
// here the arithmetic folds into its call site. Behaviour-neutral by
// construction and proved by the signature: the bench total is the same number
// either way, and the removal above is what proves it again -- this tree's
// total is the reference's exactly.
inline int quiet_history_sum(const game_t* game,
                             const search_state_t* state,
                             move_t move,
                             move_t prev_move)
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
  // against plain history's own. Summed in `int`, which the sum needs: two
  // int16_t entries at their bounds already exceed int16_t, and the weight
  // multiplies one of them by up to twenty.
  //
  // Guarded on there being a previous move to index at all -- the root ply and
  // the node right after a null move pass 0, and 0 is the (W_PAWN, a8) cell
  // and not an absent one.
  //
  // With both terms at their extremes the band this returns widens to
  // [-(QuietHistoryMax + ContHistWeight * CONT_HIST_BOUND / 100), +the same],
  // and the clearance against the countermove band above is asserted at both
  // declared maxima in tests/test_evaluation.cpp "the declared history ceiling
  // clears the band above it" -- the one-way door CLAUDE.md names, whose
  // symptom would be lost rating and not a wrong node count.
  if (prev_move != 0) {
    score +=
        (CONT_HIST_WEIGHT * continuation_entry(state, prev_move, move)) / 100;
  }

  return score;
}


int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move);
