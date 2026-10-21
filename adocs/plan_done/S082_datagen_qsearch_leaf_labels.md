id:         S082
goal:       the corpus labels a resolved position rather than the root -- the quiescence leaf, or the leaf reached by playing out a deep search's whole principal variation -- and samples few positions per game rather than many
accepts:    `tools/datagen` records the position at the leaf quiescence resolves to, with the game result unchanged as the label, and states in its own output how often the leaf differs from the root; the tactical-move filter clause is retired rather than flagged, since its whole justification was that the root might not be quiet; a corpus is regenerated and refitted, and the recipe -- which leaf, how many positions per game -- is **chosen offline by held-out loss on a fixed validation set, with no SPRT of its own**: S083's one SPRT measures the corpus that ships (DEC-259); the fixed validation set is built here once and kept for every later fit and screen -- games split before sampling, no game and no identical position on both sides of the split, its size, seed and digest recorded -- and because it chooses recipes and screens terms it is a validation set, not an untouched test set; the walk states its policy for an aborted search, a bound-only line, a quiet promotion and a terminal leaf, and the stored label keeps a defined perspective when the leaf's side to move differs from the root's; the leaf is verified to be quiet by construction -- a test asserts that the recorded position has no capture the quiescence search would still make, and fails if the walk is truncated by the depth bound instead
touches:    tools/datagen.cpp, src/search.cpp or src/search.hpp for the leaf walk, tests/, .tuning/
excludes:   the corpus size and the node budget, which are S083's; dedupe, which is S076's; the label blend, which is S075's; any change to what quiescence itself does
decisions:  DEC-055, DEC-041, DEC-258, DEC-259
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-21 11:38 -- a corpus row names a resolved position: src/leaf_walk.cpp walks quiescence's own choices until it stands pat and datagen writes that leaf (--leaf qs, the default), the played-out PV's end resolved the same way (pv) or the root, the game result the label in White's perspective; datagen prints per leaf how often the leaf differs from the root (qs 26.53 %, pv 100.00 % on the top-up; 26.61 % and 100.00 % recomputed on part 1) and every refusal by kind. The tactical clause and --allow-tactical retired. chesso links none of it: bench 6042933 with the parent's bestmoves, search_bench 9 and 12 identical (5b0e170, INV-6); datagen exits 1 on a failed write or a short pool (test_datagen_exit). Mutants LW01-LW09 killed. The pool, 150000 games at 100000 nodes with --leaf root,qs,pv: cut by a power loss at 122748 games, completed by a 27252-game top-up from the same binary (DEC-296), joined by S082_pool_merge.py (f93a3b2, fast check clean, a sync added), merge and verify DONE. The fixed validation set: tools/corpus_split.py, seed 20261013, 12000 games of 149998, qs view 1015332 rows sha256 2b508067fb64693bb4b278a2455fb9d101eaa742975a4b53f4ff56e6fa16981e, root and pv views beside, at .tuning/s082/ and archived (chesso_S082_validation_and_logs_2026-10-21.tar.xz); tuner --holdout reads it and refuses a shared position. Recipe by S082_recipe.sh's pre-registered rule, no SPRT of its own (DEC-259): outcome (a), leaf qs, 46 rows a game, L 0.072154 against the control's 0.073204, m 0.000330 (adocs/data/S082_compare.log); S083 generates at it. Fast suite 49/49 in both builds at f93a3b2 (CLANG_FORMAT_MAJOR=22), format clean, the five prose modes clean, gate green. DEV_MANUAL.md updated for the reading and the two-part pool; MANUAL.md checked, unchanged; specs.md holds no recipe, unchanged. Second tier not owed for this commit (no make_move, generator or search change; 5b0e170 carried the step's code). Implemented from the step file's description (DEC-221) by Opus subagents briefed by the coordinator; merge, compare and the reading by the coordinator.

## Amended 2026-10-08, DEC-258 and DEC-259

The corrected plan comparison found two corpus verdicts where the recipe can
be chosen offline: this step changes what is labelled, S083 how much is
generated, and each regenerated, refitted and took an SPRT. Under DEC-259
the recipe is a held-out-loss decision on a validation set fixed here, and
one SPRT (S083's) measures the corpus that ships. The direct assertion that
the recorded leaf has no capture quiescence would still make, and the
refusal of a depth-truncated walk, are unchanged -- a PV leaf out of check
and not terminal can still be unresolved. The corpus is played by the search
S261's lane, S099 and the capture-history family leave behind, which is why
this step follows them (DEC-258). Labels come from chesso's own games and
search only (DEC-016); tool oracles serve diagnostics, never labels.


## Why this exists

`adocs/eval_tuning_strategy.md` calls this "the critical design decision" and
answers it in section 2.3: label the **quiescence search score at a quiet leaf**,
because "the eval function is only well-defined on quiet positions, so the target
must be anchored at a quiescent leaf". Section 2.6 says the same as a filter:
"Drop positions where the qsearch leaf differs from the root (or rather: label
the leaf, not the root)."

chesso labels the root and works around the consequence. `tools/datagen.cpp`
records `generate_FEN(&game.board)` at the position played, and its fourth filter
clause drops any position whose best move is a capture or a promotion -- an
approximation of quietness that DEC-055 already found wanting for the opposite
reason:

> That fourth clause was justified here by the claim that evaluate() is only ever
> asked about a position quiescence has already resolved. It is not:
> `src/search.cpp` `quiescence` calls evaluate_lazy() at the top of every quiescence node,
> before a single capture is generated.

S065 loosened the clause behind `--allow-tactical` and regenerated with it on, so
today's corpus contains tactical roots labelled as if they were quiet. Anchoring
at the leaf answers the question the flag was a compromise on: the leaf is quiet
by construction, so nothing has to be dropped and nothing has to be admitted.

## What it changes about the fit

The tuner's float model evaluates the row it is given. Move the row to the leaf
and the model is fitted on the positions `evaluate()` is actually asked about,
which is the whole argument. It also removes a bias nobody has measured: a filter
that drops every position whose best move is a capture removes sharp positions
from the fit and keeps their outcomes, while `--allow-tactical` keeps them and
mislabels them.

## The trap

`MAX_QSEARCH_DEPTH`, 19 as shipped since S085 retuned it from 8, is the ply
bound `quiescence` of `src/search.cpp` tests against `qply`. A leaf reached by
exhausting that bound is not quiet, it is truncated, and recording it puts back
exactly the noise this step removes. The gate asks for a test that separates the two, and datagen
should count the truncated ones rather than silently keep them.

Second trap: the leaf of a search is not the leaf of a plain quiescence call from
the root. Which one is recorded has to be stated -- the principal variation's
quiescent end, or a quiescence run from the played position -- because the two
differ and only one of them is reproducible from the FEN alone.

## Cost

One night of generation, on the S065 precedent: 120000 games at 100000 nodes took
about eight hours for 11.0 M rows. One fit, minutes. One SPRT, three to four and
a half hours.


## Widened 2026-08-19

Two additions, both cheap once datagen is being changed anyway: the first from
the published method, the second this project's own.

**Resolve by playing out the principal variation.** Search the sampled position
deeply, play the **whole** PV, and store the leaf. This makes the position
quiet by construction rather than by filter, which is what the quiescence-leaf
label is reaching for by a shorter route. It costs some label precision -- the
game result is now attached to a position several plies from where it was
sampled -- and the published assessment is that the diversity and the
resolution are worth more than the precision.

**Sample few positions per game.** This is **chesso's own choice and not a
published practice**, and the reason is correlation: positions inside one game
share an opening, a material balance and a result, so a hundred rows from one
game are not a hundred independent observations. The corpus this engine fits
on is about 92 rows per game by construction -- S066 fixed the splitter that
this broke, and the underlying density was never revisited. The one sourced
density located runs the other way, Texel's about 140 positions a game
(section 1), which is why the choice is written here as a **hypothesis** and
handed to S083 to measure at its start: rows per game and games needed, read
against held-out error. Density and size are the same night spent differently,
which is why this step states the density and S083 decides the corpus.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The quiet-leaf label is the wiki's own formulation, and it is cited.** CPW
*Texel's Tuning Method*
(https://www.chessprogramming.org/Texel%27s_Tuning_Method, fetched 2026-09-13)
defines the objective over **the engine's quiescence search score** compared
against the game result, labelled 0 for a black win, 0.5 for a draw and 1 for
a white win, with a scaling constant `K` fitted once and then held. Labelling
the score quiescence returns and labelling the position quiescence resolves to
are the same anchoring argument reached from two ends, and this step takes the
second because it also removes the filter.

**The filter this step retires was tried in the published method and
withdrawn.** The same page records the original rule -- drop positions whose
quiescence score deviates from the search score -- and its author's reversal:
"I now believe that even though including these positions causes noise, the
q-search function has to deal with them all the time in real games." That is
the published support for retiring the tactical-move clause rather than
tightening it, and it is stronger than the argument in "Why this exists"
above, which rests on DEC-055 alone.

**The one published positions-per-game density runs the other way, and this
file's own density is now a hypothesis.** CPW *Texel's Tuning Method* states,
for the original method: "Take 64000 games played at a fast time control",
"This typically gives about 8.8 million positions", "about 140 positions per
game on average". This file used to assert a far lower density as published
practice; two searches found no source for it, and DEC-203 deleted the figure
and restated sampling few rows a game as **chesso's own choice, made for
correlation inside a game and measured by S083** (section 5). Searched this
pass: CPW *Texel's Tuning Method* and *Automated Tuning*; a web search for
datagen sampling density ("positions per game", dataset quality, self-play);
TalkChess "Generating original training data"
(https://talkchess.com/viewtopic.php?t=84811), which discusses generation but
states no per-game density. Searched on 2026-09-04 by the plan review: the
same class, no source.

**The node budget has one published statement.** TalkChess "Generating
original training data" (https://talkchess.com/viewtopic.php?t=84811, Sapling,
2025-02-07): "Set a hard node limit for your search e.g 5k nodes". That is a
forum recommendation, not a measurement, and it agrees with
`tools/datagen`'s own default rather than with the 100000 the S065 run used --
which is S083's trade and not this step's.

**What the record does not give**: no located source prices the leaf-versus-root
label change in Elo, at any band. The plan does not rest on one -- this step's
argument is DEC-055's observation about where `evaluate()` is actually called,
and its verdict is its own SPRT.

### 2. Shape for chesso

Two leaves are on the table and the file already says only one is recorded.
The enrichment adds which one the published record anchors:

- **Quiescence from the played position.** Reproducible from the FEN alone,
  which is the property the corpus row needs, and it is exactly the function
  CPW's formulation scores. This is the leaf the wiki's method implies.
- **The whole principal variation played out.** Quieter by construction and
  more diverse, at the cost of moving the label several plies from where the
  result was sampled. No located source prices the difference.

The step states which it records. If both are wanted that is two corpora and
two verdicts, on MEASUREMENT's one-change-at-a-time rule.

Sites: `tools/datagen.cpp` is where the row is written;
`src/search.cpp` `quiescence` is the function whose leaf is being asked for,
and `src/search.cpp` `MAX_QSEARCH_DEPTH` is the bound that makes a leaf
truncated rather than quiet.

### 3. Implementation sketch

- Record the leaf, the ply distance from the root to it, and a truncation
  flag. Datagen prints the share of rows where the leaf differs from the root
  and the share truncated by the depth bound, both in its own output, which is
  what the accepts asks for.
- Retire the fourth filter clause and `--allow-tactical` with it; the clause's
  justification is the one CPW's author withdrew.
- The quietness test asserts the recorded position has no capture quiescence
  would still make, and **fails when the walk stopped on the depth bound**:
  the two failure modes are different and a test that conflates them passes on
  a truncated corpus.

### 4. Constants and seeds

One constant is in scope and it is not fitted here: the **rows per game**.
Under DEC-105 it is stated as **(c) a declared range and its midpoint**, not as
a published practice, because the only sourced density is CPW's about 140 a
game (section 1) and chesso's reason for wanting fewer -- correlation inside a
game -- is its own. Declare the range by
purpose -- 1 is the minimum that keeps a game in the corpus at all, and the
current 92 per game is the incumbent -- and let S083's held-out curve decide,
since density and size are the same night spent differently. `MAX_QSEARCH_DEPTH`
is not a seed: it is read at this step's own HEAD.

### 5. Pitfalls

- **The truncated leaf.** `src/search.cpp` `MAX_QSEARCH_DEPTH` is 19 since
  S085. A leaf reached by exhausting it is not quiet, and recording it puts
  back the noise the step removes.
- **The density is a hypothesis and must not silently become a requirement.**
  Sampling few rows a game is chesso's own choice, made for correlation inside
  a game and supported by no located source; DEC-203 deleted the figure that
  used to state it as published practice, and S083's held-out curve is what
  decides it. Until that curve exists, neither file converts a target row
  count into a game count: the arithmetic depends on the density and the
  density is the open question.
- **The label is the game result, not the score.** `tools/tuner.cpp`'s
  `--lambda` blends them; the corpus row carries the result and the blend is
  S075's question.
- **A leaf label changes what the filter counts.** Filter counts recorded
  against the old clause are not comparable with the new ones; the step file
  records both.

### 6. Measurement

Held-out error on the by-game splitter first -- a leaf corpus that fits worse
is a finding and not a failure -- then **one** candidate to an SPRT against the
shipping weights, verdict recorded whatever it is. The leaf-differs share and
the truncation share are reported whether or not the SPRT resolves; they are
what the next corpus decision is read against.

### 7. Interactions

- **S083 (immediately after)**: size and node budget, decided on this step's
  sampling density. The two are one design and are split only so each gets one
  verdict.
- **S076 (dedupe), S075 (blend)**: both act on the rows this step defines.
- **S126 (block end)**: the full refit runs on this corpus; the provenance
  stamp S077 added carries the datagen parameters.
- **S135, S136 (before, in plan order)**: both fit on the corpus this step
  produces, which is why the corpus steps precede them in block 3.

### 8. References

- - https://www.chessprogramming.org/Texel%27s_Tuning_Method -- quiescence
  score against the game result, labels 0 / 0.5 / 1, `K` fitted once; the
  withdrawn deviation filter and the author's reversal quoted above; "64000
  games", "about 8.8 million positions", "about 140 positions per game on
  average". Fetched 2026-09-13.
- - https://talkchess.com/viewtopic.php?t=84811 -- "Generating original
  training data"; Sapling, 2025-02-07, "Set a hard node limit for your search
  e.g 5k nodes". No statement of positions per game. Fetched 2026-09-13.
- - https://www.chessprogramming.org/Automated_Tuning -- supervised learning of
  evaluation weights; no dataset-size or sampling-density guidance. Fetched
  2026-09-13.
- - `adocs/eval_tuning_strategy.md` sections 2.3 and 2.6 -- the project's own
  strategy document, quoted in "Why this exists". Local, not literature.

## From the 2026-09-27 tuning comparison (the coordinator, 2026-09-28, DEC-237)

`adocs/data/2026-09-27_article_finetuning_comparison.md` section 6 keeps one
idea for this step: **a coverage report by phase and score band**, printed by
`tools/datagen` (or the tuner) as a standard part of the corpus rewrite this
step does -- the matrix S065 measured one cell of by hand (a queen at `phase
<= 4` on 0.0889 % of rows). A print, not a resampling: score-stratified
resampling changes the label distribution the objective is defined on and is
refused there. Tools-only, no verdict owed; the brief carries it.


## As built (2026-10-13, phase 1 of 2)

Written blind under DEC-260 in the worktree `/home/max/ws/chesso-s082` on
`e1c59f8`, while S261's SPSA lane held every core: nothing compiled, tested or
run (one `bash -n` over the script, a parse). Phase 2 builds, gates and runs it
after S261, S099, S023 and S039 land, in that order, and fills every figure
this section leaves pending.

### Deviations

- **The walk is a file of its own, `src/leaf_walk.{hpp,cpp}`, not
  `src/search.*` or `tools/datagen.cpp`.** Two reasons: `tools/mutation_check.py`
  refuses a mutant outside `src/`, and the accepts want the quiet test's mutant
  observed; and S023 and S039 both edit `quiescence()` and S099 `negamax_at()`,
  so a new file merges with none of them. `chesso_engine` is a static library
  and nothing the engine plays references the file, so the `chesso` binary does
  not link its object -- phase 2 shows it with `nm`. `src/search.hpp` changes a
  comment only: `quiescence()` was "called by nothing outside search.cpp", and
  the walk calls it now.
- **The tuner changes, outside `touches:`**, for two reasons the step implies
  and does not name. (1) The fixed validation set is "kept for every later fit
  and screen" and S083 reads its held-out loss there while excluding any tuner
  change, so the tuner has to read a fixed set: `tools/tuner.cpp` gains
  `--holdout FILE`. (2) A leaf breaks S066's game reconstruction (surprise
  below), so `tools/tuner_split.hpp` `row_ply` reads datagen's new ply column
  when a row has it, and `tools/tuner_trace.hpp` names that in its refusal.
- **datagen's row grows three columns**, `fen result score phase game ply
  distance`: `game` for the builder, `ply` for the split, `distance` for the
  reading. Every reader of the four-column format reads the first four
  (`tuner_trace` `parse_line`, `corpus_dedupe` `row_fen`, `feature_audit`,
  `eval_spread`, `truncation_scan`), checked by reading each.
- **A position whose search completed no iteration is no longer written**; it
  was written with the score 0 that `run_search()` leaves when no iteration
  completes. Unreachable at any budget a run uses (depth 1 completes), counted
  as `no completed iteration` so a budget that reaches it says so.
- **`--allow-tactical` is refused as an unknown option**, not accepted and
  ignored: a script that asks for it asks for a corpus datagen no longer makes.
  S065's command in `DEV_MANUAL.md` is kept as history and says so.
- **The coverage report is in** (the section above): a five-by-five count of
  written rows by phase band and score band, per leaf. Not in the brief; the
  step file assigns it here.
- **No new golden**: the tests' numbers are preconditions read from the engine
  at run time (`> 0` counts, a mate score below -10000 the search tests
  already rely on) and a narrowed bound the case sets itself.
- **Not done here**, and why: the corpus is not regenerated and nothing is
  refitted -- phase 2's night run; S262's note that whoever next compares
  optimisers on a new corpus should sweep AdaGrad past 30 is not taken up --
  this comparison holds the fit fixed and compares corpora, one change at a
  time.

### Surprise: a leaf breaks S066's split

`tools/tuner_split.hpp` finds where one game ends by the FEN's ply, which only
advances inside a game -- true of roots, false of leaves. Sampled at ply 20 and
resolving two plies on, a row stands at 22; the next, sampled at 21 and quiet,
stands at 21: read from the FEN, a boundary inside one game, so the game lands
on both sides of the cut. Every leaf corpus fitted with the old reader would
have been contaminated a little, silently. datagen now writes the sampled
position's ply as a column and `row_ply` reads it when present, which restores
S066's construction exactly: one row per sampled position, one sampled
position per ply. `tests/test_tuner_split.cpp` "a row carrying datagen's ply
column is placed by it" asserts both halves, the FEN reading cutting the game
first.

### Design

**The leaf.** `src/leaf_walk.cpp` `leaf_walk_quiescence` resolves a position by
asking quiescence itself, one fresh call a ply: `quiescence()` at the widest
window on a table of the walker's own, cleared before the call
(`leaf_quiescence_choice`); the move chosen is read from the exact entry the
call's own root stored -- the last store of the call, so its subtree cannot
have overwritten it -- and checked against the generator before it is made;
the walk plays it and asks again until quiescence stands pat. That is the leaf:
a position where no move quiescence searches beats the static score, and
quiescence's value there is `evaluate()`'s own number, which the walk checks.
No copy of quiescence's move set, exchange gate or ordering lives in the walk,
and nothing about what quiescence does changes (`excludes:`). The table is
cleared per call because an entry left by the previous call is a bound
quiescence's root would read as its stand pat (S130), and a walk that stood pat
on it would record a position a capture still improves. The walker's search
state is its own too, so the leaf is a function of the FEN alone -- the step's
second trap, answered for `qs`.

**The PV leaf.** `src/leaf_walk.cpp` `leaf_walk_pv` plays the last completed
iteration's principal variation from the sampled position, each move checked
against the generator, and resolves its end the same way: "a PV leaf out of
check and not terminal can still be unresolved" (the amendment above). Not
reproducible from the FEN, because the line is the search's.

**datagen.** `--leaf root|qs|pv`, comma separated, `qs` by default; several
write one file each, `--out` with the kind appended, from the same games and
the same sampled positions -- the pool the recipe is chosen from, every file of
it a corpus the tuner reads as it is. The three remaining clauses are read at
the sampled position, as before; each leaf is then asked of what passed
(`tools/datagen.cpp` `sample_leaf`). The fourth clause and its flag are gone.
Its own output states, per leaf, the rows written, how many differ from the
root and by how many plies on average, and every refusal by kind with its
share; then the coverage matrix and what the walks cost in nodes against the
games' own searches.

**The fixed validation set and the corpora.** `tools/corpus_split.py split`
holds out whole games chosen under a seed before anything is sampled, every row
of them, one `.validation` and one `.train` per leaf file, the same games out of
every file; it reads the written files back for the no-game-on-both-sides
property and prints the seed, the held-out game list's sha256 and each output's
rows, games and sha256. `sample` takes games in a seeded order, drops every row
standing on a position any named validation view holds, keeps at most
`--per-game` a game in their order, and writes exactly `--rows` or refuses.
`tuner --holdout FILE` fits on every corpus row and reads every held-out figure
on `FILE`, loaded and checked against `evaluate()` as the corpus is, and
refuses a corpus sharing a position with it (`tools/tuner_split.hpp`
`shared_positions`, the FEN's first four fields, so clocks do not hide one).

### The policies the accepts ask for, and why

- **An aborted search.** The game search's last iteration usually aborts on
  the node budget; its move is played, as before, but its line is never walked:
  the PV leaf walks the last completed iteration's line, whose root score was
  searched at the full window (datagen does not aspirate). A search that
  completed none writes no row. A walk's own quiescence call that reaches
  `src/leaf_walk.hpp` `LEAF_WALK_NODE_CAP` refuses the row as aborted: the call
  answered with a static number in place of a search.
- **A bound-only line.** The walk advances only on its call's exact root entry.
  On a cleared table at the widest window that is the only entry the root can
  leave, so a bound is a defect and the row is refused as uncertified; the same
  refusal covers a PV or table move the generator does not produce here.
- **A quiet promotion.** A queen promotion that takes nothing is in
  quiescence's move set (S131), so a position whose best quiescence move queens
  is not a leaf and the walk plays the promotion. The three underpromotions
  that take nothing are not in that set and do not make a position unquiet.
  The walk follows quiescence's set and has none of its own.
- **A terminal leaf.** Mate, stalemate, insufficient material, the fifty-move
  rule, or a repetition the search would draw from the sampled position, reached
  by the line: refused and counted. `evaluate()` is never the last word there
  -- quiescence and the search return the rule's value before it is asked -- so
  a row there fits a number to a draw or a mate. **The repetition is the
  search's, not the game's** (fast check): `src/leaf_walk.cpp` `rules_decide`
  asks `classify_repetition()` with the sampled position as the root, so a
  position repeated inside the line, or a third occurrence, is terminal, and a
  single occurrence before the root is not -- the search goes on searching
  such a position and `evaluate()` is asked about it, so refusing it would drop
  a real leaf for the game's history, not its own. datagen's game-ending test,
  `is_position_repeated()`, a two-fold anywhere, is unchanged: it adjudicates
  games, not leaves. Terminal refusals are a policy outcome, printed and never
  gated on.
- **A truncated walk.** A walk that makes `MAX_QSEARCH_DEPTH` plies, the
  horizon of one quiescence call, without reaching a position quiescence
  stands pat at is refused as truncated and counted, never recorded. The bound
  is read when the walk starts, so the tune build's setting is honoured.
- **The label's perspective.** `result` and `score` are White's whoever moves
  at the leaf; the tuner turns its White-relative model into the side to
  move's by the row's own FEN. `score` stays the sampled position's search
  score, the number the band clause and S075's blend read.

### Tests

- `tests/test_leaf_walk.cpp`, new, `chesso_engine`, fast label, seven cases over
  the JSON suites' positions and three busy ones from the search tests:
  "every quiet leaf the walk records is one quiescence stands pat at" (the
  property read back from the recorded FEN alone, in a game and a table the
  walk never touched; every walk that moved left a root quiescence did not
  stand pat at; the game restored, history included; no aborted or uncertified
  walk; non-vacuity: walks that moved, and of two plies or more);
  "a walk stopped by its ply bound is refused as truncated" (the same walks
  bounded one ply short of their leaf, and where they stop quiescence still
  moves -- the two endings told apart); "the ply bound is quiescence's own
  horizon unless narrowed"; "a line that ends in mate is terminal, not quiet";
  "a principal variation is played out and then resolved" (the search's PVs
  over a slice, and -- the half that cannot pass without resolving past the
  line -- from every position whose quiescence walk takes two plies or more,
  that walk's own first move as a one-move line, which must land where the
  quiescence walk does; `REQUIRE` that such lines exist and resolve past
  their end); "a line the generator does not produce here is refused"; "a
  repetition refuses a leaf only where the search draws it" (king and rook
  against king, every move found by the engine's generator: a line back to a
  position the game held once before the root stays a leaf, with the
  precondition that the game-level rule refuses it; the root's own position
  recurring once stays a leaf; a position repeated inside the line is
  terminal).
- `tools/mutants/S082_leaf_walk.py`, eight mutants, each named with the case
  that should kill it: LW01 the bound records a truncated walk, LW02 the walk
  stops after one move, LW03 a terminal leaf recorded, LW04 a move left made,
  LW05 the root recorded for the leaf, LW06 the PV's end recorded unresolved,
  LW07 the game-level repetition rule refusing a leaf, LW08 a repetition
  inside the line walked past. Observed in phase 2.
- `tests/test_tuner_split.cpp`, two cases: "a row carrying datagen's ply
  column is placed by it" and "corpus rows on a position of the held-out set
  are counted".
- `tests/test_corpus_split.py`, new, fast label, eight cases over synthetic
  pools: no game on both sides and the same games out of every leaf's file;
  nothing lost and file order kept; seeded, reproducible, never overwritten; no
  validation position in a sample, from any named view, each with the
  precondition that one sat in the training side; exact rows, at most
  `--per-game`, in order; a small pool refused leaving no file; a
  non-contiguous game refused.

### What phase 2 should see, per file

- `src/leaf_walk.{hpp,cpp}`: compile clean under `-Werror` in `build/` and
  `build-tune/`, in `chesso_engine` and `chesso_engine_trace`; `nm
  build/src/chesso | grep -c leaf_` prints 0. **A re-configure is needed once**
  (`cmake -S . -B build`, same for `build-tune`): `src/CMakeLists.txt` globs its
  sources at configure time -- the change to `tests/CMakeLists.txt` triggers
  that on the next build anyway.
- `src/search.hpp`: comment only; `bench` and `tools/search_bench.py` at depth 9
  and 12 node- and move-identical to the parent (INV-6), and the gate's
  `No functional change` passes.
- `tools/datagen.cpp`: compiles; `--help` lists `--leaf` and no
  `--allow-tactical`; `--allow-tactical 1` exits 1; the smoke run prints the
  closing lines DEV_MANUAL shows.
- `tools/tuner.cpp`, `tools/tuner_split.hpp`, `tools/tuner_trace.hpp`: compile;
  on the four-column dedup corpus without `--holdout` every printed line and
  the emitted header identical to the parent's, which the existing tuner tests
  hold.
- Tests: fast suites 47/47 in both builds (45 plus `test_leaf_walk` and
  `test_corpus_split`), `test_tuner_split` 12 cases; LW01 to LW08 killed.
  **Unverified until then**: that the JSON suites hold a quiescence walk of two
  plies or more (both preconditions `REQUIRE` it; if not, a position is added,
  the test reading it from the engine), and Debug run time (each walk step
  clears a 1 MB table and `tt_reset` logs a line in Debug).
- `./clang-format.sh`: run once before the gate; the code was formatted by
  hand.
- `tools/plan_prose_check.py`: `--citations`, `--touches`, `--params` clean
  over this file.

### Documents

`DEV_MANUAL.md`: "Generate self-play data and fit the evaluation" rewritten
for the leaf -- the commands, S065's command kept as history, the format, the
kinds, the walk and its four refusals, the three clauses and the retired one,
the closing lines -- and a new "The fixed validation set, and a corpus fitted
against it"; the by-game split section says when the ply column decides.
`adocs/data/README.md`: S082's two files. `adocs/specs.md` checked, no change:
the engine's behaviour and the shipped weights are untouched, and no sentence
there describes datagen's filter or the held-out split. `MANUAL.md` checked, no
change: the UCI surface is untouched and its corpus mentions are history.

### Pre-registration

`adocs/data/S082_recipe.sh`'s header, written before anything ran, holds it
whole: the budget and size held fixed (100000 nodes, a million training rows,
S076's fit regime); the set (12000 games under seed 20261013, every row, the
quiescence-leaf view primary, DEC-275); eight cells -- root, qs, pv by every
row, 46 and 8 a game, less the control -- and the control, root at every row,
under five seeds; the margin m = 2.49 x sqrt(2) x s, s the control's spread;
the rule, `adocs/data/S082_recipe_read.py`, mechanical; the three outcomes; the
abort rule read off a 600-game smoke run; the hours.

### Settled by DEC-275

The two choices phase 1 left open are the coordinator's DEC-275 (2026-10-13,
under the owner's standing instruction): the quiescence view of the set is
primary, the root view fitted beside it in every cell and reported, never
decisive; the set's games are played at 100000 nodes whatever S083 chooses.
The script and the reader carry both as written.

### Fast check (2026-10-13): four findings fixed, still written blind

1. **The abort rule gated on terminal refusals**, a policy outcome. It now
   aborts only when truncated, aborted and uncertified refusals pass 1 % of
   what passed the filter, per walked leaf, and prints terminal beside them
   (the reader's `--smoke` mode, run by the script's `smoke` stage, which
   fails so the night does not start). **And the repetition rule was the
   game's**: `is_position_repeated()`, a two-fold anywhere including before the
   root, refused a PV leaf that met a position the game had held once. It is
   the search's now -- `classify_repetition()` from the sampled position --
   for the reason the policy above states; LW07 and LW08 hold it, with a new
   case.
2. **The margin rested on one A/A pair** against a best-of-eight pick, with
   S075's in-fit jitter as a floor. The control is now sampled under five
   seeds, s is their spread, m = 2.49 x sqrt(2) x s -- 2.49 the 95 % point of
   the largest of eight standard normals -- with no floor, and the looseness
   of s from five draws is stated in the script's header. The root view stays
   reported only. Thirteen samples and twenty-six fits: `compare` about
   1 h 15 m.
3. **The pv path was unguarded.** `REQUIRE(past_the_line > 0)`, made certain by
   one-move lines built from the quiescence walk's own first move, which must
   land on the quiescence leaf; LW06 skips the resolution in `leaf_walk_pv`.
   Phase 2 confirms both `REQUIRE`s from the engine -- they are its answer over
   the suites' positions, the same answer the first case's two-ply
   precondition reads -- and no position was chosen by judgement.
4. **The reader did not print the refusal shares** its rule 5 promises. It
   parses datagen's `leaf <kind>:` lines from `generate.log` and prints them,
   with every share, beside the reading.

### Shared with the steps landing first

- **S099**: `tools/datagen.cpp` -- its `run_search()` signature, its includes
  and three places in `play_games()`; this diff's edits there sit on other
  lines (one blank line apart at the includes, one at the attempt loop's
  `samples` clear). After it, the walker's search state carries no correction
  table, which `quiescence()` does not read. `src/search.hpp` -- it appends at
  the end, this edits the `quiescence()` comment.
- **S023**: `quiescence()` reads `capture_history` for its futility margin.
  The walker's state never runs `negamax_at()`, so its table stays zero and the
  leaf stays a function of the FEN; at the walk's widest window futility cannot
  fire anyway. `src/search.hpp` -- a different region. Recheck at the rebase
  that S023's quiescence writes no table.
- **S039**: `tools/datagen.cpp`'s header paragraph on DEC-055 (S039 rewords it,
  this deletes it with the clause: take this side) and `DEV_MANUAL.md`'s
  matching paragraph (the same); `tools/tuner.cpp` and `tools/tuner_trace.hpp`
  -- S039's hunks are the trace print line, the clamp, `load_report_t` and
  `error_range()`, none of them lines this diff changes, and nothing here reads
  `load_report_t::clamped`, which S039 deletes; `adocs/data/README.md` -- both
  append a section at the end: keep both.
- **S261**: none, unless its vector moves `MaxQsearchDepth`, which this file's
  older text states as shipped -- `test_plan_params` reads it either way, and
  nothing in this diff's code or tests depends on its value.

### Phase 2, with hours

DEC-155. Budgeted from S065's 4.669 games/s at 100000 nodes and S076's 1378 s
fit, the only measured datagen and fit throughput on this workstation:
`.moltke.local.md` records neither, so step 5's smoke run is the first figure
taken on today's engine with the walks, and the night is re-estimated from it.

1. Rebase onto the landing of S261, S099, S023, S039 (conflicts: below), 10 min.
2. Re-configure, build both, gate, `./clang-format.sh`, prose checks: 20 min.
3. INV-6: `bench`, `search_bench` 9 and 12 against the parent, `nm` on
   `chesso`: 10 min.
4. `tools/mutation_check.py tools/mutants/S082_leaf_walk.py`, eight
   mutants: about 25 min.
5. `adocs/data/S082_recipe.sh smoke`: about 3 min; the abort rule read.
6. `generate`: 150000 games, about 9 h at S065's rate plus the walks --
   **the night**.
7. `compare`: split, thirteen samples, twenty-six fits, the reading: about
   1 h 15 m.
8. Archive the pool's three `.validation` files with a `.sha256` (DEC-235),
   record the set's identity -- seed, games, rows, digests -- and the reading
   here; the readings and logs to `adocs/data/`.

Second tier not owed: no `make_move`, generator or search change.
`tools/gate_extra.sh` is due weekly anyway.

### Proposed `done:` stamp

2026-10-1x -- the corpus row names a resolved position: `src/leaf_walk.cpp`
walks quiescence's own choices, one fresh call a ply on a table of its own,
until it stands pat, and datagen writes that leaf (`--leaf qs`, the default),
the PV's end resolved the same way (`pv`) or the root, with the game result
the label and White's perspective throughout; it prints per leaf how often the
leaf differs from the root (<p> % qs, <p> % pv on the pool) and every refusal
-- terminal, truncated, aborted, uncertified -- with its share. The tactical
clause and `--allow-tactical` retired. `chesso` links none of it: bench <n>
and search_bench 9/12 identical. The fixed validation set: 12000 games,
seed 20261013, <rows> rows, qs view, sha256 <...>, archived; `tuner --holdout`
reads it and refuses a shared position; `tools/corpus_split.py` builds it and
the corpora. Recipe, by `adocs/data/S082_recipe.sh`'s pre-registered rule:
<outcome>. S066's split kept exact for leaf rows by datagen's ply column.
Tests: test_leaf_walk (8), test_corpus_split (8), test_tuner_split +2;
LW01-LW09 killed. Fast suites 47/47 both, format clean, prose checks clean.

### Commit text (proposed)

```
Label the corpus at the quiescence leaf and fix its validation set (S082)

The tuner fits evaluate() to game outcomes, and evaluate() is only the
last word on a position no capture changes. datagen wrote the position
the game stood on and kept a clause, --allow-tactical since DEC-055,
whose only reason was that the root might not be quiet. A row now names
the leaf quiescence resolves its position to: src/leaf_walk.cpp asks
quiescence() itself, one fresh call a ply, until it stands pat, and
refuses a terminal, truncated, aborted or uncertified walk rather than
recording it. The clause retires. chesso does not link the walk.

A leaf's FEN no longer says where the game stood, which would have cut
games in two at the tuner's S066 split, so datagen writes the sampled
ply and the split reads it. tools/corpus_split.py builds the fixed
validation set DEC-259 screens on, whole games held out before sampling
and no position on both sides, and tuner --holdout reads it.

No functional change
```


### Rebased onto `001c82e` (2026-10-14, phase 1, still blind)

Written blind under DEC-260 in the worktree `/home/max/ws/chesso-s082c`
(branch `s082c` at `001c82e`: S099's candidate `8758622` plus documents) while
S099's SPRT holds every core: nothing compiled, tested or run. S023 and S039,
which land between S099 and this step, are not in this base; where this diff
meets theirs is read off trial merges below. The diff is
`.tuning/coord/S082/s082_on_001c82e.diff`.

#### Deviations

- **A test case and a mutant, for S279.** `870d210` made quiescence test its
  limits at every node it counts, above the table probe, so the walk's
  "aborted" policy is now exactly the cap's effect -- and it was the one policy
  with no case. `tests/test_leaf_walk.cpp` "a call past the walker's node cap is
  refused as aborted": from up to 32 positions whose walk is quiet and whose
  first call searches two nodes or more (both read from the engine), a walker
  capped one node short is refused as aborted at distance 0, on exactly the
  cap's node count, the game restored. LW09 (the cap never reaches the search
  state) is the mutant it kills. A guard, not a fix: no defect was found, so no
  red-first observation is owed.
- `src/leaf_walk.hpp`'s "aborted" policy comment: the call's answer is 0 from
  the node that reached the cap and a stand pat above it, no longer "a static
  number".
- `adocs/data/S082_recipe.sh` gains the open-findings paragraph plan.md asks of
  every run's pre-registration (DEC-171): none open; the README entry says it
  was added. The rest of the pre-registration is unchanged.
- `DEV_MANUAL.md`: the merge put S099's datagen paragraph between "made before
  S082 by" and its command, splitting the sentence; moved up, after "Those are
  S082's comparison settings", its text unchanged.
- Hand formatting in `tools/datagen.cpp`: the header's format paragraph
  reflowed, one `fprintf` wrapped. The proposed stamp above now reads
  test_leaf_walk (8) and LW01-LW09.

#### Conflicts and their resolutions

`git apply --3way` of the old-base diff: **one textual conflict**,
`adocs/data/README.md`, both sections appended at the end -- both kept, S099's
first. Everything else applied cleanly, including the files the brief expected
to conflict:

- `tools/datagen.cpp`: S099's include, `run_search()`'s new parameter, the
  per-worker table and its per-game reset each sit one unchanged line from this
  diff's edits (the `evaluation.hpp` include; `tt_reset`). Read whole after the
  merge: no S082 line passes or reads the correction table.
- `src/search.hpp`: S099 appends at the end, this edits the `quiescence()`
  comment.
- `tests/CMakeLists.txt` and `tools/tuner*`: untouched between `e1c59f8` and
  `001c82e`. The tuner conflicts the brief expected are S039's, below.
- `DEV_MANUAL.md`: clean as text, wrong as prose -- the move above.

#### S279: can the walk be cut short?

`src/search.cpp` `check_limits` reads four fields of the state: `aborted`;
`node_limit` against `explored_nodes`, exactly at every call; and `*stop`, when
`explored_nodes` is a multiple of 2048. `src/search.cpp` `quiescence` calls it
right after counting its node, above the probe, and a node whose child aborted
returns its stand pat without counting another. The walker's state is its own
(`src/leaf_walk.cpp` constructor): `stop` points at the walker's own flag,
false and never set; `src/leaf_walk.cpp` `leaf_quiescence_choice` sets
`aborted` false, `explored_nodes` 0 and `node_limit` to the walker's cap
(`src/leaf_walk.hpp` `LEAF_WALK_NODE_CAP`, a million) before every call. So
the one way a walk aborts is the cap, by design, and nothing of the game's
budget or clock reaches it. What changed is precision: before S279 a table hit
on the node that reached the cap returned unchecked, so a call could run a few
nodes past it -- or finish past it unrefused; now the call stops on that node.
The policy is unchanged: `leaf_quiescence_choice` tests `aborted` before it
reads the value or the table, and `src/leaf_walk.cpp` `walk_from` refuses the
row as aborted; datagen counts it and the smoke abort rule gates on it.

#### S261's vector

`MaxQsearchDepth` stays 19 (`1b84f4b` does not move it), so the older text
above holds. QsFutilityMargin 188 to 226 changes quiescence's trees and so the
leaves; no code or test here depends on it.

#### S099: the leaf stays a function of the FEN

The walker's `search_state_t` is built by `make_unique` and given only its
table and its flag, so its `pawn_correction` keeps the null default
(`src/data_structures.hpp` `search_state_t`). quiescence reads nothing of
S099's: the corrected evaluation is computed and taught only in
`negamax_at`, which the walk never runs, and both table stores keep the raw
evaluation. The walk's table is its own and cleared before every call; the
worker's game table and correction table (`tools/datagen.cpp` `play_games`)
are never handed to it. make_move and unmake_move keep S099's pawn key, and the
walk unmakes everything it makes. So no correction table is read and no state
of the game's search reaches a `qs` leaf. The `pv` kind was never FEN-only: its
line is the game search's, which since S099 the per-game correction also
shapes. `score`, the sampled position's search score, is the corrected
search's from S099 on, as every row S099's tree writes.

#### Where this diff meets S023 and S039

Read off `git merge-file` of this tree against each rebased diff
(`.tuning/coord/S023/s023_on_c3ed1ef.diff`, `.tuning/coord/S039/s039_on_8ef820f.diff`)
over `001c82e`, written to scratch only.

S023:
- `adocs/data/README.md`: conflict, both append -- keep all sections.
- `DEV_MANUAL.md`, `src/search.hpp`: clean (S023 declares its update beside
  `history_on_quiet_cutoff`, this edits the `quiescence()` comment).
- In the code: S023's quiescence reads a capture-history cell in its
  per-move futility margin and writes none; the table is written only at
  `negamax_at`'s fail-highs. The walker never runs `negamax_at`, so every cell
  it reads is 0, the margin is QsFutilityMargin itself, and the walk is node
  for node the one S023's parent makes. **A correction to the older bullet
  above**: futility does fire inside the walk's calls -- the root raises alpha
  to its stand pat and every child runs on a finite window -- so what keeps the
  leaf a function of the FEN is the zero cell, not the window.

S039:
- `tools/datagen.cpp`: one conflict, the header paragraph on DEC-055 -- S039
  rewords it, this deletes it with its clause: take this side.
- `DEV_MANUAL.md`: one conflict, the `--allow-tactical` table row and its
  paragraph -- the same: take this side.
- `adocs/data/README.md`: conflict, keep all sections.
- `tools/tuner.cpp`, `tools/tuner_trace.hpp`: clean. S039 edits the header
  comment, the trace print line, `load_report_t`, the clamp in `smooth()` and
  `gradient()`; `--holdout` reads `mismatches` and `first_mismatch` only, and
  `error_range()`'s signature does not move.
- In the code: the walk calls `quiescence()` and `evaluate()`, never the lazy
  path S039 deletes. Its stand-pat check (`value != evaluate()`) holds on both
  trees: here quiescence's root asks the lazy evaluation at the widest window,
  where neither shortcut can fire; after S039 it asks `evaluate()`. The leaves
  of one FEN may differ between the two trees (lazy bounds inside children),
  each a function of the FEN on its own tree.

**On S099's H0** (`8758622` reverted): the revert's trial merge against this
tree is clean in `tools/datagen.cpp` and `src/search.hpp` -- no S082 line reads
S099's table -- and conflicts in `DEV_MANUAL.md` (S099's datagen paragraph,
which this text now surrounds: drop it, keep "The corpus the shipping weights
descend from") and `adocs/data/README.md` (drop S099's section, keep this
one). No code or test hunk of this diff would differ.

#### Goldens

S082 owns none: every number its tests compare is read from the engine at run
time, and the new case's 32 is a slice, not an expected value. INV-6 on this
tree: `bench` 6042933, the parent's, and `tools/search_bench.py` at 9 and 12
node- and move-identical, so the commit says `No functional change`. On the
tree S082 finally lands on, the parent is whatever S023 and S039 leave.

#### The recipe on this tree

`adocs/data/S082_recipe.sh` holds: the budget, size, fit regime, set, cells,
margin, rule and outcomes do not depend on the engine, and the engine is
"whatever HEAD is when `generate` starts, stated in the log". Its rates are
S065's and the engine they were taken on is gone (S261's vector, S099's
correction, S279), so the smoke run's games an hour re-estimates the night, as
the header already says. Open findings: none (plan.md's Open list at
`001c82e`; S281 is a conditional study, not a defect).

#### What phase 2 should see, changed from the list above

- `tests/test_leaf_walk.cpp`: compiles clean under `-Werror` in both builds;
  eight cases green; the new one reaches 32 cases, each aborted on exactly the
  cap. LW09 killed beside LW01-LW08.
- Fast suites still 47/47 in both builds: `tests/CMakeLists.txt` did not change
  between the two bases.
- `tools/datagen.cpp`, `src/leaf_walk.*`, `tools/tuner*`: compile clean;
  `./clang-format.sh` may still rewrap `tools/datagen.cpp` `parse_leaves` and
  one `CHECK_FALSE` in `tests/test_tuner_split.cpp`, each over 80 columns.
- `bench` 6042933 here and equal to the parent wherever it lands;
  `nm build/src/chesso | grep -c leaf_` prints 0.

#### Phase 2, with hours (replaces the list above)

DEC-155. The pool is the night; the rest is about 1 h 25 m before it and
1 h 30 m after.

1. Rebase onto the landed S023 and S039 (and S099's revert on H0), the
   conflicts as listed above: 15 min.
2. Re-configure both builds (`src/` globs at configure time), build, the gate
   in both builds, `./clang-format.sh` first, the three prose checks: 25 min.
3. INV-6: `bench`, `tools/search_bench.py` at 9 and 12 against the parent,
   `nm`: 10 min.
4. `tools/mutation_check.py tools/mutants/S082_leaf_walk.py` over its fixture,
   nine mutants: about 30 min. The fixture is a clean linked worktree at its
   own HEAD, so that HEAD must hold this step: S082's commit, or a scratch
   commit on a throwaway branch.
5. `adocs/data/S082_recipe.sh smoke`: 3 min; the abort rule read.
6. `generate`: 150000 games, about 9 h at S065's rate plus the walks,
   re-estimated from step 5 -- **the night**.
7. `compare`: about 1 h 15 m.
8. Archive and record as listed above: 15 min.


### Rebased onto `ced5d77` (S099 and S023 reverted, S039's candidate in), 2026-10-15, phase 1, still blind

Written blind under DEC-260 in the worktree `/home/max/ws/chesso-s082d`
(branch `s082d` at `ced5d77`) while S283's SPRT holds every core: nothing
compiled, tested or run. Since `001c82e`: S099 read H0 and `8758622` is
reverted whole (`5a04ffa`); S023 verdict 1 read H0 and is reverted
(`0cdd39a`); S039's candidate `ca321b6` is committed, its SPRT after S283's;
S284, S282, S285 and DEC-286 to DEC-293 touch nothing here. The diff is
`.tuning/coord/S082/s082_on_ced5d77.diff`.

#### Deviations

- **No code or test line changed.** Every code and test hunk of the
  `001c82e` diff applied as written; the edits of this rebase are the three
  conflict resolutions below, one paragraph of the recipe script, one README
  line and this section.
- `adocs/data/S082_recipe.sh`'s open-findings paragraph gains a dated re-read
  at `ced5d77` (none open, "The recipe on this tree" below), and the README
  entry says so. The rest of the pre-registration is unchanged.
- The worktree's index still holds the three conflicted paths as unmerged:
  resolved in the working tree, but `git add` is outside what phase 1 may run,
  so phase 2 stages them first. The saved diff is complete without it.
- Tools of this phase, both read-only over source text: a stdin `python3`
  one-off spliced the conflict hunks as an editor, and a scratch bash loop
  around `grep -P` counted the mutant anchors. Neither ran project code, a
  test or a tool, or read data.

#### Conflicts and their resolutions

`git apply --3way` of `s082_on_001c82e.diff` onto `ced5d77`: three paths
conflict, everything else applies cleanly.

1. `tools/datagen.cpp`, the header: S039 rewords the DEC-055 paragraph
   ("evaluate_lazy() when DEC-055 was written, evaluate() since S039"), this
   diff deletes it with its clause. **This side**, as ruled: the paragraph
   that replaces it states the retired clause and DEC-055 without naming
   either evaluation.
2. `DEV_MANUAL.md`, two hunks. (a) S099's datagen paragraph, which the
   `001c82e` diff carried between "Those are S082's comparison settings" and
   "The corpus the shipping weights descend from": gone at `ced5d77` with the
   revert, so dropped, and the S082 sentence kept -- the text is `e1c59f8`'s
   again. (b) The `--allow-tactical` table row and paragraph, which S039
   amended ("the lines of the day; `quiescence()` calls `evaluate()` there
   since S039"): **this side**, deleted, as at `001c82e`'s trial merge.
3. `adocs/data/README.md`: both append. Every section `ced5d77` holds is kept
   -- S099's, S023's, S285's, S039's and S283's, all evidence -- and S082's
   goes after S283's.

Clean, and read whole after the merge:

- `tools/datagen.cpp`'s code: the revert took out `correction_history.hpp`,
  `run_search()`'s correction parameter, the per-worker table and its
  per-game reset; this diff's hunks sat one line from each and land on the
  pre-S099 lines unchanged. No S082 file names a correction, a pawn key, a
  pawn hash or capture history (grep over every file of the diff).
- `tools/tuner.cpp`, `tools/tuner_trace.hpp`: S039's hunks -- the header
  comment, the trace print line, `load_report_t::clamped`, `clamped_site()`,
  the clamp in `exact_score()`, `smooth()` and `gradient()` -- share no line
  with this diff's.
- `src/search.hpp`: S099's appended probes are gone; this diff's
  `quiescence()` comment hunk is untouched by either.
- `tests/CMakeLists.txt`: unchanged since `001c82e`.

#### 1. Is the leaf still a function of the FEN under S039's quiescence?

**Yes.** `quiescence()` at `ced5d77` is `001c82e`'s but for S039's one
change: the static score is `evaluate()` (or the probed entry's `eval`) at
every window, and every store carries it where a node the lazy shortcut
answered used to store `TT_EVAL_NONE`. The walk's reading of that:

- **What a call reads.** `quiescence()` reads five fields of its search
  state -- `tt`, `explored_nodes`, `node_limit`, `aborted` and `stop`, the
  last four through `check_limits()` -- and nothing else of it: with S023
  reverted no history table, with S099 reverted no correction table. The
  walker's state is its own (`leaf_walker_t`'s constructor), `stop` points at
  its own flag, and `leaf_quiescence_choice()` resets the three counters and
  clears the walker's table with `tt_reset()`, a `memset`, before every call.
  `evaluate()` is a function of the board; its only side effect is the trace
  sink, a thread-local null unless a trace tool sets it. So a call is a
  function of the board it is handed.
- **What a call writes.** The same store sites as before S039 -- the
  stand-pat cut, a beta cut, a mate, the node's final store -- now each with
  the static score in `eval`. Inside one call a transposition reads that
  number back in place of `evaluate()`: the same number, the position being
  the same (a partial-key collision, 7e-10 a probe, is a property of the tree
  and deterministic per FEN, and was one for the score and the bound before
  S039). Every write lands in `walker.tt`, a `TT_MIN_MB` table of the
  walker's own, cleared before the next call.
- **The one store the walk depends on** is the call's own root store, read by
  `leaf_quiescence_choice()` right after the call. At the root the probe
  misses (the table was just cleared), so the stand pat is `evaluate()`
  itself, unsubstituted and exact; the window is the widest, so no stand-pat
  cut and no beta cut; the futility fold cannot pass the stand pat at the root
  (a fold needs a best case at or below alpha, which is the stand pat there);
  so the final store is `TT_PV_NODE`, the last statement before the return.
  `tt_store_entry()` never refuses a store of a position the cluster does not
  hold, and refuses one it holds only for a shallower depth in the same
  generation -- every entry in the walker's table is `TT_DEPTH_QS`. The
  stand-pat certification, `value != evaluate()`, holds as it did.
- **Does it reach datagen's next search?** No. `run_search()` is handed
  `play_games()`'s own `tt` and a fresh state per move; the walker never holds
  either, and the game is handed back by `unmake_move()` -- board, hash,
  accumulators, history size (`tests/test_leaf_walk.cpp` asserts all but the
  accumulators, which `test_invariants` checks across make and unmake over its
  corpus). So
  the walks cannot change a game: one seed plays the same games whatever
  `--leaf` lists, which phase 2 can show in two minutes (step 3 below).
- **The pv kind** is unchanged and never was FEN-only: its line is the game
  search's, S039's on this tree.

The leaves of one FEN differ between `001c82e`'s tree and this one -- exact
stand pats move cutoffs inside a call, and `evaluate()` lost its clamp --
each a function of the FEN on its own tree.

#### 2. S279's limit check: does the answer hold?

**Yes, word for word.** `check_limits()` is the same text at `001c82e` and
`ced5d77`, and `quiescence()` still counts its node, then tests the limits,
above the probe; a node whose child aborted still returns its stand pat
without counting another. The `quiescence()` bodies of the two bases differ
only in S039's static-score lines and comments. So the walk aborts only on
the walker's cap, on exactly the node that reaches it, and
`leaf_quiescence_choice()` still tests `aborted` before it reads the value or
the table. "a call past the walker's node cap is refused as aborted" and LW09
hold as written.

#### 3. Does any S082 code read what S039 deleted?

**No.** Grep over every file of the diff: no `evaluate_lazy`,
`LAZY_EVAL_MARGIN`, `LazyEvalMargin`, `load_report_t::clamped`,
`clamped_site` or `clamp_binds`; the clamps the grep finds in
`tools/tuner_split.hpp` and `tests/test_tuner_split.cpp` are S066's
validation-fraction clamp. The walk calls `quiescence()` and `evaluate()`.
`tuner --holdout` reads `load_report_t::mismatches` and `first_mismatch`,
both kept, and calls the eight-argument `error_range()`, whose signature S039
did not move. **No conflict in the loss**: the held-out figure is the same
`error_range()` over `smooth()` as the training one, handed another dataset
and index range, so S039's unclamped `smooth()` applies to the fixed set and
the corpus alike, and the fixed set's identity check is S039's unclamped
`exact_score()` against S039's `evaluate()`, as the corpus's is. Held-out
losses read on this tree are not comparable with ones read before S039 --
the clamp bound on 0.338 % of the dedup corpus's rows at the shipped weights
-- which touches nothing here: the recipe's twenty-six fits are read on one
tree.

#### 4. On S039's H0, per hunk

A revert of `ca321b6` against this diff, in either order, conflicts in
exactly the two places it did at `001c82e`'s trial merge, both documentation:

| file | hunk | on S039's H0 |
|---|---|---|
| `tools/datagen.cpp` | the header, DEC-055's paragraph deleted | **differs**: the revert restores the pre-S039 wording, which names `evaluate_lazy()` by its line in `quiescence()`; conflict, take this side |
| `tools/datagen.cpp` | the sixteen code hunks | same |
| `DEV_MANUAL.md` | the datagen section's opening, commands and corpus sentence | same |
| `DEV_MANUAL.md` | format, leaf kinds, the walk, the clauses, closing lines, rates, the fixed validation set | **differs**: the revert restores the `--allow-tactical` paragraph's pre-S039 ending; conflict, take this side. S039's other DEV_MANUAL hunks sit 60 lines and more from this one |
| `DEV_MANUAL.md` | the by-game split's two hunks | same |
| `adocs/data/README.md` | S082's section, appended after S283's | same |
| `tools/tuner.cpp` | all ten | same: the revert's two hunks, the header comment and the trace print line, share no line with them. The fixed set's clamp count is not printed on H0, the corpus's is: `--holdout` never reads `clamped`, a diagnostic |
| `tools/tuner_trace.hpp` | `parse_line()`'s refusal | same |
| `src/leaf_walk.{hpp,cpp}`, `src/search.hpp` | all | same: at the walk's root the window is the widest, where the lazy shortcut cannot fire, so the root's stand pat is `evaluate()` there too and the certification holds; inside a call a node the shortcut answers stores `TT_EVAL_NONE`, deterministic per FEN |
| `tests/`, `tools/corpus_split.py`, `tools/tuner_split.hpp`, `tools/mutants/S082_leaf_walk.py`, the recipe and its reader | all | same; no anchor moves |

The parent's bench moves with the verdict: 6100949 on H1 or no verdict (as
landed, DEC-274), 5636835 on H0. S082's commit says `No functional change`
on either.

#### Mutant anchors

Every mutant anchor of every file in `tools/mutants/` occurs exactly once in
its target on this tree: 197 mutants in 27 files, 180 literal anchors and the
three shared gate strings (`S113_probcut.py` `GATE`, `S116_razoring.py`
`GATE`, `S131_quiet_queen_promotions.py` `FILTER`) that build the other 23,
each counted by `grep -Pzo` over the whole target. LW01 to LW09 among them.
The only `src/` files this diff touches are `src/search.hpp` (a comment, no
mutant names it) and the two new `leaf_walk` files.

#### Goldens

S082 owns none, unchanged: every number its tests compare is read from the
engine at run time or set by the case itself. INV-6 is the expectation:
`bench` 6100949, the parent's, and `tools/search_bench.py` at 9 and 12 node-
and move-identical to `ced5d77`; on S039's H0, 5636835 and the revert's.

#### The recipe on this tree

`adocs/data/S082_recipe.sh` and `S082_recipe_read.py` hold. Nothing in the
budget, size, fit regime, set, cells, margin, rule or outcomes depends on the
engine. The reader parses `best: train ... validation ...` and `fitted K =`
from the tuner and datagen's `leaf <kind>:` lines; S039 changed neither (it
changed the tuner's `trace:` line, which nothing here reads). The engine is
"whatever HEAD is when `generate` starts" and "S261, S099, S023 and S039 land
before this runs": S099 and S023 have, as reverts; S039 lands with its
verdict, which follows S283's. Open DEC-171 findings at `ced5d77`'s Open
list: **none** -- S283 is an SPRT, S039 a committed candidate, S281 void on
S099's H0, S282 and S285 (both filler) closed. The paragraph now says so.
S039 cost 2.79 % of nps and the walks are new, so the smoke run's games an
hour re-estimates the night, as the header already says.

#### Phase 2, with hours (replaces both lists above)

DEC-155. After S283 and S039's SPRT, since the pre-registration needs S039
decided. The pool is the night; about 1 h 25 m before it and 1 h 30 m after.

1. Rebase onto the tree S039's verdict leaves: on H1 or no verdict nothing
   moves; on H0 the two documentation conflicts above, this side both. Then
   `git add DEV_MANUAL.md adocs/data/README.md tools/datagen.cpp`. 15 min.
2. `cmake -S . -B build` and `cmake -S . -B build-tune` (`src/` globs at
   configure time), `./clang-format.sh` first, then the TESTS gate in both
   builds, and `tools/plan_prose_check.py` `--citations`, `--touches`,
   `--params` over this file: 25 min.
3. INV-6: `build/src/chesso bench` against the parent's total and eight
   replies, `tools/search_bench.py` at 9 and 12 against the parent,
   `nm build/src/chesso | grep -c leaf_`: 10 min. **Proposed, not in the
   brief**: the walks leave the games alone (answer 1), shown by
   `build/tools/datagen --leaf root --games 20 --threads 1 --seed 7 --out a`
   against `--leaf root,qs,pv --out b` at the same settings and `cmp a
   b.root`, one thread so the row order is the games' -- 2 min.
4. `python3 tools/mutation_check.py tools/mutants/S082_leaf_walk.py
   .ref-builds/mut` over a clean fixture whose HEAD holds S082 (its commit, or
   a scratch commit on a throwaway branch), nine mutants: about 30 min.
5. `adocs/data/S082_recipe.sh smoke`: 3 min; the abort rule read.
6. `adocs/data/S082_recipe.sh generate`, detached under a four-exit watcher:
   150000 games, about 9 h at S065's rate plus the walks, re-estimated from
   step 5, abort past 18 h -- **the night**.
7. `adocs/data/S082_recipe.sh compare`: about 1 h 15 m.
8. Archive the three `.validation` files with a `.sha256` (DEC-235), record
   the set's identity and the reading here, the logs to `adocs/data/`: 15 min.

Second tier not owed: no `make_move`, generator or search change.

#### What phase 2 should see (changed from the lists above)

- `src/leaf_walk.{hpp,cpp}`, `tools/datagen.cpp`, `tools/tuner*`: compile
  clean under `-Werror` in both builds; `./clang-format.sh` rewraps at least
  the two lines past 80 columns, in `tools/datagen.cpp` `parse_leaves` and one
  `CHECK_FALSE` in `tests/test_tuner_split.cpp`; the rest was formatted by
  hand.
- Fast suites 47 of 47 in both builds: `tests/CMakeLists.txt` has not moved
  since `001c82e`. `test_leaf_walk` eight cases green, `test_tuner_split` its
  two new cases green, `test_corpus_split` eight.
- **Unverified until then, as before**: that the JSON suites still hold
  quiescence walks of two plies or more on S039's tree, where the leaves
  moved with the evaluation. Three `REQUIRE`s read it from the engine.
- `bench` 6100949, equal to the parent; on S039's H0, 5636835. `nm` prints 0.
- LW01 to LW09 killed.
- Smoke: truncated, aborted and uncertified each 0 or near it per walked
  leaf, aborted and uncertified being unreachable by construction; terminal
  printed, never gated.


### Phase 2 on `816d70c`, 2026-10-16, up to the pool

The machine was this step's alone, by the coordinator's brief. Worktree
`/home/max/ws/chesso-s082e`, branch `s082e` at `816d70c`: S039 read H0 and is
reverted (`74d0adf`), so the lazy shortcut, the clamp and `LazyEvalMargin`
are back; S283 read H1 and S286 relanded S099 (`816d70c`, bench 6042933), the
pawn key and the pawn-keyed correction, one table per datagen worker cleared
per game (DEC-294: S082's corpus is generated by this engine). `src/` at
`816d70c` is `001c82e`'s but for one comment in `src/data_structures.hpp`.
Nothing committed, no ref moved; `generate` not started -- the coordinator
starts runs (PLAN). Everything is staged; the diff is
`.tuning/coord/S082/s082_phase2.diff`.

#### Deviations

- **A mutation fixture commit object, no branch.** `tools/mutation_check.py`
  needs a clean linked worktree whose HEAD holds the step, and nothing may be
  committed: `git commit-tree` made `8dc4e92` (parent `816d70c`, the staged
  tree `168db29`), checked out detached at `.ref-builds/mut-s082` -- S286's
  form, whose fixture `4063e59` had no branch either. No ref names it. The
  parent's own build for INV-6 is the detached worktree `.ref-builds/816d70c`.
- **LW06 was stillborn and is fixed.** In Release the `assert` in
  `leaf_walk_pv()` is compiled out, so the mutant that drops `walk_from()`
  left `walker` unread and `-Werror=unused-parameter` refused it -- the class
  the tool's docstring names, answered the documented way: the mutant now
  writes `(void)walker;` before its replacement. A defect in the mutant list,
  not in the walk; re-run alone, killed.
- **The first mutation pass refused at its baseline**:
  `test_clang_format_script` red without `CLANG_FORMAT_MAJOR=22`, this
  machine's override (DEC-146, `.moltke.local.md`). Re-run with it; the
  refused log is kept beside the others.
- **`./clang-format.sh` reformatted eight places in three files**
  (`tests/test_leaf_walk.cpp`, `tests/test_tuner_split.cpp`,
  `tools/datagen.cpp`), not the two lines the blind record predicted; layout
  only.
- **`test_plan_citation_freshness` flagged the `ced5d77` section above** for a
  line-form quote of DEC-055's old wording; reworded to name the symbol.
- **The fast suite is 48 tests, not 47**: 46 on the parent (`ctest -N -L fast`
  in the main tree's build) plus the two this step adds. The blind count of
  45 was one short.
- **`adocs/data/S082_recipe.sh`, before the pool exists** (its header allows
  it until then): `engine_stamp()` names a tree with `src/` or `tools/`
  changed by the git blob id of that diff and saves it beside the log --
  fastchess.sh's form for a dirty candidate (S256) -- where it printed only
  "-dirty", which names no engine and is what an uncommitted S082 would have
  written; the DEC-171 paragraph names S287; a LAUNCH paragraph carries the
  command, OUT, hours and markers. The rule, cells, margin and outcomes are
  untouched.
- **Run beside the list**: the end-to-end path `compare` takes, on the
  two-minute pool below -- `tools/corpus_split.py split` and `sample`, then
  `tuner --holdout` -- so a defect there shows today and not after the night.

#### Rebase

`git apply --3way` of `s082_on_ced5d77.diff`: the three conflicts the record
predicted, resolved as ruled -- the `tools/datagen.cpp` header and
`DEV_MANUAL.md`'s `--allow-tactical` paragraph to this side (both now against
the pre-S039 wording S039's revert restored), `adocs/data/README.md` both
appended, S082's after S286's. S099's per-worker table, `run_search()`'s
correction parameter and the include merged in from the base without a
conflict. **Every code file is byte-identical to the `001c82e` resolution**
-- `tools/datagen.cpp`, `src/search.hpp`, `src/leaf_walk.{hpp,cpp}`,
`tools/tuner.cpp`, `tools/tuner_trace.hpp`, `tools/tuner_split.hpp`,
`tests/test_leaf_walk.cpp`, compared against that diff applied to `001c82e`
in a scratch index -- because `816d70c`'s `src/` and `tools/` are
`001c82e`'s. One prose fix, as at `001c82e`: S286's datagen paragraph landed
between "made before S082 by" and its command, and moved up, its text
unchanged. Mutant anchors: 217 mutants in 29 files, 200 literal anchors and
the three shared gate strings for the other 23, each exactly once.

#### The four answers on this tree

1. **The leaf is a function of the FEN, and the walk never touches the game's
   correction table.** `quiescence()` reads `tt`, `explored_nodes`,
   `node_limit`, `aborted` and `stop` of its state and nothing else; the
   corrected static score (`corrected_static_eval()`) and the table's update
   (`correction_update()`) are reached only from `negamax_at()`, which the
   walk never runs, and the tune build's pawn-key census only from the
   former. The walker's state is built by `make_unique`, so its
   `pawn_correction` is null, and nothing assigns it; datagen's per-worker
   table is handed to `run_search()` alone. `make_move()` keeps the pawn key
   on the board and the walk unmakes every move it makes. **Measured**: from
   one seed, `--leaf root` and `--leaf root,qs,pv` write byte-identical root
   files (20 games, 5000 nodes, one thread, seed 7: 1606 rows, sha256
   `6b56eecf...`) -- with the correction table live in every game, a walk that
   disturbed the board, the pawn key, the table or the game's search would
   have changed a move.
2. **S279 holds**: `check_limits()` and its place in `quiescence()` are
   `001c82e`'s, and LW09 is killed.
3. **Nothing S082 reads was deleted**, since nothing is now. At the walk's
   root the window is the widest, where `evaluate_lazy()`'s shortcut cannot
   fire, so the root's stand pat is `evaluate()` and the certification holds;
   inside a call a node the shortcut answers stores `TT_EVAL_NONE`, which no
   later call sees, the table being cleared.
4. **`--holdout` against the clamped trace: one computation for both sides.**
   The fixed set is loaded by the same `load()`, its identity check the
   clamped `exact_score()` against the clamped `evaluate()`, and its error
   read by the same `error_range()` over the clamped `smooth()`. Its clamp
   count is collected and not printed, a diagnostic; the corpus's is on the
   `trace:` line. Run end to end on the two-minute pool: `split` (4 of 20
   games held out), `sample` (600 rows, 0 dropped), and a 30-epoch fit with
   `--holdout` on the qs view -- 0 rows differ from `evaluate()`, the clamp
   binds on 5, the header's `split` line names the set and its sha256, and the
   reader's `best:` and `fitted K =` patterns match; the same fit with the
   uncut pool as corpus is refused, 273 rows on a held-out position.

#### Readings

| check | reading |
|---|---|
| configure, build | both builds re-configured (`src/` globs) and built, `-Werror`, no warning |
| fast suite | 48 of 48 in `build/` and in `build-tune/`, `CLANG_FORMAT_MAJOR=22`; `test_leaf_walk` 8 cases, 36781 assertions; `test_tuner_split` 12 cases; `test_corpus_split` 8 |
| format | `./clang-format.sh --check` clean after formatting |
| prose | `tools/plan_prose_check.py` `--prose`, `--citations`, `--touches` over this file clean; `--params` clean in its whole-tree form (given one step file it exits 1 on any step file, main's included, its phrases living in the documents); `--gate` agrees |
| INV-6, bench | `build/src/chesso bench` 6042933, the parent's, and the eight replies identical |
| INV-6, search_bench | depth 9: 62042, 125888, 34107 nodes, best c3d5, e2a6, d7c8q; depth 12: 214973, 595000, 217107, best c3d5, d5e6, d7c8q -- identical to the parent's |
| `nm build/src/chesso` | 0 `leaf_` symbols of 579; `datagen` carries the walk |
| mutation | LW01-LW05 and LW07-LW09 killed in the first full pass, LW06 killed alone after its fix: 9 of 9, each by its own case; bench unmoved by every one |
| walks leave the games | `--leaf root` and `--leaf root,qs,pv`, same seed: root files byte-identical |
| smoke (`S082_recipe.sh smoke`, OUT `.tuning/s082`) | 600 games in 192 s, **11250 games an hour**; 51459 passed the filter of 61284; root 85.77 rows a game; qs 51444 rows, differs from the root on 26.18 %, 0.498 plies on average, refused terminal 15 (0.029 %); pv 50194 rows, 13.86 plies on average, 13.38 of them the line's, refused terminal 1265 (2.458 %); truncated, aborted, uncertified 0 on both; walks 0.019 % of the games' nodes; **abort rule clear** |

The smoke's figures are a 600-game reading, recorded for the pool's log to be
read against, and decide nothing but the abort rule.

#### The fast check's two gaps, closed (2026-10-16)

The fast check found no blocking defect and two gaps in the recipe script,
both closed before the pool exists. Deviations first:

- **datagen itself now fails, a `tools/` change** the fast check offered as
  optional and this step took: one row or flush that does not reach its file
  sets `write_failed`, every worker stops at its next game, `fclose()` is
  checked, and `main()` exits 1 after its closing lines with a `datagen:
  FAILED` line -- for a failed write, and for fewer games played than
  `--games` booked (a worker at its attempt cap). Both exited 0 before.
  `tests/test_datagen_exit.sh`, new, fast label (49 tests now): a run to
  `/dev/full` (skipped where there is none) and a run whose every opening is
  discarded (`--opening-limit -1`) must exit non-zero and say why, each
  precondition read from the run's own closing line, beside a control that
  writes what it counts and exits 0. **Observed red** against the datagen
  before the change: the `/dev/full` run counted 55 rows and exited 0, the
  discarded-opening run played 0 of 1 games and exited 0; green after. INV-6:
  `bench` 6042933 and `nm` 0, `src/` untouched. DEV_MANUAL's datagen section
  says so.
- **A `verify` stage** runs the pool check on demand (`verify [PREFIX LOG
  GAMES]`, by default the pool `generate` writes), so the check can be shown
  failing on a copy, re-run after the night, and is what `compare` runs
  before it reads the pool. Not asked for; it is the dry path the fast check
  allowed.

**Gap 1, DONE without a completeness check.** `check_pool()`: per leaf, the
log's line plays the booked games and names this file, the file is a regular
file ending on a newline, holds the rows the line counts, and every row has
seven fields (`awk`, all rows). `generate` runs it whatever datagen exited,
writes its findings and the file digests to `generate.log`, prints them, and
returns DONE only on datagen's exit 0 and a clean check; `smoke` runs it on
the smoke's files, `compare` before its split.

**Gap 2, the stamp names the tree and not the binary.** `binary_line()`
writes the sha256 and path of what a stage executes -- datagen in `smoke` and
`generate`, the tuner, `corpus_split.py`, the reader and `python3 --version`
in `compare`. `engine_stamp()` refuses, printing the files and nothing on
stdout, when `git ls-files --others` finds any file under `src/` or `tools/`
but `__pycache__`, ignored files included, since `src/`'s GLOB compiles any
`.cpp` there.

**Shown**, each on this tree's datagen (sha256 `4b0e2848...`):

| run | result |
|---|---|
| `smoke` re-run, OUT `.tuning/s082` (the first smoke's log and console kept as `smoke/datagen.first.log`, `smoke.first.console`) | stamp `816d70c+31ef3a300bc7` with its diff, the datagen line, the three checks clean (51459, 51444, 50194 rows of 600 games, the first smoke's counts exactly), 193 s, 11191 games an hour, abort rule clear, `S082-SMOKE-DONE` |
| `generate`, a scratch copy of the script at 4 games and 2000 nodes, OUT a scratch directory | three checks clean, the binary line in the log, `S082-GENERATE-DONE` |
| the same with `pool.tsv.pv` a link to `/dev/full` | datagen exits 1 with its FAILED line; the check reports the pv file not a regular file; `S082-GENERATE-FAILED`, the reasons above it and in the log |
| `verify` on copies of that good pool | clean copy DONE; qs cut 10 bytes short: no final newline, 300 rows of 301, one row short of seven fields -- FAILED; last pv row dropped: 298 of 299 -- FAILED; 5 games booked: each leaf 4 of 5 -- FAILED; a six-field row in root -- FAILED; a copy read against the original's log: the line names another file -- FAILED |
| `smoke` with an untracked `src/zz_s082_untracked_probe.cpp`, and with the ignored `src/tmp.hpp` | refused before datagen, the file named, `S082-SMOKE-FAILED`; both probes removed after |

The scratch copy of the script and both probes were deleted after their runs.
The first draft's hang -- the check's summary hashing `/dev/full` through
the link -- was fixed by requiring regular files in both places.

#### The pool, for the coordinator to start (supersedes the first draft)

The coordinator commits S082 to main and launches from main's `build/`, so
the stamp reads a clean sha; nothing untracked may sit under `src/` or
`tools/` there (`__pycache__` excepted), or the stage refuses.

```
cd /home/max/ws/chesso && cmake --build build -j12 &&
mkdir -p .tuning/s082 &&
OUT=/home/max/ws/chesso/.tuning/s082 THREADS=12 nohup \
    adocs/data/S082_recipe.sh generate \
    > /home/max/ws/chesso/.tuning/s082/generate.console 2>&1 &
```

- **OUT** `/home/max/ws/chesso/.tuning/s082` (both smokes are under `smoke/`
  there): `pool.tsv.root`, `pool.tsv.qs`, `pool.tsv.pv`, `generate.log` --
  stamp, datagen's sha256, command, datagen's lines, elapsed, the check's
  findings, each file's rows and sha256. It refuses if `pool.tsv.qs` exists.
- **Hours**: about 13.4 h at the smokes' 11191 to 11250 games an hour, about
  12 h if the full run beats its smoke as S065's did (12 %). Past one night:
  DEC-155's night scheduling holds, and the morning's work waits on it.
  Watcher ceiling 27 h.
- **Markers**: `S082-GENERATE-DONE` or `S082-GENERATE-FAILED`, the last line
  of `generate.console`, the reasons above a FAILED. Poll the whole file,
  with the pid and the ceiling (WATCHERS). `adocs/data/S082_recipe.sh verify`
  with the same OUT re-reads the pool at any time.
- **Disk**: about 2.9 GB for the pool, as much again for `compare`'s split,
  about 1 GB for its samples; 21 GB free on 2026-10-16.
- **Before launch**: plan.md's Open list re-read (the script's DEC-171
  paragraph names S287 open), the machine idle and on the `performance`
  governor, and the tree's `src/`, `tools/` and `build/` left alone until the
  pool is written.

After the pool: `adocs/data/S082_recipe.sh compare` with the same OUT, about
1 h 15 m to 1 h 30 m, then archiving, as listed above.


### 2026-10-20: the pool cut by a power loss, and its top-up (DEC-296)

Written in the worktree `/home/max/ws/chesso-s082f` (branch `s082f` at
`df0d3ab`) while the top-up held every core: nothing built, and no engine,
datagen, fit or match run. What ran: the merge on fixtures cut from the smoke
pool, under the scratchpad, and the read-only part of it -- part 1's cut and
what its rows give -- on the real pool. Nothing committed; the diff is
`.tuning/coord/S082/s082_phase3.diff`. **Every pre-registered cell, seed and
reading rule stands**: `adocs/data/S082_recipe.sh`'s header is unchanged
inside its two rules, and a dated note below the closing one points here.

#### What happened

- `generate` ran from main's `build/` at `5b0e170`: 150000 games, 100000
  nodes a move, 12 threads, seed 20261013, datagen sha256 `4b0e2848...`. A
  power cut stopped it at 2026-10-16 12:58, 11 h 22 m in.
  `.tuning/s082/generate.log` holds the stage's header and datagen's first
  line and nothing after: no closing lines, no elapsed time, no check, so
  `check_pool()` cannot read the pool.
- `pool.tsv.root`, `.qs` and `.pv` each end on a newline and then a tail of
  NUL bytes -- 1691, 1360 and 4054 -- and nothing else is damaged: no NUL
  before the tail, every row seven fields, each game's rows contiguous, qs's
  and pv's games all root's and in root's order, all three ending on game
  122926 (95 rows in each). The coordinator measured it; the dry run below
  reproduces it from the files.
- **Part 1** (DEC-296) is the 122748 games before game 122926 in file order;
  game 122926 is dropped with the NUL tails.
- **Part 2** is the top-up: `datagen --out .tuning/s082/topup/pool.tsv --leaf
  root,qs,pv --games 27252 --nodes 100000 --threads 12 --seed 2026102000`,
  the same binary (sha256 checked equal before launch), log
  `.tuning/s082/topup/topup.log`, console `topup.console` ending
  `S082-TOPUP-DONE` or `-FAILED`. Its game ids start at 0 and collide with
  part 1's.

#### The merge

A fifth stage, `adocs/data/S082_recipe.sh merge`, and the program it runs,
`adocs/data/S082_pool_merge.py` (standard library only).

- **Where the pool sits.** `compare`, `verify` and the reader read
  `$OUT/pool.tsv.*` and `$OUT/generate.log`. The merge moves part 1's three
  files and its log once, by `mv` on one filesystem (checked), to
  `$OUT/part1/`, writing their sha256 first to `$OUT/part1/originals.sha256`,
  and checks them against it on every run; it never opens them for writing.
  The merged pool and its log then sit at the names `compare` reads. This is
  the smaller change: teaching `compare` another prefix touches its split,
  its thirteen samples and twenty-six fits' paths and still leaves the reader
  reading `$OUT/generate.log`; the move leaves `compare`, `verify` and the
  reader as they are, and `compare.log` names the paths the pre-registration
  names. Part 2 stays where it was written, `$OUT/topup/`, read only.
- **The merged log says what it is.** Its first line is `MERGED_HEADER`,
  "S082 pool, merged from two datagen runs (DEC-296); not one run's log";
  then the merger's and the script's sha256, part 1's log verbatim and
  indented (its command, seed and binary hash), that it stopped before
  datagen's closing lines, the originals' sha256, part 2's log verbatim and
  indented (its command, seed, binary hash, every datagen line, exit and
  elapsed), part 2's check against its own log, then the merger's lines --
  both runs' settings, part 1's cut, the figures side by side, part 1's
  recomputed coverage, what part 1 lost, the offset, the read-back -- and one
  `merged <leaf>: R rows from 150000 games written to FILE, sha256 X; part
  1's 122748 games with a row before its cut and part 2's 27252 games
  played, ...` line per file, then the final check. No line imitates
  datagen's output for part 1; datagen's own lines appear only quoted,
  indented, under the part they belong to.
- **`check_pool()` reads a merged log honestly.** A log whose first line is
  `MERGED_HEADER` is read by its `merged <leaf>:` lines instead of datagen's
  `leaf <leaf>:` lines -- the same shape, so the same checks: the games equal
  the booked 150000, the line names the file, the file ends on a newline,
  holds the rows the line counts, every row seven fields -- and one more, the
  file's sha256 equals the one the merge wrote. Any other log is read as
  before.
- **Part 1's cut is computed from the files.** One pass per file indexes its
  games in file order and refuses -- never cuts -- on a NUL byte before the
  tail, anything but NUL bytes after the last newline, a row without seven
  fields or with a score, phase, game or distance that is not a number, a
  game whose rows are not contiguous; then a qs or pv game root does not hold
  or holds in another order, or a kept game with more rows in a leaf than in
  root. The cut is the earliest, in root's order, of the last game each file
  reaches; the games before it are kept, a prefix of each file copied byte
  for byte. What the kept rows give is the whole file's less a re-read of the
  dropped range.
- **Part 2 is checked twice before anything is written.** `check_pool()`
  against `topup.log` at the games its own `datagen:` line booked, exactly as
  `generate` would have; then the merger checks the two logs -- one datagen
  sha256, part 1's seed `SEED` and part 2's `TOPUP_SEED` (2026102000, a new
  constant beside `SEED`), every other setting equal, per-worker seed ranges
  apart, part 2 `exit 0` with no `datagen: FAILED` line, part 1's log without
  closing lines -- and that part 2's rows give every figure its log prints
  that rows determine: rows, how many differ from the root and the mean
  distance, the positions that passed (root's rows), each leaf's refusals'
  total (root's rows less the leaf's), every coverage cell. Part 1's kept
  games and part 2's booked games must make `GAMES`, 150000.
- **The offset** is one constant, stated in the log: the smallest power of
  ten above every game id in part 1's files -- 1000000 on the real pool,
  whose largest id is 123601. Part 2's rows change in the game field only.
  `tools/corpus_split.py` assumes nothing of ids but that they are integers
  and contiguous per game: `split` pools the ids as a set, sorts and shuffles
  them, `sample` indexes games by position in the file; neither assumes ids
  are contiguous, bounded or as many as the games played (read whole), and
  the tuner reads no game field. Because `split` sorts the ids before its
  seeded shuffle, any offset above part 1's largest id holds out the same
  games.
- **Written, read back, renamed, checked.** The merger writes
  `$OUT/pool.tsv.<leaf>.merging`, reads them back -- the same structural
  checks, every id once, each file's games part 1's kept then part 2's
  shifted, rows and every recomputed figure equal to part 1's kept plus part
  2's -- and removes them on any refusal. The stage renames the three files
  and the log, and runs `check_pool()` on the result by its merged branch.
- **Re-runnable, never overwriting.** A failed run leaves part 1 moved and
  checked, part 2 untouched and no merged file; its log is kept as
  `$OUT/merge.failed.log`; the next run starts from there. A run that finds a
  merged pool, or anything at `$OUT/generate.log` once part 1 is moved,
  refuses. The stage's last stdout line is `S082-MERGE-DONE` or
  `S082-MERGE-FAILED`.

#### What part 1 lost, and what its rows still give

Lost with datagen's closing lines, and not in the rows -- each of them
printed, never gated, and deciding nothing (DEC-275):

- the filter's considered positions and its skips by clause (in check, mate,
  score past the limit) and searches that completed no iteration;
- the refusals **by kind** -- terminal, truncated, aborted, uncertified -- of
  which only each leaf's total survives, as root's rows less the leaf's;
- the pv line's share of the pv distance ("of them the line's");
- the walks' nodes and the games' search nodes;
- the elapsed time (the log's start, 01:36:14, and the files' last write,
  12:58, bound the run's, not the kept games');
- the games played: a game whose every position failed the filter writes no
  row and is invisible. The dry run counts 3 such games inside workers'
  runs, a lower bound, so part 1's games are counted as games with a row.

**Not lost, against the brief's list: the coverage tables.** A cell counts a
written row by its own phase field and the |score| field, both in the row.
The merger recomputes them, and on the smoke pool's three files (`check`
mode, read only) and on the fixture's part 2 they equal datagen's tables cell
for cell, with the rows, the differ count, the mean distance, the passed
positions and every leaf's refusals' total. So part 1's coverage is printed,
recomputed, labelled as such.

The reader's rule-5 print of "the pool's leaves, from generate.log" reads
`leaf <kind>:` lines and finds none in the merged log, where datagen's lines
sit indented inside part 2's quoted log: it prints the heading and nothing.
That is the honest output -- no datagen run made this pool, and printing part
2's lines there would give 27252 games' figures as the pool's. The reader is
unchanged; the figures are in the merged log, side by side.

#### Deviations

- **The coverage tables are recomputed**, above, where the brief listed them
  as lost.
- **A leaf file reaching past root's last game is refused, not cut**: its
  game there is "a qs or pv game root does not hold". A power loss persists
  each file's pages on its own, so it could happen; the real pool's three
  files end on one game. The fixture was first built that way and refused,
  which is how this was found; it was rebuilt with root reaching furthest.
- **Two checks beyond the brief's list**, both cheap: no kept game has more
  rows in a leaf than in root (a leaf row is a root row resolved, and the
  refusals' total rests on it), and part 2's rows against its log's figures.
- **No test in `tests/`**, which the brief excludes. The fixture builder and
  the driver are kept, uncommitted, at `.tuning/coord/S082/merge_test/`:
  `bash merge_test.sh WORKTREE SMOKE_DIR SCRATCH`, about two minutes.
- `DEV_MANUAL.md` checked, no change: it shows the pipeline's commands and
  names the recipe script as the place for S082's runs, and the merge is one
  pool's repair. `MANUAL.md` checked, no change. `adocs/data/README.md`:
  the recipe's entry names the amendment, and the merger has its own row.

#### Tested on fixtures (2026-10-20)

Each fixture is cut from `.tuning/s082/smoke/pool.tsv.*` (600 games): part 1
is root to the game at root position 402, qs to 400, pv to 401, each stopped
inside that game, with NUL tails of 1691, 1360 and 4054 bytes and a
`generate.log` in `generate`'s form booking 1000 games; part 2 is the smoke
pool whole, with its own datagen log as the top-up's `run.sh` writes one
(paths and seed rewritten). The stage runs from a scratch copy of the script
with `GAMES=1000`. Every run also checks part 1's four files' sha256
against the fixture's, wherever they then sit: unchanged in all 15 runs.

| case | result |
|---|---|
| good | DONE: cut at qs's last game, 400 games kept (root 3, qs 1, pv 2 games dropped); offset 1000; merged 85910, 85880, 83839 rows, 1000 games |
| `verify` on it | DONE, by the merged lines |
| the merged files | each is part 1's kept bytes (`cmp`) then part 2 whole with ids + 1000 (`awk`, `cmp`), every id once |
| `corpus_split.py split` on it | reads it: 1000 games, 100 held out |
| a merged file touched after | `verify` FAILED: sha256 not the merge's, rows not the line's |
| merge again | FAILED: a merged pool is never overwritten |
| NUL inside a qs row | FAILED: "a NUL byte before the file's tail" |
| a root row of six fields | FAILED: "6 fields, datagen's rows have seven" |
| a qs row moved behind the next game | FAILED: "the rows of game 8 are not contiguous" |
| two pv games swapped | FAILED: "game 54 is listed in another order than root lists it" |
| a qs game renumbered 99999 | FAILED: "game 99999 is not in root's file" |
| `x` inside pv's NUL tail | FAILED: "what follows its last one is not a tail of NUL bytes" |
| each of those six | part 1 moved and checked, no merged or `.merging` file, `merge.failed.log` kept |
| part 1's log with an `elapsed` line | FAILED: "has datagen's closing lines" |
| part 2's binary sha256 another | FAILED: "datagen sha256 4b0e28483f64 in part 1, 000000000000 in part 2" |
| one coverage cell of part 2's log off by one | FAILED: "qs coverage differs from the rows'" |
| `GAMES=999` | FAILED: "1000, not 999" |
| no `exit 0` in part 2's log | FAILED before anything moved |
| part 2's pv a row short | FAILED by `check_pool()`: "holds 50193 rows, its log line 50194" |
| the same, pv restored, re-run | DONE |

#### Dry run on the real part 1 (read only, 2026-10-20)

`nice -n 5 ionice -c3 python3 -I adocs/data/S082_pool_merge.py part1
.tuning/s082/pool.tsv .tuning/s082/generate.log`, beside the top-up: 68.6 s of
CPU, 2 m 43 s wall, 100 MB resident, exit 0. The four files' sizes and
modification times are what they were (437, 766455808, 810041344 and
817041408 bytes; 01:36:14 and 12:58:47 on 2026-10-16).

- **Files**: root 10394836 rows in 122749 games, qs 10392438 in 122749, pv
  10125441 in 122739; NUL tails 1691, 1360 and 4054 bytes; each ends on game
  122926, 95 rows. Every structural check clean.
- **Cut**: the 122748 games before game 122926 are kept -- DEC-296's count,
  now computed -- and game 122926 is dropped with the tails, one game and 95
  rows from each file. Kept: root 10394741 rows (bytes 0 to 817031797), qs
  10392343 (to 810032146), pv 10125346 in 122738 games (to 766444212).
- **Games without a row**: at least 3.
- **Offset**: 1000000, above the largest id, 123601.

| leaf | rows | a game | differs from the root | plies on average | refused, kind unknown |
|---|---|---|---|---|---|
| root | 10394741 | 84.68 | 0.00 % | 0.000 | 0 |
| qs | 10392343 | 84.66 | 26.61 % | 0.506 | 2398 (0.0231 % of 10394741 passed) |
| pv | 10125346 | 82.49 | 100.00 % | 13.861 | 269395 (2.5916 %) |

Against the 600-game smoke on the same binary (85.77, 85.74 and 83.66 rows a
game; qs 26.18 % and 0.498 plies, 0.029 % refused; pv 13.860 plies, 2.458 %
refused) and printed beside part 2's own figures by the merge. Recomputed
coverage, phase band by |score| band (<50, <150, <300, <600, rest):

| phase | root | qs | pv |
|---|---|---|---|
| 0-4 | 155479 179571 245561 291996 262962 | 165339 197161 265593 312000 276625 | 176056 344266 420988 455582 361707 |
| 5-9 | 260097 410822 498776 645761 416771 | 272930 432790 519348 664591 419620 | 317583 617702 667063 762002 412463 |
| 10-14 | 310297 464872 406345 314508 112210 | 327869 488808 416909 312805 108057 | 384915 611791 441516 278147 79972 |
| 15-19 | 224973 354247 323798 263463 74988 | 230230 361880 326710 258975 67975 | 282975 445006 334738 205089 29271 |
| 20-24 | 1308875 1678408 840008 326852 23101 | 1261182 1607275 785909 294160 17602 | 853421 1063049 442896 134138 3010 |

These are a reading of what part 1 holds and decide nothing (DEC-275).

#### For the coordinator, after `S082-TOPUP-DONE`

With this diff in main's tree (committed or applied; the stage executes no
engine, so the tree's stamp does not name the pool):

```
cd /home/max/ws/chesso &&
OUT=/home/max/ws/chesso/.tuning/s082 nohup adocs/data/S082_recipe.sh merge \
    > /home/max/ws/chesso/.tuning/s082/merge.console 2>&1 &
```

- **Markers**: `S082-MERGE-DONE` or `S082-MERGE-FAILED`, the last line of
  `merge.console`, the reasons above a FAILED; poll the whole file with the
  pid and a 30 min ceiling (WATCHERS).
- **Time**: about five minutes on an idle machine -- part 1 parsed once and
  its tails re-read, part 2 parsed, about 2.9 GB written, read back and parsed,
  the originals hashed twice and the merged files once.
- **Disk**: about 2.9 GB more, against 19 GB free on 2026-10-20 before the
  top-up's own 0.5 GB; `compare`'s split and samples need about 4 GB after.
- **Then** `adocs/data/S082_recipe.sh verify` (DONE by the merged lines), and
  `compare` as planned. Part 1's originals stay in `$OUT/part1/` with their
  sha256 sidecar, part 2 in `$OUT/topup/`; both are archived with the pool
  (DEC-235).

#### 2026-10-21: the merge's fast check

A read-only fast check over the phase 3 diff found nothing that could lose,
truncate or corrupt either part, and no edit to the pre-registered block. It
re-ran the test kit (28 of 28) and the merger's read-only `check` on the real
part 2 (every figure equal to `topup.log`, coverage included). Four minor
items:

1. `merge()` printed DONE without a `sync`; one is added before the stage
   returns, since this stage exists because of a lost page cache.
2. Only the `datagen:` settings line is compared between the two logs, so a
   flag datagen does not echo would go unchecked. On the real logs the two
   `command` lines are identical apart from `--out`, `--games` and `--seed`;
   not changed.
3. `compare`'s reading prints an empty refusal-share block on the merged pool,
   as this section says above: expected, not a failure.
4. The split sees 149998 games with a root row (at least two of part 2's
   games wrote none, and part 1's count is of games with a row), so 12000 are
   held out and 137998 trained on, against the pre-registration's 138000.

### 2026-10-21: the merge, `compare` and the reading (the coordinator)

**Merge** at `f93a3b2`, finished 10:59: `S082-MERGE-DONE`. Part 1 kept 122748
games, part 2's ids offset by 1000000; merged root 12708457 rows, qs
12705516, pv 12379744, each over 150000 games
(`adocs/data/S082_pool.log`). `verify` DONE.

**`compare`** at `f93a3b2`, 11:00-11:33, 34 min against the 1 h 15 m to
1 h 30 m estimated: `S082-COMPARE-DONE` (`adocs/data/S082_compare.log`).

**The fixed validation set** (seed 20261013, 12000 games of the 149998 with a
root row; game-list sha256 `88f338af...`):

| view | rows | games | sha256 |
|---|---|---|---|
| root | 1015607 | 12000 | `58bcbb315fd82bd332372d0b2f1df7d7370693568bd27fa9ed6f17b8e74652d6` |
| qs | 1015332 | 12000 | `2b508067fb64693bb4b278a2455fb9d101eaa742975a4b53f4ff56e6fa16981e` |
| pv | 988086 | 11999 | `24c4f671131c4a6f3f945ef0f5bb0d8cf85969b41d2e846e745539ef2d53ffdd` |

It sits at `.tuning/s082/pool.tsv.<view>.validation` and is archived with the
fit logs and the run's consoles as
`chesso_S082_validation_and_logs_2026-10-21.tar.xz` (37.7 MB, `.sha256`
checked), DEC-235. Every later fit and screen reads it from there.

**The reading**, by the pre-registered rule, mechanically: the control's L
0.073204 (five seeds, s 0.000094, m 0.000330); the best cell qs at 8 rows a
game, L 0.071904; within m of it, qs 8 and qs 46 (0.072154); rule 4 takes
the most rows a game. **Outcome (a): leaf qs, 46 rows a game**, L 0.072154,
0.001050 under the control (3.2 m). S083 generates at that leaf and density.

Beside the reading, deciding nothing (DEC-275):

- On the root view the order reverses: every root-leaf fit reads lower
  (0.078905 to 0.079677) than every qs or pv fit (0.079872 to 0.079968). A
  model fitted to resolved leaves predicts the unresolved roots worse, as the
  views measure different positions; the primary view is qs by the
  pre-registration.
- K fitted near 0.72-0.73 on qs and pv samples against 0.63-0.65 on root.
- Fewer rows a game from more games fits better on every leaf (8 beats 46
  beats all), so S083's per-game cost is the trade the rule names.
- The refusal-share block is empty on the merged pool, as the merge section
  says: part 2's datagen lines are quoted, and part 1's closing lines were
  lost. Part 2's own: qs 0.0235 % refused, all terminal; pv 2.5638 %, all
  terminal; none truncated, aborted or uncertified.
- The split holds 137998 games to train on, not the pre-registration's
  138000: part 1 counts games with a row, and two of part 2's wrote none.
