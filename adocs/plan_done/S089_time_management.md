id:         S089
goal:       a time budget that scales with best-move stability and with a falling score, instead of remaining over a fixed movestogo plus half the increment
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); a soft limit that decides whether to start another iteration and a hard limit that stops the search inside one, with the hard limit never exceeding the time the clock actually has and a test that asserts it on a short clock; the budget scales by how long the best move has been stable and by a score that has fallen since the previous iteration, each factor a constant in src/search_params.hpp with a stated range (S073); no fixed movestogo assumption at a sudden-death control; tests/test_engine.cpp keeps a case that a 1 ms clock returns a legal move, which is the S036 defect (amended 2026-08-18 by DEC-078 from tests/test_search.cpp, which has no such case and never had one); the fast suite green
touches:    src/chesso.cpp compute_search_time_ms and the iterative deepening loop, src/uci.hpp DEFAULT_MOVES_TO_GO, src/search_params.hpp, tests/test_search.cpp
excludes:   pondering; multi-threading; any change to what the search itself computes
decisions:  DEC-071
closes:
blocks:
paused_by:
done:      SPRT H1 accepted at elo0=0 elo1=10, 500 games in 21 m 59 s against 1069cc6, Elo +45.42 +/- 23.47, nElo +59.71 +/- 30.45, Ptnml [13, 44, 94, 63, 36]. The point estimate is not the effect size; recorded verdict is not a regression, sign positive. 0 time forfeits in the 500 games, checked from the PGN filtered to this run after the log-based check proved vacuous. Full suite 19/19, clang-format clean.

## What is there now

`src/chesso.cpp:405-410`:

    budget = (remaining_ms / movestogo) + (increment_ms / 2)

with `DEFAULT_MOVES_TO_GO 20` (`src/uci.hpp:13`) used when the GUI sends no
`movestogo`, and a soft-limit percentage at `chesso.cpp:589-592` deciding at
`:761` whether to begin another iteration. Nothing looks at the search at all:
a position whose best move has been the same since depth 6 gets the same slice
as one where it changed at every iteration, and a score that just fell 200 cp
buys no extra time.

The whole match record of this project, including the 2570, was played on that
allocation.

## Why it is first of the search block

It is the only item on the DEC-071 list that costs no search correctness risk:
it prunes nothing, reduces nothing and cannot hide a mate. It also interacts
with nothing else queued, so its verdict is clean whenever it runs.

## Hazard

The failure mode is losing on time, and a forfeit is a whole point rather than
noise. The hard limit is asserted by a test, not by argument, and the SPRT log
is checked for time losses before the verdict is read.
author:    Maksym Bodnar

## As executed

Implemented at `a10ca84`; verdict and documents in the completing commit.

### The verdict

**`SPRT ([0.00, 10.00]) completed - H1 was accepted`**, 500 games in
**21 m 59 s** against `1069cc6`. `Elo: 45.42 +/- 23.47`, `nElo: 59.71 +/- 30.45`,
`Ptnml(0-2): [13, 44, 94, 63, 36]`, `LLR: 2.21 (100.7 %)`.

**+45.42 is not the effect size.** An SPRT stops early exactly when the observed
effect has run favourable; S068's pooled estimate fell from +12.18 to +5.02 on
that correction. The recorded verdict is **not a regression, sign positive**.

`REF` was passed explicitly. `fastchess.sh:22` defaults it to the fixed old
commit `7b4d9a4`, so the default would have measured this change against months
of unrelated work -- DEC-020's contamination, still armed in the script.

### The hazard check, and the vacuous version of it that came first

The step names the hazard: "the failure mode is losing on time, and a forfeit is
a whole point rather than noise... the SPRT log is checked for time losses before
the verdict is read."

**The log check was vacuous.** `/tmp/fastchess_fast.log` was **0 bytes, dated two
days earlier**, so grepping it for `loses on time` returned 0 for free. It was
nearly reported as a pass.

Diagnosed rather than patched around: `-log file=X` over a 2-game probe wrote
**0 bytes**, and the identical match with `level=trace` wrote **230 KB**. The log
is WARN-and-above -- a time loss *does* reach it, which is how `rating.sh` caught
Stash -- but an empty log is **indistinguishable from one that was never
written**.

**The real check is the PGN**, and it found a second trap: `/tmp/fastchess_fast.pgn`
is **appended across runs** and held 587 games, 87 of them against
`ref-c56ab41`. An unfiltered census reported 421 `adjudication` / 166 `normal`
instead of this run's real 359 / 141. Filtered to the 500 games naming
`ref-1069cc6`: **0 time forfeits**. Non-vacuous by construction -- the same pass
classifies all 500 terminations, so a forfeit could not hide in an empty input.

Both traps are now in `DEV_MANUAL.md`.

### What the SPRT actually measured

**Not a re-tuned base allocation.** 5 % of the clock plus 50 % of the increment
is arithmetically what `remaining/20 + inc/2` produced, so the base is unchanged
on purpose. What moved is the **soft/hard split** -- the hard limit went from
1.0x the allocation to 3.0x, so the engine can finish an iteration it started --
and the **scaling** by best-move stability and by a falling score. One change at
a time, and the verdict is attributable to that pair rather than to a bigger
budget.

### Carried out of the step, not fixed in it

Reported by the implementing agent, out of `touches:`, and left as findings:

1. **`stop_search_after_ms()` spawns a detached thread per `go`** that sleeps the
   whole budget before checking `session_id`. At 3x the old hard limit more of
   them overlap. They exit and the session check works, so it is not a leak.
2. **The first iteration is still not abortable** -- `state.stop = &never_stop`
   until depth 1 completes. Pre-existing and deliberate: it is what makes the
   1 ms case answer at all. So "the hard limit stops the search inside an
   iteration" holds from depth 2 on.
3. `uci_search_options_t::movestogo` defaulted to 1 while `command_go` overwrote
   it with 20. Inert, since the deepening loop never read it; now 0, and the
   three dead assignments in `test_engine.cpp` are gone.

DEC-078 records the `accepts:` clause that named the wrong test file.
