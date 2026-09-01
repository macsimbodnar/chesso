id:         S143
goal:       the completion gate builds and tests the tune build as well as the shipping one, so a change cannot leave build-tune broken unnoticed
accepts:    the documented completion gate builds `build-tune` and runs its `fast` label alongside `build`'s, and `DEV_MANUAL.md`'s stated gate command matches what actually runs; a red test is observed first -- reintroduce the `static_assert(ASPIRATION_MIN_DEPTH >= 2)` that found this, confirm the extended gate goes red on it and the old gate does not, then revert it; the added cost is measured and stated rather than assumed, since the gate runs at every step completion; nothing in `src/` changes
touches:    DEV_MANUAL.md, and whatever runs the gate
excludes:   the PGO and portable release targets, which are a separate question about release coverage rather than about the tune build; changing any test to make either build pass
decisions:  DEC-049, DEC-118
closes:
blocks:
paused_by:
done:       2026-09-01. The gate builds and tests **both** builds now --
            `cmake --build build -j8 && ctest --test-dir build -L fast
            --output-on-failure && cmake --build build-tune -j8 && ctest
            --test-dir build-tune -L fast --output-on-failure &&
            ./clang-format.sh --check` -- in `AGENTS.md`'s TESTS rule and in
            DEV_MANUAL.md's Test section, which are the only two places the
            command is written and are now identical. DEC-118.

            **The red was observed, not assumed.** With
            `static_assert(ASPIRATION_MIN_DEPTH >= 2)` put back at
            `tests/test_engine.cpp:1628`, the old gate exits **0** and the
            extended gate exits **2**, failing at the `build-tune` compile with
            `read of non-const variable 'ASPIRATION_MIN_DEPTH' is not allowed
            in a constant expression`. The assertion was then reverted; nothing
            in `src/` or `tests/` is changed by this step.

            **Cost, measured rather than assumed, on 8 cores on mains.** The
            tune build's `fast` label is **45.9 s over 22 tests**, against the
            shipping build's **42.4 s** over the same 22 -- the same suite, and
            the step file's "18/18 in 14 s" is what it was when this was found.
            Building `build-tune` adds **0.46 s** no-op, **1.38 s** for a full
            rebuild with its ccache warm, **18.03 s** with `CCACHE_DISABLE=1`.
            End to end the extended gate ran green in **93.59 s**, 22/22 twice,
            against about 45 s before: it roughly doubles.

            **Two things the measurement showed that the step file did not
            predict.** `build/` is configured with **no compiler launcher**
            and `build-tune/` with `ccache`, so only the tune build has a
            warm-cache case and the shipping build pays its compiles in full.
            And DEV_MANUAL.md's Test section still claimed the fast suite was
            "about 18 s" in two places; it is 42 s over 22 tests, so both were
            corrected in passing -- a stale figure in the same section the gate
            is documented in.

            **Not done, and parked rather than planned.** Nothing checks that
            the two copies of the gate command agree: `tools/plan_prose_check.py
            --params` is the precedent for turning exactly this class of prose
            drift into a `fast` test, and DEC-118's "they are changed together"
            is an assumption where that would be a guard. A step is created by
            a decision and none has been taken on this.

## How this was found

Fixing a Tier 1 review finding on S085, a `static_assert(ASPIRATION_MIN_DEPTH >=
2)` was added to `tests/test_engine.cpp`. It compiles in the shipping build,
where the parameter is `inline constexpr int`, and **fails to compile in the
tune build**, where S073 deliberately makes it a plain `int` so it can be set
over UCI:

```
error: non-constant condition for static assertion, at the
  ASPIRATION_MIN_DEPTH REQUIRE in tests/test_engine.cpp
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
