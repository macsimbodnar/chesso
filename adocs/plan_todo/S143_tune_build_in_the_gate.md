id:         S143
goal:       the completion gate builds and tests the tune build as well as the shipping one, so a change cannot leave build-tune broken unnoticed
accepts:    the documented completion gate builds `build-tune` and runs its `fast` label alongside `build`'s, and `DEV_MANUAL.md`'s stated gate command matches what actually runs; a red test is observed first -- reintroduce the `static_assert(ASPIRATION_MIN_DEPTH >= 2)` that found this, confirm the extended gate goes red on it and the old gate does not, then revert it; the added cost is measured and stated rather than assumed, since the gate runs at every step completion; nothing in `src/` changes
touches:    DEV_MANUAL.md, and whatever runs the gate
excludes:   the PGO and portable release targets, which are a separate question about release coverage rather than about the tune build; changing any test to make either build pass
decisions:  DEC-049
closes:
blocks:
paused_by:
done:

## How this was found

Fixing a Tier 1 review finding on S085, a `static_assert(ASPIRATION_MIN_DEPTH >=
2)` was added to `tests/test_engine.cpp`. It compiles in the shipping build,
where the parameter is `inline constexpr int`, and **fails to compile in the
tune build**, where S073 deliberately makes it a plain `int` so it can be set
over UCI:

```
tests/test_engine.cpp:1620: error: non-constant condition for static assertion
  the value of 'ASPIRATION_MIN_DEPTH' is not usable in a constant expression
```

The gate that runs at every step completion is
`cmake --build build -j12 && ctest --test-dir build -L fast && ./clang-format.sh --check`.
It went **green** with `build-tune` broken, because it never touches it. The
assertion was replaced with a runtime `REQUIRE`, which works in both, and this
step is the hole it exposed rather than the assertion.

## Why it matters more than it looks

`build-tune` is not a developer convenience. It is the binary every tuning run
plays: S085's SPSA run drove it for 60000 games, S127 will drive it again over
the full parameter set, and S068 and S039 are hand-tunes through the same
surface. A break in it is discovered when someone tries to tune, which may be
months after the commit that caused it, and the whole point of `search_params.hpp`
is that the two builds are *deliberately different code* -- so they can diverge
in exactly this way.

The two builds already have a common anchor, `search_param_info()`, which
`tests/test_search_params.cpp` holds the live values against so a default cannot
drift in one build alone. That anchor covers the values. Nothing covers whether
the tune build still compiles.

## Cost

No match, no verdict. The gate gets slower by one build and one suite, which is
what the accepts asks to be measured -- `build-tune` built from warm in about
the same time as `build` and its fast label ran 18/18 in 14 s when this was
found, but a figure taken once while a subagent held six cores is not a figure.
