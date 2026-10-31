id:         S125
goal:       phalanx, supported and weak unopposed pawns join the isolated, doubled and backward pawns that exist, backward stops counting an isolated pawn twice, and each is fitted
accepts:    one SPRT verdict for the family, recorded whatever it is, after each group passes DEC-259's offline screen, and a family H0 bisected by group; the terms are indexed by file or by rank where the surveyed record says the indexing is what pays, and the step states which indexing it chose and why; **an isolated pawn is never also counted backward**, which is a documented double-count worth +4.01 to fix; every constant is fitted (DEC-084); **the pawn hash is not a precondition and the plan order is not reversed** -- these terms are what make S118 worth building, so they are measured recomputed at every `evaluate()` call, and the per-call cost the additions carry is measured here (`bench_eval`, and `build/tools/eval_spread` for where it lands) and recorded as the baseline S118 is later asked to reclaim
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_trace.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp, tools/tuner_trace.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tests/test_evaluation.cpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_eval_positions.hpp, tests/test_walked_counts.hpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, .tuning/verify_fit.py, adocs/data/S192_anchors.py, adocs/data/S123_coverage_rows.cpp (scoped), adocs/data/S125_*, DEV_MANUAL.md, adocs/data/README.md; MANUAL.md read and unchanged at phase 1; at phase 2 also tests/test_search.cpp, tests/test_engine.cpp, tests/test_mate_breadth.cpp, tests/test_mate_carry.cpp, tests/test_mate_pv.cpp, adocs/data/S170_cases.tsv and MANUAL.md; the close (2026-10-31, H1): src/evaluation.cpp and src/evaluation.hpp (the fold and the comments), tests/test_walked_counts.hpp and tests/test_eval_model.cpp and tests/test_evaluation.cpp (the definition, the hand cases, the guard), comments in tests/test_eval_trace.cpp, tests/test_eval_positions.hpp and tests/test_tuner_gradient.cpp, tools/tuner_layout.hpp and tools/tuner_groups.hpp (comments), adocs/data/S192_anchors.py (its prose), adocs/data/S125_sprt.log, adocs/data/S125_close_inv6.txt, adocs/data/S125_bench_eval.txt, adocs/data/S125_cost_reading.txt (RUN 3), adocs/data/README.md, DEV_MANUAL.md, this file; src/eval_trace.hpp, tools/tuner.cpp, tools/tuner_trace.hpp, tools/feature_audit.cpp, .tuning/verify_fit.py and MANUAL.md read and unchanged at the close
excludes:   passed pawns, which are S123
decisions:  DEC-071, DEC-084, DEC-259, DEC-315, DEC-326, DEC-329, DEC-330, DEC-331, DEC-221, DEC-134, DEC-105, DEC-090, DEC-047, DEC-142, DEC-316
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-31 10:32 -- H1 at {0, 5} nElo, 8+0.08, Hash 16, CAND cf8661b vs REF 9cbeb68: Elo +22.89 +/- 10.02, nElo +31.16 +/- 13.60, LLR 2.96, 2508 games, 1 h 12 m, 0 forfeits (adocs/data/S125_sprt.log, archived as chesso_S125_sprt_2026-10-31.tar.xz); the banner as pinned (9bcafb9), both binaries at their bench (5707449, 5434393), no crash or disconnect, quarters +18.3, +37.8, +14.4, +21.6, kernel journal clean; no warning of any kind, 0 incomplete mating PVs; so by the pre-registered H1 row the family ships -- P2 isolated pawns by file b to h, P5 phalanx by own rank 2 to 7, P6 supported by own rank 3 to 7, P7 one weak unopposed weight, the three flats refitted beside them, the screen's 02567 fit (DEC-259, DEC-331), d +0.000012, +0.000036, +0.000031 and +0.000017, each past its margin; P1, P3 and P4 dropped, P3 and P4 at zero and in no expression -- and no bisection; the stopping Elo is not the effect size (DEC-063). An isolated pawn is never also counted backward: the fold of DEC-331 (3), (13), (15), isolated_backward taking backward's flat plus the zero increment, {-6} / {0}, INV-6 against cf8661b (adocs/data/S125_close_inv6.txt), the tuner exact on all 6226944 corpus rows, the audit's rank 1026 of 1073 and no new dependency, the hand cases rewritten with the definition, a guard in both colours seen red with the mask removed. Every family weight fitted on S083's corpus from zero and the three flats from S083's own {-9, -2, -11} / {-15, -18, -2}, with every other constant frozen (DEC-084, no seed), monotonicity not imposed; every definition implemented from its description, CPW's pages and DEC-331 (2) (DEC-221); the mirror test written and seen red before any fit; cost cand +3.50 %, all +4.20 %, p1 zero (adocs/data/S125_cost_reading.txt), the baseline S118 is asked to reclaim, with bench_eval per call, evaluate() alone on ten positions, REF 63.99 ns, CAND 74.06, the fold 75.93 (adocs/data/S125_bench_eval.txt; one layout, not perturbed, deciding nothing); both mate floors hold; every golden the fold moves re-derived by its script, the gauge walk alone moving (0.0882, 0.1165); the far cells listed for S126 (adocs/data/S125_cells.txt). Fast suite 50/50 in both builds, the clang pair clean, gate_extra 6/6 in 1514 s; Tier 2 does not bind. Implemented by Opus 5.5 subagents per phase, fast-checked; the runs the coordinator's; inspected, folded and recorded by one Opus 5.5 subagent.

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

**Corrected 2026-10-08:** the goal used to list backward pawns as new;
`src/evaluation.cpp` already computes them beside isolated and doubled pawns.
What is new for backward is the double-count fix in the accepts.

Each term -- and each part this file used to give a verdict of its own --
is first fitted with every other constant frozen and kept only if it lowers
held-out loss on S082's fixed validation set by more than the margin the
pre-registration states; a term that fails is recorded with its figures and
does not enter the match. The survivors ship under **one** `{0, 5}` nElo SPRT
for the family, with DEC-143's worst case and abort rule pre-registered. On
H0 or no verdict the family is bisected along a partition written in the
pre-registration, each part its own SPRT. Where this file names
`tools/eval_model.hpp` or `test_eval_model` as the tuner's mirror of a term,
S262's trace and its exact-reconstruction test discharge that clause: S262
landed 2026-10-12 and S276 deleted `tools/eval_model.hpp`; the trace, its
`term_layout()` line and the walk in `tests/test_walked_counts.hpp` replace
it (corrected 2026-10-30). S263 built no per-side attack set, since no term read one;
a term here that needs one builds it once per evaluation -- S125 builds none:
every predicate reads the pawn fills `evaluate_pawns()` already builds
(corrected 2026-10-30). S039 read H0 (record `ca578de`,
revert `74d0adf`): the lazy shortcut and the clamp are kept. S122's design
amendment (DEC-258) was discharged by DEC-324 (2026-10-27: the shortcut and
the clamp kept, S122 re-derives the margin), and S121 and S123 are done (H1);
the corpus is S083's, with S082's qs view the fixed validation set (corrected
2026-10-30).

**The accepts' cost clause, corrected 2026-10-30.** `build/tools/eval_spread`
reads `evaluate_expensive_terms()` -- mobility and king safety -- only, so a
stage-one term never appears in it. The per-call cost is DEC-047's
forced-weight run per group (`adocs/data/S125_cost.sh`, S123's form, an A/A
floor), recorded beside the verdict as the baseline S118 is asked to
reclaim; `bench_eval`'s ns per call may be printed beside it. Read so, the
accepts' clause stands.

## What is there

Three terms -- isolated, doubled, backward -- each one flat weight, fitted by
S083's refit (01222f3) to `{-9, -2, -11}` middlegame and `{-15, -18, -2}`
endgame (corrected 2026-10-30: this read S076's `{-10, -9, -11}` and
`{-12, -32, -8}`). What is missing is phalanx pawns, supported pawns -- CPW's
connected pawns are the two together, so no third column (DEC-331 (1)) -- and
weak unopposed pawns, three terms (corrected 2026-10-30: this read four); and
what the surveyed record says pays more than any of them is **conditioning**
rather than adding: the largest pure pawn-structure patch on record is "apply
the isolated penalty only when there is no pawn capture available", at +10.68.
Indexing the existing three by file and by rank is reported at +6.60 and +3.77.
**All four figures -- those three and the +4.01 double-count in the accepts --
are sourced**, each to an Ethereal commit message with its URL: section 1's
table names the commit, the date, the time control and the long-control half
of every one. The 2026-09-04 literature check had found none of them; S186
traced them through
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+isolated+pawn
and
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+backward+pawn.
Two readings come with them and both are in section 1: **+3.77 is a bundle**
(the same commit retuned the pawn PSQT), and **every one of the four shrinks
at 60.0+0.6s**, so a short-control verdict here reads high. The group's
largest traced entry is still Stash v31's connected pawns, phalanx and
defender, **+25.38 +/- 10.40** at 8+0.08 and +18.57 +/- 8.45 at 40+0.4, in
`mhouppin/stash-bot`'s `CHANGELOG.md` (https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md), which is the figure
the block order uses; and CPW *Pawn Structure*
(https://www.chessprogramming.org/Pawn_Structure) states no figure at all. The
conditioning claim this step's shape rests on -- that conditioning pays more
than adding -- is therefore the largest *sourced* figure of the four, and
section 6 prices the conditioning run at the +10 class with the long-control
caveat attached. **The conditioning is not built** (DEC-331 (1), corrected
2026-10-30): the record names it by a commit title alone, and no computable
rule -- whose capture, of what -- is stated anywhere, so it is recorded here
as unbuilt and is no part of the family.

So this step is as much about giving the three terms that exist more shape as
about adding three more (corrected 2026-10-30: four).

## The pawn hash comes after, and that is deliberate

The `accepts:` used to require these terms to "live behind the pawn hash from
S118", which S118 cannot supply at this point in the order. S139 dropped the
dependency rather than reorder, because every document that touches the
question orders the terms first and says why:

- `adocs/plan.md` "the pawn cache once the pawn terms are worth caching" --
  the order since the 2026-10-08 rewrite (DEC-258) keeps the terms first and
  the cache after them, as the order before it did (`git show
  a43ef3e:adocs/plan.md`, its block-3 paragraph). The parenthetical that used
  to be quoted here read the published slowdown as a cheap pawn evaluation
  cached; DEC-203 replaced it with what the source says -- a table sized past
  the last-level cache -- and the ordering argument is unchanged by the
  correction.
- DEC-087 (h) -- *"S118 moves from the speed block into the evaluation block,
  behind the expensive pawn terms -- caching a cheap pawn evaluation measured a
  10 % slowdown in the published record"* -- and (i) prices the block in Stash
  ledger order, connected/phalanx ahead of the pawn hash. **DEC-087 (h)'s
  reading of that slowdown is wrong and DEC-203 says so**; the decision is not
  amended because the order it sets does not move, and the order's own reason
  is the next bullet.
- `adocs/plan_todo/S118_pawn_hash_table.md` "a cache of them saves little
  until S123 and S125 have made the pawn evaluation worth caching" -- S118's
  own body, which puts the argument on chesso's three pawn terms being cheap
  by construction and not on anybody's published figure, and
  `adocs/plan_todo/S118_pawn_hash_table.md` "is what makes S125's richer pawn
  terms affordable" for the same boundary read from S118's side.
- The list itself: `adocs/plan.md` "pawn structure (S125, v31 +25.38)" -- in
  the Open list S125 sat at 22 and S118 at 23 (2026-10-08), and sit at 1 and
  3 since S123's close (corrected 2026-10-30).

Reversing the pair would put a cache in front of the cheap computation it
caches, which buys little by this project's own arithmetic -- not by the
published 10 % slowdown, which was an oversized table and is a sizing
constraint on S118 rather than an ordering argument (DEC-203). So the
cost this step adds is paid per call and measured per call; S118 is the step
that reclaims it, and this step's numbers are what its verdict is read against.

**And it is not CLAUDE.md's accumulate-don't-recompute hazard either**, which
is about an accumulated quantity being rebuilt from the bitboards at every node
-- INV-4's four fields, and the 25 % of nps S014 took back. These three terms do
not exist in any form yet, so nothing stops being accumulated. What they cost
is what a new pawn term costs before a cache exists, and the family's one SPRT
prices exactly that (corrected 2026-10-30: "these four terms", "the SPRT per
group"): term value net of term cost, which is the honest question
at this position in the order. S118 reads the same boundary from its side
(`adocs/plan_todo/S118_pawn_hash_table.md` "the bitboards at every node; this
is a cache keyed").
## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**Every one of the four unverified figures in this file is now traced, and
three of the four are smaller than they were written.** They are Ethereal
commit messages, reachable through
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+isolated+pawn
and
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+backward+pawn
(both fetched 2026-09-13). Commit messages only; no source and no table was
read (DEC-016).

| what this file quoted | traced to | figure as the message states it |
|---|---|---|
| "+10.68 conditioning" | commit 7c9cf64ff40bc77350c5383d1c0ffd94aa3b8b04, 2019-07-09, **"Only apply Isolated penalty when there is not a pawn capture"** | **+10.68 +/- 6.14** at 10.0+0.1s; **+4.00 +/- 3.01** at 60.0+0.6s |
| "+6.60 indexing" | commit c2d2d03752cc3d5b5797d0a42511b0a69be9d19e, 2020-09-09, **"Evaluate Stacked and Isolated pawns by File"** | **+6.60 +/- 4.44** at 10.0+0.1s Hash 8MB; **+5.55 +/- 3.56** at 60.0+0.6s Hash 64MB |
| "+3.77 indexing" | commit 7afbeb6edf2a346730bad019a58c6c993843ddc0 (#121), 2020-02-02, **"Make the backward pawn penalty different by rank"** | **+3.77 +/- 3.01** at 12.0+0.12s; **+2.33 +/- 1.84** at 60.0+0.6s |
| "+4.01 double-count" | commit 923864abff95847044704536eba9ba66fde54e30, 2019-10-23, **"Don't consider isolated pawns to ever be backwards"** | **+4.01 +/- 3.15** at 10.0+0.1s; **+2.47 +/- 1.95** at 60.0+0.6s |

Four readings, and the third is a correction to this file's argument.

1. **The conditioning claim survives its own trace.** +10.68 is the largest of
   the four and it is the only one that changes *when* a penalty applies rather
   than *how finely it is indexed*. The step's shape -- conditioning pays more
   than adding -- is now sourced and is not an unsupported claim.
2. **The long-control halves are all smaller than the short ones**, +4.00
   against +10.68 and +5.55 against +6.60 and +2.33 against +3.77 and +2.47
   against +4.01. Every one of the four shrinks at 60.0+0.6s. That is the
   DEC-019 pattern with a direction attached: this group's value is
   control-sensitive downward, so an 8.0+0.08s verdict here reads high.
3. **+3.77 is a bundle.** The message says the commit "also retuned the pawn
   PSQT" alongside making the backward penalty rank-dependent, so it prices a
   retune plus an indexing change, not the indexing alone. Quoted for order of
   magnitude only.
4. **The conditioning idea has a second instance** the record also carries:
   commit 84009a58fd0e5b32411074911a2b0b15a7caf24d, 2017-12-05, "Don't apply
   connected bonus to backward pawns", which reports LLR and game counts and
   **no Elo figure**. Same class, no number.

**The group's largest traced entry is still Stash's**, +25.38 +/- 10.40 at
8.0+0.08s and +18.57 +/- 8.45 at 40.0+0.4s for connected pawns (phalanx and
defender) at v31, in `mhouppin/stash-bot`'s `CHANGELOG.md`
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md), and it is
what the block order uses. Nothing in this pass changes that.

**The wiki defines three of the four new terms and prices none of them.** CPW
*Pawn Structure* (https://www.chessprogramming.org/Pawn_Structure, fetched
2026-09-13) is an index: isolated, doubled, backward, connected and weak pawns
are links, **phalanx, supported pawns and unopposed pawns are not mentioned at
all**, and the page carries no figure. The definitions are on the dedicated
pages:

- CPW *Backward Pawn* (https://www.chessprogramming.org/Backward_Pawn, fetched
  2026-09-13): "A backward pawn is a pawn no longer defensible by own pawns
  and whose stop square lacks pawn protection but is controlled by a sentry."
  **The page says nothing about whether an isolated pawn can also be
  backward** -- so Ethereal's +4.01 is the only evidence for the double-count
  rule and the wiki neither supports nor contradicts it. No figure. *Backward
  Pawns (Bitboards)* asks the question and leaves it open (corrected
  2026-10-30, the research pass).
- CPW *Connected Pawns* (https://www.chessprogramming.org/Connected_Pawns,
  fetched 2026-09-13): "two or more pawns on adjacent files - as duo or
  phalanx on the same rank, mutually protecting their stop squares - or either
  as defending or defended member of a pawn chain". The page distinguishes the
  phalanx from the defended member, which is the split Stash's v31 entry also
  makes, and it notes that Stockfish "gives a connected pawn bonus based on
  the rank of the phalanx or defending pawn" -- a shape, with **no numerical
  values**.

So: "phalanx" and "supported" as separate terms come from the engine record
and not from the wiki, and that is stated rather than assumed; "weak unopposed"
has no located definition as one term, and its parts are on CPW: *Weak Pawns*'
"not defended and not defensible by the pawns of the same color" and *Open
Pawns (Bitboards)*' "no mechanical obstruction - an opponent pawn in front",
which S125 composes and names itself, saying so (corrected 2026-10-30,
DEC-331 (2); this read that no definition was located anywhere).

### 2. Shape for chesso

Today: `src/evaluation.cpp` `pawn_structure_mg` and `src/evaluation.cpp`
`pawn_structure_eg`, three flat weights each over `PS_FEATURE_COUNT`
features. The step adds four terms and gives the three that exist an index.

**Departures, stated:**

- The wiki's connected-pawn shape is one bonus by rank; Stash and the
  Ethereal record split phalanx from defender. This step follows the split and
  says so -- it is the engine record's form, read for form only.
- The double-count rule (an isolated pawn is never also backward) is not on the
  wiki. It is Ethereal's, traced above, and chesso adopts it as a *definition*
  choice with its own verdict.
- The indexing choice -- by file or by rank -- is the step's, and the record
  gives one instance of each: by file for stacked and isolated (+6.60), by rank
  for backward (+3.77). The accepts already requires the step to state which
  it chose and why; section 1 is now the evidence it chooses on.

### 3. Implementation sketch

- The three new terms, the increment and the indexing are appended as blocks
  after S123's, each with its own term, group and end constant, and
  `PS_FEATURE_COUNT` does not move: widening it would move every index from
  939 up (corrected 2026-10-30, DEC-331 (4)). `tools/eval_model.hpp` is gone
  (S276); `tools/tuner_layout.hpp`, `tools/tuner_groups.hpp` and the trace's
  `term_layout()` move in the same commit, and the tiling in
  `tests/test_tuner_groups.cpp` is what catches a missed base.
- All three terms read the same pawn fills `src/evaluation.cpp`
  `evaluate_pawns` already builds (corrected 2026-10-30: four). Anything
  needing a new pass over the board is out of scope by the same rule S102
  uses.
- The per-call cost is measured here -- DEC-047's forced-weight run,
  `adocs/data/S125_cost.sh`, with `bench_eval` beside it; `eval_spread` reads
  stage two only (corrected 2026-10-30) -- and recorded as the baseline
  S118 is asked to reclaim.

### 4. Constants and seeds

**No seeds.** Every constant is fitted (DEC-084 and the accepts). Section 1's
figures are Elo measurements of other engines' patches and are not starting
values; no engine's pawn-penalty array seeds anything here wherever it is
republished (DEC-105, DEC-134), and the wiki's connected-pawn sentence gives a
shape with no values.

The one number the step declares rather than fits is the **index width**, and
it is **(b) derived from the board**, per term (corrected 2026-10-30: this read
8 files or 6 usable ranks for every term): 8 files less the a-file baseline
the flat holds, 7, for isolated and doubled; own ranks 3 to 5 for backward,
which stands on 2 to 5 only, rank 2 the flat's; 2 to 7 for phalanx; 3 to 7
for supported, the defender a rank behind. Each comes from the rules of
chess. **No seeds** holds: every new weight starts the fit at zero, and the
flats at what ships (DEC-331 (4), (5)).

### 5. Pitfalls

- **The eight-times-in-one-commit trap.** Three new terms plus an indexing
  change over three existing ones is not one verdict. Since DEC-259 each is a
  part of an offline screen (P0 to P7, DEC-331 (6)) and the survivors share
  one `{0, 5}` SPRT, bisected A | B | C on H0 (DEC-331 (7)) (corrected
  2026-10-30: this read four terms and an SPRT verdict per group).
- **The double-count is a definition change, not a bonus.** Fixing it moves
  the *meaning* of the backward feature. S125 lands it as an increment at zero
  and folds it neutrally at the close (DEC-331 (3)), and P0 refits the three
  flats before any part (DEC-331 (5)), so nothing is carried that was fitted
  over the other meaning (corrected 2026-10-30).
- **Everything here is recomputed at every `evaluate()` call by design.** That
  is the plan order (DEC-087 (h) and (i)) and not the INV-4 hazard, which is
  about an accumulated quantity being rebuilt; these terms are not accumulated
  and never were. The cost is the number S118 later reclaims.
- **Long control reads lower.** All four traced figures shrink at 60.0+0.6s.
  A verdict taken only at 8.0+0.08s over-reports this group, and S151's
  block-boundary long-control reading is where that is checked.
- **"Weak unopposed" has no published definition as one term.** Composing one
  from CPW's *Weak Pawns* and *Open Pawns (Bitboards)* is part of the step
  (DEC-331 (2), corrected 2026-10-30); borrowing another engine's predicate by
  reading its source is not.

### 6. Measurement

One family SPRT, `{0, 5}` nElo at 8+0.08, Hash 16, `noob_3moves.epd`,
`adocs/data/S125_sprt.sh`, bounds stated in advance with the nElo worst case
and the abort rule (DEC-143); its H0 bisected A | B | C (DEC-331 (7))
(corrected 2026-10-30: this read one SPRT per group at the S105 regime, the
conditioning at the +10 class). The per-call cost is recorded whatever the
verdict is, by `adocs/data/S125_cost.sh`, with `bench_eval` beside it.

### 7. Interactions

- **S123 (before)**: the other pawn group; the two share the fills. Done,
  H1, 2026-10-30; S125's phase 1 lands on its close (corrected 2026-10-30).
- **S118 (after)**: caches what this step makes expensive. This step's measured
  per-call cost is the baseline S118's verdict is read against, which is the
  whole reason the order is not reversed.
- **S122 (after S118)**: reads the shelter and storm slots the cache adds.
- **S039 (done, H0, record `ca578de`)**: the lazy margin, kept, bounds a sum
  these terms do not enter -- they are cheap-stage terms -- but the step states
  which stage each new term lands in, as S101's accepts does.
- **S126 (block end)**: refits everything. S125 hands it no new exact
  dependency (DEC-331 (4), the audit's rank 1026 of 1073) and its fold of the
  increment at the close (corrected 2026-10-30).

### 8. References

- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+isolated+pawn
  -- commits 7c9cf64f (+10.68 +/- 6.14 / +4.00 +/- 3.01), c2d2d037 (+6.60 +/-
  4.44 / +5.55 +/- 3.56), 923864ab (+4.01 +/- 3.15 / +2.47 +/- 1.95). Commit
  messages only. Fetched 2026-09-13.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+backward+pawn
  -- commit 7afbeb6e (#121) (+3.77 +/- 3.01 / +2.33 +/- 1.84, bundled with a
  pawn PSQT retune); commit 84009a58, connected bonus not applied to backward
  pawns, no Elo stated. Commit messages only. Fetched 2026-09-13.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- v31
  connected pawns (phalanx and defender) +25.38 +/- 10.40 at 8.0+0.08s and
  +18.57 +/- 8.45 at 40.0+0.4s. Changelog entries only.
- - https://www.chessprogramming.org/Pawn_Structure -- an index; phalanx,
  supported and unopposed pawns absent; **no figure**. Fetched 2026-09-13.
- - https://www.chessprogramming.org/Backward_Pawn -- the definition quoted
  above; nothing about isolated-and-backward; **no figure**. Fetched
  2026-09-13.
- - https://www.chessprogramming.org/Connected_Pawns -- the definition quoted
  above, phalanx distinguished from the defended member, Stockfish's
  rank-based shape named with **no values**. Fetched 2026-09-13.

## 2026-10-30: phase 1

Opus 5.5 subagent, coordinated; worktree `../chesso-s125`, branch `s125` on
`42631b4` (S123's close, H1). The family landed at zero, `No functional
change`; the screen, the cost run and the SPRT pre-registered; nothing fitted,
timed or played. Every definition implemented from its description: CPW's
*Duo Trio Quart (Bitboards)*, *Defended Pawns (Bitboards)*, *Weak Pawns*,
*Open Pawns (Bitboards)*, and the accepts' own rule for the double count
(DEC-221); no engine's source was read.

### The rulings as read (DEC-331, the research pass)

- (1) Three terms -- phalanx, supported, weak unopposed -- and the fix; the
  conditioning out, recorded as unbuilt (no computable rule); no separate
  connected column.
- (2) Phalanx: an own pawn level with an own pawn on a neighbouring file, each
  member counted. Supported: an own pawn an own pawn attacks, once. Weak
  unopposed: CPW's weak set (no own pawn on a neighbouring file level or
  behind) that is also open (no enemy pawn ahead on the file). Passers in all
  three. IB: isolated and its stop square enemy-pawn-attacked, exactly the
  old backward set intersected with isolated.
- (3) IB lands as an increment at zero; the close folds it neutrally, backward
  then reading `backward & ~isolated`. Backward's rank cells read `backward &
  ~isolated` from the landing, so the fold stays neutral.
- (4) Flats kept, one baseline cell dropped where a flat stands: IB 1,
  isolated by file 7 (a dropped), doubled by file 7, backward by own rank 3 to
  5, phalanx 2 to 7, supported 3 to 7, weak unopposed 1; 30 a half, 60 in all;
  `PARAM_COUNT` 1013 -> **1073 (built: 1073)**.
- (5), (6) P0 refits the flats alone, its g recorded; P1 to P7 in order, each
  freeing its own weights, the earlier survivors' and the flats; paired d
  against the last survivor's fit or P0's; m = max(1.645 sd, 2e-6); ties under
  1000 games to the nearest supported cell, written by the audit before any
  fit.
- (7) H0: A = P0 to P4 | B = P5, P6 | C = P7, survivors only, in order;
  as the coordinator ruled it at the fast check, A = P1 to P4, a part with no
  survivor does not exist, and P0's flats never land alone (below).
- (8), (9) Stage one, summed into the pawn pair, each group compiled out at
  zero; pawns only, nothing read from a piece.
- (10) to (13): below.

### What landed

- `src/evaluation.cpp`: fourteen zero arrays, `isolated_backward`,
  `isolated_file`, `doubled_file`, `backward_rank`, `phalanx`, `supported` and
  `weak_unopposed`, mg and eg -- the code's names for the brief's IB,
  isolated by file and so on, as S123 named its arrays by their term -- their
  packed copies, a liveness flag per group, and seven blocks in
  `evaluate_pawns()` after S123's candidate passers, each computed only where
  its weights are live, in the trace build and when collecting
  (`for_each_cell()` walks a table's pawns; `own_rank()` gives the cell). The
  stage-one bound gained `structure_family_half()`, each array 15 times its
  spread. The stale comment over the flats now names S083's refit.
- `src/evaluation.hpp`: every set and cell defined; `structure_family_t`, the
  six sets and seven count arrays per colour, and `structure_family_counts()`.
- `src/eval_trace.hpp`: seven terms after `TERM_CANDIDATE_PASSER`.
- `tools/tuner_layout.hpp`: blocks `IB_`, `ISF_`, `DBF_`, `BKR_`, `PHX_`,
  `SUP_`, `WKU_`, an end constant each, `WEAK_UNOPPOSED_END == PARAM_COUNT`
  asserted; no earlier index moved. `tools/tuner_groups.hpp`: groups
  `isolated_backward`, `isolated_files`, `doubled_files`, `backward_ranks`,
  `phalanx`, `supported`, `weak_unopposed`, and tie rows by array name.
  `tools/tuner.cpp` emits the fourteen arrays (`verify_fit` 1073 of 1073 on a
  zero-epoch probe; `apply_fit` places 40 arrays). `tools/feature_audit.cpp`
  names the columns, lists them in occurrence, and prints their support under
  `structure family (S125) support:`, S123's support scoped to S123's block.
- **The trace records every coefficient**: the two counts as White's minus
  Black's on every evaluation, zero included, as the three flats are; the
  tables one entry per pawn of a set, so a cell no pawn reaches records
  nothing and two cancelling pawns record +1 and -1. The exact-reconstruction
  contract holds either way -- a zero coefficient adds nothing -- and
  `sum_record()` drops zeros before a row is stored, so the corpus stores
  non-zero cells only: 31.86 entries a row at the parent, 36.04 at the
  landing, +13.1 % (first 20000 rows of S083's corpus).
- Tests: the mirror cases (`test_evaluation`, "evaluation: structure family
  symmetry"); engine against walk over S123's population and by hand
  (`test_eval_model`, "eval counts: S125's pawn structure family", sixteen
  placements run as written and as their mirrors, every cell of every array
  reached by each colour with each sign); two non-isolated backward cases, one
  a colour, in the pawn structure hand cases, the double-count cases
  unchanged (DEC-331 (13)); every coefficient against the accessor
  (`test_eval_trace`); the packed halves (`test_evaluation`); the tiling, the
  last block and S125's ties (`test_tuner_groups`); the walk
  `structure_family_features()` (`test_walked_counts.hpp`, square by square,
  sets and cells, no engine code); the planted recoveries
  (`test_tuner_gradient`, below).
- `.tuning/verify_fit.py` 1073 and the fourteen widths;
  `adocs/data/S192_anchors.py` a `structure_family` entry on every case: e2
  isolated on the e file and weak unopposed, Re8's f7 g7 h7 three phalanx
  pawns on Black's second, checked against the engine's accessor and the walk
  by a throwaway; 10 of 10 reproduced.

### The symmetry test first (DEC-331 (10))

Written and observed before any line that scores: the blocks first collected
sets and counts alone, the score and trace lines were added after both cases
were seen red and green.

- Green over 34676 positions (`all_test_fens()` and S123's 20000 seeded
  placements, both sides to move, kept when the position and its mirror
  load): every set held on each edge file by each colour on 143 positions or
  more; every cell non-zero for each colour on 313 or more; 2582099
  assertions.
- **Red** on each planted one-colour defect, each reverted (`cmp` identical)
  and green again:
  - Black's phalanx without the edge masks: sets, phalanx 0 ==
    140741783322624 at `2k5/6p1/4K1Pp/pr1pP2P/P7/8/8/8 b - - 1 1`; counts,
    phalanx cell 1, 0 == 1, the same position.
  - Black's IB stop one rank off (`>> 16`): sets, isolated_backward 1280 == 0
    at `5rk1/p5pp/4p3/1pR3Q1/3n4/2q4r/P1P2PPP/5RK1 w - b6 0 2`; counts, 2 == 0.
  - Black's supported ranked with White's arithmetic: sets green (the set is
    right), counts red, supported cell 1, 1 == 0 at
    `1k6/7b/2p2p2/4pP2/4K3/r7/8/8 w - e6 0 1`.
  - Black's isolated files mirrored: sets green, counts red, isolated_file
    cell 0, 0 == 1 at `1NNNNNNN/2N3p1/8/p1kp1K2/8/8/8/N6N b - - 1 1`.

### INV-6

`adocs/data/S125_inv6.txt`: against `42631b4`, Release and tune builds,
`bench` 5434393, `bench 12` 2089917, `tools/search_bench.py` depth 9 c3d5
26906 / d5e6 93894 / d7c8q 35629 and depth 12 c3d5 317968 / d5e6 341523 /
d7c8q 205179, every stream byte-identical; every function of
`evaluation.cpp.o` the search reaches identical once the padding is dropped
(`adocs/data/S125_objdiff.py`), `evaluate_cheap()` 42 instructions and
`evaluate_pawns<false>`'s clone 699 in both trees and both builds; only the
collecting instantiation and its accessors differ, and the comparison reads
red on the all-forced cost build (699 -> 899).

### Coverage (DEC-331 (11))

- `adocs/data/S123_coverage_rows.cpp` scoped in its four places -- its
  features end at `TERM_CANDIDATE_PASSER`, its gradient check at
  `CANDIDATE_PASSER_END`, its both-ways check at S123's last term, its block
  at S125's marker -- its rule, picks and block unchanged: 4 pairs, 0
  ineligible, gradient smallest 5.985e-09 (family 4.669e-07, S123's phase 2
  figures), 0 one-way, "reproduces the landed block: yes".
  `adocs/data/S121_coverage_rows.cpp` needs no change (its scopes end before
  S123's terms and at S123's marker): 19 pairs, 0 ineligible, smallest
  6.584e-09, "reproduces: yes".
- `adocs/data/S125_coverage_rows.cpp`, S123's shape and S121's rule, base the
  88 rows: 15 of 60 needs open (doubled by file on b, c, e and g, backward on
  its fourth, phalanx on its fourth and sixth, both halves; supported on its
  seventh, mg); `all_test_fens()` closes none; two pairs from S083's corpus
  close all fifteen; the fixture grows 88 -> 92, all four rows VALID by
  python-chess. The constructive fallback, run once without the corpus,
  closes the same fifteen from its own draws.

### The preconditions, re-derived by script (`adocs/data/S125_preconditions.txt`)

- Fixture clear of the clamp: largest |stage two| 121.1 against 183.0.
- The clamp case's push, `S083_tuner_preconditions.cpp clamp`: **+30 still**
  by its rule (two rows bound, the farthest 197.3, the nearest other 158.8,
  every row 13.33 from the kink, finite differences 8.75e-8); no input moves.
- The gauge walk: smallest 0.0666 under Adam and 0.0760 under AdaGrad, both
  the bishop's, over the case's 0.01 floor; holds at most 1.22e-14.
- The gradient case's worst: **3.45e-7 at parameter 857,
  `mobility_eg[bishop][9]`**, against 1e-6; 2.79e-7 at 985 over the 88 rows
  before the block, which read red on S125's parameters' non-vacuity. No
  DEC-316 rows owed. The clamp case 8.75e-8; the synthetic group 6.23e-8.
- **Planted recoveries: a pawn-structure plant added.** The three flats and
  S125's `isolated_backward`, `isolated_files`, `backward_ranks`, `supported`
  and `weak_unopposed` each come back within half a centipawn, fitted alone
  from the shipped constants, and are in the case. `doubled_files` (none of
  14 halves) and `phalanx` (7 of 12) do not on the fixture's 92 rows: recorded
  unmeasured, the reason inferred -- too few phases or independent
  combinations reach their cells -- and `feature_audit --plant` over a corpus
  slice is where it would go.

### The audit, run once at phase 1

`tools/feature_audit --free pawn_structure,isolated_backward,isolated_files,
doubled_files,backward_ranks,phalanx,supported,weak_unopposed` over S083's
corpus, 2026-10-30 08:55:11 to 08:56:11 (60 s), after `S125_screen.sh`'s
pre-registration and `S125_screen.py`'s rules were written and before any
fit: rank exactly **1026 of 1073**, nullity 47 -- the 32 dead pawn columns,
S134's seven and S121's eight levels held by the freeze -- and **0 further
exact dependencies**, as DEC-331 (4) expected; no family column zero.
`S125_screen.py audit-check` passes. **After the audit** `S125_screen.sh` was
edited twice, at 08:56 and 08:57, both in its MACHINE HOURS paragraph and
neither deciding anything: the audit's 60 s, and the attribution of the
31.86 entries a row to a060435 (`S125_screen.py` last changed at 08:52:32,
before the audit); the fast check's fixes followed, the same day and before
any fit (below). The ties rule ties **2 of 60** halves:
`phalanx_mg[5]` (264 rows, 210 games) and `phalanx_eg[5]` (265, 211), a
phalanx on its own seventh, each to its sixth; the next thinnest are
phalanx's sixth (2000 and 1999 games) and supported's seventh (2525, 2528).
The screen's own `audit` stage runs it again and writes the ties.

### Guards, seen in a scratch clone

The phase 1 diff applied in a throwaway clone of the worktree under the
scratch directory, committed there as the landing; nothing of it enters the
project's repository.
- `S125_screen.py landing-check` passes at the landing and refuses a planted
  `phalanx_eg[4] = 4` and a planted flat (`pawn_structure_mg` doubled -2 ->
  -3); `screen P0` and `all` stop at it.
- `S125_sprt.sh` refuses while unpinned. A dry copy, pinned to the scratch
  landing and a family-and-flats paste on it, prints DRY-RUN-WOULD-EXEC, as it
  does with a second flat moved; it refuses a mobility continuation line
  moved beside the paste, code appended after a family `};`, a flag added to
  `CMakeLists.txt`, a family definition wrapped onto two lines, and a moved
  S123 weight; `only-family-pair` alone names `mobility_bishop_mg` and
  `passed_free_mg` in the two weight plants.
- `S125_cost.sh check` passes on the scratch landing, refuses a PARENT that
  carries the family and an unknown row.
- The cost patch, applied in a scratch copy: bench 5434393 at all eight masks,
  nine distinct `.text` sections with zero's.

### Readings DEC-331 leaves open, the phase 1 agent's, flagged

- **No survivor among P1 to P7: no candidate, and P0's refit is recorded, not
  pasted** (`S125_screen.sh`'s header): the flats' refit alone is no S125
  term, and S126 refits them. Pre-registered, for the coordinator to confirm
  or amend before `screen P0`.
- The bisection's A held P0 always until the fast check, whose ruling
  replaced it: no part for the flats alone (below).
- **The SPRT's line guard reads whole one-line family definitions only**, as
  the brief asks; a definition clang-format wraps after the paste is refused,
  not read, and is joined before pinning.
- **The cost run has no P0 row**: the flats are computed today.

### The fast check's four, fixed before completion (2026-10-30)

- **The bisection against the no-survivor rule.** `bisection_parts()` kept A
  whenever P0 existed, so with survivors in P5 to P7 alone an H0 would have
  played P0's flats alone, which the no-survivor reading forbids. The
  coordinator's ruling, as DEC-331 (7)'s "survivors only" reads: A = P1 to P4,
  B = P5, P6, C = P7, a part existing only with a survivor; P0's flats never
  land alone. **Chosen, following the screen's freeing rule**: every part that
  exists frees the flats with its groups, as every screen part from P1 on
  freed them, so they enter with the first part that exists and each later
  part refits them over what has landed. On synthetic survivor sets: {P5}
  B = P0,P5; {P7} C = P0,P7; {P2,P6} A = P0,P2 | B = P0,P6; {P1..P7}
  A = P0,P1,P2,P3,P4 | B = P0,P5,P6 | C = P0,P7; {} no part, `CANDIDATE none`,
  and `part A|B|C` each refused. `S125_screen.py`, the `S125_screen.sh`
  header, `S125_sprt.sh`'s H0 branch, `adocs/data/README.md` and this file
  say so.
- **The pre-registration's timing**: recorded above, under the audit, and in
  the header.
- **`part()`'s K unchecked** (inherited from S123, whose copy is S299's):
  `k_check` refuses a K file that is not one positive decimal, in `part` and
  in `samples` (whose existing-file skip had the same hole). Seen in a
  scratch OUT with a stand-in tuner that prints no K, the audit marked done so
  no corpus fit ran: an empty `full.k` and one holding `junk` each
  `S125-PART-FAILED` at the check; `0.6080` passes it.
- **Prose**: S123's fits restated from its reading (one quarter fit, ABC's
  q3, at the cap with its best at 4200; the other seventeen on patience,
  best reports at epochs 100 to 2000, ABC's q5 the latest; this read "by
  epoch 1200"); the cost run's builds and worktrees are ten -- zero, parent,
  p1 to p7, all -- not nine, about 1 h 55 m from S123's 67 min for six
  comparisons and six builds.

### Gates

Both builds without a warning; both fast suites 50 of 50, sequential;
`./clang-format.sh --check` clean (`CLANG_FORMAT_MAJOR=22`); the S297 clang
pair (`STAGES=clang tools/gate_extra.sh`), `build-clang` and
`build-clang-tune` with clang 22.1.8, no warning, both fast suites 50 of 50,
356 s; `tools/plan_prose_check.py`, one mode per call, each exit 0:
`--touches` 0 flagged over 19 files, `--params`, `--citations` 0 flagged over
19 files, `--gate` agrees, `--prose` 0 flagged.
`tools/gate_extra.sh` is not run: no search, generator or make/unmake change,
so DEC-141's tier two does not bind.

### Commands for the coordinator, in order

1. Commit phase 1; rebuild `build/` at the commit in the main checkout.
2. `nohup adocs/data/S125_screen.sh all > .tuning/s125_screen_all.console 2>&1 &`
   -- locate under a minute, audit about 1 min, P0 to P7 of six fits each,
   about 18 min a part: about 2 h 25 m typical, 4 h 50 m at the 5000-epoch
   cap; watcher ceiling 10 h; `S125-ALL-DONE|FAILED`. Daytime. Confirm the
   no-survivor reading above first.
3. `CAND_SHA=<the landing> PARENT_SHA=<its parent> adocs/data/S125_cost.sh check`,
   then `run`: about 1 h 55 m (ten builds, ten comparisons), ceiling 4 h,
   the machine idle;
   `S125-COST-DONE|FAILED`. Any time after step 1, not beside step 2.
4. `adocs/data/S125_screen.sh apply <a clean worktree at HEAD>`, about 8 min,
   then phase 2 (an agent): every golden the paste moves, each by its script
   (S123's phase 2 table is the list; the anchors with pawns are e2 and Re8).
5. Pin `adocs/data/S125_sprt.sh` (the bracketed lines) and run it at night: up
   to 41900 games, 19.86 h; watcher ceiling 40 h; on H0, `S125_screen.sh part
   A|B|C` (about 17 min each) and up to three more SPRTs.

## 2026-10-31: the screen and the candidate

One Opus 5.5 subagent, worktree `../chesso-s125b`, branch `s125b` at `f08d9db`
(`b5f133c`, the phase 1 landing, with S299 to S302 on top, none touching `src/`
or `CMakeLists.txt`). Written in two halves: the reading while the coordinator's
cost run held the machine, the runs after its "go" and its rulings on the half 1
points. Every reading with its command is in `adocs/data/S125_goldens.txt`. No
match, SPRT, fit, datagen or timing ran. Every golden the paste moves is
re-derived by its script, every guard held, and nothing stopped for the owner.
One question went to the coordinator and came back ruled: the planted recovery
(below).

**The screen's launch, by the coordinator.** `S125_screen.sh all` started
2026-10-31 00:35 from the main checkout at `f08d9db`, PID 8772, console
`.tuning/s125_screen_all.console`, OUT `.tuning/s125`, and read `S125-ALL-DONE`
by 03:44; load about 1 at launch, 35 GB free. HEAD is not the phase 1 landing
`b5f133c` but a descendant whose `src/` and `CMakeLists.txt` are the landing's
byte for byte: `git diff --stat b5f133c f08d9db` over `src`, the tuner's
sources, `feature_audit`, `CMakeLists.txt` and `tests` is empty, and
`cmake/pgo.cmake` differs by S302 alone. `landing-check` passed at launch. Two
earlier runs are void and no reading (the step file and `status.md`):
`.tuning/s125_void1/`, `.tuning/s125_void2/`. The no-survivor reading in the
screen's header was confirmed by DEC-331 (14) before P0. Open findings (BUGS as
scoped by DEC-171): S304, tools only, `depth_vs_eval.py` and `spsa_driver.py`,
neither reached by the screen; S303, the filler open at this launch, landed at
`9cbeb68` and left S304 as its residuals.

### The screen, as read

`adocs/data/S125_screen.sh all`, on S083's corpus (6226944 rows, sha256
`2e0e3de9...`) against S082's qs validation view. The eight readings, the corpus
line and every fit's and tie file's sha256 are in
`adocs/data/S125_screen_reading.txt`.

| part | against | full gain | s | m | verdict |
|---|---|---|---|---|---|
| P0, the three flats | its start (g, DEC-315) | +0.000007 | 0.000003 | 0.000005 | recorded |
| P1, the isolated and backward increment | P0 (paired d, DEC-326 (3)) | -0.000004 | 0.000002 | 0.000003 | dropped |
| P2, isolated pawns by file, b to h | P0 | +0.000012 | 0.000003 | 0.000004 | kept |
| P3, doubled pawns by file, b to h | P2 | -0.000002 | 0.000006 | 0.000011 | dropped |
| P4, backward pawns not isolated by own rank, 3 to 5 | P2 | +0.000005 | 0.000005 | 0.000008 | dropped |
| P5, phalanx pawns by own rank, 2 to 7 | P2 | +0.000036 | 0.000002 | 0.000003 | kept |
| P6, supported pawns by own rank, 3 to 7 | P5 | +0.000031 | 0.000003 | 0.000005 | kept |
| P7, weak unopposed pawns | P6 | +0.000017 | 0.000003 | 0.000005 | kept |

- Survivors P2, P5, P6, P7: `CANDIDATE 02567 groups
  pawn_structure,isolated_files,phalanx,supported,weak_unopposed`. The candidate
  is `.tuning/s125/fits/02567/full.hpp` (sha256 `176755da...`): best validation
  0.071322 from 0.071425, at epoch 3000, stopped on patience.
- The bisection order printed: A = P0,P2 | B = P0,P5,P6 | C = P0,P7.
- **Ties, as the tie files hold them.** `ties_P5.txt` holds two lines:
  `phalanx_mg 5 4` (rows 264, games 210) and `phalanx_eg 5 4` (rows 265, games
  211), a phalanx on its own seventh sharing its sixth's cell. The other seven
  files hold their two comment lines and no tie.
- **The cap, as read; it decides nothing.** 7 of the 48 fits stopped at the
  5000-epoch cap: 0's full (best at 4600) and q5 (3400), 02's q5 (3800), 025's q2
  (4300) and q5 (3700), 0256's q2 (3100), and 02567's q5 (5000). The other 41
  stopped on patience, five of them with their best at epoch 0: 0's q1, 01's q1
  and q5, and 023's q3 and q5.
- Stage-one headroom at 02567: mg 24227 and eg 27929 of 32767 (8540 and 4838 of
  room).
- **The full fit's tables, recorded; they decide nothing**, and monotonicity is
  not imposed (DEC-331 (6)). Isolated by file b to h; phalanx own rank 2 to 7;
  supported own rank 3 to 7; the flats isolated, doubled, backward.

      pawn_structure_mg  {1, -7, -6}                    was {-9, -2, -11}
      pawn_structure_eg  {-11, -20, 0}                  was {-15, -18, -2}
      isolated_file_mg   {1, -6, -11, -10, -3, -7, 1}
      isolated_file_eg   {-3, -1, -2, 0, -1, 3, 2}
      phalanx_mg         {5, 2, 7, 20, 86, 86}
      phalanx_eg         {-1, 1, 4, 7, 68, 68}
      supported_mg       {8, 5, 7, 32, 187}
      supported_eg       {11, 0, 4, 26, -29}
      weak_unopposed_mg  {-10}
      weak_unopposed_eg  {-6}

  `isolated_backward`, `doubled_file` and `backward_rank`, dropped, stay zero and
  are in no expression at the candidate.
- **The cells far from their neighbours** (`adocs/data/S125_cells.txt`, by
  `adocs/data/S125_cells.py`: the value less its neighbours' mean, S123's cutoff
  |v-nbr| >= 10, deciding nothing), rows and games from the audit:
  - `supported_mg` own 7th, 187 (+155.0), 9476 / 2525; own 6th, 32 (-65.0), 99795
    / 17225; own 5th, 7 (-11.5), 1153721 / 95910;
  - `supported_eg` own 7th, -29 (-55.0), 9515 / 2528; own 6th, 26 (+38.5), 98067 /
    17057; own 3rd, 11 (+11.0), 2694291 / 133350;
  - `phalanx_mg` own 6th, 86 (+33.0), 4088 / 2000; own 5th, 20 (-26.5), 77008 /
    23000;
  - `phalanx_eg` own 6th, 68 (+30.5), 4086 / 1999; own 5th, 7 (-29.0), 71647 /
    21545.

  The phalanx seventh shares the sixth's cell by the tie, so its 86 and 68 stand
  on the sixth's support: the two halves' 2000 and 1999 games are about twice the
  tie rule's 1000, and the seventh's own 210 and 211 are under it. The end cells of a
  row have one neighbour, so a steep row's last cell reads far by construction. A
  reading of the fit, not a chess judgement, and no constant is changed for it
  (DEC-084). Recorded for S126, which refits them.

### The apply

`S125-APPLY-DONE` (console and OUT `.tuning/s125/apply_02567/`).
- start-check passed: 49 definitions against HEAD `f08d9db`, every held value
  HEAD's.
- headroom: mg 24227, eg 27929 of 32767.
- `verify_fit`: 1073 of 1073 identical to the emitted header. only-family: ten
  arrays moved, `isolated_file`, `pawn_structure`, `phalanx`, `supported` and
  `weak_unopposed`, mg and eg, nothing else.
- Both builds with no warning. Bench 5434393 -> **5707449**.
- 46 of 50 in each build. The reds: `test_evaluation` (the pawn on e2, 107
  against 109), `test_search` (Re8, 199 against 196; then the ProbCut row, no
  mate at its depth), `test_tuner_gradient` (the planted recovery on
  `supported`'s own seventh) and `test_eval_trace` (the truncation pins).
  `test_engine`, `test_mate_breadth`, `test_mate_carry` and `test_mate_pv` were
  green at the paste.

### The coverage rows at the candidate

All three tools, run on the candidate's constants against S083's corpus, as
their headers say.

- **`adocs/data/S121_coverage_rows.cpp` refused**: landed block 19 pairs, 1 not
  eligible (`k6K/8/8/8/8/8/1ppppppp/NqNNNNNN w - - 0 2` and its mirror, past the
  800 cp, seven pawns on their own seventh each a phalanx member), "reproduces
  the landed block: NO". Coverage itself held: 0 parameters at or under 1e-12
  (smallest 3.250e-08), 0 features one way.
- **By the coordinator's ruling, S121's ruling (b), the tool's picks replace the
  block.** 19 pairs still, so S123's and S125's base counts stand: queen 3 is now
  `3k4/8/1K6/8/8/8/1ppppppp/RqRRRRRR w - - 0 2` and its mirror, both VALID by
  python-chess, picked earlier in the list; the other 18 pairs are as they were.
  The constructive fallback was not needed.
- Re-run on the new fixture, all three reproduce, no parameter open:
  - S121: 19 pairs, 0 ineligible, from the base 42, 55 of 132 needs open;
    gradient smallest 3.250e-08; 0 one-way; "reproduces: yes".
  - S123: 4 pairs, 0 ineligible, from the base 80, 11 of 58 open, the same 8
    picks; gradient smallest 2.955e-08, over its family 5.423e-07; 0 one-way;
    "reproduces: yes".
  - S125: 2 pairs, 0 ineligible, from the base 88, 15 of 60 open, the same 4
    picks; gradient smallest 2.563e-09, over its family 1.452e-07; 0 one-way;
    "reproduces: yes".

### The preconditions, each by its script

- Fixture clear of the clamp: 121.1 against 183.0, unmoved (stage two does not
  move with a stage-one family).
- The clamp case's push, `S083_tuner_preconditions.cpp clamp`: **+30 still** by
  its rule: four rows bound, the farthest at 197.3, the nearest other at 158.8,
  every row 13.25 from the kink, finite differences 1.01e-7.
- The gauge walk: **0.0124 under Adam**, the pawn's, and 0.0538 under AdaGrad, the
  bishop's, over the case's 0.01 floor by a factor of 1.24; held at most
  1.35e-14.
- The gradient case's worst (`test_tuner_gradient -s`, both builds): **9.22e-7 at
  parameter 800, `mobility_mg[rook][4]`**, against 1e-6, green by a factor of
  1.08, the thinnest since S121's 9.50e-7. No DEC-316 rows owed; they are the
  answer if a later fit passes the bound, which does not move. The clamp case
  1.01e-7; the synthetic group 3.62e-8.
- **Planted recoveries: red at the paste, the coordinator's ruling, green.** At
  6000 epochs `supported`'s own seventh did not come back: 16.735 against plant 11
  (parameter 1065) and 8.870 against plant 12 (1070). The cause is the distance
  and not the reach, because the paste moved the fit's start to 187 and -29. A
  scratch copy of the case, the tracked test untouched, bound 0.5, `supported`'s
  plant alone:

  | epochs | mg[4] | eg[4] | worst of the ten cells |
  |---|---|---|---|
  | 6000 | 16.735 | 8.870 | 5.735, red |
  | 8000 | 12.896 | 10.964 | 1.896 |
  | 10000 | 11.625 | 11.658 | 0.625 |
  | 12000 | 11.206 | 11.887 | 0.206 |
  | 16000 | 11.022 | 11.988 | 0.022 |
  | 24000 | 11.000 | 12.000 | 0.000 |

  The ruling, DEC-331 (15): the same fit and the same 0.5, `supported`'s plant at
  16000 epochs and every other plant at 6000; the budget's reason is at the site.
  **The 16000 is a budget and not a golden**: the case reads the recovery within
  0.5, a budget too small goes red, as the table shows, and the site carries no
  GOLDEN tag. The plant and the bound do not move, so the assertion stays as
  strict. The site's comment already held the class: 3000 -> 6000 for passed
  bucket 1. `doubled_files` and `phalanx` stay unmeasured here, as phase 1
  recorded.

### Every golden, each by its script

| golden | before | now | script |
|---|---|---|---|
| six piece anchors | 109, 283, 353, 590, 1021, 0 | **107**, 283, 353, 590, 1021, 0 | `S192_anchors.py`: 10 of 10 at the landing, then on the fit, then 10 of 10 on this tree. Only the pawn on e2 holds a pawn |
| `QUIET_ROOK_EVAL`, `_CHEAP`; 394 | 590, 578; 394 | unmoved: no pawn | the same |
| `black_static`; quiet evasion; repetition | 196; -541; -603 | **199** (f7, g7, h7, three phalanx pawns); -541 unmoved, the four leaves pawnless and still the four king moves standing pat; -603 unmoved, no pawn | the same |
| `POSITIONAL_ROOM` | -- | did not fire: the white pawn's residual is \|107 - 84\| = 23 against 150 | -- |
| `truncation_positions` | S123's four, phases 5, 7, 13, 17 | **re-picked**: phases 5, 11, 17 and 23. 111 past 2.8, 35 at 2.875, 31 distinct. The worst is 2.875 = 69/24, so N stays 3, tempo zero and DEC-322's fourth division not arising. Of S123's four, two read 1.917 and 1.958 and the worst 2.75 | `truncation_scan --data <S083's corpus> --min 2.8`, then `S083_truncation_pick.py` |
| coverage blocks | 19, 4, 2 pairs | S121's **queen 3 pair replaced** by the tool's pick; S123's and S125's unmoved; all three reproduce | `S121_`, `S123_`, `S125_coverage_rows.cpp` |
| clamp push; walk | +30; 0.0666 / 0.0760 | +30 unmoved; **0.0124 / 0.0538** | `S083_tuner_preconditions.cpp both` |
| fixture clear of the clamp | 121.1 | 121.1 against 183 | the same |
| gradient check (DEC-316) | 3.45e-7 at 857 | **9.22e-7 at 800** | `test_tuner_gradient -s` |
| `EXACT_FLOOR` 159 | 160 / 153 | **held**: 168 shipping, 158 weakened; the gate at RfpMinPly 1 red at 158 | `S156_mined_floor_sweep.py --floor 159` via `S136_overlay_run.py` (the paste's one file, `src/evaluation.cpp`) |
| `MATE_IN_THREE_FLOOR` 11 | 11 / 10 | **held**: 12 shipping, 10 weakened; `red` red at 10 >= 11 and on mate-in-two exactness | `S154_floor_margin_sweep.py floor`, `red`, the same overlay |
| node budget | 65024 / 3251 (12104, 0.74 of c) | unmoved: 10306 nodes, 0.63 of c, inside [7270, 32512] | `S192_node_budget.py` |
| first-mate; `DRIVE_DEPTH`; ply-2 | 9, 9, 5; 4; 6 | unmoved | `S188_repair_goldens.py first-mate`, `s207`; `S083s_rfp_ply2.py` |
| `capture_mates`; extra ply | 9, 8, 11, 12; 8 (mate 4) | unmoved: each at its asserted depth (the fourth capture row at 9 and 12); mutant separation not re-read, every case green | `S230_mine_r01_row.py depths`, candidate and parent |
| `mate_probcut_hides` | S121's row, depth 10, mate in 4 | **re-mined**: `1r3k2/3Q4/p2N1p2/2pqp3/2P4P/1P4PK/8/1R6 w - - 0 52`, depth 9, **mate in 5**. Shipped `d9` against B04's `d10` in the range 7 to 10; `d9 d11 d12` over 3 to 12. 3 of 269 separate, two by distance only; the guard-mode witness fires at 7 to 10. Stockfish at 5000000 nodes reads `#+5`, and python-chess reads its line as ending in mate. Observed red under B04, green shipped. S121's row read no mate at any depth from 3 to 12 | `S277_mine_mate_row.py`, six stages, guard mode |
| `LATE_CAPTURE_*` | LQ02 at 7, LQ03 at 3 | unmoved, node counts apart | `S261_mine_late_capture.py sweep`, `pick` |
| aborted-line cell | line 101 at 40000 (30000..60000) | **line 114 (game 4543) at 250000** (run 150000..360000; margin 100000 below, 110000 above; 4 of 318 separate). Line 101's run stays 30000..60000 and the rule takes the longest run. Observed red under MW02 (9 >= 11), green shipped | `S294_fallback_cell.py`, four stages, MW02 on a throwaway tree of the candidate |
| `ALPHA` | -2108 (band -2153 .. -1923) | **-2108 kept**: band -2165 .. -1935, 57 below and 173 above; the trace meets the node at -2013, -2088, -2204 and -2383 | `S121_alpha_window.cpp` (it reproduces S123's band on the parent); `S136_trace_patch.py` |
| `short_line_ceiling` | 2, 13, 4, 1, 2, 3 | unmoved; the command gains `adocs/data/S125_sweep_s170.txt` | `S203_case_sweep.sh --ceilings`: the ten grids plus S125's |
| S170 budgets | A 1200000, B 100000, C 1200000, D 1500000, E 300000, F 100000 | **A 1000000, C 1000000, F 500000**; B, D and E unmoved | `S170_pick_budgets.sh` |
| readings | 697 mate_pv lines; defender 86/104 | 699; 86/104 | `test_mate_pv -s`; `S165_nmp_defender_sweep.py` |
| `golden_defaults`, option count, census, book | -- | unmoved by construction | -- |

**DEC-299's guard is met**: both mate floors hold and no ceiling moves, so the
budgets the picker answers are applied, with a header paragraph in
`adocs/data/S170_cases.tsv`, and the ceilings' command gains S125's grid
(`tests/test_mate_carry.cpp`, `DEV_MANUAL.md`). S125's grid: 108 cells, 1404 mate
lines, 22 short; it alone answers 0, 1, 1, 0, 2, 2.

**The floors' margins.** `MATE_IN_THREE_FLOOR` 11 is one below the 12 that ships
again, where DEC-330 left it at zero, and one above the weakened 10: on an H0 the
paste reverts and the zero margin returns. `EXACT_FLOOR` 159 is 9 below the 168
that ships and 1 above the weakened 158. A floor that holds is not moved here; one
below the shipping count, 167, is a rise for the coordinator to propose.

### Gates

- Both builds: 0 warnings. `ctest -L fast`: **50 of 50** in `build/` and in
  `build-tune/`, sequential, `CLANG_FORMAT_MAJOR=22`.
- `./clang-format.sh --check` clean.
- **The S297 clang pair** (`STAGES=clang tools/gate_extra.sh`): `build-clang` and
  `build-clang-tune` with clang 22.1.8, no error and no warning; both fast suites
  50 of 50; 357 s.
- `tools/plan_prose_check.py`, one mode per call, each exit 0: `--citations` 0
  flagged over 19 files; `--touches` 0; `--params`; `--gate` agrees; `--prose` 0
  flagged.
- **`tools/gate_extra.sh`**, all six stages, on the candidate tree, detached under
  a four-exit watcher: `GATE-EXTRA-DONE 6 stages 1441 s`. prose 0 s, citations 0 s,
  debug 394 s (6 of 6), sanitize 736 s (the fast label 50 of 50 under the
  sanitizers; INV-6 across builds, both 5707449), perft 51 s, clang 260 s (50 of 50
  twice). No `Assertion` in its logs.
- **The ruled plant under the sanitizers**: `test_tuner_gradient`, its `supported`
  plant at 16000 epochs, passes in 79.86 s against that build's 600 s timeout
  (12.47 s and 12.29 s under clang), so nothing more is owed on it.
- **Tier 2 (DEC-141) does not bind**, by what the diff touches: under `src/` only
  `src/evaluation.cpp`, its ten arrays and whole-line comments; no `make_move`,
  `unmake_move`, generator or search change. `gate_extra` ran anyway, as the brief
  asks, because the candidate puts the family's code into every stage-one
  evaluation. The Debug self-play is the coordinator's.
- **Bench 5707449** in both builds: the commit's `Bench:`.

### The cost (the coordinator's run)

`adocs/data/S125_cost_reading.txt`, 2026-10-31 03:44:37 to 05:43:05, `b5f133c`
against `63d5bcf`, `S125-COST-DONE` (`.tuning/s125_cost.console`): INV-6 bench
5434393 on every binary; nine distinct `.text` sections; 16 of 16 pairs on every
row.
- all **+4.20 %**; p2 **+1.18 %**, p3 **+0.65 %**, p4 **+0.55 %**, p5 **+1.36 %**,
  p6 **+1.05 %**: cycles and wall beyond the floor, instructions the same way.
- p1, p7 and parent: zero, inside the A/A floor.
- The A/A floor: bench16 cycles 0.315 %, pos11 cycles 0.380 %, wall 0.364 %.
- Forced instructions, printed and not gated: bench16 the seven rows sum to
  +4.866 % against all +4.642 %; pos11 +4.394 % against +4.007 %.

**`all` is not the candidate's set**: it also forces P1, P3 and P4, which the
candidate drops. The candidate's set is mask 114, P2, P5, P6 and P7, so **the row
`cand`, `S125_COST=114`, is added**, by the coordinator's ruling, to
`adocs/data/S125_cost.sh` and `S125_cost.py` in the existing rows' form: nested
inside all and over p2, p5, p6 and p7 in the reading's ordering check. Nothing was
timed.

**That holds for CAND and not after the close.** The close's fold (DEC-331 (3),
(15)) makes `isolated_backward` live, so the engine then also computes P1 and the
`& ~isolated`. The fold adds P1's forced row, +0.37 % instructions, read zero with
its cycles and wall CIs reaching inside the A/A floor, and S118's baseline is read
with both rows, `cand` and p1. No further row is added.
- `check` with `ROWS=cand`: `rows aa (zero against its copy), cand (S125_COST=114),
  each against zero`, `S125-COST-CHECK-DONE`. `p9`, `cand cand` and a PARENT that
  is not CAND^ each refuse.
- `S125_cost.py read` over the first run's pairs, its nine rows named, reproduces
  its reading line for line (0 diff lines).
- The row builds: `b5f133c` with the patch at `S125_COST=114` in a scratch tree,
  Release, padded, no warning, bench 5434393 and `bench 12`'s stream equal zero's;
  its `.text` (`7eb198c6...`) is none of zero's, p2's, p5's, p6's, p7's or all's.

**The command, from the main checkout, once the candidate's commit carries the
row**, machine idle, after the gates and before the SPRT:

```
cd /home/max/ws/chesso
CAND_SHA=b5f133c PARENT_SHA=63d5bcf ROWS=cand OUT=/home/max/ws/chesso/.tuning/s125/cost_cand adocs/data/S125_cost.sh check
CAND_SHA=b5f133c PARENT_SHA=63d5bcf ROWS=cand OUT=/home/max/ws/chesso/.tuning/s125/cost_cand \
  nohup adocs/data/S125_cost.sh run > .tuning/s125_cost_cand.console 2>&1 &
```

It times `aa` and `cand`, so the row comes with its own session's A/A floor: about
25 min (two builds, two comparisons, the streams; S121's one-row run took 23 min),
watcher ceiling 50 min, `S125-COST-DONE|FAILED`. `OUT` is fresh because a run
refuses an OUT holding a `reading.txt`, which `.tuning/s125/cost/` does. No guard
is weakened. The reading's place is `adocs/data/S125_cost_reading.txt`'s RUN 2 and
the next paragraph.

**The row `cand`'s reading, the coordinator's, 2026-10-31: COST +3.50 %**, cycles
and wall beyond the floor, instructions the same way: bench16 instructions +3.782 %,
cycles +4.153 %; pos11 instructions +3.213 %, cycles +3.112 %, wall +3.248 %; its
session's A/A floor bench16 cycles 0.271 %, pos11 cycles 0.139 %, wall 0.227 %.
`adocs/data/S125_cost_reading.txt`, RUN 2.

**`bench_eval`'s per-call figure, the coordinator's, at the close:** REF `9cbeb68`
63.79, 63.87, 64.30 ns, mean **63.99**; CAND `cf8661b` 74.50, 73.91, 73.77 ns, mean
**74.06**; the fold 75.64, 76.36, 75.80 ns, mean **75.93**: machine idle, interleaved,
3 rounds each of `-r 9 -n 4000000`, resolutions 0.0 to 0.3 %
(`adocs/data/S125_bench_eval.txt`, `adocs/data/S125_cost_reading.txt` RUN 3). CAND over
REF +15.7 %; the fold over CAND +2.5 %. It is evaluate() alone on ten compiled-in
positions, so it does not compare with the whole-engine rows, +3.50 % `cand` and p1's
+0.37 % instructions. CAND and the fold read the same checksum, -13730545627483, the
neutrality again. The fold's +2.5 % is under DEC-266's 3 % line for layout and clears its
2 % workstation rule on one layout only: one layout, not perturbed, deciding nothing.
**S118's baseline: 75.93 ns on the tree it starts from.**

**Debug self-play, the coordinator's, at `cf8661b`:** 8 games at 4+0.04, 0
`Assertion`, 0 `disconnect`, in the form DEV_MANUAL.md gives
(`.tuning/coord/S125/debug_selfplay/`).

### The SPRT script

`adocs/data/S125_sprt.sh`, re-read against this candidate. The markers are left
unfilled and the script refuses on them before anything else.
- **The landing guard.** `landing=b5f133c` would refuse every descendant, since
  S302 changed `cmake/pgo.cmake`: seen by git, `b5f133c` is an ancestor of
  `f08d9db`, `src` and `CMakeLists.txt` equal, `cmake/pgo.cmake` differing. By the
  coordinator's ruling the bracket stays, its text names `f08d9db`, the commit the
  screen ran from, `phase1="b5f133c"` is a filled constant, and THE LANDING
  paragraph says so in words. The run-time check: `phase1` is an ancestor of
  `landing`, and the build-path diff between them is exactly `cmake/pgo.cmake`.
- **The header's parts** now name the survivors P2, P5, P6 and P7 and the three
  dropped, P1, P3 and P4, with their figures.
- **The line guard narrowed**, a strengthening, by the coordinator's ruling: from
  the sixteen arrays to the ten that move. Before it, nothing enforced "a dropped
  group's arrays stay zero". The weight-by-name check still reads the sixteen.
- **How the guards read a part.** A new booked setting, `part=`, empty for the
  family or A, B or C, is re-pinned with `ref=` and `cand=` for a part's run; each
  part reads the same line test with its own subset, never a wider list. CAND may
  move A: flats, `isolated_file`; B: flats, `phalanx`, `supported`; C: flats,
  `weak_unopposed`. REF may carry past `landing` nothing for the family and A; A's
  arrays for B; A's and B's for C, read by the same line test from `landing` to
  REF, because a later part plays over what has landed. The family's guard on REF
  is as it was, and REF must also descend from `landing`.
- **The H1 row**: "A dropped group stays at zero" was false of IB after the fold;
  it now says that with P1 dropped the increment is zero, so IB's weight at the
  fold is backward's flat alone, {-6} and {0}, and P3 and P4 stay at zero.
- **The H0 and No verdict rows** now say the fold is owed on those outcomes too,
  with what it is folded over: on a family fully H0, the parent's flats, {-11} and
  {-2} (DEC-331 (15)).
- **WHAT THE VERDICT PRICES** says that the dropped groups cost nothing at CAND,
  and that the close's fold, which this verdict does not price, adds P1's row.
- **The H0 row**: the partition as the screen printed it and each part's model, A
  `pawn_structure,isolated_files`, B `pawn_structure,phalanx,supported` with P5's
  two ties, C `pawn_structure,weak_unopposed`, each refit over what has landed, so
  B's and C's models under H0 are not the screen's. "At worst three more runs after
  this one, four in all" holds.
- **Its mode.** The file was 100644 where its siblings are 100755, so the header's
  `nohup adocs/data/S125_sprt.sh` would have read `Permission denied` at launch;
  now 100755. Phase 1's guards were shown through a dry copy, which is why it did
  not show.
- **Shown on a scratch pin** (`adocs/data/S125_goldens.txt` section 11), a
  throwaway clone, each pair committed there, the exec replaced by an echo: the
  real pair over a tools-only descendant, S303's shape, prints
  DRY-RUN-WOULD-EXEC, and so do part A; B with A at H0 and with A landed; C with
  nothing, A, B, and A and B landed. **Refused, 23 of 23**: a dropped group's array
  moved (three plants); S123's weight; a mobility continuation line; code appended
  after a `};`; a wrapped definition; a flag in `CMakeLists.txt`;
  `src/evaluation.hpp` touched; `landing` the phase 1 commit itself, not descending
  from it, with `src/` moved, and with a second build file moved; REF's
  `cmake/arch.cmake` moved, a part landed in the family's REF, REF older than
  `landing`; part A moving `phalanx`, part B moving `weak_unopposed`, part B moving
  A's landed array, part A over a landed part, part B's REF carrying
  `weak_unopposed`, part C's REF carrying a dropped array, and part D.

### What the close owes after P1's drop: the ruling, DEC-331 (15)

The accepts says "an isolated pawn is never also counted backward", with no
condition, and DEC-331 (3) and (13) put the fold at the close. **By the
coordinator's ruling, DEC-331 (15), the fold is owed at S125's close on every
verdict.** It is neutral (INV-6). P1's drop sets the increment to zero, so IB takes
backward's flat as it stands on the tree at the close: {-6} and {0} on H1; on a
family fully H0, the parent's flats, {-11} and {-2}; with a part landed, that part's
refit of them. Rejected: IB scoring isolated alone, which DEC-331 rejected and which
is unmeasured; and amending the accepts with no fold, which needs the owner's words
for a clause the fold already satisfies.

### Also changed

- `src/evaluation.cpp`: whole-line comments only beside the ten arrays: the
  family's paragraph (the 02567 fit, pasted unchanged, the dropped groups at zero),
  the flats' provenance, and the stage-one bound's 24227 and 27929. `src/evaluation.hpp`
  is untouched: the SPRT guard allows `src/evaluation.cpp` alone, and the definitions'
  comments wait for the closing commit.
- `adocs/data/S192_anchors.py`: the two anchors and the family's note.
- `DEV_MANUAL.md`: every goldens row S125 moved or re-read, the `--ceilings` command
  with S125's grid, the gradient and walk sentences. Where the step file and
  `DEV_MANUAL.md` differ, the manual's commands were followed; they did not differ on
  any command run here. `.tuning/coord/S125/brief_inputs.md` section 5 is stale where
  it differs from the manual: its walk and gradient figures are S123's, and it omits
  `mate_probcut_hides` and `mate_the_extra_ply_hides`, both of which the manual carries.
- `MANUAL.md`: the `RfpMinPly` row's mined-set figures (168, 162, 158 against 159).
- `adocs/data/README.md`: rows for `S125_screen_reading.txt`, `S125_cost_reading.txt`,
  `S125_sweep_s170.txt`, `S125_cells.py`, `S125_cells.txt` and `S125_goldens.txt`,
  and the `cand` row in `S125_cost.sh`'s and `S125_cost.py`'s.
- `tests/test_eval_trace.cpp`: the header's sentence on what ships at zero.

### Proposed commit message

```
Paste S125's pawn structure fit for its SPRT (S125)

The screen kept isolated pawns by file, phalanx, supported and weak
unopposed pawns and dropped the increment, doubled by file and backward
by rank (DEC-331). Their full-corpus fit, 02567, is pasted over ten
arrays, the three flats among them, for the one {0, 5} SPRT that decides
the family (DEC-259); an H0 is bisected A | B | C.

Every golden the paste moves is re-derived by its script (DEC-142): the
anchors 107 and 199; the truncation pins re-picked, N still 3; S121's
queen 3 pair replaced by the coverage tool's pick; the ProbCut row
re-mined; the aborted-line cell at line 114, 250000 nodes; ALPHA kept
inside its band. Both mate floors hold: EXACT_FLOOR 159 against 168 and
158, MATE_IN_THREE_FLOOR 11 against 12 and 10. No mate-carry ceiling
moves, so the picker's budgets for A, C and F apply (DEC-299). The
supported plant takes 16000 epochs, a budget and not a golden: the paste
moved its start, the bound and the plant did not move (DEC-331 (15)).

S125_sprt.sh's landing guard holds REF to f08d9db, the commit the screen
ran from, and checks that against b5f133c; its line guard reads the ten
arrays that move, a part's own subset by part=. S125_cost.sh gains the
row cand, the candidate's set; S118's baseline reads it with p1, which
the close's fold adds. The fold is owed on every verdict (DEC-331 (15)).

Bench: 5707449
```

No `done:` stamp: the SPRT comes first.

## 2026-10-31: the SPRT read H1

One Opus 5.5 subagent, in `../chesso-s125d` (branch `s125d` at `9bcafb9`, the
SPRT's pin), read only over the run's outputs and binaries. No match, fit or
timing ran in it; the `bench_eval` reading below is the coordinator's, after it.
Not comments only: the fold is code, and neutral.

### The run

`adocs/data/S125_sprt.sh` as pinned at `9bcafb9`: `cf8661b` (the candidate, the
screen's 02567 fit over ten arrays, bench 5707449) against its parent `9cbeb68`
(the phase 1 landing's `src/`, the family at zero, bench 5434393), launched
2026-10-31 07:52:21, `SPRT-RUN-DONE` after fastchess's Total Time 01:12:21:

```
SPRT | cand cf8661b vs ref 9cbeb68, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | 22.89 +/- 10.02, nElo 31.16 +/- 13.60
LLR | 2.96 (-2.94, 2.94) -> H1
Games | N: 2508 W: 776 L: 611 D: 1121, Ptnml [70, 269, 465, 326, 124]
Wall | 1 h 12 m, 2079.9 games/h, forfeits 0
Log | adocs/data/S125_sprt.log
```

`Wall`'s rate is N over fastchess's `Total Time`, S296's, S136's, S121's and
S123's rule: 2508 games in 4341 s.

### The inspection, before the verdict was read as the record

- **Banner** (the console's): candidate `cf8661b 2026-10-31`, reference
  `9cbeb68 2026-10-31` (the commits' dates; their times, 07:21 and 05:55, are
  `git log`'s and the pinned sentence's, and fastchess.sh prints the date alone,
  as at S123), `arch native tune off`, padding on both, 8+0.08, hash 16, 12 of 12
  cores, `noob_3moves.epd`, seed 20261031075221, `elo0=0 elo1=5 alpha=0.05
  beta=0.05`, cap 20950 rounds and 41900 games from `SPRT_ROUNDS`, out
  `.tuning/sprt_s125_20261031_075221`; no `diff`, `relay` or `A/A CALIBRATION`
  line; `id name`s `cand-cf8661b Chesso cf8661b native` and `ref-9cbeb68 Chesso
  9cbeb68 native`; `Started game 1 of 41900`. As the pin says. Above it the
  script's guards printed the ten arrays as the only weights moved and
  `only-family-pair: every weight outside the family and the flats is REF's`.
  The governor reads `performance` at this reading.
- **Binaries**: `.ref-builds/cf8661b/build/src/chesso` benches **5707449** and
  `.ref-builds/9cbeb68/build/src/chesso` **5434393**, both worktrees clean at
  their sha. Written 07:52:27 and 07:52:33 and not since; the first game's
  `GameStartTime` is 07:52:33, the last `GameEndTime` 09:04:51.
- **Crashes, disconnects, illegal moves**: none. `fastchess.log` is empty (0
  bytes), `Assertion`, `disconnect` and `illegal` each grep 0 over the console,
  and the console holds nothing but the guards' and build lines, the banner, the
  game lines, fastchess's periodic result blocks, its stop lines and the foot.
  2522 games started and 2511 finished, the 11 others in flight at the stop. The
  PGN's 2511 games carry fastchess's game numbers 1 to 2513, each at most once,
  2510 and 2511 absent: 1254 complete pairs, the 2508 games fastchess scored, and
  games 2509, 2512 and 2513 whose partners never finished. Terminations: 1668
  adjudication (1390 decisive, 278 drawn) and 843 normal, all drawn (537
  threefold, 203 insufficient material, 94 fifty-move, 9 stalemate).
- **Forfeits**: `tools/forfeit_report.py` reads 0 of 2511 for each engine.
- **Console warnings**: none of any kind; no PV-past-threefold block
  (`trim_console.py`'s count) and no `Incomplete mating PV` (next section).
- **Quarters**, in game-number order over the PGN's 2511 games, 627 or 628 each,
  by `adocs/data/S121_sprt_mate_pvs.py quarters`: **+18.3, +37.8, +14.4, +21.6**,
  each +/- 20.1 to 20.4. Their spread about the run's mean is chi-square 2.99 on
  3 degrees of freedom, p 0.39.
- **Kernel journal**, 07:52 to 09:06 (boot `98b74e5d`, from 2026-10-31 00:30:36,
  no suspend): no kernel entry; the boot's last is 00:36:35's `usb 3-2.4: USB
  disconnect`, the monitor's control device, as on S121's and S123's nights. Six
  entries in the window in all: 08:17:01's `cron.hourly` and 08:23:44's
  `fwupd-refresh`; none at warning level or above. No machine check, OOM or
  thermal event.

The run stands.

### Mating PVs

`grep -c "Incomplete mating PV" .tuning/s125_sprt.log` reads **0**, and `grep -ci
warning` over the same console reads 0: no warning of any kind, so no
mate-distance check is owed and DEC-319 (3)'s oracle and DEC-320's deeper reading
do not arise. `adocs/data/S296_sprt_mate_pvs.py` reads the same marker and would
find nothing. A count, recorded: the candidate's rate this run is 0 per 10000
side-games over 2511.

### The verdict, by the pre-registration

**The H1 row executes.** At 8+0.08 the fitted family is accepted as gaining: H1's
5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It ships**: P2, isolated pawns
by file b to h; P5, phalanx pawns by own rank 2 to 7; P6, supported pawns by own
rank 3 to 7; P7, one weak unopposed weight; and P0's flats refitted beside them,
the 02567 fit. **No bisection follows an H1**, and none is owed. P3 and P4 stay at
zero, in no expression, recorded as zeros (S126's rule). P1's increment is zero,
and the fold below is the one thing that takes `isolated_backward` out of zero, by
DEC-331 (3)'s arithmetic and at no change of score.

**The stopping run's Elo is upward-biased (DEC-063) and is not the effect size.**
The run stopped at a bound in 2508 games, far under the 25591 the pre-registration
expected with the truth on a bound; nElo +31.16 is the early stop's figure.

**The cost, beside it**: `cand`, the set the run played, **+3.50 %** on its own A/A
floor (`adocs/data/S125_cost_reading.txt`, RUN 2), on cost-only builds of the
landing at forced weights (DEC-047). It decides nothing; the SPRT priced it.
`bench_eval` per call, evaluate() alone: REF 63.99 ns, CAND 74.06, the fold 75.93 (RUN 3,
the cost section above).

### The trim and the archive

`tools/trim_console.py --summary` from `.tuning/s125_sprt.log` (6086 lines, 411880
bytes) into **`adocs/data/S125_sprt.log`**, 6091 lines, 412085 bytes: its first
6086 lines are the console byte for byte, since no PV-past-threefold block exists,
and the foot's five lines say so. Its `adocs/data/README.md` row added. The
untrimmed console and the run directory (`games.pgn`, `fastchess.log`) are
**`chesso_S125_sprt_2026-10-31.tar.xz`** in the owner's archive (DEC-235), built,
verified and moved in by the coordinator, beside
`chesso_S125_screen_cost_2026-10-31.tar.xz`; both sidecars re-verified from inside
the folder here.

### The fold, and its INV-6

The H1 row's fold, DEC-331 (3), (13), (15), against CAND `cf8661b`:

- `src/evaluation.cpp` `evaluate_pawns`: `white_backward` and `black_backward`
  carry `~white_isolated` and `~black_isolated`, so `PS_BACKWARD` counts no isolated
  pawn. `backward_rank`'s cells read that set directly, the local `& ~isolated` gone
  from the block; `backward_only_set` is that set. The name stays: a rename is ten
  lines of churn for no meaning, and the header says what it is now.
- `isolated_backward_mg` {0} -> **{-6}**, `isolated_backward_eg` {0} stays: backward's
  flat, {-6} / {0}, plus the increment P1's drop left at zero. An isolated pawn whose
  stop an enemy pawn attacks paid isolated + backward + 0 and now pays isolated +
  `isolated_backward`, the same number.
- **Liveness**: `isolated_backward_live` reads true on its own, by `any_weight()`.
  **The gating keeps a purpose, and stays**: `doubled_file_live` and
  `backward_rank_live` are false and those two loops compiled out, and a refit could
  zero another group. The comment over the flags says so.
- **The stage-one bound**: `structure_family_half()` already summed the array, so no
  code moves there. `S125_screen.py headroom` reads mg 24227 -> **24317**, eg 27929
  unmoved, 8450 and 4838 of room: the reach of -6 over 15 pawns is 90.
- `src/evaluation.hpp`: backward's definition, the "not exclusive" paragraph, and the
  family's `isolated_backward` and `backward_only` entries. `structure_family_counts()`
  moves with the collecting call, no code of its own. `src/eval_trace.hpp`: read;
  nothing in it says the family is at zero, pending or a double count. Unchanged.
- `tests/test_walked_counts.hpp` `pawn_structure_features` and
  `structure_family_features`, and `tests/test_evaluation.cpp`'s
  `pawn_counts_by_definition`: the walks read the new definition, square by square.
- `tools/tuner_layout.hpp`'s table line and `tools/tuner_groups.hpp`'s group comment
  no longer call it an increment on backward. `tools/tuner.cpp`, `tools/tuner_trace.hpp`
  and `tools/feature_audit.cpp`: re-read, nothing false; the coefficient comes from
  `EVAL_TRACE` at the engine's own difference. `.tuning/verify_fit.py` unchanged.

**The hand cases, DEC-331 (13)**, `tests/test_eval_model.cpp` "the walk and the engine
count pawn structure by hand": the four that asserted the double count, and the isolated
half of the two phase 1 added, now read backward 0 on an isolated pawn, and the comment
"not exclusive and are not meant to be" is gone with the definition. **Phase 1's
non-isolated backward cases hold unchanged where they are about that pawn**: b3 on the
White row and b6 on the Black row still count {0, 0, 1}; only a5's and a4's rows fall,
{1, 0, 1} -> {1, 0, 0}, being the isolated pawn beside each. Added: `4k3/8/8/2P5/pP6/8/1P6/4K3`
and its mirror, a pawn doubled and backward at once, one a colour, because doubled still
overlaps and "not exclusive" is now a statement about that pair. In "the walk and the
engine count the structure family by hand", three descriptive strings only: no cell
moved. Reds seen before the rewrite: only that case, in `test_eval_model`; `test_evaluation`
and `test_eval_trace` read green on the fold unedited but for the walk.

**The guard**, `tests/test_eval_model.cpp` "an isolated pawn with its stop attacked is
not backward": four placements, each colour twice, each first requiring the pawn
isolated and counted `isolated_backward`, then backward 0, `backward_only_set` empty for
that colour, and the other colour's flat as stated; then over S123's population, 10906
positions, the two sets disjoint and the flat's count the set's, per colour, with
`isolated_backward` met on 1003 and 935 and backward on 732 and 754.

**Observed red on the planted defect**, the mask removed and restored, `cmp` identical:

- Both colours: 5 of `test_eval_model`'s 19 cases red, the guard among them (flat
  1 == 0 and the set 562949953421312 == 0 at `4k3/8/8/3p4/2p5/8/1P6/4K3`), and in
  `test_evaluation` "the shared pawn sets count what the definitions say" alone.
- Black alone: the guard at `4k3/1p6/8/2P5/3P4/8/8/4K3`, the same five, and in
  `test_evaluation` five: the structure family's sets (134217728 == 0) and counts mirror,
  "colour symmetry over every test position" (299 == 304), the corrected score and the
  definitions. White alone: the same, 0 == 134217728.

**How the mirror test reads it**: "the structure family's sets mirror exactly" reads
`backward_only_set` per colour, so it goes red on a defect in one colour and **stays green
on the same defect in both**, because a symmetric wrong set mirrors. The hand guard and the
walks carry that case; the comment over the mirror suite says so. `test_eval_trace` is green
under all three plants: the trace and the accessors agree on what the defect counts. Its
header now names this pair, the two weights being equal, so the identity cannot tell a
coefficient on one from the other, and "every coefficient is the count its index names" with
the guard is what does.

**INV-6 against `cf8661b`** (`adocs/data/S125_close_inv6.txt`): `chesso bench` **5707449**
in both builds; `bench`, `bench 12` (1861982) and `tools/search_bench.py` at depth 9
(c3d5 37187, d5e6 105585, d7c8q 56111) and depth 12 (c3d5 147063, d5e6 343485, d7c8q
192032) byte-identical once timing is stripped, across the played binary, the worktree's
`build/` and `build-tune/` before the fold, and both after it. The fold's binaries differ
from the played one, as a code change must.

**The tuner**: `tuner --epochs 0` on S083's 6226944 rows: **0 differ from `evaluate()`**,
the clamp binds on 22083 as at the audit, 226750073 entries (36.41 a row), start validation
0.071323; `verify_fit` 1073 of 1073. **The audit**: rank exactly **1026 of 1073**, nullity 47,
**no new exact dependency**, `audit-check` passes; `diff` against phase 1's report reads one
line, the entries. It could not read otherwise: the new column is the old one less
`isolated_backward`'s, a change of basis. **The screen's reading stands as it was read on the
old definition**; no part is re-read, and DEC-331 (4)'s expectation holds.

### The goldens and preconditions, each by its script

`adocs/data/S125_close_inv6.txt` holds each command's reading.

| golden | at the candidate | at the fold | script |
|---|---|---|---|
| gauge walk | 0.0124 / 0.0538 | **moved: 0.0882 / 0.1165**, both the bishop's, over the floor by 8.82; the plain run descends two columns differently; the floor does not move | `S083_tuner_preconditions.cpp both` |
| clamp push | +30, 13.25 from the kink, 1.01e-7 | unmoved | the same |
| fixture clear of the clamp | 121.1 against 183 | unmoved | the same |
| gradient check (DEC-316) | 9.22e-7 at 800 | **unmoved**: the same function, and only two columns moved, neither the worst; no DEC-316 rows owed, no bound relaxed | `test_tuner_gradient -s`, both builds |
| coverage blocks | 19, 4, 2 pairs | unmoved, all three "reproduces: yes", 0 ineligible, the same smallest gradients: each tool reads its own terms and the scores. S121's ruling (b) did not arise | `S121_`, `S123_`, `S125_coverage_rows.cpp` |
| planted recoveries | green, `supported` at 16000 | green, the same budgets and bound; `isolated_backward`'s plant now starts from -6 / 0 | `test_tuner_gradient` |
| anchors | 107, 283, 353, 590, 1021, 0, 199, -603, -541 | unmoved, 10 of 10: no anchor holds a pawn whose stop an enemy pawn attacks; its prose moves | `S192_anchors.py` |
| truncation pins | phases 5, 11, 17, 23, N = 3 | unmoved: 111 past 2.8, 35 at 2.875, the same four | `truncation_scan`, `S083_truncation_pick.py` |
| both mate floors, ceilings, S170 budgets, node budget, first-mate, `DRIVE_DEPTH`, ply 2, `capture_mates`, the extra ply, `mate_probcut_hides`, `LATE_CAPTURE_*`, the aborted-line cell, `ALPHA` | as phase 2 | unmoved, **by INV-6**: the same scores make the same search, and each is green in both fast suites; not re-run | -- |

No test bound was relaxed.

### The comment pass

Every comment that said the family is at zero, in no expression, compiled out or pending
its SPRT now says what ships: `src/evaluation.hpp` (the family's heading, `isolated_backward`'s
entries, `structure_family_t`'s), `src/evaluation.cpp` (the paragraph over the arrays, the
liveness flags, the stage-one bound, the flats' overlap, `evaluate_pawns`'s backward and
isolated_backward blocks). Doubled by file and backward by rank **stay at zero and in no
expression, and the comment over the arrays says that**. In tests, comments only beyond the
rewrite: `tests/test_evaluation.cpp`, `tests/test_eval_trace.cpp`,
`tests/test_eval_positions.hpp`, `tests/test_tuner_gradient.cpp` (the walk's new reading, the
gradient's and the push's as read again), `tests/test_eval_model.cpp`'s heading.
`adocs/data/S192_anchors.py`'s two prose lines. `adocs/data/README.md` and `DEV_MANUAL.md`:
no site called the family at zero or pending other than as the landing's history.

### S118, after DEC-331 (9)

The pawn-key cache can hold S125's whole family, which reads pawns alone; `isolated_backward`
live and backward's new mask read pawns too. S118's accepts, whose sentence reads "the table
stores the mg/eg pawn score, the passed-pawn bitboard and **both kings' shelter and storm
scores**", still needs the correction S123 quoted for A and B, and it can now say what the
pawn score is. S118's file is not this step's; the sentence is quoted to the coordinator. Its
baseline, per `adocs/data/S125_sprt.sh`, is rows `cand` and p1 together, with `bench_eval`'s
75.93 ns on the tree it starts from.

### What H1 leaves unmeasured

- **Each group's own share.** P2, P5, P6 and P7 shipped on one verdict and were never played
  apart; each has only its screen gain (d +0.000012, +0.000036, +0.000031 and +0.000017
  against m 0.000004, 0.000003, 0.000005 and 0.000005).
- **The cost apart from the verdict.** +3.50 % is priced inside it; no run measured the
  family's strength net of its speed or a group's cost in play. The fold adds P1's row, +0.37 %
  instructions, read zero with its cycles and wall CIs reaching inside the A/A floor, and this
  verdict did not play it.
- **`isolated_backward`'s own weight.** -6 / 0 is backward's flat, inherited: P1 dropped the
  increment, and no fit has read these pawns apart with the flat holding only the others. S126's.
- **The far cells, for S126** (`adocs/data/S125_cells.txt`): **`supported_mg` own seventh +187
  above all** (+155.0 over its neighbour, 9476 rows, 2525 games), `supported_eg` own seventh -29
  (-55.0), own sixth 26 (+38.5), `supported_mg` own sixth 32 (-65.0), `phalanx_mg` own sixth 86
  (+33.0) and own fifth 20 (-26.5), `phalanx_eg` own sixth 68 (+30.5) and own fifth 7 (-29.0);
  the phalanx seventh tied to its sixth. A reading of the fit, no constant changed for it (DEC-084).
- **The long control.** Section 1 says every traced figure in this group shrinks at 60+0.6, so an
  8+0.08 verdict over-reports it. Nothing here measured that: S267's DEC-202 reading at 32+0.32
  and its drift series are where it is read, an estimate and never a verdict.
- **The thin margins.** The gradient, 9.22e-7 against 1e-6, a factor of 1.08, unmoved, and DEC-316's
  class on any later fit. The walk at 8.82 now and not 1.24, but only because the fold reshaped two
  columns; a refit can take it back. `MATE_IN_THREE_FLOOR` 11 is one below the 12 that ships and one
  above the weakened 10.
- **Doubled pawns by file and phalanx pawns' planted recovery**, unmeasured on the fixture, as phase
  1 recorded; `feature_audit --plant` is where it would go. P3 and P4 themselves: zeros, by the
  screen and not by any match.

### Documents

- `DEV_MANUAL.md`: checked. Three sentences added, each true of the fold: the audit reads the same,
  the gradient the same, the walk moves. Its S125 goldens rows state the candidate's readings, which
  are what ships. Nothing else false.
- `MANUAL.md`: checked; no sentence names isolated or backward pawns, and nothing on the UCI surface
  moved. Its `RfpMinPly` row's S125 figures are INV-6's. Unchanged.
- `adocs/data/README.md`: a close section, `S125_sprt.log` and `S125_close_inv6.txt`.

### The accepts, clause by clause

- **One SPRT verdict for the family, recorded whatever it is**: H1, above, with DEC-220's block for
  the closing commit. Holds.
- **After each group passes DEC-259's offline screen**: P2, P5, P6 and P7 each cleared its margin on
  S082's fixed set before the match; P1, P3 and P4 did not and did not enter it
  (`adocs/data/S125_screen_reading.txt`). Holds.
- **A family H0 bisected by group**: the verdict is H1, so no bisection is owed; the partition A = P0,P2 |
  B = P0,P5,P6 | C = P0,P7 was printed and pre-registered, the guards shown on a scratch pin. Holds.
- **Indexed by file or by rank where the surveyed record says the indexing is what pays, and the step
  states which and why**: isolated by file, kept; doubled by file and backward by rank, offered and
  dropped; phalanx and supported by own rank; weak unopposed flat. Why: section 1's record gives one
  instance of each indexing, and section 4 derives each width from the board with DEC-331 (4)'s baseline
  cell. Holds.
- **An isolated pawn is never also counted backward**: the fold, with no condition on the verdict
  (DEC-331 (15)); `src/evaluation.cpp` `evaluate_pawns`, the guard red with the mask removed, the corpus's
  trace exact. Holds.
- **Every constant is fitted (DEC-084)**: the 02567 fit pasted unchanged, no seed, every family weight from
  zero and the three flats from S083's {-9, -2, -11} / {-15, -18, -2}, this project's own fit; P3's and P4's zeros
  recorded as the screen's; `isolated_backward` is backward's fitted flat plus P1's zero, the form DEC-331
  (3) names, and S126 fits it. Holds, with that named.
- **The pawn hash is not a precondition and the plan order is not reversed; measured recomputed at every
  call, the per-call cost recorded as S118's baseline**: S118 has not started, every term is recomputed, and
  the cost is DEC-047's forced-weight run (`adocs/data/S125_cost_reading.txt`: `cand` +3.50 %, all +4.20 %,
  p2 +1.18 %, p5 +1.36 %, p6 +1.05 %, p1 and p7 zero), with `bench_eval` beside it per call: REF 63.99 ns,
  CAND 74.06, the fold 75.93 (`adocs/data/S125_bench_eval.txt`; one layout, not perturbed, deciding nothing),
  S118's baseline 75.93 ns on the tree it starts from. `eval_spread` reads stage two only, so where a stage-one
  term lands is the forced-weight rows (corrected above, DEC-331 (14)). Holds.

### Gates

- Both builds: 0 warning lines over a rebuild of `src/evaluation.cpp` and everything that
  links it. `ctest -L fast`: **50 of 50** in `build/` and in `build-tune/`, sequential,
  `CLANG_FORMAT_MAJOR=22`. Bench **5707449** in both: the commit's `Bench:`.
- `./clang-format.sh --check` clean.
- **The S297 clang pair**, inside `gate_extra`'s stage 6: `build-clang` and
  `build-clang-tune` with clang 22.1.8, no error and no warning, both fast suites 50 of 50.
- `tools/plan_prose_check.py`, one mode per call, each exit 0: `--citations` 0 flagged over
  19 files, `--touches` 0 over 19, `--params`, `--gate` agreeing, `--prose` 0 flagged. Read
  again after this section was written.
- **`tools/gate_extra.sh`**, all six stages, on this tree, detached under a four-exit watcher:
  `GATE-EXTRA-DONE 6 stages 1514 s`. prose 0 s, citations 0 s, debug 390 s (6 of 6), sanitize
  729 s (the fast label 50 of 50 under the sanitizers), perft 52 s, clang 343 s (50 of 50
  twice). 0 `Assertion` in the debug and sanitize logs. One comment sentence in
  `evaluate_pawns` was reworded after it, and both binaries read byte-identical after the
  rebuild, so it stands.
- **Tier 2 (DEC-141) does not bind**, by what the diff touches: under `src/` only
  `evaluate_pawns`'s backward sets and the rank block, one weight, and comments; no
  `make_move`, `unmake_move`, generator or search change, and no pruning, reduction or
  extension rule. `gate_extra` ran anyway, as the brief asks, because the fold is code in
  every stage-one evaluation. The Debug self-play at landing is the coordinator's.

Every clause holds. Done.
