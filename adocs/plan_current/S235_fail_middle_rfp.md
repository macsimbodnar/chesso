id:         S235
goal:       a node pruned by reverse futility returns a point between its estimate and beta, weighted by one fitted parameter, instead of the estimate itself -- the first of four fail-middle sites, one site per verdict
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; one parameter in `src/search_params.hpp` with a stated range whose one end returns the estimate (today's behaviour, the off value) and whose other end returns beta, seeded at form (c), the midpoint; at the off value the tree is the parent's exactly, bench signature identical (INV-6), proved on the tree (DEC-215); a test that observes the returned score move between the two ends as the parameter moves, with the precondition counted; a mate-band estimate never interpolates, covered by a case, and `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant that interpolates on the wrong side of beta is killed (`tools/mutation_check.py`); Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tools/mutants/
excludes:   the ProbCut, multi-cut and quiescence stand-pat returns -- each is its own later step once its site exists (S113, S097), one site per verdict; the reverse-futility margin `RFP_MARGIN` and depth ceiling themselves; any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-105, DEC-134, DEC-141, DEC-215
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-24; a fresh Opus 5 subagent, briefed the same way, resumed it at its measurement phase 2026-09-25 and took everything in "Measurements" below
done:

## Why this exists

A node that prunes on a margin returns the margin's own estimate today. The
2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, row N2)
found the open-source record returning a point interpolated between that estimate
and beta at the reverse-futility site, measured at +9.71 over 4654 games, with
three further sites each measured separately afterwards at +1.4 to +4.5. The
site here is the `RFP_MARGIN` test in `src/search.cpp` `negamax`; the
returned value carries how far past beta the node stood instead of the raw
estimate. One weight, fitted here. Reported figures decide what to try, never
what to conclude (DEC-019).

## Shape

At the reverse-futility return in `src/search.cpp` `negamax`, replace the
returned estimate by `beta + (estimate - beta) * w` in integer arithmetic with
`w` the parameter over its declared scale, so that one end of the range is the
estimate and the other is beta. Mate-band estimates return unchanged.

## Seeds (DEC-134)

One parameter, form (c): the midpoint of its declared range, stated as such.
No published value seeds it and no other engine's does.

## From the description (DEC-221)

The implementing agent's brief carries this file and the analysis's row N2 in
prose and nothing else; the technique is implemented from that description,
every constant seeded in DEC-134's forms, and the stamp says so.

## Cost

One verdict at DEC-143's price, 12 to 20 hours.

---

Everything below was written by the implementing agent, 2026-09-24. The
sections above are the step as it was agreed and are not edited.

## What the tree already did, and what this step adds

**The site returns one number and has since reverse futility landed.** In
`src/search.cpp` `negamax_at`, the reverse-futility block computes
`margin = RFP_MARGIN * depth`, tests `rfp_eval - margin >= beta` and returns
`rfp_eval - margin`. Since S234 (`169b4cb`, this afternoon) `rfp_eval` is
`pruning_eval` at `RfpTtEstimate` 1 and `static_eval` at 0 -- S109's
table-tightened estimate behind a switch -- and the returned value is that
number less the margin either way.

**That returned value is what this step file calls "the estimate".** On this
tree it is `rfp_eval - margin` and nothing else, so the file's "a point between
its estimate and beta" is a point between `rfp_eval - margin` and beta. The
code and the documents say `bound` where the file says estimate, because the
node's estimate is `rfp_eval` and the margin has already been taken off the
number the blend runs from.

**Orthogonal to S234's verdict, deliberately and in the tests as well as the
code.** S234's SPRT runs tonight and may flip `RfpTtEstimate` to 0 and take the
switch out; this step replaces what the site *returns* and never what it
compares, so the blend runs over whichever number the margin was subtracted
from. The drive the cases are built on plants nothing at all -- an empty table,
so the estimate is the node's own static score at either setting -- which is
why every case here holds on the tree S234 leaves, whichever that is. One case
is the exception and says so: the mate-band case drives S234's own
`rfp_estimate_drive_t`, because a mate-band *estimate* only exists where the
entry can supply one.

## What lands

**`src/search_params.hpp`**: one X-macro row, `RFP_RETURN_WEIGHT` /
`RfpReturnWeight`, default 50, range 0 to 100. The unit is hundredths of the gap
between beta and the bound the site's test argued, and the range is stated by
purpose: at 100 the site returns that bound, which is the tree before this step
and is the off value; at 0 it returns beta exactly, which is still a legal
fail-soft bound; outside those two the node would claim more than its own test
argued or would stop failing high. The seed is **DEC-105 (c), the midpoint**,
50, stated as such -- no publication carries a number for this and the record's
own weight is another engine's tuned output, which is never a seed here
wherever it is republished (DEC-084 as amended by DEC-105, DEC-134). Implemented
from the description (DEC-221).

**`src/search.cpp`**: two locals inside the block's return -- `rfp_bound`, the
number the test argued, and `rfp_blended`,
`beta + (rfp_bound - beta) * RFP_RETURN_WEIGHT / RFP_RETURN_SCALE` -- and three
asserts: at or above beta, at or below the bound, strictly inside the mate band.
The comparison line is untouched, so the same nodes prune. One compile-time
constant, `RFP_RETURN_SCALE` at 100, beside the reduction's own scale, and
`search_rfp_return_scale_probe()` beside `search_lmr_scale_probe()`.

**The two ends, by arithmetic, in the case the tests drive.** The drive puts
beta a stated gap below the bound; take the gap as 200. At weight 100 the second
term is `200 * 100 / 100 = 200` and the site returns `beta + 200`, which is
`rfp_bound` -- today's tree. At weight 0 it is `200 * 0 / 100 = 0` and the site
returns beta. At the seed it is `200 * 50 / 100 = 100`, halfway. **The rounding
is toward beta**: the gap is non-negative because the block returns only where
`rfp_bound >= beta`, so the integer division floors -- a gap of 3 at the seed is
`3 * 50 / 100 = 1`, not 2 -- and the returned point is always in
`[beta, rfp_bound]`. Both ends of that interval are strictly inside the mate
band, beta by the block's own guard and `rfp_bound` because the number the
margin came off is either the static score, which never reaches the band, or a
table score S109's tightening refused from inside it (the guard this step's own
H04 rationale names; corrected at the cold fast check, 2026-09-25), so the point
between them is too; a mate score cannot come back this way at any weight.

**The product cannot overflow.** `rfp_bound - beta` is under `2 * MATE_MIN`,
96000, because both are inside the band, and `96000 * 100` is 9.6e6, four orders
inside `int32_t`.

**`tests/test_search.cpp`**, in the "search: pruning and reduction guards"
suite, on a new `rfp_return_drive_t` over `guard_fixture_t`. The drive plants
nothing, asserts the table is empty for the node before it runs -- so the
estimate is the node's own static score at either setting of `RfpTtEstimate` --
and asserts the whole of the block's condition, the gap it was asked for, and
**the precondition this step's accepts counts: the node pruned by reverse
futility and returned from that block**, `probe.rfp_cutoff` with
`move_count == 0`, so the value read is the rule's return and not a search
result.

| case | build | what it asserts |
|---|---|---|
| "a pruned node returns a point between beta and the bound its own test argued" | both | the returned value is the blend of this drive's own gap; it is at or above beta and at or below the bound; and where it sits, by branch over the whole declared range, so an H0 that returns the weight to 100 leaves the case true rather than red |
| "the blend rounds toward beta" | both | over a gap of 3, that what was taken is at or below the exact share and one point more would be above it -- the floor property, written in the scale's own units and not as a repeat of the site's expression |
| "the blend's scale is the weight's own declared range top" | both | `search_rfp_return_scale_probe()` equals the row's declared maximum and the floor is 0, so the scale cannot move without the off value moving with it |
| "a mate-band estimate is never the number the blend is taken over" | both | on S234's drive, two legs: a score well inside the band and the band's own edge, each with the counterfactual established (taken, it would have cut off; the static score would not have) and the hazard stated as arithmetic -- the number the blend would have run over is itself a mate score -- and the observable, that the block never fires, so nothing is interpolated |
| "the returned bound walks from beta to the site's own bound as the weight walks its range" | tune | five weights across the range, each returning the blend of that weight, each inside the bracket, each strictly above the last; both ends driven again and asserted exactly; and, through the drive's own preconditions, that the cutoff happened at every one of them -- the weight moves what the node says and never whether it says it |

**One existing case was repaired and not weakened**: `tests/test_search.cpp`
"reverse futility prunes on the stored static score", S103's. It asserts the
value the site returns, three times, at a beta far below the bound -- the
precondition moved, because that value is now the blend of the bound. The claim
is unchanged and so is what it discriminates: the two candidate bounds still
arrive as two different numbers, and the case now asserts that they do rather
than assuming it, which is the one setting (a weight of zero) that would make
the leg vacuous. Observed red before the repair, under the S033 protocol.

**`tools/mutants/S235_fail_middle_return.py`**: `H01_blend_below_beta` (the
point taken downward from beta, which breaks the fail-soft contract),
`H02_blend_scale_dropped` (the weight as a multiplier instead of a share),
`H03_blend_weight_inverted` and `H04_blend_over_a_mate_estimate`. Each anchor
occurs exactly once in `src/search.cpp` as the formatter leaves it, checked.

**H03 is declared equivalent, and the reason is arithmetic rather than a
concession.** The seed is the midpoint of 0 to 100, where `scale - weight` is
`weight`, so the inversion is the shipped engine line for line in the release
build `tools/mutation_check.py` measures -- no case there can tell them apart
and none should be written to pretend otherwise. The tune build does, at 25 and
75, which is what the walking case drives, and the red is observed by hand under
the S033 protocol as S234's `G02` was. S127's fit moves the default off the
midpoint, and on the day it does this mutant becomes ordinary.

**H04 cuts the line S234's `G03` cuts**, S109's mate-band exclusion, under its
own id. That is deliberate: this step gives that guard a second consumer and the
case that kills H04 is this step's own, asking whether a mate score can reach
the blend rather than whether it can decide the cutoff. The two ids never load
in one run.

**Documents**: `MANUAL.md` gains the option row, traced to the return of
`negamax_at`'s reverse-futility block; `DEV_MANUAL.md` gains the node-signature
ledger entry and its `golden_defaults` row moves to 63;
`adocs/data/README.md` gains the pre-registration's row;
`adocs/data/S235_sprt.sh` is the pre-registration.

## Deviations, stated rather than buried

1. **`touches:` is short by five files.** Adding a parameter forces
   `tests/test_search_params.cpp` (the golden table and its count) and
   `MANUAL.md` (`test_uci_surface` requires every option to be documented); the
   probe forces `src/search.hpp`; and the step's own documents are
   `DEV_MANUAL.md`, `adocs/data/README.md` and `adocs/data/S235_sprt.sh`. None
   of them changes behaviour.
2. **`src/search.hpp` gains a probe, which is more than "the return and the
   X-macro row".** The scale is one number in two places -- the divisor in the
   expression and the weight's declared range top, which is the off value -- and
   nothing but a case can hold them together. S236's `search_lmr_scale_probe`
   is the precedent and the same sentence is in its comment. No behaviour, and
   the release build folds the constant exactly as before.
3. **`DEV_MANUAL.md`'s `golden_defaults` row read 61 against a table of 62 on
   arrival**: S234 added `RfpTtEstimate` and did not carry the count. This step
   writes 63 and states both moves in the row rather than letting the second
   silently repair the first.
4. **The mate-band case duplicates S234's plant** and its mutant duplicates
   S234's cut, each under its own id, for the reason above.
5. **The second tier is owed** (DEC-141): Debug self-play and
   `tools/gate_extra.sh` before completion. This step touches the search, so
   both bind, and the blend is not a new pruning rule -- it changes what an
   existing one returns -- so the guard test and the mutant the rule clause asks
   for are the five cases and the four mutants above.
6. **The measurements are owed**: the machine was held by S234's second tier and
   its SPRT while this was written, so nothing here has been built or run yet.
   The section below is what fills.

## Proposed `adocs/specs.md` sentences, for the coordinator

Two edits to the search row, both in its current-state wording, both phrases
quoted rather than located (DEC-135).

1. The passage opens "Reverse futility returns a static lower bound instead of
   searching and therefore cannot see a mate". It becomes:

   > Reverse futility returns a static lower bound instead of searching and
   > therefore cannot see a mate -- **and since S235 what it returns is a point
   > between beta and that bound**, `RfpReturnWeight` hundredths of the way up
   > from the first to the second, rounded toward beta.

2. The S234 clause "and the bound returned is that same estimate less the
   margin, which on the lower-bound branch is weaker than what the entry already
   certifies" becomes:

   > and the bound the rule argues is that same estimate less the margin, which
   > on the lower-bound branch is weaker than what the entry already certifies;
   > **what the node hands its parent is `RfpReturnWeight` hundredths of the way
   > from beta up to it** (S235), which at 100 is that bound itself and is the
   > tree before that step, bench signature included, and at 0 is beta. The
   > weight moves what a pruned node returns and never which nodes prune, and it
   > is independent of `RfpTtEstimate`: it blends whichever number the margin was
   > subtracted from. Both ends are strictly inside the mate band, so no setting
   > of it can return a mate score.

The verdict's number replaces nothing above: the SPRT decides whether
`RfpReturnWeight` ships at 50 or at 100, and the sentences are written for
whichever it is with the losing half struck.

## Measurements, 2026-09-25

Taken by the agent that resumed the step, on a machine held by nothing else:
the loads read 0.97 and 0.90 at the two bench runs, and no match, fit or timing
run was started. The tree measured is the step's own work rebased onto S234's
completion, and its parent for every equality below is `169b4cb` -- the landing
commit of S234 -- built fresh in a throwaway worktree of this step's own.
`git diff --stat 169b4cb <achesso HEAD> -- src tests` is empty, so the parent
binary is the tree this step started from and not an approximation of it.

**The off value, proved on the tree and not declared from the range's end**
(DEC-215). A Release build of this tree with the X-macro default forced to 100
benches **4803214** -- `169b4cb`'s own total to the node -- with all eight
`bestmove` replies identical (c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), and
`tools/search_bench.py` reproduces that commit exactly at both depths: 48522 /
85714 / 28080 with c3d5 / e2a6 / d7c8q at 9, and 129499 / 411457 / 172984 with
c3d5 / e2a6 / d7c8q at 12 (INV-6). Evidence:
`.tuning/coord/S235_bench.log`, `.tuning/coord/S235_searchbench.log`.

**The candidate.** `bench` 4803214 -> **4823539, +0.42 %**, with all eight
`bestmove` replies the parent's. `search_bench` moves both ways, which is the
shape of a rule that hands its parent a weaker bound: at depth 9 midgame falls
48522 -> 25910, kiwipete 85714 -> 85285 and tactical rises 28080 -> 29018, all
three best moves the parent's; at depth 12 midgame falls 129499 -> 98898 and
tactical 172984 -> 125670, while kiwipete grows 411457 -> 469656 **and its best
move moves, e2a6 -> d5e6**. Node counts moving is the construction and not a
finding; nothing about the direction is a prediction (DEC-019) and
`adocs/data/S235_sprt.sh` prices it.

**Both suites 40 of 40**, Release and tune, and `./clang-format.sh --check`
clean with `CLANG_FORMAT_MAJOR=22` (`.tuning/coord/S235_tests.log`).
`tools/plan_prose_check.py` clean in all three modes -- the tool takes one mode
an invocation, so `--citations`, `--touches` and `--params` were run separately:
0 flagged over 43 files, 0 flagged over 43 files, and no parameter finding.
Every case this step added is green in both builds by name, the tune-only
walking case at 125 assertions; "pruning does not hide a forced mate" is green
in both, 54 assertions each, and nothing in it was touched.

**Two reds observed, both under the S033 protocol, both on 2026-09-25**
(`.tuning/coord/S235_observe_red.log`).

1. `tests/test_search.cpp` "reverse futility prunes on the stored static
   score", S103's case, built with its **pre-S235 assertions** over this tree's
   `src/`: `REQUIRE_EQ( run(false), pruned )` reads `REQUIRE_EQ( 300, 500 )`
   -- the bound is 500, beta 100, and the blend of them at the shipped weight
   is 300. Green with the repair in place, 12 assertions. The repair asserts
   the same claim over the value the site now returns and adds the
   discrimination the old leg assumed.
2. `H03_blend_weight_inverted` in the **tune** build, which the release-only
   mutation tool cannot reach: "the returned bound walks from beta to the
   site's own bound as the weight walks its range" fails at the sweep's first
   weight, `REQUIRE_EQ( last_score, blended_at(weight) )` reading
   `REQUIRE_EQ( -244, -444 )` -- the inversion returns the bound where the
   weight asks for beta. Green on the shipped tree, 125 assertions. The
   inversion differs at every weight the case drives except the midpoint, so
   the first one it reaches is where it fires; the section above says "at 25
   and 75" and the observation is that 0 is enough.

**Mutation: 3 of 3 killed, 100 %, one equivalent as declared.**
`tools/mutation_check.py` over H01 to H04, 1804 s, on a linked worktree of this
tree (`.tuning/coord/S235_mutation.log`). The header's three lines:

```
worktree /home/max/ws/chesso-s235/.ref-builds/mut at 02a98e8 clean
build    /home/max/ws/chesso-s235/.ref-builds/mut/build   jobs 8   label fast
list     /home/max/ws/chesso-s235/tools/mutants/S235_fail_middle_return.py   clean
```

The baseline is green over 40 tests and benches 4823539, the candidate's own
total. What caught each:

| mutant | verdict | killed by |
|---|---|---|
| `H01_blend_below_beta` | killed, 6 of 40 binaries red | this step's "a pruned node returns a point between beta and the bound its own test argued" and "the blend rounds toward beta", S103's repaired case, and "pruning does not hide a forced mate" with them; the bench did not finish inside its timeout, which is itself the mutant |
| `H02_blend_scale_dropped` | killed, bench moved | the same two cases of this step's, S103's, the mate case, and the node-budget pair |
| `H03_blend_weight_inverted` | equivalent, bench same, 0 of 40 | nothing in the release build can: `scale - weight` is `weight` at the shipped midpoint, so the mutant is the engine line for line there. The tune-build red above is the separation, by hand |
| `H04_blend_over_a_mate_estimate` | killed, bench moved | "a mate-band estimate is never the number the blend is taken over" at `REQUIRE( !inside.rfp_cutoff )`, S234's "a mate score in the entry never becomes the estimate" beside it, and `test_mate_carry` |

**One claim in the section above was written before it could be true, and is
corrected here rather than edited there.** "Observed red before the repair,
under the S033 protocol" was written on 2026-09-24, when the machine was held
and nothing in this step had been built -- the same file says so two sections
later. The red is real and is the first of the two above; it was observed on
2026-09-25 and not on the day the sentence was written.

**Owed to the coordinator at landing, not run here** (DEC-141): the Debug
self-play, four rounds of `fastchess` at 4+0.04 grepped for `Assertion`, and
`tools/gate_extra.sh`. Both load the machine the way a match does and this
step's brief reserves that to the coordinator.
