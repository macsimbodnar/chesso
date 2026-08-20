id:         S137
goal:       a refused or unknown search parameter is observable over UCI, so a tuner cannot play games against a silently-default value
accepts:    in the tune build, a `setoption` naming a parameter in `search_params.hpp` with a value outside its range prints one `info string` line naming the parameter, the value and the range, and a `setoption` naming no known option prints one naming the name; a test drives the binary over UCI and asserts both lines appear and that a legal value prints none; `MANUAL.md` documents the two lines and `test_uci_surface` covers them; the release build is unchanged and prints nothing new, proved by identical node counts and best moves from `tools/search_bench.py` (INV-6), so no SPRT is owed
touches:    src/chesso.cpp, tests/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md,
            and -- outside this list, because this commit is what falsified them --
            tools/spsa_driver.py and tests/test_spsa_driver.py, comments only
excludes:   changing what is refused -- the refusal itself is right (`src/search_params.hpp:231-239`) and clamping instead would be worse; a readback command for live parameter values, which is a bigger surface and a separate decision; anything under `#ifndef CHESSO_TUNE`
decisions:  DEC-093
closes:
blocks:
paused_by:
done:      Three info string lines, tune build only. A value outside a parameter's range, a
            value that is not an integer, and a name no option has each answer with one
            `info string refused [...]` line on stdout; a legal value prints nothing.
            Measured on the real binary: `setoption name RfpMargin value 5000` ->
            `info string refused [RfpMargin] value 5000, outside [0, 2000]`, `Rfpmargin` ->
            `unknown option`, `value abc` -> `not an integer, range [0, 2000]`, and
            `Hash 32` / `Use Book true` -> nothing. The release binary prints nothing for
            any of them.
            
            The non-integer case is a third line the accepts did not enumerate. Kept: it is
            the same silence, on the same code path (the `catch` that used to be LOG_W),
            and leaving it out would have failed the goal rather than the accepts.
            
            Red first, and the red was nearly invisible. `REQUIRE(out_of_range.size() == 1)`
            failed at 0 with no report printed, because doctest writes its failure report to
            `std::cout` and that is the stream `stdout_capture_t` is holding. The
            assertions now run outside the capture scope through
            `lines_from_setoption()`; the same trap is in the other cases of that file and
            they were left alone.
            
            INV-6 discharged, no SPRT owed: `tools/search_bench.py ./build/src/chesso 9`
            identical before and after -- 164302 / 672272 / 85168 nodes, best `c3d5` /
            `e2a6` / `d7c8q`. Both configurations green, 18 of 18 fast tests each.
            
            Guard against the duplication the fix introduced: `command_setoption` now
            carries its own list of the three option names that are not search parameters,
            so "what uci advertises, setoption recognises" drives every name out of the
            engine's own `option` lines through `setoption` and fails if one comes back
            unknown. Non-vacuous by a REQUIRE that an unadvertised name does.
            
            Documents. `MANUAL.md` and `DEV_MANUAL.md` both said the refusal was
            unobservable and now carry the three templates verbatim, which
            `test_uci_surface` pins. `adocs/specs.md` gained the behaviour. Three more
            sites were corrected outside `touches:` because this commit is what made them
            false -- `tools/spsa_driver.py`'s module and `probe_nodes` docstrings and
            `tests/test_spsa_driver.py`'s clamp docstring, comments only, no behaviour.
            The driver still clamps and still bounds-checks before a game: a line printed
            mid-run is a post-mortem, not a substitute for validating a config.
            
            Two stale measurements found while in those paragraphs, re-measured here.
            DEV_MANUAL's `RfpMargin` node series was last taken at S103's commit and S104,
            S106 and S107 have moved the tree since: 164123 / 223454 / 476911 / 743308 ->
            **164302 / 223857 / 474204 / 742729**, best move `c3d5` throughout, and 164302
            with no `setoption` at all, which is what the release build reports to the
            node. And specs.md's "the ten parameters in src/search_params.hpp" have been 22
            since S089; the count is now not written there, because `test_search_params` is
            where it is pinned.
            
            `README.md` checked, owner-written, no change needed.

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
author:    Maksym Bodnar
