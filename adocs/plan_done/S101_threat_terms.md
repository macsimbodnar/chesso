id:         S101
goal:       evaluation terms for a piece attacked by a lesser piece, for an attacked piece nobody defends, and for a safe pawn push that would attack a piece, fitted like every other constant
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the terms are fitted with every other constant frozen and the held-out error reported before and after, which is how every constant in this engine was fitted; the term is computed in a stage that has the attack bitboards it needs, looked up once and never a second time, so the unions it builds are one OR a piece in stage two's own loop and the shortcut's nodes pay nothing for them (DEC-333 (1)); INV-5 holds -- mirroring a position agrees rather than negates, asserted in tests/test_evaluation.cpp; the step states which stage the terms land in and why (S039 read H0 and kept the lazy bound, record `ca578de`, so the bound must still hold by construction: stage two, under the clamp, DEC-333 (1)); the trace, `term_layout()` and the walk gain the features and tools/tuner_groups.hpp a group whose partition properties still hold; each threat kind is screened before the family's one SPRT (DEC-259); the fast suite green
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_trace.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp, tools/tuner_trace.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tools/eval_spread.cpp, tests/test_evaluation.cpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_eval_positions.hpp, tests/test_walked_counts.hpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_mate_pv.cpp, tests/test_mate_breadth.cpp, tests/test_mate_carry.cpp, .tuning/verify_fit.py, adocs/data/S192_anchors.py, adocs/data/S125_coverage_rows.cpp (scoped), adocs/data/S170_cases.tsv, adocs/data/S101_*, DEV_MANUAL.md, MANUAL.md, adocs/data/README.md; MANUAL.md read and unchanged at phase 1, its `RfpMinPly` row's figures at phase 2 and its floor sentence at the close
excludes:   outposts and space, which are S102; king safety, which exists and is fitted, and whose squares and checks are S122's (DEC-333 (9)); overload threats, which have no owner
decisions:  DEC-071, DEC-258, DEC-259, DEC-333, DEC-334, DEC-335, DEC-342, DEC-343, DEC-324, DEC-332, DEC-315, DEC-316, DEC-326, DEC-329, DEC-330, DEC-331, DEC-221, DEC-134, DEC-084, DEC-142, DEC-143, DEC-047, DEC-266
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-01 20:36 -- H1 at {0, 5} nElo, 8+0.08, Hash 16, cand 4e34e61+7d6f96ec896c vs ref 4e34e61: Elo +33.87 +/- 12.33, nElo +46.57 +/- 16.84, LLR 2.95, 1636 games, 0 h 45 m, 0 forfeits (adocs/data/S101_sprt.log, adocs/data/S101_candidate.diff, archived as chesso_S101_sprt_2026-11-01.tar.xz); the banner as pinned, both binaries at their bench (5482277, 5707449), candidate.diff the tree played, no crash or disconnect, quarters +46.1, +20.4, +39.2, +30.6, kernel journal clean; no warning of any kind, 0 incomplete mating PVs; so by the pre-registered H1 row the family ships -- all seven parts, the screen's 1234567 fit (DEC-259, DEC-333), d +0.000051, +0.000025, +0.000047, +0.000006, +0.000014, +0.000008 and +0.000053, each past its margin -- and no bisection; the stopping Elo is not the effect size (DEC-063). EXACT_FLOOR 159 -> 157 by the owner (DEC-343), one below the 158 shipped and 3 above the 154 weakened, the gate at RfpMinPly 1 with 157 compiled in red at 154 (adocs/data/S101_mined_floor_157.log); MATE_IN_THREE_FLOOR holds, 12 and 10; E's ceiling 2 -> 3 and S170's budgets A 100000, D 1000000, F 100000 by their scripts. Stage two, summed into mobility's pair under the clamp, 0.400 % of S083's rows against 1.0 %; every family weight fitted on S083's corpus from zero with every other constant frozen (DEC-084, no seed), held-out 0.071321 -> 0.071117, monotonicity not imposed; every definition implemented from its description, CPW's pages and DEC-333 (DEC-221), and no engine's source was read; the mirror test written and seen red before any line that scores; cost all +4.68 %, p7 +1.06 %, p1 to p6 zero (adocs/data/S101_cost_reading.txt); under src/ the tree played plus comments, the Release binary byte-identical to it (adocs/data/S101_close_identity.txt); the fast check's eight findings each fixed or answered, no pick moved; the far and negative cells listed for S126. Fast suite 50/50 in both builds, gate_extra 6/6 in 1525 s, the clang pair clean inside it; Tier 2 does not bind. Implemented by Opus 5.5 subagents per phase, fast-checked; the runs the coordinator's; inspected and recorded by one Opus 5.5 subagent.

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

**Scope gained 2026-10-08 (DEC-258):** hanging pieces -- a piece attacked
and not defended, by type, and an undefended piece attacked by the enemy
king -- and pawn-push threats, a safe pawn push that would attack a piece
(Ethereal's release notes V10.55, V11.00 and V11.25, titles with no figure; CPW
*Hanging Piece*; Stash's changelog, v26, +10.13 for its threat-based initiative term). DEC-138 had
deferred them beyond this step's original goal; S217 had counted Ethereal's
pawn-push threats as covered here while this file excluded them. The
sections below that call them out of scope are superseded on that point.

Each term -- and each part this file used to give a verdict of its own --
is first fitted with every other constant frozen and kept only if it lowers
held-out loss on S082's fixed validation set by more than the margin the
pre-registration states; a term that fails is recorded with its figures and
does not enter the match. The survivors ship under **one** `{0, 5}` nElo SPRT
for the family, with DEC-143's worst case and abort rule pre-registered. On
H0 or no verdict the family is bisected along a partition written in the
pre-registration, each part its own SPRT. The tuner's mirror of a term is
gone: S276 deleted it, and S262's trace, `term_layout()` and the walk in
tests/test_walked_counts.hpp hold a term now, with `test_eval_model` reading
engine against walk. S263 built no per-side attack set, since no term read one;
a term here that needs one builds it once per evaluation. S039 read H0 (record `ca578de`,
revert `74d0adf`): the lazy shortcut and the clamp are kept, and DEC-324, the
owner's amendment S122's design waited on, leaves `LAZY_EVAL_MARGIN` S122's to
re-derive.

## The hazard is where it is computed, not what it scores

A term that reads where the other side stands cannot be accumulated, so what it
must be is shared. Rebuilding the tables from the bitboards was 25 % of nodes per
second before S014 removed it, and quiescence calls `evaluate_lazy()` at every
node, whose stage one is paid everywhere and whose stage two is skipped on 51.4 %
of those calls on `bench` (the research pass below). S027's pawn terms landed in the
cheap stage because the lazy clamp would truncate a passer; a threat term has the same
question to answer and answers it in the step, with the answer measured: stage two,
DEC-333 (1).

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki has no page for this technique.** `https://www.chessprogramming.org/Threat`
and `https://www.chessprogramming.org/Threats` both return HTTP 404 (fetched
2026-09-13). The nearest page is CPW *Hanging Piece*
(https://www.chessprogramming.org/Hanging_Piece, fetched 2026-09-13), which
defines a hanging piece as "an attacked piece not defended by own man exposed
to capture" and says loose pieces and undefended pawns "may be considered in
evaluation, specially if there are more than one or even two per side, and the
opponent has forces and possibilities for double attacks and knight forks".
**Absent from that page: any treatment of a piece attacked by a *lesser*
piece, which is this step's whole goal; any pawn-push threat; and every
centipawn and Elo figure.**

So the form check's answer is: **the definition this step implements comes from
the engine record and not from the wiki**, and that is stated rather than
implied. It is read for form only.

**The one traced figure for the group is Stash's, and it is a wider term than
this step's.** `mhouppin/stash-bot`'s `CHANGELOG.md`
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md), v26: dynamic
initiative from pieces threatened by lower-valued ones, **+10.13 +/- 6.50** at
10.0+0.1s and **+7.77 +/- 5.23** at 60.0+0.6s. The 2026-09-04 literature check
already recorded the mismatch and `adocs/plan.md` states it at the site:
Stash's is an *initiative* term **built on** threatened pieces, which is
broader than "a piece attacked by a lesser piece". The figure is quoted for
order of magnitude and for direction, never as this step's expectation
(DEC-019).

**What the record has that this step's goal excludes.** The 2026-09-04 check's
part B3 inventories the threat family the surveyed engines carry: a minor
attacked by a pawn, a minor attacked by a minor, a major attacked by a minor,
hanging pieces, pawn-push threats and overloaded pieces. This step's goal was
the first three, recorded there as "partial"; DEC-258 (7) brought hanging and
pawn-push threats in, DEC-333 (2) keeps a minor on a minor as its own part though
the two are one rank and so not "lesser", and overloaded pieces stay out, with
no owner.

**No located source prices the narrow term alone**, at any band. Searched this
pass: CPW *Threat* and *Threats* (both 404), CPW *Hanging Piece*,
`mhouppin/stash-bot`'s `CHANGELOG.md`. Searched 2026-09-04: the same family in
part B3. The step's verdict is its own SPRT and no ordering argument here
rests on a published number.

### 2. Shape for chesso

The term needs, for each side, the set of squares attacked by each enemy piece
class. No stage holds those unions: the loop that computes mobility
(`src/evaluation.cpp` `evaluate_mobility_and_king_safety`) looks each piece's
attacks up once and keeps them no further, and its comment said the first term that
reads a union builds it there. It holds `pawn_attacks[2]`, S121's, the input a
pawn-threat term needs, and `zone[2]`, which serves as the king's attacks; its
mobility `area` is no input here.

**Departure, stated:** the surveyed engines fold their threat terms into a
larger initiative or threat block scored by a table; this step scores a
smaller, per-class set of weights fitted like every other constant, because
the tuner's model has to stay linear in the parameters
(`tools/tuner_trace.hpp`). That is a narrower form than the record's and it is
the form that keeps the gradient closed.

### 3. Implementation sketch

- The step states which stage it lands in and why, and the accepts requires the
  answer to be measured, not argued: if it lands in the expensive stage the
  lazy bound still holds by construction because the clamp is inside
  `src/evaluation.cpp` `expensive_score`; if it lands in the cheap stage it
  escapes the clamp and is paid at every quiescence node. DEC-333 (1): the
  expensive stage, by the research pass's figures.
- `term_layout()` and the trace gain the feature and `tools/tuner_groups.hpp` a
  group, with the partition properties in `tests/test_tuner_groups.cpp` green.
- INV-5 is asserted in `tests/test_evaluation.cpp`: mirroring a position agrees
  rather than negates.

### 4. Constants and seeds

**No seeds.** Every weight is fitted with every other constant frozen and the
held-out error reported before and after, which is the accepts. Stash's +10.13
is an Elo measurement, not a constant. No engine's threat table seeds anything
here wherever it is republished (DEC-105, DEC-134), and the wiki offers no
value to take even if one were wanted.

The one declared number is the **feature count** -- how many attacker/victim
class pairs the term carries -- and it is **(b) derived**: the pairs follow
from the piece classes chesso's evaluation already distinguishes, and the step
states the enumeration rather than choosing a width: nine strictly lesser pairs, two
minor on minor, and 24 cells in all (src/evaluation.hpp, DEC-333 (2)).

### 5. Pitfalls

- **Where it is computed, not what it scores.** Looking attack sets up a second
  time is the class of cost S014 removed. The term builds its unions from the
  attacks the existing loop already looked up, or it does not ship.
- **The stage decision changes the lazy margin's input.** A term in the
  expensive stage enters the sum the clamp bounds at `LAZY_EVAL_MARGIN`, kept
  by S039's H0 (record `ca578de`); a term in the cheap stage does not. The
  choice decides whether the clamp can truncate the term, and the step says
  which stage it takes.
- **The band is not this engine's.** Stash's +10.13 is at a much higher rating
  than chesso's, and DEC-019's three recorded cases are exactly reported
  figures that did not transfer. Bounds are chosen for a term that may measure
  zero.
- **A zero verdict is a legitimate outcome** and is recorded as zero (DEC-019,
  and S005, S006 and S015 are the precedents).

### 6. Measurement

One SPRT against a named commit, `{0, 5}` at 8+0.08, Hash 16, `noob_3moves.epd`,
bounds stated in advance with the nElo worst case and the abort rule (DEC-143,
adocs/data/S101_sprt.sh). Held-out error before and after the fit, reported. The
cost recorded, forced-weight by adocs/data/S101_cost.sh, because the answer to
"which stage" is a speed question as much as a score question.

### 7. Interactions

- **S121 (done, H1)**: built `pawn_attacks[2]` and the mobility area, and no union.
  This step builds the unions, in the same loop.
- **S039 (done, H0, record `ca578de`)**: the clamp on the sum this term may
  enter is kept. DEC-172 had moved S039 behind this step to size the clamp on
  that sum; DEC-258 re-formed it into retiring the shortcut and the clamp,
  which read H0.
- **S102 (separate)**: outposts and space, the other two reuse-the-fills terms.
- **S122 (after)**: king safety counts attacks too, and the two divide by target
  (DEC-333 (9)): S101 owns attacks on men, S122 squares and the king: zone
  incidences, weak squares, defenders, safe checks, a pawn's among them. A man in
  its own king's zone scores in both: 7.99 % of lesser victims and 2.90 % of push
  victims, and no hanging man, its king defending it. S122 re-derives its margin
  with threats inside the sum.
- **S126 (block end)**: refits everything.

### 8. References

- - https://www.chessprogramming.org/Hanging_Piece -- "an attacked piece not
  defended by own man exposed to capture"; loose pieces "may be considered in
  evaluation"; **no piece-attacked-by-a-lesser-piece treatment, no pawn-push
  threat, no figure**. Fetched 2026-09-13.
- - `https://www.chessprogramming.org/Threat` and
  `https://www.chessprogramming.org/Threats` -- **HTTP 404, both**. Fetched
  2026-09-13. The technique has no wiki page.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- v26
  dynamic initiative from pieces threatened by lower-valued ones,
  +10.13 +/- 6.50 at 10.0+0.1s and +7.77 +/- 5.23 at 60.0+0.6s. Changelog
  entries only.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` part B3 -- the
  threat family inventory and the "partial" verdict on this step's scope.

## 2026-11-01: the research pass

Read at `4d3edf6`: counts and instructions only, no match, fit, tuner or timing. A private
`git archive`, bench 5707449 reproduced. Every figure's source is adocs/data/S101_census.txt,
by adocs/data/S101_census.cpp, S101_census_zone.cpp and S101_research_prototype.patch; the
census reads the FEN field alone, and the instruction rows are `perf_counters` with the
cycles and wall unread. The rulings on it are DEC-333.

**What exists.** `expensive_score` clamps `evaluate_mobility_and_king_safety<false>` to
+/-`LAZY_EVAL_MARGIN`, 184; `evaluate_expensive` is the pure wrapper. Stage two's locals:
`zone[2]`, `pawn_attacks[2]`, `pinned[2]`, `king_square[2]`, per-colour `area`, per-piece
`attacks` and `moves`. No union by class and no side total. `pawn_entry_t` holds both pawn
bitboards, `passers`, `score`, both file bytes and 16 spare bits: no room for attack sets
and no need of them, two shifts a side rebuilding each, and stage two does not receive the
entry. So every set a threat term reads costs: pawns and king nothing, the four unions one
OR a piece, `nonking` four ORs, `all` one; after that AND, POPCNT, a multiply and an add a
cell, per side. No S101 term can sit in the entry: each reads a piece.

**The spread, S083's corpus.** Binds on 22083 of 6226944, 0.355 %, S121's figure exactly.
|stage two| p50 30, p90 82, p99 153, p99.9 222. Within 25 of the margin 0.804 %, 50 1.806 %,
100 9.207 %. S082's qs set: 0.278 %. |White minus Black count|, all kinds: p90 2, p99 3,
p99.9 4, max 8.

| clamp share at a flat hypothetical weight, a probe, no seed | w=20 | w=40 | w=60 | w=80 |
|---|---|---|---|---|
| all kinds | 0.401 | 0.810 | 2.142 | 4.673 |
| lesser 9 | 0.362 | 0.411 | 0.521 | 0.741 |
| hanging 5 | 0.370 | 0.497 | 0.858 | 1.667 |
| push | 0.366 | 0.460 | 0.724 | 1.352 |
| king-only | 0.353 | 0.354 | 0.358 | 0.372 |

**Calls, `bench`.** 716329 `evaluate` + 2246909 lazy = 2963238, S118's probe count. Stage
two runs on 1807797, S118's slot probes over 2: the shortcut skips 39.0 % overall and 51.4 %
of lazy calls. One stage-two call is 743 instructions (run twice, +11.54 %).

| the prototype's instructions on `bench`; `bench 16` within 0.03 | naive | gated on an empty set |
|---|---|---|
| lesser 9 | +2.59 % | +1.90 % |
| hanging 5 | +1.81 % | +1.93 % |
| king-only 4 | +1.77 % | +1.75 % |
| push 4 | +2.61 % | +2.73 % |
| those four kinds | +7.66 %, 493 a call | +5.35 %, 344 a call |

Minor on minor +1.18 % naive; all kinds naive +7.90 %. Not additive, the sets being shared.
In stage one with the sets rebuilt at every call +19.1 %; with the loop hoisted about +16 to
+20 %, inferred; the four pawn cells alone at every call +1.91 %. For comparison S121 +3.22 %,
S125 `cand` +3.50 %, S118 -9.0 %.

**The transfer gap.** The corpus is quiescence leaves. A row or a call:

| kind | corpus, attacker on move | a row | search's lazy calls, on move | a call |
|---|---|---|---|---|
| lesser 9 | 8.0 % | 0.114 | 54.2 % | 0.446 |
| hanging N B R Q | 8.6 % | 0.121 | 56.2 % | 0.578 |
| hanging pawn | 25.8 % | 0.236 | 50.9 % | 0.652 |
| push | 53.8 % | 0.377 | 51.2 % | 0.651 |

**Rows with a non-zero coefficient, %.** P>N 2.96, P>B 1.56, P>R 0.42, P>Q 0.53, N>R 1.27,
N>Q 0.92, B>R 1.91, B>Q 0.75, R>Q 0.86; B>N 14.56, N>B 4.04. Hanging P 18.04, N 3.04, B 3.55, R
3.08, Q 1.99. King-only P 0.84, N 0.45, B 0.20, R 0.67, Q **0 of 6226944**; king and others 1.00,
0.52, 0.24, 0.68. Push, safe by reading (2), single and double: N 15.57, B 8.64, R 4.42, Q 4.89.
Reading (1), nothing enemy attacks the target, DEC-329 (2)'s word: **B and Q 0 rows**, the
victim attacking the target itself. Reading (0), no enemy pawn: 17.16, 14.03, 5.28, 8.57. Single
pushes alone lose 20 to 30 % of rows.

**Overlaps.** Hanging rows also attacked by a lesser piece: Q 72 %, R 55 %, N 20 %, B 19 %. A
victim with two lesser attacker classes: under 1 %. Push with hanging: 0.1 to 0.4 % of rows.
Victims in their own king's zone: lesser 7.99 %, push 2.90 %, hanging 0 of 2218608. On the
fixture's 92 rows, 8 of 24 cells were reached neither way or one way. No S192 anchor holds a man
any cell counts: 0 on all twelve.

**Sources, DEC-221.** CPW *Hanging Piece*, *Loose Piece*, *Pawn Pushes (Bitboards)*, as
src/evaluation.hpp quotes; *Attack and Defend Maps* has no lesser or hanging treatment.
Ethereal's release notes, titles with no figure: 10.31 "Evaluate threats by safe pawn pushes",
10.79 "Extend ThreatByPawnPush to understand double moves and enpass threats", 11.03 "Penalty for
minor threats against our minors". Stash v26 verified, +10.13 and +7.77, an initiative term. "Safe"
has no source. The 2026-09-19 study names threats only as a history index. No engine's source was
read.

## 2026-11-01: phase 1

One Opus 5.5 subagent, coordinated; worktree `../chesso-s101`, branch `s101` on `b0338b5`. The
family landed at zero, `No functional change`; the screen, the cost run and the SPRT
pre-registered; nothing fitted, timed or played. Every definition implemented from its
description (DEC-221); no engine's source was read.

### The design as built, DEC-333

- **`threat_terms<collect>()`**, called from `evaluate_mobility_and_king_safety` after the piece
  loop, adds into a `threat_sum` beside `mobility_sum`, and the two share mobility's one taper:
  N stays 3, no new site. `EVAL_TRACE` per cell, `SITE_MOBILITY` in `term_layout()`, so
  `exact_score()`, the smooth score and `gradient()` take the term and the clamp with no line of
  their own. The trace records a cell only when its set is non-empty; a zero coefficient adds
  nothing and `sum_record()` drops zeros anyway: 36.49 -> 37.37 entries a row over the corpus.
- **Seven arrays, 48 parameters, `PARAM_COUNT` 1073 -> 1121 (built: 1121)**: `threat_pawn` 4,
  `threat_piece` 5, `threat_minor` 2, `hanging_piece` 4, `hanging_pawn` 1, `threat_king` 4,
  `threat_push` 4, blocks `TPW_`, `TPC_`, `TMN_`, `HGP_`, `HGW_`, `TKG_`, `TPU_`, an end constant
  and a group each, `THREAT_PUSH_END == PARAM_COUNT` asserted; no earlier index moved. Each
  group compiled out at zero; `threat_unions_live` compiles out the ORs in the piece loop, and P1
  alone needs none.
- **The separate column (DEC-333 (10)).** `evaluate_expensive_terms()` takes `threats`: the taper of
  the pair less the taper of mobility alone, so mobility stays S121's and the three add up digit
  for digit. `tools/eval_spread` keeps its table's three columns, "combined" now the three, and adds
  a short table of the threats alone, so `S121_screen.py`'s reader is unmoved.
  `test_evaluation` "the unclamped terms are the engine's own" reads the three: the signature made
  it, and it is stricter by a term.
- **The packed bound, per array:** `threat_family_half()` beside mobility's 32 x largest:
  `pawn_cells_reach()` where a man has one cell, `threat_piece_reach()` for a rook in two and a
  queen in three, 8 and not 15 for the king's. `S101_screen.py headroom` repeats it per fit: 3808 of
  32767 at zero, and it refuses a planted 2000.
- **Comments replaced in the loop touched**: the union sentence over the fused function, and the
  packed sums'. No other.

### The symmetry test first (DEC-329 (6))

Written and observed before any line that scores: the sets and counts first collected, the score
and trace lines added after both cases were seen red and green. `test_evaluation` "evaluation:
threat family symmetry", over `threat_mirror_population()`, all_test_fens() and seeded placements
with pieces. **Green, 29846 positions: the rarest set and edge file held on 291, the rarest cell
non-zero on 535, 2274363 assertions.** **Red** on each planted one-colour defect, each reverted
(`cmp` identical) and green again:

| plant | sets | counts |
|---|---|---|
| Black's double push on its sixth, not its fifth | red, `push_targets` | red, `threat_push` cell 1 |
| Black's push shifted the wrong way | red, `push_targets` | red, `threat_push` cell 0 |
| Black's king not in its defence | red, `all` | red, `hanging_pawn` |
| Black's pawns not in its defence | red, `nonking` | red, `hanging_piece` cell 0 |
| Black's reach shifted the wrong way | red, `push_hits` | red, `threat_push` cell 0 |
| Black's double push keeps the en passant square | red, `push_targets` | red, `threat_push` cell 2 |
| Black's king-only cell off by one | green: the set is right | red, `threat_king` cell 0 |

### Against the walk and by hand

`walked_counts::threat_family_features()`, every ray followed to the first man, no magic table: every
set bit for bit and every cell, 12047 positions, the rarest cell non-zero on 121. Thirty hand cases,
each as written and as its mirror, every cell reached by each colour and each sign, five with no cells
at all holding what would make a count happen and the one clause that stops it: a pawn defended by a
pawn and by a king, a king's victim defended, a push onto a pawn-attacked square, a double push with its
en passant square attacked. They passed on first run; the walk and the engine were written from the
same definitions, and the mirror's reds above are what says the code can fail.

### INV-6

`adocs/data/S101_inv6.txt`, against `b0338b5`, Release and tune: `bench` 5707449, `bench 12` 1861982,
`tools/search_bench.py` depth 9 c3d5 37187 / d5e6 105585 / d7c8q 56111 and depth 12 c3d5 147063 /
d5e6 343485 / d7c8q 192032, every stream byte-identical; every function of `evaluation.cpp.o` the
search reaches identical once the padding is dropped (`adocs/data/S101_objdiff.py`), `expensive_score()`
526 instructions, `evaluate_cheap()` 456; only the collecting instantiation and its accessors differ,
and the comparison reads red on the all-forced cost build, 526 -> 890.

### Coverage

`adocs/data/S125_coverage_rows.cpp` scoped: its features, span and terms already ended at S125's, and its
landed block now ends at S101's marker; its rule, picks and block unchanged. With S083's corpus named,
S125's, S123's and S121's each read "reproduces: yes", 2, 4 and 19 pairs, 0 ineligible, 0 one-way.
`adocs/data/S101_coverage_rows.cpp`, base the 92 rows: 14 of 48 needs open (`threat_pawn[0]`, `threat_minor[0]`,
`hanging_piece[3]`, all of `threat_king`, both halves); `all_test_fens()` closes all fourteen in 6 pairs, so
neither the corpus nor the fallback is read, and the run with the corpus is byte-identical. Fixture 92 -> 104,
all twelve VALID by python-chess; gradient smallest 2.500e-08, over the family 9.580e-08; 0 one-way;
"reproduces: yes".

### The preconditions, each by its script (`adocs/data/S101_preconditions.txt`)

- Fixture clear of the clamp: 121.1 against 183.0, unmoved.
- The clamp case's push, `S083_tuner_preconditions.cpp clamp`: **+30 still** by its rule: four rows bound, the
  farthest 197.3, the nearest other 158.8, every row 13.25 from the kink, finite differences 1.53e-7.
- The gauge walk: smallest **0.9618** under Adam and **1.2480** under AdaGrad, both the bishop's; floor 0.01;
  holds at most 8.22e-15. Up from S125's close, 0.0882 and 0.1165: the rows move it, and a fit can take it back.
- The gradient case's worst: **4.98e-7 at parameter 990, `passed_king_own_eg[5]`**, against 1e-6, a factor of 2.0
  (S125's close: 9.22e-7 at 800). No DEC-316 rows owed. The clamp case 1.53e-7; the synthetic group 3.46e-8.
- **Planted recoveries: all seven groups added**, each within half a centipawn fitted alone from the shipped
  constants. `threat_king` runs 32000 epochs, its pawn cell's one middlegame row standing at phase 2: 0.70 at
  6000 and 2.39 at 24000 against 3 planted. A budget and not a golden. None unmeasured.

### The audit, run once at phase 1

`tools/feature_audit --free threat_pawn,threat_piece,threat_minor,hanging_piece,hanging_pawn,threat_king,threat_push`
over S083's corpus, 2026-11-01 03:25:21 to 03:26:27 (66 s), after `S101_screen.sh`'s rules and `S101_screen.py` were
written and before any fit: rank exactly **1074 of 1121**, nullity 47 -- the 32 dead pawn columns, S134's seven and
S121's eight levels held by the freeze -- and **0 further exact dependencies**, no family column zero, as DEC-333
expected. `S101_screen.py audit-check` passes. **Ties: 0 of 48**; the thinnest cell is `threat_king_eg[2]`, 12347
rows and 9149 games, against 1000. After the audit `S101_screen.sh` took this reading and the entries figure, and
`S101_screen.py` `landing-check --commit` for the SPRT's guard; neither moves a rule. The screen's own `audit` stage
runs it again and writes the ties.

### Guards, seen in a scratch clone

- `landing-check` passes on the scratch landing, refuses a planted `threat_king_eg[2]` and the worktree's HEAD.
- `S101_sprt.sh` refuses while unpinned. Its guards name each commit by what they check, and a dry copy over 26
  scenarios runs 7 and refuses 19 (`adocs/data/S101_sprt_guards.txt`): **a tests-only commit, S307's shape, either
  side of the pair trips nothing**; S308's shape, a comment under `src/`, is refused, as DEC-333 (11) wants until the
  close; a moved mobility weight, code after a `};`, a wrapped definition, `CMakeLists.txt` or `src/evaluation.hpp`
  touched, a screened commit not at zero, a `screened` the screen's stamp does not name or no stamp, REF off the line,
  CAND no child of REF, and each part's wrong array.
- `S101_cost.sh check` passes, and refuses an unknown row and `cand` with no `CAND_MASK`. The patch at each of the eight
  masks in a scratch copy: bench 5707449 at each, eight distinct `.text` sections, none zero's; +5.64 % and +5.67 %
  instructions on `bench` and `bench 16` at 127, inside the prototype's range. Instructions and not a timing.
- `verify_fit` 1121 of 1121 on a zero-epoch probe; `start-check` on it.
- 2026-11-01, after the audit, three fast-check findings fixed, no rule moved: command 5 and `S101_cost.sh`'s `ROWS=cand`
  lines name the pair; `fit_one` runs `start-check` on resume and `read` calls a missing `full.hpp` unreadable;
  `S101_sprt.sh` compares `screened` with `.tuning/s101/bin/stamp`.

### Readings DEC-333 leaves open, the phase 1 agent's, flagged

- **`hanging_set` and `king_only_set` hold pawns and pieces**, the one set the two pawn and piece cells read.
- **The victim of a push is counted once however many targets reach it**: a set, as a pawn attack set is.
- **`threat_piece` and `threat_minor` read their unions directly**, so P2 or P3 alone builds all four unions though
  it reads three or two: one flag and not four, for a difference the cost run's rows will show if it matters.

### Gates

Both builds without a warning; both fast suites 50 of 50; `./clang-format.sh --check` clean
(`CLANG_FORMAT_MAJOR=22`); the S297 clang pair (`STAGES=clang tools/gate_extra.sh`), `build-clang` and
`build-clang-tune` with clang 22.1.8, no warning, both fast suites 50 of 50, 416 s, sharing the machine
with a fast suite; `tools/plan_prose_check.py`, one mode per call, each exit 0: `--touches`, `--params`,
`--citations`, `--gate`, `--prose`, 0 flagged. `S192_anchors.py` 10 of 10. **Tier 2 (DEC-141) does not bind**, by
what the diff touches: no `make_move`, `unmake_move`, generator or search change, and no pruning, reduction or
extension rule; under `src/` only `evaluation.cpp`, `evaluation.hpp` and `eval_trace.hpp`. `tools/gate_extra.sh`'s other
stages were not run. MANUAL.md read and unchanged: no option, command or output moves.

### Commands for the coordinator, in order

1. Commit phase 1; rebuild `build/` at the commit in the main checkout. `adocs/data/S101_screen.sh landing-check`.
2. `nohup adocs/data/S101_screen.sh all > .tuning/s101_screen_all.console 2>&1 &` -- locate under a minute, audit
   about 1 min, P1 to P7 of six fits each, about 24 min a part: about 2 h 50 m typical, 5 h 40 m at the 5000-epoch cap;
   watcher ceiling 12 h; `S101-ALL-DONE|FAILED`. Daytime. **No commit while it runs**, S307 and S308 included.
3. `CAND_SHA=<the landing> PARENT_SHA=<its parent> adocs/data/S101_cost.sh check`, then `run`: about 2 h (ten builds, ten
   comparisons), ceiling 4 h, the machine idle; `S101-COST-DONE|FAILED`. Any time after step 1, not beside step 2.
4. `adocs/data/S101_screen.sh apply <a clean worktree at HEAD>`, about 10 min with the clamp's two readings: over 1.0 % it
   prints STOP and phase 2 stops for the coordinator. Then phase 2 (an agent): every golden the paste moves, each by its
   script; `MATE_IN_THREE_FLOOR` under 11 stops for the owner.
5. `CAND_SHA=<the landing> PARENT_SHA=<its parent> ROWS=cand CAND_MASK=<the survivors' mask> OUT=.tuning/s101/cost_cand
   adocs/data/S101_cost.sh check`, then `run`, about 25 min. Step 3's pair, the landing and never the paste: the patch forces
   the groups over the family at zero, and by then HEAD's parent carries the family, which the defaults would refuse.
6. Pin `adocs/data/S101_sprt.sh` (the bracketed lines, `moves` narrowed to the survivors) and run it at night: up to 41900
   games, 19.86 h; watcher ceiling 40 h; on H0, `S101_screen.sh part A|B|C` (about 17 min each) and up to three more SPRTs.
   S307 may land between runs; S308 after the close.

## 2026-11-01: phase 2

One Opus 5.5 subagent, worktree `../chesso-s101`, branch `s101-p2` on `c2f2025`, on the coordinator's paste
(`adocs/data/S101_screen.sh apply`, the screen's 1234567 fit, all seven parts). Every reading with its command is in
`adocs/data/S101_goldens.txt`, the lost mates in `adocs/data/S101_lost_mates.txt`. No fit, match, SPRT, datagen or timing ran.

### STOPPED AT `EXACT_FLOOR`, for the owner (DEC-299, DEC-303, DEC-323's shape)

**`EXACT_FLOOR` falls.** `S156_mined_floor_sweep.py` reads the candidate at **158 shipping and 154 weakened** against the
floor's 159; the parent 168 and 158. `test_mate_breadth` stays red, in both builds, at 158 >= 159, and nothing is written
over the floor: lowering it is the owner's (DEC-303's consequence, DEC-304 (11), DEC-313's "under DEC-299's guard"). The script's
band is 155 to 158, so a floor can still separate: 157 is one below what ships (DEC-313 (1)'s rule), 3 above the weakened
count, and the gate at `RfpMinPly` 1 is red at 154. **`MATE_IN_THREE_FLOOR` holds**: 12 of 24 shipping, 10 weakened, red at
10 >= 11 and on mate-in-two exactness, one below what ships, DEC-333 (6)'s stop not reached.

**The attribution, S136's method (DEC-305, DEC-308 (2)).** 16 rows lost and 6 gained at depth 10. All 16 come back by depth 16
and hold: 12 within 2 plies of the parent, rows 107, 129 and 136 at 3, row 163 at 6 (10 -> 16). The oracle reads every lost
row's label at 5000000 nodes; gained row 246 needed DEC-320's deeper reading, mate 7 at 1000000000. The count is a depth 10
effect:

| exact of 318 | depth 10 | depth 11 | depth 12 |
|---|---|---|---|
| parent | 168 | 187 | 199 |
| candidate | **158** | **191** | **206** |

Traced, S295's patch through S136's: in rows 129, 136 and 163 the mating move is scouted and a mated side's non-PV node returns
by reverse futility (DEC-158's price, DEC-319 (2)'s shape); in row 107 the mated side's ply 1 node returns by the multicut at
d9 to d11, the rule whose mate row DEC-313 (4) retired on S291's attribution. No row reads as a rule defect by the trace; the
ruling on that is the coordinator's, as DEC-323 (1) was. Many single options restore each row, S136's "fragile tree".

**S170's answer is read and not applied**, DEC-299 asking both floors to hold or rise: `--ceilings` over the twelve grids answers
2, 13, 4, 1, **3**, 3, E's one rise inside the 2 the guard allows, and the picker A 1000000 -> 100000, D 1500000 -> 1000000, F
500000 -> 100000. `test_mate_carry` is green at what it carries. The grid is `adocs/data/S101_sweep_s170.txt`.

### Every golden, each by its script

| golden | before | now | script |
|---|---|---|---|
| six piece anchors, `QUIET_ROOK_EVAL` and `_CHEAP`, 394, `black_static`, quiet evasion, repetition | 107, 283, 353, 590, 1021, 0; 590, 578; 394; 199; -541; -603 | **unmoved**, 10 of 10: no anchor holds a man any cell counts | `S192_anchors.py` |
| `POSITIONAL_ROOM` | -- | did not fire: the pawn's residual \|107 - 84\| = 23 against 150 | the same |
| `truncation_positions` | S125's, phases 5, 11, 17, 23 | **re-picked**: phases 5, 11, 17, 17. 99 past 2.8, 27 at 2.875, 26 distinct. Worst 2.875 = 69/24, so N stays 3. Of S125's four, three still read 2.875 and phase 23's 1.958 | `truncation_scan --data <S083's corpus> --min 2.8`, `S083_truncation_pick.py` |
| coverage blocks | 19, 4, 2, 6 pairs | **unmoved**: all four reproduce, eligibility re-checked, 0 one-way; S101's tool scoped to end at DEC-316's pair | `S121_`, `S123_`, `S125_`, `S101_coverage_rows.cpp` |
| the fixture, 104 rows | 104 | **106**, DEC-316's pair, below | `S101_gradient_rows.cpp` |
| clamp push | +30 | +50 on the 104 rows; **+30 unmoved** on the 106: two rows bound, 198.7, the nearest other 158.8, every row 14.67 from the kink, fd 1.52e-7 | `S083_tuner_preconditions.cpp clamp` |
| gauge walk | 0.9618 / 1.2480 | 0.8763 / 1.0530 on the 104 rows; **0.7951 (Adam, the knight) / 0.6157 (AdaGrad, the bishop)** on the 106, over the 0.01 floor; holds at most 1.13e-14 | `S083_tuner_preconditions.cpp walk` |
| runs apart (`apart < 1e-6`) | 1.19e-12 / 4.25e-11 | 4.96e-6 on the 104 rows under AdaGrad, red; **1.36e-12 / 7.39e-12** on the 106 | the same |
| fixture clear of the clamp | 121.1 | **113.3** against 183 | the same |
| gradient check (DEC-316) | 4.98e-7 at 990 | 2.07e-6 at 984 on the 104 rows, red; **2.68e-7 at 985**, a factor of 3.7 | `test_tuner_gradient -s` |
| `EXACT_FLOOR` 159 | 168 / 158 | **158 / 154, red, the owner's** | `S156_mined_floor_sweep.py --floor 159` via `S136_overlay_run.py` |
| `MATE_IN_THREE_FLOOR` 11 | 12 / 10 | **held**: 12 / 10; `red` red at 10 >= 11 and on mate-in-two exactness | `S154_floor_margin_sweep.py floor`, `red`, the same overlay |
| node budget | 65024 / 3251 (10306, 0.63 of c) | unmoved: **10467**, 0.64 of c, inside [7270, 32512] | `S192_node_budget.py` |
| first-mate; `DRIVE_DEPTH`; ply-2 | 9, 9, 5; 4; 6 | unmoved | `S188_repair_goldens.py first-mate`, `s207`; `S083s_rfp_ply2.py` |
| `capture_mates`; extra ply | 9, 8, 11, 12; 8 (mate 4) | unmoved: each at its asserted depth (the third from 10, the fourth at 9, 11 and 12); mutant separation not re-read, every case green | `S230_mine_r01_row.py depths`, candidate and parent |
| `mate_probcut_hides` | S125's row, depth 9, mate in 5 | **re-mined**: `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42`, depth 10, **mate in 6**. S125's row still separates, `d9` against `d10`, and the pick moved to the cheaper cell. 3 of 269 separate, one by distance only; shipped `d10` against B04's none in 7 to 10; witness at 7 to 10; Stockfish at 5000000 nodes `#+6`, its line ending in mate. Observed red under B04, green shipped | `S277_mine_mate_row.py`, six stages, guard mode |
| `LATE_CAPTURE_*` | LQ02 at 7, LQ03 at 3 | unmoved, node counts apart | `S261_mine_late_capture.py sweep`, `pick` |
| aborted-line cell | line 114 at 250000 | **line 162 (game 4593) at 190000** (run 140000..240000; margin 50000 each way; 4 of 318 separate). Line 114's old cell ends in an iteration with no line. Observed red under MW02 (9 >= 13), green shipped | `S294_fallback_cell.py`, four stages, MW02 on a throwaway tree |
| `ALPHA` | -2108 (band -2165 .. -1935) | **-2108 kept**, the band unmoved, 57 below and 173 above; the trace meets the node at -2055, -2171 and -2350 | `S121_alpha_window.cpp`; `S136_trace_patch.py` |
| `short_line_ceiling` | 2, 13, 4, 1, 2, 3 | **read, not applied**: 2, 13, 4, 1, 3, 3 | `S203_case_sweep.sh --ceilings`: the eleven grids plus S101's |
| S170 budgets | A 1000000, B 100000, C 1000000, D 1500000, E 300000, F 500000 | **read, not applied**: A 100000, D 1000000, F 100000; B, C and E unmoved | `S170_pick_budgets.sh` |
| "a later board sent as a bare fen keeps the table", its fixture | the fixed pair g1f3 b8c6 | **two plies down the line the search reports**, no golden: below | -- |
| readings, none asserted | 699 mate_pv lines; defender 86/104 | 699; **83/104** | `test_mate_pv -s`; `S165_nmp_defender_sweep.py` |
| `golden_defaults`, option count, census, book | -- | unmoved by construction | -- |

### DEC-316's pair

The gradient case read 2.07e-6 at parameter 984, `passed_king_own_mg[5]`, on the 104 rows: analytic +7.507e-5, 1.55e-10 from the central
difference, the relative error O(h²) (h 0.05 to 0.002: 5.17e-5, 8.26e-6, 2.07e-6, 5.17e-7, 8.22e-8). The derivative is right. **Not S135's
class exactly**: S135's weight was all but stationary, this one's gradient stands among its siblings', 4.3e-6 to 5.8e-4, so S135's threshold is
already met and would take whichever pair came first. **This applies DEC-316's rows to a case its text does not name**: that text is a weight near
zero, and this weight is not. The coordinator records it in DEC-343: rows again, the metric, h and the 1e-6 unmoved, the floored metric left as the
recorded alternative. `adocs/data/S101_gradient_rows.cpp`'s rule, stated in its header before it ran, reads the analytic gradient alone: the first
eligible pair of `all_test_fens()` that triples it, the law's 2.07 and a factor of 1.45 to spare. Candidate 55, `3k4/8/1K6/8/8/8/p1pppppp/RRqRRRRR w - - 0 2` and its mirror, |g| 6.16e-4 against
2.25e-4; python-chess Status.VALID both. It reproduces.

**The hold's `apart` came back with it, and not by design.** 4.96e-6 on the 108 gauge rows was AdaGrad at lr 10 going chaotic late, last-bit rounding
growing 5e-9 at epoch 250, 1e-6 at 275, 5e-6 at 300, at lr 7 no more than 5e-12: the class the case's own comment gives for a step of 30. DEC-316 speaks
of the gradient and not of this. The pair's rows move the descent and it reads at most 6.28e-11, 7.39e-12 at epoch 300. **So the margin is not a measured distance**:
a later fit can bring the onset back, and the case's input, the step, is then the question; no bound moved. The case's comment says so.

The binary's time against its 60 s clock: 22.73 s and 22.78 s in the two suites; alone on an idle machine the planted recovery 21.51 s and the derivative case itself 0.065 s (`doctest -d`). S309 owns the
`TIMEOUT` question. The clamp case reads 1.52e-7 at parameter 141. The synthetic group reads 3.46e-7 at 1124, against 3.46e-8 at phase 1, a factor of 2.9 under its 1e-6; a reading, the case green. The planted recoveries are green in both
builds with no budget moved.

### The later board's fixture

`tests/test_engine.cpp` "a later board sent as a bare fen keeps the table" searched from e2e4 e7e5 and needed the entry for e2e4 e7e5 g1f3 b8c6: a fixed pair, read
off an older line. The candidate's depth 6 line is b1c3 b8c6 and the old board is in no search, so the precondition went red with the rule under test intact. **The
site's rule is "the search stored the later board"**, so the later board is now the board two plies down the line the search reports: no pair to re-derive, and no
golden. Observed: with S271's `A undone` back in `commit_position_base()` the case fails at "a new base FEN cleared the transposition table", 5 of 6 assertions,
the preconditions among them; green shipped.

### What the paste leaves for the close

`src/evaluation.cpp`'s paragraph over the arrays now says what ships. `src/evaluation.hpp`'s sentence "Landed at zero" is still true. `adocs/specs.md`'s two rows
wait for the coordinator. `adocs/data/S101_sprt.sh`'s header said the commit was green and it had no working-tree mode when this was written; DEC-334 switched it to
S136_sprt.sh's form at REF `4e34e61`, DEC-323 the precedent.

### Gates

Both builds: no warning or error over the rebuild (the build's grep empty; `build rc 0`, `build-tune rc 0`). `ctest -L fast`: **49 of 50** in `build/` and in `build-tune/`, sequential, `CLANG_FORMAT_MAJOR=22`; the one red in both is `test_mate_breadth`, `REQUIRE( found_exact >= EXACT_FLOOR )`, 158 >= 159, the stop above. `./clang-format.sh --check` clean. **Bench 5482277** in both builds. `S192_anchors.py` 10 of 10. `S101_coverage_rows.cpp`, `S125_`, `S123_`, `S121_`: "reproduces: yes". `S101_gradient_rows.cpp`: "reproduces the landed pair: yes". `tools/plan_prose_check.py`, one mode per call, each exit 0: `--touches`, `--params`, `--citations`, `--gate`, `--prose`, 0 flagged, read again after this section. `git diff -U0 c2f2025 -- src`, outside whole-line comments, is the fourteen one-line definitions and nothing else, `S101_sprt.sh`'s line test reading 28 lines, 0 outside its form; `src/evaluation.cpp` the one path under `src`, `CMakeLists.txt` and `cmake`. **Tier 2 (DEC-141) does not bind**, by what the diff touches: no `make_move`, `unmake_move`, generator or search change, and no pruning, reduction or extension rule. `gate_extra`, the clang pair and the Debug self-play were not run here, and are the coordinator's if the brief's form holds. `DEV_MANUAL.md` checked: the goldens rows, the gradient, walk, `eval_spread` and `test_mate_pv` sentences moved. `MANUAL.md` checked: the `RfpMinPly` row's figures moved; no option, command or output moved.

### Readings this phase took that DEC-333 and this file leave open, flagged

- **`EXACT_FLOOR` read as a stop**, though DEC-333 (6) and the brief name only `MATE_IN_THREE_FLOOR`: DEC-303's consequence says "a fallen floor", DEC-299's guard names both floors, and DEC-313 re-reads both under that guard at every refit.
- **S170's answer held with it**, DEC-299's words. E's rise of 1 alone would be inside the guard.
- **The rest done regardless**, DEC-323 (2)'s shape: each golden below holds on any ruling that keeps this paste.
- **`mate_probcut_hides` re-mined though green**, because its script's pick moved. S125's row still separated; earlier steps read such a row and kept it. The brief's rule, a golden whose script gives another number is re-derived, decided it.
- **DEC-316's threshold**: a multiple of the gradient, not S135's tenth of a sibling, for the reason above.
- **The later board by rule and not by another mined pair**, so nothing there asks a re-derivation again.

### Verdict fields left out

No verdict at this phase, so no `done:`. Elo, LLR, games, wall and forfeits are the next section's. The cost `cand` row was not run: its mask, 127, is the `all` row (DEC-334).

## 2026-11-01: the SPRT read H1

One Opus 5.5 subagent, in `../chesso-s101` (branch `s101-p2`, HEAD REF `4e34e61`, the played tree), read only over the run's outputs and binaries. No match,
SPRT, fit or timing ran in it. No code line under `src/` moves; the comment pass below is proved so.

### The run

`adocs/data/S101_sprt.sh` as pinned 2026-11-01 18:52: the worktree's uncommitted tree (the screen's 1234567 fit over the family's fourteen arrays, bench 5482277) against its HEAD
`4e34e61` (the phase 1 landing's `src/`, the family at zero, bench 5707449), launched 18:52:21, `SPRT-RUN-DONE` after fastchess's Total Time 00:45:59:

```
SPRT | cand 4e34e61+7d6f96ec896c vs ref 4e34e61, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | 33.87 +/- 12.33, nElo 46.57 +/- 16.84
LLR | 2.95 (-2.94, 2.94) -> H1
Games | N: 1636 W: 522 L: 363 D: 751, Ptnml [35, 171, 302, 220, 90]
Wall | 0 h 45 m, 2134.7 games/h, forfeits 0
Log | adocs/data/S101_sprt.log
```

`Wall`'s rate is N over fastchess's `Total Time`, S296's rule: 1636 games in 2759 s, against the 2110 an hour budgeted (DEC-190).

### The inspection, before the verdict was read as the record

- **Banner** (the console's): candidate `4e34e61 2026-11-01 + uncommitted changes`, `diff .../sprt_s101_20261101_185221/candidate.diff blob 7d6f96ec896c`, reference `4e34e61 2026-11-01`
  (the commit's date; its time, 09:35, is `git log`'s and the pin's, and fastchess.sh prints the date alone), `arch native tune off`, padding on both, 8+0.08, hash 16, 12 of 12 cores,
  `noob_3moves.epd`, seed 20261101185221, `elo0=0 elo1=5 alpha=0.05 beta=0.05`, cap 20950 rounds and 41900 games from `SPRT_ROUNDS`, out `.tuning/sprt_s101_20261101_185221`; no `relay` or `A/A
  CALIBRATION` line; `id name`s `cand-4e34e61+7d6f96ec896c Chesso 4e34e61-dirty native` and `ref-4e34e61 Chesso 4e34e61 native`; `Started game 1 of 41900`. As the pin says. Above it the
  script's guards printed `landing-check: c2f2025's fourteen family arrays are all zero` and the fourteen arrays as the only weights moved. The governor reads `powersave` (DEC-195).
- **Binaries.** The reference `.ref-builds/4e34e61/build/src/chesso`, written 18:52:29 and not since, its worktree clean at its sha, benches **5707449**. The candidate played from fastchess.sh's
  snapshot of `build/src/chesso`, deleted at the end: that file, written 18:51:16, benched **5482277** before this close touched anything, sha256 `6c936a8158c0608f`, kept for the identity below.
- **The tree played.** `candidate.diff` is blob `7d6f96ec896c020789b141be2ac5ef28ced213db`, the banner's and the engine's name. `git add -A` of the worktree into a scratch index and `git diff-tree -p
  --binary --full-index HEAD <tree>`, fastchess.sh's own lines, equalled it byte for byte before this close touched anything: the diff is the tree played.
- **Crashes, disconnects, illegal moves**: none. `fastchess.log` is empty (0 bytes), `Assertion`, `disconnect` and `illegal` each grep 0 over the console, and it holds nothing but the guards' and
  build lines, the banner, the game lines, the result block, the stop lines and the foot. 1649 games started and 1638 finished, the 11 others in flight at the stop. The PGN's 1638 games carry
  fastchess's game numbers 1 to 1640, each at most once, 1634 and 1639 absent: 818 complete pairs, the 1636 games fastchess scored, and games 1633 and 1640 whose partners never finished. Python-chess
  reads every game with no error. First `GameStartTime` 18:52:29, last `GameEndTime` 19:38:25.
- **Terminations**: 1067 adjudication (886 decisive, 181 drawn) and 571 normal, all drawn (396 threefold, 122 insufficient material, 47 fifty-move, 6 stalemate).
- **Forfeits**: `tools/forfeit_report.py` reads 0 of 1638 for each engine.
- **Console warnings**: none of any kind: `grep -ci warning` 0 and no PV-past-threefold block (`trim_console.py`'s count).
- **Quarters**, in game-number order over the PGN's 1638 games, 409 or 410 each, by `adocs/data/S121_sprt_mate_pvs.py quarters`: **+46.1, +20.4, +39.2, +30.6**, each +/- 24.3 to 25.2. Their spread about the
  run's mean, +34.05 over those games, is chi-square 2.35 on 3 degrees of freedom, p 0.50.
- **Kernel journal**, 18:45 to 19:42 (boot `b687a392`, from 18:35:16, no suspend): no kernel entry. From 18:52 to 19:40 three entries in all, 19:17:01's `cron.hourly`, none at warning level or above. No machine
  check, OOM or thermal event.

The run stands.

### Mating PVs

`grep -c "Incomplete mating PV" .tuning/s101_sprt.console` reads **0**, and `grep -ci warning` over the same console reads 0: no warning of any kind, so no mate-distance check is owed and DEC-319 (3)'s oracle and
DEC-320's deeper reading do not arise. `adocs/data/S296_sprt_mate_pvs.py` reads the same marker and would find nothing. A count, recorded: 0 per 10000 side-games over 1638, on either side.

### The verdict, by the pre-registration

**The H1 row executes.** At 8+0.08 the fitted family is accepted as gaining: H1's 5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It ships**: all seven parts, P1 a piece a pawn attacks, P2 a piece a lesser piece
attacks, P3 minor on minor, P4 hanging pieces, P5 hanging pawns, P6 what only the king attacks, P7 a safe push, the screen's 1234567 fit. **No bisection follows an H1**, and none is owed. No part stays at zero: the
screen dropped none.

**The stopping run's Elo is upward-biased (DEC-063) and is not the effect size.** The run stopped at a bound in 1636 games, far under the 25591 the pre-registration expected with the truth on a bound; nElo +46.57 is the
early stop's figure.

**The cost, beside it**: all seven live, **+4.68 %** on its own A/A floor, instructions +5.67 % and +5.93 %, p7 +1.06 %, p1 to p6 and the parent zero (`adocs/data/S101_cost_reading.txt`), on cost-only builds of the landing at
forced weights (DEC-047). No `cand` row, its mask being `all`'s (DEC-334). It decides nothing; the SPRT priced it. The clamp binds on 0.400 % of S083's rows against 1.0 %, 0.355 % before; S122 inherits the clamp's readings and
the sets the survivors build, and `LAZY_EVAL_MARGIN` is S122's to re-derive (DEC-324, DEC-333 (6)).

### The ruling, DEC-343

**The owner, 2026-11-01: `EXACT_FLOOR` 159 -> 157**, one below the 158 shipped (DEC-313 (1)'s rule), 3 above the 154 weakened, so it still separates. DEC-299's and DEC-334 (3)'s held items land with it, each by its script:

| item | old | new | by |
|---|---|---|---|
| `EXACT_FLOOR` | 159, red at 158 | **157**, its site comment and the header's two ends, margin and band rewritten | the ruling |
| the gate at `RfpMinPly` 1, 157 compiled in | -- | **red at 154**; 158, 159, 154, 154 at `RfpMinPly` 3, 2, 1, 0, 157 inside the band 155 to 158 | `S156_mined_floor_sweep.py --floor 157` through `S136_overlay_run.py`, `adocs/data/S101_mined_floor_157.log` |
| `short_line_ceiling` E | 2 | **3**, E's 1500000 cell at stride 1, 3 of 6; the other five unmoved | `S203_case_sweep.sh --ceilings`, the twelve grids, re-run at the close |
| S170 budgets | A 1000000, D 1500000, F 500000 | **A 100000, D 1000000, F 100000**, B, C and E unmoved, with a header paragraph | `S170_pick_budgets.sh`, re-run; byte-identical to `.tuning/coord/S101/S170_cases_after_ruling.tsv` |
| `adocs/data/S101_sweep_s170.txt` | -- | in `tests/test_mate_carry.cpp`'s two commands and table and `DEV_MANUAL.md`'s command, the eleventh grid added | -- |
| `DEV_MANUAL.md` floor and ceiling rows, `MANUAL.md` `RfpMinPly` row | held | what ships | -- |

`MATE_IN_THREE_FLOOR` holds at 11, 12 shipping and 10 weakened: the same engine as phase 2's reading, the binary byte-identical below. The DEC-333 (6) stop did not arise.

### The fast check over phase 2's diff, owed by DEC-334 (5)

1. **`tests/test_tuner_gradient.cpp`'s "984 reads 1.11e-07"** was in no record, and the 1/|g| law suggests 2.5e-7. Read: **1.11153e-07 is right**. The pair moves the truncation term as well as the gradient, 1.551e-10 to 6.852e-11,
   and flips the gradient's sign. `adocs/data/S101_gradient_rows.cpp` now prints it after the pick, no part of the rule, and `adocs/data/S101_goldens.txt` section 14 records it; the comment says why it undershoots the law.
2. **"`S101_sprt.sh`... has no working-tree mode"** said of a file REF changed: reworded in the phase 2 section. DEC-334, DEC-335, DEC-342 and DEC-343 added to `decisions:`.
3. **`adocs/data/S101_gradient_rows.cpp`'s header.** "Every candidate it tried until the pick" against `picked || tried <= 10`: the program now prints every candidate, the honest fix, 55 lines, so the record shows no earlier pair reached the multiple. "The rule's
   threshold is the one difference" from S135: the header now names the three, the threshold, eligibility (S121's, of which S135's clamp clause is one clause) and the failed read. **The pick did not change**: candidate 55, the same pair.
4. **"Keeps DEC-316 as written"**: false, the weight not being near zero. Phase 2's section, the tool's header, the case's comment, `DEV_MANUAL.md` and `adocs/data/README.md` now say this applies DEC-316's rows to a case its text does not name, which DEC-343 records: rows
   again, the metric, h and bound unmoved, the floored metric left as the recorded alternative.
5. **`DEV_MANUAL.md`'s "191 at depth 11 and 206 at 12..."** cited `S101_goldens.txt`: it now cites `S101_lost_mates.txt`, where they are.
6. **`adocs/data/S101_coverage_rows.cpp`'s "which is the landed list"**: false at 106 rows against 104. It now says the list through this block, 104 of the landed 106, DEC-316's pair being the other tool's.
7. **The gauge case's "At 3 and at 10 the two stay within 6e-13"** was S262's reading on S262's rows in the present tense: now attributed to S262, and the comment gives today's, read again on the 110 rows every 25 epochs: within **1.30e-11 at 3** and **6.28e-11 at 10**, 7.39e-12 at
   epoch 300 at 10, a reading and no margin. Phase 2's 1.43e-11 was lr 3 on the 108 rows.
8. **`S101_gradient_rows.cpp`'s `if (!gradient_of(rows, path, &g)) { continue; }`**: made to fail. A pair `eligible` passed that then fails to load or to trace is a defect and not a pair the rule passes over; swallowing it could have moved the pick silently. The header's rule states it.
   **It did not fire**: 55 eligible pairs tried, 55 read, the same numbering as before the change.

### The comment pass, and the identity

Every comment under `src/` that said the family is pending or a verdict could send a part back now says what ships: `src/evaluation.hpp`, the family's heading; `src/evaluation.cpp`, the paragraph over the arrays and the one over the flags. `src/eval_trace.hpp`: read, nothing in
it says the family is at zero or pending. Unchanged. **The liveness gating keeps a purpose, and stays**: every flag is true today and gates nothing, but a refit could zero a group, S126 refits everything, and a group a later screen drops is held at zero, its ORs out of stage two. The
comment over the flags says so.

**No code line moved** (`adocs/data/S101_close_identity.txt`, `adocs/data/S306_identity.txt`'s form): `g++ -fpreprocessed -dD -E -P` over the played `src/evaluation.cpp` and over the worktree's byte for byte the same, and likewise `src/evaluation.hpp`; every changed line under `src/` a whole-line
comment; **the Release binary rebuilt from this tree is byte-identical to the played one**, sha256 `6c936a8158c0608f`; bench **5482277** in both builds.

Elsewhere: `DEV_MANUAL.md`'s `eval_spread` sentence ("At S101's landing they are zero; at its candidate") now says what ships. Tests and tools hold no site calling the family at zero or pending other than as the landing's history. `adocs/data/README.md`'s three rows that said held or not applied now say
applied. `adocs/data/S101_sprt.sh` stays as played.

**S122's step file is not this step's; the sentences the H1 makes false are quoted to the coordinator**: "S263 built no per-side attack set, since no term read one; a term here that needs one builds it once per evaluation." and "**S121 (before)**: the attack sets and the mobility area.", S121 having
built no union; and "both read attack sets the mobility loop already builds (S121)" is now true by S101, DEC-333's consequence.

### The trim and the archive

`tools/trim_console.py --summary` from `.tuning/s101_sprt.console` (3982 lines, 314436 bytes) into **`adocs/data/S101_sprt.log`**, 3987 lines, 314641 bytes: its first 3982 lines are the console byte for byte, since no PV-past-threefold block exists, and the foot's five lines say so. `candidate.diff` byte for byte to
**`adocs/data/S101_candidate.diff`**, blob `7d6f96ec896c` (DEC-254). The untrimmed console and the run directory are **`chesso_S101_sprt_2026-11-01.tar.xz`** in the owner's archive (DEC-235), beside `chesso_S101_screen_cost_2026-11-01.tar.xz`, both the coordinator's, verified by it.

### What H1 leaves unmeasured

- **Each part's own share.** The seven shipped on one verdict and were never played apart; each has only its screen gain, d +0.000051, +0.000025, +0.000047, +0.000006, +0.000014, +0.000008 and +0.000053 against m 0.000002, 0.000002, 0.000006, 0.000002, 0.000002, 0.000003 and 0.000002. P4 and P6 are the
  thinnest.
- **The transfer gap, DEC-333 (8).** The corpus is quiescence leaves: 8.0 % of lesser and 8.6 % of hanging piece threats there have their attacker on move, against 54.2 % and 56.2 % of the search's lazy calls on `bench`. One weight serves both. The match plays both and does not separate them, so H1 says the
  one weight gains and not that it is the right weight for either case. The cells doubled by side to move stay unbuilt.
- **The cost apart from the verdict.** +4.68 % is priced inside it; no run measured the family's strength net of its speed or a part's cost in play.
- **The long control.** Stash's traced figure shrinks, +10.13 at 10+0.1 to +7.77 at 60+0.6, and that term is wider than this one. Nothing here measured that: S267's DEC-202 reading and its drift series are where it is read, an estimate and never a verdict.
- **The far and negative cells, for S126**: `threat_king_mg` [12, 48, 42, 62], `hanging_piece_eg` [10, 12, -7, -14], `threat_king_eg` [20, 0, 13, -11]. A negative weight on a hanging rook or queen in the endgame and a weight of 62 on what only the king attacks are the fit's readings, each cell adding to the lesser cells beside
  it (DEC-333 (3)); no constant changed for them (DEC-084). The thinnest cell is `threat_king_eg[2]`, 12347 rows and 9149 games, against the audit's 1000.
- **P7's q3 fit stopped at the 5000-epoch cap**, where P7's five other fits stopped on patience by epoch 200 (`adocs/data/S101_screen_reading.txt`): best 0.071116 from 0.071319. P7's d stands far past its margin, +0.000052 at that quarter against m 0.000002, so no decision turned on it; that one fit had not met the patience rule when it stopped, and the candidate is the full fit, which had.
- **The thin margins.** `EXACT_FLOOR` 157: 1 below the 158 that ships, 3 above the 154 weakened, and the count falls at depth 10 and leads from depth 11 (191 and 206 against 187 and 199). `MATE_IN_THREE_FLOOR` 11: one below the 12, one above the 10. The gradient 2.68e-7, a factor of 3.7, and DEC-316's answer on a later fit. The gauge
  case's `apart`: the pair's rows moved the onset of last-bit chaos at lr 10 by chance, so a later fit can bring it back, and its input, the step, is then the question (S310). The `short_line_ceiling` E 3 and each budget are one grid's worst or cheapest cell, the knife edge the TSV says they are.

### Documents

- `DEV_MANUAL.md`: checked. The floor row, the ceiling row and its command, `MATE_IN_THREE_FLOOR`'s row (it said `EXACT_FLOOR` 159 does not separate, false since S121), the gradient sentence and the `eval_spread` sentence changed. Nothing else false.
- `MANUAL.md`: checked. The `RfpMinPly` row's floor sentence changed; no option, command or output moved.
- `adocs/data/README.md`: a close section, `S101_candidate.diff`, `S101_sprt.log`, `S101_mined_floor_157.log`, `S101_close_identity.txt`; the three rows above.

### The accepts, clause by clause

- **An SPRT verdict against a named commit, recorded whatever it is (INV-6)**: H1 against `4e34e61`, above, with DEC-220's block for the closing commit. Holds.
- **Fitted with every other constant frozen, the held-out error reported before and after**: `S101_screen.sh`, S082's fixed set, `--only` the family and every other weight frozen (`S101_screen.py only-family` over the paste), start 0.071321, best 0.071117 at the full 1234567 fit (`adocs/data/S101_screen_reading.txt`). Holds.
- **Computed in a stage that has the attack bitboards, looked up once and never a second time, the unions one OR a piece in stage two's own loop, the shortcut's nodes paying nothing**: `src/evaluation.cpp` `evaluate_mobility_and_king_safety`, `threat_terms()` reading what the loop built and looking nothing up; `evaluate_cheap()` holds no
  term of it, and phase 1's `adocs/data/S101_inv6.txt` read `evaluate_cheap()`'s 456 instructions identical against the parent at zero. Holds.
- **INV-5, asserted in tests/test_evaluation.cpp**: "the threat family's sets mirror exactly" and "the threat family's counts mirror exactly", 29846 positions, each planted one-colour defect seen red (phase 1's table), and `colour symmetry over every test position` green at the fit's weights. Holds.
- **The step states which stage and why**: stage two, summed into mobility's pair under the clamp, so the lazy bound holds by construction, S039's H0 keeping it, measured against about +16 to +20 % in stage one (DEC-333 (1), the research pass). The clamp reads 0.400 % at the fit. Holds.
- **The trace, `term_layout()` and the walk gain the features, `tools/tuner_groups.hpp` a group whose partition properties hold**: seven groups, `SITE_MOBILITY`, `walked_counts::threat_family_features()`; `test_eval_trace`, `test_eval_model` and `test_tuner_groups` green. Holds.
- **Each threat kind screened before the family's one SPRT (DEC-259)**: seven parts in DEC-333 (2)'s order, each past its margin, the readings above. Holds.
- **The fast suite green**: 50 of 50 in both builds, below. Holds.

### Gates

- Both builds: 0 warning lines over the rebuild. `ctest -L fast`: **50 of 50** in `build/` and in `build-tune/`, sequential, `CLANG_FORMAT_MAJOR=22`, 138.8 s and about the same. Bench **5482277** in both: the commit's `Bench:`.
- `./clang-format.sh --check` clean.
- `tools/plan_prose_check.py`, one mode per call, each exit 0: `--citations` 0 flagged over 20 files, `--touches` 0 over 20, `--params`, `--gate` agreeing, `--prose` 0 flagged. Read again after this section was written.
- `adocs/data/S192_anchors.py` 10 of 10. `adocs/data/S101_gradient_rows.cpp` "reproduces the landed pair: yes".
- **`tools/gate_extra.sh`**, all six stages, on this tree, detached under a four-exit watcher: `GATE-EXTRA-DONE 6 stages 1525 s`. prose 0 s, citations 0 s, debug 388 s (6 of 6), sanitize 729 s (the fast label 50 of 50 under the sanitizers, past the floor where the pass before the run stopped), perft 52 s, clang 356 s (50 of 50 twice, no warning). 0 `Assertion`, 0 `runtime error`, 0 `AddressSanitizer` over the stage logs. Only this file changed after it.
- **Tier 2 (DEC-141) does not bind**, by what the diff touches: under `src/` the fourteen one-line definitions and comments; no `make_move`, `unmake_move`, generator or search change, and no pruning, reduction or extension rule. `gate_extra` ran as the brief asks, since the family's live path had run under no Debug or sanitizer build before DEC-334 (5)'s pass. The Debug self-play at landing is the coordinator's; the one on this tree before the run read 8 games, 0 `Assertion`, 0 `disconnect`.

Every clause holds. Done.
