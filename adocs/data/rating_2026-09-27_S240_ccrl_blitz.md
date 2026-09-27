# chesso on the CCRL Blitz scale, S240, 2026-09-27

**chesso ~= 2766 on the CCRL Blitz scale (five-anchor mean), spread 94.1; +207
over S088's 2559 on the same five anchors.** The interval on the figure is no
narrower than about +/-60 from anchor choice and statistics alone, as
`adocs/specs.md` says of S088's; the per-anchor `ordo` +/-23 to +/-36 below is
one anchor's interval, not the interval on the number.

Engine: `1680439` (`bench` 4803214), the tree S237's and S238's removals left.
Run 2 of S240; run 1 was void (an anchor disconnected,
`S240_rating_run1_INVALID_report.txt`).

| anchor | its CCRL Blitz (read 2026-09-27) | chesso's score against it | chesso solves to | 95 % |
|---|---|---|---|---|
| Blunder 7.1.0 | 2388 +/-18 | 89.4 % | 2762.0 | +/-35.5 |
| Leorik 2.1 | 2568 +/-18 | 68.3 % | **2702.6** | +/-24.9 |
| Blunder 8.5.5 | 2662 +/-11 | 68.0 % | 2794.2 | +/-24.8 |
| Stash v21.0 | 2713 +/-14 | 58.5 % | 2773.1 | +/-24.0 |
| Leorik 2.4 | 2830 +/-11 | 45.3 % | 2796.7 | +/-23.3 |

**All five: mean 2765.7, spread 94.1. Without Leorik 2.1: mean 2781.5,
spread 34.7.** S088 on 2026-08-18: 2558.5 and 121.8; without Leorik 2.1,
2579.0 and 64.7. Leorik 2.1 is again the low outlier, as S088 found (DEC-077).
The top anchor, Leorik 2.4, still scores above chesso, so the set still
brackets it -- barely: the next measurement needs an anchor above ~2850.

## The run

| setting | S240 run 2 | S088 |
|---|---|---|
| time control | 10+0.2 | 10+0.2 |
| hash | 128 MB (hard-wired in `rating.sh` since) | 64 MB |
| concurrency | 12 of 12 | 6 |
| games | 3340, 668 per anchor | 3340 |
| book | `8moves_v3.pgn`, unseeded draw | the same book, a different draw |
| adjudication | two-sided (DEC-174, S212) | one-sided |
| wall | about 2 h 30 m | 5 h 03 m |

Every difference is the script's at HEAD, named in `specs.md` since S212;
none was patched for this run (S240 excludes it). Anchors' md5s matched
`references.tsv` on 2026-09-27 before the first game.

Terminations: 2813 adjudication, 524 normal, 3 time forfeits, 0 crashes or
disconnects. Forfeits: Stash v21.0 3 of 668 (0.45 %, inside DEC-075's 1 %),
every other engine 0; chesso gained 3 points from them. Evidence:
`S240_rating_report.txt`, `S240_rating_anchors.tsv`,
`S240_rating_forfeits.txt`; the PGN stays under
`.tuning/rating_S240r2_20260927_093311/`.

## Reading it

The self-play ledger since S088 kept eleven gainers and S085's fit, point
estimates summing to about +250; the coordinator's forecast from it, corrected
for early stopping, was 2600 to 2720. The measurement is above that band: the
self-play-to-gauntlet factor here was larger than assumed, or the stopping
bias smaller, and this run cannot say which. It cannot attribute the gain to
any change either.
