id:         S068
goal:       re-decide the reverse futility margin against a verdict, starting from the 75 that S033 left on the table
accepts:    an SPRT against the commit shipping margin 100 returns a verdict, recorded whatever it is; every mate case in the fast suite stays green at the setting that ships, the material-leader case included
touches:    src/search.cpp RFP_MARGIN
excludes:   the ply bound and the depth bound, which S033 set from measurement and DEC-060 records; any second pruning rule
decisions:  DEC-060
closes:
blocks:
paused_by:
done:

## Why this exists

S033 shipped `RFP_MARGIN` 100 and said so: a first setting, one pawn per
remaining ply, fitted to nothing. It bought +59.98 +/- 17.24 Elo. The sweep that
chose it also measured margin 75 at the same ply and depth bounds, and 75 is
green on every mate case and visits **1216123 nodes against 1422053**, 14.5 %
fewer, on `tools/search_bench.py` at depth 9.

That is not a reason to ship it. Fewer nodes is not more Elo -- DEC-019 is three
techniques that were quoted a gain and measured zero or worse -- and S033 shipped
the more conservative of two greens deliberately, one change at a time. This step
is where the other one gets its verdict.

## What the sweep already knows

All at ply bound 3, depth bound 6, from `adocs/data/S033_rfp_ply_sweep.tsv`:

| margin | nodes at depth 9 | fast suite |
|---|---|---|
| 75 | 1216123 | green |
| 100 | 1422053 | green, shipping |

The shipping row reproduces exactly. `tools/search_bench.py ./build/src/chesso 9`
at `d7901e3` gives 292313 + 1026739 + 103001 = **1422053**, best moves
`c3d5 e2a6 d7c8q`. The 75 row does not, without the hand edit the next section
describes.

Margins 150, 200 and 300 were measured too, in
`adocs/data/S033_rfp_sweep.tsv`, and prune less, so they are the wrong direction
unless 75 loses. **Do not read node counts across from that file.** It sweeps a
different floor and its rows are not at the shipping configuration; the next
section is why.

## Read the two tables' columns before quoting either

`adocs/data/README.md` states this in full and it is the trap this step's
evidence sets:

- `min_ply` in `S033_rfp_ply_sweep.tsv` is **`RFP_MIN_PLY`**, distance from the
  root. `src/search.cpp:47`, value 3 at HEAD.
- `min` in `S033_rfp_sweep.tsv` is **`RFP_MIN_DEPTH`**, remaining depth. It is
  in no source file and no commit -- a constant that existed only in S033's
  uncommitted working tree, which is what all three sweeps ran against.

So margin 75 reads green at 1216123 in the ply table and RED at 2886952 in the
other, and both rows say "75" and "3". The 3 names a different constant in each.
Raising the remaining-depth floor to 3 gives up the prunes at depth 1 and 2,
where nearly all the nodes are, which is why the count more than doubles -- and
it stays RED anyway, because the prune that hides the mate is at ply 1 and a
depth floor cannot reach a ply. Raising the ply floor gives up ply 1 and 2, a
handful of nodes, and is what turns the suite green. The two tables agree to the
node where the floors coincide at 1 (1190649 at margin 75, 1398911 at margin
100), which is what says they are the same code measured twice.

**Only `S033_rfp_ply_sweep.tsv` maps onto HEAD.** Its `3 100 6` row is the
shipping engine. `S033_rfp_sweep.tsv` cannot be reproduced at HEAD at all, by a
hand edit or otherwise, because the constant it sweeps is in no commit.

## How to vary the margin at HEAD

By hand, in the source. `src/search.cpp:38`:

```
#define RFP_MARGIN 100
```

Edit the literal, rebuild, and put it back. **`-DRFP_MARGIN=75` does not
compile** -- the sweep scripts in `adocs/data/` use it, and it needed the
`#ifndef` guards S033's uncommitted tree had and the shipping one does not:

```
$ cmake -S . -B build-probe -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DRFP_MARGIN=75"
$ cmake --build build-probe -j12 --target chesso
/home/max/ws/chesso/src/search.cpp:38: error: "RFP_MARGIN" redefined [-Werror]
   38 | #define RFP_MARGIN 100
      |
<command-line>: note: this is the location of the previous definition
cc1plus: all warnings being treated as errors
```

Reproduced 2026-08-16 at `d7901e3`, g++ 13.3.0. Note what the scripts do with
it: `S033_rfp_sweep.sh` swallows a failed build into a `BUILD_FAIL` row and keeps
going, so re-running one produces a table of failures rather than an error.

**S073 is the step that makes `-D` work**, turning the search constants into one
addressable parameter set with a tune build. If S073 has landed before this step
starts, use it instead of editing the literal. It is not a prerequisite: one hand
edit and one rebuild is the whole cost either way.

## The trap this step walks into

**A margin that prunes harder makes the rule wronger, not just faster.** RFP
returns a static lower bound and a mate is what that bound cannot respect
(DEC-060). The mate cases are green at 75 today, and that is one position's worth
of evidence, not a proof. Any margin below 75 needs the same treatment: the fast
suite green first, the SPRT second, and a margin that goes green only by
arithmetic is what the material-leader case exists to catch.

And do not reach for a guard when a margin goes red.
`adocs/data/S033_rfp_guard_sweep.tsv` is the third table: five candidate guards,
none of which works. Two of them -- "the side to move still has a piece" and
`game_phase() >= 6` -- do not move the bench by a single node, which is the
measurement saying they never fire. That table is on the same pre-commit tree as
the other two, so read its `guard` and `margin` columns and not its node counts.

## Cost

One verdict. S033's took 44 m 10 s at 12 cores for 1012 games; a smaller effect
takes longer, and this one is expected to be small either way.
