# Toolchain for agent-assisted development

An AI agent works through a shell. It cannot read an IDE, hover a symbol, or
look at a flame graph in a browser. Everything it knows about this codebase has
to come out of a command as text. These tools are what turn "the change feels
faster" into a number, and they are the difference between an agent that
measures and an agent that guesses.

All commands below are verified working on macOS arm64 (Apple M1, Apple clang
16 for builds, Homebrew LLVM 22 for analysis). The gotchas are recorded because
every one of them cost a debugging round to find.

## Install

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
| `ccache` | rebuild cost | ~25 s per ablation instead of 0.3 s |
| `samply` + `tools/samply_report.py` | where the time goes, per function | guessing which function to optimise |
| `clang-tidy` | narrowing, sign and lifetime bugs | they surface later as a wrong perft count |
| `llvm-mca` | is this loop front-end or dependency bound | hand-tuning blind |
| `ctest`, `bench_movegen` | correctness, then speed | everything else is meaningless |

## ccache

Wire it into every build directory. Installed but unwired does nothing:

```bash
cmake -S . -B build       -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build-debug -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build-prof  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

Measured here: full rebuild 0.34 s at a 100 % hit rate.

## hyperfine

The engine benchmark reports its own best-of-N sweep. Let hyperfine own the
repetition instead, and give it two binaries so the runs interleave and machine
drift cancels:

```bash
hyperfine --warmup 1 --runs 10 \
  './bench_before -r 1' './bench_after -r 1'
```

Interleaving matters more than the run count. A before/after pair measured
minutes apart on this machine can disagree by 3 % from thermal drift alone.

## samply

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

## clang-tidy

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

## llvm-mca

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
  port layout or latency tables. On this machine treat `llvm-mca` as
  directional only (front-end bound vs. dependency bound), never as a number.
  It becomes trustworthy on x86 (`-mcpu=znver4`, `-mcpu=skylake-avx512`).

## clang-format

The version is pinned, because clang-format changes its output between major
versions and an unpinned formatter rewrites files nobody touched.

```bash
./clang-format.sh           # format in place
./clang-format.sh --check   # dry run, non-zero if anything is unformatted
```

The script requires **major version 22** and refuses to run on anything else
rather than silently reformatting the tree. Override for a different toolchain:

```bash
CLANG_FORMAT_MAJOR=15 ./clang-format.sh --check
```

On macOS it finds Homebrew's keg-only binary at
`/opt/homebrew/opt/llvm/bin/clang-format` even when that is not on PATH. On
Ubuntu it looks for `clang-format-22`.

## The rule the tools exist to serve

Node counts first, timings second. A change under 3 % has not been shown to do
anything on this machine unless `hyperfine` says otherwise with a tight sigma.
One change at a time — two at once and neither number means anything.

