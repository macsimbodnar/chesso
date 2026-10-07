id:         P29 (proposed; the S-id is allocated at adoption)
goal:       chesso rated on the CCRL Blitz scale against an anchor set that brackets 3000
accepts:    (1) triggered by the owner (DEC-074, DEC-108) when T4's projection is at least 2970; (2) rating.sh with S088's anchors plus P04's, every md5 checked against references.tsv before the first game; forfeits within DEC-075's 1 %; (3) solved with ordo on each anchor in turn; five-anchor-style mean and spread over the full set, the interval stated with the anchor dispersion term (specs: no narrower than about ±60); (4) the record under adocs/data/rating_<date>.md with the report trimmed by tools/trim_console.py (DEC-235); (5) specs.md's measured-strength paragraph rewritten; (6) the self-play to gauntlet factor re-estimated from the H1 point estimates since S240 against the measured change
touches:    adocs/data/rating_*, adocs/specs.md
excludes:   changing rating.sh's regime in the same step
closes:
paused_by:
author:
done:

## From the record

`specs.md` names a near-3000 rating run already owed with both time controls;
if that run is kept at adoption, P29 is that run with P04's anchors added,
not a second one.
