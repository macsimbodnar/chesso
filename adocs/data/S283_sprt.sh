#!/usr/bin/env bash
#
# S283: S099'S PAIR AT THE LONGER CONTROL -- the pawn key and the pawn-keyed
# correction against the tree before them, one SPRT at 32+0.32, Hash 64,
# {0, 10} nElo, by the owner's ruling (DEC-286).
#
#   nohup adocs/data/S283_sprt.sh > .tuning/s283_sprt.log 2>&1 &
#
# Written 2026-10-15, blind, on `4e6ca63` while S023 verdict 1's SPRT held the
# machine (DEC-260), and brought onto `be48e02` the same day, blind, while
# S039's phase 2 held it: nothing here was run, built or timed; every figure
# below is read from a file or a git object and names it.
#
# THE PAIR, AND WHAT IT ALREADY READ. Both commits exist and are pinned now:
#
#   CAND  `8758622` (2026-10-14), S099: the branchless pawn key
#         (`board_t::pawn_hash`, kept by add_piece, remove_piece and
#         move_piece) and `pawn_correction_t`, 4096 slots a side, read by
#         `negamax_at` over the raw static evaluation and taught a
#         moving-average step of `best - raw`; `CorrHistMax` 47,
#         `CorrHistWeightCap` 144. `adocs/data/S099_sprt.sh` has the whole
#         candidate, its seeds and their derivations; nothing here changes it
#         (the step's excludes). Bench 5636835 -> 6042933 against REF, by that
#         script's header.
#   REF   `bacead1` (2026-10-14), S261's closing commit on its H1: the tree
#         before the key.
#
# CAND's parent is `e2c256c`, three commits past REF (`6fc7f23` S280,
# `c5bd0de` and `e2c256c` DEC-284); `git diff bacead1 e2c256c` touches
# DEV_MANUAL.md, adocs/ and tools/mutation_check.py, nothing in src/, read
# 2026-10-15. The script refuses unless that stays so, as S099's did.
#
# Its two readings, quoted and not re-derived:
#   - S099's SPRT, `{0, 5}` nElo at 8+0.08, Hash 16
#     (`adocs/data/S099_sprt.log`): **H0**, `Elo -3.47 +/- 5.03`, `nElo
#     -4.71 +/- 6.84`, LLR -2.96, 9916 games, Ptnml [377, 1247, 1791, 1184,
#     359], 0 forfeits, 4 h 36 m.
#     Reverted whole as `5a04ffa`.
#   - DEC-270 (3)'s estimate, the same pair at 32+0.32, Hash 64, a fixed 1000
#     pairs (`adocs/data/S099_ltc.log`): `Elo +8.34 +/- 10.20`, `nElo +12.46
#     +/- 15.23`, 2000 games, W 500 L 452 D 1048, Ptnml [54, 215, 408, 275,
#     48], 0 forfeits, 3 h 42 m 38 s. An estimate, never a verdict; positive,
#     so the owner ruled (DEC-286): this run.
#
# The pair is fixed and independent of the tree, so what lands before the run
# -- S023 verdict 1's revert `0cdd39a` and S039's candidate (DEC-290)
# included -- does not change what it measures (the step file's "Why this
# exists"). The script runs fastchess.sh from the then-tree; both sides are
# built from their own commits.
#
# DEC-221 and the copying rules: this run measures S099's candidate as it was
# built, from the Chess Programming Wiki's description and published prose;
# `S099_sprt.sh` carries its provenance and its anti-seeds. Nothing is added.
#
# THE HARNESS. At `4e6ca63`, where this was first written, fastchess.sh could
# take neither this run's bounds nor a round cap that keeps the SPRT: the mode
# argument fixed the pair (`--fast` alone gave `{0, 10}`, at alpha = beta =
# 0.10 and 1500 rounds) and `ROUNDS` empties the SPRT. **S284 (`769ce7a`,
# DEC-289) closed that gap** under the names this script proposed. Every
# citation below is to `fastchess.sh` at `be48e02`:
#
#   REF, CAND    :169 and :178, resolved at :452 and :459, built by
#                `build_ref` (:755) at :807 and :853; under CAND the working
#                tree is neither read nor played
#   TC           :206 `tc="${TC:-8+0.08}"`, used at :1126 `-each tc=`
#   HASH         :219 `hash="${HASH:-16}"`, used at :1126 `option.Hash=`
#   SPRT_BOUNDS  usage :18-20; the block :364-424 checks the form, elo0 below
#                elo1 and alpha, beta inside (0, 0.5) (:395-409) and sets
#                `sprt` at :410; :442 appends `model=normalized`; used at
#                :1127
#   SPRT_ROUNDS  usage :21-25; checked at :415-418, the cap set at :419-420,
#                used at :1130 `-rounds`; the SPRT kept
#   either one   refused beside a mode argument (:390-391) or beside `ROUNDS`
#                (:392-393), so the exec passes neither; a `ROUNDS` left in
#                the caller's environment refuses the run rather than changing
#                it. The tag becomes `env` (:423): the PGN's event (:1114)
#                and the done marker (:1239) read `env`, not `full`
#   OUT          :576, the directory :578-579 put the log and the PGN in
#   banner       :984 `tc 32+0.32  hash 64 ...`, :993 `bounds ...`, and S284's
#                cap line :997, `cap        5250 rounds, 10500 games, from
#                SPRT_ROUNDS; no bound by then is no verdict`
#
# CONCURRENCY (:287, every core) and the book (:194, `noob_3moves.epd`, no
# variable) are this run's settings by default.
#
# The guard below is kept: it refuses unless `./fastchess.sh` expands both
# names, because an unread variable is ignored without a word and the run
# would play `{0, 5}` to 20000 rounds -- 40000 games, about 75 h here -- under
# this file's name. Read by grep on `be48e02`'s fastchess.sh, 2026-10-15: the
# guard's pattern finds `SPRT_BOUNDS` at :386, :395, :398, :399 and :410 and
# `SPRT_ROUNDS` at :387, :415, :416, :417 and :419, so it passes there.
#
# BOUNDS. `elo0=0 elo1=10 alpha=0.05 beta=0.05`, `model=normalized` -- nElo
# (fastchess.sh:44-57 at `be48e02`, S157), appended by fastchess.sh itself at
# :442. The pair DEC-286 names; `{0, 5}` at this control was rejected there at
# about 77 h worst case.
#
# WHAT THE PAIR COSTS (DEC-143). `adocs/testing_strategy.md` section 1.1:
# D / (e1 - e0)^2 games with the truth at the interval's midpoint, D = 1046535
# at alpha = beta = 0.05, and 639770 / (e1 - e0)^2 with it on a bound. Here
# e1 - e0 = 10:
#
#   truth at the midpoint   **10465 games**   19.6 h at 534/h   19.2 h at 545/h
#   truth on a bound        **6398 games**    12.0 h at 534/h   11.7 h at 545/h
#
# (the table's own `{-5, 5}` row, the same width, reads 10465 and 6398). The
# throughputs are `.moltke.local.md`'s two measured at 32+0.32, Hash 64, on
# `noob_3moves.epd`, concurrency 12: **545 games an hour** (S151, 2026-09-12,
# governor `performance`) and **534** (the S109 block's reading, 2026-09-13);
# budgeted on the lower. `S099_ltc.log`'s own 2000 games in 3 h 42 m 38 s are
# 539 an hour, between the two. DEC-286's "about 10500 games ... about 19 h at
# 540 games an hour" agrees.
#
# THE CAP. **5250 rounds, 10500 games**: the midpoint figure, 10465, rounded up
# to a whole hundred, as S099's 40000 sat at its own 41861 -- **19.7 h at 534
# an hour** (19.3 h at 545). A cap at the midpoint figure means a truth near
# the interval's middle, about +5 nElo, can end the run at the cap with
# neither bound reached; that outcome is pre-registered below. Four hours or
# more, so the run starts by DEC-155's night rule. Watcher ceiling 144000 s
# (40 h), at least twice the cap, and wall-clock only while the machine is
# awake.
#
# REGIME. 32+0.32, Hash 64 (DEC-286; the pair that keeps DEC-088's table
# pressure at four times the clock, fastchess.sh:208-219 at `be48e02`, S151),
# one thread a side, `books/noob_3moves.epd` (DEC-189), concurrency 12
# (DEC-050), fastchess.sh's adjudication, whatever governor the machine is
# under, recorded and not set (DEC-195). The one exception to the 8+0.08
# harness, by the owner, for one pair (DEC-286's consequences). If fastchess,
# the book, the adjudication or the machine has moved since the last
# fixed-rounds A/A, that A/A of 1000 games comes first (DEC-143); S284 moved
# none of them and owes none (DEC-289); the coordinator checks on the day it
# launches.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Stop it and report nothing, too, if
# the machine loses mains power (POWER) or another load starts on it while the
# run plays (MACHINE), or if the banner below is not what it prints. Nothing
# else stops it -- not the length.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES. fastchess.sh prints it at
# :971-1002 and the identity lines at :1071 (`be48e02`). Each line below,
# after this comment's `#   `, is what it must print byte for byte -- the
# spaces too -- except the seed and the stamp, which are the run's own:
#
#   candidate  8758622  2026-10-14
#   reference  bacead1  2026-10-14
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 32+0.32  hash 64  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=10 alpha=0.05 beta=0.05
#   cap        5250 rounds, 10500 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s283_<stamp>
#
#   id name    cand-8758622  Chesso 8758622 native
#   id name    ref-bacead1  Chesso bacead1 native
#
# with no `diff`, `relay` or `A/A CALIBRATION` line anywhere above them, and
# fastchess's first `Started game 1 of N` line reads N = 10500: the banner
# says what was asked for, that line what fastchess was handed. Sources: the
# two dates are each sha's committer date, `git show -s --date=short
# --format=%cd` (:942-945), read 2026-10-15; `config` comes from main's
# `build/CMakeCache.txt` (:655-663), and `padding` and the `native` token from
# each side's own build; the six lines S099's estimate printed for this pair
# -- candidate, reference, config, padding and both id names
# (`adocs/data/S099_ltc.log`) -- read the same, byte for byte.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171),
# re-read against plan.md's Open list at `be48e02` (2026-10-15): **none**.
# The list is S283, S039 (DEC-290) and strength steps after them. Closed since
# this was first written on `4e6ca63`: S282 (the node band's middle half,
# `2bdc7cc`), the one open filler then, and S285 (the extra-ply mate row
# re-mined on the reverted tree, `be48e02`). S023 verdict 1 read H0 and was
# reverted (`0cdd39a`); S284 closed the harness gap (`769ce7a`).
# Filled by the coordinator on the day it pinned, 2026-10-15: S039's
# candidate is committed as `ca321b6` (Bench: 6100949, gated green) with its
# pre-registration pinned, and is the tree this runs from (DEC-290); the pair
# here is fixed, so the tree changes nothing it plays. Between `be48e02` and
# the pin the list gained no finding: DEC-291 to DEC-293 (`2833636`) are
# S039's rulings. Open findings: none.
# The script refuses to start while its own text still holds that bracketed
# sentence's opening marker: filling it in, brackets and all, is what lets it
# run (the self-check below).
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063), as
# DEC-286 states the outcomes.
#
#   H1 accepted  -> at 32+0.32, Hash 64, the pair is accepted as gaining: H1's
#                   10 nElo preferred to H0's 0 at alpha = beta = 0.05.
#                   **S099 relands**, rebased onto the then-tree -- S039's
#                   outcome (DEC-290) -- by its own step, every golden
#                   re-derived by its script; **S110 and S111 return to the
#                   Open list, conditional** as before. The first change
#                   shipped on a longer control against an H0 at the
#                   harness's; DEC-286 is its record. The stopping run's Elo
#                   is upward-biased (DEC-063) and is not the effect size.
#
#   H0           -> not shown to gain at this control either. **Closed:
#                   nothing ships**, and correction history stays absent until
#                   a re-plan proposes it anew. S110 and S111 stay in the
#                   reserve.
#
#   No verdict   -> the 10500-game cap with neither bound: read as H0 is,
#                   closed, nothing ships, with the games and the interval
#                   recorded. No follow-up run and no second pair (DEC-063,
#                   DEC-286).
#
# THE CLOSING COMMIT carries DEC-220's block, in fastchess's own line names --
# `SPRT | cand 8758622 vs ref bacead1, 32+0.32, Hash=64, noob_3moves.epd,
# {0, 10} nElo` and the `Elo`, `LLR`, `Games`, `Wall` and `Log` lines after
# it -- before its `Bench:` line if it has one, with the log trimmed into
# `adocs/data/S283_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
# regenerates plan.md's ledger table from it.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path -- the done marker reads `SPRT-RUN-DONE
# env <out>` under S284's tag -- and this script execs it and adds nothing
# after.

set -uo pipefail

fail()
{
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

# THE BOOKED SETTINGS, AND NOTHING ELSE FROM THE CALLER'S ENVIRONMENT. Every
# variable fastchess.sh reads from the environment at `be48e02`, and what this
# script does with it:
#
#   pinned: refused if set to anything but the booked value, and passed
#   explicitly on the exec line
#     REF :169, CAND :178, TC :206, HASH :219, CONCURRENCY :287,
#     SPRT_BOUNDS and SPRT_ROUNDS :386-420
#   booked unset: refused if set at all, even empty, and unset on the exec
#   line
#     ROUNDS :392, :443 (removes the SPRT; S284 refuses it beside the two
#     SPRT names anyway), BARE_FEN :249 (the relay regime, S272), SRAND :560
#     (the opening sequence), AA :527 (permits an A/A)
#   unset on the exec line, not refused: RELAY_PYTHON :254, read only under
#     BARE_FEN
#   overridden on the exec line: OUT :576, which names a directory and changes
#     no game
#   not touched: HOME :254 (RELAY_PYTHON's default), TMPDIR :478, :842, :869
#     (where the scratch index, the candidate's snapshot and the relay's copy
#     go), and PATH, which picks the fastchess, git, cmake and compiler
#     binaries -- the fastchess version is DEC-143's A/A question on the day
#
# A leaked `REF=HEAD` used to pass: this read `${REF:-bacead1}`, and the src
# guard below compares REF with CAND's parent, which a wrong REF can satisfy.
ref="bacead1"
cand="8758622"
tc="32+0.32"
hash="64"
concurrency="12"
sprt_bounds="elo0=0 elo1=10 alpha=0.05 beta=0.05"
sprt_rounds="5250"

for pair in "REF=$ref" "CAND=$cand" "TC=$tc" "HASH=$hash" \
            "CONCURRENCY=$concurrency" "SPRT_BOUNDS=$sprt_bounds" \
            "SPRT_ROUNDS=$sprt_rounds"; do
  name="${pair%%=*}"
  want="${pair#*=}"
  if [[ -n "${!name+set}" && "${!name}" != "$want" ]]; then
    fail "$name is '${!name}' in the environment, and this run is booked at" \
         "$name=$want; unset it"
  fi
done
for name in ROUNDS BARE_FEN SRAND AA; do
  if [[ -n "${!name+set}" ]]; then
    fail "$name is set in the environment ('${!name}'), and this run is" \
         "booked without it; unset it"
  fi
done

# THE COORDINATOR'S SENTENCE. The header's open-findings paragraph carries a
# bracketed sentence the coordinator fills on the day it pins; while its
# opening marker is still in this file, the pre-registration is unfinished
# and the run does not start. The marker is assembled in two halves so this
# line does not match itself. Read before the cd, by the path bash ran.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled sentence ($marker" \
          "...]); fill it before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's sentence" ;;
esac

cd /home/max/ws/chesso || fail "cd"

# THE PAIR -- two committed shas, so DEC-220's block names two plain shas that
# `tools/gate.sh` checks against the log and no `candidate.diff` blob is needed
# (DEC-254). The banner prints both with their commit dates, and each side's
# padding, before the first game.
if ! git diff --quiet "$ref" "$cand^" -- src/; then
  fail "CAND's parent differs from REF in src/ -- REF is the engine the" \
       "candidate was built on, or the pair measures two changes"
fi

[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh"

# THE HARNESS, in the header: fastchess.sh must read both names (S284 made it,
# `769ce7a`), or it ignores them and plays its own `{0, 5}` to 20000 rounds.
for name in SPRT_BOUNDS SPRT_ROUNDS; do
  if ! grep -qE '\$\{?'"$name"'([^A-Za-z0-9_]|$)' ./fastchess.sh; then
    fail "./fastchess.sh does not read $name, so this run's bounds or cap" \
         "would be ignored -- see THE HARNESS in adocs/data/S283_sprt.sh"
  fi
done

shopt -s execfail
exec env -u ROUNDS -u BARE_FEN -u SRAND -u AA -u RELAY_PYTHON \
     REF="$ref" \
     CAND="$cand" \
     TC="$tc" \
     HASH="$hash" \
     CONCURRENCY="$concurrency" \
     SPRT_BOUNDS="$sprt_bounds" \
     SPRT_ROUNDS="$sprt_rounds" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s283_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
