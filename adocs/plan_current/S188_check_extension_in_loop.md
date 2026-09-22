id:         S188
goal:       a move that gives check is extended by one ply inside the move loop, bounded so a chain of checks cannot run away, and decided by SPRT whatever it returns
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); **amended 2026-09-22 under DEC-228, before any game: the extension applies only to a checking move static exchange at threshold zero calls safe, restricted to the last `CheckExtMaxDepth` plies before the horizon, and the run is booked only against the bar DEC-228 states -- at most one ply lost on any of the three `search_bench` positions at `go nodes 1000000` and at most two in total, `bench` under +30 %, every fast-suite case inside its ceiling**; the extension applies inside the move loop to a move that gives check, never at the root and never past a stated depth cap, and the total extension along a line is bounded by the extension budget and depth guard S097 lands, so a sequence of checks cannot exceed the cap -- each condition asserted by a test in `tests/test_search.cpp` that fails when the precondition is removed; the late-move-reduction exemption for a checking move at `src/search.cpp` `is_check_move` stays as it is and the step states how the two interact; the mate cases in the fast suite pass -- `tests/test_search.cpp` "pruning does not hide a forced mate", "a side in check may not stand pat", "mate is recognised at depth zero" -- and the S145 mate sets are re-run with the found counts compared against the pre-change figures; every constant lives in `src/search_params.hpp` with a range and is seeded in a DEC-105 form (the wiki's one-ply form or the range midpoint), never from another engine's value; `MANUAL.md` and `DEV_MANUAL.md` checked; fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tools/mutants/, adocs/data/, MANUAL.md, DEV_MANUAL.md
excludes:   extensions not triggered by a check -- singular, negative, double -- which are S097 and later; a check extension before the move loop, the form Ethereal removed; any change to which moves the reduction exempts; the retired S096's id, which is not reused
decisions:  DEC-087, DEC-133, DEC-105, DEC-222, DEC-143
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-22 01:57, the machine idle and no match running
done:

## Why this exists

DEC-087 (a) retired S096, check extensions, on two records: "Ethereal removed
check extensions for +4.1/+4.5 and Stormphrax removed them too". The
2026-09-04 review's literature pass fetched both. Ethereal commit `3f4ef537`
(2018-06-25) is titled "Simplify and remove the pre-moveloop check extensions"
-- it removed the extension applied before the move loop, at bounds [-3, 1],
+4.14 +/- 4.15 and +4.54 +/- 4.13 -- and Ethereal's master `search.c` still
extends a checking move inside the move loop, which is where the wiki's
Ethereal page lists the technique. Stormphrax's commit #67 (2024-03-04)
removed check extensions outright and carries a bench and no Elo; Stormphrax
comes from a record far above the band. Of the hand-crafted engines the plan
reads as existence proofs, Weiss 1.2 at 3055 and Stash both carry the in-loop
form. Chesso has no extension of any kind; the reduction exempts a checking
move and nothing extends one.

DEC-133 is the owner's decision of 2026-09-04: the retirement stands by id,
and the in-loop form is a new step with one verdict, placed after S097 because
S097 builds the extension plumbing -- the depth guard and the budget that stop
extensions compounding -- and a check extension is the smallest thing that
plumbing carries. DEC-019 is why it is measured rather than argued either
way: the record says every band engine has it; only a verdict says whether
chesso wants it.

## The hazard

Pruning that hides a mate is this engine's recurring bug, and an extension has
the opposite failure: a chain of checks that extends without bound blows the
tree up at the horizon. The cap and the budget are the guard, and the accepts
requires each to be observed load-bearing -- remove it, watch the test go red
-- rather than assumed.

## Cost

A few lines of code and one verdict at the ledger's price, about five hours of
machine at DEC-063's pairs.

## Prior, recorded 2026-09-19 (DEC-222)

The 2026-09-19 study review (`adocs/audit/2026-09-19_study_review.md`, F11)
adds a third removal record and prices the run. Three engines have removed a
check extension: Ethereal's was the pre-move-loop form (above); Stormphrax's
message states neither form nor Elo; the open-source record removed one on
2025-04-17 at -0.26 +/- 1.49 over 58,032 games on a non-regression pair, its
message not stating the form, and its 2024 history had moved the extension
inside the loop and back before it within a fortnight. So the prior is small
or zero, and no record establishes the in-loop form measured at zero. The
pair stays `{0, 5}` as DEC-133 decided: a `{-5, 0}` pair, which the study
proposed, accepts H1 on a truth of zero and would ship an extension that does
nothing after the full walk. The pre-registration states the prior, the
worst-case walk of 41,861 games (DEC-143, about 20 hours), and an abort rule;
a verdict of zero is recorded as zero and the extension leaves the tree.


## FORM 1, the first written, kept as the instrument's finding (DEC-228)

What follows is the form measured first -- **every** move that gives check
extended -- and DEC-228 is the decision that its numbers, not a night of games,
re-formed the step. It is kept whole because the numbers are the evidence and
because the code, the cases and the mutants below are what the later forms were
built from. The forms measured after it are in "Three forms measured" at the
end of this file.

### What is written, what it measures, and the four reds it leaves

**From the description (DEC-221).** The technique reached this agent as the
step file's own prose above and as two pages of the Chess Programming Wiki,
fetched 2026-09-22: Check Extensions, which states "typical depth to extend is
one ply" (https://www.chessprogramming.org/Check_Extensions), and Extensions,
which warns that "care must be taken so that the search is not extended
infinitely" and states no factor, ratio or formula for the bound
(https://www.chessprogramming.org/Extensions). No other project's source was
opened. The one ply is DEC-105 **(a)**; the cap's number is **(c)**, the
midpoint of a range stated by purpose, and no (a) existed to take.

#### The rule, as coded

`src/search.cpp` `negamax_at` gains one node-level constant above the move
loop, `check_extension_window` -- `CHECK_EXTEND != 0`, `ply > 0`, and
`ply < CHECK_EXT_PLY_FACTOR * depth` -- and one term inside it: a move whose
`gives_check` is true is searched at `depth` instead of `depth - 1`.

**The budget is one ply and it is shared with S097** (`?:` and not a sum):
`(check_extension_window && gives_check) ? 1 : ((moves[i] == tt_move) ?
se_extension : 0)`. A move that is both the table's singular move and a
checking move is searched one ply deeper and not two, asserted at the site --
`assert(child_depth >= depth - 1 && child_depth <= depth)`, live in the Debug
build the second tier self-plays -- and pinned by a case at `SeMinDepth` rather
than argued.

**The two bounds the accepts names, and both are observed load-bearing.** The
cap is S097's own form with a number of its own, because the two rules extend
different amounts of the move list: S097 at most one move per node, this one
every checking move there is. It binds **downwards** -- a child sits at a
higher ply with a remaining depth no greater than its parent's -- so a node at
or past `CheckExtPlyFactor * depth` switches the rule off for its whole
subtree, which is what makes a chain of checks finite: an extended child keeps
its parent's remaining depth, so along a chain the depth never falls and only
the ply rises.

#### The checking-capture decision, and what it cost to keep the scan honest

**A capture that gives check is extended.** The step says "a move that gives
check"; a checking capture is the most forcing move on the board, and a rule
that skipped it would measure a different technique than the one the record
traces. `is_check_move` in the move loop is hardcoded false on a capture and
S107 left a comment warning against reusing it, so the rule reads a shared
scan instead: `capture_is_check` is the one `is_check(game)` call, taken where
**either** S091's two rules or this step's window wants it, and
`capture_gives_check` is that scan ANDed with S091's own two askers -- so which
moves the reduction exempts is **exactly** what it was, which the step's
excludes requires. The cost is one attack scan per capture at a node inside the
window, and it is paid by the candidate and priced by the SPRT with the rest of
the rule; at `CheckExtend` 0 the term folds away in the release build and the
scan is back to S091's set to the instruction.

#### Tests, and what each one holds

Eight cases in `tests/test_search.cpp`'s "search: pruning and reduction guards"
suite, six in both builds and two in the tune build alone, all on
`check_ext_drive_t` (a `guard_fixture_t`), all asserting their preconditions
**inside the drive** so that a case reading "nothing was extended" cannot pass
because some other term was false. Three positions, every property of each one
a tool's answer and not an author's (CHESS, DEC-023): python-chess on
2026-09-22 reports 15 legal moves and exactly one check and no capture at the
S097 fortress plus a white rook on b1; the same 15 with the one check and the
one capture being the same move at that position with a black rook on b8; and
28 legal moves of which 6 give check for a white queen against a bare king.

| case | what it holds |
|---|---|
| a checking move is extended and no other move is | no entry is planted, so `!se_verified` and every ply at this node is this rule's; all 15 legal moves searched; the one checking move a ply deeper and the other 14 not |
| a capture that gives check is extended too | the same, where the one check is the one capture, built through the generator as a capture and asserted to be one |
| every checking move at a node is extended, not only the first | six of twenty-eight, each named by python-chess and built through the generator |
| the root extends no checking move | ply 0 extends nothing, and the same drive at ply 1 does -- the control is in the case |
| no checking move is extended past the ply cap | `CheckExtPlyFactor * depth - 1` extends, `CheckExtPlyFactor * depth` does not, the boundary derived from the parameter so a refit moves the case with it |
| a move that is both singular and checking is extended one ply | at `SeMinDepth` with the entry's move being the checking move: `se_verified`, `se_extended`, and the child depth one above the node's and not two |
| the check extension does not run at its off value (**tune**) | the same drive at `CheckExtend` 1 and 0, with a destructor restoring 1 |
| a chain of checking moves stops extending at the cap (**tune**) | at the cap the subtree costs **exactly** what the same drive costs with the rule switched off, which is every node below it and not only the probed one; one ply lower the two differ |

**Why the chain case is a node-count identity and lives in the tune build.**
The probe records one node at one ply and no child is at that ply
(`src/search.hpp` `negamax_probed`), so no field of it can see a line twenty
plies down. A node **ceiling** was measured and rejected as the instrument: at
that drive the uncapped subtree costs 11982 nodes against 2226 capped, 5.4
times, and the ratio *falls* with depth -- 3.7 at depth 8, 2.7 at depth 10 --
because only the checking side's plies are extended, so a chain loses a ply of
depth every second ply and grows by a constant factor rather than without
bound. That is a fact about this rule worth having and no honest margin could
straddle it, so the claim is the exact one instead, which needs the rule
switched off under the same drive -- a variable in the tune build and a
compiled constant in the release one (DEC-118). X04 is killed in the release
build by the boundary case.

#### Mutants, and every red observed

`tools/mutants/S188_check_extension.py`, prefix **X**, ten mutants: one per
rule, one per gate, each with its killer named in the file's header. **All ten
were applied by hand, built and observed red, then reverted** -- the S033
protocol, because `tools/mutation_check.py` refuses a baseline that is not
green and this tree is not (below). The log is
`adocs/data/S188_form1_reds.log` and the driver that took it reads the anchors
out of the mutant file itself; the failing line and values of each are
recorded at the case that kills it. The full `mutation_check.py` pass over
`tools/mutants/` is **owed** and is the first thing to run once the four reds
below are decided, because E01 to E23 must be re-proved on a tree whose release
build compiles a different `child_depth` (S097's own precedent at its verdict
2) and because E01's anchor was re-pointed here.

| mutant | killed by | observed |
|---|---|---|
| X01 the ply is added to every move | the first two cases | `REQUIRE_EQ( 4, 3 )` |
| X02 no ply is ever added | all six | `REQUIRE_EQ( 3, 4 )` |
| X03 the root gate dropped | the root case | `REQUIRE_EQ( 4, 3 )` |
| X04 the ply cap dropped | the cap case | `REQUIRE_EQ( 4, 3 )` |
| X05 the rule reads `is_check_move`, false on a capture | the capture case | `REQUIRE_EQ( 3, 4 )` |
| X06 the shared scan loses the extension's own term | the capture case | `REQUIRE_EQ( 3, 4 )` |
| X07 the two rules add instead of sharing the ply | the singular case | `REQUIRE_EQ( 11, 10 )` |
| X08 the switch read the wrong way round | five cases | `REQUIRE_EQ( 3, 4 )` |
| X09 the node **is** in check instead of the move **giving** one | five cases | `REQUIRE_EQ( 3, 4 )` |
| X10 two plies instead of one | all six | `REQUIRE_EQ( 5, 4 )` |

#### Measured 2026-09-22, machine idle, load average 0.70 over 12 threads

| instrument | parent `f1869b4` | candidate |
|---|---|---|
| `chesso bench` | 4493659 | **8744373, +94.6 %** |
| `search_bench` depth 9 | 21479 / 102462 / 33148 | 54448 / 102373 / 36909, midgame's best move `g5f6` -> `b2b4` |
| `search_bench` depth 12 | 149688 / 459216 / 219544 | 170984 / 1041772 / 287412, no best move moves |
| `S097_fixed_node_depth.py` at `go nodes 1000000` | 17 / 13 / 15, total 45 | **15 / 11 / 14, total 40** |
| `S145_mined_set.py score --depth 10` | 146 exact, 148 right sign, 0 wrong sign | **189 exact, 203 right sign, 0 wrong sign** |
| `test_mate_breadth` wall time | 19 s | **212 s**, over the 120 s ceiling `tests/CMakeLists.txt` sets |
| off value: tune build at `CheckExtend` 0 | -- | 4493659 and **all eight `bestmove` replies identical** to a Release build of the parent |

**The explosion signature is present and is reported rather than explained
away** (the brief's own instruction, and S097 verdict 1's precedent at
17/13/17 -> 16/13/15): five plies of iteration depth over three positions at a
fixed budget. The cap is not what costs it --
`adocs/data/S188_cap_sweep.log` sweeps the whole range and reads `bench`
6883468 at 1 against 8744373 at the shipped 4, fixed-node totals 43 / 44 / 43 /
40 / 41 / 42 / 42 against 45 with the rule off -- so the seed sits at the worst
cell of both and is kept anyway, because it is the range's own midpoint and
depth at a fixed budget is not Elo (DEC-105 (c), DEC-019, S021's precedent).
Against it, the mined mate set moves further than it ever has: 43 more of this
project's own labelled mates found at the labelled distance, 0 wrong sign.

#### The four reds this landing leaves, and not one of them is mine to decide

Both builds are 37 of 40 with the **same** three tests failing and the same
four assertions. Every one of them is an existing test whose claim the
extension moves; none is a defect found in the rule, and the rule's own eight
cases are green in both builds.

1. **`tests/test_search.cpp` "the reported line runs the full depth"** --
   `warm.pv.length == depth`, 6 against 5 at depth 5 on the tactical FEN. Its
   own comment states the claim: "Outside of a mate the line is exactly as long
   as the search was deep." An extension makes a reported line **longer** than
   the nominal depth, which is a behaviour change `adocs/specs.md` has to carry
   and which the case cannot survive as an equality. The purpose the case
   exists for -- a table cutoff chopping the line short -- is `>=`.
2. **`tests/test_search.cpp` "pruning does not hide a forced mate"**, the row
   `mate the extra ply hides, depth 11`, S095's mined row. The candidate's
   profile over depths 3 to 14 on single cold searches is `2 2 2 2 - 2 2 2 - 2
   - 3` against the parent's every depth 3 to 12; over UCI `go depth N` **both**
   binaries answer mate 2 at every depth 3 to 14, so what moved is the
   fixed-depth cold search the row was mined on. The row's own GOLDEN block
   says it "moves legitimately on: any change to ordering, pruning or
   reduction" and names the scripts that re-mine it. The mined breadth set
   moving 146 -> 189 is the evidence that this is a fragile row moving and not
   mates being hidden; re-mining it is a machine job and a shared artifact.
3. **`tests/test_search.cpp` "search() hands the rule the root's own index"**
   (S207) -- `REQUIRE_LT(root_recurrence.score, -300)` reads 0. The case's
   cycle `h4d8 g8h7 d8h4 h7g8` is a sequence of checks, so the depth-4 drive
   now reaches the in-tree draw at ply 5 that the case is built to keep out of
   reach. The repetition rule is not what changed; the construction assumed no
   extension.
4. **`tests/test_engine.cpp` "a narrowed window still finds a mate that appears
   mid-search"** -- `first_mate_depth` is 9 for `r3r1k1/pp3pbp/1qp1b1p1/1BB5/
   3P4/Q1n2N2/P4PPP/3R1K1R b - - 5 18` and the candidate reports the mate at 8,
   so precondition 2 (the iteration below it scores in centipawns) fails. The
   constant is a measured input the case's own comment says was found by
   measurement; it has no re-derivation script, which is itself a DEC-142
   finding.

And **`test_mate_breadth` times out at 120 s** having passed its assertions
with room (189 against a floor of 143): 212 s against a ceiling the CMake
comment derives as "over six times the Release one".

None of these is weakened, edited or deleted here (AGENTS.md TESTS, and the
brief's own limit). What each needs -- an invariant reworded, a row re-mined, a
construction re-based, a measured constant re-derived, a timeout re-derived --
is a change to somebody else's test or to a shared artifact, and the last of
them is a cost the verdict may decide not to pay at all.

#### Proposed for `adocs/specs.md` (the coordinator writes it)

The search row's "absent, search" entry says "extensions other than the
singular one"; that stops being true. Beside S097's own passage:

> **Check extension.** At a non-root node whose remaining depth is at most
> `CheckExtMaxDepth` and whose ply has not reached `CheckExtPlyFactor * depth`,
> a move that gives check -- a capture that gives check included -- is searched
> one ply deeper, provided the move's static exchange at threshold zero holds on
> the position it is made from, so a check that hangs the checking piece is not
> extended. The ply is the node's one extension budget and is shared with the
> singular extension: a move both rules want is searched one ply deeper and not
> two, and at the shipped settings no node reaches both rules. `CheckExtend` is
> the switch and at 0 the engine is the one before S188, to the node.

*(Amended on form 3, DEC-228. The passage above replaces the one this section
first proposed, which described the ungated form.)*

And the sentence the first red above owes: a reported principal variation may
now run **past** the nominal depth of the iteration that produced it, because an
extended line is longer than the search that started it.

#### Follow-ups this step does not take

- **A SEE gate on the extension.** The wiki's Check Extensions page records
  that "some programmers don't extend checks (and captures) with negative SEE
  or even reduce them". That is a second rule with its own verdict (MEASUREMENT,
  one change at a time) and it is the obvious answer to the fixed-node table
  above if the SPRT reads H0 for a cost reason rather than a value one.
- **The cap as an axis.** S127 fits `CheckExtPlyFactor`; the sweep says the
  axis has something to find and says nothing about Elo.

## Three forms measured, 2026-09-22, against DEC-228's bar

DEC-228 re-formed the step before a game was played and stated the bar the run
is booked against: on the three `search_bench` positions at `go nodes 1000000`
the form loses **at most one ply on any position and at most two in total**
against the parent's 17 / 13 / 15, `bench` grows by **less than 30 %** over
4493659, and **every fast-suite case stays inside its ceiling**. All three
numbers below are the parent `f1869b4` against a candidate built from the same
tree, machine idle; the fixed-node depths and the bench totals are exact
(node-limited) and do not depend on the load.

| | form 1, every check | form 2, the safe-check gate | form 3, the gate near the horizon |
|---|---|---|---|
| the rule | every move that gives check | + `see_ge(board, move, 0)` on the parent board | + `depth <= CheckExtMaxDepth` |
| `bench` | 8744373, **+94.6 %** | 7748319, **+72.4 %** | **5756104, +28.1 %** |
| fixed-node depths | 15 / 11 / 14, total 40 | 17 / 12 / 15, total 44 | **16 / 13 / 15, total 44** |
| plies lost, worst / total | 2 / 5 | 1 / 1 | 1 / 1 |
| `test_mate_breadth` | 212 s, **over** its 120 s ceiling | 114.9 s | **109.2 s** (110.3 s tune) |
| fast suite | 37 / 40, four assertions | 37 / 40, five assertions | **38 / 40, three assertions** |
| mined mates at depth 10 | 189 exact / 203 sign | 180 / 190 | 180 / 190 |
| **bar** | missed on every clause | **missed on bench** | **met on all three** |

Parent figures for the last two rows: 146 exact / 148 right sign, 0 wrong sign,
and 19 s. Every form reads 0 wrong sign.

### Form 2, the safe-check gate: what it changed and what it did not

`see_ge(&game->board, moves[i], 0)` asked before `make_move` and inside the
window, so a node the rule cannot reach pays nothing and a capture the exchange
refuses no longer pays the check scan either. It cannot share an answer with
S091's call at the same threshold (that one is inside late move reduction's
eligibility, a different set of moves) or with S109's two (a margin scaled by
the reduced depth).

The gate bought **four plies of the five** form 1 lost at a fixed budget and
16 percentage points of the bench growth; it did not bring the bench inside the
bar. Swept over the ply factor (`adocs/data/S188_cap_sweep_gated.log`) the
growth reads +23.8 / +54.1 / +41.1 / +72.4 / +71.7 / +70.3 / +90.9 % at 1 to 7
with fixed-node totals 43 / 44 / 45 / 44 / 43 / 41 / 43 -- so under the gate the
ply factor **is** load-bearing on cost where under form 1 it was not (+53 % at
its tightest there), and the only cell inside the bench bar is the factor's
floor. The seed stays the range's own midpoint (DEC-105 (c)); re-stating the
range to put the midpoint on the cell that passes would be fitting the seed to
the bar, and S127 is where that axis is fitted.

Form 2 also moved a **fifth** case that form 1 left green:
`tests/test_mate_carry.cpp` "a mate score carried across searches keeps a line
that reaches it", cell `C_mate7_depth11`, 1 short mating PV against a recorded
ceiling of 0 (DEC-162). Form 3 leaves it green. Form 2's own reading of the
other four is form 1's, except that S095's mined row at depth 11 comes back and
S097 verdict 2's mined row at depth 14 goes red instead -- the two mined rows
trade places under the two trees, which is what "moves legitimately on any
change to ordering, pruning or reduction" means in practice.

### Form 3, the horizon restriction: the readings, and the one thing it breaks

`CheckExtMaxDepth` 8, **(c) the midpoint** of 1 to 16 stated by purpose in
`src/search_params.hpp`. Swept (`adocs/data/S188_horizon_sweep.log`), with the
ply factor held at its seed:

| `CheckExtMaxDepth` | 1 | 2 | 4 | 6 | 8 | 10 | 12 | 16 |
|---|---|---|---|---|---|---|---|---|
| `bench` | 4282248 | 4772731 | 4930329 | 6368966 | **5756104** | 6708619 | 7748314 | 7748319 |
| vs parent | **-4.7 %** | +6.2 % | +9.7 % | +41.7 % | **+28.1 %** | +49.3 % | +72.4 % | +72.4 % |
| fixed-node total | **46** | **46** | 43 | 41 | **44** | 43 | 43 | 44 |

At 16 the restriction stops binding and the total is form 2's **to the node**,
7748319, which is the ceiling's own purpose statement measured; at 12 it is
already within five nodes of it, 7748314. **The two cells at
the floor read a smaller tree at a fixed depth *and* a deeper search at a fixed
budget than the parent** -- 46 against 45, with `bench` below 4493659 at 1 --
which is the one shape in this whole step that costs nothing by either
instrument. It is a reading and not a verdict (DEC-019) and it is not the seed.

**What form 3 breaks, and it is this step's own case.** The check extension and
the singular extension can no longer meet: S097 wants `depth >= SeMinDepth` 10
and form 3 wants `depth <= CheckExtMaxDepth` 8, so at the shipped seeds **no
node can reach both rules**. "A move that is both singular and checking is
extended one ply" still passes -- the move is extended, by S097 alone -- which
makes it a case that passes for the wrong reason, exactly what the block's
discipline is written against. Under form 3 that case has to move to the tune
build and set one of the two parameters into range, and the site's
`assert(child_depth >= depth - 1 && child_depth <= depth)` stays as the only
thing holding the shared budget in the release build. Not done here: nothing
lands until the form is chosen.

### The three cases still red under form 3

The same three as form 1's first, third and fourth, unchanged in kind:
`tests/test_search.cpp` "the reported line runs the full depth" (an extension
makes a reported line longer than the nominal depth); `tests/test_search.cpp`
"search() hands the rule the root's own index", whose cycle
`h4d8 g8h7 d8h4 h7g8` is a sequence of checks, so a depth-4 drive now reaches
the ply-5 in-tree draw the construction keeps out of reach; and
`tests/test_engine.cpp` "a narrowed window still finds a mate that appears
mid-search", where `first_mate_depth` 9 reads 8. Both mate-row goldens and
`test_mate_carry` are green under form 3. Nothing is weakened, edited or
deleted in any of them.

### What the tree holds

**Form 3**, and the coordinator's reading of 2026-09-22 is that DEC-228 already
names it -- the one further form to try when the gate alone misses -- so no new
decision was needed: form 2 missed on bench alone and form 3 meets all three
clauses. `src/search_params.hpp` carries three settings with their seeds and
stated ranges, `src/search.cpp` carries the window with the horizon term and
the exchange gate, `MANUAL.md` carries three option rows and `golden_defaults`
63 rows. Form 2 is that tree without `depth <= CHECK_EXT_MAX_DEPTH` and its
X-macro row; form 1 is form 2 without the `see_ge` call.

The `CheckExtMaxDepth` 1 cell -- `bench` below the parent's and a fixed-node
total of 46 against 45 -- is recorded as a reading for S127's lane and is not
the seed.

## Landed on form 3, 2026-09-22: the rule, the repairs and the evidence

### The rule, as it now stands

`src/search.cpp` `negamax_at`, one node-level constant above the move loop and
two terms inside it:

    check_extension_window = CHECK_EXTEND != 0 && ply > 0 &&
                             depth <= CHECK_EXT_MAX_DEPTH &&
                             ply < CHECK_EXT_PLY_FACTOR * depth
    check_extension_safe   = check_extension_window &&
                             see_ge(&game->board, moves[i], 0)
    child_depth            = depth - 1 +
                             ((check_extension_safe && gives_check)
                                  ? 1
                                  : ((moves[i] == tt_move) ? se_extension : 0))

`check_extension_safe` is asked **before `make_move`**, where `see_ge` reads the
position the move is made from, and inside the window, so a node the rule
cannot reach pays no exchange evaluation. It cannot share an answer with S091's
call at the same threshold -- that one is inside late move reduction's own
eligibility, a different set of moves -- or with S109's two, which ask about a
margin scaled by the reduced depth. `gives_check` is the scan that was already
paid: `is_check_move` on a quiet, and on a capture the shared scan whose third
asker is this rule's own gate, so a capture the exchange already refused never
pays it and `capture_gives_check` keeps exactly the moves S091 gave it.

Three settings, every seed in a DEC-105 form and the form named at the site:
the **one ply** is (a), the wiki's own; `CheckExtPlyFactor` 4 is (c), the
midpoint of 1 to 7; `CheckExtMaxDepth` 8 is (c), the midpoint of 1 to 16.
`CheckExtend` is the switch, and its off value is proved on the tree.

### The three repairs, each a consequence and none a weakening

| case | what moved | what it still asserts |
|---|---|---|
| `tests/test_search.cpp` "the reported line runs the full depth" | `pv.length == depth` becomes `>= depth` | that a table cutoff did not chop the line short, which is the claim the case was written for. The equality was a statement about a tree with no extensions in it; `adocs/specs.md` gains the sentence that a reported line may now run past its iteration's nominal depth. Nothing bounds it above today and the comment says so: the check extension's cap bounds plies of search, not plies of line |
| `tests/test_search.cpp` "search() hands the rule the root's own index" | the drive depth 4 becomes `DRIVE_DEPTH` 3, and searches 2 and 3 with it | every assertion, unchanged: the root's own occurrence answers the material, one ply deeper the in-tree draw answers 0, and the same board with the cycle played pre-root answers 0 at the same depth. **The construction gained an argument it did not have**: the pair 1-and-3 at the same depth is what makes 1 non-vacuous, since 3 finds a draw where 1 does not, so 1's material score is the boundary classifying that occurrence and not a tree too short to have reached it |
| `tests/test_engine.cpp` "a narrowed window still finds a mate that appears mid-search" | `first_mate_depth` 9 becomes 8 on both positions | all three preconditions and the answer: the mate first appears above `AspirationMinDepth`, the iteration below it scores in centipawns, the schedule failed and re-searched, and every iteration from the first mate on reports the same distance |

Both depth constants are **measured** and both were bare measurements: the
first had never been re-derivable and the second's own comment said it "was
found by measurement" against a reference binary that no longer exists. That
absence was a DEC-142 finding and it is closed here by
`adocs/data/S188_repair_goldens.py`, two stages with a GOLDEN block at each
site naming the command:

- `s207` compiles `search_after()` of the case, line for line, against the
  engine library as it stands and sweeps depths 2 to 8, printing the three
  scores per row and the separating depth. **On this tree it separates at 3 and
  on the parent at 4**, which is the one-ply shift the extension caused and the
  evidence that the case was re-based and not relaxed.
- `first-mate` drives `deepen()`'s own `go depth 10` over both positions and
  prints every iteration's kind and value, the first mate's depth, the
  precondition below it and the set of distances from there on. Both positions
  read `d1..d7 cp`, `d8..d10 mate5`: `first_mate_depth` 8 and `mate_in` 5.

`tests/test_mate_carry.cpp`'s DEC-162 ceiling and both mined mate rows are
green on form 3 and were **not** touched; form 2 moved the first of those and
form 1 moved one of the mined rows.

### The budget case moved to the tune build, and what holds it in the release one

At the shipped seeds S097 wants `depth >= SeMinDepth` 10 and this rule wants
`depth <= CheckExtMaxDepth` 8: **the two ranges are disjoint, so no node in the
release build can reach both rules.** The case that reads the shared ply would
pass there because S097 extended the move on its own -- green for the wrong
reason -- so it is `CHESSO_TUNE`-only, raises `CheckExtMaxDepth` to the node's
own depth with a destructor restoring the shipped value, and asserts the window
is shut at 8 before it opens it. In the release build the budget is held by the
site's own `assert(child_depth >= depth - 1 && child_depth <= depth)`, live in
the Debug binary the second tier self-plays, and X07 is declared **equivalent**
for the release build with that arithmetic as the argument rather than a claim
about behaviour.

### Cases and mutants as they now stand

Ten cases in `tests/test_search.cpp`'s "search: pruning and reduction guards",
**seven in both builds and three in the tune build alone** (the off value, the
chain identity and the budget) -- 7 in the release binary and 10 in the tune
one, counted off `--list-test-cases` and not off the file. The two form 3 added: "a checking move the
exchange calls unsafe is not extended", the gate's direct guard case (DEC-141),
driven at a white queen and king against a bare king where python-chess reports
21 legal moves, 5 checks and no capture, and the engine's own `see_ge` refuses
two of the five -- both halves at one node, with the verdicts asserted on the
parent board before the drive reads their consequence; and "no checking move is
extended above the horizon restriction", the depth cap's boundary pair at
`CheckExtMaxDepth` and one above it.

`tools/mutants/S188_check_extension.py` carries **twelve** mutants (X01 to
X12): the two new ones drop the exchange gate and the horizon restriction, so
each of DEC-228's three forms is one conjunction away from the one before it and
a case reads the difference at a node. Every anchor occurs exactly once as the
formatter leaves the lines, re-checked after each form.

## E21 survived the pass, and the re-mine that answers it

`tools/mutation_check.py` over 34 mutants on the fixture (below) scored
**`E21_multicut_mate_band_gate_dropped` as a survivor**: `fast 0/40`, the whole
suite green, with its bench signature moved. E21 is S097's guard on the value
the multicut hands back, and at S097 verdict 2's landing it was killed by the
mined row `mate_the_multicut_hides` at depth 14 -- the row that file's own
GOLDEN block says "moves legitimately on: any change to ordering, pruning or
reduction". This step's extension is such a change and it has unpinned that
guard.

**Not parked as a finding**: DEC-142's rule is that a mined row is re-derived by
its script whenever either end moves, and this tree moved it. The coordinator's
reading of 2026-09-22, on the owner's standing priority of strength and
correctness over machine time, is to re-mine before the report.

**The estimate, stated before the run** (RUNS): stage 1 is skipped, because
`adocs/data/S097_candidates.tsv` is committed and its FEN list survived in the
coordinator's scratch, so no Stockfish labelling pass is needed. What remains
is two sweeps of 269 candidates over depths 11 to 14 -- the range the script
derives from `SeMinDepth` + 1, unmoved by this step -- run concurrently as the
script's header instructs, the shipped library against an out-of-tree build
with E21 applied; then `fires` over the separators on the tune library, and
`pick`. **About 45 to 60 minutes**, well inside DEC-155's four hours, and the
machine holds nothing else.

**That estimate was wrong and the measured figure is 73 minutes a sweep.** It
came from extrapolating the single row of the paragraph below -- 2.4 s over its
four cells -- and that row is cheaper than the candidate set's average: 198 of
the 269 report a mate at all four depths and the depth-14 cell is where the
nodes are. Two sweeps and the witness are **about two and a half hours**,
still inside DEC-155's four, and the second sweep is budgeted from the first's
measured 73 minutes rather than from an extrapolation again. `src/search.cpp` is restored from a pristine copy
taken before the mutation is applied and never with `git checkout --`: this
tree is uncommitted and that command would discard the step.

### Where the separation went, measured before the re-mine

`fast 0/40` from the pass is the first half of the answer: with E21 applied the
**whole** suite is green, so nothing else in `tests/` holds that guard either.
The second half is the row itself, swept at depths 11 to 14 on this tree by
`adocs/data/S097_mine_mate_row.py sweep` against the shipped library and
against one built out of tree with E21 applied -- `distance:nodes:ms` per cell,
and `src/search.cpp` restored from a pristine copy afterwards, never with
`git checkout --`:

| depth | shipped (form 3) | guard dropped |
|---|---|---|
| 11 | `-` 518327 | `-` 518327 |
| 12 | m5 1961277 | m5 1961277 |
| 13 | `-` 2615072 | `-` 2615072 |
| 14 | **m5** 11878601 | **m5** 12370187 |

The row's shipped profile is what it was, `d12 d14`, and the case that reads it
at depth 14 is green. **What moved is the other build**: the guard-dropped
profile was `d12` at S097's landing and is now `d12 d14`, so the row no longer
separates the two. The rule is still live on the position -- the depth-14 node
counts differ by 4.1 % -- so this is not a position the multicut stopped
reaching; it is one where a deeper tree finds the mate anyway, with or without
the guard. That is the whole of the regression, and it is a property of the
tree and not of the guard.

### The re-mine, and the row it produced

Stages 2 to 6 of `adocs/data/S097_mine_mate_row.py` on this tree, stage 1
skipped because `adocs/data/S097_candidates.tsv` is committed and the pool is
not what moved. **Two of the 269 candidates separate the shipped build from the
guard-dropped one**, and the script's own rule took one and skipped the other:

| candidate | shipped | guard dropped | the rule fires | depth-14 cell |
|---|---|---|---|---|
| `r7/p5Rk/2p1p2p/2B1PP2/3P3n/P1P4q/1r3P2/2RQ2K1 b - - 0 35` | d11 d12 d13 d14 | d11 d12 d13 | **never** | 43758845 nodes, 5.4 s |
| `1R6/8/2p3p1/P5P1/1p2b2P/4k3/6pK/8 b - - 1 54` | d11 d12 d13 d14 | d11 d12 d13 | d13, d14 | 4227541 nodes, 0.56 s |

The first is **skipped** and that is the witness stage earning its keep: the
tune build at `SeMultiCut` 0 against 1 gives the same tree on it at every
depth, so its depth-14 separation cannot have come from this rule and a red
there would have meant something else. The second is taken, at depth 14 and
mate in 5.

**It is a better row than the one it replaces** on the property the tie-break
is for: its shipped profile is the whole swept range, `d11 d12 d13 d14`, where
the old row's was `d12 d14` with a gap, so three other depths have to go before
it can fall silent again; and it costs 4227541 nodes and 0.56 s against
4206525 and 0.65 s. The oracle is not chesso (CHESS): stockfish through
python-chess in a fresh process answers `#+5` with `Kf2 Rf8+ Bf5 Rxf5+ gxf5
Kh3 g1=Q a6 Qh1#`, and the committed candidates file carries the same label
from the pool's own run. The red was observed by hand under E21 and reverted,
and `src/search.cpp` came back from a pristine copy rather than from
`git checkout --`, which in this uncommitted tree would have discarded the
step. Evidence: `adocs/data/S188_remine.log`.

### The mutation evidence, both runs

**The pass**, `adocs/data/S188_mutation_pass.tsv`, fixture `504a43b`, baseline
green over 40 tests, 34 mutants, wall 9488 s: **31 killed of 32 scored, 97 %**.

- **`X07_budget_is_a_sum` -- equivalent, and the tool's own second oracle says
  so**: `bench same`. The declaration is arithmetic and not a claim about
  behaviour -- `SeMinDepth` 10 and `CheckExtMaxDepth` 8 leave no node inside
  both extension rules, so a sum and a `?:` compile to the same tree. It is
  killed in the tune build by the budget case and in Debug by the site's
  assertion.
- **`X10_extends_two_plies` -- unmeasured, and it is the tool being strict
  rather than a gap**: its four failing rows are `test_search(Timeout)`,
  `test_engine(Timeout)`, `test_mate_breadth(Timeout)` and
  `test_mate_pv(Timeout)`. Two plies a check blows the tree up and four tests
  hit their ceilings; DEC-165 is why a Timeout alone is not scored as a kill.
  The same mutant was observed red **by assertion** under the step's first
  form, `REQUIRE_EQ( 5, 4 )` in `adocs/data/S188_form1_reds.log`.
- **`E21_multicut_mate_band_gate_dropped` -- survived**, which the section
  above answers.

**The targeted re-runs**, `adocs/data/S188_mutation_targeted.tsv`, fixtures
`a6e0475` and `799c42d`: **6 of 6 killed**. Every mutant here is one that a
**mined row** is supposed to take, which is what makes the set the right one to
re-prove on a tree whose search reaches further.

| mutant | killed by |
|---|---|
| `E21` | "pruning does not hide a forced mate", the re-mined row |
| `R01_extra_reduction_gives_check` | "a capture that gives check is not reduced" -- S091's own case, re-proved because this step rewrites the expression that rule reaches through |
| `R02_extra_reduction_sign` | "pruning does not hide a forced mate" among others, so S091's `capture_mates` rows still take it |
| `J01`, `J02` | their own cases **and** "pruning does not hide a forced mate", so S095's mined row still does its job |
| `J03` | its own case alone |

**J03 is where a second row narrowed.** `tests/test_search.cpp` said "J03 also
takes 'pruning does not hide a forced mate' with it" and on this tree it does
not -- the same motion that made S097's row go quiet, one step smaller. No
coverage is lost, because that case is J03's own killer and always was its
first, so the sentence is corrected against the run rather than left standing
as a claim the tree no longer supports. S095's row is not re-mined: it is not a
survivor and the rule for re-mining is a row that stopped separating its own
mutant, which this is not.

## What the fast check moved, 2026-09-22

The Tier-1 check over the diff confirmed `src/` sound -- the child-depth
budget, the exemption set's identity by algebra, the chain bound, the board
`see_ge` is asked on, the twelve anchors, and that no extended move can be
reduced -- and returned five findings of record. What each one changed:

1. **`test_mate_breadth`'s headroom, and it is thin.** This landing recorded
   **109.2 s** Release (110.3 s tune) against the 120 s
   `CHESSO_MATE_BREADTH_TIMEOUT`; the reviewer measured **116.82 s** at load
   0.79 on the same tree. **2.6 % of headroom is inside this project's own
   noise floor**, which `bench_movegen` has printed at 0.1 % on an idle
   machine and 2.0 % on a busy one (CLAUDE.md). The ceiling is **not** touched
   -- that is the relaxation TESTS forbids -- and the gap is named as an open
   test-side finding in `adocs/data/S188_sprt.sh` under DEC-171: it cannot move
   a reported score, move or line. If the verdict keeps the rule, a step
   re-derives the ceiling by the rule the CMake comment states for it or
   restructures the case; if the rule leaves, the case is back at 19 s and the
   finding closes with it.
2. `adocs/data/README.md`'s row for the pre-registration described the
   **rejected first form** -- no exchange gate, no horizon term, 15/11/14 at
   +94.6 %. Rewritten to what the script reads, and the row says it was
   corrected.
3. **X10 is now killed by assertion on this tree, by hand** (S033): with it
   applied, `test_search --test-case="a checking move is extended and no other
   move is"` fails at `REQUIRE_EQ( record.child_depth[k], depth )`, `values:
   REQUIRE_EQ( 5, 4 )`, in seconds -- `require_extended_exactly` reads the
   child depth directly, where the whole binary times out. Reverted from the
   pristine copy. The mutant file's note carries it and the tool's
   `unmeasured` row is left exactly as the tool wrote it.
4. **"a check that hangs the checking piece is not extended" was wrong on a
   discovered check**: `see_ge` is asked on the **moved** piece's destination,
   which on a discovered check is not the piece giving check. `MANUAL.md`'s
   `CheckExtend` row carries the accurate clause.
5. **R02 was the one other mined-row mutant the pass missed** -- it pins
   `capture_mates`' four depths -- and it is re-run targeted on the fixture
   beside the other five.

Four trivial ones fixed with them: `tests/test_engine.cpp`'s prose still said
"eight iterations and then a mate at depth 9" against the 8 its own golden now
holds; this file said the horizon restriction stops binding "at 12 and above"
where the sweep reads 7748314 at 12 and form 2's 7748319 to the node at 16;
the re-mine's GOLDEN block named the committed candidates file without the
`grep -v '^#' ... | cut -f1` that derives the FEN list the script's `--fens`
actually wants; and `DEV_MANUAL.md` wrote the guard-dropped profile as "d12 d13
plus d11" where the log reads `d11 d12 d13`.
