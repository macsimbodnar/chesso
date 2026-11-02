id:         S322
goal:       every doctest binary reads its fixtures from the source tree by construction, so a new binary cannot pass under ctest and fail from another directory, and the build's unread copy of `tests/assets/` goes
accepts:    `CHESSO_SOURCE_DIR` reaches every doctest target from one place, or the reason it cannot is stated; a planted new binary calling `load_json` without the per-target line reads the same tally from the repository root, `build/tests` and a third directory, seen red first on the parent; `tests/CMakeLists.txt`'s `file(COPY ... assets)` removed only after every reader of `build/tests/assets` is shown gone (tests, scripts under `adocs/data/` and `tools/`, `DEV_MANUAL.md`), each reader named with what replaced it; no case, bound or assertion changes; both builds green
touches:    tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   refactoring `load_json` or the per-binary loaders beyond S310's `#ifdef CHESSO_SOURCE_DIR` form; changing what any case asserts
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-02 by S320's implementer, and left by it as beyond its step. S320 gave eight binaries `CHESSO_SOURCE_DIR`
and a root `WORKING_DIRECTORY` per ctest entry. The guard covers only the entries it moved. A new doctest binary that
calls `load_json` without the macro still passes under ctest, which runs it from `build/tests`, where the
`file(COPY .../assets)` at the top of `tests/CMakeLists.txt` puts a copy, and it fails from anywhere else. Once every
reader takes the source tree, nothing reads that copy. S320's implementer named `add_doctest_target_linking` as the one
place a definition would reach every target. Filler by DEC-171: green under ctest, not reachable in play.
