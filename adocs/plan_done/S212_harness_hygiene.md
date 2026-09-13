id:         S212
goal:       the two match harnesses adjudicate resignation two-sided as their comment already claims, the engine says which build it is and `fastchess.sh` checks that against what it labelled, a cached reference is checked before it is played, a crash voids a run, and the busy-machine guard measures load -- then one fixed-rounds A/A re-calibrates the harness as DEC-143 requires
accepts:    `fastchess.sh` and `rating.sh` pass `-resign movecount=3 score=400 twosided=true` and the inverted comment in `rating.sh` ("Both are two-sided") is true after the change instead of before it, `score=400` kept and the reason recorded (DEC-174); `id name` in `src/chesso.cpp` `command_uci` carries the short sha from `git describe --always --dirty` and the `CHESSO_ARCH` value, stamped at configure time through a generated header, with `tune` appended on `CHESSO_TUNE=ON`, and `MANUAL.md` and `adocs/specs.md` state the form before `test_uci_surface`'s `id name Chesso` check becomes a pattern check (SURFACE); `fastchess.sh` asks each side `uci` before the first game and refuses the run when a side's `id name` sha is not the sha it labelled that side with, the way `rating.sh` already refuses a mismatched opponent; a cached `.ref-builds/<sha>` is played only if its worktree is clean and at `<sha>` and its `CMakeCache.txt` carries the same `CHESSO_ARCH` and `CHESSO_TUNE` as `build/`'s, otherwise it is rebuilt, and the banner prints the candidate's configuration; a game ending by crash or disconnect voids the run with a `SPRT-RUN-INVALID` line beside the existing markers and the PGN census counts them, matching `rating.sh`'s `RATING-RUN-INVALID`; the busy guard reads the one-minute load average against the core count instead of summing `ps` lifetime percentages, and its threshold is stated; `tests/test_fastchess_script.sh` gains one property per behaviour, each observed red against `git show HEAD:fastchess.sh` saved to a file and green after, and `tests/test_rating_script.sh` covers the `twosided` token; **then** `ROUNDS=500 AA=1 ./fastchess.sh` plays 1000 fixed-rounds games and `adocs/data/S198_pairs.py` reads the pair variance against the S105 and S198 bands (DEC-143), with games an hour recorded beside the S198 figure so the throughput cost of `twosided`, if any, is a number; the `adocs/specs.md` sentence about what `rating.sh` re-derives, corrected on 2026-09-11 to say 2559 is not re-derivable at HEAD, is either left as the truth or `rating.sh` gains a flag that reproduces the S088 regime (Hash 64, concurrency 6, 334 unseeded openings), and the stamp says which
touches:    fastchess.sh, rating.sh, src/chesso.cpp, CMakeLists.txt, src/CMakeLists.txt, cmake/, tests/test_fastchess_script.sh, tests/test_rating_script.sh, tests/test_uci_surface.cpp (golden), tests/test_engine.cpp, MANUAL.md, DEV_MANUAL.md, adocs/specs.md, adocs/data/
excludes:   the resign score (600 is fishtest's and the audit's note; changing it is a throughput trade that gets its own decision if wanted); the draw adjudication, already effectively two-sided; re-scoring `S088_rated_c6.pgn` per side, which the audit already did and F04's table records; any change to bounds, book, control or hash
decisions:  DEC-170, DEC-174
closes:     2026-09-10_adversarial-F04, 2026-09-10_adversarial-F05, 2026-09-10_adversarial-F06, 2026-09-10_adversarial-F07, 2026-09-10_adversarial-F31, 2026-09-10_adversarial-F32
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); the closing A/A is the coordinator's; started 2026-09-13 03:23 on the idle machine
done:       2026-09-13 05:00. **The instrument is fixed in six places and the A/A that follows says its statistics did not move.** The implementation half landed in `f9d705c` (its "Landed" section below): `-resign movecount=3 score=400 twosided=true` in both harnesses (F04, DEC-174); `id name Chesso <sha>[-dirty] <arch>[ tune]` from `cmake/build_info.cmake`'s build-time header and `fastchess.sh`'s `check_identity` refusing a side whose sha is not the label's (F05); a cached `.ref-builds/<sha>` played only if clean, at its sha and configured like `build/`, else rebuilt with that configuration, the banner printing `config arch native tune off` (F06); a termination outside `normal`, `adjudication` and `time forfeit` printing `SPRT-RUN-INVALID` and then `SPRT-RUN-FAILED` last (F31); the busy guard on the one-minute load average at 0.25 per core (F32); the S088 regime gets no flag (F07); twelve properties observed red first; `bench` 27322394 unchanged, `No functional change`; DEC-204 records the four rulings. **The A/A, pre-registered in `.tuning/coord/s212_aa.sh`** (1000 games, working tree against itself, default regime, estimate 28.4 min at 2110 an hour, ceiling 3400 s): launched 04:17 after the rebuild the stamp demands -- the identity lines `Chesso f9d705c native` printed for both sides -- `SPRT-RUN-DONE` at 04:45:38, **1000 games in 28 m 08 s, 2133 an hour** against S219's 2110 on the same book, so two-sided resignation costs nothing measurable; `Elo 8.69 +/- 16.53`, `nElo 11.33 +/- 21.53`, `Ptnml [42, 106, 187, 115, 50]`, 0 forfeits either side, 739 adjudications and 261 natural ends, the crash census quiet; **pair variance 0.2939 +/- 0.0186 against the current book's 0.2905 +/- 0.0184 (S219, DEC-190), z +0.13, inside the band** (`adocs/data/S212_aa_pairs.txt`; `adocs/data/S198_pairs.py`'s band constant moved from S105's UHO-book PGN to S219's in this commit, because against the old book every run on this one read 'outside' by construction). Evidence `adocs/data/S212_aa.log`. The fast check over the implementation found nothing that could corrupt a measurement; one coverage gap stands (no property plays a `-dirty` stamp against a clean tree) and is noted, not a finding. `tools/gate_extra.sh` not owed (no `make_move`, generator or search change). MANUAL.md and DEV_MANUAL.md updated in the implementing commit; checked again here, no further change. By an Opus 5 subagent for the implementation and the coordinator for the run and the reading (DEC-185, DEC-199).

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

## Landed 2026-09-13 03:52

Everything in the accepts except the closing A/A, which is the coordinator's
and which the `done:` stamp waits on.

**F04, two-sided resign.** `fastchess.sh` and `rating.sh` both pass
`-resign movecount=3 score=400 twosided=true`. `rating.sh`'s comment claiming
the property is unchanged and is now true; the paragraph under it says in as
many words that it became true on this date and was written before it, and
carries the re-scored 514 of 2627 with its per-anchor direction. `score=400`
and `movecount=3` are untouched, with DEC-174's reason beside them in both
scripts. Red first: `.tuning/coord/S212_head_fastchess.sh` failed
*"the resign adjudication did not reach fastchess as 'twosided=true'"*, and
`.tuning/coord/S212_head_rating.sh` failed the same assertion as case 6.

**F06, cached reference validity.** `fastchess.sh` `ref_cache_ok` asks three
questions of `.ref-builds/<sha>` -- a worktree with `git status --porcelain`
empty, at `<sha>`, and a `CMakeCache.txt` carrying the `CHESSO_ARCH` and
`CHESSO_TUNE` `build/`'s does -- and any `no` clears the worktree through
`git worktree remove --force` and rebuilds. `build_ref` now *configures* with
that same pair, which is what stops the check looping: a reference configured
with bare defaults and judged against `build/`'s would be stale and rebuilt on
every run. Verified out of band that a commit predating the options keeps
them as `CHESSO_ARCH:UNINITIALIZED=native` in its cache, so old references are
stable too. The banner's `config` line is that pair. Reds, all four:
*"dirty cached reference: 417fc40 was not rebuilt before it was played"*,
*"moved cached reference: ... was not rebuilt"*, *"a cached reference built for
another CHESSO_ARCH was played unrebuilt"*, *"the banner does not print the
candidate's arch and tune"* and *"... does not read the candidate's
configuration from build/CMakeCache.txt"*. Case 22 -- a valid cache played
with no build -- was green against HEAD by construction and is the negative
control that stops the check being written as "rebuild always".

**F05, `id name`.** `src/chesso.cpp` `command_uci` answers
`id name Chesso <sha>[-dirty] <arch>[ tune]`, one string literal concatenated
at compile time from `cmake/build_info.cmake`'s header. Observed:
`id name Chesso 47be85b-dirty native` from `build/` and
`id name Chesso 47be85b-dirty native tune` from `build-tune/`.

**Build time, not configure time, and the accepts' intent is met by it.** The
accepts says "at configure time"; a stamp captured by `cmake -S . -B build`
goes stale on the next commit, and the check this same step adds to
`fastchess.sh` would then refuse a run whose binary is perfectly good. S077
had already built the build-time mechanism for the tuner's provenance line and
had already written down that reason; this step extends that script with the
arch and the tune flag and moves its custom target from `tools/CMakeLists.txt`
to the top-level `CMakeLists.txt`, because it now has two readers. The header
moved with it, `build/tools/generated/` to `build/generated/`.

**The sha is `git rev-parse --short` and not `git describe --always --dirty`.**
The accepts names the latter; this repository carries nine tags from the
pre-`achesso` history, so `describe` answers `v0.3.0-520-g47be85b`, which is
not a short sha and does not compare against the `git rev-parse --short` that
`fastchess.sh` labels each side with. The value the accepts asks for -- the
short sha, `-dirty` when the tree is dirty -- is exactly what the commands
already in `cmake/build_info.cmake` produce.

**The check, and the one answer it accepts without checking.**
`fastchess.sh` `check_identity` asks each side `uci` through a GNU `timeout`,
with `rating.sh` `identify`'s guards verbatim, and refuses when the sha
disagrees with the label. A binary answering the bare literal `id name Chesso`
-- every commit before this one -- plays with a printed line saying the check
did not happen, because refusing would make `REF=<any older sha>` unrunnable,
which is most of what the reference mechanism is for. `-dirty` is allowed on
the candidate when the banner shows `+ uncommitted changes` and never on the
reference; allowed and not required, because the dirty flag is raised by any
tracked file and requiring it would refuse a run whose binary is correct
because a step file was edited after the build. Reds: *"a candidate answering
another commit's sha was played anyway"* and *"a reference predating the build
stamp played without saying so"*.

**SURFACE order.** `MANUAL.md` describes the form (the sample session and a
paragraph under it) and the `adocs/specs.md` wording is proposed to the
coordinator for the same commit; only then does `tests/test_uci_surface.cpp`
"the uci reply carries the identification a GUI needs" become a pattern check.
It is a `std::regex` over the whole line plus a both-directions assertion on
` tune`, and it names the two commands that re-derive what it must match
(DEC-142).

**F31, a crash voids the run.** `fastchess.sh` counts terminations outside
`normal`, `adjudication` and `time forfeit` -- `rating.sh`'s own census, so a
failure cannot hide behind a spelling nobody guessed -- prints
`SPRT-RUN-INVALID: <n> crashes/disconnects` and then calls `fail`, so the last
line is `SPRT-RUN-FAILED` and every watcher still terminates. Reds: *"a run
with a disconnect termination exited 0"*. The negative control -- a PGN of
`adjudication` and `time forfeit` reaching `SPRT-RUN-DONE` -- was green
against HEAD and stops the case passing for the wrong reason.

**F32, the busy guard.** Both scripts read the one-minute load average from
`/proc/loadavg`, falling back to `sysctl -n vm.loadavg` and then to a printed
non-check, and warn above **0.25 per core** -- 3.00 of 12 here. The threshold
is argued in the comment: a match already books every core (DEC-050), so what
matters is not whether anything else runs but whether enough runs that the
games queue behind it, and an idle machine here reads 0.00 to 1.00 one-minute,
so the guard stays quiet and therefore carries information. It warns and
refuses nothing, as before. Reds: *"the busy guard still sums ps lifetime
percentages"*, *"the busy guard does not read the one-minute load average"*.

**F07, and it does not get a flag.** The `adocs/specs.md` sentence corrected on
2026-09-11 stays as the truth. Three reasons, the third decisive: only
`hash_mb` is hard-wired (concurrency and rounds are already overrides), so a
flag is small but buys little; the opening draw cannot be reproduced at all,
since `rating.sh` passes no `-srand`; and **this same step changes the
adjudication**, so no invocation of `rating.sh` at HEAD can play S088's
experiment whatever its hash and concurrency. A flag named for reproducing
S088 would assert a reproducibility the commit that added it falsifies.

**Bench and INV-6.** `./build/src/chesso bench` totals **27322394 nodes**
before and after -- the commit carries `No functional change`.
`tools/search_bench.py ./build/src/chesso 9` identical on both:
midgame 121515 `c3d5`, kiwipete 801408 `e2a6`, tactical 72895 `d7c8q`.

**Tests.** `tests/test_fastchess_script.sh` 17 properties to 26, each new one
observed red against `.tuning/coord/S212_head_fastchess.sh` and green after;
the sandbox now builds real `git worktree` caches with a `CMakeCache.txt` and
stub engines that answer `id name`, which is what the new cases need and what
the old ones tolerate unchanged. `tests/test_rating_script.sh` 5 properties to
6. Gate green in both builds -- `100% tests passed, 0 tests failed out of 37`
twice, then `./clang-format.sh --check` -- with `CLANG_FORMAT_MAJOR=22`
exported and `pipefail` set, log at `.tuning/coord/S212_gate.log`.
`test_fastchess_script` costs 2.29 s of its 60 s ctest timeout.

**Left alone, deliberately.** The profiling adjudication DEC-031 documents
(`-resign movecount=8 score=900`) is a different regime for a different
question. The `extra` block in `adocs/data/S085_spsa_run.json` is a frozen
record of a past run and is history; the next SPSA config is written by the
step that runs it and wants `twosided=true` in it -- named here because
nothing else will.

## Cost

Agent work, half a day; one 1000-game fixed-rounds A/A, about 26 minutes at
the S198 throughput, under the four-hour line (DEC-155) and so a daytime run.
The `id name` commit touches `src/` and carries `No functional change`.
