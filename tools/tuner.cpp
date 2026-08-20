// Fits chesso's evaluation constants to self-play game outcomes. S028.
//
//   build/tools/tuner --data data.tsv --out tuned_tables.hpp
//
// Reads what tools/datagen writes, `fen result score phase`, and fits the
// constants that evaluate() already uses -- piece_value[0..4], psqt_mg[6][64]
// and psqt_eg[6][64], the mobility weights since S034, the king safety weights
// since S027, the passed pawn weights since S035 and the pawn structure, piece
// placement and tempo weights since S027, 827 numbers -- so that a sigmoid of
// the evaluation predicts the outcome of the game the position came from.
// This is Texel tuning; the objective is the mean squared error of that
// prediction.
//
// Since S075 the label is a blend rather than the outcome alone,
//
//   target = lambda * sigma(K * score) + (1 - lambda) * result
//
// with `--lambda 0`, the default, the pure game outcome this fitted before.
// `tools/tuner_target.hpp` holds the target and the two rules that come with
// it: K is fitted against the game result and held, and held-out error against
// the game result -- not against the training objective -- is what compares one
// lambda with another. DEC-064.
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
#include "chesso_build_info.hpp"
#include "corpus_hash.hpp"
#include "eval_model.hpp"
#include "tuner_groups.hpp"
#include "tuner_model.hpp"
#include "tuner_split.hpp"
#include "tuner_target.hpp"

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
using eval_model::TEMPO_EG_BASE;
using eval_model::TEMPO_MG_BASE;

// Both moved to tools/tuner_groups.hpp at S041 so that
// tests/test_tuner_groups.cpp can reach them. Imported under their old names so
// the call sites below are unchanged.
using tuner_groups::free_mask;
using tuner_groups::freeze_mask;
using tuner_groups::GROUP_LIST;

// Moved into tools/tuner_target.hpp at S075 so that the sigmoid the fit is
// scored through and the sigmoid the blended label is built from are one
// function. Imported under their old names so the call sites below are
// unchanged.
using tuner_target::LN10_OVER_400;
using tuner_target::sigmoid;


// The dataset, load(), the forward model over one row, error_range() and the
// full-batch gradient moved to tools/tuner_model.hpp at S100 so that a test and
// tools/feature_audit.cpp can reach them -- the gradient in particular was the
// one piece of the fit nothing outside this file could call, and its own
// comments record three times a term block was forgotten there. Imported under
// their old names so the call sites below are unchanged.
using tuner_model::dataset_t;
using tuner_model::error_range;
using tuner_model::evaluate_position;
using tuner_model::gradient;
using tuner_model::load;


// Every hardware thread the machine has. DEC-050 sets the policy at all of
// them, SMT included; asking the machine rather than writing the number keeps
// it true on the next one. hardware_concurrency() may answer 0 when it cannot
// tell.
unsigned default_threads()
{
  const unsigned reported = std::thread::hardware_concurrency();
  return (reported > 0) ? reported : 1;
}


// What the corpus was, rather than where it was read from. S077: a path has
// already meant two different files here, and both S082 and S083 write another
// one. Filled once, before the fit, and printed into the emitted header.
struct corpus_id_t
{
  std::string sha256;
  uint64_t bytes = 0;
  uint64_t rows = 0;
};


struct options_t
{
  std::string data;
  std::string out = "tuned_tables.hpp";
  std::string only = "all";
  std::string freeze;  // empty means nothing is held, the behaviour before S065
  double k = 0.0;      // 0 means fit it
  double lambda = 0.0;  // 0 is the game outcome alone, the fit before S075
  double lr = 1.0;
  int epochs = 20000;
  int report = 100;
  int patience = 20;
  double validation = 0.1;
  unsigned threads = default_threads();
  uint64_t seed = 1;
};


// K is the scale that turns a centipawn score into a win probability, and it
// belongs to the data rather than to the evaluation: fit it once, on the
// starting parameters, and hold it.
//
// `target` is the game result and never the blend, whatever --lambda says. The
// blend is built *from* K, so fitting K against it would have K chasing itself,
// and no two lambda runs would share a scale to be compared on. DEC-064.
double fit_k(const dataset_t& data,
             const double* params,
             const std::vector<uint32_t>& index,
             size_t first,
             size_t last,
             unsigned threads,
             const std::vector<double>& target)
{
  double low = 0.1;
  double high = 5.0;

  // Golden section would be fewer evaluations; ternary search is two lines and
  // this runs once.
  for (int step = 0; step < 40; ++step) {
    const double a = low + (high - low) / 3.0;
    const double b = high - (high - low) / 3.0;

    const double ea =
        error_range(data, params, a, index, first, last, threads, target);
    const double eb =
        error_range(data, params, b, index, first, last, threads, target);

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
                  const corpus_id_t& corpus,
                  size_t positions,
                  size_t games,
                  size_t validation_rows,
                  double train_error,
                  double validation_error,
                  double wdl_validation_error,
                  double start_wdl_validation_error)
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

  // The number that compares this fit with a fit at another lambda, and the
  // number it had to beat to be worth a match at all: the held-out error
  // against the game result, at these constants and at the ones the run started
  // from. Written only at a non-zero lambda, where it is a second number -- at
  // lambda 0 the training objective is the game result and the `error` line
  // above already reports it. DEC-064.
  std::string wdl_line;

  if (opts.lambda > 0.0) {
    char text[256];
    snprintf(
        text, sizeof text,
        "// wdl error  %.6f validation, against %.6f at the constants this "
        "started\n"
        "//            from. This selects lambda; loss is not Elo and an "
        "SPRT decides.\n",
        wdl_validation_error, start_wdl_validation_error);
    wdl_line = text;
  }

  fprintf(
      out,
      "// Fitted by tools/tuner from %zu self-play positions.\n"
      "// engine     %s, the commit this tuner was built from\n"
      "// data       %s\n"
      "// corpus     sha256 %s\n"
      "//            %" PRIu64 " rows, %" PRIu64
      " bytes; `sha256sum` over the same file prints the same\n"
      "// K          %.4f\n"
      "// error      %.6f train, %.6f validation\n"
      "// seed       %" PRIu64
      ", lr %.3f, validation split %.2f\n"
      "// split      by game, S066: %zu games, %zu rows held out, %.4f%%\n"
      "// only       %s\n"
      "// freeze     %s\n"
      "// lambda     %.4f  target = lambda * sigma(K * score) + (1 - lambda) * "
      "result\n"
      "%s"
      "//\n"
      "// Paste over the corresponding definitions. The piece defines and the\n"
      "// two tables live in src/eval_tables.hpp; the mobility, king safety,\n"
      "// passed pawn, pawn structure, piece placement and tempo weights at\n"
      "// the end live in src/evaluation.cpp, a different file and easy to\n"
      "// miss.\n"
      "// S027, S028, S034 and S035, DEC-015: measured by SPRT before any of\n"
      "// it is kept.\n\n",
      positions, CHESSO_BUILD_COMMIT, opts.data.c_str(), corpus.sha256.c_str(),
      corpus.rows, corpus.bytes, opts.k, train_error, validation_error,
      opts.seed, opts.lr, opts.validation, games, validation_rows,
      100.0 * static_cast<double>(validation_rows) /
          static_cast<double>(positions),
      opts.only.c_str(),
      opts.freeze.empty() ? "(nothing)" : opts.freeze.c_str(), opts.lambda,
      wdl_line.c_str());

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
  // since S027, the passed pawn weights since S035 and the pawn structure,
  // piece placement and tempo weights since S027, and none of them lives in
  // eval_tables.hpp with everything else. Emitting them here is what stops a
  // fit quietly discarding fifty-four of its own parameters, which is exactly
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

  // One number per table and no array, so these two lines are the whole term.
  // Emitted here for the reason the eight above are: a parameter the fit moves
  // and the writer forgets is a parameter the run silently discards.
  fprintf(out, "\nconst int tempo_mg = %d;\nconst int tempo_eg = %d;\n",
          static_cast<int>(std::lround(params[TEMPO_MG_BASE])),
          static_cast<int>(std::lround(params[TEMPO_EG_BASE])));

  fclose(out);
  fprintf(stderr, "constants written to %s\n", path.c_str());
}


void usage()
{
  fprintf(
      stderr,
      "tuner --data FILE [options]\n"
      "  --out FILE       where the fitted constants are written\n"
      "  --only GROUP     fit only this group and hold the rest at what\n"
      "                   the engine ships: %s\n"
      "  --freeze LIST    hold these groups instead, comma separated, and\n"
      "                   fit everything else; not `all`. Applied after\n"
      "                   --only, so the two intersect\n"
      "  --k VALUE        sigmoid scale; 0 fits it from the data, against\n"
      "                   the game result whatever --lambda says\n"
      "  --lambda F       fit against F * sigma(K * score) + (1 - F) *\n"
      "                   result, F in [0, 1]. 0, the default, is the game\n"
      "                   outcome alone and is the fit before S075. Two\n"
      "                   lambdas are compared on held-out error against\n"
      "                   the game result, never on the training error\n"
      "  --lr VALUE       Adam step size (default 1.0)\n"
      "  --epochs N       maximum full-batch steps (default 20000)\n"
      "  --report N       report and checkpoint every N epochs (100)\n"
      "  --patience N     stop after this many reports without a new best\n"
      "  --validation F   held-out fraction, whole games (default 0.1)\n"
      "  --threads N      worker threads (default: every hardware thread)\n"
      "  --seed N         seed the game-level split shuffles (default 1)\n",
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
    } else if (arg == "--freeze") {
      opts.freeze = value;
    } else if (arg == "--k") {
      opts.k = atof(value.c_str());
    } else if (arg == "--lambda") {
      opts.lambda = atof(value.c_str());
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

  // Refused rather than clamped: a lambda outside [0, 1] is not a blend of two
  // labels, and `atof` answers 0.0 for a value it cannot read -- which is the
  // pure game outcome and would look like a run that worked. S075.
  if (!tuner_target::valid_lambda(opts.lambda)) {
    fprintf(stderr, "--lambda must be in [0, 1]\n");
    return 1;
  }

  std::vector<uint8_t> mask;

  if (!free_mask(opts.only, &mask)) {
    fprintf(stderr, "unknown --only group '%s'; groups are: %s\n",
            opts.only.c_str(), GROUP_LIST);
    return 1;
  }

  // --freeze applies after --only, so the two compose: --only psqt --freeze
  // material frees the psqt block and nothing else, since material was already
  // held. `all` is refused, as is any name free_mask does not know. S065.
  if (!freeze_mask(opts.freeze, &mask)) {
    fprintf(stderr,
            "unusable --freeze list '%s'; it takes a comma-separated subset of "
            "%s, and not `all`\n",
            opts.freeze.c_str(), GROUP_LIST);
    return 1;
  }

  size_t free_count = 0;
  for (const uint8_t value : mask) {
    free_count += value;
  }

  // A --freeze that frees nothing would run every epoch and emit the constants
  // it was handed. free_mask's own groups are non-empty, so this can only be
  // reached by a --freeze that covers the --only group.
  if (free_count == 0) {
    fprintf(stderr,
            "--only %s and --freeze %s leave no parameter free; nothing would "
            "be fitted\n",
            opts.only.c_str(), opts.freeze.c_str());
    return 1;
  }

  // Before the fit, and refused rather than left blank: a table stamped with a
  // corpus nobody could hash names a file that may not have been readable, and
  // an empty provenance line reads as "no corpus" rather than as "not
  // measured". S077.
  corpus_id_t corpus;

  if (!corpus_hash::hash_file(opts.data, &corpus.sha256, &corpus.bytes,
                              &corpus.rows)) {
    fprintf(stderr, "could not hash %s\n", opts.data.c_str());
    return 1;
  }

  fprintf(stderr,
          "engine %s, corpus sha256 %s, %" PRIu64 " rows, %" PRIu64 " bytes\n",
          CHESSO_BUILD_COMMIT, corpus.sha256.c_str(), corpus.rows,
          corpus.bytes);

  dataset_t data;

  if (!load(opts.data, &data)) {
    fprintf(stderr, "could not read %s\n", opts.data.c_str());
    return 1;
  }

  // Positions from one game are consecutive and share a label, so a row-level
  // split holds out rows whose game is in the training set: 99.47 % of games
  // landed on both sides of the cut and the held-out error was measuring
  // memorisation. Whole games are held out instead, and tools/tuner_split.hpp
  // is where the boundary between them comes from. S066.
  std::vector<uint32_t> starts;
  tuner_split::game_starts(data.ply, &starts);

  std::vector<uint32_t> index;
  const size_t train_count = tuner_split::split(
      starts, data.size(), opts.validation, opts.seed, &index);
  const size_t validation_count = data.size() - train_count;

  fprintf(stderr,
          "%zu positions, %zu games, %zu train, %zu validation (%.4f%%), "
          "%zu parameters, group %s, freeze %s, %zu free\n",
          data.size(), starts.size(), train_count, validation_count,
          100.0 * static_cast<double>(validation_count) /
              static_cast<double>(data.size()),
          PARAM_COUNT, opts.only.c_str(),
          opts.freeze.empty() ? "(nothing)" : opts.freeze.c_str(), free_count);

  // A held-out set of nothing reports an error of 0.000000, which reads as a
  // perfect fit rather than as an empty set. It happens when the corpus is one
  // game: a block is indivisible and the last one is never held out, so there
  // is nothing to give. Say it here rather than leaving the zero to be read.
  if (validation_count == 0 && opts.validation > 0.0) {
    fprintf(stderr,
            "WARNING: %zu game(s) in this corpus, so nothing could be held "
            "out. Every validation figure below is over an empty set.\n",
            starts.size());
  }

  // Starting point is what the engine ships with, so a fit that finds nothing
  // reports the error of the hand-written constants rather than of noise.
  std::vector<double> params(PARAM_COUNT, 0.0);
  eval_model::starting_params(params.data());

  // The game result, as a target vector. K is fitted against this and never
  // against the blend, and every comparison between two lambdas is made on it.
  // DEC-064. 11.0 M rows cost 88 MB.
  std::vector<double> wdl_target(data.size());

  for (size_t i = 0; i < data.size(); ++i) {
    wdl_target[i] = data.result[i];
  }

  if (opts.k <= 0.0) {
    opts.k = fit_k(data, params.data(), index, 0, train_count, opts.threads,
                   wdl_target);
    fprintf(stderr, "fitted K = %.4f\n", opts.k);
  }

  // What the fit descends. At lambda 0 it is the game result and nothing else,
  // by this copy rather than by arithmetic that happens to be exact -- blend()
  // is exact there too, and the branch is what saves 11 M calls to exp().
  std::vector<double> train_target = wdl_target;

  if (opts.lambda > 0.0) {
    for (size_t i = 0; i < data.size(); ++i) {
      train_target[i] = tuner_target::blend(opts.lambda, data.score[i],
                                            data.result[i], opts.k);
    }
  }

  const double start_train =
      error_range(data, params.data(), opts.k, index, 0, train_count,
                  opts.threads, train_target);
  const double start_validation =
      error_range(data, params.data(), opts.k, index, train_count, data.size(),
                  opts.threads, train_target);

  // The selection metric at the constants the run started from. At lambda 0 the
  // two targets are the same vector, so this is the same number and the same
  // pass is not made twice.
  const double start_wdl =
      (opts.lambda > 0.0)
          ? error_range(data, params.data(), opts.k, index, train_count,
                        data.size(), opts.threads, wdl_target)
          : start_validation;

  // A `wdl` column only where there is a second number to print. At lambda 0 it
  // would repeat `validation`, and every line this tuner has ever printed would
  // have changed for nothing.
  const auto wdl_column = [&](double value) {
    if (opts.lambda <= 0.0) { return std::string(); }

    char text[32];
    snprintf(text, sizeof text, "  wdl %.6f", value);
    return std::string(text);
  };

  fprintf(stderr, "start: train %.6f  validation %.6f%s\n", start_train,
          start_validation, wdl_column(start_wdl).c_str());

  // Adam. Plain gradient descent needs a step size per parameter here: a pawn
  // square that appears in every position and a king square that appears in a
  // handful do not want the same one.
  std::vector<double> moment(PARAM_COUNT, 0.0);
  std::vector<double> velocity(PARAM_COUNT, 0.0);
  std::vector<double> grad;

  const double beta1 = 0.9;
  const double beta2 = 0.999;
  const double epsilon = 1e-8;

  // The checkpoint kept, and the early stop, are both decided on the held-out
  // error against the **game result** rather than against the training target.
  // That is DEC-064 applied inside a run as well as across runs: the vector
  // this emits is the best outcome predictor the trajectory passed through, so
  // a lambda is never rejected because its own target's optimum sat a few
  // hundred epochs from the game result's. At lambda 0 the two are the same
  // number and this is the selection the tuner has always made.
  std::vector<double> best = params;
  double best_validation = start_validation;
  double best_wdl = start_wdl;
  int since_best = 0;

  for (int epoch = 1; epoch <= opts.epochs; ++epoch) {
    gradient(data, params.data(), opts.k, index, 0, train_count, opts.threads,
             train_target, &grad);

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
                                     train_count, opts.threads, train_target);
    const double validation =
        error_range(data, params.data(), opts.k, index, train_count,
                    data.size(), opts.threads, train_target);
    const double wdl =
        (opts.lambda > 0.0)
            ? error_range(data, params.data(), opts.k, index, train_count,
                          data.size(), opts.threads, wdl_target)
            : validation;

    fprintf(stderr, "epoch %6d  train %.6f  validation %.6f%s%s\n", epoch,
            train, validation, wdl_column(wdl).c_str(),
            (wdl < best_wdl) ? "  *" : "");

    if (wdl < best_wdl) {
      best_wdl = wdl;
      best_validation = validation;
      best = params;
      since_best = 0;
    } else if (++since_best >= opts.patience) {
      fprintf(stderr, "no improvement in %d reports, stopping\n", since_best);
      break;
    }
  }

  const double final_train =
      error_range(data, best.data(), opts.k, index, 0, train_count,
                  opts.threads, train_target);

  fprintf(stderr,
          "best: train %.6f  validation %.6f%s  (start %.6f / %.6f%s)\n",
          final_train, best_validation, wdl_column(best_wdl).c_str(),
          start_train, start_validation, wdl_column(start_wdl).c_str());

  // Against the selection metric, so a non-zero lambda is refused for failing
  // to predict outcomes better than the constants it started from -- not for
  // failing to predict its own target, which it always will.
  if (best_wdl >= start_wdl) {
    fprintf(stderr,
            "WARNING: held-out error did not improve on the hand-written "
            "constants. Do not ship this.\n");
  }

  write_tables(opts.out, best.data(), opts, corpus, data.size(), starts.size(),
               validation_count, final_train, best_validation, best_wdl,
               start_wdl);

  return 0;
}
