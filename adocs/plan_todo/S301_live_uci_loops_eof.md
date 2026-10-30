id:         S301
goal:       the live tools that drive the engine over UCI stop with an error, instead of spinning, when the engine closes its output during a handshake or a search, as `tools/search_bench.py` does since S300
accepts:    `build_release.sh`, `adocs/data/S135_cost.py` and `tools/error_profile.py` each raise at end of file in every loop that waits for `uciok`, `readyok` or `bestmove`, with a message naming what it waited for, shown on a planted stand-in engine that exits and one that closes stdout and stays alive; `tools/search_bench.py`'s `go` loop raises at end of file instead of dropping that position's row; each tool's output on the real binary is unchanged (nodes and moves equal, timings stripped); no timing is taken
touches:    build_release.sh, adocs/data/S135_cost.py, tools/error_profile.py, tools/search_bench.py
excludes:   the frozen scripts under `adocs/data/` that no live script calls (they are history, re-run only to reproduce their step); any change to a tool's protocol, output or workload; any timing run
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

S300's subagent found the spinning loop it fixed in `tools/search_bench.py`
live in three more places. One is `build_release.sh`'s handshake. Another is
`adocs/data/S135_cost.py`'s, the workload that `S121_cost.sh`,
`S123_cost.sh`, `S125_cost.sh` and `S135_cost.sh` time. The third is
`tools/error_profile.py`'s `uciok` and `readyok` waits. Each is
`while "<token>" not in readline(): pass`. At end of file `readline()`
returns the empty string forever, so an engine that dies mid-handshake
spins the tool at full CPU until something kills it.

`search_bench.py`'s `go` loop breaks silently at end of file instead. It
drops that position's row, and only a caller that counts rows notices.
The cost scripts count rows, so they do.

None of these can pass a wrong number; each can only hang until a watcher's
ceiling or a timeout. Not reachable in ordinary play, on the UCI surface,
or in a reported score (DEC-171). Filler, behind the next strength step.
Filed by the coordinator, 2026-10-30.
