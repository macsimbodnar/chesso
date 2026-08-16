id:         S071
goal:       plan.md and testing.md cite the checker by version and symbol, not by a path this repository does not have
accepts:    no line in `adocs/plan.md` or `adocs/testing.md` cites `bin/moltke.py` as if it were in this tree; each citation names the plugin version it was read at and the symbol it rests on (`PLAN_DONE_KEPT`, and the function that prunes), so a reader can find it under a different version; the claims themselves are unchanged and still true at the version named; `moltke --validate` clean
touches:    adocs/plan.md, adocs/testing.md
excludes:   the retention rule itself, which S064 and S053 established and which was re-verified by the 2026-08-16 audit; vendoring the checker; any change under `adocs/plan_done/`
decisions:
closes:     2026-08-16_plan_review-F10
blocks:
paused_by:
done:      Every bin/moltke.py citation in plan.md and testing.md now names moltke 0.11.0
                and the symbols prune_plan() and PLAN_DONE_KEPT, with the line numbers kept but
                demoted to 'at that version'. Paragraph-scoped check green, 0 bad blocks; red at
                HEAD, 3. The version is load-bearing: 0.1.0 is also installed and has neither
                symbol. status.md:25 and S069's ledger row re-pointed from plan.md:173-182 to
                :173-187, since the paragraph grew. moltke --validate clean; ctest -L fast 12/12
                in 20.07 s; clang-format.sh --check exit 0.

## Why this exists

`adocs/plan.md:130-131` and `adocs/testing.md:13` argue from a file that is not
here:

> `bin/moltke.py:1698-1700` collects the completed entries by position in the
> file and drops all but the final `PLAN_DONE_KEPT` of them, 5 at `:1681`.

```
$ ls /home/max/ws/chesso/bin/
ls: cannot access '/home/max/ws/chesso/bin/': No such file or directory
```

The claim is true. The audit resolved it against
`~/.claude/plugins/cache/moltke/moltke/0.11.0/bin/moltke.py`, where `:1681` is
`PLAN_DONE_KEPT = 5` and `:1698-1700` is the pair that collects and drops. Two
versions are installed, `0.1.0` and `0.11.0`, and the line numbers move between
them.

So the defect is not the claim, it is that the citation cannot be followed from
this repository and silently rots on a plugin upgrade that this repository does
not record.

## What a good citation looks like here

The version and the symbol, both. `PLAN_DONE_KEPT = 5` at moltke 0.11.0 is
findable by grep under any version and states what changed if the constant moves;
a bare line number states nothing once the file moves under it. This is the same
remedy S079 applies to the step files' `src/` citations, for the same reason.

## Cost

Minutes, no build, no match.
author:    Maksym Bodnar
