id:         S145
goal:       a mate-safety test set built for this engine, spanning the plies the guard actually covers, so the floor that fences the tuner rests on evidence rather than on three positions and one motif
accepts:    a constructed set of at least 20 positions, each a forced mate verified by **two** independent tools (exhaustive enumeration through `python-chess` and `stockfish`, or a Syzygy probe where the material allows) and each reachable by `position_is_reachable()`, spanning mate distances 2 through 5 so plies 1, 3 and 5 are exercised rather than ply 1 alone; every position built by the S033 method -- start from a verified short mate, add legal material to the mated side until its static score clears `RFP_MARGIN * depth`, keep the key move quiet -- and the construction recorded as a script under `adocs/data/`, not as a list of FENs whose derivation is lost; the set asserted **under iterative deepening at a depth above the minimum**, because a fixed-depth cold-table call at exactly `2m - 1` cannot tell a lost mate from a one-iteration delay and the existing cases conflate them; `RFP_MIN_PLY` and `RFP_MAX_DEPTH` each measured against the set **with the other held**, so the floor and the ceiling are separated rather than substituted; a mined breadth set built from `.spsa/S085/games.pgn` at one position per game, labelled by `stockfish`, scored as a **count with a floor** and not per-position pass/fail; `fastchess.sh` passes `-check-mate-pvs`; the fast suite green and no default changed
touches:    tests/test_engine.cpp, adocs/data/, fastchess.sh, DEV_MANUAL.md, MANUAL.md, adocs/specs.md, adocs/testing.md
            # amended when the plan met the code, 2026-08-21. It said tests/test_search.cpp,
            # and it could not be: the accepts requires the set asserted under iterative
            # deepening, and iterative_deepening_search() lives in src/chesso.cpp and reaches
            # a test only through the UCI surface, which test_search does not link. The new
            # suite therefore sits beside the deepen() harness the aspiration cases already
            # use in tests/test_engine.cpp. MANUAL.md, specs.md and testing.md are the
            # completion checks landing, and the two documents each carried a stale reverse
            # futility constant -- depth 6 and margin 75, both moved by S085 -- which this
            # step's own subject made impossible to leave. This is the class S141 exists for.
excludes:   changing `RFP_MIN_PLY`'s or `RFP_MAX_DEPTH`'s declared bounds, which is S142's and which this step supplies the evidence for; changing any default; mate distance pruning, which the research below shows is a speed and analysis feature and not a guard against this hazard, and which would need its own step and its own verdict; any external position file
decisions:  DEC-095, DEC-016, DEC-019
closes:
blocks:     S142
paused_by:
done:      48 proved mates spanning distances 2 to 5, both oracles, verify clean; the floor separates at 2 and the deep classes belong to the ceiling; fast suite 18 of 18 in 14.89 s

## Why S142 is paused behind this

S085's SPSA run walked `RFP_MIN_PLY` to its illegal end and stayed: 906 of 1250
iterations at 0, 82 at 1, 242 at 2, 20 at 3. The only thing standing between the
tuner and that value is three mate cases, and the owner's objection was that he
picked those positions for a different engine. Setting the bound first and
validating the evidence afterwards would fence the next tuning run with a number
whose justification arrived late.

## What the research found, 2026-08-21

Fourteen engines surveyed at pinned commits, plus Stockfish's git history, plus
measurement on this engine. Sources are prose and documentation; nothing was
copied (DEC-016).

### 1. The gate is real, and the axis it guards is worth almost nothing

Both at once, and they are not in tension.

**The axis is noise.** 6347 positions -- all 1347 `mate=1` rows of
`adocs/data/S018_raw.tsv`, a phase-stratified 1000 from the rest, and a uniform
reservoir sample of 4000 from `.tuning/selfplay_v2_dedup.tsv` -- driven at depth
12 through the tune build:

| floors | node sum | median | geometric mean |
|---|---|---|---|
| 1 / 3 | 0.9952 | 0.9996 | 0.9826 |
| 2 / 3 | 1.0003 | 1.0000 | 0.9923 |
| 4 / 3 | 1.0563 | 1.0423 | 1.0700 |

The whole 1-to-3 axis is **0.5 % of nodes by sum and 1.7 % by geometric mean**,
inside CLAUDE.md's own "under 3 % is noise". Floors 2 and 3 are
indistinguishable at 1.0003, so **the owner's choice of 2 is free**. The cliff is
at 4, costing 5.6 %, which is S033's shape at a thousand times the sample.

**So the trajectory needs no strength in it to be explained.** 0 and 1 are
byte-identically the same engine, so 988 of 1250 iterations sat at one effective
value with a degenerate neighbour, against a clamping boundary, on an axis worth
half a percent of nodes. That is boundary absorption of a null gradient, not a
preference. And S085's verified +21.02 Elo was measured with `RfpMinPly` held at
3 -- the tuner's own answer on this axis was never SPRT-tested.

### 2. A representative sample cannot decide this floor

**Corrected by measurement, 2026-08-21.** This section was half right and the
half that is wrong is worth stating plainly, because it was the argument for
building the set. The claim "there is no sample size that fixes this" was
reached from node counts and from a 61-position sensitivity frame, and it does
not survive a mate-finding count: the mined set of 318 positions reads **146
exact at `RfpMinPly` 2 and 3 and 139 at 1 and 0**, so a mined sample does see
the boundary that matters. What it still cannot do is separate 2 from 3, which
is the part below that holds. The constructed set is what separates the deep
classes and what supplies the per-ply coverage, and that justification stands on
its own -- the sample argument was not needed and should not have been stated as
absolute.

Measured, and it is the finding that shapes the whole step. The hazard needs a
quiet non-PV node whose static score clears a displaced beta by
`RFP_MARGIN * depth` while the node is a forced loss -- a structured coincidence.
Of 191 positions in the sample where this engine says the side to move is mated
within 6, **one** has a non-negative score for the mated side; the median is
**-1093**; **none** reaches the +500 the S033 construction used. On material over
the mate-in-4-or-less subset, **0 of 140** are ahead by 300 or more. When this
engine is being mated it is down a rook and a half.

Confirmed directly on the most sensitive frame available: the 61 positions of
6347 where the four floors disagree at fixed depth, adjudicated by `stockfish` at
depth 26 and re-run under real iterative deepening to depth 16. Every pairwise
McNemar exact test is **non-significant, p >= 0.388**, and the discordant pairs
split nearly evenly both ways. On a population pre-selected for sensitivity the
floor is undetectable, and an unselected sample would be flatter.

**There is no sample size that fixes this**, because the failure mode is absent
from the frame. Hence a constructed set, which is what S033 did and what the
accepts asks to be generalised. The mined set is still worth building -- for
breadth against future parameter changes and for mate finding generally -- but it
is a different job with a different scoring rule.

### 3. What the existing three actually cover

`:125` and `:1887` are **the same two FENs, the same depth loop and the same
assertions**; they differ in title, comments and failure strings. The
material-leader position at `:1926` is `MATE_IN_2_B_POS` with White material
added -- same black king on e5, same black queen on a7, same white king on e8,
same key `e5e6`. So three positions are **one geometry, mirrored, plus one padded
variant**, and all three are mate-in-2, which exercises **ply 1 and nothing
else**. `src/search.cpp:517` concedes that no test covers ply 2; that is why
floors 2 and 3 are indistinguishable on everything currently tested.

Provenance, from git, correcting DEC-095's premise: the two `MATE_IN_2` positions
enter at `3ed3b11` and are on `master` and `bitboard` as well, so the objection
holds for them. The material-leader position enters at `6bd650e` on `achesso`
only, **built during S033 for this hazard** -- and it is the only one of the three
that carries the gate.

### 4. The floor and the ceiling are substitutes, and the suite cannot separate them

Measured under iterative deepening on the material-leader position, `go depth 14`:

| `RfpMinPly` | `RfpMaxDepth` 6 | `RfpMaxDepth` 15 |
|---|---|---|
| 1 | mate at iteration 8, plays `e5e6` | **never to depth 14, plays `e5f6`** |
| 2 | iteration 3 | iteration 3 |
| 3 | iteration 3 | iteration 3 |

At floors 0 and 1 the mate first appears at exactly `RFP_MAX_DEPTH + 2`, because
RFP fires at ply 1 while `depth - 1 <= RFP_MAX_DEPTH`. S033 shipped 6, so a low
floor cost five iterations and the mate was still found; **S085 tuned it to 15,
which turns the same defect into a permanent miss** at a median search depth of
11. Shipping is safe and the owner's floor of 2 is safe -- both find it at
iteration 3 -- but 15 is safe only *because* the floor masks it, and the two
values are one guard.

This is exactly where the published record points. **Zero of fourteen surveyed
engines has a ply floor on reverse futility**; every gate in practice is
depth-remaining, and `RFP_MIN_PLY` appears to be unique to this engine. Stockfish
removed its RFP and parent-futility depth limits in July 2021 (`09b6d283`,
`dbd7f602`), **both passed non-regression SPRT at STC and LTC**, and mates found
on ChestUCI at 1M nodes fell from **2427 to 1246**; restoring either condition
alone recovered only 1282 and 1630, so both were needed. Reverted at `dabaf222`,
and the source has carried "The depth condition is important for mate finding"
ever since. Master now makes the ceiling *dynamic* -- a table returning 19 at
small scores decaying to 13 as scores approach the decisive band -- so the
pruning depth shrinks where mates live.

### 5. Two guards that are not guards here

**The beta mate-band condition is inert in this engine.** `evaluate_expensive()`
is clamped to `+/-LAZY_EVAL_MARGIN`, so the static score cannot approach a mate
score, so `static_score - margin >= beta` cannot hold with beta in the band --
`src/search.cpp:512-517` already says this. Only 3 of 14 surveyed engines guard
on beta at all; Stockfish carried exactly this condition through SF5 and
**deleted it** in `27a1877` on the argument that the guard that matters is on
alpha, with a *better* Chest mate score after removal. S033's own trace confirms
it: the harmful prune had `beta = -964`, nowhere near the band. It costs nothing
and it protects nothing, and no step should be written believing it does.

**Mate distance pruning is not a defence against this.** 12 of 14 engines have
it; CPW is explicit it adds little playing strength and helps analysis and mate
solving. Its clamps are mate-band values, so they bind only when alpha or beta
is already in the band -- precisely where this engine's RFP is already off. This
engine has none, and adding it would not touch this hazard. Excluded above for
that reason, not deferred.

### 6. How the practice tests this, and the free check we are not using

**A mate-safety unit test is a minority practice, and SPRT does not catch these
bugs** -- across roughly thirty verified mate bugs in the survey, SPRT caught
none; it appears only as the gate on the fix. Of seventeen engines, nine have no
CI, four compile and re-grep a bench node count, four run assertions, and
**three assert a mate result**. Two projects wrote exact mate-distance tests,
watched pruning break them, and **switched the tests off rather than the
pruning** -- one marks them explicitly skipped with a "requires no pruning"
category, the other deleted its CI step as flaky. So this project's
three-position gate is *unusual rather than weak*: it is the thing the field
mostly declined to keep. That is also the warning: per-position pass/fail is what
made those two projects disable their tests, which is why the mined set is scored
as a count with a floor.

Stockfish is the exception and it moved the check out of unit tests into a
required CI job over thousands of positions, hard-failing on **inconsistency** --
invalid mate scores, wrong distance, wrong sign, bad PVs -- and treating failure
to *find* as a soft score compared against master by hand.

**`fastchess` here already supports `-check-mate-pvs`**, verified against the
installed binary: "Check that PVs for mate scores have the correct length and end
in checkmate." `fastchess.sh` does not pass it. It costs nothing, copies nothing,
and turns every SPRT into a mate-PV consistency check -- the residue no unit test
reaches. It is in the accepts for that reason.

### 7. Position sources: one clean licence, and no free mate suite exists

Checked, not assumed. **There is no public-domain or CC0 collection of forced
mate positions.** Every classical mate suite is copyrighted, silent on licence,
or copyleft as hosted, and the only free-licensed mate positions at scale are
derived from another engine's search -- which DEC-016 bars regardless of licence.

- **Usable:** `arasan2026.epd`, **MIT**, the licence naming its test directory
  explicitly. Its best-move derivation is unstated, which stops mattering for
  mates because a forced mate is independently decidable by our own generator.
- **Barred by licence:** Stockfish bench positions (GPL-3.0-or-later), the CPW
  position pages (CC BY-SA), LCT II and ECM (express copyright), and every suite
  with no licence file at all -- which is most of them, including the ones whose
  READMEs *assert* public domain over files that carry an explicit copyright.
- **Barred by provenance even where the licence is clean:** the Lichess puzzle
  database is CC0 but its positions were re-analysed with another engine's
  network; `matetrack` and `mates2000.epd` are unlicensed *and* selected by what
  Stockfish can solve.

The law is thinner than the rule -- facts are uncopyrightable, and rule-driven
compilations get no EU database right -- but this project's rule is stricter than
the law on purpose, and the real exposure is not litigation, it is a file with no
answer to "where did this come from".

**Which is the argument for building our own, and it is the one place the
research and the constraint agree.**

### 8. Cost

No match and no verdict: nothing here changes play. Construction, verification
through two oracles, a test rewrite, one flag in `fastchess.sh`, and the fast
suite green. The measurements above are done and are not repeated.

## References

- Stockfish `09b6d283`, `dbd7f602`, `dabaf222`, PRs #3606 / #3612 / #3641 -- the
  controlled experiment: depth limits removed, SPRT-neutral at STC and LTC, mate
  finding halved, reverted.
- Stockfish `27a1877` (PR #52) and `55a3e0af` -- the beta mate-band guard removed
  from RFP, razoring and NMP, with a better mate score after.
- Stockfish `fa8b6add` (PR #7040) -- the depth ceiling made a function of how
  close the score is to decisive.
- https://www.chessprogramming.org/Mate_Distance_Pruning -- "adds little playing
  strength", helps analysis and mate solving.
- https://www.chessprogramming.org/Test-Positions -- position testing has fallen
  out of favour against SPRT.
- `fastchess --help`, installed **alpha 1.8.1 20260720-daa3ea2** -- the version
  in this step's research was written as 1.8.2 and the binary reports 1.8.1;
  `-check-mate-pvs` is present and was verified on a real match, not on `--help`
  alone.
