#pragma once
// S101's research pass, 2026-11-01: the candidate threat sets, from their descriptions
// (CPW Hanging Piece, Loose Piece, Pawn Pushes (Bitboards)), for adocs/data/S101_census.cpp
// and S101_census_zone.cpp. Scratch of the research, kept because the figures in
// S101's step file rest on it; the engine's own sets are src/evaluation.cpp threat_terms().
#include <cstdint>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"

namespace s101
{
enum
{
  P,
  N,
  B,
  R,
  Q,
  K
};

inline bb_t beside(bb_t squares)
{ return ((squares << 1) & ~file_masks[0]) | ((squares >> 1) & ~file_masks[7]); }

struct sets_t
{
  bb_t by[2][6];
  bb_t nonking[2];
  bb_t all[2];
  bb_t pieces[2][6];
};

inline void build_sets(const bb_tables_t* t, const board_t* b, sets_t* s)
{
  const bb_t occ = b->occupancies[BOTH];
  for (int c = 0; c < 2; ++c) {
    const int base = c == WHITE ? W_PAWN : B_PAWN;
    for (int i = 0; i < 6; ++i) { s->pieces[c][i] = b->bitboards[base + i]; }
    s->by[c][P] = c == WHITE ? beside(s->pieces[c][P]) >> 8
                             : beside(s->pieces[c][P]) << 8;
    bb_t kn = 0, bi = 0, ro = 0, qu = 0, x;
    for (x = s->pieces[c][N]; x; x &= x - 1) {
      kn |= t->knight_attacks[get_lsb_index(x)];
    }
    for (x = s->pieces[c][B]; x; x &= x - 1) {
      bi |= get_bishop_attacks(t, get_lsb_index(x), occ);
    }
    for (x = s->pieces[c][R]; x; x &= x - 1) {
      ro |= get_rook_attacks(t, get_lsb_index(x), occ);
    }
    for (x = s->pieces[c][Q]; x; x &= x - 1) {
      qu |= get_queen_attacks(t, get_lsb_index(x), occ);
    }
    s->by[c][N] = kn;
    s->by[c][B] = bi;
    s->by[c][R] = ro;
    s->by[c][Q] = qu;
    s->by[c][K] = t->king_attacks[get_lsb_index(s->pieces[c][K])];
    s->nonking[c] = s->by[c][P] | kn | bi | ro | qu;
    s->all[c] = s->nonking[c] | s->by[c][K];
  }
}

// The eleven attacker-victim pairs: nine strictly lesser by class, and the two
// minor-on-minor.
inline constexpr int PAIRS = 11;
inline constexpr int PAIR_A[PAIRS] = {P, P, P, P, N, N, B, B, R, N, B};
inline constexpr int PAIR_V[PAIRS] = {N, B, R, Q, R, Q, R, Q, Q, B, N};
inline constexpr const char* PAIR_NAME[PAIRS] = {
    "P>N", "P>B", "P>R", "P>Q", "N>R", "N>Q", "B>R", "B>Q", "R>Q", "N>B", "B>N"};

// Pawn push targets of side c. a8 is index 0, so White pushes by >> 8.
inline bb_t single_push(int c, bb_t pawns, bb_t empty)
{ return (c == WHITE ? pawns >> 8 : pawns << 8) & empty; }

inline bb_t double_push(int c, bb_t single, bb_t empty)
{
  constexpr bb_t RANK4 = 0x000000FF00000000ULL;  // indices 32..39
  constexpr bb_t RANK5 = 0x00000000FF000000ULL;  // indices 24..31
  return c == WHITE ? (single >> 8) & empty & RANK4
                    : (single << 8) & empty & RANK5;
}

inline bb_t pawn_attacks_from(int c, bb_t squares)
{ return c == WHITE ? beside(squares) >> 8 : beside(squares) << 8; }

// The three readings of "safe".
//   0  no enemy pawn attacks the target
//   1  nothing of the enemy's attacks it, king included (DEC-329 (2)'s word)
//   2  no enemy pawn attacks it, and it is unattacked or defended by own
inline bb_t safe_target(const sets_t& s, int c, int reading)
{
  const int e = 1 - c;
  switch (reading) {
    case 0:
      return ~s.by[e][P];
    case 1:
      return ~s.all[e];
    default:
      return ~s.by[e][P] & (~s.all[e] | s.all[c]);
  }
}
}  // namespace s101
