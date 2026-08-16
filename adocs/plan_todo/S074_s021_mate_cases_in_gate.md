id:         S074
goal:       S021's accepts carries the fast suite's mate cases, as every other pending pruning step does
accepts:    `S021_aspiration_windows.md`'s `accepts:` names the three cases by their test titles -- "mate in one", "mate in two is found at the right distance" and "pruning does not hide a mate against the material leader" -- green at the window schedule that ships, before the SPRT verdict is read; the SPRT clause it already carries is unchanged
touches:    adocs/plan_todo/S021_aspiration_windows.md
excludes:   implementing aspiration windows, which is S021's; the window schedule, the re-search policy and the expected effect size; any other pending step's gate
decisions:  DEC-060
closes:     2026-08-16_plan_review-F07
blocks:
paused_by:
done:

## Why this exists

S021 is the first play-altering step after S068 and its whole gate is
`S021_aspiration_windows.md:3`:

> accepts:    an SPRT with bounds matched to the expected effect size returns a
> verdict

Every neighbouring step that touches pruning carries a mate clause and S021 is
the only one that does not: `S033:3` ("a position with a forced mate inside the
pruned depth is in the fast suite and passes before the feature is called
done"), `S068:3` ("every mate case in the fast suite stays green at the setting
that ships, the material-leader case included"), `S026:16` and S060 behind it.

## Why the omission is not cosmetic here in particular

DEC-060 measured that reverse futility's mate exposure is a property of **the
bound the parent passes down**, not of the eval:

> RFP ply=1 depth=2 alpha=-965 beta=-964 eval=-764 ret=-964
> [...] It fails high because `beta` is **-964**: the parent is a null-window
> scout hunting a mate score [...] so the guard the step prescribed does
> nothing.

and its Consequences say, in a sentence that also ships at `src/search.cpp:337`:
"A mate that first becomes visible below ply 3 can still be missed for an
iteration and nothing in the suite covers that. Stated, not fixed."

Aspiration windows change exactly that quantity -- the bounds every node below
the root inherits. Whether narrowing the root window moves any node's `beta`
into the shape DEC-060 traced is **unmeasured**, and the audit recorded it as
unconfirmed rather than as a defect. The step is cheap either way: S021's own
expected effect is +9 +/- 17, an effect size at which an SPRT would not separate
a missed mate from noise.

## Cost

Minutes to edit the gate. The gate itself costs one `ctest -L fast` run inside
S021, which S021 pays anyway.
