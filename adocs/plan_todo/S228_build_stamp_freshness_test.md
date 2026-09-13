id:         S228
goal:       a test proves the `id name` build stamp follows the tree without a reconfigure -- configure once, change the commit or the dirty state, rebuild, and the reply changes -- so S212's identity check cannot be defeated by a stale header (2026-09-12_plan_adversarial-F02, DEC-206)
accepts:    a test in the fast suite (shell or Python, registered in `tests/CMakeLists.txt`) builds a sandbox checkout with a tiny CMake project or drives `cmake/build_info.cmake` as `cmake -P` the way the `chesso_build_info` target does: it generates the header at one commit, commits a change or dirties a tracked file, re-runs the generation step **without reconfiguring**, and asserts the header's sha and `-dirty` suffix moved; a second case asserts an unchanged tree leaves the header byte-identical (no rebuild storm); observed red against a version of the script that captures the sha once; `DEV_MANUAL.md`'s paragraph on the stamp cites the test; no `src/` change
touches:    tests/, tests/CMakeLists.txt, cmake/build_info.cmake, DEV_MANUAL.md
excludes:   the stamp's form (DEC-204); `fastchess.sh`'s identity check itself (S212's properties cover it)
decisions:  DEC-206, DEC-204
closes:     2026-09-12_plan_adversarial-F02
blocks:
paused_by:
author:
done:

## Why this exists

The owner's plan review of 2026-09-12 (F02) found S212's accepts naming
configure time for the stamp -- the trap `cmake/build_info.cmake` was written
to avoid -- and asked for a property test that a rebuild without a reconfigure
moves the identity. S212 landed the build-time mechanism (DEC-204 (a)); the
test does not exist. A harness-test gap, not reachable in play: filler behind
S225 (DEC-171).

## Cost

Agent work, an hour; the fast suite once.
