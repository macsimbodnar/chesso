id:         S006
goal:       negamax searches the captures before generating the quiets
accepts:    fixed-depth time falls; an SPRT against the immediately preceding commit returns a verdict; the verdict is recorded whatever it is
touches:    src/search.cpp negamax staging and the move picker
excludes:   staging inside quiescence
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 6afce96, result recorded in ca597fb. README and MANUAL checked at adoption, not when this shipped.

## Measurement -- -17 % at fixed depth, and **0 Elo**

```
unstaged   9.041  7.878  7.837 s
staged     7.346  6.281  6.779 s     about -17 %
```

SPRT against `62fbdcb`, this change and nothing else:

```
Games 340   W 102   L 98   D 140   Points 172.0 (50.59 %)
Elo   +4.09 +/- 27.52      nElo +5.50 +/- 36.93
Ptnml(0-2)  [13, 41, 57, 47, 12]
LLR    0.01   bounds (-2.20, 2.20)   [0.00, 10.00]
```

The LLR wandered between -0.10 and +0.15 over five checkpoints and never
trended. Stopped rather than left to grind.

## Why the speed did not become strength

The 17 % was given back by an ordering change nobody planned. Unstaged, every
score including the quiets was computed before any child search ran. Staged,
the quiet scores are computed after the capture stage has been searched, by
which point the killer, history and countermove tables have been updated by
those searches. Quiet-against-quiet ordering therefore differs, on fresher
data, and on these positions that is slightly worse. Faster nodes, more of them.

Predicted at 30-50 Elo from published reports. **Worth nothing here** -- this
engine had no LMR, no NMP and no PVS re-search at the time, so it searched far
more quiet moves than a comparable engine and the ordering mattered more than
the generation cost. See DEC-004.

**Kept** on two grounds that are not Elo: each node is cheaper, which shows
once the pruning above it exists, and it is the mechanism any further staging
needs.
