id:         S232
goal:       the seed sections of S099, S110 and S111 are rewritten in DEC-134's three forms, so no constant quoted from another engine's commit message or pull request remains in a pending correction-history step
accepts:    each of the three files' constants-and-seeds section names every seed as one of (a) a value from a publication about the technique with its URL, (b) a derivation over chesso's own data or scale that the owning step runs at its start, or (c) the range midpoint or off value stated as such; the Stockfish commit-prose figures in S099 ("entry / 32", "~32 internal units", "clamp near 1024") leave or move to an anti-seed paragraph that names them as another engine's numbers; S111's pull-request source is cited as record only, never as a seed; S110's twelve unsourced figures are sourced or struck; `tools/plan_prose_check.py --citations` and `--touches` green over the three files; no code, no order change
touches:    adocs/plan_todo/S099_correction_history.md, adocs/plan_todo/S110_correction_history_non_pawn.md, adocs/plan_todo/S111_correction_history_continuation.md
excludes:   any change to the three steps' goals, gating or placement (DEC-133, DEC-176 (c)); any code; running S099 -- that is the probe night DEC-133 permits, taken once this step is done
decisions:  DEC-134, DEC-105, DEC-222
closes:     2026-09-19_study_review-F06
blocks:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-19
paused_by:
done:       2026-09-20 03:30 -- every seed in the constants-and-seeds sections of S099, S110 and S111 is named as (a), (b) or (c) in DEC-134's forms, eighteen in the step file's table: the applied maximum a (c) midpoint of 0 to one pawn in chesso's scale (`PAWN` 94 at HEAD), the weight cap a (b) over `CONT_HIST_REF_DEPTH`, the entry clamp arithmetic over those two, the grain the geometric midpoint of the power-of-two range 64 to 1024, the table size a census the owning step runs with a (c) fallback, the blend weights a (c) percentage of the pawn table's share with 100 the midpoint; the Stockfish commit-prose figures ("entry / 32", "~32 internal units", "clamp near 1024", "a third of a pawn") left the seed lines for anti-seed paragraphs that name them as another engine's; S111's pull request is cited as record only; S110's twelve unsourced figures are struck with the date and reason after a fourth search found no publication carrying them; one seed beyond the brief caught on the way -- the update-weight formula S099 credited to the wiki is another engine's code reproduced there (DEC-105's PeSTO case) and is now a (b); `tools/plan_prose_check.py --citations` and `--touches` 0 flagged over the tree; no code, no goal, gating or order change (DEC-133, DEC-176 (c)). Fast check: trivial fixes (two labels, the grain's form, the cap's derivation made self-contained, two step-file claims), all applied; one reviewer finding withdrawn on evidence, `RfpMaxDepth`'s comment still records the median depth 11. Gate at completion: the 40 of 40 both-builds run of `.tuning/coord/gate_2026-09-20_s233_s232.log` covered this tree. Closes `2026-09-19_study_review-F06`; S099 may now run as DEC-133's probe.
## Why this exists

DEC-134 (2026-09-04) ruled that a constant quoted in another engine's commit
message is that engine's constant and had S180 reseed seven step files. S099
was not among them and still reads, in its section 4, an applied correction
of "`entry / 32` with the max adjustment ~32 internal units (implying an entry
clamp near 1024 in SF's scale) -- SF commit message prose"; its hedge, "take the
shape, not the number", does not remove the numbers from the page an
implementer reads. S111 names a Stockfish pull request as its source and S110
carries twelve figures the 2026-09-04 literature check could not source (row
A22). The 2026-09-19 study review found this while checking the study's first
recommendation (`adocs/audit/2026-09-19_study_review.md`, F06), and the owner
chose to run S099 as DEC-133's probe on the next idle night -- which cannot
happen while its seeds are another engine's.

## Shape

A document step in S180's pattern: each seed rewritten in one of the three
forms with the form named, anti-seeds kept as record in a paragraph that says
so, and the citation checker run over the three files. Nothing in `src/`
moves, so no `Bench:` line is owed; the completing commit says `No functional
change` only if it touches `src/`, which it does not.

## Cost

An hour of agent time, no machine. It gates the probe night and nothing else.

## What was found first: the weight formula is not the wiki's own

The brief asked for one attribution to be verified, and it did not hold.
S099's section 1 and its section 4 both attributed the update weight
`w = min(depth*depth + 2*depth + 1, 128)` to CPW, as the wiki's own
derivation, which under DEC-105 would have made it a legal **(a)** seed. The
page was fetched three times on 2026-09-19 -- the rendered article, then the
section structure, then the Types table -- and every read agrees: the formula
sits inside a
code block the page introduces with "A version as seen in Alexandria is shown
below". It is another engine's source republished on a wiki -- DEC-105's
PeSTO case exactly, and the same class as the `66 * cv / 512` snippet the
file already refused two bullets further down. So the cap **128** joins the
Stockfish figures in the anti-seeds and section 1 carries a corrected
attribution; only the *shape* (an EMA per entry, the weight rising with depth
and capped) is kept, which is a form and not a constant.

Two further things the same fetches settled. The page states **no value** for
the grain, the scale, the clamp or the table size, which is what S099 already
said -- so **(a)** is genuinely unavailable for those and the three files now
say so instead of leaving it implied. And the page's own prose does carry the
update gating list and the time-control sentence S110's argument leans on,
both verbatim, so those citations stand and S110 now quotes the second one
rather than paraphrasing it.

## Every seed touched

"Today" is the value at HEAD `8ceb1d6`; "origin" is what the file itself
claimed before this step; "replacement" is the DEC-105 form and its value or
procedure. Rows marked (F06) are the ones the 2026-09-19 study review named.

| file | constant | today | origin the file stated | replacement |
|---|---|---|---|---|
| S099 (F06) | update weight cap | `min(depth*depth + 2*depth + 1, 128)` | "CPW" | **(b)** **144**. The shape stays `min((depth + 1)^2, cap)`; the cap is `(CONT_HIST_REF_DEPTH + 1)^2`, that constant being chesso's own recorded median remaining depth **11** in `src/data_structures.hpp` -- a code constant, not a comment, so the derivation is self-contained and follows the constant. A re-measurement at the 8+0.08 verdict control is written beside it as the procedure this step may run. The 128 is Alexandria's, to the anti-seeds |
| S099 (F06) | per-update cap | "half the entry clamp" | SF commit message prose | **struck from the seeds.** It is the history-gravity form's device and chesso takes the EMA form, where the weight already bounds one update's movement. To the anti-seeds as record |
| S099 (F06) | applied correction and its maximum | `entry / 32`, "max adjustment ~32 internal units" | SF commit message prose | **(c)** maximum applied correction **47**, the midpoint of a range declared by purpose: 0 (off) to one pawn, past which a correction rewrites the material balance rather than correcting the score |
| S099 (F06) | entry clamp | "near 1024 in SF's scale"; elsewhere "sized so the applied maximum lands in the 32-64 cp class" | SF commit message prose; the cp class is SF's internal units read as centipawns | **(b)** **12032** = applied maximum x grain = 47 x 256, arithmetic over the two seeds above, written out so it re-derives when either end moves |
| S099 (F06) | the unit hedge | "max correction of roughly a third of a pawn ... the shape, not the number" | SF commit message prose | **deleted from the seed line.** A units paragraph replaces it: the correction is added to `evaluate()`'s output, so its magnitude is `PAWN` in `src/eval_tables.hpp`, **94** at this step's HEAD, and not any engine's internal unit (DEC-134's units clause) |
| S099 | EMA scale | 256, "own choice (>= max weight)" | own, but hinged on the Alexandria cap | **(b)** **256**, the smallest power of two at or above the (b) cap 144. Not swept apart from the cap: the pair sets one quantity, the maximum EMA rate |
| S099 | grain | 256, "own choice" | own, unstated | **(c)** **256**, the midpoint of a range declared by purpose and stated as such: 64 to 1024 in powers of two, and on a power-of-two axis the midpoint is the geometric one, `sqrt(64 * 1024)` = 256. Nothing derives it -- it is the fixed-point resolution and carries no Elo once the applied maximum is fixed, which the bullet now says instead of justifying 256 after the fact |
| S099 | table entry count | 16384 per side to move, "own choice" | own, unstated | **(b)** a census this step runs at its start -- distinct pawn keys over `go depth 12` on the 300 positions of the S021 stratified pick, next power of two above the 95th percentile. Fallback **(c)** **32768**: midpoint exponent 15 of a 2^10 to 2^20 range declared by purpose |
| S099 | `66 * cv / 512` | already refused in a seed bullet | SF source republished on the wiki | refusal kept and moved into the anti-seed paragraph, where a refused number belongs |
| S110 (F06) | the twelve Elo figures | pawn +11.29 / +12.40 and +4.87 / +11.70, non-pawn +6.98 / +12.28 and +2.80 / +6.84, continuation +2.58 / +5.46 and +2.75 / +5.46 | "reported", no source; row A22 of the 2026-09-04 literature check | **struck** with `~~...~~`, dated, reason under it, after a fourth search on 2026-09-19 found no publication carrying them. (S180 deleted its class outright and recorded each removal in its stamp; the strike keeps the history readable in a file nobody has started.) The search history is kept as the record of what was looked for |
| S110 (F06) | "the long-control figure is consistently about twice the short one", and the "under-measure by half" that follows from it | a factor of two | the twelve struck figures | **struck.** The direction survives on CPW's own sentence, now quoted verbatim and re-fetched; the file says in as many words that the size of the under-measurement has no source |
| S110 | weight cap, EMA scale, applied maximum, grain, entry clamp | no constants section existed | -- | **(b)** the values S099's own fit returns, read at this step's HEAD -- chesso's own SPSA output, the same treatment S180 gave `NULL_MOVE_BASE` |
| S110 | entry count for the non-pawn key | no constants section existed | -- | **(b)** S099's census re-run for this key; fallback **(c)**, S099's exponent range and midpoint |
| S110 | weight of this table against S099's | no constants section existed | -- | **(c)** **100**, the midpoint of 0 to 200. The constant is named and its unit is a percentage share of the pawn table's applied weight: 0 is this table off -- the state S099 measured -- 200 is twice the pawn table's share, 100 is equal weight. A blend weight is a ratio, so the material-scale bound the first draft used was the wrong unit and is dropped |
| S111 (F06) | Stockfish pull request #5617 | "where continuation correction history landed" | the pull request | kept, and the sentence now says it is **a record of where the technique landed and nothing more**; no number out of it appears anywhere in the file and none seeds anything |
| S111 | the two index offsets, two and four plies | in the `goal:` | the technique's form | **not a seed.** It is the step's own form, fixed by its `goal:`, not a tunable with a starting value -- said explicitly so nobody looks for a source |
| S111 | blend weights across the tables | `accepts:` says "fitted at S127 and not taken from anywhere (DEC-084)" | own | **(c)** **100**, the midpoint of 0 to 200, one named constant per table, the same percentage-of-the-pawn-table unit as S110's. The `accepts:` is unchanged; the new section states the form it was missing |
| S111 | weight cap, scale, applied maximum, grain, clamp, entry count | no constants section existed | -- | **(b)** the values S099's and S110's own fits return, read at this step's HEAD; entry count by S099's census re-run for the continuation key |

## The anti-seed paragraphs, and where they are

Three, one per file, each headed **Anti-seeds -- records, not seeds** in the
wording S095, S098, S109 and S114 already use, so the class is greppable.

- **S099**, at the end of its section 4. Carries every number this step took
  out of a seed position and names each as another engine's: Alexandria's cap
  128 and its symbol set from the wiki's reproduced code; Stockfish's
  `66 * cv / 512` snippet; and that engine's commit-message prose -- `entry /
  32`, about 32 internal units, an entry clamp near 1024, the per-update cap
  at half the table maximum, and "roughly a third of a pawn" as a unit
  statement in its own scale. PiChess's "+/-150cp" from the section 1 commit
  record is named there too, because it is the one other number in the file
  an implementer could mistake for a clamp. The paragraph says outright that
  none of them is a seed, a fallback, or a sanity check, and that the hedge
  the 2026-08-19 pass used is what F06 found insufficient.
- **S110**, at the end of its new constants section. The Elo records --
  Sirius's +7.43 +/- 4.78, Tcheran's four changelog lines, S099's sourced
  +11.35 -- are direction under DEC-019 and are not constants of this table
  in the first place.
- **S111**, at the end of its new constants section. Pull request #5617 and
  Tcheran's continuation line, as records; the unsourced +1.8 to +4.6 as not
  even that.

Section 1 of S099 keeps its Elo record intact, which is what the brief and
DEC-019 both want: a record says which direction is worth trying. The one
change there is the corrected attribution above, because a seed rested on it.

**One class marker, so one grep finds every reseeded file.** Each of the
three sections ends with the line S180 put at the foot of its seven,
verbatim except the date: `seeds re-derived 2026-09-19 under DEC-105
(DEC-134)`. `grep -rln 'seeds re-derived' adocs/plan_todo/` now returns
**eight** -- S095, S097, S113, S114 and S132, which are the five of S180's
seven still pending, and S099, S110 and S111. S098 and S109 carry the same
line and have since completed into `plan_done/`.

## What was fetched, and what was not

Fetched 2026-09-19 and used:

- `https://www.chessprogramming.org/Static_Evaluation_Correction_History` --
  three reads. Confirms: the EMA update block is introduced "A version as
  seen in Alexandria is shown below"; the application snippet `66 * cv / 512`
  is introduced "as introduced by Stockfish"; the page states no numeric
  value for the grain, scale, clamp or size; it states no Elo figure
  anywhere; its own prose carries the four update-gating conditions S099
  cites and the phrase "Color and Hash" S099's section 2 cites; and it
  carries the time-control sentence S110 now quotes, "Correction history has
  been shown to exhibit non-linear scaling behavior with respect to
  increasing time control, with larger gains in longer searches".

Searched and found nothing, which is the fourth such pass:

- Two open web searches for S110's twelve numerals against "correction
  history", by several of the numerals at a time. Every hit is a source this
  family already cites or an unrelated engine's own figure; no publication
  carries the twelve. They are struck.

Tried and failed, with no consequence: the wiki's `index.php?title=...
&action=raw` endpoint answers 404, so the wikitext could not be read
directly. The rendered page was read three times instead and the three reads
agree on every attribution above, so nothing here is marked `unverified`.

Not fetched, on purpose: no engine's source tree, and no pull request or
commit page. The commit and pull-request URLs already in the three files were
left exactly as they are, as record. Nothing in this step needed a number
from one.

## Verification

Both modes green over all 52 pending files, the three edited ones and this
one included:

```
adocs/plan_todo/S099_correction_history.md: 23 code citations, 0 flagged (0 line form, 0 bare, 0 document ungated)
adocs/plan_todo/S110_correction_history_non_pawn.md: 0 code citations, 0 flagged (0 line form, 0 bare, 0 document ungated)
adocs/plan_todo/S111_correction_history_continuation.md: 0 code citations, 0 flagged (0 line form, 0 bare, 0 document ungated)
adocs/plan_current/S232_reseed_correction_history.md: 0 code citations, 0 flagged (0 line form, 0 bare, 0 document ungated)
citations flagged: 0 over 52 files
touches flagged: 0 over 52 files (2 symbols read, 0 noted and ungated)
```

Then a grep over the three files for every figure the brief named. Each
survivor sits in a record or anti-seed paragraph that names it as another
engine's number, and none is in a seed position:

- S099 `entry / 32`, "32 internal units", 1024, "a third of a pawn",
  `66 * cv / 512`, 128: only in the corrected-attribution paragraph of
  section 1, in the PiChess commit record of section 1, and in the anti-seed
  paragraph of section 4. `16384` and the "32-64 cp class" are gone entirely.
- S110's twelve numerals: only inside the `~~ ~~` strike. "about twice" only
  in the sentence withdrawing it.
- S111 #5617: the record-only sentence and the anti-seed paragraph. `+1.8 to
  +4.6` is the DEC-087 claim the section refutes and is marked unverified
  where it stands -- left alone deliberately, since striking a claim a
  paragraph exists to rebut would make the paragraph unreadable.

No build, no test run, no match: this step changes documents only. No
`Bench:` line is owed and the completing commit touches nothing under `src/`.

## Proposed status.md paragraph

> **S232 done (2026-09-19).** The three correction-history step files are
> reseeded in DEC-134's forms. S099's section 4 is rewritten: the weight cap
> is **(b)** 144 = `(CONT_HIST_REF_DEPTH + 1)^2` off chesso's own recorded
> median depth, the EMA scale **(b)** 256 above it, the maximum applied
> correction **(c)** 47 -- the midpoint of 0 to one pawn in chesso's own
> scale, `PAWN` = 94 -- the grain **(c)** 256, the geometric midpoint of a
> 64-to-1024 power-of-two range, the entry clamp **(b)** 12032 from those
> two, and the table size **(b)** a census with a **(c)** fallback of
> 32768. A new finding on the way: the weight formula the file called CPW's
> is Alexandria's code republished on the wiki, verified by two fetches, so
> its 128 was never an (a) seed either; section 1 carries the corrected
> attribution. S110's twelve unsourced figures are struck after a fourth
> search found no source, and the "about twice" factor that rested on them
> goes with them -- the direction survives on CPW's own time-control
> sentence, quoted. S111 cites pull request #5617 as a record only. S110 and
> S111 gain the constants sections they never had, each blend weight a named
> percentage share of the pawn table's, range 0 to 200, seeded **(c)** at the
> midpoint 100. Three anti-seed paragraphs
> carry the withdrawn numbers as record, and all three files end with S180's
> own marker line so one grep finds the ten reseeded files. `--citations` and `--touches` green
> over all 52 pending files; no code, no order change, no `Bench:` owed. The
> probe night DEC-222 (1) gates on this is now unblocked.

## Left for the coordinator

- `2026-09-19_study_review-F06`'s `Status:` line flips from `planned` to
  `closed`.
- **Where the median depth 11 comes from, traced on the fast check's
  request. Nothing is stale and no repair is owed.** The chain is: S085
  measured it -- its own step file carries the `tc / s per wave / median
  depth / p10 depth` table and the sentence "**2+0.02** is the choice: it
  reaches median depth 11 against 5+0.05's 12" -- and `RfpMaxDepth`'s
  comment does still record it, in the sentence "median 11 at the tuning
  control". An earlier draft of this step said that comment had lost the
  number in the S213 rewrite; **that was wrong and is withdrawn**. The two
  comments pointing at it, at `FUT_MAX_LMRDEPTH` and at the
  reduction-adjustment term, are correct as written. S222 promoted the same
  number to a named constant, `CONT_HIST_REF_DEPTH` in
  `src/data_structures.hpp`, as the unit its two shares are read in, and
  that constant is what this step's weight-cap derivation now reads -- a
  symbol rather than a comment, so the seed re-derives itself if the median
  is ever re-measured. No committed script re-derives 11 on demand; the
  measurement is S085's table and the arithmetic to re-take it is written
  into S099's bullet as a procedure. One caveat worth carrying: 11 is the
  median at the **2+0.02 tuning control**, not at the 8+0.08 verdict
  control, which reaches deeper -- true of `CONT_HIST_REF_DEPTH`'s present
  uses as well, and a question for whoever next touches that constant.
