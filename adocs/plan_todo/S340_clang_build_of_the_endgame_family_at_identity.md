id:         S340
goal:       the tree builds under clang with `-Werror` whatever S124's family ships, its identity included
accepts:    (1) `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE` on a tree whose `endgame_scale` is all 128 and whose mop-up arrays are all zero, where at `7214ce5` it ends `GATE-EXTRA-FAILED: clang`, the failure seen first; (2) the same on a tree with mop-up live and the factor at its identity, and on one with the factor live and mop-up zero, the two bisection parts' shapes, each by a throwaway edit of the three arrays; (3) `chesso bench` reads the parent's total in both gated builds and `tools/search_bench.py` reads identical node counts and best moves (INV-6); the commit says `No functional change`; (4) no warning is silenced tree-wide and no flag is added or removed: the two functions say at their sites why a build may not need them
touches:    src/evaluation.cpp, DEV_MANUAL.md
excludes:   any change to what the evaluation computes; `tools/gate_extra.sh` and `tools/gate.sh`; moving the clang stage into the automatic gate, which is a decision of its own
decisions:  DEC-171, DEC-141, DEC-350
closes:
blocks:
paused_by:
author:
done:

## Why

Found by the weekly `tools/gate_extra.sh` on `7214ce5`, 2026-11-12 00:09
(`.tuning/gate_extra_2026-11-11.log`; prose, citations, debug, sanitize and perft ok, clang exit 1):

```
src/evaluation.cpp:2076:20: error: function 'mopup_terms' is not needed and will not be emitted [-Werror,-Wunneeded-internal-declaration]
src/evaluation.cpp:2097:19: error: function 'scaled_taper' is not needed and will not be emitted [-Werror,-Wunneeded-internal-declaration]
```

in `build-clang` and `build-clang-tune` alike. Both functions are called only under
`if constexpr (mopup_live || trace_build)` and `if constexpr (endgame_scale_live || trace_build)`, and at the identity
both flags are false in the engine library, so clang sees two `static inline` functions no translation unit needs. GCC,
which both gated builds use, says nothing. The tree has been in this state since S124's phase 1 landed the family at its
identity on 2026-11-06; the last `GATE-EXTRA-DONE` before that was 2026-11-05.

What it breaks: any clang build with `-Werror`, the owner's MacBook's among them (S297, S298), of every commit from
phase 1's landing on. It moves no play, no score and no UCI output, so by DEC-171 it is a filler step, named in the
pre-registration of every run taken while it is open. S124's SPRT is the first such run.

It binds whatever S124's verdict is. On H1 the pasted values make both functions needed and the warning goes with them,
but only while both stay live; on H0 the identity stays, and each bisection part leaves one of the two unneeded.

## Shape

One Opus subagent, agent-only, no run. The fix is at the two definitions, not in the build flags. It lands when no
timed run holds the machine, and not in a worktree a run is playing from.
