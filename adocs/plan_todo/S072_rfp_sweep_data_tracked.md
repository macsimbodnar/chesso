id:         S072
goal:       S068's sweep evidence lives in adocs/data/ and its columns are stated, so the next step argues from a tracked file
accepts:    `adocs/data/` holds the three sweep tables and the three scripts that produced them, committed; `S068` cites them by tracked path and no longer by a `.../scratchpad/` path; the column meanings are written down, `min` for `RFP_MIN_DEPTH` against `min_ply` for `RFP_MIN_PLY`, and the two tables' apparent disagreement at margin 75 is resolved in prose; S068 states how a margin is varied at HEAD -- by editing `src/search.cpp:38`, because `-DRFP_MARGIN=75` does not compile -- with a forward pointer to S073; the quoted `-Werror` output is reproduced once and pasted in
touches:    adocs/data/, adocs/plan_todo/S068_rfp_margin_retune.md
excludes:   any change to `src/`, including the `#ifndef` guards, which are S073's; the margin decision itself, which is S068's; re-running the sweep, which this step only makes possible
decisions:
closes:     2026-08-16_plan_review-F04
blocks:
paused_by:
done:

## Why this exists

S068 is the next step in the order and its entire evidence base is outside the
repository. `S068:27` and `:35` cite `.../scratchpad/rfp_ply_sweep.tsv` and
`.../scratchpad/rfp_sweep.tsv`. Those resolve today only inside one dead
session's scratchpad:

```
/tmp/claude-1000/-home-max-ws-chesso/e6b3ca05-ee17-4b5d-bd45-f4a1a2377e53/scratchpad/
  rfp_sweep.sh        rfp_sweep.tsv        rfp_sweep.log
  rfp_ply_sweep.sh    rfp_ply_sweep.tsv    rfp_ply_sweep.log
  rfp_guard_sweep.sh  rfp_guard_sweep.tsv  rfp_guard_sweep.log
```

AGENTS.md section 12: nothing that matters is allowed to exist only in an
agent's own memory or tool-local notes. `.tuning/` losing `selfplay_v1.tsv` at
DEC-049 is the same failure and cost S065 a night; a scratchpad is more volatile
than `.tuning/`, not less. **Copy the files first, before anything else in this
step** -- they are 10 and 46 rows and the directory is one `rm -rf` from gone.

## The method also stopped working

Both scripts vary the constant with a compiler flag:
`flags="-DRFP_MARGIN=$margin -DRFP_MIN_DEPTH=$mind -DRFP_MAX_DEPTH=$maxd"`. At
HEAD the constants are unconditional `#define`s at `src/search.cpp:38-47` with no
`#ifndef` guard, so:

```
$ cmake -S . -B build-probe -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DRFP_MARGIN=75"
$ cmake --build build-probe -j12 --target chesso
/home/max/ws/chesso/src/search.cpp:38: error: "RFP_MARGIN" redefined [-Werror]
```

`rfp_sweep.sh` swallows that into `BUILD_FAIL` and keeps going, so a re-run
produces a table of failures rather than an error. Making `-D` work is S073's
job and is deliberately not this step's: this one records what was actually run
and what it means, so that S068 can be executed at all in the meantime.

## What reproduces and what does not

The shipping row of S068's table is exact -- `tools/search_bench.py
./build/src/chesso 9` sums 292313 + 1026739 + 103001 = 1422053. The margin-75
row, 1216123, cannot be reproduced without a hand edit that S068 does not
mention.

## The two tables read as contradictory and are not

Margin 75 is green at 1216123 nodes in one table and RED at 2886952 in the
other, because `min` and `min_ply` are different constants and S068's prose does
not distinguish them. Whoever executes this states the columns, or the next
reader concludes the sweep is unreliable and re-runs a sweep that no longer
builds.

## Cost

Minutes plus one probe build. No match.
