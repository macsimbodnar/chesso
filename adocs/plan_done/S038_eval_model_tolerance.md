id:         S038
goal:       the tuner-model guard states a tolerance the truncation arithmetic actually supports
accepts:    test_eval_model passes over a corpus that includes the FENs recorded in the audit finding; the tolerance and its comment agree with the number of integer divisions evaluate() actually performs
touches:    tests/test_eval_model.cpp
excludes:   any change to src/evaluation.cpp or tools/eval_model.hpp at all -- the evaluation's output stays bit-identical, so this step alters no play and gets no SPRT
decisions:  DEC-053
closes:     2026-08-13_adversarial-F04
blocks:
paused_by:
done:      Tolerance 2.0 -> 3.0 in tests/test_eval_model.cpp, comment re-derived from the code, four F04 FENs pinned. Nothing else changed: git diff --quiet 7068cd2 -- src/evaluation.cpp tools/eval_model.hpp exits 0 and sha256 agrees either side (18ab9550..5323, b3a96b65..5335), so evaluate() is bit-identical, this alters no play and owes no SPRT.
                Re-derived the truncation count: evaluate() divides by GAME_PHASE_MAX FOUR times, not the two the old comment claimed nor the three F04 counted -- :640-643 psqt plus pawn terms, :668-670 tempo, :951-952 mobility, :953-954 king safety, with GAME_PHASE_MAX defined once at evaluation.hpp:289 so there is no fifth. F04 missed tempo. Its bound of 2.875 was right anyway: tempo ships at tempo_mg == tempo_eg == 0 (:582-583), so that division truncates 0/24 exactly and three of four can round, 3 x 23/24 = 2.875. Over all four it is 3.833, which the new case asserts the premise of.
                Red observed twice. With the four FENs added and the tolerance still 2.0: CHECK( 2.875 <= 2 ), CHECK( 2.33333 <= 2 ), CHECK( 2.25 <= 2 ), CHECK( 2.125 <= 2 ), each logging its FEN, the 26 pre-existing positions all passing. Those four differences reproduce F04's recorded DRIFT values to the digit, which is what says model and engine disagree here exactly as they did on the retired machine -- .tuning/ does not exist on this one, so F04's own reproduction cannot be re-run. Then the new case with one pinned FEN swapped for the start position: CHECK( 0 > 2 ) naming it at difference 0.000000, and CHECK( 2.33333 > 2.8 ) for the worst, so a pinned position that agrees for free cannot pass.
                Gate green: cmake --build build -j12 exit 0, ctest -L fast 11/11 (test_eval_model 17 cases, 2604 assertions, 0.01 s), ./clang-format.sh --check exit 0. DEV_MANUAL.md checked -- its only claim about this test is that it stops the model drifting, still true and carrying no number; MANUAL.md checked, no mention. README.md owner-written, no change needed. Three testing.md rows.
                Rejected route is S055 in plan_todo, after S042 and before S029, unstarted. Found while re-deriving: tools/eval_model.hpp:973-974 already tapers the summed pair once, so S055 moves the engine to the shape the model has and needs no model arithmetic change. 2026-08-13_adversarial-F04 stays planned; only an audit re-run closes it.

## What is broken

`tests/test_eval_model.cpp:313` asserts `std::abs(model - engine_white) <= 2.0`,
with a comment saying `evaluate()` truncates "twice, once for the tables and
once for mobility".

The test passes only because its corpus is 26 hand-picked FENs. Over the real
tuning corpus, 3446 of 200000 positions exceed 2 cp, worst exactly 2.875:

```
positions 200000 beyond 2cp slack 3446 worst 2.875
worst fen 2r1r1k1/4Q1p1/p1P1p1q1/3p3p/1P1PpP2/4P2P/PB4P1/R4RK1 w - - 5 29
```

## It truncates four times, not two and not three

Re-derived from the code at `7068cd2` rather than taken from the audit, which is
what the accepts asks for. `evaluate()` is
`evaluate_cheap() + evaluate_expensive()` (`src/evaluation.cpp:990-991`) and
divides by `GAME_PHASE_MAX` four times, in execution order:

| line | what is tapered | in |
|---|---|---|
| `src/evaluation.cpp:640-643` | the psqt pair plus the pawn terms | `evaluate_cheap()` |
| `src/evaluation.cpp:668-670` | tempo | `evaluate_cheap()` |
| `src/evaluation.cpp:951-952` | mobility | `evaluate_mobility_and_king_safety()` |
| `src/evaluation.cpp:953-954` | king safety | same function |

`grep -n 'GAME_PHASE_MAX' src/evaluation.cpp` finds no fifth, and
`GAME_PHASE_MAX` is defined only at `src/evaluation.hpp:289`.

**F04 counted three and missed tempo**, whose own comment
(`src/evaluation.cpp:647-670`) documents it as a deliberate second truncation and
prices it: "one centipawn against the tuner's floating-point model". The audit's
*bound* of 2.875 was nevertheless right, and for a reason it did not state:
tempo ships at `tempo_mg == tempo_eg == 0` (`src/evaluation.cpp:582-583`), so
that division truncates `0 / 24` exactly and contributes nothing. Three
effective truncations, 3 x 23/24 = 2.875. F04's measured worst case of exactly
2.875 is that arithmetic confirmed.

The general bound over all four is 4 x 23/24 = 3.833, and it becomes the live one
the moment tempo is fitted. That is a premise, so the test asserts it rather than
commenting it.

## What is not broken

The model itself. The clamp sits in the same place in both
(`tools/eval_model.hpp:976` against `src/evaluation.cpp:984`), colour symmetry
holds over 300000 positions, and the worst observed difference is exactly the
truncation bound rather than anything larger. What is wrong is the guard.

Found while re-deriving it: `tools/eval_model.hpp:973-974` already sums mobility
and king safety before tapering them **once**. The extra truncation is the
engine's alone, so S055 moves the engine to the shape the model already has and
needs no arithmetic change on the model side.

## Why it matters at all

This is the only thing stopping `tools/eval_model.hpp` drifting from
`evaluate()`, and a fit is only as good as that correspondence. At 2 cp against
a legitimate 2.875 cp of truncation the tolerance cannot separate rounding from
a real model error of up to about 2.8 cp per position — which, over 1.49 M
positions, is a systematic bias the fit absorbs happily and silently.

It also trains the wrong reflex: extending the corpus toward real play makes the
test fail for a reason unconnected to any defect, and the obvious response is to
widen the tolerance rather than read it.

## The route, decided

Two routes existed and this file did not choose between them. **The owner chose:
raise the slack, do not touch the evaluation.** DEC-053 records both, their
costs, and the three rejected alternatives.

- Taken: tolerance 2.0 -> 3.0, the comment re-derived to name all four divisions
  with their lines and to say which one is inert and why, and the four FENs F04
  recorded pinned into the corpus with a case that keeps them exercising the
  bound.
- Deferred to **S055**: taper mobility and king safety through one summed
  division, which removes the third effective truncation, puts the bound at
  2 x 23/24 = 1.917 and the tolerance back to 2. It alters play, so it is an
  SPRT, and its expected verdict is 0 — which is exactly why it does not belong
  in this step. Placed after S042 and before S029.

The corpus this was measured on is gitignored and does not exist on the DEC-049
machine, so F04's reproduction cannot be re-run here; the pinned FENs are the
record of it inside the suite. All four reproduce the audit's differences to the
digit, which is what says the model and the engine disagree here exactly as they
did there.

`2026-08-13_adversarial-F04` stays `planned`. Only an audit re-run closes a
finding.
