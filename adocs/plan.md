# Plan

The strongest open-source chess engine in the world, in C++20, bitboard based,
built on the `achesso` branch to find out what AI-driven development can produce
(DEC-013). See `specs.md` for what it must do and what it is today.

**This whole list is phase one:** reach the level the published literature
already describes, by reading documented technique and implementing it here —
never by copying it (DEC-016, DEC-014). Phase two is experimentation and has no
steps yet, and should not get any until the engine is strong enough for an
experiment to mean something. S029 was the one step where the agent stops short
of the run itself — it prepares the data and the training program, and the owner
runs the network training (DEC-015 as amended by DEC-041 — fits, measurements
and evaluation tuning are the agent's to run). S029 is parked at DEC-054, so
nothing now in the pending order stops short of its own run; the boundary stands
for whenever it resumes.

**Phase one has a number: at least 3000 on the CCRL Blitz scale, without a
network.** S088 measured chesso at **2559, 95 % +/-25, soft**. DEC-071 is the
owner's decision that 3000 is the next mark and that DEC-054 stands while it is
pursued: no NNUE. The distance is about 440 Elo and this list is what it costs.

**The order was rewritten on 2026-08-19 by an adversarial review of this file
against the published record and against the code.** Six decisions came out of
it — DEC-081 to DEC-086 — and they are what the order below now expresses.
Everything above the review's own findings still stands: S001 to S018 are the
record of what each early change cost and bought, and DEC-019 is still why a
published figure decides what to try and never what to conclude.

## What the review found, and why the order changed

**Chesso is tree-shape-limited, not evaluation-limited. DEC-081.** The plan's
order came from DEC-033, which measured that sixteen times the *nodes* removes
only 24.1 % of the error, 29.8 % after S028's fit. That measurement stands and
is not withdrawn. What it does not say is how many *plies* sixteen times the
nodes buys, and here it is about four. Measured 2026-08-19 after
`e4 e5 Nf3 Nc6 Bb5 a6` at `go movetime 250`: chesso reaches **depth 12 on
1448572 nodes**, Stockfish **depth 15 to 16 on 158837 nodes**. Nine times the
nodes, four plies shallower. The public record agrees from the other side:
**Leorik 2.5 is 2917 with less evaluation than chesso ships** — no king safety,
no threats, no outposts, no bishop pair, knight mobility commented out — and its
largest jump, +436, was four search features and no evaluation change. So the
search block leads and the evaluation block follows it. The evaluation was the
constraint in 2026-08 and S028 (+188.74) and S065 (+21.10) are that conclusion
already cashed.

**Free speed was sitting on the floor, and S104 has picked it up.** The binary
shipped at `20d058a` contained **zero `popcnt` instructions**: `CMakeLists.txt`
added no architecture flag, so every `count_bits` in mobility, king safety,
`game_phase` and move generation was a software popcount. **Done 2026-08-19:
+18.22 % on the shipping profile-guided `bmi2` target, 95 % CI +16.19 to +20.28,
node-identical**, so DEC-083 took a timing and not a match. The architecture flag
is +12.62 % of it and profile-guided optimisation the remaining +4.98 %. It was
not "one line of CMake" as this paragraph predicted: three targets, a PGO driver,
and a bug in the first design that made the profile silently unreachable.

**The measurement instrument was set three ways that spend the budget.** The
SPRT runs at `10+0.2` where the engines this plan reads from test at 8+0.08, at
`Hash=16` where the rating list runs 128 to 256, and on a balanced book. The
hash mismatch is not cosmetic: measured here, 16 MB against 512 MB is 36 %
fewer nodes and **21 % lower nps**, so every verdict has been taken in a table
regime the list never runs. S105 fixes all three and DEC-083 adds the rule that
a behaviour-neutral change is accepted on an interleaved timing, never on a
match — below about 0.7 % a speed-up is invisible to an SPRT anyway. Together,
roughly three times the verdicts per night, against a plan whose own budget line
says measurement capacity is what binds.

**The next step as it stood would have measured zero.** S092, the improving
flag, was first in the pending order. It is a modifier of reverse futility, late
move pruning, futility, null move and the reduction; chesso has two of those
five. And it needs a static evaluation at every node, which the main search
computes only at reverse-futility nodes. S092 is retired into **S108**, which
supplies the input, and its consumers arrive at **S109**.

**The pruning rules only pay together. DEC-082.** Stockfish's own removal test
measures move-count pruning at **~0 alone** and the block it belongs to at
**~204**: the rules prune overlapping sets, so each measured without the others
returns nothing. The old plan had them as four steps at four positions — S090,
S026, part of S091, and history pruning with no step at all. They are now one
step, **S109**, measured once, with per-part attribution forfeited on the record
rather than discovered later. S090 and S026 are retired into it and their ids
are not reused. This is a redrawn step boundary, not a suspended rule: one step
still owes one test.

**The literature is read for form, never for constants. DEC-084.** A number from
an open paper, wiki page or article may seed a fit. Nothing from another
engine's source or tables is used, not even as a seed. And **no constant ships
unfitted** in either case — a seed is where our own tuner or SPSA begins, and a
fit that lands on the published value is a confirmation and a good outcome.
Every step below that takes a formula from the record ends in a fit for this
reason.

**Two items were carried on a false premise. DEC-085.** Checked against the CCRL
Blitz testing conditions rather than assumed: the main list is **single CPU**,
the engine's own book must be **disabled**, and book and position learning must
be **off** — an engine that cannot disable its book is not listed. So **S086 is
retired**, it was a rule violation with no upside, and threading is not a
phase-one step and gets no id. The concurrency decisions DEC-048, DEC-050 and
DEC-073 are untouched; they are about running matches. Tablebases are the one
thing on that list the rules *do* allow, which is why S129 exists and sits last.
**S031 is also retired** — under 1 % by its own file, below every instrument
here.

**Ten steps whose whole content was a correction to another step are folded into
the steps they correct. DEC-086.** S056, S057, S058, S059, S060, S061, S063,
S079, S080 and S081 were the plan_review audits' findings about other steps'
acceptance criteria. Each correction is now in the file that owns it, each
corrected file names the retired id, and the ids are not reused. Folding a
finding into the step it corrects answers it; **it does not close it**, and the
parked 2026-08-16 re-run is still owed.

## Three things the review measured that no step existed for

**The lazy-evaluation clamp is the ceiling on the evaluation.**
`evaluate_expensive()` clamps mobility **plus** king safety to
`+/-LAZY_EVAL_MARGIN` — 150 centipawns for the two together — and `evaluate()`
is `evaluate_cheap() + evaluate_expensive()`, so the clamp is on the real score
and not only on the shortcut's. A mating attack is worth four to six hundred
centipawns and this evaluation cannot say more than one hundred and fifty about
the king and the mobility combined. Measured cost of removing it:
`LazyEvalMargin` 0 / 150 / 2000 gives **6.55 / 5.46 / 4.82 Mnps** — full
evaluation everywhere costs 11.7 %, against the +18.22 % S104 has now measured.
Those three figures predate S104 and are on the unflagged binary, so the ratio
between them is what carries and not the absolute Mnps. So
S039 moved from position 89 to just ahead of the king-safety rebuild, and it is
no longer a micro-tune: it is the architectural prerequisite, with S120's
evaluation cache buying back what it spends.

**Mobility and king safety are in their weakest published forms.** Mobility is
one linear weight per piece over a raw count with no exclusions, fitted to
`{-1, 5, 8, 3}` and `{0, 5, 0, -6}` — knight middlegame **-1**, rook endgame
**0**, queen endgame **-6**. Those are what a correct fit returns when the model
cannot hold the shape. King safety is linear in attacker counts. Both are
rebuilt, S121 and S122, and neither had a step.

**Five terms ship at exactly zero and that is a pattern, not five results.**
Bishop pair, rook on an open file, rook on a half-open file, rook on the
seventh, and tempo. Published fits of the same terms measure +16.7, +4.2, +9.2
and +12.99. `passed_pawn_mg` is the fourth data point: `{0, -6, -4, 19, 59,
-17}`, seventh rank fitted below sixth and negative. Whatever causes it is also
acting on the other 817 constants, so **S100 moved from position 84 to the front
and became a diagnostic**.

## The order, in four blocks

**Block 0, instruments and free wins, S104 to S085.** None of it is a feature.
S104 and S105 are the build and the harness and they multiply everything below;
S106 is a correctness sweep, because a bound-sign inversion in quiescence has
been measured at 64 Elo elsewhere and S094 already found one defect of exactly
that shape here; S107 is one line that stops the ordering tables refusing to
remember checking quiet moves; S100 is the zero-weight diagnostic; S084 and S085
are the SPSA driver and a first run over the twenty parameters that exist today.
S085 moved forward because those twenty are demonstrably mis-set —
`MaxQsearchDepth` is 8, the bound binds, and raising it to 16 moves both the
node count and the score on two of three positions in opposite directions.

**Block 1, the search, S093 to S025.** Ordered so each step's consumers exist
before it. History first (S093 — malus, gravity, butterfly indexing; +37.49
reported and the largest single history patch on record), then the static
evaluation and improving that the pruning needs (S108), then continuation
history (S024), then the block itself (S109), then SEE pruning in the main
search (S091 — the headline change of the +151 version), then the reduction
rebuild (S098), then correction history in three steps (S099, S110, S111), then
quiescence futility and the delta-pruning decision it re-opens (S112, S022),
then the extensions (S097, S096), then capture history (S023), then the small
tail (S095, S113, S114, S115, S116) and the bad-capture retry that S023 and S024
were the precondition for (S025).

**Block 2, speed, S020 to S030.** All of it either behaviour-neutral or nearly
so, and cheap under DEC-083 because most of it owes a timing rather than a
match: the duplicated per-node `is_check` (S020), the merged taper (S055), the
packed middlegame/endgame score (S117 — +25.41 reported, the largest evaluation
speed number surveyed), the pawn hash (S118), the evaluation cache (S120), the
table layout that the 21 % measurement above indicts (S119), and the movegen
work that was always correctly last (S042, S032, S030).

**Block 3, the corpus and the evaluation, S082 to S126.** Corpus first, because
fitting a term on a corpus about to be replaced is the trap
`src/evaluation.cpp` already records against S027. Then the clamp decision
(S039), then the terms in the order the published record ranks them for an
engine that has none of them: mobility curves, king safety, passed pawns,
endgame scaling, threats, pawn structure, outposts and space. Then one full
refit (S126), because every weight fitted before S109 was fitted against a tree
that no longer exists.

**Block 4, close, S127 to S129.** The full SPSA run over everything the search
block added, the rated run at a control near the list's own — DEC-077 named the
time control as the leading suspect for the 121.8 Elo anchor spread and said
attacking it was nobody's step, and it is S128 — and Syzygy last and optional,
because the no-copy rule prices 13 Elo at a from-scratch implementation of a
compressed table format.

## What this costs

About thirty steps alter play and owe a verdict each. At the S105 settings with
`--fast` bounds that is one to one and a half hours apiece, and with failures
and retunes **roughly 90 to 140 machine-hours** — against the 150 to 250 the old
plan priced for less ground. Six steps stop owing a match at all under DEC-083.

**The Elo arithmetic is a range and not a forecast, and DEC-019 is why.**
Discounting self-play to list Elo at the ratio the published per-release records
support, and taking a fifth off for interaction: search +180 to +280, evaluation
+90 to +160, speed +40 to +90, tuning +50 to +90. The midpoint clears 3000. Two
things are not promised by that: the 2559 anchor is soft by 121.8 Elo of
internal disagreement until S128 runs, and this project has taken three
published figures at face value and measured 0, 0 and *slower*.

## How this file works

Order lives here and nowhere else. Step detail lives in the step files under
`plan_todo/`, `plan_current/`, and `plan_done/`. Ids are allocated in creation
order and never renumbered or reused — including for the retired S019, S026,
S031, S056 to S061, S063, S079 to S081, S086, S090 and S092 — so reordering is a
one-line edit to this list.

Order is read from the list entries below, the lines starting `1.`, `-`, or `*`.
An id named in a sentence anywhere else in this file is prose: it does not change
the order, and it is not checked. Every pending step file must appear as a list
entry and every list entry must name an existing step file; the workflow checker
enforces the correspondence and adds an entry when a step is created.

**It also prunes completed entries, and it keeps the last five in *list order*,
not the five most recently completed.** The checker is not in this tree. It is
the moltke plugin, read here at **version 0.12.0**, the one
`installed_plugins.json` names, where `prune_plan()` collects the completed
entries by position in the file and drops all but the final `PLAN_DONE_KEPT` of
them, `PLAN_DONE_KEPT = 5`. Follow it by those two symbols and not by a line
number: they sat at `bin/moltke.py:1698-1700` and `:1681` in 0.11.0 and at
`:1947` and `:1928` in 0.12.0, and they move again under any other version. More
than one copy is on this machine — `cache/moltke/moltke/0.11.0` is still on disk
beside 0.12.0, and `~/ws/moltke` is a working tree — and the one the hooks run
is the path `installed_plugins.json` names. A step whose entry sits low in the
list therefore outlives completions that came after it, and "last done" can name
an older step than the newest completion.

A pruned entry takes ledger rows with it, and the rule there is narrower than it
looks: a `testing.md` row leaves only when **every** `S<nnn>` it names was
dropped in that same pass. Naming an invariant or a decision does not protect a
row — the checker matches step ids and nothing else — and 17 rows naming an
`INV-n` or a `DEC-n` have been pruned. A row naming no step id at all is never
pruned, which is why the `INV-1` to `INV-6` rows at the head of the ledger stay.
`plan_done/` and git history keep everything pruned.

<!-- 1. S001  short goal -->
2. S089  a time budget that scales with best-move stability and with a falling score, instead of remaining over a fixed movestogo plus half the increment
3. S094  quiescence probes and stores the transposition table, and an entry carries the static evaluation it was scored with
4. S103  reverse futility reads the static evaluation already in the table entry instead of recomputing it
5. S053  testing.md's header states the checker's retention: a pruned plan entry takes its ledger rows with it
6. S104  the release build targets the machine's instruction set and is profile-guided, so count_bits stops being a software popcount
7. S105  fastchess.sh plays in the rating list's regime -- 8+0.08, Hash 128, an unbalanced book -- and a behaviour-neutral change is accepted on an interleaved timing instead of a match
8. S106  the transposition bound signs and the mate-score round trip are checked against a red test in both searches, not assumed
9. S107  a quiet move that gives check becomes eligible for the killer, history and countermove tables it is excluded from today
10. S100  find out why five evaluation terms fit to exactly zero -- feature extraction, corpus composition or a real result -- before any further weight is fitted beside them
11. S084  an SPSA driver over the exposed search parameters, verified against an objective whose optimum is known
12. S085  the first SPSA run, over the twenty search parameters that exist today, and an independent SPRT of what it returns
13. S093  history gets a malus for the moves that were tried and failed, a gravity update that ages it by construction, butterfly indexing, and survives across go within one game
14. S108  every node that is not in check computes its static evaluation once, stores it in its table entry and reads it back, so improving and the pruning margins have an input
15. S024  history indexed by the move played n plies ago and the current move
16. S109  late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
17. S091  skip captures the exchange evaluation says lose material, in the main search rather than in quiescence alone, and reduce a negative-SEE move by an extra ply
18. S098  the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
19. S099  a static evaluation correction learned from the difference between the static score and what the search returned, keyed on the pawn structure
20. S110  a second correction table keyed on the non-pawn structure, split by colour
21. S111  correction tables indexed by the move played two and four plies ago
22. S112  quiescence skips a capture whose best case cannot reach alpha, per move, before the exchange evaluation is consulted
23. S022  decide between delta pruning and the per-move futility S112 adds, by measurement -- deleting delta pruning is a valid recorded outcome
24. S097  extend the one move a verification search says is singular, and take the multicut the same search offers
25. S096  extend a node that gives check, so a forcing line is not cut at the horizon
26. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
27. S095  reduce a node whose table entry carries no move instead of searching it at full depth
28. S113  a shallow verification search over good captures prunes a node whose score is already far above beta
29. S114  the null move reduction scales with how far the static score is above beta, and a verification search guards the deep case
30. S115  the window's half-width comes from the score's own volatility, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply
31. S116  a node whose static score is hopelessly below alpha drops straight to quiescence, at depth one only
32. S025  retry searching losing captures after the quiets, now that capture history exists
33. S020  compute the in-check state once per node instead of once per call site
34. S055  taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
35. S117  the middlegame and endgame halves of every evaluation term travel in one integer instead of two
36. S118  the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
37. S120  a small cache of full evaluations by position key, so the score behind the lazy shortcut can be paid for once
38. S119  the table becomes cache-line clusters with an aged replacement, a prefetch issued when the key is known, and huge pages
39. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
40. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
41. S030  move_t drops the moving piece and becomes 16 bits
42. S082  the corpus labels a resolved position rather than the root -- the quiescence leaf, or the leaf reached by playing out a deep search's whole principal variation -- and samples few positions per game rather than many
43. S083  a corpus past 50 M positions, and a measured answer on what nodes per move buys against volume
44. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
45. S121  mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on
46. S122  king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks and weak squares, and it is no longer clamped
47. S123  passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
48. S124  the endgame half of the score is scaled toward a draw by what is actually on the board
49. S101  evaluation terms for a piece attacked by a lesser piece, fitted like every other constant
50. S125  backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
51. S102  outpost and space terms in the evaluation, fitted like every other constant
52. S126  every constant in the evaluation is refitted once the search that consumes them has stopped moving
53. S127  an SPSA run over the whole search parameter set as it stands after the search block, and an independent SPRT of what it returns
54. S128  the gauntlet is replayed at a time control near the rating list's own, to test whether the anchor spread is scale compression from 10+0.2
55. S129  three, four and five man tablebase probing, written from the format description
56. S029  **parked, DEC-054** — a perspective network evaluation trained on chesso's own self-play
