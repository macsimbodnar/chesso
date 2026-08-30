# Plan

The strongest CPU chess engine in the world — MIT-licensed, nothing
copy-pasted, every change proved by SPRT — in C++20, bitboard based, built on
the `achesso` branch to find out what AI-driven development can produce
(DEC-013, DEC-104). See `specs.md` for what it must do and what it is today.

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
network — and the scale is the 1CPU entry (DEC-089).** S088 measured chesso at
**2559, 95 % +/-25, soft**. DEC-071 is the owner's decision that 3000 is the
next mark and that DEC-054 stands while it is pursued: no NNUE. The distance is
about 440 Elo and this list is what it costs.

**The order was rewritten on 2026-08-19 by an adversarial review of this file
against the published record and against the code (DEC-081 to DEC-086), and
that review was itself reviewed the same day against per-patch SPRT records
and the live CCRL conditions (DEC-087 to DEC-089).** The second pass confirmed
the block structure and corrected nine contents. Everything above the reviews'
own findings still stands: S001 to S018 are the record of what each early
change cost and bought, and DEC-019 is still why a published figure decides
what to try and never what to conclude.

## What the first review found, and why the order changed

**Chesso is tree-shape-limited, not evaluation-limited. DEC-081.** The plan's
order came from DEC-033, which measured that sixteen times the *nodes* removes
only 24.1 % of the error, 29.8 % after S028's fit. That measurement stands and
is not withdrawn. What it does not say is how many *plies* sixteen times the
nodes buys, and here it is about four. Measured 2026-08-19 after
`e4 e5 Nf3 Nc6 Bb5 a6` at `go movetime 250`: chesso reaches **depth 12 on
1448572 nodes**, Stockfish **depth 15 to 16 on 158837 nodes**. Nine times the
nodes, four plies shallower. The public record agrees from the other side:
**Leorik 2.5 is 2917 with less evaluation than chesso ships**, and its largest
jump, +436, was four search features and no evaluation change. The second pass
added a third independent line: Ethereal's own feature-removal ledger prices
its history at -759, its LMR at -249 and its quiet-pruning family at -175,
against -32 for reverse futility and single digits for most evaluation terms.
So the search block leads and the evaluation block follows it. The evaluation
was the constraint in 2026-08 and S028 (+188.74) and S065 (+21.10) are that
conclusion already cashed.

**Free speed was sitting on the floor, and S104 has picked it up.** The binary
shipped at `20d058a` contained **zero `popcnt` instructions**: `CMakeLists.txt`
added no architecture flag, so every `count_bits` in mobility, king safety,
`game_phase` and move generation was a software popcount. **Done 2026-08-19:
+18.22 % on the shipping profile-guided `bmi2` target, 95 % CI +16.19 to +20.28,
node-identical**, so DEC-083 took a timing and not a match.

**The measurement instrument was set two ways that spend the budget and one
that measured the wrong invariant.** The SPRT runs at `10+0.2` where the
engines this plan reads from test at 8+0.08, and on a balanced book where every
surveyed engine defaults to a UHO book — both cost throughput and S105 fixes
both. On hash, DEC-083 ordered a move to 128 MB to match the list's absolute
size; the second pass corrected the invariant (DEC-088): what transfers across
time controls is table **pressure**, every OpenBench preset tests STC at 8 to
32 MB, and 16 MB at 8+0.08 reproduces the list's ~60-120 overwrites per entry
where 128 MB undershoots them eightfold. `fastchess.sh` keeps Hash=16;
`rating.sh` keeps the list's absolute regime. DEC-083's other rule stands: a
behaviour-neutral change is accepted on an interleaved timing, never on a
match. ~~Together, roughly three times the verdicts per night.~~ **Measured
2026-08-20 when S105 landed: x1.67, 23.1 to 38.7 games a minute** -- x1.41 from
the control and x1.20 from shorter games, with the book contributing nothing at
the pair level (variance ratio 1.022). The three-times figure was a prediction
and it was wrong; the machine-hours below were already priced near the measured
number, so the order does not change.

**The next step as it stood would have measured zero.** S092, the improving
flag, was first in the pending order. It is a modifier of reverse futility, late
move pruning, futility, null move and the reduction; chesso has two of those
five. And it needs a static evaluation at every node, which the main search
computes only at reverse-futility nodes. S092 is retired into **S108**, which
supplies the input, and its consumers arrive at **S109**.

**The pruning rules pay together and are measured together. DEC-082, narrowed
by DEC-087.** Stockfish's removal test measures move-count pruning at ~0 alone
and its block at ~204; Lynx measured late move pruning and futility at **+4.7
each** when added alone, so the parts are small-positive rather than inert.
The block form is kept on measurement-budget grounds — four verdicts near +5
crawl at the bounds (DEC-063) where one verdict on +40 to +120 resolves in an
hour — and a failing block is bisected. S090 and S026 stay retired into
**S109**; one step still owes one test.

**The literature is read for form, never for constants. DEC-084, its seed rule
made provenance-based by DEC-105.** A number may seed a fit only if it
originates in a publication about the technique — a paper, an article, the
wiki's own derivations. A number that originates as another engine's tuned
output is never a seed, wherever it is republished: PeSTO's tables sit on the
wiki and are still an engine's tables. And **no constant ships unfitted** in
either case — a seed is where our own tuner or SPSA begins, and a fit that
lands on the published value is a confirmation and a good outcome. Every step
below that takes a formula from the record ends in a fit for this reason.

**Two items were carried on a false premise. DEC-085, premise corrected by
DEC-089.** The CCRL Blitz list rates 1CPU, 4CPU and 8CPU builds as separate
entries — not "single CPU" as DEC-085 said — but the 2559 anchor and DEC-071's
reachability table were both taken against **1CPU entries**, so the target is
the 1CPU scale and the conclusion survives its corrected premise: **S086 stays
retired** (own books must be disabled; learning off) and threading is not a
phase-one step. An 8-thread search is worth about +180 at LTC on the published
measurement, which is a phase-two rating if the owner ever wants one.
Tablebases are the one thing on that list the rules allow, which is why S129
exists and sits last. **S031 is also retired** — under 1 % by its own file,
below every instrument here.

**Ten steps whose whole content was a correction to another step are folded
into the steps they correct. DEC-086.** S056 to S061, S063 and S079 to S081
live on inside the files they corrected; ids not reused; the parked 2026-08-16
plan_review re-run is still owed.

## What the second pass changed, DEC-087

**Retired:** S096 check extensions — Ethereal removed check extensions for
+4.1/+4.5 and Stormphrax removed them too; LMR here already exempts checking
moves and S097 covers the forcing-line concern.

**Reserve, behind the 3000 push:** S023 capture history (failed four SPRTs at
~2600, STC-negative at Weiss; its value returns as a reduction/margin input
above 3000), S025 which requires it, and S110/S111 correction-history
extensions (+3 to +8, measured only above ~3100). **S099 pawn correction
history stays** — +11.4 at ~2850 is the one sub-3000 correction number.

**Added:** S130 quiescence stand-pat from the table score where the bound
permits (+10.8/+12.1 at Weiss; S094 already pays the probe); S131 quiescence
searches non-capture queen promotions (this engine's own recorded TODO); S132
the soft time limit scales with the best root move's node share (+9.9/+9.7 at
Ethereal); S133 king-relative piece-square tables (~+88 Leorik 2.5, ~+65
Berserk 4.3 — the largest documented evaluation item the plan had no step
for), owner-approved with its INV-4 rebuild cost stated.

**Re-scoped:** S114 drops the null-move verification search and S115 the
volatility-seeded window — neither has evidence below 3000; S083's 50 M floor
becomes a held-out-error decision under a stated datagen budget (the band
fitted on 4.5 to 10 M resolved positions); S100 is diagnosis first, its
per-term SPRTs deferred into the evaluation block; S093 lands malus and
gravity as one verdict, persistence as a second; S118 moves out of the speed
block to land after the pawn terms it caches are worth caching (a cheap pawn
eval cached measured a published slowdown).

**Two existence proofs sharpen the target.** Weiss 1.2 sat at 3055 CCRL Blitz
with a 301-line evaluation, no capture, continuation or correction history,
and the full pruning stack — after this search block chesso's search is a
superset of it. Stash crossed 3000 at v27 and reached 3424 with no network.

## Three things the first review measured that no step existed for

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
between them is what carries and not the absolute Mnps. So S039 sits just
ahead of the evaluation rebuilds, and it is no longer a micro-tune: it is the
architectural prerequisite, with S120's evaluation cache buying back what it
spends.

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
-17}`, seventh rank fitted below sixth and negative. Whatever causes it might
also be acting on the other 817 constants, so **S100 is a diagnostic and goes
early**; its per-term refits follow in the evaluation block once the corpus they
would be fitted on exists.

**S100 answered it on 2026-08-20, and the premise above was half wrong.**
Nothing is acting on the other 817 constants: the extraction, the gradient and
the whole fit pipeline are clean, checked over all 10795695 corpus rows, against
finite differences on all 827 parameters, and by recovering a planted vector end
to end. The pattern has two causes and neither is a defect. **Two of the six
symptoms are exact algebraic degeneracies** — rook-on-the-seventh against
`psqt[rook][8..15]` and passer bucket 5 against `psqt[pawn][8..15]`, R² exactly
1.000000 — so their split from the tables is not identified and `-17` is a ridge
position rather than a valuation, which is why three fits put that one bucket at
+22, -1 and -17 while no other bucket moves by more than 4. **The other four were
never separately measured**: three share one bundled SPRT at `--fast` bounds that
cannot resolve their published effect size, tempo reached neither bound, and all
five have been *held* at zero by `--freeze tempo,piece_placement` since S065
rather than fitted to it. So the refits below are still owed, their shape is
fixed — bishop pair alone first, frozen-base for the two degenerate ones, bounds
that can resolve single digits — and **`--freeze tempo,piece_placement` (DEC-057)
now rests on a verdict the diagnosis calls procedural, which is a decision the
owner is owed.** `adocs/plan_done/S100_zero_weight_terms_reexamined.md` is the
ledger.

## What the 2026-08-22 audit inserted

One correctness batch, S160 to S165, directly after S130 — found bugs are
fixed before anything else starts, because a known defect in the tree
contaminates every measurement taken after it. The audit
(`adocs/audit/2026-08-22_adversarial.md`: no high finding, two medium, five
low, S130-in-flight verified sound line by line) priced the batch so the
search block resumes essentially undelayed: S160 disarms the instrument first
— `fastchess.sh`'s default reference was still the 2026-08-08 baseline while
`CLAUDE.md` taught the bare invocation, the DEC-020 class armed in the
per-change instrument — then S161 closes the FEN semantic hole through which
accepted input corrupts the board in Release, the one prime-directive
violation found; S162 scores checkmate ahead of the 100-halfmove draw; S163
removes the stale-timer race that can kill a fresh search at depth 1; S164
pins S130's substitution on the lazy-bound path its tests do not reach; and
S165 settles the null-move mate-band asymmetry, sweep first, guard or
decision second. **S165 alone owes a match**, one `--nonreg` verdict: S162's insurance run was
killed at 3304 games on a census of its own PGN -- 0 positions checkmate at a
halfmove clock of 100 or more over 3356 games, so the only count it could have
measured was zero -- and DEC-107 records the rule that came out of it. The seventh finding, the verbatim Polyglot constant
table, folds into S146, whose own excludes had left it out of scope.

**S166 was found by running the batch, not by the audit.** S162's accepts
requires its regression position to be tool-verified with `stockfish`, and the
obvious invocation of it -- `printf 'position fen ...\ngo depth 20\nquit\n' |
stockfish` -- answered `score cp 0` and `bestmove a1b1` for a position that is
mate in one, because `quit` stops the search the instant `go` starts it. The
oracle `CLAUDE.md` names for every chess question the agent may not answer
itself can therefore return a confident wrong answer with `nodes 0` on the same
line, and `TOOLCHAIN.md` has no stockfish section to say so. It is documentation
only and it sits with the batch rather than at the end of the plan because it
guards DEC-023.

## The order, in four blocks

**Block 0, instruments and free wins, S105 to S085.** None of it is a feature.
S105 is the harness — 8+0.08, Hash 16, a UHO book, stated default bounds — and
it multiplies everything below; S106 is a correctness sweep, because a
bound-sign inversion in quiescence has been measured at 64 Elo elsewhere and
S094 already found one defect of exactly that shape here; S107 is one line that
stops the ordering tables refusing to remember checking quiet moves; S100 is
the zero-weight diagnostic; S084 and S085 are the SPSA driver and a first run
over the twenty parameters that exist today, which are demonstrably mis-set —
`MaxQsearchDepth` is 8, the bound binds, and raising it moves both the node
count and the score. S137 sits between them because S084 found the tune build
cannot report a refused `setoption` at all — `LOG_W` is compiled out of a
Release build — so a tuner's two ways of being wrong, a value out of range and a
misspelled name, are both indistinguishable from success. The driver defends
itself against both; the binary should not need it to.

**Block 1, the search, S093 to S132.** Ordered so each step's consumers exist
before it, biggest evidence first. History malus and gravity (S093 — done:
+37.5 and +28 reported, **+10.73 ± 6.70 measured** and kept; persistence across
`go` reported +12.5, **−1.65 ± 4.22 measured** and reverted, DEC-101), the free
quiescence
stand-pat consumer (S130), the 2026-08-22 audit batch (S160 to S165,
correctness and instrument hygiene, near-zero match time), the static
evaluation and improving flag the pruning needs (S108), continuation history (S024 — +44.7/+34.0 reported for
the one-ply table, the largest ordering gain surveyed), the pruning block
itself (S109), SEE pruning of captures in the main search (S091), the
reduction rebuild (S098), internal iterative reduction moved up because it is
three lines (S095), pawn correction history (S099), singular extensions with
multicut folded in (S097 — the one feature every 3000-plus engine has and this
one lacks), then quiescence: per-move futility (S112), non-capture promotions
(S131), the delta-pruning decision (S022). The tail is ProbCut (S113), the
eval-scaled null move (S114), the aspiration plumbing (S115), razoring at
depth one (S116), and the node-fraction time scaler (S132).

**Block 2, speed, S020 to S030.** All of it either behaviour-neutral or nearly
so, and cheap under DEC-083 because most of it owes a timing rather than a
match: the duplicated per-node `is_check` (S020), the merged taper (S055), the
packed middlegame/endgame score (S117 — +25.41 reported, the largest evaluation
speed number surveyed), the evaluation cache (S120), the table layout at the
S105 pressure setting (S119), and the movegen work that was always correctly
last (S042, S032, S030). The pawn hash left this block for block 3 (DEC-087).

**Block 3, the corpus and the evaluation, S134 to S126.** It opens with the one
step that fits nothing: S134 folds rook-on-the-seventh and passer bucket 5 into
the piece-square tables, deleting the two exact algebraic degeneracies S100
found, bit-exact and so discharged on node counts rather than on a match
(DEC-090). It sets `PARAM_COUNT` to 823 and it `blocks:` S135, which is why it
goes before the corpus rather than beside the refits. Then corpus, because
fitting a term on a corpus about to be replaced is the trap
`src/evaluation.cpp` already records against S027: leaf-resolved labels at a
few rows per game (S082), then the size-versus-nodes answer under a stated
budget (S083). Then the two zero-weight groups S100 showed were held at zero
rather than measured to it, each one bundled verdict at bounds that can resolve
single digits: the three remaining placement features (S135) and tempo (S136,
which also re-derives the taper truncation guard its zero weight holds down).
Then the clamp decision (S039), then the terms in the order the
Stash ledger prices them: mobility area and curves (S121, +20 class), passed
pawns with king distance (S123, +22.3 the largest single entry), the connected
and phalanx pawn work (S125, +25.4 class), the pawn hash that makes them
affordable (S118), threats (S101, +10 class), king safety (S122 — biggest
cumulative payoff and the documented failure magnet, so it follows the corpus
work), endgame scaling (S124), outposts and space (S102), the king-relative
tables (S133), and one full refit (S126), because every weight fitted before
S109 was fitted against a tree that no longer exists.

**Block 4, close, S127 and S129.** The full SPSA run over everything the search
block added, and Syzygy last and optional, because the no-copy rule prices 13 to 25
Elo at a from-scratch implementation of a compressed table format (the
MIT-licensed Fathom route is on the record in DEC-087 if the owner ever wants
it).

**Reserve, after the mark or on spare nights: S023, S025, S110, S111.** Kept
whole with their evidence; they are 3100-band techniques by the record.

## What this costs

The pending order owes **roughly 45 to 55 SPRT verdicts** once multi-verdict
steps are counted honestly (S024 two, S098 three, S097 and S022 two each, the
evaluation groups per-term), plus two SPSA nights, one to three datagen nights,
and S152's two five-hour gauntlets at the end. At the S105 settings a typical
verdict is 45 to 75 minutes, so **roughly 75 to 110 machine-hours** — against
the 150 to 250 the pre-review plan priced for less ground.

**S093 is the first multi-verdict step to close and it cost more than that
estimate, both in hours and in what it bought.** Its two verdicts took 2 h 44 m
and 6 h 35 m over 21810 games, against the 45-to-75-minute typical — a null
against `elo0=0 elo1=5` runs to the wall by construction, and the second verdict
did. One of the two was kept. Read the estimate as verdicts owed, not as
verdicts that will land. Six steps stop
owing a match at all under DEC-083.

**The Elo arithmetic is a range and not a forecast, and DEC-019 is why.**
Discounting self-play to list Elo at the ratio the published per-release records
support (~60 % sticks), and taking a fifth off for interaction: search +180 to
+280, evaluation +90 to +160, speed +40 to +90, tuning +50 to +90, and S133 is
+30 to +60 of margin on top. The midpoint clears 3000. Two things are not
promised by that: the 2559 anchor is soft by 121.8 Elo of internal disagreement
until S152 runs, and this project has taken three published figures at face
value and measured 0, 0 and *slower*. **And nothing between here and there
measures the total.** DEC-108 is the owner's decision that the engine is re-rated
once, near the goal, rather than at the block boundary S152 was written for:
a checkpoint buys information and no strength, and no number it could return
would reorder this list. So the evidence of progress until then is the
per-change SPRT ledger and nothing else, which is what makes each run's
pre-registered reading load-bearing rather than a formality. S128's
anchor-spread question folds into S152's run, which therefore carries both
controls: the spread is 121.8 Elo over five references and 64.7 without
Leorik 2.1, against the 30 the procedure allows, and DEC-077 named the
control as the leading candidate. Asked for an estimate on 2026-08-23 the honest answer was 2600 to
2650: the kept positive point estimates since S088 sum to about +90, DEC-063's
own correction factor cuts that to about +49, and S104's +18.22 % is unpriced
because what a ply is worth here has never been measured. That is arithmetic,
not a measurement, and 2559 +/-25 soft stays the engine's rating.

## How this file works

Order lives here and nowhere else. Step detail lives in the step files under
`plan_todo/`, `plan_current/`, and `plan_done/`. Ids are allocated in creation
order and never renumbered or reused — including for the retired S019, S026,
S031, S056 to S061, S063, S079 to S081, S086, S090, S092 and S096, and for
S128, folded into S152 by DEC-108 — so reordering is a one-line edit to the
Open list.

Order is read from the numbered entries under `## Open`, and the next step is
the first of them. An id named in a sentence anywhere else in this file is
prose: it does not change the order. Every pending step file appears as an
Open entry and every Open entry names an existing step file.

**Both lists are maintained by hand, and that is new since 2026-08-29.** Until
then a checker in the moltke plugin added an entry when a step was created,
pruned a completed one, and kept the last five completed in *list order* rather
than by completion date — which is why S145 sat among four newer entries and
why `status.md`'s "last done" could name an older step than the newest
completion. moltke v1 has no checker, no hooks and no `--step` command
(DEC-109), so completing a step now means moving its file, moving its entry
from Open into `## Done recently`, dropping the oldest of the five, and
rewriting `status.md` — deliberately, in the completing commit. Nothing
enforces it but the diff.


## Done recently

- S108  every node that is not in check computes its static evaluation once, stores it in its table entry and reads it back, so improving and the pruning margins have an input
- S167  fastchess.sh runs and reports failure correctly under bash 3.2, the macOS /bin/bash
- S153  an agent-only step and a match-owning step may be active at once, so a document step does not leave the machine idle
- S158  S146 carries the shipped book's digest and entry count so its origin can be searched for rather than guessed
- S157  every recorded conclusion that states an SPRT bound in plain Elo says nElo instead, and DEV_MANUAL.md says which scale the bounds are in

## Machine scope, 2026-08-30 to whenever the workstation is back

**Temporary, DEC-112, and it exists to be removed.** The owner is away from the
Linux workstation with the MacBook `.moltke.local.md` describes. That machine
measured itself at 2700 games/h on 8 threads at 8+0.08 and, twice, went down
after four to five hours of a full-core match -- which is what ended S024 here
(DEC-111). It is a fine machine for documents, for a behaviour-neutral change
proved on node counts, and for a single short verdict. It is the wrong machine
for a step that owes three.

So the Open list below is **reordered, not rewritten**: the machine-light steps
are lifted to the head and everything else keeps the relative order the
2026-08-19 review gave it (DEC-081 to DEC-086). Nothing is renumbered, no
step's content changes, and putting the list back is one edit. Read the block
structure in `## The order, in four blocks` above -- it still describes the
real dependency order and this section does not supersede it.

**The lane, entries 1 to 12.** Seven steps that touch no engine and own no match
(S150, S155, S156, S154, S143, S144, S146 -- S153, S158 and S157 are done),
three that are
behaviour-neutral and discharged on identical `tools/search_bench.py` node
counts and best moves plus a `hyperfine` timing (S147, S020, S030), and two
that each own exactly one self-contained verdict if the owner wants a short run
(S148, S159). S148 and S159 depend on nothing in the search block, which is why
they and not S095 or S131 are the ones that can move.

**What this machine cannot do, and why, so nobody rediscovers it:**

| what | why |
|---|---|
| S032 | `_pext_u64` is BMI2 and this is an M1. There is no path to measure it here |
| S119 | huge pages and the prefetch it pairs with are the x86 tuning the step is written against, and it alters replacement so it owes a verdict too |
| S117 | its `accepts` reproduces the taper exactly, and after S055 that is one division rather than two -- so it follows S055, which owes its own SPRT |
| S109, S091, S098, S097, S022 | three or more verdicts each. At four to five hours of match per attempt this is where the machine runs out, not the plan |
| S082, S083, and all of block 3 | `selfplay_v2.tsv` is 683 MB, gitignored, and never made the machine move. Only `selfplay_v1.tsv` is here, so every fit and every corpus step is on the workstation by construction |
| S024 | discarded here and redone there, DEC-111 |
| S127, S126, S152 | an SPSA night, a full refit and a five-hour gauntlet pair |

**Restoring the order** means deleting this section and sorting the Open list
back to the block order above. Do it as a decision, not as a tidy-up, so the
reason the lane existed stays findable.

## Open

1. S150  a number stated in a plan or manual document about a search parameter is checked against search_param_info(), and the three values S085 left stale in specs.md and MANUAL.md are corrected
2. S155  the constructed mate set's single motif is stated where its breadth is claimed, and what the gate therefore cannot catch is written down
3. S156  the mined breadth set is either asserted as the count with a floor its accepts asked for, or the accepts is discharged in the stamp with the reason
4. S154  the mate-in-three floor's tolerance to a neutral tree change is measured rather than asserted, and the RfpMinPly-1 mate-in-two count is restated as what the assertion actually fails
5. S143  the completion gate builds and tests the tune build as well as the shipping one, so a change cannot leave build-tune broken unnoticed
6. S144  a citation in a plan document carries its own path, so the 467 bare line references that inherit a path from prose become checkable
7. S146  the 5.2 MB opening book compiled into the shipped binary has a recorded origin and licence, or it is replaced by one that does
8. S147  a mate score is reported with a principal variation long enough to reach the mate it claims, so fastchess's -check-mate-pvs stops warning on a truncation
9. S020  compute the in-check state once per node instead of once per call site
10. S030  move_t drops the moving piece and becomes 16 bits
11. S148  the reverse futility depth ceiling is re-decided against the deep mates S145 measured it losing, by SPRT and not by argument
12. S159  measure whether the second killer slot wants ageing rather than distinctness: the unguarded shift discards slot 1 on every repeat, so S149's -11 Elo may be the guard preserving a stale killer for a whole go
13. S024  history indexed by the move played n plies ago and the current move
14. S109  late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
15. S091  skip captures the exchange evaluation says lose material, in the main search rather than in quiescence alone, and reduce a negative-SEE move by an extra ply
16. S098  the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
17. S095  reduce a node whose table entry carries no move instead of searching it at full depth
18. S099  a static evaluation correction learned from the difference between the static score and what the search returned, keyed on the pawn structure
19. S097  extend the one move a verification search says is singular, and take the multicut the same search offers
20. S112  quiescence skips a capture whose best case cannot reach alpha, per move, before the exchange evaluation is consulted
21. S131  quiescence searches non-capture queen promotions instead of filtering them out
22. S022  decide between delta pruning and the per-move futility S112 adds, by measurement -- deleting delta pruning is a valid recorded outcome
23. S113  a shallow verification search over good captures prunes a node whose score is already far above beta
24. S114  the null move reduction scales with how far the static score is above beta, and the base reduction is re-decided
25. S115  the widening schedule is re-swept fail-soft, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply
26. S116  a node whose static score is hopelessly below alpha drops straight to quiescence, at depth one only
27. S132  the soft time limit scales with the share of the root's nodes the best move consumed, spending less when the choice is not in doubt
28. S055  taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
29. S117  the middlegame and endgame halves of every evaluation term travel in one integer instead of two
30. S120  a small cache of full evaluations by position key, so the score behind the lazy shortcut can be paid for once
31. S119  the table becomes cache-line clusters with an aged replacement, a prefetch issued when the key is known, and huge pages
32. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
33. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
34. S134  delete rook-on-the-seventh and passer bucket 5 by folding their weights into the piece-square tables, which is bit-exact, and shrink the parameter vector to 823
35. S082  the corpus labels a resolved position rather than the root -- the quiescence leaf, or the leaf reached by playing out a deep search's whole principal variation -- and samples few positions per game rather than many
36. S083  the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, not by a volume target
37. S135  unfreeze the piece placement group and refit it, one bundled SPRT over the three remaining features, by the owner's decision of 2026-08-20
38. S136  unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- at two divisions once S055 has landed -- refit and resolve it at bounds that can
39. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
40. S121  mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on
41. S123  passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
42. S125  backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
43. S118  the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
44. S101  evaluation terms for a piece attacked by a lesser piece, fitted like every other constant
45. S122  king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks and weak squares, and it is no longer clamped
46. S124  the endgame half of the score is scaled toward a draw by what is actually on the board
47. S102  outpost and space terms in the evaluation, fitted like every other constant
48. S133  the piece-square tables become king-relative -- indexed by a king bucket as well as piece and square -- and every entry is fitted
49. S126  every constant in the evaluation is refitted once the search that consumes them has stopped moving
50. S127  an SPSA run over the whole search parameter set as it stands after the search block, and an independent SPRT of what it returns
51. S129  three, four and five man tablebase probing, written from the format description
52. S023  **reserve, DEC-087** — history indexed by piece, target and victim, to order captures MVV-LVA rates equal
53. S025  **reserve, DEC-087** — retry searching losing captures after the quiets, now that capture history exists
54. S110  **reserve, DEC-087** — a second correction table keyed on the non-pawn structure, split by colour
55. S111  **reserve, DEC-087** — correction tables indexed by the move played two and four plies ago
56. S029  **parked, DEC-054** — a perspective network evaluation trained on chesso's own self-play
57. S151  a change that moves a pruning or reduction parameter has its verdict re-taken at a control at least four times longer before the number is banked, starting with S085's shipped vector
58. S152  **deferred, DEC-108** — the engine's absolute rating is re-measured once, near the 3000 mark rather than at a block boundary, at both time controls so S128's anchor-spread question is answered by the same run
