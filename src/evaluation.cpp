#include "evaluation.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "eval_tables.hpp"
#include "eval_trace.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"

// clang-format off

// Move-ordering piece values. These are deliberately not the evaluation's
// piece_value[] and they carry their own names so that the two cannot be
// confused or accidentally unified. They used to be spelled PAWN..QUEEN, the
// same names eval_tables.hpp defines, and only compiled because both sides
// happened to hold the same numbers.
//
// Ordering needs a stable ranking, not an accurate price, and the bands below
// clear each other by exactly 100 points: a king capturing a pawn scores
// ORDER_CAPTURE + MVV_PAWN - MVV_KING = 900100, against 900000 for a killer.
// Feed a fitted queen value of 1026 into that and the arithmetic still holds,
// but nothing in the tree would say so out loud if it stopped holding. The
// symptom of getting it wrong is a strength regression, not a wrong node count.
#define MVV_PAWN   100
#define MVV_KNIGHT 300
#define MVV_BISHOP 300
#define MVV_ROOK   500
#define MVV_QUEEN  900
#define MVV_KING   100000

#define ORDER_TT_MOVE 2000000
#define ORDER_CAPTURE 1000000
#define ORDER_KILLER_0 900000
#define ORDER_KILLER_1 800000
#define ORDER_COUNTER 700000

/* board representation */

static constexpr int piece_values_abs[] = {
    MVV_PAWN,  MVV_KNIGHT, MVV_BISHOP, MVV_ROOK, MVV_QUEEN, MVV_KING, MVV_PAWN,
    MVV_KNIGHT, MVV_BISHOP, MVV_ROOK,  MVV_QUEEN, MVV_KING, 0};


// clang-format on

// Every weight pair below is also kept packed (score_t, data_structures.hpp),
// S117, and the evaluation reads the packed copy: one load and one add per
// feature where there were two. The plain mg/eg arrays stay the source of
// truth because tools/tuner writes them in that form and tools/tuner_layout.hpp
// reads them; the packed copy is derived here at compile time, which is why
// they are constexpr and not merely const -- a const array's elements cannot
// be read in a constant expression.
template <size_t N>
static constexpr std::array<score_t, N> pack_scores(const int (&mg)[N],
                                                    const int (&eg)[N])
{
  std::array<score_t, N> packed{};
  for (size_t i = 0; i < N; ++i) {
    packed[i] = make_score(mg[i], eg[i]);
  }
  return packed;
}


// The largest magnitude in either half of a weight table, for the headroom
// checks below.
template <size_t N>
static constexpr int largest_weight(const int (&mg)[N], const int (&eg)[N])
{
  int largest = 0;
  for (size_t i = 0; i < N; ++i) {
    largest = std::max(
        {largest, (mg[i] < 0) ? -mg[i] : mg[i], (eg[i] < 0) ? -eg[i] : eg[i]});
  }
  return largest;
}

// Fitted with the other 799 constants frozen -- `tuner --only passed_pawns`
// over 1490839 self-play positions, held-out error 0.107106 to 0.106900.
//
// **These are a residual, not a valuation.** A frozen fit prices only what the
// term adds on top of what is already there, and psqt_mg's pawn table already
// pays for a pawn far up the board. That is why the buckets are not monotonic.
// Reading a bucket as "a passed pawn there is worth this much" is the same
// mistake as reading a pawn's fitted value of 83 as what a pawn is worth.
//
// **There were six buckets until S134, and S100 proved the sixth was not a
// quantity at all.** A pawn on its own seventh rank is a passer by
// *definition* -- "ahead" is the enemy back rank and the extraction refuses a
// pawn standing there -- and in the mirrored indexing it always occupies one of
// squares 8..15. So that bucket *was* the signed sum of eight psqt[PAWN]
// columns: R^2 exactly 1.000000, 0 violations in 550880 non-zero corpus rows,
// and the same read out of the engine's own passed_pawn_counts()
// (adocs/data/S134_engine_identity.txt). In a joint fit the split was an
// unidentified direction: across three real fits buckets 0 to 4 moved by at
// most 4 while bucket 5 read +22, -1 and -17 -- a 39-point spread that changed
// sign twice. A synthetic-label run made it explicit: planted
// {7,14,21,33,48,66}, the joint fit returned {7,14,21,33,48,24} at train error
// exactly 0, and `--only passed_pawns` returned all six exactly.
// Knight mobility fitted to nothing against this same effect at S034, DEC-040,
// and DEC-044 records it for king safety's attacker counts.
//
// **S134 folded it.** Its weights, mg -17 and eg +42, were added to each of
// psqt_mg[PAWN][8..15] and psqt_eg[PAWN][8..15] in eval_tables.hpp and the
// bucket deleted, DEC-090. Bit-exact rather than approximately so:
// evaluate_cheap() adds the pawn terms to the piece-square pair before its one
// taper, so the weight moved between two summands of a single division. A
// pawn on its own seventh scores through the table alone now, and
// evaluate_pawns() says so where it collects the passers.
//
// The term costs 4.5 % of a depth 12 search, measured with the weights forced
// non-zero because at zero the compiler deletes it, DEC-047. Its SPRT was net
// of that: +17.34 +/- 13.51 Elo over 1584 games, H1 accepted.
constexpr int passed_pawn_mg[5] = {9, -5, -2, 21, 38};
constexpr int passed_pawn_eg[5] = {18, 24, 51, 75, 119};
constexpr std::array<score_t, 5> passed_pawn_score =
    pack_scores(passed_pawn_mg, passed_pawn_eg);


// S123's passed pawn family, three groups on top of the buckets above
// (DEC-329): A, a free and a safe increment per own rank 2 to 7; B, a slope
// per own rank 2 to 7 on each king's distance to the push square; C, a weight
// per own rank 2 to 6 for a candidate passer. evaluation.hpp defines each
// count; passer_family_side() and candidate_passers() below compute them.
//
// **Landed at zero, S135's form, fitted afterwards, and shipped on one SPRT's
// H1** (DEC-259's screen, adocs/data/S123_screen.sh). No seed: nothing here
// was taken from any engine or page, and every entry started the fit at zero
// (DEC-084). At zero a group is in no expression -- the liveness flags below
// keep a group whose weights are all zero out of the engine's
// evaluate_cheap() -- so the landing scored every position as before and
// cost nothing (INV-6, adocs/data/S123_inv6.txt).
//
// The values below are the screen's full fit with all three groups kept, ABC
// (adocs/data/S123_screen_reading.txt), pasted unchanged: monotonicity is not
// imposed, so a row that is not monotonic is the fit's, not corrected by hand.
// Their one SPRT, `{0, 5}` nElo at 8+0.08, read H1 (adocs/data/S123_sprt.log),
// so the three groups ship together and no bisection follows. Their speed
// cost, +4.96 % with every group live, measured with the weights forced
// (DEC-047, adocs/data/S123_cost_reading.txt), is inside that verdict and
// was never priced apart from it.
//
// Rank 7 enters A and B as increments and slopes and never as a base cell: a
// per-rank weight on the seventh alone would be the signed sum of eight
// psqt[PAWN] columns again, S134's bucket 5. C stops at the sixth because a
// pawn on its own seventh is always passed.
constexpr int passed_free_mg[6] = {2, 8, 2, 7, 28, 76};
constexpr int passed_free_eg[6] = {23, 3, 14, 13, 5, 6};
constexpr int passed_safe_mg[6] = {-1, 1, 2, 0, 10, 33};
constexpr int passed_safe_eg[6] = {8, 6, 10, 22, 51, 53};
constexpr int passed_king_own_mg[6] = {2, 5, 7, 3, -2, -3};
constexpr int passed_king_own_eg[6] = {-3, -5, -13, -16, -17, -19};
constexpr int passed_king_enemy_mg[6] = {-2, -7, -7, -5, 0, 2};
constexpr int passed_king_enemy_eg[6] = {-3, 3, 9, 15, 18, 23};
constexpr int candidate_passer_mg[5] = {-6, -16, 4, 10, -2};
constexpr int candidate_passer_eg[5] = {4, 8, 30, 46, 59};
constexpr std::array<score_t, 6> passed_free_score =
    pack_scores(passed_free_mg, passed_free_eg);
constexpr std::array<score_t, 6> passed_safe_score =
    pack_scores(passed_safe_mg, passed_safe_eg);
constexpr std::array<score_t, 6> passed_king_own_score =
    pack_scores(passed_king_own_mg, passed_king_own_eg);
constexpr std::array<score_t, 6> passed_king_enemy_score =
    pack_scores(passed_king_enemy_mg, passed_king_enemy_eg);
constexpr std::array<score_t, 5> candidate_passer_score =
    pack_scores(candidate_passer_mg, candidate_passer_eg);

// Whether a weight array pair has a non-zero entry: a group none of whose
// arrays does is not computed by the engine's evaluate_cheap() at all.
//
// Since S123's H1 every group is live, so the three flags below are true and
// gate nothing. They stay for the refit that zeroes a group again -- S126
// refits everything S123 landed, and a group a later screen drops is held at
// zero and recorded as a corpus finding (S126's rule). Without them such a
// group's work is left to the compiler to prove dead, and A's could not be:
// is_attacked() is an out-of-line call it cannot delete as it deletes a dead
// popcount (passer_family_side()).
template <size_t N>
static constexpr bool any_weight(const std::array<score_t, N>& packed)
{
  for (size_t i = 0; i < N; ++i) {
    if (packed[i] != 0) { return true; }
  }
  return false;
}

static constexpr bool passer_advance_live =
    any_weight(passed_free_score) || any_weight(passed_safe_score);
static constexpr bool passer_kings_live =
    any_weight(passed_king_own_score) || any_weight(passed_king_enemy_score);
static constexpr bool candidate_passer_live =
    any_weight(candidate_passer_score);

// The trace build records every coefficient whatever its weight -- the tuner
// fits the zero ones too -- so it computes every group.
#ifdef CHESSO_TRACE
static constexpr bool trace_build = true;
#else
static constexpr bool trace_build = false;
#endif


// The three pawn structure features, in the order their weights are indexed.
// Shared between the weights, the counts and the accessor so that none of the
// three can disagree with the others about which index means what.
enum
{
  PS_ISOLATED,
  PS_DOUBLED,
  PS_BACKWARD,
  PS_FEATURE_COUNT
};


// First fitted at S027 with the other 811 constants frozen -- `tuner --only
// pawn_structure` over 1490839 self-play positions, held-out error 0.106892 to
// 0.106766. What ships is S083's refit of the whole evaluation at 46 rows a
// game (01222f3, H1), {-9, -2, -11} and {-15, -18, -2}; S125's screen refit
// these three alone first, its P0, and its family's tables by file and by rank
// take them as their baselines (DEC-331 (4), (5)). The values below are the
// flats of its candidate, the full fit that freed them with its four kept
// groups (adocs/data/S125_screen_reading.txt).
//
// That improvement is the smallest of S027's three terms: king safety moved the
// error 0.000304 and measured +20.87 Elo, passed pawns 0.000206 and measured
// +17.34, this one 0.000126 against a 4.1 % speed cost it has to pay back.
//
// A residual, not a valuation. The middlegame doubled weight was the one that
// showed it while S027's fit stood: +5, which read as chess would say doubling
// a pawn helps. It said nothing of the kind, and the refits since -- S065 to
// {-9, -10, -12}, S076's deduplicated corpus to {-10, -9, -11} and S083's to
// {-9, -2, -11} -- moved it to a sign that happens to read the other way, which
// is the same non-statement.
// The features overlap by construction -- a doubled pawn can be isolated or
// backward too, the header says how -- and they sit on top of a pawn
// piece-square table that has already priced the files, so the fit distributes
// one effect across whatever is available to it. Third time in this file;
// DEC-044 and the passed pawn comment above record the other two. Isolated and
// backward overlapped as well until S125's close, which made them exclusive
// (DEC-331 (3)): backward's flat now prices the backward pawns that are not
// isolated.
//
// The exact definition of each of the three is in evaluation.hpp, written out
// there because the walk in tests/test_walked_counts.hpp is built from it and
// the two have to mean the same thing.
//
// Marginal cost 4.1 % of a depth 14 search, measured on top of the passed pawn
// term with the weights forced non-zero because at zero the compiler deletes
// them, DEC-047. The SPRT was net of that: +13.05 +/- 11.15 Elo over 2290
// games, H1 accepted.
constexpr int pawn_structure_mg[PS_FEATURE_COUNT] = {1, -7, -6};
constexpr int pawn_structure_eg[PS_FEATURE_COUNT] = {-11, -20, 0};
constexpr std::array<score_t, PS_FEATURE_COUNT> pawn_structure_score =
    pack_scores(pawn_structure_mg, pawn_structure_eg);


// S125's pawn structure family, seven groups on top of the three features
// above (DEC-331): isolated pawns whose stop an enemy pawn attacks, isolated
// and doubled pawns by file, backward pawns by rank, phalanx and supported
// pawns by rank, and weak unopposed pawns. evaluation.hpp defines each set and
// count; evaluate_pawns() computes them from the fills it already builds.
//
// **Fitted, and shipped on one SPRT's H1** at {0, 5} nElo, 8+0.08: the screen's
// four kept groups and the three flats above, together
// (adocs/data/S125_sprt.log; the stopping Elo is not the effect size, DEC-063).
// The family landed at zero, S123's form, so that DEC-259's screen could read
// each group from the shipped engine (INV-6, adocs/data/S125_inv6.txt). No
// seed: nothing here was taken from any engine or page. Every family entry
// started the fit at zero and the three flats at S083's {-9, -2, -11} and
// {-15, -18, -2}, this project's own fit (DEC-084, DEC-331 (5)). What it costs:
// adocs/data/S125_cost_reading.txt, rows `cand` and p1, and RUN 3's per call.
//
// A table indexed by file or by rank drops one baseline cell when a flat
// weight above already prices the whole set: the a file for isolated and
// doubled, own rank 2 for backward. Without the drop the cells would sum to
// the flat's column on every row, one exact dependency a half a term
// (DEC-331 (4)). Phalanx and supported have no flat, so their tables are
// whole.
//
// The values below are the screen's full fit with four groups kept, 02567
// (adocs/data/S125_screen_reading.txt), pasted unchanged: monotonicity is not
// imposed, so a row that is not monotonic is the fit's, not corrected by hand.
//
// isolated_backward is not a fit of its own. It landed as an increment on top
// of isolated and backward, and the screen dropped it, so the increment is
// zero. S125's close then took those pawns out of backward's flat and gave them
// this weight, backward's flat plus that zero: every score is what it was
// (DEC-331 (3), (15); INV-6 against the candidate,
// adocs/data/S125_close_inv6.txt). These pawns have their own weight for S126
// to fit, and until then pay what they paid.
//
// The two other groups the screen dropped -- doubled by file, backward by rank
// -- stay at zero and in no expression.
constexpr int isolated_backward_mg[1] = {-6};
constexpr int isolated_backward_eg[1] = {0};
constexpr int isolated_file_mg[7] = {1, -6, -11, -10, -3, -7, 1};
constexpr int isolated_file_eg[7] = {-3, -1, -2, 0, -1, 3, 2};
constexpr int doubled_file_mg[7] = {0, 0, 0, 0, 0, 0, 0};
constexpr int doubled_file_eg[7] = {0, 0, 0, 0, 0, 0, 0};
constexpr int backward_rank_mg[3] = {0, 0, 0};
constexpr int backward_rank_eg[3] = {0, 0, 0};
constexpr int phalanx_mg[6] = {5, 2, 7, 20, 86, 86};
constexpr int phalanx_eg[6] = {-1, 1, 4, 7, 68, 68};
constexpr int supported_mg[5] = {8, 5, 7, 32, 187};
constexpr int supported_eg[5] = {11, 0, 4, 26, -29};
constexpr int weak_unopposed_mg[1] = {-10};
constexpr int weak_unopposed_eg[1] = {-6};
constexpr std::array<score_t, 1> isolated_backward_score =
    pack_scores(isolated_backward_mg, isolated_backward_eg);
constexpr std::array<score_t, 7> isolated_file_score =
    pack_scores(isolated_file_mg, isolated_file_eg);
constexpr std::array<score_t, 7> doubled_file_score =
    pack_scores(doubled_file_mg, doubled_file_eg);
constexpr std::array<score_t, 3> backward_rank_score =
    pack_scores(backward_rank_mg, backward_rank_eg);
constexpr std::array<score_t, 6> phalanx_score =
    pack_scores(phalanx_mg, phalanx_eg);
constexpr std::array<score_t, 5> supported_score =
    pack_scores(supported_mg, supported_eg);
constexpr std::array<score_t, 1> weak_unopposed_score =
    pack_scores(weak_unopposed_mg, weak_unopposed_eg);

// One flag a group, as S123's above: a group whose weights are all zero is
// not in the engine's evaluate_cheap() at all. That keeps a purpose here:
// doubled_file and backward_rank are at zero today, so those two flags are
// false and their loops compiled out, and a refit could zero another.
static constexpr bool isolated_backward_live =
    any_weight(isolated_backward_score);
static constexpr bool isolated_file_live = any_weight(isolated_file_score);
static constexpr bool doubled_file_live = any_weight(doubled_file_score);
static constexpr bool backward_rank_live = any_weight(backward_rank_score);
static constexpr bool phalanx_live = any_weight(phalanx_score);
static constexpr bool supported_live = any_weight(supported_score);
static constexpr bool weak_unopposed_live = any_weight(weak_unopposed_score);


// The three piece placement features, in the order their weights are indexed.
// Shared between the weights, the counts and the accessor so that none of the
// three can disagree with the others about which index means what. PL and not
// PP because tools/tuner_layout.hpp already spells passed pawn PP_, and two
// prefixes one letter apart on the same seven arrays is a mistake waiting to be
// made. The passed pawn buckets are indexed by rank and have no enum.
//
// Four until S134, which deleted rook on the seventh: it was the signed sum of
// eight psqt[ROOK][8..15] columns, below, and its zero weight folded into
// those squares as +0 (DEC-090). Seven since S135, which appended the two
// bad bishop counts and the two trapped piece counts DEC-258 gave the group.
// Appended rather than interleaved, so the first three keep the indices every
// fit and every record since S027 has used for them.
enum
{
  PL_BISHOP_PAIR,
  PL_ROOK_OPEN,
  PL_ROOK_HALF_OPEN,
  PL_BAD_BISHOP,
  PL_BAD_BISHOP_RAMMED,
  PL_TRAPPED_MINOR,
  PL_TRAPPED_ROOK,
  PL_FEATURE_COUNT
};


// Landed at zero, deliberately, and fitted by tools/tuner afterwards -- at S027
// and again at S135. Same staging as the three terms above and for the same
// reason: at zero the landing commit is provably behaviour-neutral, so the
// SPRT that follows measures the fitted term instead of a guess. No published
// table was consulted, DEC-016.
//
// The exact definition of each of the seven is in evaluation.hpp, written out
// there because the walk in tests/test_walked_counts.hpp is built from it and
// the two have to mean the same thing. What is here is why the arithmetic
// below computes them the way it does. The costs and the verdict that follow
// are S027's, measured on four features, rook on the seventh among them,
// before S134 deleted it; S135's verdict and cost close this comment.
//
// **Nothing measured at the zero weights S027 landed says anything about what
// they cost** -- clang sees the zeros and deletes the popcounts and the
// arithmetic that feed them, DEC-047, and evaluate_cheap() disassembles to the
// same 164 instructions with the term as without it.
//
// Forced non-zero with the other three terms already live, so the figure is the
// marginal cost of adding this one and not the cost of the four: **3.1 % to
// 4.0 % of a depth 14 search**, 7.539 s to 7.786 s in one run order and 7.539 s
// to 7.838 s in the other, hyperfine 20 runs per build at sigma 0.7 to 0.9 %.
// The costing build computes the term at forced weights and throws it away
// behind an asm barrier so the score is unchanged, which is what makes the two
// node-for-node identical -- 47438623 nodes either way over the three
// search_bench positions -- and the wall time the cost and nothing else. That
// figure is the sum of the three last iterations, which is what `info nodes`
// reported until S037; the whole-search count is roughly twice it.
//
// The barrier is not what is being measured: the same build with the barrier
// left in place and these weights back at zero ran within 0.5 % of the shipping
// build with no barrier at all, and in opposite directions in the two orders
// (+0.34 % and -0.52 % on medians), so its own cost is below what this
// instrument resolves.
//
// **It was expected to be much cheaper than that and it is not.** Passed pawns
// cost 4.5 % and pawn structure 4.1 %, and this term shares their fills, adds
// no fifth one and reads four extra bitboards -- yet it lands in the same band.
// The disassembly says why, and it is not recomputation: six of the eight
// counts are a real popcount, and on this target each one is a round trip out
// of the general registers and back, +6 cnt.8b, +6 uaddlv.8b and +12 fmov for
// 65 instructions on a function that was 164. The bishop pair is the one
// feature that costs nothing -- clang rewrites count_bits(x) >= 2 into x & (x -
// 1), so it compiles to a tst and a cset. The 3.1 to 4.0 % was paid by S027's
// weights and their SPRT was net of it; DEC-040 is the warning about judging a
// term while its speed cost is still unpaid.
//
// **S027's weights went back to zero because they measured zero.** They were
// mg {2, 32, 8, -26} and eg {55, -8, 12, 11}, fitted with the other 817
// constants frozen, held-out error 0.106766 to 0.106605 -- and they measured
// -5.48 +/- 11.46 Elo over 2284 games, H0 accepted. At zero the compiler
// deleted the whole term, checked rather than assumed: evaluate_cheap() was
// the same 164 instructions with it as without. So it cost nothing to leave,
// and what it left was the plumbing S135 refitted (below).
//
// **This is the term that broke the pattern, and that is the useful part.** Its
// error improvement was 0.000161, the second largest of the step's four terms,
// above pawn structure's 0.000126 which measured +13.05 Elo. Three terms in a
// row had held-out error understating the Elo; this one had it overstating by
// enough to change the sign. Held-out error is not a weak predictor of
// strength, it is an unreliable one in both directions, and the only thing that
// decides a term here is the match.
//
// Two candidates for why, neither of them tested:
//
// The bishop pair is the only one of the four that is free -- clang rewrites
// count_bits(x) >= 2 into x & (x - 1) -- and it carries the largest weight the
// step fitted. The three rook features cost six popcounts between them, 3.1 to
// 4.0 % of a search, and their weights are small and mutually cancelling. A
// split measuring the pair alone would separate a cheap term that may work from
// three expensive ones that may not.
//
// And the corpus is self-play by an engine that predates all of S027. A
// residual fitted on it encodes what correlated with winning in *those* games,
// which for a feature like a rook on the seventh may be a consequence of
// already being better rather than a cause of becoming so. Reducing squared
// error on that corpus and playing better are different objectives, and this is
// the first term where they came apart.
//
// **S100 diagnosed the zero and it is not a result about these features.**
// Three of the four causes are excluded: the counts are right (all 10795695
// corpus rows re-extracted, 0 disagreements), the gradient is the derivative of
// the error (worst 4.9e-8 against finite differences over all 827 parameters),
// and the pipeline recovers a planted vector end to end. What is left is the
// measurement: **one bundled SPRT for four features** at `--fast` bounds
// `elo0=0 elo1=10`, against published figures of +8.2 for the pair and +9.86
// for a rook file retune -- bounds that cannot resolve either (DEC-063,
// DEC-084). Then the weights were zeroed by hand and every fit from S065 to
// S083 held them there with `--freeze piece_placement` (DEC-057): held at
// zero, never fitted to it, until S135 fitted them again (below).
//
// **Three of the four were the real candidates and one was not a quantity.**
// R^2 of each column on the tables that could absorb it: bishop pair 0.487,
// rook open 0.265 and rook half-open 0.168, all far under redundancy -- so
// those three carry signal the tables do not. Rook on the seventh was different
// in kind: index 0 is a8 and a black piece mirrors by xor 56, so a rook on its
// own seventh always lands on squares 8..15 and that column *was* the signed
// sum of eight psqt_mg[ROOK] columns -- R^2 exactly 1.000000, 0 violations in
// 1264773 non-zero rows, and the same out of the engine's own
// piece_placement_counts(). Only the sum psqt[rook][8..15] + placement[seventh]
// was identified, so S134 deleted the feature and folded its weight, zero, into
// those eight squares (DEC-090). The three left are the real candidates.
//
// **S135 added four columns, at zero for the reason the first paragraph
// gives**: the two bad bishop counts and the two trapped piece counts,
// defined in evaluation.hpp from CPW's *Bad Bishop* and *Trapped Pieces* and
// computed in piece_placement_features() below. No number was taken from
// either page -- the trapped piece page prints penalties, and a published
// penalty is not even a seed here (DEC-105, DEC-134) -- so all seven start
// the fit at zero. Not one of the seven is a sum of piece-square columns:
// each new one is a product or a conjunction of two pieces' squares, which a
// table indexed by one piece's square cannot express; tools/feature_audit's
// identity report is what checks that and not this sentence. The seven are
// screened one term at a time before any of them plays a match (DEC-259) --
// tools/tuner_groups.hpp `PLACEMENT_TERMS` names the terms.
//
// **The weights below are S135's screen and its joint fit** (DEC-259,
// DEC-315): each term fitted alone had to lower held-out loss past a stated
// margin to play at all, so the bishop pair, the two rook files and the two
// trapped counts were fitted together on S083's corpus, every other constant
// frozen, and the bad bishop and its rammed share stay at zero because the
// screen dropped them -- their fit never beat its start. Their one SPRT,
// `{0, 5}` nElo at 8+0.08, read H1 (adocs/data/S135_sprt.log), so the five
// ship together and no bisection follows; their speed cost, between S135's
// cost run's +0.92 % and +3.92 % on g++ 13.3
// (adocs/data/S135_cost_reading.txt), is inside that verdict.
constexpr int piece_placement_mg[PL_FEATURE_COUNT] = {-5, 23,  3,  0,
                                                      0,  -40, -23};
constexpr int piece_placement_eg[PL_FEATURE_COUNT] = {37, -4,  13, 0,
                                                      0,  -68, -16};
constexpr std::array<score_t, PL_FEATURE_COUNT> piece_placement_score =
    pack_scores(piece_placement_mg, piece_placement_eg);

// How far one side's count of each placement feature reaches on a board
// load_FEN accepts, in the order the weights are indexed, for the bound below.
// The bishop pair is 0 or 1 a side. At most 15 rooks a side. The two bad
// bishop counts are a sum of bishops on a colour times own pawns on it, and a
// side's bishops and pawns number at most 15 between them, so they reach 7 x 8
// = 56. A trapped minor stands on one of six squares (a6, a7, a8, h6, h7, h8
// for White) and a trapped rook on one of four beside its king's corner, one
// piece a square. Each enters as White's count minus Black's, two counts from
// 0 to the reach, so the reach bounds the difference too.
static constexpr int piece_placement_reach[PL_FEATURE_COUNT] = {1,  15, 15, 56,
                                                                56, 6,  4};


// The largest magnitude in one weight array, one half.
template <size_t N>
static constexpr int largest_entry(const int (&weights)[N])
{
  int largest = 0;
  for (size_t i = 0; i < N; ++i) {
    largest = std::max(largest, (weights[i] < 0) ? -weights[i] : weights[i]);
  }
  return largest;
}


// How far the piece-square sum can reach in one half. White's pieces add their
// entries and Black's subtract theirs (eval_add_piece()), and load_FEN accepts
// at most one king and fifteen other pieces a colour. So White's king and
// Black's together lie within the king table's highest entry, or 0, less its
// lowest, or 0; and the fifteen others a side within fifteen times the same
// spread over the other five tables. The 0s are what keep the bound true for a
// side with fewer pieces -- every subset make_move passes through on its way
// from one position to the next included -- and the unreachable pawn squares
// on ranks 1 and 8 are read as well, which only widens it.
static constexpr int table_reach(const int (&table)[6][64])
{
  int king_high = 0;
  int king_low = 0;
  int piece_high = 0;
  int piece_low = 0;

  for (int square = 0; square < 64; ++square) {
    king_high = std::max(king_high, table[5][square]);
    king_low = std::min(king_low, table[5][square]);

    for (int type = 0; type < 5; ++type) {
      piece_high = std::max(piece_high, table[type][square]);
      piece_low = std::min(piece_low, table[type][square]);
    }
  }

  return (king_high - king_low) + 15 * (piece_high - piece_low);
}


// S123's family in one half, bounded the same way: a side has at most 15
// pawns, each in one rank cell of A's two arrays, of B's two with a distance
// of at most 7, and of C's, so each array's White-minus-Black sum is within
// 2 x 15 (x 7 for a distance) of its largest entry.
static constexpr int passer_family_half(const int (&free)[6],
                                        const int (&safe)[6],
                                        const int (&king_own)[6],
                                        const int (&king_enemy)[6],
                                        const int (&candidate)[5])
{
  return 2 * 15 * (largest_entry(free) + largest_entry(safe)) +
         2 * 15 * 7 * (largest_entry(king_own) + largest_entry(king_enemy)) +
         2 * 15 * largest_entry(candidate);
}


// How far one of S125's arrays reaches in one half. Each puts a side's pawns
// in one cell each, at most 15 of them, so White's sum lies between 15 times
// the array's lowest entry, or 0, and 15 times its highest, or 0, and Black's
// is subtracted from it: the difference is within 15 times the spread.
template <size_t N>
static constexpr int pawn_cells_reach(const int (&weights)[N])
{
  int high = 0;
  int low = 0;
  for (size_t i = 0; i < N; ++i) {
    high = std::max(high, weights[i]);
    low = std::min(low, weights[i]);
  }
  return 15 * (high - low);
}


// S125's family in one half: the seven arrays' reaches summed.
static constexpr int structure_family_half(const int (&isolated_backward)[1],
                                           const int (&isolated_file)[7],
                                           const int (&doubled_file)[7],
                                           const int (&backward_rank)[3],
                                           const int (&phalanx)[6],
                                           const int (&supported)[5],
                                           const int (&weak_unopposed)[1])
{
  return pawn_cells_reach(isolated_backward) + pawn_cells_reach(isolated_file) +
         pawn_cells_reach(doubled_file) + pawn_cells_reach(backward_rank) +
         pawn_cells_reach(phalanx) + pawn_cells_reach(supported) +
         pawn_cells_reach(weak_unopposed);
}


// One half of stage one's packed sum, bounded. Passers add for White and
// subtract for Black, at most 15 a side, so all 30 can push the half the same
// way. Structure and placement enter as White's count minus Black's: two
// counts of 0 to 15 for each structure feature, and each placement feature's
// reach above. S123's family is `family`, passer_family_half() above, and
// S125's `structure_family`, structure_family_half().
static constexpr int stage_one_half(const int (&table)[6][64],
                                    const int (&passed)[5],
                                    const int (&structure)[PS_FEATURE_COUNT],
                                    const int (&placement)[PL_FEATURE_COUNT],
                                    int family,
                                    int structure_family)
{
  int bound = table_reach(table) + 2 * 15 * largest_entry(passed) +
              PS_FEATURE_COUNT * 15 * largest_entry(structure) + family +
              structure_family;

  for (int f = 0; f < PL_FEATURE_COUNT; ++f) {
    const int weight = (placement[f] < 0) ? -placement[f] : placement[f];
    bound += piece_placement_reach[f] * weight;
  }

  return bound;
}


// Stage one sums the board's packed piece-square score and the pawn terms
// before it takes the halves apart, so that sum is what has to fit 16 bits,
// each half on its own. Bounded by what load_FEN accepts, not by what a game
// reaches. A refit that breaks it fails to compile rather than wrapping one
// half into the other.
//
// S135 tightened it, and only this check reads it. Until S135 the tables
// were bounded by 32 times the largest entry of either half -- every piece
// on the king table's top square, 26912 at S134's weights -- which left about
// 1535 of the 32767 for every placement weight times its reach. Read one half
// at a time and by the spread a king and fifteen pieces a side can actually
// sum to, S134's tables bound at 17085 in the middlegame and 6394 in the
// endgame, and S083's, which ship, at 10930 and 7785: with the pawn terms
// 12565 and 12165, 20202 and 20602 of room, so a fit of the seven placement
// weights has it. The bound is still a worst case over boards no game
// reaches -- fifteen queens against fifteen bishops -- and still a proof.
// adocs/data/S135_screen.py's `headroom` repeats this arithmetic for a fitted
// header, so a fit that would fail the assert is refused before it is pasted.
// With S123's family shipping fitted the whole bound reads 19982 and 25799,
// 12785 and 6968 of room (adocs/data/S123_goldens.txt, by S123_screen.py's
// `headroom`, which adds passer_family_half()). S125's family adds
// structure_family_half(); adocs/data/S125_screen.py's `headroom` adds it to
// S123's arithmetic for every fit before a paste: with the family shipping
// fitted, and isolated_backward's weight from S125's close, the whole bound
// reads 24317 and 27929, 8450 and 4838 of room
// (adocs/data/S125_close_inv6.txt).
static constexpr int stage_one_bound =
    std::max(stage_one_half(psqt_mg,
                            passed_pawn_mg,
                            pawn_structure_mg,
                            piece_placement_mg,
                            passer_family_half(passed_free_mg,
                                               passed_safe_mg,
                                               passed_king_own_mg,
                                               passed_king_enemy_mg,
                                               candidate_passer_mg),
                            structure_family_half(isolated_backward_mg,
                                                  isolated_file_mg,
                                                  doubled_file_mg,
                                                  backward_rank_mg,
                                                  phalanx_mg,
                                                  supported_mg,
                                                  weak_unopposed_mg)),
             stage_one_half(psqt_eg,
                            passed_pawn_eg,
                            pawn_structure_eg,
                            piece_placement_eg,
                            passer_family_half(passed_free_eg,
                                               passed_safe_eg,
                                               passed_king_own_eg,
                                               passed_king_enemy_eg,
                                               candidate_passer_eg),
                            structure_family_half(isolated_backward_eg,
                                                  isolated_file_eg,
                                                  doubled_file_eg,
                                                  backward_rank_eg,
                                                  phalanx_eg,
                                                  supported_eg,
                                                  weak_unopposed_eg)));
static_assert(stage_one_bound <= 32767,
              "stage one's packed sum can overflow a 16-bit half");


// Toward rank 8, which is toward index 0 because index 0 is a8. White's
// forward direction. Three shifts and not seven: each one doubles the distance
// already filled, so 8, 16 and 32 between them cover all eight ranks.
static inline bb_t fill_toward_rank8(bb_t squares)
{
  squares |= squares >> 8;
  squares |= squares >> 16;
  squares |= squares >> 32;
  return squares;
}


// Toward rank 1, which is toward index 63. Black's forward direction.
static inline bb_t fill_toward_rank1(bb_t squares)
{
  squares |= squares << 8;
  squares |= squares << 16;
  squares |= squares << 32;
  return squares;
}


// The two neighbouring files of every square in the set, and not the squares
// themselves. The file masks are the whole point of the function: a bit on the
// h file shifted one to the left lands on the a file of the next rank down, so
// without them a pawn on one edge of the board would stop a passer on the
// other. That is the bug these terms are most likely to have and it is
// invisible while the weights are zero, so it was not reasoned about: dropping
// the two masks and re-running the corpus comparison that replaced the old
// per-pawn form disagreed on 3 of the 2696 test positions and 129574 of the
// 1490839 tuning ones.
static inline bb_t neighbour_files(bb_t squares)
{
  return ((squares << 1) & ~file_masks[0]) | ((squares >> 1) & ~file_masks[7]);
}


// One of the board's two square colours: the squares whose row and file sum
// to an even number, a8 and h1 among them. Which colour it is does not matter
// to the bad bishop counts, which ask only whether a bishop and a pawn share
// one; the complement is the other colour.
static constexpr bb_t one_colour = [] {
  bb_t squares = 0;
  for (int square = 0; square < 64; ++square) {
    if ((((square >> 3) + (square & 7)) & 1) == 0) {
      squares |= BB_1 << square;
    }
  }
  return squares;
}();


// Every own pawn on each bishop's colour, summed over the bishops: the bishops
// on one colour times the pawns on it, and the same for the other colour. Six
// popcounts and no loop, so three bishops off a promotion cost what one does.
// `pawns` is all of a side's pawns for PL_BAD_BISHOP and the rammed ones alone
// for PL_BAD_BISHOP_RAMMED.
static inline int pawns_on_bishop_colour(bb_t bishops, bb_t pawns)
{
  return count_bits(bishops & one_colour) * count_bits(pawns & one_colour) +
         count_bits(bishops & ~one_colour) * count_bits(pawns & ~one_colour);
}


// A square of White's trapped piece patterns, or for Black the same square
// with its rank mirrored, rank r to 9 - r -- which is xor 56 in this indexing,
// as eval_add_piece() mirrors Black's table squares. evaluation.hpp names each
// pattern by White's squares; this is the one place Black's are derived.
template <color_t colour>
static constexpr bb_t pattern_square(int square)
{ return BB_1 << ((colour == WHITE) ? square : (square ^ 56)); }


// Own knights and bishops on one of the trapped minor patterns, each piece once
// however many patterns it meets. CPW *Trapped Pieces* gives the patterns as
// White's on the h side; the a side is their mirror across the d and e files,
// so the feature does not prefer one wing.
//
// The trap squares are built from the enemy pawns and then intersected with
// the pieces, rather than asked per piece, so a knight on h7 that meets both
// of its patterns is one bit and counts once. The early return is for the
// common case: no own minor on any of the six squares a pattern can name, and
// then no pawn is looked at.
template <color_t colour>
static inline int trapped_minor_count(const board_t* board)
{
  const bb_t knights =
      board->bitboards[(colour == WHITE) ? W_KNIGHT : B_KNIGHT];
  const bb_t bishops =
      board->bitboards[(colour == WHITE) ? W_BISHOP : B_BISHOP];

  constexpr bb_t knight_squares =
      pattern_square<colour>(a8) | pattern_square<colour>(h8) |
      pattern_square<colour>(a7) | pattern_square<colour>(h7);
  constexpr bb_t bishop_squares =
      pattern_square<colour>(a7) | pattern_square<colour>(h7) |
      pattern_square<colour>(a6) | pattern_square<colour>(h6);

  if (((knights & knight_squares) | (bishops & bishop_squares)) == 0) {
    return 0;
  }

  // The enemy's pawns, which are what every pattern names.
  const bb_t pawns = board->bitboards[(colour == WHITE) ? B_PAWN : W_PAWN];

  const auto all = [pawns](bb_t need) { return (pawns & need) == need; };
  const auto any = [pawns](bb_t need) { return (pawns & need) != 0; };
  const auto at = [](int square) { return pattern_square<colour>(square); };

  bb_t knight_traps = 0;

  // A knight on h8 with an enemy pawn on h7 or on f7, and its mirror on a8.
  if (any(at(h7) | at(f7))) { knight_traps |= at(h8); }
  if (any(at(a7) | at(c7))) { knight_traps |= at(a8); }

  // A knight on h7 with enemy pawns on g7 and h6, or on g7 and f6.
  if (all(at(g7)) && any(at(h6) | at(f6))) { knight_traps |= at(h7); }
  if (all(at(b7)) && any(at(a6) | at(c6))) { knight_traps |= at(a7); }

  bb_t bishop_traps = 0;

  // A bishop on h7 with enemy pawns on f7 and g6; on h6 with them on g5 and
  // f6. Both pawns, in each.
  if (all(at(f7) | at(g6))) { bishop_traps |= at(h7); }
  if (all(at(c7) | at(b6))) { bishop_traps |= at(a7); }
  if (all(at(g5) | at(f6))) { bishop_traps |= at(h6); }
  if (all(at(b5) | at(c6))) { bishop_traps |= at(a6); }

  return count_bits(knights & knight_traps) +
         count_bits(bishops & bishop_traps);
}


// Own rooks hemmed in by their own king: on h1, g1, h2 or g2 while the king
// stands on f1 or g1, or on the mirror squares a1, b1, a2, b2 while it stands
// on c1 or b1 -- White's squares, Black's mirrored. One king, so at most one
// corner is live at a time.
template <color_t colour>
static inline int trapped_rook_count(const board_t* board)
{
  const bb_t king = board->bitboards[(colour == WHITE) ? W_KING : B_KING];
  const bb_t rooks = board->bitboards[(colour == WHITE) ? W_ROOK : B_ROOK];

  const auto at = [](int square) { return pattern_square<colour>(square); };

  const bb_t king_corner = ((king & (at(f1) | at(g1))) != 0)
                               ? (at(h1) | at(g1) | at(h2) | at(g2))
                               : 0;
  const bb_t queen_corner = ((king & (at(c1) | at(b1))) != 0)
                                ? (at(a1) | at(b1) | at(a2) | at(b2))
                                : 0;

  return count_bits(rooks & (king_corner | queen_corner));
}


// The seven piece placement counts per side, WHITE first, exactly as
// evaluation.hpp defines them.
//
// `white_pawn_files` and `black_pawn_files` are whole-file sets -- every square
// on a file that side has a pawn on -- and they are arguments rather than
// something computed here because the pawn terms have already built them out of
// their own fills. That sharing is the reason this term is not a pass of its
// own: what is left to compute is two popcounts and a mask-and per side.
//
// Both file sets cover the whole file, so intersecting a rook with one asks
// only about the rook's file and the rook's rank drops out of it.
//
// **The four S135 added read pieces, pawns and the king square and nothing
// else** -- no occupancy and no attack set -- so they sit here, in stage one
// with the three before them, for the reason the comment over
// evaluate_cheap() gives for those: they need nothing the expensive stage
// builds, so nothing about them belongs behind the lazy shortcut and its
// clamp. They are computed from the bitboards at every evaluation, as the
// pawn terms are, and that is not the recomputation INV-4 forbids: INV-4 holds
// the accumulators make_move maintains -- material, the tables, the phase --
// and none of these is a sum over pieces that make_move could keep. Each is a
// product or a conjunction across two piece sets, one of them usually the
// other side's pawns, so a move by either side changes it with no O(1) delta.
static inline void piece_placement_features(const board_t* board,
                                            bb_t white_pawn_files,
                                            bb_t black_pawn_files,
                                            int features[2][PL_FEATURE_COUNT])
{
  const bb_t white_rooks = board->bitboards[W_ROOK];
  const bb_t black_rooks = board->bitboards[B_ROOK];
  const bb_t white_pawns = board->bitboards[W_PAWN];
  const bb_t black_pawns = board->bitboards[B_PAWN];

  // Files with no pawn of either colour. Pieces do not close a file, only
  // pawns do, which is why nothing but the two pawn sets is read here.
  const bb_t open_files = ~(white_pawn_files | black_pawn_files);

  // Rammed: the square directly ahead holds an enemy pawn. Ahead is toward
  // index 0 for White, so a Black pawn there sits eight below the White one's
  // index and the shift by eight brings it onto it; mirrored for Black. Only a
  // pawn rams -- a piece in front can move away, and the bad bishop page's
  // point is that this restriction is semi-permanent.
  const bb_t white_rammed = white_pawns & (black_pawns << 8);
  const bb_t black_rammed = black_pawns & (white_pawns >> 8);

  features[WHITE][PL_BISHOP_PAIR] =
      (count_bits(board->bitboards[W_BISHOP]) >= 2) ? 1 : 0;
  features[WHITE][PL_ROOK_OPEN] = count_bits(white_rooks & open_files);
  features[WHITE][PL_ROOK_HALF_OPEN] =
      count_bits(white_rooks & ~white_pawn_files & black_pawn_files);
  features[WHITE][PL_BAD_BISHOP] =
      pawns_on_bishop_colour(board->bitboards[W_BISHOP], white_pawns);
  features[WHITE][PL_BAD_BISHOP_RAMMED] =
      pawns_on_bishop_colour(board->bitboards[W_BISHOP], white_rammed);
  features[WHITE][PL_TRAPPED_MINOR] = trapped_minor_count<WHITE>(board);
  features[WHITE][PL_TRAPPED_ROOK] = trapped_rook_count<WHITE>(board);

  features[BLACK][PL_BISHOP_PAIR] =
      (count_bits(board->bitboards[B_BISHOP]) >= 2) ? 1 : 0;
  features[BLACK][PL_ROOK_OPEN] = count_bits(black_rooks & open_files);
  features[BLACK][PL_ROOK_HALF_OPEN] =
      count_bits(black_rooks & ~black_pawn_files & white_pawn_files);
  features[BLACK][PL_BAD_BISHOP] =
      pawns_on_bishop_colour(board->bitboards[B_BISHOP], black_pawns);
  features[BLACK][PL_BAD_BISHOP_RAMMED] =
      pawns_on_bishop_colour(board->bitboards[B_BISHOP], black_rammed);
  features[BLACK][PL_TRAPPED_MINOR] = trapped_minor_count<BLACK>(board);
  features[BLACK][PL_TRAPPED_ROOK] = trapped_rook_count<BLACK>(board);
}


// One placement column's share of the sum, and its trace entry.
//
// A template on the column, and the weight tested with `if constexpr`,
// because the plain loop over the seven let g++ 13.3 vectorise columns 0 to 3
// into one vpmulld against the packed weights: a zero lane is still loaded and
// multiplied, so with S135's candidate shipping bad bishop at zero beside
// three non-zero columns, its count was computed at every node for nothing
// (S135's apply, 2026-10-25: 8 extra popcnt and its colour masks in
// evaluate_cheap()). Here a zero-weighted column is in no expression, so its
// count is dead and deleted whatever the columns beside it weigh. The trace
// still records every column: the tuner fits the zero ones too.
template <int f>
static inline void add_placement_column(
    const int placement[2][PL_FEATURE_COUNT],
    score_t* sum)
{
  const int difference = placement[WHITE][f] - placement[BLACK][f];

  if constexpr (piece_placement_score[f] != 0) {
    *sum += difference * piece_placement_score[f];
  }
  EVAL_TRACE(TERM_PIECE_PLACEMENT, f, difference);
}


// The seven columns in index order, the order the loop traced them in.
template <int... f>
static inline void add_placement_columns(
    const int placement[2][PL_FEATURE_COUNT],
    score_t* sum,
    std::integer_sequence<int, f...>)
{ (add_placement_column<f>(placement, sum), ...); }


// The king's distance between two squares, 0 to 7: the larger of the rank gap
// and the file gap.
static inline int chebyshev(int a, int b)
{
  const int ranks = (a >> 3) - (b >> 3);
  const int files = (a & 7) - (b & 7);
  return std::max((ranks < 0) ? -ranks : ranks, (files < 0) ? -files : files);
}


// S123's A and B for one side's passers, its own seventh included, into the
// stage-one sum and the trace, and into `out` when collecting. evaluation.hpp
// defines the counts.
//
// `advance` and `kings` say which groups are computed at all. The engine
// passes the groups' liveness, so a group whose weights are all zero is not
// in its evaluate_cheap() -- is_attacked() is an out-of-line call the
// compiler could not delete as it deletes a dead popcount, so this is what
// kept the zero-weight landing free (S135's form, DEC-329 (3)). Both groups
// are live since S123's H1; the gate is there for a refit that zeroes one
// again. The trace build and the accessors pass true for both.
//
// Safe reads is_attacked() once per passer whose push square is empty, and
// only then: a blocked passer is neither free nor safe. That repeats slider
// lookups stage two makes for mobility, deliberately -- stage two is behind
// the lazy shortcut and its clamp, which would truncate these terms on the
// positions they exist for (DEC-329 (1)).
template <color_t colour, bool advance, bool kings, bool collect>
static inline void passer_family_side(const board_t* board,
                                      bb_t passers,
                                      score_t* sum,
                                      passer_family_t* out)
{
  constexpr int sign = (colour == WHITE) ? 1 : -1;
  constexpr color_t enemy = (colour == WHITE) ? BLACK : WHITE;

  // A passer whose push square is empty: White's push square is eight below
  // its index and Black's eight above, so the empty set shifted the other way
  // lands each push square's bit on its pawn.
  const bb_t empty = ~board->occupancies[BOTH];
  const bb_t free = passers & ((colour == WHITE) ? (empty << 8) : (empty >> 8));

  int own_king = 0;
  int enemy_king = 0;
  if constexpr (kings) {
    own_king =
        get_lsb_index(board->bitboards[(colour == WHITE) ? W_KING : B_KING]);
    enemy_king =
        get_lsb_index(board->bitboards[(colour == WHITE) ? B_KING : W_KING]);
  }

  while (passers) {
    const int square = get_lsb_index(passers);
    passers &= passers - 1;

    // The own rank, 0 for the second to 5 for the seventh; a pawn cannot
    // stand on the first or the eighth.
    const int rank = (colour == WHITE) ? 6 - (square >> 3) : (square >> 3) - 1;
    const int push = (colour == WHITE) ? square - 8 : square + 8;

    assert(rank >= 0 && rank < 6);

    if constexpr (advance) {
      if ((free >> square) & 1) {
        *sum += sign * passed_free_score[rank];
        EVAL_TRACE(TERM_PASSED_FREE, rank, sign);
        if constexpr (collect) { out->free[colour][rank]++; }

        if (!is_attacked(game_tables(), board, static_cast<index_t>(push),
                         enemy)) {
          *sum += sign * passed_safe_score[rank];
          EVAL_TRACE(TERM_PASSED_SAFE, rank, sign);
          if constexpr (collect) { out->safe[colour][rank]++; }
        }
      }
    }

    if constexpr (kings) {
      const int own = chebyshev(own_king, push);
      const int other = chebyshev(enemy_king, push);

      *sum += (sign * own) * passed_king_own_score[rank];
      *sum += (sign * other) * passed_king_enemy_score[rank];
      EVAL_TRACE(TERM_PASSED_KING_OWN, rank, sign * own);
      EVAL_TRACE(TERM_PASSED_KING_ENEMY, rank, sign * other);

      if constexpr (collect) {
        out->king_own[colour][rank] += own;
        out->king_enemy[colour][rank] += other;
      }
    }
  }
}


// S123's C, set-wise: each side's candidate passers, evaluation.hpp's
// definition. `white_passers` and `black_passers` are each side's passers,
// the own seventh included; `white_front` and `black_front` each side's front
// span, which for the other side's pawns is the set of squares with an enemy
// pawn strictly ahead on the file -- both built for the passers already.
//
// A square's pawn control, per colour, is a count of 0 to 2, held as two sets:
// attacked by at least one pawn of the colour, and by two. The enemy controls
// a square with more pawns than the side when it attacks it twice and the
// side not twice, or at all and the side not at all. A pawn whose path holds
// such a square is behind it on its file: that square's backward fill, less
// the square itself.
static inline void candidate_passers(bb_t white_pawns,
                                     bb_t black_pawns,
                                     bb_t white_passers,
                                     bb_t black_passers,
                                     bb_t white_front,
                                     bb_t black_front,
                                     bb_t out[2])
{
  // A White pawn attacks the squares one rank up and one file to each side,
  // index minus 9 and minus 7; Black's plus 7 and plus 9. The file masks drop
  // the pawns on the edge file a shift would wrap from.
  const bb_t white_west = (white_pawns & ~file_masks[0]) >> 9;
  const bb_t white_east = (white_pawns & ~file_masks[7]) >> 7;
  const bb_t black_west = (black_pawns & ~file_masks[0]) << 7;
  const bb_t black_east = (black_pawns & ~file_masks[7]) << 9;

  const bb_t white_once = white_west | white_east;
  const bb_t white_twice = white_west & white_east;
  const bb_t black_once = black_west | black_east;
  const bb_t black_twice = black_west & black_east;

  const bb_t black_holds =
      (black_twice & ~white_twice) | (black_once & ~white_once);
  const bb_t white_holds =
      (white_twice & ~black_twice) | (white_once & ~black_once);

  // Behind for White is toward index 63, so a held square's fill toward rank
  // 1, shifted one rank further, is every square with it strictly ahead.
  const bb_t white_contested = fill_toward_rank1(black_holds) << 8;
  const bb_t black_contested = fill_toward_rank8(white_holds) >> 8;

  out[WHITE] = white_pawns & ~white_passers & ~black_front & ~white_contested;
  out[BLACK] = black_pawns & ~black_passers & ~white_front & ~black_contested;
}


// A pawn's own rank, 2 to 7 on the squares a pawn can stand on: White's
// second rank is row 6 of this indexing, index 0 being a8, and Black's row 1.
template <color_t colour>
static inline int own_rank(int square)
{ return (colour == WHITE) ? 8 - (square >> 3) : (square >> 3) + 1; }


// Each pawn of `pawns` into one of S125's tables: `cell` maps the pawn's
// square to its cell, or to -1 for the baseline cell no parameter holds, and
// `record` takes the cell once per pawn.
template <typename Cell, typename Record>
static inline void for_each_cell(bb_t pawns, Cell cell, Record record)
{
  while (pawns) {
    const int c = cell(get_lsb_index(pawns));
    pawns &= pawns - 1;
    if (c >= 0) { record(c); }
  }
}


// Three terms -- passed pawns, pawn structure and piece placement -- White
// minus Black, as one untapered middlegame and endgame pair for the caller to
// interpolate. Since S123 the passed pawns are the buckets and the family
// beside them, A, B and C, summed into the same pair.
//
// One function and not three because they are the same four fills read several
// different ways, and a second pass over the pawns would recompute them.
// The fusion is the one mobility and king safety already use, for the reason
// recorded there: asking the same question twice cost 7.7 % of the search.
//
// Passed pawns falls out of one identity, set-wise, in a fixed dozen shifts for
// the whole board. Widen the enemy pawns by one file each way, fill that toward
// the enemy's own back rank, and the result is every square an enemy pawn
// stands in the way of -- so the passers are the pawns not in it. The extra
// shift by eight makes the span exclusive, which is what stops two pawns facing
// each other on the same file from each deciding the other is behind it.
//
// Pawn structure is three counts per side and evaluation.hpp states each one
// exactly. Their signs are not decided here: the weights are fitted, so a
// penalty arrives as a negative weight rather than as a minus in this file.
//
// Piece placement is here for its two rook file counts and gets the bishop pair
// for free. The rook file features need to know which files carry which side's
// pawns, and the fills below already answer that; the bishop pair reads no pawn
// at all. A third pass over the pawns to rebuild what is already in registers
// is exactly what the fusion above exists to avoid.
//
// The first form of this looped over every pawn and asked
// passed_w_pawns_masks[square] whether that one was passed. It cost 8.50 ns per
// evaluate_cheap() call on kiwipete's sixteen pawns against 1.14 without the
// term, because the answer per pawn is a branch nothing can predict. This form
// does not look at a pawn at all until it already knows the pawn is passed:
// 2.75 ns, flat in the number of pawns, and 10.8 % of a depth 12 search down to
// 4.5 %. Both forms were run over 1493535 positions -- the 2696 of
// all_test_fens() and all 1490839 of the tuning corpus -- and agreed on the
// mg/eg pair in every one.
//
// Buckets by how far the pawn has advanced, not one weight multiplied by the
// rank. The value is not linear in the rank -- a pawn one square from queening
// is worth several times one on the third -- and the tuner fits a linear
// function of its parameters, so a non-linear shape has to sit in the features
// or it cannot be fitted here at all. Same constraint that made king safety
// linear, DEC-044. Five buckets, own second rank to own sixth: a sixth, a pawn
// on its own seventh, was a passer by definition and therefore exactly eight
// piece-square columns, and S134 folded it into them (DEC-090).
//
// The bucket arithmetic is the part of this term that is easy to get backwards.
// Index 0 is a8 and index 63 is h1, so White advances by *decreasing* the
// index: square >> 3 is 6 on White's second rank and 2 on the sixth. Reversed
// it pays for retreating pawns, and while the weights are zero no test can say
// so, because every score it produces is the same score. What can say so is
// passed_pawn_counts() below, which is why `collect` is here. Pawn structure
// has the same hole and pawn_structure_counts() is the same answer to it.
//
// `collect` is how the three accessors read the counts out without the search
// paying for it. A template rather than a flag, the arrangement king safety
// arrived at: at <false> the stores and the out pointers are not compiled at
// all, and the instantiation the search calls keeps exactly one caller, so
// clang still inlines it and the zero weights still fold everything downstream
// away. Handing that instantiation a second caller instead is what cost 21 %
// once -- see king_shelter_features() below.
template <bool collect>
static inline void evaluate_pawns(const board_t* board,
                                  score_t* score,
                                  int passed_out[2][5],
                                  int structure_out[2][PS_FEATURE_COUNT],
                                  int placement_out[2][PL_FEATURE_COUNT],
                                  bb_t passer_set_out[2],
                                  passer_family_t* family_out,
                                  structure_family_t* structure_family_out)
{
  const bb_t white_pawns = board->bitboards[W_PAWN];
  const bb_t black_pawns = board->bitboards[B_PAWN];

  // Four fills, each spent on more than one thing, which is the reason the
  // three terms share a function at all. Forward -- toward rank 8 for White,
  // toward rank 1 for Black -- gives the passed pawn spans and the "no
  // neighbour at or behind" half of backward. Backward gives doubled. The union
  // of the two is the set of files a side has a pawn on, which is what isolated
  // and both rook file features ask about. Computed as independent passes it is
  // eight fills instead of four; the same fusion argument that recovered 7.7 %
  // on mobility and king safety.
  const bb_t white_ahead = fill_toward_rank8(white_pawns);
  const bb_t white_behind = fill_toward_rank1(white_pawns);
  const bb_t black_ahead = fill_toward_rank1(black_pawns);
  const bb_t black_behind = fill_toward_rank8(black_pawns);

  // Every square on a file that side has a pawn on. Named because three
  // features read it: isolated widens it by a file, and the two rook file
  // features intersect it with the rooks as it stands.
  const bb_t white_pawn_files = white_ahead | white_behind;
  const bb_t black_pawn_files = black_ahead | black_behind;

  // Strictly ahead. The extra shift by eight is what makes the span exclusive,
  // which is what stops two pawns facing each other on the same file from each
  // deciding the other is behind it.
  const bb_t white_front = white_ahead >> 8;
  const bb_t black_front = black_ahead << 8;

  // The pawn attack sets, each built once and read by every pawn term that
  // needs it (S263). Until then backward took neighbour_files() of the forward
  // fill and of the shifted pawns, and passed pawns of the front span, so the
  // files beside the front span were built twice: four calls a side with
  // isolated's, three now.
  //
  //   beside       the squares level with each pawn on the two files next to it
  //   attacks      the squares the pawns attack: beside, one rank forward
  //   attack span  every square the pawns can ever attack while they advance:
  //                the files next to the front span
  //
  // Two identities make that exact, and both hold for any bitboard, the a and h
  // file edges included: a shift by a whole rank commutes with
  // neighbour_files(), because its file masks are the same on every rank, so
  // beside shifted forward is neighbour_files() of the shifted pawns; and
  // neighbour_files() of a union is the union of the two, since shifts and
  // masks distribute over it. test_evaluation "the shared pawn sets count what
  // the definitions say" holds the counts to a square-by-square reading.
  const bb_t white_beside = neighbour_files(white_pawns);
  const bb_t black_beside = neighbour_files(black_pawns);
  const bb_t white_attacks = white_beside >> 8;
  const bb_t black_attacks = black_beside << 8;
  const bb_t white_attack_span = neighbour_files(white_front);
  const bb_t black_attack_span = neighbour_files(black_front);

  // Every square an enemy pawn stands in the way of: the front span and the
  // attack span beside it. The comment over this function widens the pawns and
  // then fills; this fills and then widens, so that the attack span is a set of
  // its own for backward to read. The two orders are equal because a horizontal
  // shift and a vertical fill commute and the file masks are invariant under
  // the vertical one -- and that equality was checked by brute force over
  // 200000 random pawn placements rather than argued, the a and h file wrap
  // being exactly what the masks are there to stop.
  const bb_t white_stops = white_front | white_attack_span;
  const bb_t black_stops = black_front | black_attack_span;

  // Every passer, the own seventh rank included: S123's terms read those.
  const bb_t white_passers = white_pawns & ~black_stops;
  const bb_t black_passers = black_pawns & ~white_stops;

  // The buckets leave a pawn on its own seventh rank out, here and explicitly
  // rather than by a weight: it is a passer by definition, so its bucket was
  // exactly eight piece-square columns, and S134 moved that bucket's weight
  // into psqt[PAWN][8..15] (DEC-090). The table prices it. Left in, it would
  // compute bucket 5 and read past the end of the five-entry tables below.
  bb_t white_passed = white_passers & ~rank_masks[a7];
  bb_t black_passed = black_passers & ~rank_masks[a2];

  // A pawn is isolated when neither neighbouring file holds an own pawn at any
  // rank, so the file fill is the whole file set and the rank drops out.
  const bb_t white_isolated = white_pawns & ~neighbour_files(white_pawn_files);
  const bb_t black_isolated = black_pawns & ~neighbour_files(black_pawn_files);

  // white_behind shifted one further is every square with an own pawn strictly
  // ahead of it, so intersecting that with the pawns is the definition read
  // literally. The other reading -- pawns with an own pawn strictly behind
  // -- selects a different set of squares and the same number of them, n-1 per
  // file either way, so only the comment can tell the two apart and it does.
  const bb_t white_doubled = white_pawns & (white_behind << 8);
  const bb_t black_doubled = black_pawns & (black_behind >> 8);

  // Backward, both halves, and never an isolated pawn: one meets the neighbour
  // half trivially, and with its stop square attacked is isolated_backward's,
  // below (DEC-331 (3)). The neighbour test uses the *forward* fill: a square
  // is in neighbour_files(white_ahead) exactly when an own pawn on an adjacent
  // file stands at or behind it, since the fill of that pawn covers every
  // square from it forward. The forward fill is the pawns and their front span,
  // so that set is beside and the attack span together, both built above. The
  // stop-square test shifts the enemy attack set back by one rank instead of
  // the pawns forward by one, so the pawn's own bit is what is tested and no
  // second shift is needed to get back.
  const bb_t white_backward = white_pawns & ~white_isolated &
                              ~(white_beside | white_attack_span) &
                              (black_attacks << 8);
  const bb_t black_backward = black_pawns & ~black_isolated &
                              ~(black_beside | black_attack_span) &
                              (white_attacks >> 8);

  score_t sum = 0;

  const int structure[2][PS_FEATURE_COUNT] = {
      {count_bits(white_isolated), count_bits(white_doubled),
       count_bits(white_backward)},
      {count_bits(black_isolated), count_bits(black_doubled),
       count_bits(black_backward)}};

  for (int f = 0; f < PS_FEATURE_COUNT; ++f) {
    const int difference = structure[WHITE][f] - structure[BLACK][f];

    sum += difference * pawn_structure_score[f];
    EVAL_TRACE(TERM_PAWN_STRUCTURE, f, difference);
  }

  int placement[2][PL_FEATURE_COUNT];
  piece_placement_features(board, white_pawn_files, black_pawn_files,
                           placement);

  add_placement_columns(placement, &sum,
                        std::make_integer_sequence<int, PL_FEATURE_COUNT>{});

  if constexpr (collect) {
    for (int colour = 0; colour < 2; ++colour) {
      for (int f = 0; f < PS_FEATURE_COUNT; ++f) {
        structure_out[colour][f] = structure[colour][f];
      }

      for (int f = 0; f < PL_FEATURE_COUNT; ++f) {
        placement_out[colour][f] = placement[colour][f];
      }

      for (int bucket = 0; bucket < 5; ++bucket) {
        passed_out[colour][bucket] = 0;
      }
    }

    passer_set_out[WHITE] = white_passers;
    passer_set_out[BLACK] = black_passers;
  }

  while (white_passed) {
    // Rank 2 is squares 48..55 and rank 6 is 16..23, so this runs 0 to 4 up the
    // board. What keeps it in range is that a pawn cannot stand on rank 1 or
    // rank 8 and that rank 7 was masked out above; the debug build checks that
    // instead of trusting it, because the penalty for being wrong is a read off
    // the end of a five-entry table.
    const int bucket = 6 - (get_lsb_index(white_passed) >> 3);
    white_passed &= white_passed - 1;

    assert(bucket >= 0 && bucket < 5);

    sum += passed_pawn_score[bucket];
    EVAL_TRACE(TERM_PASSED_PAWN, bucket, 1);

    if constexpr (collect) { passed_out[WHITE][bucket]++; }
  }

  while (black_passed) {
    // Mirrored: Black's own second rank is rank 7, squares 8..15.
    const int bucket = (get_lsb_index(black_passed) >> 3) - 1;
    black_passed &= black_passed - 1;

    assert(bucket >= 0 && bucket < 5);

    sum -= passed_pawn_score[bucket];
    EVAL_TRACE(TERM_PASSED_PAWN, bucket, -1);

    // Black's own row counts Black's passers as positives. The minus above is
    // the score's sign and not the count's, and folding the two together is
    // what would make a row mean two different things.
    if constexpr (collect) { passed_out[BLACK][bucket]++; }
  }

  // S123's family. A group is computed where its weights are live, in the
  // trace build, and when collecting, and nowhere else (passer_family_side()
  // says why that matters for A).
  constexpr bool advance = collect || trace_build || passer_advance_live;
  constexpr bool kings = collect || trace_build || passer_kings_live;
  constexpr bool candidates = collect || trace_build || candidate_passer_live;

  if constexpr (collect) { *family_out = passer_family_t{}; }

  if constexpr (advance || kings) {
    passer_family_side<WHITE, advance, kings, collect>(board, white_passers,
                                                       &sum, family_out);
    passer_family_side<BLACK, advance, kings, collect>(board, black_passers,
                                                       &sum, family_out);
  }

  if constexpr (candidates) {
    bb_t candidate[2];
    candidate_passers(white_pawns, black_pawns, white_passers, black_passers,
                      white_front, black_front, candidate);

    // A pawn on its own seventh is always passed, so a candidate's rank is
    // 0 to 4, the second to the sixth.
    while (candidate[WHITE]) {
      const int rank = 6 - (get_lsb_index(candidate[WHITE]) >> 3);
      candidate[WHITE] &= candidate[WHITE] - 1;
      assert(rank >= 0 && rank < 5);

      sum += candidate_passer_score[rank];
      EVAL_TRACE(TERM_CANDIDATE_PASSER, rank, 1);
      if constexpr (collect) { family_out->candidate[WHITE][rank]++; }
    }

    while (candidate[BLACK]) {
      const int rank = (get_lsb_index(candidate[BLACK]) >> 3) - 1;
      candidate[BLACK] &= candidate[BLACK] - 1;
      assert(rank >= 0 && rank < 5);

      sum -= candidate_passer_score[rank];
      EVAL_TRACE(TERM_CANDIDATE_PASSER, rank, -1);
      if constexpr (collect) { family_out->candidate[BLACK][rank]++; }
    }
  }

  // S125's family (DEC-331), after S123's: seven groups over the sets above,
  // each computed where its weights are live, in the trace build, and when
  // collecting, and nowhere else -- at zero a group is in no expression, as
  // S123's are. evaluation.hpp defines every set and cell.
  constexpr bool isolated_backward_on =
      collect || trace_build || isolated_backward_live;
  constexpr bool isolated_file_on =
      collect || trace_build || isolated_file_live;
  constexpr bool doubled_file_on = collect || trace_build || doubled_file_live;
  constexpr bool backward_rank_on =
      collect || trace_build || backward_rank_live;
  constexpr bool phalanx_on = collect || trace_build || phalanx_live;
  constexpr bool supported_on = collect || trace_build || supported_live;
  constexpr bool weak_unopposed_on =
      collect || trace_build || weak_unopposed_live;

  if constexpr (collect) { *structure_family_out = structure_family_t{}; }

  // An isolated pawn whose stop square an enemy pawn attacks: backward's
  // second clause, read from the same shifted attack set backward reads, on the
  // pawns backward leaves out.
  if constexpr (isolated_backward_on) {
    const bb_t white_ib = white_isolated & (black_attacks << 8);
    const bb_t black_ib = black_isolated & (white_attacks >> 8);
    const int difference = count_bits(white_ib) - count_bits(black_ib);

    sum += difference * isolated_backward_score[0];
    EVAL_TRACE(TERM_ISOLATED_BACKWARD, 0, difference);

    if constexpr (collect) {
      structure_family_out->isolated_backward_set[WHITE] = white_ib;
      structure_family_out->isolated_backward_set[BLACK] = black_ib;
      structure_family_out->isolated_backward[WHITE][0] = count_bits(white_ib);
      structure_family_out->isolated_backward[BLACK][0] = count_bits(black_ib);
    }
  }

  // A table by file takes the pawn's file less one, the a file the baseline;
  // a table by rank, its own rank less the table's first.
  if constexpr (isolated_file_on) {
    const auto file_cell = [](int square) { return (square & 7) - 1; };

    for_each_cell(white_isolated, file_cell, [&](int c) {
      sum += isolated_file_score[c];
      EVAL_TRACE(TERM_ISOLATED_FILE, c, 1);
      if constexpr (collect) {
        structure_family_out->isolated_file[WHITE][c]++;
      }
    });
    for_each_cell(black_isolated, file_cell, [&](int c) {
      sum -= isolated_file_score[c];
      EVAL_TRACE(TERM_ISOLATED_FILE, c, -1);
      if constexpr (collect) {
        structure_family_out->isolated_file[BLACK][c]++;
      }
    });
  }

  if constexpr (doubled_file_on) {
    const auto file_cell = [](int square) { return (square & 7) - 1; };

    for_each_cell(white_doubled, file_cell, [&](int c) {
      sum += doubled_file_score[c];
      EVAL_TRACE(TERM_DOUBLED_FILE, c, 1);
      if constexpr (collect) { structure_family_out->doubled_file[WHITE][c]++; }
    });
    for_each_cell(black_doubled, file_cell, [&](int c) {
      sum -= doubled_file_score[c];
      EVAL_TRACE(TERM_DOUBLED_FILE, c, -1);
      if constexpr (collect) { structure_family_out->doubled_file[BLACK][c]++; }
    });
  }

  // Backward pawns by own rank 3 to 5: rank 2 is the baseline, and no backward
  // pawn stands past its own fifth.
  if constexpr (backward_rank_on) {
    for_each_cell(
        white_backward, [](int square) { return own_rank<WHITE>(square) - 3; },
        [&](int c) {
          assert(c < 3);
          sum += backward_rank_score[c];
          EVAL_TRACE(TERM_BACKWARD_RANK, c, 1);
          if constexpr (collect) {
            structure_family_out->backward_rank[WHITE][c]++;
          }
        });
    for_each_cell(
        black_backward, [](int square) { return own_rank<BLACK>(square) - 3; },
        [&](int c) {
          assert(c < 3);
          sum -= backward_rank_score[c];
          EVAL_TRACE(TERM_BACKWARD_RANK, c, -1);
          if constexpr (collect) {
            structure_family_out->backward_rank[BLACK][c]++;
          }
        });

    if constexpr (collect) {
      structure_family_out->backward_only_set[WHITE] = white_backward;
      structure_family_out->backward_only_set[BLACK] = black_backward;
    }
  }

  // Own pawns level with an own pawn on a neighbouring file, by own rank 2
  // to 7: beside is exactly the squares level with one.
  if constexpr (phalanx_on) {
    const bb_t white_phalanx = white_pawns & white_beside;
    const bb_t black_phalanx = black_pawns & black_beside;

    for_each_cell(
        white_phalanx, [](int square) { return own_rank<WHITE>(square) - 2; },
        [&](int c) {
          assert(c < 6);
          sum += phalanx_score[c];
          EVAL_TRACE(TERM_PHALANX, c, 1);
          if constexpr (collect) { structure_family_out->phalanx[WHITE][c]++; }
        });
    for_each_cell(
        black_phalanx, [](int square) { return own_rank<BLACK>(square) - 2; },
        [&](int c) {
          assert(c < 6);
          sum -= phalanx_score[c];
          EVAL_TRACE(TERM_PHALANX, c, -1);
          if constexpr (collect) { structure_family_out->phalanx[BLACK][c]++; }
        });

    if constexpr (collect) {
      structure_family_out->phalanx_set[WHITE] = white_phalanx;
      structure_family_out->phalanx_set[BLACK] = black_phalanx;
    }
  }

  // Own pawns an own pawn attacks, by own rank 3 to 7.
  if constexpr (supported_on) {
    const bb_t white_supported = white_pawns & white_attacks;
    const bb_t black_supported = black_pawns & black_attacks;

    for_each_cell(
        white_supported, [](int square) { return own_rank<WHITE>(square) - 3; },
        [&](int c) {
          assert(c < 5);
          sum += supported_score[c];
          EVAL_TRACE(TERM_SUPPORTED, c, 1);
          if constexpr (collect) {
            structure_family_out->supported[WHITE][c]++;
          }
        });
    for_each_cell(
        black_supported, [](int square) { return own_rank<BLACK>(square) - 3; },
        [&](int c) {
          assert(c < 5);
          sum -= supported_score[c];
          EVAL_TRACE(TERM_SUPPORTED, c, -1);
          if constexpr (collect) {
            structure_family_out->supported[BLACK][c]++;
          }
        });

    if constexpr (collect) {
      structure_family_out->supported_set[WHITE] = white_supported;
      structure_family_out->supported_set[BLACK] = black_supported;
    }
  }

  // Weak: backward's first clause alone, no own pawn on a neighbouring file
  // level with the pawn or behind it. Open: no enemy pawn ahead on its file,
  // the other side's front span missing it, as candidate_passers() reads it.
  if constexpr (weak_unopposed_on) {
    const bb_t white_weak = white_pawns & ~(white_beside | white_attack_span);
    const bb_t black_weak = black_pawns & ~(black_beside | black_attack_span);
    const bb_t white_open = white_pawns & ~black_front;
    const bb_t black_open = black_pawns & ~white_front;
    const int difference = count_bits(white_weak & white_open) -
                           count_bits(black_weak & black_open);

    sum += difference * weak_unopposed_score[0];
    EVAL_TRACE(TERM_WEAK_UNOPPOSED, 0, difference);

    if constexpr (collect) {
      structure_family_out->weak_set[WHITE] = white_weak;
      structure_family_out->weak_set[BLACK] = black_weak;
      structure_family_out->open_set[WHITE] = white_open;
      structure_family_out->open_set[BLACK] = black_open;
      structure_family_out->weak_unopposed[WHITE][0] =
          count_bits(white_weak & white_open);
      structure_family_out->weak_unopposed[BLACK][0] =
          count_bits(black_weak & black_open);
    }
  }

  *score = sum;
}


// The counts on their own, weights and taper discarded. Only the test calls it,
// so the packed score it also computes is thrown away rather than split out: a
// second entry point into the same loop would be one more thing to keep in
// step.
void passed_pawn_counts(const board_t* board, int out[2][5])
{
  score_t score = 0;
  int structure[2][PS_FEATURE_COUNT];
  int placement[2][PL_FEATURE_COUNT];
  bb_t sets[2];
  passer_family_t family;
  structure_family_t structure_family;

  evaluate_pawns<true>(board, &score, out, structure, placement, sets, &family,
                       &structure_family);
}


// Another of the same call, and thrown away the same way. All three accessors
// go through the collecting instantiation, so the one evaluate_cheap() calls
// keeps exactly one caller and clang keeps inlining it -- handing a hot-path
// function a second caller is what cost 21 % once, see king_shelter_features()
// below.
void pawn_structure_counts(const board_t* board, int out[2][3])
{
  score_t score = 0;
  int passed[2][5];
  int placement[2][PL_FEATURE_COUNT];
  bb_t sets[2];
  passer_family_t family;
  structure_family_t structure_family;

  evaluate_pawns<true>(board, &score, passed, out, placement, sets, &family,
                       &structure_family);
}


// The third, on the same terms. Piece placement reads the bishop pair off
// pieces rather than pawns, and still comes through here: the file sets its
// rook features need are the pawn fills, and a separate entry point would be a
// second implementation free to drift from the one the search runs.
void piece_placement_counts(const board_t* board, int out[2][PL_FEATURE_COUNT])
{
  score_t score = 0;
  int passed[2][5];
  int structure[2][PS_FEATURE_COUNT];
  bb_t sets[2];
  passer_family_t family;
  structure_family_t structure_family;

  evaluate_pawns<true>(board, &score, passed, structure, out, sets, &family,
                       &structure_family);
}


// The passer sets, S123, through the same collecting call as the three above,
// so the set the test mirrors is the set the terms read and not a second
// computation of it.
void passed_pawn_sets(const board_t* board, bb_t out[2])
{
  score_t score = 0;
  int passed[2][5];
  int structure[2][PS_FEATURE_COUNT];
  int placement[2][PL_FEATURE_COUNT];
  passer_family_t family;
  structure_family_t structure_family;

  evaluate_pawns<true>(board, &score, passed, structure, placement, out,
                       &family, &structure_family);
}


// The family's counts, through the same collecting call once more.
void passer_family_counts(const board_t* board, passer_family_t* out)
{
  score_t score = 0;
  int passed[2][5];
  int structure[2][PS_FEATURE_COUNT];
  int placement[2][PL_FEATURE_COUNT];
  bb_t sets[2];
  structure_family_t structure_family;

  evaluate_pawns<true>(board, &score, passed, structure, placement, sets, out,
                       &structure_family);
}


// S125's family, sets and counts, through the same collecting call again, so
// the sets the mirror case reads are the sets the terms score.
void structure_family_counts(const board_t* board, structure_family_t* out)
{
  score_t score = 0;
  int passed[2][5];
  int structure[2][PS_FEATURE_COUNT];
  int placement[2][PL_FEATURE_COUNT];
  bb_t sets[2];
  passer_family_t family;

  evaluate_pawns<true>(board, &score, passed, structure, placement, sets,
                       &family, out);
}


// Zero, deliberately, and fitted by tools/tuner afterwards. Same staging as the
// four terms above and for the same reason: at zero the commit is provably
// behaviour-neutral, so the SPRT that follows measures the fitted term instead
// of a guess. No published value was consulted, DEC-016 -- and there is a
// well-known one for this term, which is exactly why the rule exists.
//
// The exact definition is in evaluation.hpp, with what checks it. The short
// version: it is added after evaluate_cheap() has resolved the sign, so it
// belongs to the side on move rather than to White.
//
// One parameter each and no feature vector, because the feature is the constant
// 1 for whoever is to move. In the tuner's White-relative model that is +1 with
// White to move and -1 with Black, which is the only place the term needs a
// sign at all.
// **Back at zero: the term measured nothing.** The fitted pair was mg 10, eg 0,
// from `tuner --only tempo` over 1490839 positions with the other 825 constants
// frozen, held-out error 0.106766 to 0.106723.
//
// Its SPRT ran the full 3000 games and reached neither bound -- LLR -1.46
// against a -2.20 boundary -- for -0.69 +/- 9.64 Elo. **That is unresolved and
// not H0 accepted**, and the difference matters: what the run establishes is
// that the term is smaller than this instrument can see, not that it is
// harmful. The interval is the tightest of S027's five terms and it is centred
// on zero.
//
// Kept inert rather than deleted because the cost of finding out again is one
// fit and one match, and the plumbing is the expensive part. Seven instructions
// when the weights are non-zero, six of which are the taper.
//
// If this is ever revisited it needs tighter bounds than `--fast` gives, and
// 3000 games at 10+0.2 is already three hours. A term this small is a thing to
// measure when there is a faster machine, not a thing to argue about.
//
// **S100 left this unresolved, which is the same verdict and more evidence.**
// The extraction, the gradient and the recovery are all clean -- `--only tempo`
// on a synthetic corpus gets a planted 23/12 back exactly -- and the corpus
// carries both sides to move, so there is nothing wrong with the machinery.
// What S100 added is the size of the signal. Tempo's entire contribution to a
// WDL label is the gap between the mean outcome of White-to-move and
// Black-to-move rows, and over 10795695 rows that gap is **0.007878** (0.556559
// against 0.548682). At the corpus mean and K = 0.7624 the sigmoid slope is
// 0.00108502 per centipawn, so the gap is a 7.26 cp score difference between
// the groups and a tempo weight near 3.6 cp. Confounded -- the two groups
// differ in more than whose move it is, and datagen skips in-check positions,
// which removes rows correlated with the mover standing worse -- so it is the
// size of what is available, not a prediction. Against it, S027's --only fit
// said mg 10 and the unfrozen S065 fit said mg 39: a fourfold disagreement on a
// few centipawns of signal. Re-measuring this needs a WDL-heavy label and
// bounds that can resolve single digits, not another 3000 games at --fast.
constexpr int tempo_mg = 0;
constexpr int tempo_eg = 0;
constexpr score_t tempo_score = make_score(tempo_mg, tempo_eg);


#ifdef CHESSO_TRACE
constinit thread_local eval_trace::record_t* eval_trace::sink = nullptr;

// What make_move accumulated into board->material and board->psqt, written out
// as coefficients: one per piece against its value and its square, signed and
// mirrored for Black exactly as eval_add_piece() does it. The accumulators are
// sums over the pieces, so the trace build walks the pieces instead of reading
// the sums -- the recomputation the search was rid of at S014, and the reason
// this exists only where CHESSO_TRACE is defined. The king's value is 0 and is
// not a parameter: one king a side, so it would cancel whatever it was.
static void trace_accumulators(const board_t* board)
{
  if (eval_trace::sink == nullptr) { return; }

  for (int piece = W_PAWN; piece <= B_KING; ++piece) {
    const bool white = (piece < B_PAWN);
    const int type = white ? piece : piece - B_PAWN;
    const int sign = white ? 1 : -1;
    bb_t pieces = board->bitboards[piece];

    while (pieces) {
      const index_t square = get_lsb_index(pieces);
      pieces &= pieces - 1;

      const int table_square = white ? square : (square ^ 56);

      if (type != W_KING) { EVAL_TRACE(TERM_MATERIAL, type, sign); }
      EVAL_TRACE(TERM_PSQT, type * 64 + table_square, sign);
    }
  }
}

#define EVAL_TRACE_ACCUMULATORS(board) trace_accumulators(board)
#else
#define EVAL_TRACE_ACCUMULATORS(board) ((void)0)
#endif


// Stage one: what is cheap enough to pay at every node, including the nodes
// evaluate_lazy() takes the shortcut on.
//
// Most of it was already computed by make_move: the material balance, the two
// piece-square sums and the phase are maintained as pieces move rather than
// rebuilt from the bitboards, which is why this function stopped being 40% of
// the search.
//
// The pawn terms are the exception and the first here that are computed rather
// than accumulated. "Every evaluation term must be accumulated" was the rule
// and it is not the rule any more: a term that reads where the *other* side's
// pawns stand has no O(1) delta make_move could apply, so it cannot be
// accumulated at all and what is left to choose is the stage.
//
// They are here rather than behind the shortcut with mobility and king safety
// because evaluate_expensive() clamps stage two to +/-LAZY_EVAL_MARGIN. That
// clamp is what makes the shortcut sound, and tools/eval_spread already puts
// the stage-two correction at p99 128 and max 330 over 1490839 corpus
// positions, clamped on 0.365 % of them. A pawn one square from promotion
// swings further than the clamp would let through, so stage two would truncate
// the passed pawn term on exactly the positions it exists for. Pawn structure
// and piece placement are in the same call and therefore the same stage; they
// have no clamp argument of their own, they share the fills. Piece placement
// would be in stage one whatever the clamp did: it reads no occupancy and needs
// no attack set, so nothing about it belongs behind the shortcut. What it costs
// there is 3.1 to 4.0 % of a depth 14 search, measured at forced weights and
// written out over piece_placement_mg -- for the features it had before S135;
// the four S135 added read no occupancy either and are priced by S135's own
// forced-weight measurement.
//
// The accumulators are White relative, so the sum is negated once at the end
// for Black. Doing it here rather than at every call site is what keeps a later
// term from picking up the wrong sign.
//
// Tempo is here and not in evaluate() because evaluate() is not what the search
// calls: quiescence goes through evaluate_lazy(), which is built on this
// function and takes its shortcut on the cheap score alone. A bonus added in
// evaluate() would be invisible to every node the search actually evaluates.
// The lazy margin is unaffected -- it bounds the difference between the two
// stages, and this term is in the same stage on both sides of that difference.
int evaluate_cheap(const board_t* board)
{
  // Interpolate the two tables on how much material is left, so a term slides
  // from its middlegame value to its endgame one instead of jumping when some
  // arbitrary piece comes off.
  const int phase = game_phase(board);
  EVAL_TRACE_PHASE(phase);
  EVAL_TRACE_ACCUMULATORS(board);

  score_t pawns = 0;
  evaluate_pawns<false>(board, &pawns, nullptr, nullptr, nullptr, nullptr,
                        nullptr, nullptr);

  // Summed into the accumulated pair before the interpolation rather than
  // tapered on its own. One integer division instead of two, on a function that
  // costs 1.36 ns in total, and one truncation towards zero instead of two --
  // which keeps one more 23/24 out of the bound test_eval_trace holds the
  // tuner's smooth score to. Both are packed, so the sum is one add and taper()
  // takes the halves apart after it.
  const int positional = taper(board->psqt + pawns, phase);

  const int score = board->material + positional;

  // Tapered on its own rather than summed into the pair above, and that is not
  // a stylistic choice: everything above is White relative and is negated below
  // for Black, and a bonus for having the move must survive that negation
  // unchanged. Inside the pair it would be a bonus for White.
  //
  // Folding it in with the side-to-move sign would be arithmetically the same
  // score and one truncation cheaper -- the comment over the pair above is
  // exactly that argument, made for the pawn terms -- and it is not done here.
  // It would entangle this term's truncation with the board's: the two sides'
  // scores would then sum to trunc((P + T) / 24) - trunc((P - T) / 24), which
  // is not twice the bonus, so the one property this term can be tested on
  // stops being exactly true. See test_eval_model, "the move is worth the same
  // to either side".
  //
  // What that costs is a second truncating division, under one centipawn more
  // against the tuner's smooth score -- test_eval_trace's bound is 3 x 23/24
  // with tempo at zero and 4 x 23/24 fitted -- and seven instructions,
  // measured: evaluate_cheap() disassembles to 164 at these zero weights and
  // 171 with them forced, the extra six being the taper and only one of them
  // the add. At zero the compiler deletes all seven, DEC-047, which is why the
  // count is the measurement and no timing was run.
  const int tempo = taper(tempo_score, phase);

  // White relative like every other coefficient in the trace: the bonus goes
  // to the side to move, which from White's side is +1 or -1 of it.
  EVAL_TRACE(TERM_TEMPO, 0, (board->active_color == WHITE) ? 1 : -1);

  return ((board->active_color == WHITE) ? score : -score) + tempo;
}


// Mobility, and the reason this file now has two stages. It cannot be
// accumulated: it is a function of occupancy, not of a piece and its square, so
// moving any piece changes it for every slider whose ray crosses the from or
// the to square and there is no O(1) delta for make_move to apply. Measured at
// 56 ns per node in a real search, DEC-039, where the isolated benchmark said
// 14.6 -- the difference is cache misses in 2 MB of rook attack tables that a
// ten-position benchmark loop never pays.
//
// A table per piece type since S121, one cell per count the piece can have:
// a piece scores its own cell, not its count times a weight, so the value of
// a square need not be the same for the first square as for the twentieth.
// The widths are the board's (src/evaluation.hpp, MOBILITY_WIDTH).
//
// **Every cell is S121's screen's fit** (adocs/data/S121_screen.sh, DEC-326):
// `--only mobility` on S083's corpus with every other constant frozen, over
// the count the screen's survivors define -- enemy pawn attacks excluded and
// the pin rule (src/evaluation.hpp) -- and shipped on one SPRT's H1 at
// 8+0.08 (adocs/data/S121_sprt.log).
// The queen's cells 26 and 27 share cell 25's value in both halves, each
// under 1000 games of support (DEC-326 (5), adocs/data/S121_cells.txt). The
// fit started from the identity, each cell c times the linear weight it
// replaced, mg {1, 6, 8, 2} and eg {0, 5, 2, 4}: S083's fit (01222f3), our
// own constants, so the seed is DEC-134 form (b) and no engine's table is in
// it (DEC-084, DEC-105). The identity's packed sum was the packed sum count *
// weight was, bit for bit -- packing is exact under integer multiplication
// (data_structures.hpp) -- so the phase 1 landing scored every position as
// the linear term did (INV-6). The linear weights were fitted, not chosen:
// the hand-picked set before them measured
// -14.93 Elo and the fitted one +28.46 over 942 games, DEC-040. No published
// table was consulted, DEC-016.
//
// What a cell is worth when the fit has spoken is a residual over the
// piece-square table, not a valuation: every piece adds exactly one cell, so
// adding c to every cell of a type and taking c off every square of its
// tables is the same evaluation wherever the clamp does not bind -- eight
// directions the data cannot see, one per type and half (tools/feature_audit
// lists them). An `--only mobility` fit holds the tables, so it cannot walk
// them.
constexpr int mobility_knight_mg[9] = {-9, 10, 15, 15, 17, 20, 22, 29, 42};
constexpr int mobility_knight_eg[9] = {-46, -15, 3, 7, 8, 9, 8, 0, -19};
constexpr int mobility_bishop_mg[14] = {11, 20, 29, 38, 44, 48, 52,
                                        53, 59, 62, 68, 88, 70, 91};
constexpr int mobility_bishop_eg[14] = {-29, -17, -1, 8,  18, 26, 33,
                                        33,  35,  32, 32, 21, 35, 7};
constexpr int mobility_rook_mg[15] = {16, 27, 35, 42, 47, 55, 63, 70,
                                      75, 84, 88, 94, 97, 97, 99};
constexpr int mobility_rook_eg[15] = {-48, -8, 3,  9,  12, 13, 18, 17,
                                      22,  26, 27, 30, 32, 33, 25};
constexpr int mobility_queen_mg[28] = {
    -13, -13, -12, -9, -7, -6, -2, -1, 3,  6,  7,  9,  8,  7,
    10,  7,   8,   12, 19, 24, 46, 78, 92, 96, 39, 79, 79, 79};
constexpr int mobility_queen_eg[28] = {
    -57, 18,  11,  6,   38,  44, 49, 65, 76, 89, 89, 96,  102, 111,
    109, 110, 119, 117, 109, 94, 76, 48, 25, 8,  42, -22, -22, -22};

// The widths of the arrays above against the board's, so a cell added or lost
// in a paste is a build error and not a read past a row.
template <size_t N>
static constexpr int width_of(const int (&)[N])
{ return static_cast<int>(N); }

static_assert(width_of(mobility_knight_mg) == MOBILITY_WIDTH[0] &&
                  width_of(mobility_knight_eg) == MOBILITY_WIDTH[0] &&
                  width_of(mobility_bishop_mg) == MOBILITY_WIDTH[1] &&
                  width_of(mobility_bishop_eg) == MOBILITY_WIDTH[1] &&
                  width_of(mobility_rook_mg) == MOBILITY_WIDTH[2] &&
                  width_of(mobility_rook_eg) == MOBILITY_WIDTH[2] &&
                  width_of(mobility_queen_mg) == MOBILITY_WIDTH[3] &&
                  width_of(mobility_queen_eg) == MOBILITY_WIDTH[3],
              "a mobility table is not the width the board gives its piece");

// The four tables packed into one, the cells of type t at MOBILITY_OFFSET[t]
// onwards, so the loop below reads one cell with one load.
static constexpr std::array<score_t, MOBILITY_CELLS> pack_mobility()
{
  std::array<score_t, MOBILITY_CELLS> packed{};
  const int* const mg[4] = {mobility_knight_mg, mobility_bishop_mg,
                            mobility_rook_mg, mobility_queen_mg};
  const int* const eg[4] = {mobility_knight_eg, mobility_bishop_eg,
                            mobility_rook_eg, mobility_queen_eg};

  for (int type = 0; type < 4; ++type) {
    for (int c = 0; c < MOBILITY_WIDTH[type]; ++c) {
      packed[MOBILITY_OFFSET[type] + c] = make_score(mg[type][c], eg[type][c]);
    }
  }

  return packed;
}

constexpr std::array<score_t, MOBILITY_CELLS> mobility_score = pack_mobility();


// The four types every stage-two term iterates over, in the order the weight
// arrays are indexed. Shared so that mobility and king safety cannot disagree
// about which weight belongs to which piece.
static bb_t piece_attacks(const bb_tables_t* tables,
                          int type,
                          index_t square,
                          bb_t occupancy)
{
  switch (type) {
    case 0:
      return tables->knight_attacks[square];
    case 1:
      return get_bishop_attacks(tables, square, occupancy);
    case 2:
      return get_rook_attacks(tables, square, occupancy);
    default:
      return get_queen_attacks(tables, square, occupancy);
  }
}


// King safety, as nine counts per side scored linearly. Linear is a hard
// constraint rather than a preference: the tuner fits a linear function of its
// parameters, so the attack_table[weighted_attacker_count] curve the literature
// describes cannot be fitted here and is not used. DEC-016 rules out taking
// anyone else's table for it in any case.
//
// Fitted, never guessed. tools/tuner over 1490839 self-play positions with the
// other 781 constants frozen -- `--only king_safety`, which exists so that this
// change is one change and its SPRT measures one thing. Held-out error 0.107413
// to 0.107109. Fitting all 799 jointly reached 0.106964 instead, so the refit
// of the rest is worth a further 0.000145 and is a separate change if it is
// worth anything at all.
//
// Do not read a sign here as a chess statement. The four attacker counts and
// the incidence count are collinear by construction -- a piece that adds one to
// a count adds several to the incidence total -- so the fit splits one effect
// across several parameters, exactly the way piece_value and the piece-square
// tables are degenerate by five dimensions. What is fitted is the sum. DEC-044.
constexpr int king_safety_mg[KS_FEATURE_COUNT] = {18, 13, 5,   30, -27,
                                                  16, 7,  -47, -25};
constexpr int king_safety_eg[KS_FEATURE_COUNT] = {-5, -9, 7,  -52, 10,
                                                  -8, -1, -4, 19};
constexpr std::array<score_t, KS_FEATURE_COUNT> king_safety_score =
    pack_scores(king_safety_mg, king_safety_eg);

// Stage two's two packed sums, bounded the way stage one's is: for mobility,
// 32 pieces each adding one cell (S121; 32 x 27 x its weight while it was
// linear); for king safety, 16 attackers a side each adding one attacker
// count and at most nine zone incidences, plus four shelter features a side
// that count files and pawns around the king and stay under 8.
static_assert(
    32 * std::max({largest_weight(mobility_knight_mg, mobility_knight_eg),
                   largest_weight(mobility_bishop_mg, mobility_bishop_eg),
                   largest_weight(mobility_rook_mg, mobility_rook_eg),
                   largest_weight(mobility_queen_mg, mobility_queen_eg)}) <=
        32767,
    "mobility's packed sum can overflow a 16-bit half");
static_assert(2 * (16 * (1 + 9) + 4 * 8) *
                      largest_weight(king_safety_mg, king_safety_eg) <=
                  32767,
              "king safety's packed sum can overflow a 16-bit half");


// A king's own zone: its square and the eight around it.
//
// This returned an empty zone on a kingless board until S213. S223 made
// load_FEN refuse a board without a king of each colour, so the branch was
// unreachable from any accepted position, and it is the fourth of the group
// S223 converted -- is_check(), generate_moves_impl() and
// king_shelter_features() are the three it named. Same reason as those:
// get_lsb_index() answers 64 on an empty board, and 64 indexes king_attacks
// and every other table here off its end.
static bb_t king_zone(const bb_tables_t* tables,
                      const board_t* board,
                      int colour)
{
  const bb_t king = board->bitboards[(colour == WHITE) ? W_KING : B_KING];

  assert(king != 0);

  const index_t square = get_lsb_index(king);

  return tables->king_attacks[square] | (BB_1 << square);
}


// The four features that read only pawns and the king square. They are the ones
// that do not need a single attack set, which is why they sit outside the piece
// loop rather than inside it. Indices 0 to 4 are counted there.
//
// `inline` is load-bearing and was measured, not assumed. Collecting the counts
// for the test gave this function a second caller, clang stopped inlining it at
// the first, and evaluate() went from 18.60 to 22.55 ns per call in bench_eval
// -- 21 % for a term that is supposed to be free. Inlined, the zero weights
// downstream fold the whole thing away again and the number comes back to
// 18.54.
static inline void king_shelter_features(const board_t* board,
                                         int colour,
                                         int features[KS_FEATURE_COUNT])
{
  for (int f = 0; f < KS_FEATURE_COUNT; ++f) {
    features[f] = 0;
  }

  const bb_t king = board->bitboards[(colour == WHITE) ? W_KING : B_KING];

  // Until S223 this returned early on a kingless board, which load_FEN() let
  // through. It refuses one now, so the early return is an assertion:
  // get_lsb_index() answers 64 on an empty board and every index below is
  // derived from it.
  assert(king != 0);

  const index_t king_square = get_lsb_index(king);

  const bb_t own_pawns = board->bitboards[(colour == WHITE) ? W_PAWN : B_PAWN];
  const bb_t enemy_pawns =
      board->bitboards[(colour == WHITE) ? B_PAWN : W_PAWN];

  // The king file and its neighbours, already clipped at the a and h files by
  // the way isolated_file_masks is built.
  const bb_t files = file_masks[king_square] | isolated_file_masks[king_square];

  // Toward the enemy. Index 0 is a8, so White advances by subtracting.
  const int step = (colour == WHITE) ? -8 : 8;
  const int near_square = king_square + step;
  const int far_square = near_square + step;

  // A king on the last rank has nothing in front of it to shelter behind, and
  // one on the seventh has only the near rank.
  if (near_square >= 0 && near_square < 64) {
    features[KS_SHIELD_NEAR] =
        count_bits(own_pawns & files & rank_masks[near_square]);
  }

  if (far_square >= 0 && far_square < 64) {
    features[KS_SHIELD_FAR] =
        count_bits(own_pawns & files & rank_masks[far_square]);
  }

  const int king_file = king_square & 7;

  for (int file = king_file - 1; file <= king_file + 1; ++file) {
    if (file < 0 || file > 7) { continue; }

    // file_masks is indexed by square; its first eight entries are the files.
    const bb_t mask = file_masks[file];

    if ((own_pawns & mask) != 0) { continue; }

    if ((enemy_pawns & mask) != 0) {
      features[KS_HALF_OPEN_FILE]++;
    } else {
      features[KS_OPEN_FILE]++;
    }
  }
}


// Mobility and king safety in one pass, White relative. The two terms ask the
// same question of every piece -- where does it attack -- and asking it twice
// cost 7.7 % of the search at depth 12 with king safety weighted at zero, since
// the attack set is a magic-table lookup and the arithmetic on it is not.
//
// They are still tapered and summed as two separate terms below, so the fusion
// is exact rather than nearly exact: the score is identical to the unfused form
// for any weights, not only for the zeros king safety currently ships.
//
// The piece's own attack set is the only one either term reads, and it is
// looked up once here and read by both. No per-side union -- attacks by piece
// type, all attacks, squares attacked twice -- is built, because no term reads
// one yet: each would be paid at every node for nothing (S263). The first term
// that reads one builds it in this loop and its measurement pays for it.
//
// The sign is the part to read carefully. Mobility belongs to the piece that
// has it, an attack on a king zone belongs to the king being attacked, and
// those are opposite sides for the same piece -- so one loop carries both
// signs and neither is the loop's own colour by default.
//
// `collect` is how king_safety_features() reads the nine counts out without the
// search paying for it. A template rather than a flag: at <false> the stores
// and the out pointer compile away entirely, so the inner loop of the term this
// fusion exists to make free does not acquire a branch. The test gets the
// counts from this code and not from a copy of it, which is the point -- a
// second extraction would be free to agree with the model and disagree with the
// engine.
//
// The two tapered terms are handed back separately through `mobility_out` and
// `safety_out` for the same reason and under the same `if constexpr`: at
// <false> the pointers and the stores are not compiled at all.
template <bool collect>
static int evaluate_mobility_and_king_safety(const board_t* board,
                                             int out[2][KS_FEATURE_COUNT],
                                             int* mobility_out = nullptr,
                                             int* safety_out = nullptr)
{
  const bb_tables_t* tables = game_tables();
  const bb_t occupancy = board->occupancies[BOTH];

  const bb_t zone[2] = {king_zone(tables, board, WHITE),
                        king_zone(tables, board, BLACK)};

  score_t mobility_sum = 0;
  score_t safety_sum = 0;

  if constexpr (collect) {
    for (int colour = 0; colour < 2; ++colour) {
      for (int f = 0; f < KS_FEATURE_COUNT; ++f) {
        out[colour][f] = 0;
      }
    }
  }

  // What S121's count reads beyond the attack set (src/evaluation.hpp): the
  // squares each side's pawns attack, stage one's own sets built again here
  // because stage one keeps them local (specs.md), two shifts a side; and each
  // side's pieces pinned to its own king, both colours, which the generator's
  // side-to-move masks are not.
  const bb_t pawn_attacks[2] = {neighbour_files(board->bitboards[W_PAWN]) >> 8,
                                neighbour_files(board->bitboards[B_PAWN]) << 8};
  const bb_t pinned[2] = {pinned_pieces(tables, board, WHITE),
                          pinned_pieces(tables, board, BLACK)};
  const index_t king_square[2] = {get_lsb_index(board->bitboards[W_KING]),
                                  get_lsb_index(board->bitboards[B_KING])};

  for (int colour = 0; colour < 2; ++colour) {
    // The piece's own side, which is what mobility is worth to.
    const int mover = (colour == WHITE) ? 1 : -1;

    // The side whose king these pieces are attacking, which is the other one.
    const int defender = -mover;

    const bb_t own = board->occupancies[colour];
    const bb_t defended_zone = zone[1 - colour];
    const int base = (colour == WHITE) ? W_KNIGHT : B_KNIGHT;

    // The squares the count reads: none holding an own piece, and none an
    // enemy pawn attacks (P1).
    const bb_t area = ~own & ~pawn_attacks[1 - colour];

    for (int type = 0; type < 4; ++type) {
      bb_t pieces = board->bitboards[base + type];

      while (pieces) {
        const index_t square = get_lsb_index(pieces);
        pieces &= pieces - 1;

        const bb_t attacks = piece_attacks(tables, type, square, occupancy);

        // The count is `area`'s, above, and for a pinned piece only the
        // squares on its pin line (P3): a pinned knight has none, a piece
        // pinned across its own lines none either. Never full safe mobility,
        // which would exclude every attacked square. The count picks the
        // piece's cell, which the board keeps inside its row: a knight
        // reaches 8 squares at most, a bishop 13, a rook 14, a queen 27
        // (MOBILITY_WIDTH).
        bb_t moves = attacks;
        if ((pinned[colour] >> square) & 1) {
          moves &= tables->line[king_square[colour]][square];
        }

        const int count = count_bits(moves & area);
        assert(count < MOBILITY_WIDTH[type]);

        const int cell = MOBILITY_OFFSET[type] + count;

        mobility_sum += mover * mobility_score[cell];
        EVAL_TRACE(TERM_MOBILITY, cell, mover);

        const bb_t hits = attacks & defended_zone;

        if (hits == 0) { continue; }

        const int attacker = KS_KNIGHT_ATTACKERS + type;

        safety_sum += defender * king_safety_score[attacker];
        EVAL_TRACE(TERM_KING_SAFETY, attacker, defender);

        // Attacker-square incidences, not distinct squares under fire: two
        // pieces bearing on the same square count twice. Distinct squares
        // saturate at nine and stop separating one attacker from four, which is
        // the whole thing a linear model has to read off this number.
        const int incidences = count_bits(hits);

        safety_sum +=
            defender * incidences * king_safety_score[KS_ZONE_ATTACKS];
        EVAL_TRACE(TERM_KING_SAFETY, KS_ZONE_ATTACKS, defender * incidences);

        // The count belongs to the king under fire, which is the row of the
        // colour that is not moving these pieces.
        if constexpr (collect) {
          out[1 - colour][attacker]++;
          out[1 - colour][KS_ZONE_ATTACKS] += incidences;
        }
      }
    }
  }

  for (int colour = 0; colour < 2; ++colour) {
    const int sign = (colour == WHITE) ? 1 : -1;

    int features[KS_FEATURE_COUNT];
    king_shelter_features(board, colour, features);

    for (int f = KS_SHIELD_NEAR; f <= KS_HALF_OPEN_FILE; ++f) {
      safety_sum += sign * features[f] * king_safety_score[f];
      EVAL_TRACE(TERM_KING_SAFETY, f, sign * features[f]);

      if constexpr (collect) { out[colour][f] = features[f]; }
    }
  }

  const int phase = game_phase(board);

  // Tapered one term at a time, two truncating divisions and not one. Merging
  // them is a play change and was measured as one: S055's single taper read H0
  // at nElo -8.78 +/- 6.39 and was reverted. The packing keeps both
  // truncations exactly, which is what makes it behaviour-neutral (S117).
  const int mobility = taper(mobility_sum, phase);
  const int safety = taper(safety_sum, phase);

  if constexpr (collect) {
    if (mobility_out != nullptr) { *mobility_out = mobility; }
    if (safety_out != nullptr) { *safety_out = safety; }
  }

  return mobility + safety;
}


// Everything the lazy shortcut skips, and the one place its margin is enforced.
// One clamp over the sum rather than one per term: two terms each bounded by
// LAZY_EVAL_MARGIN can correct by twice it between them, and the shortcut is
// unsound the moment that happens.
//
// Clamped at all so that the margin is a guarantee rather than a hope. Without
// it the bound is an observation about the positions someone sampled, and a
// position outside the sample silently breaks the shortcut -- which is not
// hypothetical: the corpus contains 1Bk5/B1B5/1B1B4/B1B4B/8/B1B5/5K2/8, nine
// white bishops from promotions, where mobility alone reached 155 against the
// margin of 150 that shipped then, and the test caught it. S085 raised the
// margin to 184, so that position no longer binds and the incident is history
// rather than a live demonstration -- the argument it supports is unchanged,
// because whether the clamp is a guarantee or a hope does not depend on which
// sampled position last exceeded it.
//
// It costs nothing in real play. Over 149084 positions of S028 self-play
// mobility ran p99 81 and a maximum of 143, so the clamp does not bind there at
// all; it binds only in the promotion pile-ups where the number was never
// meaningful anyway.
static int evaluate_expensive(const board_t* board)
{
  const int score = evaluate_mobility_and_king_safety<false>(board, nullptr);
  const int bounded = std::clamp(score, -LAZY_EVAL_MARGIN, LAZY_EVAL_MARGIN);

  return (board->active_color == WHITE) ? bounded : -bounded;
}


int evaluate(const board_t* board)
{ return evaluate_cheap(board) + evaluate_expensive(board); }


// The counts on their own, weights and taper discarded. Only the test calls it,
// so the score it also computes is thrown away rather than split out: a second
// entry point into the same loop would be one more thing to keep in step.
void king_safety_features(const board_t* board, int out[2][KS_FEATURE_COUNT])
{ evaluate_mobility_and_king_safety<true>(board, out); }


// What evaluate_expensive() computed before the clamp took it away. The counts
// it also collects are thrown away here, for the reason above: the collecting
// instantiation is the one the search does not compile, so a tool riding on it
// cannot slow the search down whatever clang decides to inline.
void evaluate_expensive_terms(const board_t* board, int* mobility, int* safety)
{
  int counts[2][KS_FEATURE_COUNT];
  int mobility_white = 0;
  int safety_white = 0;

  evaluate_mobility_and_king_safety<true>(board, counts, &mobility_white,
                                          &safety_white);

  // The same sign evaluate_expensive() applies, so the sum is the correction as
  // the caller would have received it and not a White-relative number that has
  // to be turned round again. The clamp is symmetric, so applying the sign
  // before it rather than after changes nothing.
  const int sign = (board->active_color == WHITE) ? 1 : -1;

  *mobility = sign * mobility_white;
  *safety = sign * safety_white;
}


int evaluate_lazy(const board_t* board, int alpha, int beta, bool* exact)
{
  const int cheap = evaluate_cheap(board);

  // Set on both shortcut paths before either returns, so a caller reading it
  // after a bound sees false rather than whatever it initialised.
  if (exact != nullptr) { *exact = false; }

  // Both tests are one-sided on purpose. If the cheap score is already a
  // margin clear of beta then the full score is above beta too, so the caller
  // fails high on either number and the expensive stage would change nothing it
  // does. Same argument mirrored at alpha. Anywhere between the two, the
  // correction can decide the node and has to be computed.
  if (cheap - LAZY_EVAL_MARGIN >= beta) { return cheap - LAZY_EVAL_MARGIN; }
  if (cheap + LAZY_EVAL_MARGIN <= alpha) { return cheap + LAZY_EVAL_MARGIN; }

  if (exact != nullptr) { *exact = true; }

  // Identical to evaluate() by construction: that function is these two terms
  // added, so the number kept here is the static evaluation and not an
  // approximation of it.
  return cheap + evaluate_expensive(board);
}


int game_phase(const board_t* board)
{
  // Promotions can put more material on the board than the opening had.
  return (board->phase > GAME_PHASE_MAX) ? GAME_PHASE_MAX : board->phase;
}


inline piece_t captured_piece(const board_t* board, index_t square)
{
  assert(square < 64);
  return board->squares[square];
}


int capture_score(const board_t* board, move_t move)
{
  // An en-passant victim does not sit on the target square
  const piece_t victim =
      MOVE_EN_PASSANT(move) ? ((board->active_color == WHITE) ? B_PAWN : W_PAWN)
                            : captured_piece(board, MOVE_TO(move));

  // MVV-LVA
  assert(victim < sizeof(piece_values_abs) / sizeof(piece_values_abs[0]));
  return piece_values_abs[victim] - piece_values_abs[MOVE_PIECE(move)];
}


int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move)
{
  if (tt_move != 0 && move == tt_move) { return ORDER_TT_MOVE; }

  assert(static_cast<size_t>(MOVE_PROMOTED(move)) <
         sizeof(piece_values_abs) / sizeof(piece_values_abs[0]));

  // Promotion_t maps onto W_KNIGHT..W_QUEEN by construction.
  const int promotion_bonus =
      MOVE_PROMOTED(move) ? piece_values_abs[MOVE_PROMOTED(move)] : 0;

  if (MOVE_CAPTURE(move)) {
    return ORDER_CAPTURE + promotion_bonus + capture_score(&game->board, move);
  }

  if (promotion_bonus != 0) { return ORDER_CAPTURE + promotion_bonus; }

  if (move == state->killer_moves[0][ply]) { return ORDER_KILLER_0; }
  if (move == state->killer_moves[1][ply]) { return ORDER_KILLER_1; }

  if (prev_move != 0 &&
      move == state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]) {
    return ORDER_COUNTER;
  }

  // The quiet ordering band, and it is the raw history sum, factored out into
  // src/evaluation.hpp so that this is not the only place that arithmetic
  // exists. S098 read it there too and its term measured zero and left
  // (DEC-213); S236 read it again, as a fraction of a ply this time, and its
  // two verdicts read a walk and then a zero, so that term left as well
  // (DEC-231). The factoring stayed through both, because it is
  // behaviour-neutral and because the next consumer of a move's history should
  // read the number this function returns rather than write its own copy.
  return quiet_history_sum(game, state, move, prev_move);
}
