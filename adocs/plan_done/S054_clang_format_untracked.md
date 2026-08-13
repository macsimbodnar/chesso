id:         S054
goal:       clang-format.sh --check sees untracked source files, so a step that adds a file cannot pass a vacuous format check
accepts:    a deliberately misformatted untracked source file makes ./clang-format.sh --check exit non-zero; the same file gitignored is still skipped; a test in the fast suite asserts both and was observed failing against the old file selection; gitignored trees (build/, .ref-builds/) remain excluded
touches:    clang-format.sh, tests/
excludes:   the clang-format configuration itself, the .clang-format pin, and reformatting any existing file
decisions:
closes:
blocks:
paused_by:
done:      clang-format.sh now selects `git ls-files --cached --others --exclude-standard`, filtered to
            paths that exist and passed through `sort -u`. Measured, not asserted:
            
            Selection. 42 source paths under the old line and 42 under the new one with no
            untracked sources present, so the fix adds nothing to a clean tree. With two
            untracked probes written, 42 against 44, and `comm -13` names the delta as
            exactly those two files.
            
            Ignored trees. Paths in the new selection matching `^(build|\.ref-builds|\.no_git)`:
            0. Dropping `--exclude-standard` adds 7 -- the CMake compiler-id probes under
            build/, build-debug/ and build-prof/, plus .no_git/search_old.cpp.
            .ref-builds/'s 32 sources on disk never appear even without it, because
            .ref-builds/7b4d9a4/.git is a `gitdir:` pointer and `git ls-files --others` does
            not descend into a nested repository; the sandbox's plain .ref-builds/ directory
            is what exercises the .gitignore mechanism itself.
            
            Deleted file. With src/zz_probe_del.cpp in the index and gone from the worktree,
            the old selection makes clang-format print
            "src/zz_probe_del.cpp: No such file or directory" and exit 1; the fixed script
            exits 0.
            
            The point of the step. With the fix in place, a misformatted
            src/zz_unstaged_probe.cpp that was never staged makes --check exit 1.
            
            Test. tests/test_clang_format_script.sh, CTest name test_clang_format_script,
            label fast, TIMEOUT 60, 0.25 s, five assertions in a throwaway git repository.
            Two are preconditions -- a clean sandbox exits 0, and a misformatted TRACKED
            file exits non-zero and is named in the output -- so a script that always fails
            and a fixture that is not really misformatted both fail the test. Then the
            untracked case, the gitignored case, the deleted-file case, and a count that
            five sandboxes were built.
            
            Red. clang-format.sh:60 reverted to `git ls-files` in place and run through
            `ctest --test-dir build -L fast -R test_clang_format_script`: exactly two
            assertions failed, verbatim "FAIL: a misformatted UNTRACKED source file passed
            the check (exit 0); the file selection lists the index only" over an empty
            out.txt, and "FAIL: a file deleted from the worktree but still in the index was
            handed to clang-format (exit 1)" over "src/gone.cpp: No such file or directory".
            The three preconditions and the gitignored half passed in that run.
            
            No droppings. `git status --porcelain` before and after `ctest -L fast`: diff
            identical, five entries either way. Leftover ${TMPDIR}/chesso-clang-format.*
            roots after a green run 0, after a red run 0.
            
            Gate. `cmake --build build -j12` exit 0 with 0 lines matching warning or error;
            `ctest --test-dir build -L fast --output-on-failure` 11/11 passed, 0 failed;
            `./clang-format.sh --check` exit 0. Run with the two new files staged.
            
            Documents. DEV_MANUAL.md gained the selection and what --exclude-standard holds
            out in the Format section, and test_clang_format_script beside
            test_fastchess_script in the test section; it had never said what the check
            covers. MANUAL.md mentions no formatter -- checked, no change needed.
            README.md owner-written, untouched. No decisions.md entry: nothing was chosen
            between real options.

## Why this cuts the queue

`clang-format.sh:60` was:

```
FILES=$(git ls-files | grep -E '\.(c|cc|cpp|h|hpp|hh)$')
```

`git ls-files` with no flags lists the index. An untracked file is invisible to
it, so `./clang-format.sh --check` reported success over a file it never opened.
Reproduced at `d853843`:

```
printf 'int  main( ){int    x=1;return\nx;}\n' > src/zz_format_probe.cpp
./clang-format.sh --check   # exit 0, 42 files scanned  <-- false green
git add src/zz_format_probe.cpp
./clang-format.sh --check   # exit 1, 43 files scanned  <-- caught, only once staged
```

That check is one of the three commands in `.moltke.json`'s `test_command`, so
it runs at every step completion. **Every step that adds a source file got a
vacuous format check for exactly the files it added**, unless the author
happened to stage them first. It has already produced a false stamp: S041's
completion commit `96863ae` claimed the gate green while three lines in the two
files it added were 81 characters, and `d853843` had to repair it — the third
consecutive step to seal a claim that was not true, and this defect is the
mechanical cause of that one.

No audit reported it. It was found while completing S041, which is why `closes:`
is empty. It is placed ahead of S038 under the house rule in `AGENTS.md` §0: a
bug that has been found is fixed before anything else starts, because a known
defect in the tree contaminates every measurement taken after it. Here the
defect is *in the gate* the steps below it report through, which is the sharpest
form of that argument available.

## The fix

Select tracked *and* untracked-but-not-ignored files:

```
git ls-files --cached --others --exclude-standard
```

Three constraints on it, each of which is a way to get this wrong:

- **`--exclude-standard` is load-bearing.** `build/`, `build-debug/`,
  `build-prof/`, `.ref-builds/` and `.no_git/` hold generated and third-party
  sources. Without it the selection picks up CMake's compiler-id probes.
- **`--cached` can name a file deleted from the worktree but still in the
  index**, which clang-format cannot open. The list is filtered to paths that
  exist. The old line had the same weakness.
- **`sort -u`** so the two sets cannot double-list a path.

Everything else about the script is unchanged: same flags, same candidate
search, same version pin, same `--check` versus format-in-place split, same
exit codes.

## The test

`tests/test_clang_format_script.sh`, registered with CTest under label `fast`,
wired the way `test_fastchess_script` is: a plain `add_test` running `bash` over
the script with the path to `clang-format.sh` as its argument, so a past
revision of the script can be run through the same assertions.

It runs in a throwaway git repository — the real tree is never read or written —
and asserts five things in order, the first two being the preconditions that
make the rest evidence:

1. a sandbox whose only source file is correctly formatted exits **0**, so a
   script that always fails cannot pass this test
2. a misformatted **tracked** file exits non-zero and names that file, so the
   fixture text really is misformatted under this `.clang-format` and the check
   functions at all
3. a misformatted **untracked** file exits non-zero and names that file — the
   defect
4. the same misformatted file inside a gitignored directory exits **0**, which
   is what a whole-worktree scan would fail
5. a file the index still names but the worktree no longer has is not opened,
   preceded by asserting that the index really does still name it

Then a count that five sandboxes were built, so a case that silently did not run
is not read as a pass.

The clean fixture is `int format_fixture_ok = 0;` and a comment, deliberately
dull. A short function body is not safe: `AllowShortBlocksOnASingleLine`
collapses `int ok() { return 0; }` under clang-format 22 and leaves it alone
under 18, which made assertion 1 fail on the first draft.

## Measured

- **File selection.** 42 source paths under the old line, 42 under the new one
  with no untracked sources present — the fix adds nothing to a clean tree. With
  two untracked probes written: 42 against 44, and `comm -13` names the delta as
  exactly those two files.
- **Ignored trees.** Paths in the new selection matching
  `^(build|\.ref-builds|\.no_git)`: **0**. Dropping `--exclude-standard` adds
  **7** — the CMake compiler-id probes under `build/`, `build-debug/` and
  `build-prof/`, plus `.no_git/search_old.cpp`. `.ref-builds/`'s 32 sources on
  disk never appear even without it, because `.ref-builds/7b4d9a4/.git` is a
  `gitdir:` pointer and `git ls-files --others` does not descend into a nested
  repository. Held out twice over; the sandbox's plain `.ref-builds/` directory
  is what exercises the `.gitignore` mechanism itself.
- **Deleted file.** With `src/zz_probe_del.cpp` in the index and gone from the
  worktree, the old selection makes clang-format print
  `src/zz_probe_del.cpp: No such file or directory` and exit 1; the fixed script
  exits 0.
- **The point of the step.** With the fix in place, a misformatted
  `src/zz_unstaged_probe.cpp` that was never staged makes `--check` exit 1.
- **No droppings.** `git status --porcelain` before and after
  `ctest -L fast`: identical. Leftover `${TMPDIR}/chesso-clang-format.*` roots
  after a green run 0, after a red run 0. The first draft leaked every sandbox —
  `sandboxes+=("$tmp")` ran inside the `$(make_sandbox)` command substitution,
  so the parent's array was empty and the cleanup loop removed nothing. Replaced
  with one root and a `trap … EXIT`.
- **Red.** With `clang-format.sh:60` reverted in place and run through
  `ctest --test-dir build -L fast -R test_clang_format_script`, verbatim:

```
FAIL: a misformatted UNTRACKED source file passed the check (exit 0); the file selection lists the index only
--- /tmp/chesso-clang-format.KsVCEX/case.0p2lGa/out.txt ---
--- end ---
FAIL: a file deleted from the worktree but still in the index was handed to clang-format (exit 1)
--- /tmp/chesso-clang-format.KsVCEX/case.syN8Pe/out.txt ---
    src/gone.cpp: No such file or directory
--- end ---
/home/max/ws/chesso/clang-format.sh: 2 assertion(s) failed
```

  The three preconditions and the gitignored half passed in that run, which is
  what makes these two failures about the selection rather than about the
  harness.

## Documents

`DEV_MANUAL.md` said only that the check pins clang-format 22; it never said
what it covers, which is the sentence that would have been wrong. Two additions:
the selection and what `--exclude-standard` holds out, in the `Format` section,
and `test_clang_format_script` beside `test_fastchess_script` in the test
section. `MANUAL.md` is end-user facing and mentions no formatter at all —
checked, no change needed. `README.md` is owner-written and untouched.

No `decisions.md` entry: nothing was chosen between real options. The old line
was a defect and `git ls-files --cached --others --exclude-standard` is the
documented way to ask git the question the script was already asking.
author:    Maksym Bodnar
