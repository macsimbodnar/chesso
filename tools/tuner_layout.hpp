#pragma once
// The tuner's parameter vector: which number sits at which index, and the
// values the engine ships at each. S262 moved it out of tools/eval_model.hpp,
// where it was written at S028 beside the hand-written mirror of evaluate()
// the tuner fitted until then. The tuner now reads the evaluation's own trace
// (tools/tuner_trace.hpp), and so since S276 do tools/feature_audit and
// tools/truncation_scan, which kept the mirror until S276 deleted it; the
// layout lives here, where the trace's term table, those tools and
// tools/tuner_groups.hpp read it. Nothing in it changed in the move: the
// indices, the order and the 823 are the ones every fit since S134 emitted.
#include <cstddef>
#include <cstdint>
#include "eval_tables.hpp"
#include "evaluation.hpp"

namespace tuner_layout
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

// King safety, added at S027: nine counts about a side's own king, weighted for
// the middlegame and for the endgame and tapered the way mobility is. Linear in
// its weights for the same reason, so the counts enter as features too.
//
// Taken from the engine's enum rather than written as 9, so that a feature
// added there cannot leave the model fitting a shorter vector than the engine
// reads.
constexpr size_t KING_SAFETY_COUNT = KS_FEATURE_COUNT;

// Passed pawns, added at S035: five weights for the middlegame and five for the
// endgame, indexed by how far the pawn has advanced rather than by anything
// about the pawn itself. Bucket 0 is a pawn on its own second rank and bucket 4
// one on its own sixth. A pawn cannot stand on its own first or eighth rank,
// and one on its own seventh is in no bucket: it is a passer by definition, so
// a bucket for it was exactly the signed sum of eight psqt[pawn] columns, and
// S134 folded that bucket into them and deleted it (DEC-090). Six until then.
//
// Buckets rather than one weight scaled by the rank because the value of a
// passed pawn is not linear in how far it has come, and because the model is
// only allowed to be linear in its own parameters -- a rank multiplier would be
// a feature, not a parameter.
//
// Taken from the width of the engine's own weight array, as the two counts
// below are, so the model cannot fit a different number of buckets than the
// engine reads.
constexpr size_t PASSED_PAWN_COUNT =
    sizeof(passed_pawn_mg) / sizeof(passed_pawn_mg[0]);

// Pawn structure, added at S027: three counts per side -- isolated, doubled,
// backward, in that order -- weighted for the middlegame and for the endgame.
// The counts are a property of the position, so they enter as features and the
// model stays linear, exactly like the three terms above.
//
// Taken from the width of the engine's own weight array rather than written as
// 3, so that a feature added there cannot leave the model fitting a shorter
// vector than the engine reads. King safety takes the same guarantee from
// KS_FEATURE_COUNT; this term has no enum to take it from, because its index
// order is stated in prose over the arrays in src/evaluation.hpp instead.
constexpr size_t PAWN_STRUCTURE_COUNT =
    sizeof(pawn_structure_mg) / sizeof(pawn_structure_mg[0]);

// Piece placement, added at S027: three counts per side -- bishop pair, rook on
// an open file, rook on a half-open file, in that order -- weighted for the
// middlegame and for the endgame. The counts are a property of the position, so
// they enter as features and the model stays linear, exactly like the four
// terms above. Four until S134 deleted rook on the seventh, the signed sum of
// eight psqt[rook] columns (DEC-090).
//
// Taken from the width of the engine's own weight array rather than written as
// 3, for the reason the pawn structure count is: a feature added there cannot
// then leave the model fitting a shorter vector than the engine reads.
constexpr size_t PIECE_PLACEMENT_COUNT =
    sizeof(piece_placement_mg) / sizeof(piece_placement_mg[0]);

// Tempo, added at S027: one weight for the middlegame and one for the endgame,
// tapered the way everything above is.
//
// The odd one out, and in the one way that matters here. Every other feature is
// a count of something on the board and is read out of the placement field;
// this one is not read from the board at all. The engine adds the bonus to the
// side to move's score, so in this White-relative model the feature is +1 with
// White to move and -1 with Black -- which means the model needs the FEN's
// *second* field, the only term here that does.
constexpr size_t TEMPO_COUNT = 1;

constexpr size_t MG_BASE = MATERIAL_COUNT;
constexpr size_t EG_BASE = MATERIAL_COUNT + SQUARE_COUNT;
constexpr size_t MOB_MG_BASE = EG_BASE + SQUARE_COUNT;
constexpr size_t MOB_EG_BASE = MOB_MG_BASE + MOBILITY_COUNT;
constexpr size_t KS_MG_BASE = MOB_EG_BASE + MOBILITY_COUNT;
constexpr size_t KS_EG_BASE = KS_MG_BASE + KING_SAFETY_COUNT;
constexpr size_t PP_MG_BASE = KS_EG_BASE + KING_SAFETY_COUNT;
constexpr size_t PP_EG_BASE = PP_MG_BASE + PASSED_PAWN_COUNT;
constexpr size_t PS_MG_BASE = PP_EG_BASE + PASSED_PAWN_COUNT;
constexpr size_t PS_EG_BASE = PS_MG_BASE + PAWN_STRUCTURE_COUNT;
constexpr size_t PL_MG_BASE = PS_EG_BASE + PAWN_STRUCTURE_COUNT;
constexpr size_t PL_EG_BASE = PL_MG_BASE + PIECE_PLACEMENT_COUNT;
constexpr size_t TEMPO_MG_BASE = PL_EG_BASE + PIECE_PLACEMENT_COUNT;
constexpr size_t TEMPO_EG_BASE = TEMPO_MG_BASE + TEMPO_COUNT;

constexpr size_t PARAM_COUNT =
    MATERIAL_COUNT + 2 * SQUARE_COUNT + 2 * MOBILITY_COUNT +
    2 * KING_SAFETY_COUNT + 2 * PASSED_PAWN_COUNT + 2 * PAWN_STRUCTURE_COUNT +
    2 * PIECE_PLACEMENT_COUNT + 2 * TEMPO_COUNT;


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

  for (size_t t = 0; t < KING_SAFETY_COUNT; ++t) {
    params[KS_MG_BASE + t] = king_safety_mg[t];
    params[KS_EG_BASE + t] = king_safety_eg[t];
  }

  for (size_t t = 0; t < PASSED_PAWN_COUNT; ++t) {
    params[PP_MG_BASE + t] = passed_pawn_mg[t];
    params[PP_EG_BASE + t] = passed_pawn_eg[t];
  }

  for (size_t t = 0; t < PAWN_STRUCTURE_COUNT; ++t) {
    params[PS_MG_BASE + t] = pawn_structure_mg[t];
    params[PS_EG_BASE + t] = pawn_structure_eg[t];
  }

  for (size_t t = 0; t < PIECE_PLACEMENT_COUNT; ++t) {
    params[PL_MG_BASE + t] = piece_placement_mg[t];
    params[PL_EG_BASE + t] = piece_placement_eg[t];
  }

  // No loop: TEMPO_COUNT is 1 and writing it as one would suggest there is a
  // vector here to index, which is exactly the thing this term does not have.
  params[TEMPO_MG_BASE] = tempo_mg;
  params[TEMPO_EG_BASE] = tempo_eg;
}
}  // namespace tuner_layout
