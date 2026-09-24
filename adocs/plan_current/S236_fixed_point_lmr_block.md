id:         S236
goal:       the late move reduction is accumulated in fixed point -- the table's value and every node and move term as fractions of a ply -- and rounded to whole plies once at the site that uses it, and S098 verdict 1's history-scaled term returns as a fractional contribution, the two measured as one block because neither means anything without the other
accepts:    one SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; the accumulator's scale is a compile-time constant in `src/search.cpp` beside `build_lmr_table`, the table holds fixed-point values, `lmr_adjusted_reduction` sums table and node terms in that unit and rounds once at its return, and `lmr_node_adjustment`'s four S098 terms are expressed in the same unit; with the history term at its off value and the rounding set to truncate exactly as today's `uint8_t` cast does, the tree is the parent's exactly, bench signature identical (INV-6), proved on the tree and not assumed from a range's end (DEC-215); the history term's scale and clamp are parameters in `src/search_params.hpp` with stated ranges, seeded at form (b) from a census of the fitted quiet-history sum's distribution at depth 12 the step runs at its start (the DEC-212 pattern), never from S098 verdict 1's fitted 699 / 3, which were fitted against a term of a different unit; `accepts` states that per-part attribution is deliberately forfeited (DEC-082) and names the bisection on H0 -- leg 1 the accumulator alone at `{-5, 0}`, leg 2 the term alone at `{0, 5}`; a test asserts the rounding rule at its boundary and a test asserts the fractional term moves a reduction by less than a ply where the whole-ply form could not; a mutant per part is killed (`tools/mutation_check.py`); `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tools/mutants/ (S236's own, and seven anchors of S095's and S098's that the new unit moved), adocs/data/, MANUAL.md, DEV_MANUAL.md; src/data_structures.hpp was named when the step was written and was not needed
excludes:   any further reduction term (S237, S238, corrplexity); which moves are reduced; the re-search rule (`lmr_research_depth`); any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-213, DEC-082, DEC-212, DEC-215, DEC-141, DEC-105, DEC-134
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-22
done:

## Why this exists

`src/search.cpp` `build_lmr_table` truncates a `double` into a `uint8_t` and
`lmr_node_adjustment` adds whole plies to it: a term that wants to say
"reduce a third of a ply less here" can say nothing or a whole ply. S098
verdict 1 measured the history-scaled reduction at -2.92 +/- 5.02 and its
bisection leg at -2.34 +/- 4.73, and the term left the tree (DEC-213). The
2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, section
3.3) records that the open-source record measures the same idea at +9.09 over 4970
games on a reduction accumulated in fixed point, and names three differences
that could explain a sign flip; the accumulator is the one testable without
also changing the history sum. Its review (`adocs/audit/2026-09-19_study_review.md`,
F13) adds the honest framing: no published number prices fixed point itself,
and the accumulator alone changes only rounding, so the hypothesis is tested
only with the term restored as a fraction. Hence one block: the parts are
inert in isolation, which is DEC-082's precondition, and H0 is bisected.

## Shape

The table becomes an array of fixed-point values; `lmr_adjusted_reduction`
returns the rounded sum; the four S098 node terms scale to the unit so the
shipped tree is reproduced exactly at the off configuration. The history term
returns as `-(hist_sum * scale) / divisor`, clamped, in the same unit -- a
fraction of a ply at typical sums rather than S098 verdict 1's whole ply.

## Seeds (DEC-134)

The accumulator's scale is a power of two chosen for the integer range,
stated as such (form (c), a design constant and not a tuned one). The
history term's divisor and clamp: form (b), a census of `quiet_history_sum`'s
distribution at depth 12 over the bench positions, run at the step's start
and recorded under `adocs/data/`, seeding the divisor at the value that makes
the p90 sum a half-ply.

## From the description (DEC-221)

The implementing agent's brief carries this file, the analysis's section 3.3
in prose and DEC-213's record; the technique is implemented from that
description, every constant seeded in DEC-134's forms, and the stamp says so.

## Cost

One verdict at DEC-143's price, 12 to 20 hours. The two bisection legs the step
was written with are gone: leg 1 is an identity a rebuild proves, and leg 2 is
the verdict itself. One pre-registered follow-up of the same price exists and is
taken only on an inconclusive reading -- the same term at `LmrHistClamp` 1024.

## What landed

Written 2026-09-22 with the machine held by S132's confirmation, built and
measured 2026-09-23 when it freed. The three subsections below are the design,
and every number in them was taken on this tree; where a choice was made before
the measurement and the measurement then overruled it, both are here, in that
order, because a step file that records only the surviving choice is a step file
that cannot be audited.

### The accumulator

`LMR_SCALE_SHIFT` and `LMR_SCALE` sit beside `build_lmr_table` in
`src/search.cpp`, 10 and 1024. The table's element type went from `uint8_t` to
`int32_t` and it stores `r * LMR_SCALE` truncated rather than `r` truncated,
which is 16 KB against 4 KB and is the one cost of this shape that is not free:
the working set of the hot region grows with it and the nodes per second are
read at the landing rather than assumed. Nothing narrower holds what a tune
build can ask for -- at `LmrBase` 400 and `LmrDivisor` 1 the formula reaches
1720.5 plies, 1.76e6 ticks -- and capping the table instead would be choosing a
range for the storage rather than for the rule.

Two properties decide the scale and neither is a guess at a good value: a power
of two makes the rounding divide an arithmetic shift, which is an exact floor at
both signs where C's `/` truncates a negative sum toward zero, and 1024 is the
largest power of two that keeps `hist_sum * LMR_SCALE` -- the widest
intermediate in the accumulator -- a factor of three inside `int32_t` at the
widest vector a tune build can be driven to. DEC-134 (c), a design constant,
stated as one. A `static_assert` beside it holds the shift to a floor rather
than trusting the sentence about C++20.

`lmr_node_adjustment` returns its five terms in ticks (`LMR_CUTNODE * LMR_SCALE`
and so on); the five constants keep their whole-ply meaning, their ranges and
their UCI surface. `lmr_adjusted_reduction` sums table, node terms and the
history term and rounds once at its return, through `lmr_plies_of`. Nothing
below it rounds: `SeeLmrExtra` is still added in plies at the site, the clamp to
`child_depth - 1` is still there, and `lmr_depth_of` still clamps at zero from
below.

### The rounding, and what the mate guard said about it

`LmrRoundBias` is the rounding rule and not a coefficient of it:
`(ticks + LmrRoundBias) >> LMR_SCALE_SHIFT`. At 0 it floors, which is exactly
what the `uint8_t` cast did, and that is the off value DEC-215 wants proved on
the tree. At 512 it rounds to nearest. The range is 0 to 1023 -- below 0 is not
a rounding rule and at 1024 the bias would add a whole ply to every reduction,
which is `LmrBase`'s job.

**It ships at 0.** The seed was 512, the midpoint, and the fast suite refused
it: "pruning does not hide a forced mate" goes red there, and at every bias from
one tick upward with the history term live. The whole reading is in the stamp
section below, with the six builds that produced it. The consequence for this
step's shape is large and good: the accumulator is bit-identical to the parent,
so the candidate is one change and not two.

### The history term, and where it is read

`lmr_history_ticks` is `clamp(hist_sum * LMR_SCALE / LmrHistDiv,
+/-LmrHistClamp)` and the reduction subtracts it. The divisor is stated as **the
sum that buys one whole ply**, which is the unit that makes the seed readable;
the clamp is in ticks, so 1024 is one ply.

`hist_sum` is `quiet_history_sum`, read **pre-make** for quiet moves only, at
the same site S098 verdict 1 read it: after `make_move` the butterfly table is
indexed by the other side, and a read there would score the move against the
opponent's half of the table. It reaches both consumers, the reduction and
S109's shallow-depth gate through `lmr_depth_of`, which is verdict 1's shape and
what keeps "the gate and the reduction are one number" true -- the sentence that
function's own comment has carried since verdict 2. The two sites with no move
to read a history for -- the late-move-pruning count, decided before
`pick_next_move`, and the capture gate -- pass `NO_HISTORY_SUM`, which is back
with its term.

## The census and the two seeds

**Estimated before it ran, 2026-09-23 03:55 (RUNS, DEC-155):** under fifteen
minutes, so it starts at once rather than waiting for a night. Two Release
builds of the `chesso` target in a fresh worktree against a warm ccache, two
`bench` runs at depth 14 for the signature comparison, and three drives of the
eight bench positions -- depth 10, depth 12, and depth 12 again for the
counterfactual. Nothing here is a match and nothing holds the machine for an
hour; the four-hour line is not in question.

`adocs/data/S236_hist_census.py` ran in about four minutes of that estimate,
most of it the two builds. Its method, in full, is its own docstring; what
matters here is what it decides and what it refuses.

- **The tree it measures is the tree the term is added to.** The throwaway copy
  has `LmrHistClamp` forced to 0, and the control binary the signature is
  compared against is built from the same forced source, so the comparison is
  like against like and the distribution is the term's input rather than its
  output. The two binaries printed `bench` 4493659 apiece -- the parent's own
  signature, which is what that tree is -- and their equality is what says the
  counters are write-only.
- **The seed rules are stated before the numbers exist.** `LmrHistDiv` is twice
  the p90 of the absolute sum, so the p90 site is worth half a ply and the
  typical site less -- a fraction at the typical site is the whole hypothesis.
  `LmrHistClamp` is the p99's own contribution under that divisor, capped at the
  declared two plies, and the file says so if the cap binds.
- **A second drive prices the pair.** Percentiles say how large the term is;
  for a fractional term only a crossing count says how often it changes a
  reduction, because that depends on where each site's accumulated sum sits
  against the rounding boundary. The same binary is driven again with the
  derived pair in the environment and counts, per site, whether the pair would
  have moved the whole-ply reduction and by how much.

**It ran on 2026-09-23 and the numbers are in
`adocs/data/S236_hist_census.txt`.** Two binaries built from the same
clamp-0 source, `bench` 4493659 on both -- the parent's own signature -- so the
counters are write-only. At
depth 12, over 232136 sites in 1826371 nodes: p50 **0**, p75 53, p90 **367**,
p95 1219, p99 **4883**, and **67.12 % of the sums are exactly zero**. At depth
10 the shape is the same on 74595 sites (p90 174, p99 3885).

The two rules give `LmrHistDiv` **734**, which is twice that p90, and a clamp
of 4883 * 1024 / 734 = 6812 ticks -- **6.7 plies, so the declared top of two
plies bound it and S236 shipped `LmrHistClamp` at 2048, a range bound and not a
fit** (v2 flips it to `LmrHistClamp` 1024, which is the verdict's own
pre-registered follow-up and not a re-reading of the census -- the last section
of this file). The rule's own assumption is what failed: it sized the clamp from
the p99 on the expectation of S098's tail, where p99 / p90 was 3.7; here it is
**13.3**. The cap was pre-registered for exactly this case and the file says it
bound.

What the cap costs was measured rather than argued, in the census's second pass
and again on the bench. The counterfactual says **12.54 % of sites see their
whole-ply reduction move** at (734, 2048) -- 7.12 % by one ply, 5.42 % by two --
and **the same 12.54 % move at (734, 1024)**, all of them by one ply. The cap
does not change which sites move, only how far 5.42 % of them go. On the bench
the difference is not small: against the parent's 4493659 nodes the shipped pair
searches **6858745, +52.6 %**, where a clamp of 1024 searches 5193174 (+15.6 %),
512 searches 5406957 (+20.3 %) and the shipped clamp with the divisor doubled
searches 5713670 (+27.2 %).

**A reviewer should read that as the open question of this step.** Every one of
those pairs is inside the declared range, the census sizes none of them better
than another, and choosing one after seeing these numbers is a decision rather
than a seed -- so the pre-registered rule's own output ships and the choice is
on the record. What can be said without a game is that the shipped pair pays
53 % more nodes per depth for a term the census says touches one site in seven.

S098 verdict 1's fitted 699 and 3 are **not** reused and are not comparable:
they were fitted against a term whose smallest step was a whole ply, and that
term measured zero at three scales (DEC-213).

## Tests and mutants

Both fast suites are green, 40 of 40 in each, and `./clang-format.sh --check`
passes. Five cases were red on the way there and each red is recorded with what
caused it.

| case | what it pins |
|---|---|
| `the reduction's rounding is the bias added before the shift` | the scale is a power of two; `LmrRoundBias`'s declared range is the scale's own at both ends; both sides of the boundary at the live bias; the floor at negative sums, where a division would answer zero; the table's ticks against `build_lmr_table`'s own formula, so a self-consistently wrong unit cannot pass |
| `the history term is a signed fraction of a ply inside its clamp` | zero sum is no term; the sign, both ways; the divisor is the sum worth one ply and half of it is half a ply to within the integer division's own tick; the clamp on both signs at sums outside the band; and the **application**'s sign, where a fractional term at the rounding boundary drops the reduction by one ply and no more |
| `a fraction of a ply of history moves a real reduction` | the site. Two drives of one node differing only in a flat butterfly fill, with the whole-ply form's own answer -- `hist_sum / LmrHistDiv` -- asserted to be zero at that fill, so every difference read is one S098 verdict 1's form could not have produced. Direction and magnitude together: never reduced more, never by more than a ply |
| `at the off configuration the reduction is the engine before S236` | the tune build at `LmrRoundBias` 0 and `LmrHistClamp` 0 over the whole table, at three node adjustments and three history sums, against the parent's own line restated -- `static_cast<uint8_t>` of the double. DEC-215's arithmetic half; the tree half is the bench signature above |
| `the rounding bias moves the boundary it is the rule for` | **new, and tune-build only.** Four biases across the range, each at its own boundary and on the negative side too. It exists because the shipped bias is 0, where the release suite cannot see the rule at all -- and it is why W01 below is declared equivalent rather than left to a pass that builds Release only |
| `the node-type adjustment is the sum of its four terms` | unchanged in meaning and moved to the new unit: the five terms are asserted against `LMR_CUTNODE * scale` and the rest, and a whole number of plies still survives the rounding exactly |
| `pruning does not hide a forced mate` | unchanged, and it decided the shipped rounding. See the stamp section |

**The five reds on the way, each with its cause.** Four of them were the same
one: `a node expected to fail high...`, `...is not improving...`, `...table move
is a capture...` and `...carries no move...` each failed their own precondition
`REQUIRE(reduction[k] < NODE_TYPE_DEPTH - 2)` at `4 < 4`, because at
`LmrRoundBias` 512 the drive's reductions round up into the move loop's clamp
and the case can no longer see a one-ply term. The fifth was
`pruning does not hide a forced mate`. All five are green at the shipped bias of
0 and none of them was touched. The sixth red was mine and it was a wrong
assertion: the rounding case pinned `declared_default("LmrRoundBias")` to the
scale's midpoint, which is a seed and not a rule, and `golden_defaults` in
`tests/test_search_params.cpp` is where every default is pinned (DEC-142). It
now asserts the default is inside the declared range and says why the midpoint
is not it.

The fixture gained one argument for the site case: `node_type_drive_t::run`
takes a flat quiet-history fill. Flat rather than a bonus on one move, for two
reasons that are both about keeping two drives comparable -- with `prev_move` 0
the continuation half of `quiet_history_sum` is skipped, so a flat fill makes
the sum exactly the number the case chose at every quiet, and score_move adds
the same entry to every quiet's order, so the fill cannot reorder the node where
a bonus on one move would promote it and shift every index after it.

`tools/mutants/S236_fixed_point_lmr.py` holds six, one per part and two for the
unit. **The pass was re-run on the final tree** -- the seeds moved and the mate
row was re-mined after the first one, so the first pass's fixture no longer
described the engine -- over those six, the seven anchors this step re-cut,
S095's J03 (the re-mined row's third killer) and E21 for the record:
**`mutation score 13 of 14 (93%)   equivalent 1, killed 13, survived 1`**,
2125 s, fixture `79bf5fe` clean at its own HEAD and the mutant list clean too,
baseline green over 40 tests at bench 6858745
(`.tuning/coord/S236_mutation2.log`, results in
`.tuning/coord/S236_mutation2.tsv`). The one survivor is E21, which is the
finding below and not a hole in this step's own guards:

| mutant | result | killed by |
|---|---|---|
| `J01_no_tt_move_inverted` | killed | 2 of 40, bench moved -- **re-cut anchor**, and the re-mined row is one of the two |
| `J02_no_tt_move_dropped` | killed | 1 of 40, bench moved -- re-cut anchor |
| `J03_site_entry_absent_only` | killed | 1 of 40, bench moved -- S095's third, run because the re-mined row is its row |
| `T02_cutnode_inverted` | killed | 2 of 40, bench moved -- re-cut anchor |
| `T03_improving_inverted` | killed | 2 of 40, bench moved -- re-cut anchor |
| `T04_ttcapture_inverted` | killed | 1 of 40, bench moved -- re-cut anchor |
| `T05_pv_added` | killed | 1 of 40, bench moved -- re-cut anchor |
| `T06_adjusted_reduction_ignores_node` | killed | 1 of 40, bench moved -- re-cut anchor |
| `W01_round_bias_dropped` | **equivalent** | 0 of 40 and **bench same**, which is the tool's own oracle agreeing with the argument: at a shipped bias of 0 the deleted term is provably zero and the mutated engine is the same engine |
| `W02_table_unit_halved` | killed | 1 of 40, bench moved |
| `W03_hist_clamp_dropped` | killed | 1 of 40, bench moved |
| `W04_hist_sign_flipped` | killed | 1 of 40, bench moved |
| `W05_hist_whole_plies` | killed | 1 of 40, bench moved |
| `W06_node_terms_unscaled` | killed | 1 of 40, bench moved |
| `E21_multicut_mate_band_gate_dropped` | **survived** | 0 of 40, bench moved -- S097's, killed on the parent and not here; the finding below |

The named cases each mutant died to:

| mutant | killed by |
|---|---|
| `W01_round_bias_dropped` | **declared equivalent at the shipped default**, with the argument in the mutant file: at `LmrRoundBias` 0 the term it deletes is provably zero, so the mutated engine is the same engine and no Release case can kill it. Observed red by hand under the tune-build case above -- the S033 protocol, log `.tuning/coord/S236_W01_red.log` -- and the declaration comes back out the day a non-zero bias ships |
| `W02_table_unit_halved` | the rounding case's formula assertion and the off-configuration case |
| `W03_hist_clamp_dropped` | the term case |
| `W04_hist_sign_flipped` | the term case's application half and the site case |
| `W05_hist_whole_plies` | the site case -- this is S098 verdict 1's own shape in this step's unit, and a suite that cannot tell the two apart cannot tell whether this step did anything |
| `W06_node_terms_unscaled` | the node-type case |

## The verdict

`adocs/data/S236_sprt.sh` is written and unpinned, and the measurements above
shortened it. **There is no bisection.** The block was written as two parts with
per-part attribution forfeited; the rounding then measured out of the tree, the
accumulator is a proved-neutral refactor at the shipped bias, and what plays is
the history term alone. Leg 1 is struck -- an identity INV-6 proves in a rebuild
owes no games -- and DEC-082's forfeit is void with it: whatever the run reads,
it reads about the term.

The three readings, all written before a game:

- **H1** -- keep both seeds for S127 to fit, the clamp first because it ships on
  its cap; and **before the step completes, re-mine the E21 row** with
  `adocs/data/S097_mine_mate_row.py` on the shipping tree, the new row proved by
  a targeted `--only E21` scoring it killed.
- **H0 with the interval wholly below zero** -- a loss. The term leaves the tree
  again with its two settings, its cases and its mutants, and DEC-213 is
  reaffirmed on the form it did not have: whole-ply zero twice, fractional a
  loss, three readings of one idea. **The accumulator stays** at `LmrRoundBias`
  0, because it is behaviour-neutral by the identity proved above and because
  S237 and S238 both need a reduction that can carry a fraction; the stamp says
  so, so that a later reader does not read the surviving code as an unmeasured
  rule.
- **H0 whose interval reaches above zero, or a stalled walk** -- one
  pre-registered follow-up and only one: the same term at `LmrHistClamp` 1024,
  its own pinned pair, `{0, 5}`. If that reads H0 the term leaves as above.
  **Why it is pre-registered now and not chosen now**: the shipped clamp is a
  range bound and not a measured value -- the rule asked for 6812 ticks and the
  declared top of 2048 bound it -- and the census, read before any game, says
  the cheaper clamp moves the same 12.54 % of sites, all by one ply, for +15.6 %
  of tree instead of +52.6 %. That makes "the term is right and the cap is too
  expensive" a hypothesis an inconclusive walk would make worth a night. It is
  not a number picked off the bench after a verdict, and the difference is that
  this paragraph exists first.

The file also carries DEC-143's cost (41861 and 25591 games, 19.8 h and 12.1 h
at 2110 an hour), the abort rule read per side, the prior in full, the measured
sections above, and the open findings by id -- **E21 unpinned on this tree**
(DEC-171, this candidate's own doing) and **S239 closed** rather than open,
since its two refusals landed at `3b0fbe6` before this step's pass ran on them.

## Proposed sentences for `adocs/specs.md`, search row

The coordinator owns that file; these are the words this step proposes, to be
applied when the block lands and amended when the verdict closes.

> The late move reduction is accumulated in fixed point. The table holds ticks
> of a ply -- 1024 to the ply, a compile-time constant beside the table's
> builder -- the node-type terms and the move's history term are summed in the
> same unit, and the sum is rounded to whole plies once, at the site that uses
> it, by a bias added before the shift. `LmrRoundBias` ships at **0**, which
> floors and is what the engine did before: round-to-nearest loses a mate the
> fast suite guards, so the rounding rule is live and settable but plays as the
> parent's. What plays differently is the move's own history: a late quiet's
> reduction moves by a **fraction** of a ply --
> `clamp(hist_sum * 1024 / LmrHistDiv, +/-LmrHistClamp)`, subtracted, on the
> same sum the move ordering reads -- and the same number gates the
> shallow-depth pruning block. `LmrHistClamp` ships on its declared cap of two
> plies, which its census rule overshot, and at 0 the reduction is the engine
> before S236, bench signature included.

**And one sentence in that row is stale the moment this lands**, in the
shallow-depth block's paragraph: it gives the gate as the reduction-adjusted
depth `max(0, depth - lmr_reduction(depth, move_number))`. That function no
longer exists under that name -- the table's own cell is `lmr_reduction_ticks`
now and the gate reads `lmr_depth_of`, which since S098 verdict 2 has carried
the node adjustment and since this step carries the move's history as well. No
test flags it (`--citations` runs over the two plan directories and not over
`specs.md`), which is exactly why it is named here. Proposed:

> ... all gated on the same reduction the move is searched with, `max(0, depth -
> lmr_adjusted_reduction(...))`: the table's ticks, the node-type terms and the
> quiet's own history summed and rounded once, which is why a node type that
> reduces harder also prunes earlier ...

## The seven anchors this step re-cut

`tools/mutation_check.py` refuses the **whole** run when one anchor is stale, so
this step's own mutation pass was blocked by five of S098's mutants and two of
S095's: T02 to T06 and J01, J02 anchor on the five lines that now read
`* LMR_SCALE`, and T06 on a helper body that now rounds. Each was re-cut to the
line as it reads today, inverting, dropping or flipping exactly what it
inverted, dropped or flipped before, and both files' headers say so. **No
mutant changed meaning and no id was retired**: an anchor is a coordinate, and
the kills those seven are on record for are not re-earned by opening new ids.
The whole list of 122 validates against this tree, and **all seven were re-run
on it and all seven are still killed**, which is the evidence that re-cutting
them kept what they were for.

## The census was re-taken, and the seeds moved with it

**The first census measured a tree that was then rejected.** It ran at 03:56,
while the working tree still carried `LmrRoundBias` 512; its instrumented bench
was 3928807, the round-to-nearest accumulator's. An hour later the fast suite
put the bias back to 0, and that first census's tree stopped existing. The rule
it was taken under says the census runs "on the tree the term is added to", so
the run was re-taken on the tree that ships minus the term: **instrumented bench
4493659, the parent's own signature**, which is what that tree is.

Same script, same rules, different tree, and the numbers moved: p90 318 -> 367,
p99 4815 -> 4883, and `LmrHistDiv` 636 -> **734**. The census's one-pass caveat,
happening in the small and inside one morning. The superseded run is named in
`adocs/data/S236_hist_census.txt`'s own appendix, with its bench, so a reader
who finds 636 quoted in an earlier draft can tell which tree it belonged to.

**And the seeds moving turned the fast suite red.** At (734, 2048) the case
`pruning does not hide a forced mate` fails its row "mate the extra ply hides,
depth 11": the engine finds a forced mate and reports it as **mate in 3 where
the position is mate in 2**. That is the next section.

## S236 v2: the flip, and S097's row re-mined with it

DEC-231's third reading binds and the follow-up is one default:
`LmrHistClamp` 2048 -> **1024**, with its `golden_defaults` row and its
`MANUAL.md` row. Nothing else in `src/` moves.

**The flip turns a second mined row false.** At 1024 the case
`pruning does not hide a forced mate` fails at S097's own row, "mate the
multicut hides, depth 14" -- `REQUIRE( result.mate_found )`, `values:
REQUIRE( false )` -- while S095's row, the one this step re-mined on the 2048
tree, passes. The same rule applies as before: a golden whose derivation moved
is re-derived by its own script, `adocs/data/S097_mine_mate_row.py`, and both
rows stay in the GOLDEN block with the tree each belongs to.

**A third golden moved with the flip, and a second estimate, 2026-09-23 16:24
(RUNS, DEC-155):** `tests/test_mate_carry.cpp` "a mate score carried across
searches keeps a line that reaches it" reports **2 short mating PVs for
`C_mate7_depth11` against a recorded ceiling of 0**. The case's own comment says
what to do and forbids the shortcut: re-run `adocs/data/S203_case_sweep.sh`
before deciding whether it is a regression or a cell the grid does not cover,
and never raise the ceiling to match a run. The grid is 6 cases x 2 strides x 9
budgets, 108 cells, the largest at 4 M nodes: **about forty minutes**, and the
ceilings are then re-derived by the script's `--ceilings` mode over the tracked
grids.

**Estimated before it ran, 2026-09-23 15:46 (RUNS, DEC-155):** about forty
minutes. Two sweeps of 269 candidates over depths 11 to 14 -- the range the
script derives from `SeMinDepth` and does not write out -- one against this
tree's engine library and one against a library built with `E21` applied, the
separator set derived from the two, the firing witness over the separators on
the tune library at `SeMultiCut` 0 and 1, and the pick. The candidate set is
not re-derived: it is the corpus and the oracle, neither of which moved.

### The mutation pass on the flipped tree, and E21 closed

Re-run on a fixture of the final tree, `baf1721` clean at its own HEAD with the
mutant list clean too, baseline green over 40 tests at bench 5193174:
**`mutation score 14 of 14 (100%)   equivalent 1, killed 14`**, 2197 s
(`.tuning/coord/S236v2_mutation.log`, results in `S236v2_mutation.tsv`). The six
W mutants, the seven anchors this step re-cut, S095's J03 and E21 -- all killed
but W01, which is equivalent at a shipped bias of 0 with its bench unmoved, as
it has been since the bias was measured out of the tree.

**`E21_multicut_mate_band_gate_dropped` is killed here**, where it survived
verdict 1's tree, and that closes the finding this step opened: the row that
kills it had been mined on a tree the term does not produce, the flip turned
that row false outright, and the row the re-mine returned kills the mutant --
1 of 40 cases with the **bench signature unmoved**, which is the tool's own
example of a mutant the signature cannot see and the suite can. What remains is
a dependency rather than a hole: the kill rests on a row that belongs to this
tree, so an H0 restores the predecessor and E21's kill goes back to resting on
that.

### The follow-up's own verdict

`adocs/data/S236_v2_sprt.sh` is written and unpinned. It is **DEC-231's one
follow-up and not a second try**: the value was in verdict 1's own
pre-registration before its first game, with the reason -- the shipped clamp was
a range bound rather than a fit, and the census had already said what the
cheaper one does. The file quotes verdict 1's result block as what is being
followed up, pins `REF` to **`666b5a0`**, the tree without the term, rather than
to the flip's own parent, because the question is what the term at this reach is
worth against no term at all.

The two readings, DEC-231's own words:

- **H1** -- keep the term at (734, 1024) for S127 to fit, noting that fitting
  the clamp upward needs the range widened past its top; and re-mine the E21 row
  on whatever tree then ships before the step completes.
- **H0, or a stalled walk** -- **the term leaves the tree.** Its two settings,
  the cases that read them, the three mutants that break them, both re-mined
  mate rows and the raised mate-carry ceiling go with it, and their predecessors
  come back from where each is kept -- a revert, not a third re-mine. DEC-213 is
  then reaffirmed on the form it did not have: whole-ply zero twice, fractional
  twice more. **The accumulator stays** at `LmrRoundBias` 0, behaviour-neutral
  by the identity proved above and needed by S237 and S238. **There is no third
  run.**

### What the flip measured

| | |
|---|---|
| `bench`, parent `666b5a0` -> candidate | 4493659 -> **5193174, +15.6 %** -- the census's own prediction, to a tenth of a per cent |
| the eight replies | **all eight are the parent's**; at 2048 two of them moved |
| `search_bench` at 9 | 21479 / 102462 / 33148 -> 25231 / 104152 / 29307, best moves unchanged |
| `search_bench` at 12 | 149688 / 459216 / 219544 -> 221227 / 485833 / 204838, best moves unchanged |
| the off value, `LmrHistClamp` 0 | 4493659 with all eight replies identical, and `search_bench` identical at both depths |
| verdict 1's tree, for the record | 6858745, +52.6 % -- the follow-up buys back two thirds of what the term cost |

The third bench position **shrinks** at both fixed depths while the first two
grow, which one total hides.

### The rows this flip moved

S095's row, re-mined for the term at 2048, still holds at 1024. Two other
goldens did not:

1. **S097's own row** -- "mate the multicut hides", depth 14 -- reports no mate
   at all on this tree where it reported 5 at 2048. Re-mined by its own script;
   the section below is the whole record. Exactly one of the 269 candidates
   separates here, so there was nothing to choose between, and the row it
   returned costs **4.5 s against the old row's 0.65 s**. That is the price of a
   depth-14 mate that separates on this tree and it is recorded, not traded
   away.
2. **`test_mate_carry`'s short-line ceiling** for `C_mate7_depth11`: 2 short
   mating PVs against a recorded ceiling of 0. The case's own comment forbids
   raising the ceiling to match a run and names the re-derivation, so
   `adocs/data/S203_case_sweep.sh` was re-run over its whole grid -- 108 cells,
   committed as `adocs/data/S236_v2_sweep.txt` -- and the ceilings re-derived by
   the script's own `--ceilings` rule over the **tracked** grids, which are now
   five. The derivation was verified before it was used: the four grids the case
   already names reproduce its current numbers exactly, 5, 15, 0, 2, 11, 5.
   Adding this tree's grid moves **one** of them, C from 0 to 2, and leaves the
   other five alone. C's worst cell at its own stride is 2 short of 2 mate lines
   at 1500000 nodes, and `unreached.empty()` -- the clause that says a published
   line ends in mate -- holds on all six cases, so what rose is the residue
   DEC-122 calls expected and not the promise. S095 did the same thing one rule
   earlier and its grid is the fourth of the five.

**Three mined goldens in two clamps is the pattern worth naming**: this term
does not make the engine miss mates, it **shuffles which fixed depth finds
them**, and every golden that pins a mate at a fixed depth from a cold table is
therefore a property of the reduction vector. The two re-mined rows and the
re-derived ceiling all belong to trees this verdict can remove, so each keeps
its predecessor beside it.

## The mate row was re-mined, because the candidate made the old one false

**Estimated before it ran, 2026-09-23 06:55 (RUNS, DEC-155):** about fifteen
minutes for the re-mine, so it runs at once. Two depth sweeps of S095's 141
candidate positions at depths 3 to 12, one against this tree's engine library
and one against the same library with the guard opened, plus the two library
builds, the textual revert and its sha check, and the pick. The candidate set
itself is not re-derived: it is a property of the corpus and the oracle, not of
the engine, and `adocs/data/S095_candidates.tsv` already holds it.


The full picture, from ten builds of the case at different settings -- this is
the measurement, not an impression:

| `LmrHistDiv` | `LmrHistClamp` | `pruning does not hide a forced mate` |
|---|---|---|
| any | 0 | **pass** -- the term off is the parent |
| 636 | 2048 | **pass** -- the superseded census's divisor |
| 700 | 2048 | fail: `mate_in` 3 where the row wants 2 |
| **734** | **2048** | **fail: `mate_in` 3 where the row wants 2** -- the shipped pair |
| 768 | 2048 | fail: `mate_in` 3 |
| 800 | 2048 | fail: `mate_in` 3 |
| 1024 | 2048 | fail: `mate_found` false, the multicut row |
| 734 | 1024 | fail: `mate_found` false, the multicut row |
| 734 | 512 | fail: `mate_found` false, S095's row |
| 734 | 256 | fail: `mate_found` false, a capture-mate row |

Nine of the ten term-live settings fail some row of that case, and the one that
passes is the divisor from the census of a tree that no longer exists. **The
green was luck**, and reading it as a property of 636 would have been reading
noise as structure. Through the ordinary UCI path -- iterative deepening from a
real hash, which is how a game reaches a position -- the engine reports `g5c5`
mate 2 and `a2a7` mate 5 at every one of those settings; what flips is the
single fixed-depth search from a cold 4 MB table that `search_fen` drives.

**The row is a golden and its derivation moved, so it was re-derived** --
DEC-142's own rule, and S188's precedent when its check extension stopped the
multicut row separating. Not relaxed, not chosen: `S095_mine_mate_row.py`'s
procedure was run on the candidate tree at the shipped seeds, and the pick rule
stated in that script's header returned the row.

| | old row | re-mined row |
|---|---|---|
| position | `4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1` | `8/8/8/4k3/6q1/p1p1p3/P1P1P3/RBRB1K2 b - - 0 1` |
| shipped profile here | `2 2 2 2 2 2 2 2 3 -` over depths 3 to 12 -- a **3** at its own depth 11 and no mate at 12 | `2 2 2 2 2 2 2 2 2 2` -- distance 2 at every swept depth, the longest run the sweep can return |
| guard opened | loses d11 only on the parent; here it finds d11 and the shipped build is the one that moved | loses **d9** only |
| the row | depth 11, `mate_in` 2 | depth 9, `mate_in` 2 |

103 of the 141 candidates carry a mate the shipped build finds somewhere and 28
of those separate the two builds -- the same 28 of 141 S095's own run found on
its tree. The oracle confirmed the row in a fresh process at depth 20: **`#+2`
for Black in 1386 nodes, pv `Kd4 Ke1 Qg1#`**, the position valid and not in
check with 30 legal moves, and the mating side's `Kd4` on the line quiet,
checkless and played out of no check -- the class the term reduces. The red was
observed with the guard opened and the green with it in place, the source
reverted against its own sha (`.tuning/coord/S236_observe_red.log`), and the
whole run is `adocs/data/S236_remine.log`.

**The old row's text stays in the case's GOLDEN block**, because an H0 on this
step's verdict takes the term out and makes it the right row again: restoring it
is then a revert and not a second re-mine. The pre-registration's H0 reading
says so in the same words.

Both fast suites are 40 of 40 with the re-mined row, Release and tune.

## The coverage this candidate costs: E21, and it is measured on both trees

A second pass over S097's twenty-two singular-extension mutants scored **21 of
22** on this tree. The one survivor is
`E21_multicut_mate_band_gate_dropped` -- the multicut rule's mate-band gate
removed -- and its bench moved, so it is a real behavioural change that no case
in the fast suite caught. **On the parent it is killed**: a targeted run of
S097's list alone against a clean `444b808` fixture scored it killed, 1 of 40
cases failing, at baseline bench 4493659
(`.tuning/coord/S236_E21_parent.log`). So the loss of that kill is this
candidate's doing and not a pre-existing hole.

This is the thing S188 wrote down and it is happening again: **a mined row is a
property of the tree it was mined on.** The row that kills E21 is "mate the
multicut hides" in `pruning does not hide a forced mate`, mined on the tree that
ships today; while S188's check extension was in the tree that row stopped
separating and a re-mine picked another, and when S188's H0 reverted the tree
the original came back. The history term grows the tree by half again, and the
row stops separating for the same reason.

It is **test-side under DEC-171** -- it cannot move a reported score, move or
line -- so it is named by id in `adocs/data/S236_sprt.sh`'s open findings and
not fixed ahead of the verdict, and the resolution is the verdict's:

- **H1** -- the term stays, the row is re-mined on the tree that then ships
  **before the step completes**, with `adocs/data/S097_mine_mate_row.py` -- the
  instrument written for this exact mutant, five stages from S230's labelled
  mates -- and the new row is proved by a targeted `--only E21` scoring it
  killed. The S188 procedure, on the script S097 left behind.
- **H0** -- the term leaves, the tree returns to the parent's, and the kill
  returns with it. The finding closes with the revert and nothing is owed. This
  is exactly how S188's own instance closed.

Mining a row now would be mining it on a tree that may not ship, which is the
error S188 recorded rather than the fix.

## What is measured, and what is not

Measured on 2026-09-23, machine idle, load 1.3 to 2.2 on twelve cores:

| | |
|---|---|
| census | 232136 sites at depth 12, two binaries at `bench` 4493659 apiece |
| seeds | `LmrHistDiv` 734, `LmrHistClamp` 2048 on its cap, `LmrRoundBias` 0 |
| off-configuration identity | re-run on the final tree: `bench` 4493659 with all eight `bestmove` replies identical to `444b808` (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6); `tools/search_bench.py` identical at both depths -- 21479 / 102462 / 33148 at 9 and 149688 / 459216 / 219544 at 12 |
| the shipped tree at those depths | 25248 / 104157 / 28867 at 9 and 235756 / 664570 / 163705 at 12, best moves unchanged at 9 and `e2a6` -> `d5e6` on kiwipete at 12. **The third position shrinks at both depths** while the first two grow, which one bench total hides |
| the accumulator's own cost | the candidate **half a per cent faster**, not slower: 3826388 against 3845136 nodes per second over twelve interleaved pairs of two identical trees, +0.49 % on the means and +0.52 % paired against spreads of 1.8 %, 1.2 % and 2.1 % -- no cost and no speed-up, below the noise floor (`adocs/data/S236_nps_interleaved.txt`) |
| `bench` parent to candidate | 4493659 -> 6858745, +52.6 %, two of eight best moves changed (`e2a6` -> `d5e6`, `d8e7` -> `d8d6`) |
| both fast suites | **40 of 40, Release and tune**, with the re-mined row; `./clang-format.sh --check` clean |
| `test_mate_breadth` | 20.1 s Release, 20.5 s tune against its 120 s ceiling. S188's 2.6 %-headroom finding was about the tree that step's H0 reverted; on this one the case has 83 % of headroom and this candidate costs it about a second |
| mutation | re-run on a fixture of the final tree; the score line and the header are in the mutation section |
| mutation, S097's twenty-two | 21 of 22 killed; `E21_multicut_mate_band_gate_dropped` survives here and is killed on the parent, which is the finding above |

Not measured, and each is somebody's named next step: the Debug self-play and
`tools/gate_extra.sh` DEC-141 requires, which are the coordinator's on the
landing commit; the SPRT itself; and whether a clamp below its cap is the better
pair, which no census can answer and which S127's lane fits against games.

## For the completion stamp

Two things the stamp must carry beyond the usual, because both are choices
rather than results.

**`LmrRoundBias` ships at 0, and that is a measurement and not a preference.**
The seed was to be 512 -- the range's midpoint, which is round-to-nearest -- and
the reading of `accepts` that wanted it was that leg 1 of the bisection has to
be a run rather than an A/A. The fast suite refused it: at 512 the engine loses
the mate in `tests/test_search.cpp` "pruning does not hide a forced mate", row
"mate the multicut hides" at depth 14, which the same `accepts` requires to stay
green. The two clauses conflicted and the mate guard is the harder one, so the
bias ships at the parent's own truncation.

**The size of the bias is not what the guard is reacting to.** Ten builds were
made, each with its own pair of defaults, and each ran that one case. This is
the measurement that decided the shipped value, so it is here in full rather
than summarised:

| `LmrRoundBias` | `LmrHistClamp` | `pruning does not hide a forced mate` |
|---|---|---|
| 0 | 0 | **pass** -- the parent's own configuration |
| 0 | 2048 | **pass** -- the term alone, which is what ships |
| 1 | 0 | **pass** -- one tick of rounding, no term |
| 1 | 2048 | fail |
| 8 | 2048 | fail |
| 64 | 2048 | fail |
| 128 | 2048 | fail |
| 256 | 2048 | fail |
| 512 | 0 | fail -- round-to-nearest alone, no term |
| 512 | 2048 | fail -- the pair the step was written for |

Read the table as a whole and the shape is plain: nothing that leaves the
reduction where the parent had it fails, and nothing that moves it upward
passes, at any size. With the history term live, biases of 1, 8, 64,
128, 256 and 512 ticks **all** lose the row and 0 keeps it. With the term off, a
bias of 1 keeps it and 512 loses it. One tick is a thousandth of a ply: with the
term off it changes the rounding only where a table cell's fraction is within
one tick of the next ply, which is a handful of 3969 cells, and the mate
survives; with the term live the sums at the sites are spread, so about one site
in a thousand rounds differently -- and that is enough. What the row is
measuring at depth 14 from a cold table is therefore not "the reduction got
harder by half a ply" but "the reduction moved upward at all". The row is
knife-edge on this tree. That is a property of the row, it is reported as one,
and it is the second thing a reviewer should look at, because **it binds every
later step that reduces more** (S237 and S238 are both in that class) and
because this very row has been re-mined once before, at S188, for the same kind
of reason.

What the 0 buys: the accumulator is then bit-identical to the parent, so the
candidate is **exactly one change** -- the history term -- an H1 attributes to
that term alone, DEC-082's forfeit narrows to nothing, and leg 1 of the
bisection becomes an identity a rebuild proves instead of a run that costs a
night. What it gives up is the measurement of rounding itself: whether rounding
once, to nearest, is worth anything is now **untested rather than rejected**,
and it stays that way until either S127's lane sweeps the parameter or a step
re-mines the row that forbids it.

**The seeds are the step's own census and not S098 verdict 1's pair, and the
clamp is on its cap.** The stamp names the census file, the percentiles it read
(p50 0, p90 367, p99 4883 at depth 12, with 67.12 % of the sums exactly zero),
the two values the rules gave -- 734, and 6812 ticks capped to 2048 -- and says
plainly that the cap bound, because a reader who finds the shipped clamp sitting
exactly on its range top should be able to tell a bound from a fit without
opening another file.

**That cap is the first thing a reviewer should look at.** The term at (734,
2048) moves the whole-ply reduction at 12.54 % of reduction sites, 5.42 % of
them by two plies, and `bench` grows from 4493659 to 6858745, **+52.6 %**. The
same divisor with the clamp at one ply moves the same 12.54 % of sites, all of
them by one ply, and costs +15.6 %. So the cap buys no extra reach, only extra
depth of cut at one site in sixteen, and it is responsible for most of the
tree. The pre-registered rule's own output is what ships, because picking the
other pair after seeing these numbers would be a seed chosen from a measurement
it was not derived by. What (734, 1024) gets instead is a **pre-registered
follow-up**: it is the one run the pre-registration allows if this verdict reads
H0 with the interval reaching above zero, or stalls, and nothing at all if the
verdict is clean either way. The difference between a follow-up written before
the first game and a number picked off the bench after the last one is the whole
of DEC-213's objection, and it is why the paragraph is in the script and not in
a later conversation.

## Fast check, landing and second tier (coordinator, 2026-09-23)

**Fast check** by a cold Opus 5 reviewer over the diff before it landed:
**nothing in the arithmetic** -- no `int32` overflow at the ranges' extremes
(the widest intermediate 3.05x inside the type), the sign matching
`quiet_history_sum` and DEC-213's form, the shift flooring negative ticks
with both consumers clamping at zero as the parent did, the truncation
against floor asymmetry stated and tested, the shallow-depth gate reached
only through `lmr_depth_of`, the pre-make read confirmed, the cases not
vacuous, W01's equivalence sound, the seven re-cut anchors mutating what
their notes say; the reviewer reproduced `bench` 4493659 on the parent and
6873143 on the then-candidate, the identity at `LmrHistClamp` 0 with all
eight replies, the UCI declarations and the targeted cases. Three findings
of record: the branch base (the coordinator rebased before applying), the
census file's stale clamp line, and the decision the accepts deviation owed
(**DEC-231**); seven trivial wordings. **Fixing the census line exposed that
the census had run on the rejected round-to-nearest tree**: re-taken on the
tree the term is added to, `LmrHistDiv` 636 -> 734, and at those seeds the
mined row "mate the extra ply hides" reported the mate one ply long in the
cold fixed-depth search; the row was re-derived by its own script (DEC-142,
S188's precedent) to `8/8/8/4k3/6q1/p1p1p3/P1P1P3/RBRB1K2 b - - 0 1` at
depth 9 (distance 2 at all ten swept depths, Stockfish `#+2`, red observed
with the guard opened), the old row kept in the GOLDEN block as the one an
H0 restores; the mutation pass re-run on the final seeds (13 of 14, W01
equivalent, E21 the named gap); the nps of the accumulator alone re-measured
over twelve interleaved pairs, +0.5 % for the candidate inside a 2 % spread.

**Landed as `8b1bc79`**, `bench` 4493659 -> 6858745, squashed from the branch
`s236`'s three WIP commits after the rebase onto `666b5a0`; the specs passage
and the corrected shallow-depth gate sentence landed in the same commit.

**Debug self-play, DEC-141 clause 1**, on the landing tree's Debug build: four
rounds at 4+0.04 on `books/noob_3moves.epd`, concurrency 8, `-log level=trace
engine=true` -- **8 games, 0 `Assertion`, 0 `disconnect`**, 138548 trace lines
with 1061 `bestmove` lines (`.tuning/coord/s236_debug_selfplay/`), 07:43.

`tools/gate_extra.sh` launched detached on `8b1bc79` at 07:43
(`.tuning/gate_extra_2026-09-23_s236.log`), watcher armed with four exits and
a 55-minute ceiling; its marker is recorded below before `CAND` is pinned and
the SPRT starts.

**`tools/gate_extra.sh` on `8b1bc79`: `GATE-EXTRA-DONE 5 stages 1140 s`**
(07:43 to 08:02, `.tuning/gate_extra_2026-09-23_s236/`), prose, citations,
debug, sanitize and perft green. **`CAND` pinned to `8b1bc79` and `REF` to
`666b5a0`**, the commit the landing sits on -- the tree without the term,
whose `src/` is `3b0fbe6`'s and `444b808`'s (the parent every identity above
was proved against; the commits between are documents only) -- in
`adocs/data/S236_sprt.sh`. The SPRT is the coordinator's next action, by day:
nothing else needs the machine (DEC-155).

## Verdict, 2026-09-23: H0 read as a walk (coordinator)

The gainer SPRT of `8b1bc79` (the fractional history term at its census seeds,
`LmrHistDiv` 734, `LmrHistClamp` 2048) against `666b5a0` (the tree without
it), `{0, 5}` nElo at 8+0.08 with Hash 16 on `noob_3moves.epd`, seed
20260923080315, launched 2026-09-23 08:03:15 and `SPRT-RUN-DONE` at 15:35:32,
**accepted H0 after 15658 games**:

```
SPRT | cand 8b1bc79 vs ref 666b5a0, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | -1.60 +/- 4.25, nElo -2.05 +/- 5.44
LLR | -2.95 (-2.94, 2.94) -> H0
Games | N: 15658 W: 4737 L: 4809 D: 6112, Ptnml [752, 1843, 2665, 1863, 706]
Wall | 7 h 31 m, 2081.1 games/h, forfeits 0
Log | adocs/data/S236_sprt.log
```

LOS 23.05 %, draw ratio 34.04 %, pairs ratio 0.99. **0 time forfeits on either
side** over the PGN's 15660 games (10797 adjudications, 4863 natural ends).
`Incomplete mating PV` 8 candidate against 0 reference, an observation and not
a diagnosis (CHESS) -- the candidate's larger tree at a fixed clock reaches
fewer plies on mating lines. `adocs/data/S105_pairs.py`: 7829 complete pairs,
pair score mean 1.0056, variance 0.3051, sd 0.5523, 119.6 plies a game;
2081.1 games an hour, the candidate's tree being half again the parent's at a
fixed depth. The walk: LLR +0.20 at one hour, -0.83 at three, -2.17 at four,
back to -1.83 at six, the bound at seven and a half. Evidence:
`adocs/data/S236_sprt.log`, `adocs/data/S236_sprt_pairs.txt`; the run
directory `.tuning/sprt_s236_20260923_080315`.

### The reading

The nElo interval is [-7.49, +3.39]: it reaches above zero, so this is not the
loss DEC-231's second reading names (an interval wholly below zero) but its
third -- a zero read as a walk. The term at the census clamp does not gain 5
nElo, and it does not measurably lose either; the point estimate is two nElo
under zero with a tree half again the parent's. **DEC-231's pre-registered
follow-up binds, and it is the only one**: the same term at `LmrHistClamp`
1024 -- the census said before any game that the same 13.6 % of reduction
sites move, all by one ply, at +15.6 % of tree against +52.6 % -- as its own
pinned pair at `{0, 5}`, `S236 v2`. It is one default flip on this tree
(`LmrHistClamp` 2048 -> 1024 in `src/search_params.hpp`, its `golden_defaults`
row and its `MANUAL.md` row), landed by the step's agent with the bench
signature and `search_bench` re-taken, the mate rows re-checked at the new
clamp (the re-mined row was picked on the 2048 tree; the fast suite decides
whether it holds at 1024, and if not the row is re-derived by its script
before the landing, as before), the pre-registration's measured rows filled,
then the second tier and the SPRT. If that reads H0 the term leaves and the
accumulator stays, as DEC-231 wrote; there is no third run.

## The follow-up's fast check, landing and second tier (coordinator, 2026-09-23)

**Fast check** by a cold Opus 5 reviewer over the flip diff: one code line in
`src/` (the X-macro default 2048 -> 1024), everything else comment; the two
re-mined rows kept under the GOLDEN block with the tree each belongs to;
`REF` documented as `666b5a0` with its reason; the reviewer reproduced
`bench` 5193174 with the parent's eight replies, the off value 4493659,
`search_bench` exact at both depths, `test_search` at 9.5 s inside its 60 s
ceiling and `test_mate_carry` at 58 s inside its 120 s. Two findings of
record, both closed before the landing: the mate-carry golden's own header
and re-derivation command still quoted the ceiling of 0 and four grids
(brought to 2 and five, the command as written now reproducing the number
shipped), and the pre-registration's H0 removal list omitted the raised
ceiling and its grid (added, with the README row); two trivial wordings in
`DEV_MANUAL.md`.

**Landed as `85901b3`**, `bench` 6858745 -> 5193174, squashed from the branch
`s236-v2`'s two WIP commits; the specs passage now carries verdict 1's result
and the follow-up with a placeholder for its verdict.

**Debug self-play, DEC-141 clause 1**, on the landing tree's Debug build: four
rounds at 4+0.04 on `books/noob_3moves.epd`, concurrency 8, `-log level=trace
engine=true` -- **8 games, 0 `Assertion`, 0 `disconnect`**, 182674 trace lines
with 1270 `bestmove` lines (`.tuning/coord/s236_v2_debug_selfplay/`), 17:41.

`tools/gate_extra.sh` launched detached on `85901b3` at 17:41
(`.tuning/gate_extra_2026-09-23_s236v2.log`), watcher armed with four exits
and a 55-minute ceiling; its marker is recorded below before `CAND` is pinned
and the SPRT starts.

**`tools/gate_extra.sh` on `85901b3`: `GATE-EXTRA-DONE 5 stages 1259 s`**
(17:41 to 18:02, `.tuning/gate_extra_2026-09-23_s236v2/`), prose, citations,
debug, sanitize and perft green. **`CAND` pinned to `85901b3` and `REF` to
`666b5a0`**, the tree without the term -- the pre-registration's own pair,
not the flip's parent, because the question is the term at one ply of reach
against no term; the `src/` between them is the accumulator (identical to the
parent at its shipped rounding) and the term at (734, 1024) -- in
`adocs/data/S236_v2_sprt.sh`. The SPRT is the coordinator's next action, the
night run.

## The follow-up's verdict, 2026-09-24: no verdict, a zero (coordinator)

The gainer SPRT of `85901b3` (the fractional history term at one ply of reach,
`LmrHistDiv` 734, `LmrHistClamp` 1024) against `666b5a0` (the tree without the
term), `{0, 5}` nElo at 8+0.08 with Hash 16 on `noob_3moves.epd`, seed
20260923180311, launched 2026-09-23 18:03:11, **reached the harness's own
40000-game limit at 13:11:42 on 2026-09-24 with neither bound crossed**:

```
SPRT | cand 85901b3 vs ref 666b5a0, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | 2.61 +/- 2.64, nElo 3.38 +/- 3.40
LLR | 1.46 (-2.94, 2.94) -> none
Games | N: 40000 W: 12382 L: 12081 D: 15537, Ptnml [1743, 4725, 6895, 4762, 1875]
Wall | 19 h 8 m, 2090.5 games/h, forfeits 0
Log | adocs/data/S236_v2_sprt.log
```

LOS 97.41 %, draw ratio 34.48 %, pairs ratio 1.03. **0 time forfeits on either
side** over the PGN's 40000 games (27671 adjudications, 12329 natural ends).
`Incomplete mating PV` 25 candidate against 23 reference, an observation and
not a diagnosis (CHESS). `adocs/data/S105_pairs.py`: 20000 complete pairs,
pair score mean 1.0009, variance 0.3121, sd 0.5586, 119.7 plies a game. The
walk, from the watcher's hourly rows: LLR -0.65 at one hour, +0.36 at two,
-0.41 at three, +1.29 at six, -0.25 at seven, +1.28 at twelve, -0.33 at
fourteen, -0.63 at sixteen, +1.67 at eighteen, 1.46 at the limit -- six
crossings of zero and never within 1.2 of either bound; the estimate sat
between +2 and +4 nElo from the ninth hour on. Evidence:
`adocs/data/S236_v2_sprt.log`, `adocs/data/S236_v2_sprt_pairs.txt`; the run
directory `.tuning/sprt_s236_v2_20260923_180311`.

### The reading

This is the stalled walk the pre-registration and DEC-063 name, and it ran to
the pre-registered worst case and a little short of it (41861 games) before the
harness's round limit ended it: **a zero, read by the pre-registration's third
row**. The interval [-0.02, +6.78] nElo says what such a walk always says --
the truth sits inside the pair, most likely around +3 nElo, which is neither
the 5 the step booked nor a loss -- and the reading is the interval, not the
point estimate (S068 run 1's precedent; DEC-063). **The pre-registered outcome
binds: the term leaves the tree**, its two settings, its cases and mutants, the
two mate rows re-mined on its trees and the raised mate-carry ceiling with it,
each predecessor restored as its GOLDEN block says; **the accumulator stays**
at `LmrRoundBias` 0, behaviour-neutral by the identity proved at every landing
and the precondition S237 and S238 need; and **there is no third run**
(DEC-231). The removal is proved and not argued: `bench` 4493659 with all eight
replies identical to `666b5a0`'s binary and `tools/search_bench.py` identical
at depths 9 and 12 (INV-6).

What the two runs together say about the idea: at two plies of reach the term
read `nElo -2.05 +/- 5.44` on a tree half again the parent's, at one ply
`+3.38 +/- 3.40` on a tree a third larger. The fractional form does not repeat
S098 verdict 1's whole-ply loss, and it does not clear the bar either; DEC-213
stands on the record's shape, and S127's lane is where a pair for this term
would be fitted against games if the idea is ever reopened -- not by a third
run picking a value after two zeros.

## Removed, 2026-09-24

DEC-231's reading of the second verdict is executed: **the term is out, the
accumulator stays, and the tree is the whole-ply engine's again**.

**What went.** `LmrHistDiv` and `LmrHistClamp` with their X-macro block, their
`golden_defaults` rows and their `MANUAL.md` rows; `lmr_history_ticks` and
`NO_HISTORY_SUM`; the third argument through `lmr_adjusted_reduction`,
`lmr_depth_of` and `search_lmr_adjusted_reduction_probe`;
`search_lmr_history_ticks_probe`; the pre-make read of `quiet_history_sum` for
the reduction and the four call sites' arguments; the term's two cases and the
history half of the tune-only off-value case; the fixture's flat-fill argument;
and mutants **W03, W04 and W05**. A mutant whose anchor is not in `src/` is a
mutant nobody can run, so they went with the code they broke rather than staying
as comments.

**What stayed, and why.** `LMR_SCALE` and its shift, the `static_assert` on the
arithmetic shift, the `int32_t` table in ticks, `lmr_plies_of`, `LmrRoundBias`
at 0, the two probes the accumulator owns, its rounding case and its tune-only
case, **W01 (equivalent) and W02** -- and **W06**, which the removal brief's
range would have taken but which is the accumulator's own: its anchor
`if (cut_node) { adjustment += LMR_CUTNODE * LMR_SCALE; }` is live and the case
that kills it is live, so removing it would have dropped a working guard on the
surviving code. Flagged rather than done silently.

**The proofs.**

| | |
|---|---|
| `src/` against `666b5a0` | the only code hunks left are the accumulator's: the scale constants and the static_assert, `uint8_t` -> `int32_t`, `r * LMR_SCALE` into the table, `lmr_reduction_ticks` and `lmr_plies_of`, the five node terms scaled, the helper rounding once, the census dump reading plies, and `LmrRoundBias`'s row. Everything else is comments |
| `bench` | **4493659** with all eight `bestmove` replies identical to a binary built from `666b5a0` -- c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 |
| `tools/search_bench.py` | identical at both depths: 21479 / 102462 / 33148 with g5f6 / e2a6 / d7c8q at 9, and 149688 / 459216 / 219544 with c3d5 / e2a6 / d7c8q at 12 |
| both fast suites | 40 of 40, Release and tune; `./clang-format.sh --check` clean; the prose checks clean |
| `adocs/data/S188_repair_goldens.py` | re-run on this tree: `s207` answers **4** and `first-mate` answers **9**, both unmoved, which is what says this removal did not reach them |
| mutation | fixture `003eeff` clean at its own HEAD, baseline green over 40 tests at bench 4493659: **`mutation score 11 of 11 (100%)   equivalent 1, killed 11`**, 1685 s. W01 equivalent with its bench unmoved, W02 and W06 killed, the seven re-cut anchors killed, J03 killed -- and **E21 killed**, which is the restored S097 row doing again what the row that replaced it did, and the closing of the dependency S236 v2 left open |

**The three goldens the term moved are back, each by its own derivation and not
by an edited number.** S095's row returns to
`4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1` at depth 11, S097's to
`4N3/8/3P1ppk/4p2p/4P2P/1n1P2P1/Q4PK1/3q4 w - - 5 46` at depth 14 distance 5,
and `test_mate_carry`'s `C_mate7_depth11` ceiling to 0 by dropping
`adocs/data/S236_v2_sweep.txt` from the `--ceilings` command -- the same rule
that raised it, run with one grid fewer. Each block keeps the row the term's
tree used, with the tree it belonged to, because that is what made these three
reverts one line each. The evidence files stay under `adocs/data/`: the census
and its script, both re-mine logs, the sweep grid and the interleaved nps table
are the record of what was measured, and nothing in them is invalidated by the
term leaving.

**What the two verdicts bought.** A term this project had already measured at
zero twice in whole plies (DEC-213) has now been measured at zero twice more as
a fraction, which closes the question rather than leaving it open, and the unit
that made the fractional form expressible is in the tree at no cost for S237 and
S238 to use. That is the result; it is not a gain.
