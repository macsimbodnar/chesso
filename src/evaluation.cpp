#include "evaluation.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"


// clang-format off

// Material values from PeSTO (Texel-tuned, widely validated)
#define MG_PAWN     82
#define EG_PAWN     94
#define MG_KNIGHT  337
#define EG_KNIGHT  281
#define MG_BISHOP  365
#define EG_BISHOP  297
#define MG_ROOK    477
#define EG_ROOK    512
#define MG_QUEEN  1025
#define EG_QUEEN   936
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

// Connected rooks: two rooks on the same file or rank with no pieces between them.
#define MG_CONNECTED_ROOKS  15
#define EG_CONNECTED_ROOKS  10

// Rook on 7th rank (2nd for black): strong bonus, even more when enemy king
// is trapped on the back rank.
#define MG_ROOK_ON_7TH          20
#define EG_ROOK_ON_7TH          30
#define MG_ROOK_ON_7TH_KING     10
#define EG_ROOK_ON_7TH_KING     15

// Knight outpost: defended by own pawn, no enemy pawn can ever reach an
// adjacent file to attack it.
#define MG_OUTPOST_BONUS   20
#define EG_OUTPOST_BONUS   10

// Backward pawn: cannot advance (square ahead attacked by enemy pawn)
// and no friendly pawn on adjacent file can support it from behind.
#define MG_BACKWARD_PAWN_PENALTY  -10
#define EG_BACKWARD_PAWN_PENALTY  -15

// Passed pawn enhancements
#define PASSED_ROOK_BEHIND_MG       15
#define PASSED_ROOK_BEHIND_EG       20
#define PASSED_ROOK_ENEMY_MG        15
#define PASSED_ROOK_ENEMY_EG        20
#define PASSED_SUPPORTED_MG         10
#define PASSED_SUPPORTED_EG         15

// King safety pawn shield penalties (MG only)
#define SHIELD_MISSING_PAWN     -20
#define SHIELD_PUSHED_PAWN      -10

// Bishop pair bonus
#define MG_BISHOP_PAIR_BONUS      40
#define EG_BISHOP_PAIR_BONUS      60

// Mobility bonuses (per reachable square, excluding own pieces)
#define MG_KNIGHT_MOBILITY         4
#define EG_KNIGHT_MOBILITY         4
#define MG_ROOK_MOBILITY           2
#define EG_ROOK_MOBILITY           3


// Piece-square tables from PeSTO's evaluation function (Texel-tuned).
// Indexed a8(0) to h1(63), white's perspective.
// Black pieces use black_indexes[] to mirror the table.

static inline constexpr int mg_pawn_pst[64] = {
   0,   0,   0,   0,   0,   0,   0,   0,
  98, 134,  61,  95,  68, 126,  34, -11,
  -6,   7,  26,  31,  65,  56,  25, -20,
 -14,  13,   6,  21,  23,  12,  17, -23,
 -27,  -2,  -5,  12,  17,   6,  10, -25,
 -26,  -4,  -4, -10,   3,   3,  33, -12,
 -35,  -1, -20, -23, -15,  24,  38, -22,
   0,   0,   0,   0,   0,   0,   0,   0
};

static inline constexpr int eg_pawn_pst[64] = {
   0,   0,   0,   0,   0,   0,   0,   0,
 178, 173, 158, 134, 147, 132, 165, 187,
  94, 100,  85,  67,  56,  53,  82,  84,
  32,  24,  13,   5,  -2,   4,  17,  17,
  13,   9,  -3,  -7,  -7,  -8,   3,  -1,
   4,   7,  -6,   1,   0,  -5,  -1,  -8,
  13,   8,   8,  10,  13,   0,   2,  -7,
   0,   0,   0,   0,   0,   0,   0,   0
};

static inline constexpr int mg_knight_pst[64] = {
-167, -89, -34, -49,  61, -97, -15,-107,
 -73, -41,  72,  36,  23,  62,   7, -17,
 -47,  60,  37,  65,  84, 129,  73,  44,
  -9,  17,  19,  53,  37,  69,  18,  22,
 -13,   4,  16,  13,  28,  19,  21,  -8,
 -23,  -9,  12,  10,  19,  17,  25, -16,
 -29, -53, -12,  -3,  -1,  18, -14, -19,
-105, -21, -58, -33, -17, -28, -19, -23
};

static inline constexpr int eg_knight_pst[64] = {
 -58, -38, -13, -28, -31, -27, -63, -99,
 -25,  -8, -25,  -2,  -9, -25, -24, -52,
 -24, -20,  10,   9,  -1,  -9, -19, -41,
 -17,   3,  22,  22,  22,  11,   8, -18,
 -18,  -6,  16,  25,  16,  17,   4, -18,
 -23,  -3,  -1,  15,  10,  -3, -20, -22,
 -42, -20, -10,  -5,  -2, -20, -23, -44,
 -29, -51, -23, -15, -22, -18, -50, -64
};

static inline constexpr int mg_bishop_pst[64] = {
 -29,   4, -82, -37, -25, -42,   7,  -8,
 -26,  16, -18, -13,  30,  59,  18, -47,
 -16,  37,  43,  40,  35,  50,  37,  -2,
  -4,   5,  19,  50,  37,  37,   7,  -2,
  -6,  13,  13,  26,  34,  12,  10,   4,
   0,  15,  15,  15,  14,  27,  18,  10,
   4,  15,  16,   0,   7,  21,  33,   1,
 -33,  -3, -14, -21, -13, -12, -39, -21
};

static inline constexpr int eg_bishop_pst[64] = {
 -14, -21, -11,  -8,  -7,  -9, -17, -24,
  -8,  -4,   7, -12,  -3, -13,  -4, -14,
   2,  -8,   0,  -1,  -2,   6,   0,   4,
  -3,   9,  12,   9,  14,  10,   3,   2,
  -6,   3,  13,  19,   7,  10,  -3,  -9,
 -12,  -3,   8,  10,  13,   3,  -7, -15,
 -14, -18,  -7,  -1,   4,  -9, -15, -27,
 -23,  -9, -23,  -5,  -9, -16,  -5, -17
};

static inline constexpr int mg_rook_pst[64] = {
  32,  42,  32,  51,  63,   9,  31,  43,
  27,  32,  58,  62,  80,  67,  26,  44,
  -5,  19,  26,  36,  17,  45,  61,  16,
 -24, -11,   7,  26,  24,  35,  -8, -20,
 -36, -26, -12,  -1,   9,  -7,   6, -23,
 -45, -25, -16, -17,   3,   0,  -5, -33,
 -44, -16, -20,  -9,  -1,  11,  -6, -71,
 -19, -13,   1,  17,  16,   7, -37, -26
};

static inline constexpr int eg_rook_pst[64] = {
  13,  10,  18,  15,  12,  12,   8,   5,
  11,  13,  13,  11,  -3,   3,   8,   3,
   7,   7,   7,   5,   4,  -3,  -5,  -3,
   4,   3,  13,   1,   2,   1,  -1,   2,
   3,   5,   8,   4,  -5,  -6,  -8, -11,
  -4,   0,  -5,  -1,  -7, -12,  -8, -16,
  -6,  -6,   0,   2,  -9,  -9, -11,  -3,
  -9,   2,   3,  -1,  -5, -13,   4, -20
};

static inline constexpr int mg_queen_pst[64] = {
 -28,   0,  29,  12,  59,  44,  43,  45,
 -24, -39,  -5,   1, -16,  57,  28,  54,
 -13, -17,   7,   8,  29,  56,  47,  57,
 -27, -27, -16, -16,  -1,  17,  -2,   1,
  -9, -26,  -9, -10,  -2,  -4,   3,  -3,
 -14,   2, -11,  -2,  -5,   2,  14,   5,
 -35,  -8,  11,   2,   8,  15,  -3,   1,
  -1, -18,  -9,  10, -15, -25, -31, -50
};

static inline constexpr int eg_queen_pst[64] = {
  -9,  22,  22,  27,  27,  19,  10,  20,
 -17,  20,  32,  41,  58,  25,  30,   0,
 -20,   6,   9,  49,  47,  35,  19,   9,
   3,  22,  24,  45,  57,  40,  57,  36,
 -18,  28,  19,  47,  31,  34,  39,  23,
 -16, -27,  15,   6,   9,  17,  10,   5,
 -22, -23, -30, -16, -16, -23, -36, -32,
 -33, -28, -22, -43,  -5, -32, -20, -41
};

// King: MG prefers castled corners, EG prefers centralizing.
static inline constexpr int mg_king_pst[64] = {
 -65,  23,  16, -15, -56, -34,   2,  13,
  29,  -1, -20,  -7,  -8,  -4, -38, -29,
  -9,  24,   2, -16, -20,   6,  22, -22,
 -17, -20, -12, -27, -30, -25, -14, -36,
 -49,  -1, -27, -39, -46, -44, -33, -51,
 -14, -14, -22, -46, -44, -30, -15, -27,
   1,   7,  -8, -64, -43, -16,   9,   8,
 -15,  36,  12, -54,   8, -28,  24,  14
};

static inline constexpr int eg_king_pst[64] = {
 -74, -35, -18, -18, -11,  15,   4, -17,
 -12,  17,  14,  17,  17,  38,  23,  11,
  10,  17,  23,  15,  20,  45,  44,  13,
  -8,  22,  24,  27,  26,  33,  26,   3,
 -18,  -4,  21,  24,  27,  23,   9, -11,
 -19,  -3,  11,  21,  23,  16,   7,  -9,
 -27, -11,   4,  13,  14,   4,  -5, -17,
 -53, -34, -21, -11, -28, -14, -24, -43
};

// King safety table: penalty indexed by total attack units.
// Attack units = sum of (attacked squares in king zone * weight per piece type):
//   knight=2, bishop=2, rook=3, queen=5 per square attacked.
// Non-linear curve rises slowly then steeply; capped at 200cp.
static inline constexpr int king_safety_table[50] = {
   0,  0,  0,  0,  1,  2,  3,  5,  7,  9,
  12, 15, 18, 22, 26, 30, 35, 40, 45, 51,
  57, 64, 71, 78, 86, 94,102,111,120,130,
 140,150,160,170,180,190,200,200,200,200,
 200,200,200,200,200,200,200,200,200,200
};

// Passed pawn bonuses by rank (0=starting, 7=promotion).
static inline constexpr int mg_passed_pawn_bonus[8] = {  0,  5, 10, 20, 35,  60, 100, 200 };
static inline constexpr int eg_passed_pawn_bonus[8] = {  0, 10, 25, 50, 75, 100, 150, 200 };

// King proximity weight scales with pawn rank advancement.
// Only ranks 4+ get meaningful king proximity adjustment.
static inline constexpr int passed_rank_factor[8] = { 0, 0, 0, 0, 2, 4, 7, 0 };

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
  return (dr < 0 ? -dr : dr) > (df < 0 ? -df : df) ? (dr < 0 ? -dr : dr)
                                                   : (df < 0 ? -df : df);
}


int evaluate(const bb_tables_t* tables, const board_t* board)
{
  assert(board != nullptr);

  const index_t w_king_sq = get_lsb_index(board->bitboards[W_KING]);
  const index_t b_king_sq = get_lsb_index(board->bitboards[B_KING]);

  int mg_score = 0;
  int eg_score = 0;
  int phase = 0;

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

          // Backward pawn: square ahead attacked by enemy pawn, no support from
          // behind
          {
            const index_t front = index - 8;
            const bb_t behind_adj =
                isolated_file_masks[index] & ~((BB_1 << index) - 1);
            if ((tables->pawn_attacks[WHITE][front] &
                 board->bitboards[B_PAWN]) &&
                (board->bitboards[W_PAWN] & behind_adj) == 0) {
              mg_score += MG_BACKWARD_PAWN_PENALTY;
              eg_score += EG_BACKWARD_PAWN_PENALTY;
            }
          }

          // Passed pawn
          if ((passed_w_pawns_masks[index] & board->bitboards[B_PAWN]) == 0) {
            const uint8_t rank = 7 - (index / 8);
            assert(rank < 8);
            mg_score += mg_passed_pawn_bonus[rank];
            eg_score += eg_passed_pawn_bonus[rank];

            // King proximity scaled by rank: only matters for advanced pawns
            const int rf = passed_rank_factor[rank];
            if (rf > 0) {
              eg_score += (7 - chebyshev_dist(index, w_king_sq)) * 3 * rf / 4;
              eg_score -= (7 - chebyshev_dist(index, b_king_sq)) * 4 * rf / 4;
            }

            // Rook behind passer (Tarrasch rule)
            const bb_t behind =
                file_masks[index] & ~((BB_1 << (index + 1)) - 1);
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
            const int mobility = count_bits(tables->knight_attacks[index] &
                                            ~board->occupancies[WHITE]);
            mg_score += mobility * MG_KNIGHT_MOBILITY;
            eg_score += mobility * EG_KNIGHT_MOBILITY;
          }
          // Outpost: defended by own pawn AND no black pawn can ever attack
          // this square
          if ((tables->pawn_attacks[BLACK][index] & board->bitboards[W_PAWN]) &&
              (board->bitboards[B_PAWN] &
               (isolated_file_masks[index] & ((BB_1 << index) - 1))) == 0) {
            mg_score += MG_OUTPOST_BONUS;
            eg_score += EG_OUTPOST_BONUS;
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

          // Rook on 7th rank (indices 8-15)
          if (index >= 8 && index <= 15) {
            mg_score += MG_ROOK_ON_7TH;
            eg_score += EG_ROOK_ON_7TH;
            if (b_king_sq <= 7) {
              mg_score += MG_ROOK_ON_7TH_KING;
              eg_score += EG_ROOK_ON_7TH_KING;
            }
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

          // King safety: count attacked squares in king zone, weighted by piece
          // type
          {
            const bb_t king_zone =
                tables->king_attacks[index] | (BB_1 << index);
            int attack_units = 0;

            bb_t attackers = board->bitboards[B_KNIGHT];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(tables->knight_attacks[sq] & king_zone) * 2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[B_BISHOP];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(
                      get_bishop_attacks(tables, sq, board->occupancies[BOTH]) &
                      king_zone) *
                  2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[B_ROOK];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(
                      get_rook_attacks(tables, sq, board->occupancies[BOTH]) &
                      king_zone) *
                  3;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[B_QUEEN];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(
                      get_queen_attacks(tables, sq, board->occupancies[BOTH]) &
                      king_zone) *
                  5;
              POP_BIT(attackers, sq);
            }

            if (attack_units > 49) attack_units = 49;
            mg_score -= king_safety_table[attack_units];
          }

          // Pawn shield: king on ranks 1-2 (row indices 7 and 6)
          {
            const int king_row = index / 8;
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

          // Backward pawn
          {
            const index_t front = index + 8;
            const bb_t behind_adj =
                isolated_file_masks[index] & ((BB_1 << (index + 1)) - 1);
            if ((tables->pawn_attacks[BLACK][front] &
                 board->bitboards[W_PAWN]) &&
                (board->bitboards[B_PAWN] & behind_adj) == 0) {
              mg_score -= MG_BACKWARD_PAWN_PENALTY;
              eg_score -= EG_BACKWARD_PAWN_PENALTY;
            }
          }

          // Passed pawn
          if ((passed_b_pawns_masks[index] & board->bitboards[W_PAWN]) == 0) {
            const uint8_t rank = index / 8;
            assert(rank < 8);
            mg_score -= mg_passed_pawn_bonus[rank];
            eg_score -= eg_passed_pawn_bonus[rank];

            // King proximity scaled by rank
            const int rf = passed_rank_factor[rank];
            if (rf > 0) {
              eg_score -= (7 - chebyshev_dist(index, b_king_sq)) * 3 * rf / 4;
              eg_score += (7 - chebyshev_dist(index, w_king_sq)) * 4 * rf / 4;
            }

            // Rook behind passer
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
            const int mobility = count_bits(tables->knight_attacks[index] &
                                            ~board->occupancies[BLACK]);
            mg_score -= mobility * MG_KNIGHT_MOBILITY;
            eg_score -= mobility * EG_KNIGHT_MOBILITY;
          }
          // Outpost: defended by own pawn AND no white pawn can ever attack
          // this square
          if ((tables->pawn_attacks[WHITE][index] & board->bitboards[B_PAWN]) &&
              (board->bitboards[W_PAWN] &
               (isolated_file_masks[index] & ~((BB_1 << (index + 1)) - 1))) ==
                  0) {
            mg_score -= MG_OUTPOST_BONUS;
            eg_score -= EG_OUTPOST_BONUS;
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

          // Rook on 2nd rank (indices 48-55)
          if (index >= 48 && index <= 55) {
            mg_score -= MG_ROOK_ON_7TH;
            eg_score -= EG_ROOK_ON_7TH;
            if (w_king_sq >= 56) {
              mg_score -= MG_ROOK_ON_7TH_KING;
              eg_score -= EG_ROOK_ON_7TH_KING;
            }
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

          // King safety: count attacked squares in king zone, weighted by piece
          // type
          {
            const bb_t king_zone =
                tables->king_attacks[index] | (BB_1 << index);
            int attack_units = 0;

            bb_t attackers = board->bitboards[W_KNIGHT];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(tables->knight_attacks[sq] & king_zone) * 2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[W_BISHOP];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(
                      get_bishop_attacks(tables, sq, board->occupancies[BOTH]) &
                      king_zone) *
                  2;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[W_ROOK];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(
                      get_rook_attacks(tables, sq, board->occupancies[BOTH]) &
                      king_zone) *
                  3;
              POP_BIT(attackers, sq);
            }
            attackers = board->bitboards[W_QUEEN];
            while (attackers) {
              const index_t sq = get_lsb_index(attackers);
              attack_units +=
                  count_bits(
                      get_queen_attacks(tables, sq, board->occupancies[BOTH]) &
                      king_zone) *
                  5;
              POP_BIT(attackers, sq);
            }

            if (attack_units > 49) attack_units = 49;
            mg_score += king_safety_table[attack_units];
          }

          // Pawn shield: king on ranks 7-8 (row indices 0 and 1)
          {
            const int king_row = index / 8;
            const int king_file = index % 8;
            if (king_row <= 1) {
              const int f0 = (king_file > 0) ? king_file - 1 : 0;
              const int f1 = (king_file < 7) ? king_file + 1 : 7;
              for (int f = f0; f <= f1; ++f) {
                const bb_t rank7 = BB_1 << (8 + f);
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

  // Connected rooks: apply bonus once per side if any rook can see another
  if (count_bits(board->bitboards[W_ROOK]) >= 2) {
    bb_t rooks = board->bitboards[W_ROOK];
    while (rooks) {
      const index_t sq = get_lsb_index(rooks);
      if (get_rook_attacks(tables, sq, board->occupancies[BOTH]) &
          board->bitboards[W_ROOK]) {
        mg_score += MG_CONNECTED_ROOKS;
        eg_score += EG_CONNECTED_ROOKS;
        break;
      }
      POP_BIT(rooks, sq);
    }
  }
  if (count_bits(board->bitboards[B_ROOK]) >= 2) {
    bb_t rooks = board->bitboards[B_ROOK];
    while (rooks) {
      const index_t sq = get_lsb_index(rooks);
      if (get_rook_attacks(tables, sq, board->occupancies[BOTH]) &
          board->bitboards[B_ROOK]) {
        mg_score -= MG_CONNECTED_ROOKS;
        eg_score -= EG_CONNECTED_ROOKS;
        break;
      }
      POP_BIT(rooks, sq);
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
      case TO_QUEEN:
        promo_score = 20000;
        break;
      case TO_ROOK:
        promo_score = 15000;
        break;
      case TO_KNIGHT:
      case TO_BISHOP:
        promo_score = 10000;
        break;
      default:
        break;
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
             move ==
                 state->counter_moves[MOVE_PIECE(state->node_prev_move[ply])]
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


int get_max_gain()
{
  return MG_QUEEN;
}

int get_margin_value()
{
  return MG_PAWN;
}

int get_piece_value(piece_t piece)
{
  switch (piece) {
    case W_PAWN:
    case B_PAWN:
      return MG_PAWN;
    case W_KNIGHT:
    case B_KNIGHT:
      return MG_KNIGHT;
    case W_BISHOP:
    case B_BISHOP:
      return MG_BISHOP;
    case W_ROOK:
    case B_ROOK:
      return MG_ROOK;
    case W_QUEEN:
    case B_QUEEN:
      return MG_QUEEN;
    case W_KING:
    case B_KING:
      return VALUE_KING;
    default:
      return 0;
  }
}

int get_futility_margin()
{
  return MG_ROOK;
}
int get_reverse_futility_margin()
{
  return MG_PAWN + 20;
}
int get_delta_margin()
{
  return 200;
}
