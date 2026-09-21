id:         S097
goal:       extend the one move a verification search says is singular, and take the multicut the same search offers
accepts:    an SPRT verdict per change, measured separately -- the extension and the multicut are two changes off one verification search; the verification search excludes the table move, runs at a reduced depth against a window below the table score, and is skipped at the root and where the entry is too shallow or its bound is wrong, each condition asserted by a test that fails if the precondition is absent; the margins and the reduced depth are constants in src/search_params.hpp with stated ranges (S073); a position with a forced mate inside the multicut's pruned depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; the fast suite green
touches:    src/search.cpp negamax and negamax_at, src/search.hpp, src/search_params.hpp, src/data_structures.hpp search_node_probe_t, tests/test_search.cpp, tests/test_search_params.cpp, tools/mutants/, adocs/data/, MANUAL.md, DEV_MANUAL.md
excludes:   check extensions, which are retired outright by DEC-087 (a) and have no successor step -- Ethereal and Stormphrax both removed them for a gain, `src/search.cpp` `negamax` already exempts a checking move from the reduction, and the forcing-line concern is this step; any extension not derived from the verification search
decisions:  DEC-071, DEC-105, DEC-134
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-20 19:10 while S095's gainer SPRT holds the machine, so the code, the tests, the mutants and the two pre-registrations are written first and every measurement comes after
done:

## Implemented 2026-09-20: what is written, and what decides it

Written while S095's gainer SPRT held all twelve threads, so everything in this
section is code, tests, mutants and pre-registration. **Nothing in it has been
compiled, run or measured**; every number the step owes is taken after the
machine comes free and is stamped in the completion note. **From the
description (DEC-221)**: the technique reached this agent as prose -- the
sections below, written from publications and from the open-source record's
published measurements -- and was implemented from that description. No other
project's source was opened.

### The shape: two verdicts, and the second ships inert

The accepts prices **two verdicts off one verification search**, and the shape
that makes them attributable is the coordinator's brief and is taken as
written. V1 is one commit-ready change: the `excluded_move` plumbing, the
verification search, the five gates on a node searched under an exclusion, the
extension, four seeded constants, the cases and the mutants. **V2's code ships
with it, inert**, behind `SE_MULTICUT` at its off value, range 0 to 1, so that
V2's landing is that switch moved to 1 and nothing else in `src/`. (The clause
read "default 0" until verdict 2 landed; the wording changed and the claim did
not, because `tools/plan_prose_check.py --params` reads a default stated in a
pending step file against the one the engine compiles and the sentence would
otherwise have been a gate failure describing the past correctly. The same
edit is made at the proposed specs passage below and nowhere else.)

One deviation from the brief's "one default flip plus its pre-registration",
argued here rather than done quietly. **The multicut's direct guard case, the
accepts' mate row and its three mutants land with the flip and not with V1.**
`SE_MULTICUT` is `inline constexpr int` in the release build -- the build the
gate runs and the only one `tools/mutation_check.py` compiles -- so at 0 the
compiler folds the branch away entirely: a case cannot drive a rule that is not
in the binary, and a mutant of one is equivalent by construction and would be
scored as a survivor. What V1 can hold, and does, is the other half of DEC-215:
a case that establishes **every** condition of the multicut at a drive and
asserts the node searched its moves anyway (`tests/test_search.cpp` "the
multicut does not fire at its off value").

### The verification search and its gates, as coded

`src/search.cpp` `negamax_at` gains a trailing `move_t excluded_move`, defaulted
to 0 on the two entry points `src/search.hpp` `negamax` and `src/search.hpp`
`negamax_probed` and on nothing else -- the house pattern S098's `cut_node`
set. Every recursion hands its children 0: the exclusion is a property of the
question being asked at one node, not of the position.

The block sits **between the null-move block and the move loop** in
`src/search.cpp` `negamax_at`, so a node the pass already cut off never pays for
a verification. Its conditions, in the order they are written:

| condition | what it is for |
|---|---|
| `SE_EXTEND != 0` | the switch DEC-215 clause 2 requires, added after the fast check; at 0 the whole block is skipped and the tree is the parent's |
| `ply > 0` | the root has to produce a move and its entry is the previous iteration's own answer |
| `excluded_move == 0` | no verification inside a verification -- the published rule |
| `tt_move != 0 && tt_entry != nullptr` | there is a move to call singular and an entry to measure it against; the entry is tested in its own right because at ply 0 the root hint can make the first true with the second false |
| `depth >= SE_MIN_DEPTH` | the node is worth the nodes |
| `tt_entry->depth >= depth - SE_TT_DEPTH_MARGIN` | the entry was searched deeply enough for its move to be worth verifying |
| `type is TT_BETA_NODE or TT_PV_NODE` | a lower or exact bound; an upper bound is a ceiling and a margin cannot be subtracted from one |
| `ply < SE_PLY_FACTOR * depth` | the explosion cap |
| `abs(tt_score) < MATE_MIN` | a mate score is a distance, not a value |
| `singular_beta > -MATE_MIN` | and a score one point outside the band still lands inside it once the margin comes off |

`tt_score` is `de_normalize_score(entry->score, ply)` -- this is a **second
reader** of that field beside `src/search.cpp` `tt_entry_answers`, and S106's
round trip applies to it. `singular_beta` is `tt_score - SE_MARGIN_PER_DEPTH *
depth`. The verification runs at `(depth - 1) / 2` with the window
`(singular_beta - 1, singular_beta)` and `excluded_move = tt_move`, through
`negamax_at<false>` -- never the probing instantiation, because this is the one
search in the engine at **its own node's ply** and the probing one would
overwrite the record of the node that asked the question. It is labelled
`is_pv = false, cut_node = false`: a zero window under the entry's score is
asked expecting everything to fail low, which is CPW's ALL node.

`state->aborted` is checked immediately, before anything reads `vscore`, and
`state->pv_length[ply]` is cleared after the verification returns -- the search
wrote this ply's row for a node that was searched without one of its moves, and
every path out of the function has to leave the row valid or empty.

**Two side effects are inherent and are named rather than guarded**: the
verification is a real search, so its cutoffs write killers at this same ply and
grade the history and continuation tables, and the parent then orders its own
moves under what the verification learned. That is what the published technique
does and what the SPRT prices; the alternative -- saving and restoring the
tables around it -- is a second change with its own verdict and is not made
here. Its nodes are counted in `state->explored_nodes` like any others, which is
what makes a node budget and the time check bound it.

**Five things a node searched under an exclusion does not do**, each one
comparison, each with its own case and its own mutant:

1. **no table cutoff** -- the entry was written by a search that was allowed to
   play the move being set aside, so it would answer the verification with the
   thing being verified; an entry whose lower bound already clears the window
   would fail it high every time, vacuously. **The probe still runs and
   `tt_move` is still copied out**: the step file's section 2 says "gate the
   probe and the store", its section 5 and section 7 say the probe still finds
   the entry, and the brief says not to mask `tt_move`. The sections win --
   masking it would switch S095's rule on and off inside the verification for
   no stated reason.
2. **no store** -- what the node computed is the value of a position with one of
   its moves removed, under the key of the position with the move in it.
3. **no null move** -- a null-move bound answers the verification with no
   alternative searched, which is the one thing that search exists to do. Gated
   on the exclusion directly and never by abusing `prev_move`, which has to keep
   flowing for the countermove and continuation tables.
4. **no reverse futility** -- **this step's own decision, and the step file
   asks for one**: "decide with a test, record which". A static bound can only
   fail the node high, so inside a verification it can only ever say "not
   singular", and it says it with no move searched, at a depth whose smaller
   margin makes a fail-high easier than the parent's own reverse futility
   already found it. Under the multicut it would return a static margin as the
   node's value -- a second pruning rule folded inside the first, which nothing
   could attribute. The case is `tests/test_search.cpp` "an excluded node runs
   no reverse futility" and the mutant is E11.
5. **no verification of its own** -- no recursive exclusion.

**S109's pruning stays live inside the verification** (section 5): the published
record treats the biased quiet tail as a feature and the SPRT prices it. **IIR
(S095) stays off inside it by construction** and nothing is owed (section 7).

### The extension

`src/search.cpp` `negamax_at`'s `child_depth` is now
`depth - 1 + (moves[i] == tt_move ? se_extension : 0)`, and `se_extension` is 1
only where the verification came back below its window. Folding it into that one
line is what makes the late-move-reduction clamp, S098 verdict 3's re-search and
all four recursion sites follow the extended depth -- the edit S098's file
assigns to this step. One ply, once per node, and no move but the table's can
match because `se_extension` is 0 wherever `tt_move` is.

### The one case the step file leaves open, and how it is answered

**A node whose only legal move was the excluded one.** Section 2 says the mate
side reads as fail-low and is correct, and that the stalemate side has no traced
prose: "decide and pin with a test". Neither terminal score is returned. Both
are claims about a position nobody is in -- the move that answers is on the
board and this search set it aside -- and `DRAW_SCORE` is the dangerous one: to
a verification whose window sits below zero it reads as a **fail-high**, which
under the multicut returns 0 as the node's value on the strength of a stalemate
that does not exist. What is true is that nothing reached the window, so the node
returns `alpha0`, fails low, and the table move is singular in the only sense the
rule means -- it is the one legal move there is. Pinned by
`tests/test_search.cpp` "a node whose only move is excluded is neither mated nor
stalemated", on two tool-built positions, and by mutant E15.

### Constants, seeds and the off value DEC-215 asks for

Six rows in `src/search_params.hpp`'s X-macro, documented there in full and in
`MANUAL.md`'s option table. `SE_MIN_DEPTH` 10 (4..16), `SE_TT_DEPTH_MARGIN` 4
(0..8), `SE_PLY_FACTOR` 5 (2..8), `SE_MARGIN_PER_DEPTH` 9 (1..18) -- **all four
DEC-105 (c), the midpoint of a range stated by purpose**, and no engine's
shipped depth, margin or ply count seeds any of them wherever it is republished
(DEC-084 as amended by DEC-105, DEC-134). The **(b)** alternative section 4
offers for `SE_MIN_DEPTH` -- profiling how often this engine's own stored move
survives a full-width search at the same depth -- was available and **was not
taken**: it is a new instrument and a machine job, the range's own midpoint is
what the four terms beside it took, and S127 refits all four. The verification
depth is **not** a parameter: `(depth - 1) / 2` is written at the site, which is
the choice section 4 leaves to this step.

`SE_EXTEND` 1 (0..1) and `SE_MULTICUT` 0 (0..1) are switches and not settings,
one per verdict, and both off values are proved on the tree rather than
declared: the tune build at `SeExtend` 0 benches the parent's total and at
`SeMultiCut` 1 it benches something else than V1's. `SeExtend` arrived after
the coordinator's fast check and the section below records why.

**Section 4's off-value sentence is wrong and is kept with the correction here,
not amended in place** (DEC-215 clause 3). It says "the feature's off switch is
`SE_MIN_DEPTH` at its own range top". At 16 the rule still fires at every depth
above 16, which a game at 8+0.08 reaches; the range top is not an off value, for
exactly the reason S098 verdict 3's margin was not one -- and the measured
table below shows how convincingly it can look like one, benching the parent's
total exactly because `bench` stops at depth 14. `SeExtend` is what the rule
ships behind instead, and clause 2 is what asks for it.

### Tests, and what each one holds

Fourteen cases in both builds and a fifteenth in the tune build alone, all in
`tests/test_search.cpp`'s "search: pruning and reduction guards" suite, which is where the null-move, reverse-futility, reduction and
shallow-depth guards already live and whose `guard_fixture_t` and `quiet_move`
they reuse. Every drive of the block runs at the blocked-pawn fortress the
reverse-futility depth case already uses -- five legal moves, no captures
anywhere below it -- which is what makes a node at `SeMinDepth` affordable in a
suite the gate runs, and every one asserts its preconditions **inside the
drive** so that a case saying "the block did not fire" cannot pass because some
other condition was false.

The probe (`src/data_structures.hpp` `search_node_probe_t`) gains six fields --
`se_verified`, `se_extended`, `se_multicut`, `se_singular_beta`, `se_vscore`,
`se_vdepth` -- and a `child_depth` per searched move. They cost the shipping
binary nothing: S191's `PROBING` parameter means `negamax_at<false>` holds no
probe code at all.

| case | what it holds | mutants |
|---|---|---|
| "the extension lands on the table move and on no other" | the window is the entry's score less the margin, the verification ran at half depth, it came back below the window, and exactly one searched move -- the table's -- has a child depth a ply above the node's | E01, E03, E13, E14 |
| "a verification that fails high extends nothing" | the inverse, with the precondition asserted | E02 |
| "the multicut does not fire at its off value" | every condition of the rule holds and the node searches anyway | none: see the shape above |
| "the root never runs a verification search" | ply 0 does not, ply 1 with the same plant does | E04 |
| "an entry shallower than the depth margin verifies nothing" | one ply below the margin does not, the margin itself does | E05 |
| "an upper-bound entry verifies nothing" | `TT_ALPHA_NODE` does not; both admitted types do | E06 |
| "a mate score never becomes a verification window" | a mate-band entry does not verify, **and** one point outside the band whose derived window lands inside it does not either | E16, E17 |
| "a node searched under an exclusion starts no verification of its own" | and the excluded move is absent from what the node searched | E07 |
| "an excluded node takes no table cutoff" | the control returns the planted score with no move searched; the same node under an exclusion searches | E08 |
| "an excluded node writes no entry for the position" | nothing in the table afterwards, where the control does store | E09 |
| "an excluded node makes no null move" | the control passes, the exclusion does not | E10 |
| "an excluded node runs no reverse futility" | the control takes the cutoff, the exclusion does not | E11 |
| "the excluded move is the one move the node does not search" | the exclusion unit test through the `negamax` seam, on a mate in one with exactly one mating move: excluded, no mate score comes back; any other move excluded, it does; and the probe shows every legal move but one searched | E12 |
| "a node whose only move is excluded is neither mated nor stalemated" | `alpha0`, on an in-check position and a stalemate-shaped one | E15 |
| "the extension does not run at its off value" (**tune build only**) | the same drive verifies and extends at `SeExtend` 1 and does neither at 0, with every child depth back at `depth - 1` | E18, which the release build's extension case kills |

**Chess judgement comes from tools throughout** (CLAUDE.md, DEC-023). The
mate-in-one is `7k/6pp/8/8/8/8/8/R6K w - - 0 1`, already the third row of
`tests/test_search.cpp` "mate in one": python-chess reports 16 legal moves and
exactly one giving checkmate, and -- enumerated rather than reasoned about --
after **every** one of those 16 moves Black has no capture at all and is in
check only after `a1a8`, which is why the drive is one ply deep and why "no mate
is left" is a property of the position rather than a hope about the tree. The
one-legal-move positions are `7k/8/6K1/8/8/8/8/7R b - - 0 1` (in check, one
escape) and `k7/8/8/8/8/8/8/KQ6 b - - 0 1` (not in check, one move), both from
python-chess. Stockfish confirmation of the mate row is owed at Report 2.

### Mutants

`tools/mutants/S097_singular_extension.py`, prefix **E**, eighteen mutants:
one per rule, one per gate, one for the switch, each with a named killer
above. Every anchor was
checked to occur exactly once in `src/search.cpp` as the formatter leaves it.
E20 to E22 are **named and reserved** in the file's header for the multicut and
land with the flip, with the reason stated there.

### The two pre-registrations

`adocs/data/S097_v1_sprt.sh` and `adocs/data/S097_v2_sprt.sh`, both `{0, 5}`
nElo at the S105 regime, both refusing to run until `REF` and `CAND` are pinned
(DEC-020), both with `OUT` under `.tuning/`. They carry DEC-143's cost for the
pair -- 41861 expected games at the midpoint and 25591 on a bound, 19.8 h and
12.1 h at 2110 an hour, so a night each (DEC-155) -- the abort rule, the three
open test-side findings, and the outcomes. Two readings are written in advance
because they are the ones a day's enthusiasm would get wrong:

- **if V1 reads H0**, the recommendation is to drop both and record the zero.
  The multicut needs the verification search the extension pays for, so an H0
  prices that search at zero and the multicut would have to buy it alone; its
  traced +5.7 and +6.2 are from engines three hundred points above this one and
  one engine dropped it at this band. The exception stated with it: a walk near
  the bounds rather than a measured loss leaves the question open.
- **if V2 stalls near +3** it is terminated, recorded as a zero and dropped
  (DEC-063), which is the same reading as H0 -- said in advance so that a long
  run is not read as encouragement.

`adocs/data/S097_fixed_node_depth.py` is the step's own instrument: the depth
reached at `go nodes 1000000` over `tools/search_bench.py`'s three positions.
Section 6 asks for it per verdict, and it is the only measurement before the
games that can see an over-firing extension -- `bench` and `search_bench` both
hold the depth fixed, which is exactly what this rule trades.

### Proposed for `adocs/specs.md`, once V1 lands (the coordinator writes it)

**Two places move, and the second is easy to miss.** The "absent, search" row
names "extensions of any kind", which stops being true at this landing --
singular extensions leave that row exactly as reverse futility left it at S033
and the four shallow-depth rules at S109, and the row's own convention is to
say when and under which step. The paragraph below it that reads "Singular
extensions are S097" is the second. The new passage:

> **Singular extension.** At a non-root node with no exclusion active, whose
> transposition entry carries a move, was searched to at least
> `depth - SeTtDepthMargin`, certifies a lower or exact bound and holds a score
> outside the mate band, the node is searched again at `(depth - 1) / 2` against
> the window `(SeMarginPerDepth * depth)` below that score, with the entry's
> move excluded, and never at a node whose ply has reached
> `SePlyFactor * depth`. If every alternative fails below that window the
> entry's move is searched one ply deeper -- one ply, once per node, and never
> recursively. A node searched under an exclusion takes no table
> cutoff, writes no entry, makes no null move, runs no reverse futility and
> starts no verification of its own; a node whose only legal move was excluded
> returns its own alpha rather than a mate or a draw. `SeMinDepth`,
> `SeTtDepthMargin`, `SePlyFactor` and `SeMarginPerDepth` are the settings;
> `SeMultiCut` ships at its off value, so the verification never returns a
> score of its own.

**Superseded by "Verdict 2 landing" below**, which carries the passage the
coordinator writes once the multicut's default moves. The last clause above
named the switch and its shipped number where it now names the off value;
only the wording moved, for the gate reason given at the top of this section,
and the number it used to carry is the one verdict 2 changes.

### Proposed decision

One choice here is worth a `DEC-` entry because a future reader would re-derive
it and could reasonably decide the other way: **reverse futility is suppressed
at an excluded node, and the terminal scores are not returned there.** Both are
the same judgement -- what a verification search may be answered by -- and both
are this project's own where the record is silent. The coordinator owns the
wording; the argument is in the two sections above and each half is pinned by a
case and a mutant.

## Measured 2026-09-21, machine idle, on the tree S095's H1 left

S095's gainer SPRT read H1, so the tree V1 lands on is `5c76ea9` and its `src/`
is `3961c13`'s -- the fifth reduction term stays. Every number below was taken
on this workstation with nothing else running, against a linked worktree built
at that commit.

### The gate

Both fast suites green, 40 of 40 each: Release 121.4 s, tune 123.5 s, and
`./clang-format.sh --check` clean at `CLANG_FORMAT_MAJOR=22`. The new cases
run in **16 ms together** -- the fortress drive is five king moves, no
capture below it, and the repetition rule collapses the rest, so a node at
`SeMinDepth` costs a test nothing.

**One case was red when the coordinator's gate run compiled this tree before I
did, and it is fixed here rather than reported and left.** "the excluded move is
the one move the node does not search" counted the moves the node searched with
an *ordinary* move excluded: the mating move is then still on the list, it
clears beta, the loop breaks on it and the count measures where the ordering put
the mate -- `REQUIRE_EQ( 1, 15 )`. With the mating move excluded instead,
nothing in the position beats an alpha of `MATE_MIN`, every move fails low and
the count is exact. The case now excludes the mating move in that drive and says
why in its own comment.

### The bench ledger's number, and the off value proved on the tree

| tree | `bench` | against the parent |
|---|---|---|
| parent `5c76ea9` | **4579468** | -- |
| candidate, V1 | **5066204** | +10.63 % |
| tune build, `SeMultiCut` 0 | **5066204** | the candidate's total **to the node** |
| tune build, `SeMultiCut` 1 | 4493659 | -11.3 % on the candidate: the multicut is live and prunes |

The off value is proved and not declared (DEC-215), and with the **full bench
signature**: all eight `bestmove` replies of the tune build at `SeMultiCut` 0
are identical to the Release candidate's
(`.tuning/coord/S097_bench_release.log`, `.tuning/coord/S097_bench_tune0.log`).
The 1 row is what verdict 2 will be measured on and it is here to show the
switch is not inert.

### What the tree does at a fixed depth

`tools/search_bench.py`, parent -> candidate:

| position | depth 9 | depth 12 |
|---|---|---|
| midgame | 21479 -> 21479, `g5f6` | 154423 -> 154388, `c3d5` |
| kiwipete | 102462 -> 102462, `e2a6` | 578047 -> 459115, **`d5e6` -> `e2a6`** |
| tactical | 33148 -> 33148, `d7c8q` | 173394 -> 239314, `d7c8q` |

**Depth 9 is byte-identical on all three positions**, which is the depth gate
measured rather than argued: `SeMinDepth` is 10 and a depth-9 iteration has no
node deep enough to verify anything. At depth 12 two positions move in opposite
directions and one best move moves, which is the ordinary signature of a search
that reorders as well as deepens.

### The node-explosion check, and it is the number to argue about

`adocs/data/S097_fixed_node_depth.py`, `go nodes 1000000`, Hash 16
(`.tuning/coord/S097_fixed_node_depth.log`):

| position | parent depth | candidate depth | best move |
|---|---|---|---|
| midgame | 17 | **16** | `c3d5` both |
| kiwipete | 13 | 13 | `d5e6` -> **`e2a6`** |
| tactical | 17 | **15** | `d7c8q` both |
| total | 47 | 44 | |

**Two of three positions lose depth at a fixed budget, one by two plies.** That
is the trade the technique makes and it is the cost side of it, stated before
the games: the extension has to buy back a ply of tactical resolution with
better lines, and this is what it is paying. It is **not** the explosion
signature as section 6 defines it -- that is a fall on every position -- and
kiwipete holds its depth while changing its move, which is the shape the rule is
for. Whether the trade is worth anything is `adocs/data/S097_v1_sprt.sh`'s to
say and not this table's (DEC-019).

### Where the seeds sit, measured across their own ranges

Not asked for by the step and taken because the depth loss above deserves
context: `bench` on the tune build at five settings of each of the two axes that
decide how often the rule fires (`.tuning/coord/S097_seed_ablation.log`).

| `SeMarginPerDepth` | 1 | 5 | **9** | 14 | 18 |
|---|---|---|---|---|---|
| bench | 6639177 | 5965342 | **5066204** | 4633274 | 4636973 |

| `SeMinDepth` | 4 | 8 | **10** | 13 | 16 |
|---|---|---|---|---|---|
| bench | 6081929 | 5089613 | **5066204** | 4424094 | 4579468 |

Two readings, and the second is a correction to this file.

**The margin is the axis that decides the cost.** At the range floor the tree is
45 % larger than the parent's and at the top it is 1 % larger: the seed at 9
sits mid-slope, which is where a midpoint should sit and is the region S127's
lane will move in.

**`SeMinDepth` at its range top benches the parent's total exactly -- 4579468 --
and that is the trap DEC-215 exists for, caught here rather than assumed either
way.** Section 4 called the range top the feature's off switch; this table looks
like it agrees and does not. `bench` searches to depth 14, so at `SeMinDepth` 16
the rule cannot fire on *these positions* and the equality is an artefact of the
instrument. A game at 8+0.08 reaches depth 16 and beyond, where the same setting
still extends. **Off on the bench is not off in play**, so the correction above
stands: V1 has no off value inside its ranges and its H0 path is a revert.

### Goldens, re-derived by their scripts (DEC-142)

- `tests/test_search_params.cpp` `golden_defaults`: **51 -> 57 rows**, the six
  this step adds -- four settings and two switches. No script; `src/search_params.hpp` is the derivation and the
  diff of the two is the re-derivation.
- `tests/test_mate_carry.cpp` `short_line_ceiling`: the grid was re-taken on
  this tree (`.tuning/coord/S097_sweep_block.txt`, 108 cells) and the ceiling
  rule run over all five recorded grids --
  **5, 15, 0, 2, 11, 5, unchanged**, so no ceiling moves and no decision is
  owed. This tree's own grid is **better** than the two before it at every
  case: 0, 8, 0, 1, 4, 3 against S095's 0, 15, 0, 0, 11, 0 and against the
  shipped ceilings. A line the search stored deeper is a line the walk can
  certify, which is the same mechanism S109 and S095 recorded pushing the other
  way.
- `tests/test_search.cpp` `capture_mates`: the four rows' shipped profiles
  re-derived by `adocs/data/S230_mine_r01_row.py depths` over
  `adocs/data/S230_table_fens.txt`
  (`.tuning/coord/S097_capture_mates_shipped.txt`): rows 1, 2 and 4 report their
  mate at depths 9 to 12 and row 3 at 10 to 12, so **every pinned depth -- 9, 9,
  10, 10 -- is still in its row's profile** and no depth moves. The mutant half
  of those labels is the mutation run below.

### The oracle, S033's protocol (`.tuning/coord/S097_oracle.log`)

Every position the new cases introduce, confirmed by stockfish at depth 20
through python-chess as well as by python-chess's own enumeration:
`7k/6pp/8/8/8/8/8/R6K w` is **`#+1` in 320 nodes, pv a1a8**, with exactly one
mating move of sixteen legal ones; `7k/8/6K1/8/8/8/8/7R b` is `#-2` with one
legal move `h8g8`, lost and **not mated**; `k7/8/8/8/8/8/8/KQ6 b` is `#-7` with
one legal move `a8a7`, lost and **not stalemate** -- which is what makes a
returned `DRAW_SCORE` there a wrong answer rather than a harmless one.

### Twelve anchors of four older mutant files were re-pointed

`tools/mutation_check.py` validates **every** mutant in the directory before it
runs any of them, and it refused this step's run outright: gating the table
cutoff, reverse futility and the null move on `excluded_move == 0` grew three
conditions that twelve older mutants anchor on, and two recursion sites gained
a trailing argument that two more name. D08 (S098), H03 (S222), N02 to N06
(S191) and M02, M03, M06a, M06b, M11 (`search.py`) were re-pointed at the lines
as they now read; **every mutation is unchanged** -- each still removes exactly
the guard its note names -- and each file records what moved and why.

Two of them were anchored on more text than they needed: H03 named a whole
two-line call to mutate one argument of it, and it had already been re-pointed
once at S098 verdict 2 for the same reason. Both now anchor the argument line
alone. That the tool refuses the run rather than skipping a stale mutant is
what made this visible in one command instead of in a silently smaller
mutation score.

### The mutation runs: 23 of 23, then 13 of 13

`tools/mutation_check.py`, 3084 s over a linked worktree, **23 of 23 killed,
mutation score 100 %** (`.tuning/coord/S097_mutation.log`, per-mutant rows in
`.tuning/coord/S097_mutants/results.tsv`, the killer per mutant in
`.tuning/coord/S097_killers.txt`). The run covers this step's seventeen and
**S091's six as well** -- C02, C05, C06, C07, R01 and R02 -- because a step
that moves the search moves what the `capture_mates` rows separate, and all
six are still killed at their own depths, so that golden's mutant half stands
with its depths.

Thirteen of the fourteen new cases are a named killer of at least one mutant.
The fourteenth is "the multicut does not fire at its off value", which has no
mutant by construction and says so.

| mutant | killed by |
|---|---|
| E01 extension on every move | the extension lands on the table move and on no other |
| E02 condition inverted | that case, and "a verification that fails high extends nothing" |
| E03 extension dropped | that case, and `test_mate_carry` |
| E04 root gate | the root never runs a verification search |
| E05 entry-depth margin | an entry shallower than the depth margin verifies nothing |
| E06 bound type widened | an upper-bound entry verifies nothing, **and "pruning does not hide a forced mate"** |
| E07 recursion gate | a node searched under an exclusion starts no verification of its own, and `test_mate_carry` |
| E08 table cutoff gate | an excluded node takes no table cutoff |
| E09 store gate | an excluded node writes no entry for the position |
| E10 null-move gate | an excluded node makes no null move |
| E11 reverse-futility gate | an excluded node runs no reverse futility |
| E12 loop skip | four cases, including the exclusion unit test |
| E13 verification depth | the extension lands on the table move and on no other |
| E14 singular beta sign | that case, the mate-window case, and `test_mate_carry` |
| E15 no-legal-alternative return | a node whose only move is excluded is neither mated nor stalemated |
| E16 entry mate band | a mate score never becomes a verification window |
| E17 window mate band | the same case's second leg |

**The twelve re-pointed older mutants were re-run and all twelve are killed**
(`.tuning/coord/S097_mutation2.log`, 1758 s, 13 of 13 with E18), each by the
case its own note names: N02 by "a node at the positive edge of the mate band
makes no null move", N03 by "reverse futility does not fire at a node in
check", N04 by its PV case, N05 by its depth-bound case, N06 by "reverse
futility does not fire inside the mate band" plus `test_engine` and
`test_mate_breadth`, H03 by "the node after a null move has no previous move
to index", M02, M03, M06a and M06b by their own null-move and ply-floor cases,
M11 by "the reported line runs the full depth" with `test_mate_carry`, and D08
by "pruning does not hide a forced mate" with its re-search case. **N03 and N05
moved no bench signature and were killed anyway**, which is the same point the
four below make. Each mutant file now records the result rather than the
intention.

**E18, the switch read the wrong way round, is killed by seven cases at once**
-- every drive of the block, because at the shipped default the feature would
then be off. That is the property the switch has to have for an H0 to mean
anything.

**Four of the seventeen left the bench signature unmoved** -- E07, E09, E15 and E17 --
and the suite killed all four anyway. That is the half of DEC-141 clause 2 the
signature cannot do: a gate that only fires inside a verification, or on the
one node whose alternatives were all excluded, changes no bench total and
changes the search where it matters. Without the direct cases those four would
be proved gaps.

### The fast check found a defect in the gate, and it is fixed here

**The three entry fields the gate reads were read after the null-move search
had recursed.** `tt_entry` is the slot's own address -- the copy-out three
hundred lines above says so, and copies `tt_move` and the stored evaluation for
exactly that reason -- and the block sits below a search that can store into
that slot. A node in the null subtree whose key lands on the same index would
have left `singular_beta` derived from another position's score while the
extension was applied to this entry's move. Nothing crashes, no node count is
wrong, and the symptom is rating.

It is masked at the shipped seeds, which is not a reason to leave it: the null
subtree's stores stay below `depth - SeTtDepthMargin` at the shipped 4, and at
8 -- inside the range S127 sweeps -- or at a smaller null reduction they do
not. **Fixed by the house pattern**: `tt_entry_depth`, `tt_entry_type` and
`tt_entry_score` are copied out beside `tt_move` and `tt_eval`, the score
de-normalised there where the entry is still known to be the one the probe
found, and the gate reads the copies. The reason is written at the copy-out.

**No direct case, and here is why.** The hazard needs another position's entry
to land on this position's slot during the null search, which is a Zobrist
**index collision** and not something a drive can arrange: the same key cannot
recur in that subtree at all, because a null move flips the side to move
permanently, so nothing but a collision can overwrite the slot. Constructing
one means mining a pair of positions that collide at a chosen table size and
that the null subtree actually visits and stores -- a step of its own, and a
test that plants it would be pinning the table's hash rather than this rule. A
mutant that reads the fields directly again is **not** written for the mirror
reason: it is equivalent at the shipped seeds and is caught by nothing, so it
would be a survivor that proves only what this paragraph already says. What the
tree has instead is the copy-out, its comment, and the fact that the gate names
locals a reviewer can see are copies.

### The extension's own off value, DEC-215 clause 2

`SeExtend`, 0 or 1, default 1, gating the whole block. The four settings have
no off value between them and `SeMinDepth`'s range top is not one -- it benches
the parent only because `bench` stops at depth 14 -- so the rule ships behind a
switch whose range end really is off. **Proved on the tree with the full
signature**: the tune build at `SeExtend` 0 prints **4579468**, the parent's
total to the node, with all eight `bestmove` replies identical to a build of
`5c76ea9` itself (`.tuning/coord/S097_bench_extend_off.log` against
`.tuning/coord/S097_bench_parent.log`). At the seed the bench is 5066204,
unmoved by the switch's arrival.

That makes V1's H0 path a **default flip** and not a whole-commit revert, and
`adocs/data/S097_v1_sprt.sh` says so in its outcome text. The switch is pinned
by a case and a mutant: `tests/test_search.cpp` "the extension does not run at
its off value" is `CHESSO_TUNE`-only, because a compiled constant cannot be
moved by a case in the build that folds it away -- the tune build is in the
gate for exactly this class of difference (DEC-118) -- and the mutant is E18,
the switch read the wrong way round, which the release build's own extension
case kills. Dropping the condition instead is equivalent at the shipped 1 and
is declared in the mutant file rather than written.

### What is left, and whose it is

Nothing on this agent's side. DEC-141's second tier -- the Debug self-play and
`tools/gate_extra.sh` -- is the coordinator's on the landing commit, and both
SPRTs are the coordinator's to pin and launch. `adocs/data/S097_v1_sprt.sh`
already carries `REF` pinned to `5c76ea9` and every measurement above; `CAND`
is pinned after the landing.


## The most expensive item in the block

It searches the node twice at some nodes and it is the one technique here whose
cost shows up as a nodes-per-second loss before any Elo appears. It also
depends on the table entry carrying a usable depth and bound, so it sits after
S094. It is kept in the plan because it is the one search feature that is on
every engine in the 3000-plus hand-crafted band and absent here.

Reported figures put it at 30 to 60 Elo. DEC-019: that decides that it is
tried, and the SPRT decides what is kept.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol if drifted. Every GitHub read
below was a PR body, commit message, release note or README rating table,
never a diff or source file (DEC-016, DEC-084). CCRL's own pages refused this
pass (ccrl.chessdom.com DNS-dead, computerchess.org.uk 403), so rating bands
lean on repo-recorded numbers and the engines' own records, marked where soft.

### 1. State of the art

**Origin, acknowledged.** Anantharaman, Campbell and Hsu, "Singular
Extensions: Adding Selectivity to Brute-Force Searching" — AAAI Spring
Symposium 1988, ICCA Journal 11(4), Artificial Intelligence 43(1) 1990 —
from ChipTest/Deep Thought: extend the move that is much better than every
alternative, detected by searching the alternatives against a null window
lowered by a margin. Follow-ups: Anantharaman's two 1991 ICCA papers; Hsu,
*Behind Deep Blue* (2002). The modern restriction CPW dates to Stockfish 1.6
(2009): only the **TT move** is ever tested — "restricted to moves found in
the TT with a lower bound flag set", later relaxed to exact bounds too.

**The verification-search form (published prose).** At a non-root node with
no exclusion already active: a TT move whose entry is deep enough —
`entry depth >= depth - 3` is the traced form (edwardyu's 2011 talkchess
writeup of the SF-inspired implementation: "depth - nBetaDepth <= 3") — with
a lower-or-exact bound, node depth at or above a threshold (SF prose: "depth
threshold 6 unless this is a PvNode... to 8", 8e823459; Cardoso 2018 quotes
"depth>8*ONE_PLY"), and a TT score that is not terminal (Weiss #756 prose:
allow SE "as long as the score from TT isn't a terminal score"). Then
`singularBeta = ttScore - margin(depth)` — the margin **scales with depth**
(SF 23cbb221, "Depth dependant singular extension margin"; flat margins
10..100 are CPW's recorded experimentation space) — and the node is
re-searched at about half depth (CPW: "depth/2" up to "depth-N"), zero window
`(singularBeta-1, singularBeta)`, **with the TT move excluded**. Every
alternative failing low means the TT move is singular: search it one ply
deeper. Extension amount 1 at every traced introduction.

**Multicut fold-in.** The same verification failing HIGH means an alternative
also clears a bar just under the TT score — Björnsson/Marsland's multi-cut
signal (CG 1998; Information Sciences 122, 2000; TCS 252, 2001: M candidate
moves, C cutoffs, reduced depth R) collapsed onto the search SE already paid
for. Ethereal d397a7b5 (2019-12): "When a singular search shows a move
besides the tt move fails high we assume it's safe to prune. ELO | 5.74
+- 4.14". Lynx: returning the verification's own fail-soft score measured
**+6.21 +/-3.46** LTC (#1751 "multicut with singularScore", merged) where
returning singularBeta itself failed (#1750, -0.84 STC, unmerged) — return
the score, not the bound — with a mate-range guard on the returned value
(#1761, merged). Stash's first SE attempt carried multicut and the retry
dropped it: "Second try to Singular Extensions, no Multi-Cut this time"
(ad8a87c6, 2020-12) — dropped at roughly the band chesso is heading for.

**Negative extension.** Verification says not-singular and the TT score
already sits at or above beta (or at/below alpha — engines differ): reduce
the TT move instead. SF prose traces the churn (e1f12aa 2022 intro,
39da50e/98dafda/a48573e 2023, c43425b 2024 simplifies one away); Ethereal
f71ac476 "+4.66" (2023, ~3400); Stash #171 +2.66/+2.05 (2024); Weiss #655
+2.66/+3.14 (2023), #781 +3.25/+3.55 (2026); Lynx #1743 **+11.99 +/-5.03**
LTC at its introduction release.

**Introduction records, banded honestly.**
- Weiss #354 (2020-09-26): **+11.52 +/-6.84** at 10+0.1, **+24.21 +/-9.83**
  at 60+0.6; shipped in v1.2 (2020-10-17 release note "Singular Extension"),
  the release DEC-087 records at 3055 CCRL Blitz — introduced just below it.
- Berserk #42 (2021-04-07): **+19.82 +/-9.34** at 10+0.1, five weeks after
  the author estimated v2.0.0 at ~2450 (CCRL board prose) — the one clearly
  sub-3000 introduction; two days later e06b444 "Disable TT and NMP on
  singular search".
- Stash ad8a87c6/2db36e4f (2020-12 / 2021-02): lands across the v27 3000
  crossing DEC-087 records; later Stash: min depth 8→7 **+7.87/+4.97**
  (dfa889e7, 2022), "do more" +2.50/+6.27 (2023).
- Ethereal aa36bc09 (2018-04, ~10.x, low-3100s): three stacked patches —
  basic SE +6.23, don't stack check extension on it +3.46, threshold 8→10
  **+12.68**.
- Lynx #1731 (merged 2025-06-08, band **3224-3291**, S181):
  **+20.53 +/-8.48** LTC, branch named
  "se-04-7-no-tt-cutoffs"; v1.10.0 shipped SE plus double extension (#1742
  +18.33, margin 15), negative extension (#1743 +11.99), multicut (#1751
  +6.21) and a `ply < 3 * depth` cap (#1768) in one release.
- The old record: Komodo's Dailey, "one of the really big elo gains"; Hyatt
  measured no gain in Crafty (talkchess t=38104) — DEC-019's oldest case.

**The honest split.** The extension: introductions cluster from ~2500 to the
3000-3100 boundary — in-band for this plan, and plan.md's premise ("the one
feature every 3000-plus engine has") is what the record shows. Multicut:
+5.7/+6.2 at 3300-class engines, dropped by Stash at ~3000 — thin at the
band, hence the second verdict, and a zero is a live outcome. 3100+
refinements, not this step's: negative extensions, double/triple extensions
(Berserk #102 +2.90, #542 triple +6.73; Weiss #656 +11.52/+11.18; SF
16566a8f couples TT-capture LMR to it), the quiet-limit cost control
(Ethereal 464fa339 **+10.82**), ttPv singularBeta bonus (Lynx #2331 +4.95 —
needs a PV flag bit tt_entry_t does not carry, S098's ttPv wall).

**Band caution, resolved 2026-09-11 by S181: the README was right.** This
paragraph flagged on 2026-08-19 that Lynx's own README rates v1.8.0 at **3144**
and v1.10.0 at **3293** CCRL Blitz where S098's trace and DEC-087 placed that
era at high-2800s / "~2850", and that if the README were right, DEC-087's
Lynx-based banding read about 300 high. It did. The CCRL Blitz list computed
2026-09-05 and read 2026-09-11 gives **v1.8.0 3138, v1.9.0 3224, v1.10.0
3291** on the 1CPU rows, agreeing with the README to within a few points
(`adocs/data/S181_lynx_bands.md`). So **Lynx's singular-extension block is 3224-3291 evidence, not
sub-3000** — #1731, #1743 and #1751 all merged between v1.9.1 and v1.10.0 —
and the split above says so. The reading this step's case rests on is
unchanged, because the sub-3000 introduction it names is Berserk #42 at ~2450
and the band evidence is Weiss, Berserk and Stash; what moved is that the Lynx
block no longer supports it at this engine's band. S099's "+11.4 at ~2850" is
3224-3291 by the same table, which is `2026-09-04_plan_review-F02` and DEC-133's
reason for moving S099 to the reserve head.

### 2. Shape for chesso

- **negamax is one function and has no excluded-move plumbing** — verified:
  signature `(alpha0, beta, depth, ply, game, state, prev_move, is_pv)`
  (`src/search.hpp` `negamax`, `src/search.cpp` `negamax`); no per-ply search
  stack, only per-ply arrays in search_state_t (`src/data_structures.hpp`
  `search_state_t`). Published shape is a per-ply excludedMove (SF prose
  d6bdcec5 "(ss+1)-> excludedMove ... reset right after singular search is
  finished"); chesso's natural form is one more parameter, `move_t
  excluded_move`, travelling exactly as prev_move does. negamax is
  test-callable since S103.
- - **No extensions of any kind exist** (specs.md "absent, search"; S096
  retired by DEC-087), so this is the tree's first depth increase. Plumbing:
  `child_depth = depth - 1` (`src/search.cpp` `negamax`); the singular move
  alone searches at `child_depth + 1`. MAX_PLY walls carry it: negamax returns
  evaluate() at `ply + 1 >= MAX_PLY` (`src/search.cpp` `negamax`), quiescence
  stand-pat at `src/search.cpp` `quiescence`; MAX_PLY 128, MAX_DEPTH 126
  (`data_structures.hpp` `MAX_PLY` and `data_structures.hpp` `MAX_DEPTH`). An
  always-extending path terminates on the ply wall by construction; the
  published cap that keeps the wall theoretical is Lynx's `ply < 3 * depth`
  (#1768).
- - **TT entry fields suffice** — verified at `src/data_structures.hpp`
  `tt_entry_t`: `depth` int16_t, `type` uint8_t (TT_BETA_NODE = lower,
  TT_PV_NODE = exact, `src/data_structures.hpp` "the same all-or-nothing gate a
  table move is (DEC-122)"), `score` int32_t, `best_move`; probe and `tt_move`
  copy-out are both in `src/search.cpp` `negamax`. No change to the 24-byte
  layout is needed for V1/V2.
- - **The score read is a new de-normalize site.** entry->score is stored
  normalized (`src/search.cpp` `negamax`); SE bypasses tt_entry_answers (it
  wants the value, not a cutoff), so it calls `de_normalize_score(entry->score,
  ply)`
  (`src/search.cpp` `de_normalize_score`) itself, then requires `|tt_score| < MATE_MIN` before
  deriving singularBeta — S106's round-trip lesson applied at the new reader.
- - **TT while excluding, published practice:** at the excluded node take no TT
  cutoff (Berserk e06b444 "Disable TT and NMP on singular search"; Lynx's
  merged branch literally named "no-tt-cutoffs") and **write no store** (SF
  ebe021f6, "Don't update TT at excluded move ply") — gate the probe and the
  store in `src/search.cpp` `negamax` on `excluded_move == 0`. The subtree
  below probes and stores normally in every traced writeup. The alternative —
  hashing the exclusion into the key so the verification owns a separate entry
  — is known practice whose open prose this pass could not trace: **unknown,
  not taken** (DEC-084 caution). Weiss #600 (SMP probe rule) and #644 (skip TB)
  are out of scope here.
- - **Also suppressed at the excluded node:** NMP (same Berserk prose — a
  null-move bound would answer the verification with no alternative searched;
  gate it on excluded_move directly, never by abusing the `prev_move != 0` gate
  at `src/search.cpp` `negamax`, since prev_move must keep flowing for
  countermoves/S024) and SE itself (`excluded_move == 0` in the conditions — no
  recursive exclusion, the published rule). Whether RFP (`src/search.cpp`
  `negamax`) needs suppressing too has no traced prose: a static fail-high
  there answers "not singular" without any move searched — decide with a test,
  record which.
- - **The move loop under exclusion:** skip `moves[i] == excluded_move` before
  make_move and before legal_moves_counter++ (`src/search.cpp` `negamax`).
  score_move still ranks the excluded move first — one wasted pick, harmless.
  If no legal alternative exists, `src/search.cpp` `negamax` returns mate/draw:
  the mate side reads as fail-low (singular — correct, it is the only legal
  move); the stalemate DRAW_SCORE side reads as fail-high when singularBeta <=
  0 — no traced prose, decide and pin with a test.

### 3. Implementation sketch

Two verdicts off one verification search, as the accepts prices: the
extension first, the multicut second, each its own SPRT.

**V1 — verification search + extension:**
1. Thread `excluded_move` (default 0) through negamax; the three gates above
   (no TT cutoff, no store, no NMP, no SE while excluding).
2. Between the NMP block and move generation (a null-pruned node then never
   pays for verification): when `ply > 0 && excluded_move == 0 &&
   depth >= SE_MIN_DEPTH && tt_move != 0 && tt_entry != nullptr &&
   tt_entry->depth >= depth - SE_TT_DEPTH_MARGIN && (type is BETA or PV) &&
   |tt_score| < MATE_MIN && ply < SE_PLY_FACTOR * depth`:
   `singular_beta = tt_score - se_margin(depth)`;
   `vscore = negamax(singular_beta - 1, singular_beta, (depth - 1) / 2, ply,
   game, state, prev_move, false)` with excluded_move = tt_move; check
   `state->aborted` before using vscore (`src/search.cpp` `negamax`'s pattern).
   `vscore < singular_beta` → `se_extension = 1`.
3. 3. In the loop: the searched depth for `moves[i] == tt_move` becomes
   `child_depth + se_extension`; the LMR clamp (`src/search.cpp` `negamax`) and
   S098-V3's deeper cap follow the extended child depth — the edit S098 §5
   assigns here.
4. Tests, red first: **exclusion unit test through the negamax seam** — a
   tool-built mate-in-1 with exactly one mating move (python-chess enumeration
   + Stockfish confirmation, S033 protocol, DEC-023): with excluded_move = the
   mating move negamax must not return a mate score; with excluded_move =
   another legal move and with 0 it must — excludes exactly the given move.
   **Condition tests per the accepts**, each by planting entries with
   tt_store_entry (public) and failing if the precondition is absent: root
   never verifies; an entry shallower than `depth - SE_TT_DEPTH_MARGIN` never
   does; a TT_ALPHA_NODE bound never does — observed via node counts moving
   only when the condition holds. Both mate cases re-run -- "pruning does not
   hide a forced mate", `tests/test_search.cpp` "pruning does not hide a forced
   mate" and "pruning does not hide a mate against the material leader",
   `tests/test_search.cpp` "pruning does not hide a mate against the material
   leader"; fast suite. SPRT.

**V2 — multicut:**
1. 1. `vscore >= singular_beta && vscore >= beta && |vscore| < MATE_MIN &&
   !is_pv` → return vscore. Fail-soft score, not singularBeta (Lynx #1751 vs
   #1750); mate guard is #1761's; the !is_pv gate is the house pattern for
   bound-returning prunes, RFP and NMP in `src/search.cpp` `negamax` —
   direct prose untraced, stated as a choice in the commit.
2. The accepts' mate case: a forced mate inside the multicut's pruned depth
   added beside "pruning does not hide a forced mate", `tests/test_search.cpp`
   "pruning does not hide a forced mate", observed red with the mate-range
   guard removed.
3. SPRT; a random walk near +3 is terminated and recorded as zero (DEC-063),
   and dropping the multicut while keeping the extension is the default at
   zero (S005/S006/S015 precedent).

### 4. Constants and seeds

All in the `CHESSO_SEARCH_PARAMS` X-macro in `src/search_params.hpp` with
stated ranges; every number is a **seed — must be fitted/SPSA'd here**
(S127). Under DEC-105 each is one of three forms and says which: **(a)** a
value from a publication about the technique, with its URL; **(b)** a
derivation over chesso's own data or scale; **(c)** the range midpoint or off
value, stated as such. Where a midpoint is not an integer this step takes the
integer below it and says so. No engine's shipped depth, margin or ply count
seeds anything here, wherever it is republished; those records are in section
1 and, as anti-seeds, in section 5.

**Units, once for this file (P6).** A margin compared against `evaluate()` is
in chesso's material scale, `piece_value` in `src/eval_tables.hpp`: `PAWN`
94, `KNIGHT` 327, `BISHOP` 308, `ROOK` 487, `QUEEN` 716. The header's own
comment says the split between `piece_value` and `psqt_mg` / `psqt_eg` is
degenerate, so the material term alone is the unit — a pawn measured by
removing one from a board is not 94. Plies have no unit at all.

- `SE_MIN_DEPTH` **10**, range 4..16 — **(c) midpoint.** No (a) exists to
  take: the wiki's Singular Extensions page states no depth
  (https://www.chessprogramming.org/Singular_Extensions, fetched
  2026-09-05), and the Anantharaman, Campbell and Hsu papers behind the
  technique are paywalled and their parameters **unverified**
  (https://dl.acm.org/doi/abs/10.1016/0004-3702(90)90073-9,
  https://journals.sagepub.com/doi/abs/10.3233/ICG-1988-11402). The **(b)**
  alternative belongs to this step at its start and is stated here so it can
  be taken instead: profile chesso's own table-entry reliability by depth —
  over the 300-position stratified pick (`adocs/data/S021_aspiration_sweep.py`),
  how often the stored move of an entry at depth `d` survives as best move of
  a full-width search at the same depth — and seed at the shallowest depth
  where that share is high enough for a verification search to be worth its
  nodes. Whichever is used, the stamp records it.
- `SE_TT_DEPTH_MARGIN` **4**, range 0..8 — **(c) midpoint**, exactly.
- `SE_PLY_FACTOR` **5**, range 2..8 — **(c) midpoint**, exactly.
- `SE_MARGIN_PER_DEPTH` **9** — **(c) midpoint** of a range stated by
  purpose, which is what `src/search_params.hpp`'s header asks a bound to be.
  Form `margin = SE_MARGIN_PER_DEPTH * depth`, compared against `evaluate()`,
  so the units above apply. Floor **1**: the smallest margin that is not off.
  Top **18**: the value at which the margin at the `SE_MIN_DEPTH` seed
  reaches two pawns, `2 * PAWN / SE_MIN_DEPTH` = `2 * 94 / 10` = 18.8,
  floored — above that the verification window is wider than the material a
  singular move is being claimed to win. Midpoint of 1..18 is 9.5, so the
  seed is **9**. **Off value: none inside this range**, and that is stated
  rather than fudged -- at the top the margin is still only two pawns at
  `SE_MIN_DEPTH` and the extension keeps firing. The feature's off switch is
  `SE_MIN_DEPTH` at its own range top, and if the sweep wants a margin that
  parks the term it needs a wider declared range and a purpose for it.
  The "flat 10..100 experimentation range" the 2026-08-19 pass attributed to
  the wiki **is not on the page as fetched 2026-09-05** and is dropped.
- Verification depth: `(depth - 1) / 2` as the shipped form. The
  "depth/2 to depth-N" space the 2026-08-19 pass attributed to the wiki is
  **unverified** — not on the page as fetched 2026-09-05 — so the form is
  chesso's own choice, **(b)**: if parameterised, `SE_VDEPTH_SUB` with the
  halving stated, else record the fixed form in the commit.
- Multicut adds no constant beyond the guards; a return margin is S127-era.

seeds re-derived 2026-09-04 under DEC-105 (DEC-134)

### 5. Pitfalls

- **Anti-seeds — records, not seeds.** Records of a value that measured
  negative elsewhere; DEC-019 lets them say which direction is worth trying
  and DEC-105 forbids any of them starting a sweep, which is why section 4
  names no engine. Returning singularBeta from multicut (Lynx #1750 failed
  where #1751 passed); stacking a check extension on a singular one (SF
  30c58320 prose; moot here — none exist, DEC-087); extending more than 1
  without the double-extension guards (Lynx caps doubles at 6 per line,
  #1777). The min-depth direction is engine-dependent and section 1 carries
  it both ways (Ethereal 8→10 +12.68 up, Stash 8→7 +7.87 down, Weiss
  #639/#641 lower still at ~3300) — sweep, do not seed.
- - **Search explosion is the published hazard, not a hidden mate** — the
  extension only adds depth. Caps: +1 once per node, no recursive exclusion,
  `ply < SE_PLY_FACTOR * depth`, the MAX_PLY walls (`src/search.cpp` `negamax`,
  `src/search.cpp` `quiescence`). §6's fixed-node depth check is the instrument
  that catches a blowup before an SPRT spends a night on it.
- - **TT pollution from the verification:** its result is computed with the
  best move removed — stored, it poisons every later probe of the position. The
  no-store gate (`src/search.cpp` `negamax`) is SF-prose-backed (ebe021f6); the
  no-cutoff gate keeps the entry from answering its own verification (the
  entry's lower bound >= singularBeta would multicut every time, vacuously).
- - **Mate scores.** De-normalize before deriving anything (S106's lesson;
  tt_entry_answers does it at `src/search.cpp` `tt_entry_answers` for cutoffs,
  this is a second
  reader); `|tt_score| < MATE_MIN` gates entry; singularBeta itself must stay
  out of the mate band (Lynx #2559/#2560 — the merged guard measured ~0 but
  exists to stop false mate reports); multicut never returns a mate-range value
  (#1761). MATE_MIN/MATE_MAX are `src/search.cpp` `MATE_MAX` and
  `src/search.cpp` `MATE_MIN`.
- **IIR (S095, lands before) is disjoint by construction** — IIR fires on
  `tt_move == 0`, SE requires `tt_move != 0`; inside the verification node
  the probe still finds the entry, so IIR stays off there unless the
  implementation masks tt_move under exclusion — do not. Subtree records
  both ways: Berserk #91 +3.64 disabling IIR during SE, Lynx #1734 +0.09
  ~0 — no edit owed by default; S127 revisits.
- **S109's pruning stays live inside the verification** — pruning the quiet
  tail biases toward "singular", and the published record treats that as a
  feature: Ethereal deliberately limits quiets tried to disprove singularity
  (+10.82). No suppression owed; the SPRT prices the bias.
- **A collided or stale tt_move** not in the generated list: the exclusion
  matches nothing, the verification degenerates to a half-depth re-search
  and the extension never fires — harmless but wasted; gating on the move
  appearing in the list is one comparison if the waste shows in profiles.
- - **The abort path:** vscore from an aborted verification is garbage — check
  state->aborted immediately (the `src/search.cpp` `negamax` pattern) and take
  no decision from it.
- **The repo gate:** both mate suites re-run per verdict (CLAUDE.md: any new
  pruning gets the mate treatment before it is called done — multicut is
  pruning); tune-build strength numbers forbidden (S073).

### 6. Measurement

Two SPRTs at the S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`,
V1 then V2, each against the commit before it; fast suite plus both mate
cases green first, new tests observed red first with printouts recorded.
Node counts move by construction — INV-6 takes the SPRT path both times;
search_bench depths 9/12 node counts recorded per verdict in the stamp.
**Node-explosion check recorded per verdict:** depth reached at fixed
`go nodes 1000000` over the three search_bench positions, before and after —
SE trades nodes-per-depth for time-to-mate-class-information; a falling
fixed-node depth with no SPRT gain is the explosion signature. Expectations
(DEC-019, direction only): V1 +10..+20 (Weiss +11.5, Berserk +19.8 at
10+0.1; Lynx +20.5 LTC); V2 0..+6 (Ethereal +5.7, Lynx +6.2 LTC, Stash
dropped it at ~3000). A stalled V2 near +3 straddles the {0,5} bounds
(DEC-063): terminate, record zero, drop by default.

### 7. Interactions

- **S095 (before):** node conditions disjoint (tt_move == 0 vs != 0); IIR
  inside the verification is off by construction; records both ways carried
  in §5; no edit owed.
- **S098 (before):** the LMR clamp and the V3 deeper-cap follow the extended
  child depth — the edit S098 §5 assigns to this step; SE needs nothing from
  its cut_node plumbing for V1/V2.
- **S099 (before):** correction history runs inside the verification
  unchanged — Lynx #1844 "Avoid correcting on SE verification" measured
  +0.49 ~0; nothing owed now.
- **S109 (before):** its prunes run inside the verification subtree and
  against `singular_beta - 1` at the excluded node; kept live (§5).
- **S113 (after):** ProbCut also runs reduced verification-like searches but
  asks the opposite question — a raised bar (`beta + margin`) over captures
  to prune the node, where SE asks a lowered bar (`ttScore - margin`) over
  the alternatives to classify one move; multicut is where the two answers
  meet. Distinct machinery; no sharing owed.
- **S127:** every SE constant refit; the deferred refinements priced there
  or as a follow-up step — negative extensions, double/triple extensions,
  the quiet-limit, ttPv bonus (blocked on an entry flag bit).
- **S130 (before):** unaffected — the verification's no-store keeps the
  stand-pat TT reads it added clean.

### Scope concerns

1. **The Lynx band discrepancy** (§1) — **closed 2026-09-11, S181, in the
   README's favour.** The CCRL Blitz list computed 2026-09-05 gives
   **3138/3224/3291** for v1.8.0/v1.9.0/v1.10.0 against the README's
   3144/3225/3293, so DEC-087's Lynx-anchored banding did sit about 300 low
   and S099's headline number was measured at 3224-3291. S097's case survives
   as this pass predicted, on Weiss, Berserk and Stash; DEC-133 moved S099 to
   the reserve head and DEC-176 records what the re-banding does and does not
   change. `adocs/data/S181_lynx_bands.md` is the table.
2. **Negative and double extensions measured +11.99 and +18.33 LTC at
   Lynx's introduction release** — large enough that a follow-up step (not
   a smuggled third change; the accepts prices exactly two verdicts) is
   worth creating if V1 passes. Routed to S127-era or a new step id.

### 8. References

- - https://www.chessprogramming.org/Singular_Extensions — origin papers,
  Stockfish 1.6 lower-bound restriction, exact-bound relaxation, margin/depth
  experimentation space.
- - https://www.chessprogramming.org/Multi-Cut — Björnsson/Marsland papers,
  M/C/R parameters, the SE fold-in as modern practice.
- - Anantharaman, Campbell, Hsu 1988/1990; Anantharaman ICCA 14(1)/14(2) 1991;
  Hsu, Behind Deep Blue 2002 — cited via CPW's reference list, not fetched.
- - https://api.github.com/search/issues?q=repo:TerjeKir/weiss+singular+type:pr
  — #354 intro +11.52/+24.21; #639/#641 lower depths; #655 negative
  +2.66/+3.14; #656 double +11.52/+11.18; #600 SMP TT probing; #644 skip TB;
  #756 terminal-score condition; #781 negative-for-cutnodes +3.25/+3.55.
- - https://api.github.com/repos/TerjeKir/weiss/pulls/354 — "Code inspired by
  SF, Eth and many other engines"; the two SPRT blocks.
- - https://api.github.com/repos/TerjeKir/weiss/releases — v1.2 2020-10-17
  "Singular Extension"; v1.1 2020-09-02.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+singular+type:pr
  — #1731 intro +20.53 LTC ("se-04-7-no-tt-cutoffs"); #2331 ttPv bonus +4.95;
  #2559/#2560 singularBeta mate guards.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+SE+in:title+type:pr
  — #1737 no check-ext in verification +1.14; #1734 IIR-during-SE +0.09
  unmerged; #1844 no correcting on verification +0.49; #2423 double negext on
  cutnode +1.73.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+multicut+type:pr
  — #1751 singularScore +6.21 merged; #1750 singularBeta -0.84 unmerged; #1752
  reduce-instead +0.78 unmerged; #1761 mate guard; #2450 fail-firm -0.75
  unmerged.
- - https://api.github.com/repos/lynx-chess/Lynx/releases/tags/v1.10.0 — the SE
  block shipped whole: #1731/#1742/#1743/#1751/#1761/#1768/#1777.
- - https://api.github.com/repos/lynx-chess/Lynx/issues/1742 and /1743 — double
  ext +18.33 (margin 15), negative ext +11.99.
- - https://raw.githubusercontent.com/lynx-chess/Lynx/main/README.md — the
  version/rating table behind the band caution (1.8.0=3144, 1.10.0=3293).
- - https://api.github.com/search/commits?q=repo:jhonnold/berserk+singular —
  b0eea29 #42 intro; e06b444 #45 "Disable TT and NMP on singular search";
  60bcdd6 #102 double +2.90; 0e42746 #542 triple +6.73; 8cae7ac #424 negative
  reductions +2.13; 1b95cfe #198 eval reuse +1.22.
- - https://api.github.com/repos/jhonnold/berserk/pulls/42 — +19.82 +/-9.34 at
  10+0.1, 2021-04-07.
- - https://kirill-kryukov.com/chess/discussion-board/viewtopic.php?t=12771 —
  Berserk v1.2.0 ~2000, v2.0.0 ~2450 author estimates (2021-02/03): the
  introduction band anchor.
- - https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+singular —
  ad8a87c6 "no Multi-Cut this time" 2020-12; 2db36e4f retry 2021-02; dfa889e7
  depth 8→7 +7.87/+4.97; c9238d02/df1e1c9b more SE +2.50/+6.27; 07b3c88e
  negative +2.66/+2.05; 343ed1c5 skip known win/loss +5.73; c34419cc quiet
  3-ply 2025.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+singular —
  aa36bc09 2018-04 intro (+6.23/+3.46/+12.68); d397a7b5 multicut +5.74;
  464fa339 quiet limit +10.82; f71ac476 negative +4.66.
- - https://api.github.com/repos/AndyGrant/Ethereal/releases — 11.16 "apply
  Singular Extensions aggressively"; 11.38 quiet limit; 12.08 "singular moves
  mistaken as MultiCut moves" fix; 13.76 singularity() simplification.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+excluded
  — ebe021f6 2017 "Don't update TT at excluded move ply"; d6bdcec5 excludedMove
  reset prose; 5bec768d/25c22ffe 2009 tte/ttMove condition prose.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+singular
  — 23cbb221 depth-dependent margin; 8e823459 threshold 6/8 prose; 30c58320
  check-ext exclusivity; 16566a8f capture coupling; 4d0981fe mate-position
  revert; b34a690c/b1f52293 singular result reused by MCP.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22negative+extension%22
  — e1f12aa 2022 intro; 39da50e/98dafda/a48573e 2023; c43425b 2024
  simplify-away; c5aef2b 2026.
- - https://talkchess.com/forum3/viewtopic.php?t=38104 — edwardyu 2011: skip
  hashmove, sing_beta = value - margin, depth>=6, entry within 3 of depth;
  Dailey (Komodo "really big"), Hyatt (Crafty none).
- - https://www.talkchess.com/forum3/viewtopic.php?t=68290 —
  once-per-line/explosion discussion; Cardoso: "depth>8*ONE_PLY", node-count
  cost.
- - Unreachable this pass: ccrl.chessdom.com (DNS), computerchess.org.uk (403)
  — Weiss 1.1 / Berserk 3.x-4.x blitz numbers untraced, said so above.

## Second tier on V1's landing commit `88ec74f`, 2026-09-21 (coordinator)

**Fast check** by a cold reviewer over the landing diff before it landed: one
real finding -- the verification gate read the entry's depth, bound and score
through the raw slot pointer after the null-move recursion, masked at today's
seeds only -- repaired by copying the three fields out beside the table move
before any recursion; plus `SeExtend`, the off switch DEC-215 clause 2 asks
for, proved to the node against the parent (4579468, eight identical
`bestmove` replies); the twelve re-pointed older mutants re-run and killed
with the switch's own (13 of 13); one instrument described as it prints. The
reviewer reproduced the bench, the multicut's off value, `search_bench` at
both depths, the fixed-node depths and the mutant kills. DEC-226 records the
two rules the verification search needed where the record was silent.

**Debug self-play, DEC-141 clause 1**, on the landing tree's Debug build: four
rounds at 4+0.04 on `books/noob_3moves.epd`, concurrency 8, `-log level=trace
engine=true` -- **8 games, 0 `Assertion`, 0 `disconnect`**, 186396 trace lines
with 1261 `bestmove` lines (`.tuning/coord/s097_v1_debug_selfplay/`).

`tools/gate_extra.sh` launched detached on `88ec74f` at 04:31
(`.tuning/gate_extra_2026-09-21_s097v1.log`), watcher armed with four exits and
a 90-minute ceiling; its marker is recorded below before `CAND` is pinned and
the SPRT starts.

**`tools/gate_extra.sh` on `88ec74f`: `GATE-EXTRA-DONE 5 stages 1132 s`**
(04:31 to 04:50, `.tuning/gate_extra_2026-09-21_s097v1/`), prose, citations,
debug, sanitize and perft green. **`CAND` pinned to `88ec74f`** in
`adocs/data/S097_v1_sprt.sh`; `REF` is `5c76ea9`. The SPRT is the
coordinator's next action.

## Verdict 1, 2026-09-21: H0, a zero (coordinator)

The gainer SPRT of `88ec74f` (the singular extension, `SeExtend` 1,
`SeMultiCut` 0) against `5c76ea9` (the tree without the verification search),
`{0, 5}` nElo at 8+0.08 with Hash 16 on `noob_3moves.epd`, seed
20260921045106, launched 2026-09-21 04:51:09 and `SPRT-RUN-DONE` at 14:18:33,
**accepted H0 after 20080 games**:

```
SPRT | cand 88ec74f vs ref 5c76ea9, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | -0.81 +/- 3.70, nElo -1.06 +/- 4.81
LLR | -2.96 (-2.94, 2.94) -> H0
Games | N: 20080 W: 6123 L: 6170 D: 7787, Ptnml [925, 2332, 3507, 2417, 859]
Wall | 9 h 27 m, 2124.6 games/h, forfeits 0
Log | adocs/data/S097_v1_sprt.log
```

LOS 33.32 %, draw ratio 34.93 %, pairs ratio 1.01. **0 time forfeits on
either side** over the PGN's 20082 games (13969 adjudications, 6113 natural
ends); `Incomplete mating PV` 14 candidate against 9 reference, an
observation and not a diagnosis (CHESS). `adocs/data/S105_pairs.py`: 10040
complete pairs, pair score mean 1.0156, variance 0.3160, sd 0.5622, buckets
9.4 / 23.0 / 32.9 / 24.3 / 10.4 %, white winning both of 947 pairs (9.4 %),
117.2 plies and 19.8 s a game. The abort rule was never near: 2124.6 games an
hour against the 2110 budgeted, and the load of 17 on twelve threads was
fastchess alone. Evidence: `adocs/data/S097_v1_sprt.log`,
`adocs/data/S097_v1_sprt_pairs.txt`; the run directory
`.tuning/sprt_s097_v1_20260921_045106`.

### The reading

The extension alone does not gain 5 nElo over the tree without it. The nElo
interval is [-5.87, +3.75], its centre within about one nElo of zero and its
top short of the bound: a zero, not the loss DEC-194 refused in S231's
[-9.60, +2.80], and the walk reached the bound in fewer games than a truth at
zero expects (25591 on a bound, DEC-143). The pre-registration's H0 text
(`adocs/data/S097_v1_sprt.sh`) wrote two things before the number, and the
coordinator reads both here rather than following them blind:

- **The one-line flip of `SeExtend` to 0 is not one line.** The release build
  compiles the switch as a constant, and the fifteen cases and eighteen
  mutants that pin the block assert it fires; a flip to 0 guards or retires
  every one of them, and the removal or the keep that verdict 2 decides would
  undo that pass either way. The flip buys nothing measurable meanwhile --
  S132's reference and candidate share the tree whichever value the switch
  holds -- so the block stays at `SeExtend` 1, recorded as a zero and not as a
  gain, as the carrier of verdict 2.
- **"Drop both" was priced for a loss, and this is a walk.** The
  recommendation's argument -- the multicut would have to pay for the
  verification search alone -- is the argument for a multicut without the
  extension. With the extension in the tree the search is paid for at zero
  (its gain matched its cost, which is what the interval says), and the
  multicut's verdict is a pure-margin question, the most favourable form the
  pair can be measured in. The pre-registration's own exception, a walk near
  the bounds rather than a measured loss, is this reading; the owner's
  standing priority of strength and correctness over machine time
  (2026-09-19) is why a night is spent on the answer rather than saved.

**DEC-227** records the decision: verdict 2 runs exactly as
`adocs/data/S097_v2_sprt.sh` pre-registered it -- one default, `SeMultiCut` 0
to 1, with its guard case, its mined mate row and mutants E20 to E22, against
the tree with the extension -- and its reading decides the block whole: H1
keeps both with the extension's zero on record; H0 or a stalled walk
(DEC-063) removes the block -- code, cases, mutants and the six settings -- in
one revert to `5c76ea9`'s search, proved by the bench signature 4579468 and
not argued.

### What comes next, and in which order

The machine goes to **S132 first**: its code is written, its agent is waiting
for "machine free", and its increments are the better work DEC-155 wants done
while a night run is not yet ready. Verdict 2's landing is written meanwhile
by the step's agent **in a separate worktree**, so the two agents never share
a file, and lands after S132's completion; verdict 2's SPRT follows S132's.
The fixed-node depth instrument runs again at that landing (section 6, per
verdict).


## Verdict 2 landing, 2026-09-21: one default and the coverage it makes possible

Written in a linked worktree (`../chesso-s097v2`, branch `s097-v2`, from
`b9ae156`) while S132 holds the machine, so everything in this section is code,
tests, mutants, a mining script, an amended pre-registration and document rows.
**Nothing in it has been compiled, run or measured**; every number the verdict
owes is taken after the machine comes free and after the rebase onto S132's
landing, and is stamped then. DEC-227 is the decision this lands under: verdict
1's H0 is a walk and not a loss, the block stays at `SeExtend` 1 as this
verdict's carrier, and this verdict's reading decides the block whole.

### What moves in `src/`, and it is one line

`src/search_params.hpp`'s X-macro row for `SE_MULTICUT`, from the off value to
1, range unchanged. **No line of the rule itself moved.** The multicut in
`src/search.cpp` `negamax_at` is exactly what verdict 1 landed: the `else` leg
of the singular test, gated on the switch, on `vscore >= beta`, on `!is_pv`, on
the verification's own score being outside the mate band and on `beta` being
outside the negative one, returning the fail-soft `vscore`. Writing the guard
case against it showed no defect, so nothing was repaired and nothing is
reported as repaired.

The X-macro's own comment moved with the default: it now says the switch shipped
inert with verdict 1, that verdict 2 is this default and nothing else in `src/`,
and that 0 is still the off value proved on the tree rather than declared from
the range's end.

### The rule's direct guard case, DEC-141 clause 2

`tests/test_search.cpp` "the multicut returns the verification's score and
searches nothing", in the "search: pruning and reduction guards" suite, on
`se_drive_t` and the blocked-pawn fortress the other fifteen cases of the block drive -- sixteen with it, fourteen in both builds and two in the tune build alone. Every
precondition is asserted **inside the drive** so that no leg of it can pass
because some other condition was false:

| what the case reads | why it is there |
|---|---|
| `se_verified`, `se_vscore >= se_singular_beta`, `se_vscore >= beta` | the verification ran and failed high at or above this node's own beta, which is the rule's whole premise |
| `se_vscore` and `beta` both outside the mate band | the two mate guards hold at this drive, so the leg below is about the rule and not about a guard refusing it |
| `se_multicut` | the rule fired |
| `move_count == 0` | the node answered **without searching a move**, which is what makes this pruning and not ordering, and the half a returned score cannot show |
| `se_vscore != se_singular_beta`, then the node's return `== se_vscore` and `!= se_singular_beta` | the fail-soft score and never the bound -- this project's own choice where the record is silent, and the inequality is asserted first so the claim is about the rule and not about two numbers that coincide |
| the returned value outside the mate band | the property the guard exists for |
| the same drive at `is_pv` true: verified, failed high over beta, **and no multicut**, and the node searched | the PV gate, the second choice the record leaves open |

Two things the fixture gained, both local so the rebase stays small: a
`last_score` member, because the probe records what a node decided and not what
it answered with and the multicut's whole effect is the value that comes back;
and a trailing `is_pv` argument defaulted to what every earlier case passed, so
the fifteen of them are unchanged by its arrival.

**The window is the case's own and it is why the flip moves no other case.**
Every other drive in the block runs at `SE_BAND_BETA`, a mate distance no score
from a fortress can clear, so the multicut cannot fire under any of them at
either value of the switch. The multicut case runs at `SE_MULTICUT_BETA`, a
hundred points below the planted entry's score, where the verification's own
answer clears the node's beta.

### The off-value case changed meaning, and moved to the build that can drive it

"the multicut does not fire at its off value" was a release-build case that
asserted every condition of the rule held and the node searched anyway -- all a
folded-away branch could be held to. At the shipped 1 that reading is the off
value's, so the case is now `CHESSO_TUNE`-only and is the mirror of "the
extension does not run at its off value": control at 1 with the rule firing and
the node searching nothing, then `search_param_set("SeMultiCut", 0)` with a
destructor restoring 1, then the same drive verifying, failing high over beta
and **not** multicutting, with the node searching its moves. What the release
build holds instead is the bench signature at the off value, which is the same
pairing `SeExtend` has.

### The accepts' mate row, and the script that mines it

Owed at the second report and **mined, never chosen** (CHESS, DEC-023).
`adocs/data/S097_mine_mate_row.py`, four stages, written in the shape of
`adocs/data/S095_mine_mate_row.py` and `adocs/data/S230_mine_r01_row.py` and
**importing** the second rather than copying it -- the pool, the node-limited
shortlist and the fresh-process oracle call are S230's, and this directory is
append-only. Positions are this project's own and nothing else (DEC-016).

Two things it measures that S230's sweep does not, and both are the multicut's:

- **The swept range is not 3 to 12.** The block wants `ply > 0` and a remaining
  depth of at least `SeMinDepth`, and a `search_fen()` root is at ply 0, so the
  shallowest fixed depth at which any node of the tree can multicut is
  `SeMinDepth` + 1. The script reads that parameter out of
  `src/search_params.hpp`'s X-macro rather than writing it down, so a refit
  moves the sweep with it.
- **The rule has to be shown to fire on the position by the engine.** The
  `fires` stage drives the **tune** library, where the switch is a variable,
  and sweeps each candidate twice, at 0 and at 1; a cell whose node count or
  mate reading moves is the rule reaching that position. A candidate the rule
  never reaches is dropped before the mutant sweep, because a red there could
  not have come from this rule. No `src/` change was needed for the census and
  none was made.

The driver is `search_fen()` of `tests/test_search.cpp` line for line, for the
reason S230's header gives, and it prints the node count and the milliseconds of
each cell as well as the distance: the row is read at one depth by a case the
gate runs on every commit, so what it costs is part of choosing it. The red is
observed under `E21_multicut_mate_band_gate_dropped` applied by hand, observed
and reverted -- the S033 protocol -- and the failing line is recorded with it.
**The pick rule is stated before the sweep runs** (DEC-209 clause 4): the lowest
depth the shipped build reports the mate at and the guard-dropped build does
not, ties broken by the longest run of consecutive shipped depths and then by
the cheaper cell. Stockfish confirmation of the chosen row follows S033's
protocol through python-chess.

### Mutants E20 to E22, and why every E mutant re-runs (E23 arrives later)

`tools/mutants/S097_singular_extension.py` gains the three ids its own header
reserved at verdict 1, with the reasons that header gave:

| mutant | killed by |
|---|---|
| E20 the multicut returns `singular_beta` instead of the fail-soft score | "the multicut returns the verification's score and searches nothing" |
| E21 the mate-range guard on the returned value is dropped | the mined row of "pruning does not hide a forced mate" |
| E22 the `!is_pv` guard is dropped | the second leg of the same guard case, the drive relabelled |

E21 drops the two terms about `vscore` and leaves `beta > -MATE_MIN`, which is
S165's guard on the node's own window and a different rule. Every anchor was
checked to occur exactly once in `src/search.cpp` as the formatter leaves it,
all twenty-one of them, and the three new mutations were printed and read
before anything was run.

**At the second report every E mutant re-runs, E01 to E22** -- E23 did not exist when this was written and the fast check is what added it. The flip changes
what the release build compiles, so verdict 1's eighteen kills are re-proved
and not inherited.

### Goldens, DEC-142

- `tests/test_search_params.cpp` `golden_defaults`: the `SeMultiCut` row moves
  with the default. No script and none is owed -- `src/search_params.hpp` is
  the derivation and a diff of the two is the re-derivation -- and the row
  count is unchanged at 57.
- `tests/test_mate_carry.cpp` `short_line_ceiling` and `tests/test_search.cpp`
  `capture_mates`: a pruning rule moves what both measure, so both are
  re-derived at the second report by the scripts their own GOLDEN blocks name
  and each one that moves is named. Neither is assumed unmoved.

### The pre-registration, amended before a game is played

`adocs/data/S097_v2_sprt.sh`, five amendments and DEC-143's cost lines, regime
and abort rule left as they were:

1. **The H0 and no-verdict outcomes now read as DEC-227 has them**, with the
   reason written before the number. The old text said "drop the multicut and
   keep the extension"; that was written while verdict 1 was still open and
   verdict 1 read H0, so there is nothing left for an H0 here to fall back to.
   Both readings now remove the whole block -- the verification search, the
   extension, the five gates, the six settings, the sixteen cases and the
   twenty-two mutants -- in one revert to `5c76ea9`'s `src/`, proved by `bench`
   4579468 and `tools/search_bench.py` identity at both depths and not argued.
2. Verdict 1's number and reading sit in the attribution paragraph, with the
   paragraph that says why the multicut is still worth a night after a zero.
3. `REF` is the tree the flip lands on -- its parent, not `88ec74f`, because
   S132 lands between the two verdicts -- and `PIN_ME` with its refusal stays
   (DEC-020).
4. The bench, `search_bench` and fixed-node-depth rows are filled at the second
   report and marked to be re-taken after the rebase, because a pair quoted
   across two trees attributes nothing.
5. The three open test-side findings are named again, each re-checked against
   this tree and each still open, with the note that `adocs/plan.md`'s Open list
   names no finding step at all and that S097's fast check and verdict-1 run
   opened none.

### Documents

- `MANUAL.md`'s `SeMultiCut` row: default 1, what the rule returns and the four
  things it never does, and what 0 now means -- traced to the branch in
  `src/search.cpp` `negamax_at` and not to the option table's old sentence.
- `DEV_MANUAL.md`: a bench ledger entry for the flip, with the direction a
  pruning rule's total should move and what an unmoved one would mean (DEC-212,
  DEC-214), its numbers filled at the second report; a golden-list row for the
  mined mate row with the four-stage command that re-derives it; and the
  fixed-node-depth example switched to the off value, which is now the
  interesting one.
- `adocs/data/README.md`: the amended `S097_v2_sprt.sh` row, a row for
  `S097_mine_mate_row.py` and a row for `S097_candidates.tsv`, its committed
  artefact.
- **Two in-scope corrections, made here rather than left.** `DEV_MANUAL.md`'s
  golden list said `golden_defaults` held 56 defaults and five S097 rows where
  the table holds 57 and six -- stale since verdict 1 -- and it is the golden
  this verdict moves, so it is corrected in the same edit. And two sentences in
  this file stated the multicut's old default in a form
  `tools/plan_prose_check.py --params` reads as a claim about the value the
  engine compiles; both were true of verdict 1 and both are reworded, with the
  reason beside them, because that check is in the gate.

### Measured 2026-09-21, 16:15 to 19:15, machine held exclusively

Taken in the worktree on the rebased tree, `7a752ee` = `474c288` plus the flip,
with the reference a Release build of `474c288` in a linked worktree of its
own. **`.ref-builds/88ec74f` was not usable as the reference** and the
coordinator's note that it carries the same `src/` is wrong: S132's landing
moved 356 lines of `src/` across five files, so the parent was built rather
than borrowed.

**The gate.** At the first pass Release and tune were 40 of 40 but for
`test_plan_params`, red on one line because `adocs/specs.md` still said the
multicut was off and the check reads a documented default against the engine's
own; the coordinator's passage closed it. **At the verification pass both
builds are 40 of 40 green**, 123.8 s and 124.7 s, with
`./clang-format.sh --check` clean at `CLANG_FORMAT_MAJOR=22` and
`tools/plan_prose_check.py` clean on `--params`, `--citations` and
`--touches`. **The flip reddens no case and no golden**, and the mate row
costs the gate about 0.65 s a build.

The three cases this landing is about were run on their own and are green: "the
multicut returns the verification's score and searches nothing" (63
assertions, Release), and in the tune build "the multicut does not fire at its
off value" and "the extension does not run at its off value" (55 each).

**The bench, and the off value proved on the tree.**

| tree | `bench` | against the reference |
|---|---|---|
| reference `474c288` | **5066204** | -- |
| candidate, `SeMultiCut` 1 | **4493659** | **-11.30 %** |
| tune build, `SeMultiCut` 0 | 5066204 | the reference's total **to the node**, all eight `bestmove` replies identical |

The two differ, so the flip is not inert, and the off value is proved with the
full signature rather than declared (DEC-215). The candidate's own eight
`bestmove` replies are **also identical to the reference's**: the rule prunes
an eighth of the tree at a fixed depth and moves no answer the bench reads.

**What the tree does at a fixed depth.** `tools/search_bench.py`, reference ->
candidate:

| position | depth 9 | depth 12 |
|---|---|---|
| midgame | 21479 -> 21479, `g5f6` | 154388 -> 149688, `c3d5` |
| kiwipete | 102462 -> 102462, `e2a6` | 459115 -> 459216, `e2a6` |
| tactical | 33148 -> 33148, `d7c8q` | 239314 -> 219544, `d7c8q` |

**Depth 9 is identical on all three**, and that is the gate measured rather
than argued: the block wants `ply > 0` at a remaining depth of at least
`SeMinDepth`, and a depth-9 root has no such node. No best move moves at
either depth.

**The node-explosion check, run in the other direction.**
`adocs/data/S097_fixed_node_depth.py`, `go nodes 1000000`, Hash 16:

| position | reference depth | candidate depth | best move |
|---|---|---|---|
| midgame | 16 | **17** | `c3d5` both |
| kiwipete | 13 | 13 | `e2a6` both |
| tactical | 15 | 15 | `d7c8q` both |
| total | 44 | **45** | |

This is the direction the pre-registration named before the numbers were taken
(DEC-212, DEC-214): a multicut prunes, so a smaller tree at a fixed depth and a
deeper search at a fixed budget. **One of the two plies verdict 1's extension
cost comes back** -- the parent of verdict 1 read 17 / 13 / 17 for 47 and this
tree reads 45. Whether that trade is worth anything is
`adocs/data/S097_v2_sprt.sh`'s to say and not this table's (DEC-019).

**Goldens, re-derived by their own scripts (DEC-142).** Every one of them was
green; each was re-derived anyway, because the code end moved.

- `tests/test_search_params.cpp` `golden_defaults`: the `SeMultiCut` row moves
  with the default, 0 to 1. No script and none owed --
  `src/search_params.hpp` is the derivation. 60 rows after S132's three.
- `tests/test_search.cpp` `capture_mates`: re-derived by
  `adocs/data/S230_mine_r01_row.py depths` over
  `adocs/data/S230_table_fens.txt`, depths 3 to 12
  (`.tuning/coord/S097_v2_capture_mates_shipped.txt`). Profiles
  `d9 d10 d11 d12`, `d9 d10 d11 d12`, `d10 d11 d12`, `d9 d10 d11 d12` with
  distances 5, 5, 4, 5 -- **every pinned depth (9, 9, 10, 10) is still in its
  row's profile and no distance moves**, the same reading S095's pass gave.
  The mutant half of those labels is the mutation run below.
- `tests/test_mate_carry.cpp` `short_line_ceiling`: a fresh 108-cell grid was
  taken on this tree (`.tuning/coord/S097_v2_grid.txt`) and the ceiling rule
  run by `adocs/data/S203_case_sweep.sh --ceilings` over all five recorded
  grids plus it -- **5, 15, 0, 2, 11, 5, unchanged**, so no ceiling moves and
  no decision is owed. This tree's own grid is 2, 5, 0, 1, 6, 2, at or under
  verdict 1's on five of six cases and far under the shipped ceilings
  everywhere.

### What is left, and whose it is

**The verification pass, and it is this agent's.** Four things go unmeasured
until the machine comes back and none of them may be assumed:

1. **The guard case's third leg and E23** -- written after the fast check,
   never compiled. Both suites re-run, and the leg observed green.
2. **`tools/mutation_check.py` over E01 to E23, once, on a clean worktree**
   -- clean but for the specs passage the landing commit itself carries, so
   the baseline is green without a hand edit that the tool's own guard cannot
   see. That run's marker and score replace the two above and are what the
   landing quotes.
3. **`bench` re-taken**, because a `src/` that has not moved should print
   4493659 again and a signature quoted from before an edit pass is a
   signature nobody re-read.
4. **`pick` and `fires` re-run from the existing sweep outputs**, which is
   what proves the corrected recipe end to end. `separators` was already run
   against the two sweeps while the machine was busy -- it is a text join over
   two files, no engine -- and it returns exactly the three FENs `fires` was
   given, so the derived stage reproduces the set that was used.

**A full re-sweep of the 269 is not owed, and here is the test for that.** The
two sweeps compile a driver against `src/`'s library and read nothing else;
`src/` has not moved since they ran -- the flip is one X-macro row and every
edit since has been in `tests/`, `tools/mutants/`, `adocs/` and the two
manuals -- and the E21 library was built from that same `src/` with one guard
removed. E23 adds a mutant, not a line of the engine. What **would** owe a
re-sweep is any change to `src/`, a rebase onto anything that moves it, or a
change to the pick rule's predicate; the first two are worth watching, because
this branch has already been rebased once.

**One thing is the coordinator's and the gate is red until it is done**:
`adocs/specs.md` on `achesso` still says the multicut is off, so
`tools/plan_prose_check.py --params` -- `test_plan_params` in both suites --
fails on that one line. The passage is already on this branch in the
coordinator's own WIP commit and belongs in the landing commit; **and it is
the sentence the fast check corrected**, because "each pinned by a case and a
mutant" is true of all three only once the third leg and E23 are in and green.

DEC-141's second tier is the coordinator's on the landing commit: the Debug
self-play and `tools/gate_extra.sh`. The SPRT is the coordinator's to pin and
launch; `adocs/data/S097_v2_sprt.sh` carries every number above, `REF` is the
flip's parent and `CAND` is pinned after the landing.

### The landing commit, proposed

    Switch the singular verification's multicut on for its own SPRT

    S097 verdict 2. One default -- SeMultiCut 0 to 1 in
    src/search_params.hpp -- and the coverage that default makes possible:
    the rule itself shipped inert with verdict 1 and no line of it moves
    here. At 0 the release build folds the branch away, so a case could not
    drive the rule and a mutant of it was equivalent by construction; the
    flip is what lets both land.

    The rule's direct guard case (DEC-141 clause 2) asserts the
    verification failed high at or above the node's beta, that the node
    returned without searching a move, and that what came back is the
    fail-soft score and never singular_beta -- checking first that the two
    are different numbers at the drive -- with a second leg at a PV node
    where the rule must not fire. The case that read the off value moves to
    the tune build, where a compiled constant can be moved (DEC-118).

    The accepts' mate row is mined and not chosen (CHESS):
    adocs/data/S097_mine_mate_row.py over 269 of this project's own
    labelled mates, swept at depths 11 to 14 on the shipped build and again
    with the mate guard dropped. Three separate the guard and one loses the
    mate; the oracle puts it at #+5 in 7918 nodes. Observed red with the
    guard removed, green with it.

    bench 5066204 -> 4493659, -11.30 %, with every bestmove unmoved; the
    tune build at SeMultiCut 0 prints the reference's total to the node.
    search_bench identical at depth 9 and smaller at 12 with no best move
    moving; the fixed-node depths go 44 -> 45, one of the two plies the
    extension cost coming back. Mutants E01 to E23 on a clean worktree,
    22 of 22 killed, each by the assertion written for it.

    Bench: 4493659

`Bench:` is the candidate's total and the subject is 60 characters. The
verdict-closing commit that follows the SPRT carries DEC-220's result block.

### The accepts' mate row, mined and observed red

**Mined, not chosen** (CHESS, DEC-023), by `adocs/data/S097_mine_mate_row.py`
over this project's own positions and nothing else (DEC-016): S230's pool,
269 labelled mates that stockfish at depth 20 in a fresh process still calls a
forced mate in 2 to 6 for the side to move (`adocs/data/S097_candidates.tsv`),
swept over depths **11 to 14** -- the range starts there and not at 3 because
the block wants `ply > 0` at a remaining depth of at least `SeMinDepth` and a
`search_fen()` root is at ply 0, which the script reads out of the X-macro
rather than writing down.

Both sweeps ran on the same tree, one against a Release library built with
`E21_multicut_mate_band_gate_dropped` applied and the source reverted
immediately after, so the two differ in that guard and in nothing else.
**Three of the 269 separate the guard anywhere in the range, and only one
loses the mate.** The other two report a *different distance* -- #7 read as #6
and #6 read as #7 -- which is a red case and is **not** what the rule
pre-registered in the script's header takes; they are printed as their own
class and left. That the first implementation of the pick line counted them is
recorded at the line itself rather than silently widened (DEC-209 clause 4).

The row, taken uniquely by the rule:

    4N3/8/3P1ppk/4p2p/4P2P/1n1P2P1/Q4PK1/3q4 w - - 5 46   depth 14, mate in 5

shipped profile `d12 d14`, guard-dropped `d12`, 4206525 nodes and 649 ms in the
cell. **The oracle** (S033's protocol, python-chess with a fresh process, never
a printf pipe): `#+5` at depth 20 in 7918 nodes, pv
`Qa7 Qf3+ Kg1 Qd1+ Kh2 Qh1+ Kxh1 g5 Qg7#`; python-chess reports `is_valid()
True`, `is_check() False`, 23 legal moves of which 2 are captures and none a
promotion (`.tuning/coord/S097_v2_oracle.log`).

**The firing witness, and it is the engine's word and not an argument.** The
`fires` stage drives the tune library, where the switch is a variable, and
sweeps each position at `SeMultiCut` 0 and at 1: on this position the node
counts differ at depths 13 and 14 and are identical at 11 and 12, so the rule
is live exactly where the separation is. No `src/` change was needed for that
census and none was made.

**Observed red, and this is the observation.** With the guard dropped by hand,
`./build/tests/test_search --test-case="pruning does not hide a forced mate"`
fails at this row -- `FATAL ERROR: REQUIRE( result.mate_found ) is NOT
correct!`, `values: REQUIRE( false )`, `logged: mate the multicut hides, depth
14` -- and passes with the guard in place, 54 assertions. Applied by hand,
observed, reverted (`.tuning/coord/S097_v2_mate_row_red.log`).

**One defect in the mining script, found by its own output and fixed.** The
firing witness first compared whole sweep cells, milliseconds included, so two
runs of the same binary differed everywhere and a 20-position probe came back
20 of 20 with identical node counts in every cell. The comparison now drops
the wall time; the corrected witness is what the three rows above carry.

### Mutants: 22 of 22 killed in one run, and two runs of history behind it

`tools/mutation_check.py` over `tools/mutants/S097_singular_extension.py`,
E01 to E23, on a linked worktree created fresh for it at `aa45ea2` and clean
by its own `git status`: **`baseline green, 40 tests, bench 4493659 nodes via
engine`** -- a real test count and the tree's own total -- then
**`mutation score 22 of 22 (100%)`, `MUTATION-RUN-DONE`, wall 2961 s**
(`.tuning/coord/S097_v2_mutation3.log`, per-mutant rows in
`.tuning/coord/S097_v2_mutants3/results.tsv`, the killing assertion per mutant
in `.tuning/coord/S097_v3_killers.txt`). **Every E mutant was re-run rather
than inherited from verdict 1**, because the flip changes what the release
build compiles. Nothing survived and nothing was stillborn.

Each is killed by the assertion written for it, the four of the multicut
included:

| mutant | killed by |
|---|---|
| E20 the multicut returns `singular_beta` | "the multicut returns the verification's score and searches nothing", at `REQUIRE_EQ( last_score, record.se_vscore )` |
| E21 the mate-range guard on the returned value dropped | "pruning does not hide a forced mate", at `REQUIRE( result.mate_found )` -- **and by nothing else in the suite** |
| E22 the `!is_pv` guard dropped | the same guard case's second leg, at `REQUIRE( !pv.se_multicut )` |
| E23 the defender's-beta guard dropped | the same guard case's **third leg**, at `REQUIRE( !defender.se_multicut )` -- **and by nothing else in the suite** |

**E21 and E23 are each killed by one case and one case only**, which is the
sharpest thing this run says: without the mined row and without the third leg
those two guards would be proved gaps rather than proved guards. The eighteen
from verdict 1 are all killed again, each by its own case -- E01, E02, E03,
E08 and E13 by "the extension lands on the table move and on no other", E04 by
the root case, E05 by the entry-depth case, E06 by the mate case, E07 and E12
by the recursion case, E09, E10 and E11 by their own excluded-node cases, E14
by the extension case's window assertion, E15 by the only-move case, E16 and
E17 by the two legs of the mate-window case, and E18 by eight cases at once,
which is the property a switch has to have for an H0 to mean anything.

**Six of the twenty-two left the bench signature unmoved and the suite killed
all six** -- E07, E09, E15, E17, and now **E22 and E23**, the two multicut
guards that only ever fire at a node type or a window a fixed-depth bench from
a cold table never presents. That is the half of DEC-141 clause 2 a signature
cannot do, and this verdict added two more cases to the list.

**Two earlier runs are history and the record keeps them.** Run 1
(`.tuning/coord/S097_v2_mutation.log`) ended `MUTATION-RUN-FAILED` at 19 of 20
built: `E05` was **stillborn** -- verdict 1's fast-check copy-out left the
gate as the only reader of `tt_entry_depth`, so the mutant orphaned it and
Release with `-Werror` refused it, a fall out of the score that had been
invisible since the landing, repaired here with the `(void)` the tool's header
prescribes -- and `E21` **survived**, correctly, the worktree carrying no
mined mate row and that row being E21's only killer. Run 2
(`.tuning/coord/S097_v2_mutation2.log`) killed both but its header named
`7a752ee` while its `tests/test_search.cpp` had been copied in and its
`adocs/specs.md` patched; `require_clean_src` guards `src/` alone, so neither
showed. The run above is what the landing quotes and
`.tuning/coord/S097_v2_mutation_results.tsv` is that run, with a header
pointing at the two logs.

**And one finding about the tool, unfixed and outside this step.**
`tools/mutation_check.py`'s `--label` is the ctest label and defaults to
`fast`; given a label no test carries -- which this agent did on the first
launch -- it printed `baseline green, ? tests` and scored every mutant a
survivor in 154 s. The misuse was the agent's and the run was re-launched, but
a tool that cannot tell "nothing failed" from "nothing ran" will do it again,
and the fix is one comparison on the baseline's test count. `require_clean_src`
guarding `src/` alone is the same class and is what let run 2's header be
wrong. Both are test-side by DEC-171 and belong in a step of their own.

**E23 and the guard case's third leg are the fast check's finding.** The specs
passage said the three things the multicut never does are "each pinned by a
case and a mutant" and only two were: `beta > -MATE_MIN` guards the **node's
own window** where E21's term guards the **value it returns**, E21 deliberately
leaves it in place, and no drive in the block used a beta inside the negative
mate band -- every one runs at `SE_BAND_BETA` or `SE_MULTICUT_BETA`, and the
guard case asserted `SE_MULTICUT_BETA > -MATE_MIN` as a precondition and not as
the thing under test. The coverage was added rather than the sentence softened:
a third leg drives the same plant and fortress at `SE_DEFENDER_BETA`, the band
edge `-MATE_MIN` itself, which denies a strict `>`, and asserts that the
verification still ran, still failed high, still cleared this beta and is
itself outside the band, then that the rule did **not** fire and the node
searched. **It compiled and passed first time** -- the guard case now carries
92 assertions where it carried 63 -- so the reasoning from the gates that said
the drive would reach the branch is now a measurement.

### The verification pass, 2026-09-21 19:23 to 20:25, machine idle

Taken after the branch was rebased onto `f02f59a` ("Complete S132"). **No code
moved under the flip across that rebase**: `git diff --stat 474c288 f02f59a --
src tests` is empty, every commit between the two being documents, so the
`src/` the two 269-position sweeps ran against is the `src/` measured here and
no re-sweep is owed by the test stated below. Both builds are Release, one
with `-DCHESSO_TUNE=ON`, read off their own `CMakeCache.txt`.

**Step 1 reddened nothing.** The guard case's third leg and E23's case code had
never been compiled; both builds compiled them without a diagnostic and the
guard case passes with **92 assertions** where it had 63, so the drive does
reach the multicut branch at a beta on the negative mate band edge exactly as
the gates said it would. No edit was needed and none was made. Both fast
suites 40 of 40, format and the three prose checks clean.

**Step 2, the bench, reproduces to the node.** Candidate **4493659**,
reference `474c288`'s own Release binary **5066204**, tune build at
`SeMultiCut` 0 **5066204**. All eight `bestmove` replies are identical across
all three: the off value is proved with the full signature, and the candidate
prunes 11.3 % of the tree while moving no answer the bench reads.

**`tools/search_bench.py` and the fixed-node depths stand from the first
pass** and were not re-taken, for the reason DEC-142 uses for a golden: neither
end moved. `src/` under the flip is one X-macro row against `f02f59a`, the
build flags are the same two, and `bench` reproduced its total and all eight
replies -- which is the instrument saying the tree is the one those rows were
read on. They would be owed again on any `src/` change or a rebase that moves
one.

**Step 3, one clean mutation run, 22 of 22.** Recorded in its own section
above: a worktree created fresh at `aa45ea2`, clean by `git status`, a baseline
reporting a real 40 tests at the tree's own bench, and every mutant killed by
the assertion written for it. E23 and the guard case's third leg were the only
untested things this landing carried and both are now measured.

**Step 4, the mining recipe re-run from its own header, reproduces the row.**
`separators` over the two recorded sweeps returns the same three FENs; `fires`
over them on the tune library puts the rule live at depths 13 and 14 on all
three and nowhere below; `pick` prints the two distance-moved rows it declines
and takes the same position at the same depth and distance -- `d14`, `#5`,
shipped `d12 d14`, guard dropped `d12`, 4206525 nodes. The later stages
reproduce without the sweeps being re-run, which is what they were re-run to
show.

### The rebase onto S132, and what was re-checked by reading

The worktree branch was rebased onto S132's landing `474c288` ("Scale the soft
time limit by the best move's share of the root", whose own commit line reads
`Bench: 5066204` -- verdict 1's total unmoved, which is what a time-management
change does to a fixed-depth bench), with no conflict. Re-checked without
building, because a clean rebase is not the same as a correct one:

- **Mutant anchors: 115 of 115 across `tools/mutants/` occur exactly once** on
  the rebased tree, this step's twenty-one and S132's seven included. S132
  added 28 lines to `src/search.cpp` and none of them collides with an anchor.
- **Case names: 155 in `tests/test_search.cpp`, no duplicate.** S132's new
  cases collide with neither the case this verdict adds nor the one it moves
  to the tune build.
- **`src/search.hpp` is untouched by S132**, so `negamax_probed`'s signature is
  the one `se_drive_t` calls, and S132's additions to
  `src/data_structures.hpp` are `search_state_t`'s root-move buckets and not
  `search_node_probe_t`. The drive calls `negamax_probed` and never `search()`,
  so neither S132's root loop nor its soft-limit multiplier is inside it.
- **`golden_defaults` is 60 rows**, S132's three node-fraction settings beside
  this step's six, and the `SeMultiCut` row carries 1. The table's own GOLDEN
  block already said 60; `DEV_MANUAL.md`'s golden list said 57 and now says 60
  with both additions named.
- The pre-registration's `REF` paragraph names `474c288` as the base instead of
  describing a landing that had not happened yet, and `88ec74f` survives in it
  only in the sentence saying REF is no longer that commit.

None of this is a substitute for the build: the cases, the mutants and the
goldens are still unverified and the reds the flip causes are still unseen.

### Proposed for `adocs/specs.md`, once this lands (the coordinator writes it)

The search row's singular-extension passage ends today with a clause saying the
multicut is off and the verification never returns a score of its own. That
clause is what this verdict changes, and it is also a **gate failure until the
coordinator lands the replacement**: `tools/plan_prose_check.py --params` reads
it against the value the engine compiles and flags it. The proposed ending:

> A verification that fails high instead ends the node: where its score reaches
> this node's own beta, some move other than the entry's already clears a bar
> just under the entry's score at a reduced depth, and the node returns **that
> score** -- the fail-soft value the search found and never the window's own
> bound -- without searching a move. Never at a PV node, never a value inside
> the mate band, and never against a beta inside the negative mate band.
> `SeMultiCut` is the switch, 1 since the second verdict, and at its off value
> the tree is verdict 1's exactly.

The first verdict's own passage is unchanged by this except for that ending.
