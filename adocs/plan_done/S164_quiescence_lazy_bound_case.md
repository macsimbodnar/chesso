id:         S164
goal:       the S130 substitution is pinned by a unit case on the lazy-bound stand-pat path, the one input class its tests do not reach
accepts:    one unit case on the quiet anchor position with window (0, 100), so the lazy shortcut fires high (cheap 567 − 184 = 383 ≥ beta): plant `TT_ALPHA_NODE` score 200 with `TT_EVAL_NONE` — 200 > alpha so the probe does not answer, 200 < 383 so the cap fires — and assert the return is 200 and the stored entry is `TT_BETA_NODE(200)` with `eval == TT_EVAL_NONE`; the case is shown red under a temporarily inverted cap comparison (local mutation, never committed) and green on the tree as written
touches:    tests/test_search.cpp, adocs/testing.md, adocs/specs.md
            -- the last two are the ledger row and the one clause in the S130
            paragraph that claimed coverage this class did not have
excludes:   any change to src/search.cpp — this step adds coverage, not behaviour
decisions:
closes:     2026-08-22_adversarial-F07
blocks:
paused_by:
done:      One unit case pins the S130 substitution where the stand pat is a bound rather than a static score: window (0, 100) on the anchor, cheap 567 less margin 184 = 383, planted TT_ALPHA_NODE(200) with TT_EVAL_NONE, returns 200 and stores TT_BETA_NODE(200) with eval TT_EVAL_NONE. 18 assertions green. Red under two mutations, and the decisive one is gating the substitution on static_eval_is_exact: invisible to all 70 other test_search cases and to the other 19 fast tests, caught only here. src/search.cpp byte-identical. Fast suite 20/20, clang-format clean. Closes 2026-08-22_adversarial-F07.

## Evidence

2026-08-22_adversarial-F07. S130's substitution compares the table score
against `stand_pat` (`src/search.cpp:352-368`), and `stand_pat` is not always
a static score: when the probe carries no usable eval and the lazy shortcut
fires, `evaluate_lazy()` returns a window-side bound (`cheap ±
LAZY_EVAL_MARGIN`). The audit verified the soundness argument for
substituting against that bound case by case — but every S130 test uses
windows wide enough that the shortcut cannot fire, so `static_eval` is the
exact 563 in every assertion. A future edit that reorders the lazy call and
the substitution, or flips one comparison on the bound path, leaves every
current test green — the S106 failure class, on exactly the input class the
tests do not reach.

## Contingency

This step exists only if S130's substitution ships. If S130's verdict
reverts it, retire this step by a recorded decision naming the finding.
`tests/test_search.cpp` is S130's own landing file, so this runs strictly
after S130 closes.

## Cost

One test case, no match.
author:    Maksym Bodnar
