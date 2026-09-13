id:         S227
goal:       pixello and the debug GUI leave the branch -- the `tests/pixello` submodule, the `CHESSO_BUILD_GUI` option and `debug_gui` target, `tests/debug_gui.cpp` and everything under `tests/assets/gui/` -- on the owner's instruction (DEC-207)
accepts:    `.gitmodules` has no `tests/pixello` entry and the gitlink is removed with `git rm` (`git submodule deinit -f tests/pixello` first, `.git/modules/tests/pixello` gone); `CMakeLists.txt` has no `CHESSO_BUILD_GUI` option and `tests/CMakeLists.txt` no `debug_gui` target, no `add_subdirectory(pixello)`, no `PIXELLO_ENABLE_TESTS`; `tests/debug_gui.cpp` is deleted; `tests/assets/gui/` is deleted whole, `THIRD_PARTY.md` included, and the `file(COPY ... assets)` line still copies `perft_json` and `test_jsons` for the tests that read them; `git grep -i pixello` over the tree outside `adocs/plan_done/`, `adocs/audit/`, `adocs/decisions.md` and `adocs/status.md` returns nothing; `books/fetch_book.sh`'s sentence about the submodules names `nlohmann/json` and `doctest` only; `DEV_MANUAL.md` and `TOOLCHAIN.md` lose every SDL2 and GUI-build line (`MANUAL.md` checked); `README.md` is not touched and the stamp lists its lines 33-34 and 63-68 for the owner to remove; the fast suite is green in both builds from a fresh configure of `build` (the option's removal changes the cache); no `src/` change, `No functional change`
touches:    .gitmodules, CMakeLists.txt, tests/CMakeLists.txt, tests/debug_gui.cpp, tests/assets/gui/, tests/pixello, books/fetch_book.sh, DEV_MANUAL.md, TOOLCHAIN.md
excludes:   `README.md`; the `nlohmann/json` and `doctest` submodules; anything under `src/`
decisions:  DEC-207, DEC-201
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-13 14:05 on the idle machine, the owner's instruction of the same day (DEC-207)
done:

## Why this exists

The owner's instruction of 2026-09-13 (DEC-207). Pixello was an early
debugging aid; nothing in the gate, the tools or the harness builds it, and it
is the reason the branch carried third-party artwork at all (S211 licensed it,
S224 deleted the sounds). Removing it closes the parked question about the
submodule's own test sounds.

## Cost

Agent work, under an hour; one fresh configure and the gate.
