#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "tuner_groups.hpp"

// The tuner's `--only` groups have to partition its parameter vector, and until
// S041 nothing checked that they did. free_mask() has swallowed the term
// appended after it four times -- king_safety past the passed pawn weights,
// passed_pawns past pawn structure, pawn_structure past piece placement,
// piece_placement past tempo -- because each group's range ran to PARAM_COUNT
// and the next block was appended without re-ending it. All four comments are
// still in the function. Nothing in tests/ linked tuner.cpp at all
// (2026-08-13_adversarial-F07).
//
// The failure is silent and camouflaged by the workflow around it: a swallowed
// group hands the new term's weights back exactly as it received them, which
// reads as "the fit found nothing", and DEV_MANUAL already says a term fitting
// to almost nothing is normal. The cost of one occurrence is a wasted fit plus
// the SPRT after it.
//
// Three properties, no dataset:
//
//   1. every group's range is non-empty
//   2. the groups are pairwise disjoint
//   3. their union is exactly [0, PARAM_COUNT)
//
// Property 2 is what fires when a group is appended without re-ending the one
// before it: the predecessor still covers the new block, so both claim it.
// Property 3 fires when the block is appended and given no group at all, in
// every layout where the last group does not itself run to PARAM_COUNT.

// The group names come from GROUP_LIST rather than from a list written here.
// That is deliberate: GROUP_LIST is what `--help` prints and what an unknown
// `--only` is reported against, so the names a user can reach and the names
// free_mask accepts have to be the same set. A list copied into this file would
// check free_mask against the copy and let the two drift.
static std::vector<std::string> parse_group_list(const char* list)
{
  std::vector<std::string> out;
  std::string current;

  for (const char* p = list; *p != '\0'; ++p) {
    if (*p == ',') {
      if (!current.empty()) { out.push_back(current); }
      current.clear();
      continue;
    }

    // The literal is wrapped across source lines and separated with ", ", so
    // spaces are padding and never part of a name.
    if (*p == ' ') { continue; }

    current.push_back(*p);
  }

  if (!current.empty()) { out.push_back(current); }

  return out;
}


// Everything GROUP_LIST offers except `all`. `all` is the whole vector by
// construction, so it overlaps every other group and cannot be part of a
// partition; it is checked on its own below.
static std::vector<std::string> partition_groups()
{
  std::vector<std::string> out;

  for (const std::string& name : parse_group_list(tuner_groups::GROUP_LIST)) {
    if (name != "all") { out.push_back(name); }
  }

  return out;
}


TEST_CASE("the group list and the parameter vector are what the code says")
{
  const std::vector<std::string> groups =
      parse_group_list(tuner_groups::GROUP_LIST);

  // Preconditions for everything below. An empty or one-entry list, or a
  // zero-length parameter vector, would let the partition case pass over
  // nothing at all.
  REQUIRE(tuner_layout::PARAM_COUNT > 0);
  REQUIRE(groups.size() >= 2);
  REQUIRE(std::count(groups.begin(), groups.end(), std::string("all")) == 1);

  // PARAM_COUNT is tuner_layout.hpp's own sum of the block widths; the base
  // constants are a separate chain of running offsets. The two are written
  // independently and can disagree -- and they disagree exactly when a block is
  // appended after the last one, which the partition alone could not see while
  // the last group ran to PARAM_COUNT: it would cover the new block and all
  // three properties would still hold.
  //
  // Until S123 the last block was tempo's and `tempo` ran to PARAM_COUNT.
  // S123 appended its family after it, did what this comment then said -- gave
  // each block a group, moved `tempo`'s end to its own block, and named the
  // new last block here -- and ended every group at its own block's end
  // constant, so no group runs to PARAM_COUNT any more. The case below, "every
  // group frees exactly its own block", now holds each group to its block, the
  // last included; this line still says, in the event's own words, what an
  // appended block looks like. S125 appended its family after S123's and named
  // its last block, weak unopposed pawns, here.
  //
  // The message is the whole value of this assertion (DEC-184 item 4): without
  // it a failure reads as two constants disagreeing, which is the symptom and
  // not the event, and the reader has to re-derive the event from the comment
  // above.
  REQUIRE_MESSAGE(
      tuner_layout::WKU_EG_BASE + tuner_layout::WEAK_UNOPPOSED_COUNT ==
          tuner_layout::PARAM_COUNT,
      "a parameter block was appended after S125's weak unopposed pawns, the "
      "last block, and given no group of its own: WKU_EG_BASE "
          << tuner_layout::WKU_EG_BASE << " + WEAK_UNOPPOSED_COUNT "
          << tuner_layout::WEAK_UNOPPOSED_COUNT << " = "
          << (tuner_layout::WKU_EG_BASE + tuner_layout::WEAK_UNOPPOSED_COUNT)
          << " against PARAM_COUNT " << tuner_layout::PARAM_COUNT
          << ". Give it an end constant and a group, and name it here and in "
             "\"every group frees exactly its own block\".");

  std::vector<uint8_t> mask;

  // `all` frees everything, which is also what fixes the mask's length for
  // every other group: free_mask() assigns PARAM_COUNT entries whatever group
  // it was asked for.
  REQUIRE(tuner_groups::free_mask("all", &mask));
  CHECK(mask.size() == tuner_layout::PARAM_COUNT);
  CHECK(static_cast<size_t>(std::count(mask.begin(), mask.end(), uint8_t{1})) ==
        tuner_layout::PARAM_COUNT);

  // Every name the list offers has to be a name free_mask accepts. Without
  // this, a group named in GROUP_LIST and missing from free_mask would leave
  // the partition below one group short and its hole would read as an
  // unassigned parameter rather than as a broken group.
  for (const std::string& name : groups) {
    REQUIRE_MESSAGE(tuner_groups::free_mask(name, &mask),
                    "GROUP_LIST offers a group free_mask refuses: " << name);
  }

  // And a name it must refuse, so the loop above is not passing because
  // free_mask returns true for anything at all.
  CHECK_FALSE(tuner_groups::free_mask("not_a_group", &mask));
}


TEST_CASE("the --only groups partition the parameter vector")
{
  const std::vector<std::string> groups = partition_groups();

  // Precondition: disjointness over fewer than two groups is vacuous.
  REQUIRE(groups.size() >= 2);
  REQUIRE(tuner_layout::PARAM_COUNT > 0);

  // How many groups claim each parameter, and which was the first to claim it.
  std::vector<size_t> claims(tuner_layout::PARAM_COUNT, 0);
  std::vector<std::string> owner(tuner_layout::PARAM_COUNT);

  size_t overlaps = 0;
  size_t first_overlap = tuner_layout::PARAM_COUNT;
  std::string overlap_pair;

  for (const std::string& name : groups) {
    std::vector<uint8_t> mask;

    REQUIRE(tuner_groups::free_mask(name, &mask));
    REQUIRE(mask.size() == tuner_layout::PARAM_COUNT);

    size_t freed = 0;

    for (size_t i = 0; i < mask.size(); ++i) {
      if (!mask[i]) { continue; }

      freed++;

      if (claims[i] > 0) {
        if (overlaps == 0) {
          first_overlap = i;
          overlap_pair = owner[i] + " and " + name;
        }

        overlaps++;
      } else {
        owner[i] = name;
      }

      claims[i]++;
    }

    // 1. Non-empty. A group that frees nothing is a group whose --only run
    // reports the error of the constants it started from and changes not one
    // number.
    CHECK_MESSAGE(freed > 0, "group frees no parameter at all: " << name);
  }

  // 2. Pairwise disjoint. This is the one that fires when a term is appended
  // and the group before it is left running past its own weights: the
  // predecessor and the new group both claim the new block.
  CHECK_MESSAGE(overlaps == 0,
                overlaps << " of " << tuner_layout::PARAM_COUNT
                         << " parameters are freed by more than one --only "
                            "group; the first is index "
                         << first_overlap << ", claimed by " << overlap_pair
                         << ". A group appended without re-ending the one "
                            "before it looks exactly like this");

  size_t uncovered = 0;
  size_t first_uncovered = tuner_layout::PARAM_COUNT;

  for (size_t i = 0; i < claims.size(); ++i) {
    if (claims[i] > 0) { continue; }

    if (uncovered == 0) { first_uncovered = i; }

    uncovered++;
  }

  // 3. The union is exactly [0, PARAM_COUNT). With property 2 holding, this
  // makes the groups a partition: no parameter can be fitted by two --only runs
  // and none is unreachable from every one of them.
  CHECK_MESSAGE(uncovered == 0,
                uncovered << " of " << tuner_layout::PARAM_COUNT
                          << " parameters are in no --only group; the first is "
                             "index "
                          << first_uncovered
                          << ". A parameter block added without a group of its "
                             "own looks exactly like this");
}


// S123, DEC-329 (7). Each group frees exactly its own block, read off the
// layout's base chain and widths here rather than off the end constants
// free_mask() uses, so a group that stops at the wrong end -- `tempo` running
// on into the block appended after it, the defect this file's first comment
// counts four times and S123 would have been the fifth -- reads red with the
// group and the index named. The blocks are listed in vector order and must
// tile it, so a block with no group fails the last check.
TEST_CASE("every group frees exactly its own block")
{
  using namespace tuner_layout;

  struct block_t
  {
    const char* group;
    size_t first;
    size_t last;  // one past
  };

  const block_t blocks[] = {
      {"material", 0, MATERIAL_COUNT},
      {"psqt", MG_BASE, EG_BASE + SQUARE_COUNT},
      {"mobility", MOB_MG_BASE, MOB_EG_BASE + MOBILITY_COUNT},
      {"king_safety", KS_MG_BASE, KS_EG_BASE + KING_SAFETY_COUNT},
      {"passed_pawns", PP_MG_BASE, PP_EG_BASE + PASSED_PAWN_COUNT},
      {"pawn_structure", PS_MG_BASE, PS_EG_BASE + PAWN_STRUCTURE_COUNT},
      {"piece_placement", PL_MG_BASE, PL_EG_BASE + PIECE_PLACEMENT_COUNT},
      {"tempo", TEMPO_MG_BASE, TEMPO_EG_BASE + TEMPO_COUNT},
      {"passed_advance", PF_MG_BASE, PSF_EG_BASE + PASSED_SAFE_COUNT},
      {"passed_kings", PKO_MG_BASE, PKE_EG_BASE + PASSED_KING_COUNT},
      {"candidate_passers", CP_MG_BASE, CP_EG_BASE + CANDIDATE_PASSER_COUNT},
      {"isolated_backward", IB_MG_BASE, IB_EG_BASE + ISOLATED_BACKWARD_COUNT},
      {"isolated_files", ISF_MG_BASE, ISF_EG_BASE + ISOLATED_FILE_COUNT},
      {"doubled_files", DBF_MG_BASE, DBF_EG_BASE + DOUBLED_FILE_COUNT},
      {"backward_ranks", BKR_MG_BASE, BKR_EG_BASE + BACKWARD_RANK_COUNT},
      {"phalanx", PHX_MG_BASE, PHX_EG_BASE + PHALANX_COUNT},
      {"supported", SUP_MG_BASE, SUP_EG_BASE + SUPPORTED_COUNT},
      {"weak_unopposed", WKU_MG_BASE, WKU_EG_BASE + WEAK_UNOPPOSED_COUNT},
  };

  size_t next = 0;

  for (const block_t& block : blocks) {
    CAPTURE(block.group);

    // The blocks tile the vector in order: each starts where the last ended.
    CHECK(block.first == next);
    REQUIRE(block.last > block.first);
    next = block.last;

    std::vector<uint8_t> mask;
    REQUIRE(tuner_groups::free_mask(block.group, &mask));
    REQUIRE(mask.size() == PARAM_COUNT);

    size_t outside = 0;
    size_t missing = 0;
    size_t first_wrong = PARAM_COUNT;

    for (size_t i = 0; i < PARAM_COUNT; ++i) {
      const bool inside = (i >= block.first && i < block.last);
      if (mask[i] != 0 && !inside) { outside++; }
      if (mask[i] == 0 && inside) { missing++; }
      if ((mask[i] != 0) != inside && first_wrong == PARAM_COUNT) {
        first_wrong = i;
      }
    }

    CHECK_MESSAGE(outside + missing == 0,
                  std::string(block.group)
                      << " frees " << outside
                      << " parameters outside its block and leaves " << missing
                      << " of it held; the first is index " << first_wrong
                      << ", the block [" << block.first << ", " << block.last
                      << ")");
  }

  CHECK(next == PARAM_COUNT);

  // Every partition group is in the list above, so the tiling covers all of
  // GROUP_LIST and a group added there without a block here fails.
  CHECK(partition_groups().size() == sizeof(blocks) / sizeof(blocks[0]));
}


// S123's screen frees a part's own weights with the earlier survivors':
// `--only` with a list is the union of the names, one name is exactly
// free_mask()'s, and a list with an unknown name, an empty name or `all` is
// refused whole rather than half applied.
TEST_CASE("--only takes a list and frees exactly the union")
{
  std::vector<uint8_t> advance;
  std::vector<uint8_t> kings;
  std::vector<uint8_t> candidates;
  REQUIRE(tuner_groups::free_mask("passed_advance", &advance));
  REQUIRE(tuner_groups::free_mask("passed_kings", &kings));
  REQUIRE(tuner_groups::free_mask("candidate_passers", &candidates));

  // Preconditions: three non-empty, disjoint groups.
  REQUIRE(std::count(advance.begin(), advance.end(), uint8_t{1}) == 24);
  REQUIRE(std::count(kings.begin(), kings.end(), uint8_t{1}) == 24);
  REQUIRE(std::count(candidates.begin(), candidates.end(), uint8_t{1}) == 10);

  std::vector<uint8_t> one;
  REQUIRE(tuner_groups::only_mask("passed_kings", &one));
  CHECK(one == kings);

  std::vector<uint8_t> two;
  REQUIRE(tuner_groups::only_mask("passed_advance, passed_kings", &two));
  std::vector<uint8_t> three;
  REQUIRE(tuner_groups::only_mask(
      "passed_advance,passed_kings,candidate_passers", &three));

  for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
    CAPTURE(i);
    CHECK(two[i] == ((advance[i] || kings[i]) ? 1 : 0));
    CHECK(three[i] == ((advance[i] || kings[i] || candidates[i]) ? 1 : 0));
  }

  // A term beside a group is a name like any other.
  std::vector<uint8_t> mixed;
  REQUIRE(
      tuner_groups::only_mask("piece_placement.trapped, passed_kings", &mixed));
  CHECK(std::count(mixed.begin(), mixed.end(), uint8_t{1}) == 4 + 24);

  for (const char* bad :
       {"passed_advance,not_a_group", "passed_advance,,passed_kings",
        "passed_advance,", "all,passed_kings", ","}) {
    CAPTURE(bad);
    std::vector<uint8_t> refused;
    CHECK_FALSE(tuner_groups::only_mask(bad, &refused));
  }
}


// --freeze, added at S065. The inverse of --only: free everything except the
// named groups. What it exists for is a fit that has to hold two groups at zero
// while the other 815 parameters move (817 at S065, before S134 narrowed the
// vector), which --only cannot express -- it says
// "fit only tempo", never "fit everything but tempo".
//
// The properties are the ones a wrong implementation would break silently. A
// freeze that cleared one index too few leaves a parameter fitted that the run
// meant to hold, and the emitted header looks exactly like a correct one: the
// held value is simply not what was asked for, and nothing downstream can tell.
TEST_CASE("--freeze clears exactly the named groups")
{
  const std::vector<std::string> groups = partition_groups();

  REQUIRE(groups.size() >= 2);
  REQUIRE(tuner_layout::PARAM_COUNT > 0);

  std::vector<uint8_t> all;
  REQUIRE(tuner_groups::free_mask("all", &all));
  REQUIRE(all.size() == tuner_layout::PARAM_COUNT);

  // One group at a time, against that group's own --only range. Freezing a
  // group out of `all` and freeing it with --only have to be complements, which
  // is the whole claim: the same range, once as what moves and once as what
  // does not.
  for (const std::string& name : groups) {
    std::vector<uint8_t> only;
    REQUIRE(tuner_groups::free_mask(name, &only));

    const size_t in_group =
        static_cast<size_t>(std::count(only.begin(), only.end(), uint8_t{1}));

    // Precondition. A group freeing nothing would make every claim below hold
    // over an empty set, which is the vacuous pass this file exists to refuse.
    REQUIRE(in_group > 0);

    std::vector<uint8_t> mask = all;
    REQUIRE_MESSAGE(tuner_groups::freeze_mask(name, &mask),
                    "freeze_mask refuses a group free_mask accepts: " << name);

    size_t wrongly_frozen = 0;
    size_t wrongly_free = 0;

    for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
      if (only[i] && mask[i]) { wrongly_free++; }
      if (!only[i] && !mask[i]) { wrongly_frozen++; }
    }

    CHECK_MESSAGE(wrongly_free == 0,
                  wrongly_free << " parameter(s) of group " << name
                               << " are still free after freezing it");
    CHECK_MESSAGE(wrongly_frozen == 0, wrongly_frozen
                                           << " parameter(s) outside group "
                                           << name << " were frozen with it");
  }
}


TEST_CASE("--freeze takes a list, and refuses what it cannot honour")
{
  std::vector<uint8_t> all;
  REQUIRE(tuner_groups::free_mask("all", &all));

  std::vector<uint8_t> tempo;
  std::vector<uint8_t> placement;
  REQUIRE(tuner_groups::free_mask("tempo", &tempo));
  REQUIRE(tuner_groups::free_mask("piece_placement", &placement));

  const size_t tempo_count =
      static_cast<size_t>(std::count(tempo.begin(), tempo.end(), uint8_t{1}));
  const size_t placement_count = static_cast<size_t>(
      std::count(placement.begin(), placement.end(), uint8_t{1}));

  // Preconditions: two non-empty, disjoint groups, or "both were cleared" is
  // not a stronger statement than "one was".
  REQUIRE(tempo_count > 0);
  REQUIRE(placement_count > 0);

  for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
    // As a variable because doctest decomposes the expression it is handed and
    // refuses a `&&` inside one.
    const bool in_both = (tempo[i] != 0) && (placement[i] != 0);
    REQUIRE_FALSE(in_both);
  }

  // The S065 run: everything free except tempo and piece placement, from one
  // comma-separated argument.
  std::vector<uint8_t> mask = all;
  REQUIRE(tuner_groups::freeze_mask("tempo,piece_placement", &mask));

  const size_t free_count =
      static_cast<size_t>(std::count(mask.begin(), mask.end(), uint8_t{1}));
  const size_t expected_free =
      tuner_layout::PARAM_COUNT - tempo_count - placement_count;

  CHECK(free_count == expected_free);

  for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
    CAPTURE(i);

    if (tempo[i] || placement[i]) {
      CHECK(mask[i] == 0);
    } else {
      CHECK(mask[i] == 1);
    }
  }

  // Order and spacing are not part of the meaning.
  std::vector<uint8_t> reversed = all;
  REQUIRE(tuner_groups::freeze_mask("piece_placement, tempo", &reversed));
  CHECK(reversed == mask);

  // An empty list is what a run that passes no --freeze gets, and it has to be
  // a no-op: this is what says the flag's default cannot change any earlier
  // run's result.
  std::vector<uint8_t> untouched = all;
  REQUIRE(tuner_groups::freeze_mask("", &untouched));
  CHECK(untouched == all);

  // A name free_mask refuses is refused here too. Without this the loops above
  // would pass on an implementation that silently ignored every name it did not
  // recognise -- which is the one failure mode that costs a whole fit.
  std::vector<uint8_t> bad = all;
  CHECK_FALSE(tuner_groups::freeze_mask("not_a_group", &bad));

  // And one good name beside one bad one is still refused, rather than the good
  // half being applied.
  std::vector<uint8_t> half = all;
  CHECK_FALSE(tuner_groups::freeze_mask("tempo,not_a_group", &half));

  // `all` is a name free_mask accepts and freeze_mask must not: it would fit
  // nothing at all and report the error of the constants it started from, which
  // reads as a fit that found nothing rather than as a run that never moved.
  std::vector<uint8_t> everything = all;
  CHECK_FALSE(tuner_groups::freeze_mask("all", &everything));
}


// S275. The seven gauges are the seven exact null directions feature_audit
// reports beside the 32 zero columns, and their shape is what makes holding
// them safe: each lives in the material and piece-square blocks, no two share a
// column -- a shared one would let one hold undo another -- and none touches a
// pawn column on rank 1 or 8, which no row reaches and the fit must leave
// exactly as it found it.
TEST_CASE(
    "the seven gauges are apart, in the tables, and clear of the dead pawn "
    "squares")
{
  const std::vector<tuner_groups::gauge_t> gauges = tuner_groups::gauges();
  REQUIRE(gauges.size() == 7);

  const size_t psqt_first = tuner_layout::MG_BASE;
  const size_t psqt_last = tuner_layout::MOB_MG_BASE;

  std::vector<int> claims(tuner_layout::PARAM_COUNT, 0);

  for (size_t g = 0; g < gauges.size(); ++g) {
    const tuner_groups::gauge_t& gauge = gauges[g];
    CAPTURE(gauge.name);

    // Five material/table directions, then the two king tables.
    if (g < tuner_layout::MATERIAL_COUNT) {
      CHECK(gauge.absorb == g);
      CHECK(gauge.level.size() == ((g == 0) ? 96u : 128u));
    } else {
      CHECK(gauge.absorb == tuner_layout::PARAM_COUNT);
      CHECK(gauge.level.size() == 64u);
    }

    if (gauge.absorb < tuner_layout::PARAM_COUNT) { claims[gauge.absorb]++; }

    for (const size_t column : gauge.level) {
      CAPTURE(column);
      CHECK(column >= psqt_first);
      CHECK(column < psqt_last);

      const size_t square = (column - psqt_first) % (6 * 64);
      const size_t type = square / 64;
      CHECK(type == ((g < tuner_layout::MATERIAL_COUNT) ? g : 5u));

      // Written out, not asked of pawn_can_stand(): a pawn stands on squares
      // 8..55 of its table.
      if (type == 0) {
        CHECK(square % 64 >= 8);
        CHECK(square % 64 < 56);
      }

      claims[column]++;
    }
  }

  size_t shared = 0;
  size_t claimed = 0;
  for (const int count : claims) {
    if (count > 1) { shared++; }
    if (count > 0) { claimed++; }
  }

  CHECK(shared == 0);

  // 5 values, 96 + 4 x 128 table squares, 2 x 64 king squares; with the 32
  // dead pawn squares that is the whole of the two blocks.
  CHECK(claimed == 5 + 96 + 4 * 128 + 2 * 64);
  CHECK(claimed + 32 == psqt_last);
}


// S275. --only and --freeze promise that a held parameter comes out
// bit-identical, and the gauges must not break that promise. A gauge is held
// when every column it moves is free and left alone otherwise -- the freeze
// already fixes it, since the free columns cannot move along it without moving
// a frozen one. For every --only group and every single --freeze: the held set
// is exactly the gauges the mask frees whole, a hold restores each one's level,
// and it writes no frozen parameter.
TEST_CASE("a gauge is held only where the mask frees all of it")
{
  const std::vector<tuner_groups::gauge_t> gauges = tuner_groups::gauges();
  REQUIRE(gauges.size() == 7);

  std::vector<std::pair<std::string, std::vector<uint8_t>>> masks;

  for (const std::string& name : parse_group_list(tuner_groups::GROUP_LIST)) {
    std::vector<uint8_t> only;
    REQUIRE(tuner_groups::free_mask(name, &only));
    masks.push_back({"--only " + name, only});

    if (name == "all") { continue; }

    std::vector<uint8_t> frozen;
    REQUIRE(tuner_groups::free_mask("all", &frozen));
    REQUIRE(tuner_groups::freeze_mask(name, &frozen));
    masks.push_back({"--freeze " + name, frozen});
  }

  // A start, and a vector that has walked off it on every column, so a hold
  // has a level to put back on every gauge it holds.
  std::vector<double> start(tuner_layout::PARAM_COUNT);
  std::vector<double> walked(tuner_layout::PARAM_COUNT);
  for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
    start[i] = static_cast<double>(i % 17) - 8.0;
    walked[i] = start[i] + 0.25 * static_cast<double>(i % 7) + 1.0;
  }

  size_t held_somewhere = 0;
  size_t skipped_somewhere = 0;

  for (const auto& [label, mask] : masks) {
    CAPTURE(label);

    const std::vector<tuner_groups::gauge_t> held =
        tuner_groups::held_gauges(mask);

    // Written out rather than derived: the material/table five need both the
    // material and the psqt group free, the king pair needs psqt, and no other
    // group touches any of them.
    size_t expected = label.rfind("--only", 0) == 0 ? 0 : 7;
    if (label == "--only all") { expected = 7; }
    if (label == "--only psqt" || label == "--freeze material") {
      expected = 2;
    }
    if (label == "--freeze psqt") { expected = 0; }

    REQUIRE(held.size() == expected);
    for (size_t g = 0; g < held.size(); ++g) {
      CHECK(held[g].name == gauges[gauges.size() - held.size() + g].name);
    }
    held_somewhere += held.size();
    skipped_somewhere += gauges.size() - held.size();

    // The hold the tuner runs after a step: only what the mask frees, and the
    // whole of it.
    std::vector<double> params = walked;
    for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
      if (!mask[i]) { params[i] = start[i]; }
    }

    const std::vector<double> before_hold = params;
    tuner_groups::hold_gauges(held, start.data(), params.data());

    size_t frozen_written = 0;
    for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
      if (!mask[i] && params[i] != before_hold[i]) { frozen_written++; }
    }
    CHECK(frozen_written == 0);

    for (const tuner_groups::gauge_t& gauge : held) {
      CAPTURE(gauge.name);

      double before = 0.0;
      double after = 0.0;
      for (const size_t column : gauge.level) {
        before += before_hold[column] - start[column];
        after += params[column] - start[column];
      }

      // Precondition: the walk moved this level, so the hold had work to do.
      CHECK(std::fabs(before) > 1.0);
      CHECK(std::fabs(after) < 1e-9);
    }
  }

  // Both branches were taken: some mask held a gauge and some left one to the
  // freeze, or one of the checks above ran over nothing.
  CHECK(held_somewhere > 0);
  CHECK(skipped_somewhere > 0);
}


// S135. The piece placement group's terms, the names DEC-259's screen fits one
// at a time: `--only piece_placement.bad_bishop` and the rest, and then the
// terms that passed together, `--only piece_placement` with the ones that did
// not in `--freeze`. They are not in GROUP_LIST and cannot be -- each overlaps
// the group it belongs to -- so the partition cases above never see them.
// What they have to be is the same three properties one level down: every
// term frees something, no two terms free the same parameter, and together
// they free exactly what `piece_placement` frees -- its middlegame and endgame
// weights for the same columns, which are two ranges and not one.
TEST_CASE("the piece placement terms partition the piece placement group")
{
  const std::vector<std::string> terms =
      parse_group_list(tuner_groups::PLACEMENT_TERM_LIST);

  // The list --help prints and the table free_mask reads are the same set, in
  // the same order, or a term could be reachable from one and not the other.
  REQUIRE(terms.size() == tuner_groups::PLACEMENT_TERM_COUNT);
  for (size_t t = 0; t < terms.size(); ++t) {
    CHECK(terms[t] == tuner_groups::PLACEMENT_TERMS[t].name);
  }

  std::vector<uint8_t> group;
  REQUIRE(tuner_groups::free_mask("piece_placement", &group));

  const size_t group_count =
      static_cast<size_t>(std::count(group.begin(), group.end(), uint8_t{1}));
  REQUIRE(group_count == 2 * tuner_layout::PIECE_PLACEMENT_COUNT);

  std::vector<size_t> claims(tuner_layout::PARAM_COUNT, 0);

  for (size_t t = 0; t < terms.size(); ++t) {
    CAPTURE(terms[t]);

    std::vector<uint8_t> mask;
    REQUIRE_MESSAGE(tuner_groups::free_mask(terms[t], &mask),
                    "PLACEMENT_TERM_LIST offers a term free_mask refuses");
    REQUIRE(mask.size() == tuner_layout::PARAM_COUNT);

    const tuner_groups::placement_term_t& term =
        tuner_groups::PLACEMENT_TERMS[t];

    // Exactly the term's columns, in both halves, and nothing else.
    for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
      CAPTURE(i);

      bool expected = false;
      for (size_t c = term.first; c < term.first + term.width; ++c) {
        expected = expected || i == tuner_layout::PL_MG_BASE + c ||
                   i == tuner_layout::PL_EG_BASE + c;
      }

      CHECK((mask[i] != 0) == expected);
      if (mask[i] != 0) { claims[i]++; }
    }

    CHECK(static_cast<size_t>(std::count(mask.begin(), mask.end(),
                                         uint8_t{1})) == 2 * term.width);
  }

  // Disjoint, and their union the group.
  for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
    CAPTURE(i);
    CHECK(claims[i] == (group[i] != 0 ? 1u : 0u));
  }
}


// The candidate fit's shape: the group free but for the terms that failed the
// screen. `--freeze` with a term inside `--only piece_placement` has to clear
// exactly that term's columns, and a list of terms all of theirs; a term
// beside a group in one list is allowed, and an unknown term refuses the list.
TEST_CASE("--freeze takes terms, and clears exactly their columns")
{
  std::vector<uint8_t> group;
  REQUIRE(tuner_groups::free_mask("piece_placement", &group));

  std::vector<uint8_t> trapped;
  std::vector<uint8_t> pair;
  REQUIRE(tuner_groups::free_mask("piece_placement.trapped", &trapped));
  REQUIRE(tuner_groups::free_mask("piece_placement.bishop_pair", &pair));

  // Preconditions: two non-empty, disjoint terms inside the group.
  REQUIRE(std::count(trapped.begin(), trapped.end(), uint8_t{1}) == 4);
  REQUIRE(std::count(pair.begin(), pair.end(), uint8_t{1}) == 2);

  std::vector<uint8_t> one = group;
  REQUIRE(tuner_groups::freeze_mask("piece_placement.trapped", &one));

  std::vector<uint8_t> two = group;
  REQUIRE(tuner_groups::freeze_mask(
      "piece_placement.trapped, piece_placement.bishop_pair", &two));

  for (size_t i = 0; i < tuner_layout::PARAM_COUNT; ++i) {
    CAPTURE(i);
    CHECK(one[i] == ((group[i] != 0 && trapped[i] == 0) ? 1 : 0));
    CHECK(two[i] ==
          ((group[i] != 0 && trapped[i] == 0 && pair[i] == 0) ? 1 : 0));
  }

  // The screen's own shape, a term alone: everything but it is held.
  std::vector<uint8_t> alone;
  REQUIRE(tuner_groups::free_mask("piece_placement.bad_bishop", &alone));
  CHECK(std::count(alone.begin(), alone.end(), uint8_t{1}) == 4);

  // A term beside a group in one list freezes both, and a misspelt term
  // refuses the whole list rather than applying the good half.
  std::vector<uint8_t> all;
  REQUIRE(tuner_groups::free_mask("all", &all));

  std::vector<uint8_t> mixed = all;
  REQUIRE(tuner_groups::freeze_mask("tempo, piece_placement.trapped", &mixed));
  CHECK(
      static_cast<size_t>(std::count(mixed.begin(), mixed.end(), uint8_t{1})) ==
      tuner_layout::PARAM_COUNT - 2 - 4);

  std::vector<uint8_t> misspelt = all;
  CHECK_FALSE(tuner_groups::freeze_mask("piece_placement.traped", &misspelt));
  CHECK_FALSE(tuner_groups::free_mask("piece_placement.", &misspelt));
}


// S121's ties. A tie list names cells by table and count, and parse_ties()
// turns each line into the two parameters it joins; fold and copy are the
// chain rule and the shared value. What this holds: a line lands on the
// parameter tools/tuner_layout.hpp puts that cell at, every malformed or
// contradictory line is refused rather than half applied, the gradient a tie
// folds is the derivative of the tied parameterisation, and after a copy the
// tied cell is its anchor times the scale.
TEST_CASE("a tie joins the cells it names and refuses every other line")
{
  std::vector<tuner_groups::tie_t> ties;
  std::string error;

  REQUIRE(tuner_groups::parse_ties(
      "# queen tail\nmobility_queen_mg 27 24\n\nmobility_knight_eg 0 1 "
      "0.5  # half\n",
      &ties, &error));
  REQUIRE(ties.size() == 2);

  const size_t queen_mg =
      tuner_layout::MOB_MG_BASE + static_cast<size_t>(MOBILITY_OFFSET[3]);
  const size_t knight_eg =
      tuner_layout::MOB_EG_BASE + static_cast<size_t>(MOBILITY_OFFSET[0]);
  CHECK(ties[0].tied == queen_mg + 27);
  CHECK(ties[0].anchor == queen_mg + 24);
  CHECK(ties[0].scale == 1.0);
  CHECK(ties[1].tied == knight_eg + 0);
  CHECK(ties[1].anchor == knight_eg + 1);
  CHECK(ties[1].scale == 0.5);

  // Every one of these is refused, with a reason.
  for (const char* bad : {
           "mobility_queen_mg 28 24\n",    // past the queen's row
           "mobility_knight_mg 9 1\n",     // past the knight's row
           "mobility_king_mg 1 2\n",       // no such table
           "mobility_rook_mg 3 3\n",       // tied to itself
           "mobility_rook_mg 3\n",         // too few words
           "mobility_rook_mg 3 4 x\n",     // an unreadable scale
           "mobility_rook_mg 3 4 nan\n",   // not a number
           "mobility_rook_mg 3 4 -inf\n",  // not finite
           "mobility_rook_mg -1 4\n",      // not a count
           "mobility_rook_mg 3 4\nmobility_rook_mg 3 5\n",  // tied twice
           "mobility_rook_mg 3 4\nmobility_rook_mg 4 5\n",  // a chain
           "mobility_rook_mg 4 5\nmobility_rook_mg 3 4\n",  // the same chain
       }) {
    CAPTURE(bad);
    std::vector<tuner_groups::tie_t> refused;
    std::string why;
    CHECK_FALSE(tuner_groups::parse_ties(bad, &refused, &why));
    CHECK(!why.empty());
  }

  // Both ends free, or the list is refused for this run.
  std::vector<uint8_t> mobility;
  REQUIRE(tuner_groups::free_mask("mobility", &mobility));
  CHECK(tuner_groups::ties_fit_mask(ties, mobility, &error));

  std::vector<uint8_t> tempo;
  REQUIRE(tuner_groups::free_mask("tempo", &tempo));
  std::string why;
  CHECK_FALSE(tuner_groups::ties_fit_mask(ties, tempo, &why));
  CHECK(!why.empty());

  // The chain rule: with the tied cell set to scale * anchor, a gradient over
  // the two cells folds onto the anchor as g_anchor + scale * g_tied, and the
  // tied cell takes no step of its own.
  std::vector<double> grad(tuner_layout::PARAM_COUNT, 0.0);
  grad[knight_eg + 1] = 3.0;
  grad[knight_eg + 0] = 4.0;
  grad[queen_mg + 24] = -1.0;
  grad[queen_mg + 27] = 2.0;
  tuner_groups::fold_ties(ties, &grad);
  CHECK(grad[knight_eg + 1] == 3.0 + 0.5 * 4.0);
  CHECK(grad[knight_eg + 0] == 0.0);
  CHECK(grad[queen_mg + 24] == -1.0 + 2.0);
  CHECK(grad[queen_mg + 27] == 0.0);

  std::vector<double> params(tuner_layout::PARAM_COUNT, 7.0);
  params[knight_eg + 1] = 10.0;
  params[queen_mg + 24] = -3.0;
  tuner_groups::copy_ties(ties, params.data());
  CHECK(params[knight_eg + 0] == 5.0);
  CHECK(params[queen_mg + 27] == -3.0);
  CHECK(params[queen_mg + 26] == 7.0);  // untied, untouched
}


// S123's rows take ties as mobility's do (DEC-329 (4)): a rank cell lands on
// the parameter tools/tuner_layout.hpp puts it at, its own half and array,
// and a cell past the array's ranks is refused.
TEST_CASE("a tie names S123's rank cells by array and half")
{
  std::vector<tuner_groups::tie_t> ties;
  std::string error;

  REQUIRE(tuner_groups::parse_ties(
      "passed_free_mg 5 4\npassed_safe_eg 0 1\npassed_king_own_mg 2 3\n"
      "passed_king_enemy_eg 5 4 0.5\ncandidate_passer_eg 4 3\n",
      &ties, &error));
  REQUIRE(ties.size() == 5);

  CHECK(ties[0].tied == tuner_layout::PF_MG_BASE + 5);
  CHECK(ties[0].anchor == tuner_layout::PF_MG_BASE + 4);
  CHECK(ties[1].tied == tuner_layout::PSF_EG_BASE + 0);
  CHECK(ties[1].anchor == tuner_layout::PSF_EG_BASE + 1);
  CHECK(ties[2].tied == tuner_layout::PKO_MG_BASE + 2);
  CHECK(ties[3].tied == tuner_layout::PKE_EG_BASE + 5);
  CHECK(ties[3].scale == 0.5);
  CHECK(ties[4].tied == tuner_layout::CP_EG_BASE + 4);
  CHECK(ties[4].anchor == tuner_layout::CP_EG_BASE + 3);

  for (const char* bad : {
           "passed_free_mg 6 5\n",       // past rank 7
           "candidate_passer_mg 5 4\n",  // past rank 6
           "passed_free 1 2\n",          // no half
           "passed_kings_mg 1 2\n",      // a group, not an array
       }) {
    CAPTURE(bad);
    std::vector<tuner_groups::tie_t> refused;
    std::string why;
    CHECK_FALSE(tuner_groups::parse_ties(bad, &refused, &why));
    CHECK(!why.empty());
  }

  // Both ends free under the group the screen fits them in, and refused under
  // another.
  std::vector<uint8_t> advance;
  REQUIRE(tuner_groups::only_mask(
      "passed_advance,passed_kings,candidate_passers", &advance));
  CHECK(tuner_groups::ties_fit_mask(ties, advance, &error));

  std::vector<uint8_t> kings;
  REQUIRE(tuner_groups::free_mask("passed_kings", &kings));
  std::string why;
  CHECK_FALSE(tuner_groups::ties_fit_mask(ties, kings, &why));
}


// S125's arrays the same way (DEC-331 (6)): a cell lands on its own array and
// half, the cells being its weights' -- files b to h, the tables' own ranks
// from their first -- and a cell past the array is refused, the one-wide
// counts' included.
TEST_CASE("a tie names S125's cells by array and half")
{
  std::vector<tuner_groups::tie_t> ties;
  std::string error;

  REQUIRE(tuner_groups::parse_ties(
      "isolated_file_mg 0 1\ndoubled_file_eg 6 5\nbackward_rank_mg 2 1\n"
      "phalanx_eg 5 4\nsupported_mg 4 3 0.5\n",
      &ties, &error));
  REQUIRE(ties.size() == 5);

  CHECK(ties[0].tied == tuner_layout::ISF_MG_BASE + 0);
  CHECK(ties[0].anchor == tuner_layout::ISF_MG_BASE + 1);
  CHECK(ties[1].tied == tuner_layout::DBF_EG_BASE + 6);
  CHECK(ties[1].anchor == tuner_layout::DBF_EG_BASE + 5);
  CHECK(ties[2].tied == tuner_layout::BKR_MG_BASE + 2);
  CHECK(ties[3].tied == tuner_layout::PHX_EG_BASE + 5);
  CHECK(ties[4].tied == tuner_layout::SUP_MG_BASE + 4);
  CHECK(ties[4].scale == 0.5);

  for (const char* bad : {
           "isolated_file_mg 7 6\n",      // past the h file
           "backward_rank_eg 3 2\n",      // past own rank 5
           "supported_mg 5 4\n",          // past own rank 7
           "isolated_backward_mg 1 0\n",  // a count has one cell
           "weak_unopposed_eg 1 0\n",     // and so has this one
           "phalanx 1 2\n",               // no half
           "isolated_files_mg 1 2\n",     // a group, not an array
       }) {
    CAPTURE(bad);
    std::vector<tuner_groups::tie_t> refused;
    std::string why;
    CHECK_FALSE(tuner_groups::parse_ties(bad, &refused, &why));
    CHECK(!why.empty());
  }

  std::vector<uint8_t> free_set;
  REQUIRE(tuner_groups::only_mask(
      "isolated_files,doubled_files,backward_ranks,phalanx,supported",
      &free_set));
  CHECK(tuner_groups::ties_fit_mask(ties, free_set, &error));

  std::vector<uint8_t> phalanx;
  REQUIRE(tuner_groups::free_mask("phalanx", &phalanx));
  std::string why;
  CHECK_FALSE(tuner_groups::ties_fit_mask(ties, phalanx, &why));
}


// S121's eight mobility levels have no gauge, so a free set that frees both a
// row's cells and its piece's table half is refused: the precondition first,
// that `--only all` really frees both sides of every level, then the refusal,
// then the masks the screen and the table fits use, each of which freezes one
// side and passes.
TEST_CASE("a free set that walks a mobility level is refused")
{
  std::vector<uint8_t> all;
  REQUIRE(tuner_groups::free_mask("all", &all));

  const size_t knight_cells =
      tuner_layout::MOB_MG_BASE + static_cast<size_t>(MOBILITY_OFFSET[0]);
  const size_t knight_table = tuner_layout::MG_BASE + 64;
  REQUIRE(all[knight_cells] == 1);
  REQUIRE(all[knight_table] == 1);

  std::string why;
  CHECK_FALSE(tuner_groups::mobility_levels_held(all, &why));
  CHECK(why.find("knight middlegame") != std::string::npos);

  // One level walkable alone is enough: everything frozen but the queen's
  // endgame row and one square of the queen's endgame table.
  std::vector<uint8_t> one(tuner_layout::PARAM_COUNT, 0);
  one[tuner_layout::MOB_EG_BASE + static_cast<size_t>(MOBILITY_OFFSET[3]) + 5] =
      1;
  one[tuner_layout::EG_BASE + 4 * 64 + 27] = 1;
  why.clear();
  CHECK_FALSE(tuner_groups::mobility_levels_held(one, &why));
  CHECK(why.find("queen endgame") != std::string::npos);

  for (const char* group : {"mobility", "psqt", "material", "king_safety"}) {
    CAPTURE(group);
    std::vector<uint8_t> mask;
    REQUIRE(tuner_groups::free_mask(group, &mask));
    std::string none;
    CHECK(tuner_groups::mobility_levels_held(mask, &none));
  }

  std::vector<uint8_t> without = all;
  REQUIRE(tuner_groups::freeze_mask("mobility", &without));
  CHECK(tuner_groups::mobility_levels_held(without, &why));

  // A pawn's table is no level's: freeing it beside the mobility rows is fine.
  std::vector<uint8_t> pawn_too;
  REQUIRE(tuner_groups::free_mask("mobility", &pawn_too));
  pawn_too[tuner_layout::MG_BASE + 20] = 1;
  CHECK(tuner_groups::mobility_levels_held(pawn_too, &why));
}
