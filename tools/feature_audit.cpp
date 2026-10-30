// What the tuning corpus actually contains, per evaluation feature. S100.
//
//   build/tools/feature_audit --data .tuning/selfplay_v2_dedup.tsv
//
// S100 is a diagnosis: bishop pair, rook on an open file, rook on a half-open
// file, rook on the seventh and tempo all shipped at zero, and
// passed_pawn_mg[5] fitted below the bucket beneath it. Five terms at exactly
// zero is a pattern rather than five results, and four causes can produce it --
// a feature extracted wrongly, a corpus that cannot discriminate the feature, a
// measurement procedure that could not resolve the effect, or a real zero. This
// tool answers the corpus half of that. It decides nothing on its own; what it
// produces is evidence for the ledger in the step file.
//
// Four reports and one writer, all off the same load. Since S276 the load is
// the tuner's own: every row put through evaluate() on the trace build and
// summed into the coefficients evaluate() applied (tools/tuner_trace.hpp),
// checked row by row against evaluate() the way the tuner checks it. Until
// S276 it was the mirror's, tools/eval_model.hpp's hand-written extractors
// packed per row by tools/tuner_model.hpp, both deleted then.
//
//   columns      every stored row against its FEN traced again, one at a time
//                and in file order, over the whole corpus. It runs the same
//                trace load() runs, so it is not a check of what a coefficient
//                *means* -- tests/test_eval_trace.cpp holds those, against the
//                engine's per-colour counts and a hand walk, and
//                tests/test_eval_model.cpp holds the counts against the walk
//                and against placements worked out by hand. What it catches is
//                everything between a correct trace and a fitted weight: a row
//                stored in the wrong slot, an offset off by one, blocks put
//                back in the wrong order. It also counts the rows whose stored
//                phase column disagrees with the engine's phase, which load()
//                uses instead (2026-09-10_adversarial-F36, S134).
//
//   identities   the rank of the design the tuner descends and every exact
//                linear dependency in it, over every row. S100 found two by
//                argument and checked them here: rook on the seventh and passer
//                bucket 5 were each identically the signed sum of eight
//                piece-square columns, because a rook or a pawn on its own
//                seventh always lands on squares 8..15 once Black is mirrored
//                by xor 56, and that pawn is a passer by definition. S134
//                folded both into the tables (DEC-090) and replaced the two
//                checks with the general one: the Gram matrix of the design,
//                summed exactly, its rank modulo a prime, the dependencies the
//                parameterisation has by construction verified on every row,
//                and anything left over found and printed instead of argued
//                about. The modular rank bounds the true rank below and the
//                exactly verified, linearly independent null vectors bound it
//                above, so the report states the rank only when the two meet. A
//                joint fit cannot resolve a dependency and an --only fit
//                resolves one only against the frozen table, so a new term
//                defined on a single rank -- S123's, S133's buckets -- is
//                checked here before it is fitted. The engine-side check S134
//                ran before the fold is adocs/data/S134_engine_identity.diff.
//                Since S275 it ends on what the fit does about them: each
//                gauge tools/tuner_groups.hpp holds checked null on every row,
//                and the gauges with the zero columns shown to span the whole
//                null space.
//
//   occurrence   how often each column is non-zero, whole corpus and by phase
//                band. The coverage form of the corpus hypothesis: a column
//                that is almost never non-zero has almost no signal to fit.
//                Split by phase because a column can be well covered overall
//                and empty in the band whose weight it is -- S065's
//                queens-under-phase-4 story, and the reason four positions had
//                to be added to tests/test_eval_positions.hpp at S100.
//
//   collinearity R^2 of each term column regressed on the piece-square columns
//                it overlaps. The correlation form of the same hypothesis, and
//                the one counting cannot answer. Above 0.99 the column carries
//                almost nothing the tables do not already carry, and a fit
//                returning zero for it is what a *correct* pipeline does. The
//                two exact identities S100 found came back at exactly 1.0,
//                which was this report checking itself; both columns are gone.
//
//   --plant      writes a copy of the corpus with the result column replaced by
//                sigmoid(K * score) at planted, non-zero term weights, the
//                score the trace's smooth one -- the score the tuner descends.
//                The labels are then generated by a known parameter vector, so
//                a fit that cannot recover it has a defect and a fit that can
//                has its pipeline vouched for end to end. Every number planted
//                is written here and derived from nothing outside this
//                repository.
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "tuner_groups.hpp"
#include "tuner_layout.hpp"
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
using tuner_trace::dataset_t;

// The seven piece placement features in the order their weights are indexed,
// which src/evaluation.hpp states in prose over the arrays. Numbered by hand:
// the engine's own enum sits inside src/evaluation.cpp, where no tool can
// include it, so the assert below ties the count to the engine's arrays
// (through tools/tuner_layout.hpp) and nothing ties the order but this
// comment. A reordered engine enum mislabels the collinearity report's
// placement rows; it changes no rank and no identity. The last four are
// S135's.
constexpr size_t PL_BISHOP_PAIR = 0;
constexpr size_t PL_ROOK_OPEN = 1;
constexpr size_t PL_ROOK_HALF_OPEN = 2;
constexpr size_t PL_BAD_BISHOP = 3;
constexpr size_t PL_BAD_BISHOP_RAMMED = 4;
constexpr size_t PL_TRAPPED_MINOR = 5;
constexpr size_t PL_TRAPPED_ROOK = 6;
static_assert(PL_TRAPPED_ROOK + 1 == PIECE_PLACEMENT_COUNT,
              "the engine's piece placement arrays changed width: name the "
              "features here again");

// The piece types as the trace's table index codes them: index / 64 of a
// TERM_PSQT entry, Black's squares mirrored the way eval_add_piece() mirrors
// them.
constexpr size_t PAWN_TYPE = 0;
constexpr size_t KNIGHT_TYPE = 1;
constexpr size_t BISHOP_TYPE = 2;
constexpr size_t ROOK_TYPE = 3;
constexpr size_t KING_TYPE = 5;

// The piece placement features and the passer buckets, in the order their
// weights are indexed, for the reports below.
const char* const PLACEMENT_NAMES[PIECE_PLACEMENT_COUNT] = {
    "bishop pair",   "rook open",     "rook half-open", "bad bishop",
    "bishop rammed", "trapped minor", "trapped rook"};
const char* const PASSER_NAMES[PASSED_PAWN_COUNT] = {
    "passer bucket 0", "passer bucket 1", "passer bucket 2", "passer bucket 3",
    "passer bucket 4"};


struct options_t
{
  std::string data;
  std::string plant;
  std::string plant_group = "all";
  double plant_k =
      0.7624;  // S065's fitted K over this corpus, printed by that run
  size_t sample = 200000;  // rows the regression uses; every row for the rest
  // S121: the free set the identity report is read for, a tuner `--only`
  // group. A null direction the run's frozen columns hold is held; one free
  // on every side is a direction the fit can walk.
  std::string free_group = "all";
  bool columns = true;
  bool identities = true;
  bool occurrence = true;
  bool collinearity = true;
};


// The feature a term's entry `index` is stored under, in the trace's numbering.
uint32_t feature_of(eval_trace::term_t term, size_t index)
{
  return tuner_trace::feature_table().base[term] + static_cast<uint32_t>(index);
}


// One row's coefficient for one feature, 0 when the row has none: the trace
// drops zeros, so absence is the zero.
int coefficient(const dataset_t& data, size_t row, uint32_t feature)
{
  for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
    if (data.entries[j].feature == feature) { return data.entries[j].coef; }
  }

  return 0;
}


// The signed occupancy of one piece-square column: +1 for a White piece there,
// -1 for a Black one, which is exactly the coefficient that column carries in
// the linear model. Read off the trace's table entries, because those are what
// the fit reads; a White piece and a Black one mirrored onto the same square
// have already cancelled there, as they cancel in the fit.
struct occupancy_t
{
  std::vector<int8_t> value;  // 64 per row, one piece type
};


occupancy_t occupancy_of_type(const dataset_t& data,
                              size_t type,
                              const std::vector<uint32_t>& rows)
{
  const uint32_t first = feature_of(eval_trace::TERM_PSQT, type * 64);
  const uint32_t last = feature_of(eval_trace::TERM_PSQT, type * 64 + 64);

  occupancy_t out;
  out.value.assign(rows.size() * 64, 0);

  for (size_t r = 0; r < rows.size(); ++r) {
    const size_t row = rows[r];

    for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
      const tuner_trace::entry_t e = data.entries[j];

      if (e.feature < first || e.feature >= last) { continue; }

      out.value[r * 64 + (e.feature - first)] = static_cast<int8_t>(e.coef);
    }
  }

  return out;
}


// Whether row `row` of `data` is exactly `trace`: the same entries in the same
// order, the same per-side entries, the same phase and side to move. The
// columns report's comparison.
bool row_is(const dataset_t& data,
            size_t row,
            const tuner_trace::row_trace_t& trace)
{
  const uint32_t first = data.offsets[row];
  const uint32_t count = data.offsets[row + 1] - first;

  if (count != trace.entries.size()) { return false; }

  for (uint32_t i = 0; i < count; ++i) {
    const tuner_trace::entry_t a = data.entries[first + i];
    const tuner_trace::entry_t b = trace.entries[i];

    if (a.feature != b.feature || a.coef != b.coef) { return false; }
  }

  const uint32_t side_first = data.side_offsets[row];
  const uint32_t side_count = data.side_offsets[row + 1] - side_first;

  if (side_count != trace.side_entries.size()) { return false; }

  for (uint32_t i = 0; i < side_count; ++i) {
    const tuner_trace::side_entry_t a = data.side_entries[side_first + i];
    const tuner_trace::side_entry_t b = trace.side_entries[i];

    if (a.group != b.group || a.side != b.side || a.input != b.input ||
        a.coef != b.coef) {
      return false;
    }
  }

  return data.phase[row] == trace.phase && data.stm[row] == trace.stm;
}


// Ordinary least squares of `y` on `columns` plus an intercept, by normal
// equations, returning R^2. The design is at most 65 wide and the entries are
// in -1..1, so a Cholesky on the Gram matrix is enough; a column that is
// constant over the sample makes the matrix singular and is dropped rather than
// pivoted, since a column nothing varies explains nothing.
double r_squared(const std::vector<double>& y,
                 const std::vector<std::vector<double>>& columns)
{
  const size_t n = y.size();
  if (n == 0) { return 0.0; }

  std::vector<const std::vector<double>*> keep;
  for (const std::vector<double>& column : columns) {
    double lo = column[0];
    double hi = column[0];

    for (const double value : column) {
      lo = std::min(lo, value);
      hi = std::max(hi, value);
    }

    if (hi > lo) { keep.push_back(&column); }
  }

  const size_t width = keep.size() + 1;  // + intercept
  std::vector<double> gram(width * width, 0.0);
  std::vector<double> rhs(width, 0.0);

  for (size_t i = 0; i < n; ++i) {
    std::vector<double> row(width);
    row[0] = 1.0;
    for (size_t c = 0; c < keep.size(); ++c) {
      row[c + 1] = (*keep[c])[i];
    }

    for (size_t a = 0; a < width; ++a) {
      rhs[a] += row[a] * y[i];
      for (size_t b = 0; b < width; ++b) {
        gram[a * width + b] += row[a] * row[b];
      }
    }
  }

  // Ridge of 1e-9 on the diagonal: the design has exactly collinear columns by
  // construction here -- that is the thing being measured -- and a bare
  // Cholesky on a singular Gram matrix fails rather than returning the fit that
  // exists.
  for (size_t a = 0; a < width; ++a) {
    gram[a * width + a] += 1e-9;
  }

  // Cholesky, in place.
  for (size_t a = 0; a < width; ++a) {
    for (size_t b = 0; b <= a; ++b) {
      double sum = gram[a * width + b];

      for (size_t c = 0; c < b; ++c) {
        sum -= gram[a * width + c] * gram[b * width + c];
      }

      if (a == b) {
        if (sum <= 0.0) { return -1.0; }
        gram[a * width + a] = std::sqrt(sum);
      } else {
        gram[a * width + b] = sum / gram[b * width + b];
      }
    }
  }

  std::vector<double> beta(width, 0.0);
  for (size_t a = 0; a < width; ++a) {
    double sum = rhs[a];
    for (size_t c = 0; c < a; ++c) {
      sum -= gram[a * width + c] * beta[c];
    }
    beta[a] = sum / gram[a * width + a];
  }
  for (size_t a = width; a-- > 0;) {
    double sum = beta[a];
    for (size_t c = a + 1; c < width; ++c) {
      sum -= gram[c * width + a] * beta[c];
    }
    beta[a] = sum / gram[a * width + a];
  }

  double mean = 0.0;
  for (const double value : y) {
    mean += value;
  }
  mean /= static_cast<double>(n);

  double residual = 0.0;
  double total = 0.0;

  for (size_t i = 0; i < n; ++i) {
    double predicted = beta[0];
    for (size_t c = 0; c < keep.size(); ++c) {
      predicted += beta[c + 1] * (*keep[c])[i];
    }

    residual += (y[i] - predicted) * (y[i] - predicted);
    total += (y[i] - mean) * (y[i] - mean);
  }

  if (total <= 0.0) { return -1.0; }

  return 1.0 - residual / total;
}


// ------------------------------------------------ the design and its rank
//
// S134. One row of the design: GAME_PHASE_MAX times the derivative of the
// score with respect to every parameter, the stage-two clamp left out, so the
// identities reported are the linear score's. The scale is what keeps every
// entry an integer -- the taper weights are phase / 24 and (24 - phase) / 24
// -- so the Gram matrix below is summed exactly.
//
// Since S276 the row is the trace's: each coefficient evaluate() recorded,
// times 24 for a piece value, which is untapered, and times the phase and 24
// minus the phase for a tapered pair's two halves. Until S276 it was the same
// derivative taken from the mirror's features, with the clamp left out the
// same way, which the mirror's gradient did too.
struct design_row_t
{
  std::vector<uint32_t> index;  // ascending, no repeats
  std::vector<int64_t> value;   // all non-zero
};


class design_builder_t
{
public:
  design_builder_t() : dense_(PARAM_COUNT, 0), marked_(PARAM_COUNT, 0) {}

  void build(const dataset_t& data, size_t row, design_row_t* out)
  {
    const std::vector<tuner_trace::feature_t>& features =
        tuner_trace::feature_table().features;

    const int64_t mg = data.phase[row];
    const int64_t eg = GAME_PHASE_MAX - mg;

    // Accumulated rather than appended, as before S276, although the trace
    // has already summed each feature once: two features never share a
    // parameter (tests/test_eval_trace.cpp, "the trace's features cover the
    // parameter vector exactly once"), so this costs nothing and assumes
    // nothing.
    for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
      const tuner_trace::entry_t e = data.entries[j];
      const tuner_trace::feature_t& f = features[e.feature];
      const int64_t coef = e.coef;

      if (f.site == tuner_trace::SITE_MATERIAL) {
        add(f.mg, coef * GAME_PHASE_MAX);
        continue;
      }

      add(f.mg, coef * mg);
      add(f.eg, coef * eg);
    }

    std::sort(touched_.begin(), touched_.end());
    out->index.clear();
    out->value.clear();

    for (const uint32_t i : touched_) {
      if (dense_[i] != 0) {
        out->index.push_back(i);
        out->value.push_back(dense_[i]);
      }

      dense_[i] = 0;
      marked_[i] = 0;
    }

    touched_.clear();
  }

private:
  void add(size_t index, int64_t value)
  {
    if (value == 0) { return; }

    if (!marked_[index]) {
      marked_[index] = 1;
      touched_.push_back(static_cast<uint32_t>(index));
    }

    dense_[index] += value;
  }

  std::vector<int64_t> dense_;
  std::vector<uint8_t> marked_;
  std::vector<uint32_t> touched_;
};


// What each parameter is, for a dependency a reader has to be able to read.
// A piece-square column is named by the square White's piece stands on, which
// is the square a Black piece is mirrored onto.
std::string column_name(size_t i)
{
  static const char* const types[6] = {"pawn", "knight", "bishop",
                                       "rook", "queen",  "king"};
  static const char* const mobility[4] = {"knight", "bishop", "rook", "queen"};
  static const char* const safety[KING_SAFETY_COUNT] = {
      "knight attackers", "bishop attackers", "rook attackers",
      "queen attackers",  "zone attacks",     "shield near",
      "shield far",       "open file",        "half open file"};
  static const char* const structure[PAWN_STRUCTURE_COUNT] = {
      "isolated", "doubled", "backward"};

  const auto square = [](size_t s) {
    return std::string(1, static_cast<char>('a' + s % 8)) +
           std::string(1, static_cast<char>('8' - s / 8));
  };
  const auto block = [](const char* name, bool mg, const std::string& what) {
    return std::string(name) + (mg ? "_mg[" : "_eg[") + what + "]";
  };

  if (i < MATERIAL_COUNT) { return std::string("material[") + types[i] + "]"; }
  if (i < MOB_MG_BASE) {
    const bool mg = i < EG_BASE;
    const size_t s = i - (mg ? MG_BASE : EG_BASE);
    return block("psqt", mg,
                 std::string(types[s / 64]) + "][" + square(s % 64));
  }
  if (i < KS_MG_BASE) {
    // A cell of S121's table: its piece type and its count.
    const bool mg = i < MOB_EG_BASE;
    const int cell = static_cast<int>(i - (mg ? MOB_MG_BASE : MOB_EG_BASE));
    int type = 3;
    while (cell < MOBILITY_OFFSET[type]) {
      type--;
    }
    return block("mobility", mg,
                 std::string(mobility[type]) + "][" +
                     std::to_string(cell - MOBILITY_OFFSET[type]));
  }
  if (i < PP_MG_BASE) {
    const bool mg = i < KS_EG_BASE;
    return block("king_safety", mg, safety[i - (mg ? KS_MG_BASE : KS_EG_BASE)]);
  }
  if (i < PS_MG_BASE) {
    const bool mg = i < PP_EG_BASE;
    return block("passed_pawn", mg,
                 std::to_string(i - (mg ? PP_MG_BASE : PP_EG_BASE)));
  }
  if (i < PL_MG_BASE) {
    const bool mg = i < PS_EG_BASE;
    return block("pawn_structure", mg,
                 structure[i - (mg ? PS_MG_BASE : PS_EG_BASE)]);
  }
  if (i < TEMPO_MG_BASE) {
    const bool mg = i < PL_EG_BASE;
    return block("piece_placement", mg,
                 PLACEMENT_NAMES[i - (mg ? PL_MG_BASE : PL_EG_BASE)]);
  }
  if (i < tuner_layout::PF_MG_BASE) {
    return (i == TEMPO_MG_BASE) ? "tempo_mg" : "tempo_eg";
  }

  // S123's family: an array, its half and the pawn's own rank index, 0 for
  // the second -- the cell numbering `tuner --tie` reads. S125's after it,
  // each by its weights' cell (src/evaluation.hpp).
  struct family_block_t
  {
    const char* name;
    size_t mg_base;
    size_t eg_base;
    size_t width;
  };

  static const family_block_t family[] = {
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
      {"isolated_backward", tuner_layout::IB_MG_BASE, tuner_layout::IB_EG_BASE,
       tuner_layout::ISOLATED_BACKWARD_COUNT},
      {"isolated_file", tuner_layout::ISF_MG_BASE, tuner_layout::ISF_EG_BASE,
       tuner_layout::ISOLATED_FILE_COUNT},
      {"doubled_file", tuner_layout::DBF_MG_BASE, tuner_layout::DBF_EG_BASE,
       tuner_layout::DOUBLED_FILE_COUNT},
      {"backward_rank", tuner_layout::BKR_MG_BASE, tuner_layout::BKR_EG_BASE,
       tuner_layout::BACKWARD_RANK_COUNT},
      {"phalanx", tuner_layout::PHX_MG_BASE, tuner_layout::PHX_EG_BASE,
       tuner_layout::PHALANX_COUNT},
      {"supported", tuner_layout::SUP_MG_BASE, tuner_layout::SUP_EG_BASE,
       tuner_layout::SUPPORTED_COUNT},
      {"weak_unopposed", tuner_layout::WKU_MG_BASE, tuner_layout::WKU_EG_BASE,
       tuner_layout::WEAK_UNOPPOSED_COUNT},
  };

  for (const family_block_t& f : family) {
    if (i >= f.mg_base && i < f.mg_base + f.width) {
      return block(f.name, true, std::to_string(i - f.mg_base));
    }
    if (i >= f.eg_base && i < f.eg_base + f.width) {
      return block(f.name, false, std::to_string(i - f.eg_base));
    }
  }

  return "unnamed[" + std::to_string(i) + "]";
}


// Arithmetic modulo the Mersenne prime 2^61 - 1. The rank of an integer matrix
// reduced modulo a prime is never more than its rank over the rationals -- a
// minor that is non-zero modulo p is a non-zero integer -- so a full rank here
// is a full rank there, which is the direction the report needs.
constexpr uint64_t PRIME = (uint64_t{1} << 61) - 1;

uint64_t mul_mod(uint64_t a, uint64_t b)
{
  const __uint128_t product = static_cast<__uint128_t>(a) * b;
  uint64_t x = static_cast<uint64_t>(product & PRIME) +
               static_cast<uint64_t>(product >> 61);
  x = (x & PRIME) + (x >> 61);
  return (x >= PRIME) ? x - PRIME : x;
}

uint64_t to_mod(int64_t value)
{
  const int64_t r = value % static_cast<int64_t>(PRIME);
  return static_cast<uint64_t>((r < 0) ? r + static_cast<int64_t>(PRIME) : r);
}

uint64_t inverse_mod(uint64_t a)
{
  uint64_t result = 1;
  uint64_t base = a;

  for (uint64_t e = PRIME - 2; e > 0; e >>= 1) {
    if (e & 1) { result = mul_mod(result, base); }
    base = mul_mod(base, base);
  }

  return result;
}


// Gauss-Jordan over the columns `keep` of the symmetric matrix `gram`. Returns
// the rank and, for each column no pivot was found in, the null vector it
// spans: that column at 1 and each pivot column at minus its entry, read back
// as the signed integer nearest zero.
size_t rank_mod_p(const std::vector<int64_t>& gram,
                  const std::vector<size_t>& keep,
                  std::vector<std::vector<std::pair<size_t, int64_t>>>* null)
{
  const size_t m = keep.size();
  std::vector<uint64_t> a(m * m);

  for (size_t r = 0; r < m; ++r) {
    for (size_t c = 0; c < m; ++c) {
      a[r * m + c] = to_mod(gram[keep[r] * PARAM_COUNT + keep[c]]);
    }
  }

  std::vector<size_t> pivot_column;
  size_t rank = 0;

  for (size_t c = 0; c < m && rank < m; ++c) {
    size_t pivot = rank;
    while (pivot < m && a[pivot * m + c] == 0) {
      pivot++;
    }
    if (pivot == m) { continue; }

    if (pivot != rank) {
      for (size_t k = 0; k < m; ++k) {
        std::swap(a[pivot * m + k], a[rank * m + k]);
      }
    }

    const uint64_t scale = inverse_mod(a[rank * m + c]);
    for (size_t k = c; k < m; ++k) {
      a[rank * m + k] = mul_mod(a[rank * m + k], scale);
    }

    for (size_t r = 0; r < m; ++r) {
      if (r == rank || a[r * m + c] == 0) { continue; }

      const uint64_t factor = a[r * m + c];
      for (size_t k = c; k < m; ++k) {
        const uint64_t sub = mul_mod(factor, a[rank * m + k]);
        const uint64_t cur = a[r * m + k];
        a[r * m + k] = (cur >= sub) ? cur - sub : cur + PRIME - sub;
      }
    }

    pivot_column.push_back(c);
    rank++;
  }

  if (null != nullptr) {
    null->clear();
    std::vector<uint8_t> is_pivot(m, 0);
    for (const size_t c : pivot_column) {
      is_pivot[c] = 1;
    }

    for (size_t f = 0; f < m; ++f) {
      if (is_pivot[f]) { continue; }

      std::vector<std::pair<size_t, int64_t>> vector = {{keep[f], 1}};

      for (size_t r = 0; r < rank; ++r) {
        const uint64_t entry = a[r * m + f];
        if (entry == 0) { continue; }

        const uint64_t negated = PRIME - entry;
        const int64_t value = (negated > PRIME / 2)
                                  ? -static_cast<int64_t>(PRIME - negated)
                                  : static_cast<int64_t>(negated);
        vector.push_back({keep[pivot_column[r]], value});
      }

      std::sort(vector.begin(), vector.end());
      null->push_back(vector);
    }
  }

  return rank;
}


// v' G v, which is the sum over every row of (x . v)^2 -- so it is zero exactly
// when the dependency v holds on every row of the corpus, and no row has to be
// read again to say so. In 128 bits: the Gram entries reach the 10^14s.
__int128 quadratic_form(const std::vector<int64_t>& gram,
                        const std::vector<std::pair<size_t, int64_t>>& v)
{
  __int128 sum = 0;

  for (const auto& [i, a] : v) {
    for (const auto& [j, b] : v) {
      sum += static_cast<__int128>(a) * b * gram[i * PARAM_COUNT + j];
    }
  }

  return sum;
}


// The rank modulo the prime of a set of sparse vectors over the parameters,
// stacked as rows. Equal to their count means they are linearly independent
// modulo the prime, and so over the rationals: a dependency over the
// rationals, scaled to coprime integers, is still one modulo any prime.
size_t rank_of_vectors(
    const std::vector<std::vector<std::pair<size_t, int64_t>>>& vectors)
{
  const size_t k = vectors.size();
  std::vector<uint64_t> a(k * PARAM_COUNT, 0);

  for (size_t r = 0; r < k; ++r) {
    for (const auto& [i, c] : vectors[r]) {
      a[r * PARAM_COUNT + i] = to_mod(c);
    }
  }

  size_t rank = 0;

  for (size_t c = 0; c < PARAM_COUNT && rank < k; ++c) {
    size_t pivot = rank;
    while (pivot < k && a[pivot * PARAM_COUNT + c] == 0) {
      pivot++;
    }
    if (pivot == k) { continue; }

    for (size_t j = 0; j < PARAM_COUNT; ++j) {
      std::swap(a[pivot * PARAM_COUNT + j], a[rank * PARAM_COUNT + j]);
    }

    const uint64_t scale = inverse_mod(a[rank * PARAM_COUNT + c]);
    for (size_t j = c; j < PARAM_COUNT; ++j) {
      a[rank * PARAM_COUNT + j] = mul_mod(a[rank * PARAM_COUNT + j], scale);
    }

    for (size_t r = rank + 1; r < k; ++r) {
      const uint64_t factor = a[r * PARAM_COUNT + c];
      if (factor == 0) { continue; }

      for (size_t j = c; j < PARAM_COUNT; ++j) {
        const uint64_t sub = mul_mod(factor, a[rank * PARAM_COUNT + j]);
        const uint64_t cur = a[r * PARAM_COUNT + j];
        a[r * PARAM_COUNT + j] = (cur >= sub) ? cur - sub : cur + PRIME - sub;
      }
    }

    rank++;
  }

  return rank;
}


std::string describe(const std::vector<std::pair<size_t, int64_t>>& v)
{
  std::string out;

  for (const auto& [i, c] : v) {
    out += (c < 0) ? " -" : " +";
    if (c != 1 && c != -1) { out += std::to_string((c < 0) ? -c : c) + " "; }
    out += column_name(i);
  }

  return out;
}


// The identity report. What S100 proved for two columns, asked of all of them:
// the rank of the design over the whole corpus, the dependencies the
// parameterisation has by construction checked exactly, and whatever is left
// after those found and printed rather than argued about.
void report_identities(const dataset_t& data,
                       size_t sample,
                       const std::vector<uint8_t>& mask,
                       const std::string& free_name)
{
  // A nonlinear group's per-side entries have no column in a linear design.
  // load() refuses them while no group is declared (tools/tuner_trace.hpp,
  // group_of_term()), so this is the day one is: the rank of the linear part
  // alone would be printed as the rank of the whole.
  if (!data.side_entries.empty()) {
    printf(
        "identities: not computed: the trace carries %zu per-side entries, "
        "which a linear design has no column for\n",
        data.side_entries.size());
    return;
  }

  const unsigned threads = std::max(1u, std::thread::hardware_concurrency());

  // X'X, upper triangle per thread, summed exactly.
  std::vector<std::vector<int64_t>> partial(threads);
  std::vector<double> worst(threads, 0.0);
  std::vector<size_t> compared(threads, 0);
  std::vector<std::thread> workers;

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  const size_t stride = (data.size() > sample) ? data.size() / sample : 1;

  for (unsigned t = 0; t < threads; ++t) {
    workers.emplace_back([&, t]() {
      std::vector<int64_t>& gram = partial[t];
      gram.assign(PARAM_COUNT * PARAM_COUNT, 0);
      design_builder_t builder;
      design_row_t x;

      for (size_t row = t; row < data.size(); row += threads) {
        builder.build(data, row, &x);

        const size_t n = x.index.size();
        for (size_t a = 0; a < n; ++a) {
          int64_t* line = &gram[x.index[a] * PARAM_COUNT];
          const int64_t va = x.value[a];

          for (size_t b = a; b < n; ++b) {
            line[x.index[b]] += va * x.value[b];
          }
        }

        // The design is the score's own: on a row where the clamp does not
        // bind, x . params / 24 is the trace's smooth score itself, which
        // sums the same entries by site rather than by parameter.
        if (row % stride != 0) { continue; }

        double total = 0.0;
        double stage_two = 0.0;
        for (size_t a = 0; a < n; ++a) {
          const double term = static_cast<double>(x.value[a]) *
                              params[x.index[a]] / GAME_PHASE_MAX;
          total += term;
          if (x.index[a] >= MOB_MG_BASE && x.index[a] < PP_MG_BASE) {
            stage_two += term;
          }
        }

        if (std::fabs(stage_two) >= LAZY_EVAL_MARGIN) { continue; }

        const double smooth = tuner_trace::score(data, row, params.data());
        worst[t] = std::max(worst[t], std::fabs(smooth - total));
        compared[t]++;
      }
    });
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  std::vector<int64_t> gram(PARAM_COUNT * PARAM_COUNT, 0);
  for (const std::vector<int64_t>& part : partial) {
    for (size_t i = 0; i < gram.size(); ++i) {
      gram[i] += part[i];
    }
  }
  for (size_t i = 0; i < PARAM_COUNT; ++i) {
    for (size_t j = 0; j < i; ++j) {
      gram[i * PARAM_COUNT + j] = gram[j * PARAM_COUNT + i];
    }
  }

  double design_worst = 0.0;
  size_t design_rows = 0;
  for (unsigned t = 0; t < threads; ++t) {
    design_worst = std::max(design_worst, worst[t]);
    design_rows += compared[t];
  }

  printf(
      "identities: the design the tuner descends, %zu parameters over %zu "
      "rows\n",
      PARAM_COUNT, data.size());
  printf(
      "identities: method: each row is evaluate()'s trace, each coefficient "
      "times its taper weight times %d (%d for a piece value, the phase and "
      "%d minus it for a tapered pair's halves), which is %d x d(score)/"
      "d(param) with the stage-two clamp left out; X'X summed exactly in "
      "64-bit integers over every row; rank by Gauss-Jordan modulo 2^61-1, "
      "which cannot exceed the rank over the rationals; each dependency "
      "checked exactly as v'X'Xv == 0, the sum over every row of (x.v)^2\n",
      GAME_PHASE_MAX, GAME_PHASE_MAX, GAME_PHASE_MAX, GAME_PHASE_MAX);
  printf(
      "identities: design check: x.params/%d against the trace's smooth "
      "score on %zu rows where the clamp does not bind, worst |difference| "
      "%.3g\n",
      GAME_PHASE_MAX, design_rows, design_worst);

  // Columns no row ever touches. A pawn cannot stand on either back rank, so
  // thirty-two of these are there by construction; any other is a gap in the
  // corpus, and either way the fit has nothing to identify it by.
  std::vector<uint8_t> dropped(PARAM_COUNT, 0);
  std::string zero_names;
  size_t zero_count = 0;

  for (size_t i = 0; i < PARAM_COUNT; ++i) {
    if (gram[i * PARAM_COUNT + i] != 0) { continue; }

    dropped[i] = 1;
    zero_count++;
    zero_names += " " + column_name(i);
  }

  printf("identities: %zu columns zero on every row:%s\n", zero_count,
         zero_names.c_str());

  // The dependencies the parameterisation has by construction. Material and a
  // piece-square table: adding c to every square of both tables of a type and
  // subtracting it from that type's value is the same evaluation. The king: one
  // a side, so each of its tables sums to zero across the board.
  struct known_t
  {
    std::string what;
    std::vector<std::pair<size_t, int64_t>> vector;
    size_t drop;  // the column the reduced design leaves out for it
  };

  std::vector<known_t> known;

  for (size_t type = 0; type < 6; ++type) {
    known_t k;

    if (type < MATERIAL_COUNT) {
      const std::string name = column_name(MG_BASE + type * 64);
      const std::string piece = name.substr(8, name.find(']') - 8);
      k.what = "material[" + piece + "] = the sum of every psqt_mg[" + piece +
               "] and psqt_eg[" + piece + "] column";
      k.vector.push_back({type, -1});
      k.drop = type;
    } else {
      k.what = "sum of psqt_mg[king] = 0";
      k.drop = PARAM_COUNT;
    }

    for (size_t s = 0; s < 64; ++s) {
      const size_t mg = MG_BASE + type * 64 + s;
      if (!dropped[mg]) {
        k.vector.push_back({mg, 1});
        if (k.drop == PARAM_COUNT) { k.drop = mg; }
      }
    }

    if (type < MATERIAL_COUNT) {
      for (size_t s = 0; s < 64; ++s) {
        const size_t eg = EG_BASE + type * 64 + s;
        if (!dropped[eg]) { k.vector.push_back({eg, 1}); }
      }
    }

    known.push_back(k);

    if (type == 5) {
      known_t e;
      e.what = "sum of psqt_eg[king] = 0";
      e.drop = PARAM_COUNT;
      for (size_t s = 0; s < 64; ++s) {
        const size_t eg = EG_BASE + type * 64 + s;
        if (!dropped[eg]) {
          e.vector.push_back({eg, 1});
          if (e.drop == PARAM_COUNT) { e.drop = eg; }
        }
      }
      known.push_back(e);
    }
  }

  // S121's eight: every piece of type t adds one cell of its mobility row, so
  // in each half the row's cells summed and the type's table squares summed
  // carry the same coefficient on every row, White's pieces of that type
  // minus Black's. Adding c to every cell and taking c off every square of
  // the half's table is the same linear score; the clamp over stage two is
  // the only thing that tells them apart, and the design leaves it out. One
  // per piece type with a mobility row and half: knight, bishop, rook, queen.
  for (int type = 0; type < 4; ++type) {
    for (const bool mg : {true, false}) {
      known_t k;
      const std::string name =
          column_name((mg ? MOB_MG_BASE : MOB_EG_BASE) +
                      static_cast<size_t>(MOBILITY_OFFSET[type]));
      const std::string half = mg ? "mg" : "eg";
      const std::string piece = name.substr(12, name.find(']') - 12);
      k.what = "the sum of every mobility_" + half + "[" + piece +
               "] cell = the sum of every psqt_" + half + "[" + piece +
               "] column (S121)";
      k.drop = PARAM_COUNT;

      for (int c = 0; c < MOBILITY_WIDTH[type]; ++c) {
        const size_t cell = (mg ? MOB_MG_BASE : MOB_EG_BASE) +
                            static_cast<size_t>(MOBILITY_OFFSET[type] + c);
        if (!dropped[cell]) {
          k.vector.push_back({cell, 1});
          if (k.drop == PARAM_COUNT) { k.drop = cell; }
        }
      }

      // The piece-square table of the same piece: psqt type = mobility type
      // + 1, the pawn having no mobility row.
      for (size_t s = 0; s < 64; ++s) {
        const size_t column =
            (mg ? MG_BASE : EG_BASE) + static_cast<size_t>(type + 1) * 64 + s;
        if (!dropped[column]) { k.vector.push_back({column, -1}); }
      }

      known.push_back(k);
    }
  }

  size_t known_held = 0;

  for (const known_t& k : known) {
    const bool holds = quadratic_form(gram, k.vector) == 0;

    printf("identities: by construction, %s: %s\n", k.what.c_str(),
           holds ? "holds on every row" : "FAILS");

    if (holds && k.drop < PARAM_COUNT) {
      dropped[k.drop] = 1;
      known_held++;
    }
  }

  std::vector<size_t> all(PARAM_COUNT);
  std::vector<size_t> keep;
  for (size_t i = 0; i < PARAM_COUNT; ++i) {
    all[i] = i;
    if (!dropped[i]) { keep.push_back(i); }
  }

  const size_t full_rank = rank_mod_p(gram, all, nullptr);

  printf(
      "identities: rank modulo 2^61-1: %zu of %zu, nullity %zu; %zu zero "
      "columns and %zu dependencies by construction account for %zu of it\n",
      full_rank, PARAM_COUNT, PARAM_COUNT - full_rank, zero_count, known_held,
      zero_count + known_held);

  std::vector<std::vector<std::pair<size_t, int64_t>>> null;
  const size_t reduced_rank = rank_mod_p(gram, keep, &null);

  printf(
      "identities: the %zu columns left once each of those drops one: rank "
      "%zu, %zu further exact dependencies\n",
      keep.size(), reduced_rank, null.size());

  for (size_t d = 0; d < null.size(); ++d) {
    bool small = true;
    for (const auto& entry : null[d]) {
      if (entry.second > 1000000 || entry.second < -1000000) { small = false; }
    }

    const char* verdict = !small ? "coefficients not small integers, unverified"
                          : quadratic_form(gram, null[d]) == 0
                              ? "holds on every row"
                              : "FAILS over the rationals";

    printf("identities: dependency %zu: 0 =%s: %s\n", d + 1,
           describe(null[d]).c_str(), verdict);
  }

  if (null.empty()) {
    printf("identities: no exact dependency beyond the ones listed above\n");
  }

  // The rank over the rationals, from both sides. Below: the rank modulo the
  // prime, which a reduction can only lower. Above: every null vector that
  // holds exactly on every row, provided they are linearly independent -- each
  // one of them takes a dimension off the rank.
  std::vector<std::vector<std::pair<size_t, int64_t>>> exact;
  size_t found_held = 0;

  for (size_t i = 0; i < PARAM_COUNT; ++i) {
    if (gram[i * PARAM_COUNT + i] == 0) { exact.push_back({{i, 1}}); }
  }
  for (const known_t& k : known) {
    if (quadratic_form(gram, k.vector) == 0) { exact.push_back(k.vector); }
  }
  for (const auto& v : null) {
    bool small = true;
    for (const auto& entry : v) {
      if (entry.second > 1000000 || entry.second < -1000000) { small = false; }
    }

    if (small && quadratic_form(gram, v) == 0) {
      exact.push_back(v);
      found_held++;
    }
  }

  std::vector<uint8_t> claimed(PARAM_COUNT, 0);
  bool disjoint = true;

  for (const auto& v : exact) {
    for (const auto& entry : v) {
      if (claimed[entry.first]) { disjoint = false; }
      claimed[entry.first] = 1;
    }
  }

  const size_t independent = rank_of_vectors(exact);
  const size_t upper = PARAM_COUNT - independent;

  printf(
      "identities: upper bound: %zu null vectors hold exactly on every row "
      "(%zu zero columns, %zu by construction, %zu found); their supports are "
      "%s and %zu of them are linearly independent, so the rank over the "
      "rationals is at most %zu - %zu = %zu\n",
      exact.size(), zero_count, exact.size() - zero_count - found_held,
      found_held, disjoint ? "pairwise disjoint" : "not disjoint", independent,
      PARAM_COUNT, independent, upper);
  printf(
      "identities: lower bound: the rank modulo 2^61-1 is %zu, and reducing an "
      "integer matrix modulo a prime cannot raise its rank, so the rank over "
      "the rationals is at least %zu\n",
      full_rank, full_rank);

  if (full_rank == upper) {
    printf(
        "identities: rank over the rationals exactly %zu of %zu, nullity %zu, "
        "every null direction listed above\n",
        full_rank, PARAM_COUNT, PARAM_COUNT - full_rank);
  } else {
    printf(
        "identities: rank over the rationals between %zu and %zu: a null "
        "direction is unaccounted for\n",
        full_rank, upper);
  }

  // What the fit does about them, S275. Each gauge tools/tuner_groups.hpp holds
  // has to be null on every row -- a hold that moved one row's evaluation would
  // be a pin that changes the fit -- and the gauges with the zero columns,
  // which Adam never moves, have to span the whole null space, or the fit is
  // free to walk along a direction nobody holds. A new term defined on a single
  // rank fails the second line before it is fitted.
  std::vector<std::vector<std::pair<size_t, int64_t>>> pinned;
  size_t gauges_null = 0;

  for (size_t i = 0; i < PARAM_COUNT; ++i) {
    if (gram[i * PARAM_COUNT + i] == 0) { pinned.push_back({{i, 1}}); }
  }

  for (const tuner_groups::gauge_t& gauge : tuner_groups::gauges()) {
    std::vector<std::pair<size_t, int64_t>> v;
    if (gauge.absorb < PARAM_COUNT) { v.push_back({gauge.absorb, 1}); }
    for (const size_t column : gauge.level) {
      v.push_back({column, -1});
    }

    const bool holds = quadratic_form(gram, v) == 0;
    printf("identities: the tuner holds %s: %s\n", gauge.name.c_str(),
           holds ? "null on every row" : "FAILS, the hold moves a row");

    if (holds) {
      pinned.push_back(v);
      gauges_null++;
    }
  }

  // S121's eight mobility levels are no gauge: they are null only where the
  // clamp does not bind, so a hold would move the rows where it does. A level
  // is held when the audited free set freezes either side of it -- its row's
  // cells or its piece's table half -- since no free parameter can then move
  // along it; every S121 fit is `--only mobility`, the tables frozen. A level
  // free on both sides (S126's `--only all`) is a direction nothing holds and
  // fails the span below, as S275's walk did before its gauges, and the tuner
  // refuses such a mask (tools/tuner_groups.hpp, mobility_levels_held()).
  size_t levels_frozen = 0;

  for (const known_t& k : known) {
    if (k.what.find("(S121)") == std::string::npos) { continue; }
    if (quadratic_form(gram, k.vector) != 0) { continue; }

    bool cells_free = false;
    bool table_free = false;
    for (const auto& [column, sign] : k.vector) {
      if (!mask[column]) { continue; }
      if (sign > 0) {
        cells_free = true;
      } else {
        table_free = true;
      }
    }

    if (cells_free && table_free) {
      printf(
          "identities: mobility level, %s: free on both sides under --free "
          "%s, and no gauge holds it; a fit freeing both must hold or freeze "
          "it\n",
          k.what.c_str(), free_name.c_str());
      continue;
    }

    printf(
        "identities: mobility level, %s: held by the freeze under --free %s "
        "(its %s are frozen); a fit freeing both must hold or freeze it\n",
        k.what.c_str(), free_name.c_str(),
        table_free ? "cells" : "table squares");
    pinned.push_back(k.vector);
    levels_frozen++;
  }

  const size_t spanned = rank_of_vectors(pinned);
  const size_t nullity_bound = PARAM_COUNT - full_rank;

  printf(
      "identities: %zu zero columns, %zu held gauges and %zu frozen mobility "
      "levels span %zu of at most %zu null dimensions: %s\n",
      zero_count, gauges_null, levels_frozen, spanned, nullity_bound,
      (spanned != nullity_bound)
          ? "a null direction the tuner does not hold, pin it or measure it "
            "before a fit"
      : (levels_frozen == 0)
          ? "every null direction is one Adam never moves or one the tuner "
            "holds"
          : "every null direction is one Adam never moves, one the tuner "
            "holds, or a mobility level the run's frozen columns hold");
}


void usage()
{
  printf(
      "usage: feature_audit --data FILE [--plant FILE] [--plant-k K]\n"
      "                     [--plant-group "
      "all|passed_pawns|piece_placement|tempo]\n"
      "                     [--sample N] [--free GROUP] [--no-columns]\n"
      "                     [--no-identities] [--no-occurrence] "
      "[--no-collinearity]\n"
      "  --free GROUP   the tuner `--only` group the identity report is read\n"
      "                 for (default all), or a comma-separated list of\n"
      "                 them, their union (S123); S121's eight mobility\n"
      "                 levels are held only where it freezes one side of\n"
      "                 each\n");
}

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    const bool has_value = (i + 1 < argc);

    if (arg == "--data" && has_value) {
      opts.data = argv[++i];
    } else if (arg == "--plant" && has_value) {
      opts.plant = argv[++i];
    } else if (arg == "--plant-group" && has_value) {
      opts.plant_group = argv[++i];
    } else if (arg == "--plant-k" && has_value) {
      opts.plant_k = atof(argv[++i]);
    } else if (arg == "--sample" && has_value) {
      opts.sample = static_cast<size_t>(atoll(argv[++i]));
    } else if (arg == "--free" && has_value) {
      opts.free_group = argv[++i];
    } else if (arg == "--no-columns") {
      opts.columns = false;
    } else if (arg == "--no-identities") {
      opts.identities = false;
    } else if (arg == "--no-occurrence") {
      opts.occurrence = false;
    } else if (arg == "--no-collinearity") {
      opts.collinearity = false;
    } else {
      usage();
      return 1;
    }
  }

  if (opts.data.empty()) {
    usage();
    return 1;
  }

  // Every row through evaluate() on the trace build, on every thread, each
  // checked against evaluate() as the tuner checks it. S276.
  dataset_t data;
  tuner_trace::load_report_t trace;
  std::string load_error;
  const unsigned threads = std::max(1u, std::thread::hardware_concurrency());

  if (!tuner_trace::load(opts.data, &data, &trace, &load_error, threads)) {
    fprintf(stderr, "could not load %s: %s\n", opts.data.c_str(),
            load_error.c_str());
    return 1;
  }

  printf("corpus %s, %zu rows\n", opts.data.c_str(), data.size());
  printf("trace: %" PRIu64 " rows, %" PRIu64 " entries; %" PRIu64
         " differ from evaluate(), the clamp binds on %" PRIu64 "\n",
         trace.rows, trace.entries, trace.mismatches, trace.clamped);

  // What every report below reads is the trace, so a corpus on which the trace
  // is not evaluate() is not audited at all -- the tuner refuses the same one.
  if (trace.mismatches > 0) {
    fprintf(stderr,
            "the trace disagrees with evaluate() on %" PRIu64
            " rows, the first: %s\nnothing audited\n",
            trace.mismatches, trace.first_mismatch.c_str());
    return 1;
  }

  // --------------------------------------------------------------- columns
  //
  // The FEN text has to be read a second time for this, because load() keeps
  // the trace and not the string it came from. One game and one row at a time,
  // in file order: the shape load() does not have, so a row it put in the
  // wrong slot cannot agree with this by sharing the mistake.
  if (opts.columns) {
    std::ifstream file(opts.data);
    std::string line;
    size_t row = 0;
    size_t disagreements = 0;
    size_t phase_disagreements = 0;

    game_t game;
    initialize_game_const_data(&game);

    while (std::getline(file, line)) {
      if (line.empty()) { continue; }

      if (row >= data.size()) {
        row++;
        continue;
      }

      std::istringstream fields(line);
      std::string fen, result_text, score_text, phase_text;
      std::getline(fields, fen, '\t');
      std::getline(fields, result_text, '\t');
      std::getline(fields, score_text, '\t');
      std::getline(fields, phase_text, '\t');

      // load() tapers on the engine's phase and reads the stored column for its
      // range only (2026-09-10_adversarial-F36), so the two disagreeing is not
      // a defect in the fit; it is what a corpus written under another
      // phase_value looks like, and worth knowing about.
      if (atoi(phase_text.c_str()) != data.phase[row]) {
        phase_disagreements++;
      }

      tuner_trace::row_trace_t again;
      std::string why;

      if (!load_FEN(fen, &game) ||
          !tuner_trace::trace_board(&game.board, &again, &why) ||
          !row_is(data, row, again)) {
        disagreements++;
      }

      row++;
    }

    printf("columns: %zu rows traced again one at a time, %zu disagree\n", row,
           disagreements);
    printf(
        "columns: stored phase column against the engine's phase: %zu "
        "disagreements\n",
        phase_disagreements);

    if (row != data.size()) {
      printf("columns: FAIL row count %zu against %zu loaded\n", row,
             data.size());
    }
  }

  // ------------------------------------------------------------ identities
  if (opts.identities) {
    // A list is the union, as `tuner --only` reads one since S123.
    std::vector<uint8_t> mask;
    if (!tuner_groups::only_mask(opts.free_group, &mask)) {
      fprintf(stderr, "unknown --free group '%s'; groups are: %s\n",
              opts.free_group.c_str(), tuner_groups::GROUP_LIST);
      return 1;
    }
    report_identities(data, opts.sample, mask, opts.free_group);
  }

  // ----------------------------------------------------------- occurrence
  if (opts.occurrence) {
    // Seven bands rather than a histogram of 25: the taper is what the bands
    // are about, and phase 0 is its own band because mg_weight is exactly zero
    // there and a middlegame weight gets no gradient at all from those rows.
    struct band_t
    {
      const char* name;
      int lo;
      int hi;
    };

    static const band_t bands[] = {
        {"0", 0, 0},       {"1-4", 1, 4},     {"5-8", 5, 8},    {"9-12", 9, 12},
        {"13-16", 13, 16}, {"17-20", 17, 20}, {"21-24", 21, 24}};
    constexpr size_t BAND_COUNT = sizeof(bands) / sizeof(bands[0]);

    // A column is one feature of the trace: a row's coefficient for it, the
    // White-minus-Black count its weight multiplies.
    struct column_t
    {
      const char* name;
      uint32_t feature;
    };

    std::vector<column_t> columns;

    for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
      columns.push_back(
          {PASSER_NAMES[i], feature_of(eval_trace::TERM_PASSED_PAWN, i)});
    }
    for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
      columns.push_back({PLACEMENT_NAMES[i],
                         feature_of(eval_trace::TERM_PIECE_PLACEMENT, i)});
    }

    // S123's family and S125's after it, one column a cell; named by the
    // middlegame half's parameter, which is the cell's own name less the half.
    std::vector<std::string> family_names;
    for (uint32_t f =
             tuner_trace::feature_table().base[eval_trace::TERM_PASSED_FREE];
         f < tuner_trace::feature_table().base[eval_trace::TERM_COUNT]; ++f) {
      std::string name =
          column_name(tuner_trace::feature_table().features[f].mg);
      name.replace(name.find("_mg["), 4, "[");
      family_names.push_back(name);
    }
    {
      const uint32_t first =
          tuner_trace::feature_table().base[eval_trace::TERM_PASSED_FREE];
      for (size_t i = 0; i < family_names.size(); ++i) {
        columns.push_back(
            {family_names[i].c_str(), first + static_cast<uint32_t>(i)});
      }
    }

    size_t band_rows[BAND_COUNT] = {};
    for (size_t row = 0; row < data.size(); ++row) {
      for (size_t b = 0; b < BAND_COUNT; ++b) {
        if (data.phase[row] >= bands[b].lo && data.phase[row] <= bands[b].hi) {
          band_rows[b]++;
        }
      }
    }

    printf("\noccurrence: percent of rows with a non-zero differential\n");
    printf("%-22s %8s", "column", "all");
    for (size_t b = 0; b < BAND_COUNT; ++b) {
      printf(" %8s", bands[b].name);
    }
    printf("\n");
    printf("%-22s %8.2f", "rows (millions)",
           static_cast<double>(data.size()) / 1e6);
    for (size_t b = 0; b < BAND_COUNT; ++b) {
      printf(" %8.2f", static_cast<double>(band_rows[b]) / 1e6);
    }
    printf("\n");

    for (const column_t& column : columns) {
      size_t all = 0;
      size_t per_band[BAND_COUNT] = {};

      for (size_t row = 0; row < data.size(); ++row) {
        if (coefficient(data, row, column.feature) == 0) { continue; }

        all++;

        for (size_t b = 0; b < BAND_COUNT; ++b) {
          if (data.phase[row] >= bands[b].lo &&
              data.phase[row] <= bands[b].hi) {
            per_band[b]++;
          }
        }
      }

      printf(
          "%-22s %8.2f", column.name,
          100.0 * static_cast<double>(all) / static_cast<double>(data.size()));
      for (size_t b = 0; b < BAND_COUNT; ++b) {
        const double share = (band_rows[b] > 0)
                                 ? 100.0 * static_cast<double>(per_band[b]) /
                                       static_cast<double>(band_rows[b])
                                 : 0.0;
        printf(" %8.2f", share);
      }
      printf("\n");
    }

    // S121's support report: per mobility cell and half, the rows on which the
    // cell's coefficient is non-zero and the half's taper weight is too --
    // phase above 0 for the middlegame half, below 24 for the endgame one --
    // and how many games those rows come from, games being where the labels
    // are independent (S066's boundaries, tools/tuner_split.hpp). A half with
    // no such row is a zero column of the identity report. One line a column,
    // `support <name> rows N games G`, which adocs/data/S121_screen.py ties
    // reads to tie the cells under its pre-registered threshold. S123 reads
    // the same lines for its family's cells (adocs/data/S123_screen.py ties),
    // printed after the mobility block under a heading of their own, and
    // S125 for its family's (adocs/data/S125_screen.py ties) under another.
    {
      std::vector<uint32_t> starts;
      tuner_split::game_starts(data.ply, &starts);

      const auto support = [&](uint32_t first, uint32_t last,
                               const char* heading) {
        const size_t width = last - first;

        std::vector<uint64_t> rows(2 * width, 0);
        std::vector<uint64_t> games(2 * width, 0);
        std::vector<int64_t> seen(2 * width, -1);  // the last game counted
        size_t game = 0;

        for (size_t row = 0; row < data.size(); ++row) {
          while (game + 1 < starts.size() && starts[game + 1] <= row) {
            game++;
          }

          const bool has_mg = data.phase[row] > 0;
          const bool has_eg = data.phase[row] < GAME_PHASE_MAX;

          for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
            const tuner_trace::entry_t e = data.entries[j];
            if (e.feature < first || e.feature >= last || e.coef == 0) {
              continue;
            }

            const size_t cell = e.feature - first;
            for (size_t half = 0; half < 2; ++half) {
              if ((half == 0) ? !has_mg : !has_eg) { continue; }

              const size_t slot = half * width + cell;
              rows[slot]++;
              if (seen[slot] != static_cast<int64_t>(game)) {
                seen[slot] = static_cast<int64_t>(game);
                games[slot]++;
              }
            }
          }
        }

        printf(
            "\n%s support: rows with a non-zero cell coefficient and a "
            "non-zero taper weight for the half, and the games they come "
            "from, %zu games in all\n",
            heading, starts.size());

        const std::vector<tuner_trace::feature_t>& features =
            tuner_trace::feature_table().features;

        for (size_t half = 0; half < 2; ++half) {
          for (size_t cell = 0; cell < width; ++cell) {
            const size_t slot = half * width + cell;
            const tuner_trace::feature_t& f = features[first + cell];
            printf("support %s rows %" PRIu64 " games %" PRIu64 "\n",
                   column_name((half == 0) ? f.mg : f.eg).c_str(), rows[slot],
                   games[slot]);
          }
        }
      };

      support(feature_of(eval_trace::TERM_MOBILITY, 0),
              feature_of(eval_trace::TERM_MOBILITY,
                         static_cast<size_t>(MOBILITY_CELLS)),
              "mobility");
      support(
          tuner_trace::feature_table().base[eval_trace::TERM_PASSED_FREE],
          tuner_trace::feature_table().base[eval_trace::TERM_ISOLATED_BACKWARD],
          "passer family (S123)");
      support(
          tuner_trace::feature_table().base[eval_trace::TERM_ISOLATED_BACKWARD],
          tuner_trace::feature_table().base[eval_trace::TERM_COUNT],
          "structure family (S125)");
    }

    // Tempo is not a count and has no zero case; what it has is a label-side
    // question. Its whole signal is the gap between the mean outcome of rows
    // with White to move and rows with Black to move, so that gap is the thing
    // to report. datagen skips a position whose side to move is in check, which
    // removes rows correlated with the mover standing worse -- a selection the
    // gap below is measured against and not corrected for.
    size_t white_rows = 0;
    size_t black_rows = 0;
    double white_result = 0.0;
    double black_result = 0.0;

    for (size_t row = 0; row < data.size(); ++row) {
      if (data.stm[row] > 0) {
        white_rows++;
        white_result += data.result[row];
      } else {
        black_rows++;
        black_result += data.result[row];
      }
    }

    printf("\ntempo: %zu rows White to move, mean result %.6f\n", white_rows,
           (white_rows > 0) ? white_result / static_cast<double>(white_rows)
                            : 0.0);
    printf("tempo: %zu rows Black to move, mean result %.6f\n", black_rows,
           (black_rows > 0) ? black_result / static_cast<double>(black_rows)
                            : 0.0);
    printf(
        "tempo: gap %.6f, White relative\n",
        ((white_rows > 0) ? white_result / static_cast<double>(white_rows)
                          : 0.0) -
            ((black_rows > 0) ? black_result / static_cast<double>(black_rows)
                              : 0.0));
  }

  // --------------------------------------------------------- collinearity
  if (opts.collinearity) {
    const size_t stride =
        (data.size() > opts.sample) ? (data.size() / opts.sample) : 1;

    std::vector<uint32_t> rows;
    for (size_t row = 0; row < data.size(); row += stride) {
      rows.push_back(static_cast<uint32_t>(row));
    }

    printf(
        "\ncollinearity: R^2 of a term column on the piece-square columns "
        "it overlaps, %zu rows (every %zuth)\n",
        rows.size(), stride);

    // The types whose tables could absorb the column, which is not always the
    // one type the feature is named after. A rook on an open file is a
    // statement about a rook *and* about the pawns on its file, and both are
    // priced by a table, so the honest question puts 128 columns on the right
    // hand side and not 64. The passer buckets are the opposite case and stay
    // at pawns alone: a passer depends on the enemy pawns ahead of it, and the
    // signed occupancy column cannot separate a White pawn on a square from a
    // Black one mirrored onto it -- which is the point, because the model
    // cannot either. What is being asked throughout is whether the tables *as
    // the fit sees them* can express the column.
    struct target_t
    {
      const char* name;
      std::vector<size_t> psqt_types;
      uint32_t feature;
    };

    std::vector<target_t> targets = {
        {"bishop pair",
         {BISHOP_TYPE},
         feature_of(eval_trace::TERM_PIECE_PLACEMENT, PL_BISHOP_PAIR)},
        {"rook open",
         {ROOK_TYPE, PAWN_TYPE},
         feature_of(eval_trace::TERM_PIECE_PLACEMENT, PL_ROOK_OPEN)},
        {"rook half-open",
         {ROOK_TYPE, PAWN_TYPE},
         feature_of(eval_trace::TERM_PIECE_PLACEMENT, PL_ROOK_HALF_OPEN)},
        // S135's four, each against the tables of every piece type it names:
        // a bishop and its pawns; a trapped knight or bishop and the enemy
        // pawns that trap it; a rook and its king.
        {"bad bishop",
         {BISHOP_TYPE, PAWN_TYPE},
         feature_of(eval_trace::TERM_PIECE_PLACEMENT, PL_BAD_BISHOP)},
        {"bishop rammed",
         {BISHOP_TYPE, PAWN_TYPE},
         feature_of(eval_trace::TERM_PIECE_PLACEMENT, PL_BAD_BISHOP_RAMMED)},
        {"trapped minor",
         {KNIGHT_TYPE, BISHOP_TYPE, PAWN_TYPE},
         feature_of(eval_trace::TERM_PIECE_PLACEMENT, PL_TRAPPED_MINOR)},
        {"trapped rook",
         {ROOK_TYPE, KING_TYPE},
         feature_of(eval_trace::TERM_PIECE_PLACEMENT, PL_TRAPPED_ROOK)},
    };

    for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
      targets.push_back({PASSER_NAMES[i],
                         {PAWN_TYPE},
                         feature_of(eval_trace::TERM_PASSED_PAWN, i)});
    }

    // One occupancy block per piece type, built once and shared.
    std::vector<occupancy_t> occupancy(6);
    for (const target_t& target : targets) {
      for (const size_t type : target.psqt_types) {
        if (occupancy[type].value.empty()) {
          occupancy[type] = occupancy_of_type(data, type, rows);
        }
      }
    }

    for (const target_t& target : targets) {
      std::vector<double> y(rows.size());
      for (size_t r = 0; r < rows.size(); ++r) {
        y[r] = coefficient(data, rows[r], target.feature);
      }

      std::vector<std::vector<double>> columns;
      std::string right_hand;

      for (const size_t type : target.psqt_types) {
        const occupancy_t& block = occupancy[type];

        for (size_t square = 0; square < 64; ++square) {
          std::vector<double> column(rows.size());
          for (size_t r = 0; r < rows.size(); ++r) {
            column[r] = block.value[r * 64 + square];
          }
          columns.push_back(std::move(column));
        }

        if (!right_hand.empty()) { right_hand += "+"; }
        right_hand += (type == PAWN_TYPE)     ? "pawn"
                      : (type == KNIGHT_TYPE) ? "knight"
                      : (type == BISHOP_TYPE) ? "bishop"
                      : (type == ROOK_TYPE)   ? "rook"
                                              : "king";
      }

      printf("%-18s R^2 %.6f  on %s psqt, %zu columns\n", target.name,
             r_squared(y, columns), right_hand.c_str(), columns.size());
    }
  }

  // ---------------------------------------------------------------- plant
  //
  // The planted weights. Every one is written here, chosen to be well clear of
  // the rounding lround() applies when a fit is emitted and distinguishable
  // from its neighbours, and derived from nothing outside this repository --
  // their only job is to be a vector a correct fit gets back.
  if (!opts.plant.empty()) {
    std::vector<double> params(PARAM_COUNT, 0.0);
    tuner_layout::starting_params(params.data());

    // S134 dropped the planted bucket 5 (66, 95) and rook seventh (13, 22)
    // with the columns they planted; the rest are the numbers S100 planted.
    static const double planted_passed_mg[PASSED_PAWN_COUNT] = {7, 14, 21, 33,
                                                                48};
    static const double planted_passed_eg[PASSED_PAWN_COUNT] = {11, 19, 30, 45,
                                                                70};
    // S135's four columns planted beside S100's three, so a recovery run on
    // the widened group has a number to get back for every column it frees.
    static const double planted_placement_mg[PIECE_PLACEMENT_COUNT] = {
        28, 17, 9, -3, -6, -31, -14};
    static const double planted_placement_eg[PIECE_PLACEMENT_COUNT] = {
        41, 8, 5, -7, -4, -26, -9};
    static const double planted_tempo_mg = 23;
    static const double planted_tempo_eg = 12;

    // Which groups the plant moves, because it decides what a recovery run can
    // prove. `--only GROUP` holds every other parameter at what the engine
    // ships, so a plant that moved three groups and a fit that frees one leaves
    // the freed group absorbing the other two -- a real disagreement with the
    // planted vector that says nothing about the pipeline. Planting one group
    // and fitting exactly that group is the only shape whose prediction is
    // "recovers the planted number", full stop.
    const bool all = (opts.plant_group == "all");
    const bool plant_passed = all || opts.plant_group == "passed_pawns";
    const bool plant_placement = all || opts.plant_group == "piece_placement";
    const bool plant_tempo = all || opts.plant_group == "tempo";

    if (!plant_passed && !plant_placement && !plant_tempo) {
      fprintf(stderr, "unknown --plant-group %s\n", opts.plant_group.c_str());
      return 1;
    }

    if (plant_passed) {
      for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
        params[PP_MG_BASE + i] = planted_passed_mg[i];
        params[PP_EG_BASE + i] = planted_passed_eg[i];
      }
    }
    if (plant_placement) {
      for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
        params[PL_MG_BASE + i] = planted_placement_mg[i];
        params[PL_EG_BASE + i] = planted_placement_eg[i];
      }
    }
    if (plant_tempo) {
      params[TEMPO_MG_BASE] = planted_tempo_mg;
      params[TEMPO_EG_BASE] = planted_tempo_eg;
    }

    printf(
        "\nplant: group %s, K %.6f, labels are sigmoid(K * the trace's smooth "
        "score)\n",
        opts.plant_group.c_str(), opts.plant_k);
    // Only what was actually planted is printed. A log naming a weight the run
    // held at zero is a log that cannot be read back as the recovery target.
    if (plant_passed) {
      printf("plant: passed_pawn_mg {%g, %g, %g, %g, %g}\n",
             planted_passed_mg[0], planted_passed_mg[1], planted_passed_mg[2],
             planted_passed_mg[3], planted_passed_mg[4]);
      printf("plant: passed_pawn_eg {%g, %g, %g, %g, %g}\n",
             planted_passed_eg[0], planted_passed_eg[1], planted_passed_eg[2],
             planted_passed_eg[3], planted_passed_eg[4]);
    }
    if (plant_placement) {
      for (const bool mg : {true, false}) {
        const double* planted =
            mg ? planted_placement_mg : planted_placement_eg;

        printf("plant: piece_placement_%s {", mg ? "mg" : "eg");
        for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
          printf("%s%g", (i == 0) ? "" : ", ", planted[i]);
        }
        printf("}\n");
      }
    }
    if (plant_tempo) {
      printf("plant: tempo_mg %g, tempo_eg %g\n", planted_tempo_mg,
             planted_tempo_eg);
    }
    printf("plant: every other parameter is what the engine ships\n");

    // The FEN, the score and the phase are copied through verbatim so that only
    // the label differs: a synthetic corpus whose features moved would be
    // testing two things at once. The label's score is the one the tuner
    // descends: the trace's, every taper exact, the clamp kept.
    std::ifstream in(opts.data);
    std::ofstream out(opts.plant);

    if (!out) {
      fprintf(stderr, "could not write %s\n", opts.plant.c_str());
      return 1;
    }

    std::string line;
    size_t row = 0;
    size_t written = 0;

    while (std::getline(in, line)) {
      if (line.empty()) { continue; }

      std::istringstream fields(line);
      std::string fen, result_text, score_text, phase_text;
      std::getline(fields, fen, '\t');
      std::getline(fields, result_text, '\t');
      std::getline(fields, score_text, '\t');
      std::getline(fields, phase_text, '\t');

      const double score = tuner_trace::score(data, row, params.data());
      const double label = tuner_target::sigmoid(score, opts.plant_k);

      out << fen << '\t' << label << '\t' << score_text << '\t' << phase_text
          << '\n';

      row++;
      written++;
    }

    printf("plant: %zu rows written to %s\n", written, opts.plant.c_str());
  }

  return 0;
}
