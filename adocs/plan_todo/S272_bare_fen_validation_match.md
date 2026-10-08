id:         S272
goal:       one SPRT on the workstation, both engines driven through a relay that sends every position as a bare FEN, measures what S271 bought for a client that sends the board
accepts:    (1) a relay under `tools/` that sits between fastchess and the engine, rewrites each `position ... moves ...` line into `position fen <the board it reaches>`, and passes every other line both ways unchanged, with a test over a recorded command log; (2) DEC-143's fixed-rounds A/A of 1000 games through the relay, both sides the same binary, read with `adocs/data/S105_pairs.py` before the verdict; (3) pre-registered in a script under `adocs/data/` before the first game: candidate S271's landing commit, reference its parent, both through the relay, `{0, 5}` nElo, 8+0.08, Hash=16, `noob_3moves.epd`, the worst-case expected games from the nElo formula, the abort rule and the three outcomes, and the open DEC-171 findings named (S259 while it is open); (4) the conversion's prediction from `adocs/data/S271_replay.md` (about +50 Elo at Hash 16) stated beside the result and never quoted as it; (5) the result block in the closing commit (DEC-220)
touches:    adocs/data/S271_replay.md
excludes:   any engine change; the harness's default regime, which keeps the base fixed; a match on the MacBook (DEC-258 (2))
decisions:  DEC-264, DEC-143, DEC-220, DEC-258
closes:
blocks:
paused_by:
author:
done:

## Why

S271 lands on node identity in the harness and on the replay's node counts;
neither is a game result. This match turns the replay's conversion into a
measured number, in the one regime where S271 changes play. The owner
recorded it as validation, to run on the workstation when the machine is
free, and only if it is still needed then (DEC-264): S271 does not wait on
it.

## Lane

A workstation run, under four hours by the replay's prediction; the relay is
agent work and may be written while another run holds the machine (DEC-260).
