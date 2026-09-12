id:         S125
goal:       backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
accepts:    an SPRT verdict per group, recorded whatever it is; the terms are indexed by file or by rank where the surveyed record says the indexing is what pays, and the step states which indexing it chose and why; **an isolated pawn is never also counted backward**, which is a documented double-count worth +4.01 to fix; every constant is fitted (DEC-084); **the pawn hash is not a precondition and the plan order is not reversed** -- these terms are what make S118 worth building, so they are measured recomputed at every `evaluate()` call, and the per-call cost the additions carry is measured here (`bench_eval`, and `build/tools/eval_spread` for where it lands) and recorded as the baseline S118 is later asked to reclaim
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp
excludes:   passed pawns, which are S123
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## What is there

Three terms -- isolated, doubled, backward -- each one flat weight, fitted to
`{-10, -9, -11}` middlegame and `{-12, -32, -8}` endgame. What is missing is
phalanx and connected pawns, supported pawns, and weak unopposed pawns; and
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
caveat attached.

So this step is as much about giving the three terms that exist more shape as
about adding four more.

## The pawn hash comes after, and that is deliberate

The `accepts:` used to require these terms to "live behind the pawn hash from
S118", which S118 cannot supply at this point in the order. S139 dropped the
dependency rather than reorder, because every document that touches the
question orders the terms first and says why:

- `adocs/plan.md` "pawns with king distance (S123" -- *"the
  connected and phalanx pawn work (S125, +25.4 class), the pawn hash that makes
  them affordable (S118)"*. Terms, then the cache.
- `adocs/plan.md` "block to land after the pawn terms it caches are worth
  caching" -- S118 moves out of the speed block to land after them. The
  parenthetical that used to follow that clause, and used to be quoted here,
  read the published slowdown as a cheap pawn evaluation cached; DEC-203
  replaced it with what the source says -- a table sized past the last-level
  cache -- and the ordering argument is unchanged by the correction.
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
- The list itself: `adocs/plan.md` "backward, phalanx, supported and weak
  unopposed pawns join the three", S125 at 54 and S118 at 55. (Both ranges
  moved when S140 rewrote the block-3 paragraph on 2026-08-21; this one was
  also one entry low before that, naming 55 and 56 while the sentence claims 54
  and 55.)

Reversing the pair would put a cache in front of the cheap computation it
caches, which buys little by this project's own arithmetic -- not by the
published 10 % slowdown, which was an oversized table and is a sizing
constraint on S118 rather than an ordering argument (DEC-203). So the
cost this step adds is paid per call and measured per call; S118 is the step
that reclaims it, and this step's numbers are what its verdict is read against.

**And it is not CLAUDE.md's accumulate-don't-recompute hazard either**, which
is about an accumulated quantity being rebuilt from the bitboards at every node
-- INV-4's four fields, and the 25 % of nps S014 took back. These four terms do
not exist in any form yet, so nothing stops being accumulated. What they cost
is what a new pawn term costs before a cache exists, and the SPRT per group
prices exactly that: term value net of term cost, which is the honest question
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
  rule and the wiki neither supports nor contradicts it. No figure.
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
has no located definition anywhere and the step must write its own and say so.

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

- The four new terms and the indexing both widen `PS_FEATURE_COUNT`, so
  `tools/eval_model.hpp` and `tools/tuner_groups.hpp` move in the same commit
  and the partition properties in `tests/test_tuner_groups.cpp` are what catch
  a missed base.
- All four terms read the same pawn fills `src/evaluation.cpp` `evaluate_pawns`
  already builds. Anything needing a new pass over the board is out of scope by
  the same rule S102 uses.
- The per-call cost is measured here -- `bench_eval` and
  `build/tools/eval_spread` for where it lands -- and recorded as the baseline
  S118 is asked to reclaim.

### 4. Constants and seeds

**No seeds.** Every constant is fitted (DEC-084 and the accepts). Section 1's
figures are Elo measurements of other engines' patches and are not starting
values; no engine's pawn-penalty array seeds anything here wherever it is
republished (DEC-105, DEC-134), and the wiki's connected-pawn sentence gives a
shape with no values.

The one number the step declares rather than fits is the **index width**, and
it is **(b) derived from the board**: 8 files or 6 usable ranks per term. Both
come from the rules of chess.

### 5. Pitfalls

- **The eight-times-in-one-commit trap.** Four new terms plus an indexing
  change over three existing ones is not one verdict. The accepts says "an
  SPRT verdict per group"; a group is a term or the indexing, not the file.
- **The double-count is a definition change, not a bonus.** Fixing it moves
  the *meaning* of the backward feature, so the fitted backward weight before
  and after are not comparable and the step must refit rather than carry.
- **Everything here is recomputed at every `evaluate()` call by design.** That
  is the plan order (DEC-087 (h) and (i)) and not the INV-4 hazard, which is
  about an accumulated quantity being rebuilt; these terms are not accumulated
  and never were. The cost is the number S118 later reclaims.
- **Long control reads lower.** All four traced figures shrink at 60.0+0.6s.
  A verdict taken only at 8.0+0.08s over-reports this group, and S151's
  block-boundary long-control reading is where that is checked.
- **"Weak unopposed" has no published definition.** Writing one is part of the
  step; borrowing another engine's predicate by reading its source is not.

### 6. Measurement

One SPRT per group at the S105 regime, bounds stated in advance with the nElo
worst case and the abort rule (DEC-143), sized on the traced figures: the
conditioning group at the +10 class, the indexing groups at the +4 to +7 class,
the connected/phalanx group at the +25 class. `bench_eval` before and after,
and the per-call cost recorded whatever the verdicts are.

### 7. Interactions

- **S123 (before)**: the other pawn group; the two share the fills.
- **S118 (after)**: caches what this step makes expensive. This step's measured
  per-call cost is the baseline S118's verdict is read against, which is the
  whole reason the order is not reversed.
- **S122 (after S118)**: reads the shelter and storm slots the cache adds.
- **S039 (before S122)**: the lazy margin is sized on a sum these terms do not
  enter -- they are cheap-stage terms -- but the step states which stage each
  new term lands in, as S101's accepts does.
- **S126 (block end)**: refits everything.

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
