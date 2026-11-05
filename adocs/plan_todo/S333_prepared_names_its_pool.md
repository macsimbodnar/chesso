id:         S333
goal:       S122's corpus preparation refuses a pool written before S331, and every `.done` marker names the pool it was prepared from
accepts:    `S122_corpus.sh verify` refuses a one-run pool whose `generate.log` holds no `repetition:` line, as `merge` already refuses a top-up's, the refusal seen on a fixture log; each `prepared/` `.done` marker records the pool's `generate.log` sha256 beside its own file's, and `filter`, `split`, `sample` and `k` refuse a marker whose pool is not the one `POOL` names now, each refusal seen on a planted swap; the regenerated pool's preparation (`.tuning/s122/prepared/`) read by the new guards, or the step says why it is left as it stands; `bash -n` and `S122_corpus.sh smoke` green
touches:    adocs/data/S122_corpus.sh, adocs/data/S333_*, adocs/data/README.md
excludes:   `src/`; `tools/`; re-running the preparation over the 150000-game pool; any fit
decisions:  DEC-171, DEC-296, DEC-349
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-04 by S122's DEC-349 implementer (S122's step file, "What still guards only by the move" and "Findings
beyond the task, not fixed"). The void two-fold pool was kept out of the regenerated preparation only by moving its
directories aside: `verify` takes a pool whose datagen predates S331, and `prepared/`'s markers hold their own file's
sha256, not the pool's. A later corpus built with these scripts has no guard against the same mix. Filler by DEC-171.
