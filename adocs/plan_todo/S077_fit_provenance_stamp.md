id:         S077
goal:       an emitted table names the engine commit and the corpus hash it was fitted from
accepts:    the header `tools/tuner` writes carries the engine commit it was built from, a content hash of the corpus file and that file's row count, alongside the K, seed, learning rate and split it already records; the hash is reproducible -- hashing the same file twice gives the same value, and it is stated which hash and over what; a fit run twice on the same corpus and seed emits identical headers apart from nothing; nothing under `src/` changes, so no verdict is owed
touches:    tools/tuner.cpp, DEV_MANUAL.md
excludes:   tracking the corpus itself, which `c56ab41` decided against for a 715 MB file in a permanent history; changing what is fitted; `tools/datagen`'s output format
decisions:  DEC-041
closes:
blocks:
paused_by:
done:

## Why this exists

`adocs/eval_tuning_strategy.md` section 10: "Version every weight vector
alongside the git commit of the engine and the dataset hash that produced it.
Untraceable weights are unreproducible results."

The header the tuner writes today records the data **path**, K, the train and
validation error, the seed, the learning rate, the split and the group
selection. A path is not a corpus: `.tuning/selfplay_v2.tsv` has already been a
different file once, `selfplay_v1.tsv` did not survive DEC-049, and both S082 and
S083 will write a third and a fourth under names a header cannot distinguish by
path alone.

The engine commit matters for the same reason in the other direction. What the
tuner fits is a model of `evaluate()`, and `tools/eval_model.hpp` tracks the
engine by hand with a guard test and a tolerance (S038, DEC-053). A table emitted
against one commit and applied to another is exactly the failure that guard
exists to catch, and the header currently does not say which commit it was.

## What this is worth and what it is not

It buys nothing in Elo and it is not measured by a match. It buys the ability to
answer "which corpus produced the constants that ship" without inference, which
is the question `2026-08-16_plan_review-F01` and `-F04` both turned out to be --
figures quoted from a run nobody could re-identify.

## Cost

Minutes. No fit, no match. The hash costs one pass over 715 MB at fit time,
which is seconds against an epoch.
