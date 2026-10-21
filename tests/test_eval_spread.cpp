#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cstring>
#include <set>
#include <string>
#include <vector>
#include "eval_spread_margins.hpp"
#include "search_params.hpp"

// tools/eval_spread's candidate margins. S288, 2026-09-10_adversarial-F25: the
// list was {150, 200, 250, 300, 400} while the margin shipped at 184, so the
// tool whose job is to judge the margin never reported the one in use.
//
// What this file holds:
//
//   1. the shipped margin is the first candidate, exactly once
//   2. the fixed comparison points are all still there, and no value is listed
//      twice
//   3. tune build only: the first candidate follows LazyEvalMargin when it is
//      set, so it is read and not a typed 184 that happens to match today
//
// Case 1 compares against the default in search_param_info(), found by UCI
// name, not against LAZY_EVAL_MARGIN, the symbol the header reads: two reads of
// the same list, so neither side can agree with itself.

namespace
{

int shipped_margin()
{
  for (size_t i = 0; i < search_param_count(); ++i) {
    if (std::strcmp(search_param_info(i).name, "LazyEvalMargin") == 0) {
      return search_param_info(i).default_value;
    }
  }

  return -1;
}


// Not goldens: the tool's fixed comparison points, chosen rather than
// measured, and the same five it printed before S288.
const std::vector<int> FIXED = {150, 200, 250, 300, 400};


long count_of(const std::vector<int>& margins, int value)
{ return std::count(margins.begin(), margins.end(), value); }

}  // namespace


TEST_SUITE("eval_spread margins")
{
  TEST_CASE("the shipped margin is the first candidate")
  {
    const int shipped = shipped_margin();
    REQUIRE(shipped >= 0);

    const std::vector<int> margins = eval_spread::candidate_margins();
    REQUIRE(!margins.empty());

    CHECK(margins.front() == shipped);
    CHECK(count_of(margins, shipped) == 1);
  }


  TEST_CASE("the fixed points stay and no candidate is listed twice")
  {
    const std::vector<int> margins = eval_spread::candidate_margins();
    const std::set<int> distinct(margins.begin(), margins.end());

    CHECK(distinct.size() == margins.size());

    for (const int margin : FIXED) {
      CHECK_MESSAGE(count_of(margins, margin) == 1,
                    ("fixed point " + std::to_string(margin) + " missing"));
    }
  }


#ifdef CHESSO_TUNE
  TEST_CASE("the first candidate follows the parameter when it is set")
  {
    const int shipped = shipped_margin();
    REQUIRE(shipped >= 0);
    REQUIRE(LAZY_EVAL_MARGIN == shipped);

    // 777 is inside LazyEvalMargin's range and is neither the shipped value nor
    // a fixed point, so it can reach the list only by being read.
    REQUIRE(search_param_set("LazyEvalMargin", 777));

    const std::vector<int> moved = eval_spread::candidate_margins();
    REQUIRE(!moved.empty());
    CHECK(moved.front() == 777);
    CHECK(moved.size() == FIXED.size() + 1);

    // A set onto a fixed point exercises the deduplication, which the shipped
    // value does not reach unless a refit lands on one of the five.
    REQUIRE(search_param_set("LazyEvalMargin", 200));

    const std::vector<int> landed = eval_spread::candidate_margins();
    REQUIRE(!landed.empty());
    CHECK(landed.front() == 200);
    CHECK(count_of(landed, 200) == 1);
    CHECK(landed.size() == FIXED.size());

    REQUIRE(search_param_set("LazyEvalMargin", shipped));
    CHECK(eval_spread::candidate_margins().front() == shipped);
  }
#endif
}
