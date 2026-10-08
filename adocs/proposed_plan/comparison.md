# Comparison: the current plan and the proposed road to 3000

Date: 2026-10-07. Original comparison by Claude at `c5fd1f2`; corrected by
Codex at the owner's request, with an independent read-only review.
Current plan: `adocs/plan.md` and its 29 pending S files at `c5fd1f2`.
Candidate: `adocs/proposed_plan/plan.md`, `report.md` and P01–P29 at
`e98419b`. This file is a recommendation; adoption is a separate plan change.

**Goal: at least 3000 CCRL Blitz 1CPU, without NNUE or any network.** MIT,
independent implementation, own fitting data, SPRT, the repository's testing
and documentation gates, and the owner's recorded decisions all apply.
DEC-071, DEC-089 and DEC-179 define the goal. A disappointing block triggers
more hand-crafted work; it does not reopen NNUE before the mark.

## 1. Recommendation

**Keep the current plan as the base and merge selected tooling and measurement
improvements from the candidate.** Refresh its baseline and remaining-work
arithmetic; add an early search-block SPSA lane; build the trace tuner and
stronger rating anchors; decide the lazy shortcut and clamp through separate
experiments before fitting richer mobility and king safety. Preserve the
existing S files' tests, provenance, dependencies and separate verdicts.

The candidate identifies useful infrastructure and an evaluation block worth
pursuing. It also bundles changes that the current plan measures separately,
omits open work, and proposes experiments beyond recorded scope. The current
plan has a stronger implementation record but stale summary figures and an
unlisted tuning cadence. Neither document proves that its remaining features
will supply the approximately 234 points still needed.

Correction history remains a bounded reserve probe unless the owner changes
its priority. Further histories, extension refinements, complexity and
specialised endgames remain conditional candidates. Published results can
support trying them; they cannot establish a gain in chesso.

## 2. Corrections to the original comparison

The source of truth is the code and pending step files, read alongside the
specific decision entries. Completed steps are evidence and are not amended.
The following replaces the original comparison's strongest claims.

| original claim | corrected reading | evidence |
|---|---|---|
| C1: the plan is still based on 2559 | Confirmed. The latest measured baseline is approximately **2766**, with an interval no narrower than about ±60; it belongs to engine `1680439`, not a fresh measurement of today's HEAD. The central gap is 234. Completed search gains must be removed from remaining-work estimates. | `adocs/specs.md`, measured-strength paragraph; `adocs/data/rating_2026-09-27_S240_ccrl_blitz.md`; current plan's opening and Elo arithmetic |
| C2: four evaluation steps all fit under a soon-retired clamp | Only mobility among those four is inherently in the existing mobility-plus-king-safety clamp. Passers and current pawn structure belong to the cheap stage; S101 requires an explicit stage choice. Avoid a mobility fit immediately before changing its score function, but do not describe all pawn fits as clamped. | `src/evaluation.cpp` `evaluate_cheap`, `evaluate_expensive`; S101/S121 accepts |
| C3: S120 shows that the clamp should be retired | **False.** S120 disabled the shortcut in a throwaway build **while keeping the clamp at 184**. Its −5.01 % nps, −7.5 % nodes and −2.64 % wall time to depth 11 are timing evidence for that candidate, with no SPRT. DEC-257 keeps the shipping shortcut. Clamp removal remains unmeasured. | `adocs/data/S120_measurements.txt`, “Exact everywhere”; DEC-257; S120 section (c) |
| C4: an early SPSA lane is owed | Confirmed: DEC-222 (8) specifies a lane per completed block, with S127 as the last run. Add an explicit bounded lane over the search block's added, reachable axes and an independent verification SPRT. Eight to nine hours is a sizing constraint, not proof that every added axis can be fitted together in that time. | DEC-222 (8); S127; `DEV_MANUAL.md`, SPSA sizing/reachability |
| C5: promoting S099 “reverses nothing” | **False.** DEC-222 (1) retains the reserve placement, allows a probe on an idle night, and explicitly rejects promotion into the main order on cost grounds. Scheduling a reserved probe window is a proposed priority amendment. S110/S111 and correction consumers are gated on **S099's H1**. | DEC-133; DEC-176 (c); DEC-222 Decision/Rejected |
| C6: the trace tuner is needed | A sound tooling proposal. Existing gradient and recovery tests already protect the current tuner. A trace reduces duplicated feature extraction; nonlinear transforms still need a model, derivatives and tests. It does not make missing gradients impossible. | `tools/tuner_model.hpp` header; `tests/test_tuner_gradient.cpp`; P02 |
| C7: PEXT is about 1 % of perft and should go to the tail | The step's own quoted 12.8 versus 13.5 seconds implies **5.19 % less time, or 5.47 % more throughput**, not 1 %. The quote still does not measure whole-search speed here. Keep S032/S030 as bounded neutral experiments, ordered by local timings rather than an unsupported estimate. | S032 “Measurable here”; INV-6; DEC-083 |
| C8: merging corpus steps saves a redundant verdict | It saves a verdict by changing what can be attributed. S082 changes the sampling/leaf recipe; S083 changes scale or generation effort. S083 explicitly requires two verdicts if both size and node budget change. A joint recipe can be proposed, but cannot silently inherit acceptance under “one change at a time.” | S082 accepts; S083 “One change at a time”/Measurement |
| C9: no anchor can show 3000 | Stronger anchors are needed for reliable coverage. Existing anchors do not mathematically cap an estimated rating at 2830, but a 3000 estimate would extrapolate beyond the current set. Preserve S152's **two controls** and add a manifest-preparation task. | S240 reading; S152 accepts/excludes; P04; `rating.sh` `manifest`, `tc`, `rounds` |
| X9: king safety is worth +24.8 in three isolated changes | The cited +10.86 Stash patch also changes the mobility zone. Its sum with the earlier king-safety tests is not an isolated king-safety estimate. Dependencies and local cost decide the order; the primary record supplies direction. | [Stash changelog](https://raw.githubusercontent.com/mhouppin/stash-bot/master/CHANGELOG.md), v31/v32 |
| merged order: S122 before S118 and S101 | Invalid for the retained S122: it consumes S118's shelter/storm cache and follows S101 so attacks are not double-counted. Importing only P15's attack sets does not supply those cache entries. | S122 accepts/Interactions; S118 Interactions |
| retained S136 is already current after S055 H0 | Its goal was corrected, but its accepts and old citations still assume a successful single-taper S055 in places. Re-derive from HEAD's divisions and refresh pending prose when adopted; do not relax the model test from the stale numbers. | S136 accepts; citation checker notes; `src/evaluation.cpp` `evaluate_mobility_and_king_safety` |
| 21–28 verdicts, 185–325 hours | Unsupported: required exclusions, pawn groups and scaling cases were omitted, and the second rating control was not priced. Replace with the explicit inventory in §6. | S121/S123/S124/S125 accepts/Measurement; S152 |
| 0.83 replaces the current plan's “wrong” 0.38 discount | The ratios describe **different transfers**. 0.38 maps another engine's published gain to a local gain. About 0.83 is +207 gauntlet points divided by about +250 summed local stopping estimates, from one interval with changed rating settings. They are not competing calibrations. | S183 inputs; S240 “Reading it”; specs measured-strength history |
| T2 may reopen the network below 2900 | Incompatible with the requested goal and DEC-179, which explicitly rejects a pre-3000 network trigger. Remove this escape route. | DEC-179 Decision/Rejected; owner's request |

The PEXT arithmetic is ordinary timing arithmetic, not a new benchmark:
`1 - 12.8/13.5 = 5.19 %`; `13.5/12.8 - 1 = 5.47 %`. Its source and applicability
still need checking before the experiment. The current file's “about 1 %” is
not evidence for moving it behind every evaluation fit.

The original comparison records the owner's choice of the Linux workstation.
This review runs in an ARM64 MacBook checkout whose `.moltke.local.md` describes
an M1, eight logical cores and different tools. Retain **2110 games/h as the
historical workstation budgeting reference**, not a current measurement here.
PEXT and Linux huge pages are target-specific experiments; an allocation hint
may fail normally. At run time use the selected machine's local notes, check
power and tool availability, and perform DEC-143's A/A after a harness or
machine change. A machine change does not automatically erase old evidence.

## 3. What to import, preserve and defer

| candidate item | relation to the current plan | recommended disposition |
|---|---|---|
| P01 incremental keys | S099/S110/S118 already require the relevant keys | Share the pawn-key implementation and prove make/unmake/FEN invariants. Add each non-pawn key when a kept consumer needs it; unused keys have a per-node cost. A standalone neutral prerequisite is reasonable, with its timing recorded. |
| P02 trace tuner | New tooling supporting S121–S126/S133 | Import with exact integer reconstruction, smooth-model gradient tests, recovery, freeze groups, provenance and `constexpr int` output. Compare against the existing tuner before shipping any changed weights. |
| P03 corpus/refit | S082 plus S083, with a fixed validation set | Import by-game sampling and split hygiene. Keep separate S steps/verdicts by default. A joint replacement requires a decision naming the attribution sacrificed and preserving all leaf guards. |
| P04 stronger anchors | Prerequisite missing from S152's allowed touches | Import preparation and smoke checks for multiple independent families around and above 3000. Binary tool use is allowed by DEC-016/069; new dependencies still need DEPS approval. |
| P05 pawn correction | S099 | Keep S099's described form and reserve probe. P05 also applies correction to quiescence immediately, which DEC-222 reserves for a separate consumer after S099 H1. Do not silently widen the probe. |
| P06 extra correction tables | S110/S111 | Keep the S099-H1 gate. P06's continuation index differs from S111's two/four-ply form; reconcile that design before implementation. P06's text does not consistently enforce the H1 gate. |
| P07 capture history/bad captures | S023/S025 | Keep in reserve. Evidence is mixed: Weiss's cited STC loss and Stash's STC gain concern different implementations. A retry needs a changed mechanism, fitted scale or measured cost case; neither outside result decides chesso. Retain S025's timing gate. |
| P08 table buckets | S119 | Keep first. Import exact allocation for arbitrary accepted Hash sizes, explicit collision handling and same-position move preservation. Separate neutral prefetch/allocation work from replacement changes where practical. |
| P09 added quiet histories | Pawn history overlaps DEC-138; other subforms need their own scope/source review | Reserve; do not claim all three were specifically rejected by DEC-138. Obtain a description and new local reason before creating a step. |
| P10 capture LMR | New candidate, with capture-history dependency | Reserve. A version without capture history is a different pre-registered candidate, not an automatic fallback after S023 H0. |
| P11 double/negative extensions | Considered under DEC-138 | Keep deferred. S097's base-extension zero alone does not prove these variants useless, but they need scope reopening and descriptions before a run. |
| P12 correction magnitude in LMR | Conditional consumer contemplated by DEC-222 | Create only after S099 H1; keep separate from the table probe and other correction consumers. |
| P13 fifty-move damping | New candidate; changes static-evaluation input | Reserve pending a defined form and reach/cost reading. Reach is not Elo (DEC-239); apply outside position-only cached raw eval, preserve rule draws/mates and account for TT reuse with different clocks. |
| P14/P28 SPSA | DEC-222 cadence and S127 | Add the missing early lane; preserve S127 as final. Reachability and guards determine eligible axes, not “every parameter.” |
| P15 shared attacks/pawn cache | Attack reuse supports term steps; cache is S118 | Import attack sharing where it has consumers and preserves score identity. Keep S118 after S123/S125; do not ship an unused cache switched off merely because it was built early. |
| P16 clamp retirement | Broader than S039 | Propose a scoped amendment to S039; separate shortcut and clamp candidates (§4). Retain spread/corpus reading and findings F16/F25 until actually discharged. |
| P17 nonlinear king safety | S122 | Keep after mobility, pawns, S118, S101 and a sound clamp decision. Tuner support and attacking-position coverage are prerequisites. |
| P18 mobility | S121 plus pin/king/queen exclusions | Keep S121's separate curve/exclusion experiments. Imported pin handling or further exclusions add verdicts and must be named in the brief. |
| P19 passers | S123 plus wider path/rook features | Keep S123's three groups. Tool-labelled diagnostics are useful; wider independent features require scope/verdict accounting. |
| P20 pawn structure | S125, bundled differently | Reconcile term inventory/indexing; current code already has backward pawns. Preserve separate experiments until the owner approves a specific bundle. |
| P21 threats | S101 plus hanging/pawn-push/king threats | Keep S101's scope. Hanging and pawn-push terms are explicitly deferred by DEC-138 too; the original comparison cannot import them while rejecting other DEC-138 items. |
| P22 placement/outposts | S102/S135; candidate retains S134's redundant seventh-rank feature | Preserve S134 before fitting, then S135's three-feature bundle and bisection. Keep outposts distinct from space; bad-bishop/shielded-minor extras need the same scope review as other DEC-138 terms. |
| P23 space | S102's second verdict | Keep its own verdict, reuse existing fills, describe the feature before implementation. |
| P24 scaling/mop-up | S124 plus specialised endgames deferred by DEC-138 | Keep S124's two cases separately. Mop-up is an additional conditional step, with tool-established conversions and its own verdict. |
| P25 complexity | DEC-138/192 reserve candidate | Defer until a local evaluation limitation or new sourced reason supports the owner's reopening. No network fallback is involved. |
| P26 joint fresh-data refit | S126 | Keep after S133. A fresh corpus is an additional recipe/run decision, not something S126 currently requires; freeze the recipe if refreshing it. |
| P27 king-relative PSQT | S133, already owner-approved in DEC-087 | Preserve S133's small-bucket design and accumulator tests. It is not NNUE and needs no renewed approval merely for being king-relative. Coverage and bucket-change cost still matter. |
| P29 final rating | S152 | Preserve two controls, anchor dispersion, forfeits, binary stamps and archive/report rules. Stronger anchors prepare the measurement; summed SPRT points do not replace it. |
| omitted pending items | S259, S032, S030, S134, S136, S129 | Preserve or explicitly retire each. S259 is named in every pre-registration until closed under DEC-171. No silent omissions. |

Line counts are not a step's acceptance criterion. The enriched S files are
preferable because they carry tests, seed procedures and recorded decisions;
imports must retain those obligations regardless of length.

## 4. Technical conditions the merge must carry

**Lazy evaluation.** P16 permits retaining the shortcut after removing the
clamp that guarantees its bounds. That candidate must not inherit the current
bound guarantee. Recommended sequence, weights frozen: (a) test exact
evaluation everywhere with the clamp retained, the candidate S120 timed;
(b) if (a) is kept, test removal of the clamp independently. Each play change
takes an SPRT with its pair and H1/H0/no-verdict actions written first. The
timing supports trying (a), not shipping it. An alternative guaranteed shortcut
needs a demonstrated bound and separate scope; a sample census cannot prove
that every legal position respects it.

Settle this mechanism before fitting richer mobility/king safety. If a leg
fails, record the result and amend the dependent S122 design explicitly; do not
force the next step to claim unclamped scores under a retained clamp. Removing
`LazyEvalMargin` from the tune surface, retiring tests or changing the
truncation contract requires the named decision and SURFACE/DOCS updates.
Removing the clamp alone does not remove the taper divisions: S055 read H0.

**Corpus.** A PV leaf that is out of check and nonterminal can still be
unresolved. Preserve S082's direct assertion that quiescence would make no
further capture, and its rejection of depth-truncated walks; define policies
for aborted searches, bound-only lines, quiet promotions and terminal leaves.
The stored WDL/score label must retain a defined perspective when the leaf's
side to move differs from the root. Use chesso's own games/search, with tool
oracles for diagnostics, not another engine's training labels.

Split by game before sampling and prevent identical positions from leaking
across splits. P03's fixed set chooses recipes and later fits, so it is a
validation set, not an untouched final test. If final offline performance is
claimed, reserve a separate test set or report selection exposure. SPRT remains
the strength gate. Read coverage per feature and phase; row count alone cannot
establish support for king buckets or attack terms.

**Tuner.** Exact reconstruction reproduces the engine's separate integer
tapers and clamp at the weights under test. Test analytic gradients against
the smooth optimisation objective, with truncation/clamp treatment stated;
derivatives of the integer evaluator are not the existing smooth gradient
contract. A nonlinear per-side transform needs both sides' coefficients, not
only their difference. P17's “every weight from zero” can leave `max(0,x)^2`
with zero gradient at `x=0`; derive a nondegenerate own-data seed and prove
recovery before a long fit. Trace storage/extraction are timed and budgeted.
Tooling identity owes full INV-6, not only a bench total.

**Tables and caches.** Clusters of 32/64 bytes alone fill only suitable
power-of-two allocations if indexing still masks a rounded-down count. Carry
P08's arbitrary-bucket indexing and test non-power-of-two Hash requests and
allocation fallback. A zero-move store can preserve an older move only for
the matched position. Partial-key legality checks protect move consumers;
they do not authenticate a stored score, evaluation, depth or mate certificate.
State and measure that collision trade, including every PV/mate reader.

For S118, pawn-only entries may cache pawn-only features. King shelter/storm
must also validate king squares, as S118's body already says; king-distance
passer bonuses, attack-dependent paths and rook-dependent terms cannot be
cached solely by pawn key. Share reusable sets while computing final scores
from their actual inputs.

**Scope.** The complexity candidate has an inconsistent test: a sign-preserving
adjustment may reduce a nonzero score to zero, contrary to P25's “zero only
where the unadjusted one is.” Reconcile formula, purpose and guard before
reopening it. No optional term acquires approval from being folded into an
already approved S file.

## 5. Proposed order and dependencies

This is an adoption recommendation, not the operative Open list. Keep S ids;
allocate new ids only for accepted additions. The coordinator owns runs/shared
documents. Under the current reading rule, `src/` work waits for the machine;
P01/P02/P15 cannot start during a match merely because their lane says “A.”
Isolated-worktree implementation during runs requires a rule amendment and
still cannot contend for cores through builds or timings.

| order | work | prerequisites and measurement |
|---|---|---|
| 1 | S119, then S259 at that block's boundary | Table SPRT; neutral subchanges timed separately. S259 may run in the permitted tests lane, with heavy mutant builds between timed runs. |
| 2 | Search-block SPSA lane, independent verification, drift/long-control boundary readings | DEC-222's overdue cadence, reachable added axes, mate guards and DEC-202. Complete before regenerating fit data. |
| 3 | S032 and S030, bounded neutral trials | Native-target timings on the idle machine; fallback perft and INV-6. Stop a candidate at its predeclared implementation/timing budget if it does not pay. Owner may move this slot later, but not on the erroneous 1 % claim. |
| 4 | S134, trace tuner, shared attack infrastructure as needed | Bit-exact fold before fits; tooling reconstruction/gradient/recovery tests; INV-6/timings for engine changes. |
| 5 | S082, then S083 | Resolved-leaf recipe first, scale/effort second; held-out selection under a stated datagen budget. Retain separate candidates/verdicts. |
| 6 | Expanded S039, separate shortcut/clamp legs | Adoption of the scope/order change; settle the scoring contract before mobility and king-safety fits. |
| 7 | S121, S123, S125 | Curve/exclusions, three passer groups, then reconciled pawn groups; each verdict against the preceding kept tree. |
| 8 | S118, S101, S122 | Cache after expensive pawn work; threats and cache both before king safety. Attack reuse/nonlinear tuner ready. |
| 9 | S135 and S136 | S134/corpus ready; placement bisection on H0; tempo has its own WDL-heavy fit and freshly derived truncation guard. Moving these from their original position is an order amendment to test larger documented families first. |
| 10 | S124, S102, S133, S126 | Separate scaling cases, outposts/space, small king-relative tables, then full refit. Preserve S133 before S126. |
| 11 | S127 and block-boundary readings | Final tuning against completed evaluation, independent SPRT and DEC-202 where applicable. |
| 12 | S129 if its permitted route is feasible; S152 | Tablebase-specific measurement regime, then **both** rating controls on prepared stronger anchors. |

Prepare the anchor manifest in an allowed tool/document slot before S152;
smoke matches hold the machine. The optional S099 probe is outside this main
sequence unless the owner reserves a window. On H1 allocate gated follow-ups
and refit/tune changed search inputs before later corpus generation. Do not
place all follow-ups ahead of the evaluation block automatically.

## 6. Verdict and time accounting

Both plans' old totals are unsuitable for scheduling. The candidate bundles
multiple evaluation changes; the original comparison then undercounts the
S steps it preserves. Use this inventory for the recommended sequence,
assuming two retained clamp legs and neutral infrastructure:

| work | SPRTs owed |
|---|---:|
| S119; early search-block SPSA verification | 1 + 1 |
| S082 recipe; S083 one changed scale/effort axis | 1 + 1 |
| S039 shortcut then clamp | 2 |
| S121 curve plus three separately required exclusions | 4 |
| S123 rank/advance, king distances, candidates | 3 |
| S125 pawn groups/indexing | **V125**, to reconcile; its text describes four additions and indexing, while backward pawns already exist |
| S101; S122 | 1 + 1 |
| S135 placement bundle; S136 tempo | 1 + 1, plus placement bisections if needed |
| S124 two scaling cases; S102 outposts and space | 2 + 2 |
| S133; S126; S127 verification | 1 + 1 + 1 |
| **main path** | **24 + V125**, before conditional legs |

Additional verdicts: a second changed corpus-budget axis, imported mobility
pin handling, wider passer/threat terms, placement bisections, an S099 probe
and its H1-gated consumers, and S129. A changed cache score also owes an SPRT;
an exact cache, PEXT, move encoding, keys and attack sharing owe identity and
timing. An approved bundle states its scientific question and reason; a
cheaper total is not achieved by dropping acceptance clauses.

At the historical workstation rate of 2110 games/h, DEC-143 gives:

| nElo pair, alpha = beta = 0.05 | midpoint expected games / hours | on-bound expected games / hours |
|---|---|---|
| `{0, 5}` or `{-5, 0}` | 41861 / 19.84 h | 25591 / 12.13 h |
| `{-5, 5}` | 10465 / 4.96 h | 6398 / 3.03 h |

These are expected lengths, not hard ceilings. A 40000-game cap is about
18.96 hours here and can produce no verdict. The generated ledger's 6 h 19 m
overall and 8 h 52 m slow-class means describe past runs; neither predicts the
class of a new candidate. Bound choice and abort/no-verdict actions are fixed
before a run, not chosen from a reach census or hoped-for published gain.

For illustration only, if V125 is resolved as five verdicts, the base is 29:
**183.61 hours** at 43 ledger runs' exact mean (`272.25/43`), or **257.13 hours**
if each took the rounded 8 h 52 m slow-class mean. Neither includes generation,
fits, SPSA lanes, builds, timings, bisections, optional features or ratings.
Most near-zero five-point tests can approach the 19-hour cap individually.
The original 185–325-hour all-in estimate and four-to-six-week schedule are
withdrawn pending those budgets and the resolved experiment inventory.

Budget both S152 gauntlets explicitly. `rating.sh` defaults to 334 paired
rounds per opponent, or 668 games per anchor, so adding anchors increases cost.
The second control should address CCRL's machine-adjusted 2+1 regime; simply
setting `TC=2+1` on a different CPU does not establish equivalence.
[CCRL's conditions](https://computerchess.org.uk/404/about.html) also specify
128/256 MB hash, generic openings, disabled learning and permitted tablebases.
The DEC-202 32+0.32/Hash64 reading is a search-transfer estimate and does not
replace S152's rating-control comparison.

## 7. Milestones and the 3000 claim

Record every local SPRT, including H0/no verdict, with DEC-220's result block.
H1 on a non-regression pair is not automatically a demonstrated gain. Avoid
adding only positive stopping estimates and treating the sum as a rating;
selection and early stopping both affect it. The +207/+250 ratio is one
historical reading, useful for context only. No defensible additive gain band
in either plan establishes a final 2925–3065 or 2940–3080 rating forecast.

| checkpoint | evidence to read | action |
|---|---|---|
| foundations complete | Tuner reconstruction/recovery, split integrity, resolved leaves, coverage, table/cache identity | Resolve failed prerequisites before fitting new terms. |
| first evaluation families complete | SPRT outcomes, own cost, held-out loss and coverage for mobility/passers/pawns | If gains repeatedly fail or cost rises without measured benefit, diagnose model/data before expanding. A +25 sum is not a calibrated stop threshold. |
| S126 full refit | Kept verdicts, zeroed/unsupported columns, own engine versus prepared anchors only if the owner authorises a rating checkpoint | Re-prioritise supported hand-crafted candidates if needed; keep the no-network goal. Existing drift readings can inform this without another absolute rating run. |
| S152 near the mark | Two controls, exact version/settings, anchors bracketing the region, per-anchor estimates, forfeits and full uncertainty | Report an approximate CCRL-scale rating with its limitations. A point estimate over 3000 whose interval crosses 3000 does not demonstrate a lower bound of 3000. Pre-register the claim standard before reading the result. |

No automatic extra gauntlet is introduced: DEC-074/108 leaves its timing to
the owner, and DEC-234's S240 exception does not repeal that rule. Record the
phase transition when the agreed measurement shows the mark. If it falls
short, the next work remains hand-crafted under DEC-179.

## 8. Adoption decisions and repository constraints

The review and correction of this file are already authorised. The following
turn recommendations into a different operative plan; they are not approvals
requested to finish this review.

1. Adopt the scoped merge/order, early DEC-222 tuning lane, trace tooling and
   anchor preparation. Preserve omitted S items or record their retirement;
   reconcile V125 and conditional legs.
2. Amend S039's excludes/order for separately measured shortcut/clamp
   experiments, including S122's failure path. DEC-257 currently keeps the
   shortcut; no fixed-depth timing supersedes it.
3. Decide whether to reserve a night for S099 now. A dedicated window changes
   DEC-222's priority; the H1 gate and separate consumers remain.
4. Keep the lane rule or explicitly authorise isolated `src/` work during
   runs with contention excluded. A lane label is not authority.
5. Keep S129's independent-prober route subject to a documentation feasibility
   check. The proposal's absolute claim that no independent specification
   exists is not established by its research. The author's
   [tablebase README](https://raw.githubusercontent.com/syzygy1/tb/master/README.md)
   describes WDL/DTZ but does not alone establish enough compressed-format
   detail for this step. [Fathom is MIT](https://github.com/jdart1/Fathom/blob/master/LICENSE);
   integrating foreign probing code still requires an explicit COPYING/DEPS
   exception from the owner. Licence compatibility alone grants none.
6. Reopen DEC-138/192 terms individually with a new reason recorded:
   pawn-push/hanging threats, specialised mop-up, complexity, extra histories
   or extensions. There is no pre-3000 NNUE option in this recommendation.

On adoption the coordinator records the owner's rulings, allocates new S ids,
amends pending files, updates Open/status and specs where behaviour changes,
and commits a green plan change. Completed history stays intact. Each step
keeps the AGENTS delegation rule, both-build fast gate, required Debug,
sanitizer/perft/mutation checks, scripted goldens, own-data fits, one-change
measurement and seed provenance. MANUAL/DEV_MANUAL are checked; README is
owner-only. Dependencies require approval; agents commit and never push.
Timed runs use the selected machine's measured throughput, full cores, mains
power and detached-run/terminal-marker contract; no watcher is claimed
without the tool that actually keeps it alive.

## 9. Verification of this correction

The independent reviewer found no blocking issue in the corrected experiment
count, arithmetic, dependencies, scope or no-network rule. Path checks and
`git diff --check` pass. The repository's prose, citation, touches, parameter
and gate-command checks pass; checking this comparison's code citations also
passes. The citation scan notes S136's stale S055 assumptions as described
above. MANUAL/DEV_MANUAL were checked against the referenced paths and options;
this recommendation changes no shipped behaviour or interface.

Both required local builds are blocked by a pre-existing Apple Clang error:
`tests/test_search.cpp` declares an unused `ORDINARY_BETA` in the guard-fixture
namespace, and `-Werror` makes that a compilation failure. That source file is
unchanged from `c5fd1f2`. The full both-build fast gate cannot be claimed green
on this machine, so the correction is left uncommitted under COMMITS. This
test-build portability finding needs a scoped repair; it is not evidence of
an ordinary-play defect or a reason to change a strength verdict.
