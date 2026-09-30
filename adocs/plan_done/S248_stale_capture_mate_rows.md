id:         S248
goal:       the capture-mate table's mined rows in `tests/test_search.cpp` read what their script derives on the tree that ships
accepts:    `adocs/data/S230_mine_r01_row.py depths` re-run on the shipping tree gives the rows the file carries, labels included, with the old rows quoted at the site (DEC-142, DEC-233); moot and closed by the boundary if S114 verdict 1's landing stays, whose rows were derived on its own tree; executed if S114 is reverted, since a byte-for-byte restore brings the parent's rows back
touches:    tests/test_search.cpp (four GOLDEN rows)
excludes:   the miner's rule; any row that is not mined
decisions:  DEC-142, DEC-233, DEC-171
closes:
blocks:
paused_by:
author:     executed inside S114 verdict 1's removal by its Opus subagent, briefed by the coordinator (DEC-185); the coordinator stamps it
done:       2026-09-30 -- executed, not moot: S114 verdict 1 read H0 and its removal `f3868fb` restored the parent's capture-mate rows byte for byte, so the same commit re-derived them on the reverted tree by `adocs/data/S230_mine_r01_row.py depths`, seven sweeps byte for byte the parent sweeps of S114's rebase: depths 7, 9, 11, 10 -> **7, 7, 10, 10**, row 3 labelled "C02, R02, since S248", row 4 "R02, since S248", no mate distance moved, the old rows quoted at the site (DEC-142, DEC-233). The removal's cold fast check read the rows; both fast suites green at them; the S170 budgets followed on the same tree as `f5eaa99`. A filler under DEC-171, named by id in `adocs/data/S114_sprt.sh`'s open findings while it was open.

## Why this exists (2026-09-30)

Rebasing S114 verdict 1 onto the tree without S022's early-out
(2026-09-29), the same seven sweeps of `S230_mine_r01_row.py` on that parent
tree gave depths 7, 7, 10, 10 (row 3 labelled C02, R02; row 4 R02) against
the committed 7, 9, 11, 10: the parent's own rows were already stale before
S114, which predates that step and reaches no play. S114's landing carries
rows derived on its own tree, so this is moot while S114 stays; on an H0 the
restore brings the stale rows back and this step re-derives them. A filler
behind S114 (DEC-171), named in `adocs/data/S114_sprt.sh`'s open findings.
