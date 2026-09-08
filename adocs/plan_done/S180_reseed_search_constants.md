id:         S180
goal:       the seven pending search steps whose constants are seeded from another engine's constants quoted in commit or PR prose are reseeded under DEC-105 -- a value from a publication about the technique, a derivation over chesso's own data or scale, or a range midpoint -- so no engine-originated number is a starting point anywhere in the plan
accepts:    every seed in the "Constants and seeds" sections of S095, S097, S098, S109, S113, S114 and S132 is one of three things and the file says which: (a) a number from a publication about the technique with its URL beside it -- a paper, an article, the wiki's own derivation or example; (b) a derivation procedure the owning step runs at its start over chesso's own positions, data or scale, written out as the procedure and never as a number taken from elsewhere; (c) the parameter's range midpoint or off value, stated as such; no seed cites another engine's commit message, PR body, source, table or shipped value, and the sentence in S114 that calls prose-quoted numbers legal seeds is deleted; each rewritten section carries the line "seeds re-derived 2026-09-04 under DEC-105 (DEC-134)"; the F01 table in `adocs/audit/2026-09-04_plan_review.md` is walked row by row and each row's replacement is named in this step's done stamp; `tools/plan_prose_check.py --touches` and `--params` green; no engine file changes
touches:    adocs/plan_todo/S095_internal_iterative_reduction.md, adocs/plan_todo/S097_singular_extensions.md, adocs/plan_todo/S098_reduction_refinement.md, adocs/plan_todo/S109_shallow_depth_pruning_block.md, adocs/plan_todo/S113_probcut.md, adocs/plan_todo/S114_null_move_refinements.md, adocs/plan_todo/S132_time_management_node_fraction.md, adocs/plan_todo/S184_pending_documents_at_head_values.md (the DEC-157 hand-off note only), adocs/decisions.md, adocs/plan.md, adocs/status.md
excludes:   the seeds S091, S112 and S116 already take from the wiki or from chesso's own scale, which are the pattern and need no change; any engine code; running any derivation -- the owning step runs it at its start, this step writes the procedure down
decisions:  DEC-105, DEC-134, DEC-157
closes:     2026-09-04_plan_review-F01
blocks:
paused_by:
author:     Claude Opus 5 (coordinator)
done:       2026-09-08. **The seven block-1 search steps no longer tell an implementer to start from another engine's constant.** Documents only: no file under `src/` changed, `tools/gate.sh` prints **GATE-DONE 26851183**, the parent's number, so the commit carries no `Bench:` line and no `No functional change` trailer (DEC-140 binds commits that touch `src/`). No file had been reseeded by a DEC-145 enrichment pass first -- `grep -l 'Seeds replaced'` over the seven matched only this step's own file -- so all seven were rewritten and none was verified in place. **The F01 table of `adocs/audit/2026-09-04_plan_review.md`, walked in its own order, ten rows:** (1) S114 `NULL_MOVE_EVAL_MARGIN` 100, "SF 074c7a3 prose: one pawn" -> **(b) 94**, one pawn in chesso's own material scale, `PAWN` in `src/eval_tables.hpp`, the treatment DEC-134 decided by name. (2) S114 `NULL_MOVE_EVAL_CAP` 3, "SF 7ed15af prose: 'three plies'" -> **(c) 8**, midpoint of 0..16 stated as such, with the P4 measured bound written beside it and the mate suites named as what decides the seed's admissibility before any sweep -- the owner's answer of 2026-09-08, DEC-157. (3) S113 `PROBCUT_MARGIN` 150, four engines' margins averaged -> **(b)**, P1: Buro's own regression run over chesso's own positions at the paper's threshold `t` = 1.0 (**(a)**, https://skatgame.net/mburo/ps/chessmpc.pdf, Figure 2 `#define T 1.0`), procedure written out with its exclusions, its least squares and its `a` / `b` / `sigma` recording, and "fewer than 200 surviving positions is a finding, not a seed". (4) S113 `PROBCUT_DEPTH_OFFSET` 4, "SF 71cc01c" -> **(a) 4**, the same paper's single-pair implementation (Figure 2 `#define S 4`, `#define H 8`; Table 1 regresses (3,5) and (4,8)). The value is unchanged and the provenance is not: it is the paper's, and the coincidence with an engine's 4 is irrelevant. (5) S132 `TM_NODE_BASE_PCT` 200 / `TM_NODE_SCALE_PCT` 100, "the Lynx #1203 prose pair" -> **(b)**, P3: a census of the best move's share of root-child nodes over the 300-position pick, with two constraints that are chesso's own -- the factor is 100 at the census median so the seed spends today's allocation in expectation, and `TM_SCALE_MIN_PERCENT` at `f` = 1 -- giving `TM_NODE_SCALE_PCT = 7000 / (100 - 100 f_med)` and `TM_NODE_BASE_PCT = 100 + 3000 / TM_NODE_SCALE_PCT`; fallback **(c)** midpoints 250 and 150, said so in the stamp if taken. (6) S098 `LMR_HIST_CLAMP` 2, "Weiss #451's 'between +2 and -2'" -> **(b)**, P5: half chesso's own reduction table at the census median depth and the last move index, `0.52 + ln(11) * ln(63) / 1.82` = 5.98 stored floored as 5, half rounded down = **2**, the arithmetic written in so it is re-derived when `LMR_BASE` / `LMR_DIVISOR` move; fallback **(c)** midpoint of 0..4, the same integer. (7) S109 `LMP_DEPTH_COEFF` 10 / `LMP_MAX_DEPTH` 3, "Lynx #512 PR prose" -> **(b)**, P2: a cutoff-index census under `#ifdef CHESSO_TUNE`, 95th percentile per depth, least squares over depths 1..3, and `LMP_MAX_DEPTH` at the largest depth where the fitted threshold is still below the median quiet-move count -- a rule that never binds is not measurable. (8) S109 `SEE_QUIET_MAX_LMRDEPTH` 9, "SF a834bfe prose" -> **(c) 8**, midpoint of 0..16 where the range is declared by purpose: 0 is off and 16 is above the median depth 11 the `RFP_MAX_DEPTH` comment records, where the gate stops binding. (9) S097 `SE_MIN_DEPTH` 8 and `SE_TT_DEPTH_MARGIN` 3, "SF prose 8e823459" and a talkchess write-up -> **(c) 10** of 4..16 and **(c) 4** of 0..8. No (a) was available and the file says so: the wiki's page states no depth and the 1988/1990 papers are paywalled, **unverified**, both URLs recorded. The (b) alternative -- a reliability profile of table-entry moves by depth -- is written out as S097's to run instead. (10) S095 `IIR_MIN_DEPTH` 4, "Lynx #507's introduction value" -> **(a) 6**, the wiki's own hypothetical "only use IIR if depth > 5, say" (https://www.chessprogramming.org/Internal_Iterative_Reductions, fetched 2026-09-05) read against the sketch's `depth >= IIR_MIN_DEPTH`. **The inventory rows F01 did not list, which `accepts` binds as "every seed" and DEC-157 confirms belong in this stamp:** S095 `IIR_REDUCTION` -> **(c)** 1, the integer below the 1.5 midpoint of 0..3, with the rule stated once per file. S097 `SE_PLY_FACTOR` -> **(c)** 5 of 2..8; `SE_MARGIN_PER_DEPTH` -> **(c)** 9, midpoint of a range declared by purpose (floor 1, top 18 = `2 * PAWN / SE_MIN_DEPTH` floored), and the "flat 10..100 experimentation range" the 2026-08-19 pass attributed to the wiki is **not on the page as fetched** and is dropped, as is the "depth/2 to depth-N" verification space, marked **unverified**. S098: the `LMR_BASE` / `LMR_DIVISOR` re-sweep clause quoting three engines' curve coefficients off the wiki's LMR page is **deleted** -- DEC-105's own PeSTO case -- leaving the shipped 52 / 182, S085's SPSA output over chesso's games, as **(b)** and the only admissible seed; `LMR_HIST_DIV` kept as **(b)** with `HISTORY_MAX` corrected to `QUIET_HISTORY_MAX` (8192) and the `3M` sum's dependence on S024's continuation tables stated (`M / LMR_HIST_CLAMP` until they land); `LMR_CUTNODE`, `LMR_NOT_IMPROVING`, `LMR_TT_CAPTURE`, `LMR_PV` -> **(c)** 1 each of 0..2 with their attributions struck; `LMR_MIN_MOVES` -> **(b)** 4, the parameterised `legal_moves_counter > 3` in `negamax`; `LMR_MIN_MOVES_PV` -> **(c)** 6 of 4..8; `LMR_DEEPER_MARGIN` / `LMR_SHALLOWER_MARGIN` -> **(c)** 47, midpoint of 0..`PAWN`. S109 `FUT_BASE` / `FUT_SLOPE` -> **(a)** Heinz's margins as the wiki states them (https://www.chessprogramming.org/Futility_Pruning, fetched 2026-09-05), converted in chesso's scale: minor `(KNIGHT + BISHOP) / 2` = 317, rook `ROOK` = 487, so slope **170** and base **147** where the 2026-08-19 pass had 100 / 200 -- **it had read the same two words in `see_value`'s scale, which is the wrong pawn**; `FUT_MAX_LMRDEPTH` and `HP_MAX_DEPTH` -> **(c)** 8 of 0..16 by the same declared purpose; `HP_COEFF` -> **(c)** 576, the arithmetic midpoint `9M/128` of the file's own `M/64..M/8` region at `QUIET_HISTORY_MAX` 8192, with Weiss's "low depths" phrase struck; `SEE_QUIET_COEFF` kept as **(b)** and its `see_value` citation reduced to the symbol. S113 `PROBCUT_MIN_DEPTH` 5, unstated origin with an SF sentence beside it -> **(a) 8**, the paper's check height for the pair (4,8), so at the minimum depth the shallow search is the paper's 4-ply search; the SF sentence struck, and a second seed attribution found outside section 4 (`PROBCUT_DEPTH_OFFSET` "seed 4, SF prose" in the section 2 arithmetic) rewritten to point at the paper. S114 `NULL_MOVE_BASE` -> **(b)** the shipped 3, with Heinz's "R=3 when normal search depth exceeds 6 plies and R=2 otherwise" (https://www.chessprogramming.org/Depth_Reduction_R, fetched 2026-09-05) kept as **(a)** context for the sweep range and Fruit's 3 struck; `NULL_MOVE_DIVISOR` -> **(b)** the shipped 6, the Lynx "depth/5" clause struck; the entry-gate margin attribution struck. S132 `TM_NODE_MIN_DEPTH` -> **(b)** equal to `ASPIRATION_MIN_DEPTH` as compiled at S132's HEAD, **and (c) is refused there with the refusal stated**: the midpoint 32 of 0..64 is above every depth this engine reaches and would seed the feature switched off. **The two rule sentences are gone.** S114's *"numbers quoted from message prose are legal seeds, the S113 pass's precedent"* and S113's *"Buro's papers are open literature and were read in full; their numbers are legal seeds (DEC-084)"* are replaced by DEC-105's provenance form -- S113's keeps the true half, that a paper about the technique is form (a), and adds that an engine's shipped number is form (a) for nothing. **Two stale shipping values fixed in passing (DEC-157, answer three):** S114's "`NULL_MOVE_BASE` seed 2, ships today" -> 3 since S085, and S132's `TM_NODE_MIN_DEPTH` "5, seeded beside `ASPIRATION_MIN_DEPTH`" -> 2 as compiled. `adocs/plan_todo/S184_*.md` carries a paragraph saying both are done and not to look for them; the rest of its F05 class is untouched. **Two things the guide did not anticipate, corrected on the reviewer's read:** narrowing `SE_MARGIN_PER_DEPTH`'s range to 1..18 by purpose leaves **no off value inside it** -- at the top the margin is still two pawns at `SE_MIN_DEPTH` and the extension keeps firing -- so the file says that instead of repeating the old "off = range top"; and S098's `LMR_DEEPER_MARGIN` / `LMR_SHALLOWER_MARGIN` off values are opposite ends of 0..`PAWN` (top and 0 respectively), which one shared sentence had flattened into "0 is off". Every rewritten section also gained the units paragraph (P6) where a margin is compared against `evaluate()` -- S097, S098, S109, S113, S114 -- naming `piece_value` and, in S109, both scales side by side with the sentence that the 2026-08-19 pass conflated them. The seven `decisions:` fields gained `DEC-105, DEC-134`, so a reader opening any of them meets the rule the section was written under. **Checks (section 6).** 1: the engine-name and citation regex over all seven `### 4` sections prints **nothing**. 2: `grep -n 'legal seed'` over S113 and S114 prints **nothing**. 3: `seeds re-derived 2026-09-04 under DEC-105 (DEC-134)` appears **exactly once** in each of the seven. 4: every seed line carries (a), (b) or (c) -- reviewer's read, done twice, the two off-value corrections above being what it found. 5: `plan_prose_check.py --touches` prints `touches flagged: 0 over 69 files`, `--params` exits 0. 6: fast suite green in both builds, 28 of 28 including `test_plan_touches` and `test_plan_params`, format clean under `CLANG_FORMAT_MAJOR=22` (DEC-146). **Docs.** `DEV_MANUAL.md` and `MANUAL.md` checked and need nothing -- no command, no option, no default moved. `adocs/specs.md` needs nothing: its Non-goals already carry "Another engine's constants are never seeds, and republication does not change their origin". `README.md` untouched, human-owned. **Not done here, on purpose.** The F01 `Status:` line in the audit report stays `planned -- S180`; it moves to `closed` at the next audit's re-measurement. No derivation was run: P1, P2, P3 and P5 are procedures for their owning steps, which is what `excludes` requires, and each owning step's stamp is where their outputs land. Closes `2026-09-04_plan_review-F01`.

## Why this exists

`2026-09-04_plan_review-F01`. The enrichment pass of 2026-08-19 wrote the
"Constants and seeds" sections of the block-1 search steps under DEC-084's
venue rule: a commit message or PR body is a published write-up, so a constant
quoted in one was a legal seed. Three days later DEC-105 made the rule
provenance-based -- a number that originates as another engine's tuned output
is never a seed, wherever it is republished -- and named `plan.md`,
`CLAUDE.md`, `AGENTS.md` and `specs.md` as the documents carrying the new
form. The step files were not on that list and were never re-read. S114 still
says, in so many words, that "numbers quoted from message prose are legal
seeds, the S113 pass's precedent"; S113 says its numbers "are legal seeds
(DEC-084)".

Nothing in the engine carries these constants. The defect is that an
implementer opening any of the seven files is instructed to start a sweep or
an SPSA run from Stockfish's, Weiss's, Lynx's, Ethereal's or Berserk's number,
and DEC-105's own reasoning is why that matters: a seeded-then-refit vector
converges near its seed by construction, so "we refit it" is not a defence.
The owner's decision of 2026-09-04 (DEC-134) is to stick to the rule.

## The seven, and what replaces each

Taken from the F01 table of the report, verified against the files at
`18deccf`. "Prose" means the number was read from a commit message or PR body.

| step | engine seed | replacement |
|---|---|---|
| S095 `IIR_MIN_DEPTH` 4 | Lynx #507's introduction value | the wiki's own page says "depth > 5, say" -- a literature seed, cite `https://www.chessprogramming.org/Internal_Iterative_Reductions` |
| S114 eval-scaled null-move margin 100 | SF prose "one pawn" | one pawn *in chesso's own material scale* -- the pawn weight `evaluate()` ships, read at the owning step's HEAD; the unit is chesso's, not Stockfish's 100 |
| S114 eval-scaled cap 3 | SF prose "three plies" | range midpoint, swept |
| S113 ProbCut margin 150 | SF, Weiss, Ethereal, Berserk constants averaged | Buro's own method, which the file already names: regress chesso's shallow-vs-deep scores over its own positions and set the margin at t x sigma; the procedure is written, the owning step runs it |
| S113 ProbCut depth offset 4 | SF prose "depth - 4 plies" | the depth pairs the wiki's ProbCut page describes, or the range midpoint -- the file says which |
| S132 time-management base 2.0, scale 1.0 | Lynx #1203 prose | own census: choose base and scale so the expected time spent equals today's allocation at chesso's measured mean best-move node fraction over a census run, so the seed is behaviour-preserving in expectation |
| S109 late-move-pruning threshold `depth * 10`, cap 3 | Lynx #512's shipped constants | own census: the index of the cutoff move per depth over fixed-depth runs on chesso's positions; threshold at the stated percentile |
| S109 quiet-SEE reduced-depth gate 9 | SF prose | range midpoint, swept |
| S098 history clamp 2 | Weiss #451 prose | a stated fraction of chesso's own reduction table range, or the range midpoint |
| S097 singular min depth 8, table-depth margin 3 | SF prose; a talkchess write-up of SF-inspired code | the 1988 Anantharaman, Campbell and Hsu paper's parameters if S186-style research fetches them -- a publication about the technique -- else a profile of chesso's own table-entry reliability by depth, or the midpoint |

The three clean sections are the pattern: S112 seeds its margin from the
wiki's "typically around 200" and its victim table from chesso's own
`see_value`; S116 seeds from the wiki's "about three pawns" and declines the
Stockfish snippet the same page reproduces; S091 and S109 derive their SEE
and history coefficients from chesso's own scale.

## What this step does not do

It runs nothing. A derivation is written as a procedure with its inputs, its
tool and its acceptance, and the owning step runs it at its start -- that is
where the census or regression belongs, against the tree as it stands then.
Where a derivation is not cheap the fallback is the range midpoint, stated as
such, and the fit finds the value.

## Cost

Documents only. A few hours of careful rewriting over seven files, and one
pass of the checker.

## Implementation guide (2026-09-05)

Written under DEC-145 against HEAD `bdb5d56`. Symbols per DEC-135, no line
numbers. Every number from outside the repository carries its URL or the
word `unverified`.

### 1. What this step is, for someone new

Seven pending step files -- S095, S097, S098, S109, S113, S114, S132 -- carry
a section `### 4. Constants and seeds` written on 2026-08-19. It tells each
step's implementer where to *start* a sweep or an SPSA run, and several of
those starting values are numbers other engines shipped, read out of their
commit messages and pull-request bodies. DEC-105 says such a number is never
a seed wherever it is quoted; DEC-134 ordered the seven sections rewritten.
This step is that rewrite. It touches no code, runs nothing and measures
nothing: a replacement that is a derivation is written as a procedure with
its inputs, tool and acceptance, and the owning step runs it when it starts.
The deliverable is seven edited documents, one commit, and a `done:` stamp
that walks the F01 table of `adocs/audit/2026-09-04_plan_review.md` row by
row.

Two things make it more than search-and-replace. DEC-145's enrichment pass may
reach some of the seven before this step starts and replace their seeds in
place under a paragraph "Seeds replaced"; for such a file this step verifies
rather than rewrites (`grep -l 'Seeds replaced'` over the seven). And the F01
table lists eleven seeds while the sections hold more engine-originated
numbers than that; `accepts` binds *every* seed, so the inventory in section
4 is the work list, F01 rows first.

### 2. The technique as published

A document step, so the "technique" is a standard: the provenance rule for
seeds. `adocs/decisions.md` `## DEC-105`: "A number may seed a fit or an SPSA
run only if it originates in a publication about the technique -- a paper,
an article, the wiki's own derivations and example formulas. A number that
originates as another engine's tuned output is never a seed, wherever it is
republished." `## DEC-134` extends it to the step files -- "a commit message
is a venue like the wiki" -- and to units: "'one pawn' is expressed in
chesso's own material scale". The reason is in DEC-105's Context: a
seeded-then-refit vector converges near its seed by construction, so "we
refit it" is no defence.

The three admissible forms, lettered as this guide uses them:

- **(a) literature value**: a number from a publication *about the
  technique* -- a paper, an article, the wiki's own example -- with its URL.
  A wiki page reproducing an engine's tuned formula (the LMR page's Obsidian,
  Weiss and Ethereal coefficients; PeSTO on the Piece-Square Tables page) is
  the engine's number, DEC-105's own example.
- **(b) derivation over chesso's own data or scale**: a procedure the owning
  step runs at its start, or a number read from chesso's own constants
  (`PAWN` in `src/eval_tables.hpp`, `see_value` in `src/bitboard.cpp`, a
  value S085's SPSA produced). Written as the procedure, never as the number
  it is expected to return.
- **(c) range midpoint or off value**, stated as such, of the range the
  section declares for `src/search_params.hpp`, whose header says what a
  bound may be: arithmetic, stated purpose or measured -- never a guess at
  where the good values are.

The pattern files show each form: S112's `### 4. Constants and seeds` seeds
its margin from the wiki's Delta Pruning "typically around 200" (a) and its
victim table from `see_value` (b); S116 seeds `RAZOR_MARGIN` from the wiki's
Razoring "~three pawns" (a) and *declines* the Stockfish formula the same
page reproduces; S091's `SEE_CAPT_COEFF` is "the bar at depth 1 under one
pawn of `see_value`" (b), and its `SEE_CAPT_MAX_DEPTH` "sweep 4..10" names no
seed -- read it as (c), midpoint 7, and say so if you touch it.

### 3. What chesso has today, and where the change plugs in

Nothing in `src/` carries any constant in question; each is a future `X(...)`
row of `CHESSO_SEARCH_PARAMS` in `src/search_params.hpp`. What exists at HEAD
and matters:

- Material scale, `src/eval_tables.hpp`: `PAWN` 94, `KNIGHT` 327, `BISHOP`
  308, `ROOK` 487, `QUEEN` 716, collected in `piece_value`. The header's own
  comment says the split between `piece_value` and `psqt_mg` / `psqt_eg` is
  degenerate, so `piece_value` alone is the unit (P6).
- Exchange scale, `src/bitboard.cpp`: `see_value` `{100, 300, 300, 500, 900,
  10000}`, read by `see_ge`. A threshold compared against a SEE result is in
  this scale; a margin compared against `evaluate()` is in the material
  scale. The two pawns differ.
- Shipping values the sections quote and must quote correctly, from the
  `X(...)` list: `NULL_MOVE_BASE` 3 (S114 says 2 "ships today"; S085 moved it
  to 3), `NULL_MOVE_DIVISOR` 6, `LMR_BASE` 52 / `LMR_DIVISOR` 182 (S085's
  SPSA output), `QUIET_HISTORY_MAX` 8192 (S098 and S109 write `M` or
  `HISTORY_MAX`; that symbol does not exist), `ASPIRATION_MIN_DEPTH` 2 (S132
  seeds a depth gate "beside ASPIRATION_MIN_DEPTH" at 5, its pre-S085
  value), `TM_SCALE_MIN_PERCENT` 30, `TM_SOFT_PERCENT` 60.
- Reduction table: `lmr_reduction` in `src/search.cpp`, `LMR_BASE/100 +
  ln(depth) * ln(move_number) / (LMR_DIVISOR/100)`, both axes clamped to 63,
  the quotient floored; tests read it through `search_lmr_reduction_probe`.
  LMR's move guard is `legal_moves_counter > 3` in `negamax`.
- Instruments a derivation runs on: the tune build (`-DCHESSO_TUNE=ON`,
  DEV_MANUAL.md "The tune build"), where `setoption` sets any `X(...)` row
  without a rebuild; `adocs/data/S018_raw.tsv` (13522 rows, columns `game ply
  phase cost ref own mate san fen`) and the stratified 100-per-offset pick
  `adocs/data/S021_aspiration_sweep.py` documents, three offsets making the
  "300-position set"; the three `POSITIONS` of `tools/search_bench.py`; the
  mate suites `TEST_SUITE("engine: mate safety")` in `tests/test_engine.cpp`
  and `TEST_CASE` "pruning does not hide a forced mate" in
  `tests/test_search.cpp`.

Order of edits, per file: (1) read `### 4. Constants and seeds` and `### 1.
State of the art` once -- the seeds lean on records the earlier section
quotes; (2) rewrite the seeds section against the inventory; (3) move every
"anti-seed" sentence (an Elo record of a value that measured negative
elsewhere) into `### 5. Pitfalls` under one line "Anti-seeds -- records, not
seeds", so the seeds section names no engine and check 1 of section 6 reads
empty; (4) delete S114's "numbers quoted from message prose are legal seeds,
the S113 pass's precedent" and S113's "their numbers are legal seeds
(DEC-084)"; (5) append to each rewritten section the exact line
`seeds re-derived 2026-09-04 under DEC-105 (DEC-134)` -- the date is the
decision's and is quoted in `accepts`, write it as it stands; (6) section 6's
checks; (7) one commit, section 8.

New text cites code by symbol (DEC-135) and repeats the path (DEC-120). The
old sections' path-colon-line citations are S187's to convert: leave any you
are not otherwise rewriting, and write no new one.

### 4. Constants and seeds

"Today" is the value at `bdb5d56`; "origin" the file's own attribution;
"replacement" the DEC-105 form and value or procedure. F01 rows are marked;
the rest are seeds F01 did not list but `accepts` covers. P1 to P6 follow.

| file | constant | today | origin the file states | replacement |
|---|---|---|---|---|
| S095 (F01) | `IIR_MIN_DEPTH` | 4 | "Lynx #507's introduction value" | **(a)** 6: the wiki's own "only use IIR if depth > 5, say", https://www.chessprogramming.org/Internal_Iterative_Reductions -- stated as the page's hypothetical, no engine attached (fetched 2026-09-05); with the sketch's `depth >= IIR_MIN_DEPTH`, "depth > 5" is 6. Range 2..63 stands |
| S095 | `IIR_REDUCTION` | 1 | "every traced introduction" | **(c)** midpoint of 0..3 is 1.5; take 1, the integer below, and say so -- the wiki states no amount (fetched). The "-12.38 / -46.23 at 2" sentences are anti-seeds: to Pitfalls |
| S097 (F01) | `SE_MIN_DEPTH` | 8 | "SF prose 8e823459" | **(c)** midpoint of 4..16 = 10. The 1988/1990 paper is paywalled (https://dl.acm.org/doi/abs/10.1016/0004-3702(90)90073-9) and the wiki page states no depth: no (a) available, **unverified**. Name the (b) option -- a reliability profile of TT-entry moves by depth -- as the owning step's, and seed the midpoint |
| S097 (F01) | `SE_TT_DEPTH_MARGIN` | 3 | edwardyu 2011 talkchess write-up of SF-inspired code | **(c)** midpoint of 0..8 = 4 |
| S097 | `SE_PLY_FACTOR` | 3 | "Lynx #1768 title prose" | **(c)** midpoint of 2..8 = 5 |
| S097 | `SE_MARGIN_PER_DEPTH` | "a few tens of cp at depth 8"; "CPW's flat 10..100 experimentation range" | no number; a wiki claim | **(c)** with the range stated by purpose: floor 1, top where the margin at the `SE_MIN_DEPTH` seed reaches two pawns, `2 * PAWN / 10` = 18 (P6), midpoint 9 or 10 -- say which. "10..100" is **not on the wiki page as fetched 2026-09-05**: drop it or mark `unverified` |
| S098 (F01) | `LMR_HIST_CLAMP` | 2 | "Weiss #451's 'between +2 and -2', PR prose" | **(b)** P5: half the table's reduction at the census median depth and last move index, rounded down -- 2 at today's table, the same integer Weiss quotes, which DEC-084 allows when the arithmetic is chesso's and stated. Fallback **(c)** midpoint of 0..4 = 2 |
| S098 | `LMR_BASE` / `LMR_DIVISOR` re-sweep seeds | "Obsidian 0.99/3.14, Weiss 1.35/2.75, Ethereal 0.7844/2.4696 (all CPW prose)" | engine constants on the wiki's LMR page | **delete the clause** -- DEC-105's PeSTO case exactly. The shipping 52 / 182 are S085's SPSA output over chesso's games, **(b)**, and the only seed a re-sweep needs |
| S098 | `LMR_HIST_DIV` | `3M / LMR_HIST_CLAMP`, M = 2^13 | own scale | keep, **(b)**; name `QUIET_HISTORY_MAX` (8192), and say the 3 counts S024's two continuation tables -- before S024 lands the sum is `[-M, +M]` and the divisor `M / LMR_HIST_CLAMP` |
| S098 | `LMR_CUTNODE`, `LMR_NOT_IMPROVING`, `LMR_TT_CAPTURE`, `LMR_PV` | 1 each | Lynx #1233 / #1135, Weiss #536/#666 + Lynx #1529, Weiss #71 | **(c)** each a 0..2 ply term, midpoint 1, off 0; strike the attributions from the seed lines (they stay in section 1 as records) |
| S098 | `LMR_MIN_MOVES_PV` vs `LMR_MIN_MOVES` | 5 vs 4 | "Weiss #71 started one later at PV" | `LMR_MIN_MOVES` 4 is chesso's own `legal_moves_counter > 3`, **(b)**; `LMR_MIN_MOVES_PV` **(c)** midpoint of 4..8 = 6 |
| S098 | `LMR_DEEPER_MARGIN`, `LMR_SHALLOWER_MARGIN` | "several tens of cp first" | none | **(c)** range 0..`PAWN` (0 off, one pawn top), midpoint 47 (P6) |
| S109 (F01) | `LMP_BASE`, `LMP_DEPTH_COEFF`, `LMP_MAX_DEPTH` | 0, 10, cap 3 | "Lynx #512 PR prose" (its shipped constants) | **(b)** P2. Off stays `LMP_BASE` = `MAX_MOVES` (270, `src/data_structures.hpp`) |
| S109 | `FUT_BASE`, `FUT_SLOPE`, `FUT_MAX_LMRDEPTH` | 100, 200, cap 6-8 | "CPW's minor/rook wording" read at pawn = 100 | **(a)** Heinz via https://www.chessprogramming.org/Futility_Pruning: depth 1 "should not exceed the value of a minor piece", depth 2 "more like the value of a rook" (Heinz 1998, fetched). In chesso's scale (P6) minor = `(KNIGHT + BISHOP) / 2` = 317, rook = `ROOK` 487, so `FUT_SLOPE` 170, `FUT_BASE` 147. Cap **(c)** midpoint of the declared range |
| S109 | `HP_COEFF`, `HP_MAX_DEPTH` | "first setting in `M/64..M/8`"; cap "3-4 ('low depths', Weiss #446)" | own scale; a Weiss phrase | coefficient **(c)** the region's arithmetic midpoint `9M/128` = 576 at `QUIET_HISTORY_MAX` 8192, said so; cap **(c)** midpoint of its declared range; strike the Weiss phrase |
| S109 (F01) | `SEE_QUIET_MAX_LMRDEPTH` | 9 | "SF a834bfe prose" | **(c)** declare 0..16 (0 off; 16 is above the median depth 11 the `RFP_MAX_DEPTH` comment in `src/search_params.hpp` records, where the gate stops binding), midpoint 8 |
| S109 | `SEE_QUIET_COEFF` | bar at lmrDepth 1 under a pawn of `see_value` | own scale | keep, **(b)**; cite `see_value` in `src/bitboard.cpp` without the line number |
| S113 (F01) | `PROBCUT_MARGIN` | 150 | SF, Weiss, Ethereal, Berserk averaged | **(b)** P1, Buro's regression over chesso's positions with the paper's threshold t = 1.0 (**a**, https://skatgame.net/mburo/ps/chessmpc.pdf: Figure 2 `#define T 1.0`; "the cut threshold 1.5 is no good") |
| S113 (F01) | `PROBCUT_DEPTH_OFFSET` | 4 | "SF 71cc01c, 'depth - 4 plies'" | **(a)** 4: the paper's single-pair implementation is depth pair (4,8) -- Figure 2 `#define S 4 // depth of shallow search`, `#define H 8 // check height`; Table 1 regresses (3,5) and (4,8). Same URL. The coincidence with Stockfish's 4 is irrelevant; provenance is the paper |
| S113 | `PROBCUT_MIN_DEPTH` | 5 | unstated; "SF prose allows 3 as of 2025" | **(a)** 8, the paper's check height for pair (4,8), so at the minimum depth the shallow search is the paper's 4-ply search; range 3..10 stands. Strike the SF sentence |
| S114 (F01) | `NULL_MOVE_EVAL_MARGIN` | 100 | "SF 074c7a3 prose: one pawn" | **(b)** one pawn in chesso's scale = `PAWN` = 94 (P6); DEC-134 decided this treatment by name |
| S114 (F01) | `NULL_MOVE_EVAL_CAP` | 3 | "SF 7ed15af prose: 'three plies'" | **(c)** midpoint of 0..16 = 8, stated as such -- what this step's own table says. Section 5 says why a midpoint is a poor seed for a safety term; section 10 puts P4 to the owner |
| S114 | `NULL_MOVE_BASE` | "seed 2, sweep 1..4 (ships today; CPW '2 or 3', Fruit 3, Heinz 2/3)" | Fruit's 3 is an engine value | **(b)** the shipping 3, S085's SPSA output; correct "ships today" to 3. Keep Heinz as **(a)** context for the range: "R=3 when normal search depth exceeds 6 plies and R=2 otherwise", https://www.chessprogramming.org/Depth_Reduction_R (Heinz 1999, fetched). Strike Fruit |
| S114 | `NULL_MOVE_DIVISOR` | "seed 6 (ships today; Lynx prose 'depth/5')" | Lynx's 5 | **(b)** the shipping 6, chesso's own; strike the Lynx clause |
| S132 (F01) | `TM_NODE_BASE_PCT`, `TM_NODE_SCALE_PCT` | 200, 100 | "the Lynx #1203 prose pair (base 2.0, scale 1.0)" | **(b)** P3; fallback **(c)** midpoints 250 of [100, 400] and 150 of [0, 300]. No (a): the wiki's Time Management page lists "the ratio of the size of the subtree under the best move versus the size of the whole search tree" with no method and no number (fetched) |
| S132 | `TM_NODE_MIN_DEPTH` | 5, "seeded beside ASPIRATION_MIN_DEPTH" | chesso's own, stale | **(b)** equal to `ASPIRATION_MIN_DEPTH` as compiled at the owning step's HEAD (2 today, not 5). The midpoint 32 of [0, 64] exceeds every depth chesso reaches and would switch the feature off: (c) refused here, and the refusal stated |

**P1 -- ProbCut regression (S113 `PROBCUT_MARGIN`).** Tune build. Over the
300 positions of the S021 stratified pick, `go depth d'` and `go depth d` with
`d' = PROBCUT_MIN_DEPTH - PROBCUT_DEPTH_OFFSET` and `d = PROBCUT_MIN_DEPTH`
(4 and 8 at the seeds), reading `score cp` from the last `info` line. Drop a
position where either score is a mate score or `|v'| > 3 * PAWN` -- the
paper's exclusions ("We only used v' data points in the range [-300, 300]";
mate scores excluded because a 4-ply search misses a mate an 8-ply search
finds "roughly once every 1000 positions"). Least-squares `v = a*v' + b`,
`sigma` = standard deviation of the residuals; seed `PROBCUT_MARGIN =
round(t * sigma)`, t = 1.0; record `a`, `b`, `sigma`, the surviving and
excluded counts in the S113 stamp. The file's `probBeta = beta + margin` is
the paper's `(t*sigma + beta - b)/a` at a = 1, b = 0; the regression says how
far chesso is from that. The paper found a = 0.998 to 1.11, b = -7.0 to 2.36,
sigma = 51.8 to 82.0 at pawn = 100 over about 2700 Crafty positions -- expect
that order in chesso's scale and do not seed from it. Fewer than 200
surviving positions is a finding, not a seed: widen the set.

**P2 -- late-move cutoff census (S109 `LMP_*`).** Tune build, instrumented
under `#ifdef CHESSO_TUNE` only: at every beta cutoff on a quiet move in
`negamax` in `src/search.cpp`, record `(depth, legal_moves_counter)`. Run `go
depth 10` over the 300 positions. Per remaining depth 1..8 take the 95th
percentile of the cutoff index -- the count past which 19 of 20 quiet cutoffs
have already happened, which is what "late enough to skip" means (the
percentile is this guide's choice; the sweep decides). Least-squares
`threshold = LMP_BASE + LMP_DEPTH_COEFF * depth` over depths 1..3; seed
`LMP_MAX_DEPTH` as the largest depth at which the fitted threshold is below
the median number of quiet moves generated at that depth in the same census
(a rule that never binds is not measurable). Record the percentile table in
the S109 stamp.

**P3 -- node-fraction census (S132 `TM_NODE_*`).** Release build with S132's
counting half landed (behaviour-neutral, and first in its own sketch). Over
the 300 positions at `go depth 12` record the best move's share `f` of
root-child nodes. The file's factor is `scale * (base - f)`; in the X-macro's
percent units, `factor_pct = (TM_NODE_BASE_PCT - 100 f) * TM_NODE_SCALE_PCT /
100`. Two constraints: `factor_pct` = 100 at the census median `f_med`, so
the seed spends today's allocation in expectation and the SPRT measures
redistribution, not a longer clock; and `factor_pct` = `TM_SCALE_MIN_PERCENT`
(30) at `f` = 1, chesso's own floor for the scaled soft limit. Solving:
`TM_NODE_SCALE_PCT = 7000 / (100 - 100 f_med)`, `TM_NODE_BASE_PCT = 100 +
3000 / TM_NODE_SCALE_PCT`, rounded to integers -- 140 and 121 at `f_med` =
0.5. Record `f_med` and the quartiles in the S132 stamp. If the census cannot
run before the multiplier is written, take the (c) midpoints and say so.

**P4 -- measured bound for a safety cap (S114 `NULL_MOVE_EVAL_CAP`,
alternative).** Tune build with the eval term landed. Sweep the cap downward
from the range top; record the largest value at which `TEST_SUITE("engine:
mate safety")` in `tests/test_engine.cpp` and `TEST_CASE` "pruning does not
hide a forced mate" in `tests/test_search.cpp` both pass, and seed one below
it. This is the "measured" bound kind `src/search_params.hpp`'s header
defines, the kind `RFP_MIN_PLY`'s floor is. Offered as the alternative to
the midpoint (section 10).

**P5 -- fraction of the reduction table (S098 `LMR_HIST_CLAMP`).** Read
`search_lmr_reduction_probe(depth, move)` in `src/search.cpp` at the census
median depth (11 at the S085 control per the `RFP_MAX_DEPTH` comment;
re-read at the owning step's HEAD) and the table's last move index, 63.
Today: `0.52 + ln(11) * ln(63) / 1.82` = 5.98, stored floored as 5; half,
rounded down, is 2. Write the arithmetic into the section so the number is
re-derived when `LMR_BASE` / `LMR_DIVISOR` move.

**P6 -- units.** "One pawn", "a minor", "a rook" convert with `piece_value`
in `src/eval_tables.hpp` when the margin is compared against `evaluate()`
(futility, razoring, ProbCut, the null-move eval term, LMR re-search
margins): `PAWN` 94, minor `(KNIGHT + BISHOP) / 2` = 317, `ROOK` 487, `QUEEN`
716; N pawns is `N * PAWN`. A threshold compared against a SEE result (S091,
S109's quiet SEE, S112's victims) uses `see_value` in `src/bitboard.cpp`,
pawn 100. Write the conversion beside the number ("`FUT_SLOPE` 170 = `ROOK` -
minor") so it is re-derived when the material scale is refit. State once per
file that `psqt_mg` / `psqt_eg` are degenerate with `piece_value`, so the
material term alone is the unit: the tables' mean over the pawn's ranks is
about 8 in the middlegame and 76 in the endgame at the S028 fit, which is why
a pawn measured by removing one from a position is not 94. Plies have no
unit; a ply count quoted from an engine is that engine's constant and takes
form (c).

### 5. Interactions and traps

- **A midpoint is a poor seed where the range top is "off" and the term is a
  safety cap.** `NULL_MOVE_EVAL_CAP` at 8 lets a large static lead take eight
  extra plies off the null-move depth; S114's own `accepts` says an uncapped
  term "re-creates the mate-hiding bug this engine has already shipped twice"
  and demands the cap be observed load-bearing. Write the midpoint, as this
  step's table says, and beside it that S114's mate instruments decide the
  seed's admissibility before any sweep, with P4 as the alternative. Do not
  pick P4 silently: section 10.
- **Two pawns.** The 2026-08-19 sections read the wiki's "minor" and "rook"
  as 300 and 500 -- `see_value`'s scale -- for margins compared against
  `evaluate()`. P6 turns `FUT_BASE` / `FUT_SLOPE` from 100 / 200 into 147 /
  170. Every margin in the seven files gets "compared against which scale"
  answered in the text.
- **The wiki republishes engines.** The LMR page's six formulas are all named
  engines' ("The page itself proposes no independent formulas", fetched); the
  Razoring page reproduces a Stockfish formula S116 already declines; the Null
  Move page's "R=3" and eval gate are Fruit's. The wiki's *own* statements
  used here are three: IIR's "depth > 5, say", Futility's Heinz margins,
  Depth_Reduction_R's Heinz rule.
- **Stale shipping values inside the sections.** `NULL_MOVE_BASE` "2 ships
  today" (S114) and `TM_NODE_MIN_DEPTH` "5 beside ASPIRATION_MIN_DEPTH" (S132)
  are S184's class (F05) but sit in the sections this step rewrites: fix them
  here, stamp it, tell the coordinator so S184 does not look for them. Once
  S184 extends `tools/plan_prose_check.py --params` to `adocs/plan_todo/`
  with constant-name phrases (`PARAM_PHRASES`), a wrong shipping value here
  is a red fast test.
- **`HISTORY_MAX` does not exist**; the symbol is `QUIET_HISTORY_MAX`, and the
  `[-3M, +3M]` sum assumes S024's continuation tables have landed. Say both.
- **Claims about the wiki it does not make today.** S097 says the Singular
  Extensions page records a "flat 10..100" margin space and a "depth/2 up to
  depth-N" verification-depth space; neither is on the page as fetched
  2026-09-05. Mark `unverified` where touched; the S097 enricher re-fetches.
- **Anti-seeds are records**, allowed by DEC-019 as direction, not seeds; in
  a seeds section they trip check 1. Move them to Pitfalls.
- **`plan_done/` is history**: S085 and S138 state older values and are not
  edited.
- **The required line's date is 2026-09-04**, the decision's, quoted in
  `accepts`; do not "correct" it.

### 6. Tests

No code changes, so no guard test, mutant, golden, INV-6 run or Debug
self-play (DEC-141's second tier is for steps touching the search). The
checks that prove `accepts`:

1. **No engine named in any seeds section.** Expected output: nothing.
   ```
   for f in adocs/plan_todo/S095_*.md adocs/plan_todo/S097_*.md adocs/plan_todo/S098_*.md \
            adocs/plan_todo/S109_*.md adocs/plan_todo/S113_*.md adocs/plan_todo/S114_*.md \
            adocs/plan_todo/S132_*.md; do
     awk '/^### 4\. Constants and seeds/,/^### 5\. Pitfalls/' "$f" \
       | grep -nE 'Stockfish|\bSF\b|Weiss|Lynx|Ethereal|Berserk|Stash|Koivisto|Fruit|Obsidian|Halogen|Senpai|Leorik|Rebel|Crafty|Komodo|Stormphrax|Seer|Yace|#[0-9]{2,}|[0-9a-f]{7,}' \
       | sed "s|^|$f: |"
   done
   ```
   `Crafty` and `Yace` are listed because P1's paper experimented on them:
   the seeds section names the *paper*; its Crafty numbers go in a "do not
   seed from these" sentence in Pitfalls.
2. **The two rule sentences are gone**: `grep -n 'legal seed'
   adocs/plan_todo/S113_*.md adocs/plan_todo/S114_*.md` prints nothing.
3. **The required line, once per file**: `grep -c 'seeds re-derived
   2026-09-04 under DEC-105 (DEC-134)'` over the seven prints 1 each.
4. **Every remaining seed states its form**: each constant line carries
   "(a)", "(b)", "(c)" or the words literature, derivation, midpoint or off
   value beside its value. No checker; the reviewer's read.
5. **The checkers**: `python3 tools/plan_prose_check.py --touches | tail -1`
   prints `touches flagged: 0`; `python3 tools/plan_prose_check.py --params`
   exits 0. Today `--params` reads only its `PARAM_DOCS` (`adocs/specs.md`,
   `MANUAL.md`, `DEV_MANUAL.md`, `adocs/plan.md`) and cannot see the seven
   files; run it because `accepts` names it, knowing S184 is what extends it.
6. **The fast suite**, which carries `test_plan_touches`
   (`tests/CMakeLists.txt`) over the pending files: the gate command below.

### 7. Measurement

None. The step alters no behaviour, so neither INV-6 nor an SPRT applies, and
`excludes` forbids running any derivation here. What it leaves is measurable
later: each owning step's stamp records its procedure's output (P1's `a`,
`b`, `sigma`; P2's percentile table; P3's `f_med`), which is how a future
reader tells a derived seed from a number.

### 8. Completion checklist

- Gate, both builds, before the commit (AGENTS.md TESTS):
  `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`.
  If `tools/gate.sh` exists at your HEAD (S189, Open entry 4, adds it), run
  that; it does not exist at `bdb5d56`.
- Section 6's checks, outputs of 1 to 3 pasted into the stamp.
- **No `Bench:` line and no `No functional change` trailer**: the commit
  touches no `src/`; DEC-140's trailer is for commits that do.
- **One commit for all seven**, not seven: `accepts` is one unit -- the F01
  table walked in one stamp -- and every commit must be green, so one gate
  run instead of seven for a change that cannot break a build. Subject under
  72 characters, imperative ("Reseed the seven block-1 search steps under
  DEC-105 (S180)"); body: the F01 rows in order, one line each with the
  replacement; then the inventory's extra rows; then the stale values fixed
  in passing.
- Stamp: date; F01 rows with form letter and value or procedure; extra rows;
  the two deleted sentences quoted; stale values corrected; check outputs;
  "verified in place" for any file DEC-145's pass had already reseeded.
- Docs: `DEV_MANUAL.md` and `MANUAL.md` need nothing (no command, no option
  added) -- say so in the stamp. `adocs/specs.md`'s Non-goals already carries
  "Another engine's constants are never seeds, and republication does not
  change their origin"; no edit.
- `adocs/plan.md` (out of Open, into the last-five Done) and
  `adocs/status.md` go through the coordinator (AGENTS.md PLAN).
- The F01 `Status:` line in `adocs/audit/2026-09-04_plan_review.md` stays
  `planned -- S180`; it moves to `closed` at the next audit's re-measurement,
  not in this commit.
- Move the file to `adocs/plan_done/` last, after the stamp.

### 9. Sources read

- `adocs/decisions.md` `## DEC-084`, `## DEC-105`, `## DEC-134`, `## DEC-135`,
  `## DEC-120`, `## DEC-145`; `adocs/audit/2026-09-04_plan_review.md`
  `### 2026-09-04_plan_review-F01`; the seven files' sections 1, 4 and 8 at
  `bdb5d56`; S091, S112, S116 section 4; `adocs/plan_todo/S184_*.md`.
- `src/search_params.hpp`, `src/eval_tables.hpp`, `src/bitboard.cpp`
  (`see_value`, `see_ge`), `src/search.cpp` (`lmr_reduction`,
  `search_lmr_reduction_probe`, `negamax`, `legal_moves_counter`),
  `src/data_structures.hpp` (`MAX_MOVES`), `tools/plan_prose_check.py`
  (`PARAM_DOCS`, `PARAM_PHRASES`), `tests/CMakeLists.txt`
  (`test_plan_touches`), `adocs/data/S021_aspiration_sweep.py`,
  `adocs/data/S018_raw.tsv`, `adocs/data/README.md`.
- https://www.chessprogramming.org/Internal_Iterative_Reductions -- "only use
  IIR if depth > 5, say", the page's own example; no amount stated.
- https://www.chessprogramming.org/ProbCut -- Buro's model and cut condition;
  Jiang 2003's table (pairs (3,5), (4,8); sigma 51.8 to 82.0) reproduced
  from the paper; pseudo-code threshold "T(1.5)".
- https://skatgame.net/mburo/ps/chessmpc.pdf -- Jiang and Buro, ACG 10,
  fetched and text-extracted locally: Figure 2 `S 4`, `H 8`, `T 1.0`; about
  2700 positions at depths 1..10, `|v'| <= 300`, mate scores excluded; Table
  1; thresholds 1.0 and 1.5 tried, "1.5 is no good"; MPC pairs (2,6), (3,7),
  (4,8), (3,9), (4,10); ProbCut beside null move "no better nor worse";
  59.4 % in 64 games.
- https://skatgame.net/mburo/ps/probcut.pdf -- Buro 1995: fetched, bitmap
  fonts, not readable locally, **unverified**; not needed, the chess paper
  carries the pairs.
- https://www.chessprogramming.org/Singular_Extensions -- no margin, depth
  or reduced-depth number on the page; papers listed without URLs.
  https://dl.acm.org/doi/abs/10.1016/0004-3702(90)90073-9 and
  https://journals.sagepub.com/doi/abs/10.3233/ICG-1988-11402 -- the 1990 and
  1988 papers, paywalled, parameters **unverified**.
- https://www.chessprogramming.org/Depth_Reduction_R -- Heinz 1999: "R=3 when
  normal search depth exceeds 6 plies and R=2 otherwise".
- https://www.chessprogramming.org/Null_Move_Pruning -- the eval-scaled
  factor sentence, no number; "R=3" and the eval gate are Fruit's.
- https://www.chessprogramming.org/Futility_Pruning -- Heinz's minor / rook
  margins; Move Count Based Pruning stated without a threshold.
- https://www.chessprogramming.org/Late_Move_Reductions -- six engine
  formulas; "reduce less based on the history value", no number.
- https://www.chessprogramming.org/Time_Management -- the subtree ratio named
  as a consideration, no method, no number.
- https://www.chessprogramming.org/Static_Exchange_Evaluation -- "linear depth
  margin for captures, and a quadratic depth margin for quiets", no numbers.

### 10. Questions deferred to the owner

1. `NULL_MOVE_EVAL_CAP`: this step's own table says range midpoint, 8 of
   0..16 -- near the off end for a term whose job is mate safety. The
   alternative is P4, a measured bound in `src/search_params.hpp`'s own
   taxonomy. Which does S114 seed from? The guide writes the midpoint with P4
   beside it; the choice does not change `accepts`.
2. The inventory holds more engine-originated or unformed seeds than F01's
   eleven rows (`SE_PLY_FACTOR`, `SE_MARGIN_PER_DEPTH`, S098's four ply terms,
   `LMR_MIN_MOVES_PV`, the S098 re-sweep clause, `PROBCUT_MIN_DEPTH`,
   `NULL_MOVE_BASE`'s Fruit clause, `NULL_MOVE_DIVISOR`'s Lynx clause,
   `TM_NODE_MIN_DEPTH`, `HP_MAX_DEPTH`'s Weiss phrase, `FUT_*`'s scale).
   `accepts` covers them ("every seed"); its "F01 table walked row by row"
   clause does not name them. Confirm the stamp lists both, or amend
   `accepts` to say so.
3. S114's stale `NULL_MOVE_BASE` "2 ships today" is in no step's `accepts`
   (S184's touches omit S114). The guide has S180 fix it in passing; if the
   owner prefers S184, its touches gain the file.
