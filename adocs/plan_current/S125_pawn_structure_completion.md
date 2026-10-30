id:         S125
goal:       phalanx, supported and weak unopposed pawns join the isolated, doubled and backward pawns that exist, backward stops counting an isolated pawn twice, and each is fitted
accepts:    one SPRT verdict for the family, recorded whatever it is, after each group passes DEC-259's offline screen, and a family H0 bisected by group; the terms are indexed by file or by rank where the surveyed record says the indexing is what pays, and the step states which indexing it chose and why; **an isolated pawn is never also counted backward**, which is a documented double-count worth +4.01 to fix; every constant is fitted (DEC-084); **the pawn hash is not a precondition and the plan order is not reversed** -- these terms are what make S118 worth building, so they are measured recomputed at every `evaluate()` call, and the per-call cost the additions carry is measured here (`bench_eval`, and `build/tools/eval_spread` for where it lands) and recorded as the baseline S118 is later asked to reclaim
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_trace.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp, tools/tuner_trace.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tests/test_evaluation.cpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_eval_positions.hpp, tests/test_walked_counts.hpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, .tuning/verify_fit.py, adocs/data/S192_anchors.py, adocs/data/S123_coverage_rows.cpp (scoped), adocs/data/S125_*, DEV_MANUAL.md, adocs/data/README.md; MANUAL.md read and unchanged at phase 1
excludes:   passed pawns, which are S123
decisions:  DEC-071, DEC-084, DEC-259, DEC-315, DEC-326, DEC-329, DEC-330, DEC-331, DEC-221, DEC-134, DEC-105, DEC-090, DEC-047, DEC-142, DEC-316
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:

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
