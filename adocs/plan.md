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
engines this plan reads from test faster than that — **14 of the 20 OpenBench
presets at 8.0+0.08 and the other six at 10.0+0.1**, Berserk, Ethereal, Igel,
RubiChess, Stockfish and Weiss being the six, so the split is real and 8+0.08
is the majority rather than the universal setting (row A33 of
`adocs/data/2026-09-04_plan_review_literature_check.md`, fetched 2026-09-04) —
and on a balanced book where every surveyed engine defaults to a UHO book —
both cost throughput and S105 fixes both. On hash, DEC-083 ordered a move to 128 MB to match the list's absolute
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
moves and S097 covers the forcing-line concern. **Corrected 2026-09-04,
DEC-133:** Ethereal's commit removed only the check extension applied before
the move loop and its master still extends a checking move inside it;
Stormphrax's removal carries no Elo; Weiss 1.2 and Stash carry the in-loop
form. S096 stays retired by id and the in-loop extension is **S188**, after
S097.

**Reserve, behind the 3000 push:** S023 capture history (failed four SPRTs at
~2600, STC-negative at Weiss; its value returns as a reduction/margin input
above 3000), S025 which requires it, and S110/S111 correction-history
extensions (+3 to +8, measured only above ~3100). **S099 pawn correction
history stays** — +11.4 at ~2850 is the one sub-3000 correction number.
**Corrected 2026-09-04, DEC-133:** the +11.4 (Lynx #1662) was measured between
Lynx v1.9.0 and v1.10.0, 3226 and 3293 on the list, so no correction table has
sub-3000 evidence and the criterion separates nothing; **S099 moves to the
head of the reserve** as the family's probe, S110 and S111 gated on it, and
S181 re-bands every Lynx figure the steps cite.

**Added:** S130 quiescence stand-pat from the table score where the bound
permits (+10.8/+12.1 at Weiss; S094 already pays the probe); S131 quiescence
searches non-capture queen promotions (this engine's own recorded TODO); S132
the soft time limit scales with the best root move's node share (+9.9/+9.7 at
Ethereal); S133 king-relative piece-square tables (~+88 Leorik 2.5, ~+65
Berserk 4.3 — the largest documented evaluation item the plan had no step
for), owner-approved with its INV-4 rebuild cost stated. **Corrected
2026-09-04, DEC-133:** both figures are release-bundle deltas — Berserk
4.3.0's +65 is the author's estimate for the whole release, Leorik's +88 the
2.4 to 2.5 CCRL delta of a release that also shipped PEXT, threads and .NET 8
— and no isolated number exists; S133 is kept on adoption breadth.

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
`LazyEvalMargin`, 184 as shipped since S085's SPSA run raised it from 150, for
the two together — and `evaluate()` is
`evaluate_cheap() + evaluate_expensive()`, so the clamp is on the real score
and not only on the shortcut's. A mating attack is worth four to six hundred
centipawns and this evaluation cannot say more than a hundred and eighty-four
about the king and the mobility combined. Measured cost of removing it, at the
150 that compiled when the sweep was taken:
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
table, folded into S146 and is settled at DEC-121: format-defining
specification, kept and cited at the array.

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

## What the 2026-09-03 audit inserted

One correctness batch, S174 to S177, at the head of the Open list -- found bugs
are fixed before anything else starts, because a known defect in the tree
contaminates every measurement taken after it. The audit
(`adocs/audit/2026-09-03_adversarial.md`: no high finding, two medium, two low;
fast suite green and both node-count baselines reproduced; the project's own
rules turned up nothing; every technique found behind the literature already
had a step) found both mediums in the code S172 and S146 shipped three days
earlier, by re-deriving the shipped book independently with python-chess rather
than by reading the stamps. S174 goes first: the SAN parser every tool sits on
fabricates a move instead of failing in Release, so `make_book` and
`pgn_to_positions` -- the DEC-023 board tool -- run on silently on a rewritten
board, and `make_book` must be unable to do that before anything is rebuilt
with it. S175 fixes the Polyglot key, which wraps round the board edge for an
en-passant square on the a- or h-file, and rebuilds the shipped book to the
format's keys -- 7 of 172232 entries move, the digest changes, and the
independent re-derivation becomes the conformance check. S176 makes `position
fen` accept the four- and five-field forms and keep the whole previous position
on a bad FEN. S177 gives `rating.sh` and `build_release.sh` the portability S167
gave `fastchess.sh`. **None of the four owes a match**: nothing in the batch
touches a search path, and INV-6 is discharged on identical node counts in each.

Two techniques the audit found absent and in no step are recorded there and
given none: mate-distance pruning and a per-node reset of the killers two plies
down, both small in the published record and below the instrument at this
strength. **The batch is done** -- S174 to S177 are all in `plan_done/` -- and
the machine-scope lane this paragraph once deferred to no longer exists: DEC-144
deleted it on 2026-09-05 and the section below is what replaced it. S020 sits in
block 2, where the 2026-08-19 review put it.

## What the 2026-09-04 plan review inserted

The third review of this file (`adocs/audit/2026-09-04_plan_review.md`: no
high, five medium, five low; fast suite green in both builds, both node-count
baselines reproduced, 45 of 46 prior plan-review findings closed on
re-measurement) read the plan against the published record, against the code
at HEAD and against the project's own rules, and an independent pass fetched
33 of the figures this file argues from at their sources
(`adocs/data/2026-09-04_plan_review_literature_check.md`, DEC-137). Most
figures held. What did not, and where each is answered: seven enriched search
steps seed constants from other engines' commit-message prose, which DEC-105
forbids by origin -- **S180**, DEC-134; the Lynx band DEC-087 kept S099 on is
3226 to 3293, not ~2850, so S099 is the reserve's head and S110/S111 are
gated on it -- **S181**, DEC-133; Ethereal removed only its pre-move-loop
check extension, so the in-loop form is reopened as **S188** after S097,
DEC-133; S133's two figures are release bundles and it is kept on adoption
breadth, DEC-133; the cost line prices a verdict at 45 to 75 minutes where the
ledger says 4 h 40 m -- **S182**; the Elo arithmetic has no recorded inputs
and applies no published-to-measured discount -- **S183**, DEC-136; S115
designs its sweep on a triple S085 replaced and would revert a verified axis,
with three smaller document defects beside it -- **S184**; block 3's ordering
figures had no source in the tree, both ledgers are located now and **S185**
records them, and the enrichment DEC-097 promised becomes **S186**, ordered
before block 3, DEC-137; 59 citations drifted three days after the last
re-anchor, so citations from pending files into code become symbols --
**S187**, DEC-135. Techniques the surveyed engines carry and this plan gives
no step are recorded in DEC-138 so the next audit does not re-find them.

Nine steps. Eight are documents and owe no run; S188 owes one SPRT. The seven
documentation steps sit in the instrument lane directly behind the
workstation's first runs (DEC-144), S180 and S184 first because each removes a
hazard an implementer would otherwise follow. **The cost and Elo figures in
"What this costs" below stand as written until S182 and S183 rewrite them, and
the review's ledger says the cost is two to three times what they state.**

## What the 2026-09-04 test review inserted

The owner asked how this engine decides that a change is an improvement, and
whether its tests are worth what they claim. The assessment
(`adocs/audit/2026-09-04_test_review.md`: no high, three medium, seven low)
measured the fast suite instead of reading it -- 33 hand-written engine bugs
injected one at a time, **31 of 32 non-equivalent caught**, the survivor a
fifty-move boundary one halfmove late; 98.9 % line coverage of the search;
35 runs with no unexplained failure -- and found where the detection comes
from: two invariants live in assertions both gated builds compile out, the
null-move and reduction guards are covered by one golden count and nothing
else, and much of the suite's sensitivity is golden numbers every legitimate
search change will also move. The survey (`adocs/testing_strategy.md`, every
figure fetched at source or marked unverified) put the SPRT regime beside
fishtest's and found it right and uncalibrated on this machine, and priced
the bounds: a `{0,5}` pair costs 41861 expected games at the midpoint, 17.9 h
at the 2337 games an hour the ledger measures here, which is what S182's
rewrite carries.

Fourteen recommendations, decided one at a time on 2026-09-05 (DEC-139):
**S189** the bench signature in every `src/` commit and `tools/gate.sh`
(DEC-140); **S190** INV-2 and INV-4 in a Release test, with a Debug
self-play habit (DEC-141); **S191** direct tests for the pruning and
reduction guards, before S109; **S192** every golden named and scripted
(DEC-142); **S193** the vacuous assertions, the fifty-move boundary and the
suite's mechanical hazards; **S194** the UCI book path under test; **S195**
a reproducibility test across `ucinewgame`, the Zobrist keys joining S179;
**S196** the fault-injection driver as a tool and a mutant per new search
rule (DEC-141); **S197** `tools/gate_extra.sh` and the coverage recipe;
**S198** a seeded opening order and richer PGN fields, calibrated by the
workstation's A/A (DEC-143); **S199** a fixed-rounds drift match against a
pinned reference after each block. Calibrating the harness on this MacBook
was refused -- the workstation is back soon -- and a `NodesTime` screening
clock is deferred to S127's design. Nine of the eleven are machine-free and
sit in the instrument lane ahead of the search block (DEC-144); S198's A/A is
the workstation's first calibration and S199's first point follows S109.

## The order, in four blocks

**Block 0, instruments and free wins, S105 to S085.** None of it is a feature.
S105 is the harness — 8+0.08, Hash 16, a UHO book, stated default bounds — and
it multiplies everything below; S106 is a correctness sweep, because a
bound-sign inversion in quiescence has been measured at 64 Elo elsewhere and
S094 already found one defect of exactly that shape here; S107 is one line that
stops the ordering tables refusing to remember checking quiet moves; S100 is
the zero-weight diagnostic; S084 and S085 are the SPSA driver and a first run
over the twenty parameters that exist today, which are demonstrably mis-set —
`MaxQsearchDepth` was 8 when this was written, the bound binds, and raising it
moves both the node count and the score -- S085 shipped 19. S137 sits between them because S084 found the tune build
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
Open list. S096's technique is reopened as S188 by DEC-133; the id stays
retired.

Order is read from the numbered entries under `## Open`, and the next step is
the first of them. An id named in a sentence anywhere else in this file is
prose: it does not change the order. Every pending step file appears as an
Open entry and every Open entry names an existing step file.

**A citation names a file and a symbol, and repeats the path for each one.**
`src/search.cpp` `negamax`, `src/evaluation.hpp` `LAZY_EVAL_MARGIN`,
`tests/test_search.cpp` "pruning does not hide a forced mate" -- and, where
there is no definition to name, a phrase quoted out of the file. Never a line
number, and never `:917` after a sentence that named the file. Backtick the
symbol: the checker fires on a backticked identifier or a double-quoted phrase
directly after the path and on nothing else, so a path followed by an ordinary
word is prose and goes unchecked.

Two rules, and both were paid for. **The path is repeated** because
`tools/plan_prose_check.py --citations` cannot decide which file an inherited
path meant: before S144 it counted the bare ones ungated -- 383 of them, stale
in bulk with nothing saying so, and one of the eight steps that pointed an
implementer at the wrong mate-safety test was missed by an audit for exactly
that reason. A bare `:line` is `BARE` and fails the run. DEC-120. **The symbol
replaces the line** because repairing the drift one round at a time stopped
working: S169 re-anchored 97 citations on 2026-09-01 and 59 had drifted again
three days later, every one of them sitting beside the symbol it named. So the
symbol became the reference and the line was dropped. DEC-135, converted by
S187 -- 572 of them -- and gated in the fast suite since (DEC-159): a
`path:line` in a pending step file is `LINE`, and a symbol the named file does
not carry is `MISSING`.

The check is existence, not relevance. `src/search.cpp` `state` passes because
`state` occurs in the file. What proves a citation means what it says is the
mapping a conversion was made through -- `adocs/data/S144_pathings.tsv`,
`adocs/data/S169_recitations.tsv`, `adocs/data/S187_symbols.tsv` -- and never
a green run.

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


- 

## Done recently

- S159  **the ageing reading of S149's -11 Elo is refuted by its own census, before a game was played, and no SPRT was spent on it** -- counted over 18166063 nodes on 11 recorded positions, on HEAD and on HEAD with S149's guard re-applied to an instrumented copy: **the stale share of distinct second-killer offers is 1.96 % against 1.91 %, flat**. The guard did not preserve proportionally staler killers; it roughly doubled how often a distinct second killer is offered at all, **45.4 % of nodes to 88.8 %**, and the stale count rose only with it. On HEAD the unguarded shift is already the ageing mechanism -- it discards slot 1 on **72.0 % of stores** -- so candidate A, the table cleared once per iteration, had **0.89 % of nodes** to act on and was built, tested and reverted unrun. The `{-5, 5}` run was pre-registered, priced at 10465 games and 4 h 36 m worst case, and declined on what the census had already shown: DEC-160, and the census sits in the script header so it could not be reread over the games afterwards. Duplicate rates reproduce S149's on the new set, 72.0 % of stores and 44.0 % of nodes against 66.0 % and 44.4 %. **Candidate B is neutral as the file predicted**, node- and best-move-identical at depths 9 and 12 (INV-6), its only observable the duplication fence going red for a change that alters no game. **S149's own driver turned out not to be reproducible** -- its 11 positions are named in two documents and their FENs recorded in neither -- so S159's set, driver, instrumentation patch and outputs are recorded under `adocs/data/S159_*`. **And the step found a bug and did not carry it**: candidate A reddened `test_mate_carry` against a green HEAD, the S203 sweep over both sides showed `C_mate7_depth11` reporting mate lines in **one cell of nine** at HEAD with its pinned budget being that cell, and that is S204 and DEC-161. No `src/` change ships
- S190  **INV-2 and INV-4 are enforced by the gate** -- `tests/test_invariants` walks every test FEN two plies deep plus the five positions of `test_engine`'s hash oracle at their own depths, **2132167 `make_move` calls**, and after every make and unmake rebuilds the four accumulators with `eval_refresh`, rebuilds `squares[]` from the bitboards, and compares the whole `board_t` against its pre-make copy. **1.816 s +/- 0.018 s** in Release against a 5 s budget, 12.7 s in Debug. Both gated builds are Release, where every `assert` in `src/` is dead, so the two helpers moved out of `#ifndef NDEBUG` and lost `static`; the assert sites are untouched and the engine's behaviour does not change -- identical node counts and best moves against `dc878de` at both depths and `bench 26851183` on both sides. **The `accepts:`' depth 3 was unsatisfiable** at 53975914 makes and about 34 s, and the owner re-decided it to depth 2 plus the five on 2026-09-09. **Six mutants, all killed, each naming the field it broke**, and two of the guide's predictions came out wrong and are recorded as wrong: the `squares[]` mutant did **not** redden perft, and the `phase` one reddened three other binaries rather than none. No mutant is caught by this test alone -- `test_engine`'s `memcmp` sees them all **after unmake**; what this adds is **after make**. Census floors re-derived by `adocs/data/S190_walk_census.py` with python-chess, agreeing exactly. **The Debug self-play line is now traced to observed output**: a default `-log file=...` reads **0** `Assertion` on a run where every game aborted, against **2** at `level=trace engine=true`, so `DEV_MANUAL.md` documents the level and why. The section 7 timing formality resolved nothing and says so -- an A/A of one binary against itself read 1.07x, wider than either A/B reading. Closes `2026-09-04_test_review-F01`
- S187  **every citation from a pending step file into code names a symbol, and the checker refuses a line number** -- **572 converted over 27 files**, from a census of 558 code citations and 14 document ones banked at `2b198f6` in `adocs/data/S187_citations_before.txt` with **52 DRIFT** standing; `--citations` now reads 0 flagged. The mapping is `adocs/data/S187_symbols.tsv` and the generator `adocs/data/S187_symbolise.py`, S144's and S169's shape, because the evidence for a conversion is the mapping and not a green run (DEC-119): 393 WALK, 47 AUTO, 50 TITLE, 37 CHOSEN by hand, 20 HAND read and rewritten, 15 SPAN over a pair with the path repeated, 10 PHRASE at file scope. **Six citations were wrong at the moment they were written**, a class no checker can see because its baseline is the commit that wrote them -- `src/chesso.cpp` line 674 was cited by S099, S132 and S159 for where the per-`go` `search_state_t` is built and had fallen into the book-move helper at all three baselines -- and the four F07 cases come out right. `BOUNDS`, `ANCHOR` and `DRIFT` retire with the line number; `LINE`, `MISSING` and `BARE` replace them, `LINE` found by a text scan because a path preceded by a slash was invisible to the walk and four such citations were hiding in S020 and S117. **The mode joins the fast suite (DEC-159)** as `test_plan_citation_freshness`, 0.45 s against 6.3 s under DRIFT, beside the 18 planted cases of `tests/test_plan_citations.py`. Fifty paragraphs the conversion made vacuous were rewritten rather than left saying "between negamax and negamax". Documents and tooling only: no `src/` change, no `Bench:` line. Closes `2026-09-04_plan_review-F07`
- S148  **the reverse futility depth ceiling stays at 15, and the deep mates it loses are its measured price** -- the ceiling was swept over **every value from 0 to 15** against the 82-row constructed set and the 318-row mined one, a grid no earlier reading had (S145 and S154 both ran six values over 48 rows). The mate in five class is the binding one and it is a cliff: 11 / 9 / 6 / 4 / 1 of 16 at ceilings 0 to 4 and **0 from 5 up**, so the pre-registered rule names **C1 = 4**, where the set reads 52 of 82 exact against 39 at the shipping value, mates in three 18 of 24 against 12 and the deep classes 7 of 16 and 1 of 16 against 1 and 0. **4 lost its SPRT**: `{-5, 0}` nElo at 8+0.08, **H0 accepted at LLR -2.95, nElo -7.31 +/- 5.60, Elo -5.66 +/- 4.33 over 14808 games in 6 h 19 m 35 s, 0 time forfeits** -- so finding those mates costs more rating than it returns and the trade is declined on the games rather than argued (DEC-158). A verdict of "keep the incumbent" is a result and the step completes on it. Two readings the coarse grids could not give: the **plateau starts at 10**, not 15, every count from 10 up being identical; and mate in four is **1 of 16** at the shipping value, not S145's "0 of 8", the set having grown at S168. The run also read the `Incomplete mating PV` class from both sides -- **7 candidate against 13 reference** -- so the class is not this ceiling's, a reading S202 inherits. `test_mate_carry` went red on the candidate and was established as budget calibration before the machine was committed: `S203_case_sweep.sh` shows the short lines appearing and vanishing with the node budget, DEC-122's class and S203's own knife-edge warning. Neither deep class was promoted to an asserted floor -- four ships at 1, the zero-margin floor DEC-116 rejected -- and the five places 15 is written now carry the price beside it
- S184  **the pending documents state the engine as it ships at HEAD, and the params checker now reaches them** -- S115's aspiration sweep is designed on 2 / 21 / 437 with the "keep 5" instruction gone, S082's quiescence cap on 19, S120's, S122's and `plan.md`'s lazy clamp on 184, S118's cost paragraph dated to before S104 with the `specs.md` line quoted beside it, and S127's pre-S085 sentence repaired in S150's tense form (discovered by the extended checker, not by the review). S042's `touches:` names all four sites that build the en-passant key instead of one, and its body states the single rule applied at each -- **X-FEN's pseudo-legal reading, the owner's choice of 2026-09-08**, so its Stockfish comparison uses `en_passant='xfen'`. Five F09 sentences corrected, of which two DEC-144 had already closed and **three are void: S171 completed into `plan_done/` on 2026-09-07/08 and history is never edited**. `tools/plan_prose_check.py --params` now reads the 68 pending files as well as the four documents, and matches the **C++ symbol** with both backticks mandatory -- optional, S114's formula line reads as a claim about its divisor. Red first on exactly four lines, green after, and non-vacuous from the other side: `AspirationDelta` moved to 22 fires `specs.md`, `MANUAL.md` and S115's rewritten sentence. Cost 0.37 s to 1.27 s, once per build. Documents only -- `git diff --stat -- src/` empty, 28/28 in both builds. A header audit corrected the step's own premise (an absent `author:` is the convention, not a defect) and found one fresh instance of F10, S202's missing `done:`. Closes `2026-09-04_plan_review-F05`, `-F08`, `-F09`, `-F10`

## What the 2026-09-05 reorder changed, DEC-144

**The machine-scope lane is gone.** DEC-112 lifted the machine-light steps to
the head of this list while the owner worked from the MacBook, and said that
putting the order back is a decision. It is DEC-144: from here on every step is
assumed to run on the Linux workstation, and the list is sorted for the goal
under the project's rules rather than for a machine. The section this one
replaces is in the history of this file at `66cbc54` and its reasoning is in
DEC-112 if the owner is ever away from the workstation again.

**The head is the workstation's first week.** Two tool bugs first (S178, S173;
BUGS rule, no match owed). Then the two runs three decisions already place
first: S171's census, which DEC-128 says is the first thing taken there, and
S198's harness flags with the 1000-game fixed-rounds A/A that DEC-143 requires
after a machine change and before the next verdict. Between them sit S189 and
S179, agent-only work for the two hours the census holds the machine: the
`bench` signature and `tools/gate.sh`, which DEC-140 binds to every `src/`
commit from S189's completing commit on, and the magic numbers under a project
seed, proved unchanged on node counts.

**Then an instrument lane, interleaved so the machine is never idle.** The
sixteen document and test steps the two 2026-09-04 reviews produced are
ordered by what each removes or provides: S180 and S184 first, because each
removes a hazard an implementer would otherwise follow; S187 next, so every
later edit to a pending file is under the symbol-citation check; the test
steps in the order the test review gave them, S191, S196 and S197 before S109
as DEC-141 requires; S182 before the first verdict lands, so the cost table has
its re-derivation rule from the start; S183 last, after S181 and S185 supply
its inputs. Woven between them are the only three runs that depend on nothing
in the search block -- S148 and S159, one self-contained verdict each, and
S151's longer-control re-test of S085's vector -- so that a single agent
reading this list launches a run and has the next entry to take while it plays.
S151's control prices a `{-5, 0}` pair near 72 hours worst case, which is why
it sits behind the two cheap verdicts and why its pair is the owner's question
before the run is committed to.

**The four blocks follow in the 2026-08-19 order as DEC-133 corrected it**, and
nothing inside them moved: S199's first drift point after the S109 block, S188
after S097, S186 before S134, S152 after S129 as the close of the main order
(DEC-108 -- re-rated once, near the goal). S020 and S030 return to block 2,
where the review put them; on the workstation a behaviour-neutral speed change
owes a `hyperfine` timing and a timing needs the machine quiet, so it is
machine work like a verdict and waits its turn. The reserve and the parked
network close the list as before.

**How to read the list with one machine.** The coordinator holds the machine
(DEC-113) and takes the first Open entry that owns a run. While that run plays,
the next entry that owns no run -- a document, a tool, a test -- may start, in
list order; a change to `src/` waits, because either it alters play and
MEASUREMENT says one at a time, or it is behaviour-neutral and its timing
needs the idle machine. An entry's dependencies are the entries above it, so a
step is never started out of order to fill the machine. The next step is the
first entry, as always.

**The enrichment pass of 2026-09-05, DEC-145.** On the same day the owner asked
that every pending step file be enriched for the agent that will implement it,
one agent per file, sequentially, in this order: the technique as published,
chesso's form, the symbols it touches at HEAD, seeds in DEC-105 form only, the
tests DEC-141 and DEC-142 require, the measurement plan with its pair priced
per DEC-143, and the sources read. It is not a plan step and it enters no
order; `grep -L 'Implementation guide (2026-09-05)' adocs/plan_todo/*.md` names
what it has not reached.

## Open

1. S204  **in progress** -- `test_mate_carry`'s cases stop being pinned to isolated spikes in the node-budget grid, so the guard fires on a mate the engine has stopped finding and not on any change that moves the tree (BUGS, DEC-161)
2. S193  the fast suite's vacuous assertions made falsifiable, the fifty-move boundary pinned, `test_perft` Release-safe, temp-file, date and case-order hazards removed (F04, F05, F09)
3. S191  every null-move, reverse-futility and reduction guard has a direct test with its precondition, and the S165 defender set is a registered fixture -- before S109 (F02, DEC-141)
4. S196  the fault-injection driver becomes `tools/mutation_check.py` over a tracked mutant list; a new search rule ships with a mutant its test kills (DEC-141)
5. S197  `tools/gate_extra.sh` -- Debug binaries, sanitizer build, deep perft, prose and citation checks -- with a terminal marker, and the coverage recipe documented (DEC-141)
6. S192  every golden in `tests/` named and scripted, the piece anchors re-derivable in-repo, the soft-limit scaling asserted on a constructed history (F03, DEC-142)
7. S195  node-limited searches reproducible across `ucinewgame` in a fast test, and `bench` resets the table per position (F08)
8. S194  the UCI book path executed by the fast suite, the weighted draw seeded through `CHESSO_BOOK_SEED` (F06)
9. S151  a change that moves a pruning or reduction parameter has its verdict re-taken at a control at least four times longer before the number is banked, starting with S085's shipped vector
10. S181  every Lynx figure the steps and DEC-087 cite is banded by the CCRL rating of the release it was measured between, from a dated table, and S098's "sub-3000 evidence" grouping is redrawn on it (F02)
11. S185  every published figure the plan argues from carries its URL or the word unverified -- the Ethereal ledger commit and the Stash changelog behind the block order recorded, the mis-attributed zero-weight and tablebase figures corrected (F06)
12. S182  the cost line is priced from the ledger of runs since S105 by effect class -- 4 h 40 m mean per verdict, not 45 to 75 minutes -- and re-derived at every verdict-landing commit (F03, DEC-136)
13. S183  the Elo arithmetic is re-derived from tabled per-step inputs with the ledger's measured transfer ratio as a third discount, and "the midpoint clears 3000" survives only if the number does (F04, DEC-136)
14. S024  history indexed by the move played n plies ago and the current move
15. S109  late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
16. S199  a fixed-rounds drift match against a pinned early-S105 reference after each block boundary, read as a trend -- first point after the S109 block, on the workstation (DEC-108, DEC-139)
17. S091  skip captures the exchange evaluation says lose material, in the main search rather than in quiescence alone, and reduce a negative-SEE move by an extra ply
18. S098  the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
19. S095  reduce a node whose table entry carries no move instead of searching it at full depth
20. S097  extend the one move a verification search says is singular, and take the multicut the same search offers
21. S188  a move that gives check is extended by one ply inside the move loop, bounded by S097's extension plumbing, decided by SPRT -- the in-loop form the retired S096's evidence turned out not to cover (DEC-133)
22. S112  quiescence skips a capture whose best case cannot reach alpha, per move, before the exchange evaluation is consulted
23. S131  quiescence searches non-capture queen promotions instead of filtering them out
24. S022  decide between delta pruning and the per-move futility S112 adds, by measurement -- deleting delta pruning is a valid recorded outcome
25. S113  a shallow verification search over good captures prunes a node whose score is already far above beta
26. S114  the null move reduction scales with how far the static score is above beta, and the base reduction is re-decided
27. S115  the widening schedule is re-swept fail-soft, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply
28. S116  a node whose static score is hopelessly below alpha drops straight to quiescence, at depth one only
29. S132  the soft time limit scales with the share of the root's nodes the best move consumed, spending less when the choice is not in doubt
30. S202  a mate score inherited from the table at a depth too shallow to back it is given a line that reaches it or is not published as a mate -- the class S171's census measured, at a ceiling of 8 lines from 1 search in 3000 games and not a zero, free to alter play under its own SPRT where S171 was not (DEC-150)
31. S020  compute the in-check state once per node instead of once per call site
32. S055  taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
33. S117  the middlegame and endgame halves of every evaluation term travel in one integer instead of two
34. S120  a small cache of full evaluations by position key, so the score behind the lazy shortcut can be paid for once
35. S119  the table becomes cache-line clusters with an aged replacement, a prefetch issued when the key is known, and huge pages
36. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
37. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
38. S030  move_t drops the moving piece and becomes 16 bits
39. S186  the DEC-097 enrichment pass runs over block 3's files before block 3 starts, every figure sourced or marked unverified, every seed in a DEC-105 form (DEC-137)
40. S134  delete rook-on-the-seventh and passer bucket 5 by folding their weights into the piece-square tables, which is bit-exact, and shrink the parameter vector to 823
41. S082  the corpus labels a resolved position rather than the root -- the quiescence leaf, or the leaf reached by playing out a deep search's whole principal variation -- and samples few positions per game rather than many
42. S083  the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, not by a volume target
43. S135  unfreeze the piece placement group and refit it, one bundled SPRT over the three remaining features, by the owner's decision of 2026-08-20
44. S136  unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- at two divisions once S055 has landed -- refit and resolve it at bounds that can
45. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
46. S121  mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on
47. S123  passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
48. S125  backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
49. S118  the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
50. S101  evaluation terms for a piece attacked by a lesser piece, fitted like every other constant
51. S122  king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks and weak squares, and it is no longer clamped
52. S124  the endgame half of the score is scaled toward a draw by what is actually on the board
53. S102  outpost and space terms in the evaluation, fitted like every other constant
54. S133  the piece-square tables become king-relative -- indexed by a king bucket as well as piece and square -- and every entry is fitted
55. S126  every constant in the evaluation is refitted once the search that consumes them has stopped moving
56. S127  an SPSA run over the whole search parameter set as it stands after the search block, and an independent SPRT of what it returns
57. S129  three, four and five man tablebase probing, written from the format description
58. S152  **deferred, DEC-108; closes the main order** — the engine's absolute rating is re-measured once, near the 3000 mark rather than at a block boundary, at both time controls so S128's anchor-spread question is answered by the same run
59. S099  **reserve head, DEC-133** — a static evaluation correction learned from the difference between the static score and what the search returned, keyed on the pawn structure; the probe for the correction-history family, run on a spare night; S110 and S111 gated on its verdict
60. S023  **reserve, DEC-087** — history indexed by piece, target and victim, to order captures MVV-LVA rates equal
61. S025  **reserve, DEC-087** — retry searching losing captures after the quiets, now that capture history exists
62. S110  **reserve, DEC-087** — a second correction table keyed on the non-pawn structure, split by colour
63. S111  **reserve, DEC-087** — correction tables indexed by the move played two and four plies ago
64. S029  **parked, DEC-054** — a perspective network evaluation trained on chesso's own self-play
