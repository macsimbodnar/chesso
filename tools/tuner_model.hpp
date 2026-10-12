#pragma once
// The mirror's dataset and the two functions that read it: load(), and the
// forward model over one row. S100.
//
// Until S262 this was the tuner's model, and it held the fit's error, its
// gradient and its step too. S262 moved the tuner onto the engine's own trace
// (tools/tuner_trace.hpp) and its step into tools/tuner_optimiser.hpp, and
// retired the mirror's error_range() and gradient() with the old path once
// S262's held-out comparison held. What is left is what tools/feature_audit
// reads: the mirror's features (tools/eval_model.hpp) packed per row, which its
// identities, its columns and its `--plant` labels are built from. Its loader's
// cases moved with it to tests/test_eval_model.cpp.
//
// load() refuses a row without one king a side, as load_FEN() does (S275): the
// king gauges feature_audit checks are null only there.
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "eval_model.hpp"
#include "tuner_groups.hpp"
#include "tuner_split.hpp"
#include "tuner_target.hpp"

namespace tuner_model
{

using eval_model::BLACK_BIT;
using eval_model::KING_SAFETY_COUNT;
using eval_model::PASSED_PAWN_COUNT;
using eval_model::PAWN_STRUCTURE_COUNT;
using eval_model::PIECE_PLACEMENT_COUNT;


struct dataset_t
{
  std::vector<uint16_t> pieces;   // concatenated, indexed by offsets
  std::vector<uint32_t> offsets;  // size = positions + 1
  std::vector<uint8_t> phase;
  std::vector<float> result;

  // The engine's own score for the position, White relative, in centipawns --
  // datagen's third column, written since S028 and read by nothing until S075.
  // int16 because datagen bounds it by --quiet-limit and the whole 11003693-row
  // `selfplay_v2.tsv` lies in -999 .. 999; parse_score() refuses a corpus that
  // does not fit rather than truncating one. 11.0 M rows cost 22 MB.
  std::vector<int16_t> score;

  // Mobility counts, White minus Black, four per position. A property of the
  // position rather than of the parameters, so it is extracted once here and
  // the model stays linear. int16 because the extremes are in the low
  // hundreds: 1.49 M rows cost 12 MB.
  std::vector<int16_t> mobility;

  // King safety counts, White minus Black, nine per position, extracted once
  // for the same reason. The counts are bounded by the number of pieces on the
  // board, so int16 is again far more room than they need: 1.49 M rows cost
  // 27 MB.
  std::vector<int16_t> king_safety;

  // Passed pawn counts, White minus Black, five per position since S134,
  // extracted once for the same reason. Eight pawns per side bounds every
  // bucket, so int16 is again far more room than they need: 1.49 M rows cost
  // 15 MB.
  std::vector<int16_t> passed_pawn;

  // Pawn structure counts, White minus Black, three per position, extracted
  // once for the same reason. Eight pawns per side bounds every one of them,
  // so int16 is again far more room than they need: 1.49 M rows cost 9 MB.
  std::vector<int16_t> pawn_structure;

  // Piece placement counts, White minus Black, three per position since S134,
  // extracted once for the same reason. The bishop pair is 1, 0 or -1 and the
  // two rook counts are bounded by the rooks on the board, so int16 is again
  // far more room than they need: 1.49 M rows cost 9 MB.
  std::vector<int16_t> piece_placement;

  // The tempo feature, one per position: +1 with White to move and -1 with
  // Black. Not a count of anything and not extracted from the placement -- it
  // is the FEN's second field, which is why load() below has to split the FEN
  // rather than take its first token and stop. int8 because the only two values
  // it ever holds are +1 and -1: 1.49 M rows cost 1.5 MB.
  std::vector<int8_t> tempo;

  // The ply each position stands at, from the FEN's move number and side to
  // move. Nothing in the model reads it: it is what tells one game's rows from
  // the next's, and only the validation split needs that. S066. 11.0 M rows
  // cost 44 MB.
  std::vector<uint32_t> ply;

  size_t size() const { return phase.size(); }
};


inline bool load(const std::string& path, dataset_t* data)
{
  std::ifstream file(path);

  if (!file) { return false; }

  std::string line;
  std::vector<uint16_t> pieces;

  data->offsets.push_back(0);

  while (std::getline(file, line)) {
    if (line.empty()) { continue; }

    std::istringstream fields(line);
    std::string placement, side_to_move, result_text, score_text, phase_text;

    // The FEN's first field is the placement and its second is the side to
    // move, which the tempo term reads and nothing else does. The line is split
    // on tabs first and the FEN on spaces after, because the space between
    // those two fields is inside the first tab-separated column.
    std::string fen;
    if (!std::getline(fields, fen, '\t')) { continue; }
    if (!std::getline(fields, result_text, '\t')) { continue; }
    if (!std::getline(fields, score_text, '\t')) { continue; }
    if (!std::getline(fields, phase_text, '\t')) { continue; }

    std::istringstream fen_fields(fen);
    if (!(fen_fields >> placement)) { continue; }
    if (!(fen_fields >> side_to_move)) { continue; }

    pieces.clear();

    if (!eval_model::parse_placement(placement, &pieces)) {
      fprintf(stderr, "bad placement: %s\n", placement.c_str());
      return false;
    }

    // One king a side, as load_FEN() requires of every position the engine
    // evaluates. The king gauges the fit holds are null only on such a row, so
    // a hold on any other would move its score. S275.
    int kings[2] = {0, 0};
    for (const uint16_t code : pieces) {
      if ((code & ~BLACK_BIT) / 64 == tuner_groups::KING_TYPE) {
        kings[(code & BLACK_BIT) ? 1 : 0]++;
      }
    }

    if (kings[0] != 1 || kings[1] != 1) {
      fprintf(stderr, "not one king a side: %s\n", line.c_str());
      return false;
    }

    const double result = atof(result_text.c_str());
    const int stored_phase = atoi(phase_text.c_str());

    if (result < 0.0 || result > 1.0 || stored_phase < 0 ||
        stored_phase > GAME_PHASE_MAX) {
      fprintf(stderr, "bad row: %s\n", line.c_str());
      return false;
    }

    // The phase the model tapers on is recomputed from the placement, the way
    // evaluate() gets it from the board, and the stored column is read for
    // its range and nothing else. Trusting it was latent
    // (2026-09-10_adversarial-F36): equal on every corpus so far, and wrong on
    // all of them the day phase_value changes, with the range check still
    // passing. S134; tools/feature_audit's columns report counts the rows
    // where the two disagree.
    const int phase = eval_model::phase_of(pieces.data(), pieces.size());

    // Strictly, and refused rather than defaulted: `atoi` answers 0 for a field
    // it cannot read, and 0 is a legal score meaning "the search called this
    // equal". A corpus whose third column is unreadable would fit at any
    // non-zero --lambda against a label that says every position is drawn.
    // S075.
    int score = 0;
    if (!tuner_target::parse_score(score_text, &score)) {
      fprintf(stderr, "unusable score column '%s' in row: %s\n",
              score_text.c_str(), line.c_str());
      return false;
    }

    int counts[4] = {0, 0, 0, 0};
    if (!eval_model::mobility_features(placement, counts)) {
      fprintf(stderr, "bad placement for mobility: %s\n", placement.c_str());
      return false;
    }

    int safety[KING_SAFETY_COUNT] = {0};
    if (!eval_model::king_safety_features(placement, safety)) {
      fprintf(stderr, "bad placement for king safety: %s\n", placement.c_str());
      return false;
    }

    int passed[PASSED_PAWN_COUNT] = {0};
    if (!eval_model::passed_pawn_features(placement, passed)) {
      fprintf(stderr, "bad placement for passed pawns: %s\n",
              placement.c_str());
      return false;
    }

    int structure[PAWN_STRUCTURE_COUNT] = {0};
    if (!eval_model::pawn_structure_features(placement, structure)) {
      fprintf(stderr, "bad placement for pawn structure: %s\n",
              placement.c_str());
      return false;
    }

    int placement_counts[PIECE_PLACEMENT_COUNT] = {0};
    if (!eval_model::piece_placement_features(placement, placement_counts)) {
      fprintf(stderr, "bad placement for piece placement: %s\n",
              placement.c_str());
      return false;
    }

    int tempo = 0;
    if (!eval_model::tempo_feature(side_to_move, &tempo)) {
      fprintf(stderr, "bad side to move: %s\n", side_to_move.c_str());
      return false;
    }

    // The validation split is cut between games and the boundary is
    // reconstructed from this, since the four-column format carries no game id.
    // A row without a FEN move number is refused rather than guessed at: a row
    // whose ply is unknown cannot be attributed to a game, and a split that
    // guesses is the defect S066 exists to remove.
    uint32_t ply = 0;
    if (!tuner_split::row_ply(line, &ply)) {
      fprintf(stderr, "row has no usable FEN move number: %s\n", line.c_str());
      return false;
    }

    data->pieces.insert(data->pieces.end(), pieces.begin(), pieces.end());
    data->offsets.push_back(static_cast<uint32_t>(data->pieces.size()));

    for (const int count : counts) {
      data->mobility.push_back(static_cast<int16_t>(count));
    }
    for (const int count : safety) {
      data->king_safety.push_back(static_cast<int16_t>(count));
    }
    for (const int count : passed) {
      data->passed_pawn.push_back(static_cast<int16_t>(count));
    }
    for (const int count : structure) {
      data->pawn_structure.push_back(static_cast<int16_t>(count));
    }
    for (const int count : placement_counts) {
      data->piece_placement.push_back(static_cast<int16_t>(count));
    }
    data->tempo.push_back(static_cast<int8_t>(tempo));
    data->ply.push_back(ply);
    data->phase.push_back(static_cast<uint8_t>(phase));
    data->result.push_back(static_cast<float>(result));
    data->score.push_back(static_cast<int16_t>(score));
  }

  return data->size() > 0;
}


inline double evaluate_position(const dataset_t& data,
                                size_t index,
                                const double* params)
{
  const int mobility[4] = {
      data.mobility[index * 4 + 0], data.mobility[index * 4 + 1],
      data.mobility[index * 4 + 2], data.mobility[index * 4 + 3]};

  int safety[KING_SAFETY_COUNT];
  for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
    safety[i] = data.king_safety[index * KING_SAFETY_COUNT + i];
  }

  int passed[PASSED_PAWN_COUNT];
  for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
    passed[i] = data.passed_pawn[index * PASSED_PAWN_COUNT + i];
  }

  int structure[PAWN_STRUCTURE_COUNT];
  for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
    structure[i] = data.pawn_structure[index * PAWN_STRUCTURE_COUNT + i];
  }

  int placement[PIECE_PLACEMENT_COUNT];
  for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
    placement[i] = data.piece_placement[index * PIECE_PLACEMENT_COUNT + i];
  }

  return eval_model::evaluate(&data.pieces[data.offsets[index]],
                              data.offsets[index + 1] - data.offsets[index],
                              data.phase[index], mobility, params, safety,
                              passed, structure, placement, data.tempo[index]);
}


}  // namespace tuner_model
