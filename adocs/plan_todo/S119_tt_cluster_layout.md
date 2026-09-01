id:         S119
goal:       the table becomes cache-line clusters with an aged replacement, a prefetch issued when the key is known, and huge pages
accepts:    an SPRT verdict **at the S105 harness setting, with the pressure ratio stated** -- the ratio being overwrites per entry, which is what transfers across time controls: the rating list's 2'+1" writes on the order of 660 M nodes against 5.6 to 11 M entries, 60 to 120 apiece, and 16 MB at 8+0.08 reproduces that while 128 MB undershoots it eightfold and "would flatter every table-hungry change S119 is about to make" (DEC-088, whose `Consequences:` line is what replaced "at Hash 128" here); `sizeof` the cluster is exactly 32 or 64 bytes and the array is aligned so no cluster straddles a cache line, asserted at compile time; the replacement prefers depth **and** age together rather than depth within a generation alone; the prefetch is issued as soon as the key is known in make_move; huge pages are requested and the failure path is a normal allocation, not an abort; `hashfull` is reported over UCI and is checked to stay low at the rating control; the nps and the nodes-to-depth are both recorded, because this step moves them in opposite directions
touches:    src/transposition_table.cpp, src/transposition_table.hpp, src/data_structures.hpp, src/bitboard.cpp make_move
excludes:   the static evaluation field, which exists since S094; bound-sign correctness, which is S106
decisions:  DEC-083
closes:
blocks:
paused_by:
done:

## The verdict is taken at the harness setting, not at Hash 128

Applied by S139, and it is DEC-088's own `Consequences:` line rather than a new
choice: *"S119's SPRT clause changes from 'at Hash 128' to 'at the S105 harness
setting, with the pressure ratio stated'"*. The harness is what settles it --
`fastchess.sh:348` is

```
  -each tc="$tc" option.Hash=16 option.Threads=1 \
```

and `grep -n Hash fastchess.sh` returns that line and the `echo` above it at
`:215` and nothing else, so there is no hash override to pass. A clause asking
for 128 could only be run by editing the harness, which this step's `touches:`
does not include, and DEC-088's reason for refusing 128 is that it flatters
exactly this step.

The rating regime is still reachable and is a different tool:
`rating.sh:36` is `hash_mb=128`, fed to `-each` at `:172`. If a second verdict
there is ever wanted it is a `rating.sh` run, named as one, with `rating.sh` in
`touches:` -- not this step's SPRT.

## What is there

`tt_entry_t` is **24 bytes** -- a full 64-bit key, a 32-bit score, a 32-bit
move, depth, eval, type, generation. The table is **direct-mapped**: one entry
per slot, `hash & index_mask`, no bucket. Replacement is depth-preferred inside
a generation and always-replace across generations. There is no prefetch and no
page hint.

Three consequences. 24 bytes divides neither 32 nor 64, so **entries straddle
cache lines**. Direct-mapped means a single colliding position evicts a deep
entry with no second chance. And the probe miss is fully exposed, because
nothing is issued early.

Measured 2026-08-19, `go movetime 2000` from the start position: 16 MB to
512 MB is **36 % fewer nodes and 21 % lower nps**. The node reduction outweighs
the nps loss, so a bigger table is net positive -- but a fifth of it is being
eaten by the layout, and the rating list runs at the size where that bites
hardest.

**Expect the replacement scheme itself to be worth little** -- the surveyed
consensus is 5 to 15 Elo for bucket-plus-aging over always-replace, and the one
part that is not optional is the aging. The layout, the alignment and the
prefetch are where the measured 21 % lives.
