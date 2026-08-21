id:         S149
goal:       the second killer slot holds a move distinct from the first, so a repeated fail-high stops destroying it, and the test asserts distinctness rather than non-zeroness
accepts:    the store at `src/search.cpp:761-762` does not copy slot 0 into slot 1 when the move being stored already equals slot 0, so the two slots always hold distinct moves -- the published rule, CPW Killer Heuristic: "The replacement scheme ought to ensure that all the available slots contain different moves"; the duplicate rate is re-counted after the change on the same instrumentation that measured 66 % of stores and 44 % of nodes before it, and reported; `tests/test_search.cpp:477-499` is re-targeted from counting non-zero slots to counting **distinct** slots and is observed red at the parent commit, because the existing assertion passes on a duplicated slot and is the reason this went unseen; play-altering, so INV-6 is not available and it is decided by SPRT -- a verdict of zero is recorded as zero and the guard may still be kept with the reason stated; `adocs/specs.md`'s ordering row states the eligibility and the distinctness rule together
touches:    src/search.cpp, tests/test_search.cpp, adocs/specs.md, adocs/decisions.md, adocs/audit/2026-08-21_adversarial.md, adocs/data/S149_sprt.sh, adocs/data/S149_sprt.log
excludes:   the killer slot count, which stays at two; the ordering band constants, which CLAUDE.md and `src/search_params.hpp:29` keep out of the tuned set; history malus, gravity and butterfly indexing, which are S093 and which this step must land before rather than merge with
decisions:  DEC-019, DEC-063, DEC-098, DEC-099, INV-6
closes:     2026-08-21_adversarial-F01
blocks:
paused_by:
done:      H0 accepted: the CPW killer-distinctness guard measured -11.02 +/- 10.53 Elo, nElo -14.21, LLR -2.97 at [-5,5] over 2522 games in 1 h 05 m against ac4c588, 0 forfeits in 2524, and was reverted. F01's duplication reproduced exactly (351422/532133 stores 66.0 %, 5115505/11531069 nodes 44.4 %) and the guard removed it completely (0/11146351) for -3.34 % negamax nodes -- and still lost. DEC-019's fourth entry, recorded as DEC-098; bounds as DEC-099. The test is re-targeted a third time to assert the duplication, red-checked under a re-applied guard, and carries the number as a fence. F01 accepted, not fixed. The ageing hypothesis is S159.

## Why this is first

AGENTS.md par.0: a bug that has been found gets fixed before anything else
starts. This is a found bug in shipped code, measured at 44.4 % of negamax
nodes, so it precedes S142 and every other pending step.

**It must land before S093.** S093 rewrites this same block. A verdict taken on
S093's history rewrite while the second killer slot is dead attributes to history
whatever the dead slot was costing, and neither number then means anything.

## What was done, 2026-08-21

### Red first, and observed

`tests/test_search.cpp` "a search fills the ordering tables" counted slots that
were **non-zero**, which a slot 0 copied onto itself satisfies. It now counts
slots that are **distinct**, and the old non-zero count stays as the
precondition so "distinct" cannot pass on a table that is simply empty:

```
REQUIRE(killers_1 > 0);                    // a second killer was written at all
REQUIRE(killers_1_distinct == killers_1);  // and each holds a move slot 0 does not
```

Observed red at `ac4c588`, before any change to `src/search.cpp`:

```
/home/max/ws/chesso/tests/test_search.cpp:520: FATAL ERROR: REQUIRE( killers_1_distinct == killers_1 ) is NOT correct!
  values: REQUIRE( 2 == 5 )
```

The search depth in that case moved from 6 to 8 in the same edit, and that is
the only collateral change. At depth 6 the search fills three second slots and
duplicates one of them, so the red is a margin of one; at depth 8 it fills five
and duplicates three, and the case still runs in 53 ms. A regression test for a
guard is worth only as many instances as it produces.

### The fix

`src/search.cpp`, inside the existing `if (!is_capture)`:

```cpp
        if (moves[i] != state->killer_moves[0][ply]) {
          state->killer_moves[1][ply] = state->killer_moves[0][ply];
          state->killer_moves[0][ply] = moves[i];
        }
```

Nothing else moved. Slot count still two, bands untouched, history and
countermove stores unchanged -- `excludes:` held.

### The duplicate rate, before and after, on the audit's own instrumentation

The audit's four counters at the audit's own placement (`killer_stores` and
`killer_dup_store` at the store, `km_probe` and `km_dup_live` in `negamax`
after the `tt_eval` read), rebuilt on scratchpad copies of the tree under
`/tmp/.../scratchpad/inst_before` and `inst_after`, Release, Ninja, engine
target only, never in the repository tree. Driver: the audit's own 11 positions
at depths 12 to 22, `Hash 64`, one process --
startpos 13, kiwipete 13, lasker 18, promo-mess 12, 9bishops 14, kpk 22,
perpetual 16, mate-QR 15, underpromo 14, tactical 13, checkfest 13.

**The before figures reproduce the audit's exactly**, which is what makes the
pair comparable:

| | before (`ac4c588`) | after |
|---|---|---|
| killer stores | 532133 | 498618 |
| ... that re-store the move already in slot 0 | 351422, **66.0 %** | 330666, **66.3 %** |
| negamax nodes | 11531069 | 11146351 |
| ... with both slots holding the same non-zero move | 5115505, **44.4 %** | 0, **0.0 %** |

The repeat rate itself does not move and was never expected to: it is a
property of the search, not of the store. What moves is what a repeat costs.
Those stores are now no-ops instead of slot-1 kills, and **no node in the run
sees a duplicated pair**.

### What it does to the tree

Negamax nodes **-3.34 %**. Total nodes over the same 11 positions **-2.87 %**,
18985913 to 18440510, and the spread is what says this is a real reordering
rather than a rounding: kpk -73.29 %, 9bishops -34.30 %, checkfest -19.67 %,
startpos -8.35 % against lasker +32.65 % and tactical +24.08 %. Two positions
change best move with a changed score -- 9bishops `d6g3` at cp 3538 to `h5e8`
at cp 3536, kpk `a1b1` at cp 107 to `a1b2` at cp 90. No chess judgement is
attached to either; they are recorded as evidence that play changed.

`tools/search_bench.py` at depth 12, candidate against `.ref-builds/ac4c588`:
560233 / 3511640 / 397722 becomes 562863 / 3514424 / 386441, +0.47 % / +0.08 %
/ -2.84 %, best move unchanged at all three. Node counts differ, so INV-6 is
not available and the SPRT is owed.

### Gates

- `build` (Release, native, PGO off, `CHESSO_TUNE=OFF`): 18/18 fast, 15.58 s.
- `build-tune` (`CHESSO_TUNE=ON`): 18/18 fast, 15.69 s.
- `./clang-format.sh --check`: exit 0.
- **The mate suite is in the fast label and did not move.** `test_engine`
  carries the S145 set of 48 forced mates and `test_search` its own mate cases;
  both passed unchanged. Pruning and ordering changes hiding a mate is this
  project's recurring bug, so this is checked rather than assumed.
- `tools/plan_prose_check.py`: 33 DRIFT notes with and without the `specs.md`
  edit, so the edit introduced none.

### The SPRT, pre-registered before launch

`adocs/data/S149_sprt.sh`, mirroring `fastchess.sh`'s live engine block --
`8+0.08`, `Hash 16`, `Threads 1`, `UHO_Lichess_4852_v1.epd`, `-repeat`,
`-recover`, the adjudication pair, `-check-mate-pvs`, concurrency 12 -- against
`ac4c588`, with the candidate snapshotted before the first game (DEC-020) and a
terminal marker on every exit path (DEC-061). The success and failure paths were
both smoke-tested before launch: a 4-game run printed `SPRT-RUN-DONE s149`, and
an occupied output directory printed `SPRT-RUN-FAILED`.

**Bounds `elo0=-5 elo1=5 alpha=0.05 beta=0.05`**, and the pair is the decision.
The effect is genuinely two-sided: the mechanism argues positive, but DEC-019's
ledger is three published figures that measured 0, 0 and *slower* on this code,
capture ordering among them at a reported 150 Elo. And a pair that cannot
contain the truth random-walks -- DEC-063 measured `elo0=0 elo1=5` running
6 h 36 m over 9036 games for nothing where `elo0=-5 elo1=5` returned a verdict
in 1 h 41 m over 2312, same binaries.

**The reading of all three outcomes, written before a game was played:**

- **H1 accepted** -- not a regression of 5 Elo or more, sign positive. Keep and
  ship, and record the point estimate as biased upward by the early stop
  (DEC-063): what the run establishes is the pre-registered claim, not the
  magnitude.
- **H0 accepted** -- the guard costs 5 Elo or more. DEC-019's fourth entry, a
  published rule that measured negative here, recorded as one. Revert the two
  lines; do not keep them on the strength of the argument.
- **No verdict** -- recorded as zero, and a zero is a real result (S005, S006,
  S015). The guard is kept anyway with the reason stated rather than implied:
  two lines, it removes a state the design does not intend, it is what CPW's
  *Killer Heuristic* page specifies, and it must be in the tree before S093
  rewrites this block. No re-run at other bounds without a `decisions.md` entry.

`adocs/specs.md` was written at this point stating the distinctness rule and the
measured duplicate rates, with no Elo claim, coupled to the two lines exactly as
the code was. The verdict below took the other branch, so it was rewritten
rather than reverted: the rates stayed, the rule became its opposite, and the
number that decided it was added.

## The verdict: H0 accepted. The guard costs about 11 Elo and was reverted

```
Results of candidate-s149-killerdedupe vs ref-ac4c588 (8+0.08, 1t, 16MB, UHO_Lichess_4852_v1.epd):
Elo: -11.02 +/- 10.53, nElo: -14.21 +/- 13.56
LOS: 2.00 %, DrawRatio: 35.92 %, PairsRatio: 0.88
Games: 2522, Wins: 782, Losses: 862, Draws: 878, Points: 1221.0 (48.41 %)
Ptnml(0-2): [134, 295, 453, 275, 104], WL/DD Ratio: 1.94
LLR: -2.97 (-100.7%) (-2.94, 2.94) [-5.00, 5.00]
SPRT ([-5.00, 5.00]) completed - H0 was accepted
Total Time: 01:05:35
```

**0 time forfeits in 2524 games**, 34.8 % draws and 65.2 % decisive, which is
the unbalanced book doing its job. The run is `adocs/data/S149_sprt.sh`, its log
is `adocs/data/S149_sprt.log`, and the 7.2 MB PGN is left uncommitted at
`/tmp/chesso_sprt_s149_20260821_183204/games.pgn` -- the log carries the
verdict, the pentanomial and the census.

**The pre-registered H0 clause governs and it says revert.** Two lines out of
`src/search.cpp`; the store is byte-identical to `ac4c588` again and carries a
comment saying the shift is unguarded on purpose, with the number. The guard is
not kept on the argument that it is the published rule -- that is exactly
DEC-019's failure mode, and the clause above forbids it by name.

**This is DEC-019's fourth entry and the strongest of the four.** The other
three measured zero; this one measured negative. DEC-098 records it. DEC-099
records the bounds decision, and the run is a second confirming case for
DEC-063: `elo0=-5 elo1=5` returned a completed verdict in 1 h 05 m over 2522
games, in the direction nobody predicted, where DEC-063's default pair spent
6 h 36 m over 9036 games on nothing.

### The test, re-targeted a third time, at the truth

Not reverted with the code, because F01's second half is right regardless of the
verdict: counting **non-zero** slots is vacuous, a slot 0 copied onto itself
being non-zero. It counts **duplicated** slots now:

```cpp
REQUIRE(killers_1 > 0);              // precondition: a second slot was filled
REQUIRE(killers_1_duplicated > 0);   // and some of them hold slot 0's move
```

It is a fence, and the comment beside it carries the whole measurement -- the
66.0 % and 44.4 %, the guard, the -11.02 +/- 10.53 over 2522 games, H0 -- so an
agent who notices the duplication and "fixes" it goes red here and finds the
number instead of repeating the night. **Non-vacuity was checked in both
directions**, which is what makes it a fence rather than a description: green on
the shipped store, and red under a temporarily re-applied guard --

```
tests/test_search.cpp:535: FATAL ERROR: REQUIRE( killers_1_duplicated > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )
```

Depth 8 is kept. It serves the new assertion better than depth 6 for the same
reason it served the old one: 5 second slots and 3 duplicates rather than 3 and
1.

### What the number does not say, and where that goes

**Hypothesis, not a finding -- nothing below has been measured.** The unguarded
shift does two things and only one of them is what F01 named. It duplicates the
slots. It also **discards whatever slot 1 held**, every time a quiet repeats,
which is 66 % of stores -- so the unguarded store is incidentally an aggressive
ageing mechanism for the second slot. The guard removes both at once, and since
killers persist across every iteration of one `go`, a guarded slot 1 can hold a
move that refuted something eight iterations ago and keep being tried at 800000
for the rest of the search. Retaining a stale killer longer is a plausible
reading of the 11 Elo, and it points at a different change -- ageing, or
clearing the killer table per iteration rather than per `go`. That is **S159**,
independent work, not measured here and not folded into this step. S093 rewrites
this same block, so S149's ordering constraint carries over to it unchanged.

### Gates, after the revert

- `build` (Release, native, PGO off, `CHESSO_TUNE=OFF`): 18/18 fast.
- `build-tune` (`CHESSO_TUNE=ON`): 18/18 fast.
- `./clang-format.sh --check`: exit 0.
- Mate suite in the fast label: unchanged, as it was before the revert.
- `README.md` checked -- owner-written, no change needed. `MANUAL.md` and
  `DEV_MANUAL.md` checked: neither states anything about killer slot
  replacement, so neither needs a change. The engine's shipped behaviour is
  unchanged by this step in any case, the tree ending byte-identical to
  `ac4c588` except for comments.

### The step completes, it does not fail

The goal was a property decided by SPRT and it was decided. A verdict of zero is
recorded as zero and a verdict of negative is recorded as negative -- S005, S006
and S015 are the precedents, and this is the case the rule was written for. What
this step leaves behind is the number that stops the next agent spending a night
on it, a fence that makes that automatic, and S159.

author:    Maksym Bodnar
