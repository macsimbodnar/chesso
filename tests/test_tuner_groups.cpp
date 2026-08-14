#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cstdint>
#include <string>
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
  REQUIRE(eval_model::PARAM_COUNT > 0);
  REQUIRE(groups.size() >= 2);
  REQUIRE(std::count(groups.begin(), groups.end(), std::string("all")) == 1);

  // PARAM_COUNT is eval_model.hpp's own sum of the block widths; the base
  // constants are a separate chain of running offsets. The two are written
  // independently and can disagree -- and they disagree exactly when a block is
  // appended after tempo, which is the one case the partition cannot see:
  // `tempo` runs to PARAM_COUNT, so it would cover the new block and all three
  // properties would still hold.
  //
  // If this fires, a parameter block was appended after tempo. Give it a group,
  // move `tempo`'s end to the new block, and name the new last block here.
  REQUIRE(eval_model::TEMPO_EG_BASE + eval_model::TEMPO_COUNT ==
          eval_model::PARAM_COUNT);

  std::vector<uint8_t> mask;

  // `all` frees everything, which is also what fixes the mask's length for
  // every other group: free_mask() assigns PARAM_COUNT entries whatever group
  // it was asked for.
  REQUIRE(tuner_groups::free_mask("all", &mask));
  CHECK(mask.size() == eval_model::PARAM_COUNT);
  CHECK(static_cast<size_t>(std::count(mask.begin(), mask.end(), uint8_t{1})) ==
        eval_model::PARAM_COUNT);

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
  REQUIRE(eval_model::PARAM_COUNT > 0);

  // How many groups claim each parameter, and which was the first to claim it.
  std::vector<size_t> claims(eval_model::PARAM_COUNT, 0);
  std::vector<std::string> owner(eval_model::PARAM_COUNT);

  size_t overlaps = 0;
  size_t first_overlap = eval_model::PARAM_COUNT;
  std::string overlap_pair;

  for (const std::string& name : groups) {
    std::vector<uint8_t> mask;

    REQUIRE(tuner_groups::free_mask(name, &mask));
    REQUIRE(mask.size() == eval_model::PARAM_COUNT);

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
                overlaps << " of " << eval_model::PARAM_COUNT
                         << " parameters are freed by more than one --only "
                            "group; the first is index "
                         << first_overlap << ", claimed by " << overlap_pair
                         << ". A group appended without re-ending the one "
                            "before it looks exactly like this");

  size_t uncovered = 0;
  size_t first_uncovered = eval_model::PARAM_COUNT;

  for (size_t i = 0; i < claims.size(); ++i) {
    if (claims[i] > 0) { continue; }

    if (uncovered == 0) { first_uncovered = i; }

    uncovered++;
  }

  // 3. The union is exactly [0, PARAM_COUNT). With property 2 holding, this
  // makes the groups a partition: no parameter can be fitted by two --only runs
  // and none is unreachable from every one of them.
  CHECK_MESSAGE(uncovered == 0,
                uncovered << " of " << eval_model::PARAM_COUNT
                          << " parameters are in no --only group; the first is "
                             "index "
                          << first_uncovered
                          << ". A parameter block added without a group of its "
                             "own looks exactly like this");
}


// --freeze, added at S065. The inverse of --only: free everything except the
// named groups. What it exists for is a fit that has to hold two groups at zero
// while the other 817 parameters move, which --only cannot express -- it says
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
  REQUIRE(eval_model::PARAM_COUNT > 0);

  std::vector<uint8_t> all;
  REQUIRE(tuner_groups::free_mask("all", &all));
  REQUIRE(all.size() == eval_model::PARAM_COUNT);

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

    for (size_t i = 0; i < eval_model::PARAM_COUNT; ++i) {
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

  for (size_t i = 0; i < eval_model::PARAM_COUNT; ++i) {
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
      eval_model::PARAM_COUNT - tempo_count - placement_count;

  CHECK(free_count == expected_free);

  for (size_t i = 0; i < eval_model::PARAM_COUNT; ++i) {
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
