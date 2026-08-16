id:         S086
goal:       per-position outcome statistics steer the opening choice, behind a UCI option that ships off
accepts:    a persistent store keyed on the position's zobrist key holds win, loss and draw counts and an accumulated adjustment, written at game end and read at book-move selection only; a UCI option defaults **off** and the golden surface test and `MANUAL.md` carry it; with the option off the engine's play is unchanged -- identical node counts and identical best moves from `tools/search_bench.py` (INV-6) -- and no file is created or written; the learning is measured the only way it can be, a repeated match against the same opponent from the same book, and the verdict is recorded whatever it is; the store never influences an evaluation weight or a search parameter, and a test asserts that
touches:    src/openings.cpp, src/openings.hpp, src/chesso.cpp, tests/, MANUAL.md
excludes:   the evaluation weights and the search parameters, which this must not touch; opening book contents; any default that turns it on
decisions:
closes:
blocks:
paused_by:
done:

## Why this exists

`adocs/eval_tuning_strategy.md` section 6, and the owner's decision to build it
rather than reject it. Present in commercial engines from the 1990s -- Rebel,
Crafty's book learning and position learning. "Effect: in repeated matches
against the same opponent, worth +5 to +20 Elo. In one-off games, nothing."

## What is honest about the measurement

An SPRT against a reference build measures nothing here on the first pass,
because the store starts empty and the engine that has learned nothing plays
identically. The only measurement that means anything is the second and later
matches against the same opponent from the same book -- which is a different
experiment from every other verdict in this plan, and its number does not
transfer to a tournament against an unseen opponent.

Whoever executes this states the experiment before running it: how many matches,
against which opponent, from which book, and what the store held at the start of
each.

## The risks the document names, and they are real here

- **It is overfitting to the opponent pool.** A store trained against one
  reference build steers away from lines that build punished, not from lines that
  are bad.
- **Some tournaments prohibit persistent learning files.** Default off is not
  politeness, it is what keeps the engine admissible.
- **It must never touch the eval weights.** The whole gate discipline of this
  project rests on weights changing only through a fit and an SPRT. A learning
  file that adjusts an evaluation is a weight change with no verdict behind it,
  which is why the accepts asks for a test asserting the separation rather than a
  comment promising it.

## Why it is last in the order

It is orthogonal to everything else and it is the only item here whose gain is
conditional on playing the same opponent twice. Everything above it changes what
the engine is; this changes what it remembers.

## Cost

Low development cost. The measurement is several repeated matches, which is the
expensive part and is what places it last.
