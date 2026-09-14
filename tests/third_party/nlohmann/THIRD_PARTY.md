# Third-party material in this directory

**Project:** JSON for Modern C++ (`nlohmann/json`)
**Version:** 3.11.3
**Taken from:** commit `9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03`, which
`git describe --tags` names `v3.11.3`
**URL:** https://github.com/nlohmann/json
**Licence:** MIT, the library's own `LICENSE.MIT` beside this file
**sha256 of `json.hpp`:**
`9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6`

`json.hpp` is the library's own amalgamated single-header release artefact,
copied byte for byte from `single_include/nlohmann/json.hpp` of that commit and
unmodified; this project's tests are its only consumer, and nothing in `src/`
or in the shipped engine includes it.

`json_fwd.hpp` ships beside it upstream and is not taken: nothing here includes
it. `git grep json_fwd` is the check, and it is what a future step that needs
the forward header should re-run before adding it.

## Who uses it, and how it is reached

Three test translation units include it -- `tests/test_chesso.cpp` and
`tests/test_helpers.hpp` as `<json.hpp>`, `tests/test_perft.cpp` as
`"json.hpp"` -- so every doctest binary reaches it through `test_helpers.hpp`.
Both forms resolve because this directory is on the include path as a system
directory: `add_doctest_target` in `tests/CMakeLists.txt` puts it there for
every doctest binary, and the `target_include_directories` beside the
`test_perft` target does the same for perft, which has its own `main()`. System
rather than plain, so a 900 kB header nobody here maintains does not have to
satisfy `-Wall -Wextra -Werror`.

`clang-format.sh` skips this directory by path, the way it skips `adocs/`: the
value of the file is that it is byte for byte what upstream published, and
formatting it destroys exactly that. `test_clang_format_script.sh` asserts the
exclusion.

## The whole file is MIT, and that is checkable

The amalgamation carries an SPDX tag per inlined source file, 45 of them, and
every one reads `SPDX-License-Identifier: MIT` -- Hedley included, which is
Evan Nemerson's and is marked with his copyright beside Niels Lohmann's. So
`grep -c 'SPDX-License-Identifier: MIT'` and `grep -c 'SPDX-License-Identifier'`
both answer 45, and that is the check to re-run if the version here ever moves.

## Why the single header and not a submodule

Until S229 the library was the `tests/json` submodule, a gitlink to the whole
upstream repository. Nothing here built, included or shipped anything but the
single header, yet `git submodule update --init` put the upstream *test tree*
in every checkout -- and that tree carries third-party fixtures under licences
this MIT repository will not redistribute, declared in the library's own
`.reuse/dep5`. **It must not go back to being a submodule for that reason.**
S229 is the step and DEC-207 the neighbouring decision; `books/fetch_book.sh`
carries the standing inventory of what this repository bundles.

Vendoring a dependency's own release artefact, with its licence beside it, is
not the copying the founding rule bans: that rule is about another *engine's*
source, tables and training data (DEC-016, DEC-104). No dependency is added
either -- it is the same library at the same version, in the form its authors
publish for exactly this use.
