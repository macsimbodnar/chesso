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
| `books/` | opening books for match play. `8moves_v3.pgn` is committed and is what `rating.sh` plays; the unbalanced book `fastchess.sh` plays is 175 MB, gitignored, and fetched by `books/fetch_book.sh` against a pinned digest. Every book here comes from `official-stockfish/books` (CC0-1.0) and every one of them, committed or fetched, is pinned by both digests in that script |
| `.ref-builds/` | git worktrees created by `fastchess.sh`, gitignored |
| tool config | tracked when it describes the project, ignored when it describes a machine. `CLAUDE.md` and `.cursor/rules/moltke.mdc` are the two agent pointers at `AGENTS.md`, `.vscode/` is the editor setup; `.claude/settings.local.json` is the one exception and is gitignored. DEC-051 |

Four build directories, all with `ccache` wired in: `build` (Release, the one
that gets measured), `build-debug` (asserts on), `build-prof` (RelWithDebInfo),
`build-tune` (Release with `-DCHESSO_TUNE=ON`, a tuning harness and **not** the
binary anything is measured on — see "The tune build" below). `build_release.sh`
adds `build-release-<arch>` per distributable target and `.pgo/` for the profile;
both are gitignored and neither is what a local measurement runs.

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
cmake -S . -B build-tune  -DCMAKE_BUILD_TYPE=Release        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCHESSO_TUNE=ON
```

Do not point CMake at Homebrew clang. Different compiler, different codegen, and
every recorded benchmark becomes incomparable.

## The instruction set, and which binary ships

`-DCHESSO_ARCH=`, four values, and the default is the one that must never be
distributed. `cmake/arch.cmake` is the table and every flag set in it is put to
the compiler by `check_cxx_compiler_flag` before it is used — a silently dropped
flag would leave a target named for an instruction set it does not target.

| `CHESSO_ARCH` | flags | for |
|---|---|---|
| `bmi2` | `-march=x86-64-v3` | **the target that ships to a rating list.** Intel Haswell (2013) onward, AMD Zen3 (2020) onward |
| `avx2` | `-march=x86-64-v3 -mno-bmi2` | Zen1 and Zen2, where PEXT and PDEP are microcoded at about 18 cycles against Intel's 3. `__BMI2__` is undefined here and that is the switch S032's magic path keys off |
| `portable` | `-march=x86-64-v2` | the fallback. SSE4.2 and POPCNT, so Nehalem (2008) and Bulldozer (2011) onward |
| `native` | `-march=native` | **this machine only, default, never distributed.** `build/` is the directory that gets measured and a local measurement wants the code the machine can run; the same file on anything older than this part raises SIGILL rather than running slowly. `build_release.sh` refuses the name |

The step that added this is S104 and the reason it exists is one number: the
release build carried **no** architecture flag, so `std::popcount` compiled to a
software SWAR popcount and `objdump -d build/src/chesso | grep -c popcnt`
returned **0** at `20d058a`. `count_bits` runs once per piece per evaluation in
the mobility and king-safety loops, in `game_phase`, and throughout move
generation. Configure time says which target you have:

```
-- Arch native (-march=native) -- THIS MACHINE ONLY, never distributed
-- Arch bmi2 (-march=x86-64-v3)
```

The arch flags are directory wide and set before any target exists, so the tests
and the benchmarks are built for the same instruction set as the engine. A
benchmark built for a different target is measuring a different binary.

## The release build: profile-guided, one arch at a time

```bash
./build_release.sh bmi2        # ships. Output: build-release-bmi2/src/chesso
./build_release.sh all         # bmi2, avx2, portable
```

Three passes — instrumented build, workload, optimised build — in **one** build
directory, over a profile under `.pgo/<arch>` which is gitignored and regenerated
per build. A profile is a property of one workload run against one source tree and
a stale one reads as provenance while being noise, so `-Wcoverage-mismatch` is
left as the error `-Werror` makes it: a profile that does not match these sources
stops the build rather than being corrected past. The flags are scoped to
`chesso_engine` and `chesso` rather than added directory wide, so `ctest` and the
tuner stay out of a two-pass build they gain nothing from.

**One directory, and it is load bearing.** GCC names a `.gcda` by mangling the
absolute path of the object that wrote it —
`#home#max#ws#chesso#build-release-bmi2#src#CMakeFiles#chesso_engine.dir#search.cpp.gcda`
— and looks it up under the same name on the way back in. Two build directories
give two different object paths, so every lookup misses, every translation unit
compiles with no profile at all, and the binary is an ordinary `-O3` build wearing
a PGO label. **That is not hypothetical: it is what the first version of this
script did, and it measured as PGO being worth zero.** Pass 3 reconfigures the
same directory.

**`-Wmissing-profile` is the check that catches it, so nothing silences it
tree-wide.** Under `-Werror` a missing profile stops the build, which is the right
outcome — a binary that quietly is not profile-guided is worse than one that does
not build. Blanket-silencing it was the first version of `cmake/pgo.cmake` and it
is what let the two-directory bug hide. Exactly one translation unit is exempt, by
name: every caller of `src/search_params.cpp` sits inside `#ifdef CHESSO_TUNE`
(`src/chesso.cpp:934` and `:1043`), so a release link references none of its
symbols, the linker drops the object out of the static library, its gcov
constructor never runs and no `.gcda` can be written for it by any workload. Eight
of the nine files in `src/` get a profile and the error stays live for all eight.

**The workload is the part that decides whether PGO is worth anything.** It is a
fixed-depth search — depth 10, `PGO_DEPTH` overrides — over **400 positions, 16
from each of the 25 values of the engine's own `game_phase()`**, drawn from
`adocs/data/S018_raw.tsv`, which is 13522 positions chesso actually reached in
210 of its own games. 117329931 nodes, about 63 s instrumented. Two things it is
deliberately not:

- **not perft.** perft measures generate, make and unmake. It never evaluates,
  never orders a move and never probes the table, so a profile taken from it
  would tell the compiler that three quarters of the hot code is cold.
- **not one position.** The branch that matters in an endgame is not the branch
  that matters in a middlegame. Stratifying by phase is what stops the middlegame
  answering for the endgame, the same sampling `adocs/data/S021_aspiration_sweep.py`
  uses and for the same reason.

It runs serially through one process, which is a choice against DEC-050's
default: twelve concurrent instrumented processes would finish in a twelfth of
the time, but `.gcda` merge-on-exit would then depend on the scheduler and the
profile would stop being a function of the source tree. One process also leaves
the transposition table warm across positions, which is the regime a game's
later moves search in.

The driver waits for `bestmove` between positions and that is not optional — see
"Wait for `bestmove` when you script it" below. The first version of it did not,
and the 400-position workload finished in under a second having profiled a
depth-1 tree.

**What it bought, measured 2026-08-19 against `20d058a`.** One interleaved run,
10 rotating triples of base / arch-only / arch+PGO so a first-or-last bias
cancels, each run 400 positions at depth 10 **that the profile was not trained
on** — timing a profile-guided binary on its own workload is not a measurement of
it:

```
base (no arch flag, no PGO)   median 17.409s   spread 6.70 %
bmi2, no PGO                  median 15.432s   +12.62 %   CI +11.19 .. +14.06
bmi2 + PGO, what ships        median 14.722s   +18.22 %   CI +16.19 .. +20.28
```

Ratio 0.8459 geometric mean, paired **t −21.90** over 10 pairs. The arch flag is
+12.62 % and the profile the remaining **+4.98 %**. At the published 1.43 Elo per
percent that is **+26.1 Elo — a conversion, not a verdict**, and DEC-083 is why no
SPRT is owed: the node count is identical, so the two binaries play identical
games and a match between them measures nothing but the clock.

**The overfit is small and was checked rather than assumed.** The same run over
the 400 *training* positions reads +18.65 % (CI +18.47 .. +18.83) for the shipping
build against +18.22 % held out — 0.43 points. That run also shows what a quiet
machine looks like: spread 0.45 % against 6.70 % on the held-out run an hour
earlier. **The pairing is what carries the result, not the timer.**

Machine resolution beside it, same session: `bench_movegen` **814.3 ms** for 12 M
perft nodes, **resolution 0.2 %**, spread 0.5 %; `bench_eval` **53.90 ns a call,
resolution 0.2 %**, spread 3.9 %.

`build_release.sh` builds `--target chesso` and nothing else, so the release
directories carry no test binaries. To run the suite against the binary that
ships, build the rest of that directory first — the tests are not profile-guided,
only `chesso_engine` and `chesso` are, so this adds nothing to what ships:

```bash
cmake --build build-release-bmi2 -j12 && ctest --test-dir build-release-bmi2 -L fast
```

## The tune build

`-DCHESSO_TUNE=ON`. Every parameter in `src/search_params.hpp` stops being an
`inline constexpr int` the compiler folds and becomes a variable settable over
UCI. S073 built it so the hand-tunes, S068 and S039, and the SPSA driver S084
need one build rather than one build per point measured. S068 is done and used it
for exactly one thing — node counts and suite runs, never a strength number,
which is the rule immediately below. The sweeps in `adocs/data/S033_rfp_*.sh`
recompiled per point with `-DRFP_MARGIN=$margin`, which
`2026-08-16_plan_review-F04` found no longer builds; `setoption` replaces it and
the scripts are historical evidence, not a method to re-run.

**It is not the release binary and no strength number is ever taken on it.** A
constant the compiler folds is not the same code as a variable it must load, and
the difference shows up in a timed match rather than in a node count. S085 is
where a tuned value comes back and is measured by an SPRT of the **shipping**
build carrying it.

```bash
cmake --build build-tune -j12
./build-tune/src/chesso
setoption name RfpMargin value 120
```

The parameters, their defaults and their ranges are the table in `MANUAL.md`. An
out-of-range value is refused rather than clamped
(`src/search_params.hpp`, `search_param_set`), and so is a name that is not in
the table.

**Both refusals are observable since S137, and this paragraph said otherwise
twice before that.** Each one prints a single `info string` line on stdout, in
the tune build only:

```
info string refused [<name>] value <value>, outside [<min>, <max>]
info string refused [<name>] value <value>, not an integer, range [<min>, <max>]
info string refused [<name>], unknown option
```

A legal value prints nothing, which is what makes a line evidence rather than
narration. The whole value has to be an integer: `0x50`, `120.9` and `12x` are
refused rather than read up to the first character that does not fit, which is
what `std::stoi` did until the review of S137's own diff. `Use Book`, `Hash` and
`Threads` are outside all of this — their handlers are the release build's, so a
bad value for one of them is still log-only, which under `NDEBUG` is nothing.
`Book File` is the one exception S172 added: a book it cannot load is reported
as `info string book [<path>] not loaded: <why>` in both builds, because the
value is a path a person typed and silence there reads as success.

Until S137 the refusal went to `LOG_W` — `if (false) std::clog` under `NDEBUG`
(`src/log.hpp`), and `build-tune` is a Release build — so a tuner that sent an
impossible value or misspelled a name played its games against a compiled
default and could not tell. DEC-093.

There is still **no readback**: `uci` re-prints each parameter's compiled
default, not its live value.

```
setoption name RfpMargin value 120
uci                     # still prints "default 75"
```

So the confirmation a value was taken is the absence of a refusal line plus the
node count below, not a query. `tools/spsa_driver.py` clamps every value itself
regardless — a driver that depends on the engine refusing correctly has moved
its own correctness into the thing it is measuring.

It works, and this is the measurement rather than the claim. The midgame
position of `tools/search_bench.py` at depth 9, driven over UCI on
`build-tune`, **re-measured at S137's commit**: **164302 nodes at `RfpMargin`
75, 223857 at 100, 474204 at 300, 742729 at 2000**, best move `c3d5`
throughout, and 164302 again with no `setoption` sent at all. 164302 is what
the release build reports, to the node — 75 is the shipping default since S068
(2026-08-17), and the 100 figure was the one that matched before that step.

**Those numbers move with the search and this paragraph has been stale once.**
It read 213509 / 292313 / 549374 / 917971 from S068's completing commit until
2026-08-19 — correct when written, and wrong by the time it was next read,
because every step that changes the tree changes it and none of them came here.
Which step first broke it is not recorded and was not dug out; S021 and S094
both moved the release count. A number in this file is a claim about the
current binary, so re-measure it rather than quote it, the same way
`search_bench.py`'s own figures below carry the commit they were taken at.

**Wait for `bestmove` when you script it.** `go` runs on its own thread, so
`printf 'go depth 9\nquit\n' | ./build-tune/src/chesso` sends `quit` into a
running search and reports the depth-1 count — 283 nodes on that position, from
a search that was cut off rather than finished. `tools/search_bench.py` reads
until `bestmove` and is the shape to copy.

Configure time says which one you have:

```
-- Tune build ON: the search parameters are UCI options. NOT the release binary
-- Tune build OFF
```

Two things about it are held by tests rather than by care:

- `tests/test_search_params.cpp` compiles into **both** builds and holds each
  one's live values against the rows `search_param_info()` returns from
  `src/search_params.cpp`, which sit outside every `#ifdef CHESSO_TUNE` and are
  therefore the same rows in both. A binary cannot be two builds at once, so
  the two are compared through that common anchor: `live == defaults` in each
  build gives `live == live` across them, member by member. It also pins each
  default to the value that ships, so a step that changes one says so in the
  same commit.
- Every `setoption` rebuilds whatever was derived from a parameter.
  `lmr_table` is built once from `LMR_BASE` and `LMR_DIVISOR`, so a setter that
  moved a coefficient and left the table alone would report success and change
  nothing; `search_lmr_reduction_probe()` exists in the tune build for the test
  that would catch it.

`test_uci_surface` grows one expected option line per parameter under
`CHESSO_TUNE` and requires `MANUAL.md` to document every name, so a parameter
added here cannot reach a tuner undocumented. The release build's three golden
option lines are asserted to still be exactly three.

Neutrality, measured at S073's commit against `e78faed`, all three positions at
depth 9 and both configurations: **292313 / 1026739 / 103001 nodes, best moves
`c3d5` / `e2a6` / `d7c8q`**, identical for `build` before the change, `build`
after it, and `build-tune` with no `setoption` sent. That is INV-6 discharged;
no SPRT is owed.

## Tune search parameters with SPSA

`tools/spsa_driver.py`, S084. Simultaneous perturbation stochastic
approximation over the tune build's UCI parameters: two objective evaluations
per iteration whatever the dimensionality, so twenty-two parameters cost what
one costs. The alternative is what S068 and S039 did — one step, one source
edit, one rebuild and one SPRT per number.

The installed `fastchess alpha 1.8.2 20260729-74deac2` has no tuning mode of its
own, so the driver builds the match itself: one short `fastchess` run per
iteration, theta+ as the first `-engine` and theta- as the second, `-games 2
-repeat -rounds <pairs>`, no `-sprt`. `fastchess.sh` is untouched — that is the
SPRT harness and this is not an SPRT.

Validate a config before spending anything on it:

```bash
tools/spsa_driver.py check tools/spsa_dryrun.json --engine build-tune/src/chesso
```

That is not a formality. Every name is checked against the binary's own `uci`
listing and every bound against the binary's, before any option is sent — the
`info string` refusals S137 added are a backstop for a run already under way,
not a substitute for validating a config that has not started. Then it proves
`setoption` reaches the search at all: measured at S137's commit, `RfpMargin` 75
searched **164302** nodes and 2000 searched **742729** on the midgame position
at depth 9, the same two figures this file quotes above.

Run it:

```bash
tools/spsa_driver.py run <config.json> --out .spsa/<name>
```

`--resume` continues from the checkpoint. `.spsa/` is gitignored.

### The config

`tools/spsa_dryrun.json` is the smoke config and the format, verbatim:

```json
{
  "iterations": 2,
  "pairs_per_iter": 2,
  "seed": 1,
  "r_end": 0.002,
  "match": {
    "engine": "build-tune/src/chesso",
    "tc": "1+0.01",
    "book": "books/8moves_v3.pgn",
    "book_format": "pgn",
    "book_size": 34700,
    "hash": 16,
    "threads": 1,
    "concurrency": 12
  },
  "params": [
    { "name": "RfpMargin",       "start": 75,  "min": 0, "max": 2000, "c_end": 4 },
    { "name": "LmrDivisor",      "start": 225, "min": 1, "max": 2000, "c_end": 8 },
    { "name": "MaxQsearchDepth", "start": 8,   "min": 1, "max": 64,   "c_end": 1 }
  ]
}
```

`alpha` 0.602, `gamma` 0.101, `a_ratio` 0.1 and `r_end` 0.002 are the defaults
and may be omitted. **`c_end` is the smallest change in that parameter that
could plausibly matter**, and it is per parameter: below 0.5 the config is
refused, because `round(x + c) == round(x - c)` and the axis would be perturbed
and never move. `book_size` is the opening count, used to wrap a run that
outlives the book — `grep -c '^\[Event' books/8moves_v3.pgn` is 34700 and
`books/UHO_Lichess_4852_v1.epd` is 2632036 lines.

`(wins - losses)` is summed over the iteration's pairs, so **doubling
`pairs_per_iter` doubles the step**. The two are chosen together and frozen
together.

### What a run leaves behind

`run.json` (the frozen config, written before the first game), `checkpoint.json`
(rewritten atomically every iteration: k, the float theta, the RNG state, the
pair count), `trajectory.tsv` (one row per iteration), `engine.snapshot` (the
binary copied once, so a rebuild mid-run cannot swap the engine under it, which
has already happened to the SPRT harness once), `games.pgn` and
`fastchess.log`.

The trajectory is what shows a run that is not working, and the two failure
shapes look nothing alike:

```
k	pairs	c_scale	r_k	y	wins	losses	draws	ptnml	RfpMargin	LmrDivisor	MaxQsearchDepth
0	2	1.072517	0.0025043399	2	3	1	0	0/0/1/0/1	75	225	8
1	4	1.000000	0.002	-3	0	3	1	1/1/0/0/0	75	225	8
```

That is the dry run above, and **nothing moved** — correct, and the reason the
smoke run cannot be read as a tuning result: two iterations at `r_end` 0.002
step `RfpMargin` by 0.02, which rounds back to 75. A real run whose parameters
are still barely moving after a few thousand games is not converging slowly, it
is useless; a parameter that walks its whole range and settles nowhere is
DEC-019's flatness, and its endpoint is noise.

Count forfeits from the PGN, never from `fastchess.log`, which is WARN-only and
comes back empty (S089):

```bash
tools/forfeit_report.py .spsa/<name>/games.pgn
```

### Detach it and watch the marker

The last line is `SPSA-DONE` or `SPSA-FAILED`, both of them, so a watcher exits
on the run rather than on a turn boundary (DEC-061):

```bash
nohup tools/spsa_driver.py run <config.json> --out .spsa/<name> > .spsa/<name>.log 2>&1 &
```

### Sizing a real run

S084 built the driver; **S085 measured what a real run costs and overturned four
of the constants the plan had seeded.** Read this before writing a config, and
re-measure rather than inherit -- these are figures for this machine at 12
concurrency (DEC-050).

**The budget floor is a resolution floor.** On a 12-axis synthetic objective at
2500 x 6 pairs, 30000 games resolves an objective about **36 Elo deep** and up.
At 15 Elo it recovers a sixth of what is available and goes backwards on an
unlucky seed; at 6 Elo it does nothing at any step size, and the signal run and
a zero-weight flat run drift by the same amount. Below that, a run cannot tell
you anything and the games are spent either way.

**`c_end` is the largest lever, and the obvious reading of it is wrong.** Sized
as "the smallest change that could matter" -- endpoint precision -- the seeds got
+2.3 Elo of an available 15. **Four times larger got +10.5**, which is where
`+/-c` costs about 1 Elo, and that is fishtest's own "`+/-c` should cost a few
Elo" rule reached from the other end. The best multiplier tracks a
distance-to-optimum nobody knows, but the loss is asymmetric and that is what
decides it: too small costs a couple of Elo, **too large costs tens** -- 8x
measured -39 and 16x measured -250 on an axis already near its optimum. 2x and
4x were both positive at every distance tested; 4x is the recommendation because
it stays within about 2.5 Elo of the best at every distance while 2x falls up to
10 Elo behind once the optimum is more than a few `c_end` away (+1.68 against
+11.65 at the widest tested). S085's own outcome is evidence for the wide case:
`MaxQsearchDepth` moved 2.75 `c_end` and `RfpMaxDepth` 2.25, so the real
distances were not small. Note that S085's step file states this as "4x is the
only multiplier positive at every width", which its own table contradicts --
`adocs/plan_done/` is never rewritten, so the correction lives here. Cap it per parameter
wherever a comment documents a behavioural cliff a quadratic objective cannot
express.

**Never scale a games-per-hour figure between time controls.** A game's duration
is set by its clock, not by how fast the engine searches: twelve games on twelve
SMT threads each still spend `(base + moves*inc)*2` seconds. S085's plan chained
two ratios off an older measurement and priced 5+0.05 at 7 h for 30000 games; it
is **17 h**. Measured, 12-game waves:

| tc | s per wave | median depth | p10 depth |
|---|---|---|---|
| 1+0.01 | 6.9 | 9 | 8 |
| 2+0.02 | 10.9 | 11 | 9 |
| 3+0.03 | 11.9 | 11 | 9 |
| 5+0.05 | 24.4 | 12 | 10 |

Depth is the thing to check, not the control: the depth-gated parameters have to
be exercised. 2+0.02 reaches median 11 against 5+0.05's 12 for 45 % of the cost.
0 forfeits in 120 games at 2+0.02 and at 3+0.03.

**Size the batch at fixed wall clock, not fixed games -- it inverts the answer.**
At a fixed game count the objective prefers small batches, 2 or 3 pairs over 6.
But the driver runs one `fastchess` per iteration and cannot finish until the
slowest of its games does, so a small batch pays that straggler every iteration.
Measured at 2+0.02, three reps each:

| pairs | s/iteration | s/pair |
|---|---|---|
| 3 | 7.60 | 2.53 |
| 6 | 9.14 | 1.52 |
| 12 | 15.14 | 1.26 |
| 24 | 23.47 | 0.98 |
| 48 | 40.97 | 0.85 |

More than half of a 6-pair iteration is waiting. Priced in 8 h of machine, **24
pairs at `r_end` 0.004 was the optimum** and 48 tied it with half the
iterations, so the turnover is findable rather than assumed. `r_end` scales with
*total pairs*, not with the split, because `(wins - losses)` is summed over the
iteration's pairs.

S085's frozen config is `tools/spsa_s085.json` and is the worked example: 12
parameters, 1250 x 24 pairs, 60000 games in 8 h 21 m, 0 forfeits, verified at
+21.02 +/- 9.86 Elo.

**Tune and verify on different openings and a different control.**
`adocs/eval_tuning_strategy.md` par.7. `books/fetch_book.sh` pins a second
UHO-class book, `UHO_4060_v3.epd`, for exactly this: the run tunes on it and
`fastchess.sh` verifies on `UHO_Lichess_4852_v1.epd`.

### What decides whether it worked

`tests/test_spsa_driver.py`, in the fast suite, 22 assertions and about four
seconds. It plays no games: the objective is a noisy quadratic with a known
optimum, because a driver tested by playing games cannot tell a bug in itself
from noise in the objective at any budget this machine can afford — a sign error
and a parameter that does not matter produce the same flat trajectory. The
sign-flipped run is asserted to **fail** the criterion the honest one passes.

The constants are measured there, not inherited. On that objective, at 20000
pairs, with the axis starting 300 units from its optimum:

| `r_end` | max axis error | final strength |
|---|---|---|
| 0.002 | 293 of 300 | -21.8 Elo |
| 0.008 | 28 of 300 | -0.9 Elo |
| 0.050 | 221 of 300 | -6.2 Elo |

0.002 is the OpenBench and fishtest-era seed and it moves that objective 2 % of
the way. The bottom row is fishtest RFC #535's complaint measured here: an
oversized end value does not decay away, because the end-value parametrisation
shrinks the step by only about 1.6x across a whole run. **`r_end` is calibrated
against the budget and the objective's steepness, not inherited** (DEC-084).

Choosing which of the 22 parameters to tune, their bounds, their `c_end`s and
the budget is S085's, recorded there. And the returned vector is a hypothesis:
no strength number is taken on the tune build, so S085 SPRTs the **shipping**
build carrying it.

## Test

```bash
ctest --test-dir build -L fast    # correctness, must stay green, about 80 s
ctest --test-dir build -L slow    # deep perft, minutes
```

`test_perft`, the slow label's one binary, **exits non-zero on a missing or an
unparseable asset** and on a mismatch in any of the five columns it reads —
`nodes`, `captures`, `en_passant`, `castles`, `promotions`. Both used to be
`assert`, which the Release build the gate runs compiles out, so a missing asset
iterated zero cases and exited 0; and only `nodes` reached the pass flag, so the
other four printed red and the run passed anyway (S193). It reads its assets
relative to the working directory, so run it through `ctest` or from
`build/tests`.

Every doctest binary fills the attack tables through a **fixture**, not through
a first case that happens to run first. So `-tc=<glob>` over a single case and
`--order-by=name` are both safe ways to run this suite; before S193,
`./test_chesso -tc="Basic test"` answered 20 moves as 16 in Release and aborted
in Debug.

### The gate, `tools/gate.sh`

One command runs the TESTS rule's whole chain and then checks the commit's
signature. Run it before committing, on the message you are about to use, and
again on the commit once it exists:

```bash
export CLANG_FORMAT_MAJOR=22          # this machine only, DEC-146
tools/gate.sh --message .git/COMMIT_MSG   # the staged tree, message not yet committed
tools/gate.sh                             # HEAD
tools/gate.sh <ref>                       # any other commit
tools/gate.sh --build-parent              # for 'No functional change' over an
                                          # ancestry that predates the rule
```

It prints `GATE-DONE <total>` or `GATE-FAILED: <reason>` as its last line,
success and failure both, so a detached run can be watched (WATCHERS rule).
What it does, in order: `cmake --build build` and the `fast` suite, the same
two for `build-tune`, `./clang-format.sh --check`, then the signature.

What it enforces is DEC-140. A commit touching `src/` carries exactly one of
`Bench: <n>` or `No functional change`, and the gate refuses a message that
carries neither, both, a wrong `<n>`, or an unsupportable `No functional
change` — for which it reads the parent's total off the nearest ancestor
`Bench:` line, or builds the parent with `--build-parent`. A commit touching
nothing under `src/` owes neither line. In `--message` mode the tree under test
is the staged one, so the run is refused outright when the working tree has
unstaged edits: a signature taken from a tree the commit will not contain is
worse than no signature.

`tools/search_bench.py` keeps its own role, which is timing and per-position
counts. The gate is one number for the whole tree; `search_bench.py` is three
numbers you can attribute.

Those 60 s -- 28 tests, measured 2026-09-08, against 27 tests and 52 s at
`6688a01`, 48 s over 23 before S170 added `test_mate_carry` at 6.6 s and 42 s
over 22 before S147 added `test_mate_pv` at 3.1 s. S189 accounts for the last
7 s: `test_uci_surface` went from 0.03 s to 6.8 s, because its new case
searches the whole bench set twice at `BENCH_DEPTH`, and the one test it added,
`test_gate_script`, costs 0.24 s. The label total moves a second or two between
runs, so read it as a size and not as a stopwatch -- assume `build/` was
configured `Release`. Configured `Debug`, or with an empty `CMAKE_BUILD_TYPE`,
the same suite takes about two minutes — the assertions are on and the
optimiser is off — and `test_movegen` and `test_search` take 165 s and 216 s on
their own. **Since S189 `test_uci_surface` joins them at 157 s in `Debug`**,
measured 2026-09-08 against 6.8 s in `Release`: its bench case searches the
eight-position set twice at `BENCH_DEPTH`, and that is a search, so it pays the
Debug factor like the other two. It is the shipping depth on purpose — a case
that benched shallower would not be testing the number that ships — and the
cost lands on the Debug gate, not on the per-commit one. Until
S067 every `fast` target carried a flat 60 s timeout, so a debug directory
reported those two as `Timeout`, which reads as two test failures rather than
as a wrongly configured build directory. It happened twice on 2026-08-13, the
second time because an editor rewrote the directory mid-session (DEC-052).

The timeout is now the build's: 60 s for `Release` and `MinSizeRel`, 600 s
otherwise, printed at configure time as `-- Test timeout 60s (build type
Release)`. A debug directory finishes rather than timing out, so the two
invariants that live in its assertions have a working `ctest` invocation. Check
the build type anyway before believing a timing:

```bash
grep -E 'CMAKE_BUILD_TYPE|CMAKE_C_COMPILER:' build/CMakeCache.txt
```

Piping `ctest` into `tail` or `head` hides its exit status behind the pipe, so a
gate written that way reports success on a failed suite. Redirect to a file and
read `$?`, or let `ctest` print in full.

The step-completion gate is the TESTS rule in `AGENTS.md`, and it covers
**both** builds:

```bash
cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check
```

It lived in `.moltke.json` and ran automatically until 2026-08-29; moltke v1
has no hooks and no marker file, so it is a rule an agent follows and nothing
runs it for you (DEC-109). `-j8` is this machine's core count -- the gate read
`-j12` while the Linux machine DEC-049 names was the one in use. It is `&&`
throughout on purpose: every stage's exit status reaches the shell, and the
first failure stops the chain.

**Why `build-tune` is in it, S143 and DEC-118.** `src/search_params.hpp` is
deliberately different code in the two builds -- `inline constexpr int` in the
shipping one, a plain `int` settable over UCI in the tune one (S073) -- so they
can diverge, and for the whole of the project's history only the shipping one
was built here. A `static_assert(ASPIRATION_MIN_DEPTH >= 2)` added to
`tests/test_engine.cpp` while fixing an S085 review finding compiled in `build`
and did not compile in `build-tune`:

```
tests/test_engine.cpp:1628:19: error: static assertion expression is not an
  integral constant expression
  note: read of non-const variable 'ASPIRATION_MIN_DEPTH' is not allowed in a
  constant expression
```

The old gate went green on that tree. `build-tune` is not a convenience: it is
the binary every tuning run plays -- S085's SPSA drove it for 60000 games, S127
will drive it again -- so a break in it surfaces whenever someone next tries to
tune, which can be months after the commit that caused it.

**What it costs, measured 2026-09-01 on the DEC-109 MacBook, 8 cores, on
mains.** The tune build's `fast` label is **45.9 s over 22 tests**, against the
shipping build's 42.4 s over the same 22 — 48.8 s and 48.3 s over 23 since
S147 added `test_mate_pv`, and **51.6 s and 51.5 s over 24** since S170 added
`test_mate_carry`, both on 2026-09-02; building `build-tune` adds **0.5 s**
when nothing changed, **1.4 s** for a full rebuild with its ccache warm, and
**18.0 s** for a full rebuild with `CCACHE_DISABLE=1`. So the gate roughly
doubles: **107.9 s** end to end on a warm tree measured 2026-09-02 with S170's
test in it, 93.6 s when S143 first measured it, against about 45 s before.
`build/` is configured without a compiler launcher and `build-tune/` with
`ccache`, which is why only the second figure has a warm-cache case.

`build-tune/` is gitignored like every `build*` directory, so a fresh clone
configures it once from the Build section above before the gate can run.

That gate is necessary and not sufficient. Deep perft, the debug-build
assertions and any SPRT are named in each step's `accepts:` field and run by
hand. DEC-025.

**Run test binaries from `tests/`** — they load assets by relative path:

```bash
cd tests && ../build-debug/tests/test_movegen
```

Or through ctest, which since S067 no longer times the debug build out:

```bash
cmake --build build-debug -j12 && ctest --test-dir build-debug -L fast
```

`test_invariants` is INV-2 and INV-4 in the gate, since S190. It walks every
test FEN two plies deep plus the five positions `test_engine`'s hash oracle
uses at their own depths -- 2132167 `make_move` calls -- and after every make
and unmake rebuilds the four evaluation accumulators with `eval_refresh()`,
rebuilds `squares[]` from the bitboards, and compares the whole `board_t`
against its pre-make copy. 1.816 s +/- 0.018 s over ten hyperfine runs in
Release and 12.7 s in Debug, on the machine `.moltke.local.md` describes. The five census floors it asserts are
goldens: re-derive them with `adocs/data/S190_walk_census.py`, which counts the
same tree with python-chess, whenever the corpus or either depth moves
(DEC-142).

The debug build asserts the same two things inside `make_move` and
`unmake_move` themselves, which is a stricter place to catch them: the assert
fires at the entry and exit of the call rather than after it returns. That half
is **not** in the automatic gate — both gated build directories are Release,
where every `assert` is dead. It is stage 3 of `tools/gate_extra.sh` below; by
hand it is:

```bash
cmake --build build-debug -j12 && ctest --test-dir build-debug -L fast
```

### The extra gate, `tools/gate_extra.sh`

Everything the `fast` label cannot hold, in one script with one terminal marker.
S197. It is **not** automatic and never becomes automatic — DEC-025 keeps the
per-commit gate at `tools/gate.sh` — and **DEC-141 clause 3 says when it runs**:
before a step that touched `make_move`, `unmake_move`, the generator or the
search completes, and otherwise weekly. The date and sha of the last
`GATE-EXTRA-DONE` are a bullet of their own in `adocs/status.md`, so a missed
week is visible.

```bash
tools/gate_extra.sh                             # all five stages
STAGES="sanitize perft" tools/gate_extra.sh     # a subset, after a fix
JOBS=6 OUT=/tmp/run tools/gate_extra.sh         # override cores and outdir
```

`JOBS` defaults to `nproc`, then `sysctl -n hw.logicalcpu`; `OUT` defaults to
`/tmp/chesso_gate_extra_<stamp>` and holds one log per stage,
`NN_<stage>.log`. The five stages run cheapest first, and **every stage runs
even after one fails** — the weekly run's value is the whole picture in one
pass, so the marker names every stage that failed rather than the first:

| # | stage | what it catches that the automatic gate cannot |
|---|---|---|
| 1 | `prose` | stale tense in `adocs/plan.md`, `tools/plan_prose_check.py --prose` |
| 2 | `citations` | a code citation in a pending step file that no longer resolves to its symbol |
| 3 | `debug` | INV-2 and INV-4 and every other `assert(` in `src/`, over the six binaries that drive `make_move` — `test_chesso`, `test_openings`, `test_movegen`, `test_evaluation`, `test_search`, `test_engine`. Both gated builds are Release, where those asserts are dead |
| 4 | `sanitize` | out-of-bounds reads, use-after-free, signed overflow, bad shifts, misaligned loads, and leaks on Linux — ASan and UBSan over the `fast` label, plus `bench` |
| 5 | `perft` | INV-1 at the depths the fast label does not reach, `ctest -L slow` |

**Stage 4 also asserts INV-6 across builds** (DEC-167): the sanitizer build's
`bench` total must equal `build/src/chesso`'s. The two directories differ only
in instrumentation, so a difference is a read of uninitialised or out-of-bounds
memory that moved the search tree and that neither sanitizer reported — the
class they are blind to. It is the third reading of INV-6, and the only one that
compares two *builds* of one commit rather than two commits.

The sanitizer directory is `RelWithDebInfo`, not `Release` and not `Debug`:
`-O2 -g` is the "-O1 or higher" ASan's documentation asks for plus symbolised
reports, and it falls on the 600 s side of `CHESSO_TEST_TIMEOUT`, so a 2x
slowdown cannot hit the 60 s Release ceiling and read as a failure that is
really the clock. The `SANITIZER` option carries
`-fno-sanitize-recover=undefined` since S197, and that flag is load-bearing:
**UBSan recovers by default**, so without it a run prints
`runtime error: signed integer overflow: 2147483647 + 1`, carries on to the end
and exits **0**. Measured with a planted overflow on 2026-09-10: exit 0 without
the flag, exit 1 with it. `-fno-omit-frame-pointer` is there for the same
reason ASan's documentation asks for it — with `UBSAN_OPTIONS=print_stacktrace=1`
the same report came back with six named frames instead of a bare line.

A red `citations` stage means fix the citation in the step file, by symbol
(DEC-135) — never relax the check. A red anything else is a bug and the BUGS
rule applies: fix it before anything else starts.

**It presumes the automatic gate is green.** Stage 4 runs the whole `fast`
label under the sanitizer, so any red in that label fails the sanitize stage —
and the marker then names the sanitizers for something that is not theirs, after
the build has been paid for. The first real run failed exactly that way:
`test_clang_format_script` could not resolve its pinned major, and 514 s of
Debug and sanitizer building went by before it said so. On the machine
`.moltke.local.md` describes that means exporting DEC-146's override **in the
launching shell**, because a detached run inherits no interactive environment:

```bash
export CLANG_FORMAT_MAJOR=22
```

**The stages load every core, so never run this beside a match or an SPSA**
(PLAN and MACHINE rules). A run is longer than one tool call, so detach it and
poll the whole log — never `tail -f | grep`, which cannot exit on a log that has
stopped being written (WATCHERS rule, DEC-061):

```bash
nohup tools/gate_extra.sh > .tuning/gate_extra_$(date +%F).log 2>&1 &
pid=$!
bash -c '
  end=$(($(date +%s)+3600))
  until grep -qE "GATE-EXTRA-(DONE|FAILED)" "$1"; do
    kill -0 "$2" 2>/dev/null || exit 3
    [ "$(date +%s)" -ge "$end" ] && exit 124
    sleep 20
  done' _ .tuning/gate_extra_$(date +%F).log $pid
```

`pid=$!` is load-bearing and `pgrep -f gate_extra.sh` is not a substitute: the
pattern also matches the wrapper shell whose command line contains the launch
string, and that shell exits seconds later. Arming the watcher on it reports
"died with no marker" while the run is still in stage 3 — observed 2026-09-10,
and a watcher that cries wolf is worth as little as one that never fires.

The marker is one line on every exit path, including the refusals that come
before any stage — no `cmake`, no `ctest`, no `python3`, no way to count cores,
or a root that is not the chesso tree:

```
GATE-EXTRA-DONE <n> stages <total> s <outdir>
GATE-EXTRA-FAILED: <stage> <stage>... <outdir>
```

`tests/test_gate_extra_script.sh` is the guard on all of that, in the `fast`
label at 0.46 s: seven cases over a sandbox whose `PATH` holds stubs for
`cmake`, `ctest`, `python3` and `nproc`, so no stage does any real work. Both
`fastchess.sh` and `rating.sh` shipped with a path that printed no marker at
all and both were found after the fact (S167, S177); this one is checked from
the day the script exists. The twelve cuts every case was observed red under
are `adocs/data/S197_script_mutants.py`.

#### Coverage, on demand and never a stage

`adocs/testing_strategy.md` R13: coverage is a periodic report, not a gate. The
recipe is clang's, because the gcc route produces gcov output rather than the
format the 2026-09-04 test review recorded. On macOS prefix each `llvm-*` with
`xcrun`; on this workstation the tools are suffixed (`clang++-22`,
`llvm-cov-22`, `llvm-profdata-22`) — `.moltke.local.md` has the paths.

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
  -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"
cmake --build build-coverage -j12
LLVM_PROFILE_FILE="$PWD/build-coverage/prof/%m-%p.profraw" ctest --test-dir build-coverage -L fast
llvm-profdata merge -sparse build-coverage/prof/*.profraw -o build-coverage/chesso.profdata
objs=$(for b in build-coverage/tests/test_* build-coverage/tools/*; do [ -x "$b" ] && [ ! -d "$b" ] && printf -- '-object %s ' "$b"; done)
llvm-cov report -instr-profile=build-coverage/chesso.profdata build-coverage/src/chesso $objs \
  -ignore-filename-regex='tests/|tools/|doctest|json' > build-coverage/summary.txt
llvm-cov show -instr-profile=build-coverage/chesso.profdata build-coverage/src/chesso $objs \
  -show-branches=count -show-line-counts-or-regions -sources src/ > build-coverage/show.txt
```

`summary.txt` compares with
`adocs/data/2026-09-04_test_review/coverage_summary.txt`, whose
"89 functions have mismatched data" warning is expected across 25 binaries.
Lines whose count column is `0` in `show.txt` compare with
`coverage_unexecuted.txt` **by region and by eye**, not by diff: every commit
since `5cffb70` has shifted its line numbers, and the review took it with Apple
clang and `-march=native`. Coverage is reach, not speed, so it transfers across
the two compilers. `.gitignore` already covers `build-*`, so keep the
`.profraw` and `.profdata` files inside `build-coverage/` and nothing new needs
ignoring.

### The Debug self-play habit

DEC-141. A step that touches `make_move`, `unmake_move`, the generator or the
search self-plays the Debug binary before it completes, and its `done:` stamp
says so. Four rounds at 4+0.04 is minutes and it puts the asserts above under a
real clock, on positions no test FEN reaches:

```bash
cmake --build build-debug -j8
fastchess -engine cmd=build-debug/src/chesso name=debug-a \
          -engine cmd=build-debug/src/chesso name=debug-b \
          -openings file=books/UHO_Lichess_4852_v1.epd format=epd order=random \
          -each tc=4+0.04 option.Hash=16 option.Threads=1 \
          -rounds 4 -repeat -concurrency 8 -recover -check-mate-pvs \
          -pgnout file=/tmp/debug_selfplay.pgn \
          -log file=/tmp/debug_selfplay.log level=trace engine=true \
          2>&1 | tee /tmp/debug_selfplay.out
grep -c Assertion /tmp/debug_selfplay.log /tmp/debug_selfplay.out
grep -c disconnect /tmp/debug_selfplay.out
```

`level=trace engine=true` is load-bearing and was traced to observed output,
not assumed. `-log` defaults to WARN and does not capture engine stderr, which
is where an `assert` writes: a Debug binary carrying a planted accumulator
mutant aborted in every game, and `grep -c Assertion` on a default `-log` file
read **0** while the same run logged at `level=trace engine=true` read **2**.
`fastchess.sh` uses the default form, so this line is deliberately not that
one. `disconnect` is the cheaper signal and reaches the tee'd stdout either
way, but it names a dead engine and not the assertion that killed it.

Measured with a clean Debug build, 2026-09-09: 8 games in 19 s, 0 `Assertion`,
0 `disconnect`, 0 crashes.

Time forfeits are not the failure condition — a Debug binary is tens of times
slower and will forfeit at this control. `Assertion` and `disconnect` are.

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
read. It asserts eleven things: the script reaches the `fastchess` invocation; a
script that aborts before that point exits non-zero; with `REF` unset the banner
names `HEAD`; each side's date comes from its own commit — the expected date is
read with `git show -s --date=short --format=%cd`, the form `commit_date()` uses,
so the case does not depend on the wall clock and does not fail across midnight
(S193); a clean-tree A/A is
refused however the ref is spelled, and leaves no output directory behind;
`AA=1` reaches the match and says so; a run outside a git checkout still
prints a terminal marker; the seed the banner prints is the seed `fastchess` is
handed, and `SRAND` overrides it; the PGN is asked for node counts and time
left; and `ROUNDS` reaches `fastchess` as a round count with no `-sprt` beside
it. The last three read the stub's recorded argument vector rather than the
banner alone, because what a banner says and what the process was given are two
different claims (S198). The first two exist because `44877c4` left a renamed
variable behind and the harness stopped running for a commit without anything
noticing (S035, `2026-08-13_adversarial-F01`). The rest keep the default
reference from silently freezing to a sha again (S160,
`2026-08-22_adversarial-F02`) and pin the four defects S160's own first attempt
shipped, each reproduced before it was fixed — the date case is mutation-checked
rather than merely green, because every case before it ran with the reference at
`HEAD`, where printing `HEAD`'s date on the reference line is invisible.

`test_tuner_gradient` is the fit's own guard and covers two things nothing in
`tests/` could reach before S100: that the columns `load()` packs are the columns
`evaluate_position()` reads, and that `gradient()` is the derivative of the error
it descends. All of `dataset_t`, `load()`, `evaluate_position()`, `error_range()`
and `gradient()` lived in an anonymous namespace inside `tools/tuner.cpp`, so no
test could name them; S100 moved them to `tools/tuner_model.hpp` for exactly this
and the fit is unchanged — `tuner` over the same corpus at the same seed, thread
count and epoch budget prints identical epoch reports and emits all 827 constants
bit for bit. Its corpus is a fixture it writes into the temp directory from the
same curated FEN list `test_eval_model` runs over, so it needs no dataset.

The gradient check is Jon Dart's prescription: central differences against
`gradient()` for all 827 parameters, tolerance 1e-6 relative. `h = 0.01` is
measured and a comment in the file carries the sweep — the error follows the
O(h²) truncation law down to a cancellation floor near 0.005, and h = 0.5 fails
at 5.7e-4 on a mobility weight because mobility counts reach the tens. Two things
it deliberately establishes rather than assumes: that the lazy clamp binds on no
fixture row at the shipped constants, since `gradient()` ignores that clamp by
design, and that **every one of the 54 term parameters has a gradient to check**
at all. The second is what found a real gap — three passed pawn middlegame
weights had none, because every position reaching their buckets was a phase-0
endgame, and a feature-count test cannot see that.

`tools/plan_prose_check.py` carries four plan-hygiene checks. Three of the
four -- `--touches`, `--params` and `--citations` -- are in the fast suite as
`test_plan_touches`, `test_plan_params` and `test_plan_citation_freshness`;
`--prose` is not, and the paragraph after them says why:

```bash
tools/plan_prose_check.py             # all four, exits non-zero on any
tools/plan_prose_check.py --prose     # plan.md's tense only
tools/plan_prose_check.py --citations # pending step files' citations only
tools/plan_prose_check.py --touches   # pending step files' touches only
tools/plan_prose_check.py --params    # doc numbers against the compiled params
```

**`--prose`.** `plan.md`'s ordered list is maintained by the workflow checker
and the prose around it is not, so every completion can leave a sentence saying
a finished step is next. It resolves every id in the prose against `plan_done/`,
`plan_current/` and `plan_todo/`, then flags any **sentence** that names a
completed step alongside a pending claim. Sentence-wise because the prose is
hard-wrapped and an id and the claim about it usually sit on different lines. Run
it when a step completes: this has now gone stale three times
(`2026-08-13_plan_review-F05`, `2026-08-13_plan_review.2-F07`, and again the
moment S033 finished), and S062 is the third repair.

**`--citations`.** Every citation in `plan_todo/` and `plan_current/`,
resolved against the working tree. A citation is a path followed by a
backticked symbol or a double-quoted phrase -- `src/search.cpp` `negamax`,
`tests/test_search.cpp` "pruning does not hide a forced mate" -- and a path
followed by an ordinary word is prose and is not one. Three failure classes,
none a judgement:

* `LINE`, a `path:line` or `path:line-line` in a pending step file. The
  retired form (DEC-135). Found by a scan over the file's text rather than
  through the path token, because a path preceded by a slash -- S020 and S117
  each write `path:line/path:line` -- is invisible to the token and is exactly
  what the rule forbids.
* `MISSING`, the named file carries no such symbol or holds no such phrase. A
  symbol is searched word-bounded with the file's comments blanked, so a name
  that survives only in a comment does not answer; a phrase is matched against
  the doctest titles first and then, case-folded, as text anywhere in the file,
  which is how a citation into a comment lands.
* `BARE`, a `:line` continuation with no path of its own.

`LINE` gates a citation into `adocs/` too -- those rot fastest of all -- but a
phrase missing from a document is a note and does not fail the run, because
those documents are rewritten at every completion by design. Fenced code blocks
are not scanned: a fence holds a command, and the quote closing a string
literal in one reads as a phrase opening.

S138 is why the check exists: `2026-08-20_plan_review-F01` measured 70 of 147
citations stale, and eight pending steps that each add pruning or a reduction
told their implementer to extend the mate-safety gate at a line that had come
to rest inside an unrelated test. The symptom of extending the wrong mate test
is a strength regression, not a red test. **`BOUNDS`, `ANCHOR` and `DRIFT` are
retired** -- all three needed a line number, and DRIFT needed a git baseline
per file besides, which is what made this mode cost 6.3 s. Their evidence is in
`adocs/plan_done/S138_*`, `S144_*` and `S169_*`; the last run of them is banked
at `adocs/data/S187_citations_before.txt`, 52 DRIFT over 66 files.

**`BARE`, and why the check refuses to resolve one.** `(src/evaluation.cpp:951
mobility, :953 king safety)` — the second citation inherits its path from the
first, and nothing mechanical can tell which sentence it inherits from. S138
measured the attempt: guessing produced sixty impossible line numbers, because
S095 wrote `(transposition_table.cpp:96-103), and :449 already computes` where
`:449` means `src/search.cpp`. So the check states the path is missing and stops
there. **The rule is that a citation repeats its path** — `plan.md`'s "How this
file works" carries it — and this flag is its enforcement. S144 converted the
383 that existed across 19 pending files, reading each citing sentence and then
relocating the text the citation was written against to prove the path;
`adocs/data/S144_pathings.tsv` is that mapping and `adocs/data/S144_paths.py`
the generator. DEC-120. The path is still repeated per symbol in the form that
replaced it: `src/search_params.hpp` `LMR_BASE` and `src/search_params.hpp`
`LMR_DIVISOR`, because the checker reads only the first token after each path.

**What a citation is worth, and what proves it.** The check is existence, not
relevance: `src/search.cpp` `state` passes because `state` occurs in the file.
What proves a citation means what it says is the mapping the conversion was
made through, and never a green run — `adocs/data/S144_pathings.tsv`,
`adocs/data/S169_recitations.tsv`, `adocs/data/S187_symbols.tsv`, each with its
generator beside it. S187's records six citations that were wrong at the moment
they were written, a class no checker could see because its baseline was the
commit that wrote them: `src/chesso.cpp:674` was cited by three steps for where
the per-`go` `search_state_t` is built and had fallen into the book-move helper
at all three baselines. DEC-119.

**`--touches`.** A step whose `goal:` names a code symbol that no file its
`touches:` allows it to edit carries in code. `touches:` is the scope contract a
diff is checked against, so a step whose goal is to change a symbol and whose
`touches:` omits the file holding it cannot be completed without violating its
own scope. S039 is the measured case and `2026-08-20_plan_review-F06` is the
finding: its whole goal is to re-decide `LAZY_EVAL_MARGIN`, its `touches:` named
`src/evaluation.hpp`, and the value moved to `src/search_params.hpp` at S073 —
`src/evaluation.hpp` keeps only the comment saying what the number means. **A
mention inside a comment is not a landing site**, which is the whole of that
case, so files are comment-stripped before the match: C and C++ comments, and
`#` comments in `.py` and `.sh`. Four symbol classes and no more — an ALL_CAPS
name with an underscore, a call written with its parentheses, a `_t` type, and a
UCI option name matched against the names `src/search_params.hpp` declares
rather than against a CamelCase pattern. Two ungated note classes: a symbol that
is in code nowhere, and a `touches:` naming a directory that already holds files
of the symbol's kind, which cannot be resolved against the tree as it stands
because a directory may grow a file. The script's own file is excluded from the
corpus it searches, since its documentation names the symbols it is about.

**`--params`.** A document sentence that states a different number for a search
parameter than the engine compiles. `2026-08-21_adversarial-F02` found three:
`specs.md` gave the aspiration triple as 5 / 50 / 400 where the engine runs
2 / 21 / 437, and `MANUAL.md` said aspiration windows start at depth 5 eleven
lines below its own option table saying 2 — S085 retuned them and the prose did
not follow, in the document the reading order puts first. Running the check
found a fourth the audit had missed, `plan.md` saying `MaxQsearchDepth` **is**
8. Nothing else in the tree can see this class: `--prose` and `--citations`
compare prose to prose, `tests/test_uci_surface.cpp` builds its expected option
lines *from* `search_param_info()` and requires MANUAL.md only to **name** each
option, and `tests/test_search_params.cpp` holds the code against itself.

The values come from `src/search_params.hpp`'s `X(symbol, name, default, min,
max)` list, which is the single source `search_param_info()` is generated from
in both builds — `tests/test_search_params.cpp` is what keeps those two equal,
so reading the list is reading the function. A list that parses to nothing fails
the check rather than passing it vacuously.

Three rules, and **the third has a maintenance cost that is deliberately
visible**:

- `TABLE` — MANUAL.md's option table, `` | `Name` | default | min to max | `` .
  All three numbers compared.
- `NEAR` — a sentence that names the parameter and then gives a number in one of
  a few tight forms: `` `Name` `` is N, ships at N, = N, default N, N as
  shipped. Tight on purpose — "`LazyEvalMargin` at 0, 150 and 2000" is a sweep,
  not a claim about the default, and a looser rule flags it. **The same three
  forms run against the C++ symbol too** (S184), which is how a step file names
  a parameter: `` `ASPIRATION_DELTA` `` is 21, not `` `AspirationDelta` ``.
  There **both** backticks are mandatory, and that is measured rather than
  stylistic — optional, the first form reads S114's formula line
  `` `NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR` = 3 + depth/6 `` as a claim
  that the divisor is 3, taking the formula's own closing backtick as the
  symbol's.
- `PHRASE` — a sentence that never names its parameter. **Two of F02's three
  were of this kind**, so a name-adjacency scan alone would have missed them.
  These are keyed on the wording, in `PARAM_PHRASES`, and a rule matching
  nothing anywhere reports `STALE` instead of passing quietly.

**What it does not cover, stated rather than implied.** The check is a net with
a declared mesh, not a proof. A number stated about a parameter in prose that
neither names the parameter nor matches a phrase rule is not checked and cannot
be. Tense is not understood either, and that is load-bearing: `NEAR` keys on
"is", "ships at", "=", "default", so `MaxQsearchDepth` **was** 8 reads as
history and is left alone, which is how `plan.md`'s sentence was repaired
without deleting what it records. Rewriting a keyed sentence retires its rule —
`("MaxQsearchDepth", "quiescence is capped at (\d+) plies")` was F02's third
case and is retired in the source with the reason, because `eaad88b` rewrote
that sentence to name the parameter and `NEAR` covers it now.

**Default file set:** `adocs/specs.md`, `MANUAL.md`, `DEV_MANUAL.md`,
`adocs/plan.md`, **and every file in `adocs/plan_todo/` and
`adocs/plan_current/`** — S184 added the pending step files, because the class
lived there too and the four-document set could not see it: S082 and S115 stated
pre-S085 values, S127 stated one, and S184's own text stated a fourth. A step
file is covered the day it is written, since the set comes from the same
`pending_step_files()` `--citations` and `--touches` use. `adocs/plan_done/` is
excluded because it is history and records what was true when it was written.
Cost on the workstation with 68 pending files: **0.37 s** over the four
documents, **1.27 s** over the whole set.

`--prose` is **not** registered with ctest, and that is deliberate rather than
an omission: which tense a sentence should take is a judgement, and a rewrite
of `plan.md`'s prose would redden the suite for a step that had nothing to do
with it. Run it at step completion.

The other three are registered, and the reason each one is safe there is the
same: none of them reads a line number. `--touches` moves only when a step file
is written or a symbol changes file, so S141 put it in the fast suite where a
broken scope contract fails at once instead of waiting for someone to run the
tool. `--params` compares a number to a number and no source commit can shift
it; S150 registered it as `test_plan_params` and S184 widened its file set and
its cost to 1.27 s. `--citations` was the exception until S187: the reason it
was kept out was that "any source commit shifts lines under fifty step files at
once, so gating the suite on citation freshness would make a red suite the
normal state". In the symbol form a source commit shifts nothing it reads, and
what does turn it red is a renamed or deleted symbol with a pending step still
citing it -- the event the plan wants to hear about, one line to fix, in the
same commit as the rename. S187 registered it as
`test_plan_citation_freshness`, 0.45 s over the 66 pending files, median of
five on the workstation. DEC-159.

`test_clang_format_script` is the same shape over `clang-format.sh`, and it
exists because that script is the third command in the gate above. It asserts
five things in a throwaway git repository: a clean tree passes, a misformatted
tracked file is caught, a misformatted **untracked** file is caught, a
misformatted file under a gitignored path is skipped, and a file the index still
names but the worktree no longer has is not opened. The first two are
preconditions — without them a script that always fails, or a fixture that is
not actually misformatted, would satisfy the rest by accident. See the `Format`
section below for what the selection is (S054).

### Mate safety: five instruments, and none of them substitutes for another

Pruning that hides a mate is this engine's recurring bug — null move pruning hid
a mate in 2 by reducing to depth 0, late move reduction reduced the mating move
at the root, and reverse futility can return a static score at a node whose true
value is "mated". All three were caught by a mate test and none by a benchmark
or an SPRT. S145 rebuilt what that test is.

**1. The constructed set.** `adocs/data/S145_mate_set.tsv`, asserted by
`tests/test_engine.cpp`'s `engine: mate safety` suite in the fast suite.

```bash
~/.venv/chess/bin/python adocs/data/S145_mate_set.py verify    # both oracles, from scratch
~/.venv/chess/bin/python adocs/data/S145_mate_set.py generate  # rebuild the whole file
~/.venv/chess/bin/python adocs/data/S145_mate_set.py generate --only knight,backrank
~/.venv/chess/bin/python adocs/data/S145_mate_set.py emit-cpp  # the table the test holds
```

```bash
python3 adocs/data/S155_motif_census.py                        # what varies across the 48, and what does not
python3 adocs/data/S155_motif_census.py --moves                # and what the generator says can move there
```

Every row is a forced mate proved twice and by neither chesso: an exhaustive
AND/OR enumeration over `python-chess`, iterative-deepening in the *distance* so
the answer is exact and not an upper bound, and `stockfish` at a node limit.
Stockfish only ever proposes — a candidate it calls a mate in four and the
enumeration calls a mate in three is reported and the enumeration wins.

**Which is why a plain `generate` is not how a motif is added, and `--only` is.**
The proposer is version-bound and the proof is not. Re-running family `shift0`
here against stockfish `dev-20260803-762dd1da` returned 6 of its 8 tracked rows
and two different ones — same seed, same placements, same enumeration, a
different verdict at 150000 nodes, so a different slice of placements ever
reached the proof. Both slices are sound and the difference is not a defect;
what it means is that a full regeneration on a machine whose stockfish differs
silently *replaces* positions rather than adding to them. `--only` regenerates
the families it names and carries every other row through unchanged, and
`verify` is what re-proves the file end to end — it needs no agreement from
stockfish to do it (S168, 2026-09-01).

**Broad in mate distance, narrower in shape, and the shape is counted.** The
census above reported one motif over S145's 48 — two material signatures, each
the colour mirror of the other, and a lone queen as the mating force in 48 of
48. Over the 82 rows the file carries since 2026-09-01 it reports **five
material signatures and three mating forces: a queen in 48, a lone rook in 32,
two knights in 2**, with leads 760, 1160 and 1020. S168 added families and
replaced none.

**Two of S168's findings are worth more than its rows.** A king and two knights
does not give a knight mate by construction — a knight standing beside a wall
pawn unfreezes it and the freed pawn queens with check, which is how twelve of
the first fourteen knight positions came out as pawn mates. The generator
enforces the mating piece now (`mates_with`), and refuses any candidate whose
line offers a promotion; under that rule three of the four knight families
accept nothing and the fourth accepts two, so an all-quiet knight mate is rare
and the count says so. And the rook families are named for their force, not for
a geometry: the mate lands on the mated side's own back rank in 13 of 32.

What the gate still cannot catch is explicit: **no knight mate deeper than a
mate in two, no smothered mate, no king hunt, no open-line mate or line-opening
sacrifice, no promotion mate, and no position with a realistic material
balance.** The promotion clause is the generator's count and not an eyeball:
over the 82 roots and the 186 guarded defender nodes it emits 1981 legal moves,
of which **2 are pawn moves and 0 are promotions**. A pruning rule that hides a
mate in one of those shapes passes the suite (S155, S168, 2026-09-01).

Two properties make each row bite, and the test asserts both before searching
anything. The defender is **materially ahead**, so its static score is high
where its true value is lost — the shape no game position offers, which is why
the set is built. And the mating side's moves are **quiet** everywhere except
the mate itself, so the defender nodes are not in check and reverse futility is
actually allowed to fire at them. Mate distances two to five put those nodes at
plies 1, 3, 5 and 7.

Asserted through the engine's own `iterative_deepening_search`, not through
`search()` at one depth. That is the part the old three-position gate got wrong:
a cold-table call at exactly `2m - 1` cannot tell a mate that is *gone* from a
mate that is one iteration *late*, and postponement is what a removed guard
actually causes.

**What it asserts is not "every mate is found", because that is not true.** The
82 positions on the shipping build at depth `2m - 1 + 8`:

| distance | exact at the final iteration | delay |
|---|---|---|
| mate in 2 | 26 of 26 | 0 |
| mate in 3 | 12 of 24 | up to 8 |
| mate in 4 | 1 of 16 | up to 4 |
| mate in 5 | 0 of 16 | — |

Re-taken 2026-09-08 by S148 over the whole set --
`adocs/data/S148_rfp_ceiling_sweep.log`, the `RfpMaxDepth=15` row -- where the
figures above it were S154's over the 48 rows the set held before S168. With
reverse futility switched off entirely it is 70 of 82, not 82, so no setting
makes the strong claim true. **The gap between those two numbers has a price
now**: S148 swept the ceiling over all sixteen values and ran the elbow, 4,
against the shipping 15 at 8+0.08 -- 52 of 82 against 39, deep classes 7 and 1
against 1 and 0 -- and 4 lost at nElo -7.31 +/- 5.60 over 14808 games, H0
accepted against `{-5, 0}`. So the missing mates are what the pruning costs and
the trade was declined on the games, DEC-158. What is asserted is three things
instead: **no mate score for the side being mated and none closer than the
proved minimum** — both provably false claims, measured 0 and 0 over twelve
reverse-futility settings; **every mate in two at the first iteration that can
hold it**, which is the assertion that fences the tuner and goes red the moment
the ply floor drops below 2; and **a floor of 8 on the mate in three count**,
placed strictly between the shipping 9 and the 7 the removed guard produces. The
mate in four and five counts are printed by the test as a `MESSAGE` rather than
asserted, because a floor of zero asserts nothing. Observed red under a stated
mutation: `RFP_MIN_PLY` 3 → 1 fails the mate-in-two timing at iteration 5
against 3, and the floor at `REQUIRE( 7 >= 8 )`.

**The mate-in-two clause is stronger than a count and the difference is seven
positions.** It asks that the mate be found *and* that `first_exact` be
`2m - 1`. At `RfpMinPly` 1 the set reads 13 of 16 found but only **9 of 16 on
time**, so the sweep logs' "13 of 16" understates what goes red. S145's log
carries a header saying so; `adocs/data/S154_floor_margin_sweep.log` has the
measurement and the gate's own seven failures by name.

**The floor is re-derived whenever either end of it moves, never re-read**
(DEC-116). 7 was placed between 8 and 6 by S145 and stopped separating on
2026-08-23 when S165 lifted both ends to 9 and 7 — `7 >= 7` is green, so for
nine days the line could not fail for the reason it exists. What the claim
beside it asserts is now measured: over the seventeen commits that touched
`src/` since the floor was placed, and over nine table sizes from 1 MB to
256 MB, **0 positions change verdict**; one ply of the guard itself moves five.

```bash
python3 adocs/data/S154_floor_margin_sweep.py hash   # the noise floor: resize the table, change nothing else
python3 adocs/data/S154_floor_margin_sweep.py refs   # the realised drift, one build per commit
python3 adocs/data/S154_floor_margin_sweep.py floor  # the RfpMinPly table, bound relaxed in a worktree
python3 adocs/data/S154_floor_margin_sweep.py slack  # what the depth window costs and what it buys
python3 adocs/data/S154_floor_margin_sweep.py red    # rebuild the gate with the guard weakened; it must fail
```

`MATE_DEPTH_SLACK` is a cost budget and not a margin: at slack 12 the shipping
guard and the removed one both find 10 of 16, so the separation is gone
entirely, and the pass costs 4.1 s against 0.7 s. The mate-in-three count reads
lateness under a fixed window; the mate-in-two clause is the one that does not
depend on the window at all.

**2. The mined breadth set.** `adocs/data/S145_mined_set.tsv`, one position per
game from `.spsa/S085/games.pgn`, labelled by stockfish, **scored as a count
with a floor and never per position**. In the fast suite as `test_mate_breadth`
since S156; before that it was data nothing read
(`2026-08-21_adversarial-F08`).

```bash
./build/tests/test_mate_breadth                                  # the gate
~/.venv/chess/bin/python adocs/data/S145_mined_set.py mine  --games 6000
~/.venv/chess/bin/python adocs/data/S145_mined_set.py score --depth 10
```

The scoring rule is the whole point. A search is allowed to miss any particular
deep mate, so a suite that forbids it is a suite that gets switched off — two of
the seventeen engines S145 surveyed wrote exact mate-distance tests, watched
their own pruning break them, and disabled the tests rather than the pruning. A
count with a floor is a claim a search can keep.

318 positions, mate in 1 to 10. The gate reads the tracked TSV, drives every
position through the engine's own iterative deepening at depth 10, and asserts
two things: **at least 143 at the exact distance stockfish labelled, and zero
mate scores with the wrong sign**. The wrong-sign count is not a floor — a mate
claimed for the side being mated is a defect at any total.

Where 143 comes from, re-measured at S156's commit on the machine
`.moltke.local.md` describes, depth 10, `Hash` at its default 16:

| `RfpMinPly` | exact | right sign | wrong sign |
|---|---|---|---|
| 3, ships | 145 | 147 | 0 |
| 2 | 145 | 147 | 0 |
| 1 | 141 | 143 | 0 |
| 0 | 141 | 143 | 0 |

143 sits strictly between what ships and what the weakened guard gives, which
is the property that matters: it goes red when the guard goes and not when the
tree shifts under it. S145 placed the same floor on 146 against 139; the tree
has moved since — S142, S149 and S165 all alter play — and the gap has narrowed
from 7 to 4. Note what this set can and cannot separate: it sees the difference
between a floor of 1 and a floor of 2, and it cannot tell 2 from 3.

**Reproducing that table needs a patched tree, and the reason is S142.** It
narrowed `RfpMinPly`'s minimum to 2, so `setoption name RfpMinPly value 1` is
out of range, is ignored, and leaves the engine at its default — a sweep that
does not notice reads a flat null and is wrong. `adocs/data/S156_mined_floor_sweep.py`
builds a throwaway git worktree, relaxes the bound there, sweeps, then rebuilds
the gate with the weakened value as its compiled-in *default* and runs it to
observe the red. The tracked tree is never edited.

```bash
python3 adocs/data/S156_mined_floor_sweep.py            # sweep and observe red
python3 adocs/data/S156_mined_floor_sweep.py --depth 8  # the cheaper reading
```

**It costs 18.28 s in Release**, measured directly on that machine, against
28.50 s for the whole fast label before it — the largest single test in the
gate, and its own binary for that reason, so the ctest output says where the
time went. Depth is what it buys: the same 318 positions cost about 3 s at
depth 8 and separate *wider* there, 113 against 101. Depth 10 is what the floor
was placed at and what the owner chose to assert (2026-09-01), so that is what
ships. Debug is another matter entirely — see the timeout note in
`tests/CMakeLists.txt`.

**3. `-check-mate-pvs`, on every SPRT since S145.** `fastchess.sh` passes it. It
verifies that every `info` line carrying a mate score has a principal variation
of the right length ending in checkmate, over every position both engines
actually meet — tens of thousands a night against the couple of dozen a
constructed set can hold. It costs nothing and needs no position file.

It is a **consistency** check and not a mate-finding one: it is silent about a
mate the engine never reported, which is precisely the failure reverse futility
causes, and that is what instrument 1 is for. Two steps have driven its count
down and neither reached silence, so **read a count and not a silent log**:

| build | `Incomplete mating PV` lines | distinct searches | over |
|---|---|---|---|
| before S147 | 138 | -- | 3000 games |
| S147 | 10, later measured 12 | 3 | 3000 games each |
| S170 | **5** | **1** | 3000 games |
| S171 | **8** | **1** | 3000 games, workstation; `457e355` **0** from **0** beside it |

S147 (2026-09-02) removed the truncation of a line the search had just proved.
S170 the same day removed three more ways a **true** mate score lost its line --
a score inherited across searches, a proof this search made and overwrote, and
a score printed beside a line from another iteration. The five that were left
were read at the time as a wrong mate distance; **S171 measured that reading and
it is wrong (DEC-127)**. The distance is deliverable -- the 18-ply line exists
and the same replay at `Hash=256` prints it -- and what failed was the walk,
stalled five plies from the mate on one missing slot. `certified_mate_move()`
now fills such a hole from the children of the position whose entry is gone.

**The standing figure is 8 lines from 1 search in 3000 games**, taken on the
Linux workstation on 2026-09-07 -- `REF=457e355 ./fastchess.sh --fast`, 1 h 17 m
49 s, 0 time forfeits, Elo +3.24 +/- 8.91 between two INV-6 identical builds.
`457e355` produced **0** in the same match. Read a count against 8, and anything
materially above it has found something new.

The 8-against-0 split is which side met the position, not a difference between
the builds: the same case replayed against both binaries produces three
byte-identical short lines. **What the 8 are, and why they are not closed.**
One search whose `mate 6` is the position's true distance -- `stockfish` gives
`#+6` at depth 20 and 30 -- read off the table at depth 3 on 1224 nodes, too
shallow to build the 11 plies it owes, whose line continues in the table into a
chain proving `mate 8`; both lookups in `complete_mate_pv()` are keyed on the
distance still owed and both refuse. No completion walk can close it, S171's
`excludes` forbade every other route, and **S202** owns the class. DEC-150.

The census carries across machines where a timing does not: both engines play
in the same run, so the reference's count is measured beside the candidate's
and nothing is read from a figure taken elsewhere (DEC-049 untouched). The
earlier 5 is the MacBook's and stays attributed to it.

The reproducible half of the same property is `test_mate_pv`, in the fast label
since S147: every `info` line carrying a mate score over both S145 sets, 706 of
them at depth 8, asserted for length and for ending on checkmate. The two are
complementary and neither replaces the other — this one names the FEN and the
depth and runs in the completion gate, `-check-mate-pvs` sees the positions two
engines actually meet.

**When a count comes back above the standing figure**, the question is which
table entry produced the score, and no cold search can answer it — a mate
reported in a game comes out of entries earlier searches of that game wrote.
`build/tools/mate_trace` is what asks:

```
build/tools/mate_trace --fen '<root fen>' --moves '<the game, uci>' \
    --start 40 --stride 2 --warm 'nodes 1500000' --final 'depth 11' \
    --line '<the pv it reported>' --needed 18
```

**`--stride 2`, and it is not optional. DEC-151.** A game gives one engine only
the positions *it* moves from — it never searches the ones its opponent moved
from — so searching every ply builds a table no game produces. S171's case was
read back from its census log three times at stride 1 and came up clean each
time; it reproduces at stride 2 on the first attempt. `adocs/data/S170_replay.py`
takes the same stride, from the `stride` column of `adocs/data/S170_cases.tsv`
or from `--stride-override`. Rows A to E of that file are stride 1, the shape
they were found in.

It replays the game through the real UCI layer — one process, one table, the
shape `adocs/data/S170_replay.py` uses — and then walks the reported line over a
board of its own, printing for each position the entry behind it: the score as
stored, the score a reader at that distance from the root sees, the mate
distance, the depth, the node type and the generation that wrote it. Past the
line it follows the table's own best move, which is the walk
`complete_mate_pv()` makes, so it stops where that walk stops; at a stall it
prints every legal move and what the table holds one ply on, which is what says
whether the hole is one a lookahead can fill. Nothing it does searches a node or
writes an entry. `fastchess` prints the `Position;` and `Moves;` lines beside
each warning, which is where its `--fen` and `--moves` come from. S171.

**4. The defender-side set.** `adocs/data/S165_defender_set.tsv`, swept by
`adocs/data/S165_nmp_defender_sweep.py`. Instruments 1 and 2 both ask "does the
engine find this mate", with the engine as the attacker. A guard on a mate
*bound* is not reachable from there: `beta <= -MATE_MIN` happens where the
engine is the side **being** mated, and that is what S165 needed and could not
get from either set.

```bash
cmake --build build-tune -j12
~/.venv/chess/bin/python adocs/data/S165_nmp_defender_sweep.py generate   # re-prove the set
~/.venv/chess/bin/python adocs/data/S165_nmp_defender_sweep.py sweep build-tune/src/chesso
```

The positions are not new — they are instrument 1's own `defender_nodes`
column, the mating line at plies 1, 3, 5, 7. What is new is that each one's
distance is **proved here rather than computed from the root's**, and the reason
is a trap worth naming: `representative_line()` walks the attacker's first quiet
mating move and the defender's **first legal reply**, not the reply that holds
out longest, so `root_distance - (ply + 1) / 2` is wrong on **3 of the 104
nodes**. Each distance is the smallest k with instrument 1's own
`_and_mate(node, 2k)` true, iterative in k so every shorter distance is refuted,
and stockfish at 4000000 nodes agrees on **104 of 104**.

**A mate in k against the side to move is 2k plies, not `2k - 1`** — k defender
moves and k attacker moves. `2k - 1` is the attacker-side formula, and using it
here reads every node as one mate further away than it is; the first version of
this sweep did, and reported 28 spurious `short` results before the arithmetic
was checked against the enumeration.

Reported per proved distance in instrument 1's sweep's shape. On the shipping
build, 104 nodes at depth `2k + 8`: **82 of 104 exact**, m1 50/50 at delay 0,
m2 29/31, m3 3/15, m4 0/8, `short 0` and `sign 0`. Without S165's guard the
same sweep reads 80, m2 28/31 and m3 2/15 — so it separates two mates, which is
what a 104-node set at this strength can resolve and not more.

**Measuring the guard itself.** `adocs/data/S145_rfp_sweep.py` sweeps
`RfpMinPly` and `RfpMaxDepth` against both sets on the tune build, one axis at a
time with the other held at its shipping value, because S145 measured that the
two substitute for each other and a sweep that moves both attributes nothing.

```bash
cmake --build build-tune -j12
~/.venv/chess/bin/python adocs/data/S145_rfp_sweep.py both --mined
```

It reports `exact`, `delay`, `short` and `sign` per setting **and per mate
distance**, because the answer turned out to be almost entirely a function of
the distance. `delay` — iterations between `2m - 1` and the first iteration that
reports the mate — is the reading a fixed-depth call cannot produce. `short` and
`sign` are the two columns that are defects at any count rather than strength
readings, and both are 0 everywhere so far. The output is kept at
`adocs/data/S145_rfp_sweep.log`.

**The floor sweep now stops at 2**, because S142 made 2 `RfpMinPly`'s declared
minimum on this sweep's own evidence and the tune build refuses anything below
it — `info string refused [RfpMinPly] value 1, outside [2, 63]`. The script
prints the settings it dropped and why rather than ending in the python-chess
`EngineError` that a refused option raises; `adocs/data/S145_rfp_sweep.log` is
the last reading that covered 0 and 1, and getting below 2 again means relaxing
the bound in `src/search_params.hpp` and rebuilding.
`adocs/data/S156_mined_floor_sweep.py` is that relaxation automated for the
mined set — a throwaway git worktree, patched and built there, so the tracked
tree is never edited.

Two traps in running it, both hit once. **A fresh engine process per setting**,
never `configure` on a live one: python-chess sends `setoption` for what it is
given and leaves the rest alone, so a loop that reconfigures one option at a
time measures the union of every setting it has been through — that produced a
table where turning null move pruning off appeared to fix a missed mate and it
was reverse futility from the previous row. And **a node-limited stockfish is
reproducible only inside one identical call sequence**, which is why the
constructed set's `verify` mode runs one stockfish process per position at ten
times the filter's budget, and why stockfish there corroborates rather than
decides: it reports no mate on 1 of the 48 and a longer mate on another, both
re-proved by enumeration, because a frozen defending army is a position class
its network scores badly wrong.

**5. The defender fixture, in the fast suite.** The same 104 nodes as
instrument 4, read by `tests/test_search.cpp`'s
`search: pruning and reduction guards` suite through `CHESSO_SOURCE_DIR`, and
that is the whole of the difference: instrument 4 is a sweep you run, this one
is a gate that runs itself. Until S191 the file was read by nothing in `tests/`
at all.

```bash
cd tests && ../build/tests/test_search -tc='*defender*'      # the one case
cd tests && ../build/tests/test_search -ts='search: pruning and reduction guards'
```

**What it asserts.** Every row is loaded, its ply and proved distance turned
into the mated score `s = -(MATE_MAX - ply - 2 * mated_in)` and beta into
`s + 1`, which is inside the mate band by construction and asserted to be; the
node is then driven at depth 5 through `negamax_probed()` and the probe read.
**No row may make a null move, zero tolerance**, and the failure message names
the FENs that did. Each row also asserts the three conditions that would
otherwise stop the block on the wrong guard -- the node is not in check, its
`game_phase` is above zero, its ply is above zero -- so a row that stops passing
for the right reason cannot go on passing for a wrong one.

**What it cannot see.** It drives one node with a cold table and says nothing
whatever about mate *finding*: whether the engine ever reaches these positions,
whether it reports the mate, and how late. That is instrument 1's question and
instruments 2 and 4 read it at breadth. This one holds a single guard --
`beta > -MATE_MIN` on the null-move block, S165's -- against removal, which is
the thing all four of the others together caught only through
`test_mate_carry`'s per-game floor (2026-09-04_test_review-F02).

**The row count 104 is a golden** (DEC-142), named as one at its assertion and
re-derived with `grep -vc '^#' adocs/data/S165_defender_set.tsv` -- which counts
the header row too -- or by regenerating the file with the sweep script's
`generate` mode.

The suite around it holds the other twelve guards the same way, one case each
and none of them reading the defender set: the null-move block's `!is_in_check`,
`game_phase > 0`, `prev_move != 0` and both band edges, the artefact rule that
returns beta rather than a mate a pass produced, reverse futility's
`!is_in_check`, `!is_pv`, `depth <= RfpMaxDepth` and negative band edge, and the
reduction block's `!is_capture` and `!is_check_move` plus the re-search a
reduced move that beat alpha is owed. Each was observed red under a guard-removal
mutant before it was called done; the mutants are
`adocs/data/2026-09-04_test_review/mutants.py` and `adocs/data/S191_mutants.py`.

### Stress the stale-timer disarm

`stop_search_after_ms()` arms a detached thread that raises the global stop flag
when its sleep ends, unless the search session has moved on. A move normally
ends at its soft limit, so the hard timer is still sleeping when the next `go`
arrives: stale timers are one per move of every game, not an edge case. If the
timer's session check and its store are not one decision, a timer preempted
between them kills a search it was never armed for.

```bash
taskset -c 0 tools/timer_race_stress.py ./build/src/chesso --seconds 900
```

Each iteration sends `position fen`, then `go movetime 1` — which arms a timer
and abandons it within a millisecond — then `go depth 6`, which carries no timer
of its own and must reach depth 6. A `go depth` search that reports less was
stopped by the previous iteration's timer, and the two commands land about a
millisecond apart, which is where that timer expires. `taskset -c 0` is the
point: affinity is inherited by the engine, so the timer thread and the UCI
thread share one core. 571 to 589 iterations a second measured over two
15-minute runs. Last line is `TIMER-RACE-CLEAN`, `TIMER-RACE-HIT` or
`TIMER-RACE-FAILED`, exit 0, 1 or 2. All three are terminal, so a watcher armed
on the alternation exits on a dead engine instead of waiting out its ceiling
(§12).

**The unwidened race did not fire in 513787 iterations of it against the
pre-fix build, and that is S163's recorded outcome rather than a claim about the
harness** — 15 minutes on one pinned core, 0 hits, against 530104 iterations and
0 hits on the fixed build in the same 15 minutes. At 500 µs the hit rate was
0.37 % of iterations and the real window is roughly four orders of magnitude
narrower, so 0 over half a million iterations is the expected reading on both
builds and separates nothing.

What the harness does catch is the window made observable: a scratch 500 µs
`sleep_for` between the session check and the store — widening a window that already existed, not inventing one
— produced **46 hits in 12328 iterations** before the fix and **0 in 23651**
after it, same harness, same widening. That pair is the evidence the fix rests
on; the harness is kept as the net for the narrow case. The widening is four
lines in `stop_search_after_ms()`, at the same source position on both builds,
and it is not committed:

```cpp
    if (session_id == session) {
      std::this_thread::sleep_for(std::chrono::microseconds(500));
      stop_search_signal = true;
    }
```

The widened hits report depth 5 and depth 4 rather than depth 1, because a store
500 µs late lands inside the following search instead of before its first poll.
In production the store lands at the session bump, which is where the audit's
depth-1 instant reply comes from. The widening moves *when* the store lands, not
whether.

### Mutation check, `tools/mutation_check.py`

What the fast suite is worth is not how much of the engine it executes but how
much of it breaking the engine gets caught. This tool measures that: it applies
one hand-written bug at a time to a linked worktree, rebuilds, runs the fast
label and the bench, records what went red, and reverts. The mutants live in
`tools/mutants/`, one file per area, and are data — `m(...)` calls that the tool
`exec`s with `m` bound, so a mutant file carries no driver of its own.

```bash
git worktree add --detach .ref-builds/mut HEAD
git -C .ref-builds/mut submodule update --init tests/doctest tests/json
cmake -S .ref-builds/mut -B .ref-builds/mut/build \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

export CLANG_FORMAT_MAJOR=22                       # this machine, DEC-146
python3 tools/mutation_check.py tools/mutants .ref-builds/mut            # all
python3 tools/mutation_check.py tools/mutants .ref-builds/mut --only M26 # one
```

**The submodule line is not optional and `fastchess.sh`'s recipe omits it**,
because that script builds `src/chesso` and nothing else. This tool builds the
tests, and `tests/doctest` and `tests/json` are submodules: a fresh worktree
without them fails at `fatal error: doctest.h: No such file or directory` and
the tool refuses the run with "the unmutated worktree does not build". The
`CLANG_FORMAT_MAJOR` export is the same override the gate needs here: without
it `test_clang_format_script` is red on the unmutated tree and the run refuses
before the first mutant, which is the guard working — a suite already red
cannot say whether it saw the mutant.

**Release only.** The root `CMakeLists.txt` adds `-Wall -Wextra -Werror`
everywhere and turns the unused-* warnings off in `Debug` alone, so a mutant
that deletes the last use of a local does not compile in the build the gate
runs. That is a fact about the mutant, and a Debug build would hide it. The two
mutants in that class consume the symbol they orphan — `M08` inserts
`(void)is_check_move;` and `M12` inserts `(void)depth;` — and any new mutant
that orphans a symbol needs the same.

**Verdicts.** The suite is the first oracle and the bench signature is the
second: OpenBench requires a `bench` node count that is the same every time and
Stockfish defines a functional change as one that leads to a different search
tree, so a signature that moves is a changed tree.

| suite | bench signature | `expected` | verdict |
|---|---|---|---|
| red | either | either | `killed` |
| green | moved | either | `survived` — a proved gap, no declaration can rescue it |
| green | same | `equivalent` | `equivalent` — not scored |
| green | same | `killed` | `survived` — the bench-blind class |

A mutant that does not compile is `stillborn`. A build that hits its ceiling, or
a run whose only failing rows are `(Timeout)`, is `unmeasured` -- not a kill,
and re-run with `--only` when the machine is quiet. **A `(Timeout)` beside a
`(Failed)` is a kill**, because the ceiling is then not the whole evidence and
the hang may be the mutant's own: `M22` leaves `test_uci_surface`, 9.26 s
unmutated, still running at 60 s while four other binaries fail on assertions,
`M31` does the same beside two, and a quiet machine reproduces both exactly.
Those are the only two ceilings in a 40-mutant pass and the rule that called
them unmeasured cost both (DEC-165). The score is killed over total less
equivalent, stillborn and unmeasured. The run exits 0 only when every verdict
equals its `expected`, which is what a completing step reads.

**`expected="equivalent"` is a declaration, never an inference.** Equivalent
mutants are undecidable in general, so a person argues it in the mutant file and
the tool believes them. A still bench never argues equivalence on its own: 12 of
the 33 mutants of the 2026-09-04 review left the depth-9 counts and best moves
unchanged, and the suite caught ten of those anyway. Of the remaining two, one
is the declared equivalent and the other is the survivor S193 was written to
kill -- neither was ever equivalent because its bench held still.

**A new search rule ships with a mutant its test kills.** DEC-141 clause 2: add
the mutant to the matching `tools/mutants/*.py` with the next free id and
`origin="S<id>"`, commit the rule and its test, refresh the worktree to that
commit, run `--only M<nn>`, and paste the row — id, verdict, the killing binary
and its first assertion — into the step stamp. A `survived` row means the guard
test does not bite: fix the test, not the mutant. The tool prints every failing
binary rather than a count, because a kill by one golden alone is a weak one
(DEC-142) and only the list shows it.

**Cost, and it is in no gate.** **40 mutants in 3948 s — 66 minutes — on the
workstation, 2026-09-10**: about 85 s a row, being a ccache rebuild, the bench
and one serial run of the fast label. Three rows run long because the mutant
makes the engine search more, `M22` worst at 442 s. One row on its own is
**168 s** measured, the baseline included — `--only M26` — which is what a
completing step pays under DEC-141 clause 2. Nothing runs it automatically; run
it after a change to `tests/` that adds or removes coverage, and not beside a
match — the coordinator holds the machine, and a busy one turns a passing test
into an `unmeasured` row. Detach it and watch the marker, which is
`MUTATION-RUN-DONE` or `MUTATION-RUN-FAILED` on every exit path:

```bash
nohup python3 tools/mutation_check.py tools/mutants .ref-builds/mut \
      > /tmp/mutation.log 2>&1 &
```

Per-mutant build and ctest logs, and a `results.tsv` of the whole table, land in
`.ref-builds/mut/build/mutation/`, which `.gitignore` covers.

`tests/test_mutation_check.py` is the tool's own gate, in the fast suite: a
throwaway git repository, a four-line `src/x.cpp`, and `cmake`, `ctest` and the
engine as stubs on PATH. Twenty-four cases in about 1.6 s, and it is what
catches an anchor counted wrong, a revert that leaves a mutant behind, a verdict
on the wrong branch or a missing marker. Each was observed red under a cut to
the guard it names before it was kept -- the tool held to its own rule
(DEC-141). Three of them come from S196's own fast check and are worth knowing
before you edit the loop: a mutant whose pairs interact can fail while being
applied, with the first pair already on disk, so `apply_mutant` sits inside the
`try` that reverts; `SIGTERM` is caught and raised, because Python's default
disposition exits without unwinding and a detached run is stopped with `kill`;
and ctest's trailing summary belongs to no binary, so the chunk that collects a
failing test's output ends at it rather than at end of file.

## Format

```bash
./clang-format.sh           # format in place
./clang-format.sh --check   # dry run, non-zero if anything is unformatted
```

Pins clang-format major version 23 and refuses to run on anything else,
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

### The node signature

One number for the whole search tree, over eight fixed positions at a fixed
depth. It is what every commit touching `src/` carries and what `tools/gate.sh`
checks; `MANUAL.md` has the command's contract and its caveats.

```bash
chesso bench                          # the argv form OpenBench runs
printf 'bench\nquit\n' | chesso      # the same number over stdin
chesso bench 9                        # a shallower run; NOT the signature
```

**At `S189`, on the workstation: `24880255`. At `S203`: `26851183`.** Quote it
with its commit, the way every other number on this page is quoted — it moves
with every functional change by design, which is the whole point of it. S203 is
the example worth remembering: it redrew the Zobrist keys, which changes which
positions share a table slot and therefore the whole tree, without changing a
single rule of the search.

`BENCH_DEPTH` is 14, chosen as the largest depth whose mean is at most five
seconds here. The sweep, 2026-09-08, idle, on mains, governor `performance`,
`hyperfine -w 1 -r 5`:

| depth | mean | nodes |
|---|---|---|
| 9 | 0.208 s ± 0.003 | 1587743 |
| 10 | 0.344 s ± 0.007 | 2580811 |
| 11 | 0.583 s ± 0.007 | 4437125 |
| 12 | 0.997 s ± 0.008 | 7408328 |
| 13 | 1.754 s ± 0.006 | 13064004 |
| **14** | **3.445 s ± 0.028** | **24880255** |

The search is not perft. perft measures generate, make and unmake; a search also
evaluates, orders moves and probes the transposition table, so a change can move
one number and not the other:

```bash
tools/search_bench.py ./build/src/chesso 9
```

Three positions: midgame, kiwipete, tactical. Depth 9 is **1225840 nodes and
about 0.19 s** for all three on this machine at S021 — raise the depth when a
difference is small, and do not budget from the "about ten seconds per binary"
this line used to claim, which was measured on the pre-DEC-049 machine and on an
engine with less pruning in it.

**Every timing on this page taken before S104 is on a binary with no architecture
flag and does not transfer**, the same way nothing before DEC-049 transfers across
the machine move. `build/` is `-march=native` since 2026-08-19 and the same three
positions at depth 9 read **0.149 s before the flag and 0.143 s after** — which is
also a demonstration of why three positions do not time anything: the effect is
+18 % and this reads 4 %. Node counts are unaffected and stay comparable.

**That figure moves with every change to the search or to the evaluation, so
quote it with the commit it was taken at.** It read 3136397 before S065 refitted
the constants, 3752725 at `c56ab41` after that fit, 1422053 once S033 added
reverse futility pruning at margin 100, 1216123 once S068 cut that margin to
75 — 14.5 % of the tree, for the +5 Elo or so of DEC-063's pooled estimate —
and 1225840 once S021 added aspiration windows, **918962 once S094 gave
quiescence a transposition probe** -- 164123 / 670488 / 84351 against 174078 /
927856 / 92855, same best moves. A count from one of those is
not a baseline for another.

**S021 is the one that went the wrong way, and it is the reason three
positions do not price a search change.** Aspiration windows cost 0.8 % *more*
nodes here at depth 9 and 7.5 % fewer over the 300 positions of
`adocs/data/S021_aspiration_sweep.tsv`, and the SPRT that decided the step
accepted H1. At depth 12 the same three positions read +10.2 %, -6.8 % and
+63.6 %. Use this to check that a change meant to be neutral *is* neutral, for
which three positions are enough; do not read a saving off it.

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

Today's evaluation is **53.90 ns per call, 18.6 M calls per second**, measured
2026-08-19 by `./build/tests/bench_eval` on the DEC-049 machine, 4000000 calls a
sweep over 7 sweeps, spread 3.9 %, printed resolution 0.2 %.

**It read 83.35 ns / 12.0 M earlier the same day and the difference is S104, not
the evaluation.** That figure is on a binary with no architecture flag, where
`std::popcount` was a software SWAR popcount; `count_bits` runs once per piece per
evaluation in the mobility and king-safety loops and in `game_phase`, so a third
of the call went on it. The function did not change — `-march=native` did.

**The figure here read 1.31 ns / 762 M until 2026-08-19 and does not reproduce.**
It came from DEC-036, dated 2026-08-11 — two days before DEC-049 moved this
project off Apple silicon — and the evaluation has since gained S027's terms and
S065's refit of 827 constants. Both the machine and the function changed, so the
old absolute number is not comparable and is not restated as though it were.
**DEC-036's argument is unaffected**: it rests on the *ratio* between recomputed
mobility at 15.93 ns and the evaluation beside it, 12.2 times, both measured in
the same run on the same machine, and on the 33 % of nodes per second that ratio
cost in a real search at depth 12 — worse than the 25 % S014 removed.

The lesson is the one this file already gives about `-n`: **an absolute timing is
a property of a machine and a build, and only a ratio measured in one run
travels.**

A/B timing with statistics, always interleaved so machine drift cancels:

```bash
hyperfine --warmup 1 --runs 10 './bench_before -r 2' './bench_after -r 2'
```

Check the machine is idle first: `ps aux | sort -rnk3 | head`.

## Size the lazy evaluation margin

```bash
build/tools/eval_spread --data .tuning/selfplay_v2.tsv            # 11.0 M positions, 18.6 s
build/tools/eval_spread --data .tuning/selfplay_v2.tsv --limit 200000
```

Reads `tools/datagen`'s `fen result score phase` and uses only the FEN. Prints
the distribution of the **unclamped** expensive-stage correction in absolute
centipawns — p50, p90, p95, p99, p99.9, max — for mobility, for king safety and
for the two combined, then how many positions exceed each of 150, 200, 250, 300
and 400, then the worst position per term.

Three terms and not one because a margin that binds on the sum and a margin that
binds on a single term are different problems. `--limit N` stops after N
positions; the corpus is 11003693 lines and the first N are consecutive plies of
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
./fastchess.sh                  # vs HEAD, gainer SPRT, elo0=0 elo1=5, hours
./fastchess.sh --fast           # vs HEAD, looser bounds, few hundred games
./fastchess.sh --nonreg         # vs HEAD, non-regression, elo0=-5 elo1=0
REF=HEAD~1 ./fastchess.sh       # measure against some other commit instead
OUT=<dir> ./fastchess.sh        # where the pgn and log land
AA=1 ./fastchess.sh             # A/A: identical builds, calibrates the harness
SRAND=<n> ./fastchess.sh        # replay another run's openings, seed off its banner
ROUNDS=<n> AA=1 ./fastchess.sh  # fixed rounds, no SPRT: never a verdict
```

The reference is built from a git ref into a worktree under `.ref-builds/`, so
a result is always attributable to a commit range, and the candidate binary is
snapshotted before the first game. Both exist because an SPRT once reported
+301 Elo and meant nothing — DEC-020.

**`REF` defaults to `HEAD`, so the three bare forms measure the uncommitted
diff.** The banner prints each side's short sha and its commit date before the
first game:

```
candidate  eaad88b  2026-08-22  + uncommitted changes
reference  eaad88b  2026-08-22
```

Both shas are the same one because that is what the default means: the diff on
disk against the commit it sits on. A passed `REF` puts that commit and its own
date on the reference line — `reference  7b4d9a4  2026-08-08` beside a candidate
dated `2026-08-22` is the two-week gap the date column exists to show.

It used to default to the fixed `7b4d9a4`, the 2026-08-08 pre-achesso baseline,
set that day and never moved — several hundred Elo stale by the time it was
found (S028's fit alone measured +188.74 since it). A bare `--fast` run reached
H1 in minutes whatever the change under test was, which is DEC-020 armed in the
default of the per-change instrument: S160, closing
`2026-08-22_adversarial-F02`. Two consequences worth knowing:

- **Whenever the reference resolves to `HEAD` and nothing is uncommitted** both
  sides are the same build, and the run is refused rather than played. An SPRT
  between identical engines does not return zero; it random-walks until a bound
  is crossed by luck, which at `--fast` alpha 0.10 is one run in ten reporting a
  gain that does not exist. The test is on that state, not on whether `REF` was
  passed: `REF=$(git rev-parse HEAD)`, `REF=master` and a saved runner script
  that pins the sha it was written at all reach it.
- `AA=1` is how that match is asked for on purpose, and the run prints
  `A/A CALIBRATION` before the banner. It measures the harness — pairing,
  adjudication, forfeit rate, the variance floor concurrency leaves — and never
  the engine.

**Fetch the book first, once per machine.** `fastchess.sh` plays an unbalanced
book that is 175 MB and therefore not committed:

```bash
./books/fetch_book.sh           # UHO_Lichess_4852_v1.epd, ~43 MB zipped
./books/fetch_book.sh --list    # what is pinned
```

It checks the zip's sha256 and the unpacked file's, refuses on either, and is a
no-op when the file is already there and matches. The source is
`official-stockfish/books`, which is **CC0-1.0**; Stefan Pohl's own UHO pages
state no usage licence, so nothing is taken from there. The committed
`8moves_v3.pgn` is pinned in the same table and comes from the same CC0
repository -- `./books/fetch_book.sh 8moves_v3.pgn` verifies the tracked copy
against upstream instead of downloading it. The script fails with
`FETCH-BOOK-FAILED:` and `fastchess.sh` refuses to start without the file.

### What the PGN carries, and how a run's openings are replayed

**The seed is in the banner and nowhere else fastchess put it.** `-openings
... order=random` has always been passed, so the opening sequence — and with
`-repeat` the pairing that follows from it — came from a seed the harness never
recorded. Measured on `alpha 1.8.1 20260720-daa3ea2`: fastchess echoes its seed
on no stream, in no log and in no PGN header. `fastchess.sh` now derives one
from the run stamp, prints it, and passes it:

```
book       UHO_Lichess_4852_v1.epd
seed       20260908021500
```

`SRAND=<n>` overrides it, which is how a sequence is played again; a value that
is not an unsigned integer is refused by name before the output directory is
made. The parser is 64-bit — a 20-digit value is refused with `stoull: out of
range`, and `seed + 2^32` gives a different sequence, so nothing is truncated.

**A replay reproduces the openings, not the games.** Search under a clock is
not deterministic, and the seed-to-sequence mapping is fastchess's own, so the
same openings come back only with the same book (pinned by
`books/fetch_book.sh`), the same fastchess build and the same `-openings`
options. Each PGN carries the seed in its own header —
`[Event "chesso full 20260908_021500 srand=20260908021500"]` — so a file
separated from its log still says what it played.

**Every move carries its node count and the clock left after it.** `-pgnout`
is passed `nodes=true timeleft=true`, both of which default to false, so a
census reads them without a log at `trace`:

```
3... Nc6 {+1.00/3 0.000s, tl=0.000s, n=2437}
```

The existing score, depth and time come first; `tl=` and `n=` follow.
`tools/error_profile.py` takes the comment whole and anchors at its start, so
it is unaffected.

**`ROUNDS=<n>` plays a fixed number of rounds and runs no SPRT** — the mode a
calibration and a drift reading need, and never a verdict (MEASUREMENT rule).
An SPRT stops where its likelihood ratio happens to cross a bound, so variance
read over an A/A that stopped that way is read over a denominator the answer
chose; fixed rounds give a known one and an error bar of `v * sqrt(2/(n-1))`,
about 6.3 % of `v` at 500 pairs. The banner prints no bounds line there:

```
bounds     none -- fixed 500 rounds, a calibration or drift reading, NOT a verdict
```

### Not every change goes to a match

**A change that leaves the node count identical is not sent to an SPRT.**
DEC-083. An SPRT at short time control cannot see a speed-up below about
0.24 %, and it costs a night to say so. Instead:

1. `tools/search_bench.py` on both builds — identical node counts and identical
   best moves discharge INV-6 and prove the change is behaviour-neutral.
2. `hyperfine` or `bench_movegen` for the speed, interleaved, with the noise
   floor read off the run rather than assumed.
3. Convert if a strength number is wanted, and **name it as a conversion**:
   the published figure is 1.43 Elo per percent of nps at long time control and
   2.10 at short. It is not a verdict and is never written as one.

An SPRT is for a change that alters play. That is what the rest of this section
is about.

### The regime, and why each part of it is what it is

| | `fastchess.sh` | why |
|---|---|---|
| time control | `8+0.08` | what the engines this plan reads figures from test at, and about 29 s a game against 52 s at the old `10+0.2` — DEC-083 |
| hash | `16` MB | matches table **pressure**, not table size: at the rating list's 2'+1" a game writes ~660 M nodes against 5.6–11 M entries, 60–120 overwrites per entry, and 16 MB at 8+0.08 reproduces that ratio where 128 MB undershoots it about eightfold — DEC-088 |
| book | `UHO_Lichess_4852_v1.epd` | unbalanced. A balanced book draws about 91 % between engines of equal strength and a drawn pair carries no signal, so it spends the night to say less — DEC-083 |
| threads | `1` | the target is the CCRL Blitz **1CPU** scale — DEC-089 |
| concurrency | every core the machine reports: `sysctl -n hw.physicalcpu`, else `nproc` | DEC-048, DEC-050 |
| pairing | `-repeat` | paired colours. This is what makes an unbalanced book sound, and it is never dropped |
| adjudication | `-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400` | unchanged by S105, deliberately: the book was the one variable moved |

`rating.sh` deliberately does **not** match this. It runs `Hash=128` because
its job is the rating list's absolute regime, and the list runs 128 to 256
(DEC-089). The two scripts match different invariants and S105 is where they
parted; `rating.sh`'s own time control is still `10+0.2` and is S128's
question.

**Concurrency is every core the machine reports**, whatever kind it is — 12 on
this machine, which is 6 physical cores with SMT (DEC-050); efficiency cores on
Apple silicon (DEC-048, superseding DEC-042). The scripts read it from `sysctl
-n hw.physicalcpu` where that exists and from `nproc` otherwise -- `fastchess.sh`
since S167, `rating.sh` and `build_release.sh` since S177 -- so they need no
edit per machine. `CONCURRENCY=N` overrides it; do not lower it
to be polite, since nothing else should be running during a match, and a run
that does lower it records why. `CONCURRENCY=6` is the way to one game per
physical core here if a result has to be as clean as this machine can make it.

Everything else that parallelises follows the same policy: `-j12` for a build,
and `datagen` and `tuner` default to every hardware thread rather than to a
number written into the source.

### Which bounds

The hypothesis pair sets the cost of a verdict as much as the hardware does,
and this is measured, not argued. **S068 measured one constant twice with the
same binaries**: `elo0=0 elo1=5` ran 6 h 36 m over 9036 games and returned
nothing, `elo0=-5 elo1=5` returned H1 in 1 h 41 m over 2312 — both at 1371
games/h. DEC-063.

| flag | bounds | for |
|---|---|---|
| *(none)* | `elo0=0 elo1=5`, α=β=0.05 | a change claimed to gain. The published band for an engine of this strength: CPW tabulates `{0,5}` for top-200 and `{0,10}` below it |
| `--nonreg` | `elo0=-5 elo1=0`, α=β=0.05 | a change not expected to gain that must not cost — a simplification, a rewrite, a constant moved for another reason |
| `--fast` | `elo0=0 elo1=10`, α=β=0.10 | a first look |

**Those numbers are normalized Elo, not logistic Elo.** `fastchess.sh` passes
`model=normalized`, which is fastchess's default and which its own `--help`
glosses as "normalized — Uses nElo (default)". So `--nonreg` excludes a
regression of 5 **nElo**, and the logistic figure that corresponds to is
*smaller* and is not a constant: the ratio is a property of the draw rate, so it
is read off each run's own printed `Elo` and `nElo` pair rather than converted
with a fixed number. Four worked examples, all from runs recorded in
`specs.md` — 5 nElo is **3.54** logistic Elo at S165's 44.75 % draws
(`nElo 1.34` / `Elo 0.95`), **3.64** at S108's 42.20 % (`3.13` / `2.28`),
**3.92** at S107's (`16.18` / `12.67`) and **3.99** at S021's
(`44.04` / `35.12`). Quote the bound as nElo and, if a logistic figure is
wanted, take the ratio from the run in hand.

**The two sides are wrong in opposite directions if you forget this.** A
non-regression verdict rejects "at or below `elo0`", so `--nonreg` excludes a
regression of about 3.5 to 4.0 logistic Elo — which *implies* the looser "5 Elo
or more is excluded" and then some, so that phrasing is merely conservative. A
gainer verdict is the dangerous half: accepting H1 at `elo1=5` establishes a
gain of 5 nElo, about 3.9 logistic Elo, so writing it up as "gains 5 Elo or
more" claims about 1.1 logistic Elo the run did not demonstrate.

**Do not "fix" this by switching to `model=logistic`.** The CPW table the row
above cites states no scale of its own, but its Stockfish rows — STC `{0,2}`,
LTC `{0.5,2.5}` — are fishtest's bounds, and fishtest expresses bounds in
normalized Elo. `model=normalized` is what makes `{0,5}` mean what the source it
was taken from means, and changing it would re-price every verdict this project
has taken. 2026-08-21 adversarial F09, S157.

Neither default brackets an effect from one side only, and a bound pair that
cannot contain the truth random-walks to the round limit. When the expected
effect is genuinely two-sided, edit the block for the run and record which pair
ran — `adocs/data/S021_sprt.sh` and `S076_sprt.sh` are the two worked examples,
both with the reading of all three outcomes written down *before* launch.

**An SPRT stops early exactly when the observed effect has run favourable**, so
its point estimate is biased upward and is never reported as the effect size.
S068's pooled estimate fell from +12.18 to +5.02 on that correction.

### What the run prints when it ends

Every run writes to its own stamped directory — `/tmp/chesso_sprt_<tag>_<stamp>/`
with `games.pgn` and `fastchess.log` in it, or wherever `OUT` points — and then
counts the terminations, the draw and decisive rates and the time forfeits out
of **that run's** PGN. The last line is `SPRT-RUN-DONE <tag> <dir>`, or
`SPRT-RUN-FAILED:` on any abort, which is the terminal marker a watcher exits
on (DEC-061).

The census reports, it does not void the run: both sides are chesso, so a thin
time-management margin costs both about equally. `rating.sh` is the one that
voids, because there the margin is a foreign engine's too.

**Watch the draw rate.** The unbalanced book is there to keep it down, but
**below about 45 % draws is a failure mode, not a win** — at that point the
opening is simply winning for one side and the pair scores 1:1 with no signal
in it. That is Pohl's own floor for the book class.

### What a verdict costs, measured

**The regime change was calibrated before any verdict was taken with it**
(2026-08-20, S105). Two A/A runs of 1000 games each, same machine, same hour,
fixed rounds so both numbers share a denominator —
`adocs/data/S105_calibration.sh` is the script and the PGNs are beside it.

The third column is the same measurement re-taken on 2026-09-08 under DEC-143,
which makes a fixed-rounds A/A follow every harness change — here the move to
the workstation, the seed and the two PGN fields. `adocs/data/S198_*` is its
evidence and `adocs/data/S198_pairs.py` is the band check.

| | before: 10+0.2, `8moves_v3.pgn` | after: 8+0.08, UHO | workstation, 2026-09-08 |
|---|---|---|---|
| games a minute | 23.1 | **38.7** | **37.9** |
| seconds a game | 30.4 | 17.9 | 18.3 |
| seconds a ply | 0.2579 | 0.1831 | 0.1791 |
| plies a game | 117.8 | 98.0 | 102.1 |
| draws | 40.3 % | **29.5 %** | 32.3 % |
| time forfeits | 0 of 1000 | **0 of 1000** | **0 of 1000** |
| pair score variance | 0.2343 ± 0.0148 | 0.2395 ± 0.0152 | 0.2430 ± 0.0154 |

**Budget a verdict at 2277 games an hour**, the measured figure: 1000 games in
26 m 21 s. The pair variance is inside its band at `z = +0.16`, so what a
verdict costs at fixed bounds has not moved. The throughput difference against
S105 is game length and not machine speed — seconds a ply went *down* while
plies a game went up, which is the engine, not the harness.

**Throughput went up ×1.67, not ×3.** DEC-083 priced the change at "roughly
three times the verdicts per night"; measured, it is 23.1 → 38.7 games a
minute. It decomposes cleanly: the control is ×1.41 (seconds a ply) and the
book is ×1.20 (shorter games, from more resign adjudications), and 1.41 × 1.20
= 1.69 against the 1.67 the wall clock says.

**The book buys game length and nothing else.** Its stated purpose is to spend
fewer games on draws that carry no signal, and at this engine's strength it
does not do that. The pair score variance — the quantity that sets how many
games a verdict costs at fixed Elo bounds — is **the same to within its own
error bar**, ratio 1.022. What moved instead is the wrong way: pairs scoring
1:1 went 41.6 % → 46.8 %, and pairs where whoever had white won **both** games
went 13.4 % → 19.8 %. Those are exactly the pairs Pohl's ≥ 45 % draw floor
exists to prevent.

**And the floor was already unreachable here.** Pohl measured 91.6 % draws on a
balanced book; chesso self-plays the same balanced book at **40.3 %**, below
the floor before any book was changed. His bands were measured between engines
600 points stronger, where a draw is the default outcome and an unbalanced
opening is what breaks the tie. At 2559 the games are decisive on their own.
The book is kept — it costs nothing measurable and buys ×1.20 — but the reason
DEC-083 gives for it does not hold at this strength, and if the book ever comes
up again this is the measurement to argue from. Re-run it with
`adocs/data/S105_pairs.py`.

**0 time forfeits in 1000 games at 8+0.08** was checked first, because the
faster control leaves `MOVE_OVERHEAD_MS 50` about 2.5× less room per move.
Both runs' `fastchess.log` files were **0 bytes** — the WARN-only default,
again, which is why the count comes from the PGN.

Budget from games a minute, not from the wall clock of a previous run. The
pre-S105 record, for comparison: S065 accepted H1 in 3396 games and 2 h 30 m,
S033 in 1012 games and 44 m 10 s, both at about 23 games a minute — so
throughput is the machine and games-to-verdict is the size of the effect. A
verdict cost 3 to 4.5 hours at four cores on the Apple machine (S027, six
verdicts in roughly twenty hours over 13462 games); those figures do not carry
here and neither does the four-core baseline they were taken against.


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

**The watcher also has to end by itself.** `persistent: true` outlives the turn
and it outlives `/clear` as well: the context holding the task id goes, the
process stays, and `TaskStop` is then unreachable — the only way out is `kill
<pid>`. `tail -f` has no exit condition of its own, and `| grep -m 1 DONE` does
not add one, because a log that goes quiet never makes `tail` write again and so
never delivers it SIGPIPE.

**There is no `--watch` primitive any more.** moltke v1 ships no tooling at all
(DEC-109), so the poll loop below is the mechanism and not the fallback it used
to be. It needs the same four exits the primitive had — success marker, failure
marker, watched process died, hard ceiling — and `fastchess.sh` prints the
markers itself, so nothing has to be appended to the log by hand.

**A ceiling is wall-clock only while the machine is awake.** S024's first run
hibernated for 42 hours under a 9 h ceiling and the watcher did not fire, which
is the one exit that is supposed to bound every mistake in the other three. On
a laptop the POWER rule is what actually prevents this: no timed match starts
on battery. Poll — never follow:

```bash
log=.tuning/sprt_<what>.log
pid=<the detached run's pid>
seen=0
end=$(($(date +%s) + 8 * 3600))
while true; do
  tot=$(wc -l < "$log")
  if [ "$tot" -gt "$seen" ]; then
    sed -n "$((seen+1)),${tot}p" "$log" \
      | grep -E "^Elo:|^LLR:|^SPRT-RUN-|Error|Killed|Aborted"
    seen=$tot
  fi
  grep -qE "^SPRT-RUN-(DONE|FAILED)" "$log" && break
  kill -0 "$pid" 2> /dev/null || { echo "watch: run $pid is gone"; break; }
  [ "$(date +%s)" -ge "$end" ] && { echo "watch: 8h ceiling"; break; }
  sleep 30
done
```

**The last two conditions are not optional and this loop went without them.**
AGENTS.md par.12 requires four exits, and a marker-only loop has one: a run
killed by the OOM killer or by `kill -9` writes no marker, and the watcher then
polls a finished log forever. The pid check is placed *after* the marker grep so
a run that wrote `SPRT-RUN-DONE` and exited between two polls is read as done
rather than as vanished.

Breaking out of a loop fed by `tail -f` leaves the `tail` behind for the same
reason `-m 1` does not work. This loop leaves nothing.

An S033 sweep that finished in nine minutes left a bare `tail -f` holding for two
hours on an otherwise idle machine. DEC-061.

To find one already armed:

```bash
ps -eo pid,ppid,etime,cmd | grep '[t]ail -f'
```

Progress without disturbing the match:

```bash
grep -E "^Elo:|^LLR:" .tuning/sprt_<what>.log | tail -2
grep -c "^Finished game" .tuning/sprt_<what>.log
```

### One trap when reading a fastchess.sh result

**The `fastchess.log` is WARN-and-above and is routinely empty.** Measured:
`-log file=X` over a 2-game match wrote **0 bytes**, the same match with
`-log file=X level=trace` wrote **230 KB**. A time loss does appear at WARN —
that is how `rating.sh` caught Stash — but an **empty log is indistinguishable
from a log that was never written**, so grepping it for time losses is weak
evidence and passes for free when the file is missing or stale.
`/tmp/fastchess_fast.log` sat at 0 bytes dated two days earlier while a run
completed against it.

**Check the PGN, not the log**, whenever a step's hazard is losing on time.
S089 is the case this is written for, and it is why the script now does the
count itself.

**The trap that used to sit beside it is gone.** Until S105 the PGN was a fixed
`/tmp/fastchess_<tag>.pgn` and fastchess *appends*, so a census over the whole
file mixed runs: after S089's 500-game SPRT the file held **587** games, 87 of
them against `ref-c56ab41` from an earlier run, and a straight
`grep '^\[Termination'` reported 421/166 instead of the run's real 359/141.
The per-run stamped directory removes it by construction. Old PGNs written
before S105 still have it, and `adocs/data/README.md` records which committed
files came from a shared path.

## Rate the engine against the public lists

```bash
./rating.sh --bracket           # 340 games, checks the reference set brackets chesso
./rating.sh                     # 3340 games, solves an absolute rating with ordo
TC=2+1 ./rating.sh              # pick the time control
CONCURRENCY=6 ROUNDS=50 ./rating.sh
```

`fastchess.sh` answers "is this commit stronger than that one" and cannot answer
"how strong", because both sides of it are chesso. `rating.sh` plays a gauntlet
against engines with published CCRL Blitz ratings and solves the PGN into an
absolute figure with `ordo`. It is not a gate: run it after a milestone, not
before a commit. S087, DEC-067, DEC-068.

**What it needs on the machine is checked before anything runs, and every exit
prints one marker (S177).** `fastchess`, `ordo` and a GNU `timeout` -- `timeout`
itself, or `gtimeout` from Homebrew's coreutils on macOS -- and a way to count
cores (`sysctl`, else `nproc`). Each missing one is refused as
`RATING-RUN-FAILED: <what is missing>`, the run's one terminal line, and the
marker trap is armed before the first command that can fail, so a detached run
never dies silently: before S177 the script died at `$(nproc)` with exit 127
and no marker on the machine this project is developed on
(2026-09-03_adversarial-F03), the class S167 fixed for `fastchess.sh`.
`tests/test_rating_script.sh` holds it on a PATH with none of those tools.

**Run it when substantial work has been done to the engine, and not on a
threshold.** DEC-074 is the owner's decision and it replaced DEC-071's "any
landed step an SPRT credits with 20 Elo or more". Per-change decisions belong to
the SPRT; a rated run costs about 5 hours since DEC-073 dropped it to
concurrency 6, and "substantial" is the owner's judgement rather than something
an agent totals up and books for itself.

**The two numbers are never quoted against each other.** `ordo`'s intervals are
trinomial; every SPRT verdict here runs `model=normalized` and reports nElo.

**`ROUNDS` is rounds per pairing, and each round is two games.** The totals
above are for the five engines in `references.tsv` today -- `rounds x pairings x
2` -- so they move when the manifest does, and the run prints the derived total
before it plays. The rated default is 334 rounds = **668 games per pairing**,
which is measured and not chosen: S087 got +/-34 at 334 games per pairing and
+/-24 to +/-28 at 668. In a gauntlet only chesso plays everybody, so chesso's
rating under a given anchor is fixed by that one pairing alone -- adding a rung
adds an independent estimate and narrows none of the existing ones. Both
defaults carried comments computing against three pairings until S088, and were
already wrong for the four engines DEC-069 installed.

Opponents come from `references.tsv`. The binaries are not in this repository
and never will be — the file records which build each result was played against.
**Every engine is asked `uci` before a game is played** and the run refuses on a
mismatch between `id name` and the manifest, because a filename is not evidence:
the first install of this set put the Rustic workspace development build
(`id name engine 3.99.36`) at `/usr/games/rustic`, which has no published rating
and would have been anchored at the tag build's.

Anchors are read from the CCRL Blitz list at run time, never hardcoded:

```bash
tools/ccrl_rating.py "Leorik 1.0" "Blunder 5.0.0" "Rustic Alpha 3.0.0"
```

It exits non-zero when a name is not on the list, so a missing anchor stops a
rated run instead of silently dropping a reference.

`option.Threads` is deliberately not sent. None of the reference engines exposes
it — all are single-threaded by construction — and chesso's is `min 1 max 1`.
`option.Hash=64` is inside every engine's maximum; the binding one is Blunder at
256 MB.

**A crash voids the run at zero; a time forfeit is weighed against a rate.**
DEC-075 split what used to be one check.

A termination outside `normal`, `adjudication` and `time forfeit`, or a log line
matching `disconnect`, `stall`, `illegal move` or `crash`, is a broken instrument
and **voids the run at zero**. The script still asserts the set of
`[Termination ...]` values actually present rather than grepping for a string it
guessed, so a failure cannot hide behind an unanticipated spelling.

A **time forfeit** is a real game result and is tolerated up to `FORFEIT_MAX_PCT`
(default **1.0**) of an **individual engine's own games**:

```bash
tools/forfeit_report.py <games.pgn> [--max-pct 1.0]   # 0 under, 1 over, 2 unreadable
FORFEIT_MAX_PCT=0.5 ./rating.sh                       # tighten it for one run
```

**The denominator is the engine, not the run, and that is the whole point.** 3
forfeits in 3340 games is 0.09 % overall but **0.45 % of Stash's 668**, so a
whole-run threshold of 0.5 % would tolerate sixteen forfeits landing on one
engine while printing a comfortable number.

The report prints each overrun, because the rate alone cannot tell a thin
`Move Overhead` from a broken search. S088 measured both: 149, 1118 and 1309 ms
of ordinary margin overrun at concurrency 12, and a **25360 ms hang** after 137
normal moves at concurrency 6. Raising `FORFEIT_MAX_PCT` above 1 needs a recorded
decision, not a flag on a command line.

On any failure the script prints `RATING-RUN-INVALID` with the reason and exits
non-zero.

The run ends with `RATING-RUN-DONE <mode> <OK|INVALID> <outdir>` as its last
line, which is the terminal marker a watcher exits on. DEC-061.

**The interval is set by game count and the anchor spread is not.** 1336 games
gave ±34; 2672 gave ±25. Run twice and concatenate the PGNs when one run is not
tight enough — `ordo` takes the combined file, and both runs must use the same
binaries and the same time control. No number of games narrows a disagreement
between the references themselves; that is what the anchor sweep reports.

**Last result: chesso ≈ 2559 CCRL Blitz, ±25, soft** (2026-08-18, 3340 games at
`10+0.2`, **`Hash=64`**, five engines, three families). S105 raised this
script's hash to 128 to match the list's own 128-to-256 (DEC-089), so the next
run is not comparable in absolute size with the 2559 — the instrument moved,
the way it moved at DEC-049. Record and caveats in
`adocs/data/rating_2026-08-18_S088_ccrl_blitz.md`; S087's four-engine 2570 is in
`rating_2026-08-18_ccrl_blitz.md` and is not superseded as a record, only as the
current figure. `src/` is byte-identical between the two, so the difference is
the instrument and not the engine.

### Watching a rating run

Use a watcher that **exits on the marker**, not one that merely reports it:

```bash
log=/tmp/chesso_rating.log
until grep -qE "RATING-RUN-(DONE|FAILED|INVALID)" "$log" 2>/dev/null; do sleep 30; done
grep -E "RATING-RUN-|unexpected terminations|^anchored on" "$log"
```

`tail -f "$log" | grep RATING-RUN-DONE` looks equivalent and is not: it emits
the marker as an event and then keeps running forever, because `tail -f` has no
exit condition and a log that stops growing never delivers it SIGPIPE.

**The marker is not the only way a run ends, so watch the process too.** A run
that dies before printing its marker -- an OOM kill, a `fail()` on a path the
`tee` never reaches -- leaves the loop above spinning on a log that will never
change again, which is a leaked watcher by a different route. Add a liveness
arm, and **capture the pid from `$!`**:

```bash
nohup env OUT=/tmp/rating_out ./rating.sh > "$log" 2>&1 &
pid=$!                       # nohup, env and bash all exec in place, so this
                             # is rating.sh's own pid
while ! grep -qE "RATING-RUN-(DONE|FAILED|INVALID)" "$log" 2>/dev/null; do
  kill -0 "$pid" 2>/dev/null || { echo "gone with no marker"; break; }
  sleep 60
done
```

**A `grep -c` guarded with `|| echo 0` produces `0\n0`, and every integer test
against it is an error that evaluates false.** `grep -c` already prints `0` on no
match -- it just exits 1 while doing it -- so the guard appends a second zero.
The symptom is an alarm that never fires and a progress line reading
`0\n0 forfeits`, which looks like good news. This was live for 45 minutes of
S088's five-hour run: the early forfeit alarm could not have fired at all. Write
it as `n=$(grep -c ... 2>/dev/null); n=${n:-0}` and **prove the alarm is
non-vacuous by pointing it at a log that did forfeit** -- `adocs/data/` keeps
one, and the expression must return 3 against
`S088_rated_c12_INVALID`'s fastchess log before it is trusted to return 0
against a live one.

**Do not recover the pid with `pgrep -f 'bash ./rating.sh' | head -1` after the
launch.** That was tried during S088 and it returned a pid that had already
exited -- a transient in the `nohup`/`env` chain -- so the watcher declared the
run dead **60 seconds in, while it was playing game 15 of 3340**. A false death
is cheaper than a leak and it is still wrong: the next thing an agent does with
that report is relaunch a two-and-a-half-hour match that is already running.

**This is not hypothetical. It leaked a watcher for 10 h 46 m during S087.** All
four of that step's watchers were armed as `tail -f | grep`, three were rescued
by a hand-typed `TaskStop`, and the fourth was skipped because reading the run's
result felt like closing the loop. It was found when the owner noticed the CPU
was idle and asked what was being monitored. `rating.sh` had printed the
terminal marker the whole time — the mechanism was built in the same session and
then not used. DEC-070, DEC-061.

When you take a run's result, also check nothing is still watching it:

```bash
ps -eo pid,etime,cmd | grep '[t]ail -f'
```

## Regenerate the magic numbers and the Zobrist keys

Two tables in this tree are drawn from a pseudorandom stream rather than
derived: the 128 sliding-attack magic numbers in `src/bb_tables.hpp`, and the
851 Zobrist keys `init_zobrist` fills. One command draws both, under one seed —
`CHESSO_PROJECT_SEED` in `src/bitboard.cpp`, 20260904 — so that neither is a
table whose origin can only be argued (S179, DEC-132, DEC-139).

```bash
cmake --build build -j12 --target magic_gen

# The magics: both arrays on stdout, the per-square attempt counts and the
# coincidence line on stderr.
./build/tools/magic_gen magics --seed 20260904 > /tmp/magics.txt

# The keys: the quality report, and whether the engine's are these.
./build/tools/magic_gen zobrist --seed 20260904
```

**The engine uses both since S203.** `zobrist` prints `matches init_zobrist:
yes`; if it ever prints `no`, either the seed passed here is not
`CHESSO_PROJECT_SEED` or `init_zobrist`'s four loops have been reordered, which
changes every key.

`magics` prints the two arrays in `src/bb_tables.hpp`'s exact form. Paste them
over the existing ones, leave the relevant-bit counts and everything else in that
file alone, and run `./clang-format.sh` to restore the column alignment. Then
rebuild and run the tool again: its stderr must read `coincide with the
compiled-in arrays: 128 of 128`, which is the proof that what is committed is
what the seed reproduces. Against a *different* set that same line is the
coincidence check — it read `0 of 128` when S179 replaced the 2023 arrays, which
is the number that ended their coincidence with a published set.

The generator is deterministic by construction: one thread, fixed square order,
no wall-clock input, unsigned arithmetic only. The same seed prints
byte-identical arrays on any machine. Finding all 128 takes about 1.6 s here,
9270054 candidates over the 64+64 squares.

**New magics do not change what the engine plays and new keys do.** A magic is a
perfect hash of a square's relevant occupancies into a table whose size is fixed
by the relevant-bit count, so a different valid set moves which slot an occupancy
lands in and nothing `generate_moves` returns: prove it with
`./build/tests/bench_movegen` (it verifies its perft counts before it times
anything), `ctest --test-dir build -L slow -R test_perft`, and identical node
counts and best moves from `tools/search_bench.py`. New keys move which positions
share a transposition-table slot, so the node counts move once — and they also
retire the `go` budgets in `adocs/data/S170_cases.tsv`, whose cases are eviction
reproductions. Re-derive them with `adocs/data/S203_case_sweep.sh`, which is
cheap and needs no match; the games survive a redraw and only the budgets move.
DEC-154 and DEC-156 are the history, and the grid is a knife edge — one case
reports 13 mate lines at 1500000 nodes and 0 at both 1000000 and 2000000.

**A key redraw is not the only thing that retires those budgets, and since
2026-09-09 that is measured.** Any change that moves the tree does, which is
DEC-156's own consequence, and the grid is sparse enough that the guard fires
on changes it has no opinion about: over the nine stride-1 budgets,
`C_mate7_depth11` reports a mate line in **exactly one cell** at `c982f9d`, and
that cell is its configured budget. So a red `test_mate_carry` after a `src/`
change is a question and not a verdict — sweep both sides before concluding
anything, as `adocs/data/S204_sweep_head.txt` and
`adocs/data/S204_sweep_killer_iter_clear.txt` do.

**S204 answered it: re-pinning at every step is not a guard, and the test no
longer asks you to.** DEC-162 deleted the per-case floor on mate lines, which
was the number a tree-moving change moved, and replaced it with a fixture-wide
majority — three of the five guarded cases must report a mate line. A short
mating PV is no longer forbidden either; it is S202's residue, counted against a
per-case ceiling that `adocs/data/S203_case_sweep.sh --ceilings` re-derives from
the two recorded grids. What is asserted at zero and pinned to nothing is the
one thing that does not move: a line as long as the distance it claims ends in
checkmate. So a red `test_mate_carry` now names which of the three fired, and
only the ceiling one is a budget question. The budgets themselves did not move
and a redraw still retires them.

`zobrist` reports the checks the wiki's linear-independence rule asks for at the
sizes that can be enumerated — no key zero, all 851 distinct, no pair XOR equal
to a key, no two pair XORs equal — plus the minimum pairwise Hamming distance,
which is reported and not asserted.

## The engine's own opening book

Not the match books under `books/` — this is the book the *engine* plays from,
off by default and enabled with `setoption name OwnBook value true`.

`src/openings.bin` is the built-in one: 2755712 bytes, 172232 Polyglot entries
over 129613 positions, linked into the binary by `.incbin` from
`src/openings_embedded.S`. It is the raw file, not a header — editing it and
rebuilding is all it takes to ship a different book, and `OBJECT_DEPENDS` in
`src/CMakeLists.txt` is what makes the rebuild happen.

**This project builds it, and these two commands are the whole origin** (S146,
DEC-131). The input is the committed CC0-1.0 `books/8moves_v3.pgn`, which
`fetch_book.sh` pins by both digests; the moves are read by the engine's own
`algebraic_to_move` and keyed by its own `get_key`, so nothing in the path comes
from another engine:

```bash
./books/fetch_book.sh 8moves_v3.pgn
build/tools/make_book build books/8moves_v3.pgn --out src/openings.bin

shasum -a 256 src/openings.bin
# 77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58
```

The build is deterministic — output is sorted by key and then by weight, so the
digest above is reproducible from the PGN — and the defaults are the ones used:
`--max-ply 16`, which is the full depth of every line in that PGN, and
`--min-games 1`, which drops nothing. Because every game in it is recorded
`1/2-1/2`, an entry's weight is exactly the number of book lines that played the
move.

**Checked from outside since S175.** `adocs/data/S175_book_conformance.py`
(python-chess, `~/.venv/chess`) re-derives every entry from the PGN with an
independent implementation of the key and compares the multiset with the file;
`missing 0 extra 0 weight_mismatch 0` is what the shipped file reports, exit 0.
It exists because `make_book` keys with the engine's own `get_key()`, so a book
it builds agrees with the engine whether or not the key is the format's: the
2026-09-03 file (sha256 `3b89a4ad…15b873dd`) carried 7 keys that wrapped round
the board edge on an edge-file en-passant square, the engine probed them
happily, and only a reader outside the project could see it
(2026-09-03_adversarial-F01). Run it after any rebuild:

```bash
~/.venv/chess/bin/python adocs/data/S175_book_conformance.py \
    books/8moves_v3.pgn src/openings.bin
```

The book this replaced was 2610256 bytes and 163141 entries, sha256
`47a81735…ce78fb5`. It was inherited from the `bitboard` branch, no document or
commit recorded where it came from, and the owner could not place it, so it was
deleted rather than shipped unaccounted for.

`build/tools/make_book` is the only thing in the tree that can produce such a
file. It builds one from a PGN with the engine's own parser and the engine's own
`get_key()` — a book keyed by a second definition of "the same position" loads
and then finds nothing, silently — and it reads one back:

```bash
# From a PGN. --max-ply is how deep a book line goes, --min-games how many
# games a move needs before it is written at all. A game the parser cannot
# follow to its end refuses the whole build; --allow-cut-short drops such
# games from the bad token on and builds from the rest.
build/tools/make_book build books/8moves_v3.pgn --out /tmp/x.bin \
    --max-ply 16 --min-games 1

# What a book contains, and whether the engine can load it.
build/tools/make_book dump src/openings.bin --top 10
```

`dump` runs the same two checks the engine's loader runs — the size is a whole
number of sixteen-byte entries, and the keys are sorted — and exits non-zero
when either fails, so a book it calls `loadable` is a book `Book File` will
accept.

**`loadable` does not mean complete, and neither check can make it mean that.**
Entries are 16 bytes and sorted, so *any prefix of a book is also a valid book*.
Found while doing S146: writing the 2755712-byte book onto a 1 MB volume left
901120 bytes, `dump` called the result `loadable` with 56320 entries, and the
engine loaded it.

Since S173, `build` never writes to `--out` at all until the bytes are safe. It
writes them to `<out>.tmp` beside the destination, flushes it, and renames it
over `--out` only once the write is verified, so a failed write — a full volume,
a file-size limit, a kill — leaves any previous book at `--out` byte-for-byte
unchanged, and a reader sees the old book or the new one and never anything
between. Every failure the process lives through removes the temporary and says
`book not written` with the reason; a `SIGKILL` mid-write leaves `<out>.tmp`
behind, which the next build to the same destination reuses. The name is fixed
rather than unique, so two concurrent builds to one `--out` would clobber each
other's temporary. `SIGXFSZ` is ignored by the tool for this to work: by default
it *kills*, and before S173 that killed the process mid-write — under
`ulimit -f 200`, exit 153 and a 204800-byte truncated book left at `--out` that
`dump` accepted. Ignored, the limit arrives as `EFBIG` from `write` instead.

The portable reproduction, no `sudo` and no ram disk, is a file-size limit in a
subshell (`RLIMIT_FSIZE` is per process, and the message must go through a pipe
or it hits the same limit):

```bash
printf 'previous book' > /tmp/keep.bin
( ulimit -f 200; build/tools/make_book build books/8moves_v3.pgn --out /tmp/keep.bin ) 2>&1
# wrote 204800 of 2755712 bytes to '/tmp/keep.bin.tmp': File too large -- book not written
cat /tmp/keep.bin   # previous book, 13 bytes, untouched
```

None of that makes `loadable` mean complete: if a book reaches you by any other
route than this tool, its digest is the only thing that says it is whole.

**A cut-short game refuses the build, and `games cut short 0` is evidence
since S174 and not before.** `algebraic_to_move` — the engine's SAN parser,
which this tool and `pgn_to_positions` both read moves through — used to fail
with `assert(false)` and no return, so the Release build handed back a
fabricated move that `make_move` applied: at `1d8cbac`, `1. e4!? e5 2. Nf3
Nc6 *` built a `loadable` book whose first entry was the start position with
move `a8a7`, reported `games cut short 0` and exited 0
(2026-09-03_adversarial-F02). The parser returns 0 for a token it cannot read --
which since S201 includes one beginning with anything but a piece or a file
letter, because a leading `.` or `-` used to fall to the pawn branch and the
disambiguation walk then swallowed the piece letter, turning `.Nf3` into the
legal pawn push `f2f3` -- and strips PGN suffix annotations (`!`, `?`, `!?`,
`?!`, `!!`, `??`) as it strips `+` and `#`; `build` exits non-zero and writes nothing when any game was
cut short, naming the game and the token on stderr (the first twenty). The
token is quoted as the PGN wrote it, so a glued move number is part of the
quote: `cannot parse '2.Qxf7' at ply 2` names the one line that holds it where
`Qxf7` would name every line that plays the move somewhere (S200).
`--allow-cut-short` is the deliberate form of the old behaviour: such a game is
dropped from the bad token on and the count is reported.
`tests/test_make_book_tools.sh` holds all of it, and the shipped book rebuilt
through the gated tool is byte-identical to the committed file.

**Move numbers are read in every PGN form.** `movetext_to_san` drops a move
number indication whether it stands alone, which is export format (`1. e4 e5`,
`2... Nc6`), or is glued to the move it introduces, which import format allows
(`1.e4 e5`, `2...Nc6`, `4.O-O`): digits, then one or more dots, then the move,
and only the move survives (S178). Whitespace *between* the digits and the dots
— `1 . e4`, `1 .e4`, `1. ... e5`, which 8.2.2.1 allows as well — is read too
(S200). The dots then arrive as a token with no digits in front of them and the
same rule runs from the front: `.` and `...` alone are dropped, `.e4` gives
`e4`, `...Nc6` gives `Nc6`. Nothing legal begins with a period — 7.3 makes it a
token by itself and 7.9 keeps it out of a symbol's continuation characters — so
a leading run of dots is always an indication. A token of digits and dots alone
is still dropped and a bare number is still dropped. What is left after the dots
is handed to the parser and never re-classified: `.2` cuts the game short as
`.2`, the same as `1.2` does, because silently dropping broken input is the
fault class S174 closed.

Weights are two for a win, one for a draw and nothing for a loss, from the
moving side's point of view, summed over every game that played the move; an
entry that ends at zero is dropped, since the engine's weighted draw could never
play it. Weights are clamped to 65535 and the build reports how many were.

## Analyse a game

Never by reading it. See `CLAUDE.md` and DEC-023.

```bash
# SAN moves, one per line or whitespace separated, no move numbers. Suffix
# annotations (e4!?) are fine; a token the parser cannot read exits 1 naming
# the ply instead of printing a position that never occurred (S174).
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
build/tools/datagen --out .tuning/selfplay_v2.tsv \
    --games 120000 --nodes 100000 --threads 12 --seed 20260814 \
    --allow-tactical 1

build/tools/tuner --data .tuning/selfplay_v2.tsv \
    --out .tuning/tuned_v2.hpp --threads 12
```

**The corpus on this machine is `.tuning/selfplay_v2.tsv`** and those are the
flags that produced it, which the run's own log states rather than a comment
here:

```
datagen: 120000 games, 100000 nodes per move, 12 threads, seed 20260814, quiet-limit 1000, allow-tactical 1
120000 games, 11003693 positions written to .tuning/selfplay_v2.tsv
filter: 13759085 considered, 11003693 recorded; skipped in-check 957698, tactical best move 0, mate 0, score past 1000 1205452
```

11003693 positions from 120000 games, 715409623 bytes, generated 2026-08-14
between 01:05:11 and 08:13:32 — **7 h 08 m** on 12 threads, 4.669 games/s and
428.2 positions/s, 91.70 positions per game, 65.0 bytes per row. Faster per game
and slightly thinner per game than the 600-game smoke run below, which is the
figure the size was extrapolated from. `tactical best move 0` is
`--allow-tactical 1` doing its job: the clause is switched off, so it rejects
nothing and the counter has nothing to count. S065, DEC-055.

`.tuning/selfplay_v1.tsv` was S028's corpus — 1490839 positions from 20000 games
at seed 20260810, under the old filter — and it **does not exist on this
machine**: `.tuning/` is gitignored, the work moved machines at DEC-049, and the
engine that generated it is seven steps behind the one that generated v2, so it
is not regenerable identically either.

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

### Deduplicate a corpus by position

S076. `datagen` never asks whether it has written a position before, so a
position reached in two games — or repeated inside one game on the way to a
repetition draw — carries its weight once per occurrence.

```bash
build/tools/corpus_dedupe --in .tuning/selfplay_v2.tsv \
    --out .tuning/selfplay_v2_dedup.tsv
```

**Measured on `selfplay_v2.tsv`:** 11003693 rows in, 10795695 distinct
positions out, **207998 dropped, 1.8903 %**, in 19 s and 1.0 GB resident. The
repeats are concentrated rather than spread: 10733523 positions occur once,
35124 twice, 9585 three times, 10547 four to seven times, 4284 eight to fifteen,
2632 sixteen or more, and one position appears **120** times.

The key is the engine's own zobrist — `load_FEN` then `board.hash` — so the four
FEN fields it covers decide identity (placement, side to move, castling rights,
en passant square) and the halfmove clock and move number do not. It is not the
FEN text: two rows of the same position at different clocks are one position to
`evaluate()`.

The first row of a repeated position survives, byte for byte, and no label is
averaged — DEC-065, which records the averaging variant as a separate change
rather than as a better one folded in here. So the output is still exactly what
`datagen` wrote and loads anywhere the four-column format does.

`--verify` additionally holds the four hashed fields per key and counts
key-equal rows that disagree on them, which is what a 64-bit collision looks
like. It roughly doubles memory, 1.9 GB on this corpus, and reported **0** of
the 207998. Two runs over the same input are byte identical:
`0a6b59b6af9f50c4360cac7c87c33250318e712250f2653eb09bcb9f0b64b69f`.

A row that cannot be read — fewer than four fields, or a FEN that does not load
— refuses the whole pass and removes the partial output. A filter that silently
drops rows is what this tool exists to make visible.

### Re-measure the truncation-bound positions after a fit

S076. `tests/test_eval_model.cpp` pins four positions whose float model and
integer engine scores disagree by 69/24 = 2.875, the maximum three truncating
divisions can produce, so the tolerance in "the model reproduces evaluate() on
every phase" is asserted against a corpus that reaches it. **A residual belongs
to the weights, not to the position**, so those four stop qualifying every time
the constants are refitted — DEC-057.

```bash
build/tools/truncation_scan --data .tuning/selfplay_v2_dedup.tsv --min 2.8
```

Prints `difference, phase, side to move, fen` per hit and a summary line. 47 s
over 10.8 M rows. At S076's constants: 135399 rows past 2.0, 99 past 2.8, **30
at 2.875**, and the four pinned are drawn from those 30 to span the taper with
both sides to move. It reproduces `test_eval_model`'s own numbers to six digits
on whatever it is handed, because its model is that file's `model_score_white()`
with the assertions removed.

`tuner` cost on the same machine, measured on a 56304-row corpus at 12 threads:
0.29 s to load, 2.22 ms per epoch, 10.9 MB resident. All three scale with rows,
and the whole corpus is held in memory. On the real 11003693-row corpus, 12
threads, machine idle: **54 s to load and fit K, 0.35 s per epoch, 1.2 GB
resident**, so 3000 epochs cost 1090.87 s. The per-epoch figure is the
game-level split's; a fully shuffled row index costs 0.76 s instead, 2.14x, for
the identical fit. The resident figure is pre-S075 and is **1.33 GB** since, for
the two target vectors and the `score` column `--lambda` needs; the rest of the
line was not re-measured.

S065's full fit over the same corpus took **2082 s** on the same 12 threads for
the load, the K fit and 5000 epochs together — 0.42 s an epoch inclusive, against
the 0.35 s above on an idle machine, this one sharing the machine with a desktop.
Two runs at the same seed and thread count, differing only in `--epochs`, printed
identical epoch reports to six digits through epoch 5000, so the fit is
deterministic and a shorter budget traces the same curve rather than a different
one.

S065's **second** fit over the same corpus, with `--freeze tempo,piece_placement`
(DEC-057), took **2329 s** for 817 free parameters: the same K = 0.7624, held out
0.122560 → **0.118460**, best at epoch 5000. It shared the machine with compiles
and corpus scans for its first quarter, so the 247 s over the first fit is
contention rather than the freeze.

**The second fit's constants are the ones in `eval_tables.hpp`, re-anchored.**
The first paste fired three guards and was reverted; the second fired one,
`POSITIONAL_ROOM`, and DEC-059 answered it by spending the degeneracy
`tools/tuner.cpp:26-30` documents rather than by widening the guard: 432 off
`#define QUEEN` and 432 onto all 128 of her squares, which moves the evaluation
by one centipawn on one of seven pinned positions and by nothing on the other
six. `adocs/plan_done/S065_corpus_regen_loosened_filter.md` records the
transformation and both fits' guard outcomes.

**A fitted material value only means anything together with its own tables.**
`QUEEN` reads 716 here against the 1067 that shipped before the fit and the 1148
the fit itself produced, and none of the three is comparable with the others.

**The SPRT decided it: +21.10 +/- 10.47 Elo, H1 accepted at LLR 2.95 over 3396
games in 02:30:12**, candidate against `a2f0065` at 10+0.2 on 12 cores. The
constants are kept. The held-out error ranked nothing and this is the figure
that decided it.

**S076 refitted the same corpus deduplicated and those constants ship now.**
Same settings, same seed, same freeze, `--lambda 0`; the only difference is
1.8903 % fewer rows. 1378 s, best at epoch 1600, held-out error against the game
result **0.119608 → 0.119458** over the deduplicated corpus's own held-out rows
— which is not comparable with the 0.118460 above, because a different row set
is a different number. **`fitted K = 0.7801` against 0.7595 for the same
starting constants on the full corpus**, so removing the repeats moved the
score-to-outcome scale itself by 2.7 %. The verdict: **+26.68 +/- 16.40 Elo,
nElo 34.44, H1 accepted at LLR 2.95 over 1044 games in 00:46:48** against
`a579f46`. Bias warning as always — a run that stops early stops when the
observed effect has run favourable, so the recorded claim is "not a regression
of 5 **nElo** or more, sign positive at LOS 99.93 %" rather than +26.68 — and
this run's own pair prices that bound at **3.87 logistic Elo** (`nElo 34.44`
against `Elo +26.68`, ratio 1.29).

**Refitting fires two guards, and both are answered by re-deriving rather than
by relaxing.** `.tuning/anchors.py` recomputes the ten pinned absolute anchors
from the specification — 10 of 10 at the new weights, and the suite asserts them
as exact equalities. `build/tools/truncation_scan` re-measures the four
positions `tests/test_eval_model.cpp` pins for the truncation bound, which
belong to the weights and not to the positions (DEC-057): S065's four fell to
between 0.08 and 1.83 under S076's constants, and 30 of the 10795695 rows reach
the 2.875 maximum instead. Expect both on the next fit.

### Audit what a corpus contains, per feature

S100. Five evaluation terms shipped at zero and one passer bucket fitted below
the bucket beneath it, and five terms at exactly zero is a pattern rather than
five results. This is the tool that answers the corpus half of why.

```bash
build/tools/feature_audit --data .tuning/selfplay_v2_dedup.tsv --sample 200000
```

Four reports off one load, each switchable off with `--no-columns`,
`--no-identities`, `--no-occurrence`, `--no-collinearity`. About 4 minutes over
10.8 M rows on 12 threads, most of it the load. `adocs/data/S100_feature_audit.txt`
is the run S100 read.

- **columns** re-extracts every stored feature column from the FEN text and
  compares it against what `load()` kept: **10795695 rows, 0 disagreements**. It
  calls the same extractors `load()` calls, so it is not a check of what a
  feature *means* — `tests/test_eval_model.cpp` holds those against the engine's
  own counts and against hand-computed placements. What it catches is a
  misaligned base, a wrong stride, or a column pair stored in the other order.
- **identities** checks two dependencies that are exact by construction. Index 0
  is a8 and a black piece mirrors by `^56`, so a rook or a pawn on its own
  seventh rank always lands on squares 8..15: the rook-on-the-seventh column *is*
  the signed sum of eight rook piece-square columns, and passer bucket 5 *is* the
  signed sum of eight pawn ones — a pawn on its seventh is a passer by
  definition, because "ahead" is the enemy back rank and
  `tools/eval_model.hpp:596` refuses a pawn standing there. **0 violations** in
  1264773 and 550880 non-zero rows.
- **occurrence** reports the share of rows where each column is non-zero, whole
  corpus and by phase band. Split by band because a column can be well covered
  overall and empty in the band whose weight it is: phase 0 has `mg_weight`
  exactly 0, so a middlegame weight gets no gradient at all from those rows.
  That is not hypothetical — it is what put three passed pawn parameters out of
  reach of the whole test corpus until S100 added four positions.
- **collinearity** regresses each term column on the piece-square columns that
  could absorb it and prints R². The rook file features get **rook + pawn**
  columns, 128 in all, because an open file is a statement about the pawns as
  much as about the rook. Measured: rook seventh and passer bucket 5 at
  **1.000000**, everything else between 0.168 and 0.621 — so the two exact
  identities are the only redundancy, and the regression validating itself
  against algebra already proven is the point of running it on them.

`--plant` is the other half, and it is a writer rather than a report:

```bash
build/tools/feature_audit --data slice.tsv --plant planted.tsv \
    --plant-group passed_pawns --no-columns --no-identities \
    --no-occurrence --no-collinearity

build/tools/tuner --data planted.tsv --out recovered.hpp \
    --threads 12 --epochs 20000 --only passed_pawns
```

It copies the corpus with the result column replaced by
`sigmoid(K * model_score)` at planted non-zero weights, so the labels come from a
parameter vector that is known. A fit that cannot get it back has a defect; a fit
that can has its pipeline vouched for end to end. Every planted number is printed
into the run's own log and comes from nowhere outside this repository.

**`--plant-group` matters and is not decoration.** `--only GROUP` holds every
other parameter at what the engine ships, so a plant that moved three groups
against a fit that frees one leaves the freed group absorbing the other two — a
real disagreement with the planted vector that says nothing about the pipeline.
Plant one group, free exactly that group. `all` is for the joint fit.

What S100's four runs found: the joint fit reached **train error 0.000000** and
returned 18 of the 22 identified term parameters exactly, with every deviation
inside a documented null direction — the two seventh-rank ones above, plus the
material-against-table one `tools/tuner.cpp:36-40` names, which showed up as the
pawn table uniformly 5 low against `PAWN` 5 high. `--only passed_pawns` recovers
all twelve exactly, bucket 5 included, because holding the table resolves the
degeneracy.

**Two parameters cannot be fitted jointly and read as values.** Only the sums
`psqt[rook][8..15] + placement[seventh]` and `psqt[pawn][8..15] + pp[5]` are
identified. Across three real fits on two corpora, passer buckets 0 to 4 moved by
at most 4 and bucket 5 moved by 39 and changed sign twice — the ridge, not the
pawns. A refit of either is frozen-base, and the ledger row names the sum.

### What an emitted table says it came from

S077. The header `tuner` writes names the engine commit and the corpus by
content, not by path — `.tuning/selfplay_v2.tsv` has already meant two different
files on two machines, and S082 and S083 write two more:

```
// Fitted by tools/tuner from 200000 self-play positions.
// engine     d9fb0b2-dirty, the commit this tuner was built from
// data       .tuning/selfplay_v2_dedup.tsv
// corpus     sha256 ffdcd801ab59c58390503bfee468a5caeb71a6bb38c012904ec87675382533de
//            200000 rows, 12995744 bytes; `sha256sum` over the same file prints the same
```

The same two lines are printed to stderr before the fit starts, so a run log
carries them even when the emitted file is lost.

**The hash is SHA-256 over the file's bytes, written from FIPS 180-4 in
`tools/corpus_hash.hpp`, precisely so `sha256sum` can check it** — a hash only
this binary can reproduce would not be provenance. Measured on the 706 MB
deduplicated corpus: 2.70 s here against 1.29 s for `sha256sum`, which uses the
CPU's SHA extensions, and both print
`0a6b59b6af9f50c4360cac7c87c33250318e712250f2653eb09bcb9f0b64b69f`. Against a
55 s load and 0.4 s an epoch, the stamp is under a fifth of one percent of a
fit. A corpus that cannot be hashed refuses the run rather than emitting a
table with a blank provenance line. DEC-066.

**The commit is stamped at build time, not at configure time.** `cmake
-S . -B build` runs once, so a sha captured there goes stale on the next commit
and the table would name a commit its binary was not built from.
`cmake/build_info.cmake` runs on every build through the `chesso_build_info`
target, writes `build/tools/generated/chesso_build_info.hpp` only when the value
changes, and appends `-dirty` on the same `git diff --quiet` convention
`fastchess.sh` uses. Nothing to do by hand; a rebuild after a commit picks it up
without reconfiguring.

Tables emitted before S077 carry no stamp — `.tuning/tuned_v2*.hpp` and
everything under `adocs/data/S075_fits/` and `S076_fits/`. They are evidence,
byte for byte what the tool wrote, and are not regenerated.

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
build/tools/tuner --data .tuning/selfplay_v2.tsv \
    --out .tuning/tuned_ks_only.hpp --only king_safety
```

`--only GROUP` fits one group and holds every other parameter at what the engine
ships. Groups are `all` (default), `material`, `psqt`, `mobility`,
`king_safety`, `passed_pawns`, `pawn_structure`, `piece_placement`, `tempo`, and
the emitted header records which was used. The eight named groups partition all
827 parameters: 5, 768, 8, 18, 12, 6, 8, 2 in that order, which the run's first
line reports as the free count.

```bash
build/tools/tuner --data .tuning/selfplay_v2.tsv \
    --out .tuning/tuned_v2_frozen.hpp --threads 12 --epochs 5000 \
    --freeze tempo,piece_placement
```

`--freeze LIST` is the inverse and the two compose. It takes a comma-separated
subset of the same group names, holds each of them at what the engine ships, and
fits everything `--only` left free beside them; the default is empty, which
holds nothing. `all` is refused, as is any name `--only` does not know, and a
name it does not know refuses the whole list rather than applying the half it
recognised. The run's first line and the emitted header both carry it — `freeze
tempo,piece_placement, 817 free` and `// freeze     tempo,piece_placement` —
and `(nothing)` when no `--freeze` was passed. A `--only` and `--freeze` pair
that between them leave no parameter free is refused rather than run.

**At its default it changes nothing.** The pre-S065 binary and the post-S065 one
over the same corpus at the same seed, thread count and epoch budget printed
identical epoch reports and emitted all 827 constants identically; the only
differences anywhere are the new `freeze` field on the first line and the new
`// freeze` header comment.

`--only` exists for attribution and `--freeze` for the case attribution cannot
express: **hold two groups at zero while the other 817 parameters move.** S065
needed exactly that — `tempo` frozen so the model-versus-engine truncation bound
stays at three effective divisions, and `piece_placement` frozen because S027
measured it at −5.48 +/- 11.46 and zeroed it so the compiler would delete it.
`--only` can say "fit tempo" and never "fit everything but tempo". DEC-057.

**Freeze during the fit, not afterwards.** Fitting all 827 and then zeroing two
groups is a different and worse result: with a parameter held, the remaining 817
absorb what it would have taken, so post-hoc zeroing leaves the other 825 fitted
against a value that is no longer there.

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

### `--lambda`: fit toward the search score as well as the outcome

```bash
build/tools/tuner --data .tuning/selfplay_v2.tsv --lambda 0.7 \
    --out .tuning/tuned_l07.hpp --threads 12 --epochs 5000 \
    --freeze tempo,piece_placement
```

S075. The label every row is fitted against is

```
target = lambda * sigma(K * score) + (1 - lambda) * result
```

where `score` is `datagen`'s third column — the engine's own score for the
position, White relative — and `result` the game's outcome. `--lambda 0` is the
default and is the outcome alone, which is what every fit before S075 used. The
argument for a blend is `adocs/eval_tuning_strategy.md` section 2.7: the outcome
is unbiased and very high variance, one noisy bit spread over about 92 positions,
and the search score is low variance and biased toward the evaluator that
produced it. A value outside `[0, 1]` is refused, NaN included.

**Two numbers, and only one of them ranks a lambda.** DEC-064:

- `validation` is the error against the target that was trained on. It is **not
  comparable between two lambdas** — they trained against different targets.
- `wdl` is the error against the game result, printed as an extra column at every
  report when `--lambda` is non-zero and stamped into the emitted header as
  `// wdl error`. This is what compares one lambda with another, what the kept
  checkpoint and the early stop are decided on, and what the refusal warning
  tests.

At `--lambda 0` the two are the same function, so the column is not printed and
that path is unchanged. **K is fitted against the game result whatever
`--lambda` says** and then held: the blend is built from K, so fitting K against
it would have K chasing itself and two lambda runs would not share a scale.

**At its default it changes nothing.** The pre-S075 binary and the post-S075 one
over the same 1000000-row corpus, same seed, split, freeze and epoch budget, at
`--k 1.5` so the fit moved on every one of its 30 reports: identical 34-line
logs, bar the output path each was told to write, and identical emitted
constants,
`sha256 9bbf81126aa44f93c90dcc2bb08888183306c9e2717562e980bb5646a7dbdbaa` over
everything from the first `#define`. The only difference anywhere is the new
`// lambda` header line. `--lambda 0.7` over the same fixture emits
`d0d4eb8a…`, which is what makes the equality evidence rather than a tautology.

The corpus costs 1.33 GB resident at 11003693 rows against the 1.2 GB before
S075: two `double` targets at 88 MB each and the `score` column at 22 MB.

**Measured on `selfplay_v2.tsv`, the blend is monotone harm and the flag ships
at 0.** S075 swept six lambdas twice — 5000 epochs at report 100, then epochs
1..200 at report 5 — at `--freeze tempo,piece_placement --seed 1 --validation
0.1`, K fitted at 0.7595 in all twelve runs. Held-out error against the game
result, incumbent **0.118457**:

| lambda | best `wdl` validation | vs incumbent | `validation`, i.e. the training objective |
|---|---|---|---|
| 0.0 | 0.118452 | -5.0e-06 | 0.118452 |
| 0.25 | 0.118530 | +7.3e-05 | 0.074249 |
| 0.5 | 0.118838 | +3.8e-04 | 0.042760 |
| 0.7 | 0.119014 | +5.6e-04 | 0.026729 |
| 0.85 | 0.119122 | +6.7e-04 | 0.020047 |
| 1.0 | 0.119213 | +7.6e-04 | 0.017945 |

The last column is the trap DEC-064 forbids: it improves 6.6x from top to bottom
and ranks the worst vector first. The noise floor is the lambda 0 row's own
wobble, 4e-05 peak to peak over twenty checkpoints with no trend, so lambda 0.25
is just under 2x it and everything from 0.5 up is 10 to 20x. Degradation starts
at the first checkpoint at every non-zero lambda; there is no dip anywhere.
`adocs/data/S075_lambda_sweep.log` and `S075_lambda_fine_grid.log` are the runs.
**No SPRT was spent** — the sweep selected no candidate, which
`adocs/data/S075_lambda_sweep.sh` pre-registered as an outcome before the first
fit, and `adocs/data/S075_sprt.sh` sits unrun for whoever re-asks this.

Why is untested. The available explanation is the paragraph below, and a corpus
generated by today's engine is what would separate "the blend is wrong here" from
"these scores are stale".

**The scores in a corpus are not necessarily this engine's.**
`.tuning/selfplay_v2.tsv` was written at `dd57c3c`, whose evaluator is
`a2f0065`'s to the constant, so its scores predate S065's refit of 827 constants.
That is the only thing keeping `--lambda 1` off the fixed point
`eval_tuning_strategy.md` section 1 describes — training an evaluation toward its
own search at the same depth carries no information. It expires when a corpus is
regenerated by the engine being fitted, which S082 and S083 both do. State the
corpus's generating commit beside the lambda.

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

## Reference engines in `/usr/games`

Not `~/.local/bin`, which on the DEC-049 machine holds no engine at all. What is
in `/usr/games` here: `stockfish`, `sgambetto`, `sgambetto-native`, a fixed
`chesso` (an old build — it exposes only `Use Book`, with no `Hash` or `Threads`
option), and the `rating.sh` reference engines listed in `references.tsv`.

The engines below are the *calibration and cross-check* opponents and are a
different set from the rated ones. For the gauntlet that produces an absolute
CCRL-scale figure, see "Rate the engine against the public lists".

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
