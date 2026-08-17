#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cmath>
#include <set>
#include <string>
#include <vector>
#include "search.hpp"
#include "search_params.hpp"
#include "test_helpers.hpp"
#include "uci.hpp"

// S073. The parameter set the tune build exposes, and the one thing about it
// that can fail silently: a default that differs between the two builds.
//
// This binary is compiled twice, once per configuration, and every case below
// the first block runs in both. That is the whole mechanism: a test binary
// cannot be two builds at once, so both builds are compared against a third
// thing instead -- the rows `search_param_info()` returns from
// src/search_params.cpp, which sit outside every `#ifdef CHESSO_TUNE` and are
// therefore the same rows in both. live == defaults in the release build and
// live == defaults in the tune build gives live == live, member by member,
// through that common anchor.
//
//   cmake --build build      -j12 && ctest --test-dir build      -R
//   test_search_params cmake --build build-tune -j12 && ctest --test-dir
//   build-tune -R test_search_params


// What the parameters are today, written out by hand rather than read from the
// table. The table is generated from the same list the engine compiles, so a
// value changed there would move the engine and the table together and every
// comparison above would still hold. This is the row that would go red.
//
// A step that deliberately changes a value updates this list in the same
// commit, which is the point: S068, S039 and S085 each move exactly one number
// and have to say so here.
// clang-format off
struct golden_param_t { const char* name; int value; };

static const std::vector<golden_param_t> golden_defaults = {
  {"OrderHistoryMax", 600000},
  {"MaxQsearchDepth", 8},
  {"RfpMargin",       75},
  {"RfpMaxDepth",     6},
  {"RfpMinPly",       3},
  {"NullMoveBase",    2},
  {"NullMoveDivisor", 6},
  {"LmrBase",         75},
  {"LmrDivisor",      225},
  {"LazyEvalMargin",  150},
  {"AspirationMinDepth", 5},
  {"AspirationDelta",    50},
  {"AspirationMaxDelta", 400},
};
// clang-format on


static int index_of(const std::string& name)
{
  for (size_t i = 0; i < search_param_count(); ++i) {
    if (name == search_param_info(i).name) { return static_cast<int>(i); }
  }

  return -1;
}


TEST_SUITE("search parameters")
{
  TEST_CASE("the table is populated and every name is distinct")
  {
    // The precondition for everything below. An empty table would satisfy every
    // member-by-member comparison in this file by having no members.
    REQUIRE(search_param_count() == golden_defaults.size());

    std::set<std::string> names;

    for (size_t i = 0; i < search_param_count(); ++i) {
      const search_param_t& param = search_param_info(i);

      REQUIRE(param.name != nullptr);
      CHECK_MESSAGE(
          names.insert(param.name).second,
          ("Two parameters share the name [" + std::string(param.name) + "]"));

      CHECK_MESSAGE(param.min_value <= param.max_value,
                    ("Empty range on [" + std::string(param.name) + "]"));
      CHECK_MESSAGE(param.default_value >= param.min_value,
                    ("Default below min on [" + std::string(param.name) + "]"));
      CHECK_MESSAGE(param.default_value <= param.max_value,
                    ("Default above max on [" + std::string(param.name) + "]"));
    }
  }


  TEST_CASE("both builds start from the same defaults, member by member")
  {
    REQUIRE(search_param_count() == golden_defaults.size());

    // The build's live values, with no setoption sent. In the release build
    // these are the constants the search folded; in the tune build they are the
    // variables it loads. Both are held against rows that are compiled the same
    // way in both, so agreeing here means the two builds agree with each other.
    for (size_t i = 0; i < search_param_count(); ++i) {
      const search_param_t& param = search_param_info(i);

      CHECK_MESSAGE(search_param_value(i) == param.default_value,
                    ("[" + std::string(param.name) + "] starts at " +
                     std::to_string(search_param_value(i)) +
                     " in this build but the shared default is " +
                     std::to_string(param.default_value) +
                     ". The two builds no longer agree."));
    }
  }


  TEST_CASE("the defaults are the values the engine ships")
  {
    REQUIRE(search_param_count() == golden_defaults.size());

    for (size_t i = 0; i < golden_defaults.size(); ++i) {
      const int index = index_of(golden_defaults[i].name);

      REQUIRE_MESSAGE(index >= 0, ("No parameter named [" +
                                   std::string(golden_defaults[i].name) + "]"));

      const search_param_t& param =
          search_param_info(static_cast<size_t>(index));

      CHECK_MESSAGE(param.default_value == golden_defaults[i].value,
                    ("[" + std::string(param.name) + "] is " +
                     std::to_string(param.default_value) + ", not the " +
                     std::to_string(golden_defaults[i].value) +
                     " it shipped at. Changing a value is a step of its own "
                     "and updates this list in the same commit."));
    }
  }


#ifndef CHESSO_TUNE
  TEST_CASE("the release build has no setter at all")
  {
    // search_param_set() does not exist here, so there is nothing to call. What
    // is checkable is that the option surface did not move, and
    // test_uci_surface holds that against MANUAL.md by name. This case exists
    // so the file records which half of the pair is compiled.
    CHECK(search_param_count() > 0);
  }
#endif


#ifdef CHESSO_TUNE
  TEST_CASE("setoption moves a parameter and the search sees the new value")
  {
    uci_init();

    const int index = index_of("RfpMargin");
    REQUIRE(index >= 0);

    const size_t i = static_cast<size_t>(index);

    // Precondition: it starts where the table says, so a value read after the
    // setoption is evidence the setoption did something.
    REQUIRE(search_param_value(i) == search_param_info(i).default_value);
    REQUIRE(RFP_MARGIN == search_param_info(i).default_value);

    uci_process_line("setoption name RfpMargin value 55");

    CHECK(search_param_value(i) == 55);
    CHECK(RFP_MARGIN == 55);

    uci_process_line("setoption name RfpMargin value " +
                     std::to_string(search_param_info(i).default_value));

    CHECK(RFP_MARGIN == search_param_info(i).default_value);

    uci_shutdown();
  }


  TEST_CASE("every parameter in the table is settable over UCI")
  {
    uci_init();

    for (size_t i = 0; i < search_param_count(); ++i) {
      const search_param_t& param = search_param_info(i);

      // One step towards the middle of the range, so the value is always legal
      // and always different from the default.
      const int target = (param.default_value < param.max_value)
                             ? param.default_value + 1
                             : param.default_value - 1;

      REQUIRE_MESSAGE(target != param.default_value,
                      ("[" + std::string(param.name) +
                       "] has a range of one value and nothing to set"));

      uci_process_line("setoption name " + std::string(param.name) + " value " +
                       std::to_string(target));

      CHECK_MESSAGE(
          search_param_value(i) == target,
          ("[" + std::string(param.name) + "] did not take a setoption"));

      uci_process_line("setoption name " + std::string(param.name) + " value " +
                       std::to_string(param.default_value));

      CHECK(search_param_value(i) == param.default_value);
    }

    uci_shutdown();
  }


  TEST_CASE("a value outside the declared range is refused, not clamped")
  {
    const int index = index_of("NullMoveDivisor");
    REQUIRE(index >= 0);

    const size_t i = static_cast<size_t>(index);
    const search_param_t& param = search_param_info(i);

    // Precondition: a value inside the range is taken, so a rejection below is
    // the range doing its job rather than the setter being broken.
    REQUIRE(search_param_set(param.name, param.min_value));
    REQUIRE(search_param_value(i) == param.min_value);

    CHECK(!search_param_set(param.name, param.min_value - 1));
    CHECK(search_param_value(i) == param.min_value);

    CHECK(!search_param_set(param.name, param.max_value + 1));
    CHECK(search_param_value(i) == param.min_value);

    CHECK(!search_param_set("NoSuchParameter", 1));

    REQUIRE(search_param_set(param.name, param.default_value));
  }


  TEST_CASE("an LMR coefficient rebuilds the table it was baked into")
  {
    // The trap this case exists for: LMR_BASE and LMR_DIVISOR are read once,
    // when the reduction table is built. A setter that moved a coefficient and
    // left the table alone would report success, pass every value assertion
    // above, and change nothing the search does.
    const int base = index_of("LmrBase");
    const int divisor = index_of("LmrDivisor");

    REQUIRE(base >= 0);
    REQUIRE(divisor >= 0);

    const int base_default =
        search_param_info(static_cast<size_t>(base)).default_value;
    const int divisor_default =
        search_param_info(static_cast<size_t>(divisor)).default_value;

    // The formula, restated from src/search.cpp's build_lmr_table(). The
    // expected values below are computed here rather than read off the engine,
    // so a table that never moved cannot supply its own expectation.
    auto expected = [](int b, int d, int depth, int move_number) {
      return static_cast<int>(static_cast<uint8_t>(
          (b / 100.0) +
          (std::log(depth) * std::log(move_number)) / (d / 100.0)));
    };

    const int probe_depth = 32;
    const int probe_move = 32;

    // Precondition: the table currently holds the default fit. Without this a
    // reduction that never changes would be indistinguishable from one that
    // was already at the target.
    REQUIRE(search_lmr_reduction_probe(probe_depth, probe_move) ==
            expected(base_default, divisor_default, probe_depth, probe_move));

    // Halving the divisor doubles the logarithmic term, which has to be a
    // different reduction at this depth or the case proves nothing.
    const int tighter = divisor_default / 2;
    REQUIRE(expected(base_default, tighter, probe_depth, probe_move) !=
            expected(base_default, divisor_default, probe_depth, probe_move));

    REQUIRE(search_param_set("LmrDivisor", tighter));

    CHECK(search_lmr_reduction_probe(probe_depth, probe_move) ==
          expected(base_default, tighter, probe_depth, probe_move));

    REQUIRE(search_param_set("LmrDivisor", divisor_default));

    CHECK(search_lmr_reduction_probe(probe_depth, probe_move) ==
          expected(base_default, divisor_default, probe_depth, probe_move));

    // The other coefficient, which shifts every entry by a constant.
    const int raised = base_default + 100;
    REQUIRE(expected(raised, divisor_default, probe_depth, probe_move) !=
            expected(base_default, divisor_default, probe_depth, probe_move));

    REQUIRE(search_param_set("LmrBase", raised));

    CHECK(search_lmr_reduction_probe(probe_depth, probe_move) ==
          expected(raised, divisor_default, probe_depth, probe_move));

    REQUIRE(search_param_set("LmrBase", base_default));

    CHECK(search_lmr_reduction_probe(probe_depth, probe_move) ==
          expected(base_default, divisor_default, probe_depth, probe_move));
  }


  TEST_CASE("the tune build declares one UCI option per parameter")
  {
    uci_init();

    stdout_capture_t capture;
    uci_process_line("uci");

    for (size_t i = 0; i < search_param_count(); ++i) {
      const search_param_t& param = search_param_info(i);

      const std::string line = "option name " + std::string(param.name) +
                               " type spin default " +
                               std::to_string(param.default_value) + " min " +
                               std::to_string(param.min_value) + " max " +
                               std::to_string(param.max_value);

      CHECK_MESSAGE(capture.contains(line),
                    ("The uci reply does not carry [" + line + "]"));
    }

    uci_shutdown();
  }
#endif
}
