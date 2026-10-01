id:         S249
goal:       three golden comments read what their trees report -- `test_mate_breadth`'s shipping count, `MATE_IN_THREE_FLOOR`'s comment, and S132's sweep docstring's offsets
accepts:    `tests/test_mate_breadth.cpp`'s GOLDEN comment quotes the count the shipping tree reports (145 written, 147 read on `cd50c7a`'s parent) with the command that took it; the comment beside `MATE_IN_THREE_FLOOR` in `tests/test_engine.cpp` quotes its count (12 written, 13 read) the same way; `adocs/data/S132_node_share_census.py`'s docstring names the offsets its aspiration rows use (0/1/2, not S021's 0/37/71) or is corrected to what the script does; no floor and no assertion changes; both fast suites green
touches:    tests/test_mate_breadth.cpp (a comment), tests/test_engine.cpp (the comment beside `MATE_IN_THREE_FLOOR`), adocs/data/S132_node_share_census.py (a docstring)
excludes:   the floors, the counts' derivation rule, any behaviour
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator (2026-10-01)
done:       2026-10-01 -- two items of three, the first moved to S250 by DEC-247. **Item 3**: `adocs/data/S132_node_share_census.py`'s docstring now says the census uses S021's sampling rule at offsets 0, 1 and 2 (its `OFFSETS`), while the aspiration rows were chosen at S021's 0, 37 and 71, so only the offset-0 hundred is common to the two picks (`.tuning/coord/S249/S132_offsets.log`: 300 and 297 distinct, 100 shared); code unchanged. **Item 2**: `MATE_IN_THREE_FLOOR`'s comment already reads what the tree reports, 12 of 24 at `RfpMinPly` 3 and 2 by `adocs/data/S154_floor_margin_sweep.py floor` (`S154_floor.log`), and the floor of 11 separates (7 of 24 at 1 and 0); left, as the amendment allowed. Three figures in the surrounding prose are stale (10 -> 7 weakened, 21 -> 20 at `RfpMinPly` 4, mates in four 1 -> 2 of 16) and moved to S250. **Item 1 moved**: re-deriving `test_mate_breadth`'s count by `adocs/data/S156_mined_floor_sweep.py` read 146 shipping against 143 weakened with `EXACT_FLOOR` 143: the gate built at the weakened default goes green and the script prints "THE FLOOR NO LONGER SEPARATES" (`S156_mined.log`). Moving a floor is this step's exclude, so the comment is unedited and the floor's re-placement is S250's (DEC-247). Both fast suites 41 of 41, `clang-format.sh --check` clean, `plan_prose_check.py` `--citations`, `--touches`, `--params` clean, `src/` and `tests/` untouched; cold fast check LAND, its one trivial (a docstring line over 80 columns) reflowed. `DEV_MANUAL.md` and `MANUAL.md` checked: neither quotes these figures, no change; `README.md` human-owned, no change.

## Why this exists (2026-09-30)

S115's build read the mate guards against the tree and found two golden
comments quoting shipping counts the parent no longer reads -- both floors
still hold, so nothing is red -- and its sweep script, executing S021's
picker, found S132's docstring calling offsets 0/1/2 "the aspiration rows'
300" where S021's rows are at 0/37/71. Comments and a docstring, no reach
into play: a filler behind the next strength step (DEC-171), named by id in
`adocs/data/S115_sprt.sh`'s open findings while open.

## Amended at S115's landing (2026-09-30, the coordinator)

The accepts' figures were read on `cd50c7a`, which carried S114's
static-score term. On the tree that term's H0 left, S115's rebase read the
two again (`.tuning/coord/S115b_logs/`, parent `f5eaa99` and candidate
alike): the mined set is **146** exact at depth 10, so `test_mate_breadth`'s
comment (145) is still stale, by one; mates in three are **12 of 24**, what
the comment beside `MATE_IN_THREE_FLOOR` already says, so that half is no
longer owed unless the tree moves it again. Whoever takes this step re-reads
both on the tree of the day before editing either. The S132 docstring item
stands.


## As built (2026-10-01, on `0cf3ec8`, the worktree `chesso-s249`)

**Deviation first: item 1 is stopped, not edited.** Its re-derivation found a
floor problem, which the brief and the excludes say is not a comment problem.
`python3 adocs/data/S156_mined_floor_sweep.py` (ref HEAD = `0cf3ec8`, depth 10,
log `.tuning/coord/S249/S156_mined.log`) reads:

    RfpMinPly   exact   right sign   wrong sign
            3     146          149            0
            2     146          150            0
            1     143          145            0
            0     143          145            0
    shipping value 146, weakened-guard value 143, gap 3
    THE FLOOR NO LONGER SEPARATES.
    the gate built with RfpMinPly defaulting to 1: GREEN, WHICH IS THE FAILURE

`EXACT_FLOOR` is 143 and the weakened guard now reads 143, so the gate does
not go red when the guard it fences is weakened. The last tracked reading,
`adocs/data/S156_mined_floor_sweep.log`, was 145 / 141, gap 4, red. The
shipping count (146) is above the floor and the fast suites are green, so
nothing is red; the floor has lost its separation. Re-placing it is a floor
change, excluded here (DEC-142: re-derived with a margin stated, never read
off a run) -- the coordinator's to schedule. The comment was not touched
because quoting 146 beside "141 with it weakened" and "gap 4" would state a
separation the tree no longer has.

**Item 2: already true, left as the amendment allows.**
`python3 adocs/data/S154_floor_margin_sweep.py floor` (ref HEAD, log
`.tuning/coord/S249/S154_floor.log`): RfpMinPly 3 (ships) and 2 read m3
**12/24**, what the comment ("12 of 24 at the shipping floor", "12 at the
shipping guard") says. The floor 11 still separates: RfpMinPly 1 and 0 read
m3 7/24. Not in this step's scope and left: the comment's other figures are
stale on today's tree -- "10 at RfpMinPly 1" reads 7, "21 at RfpMinPly 4"
reads 20, "1 of 16" mates in four reads 2/16 at every RfpMinPly. A finding
for whoever next re-derives `MATE_IN_THREE_FLOOR`.

**Item 3: corrected to what the script does.** The code is `OFFSETS =
(0, 1, 2)`; S021's TSV's sample column is 0, 37, 71 (21 rows each). Executing
both picks (log `.tuning/coord/S249/S132_offsets.log`): 300 distinct
positions at 0/1/2, of which 100 (the offset-0 sample) are common with the 300
at 0/37/71. The script is unchanged; only its docstring.

Old:

    THE PICK. The same 300 positions src/search_params.hpp's aspiration rows were
    chosen over: 100 per offset from adocs/data/S018_raw.tsv, four per
    game_phase() value that has at least four rows, at offsets 0, 1 and 2. It
    imports ...

New:

    THE PICK. S021's sampling rule at offsets 0, 1 and 2 (OFFSETS below): 100 per
    offset from adocs/data/S018_raw.tsv, four per game_phase() value that has at
    least four rows, 300 positions. The rule and not the positions: the aspiration
    rows of src/search_params.hpp were chosen at S021's offsets 0, 37 and 71, the
    sample column of adocs/data/S021_aspiration_sweep.tsv, so only the offset-0
    hundred is common to the two picks. It imports ...

**Gates, on the edited tree.** `cmake --build build -j8` and `build-tune -j8`
clean; `ctest -L fast` 41/41 in both (`.tuning/coord/S249/fast_release.log`,
`fast_tune.log`; the first pass without `CLANG_FORMAT_MAJOR=22` failed only
`test_clang_format_script`, DEC-146's known export, and was re-run with it);
`./clang-format.sh --check` 0; `tools/plan_prose_check.py` `--citations`,
`--touches`, `--params` each 0 (`prose--*.log`); `git diff 0cf3ec8 -- src`
empty. No floor, assertion or behaviour changed.

**Proposed** -- the coordinator's choice, since item 1 cannot close here:
(a) a new step re-places `EXACT_FLOOR` by `S156_mined_floor_sweep.py` with
its comment, and this step pauses behind it (`paused_by:`) and then takes the
comment's count; or (b) item 1 moves into that new step and this one
completes on items 2 and 3, with the stamp
`done: 2026-10-01. MATE_IN_THREE_FLOOR's comment already reads 12/24 on
0cf3ec8 (S154 floor, .tuning/coord/S249/S154_floor.log), left; S132's
docstring names its offsets 0/1/2 and S021's 0/37/71; test_mate_breadth's
item moved to S<new> (floor 143 no longer separates, 146 vs 143); both fast
suites green; no src change.` and commit text

    Name the offsets S132's census actually samples (S249)

    The docstring called its 0/1/2 pick the aspiration rows' 300, which S021
    took at offsets 0/37/71; only the offset-0 hundred is shared. The mate in
    three comment already reads today's 12/24. Comments only.

    No functional change
