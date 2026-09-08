id:         S184
goal:       the pending documents state the engine as it ships at HEAD -- retuned parameters at their shipped values, S042's touches naming every site that builds the en-passant key, prose that calls no done step pending and routes to no folded id, step headers complete -- and the params checker reaches the pending step files so the class stops recurring
accepts:    S115's "What is there" section and its constants table are rewritten against the aspiration triple `src/search_params.hpp` compiles at this step's HEAD (S085's vector), the sweep's off row is taken at those values and the "keep 5" instruction is gone; S082's quiescence-cap sentence, S120's and S122's clamp figures, `adocs/plan.md`'s "150 centipawns" sentence in "Three things the first review measured" and S118's "What it costs today" paragraph state the shipped values or carry the date they were measured; S042's `touches:` names `load_FEN`'s en-passant sanitiser and every other place the key is built beside `make_move`, and its body states the one rule applied at all of them; the five prose defects of F09 are corrected -- the machine-scope lane paragraph to the list as it stands, `status.md`'s Next line, S159's S093 clause, S171's section title, the three S128 routes to S152 -- and `adocs/plan.md`'s sentence claiming the surveyed engines test at 8+0.08 names the split the OpenBench presets show, 14 of 20 at 8+0.08 with Ethereal, Berserk, Weiss and Stockfish at 10+0.1; S148 and S171 gain their `done:` field and the `author:` lines of S148, S171 and S024 are cleared or explained in one line naming DEC-128 or DEC-111; `tools/plan_prose_check.py --params` covers `adocs/plan_todo/` with named-constant phrases for the parameters those files state, observed red on S115's old text before the rewrite and green after; fast suite green in both builds
touches:    adocs/plan_todo/S115_aspiration_refinements.md, adocs/plan_todo/S082_datagen_qsearch_leaf_labels.md, adocs/plan_todo/S120_eval_cache.md, adocs/plan_todo/S122_king_safety_rebuild.md, adocs/plan_todo/S118_pawn_hash_table.md, adocs/plan_todo/S042_en_passant_only_when_capturable.md, adocs/plan_todo/S159_killer_slot_ageing.md, adocs/plan_todo/S171_inherited_mate_distance.md, adocs/plan_todo/S109_shallow_depth_pruning_block.md, adocs/plan_todo/S110_correction_history_non_pawn.md, adocs/plan_todo/S132_time_management_node_fraction.md, adocs/plan_todo/S148_rfp_ceiling_against_deep_mates.md, adocs/plan_todo/S024_continuation_history.md, adocs/plan.md, adocs/status.md, tools/plan_prose_check.py, tests/CMakeLists.txt
excludes:   any change to what S042 does at the sites it names -- the rule is stated here, the code is S042's; re-anchoring line citations, which is S187; sourcing figures, which is S185; the engine
decisions:  DEC-108, DEC-111, DEC-128
closes:     2026-09-04_plan_review-F05, 2026-09-04_plan_review-F08, 2026-09-04_plan_review-F09, 2026-09-04_plan_review-F10
blocks:
paused_by:
author:
done:

## Why this exists

Four low-to-medium findings of the 2026-09-04 plan review, all of the same
kind: a pending document describes an engine or a plan that no longer exists,
and an implementer following it would act on the stale picture.

**F05, medium.** S115 designs its aspiration sweep on the triple S021 shipped
and S085's verified vector replaced on 2026-08-21, and its constants table
says "keep 5" for the depth gate -- followed, that reverts an axis a +21.02
SPSA verdict moved, silently, inside a step whose verdict would then be
attributed to the window refinements. S082 reasons from a quiescence cap of 8
that has been 19 since S085; S120, S122 and one sentence of `adocs/plan.md`
size their argument on a lazy clamp of 150 that has been 184 since the same
run; S118 prices itself on the pre-S104 binary. The S150 checker keys four
aspiration phrases against `specs.md`, `MANUAL.md`, `DEV_MANUAL.md` and
`plan.md` and never opens `plan_todo/`.

**F08, low.** S042's `touches:` names `make_move` alone. The code's own
comment at the en-passant update in `src/bitboard.cpp` says the key is built
in three places and they must agree, and `load_FEN`'s sanitiser since S161
keeps an en-passant square whenever the victim pawn stands behind an empty
target -- it tests the victim's presence, not whether any pawn can capture.
Change `make_move` alone and a position reached by moves drops a
non-capturable square while the same position loaded from a FEN keeps it: the
transposition mismatch the step exists to remove, moved from move order to
input path, at the root of every SPRT game.

**F09, low.** The lane paragraph counts done S147 among what is left; the
status Next line skips S179; S159 says it "lands before S093, never after"
and S093 landed 2026-08-22; S171's section is titled "Why it is first in the
Open list" while it is postponed and last; S109, S110 and S132 route
follow-up measurements to S128, folded into S152 by DEC-108. And
`adocs/plan.md` says the engines the plan reads from test at 8+0.08 where the
OpenBench presets fetched by the review show Ethereal, Berserk, Weiss and
Stockfish at 10+0.1 and 14 of 20 engines at 8+0.08.

**F10, low.** S148 and S171 have no `done:` field to write the stamp into;
S148, S171 and S024 carry a filled `author:` while sitting in `plan_todo/`,
which the schema reads as a started step.

## The checker extension

`--params` holds no line numbers, which is why it is in the fast suite where
`--citations` is not. The pending files name their parameters by constant
name, so the phrase for them is the constant -- "`ASPIRATION_DELTA` is 50" --
and a named-constant phrase holds no line number either. The extension adds
`adocs/plan_todo/` to the files the mode reads and one phrase family keyed on
the constant names `src/search_params.hpp` declares. Red first on S115's
current text, then green.

## Cost

Documents and a small checker change. Two to three hours.

## Implementation guide (2026-09-05)

Written under DEC-145 against HEAD `49ec858`. Symbols per DEC-135, no line
numbers into code; a document is cited by a phrase to grep for. Two commits
landed between the review and this guide: `2445d23` (DEC-144) deleted
`adocs/plan.md`'s machine-scope section and rewrote `adocs/status.md`'s Next
line, so two of F09's items are already closed and one orphan sentence took
their place (section 3). `accepts:` is the contract; this is the route.

### 1. What this step is, for someone new

Thirteen pending step files, `adocs/plan.md` and `adocs/status.md` describe an
engine that no longer exists: parameters at the values S021 and S033 shipped
rather than the ones S085's SPSA moved them to on 2026-08-21, a `touches:`
naming one of three sites, sentences calling a done step pending or routing to
an id DEC-108 folded away, two headers without `done:`. This step edits those
documents to state the engine as `src/search_params.hpp` compiles it, and
extends `tools/plan_prose_check.py --params` so the pending files are inside
the check that would have caught the parameter class. No `src/`, no match, no
`Bench:` trailer. Deliverable: one commit of document edits plus a small
checker change, a stamp walking the four findings, and their four `Status:`
lines in `adocs/audit/2026-09-04_plan_review.md` moved to closed.

One correction to the step body: `--params` is **already** in the fast suite,
registered as `test_plan_params` in `tests/CMakeLists.txt` (S150, 2026-08-30)
beside `test_plan_touches`. The extension changes what the mode reads and
matches, not whether it runs; the `tests/CMakeLists.txt` edit owed is the
comment above that registration, which names only the four documents.

### 2. The technique as published

A document step, so the "technique" is three practices already written down
here, plus one external convention S042's rule needs.

**Precedence.** `AGENTS.md` "Orient": "specs > plan > status" and "Code that
disagrees with specs is a bug or an unrecorded decision, never silently the new
truth". A pending step file is plan; where it states a number the code compiles
differently, the code and `adocs/specs.md` win. `specs.md` already states every
value restored here (grep "2 / 21 / 437", "19 as shipped", "53.90 ns a call").

**A checker with a declared mesh.** S150's stamp and `DEV_MANUAL.md` "What it
does not cover, stated rather than implied": three rule classes (`TABLE`,
`NEAR`, `PHRASE`), tight on purpose; a rule matching nothing prints `STALE`
rather than passing quietly; tense is deliberately not understood, so a
sentence is repaired to "**was** 8 when this was written ... S085 shipped 19"
without deleting what it records. Use that form wherever the stale sentence is
history worth keeping (S127, section 3). The step schema in `AGENTS.md` "The
plan" -- `author:` "set on start", `done:` "written last" -- is F10's standard.

**The en-passant field, for S042's rule.** CPW's FEN page
(https://www.chessprogramming.org/Forsyth-Edwards_Notation): "The en passant
target square is specified after a double push of a pawn, no matter whether an
en passant capture is really possible or not"; X-FEN "introduced a changed en
passant target square semantic, which is only specified after a double pawn
push was made beside an opponent pawn that might capture en passant if legal".
python-chess names the three readings in `Board.fen(en_passant=...)`:
`'legal'` (default, "only fully legal en passant squares are included"),
`'fen'` ("always include the en passant square after a two-step pawn move"),
`'xfen'` (https://python-chess.readthedocs.io/en/latest/core.html). Chesso
today is the standard-FEN reading in `make_move_impl` and a third, private
reading in `load_FEN` (victim pawn present, target empty, no capturer
required). S042's goal is the X-FEN reading, and the one rule S184 writes into
S042's body is: **an en-passant square is kept only when a pawn of the side to
move stands on a square from which it attacks the target -- the generator's
own candidate test, `tables->pawn_attacks[opponent][board->en_passant]` masked
with the mover's pawns in `generate_moves_body` of `src/bitboard.cpp` -- and
that test is applied at every site that decides the square.** Pseudo-legal:
pins are not examined, as in the generator before its own legality check and
as X-FEN says. Legal or pseudo-legal for the Stockfish comparison is the
owner's (section 10).

**The OpenBench presets, for the `plan.md` sentence.** `Config/config.json`
lists 20 engines (https://raw.githubusercontent.com/AndyGrant/OpenBench/master/Config/config.json).
Re-fetched today: Ethereal STC `"10.0+0.1"`, `Hash=8`
(https://raw.githubusercontent.com/AndyGrant/OpenBench/master/Engines/Ethereal.json);
Stash STC `"8.0+0.08"`, `Hash=16`
(https://raw.githubusercontent.com/AndyGrant/OpenBench/master/Engines/Stash.json).
The split -- 14 at 8.0+0.08, six at 10.0+0.1 (Berserk, Ethereal, Igel,
RubiChess, Stockfish, Weiss) -- is row A33 of
`adocs/data/2026-09-04_plan_review_literature_check.md`, fetched 2026-09-04;
cite the row rather than re-deriving it.

### 3. What chesso has today, and where the change plugs in

**The checker at HEAD**, all in `tools/plan_prose_check.py`: `PARAM_DECL`
parses the `X(symbol, "UciName", default, min, max)` rows of
`src/search_params.hpp` and **captures the UCI name only**; `search_params()`
returns `{UciName: (default, min, max)}`; `PARAM_NEAR` holds three patterns
keyed on that name (name then `is`/`ships at`/`ships`/`=`/`default` then a
number; name, a number, `as shipped`; name, `default`, a number);
`PARAM_PHRASES` holds four wording rules for sentences naming no parameter;
`PARAM_DOCS` names four documents; `check_params(path, params)` runs the three
classes over one file; `check_all_params(paths)` runs `paths or PARAM_DOCS` and
prints `STALE` for a phrase rule that fired nowhere. `pending_step_files(adocs)`
exists for `--citations` and `--touches`: `plan_todo/` plus `plan_current/`,
sorted. Measured here at HEAD: default run 0.27 s, exit 0; with the 74 pending
files added, 0.80 s.

**Today over the pending files** (`python3 tools/plan_prose_check.py --params adocs/plan_todo/*.md`,
exit 1): one `NEAR` line for S127 -- the sentence ending "and **the bound
binds**", which names `MaxQsearchDepth`, says "is" and gives the pre-S085
value; S150 fixed its twin in `plan.md` and nobody fixed this one -- plus four
`STALE` lines that are an artefact of the explicit-file mode (the phrases live
in `specs.md` and `MANUAL.md`) and vanish when the set is the union. **S127 is
not in `touches:`**; section 10.

**The extension, in order:**

1. `PARAM_DECL` captures the C++ symbol too; `search_params()` returns the
   value under both keys (or a second `search_symbols()` dictionary).
2. A symbol-keyed copy of `PARAM_NEAR` with **both backticks mandatory** around
   the symbol. Measured reason: with them optional, S114's formula line
   "`NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR` = 3 + depth/6" is flagged as
   a claim about the divisor, because the regex reads the formula's closing
   backtick. Mandatory, it is not, and no true hit is lost.
3. `check_all_params` scans `PARAM_DOCS` **plus** `pending_step_files(adocs)`
   when given no files. `plan_done/` stays out: history.
4. Docstring and the comments above `PARAM_NEAR` and `PARAM_DOCS` say what is
   read now and why the symbol form insists on backticks.
5. `tests/CMakeLists.txt`: the `test_plan_params` comment names the pending
   files and the measured cost.
6. `DEV_MANUAL.md` paragraph "**`--params`.**": file set, symbol form, cost.
   `MANUAL.md` and `adocs/specs.md` do not mention the checker (grep) and need
   nothing.

Dry run of steps 1 to 3 at HEAD (a scratch script importing the module, no
tree change): four flags -- S082's quiescence sentence, S115's first sentence,
S127's, and **this file's own "The checker extension" paragraph**, which quotes
the example phrase with the old delta. That last is the red-first observation
from an unexpected file: reword the paragraph while S184 is in `plan_current/`
("the constant name, `is`, and a number"). Nothing in the four documents fires
under the symbol form.

**Worklist, F05.** Values read from `src/search_params.hpp` at `49ec858`;
re-read at the step's own HEAD (S148 may move `RFP_MAX_DEPTH`, S039 the
margin). Where a sentence is rewritten rather than dated, write it in a `NEAR`
form so it is guarded from now on: the UCI name in backticks, a comma, the
value, `as shipped`, the step that shipped it.

| file, phrase to grep | states | at HEAD (symbol) | replacement |
|---|---|---|---|
| S115 "What is there", "widening doubles the failing side alone" | delta 50 | 21 (`ASPIRATION_DELTA`) | rewrite the section against 2 / 21 / 437, S085; drop the claim that fail-soft plumbing is missing, which the file's own "Scope concern" refutes |
| S115 "Chesso's gate of 5 was measured here (S021) and stands" | 5 | 2 (`ASPIRATION_MIN_DEPTH`) | "S021 measured 5; S085 moved it to 2, its arithmetic floor, inside a +21.02 +/- 9.86 verified vector" |
| S115 table row "depth gate" | "5, measured (S021) ... keep 5 unless the sweep says otherwise" | 2 | "2 (S085) -- hold; a row at 5 re-measures S085's axis and is not this step's" |
| S115 table row "initial delta" | "50 (S021)" | 21 | "21 (S085); excluded, S127 owns it" |
| S115 table row "max delta escape" | "400 then full" | 437 (`ASPIRATION_MAX_DELTA`) | "437 then full (S085; S021 measured it flat 100 to 2000)" |
| S115 "Implementation sketch" step 2, the sweep's off row | S021 triple implied | 2 / 21 / 437 | the off row of `adocs/data/S021_aspiration_sweep.py` is taken at the shipped triple; say so |
| S082 "The trap", "A leaf reached by exhausting that bound" | 8, with a citation into `src/search.cpp` that lands on a comment | 19 (`MAX_QSEARCH_DEPTH`, tested in `quiescence` of `src/search.cpp`) | name parameter and function; value in `NEAR` form |
| S120 "The point is not speed", "150 centipawns for the two of them together" | 150 | 184 (`LAZY_EVAL_MARGIN`) | 184 in `NEAR` form; the same paragraph's "costs 83 ns" is pre-S104 -- date it or quote `specs.md`'s 53.90 ns |
| S120 "Clamp staleness", "Every cached value carries the 150 clamp" | 150 | 184 | 184 |
| S122 "The clamp is the whole problem", "150 centipawns for both together", "one hundred and fifty" | 150 | 184 | 184, both places |
| `adocs/plan.md` "Three things the first review measured", "150 centipawns for the two together", "one hundred and fifty" | 150 | 184 | 184 in `NEAR` form; **leave "0 / 150 / 2000 gives 6.55 / 5.46 / 4.82 Mnps"** -- a three-setting measurement the tight rule keeps unflagged |
| S118 "What it costs today", "83.35 ns a call at 12.0 M calls a second ... 5.8 M nodes a second" | pre-S104 binary | `specs.md`: `bench_eval` 53.90 ns a call, 18.6 M calls per second (2026-08-19, S104) | date the paragraph "measured 2026-08-19 before S104" and quote the S104 line; a fresh `./build/tests/bench_eval` on the workstation is optional |
| S127 "the bound binds" | pre-S085 cap | 19 | S150's form: "**was** 8 when this was written (2026-08-19); S085 shipped 19" |

**Two F05-class values are already fixed and are not this step's (DEC-157).**
S180 rewrote the `### 4. Constants and seeds` section of seven block-1 search
steps on 2026-09-08 and corrected, in passing, the two stale shipping values
that sat inside them: S114's "`NULL_MOVE_BASE` seed 2, ships today" against
the 3 S085 shipped, and S132's `TM_NODE_MIN_DEPTH` "seeded beside
`ASPIRATION_MIN_DEPTH`" against that gate's pre-S085 5 where 2 compiles.
Neither value has a row in the F05 worklist above and neither is this step's
to fix; do not go looking for them. S114 is not in this step's `touches:` at
all; S132 is, but for the unrelated F09 row that routes its follow-up
measurements to S128. The rest of the F05 class is unaffected.

**Worklist, F08 -- S042.** Every site in `src/bitboard.cpp` touching
`board->en_passant` or its key, from grep at HEAD:

| function | what it does | rule applies? |
|---|---|---|
| `make_move_impl` | sets `new_en_passant` on `move.double_push`, xors `ep_randoms` out and in | yes -- the named site |
| `load_FEN` | parses field 4; the S161 sanitiser keeps the square when the target is empty and the victim stands behind it; then `compute_full_hash` | yes -- `supported` gains the capturer term |
| `set_en_passant` | sets any square and re-keys; declared in `src/bitboard.hpp`, **no caller in `src/`, `tests/` or `tools/`** | yes if kept; S042 decides |
| `compute_full_hash` | xors `ep_randoms[board->en_passant]` unconditionally, `INVALID_INDEX` included | no -- consumer, must agree with the incremental key |
| `make_null_move`, `unmake_null_move`, `unmake_move_impl` | clear or restore through the history entry | no -- consumers |
| `generate_moves_body` | the capture from `pawn_attacks[opponent][en_passant]` masked with the mover's pawns | no -- the rule's own expression |
| `generate_FEN`, `cleanup_board` | prints field 4; resets to `INVALID_INDEX` | no |

`touches:` becomes `src/bitboard.cpp make_move_impl, load_FEN, set_en_passant,
compute_full_hash` plus `tests/test_audit_fen_semantics.cpp`, whose S161 cases
"a real ep square survives and is playable", "an ep square on the mover's own
rank goes", "an occupied ep target square goes" and "a cleared field is
cleared in the hash" the sanitiser change moves through. The body gets the
rule of section 2 and F08's sentence: with `make_move_impl` alone changed, a
position reached by moves drops a non-capturable square while the same
position loaded from a FEN keeps it, and every SPRT game starts from
`position fen`. Do not change what S042 does there; `excludes:` forbids it.

**Worklist, F09.** Verify the first two, edit the rest:

- `adocs/plan.md` lane paragraph: **deleted by DEC-144** (`2445d23`; text in
  history at `66cbc54`). One orphan remains -- grep "The batch does not change
  the machine-scope lane below it; S020 resumes as the first non-batch entry"
  -- rewrite to the list as it stands (S178, S173, S171, S189, S179, S198, then
  the instrument lane S180, S184 ...). Discovered here; in scope; stamp it.
- `adocs/status.md` Next line: **rewritten** by the same commit (S178, S173,
  S171's census, S189 and S179, S198). Verify against the Open list only.
- S159, grep "this lands before S093 or is folded into it deliberately, never
  after": S093 landed 2026-08-22, H1 accepted, +10.73 +/- 6.70 over 6412 games
  (`adocs/plan_done/S093_history_malus_and_ageing.md` `done:`). Replace: the
  constraint is void by history; this step measures ageing on top of S093's
  malus and gravity and its verdict is read as that.
- S171, grep "## Why it is first in the Open list": third, postponed by
  DEC-128. Retitle "Why the BUGS rule put it first, and where DEC-128 put it".
- S109 "S128 eventually reads that", S110 "after S128 moves the measurement",
  S132 "fold into S128's rated run" and its list item "**S128:**": route to
  S152, "which absorbed S128's question (DEC-108)".
- `adocs/plan.md`, grep "engines this plan reads from test at 8+0.08": the A33
  split of section 2; "runs at `10+0.2`" is pre-S105 history and reads as such
  with "fixes both" left standing.

**Worklist, F10.** S148: add `done:` after `author:`, clear `author:` (never
in `plan_current/`; F10 checked `git log --all`). S171: add `done:`, clear
`author:`, one body line "`author:` cleared: DEC-128 moved the file back to
`plan_todo/` with its run written in". S024: the header's `author:` is already
empty and `done:` exists; the filled `author:` is a stray line just above "##
The MacBook attempt, discarded 2026-08-30" -- delete it; that section's first
sentence already names DEC-111.

### 4. Constants and seeds

None proposed. Every value written is read from `src/search_params.hpp` at the
step's own HEAD -- DEC-105 form (b) in its trivial case, chesso's own compiled
numbers -- and every cost figure from `adocs/specs.md`. Do not copy numbers
out of this guide; run `grep -n 'X(' src/search_params.hpp` first.

### 5. Interactions and traps

- **Every commit is green (COMMITS), so the checker cannot land before the
  documents**: with the file set extended and S115 unfixed, `test_plan_params`
  is red. One commit; if the owner wants two, documents first, checker second,
  never the reverse. The red-first observation lives in the stamp.
- **Run `--citations` over the thirteen files before editing any of them** and
  keep the output beside the stamp. DRIFT's baseline is the commit that last
  wrote the step file (`DEV_MANUAL.md` "Run it before editing a step file, not
  after", DEC-119); these edits clear every DRIFT flag those files carry
  without repairing one, and S187 needs the list.
- **Explicit-file runs print `STALE`** whenever `specs.md` is not on the
  command line. Read the `NEAR` lines, or run the default mode.
- **Do not loosen the symbol form** to reach the lazy-clamp sentences: S150
  measured one false positive of a looser rule ("`LazyEvalMargin` at 0, 150
  and 2000"), this guide a second (S114's formula). Sentences naming no
  parameter are fixed by rewriting them into a `NEAR` form, which never goes
  `STALE`.
- **Sweep and history sentences stay**: "0 / 150 / 2000", "8 to 19",
  "5 / 50 / 400 to 2 / 21 / 437" are records the tight patterns leave alone;
  a rewrite into "is N" would flag them.
- **`--prose` cannot see F09's shapes** ("What is left is ... (S147") and this
  step does not teach it to; a finding if the owner wants one.
- **`plan_done/` is never edited**: S021, S085, S150 keep their old values.
- **`set_en_passant` is uncalled**: report it in S042's body; deleting it is
  `src/`. **`src/evaluation.hpp` already says 184** ("184 since S085's SPSA
  run raised it from 150"); no code comment is owed.

### 6. Tests

No pruning, reduction or extension rule, no `make_move`, no generator, no
search: guard test, mutant, Debug self-play and INV-6 do not arise. The proof
is `git diff --stat HEAD~1 -- src/` printing nothing at completion, in the
stamp. The test is `test_plan_params`, observed red before it is trusted:

```
# 1. red: checker extended in the working tree, no document edited yet
python3 tools/plan_prose_check.py --params ; echo exit=$?
#   expect NEAR for S082 (stated 8, code 19), S115 (50 vs 21), S127 (8 vs 19),
#   S184's own paragraph (50 vs 21); no STALE; exit 1
# 2. green, after the edits
python3 tools/plan_prose_check.py --params ; echo exit=$?      # nothing, exit 0
# 3. non-vacuity from the other side (S150's method): move AspirationDelta's
#    default in src/search_params.hpp to 22 -- specs.md, MANUAL.md and S115's
#    rewritten sentence must all fire -- then restore
git diff -- src/                                                # empty
python3 tools/plan_prose_check.py --touches | tail -1           # touches flagged: 0
python3 tools/plan_prose_check.py --prose | tail -1             # sentences flagged: 0
```

Paste the step-1 lines into the stamp verbatim: they are the "observed red"
`accepts:` requires, and S127's line records that the class was wider than F05.

### 7. Measurement

None: no behaviour moves, so no timing lane, no SPRT, no wait on S198's A/A.
The one number is the checker's cost, 0.27 s to 0.80 s here for the default
run (`time python3 tools/plan_prose_check.py --params`), once per build in the
gate. Re-measure on the workstation for the `tests/CMakeLists.txt` comment.

### 8. Completion checklist

1. `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`.
2. `python3 tools/plan_prose_check.py` (all four); `--citations` count before
   and after in the stamp, for S187.
3. `DEV_MANUAL.md` "**`--params`.**" updated; `MANUAL.md` and `adocs/specs.md`
   checked, unchanged, said so.
4. `adocs/audit/2026-09-04_plan_review.md`: F05, F08, F09, F10 `Status:` from
   "planned -- S184" to closed; nothing else in the report.
5. Stamp: date; each finding's rows; the two F09 items DEC-144 closed,
   verified; the orphan lane sentence and S127 as discovered-in-scope; the
   red-first lines; the cost; `git diff --stat -- src/` empty; `--citations`
   before and after.
6. Commit: subject under 72 characters, body names S184 and the four finding
   ids; no `Bench:` and no `No functional change` -- the rule binds `src/`
   commits only.
7. `plan.md` and `status.md` through the coordinator; since this step's own
   edits touch both, the coordinator lands the commit (PLAN rule).

### 9. Sources read

- `adocs/audit/2026-09-04_plan_review.md` F05, F08, F09, F10 -- evidence and suggested resolutions.
- `adocs/plan_done/S085_spsa_first_run.md` -- the vector table, "shipped now" column, +21.02 +/- 9.86.
- `adocs/plan_done/S150_doc_value_check.md` -- the three rules, red-first method, tense repair, retired phrase.
- `adocs/plan_done/S161_fen_semantic_validation.md`, `tests/test_audit_fen_semantics.cpp` -- the sanitiser and its case titles.
- `tools/plan_prose_check.py`, `tests/CMakeLists.txt` (`test_plan_params`, `test_plan_touches`), `DEV_MANUAL.md` "carries four plan-hygiene checks" -- the mode as registered and documented.
- `src/search_params.hpp`; `src/bitboard.cpp` (`make_move_impl`, `load_FEN`, `set_en_passant`, `compute_full_hash`, `make_null_move`, `generate_moves_body`, `generate_FEN`, `cleanup_board`); `src/bitboard.hpp`; `src/search.cpp` `quiescence`; `src/evaluation.hpp` -- values and sites at HEAD.
- `adocs/decisions.md` DEC-108, DEC-111, DEC-128, DEC-144, DEC-145.
- `adocs/plan.md`, `adocs/status.md`, the thirteen files in `touches:`, plus S114, S127, S131, S151 -- every phrase quoted above, at `49ec858`.
- `adocs/data/2026-09-04_plan_review_literature_check.md` row A33 -- the OpenBench split.
- https://www.chessprogramming.org/Forsyth-Edwards_Notation -- FEN and X-FEN en-passant semantics.
- https://python-chess.readthedocs.io/en/latest/core.html -- `Board.fen(en_passant=...)`.
- https://raw.githubusercontent.com/AndyGrant/OpenBench/master/Config/config.json -- 20 engines; `.../Engines/Ethereal.json` 10.0+0.1 and `.../Engines/Stash.json` 8.0+0.08, re-fetched today; the other 18 rest on A33.

### 10. Questions deferred to the owner

1. **S127 is stale and not in `touches:`.** The extended checker is red on it,
   so the step cannot complete green without editing it. Add
   `adocs/plan_todo/S127_spsa_full_parameter_run.md` to `touches:`
   (coordinator's edit) or treat it as discovered-trivial in the stamp; the
   former keeps the diff contract honest.
2. **S042's rule, pseudo-legal or legal.** This guide writes X-FEN's
   pseudo-legal reading (a capturer stands on an attacking square): the
   generator's own test, no legality search in `make_move`. S042's `accepts:`
   compares `generate_FEN` with Stockfish; python-chess defaults to `'legal'`.
   The owner picks the reading S184 writes and the `en_passant=` flag S042's
   comparison will use.
3. **S171's `author:`** -- clear with the DEC-128 line (recommended) or keep as
   the record of the 2026-09-03 session.
4. **S118's cost paragraph** -- date it and quote `specs.md`'s S104 line (no
   run), or re-measure `bench_eval` on the workstation. `accepts:` allows both.
5. **`--prose` and F09's shapes** -- a later step, or read by hand at
   completion. Not this step's.
