id:         S035
goal:       restore fastchess.sh so a match actually runs, and stop the EXIT trap masking a failure as status 0
accepts:    fastchess.sh reaches the fastchess invocation with a stubbed fastchess on PATH; a script aborting before that point exits non-zero; the smoke run is a test in the suite and was observed failing at 44877c4
touches:    fastchess.sh
excludes:   the concurrency policy of DEC-048, the tc defaults, and anything about how a verdict is read
decisions:  DEC-048
closes:     2026-08-13_adversarial-F01
blocks:
paused_by:
done:      2026-08-13. fastchess.sh runs again. Three defects, not the one F01 named: `mktemp -t chesso-candidate` is a BSD-ism GNU mktemp rejects, which killed the script at line 77 before F01's line 108 was ever reached; `$perf_cores` left behind by 44877c4, now `concurrency $concurrency of $all_cores cores`; and `sysctl -n hw.logicalcpu` on the reference-build path, given the `|| nproc` fallback line 44 already had. The EXIT trap is hardened to `status=$?; rm -f; exit $status`, but F01's exit-status-0 masking does NOT reproduce under bash 5.2.21 -- the same abort exits 1 here -- so that half is recorded as unverifiable on this machine rather than as a fix seen working. Guard: tests/test_fastchess_script.sh, first test in the suite to exercise a shell script, 0.09 s, fast label, stub fastchess on PATH inside a throwaway git repository so .ref-builds/ and build/ are untouched. Red observed twice at 44877c4: as committed (mktemp abort) and with only mktemp repaired (perf_cores unbound, the audit's evidence exactly). Beyond the accepts, a bounded real match was run: reference 7b4d9a4 built into its worktree, tc 10+0.2 concurrency 12 of 12 cores, 30 games finished against the real fastchess binary, then killed -- no verdict read, the candidate carries uncommitted changes. DEV_MANUAL.md updated with what the guard covers; MANUAL.md checked, no fastchess surface, no change; README.md checked, owner-written, no change needed. 2026-08-13_adversarial-F01 stays `planned`: the audit has not been re-run. Gate: ctest -L fast 9/9, clang-format.sh --check clean.

## What is broken

`44877c4` renamed `perf_cores` to `all_cores` and left `fastchess.sh:108`
printing the old name. The script runs under `set -euo pipefail`, so the
expansion aborts it before `fastchess` is ever invoked.

The abort is invisible. `trap 'rm -f "$snapshot"' EXIT` at line 80 runs `rm -f`
successfully and bash takes the trap's status as the script's, so the script
exits **0**. The same script with the trap removed exits 1.

## Why it jumps the queue

No SPRT verdict has been obtainable since `44877c4`. INV-6 and every rule in
`CLAUDE.md` about deciding a change by measurement rather than argument have no
working tool behind them until this is fixed, so nothing that alters play can be
decided in the meantime.

The failure shape is the dangerous part. Launched the documented way —
`REF=<sha> nohup ./fastchess.sh --fast > .tuning/sprt_x.log 2>&1 &` — it dies in
under a second, status 0, three lines in the log. A watcher polling for `^Elo:`
sees exactly what a match that has not reached its first verdict looks like.

## Shape

Two separate repairs, and the second is the one that stops this recurring:

- the `$perf_cores` reference. The number the line wants is `$concurrency`,
  which is already printed on the same line.
- the trap's ability to rewrite the exit status:
  `trap 'status=$?; rm -f "$snapshot"; exit $status' EXIT`, or move the snapshot
  cleanup somewhere that cannot overwrite it.

## The guard

Nothing in `tests/` exercises any shell script today, which is why a rename
could break the only SPRT harness and stay broken across a commit. The guard is
a smoke run: a stub `fastchess` on `PATH` and an already-built reference, then
assert the stub was reached. It runs in seconds and plays no games.

Red first: run it against `44877c4`'s `fastchess.sh` and observe it fail before
the fix lands.

## What the step found that the finding did not

**A third abort, earlier than F01's, and fatal on this machine.** Line 77 was
`mktemp -t chesso-candidate`. `-t` with no X's in the template is a BSD-ism;
GNU mktemp rejects it outright, and the assignment aborts the script under
`set -e` before the trap is even armed. At `44877c4` on this machine the script
therefore dies at line 77, not at line 108, and F01's own reproduction cannot
be taken from here as written. The red run was taken twice for that reason:
once as committed, once with only the `mktemp` line repaired, and the second
one prints F01's `perf_cores: unbound variable` exactly.

**The masking half of F01 does not reproduce under bash 5.2.21.** The audit
measured the `perf_cores` abort exiting 0 and attributed it to
`trap 'rm -f "$snapshot"' EXIT`. The same script, same failure, exits **1**
here: this bash keeps the status the shell exited with unless the trap itself
runs `exit`. The audit ran under a shell where the trap does take over. The
hardening is applied anyway — it costs one assignment and is correct on both —
but it is recorded as unverifiable here rather than as a fix that was watched
working, and the guard's second case currently passes on bash's behaviour
rather than on the trap's.

**One more macOS-ism, on the reference-build path.** Line 93 fed
`sysctl -n hw.logicalcpu` to `cmake --build -j`. On Linux that prints
`sysctl: cannot stat /proc/sys/hw/logicalcpu` into the run's log and passes a
bare `-j`, so it builds at the default parallel level and is loud rather than
broken. Repaired with the same `|| nproc` fallback line 44 already uses. It is
inside this step because `.ref-builds/` was empty, so the next real SPRT was
going to hit it.

## Beyond the accepts

The guard stubs `fastchess`, so it cannot see a flag the installed `fastchess`
rejects. A bounded real match was run as well: reference `7b4d9a4` built into
its worktree, `tc 10+0.2  concurrency 12 of 12 cores`, games played and
completed against the real binary, then killed. That is the end-to-end claim the
stubbed run cannot make.
author:    Maksym Bodnar
