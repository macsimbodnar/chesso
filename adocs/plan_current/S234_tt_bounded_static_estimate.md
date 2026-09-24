id:         S234
goal:       where a node's table entry holds a score whose bound points the same way as the gap between that score and the static evaluation, the pruning margins read the table's score as the node's estimate, while the raw static evaluation stays what is stored and what any later correction learns from
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; one estimate value feeds the margin tests in `src/search.cpp` `negamax` -- reverse futility and the null-move static-score condition today, razoring when S116 lands -- and the stored eval field and every history update read the raw `static_eval`, asserted by a test that plants a table entry and observes the margin decision move while the stored eval does not; a lower-bound entry above the static evaluation and an upper-bound entry below it are the only two cases that tighten, the other two leave the estimate alone, one case per branch with the precondition counted; a mate-band score in the entry never tightens, covered by a case; `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant that tightens on the wrong bound direction is killed (`tools/mutation_check.py`); at the rule's off value the tree is the parent's exactly, bench signature identical (INV-6, DEC-215); Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tools/mutants/
excludes:   any change to what quiescence does with the table score -- S130 shipped that form at the stand-pat and DEC-103 keeps it; razoring, which does not exist until S116 (its site joins then); any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-103, DEC-105, DEC-134, DEC-141, DEC-215
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-23
done:

## Why this exists

S130 measured the table score as the quiescence stand-pat at +1.14 +/- 4.04
over 16784 games -- no verdict, recorded as zero and kept (DEC-103). The
2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, row N1)
found the same tightening generalised to every margin site in the main search,
measured at +6.07 over 8414 games in the open-source record, with the raw static
evaluation kept for storage and learning. A zero on the narrow form does not
close the wide one, and the wide one is where the margins are. Reported
figures decide what to try and never what to conclude (DEC-019); the cost of
this verdict is DEC-143's, not the record's.

## Shape

`src/search.cpp` `negamax` computes `static_eval` once per node (S108). This
step adds one local, the estimate, initialised to `static_eval` and replaced
by the entry's score under the two tightening conditions, and routes the
reverse-futility test (`RFP_MARGIN`) and the null-move static-score condition
through it. Storage of the eval field, the improving flag's history and every
history update keep reading `static_eval`. The quiescence stand-pat site
(`stand_pat` in `quiescence`) is untouched.

## Seeds (DEC-134)

The rule has no constant of its own: the tightening is a comparison, not a
margin. If the implementer finds a guard is wanted -- a minimum depth for the
entry, say -- it is declared in `src/search_params.hpp` with a range whose one
end is off, seeded at form (c), the midpoint, and stated as such.

## From the description (DEC-221)

The implementing agent's brief carries this file and the analysis's row N1 in
prose and nothing else; the technique is implemented from that description,
every constant seeded in DEC-134's forms, and the stamp says so.

## Cost

One verdict at DEC-143's price: 25591 games on a bound, 41861 at the midpoint,
12 to 20 hours.

---

Everything below was written by the implementing agent, 2026-09-23. The
sections above are the step as it was agreed and are not edited.

## What the tree already did, and what this step adds

**The estimate exists and has since S109.** `src/search.cpp` `negamax_at`
computes `static_eval` once per node (S108), writes it to the search stack for
the improving flag and stores it in the entry's evaluation field, and then
computes `pruning_eval`: initialised to `static_eval` and replaced by the
entry's de-normalised score where the entry's bound points the same way as that
score's gap from the static evaluation -- `TT_BETA_NODE` with a score above,
`TT_ALPHA_NODE` with a score below -- never inside the mate band, never at a
node in check, and never from a `TT_PV_NODE`.

What S109 did **not** do is route the margins through it. Exactly one consumer
existed before this step: the futility test in the move loop,
`pruning_eval + FUT_BASE + FUT_SLOPE * lmr_depth <= alpha`. The reverse-futility
test read the raw `static_eval` and returned `static_eval - margin`.

So this step is the routing and its switch, and the rule that computes the
estimate is untouched. That matters for attribution: the futility site keeps
reading `pruning_eval` at either setting of the switch, because that is the tree
S109's own verdict measured, and moving it would put a second change inside this
one.

**Where the file's `accepts` was wrong about the tree.** It names three
destinations -- reverse futility, "the null-move static-score condition", and
razoring when S116 lands -- and only the first exists:

* **The null-move block has no static-score condition.** Read as it stands, its
  guards are `!is_pv`, `!is_in_check`, `ply > 0`, a non-zero `prev_move`, no
  exclusion, `depth - 1 - null_reduction >= 1`, both mate-band edges of beta and
  `game_phase(&game->board) > 0`. Not one of them reads a static score. The
  reduction is `NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR` and does not either
  -- scaling it by how far the static score sits above beta is **S114**, which
  is still in `plan_todo/`. Nothing was invented to fill the gap.
* **Razoring does not exist**, as the file itself says: S116 brings it and its
  site joins under S116's own verdict.

The step is therefore one site, which is why the switch is named for it.

## What landed

**`src/search_params.hpp`**: one X-macro row, `RFP_TT_ESTIMATE` /
`RfpTtEstimate`, default 1, range 0 to 1. A switch and not a setting (DEC-215):
at 0 the reverse-futility site reads `static_eval` and returns
`static_eval - margin`, which is the parent's tree exactly; at 1 both read the
estimate. No constant of its own is declared and the block says why -- the
tightening is a comparison between two numbers the node already has, so there is
nothing to seed in DEC-134's forms. A guard was looked for and not added: a
minimum entry depth is the obvious candidate and shipping one untested inside
this candidate would be a second change the verdict could not attribute.

**`src/search.cpp`**: one declaration inside the reverse-futility block,
`rfp_eval`, read by both the comparison and the return. One local rather than
two reads, so the off value cannot be half taken, and it is `pruning_eval` and
not a second probe of the slot, so this site and the futility site decide on the
same number at the same node. An `assert` that the estimate is not the
in-check sentinel, mirroring the one the futility site carries.

**The returned bound is the estimate less the margin**, and that is a choice the
step file left open. On the lower-bound branch it is the weaker of two claims
the entry already certifies -- a `TT_BETA_NODE` says a search of this position
came back at or above its score, so that score less a margin is below something
already proved -- and on the upper-bound branch it hands back the smaller
number, which a fail-soft return may always do. The alternative, deciding on the
estimate and returning `static_eval - margin`, was rejected: the node would hand
its parent a bound its own test did not argue for, and on the lower-bound branch
that bound is the smaller of the two, which throws away the certificate the
entry brought. A mate score can never come back this way, because the mate band
is excluded from the substitution before the margin is applied.

**`tests/test_search.cpp`**, in the "search: pruning and reduction guards"
suite, on a new `rfp_estimate_drive_t` over `guard_fixture_t`. The drive plants
an entry with `tt_store_entry`, reads it back and asserts every precondition
inside the drive -- the entry is present, carries the planted depth, type and
score, carries **no** evaluation (so the node's `static_eval` is a fresh
`evaluate()`), and is shallower than the node, so it orders and never answers.
Beta is put exactly where the two numbers disagree, because the substitution has
no probe field of its own and deliberately gets none: a field would let a case
pass by reading the number the site computed instead of the decision it made.

| case | plant | what it asserts |
|---|---|---|
| "a lower-bound entry above the static score moves reverse futility" | `TT_BETA_NODE`, two pawns above | the cutoff happens where the static score would not have taken it, the value returned is the estimate less the margin and not the static score less the margin, the node searched nothing, the stack kept the raw score |
| "an upper-bound entry below the static score moves reverse futility" | `TT_ALPHA_NODE`, two pawns below | the cutoff the static score would have taken does **not** happen, the node reaches its move loop and its store, and the stored evaluation field is the raw static score |
| "a lower-bound entry below the static score changes nothing" | `TT_BETA_NODE`, two pawns below | a lower bound may not lower the input: the cutoff still happens and the value returned is the static score less the margin |
| "an upper-bound entry above the static score changes nothing" | `TT_ALPHA_NODE`, two pawns above | an upper bound may not raise it: no cutoff, and the node searches |
| "a mate score in the entry never becomes the estimate" | `TT_BETA_NODE`, in the band, two legs | a score well inside the band and the band's own edge, `MATE_MIN`, are both refused where the direction test would have taken them; beta is outside the band in both legs, so what refuses them is the test on the entry's score |
| "the estimate is not read at its off value" (tune build) | `TT_BETA_NODE`, two pawns above | the same drive cuts off at `RfpTtEstimate` 1 and does not at 0, with a restorer, because the release build compiles the switch as a constant |

The stack slot `state.static_evals[ply]` is asserted in every case and the
stored evaluation field in the three whose node reaches its store -- a node that
takes the cutoff returns before the store, which is why the field is read where
the estimate *suppresses* a cutoff rather than where it causes one.

**No history update in this engine reads a static evaluation at all**, so the
`accepts` clause about history updates is satisfied by there being no such
reader: `history_on_quiet_cutoff` takes the side, the cutoff move, the quiets
tried, the depth and the previous move, and the continuation update inside it
takes no more. The two observable readers of the raw score are the stack slot
and the stored field, and both are asserted.

**`tools/mutants/S234_tt_estimate_margins.py`**: `G01_estimate_bound_direction`
(both bound tests inverted), `G02_estimate_switch_inverted` (the site reads the
switch the wrong way round, so the candidate ships as the parent), 
`G03_estimate_mate_band_dropped` and `G04_estimate_stored_as_eval` (the estimate
written into the entry's evaluation field). G01 and G03 cut lines S109 wrote and
belong here because this step is what gives that rule a second consumer and the
cases that kill them are this step's. Each anchor occurs exactly once in
`src/search.cpp` as the formatter leaves it, checked.

**Documents**: `MANUAL.md` gains the option row, traced to `negamax_at`'s
reverse-futility block and to the tightening block above it;
`DEV_MANUAL.md`'s node-signature ledger gains this step's entry;
`adocs/data/README.md` gains the pre-registration's row;
`adocs/data/S234_sprt.sh` is the pre-registration.

## Deviations, stated rather than buried

1. **`touches:` is short by three files.** Adding a parameter forces
   `tests/test_search_params.cpp` (the golden table and its count, 63 to 64) and
   `MANUAL.md` (`test_uci_surface` requires every option to be documented), and
   the step's own documents are `DEV_MANUAL.md`, `adocs/data/README.md` and
   `adocs/data/S234_sprt.sh`. None of them changes behaviour.
2. **The null-move site named in `accepts` does not exist.** Recorded above and
   in the pre-registration's header rather than worked around.
3. **The rule's own conditions are not re-tested here.** S109's block owns them
   and this step adds no new one; what the cases above test is which sites
   consult the rule and what they do with what it returns.
4. **The accepts' clause "`pruning does not hide a forced mate` stays green" is
   satisfied with a re-mined row, not with the row as it stood.** The case is
   green, and the repair is DEC-142's -- the golden's own script re-run whole on
   the tree it now guards, which returned the same position and the same
   distance at depth 8 in place of 11. It is named here rather than buried
   because the clause reads as "unchanged" and what happened is "re-derived",
   and because the sweep behind it also measured the candidate finding eight
   fewer mate cells than the parent over the whole candidate set in that cold
   fixed-depth regime. Both are the coordinator's to reverse.
5. **The second tier is still owed** (DEC-141): Debug self-play and
   `tools/gate_extra.sh` before completion. This step touches the search, so
   both bind; neither was run here.

## Proposed `adocs/specs.md` sentences, for the coordinator

Three edits to the search row, all in its current-state wording. The phrases
they point at are quoted (DEC-135).

1. The S109 passage says the estimate has one consumer -- "on a static adjusted
   by a table score whose bound certifies a direction (S108's layer (c), the
   futility margin its only consumer)". The parenthesis becomes:

   > (S108's layer (c); the futility margin was its only consumer until S234)

2. The reverse-futility passage says the rule "returns a static lower bound
   instead of searching and therefore cannot see a mate". After that sentence:

   > **Since S234 the number it is decided on is the node's estimate and not
   > the static score, at `RfpTtEstimate` 1**: the transposition entry's own
   > score where the entry's bound type certifies the direction that score has
   > moved in from the static evaluation -- a lower bound above it, an upper
   > bound below it -- never a score inside the mate band, never at a node in
   > check and never from an exact entry, which is S109's own rule with a
   > second consumer rather than a new one. The bound returned is that same
   > estimate less the margin, which on the lower-bound branch is weaker than
   > what the entry already certifies. `RfpTtEstimate` 0 puts both the
   > comparison and the bound back on the raw static evaluation and is the tree
   > before the step, bench signature included. **This engine's null-move block
   > has no static-score condition and razoring does not exist until S116**, so
   > reverse futility is the whole of the routing.

3. Beside either passage, once, since it is the invariant both rest on:

   > The estimate is a pruning input and never a stored one: the entry's
   > evaluation field and the per-ply static scores the improving flag compares
   > both hold what `evaluate()` returned, at every setting of `RfpTtEstimate`,
   > and no history update reads a static evaluation at all.

The verdict's number replaces nothing above: the SPRT decides whether
`RfpTtEstimate` ships at 1 or 0, and the sentences are written for whichever it
is with the losing half struck.

## Measurements, 2026-09-24

The machine was idle and exclusively this step's; nothing else ran.

**The off value, proved on the tree and not declared** (DEC-215). A Release
build with the X-macro default forced to 0 benches **4493659** -- `7c12686`'s
own total to the node -- with all eight `bestmove` replies identical
(c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), and `tools/search_bench.py`
reproduces that commit exactly at both depths: 21479 / 102462 / 33148 with
g5f6 / e2a6 / d7c8q at 9, and 149688 / 459216 / 219544 with c3d5 / e2a6 / d7c8q
at 12. The parent binary is a fresh build of `7c12686` in a throwaway worktree,
not an existing one.

**The candidate.** `bench` 4493659 -> **4803214, +6.89 %**, one of the eight
replies moving (kiwipete `e2a6` -> `d5e6`). `search_bench` disagrees with the
total about the sign, which is the shape of a rule that both buys cutoffs and
takes them away: at depth 9 midgame grows 21479 -> 48522 with its best move
moving `g5f6` -> `c3d5` while kiwipete falls 102462 -> 85714 and tactical
33148 -> 28080; at depth 12 all three shrink, 149688 -> 129499, 459216 ->
411457, 219544 -> 172984, every best move the parent's.

**Both suites 40 of 40**, Release and tune, and `./clang-format.sh --check`
clean with `CLANG_FORMAT_MAJOR=22`. `tools/plan_prose_check.py --citations
--touches --params --gate` clean.

**Two reds, both the switch's, both isolated by rebuilding at the off value.**

1. `tests/test_search.cpp` "reverse futility prunes on the stored static score",
   S103's case, returned 0 where it asserts 300. Its third leg plants an entry
   whose **score** was filler -- `-9999` under a `TT_ALPHA_NODE` bound, a
   TT_DEPTH_QS entry that answers no depth-1 node -- and since this step that
   score is read: an upper bound below the static score is one of the two
   entries that replace it, so the filler suppressed the cutoff the leg exists
   to observe. **The precondition moved and the assertion did not**: the plant's
   score is now one point above the planted evaluation, an upper bound the rule
   refuses, and the leg reads the eval field alone as it always did.
2. `tests/test_search.cpp` "pruning does not hide a forced mate" lost S095's
   mined row at depth 11. The row was **re-mined by its own script on this
   tree** (DEC-142, S188's and S236's precedent) and the pick returned the same
   position at depth 8, same distance 2; only the depth literal moved, and the
   old depth is kept in the block because an H0 restores the tree it was mined
   on. Observed red at the new depth with the S095 guard opened and green with
   it in place -- the S033 protocol, `.tuning/coord/S234_observe_red.log`.
   Evidence and the sweep numbers: `adocs/data/S234_remine.log`.

**What that second red costs, stated rather than argued away.** In the cold
fixed-depth regime the mining driver and `search_fen()` use, the candidate finds
**489 mate cells against the parent's 497** over the 141-position candidate set
at depths 3 to 12 -- sixteen rows losing one, nine gaining one. The instruments
that are not cold single-depth searches do not move: `test_engine`'s mate safety
over 48 constructed forced mates and `test_mate_carry`'s grid are green, and
through iterative deepening -- how a GUI drives the engine -- the candidate
reports that row's mate in 2 at every depth from 3 to 12 with node counts
identical to the parent's, at Hash 16 and at Hash 4 alike. A fixed-depth mate
count is a reading of a different tree and not of a better one (DEC-019); the
SPRT is what prices it, and the pre-registration carries the number.

**Mutation: 4 of 4 killed, 100 %.** `tools/mutation_check.py` over G01 to G04,
run on the repaired tool (S239), `.tuning/coord/S234_mutation.log`. The header's
three lines:

```
worktree /home/max/ws/chesso-s234/.ref-builds/mut at 6751cd0 clean
build    /home/max/ws/chesso-s234/.ref-builds/mut/build   jobs 12   label fast
list     /home/max/ws/chesso-s234/tools/mutants/S234_tt_estimate_margins.py   clean
```

The fixture is a linked worktree carrying this working tree as one detached
commit, so its baseline benches 4803214 -- the candidate's own total -- over 40
green tests. Every mutant moved the bench signature as well, so none of the four
can be argued equivalent. What caught each, by case:

| mutant | killed by |
|---|---|
| `G01_estimate_bound_direction` | all four branch cases, and S103's "reverse futility prunes on the stored static score" and "pruning does not hide a forced mate" with them |
| `G02_estimate_switch_inverted` | the two tightening cases; and, in the build the Release-only tool cannot reach, "the estimate is not read at its off value" |
| `G03_estimate_mate_band_dropped` | "a mate score in the entry never becomes the estimate" and the mate case |
| `G04_estimate_stored_as_eval` | "an upper-bound entry below the static score moves reverse futility" at `REQUIRE_EQ(stored->eval, raw_eval)`, and S108's "a main-search node evaluates once and stores what it read" |

**The tune-only case has its own observed red**, because `RfpTtEstimate` is a
folded constant in Release and `tools/mutation_check.py` builds Release only:
G02 applied by hand to the tune build fails "the estimate is not read at its off
value" at `REQUIRE( on.rfp_cutoff )`, and the case passes again after the
revert -- the S033 protocol, with a sha check on `src/search.cpp`,
`.tuning/coord/S234_observe_tune_red.log`.

**The off value again, from the other build.** The tune binary at
`setoption name RfpTtEstimate value 0` benches 4493659 with all eight of the
parent's replies, which proves the switch itself routes rather than only its
compiled default.

## Fast check, landing and second tier (coordinator, 2026-09-24)

**Fast check** by a cold Opus 5 reviewer over the diff before it landed: **no
defect** -- `rfp_eval` read at exactly the comparison and the return and
nowhere else, `pruning_eval`'s computation byte-identical to S109's, the
returned bound always strictly inside the mate band and never written to the
table (reverse futility returns before the store), the upper-bound branch
returning a lower fail-soft bound than before, the `TT_EVAL_NONE` hole shared
with and unchanged from S109's assert, S103's repaired leg still
discriminating the stored eval from a fresh evaluation, the re-mined row's
GOLDEN block naming its scripts and keeping depth 11 with its tree, the six
cases' preconditions real, the four anchors unique, `golden_defaults` 62 by
declaration and count; the reviewer reproduced `bench` 4803214 with the moved
reply, the off value 4493659 with eight identical replies through the tune
binary, `search_bench` at depth 12 and the targeted cases. One trivial blank
line in `DEV_MANUAL.md` fixed; the outstanding item was the second tier, the
coordinator's, below.

**Landed as `169b4cb`**, `bench` 4493659 -> 4803214, squashed from the branch
`s234`'s four WIP commits; the specs passage, the S109 parenthesis and the
stored-eval invariant landed in the same commit.

**Debug self-play, DEC-141 clause 1**, on the landing tree's Debug build: four
rounds at 4+0.04 on `books/noob_3moves.epd`, concurrency 8, `-log level=trace
engine=true` -- **8 games, 0 `Assertion`, 0 `disconnect`**, 142668 trace lines
with 1098 `bestmove` lines (`.tuning/coord/s234_debug_selfplay/`), 15:28.

`tools/gate_extra.sh` launched detached on `169b4cb` at 15:28
(`.tuning/gate_extra_2026-09-24_s234.log`), watcher armed with four exits and
a 55-minute ceiling; its marker is recorded below before `CAND` is pinned and
the SPRT starts.
