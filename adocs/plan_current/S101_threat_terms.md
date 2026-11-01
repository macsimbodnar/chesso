id:         S101
goal:       evaluation terms for a piece attacked by a lesser piece, for an attacked piece nobody defends, and for a safe pawn push that would attack a piece, fitted like every other constant
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the terms are fitted with every other constant frozen and the held-out error reported before and after, which is how every constant in this engine was fitted; the term is computed in a stage that has the attack bitboards it needs, looked up once and never a second time, so the unions it builds are one OR a piece in stage two's own loop and the shortcut's nodes pay nothing for them (DEC-333 (1)); INV-5 holds -- mirroring a position agrees rather than negates, asserted in tests/test_evaluation.cpp; the step states which stage the terms land in and why (S039 read H0 and kept the lazy bound, record `ca578de`, so the bound must still hold by construction: stage two, under the clamp, DEC-333 (1)); the trace, `term_layout()` and the walk gain the features and tools/tuner_groups.hpp a group whose partition properties still hold; each threat kind is screened before the family's one SPRT (DEC-259); the fast suite green
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_trace.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp, tools/tuner_trace.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tools/eval_spread.cpp, tests/test_evaluation.cpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_eval_positions.hpp, tests/test_walked_counts.hpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, .tuning/verify_fit.py, adocs/data/S192_anchors.py, adocs/data/S125_coverage_rows.cpp (scoped), adocs/data/S101_*, DEV_MANUAL.md, adocs/data/README.md; MANUAL.md read and unchanged at phase 1
excludes:   outposts and space, which are S102; king safety, which exists and is fitted, and whose squares and checks are S122's (DEC-333 (9)); overload threats, which have no owner
decisions:  DEC-071, DEC-258, DEC-259, DEC-333, DEC-324, DEC-332, DEC-315, DEC-316, DEC-326, DEC-329, DEC-330, DEC-331, DEC-221, DEC-134, DEC-084, DEC-142, DEC-143, DEC-047, DEC-266
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:

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
