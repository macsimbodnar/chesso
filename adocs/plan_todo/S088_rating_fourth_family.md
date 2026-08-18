id:         S088
goal:       a fourth engine family in the reference set and the rating re-solved, so the anchor spread is inside the 30 Elo the procedure allows
accepts:    the reference set carries at least three engine families and at least five rungs, with every anchor rating read from the CCRL list at run time and never hardcoded; the added engine is built or downloaded by the agent under /home/max/ws/engines and recorded in references.tsv with its tag, source and md5, per DEC-069; a bracketing check shows the added engine inside the 10 % to 90 % band; the rated run returns a 95 % interval of +/- 30 Elo or tighter on chesso's solved rating; zero time forfeits, one invalidating the run; the anchor sweep is re-solved over every reference and the full spread is reported, and if it is still above 30 Elo the dissenting engine is named, the spread with that engine dropped is reported beside the spread with it kept, and which of the two is quoted is a recorded decision rather than the smaller number
touches:    references.tsv, rating.sh only if the set size is wired into it, adocs/data/ for the PGN and the results file, DEV_MANUAL.md, adocs/specs.md for the measured figure
excludes:   any edit under src/ or tests/; changing the time control away from 10+0.2, which would make the figure incomparable with S087's; assessing any position, move or game from the resulting PGN
decisions:  DEC-071, DEC-067, DEC-068, DEC-069
closes:
blocks:
paused_by:
done:

## Why this is first

S087's figure is soft and the reason is not game count. Blunder 7.1.0, Blunder
8.5.5 and Leorik 2.4 reproduce each other's CCRL ratings to within 16 Elo across
a 440-point span; **Leorik 2.1 comes out about 82 Elo above its listed rating**
whichever of the others is anchored, six standard errors at 668 games per
pairing. Two families cannot arbitrate that. A third can: if 2.1 stays the sole
dissenter against an unrelated engine, the anchor is wrong for that binary
rather than chesso's rating being uncertain by 83 Elo.

The target is 3000 (DEC-071). A finish line cannot be read from an instrument
83 Elo wide, and changing the reference set *during* the climb would make the
readings across it incomparable. So the set is fixed now and not later.

## Candidates

Anything with a CCRL Blitz entry, a binary the agent can produce without adding
a dependency, and a rating in or near the 2389 to 2829 the set already spans.
From the list read 2026-08-18: Stash 21.0 at 2713 or Stash 25.0 at 2932 (C,
make), Weiss 0.10 at 2847 or Weiss 1.0 at 2896 (C, make), Zurichess Neuchatel at
2920 (Go), Monolith 2 at 3011 (C++). The choice is recorded in the results file
with the reason.

## Cost

One bracketing run of about 15 minutes, then a rated run of about an hour at
10+0.2 -- longer than S087's hour because a fifth rung adds pairings. The
`ordo` sweep is seconds.
