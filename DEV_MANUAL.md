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
hand. DEC-010.

**Run test binaries from `tests/`** — they load assets by relative path:

```bash
cd tests && ../build-debug/tests/test_movegen
```

The debug build asserts `squares[]` against the bitboards and the evaluation
accumulators against a full recomputation, on every make and unmake. Any change
to `make_move`, `unmake_move` or the generator must be run through it. That is
INV-2 and INV-4.

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
+301 Elo and meant nothing — DEC-005.

Games are played at a time control, so a change that makes the engine faster
shows up and a change that only reorders equal work does not. Bounds must match
the expected effect: `elo0=0 elo1=10` cannot resolve a small change, and one run
random-walked for 340 games before being stopped.

## Analyse a game

Never by reading it. See `CLAUDE.md` and DEC-008.

```bash
# SAN moves, one per line or whitespace separated, no move numbers
build/tools/pgn_to_positions < moves.txt > positions.tsv
tools/analyse_game.py positions.tsv --engine ~/.local/bin/stockfish --depth 18
```

Prints every move that cost more than the threshold, with what was played, what
Stockfish wanted, and the evaluation either side of it. About 45 seconds for a
158-ply game at depth 18. `pgn_to_positions` uses the engine's own
`algebraic_to_move`, so there is no `python-chess` dependency.

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
