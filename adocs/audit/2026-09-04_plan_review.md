# Audit 2026-09-04 — plan_review

Type: `plan_review`.
Commit as briefed: `18deccfd16928db325abd864d63e7f669ac829ea`.
Report id stem: `2026-09-04_plan_review`.

## Scope, and what this pass did

In scope and examined: `adocs/plan.md` (ordering, Open list, prose, cost and
Elo arithmetic); all 54 pending step files in `adocs/plan_todo/`
(`adocs/plan_current/` is empty); `adocs/status.md`; `adocs/specs.md`; and
the `adocs/decisions.md` entries that constrain a pending step (DEC-054,
DEC-084, DEC-087, DEC-089, DEC-092, DEC-095, DEC-097, DEC-105, DEC-108,
DEC-112, DEC-128, DEC-132). Reviewed along the three axes the brief names:
against the published record, against the code at HEAD, against the
project's own rules. The four earlier plan_review reports and the
plan-relevant findings of the adversarial reports were re-measured from their
own reproductions; verdicts at the end.

Evidence standard, as set for this run: every literature or engine claim
below is traced to a URL fetched during this run (listed in the last section)
or to a repository path; anything else is written as `unverified`. Every code
claim carries `file:line` at `18deccf`. Every number is reproduced by a
command whose output is quoted, or named as deferred.

What was run: `cmake --build build -j8` and `ctest --test-dir build -L fast`
(27/27 passed, 62.99 s); `cmake --build build-tune -j8` and
`ctest --test-dir build-tune -L fast` (27 tests, no failure listed);
`./clang-format.sh --check` (clean); `python3 tools/search_bench.py
./build/src/chesso 9` and `12`; `tools/plan_prose_check.py` in all four
modes; a citation-drift census over `plan_todo/`; `grep`/`sed`/`awk` over
`src/`, `tests/`, `tools/` and `adocs/`; `curl` against
computerchess.org.uk, chessprogramming.org and api.github.com (unauthenticated,
32 of 60 calls left at the end). No match, SPSA run or fit was started.
Nothing outside this file was written; scratch files live under the session
scratchpad, outside the tree.

## Repository state during the audit

HEAD `18deccf`, branch `achesso`, tree clean apart from this report
(`git status --short` shows only `?? adocs/audit/2026-09-04_plan_review.md`).
HEAD did not move while the report was written.

```
$ python3 tools/search_bench.py ./build/src/chesso 9
  midgame      0.027s       121512 nodes      4568 knps  best c3d5
  kiwipete     0.103s       800769 nodes      7775 knps  best e2a6
  tactical     0.006s        62907 nodes     10154 knps  best d7c8q
$ python3 tools/search_bench.py ./build/src/chesso 12
  midgame      0.064s       639228 nodes      9996 knps  best c3d5
  kiwipete     0.381s      3430710 nodes      8995 knps  best e2a6
  tactical     0.037s       367858 nodes      9820 knps  best d7c8q
```

Both baselines `specs.md` and `status.md` record reproduce to the node.

```
$ python3 - (Open list against plan_todo/)
open entries 54 todo files 54
dups []  in list not in todo []  in todo not in list []
done recently ['S177', 'S176', 'S175', 'S174', 'S146']  all in plan_done/
```

Finding format: every finding carries `Status: open`, an id
`2026-09-04_plan_review-F<nn>`, Evidence, Impact and a Suggested resolution
that is stated and not applied.

## Findings

Ten findings: no high, five medium, five low.

---

### 2026-09-04_plan_review-F01  medium  Seven pending search steps seed constants from other engines' constants quoted in commit-message prose, which DEC-105 rules out by origin

Status: planned — S180

**Evidence.** DEC-105 (`adocs/decisions.md:5982`, 2026-08-22): *"A number
may seed a fit or an SPSA run only if it originates in a publication about
the technique -- a paper, an article, the wiki's own derivations and example
formulas. A number that originates as another engine's tuned output is never
a seed, wherever it is republished."* Its `Consequences:` lists `plan.md`,
`CLAUDE.md`, `AGENTS.md` and `specs.md` as carrying the provenance form; the
enriched step files are not in that list, and their seed sections are dated
2026-08-19, three days before the decision.

The seed sections that take an engine's constant from that engine's own commit
message or PR body, verified at HEAD:

| step | seed | stated origin |
|---|---|---|
| `S114_null_move_refinements.md:146` | `NULL_MOVE_EVAL_MARGIN` 100 | "SF 074c7a3 prose: one pawn" |
| `S114_null_move_refinements.md:148` | `NULL_MOVE_EVAL_CAP` 3 | "SF 7ed15af prose: 'three plies'" |
| `S113_probcut.md:175` | `PROBCUT_MARGIN` 150 | SF "rbeta = beta + 200", Weiss "beta + 200", Ethereal 80 to 100, Berserk 110 |
| `S113_probcut.md:186` | `PROBCUT_DEPTH_OFFSET` 4 | "SF 71cc01c, 'depth - 4 plies'" |
| `S132_time_management_node_fraction.md:140` | `TM_NODE_BASE_PCT` 200, `TM_NODE_SCALE_PCT` 100 | "the Lynx #1203 prose pair (base 2.0, scale 1.0)" -- constants Lynx itself retuned to 2.4/1.65 in #1206 |
| `S098_reduction_refinement.md:243` | `LMR_HIST_CLAMP` 2 | "Weiss #451's 'between +2 and -2', PR prose" |
| `S109_shallow_depth_pruning_block.md:277` | `LMP_DEPTH_COEFF` 10, `LMP_MAX_DEPTH` 3 | "Lynx #512 PR prose" (`i >= depth * 10`, max 3 -- fetched, it is Lynx's shipped constant) |
| `S109_shallow_depth_pruning_block.md:291` | `SEE_QUIET_MAX_LMRDEPTH` 9 | "SF a834bfe prose" |
| `S097_singular_extensions.md:231,234` | `SE_MIN_DEPTH` 8, `SE_TT_DEPTH_MARGIN` 3 | "SF prose 8e823459"; a talkchess write-up of SF-inspired code |
| `S095_internal_iterative_reduction.md:144` | `IIR_MIN_DEPTH` 4 | "Lynx #507's introduction value" -- where CPW's own page offers "depth > 5, say" |

Two of the files state the rule they followed in so many words:
`S114_null_move_refinements.md:26` *"numbers quoted from message prose are
legal seeds, the S113 pass's precedent"*; `S113_probcut.md:26` *"their numbers
are legal seeds (DEC-084)"* -- DEC-084's venue reading, which DEC-105 amended
because *"a venue rule launders exactly what the provenance rule refuses"*.

The clean cases, for contrast, show the rule is satisfiable: S112 seeds its
margin from CPW's "typically around 200" (`S112_quiescence_move_futility.md:202`) and its victim
table from chesso's own `see_value`; S116 seeds 300 from CPW's "~three pawns"
and explicitly declines the SF snippet CPW reproduces; S091 and S109 derive
`SEE_CAPT_COEFF` and `HP_COEFF` from chesso's own scale.

**Impact.** Seven of the fifteen search-block steps would start their sweep
or SPSA from another engine's constant. DEC-105's own reasoning is the
impact: *"a seeded-then-refit vector converges near its seed by
construction, so 'we refit it' is the defence that already failed"*. The
constants in question are margins and caps in centipawns and plies -- the
class most likely to survive a fit unchanged -- and the steps mark them
"seed -- must be fitted", which is DEC-084's rule and not DEC-105's. Every
one of them is marked as such in the files an implementer reads first, so
this is a breach that will happen by following the plan, not by ignoring it.

**Suggested resolution.** Re-read the seven "Constants and seeds" sections
against DEC-105. For each engine-originated number: take the open-literature
value where one exists (CPW gives "depth > 5" for IIR, "~200" for the delta
margin, "~three pawns" for razoring, minor/rook for futility), derive from
chesso's own scale as S091, S109 and S112 already do, or start the sweep from
the parameter's off value or range midpoint and let the fit find it. Where the
owner wants an engine constant anyway, DEC-105 names the form: a per-case
recorded exception "in the DEC-087 Fathom style". Delete the sentence at
`S114:26`, which states the superseded rule as current.

---

### 2026-09-04_plan_review-F02  medium  DEC-087's reason for keeping S099 in the main order and demoting S110/S111 rests on a Lynx rating band the CCRL list contradicts by ~400 Elo

Status: planned — S181

**Evidence.** `adocs/decisions.md:5015-5017` (DEC-087 (b)): *"Non-pawn and
continuation correction history measure +3 to +8 only above ~3100. **S099
stays**: pawn correction history measured +11.4 at ~2850 (Lynx), the one
correction table with sub-3000 evidence."* Repeated at `adocs/plan.md:130-131`
and `adocs/plan_todo/S099_correction_history.md:98`, which traces the +11.4 to
Lynx PR #1662.

Fetched this run:

```
https://api.github.com/repos/lynx-chess/Lynx/pulls/1662
  merged 2025-04-15T22:16:01Z  "Pawn correction history / corrhist"
  Elo 11.35 +- 5.16, 8.0+0.08s, 7502 games
https://api.github.com/repos/lynx-chess/Lynx/releases/tags/v1.9.0   published 2025-03-11
https://api.github.com/repos/lynx-chess/Lynx/releases/tags/v1.10.0  published 2025-06-29
https://computerchess.org.uk/ccrl/404/rating_list_all.html  (list computed 2026-08-28)
  Lynx 1.5.0 2819 | 1.6.0 2926 | 1.7.0 3119 | 1.8.0 3139 | 1.9.0 3226 | 1.10.0 3293 | 1.11.0 3364
```

So the +11.4 was measured on a Lynx between v1.9.0 and v1.10.0, rated
**3226 to 3293** on the scale the plan's target is stated in -- above the
"~3100" that sent S110 and S111 to the reserve, not below 3000. The Lynx
README the S097 enrichment flagged (`https://raw.githubusercontent.com/lynx-chess/Lynx/main/README.md`,
rows for 1.8.0 to 1.11.0) gives the same CCRL Blitz figures (3144, 3225,
3293, 3363).

The same mis-banding recurs where DEC-087-era text places Lynx v1.8.0
(2024-12-20, CCRL 3139): `S098_reduction_refinement.md:63` *"Lynx #1233 +9.34
(v1.8.0, 2024, high-2800s)"* and `:104-105`, which files cutnode/improving/PV
under "Sub-3000 evidence" on that band. `S097_singular_extensions.md` recorded
the discrepancy as a scope concern on 2026-08-19 ("may sit ~300 low"), and
`status.md` parks it as an owner question; nothing has moved since.

**Impact.** DEC-087 (b) uses one criterion -- evidence below 3000 keeps a
step in the main order, evidence above ~3100 demotes it -- and on the public
record that criterion does not separate S099 from S110/S111: all three
tables' Lynx evidence is above 3100. S099 may still deserve its place (its
origin term measured +1.74 at Caissa and Stockfish adopted it; Lynx #1663
measured the pure pawn key at +12.09 alone -- both fetched), but the recorded
reason is false against the list the target is measured on, and S098's
verdict-2 "sub-3000" grouping is built on the same band. Every Lynx record
from v1.7.0 (2024) onward that a step calls "high-2800s" or "~2850" is 200 to
400 Elo higher on CCRL.

**Suggested resolution.** Amend DEC-087 with a dated Lynx release-to-CCRL
table (the seven rows above are enough) and re-band every Lynx record the
enriched steps cite by merge date. Then re-decide S099 against S110/S111 on
the corrected band, stating the real reason if S099 stays. S098's split at
`:104-105` moves with it.

---

### 2026-09-04_plan_review-F03  medium  "What this costs" prices a verdict at 45 to 75 minutes; the seven verdict runs since S105 averaged 4 h 40 m

Status: planned — S182

**Evidence.** `adocs/plan.md:348-352`: *"roughly 45 to 55 SPRT verdicts ...
At the S105 settings a typical verdict is 45 to 75 minutes, so **roughly 75
to 110 machine-hours**"*. The runs recorded in `plan_done/` since S105 landed
the regime (2026-08-20):

| run | wall | games | bounds | source |
|---|---|---|---|---|
| S093 verdict 1 | 2 h 44 m | 6412 | 0/5 H1 | `adocs/plan_done/S093_history_malus_and_ageing.md:794` |
| S093 verdict 2 | 6 h 35 m | 15398 | 0/5 H0 | `...S093_history_malus_and_ageing.md:696` |
| S107 | 1 h 37 m 52 s | 3812 | -5/0 H1 | `adocs/plan_done/S107_killers_for_checking_quiets.md:270` |
| S149 | 1 h 05 m | 2522 | -5/5 H0 | `adocs/plan_done/S149_killer_slot_dedupe.md:183` |
| S108 | 5 h 26 m 38 s | 12774 | -5/0 H1 | `adocs/plan_done/S108_static_eval_at_every_node.md:20` |
| S165 | 7 h 58 m 07 s | 18598 | -5/0 H1 | `adocs/plan_done/S165_nmp_mate_band_guard.md:17` |
| S130 | 7 h 12 m, no verdict | 16784 | 0/5 | `adocs/plan_done/S130_quiescence_tt_score_stand_pat.md:724` |

Mean 4 h 40 m, median 5 h 26 m. Two of seven fell inside the plan's 45-to-75
minute window. At the plan's own count of 45 to 55 verdicts the ledger gives
**210 to 256 machine-hours**, against the stated 75 to 110, before the two
SPSA nights, the datagen nights and S152's two gauntlets. `plan.md:355-361`
already records that S093 "cost more than that estimate" and leaves the total
standing.

The reason is in the plan's own bounds rule: DEC-063 says a true effect near
+3 crawls at `elo0=0 elo1=5` and a non-regression pair runs to the wall on a
true zero. Of the pending verdicts, only S109's block is priced in the +40 to
+120 class the 45-minute figure needs; the rest are +5-class effects at
exactly the pairs that produced the table above.

**Impact.** The cost line is what the owner and DEC-112's machine-scope
reasoning schedule against. A plan that says 75 to 110 hours and costs 210 to
256 buys different decisions about which nights to spend and which steps to
keep. Nothing here changes any step's content; the number is wrong by 2 to 3x
against evidence the plan itself records.

**Suggested resolution.** Restate the cost line from the ledger: the mean
wall per verdict over the runs since S105, times the verdict count, with the
per-class split (block-sized effects at the fast end, +5-class effects at the
slow end). Re-derive it in the completing commit of every step that lands a
verdict, the way `status.md` is rewritten.

---

### 2026-09-04_plan_review-F04  medium  The Elo arithmetic's inputs are recorded nowhere, and the one discount it applies is not the one the project's ledger has measured since it was written

Status: planned — S183

**Evidence.** `adocs/plan.md:365-367`: *"Discounting self-play to list Elo
at the ratio the published per-release records support (~60 % sticks), and
taking a fifth off for interaction: search +180 to +280, evaluation +90 to
+160, speed +40 to +90, tuning +50 to +90, and S133 is +30 to +60 of margin on
top. The midpoint clears 3000."*

```
$ grep -n "180 to +280\|60 % sticks\|fifth off\|+180 to" adocs/decisions.md adocs/plan_done/*.md
(no output)
```

The per-block ranges appear in `plan.md` only; the per-step published figures
they were summed from, and the sum, are in no tracked document, so the
arithmetic cannot be re-derived or checked. The two discounts applied are
self-play-to-list (~60 %) and interaction (a fifth). No published-to-measured
discount is applied, although `plan.md:30-32` (DEC-019) says a published
figure "decides what to try and never what to conclude".

What that ratio has measured since the arithmetic was written (2026-08-19),
each from `specs.md`'s search row and the step's `plan_done/` stamp:

| step | published | measured here | ratio |
|---|---|---|---|
| S093 verdict 1 (history malus/gravity) | +37.49 Weiss, +28.0 Lynx | +10.73 +/- 6.70 | 0.29 to 0.38 |
| S093 verdict 2 (history persists across `go`) | +12.5 Lynx | -1.65 +/- 4.22, reverted | wrong sign |
| S130 (TT score as stand pat) | +10.8/+12.1 Weiss | +1.14 +/- 4.04, no verdict, kept as zero | ~0.1 |
| S149 (CPW killer replacement rule) | published practice | -11.02 +/- 10.53 | negative |
| S108 (static eval every node, improving substrate) | Lynx +4.64-class inputs | +2.28 +/- 4.40, no gain claimed | -- |

Arithmetic on the plan's own ranges: the low end sums to +390 and lands at
2949, which does not clear 3000; the midpoint (+535) lands at 3094. Applying
the largest transfer ratio measured since the arithmetic was written (0.38)
to the high end (+680) gives +258 and 2817. That is arithmetic and not a
measurement, and it is offered only to show the sentence's sensitivity to a
discount it does not apply.

**Impact.** "The midpoint clears 3000" is the plan's reachability claim for
phase one and it is unverifiable from the repository: its inputs are not
recorded, and the discount it omits is the one the project has measured four
times since, at 0 to 0.38. The plan already says the arithmetic is "a range
and not a forecast", which is honest about its status and silent about its
derivation.

**Suggested resolution.** Record the inputs -- per step, the published figure
and the block sum -- in `plan.md` or an `adocs/data/` table so the arithmetic
can be checked, and either apply the ledger's published-to-measured ratio as a
third discount or delete "The midpoint clears 3000" and leave the range as a
range. Re-derive it when S024 and S109 land, which are the two largest
published inputs.

---

### 2026-09-04_plan_review-F05  medium  S115's baseline is the pre-S085 aspiration triple and its table says to keep a value S085 moved; four other pending documents state retuned parameters at their old values

Status: planned — S184

**Evidence.** The shipped values at HEAD:

```
$ grep -n "ASPIRATION_\|MAX_QSEARCH_DEPTH\|LAZY_EVAL_MARGIN" src/search_params.hpp | grep "X("
106:  X(MAX_QSEARCH_DEPTH, "MaxQsearchDepth", 19, 1, 64)
200:  X(LAZY_EVAL_MARGIN,  "LazyEvalMargin",  184,    0, 2000)
240:  X(ASPIRATION_MIN_DEPTH, "AspirationMinDepth", 2,   2, 64)
241:  X(ASPIRATION_DELTA,     "AspirationDelta",    21,  1, 2000)
242:  X(ASPIRATION_MAX_DELTA, "AspirationMaxDelta", 437, 1, 48000)
```

(S085's vector, `21b4a21`, 2026-08-21; `specs.md` states all five.) The
pending documents:

- `adocs/plan_todo/S115_aspiration_refinements.md:14` *"`ASPIRATION_DELTA` is
  50 and widening doubles"*; `:62` *"Chesso's gate of 5 was measured here
  (S021) and stands"*; `:124-126`, the constants table: *"depth gate | 5,
  measured (S021) | ... keep 5 unless the sweep says otherwise"*, *"initial
  delta | 50 (S021)"*, *"max delta escape | 400 then full | keep; re-sweep
  confirms"*. All three are the S021 values S085 replaced with 2 / 21 / 437.
- `adocs/plan_todo/S082_datagen_qsearch_leaf_labels.md:48` *"`MAX_QSEARCH_DEPTH`
  is 8 (`src/search.cpp:27`)"* -- it is 19, and `src/search.cpp:27` is a
  comment naming the parameter.
- `adocs/plan_todo/S120_eval_cache.md:21` and `:187`, `adocs/plan_todo/S122_king_safety_rebuild.md:15`
  and `adocs/plan.md:159`: the lazy clamp is "150 centipawns"; it has been 184
  since S085 and `specs.md` says so.
- `adocs/plan_todo/S118_pawn_hash_table.md` "What it costs today": 83.35 ns a
  call, 5.8 M nodes a second -- the pre-S104 binary; `specs.md` records 53.90
  ns since S104.

The guard S150 built does not reach any of these: `tools/plan_prose_check.py:834-843`
keys four aspiration phrases against `PARAM_DOCS = ("adocs/specs.md",
"MANUAL.md", "DEV_MANUAL.md", "adocs/plan.md")`, and `plan.md:159`'s sentence
is not among the phrases. `python3 tools/plan_prose_check.py --params` exits 0
at HEAD with every line above in place.

**Impact.** S115's sweep design starts from a triple that does not ship, and
its table's "keep 5" instruction, followed, reverts `AspirationMinDepth` from
the 2 that S085's +21.02-verified vector shipped back to 5 -- a silent undo
of a measured axis inside a step whose verdict would then be attributed to
the window refinements. S082's "trap" paragraph reasons from a quiescence cap
of 8 that is 19. S120 and S122 size their argument on a clamp 34 centipawns
narrower than the one that ships; `plan.md` states it in the present tense
where `specs.md`, which outranks it, says 184.

**Suggested resolution.** Rewrite S115's "What is there" and its table against
HEAD, with the sweep's off row taken at the shipped 2 / 21 / 437; correct
S082:48, S120:21,187, S122:15, S118's cost paragraph and `plan.md:159`.
Consider adding `plan_todo/` to `PARAM_DOCS` with phrases for the constants
these files name -- the S150 comment in `tests/CMakeLists.txt:295-310` says why
the mode holds no line numbers, and a named-constant phrase holds none either.

---

### 2026-09-04_plan_review-F06  low  Block 3 is ordered by figures no tracked document sources, and the enrichment pass that would source them has not moved since DEC-097 resumed it

Status: planned — S185

**Evidence.** `adocs/plan.md:328` orders block 3 "in the order the Stash
ledger prices them: mobility area and curves (S121, +20 class), passed pawns
with king distance (S123, +22.3 the largest single entry), the connected and
phalanx pawn work (S125, +25.4 class) ... threats (S101, +10 class)"; the
figures come from DEC-087 (i) (`adocs/decisions.md:5048`), which names "the
Stash ledger" and no path, commit or URL. The thirteen unenriched step files
that carry block 3's and the reserve's figures contain no URL at all:

```
$ grep -c http adocs/plan_todo/S{023,110,111,118,119,121,122,123,124,125,126,129,133}_*.md
0 (every file)
```

and carry, among others: `S121_mobility_curves.md:28` +19.95 / +10.5 / +10.9;
`S123_passed_pawn_suite.md:24` +36.1 and +22.3; `S124_endgame_scaling_factors.md:15`
+11.3, +13.72, +7.56, +9.41; `S125_pawn_structure_completion.md:19` +10.68,
+6.60, +3.77, +4.01; `S126_full_refit_after_search.md:21` "from 2529 to 2910
on the public list" for an engine it does not name; `S129_syzygy_three_four_five.md:15,17`
"about 13 Elo" and a quotation with no source; `S133_king_relative_psqt.md:18`
Berserk 4.3.0 "~+65" (no 4.3.0 entry exists on the CCRL list fetched; 4.1.0
3133, 4.4.0 3335); `S110_correction_history_non_pawn.md:25-27` twelve
STC/LTC pairs. A commit-message search of the Stash repository found none of
the block-3 figures: `https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+passed+king+distance`
returns one unrelated commit (ca128fd4, a king-pawn cache, +8.28/+3.70),
`...+connected+pawns` and `...+mobility+area` return nothing. That is a weak
instrument and the figures stay **unverified**, not refuted.

The same applies to the plan's "third independent line" for DEC-081:
`adocs/plan.md:46` "Ethereal's own feature-removal ledger prices its history
at -759, its LMR at -249 ...", which two enrichment passes could not locate
(`S098_reduction_refinement.md:32-33` "repo-recorded via DEC-087; the primary
URL is still untraced publicly (S091 hit the same wall)";
`S091_see_pruning_main_search.md:66,335`).

DEC-097 (2026-08-21) resumed the enrichment "in parallel, on nights".
`git log --since=2026-08-21 -- adocs/plan_todo/` shows 40 commits and none of
them an enrichment; `grep -L 'Technical details (SOTA research'
adocs/plan_todo/*.md` lists 35 of the 54 pending files.

**Impact.** Under DEC-120 and S144's own rule that a citation carries its
path, and under this run's evidence standard, block 3's ordering figures and
DEC-081's third confirmation carry no weight until traced. Block 3 is far down
the order, so nothing is executed against them yet; what is at stake is that
the order will be executed later against figures nobody can check, and
S133 -- added by DEC-087 (d) on "~+88 CCRL and ~+65 estimated" -- rests on one
verified number (Leorik 2.4 2829 to 2.5 2917 on the fetched list) and one that
cannot be.

**Suggested resolution.** Either run the DEC-097 pass over the block-3 files
before block 3 starts, or record now, in DEC-087 or an `adocs/data/` table,
the commit or URL behind each "Stash ledger" and Ethereal-ledger figure; where
no source can be found, mark the figure unverified in the file that carries
it, so the order is seen to rest on what it rests on.

---

### 2026-09-04_plan_review-F07  low  59 citations have drifted three days after the re-anchor, one in an accepts field; and the checker cannot see a citation that was already wrong when it was written

Status: planned — S187

**Evidence.**

```
$ python3 tools/plan_prose_check.py --citations
...
  DRIFT  adocs/plan_todo/S159_killer_slot_ageing.md:3  src/chesso.cpp:674  -- held 'search_state_t state = {};' at fe25f46
citations flagged: 59 over 54 files
```

S169 re-anchored 97 citations on 2026-09-01; S172 and S176 then moved
`src/chesso.cpp` and S147/S170 moved `src/data_structures.hpp` and
`tests/`. My own census (compare each cited range at the file's last commit
against HEAD) agrees: 58 drifted of 549, 25 in S132, 14 in S115, and the
S159 `accepts:` field cites `src/chesso.cpp:674` for `search_state_t state =
{}`, which is at `:766` at HEAD (`:674` is `for (size_t i = 0; i < moves_cout;
++i) {`). The mode is deliberately not in the fast suite
(`tests/CMakeLists.txt:278-286` says why) and nothing runs it at step
completion.

The class the checker cannot see, because its baseline is the file's own last
commit: citations that were stale when written. Verified at HEAD:

- `S109_shallow_depth_pruning_block.md:3` (the accepts) and `:398` cite
  `src/search.cpp:678` for `is_check_move` and `:710` for the LMR guard. At
  HEAD `:678` is `if (tt_entry_answers(...))`; the flag is at `:929` and the
  guard at `:961` -- which the same file's body says at `:183` and `:218`. The
  file's last commit is `7862f41` (2026-09-01), where `:929` already held the
  flag, so the checker reports "0 flagged".
- `S055_taper_stage_two_once.md:19` cites `:953` bare and `:59-60` cite
  `src/evaluation.cpp:668` (tempo), `:951` (mobility), `:953` (king safety);
  at HEAD the four taper divisions are at `:699-700`, `:726-727`, `:1009` and
  `:1011`, as the same file's "Shape" section (`:123`, `1008-1011`) says.
- `S024_continuation_history.md:20-22` cite `src/search.cpp:517` for the
  countermove write (HEAD: `unmake_move(game);`; the write is at `:1027`) and
  `src/evaluation.cpp:1152-1155` for the read (HEAD: the capture branch; the
  read is at `:1164`) -- while `S024:3`'s own accepts says "every src/ citation
  in this file names a symbol or a test title rather than a line range".
- `S119_tt_cluster_layout.md:30` cites `rating.sh:36` for `hash_mb=128`; at
  HEAD `:36` is a comment and the assignment is at `:55` (S177 rewrote the
  file).

The gates that matter most are right: "pruning does not hide a forced mate" is
at `tests/test_search.cpp:2808`, "... against the material leader" at
`:2847`, "a side in check may not stand pat" at `:993` and "mate is
recognised at depth zero" at `:1067`, exactly where S091, S095, S097, S098,
S109, S112, S113, S116 and S131 cite them.

**Impact.** Low: every drifted citation above sits beside the symbol it
names, so an implementer who greps recovers. The recurrence rate is the
point -- 97 re-anchored on 2026-09-01, 59 stale on 2026-09-04 -- and the
checker's blind spot means a citation copied wrong survives every run of it.
This is `2026-08-20_plan_review-F01`'s class in its third recurrence.

**Suggested resolution.** Give `--citations` a symbol check: where the
sentence holding a citation names a backticked identifier, flag the citation
if the identifier is absent from the cited lines at HEAD. Re-anchor the S109,
S055, S024 and S119 lines above by hand, and make S024's Note obey its own
accepts.

---

### 2026-09-04_plan_review-F08  low  S042 scopes the en-passant change to `make_move`; the code's own comment and the S161 FEN sanitiser say the key must move in three places

Status: planned — S184

**Evidence.** `adocs/plan_todo/S042_en_passant_only_when_capturable.md:4`
`touches: src/bitboard.cpp make_move`; `:49` "One `pawn_attacks` lookup
against the enemy pawn bitboard in `make_move`". Its `accepts:` (`:3`)
requires that "generate_FEN's fourth field agrees with Stockfish over a corpus
sample".

The site the step names, `src/bitboard.cpp:815-833`, carries this comment:
*"ep_randoms has an entry for INVALID_INDEX and it must be folded in and out
like any other square. Skipping it ... leaves the incremental hash offset by a
constant from compute_full_hash() and from set_en_passant(), both of which
apply it unconditionally, so the two would disagree about the key for the same
position."* And `load_FEN`'s sanitiser since S161 (`src/bitboard.cpp:1869-1891`)
keeps an en-passant square whenever the target is empty and the victim pawn
stands behind it -- it tests the victim's presence, not whether any pawn can
capture -- then `:1904` keys the position with `compute_full_hash()`.

**Impact.** With `make_move` alone changed, a position reached by moves drops
a non-capturable square while the same position loaded from a FEN keeps it,
so the two key differently -- the exact transposition mismatch the step exists
to remove, moved from move order to input path. Every SPRT game starts from a
`position fen` of the book, so the case is not exotic. The accepts' Stockfish
comparison would also fail on any FEN carrying such a square, since
`generate_FEN` would print what `load_FEN` kept. Low, because the effect is a
key mismatch at a game's root and not a wrong move; recorded because the
`touches:` field is the diff's contract and it names one of the three sites.

**Suggested resolution.** Add `load_FEN` (the S161 sanitiser) and
`set_en_passant`/`compute_full_hash` to S042's `touches:` and state the rule
once -- a square is kept only when a capturing pawn exists -- at every place
the key is built; or state in the step why FEN input is exempt and drop the
Stockfish-agreement clause for that case.

---

### 2026-09-04_plan_review-F09  low  Prose in five places describes completed steps as pending or routes to a folded id, and the `--prose` checker passes

Status: planned — S184

**Evidence.**

- `adocs/plan.md:449-456`, the machine-scope lane: "The lane, entries 1 to
  5 ... What is left is three that are behaviour-neutral ... (S147, S020,
  S030)". S147 completed 2026-09-02 (`adocs/plan_done/S147_mate_pv_completeness.md`);
  the lane now holds six entries (S178, S179, S020, S030, S148, S159).
  `python3 tools/plan_prose_check.py --prose` reports "108 ids in prose, 46 of
  them completed, sentences flagged: 0" -- the sentence shape "What is left is
  ... (S147" is not one it recognises.
- `adocs/status.md:297-299`: "Next: S178 ... Then S020 resumes the plan
  proper", skipping S179, which the same file (`:157-158`) and the Open list
  place second.
- `adocs/plan_todo/S159_killer_slot_ageing.md:36-38`: "S093 rewrites this same
  block ... this lands before S093 or is folded into it deliberately, never
  after". S093 landed 2026-08-22; the constraint is already violated by
  history and the file does not say what that means for the step.
- `adocs/plan_todo/S171_inherited_mate_distance.md:72`: heading "Why it is
  first in the Open list"; it is 54th, postponed by DEC-128.
- `adocs/plan_todo/S109_shallow_depth_pruning_block.md:367`,
  `S110_correction_history_non_pawn.md:18`,
  `S132_time_management_node_fraction.md:190,207` route follow-up
  measurements to "S128", which DEC-108 folded into S152 on 2026-08-23.

**Impact.** Low, and the same class `2026-08-13_plan_review-F05`,
`2026-08-13_plan_review.2-F07` and `2026-08-16_plan_review-F05` recorded:
prose a cold session reads before the list says a done step is pending and a
gone id is a destination.

**Suggested resolution.** Rewrite the lane paragraph to the list as it stands
(or delete it, as its own last paragraph invites, once the workstation is
back); fix the status `Next` line; give S159 a sentence saying S093 has landed
and the ordering clause is void; retitle S171's section; re-point the three
S128 references to S152.

---

### 2026-09-04_plan_review-F10  low  Two pending step files lack the `done:` field and three carry `author:` while in `plan_todo/`

Status: planned — S184

**Evidence.** AGENTS.md's step schema ends `author: <!-- who claimed it, set
on start -->` and `done: <!-- completion stamp ... written last -->`.

```
$ grep -c "^done:" adocs/plan_todo/S148_rfp_ceiling_against_deep_mates.md adocs/plan_todo/S171_inherited_mate_distance.md
0 / 0
$ grep -n "^author:" adocs/plan_todo/S148_* adocs/plan_todo/S024_* adocs/plan_todo/S171_*
S148_rfp_ceiling_against_deep_mates.md:10:author:     Maksym Bodnar
S024_continuation_history.md:288:author:    Maksym Bodnar
S171_inherited_mate_distance.md:10:author:     agent (Claude Opus 5), coordinator, 2026-09-03
```

S148 was created at `14748c9` (2026-08-21) with `author:` filled and no
`done:` line, and has never been in `plan_current/` (`git log --all` over
both directories shows only that commit). S171's `author:` survived DEC-128's
move back to `plan_todo/` and its header has no `done:` line. S024's
`author:` sits at line 288, below its References, left from the discarded
MacBook attempt (DEC-111).

**Impact.** Low. The directory is the state and it is unambiguous; but a
claimed `author:` on a `plan_todo/` file reads as a started step to anyone
applying the schema, and a missing `done:` field is where the stamp is
"written last" -- S148's completing session would have to add the field before
it could fill it.

**Suggested resolution.** Add the `done:` line to S148 and S171; clear
`author:` on S148, S171 and S024 or record in one line why it is kept (DEC-128
for S171, DEC-111 for S024).

## Checked and clean

Stated so the re-run knows what was already settled.

- **List and files.** 54 pending files, 54 Open entries, no duplicate, every
  entry has a file and every file an entry; `Done recently` holds exactly the
  last five completions and all five are in `plan_done/`; the highest id in
  any directory is S179, the newest file.
- **Build and suite at HEAD.** `build` and `build-tune` fast suites 27/27,
  `./clang-format.sh --check` clean, `search_bench` at 9 and 12 identical to
  the recorded baselines (above).
- **The traced records the enriched steps rest on say what the steps say.**
  Fetched and matched to the decimal: Weiss `1405ec8` +44.68/+33.95 (S024);
  Berserk `bcb7d9b8` +25.41 with "Squashing ~10 commits which include rewrite
  of tuner" (S117's own caveat, verbatim); Ethereal `60f4d5c5` +9.85/+9.66/
  +20.90/+8.33 (S132); Lynx #1662 +11.35 and #1663 +3.27 / +12.09 (S099);
  Lynx #613 +11.40 (S098); Lynx #512 +4.7 with `i >= depth * 10`, max 3, and
  the quadratic form failing (S109); Lynx #1203 +3.59/+9.40 with the worked
  examples (S132); Weiss #283 +6.36/+6.65 (S113); Weiss #129 +5.36/+11.48
  (S114); Weiss #183 +6.78/+8.19 and its three-part sentence (S115); Stockfish
  PR #2401 "step 7 ... ~30 to ~49" and "step 14 ... ~170 to ~204" (S109, the
  plan's ~204); PR #3921's "if qsearch can push it above alpha" sentence
  (S116); `9a8dd81d` "very poor mate finding performance ... CI hang on a mate
  in 2", 2026-07-11 (S114); `dabaf222` "sf14 2427 / master 1246 / patched
  2467" (S148, DEC-095). Not confirmed from the body fetched: PR #4294's
  "movecount pruning ~0 alone" -- its table rows are unlabelled fishtest links
  (S109:8, `unverified`).
- **The CPW pages say what the steps attribute to them.** Corrhist weight
  `min(depth*depth + 2*depth + 1, 128)`, the four update conditions, the
  clamp and the `66 * cv / 512` snippet S099 declines; IIR "depth > 5, say",
  Rebel 2020, Chaly's cut-nodes 2021; Delta "typically around 200", 975/775,
  the late-endgame switch-off; Razoring "~three pawns" and the SF `512 +
  293*depth*depth` snapshot S116 declines; SEE "linear depth margin for
  captures, and a quadratic depth margin for quiets" (S091's scope concern
  stands); Null move "a factor scaled by the difference between (potentially
  TT-corrected) evaluation and beta", "fixed reduction of 3 or 4"; LMR's
  Obsidian `0.99 + ln(depth)*ln(moves)/3.14`, the "Reduce less in the
  PV-Nodes ... when improving ... more in an expected Cut-node ... when hash
  move is a capture" list, "bad" captures (SEE < 0); Futility's in-check,
  mate-band, one-legal-move and minor/rook wording; History's n-ply
  continuation definition and 3-to-6-ply note; Singular's Stockfish 1.6
  lower-bound restriction; ProbCut's Buro 1995 / Logistello / "until Stockfish
  proved otherwise"; Aspiration's exponential widening with the other bound
  unchanged; Killer's "all the available slots contain different moves"
  (S149/S159's premise); Countermove's Uiterwijk 1992; Mate Distance Pruning
  "will not add much to a programs playing strength" (DEC-087's omission).
- **The plan's CCRL anchors are the list's.** Stockfish 11 3565, Komodo 14.1
  3482, Xiphos 0.6 3356, Ethereal 11.75 3346 (all 1CPU entries, as DEC-089
  says; 4CPU/8CPU entries are separate rows); Weiss 1.2 3055; Stash 27.0 3049
  and 37.0 3424; Leorik 2.5 2917, and Leorik 1.0 2102 to 2.0.2 2538 is the
  "+436" the plan cites; Leorik 2.4 2829 to 2.5 2917 is S133's "~+88". The
  list's conditions line reads "Ponder off, General book up to 12 moves, up to
  6 piece EGTB, Time control: Equivalent to 2'+1" on an Intel i7-4770K" --
  DEC-085/DEC-089 and S129's tablebase premise hold.
- **Code premises of the pending steps hold at HEAD.** S020: `is_check` once
  per function at `src/search.cpp:415` and `:687`, the child scan at `:929`
  with its one consumer at `:961`. S024: no continuation history; `counter_moves[12][64]`
  (`src/data_structures.hpp:511`), `quiet_history[2][64][64]` `int16_t`
  (`:502`), `history_gravity_update` and `quiets_tried` exist (`src/search.cpp:33,854`).
  S095: `negamax` asserts a move before its store (`:1069,1100`) and only
  quiescence stores moveless entries (`:432,522,556,576`). S097: `MAX_PLY`
  128, `MAX_DEPTH` 126, `tt_entry_t` fields as described, 24 bytes. S099: one
  `hash_t hash` in `board_t` (`src/data_structures.hpp:298`), no pawn key.
  S108/S109: `improving_at()` defined (`src/search.cpp:255`) with no in-search
  call site; `static_evals[MAX_PLY]` written at `:731`. S114: `NULL_MOVE_BASE`
  3, `NULL_MOVE_DIVISOR` 6. S119: direct-mapped `entries[hash & index_mask]`
  (`src/transposition_table.cpp:99,152`); `fastchess.sh:348` `option.Hash=16`.
  S134: `passed_pawn_mg[5]` -17 and `passed_pawn_eg[5]` +42
  (`src/evaluation.cpp:80-81`); `PARAM_COUNT` 827 from the widths in
  `tools/eval_model.hpp:30-131`. S055: four `GAME_PHASE_MAX` divisions
  (`src/evaluation.cpp:699-700,726-727,1009,1011`), two of them the pair it
  merges. S148: its table is `adocs/data/S145_rfp_sweep.log` row for row;
  S085's +21.02 is in `adocs/plan_done/S085_spsa_first_run.md:14`. S151:
  `3488506` exists and carries the pre-vector values (RfpMaxDepth 6, LmrDivisor
  225, MaxQsearchDepth 8, AspirationMinDepth 5), so the re-test it names is
  well-posed. S097's `excludes:` cites `src/search.cpp:961` and that is the
  guard. S131's test FEN and SEE reasoning are consistent with `see_ge`'s
  promotion branch as described.
- **Rules.** No pending step retains a play-altering change without an SPRT
  clause, and every neutral claim carries the `search_bench` identity (S020,
  S030, S032, S117, S134, S178, S179). S129 is stricter than DEC-105 requires
  (the MIT Fathom route is on record in DEC-087 and DEC-105 as an owner
  option), which is a cost and not a breach. Nothing pending copies a table or
  source; S179 regenerates the one coincidence DEC-132 recorded.
- **Techniques the record has and the plan lacks.** Beyond the two the
  2026-09-03 audit named (mate-distance pruning, killer reset two plies down),
  the surveyed hand-crafted engines carry trapped-piece terms, a 50-move draw
  scaling and draw checks inside quiescence; each is small in the published
  record and none is priced above the plan's instrument. Nothing large is
  missing; nothing pending is already implemented.

## Verdicts on prior findings

Each re-measured from its own reproduction at `18deccf`. This is a
plan_review re-run, so these verdicts move plan_review findings; adversarial
findings are recorded as they reproduce.

### 2026-08-13_plan_review (nine findings)

All nine were closed by `2026-08-13_plan_review.2` and none reappears in its
original form. F05's class (prose calling a done step pending) recurs in new
sentences and is recorded above as F09, not as a survival.

### 2026-08-13_plan_review.2 (nine findings)

- **F01 (S055's accepts unreachable)** -- does not reproduce. `S055:3` re-pins
  the thresholds in its own commit; `tests/test_eval_model.cpp:277,335,343`
  hold the pre-change `<= 3.0`, `> 2.0`, `> 2.8` it will move. **closed.**
- **F02 (S039's corpus does not exist)** -- does not reproduce as a plan
  defect: `S039:3` names `.tuning/selfplay_v2.tsv` where present and the
  tracked 5582-row corpus otherwise, and records which was read. (The file is
  absent on this machine, as DEC-112 states.) **closed.**
- **F03 (S030's remedy indexes a 12-row table with 12)** -- does not
  reproduce. `S030` splits the sites by what they key on and derives the set
  from `grep -n MOVE_PIECE src/`. **closed.**
- **F04 (S025's gate admits one outcome)** -- does not reproduce; `S025:3` is
  outcome-neutral. **closed.**
- **F05 (S026's gate weaker than its body)** -- does not reproduce; S026 was
  retired into S109 whose `accepts:` carries the mate clause with the red
  observation required. **closed.**
- **F06 (S023's band criterion has no instrument)** -- does not reproduce;
  `S023:3` names the king-takes-pawn-versus-killer case and `closes:` carries
  the id. **closed.**
- **F07, F09** -- closed by `2026-08-16_plan_review`; unchanged.
- **F08 (three citations moved two lines)** -- the three were re-pointed; the
  class recurs as F07 above. **closed.**

### 2026-08-16_plan_review (ten findings)

- **F01 (S057 obsolete, spread understated)** -- S057 is folded into S039
  (DEC-086) and `S039:3` re-measures the mobility sentence too. The
  `src/evaluation.hpp:288,295` comment is still stale, which is S039's own
  work and `2026-08-13_adversarial-F05` (below). **closed.**
- **F02 (17 citations stale, 9 outside S063)** -- re-anchored by S138 and
  S169; the class recurs (59 at HEAD), recorded as F07. **closed.**
- **F03 (S056's numbers describe a replaced corpus)** -- S056 folded into
  S055, which states it re-measures on the pinned set at its own HEAD.
  **closed.**
- **F04 (S068's evidence in a scratchpad)** -- S068 is done and its sweeps are
  tracked (`adocs/data/S033_rfp_*.tsv`, `S145_rfp_sweep.log`). **closed.**
- **F05 (status.md machine paragraph)** -- rewritten. **closed.**
- **F06 (illegal FEN live in test_engine.cpp)** -- `7k/5Q1K` survives only in
  comments (`tests/test_engine.cpp:806`, `tests/test_search.cpp:148`).
  **closed.**
- **F07 (S021 no mate clause)** -- S021 done with S074. **closed.**
- **F08** -- accepted at DEC-062; the file is now tracked
  (`git ls-files adocs/eval_tuning_strategy.md`). unchanged.
- **F09 (S060/S061 evidence predates S033)** -- both folded. **closed.**
- **F10 (bin/moltke.py cited)** -- no such citation remains. **closed.**

### 2026-08-20_plan_review (eighteen findings)

- **F01 (70 stale citations; six to the wrong mate test)** -- the hazard is
  gone: every pending citation to the mate-safety and quiescence-mate tests
  resolves (`:2808`, `:2847`, `:993`, `:1067`). The census form reproduces at
  59 with one accepts field affected, and the checker's baseline cannot see a
  citation stale at write time -- F07 above. The gating half of its
  resolution was declined with a recorded reason (`tests/CMakeLists.txt:278-286`).
  **planned** (still reproduces in census form).
- **F02 (S119 at Hash 128)** -- `S119:3` reads "at the S105 harness setting,
  with the pressure ratio stated". **closed.**
- **F03 (S136 arithmetic one division out)** -- `S136:3` derives N at its own
  HEAD; DEC-092's literals stand with an unapproved `Proposed:` block, which is
  the form a decision takes until the owner moves it. **closed** for the plan
  documents; the decision residue is the owner's.
- **F04 (S125 requires S118)** -- the clause is gone and S125 says why.
  **closed.**
- **F05 (specs routes check extensions to S096)** -- `specs.md:539` records
  the retirement. **closed.**
- **F06 (touches omit the file)** -- S039 and S120 name them;
  `test_plan_touches` is in the fast suite and passes. **closed.**
- **F07 (S109 gives-check clause binds LMP)** -- split in `S109:3`. **closed.**
- **F08 (RfpMinPly minimum admits red values)** -- `src/search_params.hpp:174`
  is `X(RFP_MIN_PLY, "RfpMinPly", 3, 2, 63)`. **closed.**
- **F09 (S085 goal names twenty)** -- amended by DEC-094; S085 done. **closed.**
- **F10 (excludes route to retired ids)** -- all three re-pointed. **closed.**
- **F11 (block-3 prose omits S134-S136)** -- `plan.md:314` header "S134 to
  S126" and the paragraph names all three. **closed.**
- **F12 (S115 names a results file)** -- `S115:3` names the script and the
  derivation. **closed.**
- **F13 ("mate-in-quiescence" test name)** -- no occurrence; both accepts quote
  the titles. **closed.**
- **F14 (OrderHistoryMax range breaks the bands)** -- the parameter no longer
  exists; `QUIET_HISTORY_MAX` 8192 with maximum 32767 cannot reach
  `ORDER_COUNTER` 700000. **closed.**
- **F15 (S117's "or the tolerance moves")** -- gone. **closed.**
- **F16 (status census stale)** -- replaced by the `grep -L` recipe. **closed.**
- **F17 (INV-7 undefined)** -- `specs.md:135-145` records that the numbering
  ends at INV-6. **closed.**
- **F18 (S085 watcher path/ceiling)** -- S085 done. **closed.**

### Adversarial findings with a pending step (recorded, not moved)

- **2026-08-13_adversarial-F05** (lazy-margin comment false) -- still
  reproduces: `src/evaluation.hpp:295` "because king safety ships at zero
  weight", `:288` the pre-refit mobility maxima; the margin is 184. S039
  pending, entry 33. **planned.**
- **2026-08-13_adversarial-F08** (en passant set unconditionally) -- still
  reproduces: `src/bitboard.cpp:827-828` sets `new_en_passant` from
  `move.double_push` alone. S042 pending, entry 26; its scope gap is F08
  above. **planned.**
- **2026-08-21_adversarial-F03** -- S151 pending, entry 52; its reference
  commit verified pre-vector. **planned.**
- **2026-08-21_adversarial-F04** -- S152 deferred by DEC-108, entry 53.
  **planned.**
- **2026-09-04_adversarial-F01, F02, F03** -- open; no `closes:` field and no
  `decisions.md` entry names them at HEAD (`grep -l 2026-09-04_adversarial
  adocs/plan_todo/*.md adocs/decisions.md` is empty), which `status.md:22-24`
  states as waiting on the owner under DEC-035. Recorded as such.

## Sources fetched during this run

Rating list and conditions: `https://computerchess.org.uk/ccrl/404/`
(conditions line) and `https://computerchess.org.uk/ccrl/404/rating_list_all.html`
(computed 2026-08-28; all engine figures above). Lynx README:
`https://raw.githubusercontent.com/lynx-chess/Lynx/main/README.md`.

Engine records (GitHub API, message and body text only, no diff or source
opened): `repos/TerjeKir/weiss/commits/1405ec8`, `repos/TerjeKir/weiss/pulls/129`,
`/183`, `/283`; `repos/jhonnold/berserk/commits/bcb7d9b8b116810f43e696474b86eda6e280b686`;
`repos/AndyGrant/Ethereal/commits/60f4d5c5`; `repos/lynx-chess/Lynx/pulls/512`,
`/613`, `/1203`, `/1662`, `/1663`; `repos/lynx-chess/Lynx/releases/tags/v1.8.0`,
`/v1.9.0`, `/v1.10.0`; `repos/official-stockfish/Stockfish/pulls/2401` (and
`/commits`), `/3921`, `/4294`; `repos/official-stockfish/Stockfish/commits/9a8dd81d`,
`/dabaf222`; `search/commits?q=repo:mhouppin/stash-bot+...` (three queries,
F06). All under `https://api.github.com/`.

Chess Programming Wiki (`https://www.chessprogramming.org/`):
`Static_Evaluation_Correction_History`, `Internal_Iterative_Reductions`,
`Delta_Pruning`, `Razoring`, `Static_Exchange_Evaluation`,
`Late_Move_Reductions`, `Futility_Pruning`, `Null_Move_Pruning`,
`History_Heuristic`, `Singular_Extensions`, `ProbCut`, `Aspiration_Windows`,
`Killer_Heuristic`, `Mate_Distance_Pruning`, `Countermove_Heuristic`.

Not fetched, and therefore `unverified` wherever the plan relies on them: the
Ethereal feature-removal ledger (-759/-249/-175/-32), the "Stash ledger"
figures behind block 3, Berserk 4.3.0's "~+65", the "13 Elo" for tablebases,
the "2529 to 2910" release, and Stockfish PR #4294's per-row labels.
