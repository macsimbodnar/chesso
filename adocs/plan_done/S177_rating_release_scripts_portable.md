id:         S177
goal:       `rating.sh` and `build_release.sh` run on macOS as `fastchess.sh` does, and `rating.sh` arms its terminal marker before anything can fail
accepts:    `rating.sh:50` and `build_release.sh:99`, `build_release.sh:163` take the core count the portable way `fastchess.sh:170` already does (`sysctl -n hw.physicalcpu 2>/dev/null || nproc`); `rating.sh` resolves its timeout command once -- `timeout`, else `gtimeout`, else a stated refusal -- instead of assuming GNU `timeout` at `rating.sh:110`; the `RATING-RUN-*` trap at `rating.sh:139` is armed before the first command that can fail, as `fastchess.sh:91-97` has been since S167, so a detached run prints exactly one terminal marker on every exit path; on this machine `./rating.sh --bracket` no longer dies with `nproc: command not found` (exit 127) and no marker -- observed red before, and after the change it either proceeds or exits through the trap with a `RATING-RUN-FAILED` line naming what is missing; `bash -n` passes on both scripts and `test_fastchess_script`'s approach is extended to `rating.sh` where a static check can hold the marker-before-failure property; `.moltke.local.md` (machine-local, uncommitted) names which scripts run on this machine and which do not; `DEV_MANUAL.md` and `TOOLCHAIN.md` checked -- `TOOLCHAIN.md`'s "macOS throughout" is made true of these two scripts or says where it is not
touches:    rating.sh, build_release.sh, tests/ (script check), DEV_MANUAL.md, TOOLCHAIN.md, .moltke.local.md
excludes:   making `build_release.sh`'s three x86-64 targets buildable on arm64 -- DEC-112 scopes release builds to the workstation and `cmake/arch.cmake:57-62` refuses them here by design; running any rating match; changing `rating.sh`'s regime (opponents, hash, time control, `ordo`); installing `ordo` or GNU coreutils
decisions:
closes:     2026-09-03_adversarial-F03
blocks:
paused_by:
author:     claude (Fable 5.1), 2026-09-04
done:       2026-09-04. `rating.sh`: `fail()` and the EXIT trap now come first, with a `marked` flag instead of `$?` (bash 3.2 reads `$? == 0` on a `set -u` abort); cores are `sysctl -n hw.physicalcpu`, else `nproc`, else a named refusal; the GNU `timeout` `identify()` needs is resolved once as `timeout` or `gtimeout` and refused by name before any engine is probed; the `command -v x || fail` checks are `if`s; the three DONE sites mark themselves and the snapshot is removed by the one trap. `build_release.sh`: both `-j` core counts take the `sysctl -n hw.logicalcpu || nproc` form. Red observed first: `./rating.sh --bracket` on this machine exited 127 at `nproc: command not found` with 0 markers; `tests/test_rating_script.sh` (sandbox, PATH holding only the utilities it links in) failed all five properties on the old scripts. Green after: the test passes; on this machine the real script answers `RATING-RUN-FAILED: ordo not on PATH`, exit 1, one marker, no snapshot left. `bash -n` passes on both scripts. Fast suite 27/27 in `build` and `build-tune`, `clang-format --check` clean; no engine binary changed, so no node-count check is owed. `DEV_MANUAL.md` (concurrency table row and paragraph, rating section) and `TOOLCHAIN.md` (new section "GNU coreutils on macOS") updated; `.moltke.local.md` lists which scripts run here. Two test-side corrections while writing it, not to the scripts: `true` is a builtin so stubs are scripts rather than symlinks, and the bare-`nproc` check skips comment lines because both scripts now name the old form in a comment. Fast check over the diff: no issue in the change; it flagged that `DEV_MANUAL.md`'s watcher regex also matches the non-terminal `RATING-RUN-INVALID:` line, which predates this step and is harmless because that line is always followed by its `RATING-RUN-DONE <mode> INVALID` marker in the same block.

## Why this exists

`2026-09-03_adversarial-F03`, low. `./rating.sh --bracket` on this machine:

```
./rating.sh: line 50: nproc: command not found
exit=127
```

`set -euo pipefail` ends the script at `all_cores="$(nproc)"`, before the
`RATING-RUN-*` trap at `rating.sh:139` is armed, so a detached run prints no
terminal marker at all -- the class 2026-08-13_adversarial-F01 and S167 removed
for `fastchess.sh`, where a script that dies with no marker under the WATCHERS
rule leaves a watcher spinning to its ceiling. `rating.sh:110` drives every
reference engine through `timeout -k 1 5`, also absent here, and
`build_release.sh` uses `nproc` twice. Only `fastchess.sh` received the S167
portability; `adocs/specs.md:32-33` says `./rating.sh` re-derives the rating,
and nothing in `.moltke.local.md` or `TOOLCHAIN.md` says the two scripts cannot
run on this machine.

## Impact

No measurement is affected today: DEC-112 scopes the rating runs and the
release builds to the workstation. The exposure is a documented command that
fails at once on the machine the project is on, and a WATCHERS-rule hole for
whenever it is next armed.
