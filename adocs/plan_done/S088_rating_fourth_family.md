id:         S088
goal:       a fourth engine family in the reference set and the rating re-solved, so the anchor spread is inside the 30 Elo the procedure allows
accepts:    the reference set carries at least three engine families and at least five rungs, with every anchor rating read from the CCRL list at run time and never hardcoded; the added engine is built or downloaded by the agent under /home/max/ws/engines and recorded in references.tsv with its tag, source and md5, per DEC-069; a bracketing check shows the added engine inside the 10 % to 90 % band; the rated run returns a 95 % interval of +/- 30 Elo or tighter on chesso's solved rating; time forfeits at or below 1 % of an individual engine's own games, a higher rate invalidating the run, with the rate, the overruns and which side won the forfeited games all reported (amended 2026-08-18 by DEC-076 from "zero time forfeits, one invalidating the run");  the anchor sweep is re-solved over every reference and the full spread is reported, and if it is still above 30 Elo the dissenting engine is named, the spread with that engine dropped is reported beside the spread with it kept, and which of the two is quoted is a recorded decision rather than the smaller number
touches:    references.tsv, rating.sh only if the set size is wired into it, adocs/data/ for the PGN and the results file, DEV_MANUAL.md, adocs/specs.md for the measured figure
excludes:   any edit under src/ or tests/; changing the time control away from 10+0.2, which would make the figure incomparable with S087's; assessing any position, move or game from the resulting PGN
decisions:  DEC-071, DEC-067, DEC-068, DEC-069, DEC-072
closes:
blocks:
paused_by:
done:      chesso ~= 2559 CCRL Blitz, 95 % +/-25, SOFT: 3340 rated games at 10+0.2 over five engines and three families, concurrency 6, 5 h 02 m 49 s, 1 tolerated forfeit at 0.15 %. Anchor spread 121.8 Elo and 64.7 without Leorik 2.1 -- the step's premise that a third family would close it is refuted, and the disagreement is scale compression at a ratio of 0.853 rather than one misrated engine. src/ byte-identical to S087, so the instrument moved and the engine did not. Full suite 19/19. Record adocs/data/rating_2026-08-18_S088_ccrl_blitz.md

## Why this is first

S087's figure is soft and the reason is not game count. Blunder 7.1.0, Blunder
8.5.5 and Leorik 2.4 reproduce each other's CCRL ratings to within 16 Elo across
a 440-point span; **Leorik 2.1 comes out about 82 Elo above its listed rating**
whichever of the others is anchored, six standard errors at 668 games per
pairing. Two families cannot arbitrate that. A third can: if 2.1 stays the sole
dissenter against an unrelated engine, the anchor is wrong for that binary
rather than chesso's rating being uncertain by 83 Elo.

The target is 3000 (DEC-071). A finish line cannot be read from an instrument
83 Elo wide, and changing the reference set *during* the climb would make the
readings across it incomparable. So the set is fixed now and not later.

## Candidates

Anything with a CCRL Blitz entry, a binary the agent can produce without adding
a dependency, and a rating in or near the 2389 to 2829 the set already spans.
From the list read 2026-08-18: Stash 21.0 at 2713 or Stash 25.0 at 2932 (C,
make), Weiss 0.10 at 2847 or Weiss 1.0 at 2896 (C, make), Zurichess Neuchatel at
2920 (Go), Monolith 2 at 3011 (C++). The choice is recorded in the results file
with the reason.

## Cost

One bracketing run of about 15 minutes, then a rated run of about an hour at
10+0.2 -- longer than S087's hour because a fifth rung adds pairings. The
`ordo` sweep is seconds.

## As executed

### The choice, and the one measurement taken before making it

**Stash 21.0, CCRL Blitz 2713 +/-14.** DEC-072 carries the reasoning and the
rejected candidates. The short form: it is the only candidate inside the set's
existing 2389-2829 span, it has the tightest CCRL error of the six, and it is a
third author in a third language, which is what arbitrating a two-family
disagreement requires. Every other candidate sits at or above 2847 and would
have put chesso's score in the tail of the logistic curve.

All six candidate ratings and all four incumbent anchors were re-read from the
list on 2026-08-18 through `tools/ccrl_rating.py`, not carried over from S087:
Stash 21.0 2713 +/-14, Stash 25.0 2932, Weiss 0.10 2847, Weiss 1.0 2896,
Zurichess Neuchatel 2920, Monolith 2 3011; incumbents unchanged at 2389, 2568,
2664, 2829.

**`src/`, `tests/`, `CMakeLists.txt` and `cmake/` are byte-identical between
`a9f2b33` and `c2f1c43`** -- `git diff --stat` empty over all four paths.
`a9f2b33` is the commit S087 rated. So this step re-rates the *same engine*, and
the two figures differ only by the reference set and the game count. Anything
S088 reports that differs from 2570 is the instrument moving, not the engine.

### The engine, identified rather than trusted

Cloned at `--depth 1 --branch v21.0`, `git rev-parse HEAD` = `6dc8c9cd`,
`git describe --tags` = `v21.0`. Built with the project's own
`utils/unix_build.sh` at `ARCH=x86-64-bmi2` -- a two-pass PGO build,
`-fprofile-generate` then `bench` then `-fprofile-use -flto` -- which compiled
clean under the engine's own `-Wall -Wextra -Werror`. `x86-64-bmi2` is the
strongest of the four targets its Makefile offers and the machine has `bmi2`
with no `avx512` anywhere in `/proc/cpuinfo`; that Makefile has no AVX target at
all, so the AVX-512 constraint never binds.

`printf 'uci\nquit\n'` answers **`id name Stash v21.0`**, seven option lines, no
`Threads`, no book option, no NNUE or `EvalFile` option. `Hash` is
`spin default 16 min 1 max 131072`, so the harness's 64 MB is inside it.
`strace -e trace=openat` over a full session opens nothing outside libc and the
locale archive, and `ldd` lists only `libc`, `linux-vdso` and `ld-linux`: **no
external network file is needed and none is read.** Driven the way a GUI drives
it, it reaches depth 15 in 2 s at 3.76 Mnps and exits 0 on `quit`.

md5 `5117fbc26f9eb0b91480b8328985f435`. It is an identity record and not a
rebuild check -- PGO counters vary run to run, so the tag does not reproduce the
hash. `references.tsv` says so at the row.

### A defect in the engine, found before it could contaminate anything

Stash v21.0 deadlocks at startup when the commands are already in the pipe at
`exec` time. `main()` creates `engine_thread` and enters `uci_loop` with no
handshake; `engine_thread` parks on an unconditional `pthread_cond_wait` with no
predicate re-check, so a `go` that arrives first loses the broadcast, leaves
`g_engine_mode` at `THINKING`, and `wait_search_end()` blocks for good. Silent,
not a crash.

Characterised rather than worked around:

| client behaviour | trials | hangs |
|---|---|---|
| shell `printf` pipe, everything buffered before exec | 5 | **5** |
| spawn, then write each line back to back | 25 | **15** |
| spawn, then write the whole handshake in one write | 200 | 0 |
| spawn, wait for `uciok`, then send the rest | 200 | 0 |
| full GUI pacing, 12 concurrent, real `go wtime/btime` | 240 | **0** |

The last row is the match condition. fastchess spawns then waits for `uciok` and
`readyok`, so the window is shut, and `rating.sh`'s `identify()` sends only
`uci` and `quit`, which never reaches the search. Kept in the record because the
symptom would be a hung engine scored as a time loss -- which `rating.sh`
invalidates the whole run on rather than averaging in.

### rating.sh: the set size was wired into it, and wrongly

Both `ROUNDS` defaults carried comments computing the total against **three**
pairings, which was already wrong for the four engines DEC-069 installed --
`--bracket` printed 204 in a comment and played 272. Corrected to state that
`ROUNDS` is rounds *per pairing* and that the total scales with the manifest.

The rated default moves **167 -> 334 rounds**, 668 games per pairing. That is
S087's own measurement, not a preference: 334 games per pairing returned +/-34,
outside the +/-30 the procedure requires, and 668 returned +/-24 to +/-28. In a
gauntlet only chesso plays everybody, so the graph is a star and chesso's rating
under a given anchor comes from that one pairing alone; a fifth rung adds a
fifth independent estimate and narrows none of the other four. The old 167 could
not have met the criterion at any set size.

### Bracketing run: PASSED, and the new rung is inside the band

2026-08-18, **340 games**, `10+0.2`, `Hash=64`, concurrency 12,
`8moves_v3.pgn`, **15 m 25 s**. Evidence `adocs/data/S088_bracket.pgn` and
`S088_bracket_h2h.txt`.

**Zero time forfeits**: 291 `adjudication`, 49 `normal`, nothing else, and no
fastchess log line matching a time loss or a disconnect. The identity guard
passed all five, including the first row whose `uci_name` differs from its
`ccrl_name`.

| reference | CCRL Blitz | games | chesso score | implied diff |
|---|---|---|---|---|
| Leorik 2.4 | 2829 | 68 | **19.1 %** | -252.8 |
| **Stash v21.0** | **2713** | 68 | **30.9 %** | **-141.2** |
| Blunder 8.5.5 | 2664 | 68 | 41.9 % | -57.2 |
| Leorik 2.1 | 2568 | 68 | 41.2 % | -62.5 |
| Blunder 7.1.0 | 2389 | 68 | 71.3 % | +159.7 |

Overall 40.9 % over 340 games, +107 =64 -169. Both gates clear with margin --
19.1 % is well above 10 %, 71.3 % well below 90 % -- and the added engine's
30.9 % is inside the 10 % to 90 % band its own `accepts` clause names.

### Rated run 1 at concurrency 12: INVALID, and it is the most useful thing this step produced

3340 games, `10+0.2`, `Hash=64`, **concurrency 12**, **2 h 27 m 28 s**. Evidence
`adocs/data/S088_rated_c12_INVALID.pgn`, `_h2h.txt`, `_summary.txt`.

**3 time forfeits, all Stash v21.0**, by 1118 ms, 1309 ms and 149 ms at rounds
619, 774 and 884; all three games went to chesso. `rating.sh` printed
`RATING-RUN-INVALID`. One forfeit invalidates and three did.

They are **not** the DEC-072 startup deadlock: that hangs indefinitely and would
fire at the first `go`, and these are millisecond overshoots spread through the
middle of a run whose first 618 rounds were clean.

**The forfeits are not the finding. The anchors are.** Same binaries, same time
control, same engine as S087 -- `git diff` empty over `src/`, `tests/`,
`CMakeLists.txt` and `cmake/` between `a9f2b33` and `c2f1c43`:

| anchor | S087, 2672 g | S088 @12, 3340 g | shift |
|---|---|---|---|
| Blunder 7.1.0 @ 2389 | 2559.3 | 2546.4 | -12.9 |
| Leorik 2.1 @ 2568 | 2492.5 | 2485.3 | -7.2 |
| Blunder 8.5.5 @ 2664 | 2574.6 | **2623.7** | **+49.1** |
| Leorik 2.4 @ 2829 | 2575.6 | **2606.9** | **+31.3** |
| Stash v21.0 @ 2713 | -- | 2590.7 | -- |

**The solve was ruled out before the games were blamed.** Re-solving S087's own
combined PGN with the identical `ordo 1.2.6 -s 1000 -F 95 -n 12 -W -D` command
reproduces 2559.3 / 2492.5 / 2574.6 / 2575.6 and every interval **exactly**, so
nothing about the anchor sweep or the tooling changed. The difference is in the
games: chesso scored **44.3 % against Blunder 8.5.5 against S087's 37.6 %**, 6.7
points on 668 games each, about 2.5 standard errors, while Leorik 2.1 and
Blunder 7.1.0 moved by under 1.5 points.

Removing the three forfeit games was measured rather than assumed: it moves only
Stash's solve, 2590.7 -> 2588.3, **2.4 Elo**, every other anchor identical to a
decimal. So the forfeits themselves are immaterial -- which is the point. They
are a *signal about the other 3337 games*, that oversubscription is damaging
foreign engines unequally, and the anchors are where that damage lands.

DEC-067 accepted concurrency 12 against foreign engines specifically because
that risk would be **detected rather than avoided**. It was detected. DEC-073
takes the remedy DEC-067 pre-authorised.

**The spread does not answer the step's question yet and must not be quoted from
an invalid run.** For the record of what was seen and not concluded: five
anchors span 138.4 Elo and the four excluding Leorik 2.1 still span 77.3, both
worse than S087's 83.1. If that survives a valid run, the step's premise -- that
a third family arbitrates a two-family disagreement -- is wrong, and the
`accepts` clause requiring a named dissenter has no single dissenter to name.
The concurrency-6 run decides it; nothing is concluded here.

### Rated run 2 at concurrency 6: launched

Same five binaries, same book, same `10+0.2`, same 334 rounds and 3340 games,
`CONCURRENCY=6` -- one game per physical core. One variable against run 1, which
is what makes run 1 worth keeping. About 5 hours.

A bracketing run at concurrency 6 was considered as insurance and rejected on
arithmetic: 3 forfeits in 668 Stash games is 0.45 %, a 340-game bracket holds 68
Stash games and expects 0.3 forfeits, so a clean bracket proves nothing. The
watcher carries an early forfeit alarm instead, so a repeat is known at game 500
rather than after five hours.

**A hazard found while writing this up:** `rating.sh`'s concurrency default is
not changed in the same turn, because **bash reads a script lazily from disk
while executing it** and the 5-hour run is executing this one. DEC-073 records
the deferral and the `touches:` amendment it implies.

### The instrument settings changed mid-step, twice, and both were the owner's

**DEC-074: the gauntlet is re-run on substantial work, not on a 20 Elo
threshold.** DEC-071's rule would have spent a night on an absolute figure
repeatedly out of the same budget the SPRTs come from. `plan.md`, `specs.md` and
`DEV_MANUAL.md` are amended in this step's commit. The trigger is deliberately
not derivable: an agent proposes a gauntlet, it does not book one.

**DEC-075: rating runs saturate the machine at concurrency 12, and a low
time-forfeit rate is tolerated rather than voiding a run.** This reverses
DEC-073, which is marked VOID as to its decision with its evidence intact.

Two things about that reversal belong in this step's record rather than only in
the decision log.

**The measurement the owner asked for, taken on the run in flight:** concurrency
6 was using **6.2 of 12 threads** at **10.6 games/min**, against concurrency
12's **22.6**. A game runs two engine processes but only the side to move
thinks, so half the hardware threads sat idle and the cost was **2.1x**, not the
~1.3x an SMT-only penalty implies. The owner was right and the size of it was
understated.

**DEC-073's central inference was wrong and this step is where it was made.** It
read the +49.1 Elo shift on Blunder 8.5.5 as oversubscription damaging foreign
engines unequally. **S087 ran at concurrency 12 as well**, so concurrency was
identical across the two runs and cannot explain a difference between them. What
survives is narrower and still worth having: the forfeits show Stash is
time-stressed at 12, and **the anchor shift is unexplained** -- candidates being
the set going from four engines to five, and run-to-run variance at about 2.7
standard errors. It is recorded as unexplained rather than attributed.

**This step's `accepts:` IS amended, and the paragraph that said otherwise was
overtaken within the hour.** It demanded zero time forfeits, and at 1269 of 3340
games the concurrency-6 run took one -- a 25360 ms hang, not a margin overrun.
DEC-076 is the owner's decision that the tolerance applies to this run as well,
so DEC-075 decision 3 now concerns concurrency only: this run stays at 6, every
later run is at 12, and the gate on all of them is 1 % of an engine's own games. **The results file must say so**, because it makes S088's
number and its successors readings taken at two different instrument settings --
the same class of discontinuity DEC-073 noted between S087 and S088 and got the
cause of wrong.

### Rated run 2 at concurrency 6: VALID, and the answer

3340 games, `10+0.2`, `Hash=64`, **concurrency 6**, **5 h 02 m 49 s**. Evidence
`adocs/data/S088_rated_c6.pgn`; full write-up
`adocs/data/rating_2026-08-18_S088_ccrl_blitz.md`.

2807 `adjudication`, 532 `normal`, **1 `time forfeit`** -- Stash at **0.15 %** of
its own 668 games, inside the 1 % DEC-075 sets, so the run stands under DEC-076.
chesso gained 1 point from it.

| anchor | CCRL | chesso | 95 % |
|---|---|---|---|
| Blunder 7.1.0 | 2389 | 2533.8 | +/-24.8 |
| Leorik 2.1 | 2568 | **2476.7** | +/-24.0 |
| Blunder 8.5.5 | 2664 | 2586.0 | +/-23.7 |
| Stash v21.0 | 2713 | 2597.6 | +/-24.4 |
| Leorik 2.4 | 2829 | 2598.5 | +/-27.7 |

**Every interval is inside +/-30, on one run rather than the two S087 had to
combine.** The 334-round default is what bought that.

### The step's premise was wrong, and that is the result

S088 was written on "two families cannot arbitrate a disagreement between two
families; a third can". **A third family did not close the spread: it went up,
83.1 Elo over four anchors to 121.8 over five.** Leorik 2.1 is still the low
outlier, and dropping it leaves **64.7** -- twice the allowance -- so the
`accepts` clause asking for "the dissenting engine" has no single dissenter.

What the third family bought is the *shape*. Ex-Leorik 2.1 the solved rating
rises monotonically with the anchor's own rating and flattens at the top:
2389 -> 2533.8, 2664 -> 2586.0, 2713 -> 2597.6, 2829 -> 2598.5. Across a CCRL
span of 440 Elo the measured differences span **375.3, a ratio of 0.853**. That
is **compression, not scatter**, and S087's argument that a scale artifact was
excluded -- its two extreme anchors agreed to 16 Elo -- no longer holds at 668
games a pairing with a fifth rung.

**No reference set fixes this.** The leading candidate is the time control:
CCRL Blitz is "equivalent to 2'+1" on an i7-4770K" and this runs at `10+0.2`.
Attacking that is not this step's and is nobody's yet.

**The quoted figure is the five-anchor mean, 2559, SOFT** -- the lower number
carrying the larger spread, with 2579 reported beside it and not quoted, because
dropping an inconvenient reference is how a measurement gets talked into a nicer
answer. DEC-077, written to be overridden.

**chesso did not move.** `src/` is byte-identical to S087's rated commit and
S087's 2570 sits between 2559 and 2579. The instrument changed; the engine did
not. For a step whose only product was a reference engine, that is the right
outcome and it is also the check that the instrument is not drifting.

### Left open, with numbers rather than an explanation

chesso scored **37.6 %** against Blunder 8.5.5 in S087 (4 engines, concurrency
12), **44.3 %** in the voided run (5 engines, 12) and **39.1 %** in the valid one
(5 engines, 6). The five-engine concurrency-12 run is the outlier of the three by
5 to 7 points on that pairing alone. Stash is a PGO C engine at 3.76 Mnps sharing
a 12-way pool with a .NET engine and two Go engines, so pool contention is a
plausible mechanism and three single runs cannot establish it. DEC-075 puts every
later run at concurrency 12, so **the next rating run is the test**: ~44 % against
Blunder 8.5.5 again means the pool composition is real and DEC-075 needs
revisiting.

### Three defects found and fixed while doing this, all in the harness

1. **`S088_solve.sh` never created its output directory** and failed on a clean
   path. Found by running it rather than by reading it.
2. **A watcher's `grep -c ... || echo 0` produces `0\n0`**, which makes every
   integer test against it an error that evaluates false. The early forfeit alarm
   could not have fired for the first 45 minutes of a five-hour run. Fixed, and
   the fix proven non-vacuous against a log that did forfeit.
3. **A pid recovered with `pgrep -f ... | head -1`** after a launch returned an
   already-exited transient, and the watcher declared a live run dead at game 15
   of 3340. `$!` is the correct capture. All three are in `DEV_MANUAL.md`.
