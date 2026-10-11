id:         S272
goal:       one SPRT on the workstation, both engines driven through a relay that sends every position as a bare FEN, measures what S271 bought for a client that sends the board
accepts:    (1) a relay under `tools/` that sits between fastchess and the engine, rewrites each `position ... moves ...` line into `position fen <the board it reaches>`, and passes every other line both ways unchanged, with a test over a recorded command log; (2) DEC-143's fixed-rounds A/A of 1000 games through the relay, both sides the same binary, read with `adocs/data/S105_pairs.py` before the verdict; (3) pre-registered in a script under `adocs/data/` before the first game: candidate S271's landing commit, reference its parent, both through the relay, `{0, 5}` nElo, 8+0.08, Hash=16, `noob_3moves.epd`, the worst-case expected games from the nElo formula, the abort rule and the three outcomes, and the open DEC-171 findings named (S259 while it is open); (4) the conversion's prediction from `adocs/data/S271_replay.md` (about +50 Elo at Hash 16) stated beside the result and never quoted as it; (5) the result block in the closing commit (DEC-220)
touches:    adocs/data/S271_replay.md, tools/bare_fen_relay.py, tests/test_bare_fen_relay.py, tests/assets/bare_fen_relay_session.txt, tests/CMakeLists.txt, fastchess.sh, tests/test_fastchess_script.sh, adocs/data/S272_relay_session.py, adocs/data/S272_relay_latency.py, adocs/data/S272_relay_latency.txt, adocs/data/S272_aa.sh, adocs/data/S272_sprt.sh, DEV_MANUAL.md, TOOLCHAIN.md
excludes:   any engine change; the harness's default regime, which keeps the base fixed; a match on the MacBook (DEC-258 (2))
decisions:  DEC-264, DEC-143, DEC-220, DEC-258
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:

## Why

S271 lands on node identity in the harness and on the replay's node counts;
neither is a game result. This match turns the replay's conversion into a
measured number, in the one regime where S271 changes play. The owner
recorded it as validation, to run on the workstation when the machine is
free, and only if it is still needed then (DEC-264): S271 does not wait on
it.

## Lane

A workstation run, under four hours by the replay's prediction; the relay is
agent work and may be written while another run holds the machine (DEC-260).

## As built

2026-10-11, on the workstation (i7-8700K, g++ 13.3, Release), worktree at
`a7bb10e`. The agent part only: the relay, its test, the harness switch and
the two scripts. No match was run; the A/A and the SPRT are the coordinator's,
and the `done:` stamp waits for their readings.

### Deviations

- **`touches:` is amended.** It listed `adocs/data/S271_replay.md` alone; the
  relay, its test and session, `fastchess.sh` and its test,
  `tests/CMakeLists.txt`, five data files, `DEV_MANUAL.md` and `TOOLCHAIN.md`
  are added. `S271_replay.md` is not edited yet: it takes the result beside
  the prediction, and the correction below, in the finishing pass.
- **One fast test uses python-chess.** `test_bare_fen_relay` is the first
  test that does, because the relay imports it. The interpreter is the CMake
  cache variable `CHESSO_CHESS_PYTHON`, `~/.venv/chess/bin/python` by default.
  Where neither it nor the system python imports python-chess the test exits
  77, which ctest lists as skipped (the fast check's item 4, below).
  python-chess was already a dependency of `tools/`, so nothing new is
  installed. Whether the MacBook has the venv was not checked; there the test
  runs or reads skipped, never failed.
- **A finding against `S271_replay.md`'s premise, for fastchess only.** It
  says the base clear "runs inside `position`, before `go`, so it is spent on
  the engine's clock". Under fastchess it is not: fastchess sends `isready`
  after every `position`, waits for `readyok`, and starts the clock at `go`
  (recorded session below). Measured: an engine held 0.3 s on every
  `position` line by a wrapper played 44 and 79 moves of two 2+0.02 games with
  no forfeit, about 24 s of delay on a 2 s clock
  (`.tuning/coord/S272/rec/dly.*`). So this match prices the cold table and
  not the clear, and the relay's rewrite is off the clock. Other clients may
  clock differently; the sentence still holds for them.
- **The relay notes into a file, and the harness voids a run on a note.**
  fastchess discards an engine's stderr (measured: a probe engine's stderr line
  reached neither its console nor its `-log` file), so a `position` line the
  relay could not read and passed unchanged would have been invisible.
  `BARE_FEN_RELAY_NOTES` names a file the relay appends each note to;
  `fastchess.sh` sets it to `<out>/relay_notes.txt`, its census prints the
  count, and any note voids the run with `SPRT-RUN-INVALID`, as a crash does.
  The brief did not ask for this; without it the regime the banner claims
  could not be checked.
- **One pre-existing test-side defect fixed in passing**, in a file this step
  edits: `tests/test_fastchess_script.sh`'s cleanup line read
  `rm -rf "$reached_dir""$default_dir" ...`, one path made of two, so neither
  sandbox was removed and every run of the test left two directories in
  `/tmp`. 36 were there when found; one space fixes it. The 16 this session
  made were removed; the 20 older ones were left.
- **The scripts anchor to their own path** rather than `cd
  /home/max/ws/chesso` as `S238_sprt.sh` does, so the smoke ran from this
  worktree and the real runs go from the main checkout. **They also unset the
  regime variables a caller may have exported** (`ROUNDS`, `TC`, `HASH`,
  `CONCURRENCY`, `SRAND`, `AA`, `RELAY_PYTHON`; the A/A sets its own): a
  leaked `ROUNDS` would have removed the SPRT. A dry run with `ROUNDS=7
  HASH=64 CONCURRENCY=3` exported and a stub `fastchess` on `PATH` handed
  fastchess `-rounds 20000`, `option.Hash=16`, `-concurrency 12` and the
  `{0, 5}` bounds (SPRT script), and `-rounds 500`, Hash 16, concurrency 12 and
  no bounds (A/A script).

### The relay

`tools/bare_fen_relay.py`, run as `<python with python-chess>
tools/bare_fen_relay.py ENGINE [ARGS...]`.

- Each `position startpos ...` or `position fen <fen> ...` line, with or
  without `moves`, goes to the engine as `position fen <Board.fen()>`. The
  board is `chess.Board(<fen>)` or the start position, each move goes through
  `push_uci`, which refuses an illegal one. So castling rights, the en-passant
  square and both counters are python-chess's. `Board.fen()`'s default writes
  the en-passant square only when a legal capture exists, which is the form
  `S271_replay.py`'s `bare` mode sent. Chesso's loader keeps one only when a
  pawn can take it (S042); the test found no position in the session where the
  two disagree.
- Every other line goes to the engine byte for byte. The engine's output is
  copied back as raw chunks as it arrives, with no line buffering, since none
  of it is rewritten.
- A line it cannot read passes unchanged with a note: no `startpos` or `fen`
  (chesso's `position kiwipete`), an invalid FEN, an illegal move. The engine's
  own refusal rules then apply.
- The board is kept between lines. A line with the same base whose moves
  extend the last line's pushes only the new moves, two per line in a game.
  Any other line builds from scratch, and a failure part way through the moves
  drops the kept board.
- It exits with the engine's status when the engine exits, whether or not its
  own input is still open. Closing its input closes the engine's. SIGTERM,
  SIGINT and SIGHUP kill the engine first.

**Proof under real fastchess.** Two games at 1+0.01, both sides
`cmd=<python> args="<relay> <tee wrapper>"`: of 170 `position` lines the
engines received, 170 were `position fen` with six fields and none carried
`moves` (`.tuning/coord/S272/rec/relayed_received.txt`).

### Its latency

`adocs/data/S272_relay_latency.py`, reading `adocs/data/S272_relay_latency.txt`.
Two copies of `build/src/chesso`, one direct and one through the relay (under
`-I`, as `fastchess.sh` launches it), take the same fastchess-shaped turns,
alternating which goes first. A turn is `position fen <book> moves ...` and
`isready`, wait for `readyok`, then `go nodes 1`, wait for `bestmove`. The
3633 turns are every ply of the first 40 games of `S198_calibration.pgn`.
Taken last on 2026-10-11 after the fast check, the first reading started at a
one-minute load average under 0.3, waited for. Microseconds:

| span | beside it | load avg start, end | direct median / p99 | relay median / p99 | excess median / p99 |
|---|---|---|---|---|---|
| handshake (`position` to `readyok`, off the clock) | nothing | 0.28, 0.33 | 92 / 540 | 193 / 951 | 128 / 634 |
| clocked (`go` to `bestmove`) | nothing | 0.28, 0.33 | 107 / 560 | 140 / 764 | **48 / 276** |
| handshake | 12 busy engines | 0.33, 4.72 | 997 / 1998 | 1202 / 3066 | 390 / 2417 |
| clocked | 12 busy engines | 0.33, 4.72 | 2060 / 3873 | 2196 / 3973 | **17 / 2358** |

The excess is per turn, relay minus direct. Earlier readings, before the
fast check, put the clocked excess at 109 / 269 (load average 0.08 at the
end) and 38 / 255 (3.39 at the end, still falling from the loaded reading
before it -- labelled "idle" then, which it was not; the fast check's item 7).
The figures move by tens of µs from run to run. **What a move's clock pays is
a median under 0.11 ms and a p99 under 2.7 ms, on both sides alike, against
the 80 ms increment.** Twelve busy engines is a match's own load here: one
search per game at concurrency 12.

### Driving it: `BARE_FEN=1 ./fastchess.sh`

An opt-in variable in `fastchess.sh`, unset by default, and its test extended.
The run scripts call `fastchess.sh` as every SPRT here does. The alternative was
a run script calling fastchess directly with both commits built the way
`fastchess.sh` builds them, and that would have re-implemented everything DEC-220
and DEC-020 rest on. Going through `fastchess.sh` keeps:

- the `cand-<sha>` and `ref-<sha>` names, so the log's `Results of
  cand-a7bb10e vs ref-16c8883` line is the one `tools/gate.sh` checks the
  block against;
- the banner with both shas and dates, the identity probe, the candidate
  snapshot, the cached-reference checks;
- the crash census and the markers.

None of that is duplicated, and nothing in `tools/gate.sh` or
`tools/ledger.py` changes. Under `BARE_FEN=1`:

- each side is `cmd=$RELAY_PYTHON args="-I <relay snapshot> <binary>"`;
- `RELAY_PYTHON` defaults to `~/.venv/chess/bin/python`, and a python that
  cannot `import chess` under `-I` stops the run before anything is built;
- the relay is snapshotted like the candidate, into a `mktemp -d` directory
  of its own, and the banner prints its blob, whether it is HEAD's, and the
  notes file;
- the `id name` probe goes through the relay;
- the PGN's `Event` ends `bare-fen`;
- the notes file and void are as above;
- a path with a space is refused, because fastchess splits `args=` on spaces.

Unset, fastchess's argument vector, the banner and the event are the same as
before (case 30's second half).

### The test

`tests/test_bare_fen_relay.py`, `test_bare_fen_relay` in the fast suite,
about 1 s, started by the system `python3` and re-run under
`CHESSO_CHESS_PYTHON`. Its session,
`tests/assets/bare_fen_relay_session.txt` (1119 lines, 289 `position` lines,
113 KB), is written by `adocs/data/S272_relay_session.py`. It has two parts:

- what fastchess sent one engine over two games at 1+0.01, recorded through a
  `tee` wrapper and kept whole up to its final `stop`: `uci`, the two
  `setoption` lines, `ucinewgame`, the `isready` pings, `position fen <book>
  moves ...`;
- then two games from tracked PGNs with a `position` line at every ply, found
  by python-chess and not by reading them. `S018_match.pgn` game 155 is from
  the start position, castles kingside on both sides, captures en passant and
  promotes. `S198_calibration.pgn` game 593 castles queenside and
  under-promotes.

It ends with `go infinite`, `stop` and `quit`. The eight properties are in the
file's docstring:

1. coverage, asserted first;
2. every line reaches a stub engine in order, each `position` as six FEN
   fields with no `moves`, the rest byte for byte;
3. the stub's replies come back byte for byte;
4. every FEN sent equals chesso's own `fen` after the original line, and
   chesso loads it back to the same string. That is two implementations
   agreeing, so no expected FEN or golden is written down;
5. the kept board equals a fresh board per line;
6. unreadable lines pass unchanged with three notes on stderr and in the
   notes file, a failed line does not leave the kept board ahead, and a new
   base after a line with no moves is rebuilt;
7. the relay exits with the engine's status, 0 and 3, while its input is
   still open;
8. a reply comes back while the relay's input is still open, each awaited
   before the next line is written, as fastchess does: `uci` -> `uciok`,
   `position` + `isready` -> `readyok`, `go` -> `bestmove` (the fast check's
   item 5).

**Mutants**, each applied to a copy and run against its test
(`.tuning/coord/S272/mutants.sh`, log `mutants.log`); every one is red:

| id | mutation | red at |
|---|---|---|
| M1 | the kept board not dropped before pushing | 6 |
| M2 | `fen(en_passant='fen')` | 4 |
| M3 | the moves-prefix check removed | 2 |
| M4 | no rewrite | 2 |
| M5 | counters sent as `0 1` | 4 |
| M6 | the relay outlives its engine | 2 (a 20 s timeout) |
| M7 | the engine's status dropped | 7 |
| M8 | the base check removed | 6 (survived property 5 until 6 gained the base change after a no-moves line) |
| M9 | one new move skipped | 2 |
| M10 | no notes file | 6 |
| B1 | the engine's output held until it exits (`engine.stdout.read()`) | 8 only: passes 1 to 7 |
| B2 | the engine's output held in 64 KB blocks (a `BufferedReader`) | 8 only: passes 1 to 7 |

`tests/test_fastchess_script.sh` has 31 properties, up from 29:

- **30**: `BARE_FEN=1` hands both sides `cmd=<RELAY_PYTHON>` and
  `args=-I <snapshot> <own binary>`, and each side runs its own binary
  through it. One snapshot of the tree's relay, `bare_fen_relay.py` in a
  `chesso-relay.*` directory of its own, is launched four times under `-I`:
  two probes and two sides. The import probe runs once, under `-I`. The
  directory is gone after the run. The banner prints the blob, `NOT HEAD's`,
  the python-chess version and the notes file, and the event ends
  `bare-fen`. All four launches are told the run's notes file, a stale note
  planted there before the run is emptied, and the census line reads 0.
  Unset, there is no `args=`, no `bare-fen` and no relay line.
- **31**: a python that cannot import python-chess, and `BARE_FEN=yes`, stop
  the run before fastchess with a marker naming why, and a relay that writes a
  note voids the run (`SPRT-RUN-INVALID: 4 ...`, one note per stub launch,
  last line `SPRT-RUN-FAILED`).

**Red first**: against `git show HEAD:fastchess.sh` the new cases fail four
assertions (`.tuning/coord/S272/fastchess_test_red.log`). Harness mutants F1
to F12 are each red: the relay run from the tree, the probe not through the
relay, no event label, the snapshot's directory left behind, the reference
not through the relay, no void on a note, no notes variable, any `BARE_FEN`
value accepted, a side launched without `-I`, the notes file not emptied, the
import probe without `-I`, and the snapshot written straight into `/tmp`.
The driver re-ran all 24 mutants, relay and harness, on the final text:
every one red, no anchor missing.

### The two scripts

**`adocs/data/S272_aa.sh`, the A/A (accepts 2).** `CAND=REF=a7bb10e AA=1
ROUNDS=500 BARE_FEN=1`, 1000 games, 8+0.08, Hash 16, `noob_3moves.epd`,
concurrency 12. Its header carries the **reading rule**. The SPRT starts only
if:

1. `SPRT-RUN-DONE` with 1000 games and no `SPRT-RUN-INVALID`;
2. the relay line reads `(HEAD's)`;
3. the census reads 0 positions passed unrewritten;
4. `tools/forfeit_report.py` passes at 1.0 % per side;
5. the Elo interval contains 0, with one re-run on a new seed if it does not
   and the owner stopping S272 on a second exclusion.

The pair variance, read by `S105_pairs.py` and against S219's band by
`S198_pairs.py`, is recorded but does not stop it: a bare FEN takes the game's
history from both sides, so the self-play distribution may itself move, and
nElo bounds normalise by the run's own variance. The script runs `fastchess.sh`,
then the three readers into `<out>/aa_reading.txt`, and ends `S272-AA-DONE
<out>` -- fastchess.sh finished and all three readers exited 0 -- or
`S272-AA-FAILED: <why>`, on every path. Its watcher waits for those, not for
`SPRT-RUN-*`; its notes file is `<out>/relay_notes.txt`, `<out>` being
`.tuning/s272_aa_<stamp>`. The log committed is the console through
`tools/trim_console.py --summary`, as `adocs/data/S272_aa.log`.

```
nohup adocs/data/S272_aa.sh > .tuning/s272_aa.log 2>&1 &
```

Expected 28.4 min at 2110 games an hour plus the relay's own cost, which the
run measures; ceiling 3600 s.

**`adocs/data/S272_sprt.sh`, the pre-registration (accepts 3, 4).** `CAND=a7bb10e
REF=16c8883 BARE_FEN=1`, `{0, 5}` nElo, 8+0.08, Hash 16, `noob_3moves.epd`. It
execs `fastchess.sh`, so the markers are `SPRT-RUN-DONE` and
`SPRT-RUN-FAILED`.

- **Gate**: it starts only after the A/A's reading passes.
- **Worst case (DEC-143)**: 41861 games at the midpoint and 25591 on a bound,
  19.8 h and 12.1 h at 2110 an hour. The cap is 40000 games, 19.0 h.
- **The prediction**, stated as one and never as the result: the replay's
  x1.218 nodes at depth 12 and Hash 16, about +50 Elo, about +61 nElo at S212's
  33.9 % draws. If it holds, about 1200 games, 35 min (2600 games, 1 h 14 m at
  +30 nElo), from the same Brownian approximation that gives DEC-143's two
  figures.
- **Abort**: forfeits over 1.0 % on a side; a crash, disconnect or relay note
  voids; mains or a second load. Not the length.
- **Open DEC-171 findings**: S259.
- **The three outcomes.** S271 is never reverted: it is the owner's
  UCI-correctness ruling, and harness play is node-identical.
  - H1 is the replay's direction measured in games. Its Elo is upward-biased,
    stated beside the prediction.
  - H0 means the conversion does not carry to games at this control: the
    searches are shallower than the replay's depth 12, and a cold table may
    cost fewer moves than nodes. It is recorded as a zero for that claim, and
    a longer control or larger hash is a new question for the owner.
  - No verdict is a zero, read as an H0.
- **The log the block names** is the console through
  `tools/trim_console.py --summary`, as `adocs/data/S272_sprt.log`, not a
  copy (fast check item 1).
- **The block**, proposed and the coordinator's call: DEC-220's block with the
  tc field written `8+0.08 bare-FEN`, which `tools/gate.sh` and
  `tools/ledger.py` both read as free text.
- **Notes file** for a watcher: `<out>/relay_notes.txt`, `<out>` being
  `.tuning/sprt_s272_<stamp>`.

```
nohup adocs/data/S272_sprt.sh > .tuning/s272_sprt.log 2>&1 &
```

Expected about 35 min if the prediction holds, 12.1 to 19.8 h if the truth
sits on or between the bounds, 19.0 h at the cap. A watcher ceiling of 2x the
cap (38 h) covers every case.

### Smoke runs

Their outputs are discarded. Taken again after the fast check, on the final
harness.

- `SMOKE=1 adocs/data/S272_aa.sh`: 4 games at 1+0.01, concurrency 2, through
  the relay under `-I` (the banner's `on ... -I` line, and `notes to
  <out>/relay_notes.txt`). Both identity probes went through it (`Chesso
  a7bb10e native` twice), relay notes 0, `SPRT-RUN-DONE`, then the three
  readers, each `exit 0`, ending with `S272-AA-DONE`. Again with a `python3`
  on `PATH` that fails `forfeit_report.py` with exit 2: `S272-AA-FAILED: a
  reader did not read clean (forfeit_report.py exited 2)` as the last line,
  exit 1 (`.tuning/coord/S272/smoke_aa_readerfail.log`).
- `SMOKE=1 adocs/data/S272_sprt.sh`: 4 games, `cand-a7bb10e` against
  `ref-16c8883`. It built both into `.ref-builds/`, and the log carried
  `Results of cand-a7bb10e vs ref-16c8883 (1+0.01, 1t, 16MB,
  noob_3moves.epd)`, the line `tools/gate.sh` reads. Relay notes 0,
  `SPRT-RUN-DONE`.
- The relay's blob in both banners read `8e9aeeb69b59 (NOT HEAD's ...)`, as
  it must before the relay is committed. In the real runs it must read
  `HEAD's`.

### Docs

- `DEV_MANUAL.md` "Play games": the `BARE_FEN=1` line, and a paragraph on what
  it does and costs. The snapshot sentence now names the relay too.
- `DEV_MANUAL.md` "Test": the one fast test that needs python-chess, and
  `-DCHESSO_CHESS_PYTHON`.
- `TOOLCHAIN.md`'s python row: what now runs it.
- `MANUAL.md`: checked, no change. The relay is a harness tool and no UCI
  surface moves.
- `adocs/specs.md`: no wording proposed, since no engine behaviour moves.
- `README.md`: not touched.

### The fast check, round 1 (2026-10-11), each fixed in this tree

1. **The log would pass the gate's 20 MB.** A bare FEN hides the game's
   history, so fastchess prints a four-line "PV continues after threefold
   repetition" block whenever a PV runs past a repetition: 96.6 % of the SPRT
   smoke console's bytes (16.9 KB a game over 4 games), 78.9 % of the A/A
   smoke's (4.5 KB). Both scripts now pre-register the committed log as the
   console through `tools/trim_console.py --summary`. Proved on the SPRT
   smoke's console: 69871 bytes -> 2642, the `Results of cand-a7bb10e vs
   ref-16c8883 (1+0.01, 1t, 16MB, noob_3moves.epd):` line kept and the
   blocks counted at the end (34, 19 and 15 per side). `tools/gate.sh
   --message` over the staged tree with that file as
   `adocs/data/S272_sprt_dryrun.log` and a dry six-line block (tc field
   `8+0.08 bare-FEN`) passed, `SPRT block checked: cand a7bb10e vs ref
   16c8883 against adocs/data/S272_sprt_dryrun.log`, `GATE-DONE 4081329`, the
   whole suite inside it green. The same block naming `ref 1680439` was
   refused with the trimmed log's own `Results of` line quoted. Nothing
   committed. The index was restored byte for byte from a copy, and the dry
   file removed (`.tuning/coord/S272/gate_dry*.log`).
2. **Module shadowing.** Python puts a script's directory, then
   `PYTHONPATH`, ahead of python-chess, and the snapshot was written straight
   into `/tmp`. Shown with the real interpreter
   (`.tuning/coord/S272/shadowing.log`): a `chess.py` beside the relay, and
   one on `PYTHONPATH`, were each imported instead of python-chess without
   `-I`, and neither was with it. `fastchess.sh` now runs the import probe,
   the identity probes and both sides under `-I`, from a `mktemp -d`
   directory holding the snapshot alone, removed by the trap. Case 30 checks
   both. Red first: case 30 against the unfixed script failed on `args=`
   lacking `-I` (`fastchess_test_red2b.log`).
3. **A 3.12-only f-string**, a backslash inside a replacement field in
   property 3's message in `tests/test_bare_fen_relay.py` (the count of
   `go` lines), hoisted into variables. A
   tokenizer scan of all four new Python files for a backslash or a reused
   outer quote inside an f-string expression now finds none
   (`fstring_compat.py`, scratch). No older python is installed here to run
   them under.
4. **A missing python-chess skips, visibly.** The test is now started by the
   system `python3`. It re-runs itself under `CHESSO_CHESS_PYTHON -I` when it
   cannot import python-chess itself, and exits 77 with `SKIPPED -- [<python>]
   cannot import python-chess ...` when neither can.
   `tests/CMakeLists.txt` sets `SKIP_RETURN_CODE 77`. Red first, with the
   build reconfigured: an absent interpreter read `***Not Run` and a python
   without python-chess `***Failed`, both counted failed
   (`skip_red.log`); after, both read `***Skipped`, and the real venv
   `Passed` (`skip_green.log`). The cache variable is back on the venv.
   `DEV_MANUAL.md` says a skip is something to read.
5. **Property 8**, a reply while the input is open. Two buffering mutants,
   B1 and B2 above, pass properties 1 to 7 and fail 8 alone, `b'uci\n' was
   not answered b'uciok\n' within 5 s`.
6. **Late or soft signals.**
   - `fastchess.sh` empties the notes file when it sets it, before the
     probes, and prints it on the banner (`notes to <path>`). Case 30 plants
     a stale note first; against the unfixed script that run was voided
     (`fastchess_test_red2.log`).
   - `S272_aa.sh` keeps each reader's status and ends `S272-AA-FAILED: a
     reader did not read clean (<name> exited <n>)` if any is non-zero, so
     DONE means all three read clean.
   - Notes paths, for a watcher:
     `/home/max/ws/chesso/.tuning/s272_aa_<stamp>/relay_notes.txt` and
     `/home/max/ws/chesso/.tuning/sprt_s272_<stamp>/relay_notes.txt`, the
     stamp being the launch's `YYYYmmdd_HHMMSS`, printed whole on each
     banner's `notes to` line.
7. **The mislabelled reading.** The latency script prints the one-minute load
   average at both ends. Both readings were taken again under `-I`, the first
   after waiting for the average to fall under 0.3, and labelled by what was
   running beside them (table above).

### Suites

Both fast suites are 44/44, up from 43 with `test_bare_fen_relay`, on the
final tree after the fast check. `./clang-format.sh --check` is clean under
`CLANG_FORMAT_MAJOR=22`. `tools/plan_prose_check.py`, one mode per call:
`--citations` 0 flagged over 38 files, `--touches` 0 flagged over 38,
`--params` clean. One interim red, fixed: this file's first account of item 3
cited the test by line, which `--citations` (and so
`test_plan_citation_freshness`) refuses (DEC-135); it now names the message. No second tier:
nothing in `src/` moves.
