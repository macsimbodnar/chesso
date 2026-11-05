// S102's research pass, 2026-11-04: where each candidate outpost and space feature fires on a
// corpus, by phase, how much of it the existing design already spans, and (with --k, a training
// corpus only) what the labels read beside the shipped evaluation for each feature with its two
// halves free. Counts on the board; no fit, no timing, no chess judgement: every feature is a
// count of squares and men selected by pawn attacks, files and ranks. Read into
// adocs/data/S102_census.txt by adocs/data/S102_census.sh.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I src -I tools
//       adocs/data/S102_census.cpp build/src/libchesso_engine_trace.a -lpthread -o census
//   census <corpus.tsv> [--k K] [--r2 STRIDE] [--limit N]
//
// Built with -DS102_PLANT the bitboard form takes the enemy span without its neighbour files; the
// walk below must then differ, which is what says the walk can fail (adocs/data/S102_census.sh
// plant).
//
// Without --k only the FEN and game columns are read. --k reads the result column too, and is
// passed for a training corpus only: the held-out views keep their labels unread by anything that
// chooses a form. --r2 STRIDE takes every STRIDE-th row into the collinearity sample (0: none).
//
// Every definition is chesso's own and is said to be (DEC-221). White's squares are given; Black's
// are their mirror, rank r to 9 - r. "Own rank" is the side's own: White's 2 is Black's 7.
//
//   span[c]     every square a pawn of c's can ever attack as it advances: the files beside
//               c's front span (src/evaluation.cpp pawn_pass's `attack_span`)
//   attacks[c]  the squares c's pawns attack now
//   hole[c]     a square on own ranks 4 to 6 that no enemy pawn can ever attack: not in the
//               enemy's span (CPW *Outposts*' "no longer attackable by opponent pawns"); O_N7
//               alone reads the same test on own rank 7. The O_ variants read this region.
//   page[c]     the page's region, CPW *Outposts*' "in the center or on the opponent's half of
//               the board": CPW *Center*'s "four inner squares {d4, e4, d5, e5}" and own ranks 5
//               to 8, so own rank 4 on files d and e and every square of own ranks 5 to 8. The Q_
//               variants read holes and outposts in this region, and are the proposal's.
//   outpost[c]  a hole an own pawn attacks (CPW's "defended by an own pawn")
//   rear[c]     the squares behind an own pawn on its file, strictly (CPW *Space*'s "rearspan")
//   space[c]    files c to f, own ranks 2 to 4 (CPW *Space*'s description of one engine's area)
//   safe[c]     a space square no own pawn stands on and no enemy pawn attacks (chesso's own
//               reading of the page's undefined "safe squares for minor pieces")
//   open        files with no pawn of either colour
//
// The variants are listed with variant_t below, each a count per side; a term is White's count
// less Black's.
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_target.hpp"
#include "tuner_trace.hpp"

namespace
{

enum variant_t
{
  O_N,    // knights on an outpost, own ranks 4 to 6
  O_N4,   // the same on own rank 4
  O_N5,   // own rank 5
  O_N6,   // own rank 6
  O_N7,   // knights on an outpost on own rank 7 (outside O_N)
  O_NC,   // O_N on files c to f
  O_NH,   // knights on a hole, own ranks 4 to 6, that no own pawn attacks
  O_N2,   // O_N whose square two own pawns attack
  O_NX,   // O_N where the enemy has no knight and no bishop on that square's colour
  O_B,    // bishops on an outpost, own ranks 4 to 6
  O_RCH,  // own knights attacking an outpost square, ranks 4 to 6, no own man on it
  O_SQ,   // outpost squares, own ranks 4 to 6, whatever stands there (pawns alone)
  S_P,    // safe space squares off an own pawn's rearspan
  S_R,    // safe space squares on an own pawn's rearspan
  S_PUB,  // S_P + 2 S_R, the page's "counting twice"
  S_W1,   // S_PUB x (own knights, bishops, rooks, queens - open files)
  S_W2,   // S_PUB x (own men but the king - open files)
  S_A,    // S_PUB with "safe" also refusing a square any enemy man attacks (stage two's sets)
  S_XP,   // S_PUB x own knights, bishops, rooks, queens: S_W1's first factor apart
  S_XM,   // S_PUB x own men but the king: S_W2's first factor apart
  S_XO,   // S_PUB x open files: both multipliers' second factor
  Q_N,    // knights on an outpost in the page's region
  Q_N4,   // of them own rank 4 (files d and e)
  Q_N5,   // own rank 5
  Q_N6,   // own rank 6
  Q_N7,   // own rank 7
  Q_N8,   // own rank 8
  Q_N78,  // own ranks 7 and 8
  Q_NX,   // Q_N where the enemy has no knight and no bishop on that square's colour
  Q_NH,   // knights on a hole in the page's region that no own pawn attacks
  Q_N2,   // Q_N whose square two own pawns attack
  Q_B,    // bishops on an outpost in the page's region
  Q_RCH,  // own knights with a jump onto a page-region outpost square no own man holds: per knight
  Q_RCHS, // the same set-wise: own knights in the knight moves of those squares
  O_N4W,  // knights on an outpost on own rank 4 off files d and e: O_N's region outside the page's
  V_COUNT
};

const char* vname[V_COUNT] = {"O_N",  "O_N4",  "O_N5", "O_N6", "O_N7", "O_NC",
                              "O_NH", "O_N2",  "O_NX", "O_B",  "O_RCH", "O_SQ",
                              "S_P",  "S_R",   "S_PUB", "S_W1", "S_W2", "S_A",
                              "S_XP", "S_XM",  "S_XO",
                              "Q_N",  "Q_N4",  "Q_N5", "Q_N6", "Q_N7", "Q_N8", "Q_N78",
                              "Q_NX", "Q_NH",  "Q_N2", "Q_B",  "Q_RCH", "Q_RCHS", "O_N4W"};

constexpr bb_t FILE_A = 0x0101010101010101ULL;
constexpr bb_t FILE_H = FILE_A << 7;

constexpr bb_t row_mask(int row) { return bb_t{0xFF} << (8 * row); }

// Own rank r's row: White's 8 - r, Black's r - 1 (index 0 is a8).
constexpr int row_of(int colour, int rank) { return (colour == WHITE) ? 8 - rank : rank - 1; }

constexpr bb_t ranks(int colour, int lo, int hi)
{
  bb_t out = 0;
  for (int r = lo; r <= hi; ++r) { out |= row_mask(row_of(colour, r)); }
  return out;
}

constexpr bb_t FILES_CF = (FILE_A << 2) | (FILE_A << 3) | (FILE_A << 4) | (FILE_A << 5);

bb_t page_region(int colour)
{
  return (ranks(colour, 4, 4) & ((FILE_A << 3) | (FILE_A << 4))) | ranks(colour, 5, 8);
}

bb_t light_squares()
{
  bb_t out = 0;
  for (int sq = 0; sq < 64; ++sq) {
    if (((sq / 8) + (sq % 8)) % 2 == 0) { out |= bb_t{1} << sq; }
  }
  return out;
}
const bb_t LIGHT = light_squares();

bb_t fill8(bb_t x)
{
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  return x;
}
bb_t fill1(bb_t x)
{
  x |= x << 8;
  x |= x << 16;
  x |= x << 32;
  return x;
}
bb_t beside(bb_t x) { return ((x << 1) & ~FILE_A) | ((x >> 1) & ~FILE_H); }

constexpr bb_t FILE_B = FILE_A << 1;
constexpr bb_t FILE_G = FILE_A << 6;
constexpr bb_t FILES_DE = (FILE_A << 3) | (FILE_A << 4);

// Every square a knight on a square of `s` jumps to. Knight moves are symmetric, so it is also
// every square from which a knight jumps onto `s`. Index 0 is a8 and a file is the index's low
// three bits, so a shift by 17 is two rows down and one file right, and each mask refuses the
// files a jump of that shape cannot land on without wrapping.
bb_t knight_moves(bb_t s)
{
  const bb_t not_a = ~FILE_A, not_h = ~FILE_H;
  const bb_t not_ab = ~(FILE_A | FILE_B), not_gh = ~(FILE_G | FILE_H);
  return ((s << 17) & not_a) | ((s << 15) & not_h) | ((s << 10) & not_ab) |
         ((s << 6) & not_gh) | ((s >> 17) & not_h) | ((s >> 15) & not_a) |
         ((s >> 10) & not_gh) | ((s >> 6) & not_ab);
}

// The page's region for colour c: own rank 4 on files d and e, and own ranks 5 to 8.
bb_t page_region(int colour);

struct sides_t
{
  int v[2][V_COUNT];
};

// The bitboard form. `all_attacks` is stage two's `all` per colour (src/evaluation.hpp
// threat_family_t), read for S_A alone.
void features(const board_t* b, const bb_t all_attacks[2], sides_t* out)
{
  const bb_tables_t* tables = game_tables();
  const bb_t pawns[2] = {b->bitboards[W_PAWN], b->bitboards[B_PAWN]};
  const bb_t front[2] = {fill8(pawns[0]) >> 8, fill1(pawns[1]) << 8};
#ifndef S102_PLANT
  const bb_t span[2] = {beside(front[0]), beside(front[1])};
#else
  // The plant: the span taken without its neighbour files, the edge bug pawn_pass's comment over
  // neighbour_files() names. The walk must read it.
  const bb_t span[2] = {front[0], front[1]};
#endif
  const bb_t attacks[2] = {beside(pawns[0]) >> 8, beside(pawns[1]) << 8};
  const bb_t twice[2] = {((pawns[0] & ~FILE_A) >> 9) & ((pawns[0] & ~FILE_H) >> 7),
                         ((pawns[1] & ~FILE_A) << 7) & ((pawns[1] & ~FILE_H) << 9)};
  const bb_t rear[2] = {fill1(pawns[0]) << 8, fill8(pawns[1]) >> 8};
  const bb_t any_file = fill8(pawns[0] | pawns[1]) | fill1(pawns[0] | pawns[1]);
  const int open = 8 - count_bits(any_file & row_mask(0));

  for (int c = 0; c < 2; ++c) {
    const int e = 1 - c;
    const int o = (c == WHITE) ? 0 : B_PAWN;
    const int eo = (c == WHITE) ? B_PAWN : 0;
    const bb_t knights = b->bitboards[o + W_KNIGHT];
    const bb_t bishops = b->bitboards[o + W_BISHOP];
    const bb_t own_men = b->occupancies[c];
    const bb_t hole46 = ranks(c, 4, 6) & ~span[e];
    const bb_t out46 = hole46 & attacks[c];
    const bb_t out7 = ranks(c, 7, 7) & ~span[e] & attacks[c];
    const bb_t enemy_bishops = b->bitboards[eo + W_BISHOP];
    const bool enemy_knight = b->bitboards[eo + W_KNIGHT] != 0;
    bb_t unexchangeable = 0;
    if (!enemy_knight) {
      if ((enemy_bishops & LIGHT) == 0) { unexchangeable |= LIGHT; }
      if ((enemy_bishops & ~LIGHT) == 0) { unexchangeable |= ~LIGHT; }
    }

    int* v = out->v[c];
    v[O_N] = count_bits(knights & out46);
    v[O_N4] = count_bits(knights & out46 & ranks(c, 4, 4));
    v[O_N5] = count_bits(knights & out46 & ranks(c, 5, 5));
    v[O_N6] = count_bits(knights & out46 & ranks(c, 6, 6));
    v[O_N7] = count_bits(knights & out7);
    v[O_NC] = count_bits(knights & out46 & FILES_CF);
    v[O_NH] = count_bits(knights & hole46 & ~attacks[c]);
    v[O_N2] = count_bits(knights & out46 & twice[c]);
    v[O_NX] = count_bits(knights & out46 & unexchangeable);
    v[O_B] = count_bits(bishops & out46);
    v[O_SQ] = count_bits(out46);

    int reach = 0;
    for (bb_t k = knights; k; k &= k - 1) {
      if ((tables->knight_attacks[get_lsb_index(k)] & out46 & ~own_men) != 0) { reach++; }
    }
    v[O_RCH] = reach;

    // The page's region.
    const bb_t region = page_region(c);
    const bb_t qhole = region & ~span[e];
    const bb_t qout = qhole & attacks[c];
    v[Q_N] = count_bits(knights & qout);
    v[Q_N4] = count_bits(knights & qout & ranks(c, 4, 4));
    v[Q_N5] = count_bits(knights & qout & ranks(c, 5, 5));
    v[Q_N6] = count_bits(knights & qout & ranks(c, 6, 6));
    v[Q_N7] = count_bits(knights & qout & ranks(c, 7, 7));
    v[Q_N8] = count_bits(knights & qout & ranks(c, 8, 8));
    v[Q_N78] = v[Q_N7] + v[Q_N8];
    v[Q_NX] = count_bits(knights & qout & unexchangeable);
    v[Q_NH] = count_bits(knights & qhole & ~attacks[c]);
    v[Q_N2] = count_bits(knights & qout & twice[c]);
    v[Q_B] = count_bits(bishops & qout);
    int qreach = 0;
    for (bb_t k = knights; k; k &= k - 1) {
      if ((tables->knight_attacks[get_lsb_index(k)] & qout & ~own_men) != 0) { qreach++; }
    }
    v[Q_RCH] = qreach;
    v[Q_RCHS] = count_bits(knights & knight_moves(qout & ~own_men));
    v[O_N4W] = count_bits(knights & out46 & ranks(c, 4, 4) & ~FILES_DE);

    const bb_t area = ranks(c, 2, 4) & FILES_CF;
    const bb_t safe = area & ~pawns[c] & ~attacks[e];
    v[S_P] = count_bits(safe & ~rear[c]);
    v[S_R] = count_bits(safe & rear[c]);
    v[S_PUB] = v[S_P] + 2 * v[S_R];
    const int pieces = count_bits(own_men & ~pawns[c] & ~b->bitboards[o + W_KING]);
    const int men = count_bits(own_men & ~b->bitboards[o + W_KING]);
    v[S_W1] = v[S_PUB] * (pieces - open);
    v[S_W2] = v[S_PUB] * (men - open);
    v[S_XP] = v[S_PUB] * pieces;
    v[S_XM] = v[S_PUB] * men;
    v[S_XO] = v[S_PUB] * open;
    const bb_t safe_a = area & ~pawns[c] & ~all_attacks[e];
    v[S_A] = count_bits(safe_a) + count_bits(safe_a & rear[c]);
  }
}

// The same counts walked square by square from the definitions above, for a check of the bitboard
// form on every row. Pieces from the board's own bitboards; no pass of the engine's is read.
int sq_of(int row, int file) { return row * 8 + file; }
bool has(bb_t set, int row, int file)
{
  return row >= 0 && row < 8 && file >= 0 && file < 8 && ((set >> sq_of(row, file)) & 1);
}

void walked(const board_t* b, sides_t* out)
{
  const bb_t pawns[2] = {b->bitboards[W_PAWN], b->bitboards[B_PAWN]};
  int open = 0;
  for (int f = 0; f < 8; ++f) {
    bool any = false;
    for (int r = 0; r < 8; ++r) { any = any || has(pawns[0] | pawns[1], r, f); }
    if (!any) { open++; }
  }

  for (int c = 0; c < 2; ++c) {
    const int e = 1 - c;
    const int fwd = (c == WHITE) ? -1 : 1;  // the row step toward c's promotion
    const int o = (c == WHITE) ? 0 : B_PAWN;
    const int eo = (c == WHITE) ? B_PAWN : 0;
    int* v = out->v[c];
    for (int i = 0; i < V_COUNT; ++i) { v[i] = 0; }

    auto own_rank = [&](int row) { return (c == WHITE) ? 8 - row : row + 1; };
    // An enemy pawn can ever attack (row, file): one on a neighbouring file that stands nearer
    // its own back rank than the square, i.e. on the far side of the square from c's view.
    auto attackable = [&](int row, int file) {
      for (int df = -1; df <= 1; df += 2) {
        for (int r = 0; r < 8; ++r) {
          if (!has(pawns[e], r, file + df)) { continue; }
          if ((c == WHITE && r < row) || (c == BLACK && r > row)) { return true; }
        }
      }
      return false;
    };
    auto defenders = [&](int row, int file) {
      return int(has(pawns[c], row - fwd, file - 1)) + int(has(pawns[c], row - fwd, file + 1));
    };
    auto enemy_attacks_now = [&](int row, int file) {
      return has(pawns[e], row + fwd, file - 1) || has(pawns[e], row + fwd, file + 1);
    };
    auto behind_own = [&](int row, int file) {
      for (int r = row + fwd; r >= 0 && r < 8; r += fwd) {
        if (has(pawns[c], r, file)) { return true; }
      }
      return false;
    };
    const bb_t enemy_bishops = b->bitboards[eo + W_BISHOP];
    const bool enemy_knight = b->bitboards[eo + W_KNIGHT] != 0;
    auto exchangeable = [&](int row, int file) {
      if (enemy_knight) { return true; }
      const bool light = ((row + file) % 2) == 0;
      for (bb_t x = enemy_bishops; x; x &= x - 1) {
        const int s = get_lsb_index(x);
        if ((((s >> 3) + (s & 7)) % 2 == 0) == light) { return true; }
      }
      return false;
    };
    auto is_outpost46 = [&](int row, int file) {
      const int rk = own_rank(row);
      return rk >= 4 && rk <= 6 && !attackable(row, file) && defenders(row, file) > 0;
    };

    for (int row = 0; row < 8; ++row) {
      for (int file = 0; file < 8; ++file) {
        const int rk = own_rank(row);
        const bool knight = has(b->bitboards[o + W_KNIGHT], row, file);
        const bool bishop = has(b->bitboards[o + W_BISHOP], row, file);
        if (rk >= 4 && rk <= 7 && !attackable(row, file)) {
          const int d = defenders(row, file);
          if (rk == 7) {
            if (knight && d > 0) { v[O_N7]++; }
          } else {
            if (d > 0) {
              v[O_SQ]++;
              if (knight) {
                v[O_N]++;
                v[(rk == 4) ? O_N4 : (rk == 5) ? O_N5 : O_N6]++;
                if (file >= 2 && file <= 5) { v[O_NC]++; }
                if (d == 2) { v[O_N2]++; }
                if (!exchangeable(row, file)) { v[O_NX]++; }
              }
              if (bishop) { v[O_B]++; }
            } else if (knight) {
              v[O_NH]++;
            }
          }
        }
        if (rk >= 2 && rk <= 4 && file >= 2 && file <= 5 && !has(pawns[c], row, file) &&
            !enemy_attacks_now(row, file)) {
          if (behind_own(row, file)) {
            v[S_R]++;
          } else {
            v[S_P]++;
          }
        }
      }
    }

    // The page's region: own rank 4 on files d and e, own ranks 5 to 8.
    auto in_page = [&](int row, int file) {
      const int rk = own_rank(row);
      return (rk == 4 && (file == 3 || file == 4)) || rk >= 5;
    };
    auto is_outpost_page = [&](int row, int file) {
      return in_page(row, file) && !attackable(row, file) && defenders(row, file) > 0;
    };
    for (int row = 0; row < 8; ++row) {
      for (int file = 0; file < 8; ++file) {
        const int rk = own_rank(row);
        const bool knight = has(b->bitboards[o + W_KNIGHT], row, file);
        const bool bishop = has(b->bitboards[o + W_BISHOP], row, file);
        if (rk == 4 && file != 3 && file != 4 && knight && !attackable(row, file) &&
            defenders(row, file) > 0) {
          v[O_N4W]++;
        }
        if (!in_page(row, file) || attackable(row, file)) { continue; }
        const int d = defenders(row, file);
        if (d > 0) {
          if (knight) {
            v[Q_N]++;
            v[(rk == 4) ? Q_N4 : (rk == 5) ? Q_N5 : (rk == 6) ? Q_N6 : (rk == 7) ? Q_N7 : Q_N8]++;
            if (!exchangeable(row, file)) { v[Q_NX]++; }
            if (d == 2) { v[Q_N2]++; }
          }
          if (bishop) { v[Q_B]++; }
        } else if (knight) {
          v[Q_NH]++;
        }
      }
    }
    v[Q_N78] = v[Q_N7] + v[Q_N8];
    for (bb_t k = b->bitboards[o + W_KNIGHT]; k; k &= k - 1) {
      const int s = get_lsb_index(k);
      const int r0 = s >> 3, f0 = s & 7;
      const int jumps[8][2] = {{1, 2}, {2, 1}, {-1, 2}, {-2, 1}, {1, -2}, {2, -1}, {-1, -2}, {-2, -1}};
      bool hit = false;
      for (const auto& j : jumps) {
        const int r = r0 + j[0], f = f0 + j[1];
        if (r < 0 || r > 7 || f < 0 || f > 7) { continue; }
        if (is_outpost_page(r, f) && !has(b->occupancies[c], r, f)) { hit = true; }
      }
      if (hit) { v[Q_RCH]++; }
    }
    // The set-wise form is held to the same walk: a knight is in the knight moves of a square
    // set exactly when one of its jumps lands in the set.
    v[Q_RCHS] = v[Q_RCH];

    // Reach: a knight's eight jumps, each landing on an outpost square no own man holds.
    for (bb_t k = b->bitboards[o + W_KNIGHT]; k; k &= k - 1) {
      const int s = get_lsb_index(k);
      const int r0 = s >> 3, f0 = s & 7;
      const int jumps[8][2] = {{1, 2}, {2, 1}, {-1, 2}, {-2, 1}, {1, -2}, {2, -1}, {-1, -2}, {-2, -1}};
      bool hit = false;
      for (const auto& j : jumps) {
        const int r = r0 + j[0], f = f0 + j[1];
        if (r < 0 || r > 7 || f < 0 || f > 7) { continue; }
        if (is_outpost46(r, f) && !has(b->occupancies[c], r, f)) { hit = true; }
      }
      if (hit) { v[O_RCH]++; }
    }

    v[S_PUB] = v[S_P] + 2 * v[S_R];
    int pieces = 0, men = 0;
    for (int t = W_KNIGHT; t <= W_QUEEN; ++t) { pieces += count_bits(b->bitboards[o + t]); }
    men = pieces + count_bits(pawns[c]);
    v[S_W1] = v[S_PUB] * (pieces - open);
    v[S_W2] = v[S_PUB] * (men - open);
    v[S_XP] = v[S_PUB] * pieces;
    v[S_XM] = v[S_PUB] * men;
    v[S_XO] = v[S_PUB] * open;
  }
}

constexpr int MAX_GAME = 1 << 21;
constexpr int PHASE_BINS = 6;
int phase_bin(int phase) { return (phase <= 4) ? 0 : std::min((phase - 1) / 4, PHASE_BINS - 1); }
const char* phase_name[PHASE_BINS] = {"0-4", "5-8", "9-12", "13-16", "17-20", "21-24"};

struct games_t
{
  std::vector<bool> seen;
  uint64_t count = 0;
  games_t() : seen(MAX_GAME, false) {}
  void note(int64_t game)
  {
    if (game < 0 || game >= MAX_GAME) {
      fprintf(stderr, "game id %lld past the census's table\n", (long long)game);
      exit(1);
    }
    if (!seen[game]) {
      seen[game] = true;
      count++;
    }
  }
};

struct tally_t
{
  uint64_t fires = 0;    // either side's count non-zero
  uint64_t differs = 0;  // White's less Black's non-zero
  uint64_t fires_bin[PHASE_BINS] = {};
  uint64_t differs_bin[PHASE_BINS] = {};
  uint64_t side_fires = 0;  // side-rows with a non-zero count, of 2 x rows
  long long side_sum = 0;
  int side_max = 0;
  int diff_min = 0, diff_max = 0;
  games_t games_differs;
};

bool split(const std::string& line, std::vector<std::string>* out)
{
  out->clear();
  size_t start = 0;
  while (true) {
    const size_t tab = line.find('\t', start);
    out->push_back(line.substr(start, tab - start));
    if (tab == std::string::npos) { break; }
    start = tab + 1;
  }
  return out->size() >= 5;
}

// Ordinary least squares on a dense design with an intercept, by normal equations: the Gram
// matrix is factored once (Cholesky) and each target's R^2 read off its own right-hand side. A
// pivot under 1e-9 of its diagonal is a column the earlier ones span and is dropped: the existing
// design has exact dependencies by construction (S275's gauges, the pawn columns of ranks 1 and 8).
struct factor_t
{
  size_t width = 0;
  std::vector<double> l;
  std::vector<char> dead;
};

factor_t factor_gram(std::vector<double> gram, size_t width)
{
  factor_t f;
  f.width = width;
  f.dead.assign(width, 0);
  std::vector<double> diag(width);
  for (size_t a = 0; a < width; ++a) { diag[a] = gram[a * width + a]; }
  for (size_t a = 0; a < width; ++a) {
    for (size_t b = 0; b <= a; ++b) {
      if (f.dead[b]) {
        gram[a * width + b] = 0.0;
        continue;
      }
      double sum = gram[a * width + b];
      for (size_t c = 0; c < b; ++c) { sum -= gram[a * width + c] * gram[b * width + c]; }
      if (a == b) {
        if (diag[a] <= 0.0 || sum <= 1e-9 * diag[a]) {
          f.dead[a] = 1;
          gram[a * width + a] = 0.0;
        } else {
          gram[a * width + a] = std::sqrt(sum);
        }
      } else {
        gram[a * width + b] = sum / gram[b * width + b];
      }
    }
  }
  f.l = std::move(gram);
  return f;
}

// R^2 of the target whose raw sums are yy (y.y), ysum and the cross products `rhs`.
double r_squared(const factor_t& f, const std::vector<double>& rhs, double yy, double ysum,
                 double n)
{
  const size_t w = f.width;
  std::vector<double> z(w, 0.0);
  double explained = 0.0;
  for (size_t a = 0; a < w; ++a) {
    if (f.dead[a]) { continue; }
    double sum = rhs[a];
    for (size_t c = 0; c < a; ++c) { sum -= f.l[a * w + c] * z[c]; }
    z[a] = sum / f.l[a * w + a];
    explained += z[a] * z[a];
  }
  const double tss = yy - ysum * ysum / n;
  if (tss <= 0.0) { return -1.0; }
  return 1.0 - (yy - explained) / tss;
}

size_t live_columns(const factor_t& f)
{
  size_t live = 0;
  for (char d : f.dead) { live += d ? 0 : 1; }
  return live;
}

struct options_t
{
  std::string path;
  double k = 0.0;
  bool labels = false;
  size_t r2_stride = 0;
  uint64_t limit = 0;
};

// The label readings: each listed set of variants with its two halves free, every other weight
// at the shipped constants, one Gauss-Newton step from zero on the in-sample squared error, then
// the error read exactly at that step, whole-game errors. A reading and never a fit.
struct reading_t
{
  const char* name;
  std::vector<int> vars;
};

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--k" && i + 1 < argc) {
      opts.k = atof(argv[++i]);
      opts.labels = true;
    } else if (a == "--r2" && i + 1 < argc) {
      opts.r2_stride = strtoull(argv[++i], nullptr, 10);
    } else if (a == "--limit" && i + 1 < argc) {
      opts.limit = strtoull(argv[++i], nullptr, 10);
    } else if (opts.path.empty()) {
      opts.path = a;
    } else {
      fprintf(stderr, "unknown argument %s\n", a.c_str());
      return 2;
    }
  }
  if (opts.path.empty()) {
    fprintf(stderr, "census <corpus.tsv> [--k K] [--r2 STRIDE] [--limit N]\n");
    return 2;
  }

  std::ifstream in(opts.path);
  if (!in) {
    fprintf(stderr, "cannot read %s\n", opts.path.c_str());
    return 1;
  }

  game_t game;
  initialize_game_const_data(&game);

  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();
  const tuner_trace::feature_table_t& ft = tuner_trace::feature_table();

  const bool need_trace = opts.labels || opts.r2_stride > 0;

  uint64_t rows = 0, walk_differ = 0, mismatches = 0, game_reopened = 0;
  uint64_t rows_bin[PHASE_BINS] = {};
  games_t all_games;
  std::vector<tally_t> tally(V_COUNT);
  uint64_t weight_negative[2] = {0, 0};  // side-rows with the S_W1, S_W2 multiplier < 0
  uint64_t reach_forms_differ = 0;        // side-rows where Q_RCH and Q_RCHS disagree
  uint64_t pawnless = 0, pawnless_unequal = 0;  // rows with no pawn; of them knights to queens unequal
  games_t pawnless_games;

  // Stored per row for the label readings.
  std::vector<float> st_score, st_result;
  std::vector<uint8_t> st_phase;
  std::vector<int32_t> st_game;
  std::vector<int16_t> st_diff;  // V_COUNT a row

  // The collinearity sample: the trace's entries and the variants' differences.
  std::vector<std::vector<tuner_trace::entry_t>> r2_entries;
  std::vector<uint8_t> r2_phase;
  std::vector<std::array<int16_t, V_COUNT>> r2_diff;

  std::string line, why;
  std::vector<std::string> fields;
  tuner_trace::row_trace_t trace;
  int64_t last_game = -1;
  std::vector<bool> closed(MAX_GAME, false);

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }
    if (opts.limit != 0 && rows >= opts.limit) { break; }
    if (!split(line, &fields)) {
      fprintf(stderr, "a row with fewer than five fields: %s\n", line.c_str());
      return 1;
    }
    const std::string& fen = fields[0];
    const int64_t game_id = strtoll(fields[4].c_str(), nullptr, 10);
    all_games.note(game_id);
    if (game_id != last_game) {
      if (last_game >= 0) { closed[last_game] = true; }
      if (closed[game_id]) { game_reopened++; }
      last_game = game_id;
    }

    if (!load_FEN(fen, &game, &why)) {
      fprintf(stderr, "the engine refuses %s\n", fen.c_str());
      return 1;
    }
    const board_t* board = &game.board;
    const int phase = game_phase(board);

    threat_family_t threats;
    threat_family_counts(board, &threats);

    sides_t f, w;
    features(board, threats.all, &f);
    walked(board, &w);
    for (int c = 0; c < 2; ++c) {
      for (int v = 0; v < V_COUNT; ++v) {
        if (v == S_A) { continue; }  // stage two's sets: no walk of them here
        if (f.v[c][v] != w.v[c][v]) {
          walk_differ++;
          if (walk_differ <= 5) {
            fprintf(stderr, "walk differs: %s side %d %s bit %d walk %d\n", fen.c_str(), c,
                    vname[v], f.v[c][v], w.v[c][v]);
          }
        }
      }
    }

    rows++;
    rows_bin[phase_bin(phase)]++;
    for (int c = 0; c < 2; ++c) { reach_forms_differ += f.v[c][Q_RCH] != f.v[c][Q_RCHS]; }
    if ((board->bitboards[W_PAWN] | board->bitboards[B_PAWN]) == 0) {
      pawnless++;
      pawnless_games.note(game_id);
      const bb_t* bbs = board->bitboards;
      const int pw = count_bits(bbs[W_KNIGHT] | bbs[W_BISHOP] | bbs[W_ROOK] | bbs[W_QUEEN]);
      const int pb = count_bits(bbs[B_KNIGHT] | bbs[B_BISHOP] | bbs[B_ROOK] | bbs[B_QUEEN]);
      if (pw != pb) { pawnless_unequal++; }
    }
    int16_t diff[V_COUNT];
    for (int v = 0; v < V_COUNT; ++v) {
      tally_t& t = tally[v];
      const int a = f.v[WHITE][v], b = f.v[BLACK][v];
      const int d = a - b;
      diff[v] = static_cast<int16_t>(d);
      if (a != 0 || b != 0) {
        t.fires++;
        t.fires_bin[phase_bin(phase)]++;
      }
      if (d != 0) {
        t.differs++;
        t.differs_bin[phase_bin(phase)]++;
        t.games_differs.note(game_id);
      }
      t.side_fires += (a != 0) + (b != 0);
      t.side_sum += std::abs(a) + std::abs(b);
      t.side_max = std::max({t.side_max, std::abs(a), std::abs(b)});
      t.diff_min = std::min(t.diff_min, d);
      t.diff_max = std::max(t.diff_max, d);
    }
    {
      // The multipliers' signs, per side-row.
      const bb_t pawns = board->bitboards[W_PAWN] | board->bitboards[B_PAWN];
      const bb_t any_file = fill8(pawns) | fill1(pawns);
      const int open = 8 - count_bits(any_file & row_mask(0));
      for (int c = 0; c < 2; ++c) {
        const int o = (c == WHITE) ? 0 : B_PAWN;
        const bb_t men = board->occupancies[c] & ~board->bitboards[o + W_KING];
        const int pieces = count_bits(men & ~board->bitboards[o + W_PAWN]);
        if (pieces - open < 0) { weight_negative[0]++; }
        if (count_bits(men) - open < 0) { weight_negative[1]++; }
      }
    }

    const bool r2_row = opts.r2_stride > 0 && (rows - 1) % opts.r2_stride == 0;
    if (need_trace && (opts.labels || r2_row)) {
      if (!tuner_trace::trace_board(board, &trace, &why)) {
        fprintf(stderr, "trace refused (%s): %s\n", why.c_str(), fen.c_str());
        return 1;
      }
      tuner_trace::dataset_t one;
      tuner_trace::append_row(trace, 0.5f, 0, 0, &one);
      if (tuner_trace::exact_score(one, 0, weights.data()) != trace.engine_white) {
        mismatches++;
      }
      if (opts.labels) {
        st_score.push_back(static_cast<float>(tuner_trace::score(one, 0, params.data())));
        st_result.push_back(static_cast<float>(atof(fields[1].c_str())));
        st_phase.push_back(static_cast<uint8_t>(trace.phase));
        st_game.push_back(static_cast<int32_t>(game_id));
        st_diff.insert(st_diff.end(), diff, diff + V_COUNT);
      }
      if (r2_row) {
        r2_entries.push_back(trace.entries);
        r2_phase.push_back(static_cast<uint8_t>(trace.phase));
        std::array<int16_t, V_COUNT> d;
        for (int v = 0; v < V_COUNT; ++v) { d[v] = diff[v]; }
        r2_diff.push_back(d);
      }
    }
  }

  printf("S102 census of %s\n", opts.path.c_str());
  printf("rows %llu games %llu; game blocks reopened %llu\n", (unsigned long long)rows,
         (unsigned long long)all_games.count, (unsigned long long)game_reopened);
  printf("walk: %llu side-counts differ from the bitboard form (every variant but S_A, every row)\n",
         (unsigned long long)walk_differ);
  if (need_trace) {
    printf("identity: %llu traced rows differ from evaluate()\n", (unsigned long long)mismatches);
  }
  printf("rows by phase:");
  for (int i = 0; i < PHASE_BINS; ++i) {
    printf(" %s %.3f%%", phase_name[i], 100.0 * rows_bin[i] / std::max<uint64_t>(rows, 1));
  }
  printf("\nset-wise reach against the per-knight loop: %llu side-counts differ of %llu\n",
         (unsigned long long)reach_forms_differ, (unsigned long long)(2 * rows));
  printf("pawnless rows %llu in %llu games; of them with own knights to queens unequal %llu\n",
         (unsigned long long)pawnless, (unsigned long long)pawnless_games.count,
         (unsigned long long)pawnless_unequal);
  printf("multiplier < 0, of %llu side-rows: S_W1 %.3f%%, S_W2 %.3f%%\n",
         (unsigned long long)(2 * rows), 100.0 * weight_negative[0] / std::max<uint64_t>(2 * rows, 1),
         100.0 * weight_negative[1] / std::max<uint64_t>(2 * rows, 1));

  printf("\nfires: either side's count non-zero; differs: White's count unlike Black's, the rows a "
         "term reads\n");
  printf("variant  fires%%   differs%%  games(differ)  side-row%%  mean|side|  side max  diff "
         "range | differs%% by phase");
  for (int i = 0; i < PHASE_BINS; ++i) { printf(" %s", phase_name[i]); }
  printf("\n");
  for (int v = 0; v < V_COUNT; ++v) {
    const tally_t& t = tally[v];
    printf("%-7s %7.3f %9.3f %13llu %9.3f %10.3f %9d %5d..%d |", vname[v], 100.0 * t.fires / rows,
           100.0 * t.differs / rows, (unsigned long long)t.games_differs.count,
           100.0 * t.side_fires / (2.0 * rows), double(t.side_sum) / (2.0 * rows), t.side_max,
           t.diff_min, t.diff_max);
    for (int i = 0; i < PHASE_BINS; ++i) {
      printf(" %.3f", rows_bin[i] ? 100.0 * t.differs_bin[i] / rows_bin[i] : 0.0);
    }
    printf("\n");
  }

  // ------------------------------------------------------------ collinearity
  if (!r2_entries.empty()) {
    const size_t n = r2_entries.size();
    printf("\ncollinearity, %zu rows (every %zuth): R^2 of each variant's column\n", n,
           opts.r2_stride);
    printf("  psqt: the untapered difference on the piece-square columns of the named types, "
           "feature_audit's form, intercept added\n");
    printf("  all mg / all eg: the variant's middlegame (eg) column, its difference times the taper "
           "weight, on every existing parameter's column the trace writes, tapered as the tuner "
           "sees it\n");

    // Untapered psqt occupancy per row: column type*64 + square.
    const uint32_t psqt_base = ft.base[eval_trace::TERM_PSQT];
    auto psqt_r2 = [&](int var, std::vector<int> types) {
      const size_t width = 1 + 64 * types.size();
      std::vector<double> gram(width * width, 0.0), rhs(width, 0.0);
      double yy = 0, ys = 0;
      std::vector<double> x(width);
      for (size_t r = 0; r < n; ++r) {
        std::fill(x.begin(), x.end(), 0.0);
        x[0] = 1.0;
        for (const auto& e : r2_entries[r]) {
          if (e.feature < psqt_base || e.feature >= psqt_base + 6 * 64) { continue; }
          const int idx = e.feature - psqt_base;
          for (size_t t = 0; t < types.size(); ++t) {
            if (idx / 64 == types[t]) { x[1 + 64 * t + idx % 64] += e.coef; }
          }
        }
        const double y = r2_diff[r][var];
        yy += y * y;
        ys += y;
        for (size_t a = 0; a < width; ++a) {
          if (x[a] == 0.0) { continue; }
          rhs[a] += x[a] * y;
          for (size_t b = 0; b < width; ++b) { gram[a * width + b] += x[a] * x[b]; }
        }
      }
      // Raw sums with the intercept column carrying the mean.
      return r_squared(factor_gram(gram, width), rhs, yy, ys, double(n));
    };

    // Every parameter's tapered column: material untapered, the rest c * w_mg and c * w_eg.
    const size_t P = tuner_layout::PARAM_COUNT;
    std::vector<double> gram((P + 1) * (P + 1), 0.0);
    std::vector<std::vector<double>> rhs_mg(V_COUNT, std::vector<double>(P + 1, 0.0));
    std::vector<std::vector<double>> rhs_eg(V_COUNT, std::vector<double>(P + 1, 0.0));
    std::vector<double> yy_mg(V_COUNT, 0), ys_mg(V_COUNT, 0), yy_eg(V_COUNT, 0), ys_eg(V_COUNT, 0);
    std::vector<std::pair<uint32_t, double>> xs;
    for (size_t r = 0; r < n; ++r) {
      const double wm = r2_phase[r] / 24.0, we = 1.0 - wm;
      xs.clear();
      xs.push_back({0, 1.0});
      for (const auto& e : r2_entries[r]) {
        const tuner_trace::feature_t& fe = ft.features[e.feature];
        if (fe.site == tuner_trace::SITE_MATERIAL) {
          xs.push_back({fe.mg + 1, double(e.coef)});
        } else {
          xs.push_back({fe.mg + 1, e.coef * wm});
          if (fe.eg != tuner_trace::NO_PARAM) { xs.push_back({fe.eg + 1, e.coef * we}); }
        }
      }
      for (const auto& [a, va] : xs) {
        for (const auto& [b, vb] : xs) { gram[a * (P + 1) + b] += va * vb; }
      }
      for (int v = 0; v < V_COUNT; ++v) {
        const double ym = r2_diff[r][v] * wm, ye = r2_diff[r][v] * we;
        yy_mg[v] += ym * ym;
        ys_mg[v] += ym;
        yy_eg[v] += ye * ye;
        ys_eg[v] += ye;
        for (const auto& [a, va] : xs) {
          rhs_mg[v][a] += va * ym;
          rhs_eg[v][a] += va * ye;
        }
      }
    }

    struct target_t
    {
      int var;
      std::vector<int> types;
      const char* on;
    };
    const std::vector<target_t> targets = {
        {O_N, {1, 0}, "knight+pawn"},  {O_N4, {1, 0}, "knight+pawn"}, {O_N5, {1, 0}, "knight+pawn"},
        {O_N6, {1, 0}, "knight+pawn"}, {O_N7, {1, 0}, "knight+pawn"}, {O_NC, {1, 0}, "knight+pawn"},
        {O_NH, {1, 0}, "knight+pawn"}, {O_N2, {1, 0}, "knight+pawn"}, {O_NX, {1, 0}, "knight+pawn"},
        {O_B, {2, 0}, "bishop+pawn"},  {O_RCH, {1, 0}, "knight+pawn"}, {O_SQ, {0}, "pawn"},
        {S_P, {0}, "pawn"},            {S_R, {0}, "pawn"},            {S_PUB, {0}, "pawn"},
        {S_W1, {0}, "pawn"},           {S_W2, {0}, "pawn"},           {S_A, {0}, "pawn"},
        {S_XP, {0}, "pawn"},           {S_XM, {0}, "pawn"},           {S_XO, {0}, "pawn"},
        {Q_N, {1, 0}, "knight+pawn"},  {Q_N4, {1, 0}, "knight+pawn"}, {Q_N5, {1, 0}, "knight+pawn"},
        {Q_N6, {1, 0}, "knight+pawn"}, {Q_N78, {1, 0}, "knight+pawn"}, {Q_NX, {1, 0}, "knight+pawn"},
        {Q_NH, {1, 0}, "knight+pawn"}, {Q_B, {2, 0}, "bishop+pawn"},  {Q_RCHS, {1, 0}, "knight+pawn"},
        {O_N4W, {1, 0}, "knight+pawn"},
    };
    const factor_t all = factor_gram(gram, P + 1);
    printf("  the existing design's columns: %zu of %zu live after the dependent ones drop, the "
           "intercept included\n", live_columns(all), P + 1);
    printf("%-7s %10s %-12s %10s %10s\n", "variant", "R2 psqt", "on", "R2 all mg", "R2 all eg");
    for (const target_t& t : targets) {
      const double a = psqt_r2(t.var, t.types);
      const double m = r_squared(all, rhs_mg[t.var], yy_mg[t.var], ys_mg[t.var], double(n));
      const double e = r_squared(all, rhs_eg[t.var], yy_eg[t.var], ys_eg[t.var], double(n));
      printf("%-7s %10.6f %-12s %10.6f %10.6f\n", vname[t.var], a, t.on, m, e);
    }
  }

  // ---------------------------------------------------------- label readings
  if (opts.labels) {
    const size_t n = st_score.size();
    const double kk = opts.k * tuner_target::LN10_OVER_400;
    double mse0 = 0.0;
    std::vector<double> st_p0(n);
    for (size_t r = 0; r < n; ++r) {
      const double p = tuner_target::sigmoid(st_score[r], opts.k);
      st_p0[r] = p;
      mse0 += (st_result[r] - p) * (st_result[r] - p);
    }
    mse0 /= n;
    printf("\nlabel readings: %zu rows, K %.4f, in-sample MSE at the shipped constants %.6f\n", n,
           opts.k, mse0);
    printf("  each set's halves free from zero, every other weight shipped; one Gauss-Newton step "
           "on the squared error; dMSE in 1e-6 with whole-game errors: in sample, at the step "
           "rounded to integers, and across halves (the step from the even-numbered games read on "
           "the odd ones and the odd ones' read on the even, the two summed over all rows); g the "
           "gradient at zero per half, 1e-6 a centipawn, whole-game errors. A reading and never a "
           "fit.\n");

    // Each variant alone, then the sets. O_ sets read own ranks 4 to 6; Q_ sets the page's region.
    std::vector<reading_t> sets;
    for (int v = 0; v < V_COUNT; ++v) { sets.push_back({vname[v], {v}}); }
    const std::vector<int> qp1a = {Q_N4, Q_N5, Q_N6};
    const std::vector<int> qp1 = {Q_N4, Q_N5, Q_N6, Q_N78};
    auto plus = [](std::vector<int> a, std::initializer_list<int> b) {
      a.insert(a.end(), b.begin(), b.end());
      return a;
    };
    sets.push_back({"O_N4+O_N5+O_N6", {O_N4, O_N5, O_N6}});
    sets.push_back({"O_N+O_NH", {O_N, O_NH}});
    sets.push_back({"O_N+O_N2", {O_N, O_N2}});
    sets.push_back({"O_N+O_NX", {O_N, O_NX}});
    sets.push_back({"O_N+O_B", {O_N, O_B}});
    sets.push_back({"O_N+O_RCH", {O_N, O_RCH}});
    sets.push_back({"O_N4..6+O_N7", {O_N4, O_N5, O_N6, O_N7}});
    // The ranks 4 to 6 chain of the first reading, kept for comparison.
    sets.push_back({"o P1..P5", {O_N4, O_N5, O_N6, O_NX, O_NH, O_B, O_RCH}});
    sets.push_back({"o P1..P8", {O_N4, O_N5, O_N6, O_NX, O_NH, O_B, O_RCH, S_PUB, S_XO, S_XP}});
    // The page's region: P1's cells, then the proposal's order.
    sets.push_back({"Q P1 4,5,6", qp1a});
    sets.push_back({"Q P1 4,5,6,78", qp1});
    sets.push_back({"Q P1 4,5,6,7,8", {Q_N4, Q_N5, Q_N6, Q_N7, Q_N8}});
    sets.push_back({"Q P1 +O_N4W", plus(qp1, {O_N4W})});
    sets.push_back({"Q P1 +Q_N2", plus(qp1, {Q_N2})});
    sets.push_back({"Q P2 +Q_NX", plus(qp1, {Q_NX})});
    sets.push_back({"Q P3 +Q_NH", plus(qp1, {Q_NX, Q_NH})});
    sets.push_back({"Q P4 +O_N4W", plus(qp1, {Q_NX, Q_NH, O_N4W})});
    sets.push_back({"Q P5 +Q_B", plus(qp1, {Q_NX, Q_NH, O_N4W, Q_B})});
    sets.push_back({"Q P6 +Q_RCHS", plus(qp1, {Q_NX, Q_NH, O_N4W, Q_B, Q_RCHS})});
    sets.push_back({"Q P7 +S_PUB", plus(qp1, {Q_NX, Q_NH, O_N4W, Q_B, Q_RCHS, S_PUB})});
    sets.push_back({"Q P8 +S_XO", plus(qp1, {Q_NX, Q_NH, O_N4W, Q_B, Q_RCHS, S_PUB, S_XO})});
    sets.push_back(
        {"Q P9 +S_XP", plus(qp1, {Q_NX, Q_NH, O_N4W, Q_B, Q_RCHS, S_PUB, S_XO, S_XP})});
    sets.push_back(
        {"Q P7 +S_W1, the tie (d)", plus(qp1, {Q_NX, Q_NH, O_N4W, Q_B, Q_RCHS, S_PUB, S_W1})});
    // The same parts by signal: reach, the flank, bishops, the hole, then the exchange.
    sets.push_back({"Q sig +Q_RCHS", plus(qp1, {Q_RCHS})});
    sets.push_back({"Q sig +O_N4W", plus(qp1, {Q_RCHS, O_N4W})});
    sets.push_back({"Q sig +Q_B", plus(qp1, {Q_RCHS, O_N4W, Q_B})});
    sets.push_back({"Q sig +Q_NH", plus(qp1, {Q_RCHS, O_N4W, Q_B, Q_NH})});
    // Space alone and its alternatives.
    sets.push_back({"S_P+S_R", {S_P, S_R}});
    sets.push_back({"S_PUB+S_XO", {S_PUB, S_XO}});
    sets.push_back({"S_PUB+S_XO+S_XP", {S_PUB, S_XO, S_XP}});
    sets.push_back({"S_PUB+S_XP", {S_PUB, S_XP}});
    sets.push_back({"S_PUB+S_XM", {S_PUB, S_XM}});
    sets.push_back({"S_PUB+S_XM+S_XO", {S_PUB, S_XM, S_XO}});
    sets.push_back({"S_PUB+S_W1", {S_PUB, S_W1}});
    sets.push_back({"S_PUB+S_W2", {S_PUB, S_W2}});
    sets.push_back({"S_A+S_W1", {S_A, S_W1}});
    sets.push_back({"O_N+S_PUB", {O_N, S_PUB}});

    // The paired increments, across halves: the larger set's per-game dMSE less the smaller's,
    // summed, with the whole-game error of that difference.
    struct increment_t
    {
      const char* what;
      const char* from;
      const char* to;
    };
    const std::vector<increment_t> increments = {
        {"O_N6 alone, over the shipped score", "", "O_N6"},
        {"O_N2 over O_N", "O_N", "O_N+O_N2"},
        {"O_N7 over O_N4..6", "O_N4+O_N5+O_N6", "O_N4..6+O_N7"},
        {"Q_N6 alone, over the shipped score", "", "Q_N6"},
        {"ranks 7, 8 as one cell over P1's 4, 5, 6", "Q P1 4,5,6", "Q P1 4,5,6,78"},
        {"ranks 7 and 8 apart over them as one cell", "Q P1 4,5,6,78", "Q P1 4,5,6,7,8"},
        {"O_N4W, rank 4 off d and e, over P1", "Q P1 4,5,6,78", "Q P1 +O_N4W"},
        {"Q_N2 over P1", "Q P1 4,5,6,78", "Q P1 +Q_N2"},
        {"P2 Q_NX over P1", "Q P1 4,5,6,78", "Q P2 +Q_NX"},
        {"P3 Q_NH over P2", "Q P2 +Q_NX", "Q P3 +Q_NH"},
        {"P4 O_N4W over P3", "Q P3 +Q_NH", "Q P4 +O_N4W"},
        {"P5 Q_B over P4", "Q P4 +O_N4W", "Q P5 +Q_B"},
        {"P6 Q_RCHS over P5", "Q P5 +Q_B", "Q P6 +Q_RCHS"},
        {"P7 S_PUB over P6", "Q P6 +Q_RCHS", "Q P7 +S_PUB"},
        {"P8 S_XO over P7", "Q P7 +S_PUB", "Q P8 +S_XO"},
        {"P9 S_XP over P8", "Q P8 +S_XO", "Q P9 +S_XP"},
        {"the tie (d), S_W1, over P7", "Q P7 +S_PUB", "Q P7 +S_W1, the tie (d)"},
        {"P8 and P9 apart over the tie", "Q P7 +S_W1, the tie (d)", "Q P9 +S_XP"},
        {"by signal: Q_RCHS over P1", "Q P1 4,5,6,78", "Q sig +Q_RCHS"},
        {"by signal: O_N4W over that", "Q sig +Q_RCHS", "Q sig +O_N4W"},
        {"by signal: Q_B over that", "Q sig +O_N4W", "Q sig +Q_B"},
        {"by signal: Q_NH over that", "Q sig +Q_B", "Q sig +Q_NH"},
        {"by signal: Q_NX over that", "Q sig +Q_NH", "Q P6 +Q_RCHS"},
        {"ranks 4 to 6, P1 to P5 of the first reading, over the page's P1 to P6", "Q P6 +Q_RCHS",
         "o P1..P5"},
    };

    // Each row's game block, numbered in file order.
    std::vector<int32_t> block(n);
    size_t blocks = 0;
    {
      int32_t open_game = -1;
      for (size_t r = 0; r < n; ++r) {
        if (st_game[r] != open_game) {
          open_game = st_game[r];
          blocks++;
        }
        block[r] = static_cast<int32_t>(blocks - 1);
      }
    }

    // half: -1 every row, 0 the even-numbered games, 1 the odd.
    const auto in_half = [&](size_t r, int half) { return half < 0 || (st_game[r] & 1) == half; };

    // One Gauss-Newton step from zero on the rows of `half`; false if singular.
    const auto gn_step = [&](const std::vector<int>& vars, int half, std::vector<double>* w) {
      const size_t k = vars.size();
      const size_t m = 2 * k;
      std::vector<double> g(m, 0.0), H(m * m, 0.0), x(m);
      for (size_t r = 0; r < n; ++r) {
        if (!in_half(r, half)) { continue; }
        const double p = st_p0[r];
        const double slope = p * (1.0 - p) * kk;
        const double resid = st_result[r] - p;
        const double wm = st_phase[r] / 24.0, we = 1.0 - wm;
        bool any = false;
        for (size_t i = 0; i < k; ++i) {
          const double c = st_diff[r * V_COUNT + vars[i]];
          x[2 * i] = c * wm;
          x[2 * i + 1] = c * we;
          any = any || c != 0.0;
        }
        if (!any) { continue; }
        for (size_t a = 0; a < m; ++a) {
          if (x[a] == 0.0) { continue; }
          g[a] += -2.0 * resid * slope * x[a];
          for (size_t b = 0; b < m; ++b) { H[a * m + b] += 2.0 * slope * slope * x[a] * x[b]; }
        }
      }
      // H w = -g, Gaussian elimination with partial pivoting.
      std::vector<double> A = H, bvec(m);
      w->assign(m, 0.0);
      for (size_t a = 0; a < m; ++a) { bvec[a] = -g[a]; }
      for (size_t col = 0; col < m; ++col) {
        size_t piv = col;
        for (size_t r2 = col + 1; r2 < m; ++r2) {
          if (std::abs(A[r2 * m + col]) > std::abs(A[piv * m + col])) { piv = r2; }
        }
        if (std::abs(A[piv * m + col]) < 1e-12) { return false; }
        if (piv != col) {
          for (size_t c2 = 0; c2 < m; ++c2) { std::swap(A[col * m + c2], A[piv * m + c2]); }
          std::swap(bvec[col], bvec[piv]);
        }
        for (size_t r2 = col + 1; r2 < m; ++r2) {
          const double f2 = A[r2 * m + col] / A[col * m + col];
          for (size_t c2 = col; c2 < m; ++c2) { A[r2 * m + c2] -= f2 * A[col * m + c2]; }
          bvec[r2] -= f2 * bvec[col];
        }
      }
      for (size_t a = m; a-- > 0;) {
        double s = bvec[a];
        for (size_t c2 = a + 1; c2 < m; ++c2) { s -= A[a * m + c2] * (*w)[c2]; }
        (*w)[a] = s / A[a * m + a];
      }
      return true;
    };

    // The summed dMSE over the rows of `half` at weights w (rounded if asked), and the
    // whole-game variance of that sum.
    struct dsum_t
    {
      double sum = 0.0, var = 0.0;
    };
    const auto dmse = [&](const std::vector<int>& vars, const std::vector<double>& w, int half,
                          bool rounded) {
      const size_t k = vars.size();
      double total = 0.0, g2 = 0.0, open_sum = 0.0;
      int32_t open_game = -1;
      for (size_t r = 0; r < n; ++r) {
        if (st_game[r] != open_game) {
          g2 += open_sum * open_sum;
          open_sum = 0.0;
          open_game = st_game[r];
        }
        if (!in_half(r, half)) { continue; }
        const double wm = st_phase[r] / 24.0, we = 1.0 - wm;
        double add = 0.0;
        for (size_t i = 0; i < k; ++i) {
          const double c = st_diff[r * V_COUNT + vars[i]];
          if (c == 0.0) { continue; }
          const double a = rounded ? std::lround(w[2 * i]) : w[2 * i];
          const double b = rounded ? std::lround(w[2 * i + 1]) : w[2 * i + 1];
          add += c * (a * wm + b * we);
        }
        if (add == 0.0) { continue; }
        const double p0 = st_p0[r];
        const double p1 = tuner_target::sigmoid(st_score[r] + add, opts.k);
        const double y = st_result[r];
        const double d = (y - p1) * (y - p1) - (y - p0) * (y - p0);
        total += d;
        open_sum += d;
      }
      g2 += open_sum * open_sum;
      // Whole-game variance of a sum: the games' sums squared (the mean of a game's sum is
      // small beside its spread here, and dropping it overstates the error).
      return dsum_t{total, g2};
    };

    // Across halves, per game block: each row read at the step its other half took.
    const auto cross_blocks = [&](const std::vector<int>& vars, const std::vector<double>& w_even,
                                  const std::vector<double>& w_odd) {
      std::vector<double> out(blocks, 0.0);
      const size_t k = vars.size();
      for (size_t r = 0; r < n; ++r) {
        const std::vector<double>& w = ((st_game[r] & 1) == 0) ? w_odd : w_even;
        const double wm = st_phase[r] / 24.0, we = 1.0 - wm;
        double add = 0.0;
        for (size_t i = 0; i < k; ++i) {
          const double c = st_diff[r * V_COUNT + vars[i]];
          if (c != 0.0) { add += c * (w[2 * i] * wm + w[2 * i + 1] * we); }
        }
        if (add == 0.0) { continue; }
        const double p0 = st_p0[r];
        const double p1 = tuner_target::sigmoid(st_score[r] + add, opts.k);
        const double y = st_result[r];
        out[block[r]] += (y - p1) * (y - p1) - (y - p0) * (y - p0);
      }
      return out;
    };
    std::vector<std::pair<std::string, std::vector<double>>> kept;  // per-block, for increments

    printf("%-26s %10s %8s %10s %10s %8s | %-28s | %s\n", "set", "dMSE", "+/-", "at lround",
           "across", "+/-", "g mg, g eg (first var)", "w mg, w eg per var");
    for (const reading_t& set : sets) {
      const size_t k = set.vars.size();
      // The gradient at zero for the first variant's two halves, whole-game errors.
      double gg[2] = {0, 0}, gg2[2] = {0, 0}, gopen[2] = {0, 0};
      uint64_t ngames = 0;
      int32_t open_game = -1;
      for (size_t r = 0; r < n; ++r) {
        if (st_game[r] != open_game) {
          if (open_game >= 0) {
            for (int h = 0; h < 2; ++h) {
              gg[h] += gopen[h];
              gg2[h] += gopen[h] * gopen[h];
              gopen[h] = 0;
            }
            ngames++;
          }
          open_game = st_game[r];
        }
        const double c = st_diff[r * V_COUNT + set.vars[0]];
        if (c == 0.0) { continue; }
        const double p = st_p0[r];
        const double slope = p * (1.0 - p) * kk;
        const double wm = st_phase[r] / 24.0;
        gopen[0] += -2.0 * (st_result[r] - p) * slope * c * wm;
        gopen[1] += -2.0 * (st_result[r] - p) * slope * c * (1.0 - wm);
      }
      for (int h = 0; h < 2; ++h) {
        gg[h] += gopen[h];
        gg2[h] += gopen[h] * gopen[h];
      }
      ngames++;
      const double gm = gg[0] / n, ge = gg[1] / n;
      const double gm_se = std::sqrt(std::max(0.0, gg2[0] - gg[0] * gg[0] / ngames)) / n;
      const double ge_se = std::sqrt(std::max(0.0, gg2[1] - gg[1] * gg[1] / ngames)) / n;

      std::vector<double> w, w_even, w_odd;
      const bool ok = gn_step(set.vars, -1, &w);
      const bool ok_e = gn_step(set.vars, 0, &w_even);
      const bool ok_o = gn_step(set.vars, 1, &w_odd);
      if (!ok || !ok_e || !ok_o) {
        printf("%-26s singular\n", set.name);
        continue;
      }
      const dsum_t in = dmse(set.vars, w, -1, false);
      const dsum_t rnd = dmse(set.vars, w, -1, true);
      std::vector<double> cb = cross_blocks(set.vars, w_even, w_odd);
      double across = 0.0, across2 = 0.0;
      for (double x : cb) {
        across += x;
        across2 += x * x;
      }
      printf("%-26s %10.2f %8.2f %10.2f %10.2f %8.2f | %6.3f+/-%.3f %6.3f+/-%.3f |", set.name,
             1e6 * in.sum / n, 1e6 * std::sqrt(in.var) / n, 1e6 * rnd.sum / n, 1e6 * across / n,
             1e6 * std::sqrt(across2) / n, 1e6 * gm, 1e6 * gm_se, 1e6 * ge, 1e6 * ge_se);
      for (size_t i = 0; i < k; ++i) { printf(" %s %.1f,%.1f", vname[set.vars[i]], w[2 * i], w[2 * i + 1]); }
      printf("\n");
      kept.push_back({set.name, std::move(cb)});
    }

    printf("\npaired increments across halves, 1e-6: the larger set's per-game dMSE less the "
           "smaller's, whole-game errors\n");
    const auto find = [&](const char* name) -> const std::vector<double>* {
      for (const auto& [nm, v] : kept) {
        if (nm == name) { return &v; }
      }
      return nullptr;
    };
    for (const increment_t& inc : increments) {
      const std::vector<double>* to = find(inc.to);
      const std::vector<double>* from = (inc.from[0] == '\0') ? nullptr : find(inc.from);
      if (to == nullptr || (inc.from[0] != '\0' && from == nullptr)) {
        printf("%-48s missing\n", inc.what);
        continue;
      }
      double sum = 0.0, sum2 = 0.0;
      for (size_t b = 0; b < blocks; ++b) {
        const double d = (*to)[b] - ((from != nullptr) ? (*from)[b] : 0.0);
        sum += d;
        sum2 += d * d;
      }
      printf("%-48s %10.2f +/- %.2f\n", inc.what, 1e6 * sum / n, 1e6 * std::sqrt(sum2) / n);
    }
  }

  printf("S102-CENSUS-OK\n");
  return 0;
}
