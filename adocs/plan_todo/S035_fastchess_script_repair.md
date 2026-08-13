id:         S035
goal:       restore fastchess.sh so a match actually runs, and stop the EXIT trap masking a failure as status 0
accepts:    fastchess.sh reaches the fastchess invocation with a stubbed fastchess on PATH; a script aborting before that point exits non-zero; the smoke run is a test in the suite and was observed failing at 44877c4
touches:    fastchess.sh
excludes:   the concurrency policy of DEC-048, the tc defaults, and anything about how a verdict is read
decisions:  DEC-048
closes:     2026-08-13_adversarial-F01
blocks:
paused_by:
done:

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
