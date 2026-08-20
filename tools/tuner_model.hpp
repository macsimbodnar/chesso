#pragma once
// The tuner's dataset and the three functions that read it: load(), the forward
// model over one row, and the full-batch gradient. S100.
//
// Moved out of tools/tuner.cpp so that tests/test_tuner_gradient.cpp and
// tools/feature_audit.cpp can reach them. The same reason S041 moved GROUP_LIST
// into tools/tuner_groups.hpp: a function nothing outside one translation unit
// can call is a function nothing outside it can check, and the gradient is the
// one piece of the fit no test had ever touched -- the comments in gradient()
// record three separate occasions when a term's block was forgotten and the fit
// silently returned the weights exactly as it was handed them.
//
// Nothing here changed when it moved. The fit is the same fit: `tuner` over the
// same corpus at the same seed, thread count and epoch budget prints identical
// epoch reports before and after, which is the check DEV_MANUAL.md already
// documents for --freeze at its default.
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "eval_model.hpp"
#include "tuner_split.hpp"
#include "tuner_target.hpp"

namespace tuner_model
{

using eval_model::BLACK_BIT;
using eval_model::EG_BASE;
using eval_model::KING_SAFETY_COUNT;
using eval_model::KS_EG_BASE;
using eval_model::KS_MG_BASE;
using eval_model::MATERIAL_COUNT;
using eval_model::MG_BASE;
using eval_model::MOB_EG_BASE;
using eval_model::MOB_MG_BASE;
using eval_model::MOBILITY_COUNT;
using eval_model::PARAM_COUNT;
using eval_model::PASSED_PAWN_COUNT;
using eval_model::PAWN_STRUCTURE_COUNT;
using eval_model::PIECE_PLACEMENT_COUNT;
using eval_model::PL_EG_BASE;
using eval_model::PL_MG_BASE;
using eval_model::PP_EG_BASE;
using eval_model::PP_MG_BASE;
using eval_model::PS_EG_BASE;
using eval_model::PS_MG_BASE;
using eval_model::TEMPO_EG_BASE;
using eval_model::TEMPO_MG_BASE;

using tuner_target::LN10_OVER_400;
using tuner_target::sigmoid;


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

  // Passed pawn counts, White minus Black, six per position, extracted once for
  // the same reason. Eight pawns per side bounds every bucket, so int16 is
  // again far more room than they need: 1.49 M rows cost 18 MB.
  std::vector<int16_t> passed_pawn;

  // Pawn structure counts, White minus Black, three per position, extracted
  // once for the same reason. Eight pawns per side bounds every one of them,
  // so int16 is again far more room than they need: 1.49 M rows cost 9 MB.
  std::vector<int16_t> pawn_structure;

  // Piece placement counts, White minus Black, four per position, extracted
  // once for the same reason. The bishop pair is 1, 0 or -1 and the three rook
  // counts are bounded by the rooks on the board, so int16 is again far more
  // room than they need: 1.49 M rows cost 12 MB.
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

    const double result = atof(result_text.c_str());
    const int phase = atoi(phase_text.c_str());

    if (result < 0.0 || result > 1.0 || phase < 0 || phase > GAME_PHASE_MAX) {
      fprintf(stderr, "bad row: %s\n", line.c_str());
      return false;
    }

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


// Mean squared error over [first, last), against `target`. Which target that is
// is the caller's choice and it is load bearing: the blended one is what the
// fit descends, the game result alone is what compares two lambdas. DEC-064.
inline double error_range(const dataset_t& data,
                          const double* params,
                          double k,
                          const std::vector<uint32_t>& index,
                          size_t first,
                          size_t last,
                          unsigned threads,
                          const std::vector<double>& target)
{
  if (last <= first) { return 0.0; }

  const size_t count = last - first;
  std::vector<double> partial(threads, 0.0);
  std::vector<std::thread> workers;

  for (unsigned t = 0; t < threads; ++t) {
    workers.emplace_back([&, t]() {
      double sum = 0.0;

      for (size_t i = first + t; i < last; i += threads) {
        const size_t position = index[i];
        const double s = evaluate_position(data, position, params);
        const double diff = target[position] - sigmoid(s, k);
        sum += diff * diff;
      }

      partial[t] = sum;
    });
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  double total = 0.0;
  for (const double value : partial) {
    total += value;
  }

  return total / static_cast<double>(count);
}


// One full-batch gradient over the training range, against the training target.
inline void gradient(const dataset_t& data,
                     const double* params,
                     double k,
                     const std::vector<uint32_t>& index,
                     size_t first,
                     size_t last,
                     unsigned threads,
                     const std::vector<double>& target,
                     std::vector<double>* out)
{
  const size_t count = last - first;
  std::vector<std::vector<double>> partial(
      threads, std::vector<double>(PARAM_COUNT, 0.0));
  std::vector<std::thread> workers;

  for (unsigned t = 0; t < threads; ++t) {
    workers.emplace_back([&, t]() {
      std::vector<double>& grad = partial[t];

      for (size_t i = first + t; i < last; i += threads) {
        const size_t position = index[i];
        const double phase = data.phase[position];
        const double mg_weight = phase / GAME_PHASE_MAX;
        const double eg_weight = (GAME_PHASE_MAX - phase) / GAME_PHASE_MAX;

        const double s = evaluate_position(data, position, params);
        const double sig = sigmoid(s, k);

        // d/ds of (target - sigma)^2, chained through the sigmoid. The target
        // is a constant of the row whatever lambda built it from, so the blend
        // changes this line's number and not its shape. S075.
        const double outer = -2.0 * (target[position] - sig) * sig *
                             (1.0 - sig) * k * LN10_OVER_400;

        for (uint32_t j = data.offsets[position];
             j < data.offsets[position + 1]; ++j) {
          const uint16_t code = data.pieces[j];
          const double sign = (code & BLACK_BIT) ? -1.0 : 1.0;
          const size_t square = code & ~BLACK_BIT;
          const size_t type = square / 64;

          if (type < MATERIAL_COUNT) { grad[type] += outer * sign; }

          grad[MG_BASE + square] += outer * sign * mg_weight;
          grad[EG_BASE + square] += outer * sign * eg_weight;
        }

        // Mobility. The count is a property of the position, so the derivative
        // with respect to a weight is just that count, tapered. Forgetting this
        // block is a silent failure and it happened: the forward model was
        // extended first and a 20-epoch smoke run returned the eight weights
        // exactly as they started, because nothing was pushing them.
        //
        // The clamp in the model is ignored here. It binds essentially never --
        // the term's maximum over 149084 real positions was 143 against a bound
        // of 150 -- and treating the boundary as flat would stop the fit moving
        // a weight it should move.
        for (size_t m = 0; m < MOBILITY_COUNT; ++m) {
          const double count = data.mobility[position * 4 + m];

          grad[MOB_MG_BASE + m] += outer * count * mg_weight;
          grad[MOB_EG_BASE + m] += outer * count * eg_weight;
        }

        // King safety, the same shape and the same trap: the model was extended
        // first and without this block the eighteen weights come back exactly
        // as they were handed in. The clamp is ignored here for the reason
        // above, and it now bounds both terms together.
        for (size_t s = 0; s < KING_SAFETY_COUNT; ++s) {
          const double count =
              data.king_safety[position * KING_SAFETY_COUNT + s];

          grad[KS_MG_BASE + s] += outer * count * mg_weight;
          grad[KS_EG_BASE + s] += outer * count * eg_weight;
        }

        // Passed pawns, the same shape and the same trap a third time. No
        // clamp to ignore here: the term is part of evaluate_cheap(), on the
        // near side of the lazy margin, so the model does not clamp it either.
        for (size_t p = 0; p < PASSED_PAWN_COUNT; ++p) {
          const double count =
              data.passed_pawn[position * PASSED_PAWN_COUNT + p];

          grad[PP_MG_BASE + p] += outer * count * mg_weight;
          grad[PP_EG_BASE + p] += outer * count * eg_weight;
        }

        // Pawn structure, the same shape and the same trap a fourth time, and
        // no clamp to ignore for the same reason: the term is part of
        // evaluate_cheap(), on the near side of the lazy margin.
        for (size_t p = 0; p < PAWN_STRUCTURE_COUNT; ++p) {
          const double count =
              data.pawn_structure[position * PAWN_STRUCTURE_COUNT + p];

          grad[PS_MG_BASE + p] += outer * count * mg_weight;
          grad[PS_EG_BASE + p] += outer * count * eg_weight;
        }

        // Piece placement, the same shape and the same trap a fifth time, and
        // no clamp to ignore for the same reason: the term is part of
        // evaluate_cheap(), on the near side of the lazy margin.
        for (size_t p = 0; p < PIECE_PLACEMENT_COUNT; ++p) {
          const double count =
              data.piece_placement[position * PIECE_PLACEMENT_COUNT + p];

          grad[PL_MG_BASE + p] += outer * count * mg_weight;
          grad[PL_EG_BASE + p] += outer * count * eg_weight;
        }

        // Tempo, the same shape and the same trap a sixth time, and no clamp
        // to ignore for the same reason: the term is part of evaluate_cheap(),
        // on the near side of the lazy margin. No loop, because there is one
        // weight per table and the feature is the position's own +1 or -1.
        const double tempo = data.tempo[position];

        grad[TEMPO_MG_BASE] += outer * tempo * mg_weight;
        grad[TEMPO_EG_BASE] += outer * tempo * eg_weight;
      }
    });
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  out->assign(PARAM_COUNT, 0.0);

  for (const std::vector<double>& part : partial) {
    for (size_t i = 0; i < PARAM_COUNT; ++i) {
      (*out)[i] += part[i];
    }
  }

  for (size_t i = 0; i < PARAM_COUNT; ++i) {
    (*out)[i] /= static_cast<double>(count);
  }
}

}  // namespace tuner_model
