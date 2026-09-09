id:         S159
goal:       measure whether the second killer slot wants ageing rather than distinctness: the unguarded shift discards slot 1 on every repeat, so S149's -11 Elo may be the guard preserving a stale killer for a whole go
accepts:    the hypothesis below is stated as a hypothesis and then tested, not assumed -- S149 measured the guard, not the mechanism, and nothing here may be written as a finding until a run says it; at least one ageing scheme for the killer table is implemented and decided by SPRT, one change at a time, and the candidates worth trying are the two the mechanism suggests: clear `killer_moves` at the start of each iterative-deepening iteration rather than once per `go` (`src/chesso.cpp` `iterative_deepening_search` builds `search_state_t state = {}` once), and shift slot 1 down on a repeat instead of copying slot 0 onto itself, which ages the second slot without ever letting a distinct move survive a repeat; the duplicate rate is re-counted on the S149 instrumentation for whatever ships, so the ordering table's state is documented by number rather than by argument; a verdict of zero is recorded as zero and `adocs/specs.md`'s ordering clause is updated to whatever the run says, including if that is "nothing moved"
touches:    src/search.cpp, src/chesso.cpp, tests/test_search.cpp, adocs/specs.md
excludes:   the killer slot count, which stays at two; the ordering band constants, which CLAUDE.md and `src/search_params.hpp` "Not in the set, on purpose" keep out of the tuned set; history malus, gravity and butterfly indexing, which are S093; re-litigating S149's distinctness guard, which was measured at -11.02 +/- 10.53 Elo over 2522 games and is closed -- a scheme that happens to make the slots distinct is in scope only if that is a side effect of ageing and not the thing being proposed
decisions:  DEC-019
closes:
blocks:
paused_by:
author:
done:

## Where this comes from

S149 implemented CPW's *Killer Heuristic* replacement rule -- guard the shift so
the two slots always hold different moves -- and it measured **-11.02 +/- 10.53
Elo, H0 accepted over 2522 games**. The guard was reverted. The number is not in
dispute; what it *means* is, and this step is the part S149 could not answer.

**The hypothesis, and it is a hypothesis: nothing here has been measured.** The
unguarded shift does two things at once and only one of them is the bug F01
named. It duplicates the slots, yes. It also **discards whatever slot 1 was
holding**, every single time a quiet repeats at a ply -- which is 66 % of
stores. So the unguarded store is, incidentally, an aggressive ageing mechanism
for the second slot. The guard removes the duplication *and* the ageing
together, and since killers persist across every iteration of one `go`, a
guarded slot 1 can hold a move that refuted something eight iterations ago and
keep being tried at 800000 for the rest of the search.

If that reading is right, the 11 Elo is the cost of the staleness rather than
the benefit of the duplication, and the two are separable. If it is wrong, an
ageing scheme measures zero and that is recorded as zero -- which is itself
worth knowing, because it would say the second killer slot carries very little
either way on this search and S093 can stop treating it as load-bearing.

**Ordering.** The constraint S149 carried -- "this lands before S093 or is
folded into it deliberately, never after" -- is **void by history**: S093
rewrote the same block (history malus, gravity, butterfly indexing) and landed
2026-08-22, H1 accepted at +10.73 +/- 6.70 over 6412 games. So this step is
measured **on top of** S093's malus and gravity, and its verdict is read as
that and never as a statement about the killer slots against the pre-S093
table. Nothing is owed to the ordering; what is owed is that the reference
commit be after S093, which any `REF` default already is.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

The killer table remembers, per ply, the last two quiet moves that produced a
beta cutoff, and `score_move` in `src/evaluation.cpp` tries them right after
the captures. Chesso builds that table once per `go` -- `iterative_deepening_search`
in `src/chesso.cpp` declares `search_state_t state = {}` before the depth loop
-- and never clears it while the iterations run, so a killer found at depth 3
is still offered at depth 12 unless a later store at that ply displaced it.
S149 measured the wiki's "keep the two slots distinct" rule on this store and
it lost about 11 Elo; the text above reads that as a possible cost of
*staleness*, because the unguarded shift happens to throw the second slot away
on every repeat and the guard stopped that. This step tests the ageing reading
with a run. One small code change per candidate; nothing here is a pruning,
reduction or extension rule; no constant is added.

Read S149's file whole first -- pre-registration, instrumentation and revert
are the template -- and the fence comment in `tests/test_search.cpp`
`"a search fills the ordering tables"`.

### 2. The technique as published

**Definition.** Chess Programming Wiki, *Killer Heuristic*
(https://www.chessprogramming.org/Killer_Heuristic): "a dynamic, path-dependent
move ordering technique. It considers moves that caused a beta-cutoff in a
sibling node as killer moves and orders them high on the list." Storage: "When
a node fails high, a quiet move that caused a cutoff is stored in a table
indexed by ply, typically containing two or three moves per ply. The
replacement scheme ought to ensure that all the available slots contain
**different** moves." *Killer Move* (https://www.chessprogramming.org/Killer_Move):
"a quiet move which caused a beta-cutoff in a sibling Cut-node, or any other
earlier branch in the tree with the same ply distance to the root", tried
"direct after a possibly available hash move from the transposition table and
considering apparently winning captures". Neither page states an Elo figure, a
lifetime for the table or a clearing rule; the 2026-09-04 literature check
(`adocs/data/2026-09-04_plan_review_literature_check.md`) recorded "no reset
rule described".

**Ageing and locality in the record.** Three forms, none with a verified
engine result page:

- *Killers two plies up as an extra source.* The wiki: "Apart from the killer
  moves from the same depth, some programs use killers from two plies ago." A
  read, not an ageing rule; not a candidate here.
- *Per-node reset of the next level.* H.G. Muller, TalkChess thread 69744
  (https://talkchess.com/viewtopic.php?t=69744): "Killers are very local. Some
  engines even enforce that [...] by explicitly clearing the killer slots for
  the daughter level when they enter a node" -- "It means ply+1. This prevents
  inheriting killers from cousins or more distant relatives." The same thread
  carries the counter-evidence, forum claims and direction only (DEC-019):
  "Topple loses about 10 Elo if I implement this"; "RubiChess also loses Elo
  when clearing killers of ply+1. Clearing at ply+2 is more promising". Thread
  83466 (https://talkchess.com/viewtopic.php?t=83466) has the ply+2 variant for
  mate killers. **This is the "per-node reset of the killers two plies down"
  that `adocs/plan.md`'s 2026-09-03 section and the 2026-09-03 audit record as
  "absent and in no step [...] not worth a finding at this strength" and that
  DEC-138 lists among the omissions already recorded.** Not a candidate here;
  adding it needs a decision that reopens that record (section 10).
- *Per-iteration lifetime.* No page or paper describes clearing killers between
  iterations. The nearest record in this repository is for the *history* table:
  DEC-101 quotes Lynx's measurement that every alternative to keeping history
  across searches lost, and chesso's own run found keeping it across `go` worth
  nothing (-1.65 +/- 4.22 over 15398 games). That prior points down, not up,
  and it is a hypothesis for the run and nothing more.

**Which form chesso takes.** Candidate A, per-iteration clearing, as the
accepts names it first. Candidate B, as the accepts words it, is
behaviour-neutral on this code (section 3) and is proved by INV-6, not a match.

### 3. What chesso has today, and where the change plugs in

**The table.** `src/data_structures.hpp` `search_state_t` holds
`move_t killer_moves[2][MAX_PLY]` beside `quiet_history`, `counter_moves`,
`static_evals` and the PV rows; `MAX_PLY` is 128. `grep -rn killer_moves src/`
returns exactly four sites, two writes and two reads, and the step leaves it at
four or explains the fifth.

**The write.** `src/search.cpp` `negamax`, fail-high block, inside
`if (!is_capture)`: slot 1 takes slot 0, slot 0 takes `moves[i]`, then
`history_on_quiet_cutoff` and the `counter_moves` store. The comment above the
two lines carries S149's number. Candidate A does not touch it.

**The reads.** `src/evaluation.cpp` `score_move`, after the TT move, captures
and promotions: slot 0 first at `ORDER_KILLER_0` (900000), slot 1 at
`ORDER_KILLER_1` (800000), then `ORDER_COUNTER` (700000), then the signed
history value. Bands asserted in `tests/test_evaluation.cpp` `"bands are
strictly ordered"` and `"the declared history ceiling clears the band above it"`.

**The lifetime.** `src/chesso.cpp` `iterative_deepening_search` builds the
state
once per call and every `go` is one call, so the table is empty at depth 1 and
carried through every later iteration and every aspiration re-search.
`command_ucinewgame` resets the transposition table and the proven mate line
and has no killer to touch. Direct callers of `search()` -- every case in
`tests/test_search.cpp`, `search_fen` included -- see one iteration and are
unaffected by either candidate.

**Candidate A, the rule.** At the start of every iteration of the depth loop,
before that depth's aspiration window is set, every killer slot is zero.
Aspiration re-searches at the same depth are inside the iteration and keep what
the failed attempt stored: same depth, same age. History, countermoves and the
transposition table are untouched, so an iteration's first nodes are ordered by
TT move, captures, countermove and history until the first quiet cutoff at each
ply refills the slot.

```
// src/search.cpp, declared in src/search.hpp beside search():
void killers_clear(search_state_t* state)
{
  std::fill(&state->killer_moves[0][0],
            &state->killer_moves[0][0] + 2 * MAX_PLY, move_t{0});
}

// src/chesso.cpp, iterative_deepening_search, first statement of the
// `for (int current_depth = 1; ...)` body, beside `state.explored_nodes = 0`:
killers_clear(&state);
```

The name is the implementer's; the placement is not. Inside `search()` it
would fire on every aspiration re-search and on every direct caller -- a
different rule with a different tree. The call-site comment says the lifetime
became per-iteration, names S159 and, after the run, carries the number.

**Candidate B, "shift slot 1 down on a repeat", is neutral as worded.** On a
repeat (`moves[i] == killer_moves[0][ply]`) the shipped store leaves slot 0 and
slot 1 both equal to the move; the candidate leaves slot 0 equal to the move
and slot 1 zero. `score_move` compares slot 0 first, so a move equal to a
duplicated slot 1 has already returned `ORDER_KILLER_0`, and no legal move
equals zero: **the two table states rank every move identically**, and the four
sites above are the only readers. It is a no-op for the engine, provable in
minutes -- `python3 tools/search_bench.py ./build/src/chesso 9` and `12` on
candidate and reference print identical nodes and best moves. Its one
observable effect is the fence `REQUIRE(killers_1_duplicated > 0)` in
`"a search fills the ordering tables"` going red for a change that alters no
game. Do not spend a match on it and do not rewrite the fence for it; record in
the stamp that B is node-identical to HEAD with the two `search_bench` lines as
proof -- the accepts' "nothing moved" for this candidate. Section 10 asks
whether a substitute is wanted.

**Order of work.** (1) Read S149's file and `adocs/data/S149_sprt.sh`; confirm
`adocs/plan_done/S198_*` exists (DEC-143: no verdict before the workstation's
A/A). (2) Prove B neutral on a scratch branch; discard it. (3) Re-run S149's
instrumentation on HEAD with section 6's staleness counter, so the quantity the
hypothesis is about has a number before a game is played, as S165 counted
reachability first. (4) Implement A, its test, the gate; `search_bench` at 9
and 12 against the reference, deltas recorded. (5) Write and smoke-test
`adocs/data/S159_sprt.sh`, launch detached, arm the watcher, write the
`specs.md` paragraph for both outcomes before the first game. (6) On the
verdict: keep or revert per the clause, re-count the rates on what ships.

### 4. Constants and seeds

None. The clear is total and the boundary is the existing depth loop.
`src/search_params.hpp` is untouched; the slot count stays two (excludes) and
the ordering bands stay out of the tuned set (the header's own comment,
CLAUDE.md). The instrumentation's iteration stamp (section 6) lives in a
scratchpad copy and never in `src/`. A substitute second candidate with a
parameter -- an age in iterations, say -- would enter `CHESSO_SEARCH_PARAMS`
with a range in the header's three kinds and a DEC-105 default, and that is a
change to `accepts:` first.

### 5. Interactions and traps

- **Ordering moves what LMR reduces.** Killers prune nothing, but a move that
  loses its killer rank is searched later, and late moves are reduced
  (`lmr_reduction` in `src/search.cpp`); "late move reduction reduced the
  mating move at the root" is this project's recorded failure. The mate suite
  in the fast label (`tests/test_engine.cpp` `"engine: mate safety"`, S145's
  set) and `tests/test_search.cpp`'s mate cases must stay green, run and not
  assumed.
- **Fewer nodes is not the verdict.** S149's guard cut negamax nodes 3.34 %
  and lost 11 Elo; A will most likely *add* nodes at fixed depth.
- **History's lifetime is settled and stays.** Since DEC-101 `quiet_history`
  is per `go`, carried across iterations -- as killers are today. A makes the
  two lifetimes differ; that is intended and is what the run prices. Do not
  align history in the same change: one change at a time, and history is
  S093's, closed.
- **S093 has landed.** The "Ordering" paragraph above predates S093's
  completion on 2026-08-22 (`adocs/plan_done/S093_history_malus_and_ageing.md`);
  the constraint is moot and the verdict is taken against HEAD with S093 in
  it. S024's continuation history is pending and will be measured against
  *its* parent, so nothing here changes what S024's number means.
- **Plies are shared across subtrees.** A slot at ply 6 is written by every
  subtree reaching ply 6. Per-iteration clearing does nothing about that; the
  per-node reset would, and it is out of scope. A null here is not a verdict
  on locality.
- **The fence stays green under A.** `"a search fills the ordering tables"`
  runs one `search(8, ...)` from a fresh state; a red there means the clear
  landed somewhere other than the depth loop. Do not edit its assertions.
- **INV-4, fail-soft, TT bounds, the improving flag, time management:**
  untouched. The clear writes 1 KB once per iteration outside `negamax`; quote
  `search_bench`'s timings beside the node deltas anyway. The Debug self-play
  below will forfeit on time at 4+0.04; forfeits are not what it is for.

### 6. Tests

**Guard test with precondition**, `tests/test_search.cpp`, suite
`"search: move ordering state"`:

```
TEST_CASE_FIXTURE(search_fixture_t,
                  "killers_clear empties a table a search has filled")
{
  REQUIRE(load_FEN(TRICKY_POS, &game));
  // state as in "a search fills the ordering tables": own tt, never_stop
  const search_t first = search(4, &game, &state);
  REQUIRE(first.best_move != 0);
  size_t filled = 0;                        // count non-zero slots, both rows
  REQUIRE(filled > 0);                      // precondition: something to clear
  killers_clear(&state);
  size_t after = 0;                         // recount
  REQUIRE(after == 0);                      // the rule
  REQUIRE(state.quiet_history[...] != 0);   // one history cell and one
  REQUIRE(state.counter_moves[...] != 0);   //   countermove survive: only the
}                                           //   killers were cleared
```

Observe it red once by emptying `killers_clear`'s body, then restore. DEC-141's
mutant obligation does not apply -- no pruning, reduction or extension rule --
and this is the TESTS rule's "failure observed, not assumed". The placement in
the depth loop is not reachable from a ctest, since `iterative_deepening_search`
owns its `search_state_t` and exposes none of it; the placement is proved by
the instrumentation below (iteration-start slot count zero at every iteration
after the first) and by the `search_bench` deltas.

**Goldens.** None edited. A case in `tests/test_engine.cpp` or
`tests/test_mate_carry.cpp` that drives `iterative_deepening_search` and holds a
node count or a floor may move under A, since every `go depth >= 2` tree
changes. DEC-142: re-derive by the script its site names (before S192, the
case's comment or the `adocs/data/` script it cites), never re-read from the
failing run; name each in the stamp. A red mate case is a bug, not a golden.

**Instrumentation, and the number the hypothesis is about.** Re-create S149's
four counters on a scratchpad copy of the tree, never in the repository:
`killer_stores` and `killer_dup_store` at the store, `km_probe` and
`km_dup_live` in `negamax` after the `tt_eval` read. Add an iteration counter
to the copy's `search_state_t`, incremented by the depth loop, a
`killer_iter[2][MAX_PLY]` stamp written at every store, and at the probe point
`km_stale_live`: nodes where slot 0 is non-zero and its stamp is below the
current iteration. Driver exactly S149's -- the audit's 11 positions, `Hash 64`,
one process: startpos 13, kiwipete 13, lasker 18, promo-mess 12, 9bishops 14,
kpk 22, perpetual 16, mate-QR 15, underpromo 14, tactical 13, checkfest 13 --
so the before figures reproduce 66.0 % and 44.4 %. Report on HEAD and on A:
duplicate rate of stores, duplicated-pair rate of nodes, stale-slot rate of
nodes. Under A the stale rate is zero by construction; HEAD's is the size of
what A removes.

**INV-6.** `python3 tools/search_bench.py ./build/src/chesso 9` and `... 12`,
then the same on `.ref-builds/<parent sha>/build/src/chesso` (built by the
first `fastchess.sh` or `S159_sprt.sh` run). B: identical at both depths, best
moves included. A: expected to differ; quote the three deltas and best moves.

**Debug self-play (DEC-141).** The search is touched:

```
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build build-debug -j8 --target chesso
fastchess -engine cmd=./build-debug/src/chesso name=dbg-a \
          -engine cmd=./build-debug/src/chesso name=dbg-b \
          -openings file=books/UHO_Lichess_4852_v1.epd format=epd order=random \
          -each tc=4+0.04 option.Hash=16 option.Threads=1 \
          -rounds 4 -repeat -recover -concurrency 2 \
          -pgnout file=<scratch>/s159_debug.pgn > <scratch>/s159_debug.out 2>&1
grep -c 'Assertion' <scratch>/s159_debug.out      # must print 0
grep -ci 'disconnect' <scratch>/s159_debug.out    # must print 0
```

The assertion text comes from the engine's stderr, so the run's own stdout and
stderr are captured; fastchess's `-log` file is WARN-only by default and comes
back empty (`fastchess.sh`'s comment on exactly this). The stamp quotes both
counts.

### 7. Measurement

**Lane.** SPRT for A; INV-6 for B. Two verdicts at most, one change at a time,
each against the commit before it.

**Bounds.** `elo0=-5 elo1=5 alpha=0.05 beta=0.05`, nElo (`model=normalized`),
as S149 chose for this block under DEC-063 and recorded as DEC-099. The effect
is two-sided: the mechanism argues that removing stale slots helps; DEC-101's
history run and the forum figures of section 2 argue that discarding ordering
memory costs. A pair that cannot contain the truth random-walks (DEC-063,
6 h 36 m for nothing). DEC-143 pricing from `adocs/testing_strategy.md`
section 1.1 at 2337 games an hour:

| pair | worst case, truth at midpoint | truth on a bound | what H1 establishes |
|---|---|---|---|
| `{-5, 5}` | 10465 games, 4.5 h | 6398 games, 2.7 h | more likely a gain than a 5 nElo loss; magnitude not established |
| `{0, 5}` | 41861 games, 17.9 h | 25591 games, 10.9 h | a gain of 5 nElo, about 3.9 logistic at this draw rate |

`{0, 5}` only if the owner wants the run to *claim a gain* (section 10).
S149's -11.02 +/- 10.53 over 2522 games was an early-stopped estimate and is
not a prior for the size of anything here.

**Abort rule and ceiling.** `rounds=10500`, so the games cap (21000) is twice
the worst case. Watcher hard ceiling 36000 s, above twice 4.5 h, wall-clock
while awake only (WATCHERS). Either cap reached is a no-verdict, read as the
third outcome; no re-run at other bounds without a `decisions.md` entry.

**The three readings, in the script before launch:**

- **H1 accepted.** Not a regression of 5 nElo or more, direction favourable.
  Keep and ship; the point estimate is biased upward by the early stop
  (DEC-063) and is not the effect size. `specs.md`: killers cleared per
  iteration since S159, with the number.
- **H0 accepted.** A per-iteration lifetime costs 5 nElo or more. Revert the
  call and the helper; the table stays per `go`. The ageing reading of S149's
  11 Elo is refuted in its simplest form and recorded as such.
- **No verdict.** Recorded as zero. Recommendation, to confirm in the
  pre-registration: revert. Unlike S149's guard, the clear implements no
  published rule and removes no unintended state; a measured zero for an extra
  action is a reason not to ship it. `specs.md`: "measured zero, per-`go`
  lifetime kept".

**Script.** `adocs/data/S159_sprt.sh`, copied from `adocs/data/S149_sprt.sh`:
step line, candidate name `candidate-s159-killer-iter-clear`, `ref_sha` = the
parent commit's short sha (the script builds the reference worktree),
`rounds=10500`, the DEC-143 header -- pair, worst-case games and hours, abort
rule, the three readings -- and `SPRT-RUN-DONE s159` / `SPRT-RUN-FAILED` on
every exit path (DEC-061). Smoke-test both paths as S149 did: a 4-game run,
then an occupied output directory. Then

```
nohup adocs/data/S159_sprt.sh > .tuning/sprt_s159.log 2>&1 &
```

and arm the AGENTS.md poll-loop watcher on that log with the run's pid and the
36000 s ceiling. Before launch: S198's A/A is in `adocs/plan_done/`;
`git status` shows only the intended diff; nothing else runs (MACHINE).

**Record.** Log to `adocs/data/S159_sprt.log`; verdict block, pentanomial and
census in the stamp; PGN left in `/tmp`; `tools/forfeit_report.py` if forfeits
are non-zero.

### 8. Completion checklist

- Gate, both builds, `-j` at the workstation's core count:
  `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`
- Debug self-play run, both greps zero; Debug `ctest -L fast` under
  `build-debug` if `tools/gate_extra.sh` (S197) is not yet there.
- `python3 tools/plan_prose_check.py --touches | tail -1` unchanged.
- Commit: imperative subject; body names S159, the verdict, bounds and
  reference sha; last line `Bench: <total>` from `./build/src/chesso bench`
  (DEC-140, binding since S189's completing commit, which precedes this step in
  the Open list). A revert commit after H0 ends `No functional change`.
- `adocs/specs.md`, the search-machinery row that today reads "**The
  hypothesis for why, untested and recorded as a hypothesis:** [...] That
  points at a different change and it is S159, not this one": replace with the
  verdict -- the table's lifetime, Elo and nElo with error bars, games,
  bounds, reference sha, the re-counted duplicate and stale rates. Keep S149's
  numbers in the row.
- `MANUAL.md`: no option added; its only "killer" is the `killer` position
  alias row, unrelated -- checked, no change. `DEV_MANUAL.md`: no command
  changed -- checked, no change. `README.md` untouched.
- Stamp: B node-identical to HEAD with the two `search_bench` lines; HEAD's
  stale and duplicate rates; A's `search_bench` deltas; the verdict block; keep
  or revert and why; the self-play counts; goldens re-derived, if any.
  `plan.md` and `status.md` through the coordinator.

### 9. Sources read

- https://www.chessprogramming.org/Killer_Heuristic -- definition, slots,
  replacement wording, "killers from two plies ago"; no Elo, no reset rule.
- https://www.chessprogramming.org/Killer_Move -- definition, ordering
  position; no lifetime, no figures.
- https://talkchess.com/viewtopic.php?t=69744 -- the ply+1 per-node clear and
  its disputed sign (Topple about -10, RubiChess negative at ply+1, "ply+2 more
  promising"); forum claims, direction only.
- https://talkchess.com/forum3/viewtopic.php?t=61399 (the rule's "cousin
  nodes" rationale), https://talkchess.com/viewtopic.php?t=83466 (the ply+2
  mate-killer clear), https://talkchess.com/forum3/viewtopic.php?t=71454 (a
  positive report of "killers[ply-2]" as an extra source) -- no figures in any.
- `adocs/plan_done/S149_killer_slot_dedupe.md`, `adocs/data/S149_sprt.sh`,
  `adocs/data/S149_sprt.log` -- the measurement reinterpreted here; the script
  template. `adocs/audit/2026-08-21_adversarial.md` F01 -- counters and
  driver positions. `adocs/audit/2026-09-03_adversarial.md`, `adocs/plan.md`
  2026-09-03 section -- the two-plies-down reset recorded as no step.
- `adocs/decisions.md` DEC-019, DEC-063, DEC-101, DEC-138, DEC-140 to DEC-143,
  DEC-145; `adocs/testing_strategy.md` 1.1; the killer rows of
  `adocs/data/2026-09-04_plan_review_literature_check.md`.
- `src/search.cpp`, `src/evaluation.cpp`, `src/chesso.cpp`,
  `src/data_structures.hpp`, `src/search_params.hpp`, `tests/test_search.cpp`,
  `tests/test_evaluation.cpp`, `tests/test_engine.cpp`, `fastchess.sh`,
  `DEV_MANUAL.md` "Test", "Measure", "Play games", "Which bounds", "Mate
  safety", at `0a4f88f`.

Every URL was fetched on 2026-09-05; no figure is marked unverified.

### 10. Questions deferred to the owner

1. **Candidate B is behaviour-neutral as worded** (section 3), so "two
   candidates, each decided by SPRT" cannot be met literally. Either S159 runs
   one SPRT and records B's neutrality as its "nothing moved", or a second
   candidate is substituted. Two substitutes fit the hypothesis and each needs
   a decision: the per-node reset two plies down, which `plan.md` and DEC-138
   record as deliberately no-step; or A *combined with* S149's guard, the
   direct test of the claim that staleness and duplication are separable,
   which `excludes:` reads as re-litigating S149 unless the owner rules
   otherwise.
2. **Bounds.** `{-5, 5}` as S149 (recommended, 4.5 h worst case) or `{0, 5}`
   to claim a gain (17.9 h worst case).
3. **On a null verdict, revert** (recommended) or keep with the reason stated.
4. The header's `accepts:` cites `src/chesso.cpp` by a line number (674) that
   has moved; S187 converts pending citations to symbols and this file is in
   its scope.
