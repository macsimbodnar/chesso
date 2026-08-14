# Audit 2026-08-13 plan_review

Scope: `adocs/plan.md` and the 15 pending step files in `adocs/plan_todo/`
(`plan_current/` holds only `.gitkeep`), at commit
`55bad5ea2efc3fd51644715ae81f54a60083b202` (branch `achesso`, tree clean apart
from this report). The engine source is in scope only as the thing those
documents make claims about.

Method: read the plan and every pending step file, then re-measured the claims
they make. What was run: `ctest --test-dir build -L fast` (11/11 passed),
`build/tools/eval_spread` with the command S039's accepts asks for,
`tools/search_bench.py` at depth 7, the engine over UCI (`go depth 7`,
`go wtime 1 btime 300000`), `grep`/`sed` over every `file:line` citation in the
pending step files, and three scratch programs compiled outside the tree
against `build/src/libchesso_engine.a` (they measure the model-versus-engine
truncation gap under S055's proposed arithmetic, the en passant hash split, the
lazy-margin clamp over the tracked corpus, and the move-ordering band
clearance in the position `test_evaluation` uses). The workflow checker's own
source was read read-only at
`~/.claude/plugins/cache/moltke/moltke/0.11.0/bin/moltke.py` to check what
plan.md says it does. Prior findings in `adocs/audit/2026-08-13_adversarial.md`
and `adocs/audit/2026-08-13_plan_review.md` were each re-measured from their own
reproduction; verdicts at the end.

Nothing in the repository was modified except this file. No test was added: every
finding below is a defect in a plan document with a reproduction that runs from a
clean checkout, so none of them needs one to be demonstrable.

Written before any fix. A report edited while fixing stops being evidence of
what was found.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-08-13_plan_review.2-F<nn>`. The example below writes that prefix as
`<report>`: a fenced example carrying this report's real stem cannot be told
apart from a real finding that a fence has swallowed, which is INV-14 (S049).

```
### <report>-F01  high  short title

Status: open

Evidence: file and line, or the command and its output.
Impact: what breaks, for whom, under what conditions.
Suggested resolution: what would close it. Not applied here.
```

Every finding ends in one of two places: a plan step whose `closes:` field
names it, or a decision entry stating why it will not be acted on, which moves
it to `accepted`. A finding moves to `closed` only after the audit is re-run
and no longer reports it. Fixing without re-running leaves it `planned`.

## Findings

### 2026-08-13_plan_review.2-F01  medium  S055's accepts cannot be met: the change it prescribes necessarily fails the guard S038 added hours earlier, and the step file never mentions it

Status: planned

**Evidence.** `adocs/plan_todo/S055_taper_stage_two_once.md:3`:

> accepts:    evaluate() performs one fewer integer division; the tuner-model
> tolerance returns to 2 and **test_eval_model passes over the pinned corpus at
> that tolerance**; SPRT verdict recorded, zero recorded as zero

`test_eval_model` contains two cases over that corpus, not one. The second is
the non-vacuity guard S038 added at `tests/test_eval_model.cpp:381-422`, and it
asserts the *opposite* of a small disagreement:

```
412	      CHECK_MESSAGE(difference > 2.0,
413	                    (fen + ": difference " + std::to_string(difference) +
414	                     ", no longer past the old 2.0 tolerance"));
...
420	    CHECK_MESSAGE(worst > 2.8, ("worst pinned difference " +
421	                                std::to_string(worst) + ", short of 2.875"));
```

The four FENs it runs over (`truncation_positions`, `:215-220`) were pinned
*because* they exceed 2.0 under today's three effective truncations. S055's own
arithmetic says the merge puts the bound at `2 x 23/24 = 1.917`, which is below
2.0 — so no position, pinned or not, can satisfy `difference > 2.0` after the
change. Measured rather than argued (scratch program, engine `evaluate()` and
`tools/eval_model.hpp` at `starting_params`, stage two recomputed with the two
divisions merged exactly as S055 describes):

```
phase 17  stage2 two-div   63  one-div   64  engine   684 ->   685  model   686.8750  diff now 2.8750  diff merged 1.8750  2r1r1k1/4Q1p1/p1P1p1q1/3p3p/1P1PpP2/4P2P/PB4P1/R4RK1 w - - 5 29
phase 10  stage2 two-div  -34  one-div  -35  engine    90 ->    89  model    87.6667  diff now 2.3333  diff merged 1.3333  6k1/6p1/p7/2R5/2P3n1/P1N3P1/1rP4r/2R3K1 w - - 0 29
phase 23  stage2 two-div   20  one-div   21  engine   441 ->   442  model   443.2500  diff now 2.2500  diff merged 1.2500  r1bqkb1r/1pp1pp1p/5n2/p2p4/3P3R/2N2N2/PP1PPPP1/R1BQKB2 b Qkq - 2 8
phase 17  stage2 two-div   83  one-div   84  engine   487 ->   488  model   489.1250  diff now 2.1250  diff merged 1.1250  1k5r/pp3p2/5P2/q1pr4/4R2p/3B3P/P1P2QP1/1R4K1 w - - 6 26

worst now 2.8750   worst after merging 1.8750
test asserts every pinned diff > 2.0 and worst > 2.8
```

So the merge turns five passing assertions into five failures (four
`difference > 2.0`, one `worst > 2.8`) in a test labelled `fast`
(`tests/CMakeLists.txt:21`), which is the label `.moltke.json`'s
`test_command` runs: `cmake --build build -j12 && ctest --test-dir build -L
fast --output-on-failure && ./clang-format.sh --check`. At HEAD that suite is
green (11/11, `ctest --test-dir build -L fast`).

Two further statements in the same test case become false and are also
unmentioned by S055: the tempo precondition's message at `:388-391` ("the bound
is now 4 x 23/24 = 3.833 and the tolerance ... has to be 4" — after the merge a
fitted tempo makes it `3 x 23/24 = 2.875` and the tolerance 3), and the
comment at `:366-373` explaining the thresholds as "the old 2.0".

**Impact.** The step cannot complete as written: the accepts asserts a suite
state the change it prescribes makes unreachable. The likely resolutions are
both bad. Lowering `> 2.0` and `> 2.8` is the only way to keep the case, and
S055 gives no authority for that re-target, so a session under a completion
gate is pushed toward editing a test to go green — the thing AGENTS.md §11
prohibits outright and §6 allows only as a deliberate re-target. Deleting the
case removes the only reason the tolerance is a bound rather than a number
someone widened, which is the defect adversarial-F04 was raised for and S038
fixed eight commits ago. Re-pinning different positions does not help, because
the new bound is below the threshold for every position.

**Suggested resolution.** State the re-target in the step: after the merge the
pinned thresholds become `difference > 1.0` and `worst > 1.8` against the
`2 x 23/24 = 1.917` bound (the measured values above are 1.125 to 1.875), the
tempo message's arithmetic becomes 3 x 23/24 = 2.875 and tolerance 3, and the
accepts should say "test_eval_model's tolerance is 2 and its non-vacuity case is
re-pinned to the new bound" instead of "passes at that tolerance". Adding the
two thresholds to `touches:` reasoning also makes the diff reviewable as a
re-target rather than a relaxation.

### 2026-08-13_plan_review.2-F02  medium  S039's first acceptance criterion cannot be run on this machine: the corpus it names does not exist

Status: planned

**Evidence.** `adocs/plan_todo/S039_lazy_margin_redecide.md:3`:

> accepts:    the margin is chosen from **an eval_spread run over the full
> corpus at current weights** and the run is recorded; ...

The run, exactly as `DEV_MANUAL.md:217` documents it:

```
$ build/tools/eval_spread --data .tuning/selfplay_v1.tsv --limit 50000
could not read .tuning/selfplay_v1.tsv
exit=1

$ ls -d .tuning
ls: cannot access '.tuning/': No such file or directory

$ find /home/max -maxdepth 4 -name 'selfplay*.tsv'
(no output)
```

`.tuning` is gitignored (`.gitignore:11`) and `DEV_MANUAL.md:512-513` says so:
"`.tuning/` is gitignored. Datasets are hundreds of megabytes and are not
evidence in the sense `adocs/data/` is." The work moved machines at DEC-049 and
the corpus did not come with it. Regenerating it is a `datagen` run
(`DEV_MANUAL.md:400-401`, `--games 20000 --nodes 100000 --seed 20260810`),
which `adocs/status.md:37` prices at 95 minutes on the old machine — and the
result is a *different* corpus, because S027 and S028 changed the engine that
generates it, so the step's own quoted figures (0.364 % past 150, worst 279,
and the three named worst-case FENs) are not reproducible at all.

The step's premise itself is intact — measured over the tracked corpus
`adocs/data/S028_raw.tsv` instead (5582 positions, same scratch method, stage
two recomputed from `tools/eval_model.hpp` features at `starting_params`):

```
F05 positions 5582  |mobility+king safety| past the 150 margin 29 (0.520 %)  worst 242
F05 worst fen r1b2Q2/1pkr3p/2nq4/1R6/p3B3/3P2P1/P4P1P/2R3K1 w - - 5 27
```

**Impact.** The step that closes `2026-08-13_adversarial-F05` opens with a
prerequisite nobody can satisfy in this working tree, and it is not written as
a prerequisite — the accepts reads as one cheap run before the SPRT.
plan.md:49-50 prices S039 accordingly: "S039 and S042 cost an SPRT each and buy
little." A session starting S039 discovers the missing input after promoting
the step, and its choices are a two-hour datagen run it never planned for, or
quietly picking a margin from a corpus of a different size and calling it "the
full corpus".

**Suggested resolution.** Name a corpus that exists. `adocs/data/S028_raw.tsv`
is tracked and its `fen` column is the ninth field; either point `eval_spread`
at it (5582 positions is enough to place a margin at the 0.5 % tail) or state
the datagen run and its cost in the step's own text as the first item of work.
Either way, replace the three unreproducible figures with ones re-measured at
HEAD.

### 2026-08-13_plan_review.2-F03  medium  S030's neutrality remedy is wrong for two of the four sites it points at, and would index a 12-row table with 12

Status: planned

**Evidence.** `adocs/plan_todo/S030_move_encoding_16_bit.md`, "Estimate and
risk":

> The neutrality hazard is move ordering, which reads the field being removed:
> `MOVE_PIECE` (`src/data_structures.hpp:86`) indexes `history_moves` and
> `counter_moves` in `score_move` (`src/evaluation.cpp:1092,1097`). **Every such
> site must be re-pointed at `squares[from]`**, and a slip there changes
> ordering -- and so the tree -- with perft still green.

`squares[from]` is correct for the sites that index the move being scored, and
wrong for the two that index `prev_move`, which has already been played by the
time they run:

```
src/evaluation.cpp:1064:  return piece_values_abs[victim] - piece_values_abs[MOVE_PIECE(move)];
src/evaluation.cpp:1094:      move == state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]) {
src/evaluation.cpp:1099:  return state->history_moves[MOVE_PIECE(move)][MOVE_TO(move)];
src/search.cpp:503:            state->history_moves[MOVE_PIECE(moves[i])][MOVE_TO(moves[i])];
src/search.cpp:507:          state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)] =
```

`prev_move` is the move that led to the current node, so its from-square is
empty there and `squares[MOVE_FROM(prev_move)]` is `EMPTY`, which is the
thirteenth enumerator of `piece_t` (`src/data_structures.hpp:152-168`), value
12. Both countermove tables are twelve rows wide:
`src/data_structures.hpp:425` `int history_moves[12][64];` and `:434` `move_t
counter_moves[12][64];`. Following the sentence as written therefore indexes
row 12 of a 12-row array, on both the read at `evaluation.cpp:1094` and the
write at `search.cpp:507` — and the write is at a site the step does not
mention at all, since it says "in `score_move`".

The piece is recoverable from `squares[MOVE_TO(prev_move)]`, but not on a
promotion: that square then holds the promoted piece rather than the pawn, so
the countermove key changes and with it the ordering — silently, with perft
green, which is exactly the failure mode the paragraph exists to warn about.

The site count also grows before S030 runs. S024 (list entry 42) and S023 (43)
sit ahead of S030 (51), and S024's table is indexed by *the previous move's*
piece and target crossed with the current move's — the case this remedy gets
wrong — while S023 adds a `[piece][to][victim]` table.

**Impact.** The one hazard note in the step prescribes a substitution that is
out of bounds at two of the sites it names, so a session that follows it lands
in undefined behaviour or, if the index happens to be in range after a later
refactor, in a changed search tree. The step's own INV-6 clause would very
likely catch the tree change (that is why S046 put it there), which bounds the
damage to a wasted cycle rather than a silent regression — but the note is the
part of the file meant to prevent the cycle.

**Suggested resolution.** Split the sentence: sites keyed on the move being
scored take `squares[MOVE_FROM(move)]`; sites keyed on `prev_move` take the
piece from the search stack (or `squares[MOVE_TO(prev_move)]` with the
promotion case stated), and the write at `src/search.cpp:507` is named
alongside the read. Add a line saying the set of sites is whatever
`grep -n MOVE_PIECE src/` returns at the time the step starts, since S023 and
S024 add to it.

### 2026-08-13_plan_review.2-F04  low  S025's gate requires a timing outcome its own evidence says will not happen

Status: planned

**Evidence.** `adocs/plan_todo/S025_bad_capture_ordering_retry.md:3`:

> accepts:    fixed-depth time is not worse than the two-stage build on all
> three search_bench positions; only then an SPRT

The same file records the measurement that has already been taken, three ways,
at depth 13 over those three positions: exact `see()` in `score_move` **+17.6
%**, `see_ge` **+13 %**, lazily in the picker **+3 %** — all worse, which is
why DEC-022 set the step aside. The accepts admits only one outcome and has no
branch for the recorded one, so if the rebuilt version measures slower again
the step cannot complete: there is no "measured worse, recorded, not retained"
path, which is how S005, S006 and S015 completed.

This is the shape `2026-08-13_plan_review-F08` identified and S051 fixed — but
S051's `touches:` names only `adocs/plan_todo/S020_single_check_computation.md`
and `adocs/plan_todo/S030_move_encoding_16_bit.md`, and its accepts is scoped
to "both steps' accepts", so this third instance was never in its reach.

**Impact.** Low while the step sits at list entry 44 behind five others. When
it starts, the honest outcome is unbookable, and the pressure lands where
CLAUDE.md foundation 2 says it must not — on finding a number rather than
measuring one.

**Suggested resolution.** Make it outcome-neutral like S020's and S030's:
fixed-depth time measured against the two-stage build on all three positions
with its noise floor recorded, the SPRT run only if the timing is not worse,
and a timing that is worse recorded as the verdict with the step completing on
it.

### 2026-08-13_plan_review.2-F05  low  S026's completion gate is weaker than its own hazard paragraph, unlike the adjacent S033

Status: planned

**Evidence.** `adocs/plan_todo/S026_futility_and_razoring.md:3`:

> accepts:    an SPRT per technique, measured separately; **mate and tactical
> tests in the fast suite still pass**

Its body, two paragraphs down:

> Any pruning added here gets the same treatment: **a position with a forced
> mate inside the pruned depth, in the fast suite, before the feature is called
> done.**

The accepts asks that the existing tests keep passing; the body asks for a new
test aimed at the depth this step prunes. Nothing states that the existing
cases — `tests/test_search.cpp:82` "mate in one" and `:124` "mate in two is
found at the right distance" — sit inside the depth window futility and
razoring prune, and no test name asserts it. The adjacent step with the
identical hazard spells the requirement into its gate:
`adocs/plan_todo/S033_reverse_futility_pruning.md:3` — "a position with a
forced mate inside the pruned depth is in the fast suite and passes before the
feature is called done".

**Impact.** The completion gate is the accepts, so S026 can complete with only
pre-existing mate coverage. CLAUDE.md names pruning that hides a mate as the
recurring bug in this engine (NMP reduced to depth 0, LMR reduced the mating
move at the root), and both times a mate test caught it rather than a
benchmark. Two adjacent steps with the same hazard get different gates.

**Suggested resolution.** Copy S033's clause into S026's accepts, once per
technique, since forward futility and razoring prune at different places.

### 2026-08-13_plan_review.2-F06  low  S023's second acceptance criterion has no instrument, and the test nearest to it has 99200 points of slack

Status: planned

**Evidence.** `adocs/plan_todo/S023_capture_history.md:3`:

> accepts:    an SPRT returns a verdict; **the ordering bands stay disjoint**,
> which the move-ordering hazard in CLAUDE.md makes easy to break silently

The hazard is the 100-point clearance at the bottom of the capture band: a king
capturing a pawn scores `ORDER_CAPTURE + MVV_PAWN - MVV_KING = 900100` against
`ORDER_KILLER_0 = 900000` (`src/evaluation.cpp:22-36`, and the source comment
says the symptom of losing it "is a strength regression, not a wrong node
count"). The only test over the bands is `test_evaluation` "bands are strictly
ordered" (`tests/test_evaluation.cpp:643-704`), and it compares `a_capture` —
whichever capture the generator returns first — against the killers. In the
position it uses:

```
legal moves 48  captures 8  king captures 0
first capture score 1000000 (the one the test compares)
worst capture score 999200   killer band 900000
clearance of the worst capture over the killer band: 99200
```

So the assertion `s_capture > s_killer0` holds with 99200 points to spare, and
the pair the hazard is about is not reachable in that position at all. Nothing
else in `tests/` mentions the band constants or a king capture ordering case
(`grep -rn "900100\|MVV_KING" tests/*.cpp` returns nothing relevant).

**Impact.** The criterion cannot be discharged by anything `ctest` runs, so it
will be discharged by reading the diff — in the one place the project's own
documentation says the failure is invisible to node counts and shows up only as
lost Elo. S023 adds a term to the capture score, which is precisely the change
that can consume the clearance.

**Suggested resolution.** Have the accepts name the test: a case that scores a
king capture of a pawn against a killer at the same ply and asserts the capture
wins, plus a bound on the new capture-history term that keeps the sum inside
its band. Non-vacuity comes free — the case fails if the position stops
offering a king capture.

### 2026-08-13_plan_review.2-F07  low  plan.md's prose says S037 is pending and S054 is next; both are in plan_done at this commit

Status: planned

**Evidence.** `adocs/plan.md:44-46`:

> S037 is the node count that `search_bench.py` reads, which is how INV-6 is
> discharged — **the last of the three instruments still pending**.

`adocs/plan.md:62`:

> **S054 is first of everything pending**, and it is not an audit finding.

Both completed before this commit:

```
$ ls adocs/plan_done/ | grep -E 'S037|S054'
S037_cumulative_info_nodes.md
S054_clang_format_untracked.md

$ git log --format='%h %ad %s' --date=format:'%H:%M' -1 8b89411; git log --format='%h %ad %s' --date=format:'%H:%M' -1 031ff70
8b89411 19:13 Complete S037: info nodes is the whole search's count
031ff70 21:31 Complete S054: the format check sees untracked source files
```

S037's fix is live: `go depth 7` now prints monotonically cumulative counts
(`nodes 120, 351, 4440, 12387, 23838, 42778, 67013`) and
`tools/search_bench.py ./build/src/chesso 7` reports `midgame 122617 nodes`,
the whole-search figure. The same paragraph at `:53` ("S044 to S052 ... ahead
of everything still pending") is stale in the same way — all nine are done.

**Impact.** plan.md is the second document in the reading order and the prose
is what a cold session reads before the list. It currently says the instrument
that discharges INV-6 does not exist yet, which is an argument against making
any neutrality claim, and names a completed step as the next thing to do. The
list itself is right, and `plan.md:80-82` says prose ids are not checked — but
the sentence is not an id in passing, it is a status claim about tooling.
`2026-08-13_plan_review-F05` reported the same class ("the stale harness prose
tells a cold session the project's only verdict instrument is still broken");
S048 cleared it and four completions later it is back.

**Suggested resolution.** At step completion, re-read the prose that names the
completed id. Here: S037 in the past tense with S035 and S036, and the S054
paragraph rewritten as history or dropped.

### 2026-08-13_plan_review.2-F08  info  three pending step files cite line numbers that moved two lines at S037

Status: planned

**Evidence.** Every `file:line` citation in the pending step files was resolved
against HEAD. Three miss, all by the same two lines, all in `src/evaluation.cpp`:

| step file | citation | what is actually there | what was meant |
|---|---|---|---|
| S024 | `src/evaluation.cpp:1091-1094` "read as the fixed `ORDER_COUNTER` band" | `:1091` is `killer_moves[1]` | the counter-move read is `:1093-1096` |
| S030 | `src/evaluation.cpp:1092,1097` "indexes `history_moves` and `counter_moves`" | `:1092` is blank, `:1097` is blank | `:1094` and `:1099` |
| S039 | `src/evaluation.cpp:731-734` "King safety has not shipped at zero weight since S027" | a comment about collinearity | the weights are `:733-736` |

Cause: `8b89411` ("Complete S037") changed `src/evaluation.cpp` by +3/-1 at
19:13, after S045, S046 and S049 rewrote those three step files at 18:22-18:33.
Everything else resolves exactly, including S031's four xor sites
(`src/bitboard.cpp:878-880`, `1059-1061`, `1460-1462`, `1536-1537`), S042's
`src/bitboard.cpp:824-833`, S055's six `src/evaluation.cpp` lines and both
`tools/eval_model.hpp` ranges, and S030's `src/data_structures.hpp:86`.

**Impact.** Cosmetic for a reader who greps, but S030's whole neutrality
argument is a list of pointers (see F03), and a pointer that lands on a blank
line is one more reason to reconstruct the site set by hand rather than trust
the file.

**Suggested resolution.** Re-point the three, or cite the symbol instead of the
line where the file is likely to move again.

### 2026-08-13_plan_review.2-F09  info  plan.md describes the checker's pruning as oldest-completion-first; it is last-five-by-position, and the ledger follows

Status: planned

**Evidence.** `adocs/plan.md:83-86`:

> the workflow checker enforces the correspondence, adds an entry when a step is
> created, and **prunes the oldest completed entry** — taking its testing.md
> rows with it — as newer completions land.

The checker prunes by position in the file, not by when the step completed
(`~/.claude/plugins/cache/moltke/moltke/0.11.0/bin/moltke.py:1684-1705`):

```
    entry_lines = [i for i, line in enumerate(lines)
                   if (m := PLAN_ENTRY_RE.match(line)) and m.group(1) in done_ids]
    drop = set(entry_lines[:-PLAN_DONE_KEPT]) if len(entry_lines) > PLAN_DONE_KEPT else set()
```

The current file shows the difference. The five completed entries still listed
are S040 (completed 19:55), S041 (21:05), S054 (21:31), S038 (22:38) and
**S053 (18:42)** — while S037 (19:13) and S043 (19:29), both completed *after*
S053, are gone. S053 survives because its entry is the last line of the list
(`plan.md:108`, appended at creation), not because it is recent. The ledger
follows the same rule: `grep -c S043 adocs/testing.md` is 0 and
`grep -c S053 adocs/testing.md` is 1, and `git log -S"S043" -- adocs/testing.md`
shows the row leaving in `b904d1b`, the S038 completion.

**Impact.** Informational. Anyone reasoning about what the list contains, or
about which acceptance rows are still visible, gets the wrong model: retention
is a window over list positions, so a step appended late outlives newer
completions and keeps its ledger rows while theirs are pruned.
`adocs/testing.md:5-7` states the stronger and plainly false version ("it keeps
the newest five completed entries listed"), which is outside this audit's scope
but is the same misstatement.

**Suggested resolution.** Say what the checker does: the last five completed
entries *in list order* are kept, and a row leaves when every step id it names
was pruned in that pass.

## What was checked and found sound

Stated because a negative result is a result.

- **List and file correspondence.** 15 pending step files, 15 pending list
  entries, 20 entries total, every entry naming an existing file and every
  pending file appearing once. Next-step derivation (first list entry not in
  `plan_done/`) yields S033, agreeing with `status.md`.
- **INV-6 gates.** Every pending step that alters play carries an SPRT clause
  (S021, S022 two, S023, S024 two, S025, S026 per technique, S029, S033, S042,
  S055); every step claiming neutrality carries the `search_bench` identity
  clause (S020, S030 with the SPRT branch, S031, S032). The S046 fix holds.
- **S031's bit-identity claim is arithmetically right.** With the single key
  defined as `side_randoms[WHITE] ^ side_randoms[BLACK]`, one xor takes
  `side[W]` to `side[B]` and back, so `compute_full_hash`'s
  `side_randoms[active_color]` (`src/bitboard.cpp:1537`) stays consistent and
  every key is unchanged. The three two-xor sites are where the file says.
- **Premises that still hold at HEAD.** S020: `is_check` is computed per node
  in `quiescence` (`src/search.cpp:130`) and `negamax` (`:289`) and again for
  the child by the parent (`:414`). S021: `iterative_deepening_search` is at
  `src/chesso.cpp:544` and `search()` takes no window (`src/search.hpp:5`).
  S023's band arithmetic: `MVV_PAWN 100`, `MVV_KING 100000`,
  `ORDER_CAPTURE 1000000`, `ORDER_KILLER_0 900000`. S025's "queen takes pawn
  sits at 999200" — measured, 999200. S032: `x86_64`, `bmi2` in
  `/proc/cpuinfo`, 12 threads. S039's quoted comment is verbatim at
  `src/evaluation.hpp:274-282` and the margin is still 150. S042 reproduces
  exactly, hashes included. S055's model claim: `tools/eval_model.hpp:973-974`
  tapers the two terms once.
- **Nothing pending is already implemented.** `grep -rniE
  'aspiration|delta pruning|razor|futility|capture_history|continuation'
  src/*.cpp src/*.hpp` returns nothing.
- **Audit coverage.** All 18 findings from the two prior reports map one-to-one
  onto `closes:` fields (S035-S043, S044-S052). None is dangling.
- **Suite at HEAD.** `ctest --test-dir build -L fast`: 11/11 passed, 16.88 s.

## Verdicts on prior findings

Each re-measured from its own reproduction at
`55bad5ea2efc3fd51644715ae81f54a60083b202`. Where the original reproduction used
`.tuning/selfplay_v1.tsv`, that corpus is not on this machine (F02 above) and
the substitute measurement is named.

### 2026-08-13_plan_review (nine findings, all recorded `planned`)

This run is the plan_review re-run, so these are the verdicts that can move a
plan_review finding to `closed`.

- **F01 (plan.md assigns the fit to the owner under DEC-015)** — does not
  reproduce. `plan.md:11-14` now names S029's network training as the one
  owner-run step, "DEC-015 as amended by DEC-041", and marks S028's fit as
  agent-run; `specs.md:170-175` carries the same with a dated note. **closed.**
- **F02 (S024 conflates the countermove heuristic with continuation history)** —
  does not reproduce. `S024_continuation_history.md` now states chesso has the
  heuristic and no continuation history at any depth, scopes one-ply first and
  two-ply second with an SPRT each, and cites the finding. **closed.**
- **F03 (S030/S031/S032 stop at perft)** — does not reproduce. All three
  accepts name `tools/search_bench.py` identity; S030 carries the SPRT branch;
  S031 states the `side_randoms[W]^side_randoms[B]` construction and drops the
  bundling licence. **closed.**
- **F04 (S032 blocked on hardware)** — does not reproduce. S032 reads
  "Measurable here since DEC-049" and its accepts says "measured here"; S029's
  x86 caveat is gone and now says integer SIMD, NEON included; S020's 12 %
  carries "on the Apple machine under Apple clang ... has not been re-taken
  here"; `specs.md:189-194` is rewritten for the DEC-049 machine. **closed.**
  One residue outside this audit's scope: `adocs/status.md:33-39` still says
  "three usable cores", "`opendirectoryd` at half a core" and "An x86-64 Linux
  box fixes this and is needed for S032 and S029 regardless".
- **F05 (plan rule cites INV-3; 18 entries missing)** — does not reproduce.
  `plan.md:80-86` states the correspondence without an invariant label, and it
  holds (checked above). The two stale prose claims it also named are fixed;
  new ones have appeared, recorded as F07 above rather than as a survival of
  this finding. **closed.**
- **F06 (S039 cites DEC-034)** — does not reproduce. `S039...md:6` reads
  `decisions:  DEC-039`. **closed.**
- **F07 (S022 asks one run for two changes)** — does not reproduce. S022's
  accepts asks two SPRT verdicts, one per change, each against the commit
  before it, and the 12.1 % figure carries its machine condition. **closed.**
- **F08 (S020/S030 require a positive gain)** — does not reproduce in the two
  files it named: both accepts now record a measurement with its noise floor
  and complete on a zero, and S020 names hyperfine. A third instance of the
  same shape sits in S025 and is recorded as F04 above, not as a survival of
  this finding. **closed.**
- **F09 (S021 points at the wrong file)** — does not reproduce. `touches:` reads
  `src/chesso.cpp iterative_deepening_search ... src/search.cpp only to plumb
  the window through search()'s signature`, which matches
  `src/chesso.cpp:544`. **closed.**

### 2026-08-13_adversarial (nine findings, all recorded `planned`)

A different audit type, so these verdicts record what reproduces today; only an
adversarial re-run changes their status.

- **F01 (`fastchess.sh` aborts and reports success)** — fix verified. No
  `perf_cores` remains (`:45` sets `all_cores`, `:119` prints
  `concurrency $concurrency of $all_cores cores`), the trap preserves the
  entering status (`:90` `trap 'status=$?; rm -f "$snapshot"; exit $status'
  EXIT`), and `test_fastchess_script` passes in the fast suite. Correctly
  `planned` pending an adversarial re-run.
- **F02 (`go wtime 1` never answers)** — fix verified. The floor is
  `std::max(budget, std::max(1, std::min(50, remaining_ms / 2)))`
  (`src/chesso.cpp:409`). Live: `go wtime 1 btime 300000` answered
  `bestmove d2d4` before a `readyok` sent 3 s later. `planned`.
- **F03 (`info nodes` per iteration)** — fix verified. `go depth 7` prints
  120, 351, 4440, 12387, 23838, 42778, 67013 — monotone and cumulative — and
  `search_bench.py` reports `midgame 122617 nodes`, exactly the sum the finding
  computed by hand. `planned`.
- **F04 (2 cp tolerance exceeded on 1.7 % of the corpus)** — fix verified by
  substitute measurement. `tests/test_eval_model.cpp:362` allows 3.0, the
  comment at `:321-343` names all four divisions and why three round, and four
  corpus positions are pinned in the file so the bound is exercised without
  `.tuning/`. Measured on those four: 2.8750, 2.3333, 2.2500, 2.1250 — the
  worst is the predicted 2.875 and inside the tolerance. `planned`. See F01
  above: the fix S055 proposes breaks the guard this one added.
- **F05 (`LAZY_EVAL_MARGIN` precondition false)** — still reproduces.
  `src/evaluation.hpp:279-281` still says the figure holds "only because king
  safety ships at zero weight"; `src/evaluation.cpp:733-736` holds
  `{23, 13, 4, 21, -16, 3, 2, -33, -11}` and `{-8, -8, -2, -34, 8, -3, -8,
  -10, 10}`; the margin is still 150. Re-measured over the tracked corpus
  `adocs/data/S028_raw.tsv` instead of `.tuning/`: the combined correction
  passes 150 on 29 of 5582 positions (0.520 %) and reaches 242, so up to 92 cp
  of a computed term is discarded. S039 pending. `planned`.
- **F06 (DEV_MANUAL tuner section stale)** — fix verified. `DEV_MANUAL.md:421`
  says 827, `:424` includes "tempo 2", `:460` lists `tempo` among the `--only`
  groups, `:445` lists `tempo_mg`/`tempo_eg` in the paste target, `:470` records
  four occurrences of the swallowing bug. `planned`.
- **F07 (`free_mask` untested)** — fix verified. `tests/test_tuner_groups.cpp`
  exists, is registered in the fast suite, and passes;
  `tools/tuner_groups.hpp` is the extracted header it links.
  `grep -rln "free_mask\|GROUP_LIST\|tuner_groups" tests/` now hits.
  `planned`.
- **F08 (en passant square set unconditionally)** — still reproduces, with the
  finding's own numbers:

  ```
  F08 order 1  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2
       hash 11800988595472028807
  F08 order 2  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq d3 0 2
       hash 2736264005390963272
  F08 placement equal: yes   hashes equal: no
  ```

  `src/bitboard.cpp:827-828` still sets the square from `move.double_push`
  alone. S042 pending. `planned`.
- **F09 (dead `CMAKE_TOOLCHAIN_FILE`)** — fix verified. No `TOOLCHAIN` line in
  `CMakeLists.txt`. `planned`.

Summary of verdicts: of the nine plan_review findings, nine no longer
reproduce. Of the nine adversarial findings, seven are fixed and verified and
two still reproduce (F05, F08), each with its step pending at list entries 47
and 48.
