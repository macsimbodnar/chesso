id:         S105
goal:       fastchess.sh plays the surveyed engines' testing regime -- 8+0.08, Hash 16, a UHO-class unbalanced book -- and a behaviour-neutral change is accepted on an interleaved timing instead of a match
accepts:    `fastchess.sh` runs `tc=8+0.08` and `option.Hash=16`, and reads a UHO-class unbalanced opening book, with the decisive-game rate at the new control measured and recorded against the 45-60 % draw band the book class is built for; a **time-forfeit check at the new control is run first and its count recorded** -- read from the PGN filtered to the run, because the fastchess log is WARN-only and comes back empty (the S089 lesson); the games-per-hour figure before and after is recorded so the throughput claim is a number and not a hope; the default SPRT bounds are stated in `fastchess.sh` and DEV_MANUAL.md -- gainers `elo0=0 elo1=5`, non-regressions `elo0=-5 elo1=0`, DEC-063's evidence cited; `rating.sh` staying at Hash 128+ is stated as deliberate with DEC-088 as the reason; DEV_MANUAL.md's "Play games" section states the new control, the new hash, the new book, and the rule that a behaviour-neutral change is not sent to a match; tests/test_fastchess_script.sh still green
touches:    fastchess.sh, books/, rating.sh, DEV_MANUAL.md, tests/test_fastchess_script.sh
excludes:   any change under src/; the rating gauntlet's own time control, which is S128
decisions:  DEC-083, DEC-088, DEC-048, DEC-050
closes:
blocks:
paused_by:
done:

## The three mismatches, measured

**Time control.** `fastchess.sh:25` is `tc="10+0.2"`. The engines this plan
reads figures from test at 8+0.08 and 10+0.1. At roughly 80 moves a game,
10+0.2 costs about 52 s a game against about 29 s -- **1.8 times the cost per
verdict**, spent for no extra resolution.

**Hash.** `fastchess.sh:126` is `option.Hash=16` and that number turns out to
be right for the SPRT -- for a reason DEC-083 got wrong and DEC-088 corrects.
What transfers across time controls is table **pressure**, not table size:
every OpenBench engine preset tests STC at 8 to 32 MB (Stash's preset is
exactly 8+0.08 with Hash=16), because at the list's 2'+1" a game writes on the
order of 660 M nodes against 5.6 to 11 M entries -- roughly 60 to 120
overwrites per entry -- and 16 MB at 8+0.08 reproduces that ratio where 128 MB
undershoots it about eightfold. So the SPRT keeps Hash=16, and `rating.sh`
keeps 128+ because the gauntlet's job is the list's absolute regime. The
16-vs-512 measurement DEC-083 records (36 % fewer nodes, 21 % lower nps at
`go movetime 2000`) stands as a fact about table size at 2 s a move; it was
the wrong invariant to match at 0.2 s a move.

**Book.** `books/8moves_v3.pgn` is balanced. Pohl's measurement on the UHO
book class: balanced openings ran ~91 % draws where the UHO bands run 40 to
57 %, and every surveyed engine's OpenBench preset defaults to a UHO book --
8moves_v3 is registered nowhere as a default. An unbalanced book raises the
decisive-game rate and shortens every run, which is measurement capacity and
not strength.

## What this buys

Roughly three times the verdicts per night, against a plan whose own budget
line says measurement capacity is the binding constraint. It costs the
comparability of verdict sizes across this commit, which was already lost at
the DEC-049 machine move and which per-change verdicts against a named commit
never had.
