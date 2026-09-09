# S159 killer-table census

The quantity S159's hypothesis is about, counted before a game was played.
2026-09-09, Release, `g++ 13.3`, the i7-8700K workstation.

## Why the driver set is new

S149's own driver -- "startpos 13, kiwipete 13, lasker 18, promo-mess 12,
9bishops 14, kpk 22, perpetual 16, mate-QR 15, underpromo 14, tactical 13,
checkfest 13" -- names 11 positions and **records none of their FENs**, in
`adocs/plan_done/S149_killer_slot_dedupe.md` or in the
`adocs/audit/2026-08-21_adversarial.md` finding that produced it.
`grep -rn 9bishops` over the repository hits those two documents and nothing
else. That set is therefore not reconstructible and S149's 66.0 % / 44.4 %
cannot be reproduced exactly by anyone.

`S159_census_positions.txt` is built from FENs the repository does record --
`src/data_structures.hpp`'s position constants, `tools/search_bench.py`'s
three, and `tests/assets/test_jsons/` -- one source per row, at 18166063 nodes
against S149's ~19 M. Every S159 figure is measured on it, and it is recorded
here so the next agent is not in the position this one was.

## How to reproduce

```
git archive HEAD | tar -x -C <scratch>/inst
patch -p0 -d <scratch>/inst < adocs/data/S159_census_instrument.patch
cmake -S <scratch>/inst -B <scratch>/inst/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build <scratch>/inst/build --target chesso -j8
python3 adocs/data/S159_census_run.py <scratch>/inst/build/src/chesso \
        adocs/data/S159_census_positions.txt <scratch>/census
```

The instrumentation lives in the patch and never in `src/`. The driver waits
for `bestmove` before writing the next command: piping the whole script with a
trailing `quit` kills each search before it looks at a node and the census then
reads `stores=0` over the entire set, which is TOOLCHAIN.md's "the one way to
ask it that lies" in another costume. It cost an hour here.

## What was counted

Five counters at S149's own placement -- `killer_stores` and `killer_dup_store`
at the store in `negamax`'s fail-high block, `km_probe`, `km_dup_live` and
`km_stale_live` after the `tt_eval` read -- plus a `killer_iter[2][MAX_PLY]`
stamp shifted exactly as the moves are, an iteration counter set by the depth
loop in `iterative_deepening_search`, and three counters the step file did not
ask for: `k1_live`, `k1_distinct` and `k1_stale`.

The three extra ones exist because **the step file's `km_stale_live` watches
slot 0 and the hypothesis is about slot 1**. Section 6 defines it as "nodes
where slot 0 is non-zero and its stamp is below the current iteration"; the
text it serves says "a guarded slot 1 can hold a move that refuted something
eight iterations ago and keep being tried at 800000". Slot 1 is the one the
argument is about, so it is counted too.

## The numbers

Two builds: HEAD, and HEAD with S149's reverted guard re-applied on the
instrumented copy only, which is where the hypothesis says the staleness lives.

| | HEAD | S149's guard |
|---|---|---|
| killer stores | 222815 | 210270 |
| ...that re-store the move already in slot 0 | 160423, **72.0 %** | 152317, **72.4 %** |
| negamax nodes | 10189958 | 9039916 |
| ...both slots equal, `ORDER_KILLER_1` dead | 4484073, **44.0 %** | 0, **0.0 %** |
| ...offering a **distinct** slot 1 | 4623719, **45.4 %** | 8027693, **88.8 %** |
| ...distinct **and** stale | 90575, **0.89 %** | 152957, **1.69 %** |
| stale share **of distinct offers** | **1.96 %** | **1.91 %** |
| slot 0 stale | 55142, 0.54 % | 51421, 0.57 % |
| total nodes over the 11 positions | 18166063 | 16016996 |

The duplicate rates reproduce S149's on a different position set -- 72.0 %
against 66.0 % of stores, 44.0 % against 44.4 % of nodes -- and the guard
removes the duplication completely, 0 of 9039916, as S149 measured.

## What it says

**The ageing reading of S149's -11 Elo does not survive its own census.** The
last row is the one that decides it: the stale share of distinct second-killer
offers is **1.96 % on HEAD against 1.91 % under the guard**, flat. The guard
did not preserve proportionally staler killers. It raised the stale *count*
only because it raised the *offer* count -- a distinct slot 1 offered on 88.8 %
of nodes instead of 45.4 %.

So the 11 Elo attaches to **offering the second killer roughly twice as often**,
not to preserving stale ones, and candidate A -- clearing the table once per
iteration -- has **0.89 % of nodes** to act on. The reason is the first row:
on HEAD the unguarded shift already *is* an aggressive ageing mechanism, it
discards slot 1 on 72 % of stores, and almost no cross-iteration state survives
for a per-iteration clear to remove.

This is a prior and not a verdict. DEC-019 is the rule that an argument --
including this one -- decides what to try and never what to conclude, which is
why it is written into `S159_sprt.sh`'s header before the games rather than
read back over them afterwards.
