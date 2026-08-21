id:         S141
goal:       every pending step's touches field names the file its change has to land in
accepts:    S039's `touches:` names `src/search_params.hpp`, which is where `LAZY_EVAL_MARGIN` has lived since S073 and without which the step cannot change the constant its goal is about; S085's and S120's `touches:` likewise name the file their change lands in; a tracked check reads each pending step's `touches:` and reports a step whose goal names a symbol that no listed file contains, and it is run and green at completion
touches:    adocs/plan_todo/, tools/ for the checker
excludes:   implementing S039, S085 or S120; widening a touches field to cover work the step excludes
decisions:
closes:     2026-08-20_plan_review-F06
blocks:
paused_by:
done:

## Why a checker and not three edits

`touches:` is the field that says where a change is allowed to land, so a step
whose goal is to change a constant and whose `touches:` omits the file holding
that constant is a step that cannot be completed without violating its own
scope. S039 is the sharp case: its whole goal is to re-decide
`LAZY_EVAL_MARGIN` from measured spread, and the constant moved to
`src/search_params.hpp:98` when S073 built the tune build. S085 has the same
defect for the same reason and its own research section already flagged it in a
Scope concern -- which is evidence that catching this by reading does not scale,
and that a check does.

## Cost

No match. Document work plus a checker, and the fast suite green.
