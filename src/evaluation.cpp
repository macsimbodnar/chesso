#include "evaluation.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "eval_tables.hpp"
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

// Fitted with the other 799 constants frozen -- `tuner --only passed_pawns`
// over 1490839 self-play positions, held-out error 0.107106 to 0.106900.
//
// **These are a residual, not a valuation.** A frozen fit prices only what the
// term adds on top of what is already there, and psqt_mg's pawn table already
// pays 79 to 214 for a pawn on the seventh rank. That is why the buckets are
// not monotonic and why the last one, a pawn one square from promotion, fits to
// almost nothing: the piece-square table has already paid for it. Reading
// bucket 5 as "a passed pawn on the seventh is worth 2 centipawns" is the same
// mistake as reading a pawn's fitted value of 83 as what a pawn is worth.
// Knight mobility fitted to nothing against this same effect at S034, DEC-040,
// and DEC-044 records it for king safety's attacker counts.
//
// The term costs 4.5 % of a depth 12 search, measured with the weights forced
// non-zero because at zero the compiler deletes it, DEC-047. Its SPRT was net
// of that: +17.34 +/- 13.51 Elo over 1584 games, H1 accepted.
const int passed_pawn_mg[6] = {3, -16, 1, 10, 30, 2};
const int passed_pawn_eg[6] = {-15, -4, 18, 28, 25, 8};


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


// Zero, deliberately, and fitted by tools/tuner afterwards. Same staging as
// passed pawns above and for the same reason: at zero the commit is provably
// behaviour-neutral, so the SPRT that follows measures the fitted term instead
// of a guess. No published table was consulted, DEC-016.
//
// The exact definition of each of the three is in evaluation.hpp, written out
// there because tools/eval_model.hpp is built from it and the two have to mean
// the same thing. What is here is why the arithmetic below computes them the
// way it does.
//
// **Nothing measured at these weights says anything about what they cost** --
// clang sees the zeros and deletes the two extra fills, the popcounts and the
// arithmetic that feed them, DEC-047. evaluate_cheap() disassembles to the same
// 87 instructions with the term as without it, and to 166 with the weights
// forced. Every "this term is free" figure taken at zero is a figure about a
// build that does not contain it.
//
// Forced non-zero with the passed pawn term already live, so the figure is the
// marginal cost of adding this one on top of that one and not the cost of the
// pair: **4.1 % of a depth 14 search**, 5.590 s to 5.820 s, 40 hyperfine runs
// per build with per-block sigma 0.9 to 1.5 %, the two run orders bracketing
// 3.7 % and 4.5 %. The costing build computes the term at forced weights and
// throws it away behind an asm barrier so the score is unchanged, which is what
// makes the two node-for-node identical -- 32643888 nodes either way -- and the
// wall time the cost and nothing else.
//
// The barrier is not what is being measured: the same build with the barrier
// moved onto the passed pawn sums and these weights back at zero ran 5.584 s
// against 5.589 s for b5fb5ff with no barrier at all, a difference of 0.1 %
// inside a sigma of 1.0 %.
//
// Whoever fits these weights pays that 4.1 % in the same commit. DEC-040 is the
// warning: mobility measured -14.93 Elo the one time a term was judged while
// its speed cost was still unpaid.
const int pawn_structure_mg[PS_FEATURE_COUNT] = {0, 0, 0};
const int pawn_structure_eg[PS_FEATURE_COUNT] = {0, 0, 0};


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


// The same squares plus the two neighbouring files. Split from the above
// because the pawn structure terms ask about the neighbours with the pawn's own
// file left out, and passed pawns asks about both together.
static inline bb_t widen_by_one_file(bb_t squares)
{ return squares | neighbour_files(squares); }


// Both pawn terms, passed pawns and pawn structure, White minus Black, as one
// untapered middlegame and endgame pair for the caller to interpolate.
//
// One function and not two because the two terms are the same four fills read
// four different ways, and a second pass over the pawns would recompute them.
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
// Six buckets by how far the pawn has advanced, not one weight multiplied by
// the rank. The value is not linear in the rank -- a pawn one square from
// queening is worth several times one on the third -- and the tuner fits a
// linear function of its parameters, so a non-linear shape has to sit in the
// features or it cannot be fitted here at all. Same constraint that made king
// safety linear, DEC-044.
//
// The bucket arithmetic is the part of this term that is easy to get backwards.
// Index 0 is a8 and index 63 is h1, so White advances by *decreasing* the
// index: square >> 3 is 6 on White's second rank and 1 on the seventh. Reversed
// it pays for retreating pawns, and while the weights are zero no test can say
// so, because every score it produces is the same score. What can say so is
// passed_pawn_counts() below, which is why `collect` is here. Pawn structure
// has the same hole and pawn_structure_counts() is the same answer to it.
//
// `collect` is how the two accessors read the counts out without the search
// paying for it. A template rather than a flag, the arrangement king safety
// arrived at: at <false> the stores and the out pointer are not compiled at
// all, and the instantiation the search calls keeps exactly one caller, so
// clang still inlines it and the zero weights still fold everything downstream
// away. Handing that instantiation a second caller instead is what cost 21 %
// once -- see king_shelter_features() below.
template <bool collect>
static inline void evaluate_pawns(const board_t* board,
                                  int* mg,
                                  int* eg,
                                  int passed_out[2][6],
                                  int structure_out[2][PS_FEATURE_COUNT])
{
  const bb_t white_pawns = board->bitboards[W_PAWN];
  const bb_t black_pawns = board->bitboards[B_PAWN];

  // Four fills, each spent on more than one thing, which is the reason the two
  // terms share a function at all. Forward -- toward rank 8 for White, toward
  // rank 1 for Black -- gives the passed pawn spans and the "no neighbour at or
  // behind" half of backward. Backward gives doubled. The union of the two is
  // the set of files a side has a pawn on, which is what isolated asks about.
  // Computed as two independent passes it is six fills instead of four -- two
  // for passed pawns and four for structure; the same fusion argument that
  // recovered 7.7 % on mobility and king safety.
  const bb_t white_ahead = fill_toward_rank8(white_pawns);
  const bb_t white_behind = fill_toward_rank1(white_pawns);
  const bb_t black_ahead = fill_toward_rank1(black_pawns);
  const bb_t black_behind = fill_toward_rank8(black_pawns);

  // Strictly ahead. The extra shift by eight is what makes the span exclusive,
  // which is what stops two pawns facing each other on the same file from each
  // deciding the other is behind it.
  const bb_t white_front = white_ahead >> 8;
  const bb_t black_front = black_ahead << 8;

  // Widening the span rather than the pawns, which is the order the passed pawn
  // term used before pawn structure needed the bare span. The two are equal
  // because a horizontal shift and a vertical fill commute and the file masks
  // are invariant under the vertical one -- and because equality was checked by
  // brute force over 200000 random pawn placements rather than argued, the a
  // and h file wrap being exactly what the masks are there to stop.
  const bb_t white_stops = widen_by_one_file(white_front);
  const bb_t black_stops = widen_by_one_file(black_front);

  bb_t white_passed = white_pawns & ~black_stops;
  bb_t black_passed = black_pawns & ~white_stops;

  // A pawn is isolated when neither neighbouring file holds an own pawn at any
  // rank, so the file fill is the whole file set and the rank drops out.
  const bb_t white_isolated =
      white_pawns & ~neighbour_files(white_ahead | white_behind);
  const bb_t black_isolated =
      black_pawns & ~neighbour_files(black_ahead | black_behind);

  // white_behind shifted one further is every square with an own pawn strictly
  // ahead of it, so intersecting that with the pawns is the definition read
  // literally. The other reading -- pawns with an own pawn strictly behind
  // -- selects a different set of squares and the same number of them, n-1 per
  // file either way, so only the comment can tell the two apart and it does.
  const bb_t white_doubled = white_pawns & (white_behind << 8);
  const bb_t black_doubled = black_pawns & (black_behind >> 8);

  // One step forward and one file each way: the squares a side's pawns attack.
  const bb_t white_attacks = neighbour_files(white_pawns >> 8);
  const bb_t black_attacks = neighbour_files(black_pawns << 8);

  // Backward, both halves. The neighbour test uses the *forward* fill: a square
  // is in neighbour_files(white_ahead) exactly when an own pawn on an adjacent
  // file stands at or behind it, since the fill of that pawn covers every
  // square from it forward. The stop-square test shifts the enemy attack set
  // back by one rank instead of the pawns forward by one, so the pawn's own bit
  // is what is tested and no second shift is needed to get back.
  const bb_t white_backward =
      white_pawns & ~neighbour_files(white_ahead) & (black_attacks << 8);
  const bb_t black_backward =
      black_pawns & ~neighbour_files(black_ahead) & (white_attacks >> 8);

  int mg_sum = 0;
  int eg_sum = 0;

  const int structure[2][PS_FEATURE_COUNT] = {
      {count_bits(white_isolated), count_bits(white_doubled),
       count_bits(white_backward)},
      {count_bits(black_isolated), count_bits(black_doubled),
       count_bits(black_backward)}};

  for (int f = 0; f < PS_FEATURE_COUNT; ++f) {
    const int difference = structure[WHITE][f] - structure[BLACK][f];

    mg_sum += difference * pawn_structure_mg[f];
    eg_sum += difference * pawn_structure_eg[f];
  }

  if constexpr (collect) {
    for (int colour = 0; colour < 2; ++colour) {
      for (int f = 0; f < PS_FEATURE_COUNT; ++f) {
        structure_out[colour][f] = structure[colour][f];
      }

      for (int bucket = 0; bucket < 6; ++bucket) {
        passed_out[colour][bucket] = 0;
      }
    }
  }

  while (white_passed) {
    // Rank 2 is squares 48..55 and rank 7 is 8..15, so this runs 0 to 5 up the
    // board. What keeps it in range is that a pawn cannot stand on rank 1 or
    // rank 8; the debug build checks that instead of trusting it, because the
    // penalty for being wrong is a read off the end of a six-entry table.
    const int bucket = 6 - (get_lsb_index(white_passed) >> 3);
    white_passed &= white_passed - 1;

    assert(bucket >= 0 && bucket < 6);

    mg_sum += passed_pawn_mg[bucket];
    eg_sum += passed_pawn_eg[bucket];

    if constexpr (collect) { passed_out[WHITE][bucket]++; }
  }

  while (black_passed) {
    // Mirrored: Black's own second rank is rank 7, squares 8..15.
    const int bucket = (get_lsb_index(black_passed) >> 3) - 1;
    black_passed &= black_passed - 1;

    assert(bucket >= 0 && bucket < 6);

    mg_sum -= passed_pawn_mg[bucket];
    eg_sum -= passed_pawn_eg[bucket];

    // Black's own row counts Black's passers as positives. The minus above is
    // the score's sign and not the count's, and folding the two together is
    // what would make a row mean two different things.
    if constexpr (collect) { passed_out[BLACK][bucket]++; }
  }

  *mg = mg_sum;
  *eg = eg_sum;
}


// The counts on their own, weights and taper discarded. Only the test calls it,
// so the mg/eg pair it also computes is thrown away rather than split out: a
// second entry point into the same loop would be one more thing to keep in
// step.
void passed_pawn_counts(const board_t* board, int out[2][6])
{
  int mg = 0;
  int eg = 0;
  int structure[2][PS_FEATURE_COUNT];

  evaluate_pawns<true>(board, &mg, &eg, out, structure);
}


// The other half of the same call, and thrown away the same way. Both accessors
// go through the collecting instantiation, so the one evaluate_cheap() calls
// keeps exactly one caller and clang keeps inlining it -- handing a hot-path
// function a second caller is what cost 21 % once, see king_shelter_features()
// below.
void pawn_structure_counts(const board_t* board, int out[2][3])
{
  int mg = 0;
  int eg = 0;
  int passed[2][6];

  evaluate_pawns<true>(board, &mg, &eg, passed, out);
}


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
// is in the same call and therefore the same stage; it has no clamp argument of
// its own, it shares the fills.
//
// The accumulators are White relative, so the sum is negated once at the end
// for Black. Doing it here rather than at every call site is what keeps a later
// term from picking up the wrong sign.
int evaluate_cheap(const board_t* board)
{
  // Interpolate the two tables on how much material is left, so a term slides
  // from its middlegame value to its endgame one instead of jumping when some
  // arbitrary piece comes off.
  const int phase = game_phase(board);

  int pawn_mg = 0;
  int pawn_eg = 0;
  evaluate_pawns<false>(board, &pawn_mg, &pawn_eg, nullptr, nullptr);

  // Summed into the accumulated pair before the interpolation rather than
  // tapered on its own. One integer division instead of two, on a function that
  // costs 1.36 ns in total, and one truncation towards zero instead of two --
  // which is what keeps test_eval_model's one-centipawn slack against the
  // tuner's floating-point model from having to grow.
  const int positional =
      (((board->psqt_mg + pawn_mg) * phase) +
       ((board->psqt_eg + pawn_eg) * (GAME_PHASE_MAX - phase))) /
      GAME_PHASE_MAX;

  const int score = board->material + positional;

  return (board->active_color == WHITE) ? score : -score;
}


// Mobility, and the reason this file now has two stages. It cannot be
// accumulated: it is a function of occupancy, not of a piece and its square, so
// moving any piece changes it for every slider whose ray crosses the from or
// the to square and there is no O(1) delta for make_move to apply. Measured at
// 56 ns per node in a real search, DEC-039, where the isolated benchmark said
// 14.6 -- the difference is cache misses in 2 MB of rook attack tables that a
// ten-position benchmark loop never pays.
//
// These weights are fitted, not chosen. The hand-picked set they replaced
// measured -14.93 Elo; fitted, and behind the lazy shortcut, the same term
// measured +28.46 over 942 games. The fit disagreed with the guess about the
// two things that mattered -- a knight's mobility is worth nothing on top of
// its piece-square table, and a rook's middlegame mobility is worth four times
// what was guessed. DEC-040. No published table was consulted, DEC-016.
const int mobility_mg[4] = {0, 5, 9, 2};  // knight bishop rook queen
const int mobility_eg[4] = {-1, 6, 1, 3};


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
const int king_safety_mg[KS_FEATURE_COUNT] = {23, 13, 4,   21, -16,
                                              3,  2,  -33, -11};
const int king_safety_eg[KS_FEATURE_COUNT] = {-8, -8, -2,  -34, 8,
                                              -3, -8, -10, 10};


// A king's own zone: its square and the eight around it. Empty when that king
// is not on the board -- evaluate() is called on such positions,
// test_evaluation prices one to check the king carries no material, and
// get_lsb_index() of an empty board answers 64, which is off the end of every
// table here. An empty zone then costs nothing extra downstream: no attack set
// can intersect it.
static bb_t king_zone(const bb_tables_t* tables,
                      const board_t* board,
                      int colour)
{
  const bb_t king = board->bitboards[(colour == WHITE) ? W_KING : B_KING];

  if (king == 0) { return 0; }

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

  if (king == 0) { return; }

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

  int mobility_mg_sum = 0;
  int mobility_eg_sum = 0;
  int safety_mg_sum = 0;
  int safety_eg_sum = 0;

  if constexpr (collect) {
    for (int colour = 0; colour < 2; ++colour) {
      for (int f = 0; f < KS_FEATURE_COUNT; ++f) {
        out[colour][f] = 0;
      }
    }
  }

  for (int colour = 0; colour < 2; ++colour) {
    // The piece's own side, which is what mobility is worth to.
    const int mover = (colour == WHITE) ? 1 : -1;

    // The side whose king these pieces are attacking, which is the other one.
    const int defender = -mover;

    const bb_t own = board->occupancies[colour];
    const bb_t defended_zone = zone[1 - colour];
    const int base = (colour == WHITE) ? W_KNIGHT : B_KNIGHT;

    for (int type = 0; type < 4; ++type) {
      bb_t pieces = board->bitboards[base + type];

      while (pieces) {
        const index_t square = get_lsb_index(pieces);
        pieces &= pieces - 1;

        const bb_t attacks = piece_attacks(tables, type, square, occupancy);

        // Squares the piece could move to, own pieces excluded. Not safe
        // mobility: no enemy-pawn-attack mask, which is the expensive variant.
        const int count = count_bits(attacks & ~own);

        mobility_mg_sum += mover * count * mobility_mg[type];
        mobility_eg_sum += mover * count * mobility_eg[type];

        const bb_t hits = attacks & defended_zone;

        if (hits == 0) { continue; }

        const int attacker = KS_KNIGHT_ATTACKERS + type;

        safety_mg_sum += defender * king_safety_mg[attacker];
        safety_eg_sum += defender * king_safety_eg[attacker];

        // Attacker-square incidences, not distinct squares under fire: two
        // pieces bearing on the same square count twice. Distinct squares
        // saturate at nine and stop separating one attacker from four, which is
        // the whole thing a linear model has to read off this number.
        const int incidences = count_bits(hits);

        safety_mg_sum +=
            defender * incidences * king_safety_mg[KS_ZONE_ATTACKS];
        safety_eg_sum +=
            defender * incidences * king_safety_eg[KS_ZONE_ATTACKS];

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
      safety_mg_sum += sign * features[f] * king_safety_mg[f];
      safety_eg_sum += sign * features[f] * king_safety_eg[f];

      if constexpr (collect) { out[colour][f] = features[f]; }
    }
  }

  const int phase = game_phase(board);
  const int endgame = GAME_PHASE_MAX - phase;

  const int mobility =
      (mobility_mg_sum * phase + mobility_eg_sum * endgame) / GAME_PHASE_MAX;
  const int safety =
      (safety_mg_sum * phase + safety_eg_sum * endgame) / GAME_PHASE_MAX;

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
// white bishops from promotions, where mobility alone reached 155 against a
// margin of 150 and the test caught it.
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


int evaluate_lazy(const board_t* board, int alpha, int beta)
{
  const int cheap = evaluate_cheap(board);

  // Both tests are one-sided on purpose. If the cheap score is already a
  // margin clear of beta then the full score is above beta too, so the caller
  // fails high on either number and the expensive stage would change nothing it
  // does. Same argument mirrored at alpha. Anywhere between the two, the
  // correction can decide the node and has to be computed.
  if (cheap - LAZY_EVAL_MARGIN >= beta) { return cheap - LAZY_EVAL_MARGIN; }
  if (cheap + LAZY_EVAL_MARGIN <= alpha) { return cheap + LAZY_EVAL_MARGIN; }

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

  // Saturated on the way in, so it can never reach the killer band.
  return state->history_moves[MOVE_PIECE(move)][MOVE_TO(move)];
}
