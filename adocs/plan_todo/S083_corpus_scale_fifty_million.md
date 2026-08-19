id:         S083
goal:       the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, not by a volume target
accepts:    the datagen budget in nights is stated **before** generation starts; the fit is compared on held-out error across at least two corpus sizes at the S082 sampling density, so "more rows still helps" is a measured claim at this parameter count and not a quoted one; the generation command, seed, node budget, wall time and filter counts are recorded in the step file, not in a log that is gitignored; **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is; the node budget the corpus was generated at is stated as a decision with its reason, since it is the variable this step is trading
touches:    .tuning/, src/eval_tables.hpp, src/evaluation.cpp, adocs/plan_done/ on completion
excludes:   what is labelled, which is S082's and should be settled first; dedupe, which is S076's; the blend, which is S075's; any change to the tuner
decisions:  DEC-041, DEC-055, DEC-087
closes:
blocks:
paused_by:
done:

## Re-scoped 2026-08-19 by the second review, DEC-087

This step said "a corpus past 50 M positions". The 50 M floor is retired for
arithmetic and for evidence. Arithmetic: at S082's two-to-four rows a game,
50 M rows is 12 to 25 M games of datagen -- weeks of nights on the S065 rate
of 120 k games in eight hours, on the machine whose time is the plan's binding
constraint. Evidence: the band's engines fitted their hand-crafted
evaluations on **4.5 to 10 M resolved positions** -- Stash's v32 retune used
4.5 M self-play positions and its author reports the evaluation "doesn't
overfit for datasets > 500k", and Ethereal's published dumps are ~10 M rows a
generation -- against 827 shipped constants here. `adocs/eval_tuning_strategy.md`
section 2.6 says "100M+ is where results stabilize"; that sentence describes
NNUE-scale parameter counts, and this step exists to answer it with a held-out
curve at this engine's own parameter count rather than adopt either number.

## The trade this step is actually making

The same document suggests generating "at fixed low nodes (e.g. 5000
nodes/move)". chesso generated at **100000** -- twenty times that -- and the
default in `tools/datagen` is 5000, so the S065 run overrode it deliberately.

At fixed compute those two settings are the same night spent differently: a
larger corpus of noisier labels, or a smaller one of better labels. Nobody here
has measured which side of that trade is better, and the published advice and
this project's own practice disagree by a factor of twenty. So the node budget is
a recorded decision in this step, with its reason, rather than a number carried
over.

**One change at a time still applies.** If both the size and the node budget
move, the verdict says nothing about either. The honest form is: hold the node
budget where S065 had it and scale the games, or hold the games and drop the node
budget, and if both are wanted that is two corpora and two verdicts.

## Cost

One to three nights of generation inside the stated budget, held-out fits in
minutes each, one SPRT.
