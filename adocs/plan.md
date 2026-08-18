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

**Phase one now has a number: at least 3000 on the CCRL Blitz scale, without a
network.** S087 measured chesso at **2570, 95 % +/-25, soft** -- the first
absolute figure this project has ever had, and the first statement of the
distance to the goal above. DEC-071 is the owner's decision that 3000 is the
next mark and that DEC-054 stands while it is pursued: no NNUE.

The literature says that is reachable with room to spare, and the check was run
against the same list chesso is rated on rather than from memory. Single-CPU
entries, read 2026-08-18: Stockfish 11 **3565**, Komodo 14.1 3482, Xiphos 0.6
3356, Ethereal 11.75 3346, rofChade 2.3 3322, Laser 1.7 3294, Defenchess 2.2
3281, Booot 6.3.1 3266, Texel 1.07 3130 -- every one a hand-crafted evaluation,
each one version below that engine's first network release. The nearest case is
on this machine and needed no web claim: the whole Leorik 2.x line carries zero
files matching `nnue|network|neural|\.nn` and CCRL rates it 2.0.2 = 2538 up to
**2.5 = 2917**, +379 Elo of hand-crafted progress by one author starting below
where chesso is now. Blunder reaches 2664 the same way. So the ceiling is not
the constraint; what the plan contains is.

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
sit near the end because that is what they are worth -- and DEC-071 moved time
management ahead of them, which had no step at all while they had three.

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

**S087 cuts ahead of S060 and is an instrument, not a strength change,
DEC-067.** Every number this plan has produced is a delta against an earlier
chesso: `fastchess.sh` plays the working tree against a `.ref-builds/` worktree
and answers "is B stronger than A". Nothing here answers "how strong", and
self-play cannot answer it at any game count. S087 plays a gauntlet against
engines the public lists rate and solves it with `ordo` into an absolute figure
on the CCRL Blitz scale, with an interval and a measured sensitivity to which
reference is anchored. It buys zero Elo, which is the same trade S035 and S037
made — and `plan.md` already records that the rest of this plan is measured with
them. The reference binaries are compiled by the owner and installed in
`/usr/games` beside `stockfish` and `fastchess`; nothing third-party enters this
tree, and what is tracked is a manifest naming the version each result was
played against. A short bracketing run at the repository's
own `10+0.2` comes first and costs about thirteen minutes; what the rated run's
time control is, and therefore whether it costs an hour or a night, is decided
after it with a score in hand.

**S088 to S102 are DEC-071, and they are what the 3000 target costs.** The gap
is 430 Elo and the plan did not contain it. `src/search.cpp` at HEAD has
alpha-beta, the table, quiescence, PVS, aspiration, null move, late move
reduction, reverse futility, staged generation, killers, history, countermoves
and `see_ge` in quiescence alone; missing entirely, and with **no step behind any
of it**, were late move pruning, extensions of any kind, SEE pruning in the main
search, internal iterative reduction, an `improving` flag, history malus and
ageing, a quiescence table probe, a static evaluation in the entry, correction
history and any refinement of the reduction. `specs.md` listed those as parked,
and parked means nothing can derive a step from them. Time management was not
even parked: `chesso.cpp:405-410` divides the clock by a fixed movestogo and
adds half the increment, and nothing looks at whether the best move has been
stable or the score has fallen.

They are placed in four runs rather than as a block:

**S088 goes first, and it is the instrument again.** 2570 is soft by 83.1 Elo of
anchor spread against the 30 the procedure allows, isolated to Leorik 2.1. A
finish line cannot be read off an instrument that wide, and changing the
reference set during the climb would make every reading across it incomparable.
Two families cannot arbitrate a disagreement between two families; a third can.

**Then the search, S089 to S099, one technique each and ordered by what feeds
what.** S089 time management first because it prunes nothing and can hide no
mate. S094 next, because the static evaluation in a table entry is what S092's
flag compares and what S099 learns a correction from, and recomputing it puts
back part of the 25 % of nodes per second S014 removed. S092, S090, S091 and
S095 follow; S096 and S097 are the extensions, and they sit after S026 because
they make the tree larger and the pruning has to be right first. S093, S098 and
S099 come after S024 and S023, because the reduction's largest input is a
history table those two steps are still changing.

**Then the corpus and the evaluation, S082 and S083 before S100 to S102.**
Fitting a new term on a corpus that is about to be replaced is exactly the trap
`src/evaluation.cpp:194-198` records against S027 -- the fit that put five
standard terms at zero was run on self-play by an engine predating all of S027.
S100 re-asks those five against the current fit and the current search, and
splits the bishop pair from the three rook features, which S027's own comment
argues for and does not do.

**The movegen micro-steps and SPSA stay at the end.** S084 and S085 tune the
constants every step above them adds, so they cannot precede them.

**The cost is the whole measurement budget for months.** Fourteen of the fifteen
alter play and owe an SPRT each; with failures and retunes that is 40 to 60
verdicts, three to four and a half hours apiece, 150 to 250 machine hours. And
one rule that is not a step: **`./rating.sh` is re-run when substantial work has
been done to the engine** -- the owner's judgement, not a threshold an agent
derives, DEC-074. Every Elo figure this project holds is self-play against an
earlier chesso while 2570 is external and nothing here yet knows the factor
between them, so the absolute figure has to be re-read sometimes; but the
per-change decisions are the SPRT's, and at DEC-073's concurrency 6 a rated run
costs about 5 hours out of the same budget. DEC-071's "any landed step an SPRT
credits with 20 Elo or more" is superseded.

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
57. S087  an absolute rating for chesso on the CCRL Blitz scale, from a gauntlet against engines the public lists rate
58. S088  a fourth engine family in the reference set and the rating re-solved, so the anchor spread is inside the 30 Elo the procedure allows
59. S089  a time budget that scales with best-move stability and with a falling score, instead of remaining over a fixed movestogo plus half the increment
60. S094  quiescence probes and stores the transposition table, and an entry carries the static evaluation it was scored with
61. S103  reverse futility reads the static evaluation already in the table entry instead of recomputing it
62. S092  pruning and reduction margins know whether the static score is rising over the ply stack
63. S090  skip late quiet moves near the horizon by move count, once the ordering has been given its chance
64. S091  skip captures and quiets the exchange evaluation says lose material, in the main search rather than in quiescence alone
65. S095  reduce a node whose table entry carries no move instead of searching it at full depth
66. S060  S026's accepts carries the mate-inside-the-pruned-depth clause its own body demands, per technique
67. S026  drop nodes near the horizon that cannot reach alpha
68. S096  extend a node that gives check, so a forcing line is not cut at the horizon
69. S097  extend the one move a verification search says is singular, and take the multicut the same search offers
70. S079  S063 covers every pending step file and cites symbols, and S058 stops requiring a literal line number
71. S063  S024, S030 and S039 cite the src/evaluation.cpp lines that exist at HEAD
72. S024  history indexed by the move played n plies ago and the current move
73. S061  S023's disjoint-bands criterion names a test that exercises the 100-point capture-to-killer clearance
74. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
75. S093  history gets a malus for the moves that were tried and failed, ageing, and survives across go within one game
76. S098  the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
77. S099  a static evaluation correction learned from the difference between the static score and what the search returned
78. S059  S025's gate books the outcome its own evidence predicts: a timing that is worse is the recorded verdict
79. S025  retry searching losing captures after the quiets, now that capture history exists
80. S022  skip a quiescence capture that cannot reach alpha even if it wins outright
81. S020  compute the in-check state once per node instead of once per call site
82. S082  the corpus labels the quiescence leaf rather than the root, which is the position evaluate() is asked about
83. S083  a corpus past 50 M positions, and a measured answer on what nodes per move buys against volume
84. S100  the five evaluation terms shipped at zero weight are re-examined against the current fit and the current search
85. S101  evaluation terms for a piece attacked by a lesser piece, fitted like every other constant
86. S102  outpost and space terms in the evaluation, fitted like every other constant
87. S080  S057 argues from the corpus on disk and the spread measured at HEAD, and S039's stale premise is named in full
88. S057  S039's accepts names a corpus that exists in this tree, with its spread figures re-measured at HEAD
89. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
90. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
91. S081  S056's four merged disagreements are measured on the pinned positions the test loads today
92. S056  S055's accepts re-targets the pinned thresholds to the post-merge bound instead of asking for a suite state the merge makes unreachable
93. S055  taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
94. S058  S030's neutrality remedy separates sites keyed on the move from sites keyed on prev_move, and names the write site
95. S030  move_t drops the moving piece and becomes 16 bits
96. S031  one unconditional xor for the side-to-move zobrist key instead of two
97. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
98. S084  an SPSA driver over the exposed search parameters, verified against an objective whose optimum is known
99. S085  the first SPSA run on the search parameters, and an independent SPRT of what it returns
100. S086  per-position outcome statistics steer the opening choice, behind a UCI option that ships off
101. S029  **parked, DEC-054** — a perspective network evaluation trained on chesso's own self-play
102. S053  testing.md's header states the checker's retention: a pruned plan entry takes its ledger rows with it
