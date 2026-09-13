# Toolchain for agent-assisted development

An AI agent works through a shell. It cannot read an IDE, hover a symbol, or
look at a flame graph in a browser. Everything it knows about this codebase has
to come out of a command as text. These tools are what turn "the change feels
faster" into a number, and they are the difference between an agent that
measures and an agent that guesses.

**This file describes two machines.** The one the project runs on now is the
Linux workstation — Pop!_OS 24.04 on an i7-8700K — and it is the section
immediately below. Everything after it was verified on macOS arm64 (Apple M1,
Apple clang 16 for builds, Homebrew LLVM 22 for analysis) and is labelled as
the MacBook's, kept whole because that machine is still one this repository is
worked from and because DEC-146 holds the two side by side. The gotchas are
recorded because every one of them cost a debugging round to find.

Two later sections are the workstation's wherever they sit in the file, because
that is where the finding was made: "The chess oracle, and the one way to ask
it that lies" and "ThreadSanitizer, and the zero it prints when it never
started". Each says so at its head. A figure does not cross the two machines —
DEC-049 — so a number quoted here always names the machine that printed it.

## The workstation, end to end

The machine in use. Every fact in this section is followed by the command that
printed it, run here on 2026-09-13 while an SPRT held the other cores.

```bash
head -2 /etc/os-release   # NAME="Pop!_OS"   VERSION="24.04 LTS"
uname -srm                # Linux 7.1.5-76070105-generic x86_64
nproc                     # 12
free -h                   # Mem: 15Gi total
lscpu | grep -E "^Model name|^Thread|^Core"
#   Model name:  Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz
#   Thread(s) per core: 2      Core(s) per socket: 6
```

Six physical cores with SMT, not a second core type — which is what DEC-050
decided the concurrency on. The ISA reaches x86-64-v3 and stops: `grep -m1 -o
' avx2\| bmi2\| popcnt' /proc/cpuinfo` prints all three, `grep -c avx512f
/proc/cpuinfo` prints `0`. So `cmake/arch.cmake`'s `bmi2` target
(`-march=x86-64-v3`) is the one that matches this part, and `build_release.sh`
records that `-march=native` here emits AVX2 and BMI2 for the same reason.

### Install

Suffixed LLVM, because that is what the apt line carries. `grep -rhoE
"llvm-toolchain-[a-z]+-[0-9]+" /etc/apt/sources.list.d/` prints
`llvm-toolchain-noble-22` and nothing else, from
`archive_uri-https_apt_llvm_org_noble_-noble.list`:

```bash
sudo apt install g++-13 ccache hyperfine clang-format-22 clang-tidy-22 llvm-22
```

Those are the packages `dpkg -S` names for the binaries: `g++-13`, `ccache`,
`hyperfine`, `clang-format-22`, `clang-tidy-22` (which also ships
`run-clang-tidy-22`) and `llvm-22` (which ships `llvm-mca-22`). `samply` is not
an apt package and lives at `~/.cargo/bin/samply`, which is on PATH here in
both `bash` and `fish` (`command -v samply` resolves it in each).

`stockfish`, `cutechess`, `sgambetto`, `fastchess` and `ordo` are all installed
by hand: `dpkg -S` answers `no path found matching pattern` for every one of
them, so apt will not update them and their versions are the ones below.

> Do not point CMake at `clang++-22`. `g++ 13.3` is the reference compiler here
> and changing it restarts the baseline — DEC-049, which is the local form of
> the Homebrew warning in the MacBook's section.

### Every path, and what printed it

`command -v` for the path, `--version` for the string.

| tool | path here | version, as printed |
|---|---|---|
| `c++`, `g++` | `/usr/bin/c++` → `/etc/alternatives/c++` → `/usr/bin/g++` | `g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0` — the reference compiler, DEC-049 |
| `clang++-22` | `/usr/bin/clang++-22` | `Ubuntu clang version 22.1.8` — a second front end for warnings, never the build compiler |
| `clang-tidy-22`, `run-clang-tidy-22` | `/usr/bin/` | `Ubuntu LLVM version 22.1.8` |
| `llvm-mca-22` | `/usr/bin/llvm-mca-22` | `Ubuntu LLVM version 22.1.8`, default target `x86_64-pc-linux-gnu` |
| `clang-format-22` | `/usr/bin/clang-format-22` | `Ubuntu clang-format version 22.1.8` |
| `clang-format`, unsuffixed | `/usr/bin/clang-format` | `Ubuntu clang-format version 18.1.3` — what the gate resolves without the override, and refuses |
| `lld-22`, `lldb-22`, `llvm-symbolizer-22` | `/usr/bin/` | LLVM 22, same package set |
| `ccache` | `/usr/bin/ccache` | `ccache version 4.9.1` |
| `hyperfine` | `/usr/bin/hyperfine` | `hyperfine 1.18.0` |
| `samply` | `/home/max/.cargo/bin/samply` | `samply 0.13.1` |
| `cmake`, `ninja` | `/usr/bin/` | `cmake version 3.28.3`, ninja `1.11.1` |
| `stockfish` | `/usr/games/stockfish` | `Stockfish dev-20260810-5062aee5` |
| `fastchess` | `/usr/local/bin/fastchess` | `fastchess alpha 1.8.1 20260720-daa3ea2` — the build S105, S087 and S145 measured with |
| `ordo` | `/usr/local/bin/ordo` | `ordo 1.2.6` — `rating.sh` needs it and finds it, so its refusal path does not fire here |
| `cutechess` | `/usr/games/cutechess` | `Cute Chess 1.5.1` on Qt 6.4.2 — **the GUI**, see below |
| `sgambetto` | `/usr/games/sgambetto` | answers nothing to `--version` |
| python | `~/.venv/chess/bin/python` | `Python 3.12.3` with `python-chess 1.11.2`; the system pip is `EXTERNALLY-MANAGED` |
| `nproc`, `timeout`, `sha256sum`, `setarch` | `/usr/bin/` | GNU coreutils and util-linux, native — the MacBook section at the end of this file is about their absence |

Two absences worth naming rather than discovering:

- **`cutechess-cli` is not installed.** `command -v cutechess-cli` finds
  nothing, and `/usr/games/cutechess` is the Qt GUI. `cutechess --version`
  prints and exits; **`cutechess --help` does not return** — it was still
  running after two minutes here and had to be killed. Nothing in the
  repository drives it; `fastchess` is the harness.
- **`perf` is not installed.** `command -v perf` finds nothing. samply is the
  profiler and its own complaint below names only a sysctl, not `perf`.

### Concurrency, and the `-j` counts

`nproc` prints **12**, and DEC-050 settles every parallel tool at 12 here:
`-j12` for builds, concurrency 12 for `fastchess.sh` (which reaches it through
`sysctl -n hw.physicalcpu 2> /dev/null || nproc`), and
`hardware_concurrency()` as the default in `tools/datagen.cpp` and
`tools/tuner.cpp`. `CONCURRENCY=6` is one game per physical core, for a run
that has to be as clean as this machine can make it.

Every command in this section therefore says `-j12`, and every command in the
MacBook's sections says `-j8`. The one deliberate exception is the completion
command itself: `tools/gate.sh` builds with `-j8`, and so does the TESTS rule
in `AGENTS.md` and the copy of it in `DEV_MANUAL.md` — two copies that
`tools/plan_prose_check.py --gate` holds byte-equal. That wording is the gate's
and is not edited to suit a machine.

### The format gate needs `CLANG_FORMAT_MAJOR=22` here (DEC-146)

`clang-format.sh` pins major **23** and the pin has not moved. This machine
cannot install 23 from what it carries, so export the override for every gate
run:

```bash
export CLANG_FORMAT_MAJOR=22
```

Without it the gate is red for a reason that has nothing to do with the tree.
`CLANG_FORMAT_MAJOR=23 ./clang-format.sh --check` exits **1** and prints

```
clang-format 23 not found.
Found, but wrong version:
  /usr/bin/clang-format (18.1.3)
```

— Ubuntu's unsuffixed 18.1.3 being what the search falls through to. DEC-146
measured the two majors formatting this tree identically, which is why the
override is free; it stops being free the day a construct formats differently,
and the arbiter is 23, which this machine cannot run. Every committed document
keeps the number 23.

### ccache

Wired already: `build`, `build-tune`, `build-debug`, `build-prof` and
`build-sanitize` each carry `CMAKE_CXX_COMPILER_LAUNCHER:UNINITIALIZED=ccache`
in their `CMakeCache.txt`, on `CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/c++`. A
directory configured without the launcher gets nothing from an installed
ccache, so check the cache line before trusting a rebuild time.
`ccache --show-stats` here:

```
Cacheable calls:    2480 / 2485 (99.80%)
  Hits:              883 / 2480 (35.60%)
Local storage:
  Cache size (GiB):  1.1 /  5.0 (22.09%)
```

The 0.34 s full rebuild in the MacBook's ccache section is the MacBook's and
has not been re-measured here.

### hyperfine, and the governor it runs under

The interleaving argument is the same on both machines. What is different here
is that the clock policy is readable and is **recorded, never set** — DEC-195:

```bash
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor   # performance
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
#   performance powersave
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_driver     # intel_pstate
```

A run starts under whatever governor is set, the pre-registration records
which, and throughput is budgeted from the figure measured under that
governor. So a `hyperfine` pair records the governor beside its numbers rather
than waiting for one, and two figures taken under different governors are not
compared without saying so.

### samply — no `dsymutil`, one sysctl, and it is off by default

Linux samply reads DWARF out of the binary, so the MacBook's two silent
failures (`dsymutil` not run, `--unstable-presymbolicate` missing) are not
failures here. What replaces them is a kernel setting, and it is **not** set on
this machine as it stands:

```bash
sysctl kernel.perf_event_paranoid      # kernel.perf_event_paranoid = 2
```

`samply record --save-only -o /tmp/x.json.gz -- /bin/true` therefore records
nothing at all and says so:

```
'/proc/sys/kernel/perf_event_paranoid' is currently set to 2.
In order for samply to work with a non-root user, this level needs
to be set to 1 or lower.
```

It is loud rather than silent, which is the one mercy. Lower it first, and it
resets at reboot:

```bash
sudo sysctl kernel.perf_event_paranoid=1
```

Then the recipe is the MacBook's minus the `dsymutil` line:

```bash
cmake -S . -B build-prof -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -DNDEBUG -g -fno-omit-frame-pointer"
cmake --build build-prof -j12

samply record --save-only -r 2000 -o /tmp/prof.json.gz \
  -- ./build-prof/tests/bench_movegen

tools/samply_report.py /tmp/prof.json.gz
```

### clang-tidy-22

No `xcrun`, no `-isysroot`: the SDK problem is Apple's. `build/compile_commands.json`
exists here, so `-p build` is the whole configuration. There is no `.clang-tidy`
in the tree, so a check set has to be given or the run means nothing:

```bash
clang-tidy-22 -p build --quiet --checks='-*,bugprone-*' src/bitboard.cpp
```

Whole project in parallel — `run-clang-tidy-22 --help` confirms its options
take **one** dash each, `-p`, `-j`, `-quiet`, `-checks`, `-extra-arg`:

```bash
run-clang-tidy-22 -p build -j 12 -quiet -checks='-*,bugprone-*' 'src/.*'
```

### llvm-mca-22 — and here the numbers are real

The MacBook's section ends by saying to treat `llvm-mca` as directional only,
because LLVM's models for Apple cores are approximate. **That caveat does not
apply on this machine.** Coffee Lake is Skylake-derived and LLVM has a real
scheduling model for it, so `-mcpu=skylake` gives numbers worth reading:

```bash
c++ -std=gnu++20 -O3 -DNDEBUG -march=x86-64-v3 -I src -S -o /tmp/x.s src/bitboard.cpp

awk '/LLVM-MCA-BEGIN/{f=1;next} /LLVM-MCA-END/{f=0} f' /tmp/x.s \
  | grep -vE '^\s*\.' > /tmp/region.s

llvm-mca-22 -mtriple=x86_64-pc-linux-gnu -mcpu=skylake -iterations=100 /tmp/region.s
```

The first two lines are the MacBook recipe with this machine's triple and arch
flag. The third was run here on a hand-written three-instruction region
(`addq`, `blsrq`, `popcntq`) and printed

```
Iterations:        100
Instructions:      300
Total Cycles:      107
Total uOps:        300

Dispatch Width:    6
uOps Per Cycle:    2.80
IPC:               2.80
Block RThroughput: 1.0
```

A dispatch width of 6 is Skylake's, so it is that model answering and not a
generic one. The `asm volatile` marker rule and the extracted-region rule from
the MacBook's section both still hold — feed `llvm-mca` the region, never a
whole assembly file, and never ship the markers.

### The tablebase, and the venv that reads it

`ls ~/syzygy | wc -l` prints **290** files and `du -sh ~/syzygy` prints `939M`.
That is 145 tables, 3-4-5 exactly and nothing above it:

```bash
ls ~/syzygy | sed 's/\..*//' | sort -u \
  | awk '{n=gsub(/[KQRBNP]/,"&"); print n}' | sort -n | uniq -c
#       5 3
#      30 4
#     110 5
```

The engine itself does not probe them — `grep -rli syzygy src/` finds nothing,
and S129 in `adocs/plan_todo/` is the step that would — so this is the oracle's
tablebase, for the endgame question `CLAUDE.md` forbids answering from memory:

```bash
~/.venv/chess/bin/python -c '
import chess, chess.syzygy
with chess.syzygy.open_tablebase("/home/max/syzygy") as tb:
    b = chess.Board("8/8/8/8/8/8/R7/K6k w - - 0 1")
    print("wdl", tb.probe_wdl(b), "dtz", tb.probe_dtz(b))'
```

```
wdl 2 dtz 13
```

Those two integers are `python-chess`'s own encoding and what they mean is its
documentation. Reading a verdict out of them by eye is the thing `CLAUDE.md`
forbids; the probe is the answer, not the start of one.

## Install, on the MacBook

```bash
brew install hyperfine ccache samply llvm
```

Homebrew LLVM is keg-only. Put it on PATH *after* the system toolchain so
`clang-tidy` and `llvm-mca` resolve while `c++` stays Apple clang:

```fish
fish_add_path --append /opt/homebrew/opt/llvm/bin
```

Check it took:

```bash
command -v clang-tidy llvm-mca   # /opt/homebrew/opt/llvm/bin/...
command -v c++                   # must stay /usr/bin/c++
```

> Do not point CMake at Homebrew clang. Different compiler, different codegen,
> and every recorded benchmark becomes incomparable.

## What each one is for

| tool | answers | without it |
|---|---|---|
| `hyperfine` | is B actually faster than A | eyeballing interleaved runs, 3 % noise swamps the result |
| `ccache` | rebuild cost | ~25 s per ablation instead of 0.3 s (the MacBook's figures; the workstation's are in its section) |
| `samply` + `tools/samply_report.py` | where the time goes, per function | guessing which function to optimise |
| `clang-tidy` | narrowing, sign and lifetime bugs | they surface later as a wrong perft count |
| `llvm-mca` | is this loop front-end or dependency bound | hand-tuning blind |
| `ctest`, `bench_movegen` | correctness, then speed | everything else is meaningless |
| `stockfish` and `python-chess` | every chess question `CLAUDE.md` forbids the agent to answer itself | a confident wrong answer, DEC-023. **Driven wrong it also gives one** -- see the chess-oracle section below |

## ccache, and how the MacBook was wired

Wire it into every build directory. Installed but unwired does nothing:

```bash
cmake -S . -B build       -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build-debug -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build-prof  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

Measured on the MacBook: full rebuild 0.34 s at a 100 % hit rate. The
workstation's cache is already wired and its `ccache --show-stats` is in the
section above; the 0.34 s has not been re-measured there.

## hyperfine, on the MacBook

The engine benchmark reports its own best-of-N sweep. Let hyperfine own the
repetition instead, and give it two binaries so the runs interleave and machine
drift cancels:

```bash
hyperfine --warmup 1 --runs 10 \
  './bench_before -r 1' './bench_after -r 1'
```

Interleaving matters more than the run count. A before/after pair measured
minutes apart on the MacBook can disagree by 3 % from thermal drift alone. The
workstation has the same problem with a different cause — see its governor
note above.

## samply, on the MacBook

samply targets the Firefox Profiler UI, which is useless in a terminal.
`tools/samply_report.py` reads the profile and prints self time per function.

```bash
cmake -S . -B build-prof -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -DNDEBUG -g -fno-omit-frame-pointer"
cmake --build build-prof -j8

# macOS keeps debug info in the .o files until dsymutil collects it
dsymutil build-prof/tests/bench_movegen

samply record --save-only --unstable-presymbolicate \
  -r 2000 -o /tmp/prof.json.gz -- ./build-prof/tests/bench_movegen

tools/samply_report.py /tmp/prof.json.gz
```

Two failure modes, both silent:

- **No `dsymutil` run** → every frame prints as a bare hex address.
- **No `--unstable-presymbolicate`** → no `.syms.json` sidecar, same result.

macOS `sample <pid>` is the zero-setup fallback. It gives function names but no
line numbers and no call counts.

## clang-tidy, on the MacBook

Homebrew clang-tidy does not know where Apple's SDK headers live, so it fails
on `#include <bit>` unless told:

```bash
clang-tidy -p build --extra-arg=-isysroot"$(xcrun --show-sdk-path)" src/bitboard.cpp
```

Whole project, in parallel — note `run-clang-tidy` takes **one** dash on
`-extra-arg`, unlike `clang-tidy`:

```bash
run-clang-tidy -p build -j 8 -quiet \
  -extra-arg=-isysroot"$(xcrun --show-sdk-path)" 'src/.*'
```

`compile_commands.json` comes from the build directory, so configure before
running.

## llvm-mca, on the MacBook

Mark the hot region in the source, emit assembly with the real build flags,
extract the region, analyse it:

```cpp
asm volatile("# LLVM-MCA-BEGIN pawn_push_loop");
// ... the loop under test ...
asm volatile("# LLVM-MCA-END");
```

```bash
c++ -std=gnu++20 -O3 -DNDEBUG -arch arm64 -I src -S -o /tmp/x.s src/bitboard.cpp

awk '/LLVM-MCA-BEGIN/{f=1;next} /LLVM-MCA-END/{f=0} f' /tmp/x.s \
  | grep -vE '^\s*\.' > /tmp/region.s

llvm-mca -mtriple=arm64-apple-darwin -mcpu=apple-m1 -iterations=100 /tmp/region.s
```

The `grep -v` is not cosmetic: **llvm-mca crashes** on a full Apple-clang Mach-O
assembly file over directives like `.subsections_via_symbols`. Feed it only the
extracted region. If the region branches to a label outside itself, add
`-skip-unsupported-instructions=parse-failure`.

Two limits worth stating plainly:

- The `asm volatile` markers are optimisation barriers. Measure with them,
  never ship them.
- LLVM's scheduling models for Apple cores are approximate — Apple publishes no
  port layout or latency tables. On the MacBook treat `llvm-mca` as
  directional only (front-end bound vs. dependency bound), never as a number.
  It becomes trustworthy on x86 (`-mcpu=znver4`, `-mcpu=skylake-avx512`), which
  is the workstation's case and why its `llvm-mca-22` section reads the numbers
  rather than only their direction.

## clang-format, on both machines

The version is pinned, because clang-format changes its output between major
versions and an unpinned formatter rewrites files nobody touched.

```bash
./clang-format.sh           # format in place
./clang-format.sh --check   # dry run, non-zero if anything is unformatted
```

The script requires **major version 23** and refuses to run on anything else
rather than silently reformatting the tree. Override for a different toolchain:

```bash
CLANG_FORMAT_MAJOR=15 ./clang-format.sh --check
```

On macOS it finds Homebrew's keg-only binary at
`/opt/homebrew/opt/llvm/bin/clang-format` even when that is not on PATH. On
Ubuntu it looks for `clang-format-23`, which the workstation cannot install —
DEC-146, and the override it needs is in that machine's section above.

## The chess oracle, and the one way to ask it that lies

**This section is the workstation's**, not the MacBook's: every path in it
(`/usr/games/stockfish`, `~/.venv/chess/bin/python`) is that machine's, and
every invocation below was re-run there on 2026-09-13 against
`Stockfish dev-20260810-5062aee5`. They print what is written here, to the
node — the re-verification is at the end of the section.

`CLAUDE.md` forbids the agent from judging a position, a move, an ending or a
material balance from its own reasoning, and names a tool per question. This
section is about the tool, because the obvious way to drive it is silently
wrong.

**The trap.** Stockfish searches on its own thread and `quit` stops it. A pipe
delivers every line at once, so `quit` arrives the instant `go` is written and
the search is killed before it has looked at a node:

```bash
printf 'position fen 7k/6pp/8/8/8/8/8/R6K w - - 99 60\ngo depth 20\nquit\n' | stockfish
```

```
info depth 1 seldepth 0 multipv 1 score cp 0 nodes 0 nps 0 hashfull 0 tbhits 0 time 1 pv
bestmove a1b1
```

A **draw score and a non-mating move for a position that is mate in one**. The
same pipe with `position startpos` and `go depth 8` answers `bestmove a2a3` with
no depth-8 line at all. Note `nodes 0` and `depth 1`: nothing was searched, and
the reply is the first root move. It is not a matter of giving it longer — the
correct search of that position takes **320 nodes and 1 ms**, so any search
loses this race however short it is.

This is the failure mode the rule exists to prevent, arriving through the
instrument the rule names: silent, confidently specific, and in the one tool the
agent is supposed to defer to. Found while doing S162.

**Chesso has the same shape**, so a chesso pipe read this way lies too:
`command_quit` and `command_position` both call `stop_and_join_search()`
(`src/chesso.cpp`), which is why a piped `position`/`go` loop aborts every
search but the last. S165 read 1322 NMP-eligible nodes over 400 positions that
way and 301620 with a driver that waits for `bestmove` — a factor of 228.

**And that half was already written down**, which is the sharpest thing about
this finding: `DEV_MANUAL.md`'s tune-build section has carried "Wait for
`bestmove` when you script it" with its own 283-node example since S073. The
trap was known for the engine being *measured* and unwritten for the oracle
being *asked* — and the oracle is the one the agent is forbidden to
second-guess, so a wrong answer from it is the one that survives. Two documents,
one race, and the gap between them is what S162 walked into.

**What is safe. 1. `python-chess`**, which reads until the search answers:

```bash
~/.venv/chess/bin/python -c '
import chess, chess.engine
with chess.engine.SimpleEngine.popen_uci("/usr/games/stockfish") as e:
    info = e.analyse(chess.Board("7k/6pp/8/8/8/8/8/R6K w - - 99 60"),
                     chess.engine.Limit(depth=20))
    print(info["score"].relative, info["depth"], info["nodes"], info["pv"][:1])'
```

```
#+1 20 320 [Move.from_uci('a1a8')]
```

**2. A shell pipe that does not send `quit` until the search has answered.**
Correct, and worth having because it needs no python:

```bash
{ printf 'position fen 7k/6pp/8/8/8/8/8/R6K w - - 99 60\ngo depth 20\n'; sleep 3; echo quit; } | stockfish
```

```
info depth 20 seldepth 2 multipv 1 score mate 1 nodes 320 nps 320000 hashfull 0 tbhits 0 time 1 pv a1a8
bestmove a1a8
```

The `sleep` is a guess at how long the search takes and that is its weakness:
too short and it is the trap again, only less obviously. Prefer python-chess for
anything whose cost is not known. An interactive session has no race at all,
because the next line is not typed until the reply is on screen.

**3. What the repository already does correctly**, and the first thing to reach
for rather than writing a new driver:

| what | tool |
|---|---|
| a game's per-move cost against a reference | `tools/analyse_game.py`. Raw `subprocess`, not `python-chess`, and correct for the reason that matters: `evaluate()` reads until `bestmove` and `close()` is the only thing that sends `quit` |
| SAN move list to FENs, parsed by the engine and not by hand | `build/tools/pgn_to_positions` |
| a proved mate distance, no engine involved | `adocs/data/S145_mate_set.py`, exhaustive AND/OR enumeration over `python-chess` |
| a mate set measured from the defender's side | `adocs/data/S165_nmp_defender_sweep.py` |

**Two further traps in the oracle itself**, both paid for once and recorded in
`adocs/plan_done/S145_mate_safety_test_set.md`. A node-limited stockfish is
reproducible only inside one identical call sequence, so a loop that reuses one
process is not comparable to one that does not. And a frozen defending army is a
position class its network scores badly wrong — on S145's constructed set it
reported no mate on 1 of 48 and a longer mate on another, both re-proved by
enumeration, which is why it corroborates there and never decides.

**Re-verified on the workstation, 2026-09-13**, S226, on
`Stockfish dev-20260810-5062aee5` at the depths written above. The trap still
loses the race:

```
info depth 1 seldepth 0 multipv 1 score cp 0 nodes 0 nps 0 hashfull 0 tbhits 0 time 1 pv
bestmove a1b1
```

and the `position startpos` / `go depth 8` variant of the same pipe still
answers `bestmove a2a3`. Both safe forms print what they printed when this was
written. python-chess:

```
#+1 20 320 [Move.from_uci('a1a8')]
```

and the pipe that sleeps before `quit`:

```
info depth 20 seldepth 2 multipv 1 score mate 1 nodes 320 nps 320000 hashfull 0 tbhits 0 time 1 pv a1a8
bestmove a1a8
```

Same node count, same depth, same move, three weeks on. It is the same
workstation and the same binary S166 wrote the section from — `/usr/games/stockfish`
has not been touched since 2026-08-13 — so this confirms the section is current
and its paths still resolve, and claims nothing about other builds or other
machines.

The binary paths above are this machine's. `.moltke.local.md` is where they are
recorded per machine; the tools in the table take the engine as an argument and
hardcode nothing.

## ThreadSanitizer, and the zero it prints when it never started

**This section is the workstation's too.** `setarch` is util-linux and exists
only there, and S209 met the failure below on that machine on 2026-09-11.

`CMakeLists.txt`'s `SANITIZER` option is ASan plus UBSan, and `build-sanitize`
is that tree. **TSan cannot join it**: ASan and TSan cannot be linked into one
binary, so a thread-race check is a fourth build and not a flag on the third,
and it is configured by hand:

```bash
cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer -g" \
      -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
cmake --build build-tsan -j12 --target chesso
```

**The trap, and it reads as a clean result.** On this kernel TSan's shadow
mapping collides with ASLR at random, and when it does the process dies before
`main` with one line on stderr —

```
FATAL: ThreadSanitizer: unexpected memory mapping 0x…-0x…
```

— and a harness that counts `WARNING: ThreadSanitizer` lines reports **0
reports**, which is what a fixed engine also reports. S209 met this on its
first run: 0 on a tree whose defect was still in it, then 38, 41 and 36 on the
next three runs of the same binary. Run it under `setarch -R`, which disables
randomization for that process, and grep the log for `FATAL:` before believing
any count:

```bash
setarch -R ./build-tsan/src/chesso < commands.txt 2> tsan.log
grep -q "FATAL: ThreadSanitizer" tsan.log && echo "the count below means nothing"
grep -c "WARNING: ThreadSanitizer" tsan.log
```

A report count is a measurement like any other, so it needs its A/B: the same
build tree with one line changed, not two trees built differently. S209's race
went 38/41/36 to 0/0 by adding `stop_and_join_search()` to
`src/chesso.cpp` `command_clean_TT` in the tree that had just produced the
reports. DEC-178.

## The rule the tools exist to serve

Node counts first, timings second. A change under 3 % has not been shown to do
anything on either machine unless `hyperfine` says otherwise with a tight
sigma. One change at a time — two at once and neither number means anything.

## GNU coreutils on macOS

None of this applies to the workstation: `nproc`, `timeout`, `sha256sum` and
`setarch` are all at `/usr/bin/` there, `ordo 1.2.6` is at `/usr/local/bin/`,
and `uname -srm` answers `x86_64`, so the arm64 refusal in the last paragraph
does not arise. It is the MacBook that needs the rest of this section.

`nproc`, `timeout` and `sha256sum` are GNU coreutils and are not on a stock
Mac. The scripts here do not assume them: `fastchess.sh` (S167), `rating.sh`
and `build_release.sh` (S177) count cores with `sysctl -n hw.physicalcpu`
(`hw.logicalcpu` for build jobs) and fall back to `nproc`; `books/fetch_book.sh`
uses `shasum -a 256` (S024). `rating.sh` drives every reference engine through
a GNU `timeout` and has no substitute for it, so on a Mac it needs

```bash
brew install coreutils    # installs it as gtimeout; rating.sh takes either name
```

and refuses with `RATING-RUN-FAILED: neither timeout nor gtimeout on PATH ...`
before asking any engine anything. It needs `ordo` too, refused the same way.
`tests/test_rating_script.sh` holds both refusals on a PATH with none of these
tools, so the check does not depend on what the machine running the suite has.
`build_release.sh`'s three targets are x86-64 and `cmake/arch.cmake` refuses them
on arm64 by design (DEC-112).
