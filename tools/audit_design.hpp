#pragma once
// The design tools/feature_audit's identity report reads, one row of it a
// corpus row. Moved here from tools/feature_audit.cpp in S124's phase 2a, so
// that tests/test_audit_design.cpp can hold its columns to the trace's own
// score; the code that ranks it stays in the tool.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

namespace audit_design
{

using tuner_layout::PARAM_COUNT;
using tuner_trace::dataset_t;

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


// S122: a nonlinear group's columns, where they can be integers. A group's
// derivative in input i is sign * (T'(x_W) n_W,i - T'(x_B) n_B,i), each
// side's slope at its own x, so it is linear in the counts only where every
// side of every row sits at one x a half -- S122's curve at its seed, where x
// is the constant on every side. There each half's column is that one slope
// times the integer sign * (n_W,i - n_B,i), and a non-zero factor common to a
// column changes no rank and no dependency, so the design takes the integer;
// a slope of zero leaves the column zero.
//
// Anywhere else, S124's phase 2a: the design linearised at the weights read,
// the local form. T'(x) = 2 max(x, 0) / divisor, so the column is each side's
// own slope times its own counts, sign * (max(x_W, 0) n_W,i - max(x_B, 0)
// n_B,i) times the half's taper weight, with divisor / 2 (1024 for S122's
// curve) a factor common to every one of the group's columns, dropped. At
// integer weights x is an integer and so is the column; at the seed it is the
// shared form's times the shared x, the same rank and the same dependencies.
// The rank it gives is the design's at that point: which directions the loss
// sees there, not that each stays seen once a fit moves a side's x.
struct group_slopes_t
{
  bool shared = false;            // every side of every row at one x a half
  bool live[2] = {false, false};  // that x positive, mg and eg
  double x[2] = {0.0, 0.0};
  bool local = false;  // each side's own slope, row by row
};

inline group_slopes_t group_slopes(const dataset_t& data, const double* params)
{
  group_slopes_t out;
  const tuner_trace::model_t& model = tuner_trace::engine_model();

  if (model.groups.empty()) { return out; }

  bool first = true;
  out.shared = true;

  for (size_t g = 0; g < model.groups.size() && out.shared; ++g) {
    for (size_t row = 0; row < data.size(); ++row) {
      const tuner_trace::group_sums_t x =
          tuner_trace::group_sums(model.groups[g], g, data, row, params);

      for (int side = 0; side < 2; ++side) {
        if (first) {
          out.x[0] = x.mg[side];
          out.x[1] = x.eg[side];
          first = false;
        }

        if (x.mg[side] != out.x[0] || x.eg[side] != out.x[1]) {
          out.shared = false;
          return out;
        }
      }
    }
  }

  out.live[0] = out.x[0] > 0.0;
  out.live[1] = out.x[1] > 0.0;
  return out;
}


// Whether every side's sum of every group is an integer on every row, and
// every group's transform the one the local form is written for: where both
// hold the local form's columns are integers, and nowhere else.
inline bool local_form_exact(const dataset_t& data, const double* params)
{
  const tuner_trace::model_t& model = tuner_trace::engine_model();

  for (size_t g = 0; g < model.groups.size(); ++g) {
    if (model.groups[g].transform != tuner_trace::TRANSFORM_SQUARED_POSITIVE) {
      return false;
    }

    for (size_t row = 0; row < data.size(); ++row) {
      const tuner_trace::group_sums_t x =
          tuner_trace::group_sums(model.groups[g], g, data, row, params);

      for (int side = 0; side < 2; ++side) {
        if (x.mg[side] != std::round(x.mg[side]) ||
            x.eg[side] != std::round(x.eg[side])) {
          return false;
        }
      }
    }
  }

  return true;
}


// What the local form's two checks assume of the groups, S122's curve's form:
// one group, its score inside the clamp, so tools/feature_audit's design check
// reads it at that one divisor in stage two. A model with more, or with a group
// outside the clamp, is refused by name rather than misread (S124's phase 2a,
// the fast check's finding 6). Empty when the model is that form.
inline std::string local_form_refusal(const tuner_trace::model_t& model)
{
  if (model.groups.size() > 1) {
    return "the model carries " + std::to_string(model.groups.size()) +
           " groups, and the local form's design check reads one, S122's curve";
  }
  for (const tuner_trace::group_t& group : model.groups) {
    if (!group.clamped) {
      return "the group " + group.name +
             " sits outside the clamp, and the local form's design check adds "
             "a "
             "group's score to stage two";
    }
  }
  return "";
}


// S124: whether every scale factor is at F in `params`. There the factor's
// chain rule leaves every material and positional column as it was, and a
// cell's own column is d(score)/d(f_c) = E1 w_eg / F: times 24, and the 1 / F
// dropped as a factor common to the column, the integer E1 (24 - phase) on
// the rows whose E1 picks that cell (E1 an integer at integer weights). Off F
// the material and positional endgame columns carry f / F row by row, and
// this design has no exact integer row for them.
inline bool scales_at_identity(const double* params)
{
  for (size_t c = 0; c < tuner_layout::ENDGAME_SCALE_COUNT; ++c) {
    if (params[tuner_layout::EGS_BASE + c] != ENDGAME_SCALE_ONE) {
      return false;
    }
  }
  return true;
}


class design_builder_t
{
public:
  design_builder_t() : dense_(PARAM_COUNT, 0), marked_(PARAM_COUNT, 0) {}

  // `params`, when given, is where the cells' columns are read: E1 and the
  // pick (scales_at_identity() above). Without it the cells have no column.
  // The local form reads each side's x there too, and needs it.
  void build(const dataset_t& data,
             size_t row,
             design_row_t* out,
             const group_slopes_t& slopes = group_slopes_t{},
             const double* params = nullptr)
  {
    const std::vector<tuner_trace::feature_t>& features =
        tuner_trace::feature_table().features;

    const int64_t mg = data.phase[row];
    const int64_t eg = GAME_PHASE_MAX - mg;

    if (params != nullptr) {
      double e1 = 0.0;

      for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
        const tuner_trace::entry_t e = data.entries[j];
        const tuner_trace::feature_t& f = features[e.feature];

        if (f.site == tuner_trace::SITE_MATERIAL) {
          e1 += e.coef * params[f.mg];
        } else if (f.site == tuner_trace::SITE_POSITIONAL) {
          e1 += e.coef * params[f.eg];
        }
      }

      const uint8_t cell = tuner_trace::picked_cell(data, row, e1);

      if (cell != eval_trace::NO_CELL) {
        add(tuner_layout::EGS_BASE + cell, std::llround(e1) * eg);
      }
    }

    // The groups' integer columns, at the slope every side shares.
    if (slopes.shared) {
      const tuner_trace::model_t& model = tuner_trace::engine_model();

      for (uint32_t j = data.side_offsets[row]; j < data.side_offsets[row + 1];
           ++j) {
        const tuner_trace::side_entry_t e = data.side_entries[j];
        const tuner_trace::group_t& group = model.groups[e.group];
        const int64_t sign =
            static_cast<int64_t>(group.sign) * ((e.side == 0) ? 1 : -1);
        const std::pair<uint32_t, uint32_t>& in = group.inputs[e.input];

        if (slopes.live[0]) { add(in.first, sign * e.coef * mg); }
        if (slopes.live[1]) { add(in.second, sign * e.coef * eg); }
      }
    }

    // The local form: each side's own max(x, 0) per half, at `params`.
    if (slopes.local && params != nullptr) {
      const tuner_trace::model_t& model = tuner_trace::engine_model();
      positive_.resize(model.groups.size());

      for (size_t g = 0; g < model.groups.size(); ++g) {
        const tuner_trace::group_sums_t x =
            tuner_trace::group_sums(model.groups[g], g, data, row, params);

        for (int side = 0; side < 2; ++side) {
          positive_[g].mg[side] = std::llround(std::max(x.mg[side], 0.0));
          positive_[g].eg[side] = std::llround(std::max(x.eg[side], 0.0));
        }
      }

      for (uint32_t j = data.side_offsets[row]; j < data.side_offsets[row + 1];
           ++j) {
        const tuner_trace::side_entry_t e = data.side_entries[j];
        const tuner_trace::group_t& group = model.groups[e.group];
        const int64_t sign =
            static_cast<int64_t>(group.sign) * ((e.side == 0) ? 1 : -1);
        const std::pair<uint32_t, uint32_t>& in = group.inputs[e.input];
        const positive_t& at = positive_[e.group];

        add(in.first, sign * e.coef * at.mg[e.side] * mg);
        add(in.second, sign * e.coef * at.eg[e.side] * eg);
      }
    }

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
  struct positive_t
  {
    int64_t mg[2] = {0, 0};
    int64_t eg[2] = {0, 0};
  };

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
  std::vector<positive_t> positive_;
};


// ------------------------------------------- the columns against the score
//
// S124's phase 2a. What a design column is, as a multiple of d(score)/d(param)
// at the weights read: 24 for every linear one (the taper weights times 24,
// a piece value 24 times its 1), 24 F for a scale factor at F (its 1 / F
// dropped), and for a group's input 24 divisor / 2 in the local form, 24
// divisor / (2 x) in the shared form at a live half's shared x. A parameter
// the design row has no entry for is a zero column.
// The divisor of the group whose input `param` is, and which half; 0 when
// it is no group's.
inline double group_divisor(size_t param, bool* mg_half)
{
  for (const tuner_trace::group_t& group : tuner_trace::engine_model().groups) {
    for (const auto& [mg_param, eg_param] : group.inputs) {
      if (param == mg_param || param == eg_param) {
        *mg_half = (param == mg_param);
        return group.divisor;
      }
    }
  }
  return 0.0;
}

inline double column_scale(size_t param, const group_slopes_t& slopes)
{
  bool mg_half = false;
  const double divisor = group_divisor(param, &mg_half);

  if (divisor > 0.0) {
    if (slopes.local) { return GAME_PHASE_MAX * divisor / 2.0; }
    const double x = slopes.x[mg_half ? 0 : 1];
    return (x > 0.0) ? GAME_PHASE_MAX * divisor / (2.0 * x) : 0.0;
  }

  if (param >= tuner_layout::EGS_BASE &&
      param < tuner_layout::EGS_BASE + tuner_layout::ENDGAME_SCALE_COUNT) {
    return static_cast<double>(GAME_PHASE_MAX) * ENDGAME_SCALE_ONE;
  }

  return GAME_PHASE_MAX;
}


// The linearisation check, one row: every parameter the row's trace reaches
// -- its features' halves, its groups' inputs, its candidate cells -- the
// design's column read against the smooth score's central difference at that
// parameter, over its scale. The score is piecewise quadratic in a group's
// weights and linear in the rest, so the difference is exact but for
// rounding wherever no kink lies within h: a row is skipped when stage two
// lies within one centipawn of the clamp, or a side's x sits at 0 in a half,
// where max(x, 0) turns. h 1e-3 keeps every x within its integer's
// neighbourhood: no input count reaches 1000.
struct linearisation_t
{
  size_t rows = 0;
  size_t skipped = 0;
  size_t columns = 0;
  double worst = 0.0;
  size_t worst_param = PARAM_COUNT;
  size_t worst_row = 0;
};

inline constexpr double LINEARISATION_H = 1e-3;

inline void check_row(const dataset_t& data,
                      size_t row,
                      const design_row_t& x,
                      const group_slopes_t& slopes,
                      std::vector<double>* params,
                      linearisation_t* out)
{
  const tuner_trace::model_t& model = tuner_trace::engine_model();
  double* p = params->data();

  const tuner_trace::smooth_t at = tuner_trace::smooth(model, data, row, p);
  if (at.clamp_binds || std::fabs(at.stage_two) > LAZY_EVAL_MARGIN - 1.0) {
    out->skipped++;
    return;
  }

  for (size_t g = 0; g < model.groups.size(); ++g) {
    const tuner_trace::group_sums_t s =
        tuner_trace::group_sums(model.groups[g], g, data, row, p);
    for (int side = 0; side < 2; ++side) {
      if (s.mg[side] == 0.0 || s.eg[side] == 0.0) {
        out->skipped++;
        return;
      }
    }
  }

  std::vector<size_t> reach;
  for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
    const tuner_trace::feature_t& f =
        tuner_trace::feature_table().features[data.entries[j].feature];
    reach.push_back(f.mg);
    if (f.eg != tuner_trace::NO_PARAM) { reach.push_back(f.eg); }
  }
  for (uint32_t j = data.side_offsets[row]; j < data.side_offsets[row + 1];
       ++j) {
    const tuner_trace::side_entry_t e = data.side_entries[j];
    const std::pair<uint32_t, uint32_t>& in =
        model.groups[e.group].inputs[e.input];
    reach.push_back(in.first);
    reach.push_back(in.second);
  }
  for (int c = 0; c < 2; ++c) {
    const uint8_t cell = data.cells[2 * row + c];
    if (cell != eval_trace::NO_CELL) {
      reach.push_back(tuner_layout::EGS_BASE + cell);
    }
  }
  // And every column the design gives, so one written to a parameter the
  // row does not reach is read against that parameter's difference too.
  reach.insert(reach.end(), x.index.begin(), x.index.end());
  std::sort(reach.begin(), reach.end());
  reach.erase(std::unique(reach.begin(), reach.end()), reach.end());

  for (const size_t param : reach) {
    const double kept = p[param];
    p[param] = kept + LINEARISATION_H;
    const double up = tuner_trace::smooth(model, data, row, p).score;
    p[param] = kept - LINEARISATION_H;
    const double down = tuner_trace::smooth(model, data, row, p).score;
    p[param] = kept;

    const double slope = (up - down) / (2.0 * LINEARISATION_H);

    double column = 0.0;
    const auto found = std::lower_bound(x.index.begin(), x.index.end(), param);
    if (found != x.index.end() && *found == param) {
      column = static_cast<double>(x.value[found - x.index.begin()]);
    }

    const double scale = column_scale(param, slopes);
    const double apart =
        (scale > 0.0) ? std::fabs(column / scale - slope) : std::fabs(slope);

    out->columns++;
    if (apart > out->worst) {
      out->worst = apart;
      out->worst_param = param;
      out->worst_row = row;
    }
  }

  out->rows++;
}

}  // namespace audit_design
