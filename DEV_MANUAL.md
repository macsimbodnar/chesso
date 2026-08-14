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
| `tests/` | doctest suites plus `bench_movegen` and `bench_eval`, the generator and evaluation benchmarks |
| `tools/` | measurement and analysis, not shipped with the engine |
| `adocs/` | the workflow state: specs, plan, steps, decisions, testing ledger |
| `adocs/data/` | raw output of runs a decision rests on, kept because regenerating it costs hours of reference search. `adocs/data/README.md` says what each file is |
| `books/` | opening books for match play |
| `.ref-builds/` | git worktrees created by `fastchess.sh`, gitignored |
| tool config | tracked when it describes the project, ignored when it describes a machine. `CLAUDE.md` and `.cursor/rules/moltke.mdc` are the two agent pointers at `AGENTS.md`, `.vscode/` is the editor setup; `.claude/settings.local.json` is the one exception and is gitignored. DEC-051 |

Three build directories, all with `ccache` wired in: `build` (Release, the one
that gets measured), `build-debug` (asserts on), `build-prof` (RelWithDebInfo).

Nothing may reconfigure `build` behind your back. VS Code's CMake Tools used to:
it defaults to `${workspaceFolder}/build`, configures on open, and wrote Debug
and `CMAKE_C_COMPILER=gcc-14` into it against the g++ 13.3 DEC-049 pins.
`.vscode/settings.json` now points it at `build-debug`. DEC-052.

## Build

```bash
cmake --build build -j12
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
ctest --test-dir build -L fast    # correctness, must stay green, about 18 s
ctest --test-dir build -L slow    # deep perft, minutes
```

Those 18 s assume `build/` was configured `Release`. Configured `Debug`, or with
an empty `CMAKE_BUILD_TYPE`, the same suite takes 126 s and `test_movegen` and
`test_search` blow the 60 s timeout every `fast` target carries — which reads as
two test failures rather than as a wrongly configured build directory. Both
happened on 2026-08-13, the second time because an editor rewrote the directory
mid-session (DEC-052). Check it before believing either result:

```bash
grep -E 'CMAKE_BUILD_TYPE|CMAKE_C_COMPILER:' build/CMakeCache.txt
```

Piping `ctest` into `tail` or `head` hides its exit status behind the pipe, so a
gate written that way reports success on a failed suite. Redirect to a file and
read `$?`, or let `ctest` print in full.

The step-completion gate in `.moltke.json` is:

```bash
cmake --build build -j12 && ctest --test-dir build -L fast --output-on-failure && ./clang-format.sh --check
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

`test_fastchess_script` is the smoke run over `fastchess.sh`, the only shell
script any test touches. It plays no games: `fastchess` is a stub on `PATH`, the
candidate and reference are one-line shell scripts, and the whole run happens
inside a throwaway git repository, so `.ref-builds/` and `build/` are never
read. It asserts two things — the script reaches the `fastchess` invocation, and
a script that aborts before that point exits non-zero. It exists because
`44877c4` left a renamed variable behind and the harness stopped running for a
commit without anything noticing (S035, `2026-08-13_adversarial-F01`).

`test_clang_format_script` is the same shape over `clang-format.sh`, and it
exists because that script is the third command in the gate above. It asserts
five things in a throwaway git repository: a clean tree passes, a misformatted
tracked file is caught, a misformatted **untracked** file is caught, a
misformatted file under a gitignored path is skipped, and a file the index still
names but the worktree no longer has is not opened. The first two are
preconditions — without them a script that always fails, or a fixture that is
not actually misformatted, would satisfy the rest by accident. See the `Format`
section below for what the selection is (S054).

## Format

```bash
./clang-format.sh           # format in place
./clang-format.sh --check   # dry run, non-zero if anything is unformatted
```

Pins clang-format major version 22 and refuses to run on anything else,
deliberately: clang-format changes its output between major versions and an
unpinned formatter rewrites files nobody touched. Override with
`CLANG_FORMAT_MAJOR=15 ./clang-format.sh --check`.

Covers every `.c .cc .cpp .h .hpp .hh` file git reports as tracked **or
untracked and not ignored** — `git ls-files --cached --others
--exclude-standard`. So a source file you have just written is checked before it
is staged, which is the whole of S054: until then the selection was
`git ls-files` with no flags, the index only, and a step that added a file got a
vacuous pass for exactly the files it added. `--exclude-standard` is what keeps
the generated sources under `build*/` and the checked-out engine copies under
`.ref-builds/` out, so those stay unformatted; a file the index still names but
the worktree no longer has is skipped rather than handed to clang-format, which
cannot open it. `test_clang_format_script` holds all of that.

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

Three positions: midgame, kiwipete, tactical. Depth 9 is 3136397 nodes and
about 0.44 s for all three on this machine — raise the depth when a difference
is small, and do not budget from the "about ten seconds per binary" this line
used to claim, which was measured on the pre-DEC-049 machine and on an engine
with less pruning in it.

**The node count is printed next to the time on purpose.** A change meant to be
a pure speed-up must leave it identical; if the node count moved, the search
changed behaviour and the times are not comparable. That comparison is how INV-6
is discharged for a behaviour-neutral change, and it is what caught two bugs
that timings did not.

The tool keeps the last `info` line, and since S037 that line's `nodes` is the
**whole search's** count, cumulative over every iteration, so the comparison
covers the whole tree. **Every node figure recorded from this tool before
S037 — 2026-08-13 — is a sum of last iterations only**, roughly half of the
search on these positions, and a figure from before that date is not comparable
with one taken after it. The counts themselves did not move: the same run reads
609848 / 2058510 / 468039 at depth 9 where it used to read 254082 / 1022573 /
168767.

Neither of those can price an evaluation term. A term that changes the score
changes the tree, so the two builds visit different nodes and a wall time at
fixed depth mixes the cost of the term with the shape of the search it caused.
`bench_eval` calls `evaluate()` and nothing else, over ten compiled-in positions
running from a full board down to bare kings:

```bash
./build/tests/bench_eval                  # ns per call, and the resolution of the run
./build/tests/bench_eval -r 9 -n 4000000  # longer sweeps when the difference is small
```

`-n` is repetitions of the position list per sweep and it matters: the same `-n`
that gives a 640 ms sweep with an expensive term gives a 6 ms sweep without one,
and the cheap build is then the one that cannot be trusted. Compare two builds
at the same `-n`, and raise it until the printed resolution is small against the
difference being claimed. The scores and a checksum are printed before any
timing, so a build that evaluates differently says so instead of quietly being
timed as though it were the same function.

Today's evaluation is **1.31 ns per call, 762 M calls per second**. Recomputed
mobility measured 15.93 ns, 12.2 times more, and cost 33 % of nodes per second
in a real search at depth 12 — worse than the 25 % S014 removed. DEC-036.

A/B timing with statistics, always interleaved so machine drift cancels:

```bash
hyperfine --warmup 1 --runs 10 './bench_before -r 2' './bench_after -r 2'
```

Check the machine is idle first: `ps aux | sort -rnk3 | head`.

## Size the lazy evaluation margin

```bash
build/tools/eval_spread --data .tuning/selfplay_v1.tsv            # 1.49 M positions, 2.4 s
build/tools/eval_spread --data .tuning/selfplay_v1.tsv --limit 200000
```

Reads `tools/datagen`'s `fen result score phase` and uses only the FEN. Prints
the distribution of the **unclamped** expensive-stage correction in absolute
centipawns — p50, p90, p95, p99, p99.9, max — for mobility, for king safety and
for the two combined, then how many positions exceed each of 150, 200, 250, 300
and 400, then the worst position per term.

Three terms and not one because a margin that binds on the sum and a margin that
binds on a single term are different problems. `--limit N` stops after N
positions; the corpus is 1.49 M lines and the first N are consecutive plies of
the same games, so a limited run is a smoke test and not a sample.

**The unclamped number is not observable any other way.**
`evaluate_expensive()` clamps to `LAZY_EVAL_MARGIN` before it returns, which is
what makes the shortcut sound by construction, and it also means every caller
outside `evaluation.cpp` sees the truncated figure. `evaluate_expensive_terms()`
in `src/evaluation.hpp` exists for this tool and rides the collecting
instantiation `king_safety_features()` already uses, so the instantiation the
search compiles keeps one caller — S027's 21 % regression came from giving a
hot-path function a second one.

Every run re-checks itself: clamping the pair it reports must reproduce
`evaluate() - evaluate_cheap()` on every position, and it says so on the last
line or exits non-zero.

The two distributions on record differ in weights as well as corpus, so quote
them apart. Mobility with the pre-fit weights over 149084 positions: p50 19,
p95 60, p99 81, max 143 — what `LAZY_EVAL_MARGIN` was set from. Mobility with
the fitted weights of DEC-040 over all 1490839 of `.tuning/selfplay_v1.tsv`:
p50 24, p95 81, p99 113, max 244, and 0.104 % already past 150 with king safety
still at zero weight. Put the pre-fit weights back and the tool returns the
first set to within 2 cp, which is what says the gap is the fit and not the
measurement.

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

**Concurrency is every core the machine reports**, whatever kind it is — 12 on
this machine, which is 6 physical cores with SMT (DEC-050); efficiency cores on
Apple silicon (DEC-048, superseding DEC-042). The script reads it from `nproc`,
so it needs no edit per machine. `CONCURRENCY=N` overrides it; do not lower it
to be polite, since nothing else should be running during a match, and a run
that does lower it records why. `CONCURRENCY=6` is the way to one game per
physical core here if a result has to be as clean as this machine can make it.

Everything else that parallelises follows the same policy: `-j12` for a build,
and `datagen` and `tuner` default to every hardware thread rather than to a
number written into the source.

**A verdict cost 3 to 4.5 hours at four cores** on the Apple machine. Measured
across S027: 2024 games in 4 h 30 m accepting H0, 1300 games in 2 h 53 m
accepting H1, and one run that exhausted 3000 games in about three hours without
reaching either bound. Six verdicts came to roughly twenty hours and 13462
games.

Those figures do not carry to this machine — different cores, different
compiler, DEC-049 — and neither does the four-core baseline they were taken
against. **The cost of a verdict here is not known yet**: check the
games-to-verdict figure of the first runs and record what happens rather than
assuming the throughput was free.

### Detach the run, and arm a watcher that outlives the turn

```bash
REF=<sha> nohup ./fastchess.sh --fast > .tuning/sprt_<what>.log 2>&1 &
```

Same for `tuner`, which takes tens of minutes. Never hold either open in the
foreground.

Then watch the log with something **session-scoped**. In Claude Code that is
`Monitor` with `persistent: true`; a backgrounded shell loop is scoped to the
turn that started it and is killed the moment the user types, without saying so.
The run itself survives — it is `nohup`'d — so the failure is silent and looks
exactly like a match that has not finished yet. It happened twice in one
session before the rule was written down. AGENTS.md carries it too.

Progress without disturbing the match:

```bash
grep -E "^Elo:|^LLR:" .tuning/sprt_<what>.log | tail -2
grep -c "^Finished game" .tuning/sprt_<what>.log
```

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

## Generate self-play data and fit the evaluation

S028. Two programs: `datagen` plays chesso against itself and writes labelled
positions, `tuner` fits the evaluation constants to those labels.

```bash
build/tools/datagen --out .tuning/selfplay_v1.tsv \
    --games 20000 --nodes 100000 --seed 20260810

build/tools/tuner --data .tuning/selfplay_v1.tsv \
    --out .tuning/tuned_tables.hpp
```

`datagen` writes `fen result score phase`, one line per position, where
`result` is the game's outcome from White's point of view. Openings are 8
uniform random plies, discarded if the search found a mate or scored the
position past `--opening-limit`; games are adjudicated once one side holds
`--resign-score` for `--resign-plies`.

**Four independent clauses decide whether a position is recorded**, and every
run prints what each one cost:

| clause | flag | default |
|---|---|---|
| the side to move is not in check | none, always applied | — |
| the search found no mate | none, always applied | — |
| `abs(score) < --quiet-limit` | `--quiet-limit N` | 1000 |
| the search's chosen move is neither a capture nor a promotion | `--allow-tactical N` | 0 |

`--allow-tactical` is a 0/1 switch and not a rate: **1** records a position
whose best move is a capture or a promotion, **0** excludes it, which is what
`selfplay_v1.tsv` was generated under. The clause was justified here by the
claim that `evaluate()` is only asked about positions quiescence has already
resolved. It is not — `src/search.cpp:125` calls `evaluate_lazy()` at the top
of every quiescence node, before `generate_captures` at `:152` runs, and
returns on that stand-pat score at `:136-137`. DEC-055 loosened it and left the
other three alone; the numbers that decided each are in
`adocs/plan_todo/S065_corpus_regen_loosened_filter.md`.

The closing line of every run reads, and each figure is the number of positions
that would have been recorded but for that one clause — a position failing two
clauses is in neither count:

```
filter: 27839 considered, 17593 recorded; skipped in-check 1698,
        tactical best move 4814, mate 0, score past 1000 1824
```

Both programs default to every hardware thread, 12 here — DEC-050. **Rate on
this machine**, 12 threads, 100000 nodes per move, `--allow-tactical 1`: 600
games in 143.93 s, 56304 positions — 4.169 games/s, 391.2 positions/s, 93.84
positions per game, 64.15 bytes per row. Under the old filter, 73.3 positions
per game. 100000 nodes reaches depth 4 to 6 in a middlegame. The Apple
three-thread figure it replaces was 200 games in 61 s and about 77 positions
per game, and DEC-049 says that one does not carry.

`tuner` cost on the same machine, measured on a 56304-row corpus at 12 threads:
0.29 s to load, 2.22 ms per epoch, 10.9 MB resident. All three scale with rows,
and the whole corpus is held in memory. On the real 11003693-row corpus, 12
threads, machine idle: **54 s to load and fit K, 0.35 s per epoch, 1.2 GB
resident**, so 3000 epochs cost 1090.87 s. The per-epoch figure is the
game-level split's; a fully shuffled row index costs 0.76 s instead, 2.14x, for
the identical fit.

`tuner` fits 827 numbers so that a sigmoid of the evaluation predicts the game
result — Texel tuning. `piece_value[0..4]`, `psqt_mg` and `psqt_eg` are 773 of
them; mobility adds 8 at S034, and king safety 18, passed pawns 12, pawn
structure 6, piece placement 8 and tempo 2 at S027. The count is
`eval_model::PARAM_COUNT` and the run prints it on its first line, with the
number the `--only` group left free beside it. It never calls the search:
`tools/eval_model.hpp` is `evaluate()` written as a linear function of its own
constants, which is what makes a full pass cheap, and `test_eval_model` is what
stops that model drifting from the engine. It reports held-out error every
`--report` epochs and keeps the best one, prints a warning if the fit never
beats the constants it started from, and writes a header to paste in.

**The validation split is cut between games, not between rows.** `--validation F`
holds out that fraction of the corpus and `--seed N` seeds the shuffle; what is
shuffled is whole games. Until S066 it was rows, and `datagen` writes a game's
~92 rows contiguously and gives every one of them the same label — the game's
result — so a row-level split put **119360 of 119999 games, 99.47 %**, on both
sides of the cut over `selfplay_v2.tsv`. A held-out row whose game is in the
training set measures memorisation.

The corpus carries no game id, so the boundary is reconstructed from the FEN. The
ply, `2 * (fullmove - 1) + (black to move)`, only ever advances inside a game
(`src/bitboard.cpp:883`), so a row whose ply does not advance on its predecessor
cannot belong to the same game. The predicate is one-sided and the direction is
what makes the result exact:

- it **never cuts a game in half**, because it only fires where a within-game
  invariant is violated, so **zero games straddle the split by construction**
- it **can miss a boundary**, when the next game's first recorded position stands
  at a later ply than the previous game's last. The two games merge into one
  block, and a merged block still lands wholly on one side — the cost is a
  coarser split, not a contaminated one

Measured on `selfplay_v2.tsv`: **119998 blocks against the 120000 games datagen
reported**, so at most 2 missed boundaries in 120000, 0.0017 %. At most, because
a game whose every position was filtered writes no rows and is indistinguishable
from a merge. The FEN's move number on its own misses 148, 0.123 %, because a
game ending with Black to move and the next starting at the same move number is
no descent in the move number.

A block is indivisible, so the held-out fraction overshoots `--validation` by up
to the length of the last block taken: 1100388 rows against the 1100369 asked
for at seed 1, 10.0002 % against 10 %. The run prints the game count and the
realised fraction on its first line and the emitted header records both.

Indivisibility binds at the other end too, so **the last block is never held
out**. A corpus of one game holds nothing out and says so:

```
2 positions, 1 games, 2 train, 0 validation (0.0000%), 827 parameters, group all, 827 free
WARNING: 1 game(s) in this corpus, so nothing could be held out. Every validation figure below is over an empty set.
```

Without that clamp the same two-row corpus at `--validation 0.5` held all of
itself out and `gradient()` divided by zero — `0 train, 2 validation
(100.0000%)`, then `epoch 1  train 0.000000  validation -nan`. Two-row corpora
are a real case here: S040 and S041 both measured the `--only` groups over one.

**Do not compare a held-out figure across the change.** The corpus was fitted
under both splitters at the same pinned K, seed and epoch budget: 0.117352 held
out at row level, 0.117380 at game level, a difference of 2.8e-05 against the
3.7e-04 the two held-out **sets** already differ by at the untuned constants. The
splitter was fixed because the old one shared games by construction, not because
a number moved, and 827 parameters over 9.9 M rows have no capacity to memorise a
game. The same two runs put the game-level index at **1090.87 s against 2333.91 s
for 3000 epochs**, 2.14x, because a shuffled row index random-walks a 1.2 GB
dataset every epoch and blocks walk it in near-file order.

**This makes `datagen`'s row order load-bearing.** A game's rows are contiguous
because the whole `samples` loop runs inside one `lock_guard` on `output_lock`,
`tools/datagen.cpp:334-343`. A change that interleaved two games' rows would
leave the reconstruction silently wrong and `tests/test_tuner_split.cpp` could
not see it: it holds the splitter, not the writer.

**Paste target is two files, and this is every definition a fit writes.** In
emitted order:

| emitted | paste over |
|---|---|
| `PAWN`, `KNIGHT`, `BISHOP`, `ROOK`, `QUEEN` defines | `src/eval_tables.hpp` |
| `psqt_mg[6][64]`, `psqt_eg[6][64]` | `src/eval_tables.hpp` |
| `mobility_mg[4]`, `mobility_eg[4]` | `src/evaluation.cpp` |
| `king_safety_mg[KS_FEATURE_COUNT]`, `king_safety_eg[KS_FEATURE_COUNT]` | `src/evaluation.cpp` |
| `passed_pawn_mg[6]`, `passed_pawn_eg[6]` | `src/evaluation.cpp` |
| `pawn_structure_mg[3]`, `pawn_structure_eg[3]` | `src/evaluation.cpp` |
| `piece_placement_mg[4]`, `piece_placement_eg[4]` | `src/evaluation.cpp` |
| `tempo_mg`, `tempo_eg`, two scalars and not an array | `src/evaluation.cpp` |

The emitted header names both files too. Everything below the two tables is the
54 parameters that do not live in `eval_tables.hpp`, and they are what gets left
behind: a fit that is half applied looks like a fit that did not work, and a
weight still holding the value the engine shipped is indistinguishable from a
term the fit had nothing to say about.

```bash
build/tools/tuner --data .tuning/selfplay_v1.tsv \
    --out .tuning/tuned_ks_only.hpp --only king_safety
```

`--only GROUP` fits one group and holds every other parameter at what the engine
ships. Groups are `all` (default), `material`, `psqt`, `mobility`,
`king_safety`, `passed_pawns`, `pawn_structure`, `piece_placement`, `tempo`, and
the emitted header records which was used. The eight named groups partition all
827 parameters: 5, 768, 8, 18, 12, 6, 8, 2 in that order, which the run's first
line reports as the free count.

**Adding a group means re-ending the one before it.** Each group is a contiguous
range and the last one runs to `PARAM_COUNT`, so a new group appended after it
is silently swallowed unless the previous group's end moves. That has now
happened four times — `king_safety` ran past the passed pawn weights,
`passed_pawns` past the pawn structure weights, `pawn_structure` past the piece
placement weights, `piece_placement` past the tempo weights — and the first four
would have failed no test. The symptom is a fit returning the new weights
exactly as it was handed them. `tempo` is the group running to `PARAM_COUNT`
today, so it is the one that swallows the fifth term.

`test_tuner_groups` is what a fifth occurrence fails now. S041 moved
`GROUP_LIST` and `free_mask` out of `tools/tuner.cpp` into
`tools/tuner_groups.hpp` so a test could reach them, and holds three properties
over the ranges: every group frees at least one parameter, the groups are
pairwise disjoint, and their union is exactly `[0, PARAM_COUNT)`. Disjointness
is the clause a swallowing breaks — the predecessor and the new group both claim
the new block. It needs no dataset and carries the `fast` label.

**This is an attribution tool, not a speed tool.** A joint fit of a new term
also refits the constants that were already fitted, so the SPRT that follows
measures two changes at once and its number says nothing about either. Frozen
parameters come out bit-identical, and Adam skips them entirely rather than
being fed a zero gradient — a decaying moment over a decaying velocity still
takes a step from a zero gradient.

What the freeze costs is measurable and was measured at S027: king safety alone
took held-out error 0.107413 to 0.107109, and fitting all 799 jointly reached
0.106964 instead — 799 because that was the whole vector when king safety was
fitted, before the four terms and 28 parameters that followed it. The refit of
the other 781 is worth 0.000145 and is a separate change.

**The agent runs the fit**, and every test and measurement, without asking.
DEC-041, which supersedes DEC-015 for tuning. A run of several hours is
scheduled for the night if there is better work to do meanwhile. The constants
still come back to be measured by SPRT like anything else. What DEC-015 still
holds is the S029 network training, which is the owner's and is asked again when
S029 arrives.

Before DEC-041 the rule was that the agent built the tools and the owner
executed the run. The first fit was delegated for that single run and recorded
as DEC-034 rather than left implicit; the second suspension in a day is what
said the rule did not fit.

That fit is the one in `eval_tables.hpp` today: 1490839 positions, K = 1.1141
fitted from the data, held-out error 0.113852 to 0.108043, stopped at epoch
11200 on patience. It measured +188.74 +/- 32.21 Elo over 438 games. Both error
figures were taken under the row-level split S066 replaced, so neither is
comparable with a held-out figure from a fit run today (2026-08-14) — and K is
refitted per corpus anyway.

`.tuning/` is gitignored. Datasets are hundreds of megabytes and are not
evidence in the sense `adocs/data/` is.

## Profile

```bash
cmake --build build-prof -j12
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
