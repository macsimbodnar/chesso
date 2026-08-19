id:         S105
goal:       fastchess.sh plays in the rating list's regime -- 8+0.08, Hash 128, an unbalanced book -- and a behaviour-neutral change is accepted on an interleaved timing instead of a match
accepts:    `fastchess.sh` runs `tc=8+0.08` and `option.Hash=128`, and reads an unbalanced opening book; a **time-forfeit check at the new control is run first and its count recorded** -- read from the PGN filtered to the run, because the fastchess log is WARN-only and comes back empty (the S089 lesson); the games-per-hour figure before and after is recorded so the throughput claim is a number and not a hope; `rating.sh` and `fastchess.sh` no longer disagree about hash without a stated reason; DEV_MANUAL.md's "Play games" section states the new control, the new hash, the new book, and the rule that a behaviour-neutral change is not sent to a match; tests/test_fastchess_script.sh still green
touches:    fastchess.sh, books/, rating.sh, DEV_MANUAL.md, tests/test_fastchess_script.sh
excludes:   any change under src/; the rating gauntlet's own time control, which is S128
decisions:  DEC-083, DEC-048, DEC-050
closes:
blocks:
paused_by:
done:

## The three mismatches, measured

**Time control.** `fastchess.sh:25` is `tc="10+0.2"`. The engines this plan
reads figures from test at 8+0.08 and 10+0.1. At roughly 80 moves a game,
10+0.2 costs about 52 s a game against about 29 s -- **1.8 times the cost per
verdict**, spent for no extra resolution.

**Hash.** `fastchess.sh:126` is `option.Hash=16`; `rating.sh:28` is 64; CCRL
Blitz runs **128 to 256**. Three regimes, and the difference is not small.
Measured 2026-08-19, `go movetime 2000` from the start position on the
`-march=native` build:

| Hash | nodes | nps |
|---|---|---|
| 16 MB | 12673695 / 12731039 | 6.35 M |
| 64 MB | 11989705 / 12075721 | 6.02 M |
| 256 MB | 9176288 | 5.59 M |
| 512 MB | 8099708 | 4.99 M |

**36 % fewer nodes and 21 % lower nps** across that span. Every verdict on
record was taken in a table regime the rating list never runs, and the steps
that touch the table -- S119 above all -- would be measured in the wrong one.

**Book.** `books/8moves_v3.pgn` is balanced. An unbalanced book raises the
decisive-game rate and shortens every run, which is measurement capacity and
not strength.

## What this buys

Roughly three times the verdicts per night, against a plan whose own budget
line says measurement capacity is the binding constraint. It costs the
comparability of verdict sizes across this commit, which was already lost at
the DEC-049 machine move and which per-change verdicts against a named commit
never had.
