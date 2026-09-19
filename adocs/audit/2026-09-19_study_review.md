# Audit 2026-09-19 — study_review

Type: `study_review`.
Object: `adocs/data/2026-09-19_search_technique_study.md`, as recorded in the commit this report follows.
HEAD during the review: the study's recording commit, branch `achesso`, tree clean apart from two
untracked third-party directories (`tests/json/`, `tests/pixello/`) that
predate this session and were not examined.
Report id stem: `2026-09-19_study_review`.

## Scope, and what this pass did

Three questions, as asked by the owner: (1) does the study's evidence support
what it says; (2) does the document itself observe the COPYING, seed and
chess-judgement rules; (3) what should change in the plan, and is the
"SPRT result in the commit" practice worth adopting as a rule.

Two reviewers. **The coordinator** re-derived every figure the study quotes
from the open-source record with a parser written for this review
(`adocs/data/2026-09-19_technique_ledger.py`, committed with the corrections), checked
every chesso claim against the file and line it names, and read every decision
the study cites. **A cold adversarial reviewer** (Opus 5, clean context,
read-only, working from the published commit log and release material)
attacked the document's inference, omissions and rule compliance in parallel;
every one of its citations was then re-verified by the coordinator before it
entered this report. The findings below are the merged, re-ranked set; the
last section records the cold pass finding by finding with the coordinator's
verdict.

Evidence standard: every figure re-derived by a command, every chesso claim at
`file:line`. Nothing outside this file was written. No match, fit or timing
was started. §3.3's structural claims were confirmed against the open-source
record itself — no killers and no countermove table in its current form; a
parameter module that is a two-mode macro with no declared tunables; the
continuation tables carrying an in-check and a noisy axis — and no constant
from the record is reproduced anywhere.

## Reproduction of the ledger

| quantity | study | this review | note |
|---|---|---|---|
| commits with an SPRT block | 796 | 796 | exact |
| dated 2025-02 or later | 794 | 794 | exact |
| runs at 8+0.08 / 40+0.40 / 4+0.04 | 682 / 243 / 72 | 682 / 243 / 72 | exact |
| bounds `[0,3]` / `[-2.75,0.25]` / `[0,4]` / `[-3,0]` / `[0,5]` | 372 / 215 / 169 / 103 / 89 | 372 / 214 / 169 / 102 / 88 | off by one |
| commits mentioning nElo | 1 | 1 | exact |
| games, 2025-02 on | **31,493,134** | **41,964,262** all runs; 31,493,134 first run per commit | F10 |
| STC runs in the cost table | 642 | 682 | 40 dropped, not said which; shape identical |
| commits with STC and LTC | 233 of 788 | 206 / 218 / 252 of 796 by three definitions | not reproducible as stated |
| SPSA sessions, sum | 14, ≈+73 | 14, +73.2 summing each session's LTC pass; 6 of 14 are SMP sessions | F07 |
| non-regression runs | "about a quarter" | 408 of 1064 runs, 16.46M of 41.2M games, mean 40,347 | F08 |
| removal-type commits | 241 | 240 subjects, 158 with a block | fine |

Every individual Elo/game figure in §3.1, §3.2, §4, §5 and Appendix A that
was checked (about seventy) is in the record at the commit and control stated,
with the three mislabelled controls of F15. The chesso claims — `build_lmr_table()` at
`src/search.cpp:175` truncating into `uint8_t`, two killers per ply, a
countermove table beside two continuation tables, quiescence SEE at a fixed
threshold of 0 (`src/search.cpp:1027`), S149 −11.02 ± 10.53 over 2522, S130
+1.14 ± 4.04 over 16784, S098 v1 −2.92 / −2.34, A22 `NOT FOUND`, the nElo
conversions in `DEV_MANUAL.md` "Which bounds", every Open-list position named —
all hold.

## Findings

### 2026-09-19_study_review-F01 high the record's game counts do not price chesso's verdicts, and the study's operating rule says they do

Status: closed -- DEC-222 (9): every hour figure withdrawn, a verdict priced by DEC-143 alone; the study corrected in place 2026-09-19

Lines 137-142 are the load-bearing claim: rank by the STC Elo the record
measured "because that figure is a prediction of **what the verdict will
cost**, which transfers far better than the Elo itself (DEC-019 is about the
Elo, not about the variance)". Games-to-verdict is a function of the *true*
effect in chesso, which is the quantity DEC-019 says does not transfer; the
carve-out has no basis. Chesso's own record is in `plan.md` "The third
discount": of five published-to-measured transfers with a figure at both
ends, **two are exactly zero, one is the wrong sign, one is 0.10 and one is
0.33**; the most generous ratio ever measured here is 0.38. Expected games
scale roughly with 1/Elo², so a 0.38 transfer is about seven times the games
and a zero is the full walk — 25,591 games on a bound, 41,861 at the midpoint
for `{0, 5}` (DEC-143, `plan.md:628-629`): 12 to 20 hours, not the "1.5 to 4
hours each" of line 388. The study names the passes-only selection (lines
69-72) and the stopping-estimate bias is in the ledger's own words
(`plan.md`: "the stopping estimate is upward-biased", DEC-063); a +24.01 read
off 1,870 games is the most biased number in the table.

What it changes: the *ordering* signal survives — a large true effect is
cheaper to measure than a small one, and the record is the best
available guide to which effects were large *there*. Every hour figure in §6
does not survive, and a pre-registration written from this study would
understate its run by an order of magnitude. The cost of a chesso verdict is
what DEC-143 says it is.

### 2026-09-19_study_review-F02 high §6 item 1 and §7 item 2 misread why the correction-history family is in the reserve; the owner ruled on it eight days ago

Status: closed -- DEC-222 (1): the family stays where DEC-133 and DEC-176 (c) put it, S099 runs as the probe after S232; the study corrected in place 2026-09-19

Lines 390-393 and 478-480: the reserve placement "rests on a band claim
already recorded as `NOT FOUND`", so "two independent sources" now stand
against it. DEC-133's operative reason is its Rejected line: *"Keeping S099 at
entry 12 with a restated reason — it places a technique whose only evidence is
at 3200 ahead of steps with sub-3000 records."* S181 re-banded that evidence at
3224–3291 and **DEC-176 (c), 2026-09-11, by the owner: "S099, S110 and S111
stay where DEC-133 put them. The re-banding is the *reason* DEC-133 was right,
not a new question."** The record is a network engine rated far above chesso's
band (line 207): a further above-band source, which is evidence *for* the
placement rule as the owner stated it. `grep -n "S181\|DEC-176"` over the
study returns nothing. The same paragraph's superlative — "the highest
measured Elo per machine-hour anywhere in this report" — is false by the
study's own table: time management by node distribution (+17.59 over 2,352)
and maximum time by move number (+24.01 over 1,870) are 16 to 28 Elo per
thousand games against the correction family's 4 to 10, S132 owns both, and it
sits at position 12 unpromoted.

What it changes: the study's first recommendation is a request to revisit an
owner's decision on cost grounds, and should be written as one (P1). The
cheap path it does not mention is the one DEC-133 already opened: S099 is
"the probe … run on a spare night when the machine is idle" — no reordering
is needed to take it. S132 is the better-priced promotion by the study's own
numbers (P4).

### 2026-09-19_study_review-F03 medium §3.2's mechanism descriptions come from open-source resources, and neither the study nor the rules say what that implies for the implementer

Status: closed -- DEC-221: a technique taken from the analysis is implemented from its published description, seeded in DEC-134's forms; the study's section 1 corrected 2026-09-19

The rows of §3.2 are mechanisms found in open-source resources and set down
in prose, and the study concludes that the compliant route is to implement
"from first principles or from a published description". An implementer
following N1, N2, N5 or N10 implements from that specification: N10 (line 232)
names three scaling inputs, N1 (line 223) the exact bound condition and which
value feeds which consumer. Reading published material is not banned and never
was (DEC-014, DEC-016: ideas are used freely), and the descriptions are prose,
not code (see the rules check). But the house standard is one directory over —
S181's stamp certifies *"No Lynx source file, evaluation table or test data
was read (DEC-016)"* — and nothing in AGENTS.md says who may implement what an
analysing agent described.

What it changes: proposed decision 1 (§7) should say it outright: every §3.2
row is a prose description of a published mechanism; a step drawn from it is
implemented by a subagent whose brief carries the prose, with every constant
fitted here in DEC-134's forms; and the coordinator states in the stamp that
it was.

### 2026-09-19_study_review-F04 medium §5's two move-picker explanations contradict the commit chronology, and the +37.65 rule was later removed as free

Status: closed -- the study's section 5 rows corrected in place 2026-09-19

Line 363: the record's "+37.65 skip bad noisy in qsearch" and "+14.08 split
noisy by SEE" were "measured *without capture history existing*". The record
says the opposite: noisy history replaced LVA on 2025-02-22, the SEE split
landed on 2025-02-23, the qsearch skip on 2025-02-25. Line 361 attributes the
"+29.79 fully staged move picker" (2025-03-27) to deferring losing captures
behind the quiets; the SEE split had existed for a month, so the deferral
cannot be what bought it, and the commit body carries no prose. And the
2026-01-30 "Simplify away bad noisy qsearch pruning" passed non-regression at
+0.52 ± 1.51 over 51,834 STC and −0.00 ± 1.16 over 77,828 LTC — the +37.65
rule is gone in the record's current form, and the study quotes the figure
three times (lines 187, 363, 506) without saying so.

What it changes: row 3's conclusion — S025 stays gated on S023 — survives and
is strengthened: those bad-capture gains were measured *with* capture history
present. Row 1's explanation should be struck; the ledger cannot say what the
staging bought. The +37.65 is a 2025 number for a rule the record no longer
carries.

### 2026-09-19_study_review-F05 medium The trap table omits that the record replaced internal iterative reduction, chesso's next open step

Status: closed -- DEC-222 (2): S095 re-formed as the LMR term; the study's section 3.1 row corrected 2026-09-19

Line 176 prices S095 (Open position 2) at +5.11; §4 lists nothing. 2025-12-30 decoupled IIR from move-loop pruning (+0.14 over 68,504); 2026-02-01 restricted it to cut nodes (+0.04 over 87,552); **2026-02-15 "Replace IIR with a LMR reduction"** passed simplification
bounds at −0.32 ± 0.78 over 195,882 STC and +0.52 at LTC. "Increase
reduction for cut nodes without a TT move", +2.24 over 40,402, is the
shape the node-level cut was folded into. The §4 table exists, in the study's
words, so chesso does not "pay for a verdict twice"; the omission sits on the
next step.

What it changes: S095 is re-formed before it runs (P2). The mechanism's value
is real in the record; its *form* was decided there by measurement, and
chesso has the surviving form's site already (`lmr_node_adjustment`, S098 v2,
four tested terms).

### 2026-09-19_study_review-F06 medium S099, the step the study promotes, still seeds from Stockfish commit-message prose, which DEC-134 rules out by origin

Status: closed -- S232, 2026-09-20: the three files reseeded in DEC-134's forms, the Stockfish prose figures moved to anti-seed paragraphs, S110's twelve figures struck

`adocs/plan_todo/S099_correction_history.md:207-212`: "applied correction =
`entry / 32` with the max adjustment ~32 internal units (implying an entry
clamp near 1024 in SF's scale) — SF commit message prose." DEC-134 says a
constant quoted in another engine's commit message belongs to it, and reseeded
seven files (S095, S097, S098, S109, S113, S114, S132); S099 was
not on the list. S111 cites Stockfish PR as its source (`S111:18`) and
S110 carries twelve unsourced figures (literature check A22). The file's hedge
— "take the shape, not the number" — does not remove the numbers from the page
an implementer reads. Not the study's error; a breach its first recommendation
walks into.

What it changes: the family cannot start, in the reserve or out of it, until
its three seed sections are rewritten in DEC-134's three forms. One document
step, no machine time.

### 2026-09-19_study_review-F07 medium "≈+73 Elo from fourteen SPSA sessions" sums LTC passes, six of the sessions are SMP, and each is a week of this machine, not a night

Status: closed -- DEC-222 (8): SPSA as a per-block cadence in chesso's lane size, verified at 8+0.08; the study's section 3.5 corrected 2026-09-19

Lines 307-319 and 437-440. Of the fourteen SPSA-titled commits: six are
titled "SMP SPSA session", verified at multiple threads — the arena §3.4 rules
out; seven carry an STC run beside the LTC one and **five of those STC runs
are negative** (−1.64, −1.49, −6.87, −6.52, −1.43); the +73 is the sum of the
LTC pass of each session. The titles state 28k to 200k *tuning* games on top
of the verification — 13 to 91 hours at chesso's throughput, several at
40+0.40. Chesso's verdicts are at 8+0.08 only (DEC-083), its longer-control
instrument is a fixed 1000-pair estimate at 545 games an hour (DEC-202), and
its one shipped SPSA, S085, was tuned at 2+0.02 and verified once at 8+0.08 —
the same gap in miniature. Chesso's own lanes (S222 8 h 37 m, S231 estimated
8 h 45 m) are the right size for a night; the record's sessions are not the
model for one.

What it changes: proposed decision 4 is under-priced by its verification and
over-counted by its SMP sessions. The recurring cadence is affordable in
chesso's lane size; what has to be decided first is how a returned vector is
verified when a real longer-control gain can read zero at 8+0.08 (P7).

### 2026-09-19_study_review-F08 medium "A non-regression on a truly free change stops fast" is contradicted by both records

Status: closed -- DEC-222 (7): removals fold into the landing step, a night each; the study's section 4 corrected 2026-09-19

Line 340, repeated in Tier-4 item 14 (lines 441-445). The record's 408
non-regression runs total 16.46M of its 41.2M recorded games — **39 % of the
games for 38 % of the runs, mean 40,347 games**; its 151 STC runs at
`[-2.75, 0.25]` have a median of 34,548 (p90 94,776), against 40,458 for its
`[0, 3]` gainers. For chesso the arithmetic is fixed by DEC-143: a free
removal has truth 0, which sits *on* the H1 bound of `{-5, 0}`, priced at
25,591 expected games, 11 to 12 hours; the ledger's S210 F22, a non-regression
whose truth sat on the bound, walked 28,598 games in 13 h 16 m. "A quarter of
the record's budget" is a quarter of a distributed instance's budget, and the
deleting runs are the long ones.

What it changes: the simplification lane (P6) is sound in kind and wrong in
price; each removal is a night, and candidates are chosen for what they buy,
never run as a sweep.

### 2026-09-19_study_review-F09 medium §4 misses later simplifications on four of the six Tier-2 recommendations

Status: closed -- DEC-222 (5): S234, S235, S237 and S238 describe the form the record carries now; the study corrected in place 2026-09-19

All verified in the record at simplification bounds. N5 hindsight: 2025-07-11 (+0.99 over 33,724) and 2026-02-20 "Simplify away
hindsight reductions for FDS" (+0.38 over 58,266) — the mechanism survives in
the record's current form, reduced. N11: "Update quiet history on TT hit" (+3.51
over 18,786) had its formula simplified by 2025-11-19 (+0.43 over
56,136), and its cousin, PCM updates on TT cuts (+2.28), was
removed by 2026-01-31 (+0.80 over 37,198). N10: its raw-eval input
was removed by 2026-03-15 (+0.59 over 49,280). N8: its
captured-piece margin was removed by 2026-03-22 (+2.20 STC over
20,832, +0.04 LTC). Quiescence LMP and futility (lines 192-194, 247):
simplified in three later commits (the last +0.06 over 72,652).
None of these is a removal of the technique except the PCM-on-TT-cut cousin;
all say that the form to build is the current one, not the 2025 landing's, and
the study describes the landings.

What it changes: every §3.2 row that becomes a step describes the mechanism as
it stands in the record's current form, and §4 gets these rows.

### 2026-09-19_study_review-F10 medium The headline game count is the first run per commit, and the parser behind every number is not in the tree

Status: closed -- the parser committed as `adocs/data/2026-09-19_technique_ledger.py`, the headline count corrected 2026-09-19

Line 102: "794 passing SPRT runs since 2025-02 consumed at least 31,493,134
games" — exactly the sum of the *first* recorded run per commit; all recorded
runs total 41,964,262 (≈19,100 machine-hours at 2,200 an hour, 2.2 years, not
14,300 and 1.6). Conservative in direction, wrong in description. The cost
table uses 642 of 682 STC runs without saying which were dropped; over all
682 the rows read 22 / 25 / 73 / 102 / 135 / 325 runs, medians 1,342 / 4,276
/ 8,688 / 16,814 / 32,354 / 54,466 — the same curve. "233 of 788" (line 93)
is not reproducible (206 / 218 / 252 of 796 by three definitions). Lines
60-62 put `recs.json` "in the session scratchpad"; Appendix B gives the `git
log` command and not the parse.

What it changes: lines 93 and 102 corrected; the parser committed under
`adocs/data/` with a README row, so the ledger is a script and not a
transcript (DOCS; DEC-142's spirit).

### 2026-09-19_study_review-F11 medium Tier-1 item 3's pair for S188 would ship an extension on a zero, and the form behind the record's datum is unknown

Status: closed -- DEC-222 (3): S188 keeps `{0, 5}` with its prior, worst-case walk and abort rule recorded; the study corrected 2026-09-19

Lines 398-400 propose running S188 "as a `{-5, 0}` non-regression against a
stated prior of zero". That pair accepts H1 on a truth of 0 — after the full
25,591-game walk — and the step would keep an extension three engines are
said to have measured as free to remove. Line 334 calls the record's removal
(2025-04-17, −0.26 ± 1.49 over 58,032) a "third independent datum"
against the in-loop form, which is the only form S188 is. The message does
not say which form: the extension moved inside the loop in
2024-03-11, back before the loop in 2024-03-27, and the
2025-02 rebuild re-added it (2025-03-15) with no form stated.
DEC-133 (a) already ruled Ethereal's pre-loop removal irrelevant to S188 for
exactly this reason.

What it changes: S188 keeps DEC-133's shape — one `{0, 5}` SPRT, whatever it
returns — with the pre-registration recording the prior as small-or-zero, the
worst-case walk, and the record's datum as "form unknown" (P3).

### 2026-09-19_study_review-F12 medium Tier-2 item 10 invokes DEC-082 for a case DEC-082 excludes

Status: closed -- DEC-222 (5): the history-update family is four candidates, not a DEC-082 block; the study corrected in place 2026-09-19

Lines 420-424 bundle N9, N10, N11 and N12 as one block "under DEC-082's
block-form reasoning". DEC-082's precondition is that *the parts are inert in
isolation*; the study's own evidence is that each part measured positive
separately (+4.67, +3.87, +3.51, +4.35). The argument actually made — four
verdicts "crawl at the bounds individually" — is a budget argument DEC-082
does not authorise, and DEC-082 requires the step file to state that per-part
attribution is deliberately forfeited. Chesso's nearest case cuts the other
way: S098 verdict 3 read −9.97 whole and +5.75 for one half (`plan.md`
ledger).

What it changes: the history-update family is four candidates, each one
verdict, or a block that says in its `accepts` what it forfeits; not a
DEC-082 case.

### 2026-09-19_study_review-F13 low Tier-3 item 11 has no price in the record, and the accumulator alone tests rounding only

Status: closed -- DEC-222 (6): S236, one block bisected on H0; the study corrected in place 2026-09-19

Lines 259-269 and 428-432. The record's fractional LMR predates its ledger:
"Fractional LMR" 2024-03-19 and "Fully fractional LMR" 2024-08-17 carry a
`Bench:` line and nothing else; 2025-11-26 then
replaced the float table with an integer ilog2 reduction (+0.26 over 66,872).
So the +9.09 for history-scaled LMR (2025-03-13) was indeed measured
on a fractional reduction — that leg holds — but nothing prices fixed point
itself. DEC-213 removed chesso's history term, so converting the accumulator
alone changes only the rounding of what remains; the hypothesis ("truncation
killed a fractional contribution") is only tested with the term restored as a
fraction, which is a second change.

What it changes: the item is a hypothesis test with no published prior. It is
still testable in one block — accumulator plus fractional history term, whose
parts *are* inert alone, bisected on H0 — and P5 states it that way.

### 2026-09-19_study_review-F14 low Four phrasings gesture at a scale in the record where the study promises shape only

Status: closed -- the four phrasings reworded to shape only, 2026-09-19

Line 251 "fixed point — thousandths of a ply" (the accumulator's scale); line
231 "the second quiet tried is punished nearly in full, the twentieth barely"
(a decay length); line 247 "LMP after a couple of moves" (a move threshold);
line 228 "converted to 0, 1 or 2 plies by thresholds" (an output range). None
is a tuned integer and none is offered as a seed, so the study's statement
that no constant from the record appears holds on its own terms; but each came
from the record and each is the kind of hint DEC-134's reasoning is about — a
seeded vector converges near its seed. The cold reviewer rates this medium;
the coordinator rates it low because an implementer still has to fit every one
of them and the numbers named are units and ranges, not values.

What it changes: reword the four to shape only in P8's correction commit.

### 2026-09-19_study_review-F15 low Three cells quote a figure at a control or bounds pair other than the column header says

Status: closed -- the three controls relabelled in place, 2026-09-19

Line 178: "+9.19 NMP restricted to cut nodes (3934, at 6+0.06)" ran at
`[-5, 0]`, a non-regression pair; ±6.22. Line 225: "+3.77 in futility" is the
LTC figure; STC read +1.81 ± 1.46 over 57,166. Line 338: "+4.15"
for removing the move count from LMR is the LTC figure at `[-2.75, 0.25]`;
STC read +3.23 at `[0, 3]`. Ordering impact nil.

### 2026-09-19_study_review-F16 low The killers row skips two intermediate commits that strengthen its own reading

Status: closed -- the killers row carries the two intermediate commits, 2026-09-19

Line 332: "Added at +2.14 in March, gone in June." Between them: "Decrease reduction for killer moves" +7.64 over 7,054 (2025-04-07) and
"Extract killer moves into its own stage" +2.50 (2025-05-06); then
the removal at +0.50 ± 1.61 over 47,676. The value was absorbed by the history
sum and the reduction terms that landed between — the study's reading, minus
its evidence.

### 2026-09-19_study_review-F17 low N8 depends on a picker stage chesso does not have

Status: closed -- N8 waits for S023 and S025, said at Tier 2 item 8, 2026-09-19

Line 230 and Tier-2 item 8 (line 415): bad-noisy futility "fires in the
bad-noisy stage of the picker". Chesso has no such stage — losing captures are
searched in order (S025, reserve, gated on S023). N8 has nowhere to fire until
both exist.

## Rules check: the document itself

**COPYING and DEC-104.** Nothing in the study is adapted from another
project's source: every mechanism is described in prose and implemented here
from that description, so DEC-104's adaptation clause is never reached and
COPYING's ban — which is stricter than any licence — is not touched. Proposed
decision 1 (§7) is sound and should gain F03's sentence about the implementer.

**Constants (DEC-084, DEC-105, DEC-134).** §3.2's twenty-five rows scanned for
digits outside Elo, game-count, control and step-id patterns: the residue is
"0, 1 or 2 plies", "157,644" (a game count) and two chesso step ids. §3.1, §4
and Appendix A quote Elo, games, controls and bounds only — facts about tests,
not engine constants; an SPRT bounds pair is a methodology choice and adopting
one is not copying. The four soft phrasings of F14 are the whole residue. The
study's §0 statement **holds**, with F14's rewording owed.

**Prose versus code.** N1–N25 describe mechanisms at the level of what is
read, what is written and what it changes; none is transcribable as code. N2
and N5 are the most detailed and are still shape. Holds — subject to F03's
provenance labelling.

**Chess judgement (CHESS, DEC-023).** None. Every technique is valued by a
measured Elo. Holds.

**Training data, tables.** None touched. Holds.

**Use of another engine's numbers.** Elo figures and game counts from
another engine's commit messages are facts about the tests they record;
DEC-019 governs their use and the study cites it in the right places — and
then carves variance out of it, which is F01.

## Plan proposals

Each is an option set with a recommendation; the owner decides, one item at a
time. Prices are DEC-143's, not the record's (F01).

**P1 — The correction-history probe runs on the next spare night, after a
reseed, under the decision that already permits it.** DEC-133 made S099 "the
probe for the correction-history family, run on a spare night when the
machine is idle" and DEC-176 (c) confirmed the placement. No reordering is
needed to take the probe; what is needed is F06's reseed of S099, S110 and
S111 in DEC-134's forms (one document step). The record adds two things to the
case: the family measured large and cheap *there* (+9.18 / +11.81 / +6.90 /
+13.85 at 3,000 to 8,300 games), and the correction applied in quiescence and
the correction magnitude as a margin input are two consumers chesso has no
step for. Options: (a) run S099 as the probe on the next idle night after the
reseed, S110 and S111 gated on its verdict as DEC-133 wrote, and create the
two consumer steps only on S099's H1; (b) ask the owner to revisit DEC-176 (c)
and move the family into the main order on cost grounds, acknowledging that
the record is a network engine rated far above chesso's band and so more
above-band evidence, not less; (c) leave as is. **Recommend (a)**: it costs
one night and one document step, changes no owner decision, and its verdict is
what (b) would need anyway.

**P2 — S095 is re-formed before it runs (F05).** Options: (a) as written, a
node-level depth cut, `{0, 5}`; (b) one more ply of reduction in
`lmr_node_adjustment` when the node has no table move — the site S098 v2
built and tested — one verdict, S095's id kept and its `goal:` amended by a
decision; (c) both forms, two verdicts. **Recommend (b)**: the form the record
kept after measuring both, a one-line term in tested code, one verdict; (a)
stays available if it reads H0.

**P3 — S188 keeps DEC-133's shape, with the prior written down (F11).** One
`{0, 5}` SPRT whatever it returns; the pre-registration states a prior of
small-or-zero from Ethereal's pre-loop removal, Stormphrax's (form and Elo
unstated, network engine) and the record's (form unstated), the worst-case walk
of 41,861 games, and an abort rule. Options: (a) as stated; (b) retire to the
reserve on that evidence and spend the 12 to 20 hours on P4; (c) the study's
`{-5, 0}`. **Recommend (a)**: the owner reopened S188 deliberately eight days
ago and the new datum does not establish the form; (b) is the owner's call if
the hours matter more than the answer.

**P4 — S132 moves up, then four Tier-2 steps are created.** S132 (time
management by node share) owns the two best Elo-per-game rows in the record's
whole ledger (F02) and every hand-crafted engine at the band has the form;
move it from position 12 to directly after S097. Then four new steps, one
verdict each, in this order: N1 (static eval tightened by the table score at
every margin site; subsumes S130's narrow form, +1.14 ± 4.04, kept); N2
(fail-middle at the reverse-futility return; one weight fitted here; further
sites later one at a time); N5 (hindsight reductions, in the reduced form the
record now carries per F09; the parent's reduction is a first-class quantity
after S098); N4 (cutoff count). N7 (threat-indexed history) waits for a threat
map that S101 also wants — one step builds the map and prices it, then both
consume it. N8 waits for S023/S025 (F17). The history-update family (N9–N12)
is four candidates, not a DEC-082 block (F12). Every seed in DEC-134's forms;
every brief carrying the prose description (DEC-221). Placement: after P1's
probe and P5, before S112. Price: DEC-143's, 12 to 20 hours each worst case.

**P5 — The LMR accumulator moves to fixed point and S098 v1's history term is
re-tested as a fraction of a ply, as one block (F13).** On its own the
accumulator changes only rounding; the fractional term cannot exist without
it; the parts are inert alone, which is DEC-082's precondition, and the
`accepts` says so. One gainer SPRT, bisection on H0 (accumulator alone, then
with the term). Placement: before P4's N5 and N4 and before any corrplexity
step, because each wants a fraction of a ply. No price for it exists in the
record; it is a hypothesis test about a recorded chesso zero (DEC-213), and
the block design is what lets it discriminate.

**P6 — A simplification lane, priced at a night per removal (F08).**
Candidates: the killer slots once S231's fitted stack is in (+0.50 on removal
in the record; S149 and S159 already stopped investing); the countermove table
(subsumed by continuation history in the record, no number); delta
pruning (S022, already a step). Each is a `{-5, 0}` run whose truth sits on
the bound: ≈25,600 games, 11 to 12 hours. Selection rule: a removal is
scheduled when it saves measurable nodes per second or memory, or unblocks a
later step; never as a sweep. Options: (a) one reserve entry per candidate,
spare nights; (b) fold each into the step that makes it redundant as a second
verdict; (c) none. **Recommend (b)**: the redundancy claim is exactly what
the landing step has just measured.

**P7 — SPSA cadence: recurring at block boundaries, in chesso's lane size,
with the verification decided first (F07).** S127 stays as the full run after
the search block. Add a cadence: one lane per completed block over the axes
that block added, sized like S222's and S231's (eight to nine hours), never
the record's 28k-to-200k-game sessions. Pin the verification: an `{0, 5}` SPRT
at 8+0.08 of the returned vector against the incumbent (S085's shape), read
beside DEC-202's block-boundary 1000-pair estimate at 32+0.32, with the
recorded risk that a real longer-control gain can read zero or negative at
8+0.08 — five of the record's seven dual-control sessions did. Options: (a) as
stated; (b) verify by SPRT at 32+0.32, four times the cost; (c) S127 once.
**Recommend (a)**.

**P8 — The study is corrected in place, one document commit**, dated section
at its end: F01's cost claim and every §6 hour figure withdrawn in favour of
DEC-143's; F02's DEC-133/DEC-176 reading; F03's provenance label on §3.2;
F04's two rows and the removal; F05's IIR rows in §4; F07's SMP and
STC readings in §3.5; F08's medians in §4 and Tier 4; F09's rows in §4;
F10's lines 93 and 102; F11's pair and the form note; F12's DEC-082 reading;
F14's four phrasings; F15's three labels; F16's two commits; F17's dependency;
the parser committed under `adocs/data/` with a README row. The study is a
data document and takes the edit; `plan_done/` is not touched.

## The SPRT result in the commit message

The owner's idea, read as the practice the record follows: every commit that
carries a measured change also carries the harness's result block — Elo with
interval, control, threads, hash, LLR with bounds, games with W/L/D,
pentanomial — so `git log` is the measurement ledger. Three readings were
considered; the first is the one recommended.

**(A) The verdict block in the commit.** The record can put it in the *landing*
commit because the test precedes the merge. Chesso cannot: the candidate of an
SPRT is a committed sha (DEC-020 made attribution depend on it; `fastchess.sh`
pins both shas in its banner), the verdict arrives hours later, and the GIT
rule forbids amending. Chesso already has the other half of the practice:
DEC-140's `Bench:` line and `tools/gate.sh` checking it. Today a verdict is
recorded in a "Record …" commit in prose (73 such commits; `3fa692b` is
typical), in the step file, in `status.md`, and in `plan.md`'s ledger table
followed by eight hand-written "With X the ledger holds N" paragraphs whose
means, medians and games-per-hour are re-typed each time — the class of
number the 2026-09-04 plan review found stale (its F03). fastchess's final
block is in every `adocs/data/S*_sprt.log`.

What adopting (A) buys: the ledger becomes re-derivable from git alone by a
script — what this study did to the record and chesso cannot do to itself; the
eight running-total paragraphs become one generated table; `tools/gate.sh`
checks the block's shape and cross-checks its two shas against the `Results
of cand-<sha> vs ref-<sha>` line of the log the commit names; every verdict —
H1, H0, no verdict — is recorded in the same form, which is what keeps chesso's
ledger from acquiring the passes-only bias the study had to work around in the
record (§1.1). What it costs: a format (below), a gate clause of about
forty lines of bash, a ledger script of about a hundred lines of Python, and a
seed file for the twenty verdicts already taken, which are never rewritten
into history. No machine time. Risks: duplication with step stamps (accepted:
the commit is the machine-readable record, prose stays prose); a block pasted
from the wrong run (the sha cross-check catches it); format drift (the gate is
the fence).

**Verdict on (A): adopt**, as a clause of the COMMITS rule with a decision, in
chesso's own shape — fastchess's line names, nElo as the bound scale, the
book, forfeits and wall time, which chesso records and the record does not.
Draft clause, for the owner to accept, amend or refuse:

> A commit that closes an SPRT verdict — H1, H0 or no verdict — carries, after
> its body and before `Bench:`, the result block of the run it closes, in
> fastchess's own line names: `SPRT | cand <sha> vs ref <sha>, <tc>,
> Hash=<n>, <book>, {elo0, elo1} nElo`; `Elo | <x> +/- <y>, nElo <x> +/- <y>`;
> `LLR | <l> (<a>, <b>) -> H1|H0|none`; `Games | N: <n> W: <w> L: <l> D: <d>,
> Ptnml [<5>]`; `Wall | <h> h <m> m, <g> games/h, forfeits <f>`; `Log |
> adocs/data/<file>`. `tools/gate.sh` refuses a block whose shas do not match
> the named log's result line, and `tools/ledger.py` regenerates `plan.md`'s
> ledger table from `git log`; the running-total paragraphs are retired to
> that script's output. From the decision's date on; the twenty earlier
> verdicts are seeded from `plan.md`'s table once and never rewritten.

**(B) An STC and an LTC verdict per change.** The record does it on a
distributed instance, and even so only 218 of 796 commits carry both. Chesso's
longer control runs at 545 games an hour (S151) against about 2,130 at 8+0.08;
a per-change LTC verdict would quadruple the ledger's mean. DEC-202's fixed
1000-pair reading at each block boundary is the affordable form and already
exists. **No change.**

**(C) Simplification bounds as a lane.** Covered by P6 and F08; the pair
question (`{-5, 0}` versus a shifted pair) is a DEC-063 question for the owner
and should wait for the first candidate removal. **No rule change now.**

## Checked and clean

- Every Open-list position the study names (S095 2, S097 3, S188 4, S112 5,
 S113 8, S116 11, S132 12, S120 17, S119 18, S118 29, S127 37, S099 40, S023
 41, S025 42, S110 43, S111 44) matches `plan.md` at the reviewed HEAD.
- `plan.md`'s class prices (2 h 31 m / 6 h 17 m) and the 2,200 games-an-hour
 figure (ledger 2,233.9 across the set; 2,110–2,155 on the current book) are
 as quoted.
- DEC-104's adaptation clause is quoted correctly and its Consequences say
 what the study says they say.
- The §3.1 technique-to-step mapping is correct in every row checked; S022
 and S134 are indeed the only pending steps whose goal admits deletion.
- The rebuild base had a network: "Introduce NNUE" is 2023-12-29.
- "No commit prices a hand-crafted evaluation term" — confirmed by subject
 scan; the eval-sounding commits are move-ordering.
- The fractional-LMR chronology behind §3.3's S098 hypothesis holds: the
 +9.09 was measured on a fractional reduction (F13).
- §3.4's exclusions are consistent with DEC-054, DEC-071 and DEC-089 — except
 §3.5's silent re-import of SMP sessions (F07).
- The study makes no chess judgement.
- No constant from the record is reproduced in this report; the one grep that
 returned tuned integers was discarded.

## The cold reviewer's pass, finding by finding

Twelve findings returned; each re-verified by the coordinator before merging.

| cold # | claim | verdict | where |
|---|---|---|---|
| 1 | the record's game counts do not price chesso verdicts; DEC-019 covers variance; `plan.md`'s transfer ratios | **confirmed**, promoted to the top | F01 |
| 2 | §6 item 1 misreads DEC-133; DEC-176 (c) ruled; S132 is the better-priced promotion | **confirmed** (DEC-133 Rejected line and DEC-176 (c) read; the rows re-derived from the record) | F02, P1, P4 |
| 3 | §5 chronology reversed; the +37.65 rule was later removed | **confirmed**, merges with the coordinator's own finding | F04 |
| 4 | IIR replaced | **confirmed**, merges with the coordinator's own | F05 |
| 5 | SPSA: six SMP sessions, LTC-only passes, 13–91 h of tuning games | **confirmed**; the coordinator had the STC readings and missed the SMP count | F07 |
| 6 | Simplification lane costed backwards; ~40k games a run | **confirmed** (408 runs, 16.46M games, mean 40,347 — the reviewer's 345 / 15.33M used a narrower filter, same conclusion) | F08 |
| 7 | §3.2's mechanism descriptions carry no provenance label | **confirmed as medium**, not medium-high: reading published material is allowed by DEC-014/016; what is missing is the label and the rule that an implementer works from the description | F03 |
| 8 | Later simplifications on N5, N11, N10, N8, qsearch LMP/futility | **confirmed**, all eight commits and figures match; wording tightened — most are simplifications of the form, not removals | F09 |
| 9 | the record's check-extension removal is of unknown form | **confirmed**; merged with the coordinator's pair finding | F11, P3 |
| 10 | DEC-082 misapplied to N9–N12 | **confirmed** | F12 |
| 11 | Fixed-point LMR has no price in the record; accumulator alone tests rounding | **confirmed** on the price and the chronology; the "cannot discriminate" half is answered by the block-and-bisect design | F13, P5 |
| 12 | Four phrasings pin constants | **partially**: the phrasings exist and are reworded in P8; rated low, not medium — units and ranges, not values, and none offered as a seed | F14 |

Its "checked and clean" list agrees with the coordinator's on every item.
