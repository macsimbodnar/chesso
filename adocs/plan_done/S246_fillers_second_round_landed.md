id:         S246
goal:       S242's and S243's second-round fix-ups, proved by their agent and described by their stamps, land in the tree they were left out of -- the coordinator's landing script read the fillers' worktree index, where the first round had been staged, instead of its working tree
accepts:    `tests/test_engine.cpp` at HEAD is byte-identical to blob `e88103f` (S242's DEC-240 form: the timer half on its own board with a 402 ms bound asserted once, the stop half's clock precondition, the case renamed), the form `aef0257`'s message and `plan_done/S242`'s stamp describe; `tests/test_search.cpp` carries S243's `CHECK(last_score < MATE_MIN_LOCAL)` with its comment as fixture `28723ae` holds them; `DEV_MANUAL.md`'s `mate_the_multicut_hides` row carries S243's sentence on how the row's going stale now shows; both fast suites green in both builds, the format check and the prose checks clean; `tools/mutation_check.py --only E21` on a clean fixture of the landed tree kills E21 at the new CHECK; a 200-run focused loop of the renamed case with 0 failures and no retry on the timer half; the stamp names `aef0257` and `6eb2674` as the commits whose descriptions this landing makes true
touches:    tests/test_engine.cpp, tests/test_search.cpp, DEV_MANUAL.md
excludes:   any change of either step's design; the S022 v2 board re-derivation, which follows on this tree
decisions:  DEC-240, DEC-238, DEC-142
closes:
blocks:     S022 verdict 2's board re-derivation
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); created by the coordinator 2026-09-29
done:       2026-09-29 -- S242's and S243's second-round forms, proved by their agent and described by their stamps, are in the tree `aef0257` and `6eb2674` left them out of, and both commits' descriptions are true of it from this landing on: `tests/test_engine.cpp` is blob `e88103f`, S242's DEC-240 form (renamed "a stop inside the first iteration cuts it and the hard timer ends the search within its bound", the hard-timer half on its own board with a 402 ms bound asserted once, the stop half's clock precondition), which `aef0257`'s message and `plan_done/S242`'s stamp describe; `tests/test_search.cpp` carries S243's `CHECK(last_score < MATE_MIN_LOCAL)` with its comment, byte-equal to fixture `28723ae`'s file, and `DEV_MANUAL.md`'s `mate_the_multicut_hides` row carries S243's sentence on how the row's going stale now shows, beside `39841ed`'s -- what `6eb2674`'s message and `plan_done/S243`'s stamp describe. Cut with `git diff 6eb2674 28723ae` over the three files, applied three-way without staging and rebased onto `39841ed` at the coordinator's direction; the one conflict, that row both times, resolved by keeping both sides and checked three ways; nothing else moves. Both fast suites 41 of 41 in both builds, the format check and the three prose checks clean; `tools/mutation_check.py --only E21` on a clean fixture of the rebased tree kills E21 at the new CHECK (`48998 < 48000`, then the branch) and at the mined row, fast 1/41, bench same, baseline 3429473; 200 focused runs of the renamed case, 0 failed, all at 7 assertions, no retry. Documents and tests only, `src/` untouched, so no `Bench:` line and no second tier; `MANUAL.md` and `specs.md` checked, no change. `plan_done/` is not edited; this stamp records the correction. Written by an Opus subagent briefed by the coordinator.

## Why this exists

The verdict-2 agent of S022 found on 2026-09-29 that no commit carries the
case name S242's stamp gives ("a stop inside the first iteration cuts it and
the hard timer ends the search within its bound"): `aef0257` landed the
fillers agent's **first** redesign of `tests/test_engine.cpp` -- the retried
hard-timer claim DEC-240 rejected -- and `6eb2674` landed S243 without the
returned-score CHECK its report added at the fast check and without
`DEV_MANUAL.md`'s sentence. The cause is the coordinator's: the landing
script cut each step's patch with `git diff --cached` from the fillers'
worktree, whose index held the first round (staged by the coordinator to
produce the review patch), while the second-round fix-ups sat unstaged in
the working tree, which the worktree's removal then discarded. The final
forms survive as the fillers agent's fixture commit `28723ae` (13caf43 plus
both steps' final tests, byte-equal in the three files to what its reports
measured) and as blob `e88103f`. Two done stamps are false against the tree
until this lands; `plan_done/` is history and is not edited -- this step's
stamp records the correction.

## Shape

Take the three files' second-round hunks from `git diff 6eb2674 28723ae --
tests/test_engine.cpp tests/test_search.cpp DEV_MANUAL.md` and apply them
onto HEAD (S022 verdict 1 has touched `tests/test_search.cpp` and
`DEV_MANUAL.md` since; a three-way apply), then prove the result: the
engine file equal to the blob, the CHECK present and red under E21, the
suites, the focused loop. Nothing else moves.

## What was found, 2026-09-29

In the linked worktree `chesso-s246`, first on `5bf73d3` and then, at the
coordinator's direction, on `39841ed`, S022 verdict 1's revert. Every command
niced at 19, builds `-j4`, two other agents on the machine and no match. Logs
under `.tuning/coord/S246_logs/`.

- **The three files are the whole of it.** Against `6eb2674` the fixture
  `28723ae` differs in `src/`, `tests/` and `tools/` only in
  `tests/test_engine.cpp` and `tests/test_search.cpp`: the other two paths
  of the two steps, `tests/test_helpers.hpp` and
  `tools/mutants/S097_singular_extension.py`, landed final in `aef0257` and
  `6eb2674`. Among the documents it differs in one row of `DEV_MANUAL.md`
  and in the two step files, which are history in `plan_done/` and stay as
  they are.
- **`tests/test_engine.cpp` had not moved since `aef0257`**, so the second
  round applies onto its own preimage and the file comes out as blob
  `e88103f` byte for byte, on either base.
- **`tests/test_search.cpp` is the fixture's whole file after the rebase.**
  S022 verdict 1 had changed it elsewhere, and the three-way apply onto
  `5bf73d3` merged around that; `39841ed` put `tests/` back to `d446783`'s,
  which is `6eb2674`'s, so on the rebased tree the file is blob `68c8eb2`,
  `28723ae`'s own.
- **One conflict, the same row both times.** The DEC-142 row for
  `mate_the_multicut_hides` in `DEV_MANUAL.md` was rewritten by S243's
  second round, by S022 verdict 1 (its re-mine paragraph) and by `39841ed`
  (that paragraph out, one sentence in saying the re-mine came and went).
  The changes are disjoint spans of the one line, so both are kept: S243's
  sentence, then `39841ed`'s. Checked three ways by the resolver kept in the
  logs: without the revert's span the row is the WIP's, without S243's it is
  `39841ed`'s, and the result is `28723ae`'s row with `39841ed`'s sentence
  where `39841ed` put it.
- **The rename is real.** In the landed binary the case's old title, "the
  first iteration honours stop and the hard timer", selects no case, and the
  new one selects exactly one.

## What landed

- `tests/test_engine.cpp` "a stop inside the first iteration cuts it and the hard timer ends the search within its bound",
  the file at blob `e88103f`, S242's DEC-240 form. The stop half reads its
  precondition from this thread's clock, `tests/test_engine.cpp`
  `prompt_stop_us`, and repeats only an attempt whose precondition failed;
  the hard-timer half sends `go movetime 1` once, on a board of its own,
  `tests/test_engine.cpp` `timer_fen`, and asserts `bestmove` within
  `tests/test_engine.cpp` `bestmove_bound_ms`, 402 ms, never retried.
- `tests/test_search.cpp` "the multicut never ends a node on a mate from its verification"
  carries S243's `CHECK(last_score < MATE_MIN_LOCAL)` before the branch's
  `REQUIRE`s, with its comment, and the case's mutation header names it as a
  killer line. The file is fixture `28723ae`'s byte for byte.
- `DEV_MANUAL.md`, the DEC-142 row for `mate_the_multicut_hides`: S243's
  sentence -- the direct case asserts that the node does not return the mate
  and that the rule does not fire, and the row going stale now shows only as
  E21's kill list in a mutation run shrinking from two cases to one, when
  the row is re-derived by its script -- followed by `39841ed`'s sentence on
  S022's re-mine.
- The source, both in the object store and neither landed itself: fixture
  `28723ae`, `13caf43` plus both steps' final tests, whose three files are
  what the fillers agent's reports measured, and blob `e88103f`. Cut with
  `git diff 6eb2674 28723ae -- tests/test_engine.cpp tests/test_search.cpp DEV_MANUAL.md`
  (103 insertions, 55 deletions) and applied with `git apply --3way` against
  a scratch copy of the worktree's index, so nothing was staged by it. The
  branch is `39841ed` plus one commit.

## Evidence

On the rebased tree, `39841ed` plus this step's commit.

- `git hash-object tests/test_engine.cpp` prints
  `e88103fb76159ae58bbee820e79dcd9bff9a22b0`, and the index's entry is the
  same. `git diff --stat 39841ed` lists `DEV_MANUAL.md`, one row;
  `tests/test_engine.cpp`; `tests/test_search.cpp`; and this file, new.
  Nothing in `src/`, `tools/`, `MANUAL.md` or the four shared documents.
- Both fast suites 41 of 41, Release and `-DCHESSO_TUNE=ON`, and
  `./clang-format.sh --check` clean; `tools/plan_prose_check.py` with
  `--citations`, `--touches` and `--params` exits 0 over this file in its
  final form.
- `tools/mutation_check.py --only E21` over S097's list, on a clean detached
  fixture of the rebased tree at `248b7e4` -- this commit before these
  sections and its message, documents only -- at `--jobs 4`:

```
worktree /home/max/ws/chesso-s246/.ref-builds/mut at 248b7e4 clean
list     /home/max/ws/chesso-s246/.ref-builds/mut/tools/mutants/S097_singular_extension.py   clean
baseline green, 41 tests, bench 3429473 nodes via engine
  E21_multicut_mate_band_gate_dropped killed      fast 1/41  bench same  145.5s
   [test_search] pruning does not hide a forced mate  |  REQUIRE( result.mate_found )
   [test_search] the multicut never ends a node on a mate from its verification  |  CHECK( last_score < MATE_MIN_LOCAL )  (+1 more)
```

  The "+1 more" is `REQUIRE( !record.se_multicut )`, and the CHECK reads
  `CHECK( 48998 <  48000 )`, the verification's mate in one, as S243
  recorded it.
- 200 runs of the renamed case alone, the Release `test_engine` by
  `--test-case` with the new title: 0 failed, every run selecting exactly
  one case, all 200 at `assertions: 7 | 7 passed | 0 failed` -- no attempt
  repeated on the stop half, and the timer half has none -- at a one-minute
  load of 3.87 to 3.94 on twelve hardware threads, 0.03 to 0.04 s a run.
- Superseded by the rebase and kept in the logs: the same proofs on
  `5bf73d3`, S022 verdict 1's tree -- both suites 41 of 41, format and prose
  clean, E21 killed by the same two cases at bench 6049266.
- No golden, ceiling or `src/` line moved, so no `Bench:` line and no second
  tier. `DEV_MANUAL.md` changes by the one row; `MANUAL.md` checked, no
  change; `specs.md` untouched, since it describes the engine and the engine
  did not change.
