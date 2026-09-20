id:         S095
goal:       a node whose table entry carries no move reduces its later quiet moves by one more ply through the node adjustment, instead of the node itself being searched a ply shallower -- re-formed 2026-09-19 by DEC-222
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is (INV-6); one term in `lmr_node_adjustment` beside the four S098 verdict-2 terms, its constant in src/search_params.hpp with range 0..2, off value 0, seeded at form (c); at the off value the tree is the parent's exactly, bench signature identical, proved on the tree and not assumed from the range's end (DEC-215); the term applies only where the entry genuinely has no move, with a test asserting the precondition -- a node whose entry does have a move gets no extra ply, and the test fails if the precondition is absent; a position with a forced mate that the extra ply would hide added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the term unguarded; a mutant killed (tools/mutation_check.py); Debug self-play and tools/gate_extra.sh before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp negamax and lmr_node_adjustment, src/search_params.hpp, src/search.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tools/mutants/, adocs/data/, MANUAL.md, DEV_MANUAL.md
excludes:   the node-level depth cut this step was first written for -- available as a later step if this form reads H0; internal iterative deepening, the older and more expensive form
decisions:  DEC-071, DEC-105, DEC-134, DEC-222, DEC-221, DEC-215, DEC-141, DEC-142, DEC-143
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-20 04:55 while S231's gainer SPRT holds the machine, so the writing is done first and every measurement comes after
done:

## Amended 2026-09-19: the form changes, DEC-222

The step was written as a node-level depth cut: a node whose entry carries no
move is searched a ply shallower. The 2026-09-19 study review
(`adocs/audit/2026-09-19_study_review.md`, F05) found that the open-source
record which priced this step at +5.11 later measured the node-level cut against
a term inside the reduction -- one more ply of reduction on the node's later
moves when the entry has no move -- and kept the term, removing the cut as a
free simplification over 195,882 games. The owner chose the surviving form.
Chesso has the site already: `src/search.cpp` `lmr_node_adjustment` holds
S098 verdict 2's four node terms, each a constant in the 0..2 range with 0
off, and the no-table-move fact is one comparison on `tt_move` at the call
site. The step keeps its id; the sections below describe the original form
and stand as record -- section 4's two constants are superseded by the one
term above, seeded at form (c) like the four beside it. If this form reads H0
the node-level cut is available as a later step with its own file.


## Implemented 2026-09-20: what landed, and what decides it

Written while S231's gainer SPRT held all twelve threads, so everything in this
section is code, tests and pre-registration; every number it owes was taken
after the machine came free and is stamped in the completion note. **From the
description (DEC-221)**: the technique reached this agent as prose -- the
brief's and the amendment above -- and was implemented from that description
with its one constant seeded in DEC-134's form (c).

**The tree moved underneath the writing and the work was rebased onto it.**
S231's SPRT read H0 (`Elo -2.65 +/- 4.82` over 12070 games), its
pre-registered revert took `src/` back to `3a649c0` byte for byte (`bench`
4646334) and S231 completed on the zero (DEC-224). So this step lands on
`b25452a`: the two-ply continuation table, its axes and its
mutants are not in the tree under it, `tests/test_search_params.cpp`'s golden
table is 50 rows before this step's one, and the node-budget golden of
`tests/test_search.cpp` "ordering keeps the tree small" was re-derived on the
reverted tree to 65024 / 3251 from a count of 16256.

### The term

`src/search.cpp` `lmr_node_adjustment` gains a fifth input, `no_tt_move`, and a
fifth addend, `adjustment += LMR_NO_TT_MOVE` under that condition. The input is
**appended** after the four rather than slotted in beside the other additive
terms: the older four keep their positions, and two transposed bools are a bug
no type can catch. The sum does not care about the order.

The one call site is in `src/search.cpp` `negamax`, beside `tt_move_is_capture`,
and the condition is `tt_move == 0` -- the union of "no entry at all" and "an
entry whose move field is empty", which is the published condition and what
section 2 below reads the goal's "carries no move" as. Section 2 also verified
who writes moveless entries: `src/search.cpp` `negamax` never does (an assert
above its store says so) and `src/search.cpp` `quiescence` does, since S094, on
stand-pat beta, on mate and on a fail-low. At ply 0 the root hint can make the
condition false with the entry gone; neither consumer of the sum runs at ply 0,
so the hint decides nothing here.

Unclamped exactly as the four are -- the reduction is clamped against
`child_depth - 1` in the move loop and `src/search.cpp` `lmr_depth_of` clamps at
zero from below, and a helper that clamped would have to pick one.
`src/search.hpp` `search_lmr_node_adjustment_probe` gains the same input, so the
cases drive the rule as a function and the existing ones keep compiling.

Nothing else moved: no node-level depth cut (excluded), no change to the
reduction table, to the other four terms, to any range, or to any pruning rule.

### The seed, DEC-105 (c)

`LMR_NO_TT_MOVE`, `LmrNoTtMove` over UCI, in `src/search_params.hpp`'s
`CHESSO_SEARCH_PARAMS` list beside the four, default 1, range 0 to 2. The range
is the four terms' own stated purpose: 0 is off, 1 is the published class of
adjustment, and 2 is where the term alone equals what the table returns for the
first reducible move at the median depth. The arithmetic midpoint of that range
is 1 and that is the seed -- **form (c)**, stated as such at the constant's own
site. That every traced introduction of the older node-level form also shipped
one ply is a coincidence of that arithmetic and is not the provenance: a ply
count another engine ships is that engine's tuned output and is never a seed
here, wherever it is republished (DEC-084 as amended by DEC-105, DEC-134).
S127 refits it with the four beside it after the block; this step neither fits
nor sweeps it.

### The off value, DEC-215

0, and the site makes it inert by construction: the term is a single addend, so
at 0 the sum is the four-term sum and both consumers see exactly the number they
saw before this step. That is the same shape as the four beside it and **not**
the shape S098 verdict 3's margin had, where a range end was declared off and
the census found it still firing. It is proved on the tree rather than declared:
the tune build at `setoption name LmrNoTtMove value 0` benches the parent's
total to the node, and the completion note carries both numbers.

At the seed the node counts move by construction, so INV-6's discharge is not
available and the SPRT is what decides the step.

### The tests, DEC-141 clause 2

Three cases and three mutants, all in `tests/test_search.cpp` beside S098
verdict 2's own block except the mate row:

1. **The rule as a function** -- "the no-table-move term is one ply of its own
   on top of the other four". Through the probe: the adjustment with
   `no_tt_move` true is exactly `LMR_NO_TT_MOVE` more than with it false, at
   every setting of the four inputs around it that names a node type; an entry
   that carries a move gets no adjustment at all where nothing else fires; and
   at the declared maxima -- 2 for this term, read from `search_param_info()`
   rather than written a second time -- the shared helper still returns the raw
   table plus the whole sum, unclamped, which is the contract that makes the two
   call sites' different clamps possible. `REQUIRE(LMR_NO_TT_MOVE > 0)` opens
   it, because at the off value the two calls agree by construction and a term
   wired to nothing would pass.
2. **The rule at its site**, and the accepts' precondition case -- "a node whose
   table entry carries no move reduces its late quiets by LmrNoTtMove more".
   Three drives of one quiet position at ply 1 through S098's own fixture: no
   entry, an entry carrying a quiet move the position does not contain, and an
   entry carrying **no** move. The node with a move gets no extra ply; the node
   with none gets exactly one term more; and the moveless entry reads the same
   as no entry, which is the union the condition is written as and the half a
   site asking "is there an entry" would get wrong. **The precondition is
   established inside the drive itself**: the fixture now asserts that the
   planted entry is in the table and carries the planted move, and that a drive
   which plants nothing has no entry -- so a case whose plant did not land fails
   there instead of reading as a node of the other kind. That assertion is
   shared, so S098 verdict 2's own capture-entry case gains it too.
3. **The mate row** in `tests/test_search.cpp` "pruning does not hide a forced
   mate", beside the hanging-quiet position and not in the capture table:
   `4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1` at **depth 11**, a mate in
   2 whose key `Rc5` is a quiet rook move -- row 19 of
   `adocs/data/S145_mate_set.tsv`, proved by that file's own exhaustive search
   and agreed by stockfish at depth 20. **Observed red with the guard opened**
   (`const bool no_tt_move = tt_move == 0;` made `= true`, so the ply lands
   whether or not the entry has a move): the case fails at this row and nowhere
   else, `REQUIRE( result.mate_found )`, `values: REQUIRE( false )`,
   `logged: mate the extra ply hides, depth 11`, and it is green with the guard
   in place. Applied by hand, observed, reverted -- the S033 protocol; the log
   is `.tuning/coord/S095_observe_red.log`.

   The row is **mined, not chosen** (CHESS). `adocs/data/S095_mine_mate_row.py`
   imports S230's miner, reuses its pool of this project's own positions, its
   oracle calls and its `search_fen()` depth sweep, and adds one filter and one
   rule, both written before the sweep ran: keep a labelled mate only where the
   mating side plays a quiet, non-checking, non-promotion move out of check on
   the oracle's line, and take the lowest depth the shipped build reports the
   mate at and the unguarded build does not, tie-broken by the longest run of
   consecutive shipped depths. 297 labelled mates, 141 through the filter, 28
   separating the two builds, and the rule returned this one. What the rule
   bought and what it cost is in the case's own comment: the row's shipped
   profile is every depth from 3 to 12 and the unguarded build loses exactly
   one of them. Rows with a wider separation are in the same sweep and have
   shipped runs of one or two depths, which is what the tie-break was written
   to avoid; one of them was read and rejected on a ground the rule does not
   cover, its distance reading 5, 5, 6, 5 over its four depths.

The three mutants are `tools/mutants/S095_no_tt_move.py`, prefix `J`, **each
observed red by its own case** (`.tuning/coord/S095_observe_red.log`, applied
and reverted one at a time against a scratch build so the mate-carry grid
running beside it kept the binary it started with): `J01_no_tt_move_inverted` (the ply lands where the entry
**does** carry a move), `J02_no_tt_move_dropped` (the input reaches nothing; the
`(void)` is what keeps `-Werror=unused-parameter` from making the mutant
stillborn) and `J03_site_entry_absent_only` (the site asks whether there is an
entry instead of whether there is a move, the narrower published condition,
which exempts exactly the class quiescence writes).

`tests/test_search_params.cpp`'s golden table gains the row `LmrNoTtMove 1 0 2`
and its count moves from 50 to 51 -- DEC-142's deliberate-change detector, whose
derivation is `src/search_params.hpp` and whose re-derivation is a diff of the
two. `MANUAL.md` gains the option row the tune build now advertises, which
`test_uci_surface` requires by name.

### The measurement

`adocs/data/S095_sprt.sh`, written before a game was played: one gainer SPRT at
`{0, 5}` nElo, the harness regime (8+0.08, Hash 16, `books/noob_3moves.epd`,
concurrency 12), against the commit the landing sits on. DEC-143's cost is in
its header -- 41861 expected games at the interval's midpoint and 25591 on a
bound, 19.8 h and 12.1 h at the machine's measured 2110 games an hour -- so it
is a night and the coordinator schedules it (DEC-155). The abort rule, the open
findings it is taken while open, and the three outcomes are there too. **It
refuses to run until both shas are pinned**: the landing commit did not exist
when the file was written, and a reference guessed from `HEAD~1` is a reference
nobody checked (DEC-020).

The outcomes, in the pre-registration's own words: H1 keeps the term at its
seed and S127 fits it later; H0 records the zero and removes the term, which is
a behaviour-neutral revert here because the off value has already proved the
tree is the parent's, and the node-level cut then becomes available as a later
step with its own file; no verdict is recorded as a zero and decided with the
reason stated.

### What the term did to the suite, measured 2026-09-20

The term changes the reduction at every node whose entry carries no move, which
is **every node of a cold-table drive** -- and the fast suite is full of those.
Five cases went red on the first build and none of them was weakened to get
green; each is recorded here with what was measured and what moved.

- **S098 verdict 2's four term cases** (`tests/test_search.cpp` "a node
  expected to fail high reduces its late quiets by LmrCutNode more" and the
  three beside it). Their two drives planted no entry, so S095's term was on in
  both: the cut-node and not-improving cases hit the clamp at `child_depth - 1`
  (`REQUIRE( 4 < 4 )`), and the capture case broke outright, because planting an
  entry now switches S095's term *off* while switching LmrTtCapture *on* and the
  difference became the sum of the two, which is 0 at the shipped values. The
  fix is the block's new `QUIET_ENTRY`, a quiet move this position cannot
  generate, planted in **both** drives of all four cases: the fifth condition is
  then equal and off on both sides, each case measures its own term again, and
  the node sits at the reduction level S098 wrote the cases at.
- **S109's two capture cases**. Both read `lmr_depth_of`, the test's mirror of
  the engine's gate, whose `node_adjustment` defaulted to 0 "because every call
  site drives an ALL node at ply 1 with an empty table". That is exactly where
  S095's term now fires, so the default is `LMR_NO_TT_MOVE` and the comment
  says why. The two drive depths moved with it, each measured rather than
  stepped until green (`.tuning/coord/S095_probe_facts.cpp`): the cap case needs
  its capture searched past the cap and `f3f6` is skipped at every depth up to
  11 and searched at **12** with a reduced depth of 8, so `CAP_DRIVE_DEPTH` is
  12; the margin case needs the rule's own bar above the safe capture's
  exchange value, `g2h3` is worth between 100 and 150 by the engine's own
  `see_ge` and the bar is 50 a ply, so it needs a reduced depth of 3 and its own
  `SEE_MARGIN_DRIVE_DEPTH` of 5. No assertion of either case changed.
- **`capture_mates`, re-derived** (DEC-142). Seven sweeps -- shipped and all six
  mutants of `tools/mutants/S091_capture_see.py`, depths 3 to 12 over
  `adocs/data/S230_table_fens.txt` through `adocs/data/S230_mine_r01_row.py
  depths`; evidence `.tuning/coord/S095_capmates_*.txt`. Three of the four
  depths moved and no mate distance did: 9, 8, 11, 10 becomes **9, 9, 10, 10**,
  rows 1 and 2 separate no S091 mutant and rows 3 and 4 keep R02. The table's
  own rule picked every row.
- **`test_mate_carry` "a mate score carried across searches keeps a line that
  reaches it"**, and this one is a finding rather than a repair: `E_mate_minus9`
  reports **11 short mating PVs of 22 mate lines against its ceiling of 9**. A
  short line is DEC-122's expected residue -- the walk found no entry it could
  certify, so the line stays as the search produced it -- and the ceiling is
  the worst cell of the recorded grid. The guarantee beside it holds: no line
  published at its claimed length fails to end in mate, `unreached.empty()` is
  true on every case. The protocol is to re-run `adocs/data/S203_case_sweep.sh`
  before deciding whether the walk regressed or the case moved to a cell the
  grid does not cover, and **never to raise a ceiling to match a run**.

  The grid was re-taken whole on this tree -- 108 cells, six cases by two
  strides by nine budgets, 22 minutes -- and recorded as
  `adocs/data/S095_sweep_block.txt`, the fourth grid after S204's two and
  S109's. Read with `S203_case_sweep.sh --ceilings` over all four, the rule's
  own answer is **A 5, B 15, C 0, D 2, E 11, F 5** against the 5, 11, 0, 2, 9,
  5 the test holds: two ceilings rise, E's by 2 and B's by 4, and B is green
  at its own cell today. At E's cell -- stride 1, 1500000 nodes, the one the
  test drives -- this tree reads 22 mate lines with 11 short where S109's grid
  read 13 with 9: the absolute count rises past the ceiling and the share of
  short lines **falls**, 50 % against 69 %. That is the reading the decision
  needs and it is not the agent's to take: **raising a ceiling is relaxing a
  test and needs a decision** (DEC-162, and S109 is the precedent for a block
  that owed exactly this one). Nothing in `short_line_ceiling()` was touched.

  It blocked two things while it was open: the suite was not green, so the
  landing commit could not be green either (COMMITS), and
  `tools/mutation_check.py` refused to start -- "the unmutated worktree is red:
  1 of 40 failed (test_mate_carry)" -- which is the guard working, since a
  suite already red cannot say whether it saw a mutant.

  **Decided 2026-09-20, DEC-225, by the coordinator**: raise the two ceilings
  to exactly what `S203_case_sweep.sh --ceilings` answers over the four
  recorded grids, E to 11 and B to 15, and nothing else. **The numbers were
  read off the grid by the script and never off the failing run** -- that is
  the golden's own rule and S109's precedent for a block that owed the same
  decision. The grounds are the guarantee beside them: `unreached.empty()`
  holds on all six cases at their own budgets and strides in both builds, so
  what rose is DEC-122's expected residue under a rule that reduces more and
  not the promise, and S202 still owns closing the class. The golden's site
  carries the dated paragraph, the four-grid re-derivation line, the two worst
  cells by budget and stride, and the correction of a header line that had
  been stale since S109.
- **"ordering keeps the tree small"**: the count reads **22217** against the
  16256 the band `[3251, 65024]` was derived from on the reverted tree. The
  middle half of that band is [18694, 49581], so the count is inside it and
  DEC-142's re-derivation trigger has not fired. Read, not moved.

### The off value, proved twice on the tree

`bench` at the parent, `b25452a`, is **4646334**. The tune build at
`setoption name LmrNoTtMove value 0` prints **4646334** and a Release rebuild
with the X-macro default forced to 0 prints **4646334**, both to the node. At
the seed the Release build prints **4579468**, 1.4 % below the parent, which is
the tree the SPRT measures.

**One thing went wrong while this was measured and is recorded rather than
tidied away**: `build-tune` was built while the capture-mates sweep held
`src/search.cpp` under a mutant, so its first bench, 4472633, was a build of
mutated source. It was rebuilt on the clean tree and agrees with the Release
build at 4579468. Nothing else was built in that window and the sweep restores
and hashes the file after every mutant, so nothing else can carry it -- but a
build taken during a run that edits the tree is worth nobody's trust, and the
number is quoted here so the mistake is on the record and not only the
correction.

### Proposed sentence for `adocs/specs.md`'s search row

The coordinator owns that file; this is the sentence this step proposes, to sit
after S098 verdict 2's, with the verdict filled in at completion:

> **S095, 2026-09-20 (DEC-222 clause 2)**: the node adjustment gains a fifth
> term, so the reduction is
> `r + LmrCutNode + LmrNotImproving + LmrTtCapture + LmrNoTtMove - LmrPv`.
> `LmrNoTtMove` is added where the node's table entry carries no move at all --
> no entry, or one quiescence wrote without a move, which since S094 is the only
> way a moveless entry is made -- and it is 1 with range 0 to 2 (DEC-105 (c)),
> 0 being an off value at which the sum is the four-term sum and the engine is
> the one before the step, bench signature included (DEC-215). The step was
> written as a node-level depth cut and re-formed to this before it was
> implemented; that cut stays available as a later step. Decided by one `{0, 5}`
> nElo SPRT, `adocs/data/S095_sprt.sh`: <verdict>.

### What the held machine left owed

Taken the moment it came free, in this order, and all of it done except the
last line: both builds and the fast suite green plus the format check; the
off-value bench against the parent's and the seed's own bench;
`tools/search_bench.py` at depths 9 and 12, parent to seed; the mate row mined,
observed red under the opened guard and green under it; `tools/mutation_check.py`
over `tools/mutants/S095_no_tt_move.py`; `DEV_MANUAL.md`'s bench ledger gaining
this landing with what moved it. **What is left is the coordinator's**: Debug
self-play and `tools/gate_extra.sh` on the landing commit (DEC-141 clauses 1
and 3), pinning `CAND` in `adocs/data/S095_sprt.sh`, and the run itself.

**Three goldens move with any change to reduction and are re-derived by their
own scripts, never re-read (DEC-142), which is what S098's five verdicts and
S231 each did:**

- `tests/test_search.cpp` `capture_mates`, its four depths and their mutant
  labels. Seven sweeps -- the shipped build and all six mutants of
  `tools/mutants/S091_capture_see.py` -- over `adocs/data/S230_table_fens.txt`
  through `adocs/data/S230_mine_r01_row.py depths`, and the same rule applied
  by the same script rather than by eye.
- `tests/test_search.cpp` "a reduced move that beats alpha is searched again",
  whose position and depth are a measurement: `adocs/data/S231_research_witness.py`
  is its script and its rule. Re-derived here, and the case is green -- the
  pinned position re-searches 8 of its 26 reduced moves at the pinned depth 6.
  **The script's rule answers depth 4 and not 6**, and that is not this step's
  doing: S231 moved the pin to 4 and its H0 revert took the move back with
  everything else, so the reverted tree already pins a depth its own rule does
  not choose. Recorded as a finding and named in this step's SPRT header
  rather than repaired inside a landing an SPRT is about to judge.
- `tests/test_search.cpp` "ordering keeps the tree small", read with
  `adocs/data/S192_node_budget.py`. Its band was re-derived to 65024 / 3251 on
  S231's reverted tree, from a count of 16256, so the trigger this step has to
  answer is whether the term takes the count out of the middle half of that
  band. **Read, not moved**: a re-derivation inside a landing an SPRT is about
  to judge is what S231's own pre-registration refused, and the two findings
  that re-derivation left open are named in this step's SPRT header instead.


## What it replaces

The older technique is internal iterative deepening: search the node to a
shallower depth first, purely to get a move to order with. The reduction is the
modern form -- if there is no table move, the node is cheaper than its depth
claims and is searched one or two plies shallower instead. Both are documented;
this step implements the reduction and measures it, and the deepening form is
out of scope unless the measurement says otherwise.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol if drifted. Every GitHub read
below was a commit message, PR body or release note, never a diff or source
file (DEC-016, DEC-084). rebel13.nl (ProDeo's own writeup) answered HTTP 526
in this pass and is cited from nothing; the talkchess thread is the primary.

### 1. State of the art

**Lineage.** IID (CPW: Scott 1969; Anantharaman 1991 for Deep Thought — kept
there "despite it being pretty much a washout on average"): no hash move at a
PV node -> run a reduced search of this node purely to get one, then search at
full depth. The modern form keeps the signal and drops the purchase: **no hash
move -> cut the node's whole depth**, "in the hope that the node must not be
very important as there was no hash move present" (CPW IIR). Ed Schroder
introduced it in Rebel — talkchess 2020-08-13, "An alternative to IID": IID
out, "a full ply reduction without research", **+17 after 5000 games**, about
two plies more depth at fixed time (12.67 -> 14.49); an independent tester
confirmed +13/2.5k, +14/7.5k.

**Why reduction won, in the record.** Weiss added classic IID in 2019 at ~0
STC / +4.11 LTC (#113) and replaced it with the Rebel form two days after
Ed's post for **+8.02 +/-4.95 STC / +5.61 +/-3.76 LTC** (#316, 2020-08-15).
Stockfish deleted IID as a *simplification* the week after — non-regression
bounds — "employ a depth reduction if the position is not in TT and on the
PV" (e64b957, 2020-08-21). Ethereal had removed IID outright at 11.16
(2019-01) with nothing in its place. The deepening's shallow search
duplicates what the outer iterative-deepening loop and the table already do.

**Conditions, precisely.** Two published triggers: **entry-absent**
("position is not in TT", SF's first form) and **move-absent**
(`ttMove == 0`, which also fires on an entry whose move field is empty).
Lynx measured the widening entry->move at **+11.63 +/-5.25** (#1516) — the
union is the stronger published condition. Node types: Rebel used all node
types (CPW); Lynx ships that and measured every restriction negative in its
stack (PV-only **-45.38/-46.88**, #2024/#2026; pv||cut **-19.75/-21.08**,
#2027/#2220); Weiss went PV-only for +1.67 STC at ~3300 (#569); SF began
PV-only, Chaly extended to expected cut nodes in 2021 (CPW prose — commit
untraced publicly this pass); Ethereal built it in reverse order, cutnodes
first (**+4.39/+2.88**, 3071b40, 2023-04), PV later (+1.37/+2.08, 9243eae).
Direction is stack-dependent — DEC-019. Threshold: Lynx introduced at **min
depth 4** (#507), 4->5 +0.43 over 109k games, 5->6 0.08 — flat above 4; CPW
offers "depth > 5, say"; SF simplified its depth condition away at ~3800
(3747a19). Amount: **1 ply** at every traced introduction; 2 plies measured
**-12.38** (#1361) and **-46.23** (#2028) at Lynx.

**Traced introductions.** Rebel +17/5000. Weiss #316 +8.02/+5.61 (~3000
era). Berserk #89 "Replace IID with Ed's IIR" **+6.74 +/-4.67** at 8+0.08
(7ba082b, 2021-05). Lynx #507 **+9.5 +/-6.8** LTC, v1.1.0 (2023-12) — Lynx
1.0.1 = 2432 CCRL, the one introduction below chesso's band. Placement:
before the forward-pruning block — Weiss #450 **+2.87/+2.37**, Berserk #140
**+2.17** ("less depth to be hit by more pruning mechanisms"); after-pruning
re-tried at +0.44 ~0 (Lynx #2195, ~3100+). Root: Lynx allows it, **+1.39
+/-1.05** over 170k games (#2025). Refinements at 3400+, noted not targeted:
reduce more when the entry's depth lags the search depth (Ethereal 0850fdd
+2.86/+3.31; SF cc992e5), cutnode-with-upper-bound-move (SF db147fe), later
pulled back for "poor scaling at longer time controls" (SF 55cb235, 8b32e48,
40e0486). Stash: no IID/IIR commit traced at all.

### 2. Shape for chesso

- - The condition: tt_get_entry (`src/search.cpp` `negamax`) returns nullptr on
  a miss (`transposition_table.cpp` `tt_get_entry`), and `src/search.cpp`
  `negamax` already computes `tt_move = (tt_entry != nullptr) ?
  tt_entry->best_move : 0`. **The probe exposes both variants today** —
  `tt_entry == nullptr` is entry-absent, non-null with `best_move == 0` is
  entry-present-move-empty — and `tt_move == 0` is their union, the published
  winning condition, one variable already in scope. Verified who writes
  moveless entries: `src/search.cpp` `negamax` never -- an assert above its
  store says so; **quiescence does**, since S094 -- `src/search.cpp`
  `quiescence` stores on stand-pat beta, on mate and on a fail-low, all at
  TT_DEPTH_QS. So "entry with no move" concretely means "only quiescence has
  resolved this position" — the unimportance signal the technique prices. The
  goal's "table entry carries no move" is read as the union: a missing entry
  carries no move either.
- - The site: node level in `src/search.cpp` `negamax`, after the TT-cutoff
  block and the quiescence drop and before the RFP block — before all forward
  pruning, the placement with the two positive records. Cut `depth` once;
  everything downstream in the same function reads the reduced depth by
  construction: the RFP gate, the null-move formula, S109's move-loop gates,
  S098's table row via `lmr_reduction(depth, ...)`, `child_depth`, and the
  store, which then records the depth the node was actually searched to.
- - No ply guard: the root re-stores its entry with a move every iteration, so
  from iteration 2 it has a TT move — and Lynx's allow-root record is +1.39. No
  in-check condition either (the published simple form has none; is_in_check
  `src/search.cpp` `negamax` sits below the site anyway). Quiescence is
  untouched — no depth to cut (qply cap only), and no published implementation
  reduces there. No IID remnant exists in the tree (grepped; specs.md "absent,
  search" lists IIR as absent) — the excludes line is scope, not a deletion.

### 3. Implementation sketch

1. The change is the three lines plan.md priced:
   `if (depth >= IIR_MIN_DEPTH && tt_move == 0) depth = std::max(depth -
   IIR_REDUCTION, 1);` at the site above. The explicit floor at 1 is the
   depth<=0 guard: no reduction may create a fall-through to quiescence (the
   repo's recurring bug class — null move's depth-0 mate miss). At the seeds
   the floor never binds; it exists so no SPSA value can open that path.
2. Tests, red first. The harness is the exposed state by construction:
   search_fen (`tests/test_search.cpp` `search_fen`) resets the table and makes
   one fixed-depth search() call, so every node starts with tt_move == 0.
   - The accepts' precondition pair: cold-table search at fixed depth >=
     IIR_MIN_DEPTH, node count recorded; then pre-store an entry with a move
     for the root position (a prior shallower search through search_fen_with
     suffices) and assert the with-move count equals the feature-off count —
     the "entry with a move must not be reduced" side, failing if the
     precondition is absent.
   - The accepts' mate case: a forced mate inside the reduced depth added
     beside "pruning does not hide a forced mate", `tests/test_search.cpp`
     "pruning does not hide a forced mate" and "pruning does not hide a mate
     against the material leader", `tests/test_search.cpp` "pruning does not
     hide a mate against the material leader", observed red with the guard
     removed (threshold to 0 locally, observed, reverted — the S033 protocol;
     cold fixed-depth search is where it bites).
   - Both existing mate suites re-run; fast suite green. Node counts move by
     construction — INV-6 takes the SPRT path; search_bench depths 9/12 in
     the stamp.

### 4. Constants and seeds

**Superseded 2026-09-19 (DEC-222): the two constants below belong to the node-level form; the re-formed step has one term in `lmr_node_adjustment`, range 0..2, off 0, seeded at form (c). Kept as record.**

Both in `src/search_params.hpp` (the `CHESSO_SEARCH_PARAMS` X-macro) with
stated ranges; each is a **seed — must be fitted/SPSA'd here** (S127). Under
DEC-105 every seed is one of three forms and says which: **(a)** a value from
a publication about the technique, with its URL; **(b)** a derivation over
chesso's own data or scale; **(c)** the range midpoint or off value, stated
as such. A ply count another engine ships is that engine's tuned output and
is never a seed, wherever it is republished — those records live in section 1
and, as anti-seeds, in section 5.

- `IIR_MIN_DEPTH` **6** — **(a) literature.** The wiki's own page states the
  condition as a hypothetical with no engine attached: "only use IIR if depth
  > 5, say", https://www.chessprogramming.org/Internal_Iterative_Reductions
  (fetched 2026-09-05). Section 3's guard is `depth >= IIR_MIN_DEPTH`, so
  "depth > 5" is **6**. Range 2..63 stands: 2 keeps the floor trivially true,
  63 is the off value.
- `IIR_REDUCTION` **1** — **(c) midpoint.** The wiki states no reduction
  amount, so no (a) exists, and a ply count has no unit to derive from
  chesso's own scale. Range 0..3, 0 = off; the arithmetic midpoint 1.5 is not
  an integer, so the seed is **1**, the integer below it, and the sweep
  decides. That this is also the amount every traced introduction shipped is
  a coincidence of the arithmetic, not its provenance.
- No cutnode term: chesso gains a cut_node input only if S098's verdict 2
  shipped, and the cutnode variants are 3300+ records in both directions —
  S127-era material, not this step's.

seeds re-derived 2026-09-04 under DEC-105 (DEC-134)

### 5. Pitfalls

- **Anti-seeds — records, not seeds.** Section 1 carries them and they stay
  there: Lynx introduced the threshold at min depth 4 (#507) with 4->5 +0.43
  and 5->6 +0.08 above it, and measured a 2-ply reduction at **-12.38**
  (#1361) and **-46.23** (#2028); SF simplified its depth condition away at
  ~3800 (3747a19). Under DEC-019 a record like that says which direction is
  worth trying and never what to start from, and under DEC-105 none of these
  numbers may seed the sweep — which is why section 4 names no engine.
- **Stacking with S098/S091 is in series, not additive.** IIR cuts `depth`
  once at node level before the move loop; S098's reduction and S091's extra
  ply are per-move cuts on child_depth inside it. The per-move machinery
  reads the already-reduced depth (table row, S109 gates, clamps) and needs
  no edit. What is forbidden is a second per-move no-TT-move term inside
  LMR: Lynx measured that duplicate at **-8.08** with IIR present (#2253);
  S098's file carries the same warning from its side.
- - **Re-visits do not re-reduce forever here.** The reduced visit stores its
  entry at the reduced depth *with a move* (`src/search.cpp` `negamax`); the
  next visit at the same nominal depth finds the move — no cutoff, stored depth
  one short (`src/search.cpp` `tt_entry_answers`) — and is not reduced. The
  loop terminates because every negamax store carries a move. Residual re-fire
  paths — a slot lost to a collision (key mismatch reads as no-entry), a
  quiescence store recapturing the slot across a `go` boundary (in-generation
  it cannot: `transposition_table.cpp` `tt_store_entry` replaces only at depth
  >= entry->depth, and TT_DEPTH_QS loses to any main depth) — cost 1 ply once
  per visit, bounded. The published mitigation for the saturated-table case is
  the depth threshold (Ed's stated concern on talkchess), and S105's Hash 16
  regime is deliberately high-pressure, so the threshold is load-bearing here,
  not decorative.
- **S097 needs a TT move by definition — no overlap.** Singular extension
  verifies a node whose entry has a move and sufficient depth/bound; IIR
  fires only where tt_move == 0 — mutually exclusive at a node. The one real
  interplay, IIR firing *inside* the verification search's subtree, is
  S097's edit, records both ways: Berserk +3.64 for disabling IIR during SE
  (#91), Lynx +0.09 ~0 for the same (#1734).
- **The mate hazard is one iteration wide, and the floor is pinned.**
  Reduce-1 above the threshold keeps depth >= 1 always — never into
  quiescence. What remains is a mate at a reduced node's horizon arriving
  one iteration later; iterative deepening plus the stored move heal it in
  play, and the cold-table fixed-depth mate tests are where a wrong floor or
  threshold shows.
- - **Store the reduced depth.** Mutating `depth` before the loop makes
  `src/search.cpp` `negamax` store it correctly; cutting a copy and storing the
  original would claim depth the node never searched and poison deeper cutoffs.

### 6. Measurement

One SPRT at the S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`,
against the commit before it; fast suite and both mate cases green first, the
new tests observed red first with printouts recorded. Expectation (DEC-019,
direction only): introductions at or near the band measured +6.7..+9.5
(Berserk at 8+0.08, Lynx LTC at ~2450, Weiss STC). A zero is recorded as zero
and dropping three lines is the default outcome at zero (S005/S006/S015).

### 7. Interactions

- **S098 (before)**: composes in series, node-level before per-move; no
  per-move duplicate term — its file and section 5 both say so.
- **S097 (after)**: TT-move dependency makes the node conditions disjoint;
  verification-subtree behaviour is S097's decision, records in section 5.
- **S119 (later, note only)**: buckets and aged replacement change how often
  a probe finds an entry and whether its move survived, so the IIR fire rate
  moves with the table layout; S127 re-tunes the two constants after it.

### 8. References

- - https://www.chessprogramming.org/Internal_Iterative_Reductions — lineage
  (Schroder, Rebel 2020), all-node-types origin, SF PV-only then Chaly cutnodes
  2021, "depth > 5, say", the entry-depth refinement idea.
- - https://www.chessprogramming.org/Internal_Iterative_Deepening — Scott 1969,
  Anantharaman 1991, classical PV-only conditions, reduction amounts, Deep
  Thought's "washout on average".
- - https://talkchess.com/viewtopic.php?t=74769 — Ed Schroder, "An alternative
  to IID", 2020-08-13: +17/5000, full ply without research, ~2 plies depth
  gain, TT-saturation concern; silentshark +13/+14.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+IIR+in:title+type:pr
  — #507 +9.5 (min depth 4); #1516 !ttHit -> !ttHit||!ttMove +11.63; #2025 root
  +1.39; #2035 depth 4->5 +0.43; #2236 5->6 0.08; #1361/-12.38 and #2028/-46.23
  reduce-2; #2024/#2026 PV-only -45/-47; #2027/#2220 pv||cut -20/-21; #1237
  pv&&cut -6.73; #2195 after-pruning +0.44; #1734 SE +0.09; #1524..#1534
  ttDepth offsets ~0.
- - https://api.github.com/repos/lynx-chess/Lynx/releases/tags/v1.1.0 —
  2023-12-14, "Add Internal Iterative Reduction (IIR) (#507)".
- - https://api.github.com/search/issues?q=repo:TerjeKir/weiss+IIR+type:pr and
  +IID — #113 IID 2019 (~0/+4.11); #316 "Rebel IID" 2020-08-15
  +8.02/+5.61/+3.83; #450 before-pruning +2.87/+2.37; #569 PV-only +1.67 STC,
  LTC unresolved.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22internal+iterative%22
  — e64b957 2020-08-21 removes IID, "depth reduction if the position is not in
  TT and on the PV", non-regression; 8dea070 before-probcut 2023.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22IIR%22
  — 6d0d430, db147fe, 55cb235 ("poor scaling"), 8b32e48, 40e0486, 9cc15b3,
  3747a19 (depth condition simplified away), cc992e5 (entry depth >= search
  depth, reduce more).
- - https://api.github.com/search/commits?q=repo:jhonnold/berserk+IIR — 7ba082b
  #89 "Replace IID with Ed's IIR" +6.74 at 8+0.08; b81aa1d #91 disable during
  SE +3.64; 9c1b743 #140 before pruning +2.17.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+IIR —
  3071b40 "Introduce cutnodes, and perform IIR on them" +4.39/+2.88 (2023-04);
  0850fdd tt-depth lag +2.86/+3.31; 9243eae PV +1.37/+2.08.
- - https://api.github.com/repos/AndyGrant/Ethereal/releases — 11.16 (2019-01)
  "Remove Internal Iterative Deepening, apply Singular Extensions
  aggressively".
- -
  https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+%22iterative%22
  — negative result: no IID/IIR commit traced.
- - https://rebel13.nl/prodeo/prodeo-3.0.html — unreachable this pass (HTTP
  526); not used.
