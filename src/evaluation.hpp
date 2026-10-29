#pragma once
#include <array>
#include "data_structures.hpp"
#include "search_params.hpp"  // LAZY_EVAL_MARGIN

// Positive means the side to move is better, whatever colour that is. Callers
// use the number as it comes; there is no sign to apply.
//
// The full score, every term, no shortcut. The tuner fits this one and not the
// lazy form below: since S262 through its own trace (src/eval_trace.hpp), and
// before that through the mirror in tools/eval_model.hpp.
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

// The mobility table's shape, S121: one row per piece type in knight, bishop,
// rook, queen order, one cell per count the piece can have, 0 to the most
// squares it can reach on an empty board -- 8, 13, 14 and 27, computed with
// python-chess over all 64 squares (the S121 step file, section 4) -- so the
// widths come from the rules of chess and not from any engine. Laid end to
// end, a type's cells from MOBILITY_OFFSET[type], in the packed table below,
// the trace's indices and the tuner's vector alike.
inline constexpr int MOBILITY_WIDTH[4] = {9, 14, 15, 28};
inline constexpr int MOBILITY_OFFSET[4] = {0, 9, 23, 38};
inline constexpr int MOBILITY_CELLS = 66;

static_assert(MOBILITY_OFFSET[1] == MOBILITY_OFFSET[0] + MOBILITY_WIDTH[0] &&
                  MOBILITY_OFFSET[2] ==
                      MOBILITY_OFFSET[1] + MOBILITY_WIDTH[1] &&
                  MOBILITY_OFFSET[3] ==
                      MOBILITY_OFFSET[2] + MOBILITY_WIDTH[2] &&
                  MOBILITY_CELLS == MOBILITY_OFFSET[3] + MOBILITY_WIDTH[3],
              "the mobility rows are laid end to end");

// The mobility tables, one middlegame and one endgame array per piece type,
// indexed by the count. One-dimensional so the tuner's header pastes over them
// as it pastes every other weight array. Exposed so that the tuner
// (tools/tuner_layout.hpp's starting_params()) starts from what the engine
// ships instead of from a copy of it, which could drift.
extern const int mobility_knight_mg[9];
extern const int mobility_knight_eg[9];
extern const int mobility_bishop_mg[14];
extern const int mobility_bishop_eg[14];
extern const int mobility_rook_mg[15];
extern const int mobility_rook_eg[15];
extern const int mobility_queen_mg[28];
extern const int mobility_queen_eg[28];

// S121's four parts, as adocs/data/S121_parts.patch adds them to a throwaway
// tree for the screen's builds (adocs/data/S121_screen.sh). Committed in this
// form once, for the candidate S121's SPRT plays: the mask below is the
// screen's survivors, 5 (P1 and P3), and the closing commit folds it on H1 or
// reverts it with the paste on H0. Each changes the count a piece's cell is
// read at, and nothing else -- king safety reads the raw attack set -- and
// each is a bit of S121_PARTS, set below and overridable by the build
// (`-DS121_PARTS=<mask>`):
//
//   1  P1   a square attacked by an enemy pawn is not counted
//   2  P2a  the mobility area replaces "not an own piece": an own piece's
//           square counts, unless it holds an own blocked pawn -- one whose
//           push square holds any piece. Letting own pieces count is
//           inseparable from the first own-pawn exclusion, so it is this
//           part's price
//   8  P2b  an own pawn on the side's own second or third rank is not
//           counted either; only with P2a's area, since over "not an own
//           piece" it excludes nothing
//   4  P3   a piece pinned to its own king counts only the squares on the
//           line through the king and itself
//
// Defined from the descriptions in the S121 step file (DEC-221, DEC-326 (2));
// the own king's square is not excluded. S121_COST is the cost-only form of
// the same mask (DEC-047): the parts it names are computed and handed to an
// asm barrier, and the count scored is S121_PARTS's, which must then be 0, so
// the search is node for node the landing's (adocs/data/S121_cost.sh). When
// the screen's survivors land, the macros are folded: a surviving part's code
// made unconditional, a dropped part's deleted, and INV-6 against the screened
// build proves the fold.
#ifndef S121_PARTS
#define S121_PARTS 5
#endif
#ifndef S121_COST
#define S121_COST 0
#endif

static_assert((S121_PARTS) >= 0 && (S121_PARTS) <= 15 && (S121_COST) >= 0 &&
                  (S121_COST) <= 15,
              "S121_PARTS and S121_COST are masks over the four parts");
static_assert((S121_PARTS) == 0 || (S121_COST) == 0,
              "a cost-only build scores the landing's count");
static_assert(((S121_PARTS) & 8) == 0 || ((S121_PARTS) & 2) != 0,
              "P2b excludes low-rank pawns from P2a's area and needs it");
static_assert(((S121_COST) & 8) == 0 || ((S121_COST) & 2) != 0,
              "P2b excludes low-rank pawns from P2a's area and needs it");

inline constexpr bool MOBILITY_PAWN_ATTACKS = ((S121_PARTS) & 1) != 0;
inline constexpr bool MOBILITY_AREA = ((S121_PARTS) & 2) != 0;
inline constexpr bool MOBILITY_LOW_RANK = ((S121_PARTS) & 8) != 0;
inline constexpr bool MOBILITY_PINS = ((S121_PARTS) & 4) != 0;

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
// open file, 2 rook on a half-open file, 3 bad bishop, 4 bad bishop rammed, 5
// trapped minor, 6 trapped rook. Exposed for the same reason as the weights
// above: the tuner (tools/tuner_layout.hpp) starts from what the engine ships
// instead of from a copy of it that would be free to drift.
//
// **Fitted since S135, five of the seven.** S027 fitted piece placement (mg
// {2, 32, 8, -26} / eg {55, -8, 12, 11}), its bundled SPRT read -5.48 +/-
// 11.46 and H0, and the weights were zeroed by hand and held there by
// `--freeze piece_placement` in every fit from S065 to S083 (DEC-057). S135's
// screen kept the bishop pair, both rook files and the two trapped counts, and
// their joint fit ships on its SPRT's H1 (DEC-259, DEC-315); the bad bishop
// and its rammed share stay at zero, dropped by the screen. evaluation.cpp
// says so beside the weights, and adocs/plan_done/S100_*.md carries the
// ledger. The passed pawn, pawn structure and king safety weights above and
// below ship fitted since S027; tempo is zero for its own reason, stated at
// its block.
//
// The seven definitions, stated exactly, because the tuner's model is written
// from this comment and the two have to mean the same thing: the first three
// here, the four S135 added further down. Each is about one side's own pieces;
// the term is the White count minus the Black one, feature by feature.
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
//
// **The four S135 added (DEC-258)**, from CPW *Bad Bishop* ("a bishop whose
// mobility is restricted by own pawns", a situation "semi-permanent (more so
// if the position is blocked)") and CPW *Trapped Pieces* ("the extreme
// examples of poor mobility", given as a list of examples):
//
//   bad_bishop         for each own bishop, the own pawns standing on squares
//                      of that bishop's colour, summed over the side's bishops.
//                      A count: two bishops on one colour each count that
//                      colour's pawns, and a side with no bishop scores 0
//                      whatever its pawns. Where the pawns stand otherwise --
//                      in front of the bishop or behind it, inside a chain or
//                      not -- is not asked.
//   bad_bishop_rammed  the same sum over the rammed own pawns alone: a pawn is
//                      rammed when the square directly ahead of it holds an
//                      enemy pawn. Only a pawn rams; a piece in front does not.
//                      Every pawn counted here is also counted in bad_bishop,
//                      so the two overlap by construction and the fit prices
//                      the rammed share on top of the plain one.
//   trapped_minor      own knights and bishops standing on one of the patterns
//                      below, each piece counted once however many it meets.
//                      White's, with Black's pawns; the second of each pair is
//                      the first mirrored across the d and e files:
//                        bishop on h7, pawns on f7 and g6 / a7, c7 and b6
//                        bishop on h6, pawns on g5 and f6 / a6, b5 and c6
//                        knight on h8, a pawn on h7 or f7 / a8, a7 or c7
//                        knight on h7, pawns on g7 and either h6 or f6 /
//                          a7, b7 and either a6 or c6
//                      Black's are White's with every rank mirrored, rank r to
//                      9 - r, and White's pawns: a bishop on h2 with pawns on
//                      f2 and g3, and so on. Nothing else about the piece is
//                      asked -- whether it is attacked, or has a move.
//   trapped_rook       own rooks on h1, g1, h2 or g2 while the own king stands
//                      on f1 or g1, and own rooks on a1, b1, a2 or b2 while it
//                      stands on c1 or b1; for Black the ranks are mirrored, h8
//                      to b7 with the king on f8, g8, c8 or b8. A count: two
//                      rooks in the corner count two.
//
// None of the four is exclusive of the features above or of each other. Rank
// and file are a8 = index 0 throughout, as the rest of this file.
extern const int piece_placement_mg[7];
extern const int piece_placement_eg[7];

// The raw counts the term is built from, per colour, WHITE first, in the
// feature order above. Exists for tests/test_eval_model and per colour rather
// than as the difference, for the reasons written out over
// passed_pawn_counts(). Both of them apply here at full strength: the two bad
// bishop columns still weigh zero, so no score sees those counts at all, and a
// miscount that hits both sides equally stays invisible in a difference for
// the five that are fitted.
//
// Not on the hot path: it is the collecting instantiation of the same function
// evaluate_cheap() calls, so there is one extraction and no second
// implementation to drift.
void piece_placement_counts(const board_t* board, int out[2][7]);

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
// shortcut's soundness argument is unchanged by the move. S039 measured
// retiring the shortcut and this clamp together and read H0 (record ca578de,
// revert 74d0adf): both are kept, and this number with them.
//
// It bounds the sum of every expensive term, not each one, so king safety
// shares the same budget -- and it has shipped at fitted, non-zero weights
// since S027, so the mobility figures above are no longer the whole correction.
// The whole correction was measured at that point: tools/eval_spread over the
// same 1490839 positions put the combined number at p50 27, p95 90, p99 128,
// p99.9 178 and a maximum of 330, clamping 0.365 % of them against 0.104 % with
// king safety at zero. That was taken at the margin of 150 that shipped then,
// so on that corpus the share clamped at 184 is lower and has not been
// re-measured. tools/eval_spread measures the spread and, since S288, the
// share clamped at the shipped margin first among its candidates; the tuner's
// load report counts the same share ("the clamp binds on"), which at S262
// read 36503 of the 10795695 rows of selfplay_v2_dedup.tsv, 0.338 %.
//
// The value itself moved to src/search_params.hpp at S073, where it is one of
// the parameters the tune build exposes over UCI. What it means stays here,
// because it is a claim about the evaluation rather than about the search.

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
// above, each derived from its mg/eg pair at compile time (S117); mobility's
// is the four tables end to end, MOBILITY_OFFSET[type] + count. Declared for
// the test that compares their halves with the plain arrays: a pair packed the
// wrong way round is invisible to the bench once a refit has moved the bench
// anyway, and invisible to it now for tempo and placement's two bad bishop
// columns, whose weights are zero. The piece-square tables' copy, psqt_score,
// is in eval_tables.hpp.
extern const std::array<score_t, MOBILITY_CELLS> mobility_score;
extern const std::array<score_t, KS_FEATURE_COUNT> king_safety_score;
extern const std::array<score_t, 5> passed_pawn_score;
extern const std::array<score_t, 3> pawn_structure_score;
extern const std::array<score_t, 7> piece_placement_score;
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
