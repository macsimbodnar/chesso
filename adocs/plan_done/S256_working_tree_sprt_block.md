id:         S256
goal:       a verdict measured from the working tree can be closed with DEC-220's block, or fastchess.sh refuses a working-tree candidate for a verdict run
accepts:    one of the two holds, chosen by the step and stated: (a) fastchess.sh names a working-tree candidate in its `Results of` line by a sha the block can carry (HEAD plus a hash of the diff, saved beside the log) and `tools/gate.sh` and `tools/ledger.py` accept it; or (b) a run without `--fast` refuses a dirty tree and DEV_MANUAL.md says to commit the candidate first; either way S055's H0 (adocs/data/S055_sprt.log) reaches the ledger, by a seed row if not by the block
touches:    fastchess.sh, tools/gate.sh, tools/ledger.py, adocs/data/ledger_seed.tsv, DEV_MANUAL.md, MANUAL.md, tests/test_fastchess_script.sh, tests/test_gate_script.sh, tests/test_ledger.py, adocs/data/S024_pair_stats.py, adocs/data/S203_mine_cases.py
excludes:   any engine change
decisions:  DEC-220, DEC-171
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-05. Choice (a). fastchess.sh names a working-tree candidate `cand-<HEAD>` on a clean tree and `cand-<HEAD>+<12 hex>` on a dirty one, the hex the start of `git hash-object` of the `candidate.diff` it saves in the run's output directory; tools/gate.sh and tools/ledger.py accept the token when a file under adocs/data/ in the tree under test (gate) or the commit (ledger) has that blob id. S055's H0 reaches the ledger as a seed row (the token cannot apply to its existing log). Tests observed red first: test_fastchess_script cases 27, 28, 29; test_gate_script 21, 23; test_ledger's four new parser and tree cases. Gate green in both builds, 41/41 fast each, clang-format clean. No engine change, so no Debug self-play is owed (DEC-141 binds make_move, the generator and the search).

## Why this exists (2026-10-05, the coordinator, from S055's verdict)

S055's SPRT ran from the working tree (DEC-253: no budget set was green on
the candidate before its ceiling decision, so it could not be a commit). The
log's result line reads `Results of candidate vs ref-4a7e8ce`, and
`tools/gate.sh` requires `cand-<sha>` there, so the closing commit could not
carry DEC-220's block and `tools/ledger.py` does not see the verdict. A
tooling defect reaching no play: a filler (DEC-171).

## As built (2026-10-05, the implementer)

**Choice (a), and why not (b).** (b) would make every verdict whose candidate
cannot be a green commit before it reads -- DEC-253's case, S207's before it --
either impossible or a commit of a red tree. (a) makes the working-tree run
attributable instead: HEAD plus a content hash of exactly what was played,
saved, which is the attribution DEC-020 asks for. Nothing in (a) proved
unsound; its one limit is stated below.

**The name.** Set once, after the output directory exists, before the banner:

| candidate | engine name, `Results of` line, PGN | block's `cand` |
|---|---|---|
| `CAND=<ref>` | `cand-<sha>` | `<sha>` |
| working tree, clean | `cand-<HEAD>` (was `candidate`) | `<HEAD>` |
| working tree, dirty | `cand-<HEAD>+<12 hex>` (was `candidate`) | `<HEAD>+<12 hex>` |

The hex is the first 12 of `git hash-object --no-filters` of
`$outdir/candidate.diff`, i.e. the diff's git blob id. The banner prints a
`diff <path> blob <hex>` line under the candidate's. The diff is
`git diff-index --cached -p --binary --full-index HEAD` over a scratch copy of
the index after `git add -A`: what committing the tree would commit, untracked
files included (src/CMakeLists.txt globs `*.cpp`, so an untracked source is in
the binary), ignored files not, the real index untouched, and plumbing so no
user diff config can change the bytes. The scratch index is removed at once
and by the EXIT trap. The identity check is unchanged: the binary's sha is
still compared with HEAD for both working-tree forms, `-dirty` allowed only on
a dirty tree.

**Limit.** The hash pins the tree at launch, not the build. The `id name`
check proves the binary was built on HEAD with something uncommitted, not
which something; that was true of the banner before and is no worse now.

**The gate and the ledger.** Both patterns take `cand <sha>(+<7-40 hex>)?`.
The gate escapes the `+` before grepping the log (case 24 is the mutant that
reads it as a quantifier), then requires a file under adocs/data/ whose blob id
starts with the hex: `git ls-files -s` in `--message` mode, `git ls-tree -r
<ref>` otherwise, found by id and not by name, and names it in its `SPRT block
checked` line. `tools/ledger.py` checks the same in each block commit's tree
(`diff_in_commit`) and dies naming the hex if it is absent.

**S055 reaches the ledger by a seed row.** The token cannot apply
retroactively: the log reads `Results of candidate vs ref-4a7e8ce` and a run's
log is evidence. `adocs/data/S055_candidate.diff` is in 636bbf4, but nothing
in the log binds it to the run. The row is appended to
`adocs/data/ledger_seed.tsv` with a header paragraph saying why; the twenty
rows above are untouched. Its class is the rule's: nElo -8.78 +/- 6.39 against
{-5, 0} overlaps, slow. It prints after the twenty seed rows, not among the
commits in date order. That is an addition to a file DEC-220 called "seeded
once"; the coordinator records it.

**Goldens moved, re-derived by their script (DEC-142).** test_ledger's
`LedgerFigures` read `python3 tools/ledger.py --no-git --figures`: rows 20 ->
21, mean 4 h 27 m -> 4 h 30 m, median 4 h 18 m -> 4 h 26 m, slow mean 6 h 56 m
-> 6 h 48 m, totals 199259 games / 89.20 h / 2233.9 -> 210631 / 94.64 /
2225.7, classes (10, 10) -> (10, 11); fast mean unchanged. The old values are
kept in the comment. One test was renamed with its number
(`..._exactly_twenty_rows` -> `..._exactly_its_rows`).

**Tests, observed red before the fix.** test_fastchess_script 27 (dirty tree:
name, diff hashes to it, applies to HEAD and rebuilds tracked and untracked
content, real index untouched) and 28 (clean tree: `cand-<HEAD>`, no diff):
both red, `got []`. test_gate_script 21 (pass with committed diff), 23
(`--message`, staged diff): red; 22 (diff absent) and 24 (`+` literal) were
green before only because the shape refused the token, and each kills its own
mutant now (gate's refusal replaced by `true`: 22 red; escape removed: 24 red).
test_ledger: the token parses and `diff_blob` is set, a commit candidate has
none, and `diff_in_commit` finds and refuses in a throwaway repository: four
errors before. The sandbox's .gitignore gained `.ref-builds`, as the real one
has, so `git add -A` does not sweep its worktrees in as embedded repositories.

**Regenerating plan.md's ledger** (`python3 tools/ledger.py`, whole output
over the table and figures paragraph): one row added, S055 after S098 v3 leg 1;
figures 42 -> 43, mean 6 h 21 m -> 6 h 19 m, 563243 -> 574615 games, 266.82 ->
272.25 h, 2111.0 -> 2110.6 an hour, slow 26 -> 27 runs at mean 9 h 00 m -> 8 h
52 m, fast unchanged.

**Docs.** DEV_MANUAL.md: the "block can only be written for `CAND`" paragraph
is replaced by the three names and how to close a working-tree verdict (commit
the diff beside the log); the gate's check list, the banner example, the CAND
consequences and the fastchess test paragraph follow. MANUAL.md checked: it
describes the engine's own `id name`, which did not move; no change.

## Fast check fixes (2026-10-05, the implementer)

**Dirty now means what the diff captures.** fastchess.sh asked `git diff
--quiet HEAD`, which cannot see an untracked file, so a tree changed only by
an untracked `.cpp` (globbed into the build) read as clean, was named
`cand-<HEAD>` and was refused as an A/A at REF=HEAD. The probe now writes the
`git add -A` tree through the scratch index (`write-tree`) and is dirty when
it is not `HEAD^{tree}`; the saved diff is `git diff-tree -r -p --binary
--full-index HEAD <that tree>`, so the dirty test and the diff are one object.
It drives the name, the banner's `+ uncommitted changes`, the A/A guard and
the `-dirty` allowance alike, and is skipped under CAND (the tree is not
read). Consequence: an untracked, unignored file anywhere makes the tree dirty
and disarms the clean-tree A/A refusal -- intended, since it can be in the
build, and the run is then named and its diff saved. Case 29 (untracked
`.cpp` only: `+<hex>` name, the file in the diff) was observed red first
(exit 1, the A/A refusal). The sandbox now excludes its own harness files in
`.git/info/exclude` (everything at top level but `.gitignore`, `tracked.txt`
and `*.cpp`), or every sandbox would read dirty.

**Readers of the literal `candidate`.** `adocs/data/S024_pair_stats.py`
matched `candidate` exactly, so a `cand-` candidate's wins scored as losses
(observed on a synthetic PGN: score 0.25 where 0.75 is right; a name matching
neither side also passed silently). `adocs/data/S203_mine_cases.py` defaulted
`--engine candidate`. Both now use one rule, `is_candidate(name)`: the literal
`candidate` or the `cand-` prefix. S024 exits naming the game when neither or
both sides match; S203 reads the engines from the log's `Started game` lines
and exits when no single candidate matches or an explicit `--engine` did not
play. Checked on adocs/data/S055_sprt.log (`candidate`), the same log renamed
to `cand-4a7e8ce+0123456789ab` (same root found), and renamed to `chesso-a`
(exits 1).

**Docs.** DEV_MANUAL.md's build-stamp paragraph said `-dirty` used "the same
convention fastchess.sh uses"; it now says fastchess.sh asks more. MANUAL.md's
`-dirty` line drops "the convention the rest of the project uses" for the same
reason.
