#pragma once
// The margins tools/eval_spread counts exceedances at, the shipped one first.
// S288, 2026-09-10_adversarial-F25.
//
// Until S288 the list was typed out as {150, 200, 250, 300, 400} under a
// comment calling 150 current, while the margin shipped at 184 from S085 on:
// the value the tool exists to judge was the one it did not report. The first
// entry is now LAZY_EVAL_MARGIN, the symbol src/search_params.hpp's list
// generates, so a refit moves the row with it. It is also the margin the
// tool's own clamp check uses, so that row's combined column counts the
// positions the clamp binds on.
//
// In the tune build LAZY_EVAL_MARGIN is a variable. eval_spread never sets it,
// so it holds the list's default there as well; tests/test_eval_spread.cpp
// sets it to show the entry is read and not typed.
#include <vector>
#include "search_params.hpp"

namespace eval_spread
{

inline std::vector<int> candidate_margins()
{
  std::vector<int> margins = {LAZY_EVAL_MARGIN};

  // Fixed comparison points, kept from before S288. One equal to the shipped
  // margin is dropped so that no row is printed twice.
  for (const int margin : {150, 200, 250, 300, 400}) {
    if (margin != LAZY_EVAL_MARGIN) { margins.push_back(margin); }
  }

  return margins;
}

}  // namespace eval_spread
