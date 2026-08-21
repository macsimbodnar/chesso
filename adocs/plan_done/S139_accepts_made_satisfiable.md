id:         S139
goal:       every pending accepts field states something the harness can actually produce and the plan order can actually reach
accepts:    S119's accepts asks for its verdict at the S105 harness setting rather than at Hash 128, which is the edit DEC-088's own Consequences line ordered and which was never applied; S117's accepts no longer admits a truncation-behaviour change with "no SPRT owed", because INV-6 allows that only on identical node counts and identical best moves and S117's own body says the bound moves; S125's accepts either drops its dependency on S118's pawn hash or the plan reorders the two, and whichever is chosen is stated in the file; S136's accepts and DEC-092 agree with the taper division count that will be live once S055 has landed, since the plan orders S055 first; S109's gives-check clause either binds every rule it gates or names the rules it binds; each edit is checked against the code or the harness file that decides it, quoted in the step file
touches:    adocs/plan_todo/, adocs/decisions.md
excludes:   implementing any of the five steps; reordering the plan beyond the S118/S125 choice if that is the option taken; the numeric claims the audit listed as deferred
decisions:  DEC-088, DEC-092, DEC-087
closes:     2026-08-20_plan_review-F02, 2026-08-20_plan_review-F03, 2026-08-20_plan_review-F04, 2026-08-20_plan_review-F07, 2026-08-20_plan_review-F15
blocks:
paused_by:
done:      2026-08-21. Five pending accepts fields rewritten so each states something the harness
                        can produce and the plan order can reach. Every edit traced to the file that
                        decides it and quoted above; no code touched, no match run.
            
                        1. S119 asks for its verdict "at the S105 harness setting, with the pressure
                           ratio stated" -- DEC-088's own Consequences clause, applied at last, with the
                           60-120 overwrites per entry named. fastchess.sh:225 is option.Hash=16 and
                           grep -n Hash returns it and the echo at :215 and nothing else; there is no
                           override, so the old clause could not be run at all. rating.sh:36 keeps
                           hash_mb=128 and is named in S119 as the separate tool a rating-regime
                           verdict would need.
            
                        2. S117 no longer grants itself the exemption. The identity is now the gate
                           that buys INV-6's no-match lane rather than a claim made alongside it, the
                           "or the tolerance is re-pinned" branch is gone, a moved truncation is a
                           packing bug, and one retained deliberately owes an SPRT. specs.md:80-87 has
                           two lanes and no third. The file's "do S055 first or fold it in" question
                           is closed to S055 first, which its own Interactions had already decided.
            
                        3. S125 drops the S118 dependency; the plan order is unchanged. Four
                           independent statements order the terms before the cache and none the other
                           way -- DEC-087 (h) and (i), plan.md:143-145 and :251-253, and S118's own
                           body at :15-19 and :34-37. A cache in front of a cheap computation is the
                           published 10 % slowdown DEC-087 moved S118 to avoid. What replaces the
                           clause is a measurement: S125 records the per-call cost and that is the
                           baseline S118's verdict is read against.
            
                        4. S136's accepts derives the division count at its own HEAD instead of
                           stating a literal, and names the S055-landed case: N is 2, so bound
                           1.917 to 2.875, tolerance 2 to 3, per-position threshold > 1.0 to
                           > 2.0 = 48/24, worst-pin > 1.9 to > 2.8 -- which are today's shipped
                           numbers, because S055 removes a division and this step puts one back.
                           plan.md:352 orders S055 at 39 against S136 at 50; test_eval_model.cpp
                           :242-249, :277, :311-314, :335, :343 are the before side. DEC-092's
                           literals are owner text and were NOT edited: an explicitly unapproved
                           Proposed: block was appended to the entry instead.
            
                        5. S109 splits the exemption list. In check, PV, first move and near-mate
                           bounds bind all four rules; gives-check binds futility, history and quiet
                           SEE only and explicitly not LMP. src/search.cpp:678 computes is_check_move
                           after make_move at :666, and grep -rn over src/ returns that line and its
                           one consumer at :710 -- no pre-make predicate exists for the generation
                           branch at :629-633/:649-661 to consult. Scope concern 2 is narrowed to the
                           owner question that remains: whether to buy LMP the exemption with a
                           post-make prune.
            
                        Gates: build clean, ctest -L fast 18/18 in 14.90 s, clang-format --check clean.
                        tools/plan_prose_check.py 54 -> 50 flagged over 68 files, --prose exit 0, no
                        new flag from these edits. The 4-flag drop is not a fix and is not claimed as
                        one: editing S109 and S117 reset their drift baselines, which
                        plan_prose_check.py:240-244 documents ("touching a step file relaxes its own
                        drift check"). The four that stopped being reported, uninspected: S109:160 into
                        src/search_params.hpp:122-123, and S117:83/:92/:144 into tests/. S109's ANCHOR
                        flag at :220 survives and is S138/S141 territory.
            
                        Left alone deliberately: DEC-092's Decision literals (proposal only, the owner
                        moves them); S136's goal line and plan.md:363, which both still say "three
                        divisions" and need adocs/plan.md in a touches: field this step did not have.
                        README.md, MANUAL.md and DEV_MANUAL.md checked -- none names any of the five
                        steps, DEV_MANUAL.md:1046 already states the same pressure ratio S119 now
                        asks for, and :1053 already records rating.sh at 128. No change needed.

## What unites these five

Each is an `accepts:` field that cannot be satisfied as written, so each is a
step that would either stall at its own gate or pass by ignoring it. Two are
worth stating in full because they are decided already and simply not applied.

**S119 asks for Hash 128.** `adocs/plan_todo/S119_tt_cluster_layout.md:3`
demands "an SPRT verdict at **Hash 128** -- the regime S105 sets", and
`fastchess.sh:225` hardcodes `option.Hash=16` with no override. DEC-088 is the
decision that *chose* 16 over 128, on the ground that what transfers across time
controls is table pressure, and its own `Consequences:` line ordered this clause
changed to "at the S105 harness setting". The edit never happened. 128 MB is
also, in DEC-088's words, the setting that "would flatter every table-hungry
change S119 is about to make" -- so the unapplied edit is not cosmetic, it is
the difference between measuring S119 and flattering it.

**S117 grants itself an exemption INV-6 does not give.** Its accepts admits the
change with "no SPRT owed", and INV-6 allows that only for a change proven
behaviour-neutral by identical node counts and identical best moves. S117's own
body says the truncation bound moves. A step cannot both alter play and skip the
verdict.

## The five edits, and the file that decided each

Every line below was read at HEAD `229a309` before the edit, not recalled.

**1. S119, the hash.** Decided by the harness. `fastchess.sh:225`:

```
  -each tc="$tc" option.Hash=16 option.Threads=1 \
```

`grep -n Hash fastchess.sh` returns exactly that line and the `echo` at `:215`;
the file's own header documents `REF`, `OUT` and `CONCURRENCY` as the
overrides and there is no hash one. `rating.sh:36` is `hash_mb=128`, fed to
`-each` at `:172`, so the rating regime exists but as a different tool.
DEC-088's `Consequences:` had already ordered the wording -- *"S119's SPRT
clause changes from 'at Hash 128' to 'at the S105 harness setting, with the
pressure ratio stated'"* -- so applying it is execution, not invention. The
pressure ratio is DEC-088's own: 660 M nodes a game against 5.6 to 11 M
entries, 60 to 120 overwrites apiece.

**2. S117, the SPRT exemption.** Decided by INV-6, `adocs/specs.md:80-87`: a
change claimed behaviour-neutral *"proves it with identical node counts and
identical best moves from `tools/search_bench.py`, and **is not sent to a
match**"*, while *"a change that alters play is retained only with an SPRT
verdict against a named commit"*. Two lanes, no third, so the field's "or the
tolerance is re-pinned" branch had nowhere to live. The identity is now the
gate that buys the lane; a moved truncation is a packing bug, and if one is
ever retained deliberately it owes the verdict. S117's own Measurement section
already said the same from the other side.

**3. S125 and S118, the order.** Chosen: **drop the dependency, keep the
order.** Four independent statements order the terms before the cache, and
none the other way -- DEC-087 (h) (*"behind the expensive pawn terms -- caching
a cheap pawn evaluation measured a 10 % slowdown in the published record"*) and
(i) (the Stash-ledger order, connected/phalanx then pawn hash);
`adocs/plan.md:143-145` and `:251-253`; S118's own body at
`S118_pawn_hash_table.md:15-19` and `:34-37`. Reversing them would put a cache
in front of the cheap computation it caches, which is the measured slowdown
DEC-087 moved S118 to avoid. The clause was S125's only cost control, so what
replaces it is a measurement: the per-call cost is recorded in S125 and is the
baseline S118's verdict is read against.

**4. S136 and DEC-092, the division count.** Decided by S055 plus the plan
order. `adocs/plan.md:352` is S055 at entry 39, `:363` is S136 at entry 50.
S055's accepts (`S055_taper_stage_two_once.md:3`) re-pins the guard in its own
commit to *"the post-merge bound of 2 x 23/24 = 1.917"* with the tempo
precondition message *"3 x 23/24 = 2.875 with its tolerance of 4 becoming 3"*.
Today's shipped numbers, for the before side:
`tests/test_eval_model.cpp:242-249` names four divisions and says three round,
`:277` is `CHECK(std::abs(model - engine_white) <= 3.0)`, `:311-314` is the
tempo precondition message at 3.833/4, `:335` is `difference > 2.0`, `:343` is
`worst > 2.8`. So when S136 runs, two divisions round and unfreezing tempo
makes three: 1.917 to 2.875, tolerance 2 to 3, threshold `> 1.0` to `> 2.0`.
S136's accepts now derives N at its own HEAD and names that case. **DEC-092's
literals were not edited** -- they are owner text with no `Consequences:` line
ordering a change, so the correction is an explicitly unapproved `Proposed:`
block appended to the entry.

**5. S109, the gives-check clause.** Decided by `src/search.cpp:678`:

```cpp
    const bool is_check_move = is_capture ? false : is_check(game);
```

which sits after `make_move` at `:666`. `grep -rn "gives_check\|is_check_move"
src/` returns that line and its single consumer, the LMR guard at `:710`, and
nothing else -- there is no pre-make predicate, so a skip-quiets flag honoured
at the generation branch (`:629-633` and `:649-661`) cannot consult one. The
clause therefore names the three per-move rules it binds and states that LMP is
not among them. S109's own Scope concern 2 had reached the same conclusion and
left the field alone.

## What was left alone, deliberately

- **DEC-092's `Decision:` literals.** Proposal only; the owner moves them.
- **S136's `goal:` line and `adocs/plan.md:363`**, which both say "three
  divisions". Correcting them needs `adocs/plan.md` in `touches:`, which this
  step does not have. Noted in S136's body.
- **Pre-existing citation drift.** Editing S109 and S117 resets their
  `plan_prose_check` baselines, which is documented behaviour
  (`tools/plan_prose_check.py:240-244`: *"touching a step file relaxes its own
  drift check"*). Four DRIFT flags stop being reported as a side effect, none
  of them inspected or claimed fixed here: `S109:160` into
  `src/search_params.hpp:122-123`, and `S117:83`, `:92`, `:144` into
  `tests/test_search.cpp` and `tests/test_evaluation.cpp`.

## Cost

No match. Document work, with each edit traced to the harness file or decision
that settles it.
author:    Maksym Bodnar
