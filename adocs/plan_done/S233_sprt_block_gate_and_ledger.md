id:         S233
goal:       the gate checks the SPRT result block DEC-220 puts in every verdict-closing commit, and a ledger script regenerates the plan's ledger table from the commit log
accepts:    `tools/gate.sh` reads the commit message it already reads and, when an `SPRT |` line is present, requires all six lines of DEC-220's block -- `SPRT`, `Elo`, `LLR`, `Games`, `Wall`, `Log` -- and checks that the two shas in the `SPRT |` line are the `cand-<sha>` and `ref-<sha>` of the `Results of` line in the log file the `Log |` line names, a missing line, a malformed line or a sha mismatch being a red gate through the script's own `fail`; a commit without an `SPRT |` line is unaffected, proved by re-running the gate over the last twenty `src/`-touching commits; a new script tools/ledger.py reads every commit carrying the block plus a seed file adocs/data/ledger_seed.tsv holding the twenty verdicts taken before DEC-220 (copied once from `plan.md`'s ledger table, never rewritten), prints the ledger table and the four figures `plan.md` "What this costs" carries -- mean, median, fast-class and slow-class means -- and its output over the seed alone reproduces the table as it stands to the digit; `plan.md`'s eight hand-written running-total paragraphs are replaced by that output and a sentence saying where it comes from; DEV_MANUAL.md gains the block's format, the gate's new clause and the script's command, each traced to the code path (DOCS); a fast-suite case runs the script over the seed file and asserts the four figures, named as goldens with the script that re-derives them (DEC-142); the fast suite green in both builds and `./clang-format.sh --check` clean
touches:    tools/gate.sh, tools/ledger.py, tests/, tests/CMakeLists.txt, adocs/data/ledger_seed.tsv, adocs/data/README.md, DEV_MANUAL.md, adocs/plan.md
excludes:   any change to `fastchess.sh`'s output or markers; rewriting any commit; the block's format, which is DEC-220's and is changed only by a decision; any `src/` change
decisions:  DEC-220, DEC-140, DEC-142, DEC-136
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-19 18:46
done:       2026-09-20 03:28 -- `tools/gate.sh` refuses, through its own `fail`, a message carrying an `SPRT |` line whose DEC-220 block is missing a line, repeats one, has one out of shape, names an absent or untracked log, or whose two shas are not the named log's `Results of cand-<sha> vs ref-<sha>` (cases 11 to 17 of `tests/test_gate_script.sh`, 31 failures against the pre-S233 gate and none in cases 1 to 10); a message without the line takes the old path, proved by replaying the last twenty `src/`-touching commits' real messages through the gate in the stubbed sandbox with identical transcripts under both gates; `tools/ledger.py` reads every block in `git log` plus `adocs/data/ledger_seed.tsv` (twenty rows, seeded once) and its `--table` over the seed is byte-identical to `plan.md`'s table with the four figures digit for digit (mean 4 h 27 m, median 4 h 18 m, 199259 games in 89.20 hours, 2233.9 an hour); `plan.md`'s running-total paragraphs are replaced by that output and "Priced by class" covers twenty rows (1 h 58 m / 6 h 56 m) under the interval class rule recorded as DEC-223, with the coordinator re-deriving the pending-count arithmetic for DEC-222's seven added verdicts (325 to 394 machine-hours, floor 218 to 263) and the stale slow-class sentence; DEV_MANUAL.md carries the block's format, the gate's clause, the `CAND=<sha>` requirement and the script's command, each traced; `test_ledger` (19 cases) names its goldens with the re-deriving command (DEC-142). Fast check: trivial fixes, all applied. Gate at completion, `CLANG_FORMAT_MAJOR=22`: 40 of 40 in `build`, 40 of 40 in `build-tune`, `clang-format.sh --check` clean (`.tuning/coord/gate_2026-09-20_s233_s232.log`), run once the lane had freed the machine. No `src/`, no signature owed; MANUAL.md checked, no change.
## Why this exists

DEC-220 (2026-09-19, by the owner) adopts the practice the 2026-09-19 study
read out of an open-source commit log: the harness result block travels in
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

## What was built, and why each choice

**The gate clause, `tools/gate.sh`.** The block check sits immediately after
`message` is assigned and before `bench_lines` counts the `Bench:` lines, which
is where the message first exists in both modes. That placement was chosen over
moving it ahead of the suite: the order of everything else stays exactly as it
was, the `--message` mode's unstaged-tree refusal keeps its position, and the
whole diff to the script is one block plus a header paragraph. The cost is
stated in the script: a typo in a block is reported when the build finishes
rather than before it starts. The trigger is a line beginning `SPRT |`; with
one present all six lines are owed, **exactly one of each, in any order**. The
order is DEC-220's prose and nothing downstream reads it -- `tools/ledger.py`
parses by line name -- while a repeated line would make "the block's shas"
stop meaning one thing, so a repeat is refused. Each line is matched by an
anchored extended regular expression in DEC-220's shape; the six are the same
patterns `BLOCK_RE` carries in `tools/ledger.py`. The `Log |` path is then
resolved against the repository root, required to exist and to be tracked
(`git ls-files --error-unmatch`), and required to carry a `Results of
cand-<sha> vs ref-<sha>` line whose two shas equal the `SPRT |` line's. Every
refusal goes through the script's own `fail`, names what is missing or which
sha disagrees, and on success one line is printed -- `SPRT block checked: cand
<sha> vs ref <sha> against <log>` -- before the signature half runs. No bash-4
form is used; case 9 of the smoke test covers that and still passes.

**The script, `tools/ledger.py`.** Python 3 stdlib, executable, header stating
what it is and every rule it applies. It reads `adocs/data/ledger_seed.tsv`
and then every commit whose message carries an `SPRT |` line, oldest first,
and prints the markdown table in the ledger's own column order followed by one
paragraph carrying the count, mean, median, total games, total hours, games an
hour and the two class means -- so the eight running-total paragraphs are
fully replaceable by its output and nothing in that section is typed.
`--table`, `--figures`, `--no-git` and `--audit-seed-class` are the four
modes. **A block carries shas and not step ids**, so the run and the
description come from the commit subject: the first `S<nnn>` plus any ` v<n>`,
` leg <n>` or ` F<nn>` immediately after it is the run, and what remains once
a leading `Record `, the run phrase, the possessive, a verdict word and a
leading `for ` are stripped is the description. A subject with no `S<nnn>` is
a hard error naming the sha, as is any malformed, missing or repeated block
line: the script refuses to print rather than drop a verdict.

**The class rule, as coded.** `classify` compares the `Elo |` line's **nElo**
estimate and interval against the `SPRT |` line's bounds pair -- both are nElo
under `model=normalized`, so comparing the Elo estimate against them would be
comparing two different scales. Disjoint is fast class, overlapping is slow.
That reading is what makes `plan.md`'s own words operative: "on a bound" and
"a true zero" are statements about the truth, which is known only through the
interval, so an estimate whose interval reaches the pair has not been shown to
be outside it. The alternative -- the point estimate alone against the bounds
-- was rejected on the evidence: it reproduces six of the eleven hand-assigned
classes where the interval rule reproduces nine, and it would put S210 F22
(13 h 17 m), S098 v1 (5 h 26 m), its leg 2 (6 h 11 m) and S098 v3 leg 1
(6 h 49 m) in the fast class, which is the class used to price a *short* run.

**The seed, `adocs/data/ledger_seed.tsv`.** Twenty rows, one per row of
`plan.md`'s table on 2026-09-19, extracted from the file rather than retyped.
Ten columns: the six table cells verbatim (minus the backticks around
`bounds`, which the printer puts back), the Elo the verdict cell carries where
it carries one, the nElo estimate and interval read from the step file the
`source` column names, the class, and the source. Its header says it is never
rewritten and records the two rows where the coded rule and the recorded class
differ. Its row count is a golden.

## The tests, and what each is for

- **`tests/test_ledger.py`**, new, registered as `test_ledger` in
  `tests/CMakeLists.txt` with the `fast` label, the pattern `test_spsa_driver`
  uses. 19 cases, 0.05 s, stdlib, no build and no git call. `LedgerFigures`
  holds the row count and the four figures as goldens, each named as one at
  its site with the command that re-derives it (DEC-142), taken with
  `--no-git` so they do not move as commits land. `ClassRule` pins the rule at
  its boundary -- an interval that reaches a bound is slow, one that clears it
  by 0.01 is fast. `BlockParser` covers the whole block parsing to its fields,
  a `none` outcome printing as no verdict, and the five refusals the parser
  owes: a missing line, a malformed line (one per line name), a repeated line
  and a subject with no step id. `SubjectRule` pins the five subject shapes.
  `WallParsing` pins every wall-time shape the seed carries and the
  truncation convention.
- **`tests/test_gate_script.sh`**, extended with cases 11 to 17 in the same
  sandbox shape, header case list extended: a whole block over a tracked log
  passes and says so; each of the six lines missing in turn is refused by
  name; a malformed `LLR` line is refused as out of shape; a repeated `Games`
  line is refused; a block whose shas are not the log's is refused naming
  both; an untracked log and then an absent one are each refused; and the same
  commit with the block removed takes exactly the path it took before, with no
  block check performed. Case 11 deliberately touches no `src/`, which is the
  shape a verdict-closing commit has, and so also shows the block check runs
  before the early exit the no-`src/` path takes.

## Verification run, with its output

The machine is held by S231's SPSA lane, so nothing was built and no gate,
`ctest`, `cmake`, `fastchess.sh`, `gate_extra.sh` or `mutation_check.py` was
run. What was run costs seconds of one core.

- `bash tests/test_gate_script.sh tools/gate.sh` -> `ok`, exit 0.
- The same test against the gate as it stands at `0c3f0eb` -> **31 failures**,
  every one of them in cases 11 to 16 and none in cases 1 to 10 or 17. That is
  the non-vacuity check: the new cases fail without the new clause, and the
  old cases and the no-block case pass either way.
- `python3 tests/test_ledger.py` -> `Ran 19 tests`, `OK`.
- `python3 tools/ledger.py --table` against `plan.md`'s twenty-row table as it
  stood at `0c3f0eb`: **byte-identical**, `diff` silent.
- `python3 tools/ledger.py --figures` against `plan.md`'s last running-total
  paragraph: mean `4 h 27 m`, median `4 h 18 m`, `199259 games in 89.20
  hours`, `2233.9 an hour` -- all four reproduce digit for digit. The precise
  values are mean 4 h 27 m 36 s and median 4 h 18 m 34 s.
- `python3 tools/ledger.py --audit-seed-class` -> eighteen agree, S149 and
  S207 disagree; the two disagreements are recorded in the seed file's
  own header and in `plan.md`, so the reading survives without the log.
- **The unaffected path, the stronger proof the accepts asks for.** The real
  commit message of each of the last twenty `src/`-touching commits was
  replayed through the gate in the sandbox -- stubs for `cmake`, `ctest` and
  `clang-format.sh`, a stub binary set to that message's own claimed
  signature, the `No functional change` ones against the ancestry the replay
  builds -- once with the gate at `0c3f0eb` and once with the gate as it now
  stands. **All twenty are `GATE-DONE` in both runs and the two transcripts
  are identical.** None of the twenty messages carries an `SPRT |` line, so
  the new clause is never entered; the replay is what shows that rather than
  asserts it.
- `python3 tools/plan_prose_check.py --citations --touches` over the pending
  step files: clean.

## Discrepancies found, and what was not changed

1. **`plan.md`'s two class means were rounded where its mean and median were
   truncated.** The eleven-row fast mean stood at `2 h 31 m` from 2 h 30 m 55 s
   and the slow mean at `6 h 17 m` from 6 h 16 m 55 s, both rounded up; the
   mean `4 h 27 m` from 4 h 27 m 36 s and the median `4 h 18 m` from
   4 h 18 m 34 s are truncated. The script truncates throughout and says so;
   both eleven-row figures are superseded by the twenty-row ones anyway. No
   figure was fudged to match.
2. **No mechanical rule reproduces `plan.md`'s eleven hand-assigned classes.**
   The interval rule gets nine of eleven and the point-estimate rule six. The
   two the interval rule misses are S149 (-14.21 +/- 13.56 against `{-5, 5}`,
   interval reaching -0.65) and S207 (+4.47 +/- 6.72 against `{-5, 0}`,
   interval reaching -2.25, which `plan.md`'s own prose already calls "close
   to the near bound"). Both keep the class they were given, so no published
   figure moves; the seed's header, `plan.md` and `--audit-seed-class` all
   record it. **Whether the rule or the hand is right is a DEC-071 question
   for the owner**, not something this step decided.
3. **A stale slow-class mean two subsections below, found here and fixed by
   the coordinator.** "### What the formula says before a run, DEC-143" said
   "The slow-class mean of 6 h 17 m is about 14680 games at the ledger's
   2337"; at 6 h 56 m 20 s the games equivalent is 16216, still below the
   25591 on-a-bound case the sentence compares it to, so only the figures
   were wrong and the conclusion held. It sat outside the edits this step was
   briefed for and was reported rather than changed; **the coordinator
   re-derived it on 2026-09-19 after that report** and it now reads "6 h 56 m
   is about 16200 games at the ledger's 2337".
4. **`plan.md`'s "Two of eleven landed inside the struck 45-to-75-minute
   window" did not survive.** Over twenty rows four runs came in under 75
   minutes and three of those four were H1 on a gainer, so the old sentence's
   companion claim -- "neither was a strength verdict" -- is no longer true.
   The sentence was replaced with the derived form rather than carried over.
5. **The pending counts in the cost arithmetic disagreed with the section's
   own opening, and the coordinator reconciled them.** This step re-derived
   the two class means and their products only, as briefed, which left the
   block reading `3 fast-class` and `42 to 52 slow-class` against the opening
   paragraph's "roughly 49 to 59 SPRT verdicts" (DEC-222 raised the count and
   the class arithmetic had not followed). **The coordinator carried it on
   2026-09-19 after this step's report**: the slow count is now 46 to 56, so
   the block reads 5.9 h + 319 to 388 h = **325 to 394 machine-hours** and the
   flat-mean floor 218 to 263 hours (4 h 27 m x 49 to 59).

## What is owed

- **The Tier-1 gate, by the coordinator, after the SPSA lane ends.** Nothing
  in this step was built or ctest-ed. The exact chain, in order, is in the
  report.
- Nothing else. The completing commit touches no file under `src/`, so
  **neither `Bench:` nor `No functional change` is owed** on it, and it closes
  no SPRT verdict, so it carries no result block of its own -- the first real
  block is S231's, which this step lands in front of.
- `MANUAL.md` checked and needs no change: it documents the UCI surface, and
  this step adds no option, no default, no command and no engine output. Its
  one mention of `tools/gate.sh` is about the `bench` signature and is
  unaffected. `README.md` is the owner's and was not touched.

## Proposed `status.md` paragraph, for the coordinator

S233 done pending the gate: `tools/gate.sh` now checks DEC-220's six-line
result block whenever a message carries an `SPRT |` line -- every line in
shape, one of each, and the block's two shas equal to the `Results of` line of
the tracked log the `Log |` line names -- and `tools/ledger.py` regenerates
`plan.md`'s ledger table and its figures from `git log` plus
`adocs/data/ledger_seed.tsv`, the twenty pre-DEC-220 verdicts seeded once and
never rewritten. Its output over the seed reproduces the table byte for byte
and all four figures digit for digit (mean 4 h 27 m, median 4 h 18 m, 199259
games in 89.20 hours, 2233.9 an hour); the eight running-total paragraphs are
gone and "Priced by class" now covers twenty rows at 1 h 58 m and 6 h 56 m,
which with DEC-222's raised verdict count moves the projection to 325 to 394
machine-hours. `test_ledger` is new
in the fast suite and `test_gate_script` gained cases 11 to 17. **Three things
for the owner or the coordinator**: the coded class rule disagrees with the
hand classification on S149 and S207 (both kept as recorded, a DEC-071
question); the stale "slow-class mean of 6 h 17 m" sentence under "What the
formula says" and the cost block's pending counts were both re-derived by the
coordinator after the step's report; and the Tier-1 gate has not run -- the
machine was held by S231's lane throughout.
