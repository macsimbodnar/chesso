#include "evaluation.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"


// clang-format off

// Material values split into middlegame and endgame
#define MG_PAWN    100
#define EG_PAWN    115
#define MG_KNIGHT  300
#define EG_KNIGHT  260
#define MG_BISHOP  350
#define EG_BISHOP  340
#define MG_ROOK    500
#define EG_ROOK    510
#define MG_QUEEN   1000
#define EG_QUEEN   960
#define VALUE_KING 10000

// Tapered eval: phase is computed from remaining non-pawn material
// knights=1, bishops=1, rooks=2, queens=4, total possible = 24
#define PHASE_KNIGHT  1
#define PHASE_BISHOP  1
#define PHASE_ROOK    2
#define PHASE_QUEEN   4
#define PHASE_MAX    24

// Pawn structure penalties
#define MG_DOUBLE_PAWN_PENALTY   -10
#define EG_DOUBLE_PAWN_PENALTY   -25
#define MG_ISOLATED_PAWN_PENALTY -10
#define EG_ISOLATED_PAWN_PENALTY -20

// Open file bonuses (rook) and penalties (king)
#define MG_SEMI_OPEN_FILE_BONUS   10
#define EG_SEMI_OPEN_FILE_BONUS    5
#define MG_OPEN_FILE_BONUS        15
#define EG_OPEN_FILE_BONUS        10

// Passed pawn enhancements
#define PASSED_KING_SUPPORT_WEIGHT   3   // EG: own king close to passer (per distance unit)
#define PASSED_KING_BLOCK_WEIGHT     4   // EG: enemy king close to passer (per distance unit)
#define PASSED_ROOK_BEHIND_MG       15   // rook behind own passer
#define PASSED_ROOK_BEHIND_EG       20
#define PASSED_ROOK_ENEMY_MG        15   // enemy rook behind our passer
#define PASSED_ROOK_ENEMY_EG        20
#define PASSED_SUPPORTED_MG         10   // passer defended by own pawn
#define PASSED_SUPPORTED_EG         15

// King safety pawn shield penalties (MG only)
#define SHIELD_MISSING_PAWN     -20   // no friendly pawn on shield file
#define SHIELD_PUSHED_PAWN      -10   // pawn pushed to rank 3/6 instead of 2/7

// Bishop pair bonus: two bishops are worth more than a bishop + knight in open positions
#define MG_BISHOP_PAIR_BONUS      40
#define EG_BISHOP_PAIR_BONUS      60

// Mobility bonuses (per reachable square, excluding own pieces)
#define MG_KNIGHT_MOBILITY         4
#define EG_KNIGHT_MOBILITY         4
#define MG_ROOK_MOBILITY           2
#define EG_ROOK_MOBILITY           3


// Piece-square tables (PSTs) indexed a8(0) to h1(63), white's perspective.
// Black pieces use black_indexes[] to mirror the table.

static inline constexpr int mg_pawn_pst[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
 50, 50, 50, 50, 50, 50, 50, 50,
 10, 10, 20, 30, 30, 20, 10, 10,
  5,  5, 10, 25, 25, 10,  5,  5,
  0,  0,  0, 20, 20,  0,  0,  0,
  5, -5,-10,  0,  0,-10, -5,  5,
  5, 10, 10,-20,-20, 10, 10,  5,
  0,  0,  0,  0,  0,  0,  0,  0
};

// In endgames pawns are more valuable the further advanced they are;
// file structure matters less.
static inline constexpr int eg_pawn_pst[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
 80, 80, 80, 80, 80, 80, 80, 80,
 55, 55, 55, 55, 55, 55, 55, 55,
 30, 30, 30, 30, 30, 30, 30, 30,
 20, 20, 20, 20, 20, 20, 20, 20,
 10, 10, 10, 10, 10, 10, 10, 10,
  5,  5,  5,  5,  5,  5,  5,  5,
  0,  0,  0,  0,  0,  0,  0,  0
};

static inline constexpr int mg_knight_pst[64] = {
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  0, 10, 15, 15, 10,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50
};

// Knights are slightly less useful in pure endgames (fewer targets)
// but their PST shape stays the same.
static inline constexpr int eg_knight_pst[64] = {
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  0, 10, 15, 15, 10,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50
};

static inline constexpr int mg_bishop_pst[64] = {
-20,-10,-10,-10,-10,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5, 10, 10,  5,  0,-10,
-10,  5,  5, 10, 10,  5,  5,-10,
-10,  0, 10, 10, 10, 10,  0,-10,
-10, 10, 10, 10, 10, 10, 10,-10,
-10,  5,  0,  0,  0,  0,  5,-10,
-20,-10,-10,-10,-10,-10,-10,-20
};

static inline constexpr int eg_bishop_pst[64] = {
-20,-10,-10,-10,-10,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5, 10, 10,  5,  0,-10,
-10,  5,  5, 10, 10,  5,  5,-10,
-10,  0, 10, 10, 10, 10,  0,-10,
-10, 10, 10, 10, 10, 10, 10,-10,
-10,  5,  0,  0,  0,  0,  5,-10,
-20,-10,-10,-10,-10,-10,-10,-20
};

static inline constexpr int mg_rook_pst[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
  5, 10, 10, 10, 10, 10, 10,  5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  0,  0,  0,  5,  5,  0,  0,  0
};

static inline constexpr int eg_rook_pst[64] = {
  5,  5,  5,  5,  5,  5,  5,  5,
  5, 10, 10, 10, 10, 10, 10,  5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  0,  0,  0,  5,  5,  0,  0,  0
};

static inline constexpr int mg_queen_pst[64] = {
-20,-10,-10, -5, -5,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5,  5,  5,  5,  0,-10,
 -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
-10,  5,  5,  5,  5,  5,  0,-10,
-10,  0,  5,  0,  0,  0,  0,-10,
-20,-10,-10, -5, -5,-10,-10,-20
};

static inline constexpr int eg_queen_pst[64] = {
-20,-10,-10, -5, -5,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5,  5,  5,  5,  0,-10,
 -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
-10,  5,  5,  5,  5,  5,  0,-10,
-10,  0,  5,  0,  0,  0,  0,-10,
-20,-10,-10, -5, -5,-10,-10,-20
};

// King PSTs: MG prefers a castled position, EG prefers centralizing.
static inline constexpr int mg_king_pst[64] = {
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-20,-30,-30,-40,-40,-30,-30,-20,
-10,-20,-20,-20,-20,-20,-20,-10,
 20, 20,  0,  0,  0,  0, 20, 20,
 20, 30, 10,  0,  0, 10, 30, 20
};

static inline constexpr int eg_king_pst[64] = {
-50,-40,-30,-20,-20,-30,-40,-50,
-30,-20,-10,  0,  0,-10,-20,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-30,  0,  0,  0,  0,-30,-30,
-50,-30,-30,-30,-30,-30,-30,-50
};

// Non-linear king safety penalty table, indexed by total attack units.
// Attack units: knight/bishop = 2, rook = 3, queen = 5.
// The penalty rises slowly at first then steeply once multiple heavy pieces
// are bearing down on the king zone.
static inline constexpr int king_safety_table[20] = {
   0,   0,   5,  15,  30,  50,  75, 110, 150, 200,
 260, 330, 400, 480, 500, 500, 500, 500, 500, 500
};

// Passed pawn bonus by rank (0=starting rank, 7=promotion rank).
// EG bonuses are much larger since the pawn is much closer to queening.
static inline constexpr int mg_passed_pawn_bonus[8] = {  0,  5, 10, 20, 35,  60, 100, 200 };
static inline constexpr int eg_passed_pawn_bonus[8] = {  0, 10, 25, 50, 75, 100, 150, 200 };

static inline constexpr int black_indexes[64] = {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};


// Most valuable victim & less valuable attacker
/**
 *   (Victims) Pawn Knight Bishop   Rook  Queen   King
 * (Attackers)
 *       Pawn   105    205    305    405    505    605
 *     Knight   104    204    304    404    504    604
 *     Bishop   103    203    303    403    503    603
 *       Rook   102    202    302    402    502    602
 *      Queen   101    201    301    401    501    601
 *       King   100    200    300    400    500    600
*/

// [attacker][victim]
static inline constexpr int mvv_lva[12][12] = {
 {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
 {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
 {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
 {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
 {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
 {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},

 {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
 {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
 {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
 {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
 {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
 {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600}
};

// clang-format on


static inline int chebyshev_dist(index_t a, index_t b)
{
  const int dr = (a / 8) - (b / 8);
  const int df = (a % 8) - (b % 8);
  return (dr < 0 ? -dr : dr) > (df < 0 ? -df : df)
             ? (dr < 0 ? -dr : dr)
             : (df < 0 ? -df : df);
}


int evaluate(const bb_tables_t* tables, const board_t* board)
{
  assert(board != nullptr);

  // TODO: Connected rook bonus

  const index_t w_king_sq = get_lsb_index(board->bitboards[W_KING]);
  const index_t b_king_sq = get_lsb_index(board->bitboards[B_KING]);

  int mg_score = 0;
  int eg_score = 0;
  int phase    = 0;

  for (int piece = W_PAWN; piece <= B_KING; ++piece) {
    bb_t current_board = board->bitboards[piece];

    while (current_board) {
      const index_t index = get_lsb_index(current_board);

      switch (piece) {
        // ################################# WHITE PIECES
        case W_PAWN:
          mg_score += MG_PAWN + mg_pawn_pst[index];
          eg_score += EG_PAWN + eg_pawn_pst[index];

          // Doubled pawns
          if (count_bits(board->bitboards[W_PAWN] & file_masks[index]) > 1) {
            mg_score += MG_DOUBLE_PAWN_PENALTY;
            eg_score += EG_DOUBLE_PAWN_PENALTY;
          }

          // Isolated pawns
          if ((board->bitboards[W_PAWN] & isolated_file_masks[index]) == 0) {
            mg_score += MG_ISOLATED_PAWN_PENALTY;
            eg_score += EG_ISOLATED_PAWN_PENALTY;
          }

          // Passed pawn
          if ((passed_w_pawns_masks[index] & board->bitboards[B_PAWN]) == 0) {
            const uint8_t rank = 7 - (index / 8);
            assert(rank < 8);
            mg_score += mg_passed_pawn_bonus[rank];
            eg_score += eg_passed_pawn_bonus[rank];

            // King proximity (EG only): own king close = good, enemy king close = bad
            eg_score += (7 - chebyshev_dist(index, w_king_sq)) * PASSED_KING_SUPPORT_WEIGHT;
            eg_score -= (7 - chebyshev_dist(index, b_king_sq)) * PASSED_KING_BLOCK_WEIGHT;

            // Rook behind passer (Tarrasch rule): squares on same file behind the pawn
            // "behind" for white = toward rank 1 = higher index numbers
            const bb_t behind = file_masks[index] & ~((BB_1 << (index + 1)) - 1);
            if (board->bitboards[W_ROOK] & behind) {
              mg_score += PASSED_ROOK_BEHIND_MG;
              eg_score += PASSED_ROOK_BEHIND_EG;
            }
            if (board->bitboards[B_ROOK] & behind) {
              mg_score -= PASSED_ROOK_ENEMY_MG;
              eg_score -= PASSED_ROOK_ENEMY_EG;
            }

            // Supported passer: defended by another white pawn
            if (board->bitboards[W_PAWN] & tables->pawn_attacks[BLACK][index]) {
              mg_score += PASSED_SUPPORTED_MG;
              eg_score += PASSED_SUPPORTED_EG;
            }
          }

          break;

        case W_KNIGHT:
          mg_score += MG_KNIGHT + mg_knight_pst[index];
          eg_score += EG_KNIGHT + eg_knight_pst[index];
          phase += PHASE_KNIGHT;
          {
            const int mobility = count_bits(
                tables->knight_attacks[index] & ~board->occupancies[WHITE]);
            mg_score += mobility * MG_KNIGHT_MOBILITY;
            eg_score += mobility * EG_KNIGHT_MOBILITY;
          }
          break;

        case W_BISHOP:
          mg_score += MG_BISHOP + mg_bishop_pst[index];
          eg_score += EG_BISHOP + eg_bishop_pst[index];
          phase += PHASE_BISHOP;
          {
            const int mobility = count_bits(
                get_bishop_attacks(tables, index, board->occupancies[BOTH]));
            mg_score += mobility;
            eg_score += mobility;
          }
          break;

        case W_ROOK:
          mg_score += MG_ROOK + mg_rook_pst[index];
          eg_score += EG_ROOK + eg_rook_pst[index];
          phase += PHASE_ROOK;
          {
            const int mobility = count_bits(
                get_rook_attacks(tables, index, board->occupancies[BOTH]) &
                ~board->occupancies[WHITE]);
            mg_score += mobility * MG_ROOK_MOBILITY;
            eg_score += mobility * EG_ROOK_MOBILITY;
          }

          if ((board->bitboards[W_PAWN] & file_masks[index]) == 0) {
            mg_score += MG_SEMI_OPEN_FILE_BONUS;
            eg_score += EG_SEMI_OPEN_FILE_BONUS;
          }
          if (((board->bitboards[W_PAWN] | board->bitboards[B_PAWN]) &
               file_masks[index]) == 0) {
            mg_score += MG_OPEN_FILE_BONUS;
            eg_score += EG_OPEN_FILE_BONUS;
          }
          break;

        case W_QUEEN:
          mg_score += MG_QUEEN + mg_queen_pst[index];
          eg_score += EG_QUEEN + eg_queen_pst[index];
          phase += PHASE_QUEEN;
          {
            const int mobility = count_bits(
                get_queen_attacks(tables, index, board->occupancies[BOTH]));
            mg_score += mobility;
            eg_score += mobility;
          }
          break;

        case W_KING:
          mg_score += VALUE_KING + mg_king_pst[index];
          eg_score += VALUE_KING + eg_king_pst[index];

          if ((board->bitboards[W_PAWN] & file_masks[index]) == 0) {
            mg_score -= MG_SEMI_OPEN_FILE_BONUS;
            eg_score -= EG_SEMI_OPEN_FILE_BONUS;
          }
          if (((board->bitboards[W_PAWN] | board->bitboards[B_PAWN]) &
               file_masks[index]) == 0) {
            mg_score -= MG_OPEN_FILE_BONUS;
            eg_score -= EG_OPEN_FILE_BONUS;
          }

          // King safety: count enemy piece attacks into the king zone
          {
            const bb_t king_zone = tables->king_attacks[index] | (BB_1 << index);
            int attack_units = 0;

            bb_t attackers = board->bitboards[B_KNIGHT];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (tables->knight_attacks[sq] & king_zone) attack_units += 2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[B_BISHOP];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (get_bishop_attacks(tables, sq, board->occupancies[BOTH]) & king_zone)
                attack_units += 2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[B_ROOK];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (get_rook_attacks(tables, sq, board->occupancies[BOTH]) & king_zone)
                attack_units += 3;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[B_QUEEN];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (get_queen_attacks(tables, sq, board->occupancies[BOTH]) & king_zone)
                attack_units += 5;
              POP_BIT(attackers, sq);
            }

            if (attack_units > 19) attack_units = 19;
            mg_score -= king_safety_table[attack_units];
          }

          // Pawn shield: king on ranks 1-2 (row indices 7 and 6)
          {
            const int king_row  = index / 8;
            const int king_file = index % 8;
            if (king_row >= 6) {
              const int f0 = (king_file > 0) ? king_file - 1 : 0;
              const int f1 = (king_file < 7) ? king_file + 1 : 7;
              for (int f = f0; f <= f1; ++f) {
                const bb_t rank2 = BB_1 << (48 + f);
                const bb_t rank3 = BB_1 << (40 + f);
                if ((board->bitboards[W_PAWN] & (rank2 | rank3)) == 0) {
                  mg_score += SHIELD_MISSING_PAWN;
                } else if ((board->bitboards[W_PAWN] & rank2) == 0) {
                  mg_score += SHIELD_PUSHED_PAWN;
                }
              }
            }
          }
          break;

        // ################################# BLACK PIECES
        case B_PAWN:
          mg_score -= MG_PAWN + mg_pawn_pst[black_indexes[index]];
          eg_score -= EG_PAWN + eg_pawn_pst[black_indexes[index]];

          // Doubled pawns
          if (count_bits(board->bitboards[B_PAWN] & file_masks[index]) > 1) {
            mg_score -= MG_DOUBLE_PAWN_PENALTY;
            eg_score -= EG_DOUBLE_PAWN_PENALTY;
          }

          // Isolated pawns
          if ((board->bitboards[B_PAWN] & isolated_file_masks[index]) == 0) {
            mg_score -= MG_ISOLATED_PAWN_PENALTY;
            eg_score -= EG_ISOLATED_PAWN_PENALTY;
          }

          // Passed pawn
          if ((passed_b_pawns_masks[index] & board->bitboards[W_PAWN]) == 0) {
            const uint8_t rank = index / 8;
            assert(rank < 8);
            mg_score -= mg_passed_pawn_bonus[rank];
            eg_score -= eg_passed_pawn_bonus[rank];

            // King proximity (EG only)
            eg_score -= (7 - chebyshev_dist(index, b_king_sq)) * PASSED_KING_SUPPORT_WEIGHT;
            eg_score += (7 - chebyshev_dist(index, w_king_sq)) * PASSED_KING_BLOCK_WEIGHT;

            // Rook behind passer: "behind" for black = toward rank 8 = lower index numbers
            const bb_t behind = file_masks[index] & ((BB_1 << index) - 1);
            if (board->bitboards[B_ROOK] & behind) {
              mg_score -= PASSED_ROOK_BEHIND_MG;
              eg_score -= PASSED_ROOK_BEHIND_EG;
            }
            if (board->bitboards[W_ROOK] & behind) {
              mg_score += PASSED_ROOK_ENEMY_MG;
              eg_score += PASSED_ROOK_ENEMY_EG;
            }

            // Supported passer: defended by another black pawn
            if (board->bitboards[B_PAWN] & tables->pawn_attacks[WHITE][index]) {
              mg_score -= PASSED_SUPPORTED_MG;
              eg_score -= PASSED_SUPPORTED_EG;
            }
          }
          break;

        case B_KNIGHT:
          mg_score -= MG_KNIGHT + mg_knight_pst[black_indexes[index]];
          eg_score -= EG_KNIGHT + eg_knight_pst[black_indexes[index]];
          phase += PHASE_KNIGHT;
          {
            const int mobility = count_bits(
                tables->knight_attacks[index] & ~board->occupancies[BLACK]);
            mg_score -= mobility * MG_KNIGHT_MOBILITY;
            eg_score -= mobility * EG_KNIGHT_MOBILITY;
          }
          break;

        case B_BISHOP:
          mg_score -= MG_BISHOP + mg_bishop_pst[black_indexes[index]];
          eg_score -= EG_BISHOP + eg_bishop_pst[black_indexes[index]];
          phase += PHASE_BISHOP;
          {
            const int mobility = count_bits(
                get_bishop_attacks(tables, index, board->occupancies[BOTH]));
            mg_score -= mobility;
            eg_score -= mobility;
          }
          break;

        case B_ROOK:
          mg_score -= MG_ROOK + mg_rook_pst[black_indexes[index]];
          eg_score -= EG_ROOK + eg_rook_pst[black_indexes[index]];
          phase += PHASE_ROOK;
          {
            const int mobility = count_bits(
                get_rook_attacks(tables, index, board->occupancies[BOTH]) &
                ~board->occupancies[BLACK]);
            mg_score -= mobility * MG_ROOK_MOBILITY;
            eg_score -= mobility * EG_ROOK_MOBILITY;
          }

          if ((board->bitboards[B_PAWN] & file_masks[index]) == 0) {
            mg_score -= MG_SEMI_OPEN_FILE_BONUS;
            eg_score -= EG_SEMI_OPEN_FILE_BONUS;
          }
          if (((board->bitboards[B_PAWN] | board->bitboards[W_PAWN]) &
               file_masks[index]) == 0) {
            mg_score -= MG_OPEN_FILE_BONUS;
            eg_score -= EG_OPEN_FILE_BONUS;
          }
          break;

        case B_QUEEN:
          mg_score -= MG_QUEEN + mg_queen_pst[black_indexes[index]];
          eg_score -= EG_QUEEN + eg_queen_pst[black_indexes[index]];
          phase += PHASE_QUEEN;
          {
            const int mobility = count_bits(
                get_queen_attacks(tables, index, board->occupancies[BOTH]));
            mg_score -= mobility;
            eg_score -= mobility;
          }
          break;

        case B_KING:
          mg_score -= VALUE_KING + mg_king_pst[black_indexes[index]];
          eg_score -= VALUE_KING + eg_king_pst[black_indexes[index]];

          if ((board->bitboards[B_PAWN] & file_masks[index]) == 0) {
            mg_score += MG_SEMI_OPEN_FILE_BONUS;
            eg_score += EG_SEMI_OPEN_FILE_BONUS;
          }
          if (((board->bitboards[B_PAWN] | board->bitboards[W_PAWN]) &
               file_masks[index]) == 0) {
            mg_score += MG_OPEN_FILE_BONUS;
            eg_score += EG_OPEN_FILE_BONUS;
          }

          // King safety: count enemy piece attacks into the king zone
          {
            const bb_t king_zone = tables->king_attacks[index] | (BB_1 << index);
            int attack_units = 0;

            bb_t attackers = board->bitboards[W_KNIGHT];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (tables->knight_attacks[sq] & king_zone) attack_units += 2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[W_BISHOP];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (get_bishop_attacks(tables, sq, board->occupancies[BOTH]) & king_zone)
                attack_units += 2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[W_ROOK];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (get_rook_attacks(tables, sq, board->occupancies[BOTH]) & king_zone)
                attack_units += 3;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[W_QUEEN];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              if (get_queen_attacks(tables, sq, board->occupancies[BOTH]) & king_zone)
                attack_units += 5;
              POP_BIT(attackers, sq);
            }

            if (attack_units > 19) attack_units = 19;
            mg_score += king_safety_table[attack_units];
          }

          // Pawn shield: king on ranks 7-8 (row indices 0 and 1)
          {
            const int king_row  = index / 8;
            const int king_file = index % 8;
            if (king_row <= 1) {
              const int f0 = (king_file > 0) ? king_file - 1 : 0;
              const int f1 = (king_file < 7) ? king_file + 1 : 7;
              for (int f = f0; f <= f1; ++f) {
                const bb_t rank7 = BB_1 << (8  + f);
                const bb_t rank6 = BB_1 << (16 + f);
                if ((board->bitboards[B_PAWN] & (rank7 | rank6)) == 0) {
                  mg_score -= SHIELD_MISSING_PAWN;
                } else if ((board->bitboards[B_PAWN] & rank7) == 0) {
                  mg_score -= SHIELD_PUSHED_PAWN;
                }
              }
            }
          }
          break;

        case EMPTY:
        default:
          assert(false);
          break;
      }

      POP_BIT(current_board, index);
    }
  }

  // Bishop pair bonus
  if (count_bits(board->bitboards[W_BISHOP]) >= 2) {
    mg_score += MG_BISHOP_PAIR_BONUS;
    eg_score += EG_BISHOP_PAIR_BONUS;
  }
  if (count_bits(board->bitboards[B_BISHOP]) >= 2) {
    mg_score -= MG_BISHOP_PAIR_BONUS;
    eg_score -= EG_BISHOP_PAIR_BONUS;
  }

  // Interpolate between MG and EG scores based on remaining material.
  // phase=PHASE_MAX means full MG, phase=0 means pure EG.
  if (phase > PHASE_MAX) phase = PHASE_MAX;
  return (mg_score * phase + eg_score * (PHASE_MAX - phase)) / PHASE_MAX;
}


int evaluate_move(const board_t* board,
                  const search_state_t* state,
                  move_t move,
                  size_t ply)
{
  assert(state != nullptr);

  // Promotions (checked before captures so capture-promotions get both bonuses)
  if (MOVE_PROMOTED(move) != TO_NONE) {
    int promo_score = 0;
    switch (MOVE_PROMOTED(move)) {
      case TO_QUEEN:  promo_score = 20000; break;
      case TO_ROOK:   promo_score = 15000; break;
      case TO_KNIGHT:
      case TO_BISHOP: promo_score = 10000; break;
      default:        break;
    }
    if (MOVE_CAPTURE(move)) {
      const piece_t attacker = MOVE_PIECE(move);
      piece_t victim = get_piece(board, MOVE_TO(move));
      if (victim == EMPTY) victim = W_PAWN;
      promo_score += mvv_lva[attacker][victim];
    }
    return promo_score;
  }

  if (MOVE_CAPTURE(move)) {
    const piece_t attacker = MOVE_PIECE(move);

    // TODO: In case of en-passant this does not return the correct victim
    piece_t victim = get_piece(board, MOVE_TO(move));

    assert(attacker != EMPTY);

    // here we assume en-passant capture and set to pawn
    if (victim == EMPTY) {
      // Here we don't care about color
      victim = W_PAWN;
    }

    const int score = mvv_lva[attacker][victim];
    return 10000 + score;
  }

  // Killer, counter move, then history
  if (move == state->killer_moves[0][ply]) {
    return 10000 - 1000;
  } else if (move == state->killer_moves[1][ply]) {
    return 10000 - 2000;
  } else if (state->node_prev_move[ply] != 0 &&
             move == state->counter_moves[MOVE_PIECE(state->node_prev_move[ply])]
                                         [MOVE_TO(state->node_prev_move[ply])]) {
    return 10000 - 3000;
  } else {
    return state->history_moves[MOVE_PIECE(move)][MOVE_TO(move)];
  }

  return 0;
}


void order_moves(const board_t* board,
                 const search_state_t* state,
                 size_t ply,
                 size_t moves_size,
                 move_t moves[])
{
  assert(moves != nullptr);
  assert(moves_size <= MAX_MOVES);
  assert(state != nullptr);
  assert(state->killer_moves[0] != nullptr);
  assert(state->killer_moves[1] != nullptr);

  int scores[MAX_MOVES];
  for (size_t i = 0; i < moves_size; ++i) {
    scores[i] = evaluate_move(board, state, moves[i], ply);
  }

  // Insertion sort
  for (size_t i = 1; i < moves_size; ++i) {
    const int value = scores[i];
    const move_t move = moves[i];
    int j = i;

    while (j != 0 && scores[j - 1] < value) {
      scores[j] = scores[j - 1];
      moves[j] = moves[j - 1];
      --j;
    }

    scores[j] = value;
    moves[j] = move;
  }
}


bool should_reduce_move(move_t move)
{
  if (MOVE_PROMOTED(move) != TO_NONE || MOVE_CAPTURE(move)) { return false; }
  return true;
}


void order_captures(const board_t* board, move_t moves[], size_t moves_size)
{
  assert(moves != nullptr);
  assert(moves_size <= MAX_MOVES);

  int scores[MAX_MOVES];
  for (size_t i = 0; i < moves_size; ++i) {
    if (MOVE_CAPTURE(moves[i])) {
      const piece_t attacker = MOVE_PIECE(moves[i]);

      // TODO: In case of en-passant this does not return the correct victim
      piece_t victim = get_piece(board, MOVE_TO(moves[i]));

      assert(attacker != EMPTY);

      // here we assume en-passant capture and set to pawn
      if (victim == EMPTY) {
        // Here we don't care about color
        victim = W_PAWN;
      }

      scores[i] = mvv_lva[attacker][victim];
    } else {
      // Handling the promotions that are not captures
      scores[i] = 0;
    }
  }

  // Insertion sort
  for (size_t i = 1; i < moves_size; ++i) {
    const int value = scores[i];
    const move_t move = moves[i];
    int j = i;

    while (j != 0 && scores[j - 1] < value) {
      scores[j] = scores[j - 1];
      moves[j] = moves[j - 1];
      --j;
    }

    scores[j] = value;
    moves[j] = move;
  }
}


int get_max_gain() { return MG_QUEEN; }

int get_margin_value() { return MG_PAWN; }

int get_piece_value(piece_t piece)
{
  switch (piece) {
    case W_PAWN:   case B_PAWN:   return MG_PAWN;
    case W_KNIGHT: case B_KNIGHT: return MG_KNIGHT;
    case W_BISHOP: case B_BISHOP: return MG_BISHOP;
    case W_ROOK:   case B_ROOK:   return MG_ROOK;
    case W_QUEEN:  case B_QUEEN:  return MG_QUEEN;
    case W_KING:   case B_KING:   return VALUE_KING;
    default:                      return 0;
  }
}

int get_futility_margin()         { return MG_ROOK; }
int get_reverse_futility_margin() { return MG_PAWN + 20; }
int get_delta_margin()            { return 200; }
