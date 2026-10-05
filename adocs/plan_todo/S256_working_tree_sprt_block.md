id:         S256
goal:       a verdict measured from the working tree can be closed with DEC-220's block, or fastchess.sh refuses a working-tree candidate for a verdict run
accepts:    one of the two holds, chosen by the step and stated: (a) fastchess.sh names a working-tree candidate in its `Results of` line by a sha the block can carry (HEAD plus a hash of the diff, saved beside the log) and `tools/gate.sh` and `tools/ledger.py` accept it; or (b) a run without `--fast` refuses a dirty tree and DEV_MANUAL.md says to commit the candidate first; either way S055's H0 (adocs/data/S055_sprt.log) reaches the ledger, by a seed row if not by the block
touches:    fastchess.sh, tools/gate.sh, tools/ledger.py, adocs/data/ledger_seed.tsv, DEV_MANUAL.md
excludes:   any engine change
decisions:  DEC-220, DEC-171
closes:
blocks:
paused_by:
done:

## Why this exists (2026-10-05, the coordinator, from S055's verdict)

S055's SPRT ran from the working tree (DEC-253: no budget set was green on
the candidate before its ceiling decision, so it could not be a commit). The
log's result line reads `Results of candidate vs ref-4a7e8ce`, and
`tools/gate.sh` requires `cand-<sha>` there, so the closing commit could not
carry DEC-220's block and `tools/ledger.py` does not see the verdict. A
tooling defect reaching no play: a filler (DEC-171).
