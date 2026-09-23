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
// commit, which is the point: S068 and S039 each move exactly one number and
// have to say so here. S085 moved ten at once -- an SPSA run returns a vector,
// not a value -- and the rule is the same for a vector as for a number. S222's
// history lane moved eleven: QuietHistoryMax, plain history's six bonus and
// malus coefficients, the three continuation axes and HistPruneCoeff. A step
// that **adds** one adds a row here in the same commit, which is what S095's
// LmrNoTtMove is doing below, and S097's six singular-extension rows after
// it -- four settings and two switches, `SeExtend` and `SeMultiCut`, whose
// defaults are what the step's two verdicts move. **S097 verdict 2 moves
// `SeMultiCut` from 0 to 1** and that one row is the whole of the candidate in
// `src/`: the rule it turns on shipped with verdict 1 and no line of it moved.
// S236 adds three: `LmrRoundBias`, which is the accumulator's rounding rule
// and whose range is stated against a compile-time scale in src/search.cpp,
// and `LmrHistDiv` and `LmrHistClamp`, whose two values are that step's own
// census and not S098 verdict 1's fitted pair (DEC-213).
//
// The ranges are held here too, since S142. They had nothing holding them at
// all: the release build never reads a bound, the tune build's option lines are
// generated from this same table so test_uci_surface compares it against
// itself, and MANUAL.md's range column is prose no test parses. So a bound
// could move -- or fail to move when the reason for it did -- and the whole
// suite would stay green. Two of them were wrong on exactly that account,
// 2026-08-20_plan_review-F08 and -F14, and both are now numbers a diff has to
// change on purpose. What a bound *means* is not checkable here and is not
// meant to be: RfpMinPly's floor is asserted by the mate suite in test_engine
// and QuietHistoryMax's two edges by the band clearance in test_evaluation.
//
// GOLDEN (DEC-142): the 63 defaults and their ranges below. A deliberate-change
// detector rather than a measurement -- there is no script and none is owed,
// because src/search_params.hpp is the derivation and a diff of the two is the
// re-derivation. A step that moves a default edits both in the same commit.
// Moves legitimately on: a step that moves a default or a bound.
// Margin: exact, on every one of the three columns.
// Property beside it: "the table is populated and every name is distinct" and
// "both builds start from the same defaults, member by member", which hold
// whatever the values are.
// clang-format off
struct golden_param_t { const char* name; int value; int min; int max; };

static const std::vector<golden_param_t> golden_defaults = {
  //                       default   min      max
  {"QuietHistoryMax",        8831,     1,   32767},
  {"HistoryBonusQuad",          6,     0,    1024},
  {"HistoryBonusLin",          19,     0,    4096},
  {"HistoryBonusConst",         2, -32768,   32767},
  {"HistoryMalusQuad",          0,     0,    1024},
  {"HistoryMalusLin",          17,     0,    4096},
  {"HistoryMalusConst",        36, -32768,   32767},
  {"ContHistBonus",            17,     0,    1000},
  {"ContHistMalus",            18,     0,    1000},
  {"ContHistWeight",           26,     0,    2000},
  {"MaxQsearchDepth",          19,     1,      64},
  {"RfpMargin",                63,     0,    2000},
  {"RfpMaxDepth",              15,     0,      63},
  {"RfpMinPly",                 3,     2,      63},
  {"NullMoveBase",              3,     0,      16},
  {"NullMoveDivisor",           6,     1,      64},
  {"LmrBase",                  52,     0,     400},
  {"LmrDivisor",              182,     1,    2000},
  {"LmrRoundBias",              0,     0,    1023},
  {"LmrHistDiv",              734,     1, 35532800},
  {"LmrHistClamp",           1024,     0,    2048},
  {"LmrCutNode",                1,     0,       2},
  {"LmrNotImproving",           1,     0,       2},
  {"LmrTtCapture",              1,     0,       2},
  {"LmrPv",                     1,     0,       2},
  {"LmrNoTtMove",               1,     0,       2},
  {"LmrDeeperMargin",          47,     0,      94},
  {"LmrDeeperMinReduction",     2,     1,     126},
  {"LmpBase",                 733,     0,   27000},
  {"LmpDepthCoeff",             0,     0,   27000},
  {"LmpMaxLmrDepth",            8,     0,      16},
  {"FutBase",                 147,     0,   48000},
  {"FutSlope",                170,     0,    2000},
  {"FutMaxLmrDepth",            8,     0,      16},
  {"HistPruneCoeff",          612,     0,   16384},
  {"HistPruneMaxLmrDepth",      8,     0,      16},
  {"SeeQuietCoeff",            50,     0,   10000},
  {"SeeQuietMaxLmrDepth",       8,     0,      16},
  {"SeeCaptureCoeff",          50,     0,   10000},
  {"SeeCaptureMaxLmrDepth",     8,     0,      16},
  {"SeeLmrExtra",               1,     0,       3},
  {"SeExtend",                  1,     0,       1},
  {"SeMinDepth",               10,     4,      16},
  {"SeTtDepthMargin",           4,     0,       8},
  {"SePlyFactor",               5,     2,       8},
  {"SeMarginPerDepth",          9,     1,      18},
  {"SeMultiCut",                1,     0,       1},
  {"LazyEvalMargin",          184,     0,    2000},
  {"AspirationMinDepth",        2,     2,      64},
  {"AspirationDelta",          21,     1,    2000},
  {"AspirationMaxDelta",      437,     1,   48000},
  {"TmSoftPercent",            60,     1,     100},
  {"TmHardPercent",           300,   100,    1000},
  {"TmSuddenDeathPercent",      5,     1,     100},
  {"TmIncrementPercent",       50,     0,     100},
  {"TmStabilityMax",            8,     0,     126},
  {"TmStabilityPercent",        4,     0,      50},
  {"TmFallingMaxCp",          100,     1,    2000},
  {"TmFallingPercent",         50,     0,     400},
  {"TmScaleMinPercent",        30,     1,     100},
  {"TmNodeBasePct",           120,   100,     400},
  {"TmNodeScalePct",          151,     0,     300},
  {"TmNodeMinDepth",            2,     0,      64},
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


  TEST_CASE("the declared ranges are the ranges the engine ships")
  {
    // S142. A bound is metadata that only `search_param_set()` reads, so
    // nothing else in the suite notices one moving -- see the note above the
    // table. Held by name and not by position, like the defaults, so a
    // reordering of the list is not a failure and a rename is.
    REQUIRE(search_param_count() == golden_defaults.size());

    for (size_t i = 0; i < golden_defaults.size(); ++i) {
      const int index = index_of(golden_defaults[i].name);

      REQUIRE_MESSAGE(index >= 0, ("No parameter named [" +
                                   std::string(golden_defaults[i].name) + "]"));

      const search_param_t& param =
          search_param_info(static_cast<size_t>(index));

      CHECK_MESSAGE(
          param.min_value == golden_defaults[i].min,
          ("[" + std::string(param.name) + "] declares a minimum of " +
           std::to_string(param.min_value) + ", not the " +
           std::to_string(golden_defaults[i].min) +
           " this list holds. A bound is a claim about what the "
           "parameter may be set to and moves in its own commit."));

      CHECK_MESSAGE(
          param.max_value == golden_defaults[i].max,
          ("[" + std::string(param.name) + "] declares a maximum of " +
           std::to_string(param.max_value) + ", not the " +
           std::to_string(golden_defaults[i].max) +
           " this list holds. A bound is a claim about what the "
           "parameter may be set to and moves in its own commit."));
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


  // S209. The protocol's case rule is about the name, and a parameter's name is
  // an option name like any other: `UCI.txt` says the name "should not be case
  // sensitive", so the fold command_setoption applies to Hash and OwnBook
  // reaches this chain too. Until S209 `rfpmargin` came back as an unknown
  // option, which is a refusal the tuner driving this build had no way to
  // distinguish from a misspelling.
  TEST_CASE("a mis-cased parameter name is still the parameter")
  {
    uci_init();

    const int index = index_of("RfpMargin");
    REQUIRE(index >= 0);

    const size_t i = static_cast<size_t>(index);

    REQUIRE(search_param_value(i) == search_param_info(i).default_value);

    // Both directions from the canonical spelling, because the fold has to be
    // applied to the name in the table as well as the one that arrived.
    for (const std::string& name :
         {std::string("rfpmargin"), std::string("RFPMARGIN"),
          std::string("RfPmArGiN")}) {
      std::vector<std::string> printed;
      {
        stdout_capture_t capture;
        uci_process_line("setoption name " + name + " value 55");
        printed = capture.lines();
      }

      CHECK_MESSAGE(search_param_value(i) == 55,
                    ("[" + name + "] did not reach the parameter"));

      // A legal value prints nothing, so an honoured name is silent and the
      // unknown-option line is gone with it.
      CHECK_MESSAGE(
          printed.empty(),
          ("[" + name + "] was answered [" +
           (printed.empty() ? std::string() : printed.front()) + "]"));

      uci_process_line("setoption name RfpMargin value " +
                       std::to_string(search_param_info(i).default_value));

      REQUIRE(search_param_value(i) == search_param_info(i).default_value);
    }

    // The other half, which the fold must not swallow: a name that is not a
    // parameter in any casing is still refused.
    {
      stdout_capture_t capture;
      uci_process_line("setoption name RfpMargn value 55");

      const std::vector<std::string> printed = capture.lines();

      REQUIRE(printed.size() == 1);
      CHECK(printed.front() ==
            "info string refused [RfpMargn], unknown option");
    }

    CHECK(search_param_value(i) == search_param_info(i).default_value);

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

    // The formula, restated from src/search.cpp's build_lmr_table() **and from
    // the rounding rule that reads it**. The expected values below are computed
    // here rather than read off the engine, so a table that never moved cannot
    // supply its own expectation.
    //
    // Since S236 the table holds ticks -- LMR_SCALE of them to the ply -- and
    // `search_lmr_reduction_probe` reports the whole plies the engine would
    // use, which is `(ticks + LmrRoundBias) / scale`. The division is exact
    // against the engine's shift here because both operands are non-negative:
    // the formula is non-negative for every settable LmrBase and the bias is
    // bounded below by its own range. The truncating cast the engine used
    // before this step is held in its own case, at the off configuration.
    auto expected = [](int b, int d, int depth, int move_number) {
      const int scale = search_lmr_scale_probe();
      const double r =
          (b / 100.0) + (std::log(depth) * std::log(move_number)) / (d / 100.0);

      return (static_cast<int>(r * scale) + LMR_ROUND_BIAS) / scale;
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


  // Mutation: W02_table_unit_halved -- the table is built at half the scale the
  // sum is divided by, so every reduction is half what the formula says and the
  // off configuration stops reproducing the parent.
  //
  //   search parameters
  //    at the off configuration the reduction is the engine before S236
  //   CHECK_MESSAGE( first_mismatch.empty(), first_mismatch )
  //   values: written at this step's own mutation pass, from the observed red
  //   and not from a prediction
  TEST_CASE("at the off configuration the reduction is the engine before S236")
  {
    // DEC-215: an off value is proved on the tree and not assumed from a
    // range's end. This is the arithmetic half of that proof -- the whole
    // reduction, over the whole table, at every node adjustment and every
    // history sum -- and the tree half is the bench signature the step file
    // records beside it.
    //
    // S236's claim is exact and this case is written to be exact with it: with
    // `LmrRoundBias` at 0 and `LmrHistClamp` at 0, `lmr_adjusted_reduction` is
    // the truncated table plus whole plies of node adjustment, which is the
    // line the engine ran before the accumulator existed. That line is restated
    // below -- `static_cast<uint8_t>` of the double, exactly as
    // build_lmr_table() wrote it -- rather than derived from the tick table, so
    // an accumulator that is self-consistently wrong cannot pass.
    const int bias_default =
        search_param_info(static_cast<size_t>(index_of("LmrRoundBias")))
            .default_value;
    const int clamp_default =
        search_param_info(static_cast<size_t>(index_of("LmrHistClamp")))
            .default_value;

    const int base_default =
        search_param_info(static_cast<size_t>(index_of("LmrBase")))
            .default_value;
    const int divisor_default =
        search_param_info(static_cast<size_t>(index_of("LmrDivisor")))
            .default_value;

    REQUIRE(search_param_set("LmrRoundBias", 0));
    REQUIRE(search_param_set("LmrHistClamp", 0));

    // The `uint8_t` is the parent's own and not a narrowing this case chose:
    // at the shipped fit the table's largest value is under ten plies, so the
    // cast never wraps here and "the truncated formula" and "the parent's line"
    // are the same number. A fit that could reach 256 plies would be a
    // different question and this case would have to say so.
    auto parent = [&](int depth, int move_number) {
      return static_cast<int>(static_cast<uint8_t>(
          (base_default / 100.0) + (std::log(depth) * std::log(move_number)) /
                                       (divisor_default / 100.0)));
    };

    const int scale = search_lmr_scale_probe();

    // One string rather than a check per cell: the grid below is 63 * 63 * 3 *
    // 3 and a doctest assertion per cell would dominate the suite's runtime.
    // The first disagreement is reported in full, which is what a reader needs.
    std::string first_mismatch;

    for (int depth = 1; depth < 64 && first_mismatch.empty(); ++depth) {
      for (int move_number = 1; move_number < 64; ++move_number) {
        // Whole plies, in the unit lmr_node_adjustment hands over: the PV term
        // is the negative one and the other four are positive, so the range
        // covers a sum from either side of zero.
        for (int plies : {-1, 0, 2}) {
          // Any sum at all, including ones outside the band the tables can
          // hold: at the off clamp the term is 0 for every one of them, which
          // is the half of the off configuration the history rule owns.
          for (int hist : {0, 5000, -1000000}) {
            const int got = search_lmr_adjusted_reduction_probe(
                depth, move_number, plies * scale, hist);
            const int want = parent(depth, move_number) + plies;

            if (got != want) {
              first_mismatch =
                  "depth " + std::to_string(depth) + " move " +
                  std::to_string(move_number) + " plies " +
                  std::to_string(plies) + " hist " + std::to_string(hist) +
                  ": the off configuration answers " + std::to_string(got) +
                  " where the engine before S236 answered " +
                  std::to_string(want);
              break;
            }
          }

          if (!first_mismatch.empty()) { break; }
        }

        if (!first_mismatch.empty()) { break; }
      }
    }

    CHECK_MESSAGE(first_mismatch.empty(), first_mismatch);

    // Back to the shipped configuration, and asserted rather than assumed:
    // every case after this one in this binary reads the tree that ships, and
    // a restore that silently failed would leave them measuring the off
    // configuration instead.
    REQUIRE(search_param_set("LmrRoundBias", bias_default));
    REQUIRE(search_param_set("LmrHistClamp", clamp_default));

    CHECK_EQ(search_param_value(static_cast<size_t>(index_of("LmrRoundBias"))),
             bias_default);
    CHECK_EQ(search_param_value(static_cast<size_t>(index_of("LmrHistClamp"))),
             clamp_default);
  }


  // Mutation: W01_round_bias_dropped -- the bias is not added before the shift,
  // so the accumulator floors at every setting and the parameter decides
  // nothing. **This is the only case that can kill it**, and it is not in the
  // Release suite the mutation pass builds, which is why that mutant is
  // declared equivalent there rather than expected to die.
  //
  //   search parameters
  //    the rounding bias moves the boundary it is the rule for
  //   CHECK_EQ( plies_at(boundary), 3 )
  //   values: written at this step's own mutation pass, from the observed red
  //   and not from a prediction
  TEST_CASE("the rounding bias moves the boundary it is the rule for")
  {
    // `LmrRoundBias` ships at 0 -- the parent's own truncation, because with
    // the history term live every non-zero bias loses the mate row
    // `tests/test_search.cpp` "pruning does not hide a forced mate" guards, and
    // S236's step file records the six builds that established it. At 0 the
    // bias term is zero and a build with the addition deleted is the same
    // engine, so the release suite cannot see this rule at all. It is a live
    // rule with an off default, the shape `LmpDepthCoeff` has shipped in since
    // S109, and this is where it is guarded: the build that can set it.
    const int scale = search_lmr_scale_probe();
    const int bias_default =
        search_param_info(static_cast<size_t>(index_of("LmrRoundBias")))
            .default_value;

    REQUIRE_EQ(bias_default, 0);

    const int depth = 8;
    const int move_number = 8;
    const int table = search_lmr_reduction_ticks_probe(depth, move_number);

    auto plies_at = [&](int ticks) {
      return search_lmr_adjusted_reduction_probe(depth, move_number,
                                                 ticks - table, 0);
    };

    // Four settings across the range, each asserted at its own boundary: the
    // sum a bias carries up to the next whole ply sits `scale - bias` ticks
    // above it, and one tick below that is still the ply it started in. A build
    // that ignored the bias would answer the same at all four.
    for (int bias : {0, 1, scale / 2, scale - 1}) {
      REQUIRE(search_param_set("LmrRoundBias", bias));

      const int boundary = 2 * scale + (scale - bias);

      CHECK_EQ(plies_at(2 * scale), 2);
      CHECK_EQ(plies_at(boundary - 1), 2);
      CHECK_EQ(plies_at(boundary), 3);

      // And the negative side, where the shift has to floor: a division would
      // truncate toward zero and answer 0 a tick below the bias.
      CHECK_EQ(plies_at(-bias), 0);
      CHECK_EQ(plies_at(-bias - 1), -1);
    }

    REQUIRE(search_param_set("LmrRoundBias", bias_default));
  }


  TEST_CASE("the time scale cannot be driven to zero")
  {
    // S089's floor. At the shipping defaults it does not bind -- eight stable
    // iterations at 4 % each leaves 68 -- so the precondition has to be built
    // here rather than assumed: a stability discount large enough to take the
    // scale past zero on its own. Without moving the two parameters first this
    // case would pass on a scale function with no floor in it at all, which is
    // why it lives in the binary that can move a parameter.
    const int max_index = index_of("TmStabilityMax");
    const int percent_index = index_of("TmStabilityPercent");
    const int floor_index = index_of("TmScaleMinPercent");

    REQUIRE(max_index >= 0);
    REQUIRE(percent_index >= 0);
    REQUIRE(floor_index >= 0);

    const search_param_t& max_param =
        search_param_info(static_cast<size_t>(max_index));
    const search_param_t& percent_param =
        search_param_info(static_cast<size_t>(percent_index));

    // Precondition: at the defaults the floor is not what is being observed.
    REQUIRE(search_time_scale_percent(MAX_DEPTH, 0) > TM_SCALE_MIN_PERCENT);

    REQUIRE(search_param_set(max_param.name, max_param.max_value));
    REQUIRE(search_param_set(percent_param.name, percent_param.max_value));

    // 126 iterations at 50 % each is 6300 taken off a scale that starts at
    // 100, so nothing but the floor can be left.
    CHECK_EQ(search_time_scale_percent(MAX_DEPTH, 0), TM_SCALE_MIN_PERCENT);
    CHECK(search_time_scale_percent(MAX_DEPTH, 0) > 0);

    REQUIRE(search_param_set(max_param.name, max_param.default_value));
    REQUIRE(search_param_set(percent_param.name, percent_param.default_value));
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
