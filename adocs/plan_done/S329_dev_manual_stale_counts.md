id:         S329
goal:       six statements found stale, three by S325 and S327 and three by this step's first delivery, say what is true, history kept: five in `DEV_MANUAL.md`, one in `MANUAL.md`
accepts:    "The second fit's constants are the ones in `eval_tables.hpp`" re-read against every commit to `src/eval_tables.hpp` since `33aa3b4` and made true, dated or put in the past tense; the S187 count, "six more were wrong when written", re-read against S187's own list and `adocs/data/S187_symbols.tsv` and made true in `DEV_MANUAL.md`, with `plan_done/S187`'s stamp left unedited and the difference stated beside it; the "Measured on `selfplay_v2.tsv`" paragraph's 207998 dropped, S076's figure, dated as S076's beside what a current `corpus_dedupe` drops (208490 by S327's run, re-read); added at the coordinator's word after the first delivery, the three that say which constants ship, each re-read against `01222f3` and made true, history kept: `DEV_MANUAL.md`'s "S076 refitted the same corpus deduplicated and those constants ship now", its "`QUEEN` reads 716 here", which says whose 716 it is and what the file reads since `01222f3`, and `MANUAL.md`'s score-accuracy entry ("all 827 were refitted again on 2026-08-16 ... (S065)", "these constants"), re-read against S076 and S083 in that manual's register, one or two dated clauses; the scripts behind rows 1 and 2 committed with their outputs as `adocs/data/S329_*`, each with an `adocs/data/README.md` row; each old and new text in this step; `tools/plan_prose_check.py`'s checks and `test_plan_citation_freshness` green
touches:    DEV_MANUAL.md, MANUAL.md, adocs/data/S329_*, adocs/data/README.md
excludes:   `adocs/plan_done/`; shortening a passage or removing history (DEC-348)
decisions:  DEC-171, DEC-348
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- six statements read against their sources and made true, old and new side by side in this file, history kept (DEC-348): five in `DEV_MANUAL.md`, one in `MANUAL.md`. Row 1, the second fit's constants: true from `33aa3b4` until `77d7450`. Of the six commits to `src/eval_tables.hpp` since, three moved a constant: S076's refit, S134's bit-exact fold and S083's refit, whose constants the file holds. S253, S117 and S262 moved none (`S329_eval_tables_consts.py`). The bold sentence is now past tense and dated, and the six commits are added. Row 2, S187's six: the stamp lists eight citations and the generator's first six `HAND_CHOICES` entries cover seven, twelve in all. Read in the tree that wrote each line number, two were wrong, S024's array and write, which S138 re-anchored at `7b54f18` onto other lines. Ten were right, and all twelve had drifted by `124e557` (`S329_s187_lines.sh`). "that were wrong" became "as wrong", and a passage before `DEC-119.` states the difference. `plan_done/S187` is unedited. Row 3, S076's 207998: dated "at S076, 2026-08-17" and put in the past tense, with S327's 208490 dropped, 10795203 written and its histogram beside it. Re-read from `adocs/data/S327_verify.txt`, not re-run. Rows 4 to 6 were the first delivery's findings 1 to 3, taken in at the coordinator's word. Row 4: "those constants ship now" became "shipped from `77d7450`, 2026-08-17, until S083's refit replaced them at `01222f3`, 2026-10-25". Row 5: "`QUEEN` reads 716 here" became the second fit's re-anchored 716, kept by S076, with 983 in the file since `01222f3`, which is 710 plus 273 (DEC-307). Row 6: `MANUAL.md`'s "one more generation now" is dated by S065's paste, 2026-08-15. Its two words that were false when written are corrected, found at the coordinator's fast check and verified: 817 of 827 were refitted, not all, and the fit ran on 2026-08-14 and was pasted on 2026-08-15, with only the verdict on 2026-08-16. One sentence adds S076, S083 and the five term fits on top of S083's. "not re-measured" now covers all of them; no step re-measured S028's figures. The two scripts and their outputs are committed as `adocs/data/S329_*` with README rows. Docs only: `test_plan_*` and `test_clang_format_script` 6 of 6 in both builds, configured but not compiled; `tools/plan_prose_check.py`, all five checks, rc 0; `--citations` 0 flagged over 25 files; `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` rc 0. Two findings on `plan_done/` history recorded below, no action. Filler (DEC-171): no score, move or line moved.

## Why

Found 2026-11-03 by S325's implementer. Six commits since `33aa3b4` changed `eval_tables.hpp`, but the sentence is in
the present tense. S187's stamp says six more citations were wrong when written. Its own list names eight, and three of
them, the ones at line 674 of `src/chesso.cpp`, were right when written (S325, by `adocs/data/S169_recitations.tsv`),
so the list supports at most five; `DEV_MANUAL.md` repeats the six. Filler by DEC-171: documents only.

Also found 2026-11-03 by S327's implementer: `DEV_MANUAL.md`'s "Measured on `selfplay_v2.tsv`" paragraph gives S076's
207998 dropped in the present tense; since S042 a current binary drops 208490 on the same input (S327's
`adocs/data/S327_verify.txt`).

## 2026-11-03: the six statements, read against their sources (Opus 5.5 subagent, briefed by the coordinator)

Base: tree `4d1bd4f8`, which is `1bb3080` with S324, S325, S328, S326 and S327 staged, in the worktree `chesso-s329`.
The staged steps change no file under `src/`. Only `DEV_MANUAL.md`, `MANUAL.md`, this file and four new data files under `adocs/data/`, with their `README.md` rows, change. Rows 1 to 3 are
the step as filed. Rows 4 to 6 are findings 1 to 3 of the first delivery, taken in at the coordinator's word because
they say which constants ship, as row 1 does, and row 4 contradicted row 1's new passage on the same page. Each
statement was true or false when written, as the table says. A true one was dated or put in the past tense, and the reading of today was
added beside it. A false one had only its false words changed. Nothing was shortened and no history was removed
(DEC-348). The line numbers below are written in words, so this file carries no citation the check refuses.

The two scripts behind rows 1, 2 and 5 are committed with their outputs, at the coordinator's word, so that the
readings reach a machine without this one's `.tuning/`. They read git history only. `adocs/data/S329_eval_tables_consts.py`
takes each commit's `src/eval_tables.hpp` by `git show` and compares the five piece-value defines, `piece_value`,
`phase_value`, `psqt_mg` and `psqt_eg` with the previous commit's; its output is
`adocs/data/S329_eval_tables_consts.txt`. `adocs/data/S329_s187_lines.sh` prints each S187 citation's cited lines at
three trees: the commit that wrote its line number, the baseline the mapping records, and `124e557`, S187's start. It
also prints where the named code was in each writing tree, and the `git log -S` that found each writer. Its output is
`adocs/data/S329_s187_lines.txt`. Each has a row in `adocs/data/README.md`.

| # | where | what the check read | what it became |
|---|---|---|---|
| 1 | `DEV_MANUAL.md`, after S065's second fit: "**The second fit's constants are the ones in `eval_tables.hpp`, re-anchored.**" | written at `33aa3b4` (S065's landing, 2026-08-15, `git log -S`), true then. `git log 33aa3b4.. -- src/eval_tables.hpp` lists six commits. By `S329_eval_tables_consts.py`: `77d7450` (S076's paste, 2026-08-17) moved `PAWN` 95 to 94 and `BISHOP` 306 to 308, plus 368 of 384 entries in each of `psqt_mg` and `psqt_eg`. `1ec6e89` (S253, 2026-10-05) moved no constant: it rewrote the accumulator hooks. `2900cfe` (S117, 2026-10-05) moved none: it derives the packed copy at compile time. `0d9faba` (S134, 2026-10-12) moved 8 entries in each table, the pawn's seventh rank, mg -17 and eg +42 each, folding passer bucket 5 in with `evaluate()` identical (DEC-090). `bee84d8` (S262, 2026-10-12) moved none: one comment. `01222f3` (S083's paste, 2026-10-25) moved all five defines (`QUEEN` 716 to 983), 366 of 384 mg entries and 365 of 384 eg entries. `piece_value` and `phase_value` never changed. `1bb3080` equals `01222f3`, and this base touches no `src/` file. So the sentence was true from `33aa3b4` until `77d7450`, two days | the bold sentence put in the past tense, with both ends dated: "were ... from `33aa3b4`, 2026-08-15, until S076's paste two days later". Every other sentence of the paragraph is unchanged. Added after its last sentence: the six commits, the three that moved a constant, and what the file holds, dated 2026-11-03 |
| 2 | `DEV_MANUAL.md`, "What a citation is worth, and what proves it": "S187's records six citations that were wrong at the moment they were written" | written at `dc878de` (S187's stamp commit, 2026-09-09). S187's stamp, left as written: "**Six more were wrong when written and are recorded in `HAND_CHOICES`**". It lists eight citations: line 674 of `src/chesso.cpp` by S099, S132 and S159; S132's node counter, line 453 of `src/data_structures.hpp`; S132's per-iteration reset, line 726 of `src/chesso.cpp`; S115's two, lines 60 to 64 and 88 to 89 of `src/chesso.cpp`; S024's countermove write, line 517 of `src/search.cpp`. That is 3 + 1 + 1 + 2 + 1 = 8. S099's is not in `HAND_CHOICES`: `adocs/data/S187_symbols.tsv` has it as a WALK row. The generator's comment above `HAND_CHOICES` in `adocs/data/S187_symbolise.py` says "The first six are the blind class -- a citation that was already wrong when it was written". Its first six entries cover seven citations: S024's write and S024's array (line 436 of `src/data_structures.hpp`); S091's and S109's `see_value` at line 1096 of `src/bitboard.cpp`, S091's entry covering two citations; and S115's two. Three are on both lists, so the lists name twelve. **Read in the tree that wrote each line number (`S329_s187_lines.sh`), two were wrong when written, S024's two; the other ten were right, and all twelve had drifted by `124e557`.** The details are below the table | "that were wrong" became "as wrong", which is what S187 recorded. S325's sentence is kept. A passage is added before `DEC-119.`: six is S187's count; the eight in the stamp and the generator's six, by name; S024's two wrong when written, and how; the other ten right when written, with the commits that wrote them; all drifted by `124e557`. `plan_done/S187` is not edited |
| 3 | `DEV_MANUAL.md`, "Deduplicate a corpus by position": "**Measured on `selfplay_v2.tsv`:** ... **207998 dropped, 1.8903 %** ..." and the repeat histogram after it | written at `a579f46` (S076, 2026-08-17, `git log -S`): S076's reading, matched by S076's step file and stamp. Re-read in `adocs/data/S327_verify.txt` (staged): `a3e84e1`, S042's parent, reproduces it to the row. The base and S327 both read "11003693 rows read, 10795203 distinct positions written, 208490 dropped (1.8947%)". Their repeats: 10732552 keys once, 35598 twice, 9586 three times, 10550 four to seven, 4285 eight to fifteen, 2632 sixteen or more, most repeated 120. Both give the same kept rows with and without `--verify`. The file is clear and the pass was not re-run. 208490 - 207998 = 492 = 10795695 - 10795203, the rows S327's paragraph below attributes to S042's key | dated "at S076, 2026-08-17", with its three present-tense verbs put in the past tense. S076's figures are kept to the digit. Added beside them: what a current `corpus_dedupe` drops and writes, with its repeats, from S327's run, and one sentence pointing at S327's paragraph for the 492 |
| 4 | `DEV_MANUAL.md`, three paragraphs after row 1: "**S076 refitted the same corpus deduplicated and those constants ship now.**" | written at `d9fb0b2` (S076's completion, 2026-08-17, `git log -S`), true then: S076's paste is `77d7450`, the same day. Row 1's reading of `src/eval_tables.hpp` holds for these constants: S134's fold `0d9faba` moved eight pawn entries with `evaluate()` identical, and S083's paste `01222f3`, 2026-10-25, replaced the material and the tables. `01222f3` also rewrote the fitted weights in `src/evaluation.cpp`: passed pawns, pawn structure, mobility and king safety, each pair of arrays (`git show`). So true until `01222f3`, false since | the bold sentence in the past tense, dated at both ends: "shipped from `77d7450`, 2026-08-17, until S083's refit replaced them at `01222f3`, 2026-10-25". One sentence is added for S134's fold, which changed their form and not the evaluation. The rest of the paragraph is unchanged |
| 5 | `DEV_MANUAL.md`, "A fitted material value only means anything together with its own tables": "`QUEEN` reads 716 here against the 1067 that shipped before the fit and the 1148 the fit itself produced" | written at `33aa3b4` (`git log -S`), true then. 1067 is `33aa3b4^`'s define and 1148 is the fit's own `QUEEN`, minus 432 re-anchored, which is 716 (S065's step file). S076's paste kept `QUEEN` at 716, by the define at `77d7450` (`adocs/data/S329_eval_tables_consts.txt`). `01222f3` set it to 983: S083's fit gave 710 (S083's step file) and its re-anchoring moved 273 onto it, by the comment above the tables in `src/eval_tables.hpp` (DEC-307). So "here", read as the file, was true until `01222f3` | "reads 716 here" became "read 716 in the second fit's constants, re-anchored", which names whose 716 it is. The clause "none of the three is comparable" is kept. Two sentences are added: S076's paste kept 716; since `01222f3` the file reads 983, 710 plus 273 (DEC-307). The S275 sentences after it are unchanged |
| 6 | `MANUAL.md`, the known-limits entry "The reported score is unreliable in both directions": "They predate it by one more generation now: all 827 were refitted again on 2026-08-16 ... (S065, +21.10 Elo). The phase-by-phase disagreement above has **not** been re-measured against these constants." | written at `67eed7e` (S065's completion, 2026-08-16, `git log -S`). **Two of its words were false when written**, found at the coordinator's fast check and verified here. The date: the second fit ran on 2026-08-14 (wall 14:34 to 15:13 in S065's step file, committed with `818d4de` that day), `33aa3b4` pasted it on 2026-08-15, and 2026-08-16 is only the verdict, `67eed7e`. The count: 817 of the 827 were free, under `--freeze tempo,piece_placement`, by the run's own first line in S065's step file and by `DEV_MANUAL.md`'s "817 free parameters". The rest was true then. Since then: S076 refitted on the deduplicated corpus (`77d7450`, 2026-08-17, H1); S083 refitted on S082's pool (`01222f3`, 2026-10-25, H1). Five term fits were then pasted on top and each read H1: piece placement (S135, `c51561b`, 2026-10-26), mobility (S121, `e4ec786`, 2026-10-29), passed pawns (S123, `7621172`, 2026-10-30), pawn structure (S125, `8f9f4d0`, 2026-10-31), threats (S101, `bc550ea`, 2026-11-01). The first four are ancestors of `55d7e75`, so DEC-341's restore to that tree (`6a57c81`) kept them; S101's verdict came after it. S122's landing `b62becb` is "No functional change". The entry's figures are S028's, `adocs/data/S028_raw.tsv`, landed with `6993b89` on 2026-08-11. Nothing re-measures them later. A grep of `adocs/plan_done/` for "90th percentile", "5582", "late middlegame", "phase-by-phase", "204.3" and `analyse_game` finds S018's earlier analysis (2026-08-10), numbers that only share digits, and fixes to the tool. `adocs/data/` holds no per-phase error file later than S028's. So "now" and "these constants" were S065's, and the "not re-measured" claim still holds for every later set | "They predate it by one more generation now: all 827 were refitted again on 2026-08-16 from a second corpus, generated by the stronger engine the earlier fits produced (S065, +21.10 Elo)" became "From 2026-08-15 they predated it by one more generation: 817 of the 827 were refitted again on 2026-08-14 from a second corpus, generated by the stronger engine the earlier fits produced, and pasted the next day (S065, +21.10 Elo, its verdict on 2026-08-16)". The false words are corrected, and "now" is dated by the paste, as row 1 dates it. One sentence is added, two dated clauses in the manual's register: refitted twice since, S076 2026-08-17 and S083 2026-10-25, and the five terms fitted on top between 2026-10-26 and 2026-11-01. "against these constants" became "against any of these constants". The S034 and S028 history before it and the closing advice are unchanged |

### Row 2's twelve, each read three times

"Writer" is the commit that last introduced the citation's line number in its step file. It was found by `git log -S`
on the citation as spelled, then confirmed by reading the step file's diff at that commit. "Baseline" is the `rev` in
`adocs/data/S187_symbols.tsv`, the last commit that edited the step file before S187. Every reading is in
`adocs/data/S329_s187_lines.txt`.

| citation | writer | what the line held at the writer | at the baseline | at `124e557` |
|---|---|---|---|---|
| S099, line 674 of `src/chesso.cpp`, two lines | `7862f41` (S144) | the `search_state_t` construction and `state.tt` | `7862f41`, the same | the book helper's `legal_moves` array |
| S132, line 674 | `fe25f46` (S169) | the `search_state_t` construction | `7504c3d`: `legal_moves` | `legal_moves` |
| S159, line 674 | `fe25f46` (S169) | the `search_state_t` construction | `192a5a3`: `legal_moves` | `legal_moves` |
| S132's node counter, line 453 of `src/data_structures.hpp` | `fe25f46` (S169) | the `explored_nodes` member of `search_state_t` (the struct opened at line 447) | `7504c3d`: the brace opening the table struct | the same brace |
| S132's per-iteration reset, line 726 of `src/chesso.cpp` | `fe25f46` (S169) | `state.explored_nodes = 0;` in the deepening loop | `7504c3d`: a `break;` in the book helper | the same |
| S115, lines 60 to 64 of `src/chesso.cpp` | `7862f41` (S144, from S115's own bare lines 48 to 52) | the test-only counter's comment and declaration, `last_aspiration_failures` | `192a5a3`: the search thread and output mutex | the same |
| S115, lines 88 to 89 | `7862f41` (S144) | its accessor, `uci_last_aspiration_failures()` | `192a5a3`: a typedef and a blank | the same |
| S024's countermove write, line 517 of `src/search.cpp` | `7b54f18` (S138) | **a comment line in `negamax`**, reverse futility's; the write was at line 771 | `192a5a3`: `unmake_move(game);` | the same |
| S024's countermove array, line 436 of `src/data_structures.hpp` | `7b54f18` (S138) | **`bool mate_found;` in `search_t`**; the array was at line 459, in `search_state_t` | `192a5a3`: a padding comment | the same |
| S091's two, line 1096 of `src/bitboard.cpp` | `c0954ec`; S144 gave one of them its path at `7862f41`, the same line | the `see_value` declaration, at both | `7862f41`: the declaration, still | a blank line, the declaration one line up |
| S109's, line 1096 | `c0954ec`; pathed by S144 at `7862f41`, the same line | the `see_value` declaration, at both | `7504c3d`: a blank line | the same |

S024's two were right when first written. At `c167d05`, 2026-08-13, they were lines 434 and 507, which held the array
and the write. S138 re-anchored them at `7b54f18` to lines 436 and 517, which held other text in that tree. Lines 436
and 517 are the same in the tree S138 reports it measured against, `04bcc43`, because no `src/` file differs between
`04bcc43` and `7b54f18`. The same re-anchoring moved S024's countermove read to lines 1152 to 1155 of
`src/evaluation.cpp`. In that tree those lines were the capture branch, and the read was at line 1164, so the read was
wrong when written too. The 2026-09-04 review's F07 lists the write and the read together as its S024 item. S187's
`accepts:` names that item among its four as "S024's Note", and the stamp's eight list the write a second time. The
Note's text is unchanged through `e424032` and `a433433`, which moved the file and reverted it.

Neither reading gives six. In the writing trees, the stamp's eight hold one wrong citation, S024's write, and the
generator's seven hold two. At the baselines, the eight hold seven wrong, all but S099's, whose baseline is its
writer. The seven hold five wrong, all but S091's two, whose baseline still had the declaration on the cited line.
`DEV_MANUAL.md` takes the first reading, the one its sentence claims: "wrong at the moment they were written".

### Old and new, side by side

Row 1, `DEV_MANUAL.md`, the paragraph's first line and its last:

```
old: **The second fit's constants are the ones in `eval_tables.hpp`, re-anchored.**
     The first paste fired three guards and was reverted; the second fired one,
     ...
     `adocs/plan_done/S065_corpus_regen_loosened_filter.md` records the
     transformation and both fits' guard outcomes.
new: **The second fit's constants were the ones in `eval_tables.hpp`, re-anchored,
     from `33aa3b4`, 2026-08-15, until S076's paste two days later.**
     The first paste fired three guards and was reverted; the second fired one,
     ...
     `adocs/plan_done/S065_corpus_regen_loosened_filter.md` records the
     transformation and both fits' guard outcomes. Six commits have changed
     `src/eval_tables.hpp` since `33aa3b4`, and three of them moved a constant:
     S076's refit (`77d7450`, 2026-08-17, below), S134's fold of passer bucket 5
     into the pawn tables' seventh rank, the evaluation identical (`0d9faba`,
     2026-10-12), and S083's refit (`01222f3`, 2026-10-25), whose constants the file
     still holds on 2026-11-03. S253's, S117's and S262's (`1ec6e89`, `2900cfe`,
     `bee84d8`) changed its code and comments, not a constant.
```

The six lines elided as `...` are unchanged.

Row 2, `DEV_MANUAL.md`, the first sentence's false words and the paragraph's end:

```
old: generator beside it. S187's records six citations that were wrong at the moment
     they were written, a class no checker could see because its baseline was the
new: generator beside it. S187's records six citations as wrong at the moment
     they were written, a class no checker could see because its baseline was the

old: and all three had drifted into the helper by `124e557`. DEC-119.
new: and all three had drifted into the helper by `124e557`. **Six is S187's count;
     read in the trees that wrote them, two of the citations it names were wrong.**
     S187's stamp, left as written, says six and lists eight citations: the three
     above, S132's node counter and per-iteration reset, S115's two for the
     test-only aspiration counter and S024's countermove write. Its generator,
     `adocs/data/S187_symbolise.py`, gives that class as the first six entries of
     `HAND_CHOICES`, seven citations: S024's two, S091's two and S109's one for
     `see_value`, and S115's two. Of the twelve the two lists name, S024's two were
     wrong when written. They were first written right at `c167d05`, and S138
     re-anchored them at `7b54f18` to a line holding `bool mate_found;` in
     `search_t` and to one in a comment in `negamax`, the `counter_moves` array and
     its write lying further down. The other ten held what they name in the tree
     that wrote them, S132's two at `fe25f46` (S169), S115's two at `7862f41`
     (S144) and the three for `see_value` at `c0954ec`, two of them given their
     path by S144 at `7862f41` on the same line, and all ten had drifted by
     `124e557`. DEC-119.
```

Row 3, `DEV_MANUAL.md`, the paragraph whole:

```
old: **Measured on `selfplay_v2.tsv`:** 11003693 rows in, 10795695 distinct
     positions out, **207998 dropped, 1.8903 %**, in 19 s and 1.0 GB resident. The
     repeats are concentrated rather than spread: 10733523 positions occur once,
     35124 twice, 9585 three times, 10547 four to seven times, 4284 eight to fifteen,
     2632 sixteen or more, and one position appears **120** times.
new: **Measured on `selfplay_v2.tsv` at S076, 2026-08-17:** 11003693 rows in,
     10795695 distinct positions out, **207998 dropped, 1.8903 %**, in 19 s and
     1.0 GB resident. The repeats were concentrated rather than spread: 10733523
     positions occurred once, 35124 twice, 9585 three times, 10547 four to seven
     times, 4284 eight to fifteen, 2632 sixteen or more, and one position appeared
     **120** times. **A current `corpus_dedupe` drops 208490 of the same rows,
     1.8947 %, and writes 10795203** (S327's run, 2026-11-03,
     `adocs/data/S327_verify.txt`): 10732552 positions once, 35598 twice, 9586 three
     times, 10550 four to seven times, 4285 eight to fifteen, 2632 sixteen or more,
     and one still at 120. The 492 more are rows S042's key merges, below.
```

S327's paragraph further down already says "since S042 the pass drops 208490 rows". Row 3 leaves it unchanged.

Row 4, `DEV_MANUAL.md`, the paragraph's first line:

```
old: **S076 refitted the same corpus deduplicated and those constants ship now.**
     Same settings, same seed, same freeze, `--lambda 0`; the only difference is
new: **S076 refitted the same corpus deduplicated, and those constants shipped from
     `77d7450`, 2026-08-17, until S083's refit replaced them at `01222f3`,
     2026-10-25.** S134's fold changed their form in between, not the evaluation.
     Same settings, same seed, same freeze, `--lambda 0`; the only difference is
```

Row 5, `DEV_MANUAL.md`, the paragraph's second and third lines:

```
old: **A fitted material value only means anything together with its own tables.**
     `QUEEN` reads 716 here against the 1067 that shipped before the fit and the 1148
     the fit itself produced, and none of the three is comparable with the others.
     Since S275 a fit with material and psqt both free holds, per piece, the sum of
new: **A fitted material value only means anything together with its own tables.**
     `QUEEN` read 716 in the second fit's constants, re-anchored, against the 1067
     that shipped before the fit and the 1148 the fit itself produced, and none of
     the three is comparable with the others. S076's paste kept 716. Since S083's
     paste (`01222f3`, 2026-10-25) the file reads 983: S083's fit gave 710, and its
     re-anchoring moved 273 onto it (DEC-307).
     Since S275 a fit with material and psqt both free holds, per piece, the sum of
```

Row 6, `MANUAL.md`, the entry's closing sentences:

```
old:   predate the current evaluation. They predate it by one more generation now:
       all 827 were refitted again on 2026-08-16 from a second corpus, generated by
       the stronger engine the earlier fits produced (S065, +21.10 Elo). The
       phase-by-phase disagreement above has **not** been re-measured against these
       constants. The shape of the warning is what to keep: the typical move is
       close, the tail is wide, and a pawn endgame score deserves suspicion.
new:   predate the current evaluation. From 2026-08-15 they predated it by one more
       generation: 817 of the 827 were refitted again on 2026-08-14 from a second
       corpus, generated by the stronger engine the earlier fits produced, and
       pasted the next day (S065, +21.10 Elo, its verdict on 2026-08-16). They have
       been refitted twice since, on that corpus deduplicated (S076, 2026-08-17) and
       on a new one (S083, 2026-10-25), and the piece placement, mobility, passed
       pawn, pawn structure and threat terms were each fitted on top of S083's
       between 2026-10-26 and 2026-11-01. The phase-by-phase disagreement above has
       **not** been re-measured against any of these constants. The shape of the
       warning is what to keep: the typical move is close, the tail is wide, and a
       pawn endgame score deserves suspicion.
```

### Manuals

`DEV_MANUAL.md` changed in rows 1 to 5 above and nowhere else. `MANUAL.md` changed in row 6's entry and nowhere
else. It names none of rows 1 to 3's statements, by grep for `207998`, `208490`, `eval_tables`, `S187`, "second fit" and
`corpus_dedupe`. Its other mentions of a refit are five option-table cells that give a pawn as 94 "at the seed's
weights" and as 84 since S083's refit. They are dated already, and they agree with rows 4 and 5. `TOOLCHAIN.md` and
`adocs/specs.md` hold none of the six. No UCI surface moved, so `test_uci_surface` is not touched.

### Found

Findings 1 to 3 of the first delivery are rows 4 to 6 above, taken in at the coordinator's word. The two below concern
`adocs/plan_done/` history, which no pending citation depends on any longer. They are recorded, and no action is taken:
`plan_done/` is never edited.

1. **S138's re-anchoring at `7b54f18` wrote lines that were wrong in its own tree.** All three of S024's Note
   citations: array, write and read (row 2's details). S138's other re-anchorings were not read. Every pending
   citation has had the symbol form since S187, so nothing live points through them. The defect is history only,
   and these three are its only measured cases.
2. **S187's stamp says its six "are recorded in `HAND_CHOICES`"**, but one of the eight it names, S099's, is a WALK
   row of `adocs/data/S187_symbols.tsv` with no `HAND_CHOICES` entry. Its "more" is also not disjoint from the four
   before it: S024's write is in both, since F07's S024 item holds it. `DEV_MANUAL.md`'s new passage names both lists
   and does not repeat either claim.

### Checks

Docs only, so no build was compiled and no bench was taken. `build` and `build-tune` were configured in the worktree
(Release, ccache, `-DCHESSO_TUNE=ON` for the second), with the doctest submodule taken offline from the main tree and
`.git/config` unchanged by sha256. Only the entries that read documents were run, under `nice -n 10` beside S122's
screen, after each of the three deliveries, with the same readings each time:

- `CLANG_FORMAT_MAJOR=22 ctest -L fast -R 'test_plan_|test_clang_format_script'`: in `build`, 6 of 6 passed
  (`test_clang_format_script`, `test_plan_touches`, `test_plan_params`, `test_plan_gate`, `test_plan_citations`,
  `test_plan_citation_freshness`), and in `build-tune` the same 6 of 6.
- `python3 tools/plan_prose_check.py`, all five checks: rc 0. Prose 0 sentences flagged, citations 0 over 25 files,
  touches 0, params 0, and the gate command agrees in both documents. `--citations` alone: rc 0, 0 flagged over 25
  files, this file 0 code citations. `--citations` reads the pending directories and not `DEV_MANUAL.md`, so the
  `path:line` S325 kept there is not in its scope.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check`: rc 0.

The rest of the fast suites was not run: no code moved. `MANUAL.md`'s option names and table are untouched, so
`test_uci_surface`, which needs a compiled build, has nothing new to read.
