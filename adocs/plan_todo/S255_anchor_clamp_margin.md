id:         S255
goal:       `adocs/data/S192_anchors.py` clamps stage two at `LAZY_EVAL_MARGIN` as the engine does, read from the source rather than written as 150
accepts:    the script's clamp equals `src/search_params.hpp`'s `LAZY_EVAL_MARGIN` (184 today) by reading it, and the anchors it re-derives are unchanged on today's tree (stated, with the run)
touches:    adocs/data/S192_anchors.py
excludes:   any engine or golden change
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
done:

## Why this exists (2026-10-05, the coordinator, from S055's report)

The script that re-derives the stand-pat anchors clamps at 150 while the
engine clamps at 184. It binds on no case today, so every golden it produced
is right, but a re-derivation after S039 or S122 moves the margin or the sums
could disagree with the engine and pin a wrong golden. A tooling defect
reaching no play: a filler (DEC-171).
