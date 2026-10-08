id:         S269
goal:       once late move pruning has fired, a quiet move skips futility, history pruning, quiet SEE and S091's exchange test, none of whose answers can change what happens to it
accepts:    (1) in `src/search.cpp` `negamax_at`, the three per-move quiet rules are not evaluated for a quiet once `skip_quiets` is set, and `see_loses_material` is not computed for such a quiet; (2) **node identity** (INV-6): `bench` and `tools/search_bench.py` node counts and best moves identical to the parent at two depths; (3) an interleaved timing on the workstation, noise floor read first (CLAUDE.md rules 4 and 5), with instruction and cycle counters beside nps; kept only if faster, and timed against the parent alone, not stacked on S268; (4) under PROBING a skipped quiet records `PRUNE_LATE_MOVE`; a test case states which rule the probe records for it; (5) DEC-141's second tier: Debug self-play, four rounds at 4+0.04, the log grepped for `Assertion`, and `tools/gate_extra.sh`, both named in the stamp
touches:    src/search.cpp, tests/test_search.cpp; amended at implementation for the mutants and the anchor the change moved: tools/mutants/S269_late_quiets.py, tools/mutants/S091_capture_see.py (R02's anchor)
excludes:   any change to which moves are pruned, reduced or searched; the late move pruning threshold; the pre-make gives-check test, which is S268
decisions:  DEC-083, DEC-141, DEC-260, DEC-263
closes:     2026-10-08_performance-F02
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-08 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against d8073ed: bench 4081329 and bench 12 1860699 with every info line and bestmove identical; search_bench 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at depth 12, c3d5 / e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical. No SPRT owed (DEC-083). A quiet past late move pruning's count is no longer asked futility, history pruning, quiet SEE or S091's see_ge(0); re-derived on the S268 tree, the readers of both values reach the same branch and the exchange value is never read for such a quiet. -2.0 % instructions and -2.8 % cycles on bench 16, -2.0 % and -2.2 % on 300 positions at depth 11 (perf_counters; A/A 0.000 % instructions, +0.19 % and +0.06 % cycles); +2.09 %, 95 % CI +1.91 .. +2.27, 24 interleaved hyperfine pairs, 24/24 faster, A/A floor +/- 0.25 %; bench_movegen resolution 0.1 %; governor powersave. About +3 Elo at long time control as a conversion, not a verdict. KEPT. Under PROBING such a quiet records PRUNE_LATE_MOVE ("a quiet past late move pruning's count carries the late move mark"); two cases hold that captures past the count still meet S091's two rules. LQ01 to LQ03 and S109's ten, S091's six and M08 killed, 20 of 20; R02's anchor moved with the line. Suites 43/43 in build and build-tune, format clean (DEC-146 override), prose checks clean. Second tier (DEC-141), by the coordinator: Debug self-play 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`; `gate_extra` 5 stages green in 870 s (`.tuning/coord/S269/gate_extra/`). Fast check: nothing found.

## Why

Once `skip_quiets` is set, a quiet's fate is fixed: any rule's verdict and
the late move rule's verdict reach the same branch, and the exchange value is
read only behind a reduction guard such a move cannot pass. At `bench`, 46 %
of the quiet-SEE calls and 28 % of S091's calls are made in that state. The
audit's two-line prototype was node-identical on three checks and measured
-1.6 to -2.4 % instructions and -2.7 to -3.9 % cycles on the M1.
`adocs/audit/2026-10-08_performance.md`, F02, has the diff and the counts.

## Lane

Agent work under DEC-260; it lands on the workstation's timing.
Behaviour-neutral, so no match (DEC-083).

## As built (2026-10-08, Opus subagent)

### Deviations, first

1. **Two more cases and three mutants where the step asked for one case.**
   The brief's suggested over-reach mutant -- the exchange test behind S091's
   extra ply skipped for every move past the count, captures included (LQ02)
   -- moves the bench (4081329 -> 4015467) and left the fast suite green
   when run by hand (all but the format test, run without its override): a
   proved gap. No case drove a non-PV node where a
   capture is reached after late move pruning's flag. Two cases now do,
   one per S091 rule, and LQ02 and LQ03 are the mutants they kill.
   `touches:` amended.
2. **R02's anchor moved.** `tools/mutation_check.py` validates every anchor
   in `tools/mutants/` before any run, and R02's
   (`tools/mutants/S091_capture_see.py`) was the line S269's conjunct split.
   It is now the `see_ge` call alone; the mutation is the same sign flip.
   `touches:` amended.
3. **The late move mark keeps its `prune_rule == PRUNE_NONE` test**, though
   since S269 it can no longer be false for a quiet past the count. It is what
   keeps the rules' precedence explicit, and it is why LQ01 is visible to the
   probe at all: without it, a rule's mark would be overwritten by the late
   move mark and the parent's code would record the same rule.
4. **The new conjunct sits on the `!MOVE_PROMOTED` line**, ahead of `see_ge`,
   not on its own line after `ply > 0` as the audit's prototype put it. All
   the conjuncts are pure; only where the costly call sits matters.

### The change

`src/search.cpp` `negamax_at`, two conjuncts and their comments:

- the three per-move quiet rules are gated `may_prune && is_quiet &&
  !skip_quiets`;
- `see_loses_material` gains `!(skip_quiets && is_quiet)` ahead of its
  `see_ge(0)` call.

Nothing else in `src/` moved. The late move pruning threshold, the flag and
its mark are untouched.

### Item 1, re-derived on today's tree

Take a quiet `q` (not a capture, not a promotion) reached with `skip_quiets`
set. Lines are the new tree's.

- **Before S269** the three rules ran when `may_prune` held and could mark
  `q` FUTILITY, HISTORY or SEE. The capture rule (`:2581`) skips a quiet.
  `see_loses_material` (`:2608`) was false without a `see_ge` call when a rule
  had marked `q`, and asked `see_ge(0)` otherwise. The late move mark
  (`:2619`) then set LATE_MOVE on any `q` still unmarked. So `prune_rule` is
  not NONE after `:2619` whatever the rules said.
- **After S269** the rules do not run on `q` and `see_loses_material` is
  false without the call. `:2619` marks `q` LATE_MOVE. Again not NONE.

Every later reader of the two values, by grep of `negamax_at`:

- `:2645`, the engine node's pre-make exemption, reads `prune_rule` only as
  `!= PRUNE_NONE`: both trees skip `q` unless `move_gives_check()` says it
  checks, and seed `child_check = 1` when it does.
- `:2702`, `capture_gives_check`, starts with `is_capture`: false for `q` in
  both trees, whatever `prune_rule` and `see_loses_material` hold.
- `:2710`, the probed node's post-make exemption, reads `prune_rule` only as
  `!= PRUNE_NONE`, and its probe record (`:2714`) reads the value. That record
  is the one difference, test-only, and item 4 states it.
- `:2792`, the extra ply, reads `see_loses_material` behind `may_reduce`
  (`:2772`), which needs `!is_check_move()`. `q` reaches it only by surviving
  an exemption: in the engine node `child_check` was seeded 1, in the probed
  node `is_check_move()` was true at `:2710` (`capture_gives_check` being
  false). Either way `is_check_move()` is true and `may_reduce` false, so the
  value is never read for `q`.
- Nothing else in `negamax_at` reads either value. The skipped work is pure:
  `lmr_depth_of`, a history read, `see_ge` on a `const board_t*`. The
  `assert(pruning_eval != TT_EVAL_NONE)` it skips is Debug-only.

`may_prune` can turn false after the flag is set (alpha into the mate band);
the late move mark does not read it, in either tree, so such a quiet is
handled alike.

**What S268 changed in the argument: its shape, not its conclusion.** At the
audit's `29348c5` the late move mark and the exemption both sat after
`make_move`. S268 put the engine node's exemption before `make_move`, reading
`prune_rule` the same way, and moved the late move mark to just after
`see_loses_material`. Two consequences: "any rule's verdict and late move
pruning's reach the same branch" now holds at two sites, the engine node's
pre-make test and the probed node's post-make one; and the reduction guard is
now failed through the memo the pre-make test seeds rather than a post-make
`is_check()`. And because the mark still follows `see_loses_material`, its
`prune_rule == PRUNE_NONE` conjunct does not see LATE_MOVE, so the explicit
`!(skip_quiets && is_quiet)` is still needed -- as in the audit's prototype.
The audit's line numbers (`:2502`, `:2591-2594`, `:2650-2665`, `:2743`) all
moved; its prototype's two hunks still describe the change.

### The probe case (item 4) and the two capture cases

`tests/test_search.cpp`, three cases, each observed red under its mutant
before being called a guard:

- **"a quiet past late move pruning's count carries the late move mark"**,
  beside S109's history case. `PRUNE_POS` at depth 3, ply 1, non-PV, the wide
  window; a2a3 (no check, from the engine's own `make_move` + `is_check`) has
  its history planted at `-QUIET_HISTORY_MAX`. The precondition that would
  make the old record appear: history pruning's own test selects a2a3 at every
  move number the node can reach (`lmr_depth < HP_MAX_LMRDEPTH` and the
  threshold, asserted for 1 .. 48). The plant also sorts it last, so it is
  reached after the flag if the flag is set. Asserted: `skip_quiets_set`, a2a3
  not searched, `rule_that_pruned == PRUNE_LATE_MOVE`. Under LQ01 (the parent's
  gate): `REQUIRE_EQ( 2, 4 )`, history pruning's mark.
- **"a capture past late move pruning's count is still skipped by the capture
  rule"** and **"... still takes the extra ply"**, in the S091 block. A position
  from `tests/assets/test_jsons/castling.json` (python-chess: valid, not in
  check, 52 legal moves, 10 captures, 4 promotions that take nothing, Bxf7
  gives no check), driven at ply 2 with `static_evals[0]` planted at 20000 so
  the node is not improving and the count is the undoubled 8 -- at ply 1 the
  doubled count is never reached by this node's captures. Bxf7 loses material
  by the engine's `see_ge` (exactly 200: `see_ge(-200)` true, `see_ge(-199)`
  false). The flag's move number is computed from the engine's own probes
  (`lmp_flag_move_number`, the rule's own test over move numbers 2 ..).
  - Depth 3: Bxf7 is pruned, so its move number is bounded below by the
    captures `score_move` ranks above it that were searched (8, so >= 9 >= 8),
    and the capture margin at that bound's reduced depth (0) writes it off.
    Asserted `PRUNE_SEE_CAPTURE`. Under LQ03: `REQUIRE_EQ( 0, 5 )`.
  - Depth 10: Bxf7 is searched at index 12 (move 13 >= 8), reduced depth 5,
    margin -250 clears its -200; the extra ply is the only reduction a capture
    takes. Asserted `reduction == SEE_LMR_EXTRA`. Under LQ02: `REQUIRE_EQ( 0,
    1 )`. Depth 9 would sit on the margin exactly (reduced depth 4, -200), so
    10 is the drive depth.

Found by a throwaway explorer over `all_test_fens()` plus the two block
positions, depths 3 to 9, plies 1 and 2 (`.tuning/coord/S269/explore.inc`,
never committed): 32 late-capture extra-ply hits and 24 late-capture skip hits,
all at ply 2 and all in capture-rich positions. No golden: every number the
cases read is re-read from the engine's tables in the case.

### INV-6 (item 2)

Against a Release build of `d8073ed` (`.ref-builds/s269_base`, a detached
throwaway worktree, removed after; its binary is byte-identical to the
worktree's own build before the change). Streams stripped of `time` / `nps` /
`hashfull` only and diffed whole (`.tuning/coord/S269/inv6.sh`, S117's):

- `chesso bench` **4081329**, all 121 lines identical, every info line and
  `bestmove`;
- `chesso bench 12` **1860699**, 105 lines identical;
- `tools/search_bench.py` depth 9 **32932 / 70095 / 25178**, `c3d5` / `e2a6`
  / `d7c8q`; depth 12 **67792 / 280873 / 137893**, `c3d5` / `d5e6` /
  `d7c8q`; identical.

Every timed run below also prints the node totals: `bench 16` 10214429 and
the 300 positions 26512806 on both binaries, every run. The binary timed is
the one the final tree builds (`cmp`).

### The timing (item 3), against the parent alone

Machine: governor `powersave` (recorded, not set, DEC-195); load average
0.54 before, 0.88 to 1.24 during (the run's own engine); the top processes by
lifetime CPU were firefox, this session, the terminal, `btop` and the
compositor, none busy. `bench_movegen -r 40` on the candidate: **resolution
0.1 % / 0.1 % / 0.6 %** (perft / generation / captures). Everything
interleaved, order swapped every pair, the parent against a byte copy of
itself first; readings in `adocs/data/S269_timing.txt`, scripts in
`.tuning/coord/S269/` (S117's, repointed; S268's counter wrapper rebuilt,
since its scripts were gone).

| workload | instrument | A/A (parent vs a byte copy) | parent vs S269 |
|---|---|---|---|
| `bench 16` | instructions | +0.000 %, 12 pairs | **-1.965 %**, 16 pairs, every pair |
| `bench 16` | cycles | +0.19 %, CI -0.20 .. +0.57 | **-2.82 %**, CI -3.15 .. -2.48, t -17.6, 16/16 |
| `bench 16` | nps (engine's own) | 4292702 / 4272935 | **4252848 -> 4381935, +3.0 %** |
| 300 positions, depth 11 | instructions | -0.000 %, 8 pairs | **-1.989 %**, 12 pairs, every pair |
| 300 positions, depth 11 | cycles | +0.06 %, CI -0.28 .. +0.40 | **-2.16 %**, CI -2.28 .. -2.05, t -39.8, 12/12 |
| 300 positions, depth 11 | hyperfine wall | ratio 1.0004, CI 0.9980 .. 1.0028, t 0.35, 16 pairs | **ratio 0.9795, CI 0.9778 .. 0.9812, t -24.29, 24/24** |

**Speed-up +2.09 %, 95 % CI +1.91 .. +2.27 %** (hyperfine, per-pair sd
0.42 %, A/A floor about +/- 0.25 %). Under CLAUDE.md's 3 % line, so rule 5's
exception is what admits it: `hyperfine` over interleaved runs with a tight
sigma, every pair faster, the A/A an order of magnitude inside it, and the
instruction count -- which repeats to the last digits -- agreeing in sign and
size on both workloads. Against the audit's M1 pointer (-1.63 % instructions,
-2.70 % cycles on `bench 16`; -2.41 % and -3.88 % on its 100 positions) the
`bench 16` figures land inside its range and the positions' below it. At
DEC-083's published rates +2.09 % is **about +3 Elo at long time control and
+4 at short -- a conversion, not a verdict.**

**Keep-or-revert: KEPT on +2.09 % (CI +1.91 .. +2.27), -2.0 % instructions,
-2.2 to -2.8 % cycles.**

### The mutants

`tools/mutants/S269_late_quiets.py`, new prefix LQ: LQ01 the three rules
asked of a quiet past the count again (the parent's gate), LQ02 the extra
ply's exchange test skipped for every move past the count, LQ03 the capture
rule skipped past the count. Each observed red by hand on its own case first,
then `tools/mutation_check.py tools/mutants .ref-builds/mut --only LQ01 LQ02
LQ03 P01 P02 P03 P04 P05 P06 P07 P08 P09 P0A C02 C05 C06 C07 R01 R02 M08
--jobs 12` under `CLANG_FORMAT_MAJOR=22` on fixture `1549b27` (`d8073ed` plus
this tree through a temporary index and `git commit-tree`, no ref moved,
worktree removed after), baseline green 43 tests, bench 4081329: **20 of 20
killed**, wall 2388 s (`.tuning/coord/S269/mutation/`). A first attempt
without the format override refused at its red baseline
(`test_clang_format_script`), before any mutant.

| id | fast failed | bench | what kills it |
|---|---|---|---|
| LQ01_late_quiet_rules_asked | 1/43 | **same** | the late move mark case alone |
| LQ02_late_skip_reaches_extra_ply | 1/43 | moved | the extra ply case alone |
| LQ03_late_skip_reaches_capture_rule | 2/43 | moved | the capture rule case, `test_mate_carry` |
| P01 to P09, P0A (S109's ten) | 1 to 4/43 | moved, P04 and P0A same | their own cases, as before |
| C02, C05, C06, C07, R01, R02 (S091's six) | 1 or 2/43 | moved | their own cases, as before |
| M08_lmr_checks | 4/43 | moved | as before |

LQ01 leaves the bench still by construction: it is S269's parent. LQ02 was the
proved gap the hand run found (bench moved, suite green) and is now killed by
one case only. The new cases also go red, as collateral, under C05 (the extra
ply case's `k >= 3`), R02 (its reduction), P08 and P09 (the late move mark
case's `skip_quiets_set` precondition).

**Goldens (DEC-142).** None added. `test_search.cpp`'s `capture_mates` rows
carry S091 mutant labels (`DEV_MANUAL.md` table); not re-derived, because
neither end moved: the engine is node-identical, and every S091 mutant's tree
on this tree is its tree on the parent -- each touches captures only, and
R02, whose anchor moved, flips a value that is never read for a quiet past the
count (the item 1 argument holds under it). All 181 anchors in
`tools/mutants/` were checked to resolve exactly once on the new tree before
the run.

### Suites

`cmake --build build -j12 && ctest --test-dir build -L fast --output-on-failure
&& cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast
--output-on-failure && ./clang-format.sh --check` under
`CLANG_FORMAT_MAJOR=22` (DEC-146): **43/43, 43/43, format clean**, exit 0
(`.tuning/coord/S269/gate.log`, re-run on the final tree). The first gate run
broke in `build-tune` on a `constexpr` built from `LMR_NO_TT_MOVE`, a
settable variable there (DEC-118's case exactly); the adjustment is a function
now. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`, one
mode per call: each exit 0, 0 flagged. Debug self-play and `gate_extra.sh`
(DEC-141's second tier) are the coordinator's.

`MANUAL.md` checked: no UCI surface, option, default or output moved -- no
change. `DEV_MANUAL.md` checked: no command, flag or default moved; the
`capture_mates` re-derivation command still names
`tools/mutants/S091_capture_see.py`, whose R02 still applies -- no change.
`specs.md`: one sentence of the search row proposed in the report, not edited
here.

### Proposed `done:` stamp

2026-10-08 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against d8073ed: bench
4081329 and bench 12 1860699 with every info line and bestmove identical;
search_bench 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at
depth 12, c3d5 / e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical. No SPRT owed
(DEC-083). A quiet past late move pruning's count is no longer asked futility,
history pruning, quiet SEE or S091's see_ge(0); re-derived on the S268 tree,
the readers of both values reach the same branch and the exchange value is
never read for such a quiet. -2.0 % instructions and -2.8 % cycles on bench 16,
-2.0 % and -2.2 % on 300 positions at depth 11 (perf_counters; A/A 0.000 %
instructions, +0.19 % and +0.06 % cycles); +2.09 %, 95 % CI +1.91 .. +2.27, 24
interleaved hyperfine pairs, 24/24 faster, A/A floor +/- 0.25 %; bench_movegen
resolution 0.1 %; governor powersave. About +3 Elo at long time control as a
conversion, not a verdict. KEPT. Under PROBING such a quiet records
PRUNE_LATE_MOVE ("a quiet past late move pruning's count carries the late move
mark"); two cases hold that captures past the count still meet S091's two
rules. LQ01 to LQ03 and S109's ten, S091's six and M08 killed, 20 of 20; R02's
anchor moved with the line. Suites 43/43 in build and build-tune, format clean
(DEC-146 override), prose checks clean. Second tier (DEC-141), by the
coordinator: Debug self-play 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`; `gate_extra`
5 stages green in 870 s (`.tuning/coord/S269/gate_extra/`). Fast check: nothing found.

### Proposed commit text

```
Skip the rule tests on a quiet past late move pruning (S269)

Once late move pruning has set its flag, a quiet is skipped unless it
gives check whatever futility, history pruning and quiet SEE say, and
S091's see_ge(0) is read for a quiet only behind a reduction guard such
a move cannot pass. None of the four answers could change what happens
to it, and the quiet SEE call was the exchange evaluator's largest
caller (2026-10-08_performance-F02). S269 stops asking them.

Node-identical to d8073ed (INV-6): bench and bench 12 streams and
search_bench at depths 9 and 12 identical. On the workstation: -2.0 %
instructions, -2.2 to -2.8 % cycles, +2.09 % wall over 300 positions at
depth 11 (CI +1.91 .. +2.27, 24/24 pairs, A/A floor 0.25 %). Kept.

Under PROBING such a quiet now records PRUNE_LATE_MOVE; a case states
it, and two cases hold that a capture past the count still meets
S091's capture rule and extra ply. Mutants LQ01 to LQ03; R02's anchor
follows the line the new conjunct split.

No functional change
```
