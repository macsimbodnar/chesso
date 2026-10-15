id:         S284
goal:       `fastchess.sh` takes an SPRT's bounds and a round cap that keeps the SPRT from the environment, so a pre-registration can state both and the run plays them
accepts:    (1) `fastchess.sh` reads `SPRT_BOUNDS` ("elo0=<x> elo1=<y> alpha=<a> beta=<b>", each field validated, refused when malformed) and `SPRT_ROUNDS` (an unsigned round cap; the SPRT stays on), refuses `SPRT_ROUNDS` beside `ROUNDS`, and refuses either beside a mode argument (`--fast`, `--nonreg`); (2) the banner prints the bounds and, when `SPRT_ROUNDS` is set, the cap; (3) `tests/test_fastchess_script.sh` gains a case per new path and per refusal, observed red before the change where a case can be, and one case showing the default path's fastchess command line byte-identical to `4e6ca63`'s; (4) the header comment and `DEV_MANUAL.md`'s SPRT section state the two variables, traced to the lines that consume them; (5) it lands only after S023 verdict 1's run ends; no DEC-143 A/A is owed, because no played setting moves, and the stamp says so
touches:    fastchess.sh, tests/test_fastchess_script.sh, DEV_MANUAL.md
excludes:   any change to a mode's bounds, rounds, time control, book, adjudication or concurrency; the run itself (S283)
decisions:  DEC-289, DEC-286, DEC-143, DEC-020, DEC-061
closes:
blocks:     S283
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent, phase 1 blind beside S023 verdict 1's run (DEC-260), phase 2 on fd33954
done:       2026-10-15 07:05 -- fastchess.sh reads SPRT_BOUNDS ("elo0=<x> elo1=<y> alpha=<a> beta=<b>": four numbers in that order, elo0 below elo1, alpha and beta strictly between 0 and 0.5) and SPRT_ROUNDS (a round cap above 0, no leading zero, that keeps the SPRT) below its mode block; refuses either one beside --fast, --nonreg or ROUNDS, and when malformed, before anything is built; prints the pair on the bounds line and the cap in rounds and games on a cap line; tags such a run env. tests/test_fastchess_script.sh gains cases 33 to 40: 33 to 39 observed red against 4e6ca63's fastchess.sh (25 assertions) and green here; 40, every mode's fastchess argv token for token, green on both; each side of the alpha, beta and elo range checks has its row, and five mutants of those checks each fail the file. Fast suite 46 of 46 in both builds, format clean, test_fastchess_script about 11 s of its 60 s on a shared machine; plan_prose_check.py clean in all five modes. DEV_MANUAL.md's usage block and Which bounds state both variables; MANUAL.md and specs.md need nothing. Fast check: 3 findings; 2 fixed (the S021 and S076 history sentence in fastchess.sh and DEV_MANUAL.md, case 38's missing range edges), 1 (S283's line citations) the coordinator's at S283's pin. No DEC-143 A/A is owed: no played setting moves -- not fastchess's version, the book, the adjudication or the machine -- and case 40 shows the default path's command line unchanged. Landed after S023 verdict 1's run ended H0 (7f46c0c, reverted 0cdd39a). Implemented by an Opus subagent in two phases; reviewed and committed by the coordinator.

## Why this exists

S283's pre-registration (DEC-286) cannot reach `fastchess.sh` at `4e6ca63`:
the mode argument fixes the bounds, and `ROUNDS` drops the SPRT. Found by
S283's phase-1 agent (2026-10-15), checked by the coordinator by reading the
mode block (`fastchess.sh` around lines 331-378). DEC-289 rules the step.

## As built, phase 1 -- 2026-10-15, written blind while S023 verdict 1 holds the machine

Written under DEC-260 while S023 verdict 1's SPRT ran from main's
`fastchess.sh`: nothing was compiled, run or tested. `bash -n` over the two
scripts is the only thing executed, and both passed. Every output below is an
expectation, not an observation.

### Deviations and notes, first

1. **The golden is `4e6ca63`'s and `bb20f35`'s at once.** The accepts name
   the first, the brief the second; `git diff --stat 4e6ca63 bb20f35` touches
   only `adocs/`, so `fastchess.sh` and the test file are the same blobs.
2. **The harness ran every case with `--fast`**, and either variable beside a
   mode is refused, so `tests/test_fastchess_script.sh` `run_sandbox` gains
   `SMOKE_MODE`, the harness's own name: unset it is `--fast`, set empty it is
   no argument. No existing call site changed.
3. **Stricter than the accepts in two places, both stated at the code.**
   `SPRT_BOUNDS` must be the four fields in that order, one space apart, each
   a number with a leading digit -- the string is split unquoted into
   fastchess's argv, so a fifth token such as `model=logistic` would reach it.
   `SPRT_ROUNDS` refuses a leading zero, which refuses `0` with it and keeps
   the banner's games from being read as octal.
4. **The tag is `env`** for a run that sets either variable: the PGN event
   reads `chesso env <stamp> srand=<seed>`, the default directory
   `.tuning/sprt_env_<stamp>`, the marker `SPRT-RUN-DONE env <dir>`. S283 sets
   `OUT`, so only its event and its marker carry the tag.
5. **Two sentences that taught editing the mode block for a two-sided pair**
   -- the header of `fastchess.sh` and DEV_MANUAL's "Which bounds" -- now
   point at `SPRT_BOUNDS`. DEC-289 rejected that route, and leaving them would
   contradict the new paragraph. This is past the brief's one or two
   sentences.
6. **The test's ctest TIMEOUT is 60 s**, set in `tests/CMakeLists.txt`. S284
   adds five sandboxes and 22 runs, 19 of them refused before the first git
   call. Phase 2 reads the time ctest reports.
7. `MANUAL.md` needs nothing: it is the UCI surface. `specs.md` names no
   `fastchess.sh` variable, so nothing is owed there either.
8. **Line numbers in `fastchess.sh` move.** Against `4e6ca63`, old lines 18 to
   56 sit 8 lower, 59 to 353 sit 9 lower, 354 to 922 sit 71 lower, and 923 on
   sit 76 lower. S283's script header cites lines at `4e6ca63` by that
   commit's name, so it stays true; re-pointed to this tree, its REF and CAND
   lines are 169 and 178, their resolution 452 and 459, and the line that
   appends `model=normalized` is 442.

### What changed, by line on the s284 tree

```
fastchess.sh
  18-25    header usage list: SPRT_BOUNDS, SPRT_ROUNDS, what each replaces,
           neither with ROUNDS or a mode, read below the mode block
  65-67    two-sided pair: pass it in SPRT_BOUNDS (was: edit this block)
  364-384  the block comment: why, what is refused, what is checked, the tag
  385-387  sprt_env names whichever is set; sprt_cap starts empty
  388-392  refusal beside --fast or --nonreg ($1 non-empty after the case),
           then beside ROUNDS
  394-410  SPRT_BOUNDS: anchored ERE over the four fields, then awk for
           elo0 < elo1 and 0 < alpha, beta < 0.5; sprt="$SPRT_BOUNDS"
  412-421  SPRT_ROUNDS: case refuses 0* and *[!0-9]*; sets sprt_cap, rounds
  423      tag="env"
  426-449  the ROUNDS block, unchanged; never reached with either one set
  993      the bounds line, unchanged: it prints $sprt, now SPRT_BOUNDS
  994-998  the cap line, only when SPRT_ROUNDS is set
tests/test_fastchess_script.sh
  5, 15-16, 80-99   the property count, and cases 33 to 40 in the header list
  411-417           run_sandbox's comment: SPRT_* travel like TC; SMOKE_MODE
  423-424           run_sandbox captures SPRT_BOUNDS, SPRT_ROUNDS, SMOKE_MODE
  473-482           set-or-unset of SPRT_BOUNDS and SPRT_ROUNDS
  483               ./fastchess.sh ${mode:+"$mode"}, was ./fastchess.sh --fast
  1550-1586         case 33, SPRT_BOUNDS alone
  1588-1618         case 34, SPRT_ROUNDS alone
  1620-1652         case 35, both, in S283's shape with CAND, REF, TC, HASH
  1654-1691         one refusal sandbox and expect_refusal()
  1693-1739         cases 36 to 39, 19 refusal rows
  1741-1822         case 40, the golden argv of all three modes
  1824, 1841        the cleanup, and the closing line now says 40
DEV_MANUAL.md
  3610-3611         Play games usage block: the two variables
  3998-4008         Which bounds: the paragraph on both, traced to the block
  4039-4043         two-sided pair: SPRT_BOUNDS, was edit the block
```

The banner S283's run is expected to print, between `seed` and `out`:

```
bounds     elo0=0 elo1=10 alpha=0.05 beta=0.05
cap        5250 rounds, 10500 games, from SPRT_ROUNDS; no bound by then is no verdict
```

### The new cases, and what each is expected to print

On the s284 tree every case is silent and the run ends on one line, exit 0:

```
<repo>/fastchess.sh: 40 properties hold -- reaches fastchess, ... prints
whether each side pads its branches, plays SPRT_BOUNDS and SPRT_ROUNDS as the
pair and the cap of a run with no mode argument and prints both, refuses
either beside a mode, beside ROUNDS or malformed, and with neither set hands
fastchess the argv it was handed before S284 in every mode
```

Against `4e6ca63`'s script, from a scratch copy taken with `git show`, cases
1 to 32 and 40 are expected silent and 33 to 39 red -- 22 assertions, each
followed by its sandbox's `out.txt` -- because that script reads neither name
and plays every row:

```
FAIL: SPRT_BOUNDS: fastchess was not handed -sprt elo0=-3 elo1=7 alpha=0.04 beta=0.06 model=normalized
FAIL: SPRT_ROUNDS=777 did not reach fastchess as -rounds 777 (got '20000')
FAIL: S283's shape: fastchess was not handed -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05
FAIL: SPRT_BOUNDS --fast: the run was played instead of refused
FAIL: SPRT_BOUNDS --nonreg: the run was played instead of refused
FAIL: SPRT_ROUNDS --fast: the run was played instead of refused
FAIL: SPRT_ROUNDS --nonreg: the run was played instead of refused
FAIL: SPRT_ROUNDS with ROUNDS: the run was played instead of refused
FAIL: SPRT_BOUNDS with ROUNDS: the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=0 elo1=10': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo1=10 elo0=0 alpha=0.05 beta=0.05': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=0 elo1=ten alpha=0.05 beta=0.05': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=0 elo1=10 alpha=0.05 beta=0.05 model=logistic': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=0  elo1=10 alpha=0.05 beta=0.05': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=5 elo1=5 alpha=0.05 beta=0.05': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=10 elo1=0 alpha=0.05 beta=0.05': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=0 elo1=10 alpha=0 beta=0.05': the run was played instead of refused
FAIL: SPRT_BOUNDS='elo0=0 elo1=10 alpha=0.05 beta=0.5': the run was played instead of refused
FAIL: SPRT_ROUNDS='0': the run was played instead of refused
FAIL: SPRT_ROUNDS='05250': the run was played instead of refused
FAIL: SPRT_ROUNDS='5k': the run was played instead of refused
FAIL: SPRT_ROUNDS='-5': the run was played instead of refused
<file>: 22 assertion(s) failed
```

Case 40 holding there is the proof the accepts asks for: the default path's
argv -- token for token, with the snapshot, the stamp and the machine's
concurrency normalised -- is the one `4e6ca63` hands fastchess, in all three
modes. On the s284 tree each refusal row prints its marker and exits 1 with
no `out/` directory made, for example:

```
SPRT-RUN-FAILED: SPRT_BOUNDS beside --fast is refused: --fast fixes its own bounds and round cap, and SPRT_BOUNDS would replace them; run with no mode argument
```

### Pending, phase 2 (after S023 verdict 1's run has ended)

1. `bash tests/test_fastchess_script.sh` on the s284 tree: expect the one
   line above, exit 0.
2. The same file against `4e6ca63`'s script, from a scratch copy: expect the
   22 FAIL lines above and exit 1 -- the observed red the accepts asks for --
   and case 40 silent there.
3. The TESTS chain, both builds and the format check; read the time ctest
   gives `test_fastchess_script` against its 60 s ceiling.
4. `tools/plan_prose_check.py --citations` over this file.
5. Fast check over the diff; commit. No `src/` change, so no `Bench:` line.

### Proposed `done:` stamp

`<date>. fastchess.sh reads SPRT_BOUNDS (elo0, elo1, alpha, beta; checked
whole and per field) and SPRT_ROUNDS (a round cap above 0 that keeps the
SPRT) below its mode block, refuses either beside --fast, --nonreg or ROUNDS
and when malformed, prints the pair on the bounds line and the cap in rounds
and games on a cap line, and tags such a run env.
tests/test_fastchess_script.sh cases 33 to 40: 33 to 39 observed red against
4e6ca63's fastchess.sh (22 assertions) and green here; 40, every mode's argv
token for token, green on both. Fast suite green in both builds, format
clean, <n> s for the smoke run. No DEC-143 A/A is owed: no played setting
moves -- not fastchess's version, the book, the adjudication or the machine,
and case 40 shows the default path's command line unchanged. Landed after
S023 verdict 1's run ended (<its state>). Implemented by an Opus subagent in
two phases; tested and committed by the coordinator.`

### Commit text

    Read SPRT bounds and a round cap from the environment (S284)

    S283's pre-registration (DEC-286) books {0, 10} nElo at alpha = beta =
    0.05 with a 5250-round cap, and fastchess.sh could pass neither: the
    mode argument fixed the pair and ROUNDS removed the SPRT. SPRT_BOUNDS
    and SPRT_ROUNDS now carry both, checked before anything is built,
    refused beside a mode or ROUNDS, printed on the banner. With neither
    set every mode's argv is unchanged, so no DEC-143 A/A is owed
    (DEC-289).

## Phase 2 -- 2026-10-15, on `fd33954`, the machine handed over

S023 verdict 1 ended H0 (recorded `7f46c0c`), was reverted as `0cdd39a` and
completed as `f3f4bab`; main stood at `fd33954` with the gate green. No match
was started. The phase-1 diff was saved as `s284_pre_ff.diff`, stashed, the
worktree fast-forwarded `bb20f35..fd33954`, and the stash popped with
`--index`. `DEV_MANUAL.md` auto-merged with no conflict. `fastchess.sh` and
`tests/test_fastchess_script.sh` are byte-identical between `bb20f35` and
`fd33954`, so their line map above holds. `DEV_MANUAL.md`'s three hunks moved
12 lines up, to 3598-3599, 3986-3996 and 4027-4031.

### Readings, every one as predicted in phase 1

1. **The smoke test on this tree**: the single closing line,
   `40 properties hold`, exit 0, 7.2 s wall.
2. **The same file against `4e6ca63`'s script**, a scratch copy whose blob
   (`8fc8343`) is the one `bb20f35` and `fd33954` carry: exit 1, exactly the
   22 FAIL lines listed above in that order, then
   `22 assertion(s) failed`, 8.3 s. Cases 1 to 32 and case 40 silent. That is
   the observed red of 33 to 39 the accepts asks for, and case 40 holding on
   both scripts is the default path's command line unchanged in all three
   modes. The output is kept beside the coordinator's diffs as
   `s284_red_4e6ca63.txt`.
3. **The TESTS chain in this worktree**, both builds configured fresh (Ninja,
   Release, ccache; `build-tune` with `CHESSO_TUNE=ON`), `CLANG_FORMAT_MAJOR=22`
   exported per DEC-146: `build` 46 of 46 fast tests passed in 107.2 s,
   `build-tune` 46 of 46 in 108.1 s, `./clang-format.sh --check` exit 0.
   `test_fastchess_script` took 6.56 s and 6.55 s against its 60 s ceiling.
4. **`tools/plan_prose_check.py`, all five modes**: `--prose` 0 flagged,
   `--citations` 0 over 31 files (this file: one symbol citation, 0 flagged),
   `--touches` 0 over 31 files, `--params` exit 0, `--gate` agrees. Every
   mode exit 0.

No red test, nothing loosened. The tag `env` was accepted by the coordinator.
The second tier (DEC-141) is not owed: nothing in `make_move`, `unmake_move`,
the generator or the search is touched, and no pruning rule is added.

### Fast check: 3 findings; 2 fixed, 1 the coordinator's at S283's pin

1. **False history, fixed.** The header of `fastchess.sh` and DEV_MANUAL's
   two-sided paragraph said S021 and S076 edited the mode block for the run
   and so left what played out of git. They did not: each calls `fastchess`
   itself with its own committed argument list, its `-sprt elo0=-5 elo1=5`
   line included -- line 50 of `adocs/data/S021_sprt.sh`, line 77 of
   `adocs/data/S076_sprt.sh`. Both sentences now say that. Phase 1's
   deviation 5 above carried the same error.
2. **Case 38 left two sides of the alpha and beta check untested, fixed.**
   With only alpha at 0 and beta at 0.5, a mutant dropping alpha's upper bound
   or beta's lower bound passed all 40 cases. Rows for alpha at 0.5 and beta
   at 0 were added, and one more for the elo check: elo0 = elo1 = -5, below
   the alpha beside it, so elo1 read from alpha's capture group cannot pass.
   Five mutants of the script were run through the file, one per missing
   comparison and that capture-group swap, and each failed: exactly its own
   row for the four comparisons, and two rows for the swap, the -5 row among
   them.
3. **S283's line citations into `fastchess.sh`**, pinned at `4e6ca63`, are the
   coordinator's to re-point at S283's pin (deviation 8 above has the shifts).

The test file's lines after case 38's opening sit 6 lower than the phase-1
map: case 38 at 1711-1739, 39 at 1741-1745, 40 at 1747-1828, the cleanup at
1830 and the closing line at 1847. The `fastchess.sh` and DEV_MANUAL edits kept their
line counts.

Readings after the fixes, the machine shared with a compile not this step's
(one-minute load 2.6 when the chain started):

1. The smoke test on this tree: `40 properties hold`, exit 0, 12.0 s.
2. Against `4e6ca63`'s script: exit 1, **25** FAIL lines -- the 22 above plus
   `elo0=-5 elo1=-5`, `alpha=0.5 beta=0.05` and `alpha=0.05 beta=0`, each
   "the run was played instead of refused" -- then `25 assertion(s) failed`.
   Case 40 still silent. `s284_red_4e6ca63.txt` holds this run.
3. TESTS chain: `build` 46 of 46 in 125.7 s, `build-tune` 46 of 46 in
   120.9 s, format exit 0; `test_fastchess_script` 10.57 s and 10.84 s of
   its 60 s.
4. `plan_prose_check.py`, all five modes: exit 0, nothing flagged.

### Final proposed `done:` stamp (supersedes the phase-1 draft)

`2026-10-15 HH:MM -- fastchess.sh reads SPRT_BOUNDS ("elo0=<x> elo1=<y>
alpha=<a> beta=<b>": four numbers in that order, elo0 below elo1, alpha and
beta strictly between 0 and 0.5) and SPRT_ROUNDS (a round cap above 0, no
leading zero, that keeps the SPRT) below its mode block; refuses either one
beside --fast, --nonreg or ROUNDS, and when malformed, before anything is
built; prints the pair on the bounds line and the cap in rounds and games on a
cap line; tags such a run env. tests/test_fastchess_script.sh gains cases 33
to 40: 33 to 39 observed red against 4e6ca63's fastchess.sh (25 assertions)
and green here; 40, every mode's fastchess argv token for token, green on
both; each side of the alpha, beta and elo range checks has its row, and five
mutants of those checks each fail the file. Fast suite 46 of 46 in both
builds, format clean, test_fastchess_script about 11 s of its 60 s on a
shared machine; plan_prose_check.py clean in all five modes. DEV_MANUAL.md's
usage block and Which bounds state both variables; MANUAL.md and specs.md need
nothing. Fast check: 3 findings; 2 fixed (the S021 and S076 history sentence
in fastchess.sh and DEV_MANUAL.md, case 38's missing range edges), 1 (S283's
line citations) the coordinator's at S283's pin. No DEC-143 A/A is owed: no
played setting moves -- not fastchess's version, the book, the adjudication
or the machine -- and case 40 shows the default path's command line
unchanged. Landed after S023 verdict 1's run ended H0 (7f46c0c, reverted
0cdd39a). Implemented by an Opus subagent in two phases; reviewed and
committed by the coordinator.`

### Commit text (final)

    Read SPRT bounds and a round cap from the environment (S284)

    S283's pre-registration (DEC-286) books {0, 10} nElo at alpha = beta =
    0.05 with a 5250-round cap, and fastchess.sh could pass neither: the
    mode argument fixed the pair and ROUNDS removed the SPRT. SPRT_BOUNDS
    and SPRT_ROUNDS now carry both, checked before anything is built,
    refused beside a mode or ROUNDS, printed on the banner, and the run is
    tagged env. Cases 33 to 39 of the smoke test were observed red against
    4e6ca63's script; case 40 shows every mode's argv unchanged with
    neither set, so no DEC-143 A/A is owed (DEC-289).
