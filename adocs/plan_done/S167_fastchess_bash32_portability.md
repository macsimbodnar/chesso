id:         S167
goal:       fastchess.sh runs and reports failure correctly under bash 3.2, the macOS /bin/bash
accepts:    `ctest --test-dir build -L fast` is green, `test_fastchess_script` included, on macOS with /bin/bash 3.2.57; the two assertions that fail today -- "script aborted before fastchess but exited 0 (the EXIT trap masks it)" and "REF unset: the script exited 1" -- both pass; the failure marker and a non-zero status are produced on a `set -u` abort whatever `$?` the trap is handed; no bash feature newer than 3.2 is introduced
touches:    fastchess.sh, src/search.cpp, src/search_params.hpp
excludes:   tests/test_fastchess_script.sh -- the test is correct and caught both defects; it is not edited. No change to what the script measures: bounds, book, concurrency, adjudication and the fastchess invocation are untouched
decisions:
closes:
blocks:
paused_by:
author:    Maksym Bodnar
done:      Two bash 3.2 defects in fastchess.sh, both measured on this machine and both fixed in one file. F-b, the one that made the script unusable: the git wrapper's body was `command git`, and bash 3.2 does not suppress errexit for the `command` builtin on the left of `||`, so the dirty-tree probe `git diff --quiet HEAD || diff_status=$?` killed every working-tree run before its banner. The binary is now resolved once and called directly; the wrapper and its anchoring are unchanged. F-a, the one that made F-b silent: an EXIT trap under bash 3.2 reads $? == 0 when the shell aborts on an unbound variable under set -u, so the script printed no SPRT-RUN-FAILED and exited 0 -- a detached run dying early would have left its watcher spinning to the ceiling. The trap now asks a `completed` flag set beside SPRT-RUN-DONE instead of trusting the status, which is version-independent. test_fastchess_script went from 2 failing assertions to 8 properties holding; fast suite 20/20 green, clang-format clean. The test was not edited: S160 wrote it and it caught both. Found alongside and fixed first: src/search.cpp:23's dead MAX constant failed the clang -Werror build, which gcc had never warned on -- no behaviour, nothing read it. The status.md handover check then passed to the node at depths 9 and 12, so every deterministic figure the repository records is valid on this MacBook as written.

## Why this jumps the queue

The `achesso` work moved to a MacBook on 2026-08-23 (status.md's handover
item). `/bin/bash` there is **3.2.57(1)-release**, the last GPLv2 bash Apple
ships; the Linux machine the script was written on runs bash 5. Two bash 3.2
behaviours make `fastchess.sh` unusable on the new machine, and one of them is
silent. Every SPRT the plan still owes -- S024 alone owes two -- runs through
this script, so nothing downstream can be measured until it works.

Found while answering "what is next in the plan" after the handover, not by a
run: `ctest -L fast` reported `test_fastchess_script` failing on two
assertions. The test was written by S160 against the same defect class and it
did its job.

## The two defects

### F-a. The EXIT trap is handed status 0 for a `set -u` abort

`fastchess.sh:65-70` reads `status=$?` and prints `SPRT-RUN-FAILED: exited
$status` only when that status is non-zero. Under bash 3.2 an EXIT trap sees
`$? == 0` when the shell dies on an unbound variable under `set -u`. Measured
on this machine:

```
$ /bin/bash -c 'set -euo pipefail; trap "echo trap-sees=\$?" EXIT; echo "${nope}"'
nope: unbound variable
trap-sees=0                       # bash 5 prints 1
$ echo $?
0                                 # the script "succeeded"
```

The `set -e` path is unaffected -- a plain failing command still shows 1 in the
trap -- so only the `set -u` shape is masked. That shape is exactly what
`test_fastchess_script` case 2 injects, and it is the shape of the original
F01 the trap was written for.

Consequence beyond the exit code: a detached run that dies early prints **no
terminal marker at all**, and the watcher AGENTS.md par.12 mandates breaks on
`SPRT-RUN-(DONE|FAILED)`. It would spin to its `--ceiling` -- hours of
measurement capacity, which is the plan's binding constraint, spent on a run
that died in its first second.

### F-b. `command` on the left of `||` is not protected from errexit

`fastchess.sh:195`, the dirty-tree probe:

```
git diff --quiet HEAD || diff_status=$?
```

`git` is the wrapper function at `:82-85` whose body is `command git -C "$repo"
"$@"`. Bash 3.2 does not suppress `errexit` for the **`command` builtin** when
it is the left operand of `||`. Narrowed on this machine, each line its own
`/bin/bash` script under `set -euo pipefail`:

| form | bash 3.2.57 |
|---|---|
| `false \|\| st=$?` | continues, `st=1` |
| `command false \|\| st=$?` | **shell exits 1** |
| `g(){ false; }; g \|\| st=$?` | continues, `st=1` |
| `g(){ "$resolved" "$@"; }; g \|\| st=$?` | continues, `st=1` |
| `( command false ) \|\| st=$?` | continues, `st=1` |

So the function is not the problem and neither is `||`: the `command` builtin
is. `git diff --quiet HEAD` exits 1 on a dirty tree -- the normal case for
every `--fast` run, which measures the working tree -- and the script dies at
line 195, before the banner, with F-a masking the status. On this machine
`./fastchess.sh` cannot complete a single run today.

## The fix

1. **Resolve git once, call the binary.** `real_git="$(command -v git)"` at the
   wrapper site, `git() { "$real_git" -C "$repo" "$@"; }`. Every `git ...` below
   stays the wrapper, so the anchoring the `:78-81` comment defends is intact
   and a git call added later still inherits it; what changes is that the
   failing command is an ordinary external command, which bash 3.2 suppresses
   correctly. Row 4 of the table above is this form, measured.
2. **Stop trusting `$?` in the trap.** A `completed` flag beside `marked`, set
   to 1 immediately before the `SPRT-RUN-DONE` line, and the trap fires the
   failure marker whenever `marked == 0 && (status != 0 || completed == 0)`,
   exiting 1 when the status it was handed was 0. Version-independent: it
   asserts the script reached its own end rather than asking the shell how it
   died.

Both carry a comment naming bash 3.2, in the style the file already uses for
the traps it has walked into.

## Why not require bash 4

`fastchess.sh` uses no feature newer than 3.2 -- checked for `mapfile`,
`readarray`, `declare -A`, `${x^^}`/`${x,,}`, `&>>` and `coproc`, none present
-- and no document in the repository states a bash requirement. Re-execing
under a homebrew bash would add a machine prerequisite to the one script every
measurement depends on, to avoid two comments. Rejected on that.

## Found alongside, and fixed first: the tree did not build on this machine

Before either defect above could be measured the build had to work, and at
`af664a2` it did not:

```
src/search.cpp:23:22: error: unused variable 'MAX' [-Werror,-Wunused-const-variable]
```

`static constexpr int MAX = SEARCH_SCORE_INF;` at `src/search.cpp:23` became
dead when the root window moved to `SEARCH_SCORE_INF` itself; `MIN` beside it
is still read at `negamax` and `quiescence`. Clang enables
`-Wunused-const-variable` for C++ under `-Wall`, gcc does not, so the Linux
machine built it silently and `-Werror` turned it into a hard failure here.
Deleted, with the paired comment above it re-worded to describe the one
constant that remains and `MAX` dropped from the "Not in the set, on purpose"
list in `src/search_params.hpp`. No behaviour: nothing read it.

**The handover check of status.md then passed exactly.** `search_bench` at
depth 9 gives 121512 / 800769 / 62907 and at depth 12 gives 639228 / 3430710 /
367858, best moves `c3d5` / `e2a6` / `d7c8q` at both -- the figures the
handover item predicts, to the node. `build_lmr_table()`'s `log` was the stated
portability exposure and Apple's `log` does not move the tree. Every
deterministic figure the repository records is therefore valid on this machine
as written, and the only figure the MacBook still lacks is throughput in games
per hour, which the first real SPRT yields for free.

## What this does not cover

Only `fastchess.sh` was read for these two shapes. `rating.sh`, `clang-format.sh`
and `books/fetch_book.sh` are not in `touches` and were not audited for the
`command`-in-`||` form; `grep -n 'command '` over the tree is what would answer
it, and it belongs to whoever next finds one of them failing on this machine.
