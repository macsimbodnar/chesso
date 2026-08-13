id:         S040
goal:       re-derive the DEV_MANUAL tuner section from tools/tuner.cpp and eval_model.hpp
accepts:    every number and name in the tuner section is traced to the code that produces it; the paste target lists every weight array a fit writes, tempo included
touches:    DEV_MANUAL.md tuner section
excludes:   the tuner's behaviour -- nothing in tools/ changes here
decisions:  DEC-041
closes:     2026-08-13_adversarial-F06
blocks:
paused_by:
done:

## Four claims that no longer match the code

- `DEV_MANUAL.md:346` says the tuner fits 825 numbers. `eval_model::PARAM_COUNT`
  is **827**, and the breakdown at `:347-349` sums to 825 because it omits
  tempo's 2.
- `DEV_MANUAL.md:368-370` lists the groups and stops before `tempo`.
  `GROUP_LIST` at `tools/tuner.cpp:140-142` carries it and `free_mask`
  implements it at `:198`. A reader following the manual does not know
  `--only tempo` exists.
- `DEV_MANUAL.md:357-359` names the paste target as "the mobility, king safety,
  passed pawn, pawn structure and piece placement weights". The tempo weights
  (`src/evaluation.cpp:580-581`) are missing from that list.
- `DEV_MANUAL.md:373-378` says the group-swallowing bug "has now happened three
  times" and gives three examples. `tools/tuner.cpp:194-197` records a fourth.

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
