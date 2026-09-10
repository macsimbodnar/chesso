id:         S196
goal:       the fault-injection driver becomes `tools/mutation_check.py` over a tracked mutant list, so the suite's kill rate is measured by running it and a new search rule ships with a mutant its test kills
accepts:    `tools/mutation_check.py` takes a mutant file and a worktree path, applies each mutant, builds, runs the fast label and the bench, reverts, and prints the kill table with the failing assertion per mutant; the 33 mutants of `adocs/data/2026-09-04_test_review/mutants.py` move to `tools/mutants/` in the two `(void)` forms that compile under `-Werror`; the tool refuses an ambiguous anchor and reports an equivalent mutant (bench unchanged, suite green) apart from a survivor; a full pass at the current tree reproduces 31 of 32, and 32 of 32 once S193 has landed; `DEV_MANUAL.md` "Test" documents the tool, the per-rule mutant rule of DEC-141 and the cost of a full pass; the evidence directory keeps its copy unchanged (`adocs/data/README.md`: added, never edited)
touches:    tools/mutation_check.py, tools/mutants/, tests/test_mutation_check.py, tests/CMakeLists.txt, adocs/data/, DEV_MANUAL.md
excludes:   mull, dextool or any dependency (DEPS rule; considered in `adocs/testing_strategy.md` section 5); changing any mutant's meaning
decisions:  DEC-139, DEC-141
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-10
done:       2026-09-10. `tools/mutation_check.py` and the 40 mutants of
            `tools/mutants/` are the standing instrument, and the first full
            pass on this workstation reads **39 of 39 killed, 100 %** -- M26 the
            one declared equivalent, **nothing survived, nothing stillborn,
            nothing unmeasured**. 3948 s wall, 40 mutants over a baseline that
            was green at 31 tests with `bench 26851183`, worktree
            `.ref-builds/mut` at `44440b4`. The table is
            `adocs/data/S196_full_pass.tsv` (DEC-166) and it is what the next
            pass diffs against. `src/` is untouched, so no Bench line is owed
            and no INV-6, timing or Debug self-play is due.

            **M19 is dead and the tool is what proves it.** The `accepts` asked
            for 31 of 32 at the current tree and 32 of 32 once S193 landed;
            S193 and S191 had both landed when this started, so the list is 40
            and the score is 39 of 39. The fifty-move survivor of the
            2026-09-04 review is `killed` by `test_search`, alone.

            **The run found a bug in the rule the guide wrote for it, and it
            cost two mutants of the forty. DEC-165.** "Any `(Timeout)` row is
            `unmeasured`, not a kill" was written against a real trap -- a busy
            machine hits a 60 s ceiling and a naive parser reads the non-zero
            exit as detection. But `M22_castling_rights_on_capture` leaves
            `test_uci_surface` running at its ceiling, **9.26 s on the
            unmutated worktree in the same run**, while `test_chesso`,
            `test_movegen`, `test_engine` and `test_invariants` fail on
            assertions; `M31_lazy_bound_no_margin` does the same beside
            `test_search` and `test_engine`. The hang is the mutant's, so the
            prescribed re-run on a quiet machine reproduces it exactly and the
            kill is hidden for good. Those two are the only ceilings in the
            whole pass. Narrowed: `unmeasured` only when the ceiling is the
            whole evidence. Two further readings of the same evidence went in
            with it -- `killed` is decided before `unmeasured`, so a mutant that
            leaves the engine unable to print a bench line still reports its
            kill, and a non-zero `ctest` that ran nothing reads `unmeasured`
            rather than a kill. The first pass, on the pre-fix tool, was
            abandoned at 17 of 40 and re-run whole rather than patched with two
            rows from a second run.

            **What the table says beyond the score.** 19 of the 40 are caught by
            a single binary, and **not one of them is caught by
            `test_mate_carry` alone**: 15 by `test_search`, 3 by `test_engine`,
            1 by `test_chesso`. That is F02 and F03 closed and measured --
            the 2026-09-04 review found five guards whose only catcher was that
            golden, and DEC-142 makes a kill by a golden alone a weak one.
            `test_mate_carry` is red on 11 of 40 now, against 21 of 22 search
            mutants in the review, which is S204's narrowing showing up from
            the other side. The bench signature is still blind to a quarter:
            **10 of 40 leave the total and all eight best moves unchanged** and
            the suite catches 9 of them, M26 being the equivalent -- so a still
            bench still argues nothing.

            **Acceptance, run on the shipped tool after the pass.** `--only M26`
            reproduces its row exactly -- `yes 0/31 same equivalent equivalent`,
            168 s including the baseline -- which is also what proves the three
            edits made while the pass ran (a flushed header, `n/a` for a bench
            that would not read, one corrected citation) moved no verdict. The
            four refusals fire on the real tree in one run and before any write:
            `int depth` in `src/search.cpp` **occurs 9 times**, an invented
            anchor 0 times, a pair that replaces its anchor with itself, and a
            `file` outside `src/`. Worktree clean after, `MUTATION-RUN-FAILED`
            last, exit 1.

            **The mutants are the review's, unaltered, and that is checked
            rather than asserted.** All 40 ids, files and `(old, new)` pairs
            compare byte for byte against `adocs/data/2026-09-04_test_review/mutants.py`
            and `adocs/data/S191_mutants.py`; every one carries an `origin`; M26
            is the only `expected="equivalent"`. Both evidence files are
            unchanged, as is the whole of `adocs/data/2026-09-04_test_review/`
            (`adocs/data/README.md`: added, never edited).

            **The tool's own gate, and it holds to DEC-141's rule.**
            `tests/test_mutation_check.py` is registered in the fast label,
            **21 cases in 1.33 s** under `ctest`, over a throwaway git
            repository with `cmake`, `ctest` and the engine as stubs on PATH.
            Every verdict branch has a case -- killed, survived on a moved
            bench, the bench-blind survivor, equivalent, stillborn, unmeasured
            -- as does every refusal in `validate()`, and **each was observed
            red under a cut to the guard it names before it was kept**: the four
            of DEC-165 against the tool's earlier semantics, and the six others
            against a removal of the branch they guard. It never builds the
            engine, so the fast suite grew by 1.33 s and not by an hour.

            **Gate.** `ctest -L fast` **32 of 32 in both builds**, 80.98 s
            Release and 81.76 s tune, `./clang-format.sh --check` exit 0 with
            `CLANG_FORMAT_MAJOR=22` (DEC-146). `plan_prose_check.py --touches`
            0 flagged, `--citations` 0 flagged. `git status --porcelain -- src/`
            empty.

            **Docs.** `DEV_MANUAL.md` "Mutation check, `tools/mutation_check.py`"
            at the end of "Test": the command and the worktree recipe including
            **the submodule line `fastchess.sh` does not need and this tool
            does**, Release-only and why `-Werror` makes it a fact about the
            mutant, the verdict table with DEC-165's amendment, equivalence as
            a declaration, DEC-141 clause 2 in full, and the cost measured
            (66 minutes for a pass, 168 s for one row). `MANUAL.md`: no UCI
            surface change -- nothing under `src/` moved and `test_uci_surface`
            passes -- concluded unchanged. `adocs/specs.md`: no behaviour
            change and it carries no inventory of test binaries; the kill rate
            is a test fact and lives in the evidence file, concluded unchanged.

            **Decisions.** DEC-165 (the ceiling rule, narrowed on the run's own
            evidence) and DEC-166 (the owner's answers: the full pass stays on
            demand and out of S197's weekly script, and its table is evidence
            under `adocs/data/`). Section 10's four questions are all answered
            in the file.

## Why this exists

The 2026-09-04 test review measured the suite's fault detection for the first
time -- 31 of 32 non-equivalent hand-written bugs caught, about 50 minutes of
machine -- and the literature it drew on names the mutation score as the
accepted measure of a suite's effectiveness, correlated with real-fault
detection independently of coverage (`adocs/testing_strategy.md` section 3.1).
S145 applied the same discipline to one test ("observed red under a stated
mutation"); DEC-141 makes it the rule for every new search rule, and this
step is the tool the rule needs.

## Cost

An hour to make the driver a tool and move the list; a full pass is about 50
minutes of machine and is not part of any gate.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

A tool step. Nothing in `src/` changes, no game is played, no Bench line is
owed. The 2026-09-04 test review wrote a throwaway driver
(`adocs/data/2026-09-04_test_review/run.py`) that applied 33 hand-written bugs
to the engine one at a time, rebuilt, ran the fast suite and a node-count
bench, and reverted; that pass is where the "31 of 32" figure comes from. This
step turns the driver into `tools/mutation_check.py`, moves the bug list into a
tracked directory `tools/mutants/`, and documents both. Two uses follow. The
per-step use: an agent that adds a pruning, reduction or extension rule writes
one mutant that removes its guard and runs the tool with `--only` to prove its
new test goes red (DEC-141, clause 2). The full use: every mutant, about 50
minutes of machine, to re-measure the suite's kill rate after the tests change.
The evidence directory is left byte-for-byte as it is; the live copy is the one
under `tools/`.

### 2. The technique as published

Mutation testing, per https://en.wikipedia.org/wiki/Mutation_testing: change
a program in one small way -- "each changed version is called a mutant"; a
suite that fails on it has detected it -- "rejection is called killing the
mutant"; "the mutation score is the number of mutants killed / total number of
mutants"; a mutant "behaviorally equivalent to the original one" is an
"equivalent mutant", and "equivalent mutants detection is one of the biggest
obstacles to practical usage of mutation testing" -- undecidable in general,
so a person declares it. The article credits Lipton (1971) and DeMillo, Lipton
and Sayward (1978). Jia and Harman's TSE 2011 survey gives the same
definitions; both URLs tried for it failed (`unverified`). Just et al., FSE
2014 (`adocs/testing_strategy.md` section 3.1) is why the score is worth
measuring: it "correlates with real-fault detection independently of coverage".

Chesso's form, two decided differences from the tools. Mutants are
hand-written, one per plausible engine bug -- "a guard dropped, a sign
flipped, an off-by-one" -- not operator-generated; Mull and dextool were
refused under DEPS (`adocs/testing_strategy.md` section 5, "Mull or dextool").
And the suite gets a second oracle, the bench signature: OpenBench requires a
`bench` that "must report a final node count" from engines that "must produce
the same nodes result every time"
(https://github.com/AndyGrant/OpenBench/wiki/Requirements-For-Public-Engines);
Stockfish defines a functional change as one that leads "to a different search
tree" (https://github.com/official-stockfish/Stockfish/wiki/Developers). A
mutant whose bench total moves has changed the tree, so a green suite on it is
a proved gap and it cannot be equivalent. The converse fails: 12 of the 33
review mutants left the depth-9 counts still while the suite caught them
(`adocs/audit/2026-09-04_test_review.md`, "The bench signature is blind to a
third of them"), so a still bench never argues equivalence. Equivalence is
declared in the mutant file, never inferred by the tool.

### 3. What chesso has today, and where the change plugs in

**The list.** `adocs/data/2026-09-04_test_review/mutants.py`: a helper
`m(mid, file, klass, note, *pairs)` appends to a list `M`; each pair is
`(old, new)` and the docstring's rule is "`old` must occur exactly once in
`file`". 34 entries -- M01 to M33 with M06 split into `M06a`/`M06b` -- which
the audit counts as 33 mutants; M26 is the declared equivalent (the audit's
argument: "a two-ply repetition needs two consecutive null moves, which
`prev_move != 0` forbids"), M19 the survivor, hence 32 non-equivalent and 31
killed. Twelve anchors span lines (M03, M06a, M06b, M07, M08, M12, M15, M17,
M23, M27, M28, M32); M13 carries two pairs applied together. All 34 re-checked
today at HEAD `ac9cd36`: every anchor occurs exactly once. The tool runs that
check over the whole list before touching any file.

**The driver.** `adocs/data/2026-09-04_test_review/run.py`: `apply` counts
each anchor and refuses `n != 1`; `revert` runs `git checkout -- src`;
`bench` parses `tools/search_bench.py` output with
`^\s+(\w+)\s+[\d.]+s\s+(\d+) nodes\s+\d+ knps\s+best (\S+)`; `ctest` parses
`(\d+)% tests passed, (\d+) tests failed out of (\d+)` and the per-test
`N - name (Failed|Timeout)` lines; `main` builds with `-j8`, runs the
baseline, loops, prints `MUTATION-RUN-DONE`. Five habits the tool drops: the
hard-coded worktree (`adocs/data/mut`); continuing on a red baseline; writing
`results.tsv` into the evidence directory; not verifying the tree is clean
after a revert; not extracting the failing assertion (`kills.txt` was made
separately). `results.tsv` lacks rows for M08 and M12 -- their `(void)`
re-runs were never appended -- while `kills.txt` has all 34; the evidence
stays as it is (`adocs/data/README.md`: "added, never edited").

**`-Werror` and the `(void)` forms.** The root `CMakeLists.txt` adds
`-Wall -Wextra -Werror` to every non-MSVC build and, in `Debug` only,
`-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter`. So
in Release a mutant that deletes the last use of a local or a parameter does
not compile -- the review's first M08 and M12 "did not compile under `-Werror`
(an unused variable each)". The shipped forms consume the symbol: M08 inserts
`(void)is_check_move;` before the reduction condition in `src/search.cpp`
`negamax` (the local's only use), M12 inserts `(void)depth;` in
`src/search.cpp` `tt_entry_answers`. The tool always builds Release, as the
gate does; a `(void)`-less mutant compiles in Debug and measures nothing.

**The bench oracle.** S189 (Open 4, before this step) adds `bench` to the UCI
dispatch table of `src/chesso.cpp`, printing per-position `bestmove` lines
and last `<total> nodes <nps> nps`; the safe pipe is `printf 'bench\nquit\n'
| WT/build/src/chesso` because `bench` runs synchronously, and the regex is
`^[0-9]+ nodes [0-9]+ nps$` (S189's section 8). If `bench` is absent from
`uci_command_names()` output, fall back to `tools/search_bench.py` at depth 9
over its `POSITIONS` and say so in the table header. Never read the baseline
from a commit message: measure it on the unmutated worktree in the same run.

**Interface.** `python3 tools/mutation_check.py MUTANTS WORKTREE [--only ID
...] [--build-dir DIR] [--label fast] [--jobs N] [--log-dir DIR]`. `MUTANTS`
is one `.py` file or a directory whose `*.py` files are loaded in sorted
order; `WORKTREE` is a linked git worktree; `--build-dir` defaults to
`WORKTREE/build`; `--label` defaults to `fast`; `--jobs` to `os.cpu_count()`;
`--log-dir` to `WORKTREE/build/mutation/`, which `.gitignore`'s `build*`
covers. Standard library only (`argparse`, `subprocess`, `re`, `os`, `sys`,
`time`), like `tools/search_bench.py`.

**Mutant file format.** Keep the `m(...)` form so S191's
`adocs/data/S191_mutants.py` folds in unchanged: the tool `exec`s each file
with `m` pre-bound, so a mutant file is only `m(...)` calls and path
constants. Two new keywords: `expected="killed"` (default) or `"equivalent"`,
and `origin="2026-09-04_test_review"` or a step id. Files:
`tools/mutants/search.py` (M01 to M21), `board.py` (M22 to M28), `eval.py`
(M29 to M31), `time.py` (M32, M33), and `S191_guards.py` for S191's six if
S191 (Open 14) has landed. Ids are never reused: a new mutant takes the next
free `M<nn>` across all files. Before writing anything the tool refuses an
anchor with count `!= 1`, a replacement equal to its anchor, a `file` outside
`src/`, and a duplicate id.

**The loop, in prose.** (1) Refuse unless `git -C WT rev-parse --git-dir`
differs from `--git-common-dir`; equal means the main tree (verified today:
`.git`/`.git` in the main tree, `.git/worktrees/fcf0025`/`.git` in a
`.ref-builds/` worktree). (2) Refuse unless `git -C WT status --porcelain --
src` is empty. (3) Validate every mutant as above. (4) Build, run `ctest
--test-dir BUILD -L fast --output-on-failure` into a log, bench; a red
baseline prints `MUTATION-RUN-FAILED` and exits -- every later row would be
uninterpretable. (5) Per mutant: apply all pairs; `cmake --build BUILD -jN`
into a log, and on failure verdict `stillborn` with the first `error:` line;
else bench, then ctest into a log; then `git -C WT checkout -- <file>` per
mutated file and re-check (2) -- a leftover mutant contaminates every later
row, so a dirty tree here is `MUTATION-RUN-FAILED`. (6) Table, score,
`MUTATION-RUN-DONE`. Every exit path, exceptions included, ends with one of
the two markers (WATCHERS rule).

**Verdicts.** Suite red: `killed`, whatever the bench did. Suite green and
bench moved: `survived` (a behaviour change no test sees). Suite green, bench
same, `expected="equivalent"`: `equivalent`. Suite green, bench same,
`expected="killed"`: `survived` (bench-blind, the M19 class). Any `(Timeout)`
row: `unmeasured`, not a kill. **Amended 2026-09-10, on the run: only when
the ceiling is the whole evidence.** A `(Timeout)` beside a `(Failed)` is a
kill -- M22 hangs `test_uci_surface` past its 60 s ceiling, 9.26 s unmutated,
while `test_chesso`, `test_movegen`, `test_engine` and `test_invariants` fail
on assertions in the same run. The hang is the mutant's, so a quiet machine
reproduces it and `unmeasured` would hide that kill for good. Proposed as
DEC-165. Score printed as `killed / (total - equivalent
- stillborn - unmeasured)`. Exit 0 only when every mutant's verdict equals its
`expected` and nothing is stillborn or unmeasured; that exit is what a
completing step relies on.

**Kill table.** One row per mutant: `id`, `built`, `fast failed/total`,
`bench same|moved`, `verdict`, `expected`, then every failing binary with its
`TEST_CASE` title and first assertion in `kills.txt`'s form (`[test_search]
title | CHECK( ... ) is NOT correct!  (+n more)`). Doctest prints `TEST CASE:
<title>` (two spaces) and `<path>:<line>: ERROR: <MACRO>( <expr> ) is NOT
correct!`; both strings are in `tests/doctest/doctest/doctest.h` -- confirm
them on your own M01 log before fixing the regex. Non-doctest tests
(`test_perft`, the script and python tests) contribute their ctest `(Failed)`
line and first `FAIL:` line.

**Order of edits.** Tool; mutant files; run `--only M26` (must print
`equivalent`) and `--only M08 --only M12` (must build); the full pass
detached; the self-test (section 6); DEV_MANUAL.

### 4. Constants and seeds

No engine constant. The tool's own: `--jobs` default `os.cpu_count()` (the
TESTS rule's `-j8` is this MacBook's count; the workstation's differs);
per-run ctest ceiling 3600 s and build ceiling 600 s, from `run.py`; the bench
depth is S189's fixed one. The kill-rate baseline to hold is the repository's
own measurement, 31 of 32 (`adocs/audit/2026-09-04_test_review.md`); 32 of 32
once S193's fifty-move pair kills M19.

### 5. Interactions and traps

- **The worktree is at a commit.** Create it as `fastchess.sh` does:
  `git worktree add --detach .ref-builds/mut HEAD`, then `cmake -S
  .ref-builds/mut -B .ref-builds/mut/build -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache`. Uncommitted `tests/` work reaches it
  by `git diff HEAD -- tests | git -C .ref-builds/mut apply`; uncommitted
  `src/` work is committed first -- the tool refuses a dirty `src/` because
  `git checkout --` would revert more than the mutant.
- **Serial ctest.** The 52 s figure and the review's 40.7 to 63.9 s are
  serial, and S193 records fixed temp-file names colliding under a parallel
  `ctest`. Stay serial until S193 lands.
- **Timeouts read as kills to a naive parser.** Release ceilings are 60 s per
  test (`test_mate_breadth` 24.4 s against 120 s); a busy machine produces
  `(Timeout)`. No pass beside a match (MACHINE rule: the coordinator holds the
  machine); re-run an `unmeasured` row with `--only`. **And a timeout is not
  always the machine's** -- see the amended verdict rule in section 3: M22 hangs
  a test that passes in 9.26 s unmutated, so a row carrying both a `(Timeout)`
  and a `(Failed)` is a kill and re-running it changes nothing.
- **Single points of detection.** The review found ten mutants caught by one
  case each and `test_mate_carry` red on 21 of 22 search mutants; a kill by a
  golden alone is weak (DEC-142), and S191's `accepts` asks for red "in a
  binary other than `test_mate_carry`". The table lists every failing binary
  so that is read off it; do not collapse to a count.
- **Pipes hide `ctest`'s status** (DEV_MANUAL "Test"); read `returncode` from
  `subprocess.run`, never a pipeline's.
- **The Debug build would hide `-Werror`** (section 3); Release only, and the
  tune build adds nothing here: it differs only in `src/search_params.hpp`,
  which no mutant touches, and doubling the pass buys no verdict.
- **Evidence is append-only.** Copy, do not move: `git diff --stat HEAD --
  adocs/data/` is empty at completion.
- **Cite the mutant, not a line.** New mutants anchor on a phrase; DEC-135
  applies to the `note` text too.

### 6. Tests

A self-test `tests/test_mutation_check.py` (stdlib `unittest`, the
`tests/test_spsa_driver.py` shape) in a sandbox: `mktemp -d`, `git init`, a
fake `src/x.cpp` committed, a linked worktree of it, and stub `cmake`, `ctest`
and engine scripts first on `PATH` (the `tests/test_fastchess_script.sh`
technique). The stub `ctest` prints a fabricated `--output-on-failure`
transcript -- one `N - test_x (Failed)` line, a `TEST CASE:  title` line, one
`is NOT correct!` line -- when the mutated text is in `src/x.cpp`, else `100%
tests passed`; the stub engine prints a canned bench total that changes on a
second marker string. Cases: an ambiguous anchor refused before any write,
worktree clean; a missing anchor likewise; a killed row carries the assertion
text; `expected="equivalent"` with green suite and same bench reads
`equivalent`; `expected="killed"` that survives exits non-zero; the main tree
refused; `git status --porcelain` empty in the sandbox worktree after any
run; `MUTATION-RUN-FAILED` last on every refusal. Register it:

```
add_test(NAME test_mutation_check
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py)
set_tests_properties(test_mutation_check PROPERTIES LABELS "fast" TIMEOUT 60)
```

Registration needs `tests/` and `tests/CMakeLists.txt` in `touches:` (section
10); the argument for it is DEC-118's: a tool nobody runs for weeks is found
broken by whoever next needs it. Acceptance runs on the real tree: `--only
M26` reports `equivalent`; a fabricated file anchoring on `int depth` in
`src/search.cpp` (many occurrences) is refused; the full pass reproduces the
baseline. No INV-6 run and no Debug self-play: `src/` is untouched, shown by
`git diff --stat HEAD -- src/` empty in both the main tree and the worktree.

### 7. Measurement

No SPRT, no timing lane; the number is the kill table. Cost from
`results.tsv`: rebuild 1.2 to 2.7 s, fast suite 46.8 to 97.3 s per mutant on
this MacBook -- about a minute each, so 34 entries plus baseline is 35 to 40
minutes (the review's "about 50 minutes" shared the machine); 40 entries with
S191's six, about 45. Measure the workstation's figure into the stamp and
DEV_MANUAL. The full pass is detached (`nohup python3
tools/mutation_check.py tools/mutants .ref-builds/mut > /tmp/S196_full.log
2>&1 &`) under the WATCHERS poll loop, pattern `MUTATION-RUN-(DONE|FAILED)`,
two-hour ceiling; a `--only` run is minutes and runs in the foreground. The
table goes in the stamp; `adocs/data/S196_full_pass.tsv` is section 10's
third question.

**How a completing step uses it (DEC-141 clause 2).** Add the mutant to the
matching `tools/mutants/*.py` with the next id and `origin="S<id>"`, commit the
test and the rule, refresh the worktree to that commit, run `python3
tools/mutation_check.py tools/mutants .ref-builds/mut --only M<nn>`, and paste
its row -- id, verdict `killed`, the killing case and assertion -- into the
stamp. A `survived` row means the guard test does not bite; fix the test, not
the mutant.

### 8. Completion checklist

1. `cmake --build build -j8 && ctest --test-dir build -L fast
   --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
   build-tune -L fast --output-on-failure && ./clang-format.sh --check` green.
2. Bench line: none owed -- the commit touches no `src/` (DEC-140).
3. `DEV_MANUAL.md` "Test" gains a "Mutation check" subsection: the command,
   the `--only` form, the verdict rules, the `(void)` rule for `-Werror`, the
   per-rule DEC-141 sentence, the cost measured, and that a full pass is in
   no gate. `MANUAL.md`: no UCI surface change, concluded unchanged.
   `adocs/specs.md`: no behaviour change; the kill-rate baseline is a test
   fact, not a spec -- concluded unchanged unless the coordinator wants it
   beside INV-6.
4. `python3 tools/plan_prose_check.py --touches` prints `touches flagged: 0`;
   `git diff --stat HEAD -- adocs/data/ src/` empty.
5. Stamp: the kill table of the full pass with its date, commit and wall
   time; the `--only M26` line; the refusal observed on the ambiguous anchor;
   the self-test's case list and whether it is registered; the S191 fold or
   its absence; the DEV_MANUAL subsection name.
6. `plan.md` and `status.md` through the coordinator; the fast check over
   the diff (REVIEW rule).

### 9. Sources read

- https://en.wikipedia.org/wiki/Mutation_testing -- definitions of mutant,
  killed, mutation score, equivalent mutant; Lipton 1971, DeMillo, Lipton and
  Sayward 1978; competent programmer hypothesis and coupling effect.
- Jia and Harman, "An Analysis and Survey of the Development of Mutation
  Testing", IEEE TSE 2011 -- `unverified`: fetches of the CREST PDF and the
  IEEE page both failed.
- https://github.com/AndyGrant/OpenBench/wiki/Requirements-For-Public-Engines
  -- the `bench` output and determinism requirements.
- https://github.com/official-stockfish/Stockfish/wiki/Developers --
  functional change defined as a different search tree; the Bench line.
- `adocs/testing_strategy.md` sections 3.1, 5 and R9 -- Just et al. 2014,
  Mull and dextool refused, the tool's recommendation.
- `adocs/audit/2026-09-04_test_review.md` -- the pass in full, 31 of 32, M26's
  equivalence argument, the `-Werror` compile failures, the timings.
- `adocs/data/2026-09-04_test_review/mutants.py`, `run.py`, `kills.txt`,
  `results.tsv` -- the driver and its evidence; anchors re-verified at HEAD.
- `adocs/decisions.md` DEC-139, DEC-140, DEC-141, DEC-142, DEC-145.
- `adocs/plan_todo/S189_bench_signature_gate.md`, `S191_pruning_guard_tests.md`,
  `S193_fast_suite_repairs.md`, `S197_gate_extra_script.md` -- the bench
  format, the six new mutants and the fold, M19's repair, gate_extra's scope.
- `CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/test_spsa_driver.py`,
  `tests/test_fastchess_script.sh`, `tests/doctest/doctest/doctest.h`,
  `tools/search_bench.py`, `fastchess.sh`, `.gitignore`, `DEV_MANUAL.md`
  "Test" -- warnings, registration form, sandbox technique, reporter strings,
  bench regex, worktree creation.

### 10. Questions deferred to the owner

**All four answered 2026-09-10.** 1: registered, and `tests/` joined
`touches:`. 2 and 3: DEC-166 -- the full pass stays on demand, S197 does not
call it, and the table goes in the stamp *and* `adocs/data/S196_full_pass.tsv`
with a line in `adocs/data/README.md`. 4: moot, S191 and S193 both landed
before this step started, so the list is 40 mutants and M19 is expected
`killed`.

1. **Registering the self-test** adds `tests/test_mutation_check.py` and a
   `tests/CMakeLists.txt` entry, outside `touches:`. Recommended; if refused,
   the self-test runs by hand and the stamp says so.
2. **The weekly full pass.** A weekly run under S197's `gate_extra` has been
   proposed, but S197's `accepts:` does not list it and DEC-141 clause 3 does
   not name it; R9 says "run after any change to `tests/` that adds or removes
   coverage". Decide whether S197 calls it, or it stays on demand.
3. **Where the full-pass table lives**: the stamp only, or also
   `adocs/data/S196_full_pass.tsv` with a row in `adocs/data/README.md`
   (outside `touches:`).
4. **If S191 or S193 has not landed** when this step starts: fold only the 34
   entries, and record 31 of 32 with M19 `survived` against `expected="killed"`
   -- the tool's exit is then non-zero by design, and the stamp says why.
