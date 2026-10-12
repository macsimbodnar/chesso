#pragma once
// The tuner's `--only` groups and the parameter range each one frees. S041.
//
// Split out of tools/tuner.cpp so a test can reach it. The function below has
// swallowed the term appended after it four times -- its own comments record
// all four -- and nothing in tests/ linked tuner.cpp at all
// (2026-08-13_adversarial-F07). tests/test_tuner_groups.cpp now holds the three
// properties the ranges have to have: non-empty, pairwise disjoint, and a union
// of exactly [0, PARAM_COUNT).
//
// A header rather than a second translation unit because free_mask() has no
// state and needs nothing but the layout constants, in tools/tuner_layout.hpp
// since S262 (tools/eval_model.hpp until then). The
// using-declarations below are what let the function body stay byte-identical
// to the one that lived in tuner.cpp; they are visible as tuner_groups::MG_BASE
// and so on, which is a small price for a move that changed no line of the
// body.
//
// Since S275 it also holds the seven gauges, the null directions a fit keeps
// still, because whether one is held depends on the same mask.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "tuner_layout.hpp"

namespace tuner_groups
{

using tuner_layout::EG_BASE;
using tuner_layout::KS_MG_BASE;
using tuner_layout::MATERIAL_COUNT;
using tuner_layout::MG_BASE;
using tuner_layout::MOB_MG_BASE;
using tuner_layout::PARAM_COUNT;
using tuner_layout::PL_MG_BASE;
using tuner_layout::PP_MG_BASE;
using tuner_layout::PS_MG_BASE;
using tuner_layout::TEMPO_MG_BASE;


inline constexpr const char* GROUP_LIST =
    "all, material, psqt, mobility, king_safety, passed_pawns, "
    "pawn_structure, piece_placement, tempo";


// Marks the parameters a --only group leaves free; the rest keep the value the
// engine ships and come out of the run unchanged.
//
// This exists for attribution under SPRT. Fitting a new term jointly also
// refits the 781 constants that were already fitted, so the match that follows
// measures two changes at once and its number says nothing about either. One
// change at a time; DEC-020 is what that rule cost to learn.
//
// Each group is one contiguous range because tools/tuner_layout.hpp lays the
// vector out that way.
inline bool free_mask(const std::string& group, std::vector<uint8_t>* mask)
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
    // Ends at the tempo block for the reason above, and it reached to
    // PARAM_COUNT until that block was appended -- the fourth term in a row to
    // meet the same defect in the same function.
    first = PL_MG_BASE;
    last = TEMPO_MG_BASE;
  } else if (group == "tempo") {
    first = TEMPO_MG_BASE;
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


// Holds the named groups at what the engine ships and leaves everything else as
// the mask found it. The inverse of --only, which frees one group: this frees
// all of them but a few. S065 needed "everything except tempo and
// piece_placement" and --only could only say "tempo" or "piece_placement".
//
// It does not touch the partition above -- it selects among the same ranges
// free_mask already defines, so test_tuner_groups' three properties are
// unaffected by anything here. A name free_mask refuses is refused here too,
// and an empty list is a no-op, which is what keeps the default behaviour of a
// run that passes no --freeze byte-identical.
//
// Why freeze during the fit rather than zero two groups afterwards: with the
// two groups held, the remaining 815 absorb what they would have taken (817 at
// S065, before S134 narrowed the vector to 823). Fitting all 823 and then
// zeroing the two leaves those 815 fitted against values that are no longer
// there. DEC-057.
inline bool freeze_mask(const std::string& groups, std::vector<uint8_t>* mask)
{
  if (mask->size() != PARAM_COUNT) { return false; }

  std::string name;
  std::vector<uint8_t> group_mask;

  // One pass over the list, closing each name at a comma and at the end.
  for (size_t i = 0; i <= groups.size(); ++i) {
    const char c = (i < groups.size()) ? groups[i] : ',';

    if (c == ' ') { continue; }

    if (c != ',') {
      name.push_back(c);
      continue;
    }

    if (name.empty()) { continue; }

    // `all` is accepted by free_mask and would freeze the whole vector, which
    // is a run that fits nothing and reports the starting error. Refused here
    // rather than obeyed, because nothing wants it and a typo that produces it
    // is silent.
    if (name == "all") { return false; }

    if (!free_mask(name, &group_mask)) { return false; }

    for (size_t j = 0; j < PARAM_COUNT; ++j) {
      if (group_mask[j]) { (*mask)[j] = 0; }
    }

    name.clear();
  }

  return true;
}


// The evaluation's exact null directions, and where a fit holds them. S275.
//
// tools/feature_audit's identity report puts the design at rank 784 of 823
// (S134). Thirty-two of the 39 null directions are single pawn columns on ranks
// 1 and 8, which no row can touch -- load() refuses a pawn there -- and Adam
// leaves those exactly where they started: a gradient of exactly zero keeps the
// moment and the velocity at zero, and the step is 0 / (0 + epsilon). The other
// seven are spread over many columns, and there Adam's per-coordinate scaling
// does not keep a step orthogonal to them, so a fit walks along them for
// nothing: no evaluation moves, but up to 129 shipped numbers do, and two fits'
// tables stop being comparable. These are the seven:
//
//   material/table, one per piece but the king: piece_value[t] + c and every
//     square of psqt_mg[t] and psqt_eg[t] a piece of type t can stand on - c.
//     The taper's two weights sum to one, so no piece's score moves.
//   psqt_mg[king] and psqt_eg[king]: every square + c. One king a side, so the
//     White king's + c and the Black king's - c cancel before the taper; load()
//     refuses a row without one king a side, as load_FEN() does.
//
// The gauge picks one point on each of those lines: the level -- the sum over
// the squares its direction moves, psqt_mg[t] and psqt_eg[t] together for a
// piece, one king table's own sum for the king -- stays at the value the fit
// started from, and a piece's value carries the shift. With material and psqt
// both free, a fit's tables then compare square by square with the ones it
// started from, and a change in what a piece is worth shows up in its define,
// where a reader looks for it. Under --only psqt or --freeze material only the
// two king sums are held: the values are frozen, so the tables carry any change
// in a piece's worth, as they always did.
inline constexpr size_t KING_TYPE = 5;

struct gauge_t
{
  std::string name;
  std::vector<size_t> level;  // the columns whose sum is held
  size_t absorb =
      PARAM_COUNT;  // the column the shift goes to; PARAM_COUNT: none
};


// The model's square, mirrored for Black, so one test covers both colours: 0..7
// is a pawn's own eighth rank and 56..63 its own first.
inline bool pawn_can_stand(size_t square)
{ return square >= 8 && square < 56; }


// The seven, in the order feature_audit reports them. The pawn's level leaves
// out the 32 columns no pawn can reach, so a hold never moves them either.
inline std::vector<gauge_t> gauges()
{
  static const char* const names[KING_TYPE + 1] = {"pawn", "knight", "bishop",
                                                   "rook", "queen",  "king"};
  const size_t bases[2] = {MG_BASE, EG_BASE};

  std::vector<gauge_t> out;

  for (size_t type = 0; type < MATERIAL_COUNT; ++type) {
    gauge_t gauge;
    gauge.name = std::string("material/table ") + names[type];
    gauge.absorb = type;

    for (const size_t base : bases) {
      for (size_t square = 0; square < 64; ++square) {
        if (type == 0 && !pawn_can_stand(square)) { continue; }
        gauge.level.push_back(base + type * 64 + square);
      }
    }

    out.push_back(gauge);
  }

  for (const size_t base : bases) {
    gauge_t gauge;
    gauge.name = (base == MG_BASE) ? "psqt_mg[king] sum" : "psqt_eg[king] sum";

    for (size_t square = 0; square < 64; ++square) {
      gauge.level.push_back(base + KING_TYPE * 64 + square);
    }

    out.push_back(gauge);
  }

  return out;
}


// The gauges a run holds: those whose every column the mask leaves free. A
// direction with a frozen column needs no hold -- the free parameters cannot
// move along it without moving that column -- and holding it anyway would write
// a parameter --only or --freeze promised to leave bit-identical.
inline std::vector<gauge_t> held_gauges(const std::vector<uint8_t>& mask)
{
  std::vector<gauge_t> out;

  for (const gauge_t& gauge : gauges()) {
    bool free = (gauge.absorb == PARAM_COUNT) || mask[gauge.absorb] != 0;

    for (const size_t column : gauge.level) {
      free = free && mask[column] != 0;
    }

    if (free) { out.push_back(gauge); }
  }

  return out;
}


// Puts each held gauge's level back where `start` had it and moves the
// difference into the piece's value: c off every column of the level and onto
// the absorbing one, a move along the null direction and so invisible to every
// evaluation. Called after each step, so no vector a fit reports or emits is
// ever off the gauge.
inline void hold_gauges(const std::vector<gauge_t>& held,
                        const double* start,
                        double* params)
{
  for (const gauge_t& gauge : held) {
    double drift = 0.0;

    for (const size_t column : gauge.level) {
      drift += params[column] - start[column];
    }

    drift /= static_cast<double>(gauge.level.size());

    for (const size_t column : gauge.level) {
      params[column] -= drift;
    }

    if (gauge.absorb < PARAM_COUNT) { params[gauge.absorb] += drift; }
  }
}

}  // namespace tuner_groups
