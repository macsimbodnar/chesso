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
it believes the move — and it believes it with material and a hand-written
piece-square table that has never been fitted to anything. So the evaluation
leads: **S028 fits the constants that already exist, S027 adds terms and fits
them the same way.** The search and ordering block follows, still worth doing and
now with a number on what it is worth. S029 was the network the tuned
hand-crafted evaluation would have floored and fed; it is parked, DEC-054, and
what it was going to answer is now the hand-crafted work's to answer. S030 to S032
are movegen work worth 1-3 % each — under 1 % for S031, its own file says — and
are last because that is what they are worth.

**S035 to S043 are the 2026-08-13 adversarial audit's nine findings, one step
each, and seven of them cut ahead of S033.** Not out of politeness to the
auditor: S035 was the SPRT harness, dead since `44877c4` with no verdict
obtainable below it, repaired and smoke-tested first; S036 was a search that
never returned on a 1 ms clock, corrupting the games beside it in a match; both
are done. S037 is the node count that `search_bench.py` reads, which is how
INV-6 is discharged — the last of the three instruments still pending. Those
three are the measuring instruments, and the rest of this plan is measured with
them. S043, S040, S041 and S038 follow because
they are cheap and each one removes a way for a later fit or a later
neutrality claim to be quietly wrong. S039 and S042 cost an SPRT each and buy
little, so they wait behind the search block rather than ahead of it.

**S044 to S052 are the 2026-08-13 plan_review audit's nine findings, one step
each, ahead of everything still pending: they are pure text, cost minutes
each, and they correct the documents the pending steps execute against.** They
correct the plan and its step files rather than the engine: S045, S046, S050
and S051 each remove a way for a later step to alter play unmeasured or to
record a verdict against the wrong thing; S044, S047 and S048 retire premises
that DEC-041 and the DEC-049 machine move made false; S049 and S052 re-point
one field each. All nine together cost less than one SPRT, and every step they
correct sits behind them in this order.

**S056 to S064 are the 2026-08-13 plan_review re-run's nine findings, one step
each, placed by what each corrects rather than as a block.** S044 to S052 went
ahead of everything because they corrected documents every later step executes
against; these correct one pending step each, so each sits immediately ahead of
the step it corrects and costs nothing until that step comes up: S060 before
S026, S063 before S024, S061 before S023, S059 before S025, S057 before S039,
S056 before S055, S058 before S030. S062 and S064 correct this file's own prose
and go at the front of the pending list, behind S033 only — S033 is next in this
order, no finding touches it, and displacing it would spend the queue on text.
All nine are text, cost minutes each, and none of them alters play.

**S054 is first of everything pending, and it is not an audit finding.** It was
found while completing S041: `clang-format.sh --check` selected tracked files
only, so it was blind to a file a step had just added and not yet staged — and
that check is one of the three commands in `.moltke.json`'s step-completion
gate. S041's own commit `96863ae` claimed the gate green over two files it had
added with three lines over the column limit. It cuts the queue under the house
rule that a found bug is fixed before anything else starts, because the defect
is in the gate every step below it reports through.

**S065 is at the front of the pending order and is not an audit finding.** It
regenerates the tuning corpus, which this machine did not have at all —
`2026-08-13_plan_review.2-F02` established that `.tuning/` is gitignored, that
the corpus did not survive the DEC-049 machine move, and that S027 and S028 make
it unregenerable identically — with the one datagen filter clause the engine's
own quiescence contradicts loosened behind a flag. DEC-055 is the owner's
decision to spend a night on it ahead of S033: it is tuning rather than a match,
it needs no engine code, and a night holds one SPRT or eight hours of
generation. It alters play and ends in an SPRT, because 827 constants move at
once.

**S066 came before it and is done.** The tuner shuffled rows and held out 10 %
of them while `datagen` writes about 92 rows per game consecutively, so 119360
of 119999 games — 99.47 % on the real corpus — had rows on both sides of the
split. S065's own gate is a held-out figure, so the splitter was fixed before
that figure is read. It changed a diagnostic and no game; DEC-056 records the
reconstruction it uses and why a game-id column was not the answer for a corpus
that already exists.

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
enforces the correspondence, adds an entry when a step is created, and prunes the
oldest completed entry — taking its testing.md rows with it — as newer
completions land. `plan_done/` and git history keep everything pruned.

<!-- 1. S001  short goal -->
37. S054  clang-format.sh --check sees untracked source files, so a step that adds a file cannot pass a vacuous format check
38. S038  the tuner-model guard states a tolerance the truncation arithmetic actually supports
39. S066  hold out whole games from the fit, not rows, so the validation error is not shared with training
40. S067  repair the test defects the 2026-08-14 test review found, before any further measurement is taken
41. S065  regenerate the tuning corpus from today's engine with the tactical-move filter loosened, and fit it
42. S033  prune a node whose static score is already far enough above beta
43. S062  plan.md's prose stops describing completed steps as pending instruments and next work
44. S064  plan.md and testing.md describe the checker's retention as the last five completed entries in list order
45. S021  start the root search in a narrow window around the previous score
46. S060  S026's accepts carries the mate-inside-the-pruned-depth clause its own body demands, per technique
47. S026  drop nodes near the horizon that cannot reach alpha
48. S063  S024, S030 and S039 cite the src/evaluation.cpp lines that exist at HEAD
49. S024  history indexed by the move played n plies ago and the current move
50. S061  S023's disjoint-bands criterion names a test that exercises the 100-point capture-to-killer clearance
51. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
52. S059  S025's gate books the outcome its own evidence predicts: a timing that is worse is the recorded verdict
53. S025  retry searching losing captures after the quiets, now that capture history exists
54. S022  skip a quiescence capture that cannot reach alpha even if it wins outright
55. S020  compute the in-check state once per node instead of once per call site
56. S057  S039's accepts names a corpus that exists in this tree, with its spread figures re-measured at HEAD
57. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
58. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
59. S056  S055's accepts re-targets the pinned thresholds to the post-merge bound instead of asking for a suite state the merge makes unreachable
60. S055  taper mobility and king safety through one division instead of two, tightening the model guard's bound to 2
61. S058  S030's neutrality remedy separates sites keyed on the move from sites keyed on prev_move, and names the write site
62. S030  move_t drops the moving piece and becomes 16 bits
63. S031  one unconditional xor for the side-to-move zobrist key instead of two
64. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
65. S029  **parked, DEC-054** — a perspective network evaluation trained on chesso's own self-play
66. S053  testing.md's header states the checker's retention: a pruned plan entry takes its ledger rows with it
