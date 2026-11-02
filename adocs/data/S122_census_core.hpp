#pragma once
// S122's research pass, 2026-11-02: the candidate king-danger counts for
// adocs/data/S122_census.cpp and adocs/data/S122_research_prototype.patch. Scratch
// of the research, kept because the step file's figures rest on it; no shipped form.
//
// Every definition is chesso's own and is said to be (DEC-221). CPW *King Safety*,
// read 2026-11-02, gives the zone, the attacker count, attack units and the words
// "safe queen contact check"; it never defines "safe", and the words "weak" and
// "defend" do not occur on the page. No engine's source was read, and no number
// here is a weight.
//
//   king K, attacker A, zone  K's square and the squares around it, stage two's zone[]
//   attackers[T]    A's pieces of type T with an attack in the zone, as the engine
//                   counts today; raw attacks, no x-ray, no pins
//   incidences      their attacked zone squares, summed piece by piece, as today
//   pawn_inc        zone squares A's pawns attack
//   checks[r][T]    squares a piece of type T of A attacks, from which that type
//                   attacks K's square, not holding a man of A's, under reading r:
//     0 reach       any such square
//     1 strict      nothing of K's attacks it, K's king included: so no contact check
//     2 chesso      no man of K's but the king attacks it, and if the king does,
//                   another class of A's attacks it too
//     3 no pawn     no pawn of K's attacks it
//   weak[0]         zone squares A's men attack that no man of K's but the king does
//   weak[1]         the same, K's queen not counted as a defender either
//   weak[2]         zone squares no man of K's defends, attacked or not
//   weak_man        weak[0] squares holding a man of K's: where S101's family reads
//                   the same man, DEC-333 (9)'s overlap
//   check_capture   reading 2's check squares holding a man of K's
//   checks_e[T]     reading 2 on empty target squares only, DEC-344's P1
//   def[0], def[1]  K's knights, K's bishops with an attack in K's own zone (DEC-258)
//   def_sq          zone squares K's minors attack
//   storm[d][b]     per file of K's three, the closest enemy pawn in front of K at
//                   rank distance d+1 (1 to 4); b when K's pawn stands directly
//                   before it, CPW's "blockage". d1 blocked means a pawn beside K on
//                   K's own rank, so the enemy pawn gives check: never on a corpus row
//                   and reachable in a search (4k3/8/8/8/8/7p/6KP/8 w)
#include <cstdint>
#include "S101_threat_core.hpp"

namespace s122
{
using s101::B;
using s101::K;
using s101::N;
using s101::P;
using s101::Q;
using s101::R;

inline constexpr int READINGS = 4;

struct ks_counts_t
{
  int attackers[4] = {0, 0, 0, 0};  // N B R Q
  int incidences = 0;
  int pawn_inc = 0;
  int attackers_plus[4] = {0, 0, 0, 0};  // over the zone plus its forward rank
  int incidences_plus = 0;
  int checks[READINGS][4] = {};
  int contact_q = 0;  // reading 2's queen checks that stand beside K
  int contact_r = 0;
  int weak[3] = {0, 0, 0};
  int weak_man = 0;
  int check_capture = 0;
  int checks_e[4] = {0, 0, 0, 0};
  int def[2] = {0, 0};
  int def_sq = 0;
  int storm[4][2] = {};

  int attacker_total() const
  { return attackers[0] + attackers[1] + attackers[2] + attackers[3]; }

  int checks_total(int reading) const
  {
    return checks[reading][0] + checks[reading][1] + checks[reading][2] +
           checks[reading][3];
  }
};

// The zone and the rank in front of it, K's forward: White advances by decreasing
// the index, a8 being 0.
inline bb_t zone_plus(const bb_tables_t* t, bb_t zone, int colour)
{
  (void)t;
  return zone | ((colour == WHITE) ? zone >> 8 : zone << 8);
}

template <typename Board>
inline void ks_counts(const bb_tables_t* t,
                      const Board* b,
                      const s101::sets_t& s,
                      int king_colour,
                      ks_counts_t* c)
{
  const int A = 1 - king_colour;
  const int D = king_colour;
  const bb_t occ = b->occupancies[BOTH];
  const index_t ksq = get_lsb_index(s.pieces[D][K]);
  const bb_t ring = t->king_attacks[ksq];
  const bb_t zone = ring | (BB_1 << ksq);
  const bb_t wide = zone_plus(t, zone, D);

  *c = ks_counts_t{};

  for (int type = 0; type < 4; ++type) {
    for (bb_t x = s.pieces[A][N + type]; x; x &= x - 1) {
      const index_t sq = get_lsb_index(x);
      const bb_t att = type == 0   ? t->knight_attacks[sq]
                       : type == 1 ? get_bishop_attacks(t, sq, occ)
                       : type == 2 ? get_rook_attacks(t, sq, occ)
                                   : get_queen_attacks(t, sq, occ);
      if (att & zone) {
        c->attackers[type]++;
        c->incidences += count_bits(att & zone);
      }
      if (att & wide) {
        c->attackers_plus[type]++;
        c->incidences_plus += count_bits(att & wide);
      }
    }
  }

  c->pawn_inc = count_bits(s.by[A][P] & zone);

  const bb_t from_b = get_bishop_attacks(t, ksq, occ);
  const bb_t from_r = get_rook_attacks(t, ksq, occ);
  const bb_t from[4] = {t->knight_attacks[ksq], from_b, from_r, from_b | from_r};
  const bb_t own = b->occupancies[A];

  for (int type = 0; type < 4; ++type) {
    const bb_t reach = s.by[A][N + type] & from[type] & ~own;
    // Another class of A's, not the squares this class misses: a queen's own
    // union holds every square she reaches.
    bb_t others = s.by[A][P] | s.by[A][K];
    for (int other = 0; other < 4; ++other) {
      if (other != type) { others |= s.by[A][N + other]; }
    }

    c->checks[0][type] = count_bits(reach);
    c->checks[1][type] = count_bits(reach & ~s.all[D]);
    const bb_t chesso = reach & ~s.nonking[D] & (~ring | others);
    c->checks[2][type] = count_bits(chesso);
    c->check_capture += count_bits(chesso & b->occupancies[D]);
    c->checks_e[type] = count_bits(chesso & ~occ);
    c->checks[3][type] = count_bits(reach & ~s.by[D][P]);
    if (type == 3) { c->contact_q = count_bits(chesso & ring); }
    if (type == 2) { c->contact_r = count_bits(chesso & ring); }
  }

  c->weak[0] = count_bits(zone & s.nonking[A] & ~s.nonking[D]);
  c->weak_man = count_bits(zone & s.nonking[A] & ~s.nonking[D] & b->occupancies[D]);
  c->weak[1] = count_bits(zone & s.nonking[A] &
                          ~(s.by[D][P] | s.by[D][N] | s.by[D][B] | s.by[D][R]));
  c->weak[2] = count_bits(zone & ~s.nonking[D]);

  for (int type = 0; type < 2; ++type) {
    for (bb_t x = s.pieces[D][N + type]; x; x &= x - 1) {
      const index_t sq = get_lsb_index(x);
      const bb_t att = type == 0 ? t->knight_attacks[sq]
                                 : get_bishop_attacks(t, sq, occ);
      if (att & zone) { c->def[type]++; }
    }
  }
  c->def_sq = count_bits((s.by[D][N] | s.by[D][B]) & zone);

  // The storm: index rank 0 is the eighth, and K's forward is the side the enemy
  // pawns come from.
  const int king_file = ksq & 7;
  const int king_rank = ksq >> 3;
  const int step = (D == WHITE) ? -1 : 1;

  for (int file = king_file - 1; file <= king_file + 1; ++file) {
    if (file < 0 || file > 7) { continue; }
    for (int d = 1; d <= 4; ++d) {
      const int rank = king_rank + step * d;
      if (rank < 0 || rank > 7) { break; }
      const int sq = rank * 8 + file;
      if ((s.pieces[A][P] >> sq) & 1) {
        // Directly before it, on K's side, is one rank back toward K.
        const int before = (rank - step) * 8 + file;
        const bool blocked = (s.pieces[D][P] >> before) & 1;
        c->storm[d - 1][blocked ? 1 : 0]++;
        break;
      }
    }
  }
}

// The flat probe of reach: every attack count weighs one unit. No proposal.
inline int ks_units(const ks_counts_t& c)
{ return c.attacker_total() + c.incidences + c.checks_total(2) + c.weak[0]; }
}  // namespace s122
