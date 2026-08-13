# Audit 2026-08-13 plan_review

Scope: `adocs/plan.md` and the 19 pending step files in `adocs/plan_todo/`, at
commit `ea61e5fd58f7142ecb340122242b58374743c1bc` (branch `achesso`, tree clean).
Assessed adversarially for internal consistency, ordering, testable acceptance
criteria, agreement with the chess-programming literature the steps draw on, and
agreement with the code as it stands. Prior findings in
`adocs/audit/2026-08-13_adversarial.md` re-checked from their own reproductions;
verdicts at the end.

Method: read the plan and step files, then verified every claim they make about
the code against the code — `grep`/`sed` on the cited files, a full rebuild
(`cmake --build build -j12`), the fast suite (`ctest --test-dir build -L fast`,
9/9 passed), the engine driven over UCI (`go wtime 1`, `go depth 7`),
`tools/search_bench.py` at depth 7, and `/proc/cpuinfo` for the hardware claims.
Cross-checked plan.md and step files against `specs.md` invariants and the
`decisions.md` entries they cite. Nothing outside this report was written;
nothing was fixed.

Written before any fix. A report edited while fixing stops being evidence of
what was found.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-08-13_plan_review-F<nn>`. The example below writes that prefix as
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

### 2026-08-13_plan_review-F01  medium  plan.md still assigns the S028-style fit to the owner under DEC-015, which DEC-041 superseded

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `adocs/plan.md:11-13`:

> S028 and S029 are the two steps where the agent stops short of the run
> itself: it delivers the tuner, the data and the training program, and the
> owner executes them (DEC-015).

`adocs/decisions.md:1430` (DEC-041, 2026-08-11): "The owner grants a standing
delegation: the agent runs tests, measurements and evaluation tuning itself,
without asking. [...] This supersedes DEC-015 for evaluation tuning and for
every kind of measurement." Only the S029 network training remains the owner's.
The history agrees: the S028 fit was executed by the agent (DEC-034,
`decisions.md:937`, then DEC-041), and `adocs/plan_done/S028_texel_tuning.md`
exists — S028 is not a step where anything "stops short" anymore, it is done.

The same stale claim sits in `adocs/specs.md:165-167` ("**The agent does not
run training or table tuning.** [...] The owner executes the run. DEC-015"),
and specs outrank plan, so the wrong copy is also the authoritative one.

**Impact.** A cold session reading plan.md (or specs.md, which wins on
disagreement) concludes that every future fit — S027-style term fits, the S039
margin re-decision, S029's data generation — must be handed to the owner. That
is the exact handoff cost DEC-041 was taken to remove, reintroduced by
documents that were not updated when the decision landed. Conversely, an agent
that knows DEC-041 must now treat plan.md and specs.md as unreliable on
workflow boundaries.

**Suggested resolution.** Rewrite `plan.md:11-13` to name S029's network
training as the one owner-run step, citing DEC-015 as amended by DEC-041, and
mark S028's mention as historical. Add the dated inline note to specs.md's
non-goals in the same commit.

### 2026-08-13_plan_review-F02  medium  S024 conflates the countermove heuristic with one-ply continuation history

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `adocs/plan_todo/S024_continuation_history.md`:

> One-ply continuation history is the countermove table already present.
> Two-ply is the usual next step.

What is present is the countermove *heuristic*, not countermove *history*.
`src/data_structures.hpp:434` — `move_t counter_moves[12][64];` — stores one
move per (previous piece, previous target), written on a beta cutoff at
`src/search.cpp:507` and consumed at `src/evaluation.cpp:1091-1094` as a fixed
ordering band:

```
if (prev_move != 0 &&
    move == state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]) {
  return ORDER_COUNTER;
}
```

In the literature these are two different devices. The countermove heuristic
(Uiterwijk, 1992) remembers a single refutation move and gives it a flat bonus.
Continuation history — countermove history at one ply, follow-up history at two
— is a *score table* indexed by (previous move's piece, target) × (current
move's piece, target), accumulating graded bonuses for every quiet move, and it
is what the step's own goal line describes ("history indexed by the move played
n plies ago and the current move"). Reference engines carry both at once
precisely because they are not the same thing, and the one-ply table is
consistently reported as the stronger half of the pair.

**Impact.** The note steers the implementation to skip one-ply continuation
history on the belief it already exists, and build the two-ply table alone.
That measures the weaker variant of the technique; a zero SPRT verdict on it
would be recorded as "continuation history is worth zero here" when the
strongest form was never built. That is the DEC-019 failure mode —
concluding from a figure that does not describe what was actually tested —
manufactured locally.

**Suggested resolution.** Correct the note: chesso has the countermove
heuristic and no continuation history of any depth. Scope S024 to the one-ply
table first, the two-ply table second, one SPRT each.

### 2026-08-13_plan_review-F03  medium  S030, S031 and S032 acceptance criteria stop at perft and never discharge INV-6

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `specs.md:44-48` (INV-6): a change claimed behaviour-neutral
proves it with identical node counts and best moves from
`tools/search_bench.py`; a change that alters play needs an SPRT verdict. The
three step files' `accepts:` lines:

- `S030_move_encoding_16_bit.md`: "perft node counts unchanged; measured gain
  larger than the benchmark's own resolution; the opening book and UCI layer
  still round-trip every move"
- `S031_single_side_to_move_key.md`: "perft node counts unchanged;
  compute_full_hash() and swap_side() agree"
- `S032_pext_sliding_attacks.md`: "perft node counts unchanged on both paths;
  measured on x86-64 [...]"

None of the three names the search_bench neutrality proof, and none carries the
SPRT branch for the case where neutrality fails. Perft exercises generation and
make/unmake only; it says nothing about the search tree.

Neutrality is not free in at least two of them. S030 removes the moving piece
from the encoding, and move ordering reads it: `MOVE_PIECE` (macro at
`src/data_structures.hpp:86`) indexes `history_moves` and `counter_moves` in
`score_move` (`src/evaluation.cpp:1092,1097`) — every such site must be
re-pointed at `squares[from]`, and any slip changes ordering, which changes
play with perft green. S031 changes the zobrist key values themselves unless
the single key is defined as `side_randoms[WHITE] ^ side_randoms[BLACK]`
(current two-xor sites: `src/bitboard.cpp:878-880`, `1059-1061`, `1460-1462`;
one-xor reference at `1536-1537`); changed key values change transposition
hits, which changes the tree. The step does not say which construction is
meant, and adds "it should be carried in alongside other hash work rather than
measured on its own" — bundling, which the house rules forbid unless the
change is proven bit-identical first, which nothing in the accepts asserts.

Contrast S042, which spells the same fork out correctly ("behaviour-neutral by
INV-6's first test and not by its second, so it takes an SPRT").

**Impact.** A step in this block can be completed to the letter of its accepts
while altering play, with no measurement anywhere — the one discipline the
project states it never breaks. A silently altered tree then contaminates every
comparison taken after it, which is the same class of damage as DEC-020.

**Suggested resolution.** Add to each of the three accepts: identical
`search_bench` node counts and best moves against the preceding commit, or,
where that fails by design, an SPRT. For S031, state the
`side_randoms[W]^side_randoms[B]` construction explicitly if hash-identity is
the intent, and drop the "carried in alongside other hash work" line or
condition it on proven bit-identity.

### 2026-08-13_plan_review-F04  medium  S032's "blocked on hardware" premise is false on the machine the tree now records

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `adocs/plan_todo/S032_pext_sliding_attacks.md`: "**Blocked on
hardware.** **Zero on this machine.** ARM has no PEXT. [...] An x86-64 Linux
box is needed for this"; its accepts: "measured on x86-64, since it cannot be
measured here". The machine this tree builds on, since DEC-049
(`decisions.md:1848`, 2026-08-13, "Work moved to a Linux machine -- Pop!_OS
24.04, i7-8700K"):

```
$ uname -m; nproc; grep -o 'bmi2' /proc/cpuinfo | head -1
x86_64
12
bmi2
```

The step is not blocked, and "cannot be measured here" is false. Related stale
premises in the same block:

- `S029_nnue.md`: "**This is where x86 stops being optional**: AVX2 and VNNI
  are where the performance is and Apple Silicon has no equivalent." The
  machine constraint is moot now, and the claim was an overstatement against
  the literature anyway — published NNUE engines run integer inference on
  Apple Silicon via NEON; AVX2 is wider, not without equivalent.
- `S020_single_check_computation.md`: "`is_check` is about 12 % of the
  profile" — a figure profiled on the Apple machine under Apple clang. DEC-049's
  consequence is explicit: "Every figure recorded before this entry was
  measured on Apple silicon under Apple clang and is not comparable to anything
  measured here. [...] Those figures keep their conditions attached wherever
  they are quoted." S020 quotes it with no condition.
- `specs.md:181-185` (open items) still describes the old machine: "three
  usable cores", "this machine has a habit of running `opendirectoryd`", "An
  x86-64 Linux box resolves this and is needed for S032 and S029 regardless."

**Impact.** A session that trusts S032's step file defers a step that stopped
being blocked on 2026-08-13, and reads "needed for S029" as an unmet
prerequisite when it is already met. S020's premise number may not survive
re-profiling on this machine, and the step's cost/benefit was ranked on it.

**Suggested resolution.** One edit per step file re-stating the premise against
DEC-049 (S032: measurable here, keep magics as the fallback path; S029: drop
the hardware caveat; S020: mark the 12 % as Apple-machine and re-profile before
starting). Refresh the specs.md open item in the same commit.

### 2026-08-13_plan_review-F05  low  plan.md's own consistency rule cites the wrong invariant and does not hold of the file

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `adocs/plan.md:60-61`: "Every step file must appear as a list
entry, and every list entry must have a step file — both are INV-3." INV-3 in
`specs.md:35-37` is "`generate_captures` and `generate_quiets` partition
`generate_moves` exactly", with a matching row at `testing.md:22`. The label
points at a move-generation invariant, not a plan property; no plan-structure
invariant exists in specs.md.

The rule also fails against the file itself:

```
$ comm -13 <(grep -oE '^[0-9]+\. S[0-9]+' adocs/plan.md | grep -oE 'S[0-9]+' | sort) \
           <(ls adocs/plan_done adocs/plan_todo adocs/plan_current | grep -oE '^S[0-9]+' | sort)
S001 S002 ... S018   (18 step files with no list entry)
```

24 list entries remain, of which five are completed steps (S028, S034, S027,
S035, S036) kept in the list while the other 18 completed steps were removed —
whichever convention is intended, the file follows both at once. Two smaller
staleness points in the same file: `plan.md:39-41` still says the SPRT harness
"has not run since `44877c4`, so no verdict on anything below it is obtainable
until it is fixed" (S035 and S036 are in `plan_done/`, the suite's
`test_fastchess_script` passes, and the harness ran a 30-game live match per
S035's stamp); and `plan.md:35` prices "S030 to S032" at "1-3 % each" while
S031's own file says "**Under 1 %**".

**Impact.** The one structural rule the plan states about itself cannot be
checked (wrong invariant id) and is currently false (18 missing entries), so a
reader cannot tell intended state from drift. The stale harness prose tells a
cold session the project's only verdict instrument is still broken.

**Suggested resolution.** Pick one convention (all step files listed with done
ones marked, or pending-only) and apply it; restate the correspondence rule
without the INV-3 label, or give the property a real number in specs.md;
refresh the two stale prose claims.

### 2026-08-13_plan_review-F06  low  S039 cites DEC-034 — the one-night fit delegation — as the decision it implements

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `adocs/plan_todo/S039_lazy_margin_redecide.md` header:
`decisions: DEC-034`. DEC-034 (`decisions.md:937`) is "The owner delegated the
S028 fit to the agent for one run" — workflow delegation, nothing to do with
the lazy-evaluation margin. The decision that actually governs the lazy
shortcut is DEC-039 (`decisions.md:1291`, "Skip the expensive evaluation work
rather than remember it", tagged `s034`); the step's own excludes line names
"S034", suggesting a DEC-034/S034 transposition.

**Impact.** Traceability only, but that is what the field exists for: grep from
DEC-034 lands on a margin step it does not constrain, and the decision that
does constrain it is cited nowhere.

**Suggested resolution.** Point the field at DEC-039 (or clear it and let the
step's eventual margin decision take a new entry).

### 2026-08-13_plan_review-F07  low  S022's acceptance line asks one run to measure two changes, against its own body and the house rule

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `adocs/plan_todo/S022_delta_pruning_quiescence.md`, accepts: "an
SPRT returns a verdict; **the same run** re-measures the S015 quiescence SEE
pruning". Body of the same file: "but measure them **one at a time**, or
neither number means anything." AGENTS.md §0: "One change at a time — two at
once and neither number means anything." A completion gate satisfied by "the
same run" is satisfied by exactly the measurement the body forbids. Secondary:
the premise figure "see() cost 12.1 % more than it does now" predates DEC-049
and carries no machine condition (see F04).

**Impact.** Whichever way the step is executed it deviates from its own text —
either the accepts (two runs where it says one) or the body (one run measuring
two changes, producing two meaningless numbers at a cost S027 prices in hours).

**Suggested resolution.** Reword the accepts to two verdicts, one per change,
in either order, each against the commit before it.

### 2026-08-13_plan_review-F08  low  S020 and S030 make a positive measured gain an acceptance criterion

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `S020_single_check_computation.md` accepts: "measured gain larger
than the benchmark's own reported resolution"; `S030_move_encoding_16_bit.md`
accepts: "measured gain larger than the benchmark's own resolution". The
project's record is that predicted gains routinely measure zero: DEC-019 counts
three (0, 0, and slower), plan.md:18-19 restates it, and the recorded practice
(S005, S006, S015) is that a zero verdict is recorded as zero and the
keep-or-revert call is made with the reason stated. A gate that *requires* the
gain cannot be met by the honest outcome, and rewards finding a number over
measuring one — the incentive the noise-floor rules exist to remove. For S020
there is also no benchmark that fits the words: `bench_movegen` reports its own
resolution but does not run the search; `search_bench.py` times the search but
reports no resolution.

**Impact.** If the gain is real, no harm. If it is zero — the modal outcome in
this project's history — the step cannot complete as written, and the pressure
lands exactly where CLAUDE.md's measurement rules say it must not.

**Suggested resolution.** Make both gates outcome-neutral: a measurement with
its noise floor recorded, the keep-or-revert decision made from it, zero
recorded as zero. Name the instrument for S020 (e.g. `hyperfine` over
interleaved fixed-depth runs, per CLAUDE.md rule 5).

### 2026-08-13_plan_review-F09  low  S021 points at the wrong file for iterative deepening

Status: closed — 2026-08-13_plan_review.2 re-ran this report and does not report it

**Evidence.** `S021_aspiration_windows.md`: `touches: src/search.cpp iterative
deepening`. The iterative deepening loop is not in `src/search.cpp`:

```
$ grep -n "iterative" src/*.cpp src/*.hpp
src/uci.hpp:103:uci_search_result_t iterative_deepening_search(
src/chesso.cpp:544:uci_search_result_t iterative_deepening_search(const uci_search_options_t& conf)
```

`src/search.cpp` exposes `search(int depth, game_t*, search_state_t*)`
(`search.hpp:5`) with no window parameters, so the aspiration loop (window
around the previous score, re-search on fail high/low) lives in
`chesso.cpp:544`'s loop, with `search.cpp` touched only to plumb the window
through. S037's step file cites the correct location for the same loop, so the
two pending step files disagree with each other.

**Impact.** Minor misdirection for whoever starts the step; also an internal
inconsistency between step files about where the same code lives.

**Suggested resolution.** Correct the touches line to
`src/chesso.cpp iterative_deepening_search, src/search.cpp search() signature`.

## What was checked and found sound

Stated because a negative result is a result:

- **Ordering and dependencies.** The measuring instruments were correctly cut
  ahead (S035, S036 done; S037 is next and its "before any further neutrality
  claim" note is honoured by the order). S025 sits after both S023 and S024 as
  its excludes demands; S042 excludes S031's key and precedes it; the
  seven-ahead/two-behind split of the audit steps matches plan.md:37-47's
  description exactly. Next-step derivation (first list entry not in
  `plan_done/`) yields S037, agreeing with status.md.
- **Audit coverage.** All nine 2026-08-13_adversarial findings map one-to-one
  onto S035-S043 via `closes:` fields; none is dangling.
- **Literature formulations.** S033's reverse futility shape
  (`eval - margin*depth >= beta` at non-PV, not-in-check, shallow depth, mate
  band guarded — the guards exist at `src/search.cpp:321-323`), S023's capture
  history indexing (piece, target, victim), S022's delta pruning, S026's
  futility/razoring split with a mate test required first, S021's SPRT-bounds
  caution, and S029's `(768 -> N) x 2 -> 1` perspective architecture with
  per-ply accumulator pop over the S008 primitives are all the standard,
  documented forms. S032's ~1 % PEXT figure is consistent with published
  comparisons.
- **Code premises.** Nothing pending is already implemented: no aspiration,
  RFP, futility, razoring, delta pruning, capture or continuation history in
  `src/search.cpp`/`src/evaluation.cpp`; `move_t` is `uint32_t` with the piece
  in bits 12-15; the side-to-move key is two xors; the en passant square is set
  unconditionally; the opening book S030 names exists (`src/openings.book`).
  `is_check` is genuinely recomputed across call sites for the same node
  (parent's give-check scan at `search.cpp:414`, child's own scan at `:289`,
  quiescence's at `:130`).
- **Build and suite at HEAD.** `cmake --build build -j12` clean; `ctest -L
  fast` 9/9 including `test_fastchess_script`.

## Verdicts on prior findings (2026-08-13_adversarial, re-checked at ea61e5f)

Each re-measured from its own reproduction. Status changes to `closed` belong
to a re-run of the *adversarial* audit, not to this one; verdicts here record
what reproduces today.

- **F01 (fastchess.sh aborts, reports success)** — fix verified. `fastchess.sh:45`
  now sets `all_cores`, `:119` prints it, the EXIT trap at `:90` preserves the
  entering status (`trap 'status=$?; rm -f "$snapshot"; exit $status' EXIT`).
  `tests/test_fastchess_script.sh` (stubbed `fastchess`, red observed twice per
  S035's stamp) passes in the fast suite. Correctly `planned` pending re-run.
- **F02 (`go` with 1 ms clock never answers)** — fix verified.
  `compute_search_time_ms` floors at 1 (`src/chesso.cpp:409`,
  `std::max(budget, std::max(1, std::min(50, remaining_ms / 2)))`). Live:
  `go wtime 1 btime 300000` answered `bestmove d2d4` before a `readyok` sent
  3 s later. Cases `{1,0,20}` and `{1,100,1}` at `tests/test_engine.cpp:424-425`.
  Correctly `planned` pending re-run.
- **F03 (`info nodes` per-iteration)** — still reproduces. `go depth 7` on the
  midgame FEN prints per-iteration counts (`depth 6 nodes 53601`, `depth 7
  nodes 121373`, not cumulative); `tools/search_bench.py ./build/src/chesso 7`
  prints `midgame ... 45397 nodes`, the audit's exact last-iteration figure.
  S037 pending and next in plan order. Correctly `planned`.
- **F04 (tuner-model 2 cp tolerance)** — still reproduces.
  `tests/test_eval_model.cpp:313` asserts `<= 2.0`, comment at `:294` still
  says "twice"; three separate `/ GAME_PHASE_MAX` truncations at
  `src/evaluation.cpp:638-641` and `:949-952`. S038 pending. `planned`.
- **F05 (LAZY_EVAL_MARGIN precondition false)** — still reproduces.
  `src/evaluation.hpp:278-282` still says king safety "ships at zero weight";
  the weights at `src/evaluation.cpp:731-734` are nonzero; the margin is 150.
  S039 pending. `planned`.
- **F06 (DEV_MANUAL tuner section stale)** — still reproduces, at shifted
  lines: `DEV_MANUAL.md:385` "fits 825 numbers" (PARAM_COUNT is 827,
  `tools/tuner.cpp:9` says so itself), `:407-409` group list stops before
  `tempo` (`GROUP_LIST` at `tuner.cpp:153-155` carries it), `:395-398` paste
  target omits the tempo weights, `:413-414` "happened three times" against
  four recorded in `free_mask`. S040 pending. `planned`.
- **F07 (`free_mask` untested)** — still reproduces:
  `grep -rn "free_mask\|GROUP_LIST" tests/` exits 1 with no output. S041
  pending. `planned`.
- **F08 (en passant square set unconditionally)** — still reproduces:
  `src/bitboard.cpp:826-835` sets `new_en_passant` from `move.double_push` with
  no capturability test. S042 pending. `planned`.
- **F09 (dead CMAKE_TOOLCHAIN_FILE line)** — still reproduces:
  `CMakeLists.txt:7` unchanged, no `toolchain.cmake` in the tree, and the line
  still sits after `project()`. S043 pending. `planned`.

Summary: two fixed and verified (F01, F02), seven still present, all nine
correctly mapped to steps. No prior finding is misstated `closed`.
