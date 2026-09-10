id:         S212
goal:       the two match harnesses adjudicate resignation two-sided as their comment already claims, the engine says which build it is and `fastchess.sh` checks that against what it labelled, a cached reference is checked before it is played, a crash voids a run, and the busy-machine guard measures load -- then one fixed-rounds A/A re-calibrates the harness as DEC-143 requires
accepts:    `fastchess.sh` and `rating.sh` pass `-resign movecount=3 score=400 twosided=true` and the inverted comment in `rating.sh` ("Both are two-sided") is true after the change instead of before it, `score=400` kept and the reason recorded (DEC-174); `id name` in `src/chesso.cpp` `command_uci` carries the short sha from `git describe --always --dirty` and the `CHESSO_ARCH` value, stamped at configure time through a generated header, with `tune` appended on `CHESSO_TUNE=ON`, and `MANUAL.md` and `adocs/specs.md` state the form before `test_uci_surface`'s `id name Chesso` check becomes a pattern check (SURFACE); `fastchess.sh` asks each side `uci` before the first game and refuses the run when a side's `id name` sha is not the sha it labelled that side with, the way `rating.sh` already refuses a mismatched opponent; a cached `.ref-builds/<sha>` is played only if its worktree is clean and at `<sha>` and its `CMakeCache.txt` carries the same `CHESSO_ARCH` and `CHESSO_TUNE` as `build/`'s, otherwise it is rebuilt, and the banner prints the candidate's configuration; a game ending by crash or disconnect voids the run with a `SPRT-RUN-INVALID` line beside the existing markers and the PGN census counts them, matching `rating.sh`'s `RATING-RUN-INVALID`; the busy guard reads the one-minute load average against the core count instead of summing `ps` lifetime percentages, and its threshold is stated; `tests/test_fastchess_script.sh` gains one property per behaviour, each observed red against `git show HEAD:fastchess.sh` saved to a file and green after, and `tests/test_rating_script.sh` covers the `twosided` token; **then** `ROUNDS=500 AA=1 ./fastchess.sh` plays 1000 fixed-rounds games and `adocs/data/S198_pairs.py` reads the pair variance against the S105 and S198 bands (DEC-143), with games an hour recorded beside the S198 figure so the throughput cost of `twosided`, if any, is a number; the `adocs/specs.md` sentence about what `rating.sh` re-derives, corrected on 2026-09-11 to say 2559 is not re-derivable at HEAD, is either left as the truth or `rating.sh` gains a flag that reproduces the S088 regime (Hash 64, concurrency 6, 334 unseeded openings), and the stamp says which
touches:    fastchess.sh, rating.sh, src/chesso.cpp, CMakeLists.txt, src/CMakeLists.txt, cmake/, tests/test_fastchess_script.sh, tests/test_rating_script.sh, tests/test_uci_surface.cpp (golden), tests/test_engine.cpp, MANUAL.md, DEV_MANUAL.md, adocs/specs.md, adocs/data/
excludes:   the resign score (600 is fishtest's and the audit's note; changing it is a throughput trade that gets its own decision if wanted); the draw adjudication, already effectively two-sided; re-scoring `S088_rated_c6.pgn` per side, which the audit already did and F04's table records; any change to bounds, book, control or hash
decisions:  DEC-170, DEC-174
closes:     2026-09-10_adversarial-F04, 2026-09-10_adversarial-F05, 2026-09-10_adversarial-F06, 2026-09-10_adversarial-F07, 2026-09-10_adversarial-F31, 2026-09-10_adversarial-F32
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-10_adversarial` Part B, the second foundation: *nothing is believed
without a measurement*. Six findings about the instrument, one of them high.

**F04, high.** Both harnesses pass `-resign movecount=3 score=400` and neither
passes `twosided`; the installed `fastchess alpha 1.8.1 20260720-daa3ea2` says
`twosided` "Defaults to false". `rating.sh`'s comment says the opposite and
names the property the rating run depends on. Counted from the tracked PGNs:
76 % of the A/A's games and 84 % of the S088 rating run's ended on an engine's
own score; in the A/A 11 of 676 decisive adjudications (1.6 %) were one-sided,
in the rating run **514 of 2627 (19.6 %)** -- chesso conceded alone 311 times
and its opponents 203, opponent-specific in direction, and the per-anchor
concession rate correlates with that anchor's solved rating at r = -0.505 on
n = 5. A hypothesis and not a cause, never considered by DEC-077, and free to
test on a PGN already on disk. **For self-play the exposure is the 1.6 %
floor**, which is why it has not corrupted the SPRT ledger -- and why
`twosided=true` costs the SPRT harness almost nothing in throughput while
removing the hazard the evaluation block will create when a candidate's scale
moves (S039, S122, S126).

**F05.** `id name Chesso`, a literal; the PGN's engine names are the script's
assertion, not what ran. `rating.sh` refuses an opponent whose `id name`
mismatches the manifest (DEC-068, DEC-069, DEC-072) and nothing does that for
chesso. DEC-020's +301 Elo is this class.

**F06.** `.ref-builds/<sha>` is built only when the binary is absent; nothing
checks the worktree is still at `<sha>`, clean, or configured like the
candidate. `.ref-builds/2b54a4f` is dirty today and 11 of 30 cached binaries
predate S104 and carry no `popcnt`. No recorded verdict is contaminated -- the
mtimes and the popcnt split fall where they should -- and `DEV_MANUAL.md`
teaches a `REF=7b4d9a4` that today serves a 2026-08-13 binary against a
native candidate, a +12.62 % nps difference by S104's measurement.

**F07.** `adocs/specs.md` said `./rating.sh` re-derives 2559; it runs Hash 128
(S088 ran 64), every core (S088 ran 6, "this figure only"), and a different
unseeded opening draw, and the concurrency-12 variant of that run was voided
by the project as `S088_rated_c12_INVALID.pgn`. The sentence was corrected on
2026-09-11 when this step was filed; whether the S088 regime gets a flag is
this step's call.

**F31.** `fastchess.sh` censuses crashes and disconnects but never voids on
them; `rating.sh` does. The justification covers time forfeits only.

**F32.** The busy guard sums `ps -A -o %cpu=`, each process's lifetime
average: loadavg 0.79 read as 255 %, so it fires on every run and carries no
information. `CLAUDE.md`'s founding rule 4 recommends the same statistic; the
step corrects that sentence too.

## Order inside the step

Harness edits and their sandbox properties first, the engine's `id name`
second (it is a `src/` change and needs the idle machine for its gate run),
documents third, the A/A last -- because DEC-143 says the A/A follows the
harness change and precedes the next verdict, and S024's verdict is the next
entry that owns one.

## Cost

Agent work, half a day; one 1000-game fixed-rounds A/A, about 26 minutes at
the S198 throughput, under the four-hour line (DEC-155) and so a daytime run.
The `id name` commit touches `src/` and carries `No functional change`.
