id:         S083
goal:       a corpus past 50 M positions, and a measured answer on what nodes per move buys against volume
accepts:    a corpus of at least 50 M positions exists and its generation command, seed, node budget, wall time and filter counts are recorded in the step file, not in a log that is gitignored; the fit on it is compared to the fit on `selfplay_v2.tsv` on held-out error, and **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is; the node budget the corpus was generated at is stated as a decision with its reason, since it is the variable this step is trading
touches:    .tuning/, src/eval_tables.hpp, src/evaluation.cpp, adocs/plan_done/ on completion
excludes:   what is labelled, which is S082's and should be settled first; dedupe, which is S076's; the blend, which is S075's; any change to the tuner
decisions:  DEC-041, DEC-055
closes:
blocks:
paused_by:
done:

## Why this exists

`adocs/eval_tuning_strategy.md` section 2.6: "10M positions is a working minimum
for a few hundred parameters. 100M+ is where results stabilize. Serious HCE
tuners use 100M to 1B."

`selfplay_v2.tsv` is 11003693 positions for **827** fitted constants. That is the
working minimum, at four times the parameter count the sentence assumes.

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

## Cost, stated plainly

S065's run was 120000 games at 100000 nodes in about eight hours for 11.0 M rows.
Five times the rows at the same budget is five nights of generation. At 5000
nodes a night buys far more rows and the labels are what the doc calls noisier.
Either way this is the most expensive step in the pending order and it is placed
last for that reason, ahead only of the SPSA run it shares a constraint with.
Measurement capacity is the binding constraint on the whole plan.

## Cost

Several nights of generation, one fit, one SPRT.
