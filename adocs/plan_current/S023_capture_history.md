id:         S023
goal:       history indexed by piece, target and victim, to order captures MVV-LVA rates equal
accepts:    an SPRT returns a verdict; the ordering bands stay disjoint, discharged by a case ctest runs and not by reading the diff -- a king capture of a pawn scored against a killer at the same ply, asserting the capture wins -- written in this step's own commit; the bound on the capture-history term that keeps a capture score inside its band is stated as a constant with a range in src/search_params.hpp
            (Folded in from the retired S061 by DEC-086. Order note from the
            2026-08-19 review: capture history's measured value is in the
            capture futility and capture SEE margins, not in the ordering key --
            wire it into the margins first, and let it into score_move second.)
touches:    src/evaluation.cpp score_move, src/data_structures.hpp search_state_t -- amended by the implementer 2026-10-13 (As built, deviation 2): verdict 1 lands in src/data_structures.hpp, src/search.cpp, src/search.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_evaluation.cpp, tools/mutants/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md and adocs/data/S023_sprt.sh; src/evaluation.cpp score_move is verdict 2's. Amended again at the rebase onto c3ed1ef (2026-10-14): the PENDING phase-2 markers also land in tests/test_mate_carry.cpp, tests/test_mate_breadth.cpp, tests/test_engine.cpp and adocs/data/S170_cases.tsv, and adocs/data/README.md takes the script's row
excludes:
decisions:  DEC-258, DEC-273
closes:     2026-08-13_plan_review.2-F06
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent, phase 1 written blind beside S261's lane (DEC-260), started 2026-10-13
done:

## Promoted 2026-10-08, DEC-258: the capture-history family enters the main order

The owner moved capture history out of the reserve as a family of three
verdicts after the correction-history steps: this step, then S025 (losing
captures searched after the quiets), then S265 (late captures reduced,
scaled by this history). The reserve reasoning below is kept as history and
as the risk this step carries: Weiss measured the ordering use at
-4.17 +/- 4.83 at short control and +3.66 +/- 3.29 at long, so a zero or a
loss at 8+0.08 is an expected outcome and is recorded as such. One `{0, 5}`
nElo SPRT with DEC-143's worst case pre-registered; the ordering-band case the
accepts names is written before any score changes.

## Reserve, 2026-08-19, DEC-087

Demoted behind the 3000 push by the second review. The band evidence is
against it as an ordering term at this strength. **The Lynx half of that
evidence is withdrawn** (S181, 2026-09-11, DEC-176): this file read "Lynx
failed four SPRTs on it at ~2600 (-35.8 to -11.1)", no source was ever located
for those four runs, and the API record says the opposite of the headline --
**Lynx merged capture history**, pull request #634, 2024-02-02, into v1.3.0 at
a banded **2653** (`adocs/data/S181_lynx_bands.md`). The closed-unmerged capture-history pull
requests in that repository are later refinements of a feature already in the
tree, dated 2024-09 onward at 2925 and above, not four attempts at the
feature. What is left, and it is enough: Weiss measured it **-4.17 +/- 4.83** at
short control against **+3.66 +/- 3.29** at long -- Weiss pull request #428,
"Capture History", 2021-06-04, 9088 games at 10+0.1 and 15936 at 60+0.6, the
author's own summary being "hurts STC, but LTC shows decent gain"
(https://github.com/TerjeKir/weiss/pull/428; the -4.2 and +3.7 quoted here
before 2026-09-11 were that pair rounded). Its documented value arrives as an
*input* -- reducing tacticals with bad capture history measured **+7.22 +/-
4.72** at 10.0+0.1s over 7696 games and **+2.37 +/- 1.90** at 60.0+0.6s over
31984 games at Ethereal, **sourced 2026-09-13** (after S186's fast check):
commit dcb8560cb3c8b6bb37b6fbaa5cc57c681db7809b, 2020-09-26, "Use Capture
History to apply LMR to some Tactical Moves", whose message says the reduction
goes "from R=1 to R=2, for Tactical moves which have a negative Capture
History score"
(https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+%22capture+history%22;
commit message only, DEC-016). The "+7.2/+2.4" this file quoted before was
that pair rounded. Note the direction: the *input* use **shrinks** at long
control, where the ordering use above **grows** (Weiss, -4.17 short against
+3.66 long), so the two halves of this step are not priced by one number --
and capture futility
and SEE margins read it, which is work for the 3000-plus phase this reserve
exists for. The order note below stands for whenever it runs.

## Hazard

The move-ordering bands clear each other by 100 points. A king capturing a pawn
scores `1000000 + 100 - 100000 = 900100`, against 900000 for a killer. Any new
term added to a capture score can invert that silently, and the symptom is a
strength regression rather than a wrong node count.

## As built, phase 1 -- 2026-10-13, written blind beside S261's lane

Written under DEC-260 while S261's SPSA lane held every core: nothing here was
compiled, run, timed or tested. Every number below that a run would give is
marked pending; every expectation is an expectation.

### Deviations, first

1. **Two verdicts, and this is the first.** The Promoted section says one
   `{0, 5}` SPRT; the order note says the margins first and the ordering key
   second, and the Reserve section says the two halves "are not priced by one
   number" -- the ordering use is the one the record prices negative at short
   control (Weiss, -4.17 +/- 4.83), the margin use the one the note calls the
   value. One run over both could net a real margin gain against an ordering
   loss into a zero and send S025 and S265 back to the reserve on a number
   that priced neither (MEASUREMENT, one change at a time). So **verdict 1 is
   the table and its two margin readers**, `adocs/data/S023_sprt.sh`, and
   **verdict 2 is the ordering key** -- the record let into `score_move` as a
   bounded tie-break inside the capture band, its bound the accepts' constant
   with a range in `src/search_params.hpp`, measured against verdict 1's
   landing. The accepts' third clause is therefore verdict 2's and is not met
   by this phase; the first two are (the SPRT is pre-registered; the band case
   is written). An owner decision, proposed and not taken: the report carries
   the question in DEC-265's form.
2. **`touches:` amended** in the header, listing where verdict 1 lands.
   `src/evaluation.cpp` `score_move` is not touched by verdict 1.
3. **"Capture futility" is read as S112's per-move futility in quiescence**,
   the only capture futility margin this tree has: the main search's futility
   rule (S109) is quiet-only, and a capture futility rule in the main search
   would be a new pruning rule and a new technique, not the record wired into
   a margin. The brief names `QsFutilityMargin` as the site.
4. **S091's mutants C05 and C06 are re-pointed** in
   `tools/mutants/S091_capture_see.py`: the capture rule's lines moved, and an
   anchor that no longer occurs makes `tools/mutation_check.py` refuse the
   whole run. The mutations are the ones they were; both owe a re-observation.
5. **The capture span shares the quiet span's storage**, at the tail of
   `quiets_tried` growing down, rather than an array of its own -- a node
   searches each legal move once, so the two spans cannot meet, and a second
   `move_t[MAX_MOVES]` would add a kilobyte to every frame of a recursion a
   macOS thread gives 512 KiB. Said at the declaration.
6. **Disclosed**: two `python3` invocations with no input slipped into shell
   lines during the writing (`python3 -` on an empty stdin and `python3 -c
   1`), each an interpreter start and exit over nothing. No data was read and
   no build, test or engine ran.

### Design, from the description (DEC-221)

Implemented from the step file's goal and order note and the Chess
Programming Wiki's History Heuristic page as
`adocs/data/2026-09-19_search_technique_study.md` quotes it ("a history table
indexed by moved piece, target square and captured piece type, with bonus on
cutoff and malus otherwise"). No other engine's code was opened; the Weiss
and Ethereal records the file quotes are measurements, and no constant of
theirs is behind any value here.

- **The table**: `search_state_t` member `capture_history`,
  `int16_t[12][64][CAPT_HIST_VICTIMS]`, moving piece by target square by
  victim type, 9 KiB, per thread, zeroed per `go` with the struct. A sixth
  victim column (the king) is never written and exists so that no index
  derived from a board can leave the table.
- **The one index**: `src/data_structures.hpp` `capture_history_entry`, two
  overloads like `continuation_entry`, through `capture_victim_type` -- the
  victim read off the board the capture is played from, en passant taken as a
  pawn by an explicit branch (EMPTY modulo six is the pawn's column anyway,
  which is why no mutant of that branch is carried; the mutant file says so).
- **The update**: `src/search.cpp` `history_on_capture_cutoff`, called at
  every fail-high in `negamax_at` before the quiet tables' block: every
  capture the node searched before the cutoff move is charged, whatever class
  the cutoff move is, and the cutoff move is credited when it is a capture.
  `history_gravity_update` on `CAPT_HIST_BOUND`, the int16_t ceiling, graded
  by `capture_grade` in S222's unit (thousandths of the band at
  `CAPT_HIST_REF_DEPTH`, quadratic in the remaining depth). Main search only;
  quiescence reads and never writes.
- **The readers**, both through `capture_record_term` (`weight * entry /
  CAPT_HIST_BOUND`, so a weight is the term's whole size at either edge and an
  empty cell is exactly 0):
  - `src/search.cpp` `capture_see_margin`: S091's margin plus the term,
    **floored at 0**, so the capture rule still skips only a capture the
    exchange evaluation says loses material; the cell is read inside the cap
    only.
  - `src/search.cpp` `capture_futility_margin`: S112's margin plus the term,
    **floored at 0**, so a quiescence skip still fires only below the entry
    alpha. `futility_base` is gone; the margin is taken per capture.
- `score_move` does not read the table (verdict 2).

### Seeds and ranges (DEC-134), each stated at its site

| parameter | seed | range | form |
|---|---|---|---|
| `CaptHistBonus` | 106 | 0..1000 | (b) plain history's fitted grading at depth 11, `6*121 + 19*11 + 2` = 937 of its 8831 band |
| `CaptHistMalus` | 25 | 0..1000 | (b) the same, `17*11 + 36` = 223 of 8831 |
| `CaptHistQsWeight` | 94 | 0..2000 | (b) one pawn of `src/eval_tables.hpp`; top is the margin's own top |
| `CaptHistSeeWeight` | 100 | 0..900 | (b) one pawn of `see_value`; top is a queen there |

Off values: both weights at 0 is the parent's tree node for node (every term
is identically 0 and the floors cannot bind on a non-negative margin); both
shares at 0 too (nothing is ever written). Phase 2 proves both on the tree.
The derivation for the shares re-runs whenever plain history's coefficients
or bound move; S261's lane does not touch them.

### Tests and mutants

`tests/test_evaluation.cpp` "a king capturing a pawn outranks a killer at the
same ply" -- the accepts' band case, written before any score changes: the
king's capture of a pawn against a killer at the same ply, at the worst, no,
and best record, with the killer shown to score as a killer. Holds today
because `score_move` reads no record; verdict 2's term arrives under it.

`tests/test_search.cpp`, new suite "search: capture history":
"the capture table is indexed by mover, target and victim" (CX10), "a capture
that cuts off is credited and the captures tried before it are charged" (CX07,
CX08), "a quiet that cuts off charges the captures tried before it and
credits nothing", "a node that cuts off on a quiet charges the captures it
searched first" (CX11, CX12), "a search fills the capture table both ways"
(CX08, CX12), "the capture margins move with the record and never go below
zero" (CX01, CX02, CX04). In "search: quiescence futility": "a capture's worst
record makes a capture that could reach alpha futile" (CX04, CX06) and "a
capture's best record searches a capture that could not reach alpha" (CX06).
In "search: pruning and reduction guards": "a capture's record moves the
capture SEE margin at a pruning node" (CX03, CX01) and "no capture that holds
its material is skipped whatever its record" (CX02). In "pruning does not
hide a forced mate": every `capture_mates` row searched again with the worst
record in every cell, at its own depth and distance -- the hazard's guard,
through `search_fen_with_capture_record`.

`tests/test_search_params.cpp`: four rows, 73 to 77; "the capture table's
ranges are the ones their purpose declares"; tune-only "the capture margins
stay at zero or above at every weight the ranges admit".

`tools/mutants/S023_capture_history.py`, eleven mutants CX01-CX08, CX10-CX12
(CX09 not written, reason in the file; CX05 declared equivalent at the
shipped values, argued in the file -- the quiescence floor binds only where
the weight exceeds the margin, which the tune-only case holds). The CX prefix
has never been used in `tools/mutants/` (`git log --all -p`). Every new
single-line anchor was grepped and occurs exactly once.

### What phase 2 should see, per file (expectations, not observations)

- `src/data_structures.hpp`, `src/search.hpp`, `src/search_params.hpp`,
  `src/search.cpp`: compile clean in both builds under `-Wall -Wextra
  -Werror`. Every added line outside `search_params.hpp`'s clang-format-off
  block is at most 80 columns; `./clang-format.sh --check` may still reflow a
  wrapped call or two (it was not run). If it touches `src/search.cpp`, the
  CX11 and CX12 anchors and S091's C05/C06 are re-checked after.
- `tests/test_search_params.cpp`, `tests/test_evaluation.cpp`: green in both
  builds; nothing they assert depends on the tree.
- `tests/test_search.cpp`, the new suite and the two quiescence cases: green;
  their arithmetic is fixed by the seeds (worst quiescence margin 94 against
  188). The two guard cases rest on preconditions read from the engine -- a
  capture spared and one condemned somewhere in 3 positions x 5 depths, and a
  capture that holds its material at a reduced depth where the unfloored
  worst margin is negative -- expected to hold, unverified. The worst-record
  mate rows: expected green, unverified; a red there is the hazard and a
  finding, never a re-pin.
- **Goldens the tree change can move, each re-derived by its own script and
  never edited by hand** (DEC-142): `capture_mates` depths and labels, the
  mined rows `mate_the_extra_ply_hides`, the multicut and ProbCut rows, "ordering
  keeps the tree small", `test_mate_carry`'s ceilings, `test_mate_breadth`'s
  `EXACT_FLOOR`, `test_engine`'s `MATE_IN_THREE_FLOOR`. One existing case sits
  near the new term: S269's "a capture past late move pruning's count still
  takes the extra ply" reads Bxf7 at depth 10, where the comment says the
  margin clears it "with room"; a record charged hard enough could take that
  room.

### Files shared with S099's diff

(History: S099 read H0 and its diff was reverted as `5a04ffa`, so no file
is shared with it any more.)

`src/data_structures.hpp` (S099 adds `pawn_correction` after `proven_mate`;
this adds `capture_history` after `cont_hist`, a hunk apart), `src/search.cpp`
(includes, the eval sites and the node's end; this: the top helpers,
quiescence's futility loop, the move loop's capture rule and fail-high block),
`src/search.hpp` (S099 appends at the end; this inserts after
`history_on_quiet_cutoff`), `src/search_params.hpp` (S099 after `RazorDepth`;
this after `ContHistWeight`, `QsFutilityMargin` and `SeeCaptureMaxLmrDepth`),
`tests/test_search.cpp` (S099's includes, a guard near ProbCut's block and the
end; this after `search_fen`, a new suite, the quiescence futility suite's
end, the mate case and before S098 verdict 2's block), `tests/test_evaluation.cpp`
(S099 near the top; this after the history ceiling case),
`tests/test_search_params.cpp` (**conflicts on the count line**: S099 73 to
75, this 73 to 77, together 79; S099's new case sits before the release-only
block and this one after the populated-table case), `MANUAL.md` (S099 near the
`ucinewgame` paragraph; this the option table), `DEV_MANUAL.md` (different
rows), `adocs/specs.md` (**S099 rewrites the search row**; this edits the
"absent, search" row only, to stay off that line).

### Pending, phase 2 (after the lane, on the rebased tree)

1. Rebase onto S261's landing and S099's; resolve the count line; re-derive
   the shares' seeds if plain history moved.
2. Build both trees; `./clang-format.sh --check`; the TESTS rule's chain.
3. Off values on the tree: the tune build at both weights 0, then at both
   shares 0, prints the parent's bench and eight replies;
   `tools/search_bench.py` reproduces the parent.
4. Bench and `search_bench` on the candidate; DEC-266 timing (the quiescence
   loop now reads a cell per capture it tests); a reach census of how often
   the record moved each rule's decision. Into `adocs/data/S023_sprt.sh`.
5. Goldens above, by their scripts.
6. Mutants: `validate()` over `tools/mutants/`, then CX01-CX12, S091's C05 and
   C06, S269's LQ03 and S112's F01-F06 (their anchors did not move, the code
   around them did) through `tools/mutation_check.py`.
7. Second tier (DEC-141): Debug self-play, four rounds at 4+0.04, grep
   `Assertion`; `tools/gate_extra.sh`.
8. `tools/plan_prose_check.py` in its modes over this file and the documents.
9. Fast check; commit; pin and run `adocs/data/S023_sprt.sh`.

### Proposed `done:` stamp (verdict 1), to be completed

`<date>. **<H1|H0|no verdict>: capture history in the two capture margins is
worth <Elo> at 8+0.08, decided in <n> games.** adocs/data/S023_sprt.sh ran as
pre-registered -- candidate <sha> against <parent> ... Read as pre-registered:
<row>. Verdict 2, the ordering key, <is pre-registered next | is not run>.
Implemented from the step file's description and the CPW History Heuristic
page's (DEC-221); seeds DEC-134 (b). Second tier: <...>. Goldens re-derived:
<...>. Implemented by an Opus subagent in two phases; run, read and stamped by
the coordinator.`

### Commit text (verdict 1's landing)

    Feed capture history into the two capture margins (S023 v1)

    S023's order note puts capture history's value in the capture futility
    and capture SEE margins and the ordering key second, and the two uses
    are priced in opposite directions by the record, so they are two
    verdicts; this is the first. A table [piece][target][victim type],
    credited on a capture cutoff and charged for every capture searched
    before any cutoff, moves S091's capture SEE margin and S112's
    quiescence futility margin by up to a weight each way, floored at 0 so
    neither rule's class changes. Both weights at 0 is the parent's tree.
    Implemented from the CPW History Heuristic description (DEC-221),
    seeds DEC-134 (b). The band case the accepts names is written now,
    before score_move reads anything. SPRT pre-registered in
    adocs/data/S023_sprt.sh.

    Bench: <pending>

### Rebased onto `c3ed1ef` (2026-10-14, Opus subagent; phase 1, written blind)

**Superseded, kept as history**: S099 read H0 and was reverted as `5a04ffa`;
the section "Rebased onto `5a04ffa` (S099 reverted)" below is the live one,
and nothing this section says of S099's tree describes the tree S023 lands
on.

Written under DEC-260 while S099's SPRT holds every core: nothing compiled,
run, timed or tested; no python, no tool. Every expectation below is one.
Worktree `/home/max/ws/chesso-s023c`, branch `s023c` at `c3ed1ef` (S099's
candidate `8758622` plus its pinning commit, `src/` identical to `8758622`),
`git apply --3way` of `.tuning/coord/S023/s023_c3b7b5e.diff`. Nothing
committed, no ref moved. The index holds the merge (`apply --3way` stages
what it applies; the three resolved files were `git add`ed).

#### Deviations, first

1. **LQ02 sits on the capture margin's edge, and S023's term can push it
   off.** S261 re-mined the late-capture cases (DEC-281): Rxe4 now loses
   exactly 100 and is let through at reduced depth 2 by a margin of exactly
   100 -- zero room, where S269's Bxf7 had room (this file's "What phase 2
   should see"). The drive writes the capture table as it goes; one charge
   large enough for a term of -1 (an entry at or below -328 of 32767, i.e.
   two or more maluses at remaining depth 5 or so) on Rxe4's cell before the
   node reaches it skips it, and "a capture past late move pruning's count
   still takes the extra ply" goes red at `k >= 3`. Not a defect of S023:
   the golden is S261's, re-mined by `adocs/data/S261_mine_late_capture.py`
   on S023's candidate in phase 2. Marked PENDING at its site. The case's
   preconditions read S091's margin, not the record, and stay as they are.
2. **The DEC-266 timing is not owed**, against this file's "Pending, phase
   2" item 4. DEC-266 reads a behaviour-neutral speed verdict; this change
   alters play, so the cell quiescence reads per capture is priced by the
   SPRT, as S099's key was (DEC-271, S099's phase-2 item 14). The SPRT
   script says so. Optional, informing only.
3. **The reach census has no instrument.** Phase one wrote none (no counter
   in `src/`); the SPRT script now names it optional and informing only.
   Writing a tune-only counter in phase 2 is a new diff to review.
4. **`#include <algorithm>` added to `tests/test_search.cpp`**: S023's
   margin cases call `std::max` and the file used none before; libstdc++
   reaches it transitively, libc++ need not.
5. **`MANUAL.md`'s `ucinewgame` paragraph** names the capture table among
   the per-`go` tables -- S099 rewrote that paragraph, so a sentence about
   what is and is not reset now has a table it did not name.
6. **PENDING markers outside the phase-1 file list**: `tests/test_mate_carry.cpp`,
   `tests/test_mate_breadth.cpp`, `tests/test_engine.cpp`,
   `adocs/data/S170_cases.tsv`; `adocs/data/README.md` gains S023's section
   with the script's row. `touches:` amended; `decisions:` gains DEC-273.

#### Conflicts and resolutions

`apply --3way` merged `src/search.cpp`, `src/search_params.hpp`,
`src/data_structures.hpp`, `src/search.hpp`, `tests/test_search.cpp`,
`tests/test_evaluation.cpp`, `adocs/specs.md` and
`tools/mutants/S091_capture_see.py` without a textual conflict; each was
re-read hunk by hunk against S279's, S261's and S099's changes (below). Three
files conflicted; backups of the conflicted state are in
`.tuning/coord/S023/*.conflict.bak`.

- `DEV_MANUAL.md`, one region (the golden table's `capture_mates`,
  multicut, ProbCut, late-capture and `golden_defaults` rows): ours taken
  whole (S261's 11/12 depths and re-mined rows, the LATE_CAPTURE row, S099's
  75), then S023's two edits re-applied: the worst-record sentence into
  `capture_mates`, now marked PENDING phase 2, and `golden_defaults` to
  **79** with S023's four rows named and S099's 75 kept in its history.
- `MANUAL.md`, two regions: S261's `QsFutilityMargin` 226 and
  `SeeCaptureMaxLmrDepth` 9 rows kept, S023's `CaptHistQsWeight` and
  `CaptHistSeeWeight` rows below them.
- `tests/test_search_params.cpp`, four regions: the narrative keeps S261's
  and S099's paragraphs and appends S023's, "75 to 79"; the GOLDEN line
  **79**; the rows keep 226 and 9 and add S023's two beside them.
  `src/search_params.hpp` holds 79 `X(` rows, in the order the table lists.

Clean merges, read for intent:
- `src/search.cpp`: S279's `check_limits` at quiescence's top (line 1247)
  sits above every S023 line; S023's per-capture margin (1485-1487) is below
  the unchanged S130 substitution. S261's comment edits touch no S023
  comment. S099's corrected evaluation (1915-1936), its stores of `raw_eval`
  (2407, 3363) and its update beside the store (3373-3378) are after the
  move loop; S023's update is inside it, at the fail-high (3222-3233).
- `src/data_structures.hpp`: S023's constants and member after `cont_hist`
  (1096), S099's pointer after `proven_mate` (1116), the index at the end.
- `src/search_params.hpp`: S023's blocks after `ContHistWeight`,
  `QsFutilityMargin` (226) and `SeeCaptureMaxLmrDepth` (9); S099's after
  `RazorDepth`.
- `tests/test_search.cpp`: placements as on `c3b7b5e` -- the helper after
  `search_fen`, the suite before "search: quiescence", the two quiescence
  cases at that suite's end, the worst-record rows after `capture_mates`'
  loop, the guard block before S098 verdict 2's.
- Mutant anchors: every S023 single-line anchor, and S091's re-pointed C05
  and C06, occur exactly once in their file; CX11's and CX12's two-line
  anchors are adjacent once (search.cpp 3232-3233, 3301-3302). A grep of
  every mutant file's first anchor line over `src/`, `tools/` and `tests/`
  on `c3ed1ef` and on this tree differs only in S023's and S091's rows. CX
  is still unused elsewhere (`git log --all -p -- tools/mutants`).

#### Seeds against S261 and S099

None moves. `CaptHistBonus` 106 and `CaptHistMalus` 25 are shares of plain
history's grading (`HistoryBonus*`, `HistoryMalus*`, `QuietHistoryMax`,
depth 11): none of S261's thirty-one axes (`adocs/data/S261_sprt.sh`'s
table) is a history coefficient or bound, the continuation constants did not
move, and S099 touches no history code. `CaptHistQsWeight` 94 is a pawn of
`src/eval_tables.hpp` (unchanged), not a share of `QsFutilityMargin`, which
S261 moved 188 -> 226: the worst record now leaves 132 against 226 (was 94
against 188), the floor still never binds at the seed, and CX05's
equivalence argument holds with more room. `CaptHistSeeWeight` 100 is a pawn
of `see_value` (`src/bitboard.cpp` 1309, unchanged) and two reduced plies of
`SeeCaptureCoeff` 50 (unmoved); S261's cap 8 -> 9 extends the reader to
reduced depth 8, and `LmrBase` 52 -> 113 puts more candidates at reduced
depths 0 to 2, where the worst record's floor binds. No seed form reads
either moved value.

#### S099 interactions

- **No S023 rule reads `static_eval` or `raw_eval`.** The capture SEE term
  reads the cell and `lmr_depth` (search.cpp 2891-2896); the update reads
  `depth` (3232). Indirectly, `lmr_depth` takes `node_adjustment` (2615),
  which takes `improving` (2590), which reads the static stack -- since S099
  the corrected score (1936). That is S091's own dependence, unchanged here.
- **Quiescence futility reads the raw score.** Quiescence's `static_eval`
  is the entry's `eval` or `evaluate_lazy()` (1291-1294), the entry holding
  `raw_eval` since S099 (2407, 3363); `stand_pat` starts there (1314), may
  take a table bound (1335-1351), and S023's margin is added to it (1487).
  `corrected_static_eval()` (1119) is called from `negamax_at` alone.
- **No clearing is owed.** `capture_history` is a member of
  `search_state_t` (data_structures.hpp 1096), value-initialised with it
  for every search: `iterative_deepening_search()` builds one per `go`
  (chesso.cpp 1062), `bench` runs `reset_for_new_game()` and that function
  per position (2532, 2537), datagen's `run_search()` builds one per search
  (tools/datagen.cpp 123). S099's table needs `correction_reset()` because
  it lives outside that struct.

#### Goldens marked PENDING phase 2, each at its site with its script

Values kept as `c3ed1ef`'s. S023 has no relaxation allowance: DEC-272,
DEC-283 and DEC-284 are S099's alone, so a ceiling rise or floor drop stops
phase 2 for a ruling.

| golden | site | script |
|---|---|---|
| bench | the commit's `Bench:`, DEV_MANUAL.md's bench history | `chesso bench` |
| S170 budgets | `adocs/data/S170_cases.tsv` | `S203_case_sweep.sh` |
| carry ceilings 2, 5, 4, 0, 1, 2 | `tests/test_mate_carry.cpp` | `S203_case_sweep.sh --ceilings` over S202's, S261's, S099's and S023's grids |
| `EXACT_FLOOR` 163 | `tests/test_mate_breadth.cpp` | `S156_mined_floor_sweep.py --floor 163` |
| `MATE_IN_THREE_FLOOR` 11 | `tests/test_engine.cpp` | `S154_floor_margin_sweep.py floor`, `red` |
| `first_mate_depth` 9, 9 | `tests/test_engine.cpp` | `S188_repair_goldens.py first-mate` |
| `capture_mates` 9, 8, 11, 12 and labels | `tests/test_search.cpp`, DEV_MANUAL.md | `S230_mine_r01_row.py depths`, the seven sweeps |
| `mate_the_extra_ply_hides` 8 | `tests/test_search.cpp` | `S095_mine_mate_row.py`, `S230_mine_r01_row.py` |
| E21 row 10, B04 row 10 | `tests/test_search.cpp` | `S097_mine_mate_row.py`, `S277_mine_mate_row.py`, on a shrinking kill list |
| "ordering keeps the tree small" 65024 / 3251 | `tests/test_search.cpp` | `S192_node_budget.py` |
| LQ02 / LQ03 position and depths 3, 7 | `tests/test_search.cpp` | `S261_mine_late_capture.py` (deviation 1) |

Not goldens, measured drive depths S261 re-took, re-read only if their
preconditions go red: `CAP_DRIVE_DEPTH` 13, `NODE_TYPE_DEPTH` 7,
`SEE_MARGIN_DRIVE_DEPTH` 5, the ProbCut drives. S023's own drive depths,
`CAPT_RECORD_MIN_DEPTH` 2 and `CAPT_RECORD_MAX_DEPTH` 6, carry no golden:
their cases assert a count above zero over the range, now with S261's
`LATE_CAPTURE_POS` as the third position.

#### The SPRT script

`adocs/data/S023_sprt.sh`: CAND and REF stay `PIN_ME`. REF is S099's outcome
on achesso -- its closing commit on H1, the revert of `8758622` on H0 or no
verdict -- and the script refuses unless `git diff REF CAND^ -- src/` is
empty, S099's guard. The seeds paragraph records the re-read above; the
pre-game list names both possible parents' bench (6042933, 5636835), drops
the timing as not owed and makes the reach optional. DEC-171's line re-read
against `plan.md`'s Open list at `c3ed1ef`: none open; S110, S111 and S281
are conditional on S099's H1, S281 a reporting study. Bounds `{0, 5}`,
41861 / 25591 games, 19.8 h / 12.1 h, the 19.0 h cap, the abort rule and the
three outcomes unchanged. `bash -n` parses it.

#### Hunks that differ if S099 is reverted

`src/` applies unchanged: no S023 hunk touches a line S099 wrote. These do
not, and phase 2 rewrites them on the revert's base:
- `tests/test_search_params.cpp`: 77, not 79; S099's paragraph and two rows
  go, and S023's line reads "73 to 77" (phase one's own text).
- `DEV_MANUAL.md` `golden_defaults`: 77 "since S023", the S116 history
  as phase one wrote it.
- `MANUAL.md`: the `ucinewgame` clause re-applied to the paragraph as it
  stood before S099.
- The PENDING notes in `tests/test_mate_carry.cpp` (S261's 0, 5, 4, 0, 0, 1
  and its two grids), `tests/test_mate_breadth.cpp` (159) and
  `adocs/data/S170_cases.tsv` (its context line is S099's paragraph).
- `adocs/data/S023_sprt.sh`: REF the revert, parent bench 5636835.
- `adocs/data/README.md`: S023's section follows S099's, which goes.

Checked, not assumed: the saved diff applies with `git apply --cached
--check` to a scratch index of `c3ed1ef`; against one of `6fc7f23`, the tree
before S099, every `src/` hunk and every other test applies, and exactly the
seven files above refuse (DEV_MANUAL.md, MANUAL.md, README.md,
S170_cases.tsv, test_mate_breadth.cpp, test_mate_carry.cpp,
test_search_params.cpp).

#### Phase 2, in order, with machine time

Times are S099's recorded phase-2 walls (its "Phase 2 on `6fc7f23`") and
S261's, on this machine; `.moltke.local.md` measures match throughput only
(2110 games an hour at 8+0.08). Base first: H1's closing commit, or the
revert, with the hunks above rewritten. `export CLANG_FORMAT_MAJOR=22`.

1. Configure and build `build`, `build-tune`, `build-debug`: about 5 min
   (S099: 118 s).
2. AGENTS.md's TESTS command, both builds, format: about 5 min (S099: 258 s).
   Red in `test_mate_carry`, the floors or LQ02 is item 6-8's input; any
   other red stops phase 2.
3. Off values on the tree: the tune build at `CaptHistSeeWeight` 0 and
   `CaptHistQsWeight` 0, then at both shares 0, each printing the parent's
   `bench` stream (121 lines; the parent build is the main checkout's
   `.ref-builds/8758622`, `src/` equal to `c3ed1ef`, or `.ref-builds/bacead1`
   on the revert) and `search_bench` 9 and 12 the parent's: about 10 min.
4. `bench`, `bench 12`, `search_bench` 9 and 12 on the candidate: about
   5 min. Into the SPRT script.
5. DEC-266 timing: not owed (deviation 2). Reach census: optional, no
   instrument (deviation 3).
6. `S203_case_sweep.sh` into `adocs/data/S023_sweep_s170.txt`, the budgets
   by the rule, `--ceilings` over four grids: about 10 min.
7. The floors and first-mate (S154 `floor` and `red`, S156 `--floor 163`,
   S188 `first-mate`): about 15 min.
8. `tests/test_search.cpp` goldens: the seven `capture_mates` sweeps and
   `S192_node_budget.py`, about 20 min; LQ02/LQ03 re-mined only if red, E21
   and B04 only if their kill lists shrink in item 10 -- up to 1.5 h more.
9. The gate again with every golden in, both builds: about 5 min.
10. Mutants, `tools/mutation_check.py` on a whole-tree fixture with S280's
    `protocol.file.allow=always`: `validate()`, then CX01-CX08 and CX10-CX12
    (CX05 expected equivalent), S091's C05 and C06 on the new anchors, S269's
    LQ02 and LQ03, S112's F01-F06 -- 21 mutants at about 130 s each (S099
    1916 s for 15, S261 1288 s for 10): about 45 min.
11. Debug self-play, four rounds at 4+0.04, grep `Assertion`: about 10 min.
12. `tools/gate_extra.sh`, five stages: about 17 min (S099 1019 s, S261
    983 s).
13. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`:
    seconds.

Items 1 to 13: **about 2.5 hours** without the contingent re-mines, about 4
with them, so a day's work (DEC-155) once S099's verdict frees the machine.
Then the SPRT: **12.1 h** on a bound, **19.0 h** at the cap -- a night; a
1000-game A/A (about 28 min) first if the harness moved (DEC-143).

#### What phase 2's compiler and tests should say (expectations)

- `src/` and `tests/` compile clean in both builds under `-Wall -Wextra
  -Werror`: no new symbol from S261 or S099 is used by S023, no S023 name
  collides, and every added line in `src/` and `tests/` outside
  `search_params.hpp`'s format-off block is at most 80 columns. The format
  check may still reflow a wrapped call (never run); re-grep the CX and C05/C06
  anchors after any reflow of `src/search.cpp`.
- `test_search_params`: green both builds, count 79; the tune-only floor case
  reads 0 at both tops and 2226 / 900 at the best record.
- `test_evaluation`: the band case green; it reads no tree.
- `test_search`: the "capture history" suite green (pure arithmetic and
  direct updates, now over reduced depths 0 to 8); the two quiescence cases
  green at 132 and 320 against 226; the two guard cases expected green,
  unverified -- `LATE_CAPTURE_POS` is S261's now; the worst-record mate rows
  expected green, unverified, a red there being the hazard and a finding.
  **LQ02 may go red** (deviation 1); a golden's input, not a relaxation.
- `test_mate_carry`, `test_mate_breadth`, `test_engine`'s floors and
  first-mate, "ordering keeps the tree small": may move; inputs to items 6-8.
- Everything else as on `c3ed1ef`: green.

### Rebased onto `5a04ffa` (S099 reverted) (2026-10-14, Opus subagent; phase 1b, written blind)

S099 read H0 (`74bb9ad`: nElo -4.71 +/- 6.84, LLR -2.96, 9916 games) and
`8758622` was reverted whole as `5a04ffa`, whose `src/` is `bacead1`'s
(checked: `git diff bacead1 5a04ffa -- src/` is empty) and whose tests,
tools, MANUAL.md, DEV_MANUAL.md, specs.md and S170_cases.tsv are `6fc7f23`'s.
Written under DEC-260 while S099's 32+0.32 estimate holds every core: nothing
compiled, run, timed or tested; no python, no tool. Worktree
`/home/max/ws/chesso-s023d`, branch `s023d` at `5a04ffa`, `git apply --3way`
of `.tuning/coord/S023/s023_on_c3ed1ef.diff`. Nothing committed, no ref
moved; the index holds the merge, the seven resolved files `git add`ed.

#### Deviations, first

1. **`test_mate_carry`'s GOLDEN line was stale on this base and is
   corrected.** At `bacead1` (and so at `5a04ffa`) it read "6, 15, 5, 2, 11
   and 5", S116's six, against the function's 0, 5, 4, 0, 0, 1; S099 had
   corrected it and the revert put the stale text back. The line now names
   the shipped six and its history says how it went stale (S202's walk,
   DEC-251; S261's F, DEC-282). Comment only; no number a test reads moves.
2. **The `c3ed1ef` section above is kept as history and marked
   superseded**, as is the phase-1 "Files shared with S099's diff"; neither
   describes the tree S023 lands on. Everything live -- code, tests,
   manuals, the SPRT script, the README row, this section -- names no part
   of S099 as present.
3. Carried from the `c3ed1ef` section and still standing: LQ02 at the
   margin's edge (deviation 1 there; the coordinator's ruling: re-mined by
   `adocs/data/S261_mine_late_capture.py` if red, only "no cell found"
   stops); no DEC-266 timing owed; the reach census optional with no
   instrument; `#include <algorithm>` in `tests/test_search.cpp`; MANUAL.md's
   `ucinewgame` paragraph naming the capture table among the per-`go` tables
   (now the plain sentence `bacead1` has, with one clause added).

#### Conflicts and resolutions

Exactly the seven files the `c3ed1ef` section predicted refused a clean
apply; every `src/` hunk, `tools/` hunk, `test_search.cpp`,
`test_evaluation.cpp`, `test_engine.cpp` and specs.md applied cleanly, and
their hunks are byte-identical to the `c3ed1ef` merge's but for the
`c3ed1ef's` -> `bacead1's` wording in the PENDING notes (diffed with the
`@@` lines stripped). Backups of the conflicted state:
`.tuning/coord/S023/revert_conflicts/*.bak`.

- `tests/test_search_params.cpp`: S099's paragraph and rows gone with the
  base; S023's paragraph reads "The count goes 73 to 77", the GOLDEN line
  **77**, the four rows beside 226 and 9. `src/search_params.hpp` holds 77
  `X(` rows and the golden table 77, in the same order.
- `DEV_MANUAL.md` `golden_defaults`: the base row (stale at 71) taken, the
  head set to **77** and phase one's tail restored -- S116's two make 73,
  "this row read 71 from S116's landing until S023, stale", S023's four
  make 77. The `capture_mates` row merged clean with its PENDING sentence.
- `MANUAL.md` `ucinewgame`: the base paragraph (no pawn correction table)
  with one clause: the capture history table is among the per-`go` tables.
  The option-table rows merged clean.
- `tests/test_mate_carry.cpp`: PENDING note over S202's and S261's grids
  plus S023's, the six `bacead1`'s; the GOLDEN line corrected (deviation 1).
- `tests/test_mate_breadth.cpp`: PENDING note at 159.
- `adocs/data/S170_cases.tsv`: S099's paragraph dropped with the base, the
  PENDING note after S261's, budgets "bacead1's".
- `adocs/data/README.md`: S099's data rows stay (the files stay, evidence);
  S023's section after them, the row naming REF as S099's completing commit
  on `5a04ffa`.

Anchors: every S023 mutant anchor and S091's re-pointed C05/C06 occur once
(CX11 at search.cpp 3064-3065, CX12 at 3133-3134, adjacent);
`tools/mutants/S234_tt_estimate_margins.py` is `6fc7f23`'s byte for byte
and its four anchors occur once; S099's two mutant files are gone with the
base. The grep of every mutant file's first anchor line over this tree
differs from `5a04ffa`'s only in S023's and S091's rows.

#### What left with S099

- The pawn key, the correction table, `CorrHistMax` and `CorrHistWeightCap`
  (the count is 77, not 79), S099's tests, mutants and census.
- **The interactions, all three now "none: S099 reverted".** No corrected
  static evaluation exists: `negamax_at`'s `static_eval` is `evaluate()`'s
  (search.cpp 1761) and feeds the `improving` stack (1772) that S091's
  `lmr_depth` reads through `node_adjustment` (2422, 2447), as before S099.
  Quiescence reads the raw score as always (1144, 1167; S023's margin at
  1338-1340). No clearing is owed: `capture_history` is a member of
  `search_state_t` (data_structures.hpp 1003), built fresh per `go`
  (chesso.cpp 1049), per `bench` position (2506, 2511) and per datagen
  search (tools/datagen.cpp 121); there is no other table to clear beside it.
- S099's goldens and its three allowances: carry ceilings 0, 5, 4, 0, 0, 1
  (F by DEC-282), budgets A 1000000, B 100000, C 2000000, D 2000000, E
  300000, F 100000 at stride 2, `EXACT_FLOOR` 159, `MATE_IN_THREE_FLOOR` 11,
  first-mate 9 and 9, bench 5636835. DEC-272, DEC-283 and DEC-284 lapsed;
  S023 has no relaxation allowance.
- Seeds: unchanged by the revert -- S099 never touched the history
  machinery, and S261's values stand.

#### Goldens marked PENDING phase 2, each at its site with its script

Values kept as `bacead1`'s. A ceiling rise or a floor drop stops phase 2 for
a ruling.

| golden | value | site | script |
|---|---|---|---|
| bench | 5636835 | the commit's `Bench:`, DEV_MANUAL.md's bench history | `chesso bench` |
| S170 budgets | as above | `adocs/data/S170_cases.tsv` | `S203_case_sweep.sh` |
| carry ceilings | 0, 5, 4, 0, 0, 1 | `tests/test_mate_carry.cpp` | `--ceilings` over S202's, S261's and S023's grids |
| `EXACT_FLOOR` | 159 | `tests/test_mate_breadth.cpp` | `S156_mined_floor_sweep.py --floor 159` |
| `MATE_IN_THREE_FLOOR` | 11 | `tests/test_engine.cpp` | `S154_floor_margin_sweep.py floor`, `red` |
| `first_mate_depth` | 9, 9 | `tests/test_engine.cpp` | `S188_repair_goldens.py first-mate` |
| `capture_mates` | 9, 8, 11, 12 and labels | `tests/test_search.cpp`, DEV_MANUAL.md | `S230_mine_r01_row.py depths`, seven sweeps |
| `mate_the_extra_ply_hides` | 8 | `tests/test_search.cpp` | `S095_mine_mate_row.py`, `S230_mine_r01_row.py` |
| E21, B04 rows | 10, 10 | `tests/test_search.cpp` | `S097_mine_mate_row.py`, `S277_mine_mate_row.py`, on a shrinking kill list |
| ordering band | 65024 / 3251 | `tests/test_search.cpp` | `S192_node_budget.py` |
| LQ02 / LQ03 | S261's position, 3 and 7 | `tests/test_search.cpp` | `S261_mine_late_capture.py`, if red |

Drive depths that are not goldens (`CAP_DRIVE_DEPTH` 13, `NODE_TYPE_DEPTH`
7, `SEE_MARGIN_DRIVE_DEPTH` 5, the ProbCut drives, S023's own 2 to 6) are
re-read only if their preconditions go red.

#### The SPRT script

`adocs/data/S023_sprt.sh`: CAND and REF stay `PIN_ME`; REF is S099's
completing commit, docs-only on `5a04ffa`, and the guard refusing unless
`git diff REF CAND^ -- src/` is empty stays. The seeds paragraph is re-read
on `5a04ffa`; the pre-game list names the parent's bench 5636835 and its
`search_bench` (43496 / 160453 / 30816 at 9, 180917 / 509438 / 147594 at
12, DEV_MANUAL.md's S261 entry). DEC-171's line re-read against `plan.md`'s
Open list at `5a04ffa`: none -- S099 open only for its estimate, S110 and
S111 in the reserve, S281 void. Bounds `{0, 5}`, 41861 / 25591 games,
19.8 h / 12.1 h, the 19.0 h cap, the abort rule and the three outcomes
unchanged. `bash -n` parses it.

#### Phase 2, in order, with machine time

The `c3ed1ef` section's list, with this base. The parent build is the main
checkout's `.ref-builds/bacead1/build` (`src/` equal to `5a04ffa`'s).
`export CLANG_FORMAT_MAJOR=22`. Walls are S099's and S261's recorded ones.

1. Configure and build `build`, `build-tune`, `build-debug`: about 5 min.
2. AGENTS.md's TESTS command, both builds, format: about 5 min. Red in
   `test_mate_carry`, the floors or LQ02 feeds items 6-8; any other red stops.
3. Off values: the tune build at both weights 0, then both shares 0, each
   printing the parent's `bench` stream (5636835, 121 lines) and its
   `search_bench` at 9 and 12: about 10 min.
4. `bench`, `bench 12`, `search_bench` 9 and 12 on the candidate: about 5 min.
5. DEC-266 timing not owed; reach census optional, no instrument.
6. `S203_case_sweep.sh` into `adocs/data/S023_sweep_s170.txt`, the budgets
   by the rule, `--ceilings` over S202's, S261's and S023's grids: about
   10 min.
7. Floors and first-mate (S154 `floor` and `red`, S156 `--floor 159`, S188
   `first-mate`): about 15 min.
8. `capture_mates`' seven sweeps and `S192_node_budget.py`: about 20 min;
   LQ02/LQ03 re-mined if red, E21/B04 if their kill lists shrink: up to 1.5 h.
9. The gate again with every golden in: about 5 min.
10. Mutants on a whole-tree fixture: `validate()`, CX01-CX08 and CX10-CX12
    (CX05 expected equivalent), C05 and C06, LQ02 and LQ03, F01-F06 -- 21 at
    about 130 s: about 45 min.
11. Debug self-play, four rounds at 4+0.04, grep `Assertion`: about 10 min.
12. `tools/gate_extra.sh`: about 17 min.
13. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`:
    seconds.

**About 2.5 hours** without the contingent re-mines, about 4 with them: a
day's work once S099's estimate frees the machine (DEC-155). Then the SPRT:
**12.1 h** on a bound, **19.0 h** at the cap, a night; a 1000-game A/A
(about 28 min) first if the harness moved (DEC-143).

#### What phase 2 should say (expectations)

- `src/` and `tests/` compile clean in both builds under `-Wall -Wextra
  -Werror`: the merged `src/` is the `c3ed1ef` merge's S023 hunks on
  `bacead1`'s code, which uses nothing S099 added; no new line in `src/` or
  `tests/` outside the format-off block passes 80 columns. A reflow by the
  format check means re-grepping the CX and C05/C06 anchors.
- `test_search_params`: green both builds at **77**; the tune-only floor case
  reads 0 at both tops and 2226 / 900 at the best record.
- `test_evaluation`: the band case green.
- `test_search`: the capture-history suite green; the quiescence cases green
  at 132 and 320 against 226; the two guard cases and the worst-record mate
  rows expected green, unverified; **LQ02 may go red**, re-mined by script.
- `test_mate_carry`, `test_mate_breadth`, `test_engine`'s floors and
  first-mate, the ordering band: may move; inputs to items 6-8.
- Everything else as on `5a04ffa`: green.

### Phase 2 on `7e36271` (2026-10-14, Opus subagent) -- STOPPED at item 2

`s023d` fast-forwarded `5a04ffa` -> `7e36271` (S099's docs-only completing
commit): backup `.tuning/coord/S023/s023_pre_ff.diff`, `git stash`, `merge
--ff-only`, then `stash pop --index` refused on the expected
`adocs/data/README.md` conflict, so `git stash apply` without `--index`,
README resolved keeping both sides (S099's `S099_ltc.log` row, then S023's
section), every path `git add`ed, and the content checked equal to the
stash's against `5a04ffa` before the stash entry was dropped. Logs and
scripts: `.tuning/coord/S023/p2/`.

#### The stop, first

**`test_search` is red in both builds on S023's own worst-record mate guard**
-- "pruning does not hide a forced mate", the loop that searches each
`capture_mates` row again with `-CAPT_HIST_BOUND` planted in every cell:

    FATAL ERROR: REQUIRE( result.mate_found ) is NOT correct!
    logged: capture mate, worst record, 3N1bk1/3Q3p/6p1/p3Bp1n/1p6/3P1P1P/1q5K/8 w - - 0 33, depth 11

The same row with an empty table (the first loop) is green at 11. The phase-1
text called this exact red "the hazard and a finding, never a re-pin", and
the brief stops phase 2 on any red that is not a PENDING golden, so nothing
after item 2 ran and nothing was edited to make it pass.

**What it is, measured** (`p2/diag/`: `probe.cpp`, S230's driver -- the test's
`search_fen` line for line -- with every cell planted before each search,
linked against this tree's Release and tune libraries; depths 3..12, then
13..16; `-` no mate, otherwise its distance):

| row | record | 3 .. 12 | 13 .. 16 |
|---|---|---|---|
| 3 (mate in 4, depth 11) | empty | `- - - - - - - - 4 4` | `4 4 4 4` |
| 3 | worst, both weights | `- - - - - - - - - -` | `4 4 4 4` |
| 3 | worst, `CaptHistSeeWeight` 0 (quiescence term only) | `- - - - - - - 4 4 4` | |
| 3 | worst, `CaptHistQsWeight` 0 (SEE term only) | `- - - - - - - - 4 4` | |
| 3 | best, both weights | `- - - - - - 4 - - 4` | `4 4 4 4` |
| 4 (mate in 5, depth 12) | empty | `- - - - - - - - - 5` | |
| 4 | worst, both weights | `- - - - - - - - - 5` | `5 - 5 5` |
| 4 | worst, SEE term only | `- - - - - - - - - -` | `- 5 5 5` |
| 4 | best, both weights | `- - - - - - 5 - 5 -` | `5 - 5 5` |

Rows 1 and 2 keep their profiles under every setting. The tune build at an
empty table reproduces the Release profile. Read plainly: the worst record
delays row 3's mate by two plies (11 -> 13), not out of reach; neither term
alone does it (the quiescence term alone finds it a ply *earlier*, at 10);
the SEE term alone delays row 4 two plies (12 -> 14); and the **best** record
-- which only widens both margins, so it prunes strictly less in both rules
-- loses row 3 at 10 and 11 and row 4 at 12 as well. The fixed-depth mate
profiles of these rows move under any large perturbation of the tree, in
both directions of the record. That is a reading, not a ruling: whether
this is the hazard or a guard that measures tree shape is the question in
the report.

#### Readings before the stop

- **Builds**: Release, tune, Debug configured and built, no warning, 116 s.
- **Gate 1** (`p2/gate1.log`, 219 s): 45 of 46 in each build, red only
  `test_search`, the one assertion above (217 cases, 1 failed). Green on
  this tree at `bacead1`'s values, before any re-derivation:
  `test_mate_carry` (36 s), `test_mate_breadth`, `test_engine`'s floors and
  first-mate, LQ02 and LQ03, S023's two guard cases, the two quiescence
  cases, the capture-history suite, `test_search_params` at 77. "Ordering
  keeps the tree small" reads **13446** (band 3251 to 65024).
- **Format**: red on pure reflows of S023's own lines (18 sites in
  `src/search.cpp`, `test_evaluation.cpp`, `test_search.cpp`,
  `test_search_params.cpp`; `capture_record_term` onto one line, call and
  initialiser wraps). Applied with `clang-format-22 -i`; `./clang-format.sh
  --check` clean after. No anchor moved: S023's and S091's anchors still
  occur once each, CX11 at search.cpp 3062-3063, CX12 at 3131-3132.
- Not run: bench and the off-value identity, the sweep, the floors, the
  re-mines, mutants, Debug self-play, `gate_extra`, the prose checks, the
  SPRT script's numbers.

#### Noted on the way, not S023's

`adocs/data/S192_node_budget.py`'s middle-half test cannot be met by a band it
derives: from a count `c` the floor is `c / 5` and the budget `4c`, so the
middle half starts at `c / 5 + (4c - c / 5) / 4 = 1.15c`, above `c`. A freshly
re-derived band therefore reads its own count OUTSIDE, as S091's and S231's
both did. Reachable only in the gate's reading, so DEC-171's filler class.
(Opened by the coordinator as S282.)

### Phase 2 resumed under DEC-285 (2026-10-14, Opus subagent) -- STOPPED at item 6

DEC-285, the owner's ruling, option B: only S023's own worst-record guard is
restated -- a row passes when a mate at its own distance is found at some
depth from the row's depth to two plies past it, each from a cold table;
nothing else relaxes. The profiles that ruling was taken on are
`.tuning/coord/S023/p2/diag/` (`probe.cpp`; `rel_*.txt`, `tune_*.txt` at
depths 3..12, `deep_*.txt` at 13..16), tabled in the stopped section above.

#### The stop, first

**The carry ceilings rise on S023's tree**: `adocs/data/S203_case_sweep.sh
--ceilings adocs/data/S202_sweep_s170.txt adocs/data/S261_sweep_s170.txt
adocs/data/S023_sweep_s170.txt` answers **0, 5, 5, 0, 1, 1** against the
shipped 0, 5, 4, 0, 0, 1: **C 4 -> 5** (S023's grid, stride 1, the 1000000
cell: 5 mate lines, all 5 short; its 2000000 cell 6 lines, 4 short) and **E
0 -> 1** (the 2000000 cell: 32 lines, 1 short). DEC-285 item 3 and the
brief stop phase 2 on any ceiling rise, so nothing was applied: the
ceilings, the budgets and every golden in the tree are still `bacead1`'s.
The family reading beside it (DEC-277's F1 form, not a decision): S023's grid
reports **1402 mate lines and 10 short** over its 108 cells, against S261's
1485 and 16 on the parent's engine.

What the rule would install if the rises are accepted (`p2/pick_budgets.sh`
over the new grid, never typed): A `nodes 1000000 -> 500000` (1 line), C
`2000000 -> 500000` (2 lines), D `2000000 -> 1500000` (1 line), E `300000 ->
100000` (4 lines), none short there; B 100000 (22) and F 100000 at stride 2
(5) unchanged. The grid is committed-ready as `adocs/data/S023_sweep_s170.txt`.

#### Readings, the candidate tree (`7e36271` plus S023, fixture `0e2a31f`)

- **The window guard (DEC-285)**: written in `tests/test_search.cpp`
  "pruning does not hide a forced mate" with `WORST_RECORD_WINDOW` 2,
  DEC-285 cited at the site; DEV_MANUAL.md's `capture_mates` sentence says
  the same. Gate 2 (`p2/gate2.log`, 287 s): `test_search` green in both
  builds, every test green but `test_plan_citation_freshness`, red on this
  file's own quoted log line carrying `file:line` (DEC-135); the quote now
  drops it, and `plan_prose_check.py --citations` reads 0 flagged. The
  diag profiles say the four rows land at depths 9, 8, 13 and 12 inside
  their windows; no row needs the window's last ply but row 3.
- **Bench** (`p2/bench.log`): `bench` **5636835 -> 7045606** (+25.0 %), the
  eight replies unchanged (`c3d5 e2a6 d7c8q g7h8q a7a6 a1b2 e5e6 e5e6`);
  `bench 12` 1937969 -> 2314722, the second reply d5e6 -> e2a6.
  `tools/search_bench.py` depth 9 43496 / 160453 / 30816 -> 43477 / 151552 /
  30816, best moves `c3d5 e2a6 d7c8q` on both; depth 12 180917 / 509438 /
  147594 -> 282734 / 465448 / 236166, kiwipete d5e6 -> e2a6.
- **The off values (DEC-215)**: the tune build at `CaptHistSeeWeight` 0 and
  `CaptHistQsWeight` 0 prints the parent's `bench` stream, 0 of 121 lines
  differing, total 5636835; so does it at `CaptHistBonus` 0 and
  `CaptHistMalus` 0. The tune build at its defaults prints the Release
  candidate's stream, 0 lines differing. The parent is the main checkout's
  `.ref-builds/bacead1/build`, whose `src/` is `7e36271`'s.
- **`MATE_IN_THREE_FLOOR`** (`p2/floors/S154_floor.txt`, `S154_red.txt`):
  12 of 24 shipping, 6 weakened, the gate built at `RfpMinPly` 1 red
  (`REQUIRE( 6 >= 11 )`): **11 unmoved**, as at S261.
- **`EXACT_FLOOR`** (`p2/floors/S156.txt`, `--floor 159`): **161** shipping,
  **156** weakened, the gate at `RfpMinPly` 1 red at 159. The shipping end
  moved 160 -> 161; 159 still separates (gap 5). The golden's own margin, "1
  below the shipping count", reads 160 -- a rise, which tightens -- and is
  not applied while phase 2 is stopped; with it, S156 at `--floor 160` is
  owed to observe the gate red.
- **First-mate** (`p2/floors/first_mate.txt`): 9 and 9, mate 5: unmoved.
- **`capture_mates`** (`p2/capmates/`, the seven sweeps): the shipped
  profiles are S261's (`d9-d12`, `d8-d12`, `d11 d12`, `d12`); row 3 is lost
  at 11 by C02, C05 and R02, rows 1, 2 and 4 by no S091 mutant: **9, 8, 11,
  12 and every label unmoved**.
- **"Ordering keeps the tree small"** (`p2/s192_band.txt`): 13446 nodes,
  inside the band 3251 to 65024 (0.83 of the 16256 it was derived from).
  The script's own middle-half verdict reads OUTSIDE, which is S282's open
  defect -- it reads every freshly derived count so -- and re-deriving by
  its ratios would drop the floor 3251 -> 2689, a loosening; the pair is
  kept, and S282's accepts keep it too.
- **LQ02 and LQ03**: green in both gates on S261's rows; no re-mine owed.
- Not run: the mutation pass (and DEC-285 item 4, the window guard's kills),
  the gate with goldens in, Debug self-play, `gate_extra`, the prose checks
  but `--citations`.

#### The SPRT script

REF `7e36271`, CAND `PIN_ME_CAND`, the `src/` guard kept; DEC-171's line
re-read at `bf7e8e8`: S282 open as filler, named; S283 a run. "What the tree
does" waits for the stop's answer.

### Phase 2 resumed under DEC-287 (2026-10-14, Opus subagent) -- STOPPED on the E21 row

#### The stop, first

**`mate_the_multicut_hides` (the E21 row) no longer separates on S023's tree,
and its re-mine finds no cell in the range.** S230's driver at depths 7..12
(`p2/eb/`): the row, `6k1/5pp1/6np/1PP1p3/1Q2P1P1/3r1q1P/1B1R4/6K1 b - - 0 42`,
reads its mate in 5 at 9 to 12 on the shipped library and at 9 to 12 with
`E21_multicut_mate_band_gate_dropped` applied, so nothing separates at its
depth 10 (at S261 the shipped build read `d8 d9 d10` against E21's `d8 d9`).
The case stays green; only the separation went. Re-mined by S261's procedure
(`p2/remine_e21.sh`: `S277_mine_mate_row.py fens`, 269 candidates;
`S097_mine_mate_row.py sweep` shipped and with E21 applied over its own range
`SeMinDepth` + 1 .. + 4, 7..10): **0 of 269 separate** -- "no cell found",
which the coordinator's re-mine ruling makes a stop. Nothing was relaxed or
replaced; the row and its depth are S261's.

For the ruling, and not a re-mine: the same 269 at 11..14 (S259's range,
`p2/e21_wide.sh`, `p2/remine/wide_*.txt`) give **6 separators** --
`1q1k1r2/4p2p/p3p2b/6p1/3P4/4PB2/PPr5/R1B1K1R1 b - - 1 31`,
`2r4r/kq3pb1/N3p1p1/QPp1Pn1p/2PPRP2/7P/5BP1/R5K1 w - - 1 31` (S022's row),
`5r2/1R6/p2p4/P1pPk2p/B1P1p1pP/8/1P2r3/6K1 b - - 0 58`,
`7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42` (S131's row),
`1r5k/2p1n1pp/3Q1r2/p2n1p2/2BPp3/2B1P2q/1P3P1P/2R1R1K1 b - - 3 23` and
`1R6/8/2p3p1/P5P1/1p2b2P/4k3/6pK/8 b - - 1 54` (S188's row) -- on which the
guard-mode witness and pick have not been run. E21 itself is still killed
by S243's direct case, "the multicut never ends a node on a mate from its
verification", which reads no row (DEV_MANUAL.md's E21 row says so).

#### Done before the stop, under DEC-287 and the brief

- **Carry ceilings and budgets applied** in the candidate:
  `short_line_ceiling()` C 4 -> 5 and E 0 -> 1, DEC-287 cited in the
  function, the GOLDEN line, the table and a paragraph in DEC-284's form;
  S023's grid added to `--ceilings`' command at both sites and in
  DEV_MANUAL.md's row (whose command had also omitted S261's grid, now in);
  `adocs/data/S170_cases.tsv` takes the rule's A 500000, C 500000, D 1500000,
  E 100000 by `pick_budgets.sh`, B and F unchanged, with S023's paragraph.
- **`EXACT_FLOOR` 159 -> 160**, a tightening: S156 at `--floor 160` on the
  re-made fixture `3ede587` reads 161 shipping, 156 weakened, the gate at
  `RfpMinPly` 1 red at 160 (`p2/floors/S156_160.txt`). Test comment,
  DEV_MANUAL.md's row and command, MANUAL.md's `RfpMinPly` breadth numbers.
- **`mate_the_extra_ply_hides` re-mined** (DEC-142): the old row had stopped
  separating at 8 already on the parent, S261's engine (its guard-opened
  build reports the mate at 8 too). S095's procedure as S234 ran it over the
  141 candidates, 25 separators, `pick` takes
  `k7/6Q1/3K4/pp6/1P6/r2p4/8/8 w - - 2 80` at 8, mate in 4, shipped `d7-d12`
  against the opened build's `d7 d9-d12`; the candidates file's oracle line
  is `Kc6 Rc3+ Qxc3 d2 Qg7 d1=R Qb7#`. Observed red with the guard opened and
  green with it in place, fixture `187fa29`. Evidence
  `adocs/data/S023_remine_extra_ply.log`.
- **B04's row still separates** at its depth 10 (`p2/eb/`); its marker
  replaced. The ordering band, the capture-mate table, the first-mate depths
  and `MATE_IN_THREE_FLOOR` markers replaced by their readings (above).
- Not run: the mutation pass, the gate with goldens in, Debug self-play,
  `gate_extra`, the prose modes but `--citations`.

### Phase 2 completed under DEC-288 (2026-10-14, Opus subagent)

#### Deviations, first

1. **The E21 re-mine ran S097's miner where DEC-288 names S277's.**
   `adocs/data/S277_mine_mate_row.py` is the ProbCut miner: it imports S113's
   and re-points S097's driver from `SeMultiCut` to `ProbCut`, so its firing
   witness toggles ProbCut, not the multicut E21 guards. S261's and S259's
   E21 re-mines used S277 for stage 1 (`fens`) and `S097_mine_mate_row.py`
   for stages 2 to 6, and this one does the same; S277's `fires` and `pick`
   in guard mode were also run on the same files and take **the same row**,
   so the choice moves nothing. Both outputs are in the log.
2. **DEC-285 item 4 is met vacuously, and here is the reading.** No CX
   mutant was ever named for the worst-record guard -- neither the mutant
   file nor this file's phase 1 lists one, the guard having been written as
   a hazard sample -- and no CX mutant survives: 10 of 10 killed, CX05
   equivalent as declared. The window guard itself goes red under none of
   them. Where a CX mutant reaches "pruning does not hide a forced mate"
   (CX06, CX07, CX10) it is the empty-table rows' fatal `REQUIRE` that fires
   first, so the window loop is not reached; where it does not (CX01 to
   CX05, CX08, CX11, CX12), the window loop runs green. Nothing survives
   because of the window, so nothing stopped.
3. **Two mined rows were re-mined, not one**: the extra-ply row
   (DEC-142, DEC-288), which had stopped separating already on the parent,
   and the E21 row, one band deeper by DEC-288. Both are observed red under
   their weakened builds and green shipped.

#### The E21 row, DEC-288

S261's row stopped separating on S023's tree (shipped and E21-applied both
read its mate in 5 at 9 to 12); 0 of the 269 candidates separate in 7..10.
At 11..14 six do; the guard-mode witness on E21's own tune library fires on
all six; `pick --mode guard --lo 11` takes
`1r5k/2p1n1pp/3Q1r2/p2n1p2/2BPp3/2B1P2q/1P3P1P/2R1R1K1 b - - 3 23` at
**depth 12, mate in 4**, shipped `d11 d12 d13 d14` against the guard-dropped
build's `d11 d13 d14`, the mutant's multicut firing at 12, 13 and 14,
2903700 nodes, about 0.75 s. Stockfish at depth 20 through python-chess:
mate in 4 for Black, the side to move, its line ending in checkmate. Green
shipped, red with E21 applied (`mate the multicut hides, depth 12`, fatal
`REQUIRE( result.mate_found )`), on fixture `44fcb8a`. In the mutation pass
E21's kill list is this row and S243's direct case, two witnesses again.
Evidence `adocs/data/S023_remine_e21.log`. B04's row still separates at its
depth 10 (`- - 5 5 5 5` shipped against `- - 5 - 5 5` under B04, depths
7..12), so nothing was owed there; B04 killed by its row and its direct case.

#### Readings, the final tree (fixture `18bb267`)

- **Gate 3** (`p2/gate3.log`, 241 s): **46 of 46 in both builds**, the
  format check clean, with every golden in: ceilings 0, 5, 5, 0, 1, 1 and
  the new budgets (`test_mate_carry` 16.5 s, from 36 s), `EXACT_FLOOR` 160,
  both re-mined rows; and again on the final text, gate 4, 46 of 46 in both
  builds, format clean, 180 s.
- **Mutation pass** (`p2/mutants.log`, `p2/mutation/results.tsv`, 2474 s,
  DEV_MANUAL.md's recipe with S280's line, the fixture's own
  `tools/mutants/`): **22 of 22 killed, 1 equivalent** -- CX01-CX04,
  CX06-CX08, CX10-CX12 killed and CX05 equivalent as declared; S091's C05
  and C06 on their re-pointed anchors; S269's LQ02 and LQ03; S112's F01-F06;
  E21 and B04. Every kill is by the case its `Mutation:` line names (CX01:
  the margin arithmetic and the pruning-node record case; CX02: the
  arithmetic and "no capture that holds its material is skipped whatever its
  record"; CX03: the pruning-node case; CX04 and CX06: the two quiescence
  cases; CX07 and CX08: the direct update case and the census; CX10: the
  index case; CX11 and CX12: the quiet-cutoff node case), some with a mate
  row or `test_mate_carry` beside it.
- **Debug self-play** (DEC-141, `p2/selfplay.log`): four rounds at 4+0.04,
  DEV_MANUAL.md's command, the Debug build of the final tree: **8 games**,
  all normal terminations, **0 `Assertion`**, 0 disconnects, 0 time
  forfeits, 1326 `bestmove` lines in the trace log.
- **`tools/gate_extra.sh`** (`p2/gate_extra/`, JOBS=12): **all five stages
  ok in 929 s** -- prose, citations, debug (354 s), sanitize (525 s, 46 of 46,
  the sanitizer build's `bench` 7045606, the Release total, "INV-6 across
  builds: both 7045606 nodes"), perft (50 s).
- **Prose checks**, `tools/plan_prose_check.py`, one mode a call: `--prose`
  0 sentences flagged (and 0 over this file and the SPRT script given by
  name), `--citations` 0 flagged over 28 files, `--touches` 0 flagged,
  `--params` exit 0, `--gate` agreeing in both documents.

#### Fast check (2026-10-15): three findings, fixed

The fast check over `s023_phase2.diff` found `src/` clean and three real
problems; the coordinator's brief carried them and a ruling on the first.

1. **The H0 row kept a band case that could not build.** The SPRT script's
   H0 row said the band case in `tests/test_evaluation.cpp` "stays" while the
   revert removes `capture_history_entry` and `CAPT_HIST_BOUND`, which it
   reads; reverted whole instead, it reopens
   `2026-08-13_plan_review.2-F06`. **Fixed by the coordinator's ruling**:
   on H0 or no verdict the case is kept without its record loop, a written
   edit in the revert commit. `adocs/data/S023_h0_band_case.diff` is that
   edit against `7e36271`'s file -- the lines that go are the comment
   paragraph on the worst, no and best record, the `int16_t& record =
   capture_history_entry(...)` binding, the `for (const int planted : ...)`
   loop with its `record = ...` assignment and closing brace; one
   comparison of the king's capture against the killer stays. Built on a
   scratch worktree of `7e36271` and run: `CHECK( 900100 > 900000 )`,
   test_evaluation 32 of 32 (`.tuning/coord/S023/fast/h0check.log`). The
   script's H0 and no-verdict rows, the "On S023's H0" section below and
   the stamp's H0 branch say so.
2. **S091's capture-rule cases stated their preconditions with S091's bare
   bar.** Every precondition in `tests/` that read `SEE_CAPT_COEFF *
   lmr_depth` now reads the margin the node actually tested with: a
   write-only record in `search_node_probe_t` (`capture_rule_*` -- move,
   move number, reduced depth, the record as the node read it, and the
   margin), written in `negamax_at`'s probed instantiation beside the
   capture rule, so the shipped tree is unchanged (`bench` 7045606 with its
   stream identical line for line, `search_bench` 9 and 12 node-identical).
   Where the rule weighs the move -- C05's case (g2h3), C02's (Qxg6),
   LQ03's (Rxa4, now also its true move number), LQ02's (Rxe4) -- the case
   reads `capture_rule_margin`, and asserts the probe's move number is the
   one the case reads (and, in LQ02's, the reduced depth too); where it
   never does -- C07's only move, C06's
   move past the cap -- the case states the precondition against the widest
   margin any record allows, `search_capture_see_margin_probe(lmr_depth,
   CAPT_HIST_BOUND)`, and C06's asserts the rule never weighed it. S023's
   own floor case reads the node's record and reduced depth off the probe
   instead of assuming the planted worst survived the drive, and asserts the
   margin the node used was the floor. Every rewritten precondition is a
   strengthening and all hold on this tree. The quiescence cases read
   `QS_FUTILITY_MARGIN` with a fresh table that quiescence never writes, so
   the record there is 0 and the margin `QsFutilityMargin` exactly ("the
   capture margins move with the record..." holds `margin(0)`); they are
   unchanged.
3. **Docs stated the pre-S023 margins as current.** One clause each, traced
   to `capture_see_margin` and `capture_futility_margin`: MANUAL.md's
   `QsFutility`, `QsFutilityMargin` and `SeeCaptureCoeff` rows, specs.md's
   search row (S091's `50 * lmr_depth`, S112's stand pat plus
   `QsFutilityMargin`), and `src/search_params.hpp`'s S091 and S112 comment
   blocks (comment only; the 77 rows untouched). The test comment at
   `SEE_MARGIN_DRIVE_DEPTH` says the case now reads the node's margin.

**Readings after the fixes:**
- Gate 5: **46 of 46 in both builds**, format clean, 270 s; gate 6 on the
  final text the same, 181 s.
- `bench` **7045606**, the eight replies and the whole stream unchanged;
  `search_bench` 9 and 12 identical to phase 2's.
- Mutation pass on fixture `8fce3d4` (`p2/mutation_fast/`, 2202 s): **all
  six of `S091_capture_see.py`** (C02, C05, C06, C07, R01, R02), **all
  three of `S269_late_quiets.py`** (LQ01-LQ03) and CX01-CX12 again: **19 of
  19 killed, CX05 equivalent**, each by the case its `Mutation:` line names
  -- C02 by "a capture that gives check is not pruned" (`k >= 1`), C05 by
  "skips the captures that lose material and no others", C06 by "stops at
  its depth cap", C07 by "a node whose only legal move is a losing capture
  is never pruned", R01 by "a capture that gives check is not reduced", R02
  by "a capture that loses material is reduced by the extra ply", LQ01-LQ03
  by their own cases (LQ03 now at the new `r >= 0` precondition, Rxa4
  never weighed). CX01 and CX03 gain a kill in the floor case.
- Debug self-play again, the probe being in `src/search.cpp` (DEC-141): 8
  more games, 0 `Assertion`, 0 disconnects (`p2/selfplay_fast.log`; the PGN
  appends to phase 2's, so it holds 16). `tools/gate_extra.sh`: all five
  stages ok in 925 s, the sanitizer `bench` 7045606, the Release total.
- Prose checks, one mode a call: `--prose` 0 flagged (also over this file
  and the SPRT script by name), `--citations` 0 over 28 files, `--touches`
  0, `--params` exit 0, `--gate` agreeing; `./clang-format.sh --check`
  clean.

#### On S023's H0 or no verdict

The revert of the landing takes out, with the table and its readers, what
landed with it: DEC-287's ceilings and budgets, `EXACT_FLOOR` 160 and both
re-mined rows. S261's E21 row comes back and separates on that tree. The
extra-ply row comes back **stale** -- it had stopped separating on the parent
already -- so a filler step re-mines it on the reverted tree by the same
procedure (DEC-142, DEC-288). The SPRT script's H0 row says so.

**The band case stays, without its record loop** (the coordinator's ruling
at the fast check, below). `tests/test_evaluation.cpp` "a king capturing a
pawn outranks a killer at the same ply" is the accepts' own band guard and
closes `2026-08-13_plan_review.2-F06`; reverted whole it reopens that
finding, and kept as written it does not build, since it reads
`capture_history_entry` and `CAPT_HIST_BOUND`. So the revert commit keeps it
as a written edit, not part of `git revert`'s output:
`adocs/data/S023_h0_band_case.diff` adds it to `7e36271`'s file with the
lines that read the table gone -- the comment paragraph about the worst, no
and best record, the `int16_t& record = capture_history_entry(...)`
binding, the `for (const int planted : ...)` loop, its `record = ...`
assignment and closing brace -- and one comparison of the king's capture
against the killer in their place. It is observed green on the reverted tree
before that commit; at the fast check it built on `7e36271` and read
`CHECK( 900100 > 900000 )`, test_evaluation 32 of 32.

#### Proposed `done:` stamp (supersedes the one above)

    2026-10-14 HH:MM -- verdict 1 of two (DEC-273). <SPRT line for the
    coordinator: H1 | H0 | no verdict at {0, 5} nElo, 8+0.08, Hash 16, CAND
    <sha> vs REF 7e36271: Elo, nElo, LLR, games, forfeits
    (adocs/data/S023_sprt.log); kept, verdict 2 pre-registered next | the
    commit reverted but for the band case, kept without its record loop
    (adocs/data/S023_h0_band_case.diff) and observed green on the reverted
    tree, S025 and S265 to the reserve, the extra-ply row re-mined by a
    filler step>. Capture history -- [piece][target][victim
    type], credited on a capture cutoff and charged for every capture
    searched before any cutoff -- moves S091's capture SEE margin and S112's
    quiescence futility margin by up to a weight each way, floored at 0;
    score_move reads nothing. bench 5636835 -> 7045606; the tune build at
    both weights 0, and at both shares 0, prints the parent's stream (0 of
    121 lines, DEC-215). Fast suite 46/46 in both builds, format clean;
    mutants 22 of 22 killed and CX05 equivalent (CX01-CX12, C05, C06, LQ02,
    LQ03, F01-F06, E21, B04), each by its named case; Debug self-play four
    rounds, 8 games, 0 Assertion; gate_extra all five stages (929 s), the
    sanitizer bench equal to Release. Goldens by their scripts: carry
    ceilings C 4 -> 5 and E 0 -> 1 (DEC-287) and budgets A 500000, C 500000,
    D 1500000, E 100000; EXACT_FLOOR 159 -> 160, a rise; the extra-ply row
    and the E21 row re-mined (DEC-142, DEC-288), each observed red under its
    weakened build; MATE_IN_THREE_FLOOR 11, first-mate 9 and 9,
    capture_mates and B04's row unmoved; the ordering band kept (S282). The
    worst-record mate guard restated as a two-ply window (DEC-285). Fast
    check: 3 findings, fixed -- the H0 band case kept without its record
    loop (the coordinator's ruling, adocs/data/S023_h0_band_case.diff), the
    capture-rule cases' preconditions read the node's own margin off a
    write-only probe (bench unchanged), and the docs state the S023 margins;
    S091's six mutants, S269's three and CX01-CX12 re-run, 19 of 19 killed.
    Implemented from the step file's description and the CPW History
    Heuristic page's (DEC-221); seeds DEC-134 (b). Implemented by an Opus
    subagent in phases; run, read and stamped by the coordinator.

#### The candidate's commit text (supersedes the one above)

    Feed capture history into the two capture margins (S023 v1)

    S023's order note puts capture history's value in the capture futility
    and capture SEE margins and the ordering key second, and the record
    prices the two uses in opposite directions, so they are two verdicts
    (DEC-273); this is the first. A table [piece][target][victim type],
    credited on a capture cutoff and charged for every capture searched
    before any cutoff, moves S091's capture SEE margin and S112's
    quiescence futility margin by up to a weight each way, floored at 0 so
    neither rule's class changes; score_move reads nothing. Both weights at
    0, and both update shares at 0, print the parent's bench stream
    (DEC-215). Implemented from the CPW History Heuristic description
    (DEC-221), seeds DEC-134 (b). The band case the accepts names is written
    now, before score_move reads anything.

    The worst-record mate guard asks for each capture-mate row's mate within
    two plies of its depth, not at it (DEC-285): fixed-depth profiles move
    under any large change to the table, the best record's as well. The
    carry ceilings follow the script, C 4 -> 5 and E 0 -> 1 (DEC-287), with
    the S170 budgets A 500000, C 500000, D 1500000 and E 100000;
    EXACT_FLOOR rises 159 -> 160. Two mined mate rows had stopped
    separating and are re-mined by their scripts, each observed red under
    its weakened build: the extra-ply row (stale already on the parent) and
    the E21 row, one band deeper (DEC-288). SPRT pre-registered in
    adocs/data/S023_sprt.sh against 7e36271; on H0 or no verdict the band
    case stays without its record loop (adocs/data/S023_h0_band_case.diff).

    Fast check: 3 findings, fixed. S091's capture-rule cases now state
    their preconditions with the margin the node actually tested with,
    record included, read off a write-only probe record
    (search_node_probe_t's capture_rule_*), so the shipped tree is the
    same node for node; MANUAL.md, specs.md and search_params.hpp's
    comments state the margins as S023 leaves them.

    Bench: 7045606
