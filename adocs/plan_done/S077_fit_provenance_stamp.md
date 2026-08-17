id:         S077
goal:       an emitted table names the engine commit and the corpus hash it was fitted from
accepts:    the header `tools/tuner` writes carries the engine commit it was built from, a content hash of the corpus file and that file's row count, alongside the K, seed, learning rate and split it already records; the hash is reproducible -- hashing the same file twice gives the same value, and it is stated which hash and over what; a fit run twice on the same corpus and seed emits identical headers apart from nothing; nothing under `src/` changes, so no verdict is owed
touches:    tools/tuner.cpp, DEV_MANUAL.md
excludes:   tracking the corpus itself, which `c56ab41` decided against for a 715 MB file in a permanent history; changing what is fitted; `tools/datagen`'s output format
decisions:  DEC-041
closes:
blocks:
paused_by:
done:      An emitted header names the commit the tuner was built from, the corpus SHA-256 and its row and byte counts. sha256sum prints the same digest for both the 200000-row fixture and the 706 MB corpus, 2.70 s against its 1.29 s; two runs at the same seed emit byte-identical files, and after a commit the stamp follows HEAD with no reconfigure. SHA-256 written from FIPS 180-4 and the commit stamped at build time rather than configure time: DEC-066. Nothing under src/, no verdict owed.

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
author:    Maksym Bodnar

## What shipped

Three lines in the emitted header and the same two facts on stderr before the
fit starts, so a run log carries them when the emitted file is lost:

```
// Fitted by tools/tuner from 200000 self-play positions.
// engine     764c3d8, the commit this tuner was built from
// data       .../fit_fixture.tsv
// corpus     sha256 ffdcd801ab59c58390503bfee468a5caeb71a6bb38c012904ec87675382533de
//            200000 rows, 12995744 bytes; `sha256sum` over the same file prints the same
```

`tools/corpus_hash.hpp` is SHA-256 written from FIPS 180-4, streaming, plus
`hash_file()` which returns the digest with the byte and row counts from the
same pass. `cmake/build_info.cmake` writes the commit into a generated header on
every build. Both choices and their rejected alternatives are DEC-066.

## The check that makes the stamp worth carrying

**A tool that shares no code with this one prints the same value.** On the
200000-row fixture, `sha256sum` and the tuner both give
`ffdcd801ab59c58390503bfee468a5caeb71a6bb38c012904ec87675382533de`, and the row
and byte counts match `wc -l` and `wc -c`. On the real 706 MB corpus both give
`0a6b59b6af9f50c4360cac7c87c33250318e712250f2653eb09bcb9f0b64b69f` -- which is
also the digest S076's dedupe log recorded for that file, so the two steps'
evidence now cross-references by content instead of by filename. **2.70 s**
against `sha256sum`'s 1.29 s, which is the price of not using the CPU's SHA
extensions and is under a fifth of one percent of a real fit.

**The published vectors are the load-bearing test, and that is measured.** Two
mutations, one each: dropping `update()`'s partial-block handling fails 4 of the
7 cases and 387 assertions, but leaves the three `hash_file` cases green,
because both sides of their comparison break together. Dropping the message
length from the padding fails **only** the two vector cases, 3 assertions -- the
chunking property and the length sweep still pass, since a wrong hash can be
perfectly self-consistent. Nothing but a value from outside catches that.

**The commit does not go stale, and the proof is two trees.** The script wrote
`d9fb0b2-dirty` for this working tree and `a579f46` for `.ref-builds/a579f46`,
a worktree checked out at that commit with `git status --short` empty. After the
implementation commit, `cmake --build build --target tuner` with **no
reconfigure** moved the stamp to `764c3d8`, and a fit over the same fixture at
the same seed emitted a file differing from the earlier one in exactly one line.

## What this cannot do

The stamp says which commit the tuner was **built** from, not that the tree was
that commit -- `-dirty` is `git diff --quiet` over tracked files, so an untracked
source file that somehow compiled in would not show. It is the convention
`fastchess.sh` and every run script here already use, and the alternative,
`git status --porcelain`, would flag the untracked evidence under `adocs/` and
`.tuning/` on every build.

Tables emitted before this step carry no stamp and are not regenerated:
`.tuning/tuned_v2*.hpp`, `adocs/data/S075_fits/`, `adocs/data/S076_fits/`. They
are evidence, byte for byte what the tool wrote, and their provenance stays in
the step files and run logs beside them.
