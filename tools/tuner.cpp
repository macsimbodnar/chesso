// Fits chesso's evaluation constants to self-play game outcomes. S028.
//
//   build/tools/tuner --data data.tsv --out tuned_tables.hpp
//
// Reads what tools/datagen writes, `fen result score phase`, and fits the
// constants that evaluate() already uses -- piece_value[0..4], psqt_mg[6][64]
// and psqt_eg[6][64], the mobility weights since S034, the king safety weights
// since S027, the passed pawn weights since S035 and the pawn structure and
// piece placement weights since S027, 825 numbers -- so that a sigmoid of the
// evaluation predicts the outcome of the game the position came from. This is
// Texel tuning; the objective is the mean squared error of that prediction.
//
// It does not run the search. The model below reproduces evaluate() in floating
// point, which is what makes the fit a linear problem and lets a full pass over
// a million positions cost milliseconds instead of a million searches:
//
//   score_white = sum(sign * piece_value[type])
//               + (sum(sign * psqt_mg[type][sq]) * phase
//                  + sum(sign * psqt_eg[type][sq]) * (24 - phase)) / 24
//
// with `sq` mirrored by xor 56 for a black piece and `sign` negative for one,
// exactly as eval_add_piece() does it. The engine divides in integers and
// truncates, so an engine score and a tuner score can differ by a centipawn.
//
// The parameterisation is degenerate on purpose: adding c to every square of
// psqt_mg[t] and psqt_eg[t] is the same evaluation as adding c to
// piece_value[t]. Nothing distinguishes those solutions and nothing needs to --
// what is fitted, and what an SPRT later measures, is the evaluation, not the
// split between the tables.
//
// The agent runs this itself, without asking, and schedules a long fit for the
// night when there is better work to do meanwhile. DEC-041, which supersedes
// DEC-015 for tuning. What still belongs to the owner is the S029 network
// training. Either way the constants come back to be measured by SPRT like any
// other change.
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "eval_model.hpp"

namespace
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

constexpr double LN10_OVER_400 = 2.302585092994046 / 400.0;


struct dataset_t
{
  std::vector<uint16_t> pieces;   // concatenated, indexed by offsets
  std::vector<uint32_t> offsets;  // size = positions + 1
  std::vector<uint8_t> phase;
  std::vector<float> result;

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

  size_t size() const { return phase.size(); }
};


struct options_t
{
  std::string data;
  std::string out = "tuned_tables.hpp";
  std::string only = "all";
  double k = 0.0;  // 0 means fit it
  double lr = 1.0;
  int epochs = 20000;
  int report = 100;
  int patience = 20;
  double validation = 0.1;
  unsigned threads = 3;
  uint64_t seed = 1;
};


const char* const GROUP_LIST =
    "all, material, psqt, mobility, king_safety, passed_pawns, "
    "pawn_structure, piece_placement";


// Marks the parameters a --only group leaves free; the rest keep the value the
// engine ships and come out of the run unchanged.
//
// This exists for attribution under SPRT. Fitting a new term jointly also
// refits the 781 constants that were already fitted, so the match that follows
// measures two changes at once and its number says nothing about either. One
// change at a time; DEC-020 is what that rule cost to learn.
//
// Each group is one contiguous range because eval_model.hpp lays the vector out
// that way.
bool free_mask(const std::string& group, std::vector<uint8_t>* mask)
{
  size_t first = 0;
  size_t last = 0;

  if (group == "all") {
    first = 0;
    last = PARAM_COUNT;
  } else if (group == "material") {
    first = 0;
    last = MATERIAL_COUNT;
  } else if (group == "psqt") {
    first = MG_BASE;
    last = MOB_MG_BASE;
  } else if (group == "mobility") {
    first = MOB_MG_BASE;
    last = KS_MG_BASE;
  } else if (group == "king_safety") {
    // Ends at the passed pawn block, not at PARAM_COUNT. A group that reaches
    // to the end of the vector silently swallows whatever term is appended
    // after it, and the run that follows measures two changes at once.
    first = KS_MG_BASE;
    last = PP_MG_BASE;
  } else if (group == "passed_pawns") {
    // Ends at the pawn structure block for the reason above, and it reached to
    // PARAM_COUNT until that block was appended -- the same bug the comment
    // over king_safety describes, one term later.
    first = PP_MG_BASE;
    last = PS_MG_BASE;
  } else if (group == "pawn_structure") {
    // Ends at the piece placement block for the reason above, and it reached to
    // PARAM_COUNT until that block was appended -- the third time this same
    // function has swallowed the term appended after it.
    first = PS_MG_BASE;
    last = PL_MG_BASE;
  } else if (group == "piece_placement") {
    first = PL_MG_BASE;
    last = PARAM_COUNT;
  } else {
    return false;
  }

  mask->assign(PARAM_COUNT, 0);

  for (size_t i = first; i < last; ++i) {
    (*mask)[i] = 1;
  }

  return true;
}


bool load(const std::string& path, dataset_t* data)
{
  std::ifstream file(path);

  if (!file) { return false; }

  std::string line;
  std::vector<uint16_t> pieces;

  data->offsets.push_back(0);

  while (std::getline(file, line)) {
    if (line.empty()) { continue; }

    std::istringstream fields(line);
    std::string placement, rest, result_text, score_text, phase_text;

    // The FEN's first field is the placement; the rest of the FEN is not
    // needed, but the side to move sits between it and the tab, so the line is
    // split on tabs first and the FEN on spaces after.
    std::string fen;
    if (!std::getline(fields, fen, '\t')) { continue; }
    if (!std::getline(fields, result_text, '\t')) { continue; }
    if (!std::getline(fields, score_text, '\t')) { continue; }
    if (!std::getline(fields, phase_text, '\t')) { continue; }

    std::istringstream fen_fields(fen);
    if (!(fen_fields >> placement)) { continue; }

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
    data->phase.push_back(static_cast<uint8_t>(phase));
    data->result.push_back(static_cast<float>(result));
  }

  return data->size() > 0;
}


double evaluate_position(const dataset_t& data,
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
                              passed, structure, placement);
}


double sigmoid(double score, double k)
{ return 1.0 / (1.0 + std::exp(-k * score * LN10_OVER_400)); }


// Mean squared error over [first, last).
double error_range(const dataset_t& data,
                   const double* params,
                   double k,
                   const std::vector<uint32_t>& index,
                   size_t first,
                   size_t last,
                   unsigned threads)
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
        const double diff = data.result[position] - sigmoid(s, k);
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


// One full-batch gradient over the training range.
void gradient(const dataset_t& data,
              const double* params,
              double k,
              const std::vector<uint32_t>& index,
              size_t first,
              size_t last,
              unsigned threads,
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

        // d/ds of (r - sigma)^2, chained through the sigmoid.
        const double outer = -2.0 * (data.result[position] - sig) * sig *
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


// K is the scale that turns a centipawn score into a win probability, and it
// belongs to the data rather than to the evaluation: fit it once, on the
// starting parameters, and hold it.
double fit_k(const dataset_t& data,
             const double* params,
             const std::vector<uint32_t>& index,
             size_t first,
             size_t last,
             unsigned threads)
{
  double low = 0.1;
  double high = 5.0;

  // Golden section would be fewer evaluations; ternary search is two lines and
  // this runs once.
  for (int step = 0; step < 40; ++step) {
    const double a = low + (high - low) / 3.0;
    const double b = high - (high - low) / 3.0;

    const double ea = error_range(data, params, a, index, first, last, threads);
    const double eb = error_range(data, params, b, index, first, last, threads);

    if (ea < eb) {
      high = b;
    } else {
      low = a;
    }
  }

  return (low + high) / 2.0;
}


void write_tables(const std::string& path,
                  const double* params,
                  const options_t& opts,
                  size_t positions,
                  double train_error,
                  double validation_error)
{
  FILE* out = fopen(path.c_str(), "w");

  if (out == nullptr) {
    fprintf(stderr, "cannot write %s\n", path.c_str());
    return;
  }

  const char* names[6] = {"pawn", "knight", "bishop", "rook", "queen", "king"};

  // king_safety_feature_t's enumerators, in its order, so the emitted rows can
  // be read against the enum they index.
  const char* king_safety_names[KING_SAFETY_COUNT] = {
      "KS_KNIGHT_ATTACKERS", "KS_BISHOP_ATTACKERS", "KS_ROOK_ATTACKERS",
      "KS_QUEEN_ATTACKERS",  "KS_ZONE_ATTACKS",     "KS_SHIELD_NEAR",
      "KS_SHIELD_FAR",       "KS_OPEN_FILE",        "KS_HALF_OPEN_FILE"};

  fprintf(
      out,
      "// Fitted by tools/tuner from %zu self-play positions.\n"
      "// data       %s\n"
      "// K          %.4f\n"
      "// error      %.6f train, %.6f validation\n"
      "// seed       %" PRIu64
      ", lr %.3f, validation split %.2f\n"
      "// only       %s\n"
      "//\n"
      "// Paste over the corresponding definitions. The piece defines and the\n"
      "// two tables live in src/eval_tables.hpp; the mobility, king safety,\n"
      "// passed pawn, pawn structure and piece placement weights at the end\n"
      "// live in src/evaluation.cpp, a different file and easy to miss.\n"
      "// S027, S028, S034 and S035, DEC-015: measured by SPRT before any of\n"
      "// it is kept.\n\n",
      positions, opts.data.c_str(), opts.k, train_error, validation_error,
      opts.seed, opts.lr, opts.validation, opts.only.c_str());

  const char* material_names[5] = {"PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN"};

  for (size_t i = 0; i < MATERIAL_COUNT; ++i) {
    fprintf(out, "#define %-7s %d\n", material_names[i],
            static_cast<int>(std::lround(params[i])));
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? MG_BASE : EG_BASE;

    fprintf(out, "\nstatic constexpr int psqt_%s[6][64] = {\n",
            (table == 0) ? "mg" : "eg");

    for (size_t type = 0; type < 6; ++type) {
      fprintf(out, "  {  // %s\n", names[type]);

      for (int rank = 0; rank < 8; ++rank) {
        fprintf(out, "   ");

        for (int file = 0; file < 8; ++file) {
          const size_t square = type * 64 + rank * 8 + file;
          fprintf(out, " %4d%s",
                  static_cast<int>(std::lround(params[base + square])),
                  (rank == 7 && file == 7) ? "" : ",");
        }

        fprintf(out, "\n");
      }

      fprintf(out, "  }%s\n", (type == 5) ? "" : ",");
    }

    fprintf(out, "};\n");
  }

  // The mobility weights are fitted too, since S034, the king safety weights
  // since S027, the passed pawn weights since S035 and the pawn structure and
  // piece placement weights since S027, and none of them lives in
  // eval_tables.hpp with everything else. Emitting them here is what stops a
  // fit quietly discarding fifty-two of its own parameters, which is exactly
  // what this function did with mobility's eight until it was checked.
  fprintf(
      out,
      "\n// These eight live in src/evaluation.cpp, not eval_tables.hpp.\n");

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? MOB_MG_BASE : MOB_EG_BASE;

    fprintf(out, "const int mobility_%s[4] = {", (table == 0) ? "mg" : "eg");

    for (size_t i = 0; i < MOBILITY_COUNT; ++i) {
      fprintf(out, "%s%d", (i == 0) ? "" : ", ",
              static_cast<int>(std::lround(params[base + i])));
    }

    fprintf(out, "};  // knight bishop rook queen\n");
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? KS_MG_BASE : KS_EG_BASE;

    fprintf(out, "\nconst int king_safety_%s[KS_FEATURE_COUNT] = {\n",
            (table == 0) ? "mg" : "eg");

    // One per line with its enumerator, because nine numbers on one line is a
    // row nobody can check against the enum it is indexed by.
    for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
      fprintf(out, "    %d,  // %s\n",
              static_cast<int>(std::lround(params[base + i])),
              king_safety_names[i]);
    }

    fprintf(out, "};\n");
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? PP_MG_BASE : PP_EG_BASE;

    fprintf(out, "\nconst int passed_pawn_%s[6] = {",
            (table == 0) ? "mg" : "eg");

    for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
      fprintf(out, "%s%d", (i == 0) ? "" : ", ",
              static_cast<int>(std::lround(params[base + i])));
    }

    // The index is how far the pawn has come, not which rank it stands on, so
    // the trailing comment names the buckets from the pawn's own point of view.
    fprintf(out, "};  // second rank .. seventh\n");
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? PS_MG_BASE : PS_EG_BASE;

    fprintf(out, "\nconst int pawn_structure_%s[3] = {",
            (table == 0) ? "mg" : "eg");

    for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
      fprintf(out, "%s%d", (i == 0) ? "" : ", ",
              static_cast<int>(std::lround(params[base + i])));
    }

    fprintf(out, "};  // isolated doubled backward\n");
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? PL_MG_BASE : PL_EG_BASE;

    fprintf(out, "\nconst int piece_placement_%s[4] = {",
            (table == 0) ? "mg" : "eg");

    for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
      fprintf(out, "%s%d", (i == 0) ? "" : ", ",
              static_cast<int>(std::lround(params[base + i])));
    }

    fprintf(out, "};  // pair, open, half open, seventh\n");
  }

  fclose(out);
  fprintf(stderr, "constants written to %s\n", path.c_str());
}


void usage()
{
  fprintf(stderr,
          "tuner --data FILE [options]\n"
          "  --out FILE       where the fitted constants are written\n"
          "  --only GROUP     fit only this group and hold the rest at what\n"
          "                   the engine ships: %s\n"
          "  --k VALUE        sigmoid scale; 0 fits it from the data\n"
          "  --lr VALUE       Adam step size (default 1.0)\n"
          "  --epochs N       maximum full-batch steps (default 20000)\n"
          "  --report N       report and checkpoint every N epochs (100)\n"
          "  --patience N     stop after this many reports without a new best\n"
          "  --validation F   held-out fraction (default 0.1)\n"
          "  --threads N      worker threads (default 3)\n"
          "  --seed N         shuffle seed for the split (default 1)\n",
          GROUP_LIST);
}

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "--help") {
      usage();
      return 0;
    }

    if (i + 1 >= argc) {
      usage();
      return 1;
    }

    const std::string value = argv[++i];

    if (arg == "--data") {
      opts.data = value;
    } else if (arg == "--out") {
      opts.out = value;
    } else if (arg == "--only") {
      opts.only = value;
    } else if (arg == "--k") {
      opts.k = atof(value.c_str());
    } else if (arg == "--lr") {
      opts.lr = atof(value.c_str());
    } else if (arg == "--epochs") {
      opts.epochs = atoi(value.c_str());
    } else if (arg == "--report") {
      opts.report = atoi(value.c_str());
    } else if (arg == "--patience") {
      opts.patience = atoi(value.c_str());
    } else if (arg == "--validation") {
      opts.validation = atof(value.c_str());
    } else if (arg == "--threads") {
      opts.threads = static_cast<unsigned>(atoi(value.c_str()));
    } else if (arg == "--seed") {
      opts.seed = strtoull(value.c_str(), nullptr, 10);
    } else {
      usage();
      return 1;
    }
  }

  if (opts.data.empty() || opts.threads == 0) {
    usage();
    return 1;
  }

  std::vector<uint8_t> mask;

  if (!free_mask(opts.only, &mask)) {
    fprintf(stderr, "unknown --only group '%s'; groups are: %s\n",
            opts.only.c_str(), GROUP_LIST);
    return 1;
  }

  size_t free_count = 0;
  for (const uint8_t value : mask) {
    free_count += value;
  }

  dataset_t data;

  if (!load(opts.data, &data)) {
    fprintf(stderr, "could not read %s\n", opts.data.c_str());
    return 1;
  }

  // Positions from one game are consecutive and share a label, so a split that
  // cuts the file in two puts whole games on one side and correlates the
  // validation set with nothing. Shuffling first is what makes the held-out
  // error mean something.
  std::vector<uint32_t> index(data.size());
  for (size_t i = 0; i < index.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
  }

  std::mt19937_64 rng(opts.seed);
  std::shuffle(index.begin(), index.end(), rng);

  const size_t validation_count =
      static_cast<size_t>(static_cast<double>(data.size()) * opts.validation);
  const size_t train_count = data.size() - validation_count;

  fprintf(stderr,
          "%zu positions, %zu train, %zu validation, %zu parameters, "
          "group %s, %zu free\n",
          data.size(), train_count, validation_count, PARAM_COUNT,
          opts.only.c_str(), free_count);

  // Starting point is what the engine ships with, so a fit that finds nothing
  // reports the error of the hand-written constants rather than of noise.
  std::vector<double> params(PARAM_COUNT, 0.0);
  eval_model::starting_params(params.data());

  if (opts.k <= 0.0) {
    opts.k = fit_k(data, params.data(), index, 0, train_count, opts.threads);
    fprintf(stderr, "fitted K = %.4f\n", opts.k);
  }

  const double start_train = error_range(data, params.data(), opts.k, index, 0,
                                         train_count, opts.threads);
  const double start_validation =
      error_range(data, params.data(), opts.k, index, train_count, data.size(),
                  opts.threads);

  fprintf(stderr, "start: train %.6f  validation %.6f\n", start_train,
          start_validation);

  // Adam. Plain gradient descent needs a step size per parameter here: a pawn
  // square that appears in every position and a king square that appears in a
  // handful do not want the same one.
  std::vector<double> moment(PARAM_COUNT, 0.0);
  std::vector<double> velocity(PARAM_COUNT, 0.0);
  std::vector<double> grad;

  const double beta1 = 0.9;
  const double beta2 = 0.999;
  const double epsilon = 1e-8;

  std::vector<double> best = params;
  double best_validation = start_validation;
  int since_best = 0;

  for (int epoch = 1; epoch <= opts.epochs; ++epoch) {
    gradient(data, params.data(), opts.k, index, 0, train_count, opts.threads,
             &grad);

    const double correction1 = 1.0 - std::pow(beta1, epoch);
    const double correction2 = 1.0 - std::pow(beta2, epoch);

    for (size_t i = 0; i < PARAM_COUNT; ++i) {
      // A frozen parameter is left out of the update entirely rather than
      // having its gradient zeroed, because a zero gradient does not mean a
      // zero step: Adam divides a decaying moment by a decaying velocity and
      // keeps moving on what it accumulated before. Skipping the whole
      // iteration leaves moment and velocity at zero too, so the parameter
      // comes out bit-identical to what it went in as and the emitted file is
      // one change.
      if (!mask[i]) { continue; }

      moment[i] = beta1 * moment[i] + (1.0 - beta1) * grad[i];
      velocity[i] = beta2 * velocity[i] + (1.0 - beta2) * grad[i] * grad[i];

      const double m = moment[i] / correction1;
      const double v = velocity[i] / correction2;

      params[i] -= opts.lr * m / (std::sqrt(v) + epsilon);
    }

    if (epoch % opts.report != 0) { continue; }

    const double train = error_range(data, params.data(), opts.k, index, 0,
                                     train_count, opts.threads);
    const double validation =
        error_range(data, params.data(), opts.k, index, train_count,
                    data.size(), opts.threads);

    fprintf(stderr, "epoch %6d  train %.6f  validation %.6f%s\n", epoch, train,
            validation, (validation < best_validation) ? "  *" : "");

    if (validation < best_validation) {
      best_validation = validation;
      best = params;
      since_best = 0;
    } else if (++since_best >= opts.patience) {
      fprintf(stderr, "no improvement in %d reports, stopping\n", since_best);
      break;
    }
  }

  const double final_train = error_range(data, best.data(), opts.k, index, 0,
                                         train_count, opts.threads);

  fprintf(stderr, "best: train %.6f  validation %.6f  (start %.6f / %.6f)\n",
          final_train, best_validation, start_train, start_validation);

  if (best_validation >= start_validation) {
    fprintf(stderr,
            "WARNING: held-out error did not improve on the hand-written "
            "constants. Do not ship this.\n");
  }

  write_tables(opts.out, best.data(), opts, data.size(), final_train,
               best_validation);

  return 0;
}
