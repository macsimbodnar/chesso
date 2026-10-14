id:         S280
goal:       the mutation fixture recipe works as written on a git carrying the CVE-2022-39253 fix (2.30.6, and every maintenance release that merged it, 2.38.1 among them), where a submodule whose URL is a local path is refused over the `file` transport
accepts:    the recipe in `DEV_MANUAL.md` ("What the fast suite is worth") and in `tools/mutation_check.py`'s docstring is run as written in a fresh `.ref-builds/mut` worktree on this machine (git 2.43.0) and builds the tests, observed failing first with "transport 'file' not allowed"; the fix is the narrowest that works, a per-command `git -c protocol.file.allow=always` and not a global config change, with the reason stated beside it; any other recipe in `DEV_MANUAL.md` or `tools/` that runs `submodule update` on a worktree is checked the same way
touches:    DEV_MANUAL.md, tools/mutation_check.py (docstring)
excludes:   fastchess.sh, which builds no tests; the main tree's `submodule.tests/doctest.url`, a machine-local setting
decisions:  DEC-171 (a defect off the UCI surface and out of play: filler behind the next strength step)
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S280_brief.md), 2026-10-14
done:       2026-10-14 09:37 -- the mutation fixture recipe's submodule line, in `DEV_MANUAL.md` and `tools/mutation_check.py`'s docstring, is `git -c protocol.file.allow=always -C .ref-builds/mut submodule update --init tests/doctest`, with the reason beside it: this clone's `submodule.tests/doctest.url` is the main tree's own `tests/doctest`, pointed there by S254 because the `.gitmodules` ssh URL had no key here, and git refuses a submodule clone over the `file` transport by default since the CVE-2022-39253 fix (2.30.6, and every maintenance release that merged it, 2.38.1 among them; RelNotes 2.30.6, 2.37.4, 2.38.1; `man git-config` `protocol.allow`); on a clone keeping the `.gitmodules` URL the flag changes nothing. Both sites add the teardown, `git worktree remove --force .ref-builds/mut`. Observed on git 2.43.0 in a scratch `.ref-builds/mut` at `bacead1`: the old line fails "fatal: transport 'file' not allowed" twice and exits 1; the fixed line checks out doctest `ae7a135` (v2.4.11) with no config file touched; the `cmake` line configures, the tests build at `-j12`, and `--only NB01` kills NB01 (fast 1/46, bench same) over a green 46-test baseline at bench 5636835; the scratch worktree needed `git worktree remove --force` (a submodule) and is gone. No other recipe runs `submodule update` on a worktree. No `src/` change. Fast suite 46/46 in both builds, format clean, `plan_prose_check` clean in all five modes. Fast check: 3 findings fixed (the version claim is the CVE-2022-39253 fix's, 2.30.6 and its merges, not 2.38.1's; S254 named as the local URL's source, with the flag inert on the `.gitmodules` URL; the `--force` teardown line added to both sites). Evidence `.tuning/coord/S280/` (gitignored). Implemented by an Opus subagent briefed by the coordinator.

## Why this exists

Found 2026-10-14 by S279's DEC-141 mutant run. This repository's
`submodule.tests/doctest.url` is the main tree's own path,
`/home/max/ws/chesso/tests/doctest`, so `git -C .ref-builds/mut submodule
update --init tests/doctest` fetches over the `file` transport, which git
refuses by default since the CVE-2022-39253 fix (2.30.6, and every maintenance
release that merged it, 2.38.1 among them). The recipe line fails with
"transport 'file' not allowed"; S279 passed with `git -c
protocol.file.allow=always`. The coordinator's own worktree setup has the same
failure and copies the submodule by hand. Not reachable in play and not on the
UCI surface, so it is filler (DEC-171), not a blocker.

## As built (2026-10-14, the Opus subagent)

Worktree `/home/max/ws/chesso-s280`, branch `s280`, uncommitted. Phase 1 was
written blind on `e9d91e3` while a match held the machine (DEC-260): reads,
greps and edits only. Phase 2 ran with the machine free, after the worktree was
brought onto `bacead1` (S261's completing commit, a descendant) by stash,
`git merge --ff-only` and pop: `DEV_MANUAL.md` auto-merged with no conflict,
and the one line the edit had left at 89 characters was rewrapped. Evidence
under `.tuning/coord/S280/` in the main repository (gitignored):
`phase2.log`, `mut_build.log`, `nb01.log`, `suite.log`, `prose.log`.

### What changed

- `DEV_MANUAL.md`, "Mutation check, `tools/mutation_check.py`": the recipe's
  submodule line is now `git -c protocol.file.allow=always -C .ref-builds/mut
  submodule update --init tests/doctest`, wrapped like the `cmake` line under
  it, and one sentence in the paragraph under the block ("The submodule line
  is not optional ...") says why.
- `tools/mutation_check.py`, module docstring: the same line and the same
  sentence. No code line moves.
- After the fast check (below): both sites name S254 as where the local URL
  comes from and say the flag is inert on a clone keeping the `.gitmodules`
  URL; both carry the teardown, `git worktree remove --force .ref-builds/mut`,
  and why it takes `--force`; the version claim is the CVE fix's, not 2.38.1's.

No `src/`, test or config change. The main tree's
`submodule.tests/doctest.url` is untouched (`excludes:`).

### The git claim, as read

- `git config --get submodule.tests/doctest.url` in `/home/max/ws/chesso`:
  `/home/max/ws/chesso/tests/doctest`. `.gitmodules` names
  `git@github.com:doctest/doctest.git`, so the local path is this clone's
  `.git/config`, and the sentence says "this clone's". `git --version`:
  2.43.0.
- Where it came from: `adocs/plan_done/S254_stale_mutant_strings.md`'s
  fixture note, "`git submodule update --init tests/doctest` in a fresh
  worktree fails on this machine (the `.gitmodules` URL is SSH and no key
  answers); pointing `submodule.tests/doctest.url` at the main tree's checkout
  and passing `-c protocol.file.allow=always` worked".
- `/usr/share/doc/git/RelNotes/2.30.6.txt`, under CVE-2022-39253: "the value
  of `protocol.file.allow` is changed to be "user" by default". `2.38.1.txt`
  and `2.31.5.txt`, `2.32.4.txt`, `2.33.5.txt`, `2.34.5.txt`, `2.35.5.txt`,
  `2.36.3.txt`: "This release merges the security fix that appears in
  v2.30.6"; `2.37.4.txt`: "also includes the same security fixes as in
  v2.30.6". `2.38.0.txt` names neither 2.30.6 nor CVE-2022-39253 nor
  `protocol.file.allow` (grep): the fix reached 2.30.6's line first and 2.38
  only at 2.38.1, so "since 2.38.1", phase 1's wording, was wrong (fast check,
  finding 1).
- `man git-config`, `protocol.allow`: `file` defaults to `user`, which "is only
  able to be used when GIT_PROTOCOL_FROM_USER is either unset or has a value of
  1" and is for a protocol not to be "used by commands which execute
  clone/fetch/push commands without user input, e.g. recursive submodule
  initialization"; `protocol.<name>.allow`: `file` is "any local file-based
  path (including file:// URLs, or local paths)". `man git`,
  `GIT_PROTOCOL_FROM_USER`: "useful to restrict recursive submodule
  initialization".
- The documents give the policy and name submodule initialization as its case;
  that `git submodule update` hits it here is an observation, S279's deviation
  5, and phase 2 observes it again before the fix.
- Where the URL is still `.gitmodules`' `git@github.com:` one, the flag changes
  nothing: it governs `file` only, and `ssh`'s default is `always` (the same
  man page). The clone there needs an ssh key instead.

### Other recipes

Grepped `DEV_MANUAL.md`, `TOOLCHAIN.md`, `MANUAL.md`, `tools/`,
`adocs/data/*.sh` and the root `*.sh` for `submodule update`, `protocol.file`
and `protocol.allow`: the two sites above and nothing else. No other recipe
runs `submodule update` on a worktree. `adocs/data/S154_floor_margin_sweep.py`
and `adocs/data/S156_mined_floor_sweep.py` copy `tests/doctest` out of the main
checkout instead, so they do not fail this way; their refusal text sends the
user to `git submodule update --init` in the main tree, not in a worktree.
Their comments say the recorded URL is `git@github.com:` "here", which this
machine's `.git/config` no longer is: earlier steps' evidence scripts, noted
and not edited. `fastchess.sh` (excluded), `tools/gate.sh --build-parent` and
`adocs/data/S206_truncation_drift.sh` add worktrees and build no tests.

### Phase 2, observed

Run from `/home/max/ws/chesso-s280` at `bacead1`, the fixture a scratch
worktree of it, the recipe's lines as written (`phase2.log`):

1. `git worktree add --detach .ref-builds/mut HEAD` (exit 0), then the old
   line, `git -C .ref-builds/mut submodule update --init tests/doctest`, exit
   1, both attempts refused:

   ```
   Cloning into '/home/max/ws/chesso-s280/.ref-builds/mut/tests/doctest'...
   fatal: transport 'file' not allowed
   fatal: clone of '/home/max/ws/chesso/tests/doctest' into submodule path '/home/max/ws/chesso-s280/.ref-builds/mut/tests/doctest' failed
   Failed to clone 'tests/doctest'. Retry scheduled
   ...
   Failed to clone 'tests/doctest' a second time, aborting
   ```

2. The fixed line, both physical lines as written, exit 0: "Submodule path
   'tests/doctest': checked out 'ae7a13539fb71f270b87eb2e874fbac80bc8dda2'"
   (`submodule status`: `v2.4.11`); the fixture's `status --porcelain` is
   empty, and `git config --get protocol.file.allow` answers nothing (exit 1),
   so no config file took the setting. The `cmake -S` line as written
   configures (exit 0); `cmake --build .ref-builds/mut/build -j12` exit 0, no
   `error` line in `mut_build.log`, 25 `test_*` targets built.
3. `CLANG_FORMAT_MAJOR=22 python3 tools/mutation_check.py tools/mutants
   .ref-builds/mut --only NB01` (`nb01.log`): "worktree ... at bacead1 clean",
   "baseline green, 46 tests, bench 5636835 nodes via engine", then
   `NB01_qs_limit_check_below_probe` killed, fast 1/46, bench same, expected
   killed; caught by `test_search`'s four node-budget cases; "mutation score 1
   of 1 (100%)", wall 228 s, `MUTATION-RUN-DONE`, exit 0.
4. `git worktree remove .ref-builds/mut` refuses, "fatal: working trees
   containing submodules cannot be moved or removed" (exit 128), as `man
   git-worktree` says; `git worktree remove --force .ref-builds/mut` exit 0.
   `git worktree list` names nothing under `chesso-s280/.ref-builds`,
   `.ref-builds/` is empty, no admin directory under the main `.git/worktrees/`
   points at it, and `git worktree prune --dry-run -v` prints nothing.
5. In this worktree, with `CLANG_FORMAT_MAJOR=22` (`suite.log`): `build` and
   `build-tune` configured fresh as `DEV_MANUAL.md` gives them, then the TESTS
   command as `AGENTS.md` writes it: 46/46 fast in `build`, 46/46 fast in
   `build-tune`, `./clang-format.sh --check` silent, exit 0.
6. `tools/plan_prose_check.py` (`prose.log`), every mode exit 0: `--prose` 0
   sentences flagged; `--citations` 0 flagged over 30 files, this file 0 code
   citations (two document notes, ungated, are S136's and predate this step);
   `--touches` 0 flagged over 30 files; `--params` silent; `--gate` "gate
   command agrees in both documents".

### Fast check, 3 findings fixed

1. **The version claim.** Phase 1 wrote "by default since 2.38.1" in both
   sites, the stamp and the commit text, and the goal line said "git 2.38 and
   later"; the default came with 2.30.6 and every maintenance line that merged
   it, and 2.38.0 does not refuse (the release notes above). Every site now
   says "since the CVE-2022-39253 fix (2.30.6, and every maintenance release
   that merged it, 2.38.1 among them)", the goal line and this file's "Why"
   included.
2. **Where the local URL comes from.** Both sites now say S254 pointed
   `submodule.tests/doctest.url` at the main tree's checkout in this clone's
   `.git/config`, because the `.gitmodules` `git@github.com:` URL had no ssh
   key here to answer it, and that on a clone keeping the `.gitmodules` URL
   the flag changes nothing and the line needs an ssh key instead.
3. **Teardown.** `DEV_MANUAL.md`'s block ends with `git worktree remove
   --force .ref-builds/mut`, and the paragraph says why it takes `--force`; the
   docstring gives the same line and reason. Plain `remove` refusing is
   phase 2's step 4.

Re-checked after the fixes: `python3 -c 'import ast,sys;
ast.parse(open("tools/mutation_check.py").read())'` exit 0, and
`tools/plan_prose_check.py` in all five modes (`prose_fix.log`). No build: the
three fixes are prose.

### Proposed `done:` stamp

2026-10-14 HH:MM -- the mutation fixture recipe's submodule line, in `DEV_MANUAL.md` and `tools/mutation_check.py`'s docstring, is `git -c protocol.file.allow=always -C .ref-builds/mut submodule update --init tests/doctest`, with the reason beside it: this clone's `submodule.tests/doctest.url` is the main tree's own `tests/doctest`, pointed there by S254 because the `.gitmodules` ssh URL had no key here, and git refuses a submodule clone over the `file` transport by default since the CVE-2022-39253 fix (2.30.6, and every maintenance release that merged it, 2.38.1 among them; RelNotes 2.30.6, 2.37.4, 2.38.1; `man git-config` `protocol.allow`); on a clone keeping the `.gitmodules` URL the flag changes nothing. Both sites add the teardown, `git worktree remove --force .ref-builds/mut`. Observed on git 2.43.0 in a scratch `.ref-builds/mut` at `bacead1`: the old line fails "fatal: transport 'file' not allowed" twice and exits 1; the fixed line checks out doctest `ae7a135` (v2.4.11) with no config file touched; the `cmake` line configures, the tests build at `-j12`, and `--only NB01` kills NB01 (fast 1/46, bench same) over a green 46-test baseline at bench 5636835; the scratch worktree needed `git worktree remove --force` (a submodule) and is gone. No other recipe runs `submodule update` on a worktree. No `src/` change. Fast suite 46/46 in both builds, format clean, `plan_prose_check` clean in all five modes. Fast check: 3 findings fixed (the version claim is the CVE-2022-39253 fix's, 2.30.6 and its merges, not 2.38.1's; S254 named as the local URL's source, with the flag inert on the `.gitmodules` URL; the `--force` teardown line added to both sites). Evidence `.tuning/coord/S280/` (gitignored).

### Commit text

```
Allow the file transport on the mutation fixture's submodule (S280)

The mutation fixture recipe in DEV_MANUAL.md and mutation_check.py's
docstring ran `git -C .ref-builds/mut submodule update --init
tests/doctest`, which fails here with "transport 'file' not allowed":
this clone's submodule.tests/doctest.url is the main tree's own path,
set by S254 because the .gitmodules ssh URL had no key here, and git
refuses a submodule clone over the file transport by default since
the CVE-2022-39253 fix (2.30.6 and every maintenance release that
merged it, 2.38.1 among them). Found by S279's mutant run.

The line now passes -c protocol.file.allow=always on that one command,
not in any config file, with the reason beside it; on a clone keeping
the .gitmodules URL it changes nothing. Both sites gain the teardown,
git worktree remove --force, since a plain remove refuses a worktree
holding a submodule. Observed failing and then passing in a scratch
worktree; the tests built and NB01 was killed over a green baseline.
No other recipe runs submodule update on a worktree. Filler under
DEC-171. No src/ change.
```

No `Bench:` or `No functional change` line: `tools/gate.sh` owes neither to a
commit that touches no `src/` file ("A commit that touches no `src/` file
needs neither and may carry neither"), and S277's commit, `1edb3b1`, is the
precedent.
