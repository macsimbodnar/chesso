id:         S041
goal:       a test that fails the moment a tuner group range is appended without re-ending the one before it
accepts:    a test asserts that every group's range is non-empty, that the groups are pairwise disjoint, and that their union is exactly [0, PARAM_COUNT); it was observed failing against a deliberately re-introduced swallowing bug
touches:    tools/tuner.cpp free_mask (split so a test can reach it), tests/
excludes:   the fit itself, its objective, and its parameters
decisions:  DEC-041
closes:     2026-08-13_adversarial-F07
blocks:
paused_by:
done:      2026-08-13. tests/test_tuner_groups.cpp holds three properties over the tuner's --only group ranges: every group frees at least one parameter, the groups are pairwise disjoint, and their union is exactly [0, PARAM_COUNT). Group names are parsed out of tuner_groups::GROUP_LIST and every length and index comes from eval_model::PARAM_COUNT (827); the test contains no literal for either. Preconditions are asserted first and would fail if absent: PARAM_COUNT > 0, at least two names in the list, exactly one 'all', every listed name accepted by free_mask, 'not_a_group' refused, and TEMPO_EG_BASE + TEMPO_COUNT == PARAM_COUNT so eval_model.hpp's bases chain agrees with its independent width sum. Registered with CTest under label 'fast', TIMEOUT 60, 45 assertions in 0.00 s; the fast suite is now 10/10 in 17.1 s. **Observed red three ways.** The fourth recorded swallowing, piece_placement re-ended at PARAM_COUNT instead of TEMPO_MG_BASE: CHECK( 2 == 0 ), '2 of 827 parameters are freed by more than one --only group; the first is index 825, claimed by piece_placement and tempo'. tempo dropped from GROUP_LIST: CHECK( 2 == 0 ), '2 of 827 parameters are in no --only group; the first is index 825'. material's range emptied: CHECK( 0 > 0 ), 'group frees no parameter at all: material'. **GROUP_LIST and free_mask moved to tools/tuner_groups.hpp** so a test can reach them; tuner.cpp imports both under their old names and no call site moved. The move is textual: diff of the moved region against git show HEAD:tools/tuner.cpp differs on exactly two declaration lines, const char* const -> inline constexpr const char* and bool -> inline bool, so the body and all four historical comments are verbatim. **Behaviour identical, established two ways rather than asserted.** HEAD's own free_mask text compiled into a scratchpad translation unit and asked for all 9 groups plus 8 names it must refuse, dumping return value, mask length and all 827 bits per call: 8160 bytes over 19 lines, byte-identical to the same dump through the new header. End to end, tuner --only <group> for all 9 groups over a 2-row TSV at --k 1 --epochs 1 --report 1 --validation 0.5 --threads 1 --seed 1, the fcd82f8-built binary against the new one: emitted header and stderr identical for every group after normalising the --out filename, and the 9 emitted headers are 9 distinct files so the comparison is not vacuous. --help and the unknown --only group error are identical too. **This step file was corrected in place while current, twice**, per S043's and S040's lesson. Its two code line ranges were stale, inherited from the audit finding: the function is at tools/tuner.cpp:153-223 and the tempo branch at :209-211, not :198-200, which is the pawn_structure comment. And its Shape section named the union clause as the one that fires when a group is appended without re-ending its predecessor; it is the disjointness clause, as the first red run above shows, because the predecessor still covers the new block and the union is still the whole vector. Both corrections carry a dated note in the file. **Not closed, reported instead:** tempo still runs to PARAM_COUNT, so a block appended after it and given no group of its own is covered by tempo and all three properties still hold. The test asserts TEMPO_EG_BASE + TEMPO_COUNT == PARAM_COUNT as a precondition and fires there instead; removing the blind spot means restructuring GROUP_LIST, which this step's excludes: puts out of reach. DEV_MANUAL.md: the tuner section said the four swallowings would fail no test, which this step makes false -- narrowed to the first four, with test_tuner_groups and the three properties described. MANUAL.md checked, no change needed: the tuner is not end-user surface and --help is byte-identical. README.md checked, owner-written, no change needed. No SPRT: the tuner is not the engine and nothing here is in the search path. 2026-08-13_adversarial-F07 stays 'planned'; only an audit re-run closes it. Gate: cmake --build build -j12 exit 0, ctest --test-dir build -L fast 10/10 exit 0, ./clang-format.sh --check exit 0.

## The recurring defect

`tools/tuner.cpp:153-223` at `fcd82f8` — `GROUP_LIST` at `:153-155` and
`free_mask` at `:168-223`. Four consecutive comments in one function record the
same bug four times: `king_safety` ran past the passed pawn weights (`:186-188`),
`passed_pawns` past the pawn structure weights (`:192-194`), `pawn_structure`
past the piece placement weights (`:198-200`), and the fourth comment (`:204-206`)
says outright that it is "the fourth term in a row to meet the same defect in the
same function".

The fifth is already loaded. `tempo` ends at `PARAM_COUNT`
(`tools/tuner.cpp:209-211`), so the next group appended after it is swallowed
unless whoever appends it remembers to move that end.

*Corrected while current, 2026-08-13.* The two ranges above read `157-209` and
`198-200` when the step was written, inherited from `2026-08-13_adversarial-F07`.
`:198-200` is the `pawn_structure` comment, not the `tempo` branch; the ledger
row S040 already recorded `tempo` at `:209-211`.

```
$ grep -rn "free_mask\|GROUP_LIST" tests/
(no output)
```

Nothing in `tests/` links `tools/tuner.cpp` at all. `test_eval_model` is the
only test reaching into `tools/`, and it tests the model, not the mask.

## Why it stays invisible

A swallowed group hands the new term's weights back exactly as it received them,
which reads as "the fit found nothing" — and DEV_MANUAL already teaches that a
term fitting to almost nothing is normal, because residual fits do that. So the
failure is silent and specifically camouflaged by the surrounding workflow.

One occurrence costs a wasted fit plus the SPRT after it, which S027 prices at
three to four and a half hours.

## Shape

No dataset needed. The three properties are enough.

`free_mask` is about 50 lines with no dependencies. Either split it into a
header or compile the test against the translation unit.

Red first: re-introduce one of the four recorded swallowings, observe the
assertion fail, put it back.

*Corrected while current, 2026-08-13.* This section said the **union** clause is
the one that fires when a group is appended without re-ending its predecessor,
and it is the **disjointness** clause: the predecessor still covers the new
block, so both groups claim it and the union is still the whole vector. Observed:
re-introducing the fourth recorded swallowing failed `overlaps == 0` with "2 of
827 parameters are freed by more than one --only group", and the union clause
passed. Union is what fires when a block is appended and given no group at all
— also observed, by dropping `tempo` from `GROUP_LIST`.

The union clause has one blind spot that this step does not close, because
closing it means restructuring `GROUP_LIST`: `tempo` runs to `PARAM_COUNT`, so a
block appended after it with no group of its own is covered by `tempo` and all
three properties still hold. The test asserts a precondition against that —
`TEMPO_EG_BASE + TEMPO_COUNT == PARAM_COUNT`, the bases chain against
`eval_model.hpp`'s independent sum — and fires there instead.
author:    Maksym Bodnar
