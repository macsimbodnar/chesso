id:         S107
goal:       a quiet move that gives check becomes eligible for the killer, history and countermove tables it is excluded from today
accepts:    an SPRT verdict, recorded whatever it is; the `is_check_move` term is removed from the killer, history and countermove condition in `negamax`'s fail-high block, and the step states what `is_check_move` is still needed for after the change -- if the answer is "the late move reduction guard alone", say so, because that is what S020 then has to preserve; the fast suite green
touches:    src/search.cpp negamax
excludes:   computing the in-check state once per node, which is S020; any reduction change, which is S098
decisions:
closes:
blocks:
paused_by:
done:

## What is there, and why it looks wrong

`src/search.cpp`, the fail-high block:

    if (!is_capture && !is_check_move) {
      ... killer, history, countermove ...
    }

A quiet move that gives check is excluded from **all three** ordering tables.
Those are exactly the moves most likely to be the refutation at a sibling node,
and the engine refuses to remember any of them. No comment in the file argues
for the exclusion and no decision records it; it reads like an accident of the
condition being written once for two purposes.

Small, one line, and it alters play -- so it owes a verdict like anything else,
and a verdict of zero is recorded as zero. It goes early because it changes the
tables every later history step reads, and measuring those on a table with a
hole in it measures the hole.
