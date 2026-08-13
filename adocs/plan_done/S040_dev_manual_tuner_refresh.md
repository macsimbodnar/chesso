id:         S040
goal:       re-derive the DEV_MANUAL tuner section from tools/tuner.cpp and tools/eval_model.hpp
accepts:    every number and name in the tuner section is traced to the code that produces it; the paste target lists every weight array a fit writes, tempo included
touches:    DEV_MANUAL.md tuner section
excludes:   the tuner's behaviour -- nothing in tools/ changes here
decisions:  DEC-041
closes:     2026-08-13_adversarial-F06
blocks:
paused_by:
done:      2026-08-13. The DEV_MANUAL tuner section re-derived from the code, not from F06. **825 -> 827** (`eval_model::PARAM_COUNT`, tools/eval_model.hpp:136-139) with tempo's 2 added to the breakdown; **`tempo` added to the --only list** (GROUP_LIST, tools/tuner.cpp:153-155; free_mask :209-211); **the paste target is now a table of every definition a fit writes**, enumerated from the header write_tables() emitted on a real run -- five defines and psqt_mg/psqt_eg into src/eval_tables.hpp, then mobility_mg/eg, king_safety_mg/eg, passed_pawn_mg/eg, pawn_structure_mg/eg, piece_placement_mg/eg and the two tempo scalars into src/evaluation.cpp, 54 parameters in the second file; **three swallowing occurrences -> four**, the fourth `piece_placement` past the tempo block (tools/tuner.cpp:204-206), with `tempo` named as the group running to PARAM_COUNT today. 827 confirmed from the code path and not only by arithmetic: nine tuner runs over a two-row TSV printed 827 parameters with free counts 827, 5, 768, 8, 18, 12, 6, 8, 2, so the eight named groups partition the vector exactly -- now stated in the manual as something checkable in one command. Beyond F06: the datagen paragraph gave two of the four conditions tools/datagen.cpp:254-256 applies and never mentioned --quiet-limit, corrected. **One F06-adjacent claim was checked and left alone because the code is the wrong one.** The manual dates passed pawns to S027; tools/eval_model.hpp:52, tools/tuner.cpp:8, :628 and :669 date them to S035, which is S035_fastchess_script_repair. S027's record lists passed pawns in its goal and describes the group-boundary bug hitting their twelve weights, so the plan directory wins on what a step id names and the four comments are a defect in tools/, which excludes: puts out of reach. Reported, not fixed. **This step file was itself corrected in place while current**, per S043's lesson: it said src/eval_model.hpp for tools/eval_model.hpp, and four of its five code line ranges were stale (GROUP_LIST :140-142 -> :153-155, free_mask tempo :198 -> :209-211, tempo weights src/evaluation.cpp:580-581 -> :582-583, the fourth swallowing occurrence :194-197 -> :204-206). plan_done/ is immutable; plan_current/ is not. Six ledger rows, one per claim traced, each naming the file and lines the number came from. MANUAL.md checked -- no tuner, tuning, weight or parameter surface anywhere in it, no change. README.md checked, owner-written, no change needed. No SPRT: documentation cannot alter play. 2026-08-13_adversarial-F06 stays `planned`; only an audit re-run closes it. Gate: cmake --build build -j12 clean, ctest -L fast 9/9, clang-format.sh --check clean.

## Four claims that no longer match the code

The code locations below were re-derived while doing the step; the ones this
file was written with were stale, and the corrected ones are marked. The
`DEV_MANUAL.md` line numbers are dropped rather than corrected -- the edit moves
them, so the claim is named instead.

- The manual says the tuner fits 825 numbers. `eval_model::PARAM_COUNT` is
  **827** (`tools/eval_model.hpp:136-139`), and the breakdown after it sums to
  825 because it omits tempo's 2.
- The manual lists the `--only` groups and stops before `tempo`. `GROUP_LIST`
  carries it at `tools/tuner.cpp:153-155` (not `:140-142`) and `free_mask`
  implements it at `:209-211` (not `:198`). A reader following the manual does
  not know `--only tempo` exists.
- The manual names the paste target as "the mobility, king safety, passed pawn,
  pawn structure and piece placement weights". The tempo weights
  (`src/evaluation.cpp:582-583`, not `:580-581`) are missing from that list.
- The manual says the group-swallowing bug "has now happened three times" and
  gives three examples. The fourth is recorded at `tools/tuner.cpp:204-206`
  (not `:194-197`, which is the second): `piece_placement` reached to
  `PARAM_COUNT` until the tempo block was appended.

The file is `tools/eval_model.hpp`, not `src/eval_model.hpp` as this step file
first said. `tools/eval_model.hpp:15-16` states why: it is not part of the
engine and nothing in `src/` includes it.

## Why the paste omission is the one that matters

The manual's own warning is that "a fit that is half applied looks like a fit
that did not work". A fit applied from that list leaves the tempo weights
behind, and the symptom — a term that measures zero — is one this project has
already seen for real reasons, so nobody goes looking for a paste error.

By DEC-041 the reader of this section is now an agent running a fit
unsupervised, which removes the human who would have noticed.

AGENTS.md section 7 makes this a rule violation rather than an untidiness: doc
claims are claims about code, and both documents were "checked" at S027's
completion.

## Shape

One edit, four corrections, each re-derived from the code rather than from the
finding: 827, the tempo group in the list, the tempo weights in the paste
target, four occurrences of the swallowing bug.

One more came out of re-deriving the rest of the section, the same class of
defect one paragraph up: the quiet-position filter was given as two conditions.
`tools/datagen.cpp:254-256` applies four -- no check, the chosen move neither a
capture nor a promotion, no mate found, and the score inside `--quiet-limit` --
and the opening discard at `:233` takes a mate as well as `--opening-limit`.

One claim was checked and deliberately left alone. The manual dates passed pawns
to S027; `tools/eval_model.hpp:52`, `tools/tuner.cpp:8`, `:628` and `:669` date
them to S035. S035 is `S035_fastchess_script_repair` and touches nothing in the
evaluation, while `adocs/plan_done/S027_handcrafted_eval_terms.md:2` lists
passed pawns in its goal and `:185-187` describes the group-boundary bug hitting
their twelve weights. On what a step id names, the plan directory is the code's
authority and not the reverse, so the manual was right and the four comments are
wrong. They are in `tools/`, which `excludes:` puts out of reach.
author:    Maksym Bodnar
