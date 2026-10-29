id:         S298
goal:       `tools/gate_extra.sh` gains a clang stage, so a compile break that only clang reports is found weekly on the workstation instead of on the owner's Mac
accepts:    the new stage configures `build-clang` (Release, with `-stdlib=libc++` as the Mac's standard library) and `build-clang-tune` (`CHESSO_TUNE=ON`) with `clang++-22` through the script's existing configure-and-verify path, and refuses a cache whose compiler is not clang, as the sanitize stage refuses its own mismatch; it builds every target and runs both fast suites; a planted clang-only warning is observed to turn the stage red before it is called done, and the plant is reverted; the stage's wall time is measured and stated in `DEV_MANUAL.md` beside the other stages
touches:    tools/gate_extra.sh, DEV_MANUAL.md
excludes:   adding clang to `tools/gate.sh` (every commit); changing any warning flag
decisions:  DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why

S297 found the owner's macOS build broken by a warning that GCC 13 does not
emit and clang does, where the workstation's gate is GCC only. S297's agent
proposed a weekly stage. Its cost is two clang builds (seconds with a warm
cache) and two fast suites of about two minutes each. The coordinator took the
proposal under the owner's standing instruction to choose the recommended
option where it can (2026-10-27). Filler, behind the next strength step.
