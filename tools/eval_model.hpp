#pragma once
// evaluate() written as a linear model over its own constants. S028.
//
// The tuner fits `piece_value[0..4]`, `psqt_mg[6][64]` and `psqt_eg[6][64]` to
// game outcomes. Fitting anything needs the evaluation as a function of those
// numbers rather than of the board, and that function is linear, which is what
// makes a pass over a million positions cost milliseconds instead of a million
// calls into the engine.
//
// The claim this file makes is that it computes what evaluate() computes. That
// claim is a test: tests/test_eval_model.cpp loads positions into the engine
// and compares the two, and a model that drifts from the engine would otherwise
// tune the wrong function and nothing downstream would notice.
//
// It lives in tools/ because it is not part of the engine. Nothing in src/
// includes it.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "eval_tables.hpp"
#include "evaluation.hpp"

namespace eval_model
{

// piece_value[0..4], then psqt_mg[6][64], then psqt_eg[6][64]. The king's
// material value is not a parameter: both sides always have exactly one in a
// legal position, so it can only cancel.
constexpr size_t MATERIAL_COUNT = 5;
constexpr size_t SQUARE_COUNT = 6 * 64;

// Mobility, added at S034: four weights for the middlegame and four for the
// endgame, in knight, bishop, rook, queen order. Pawns and kings have no
// mobility term.
//
// The counts themselves are a property of the position and not of the
// parameters, so they enter as features and the model stays linear -- which is
// what keeps a pass over a million positions cheap and the gradient closed
// form.
constexpr size_t MOBILITY_COUNT = 4;

constexpr size_t MG_BASE = MATERIAL_COUNT;
constexpr size_t EG_BASE = MATERIAL_COUNT + SQUARE_COUNT;
constexpr size_t MOB_MG_BASE = EG_BASE + SQUARE_COUNT;
constexpr size_t MOB_EG_BASE = MOB_MG_BASE + MOBILITY_COUNT;

constexpr size_t PARAM_COUNT =
    MATERIAL_COUNT + 2 * SQUARE_COUNT + 2 * MOBILITY_COUNT;

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


// White relative, in floating point. evaluate() divides in integers and
// truncates towards zero, so the two agree to within one centipawn rather than
// exactly; the tuner works in floating point precisely so that the derivative
// exists.
inline double evaluate(const uint16_t* pieces,
                       size_t count,
                       int phase,
                       const int* mobility,
                       const double* params)
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

  // Clamped exactly as evaluate_mobility() clamps it, because the lazy
  // shortcut's soundness depends on that bound and a model that ignored it
  // would fit a function the engine does not compute. It binds essentially
  // never on real positions -- the term's maximum over 149084 of them was 143
  // against a bound of 150 -- so the kink it puts in the objective is not
  // somewhere the fit spends its time. If a fit ever pushes these weights far
  // enough for it to bind often, that is worth knowing rather than hiding.
  double mobility_score =
      mobility_mg_sum * mg_weight + mobility_eg_sum * eg_weight;

  if (mobility_score > LAZY_EVAL_MARGIN) { mobility_score = LAZY_EVAL_MARGIN; }
  if (mobility_score < -LAZY_EVAL_MARGIN) {
    mobility_score = -LAZY_EVAL_MARGIN;
  }

  return material + mg * mg_weight + eg * eg_weight + mobility_score;
}


// The constants the engine currently ships, as the parameter vector. This is
// the tuner's starting point, so a fit that finds nothing reports the error of
// the hand-written numbers rather than the error of noise.
inline void starting_params(double* params)
{
  for (size_t i = 0; i < MATERIAL_COUNT; ++i) {
    params[i] = piece_value[i];
  }

  for (size_t type = 0; type < 6; ++type) {
    for (size_t square = 0; square < 64; ++square) {
      params[MG_BASE + type * 64 + square] = psqt_mg[type][square];
      params[EG_BASE + type * 64 + square] = psqt_eg[type][square];
    }
  }

  for (size_t t = 0; t < MOBILITY_COUNT; ++t) {
    params[MOB_MG_BASE + t] = mobility_mg[t];
    params[MOB_EG_BASE + t] = mobility_eg[t];
  }
}

}  // namespace eval_model
