id:         S024
goal:       history indexed by the move played n plies ago and the current move
accepts:    two SPRT verdicts, one per table: the one-ply table first, the two-ply follow-up second, each against the commit before it; every src/ citation in this file names a symbol or a test title rather than a line range
            (Folded in from the retired S063 by DEC-086. Offsets beyond two
            plies are worth 1-3 Elo each at 3400 and are not part of this step.
            The sentinel plies are the known crash: (ss-1) and (ss-2) must be
            valid at ply 0 and ply 1, and a null move must still install a
            continuation pointer or the child indexes garbage.)
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp
excludes:
decisions:  DEC-019, DEC-063, DEC-084, DEC-086, DEC-087, DEC-109, DEC-111, DEC-141, DEC-143, DEC-189, DEC-190
closes:
blocks:
paused_by:
author:     a Sonnet 5 subagent briefed by the coordinator (DEC-185, DEC-188); started 2026-09-12 07:15
done:       2026-09-12 13:05, verdict 1 H0 and reverted, verdict 2 not attempted (DEC-194). The one-ply continuation history -- `cont_hist[12][64][12][64]` on `search_state_t`, one index helper, S093's bonus/malus and `QuietHistoryMax` reused unfitted, guarded on a previous move existing -- landed as `cace216` (`Bench: 22363740`) with red-first tests, two mutants killed, gate and `gate_extra.sh` green, Debug self-play 8 games 0 `Assertion`. **Gainer SPRT `{0, 5}` against `b5c357a` on `noob_3moves.epd`: H0, LLR -2.96, nElo -5.48 +/- 7.20, Elo -4.42 +/- 5.81, 8954 games in 4 h 10 m 11 s, 0 forfeits either side.** The pre-registered census (`adocs/data/S024_census_run.py`, 400 positions, depth 10, 112638096 nodes): the table reached in 97.6 % of quiet cutoffs and 96.2 % of quiet reads, non-zero in 27.1 % of consulted reads -- exercised, so the H0 is about the technique as built. **Reverted in `ecdfadb`, `Bench: 27322394` again**; the technique returns as S222 with its own fitted scale behind S127. Documents: `specs.md` records the H0 and the revert, `DEV_MANUAL.md`'s ledger keeps the 22363740 line annotated, `MANUAL.md` unaffected, `README.md` human-owned, untouched. Two Sonnet 5 subagents built and read it (DEC-185, DEC-188); the coordinator launched, reverted and stamped

## Note

Chesso has the countermove *heuristic* and no continuation history at any
depth. `move_t counter_moves[12][64]` (`src/data_structures.hpp`
`counter_moves`, written on beta cutoff at `src/search.cpp` `negamax`, read as
the fixed `ORDER_COUNTER` band at `src/evaluation.cpp` `score_move`) remembers
a single refutation move per (previous piece, target) and gives it a flat bonus
— the countermove heuristic, Uiterwijk 1992.

Continuation history is a different device: a score table indexed by
(previous move's piece, target) × (current move's piece, target), accumulating
graded bonuses and maluses for every quiet move, added into the ordering score
alongside plain history. One ply back is countermove history, two is follow-up
history; engines carry the heuristic and both tables at once, and the one-ply
table is consistently reported the stronger half of the pair.

Build the one-ply table first and take its verdict, then the two-ply table and
take its own. 2026-08-13_plan_review-F02 is why this note reads this way: the
previous version called the countermove table one-ply continuation history,
which would have scoped this step to the weaker half alone and recorded its
verdict against the wrong technique.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

CPW History Heuristic, Continuation History section: "An n-ply Continuation
History is the history score indexed by the move played n-ply ago and the
current move"; "1-ply and 2-ply continuation histories are most popular and
correspond to Counter Moves History and Follow Up History respectively"; many
programs, "notably Stockfish, also makes use of 3, 4, 5, and 6-ply". The
history-times-countermove combination was implemented in Stockfish by Stefan
Geschwentner and "dubbed Counter Moves History", and "Stockfish's History and
Countermove arrays are piece type and to-square based, not butterfly based" —
the published index is [prev_piece][prev_to] x [piece][to], verified.

**Table vs history, and they coexist.** The countermove *heuristic* (Uiterwijk
1992, CPW Countermove Heuristic) stores one refutation move per (prev piece,
prev to) and gives it a flat band — what chesso has. Countermove *history*
grades every (prev move, current move) pair. Engines carry both: Lynx tried
removing the table while holding continuation history and measured **-6.84
+/- 5.26, H0** (PR #1563, closed) — so `counter_moves` and its band stay.

Update: the same gravity/bonus/malus mechanism as S093 section 1, applied per
table to that table's own entry. Two negative controls: updating each entry
from the *sum* of continuation scores failed at Lynx (PR #2463, -2.49); and
CMH **without a malus** measured **-12.90 STC / -17.38 LTC, H0 both** inside
Lynx PR #645 — the table arrives with the malus or it loses.

Combination into ordering: summed. A quiet's score is main history plus each
active continuation entry, equal weights. Stockfish simplified its movepick
weights to "all continuation histories to 1 and doubles the weight for the
main history" (commit 55905e5); every Lynx re-weight attempt closed (#2473,
#2475 cont x2 / x0.5; #2482, #2483 main x0.5). Deeper plies, where they exist,
enter reduced (Stockfish 3-ply at "1/4 multiplier", commit 38e830a).

Traced records:

- **The plan's +44.7/+34.0 is one patch, Weiss PR #477** (commit 1405ec8,
  2021-07-03), "Counter move history": **ELO 44.68 +- 14.02** at 8+0.08
  Hash=32 and **33.95 +- 10.83** at 40+0.4 Hash=128 — STC and LTC of the
  one-ply table alone, the largest per-patch ordering gain surveyed.
- Weiss PR #594 (commit 514cbe8, 2022-09-01), follow-up history two plies
  back: **+6.13 +- 5.03 STC, +14.45 +- 8.01 LTC**, alongside a rewrite to
  "use search stack to hold points to continuation history to avoid a lot
  of ifs when looking it up".
- Lynx PR #645 (merged 2024-07-19), CMH with malus: final runs **+2.16 STC /
  +9.12 LTC**, cancelled short of a bound; an earlier iteration's LTC
  **+2.75 +- 2.17, H1**. Same technique as Weiss's +44.68, twenty times
  smaller — DEC-019 in a single pair of records.
- Lynx FMH failed twice before passing: PR #862 (-2.63/-6.71), PR #1148
  (+0.03, H0), then **PR #2459 (merged 2026-03-12): +10.90 +- 4.81, H1** —
  stated difference from the failures: "not having one table per ply", one
  shared physical table read at both offsets.
- Removal control: Lynx PR #2295 (2025-12), remove continuation history:
  **-27.83 STC / -28.90 LTC**, kept.
- Beyond two plies: Stash extended to 4 for +13.09 +- 6.75 STC (commit
  660df34); Lynx 4-ply closed repeatedly at ~3300 (#2460, #2474, #2471,
  #2472, #2539, #2540); Stockfish's 5th is simplification-sized churn
  (commits 904a016, 3d18ad7, f6b0d53). Out of scope per the folded S063 note.

Stockfish put its table on huge pages for +1.76 % nps (commit 5062aee): at
engine scale the table is TLB-visible, not free.

### 2. Shape for chesso

Symbols, per the accepts. `search_state_t` (src/data_structures.hpp) carries
`killer_moves`, `history_moves` (S093 reshapes it to butterfly
`quiet_history`) and `counter_moves`, read in `score_move`
(src/evaluation.cpp) as the ORDER_COUNTER band and written in `negamax`'s
fail-high block (src/search.cpp). No continuation history at any depth.

Reaching previous moves: `negamax` receives `prev_move` as a parameter,
passes `moves[i]` at every recursion, `0` for the null-move child, and the
root call in `search` passes 0. The ply-2 move is reachable nowhere today.
Add `move_t moves_played[MAX_PLY]` to `search_state_t` beside the per-ply
arrays already there (killers, pv rows; S108 adds `static_evals` the same
way): written with `moves[i]` immediately before each child call, written
with 0 before the null-move child, read as `moves_played[ply - 1]` /
`[ply - 2]` under `ply >= 1` / `ply >= 2` guards. `score_move` already takes
`state` and `ply`, so the two-ply lookup needs no signature change; the
`prev_move` parameter and `moves_played[ply - 1]` then say the same thing —
keep one source of truth. Weiss's per-ply pointer stack (PR #594) is the
mature form once offsets multiply; the array is the smaller first step.

The table: `int16_t cont_hist[12][64][12][64]` — 12 x 64 x 12 x 64 = 589824
entries, **1.125 MiB at int16_t** (2.25 MiB as two per-offset tables; 4.5 MiB
at the current history's 4-byte int — int16_t it is, S093's type). Verdict 2
decides sharing: the shared table is the only A/B'd shape (Lynx #2459 passed
where two per-offset attempts failed) — share first. It lives in the struct
S093 hoists beside `tt`, so persistence, the descendant rule and the
`command_ucinewgame` clear come free from S093's plumbing; 1.125 MiB is also
too big for the stack-allocated per-`go` `search_state_t` in chesso.cpp.

Where the sum enters: `score_move`'s final quiet return — today the raw
history entry — becomes butterfly + cmh (+ fmh), each term guarded, summed in
`int` (`scores[]` in `negamax` is int). Bands above are untouched.

Null-move boundary, what the record says: Weiss first skipped continuation
history where the boundary move was null or the ply too small, then removed
the guards — PR #593 "Allow continuation history for null moves. Also moves
at root/ply 1": STC stopped ~0 at 70k games, LTC passed **simplification**
bounds and failed gainer. The two shapes are equal inside measurement; take
the guard (skip when the indexed move is 0) — it is what the counter band in
`score_move` already does, and it needs no dedicated slot. Guard each offset
independently: after a null at ply-1, the ply-2 move can still be real.

### 3. Implementation sketch

Two verdicts per the accepts (plan.md prices S024 at two), each against the
commit before it.

**Verdict 1, the one-ply table.**
1. `moves_played`: the two writes (real move; null 0) and the ply guards.
2. Update in the quiet fail-high block: for the cutoff move (+bonus) and each
   S093 `quiets_tried` entry (-malus), apply the shared
   `history_gravity_update` to the butterfly cell **and** to
   `cont_hist[pc(prev)][to(prev)][pc(m)][to(m)]` when `ply >= 1` and the prev
   move is non-null. One eligibility predicate, one tried list — S093 built
   both; after S107 eligibility is `!is_capture` alone.
3. Read in `score_move`; band test re-pinned for the two-term sum.
4. Unit tests red first, on the table not through games: from a zeroed table,
   a driven fail-high leaves the cutoff cell strictly positive and each
   tried-quiet cell strictly negative; with prev null or ply 0 no cell moves;
   asymptote-driven entries keep `|sum| <= 2 * HISTORY_MAX` and the sum
   clears ORDER_COUNTER by 100 on the `score_move` result.
5. Fast suite green, SPRT.

**Verdict 2, the two-ply follow-up.** Second read/update through
`moves_played[ply - 2]` into the same physical table; band test re-pinned for
three terms; own SPRT against verdict 1's commit. Lynx's first two FMH
attempts lost Elo — a zero or worse at 2559 is a live outcome, recorded as
it comes, and keeping or reverting the second offset states its reason
(S005/S006/S015 precedent).

### 4. Constants and seeds

Verdict 1 introduces no new constant: the update reuses S093's
`HISTORY_BONUS_*`/`HISTORY_MALUS_*` and `HISTORY_MAX` at full weight for
offsets 1-2, which is the published practice (reduced weights start at 3-ply:
Stockfish 38e830a). Per-table bonus factors exist in the record (Stockfish
a3bb7e6, tuned per-ply factors) — if exposed, `CONT_BONUS_*` seeded equal to
the main constants, **seed — must be fitted/SPSA'd here** (DEC-084).

Sum weights: implicit {1, 1, 1}, no parameters added. Record: Stockfish
runs main x2 + cont x1 (55905e5); all four Lynx re-weights closed. S127
material if ever.

Initialisation: zero. Stockfish initialises slightly negative (commits
e90341f, 7d44b43) — practice noted, constant untaken (engine-source value,
DEC-084).

### 5. Pitfalls

- **The band arithmetic must be redone for the sum — S093 pins one entry,
  this step pins a sum of up to three.** Quiet range becomes [-2M, +2M] then
  [-3M, +3M], M = HISTORY_MAX. Clearance `3M + 100 <= ORDER_COUNTER (700000)`
  holds to M <= 233300 — safe at S093's 2^13/2^14 seeds. The bottom edge is
  the live one: nothing sits below quiets today, S025 (reserve) would put
  losing captures there — pin both edges in the test. And **the sum overflows
  int16 at M = 2^14** (3M = 49152 > 32767): entries int16_t, sum in int,
  asserted by the band test's asymptote case.
- **The null-move write is the new staleness bug.** Today `prev_move` is a
  parameter and cannot go stale; `moves_played[ply]` can. Miss the 0 write
  before the null-move child and it indexes the move of a previously searched
  sibling — wrong cell, silent, no crash. This is the folded S063 sentinel
  warning translated to the array shape; Lynx's stale ply-stack entry
  (PR #1182, non-deterministic bench until cleared) is the bug class on
  record. Unit-test the null path explicitly.
- **size_t underflow at the top plies.** `ply` is size_t; `ply - 2` at ply 1
  wraps enormous. Guard `ply >= 2` before any index arithmetic — S108's file
  carries the same warning for `static_evals`. At ply 0 both offsets are
  absent and root quiets order by main history alone; the root TT move from
  the previous iteration still leads.
- **Prev-move aliasing.** A capture as the indexed move is well-defined under
  (piece, to). A promotion is not: MOVE_PIECE(prev) is the pawn while the
  piece standing on prev_to is the promoted piece. `counter_moves` already
  keys on MOVE_PIECE — keep that convention at write and read through one
  index helper so the sites cannot drift. Weiss's cmh/fmh double-count bug
  (PR #544, moves that did not carry the mover) cannot arise here: chesso's
  `move_t` encodes MOVE_PIECE.
- **Do not update from the sum.** Each entry updates from its own value
  through the shared helper (Lynx PR #2463, -2.49, closed).
- **Clearing cost is a non-issue, stated so nobody argues:** a memset of
  1.125 MiB is well under a millisecond; the table clears with S093's struct
  on `ucinewgame` and on the non-descendant rule. The hazard is forgetting
  the clear, not paying for it.
- **Adjacent refinements that are not this step:** in-check and capture-split
  continuation tables (Weiss #628 +2.95 LTC, #626 +2.15/+2.52), quiescence
  reads (Stockfish uses (ss-1)/(ss-2) there, commit 057046c — chesso's
  quiescence orders by capture_score alone and stays untouched).

### 6. Measurement

Two SPRTs at the S105 regime — 8+0.08, Hash=16, UHO book — gainer bounds
`elo0=0 elo1=5` (DEC-063). The seeds straddle those bounds from far above and
from just above: verdict 1's published range runs from Weiss +44.68/+33.95
(2021, ~3100-3200 band) down to Lynx +2.16/+9.12 (2024, ~2900) — the widest
spread on this plan, and DEC-019 is why the bounds sit at the low end.
Verdict 2: Weiss +6.13/+14.45, Lynx +10.90 after two failures. Order inside
the step: fast suite green, table unit tests observed red first, band test
re-pinned per verdict, then each SPRT. Node counts differ by construction —
INV-6 takes the SPRT path both times, and a zero is recorded as zero. The nps
cost of two extra dependent loads per scored quiet is measured and recorded
beside verdict 1.

### 7. Interactions

- **S093, hard dependency, lands first**: supplies `history_gravity_update`,
  `quiets_tried`, `HISTORY_MAX`, the butterfly table and the persistent
  struct this table joins. If S093's persistence verdict reverts, cont_hist
  lands per-`go` in `search_state_t` instead — location changes, shape does
  not.
- **S107** (before S093): one eligibility predicate, `!is_capture`; this
  table is born check-inclusive.
- **S108** (immediately before): adds the per-ply `static_evals` array;
  `moves_played` follows the identical pattern and guard style.
- **S109** (immediately after): history pruning consumes the **combined**
  score — the threshold's meaning changes from one table's entry to a sum
  with triple the range, and S109 fits its constants against the post-S024
  shape, which the plan order guarantees. Keep one read path (score_move or
  a probe helper) so S109 grows no second indexing. Stockfish's cont-hist
  pruning is on record as strongly TC-sensitive (commit d37de3c) — S109's
  problem, flagged here.
- **S098**: the reduction's history input is this same sum (Stockfish scales
  by the "sum of first continuation history and main history (similar to
  movepicker)", commit 37c2b56); post-LMR continuation updates are S098
  material (Stockfish 389e607 at half bonus, Weiss #662 +2.61/+8.92, Stash
  #104 +2.20).
- **S127**: sum weights and any per-table bonus split are SPSA candidates
  with the records above.
- **S111 (reserve)**: correction history keyed on continuation — Weiss PRs
  #735-#744 measured +1.7 to +4.6 each; stays reserve per DEC-087.

### 8. References

- https://www.chessprogramming.org/History_Heuristic — Continuation History section: n-ply definition, 1/2-ply = CMH/FMH, Stockfish 3-6 ply, Geschwentner attribution, piece-type/to-square indexing.
- https://www.chessprogramming.org/Countermove_Heuristic — the table: Uiterwijk 1992, one refutation move per [from][to] or [piece][to], flat ordering bonus.
- https://github.com/TerjeKir/weiss/commit/1405ec8 — PR #477 counter move history: +44.68 STC / +33.95 LTC. The plan's +44.7/+34.0, one patch.
- https://github.com/TerjeKir/weiss/commit/514cbe8 — PR #594 follow-up history: +6.13 STC / +14.45 LTC; search-stack pointer rewrite.
- https://github.com/TerjeKir/weiss/commit/d4cb386 — PR #593 allow cont-hist for null moves and root/ply 1: ~0 STC, LTC simplification pass.
- https://github.com/TerjeKir/weiss/commit/7d410aa — PR #544 cmh/fmh double-count bug, fixed by encoding the mover in the move.
- https://github.com/lynx-chess/Lynx/pull/645 — CMH: +2.16 STC / +9.12 LTC final; no-malus ablation -12.90 / -17.38 H0.
- https://github.com/lynx-chess/Lynx/pull/862 and /pull/1148 — FMH failures: -2.63/-6.71; +0.03 H0.
- https://github.com/lynx-chess/Lynx/pull/2459 — FMH pass: +10.90 +- 4.81; "not having one table per ply".
- https://github.com/lynx-chess/Lynx/pull/2295 — removal test: -27.83 STC / -28.90 LTC.
- https://github.com/lynx-chess/Lynx/pull/1563 — remove countermove table under cont-hist: -6.84, closed; the two coexist.
- https://github.com/lynx-chess/Lynx/pull/2463 — update-from-sum: -2.49, closed. Re-weights #2473/#2475/#2482/#2483 and 4-ply #2460/#2474/#2471/#2472/#2539/#2540: titles and closed state read from the PR list.
- https://github.com/lynx-chess/Lynx/pull/1182 — stale ply-stack entry, non-deterministic bench: the staleness bug class.
- Stockfish commit messages, read as prose via the GitHub API, no source opened: 55905e5 movepick weights; 38e830a 3-ply at 1/4; 904a016 / 3d18ad7 / f6b0d53 5th-history churn; bb5589b in-check ss-4/ss-6 writes; 057046c qsearch (ss-1)/(ss-2) only; a3bb7e6 per-ply bonus factors; 389e607 post-LMR updates; d37de3c TC-sensitive cont-hist pruning; c44c62e futility threshold from cont-hist; 37c2b56 LMR stat sum; e90341f / 7d44b43 init practice; 5062aee huge pages +1.76 %; 8fadbcf Elo-info commit — its three fishtest links sit behind a bot check and were unreachable 2026-08-19.
- Stash (mhouppin/stash-bot) commit messages: 829b256 countermove history; ca25c16 opponent's move plus our previous move; 660df34 4-ply +13.09 STC; 93a6d5f average-scaled updates +4.29 STC; 2138db2 post-LMR updates +2.20.
## The MacBook attempt, discarded 2026-08-30

**Verdict 1 was built and tested here and never measured to a verdict, so it
was never retained.** The owner's decision of 2026-08-30 (DEC-111): the work is
discarded from `achesso` and redone on the Linux workstation, where a match can
run to a bound. Nothing was found wrong with the code. The machine could not
carry the run.

**The code and its tests are not lost.** Branch `s024_mac_attempt` holds
`e424032` and `8610e08` with the clang-format pin move on top, plus 14 MB of
run evidence under `adocs/data/S024_mac_attempt/` that was gitignored and would
otherwise have died with the machine. Its README states what each file is, with
digests. Cherry-pick `e424032`'s `src/` and `tests/` from that branch if the
implementation still looks right, or write it again -- either way the verdict
is taken from zero games. The branch is deleted once S024 lands.

### What was built, for the reader who reimplements it

- `src/data_structures.hpp` -- `continuation_history_t`, an
  `int16_t[12][64][12][64]`, 1.125 MiB, reached only through a single
  `continuation_entry()` helper so read and write cannot disagree on index
  order. Heap-allocated behind a `unique_ptr`: the search runs on a
  `std::thread` whose stack is 512 KB on macOS and the state is a stack object.
- `src/search.cpp` -- the same gravity/bonus/malus update as S093, applied to
  the table's own entry, guarded on `previous_move != 0`.
- `src/evaluation.cpp` `score_move` -- the continuation term summed into the
  quiet score at equal weight with main history, which is the published
  combination.

### The tests, and the mutation each one was observed red under

Reusable regardless of how the feature is written the second time. Red-first on
a new feature cannot mean "the test fails to compile", so each case was
verified by mutating the shipped code and watching that case fail:

| mutation | what went red |
|---|---|
| drop the `previous_move == 0` guard | `written == 3` against 0, "no move to reply to leaves the continuation table untouched" |
| write the cell keyed on the cutoff move instead of the previous move | cutoff cell 0, and the control cell read 64 |
| `score_move` stops adding the continuation term | `42` against `49`; and the band case `32767` against `65534` |
| swap the index order inside `continuation_entry()` | **nothing, correctly** -- read and write both go through the helper, so swapping it is a symmetric relabel. It did expose a `const` overload that nothing could call, since `unique_ptr::operator*` returns a non-const reference through a const struct; the overload was deleted |

The band case is re-pinned for the sum: both tables are driven to
`QuietHistoryMax`'s declared maximum at once, so the quiet band is `[-2M, +2M]`
and every clearance is measured against `2 * declared_max`. That assertion is
also what catches a sum accumulated in `int16_t`: two entries at 32767 wrap
negative there, and the ceiling check would otherwise pass for the wrong
reason.

### Measured before any match

Node counts move, so INV-6's node-count discharge is not available and an SPRT
is the only thing that can decide this step. Best move unchanged at all three
positions (`c3d5` / `e2a6` / `d7c8q`):

| depth | reference 25998fe | candidate |
|---|---|---|
| 9 | 121512 / 800769 / 62907 | 122266 / 794014 / 63484 |
| 13 | 944905 / 5228126 / 533227 | 875013 / 5210372 / 701417 |

Throughput, two interleaved passes at depth 11 on kiwipete: **9655 / 9508 knps
reference against 9518 / 9558 candidate** -- inside the noise this machine
resolves, so two extra dependent loads per scored quiet and a 1.125 MiB table
are not visibly paid for at bench scale.

### What the three runs cost and what they returned

All three at `elo0=0 elo1=5`, all three against `25998fe`, none reaching a
bound.

| run | games | wall | outcome |
|---|---|---|---|
| 1 | 2582 | 1 h 28 m of play | aborted: started on battery, hibernated at 1 %, woke 42 hours later still running |
| 2 | 6054 | 03:54:58 | interrupted, machine went down |
| 3, a resume of 2 | 7988 | 05:12:29 | interrupted, machine went down |

Run 1 is not pooled with the others: it spans a power transition and its two
halves do not look like the same experiment -- 1132 pairs at `-5.06 +/- 11.17`
before the sleep against 137 at `+35.63 +/- 30.99` after, about 40 Elo apart at
roughly 2.4 sigma, with mean game length moving 100.0 plies to 92.9. Its 8 time
forfeits all sit in rounds 1133 to 1137, the sleep boundary, and dropping all
five pairs moves the match by 0.27 Elo -- a footnote, not the cause.

Runs 2 and 3 are one tournament: `fastchess -config file=config.json` continues
an interrupted match with its statistics intact, which is what made run 3 cost
nothing to start. Pooled:

```
Elo: 2.87 +/- 4.37, nElo: 3.78 +/- 5.75        Games: 14038
Ptnml(0-2): [598, 1553, 2651, 1569, 648]       LLR: 0.74 (25.2%) (-2.94, 2.94)
```

**That is evidence and not a verdict**, and the next reader is the one it can
mislead. The interval contains both 0 and 5, so neither pre-registered reading
applies. **Do not use 2.87 as a prior that shortens the real run**: bounds are
chosen before the data or the error guarantee is gone.

### The lesson that transfers, and it is about bounds

At the observed effect size `elo0=0 elo1=5` wanted roughly 56000 games, about
21 hours at the 2700 games/h this machine did. The MacBook survived four to
five hours of a full-core match, twice. The bound pair and the machine were
incompatible, and no amount of resuming fixes that -- which is DEC-063 restated:
the cost of a verdict is set by the hypothesis pair and not only by the
hardware. Choose the pair against the machine that will run it, before the
first game.

### What is left

1. **Verdict 1's SPRT**, on the workstation, from zero games.
   `adocs/data/S024_sprt.sh` is the runner and pins the reference; its header
   carries the three pre-registered readings. Re-point it at whatever commit
   the reimplementation sits on.
2. **Verdict 2, the two-ply follow-up**, only after verdict 1 has a verdict and
   a commit. Adds `moves_played[MAX_PLY]` to `search_state_t` written before
   every child call including 0 before the null-move child, read at
   `[ply - 2]` under `ply >= 2`; the `prev_move` parameter's readers then move
   to `moves_played[ply - 1]` so there is one source of truth; same physical
   table; the band case re-pinned for three terms; its own SPRT against verdict
   1's commit.
3. **Then the step completes by hand** -- moltke v1 has no `--step` (DEC-109):
   write the `done:` stamp, move this file to `plan_done/`, move S024's entry
   out of `plan.md`'s Open list into `Done recently` and drop the oldest of the
   five, rewrite `status.md`, commit.

## Evidence taken, 2026-09-12

### What was built, verdict 1 only

One-ply continuation history (countermove history), keyed on the previous
move's (piece, to) and this move's (piece, to), summed into the quiet
ordering score alongside plain `quiet_history`.

**No `moves_played[MAX_PLY]` array.** The "Implementation sketch" lists it
under "Verdict 1", but its own text motivates it entirely by the *ply-2* read
verdict 2 needs -- "the ply-2 move is reachable nowhere today" -- and the
"What is left" section (from the discarded MacBook attempt, which built
verdict 1 alone) confirms this reading: it lists `moves_played` only under
"2. Verdict 2", guarded on `previous_move != 0` for verdict 1. `negamax_at`
already carries a `prev_move` parameter that is a true one source of truth
for the one-ply case -- the root call in `search()` passes `0` and the
null-move child at `src/search.cpp`'s null-move block passes the literal `0`
-- so verdict 1 reads and writes through that parameter directly. This
matches "the `prev_move` parameter and `moves_played[ply - 1]` then say the
same thing -- keep one source of truth" from the step file's own shape
section, read the other way: introducing `moves_played` now, before verdict 2
needs ply-2, would make two sources of truth for the same ply-1 fact, which
is exactly what that sentence warns against. `moves_played` is verdict 2's to
add, against this commit.

**Consequently "the known crash" does not apply verbatim.** The step file's
`accepts` describes the sentinel hazard as "(ss-1)... must be valid... or the
child indexes garbage" -- a hazard of a pointer or an array slot that can be
stale or uninitialised. `prev_move` is a function parameter, supplied fresh
on every call, and cannot go stale; `move_t` value `0` decodes to a *valid*
in-range cell, `(W_PAWN, a8)`, not an out-of-range index. So there is no
crash and no sanitizer finding available to have here -- confirmed
experimentally below, mutant S024_M02. What a dropped guard costs instead is
a silent wrong-cell write, which is what the sentinel tests and that mutant
are built to catch.

**Shape:**
- `src/data_structures.hpp`: `int16_t cont_hist[12][64][12][64]` (1.125 MiB)
  added as a plain value member of `search_state_t`, beside `counter_moves`.
  Not a `unique_ptr` as the discarded MacBook attempt used: that machine's
  search thread carried a 512 KB stack; `ulimit -s` here is 8192 KB and
  `search_state_t state = {};` (`src/chesso.cpp` `iterative_deepening_search`)
  is already rebuilt zeroed at the top of every search, which is also why
  `ucinewgame` needs no separate clear for this table, matching
  `quiet_history` and `counter_moves` today (neither has one either --
  checked; `tests/test_engine.cpp` "ucinewgame puts the board and the table
  back" tests only the position and the TT).
- One index helper, `continuation_entry()`, two overloads (`search_state_t*`
  returning `int16_t&`, `const search_state_t*` returning `int16_t`), both
  indexing `cont_hist[MOVE_PIECE(prev)][MOVE_TO(prev)][MOVE_PIECE(move)][MOVE_TO(move)]`
  -- MOVE_PIECE, not MOVE_FROM, matching `counter_moves`' existing convention
  (a promotion's mover is the pawn on the source square).
- `src/search.cpp` `history_on_quiet_cutoff()` gains a `prev_move` parameter
  and, guarded on `prev_move != 0`, applies the *same* `history_gravity_update`
  call S093 already uses for `quiet_history` to `continuation_entry()`'s cell
  -- malus for each quiet in `quiets_tried`, bonus for the cutoff move. No new
  bonus/malus formula, no new constant: `HISTORY_BONUS_*`/`HISTORY_MALUS_*`/
  `QUIET_HISTORY_MAX` are reused unchanged, exactly as section 4 of the step
  file says verdict 1 should ("Verdict 1 introduces no new constant"). The
  tune build needs no new parameter for the same reason.
- `src/evaluation.cpp` `score_move()`: the final quiet return becomes
  `quiet_history[...] + (prev_move != 0 ? continuation_entry(...) : 0)`,
  summed in `int`.
- **Band bound.** Two terms, each in `[-QuietHistoryMax, QuietHistoryMax]`, so
  the quiet band is now `[-2*QuietHistoryMax, +2*QuietHistoryMax]`. Clearance
  against `ORDER_COUNTER` (700000): `2*QuietHistoryMax + 100 <= 700000` holds
  to `QuietHistoryMax <= 349950`, and the declared range's own ceiling is
  32767 (`int16_t`'s own max), so the bound holds at every value the tuner can
  ever set it to, by construction of the declared range and not as a
  coincidence of today's default (8192). Asserted directly, not argued --
  `tests/test_evaluation.cpp` "the continuation term re-pins the band for the
  sum of two tables", both edges, plus the arithmetic fact that
  `2*QuietHistoryMax` (65534 at the declared ceiling) exceeds `int16_t`'s own
  32767, which is why the sum has to be carried in `int`.
- Initial constants: **none are new.** The formula, the clamp and the ceiling
  are all reused from S093, unfitted at their current shipped values, exactly
  as they already are for `quiet_history` -- S127's SPSA lane covers both
  tables' weights together whenever it runs, per the step file's own section
  4 and the DEC-084/DEC-105 seed rule (nothing here is a seed pulled from
  another engine; the formula shape is CPW's, the coefficients are this
  codebase's own S093 values, already shipped and already subject to a future
  fit).

### Tests, red first (DEC-141 clause 2, DEC-142)

Red observed by disabling the write side only (`const bool has_prev = false;`
in `history_on_quiet_cutoff`, temporarily) with the struct, helper and read
side all in place -- so red is a real assertion failure and not a compile
error:

```
tests/test_search.cpp:582: FATAL ERROR: REQUIRE( cont_hist_entries > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )
tests/test_search.cpp:660: FATAL ERROR: REQUIRE( continuation_entry(&state, prev_move, killer) != 0 ) is NOT correct!
  values: REQUIRE( 0 != 0 )
tests/test_search.cpp:862: ERROR: CHECK( continuation_entry(&state, prev_move, cutoff) > 0 ) is NOT correct!
  values: CHECK( 0 >  0 )
tests/test_search.cpp:865: ERROR: CHECK( continuation_entry(&state, prev_move, move) < 0 ) is NOT correct!
  values: CHECK( 0 <  0 )   (x3, one per tried quiet)
[doctest] test cases: 98 | 95 passed | 3 failed | assertions: 886491 | 6 failed
```
Reverted, rebuilt: 98/98 test cases, 889244/889244 assertions, green
(`build/tests/test_search`, run from `tests/`). `test_evaluation`: 24/24,
54402/54402, green, after fixing one self-inflicted wrong assertion of my own
first draft (`REQUIRE(2 * declared_max < 32767)`, backwards -- the point is
that the sum does *not* fit in int16_t, corrected to `>`).

Per numbered item:
1. **Sentinels.** Root / no-previous-move: "a quiet cutoff maluses the quiets
   tried before it" (direct call, `prev_move=0`) and "the cutoff move is
   credited and the quiets before it are charged" (through `negamax`,
   `prev_move=0`) both now additionally scan the whole `cont_hist` table and
   require it at 0. Real previous move: "a quiet cutoff with a previous move
   updates continuation history" (direct call) and the extended "a
   quiet move that gives check enters the ordering tables" (through
   `negamax`, real `prev_move`) both require the (prev, move) cell to move.
   "a search fills the ordering tables" (a real depth-8 search from
   `TRICKY_POS`) requires at least one `cont_hist` entry to be non-zero after
   real play, so the table is not simply dead code. Debug build: `test_search`
   passes under `build-debug` too (56.06 s under `ctest -R
   '^(test_chesso|...|test_search|...)$'`, `gate_extra.sh` stage `debug`),
   every `assert(` in `src/` live. Sanitizer: `gate_extra.sh` stage
   `sanitize` (below). **Mutant, guard dropped**
   (`const bool has_prev = true;`): no crash, no sanitizer finding -- an
   in-range cell is silently written instead. Killed by the sentinel scans:
   `CHECK_EQ( cont_hist_entries, 0 )` reads `CHECK_EQ( 4, 0 )` and
   `CHECK_EQ( 5, 0 )` at the two sentinel sites. Added to
   `tools/mutants/S024_continuation_history.py` as `S024_M02_cont_hist_no_prev_guard`;
   applied by hand and reverted (not through `tools/mutation_check.py`, which
   needs a linked worktree at a commit that includes this work -- re-validate
   there once this lands, S207's own precedent).
2. **Bookkeeping.** "a quiet cutoff with a previous move updates continuation
   history": cutoff cell `> 0`, each tried-quiet cell `< 0`, a
   different previous move's row untouched. A capture cutoff never reaches
   `history_on_quiet_cutoff()` at all (`if (!is_capture)` guards the whole
   call in `negamax_at`, unchanged by this step) so "leaves it unchanged" is
   structural, not a separate case to construct.
3. **Band bound.** "the continuation term re-pins the band for the sum of two
   tables": both tables driven to `QuietHistoryMax`'s declared maximum
   (32767) at once via the self-referencing cell `plain == prev_move`;
   `s_history == 2*declared_max`, `s_history_malused == -2*declared_max`,
   clearance against the countermove band `>= 100` both edges, and
   `2*declared_max > 32767` pins the int16_t-overflow fact the `int` sum has
   to survive.
4. **`ucinewgame`.** Structural, not a dedicated test: `search_state_t` is
   rebuilt `= {}` fresh at the top of every `iterative_deepening_search()`,
   so `cont_hist` clears the same way `quiet_history` and `counter_moves`
   already do, and neither of those has a dedicated `ucinewgame` test either
   (checked: `tests/test_engine.cpp`'s only such case covers the board and
   the TT).
5. **Mutation** (DEC-141 clause 2, S196). Two mutants,
   `tools/mutants/S024_continuation_history.py`:
   - `S024_M01_cont_hist_malus_sign` (the malus sign flipped): killed.
     `CHECK( continuation_entry(&state, prev_move, move) < 0 )`
     reads `CHECK( 64 < 0 )`, "A quiet tried before the cutoff scores 64 in
     the continuation table and not a malus."
   - `S024_M02_cont_hist_no_prev_guard` (above): killed by the two sentinel
     scans.
   Both applied by hand (edit, rebuild `test_search`, observe, revert,
   rebuild, confirm green); both validated syntactically (`load_mutants` +
   `validate` against the working tree, anchors unique); full
   `tools/mutation_check.py` run deferred to a committed worktree.

### Measurements

`tools/search_bench.py`, depth 9, best moves unchanged all three:

| position | before (`b5c357a`) | after (v1) |
|---|---|---|
| midgame | 121515 nodes, `c3d5` | 122266 nodes, `c3d5` |
| kiwipete | 801408 nodes, `e2a6` | 794606 nodes, `e2a6` |
| tactical | 72895 nodes, `d7c8q` | 75802 nodes, `d7c8q` |

`chesso bench`: **27322394 -> 22363740**, -4958654 nodes, -18.15 %. Commit
carries `Bench: 22363740`. `DEV_MANUAL.md`'s ledger line gains "At `S024` v1:
`22363740`."

Debug self-play (DEC-141 clause 1): 4 rounds at 4+0.04, `noob_3moves.epd`,
concurrency 8, **8 games in 19 s, 0 `Assertion`, 0 `disconnect`** (both
`level=trace engine=true` log and tee'd stdout). Time forfeits not checked --
not the failure condition at this control.

Gate, both builds, `CLANG_FORMAT_MAJOR=22`:
```
cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure
  -> 100% tests passed, 0 tests failed out of 34
cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure
  -> 100% tests passed, 0 tests failed out of 34
./clang-format.sh --check
  -> clean (no output)
```
Run twice end to end (once mid-implementation, once after the stash-based
before/after bench comparison put the tree back), both green.

`tools/gate_extra.sh` (five stages), `CLANG_FORMAT_MAJOR=22`:
```
-- prose ...       ok, 0s
-- citations ...   ok, 1s
-- debug ...       ok, 271s
-- sanitize ...    ok, 540s
-- perft ...       ok, 53s
GATE-EXTRA-DONE 5 stages 865 s
```
All five green. `citations` reports
`adocs/plan_current/S024_continuation_history.md: 4 code citations, 0
flagged` and `citations flagged: 0 over 59 files`; `prose` reports `0`
sentences flagged. `debug` is `ctest -R
'^(test_chesso|test_openings|test_movegen|test_evaluation|test_search|test_engine)$'`
under a fresh Debug build -- `test_search` 56.06 s, `test_evaluation` 0.22 s,
both green, every `assert(` in `src/` live. `sanitize` is the full `fast`
label under `-fsanitize=address,undefined` -- `test_movegen` 58.62 s,
`test_search` 13.03 s, all 34 green -- plus DEC-167's cross-build INV-6
check, sanitizer `bench` against a fresh Release `bench`: **22363740 nodes
both builds**, matching this step's own `after` figure exactly. `perft` is
`test_perft` at depths the fast label does not reach: green, 53.23 s.

### Documents

- `DEV_MANUAL.md`: bench ledger line, done (above).
- `MANUAL.md`: checked, no change. `QuietHistoryMax`'s documented option
  (name, default 8192, range 1-32767) is unchanged; it now additionally
  bounds `cont_hist` entries, which is an implementation detail under the
  option's existing wording ("the gravity bound on a quiet history entry"),
  not a new option, default or surface for `test_uci_surface` to catch.
- `adocs/specs.md`: not edited (hard limit). Two sentences handed to the
  coordinator for the search row, below.

## Measurement, verdict 1, pre-registered 2026-09-12

Modelled on `adocs/plan_done/S207_repetition_before_root.md`'s "## Measurement"
and `adocs/plan_done/S042_en_passant_only_when_capturable.md`'s "## Measurement,
pre-registered".

**`./fastchess.sh`**, default gainer bounds `elo0=0 elo1=5` (nElo),
`alpha=beta=0.05`, against the parent commit (`REF=<parent sha> ./fastchess.sh`;
the working tree at this step's commit is the candidate, so the banner prints
both shas with their commit dates) -- 8+0.08, `Hash=16`,
**`books/noob_3moves.epd`** (DEC-189), 12 threads.

**Worst-case games**, from the nElo run-length formula (DEC-143's own
figures for a `{0,5}` pair): **41861** at the interval's midpoint, **25591**
on a bound. Converted at **2110 games an hour** (DEC-190's calibration on this
book): **19.8 h** at the midpoint, **12.1 h** on a bound -- 12 to 20 hours
worst case, a night run (DEC-155). Launch detached, `Monitor` watcher armed
with the WATCHERS loop, four exits, ceiling 2x the worst case (40 h).

**Abort rule**: forfeit rate over 1.0 % on a side, `tools/forfeit_report.py`
over the run's own PGN, checked independently of the harness banner.

**Direction, discounted per DEC-019**: the step file's published range for
this technique runs from Weiss's +44.68 STC / +33.95 LTC (PR #477, 2021, the
plan's own headline figure) down to Lynx's +2.16 STC / +9.12 LTC (PR #645,
2024) -- the widest spread recorded for any technique on this plan, which is
why the bounds sit at the low end (`elo0=0`, not a larger floor) rather than
assuming the top of that range. A verdict of zero is a live, recordable
outcome and not a surprise (DEC-019, S005/S006/S015 precedent).

**Readings, pre-registered before the first game:**
- **H1**: the table is worth at least 5 nElo. Kept; magnitude is not the
  claim -- an SPRT that stops on a favourable swing is biased upward by
  construction (DEC-063), so the pair's own point estimate is not quoted as
  the effect size.
- **H0**: not worth 5 nElo (costs 5 or more, or gains less than 5). Before
  believing it: whether the table is actually being exercised in ordinary
  play is checked first, the same way S165's defender census and S162's
  clock census precede believing a pruning verdict -- an instrumented
  depth-10 pass over 400 corpus positions (the same corpus S165 and the
  reverse-futility notes in `src/search.cpp` already use) counting how often
  `history_on_quiet_cutoff()` reaches the `prev_move != 0` branch and how
  often `score_move()`'s continuation term is non-zero when read. A table
  that is barely touched cannot be the cause of a real regression; one that
  is never touched at all means the wiring, not the technique, is what H0
  measured, and either finding is checked before H0 is recorded as a verdict
  on continuation history itself.
- **No verdict at the cap**: recorded as zero. Kept or reverted per the step
  file's own rule -- a verdict of zero is recorded as zero and the feature
  may still be kept with the reason stated (DEC-019), and here the reason
  would be that S127's SPSA lane has not yet fitted `HISTORY_BONUS_*`/
  `HISTORY_MALUS_*`/`QuietHistoryMax` for the two-table sum this step creates,
  so an unfitted zero is not yet evidence that a fitted one would also be
  zero.

**The open defects named per DEC-171**: none of BUGS class is open at this
commit -- `adocs/plan_current/` holds only this step, and nothing in
`adocs/plan_todo/` is framed as an open defect (checked by listing both
directories; the coordinator's own tracking of `adocs/status.md` is the
authoritative source and should be re-checked at launch time, since this
subagent does not hold that document).

**Verified before the run, not assumed**: `sha256sum` of the candidate binary
the match actually plays should be checked against this commit's tree, as
S207's and S042's stamps did, once the coordinator snapshots it for the
launch.

**`adocs/data/S024_sprt.sh` already exists and is not this pre-registration.**
It predates this reimplementation, is pinned to macOS (`cd /Users/max/...`,
`caffeinate`, `pmset`), pins `REF=25998fe` -- a commit that is not this
commit's parent -- and plays `fastchess.sh`'s default book rather than
`noob_3moves.epd` (DEC-189 postdates it). Its own header already says it is
stale and names exactly two things to redo: `REF` and the bound pair; a third
now applies that its header does not know about, the book and the
throughput figure DEC-189/190 set. Not edited here -- `adocs/data/` existing
files are this subagent's hard limit -- but re-pointing it is more than the
one-line fix "re-point it at whatever commit the reimplementation sits on"
(the step file's own "What is left") suggests: it needs the workstation's
`fastchess.sh` invocation shape (no `caffeinate`/`pmset`; POWER and WATCHERS
per current `AGENTS.md`), this commit's sha as `REF`, and `noob_3moves.epd`.
`adocs/data/S024_pair_stats.py` beside it is generic PGN pair-statistics
tooling, not machine- or match-specific, and needs no change.

## Launch note, verdict 1, coordinator, 2026-09-12 08:06

Committed as `cace216` (`Bench: 22363740`) after the coordinator re-ran the
gate in both trees (34/34, 34/34, `clang-format.sh --check` clean) on the
final tree. `REF=b5c357a OUT=.tuning/s024v1_sprt_20260912_080633 ./fastchess.sh`,
detached, pid 4149701, console `.tuning/s024v1_sprt.log`. Banner: candidate
`cace216`, reference `b5c357a`, 8+0.08, hash 16, concurrency 12 of 12,
`noob_3moves.epd`, seed `20260912080633`, bounds `elo0=0 elo1=5 alpha=0.05
beta=0.05`; no busy warning, top process 2.2 % before launch. Candidate
binary the match plays, `build/src/chesso` at launch: sha256
`ad57cc154363d4f6984b530309766d405d707847e7b9b7381a0eb0a12bcf174f`. Governor
`powersave`, on mains. Watcher armed through `Monitor`, persistent:
`SPRT-RUN-(DONE|FAILED)`, process death, 40 h ceiling, polled every 60 s. Open
defects of BUGS class at launch: none (`adocs/status.md` checked). Started
in the morning rather than at night because the machine would otherwise have
idled: the ledger's fast class prices a real effect at about 2.5 h, and a null
runs to the 19.8 h wall, which the pre-registration accepts.

## Verdict 1, 2026-09-12

### SPRT: H0

`./fastchess.sh` against `b5c357a`, `elo0=0 elo1=5` nElo, `alpha=beta=0.05`,
`noob_3moves.epd`, 8+0.08, Hash 16 -- the pre-registration above, as run:

```
Elo: -4.42 +/- 5.81, nElo: -5.48 +/- 7.20
Games: 8954, Wins: 2936, Losses: 3050, Draws: 2968
Ptnml(0-2): [487, 1042, 1520, 954, 474]
LLR: -2.96 (-100.5%) (-2.94, 2.94) [0.00, 5.00]
Total Time: 04:10:11
```

**H0**: not worth 5 nElo, per the pre-registered reading -- a regression of 5
or more is not excluded either; the interval sits inside `[-5, 5]`,
0 has not been distinguished from a real effect either side of it. Forfeits
**0 of 8955** on both sides, `tools/forfeit_report.py` run over the run's own
PGN independently of the harness banner (the PGN itself holds 8955 game
records -- `grep -c '^\[Event' games.pgn` -- one more than the banner's own
`Games: 8954`; decisive-plus-draws from the forfeit report, 5987 + 2968,
matches the PGN's 8955 exactly, so nothing is missing from the record, and
the one-game gap is the banner's summary line printing before the PGN writer
appends the game that triggered the stop, not a discrepancy in what was
played). Rate: 8954 games in 4 h 10 m 11 s (15011 s) is **2147.4 games an
hour** -- next to S042's 2142.8 and well under the nine UHO-book entries'
2305 to 2346, both of this ledger's two `noob_3moves.epd` runs sharing the
new book's longer games (DEC-190: 114 plies against 102). Console
`.tuning/s024v1_sprt.log`, run directory
`.tuning/s024v1_sprt_20260912_080633/` (`games.pgn`, `fastchess.log`).

Console warnings: **22** `Incomplete mating PV` from the reference, **8**
from the candidate, and **1** `PV continues after threefold repetition` from
the candidate -- the S042 residual class named in the brief; noted, not
analysed here.

### The H0 census

**No fixed 400-position corpus file exists in this repository.** Grepped
before sampling anything: `adocs/data/`, every `S165`/`S103`/`S108` file, and
`src/search.cpp`'s own comments. `src/search.cpp`'s null-move mate-band note
("over 400 corpus positions at depth 10...") and S103's reverse-futility
hit-rate note ("300 positions...") both cite ad hoc, uncommitted instrumented
passes -- neither left a checked-in FEN list behind. So this census samples
its own 400, from `adocs/data/S219_aa_calibration.pgn` (S219's 1000-game
book-comparison self-play, the newest corpus of real chesso games on disk).

**Not through `build/tools/pgn_to_positions`, the brief's first-choice
method.** That tool hardcodes `DEFAULT_POSITION` as its replay start
(`tools/pgn_to_positions.cpp`), and every one of this PGN's 1000 games
carries its own `[SetUp "1"]`/`[FEN ...]` book-opening header -- confirmed by
grep, 1000 `[SetUp "1"]` for 1000 `[Event`. This is the exact wall S042 already
hit over this exact file (`adocs/plan_done/S042_en_passant_only_when_capturable.md`,
"FEN conformance" section): "these are self-play games from an opening book,
not from the default position, so `build/tools/pgn_to_positions` cannot
replay them, it hardcodes DEFAULT_POSITION." S042 drove the engine directly
over UCI instead, to test the engine's own FEN writer; this census's need is
narrower -- valid FENs of real-game positions, not a test of the writer -- so
`adocs/data/S024_census_run.py sample` reads each game's `[FEN]` header and
mainline with `python-chess`, already this codebase's oracle wherever a
position is checked against or built from outside the engine itself (S042's
own X-FEN comparison, S165's mate-set construction). No move is tracked by
hand (CLAUDE.md, DEC-023): python-chess replays every game and the sampled
FEN is the board after N of its own validated pushes.

**Sampling rule, deterministic.** `random.Random(24)` (this step's own id)
shuffles the 1000 game indices, then visits them in that order and takes,
from each, the position at that game's own midpoint ply
(`len(mainline_moves) // 2`) -- one position per game, so 400 distinct games
rather than several plies clustered in a few. First 400 games in shuffle
order that clear a 10-ply minimum are kept (all 1000 do -- shortest game is
31 plies, checked directly). The seed alone reproduces the set; written to
`adocs/data/S024_census_positions.txt`.

**Instrumentation.** A throwaway worktree (`git worktree add ... HEAD`,
removed after this run) at HEAD (`ef43bff`, which already carries `cace216`;
the two commits between them -- `47344f3`, `ef43bff` itself -- touch neither
`src/search.cpp`, `src/evaluation.cpp`, `src/data_structures.hpp` nor
`src/chesso.cpp`, checked by path-filtered `git log`, so the continuation-history
mechanism this census measures is byte-identical to what verdict 1's SPRT
played). Five `mutable uint64_t` counters added to `search_state_t`
(zeroed the same way `cont_hist` itself is, by `state = {}` at the top of
every `iterative_deepening_search()`): `history_on_quiet_cutoff()` increments
`census_writes_total` on every call and `census_writes_with_prev` when
`has_prev` is true; `score_move()`'s final quiet-scoring path (past the
TT/capture/promotion/killer/counter early returns) increments
`census_reads_total` on every call, and inside its own `prev_move != 0` guard,
`census_reads_with_prev` and -- when `continuation_entry(...)` reads non-zero
-- `census_reads_nonzero`. `iterative_deepening_search()` prints
`info string cont_hist_census <writes_total> <writes_with_prev> <reads_total>
<reads_with_prev> <reads_nonzero>` once per search, before `return result;`.
Release build in the worktree's own `build/` (`cmake -S . -B build
-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake
--build build -j12 --target chesso`), never touching the real tree's `build/`.

**Driven correctly, and the trap this avoided.** `printf
'...\ngo depth 10\n' | build/src/chesso` (no persistent pipe) answers depth 1
at 49 nodes -- confirmed on both an unmodified `build/src/chesso` and this
census's own instrumented binary, same input. Closing stdin the instant the
piped input is written reaches the engine the same way an explicit `quit`
would: it races `go` and the search never gets past its first iteration --
TOOLCHAIN.md's chess-oracle warning, general to this engine and not only to
Stockfish. `adocs/data/S024_census_run.py`'s `Engine` class instead keeps one
`subprocess.Popen` alive for the whole run: `uci`/`uciok`, `setoption name
Hash value 16`, `isready`/`readyok`; per position, `position fen`, then
`isready`/`readyok` (also the S216 precedent -- a refused `position fen` is
read from that reply and exits loudly rather than silently measuring the
previous board), then `go depth 10` with every line read through to
`bestmove` before anything else is sent, then `ucinewgame`; `quit` only after
the 400th `bestmove`, never behind an un-awaited `go`.

**Totals**, `adocs/data/S024_census.tsv`, 400 positions, depth 10, Hash 16:

```
total nodes:    112638096
wall time:      15.1 s
writes_total:          1388919   writes_with_prev:   1354960   (97.56 %)
reads_total:          83257904   reads_with_prev:   80083780   (96.19 %)
reads_nonzero:        21735125   of reads_with_prev: 80083780  (27.14 %)
per-position reads_nonzero/reads_with_prev share:
  min 4.15 %   median 24.06 %   max 48.37 %
```

**Reading, per the pre-registration's own two failure modes -- barely
touched, or never touched.** Neither holds. 97.56 % of
`history_on_quiet_cutoff()` calls reach the `prev_move != 0` branch and
96.19 % of quiet `score_move()` evaluations consult the continuation term
across 400 real positions at depth 10, 112638096 nodes total -- writes and
reads both reach their guarded branch on nearly every attempt, the small
remainder being exactly the root ply and the node right after a null move,
where `prev_move` is genuinely 0 by construction. Of the reads that consult
the table, 27.14 % find a non-zero entry, and this is not a property of one
or two favourable positions carrying the average: the per-position share of
non-zero-among-consulted ranges 4.15 % to 48.37 %, median 24.06 %, so every
position in the sample has the table returning a real, non-zero signal on a
substantial fraction of its own quiet-move reads. This is the opposite
profile from the null-move census this pre-registration's H0 clause is
modelled on (`src/search.cpp`: 0 of 301620 eligible nodes in ordinary play,
inert wiring where any verdict would have been a verdict on the wiring and
not the technique). Here the table is exercised almost everywhere a quiet
move is scored, and what it holds is read back as a real number roughly a
quarter of the time it is consulted. If H0 stands as the verdict on
continuation history, this census says it is not for want of the table being
touched.

### Ledger arithmetic

`adocs/plan.md` "## What this costs", recomputed with S024 v1 added to the
ledger (wall `4 h 10 m 11 s` = 15011 s exactly, 8954 games, bounds `{0, 5}`,
verdict H0, slow class -- an effect inside the interval). All eleven wall
times, converted to seconds and sorted:

| run | wall | seconds |
|---|---|---|
| S149 | 1 h 05 m | 3900 |
| S107 | 1 h 37 m 52 s | 5872 |
| S042 | 2 h 40 m 45 s | 9645 |
| S093 v1 | 2 h 44 m | 9840 |
| S024 v1 | 4 h 10 m 11 s | 15011 |
| S207 | 4 h 26 m 56 s | 16016 |
| S108 | 5 h 26 m 38 s | 19598 |
| S148 | 6 h 19 m 35 s | 22775 |
| S093 v2 | 6 h 35 m | 23700 |
| S130 | 7 h 12 m | 25920 |
| S165 | 7 h 58 m 07 s | 28687 |

Sum of the ten (cross-check against plan.md's own "107107 games in 46.10
hours"): 165953 s = 46.09806 h -> **46.0981 h**, matches to the digit; ten-set
mean 16595.3 s = 4 h 36 m 35 s -> rounds to the stated "4 h 37 m"; ten-set
median (5th+6th)/2 = (16016+19598)/2 = 17807 s = 4 h 56 m 47 s -> rounds to
the stated "4 h 57 m". Method cross-checked against the document's own
published figures before extending it.

**Eleven, with S024 v1 added.** Sum = 165953 + 15011 = **180964 s**.
- **Mean**: 180964 / 11 = 16451.2727 s = **4 h 34 m 11 s** (rounds to 4 h 34 m).
- **Median** (6th of 11 sorted): **16016 s = 4 h 26 m 56 s** -- S024 v1's
  15011 s slots in just below S207's 16016 s, so the two middle values of the
  old ten-set collapse to one exact data point rather than an average.

**Slow class, six now**: S024 v1 (15011), S108 (19598), S148 (22775),
S093 v2 (23700), S130 (25920), S165 (28687). Sum = **135691 s**.
Mean = 135691 / 6 = 22615.17 s = **6 h 16 m 55 s** (rounds to 6 h 17 m),
against the five-run 6 h 42 m 16 s ("6 h 42 m") before this verdict.

**Fast class, unchanged**: S149, S107, S093 v1, S207, S042 -- none of this
step's business, still five, still 45273 s / 5 = 9054.6 s = 2 h 30 m 55 s,
**"2 h 31 m"**.

**Totals across the set**: games 107107 + 8954 = **116061**; hours
180964 / 3600 = **50.2678 h**; rate 116061 x 3600 / 180964 = **2308.9 games
an hour** (2308.85 unrounded) -- against the ten-set's 2323.5, pulled down by
S024 v1's own 2147.4.

**S024 v1's own rate**: 8954 x 3600 / 15011 = **2147.4 games an hour**.

**Priced totals, plan.md's "Priced by class" formulas, same 42-52 / 45-55
verdict-count ranges, new means substituted:**

```
42 to 52 slow-class  x 6 h 16 m 55 s (6.2820 h) =  264 to 327 h
        (42 x 6.2820 = 263.8 h;  52 x 6.2820 = 326.7 h)

flat mean 4 h 34 m 11 s (4.5698 h)  x 45 to 55  =  206 to 251 h
        (45 x 4.5698 = 205.6 h;  55 x 4.5698 = 251.3 h)
```

For reference only, not requested but following the section's own structure
(fast class unchanged at 3 x 2 h 31 m = 7.5 h): class-split total
7.5 + 264 to 327 = **271 to 334 machine-hours**, against the pre-S024-v1
289 to 356. Both totals fall as the new slow mean (6 h 17 m) is lower than
the five-run figure it replaces (6 h 42 m) -- S024 v1 landed faster than the
slow class's previous average, not because the pair was cheaper (same
`{0, 5}`) but because `noob_3moves.epd` runs at a lower games-an-hour rate
that this ledger has now seen twice (S042, S024 v1) and both land on the low
side of the games *count* a `{0, 5}` pair needs, not the wall time -- the
two effects partially cancel and are not the same thing. **Not applied to
plan.md**: this subagent's hard limit; the coordinator recomputes the
document itself from these figures.

Note, separate from anything this subagent was asked to touch: the section's
existing prose reads "The flat mean gives **217 to 265 hours** (4 h 37 m x 45
to 55, 207 to 254 hours)" -- the parenthetical (207-254) is the arithmetic
that `4 h 37 m x 45 to 55` actually gives (4.6167 h x 45 = 207.75,
x 55 = 253.9), and the headline figure beside it (217-265) does not match
either that parenthetical or the formula it glosses. Flagged for the
coordinator, not corrected here (plan.md is this subagent's hard limit).
