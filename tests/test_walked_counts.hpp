#pragma once
// The evaluation's counts walked by hand from a FEN's placement -- mobility,
// king safety, passed pawns, pawn structure, piece placement, and since S123
// the passed pawn family -- a second implementation the engine's own counts
// are checked against. S276.
//
// These walks are the feature half of tools/eval_model.hpp, the hand-written
// mirror of evaluate() the tuner fitted from S028 to S262 and
// tools/feature_audit and tools/truncation_scan read until S276. S276 deleted
// the mirror -- its sum, its phase, its piece list and its loader -- because
// both tools read the engine's own trace now (tools/tuner_trace.hpp), and
// moved these here unchanged but for the namespace and the comments that named
// the deleted parts. They stay because they are the one implementation of each
// count that shares no code with the engine: the trace records the engine's
// counts and the engine's accessors are the same code collecting, so without
// these tests/test_eval_model.cpp could hold the engine only to itself and to
// hand-worked placements, and tests/test_eval_trace.cpp would have nothing to
// hold mobility to. They are not a model of evaluate(): nothing here weighs,
// tapers or sums a score.
//
// The definitions they are written from are src/evaluation.hpp's, stated
// there so that the two implementations mean the same thing. Squares here are
// true board squares, index 0 = a8 to index 63 = h1, the order a FEN is
// written in.
#include <cstddef>
#include <string>
#include "evaluation.hpp"
#include "tuner_layout.hpp"

namespace walked_counts
{

// The widths, from tools/tuner_layout.hpp, which takes each from the engine's
// own weight array or enum.
using tuner_layout::KING_SAFETY_COUNT;
using tuner_layout::PASSED_PAWN_COUNT;
using tuner_layout::PAWN_STRUCTURE_COUNT;
using tuner_layout::PIECE_PLACEMENT_COUNT;

// What the three counts are, in the order the weights are indexed. Named here
// because the engine names them only in that prose, and an index nobody can
// read is an index a caller gets wrong.
enum pawn_structure_feature_t
{
  PS_ISOLATED,
  PS_DOUBLED,
  PS_BACKWARD
};

// What the seven counts are, in the order the weights are indexed. Named here
// because the engine names them only in the prose over those arrays, and an
// index nobody can read is an index a caller gets wrong. The last four are
// S135's.
enum piece_placement_feature_t
{
  PL_BISHOP_PAIR,
  PL_ROOK_OPEN,
  PL_ROOK_HALF_OPEN,
  PL_BAD_BISHOP,
  PL_BAD_BISHOP_RAMMED,
  PL_TRAPPED_MINOR,
  PL_TRAPPED_ROOK
};

static_assert(static_cast<size_t>(PL_TRAPPED_ROOK) + 1 == PIECE_PLACEMENT_COUNT,
              "the engine's piece placement arrays changed width: name the "
              "features here again");


inline int piece_from_char(char c, bool* black)
{
  *black = (c >= 'a' && c <= 'z');

  switch (c) {
    case 'P':
    case 'p':
      return 0;
    case 'N':
    case 'n':
      return 1;
    case 'B':
    case 'b':
      return 2;
    case 'R':
    case 'r':
      return 3;
    case 'Q':
    case 'q':
      return 4;
    case 'K':
    case 'k':
      return 5;
    default:
      return -1;
  }
}


// Mobility, as S121's table reads it: per side, WHITE first, how many of the
// side's pieces have each count, the cell MOBILITY_OFFSET[type] + count for a
// piece of type knight, bishop, rook or queen (src/evaluation.hpp). A piece's
// count is the squares it attacks that do not hold one of its own side's
// pieces, less the squares an enemy pawn attacks, and a pinned piece keeps
// only its pin line. Walked ray by ray from the placement
// rather than read out of the engine's magic tables, deliberately: this file
// exists to disagree with the engine when the two stop computing the same
// thing, and a shared implementation cannot do that. The width is the engine's,
// and a count past its row is refused rather than written past it.
//
// S121's two shipped parts, walked from the descriptions src/evaluation.hpp
// states, each by its own rule here and none read from the engine's
// bitboards: P1 drops a square an enemy pawn attacks; P3 keeps, for a piece
// pinned to its own king, only the squares on the line through the king and
// the piece, the pin found by walking out from the king.
inline bool mobility_cells(const std::string& placement,
                           int out[2][MOBILITY_CELLS])
{
  int kind_at[64];
  bool white_at[64];

  for (int i = 0; i < 64; ++i) {
    kind_at[i] = -1;
  }

  int square = 0;
  for (const char c : placement) {
    if (c == '/') { continue; }

    if (c >= '1' && c <= '8') {
      square += c - '0';
      continue;
    }

    bool black = false;
    const int type = piece_from_char(c, &black);

    if (type < 0 || square > 63) { return false; }

    kind_at[square] = type;
    white_at[square] = !black;
    square++;
  }

  if (square != 64) { return false; }

  static const int knight_steps[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                         {1, -2},  {1, 2},  {2, -1},  {2, 1}};
  static const int bishop_rays[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  static const int rook_rays[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

  const auto inside = [](int r, int f) {
    return r >= 0 && r <= 7 && f >= 0 && f <= 7;
  };
  const auto holds = [&](int r, int f, int type, bool white) {
    return inside(r, f) && kind_at[r * 8 + f] == type &&
           white_at[r * 8 + f] == white;
  };

  // P3: each piece pinned to its own king, and the direction from that king
  // to it. Row 0 is rank 8, so White's pawns advance towards row 0.
  int pin_dr[64] = {};
  int pin_df[64] = {};
  bool is_pinned[64] = {};
  int king_r[2] = {-1, -1};
  int king_f[2] = {-1, -1};

  for (int s = 0; s < 64; ++s) {
    if (kind_at[s] == 5) {
      king_r[white_at[s] ? 0 : 1] = s / 8;
      king_f[white_at[s] ? 0 : 1] = s % 8;
    }
  }

  for (int side = 0; side < 2; ++side) {
    const bool white = (side == 0);
    if (king_r[side] < 0) { return false; }

    for (int d = 0; d < 8; ++d) {
      const int* step = (d < 4) ? rook_rays[d] : bishop_rays[d - 4];
      const int slider = (d < 4) ? 3 : 2;  // rook or bishop, and the queen
      int r = king_r[side] + step[0];
      int f = king_f[side] + step[1];
      int first = -1;

      while (inside(r, f)) {
        const int at = r * 8 + f;
        if (kind_at[at] >= 0) {
          if (first < 0) {
            if (white_at[at] != white) { break; }
            first = at;
          } else {
            if (white_at[at] != white &&
                (kind_at[at] == slider || kind_at[at] == 4)) {
              is_pinned[first] = true;
              pin_dr[first] = step[0];
              pin_df[first] = step[1];
            }
            break;
          }
        }
        r += step[0];
        f += step[1];
      }
    }
  }

  // Whether the piece on `from`, of side `white`, counts the square `to` it
  // attacks.
  const auto counted = [&](int from, int to, bool white) {
    const int r = to / 8;
    const int f = to % 8;
    const int side = white ? 0 : 1;

    if (is_pinned[from]) {
      // On the pin line when (to - king) is parallel to the pin direction.
      const int dr = r - king_r[side];
      const int df = f - king_f[side];
      if (dr * pin_df[from] - df * pin_dr[from] != 0) { return false; }
    }

    // An enemy pawn attacks `to` from the row behind it as the enemy
    // advances: a Black pawn from row r - 1, a White one from row r + 1.
    const int pr = white ? r - 1 : r + 1;
    if (holds(pr, f - 1, 0, !white) || holds(pr, f + 1, 0, !white)) {
      return false;
    }

    return !(kind_at[to] >= 0 && white_at[to] == white);
  };

  for (int side = 0; side < 2; ++side) {
    for (int i = 0; i < MOBILITY_CELLS; ++i) {
      out[side][i] = 0;
    }
  }

  for (int from = 0; from < 64; ++from) {
    const int type = kind_at[from];

    // Types 1..4 are knight, bishop, rook, queen. Pawns and kings contribute
    // nothing, exactly as evaluate_mobility() skips them.
    if (type < 1 || type > 4) { continue; }

    const bool white = white_at[from];
    const int rank = from / 8;
    const int file = from % 8;
    int targets = 0;

    if (type == 1) {
      for (const auto& step : knight_steps) {
        const int r = rank + step[0];
        const int f = file + step[1];

        if (r < 0 || r > 7 || f < 0 || f > 7) { continue; }

        const int to = r * 8 + f;
        if (counted(from, to, white)) { targets++; }
      }
    } else {
      const int (*rays)[2] = (type == 3) ? rook_rays : bishop_rays;
      const int ray_count = (type == 4) ? 8 : 4;

      for (int i = 0; i < ray_count; ++i) {
        // The queen walks both sets, so the second four come from the rook
        // table once the bishop's are exhausted.
        const int* step = (type == 4 && i >= 4) ? rook_rays[i - 4] : rays[i];

        int r = rank + step[0];
        int f = file + step[1];

        while (r >= 0 && r <= 7 && f >= 0 && f <= 7) {
          const int to = r * 8 + f;

          // The ray stops on the first piece, whoever's, and attacks it.
          if (counted(from, to, white)) { targets++; }
          if (kind_at[to] >= 0) { break; }

          r += step[0];
          f += step[1];
        }
      }
    }

    if (targets >= MOBILITY_WIDTH[type - 1]) { return false; }

    out[white ? 0 : 1][MOBILITY_OFFSET[type - 1] + targets]++;
  }

  return true;
}


// King safety counts per side, WHITE first, in king_safety_feature_t order.
// Each side's row is about its own king with the other side attacking.
//
// Walked by hand from the placement for the same reason mobility_features() is:
// this file has to be able to disagree with the engine, and code shared with it
// cannot.
//
// Per side rather than as a White-minus-Black difference, because a difference
// is not what a test can check: White gaining a count and Black gaining the
// same one cancels in it, and the engine's own counts are per side, so this is
// the form the two implementations are compared in -- tests/test_eval_model,
// "the walk counts what the engine counts".
//
// "Ahead" is toward the enemy, so a white king's shield sits on lower indices
// and a black king's on higher ones.
inline bool king_safety_features_by_colour(const std::string& placement,
                                           int out[2][KING_SAFETY_COUNT])
{
  int kind_at[64];
  bool white_at[64];

  for (int i = 0; i < 64; ++i) {
    kind_at[i] = -1;
  }

  int square = 0;
  for (const char c : placement) {
    if (c == '/') { continue; }

    if (c >= '1' && c <= '8') {
      square += c - '0';
      continue;
    }

    bool black = false;
    const int type = piece_from_char(c, &black);

    if (type < 0 || square > 63) { return false; }

    kind_at[square] = type;
    white_at[square] = !black;
    square++;
  }

  if (square != 64) { return false; }

  static const int knight_steps[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                         {1, -2},  {1, 2},  {2, -1},  {2, 1}};
  static const int bishop_rays[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  static const int rook_rays[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

  for (int side = 0; side < 2; ++side) {
    for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
      out[side][i] = 0;
    }
  }

  for (int side = 0; side < 2; ++side) {
    const bool white = (side == 0);
    int king = -1;

    for (int sq = 0; sq < 64 && king < 0; ++sq) {
      if (kind_at[sq] == 5 && white_at[sq] == white) { king = sq; }
    }

    // The tuning corpus is real self-play and always has both kings, but
    // nothing on the way in enforces that, and a missing king would otherwise
    // index a zone that was never built.
    if (king < 0) { continue; }

    const int king_rank = king / 8;
    const int king_file = king % 8;

    bool zone[64] = {false};

    for (int dr = -1; dr <= 1; ++dr) {
      for (int df = -1; df <= 1; ++df) {
        const int r = king_rank + dr;
        const int f = king_file + df;

        if (r < 0 || r > 7 || f < 0 || f > 7) { continue; }

        zone[r * 8 + f] = true;
      }
    }

    int feature[KING_SAFETY_COUNT] = {0};

    for (int from = 0; from < 64; ++from) {
      const int type = kind_at[from];

      // Types 1..4 are knight, bishop, rook, queen, and only the enemy's count
      // as attackers. Pawns and the kings themselves are not part of the term.
      if (type < 1 || type > 4) { continue; }
      if (white_at[from] == white) { continue; }

      const int rank = from / 8;
      const int file = from % 8;
      int hits = 0;

      if (type == 1) {
        for (const auto& step : knight_steps) {
          const int r = rank + step[0];
          const int f = file + step[1];

          if (r < 0 || r > 7 || f < 0 || f > 7) { continue; }

          if (zone[r * 8 + f]) { hits++; }
        }
      } else {
        const int (*rays)[2] = (type == 3) ? rook_rays : bishop_rays;
        const int ray_count = (type == 4) ? 8 : 4;

        for (int i = 0; i < ray_count; ++i) {
          // The queen walks both sets, so the second four come from the rook
          // table once the bishop's are exhausted.
          const int* step = (type == 4 && i >= 4) ? rook_rays[i - 4] : rays[i];

          int r = rank + step[0];
          int f = file + step[1];

          while (r >= 0 && r <= 7 && f >= 0 && f <= 7) {
            const int to = r * 8 + f;

            if (zone[to]) { hits++; }

            // The blocker is attacked whatever colour it is, and the defending
            // king is a blocker like any other: a rook bearing down on it still
            // counts the square it stands on.
            if (kind_at[to] >= 0) { break; }

            r += step[0];
            f += step[1];
          }
        }
      }

      if (hits > 0) {
        feature[type - 1]++;
        feature[KS_ZONE_ATTACKS] += hits;
      }
    }

    const int ahead = white ? -1 : 1;

    for (int df = -1; df <= 1; ++df) {
      const int f = king_file + df;

      if (f < 0 || f > 7) { continue; }

      for (int distance = 1; distance <= 2; ++distance) {
        const int r = king_rank + ahead * distance;

        if (r < 0 || r > 7) { continue; }

        const int sq = r * 8 + f;

        if (kind_at[sq] == 0 && white_at[sq] == white) {
          feature[(distance == 1) ? KS_SHIELD_NEAR : KS_SHIELD_FAR]++;
        }
      }

      bool own_pawn = false;
      bool enemy_pawn = false;

      for (int r = 0; r < 8; ++r) {
        if (kind_at[r * 8 + f] != 0) { continue; }

        if (white_at[r * 8 + f] == white) {
          own_pawn = true;
        } else {
          enemy_pawn = true;
        }
      }

      if (!own_pawn && !enemy_pawn) { feature[KS_OPEN_FILE]++; }
      if (!own_pawn && enemy_pawn) { feature[KS_HALF_OPEN_FILE]++; }
    }

    for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
      out[side][i] = feature[i];
    }
  }

  return true;
}


// Passed pawn counts, White minus Black, one per advancement bucket. A pawn is
// passed when no enemy pawn stands strictly ahead of it on its own file or on
// either neighbouring file; "ahead" is toward the enemy, so for White a lower
// index and for Black a higher one, and an enemy pawn level with the pawn or
// behind it does not block. Own pawns never block, so both halves of a doubled
// pair can be passed at once. A pawn on its own seventh rank is counted in no
// bucket: the piece-square table prices it alone since S134.
//
// Walked by hand from the placement for the same reason mobility_features() is:
// this file has to be able to disagree with the engine, and code shared with it
// cannot.
//
// In this indexing a white pawn's home rank is 6 and a black pawn's is 1, which
// is why the two colours count their buckets in opposite directions.
inline bool passed_pawn_features(const std::string& placement,
                                 int out[PASSED_PAWN_COUNT])
{
  int kind_at[64];
  bool white_at[64];

  for (int i = 0; i < 64; ++i) {
    kind_at[i] = -1;
  }

  int square = 0;
  for (const char c : placement) {
    if (c == '/') { continue; }

    if (c >= '1' && c <= '8') {
      square += c - '0';
      continue;
    }

    bool black = false;
    const int type = piece_from_char(c, &black);

    if (type < 0 || square > 63) { return false; }

    kind_at[square] = type;
    white_at[square] = !black;
    square++;
  }

  if (square != 64) { return false; }

  for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
    out[i] = 0;
  }

  for (int from = 0; from < 64; ++from) {
    if (kind_at[from] != 0) { continue; }

    const bool white = white_at[from];
    const int rank = from / 8;
    const int file = from % 8;

    // A pawn on a back rank is not a position the engine can reach, and it
    // would index outside the buckets. Refusing the placement is louder
    // than clamping it and the caller already reports a refusal.
    if (rank == 0 || rank == 7) { return false; }

    const int ahead = white ? -1 : 1;
    bool blocked = false;

    for (int df = -1; df <= 1 && !blocked; ++df) {
      const int f = file + df;

      if (f < 0 || f > 7) { continue; }

      for (int r = rank + ahead; r >= 0 && r <= 7; r += ahead) {
        const int sq = r * 8 + f;

        if (kind_at[sq] == 0 && white_at[sq] != white) {
          blocked = true;
          break;
        }
      }
    }

    if (blocked) { continue; }

    // Its own seventh: row 1 for White, row 6 for Black. A passer by
    // definition, folded into the table at S134, so it counts nowhere here --
    // and left in, it would index a sixth bucket that no longer exists.
    if (rank == (white ? 1 : 6)) { continue; }

    const int bucket = white ? (6 - rank) : (rank - 1);

    out[bucket] += white ? 1 : -1;
  }

  return true;
}


// Pawn structure counts, White minus Black, in isolated, doubled, backward
// order. Each is about a side's own pawns, and the three are not exclusive: one
// pawn can be counted in all of them.
//
// The definitions are src/evaluation.hpp's, stated there because this file is
// written from that comment and the two have to mean the same thing:
//
//   isolated  no own pawn anywhere on either neighbouring file, rank ignored.
//   doubled   another own pawn ahead on the same file, so a file with n own
//             pawns contributes n - 1.
//   backward  no own pawn on either neighbouring file at or behind its own
//             rank, the pawn's own rank counting as behind, and the square
//             directly ahead attacked by an enemy pawn. Whether that square is
//             occupied is not asked.
//
// Walked by hand from the placement for the same reason mobility_features() is:
// this file has to be able to disagree with the engine, and code shared with it
// cannot.
//
// "Ahead" is toward the enemy, so a lower index for White and a higher one for
// Black, and "behind" is the opposite -- which is why the two colours compare
// ranks in opposite directions.
inline bool pawn_structure_features(const std::string& placement,
                                    int out[PAWN_STRUCTURE_COUNT])
{
  int kind_at[64];
  bool white_at[64];

  for (int i = 0; i < 64; ++i) {
    kind_at[i] = -1;
  }

  int square = 0;
  for (const char c : placement) {
    if (c == '/') { continue; }

    if (c >= '1' && c <= '8') {
      square += c - '0';
      continue;
    }

    bool black = false;
    const int type = piece_from_char(c, &black);

    if (type < 0 || square > 63) { return false; }

    kind_at[square] = type;
    white_at[square] = !black;
    square++;
  }

  if (square != 64) { return false; }

  for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
    out[i] = 0;
  }

  for (int from = 0; from < 64; ++from) {
    if (kind_at[from] != 0) { continue; }

    const bool white = white_at[from];
    const int rank = from / 8;
    const int file = from % 8;
    const int ahead = white ? -1 : 1;
    const int sign = white ? 1 : -1;

    // The two neighbouring files answer both the isolated question and the
    // first half of the backward one, so they are walked once.
    bool neighbour_anywhere = false;
    bool neighbour_at_or_behind = false;

    for (int df = -1; df <= 1; df += 2) {
      const int f = file + df;

      if (f < 0 || f > 7) { continue; }

      for (int r = 0; r < 8; ++r) {
        const int sq = r * 8 + f;

        if (kind_at[sq] != 0 || white_at[sq] != white) { continue; }

        neighbour_anywhere = true;

        if (white ? (r >= rank) : (r <= rank)) {
          neighbour_at_or_behind = true;
        }
      }
    }

    if (!neighbour_anywhere) { out[PS_ISOLATED] += sign; }

    for (int r = rank + ahead; r >= 0 && r <= 7; r += ahead) {
      const int sq = r * 8 + file;

      if (kind_at[sq] == 0 && white_at[sq] == white) {
        out[PS_DOUBLED] += sign;
        break;
      }
    }

    if (neighbour_at_or_behind) { continue; }

    // An enemy pawn attacking the stop square stands one square further ahead
    // again, on a neighbouring file. Both bounds hold for any pawn a legal
    // position can have; they are here so a pawn on a back rank walks off the
    // board the way the engine's shifts do rather than reading past the array.
    const int stop = rank + ahead;
    const int attacker_rank = stop + ahead;

    if (stop < 0 || stop > 7 || attacker_rank < 0 || attacker_rank > 7) {
      continue;
    }

    for (int df = -1; df <= 1; df += 2) {
      const int f = file + df;

      if (f < 0 || f > 7) { continue; }

      const int sq = attacker_rank * 8 + f;

      if (kind_at[sq] == 0 && white_at[sq] != white) {
        out[PS_BACKWARD] += sign;
        break;
      }
    }
  }

  return true;
}


// Piece placement counts, White minus Black, in bishop pair, rook on an open
// file, rook on a half-open file, bad bishop, bad bishop rammed, trapped minor,
// trapped rook order. Each is about a side's own pieces.
//
// The definitions are src/evaluation.hpp's, stated there because this file is
// written from that comment and the two have to mean the same thing:
//
//   bishop_pair     1 when the side has two or more bishops, 0 otherwise. Not a
//                   count: three bishops off a promotion still score 1, and
//                   square colour is not asked about.
//   rook_open       own rooks standing on a file carrying no pawn of either
//                   colour. The rook's rank does not enter it, and a piece is
//                   not a pawn and does not close a file.
//   rook_half_open  own rooks standing on a file carrying no own pawn and at
//                   least one enemy pawn.
//
// The two rook features exclude each other. A file with an own pawn on it is
// neither open nor half-open, whatever the enemy has there. Rook on the
// seventh was a fourth until S134 folded it into the rook table (DEC-090).
//
// S135's four, walked here square by square from evaluation.hpp's statement
// of them, which has them in full: own pawns on each own bishop's colour,
// summed over the bishops; the same over the rammed own pawns, those with an
// enemy pawn directly ahead; own knights and bishops on one of the listed
// trap patterns, each once; own rooks in the corner beside their own king.
//
// Walked by hand from the placement for the same reason mobility_features() is:
// this file has to be able to disagree with the engine, and code shared with it
// cannot.
inline bool piece_placement_features(const std::string& placement,
                                     int out[PIECE_PLACEMENT_COUNT])
{
  int kind_at[64];
  bool white_at[64];

  for (int i = 0; i < 64; ++i) {
    kind_at[i] = -1;
  }

  int square = 0;
  for (const char c : placement) {
    if (c == '/') { continue; }

    if (c >= '1' && c <= '8') {
      square += c - '0';
      continue;
    }

    bool black = false;
    const int type = piece_from_char(c, &black);

    if (type < 0 || square > 63) { return false; }

    kind_at[square] = type;
    white_at[square] = !black;
    square++;
  }

  if (square != 64) { return false; }

  for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
    out[i] = 0;
  }

  // Row 0 is White's, row 1 is Black's. Both rook file questions are about the
  // pawns on a file and nothing else on it, so the pawns are collected once.
  bool pawn_on[2][8] = {};

  for (int sq = 0; sq < 64; ++sq) {
    if (kind_at[sq] != 0) { continue; }

    pawn_on[white_at[sq] ? 0 : 1][sq % 8] = true;
  }

  int bishops[2] = {0, 0};

  for (int sq = 0; sq < 64; ++sq) {
    const int type = kind_at[sq];

    if (type < 0) { continue; }

    const int side = white_at[sq] ? 0 : 1;
    const int sign = white_at[sq] ? 1 : -1;

    if (type == 2) {
      bishops[side]++;
      continue;
    }

    if (type != 3) { continue; }

    const int file = sq % 8;
    const bool own = pawn_on[side][file];
    const bool enemy = pawn_on[1 - side][file];

    if (!own && !enemy) { out[PL_ROOK_OPEN] += sign; }
    if (!own && enemy) { out[PL_ROOK_HALF_OPEN] += sign; }
  }

  // The one feature that is not a count. Two bishops are a pair and so are
  // three, which is why this is a comparison and not an accumulation.
  out[PL_BISHOP_PAIR] =
      ((bishops[0] >= 2) ? 1 : 0) - ((bishops[1] >= 2) ? 1 : 0);

  // S135. A square's colour by its row and file, both counted from a8; only
  // whether two squares share one is asked.
  const auto colour_of = [](int sq) { return (sq / 8 + sq % 8) % 2; };

  // A square named the way evaluation.hpp names White's patterns, "h7", for
  // the side `side` -- for Black the rank is mirrored, rank r to 9 - r.
  const auto named = [](const char* name, int side) {
    const int file = name[0] - 'a';
    const int rank = (side == 0) ? (name[1] - '0') : (9 - (name[1] - '0'));

    return (8 - rank) * 8 + file;
  };

  const auto is = [&](int sq, int type, int side) {
    return kind_at[sq] == type && white_at[sq] == (side == 0);
  };

  for (int side = 0; side < 2; ++side) {
    const int sign = (side == 0) ? 1 : -1;

    // Bad bishop and its rammed share: every own bishop against every own pawn
    // of its colour, the rammed ones being those whose square ahead -- one rank
    // up the board for White, one down for Black -- holds an enemy pawn.
    for (int b = 0; b < 64; ++b) {
      if (!is(b, 2, side)) { continue; }

      for (int p = 0; p < 64; ++p) {
        if (!is(p, 0, side) || colour_of(p) != colour_of(b)) { continue; }

        out[PL_BAD_BISHOP] += sign;

        const int ahead = (side == 0) ? p - 8 : p + 8;

        if (ahead >= 0 && ahead < 64 && is(ahead, 0, 1 - side)) {
          out[PL_BAD_BISHOP_RAMMED] += sign;
        }
      }
    }

    // Trapped minors: White's patterns as evaluation.hpp lists them, a piece
    // type, its square, and the alternatives of enemy pawn squares any one of
    // which traps it. An alternative with no first square is no alternative.
    struct trap_t
    {
      int type;  // 1 knight, 2 bishop
      const char* where;
      const char* need[2][2];
    };

    static const trap_t traps[] = {
        {2, "h7", {{"f7", "g6"}, {nullptr, nullptr}}},
        {2, "a7", {{"c7", "b6"}, {nullptr, nullptr}}},
        {2, "h6", {{"g5", "f6"}, {nullptr, nullptr}}},
        {2, "a6", {{"b5", "c6"}, {nullptr, nullptr}}},
        {1, "h8", {{"h7", nullptr}, {"f7", nullptr}}},
        {1, "a8", {{"a7", nullptr}, {"c7", nullptr}}},
        {1, "h7", {{"g7", "h6"}, {"g7", "f6"}}},
        {1, "a7", {{"b7", "a6"}, {"b7", "c6"}}},
    };

    // A piece once, however many of its patterns it meets.
    bool trapped[64] = {};

    for (const trap_t& trap : traps) {
      const int where = named(trap.where, side);

      if (!is(where, trap.type, side)) { continue; }

      for (const auto& alternative : trap.need) {
        if (alternative[0] == nullptr) { continue; }

        bool met = true;

        for (const char* pawn : alternative) {
          if (pawn != nullptr && !is(named(pawn, side), 0, 1 - side)) {
            met = false;
          }
        }

        if (met) { trapped[where] = true; }
      }
    }

    for (int sq = 0; sq < 64; ++sq) {
      if (trapped[sq]) { out[PL_TRAPPED_MINOR] += sign; }
    }

    // Trapped rooks: the king on f1 or g1 shuts in h1, g1, h2 and g2; on c1 or
    // b1, a1, b1, a2 and b2. White's squares; Black's mirrored.
    struct corner_t
    {
      const char* kings[2];
      const char* rooks[4];
    };

    static const corner_t corners[] = {
        {{"f1", "g1"}, {"h1", "g1", "h2", "g2"}},
        {{"c1", "b1"}, {"a1", "b1", "a2", "b2"}},
    };

    for (const corner_t& corner : corners) {
      if (!is(named(corner.kings[0], side), 5, side) &&
          !is(named(corner.kings[1], side), 5, side)) {
        continue;
      }

      for (const char* rook : corner.rooks) {
        if (is(named(rook, side), 3, side)) { out[PL_TRAPPED_ROOK] += sign; }
      }
    }
  }

  return true;
}


// S123's passed pawn family, per side, WHITE first, each a side's own count
// and positive: src/evaluation.hpp's definitions walked square by square from
// the placement, sharing no code with the engine -- no bitboard, no attack
// table, no fill. Indexed by the pawn's own rank from its second, 0 to 5 for
// the first four, 0 to 4 for candidates.
//
//   free        passers (no enemy pawn strictly ahead on the file or either
//               neighbouring file, the own seventh included) whose push
//               square, the square directly ahead, is empty;
//   safe        free passers whose push square no enemy piece attacks, the
//               rays walked outward and stopped at the first piece;
//   king_own    the Chebyshev distance from the side's own king to the push
//   king_enemy  square, or from the enemy king, summed over the rank's
//               passers;
//   candidate   pawns not passed with no enemy pawn strictly ahead on their
//               file, and no square strictly ahead on that file attacked by
//               more enemy pawns than own pawns.
struct passer_family_walk_t
{
  int free[2][6];
  int safe[2][6];
  int king_own[2][6];
  int king_enemy[2][6];
  int candidate[2][5];
};


inline bool passer_family_features(const std::string& placement,
                                   passer_family_walk_t* out)
{
  int kind_at[64];
  bool white_at[64];

  for (int i = 0; i < 64; ++i) {
    kind_at[i] = -1;
    white_at[i] = false;
  }

  int square = 0;
  for (const char c : placement) {
    if (c == '/') { continue; }

    if (c >= '1' && c <= '8') {
      square += c - '0';
      continue;
    }

    bool black = false;
    const int type = piece_from_char(c, &black);

    if (type < 0 || square > 63) { return false; }

    kind_at[square] = type;
    white_at[square] = !black;
    square++;
  }

  if (square != 64) { return false; }

  *out = passer_family_walk_t{};

  const auto on_board = [](int row, int file) {
    return row >= 0 && row <= 7 && file >= 0 && file <= 7;
  };
  const auto piece = [&](int row, int file, int type, bool white) {
    const int sq = row * 8 + file;
    return on_board(row, file) && kind_at[sq] == type && white_at[sq] == white;
  };

  // Whether a piece of colour `white` attacks (row, file).
  const auto attacked_by = [&](int row, int file, bool white) {
    // A pawn attacks one row toward its enemy, one file to each side: White's
    // from the row below in this indexing, Black's from the row above.
    const int pawn_row = white ? row + 1 : row - 1;
    if (piece(pawn_row, file - 1, 0, white) ||
        piece(pawn_row, file + 1, 0, white)) {
      return true;
    }

    static const int knight[8][2] = {{1, 2},   {2, 1},   {2, -1}, {1, -2},
                                     {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
    for (const auto& d : knight) {
      if (piece(row + d[0], file + d[1], 1, white)) { return true; }
    }

    for (int dr = -1; dr <= 1; ++dr) {
      for (int df = -1; df <= 1; ++df) {
        if ((dr != 0 || df != 0) && piece(row + dr, file + df, 5, white)) {
          return true;
        }
      }
    }

    // Sliders: out along each ray to the first piece of either colour.
    for (int dr = -1; dr <= 1; ++dr) {
      for (int df = -1; df <= 1; ++df) {
        if (dr == 0 && df == 0) { continue; }

        const bool diagonal = (dr != 0 && df != 0);

        for (int r = row + dr, f = file + df; on_board(r, f);
             r += dr, f += df) {
          const int sq = r * 8 + f;
          if (kind_at[sq] < 0) { continue; }

          if (white_at[sq] == white &&
              (kind_at[sq] == 4 || kind_at[sq] == (diagonal ? 2 : 3))) {
            return true;
          }

          break;
        }
      }
    }

    return false;
  };

  // How many pawns of colour `white` attack (row, file): 0 to 2.
  const auto pawn_attackers = [&](int row, int file, bool white) {
    const int pawn_row = white ? row + 1 : row - 1;
    return (piece(pawn_row, file - 1, 0, white) ? 1 : 0) +
           (piece(pawn_row, file + 1, 0, white) ? 1 : 0);
  };

  int king_square[2] = {-1, -1};
  for (int sq = 0; sq < 64; ++sq) {
    if (kind_at[sq] == 5) { king_square[white_at[sq] ? 0 : 1] = sq; }
  }

  if (king_square[0] < 0 || king_square[1] < 0) { return false; }

  const auto distance = [](int a, int b) {
    const int ranks = (a / 8 > b / 8) ? a / 8 - b / 8 : b / 8 - a / 8;
    const int files = (a % 8 > b % 8) ? a % 8 - b % 8 : b % 8 - a % 8;
    return (ranks > files) ? ranks : files;
  };

  for (int from = 0; from < 64; ++from) {
    if (kind_at[from] != 0) { continue; }

    const bool white = white_at[from];
    const int side = white ? 0 : 1;
    const int row = from / 8;
    const int file = from % 8;

    if (row == 0 || row == 7) { return false; }

    const int ahead = white ? -1 : 1;
    const int rank = white ? 6 - row : row - 1;

    bool passed = true;
    bool file_clear = true;

    for (int r = row + ahead; r >= 0 && r <= 7; r += ahead) {
      for (int df = -1; df <= 1; ++df) {
        if (piece(r, file + df, 0, !white)) {
          passed = false;
          if (df == 0) { file_clear = false; }
        }
      }
    }

    if (passed) {
      const int push_row = row + ahead;
      const int push = push_row * 8 + file;

      if (kind_at[push] < 0) {
        out->free[side][rank]++;
        if (!attacked_by(push_row, file, !white)) { out->safe[side][rank]++; }
      }

      out->king_own[side][rank] += distance(king_square[side], push);
      out->king_enemy[side][rank] += distance(king_square[1 - side], push);
      continue;
    }

    if (!file_clear) { continue; }

    bool contested = false;
    for (int r = row + ahead; r >= 0 && r <= 7; r += ahead) {
      if (pawn_attackers(r, file, !white) > pawn_attackers(r, file, white)) {
        contested = true;
      }
    }

    // A pawn on its own seventh is always passed, so this is ranks 2 to 6.
    if (!contested) {
      if (rank > 4) { return false; }
      out->candidate[side][rank]++;
    }
  }

  return true;
}

}  // namespace walked_counts
