id:         S082
goal:       the corpus labels a resolved position rather than the root -- the quiescence leaf, or the leaf reached by playing out a deep search's whole principal variation -- and samples few positions per game rather than many
accepts:    `tools/datagen` records the position at the leaf quiescence resolves to, with the game result unchanged as the label, and states in its own output how often the leaf differs from the root; the tactical-move filter clause is retired rather than flagged, since its whole justification was that the root might not be quiet; a corpus is regenerated, refitted, and **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is; the leaf is verified to be quiet by construction -- a test asserts that the recorded position has no capture the quiescence search would still make, and fails if the walk is truncated by the depth bound instead
touches:    tools/datagen.cpp, src/search.cpp or src/search.hpp for the leaf walk, tests/, .tuning/
excludes:   the corpus size and the node budget, which are S083's; dedupe, which is S076's; the label blend, which is S075's; any change to what quiescence itself does
decisions:  DEC-055, DEC-041
closes:
blocks:
paused_by:
done:

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
