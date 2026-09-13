id:         S229
goal:       the nlohmann/json dependency is carried as its MIT single header rather than as a gitlink to the whole repository, so a checkout of this branch contains no GPL-licensed file
accepts:    `tests/json` is no longer a submodule (`git submodule deinit`, `git rm`, the `.gitmodules` entry gone, `.git/modules/tests/json` gone); `tests/third_party/nlohmann/json.hpp` (the amalgamated single header of the pinned release, its version recorded) and `LICENSE.MIT` beside it are tracked, and a `THIRD_PARTY.md` in that directory names the project, the version, the URL and the licence; every include path that named `json/single_include/nlohmann` names the new directory (`tests/CMakeLists.txt`, `tools/CMakeLists.txt` and any other); `git grep -n 'GPL' -- ':!adocs/plan_done' ':!adocs/audit' ':!adocs/decisions.md' ':!adocs/status.md'` returns only the founding-rule sentences that mention the licence by name; `books/fetch_book.sh`'s submodule sentence names doctest alone and says how json is carried; the fast suite is green in both builds from a fresh configure; no `src/` change, `No functional change`
touches:    .gitmodules, tests/json, tests/third_party/, tests/CMakeLists.txt, tools/CMakeLists.txt, books/fetch_book.sh, DEV_MANUAL.md
excludes:   doctest, which stays a submodule (plain MIT throughout); any change to what the tests or tools do with JSON
decisions:  DEC-207, DEC-201, DEC-104
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S227's fast check: `tests/json/.reuse/dep5` declares the library's own test
fixtures under `tests/thirdparty/imapdl/*` as GPL-3.0-only; a
`git submodule update --init` puts them in every checkout although nothing
here builds, includes or ships them. The owner's stated reason for the no-copy
rule is "no GPL question anywhere in this codebase" (CLAUDE.md); the single
header is the library's own MIT distribution form and removes the question
without changing a line of what the tests do. Vendoring a dependency's own
release artefact with its licence is not copying engine code (DEC-104 is about
engines); the DEPS rule is not engaged because no dependency is added.

## Cost

Agent work, under an hour; a fresh configure and the gate. Filler behind S228
(DEC-171: not reachable in play).
