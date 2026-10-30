#pragma once
// The tuner's model of evaluate(), read from evaluate() itself. S262.
//
// Until S262 the tuner fitted tools/eval_model.hpp, a hand-written copy of the
// evaluation -- its own feature extractors, its own sum -- held to the engine
// by tests/test_eval_model.cpp to within the truncation bound, and its gradient
// was a third copy whose comments record three terms whose block was forgotten
// and whose weights came back exactly as they went in. Here the engine's trace
// build (src/eval_trace.hpp, CHESSO_TRACE) records, per position, the
// coefficient of every weight evaluate() applies, and the model below is a sum
// over those records. A new linear term costs its trace call in evaluation.cpp,
// its line in term_layout() below and its parameters' block in
// tools/tuner_layout.hpp; nothing here re-derives what it counts, and one loop
// differentiates every term the trace records.
//
// Implemented from the descriptions in Grant, "Evaluation & Tuning in Chess
// Engines" (2020) -- the wrapped evaluation, the sparse per-position
// coefficient layout, the derivatives of linear terms and of a per-side
// transform -- and the Chess Programming Wiki's "Texel's Tuning Method" for the
// objective. DEC-221: no engine's tuner source was read.
//
// **The contract is an identity, and it is checked on every row loaded.**
// exact_score() recomputes a row's score from its trace with the engine's own
// integer arithmetic -- the four truncating tapers, the clamp over stage two --
// and at the weights the engine was built with it equals evaluate() exactly.
// load() checks that on every row it reads and the tuner refuses a corpus on
// which it fails: a trace that disagrees with the engine is a model of a
// different function, which is the failure tools/eval_model.hpp could only
// bound.
//
// **What the fit descends is the smooth relaxation of that score**, score()
// below: the same sum with every taper exact in floating point instead of
// truncated, and the clamp kept. Truncation is dropped because a step function
// has no derivative to descend; it moves a row by less than one centipawn per
// taper that rounds, four at most (3 x 23/24 = 2.875 while tempo is zero, 4 x
// 23/24 = 3.833 once it is not). The clamp is kept, and differentiated exactly
// -- gradient() below has why, and that it reverses the treatment the tuner
// had before S262.
//
// Nonlinear groups -- a per-side transform of a weighted sum, the shape a
// king-safety curve takes -- are carried by the same records: each side's
// coefficients, not their difference, and a model_t that names the group's
// transform. None exists in evaluate() today; tests/test_tuner_gradient.cpp
// fits a synthetic one through this path.
#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "eval_trace.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_split.hpp"
#include "tuner_target.hpp"

#ifndef CHESSO_TRACE
#error "tools/tuner_trace.hpp reads the trace build: link chesso_engine_trace"
#endif

namespace tuner_trace
{

using eval_trace::term_t;
using tuner_target::LN10_OVER_400;
using tuner_target::sigmoid;


// ---------------------------------------------------------------- the layout
//
// Where each term's weighted sum goes once evaluate() has it, which is fixed by
// where the term sits in src/evaluation.cpp and is the one thing the trace does
// not record. exact_score() is what keeps this table honest: put a term on the
// wrong site and the rows where the two tapers round differently disagree with
// evaluate().
//
//   MATERIAL    board->material, added untapered in evaluate_cheap()
//   POSITIONAL  board->psqt plus the pawn terms (S123's family among them),
//               one taper
//   TEMPO       its own taper, added after the side-to-move sign
//   MOBILITY    its own taper, in evaluate_expensive()
//   KING_SAFETY its own taper, and the two summed and clamped to the margin
enum site_t : uint8_t
{
  SITE_MATERIAL,
  SITE_POSITIONAL,
  SITE_TEMPO,
  SITE_MOBILITY,
  SITE_KING_SAFETY,
  SITE_COUNT
};

constexpr bool clamped_site(site_t site)
{ return site == SITE_MOBILITY || site == SITE_KING_SAFETY; }

constexpr uint32_t NO_PARAM = std::numeric_limits<uint32_t>::max();

// One weight array pair of the evaluation: how many entries, where its
// middlegame and endgame halves sit in the parameter vector, and its site. An
// untapered array has one half, `eg_base` NO_PARAM.
struct term_layout_t
{
  size_t width;
  uint32_t mg_base;
  uint32_t eg_base;
  site_t site;
};

// The trace's terms against tools/tuner_layout.hpp's vector. The declaration a
// new linear term adds, beside its trace call and its weights.
inline term_layout_t term_layout(term_t term)
{
  using namespace tuner_layout;

  switch (term) {
    case eval_trace::TERM_MATERIAL:
      return {MATERIAL_COUNT, 0, NO_PARAM, SITE_MATERIAL};
    case eval_trace::TERM_PSQT:
      return {SQUARE_COUNT, MG_BASE, EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_MOBILITY:
      return {MOBILITY_COUNT, MOB_MG_BASE, MOB_EG_BASE, SITE_MOBILITY};
    case eval_trace::TERM_KING_SAFETY:
      return {KING_SAFETY_COUNT, KS_MG_BASE, KS_EG_BASE, SITE_KING_SAFETY};
    case eval_trace::TERM_PASSED_PAWN:
      return {PASSED_PAWN_COUNT, PP_MG_BASE, PP_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_PAWN_STRUCTURE:
      return {PAWN_STRUCTURE_COUNT, PS_MG_BASE, PS_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_PIECE_PLACEMENT:
      return {PIECE_PLACEMENT_COUNT, PL_MG_BASE, PL_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_TEMPO:
      return {TEMPO_COUNT, TEMPO_MG_BASE, TEMPO_EG_BASE, SITE_TEMPO};
    // S123's family is summed into the pawn terms' pair in evaluate_pawns(),
    // so it shares their one taper and adds no truncating division.
    case eval_trace::TERM_PASSED_FREE:
      return {PASSED_FREE_COUNT, PF_MG_BASE, PF_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_PASSED_SAFE:
      return {PASSED_SAFE_COUNT, PSF_MG_BASE, PSF_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_PASSED_KING_OWN:
      return {PASSED_KING_COUNT, PKO_MG_BASE, PKO_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_PASSED_KING_ENEMY:
      return {PASSED_KING_COUNT, PKE_MG_BASE, PKE_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_CANDIDATE_PASSER:
      return {CANDIDATE_PASSER_COUNT, CP_MG_BASE, CP_EG_BASE, SITE_POSITIONAL};
    case eval_trace::TERM_COUNT:
      break;
  }

  return {0, NO_PARAM, NO_PARAM, SITE_COUNT};
}


// A feature is one coefficient slot of a row: one entry of one term, and so
// one parameter pair. Numbered term by term in term_t's order, so the dataset
// stores a 16-bit feature and not a 32-bit parameter twice.
struct feature_t
{
  uint32_t mg;
  uint32_t eg;
  site_t site;
};

struct feature_table_t
{
  std::vector<feature_t> features;
  uint32_t base[eval_trace::TERM_COUNT + 1];
};

inline const feature_table_t& feature_table()
{
  static const feature_table_t table = [] {
    feature_table_t out;
    uint32_t next = 0;

    for (int t = 0; t < eval_trace::TERM_COUNT; ++t) {
      const term_layout_t layout = term_layout(static_cast<term_t>(t));
      out.base[t] = next;

      for (size_t i = 0; i < layout.width; ++i) {
        const uint32_t eg = (layout.eg_base == NO_PARAM)
                                ? NO_PARAM
                                : layout.eg_base + static_cast<uint32_t>(i);
        out.features.push_back(
            {layout.mg_base + static_cast<uint32_t>(i), eg, layout.site});
      }

      next += static_cast<uint32_t>(layout.width);
    }

    out.base[eval_trace::TERM_COUNT] = next;
    return out;
  }();

  return table;
}


// ------------------------------------------------------- nonlinear groups
//
// A group's score is a transform of each side's own weighted sum, subtracted:
//
//   x[side] = sum_i coef[side][i] * w[i]              (mg and eg halves apart)
//   group   = sign * (T(x[WHITE]) - T(x[BLACK])), each half, then tapered
//
// with its own taper and outside the clamp. The one transform provided is the
// one S122's king-safety curve is written for, T(x) = max(0, x)^2 / divisor:
// convex, zero below zero, and with a derivative of exactly zero at x = 0 --
// which is why a group of these cannot start from zero weights. derive_seed()
// is the answer to that (DEC-134 form b).
enum transform_t : uint8_t
{
  TRANSFORM_SQUARED_POSITIVE
};

inline double transform(transform_t, double divisor, double x)
{
  const double positive = (x > 0.0) ? x : 0.0;
  return positive * positive / divisor;
}

inline double transform_slope(transform_t, double divisor, double x)
{ return (x > 0.0) ? 2.0 * x / divisor : 0.0; }

struct group_t
{
  std::string name;
  transform_t transform = TRANSFORM_SQUARED_POSITIVE;
  double divisor = 1.0;
  double sign = 1.0;
  // One (mg, eg) parameter pair per input, in input order.
  std::vector<std::pair<uint32_t, uint32_t>> inputs;
};

// What is being fitted: how long the vector is, and the groups on top of the
// linear terms. The engine's own is tools/tuner_layout.hpp's PARAM_COUNT, 831
// since S135 and 955 since S121, and no group; a test grows it to fit a
// synthetic group.
struct model_t
{
  size_t param_count = tuner_layout::PARAM_COUNT;
  std::vector<group_t> groups;
};

inline const model_t& engine_model()
{
  static const model_t model;
  return model;
}

// Which group of engine_model() a per-side trace entry feeds: none, today. A
// term evaluate() starts recording with EVAL_TRACE_SIDE gets its group declared
// in engine_model() and its index here, and until then load() refuses its
// records rather than dropping them.
inline int group_of_term(term_t)
{ return -1; }


// ---------------------------------------------------------------- the data
//
// One row is the sparse list of (feature, coefficient) its trace summed to,
// zeros dropped, plus the per-side entries of any group. Measured storage and
// extraction time are in DEV_MANUAL.md and the S262 step file.
struct entry_t
{
  uint16_t feature;
  int16_t coef;
};

struct side_entry_t
{
  uint8_t group;
  uint8_t side;  // 0 White, 1 Black
  uint16_t input;
  int16_t coef;
};

struct dataset_t
{
  std::vector<entry_t> entries;
  std::vector<uint32_t> offsets{0};  // rows + 1, into `entries`
  std::vector<side_entry_t> side_entries;
  std::vector<uint32_t> side_offsets{0};  // rows + 1, into `side_entries`

  // The phase evaluate() tapered on, as the trace recorded it: the engine's
  // own, so a corpus's stored phase column is read for its range and nothing
  // else (2026-09-10_adversarial-F36 was the copy trusting it).
  std::vector<uint8_t> phase;

  // +1 White to move, -1 Black: what turns the White-relative scores here
  // into evaluate()'s side-to-move-relative one.
  std::vector<int8_t> stm;

  std::vector<float> result;
  // datagen's search score, White relative, for --lambda. S075.
  std::vector<int16_t> score;
  // The ply each row stands at; the S066 split reads it and nothing else.
  std::vector<uint32_t> ply;

  size_t size() const { return phase.size(); }
};


// One position's trace, summed per feature. `engine_white` is what evaluate()
// returned, turned White relative, for the identity check.
struct row_trace_t
{
  std::vector<entry_t> entries;
  std::vector<side_entry_t> side_entries;
  int phase = 0;
  int stm = 1;
  int engine_white = 0;
};


// Sums one evaluate() record per feature into `out`'s entries, zeros dropped,
// and its per-side entries into their groups. False, with `error` saying why,
// on a record the dataset cannot hold: one past its capacity, without a usable
// phase, with an index outside its term, a per-side entry for a term no group
// is declared for, or a summed coefficient outside int16. The summing buffer is
// thread local and left all zero on every return, refused or not, so a refused
// record cannot leak into the next. Split from trace_board() so a test can hand
// it a record evaluate() could not write.
inline bool sum_record(const eval_trace::record_t& record,
                       row_trace_t* out,
                       std::string* error)
{
  thread_local std::vector<int32_t> dense;

  const feature_table_t& table = feature_table();

  if (dense.size() != table.features.size()) {
    dense.assign(table.features.size(), 0);
  }

  const auto refuse = [&](const char* why) {
    std::fill(dense.begin(), dense.end(), 0);
    *error = why;
    return false;
  };

  if (record.overflow) { return refuse("the trace overflowed its record"); }

  if (record.phase < 0 || record.phase > GAME_PHASE_MAX) {
    return refuse("the trace recorded no usable phase");
  }

  out->entries.clear();
  out->side_entries.clear();

  for (size_t i = 0; i < record.count; ++i) {
    const eval_trace::entry_t& e = record.entries[i];

    if (e.term >= eval_trace::TERM_COUNT ||
        e.index >= term_layout(e.term).width) {
      return refuse("a trace entry outside its term");
    }

    if (e.side != eval_trace::SIDE_BOTH) {
      const int group = group_of_term(e.term);

      if (group < 0) {
        return refuse(
            "a per-side trace entry for a term no group is declared for");
      }

      out->side_entries.push_back({static_cast<uint8_t>(group),
                                   static_cast<uint8_t>(e.side), e.index,
                                   static_cast<int16_t>(e.coef)});
      continue;
    }

    dense[table.base[e.term] + e.index] += e.coef;
  }

  // In feature order, zeros dropped: a White and a Black piece on the same
  // mirrored square cancel here, and so do the many zero mobility counts.
  for (size_t f = 0; f < dense.size(); ++f) {
    const int32_t coef = dense[f];
    dense[f] = 0;

    if (coef == 0) { continue; }

    if (coef < std::numeric_limits<int16_t>::min() ||
        coef > std::numeric_limits<int16_t>::max()) {
      return refuse("a trace coefficient outside int16");
    }

    out->entries.push_back(
        {static_cast<uint16_t>(f), static_cast<int16_t>(coef)});
  }

  out->phase = record.phase;
  return true;
}


// Runs evaluate() on `board` with the trace on and sums what it recorded.
// False, with `error` saying why, on a record sum_record() refuses.
inline bool trace_board(const board_t* board,
                        row_trace_t* out,
                        std::string* error)
{
  thread_local eval_trace::record_t record;

  record.clear();
  eval_trace::sink = &record;
  const int engine = evaluate(board);
  eval_trace::sink = nullptr;

  if (!sum_record(record, out, error)) { return false; }

  out->stm = (board->active_color == WHITE) ? 1 : -1;
  out->engine_white = out->stm * engine;

  return true;
}


inline void append_row(const row_trace_t& trace,
                       float result,
                       int16_t score,
                       uint32_t ply,
                       dataset_t* data)
{
  data->entries.insert(data->entries.end(), trace.entries.begin(),
                       trace.entries.end());
  data->offsets.push_back(static_cast<uint32_t>(data->entries.size()));
  data->side_entries.insert(data->side_entries.end(),
                            trace.side_entries.begin(),
                            trace.side_entries.end());
  data->side_offsets.push_back(
      static_cast<uint32_t>(data->side_entries.size()));
  data->phase.push_back(static_cast<uint8_t>(trace.phase));
  data->stm.push_back(static_cast<int8_t>(trace.stm));
  data->result.push_back(result);
  data->score.push_back(score);
  data->ply.push_back(ply);
}


// --------------------------------------------------------- the exact score

// The engine's taper, written out: halves weighted by the phase and one
// division truncating towards zero. src/evaluation.hpp's taper() takes a packed
// score; the halves of a packed sum are exactly these separate sums while each
// fits 16 bits, which the engine's static_asserts guarantee for any weights it
// can be built with.
inline int64_t exact_taper(int64_t mg, int64_t eg, int phase)
{ return (mg * phase + eg * (GAME_PHASE_MAX - phase)) / GAME_PHASE_MAX; }


// The four tapered sums and the material a row's exact score is built from,
// before the clamp, so a caller can ask whether it binds.
struct exact_parts_t
{
  int64_t material = 0;
  int64_t positional = 0;
  int64_t tempo = 0;
  int64_t mobility = 0;
  int64_t safety = 0;
};

inline exact_parts_t exact_parts(const dataset_t& data,
                                 size_t row,
                                 const int64_t* w)
{
  const std::vector<feature_t>& features = feature_table().features;

  int64_t mg[SITE_COUNT] = {};
  int64_t eg[SITE_COUNT] = {};
  exact_parts_t parts;

  for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
    const entry_t e = data.entries[j];
    const feature_t& f = features[e.feature];

    if (f.site == SITE_MATERIAL) {
      parts.material += e.coef * w[f.mg];
      continue;
    }

    mg[f.site] += e.coef * w[f.mg];
    eg[f.site] += e.coef * w[f.eg];
  }

  const int phase = data.phase[row];

  parts.positional =
      exact_taper(mg[SITE_POSITIONAL], eg[SITE_POSITIONAL], phase);
  parts.tempo = exact_taper(mg[SITE_TEMPO], eg[SITE_TEMPO], phase);
  parts.mobility = exact_taper(mg[SITE_MOBILITY], eg[SITE_MOBILITY], phase);
  parts.safety = exact_taper(mg[SITE_KING_SAFETY], eg[SITE_KING_SAFETY], phase);

  return parts;
}


// A row's score, White relative, recomputed from its trace with the engine's
// integer arithmetic at integer weights `w`: material untapered, then four
// truncating tapers, the last two summed and clamped to LAZY_EVAL_MARGIN. At
// the weights the engine was built with this is evaluate() exactly, which is
// what load() checks on every row. Tempo's coefficient carries the side to
// move's sign, and both the taper's truncation and the clamp are odd, so the
// White-relative form is the engine's side-to-move one negated, digit for
// digit. Groups have no engine arithmetic to be exact to and are not read.
inline int64_t exact_score(const dataset_t& data, size_t row, const int64_t* w)
{
  const exact_parts_t parts = exact_parts(data, row, w);
  const int64_t margin = LAZY_EVAL_MARGIN;

  return parts.material + parts.positional + parts.tempo +
         std::clamp(parts.mobility + parts.safety, -margin, margin);
}


// The vector the engine was built with, as integers, for exact_score().
inline std::vector<int64_t> engine_weights()
{
  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  std::vector<int64_t> out(params.size());
  for (size_t i = 0; i < params.size(); ++i) {
    out[i] = static_cast<int64_t>(std::llround(params[i]));
  }

  return out;
}


// ---------------------------------------------------------------- loading

struct load_report_t
{
  uint64_t rows = 0;
  uint64_t mismatches = 0;  // exact_score() against evaluate()
  uint64_t clamped = 0;     // rows where the engine's clamp binds
  uint64_t entries = 0;
  std::string first_mismatch;
  double seconds = 0.0;
};


// A parsed corpus line, before the engine sees it.
struct line_t
{
  std::string fen;
  float result = 0.0f;
  int16_t score = 0;
  uint32_t ply = 0;
};


// `fen result score phase`, datagen's format, strictly. The stored phase is
// read for its range and nothing else; the trace's phase is the engine's.
inline bool parse_line(const std::string& text, line_t* out, std::string* error)
{
  std::istringstream fields(text);
  std::string fen, result_text, score_text, phase_text;

  if (!std::getline(fields, fen, '\t') ||
      !std::getline(fields, result_text, '\t') ||
      !std::getline(fields, score_text, '\t') ||
      !std::getline(fields, phase_text, '\t')) {
    *error = "fewer than four tab-separated fields";
    return false;
  }

  const double result = atof(result_text.c_str());
  const int stored_phase = atoi(phase_text.c_str());

  if (result < 0.0 || result > 1.0 || stored_phase < 0 ||
      stored_phase > GAME_PHASE_MAX) {
    *error = "result or phase out of range";
    return false;
  }

  // Refused rather than defaulted, as the mirror's load() did until S276 and
  // for its reason: 0 is a legal score, and --lambda would fit to it. S075.
  int score = 0;
  if (!tuner_target::parse_score(score_text, &score)) {
    *error = "unusable score column";
    return false;
  }

  // The split reconstructs games from this; a row it cannot place is refused
  // rather than guessed at. S066; datagen's ply column, S082.
  if (!tuner_split::row_ply(text, &out->ply)) {
    *error = "no usable ply: the FEN's move number, or datagen's ply column";
    return false;
  }

  out->fen = fen;
  out->result = static_cast<float>(result);
  out->score = static_cast<int16_t>(score);
  return true;
}


// Reads a corpus into `data`, tracing every row through evaluate(), and checks
// exact_score() against evaluate() on each at the shipped weights. Lines are
// read in blocks and traced on `threads` threads, each with its own game, then
// appended in file order, so the dataset is the same whatever the thread count.
// False, with `error` naming the line, on a line that does not parse, a FEN the
// engine refuses -- load_FEN() refuses a board without exactly one king a side,
// which the king gauges need (S275) -- or a trace the dataset cannot hold. A
// mismatch is counted, not refused here: the caller decides, and the tuner
// refuses.
inline bool load(const std::string& path,
                 dataset_t* data,
                 load_report_t* report,
                 std::string* error,
                 unsigned threads = 1)
{
  const auto started = std::chrono::steady_clock::now();
  std::ifstream file(path);

  if (!file) {
    *error = "cannot read " + path;
    return false;
  }

  if (threads == 0) { threads = 1; }

  {
    game_t warm;
    initialize_game_const_data(&warm);
  }

  const std::vector<int64_t> weights = engine_weights();
  const int64_t margin = LAZY_EVAL_MARGIN;

  constexpr size_t BLOCK = 1 << 16;
  std::vector<std::string> lines;
  std::vector<line_t> parsed(BLOCK);
  std::vector<row_trace_t> traces(BLOCK);
  std::vector<std::string> failures(threads);
  std::vector<size_t> failed_at(threads);

  bool done = false;

  while (!done) {
    lines.clear();
    std::string text;

    while (lines.size() < BLOCK && std::getline(file, text)) {
      if (text.empty()) { continue; }
      lines.push_back(text);
    }

    if (lines.size() < BLOCK) { done = true; }
    if (lines.empty()) { break; }

    std::vector<std::thread> workers;

    for (unsigned t = 0; t < threads; ++t) {
      failures[t].clear();

      workers.emplace_back([&, t]() {
        game_t game;
        initialize_game_const_data(&game);

        for (size_t i = t; i < lines.size(); i += threads) {
          std::string why;

          if (!parse_line(lines[i], &parsed[i], &why)) {
            failures[t] = why + ": " + lines[i];
            failed_at[t] = i;
            return;
          }

          std::string reason;
          if (!load_FEN(parsed[i].fen, &game, &reason)) {
            failures[t] =
                "the engine refuses the FEN" +
                (reason.empty() ? std::string() : " (" + reason + ")") + ": " +
                lines[i];
            failed_at[t] = i;
            return;
          }

          if (!trace_board(&game.board, &traces[i], &why)) {
            failures[t] = why + ": " + lines[i];
            failed_at[t] = i;
            return;
          }
        }
      });
    }

    for (std::thread& worker : workers) {
      worker.join();
    }

    // The earliest failing line, whichever thread found it, so the message
    // does not depend on the thread count.
    size_t first = lines.size();
    for (unsigned t = 0; t < threads; ++t) {
      if (!failures[t].empty() && failed_at[t] < first) {
        first = failed_at[t];
        *error = failures[t];
      }
    }

    if (first < lines.size()) { return false; }

    for (size_t i = 0; i < lines.size(); ++i) {
      if (data->entries.size() + traces[i].entries.size() >
          std::numeric_limits<uint32_t>::max()) {
        *error = "the corpus has more trace entries than a 32-bit offset holds";
        return false;
      }

      append_row(traces[i], parsed[i].result, parsed[i].score, parsed[i].ply,
                 data);

      const size_t row = data->size() - 1;

      if (exact_score(*data, row, weights.data()) != traces[i].engine_white) {
        if (report->mismatches == 0) { report->first_mismatch = lines[i]; }
        report->mismatches++;
      }

      // The rows the identity reaches the clamp on, counted so a reader can
      // see the clamp was exercised and not merely never reached.
      const exact_parts_t parts = exact_parts(*data, row, weights.data());
      const int64_t two = parts.mobility + parts.safety;
      if (two > margin || two < -margin) { report->clamped++; }
    }
  }

  report->rows = data->size();
  report->entries = data->entries.size();
  report->seconds =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - started)
          .count();

  if (data->size() == 0) {
    *error = "no rows in " + path;
    return false;
  }

  return true;
}


// -------------------------------------------------------- the smooth score

// A group's two per-side sums, middlegame and endgame halves apart.
struct group_sums_t
{
  double mg[2] = {0.0, 0.0};
  double eg[2] = {0.0, 0.0};
};

inline group_sums_t group_sums(const group_t& group,
                               size_t g,
                               const dataset_t& data,
                               size_t row,
                               const double* p)
{
  group_sums_t sums;

  for (uint32_t j = data.side_offsets[row]; j < data.side_offsets[row + 1];
       ++j) {
    const side_entry_t e = data.side_entries[j];

    if (e.group != g) { continue; }

    const std::pair<uint32_t, uint32_t>& in = group.inputs[e.input];
    sums.mg[e.side] += e.coef * p[in.first];
    sums.eg[e.side] += e.coef * p[in.second];
  }

  return sums;
}


struct smooth_t
{
  double score = 0.0;        // White relative
  bool clamp_binds = false;  // stage two lies beyond the margin
};


// The score the fit descends, White relative: exact_score() with every taper
// exact instead of truncated, the clamp kept, and the groups added on top.
inline smooth_t smooth(const model_t& model,
                       const dataset_t& data,
                       size_t row,
                       const double* p)
{
  const std::vector<feature_t>& features = feature_table().features;

  const double phase = data.phase[row];
  const double mg_weight = phase / GAME_PHASE_MAX;
  const double eg_weight = (GAME_PHASE_MAX - phase) / GAME_PHASE_MAX;

  double material = 0.0;
  double mg[SITE_COUNT] = {};
  double eg[SITE_COUNT] = {};

  for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
    const entry_t e = data.entries[j];
    const feature_t& f = features[e.feature];

    if (f.site == SITE_MATERIAL) {
      material += e.coef * p[f.mg];
      continue;
    }

    mg[f.site] += e.coef * p[f.mg];
    eg[f.site] += e.coef * p[f.eg];
  }

  smooth_t out;

  // One clamp over the two expensive terms together, where evaluate() puts it.
  const double margin = LAZY_EVAL_MARGIN;
  double stage_two = (mg[SITE_MOBILITY] + mg[SITE_KING_SAFETY]) * mg_weight +
                     (eg[SITE_MOBILITY] + eg[SITE_KING_SAFETY]) * eg_weight;

  if (stage_two > margin) {
    stage_two = margin;
    out.clamp_binds = true;
  } else if (stage_two < -margin) {
    stage_two = -margin;
    out.clamp_binds = true;
  }

  out.score = material + (mg[SITE_POSITIONAL] + mg[SITE_TEMPO]) * mg_weight +
              (eg[SITE_POSITIONAL] + eg[SITE_TEMPO]) * eg_weight + stage_two;

  for (size_t g = 0; g < model.groups.size(); ++g) {
    const group_t& group = model.groups[g];
    const group_sums_t x = group_sums(group, g, data, row, p);

    const double half_mg = transform(group.transform, group.divisor, x.mg[0]) -
                           transform(group.transform, group.divisor, x.mg[1]);
    const double half_eg = transform(group.transform, group.divisor, x.eg[0]) -
                           transform(group.transform, group.divisor, x.eg[1]);

    out.score += group.sign * (half_mg * mg_weight + half_eg * eg_weight);
  }

  return out;
}


inline double score(const model_t& model,
                    const dataset_t& data,
                    size_t row,
                    const double* p)
{ return smooth(model, data, row, p).score; }

inline double score(const dataset_t& data, size_t row, const double* p)
{ return smooth(engine_model(), data, row, p).score; }


// Mean squared error over [first, last) of `index`, against `target`. Which
// target is the caller's choice and it is load bearing: the blended one is
// what the fit descends, the game result alone is what compares two lambdas.
// DEC-064. Strided by thread and summed in thread order, as
// tools/tuner_model.hpp's was, so a run is reproducible at a given --threads.
inline double error_range(const model_t& model,
                          const dataset_t& data,
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
        const size_t row = index[i];
        const double diff =
            target[row] - sigmoid(score(model, data, row, params), k);
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

inline double error_range(const dataset_t& data,
                          const double* params,
                          double k,
                          const std::vector<uint32_t>& index,
                          size_t first,
                          size_t last,
                          unsigned threads,
                          const std::vector<double>& target)
{
  return error_range(engine_model(), data, params, k, index, first, last,
                     threads, target);
}


// The full-batch gradient of error_range() over [first, last), against
// `target`. One loop over a row's trace, whatever the term: a term the trace
// records is a term this differentiates, which is the failure the old
// per-term blocks had three times. The groups: the chain rule through each
// side's transform.
//
// **The clamp is differentiated exactly, and that reverses the parent's
// treatment.** Until S262 the tuner's gradient (tools/tuner_model.hpp) ignored
// it on purpose -- "treating the boundary as flat would stop the fit moving a
// weight it should move" -- while its forward model clamped, so the gradient
// was not the derivative of the error it reported. Here a row on which the
// tapered mobility-plus-king-safety sum lies beyond LAZY_EVAL_MARGIN gives
// those weights nothing, because they do not move its score there: the true
// gradient of the objective the fit minimises, which S262's accepts (3) checks
// against finite differences. The clamp binds on 0.338 % of the dedup corpus's
// rows at the shipped weights (36503 of 10795695), and S262's held-out
// comparison (6)
// -- 0.118446 against the old path's 0.118449 -- was taken under this
// treatment. S039 read H0 and kept the clamp (record ca578de, revert 74d0adf),
// so the treatment stays; S122's design, written for an unclamped king safety,
// took the owner's amendment before S121 started (DEC-258): DEC-324 keeps the
// shortcut and the clamp, and S122 re-sizes LAZY_EVAL_MARGIN from its own
// fitted spread. At exactly the margin the inside derivative is taken.
inline void gradient(const model_t& model,
                     const dataset_t& data,
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
  const size_t width = model.param_count;
  std::vector<std::vector<double>> partial(threads,
                                           std::vector<double>(width, 0.0));
  std::vector<std::thread> workers;

  for (unsigned t = 0; t < threads; ++t) {
    workers.emplace_back([&, t]() {
      const std::vector<feature_t>& features = feature_table().features;
      std::vector<double>& grad = partial[t];

      for (size_t i = first + t; i < last; i += threads) {
        const size_t row = index[i];
        const double phase = data.phase[row];
        const double mg_weight = phase / GAME_PHASE_MAX;
        const double eg_weight = (GAME_PHASE_MAX - phase) / GAME_PHASE_MAX;

        const smooth_t s = smooth(model, data, row, params);
        const double sig = sigmoid(s.score, k);

        // d/ds of (target - sigma)^2 through the sigmoid. S075: the target is
        // a constant of the row whatever lambda built it.
        const double outer =
            -2.0 * (target[row] - sig) * sig * (1.0 - sig) * k * LN10_OVER_400;

        for (uint32_t j = data.offsets[row]; j < data.offsets[row + 1]; ++j) {
          const entry_t e = data.entries[j];
          const feature_t& f = features[e.feature];
          const double c = outer * e.coef;

          if (f.site == SITE_MATERIAL) {
            grad[f.mg] += c;
            continue;
          }

          if (s.clamp_binds && clamped_site(f.site)) { continue; }

          grad[f.mg] += c * mg_weight;
          grad[f.eg] += c * eg_weight;
        }

        for (size_t g = 0; g < model.groups.size(); ++g) {
          const group_t& group = model.groups[g];
          const group_sums_t x = group_sums(group, g, data, row, params);

          double slope_mg[2];
          double slope_eg[2];
          for (int side = 0; side < 2; ++side) {
            slope_mg[side] =
                transform_slope(group.transform, group.divisor, x.mg[side]);
            slope_eg[side] =
                transform_slope(group.transform, group.divisor, x.eg[side]);
          }

          for (uint32_t j = data.side_offsets[row];
               j < data.side_offsets[row + 1]; ++j) {
            const side_entry_t e = data.side_entries[j];

            if (e.group != g) { continue; }

            // White's transform is added and Black's subtracted.
            const double side_sign = (e.side == 0) ? 1.0 : -1.0;
            const double c = outer * group.sign * side_sign * e.coef;
            const std::pair<uint32_t, uint32_t>& in = group.inputs[e.input];

            grad[in.first] += c * mg_weight * slope_mg[e.side];
            grad[in.second] += c * eg_weight * slope_eg[e.side];
          }
        }
      }
    });
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  out->assign(width, 0.0);

  for (const std::vector<double>& part : partial) {
    for (size_t i = 0; i < width; ++i) {
      (*out)[i] += part[i];
    }
  }

  for (size_t i = 0; i < width; ++i) {
    (*out)[i] /= static_cast<double>(count);
  }
}

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
  gradient(engine_model(), data, params, k, index, first, last, threads, target,
           out);
}


// ------------------------------------------------- a group's starting point
//
// A group of max(0, x)^2 started at zero weights never moves: every side's sum
// is 0, the transform's slope there is exactly 0, and so is every gradient the
// fit will ever see. DEC-134 form (b), a derivation over chesso's own data:
// seed each input so the mean of a side's pre-transform sum over the rows
// [first, last) of `index` -- both sides of every row -- is the transform's
// own unit, the x at which it reads one centipawn (sqrt(divisor) here), each
// input contributing an equal share of it. Every side then sits on the
// transform's rising part on average, with a slope of 2 / sqrt(divisor), and
// the fit has a gradient to follow. Middlegame and endgame halves are seeded
// alike. An input whose mean coefficient is not positive cannot raise a sum
// from there and is seeded at zero.
inline void derive_seed(const model_t& model,
                        size_t g,
                        const dataset_t& data,
                        const std::vector<uint32_t>& index,
                        size_t first,
                        size_t last,
                        double* params)
{
  const group_t& group = model.groups[g];
  std::vector<double> sum(group.inputs.size(), 0.0);
  double sides = 0.0;

  for (size_t i = first; i < last; ++i) {
    const size_t row = index[i];
    sides += 2.0;

    for (uint32_t j = data.side_offsets[row]; j < data.side_offsets[row + 1];
         ++j) {
      const side_entry_t e = data.side_entries[j];
      if (e.group == g) { sum[e.input] += e.coef; }
    }
  }

  size_t active = 0;
  for (const double value : sum) {
    if (value > 0.0) { active++; }
  }

  const double unit = std::sqrt(group.divisor);

  for (size_t input = 0; input < group.inputs.size(); ++input) {
    const double mean = (sides > 0.0) ? sum[input] / sides : 0.0;
    const double seed =
        (mean > 0.0) ? unit / (static_cast<double>(active) * mean) : 0.0;

    params[group.inputs[input].first] = seed;
    params[group.inputs[input].second] = seed;
  }
}

}  // namespace tuner_trace
