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
// state and needs nothing but the layout constants in eval_model.hpp. The
// using-declarations below are what let the function body stay byte-identical
// to the one that lived in tuner.cpp; they are visible as tuner_groups::MG_BASE
// and so on, which is a small price for a move that changed no line of the
// body.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "eval_model.hpp"

namespace tuner_groups
{

using eval_model::KS_MG_BASE;
using eval_model::MATERIAL_COUNT;
using eval_model::MG_BASE;
using eval_model::MOB_MG_BASE;
using eval_model::PARAM_COUNT;
using eval_model::PL_MG_BASE;
using eval_model::PP_MG_BASE;
using eval_model::PS_MG_BASE;
using eval_model::TEMPO_MG_BASE;


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
// Each group is one contiguous range because eval_model.hpp lays the vector out
// that way.
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

}  // namespace tuner_groups
