id:         S166
goal:       the documented stockfish invocation cannot silently answer without searching: TOOLCHAIN.md gains a chess-oracle section naming the quit-aborts-the-search trap, and CLAUDE.md's tool table points at it
accepts:    `TOOLCHAIN.md` gains a chess-oracle section that names the trap, shows what
            the bad invocation actually prints, and gives the invocations that are
            safe -- each one run and its output copied rather than described, per
            par.7; `CLAUDE.md`'s tool table no longer carries a bare
            `go depth 20` pipe and points at that section, so the rule and the safe
            invocation are one hop apart; the existing correct entry points
            (`tools/analyse_game.py`, `build/tools/pgn_to_positions`) are named there
            as what already does it right; every path and command in the new text
            traced to the thing that produces it. Documentation only -- no build, no
            test, no match, and the fast suite is expected unchanged
touches:    TOOLCHAIN.md, CLAUDE.md, adocs/testing.md
excludes:   the rest of `TOOLCHAIN.md`, which is macOS throughout and stale on the
            DEC-049 machine -- that is a parked item in `status.md` and fixing it here
            would make this step a rewrite of the file; and any change to how the
            repository's own tools invoke an oracle, since they already invoke it
            correctly
decisions:
closes:
blocks:
paused_by:
done:      TOOLCHAIN.md gains a chess-oracle section: the trap reproduced (depth 1, nodes 0, cp 0, bestmove a1b1 on a mate in one), both safe invocations run and their output copied (mate 1, 320 nodes, a1a8), the repository's own correct entry points named, and the 320-node fact that kills the 'give it longer' reading. CLAUDE.md's tool table no longer carries the bad pipe and points at the section. The chesso half of the same race was already in DEV_MANUAL.md since S073, which is the gap this closes. Two of my own doc claims were wrong and were caught by tracing them. S070's past evidence checked and cleared -- a parse-time rejection, not a search. Fast suite 20/20, clang-format clean.

## Evidence

Found while doing S162's tool verification, 2026-08-22.
`printf 'position fen 7k/6pp/8/8/8/8/8/R6K w - - 99 60\ngo depth 20\nquit\n' |
stockfish` printed `info depth 1 seldepth 0 score cp 0 nodes 0 nps 0 time 1
pv` and `bestmove a1b1` -- a **draw score and a non-mating move for a position
that is mate in one**. The same pipe with `position startpos` and `go depth 8`
answered `bestmove a2a3` with no depth-8 line at all. The cause is `quit`:
stockfish searches on its own thread, `quit` stops it, and the pipe delivers
`quit` the instant `go` is written, so the reply is the first root move and
`nodes 0`. Driving the same two positions through `python-chess`'s
`SimpleEngine.analyse` at `depth 20` returned `Mate(+1)` for the first and a
real search for the second.

The failure mode is what makes this worth a step rather than a note: it is
silent, it is confidently specific, and it is in the *one* instrument
`CLAUDE.md` names for every chess question the agent is forbidden to answer
itself (DEC-023). `TOOLCHAIN.md` has no stockfish section at all, so nothing
warns about it, and the one worked example in the tree -- the comment above
`tests/test_engine.cpp`'s terminal-position case -- shows an interactive
session, which does not carry the trap and does not name it either.

## Shape

TOOLCHAIN.md gains a chess-oracle section: the trap, the two invocations that
are safe (`python-chess` `SimpleEngine`, or an interactive session), and the
existing `tools/analyse_game.py` / `pgn_to_positions` entry points that already
do it correctly. `CLAUDE.md`'s tool table gets a pointer so the rule and the
safe invocation are one hop apart. No code.

## Cost

Documentation only, no build, no match.
author:    Maksym Bodnar
