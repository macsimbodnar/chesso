id:         S033
goal:       prune a node whose static score is already far enough above beta
accepts:    an SPRT against the preceding commit returns a verdict; a position with a forced mate inside the pruned depth is in the fast suite and passes before the feature is called done
touches:    src/search.cpp negamax
excludes:   forward futility and razoring, which are S026; late move pruning, which has no step
decisions:  DEC-033
closes:
blocks:
paused_by:
done:

## Why this exists as its own step

The plan had no entry for it. S026 is "futility and razoring" and its goal line
describes forward futility, which asks whether a move can reach alpha. Reverse
futility asks the opposite question at the node itself -- is the static score so
far above beta that the opponent cannot claw it back in the remaining depth --
and prunes the whole node. Different test, different place in `negamax`,
different failure mode. DEC-033.

Of the search techniques chesso does not have, this is the one with the largest
figure in the one public per-feature log found: Blunder measured +57.1 +/- 16.9
in self-play. That is a reported figure and therefore decides what to try and
never what to conclude (DEC-019). DEC-033 puts the ceiling on the whole search
block at about 10 cp per effective doubling on the errors that matter.

## Shape

At a non-PV node, not in check, with depth below some small bound, if
`evaluate() - margin * depth >= beta`, return without searching. It is the null
move observation applied without making the null move, which is why it shares
null move pruning's guards.

## Hazard, twice observed

Both NMP and LMR shipped with a bug that hid a mate, and both were caught by a
mate test rather than by a benchmark. This is a third pruning rule with the same
shape, and the same requirement applies: a mate inside the pruned depth, in the
fast suite, red before it is green.

The specific trap here is the mate score. A static evaluation is never a mate
score, so a node holding a forced mate for the opponent can still have a static
score above beta and be pruned. Guard `beta` against the mate band exactly as
the null move guard does.

## Depends on nothing, and that is the point

The static score is already computed at every quiescence node and is one
`evaluate()` call at an interior node. No new tables, no accumulator work, no
move ordering change. It is the cheapest thing in the search block to try.
author:    Maksym Bodnar

---

# What was measured, 2026-08-16

## The hazard is real and it fired on the first build

The rule as written -- non-PV, not in check, `ply > 0`, `depth <= 6`, `beta`
outside the mate band, `evaluate() - 100 * depth >= beta` -- turned two existing
cases red on the first `ctest` run:

```
TEST CASE:  mate in two is found at the right distance
  test_search.cpp:137: REQUIRE( black.mate_found ) is NOT correct!  logged: depth 3
TEST CASE:  pruning does not hide a forced mate
  test_search.cpp:986: REQUIRE( black.mate_found ) is NOT correct!  logged: black, depth 3
```

Both are `MATE_IN_2_B_POS`, `4K3/q7/8/4k3/8/8/8/8 b - - 0 1`, at fixed depth 3
from a cold table. The mate is not lost at depth 4 and above.

A trace printing every prune identified the node exactly, and it is **not** the
mate-band case the step file predicted:

```
RFP ply=1 depth=2 alpha=-965 beta=-964 eval=-764 ret=-964 fen=4K3/q7/4k3/8/8/8/8/8 w - - 1 2
```

The side to move there is a **bare king, losing by a queen**. It fails high
because `beta` is -964: the parent is a null-window scout hunting a mate score,
and the static claim "I am only 764 behind" clears a bound of -964 by exactly
zero. The guard on `beta` against the mate band does nothing here -- -964 is an
ordinary score. What RFP asserts is a lower bound on the node, and a forced mate
is the one thing that bound cannot be made to respect.

## The constants sweep: 45 settings, and green is luck

Margin in {75, 100, 150, 200, 300} x a lower depth bound in {1, 2, 3} x an upper
one in {4, 6, 8}, each one built and run against the fast search suite, with
`tools/search_bench.py` at depth 9 for what the setting buys.
`.../scratchpad/rfp_sweep.tsv` has all 45 rows. Baseline, the rule compiled out
through its own depth bound while the bounds were still `#ifndef`-guarded for
the sweep: **3752725 nodes**, later confirmed to the node against `c56ab41`
built from a worktree. (`DEV_MANUAL.md`'s 3136397 predates S065 and is not a
baseline for anything measured today; it is corrected in this step's commit.)

| margin | depth 1..6 | suite |
|---|---|---|
| 75 | 1190649 | RED |
| 100 | 1398911 | RED |
| 150 | 1834601 | green |
| 200 | 2052814 | green |
| 300 | 2509054 | green |

Green arrives at margin 150 and it is arithmetic luck, not safety: the harmful
prune above needs `-764 - 2 * margin >= -964`, which is true at 100 and false at
150 by 100 points. A lower depth bound of 3 -- pruning only at depth 3 and above -- is green at
margin 100 and buys almost nothing: 3087174 against a baseline of 3752725, and
150/3/6 at 3450944 and 300/3/8 at 3664431 are within a few per cent of not
pruning at all. Most of the saving is at depth 1 and 2, which is where the mate
cases break. Why the count barely moves has not been measured; the candidate
explanation is that an RFP return stores no table entry and leaves the parent
with no move to order on, so what it saves it partly pays back.

## A second position, built to be immune to luck

`4K1R1/q7/5P2/4k3/8/1P6/2P5/1B2N3 b - - 0 1`. It is `MATE_IN_2_B_POS` with White
material added until White **leads by 500**, keeping a mate in two that no
checking move also forces. Three properties matter and the first two are
asserted in the test rather than assumed:

- the mated side is **ahead**, which is what puts its node above beta
- the key is **quiet**, so the White node is not in check, where the rule is
  already forbidden and the case would prove nothing
- the mate is two moves, inside any depth bound worth having

Built with `python-chess` enumerating exhaustively; `stockfish` at depth 18
agrees independently: `score mate 2`, key `e5e6`. Both tools, because DEC-023.

This position is red at margin **150**, where the two `MATE_IN_2_B_POS` cases
pass. It is the case that separates a setting that is safe from a setting that
happens to miss.

## Five guards, none of them works

Second sweep, `.../scratchpad/rfp_guard_sweep.tsv`, all at depth 1..6:

| guard | margin 100 | suite |
|---|---|---|
| none | 1398911 | RED, both positions |
| the side to move is not losing on the static score | 1497079 | RED, the material leader |
| the parent's bound is not a losing one | 2665386 | RED, the material leader |
| the side to move still has a piece | 1398911 | RED, the material leader |
| `game_phase() >= 6` | 1398911 | RED, the material leader |

The first guard fixes the bare-king case and nothing else. The last two do not
change a single node on the bench, which is the measurement saying they never
fire on a real position.

**This is a property of the technique, not of its constants.** RFP returns a
static lower bound; a static evaluation cannot express "this side is being
mated", and in this engine it provably cannot: `evaluate_expensive()` clamps the
whole king-safety and mobility correction to `+/-LAZY_EVAL_MARGIN`, 150 cp. The
term is fitted and non-zero since S065 -- `king_safety_mg` is
`{22, 23, 25, 35, -30, 26, 13, -42, -27}` -- and still bounded two orders of
magnitude below a mate.

## What the unguarded rule cost in play

Measured on the setting that was **not** shipped -- margin 100, depth 1..6, no
ply bound -- because it is what the guard has to be judged against. Under
iterative deepening, `go depth 6` from a cold table, both mate positions were
still found, one iteration late: `mate 2` first appeared at **depth 4** where
the fixed-depth cases want it at 3. Not a lost mate, but a real cost, and the
fixed-depth call the suite makes -- `search()` at depth 3, cold table, nothing
ordered from a previous iteration -- is the stricter of the two.

## The guard that works: leave the top of the tree alone

The rule already exempts the root, because the root's answer is the one that
gets played. The mate cases say that exemption is one ply too narrow: the trace
above prunes at **ply 1**, whose returned bound is exactly what the root
compares against alpha. Exempting the first plies is the same idea one step
further down, and the sweep prices it (`.../scratchpad/rfp_ply_sweep.tsv`, all
at margin 100, depth 1..6):

| first ply pruned | nodes | against no ply bound | suite |
|---|---|---|---|
| 1 | 1398911 | -- | RED, both positions |
| 2 | 1408517 | +0.7 % | green |
| **3** | **1422053** | **+1.7 %** | **green** |
| 4 | 1781672 | +27.4 % | green |
| 5 | 1895553 | +35.5 % | green |

Two plies of protection cost 1.7 % of what the rule saves; a third costs 27 %.
The cliff is where the setting was chosen, not the greenness -- ply 2 is green
too. Margin 75 is green at the same ply bounds and saves more (1216123 at ply 3)
and is not what is shipped: one change at a time, and the margin is a tuning
question for a later step.

**Shipped: margin 100, first pruned ply 3, depth bound 6.**

Both figures below are against `c56ab41` built from a git worktree, not against
a define passed on the command line: once `RFP_MAX_DEPTH` went back to a plain
`#define`, `-DRFP_MAX_DEPTH=0` stopped overriding anything and a build that was
supposed to have the rule compiled out still had it. The worktree number agrees
with the sweep's own off row to the node, which is what says the sweep was
measuring what it claimed.

| | midgame | kiwipete | tactical | total | wall |
|---|---|---|---|---|---|
| `c56ab41` | 917971 | 2386591 | 448163 | 3752725 | 0.470 s |
| S033 | 292313 | 1026739 | 103001 | 1422053 | 0.226 s |

**62.1 % fewer nodes**, same three best moves — `c3d5 e2a6 d7c8q` — and 2.1x
the speed at fixed depth 9. Nodes per second **fall**, 8283 knps to 6145 knps:
the rule adds an `evaluate()` at interior nodes and pays for it many times over
in nodes not visited.

At that setting the mate latency is gone as well. Iterative deepening, cold
table, `go depth 6`, against the same worktree build:

| position | with RFP | `c56ab41` |
|---|---|---|
| `MATE_IN_2_B_POS` | `mate 2` at depth 3 | `mate 2` at depth 3 |
| the material-leader position | `mate 2` at depth 3 | `mate 2` at depth 3 |

Every iteration from 1 to 6 prints the same score in both builds on both
positions.

## One aborted match, recorded because it cost machine time

Before the ply bound was found, the unbounded setting was put on the machine:
`REF=c56ab41 ./fastchess.sh --fast`, stopped by hand at **87 games started, 60
finished, Elo +133.61 +/- 70.56, LLR 0.66 of 2.20**. It is not a verdict and it
is not the shipped engine -- it is the setting the mate cases call red -- and it
is kept at `.tuning/sprt_s033_unbounded_aborted.log` only so the five minutes it
took are on the record. It was stopped because a second SPRT on the shipped
setting would have cost hours that stopping cost minutes.

## What is still true and untested

A mate that first becomes visible below ply 3 can still be missed for an
iteration, and nothing in the suite covers that. It is not a defect of the
setting, it is the technique: the rule returns a static lower bound and a mate
is what a static bound cannot respect. The depth bound and the ply bound
contain it; they do not remove it.
