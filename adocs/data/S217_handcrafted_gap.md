# The hand-crafted technique gap, S217, 2026-09-12

Read against the CCRL Blitz "Complete list (all engines and versions)",
`https://www.computerchess.org.uk/ccrl/404/rating_list_all.html`, read
2026-09-12, computed by CCRL 2026-09-05 from 2'106'571 games. CCRL does not
publish a separate single-CPU-only page for this list: the 1CPU rating is the
row whose engine name carries no `4CPU`/`8CPU` suffix, in the same combined
list a suffixed row sits in. Method: five of DEC-071's seven named engines,
each at a version with no network, verified against release notes, changelogs
or the author's own dated statement; techniques listed from release notes,
changelogs, the Chess Programming Wiki and forum posts only, never from source
(DEC-016, DEC-105); the technique's CPW name used throughout so rows align
across engines; the intersection against every step chesso ships or has
pending, read from `adocs/specs.md`, `ls adocs/plan_done`, `adocs/plan_todo/`
and `adocs/plan_current/` before this survey started.

## The five engines

| engine | version | 1CPU rating | games | network-free, source |
|---|---|---|---|---|
| Weiss | 1.2 | 3055 | 1195 | no NNUE anywhere in the release notes v0.6 through v2.0 (Aug 2021); Weiss's next known version, 2.1 (2024), is still not reported as NNUE. [W-rel] |
| Texel | 1.07 | 3129 | 2697 | `ChangeLog.txt` line 74: "2023-07-30: Texel 1.09 ... Implement NNUE evaluation." 1.07 is 2017-09-30, six years earlier. [T-chg] |
| Laser | 1.7 | 3291 | 7748 | 1.7 is the `Latest` and final tagged release, no successor; no NNUE anywhere in the release history 0.1 through 1.7. [L-rel] |
| rofChade | 2.3 | 3319 | 4809 | Author's own blog write-up of 3.0: "it took some time to release **the first NN version** of rofChade" (May 2022). 2.3 is April 2020, over two years earlier. [R-nnue] |
| Ethereal | 11.75 | 3344 | 218 | CPW: NNUE first appears as a side release, V12.75 SF-NNUE, October 2020; the free line's first NNUE is 13.00 (June 2021). 11.75 is November 2019. [E-cpw] |

Two more of DEC-071's seven (Defenchess 2.2, 3279; Booot 6.3.1, 3264) were
re-read on the same list for rating only and not surveyed further -- five
engines already exceeds the four this step's `accepts` asks for, and adding
two more author-lineages past that point buys little against the reading
effort.

Source key: `[W-rel]` `https://github.com/TerjeKir/weiss/releases` (v0.6
through v2.0, two pages) -- `[W-cpw]` `https://www.chessprogramming.org/Weiss`
-- `[T-chg]`
`https://raw.githubusercontent.com/peterosterlund2/texel/master/ChangeLog.txt`
-- `[T-cpw]` `https://www.chessprogramming.org/Texel` -- `[L-rel]`
`https://github.com/jeffreyan11/laser-chess-engine/releases` -- `[L-readme]`
`https://github.com/jeffreyan11/laser-chess-engine/blob/master/README.md` --
`[R-cpw]` `https://www.chessprogramming.org/RofChade` -- `[R-talk]`
`https://talkchess.com/forum3/viewtopic.php?t=73719` (Ronald Friederich's own
release post) -- `[R-nnue]`
`https://chessengines.blogspot.com/2022/05/chess-engine-30-mac-linux-rapberry-pi.html`
-- `[E-cpw]` `https://www.chessprogramming.org/Ethereal` -- `[E-rel]`
`https://github.com/AndyGrant/Ethereal/releases/tag/V11.75` -- `[E-dec087]`
Ethereal commit `e755a814`, "Add elo estimates to search steps", 2020-01-22,
cited via `adocs/decisions.md` DEC-087/S185, whose citation this reuses rather
than re-opening the commit page (its diff view is source, DEC-016).

## The technique table

Chesso status is read against the checklist built from `specs.md`,
`plan_done/`, `plan_todo/` and `plan_current/` before this survey (see report).
"Excluded" means a decision already declined the technique, not that this
survey found nothing -- named so the intersection is honest about what was
already decided versus what is newly absent.

| technique (CPW name) | carried by (source) | published figure | band (DEC-087/176) | chesso status |
|---|---|---|---|---|
| Iterative Deepening | all five (foundation) | n/a | foundation | ships (foundation) |
| Principal Variation Search | all five (foundation) | n/a | foundation | ships (S011) |
| Aspiration Windows | all five [W-rel][T-chg][L-rel][R-cpw][E-cpw] | n/a | foundation | ships (S021); refinement pending (S115) |
| Transposition Table | all five (foundation) | n/a | foundation | ships (foundation) |
| Quiescence Search | all five (foundation) | n/a | foundation | ships (foundation) |
| Delta Pruning (quiescence) | Weiss ("dynamic delta margin", v0.7) [W-rel] | n/a | foundation | ships (foundation); S022 decides keep-vs-replace by S112 |
| Static Exchange Evaluation, ordering | all five (foundation + CPW lists) | n/a | foundation | ships (S015 in quiescence; ordering foundation) |
| SEE Pruning, main search | Ethereal (11.75 changelog 11.53, bad-capture skip; CPW "SEE Pruning") [E-rel][E-cpw] | n/a | ~3344 | **pending (S091)** |
| MVV/LVA ordering | all five (CPW lists) | n/a | foundation | ships (foundation) |
| Killer Heuristic | all five (CPW lists, README) | n/a | foundation | ships (foundation + S107, S149, S159/S216) |
| History Heuristic (incl. relative/gravity) | all five; rofChade names it "Relative History Heuristic" [R-cpw] | n/a | foundation | ships (S093) |
| Classic Internal Iterative Deepening | all five [W-cpw][T-chg][L-readme][R-cpw][E-cpw] | n/a | foundation | **related, not shipped as such** -- S095 ships Internal Iterative *Reduction*, a different CPW-named technique for the same missing-hash-move case; no step names classic IID |
| Null Move Pruning | all five | n/a | foundation | ships (S012); eval-scaled reduction pending (S114) |
| Late Move Reductions | all five | n/a | foundation | ships (S013); refinement pending (S098) |
| Late Move Pruning | all five; Texel names it explicitly at 1.00 [T-cpw] | n/a | foundation | pending (S109) |
| Futility Pruning | all five | n/a | foundation | pending (S109) |
| Reverse Futility Pruning (Static Null Move) | Weiss, Texel, Ethereal explicit [W-cpw][T-cpw][E-cpw]; Laser/rofChade bundle it under "futility" without separating it | n/a | foundation | ships (S033, S068, S103) |
| Razoring | all five [W-cpw][T-chg][L-readme][R-cpw][E-cpw] | n/a | foundation | pending (S116) |
| Check Extensions | all five, rofChade gated on SEE>=0 [R-cpw] | n/a | foundation | pre-move-loop form retired (DEC-087a); in-loop form pending (S188) |
| Singular Extensions | Weiss (1.2, named in its own release notes), Texel (1.05), Laser (README) [W-rel][T-cpw][L-readme] | n/a | 3055-3129 (Weiss 1.2, Texel 1.07) | pending (S097, multicut bundled in) |
| Mate Distance Pruning | Weiss [W-cpw] | n/a | ~3055 | **excluded** -- considered and left out, "~0 and no band evidence" (DEC-087 Consequences) |
| Quiescence search of checks | Laser (captures, promotions and checks on the first two plies) [L-readme] | n/a | ~3291 | **excluded** -- same DEC-087 sentence; this survey adds Laser's example, no new Elo evidence |
| Lazy SMP / parallel search | all five [W-rel][T-cpw][L-readme][R-cpw][E-cpw] | n/a | foundation-3344 | **excluded, phase two** (DEC-175); irrelevant to the very ratings above, all read from the 1CPU column |
| Tablebase probing (Syzygy/Gaviota, Fathom/Pyrrhic) | Weiss, Texel, Laser, Ethereal [W-rel][T-cpw][L-readme][E-cpw] | n/a | foundation | pending (S129, 3-5 men) |
| Fail-soft search | Laser (README, explicit) [L-readme] | n/a | ~3291 | pending (part of S115, DEC-087e) |
| PEXT/BMI2 sliding attacks | Weiss, Texel [W-cpw][T-chg] | n/a | foundation | pending (S032) |
| Tapered Evaluation | all five | n/a | foundation | ships (S010) |
| Material | all five | n/a | foundation | ships (S009) |
| Piece-Square Tables | all five | n/a | foundation | ships (S010, refit S028) |
| Bishop Pair | Weiss [W-cpw] | n/a | foundation | ships in code at zero weight (S027); refit pending (S135) |
| Mobility | Weiss, Laser [W-cpw][L-readme] | n/a | foundation | **not yet shipped**, pending (S121) |
| Rook on (Half) Open File | Weiss [W-cpw] | n/a | foundation | ships in code at zero weight (S027); refit pending (S135) |
| Rook on 7th rank | Ethereal [E-cpw] | n/a | foundation | ships in code at zero weight (S027); folded into PSQT, pending (S134) |
| Pawn Structure (isolated/doubled/backward/connected) | all five, most detail from Weiss/Laser/Texel/Ethereal [W-cpw][L-readme][T-chg][E-rel] | n/a | foundation | ships partial (S027, three unnamed terms); completion pending (S125) |
| Passed Pawns | Weiss, Texel, Laser, Ethereal (implied) [W-cpw][T-cpw][L-readme] | n/a | foundation | ships basic (S027); enhanced pending (S123) |
| King Safety | all five, distinct algorithms per engine (Weiss "virtual mobility"; Texel per-side piece bonus + safe queen contact checks; Laser shelter/storm + king-pawn tropism; rofChade generic; Ethereal king-pawn-file proximity) [W-cpw][T-chg][L-readme][R-cpw][E-rel] | n/a | foundation | ships basic, clamped (S027); rebuild pending (S122); shelter/storm storage pending (S118) |
| Threats (attacked by lesser piece) | Laser, Ethereal (latent pawn attacks) [L-readme][T-chg] | n/a | ~3291 | pending (S101) |
| Outposts | Laser, Ethereal [L-readme][E-cpw] | n/a | ~3291-3344 | pending (S102) |
| Automated Tuning (general) | all five, all name or descend from Texel's Tuning Method [W-rel][T-cpw][L-readme][R-cpw][E-cpw] | n/a | foundation | ships (S028) |
| Pawn Hash Table | Ethereal [E-cpw] | n/a | ~3344 | pending (S118) |
| Evaluation Cache | Laser, present 1.1-1.6, **removed at 1.7** per its own release notes [L-rel] | n/a | ~3291 (but absent at the surveyed version) | pending (S120) |
| **Complexity / conversion-chances scaling** | Ethereal (11.59-11.64) [E-rel]; rofChade ("Alayant's solution on endgame complexity", author's own release thread) [R-talk] | unverified -- no Elo figure found anywhere | 3319-3344 (rofChade 2.3, Ethereal 11.75) | **gap** |
| **Fortress Detection** | Texel (KRKB corner at 1.00; 16-pattern detection at 1.05; KQvKRM+pawns at 1.07) [T-chg][T-cpw] | unverified -- no Elo figure found anywhere | ~3129 (Texel 1.07); simpler forms present earlier, those versions not independently re-rated here | **gap** |
| **Castling Ability** (eval term on retained castling rights) | Ethereal (CPW list, no version given) [E-cpw] | unverified -- no Elo figure, no introduction version found | unpinned, <=3344 | **gap, minor** |

DEC-143 worst-case games at the fast-class pair (`{0,10}`, alpha=beta=0.10,
`--fast`): **5828 games, 2.8 h at 2110 games an hour** -- one figure, the same
for every gap row, since it prices the bounds pair and not the technique
(`DEV_MANUAL.md` "What each pair costs"). All three gap rows below carry this
same number rather than a per-row estimate.

## Gap

Three techniques, all evaluation-side, none carried by more than two of the
five surveyed engines, none with a published Elo figure anywhere in the
sources read.

**Complexity / conversion-chances scaling.** Ethereal computes a term that
widens or narrows the evaluation based on how hard the position is to convert
-- more "complex" (asymmetric pawn structure, opposite-side material) reads as
further from a dead draw regardless of the raw material score, so a level but
messy position is not scored as flat as a level, simplified one. RofChade's
author credits the same idea to a contributor ("Alayant... endgame
complexity") independently, which is why it is banded off two engines rather
than one. Proposed outline: *goal* -- an evaluation term that scales the score
by a measure of position complexity or conversion difficulty, computed from
features `evaluate()` already holds (INV-4 forbids a second pass); *accepts* --
(a) the complexity measure is derived over chesso's own data or a formula in
a cited publication, never seeded from Ethereal's or rofChade's own constants
(DEC-105); (b) fitted by chesso's own tuner; (c) an SPRT verdict is recorded,
zero included; (d) `specs.md` gains the term. *Band*: 3319-3344. *Block*: 3
(evaluation), alongside S101/S102.

**Fortress Detection.** Texel recognizes specific drawn material-and-structure
patterns -- a bishop of the wrong colour with a rook pawn cornering the
defending king, particular blocked pawn chains, a few named minor-piece
endings -- and scores them toward a draw independent of the material count a
generic scale-by-material rule would read as won. Proposed outline: *goal* --
recognize a short, named list of drawn patterns from the published record and
score them toward a draw regardless of material; *accepts* -- (a) the pattern
list is drawn from CPW's own Fortress Detection material and cited engine
write-ups, never copied from an engine's own detection code (DEC-016); (b)
each pattern is an independent, tested predicate; (c) an SPRT verdict is
recorded since it changes scores; (d) `specs.md` gains the entry or a recorded
zero. *Band*: ~3129. *Block*: 3, paired after S124 (endgame scaling) so a
specific-pattern override does not fight the general scaling rule over the
same positions.

**Castling Ability.** A small bonus or penalty tied to whether a side has
already lost the right to castle, independent of where the king actually
stands. Weakest of the three: one engine, no version pinned, and evaluation
terms this narrow are usually a fraction of a pawn. Proposed outline: *goal* --
a small evaluation term for retained castling rights; *accepts* -- (a) sourced
from a published description of the concept, not any engine's own weight; (b)
fitted, seeded only per DEC-105; (c) an SPRT verdict recorded, zero an
acceptable and likely outcome given the term's expected size. *Band*: unpinned,
<=3344. *Block*: 3, lowest priority of the three -- a candidate for folding
into S135's bundled piece-placement verdict (DEC-082's precedent for parts too
small to spend their own SPRT on) rather than its own step.

**The 3100-band rule already answers whether these become steps now.** This
step's own `accepts` creates a step automatically only for gap evidence "at or
below the 3100 band." All three gaps found sit *above* that line -- 3129,
3319-3344, and an unpinned figure no lower than Ethereal's 3344 -- so none
qualifies by the rule as written, the same test that already sits S023, S025,
S110 and S111 in the Reserve rather than the main order. The recommendation
this survey hands the coordinator is therefore not "open three steps," it is:
file the three outlines above as Reserve candidates, alongside S023/S025/S110/
S111, for the coordinator's decision to record why -- consistent with, not an
exception to, the rule already written.

## Coverage

The gap is small to the point of being close to empty, and by the step's own
words that is a finding and not a failure. Two of three gap rows are carried
by only one or two of five engines, none carries a published Elo figure, and
all three sit above the 3100 band that would trigger an automatic step. Every
other technique across five independent codebases and four separate authors --
the whole standard search stack, the whole standard pawn/king/piece evaluation
stack, every tablebase and tuning technique -- already has a ships or a pending
row in this plan. Two items this survey did not find a home for in "ships" or
"pending" are not gaps but decisions already on record: Mate Distance Pruning
and quiescence checks were considered and declined (DEC-087), and Lazy SMP is
deferred whole to phase two (DEC-175) and is in any case irrelevant to the
very ratings this table reads, all taken from the 1CPU column.

What "a longer list" could mean instead, for the coordinator: **(a)** extend
the *engine* roster rather than the technique list -- Defenchess 2.2 and Booot
6.3.1 were read for rating only here and were not searched for techniques, and
a handful of other 3000-3350 engines from DEC-071's era exist unread; more
codebases might surface more small terms like the three above, at the cost of
more reading for a shrinking return. **(b)** accept that hand-crafted technique
*coverage* is not the shortfall -- S183's own number is a discount-rate
question (0.38 at its most generous, median 0.00 across this project's own
five published-to-measured transfers), and Weiss reaching 3055 on a 301-line
evaluation with the full pruning stack suggests execution and tuning quality
per technique may matter as much as technique count -- so the lever worth
pulling is re-examining the transfer ratio once S024 and S109 land and give it
a larger sample, not adding more rows to this table. **(c)** re-price the
order rather than extend it: block 3's own ledger (Stash) already prices its
largest items in the 20-25 Elo class, and a longer list of small,
unquantified terms like the three found here is unlikely to move that
picture regardless of how many more engines are read.

## The extended list against S183

`adocs/data/S183_elo_inputs.md`'s rule: one headline published figure per
step, discounted x0.60 (list), x0.80 (interaction), x0.38 (transfer, the
project's own largest measured published-to-measured ratio); unverified
figures are summed and also given without them; a step with no published
figure at all contributes nothing and is stated rather than filled in.

Applied to the three gap rows the same way: **all three carry no published
figure at all** -- not even an unverified number, just documented presence.
Per S183's own selection rule ("where a step carries no published figure at
all, that is stated rather than filled in"), **each contributes zero to the
sum**. The extended list therefore adds nothing to S183's arithmetic, and the
landing points are unchanged:

| reading | gain | lands at |
|---|---|---|
| S183 reconstruction, all three discounts | +99.4 | 2658 |
| S183 reconstruction + this step's three gap rows | +99.4 + 0 | **2658**, unchanged |
| plan.md's own range, two discounts | +390 to +680 | 2949 to 3239 |
| plan.md's own range with the third discount | +148 to +258 | **2707 to 2817**, unchanged |
| plan.md's range + this step's three gap rows | +148 to +258 + 0 | **2707 to 2817**, unchanged |

Against the 2559 +/- 25 soft anchor and the 3000 mark: the extended list lands
at the same **2658** midpoint reconstruction and the same **2707 to 2817**
range as S183 already recorded, still 183 to 342 Elo short at the high end.
DEC-179's premise -- extend the hand-crafted list to close the shortfall -- is
not borne out by what this survey actually found: the standard technique
catalog was already almost entirely captured by the 40-plus steps already
pending before this survey started, and the handful of genuinely new items
carry no quotable Elo at all. This is the finding the Coverage section states
plainly rather than dressing up as a recommendation.

## Skipped, showed source

None. Every page read was a CCRL rating list, a GitHub Releases page, a raw
`ChangeLog.txt`, a Chess Programming Wiki page, or a TalkChess/blog post -- no
source file, table, network or training data was opened. The one commit this
file cites for content (Ethereal `e755a814`) is reused from
`adocs/decisions.md` DEC-087/S185's existing citation rather than re-opened.

## Not verified

- Complexity / conversion-chances scaling: no Elo figure found for either
  engine that carries it.
- Fortress Detection: no Elo figure found for any of its three Texel
  additions (1.00, 1.05, 1.07).
- Castling Ability: no Elo figure and no introduction version found; presence
  is Ethereal's current CPW feature list only.
- Defenchess 2.2 and Booot 6.3.1: rating and version re-read from the CCRL
  list only (3279/1484 games and 3264/442 games respectively); their
  network-free status and technique lists were not verified in this pass.
