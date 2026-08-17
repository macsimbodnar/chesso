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
and evaluation tuning are the agent's to run, and S028's fit was run that way).
S029 is parked at DEC-054, so nothing now in the pending order stops short of
its own run; the boundary stands for whenever it resumes.

The order below is not the order of expected Elo, and that is deliberate. S001
to S018 are already done and are here as the record of what each change cost and
bought. S017 came first because the workflow asserts a surface check that nothing
performed. S018 came second because this engine has now taken three published Elo
figures at face value and measured 0, 0 and *slower* (DEC-019), so what is worked
on next is chosen from chesso's own error distribution rather than from what
other engines report.

**The rest of the order is what S018 and DEC-033 measured, and it is not what it
was.** The plan used to run the search and ordering features first, on the
argument that a better score at the leaves is worth less when the tree above them
is the wrong shape. That argument was tested: 160 expensive moves re-asked at 16
times the search removed 24.1 % of the error, 10.3 cp per doubling, and 95 of the
160 moves did not change at all. The engine mostly is not missing the refutation,
it believes the move — and it believed it with material and a hand-written
piece-square table that had never been fitted to anything. So the evaluation
led: **S028 fitted the constants that already existed, S027 added terms and
fitted them the same way, and S065 refitted the lot on a regenerated corpus.**
All three are done. The search and ordering block follows, still worth doing and
now with a number on what it is worth. S029 was the network the tuned
hand-crafted evaluation would have floored and fed; it is parked, DEC-054, and
what it was going to answer is now the hand-crafted work's to answer. S030 to S032
are movegen work worth 1-3 % each — under 1 % for S031, its own file says — and
are last because that is what they are worth.

**S035 to S043 were the 2026-08-13 adversarial audit's nine findings, one step
each, and seven of them cut ahead of S033.** Not out of politeness to the
auditor: S035 was the SPRT harness, dead since `44877c4` with no verdict
obtainable below it, repaired and smoke-tested first; S036 was a search that
never returned on a 1 ms clock, corrupting the games beside it in a match. S037
made the node count `search_bench.py` reads cumulative over the whole search,
which is how INV-6 is discharged. All three are done, they are the measuring
instruments, and the rest of this plan is measured with them. S043, S040, S041
and S038 followed because
they are cheap and each one removes a way for a later fit or a later
neutrality claim to be quietly wrong. S039 and S042 cost an SPRT each and buy
little, so they wait behind the search block rather than ahead of it.

**S044 to S052 were the 2026-08-13 plan_review audit's nine findings, one step
each, and all nine went ahead of everything then pending: they were pure text,
cost minutes each, and they corrected the documents the pending steps execute
against.** They corrected the plan and its step files rather than the engine:
S045, S046, S050 and S051 each removed a way for a later step to alter play
unmeasured or to record a verdict against the wrong thing; S044, S047 and S048
retired premises that DEC-041 and the DEC-049 machine move made false; S049 and
S052 re-pointed one field each. All nine together cost less than one SPRT, and
every step they corrected still sits behind them in this order.

**S056 to S064 are the 2026-08-13 plan_review re-run's nine findings, one step
each, placed by what each corrects rather than as a block.** S044 to S052 went
ahead of everything because they corrected documents every later step executes
against; these correct one pending step each, so each sits immediately ahead of
the step it corrects and costs nothing until that step comes up: S060 before
S026, S063 before S024, S061 before S023, S059 before S025, S057 before S039,
S056 before S055, S058 before S030. S062 and S064 correct this file's own prose
and were queued behind S033 alone, because no finding touched S033 and
displacing it would have spent the queue on text. All nine are text, cost
minutes each, and none of them alters play.

**S054 went first of everything then pending, and it was not an audit finding.**
It was found while completing S041: `clang-format.sh --check` selected tracked
files only, so it was blind to a file a step had just added and not yet staged —
and that check is one of the three commands in `.moltke.json`'s step-completion
gate. S041's own commit `96863ae` claimed the gate green over two files it had
added with three lines over the column limit. It cut the queue under the house
rule that a found bug is fixed before anything else starts, because the defect
was in the gate every step below it reports through.

**S065 cut ahead of S033 and was not an audit finding.** It regenerated the
tuning corpus, which this machine did not have at all —
`2026-08-13_plan_review.2-F02` established that `.tuning/` is gitignored, that
the corpus did not survive the DEC-049 machine move, and that S027 and S028 make
it unregenerable identically — with the one datagen filter clause the engine's
own quiescence contradicts loosened behind a flag. DEC-055 was the owner's
decision to spend a night on it ahead of S033: it is tuning rather than a match,
it needs no engine code, and a night holds one SPRT or eight hours of
generation. It altered play and ended in an SPRT, because 827 constants moved at
once: **+21.10 +/- 10.47 Elo, H1 accepted**.

**S066 came before it and is done.** The tuner shuffled rows and held out 10 %
of them while `datagen` writes about 92 rows per game consecutively, so 119360
of 119999 games — 99.47 % on the real corpus — had rows on both sides of the
split. S065's own gate is a held-out figure, so the splitter was fixed before
that figure is read. It changed a diagnostic and no game; DEC-056 records the
reconstruction it uses and why a game-id column was not the answer for a corpus
that already exists.

**S069 to S072, S074, S078 to S081 are the 2026-08-16 plan_review audit's ten
findings, nine steps and one decision, placed by what each corrects.** Four sit
at the head of the order, because they are what a session reads before it acts:
S069 corrects `status.md`, which still prices a verdict at an hour on three Apple
cores and blames a macOS daemon on a Linux machine, against a `specs.md` that was
rewritten for DEC-049 and wins on precedence; S070 replaces an illegal FEN still
live in `tests/test_engine.cpp` after S067 fixed the other two sites, in the
suite every step completion reports through; S071 makes this file's citation of
the workflow checker followable, since `bin/moltke.py` is not in this repository;
and S072 got S068's evidence out of a dead session's scratchpad, which is where
the next step's entire argument had been living, into `adocs/data/`. The other
four sit immediately
ahead of the step each corrects: S078 before S060, S079 before S063, S080 before
S057, S081 before S056. The tenth finding was the untracked strategy document and
is answered by DEC-062 rather than by a step.

**F01 was the high one and it is S080.** S057 exists to hand S039 corrected
evidence and every load-bearing claim in it is false: it says the tuning corpus
does not exist, and `.tuning/selfplay_v2.tsv` is on disk with 11003693 rows; its
"re-measured at HEAD" figures predate S065's refit of 827 constants; and the real
spread is four times what both it and S039 record — 1.558 % of positions past 150
against 0.364 %, worst 489 cp against 279. So `evaluate()` discards up to 339 cp
where the step says 129. S039 picks a margin from that distribution, so it was
picking from the wrong one.

**S073, S075 to S077 and S082 to S086 come from `adocs/eval_tuning_strategy.md`,
adopted at DEC-062.** Most of what that document describes is already here — the
Texel fit, Adam, the fitted K, the held-out split by game, king safety linear in
its weights, and a pentanomial gate, since `fastchess.sh` runs `model=normalized`
and every verdict on record carries an nElo figure. What is missing is placed in
three bands. **Instrumentation first:** S073 makes the search constants one
addressable set, settable in a tune build and untouched in the shipping one,
because S068 and S039 each currently cost a source edit and a rebuild per point
measured — and the sweep method S068 argues from no longer compiles at all.
**Then the cheap fits,** each a fit and at most one verdict: S075 blends the
search score with the game result, the corpus column that has been written and
never read; S076 deduplicates by zobrist key; S077 stamps the engine commit and
the corpus hash onto what the tuner emits. **Then, last, the expensive band:**
S082 moves the label from the root to the quiescence leaf, which is the
document's own critical design decision and needs a regenerated corpus; S083
takes the corpus past 50 M positions; S084 and S085 are the SPSA driver and its
first run; S086 is book learning behind a UCI option that ships off. NNUE is not
reopened — DEC-054 stands, and a literature summary does not reverse an owner's
decision.

S019 is retired. It was written from one game, DEC-032 showed the endgame is the
cheapest phase per move, and DEC-033 showed endgame errors are the least
depth-fixable of all. Its content belonged to S027 from the start. The id is not
reused.

**S029 is parked, which is not retired.** NNUE is deferred by the owner's
decision at DEC-054: strength comes from search and from the hand-crafted
evaluation instead. Where S019 lost its file and its entry, S029 keeps both —
`plan_todo/S029_nnue.md` with its architecture, accumulator plan and data
pipeline intact, and a list entry moved to the end of the pending order and
marked parked, so nothing derives it as the next step and nothing has to be
rewritten if it resumes. The id is not reused in either case. Resuming it is a
decision, not a drift.

Order lives here and nowhere else. Step detail lives in the step files under
`plan_todo/`, `plan_current/`, and `plan_done/`. Ids are allocated in creation
order and never renumbered, so reordering is a one-line edit to this list.

Order is read from the list entries below, the lines starting `1.`, `-`, or `*`.
An id named in a sentence anywhere else in this file is prose: it does not change
the order, and it is not checked. Every pending step file must appear as a list
entry and every list entry must name an existing step file; the workflow checker
enforces the correspondence and adds an entry when a step is created.

**It also prunes completed entries, and it keeps the last five in *list order*,
not the five most recently completed.** The checker is not in this tree. It is
the moltke plugin, read here at **version 0.11.0**, where `prune_plan()`
collects the completed entries by position in the file and drops all but the
final `PLAN_DONE_KEPT` of them, `PLAN_DONE_KEPT = 5`. Follow it by those two
symbols and not by a line number: they sat at `bin/moltke.py:1698-1700` and
`:1681` in 0.11.0 and they move under any other version. The version is
load-bearing rather than decoration — 0.1.0 is installed on this machine too and
has neither symbol, so nothing in this paragraph is true of it. A step whose
entry sits low in the list therefore outlives completions that came after it.
S053 completed at 18:42 on 2026-08-13 and is still listed; S037 at 19:13 and S043 at 19:29 the same evening
are gone, and so is S066 from the day after. S053 survives because its entry is
the last line of the list, appended at creation, not because it is recent.
Retention is a window over positions and says nothing about when anything
finished.

A pruned entry takes ledger rows with it, and the rule there is narrower than it
looks: a `testing.md` row leaves only when **every** `S<nnn>` it names was
dropped in that same pass. Naming an invariant or a decision does not protect a
row — the checker matches step ids and nothing else — and 17 rows naming an
`INV-n` or a `DEC-n` have been pruned, `| S037 | the reporting change leaves the
tree identical (INV-6) |` among them. A row naming no step id at all is never
pruned, which is why the `INV-1` to `INV-6` rows at the head of the ledger stay.
`plan_done/` and git history keep everything pruned.

<!-- 1. S001  short goal -->
53. S075  the tuner fits a blend of the search score and the game result, the corpus column that is written and never read
54. S076  the fit runs on a corpus deduplicated by zobrist key, so a repeated position stops carrying repeated weight
55. S077  an emitted table names the engine commit and the corpus hash it was fitted from
56. S078  S060 points at the mate-inside-the-pruned-depth test S033 added, and S061's citation names the case it means
57. S060  S026's accepts carries the mate-inside-the-pruned-depth clause its own body demands, per technique
58. S026  drop nodes near the horizon that cannot reach alpha
59. S079  S063 covers every pending step file and cites symbols, and S058 stops requiring a literal line number
60. S063  S024, S030 and S039 cite the src/evaluation.cpp lines that exist at HEAD
61. S024  history indexed by the move played n plies ago and the current move
62. S061  S023's disjoint-bands criterion names a test that exercises the 100-point capture-to-killer clearance
63. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
64. S059  S025's gate books the outcome its own evidence predicts: a timing that is worse is the recorded verdict
65. S025  retry searching losing captures after the quiets, now that capture history exists
66. S022  skip a quiescence capture that cannot reach alpha even if it wins outright
67. S020  compute the in-check state once per node instead of once per call site
68. S080  S057 argues from the corpus on disk and the spread measured at HEAD, and S039's stale premise is named in full
69. S057  S039's accepts names a corpus that exists in this tree, with its spread figures re-measured at HEAD
70. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
71. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
72. S081  S056's four merged disagreements are measured on the pinned positions the test loads today
73. S056  S055's accepts re-targets the pinned thresholds to the post-merge bound instead of asking for a suite state the merge makes unreachable
74. S055  taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
75. S058  S030's neutrality remedy separates sites keyed on the move from sites keyed on prev_move, and names the write site
76. S030  move_t drops the moving piece and becomes 16 bits
77. S031  one unconditional xor for the side-to-move zobrist key instead of two
78. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
79. S082  the corpus labels the quiescence leaf rather than the root, which is the position evaluate() is asked about
80. S083  a corpus past 50 M positions, and a measured answer on what nodes per move buys against volume
81. S084  an SPSA driver over the exposed search parameters, verified against an objective whose optimum is known
82. S085  the first SPSA run on the search parameters, and an independent SPRT of what it returns
83. S086  per-position outcome statistics steer the opening choice, behind a UCI option that ships off
84. S029  **parked, DEC-054** — a perspective network evaluation trained on chesso's own self-play
85. S053  testing.md's header states the checker's retention: a pruned plan entry takes its ledger rows with it
