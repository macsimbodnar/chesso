// Fits chesso's evaluation constants to self-play game outcomes. S028.
//
//   build/tools/tuner --data data.tsv --out tuned_tables.hpp
//
// Reads what tools/datagen writes, `fen result score phase`, and fits the
// constants that evaluate() already uses -- piece_value[0..4], psqt_mg[6][64]
// and psqt_eg[6][64], the mobility weights since S034, the king safety weights
// since S027, the passed pawn weights since S035 and the pawn structure, piece
// placement and tempo weights since S027, 823 numbers from S134, 831 since
// S135 widened piece placement and 955 since S121 made mobility a table of 66
// cells a half -- so that a sigmoid of the evaluation predicts the outcome of
// the game the position came from.
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
// It does not run the search. Since S262 it does not re-implement the
// evaluation either: it links the engine's trace build, runs evaluate() once on
// every row, and keeps the coefficient of every weight evaluate() applied
// (tools/tuner_trace.hpp, src/eval_trace.hpp). The score it fits is a sum over
// those records, which is what keeps a full pass over ten million positions
// under a second instead of ten million searches:
//
//   score_white = sum(coef * piece_value) + sum over each taper of
//                 (sum(coef * w_mg) * phase + sum(coef * w_eg) * (24 - phase))
//                 / 24, mobility plus king safety clamped to the lazy margin
//
// Every row is also scored the engine's way -- integer weights, truncating
// tapers -- and the run refuses a corpus where that differs from evaluate() on
// a single row. The fit itself descends the untruncated form, which differs
// from the engine by under a centipawn per taper that rounds.
//
// The parameterisation is degenerate on purpose: adding c to every square of
// psqt_mg[t] and psqt_eg[t] is the same evaluation as adding c to
// piece_value[t], and adding c to every square of a king table is no change at
// all, one king a side. What is fitted, and what an SPRT later measures, is the
// evaluation, not the split. Since S275 the fit holds the split anyway --
// because Adam walked along those seven directions for nothing, +16 a square
// on each king table over one 1 M-row slice, and left two fits' tables
// incomparable. What is held is a sum at its starting value: for a piece, the
// sum of its psqt_mg and psqt_eg squares together, with its value taking the
// shift; for a king table, that table's own sum. A direction is held only when
// --only and --freeze leave all of it free, so under --only psqt or --freeze
// material only the two king sums are held and the tables carry what the
// piece values cannot. tools/tuner_groups.hpp has the seven and the hold.
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
#include "tuner_groups.hpp"
#include "tuner_layout.hpp"
#include "tuner_optimiser.hpp"
#include "tuner_split.hpp"
#include "tuner_target.hpp"
#include "tuner_trace.hpp"

namespace
{

using tuner_layout::EG_BASE;
using tuner_layout::KING_SAFETY_COUNT;
using tuner_layout::KS_EG_BASE;
using tuner_layout::KS_MG_BASE;
using tuner_layout::MATERIAL_COUNT;
using tuner_layout::MG_BASE;
using tuner_layout::MOB_EG_BASE;
using tuner_layout::MOB_MG_BASE;
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
using tuner_layout::TEMPO_EG_BASE;
using tuner_layout::TEMPO_MG_BASE;

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
using tuner_target::sigmoid;


// The dataset, its load, the score over one row, error_range() and the
// full-batch gradient: tools/tuner_trace.hpp since S262, read off the engine's
// own trace (tools/tuner_model.hpp from S100, over the hand-written mirror in
// tools/eval_model.hpp). The step, the optimiser and the gauge hold: in
// tools/tuner_optimiser.hpp. Imported under their old names so the call sites
// below are unchanged.
using tuner_optimiser::descend;
using tuner_trace::dataset_t;
using tuner_trace::error_range;
using tuner_trace::gradient;


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
  std::string tie;     // S121: a tie list's path; empty ties nothing
  double k = 0.0;      // 0 means fit it
  double lambda = 0.0;  // 0 is the game outcome alone, the fit before S075
  tuner_optimiser::kind_t optimiser = tuner_optimiser::ADAM;
  double lr = 0.0;  // the optimiser's default unless --lr names one
  bool lr_given = false;
  int epochs = 20000;
  int report = 100;
  int patience = 20;
  double validation = 0.1;
  // S082: the fixed validation set. Empty is the by-game split of the corpus
  // that `validation` and `seed` describe.
  std::string holdout;
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
                  const std::string& split,
                  double train_error,
                  double validation_error,
                  double wdl_validation_error,
                  double start_wdl_validation_error,
                  size_t gauges_held,
                  const tuner_trace::load_report_t& trace,
                  const std::string& tie_line)
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
      // S262: the model is the engine's trace, and the identity it rests on
      // was checked on every row this read; a run with a mismatch refuses
      // before it gets here, so the count is the evidence and not a warning.
      "// model      evaluate()'s trace, %" PRIu64 " rows; %" PRIu64
      " differ from evaluate() at the shipped weights\n"
      "// optimiser  %s\n"
      // S214, 2026-09-10_adversarial-F35: --report and --patience pick which
      // vector comes out of a run -- the best of the epochs that reported, and
      // the run stops after --patience reports without a new best -- so a fit
      // was not reproducible from its own stamp without them. --epochs is the
      // other way a run ends. --threads is here because error_range() and
      // gradient() stride by it and sum the partials in thread order, so the
      // summation order is a function of it; the rounded constants absorbed
      // that on every fit measured so far, which is not the same as neutral.
      "// run        %d epochs max, report every %d, patience %d reports, %u "
      "threads\n"
      "//            the emitted vector is the best reported epoch, so "
      "--report "
      "and\n"
      "//            --patience choose it as much as the data does\n"
      "// split      %s\n"
      "// only       %s\n"
      "// freeze     %s\n"
      // S275: which of the seven null directions kept the level the run started
      // from. The split between a piece's value and its tables is the gauge's,
      // not the data's, so a table emitted without it is not comparable.
      "// gauges     %zu of %zu held at the levels this started from\n"
      // S121: which cells shared a parameter, so a table with equal or
      // proportional neighbours says why.
      "// tie        %s\n"
      "// lambda     %.4f  target = lambda * sigma(K * score) + (1 - lambda) * "
      "result\n"
      "%s"
      "//\n"
      "// Paste over the corresponding definitions. The piece defines and the\n"
      "// two tables live in src/eval_tables.hpp; the mobility, king safety,\n"
      "// passed pawn, pawn structure, piece placement, tempo and S123's\n"
      "// passed pawn family weights at the end live in src/evaluation.cpp,\n"
      "// a different file and easy to miss.\n"
      "// S027, S028, S034 and S035, DEC-015: measured by SPRT before any of\n"
      "// it is kept.\n\n",
      positions, CHESSO_BUILD_COMMIT, opts.data.c_str(), corpus.sha256.c_str(),
      corpus.rows, corpus.bytes, opts.k, train_error, validation_error,
      opts.seed, opts.lr, opts.validation, trace.rows, trace.mismatches,
      tuner_optimiser::kind_name(opts.optimiser), opts.epochs, opts.report,
      opts.patience, opts.threads, split.c_str(), opts.only.c_str(),
      opts.freeze.empty() ? "(nothing)" : opts.freeze.c_str(), gauges_held,
      tuner_groups::gauges().size(), tie_line.c_str(), opts.lambda,
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
  // fit quietly discarding its own parameters, which is exactly what this
  // function did with mobility's eight until it was checked.
  //
  // Mobility since S121 is eight tables, one per piece type and half, each one
  // dimensional so .tuning/apply_fit.py pastes them as it pastes every array.
  // The widths are the engine's own (MOBILITY_WIDTH), never literals here.
  fprintf(out,
          "\n// The mobility tables live in src/evaluation.cpp, not "
          "eval_tables.hpp.\n");

  static const char* const mobility_names[4] = {"knight", "bishop", "rook",
                                                "queen"};
  static_assert(
      tuner_layout::MOBILITY_COUNT == static_cast<size_t>(MOBILITY_CELLS),
      "the tuner's mobility block is the engine's table");

  for (int type = 0; type < 4; ++type) {
    for (int table = 0; table < 2; ++table) {
      const size_t base = ((table == 0) ? MOB_MG_BASE : MOB_EG_BASE) +
                          static_cast<size_t>(MOBILITY_OFFSET[type]);

      fprintf(out, "constexpr int mobility_%s_%s[%d] = {", mobility_names[type],
              (table == 0) ? "mg" : "eg", MOBILITY_WIDTH[type]);

      for (int c = 0; c < MOBILITY_WIDTH[type]; ++c) {
        fprintf(out, "%s%d", (c == 0) ? "" : ", ",
                static_cast<int>(
                    std::lround(params[base + static_cast<size_t>(c)])));
      }

      fprintf(out, "};\n");
    }
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? KS_MG_BASE : KS_EG_BASE;

    fprintf(out, "\nconstexpr int king_safety_%s[KS_FEATURE_COUNT] = {\n",
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

    // The width is the model's, which is the engine's own array's: a literal
    // here would emit a header that still compiles when the two part, padding
    // the tail with zeros or refusing nothing.
    fprintf(out, "\nconstexpr int passed_pawn_%s[%zu] = {",
            (table == 0) ? "mg" : "eg", PASSED_PAWN_COUNT);

    for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
      fprintf(out, "%s%d", (i == 0) ? "" : ", ",
              static_cast<int>(std::lround(params[base + i])));
    }

    // The index is how far the pawn has come, not which rank it stands on, so
    // the trailing comment names the buckets from the pawn's own point of view.
    fprintf(out, "};  // second rank .. sixth\n");
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? PS_MG_BASE : PS_EG_BASE;

    fprintf(out, "\nconstexpr int pawn_structure_%s[3] = {",
            (table == 0) ? "mg" : "eg");

    for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
      fprintf(out, "%s%d", (i == 0) ? "" : ", ",
              static_cast<int>(std::lround(params[base + i])));
    }

    fprintf(out, "};  // isolated doubled backward\n");
  }

  for (int table = 0; table < 2; ++table) {
    const size_t base = (table == 0) ? PL_MG_BASE : PL_EG_BASE;

    fprintf(out, "\nconstexpr int piece_placement_%s[%zu] = {",
            (table == 0) ? "mg" : "eg", PIECE_PLACEMENT_COUNT);

    for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
      fprintf(out, "%s%d", (i == 0) ? "" : ", ",
              static_cast<int>(std::lround(params[base + i])));
    }

    fprintf(out,
            "};  // pair, open, half open, bad bishop, rammed, trapped minor, "
            "trapped rook\n");
  }

  // One number per table and no array, so these two lines are the whole term.
  // Emitted here for the reason the eight above are: a parameter the fit moves
  // and the writer forgets is a parameter the run silently discards.
  fprintf(out, "\nconstexpr int tempo_mg = %d;\nconstexpr int tempo_eg = %d;\n",
          static_cast<int>(std::lround(params[TEMPO_MG_BASE])),
          static_cast<int>(std::lround(params[TEMPO_EG_BASE])));

  // S123's family, the same way: five arrays a half, widths the model's.
  struct family_array_t
  {
    const char* name;
    size_t mg_base;
    size_t eg_base;
    size_t width;
  };

  const family_array_t family[] = {
      {"passed_free", tuner_layout::PF_MG_BASE, tuner_layout::PF_EG_BASE,
       tuner_layout::PASSED_FREE_COUNT},
      {"passed_safe", tuner_layout::PSF_MG_BASE, tuner_layout::PSF_EG_BASE,
       tuner_layout::PASSED_SAFE_COUNT},
      {"passed_king_own", tuner_layout::PKO_MG_BASE, tuner_layout::PKO_EG_BASE,
       tuner_layout::PASSED_KING_COUNT},
      {"passed_king_enemy", tuner_layout::PKE_MG_BASE,
       tuner_layout::PKE_EG_BASE, tuner_layout::PASSED_KING_COUNT},
      {"candidate_passer", tuner_layout::CP_MG_BASE, tuner_layout::CP_EG_BASE,
       tuner_layout::CANDIDATE_PASSER_COUNT},
  };

  fprintf(out,
          "\n// S123's passed pawn family, by own rank from the second.\n");

  for (const family_array_t& array : family) {
    for (int table = 0; table < 2; ++table) {
      const size_t base = (table == 0) ? array.mg_base : array.eg_base;

      fprintf(out, "constexpr int %s_%s[%zu] = {", array.name,
              (table == 0) ? "mg" : "eg", array.width);

      for (size_t i = 0; i < array.width; ++i) {
        fprintf(out, "%s%d", (i == 0) ? "" : ", ",
                static_cast<int>(std::lround(params[base + i])));
      }

      fprintf(out, "};\n");
    }
  }

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
      "                   or one piece placement term (S135): %s\n"
      "                   or a comma-separated list of them, their union\n"
      "                   (S123)\n"
      "  --freeze LIST    hold these groups or terms instead, comma\n"
      "                   separated, and fit everything else; not `all`.\n"
      "                   Applied after --only, so the two intersect\n"
      "  --tie FILE       S121: mobility cells that share a parameter, one\n"
      "                   `mobility_<piece>_<half> <cell> <anchor> [scale]`\n"
      "                   a line, or since S123 a passed pawn rank,\n"
      "                   `<passed_free|passed_safe|passed_king_own|\n"
      "                   passed_king_enemy|candidate_passer>_<half> ...`;\n"
      "                   both ends must be free\n"
      "  --k VALUE        sigmoid scale; 0 fits it from the data, against\n"
      "                   the game result whatever --lambda says\n"
      "  --lambda F       fit against F * sigma(K * score) + (1 - F) *\n"
      "                   result, F in [0, 1]. 0, the default, is the game\n"
      "                   outcome alone and is the fit before S075. Two\n"
      "                   lambdas are compared on held-out error against\n"
      "                   the game result, never on the training error\n"
      "  --optimiser NAME adam (default) or adagrad\n"
      "  --lr VALUE       step size (default: 1.0 for adam, %.1f for\n"
      "                   adagrad)\n"
      "  --epochs N       maximum full-batch steps (default 20000)\n"
      "  --report N       report and checkpoint every N epochs (100)\n"
      "  --patience N     stop after this many reports without a new best\n"
      "  --validation F   held-out fraction, whole games (default 0.1)\n"
      "  --holdout FILE   the fixed validation set: fit on every row of\n"
      "                   --data and read held-out error on FILE instead of\n"
      "                   a split, --validation and --seed unused; refused\n"
      "                   when the two share a position\n"
      "  --threads N      worker threads (default: every hardware thread)\n"
      "  --seed N         seed the game-level split shuffles (default 1)\n",
      GROUP_LIST, tuner_groups::PLACEMENT_TERM_LIST,
      tuner_optimiser::ADAGRAD_DEFAULT_LR);
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
    } else if (arg == "--tie") {
      opts.tie = value;
    } else if (arg == "--k") {
      opts.k = atof(value.c_str());
    } else if (arg == "--lambda") {
      opts.lambda = atof(value.c_str());
    } else if (arg == "--optimiser") {
      if (!tuner_optimiser::parse_kind(value, &opts.optimiser)) {
        fprintf(stderr, "unknown --optimiser '%s': adam or adagrad\n",
                value.c_str());
        return 1;
      }
    } else if (arg == "--lr") {
      opts.lr = atof(value.c_str());
      opts.lr_given = true;
    } else if (arg == "--epochs") {
      opts.epochs = atoi(value.c_str());
    } else if (arg == "--report") {
      opts.report = atoi(value.c_str());
    } else if (arg == "--patience") {
      opts.patience = atoi(value.c_str());
    } else if (arg == "--validation") {
      opts.validation = atof(value.c_str());
    } else if (arg == "--holdout") {
      opts.holdout = value;
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

  // Each optimiser's own default step, unless --lr named one. A step that is
  // not positive is refused: `atof` answers 0.0 for a value it cannot read,
  // and a run that never moves, or moves uphill, would still emit a table.
  if (!opts.lr_given) {
    opts.lr = (opts.optimiser == tuner_optimiser::ADAM)
                  ? 1.0
                  : tuner_optimiser::ADAGRAD_DEFAULT_LR;
  } else if (!(opts.lr > 0.0)) {
    fprintf(stderr, "--lr must be a positive number\n");
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

  // A comma-separated list frees the union, S123 (tools/tuner_groups.hpp,
  // only_mask()); one name frees what it always did.
  if (!tuner_groups::only_mask(opts.only, &mask)) {
    fprintf(stderr,
            "unknown --only group '%s'; groups are: %s; terms are: %s; a "
            "comma-separated list of them frees their union, never with "
            "`all`\n",
            opts.only.c_str(), GROUP_LIST, tuner_groups::PLACEMENT_TERM_LIST);
    return 1;
  }

  // --freeze applies after --only, so the two compose: --only psqt --freeze
  // material frees the psqt block and nothing else, since material was already
  // held. `all` is refused, as is any name free_mask does not know. S065.
  if (!freeze_mask(opts.freeze, &mask)) {
    fprintf(stderr,
            "unusable --freeze list '%s'; it takes a comma-separated subset of "
            "%s, or of %s, and not `all`\n",
            opts.freeze.c_str(), GROUP_LIST, tuner_groups::PLACEMENT_TERM_LIST);
    return 1;
  }

  // S121's ties, read and checked before anything is loaded: a tied cell is
  // taken out of the mask -- its parameter is its anchor's -- so it is not
  // counted free and takes no step of its own.
  std::vector<tuner_groups::tie_t> ties;
  std::string tie_line = "(nothing)";

  if (!opts.tie.empty()) {
    std::ifstream tie_file(opts.tie);
    std::stringstream tie_text;
    tie_text << tie_file.rdbuf();
    std::string tie_error;

    if (!tie_file ||
        !tuner_groups::parse_ties(tie_text.str(), &ties, &tie_error) ||
        !tuner_groups::ties_fit_mask(ties, mask, &tie_error)) {
      fprintf(stderr, "unusable --tie %s: %s\n", opts.tie.c_str(),
              tie_error.empty() ? "cannot read it" : tie_error.c_str());
      return 1;
    }

    corpus_id_t tie_id;
    if (!corpus_hash::hash_file(opts.tie, &tie_id.sha256, &tie_id.bytes,
                                &tie_id.rows)) {
      fprintf(stderr, "could not hash %s\n", opts.tie.c_str());
      return 1;
    }

    for (const tuner_groups::tie_t& tie : ties) {
      mask[tie.tied] = 0;
    }

    tie_line = opts.tie + ", sha256 " + tie_id.sha256 + ", " +
               std::to_string(ties.size()) + " cells tied";
    fprintf(stderr, "ties: %s\n", tie_line.c_str());
  }

  // S121: a mask that frees both sides of a mobility level walks a direction
  // no gauge holds -- S275's drift -- so it is refused until the level has a
  // hold (tools/tuner_groups.hpp, mobility_levels_held()).
  std::string level_error;
  if (!tuner_groups::mobility_levels_held(mask, &level_error)) {
    fprintf(stderr, "--only %s --freeze %s: %s\n", opts.only.c_str(),
            opts.freeze.empty() ? "(nothing)" : opts.freeze.c_str(),
            level_error.c_str());
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

  // Every row through evaluate() on the trace build, on every thread, and
  // every row's trace scored back the engine's way and compared with what
  // evaluate() returned. S262.
  dataset_t data;
  tuner_trace::load_report_t trace;
  std::string load_error;

  if (!tuner_trace::load(opts.data, &data, &trace, &load_error, opts.threads)) {
    fprintf(stderr, "could not load %s: %s\n", opts.data.c_str(),
            load_error.c_str());
    return 1;
  }

  fprintf(stderr,
          "trace: %" PRIu64 " rows, %" PRIu64
          " entries (%.2f a row), %.1f s; %" PRIu64
          " differ from evaluate(), the clamp binds on %" PRIu64 "\n",
          trace.rows, trace.entries,
          static_cast<double>(trace.entries) / static_cast<double>(trace.rows),
          trace.seconds, trace.mismatches, trace.clamped);

  // The identity is the model's whole claim to be evaluate(), so a corpus that
  // breaks it on one row is not fitted at all: whatever disagrees there would
  // be fitted as if it were the engine.
  if (trace.mismatches > 0) {
    fprintf(stderr,
            "the trace disagrees with evaluate() on %" PRIu64
            " rows, the first: %s\nnothing fitted\n",
            trace.mismatches, trace.first_mismatch.c_str());
    return 1;
  }

  // S082: the fixed validation set. Every later fit and screen reads its
  // held-out error on these rows rather than on a split of its own corpus, so
  // two corpora are compared on one set -- which two splits cannot do, a
  // different corpus being a different held-out set (DEC-055). Loaded and
  // checked as the corpus is, and refused when one of its positions is also
  // one the fit trains on.
  const bool fixed_holdout = !opts.holdout.empty();
  dataset_t holdout;
  corpus_id_t holdout_id;

  if (fixed_holdout) {
    if (!corpus_hash::hash_file(opts.holdout, &holdout_id.sha256,
                                &holdout_id.bytes, &holdout_id.rows)) {
      fprintf(stderr, "could not hash %s\n", opts.holdout.c_str());
      return 1;
    }

    tuner_trace::load_report_t holdout_trace;

    if (!tuner_trace::load(opts.holdout, &holdout, &holdout_trace, &load_error,
                           opts.threads)) {
      fprintf(stderr, "could not load %s: %s\n", opts.holdout.c_str(),
              load_error.c_str());
      return 1;
    }

    if (holdout_trace.mismatches > 0) {
      fprintf(stderr,
              "the trace disagrees with evaluate() on %" PRIu64
              " rows of %s, the first: %s\nnothing fitted\n",
              holdout_trace.mismatches, opts.holdout.c_str(),
              holdout_trace.first_mismatch.c_str());
      return 1;
    }

    uint64_t shared = 0;
    std::string shared_error;

    if (!tuner_split::shared_positions(opts.data, opts.holdout, &shared,
                                       &shared_error)) {
      fprintf(stderr, "%s\n", shared_error.c_str());
      return 1;
    }

    fprintf(stderr,
            "held out: %s, sha256 %s, %" PRIu64 " rows; %" PRIu64
            " corpus rows stand on one of its positions\n",
            opts.holdout.c_str(), holdout_id.sha256.c_str(), holdout_id.rows,
            shared);

    if (shared > 0) {
      fprintf(stderr,
              "a position on both sides measures recall, not generalisation; "
              "build the corpus with tools/corpus_split.py sample, which "
              "drops them\nnothing fitted\n");
      return 1;
    }
  }

  // Positions from one game are consecutive and share a label, so a row-level
  // split holds out rows whose game is in the training set: 99.47 % of games
  // landed on both sides of the cut and the held-out error was measuring
  // memorisation. Whole games are held out instead, and tools/tuner_split.hpp
  // is where the boundary between them comes from. S066. Under --holdout
  // nothing of the corpus is held out and every row trains; the blocks are
  // still counted, for the header.
  std::vector<uint32_t> starts;
  tuner_split::game_starts(data.ply, &starts);

  std::vector<uint32_t> index;
  size_t train_count = 0;

  if (fixed_holdout) {
    index.resize(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
      index[i] = static_cast<uint32_t>(i);
    }
    train_count = data.size();
  } else {
    train_count = tuner_split::split(starts, data.size(), opts.validation,
                                     opts.seed, &index);
  }

  // Where every held-out figure below is read: the fixed set's own rows, or
  // the tail of the corpus's shuffled index.
  std::vector<uint32_t> holdout_index(holdout.size());
  for (size_t i = 0; i < holdout.size(); ++i) {
    holdout_index[i] = static_cast<uint32_t>(i);
  }

  const dataset_t& held_data = fixed_holdout ? holdout : data;
  const std::vector<uint32_t>& held_index =
      fixed_holdout ? holdout_index : index;
  const size_t held_first = fixed_holdout ? 0 : train_count;
  const size_t held_last = fixed_holdout ? holdout.size() : data.size();
  const size_t validation_count = held_last - held_first;

  fprintf(stderr,
          "%zu positions, %zu games, %zu train, %zu validation (%.4f%%), "
          "%zu parameters, group %s, freeze %s, %zu free\n",
          data.size(), starts.size(), train_count, validation_count,
          100.0 * static_cast<double>(validation_count) /
              static_cast<double>(train_count + validation_count),
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

  // The null directions this run holds still, by name. One with a column that
  // --only or --freeze holds is left to the freeze, so the count is the mask's
  // to decide. S275.
  const std::vector<tuner_groups::gauge_t> held =
      tuner_groups::held_gauges(mask);
  std::string held_names;

  for (const tuner_groups::gauge_t& gauge : held) {
    held_names += (held_names.empty() ? ": " : ", ") + gauge.name;
  }

  fprintf(stderr, "gauges held: %zu of %zu%s\n", held.size(),
          tuner_groups::gauges().size(), held_names.c_str());

  // Starting point is what the engine ships with, so a fit that finds nothing
  // reports the error of the hand-written constants rather than of noise.
  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

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

  // The same two targets over the fixed set's rows, built the same way from
  // its own columns, with the K the training rows gave. S082.
  std::vector<double> holdout_wdl_target(holdout.size());

  for (size_t i = 0; i < holdout.size(); ++i) {
    holdout_wdl_target[i] = holdout.result[i];
  }

  std::vector<double> holdout_train_target = holdout_wdl_target;

  if (opts.lambda > 0.0) {
    for (size_t i = 0; i < holdout.size(); ++i) {
      holdout_train_target[i] = tuner_target::blend(
          opts.lambda, holdout.score[i], holdout.result[i], opts.k);
    }
  }

  const std::vector<double>& held_train_target =
      fixed_holdout ? holdout_train_target : train_target;
  const std::vector<double>& held_wdl_target =
      fixed_holdout ? holdout_wdl_target : wdl_target;

  const double start_train =
      error_range(data, params.data(), opts.k, index, 0, train_count,
                  opts.threads, train_target);
  const double start_validation =
      error_range(held_data, params.data(), opts.k, held_index, held_first,
                  held_last, opts.threads, held_train_target);

  // The selection metric at the constants the run started from. At lambda 0 the
  // two targets are the same vector, so this is the same number and the same
  // pass is not made twice.
  const double start_wdl =
      (opts.lambda > 0.0)
          ? error_range(held_data, params.data(), opts.k, held_index,
                        held_first, held_last, opts.threads, held_wdl_target)
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

  // The optimiser, and the levels the gauges hold every step: the ones this
  // run started from. Both moved to tools/tuner_model.hpp at S275 so a test
  // runs the same descent, and to tools/tuner_optimiser.hpp at S262.
  tuner_optimiser::state_t optimiser(PARAM_COUNT, opts.optimiser);
  std::vector<double> grad;
  const std::vector<double> start = params;

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

  // S121's ties, put on their anchors once the start figures and the start
  // vector are taken at the shipped constants -- so a fit's gain is read
  // against what ships (DEC-315) -- and before the first gradient, so every
  // step is taken on the tied parameterisation.
  tuner_groups::copy_ties(ties, params.data());

  for (int epoch = 1; epoch <= opts.epochs; ++epoch) {
    gradient(data, params.data(), opts.k, index, 0, train_count, opts.threads,
             train_target, &grad);

    tuner_groups::fold_ties(ties, &grad);
    descend(grad, mask, start.data(), opts.lr, &optimiser, params.data());
    tuner_groups::copy_ties(ties, params.data());

    if (epoch % opts.report != 0) { continue; }

    const double train = error_range(data, params.data(), opts.k, index, 0,
                                     train_count, opts.threads, train_target);
    const double validation =
        error_range(held_data, params.data(), opts.k, held_index, held_first,
                    held_last, opts.threads, held_train_target);
    const double wdl =
        (opts.lambda > 0.0)
            ? error_range(held_data, params.data(), opts.k, held_index,
                          held_first, held_last, opts.threads, held_wdl_target)
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

  // What was held out, for the header: the fixed set by name and digest, or
  // the corpus's own split as S066 cut it.
  char split_text[160];

  if (fixed_holdout) {
    snprintf(split_text, sizeof split_text,
             "fixed held-out set, S082: %zu rows, every corpus row trains",
             validation_count);
  } else {
    snprintf(split_text, sizeof split_text,
             "by game, S066: %zu games, %zu rows held out, %.4f%%",
             starts.size(), validation_count,
             100.0 * static_cast<double>(validation_count) /
                 static_cast<double>(data.size()));
  }

  std::string split = split_text;

  if (fixed_holdout) {
    split += "\n//            " + opts.holdout + "\n//            sha256 " +
             holdout_id.sha256;
  }

  write_tables(opts.out, best.data(), opts, corpus, data.size(), split,
               final_train, best_validation, best_wdl, start_wdl, held.size(),
               trace, tie_line);

  return 0;
}
