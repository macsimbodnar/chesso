id:         S330
goal:       the recipe that rebuilds `selfplay_v2_dedup.tsv` from the archived corpus names the tree that rebuilds it byte for byte
accepts:    both digests re-derived on `.tuning/selfplay_v2.tsv` read in place (sha256 `ad8c5dfe...`): `corpus_dedupe` built from a tree before S042 (`a3e84e1`, by `git archive`) writes the `0a6b59b6...` DEC-183 names, and one built at `HEAD` writes `cdb43ba2...` (10795203 rows, S327's run), each run stated; the repository's statement of the recipe (where `DEV_MANUAL.md` and the dedupe step's records give it) names the tree and both digests, history kept (DEC-348); the archive in the owner's Synckeeper folder (`.moltke.local.md` names it) is not rewritten -- a corrected `CORPUS_README.md` goes beside it as its own file with a `.sha256` sidecar, built under `.tuning` and moved in (the Synckeeper rule), or the step says why not; a decision drafted for the coordinator if the recipe's rule changes, for example which digest later fits name
touches:    DEV_MANUAL.md, adocs/data/S330_*, adocs/data/README.md, tools/corpus_dedupe.cpp
excludes:   regenerating or replacing `.tuning/selfplay_v2_dedup.tsv`; any refit; `src/`; `tools/` but for `tools/corpus_dedupe.cpp`'s opening comment
decisions:  DEC-183, DEC-171, DEC-348
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- `accepts` by clause. Both digests re-derived over `.tuning/selfplay_v2.tsv` read in place (`ad8c5dfe…`) by `adocs/data/S330_dedup_digests.sh`: `corpus_dedupe` from `git archive a3e84e1` writes 10795695 rows, `0a6b59b6…`, DEC-183's; from the base, `1bb3080` with S324 to S329 staged, 10795203 rows, `cdb43ba2…`, which S327's base `7bb327d4` writes too (S327's table), its `corpus_dedupe.cpp.o` `1bb3080`'s by `S325_identity.txt`; a third build at `a3e84e1` by the recipe as written, without doctest, the same binary and `0a6b59b6…`. The script's two early exits print `S330-DIGESTS-FAILED` since the fast check, the success path and data file unchanged (`cmp`), a planted corpus read red. `DEV_MANUAL.md` names the tree, both digests and the recipe after S076's command, history kept (DEC-348), nothing S327 or S329 wrote changed; S076's own records are `plan_done/`'s, unedited. The archive not rewritten and nothing written in Synckeeper: a corrected README, `cd6ee195…`, with a bare-name sidecar, and the archive's sidecar corrected to name its file bare, `b48ac393…`, the old one's bytes recorded below, both under `.tuning/coord/S330/archive/` for the coordinator to place at landing. No decision drafted, the recipe's rule unchanged; DEC-183 left as it is. The three findings first delivered unfixed taken in at the coordinator's word, `touches:` widened for the third: `tools/corpus_dedupe.cpp`'s opening comment, its 154 lines unmoved, comments only by S321's form, `corpus_dedupe.cpp.o` byte-equal with ccache off in both builds and the binary `70126551…` before and after (`adocs/data/S330_identity.txt`). `MANUAL.md` checked, unchanged. Run here, `CLANG_FORMAT_MAJOR=22`: the document entries and `test_corpus_dedupe` green in both builds, the prose checks and format rc 0. Not run here: the full fast suites; the commit needs both green at the coordinator's gate with the machine quiet. No `src/` change, bench unmoved. Filler (DEC-171).

## Why

Found 2026-11-03 by S327's implementer. DEC-183's archive records the `corpus_dedupe` command that rebuilds the derived
dedup file `0a6b59b6...`, which twelve `plan_done/` records name. Since S042 the key drops an en passant square no pawn
can take, so a current binary writes `cdb43ba2...` with 10795203 rows from the same input. The local file still hashes
`0a6b59b6...`, but a machine restoring from the archive with a current binary gets a different corpus from the one the
records name. Filler by DEC-171: no fit moves until someone refits on a regenerated file.

## 2026-11-03: the recipe names `a3e84e1` (Opus 5.5 subagent, briefed by the coordinator)

**Both digests, re-derived.** `adocs/data/S330_dedup_digests.sh`, run from the worktree as
`bash adocs/data/S330_dedup_digests.sh <corpus> <scratch> <doctest> 84e66058519e566466a26774eb71b89c6e15c67c`, with the
main tree's `.tuning/selfplay_v2.tsv` and its `tests/doctest` checkout by absolute path. It checks the corpus first,
11003693 rows at `ad8c5dfe…`, read in place. Then three builds, each `git archive` of a tree into a new scratch folder,
`cmake -DCMAKE_BUILD_TYPE=Release`, `cmake --build -j12 --target corpus_dedupe`, and one pass each,
`corpus_dedupe --in <corpus> --out /dev/stdout`, the kept rows counted and hashed as they stream:

| build | tree | binary sha256 | rows written | dropped | kept rows sha256 |
|---|---|---|---|---|---|
| `a3e84e1`, doctest supplied, ccache | `eeed8d7c` | `1190fa03…` | 10795695 | 207998, 1.8903 % | `0a6b59b6…` |
| the base, doctest supplied, ccache | `84e66058` | `70126551…` | 10795203 | 208490, 1.8947 % | `cdb43ba2…` |
| `a3e84e1` by the recipe: no doctest, no ccache | `eeed8d7c` | `1190fa03…` | 10795695 | 207998, 1.8903 % | `0a6b59b6…` |

The newline count off stdout equals the tool's own "distinct positions written" in each run. The repeat histograms are
S076's for `a3e84e1` and S327's for the base. Each pass took about 31 s and 437 MB resident, the one-minute load 9 to 12
on 12 threads from S122's screen: one run each, not timings. The two binary digests are the ones S327 recorded for
`a3e84e1` and for S327. The base is not the `HEAD` the `accepts:` names: it is `1bb3080` with S324 to S329 staged.
S327's table shows S327's base, tree `7bb327d4`, writing the same `cdb43ba2…`. That base's `corpus_dedupe.cpp.o` is
`1bb3080`'s: byte-equal by `S325_identity.txt`, S325's comment edit against `1bb3080`, and S324 and S326 change no file
under `src/` or `tools/`. The data file is the third of three runs. The first two ran before an echoed line lost a
trailing space and before the header named the recipe build's two differences, its folder and its `--out`. All three
agree on every digest and count. At the coordinator's fast check the script's two early exits, the usage line and the
refusal of a corpus that is not `ad8c5dfe…`, gained an `S330-DIGESTS-FAILED: <reason>` line: an `exit` does not fire the
ERR trap, so they had ended without the marker. The success path is unchanged, so the passes were not re-run: `bash -n`
rc 0, the data file byte-equal by `cmp` to the third run's output, a planted one-row corpus prints
`S330-DIGESTS-FAILED: corpus digest` and exits 1 before any build folder is made, and no arguments print
`S330-DIGESTS-FAILED: usage` and exit 2.

**The repository side.** `DEV_MANUAL.md`, "Deduplicate a corpus by position": one paragraph, one command block and one
paragraph added after S076's command, and nothing else changed. They say three things. A tool built at S042 or later
writes `cdb43ba2…` from the archived TSV, and since it truncates `--out`, S076's command run with it replaces the
recorded `0a6b59b6…` file. The tool at `a3e84e1` rebuilds that file byte for byte, by the five commands given, which
need no doctest. The archive's README gave S076's command, true on the day it was written. S076's command block, its
measured paragraph as S329 dated it, with S329's sentence on what a current tool drops, and S327's paragraph on
`--verify` are unedited (DEC-348): S076's command is still how a corpus is deduped, and it was the recipe when
written. The dedupe step's own records, S076's step file and stamp, are in `plan_done/` and are history: not edited,
and they held at S076's tree, where S076 ran. `MANUAL.md` checked: its one mention of the deduplicated corpus is
S076's refit, in its history of the constants, and gives no recipe. Unchanged.

**The archive side, prepared and not placed.** The archive is
`/home/max/Synckeeper/Chesso Archive/chesso_selfplay_v2_corpus_2026-09-11.tar.xz` (`.moltke.local.md`). It was read and
not written: it hashes `d91bd9b2…` today, DEC-183's digest, and its `CORPUS_README.md`, read by `tar -xOJf`, is
`ba41fbc2…`. Two files are prepared under `.tuning/coord/S330/archive/`, nothing in the Synckeeper folder was written,
and its two files still carry their 2026-09-11 times:

- `chesso_selfplay_v2_corpus_2026-09-11.CORPUS_README.2026-11-03.md`, sha256 `cd6ee195…`, with a sidecar in the
  bare-filename form, `….md.sha256`. Its name sits beside the archive and replaces nothing.
- `chesso_selfplay_v2_corpus_2026-09-11.tar.xz.sha256`, sha256 `b48ac393…`, the archive's sidecar corrected (finding 1
  below). Placed, it replaces the old one.

The README differs from the archived one in four places. A dated note under the title says what changed and why. The
dedup paragraph now builds the tool at `a3e84e1` by the same five commands as `DEV_MANUAL.md`, and gives about 440 MB
resident where it gave 1.9 GB. That figure was the `--verify` pass's: the archive's own `dedupe_v2.log` times 1901316 kB
for the `--verify` run, the command has no `--verify`, and the three runs here read 437 MB. A new paragraph gives
`cdb43ba2…` for a tool at S042 or later. The last paragraph gains one dated clause on where the archive sits (finding 2
below). The rest is byte-equal (`diff`). Neither file is moved in: by the brief, the coordinator places them at landing,
by `.moltke.local.md`'s rule, `mv` and a re-check from inside the folder. I recommend that the README go there. The
archive is what a restore on another machine reads first, and the README inside it gives a command that, with any tool
built since S042, writes a file no record names.

**No decision drafted.** The recipe's rule is unchanged: the archive's recipe rebuilds the file the records name,
`0a6b59b6…`, and now says from which tree. No record's digest changes, and `.tuning/selfplay_v2_dedup.tsv` was not
touched. A fit on a regenerated `cdb43ba2…` file would be on a new corpus, which DEC-183's Consequences already give
the archive-and-recipe treatment.

**Found, and taken in at the coordinator's word.** Delivered first as findings, not fixed; the coordinator asked for
all three to be taken in, and widened this step for the third.

1. **The archive's sidecar does not verify with `-c`.** It names
   `/home/max/Synckeeper/chesso_selfplay_v2_corpus_2026-09-11.tar.xz`, an absolute path from before `Chesso Archive/`
   existed. From inside the folder `sha256sum -c` exits 1 today, "FAILED open or read". The digest in it is right: the
   archive hashes `d91bd9b2…`. `.moltke.local.md` records this; the folder's other 44 sidecars name their file bare.
   **Taken in:** the archive hashed again in place, `d91bd9b2…`, equal to DEC-183's and to the old sidecar's digest, and
   the corrected sidecar written under `.tuning/coord/S330/archive/` naming the file bare, 110 bytes. Run from inside
   the folder against the archive in place, `sha256sum -c` on it prints "OK" and exits 0. The old sidecar, so that the
   replacement is reversible, is 131 bytes, sha256 `d572a7f1de25880a1c034a06fabc43d4d57716739e86bd328d19964c636ed7de`,
   one line ending in a newline:

   ```
   d91bd9b2a86939043a8ca744428fed6d4e7c9d7038593d06bd03e86be3c8797d  /home/max/Synckeeper/chesso_selfplay_v2_corpus_2026-09-11.tar.xz
   ```

   `printf '%s  %s\n' <digest> <path>` with those two fields reproduces it byte for byte (`cmp`).
2. **DEC-183 gives the archive's location as `/home/max/Synckeeper/`.** It is in `/home/max/Synckeeper/Chesso Archive/`.
   The README's last paragraph cites DEC-183 for "this archive's name, location and digests". This step's brief named
   the old path too. **Taken in:** that paragraph keeps its sentence and gains a dated clause, "Noted 2026-11-03
   (S330): the archive now sits in `/home/max/Synckeeper/Chesso Archive/`, not in `/home/max/Synckeeper/`, where
   DEC-183 put it." DEC-183 stays as it is: `decisions.md` is append-only, and the move is the coordinator's to record
   if it records it.
3. **The usage comment at the top of `tools/corpus_dedupe.cpp`** gives S076's command,
   `--out .tuning/selfplay_v2_dedup.tsv`. Run with the binary it sits beside, it replaces the recorded file with
   `cdb43ba2…`.
   **Taken in,** `touches:` widened and `excludes:` narrowed to match. Two comment lines follow the command, which
   stays: since S042 it writes `cdb43ba2...` over the `0a6b59b6...` the records name, and `DEV_MANUAL.md`'s
   "Deduplicate a corpus by position" rebuilds that. To keep the file at 154 lines, so that nothing below the comment
   moves, the two paragraphs after it were joined into one, rewrapped with their words unchanged. Comments only, by
   S321's form (`adocs/data/S330_identity.txt`): `g++ -fpreprocessed -dD -E -P` gives the same 3291 bytes before and
   after, `9e413eaa…`, and the control shows the form telling a changed constant apart from added comments. With
   ccache off, `corpus_dedupe.cpp.o` was deleted and recompiled before and after in `build/` and `build-tune/`, each
   recompile read in the build log, and each is byte-equal, `e8347d98…`. The `corpus_dedupe` binary is `70126551…`
   before and after, in both builds, the same as the base's build from `git archive` above. Bench is not run: nothing
   under `src/` changed.

**Checks.** With `CLANG_FORMAT_MAJOR=22`, after the findings were taken in: in `build/` and `build-tune/`, both
configured in the worktree and built for `corpus_dedupe` and `test_corpus_dedupe` only, the six ctest entries that read
documents, `test_clang_format_script` and the five `test_plan_*`, and `test_corpus_dedupe`, 15 test cases and 107
assertions, all green. `tools/plan_prose_check.py`'s five checks exit 0 with nothing flagged, and this file reads 0
citations in the line form. `./clang-format.sh --check` exits 0. Re-run after the fast check's fixes: the prose checks.
The full fast suites were not run here: the step edits `tools/corpus_dedupe.cpp`, so it is not docs-only, and the commit
needs both fast suites green at the coordinator's gate with the machine quiet. Nothing under `src/` changed, so bench
does not move.
