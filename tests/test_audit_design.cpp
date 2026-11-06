#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include "audit_design.hpp"
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "test_endgame_positions.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

// tools/feature_audit's design (tools/audit_design.hpp), S124's phase 2a. At
// S122's shipped curve each king has its own x, and the audit reads the design
// linearised there, each side's own slope times its own counts. Two things are
// held here, each against something the design does not compute:
//
//   the local form is the score's derivative: every column of a row, over the
//     scale the design drops, against the trace's smooth score's central
//     difference at the shipped weights -- tools/tuner_trace.hpp's smooth(),
//     which the tuner descends and which never reads the design;
//   at the curve's seed the local form is the old one: where every side
//     shares x, the shared form's column times that x, column for column, so
//     the rank and the dependencies do not move; and the rank over the
//     curve's columns and the linear cells they duplicate there reads the
//     same in both forms.
//
// The scales are written here from the definitions, not read from the
// header: a piece value's or a tapered half's column is 24 times its
// derivative, a scale factor's 24 F, the curve's 24 divisor / 2 in the local
// form and 24 divisor / (2 x) in the shared one.

namespace
{

using audit_design::design_builder_t;
using audit_design::design_row_t;
using audit_design::group_slopes_t;
using tuner_layout::PARAM_COUNT;
using tuner_trace::dataset_t;

constexpr double H = 1e-3;
constexpr double CURVE_SCALE_LOCAL =
    static_cast<double>(GAME_PHASE_MAX) * KS_CURVE_DIVISOR / 2.0;


// The fixture: the curated list both ways round, the JSON suites, and S124's
// endgame population, so the curve, the cells and mop-up all have rows.
dataset_t fixture()
{
  game_t game;
  initialize_game_const_data(&game);

  std::vector<std::string> fens;
  for (const std::string& fen : test_eval_positions::positions) {
    fens.push_back(fen);
    fens.push_back(mirror_fen(fen));
  }
  for (const std::string& fen : all_test_fens()) {
    fens.push_back(fen);
  }
  for (const std::string& fen : test_endgame_positions::population()) {
    fens.push_back(fen);
  }

  dataset_t data;
  for (const std::string& fen : fens) {
    REQUIRE(load_FEN(fen, &game));
    tuner_trace::row_trace_t trace;
    std::string error;
    REQUIRE(tuner_trace::trace_board(&game.board, &trace, &error));
    tuner_trace::append_row(trace, 0.5f, 0, 0, &data);
  }
  return data;
}


bool curve_param(size_t p, bool* mg)
{
  if (p >= tuner_layout::KSC_MG_BASE && p < tuner_layout::KSC_EG_BASE) {
    *mg = true;
    return true;
  }
  if (p >= tuner_layout::KSC_EG_BASE && p < tuner_layout::KS_CURVE_END) {
    *mg = false;
    return true;
  }
  return false;
}

bool cell_param(size_t p)
{ return p >= tuner_layout::EGS_BASE && p < tuner_layout::ENDGAME_SCALE_END; }

double column_of(const design_row_t& x, size_t p)
{
  for (size_t a = 0; a < x.index.size(); ++a) {
    if (x.index[a] == p) { return static_cast<double>(x.value[a]); }
  }
  return 0.0;
}


// The curve's two sums for each side at `p`, from the side entries.
struct sums_t
{
  double mg[2] = {0.0, 0.0};
  double eg[2] = {0.0, 0.0};
};

sums_t sums_of(const dataset_t& data, size_t row, const double* p)
{
  sums_t s;
  for (uint32_t j = data.side_offsets[row]; j < data.side_offsets[row + 1];
       ++j) {
    const tuner_trace::side_entry_t e = data.side_entries[j];
    s.mg[e.side] += e.coef * p[tuner_layout::KSC_MG_BASE + e.input];
    s.eg[e.side] += e.coef * p[tuner_layout::KSC_EG_BASE + e.input];
  }
  return s;
}


// The 22 linear cells the curve's inputs but the constant count again,
// middlegame halves: the four attacker counts and the zone attacks of
// king_safety, the four checks, weak squares and defenders.
std::vector<size_t> duplicated_cells()
{
  using namespace tuner_layout;
  const size_t mg[KS_CURVE_INPUTS - 1] = {KS_MG_BASE + KS_KNIGHT_ATTACKERS,
                                          KS_MG_BASE + KS_BISHOP_ATTACKERS,
                                          KS_MG_BASE + KS_ROOK_ATTACKERS,
                                          KS_MG_BASE + KS_QUEEN_ATTACKERS,
                                          KS_MG_BASE + KS_ZONE_ATTACKS,
                                          KSK_MG_BASE + 0,
                                          KSK_MG_BASE + 1,
                                          KSK_MG_BASE + 2,
                                          KSK_MG_BASE + 3,
                                          KSW_MG_BASE,
                                          KSD_MG_BASE};
  const size_t width[KS_CURVE_INPUTS - 1] = {
      KING_SAFETY_COUNT, KING_SAFETY_COUNT, KING_SAFETY_COUNT,
      KING_SAFETY_COUNT, KING_SAFETY_COUNT, KS_CHECKS_COUNT,
      KS_CHECKS_COUNT,   KS_CHECKS_COUNT,   KS_CHECKS_COUNT,
      KS_WEAK_COUNT,     KS_DEFENDERS_COUNT};
  std::vector<size_t> out;
  for (size_t i = 0; i < KS_CURVE_INPUTS - 1; ++i) {
    out.push_back(mg[i]);
    out.push_back(mg[i] + width[i]);
  }
  return out;
}


// The rank modulo 2^61 - 1 of the design restricted to `columns`, over every
// row: its Gram matrix, summed in 128 bits, then Gauss-Jordan. Its own code,
// so the tool's elimination is not what checks the tool's design.
constexpr uint64_t PRIME = (uint64_t{1} << 61) - 1;

uint64_t mulmod(uint64_t a, uint64_t b)
{
  return static_cast<uint64_t>((static_cast<unsigned __int128>(a) * b) % PRIME);
}

uint64_t powmod(uint64_t a, uint64_t e)
{
  uint64_t r = 1;
  for (; e > 0; e >>= 1) {
    if (e & 1) { r = mulmod(r, a); }
    a = mulmod(a, a);
  }
  return r;
}

size_t rank_over(const dataset_t& data,
                 const std::vector<size_t>& columns,
                 const group_slopes_t& slopes,
                 const double* params)
{
  const size_t m = columns.size();
  std::vector<__int128> gram(m * m, 0);
  design_builder_t builder;
  design_row_t x;
  std::vector<int64_t> dense(m);

  for (size_t row = 0; row < data.size(); ++row) {
    builder.build(data, row, &x, slopes, params);
    for (size_t c = 0; c < m; ++c) {
      dense[c] = static_cast<int64_t>(column_of(x, columns[c]));
    }
    for (size_t a = 0; a < m; ++a) {
      if (dense[a] == 0) { continue; }
      for (size_t b = 0; b < m; ++b) {
        gram[a * m + b] += static_cast<__int128>(dense[a]) * dense[b];
      }
    }
  }

  std::vector<uint64_t> a(m * m);
  for (size_t i = 0; i < m * m; ++i) {
    __int128 r = gram[i] % static_cast<__int128>(PRIME);
    if (r < 0) { r += PRIME; }
    a[i] = static_cast<uint64_t>(r);
  }

  size_t rank = 0;
  for (size_t c = 0; c < m && rank < m; ++c) {
    size_t pivot = rank;
    while (pivot < m && a[pivot * m + c] == 0) {
      pivot++;
    }
    if (pivot == m) { continue; }
    for (size_t k = 0; k < m; ++k) {
      std::swap(a[pivot * m + k], a[rank * m + k]);
    }
    const uint64_t inv = powmod(a[rank * m + c], PRIME - 2);
    for (size_t k = 0; k < m; ++k) {
      a[rank * m + k] = mulmod(a[rank * m + k], inv);
    }
    for (size_t r = 0; r < m; ++r) {
      if (r == rank || a[r * m + c] == 0) { continue; }
      const uint64_t f = a[r * m + c];
      for (size_t k = 0; k < m; ++k) {
        const uint64_t sub = mulmod(f, a[rank * m + k]);
        a[r * m + k] = (a[r * m + k] >= sub) ? a[r * m + k] - sub
                                             : a[r * m + k] + PRIME - sub;
      }
    }
    rank++;
  }
  return rank;
}


std::vector<size_t> curve_and_cells()
{
  std::vector<size_t> out;
  for (size_t p = tuner_layout::KSC_MG_BASE; p < tuner_layout::KS_CURVE_END;
       ++p) {
    out.push_back(p);
  }
  const std::vector<size_t> cells = duplicated_cells();
  out.insert(out.end(), cells.begin(), cells.end());
  return out;
}


// The shipped weights with S122's curve at its seed: the constant 45 in both
// halves and every other input 0 (DEC-344 (2)), where every side's x is 45.
std::vector<double> at_seed()
{
  std::vector<double> p(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(p.data());
  for (size_t i = 0; i < tuner_layout::KS_CURVE_COUNT; ++i) {
    p[tuner_layout::KSC_MG_BASE + i] = (i == 0) ? 45.0 : 0.0;
    p[tuner_layout::KSC_EG_BASE + i] = (i == 0) ? 45.0 : 0.0;
  }
  return p;
}

}  // namespace


TEST_CASE(
    "the audit's local form is the smooth score's derivative at the shipped "
    "weights")
{
  const dataset_t data = fixture();
  std::vector<double> shipped(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(shipped.data());

  // The precondition: the shipped point is one the shared form cannot read.
  group_slopes_t slopes = audit_design::group_slopes(data, shipped.data());
  REQUIRE_FALSE(slopes.shared);
  REQUIRE(audit_design::local_form_exact(data, shipped.data()));
  REQUIRE(audit_design::scales_at_identity(shipped.data()));
  slopes.local = true;

  design_builder_t builder;
  design_row_t x;
  std::vector<double> p = shipped;
  const tuner_trace::model_t& model = tuner_trace::engine_model();

  size_t rows = 0;
  size_t compared = 0;
  size_t curve_compared = 0;
  size_t sides_apart = 0;  // a half with both kings' x positive and unequal
  double worst = 0.0;
  size_t worst_param = PARAM_COUNT;

  for (size_t row = 0; row < data.size(); ++row) {
    const tuner_trace::smooth_t s =
        tuner_trace::smooth(model, data, row, p.data());
    if (s.clamp_binds || std::fabs(s.stage_two) > LAZY_EVAL_MARGIN - 1.0) {
      continue;
    }

    // max(x, 0) turns at 0, where no difference has its derivative.
    const sums_t sums = sums_of(data, row, p.data());
    if (sums.mg[0] == 0.0 || sums.mg[1] == 0.0 || sums.eg[0] == 0.0 ||
        sums.eg[1] == 0.0) {
      continue;
    }
    if ((sums.mg[0] > 0.0 && sums.mg[1] > 0.0 && sums.mg[0] != sums.mg[1]) ||
        (sums.eg[0] > 0.0 && sums.eg[1] > 0.0 && sums.eg[0] != sums.eg[1])) {
      sides_apart++;
    }

    builder.build(data, row, &x, slopes, p.data());
    rows++;

    // Every parameter the row's trace reaches, and every one the design
    // gives a column: smooth() reads no other, so any other's difference is
    // zero, and a column written to the wrong slot is read here too.
    std::vector<size_t> reach(x.index.begin(), x.index.end());
    for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
      const tuner_trace::feature_t& f =
          tuner_trace::feature_table().features[data.entries[j].feature];
      reach.push_back(f.mg);
      if (f.eg != tuner_trace::NO_PARAM) { reach.push_back(f.eg); }
    }
    for (uint32_t j = data.side_offsets[row]; j < data.side_offsets[row + 1];
         ++j) {
      reach.push_back(tuner_layout::KSC_MG_BASE + data.side_entries[j].input);
      reach.push_back(tuner_layout::KSC_EG_BASE + data.side_entries[j].input);
    }
    for (int c = 0; c < 2; ++c) {
      if (data.cells[2 * row + c] != eval_trace::NO_CELL) {
        reach.push_back(tuner_layout::EGS_BASE + data.cells[2 * row + c]);
      }
    }
    std::sort(reach.begin(), reach.end());
    reach.erase(std::unique(reach.begin(), reach.end()), reach.end());

    for (const size_t param : reach) {
      const double column = column_of(x, param);

      const double kept = p[param];
      p[param] = kept + H;
      const double up = tuner_trace::smooth(model, data, row, p.data()).score;
      p[param] = kept - H;
      const double down = tuner_trace::smooth(model, data, row, p.data()).score;
      p[param] = kept;
      const double slope = (up - down) / (2.0 * H);

      if (column == 0.0 && slope == 0.0) { continue; }

      bool mg = false;
      double scale = GAME_PHASE_MAX;
      if (curve_param(param, &mg)) {
        scale = CURVE_SCALE_LOCAL;
        curve_compared++;
      } else if (cell_param(param)) {
        scale = static_cast<double>(GAME_PHASE_MAX) * ENDGAME_SCALE_ONE;
      }

      const double apart = std::fabs(column / scale - slope);
      compared++;
      if (apart > worst) {
        worst = apart;
        worst_param = param;
      }
    }
  }

  MESSAGE("rows " << rows << ", columns compared " << compared << ", curve "
                  << curve_compared << ", rows with both kings' x apart "
                  << sides_apart << ", worst " << worst << " at "
                  << worst_param);
  CHECK(rows > 1000);
  CHECK(curve_compared > 10000);
  CHECK(sides_apart > 1000);
  CHECK(worst < 1e-6);
}


TEST_CASE(
    "at the curve's seed the local form is the shared one times the shared "
    "x, and the rank does not move")
{
  const dataset_t data = fixture();
  const std::vector<double> seed = at_seed();

  const group_slopes_t shared = audit_design::group_slopes(data, seed.data());
  REQUIRE(shared.shared);
  REQUIRE(shared.x[0] == 45.0);
  REQUIRE(shared.x[1] == 45.0);
  REQUIRE(shared.live[0]);
  REQUIRE(shared.live[1]);

  group_slopes_t local;
  local.local = true;

  design_builder_t builder;
  design_row_t a;
  design_row_t b;
  size_t curve_columns = 0;
  size_t mismatches = 0;

  for (size_t row = 0; row < data.size(); ++row) {
    builder.build(data, row, &a, shared, seed.data());
    builder.build(data, row, &b, local, seed.data());

    std::vector<size_t> params(a.index.begin(), a.index.end());
    params.insert(params.end(), b.index.begin(), b.index.end());
    std::sort(params.begin(), params.end());
    params.erase(std::unique(params.begin(), params.end()), params.end());

    for (const size_t param : params) {
      bool mg = false;
      const double old_form = column_of(a, param);
      const double new_form = column_of(b, param);
      const bool curve = curve_param(param, &mg);
      if (curve) { curve_columns++; }
      const double want = curve ? 45.0 * old_form : old_form;
      if (new_form != want) { mismatches++; }
    }
  }

  CAPTURE(curve_columns);
  CHECK(curve_columns > 10000);
  CHECK(mismatches == 0);

  // The curve's 24 columns and the 22 linear cells they count again: at the
  // seed the 22 pairs are one direction each and the constant's two columns
  // are zero, rank 22 in either form.
  const std::vector<size_t> columns = curve_and_cells();
  REQUIRE(columns.size() == 46);
  const size_t old_rank = rank_over(data, columns, shared, seed.data());
  const size_t new_rank = rank_over(data, columns, local, seed.data());
  CHECK(old_rank == 22);
  CHECK(new_rank == old_rank);

  // And at the shipped weights the local form parts the pairs: a reading of
  // what the restated identity rule rests on, more than 22.
  std::vector<double> shipped(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(shipped.data());
  const size_t shipped_rank = rank_over(data, columns, local, shipped.data());
  MESSAGE(
      "rank over the 46 at the shipped weights, local form: " << shipped_rank);
  CHECK(shipped_rank > 22);
}


TEST_CASE(
    "the local form refuses a model its design check would misread: more than "
    "one group, or one outside the clamp")
{
  // The engine's own model is the form the checks are written for.
  CHECK(audit_design::local_form_refusal(tuner_trace::engine_model()).empty());

  tuner_trace::model_t two = tuner_trace::engine_model();
  two.groups.push_back(two.groups[0]);
  two.groups[1].name = "a second curve";
  CHECK_FALSE(audit_design::local_form_refusal(two).empty());

  tuner_trace::model_t outside = tuner_trace::engine_model();
  outside.groups[0].clamped = false;
  const std::string why = audit_design::local_form_refusal(outside);
  CAPTURE(why);
  CHECK(why.find("outside the clamp") != std::string::npos);
}
