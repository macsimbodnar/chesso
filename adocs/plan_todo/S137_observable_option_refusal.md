id:         S137
goal:       a refused or unknown search parameter is observable over UCI, so a tuner cannot play games against a silently-default value
accepts:    in the tune build, a `setoption` naming a parameter in `search_params.hpp` with a value outside its range prints one `info string` line naming the parameter, the value and the range, and a `setoption` naming no known option prints one naming the name; a test drives the binary over UCI and asserts both lines appear and that a legal value prints none; `MANUAL.md` documents the two lines and `test_uci_surface` covers them; the release build is unchanged and prints nothing new, proved by identical node counts and best moves from `tools/search_bench.py` (INV-6), so no SPRT is owed
touches:    src/chesso.cpp, tests/, MANUAL.md, DEV_MANUAL.md
excludes:   changing what is refused -- the refusal itself is right (`src/search_params.hpp:231-239`) and clamping instead would be worse; a readback command for live parameter values, which is a bigger surface and a separate decision; anything under `#ifndef CHESSO_TUNE`
decisions:  DEC-093
closes:
blocks:
paused_by:
done:

## Why this exists

Found while doing S084, whose `excludes:` puts `src/` out of reach.

The tune build refuses an out-of-range `setoption` rather than clamping it, and
logs the refusal through `LOG_W`. Under `NDEBUG` that macro is
`if (false) std::clog` (`src/log.hpp:35`) and `build-tune` is
`CMAKE_BUILD_TYPE=Release`, so **the refusal produces no output at all**. There
is no readback either: measured 2026-08-20,

```
setoption name RfpMargin value 120
uci                     # option name RfpMargin type spin default 75 ...
```

`uci` re-prints the compiled default, not the live value.

So the two ways a tuner can be wrong -- a value outside the range, and a
misspelled name -- are both indistinguishable from success. The run plays
thousands of games against a default parameter and converges confidently on
nothing. That is worse than a crash, and it is the failure mode
`adocs/plan_done/` has the most examples of: an instrument that reports fine
while measuring the wrong thing.

`tools/spsa_driver.py` works around it from the outside -- it clamps every value
itself and its `check` mode compares the config's bounds against the binary's
own `uci` listing before a game is played, then proves `setoption` reaches the
search by node count. That is the right defence for a driver and it is not a fix:
the next tool to drive this binary starts from the same silence.

`info string` is the channel because it is already legal UCI in any build state
and needs no new option. One line per refusal, not a stream.

## Cost

A source change of a few lines, a UCI test, two documentation lines. No match:
the tune build is not a strength number and the release build is untouched.
