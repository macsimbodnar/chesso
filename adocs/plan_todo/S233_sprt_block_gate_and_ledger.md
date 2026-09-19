id:         S233
goal:       the gate checks the SPRT result block DEC-220 puts in every verdict-closing commit, and a ledger script regenerates the plan's ledger table from the commit log
accepts:    `tools/gate.sh` reads the commit message it already reads and, when an `SPRT |` line is present, requires all six lines of DEC-220's block -- `SPRT`, `Elo`, `LLR`, `Games`, `Wall`, `Log` -- and checks that the two shas in the `SPRT |` line are the `cand-<sha>` and `ref-<sha>` of the `Results of` line in the log file the `Log |` line names, a missing line, a malformed line or a sha mismatch being a red gate through the script's own `fail`; a commit without an `SPRT |` line is unaffected, proved by re-running the gate over the last twenty `src/`-touching commits; a new script tools/ledger.py reads every commit carrying the block plus a seed file adocs/data/ledger_seed.tsv holding the twenty verdicts taken before DEC-220 (copied once from `plan.md`'s ledger table, never rewritten), prints the ledger table and the four figures `plan.md` "What this costs" carries -- mean, median, fast-class and slow-class means -- and its output over the seed alone reproduces the table as it stands to the digit; `plan.md`'s eight hand-written running-total paragraphs are replaced by that output and a sentence saying where it comes from; DEV_MANUAL.md gains the block's format, the gate's new clause and the script's command, each traced to the code path (DOCS); a fast-suite case runs the script over the seed file and asserts the four figures, named as goldens with the script that re-derives them (DEC-142); the fast suite green in both builds and `./clang-format.sh --check` clean
touches:    tools/gate.sh, tools/ledger.py, tests/, tests/CMakeLists.txt, adocs/data/ledger_seed.tsv, adocs/data/README.md, DEV_MANUAL.md, adocs/plan.md
excludes:   any change to `fastchess.sh`'s output or markers; rewriting any commit; the block's format, which is DEC-220's and is changed only by a decision; any `src/` change
decisions:  DEC-220, DEC-140, DEC-142, DEC-136
closes:
blocks:
paused_by:
done:

## Why this exists

DEC-220 (2026-09-19, by the owner) adopts the practice the 2026-09-19 study
read out of a strong engine's commit log: the harness result block travels in
the commit, so `git log` is the measurement ledger. Chesso cannot put it in the
landing commit -- the SPRT's candidate is a committed sha and the verdict comes
hours later, and history is never amended -- so it goes in the "Record" commit
that closes a verdict, every verdict, H0 and no-verdict included. Chesso
already has the other half: `tools/gate.sh` checks the `Bench:` line (DEC-140),
reading the message through `bench_lines` and refusing through `fail`. This
step adds the block's check beside it and the script that turns the log back
into the table `plan.md` maintains by hand today -- eight "With X the ledger
holds N" paragraphs whose means and medians are re-typed at every verdict,
the class of number the 2026-09-04 plan review found stale (its F03).

## Shape

- **The gate.** `tools/gate.sh` already holds the message in a variable and
  counts `Bench:` lines with `bench_lines`; the block check is a second
  count over the six line prefixes, each matched by a tight regular
  expression in the shape DEC-220 states, and one `grep` over the named log
  for `Results of cand-<sha> vs ref-<sha>`. The log path is relative to the
  repository root and must exist and be tracked.
- **The script.** Reads `git log --format=%B` for messages carrying `SPRT |`,
  parses the six lines, appends the seed rows, and prints the table in the
  ledger's column order (run, what it measured, wall, games, bounds, verdict)
  followed by the four figures. The class split is the ledger's own rule --
  an effect outside the interval is fast class, inside or a true zero is slow
  -- read from the `LLR` line's verdict and the `Elo` line's interval, and
  the script states the rule in its header.
- **The seed file.** Twenty rows, one per verdict in `plan.md`'s table on
  2026-09-19, with a `source` column naming the step file each was read from.
  It is written once and its row count is a golden.

## Cost

An afternoon of agent time, no machine. The first Record commit under DEC-220
is S231's SPRT, so this step lands before that verdict closes.
