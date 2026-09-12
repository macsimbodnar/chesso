# Adversarial audit — 98af071d535dc98babf146c974821b4f4ce6c0fb

Audited 2026-09-12 against `98af071d535dc98babf146c974821b4f4ce6c0fb`.

Scope: all implemented engine techniques, public UCI paths and their safety
boundaries, the active/future plan, and the disposition of earlier adversarial
findings.  I read the current specifications, plan and status; traced the
move-generation, FEN-loading, search and UCI paths; re-ran targeted UCI
reproducers; and checked the planned finding-to-step mapping.  This is an
audit report only: no implementation or test was changed.

## Findings

### 2026-09-12_adversarial-F01 — high — `position fen` accepts a move that captures the enemy king

Status: planned — S223 (DEC-197), at Open entry 9 behind S109 beside S210 under DEC-171's reach rule, not at entry 1; the step file is rewritten to the house shape.

`adocs/specs.md`'s prime directive says that Chesso never plays or accepts an
illegal move and never corrupts its board state.  The public `position fen`
path knowingly accepts a state in which the side that just moved is in check,
then accepts the resulting pseudo-legal king capture.

Evidence:

```sh
printf 'position fen 7k/8/8/8/8/8/8/K6R w - - 0 1 moves h1h8\nfen\nquit\n' \
  | ./build/src/chesso 2>&1
```

produced:

```
7R/8/8/8/8/8/8/K7 b - - 0 1
```

The black king has been removed.  Independently, python-chess assigns that
input `STATUS_OPPOSITE_CHECK` (`1024`), confirming the FEN is not a reachable
game state; that library still exposes pseudo-legal moves on invalid boards,
so it is not used here as the move-legality oracle.

The acceptance is intentional in the implementation:

- `src/bitboard.cpp:1917-1919` says that the side not to move being in check
  is deliberately not checked.
- `src/bitboard.cpp:1954-1956` says that one king per side is deliberately not
  required.
- `src/chesso.cpp:1396-1494` loads the FEN and applies each supplied move;
  `try_move()` at `src/chesso.cpp:620-644` selects it from generated moves and
  makes it.
- `tests/test_search.cpp:3014-3025` explicitly calls this a "capturable king"
  survivability case, while `tests/test_helpers.hpp:85-86` excludes such cases
  from its reachability helper.

Impact: a caller using the documented UCI surface can make the engine accept
an illegal move and leave a board with no black king.  The test that calls this
survivable is useful as an internal robustness probe, but it cannot justify
exposing the same state as a valid `position fen` input under the prime
directive.

Suggested resolution: reject, without altering the current position, a public
FEN that has other-than-one king of either colour, adjacent kings, or a king of
the side not to move in check.  Keep intentionally malformed positions in
test-only construction instead of the UCI loader.  Add a UCI regression that
first establishes a valid current position, refuses this FEN/move sequence,
and proves the prior board remains unchanged.  Any narrower boundary needs an
explicit decision reconciling it with the prime directive.

### 2026-09-12_adversarial-F02 — low — prior F34 has no plan step or recorded decision

Status: accepted in part — the decision existed: DEC-170 (2026-09-11) ruled F34 against regularisation, by short id; the verbatim-id gap is closed by S134's `closes:` and its measured accepts (DEC-197).

`2026-09-10_adversarial-F34` found that the tuner has no regularisation and no
decision saying that deletion of the documented degenerate columns is the
chosen alternative.  It is neither closed nor assigned to a current or future
step: searching all three plan-state directories and `adocs/decisions.md` for
`2026-09-10_adversarial-F34` returns no match.  `S134` closes only F36
(`adocs/plan_todo/S134_fold_degenerate_eval_columns.md:7`), despite being the
nearest related work.

Impact: the future plan has no durable answer to whether the remaining
parameterisation is intentionally unregularised.  A future fit can therefore
rediscover the same ambiguity without a decision or acceptance condition.

Suggested resolution: either add F34 to an explicit plan step with a
testable analysis of the remaining degeneracies, or record the owner's
decision that column folding is sufficient and why regularisation is not
needed.  Do not treat S134 as closing it unless its scope and acceptance prove
that conclusion.

## Prior-finding reassessment

The current plan still gives a home to nearly all open findings from the
2026-09-10 audit: F01-F03 map to S211; F04-F07 and F31-F32 to S212; F16 and
F25 to S039; F17-F23 to S210; F24 to S126; F26-F27 and F33 to S213; F28-F29
and F35 to S214; F30 to S199; F36 to S134; and F37 to S186.  F34 is the sole
unmapped finding, as recorded in F02 above.  F08-F13 have their recorded
closing work in S207-S209; F14-F15 are recorded accepted decisions.

The still-open protocol finding F19 is broader than its original depth-only
reproducer.  This command returned `bestmove a2a3` before `stop`:

```sh
{ printf 'uci\nposition startpos\ngo infinite nodes 1\n'; sleep 0.5;
  printf 'isready\nstop\nquit\n'; } | ./build/src/chesso 2>&1 \
  | sed -n '/^id name\|^bestmove\|^readyok\|^info /p'
```

Its output was `id name Chesso`, `bestmove a2a3`, then `readyok`.  S210's
acceptance must therefore ensure that `infinite` clears every finite stop
condition, not only `depth`.

The 2026-09-04 protocol findings remain folded into S210, as its `closes:`
field states.  Earlier FEN crash/out-of-bounds findings are separately bounded
by the S208 semantic checks; the new F01 is outside that bound because the
current specification explicitly preserves broader position-at-large
acceptance.

## Plan review

The planned search/evaluation sequence remains coherently ordered around its
measurement and provenance gates.  The two issues above are plan blockers at
their respective layers: F01 is a public-surface prime-directive defect and
must be fixed under the repository's bug rule before another strength change;
F02 needs a recorded disposition before the later tuning work can be said to
cover the audit's parameter-identifiability concern.
