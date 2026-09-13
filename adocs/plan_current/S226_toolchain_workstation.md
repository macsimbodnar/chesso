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
done:

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
