id:         S226
goal:       `TOOLCHAIN.md` describes the machine the project now runs on -- the Pop!_OS 24.04 workstation `.moltke.local.md` describes -- beside the MacBook it was written for, so an agent following it lands on tools that exist here
accepts:    `TOOLCHAIN.md` gains a section for the Linux workstation covering every tool the macOS sections cover (install lines for apt and the LLVM 22 suffixed names, `ccache`, `hyperfine`, `samply` at `~/.cargo/bin` with `kernel.perf_event_paranoid`, `clang-tidy-22` with the `build/` compile database, `llvm-mca-22 -mcpu=skylake`, `clang-format-22` under the DEC-146 override, `stockfish` at `/usr/games`, `fastchess alpha 1.8.1 20260720-daa3ea2` at `/usr/local/bin`, the python-chess venv, Syzygy under `~/syzygy/`, the cpufreq governor and DEC-195's ruling), every statement traced to a command run on this machine during the step and quoted, the `-j` counts stated as 12 here and 8 on the MacBook, the macOS sections kept whole and labelled as the MacBook's; "The chess oracle, and the one way to ask it that lies" checked on this machine's `stockfish` and its two safe invocations re-verified; no `src/`, no test change, `python3 tools/plan_prose_check.py --prose` and `--citations` exit 0
touches:    TOOLCHAIN.md
excludes:   `.moltke.local.md`, which stays machine-local and uncommitted; `DEV_MANUAL.md`, whose `-j8` is the TESTS rule's wording (DEC-146 records the machine's 12); any tool not installed here
decisions:  DEC-049, DEC-050, DEC-146, DEC-195
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-13 08:25 beside S109's SPRT under the blocked-task exception, documents only
done:       2026-09-13 08:29. **`TOOLCHAIN.md` now opens on the machine it runs on.** A new
            `## The workstation, end to end` sits directly after the intro with eleven
            subsections mirroring the file's tool order -- identity, Install, the path
            table, concurrency and `-j`, the format-gate override, ccache, hyperfine and
            the governor, samply, clang-tidy-22, llvm-mca-22, the tablebase. Every
            statement carries the command that printed it, all run 2026-09-13 on one core
            beside S109's SPRT; no build, no test binary, no match.
            **Probed and quoted:** `head -2 /etc/os-release` -> `NAME="Pop!_OS"` /
            `VERSION="24.04 LTS"`; `uname -srm` -> `Linux 7.1.5-76070105-generic x86_64`;
            `nproc` -> `12`; `lscpu` -> i7-8700K, 6 cores x 2 threads; `free -h` -> 15Gi;
            avx2/bmi2/popcnt present in `/proc/cpuinfo` and `avx512f` absent. Versions as
            printed: `g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0` (DEC-049),
            `Ubuntu clang version 22.1.8`, `Ubuntu clang-format version 22.1.8` against
            the unsuffixed `18.1.3`, `ccache version 4.9.1`, `hyperfine 1.18.0`,
            `samply 0.13.1`, `cmake version 3.28.3`, ninja `1.11.1`,
            `Stockfish dev-20260810-5062aee5`, `fastchess alpha 1.8.1 20260720-daa3ea2`,
            `ordo 1.2.6`, `Cute Chess 1.5.1`, `Python 3.12.3` with `python-chess 1.11.2`.
            Governor read, never set (DEC-195): `performance`, of
            `performance powersave`, driver `intel_pstate`. `~/syzygy` is 290 files /
            939M, counted 5 three-man, 30 four-man, 110 five-man, none above, and a
            `chess.syzygy` probe prints `wdl 2 dtz 13`. DEC-050's 12 is stated for every
            command in the new section and the MacBook's `-j12` occurrences moved to 8.
            **Three findings the old text did not have.** `sysctl
            kernel.perf_event_paranoid` is **2** here, so `samply record` refuses outright
            and prints `'/proc/sys/kernel/perf_event_paranoid' is currently set to 2` --
            samply cannot profile this machine as it stands. `cutechess-cli` is **not
            installed**; `/usr/games/cutechess` is the Qt GUI and `cutechess --help`
            never returned (killed after two minutes). `perf` is not installed either.
            `CLANG_FORMAT_MAJOR=23 ./clang-format.sh --check` exits **1** with
            `clang-format 23 not found. / Found, but wrong version: /usr/bin/clang-format
            (18.1.3)`, which is DEC-146's override reproduced from the failing side.
            `llvm-mca-22 -mcpu=skylake` was run on a three-instruction region and printed
            `Dispatch Width: 6`, `IPC: 2.80`, `Block RThroughput: 1.0` -- real numbers,
            which is why the Apple "directional only" caveat is now labelled the
            MacBook's.
            **The oracle section re-verified on this machine**, and relabelled as the
            workstation's along with the ThreadSanitizer section, which needs `setarch`
            and so cannot be the MacBook's. The trap still loses the race:
            `info depth 1 seldepth 0 multipv 1 score cp 0 nodes 0 nps 0 ... time 1 pv` /
            `bestmove a1b1`, and the `position startpos` + `go depth 8` variant still
            answers `bestmove a2a3`. Both safe forms are byte-identical to what the file
            carried: python-chess prints `#+1 20 320 [Move.from_uci('a1a8')]` and the
            sleep-then-`quit` pipe prints `info depth 20 seldepth 2 multipv 1 score mate 1
            nodes 320 nps 320000 ... pv a1a8` / `bestmove a1a8`. Same binary S166 used
            (`/usr/games/stockfish` untouched since 2026-08-13), so this dates the section
            and claims nothing about other builds. No chess judgement added anywhere.
            **Checks:** `python3 tools/plan_prose_check.py --prose` exits 0 (0 sentences
            flagged) and `--citations` exits 0 (0 flagged over 55 files), each run on its
            own. Only `TOOLCHAIN.md` and this file changed; `.moltke.local.md`,
            `DEV_MANUAL.md` and the `adocs/` shared documents were not touched.
            **DOCS checked, neither needs a change.** `MANUAL.md` is the UCI surface and
            names no tool, path or machine -- a grep for `TOOLCHAIN`, `macOS`, `Homebrew`,
            `workstation`, `MacBook`, `/usr/games` and `clang-format` finds nothing in it.
            `DEV_MANUAL.md` already carries the workstation's suffixed LLVM names and its
            `-j8` is the gate's wording, which this step's `excludes` keeps.
            **Two inconsistencies found and left for the coordinator**, both outside this
            step's `touches`: `.moltke.local.md` says `~/.cargo/bin` is "not on `PATH` by
            default" where `command -v samply` resolves it in both `bash` and `fish`; and
            `DEV_MANUAL.md` names `fastchess alpha 1.8.2 20260729-74deac2` where this
            machine runs `1.8.1 20260720-daa3ea2`, the build `adocs/plan_done/S105_sprt_harness_regime.md`,
            `adocs/plan_done/S087_absolute_elo_rating.md` and `adocs/plan_done/S145_mate_safety_test_set.md`
            all record.

## Why this exists

`.moltke.local.md`'s Open item since the move to this machine: "TOOLCHAIN.md is
macOS throughout (Homebrew paths, `dsymutil`, `xcrun`, `-mcpu=apple-m1`) and
is stale here -- the `-j` counts in it are now 12, the rest is not touched."
Fourteen macOS-specific lines and no Linux section. Every agent this session
was told the machine facts in its brief instead, which is the transcript
carrying what the repository should. Filler beside S109's SPRT: documents
only, one core, nothing built.

## Cost

Agent work, an hour or two; the commands it quotes run in seconds each.
