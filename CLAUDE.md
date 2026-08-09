# Chesso — working notes for agents

Read this first. It records what has been decided, what has been measured, and
how to measure anything new. It exists so nobody has to explain the project
twice.

`README.md` is written by hand by the repository owner. **Do not write in it.**

## What this is

A UCI chess engine in C++20, bitboard based. The goal is a genuinely strong
open-source engine, worked on over a long period, with every change justified
by measurement rather than by argument.

## Decisions already made — do not re-open without being asked

- **C++**, targeting `gnu++20`. Evaluated against C, Rust and Zig; C++ wins on
  SIMD intrinsics, compile-time specialisation and the fact that the entire
  body of published chess-engine technique is written in it.
- **No code copied from other engines.** Ideas and published techniques are
  fine; source is not. Piece-square tables in `evaluation.cpp` are hand-written
  for this reason.
- **No Stockfish-derived NNUE training data.** The licence provenance of the
  network is to stay clean. Running the Stockfish binary as a tool - perft
  oracle, analysis, gauntlet opponent - creates no derivative work and is fine.
  Training data will come from self-play, decided later.
- **Evaluation is side-to-move relative.** Positive means the side to move is
  better. Callers apply no sign.

## Where the engine is

| area | state |
|---|---|
| move generation | legal-only, templated `<Color, Constrained, Type>`, ~90 Mnps perft |
| `generate_captures` / `generate_quiets` | partition `generate_moves` exactly |
| board | `board_t` 200 B, bitboards plus `squares[64]` |
| make/unmake | 16-byte history record, undo by xor, no board copy |
| evaluation | material plus tapered piece-square tables, `game_phase()` 24..0 |
| search | alpha-beta, transposition table, quiescence, staged generation, killers, history, countermoves, insufficient-material draws |
| **missing** | **PVS, null move pruning, late move reduction, aspiration windows, SEE, futility, razoring, singular extensions, delta pruning** |

The missing row is where the strength is. See `EVAL_PLAN.md`.

## Build and test

```bash
cmake --build build -j8          # release, the one that gets measured
ctest --test-dir build -L fast   # correctness, must stay green
ctest --test-dir build -L slow   # deep perft, minutes
```

Three build directories, all with `ccache` wired in:
`build` (Release), `build-debug` (asserts on), `build-prof` (RelWithDebInfo).

**Run test binaries from `tests/`** - they load assets by relative path:

```bash
cd tests && ../build-debug/tests/test_movegen
```

The debug build asserts `squares[]` against the bitboards on every make and
unmake. Any change to `make_move`, `unmake_move` or the generator must be run
through it.

`./clang-format.sh` pins clang-format major version 22 and refuses to run on
anything else, deliberately.

## How to measure — this is the part that matters

The project's history is a list of confident predictions that were wrong.
Follow the procedure rather than the intuition.

**1. Node counts before timings.** `bench_movegen` verifies perft counts before
printing anything. A generator that loses moves is faster and wrong.

**2. A change meant to be behaviour-neutral must prove it.** Same node counts
and same best moves from `tools/search_bench.py`. If they match, an SPRT is
pointless - the engines play identical games. Prove it with a determinism check
rather than burning an hour on a match.

**3. A change that alters play needs games.** `./fastchess.sh --fast`, or
`REF=<sha> ./fastchess.sh` to pick the baseline. The reference is built from a
git ref into `.ref-builds/`, so a result is always attributable to a commit
range.

**4. Know the noise floor before believing a number.** `bench_movegen` reports
its own resolution - the disagreement between the two halves of the run. It has
printed 0.1 % on an idle machine and 2.0 % on a busy one. Anything smaller than
that resolution has not been shown to exist. Check `ps aux | sort -rnk3 | head`
first; this machine has a habit of running `opendirectoryd` at half a core.

**5. Under 3 % is noise** unless `hyperfine` says otherwise with a tight sigma
over interleaved runs. On a loaded machine use the benchmark's own best-of-N,
which rejects interference by construction, rather than hyperfine's mean.

**6. One change at a time.** Two at once and neither number means anything.

**7. Published Elo figures from other engines do not transfer.** Staged move
generation was quoted at 30-50 Elo from other projects and measured **0** here,
because this engine lacks the pruning that makes those gains real. Treat
external numbers as direction, never as prediction.

**8. Record negative results.** Five changes have been measured at zero or
worse and the record is why they do not get retried. See `TMP_PLAN.md`.

## Tools

`TOOLCHAIN.md` has the full setup, the gotchas and the exact invocations. In
short:

| tool | for |
|---|---|
| `hyperfine` | A/B timing with statistics |
| `ccache` | rebuilds in under a second |
| `samply` + `tools/samply_report.py` | self time per function, as text |
| `clang-tidy`, `llvm-mca` | static analysis, hot-loop analysis |
| `tools/search_bench.py` | time and node counts for the **search**, not perft |
| `build/tests/bench_movegen` | perft and generator throughput |
| `./fastchess.sh` | SPRT against a commit |

### Reference engines in `~/.local/bin`

- **A fixed chesso build from `main`**, mailbox move generation. Two uses: an
  independent implementation to cross-check perft against, and a fixed rung to
  measure absolute progress. Chained SPRTs give relative gains that do not
  necessarily add up; a never-changing opponent catches that drift.
- **Stockfish.** Never an SPRT opponent - it is far too strong, every game is a
  loss and there is no signal. Use it for: `go perft N` as an oracle on
  arbitrary positions, analysing chesso's losses to find out what the engine is
  actually bad at, labelling positions for tuning, and as a calibration
  opponent limited by `go nodes N`. Prefer fixed nodes over `UCI_LimitStrength`,
  which injects random blunders and is high variance.

## Conventions

- **A bug that has been found gets fixed before anything else starts.** Not
  noted, not scheduled, not carried into the next change. A known defect
  sitting in the tree contaminates every measurement taken after it and makes
  the next bug harder to attribute. This is the rule with the fewest exceptions
  in the project.
- **Never weaken a test to make it pass.** If a test looks wrong, stop and say
  so. When a change alters a contract the test encodes, rewrite the test to
  assert the new behaviour - do not relax it. Several tests in this repository
  were rewritten that way and each one is explained in its commit message.
- Tests carry `ANCHOR` comments where they pin exact numbers. Those are meant
  to be revisited deliberately when the evaluation changes.
- Commit freely; **do not push**.
- Comment why, not what. `snake_case`. Match the surrounding style.
- Commit messages explain the reasoning and carry the measurement, including
  when the measurement was disappointing.

## Known hazards and one-way doors

- ~~`make_move` mutates pieces at six open-coded sites~~ **Closed.** Every
  change goes through `add_piece` / `remove_piece` / `move_piece`, which is the
  alphabet an NNUE accumulator is updated from. `unmake_move` deliberately does
  not use them: an accumulator is kept per ply in the search stack and popped,
  never reverse-updated, so only the forward direction needs the events.
- ~~`game_t` is 2.5 MB~~ **Closed.** The attack tables are a process-wide
  singleton behind `game_tables()`; `game_t` is 87 KB, of which 80 KB is the
  move history.
- **The move-ordering bands clear each other by 100 points.** A king capturing a
  pawn scores `1000000 + 100 - 100000 = 900100`, against 900000 for a killer.
  Tuning `piece_values_abs` can invert that silently.
- **`evaluate()` is about 25 % of nodes per second** and runs at every
  quiescence node, because the piece-square tables are recomputed rather than
  accumulated. Decide that before adding more evaluation terms, not after.
- `.ref-builds/` holds git worktrees created by `fastchess.sh`. Gitignored.

## Where the plans live

- `EVAL_PLAN.md` — evaluation, move ordering, and the search features that
  decide whether evaluation work pays. The current roadmap.
- `TMP_PLAN.md` — the move generation work, now mostly historical record,
  including every measurement that came out wrong and why.
- `TOOLCHAIN.md` — tool setup and the traps in each one.
