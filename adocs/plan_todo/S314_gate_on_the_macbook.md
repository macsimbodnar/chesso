id:         S314
goal:       `tools/gate.sh` reads green on the MacBook Air at a tree the workstation reads green
accepts:    each row below is read to its cause, with what the reading was; `test_fastchess_script` passes on that machine inside its clock, or a decision says why its clock differs there; the format disagreement is read against major 22 on the workstation and settled by DEC-146's route; no test relaxed; both builds green on both machines
touches:    tests/test_fastchess_script.sh, fastchess.sh, src/data_structures.hpp, clang-format.sh, TOOLCHAIN.md, DEV_MANUAL.md
excludes:   any comment cleanup, S311 and S312's; anything before the comment work and S101's run release both machines
decisions:  DEC-146, DEC-171, DEC-335
closes:
blocks:
paused_by:
author:
done:

## Why

Read 2026-11-01 at `55d7e75` on the MacBook Air M5, on battery, Apple clang 21, clang-format 23.1.4. DEC-335 (5) asks that machine's gate of every
comment commit, and it was never read before that day. `bench` there is 5707449, and 49 of 50 tests pass in each build. No code on this list
moves a score, a move or a line, so it is filler by DEC-171.

| what | read |
|---|---|
| `test_fastchess_script`, the clock | 72 s and 71 s run alone, 7 s user, 9 s sys, against `TIMEOUT 60`. What it waits on is unread |
| `test_fastchess_script`, default `TMPDIR` | "BARE_FEN=1: the args= are not -I, the relay snapshot and each side's own binary". `/var/folders` is a symlink, which is the suspicion and is not proved |
| `test_fastchess_script`, `TMPDIR` a resolved path | that case passes and the next stops it: "BARE_FEN=1: expected four isolated (-I) launches of one snapshot of the tree's relay, two probes and two sides". The cause is unread, and that chain stops at its first failure, so more can sit behind it |
| `clang-format.sh --check` | flags `src/data_structures.hpp` lines 639 and 640, `struct alignas(64) tt_cluster_t`, under major 23. Major 22 is not on that machine, so its side is unread |
