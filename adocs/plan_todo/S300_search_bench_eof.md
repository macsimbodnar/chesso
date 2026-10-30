id:         S300
goal:       `tools/search_bench.py` stops with an error, and does not spin forever, when the engine closes its output before `uciok`
accepts:    an engine that exits or closes stdout before `uciok` makes `search_bench.py` exit non-zero with a message naming the handshake, shown on a planted stand-in engine; the output on the real binary is byte-identical at depths 9 and 12 to the output before the change, nodes and best moves alike (INV-6 for the tool)
touches:    tools/search_bench.py
excludes:   any other change to the tool's protocol, output or positions; any timing run
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

S299's subagent found it while planting failing engines into the cost
scripts' `streams()`. The `uci` handshake loop in `search_bench.py` reads
`while "uciok" not in p.stdout.readline(): pass`. At end of file `readline()`
returns the empty string forever, so an engine that dies or closes its output
before `uciok` spins the loop at full CPU until something kills it. The
`isready` loop below it already raises on end of file; the `uci` loop does not.

S299's `streams()` reaches `search_bench.py` only after both benches pass, so
a dead or junk engine stops at bench. An engine with a working bench and a
UCI that dies before `uciok` would hang the stream until the watcher's
ceiling. It would never pass. The defect is in a measurement tool, not
reachable in ordinary play, on the UCI surface, or in a reported score
(DEC-171). Filler, behind the next strength step. Filed by the coordinator,
2026-10-30.
