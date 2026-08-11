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

// Everything here was already computed by make_move: the material balance, the
// two piece-square sums and the phase are maintained as pieces move rather than
// rebuilt from the bitboards. All that is left is the interpolation and the
// sign, which is why this function stopped being 40% of the search.
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
  const int positional =
      ((board->psqt_mg * phase) + (board->psqt_eg * (GAME_PHASE_MAX - phase))) /
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
// Still at zero, and no longer only because the fit had not run. It has:
// tools/tuner over 1490839 positions with the other 781 constants frozen gave
// mg {23, 13, 4, 21, -16, 3, 2, -33, -11} and eg {-8, -8, -2, -34, 8, -3, -8,
// -10, 10}, held-out error 0.107413 to 0.107109. Those weights are in
// .tuning/tuned_ks_only.hpp and they are not applied here yet.
//
// They are held back because applying them exposed a defect in evaluate_lazy
// that predates this term, and a known defect in the tree contaminates every
// measurement taken after it. The bound fix goes first and alone, so its SPRT
// and king safety's each measure one change. S027.
const int king_safety_mg[KS_FEATURE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const int king_safety_eg[KS_FEATURE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0};


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
