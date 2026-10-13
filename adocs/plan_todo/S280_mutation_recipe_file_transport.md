id:         S280
goal:       the mutation fixture recipe works as written on git 2.38 and later, where a submodule whose URL is a local path is refused over the `file` transport
accepts:    the recipe in `DEV_MANUAL.md` ("What the fast suite is worth") and in `tools/mutation_check.py`'s docstring is run as written in a fresh `.ref-builds/mut` worktree on this machine (git 2.43.0) and builds the tests, observed failing first with "transport 'file' not allowed"; the fix is the narrowest that works, a per-command `git -c protocol.file.allow=always` and not a global config change, with the reason stated beside it; any other recipe in `DEV_MANUAL.md` or `tools/` that runs `submodule update` on a worktree is checked the same way
touches:    DEV_MANUAL.md, tools/mutation_check.py (docstring)
excludes:   fastchess.sh, which builds no tests; the main tree's `submodule.tests/doctest.url`, a machine-local setting
decisions:  DEC-171 (a defect off the UCI surface and out of play: filler behind the next strength step)
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found 2026-10-14 by S279's DEC-141 mutant run. This repository's
`submodule.tests/doctest.url` is the main tree's own path,
`/home/max/ws/chesso/tests/doctest`, so `git -C .ref-builds/mut submodule
update --init tests/doctest` fetches over the `file` transport, which git
refuses by default since 2.38.1 (CVE-2022-39253). The recipe line fails with
"transport 'file' not allowed"; S279 passed with `git -c
protocol.file.allow=always`. The coordinator's own worktree setup has the same
failure and copies the submodule by hand. Not reachable in play and not on the
UCI surface, so it is filler (DEC-171), not a blocker.
