id:         P04 (proposed; the S-id is allocated at adoption)
goal:       the rating reference set brackets 2750 to 3150, so a rating run can show 3000: at least two engine families with CCRL Blitz single-CPU entries between 2850 and 3200, added beside S088's five anchors
accepts:    (1) each candidate's CCRL Blitz rating and interval read on the day, single-CPU entry, licence noted; binary built or downloaded by the agent (DEC-069), md5 recorded in references.tsv; (2) one smoke match per new anchor under rating.sh's own settings: 0 crashes, 0 illegal moves, forfeits within DEC-075's 1 %; (3) rating.sh runs on the machine the owner names (O2) -- on this M1 it lacks `ordo` and GNU `timeout` (.moltke.local.md), and installing them is the owner's call (DEPS); (4) no rating run (DEC-108; P29 is the run)
touches:    references.tsv, rating.sh (only if an anchor needs an option), DEV_MANUAL.md, adocs/data/P04_anchors.md
excludes:   changing rating.sh's regime; the rating run itself
closes:
paused_by:
author:
done:

## Why

S240's top anchor, Leorik 2.4 at 2830, still outscored chesso; its record
says the next measurement needs an anchor above about 2850. A 3000 claim
solved from anchors that all sit below 2850 is an extrapolation.

## Candidates (direction; ratings re-read on the day)

- **Stash**, versions between v21.0 (2713, already an anchor) and v30 (3166):
  one family with documented ratings across the whole band, recommended as a
  gauntlet family by the engine-testing guide.
- One or two other families from today's 2900 to 3150 band: CuckooChess 1.13
  (3071), Gaviota 1.0 (2986), Crafty 25.3 (2975), DiscoCheck 5.2.1 (2931),
  Zurichess Neuchatel (2919).

Running another engine's binary is a tool use and creates no derivative work
(DEC-016).

## From the record

DEC-067, DEC-068, DEC-072 (a third family), DEC-075/076 (forfeits), DEC-077
(anchor spread is scale compression), DEC-234 and S240.
