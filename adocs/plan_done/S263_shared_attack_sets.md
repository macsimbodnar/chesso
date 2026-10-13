id:         S263
goal:       evaluate() computes each side's attack sets once per call and every term that needs them reads the shared sets, so the new mobility, threat, king-safety and outpost terms cost a lookup rather than a recomputation
accepts:    (1) per side, computed once per evaluation where the expensive stage already computes piece attacks: attacks by piece type, all attacks, squares attacked twice, pawn attacks and pawn attack spans; mobility and king safety read them instead of computing their own; (2) **score identity**: `bench` and `tools/search_bench.py` node counts and best moves identical to the parent at two depths (INV-6) -- the same score at every node; (3) `tests/bench_eval.cpp` ns per call and nps measured interleaved against the parent with the noise floor read first (CLAUDE.md rules 4 and 5), a regression above it explained or fixed; (4) only sets with a consumer in the tree or in the next step that lands are built -- a set nobody reads is a per-node cost with no return; (5) S262's trace still reconstructs `evaluate()` exactly; (6) any table this adds states per-thread or shared at its declaration (DEC-175)
touches:    src/evaluation.cpp, src/evaluation.hpp, tests/test_evaluation.cpp, tests/bench_eval.cpp, adocs/specs.md
excludes:   any new term or weight; the pawn-structure cache, which is S118 and lands after the pawn terms it caches (DEC-087); pin detection, which S121 adds with its consumer
decisions:  DEC-258, DEC-036, DEC-037, DEC-175
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-13 -- evaluate()'s pawn stage builds each side's squares beside its pawns, pawn attacks and pawn attack span once, and backward and passed pawns read them, as locals of the cheap stage that an expensive-stage term cannot read: four neighbour_files() a side become three, evaluate_cheap() 196 -> 187 instructions, evaluate_expensive() identical (objdump). No per-side union (attacks by type, all attacks, attacked twice) is built: stage two already looks each piece's attack set up once for mobility and king safety, and no term reads a union ((4)); the later steps that cite "the attack sets come from S263" build their own with their consumer. INV-6: bench 3872032, bench 12 1730486, search_bench d9 32829/69892/25174 and d12 68090/246712/114555, streams and moves identical; evaluate() and evaluate_cheap() byte-identical on all 10795695 corpus rows. Timing under DEC-266 (A/A first, both padded, interleaved, 2026-10-13; void and drift checks pass): **zero** -- instructions -0.254 % bench 16 and -0.294 % 300 positions, but cycles -0.259 % (CI -0.526 .. +0.009, floor 0.419) and -0.330 % (CI -0.496 .. -0.164, floor 0.339) do not clear their floors; hyperfine +0.42 % speed-up (CI +0.28 .. +0.56, floor 0.26), bench_eval -0.435 % ns (CI -1.329 .. +0.467, floor 0.817); no regression; kept for score identity and fewer instructions (rule 8). Trace (S262) re-run: 10795695 rows, 0 differ from evaluate(), the clamp binds on 36503. Test: "the shared pawn sets count what the definitions say" (5392 test positions with their mirrors, 17587 distinct seeded placements, per colour, non-vacuous), six mutants red, green on the parent's source. Fast suites 45/45 both, format clean, prose checks clean (citations and touches 0 flagged over 35 files, params clean). specs evaluation row; DEV_MANUAL and MANUAL checked, no change. Implemented from CPW's Attack and Defend Maps and Grant (2020) (DEC-221). No second tier: evaluation only. Fast check: no code defect; five record and comment items (the stage scope as a deviation, two specs wordings, the fill-then-widen comment, the objdump evidence, distinct-placement counting) fixed before commit.

## Why

DEC-036/037: mobility recomputed per call once cost a third of the nps and
measured -14.93. S121, S101, S122 and S102 all read attack sets; computed once
and shared, each costs a few bit operations. Score identity makes this a
timing and not a match (DEC-083).

## Description it is implemented from

CPW *Attack and Defend Maps*; Grant 2020, the structure of a modern
hand-crafted evaluation. The 2026-10-07 proposal's P15, without its pawn
cache, which stays S118's in DEC-087's order.

## Lane

Agent work under DEC-260; compiled and timed between runs.

## Progress (paused 2026-10-12)

*Superseded by "As built" below; kept as the record of the pause. The disturbed
timing it names under `runs/` now lives in `runs_disturbed/`.*

- **Done, tree compiles** (build/ and build-tune/ configured in the worktree). `src/evaluation.cpp`: the pawn stage builds `beside` (neighbour_files of the pawns), pawn `attacks` (beside shifted a rank) and the pawn `attack_span` (neighbour_files of the front span) once a side; stops = front | span, backward's neighbour set = beside | span; `widen_by_one_file()` deleted. Four neighbour_files() a side -> three; `evaluate_cheap()` 196 -> 187 instructions; `evaluate_expensive()` identical. Comment at the fused mobility/king-safety loop. `tests/test_evaluation.cpp`: new case "the shared pawn sets count what the definitions say" (per pawn, square arithmetic, test FENs + mirrors + 20000 seeded placements). `adocs/specs.md`: one sentence in the evaluation row; `touches:` gained specs.md.
- **Finding:** stage two already looks up each piece's attack set once and both terms read it; no term reads attacks by type, all attacks or attacked twice, so none is built ((4)). S121/S101/S122/S102's "the attack sets come from S263" now point at nothing: those steps build their own sets. The only sharing available was the pawn stage's.
- **Verified:** bench 3872032, bench 12 1730486, search_bench d9 32829/69892/25174 and d12 68090/246712/114555, same moves, every stream identical to the parent (`.tuning/coord/S263/inv6_{parent,cand}`); `evaluate()` and `evaluate_cheap()` byte-identical on all 10795695 corpus rows (`.tuning/coord/S263/dump/`; a both-colours mutant changes 691209 rows). Six mutants of the shared sets, each red in the new case (M6, symmetric, red only there and in test_eval_model) (`mut_*.log`); the new case passes on the parent's source. Gate run once: both fast suites green (45/45 each), clang-format red on my files, fixed by `./clang-format.sh`; `--check` green since. **Gate not re-run after the format fix and comment edit.**
- **Timing DISTURBED, re-take whole:** `.tuning/coord/S263/runs/` (run_s263.sh). A/A clean; from the change half on, firefox (owner) ran, base cycles +7 % against the A/A. Readings, not a verdict: instructions -0.254 % bench 16, -0.294 % 300 pos; bench_eval ns -1.28 % (CI -2.06 .. -0.49); hyperfine +0.17 % (CI -0.44 .. +0.79).
- **Next:** (1) re-take the timing whole on a quiet machine: `nohup .tuning/coord/S263/run_s263.sh > .tuning/coord/S263/run_s263.log 2>&1 &` (binaries in `.tuning/coord/S263/bin/{parent,cand}`; cand = current tree, cmp before), read under DEC-266; (2) trace re-run: `build/tools/tuner --data /home/max/ws/chesso/.tuning/selfplay_v2_dedup.tsv --epochs 0`, read the `trace:` line; (3) full gate with CLANG_FORMAT_MAJOR=22, then `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`; (4) write "## As built" per the brief, save step.diff.

## As built (2026-10-12 to 2026-10-13, Opus subagent; resumed after the shutdown)

Evidence is under the worktree's `.tuning/coord/S263/` (gitignored). The
commands behind the timing, the trace, the gate and the prose checks are in
`.tuning/coord/S263/pending.md`.

### Deviations

- **No per-side union is built.** Accepts (1) lists attacks by piece type, all
  attacks and squares attacked twice; no term in the tree reads any of them,
  so under (4) none is built. Stage two already looks each piece's attack set
  up once, in the fused mobility and king safety loop, and both terms read
  that one lookup: there was nothing to share there, and a comment at the loop
  now says so. The only sharing available was the pawn stage's.
- **The pawn sets live in the cheap stage, not the expensive one.** Accepts
  (1) asks for the sets "where the expensive stage already computes piece
  attacks". The three pawn sets are locals of `evaluate_pawns()`, which runs in
  `evaluate_cheap()`. An expensive-stage term cannot read them: S121's
  enemy-pawn mask, S101's pawn threats and S102's outposts will each build the
  pawn set they need in their own stage. The specs sentence says "built once
  within its stage" for that reason.
- **The plan's later steps point at sets that do not exist.** S101, S102, S121,
  S122, S123, S124, S125 and S135 each say "the attack sets come from S263".
  None was built here; each such step builds the set it reads, in that loop,
  and its own measurement pays for it. Rewording those lines is a plan change
  and the coordinator's.
- **Touches:** `src/evaluation.hpp` and `tests/bench_eval.cpp` unchanged;
  `adocs/specs.md` added.
- **Run after S119's SPRT, by DEC-260's lane:** the timing, the trace re-run,
  the gate after the format fix and the three prose checks. The coordinator
  took the timing on 2026-10-13 from 07:05 to 07:22. The rest ran after it at
  normal priority. Meanwhile achesso moved to `338705e`. `src/`, `tests/` and
  `tools/` are unchanged there from `bee84d8`, so the diff lands as measured.
- **CPU-bound work ran in the first minutes of S119's match**, at `nice -n 19`,
  against DEC-260, about 30 s in all: a no-op `cmake --build build`, one
  `bench`, the parent clone's build of `chesso` and `bench_eval` (about 17 s),
  and a tuner start killed after about 10 s with no output. Listed with times
  in `pending.md`.

### The sets built, each with its consumer

Once per side, in `evaluate_pawns()`, each read where named:

| set | definition | read by |
|---|---|---|
| beside | `neighbour_files(pawns)`: level with each pawn, the files next to it | pawn attacks; backward's neighbour set (with the span) |
| attacks | beside shifted one rank forward | backward's stop-square test (the enemy's) |
| attack span | `neighbour_files(front span)` | passed pawns' stops (with the front span); backward's neighbour set (with beside) |

`widen_by_one_file()` is deleted: stops are `front | attack span`, and
backward's set is `beside | attack span`, which equals the old
`neighbour_files(ahead)` because `neighbour_files()` distributes over a union.
The pawn attacks were `neighbour_files(pawns >> 8)` and are now `beside >> 8`,
equal because a whole-rank shift commutes with the file masks. Four
`neighbour_files()` a side (isolated's included) become three.
The parent's and the candidate's `evaluation.cpp.o` were compared function by
function with objdump, addresses and branch targets normalised
(`fncmp_resume.txt`; `fncmp_resume_norm.txt` also drops padding prefixes and
nops):
- `evaluate_cheap()` goes from 196 to 187 instructions.
- `evaluate()`, `evaluate_lazy()`, `evaluate_expensive_terms()` and
  `capture_score()` are identical. `evaluate_expensive()` is identical too, at
  426 instructions.
- `evaluate_mobility_and_king_safety<true>` differs only in branch-padding
  prefixes, nops and the offsets they shift.
- `passed_pawn_counts()`, `pawn_structure_counts()` and
  `piece_placement_counts()` are the counting entry points that tests read; the
  search does not. They used to call one out-of-line copy of the pawn stage
  (`evaluate_pawns<true>`, 305 instructions). Each now inlines it.

No table is added, so (6) has nothing to declare.

### INV-6 and per-row score identity

- `bench` 3872032, `bench 12` 1730486. `tools/search_bench.py` depth 9:
  32829 / 69892 / 25174, `c3d5` / `e2a6` / `d7c8q`. Depth 12: 68090 / 246712 /
  114555, `c3d5` / `d5e6` / `d7c8q`. Every stream, timings stripped, is
  byte-identical to the parent's (`inv6_parent/` against `inv6_cand/`, four
  files). `bench` re-read 3872032 at resume, from the final source.
- `evaluate()` and `evaluate_cheap()` of all 10795695 corpus rows, as int32
  pairs: `dump/parent.bin` and `dump/cand.bin` are byte-identical (86365560
  bytes each). The same dump from a both-colours mutant changes 691209 rows, so
  the comparison can see a wrong set.

### The timing (accepts (3))

**Verdict: zero under DEC-266.** Instructions fall by -0.254 % on bench 16 and
-0.294 % on the 300 positions, exactly as in the first run, on every pair.
Cycles, wall and nps all point the same way. But neither workload's cycles CI
lies wholly beyond its A/A floor, and DEC-266 (3) needs both. So the gain is
recorded as zero (rule 8). The change is kept for its reason: the score is
identical at every node and on every corpus row, it runs fewer instructions,
and the evaluation has one helper fewer. There is no regression: no CI lies
above its floor.

The run is in `runs/`. The coordinator ran it from 2026-10-13 07:04:40 to
07:22:01 with `run_s263.sh` (`pending.md`, section 4). Both sides were padded.
The pairs were interleaved, order swapped every pair, A/A of byte copies first.

- **Void checks pass.**
  - Bench 16: 9631170 nodes, both sides, both halves.
  - 300 positions: workload totals `300 25551090`.
  - bench_eval checksums `-27425414745967`.
  - The A/A copies are byte-identical.
  - The candidate's sha256 equals build/'s `chesso` and `bench_eval` at the
    final source.
- **Machine.**
  - Load: one-minute 1.00 at the start (five- and fifteen-minute 3.19 and
    7.71, still decaying from S119's match), 1.0 to 1.2 through the run, the
    run's own process included.
  - The owner's firefox was open, as in the disturbed run, but the drift
    check below shows no change between the halves.
  - `bench_movegen` resolution: 0.0 to 0.7 % at the start, 0.0 to 0.4 % at
    the end.
- **Drift check passes.** The change half's base median against the A/A's base
  median: cycles on bench 16 -0.06 %, cycles on the 300 positions -0.12 %,
  bench_eval ns -0.34 %, hyperfine +0.17 %. All are under 1 %.

The floor is the A/A's largest CI bound by absolute value, per workload and
instrument:

| instrument | A/A floor | change | change CI | beyond the floor |
|---|---|---|---|---|
| instructions, bench 16 | 0.000 % | -0.254 % | exact | yes |
| instructions, 300 positions | 0.000 % | -0.294 % | exact | yes |
| cycles, bench 16 | 0.419 % | -0.259 % | -0.526 .. +0.009 | no |
| cycles, 300 positions | 0.339 % | -0.330 % | -0.496 .. -0.164 | no |
| wall_ms, bench 16 | 0.570 % | -0.172 % | -0.503 .. +0.161 | no |
| wall_ms, 300 positions | 0.546 % | -0.323 % | -0.501 .. -0.144 | no |
| hyperfine wall, 300 positions | 0.26 % | +0.42 % speed-up | +0.28 .. +0.56 | yes |
| bench_eval ns per call | 0.817 % | -0.435 % | -1.329 .. +0.467 | no |

The medians, base against candidate:
- bench 16 nps: 4525475 against 4535725.
- bench_eval: 40.600 against 40.370 ns per call.
- hyperfine: 5.799 s against 5.779 s.

Only the hyperfine wall clears its floor. Under DEC-266 (3) that is not
enough, because the cycles on both workloads must clear theirs too.

The first run, 2026-10-12 19:10 (`runs_disturbed/`), is superseded. Its change
half was disturbed: the base side's cycles on bench 16 were +6.9 % against its
A/A. Its readings agreed in sign:
- instructions identical to this run's;
- cycles -0.420 % on bench 16, -0.189 % on the 300 positions;
- bench_eval -1.277 %.

### Trace reconstruction re-run (accepts (5))

The command, run 2026-10-13 at normal priority, took 25 s and exited 0
(`trace_rerun.log`):

`build/tools/tuner --data .../selfplay_v2_dedup.tsv --epochs 0 --threads 12 --out .tuning/coord/S263/trace_rerun_tables.hpp`

It printed:

`trace: 10795695 rows, 266907900 entries (24.72 a row), 9.5 s; 0 differ from evaluate(), the clamp binds on 36503`

The rows, the mismatches and the clamp count are S262's own figures. S262's
trace still reconstructs `evaluate()` exactly. The `--out` file is scratch and
was not shipped.

### Tests

- New, `tests/test_evaluation.cpp`: "the shared pawn sets count what the
  definitions say". `passed_pawn_counts()` and `pawn_structure_counts()` per
  colour, against evaluation.hpp's definitions applied one pawn at a time with
  file and row arithmetic and no set algebra.
- Its population:
  - The test FENs and their mirrors: 5392 positions.
  - 20000 seeded random pawn placements (seed 20261012), which put pawns on
    the a and h files far more often. Each placement is emitted twice, once
    with each side to move, because `load_FEN()` refuses a position whose side
    not to move is in check. The pawn counts do not read the side to move, so
    the second load of a placement checks nothing new. 17587 distinct
    placements load, as 31738 positions.
- Its non-vacuity guards:
  - more than 20000 positions checked (37130);
  - more than 10000 random positions (31738);
  - more than 17000 distinct placements (17587), added at the fast check. The
    floor sits below the count because another standard library's shuffle
    gives a different sequence from the same seed.
  - Every count is non-zero for each colour.

  The counts were read by a print added to the test and then removed.
- Six mutants of the shared sets, each red in the new case: attacks one rank
  off, span keeps its own rank, backward loses the span, stops lose the front,
  attacks the wrong way, and both spans keep their own rank. The last is
  symmetric, and it is red only in the new case and `test_eval_model`.
  `mut_*.log`.
- The new case passes on the parent's `evaluation.cpp`
  (`parent_src_newtest.log`): it tests the definitions, not this change.
- Nothing re-targeted. The fast check changed only the guards and the
  comments, and no assertion on a count. The six mutants were not re-run.

### Suites

- First gate run (19:08): both fast suites 45/45, clang-format red on this
  step's files, fixed by `./clang-format.sh`; `--check` green since. The
  format fix and the last comment edit changed no code: build/'s `chesso`
  from the final source is byte-identical to the timed candidate.
- **The full gate, 2026-10-13, `CLANG_FORMAT_MAJOR=22` exported, `-j12`, at
  normal priority** (`gate2.log`): exit 0 after 203 s. Both builds' fast suites
  passed 45/45 and `./clang-format.sh --check` was clean. build/ had nothing
  to rebuild, and its `chesso` and `bench_eval` are still byte-identical to the
  timed candidate.
- **The gate after the fast check's fixes, 2026-10-13** (`gate3.log`, same
  command): exit 0 after 269 s. Both builds passed 45/45 and `--check` was
  clean. The fixes changed only comments in `src/`, and the rebuilt `chesso`
  is byte-identical to the timed candidate. `bench` is 3872032.
- **The prose checks, one mode per call** (`prose--*.log`, re-run after the
  fixes):
  - `--citations`: 0 flagged over 35 files. This file has 0 code citations.
  - `--touches`: 0 flagged over 35 files. This file has 1 symbol in its goal,
    0 flagged.
  - `--params`: exit 0, no output, which is its clean result.
- No second tier: evaluation only. `make_move`, the generator and the search
  are untouched.

### Documents

- `adocs/specs.md`, the evaluation row, one sentence:
  - Each attack set a term reads is built once within its stage and shared by
    that stage's terms.
  - The expensive stage looks a piece's own set up once for both mobility and
    king safety. That fusion is S027's (2026-08-12); S263 only added a
    comment.
  - The cheap stage's pawn terms build beside, the pawn attacks and the attack
    span once each (S263). They are locals of the pawn stage, so an
    expensive-stage term builds its own.
  - No per-side union is built until a term reads one.
- `DEV_MANUAL.md` and `MANUAL.md` checked: neither names these sets or
  `widen_by_one_file()`, so no change.

### Proposed `done:` stamp

`done:       2026-10-13 -- evaluate()'s pawn stage builds each side's squares
beside its pawns, pawn attacks and pawn attack span once, and backward and
passed pawns read them, as locals of the cheap stage that an expensive-stage
term cannot read: four neighbour_files() a side become three, evaluate_cheap()
196 -> 187 instructions, evaluate_expensive() identical (objdump). No
per-side union (attacks by type, all attacks, attacked twice) is built: stage
two already looks each piece's attack set up once for mobility and king
safety, and no term reads a union ((4)); the later steps that cite "the attack
sets come from S263" build their own with their consumer. INV-6: bench 3872032,
bench 12 1730486, search_bench d9 32829/69892/25174 and d12
68090/246712/114555, streams and moves identical; evaluate() and
evaluate_cheap() byte-identical on all 10795695 corpus rows. Timing under
DEC-266 (A/A first, both padded, interleaved, 2026-10-13; void and drift checks
pass): **zero** -- instructions -0.254 % bench 16 and -0.294 % 300 positions,
but cycles -0.259 % (CI -0.526 .. +0.009, floor 0.419) and -0.330 % (CI -0.496
.. -0.164, floor 0.339) do not clear their floors; hyperfine +0.42 % speed-up
(CI +0.28 .. +0.56, floor 0.26), bench_eval -0.435 % ns (CI -1.329 .. +0.467,
floor 0.817); no regression; kept for score identity and fewer instructions
(rule 8). Trace (S262) re-run: 10795695 rows, 0 differ from evaluate(), the
clamp binds on 36503. Test: "the shared pawn sets count what the definitions
say" (5392 test positions with their mirrors, 17587 distinct seeded
placements, per colour, non-vacuous), six mutants red, green on the parent's
source. Fast suites 45/45 both, format
clean, prose checks clean (citations and touches 0 flagged over 35 files,
params clean). specs evaluation row; DEV_MANUAL and MANUAL checked, no change.
Implemented from CPW's Attack and Defend Maps and Grant (2020) (DEC-221). No
second tier: evaluation only.`

### Commit text

```
Build the pawn attack sets once per evaluation (S263)

S263 asked for each side's attack sets to be computed once per
evaluation and shared. Stage two already looks each piece's attack set
up once, and mobility and king safety both read that lookup. No term
reads a per-side union, so none is built: a set nobody reads costs
every node and returns nothing (accepts (4)).

The one sharing left was in the pawn stage, where backward and passed
pawns each built the files beside the front span. The squares beside
each side's pawns, the pawn attacks and the pawn attack span are now
built once each. Four neighbour_files() calls a side become three, and
evaluate_cheap() drops from 196 to 187 instructions.

INV-6: bench, bench 12 and search_bench at depths 9 and 12 are node-
and move-identical to the parent, and evaluate() is byte-identical on
all 10795695 corpus rows. Under DEC-266 the timing reads zero:
instructions fall 0.25 to 0.29 %, but neither workload's cycles clear
the A/A floor. It is kept for score identity and fewer instructions. A
new test holds the pawn counts to the definitions read square by
square.

Implemented from CPW's Attack and Defend Maps and Grant (2020)
(DEC-221).

No functional change
```
