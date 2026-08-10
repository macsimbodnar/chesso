# Developer manual

Layout, build, test, and the exact commands. `README.md` is the owner's
hand-written introduction and is not this file; `MANUAL.md` is for someone
running the engine rather than changing it. Tool setup and the traps in each
tool are in `TOOLCHAIN.md`. The rules that govern when a number is believable
are in `CLAUDE.md`.

## Layout

| path | what |
|---|---|
| `src/` | the engine. `bitboard.cpp` is move generation, make/unmake and SEE; `search.cpp` is the search; `evaluation.cpp` and `eval_tables.hpp` are the evaluation; `uci.hpp` and `chesso.cpp` are the protocol |
| `tests/` | doctest suites plus `bench_movegen`, the perft and generator benchmark |
| `tools/` | measurement and analysis, not shipped with the engine |
| `adocs/` | the workflow state: specs, plan, steps, decisions, testing ledger |
| `adocs/data/` | raw output of runs a decision rests on, kept because regenerating it costs hours of reference search. `adocs/data/README.md` says what each file is |
| `books/` | opening books for match play |
| `.ref-builds/` | git worktrees created by `fastchess.sh`, gitignored |

Three build directories, all with `ccache` wired in: `build` (Release, the one
that gets measured), `build-debug` (asserts on), `build-prof` (RelWithDebInfo).

## Build

```bash
cmake --build build -j8
```

Configure a directory that does not exist yet, with ccache:

```bash
cmake -S . -B build       -DCMAKE_BUILD_TYPE=Release       -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug         -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build-prof  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

Do not point CMake at Homebrew clang. Different compiler, different codegen, and
every recorded benchmark becomes incomparable.

## Test

```bash
ctest --test-dir build -L fast    # correctness, must stay green, about 11 s
ctest --test-dir build -L slow    # deep perft, minutes
```

The step-completion gate in `.moltke.json` is:

```bash
cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && ./clang-format.sh --check
```

That gate is necessary and not sufficient. Deep perft, the debug-build
assertions and any SPRT are named in each step's `accepts:` field and run by
hand. DEC-025.

**Run test binaries from `tests/`** — they load assets by relative path:

```bash
cd tests && ../build-debug/tests/test_movegen
```

The debug build asserts `squares[]` against the bitboards and the evaluation
accumulators against a full recomputation, on every make and unmake. Any change
to `make_move`, `unmake_move` or the generator must be run through it. That is
INV-2 and INV-4.

`test_uci_surface` is the golden surface guard. It reads the command set out of
`uci_command_names()` and the option lines out of the `uci` reply, then holds
both against a golden list and against `MANUAL.md`. **A failure there is not a
broken test.** It means the protocol surface moved: update `MANUAL.md` and the
golden lists in `tests/test_uci_surface.cpp` in the same commit as the change.
It reads `MANUAL.md` through the `CHESSO_SOURCE_DIR` compile definition, so it
is the one test binary that does not need to run from `tests/`. The `go` and
`position` argument lists in it are maintained by hand and a newly added
argument will not fail it — DEC-028 says why.

## Format

```bash
./clang-format.sh           # format in place
./clang-format.sh --check   # dry run, non-zero if anything is unformatted
```

Pins clang-format major version 22 and refuses to run on anything else,
deliberately: clang-format changes its output between major versions and an
unpinned formatter rewrites files nobody touched. Override with
`CLANG_FORMAT_MAJOR=15 ./clang-format.sh --check`.

## Measure

Node counts first, timings second. `bench_movegen` verifies perft counts before
printing anything and reports its own resolution — the disagreement between the
two halves of the run. Anything smaller than that resolution has not been shown
to exist.

```bash
./build/tests/bench_movegen        # perft and generator throughput
./build/tests/bench_movegen -r 40  # long enough that a profiler window cannot leave the phase
```

The search is not perft. perft measures generate, make and unmake; a search also
evaluates, orders moves and probes the transposition table, so a change can move
one number and not the other:

```bash
tools/search_bench.py ./build/src/chesso 9
```

Depth 9 is about ten seconds per binary, over three positions: midgame,
kiwipete, tactical. **The node count is printed next to the time on purpose.** A
change meant to be a pure speed-up must leave it identical; if the node count
moved, the search changed behaviour and the times are not comparable. That
comparison is how INV-6 is discharged for a behaviour-neutral change, and it is
what caught two bugs that timings did not.

A/B timing with statistics, always interleaved so machine drift cancels:

```bash
hyperfine --warmup 1 --runs 10 './bench_before -r 1' './bench_after -r 1'
```

Check the machine is idle first: `ps aux | sort -rnk3 | head`.

## Play games

```bash
./fastchess.sh                  # full SPRT, elo0=0 elo1=5, runs for hours
./fastchess.sh --fast           # looser bounds, few hundred games
REF=HEAD~1 ./fastchess.sh       # pick what to measure against
```

The reference is built from a git ref into a worktree under `.ref-builds/`, so
a result is always attributable to a commit range, and the candidate binary is
snapshotted before the first game. Both exist because an SPRT once reported
+301 Elo and meant nothing — DEC-020.

Games are played at a time control, so a change that makes the engine faster
shows up and a change that only reorders equal work does not. Bounds must match
the expected effect: `elo0=0 elo1=10` cannot resolve a small change, and one run
random-walked for 340 games before being stopped.

## Analyse a game

Never by reading it. See `CLAUDE.md` and DEC-023.

```bash
# SAN moves, one per line or whitespace separated, no move numbers
build/tools/pgn_to_positions < moves.txt > positions.tsv
tools/analyse_game.py positions.tsv --engine ~/.local/bin/stockfish --depth 18
```

Prints every move that cost more than the threshold, with what was played, what
Stockfish wanted, and the evaluation either side of it. `pgn_to_positions` uses
the engine's own `algebraic_to_move`, so there is no `python-chess` dependency.
It also emits the engine's own `game_phase()` as a fifth column, which
`error_profile.py` reads and this script ignores.

That one game took about 45 seconds at depth 18. **Do not budget from it.**
Fixed depth has no bounded cost: over 12 positions sampled from real games,
depth 18 ran a median of 0.71 s and a maximum of 925.90 s. Use node limits for
anything bigger than a single game — DEC-030.

## Profile the engine's own errors over many games

```bash
tools/error_profile.py match.pgn --player achesso \
    --engine ~/.local/bin/stockfish --nodes 1000000 --workers 4 \
    --raw-out raw.tsv --resume
```

Centipawns lost bucketed by game phase, by error size, and the two crossed, plus
evaluation bias — the engine's own score against the reference's, which comes
free because fastchess writes the engine's eval into each move comment. Book
moves are excluded. Scores are clamped to +/-1000 cp so one mate cannot outweigh
a game of real errors.

Node limits, not depth, and the reason is DEC-030. Measured over 20 sampled
positions: 500000 nodes is 0.83 s mean and reaches median depth 18, 1000000 is
1.63 s and median depth 21, 3000000 is 4.84 s and median depth 24.

Records are flushed per game, so an interrupted run keeps what it had and
`--resume` continues from there. `--from-raw raw.tsv` re-buckets an existing run
with no engine at all, which is how a finished run is asked a different question
without repeating it.

**Matches played for profiling are adjudicated differently from matches played
for strength.** The `fastchess.sh` settings stop games as soon as the result is
known, which produced zero endgame moves out of 115 and would have made the run
blind to the phase it was for. Use loose settings instead, and do not compare
the two kinds of match to each other — DEC-031:

```bash
-draw movenumber=80 movecount=10 score=5 -resign movecount=8 score=900 -maxmoves 200
```

## Ask whether an error was the search or the evaluation

```bash
tools/depth_vs_eval.py adocs/data/S018_raw.tsv \
    --engine build/src/chesso --reference ~/.local/bin/stockfish \
    --out probe.tsv
```

Samples the expensive moves out of a raw profile, re-asks chesso at its in-game
node budget and at 16 times it, and re-costs both answers with the same
reference. A big drop means horizon errors and the payoff is in search; a small
one means the engine still likes the same move much deeper and the payoff is in
what it believes a position is worth.

The default sample of 160 positions at 4M against 64M nodes took 683 s on three
workers with the machine otherwise busy. `--min-cost` and `--decided` select
which errors are asked about; the defaults take errors of 100 cp or more in
positions inside +/-300, because centipawns given away in an already-decided
position do not decide games. DEC-033 is the run this produced and
`adocs/data/DEC033_depth_vs_eval.tsv` is its output.

## Profile

```bash
cmake --build build-prof -j8
dsymutil build-prof/tests/bench_movegen        # macOS keeps debug info in the .o files
samply record --save-only --unstable-presymbolicate \
  -r 2000 -o /tmp/prof.json.gz -- ./build-prof/tests/bench_movegen
tools/samply_report.py /tmp/prof.json.gz
```

Two silent failure modes: no `dsymutil` run, or no `--unstable-presymbolicate`,
and every frame prints as a bare hex address. Use a large enough `-r` that the
sampling window cannot leave the phase being measured — a profile taken with
`-r 12` once reported `generate_moves` at 41 % when the true figure was 20 %,
and it reversed the conclusion of the work that followed.

## Reference engines in `~/.local/bin`

- **A fixed chesso build from `main`**, mailbox move generation. Two uses: an
  independent implementation to cross-check perft against, and a fixed rung to
  measure absolute progress. Chained SPRTs give relative gains that do not
  necessarily add up; a never-changing opponent catches that drift.
- **Stockfish.** Never an SPRT opponent — far too strong, every game is a loss
  and there is no signal. Use it for `go perft N` as an oracle, analysing
  chesso's losses, labelling positions for tuning, and as a calibration opponent
  limited by `go nodes N`. Prefer fixed nodes over `UCI_LimitStrength`, which
  injects random blunders and is high variance.
- **`sgambetto`**, an engine written by a friend, unrelated to this codebase.
  Close enough in strength to give fought games: 39.3 % over 210 games at
  10+0.2 under the loose adjudication of DEC-031, 57.5 % over 20 under the SPRT
  settings. It is the opponent for the S018 error profile (DEC-029), chosen over
  the mailbox build, which loses 6-0 and therefore profiles an already-won
  position rather than a game.

The mailbox build and `sgambetto` are both fixed binaries that this repository
does not build. Record which one produced a number, because they are not
interchangeable.
