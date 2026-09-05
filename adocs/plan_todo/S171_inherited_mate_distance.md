id:         S171
goal:       a reported mate line reaches its mate even where the table has lost a slot the walk needs, closing the 5 `Incomplete mating PV` lines from 1 search in 3000 games S170 left -- **rescoped by DEC-127**, the original goal having assumed a wrong mate distance that measurement refuted
accepts:    the case S170 measured is reproduced and its mechanism established from the code rather than argued -- **done, and it refuted the premise this step was created on (DEC-127)**: the reported `mate -9` is deliverable, an 18-ply line from that root is legal throughout and ends in checkmate, and the same warm replay at `Hash=256` prints it and warns about nothing, so what fails at `Hash=16` is the walk and not the score; a minimized failing test observed red before any fix, driving the engine the way `tests/test_mate_carry.cpp` does because a cold table does not reproduce it; the fix decided by SPRT if it alters play at all and by identical `tools/search_bench.py` node counts and best moves if it does not (INV-6); a `fastchess.sh --fast` run reports **0** `Incomplete mating PV` lines from the candidate, which is the residual S170 could not close and the reason this step exists; `MANUAL.md`, `adocs/specs.md` and `DEV_MANUAL.md`'s instrument 3 lose the residual they state today in the same commit that removes it
touches:    src/search.cpp, tools/, tests/, adocs/data/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md
excludes:   **amended by DEC-127** -- the reporting side is now this step's ground, because the residual is a line the walk could not build and not a score; anything that alters play, including making the table keep entries longer, resizing it or changing its replacement policy, each of which owes an SPRT of its own; improving mate *finding*, which is S148's and S154's ground, and which is where a search naming a longer mate than the position's own value belongs
decisions:  DEC-122, DEC-123, DEC-125, DEC-127
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-03

## What this is

S170 removed three ways a true mate score could be reported with a line too
short to reach it, and one `Incomplete mating PV` line survived its 3000-game
run -- five `info` lines from a single search. That search's **score** is the
defect, not its line.

**The measurement, from S170's run, 2026-09-02.** Position
`8/4ppk1/2p2np1/p7/NPP1p3/P6q/3b4/1Q3R1K w - - 0 34`, reached 50 plies into a
game whose root and moves are the `E_mate_minus9` row of
`adocs/data/S170_cases.tsv`:

| asked | answer |
|---|---|
| the engine, in the game, at depths 9 to 13 | `mate -9`, 18 plies, with lines of 9 to 13 plies |
| the engine, cold table, depth 18 | `mate -7`, 14 plies, complete line |
| the engine, cold table, depths 19 to 24 | `mate -7`, held over 7.3 billion nodes |
| `stockfish`, depth 30 and depth 36 | `#-7` |

So the distance reported in the game is not the distance the position holds,
and no 18-ply line exists for S170's completion to publish. The all-or-nothing
rule (DEC-122) did the right thing by refusing; what is left is the score.

**Why it matters beyond the warning.** A mate score is not only printed. It is
stored, it answers later nodes through `tt_entry_answers()`, and a distance
that is wrong by two moves is a score that orders and cuts on a claim the
position does not support. Whether that costs anything in play is unmeasured
and is one of the things this step is for -- a verdict of zero is a valid
outcome and is recorded as zero (DEC-019, and S005, S006 and S015 are the
precedents).

## Where to look first, and what has already been ruled out

`normalize_score()` and `de_normalize_score()` (`src/search.cpp`) are the
distance's ply arithmetic and they round-trip by construction; the aspiration
loop in `iterative_deepening_search()` (`src/chesso.cpp`) was checked while
S170 was measured and is **not** the cause -- it re-searches with the full
window whenever a mate score falls outside the band and loops until the score
is inside it, so a non-aborted iteration's root score is not a bound.

That leaves the entry itself: which search wrote `mate -9` for that position,
at what depth, and whether it was exact or a bound when it was written. Three
candidates worth separating before choosing a fix:

- **A bound stored as one and read as exact.** `tt_entry_answers()` returns a
  `TT_BETA_NODE` or `TT_ALPHA_NODE` score to the caller when it passes the
  window test, which is ordinary alpha-beta; a mate *bound* carried up to the
  root and printed as an exact distance would look exactly like this.
- **A key collision.** `tt_get_entry()` compares the full 64-bit key and a
  64-bit key still collides; S147's `extend_mate_pv()` comment already names
  this as the reason a stored move is re-validated against the generator.
- **A mate proved on a reduced tree.** Late move reduction re-searches on a
  fail-high, so a mate that survives a reduction should be re-proved -- but
  "should" is an argument and this step does not run on arguments.

The first thing to build is the instrument that says which: a way to see the
entry that answered, its depth, its type and the search that wrote it, on the
reproduction above. `tools/truncation_scan.cpp` and `tools/probe_cost.cpp` are
the precedents for a tool that reads the engine's own tables.

## Why it is first in the Open list

The BUGS rule: a bug that has been found gets fixed before anything else
starts, because a known defect in the tree contaminates every measurement taken
after it. This one is found, reproducible in about twelve seconds, and it sits
in the score -- which is the input every later measurement in the search block
is taken against.

## What was measured, 2026-09-03

Everything above this line was written before the measurement. It assumed the
score was the defect. It is not, and DEC-127 is the correction.

**The instrument.** `tools/mate_trace.cpp`, built for this step: it replays a
game through the real UCI layer -- one process, one table -- then walks a
reported line over a board of its own and prints the entry behind every
position, following the table's own best move past the end of the line. At a
stall it prints every legal move and what the table holds one ply on. Nothing
it does searches a node or writes an entry.

**What it showed.** On the reproduction at `Hash=16`, `go depth 11`, node count
353576, identical to the in-game search:

| ply | entry |
|---|---|
| 0 to 9 | exact, generation 12 (this search), depths 11 down to 2, every one claiming the same root-relative score |
| 10 | **miss** -- covered by the reported `pv`, which is 11 plies long |
| 11 | exact, generation 11 (the previous search), depth 7, black mates in 4 |
| 12 | exact, generation 11, depth 6, best move `g2g4` |
| 13 | **miss** -- the walk stalls here, five plies from the mate |

The generation-11 entry at ply 11 is honest: `stockfish` at depth 30 gives
`Mate(+4)` for black on that position. And the claimed distance is deliverable:

```
h1g1 d2e3 f1f2 h3g3 g1h1 e3f2 b1f1 g3f3 f1g2 f3d1 h1h2
f6g4 h2h3 d1d3 h3g4 d3d7 g4f4 d7f5
```

18 plies, legal throughout, ending in checkmate -- replayed move by move with
python-chess. So `mate -9` is **sound and not optimal**; the position's own
value is `mate -7`, and DEC-125's "no line of the claimed length exists" was
never measured and is wrong.

**The confirmation that it is table pressure and nothing else.** The same warm
replay at three hash sizes, depths 8 to 13:

| Hash | what the engine reports |
|---|---|
| 16 MB | `mate -7` at 8 to 10, `mate -9` at 11 and 12 with an 11- and 12-ply line -- short, warns |
| 64 MB | `mate -7` at 8 only, centipawns from 9 on |
| 256 MB | `mate -9` at 9 to 12 with a **complete 18-ply line** -- no warning |

At 256 MB the engine publishes the line by itself. What differs is only which
entries survive.

**The fix.** `certified_mate_move()` in `src/search.cpp`. When the walk has
nothing to read -- neither the proven line nor an entry with a move -- it looks
one ply down and takes the move whose child carries an **exact** score at
exactly the distance the line still owes. A collision takes one slot at a time
and a mating line's nodes are scattered across the table, so the children of a
lost position usually still have theirs. Exact entries only: a lower bound of
"mate in n" leaves a faster mate open, and the all-or-nothing gate would catch a
walk that fails to reach the mate but not one that reaches it by a road the
position would not take.

**Red first, then green.** `adocs/data/S170_cases.tsv`'s `E_mate_minus9` row
becomes `guard: yes`. Without the fix `tests/test_mate_carry.cpp` reports
`6 of 9 mate lines do not reach their mate` -- the four `mate 8` lines at ply 49
at 5, 6, 7 and 9 plies of 15, and the two `mate -9` lines at ply 50 at 11 and 12
plies of 18. With it, 0 of 9, and the mate-line count is 9 either way, which is
what says the change is reporting-only. 7.9 s.

## Postponed to the desktop workstation, 2026-09-03 (DEC-128)

The fix is in and green at `136b03f`. **One thing is owed and it is a machine,
not a change**: the `fastchess.sh --fast` census the `accepts:` names.

Two attempts here failed for machine reasons:

| attempt | what stopped it |
|---|---|
| morning | `pmset -g ac` = `No adapter attached`; the POWER rule forbids a timed match on battery (DEC-109) |
| afternoon, on mains | `fastchess.sh`'s load guard: `about 387% of a core is already busy` -- Spotlight indexing PDFs through ten `CGPDFService` workers beside `mds_stores`, about half of eight cores. Killed after about a minute, `games.pgn` zero bytes |

**The run, so whoever resumes does not re-derive it:**

```bash
REF=457e355 nohup ./fastchess.sh --fast > .tuning/sprt_s171_matepv.log 2>&1 &
```

3000 games at 8+0.08, `Hash=16`, `UHO_Lichess_4852_v1.epd`, concurrency all
cores. `457e355` is the commit before the fix, so the reference prints its own
`Incomplete mating PV` count in the same match. **Accept at 0 from the
candidate.** Arm a watcher on `SPRT-RUN-(DONE|FAILED)` per DEC-061, ceiling at
least 2x the expected two hours.

Before starting it: `pmset -g ac` must report an adapter, and
`ps aux | sort -rnk3 | head` must be quiet -- the script's own guard is the
check and it fired here.

**The census is not a figure carried across machines.** Both engines play in
the same run, so the reference's count is measured beside the candidate's and
the workstation needs no re-baseline (DEC-049 untouched). The standing **5
lines from 1 search in 3000 games** is this MacBook's and stays attributed to
it until the workstation's run replaces the pair.

**On finishing there:** the run's count is the only outstanding item. Then
`adocs/specs.md` (the paragraph ending "S171's own run is owed and the figure
is not restated until it is taken") and `DEV_MANUAL.md`'s instrument 3 table
and the paragraph under it take the measured number, the `done:` stamp is
written, and the file moves to `adocs/plan_done/`.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

The engine change is committed at `136b03f`: `certified_mate_move()` in
`src/search.cpp`, the guarded `E_mate_minus9` row of `adocs/data/S170_cases.tsv`,
`tools/mate_trace.cpp`, and the three documents rewritten to DEC-127's reading.
What is left is a measurement this MacBook could not take (DEC-128): one
`fastchess.sh --fast` match, the working tree against the commit before the fix,
read for how many `Incomplete mating PV` warnings each side produced. DEC-144
makes it the first run taken on the Linux workstation. Nothing in `src/` moves;
the deliverables are the run, two document edits, the `done:` stamp and the move
to `adocs/plan_done/`. A candidate count of 0 completes the step; anything else
does not, and section 7 says what follows.

### 2. The technique as published

The instrument is fastchess's `-check-mate-pvs`, described in its manual as
"Check that PVs for mate scores have the correct length and end in checkmate"
(https://github.com/Disservin/fastchess/blob/master/man.md). It runs over every
`info` line either engine prints and reports a failure as a four-line block on
standard output: `Warning; Incomplete mating PV - from <engine name>`, then
`Info;` with the offending line, then `Position;` and `Moves;` with the game so
far. `fastchess.sh` names its sides `candidate` and `ref-<sha>` (its `-engine
... name=candidate` and `name="ref-$ref_sha"` arguments), so the name in each
warning says which build produced it.

The lane is a census, the fixed-length run the field uses for tracking rather
than deciding: fishtest's regression tests are 60000 games at a fixed length on
this same `UHO_Lichess_4852_v1.epd` book
(https://github.com/official-stockfish/Stockfish/wiki/Regression-Tests), and the
fishtest FAQ prefers a fixed game count to an SPRT for a measurement
(`adocs/testing_strategy.md` section 1.1, "Fixed games", with its URL).
DEV_MANUAL.md's "Mate safety" instrument 3 is chesso's form -- "read a count and
not a silent log" -- with both sides in one run, so the reference's count is
measured beside the candidate's and nothing crosses machines (DEC-128).

### 3. What chesso has today, and where the change plugs in

The path at HEAD: `complete_mate_pv()` in `src/search.cpp` walks the table from
the end of a mate line and extends it only when the walk ends in checkmate at
exactly the claimed distance (DEC-122). Where nothing can be read -- no stored
move, no table move, no answer from `proven_mate_move()` in `src/search.cpp` --
it calls `certified_mate_move()` in `src/search.cpp`, which makes each legal
move, probes the child with `tt_get_entry()` and returns the move whose child
holds a `TT_PV_NODE` entry whose `de_normalize_score()` equals what
`mate_score_at()` in `src/search.cpp` requires at that ply for the distance still
owed; bounds are refused (DEC-127). `tests/test_mate_carry.cpp`, `TEST_CASE("a
mate score carried across searches keeps a line that reaches it")`, replays the
five rows of `adocs/data/S170_cases.tsv` through one process, asserts at least
`expected_mate_lines()` mate lines per case first and then that none is short:
6 of 9 short without the fix, 0 of 9 with it.

The harness at HEAD, since this step drives it and not the engine. `fastchess.sh
--fast` sets `elo0=0 elo1=10 alpha=0.10 beta=0.10` and `rounds=1500` with
`-repeat`: 3000 games is the cap and an SPRT bound the only earlier exit.
`option.Hash=16`, `option.Threads=1`, `tc="8+0.08"`, `-check-mate-pvs`;
concurrency is `all_cores`, on Linux `nproc`, 12 on the workstation with its SMT
siblings (DEC-050). The run's own files land in `/tmp/chesso_sprt_fast_<stamp>/`
as `games.pgn` and a `fastchess.log` that is WARN-and-above and routinely empty
(DEV_MANUAL.md "One trap when reading a fastchess.sh result"). **The warnings
this step counts are on the script's standard output**, which the detached
command in section 7 redirects into `.tuning/sprt_s171_matepv.log`; its last line
is `SPRT-RUN-DONE fast <dir>` or `SPRT-RUN-FAILED: ...` on every other exit
(`fail()` and the EXIT trap in `fastchess.sh`), which the watcher exits on.

### 4. Constants and seeds

None: no `SEARCH_PARAM` in `src/search_params.hpp` moves, and the run parameters
are the harness's standing configuration, which S198's `excludes` also forbids
moving. One derived figure, for scheduling. `adocs/testing_strategy.md` section
1.1 prices a pair whose truth sits on a bound at
`2 C^2 [(1-a) ln((1-b)/a) - a ln((1-a)/b)] / (e1-e0)^2`, C = 347.44, which
reproduces its 639770 at a = b = 0.05; at `{0,10}`, a = b = 0.10, that is about
**4244 games** (form b, over the formula as the repository records it). Two
builds that play identically sit exactly on `elo0`, so the cap usually ends the
run -- S170's census reached 3000 at `LLR: -0.73` -- but a bound before 3000 is
not rare; section 7 reads that case.

### 5. Interactions and traps

- **POWER.** Linux has no `pmset` and a desktop has no battery. The check is
  `cat /sys/class/power_supply/*/type`: each supply's `type` is `Battery`, `UPS`,
  `Mains`, `USB` or `Wireless`
  (https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power); no
  `Battery` line, or an empty directory, satisfies the rule, and the stamp says
  which was seen. `fastchess.sh` checks no power state, only the load below.
- **The load guard is a warning, not a refusal.** `busy` in `fastchess.sh` sums
  `ps -A -o %cpu=` and prints `WARNING: about N% of a core is already busy` above
  60, then plays anyway. Read the first twenty log lines within a minute of
  launch; if the warning is there, `kill <pid>` (the trap writes `SPRT-RUN-FAILED:
  exited 0`, as the 2026-09-03 log above did), find the load with `ps aux | sort
  -rnk3 | head`, wait for it to clear, relaunch under a new log name. Do not
  lower `CONCURRENCY` to share the machine (MACHINE rule; DEC-128 rejected that).
- **Do not edit `fastchess.sh` while it runs**: bash reads a script
  incrementally, so an edit lands in the running process at an arbitrary point.
  **S198's `-srand` and `-pgnout` edits wait for `SPRT-RUN-DONE`.** Nor touch
  `.ref-builds/457e355/`: the reference runs from there directly
  (`reference="$ref_dir/build/src/chesso"` in `fastchess.sh`), not from a
  snapshot. `build/` may be rebuilt -- the candidate is snapshotted before game
  one -- but a timing needs the idle machine, so S189's and S179's proofs wait.
- **The two sides must play identically, checked and not assumed.** Every `src/`
  commit between `457e355` and HEAD discharged INV-6 on identical node counts --
  `136b03f`, then S172, S146, S174, S175, S176 in their `plan_done/` stamps;
  `18deccf` is a comment in `src/bb_tables.hpp` -- and the default configuration
  has `OwnBook` false, so no book is probed. Section 7 runs
  `tools/search_bench.py` on both binaries before launch: 121512 / 800769 /
  62907 at depth 9 with `c3d5` / `e2a6` / `d7c8q`, or stop, since the run would
  otherwise be an SPRT between different engines. Those are the DEC-049
  machine's own figures, so glibc's `log` in `build_lmr_table()`
  (`src/search.cpp`) is no risk there (`adocs/status.md`, the 2026-08-23 handover).
- **Outside the repository.** `fastchess` on `PATH` (`fastchess --version`;
  alpha 1.8.1 accepts `-check-mate-pvs` and ran S170's census); `cmake`, `ninja`,
  `ccache`, which the reference build in `fastchess.sh` uses; the book via
  `./books/fetch_book.sh`, without which the script refuses; `mkdir -p .tuning`,
  gitignored; `build/src/chesso` configured Release per DEV_MANUAL.md "Build"; a
  `.moltke.local.md` for the workstation, since the MacBook's does not travel.
- **WATCHERS, DEC-061.** Four exits, poll never follow, ceiling at least twice
  the expected run; arm through `Monitor` with `persistent: true` where that tool
  exists, else poll next turn and claim no watcher. `OUT` is never reused:
  `fastchess.sh` refuses an existing `games.pgn` because fastchess appends.

### 6. Tests

No new test and no mutant are owed: the guard is `tests/test_mate_carry.cpp`,
observed red then green at `136b03f`; `certified_mate_move()` is reporting, not a
pruning, reduction or extension rule; INV-6 is discharged. Before the stamp the
gate is green in both builds on the workstation, `-j12` there:

```bash
cmake --build build -j12 && ctest --test-dir build -L fast --output-on-failure && \
cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast --output-on-failure && \
./clang-format.sh --check
```

DEC-141's Debug self-play: `136b03f` is in `src/search.cpp` and calls
`make_move()` and `unmake_move()` on the reporting path; the rule postdates the
commit by two days but asks for the run "before completing", and it costs under
two minutes, so take it and say so in the stamp. Configure `build-debug` per
DEV_MANUAL.md "Build", then, mirroring `fastchess.sh`'s `-each` and `-openings`:

```bash
cmake --build build-debug -j12 --target chesso
fastchess -engine cmd=build-debug/src/chesso name=dbg-a \
          -engine cmd=build-debug/src/chesso name=dbg-b \
          -openings file=books/UHO_Lichess_4852_v1.epd format=epd order=random \
          -each tc=4+0.04 option.Hash=16 option.Threads=1 -rounds 4 -repeat \
          -concurrency 12 -check-mate-pvs -pgnout file=/tmp/s171_debug_selfplay.pgn \
          > .tuning/s171_debug_selfplay.log 2>&1
grep -c 'Assertion' .tuning/s171_debug_selfplay.log      # 0 is the expected count
```

A Debug binary at 4+0.04 may lose on time; that is not an assertion and is
reported, not counted. S190 lands the canonical command in DEV_MANUAL.md "Test".

### 7. Measurement

Lane: **census.** No bounds pair is pre-registered and no `adocs/data/S171_sprt.sh`
is written: nothing is decided by SPRT, the fix being behaviour-neutral (INV-6 at
`136b03f`) and the quantity read a count of warnings per engine; the `--fast`
pair only sets when the run can end before 3000 games. DEC-143's A/A is not a
precondition: it calibrates the pair variance and forfeit rate that price an Elo
verdict, and a census reads neither. It is S198's run and lands before S148, the
first verdict. Duration: 3000 games at the 2340 games/h the workstation measured
on 12 threads at 8+0.08 (`adocs/status.md`, "This machine did 2340 games/h on 12
threads for comparison") is about **1 h 17 m**; the two hours quoted above was
the MacBook's throughput.

In bash, from the repository root on the workstation:

```bash
cat /sys/class/power_supply/*/type 2>/dev/null   # no `Battery` line: POWER holds
ps aux | sort -rnk3 | head                         # quiet before launch
./books/fetch_book.sh && mkdir -p .tuning && cmake --build build -j12
# The reference, built where fastchess.sh looks for it, with its own commands;
# skip these three lines if .ref-builds/457e355/build/src/chesso already exists.
git worktree add --detach .ref-builds/457e355 457e355
cmake -S .ref-builds/457e355 -B .ref-builds/457e355/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build .ref-builds/457e355/build --target chesso -j12
python3 tools/search_bench.py ./build/src/chesso 9
python3 tools/search_bench.py .ref-builds/457e355/build/src/chesso 9   # identical, or stop
REF=457e355 nohup ./fastchess.sh --fast > .tuning/sprt_s171_matepv.log 2>&1 &
echo $! > .tuning/sprt_s171_matepv.pid
```

The watcher, in the WATCHERS rule's shape, ceiling 4 h, over three times the
expected run:

```bash
bash -c '
  end=$(($(date +%s)+14400))
  until grep -qE "SPRT-RUN-(DONE|FAILED)" "$1"; do
    kill -0 "$2" 2>/dev/null || exit 3
    [ "$(date +%s)" -ge "$end" ] && exit 124
    sleep 30
  done' _ .tuning/sprt_s171_matepv.log "$(cat .tuning/sprt_s171_matepv.pid)"
```

Progress without disturbing the match: `grep -c '^Finished game'
.tuning/sprt_s171_matepv.log`. After `SPRT-RUN-DONE`:

```bash
L=.tuning/sprt_s171_matepv.log
grep -c 'Incomplete mating PV - from candidate' $L      # the accepts: must be 0
grep -c 'Incomplete mating PV - from ref-457e355' $L    # the paired baseline
for e in candidate ref-457e355; do                      # distinct searches per side
  grep -A3 "Incomplete mating PV - from $e" $L | grep -E '^(Position|Moves);' \
    | paste - - | sort -u | wc -l
done
grep -E '^Games:|^LLR:|^Total Time' $L; tail -8 $L      # game count, LLR, forfeits, marker
```

Checked against `.tuning/S170_fast.log` on this MacBook: the same greps give 5
and 12 lines and 1 and 3 distinct searches, the figures S170's stamp records.

Three outcomes. **0 from the candidate**: the accepts holds. The stamp names the
run directory, the game count (3000, or fewer with the `LLR:` line if a bound
ended it), the duration, the forfeit count, both counts, both distinct-search
counts, and the power and load readings. The reference's count is the paired
baseline for the same defect class -- `457e355` has S170's fixes (`d08ab52` is
its ancestor) and not this step's -- recorded as measured, never compared to the
MacBook's 5. **More than 0 from the candidate**: the step does not complete and
nothing else in `src/` starts. Each warning's `Position;` and `Moves;` lines
become a row of `adocs/data/S170_cases.tsv` with `guard` `no`, as S170 did for
`E_mate_minus9`; reproduce with `python3 adocs/data/S170_replay.py --cases
adocs/data/S170_cases.tsv --only <name> --go "nodes <N>"`, read the table behind
it with `build/tools/mate_trace` (DEV_MANUAL.md instrument 3 has the
invocation). A fifth cause is a new step; a case this fix should have covered is
a bug in this step, fixed before anything else; either way a decision records
the reading, as DEC-125 and DEC-127 did, and the documents keep their "owed"
wording until a count reads 0. **The machine cut it short** -- `SPRT-RUN-FAILED`,
a kill on the load warning, watcher exit 3 or 124: nothing is read from that log;
fix the cause, relaunch. A forfeit rate not near zero is recorded and looked into
before the next run; it does not void a count of warnings, since both sides pay
it.

### 8. Completion checklist

1. The section 6 gate green in both builds; the Debug self-play's `Assertion`
   count read.
2. `adocs/specs.md`: "S171's own run is owed and the figure is not restated until
   it is taken" becomes the workstation's figure -- both counts, the game count,
   the machine -- and the rate list before it ("**138** ... **10** ... **5**")
   gains S171's.
3. `DEV_MANUAL.md` instrument 3: the row `S171 | not yet measured | -- | run
   postponed, DEC-128` takes the counts and the game count; the paragraph
   beginning "The standing figure is still 5 lines from 1 search in 3000 games"
   is rewritten to the new figure and its machine, "Read a count against 5"
   becomes a count against it, and the two postponement sentences go.
4. `MANUAL.md`: its known-bugs entry already carries DEC-127's reading and states
   no residual count. Check it, conclude no change, say so in the stamp.
5. `python3 tools/plan_prose_check.py --touches
   adocs/plan_todo/S171_inherited_mate_distance.md` prints `touches flagged: 0`.
6. The `done:` stamp, written last; `git mv` to `adocs/plan_done/`. `plan.md`
   (Open entry 3 out, "Done recently" in) and `status.md` are the coordinator's.
7. One commit: imperative subject under 72 characters, body says why and names
   S171 and INV-6. It touches no `src/`, so neither `Bench:` nor `No functional
   change` is owed (DEC-140 binds `src/` commits, from S189's completing commit
   on).

### 9. Sources read

- https://github.com/Disservin/fastchess/blob/master/man.md -- `-check-mate-pvs`,
  `-sprt`, `-rounds`, `-repeat`, `-srand` descriptions.
- https://github.com/official-stockfish/Stockfish/wiki/Regression-Tests --
  fixed-length 60000-game runs on `UHO_Lichess_4852_v1.epd`.
- https://dannyhammer.github.io/engine-testing-guide/strength-testing.html --
  "The first test that you run should be a 'sanity check'", the A/A practice.
- https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power -- the
  `type` values under `/sys/class/power_supply/<name>`;
  https://www.kernel.org/doc/html/latest/power/power_supply_class.html was
  fetched too and names the class without the sysfs path.
- https://cantate.be/Fishtest/normalized_elo_practical.pdf -- **unverified**: a
  binary this session could not read; the run-length formula is taken from
  `adocs/testing_strategy.md` section 1.1, which cites it.
- `.tuning/S170_fast.log` on this MacBook -- the warning block's format and the
  section 7 greps, checked against S170's recorded 5/12 and 1/3;
  `.tuning/sprt_s171_matepv.log` -- the 2026-09-03 abort's load warning and its
  `SPRT-RUN-FAILED: exited 0` on a kill.
- At HEAD `0d17d26`: `fastchess.sh`, `src/search.cpp`, `tests/test_mate_carry.cpp`,
  `adocs/data/S170_cases.tsv`, `tools/mate_trace.cpp`, `adocs/data/S170_replay.py`;
  `adocs/plan_done/S170_mate_line_across_searches.md`, `S147_mate_pv_completeness.md`
  and the INV-6 lines of S172, S146, S174, S175, S176; DEC-050, DEC-061, DEC-127,
  DEC-128, DEC-141, DEC-143, DEC-144; `adocs/specs.md`, `DEV_MANUAL.md`,
  `MANUAL.md`, `adocs/status.md`, `adocs/testing_strategy.md` section 1.1.

### 10. Questions deferred to the owner

- If an SPRT bound ends the run before 3000 games (section 4 prices the chance),
  is the census accepted as it stands -- recommended, since `accepts:` names a
  run and the reference is paired inside it -- or topped up with a second run
  under a new `OUT`? `accepts:` does not change either way; the standing
  figure's denominator does.
- A note for the stamp, not a question: the `accepts:` clause that `MANUAL.md`
  "lose[s] the residual [it] state[s] today" was satisfied at `136b03f`, which
  rewrote that entry to DEC-127's reading; `MANUAL.md` states no count now.
