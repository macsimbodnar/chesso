id:         S166
goal:       the documented stockfish invocation cannot silently answer without searching: TOOLCHAIN.md gains a chess-oracle section naming the quit-aborts-the-search trap, and CLAUDE.md's tool table points at it
accepts:    acceptance criteria, testable
touches:    areas of the codebase
excludes:   explicitly out of scope
decisions:
closes:
blocks:
paused_by:
done:

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
