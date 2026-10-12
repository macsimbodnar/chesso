#pragma once
// evaluate() written as a linear model over its own constants. S028.
//
// The tuner fitted this from S028 to S262: the evaluation as a function of its
// constants rather than of the board, linear, so a pass over a million
// positions cost milliseconds instead of a million calls into the engine. Since
// S262 the tuner fits the engine's own trace instead (tools/tuner_trace.hpp),
// which is evaluate() exactly rather than to within this file's truncation
// bound, and this file is the mirror two tools read: tools/feature_audit, for
// its identities, columns and planted labels, and tools/truncation_scan, which
// measures this file's residual against the engine.
//
// The claim this file makes is that it computes what evaluate() computes. That
// claim is a test: tests/test_eval_model.cpp loads positions into the engine
// and compares the two, and a mirror that drifts from the engine would hand
// those tools a different function and nothing downstream would notice.
//
// It lives in tools/ because it is not part of the engine. Nothing in src/
// includes it.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "eval_tables.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"

namespace eval_model
{

// The parameter layout and the vector the engine ships moved to
// tools/tuner_layout.hpp at S262, when the tuner stopped reading this file: the
// layout is the tuner's, and this file is now the mirror tools/feature_audit
// and tools/truncation_scan read. Imported under their old names so every
// eval_model:: reference keeps compiling.
using tuner_layout::EG_BASE;
using tuner_layout::KING_SAFETY_COUNT;
using tuner_layout::KS_EG_BASE;
using tuner_layout::KS_MG_BASE;
using tuner_layout::MATERIAL_COUNT;
using tuner_layout::MG_BASE;
using tuner_layout::MOB_EG_BASE;
using tuner_layout::MOB_MG_BASE;
using tuner_layout::MOBILITY_COUNT;
using tuner_layout::PARAM_COUNT;
using tuner_layout::PASSED_PAWN_COUNT;
using tuner_layout::PAWN_STRUCTURE_COUNT;
using tuner_layout::PIECE_PLACEMENT_COUNT;
using tuner_layout::PL_EG_BASE;
using tuner_layout::PL_MG_BASE;
using tuner_layout::PP_EG_BASE;
using tuner_layout::PP_MG_BASE;
using tuner_layout::PS_EG_BASE;
using tuner_layout::PS_MG_BASE;
using tuner_layout::SQUARE_COUNT;
using tuner_layout::starting_params;
using tuner_layout::TEMPO_COUNT;
using tuner_layout::TEMPO_EG_BASE;
using tuner_layout::TEMPO_MG_BASE;

// What the three counts are, in the order the weights are indexed. Named here
// because the engine names them only in that prose, and an index nobody can
// read is an index a caller gets wrong.
enum pawn_structure_feature_t
{
  PS_ISOLATED,
  PS_DOUBLED,
  PS_BACKWARD
};

// What the three counts are, in the order the weights are indexed. Named here
// because the engine names them only in the prose over those arrays, and an
// index nobody can read is an index a caller gets wrong.
enum piece_placement_feature_t
{
  PL_BISHOP_PAIR,
  PL_ROOK_OPEN,
  PL_ROOK_HALF_OPEN
};


// A piece is one uint16: the top bit says Black, the rest is type * 64 plus the
// square the tables are read at, already mirrored for Black the way
// eval_add_piece() mirrors it.
constexpr uint16_t BLACK_BIT = 0x8000;


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


// Reads the placement field of a FEN, the part before the first space. Index 0
// is a8 and index 63 is h1, which is the order a FEN is written in and the
// order the tables in eval_tables.hpp are written in, so the first character
// lands on index 0 and no transposition is needed anywhere.
inline bool parse_placement(const std::string& placement,
                            std::vector<uint16_t>* out)
{
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

    const int used = black ? (square ^ 56) : square;
    const uint16_t code = static_cast<uint16_t>(type * 64 + used);

    out->push_back(black ? static_cast<uint16_t>(code | BLACK_BIT) : code);
    square++;
  }

  return square == 64;
}


// The phase evaluate() tapers on, rebuilt from the same piece list. Kings and
// pawns weigh nothing and a promotion can push the sum past a full board, which
// game_phase() clamps and this clamps the same way.
inline int phase_of(const uint16_t* pieces, size_t count)
{
  int phase = 0;

  for (size_t i = 0; i < count; ++i) {
    const size_t type = (pieces[i] & ~BLACK_BIT) / 64;
    phase += phase_value[type];
  }

  return (phase > GAME_PHASE_MAX) ? GAME_PHASE_MAX : phase;
}


// Mobility counts, White minus Black, per piece type in knight, bishop, rook,
// queen order. Walked ray by ray from the placement rather than read out of the
// engine's magic tables, deliberately: this file exists to disagree with the
// engine when the two stop computing the same thing, and a shared
// implementation cannot do that.
//
// Squares here are true board squares, index 0 = a8 to index 63 = h1, not the
// mirrored ones parse_placement() stores.
inline bool mobility_features(const std::string& placement, int out[4])
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

  for (int i = 0; i < 4; ++i) {
    out[i] = 0;
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
        if (kind_at[to] < 0 || white_at[to] != white) { targets++; }
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

          if (kind_at[to] < 0) {
            targets++;
          } else {
            if (white_at[to] != white) { targets++; }
            break;
          }

          r += step[0];
          f += step[1];
        }
      }
    }

    out[type - 1] += white ? targets : -targets;
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
// Per side rather than only as the difference below, because that difference is
// what the tuner needs and not what a test can check. White gaining a count and
// Black gaining the same one cancels in the difference, and the engine's own
// counts are per side, so this is the form the two implementations are compared
// in -- tests/test_eval_model, "the model counts what the engine counts".
//
// Squares are true board squares, index 0 = a8 to index 63 = h1, not the
// mirrored ones parse_placement() stores. "Ahead" is toward the enemy, so a
// white king's shield sits on lower indices and a black king's on higher ones.
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


// The same counts as one White-minus-Black vector, which is the form the model
// is linear in: a positive count means White has more of that thing around
// White's king, so the attacker weights are expected to come back negative and
// the shield weights positive.
inline bool king_safety_features(const std::string& placement,
                                 int out[KING_SAFETY_COUNT])
{
  int by_colour[2][KING_SAFETY_COUNT];

  if (!king_safety_features_by_colour(placement, by_colour)) { return false; }

  for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
    out[i] = by_colour[0][i] - by_colour[1][i];
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
// Squares are true board squares, index 0 = a8 to index 63 = h1, not the
// mirrored ones parse_placement() stores. In that indexing a white pawn's home
// rank is 6 and a black pawn's is 1, which is why the two colours count their
// buckets in opposite directions.
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
// Squares are true board squares, index 0 = a8 to index 63 = h1, not the
// mirrored ones parse_placement() stores. "Ahead" is toward the enemy, so a
// lower index for White and a higher one for Black, and "behind" is the
// opposite -- which is why the two colours compare ranks in opposite
// directions.
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
// file, rook on a half-open file order. Each is about a side's own pieces.
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
// Walked by hand from the placement for the same reason mobility_features() is:
// this file has to be able to disagree with the engine, and code shared with it
// cannot.
//
// Squares are true board squares, index 0 = a8 to index 63 = h1, not the
// mirrored ones parse_placement() stores.
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

  return true;
}


// The tempo feature: +1 with White to move, -1 with Black, which is the White
// relative form of "the side to move gets the bonus".
//
// It takes the FEN's second field and not the placement, because it is the one
// feature that is not about the board. Refusing anything else rather than
// defaulting to White: a row whose side to move could not be read is a row this
// term would fit backwards on half the time, and the caller already reports a
// refusal.
inline bool tempo_feature(const std::string& side_to_move, int* out)
{
  if (side_to_move == "w") {
    *out = 1;
    return true;
  }

  if (side_to_move == "b") {
    *out = -1;
    return true;
  }

  return false;
}


// White relative, in floating point. evaluate() divides in integers and
// truncates towards zero, so the two agree to within one centipawn rather than
// exactly; the tuner works in floating point precisely so that the derivative
// exists.
//
// The engine carries each mg/eg pair below packed in one integer (score_t,
// src/data_structures.hpp, S117) and this model keeps them apart. The two are
// the same sums: a packed sum's halves are exactly the separate sums, and the
// engine takes them apart before any division, so the truncations this model
// is compared against did not move -- one for stage one, one for tempo, and
// two in stage two, which tapers mobility and king safety separately (S055's
// single taper read H0 and was reverted).
//
// Every feature vector is a required argument. king_safety briefly had a
// default of nullptr and it was a trap: a five-argument call dropped the term
// silently, which reads as correct only while the weights are zero and starts
// fitting a different function than the engine computes the moment they are
// not.
//
// `tempo` is the last argument and an int rather than a one-element vector, so
// the same mistake does not compile here: it is +1 or -1, not a pointer, and a
// call that leaves it out or hands it a feature array is rejected by the type
// rather than by a reviewer.
inline double evaluate(const uint16_t* pieces,
                       size_t count,
                       int phase,
                       const int* mobility,
                       const double* params,
                       const int* king_safety,
                       const int* passed_pawn,
                       const int* pawn_structure,
                       const int* piece_placement,
                       int tempo)
{
  const double mg_weight = static_cast<double>(phase) / GAME_PHASE_MAX;
  const double eg_weight =
      static_cast<double>(GAME_PHASE_MAX - phase) / GAME_PHASE_MAX;

  double material = 0.0;
  double mg = 0.0;
  double eg = 0.0;

  for (size_t i = 0; i < count; ++i) {
    const uint16_t code = pieces[i];
    const double sign = (code & BLACK_BIT) ? -1.0 : 1.0;
    const size_t square = code & ~BLACK_BIT;
    const size_t type = square / 64;

    if (type < MATERIAL_COUNT) { material += sign * params[type]; }

    mg += sign * params[MG_BASE + square];
    eg += sign * params[EG_BASE + square];
  }

  double mobility_mg_sum = 0.0;
  double mobility_eg_sum = 0.0;

  for (size_t t = 0; t < MOBILITY_COUNT; ++t) {
    mobility_mg_sum += mobility[t] * params[MOB_MG_BASE + t];
    mobility_eg_sum += mobility[t] * params[MOB_EG_BASE + t];
  }

  double king_safety_mg_sum = 0.0;
  double king_safety_eg_sum = 0.0;

  for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
    king_safety_mg_sum += king_safety[t] * params[KS_MG_BASE + t];
    king_safety_eg_sum += king_safety[t] * params[KS_EG_BASE + t];
  }

  // One clamp on the whole stage-two correction, exactly where evaluate() puts
  // it, because the lazy shortcut's soundness is a bound on the sum of the
  // expensive terms and not on each of them. Clamping mobility and king safety
  // separately would let the pair reach twice the margin and fit a function the
  // engine does not compute.
  //
  // With mobility alone it bound essentially never -- the term's maximum over
  // 149084 real positions was 143 against a bound of 150 -- so the kink it puts
  // in the objective was not somewhere the fit spent its time. King safety
  // shares the same budget now, so that is no longer a safe assumption: if a
  // fit pushes these weights far enough for the clamp to bind often, the margin
  // is what has to move, and this is where it shows up.
  double stage_two = (mobility_mg_sum + king_safety_mg_sum) * mg_weight +
                     (mobility_eg_sum + king_safety_eg_sum) * eg_weight;

  if (stage_two > LAZY_EVAL_MARGIN) { stage_two = LAZY_EVAL_MARGIN; }
  if (stage_two < -LAZY_EVAL_MARGIN) { stage_two = -LAZY_EVAL_MARGIN; }

  double passed_pawn_mg_sum = 0.0;
  double passed_pawn_eg_sum = 0.0;

  for (size_t t = 0; t < PASSED_PAWN_COUNT; ++t) {
    passed_pawn_mg_sum += passed_pawn[t] * params[PP_MG_BASE + t];
    passed_pawn_eg_sum += passed_pawn[t] * params[PP_EG_BASE + t];
  }

  // Outside the clamp, and that is the whole point of where this line sits.
  // Passed pawns are part of evaluate_cheap(), the stage the lazy shortcut
  // takes when it skips the expensive terms, so the margin says nothing about
  // them. Folding this sum into stage_two would clamp a quantity the engine
  // never clamps, and the two would agree everywhere except on the positions
  // where it binds -- which is exactly where a bound has to be right.
  const double stage_one_passed =
      passed_pawn_mg_sum * mg_weight + passed_pawn_eg_sum * eg_weight;

  double pawn_structure_mg_sum = 0.0;
  double pawn_structure_eg_sum = 0.0;

  for (size_t t = 0; t < PAWN_STRUCTURE_COUNT; ++t) {
    pawn_structure_mg_sum += pawn_structure[t] * params[PS_MG_BASE + t];
    pawn_structure_eg_sum += pawn_structure[t] * params[PS_EG_BASE + t];
  }

  // Outside the clamp for the reason written over the line above: pawn
  // structure is part of evaluate_cheap() too, so the lazy margin says nothing
  // about it and folding it into stage_two would clamp a quantity the engine
  // never clamps.
  const double stage_one_structure =
      pawn_structure_mg_sum * mg_weight + pawn_structure_eg_sum * eg_weight;

  double piece_placement_mg_sum = 0.0;
  double piece_placement_eg_sum = 0.0;

  for (size_t t = 0; t < PIECE_PLACEMENT_COUNT; ++t) {
    piece_placement_mg_sum += piece_placement[t] * params[PL_MG_BASE + t];
    piece_placement_eg_sum += piece_placement[t] * params[PL_EG_BASE + t];
  }

  // Outside the clamp for the reason written over the two lines above: the term
  // is part of evaluate_cheap(), so the lazy margin says nothing about it and
  // folding it into stage_two would clamp a quantity the engine never clamps.
  const double stage_one_placement =
      piece_placement_mg_sum * mg_weight + piece_placement_eg_sum * eg_weight;

  // Outside the clamp for the reason written over the three lines above: the
  // term is part of evaluate_cheap(), so the lazy margin says nothing about it.
  //
  // The feature carries the sign, which is what makes this White relative: the
  // engine adds the same positive bonus to whichever side is to move, and
  // multiplying by +1 or -1 is that statement turned round to White's point of
  // view. Everything else in this sum is a difference between the two colours
  // and this is the one quantity that is not.
  const double stage_one_tempo = tempo * (params[TEMPO_MG_BASE] * mg_weight +
                                          params[TEMPO_EG_BASE] * eg_weight);

  return material + mg * mg_weight + eg * eg_weight + stage_one_passed +
         stage_one_structure + stage_one_placement + stage_one_tempo +
         stage_two;
}


}  // namespace eval_model
