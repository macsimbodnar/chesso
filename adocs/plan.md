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
jump, +436, was four search features and no evaluation change -- the 2.0
release also credits "a significant increase of nodes searched per second", so
the +436 is not four features in isolation
(https://github.com/lithander/Leorik/releases/tag/2.0; the CCRL delta is
2538 - 2102 on the list of 2026-08-28). The second pass added a third
independent line: Ethereal's own feature-removal ledger prices its history at
**-759.05 +/- 57.40**, its LMR at **-248.59 +/- 10.48** and its quiet-pruning
family at **-175.08 +/- 7.24**, against **-31.95 +/- 3.21** for beta pruning --
all at 12.0+0.12, one thread, 8 MB, in Ethereal commit `e755a814`, "Add elo
estimates to search steps", 2020-01-22
(https://github.com/AndyGrant/Ethereal/commit/e755a8140fba). **That ledger
prices search steps only**: no evaluation term appears in it, and the clause
"single digits for most evaluation terms" that stood here until 2026-09-11 had
no source anywhere -- its single-digit rows are ProbCut -9.08, counter-move
pruning -8.10 and futility -3.13, which are search steps too (S185, F06).
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
seventh, and tempo. Published fits of the same terms were quoted here as
+16.7, +4.2, +9.2 and +12.99 until 2026-09-11, and **three of those four
numbers do not survive being looked up** (S185, F06; the four came into S100
from a source that was never recorded):

- **+12.99 is Weiss pull request #241, "Tempo", 2020-04-11** --
  +12.99 +/- 7.35 at 10+0.1 and +6.67 +/- 4.67 at 40+0.4
  (https://github.com/TerjeKir/weiss/pull/241). So it prices **tempo**, which is
  one of the five terms, and not rook on the seventh, which is what S100's
  mapping had it against.
- **+16.7 bishop pair: unverified.** No source searched gives it. The nearest
  fetched figures are Weiss pull request #95, "Tune Eval 1", a CLOP of bishop
  pair and king-vulnerability together, +16.10 +/- 8.74 at 10+0.1
  (https://github.com/TerjeKir/weiss/pull/95), and Lynx pull request #390,
  tapered bishop pair, +8.2 +/- 6.1 and +9.5 +/- 6.7
  (https://github.com/lynx-chess/Lynx/pull/390).
- **+4.2 rook on an open file and +9.2 rook on a half-open file: unverified,
  and no source gives a half-open-file figure on its own.** The nearest fetched
  figure is Weiss pull request #231, "Tune Rook+Queen" -- rook and queen
  piece-square values together with the open and semi-open file bonuses --
  +9.86 +/- 5.74 and +8.40 +/- 5.42
  (https://github.com/TerjeKir/weiss/pull/231).

What was searched for the three: Weiss pull requests for "bishop pair",
"seventh" and "open file", Berserk pull requests for "bishop pair", and the
Stash changelog, which carries only the *removal* of knight-pair and rook-pair
bonuses. The figures stay quoted as unverified rather than deleted, because the
pattern they were cited for -- five terms at exactly zero -- is S100's finding
and does not rest on them; S186 resolves them or records what it searched
(DEC-137). `passed_pawn_mg` is the fourth data point: `{0, -6, -4, 19, 59,
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
Stash ledger prices them -- `mhouppin/stash-bot`'s `CHANGELOG.md`
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md), entry by
entry: mobility area and curves (S121, **v27's mobility zone excluding rammed
and low-rank pawns, +19.95 +/- 9.63** at 10+0.1), passed
pawns with king distance (S123, **v32's king proximity in the passed-pawn
evaluation, +22.27 +/- 9.86** at 8+0.08, the largest single entry), the connected
and phalanx pawn work (S125, **v31's connected pawns, phalanx and defender,
+25.38 +/- 10.40** at 8+0.08 and +18.57 +/- 8.45 at 40+0.4), the pawn hash that
makes them affordable (S118), threats (S101, **v26's dynamic initiative from
pieces threatened by lower-valued ones, +10.13 +/- 6.50** at 10+0.1 and
+7.77 +/- 5.23 at 60+0.6 -- Stash's term is an *initiative* term built on
threatened pieces, which is a wider thing than S101's goal), king safety (S122 — biggest
cumulative payoff and the documented failure magnet, so it follows the corpus
work), endgame scaling (S124), outposts and space (S102), the king-relative
tables (S133), and one full refit (S126), because every weight fitted before
S109 was fitted against a tree that no longer exists.

**Block 4, close, S127 and S129.** The full SPSA run over everything the search
block added, and Syzygy last and optional, because the no-copy rule prices 13 to 25
Elo at a from-scratch implementation of a compressed table format -- **both of
those endpoints are six-men figures** (CPW *Syzygy Bases*: Stockfish 10dev
classical, 10+0.1, all WDL in RAM, +13; Topple "about 25 elo from syzygy 6
piece tablebases", talkchess t=70110) where S129 is three to five men, for
which the only figure found is Minic's **0** at three-man; S129 carries the
correction (the
MIT-licensed Fathom route is on the record in DEC-087 if the owner ever wants
it).

**Reserve, after the mark or on spare nights: S023, S025, S110, S111.** **Candidates without a step, from S217's inventory (DEC-192, 2026-09-12): complexity or conversion-chances scaling (Ethereal, rofChade), fortress detection (Texel) and a castling-ability term (Ethereal) -- three evaluation terms the 3000-to-3344 hand-crafted engines carried and this plan lacks, none with a published figure, all above the 3100 band; `adocs/data/S217_handcrafted_gap.md` has the outlines, and one becomes a step when a figure appears or the owner says so.** Kept
whole with their evidence; they are 3100-band techniques by the record.

## What this costs

The pending order owes **roughly 45 to 55 SPRT verdicts** once multi-verdict
steps are counted honestly (S024 two, S098 three, S097 and S022 two each, the
evaluation groups per-term), plus two SPSA nights, one to three datagen nights,
and S152's two five-hour gauntlets at the end.

**Struck 2026-09-11 by S182** (`2026-09-04_plan_review-F03`, DEC-136):
~~at the S105 settings a typical verdict is 45 to 75 minutes, so roughly 75 to
110 machine-hours~~. **A verdict costs
what this project's own ledger says it costs, and the ledger is four to nine
times that estimate.**

### The ledger: every SPRT verdict since the S105 regime

Read from each step's own completion stamp, not from a run log. All eight ran
at 8+0.08, Hash 16, UHO, `model=normalized`.

| run | what it measured | wall | games | bounds | verdict |
|---|---|---|---|---|---|
| S093 v1 | history malus and gravity | 2 h 44 m | 6412 | `{0, 5}` | H1 |
| S093 v2 | history persistence across `go` | 6 h 35 m | 15398 | `{0, 5}` | H0 |
| S107 | killers for checking quiets | 1 h 37 m 52 s | 3812 | `{-5, 0}` | H1 |
| S149 | killer slot dedupe | 1 h 05 m | 2522 | `{-5, 5}` | H0 |
| S108 | static evaluation at every node | 5 h 26 m 38 s | 12774 | `{-5, 0}` | H1 |
| S165 | null-move mate-band guard | 7 h 58 m 07 s | 18598 | `{-5, 0}` | H1 |
| S130 | table score as stand pat | 7 h 12 m | 16784 | `{0, 5}` | **no verdict** |
| S148 | reverse futility depth ceiling | 6 h 19 m 35 s | 14808 | `{-5, 0}` | H0 |
| S207 | a repetition before the root is not a draw | 4 h 26 m 56 s | 10258 | `{-5, 0}` | H1 |

**Mean 4 h 49 m, median 5 h 27 m** over the nine; 101366 games in 43.42 hours,
**2334.6 games an hour** across the set. Eight of the nine sit between 2328 and
2346 — including S148, at 2341, the first played on the workstation. **S207 is
the one outlier at 2305.7 and the reason is known rather than guessed**: the
coordinator wrote four document steps on the same machine for the first
forty minutes of it (DEC-172's lane), which cost about 1.2 % of throughput,
three minutes on a four-and-a-half-hour run. So the throughput is a constant
here to within what the document lane costs, and the wall time is the game
count. **Two of nine landed inside the struck 45-to-75-minute window** and
neither was a strength verdict.

### Priced by class, which is what the spread is

The eight split cleanly in two and the split is not luck — it is where the truth
sat relative to the bounds:

- **Fast class, an effect outside the interval**: S149 1 h 05 m, S107
  1 h 38 m, S093 v1 2 h 44 m, S207 4 h 27 m. **Mean 2 h 28 m.** S207 is what
  widened this class and it is worth reading: its effect was outside the
  interval but *close to the near bound* at +4.47 nElo, where the other three
  were far outside, and it took 10258 games against their 2522 to 6412. **"Far
  outside" and "just outside" are not the same price**, and the formula below
  is what separates them — 10258 is under half the 25591 its pair costs with
  the truth sitting *on* a bound, which is the case S207 nearly was.
- **Slow class, an effect inside the interval or a true zero**: S108 5 h 27 m,
  S148 6 h 20 m, S093 v2 6 h 35 m, S130 7 h 12 m, S165 7 h 58 m.
  **Mean 6 h 42 m.** Three of these five ran on a null or a small negative,
  which is what DEC-063 says a `{0, 5}` or `{-5, 0}` pair does to a true zero:
  it runs to the wall.

**The pending list is almost all slow class.** This plan prices exactly one
effect in the block class — S109's +40 to +120 — and two more steps carry a
sourced figure above +20 at a comparable band, S024 (+44.68 / +33.95, Weiss
#477) and S126 (S028 measured +188.74 here). Everything else is priced in
single digits to +25, which is inside or beside the interval. So:

    3 fast-class verdicts    x 2 h 28 m  =    7.4 h
    42 to 52 slow-class      x 6 h 42 m  =  281 to 348 h
    --------------------------------------------------
    total                                =  289 to 356 machine-hours

against the struck 75 to 110. The flat mean gives **217 to 265 hours**
(4 h 49 m x 45 to 55) and that is the **floor**, not the estimate: the ledger's
fast runs are three of eight where the pending list's fast-class effects are
three of about fifty. Add the two SPSA nights, the one to three datagen nights
and S152's two gauntlets on top, none of which is in either figure.

### What the formula says before a run, DEC-143

The ledger prices the *expected* run; the nElo run-length formula
(`adocs/testing_strategy.md` section 1.1) prices the *worst* one, and a
pre-registration states it before the first game. Both columns at the same
game counts:

| pair | truth at the midpoint | truth on a bound | at 2277 g/h (workstation) |
|---|---|---|---|
| `{-5, 5}`, alpha=beta=0.05 | 10465 games, 4.5 h | 6398 games, 2.7 h | 4.6 h / 2.8 h |
| `{0, 5}` or `{-5, 0}`, alpha=beta=0.05 | 41861 games, 17.9 h | 25591 games, 10.9 h | **18.4 h / 11.2 h** |
| `{0, 10}`, alpha=beta=0.10 (`--fast`) | 5828 games worst case, 2.5 h | | 2.6 h |

The hour figures in the middle two columns are at the ledger's 2337; the last
column is at S198's measured 2277 on this machine. **The slow-class mean of
6 h 42 m is about 15660 games, below even the on-a-bound case** — because five
of the nine runs hit a bound rather than sitting at the midpoint, and S207
reached H1 in 10258. A step that
budgets on the mean and gets the midpoint waits three times as long, which is
the reason DEC-143 asks for the worst case in writing.

**Re-derived, not restated (DEC-136).** This section is re-derived in the
completing commit of every step that lands a verdict, exactly as `status.md` is
rewritten: the table above grows by one row and the four figures — mean,
median, the two class means — move with it. A verdict that lands without moving
these numbers is a missed edit.

Read the count as verdicts owed, not as verdicts that will land. Six steps stop
owing a match at all under DEC-083.

**The Elo arithmetic is a range and not a forecast, and DEC-019 is why.**
Discounting self-play to list Elo at the ratio the published per-release records
support (~60 % sticks), and taking a fifth off for interaction: search +180 to
+280, evaluation +90 to +160, speed +40 to +90, tuning +50 to +90, and S133 is
+30 to +60 of margin on top — **+390 to +680, and those five ranges are not
reproducible from the figures this plan's own files carry.**
~~The midpoint clears 3000.~~ **Deleted 2026-09-11 by S183**
(`2026-09-04_plan_review-F04`, DEC-136). `adocs/data/S183_elo_inputs.md` is the
re-derivation: every pending step's published figure, its source URL or the
word unverified, and the per-block sums, with the selection rule and the
discount rule both written down before any sum was computed.

**The third discount, and the rule was fixed before the number.** This project
has eight published-to-measured transfers on its own record; five have a figure
on both ends, and of those five **two are exactly zero, one is the wrong sign,
one is 0.10 and one is 0.33** — mean 0.061, median 0.00. The headline discount
is **0.38, the largest ratio ever measured here** (S093 v1's +10.73 against
Lynx's +28.0), chosen because it is the most generous reading of the record
that is still a measurement; the mean and median are reported beside it rather
than averaged in, since a mean over a set containing wrong signs is not a
ratio.

**What the arithmetic says now.** This plan's own +390 to +680, with the third
discount, is **+148 to +258 and lands at 2707 to 2817**. The reconstruction
from recorded inputs is lower still: +544.9 raw published, +261.6 after the two
old discounts, **+99.4 after all three, landing at 2658**. **S217's extended list (2026-09-12) adds nothing: the three gap terms carry no published figure and contribute zero under S183's own rule, so the extended list lands at the same 2658 and 2707 to 2817 -- DEC-192, and the question of what a longer list means is the owner's.** Search reproduces
almost exactly (+184.6 against the quoted +180 to +280); evaluation, speed and
tuning do not, and **tuning's +50 to +90 has no published input behind it at
all** — its only evidence is S028's +188.74, a one-time move from hand-picked
constants to fitted ones that by construction cannot happen twice.

**So the high end does not clear 3000 either, and this is the question DEC-071
exists for.** 2817 is 183 short; adding the whole 121.8 Elo of anchor
disagreement in the favourable direction reaches 2939 and is still short.
Whether the goal is met by this list, by a longer one, or only with the network
premise DEC-054 parked, is the owner's to weigh: S183 changed no step's order
and no step's content on this result, which its `excludes:` forbids for exactly
this reason. **Answered by the owner on 2026-09-11, DEC-179: the goal stands,
without a network, and the list is extended -- S217 inventories what the
3000-to-3130 hand-crafted engines carried that this plan lacks, and the network
comes after the mark.** **Re-derived when S024 and S109 land** — +44.68 and +204, 46 % of
the raw sum between them — with the measured figure replacing the published one
and the transfer ratio re-taken over the enlarged ledger, in each of those
steps' completing commits.

Two things were not promised by the old sentence either, and both still
stand: the 2559 anchor is soft by 121.8 Elo of internal disagreement
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
not a measurement, and 2559 +/-25 soft stays the engine's rating. **S183's
reconstruction agrees with that answer from the other direction**: summing the
pending list's recorded published figures and applying all three discounts
gives 2658, against the 2600 to 2650 this paragraph reached by summing what had
already been kept. Two independent arithmetics, one forward and one backward,
landing 8 to 58 Elo apart.

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

**And, since 2026-09-11, one number more: a step that lands an SPRT verdict
re-derives "What this costs" in the same commit** (DEC-136, S182). The ledger
table there gains the run's row — step, what it measured, wall time, games,
bounds, verdict, all read from the completion stamp — and the four figures
above it move: the mean, the median, and the fast- and slow-class means. The
figure it replaced stood for three weeks while eight runs said something else,
which is the failure this rule exists to prevent. Same enforcement as the rest
of this section: the diff.

(An empty list item sat here until 2026-09-11 and was removed by S182; it
carried nothing.)

## Done recently

- S217  **the hand-crafted gap is inventoried and it is nearly empty** -- five DEC-071 engines at their last network-free version (Weiss 1.2, Texel 1.07, Laser 1.7, rofChade 2.3, Ethereal 11.75; 3055 to 3344 on the CCRL Blitz list read 2026-09-12), every technique sourced from prose and no source file opened; everything they carried has a `ships` or `pending` row here except three evaluation terms (complexity scaling, fortress detection, castling ability), none with a published figure, all above the 3100 band, so **no step is opened** and the three are Reserve candidates, DEC-192. S183 re-derived over the extended list: **unchanged at 2658 and 2707 to 2817**; DEC-179's premise is not borne out and what a longer list means is put to the owner. Documents only
- S216  **the S159 killer-slot census can no longer measure the wrong position in silence** -- `adocs/data/S216_census_run.py` fails a refused `position fen` row and exits 1, observed red first against S159's reader on HEAD (the refused row inherited the previous board's 452 nodes and a move illegal on its own board) and byte-identical on legal input; the pre-DEC-177 `promo-mess` row is replaced by the legal `KILLER_POS` as `promo-mess-s208` in a new positions file, DEC-186; the recorded census is **not re-derived** -- it pins `99000c1`, before the load bound, so it measured a board that loaded, and one row of eleven cannot move DEC-160's three ratios. The Tier-1 gap (a guard on the reason's truthiness) closed, shown red then green on a stub; verification over S159's set at HEAD refuses exactly the one row, exit 1, 2.75 s. Gate green both builds. Documents and one instrument only -- `No functional change`
- S219  **the harness book is re-decided by measurement and stays balanced on a tie** -- the pre-registered comparison (12000 games, four CC0 books, one doubling of time, pick by M = nElo^2 x games/h) put `noob_3moves.epd` first at 154.1M against the incumbent `UHO_Lichess_4852_v1.epd`'s 123.8M, clear by more than one combined error; the reader agrees with fastchess to the digit on all eight matches; `fastchess.sh` switched, DEC-189. The DEC-143 A/A on it -- 1000 games, 0 forfeits, **2110 games an hour** under `powersave`, pair variance **0.2905 +/- 0.0184** against S198's 0.2430 -- read 1.29x the cost under the step's variance/throughput rule and recommended the revert; combined with the comparison's measured 1.13x score response it reads **0.97 to 1.01 +/- 0.12**, a tie, kept balanced by the step's own tie rule, **DEC-190**, which amends the rule and resets the band. The zip digest pin waits for the owner; `rating.sh` keeps `8moves_v3.pgn`; two losing books deleted. `test_fastchess_script.sh`'s stub followed the rename, red then green; gate green in both builds. Harness and documents only -- `No functional change`
- S215  **the one line S207's rule hangs its boundary on is read by the suite at last, and both of its neighbours die on it** -- `search()`'s `state->root_history_size = game->history.size` was pinned by nothing: mutated to `history.size - 1` the whole fast suite stayed green while `bench` moved **13 %**. One case, **one board searched three times through `search()`** and never through `negamax()` with the field set by hand: a forced perpetual (one legal reply to every check, no capture in the cycle, Stockfish 0 at depth 22) at **depth 4** with nothing like the root behind it answers the material, **-909**, because the cycle returns to the root's *own* entry; the same history at **depth 5** answers **0**, the ply-1 position returning strictly inside the tree; and the same board with the cycle played *before* the root answers **0** at the depth that answered the material -- same position by hash, `history.size` 1 against 4. **All four wrong boundaries observed red**, each on its own assertion: `- 1` and `0` on the first, `+ 1` and the deleted assignment on the second, and under each the only failing assertion in 33 binaries is this case's. **M39 and M40 killed, 2 of 2**, from a worktree at the commit carrying them. `search_bench` and `bench` identical -- no `src/` file was touched. Tests and mutants only
- S209  **`setoption` follows the protocol on case, `Hash` says so when it refuses, and `clean-tt` no longer clears the table under a running search** -- the option name and a check option's value are folded once before every comparison, the search parameter names included, so `hash`, `HASH`, `ownbook`, `True` and `rfpmargin` are their canonical spellings; `Hash` is parsed with `std::from_chars` requiring the whole token and answers `not an integer` or `out of range` on the UCI channel in **both** builds, leaving the table as it was; `command_clean_TT` joins the search first. **Every red observed on `013600d`**: `hash` and `HASH` left the table at 524288 entries, `ownbook value True` ran a real search where the canonical spelling plays a book move, **`0x40` bought 1 MB and `64abc` bought 64 MB in silence**, `clean-tt` returned with `go infinite` still running, and the tune build called three casings of `RfpMargin` unknown options. **The race is measured**: a hand-built TSan tree, `go infinite` + 40 `clean-tt`, **38 / 41 / 36 reports on `013600d` against 0 / 0 / 0 on the candidate**, plus the one-line A/B inside the same build tree -- and the first run of all printed **0 on the defective tree** because ASLR killed the sanitizer before `main`, which is why `setarch -R` and a `FATAL:` guard are now in `TOOLCHAIN.md`. Five red-first cases, all five observed red; the golden `Rfpmargin` assertion **re-stated, not relaxed** (`RfpMargn`), and a new surface case pins that no two advertised names collide when folded. DEC-178 records the three choices the accepts left open. `search_bench` node- and best-move-identical (121530 / 801481 / 72924), `bench` 30046849 unchanged. `No functional change`

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

## What the 2026-09-10 audit inserted, and the 2026-09-11 reorder for the goal

The fourth adversarial audit (`adocs/audit/2026-09-10_adversarial.md`: five
high, eleven medium, twenty-one low; every re-derivable number in `specs.md`
reproduced at HEAD, move generation, make/unmake, hashing and the table's
semantics found clean under fuzzing, the SPRT and the SPSA correctly built)
was asked to review the engine against the project's goals and restrictions,
so its axes were the two founding rules. What it found on each, and where
each answer lives -- DEC-170 is the digest:

**Nothing is copied -- three breaches, one step.** Three tables in
`src/bb_tables.hpp` are byte-identical to a GPL-3.0 tutorial engine's, with
trailing whitespace on the same seven rows to prove it, from the same
`bitboard`-branch commit whose magics S179 already regenerated for exactly
this reason (F01); five mask builders are transliterations of the same source
and the source author's handle is in a shipped header (F02); twelve pieces of
third-party artwork carry no licence while `books/fetch_book.sh` says the
repository bundles nothing unlicensed (F03). **S211** derives the tables from
the engine's own geometry, rewrites the builders, renames the positions and
licenses the art, bench-identical.

**Nothing is believed without a measurement -- the instrument, one step.**
Both harnesses run **one-sided** resign adjudication while `rating.sh`'s
comment claims two-sided; 84 % of the games behind the 2559 ended on an
engine's own score, 19.6 % of those decisive adjudications one-sided, and
chesso conceded alone 311 times to its opponents' 203 (F04). The engine's
`id name` carries no version, so the identity check the project enforces on
every opponent it cannot enforce on itself (F05); a cached reference is reused
with nothing checking its worktree or configuration (F06); `specs.md` claimed
`./rating.sh` re-derives 2559 and it cannot (F07, the sentence corrected the
day the report was read). **S212** fixes all of it and closes with the A/A
DEC-143 requires; DEC-174 is the adjudication ruling.

**Three defects the engine ships, three steps.** A two-fold repetition whose
first occurrence is before the root scores as a dead draw when no draw exists,
in **ordinary play**, and a fast-suite test pins it as intended (F08) --
**S207**, one `--nonreg` SPRT, DEC-173 the convention and the test's
re-statement. `position fen` with an impossible piece count overruns the
270-entry move buffer and aborts the shipping binary (F09), and a back-rank
pawn indexes the passed-pawn table off its end, read and write (F10) --
**S208**, a third refused class beside S161's two. `setoption` is
case-sensitive against the spec, `Hash` reads `0x40` as 1 MB silently, and
`clean-tt` clears the table under a running search (F11 to F13) -- **S209**.
The seven low engine findings are one batch, **S210**; the comments and dead
API another, **S213**; the three tool findings a third, **S214**.

**The goal, and the distance to it (Part D).** No measured Elo since S093 on
2026-08-22; the first strength step sat seventh behind six document and test
entries (F14). No record anywhere of parallel search (F15) -- DEC-175 places
it in phase two with the one constraint that binds now. The whole
non-material evaluation is clamped to `+/-184` and S039, which sizes the
clamp, sat four steps before its consumer S122 (F16) -- S039 now sits
directly before S122.

### The reorder, DEC-172

The owner's instruction of 2026-09-11: prioritise the 3000 mark within the
project's constraints; bugs matter, Elo matters more, **unless the bug is
reachable in play or can move an evaluation, a UCI answer or a line** -- and
DEC-171 writes that scope into the BUGS rule rather than deviating from it
silently. So the list opens with the one defect that fires in ordinary play
and owes a run of 4 to 18 hours, S207, which is the night run of 2026-09-11;
the four document steps of the instrument lane -- S182 and S183, which F14
named as the two that would turn "the midpoint clears 3000" into a number,
and S185 and S181, which feed them -- are its filler, owning
no run and touching no `src/`; the two bug steps the owner's criterion selects,
S208 and S209, are the next morning's node-identical work; then S024, the
largest ordering gain surveyed, with S214 as its filler; then S211, S151 and
S212; then S109 and S199's first drift point, and the boundary closes S210,
S213 and S194 before S091 opens the rest of the search block in DEC-133's
order. Blocks 2, 3 and 4 stand, S039 excepted.

**S151 is a 3.4-hour estimate and not a three-day verdict.** Its section 7
priced three designs; DEC-172 takes (iii): S085's vector against `3488506` in
a fixed 1000-pair match at `32+0.32`, `Hash=64`, read as an estimate with its
interval -- under DEC-155's four-hour line, so a daytime run while the agent
writes the next step -- and the standing rule it writes is the block-boundary
form, one such reading beside each S199 drift point, which is the only scope
that meets its own budget sentence.

**Two clauses join the reading rule above** ("How to read the list with one machine"). A run that ends while the owner
is away is completed and the next run-owning entry taken without waiting; the
list is the authority and "next" is a convenience. Between the verdicts of a
multi-verdict step, a `src/` entry that is behaviour-neutral on node counts may
land, since it cannot contaminate the second verdict's attribution, and a
play-altering one may not. Every run's pre-registration names, by finding id,
the known defects DEC-171 has scoped behind it.

## What the 2026-09-11 review of the parked list decided, DEC-179 to DEC-184

The owner went through every parked question that had waited on them, on
2026-09-11, each presented with options and the agent's recommendation. Six
rulings, all by the owner:

- **The goal stands** at 3000 CCRL Blitz without a network -- DEC-071 and
  DEC-054 unamended -- reached by a longer hand-crafted list, and the network
  comes after the mark, as the engines DEC-071 lists did. **S217** is the
  sourced inventory of what those engines carried that this plan lacks, and
  the coordinator turns its gap into steps. DEC-179. The agent had recommended
  un-parking S029 after the search block; the owner overruled it, and the Elo
  paragraph above says so where it asked the question.
- **S109 ships the published late-move-pruning form**; the gives-check
  exemption is **S218**, its own step and its own SPRT directly behind it, and
  it folds into S109 if S109's mate guard turns out to need it. DEC-180.
- **F02 and F03 of the 2026-09-04 audit fold into S210**, the low-defect batch
  that already carries F01. DEC-181.
- **The harness opening book is re-decided.** **S219**: the agent surveys the
  open-licence books, the owner downloads the pick, its digests are pinned, and
  one DEC-143 A/A read against S198's -- variance, games an hour, draw rate --
  decides whether it stays. Balanced preferred where the numbers are close.
  DEC-182.
- **The self-play corpus stays gitignored and is archived** off the repository
  with its recipe and digests. DEC-183.
- **Seven hygiene items** fold into S210, S213 and S214 or are accepted, S194's
  two deferred questions are answered, and the enrichment pass continues as
  filler under the delegation. DEC-184.

The order changes by three insertions and nothing else moves: S217 and S219
behind S024 as its filler -- S219 waits on the owner's download and blocks
nothing; a verdict ready before it runs on the current book and says so -- and
S218 directly behind S109.

## What the S219 night found, DEC-187

The first match of S219's book comparison printed 236 fastchess warnings,
"PV continues after threefold repetition", in 1500 games. A subagent replayed
all of them with python-chess: every one is a genuine threefold, and the
engine misses it because `make_move` puts the en passant square into the
Zobrist key on every double pawn push, capturable or not, so the oldest
occurrence hashes differently and the repetition compare skips it. 52 of the
236 publish a non-zero score for a drawn line; a four-ply reproduction scores
the same position -313 one way and 0 the other. That is S042, planned in
block 4 as an efficiency item on a triage that was off by one, and under
DEC-171 it is the bug that is fixed before anything else starts: **S042 is
Open entry 1**, re-scoped as a fix with a red-first case, a `Bench:` line, the
Debug self-play and one `--nonreg` SPRT. S219's comparison stands -- one
binary on both sides -- and its A/A runs before S042's SPRT.

## What S219's two measurements said, DEC-189 and DEC-190

The pre-registered comparison ranked `noob_3moves.epd` first by
M = nElo^2 x games per hour -- 0.80 of the incumbent's hours per verdict --
and the harness switched to it (DEC-189). The DEC-143 A/A that followed read
the other way under the step's own cost rule: pair variance 0.2905 against
0.2430, 2110 games an hour against 2277, 1.29 times the cost. The two disagree
because they measure different things. The comparison measures signal and
noise at a large strength difference; the A/A measures noise alone at none;
and the `accepts:` priced a verdict as if the score a real strength difference
produces did not depend on the book, which the comparison measured to be
false -- 1.13 times on the balanced book. Read on one footing, the balanced
book costs 0.97 to 1.01 +/- 0.12 of the old book's hours per verdict on nElo
bounds if the balanced book's score advantage holds near zero, where SPRTs
run -- and 1.29 if it does not. DEC-190 read that as a tie; the fast check over
its commit showed the advantage was measured only at the doubling, and
**DEC-191 corrects the reading to the bracket 0.97 to 1.29**, keeps the book
provisionally on the owner's leaning, and creates **S220**: the same two books
at a quarter handicap, one night, to measure the trend toward zero and decide
by a rule written first. S220 is Open entry 2, behind S042's SPRT. Budget from
2110 games an hour until a run under `performance` re-measures it.

## Open

1. S042  **BUGS first, DEC-187** -- the en passant square enters the Zobrist key on every double push, so a threefold whose oldest occurrence follows a double push is missed: 236 of 1500 self-play games, 52 published scores wrong, a 313 cp four-ply reproduction; set it only when an enemy pawn can take it, in every place the key is built, red-first case, `Bench:`, Debug self-play, one `--nonreg` SPRT
2. S220  **gated on the owner, DEC-193** -- the balanced book's cost per verdict near zero measured at a quarter time handicap, `noob_3moves.epd` against `UHO_Lichess_4852_v1.epd`; priced honestly, two nights for a 2.3-sigma answer on which end of DEC-191's 0.97-to-1.29 bracket is true and nothing finer; the free alternative is the ledger of realized games per verdict on the new book; **does not start without the owner's word, and S024 is the next night run after S042**
3. S024  history indexed by the move played n plies ago and the current move
4. S214  `analyse_game.py` refuses to return a score it never read, `spsa_driver.py check` proves every axis reaches the search, and a fit's provenance stamp records every flag that selects the emitted vector; no `src/` (F28, F29, F35)
5. S211  the last three tables and five mask builders inherited from a GPL-3.0 tutorial engine replaced by the project's own derivations, the tutorial author's handle out of the shipped source, the artwork licensed; bench-identical (F01, F02, F03)
6. S151  S085's shipped vector measured against `3488506` at `32+0.32`, `Hash=64`, in a fixed 1000-pair match read as an estimate -- design (iii), about 3.4 h, a daytime run -- and the longer-control rule written in its block-boundary form (DEC-172)
7. S212  resign adjudication two-sided in both harnesses as the comment claims, `id name` stamped with the build and checked by `fastchess.sh`, a cached reference checked before it is played, a crash voids a run, the busy guard measures load, then one fixed-rounds A/A (F04, F05, F06, F07, F31, F32, DEC-174)
8. S109  late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
9. S218  late move pruning bought the gives-check exemption through a post-make prune, decided by its own SPRT against S109's shipped form; folds into S109 if S109's mate guard needs it (DEC-180)
10. S199  a fixed-rounds drift match against a pinned early-S105 reference after each block boundary, read as a trend -- first point after the S109 block, on the workstation (DEC-108, DEC-139)
11. S210  the seven low engine defects of the 2026-09-10 audit as one batch -- clock wrap, full history, `go infinite`, `movestogo 0`, the unstoppable first iteration, dead positions in quiescence, the bishops comment -- and the still-open `2026-09-04_adversarial-F01`; an SPRT only if F22's census finds reach (F17 to F23, DEC-171)
12. S213  the evaluation header's stale zero-weight blocks, the mate-band argument that names the wrong bound, two dead public entry points and two `<cctype>` calls on a signed `char`, with a test for the bound the argument relied on; `No functional change` (F26, F27, F33)
13. S194  the UCI book path executed by the fast suite, the weighted draw seeded through `CHESSO_BOOK_SEED` (F06)
14. S091  skip captures the exchange evaluation says lose material, in the main search rather than in quiescence alone, and reduce a negative-SEE move by an extra ply
15. S098  the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
16. S095  reduce a node whose table entry carries no move instead of searching it at full depth
17. S097  extend the one move a verification search says is singular, and take the multicut the same search offers
18. S188  a move that gives check is extended by one ply inside the move loop, bounded by S097's extension plumbing, decided by SPRT -- the in-loop form the retired S096's evidence turned out not to cover (DEC-133)
19. S112  quiescence skips a capture whose best case cannot reach alpha, per move, before the exchange evaluation is consulted
20. S131  quiescence searches non-capture queen promotions instead of filtering them out
21. S022  decide between delta pruning and the per-move futility S112 adds, by measurement -- deleting delta pruning is a valid recorded outcome
22. S113  a shallow verification search over good captures prunes a node whose score is already far above beta
23. S114  the null move reduction scales with how far the static score is above beta, and the base reduction is re-decided
24. S115  the widening schedule is re-swept fail-soft, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply
25. S116  a node whose static score is hopelessly below alpha drops straight to quiescence, at depth one only
26. S132  the soft time limit scales with the share of the root's nodes the best move consumed, spending less when the choice is not in doubt
27. S202  a mate score inherited from the table at a depth too shallow to back it is given a line that reaches it or is not published as a mate -- the class S171's census measured, at a ceiling of 8 lines from 1 search in 3000 games and not a zero, free to alter play under its own SPRT where S171 was not (DEC-150)
28. S020  compute the in-check state once per node instead of once per call site
29. S055  taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
30. S117  the middlegame and endgame halves of every evaluation term travel in one integer instead of two
31. S120  a small cache of full evaluations by position key, so the score behind the lazy shortcut can be paid for once
32. S119  the table becomes cache-line clusters with an aged replacement, a prefetch issued when the key is known, and huge pages
33. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
34. S030  move_t drops the moving piece and becomes 16 bits
35. S186  the DEC-097 enrichment pass runs over block 3's files before block 3 starts, every figure sourced or marked unverified, every seed in a DEC-105 form (DEC-137)
36. S134  delete rook-on-the-seventh and passer bucket 5 by folding their weights into the piece-square tables, which is bit-exact, and shrink the parameter vector to 823
37. S082  the corpus labels a resolved position rather than the root -- the quiescence leaf, or the leaf reached by playing out a deep search's whole principal variation -- and samples few positions per game rather than many
38. S083  the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, not by a volume target
39. S135  unfreeze the piece placement group and refit it, one bundled SPRT over the three remaining features, by the owner's decision of 2026-08-20
40. S136  unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- at two divisions once S055 has landed -- refit and resolve it at bounds that can
41. S121  mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on
42. S123  passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
43. S125  backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
44. S118  the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
45. S101  evaluation terms for a piece attacked by a lesser piece, fitted like every other constant
46. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today -- moved to sit directly before S122, its consumer, because four steps above change the sum it clamps (F16, DEC-172); `eval_spread`'s candidates include the shipping value (F25)
47. S122  king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks and weak squares, and it is no longer clamped
48. S124  the endgame half of the score is scaled toward a draw by what is actually on the board
49. S102  outpost and space terms in the evaluation, fitted like every other constant
50. S133  the piece-square tables become king-relative -- indexed by a king bucket as well as piece and square -- and every entry is fitted
51. S126  every constant in the evaluation is refitted once the search that consumes them has stopped moving
52. S127  an SPSA run over the whole search parameter set as it stands after the search block, and an independent SPRT of what it returns
53. S129  three, four and five man tablebase probing, written from the format description
54. S152  **deferred, DEC-108; closes the main order** — the engine's absolute rating is re-measured once, near the 3000 mark rather than at a block boundary, at both time controls so S128's anchor-spread question is answered by the same run
55. S099  **reserve head, DEC-133** — a static evaluation correction learned from the difference between the static score and what the search returned, keyed on the pawn structure; the probe for the correction-history family, run on a spare night; S110 and S111 gated on its verdict
56. S023  **reserve, DEC-087** — history indexed by piece, target and victim, to order captures MVV-LVA rates equal
57. S025  **reserve, DEC-087** — retry searching losing captures after the quiets, now that capture history exists
58. S110  **reserve, DEC-087** — a second correction table keyed on the non-pawn structure, split by colour
59. S111  **reserve, DEC-087** — correction tables indexed by the move played two and four plies ago
60. S029  **parked, DEC-054** — a perspective network evaluation trained on chesso's own self-play
