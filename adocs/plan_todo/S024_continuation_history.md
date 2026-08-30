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
decisions:
closes:
blocks:
paused_by:
done:

## Note

Chesso has the countermove *heuristic* and no continuation history at any
depth. `move_t counter_moves[12][64]` (`src/data_structures.hpp:436`, written
on beta cutoff at `src/search.cpp:517`, read as the fixed `ORDER_COUNTER` band
at `src/evaluation.cpp:1152-1155`) remembers a single refutation move per
(previous piece, target) and gives it a flat bonus — the countermove heuristic,
Uiterwijk 1992.

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
author:    Maksym Bodnar

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
