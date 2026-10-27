#!/usr/bin/env bash
#
# S136: TEMPO -- the side-to-move bonus fitted alone with every other constant
# frozen, `--only tempo --lambda 0` on S083's corpus, kept by DEC-315's screen,
# against the tree that ships it at zero: one SPRT at the harness's 8+0.08,
# Hash 16, {0, 5} nElo (DEC-259 (b), DEC-092). A family of one: no bisection.
#
#   nohup adocs/data/S136_sprt.sh > .tuning/s136_sprt.log 2>&1 &
#
# Written 2026-10-27 in S136's phase 1, at 80520d7, in adocs/data/S135_sprt.sh's
# and S296_sprt.sh's shape, whose harness section -- S283_sprt.sh's before
# them -- this file relies on and does not repeat. Nothing here was run: every
# figure below is read from a file or a git object and names it. The pair does
# not exist yet; the coordinator pins it in the bracketed lines below, and
# until then the script refuses to start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  S136's candidate commit: `adocs/data/S136_screen.sh apply` pasted
#         the screen's full-corpus fit, `.tuning/s136/screen/full_tempo.hpp`
#         -- `--only tempo --lambda 0` on S083's candidate corpus as `locate`
#         recorded it (curve 0's top rung, 6226944 rows, sha256 2e0e3de9...,
#         S136_screen.sh's CORPUS_SHA), against S082's fixed validation set --
#         over src/evaluation.cpp `tempo_mg` and `tempo_eg`; the truncation
#         guard re-targeted by the step file's guard plan (tests/
#         test_eval_trace.cpp, its red observed first); every golden the paste
#         moved re-derived by its own script (DEC-142); so the commit is green.
#   REF   CAND's parent: the tree that ships tempo at zero -- `80520d7`'s
#         engine, S136's phase 1 having changed nothing under src/, at the
#         phase 1 landing or a documents-only descendant of it.
#
# The two differ by the two tempo weights and what they move, which the
# guards below check: CAND^ is REF, REF's src/ is byte for byte `80520d7`'s,
# src/eval_tables.hpp is the same at both, src/evaluation.cpp is the only
# file under src/ that differs, and outside its comments the only lines that
# differ there are the two `constexpr int tempo_mg` / `tempo_eg` definitions.
#
# WHAT THE VERDICT PRICES. The fitted pair and its cost together: at zero the
# compiler deletes the term, at a non-zero pair every evaluation pays one
# taper -- seven instructions, `evaluate_cheap()` 164 at zero against 171 at
# forced weights, as src/evaluation.cpp `evaluate_cheap`'s comment records
# S027's disassembly (2026-08-13, on the MacBook's compiler; not re-read on
# g++ 13.3) -- and that cost is inside the verdict. **No speed-cost run is owed**: DEC-047's reason for
# timing a term apart is a count the compiler drops at zero weight and keeps
# at any other, so a zero-weight landing measures nothing; tempo is a
# constant, not a count, there is no feature to compute and nothing to time
# beyond the taper the verdict carries.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's
# control for every verdict but the correction family's, and this is an
# evaluation term. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names it
# for a family's survivors, tempo being one of its families of one; the step's
# accepts asks for "bounds that resolve single digits", read as this pair (the
# step file's 2026-10-27 section, the coordinator's ruling). What is known of
# the size, deciding nothing (DEC-019, DEC-084): S027's run reached neither
# bound at `{0, 10}` `--fast` over 3000 games, -0.69 +/- 9.64 Elo; S100 put the
# label-side signal near 3.6 cp of tempo weight on the old corpus and S135's
# audit near -8.7 cp on S083's, the opposite sign, both confounded; the one
# traced published figure prices adding the term from nothing at another
# engine's band (+12.99 +/- 7.35 at 10+0.1, +6.67 +/- 4.67 at 40+0.4). A true
# effect near 2.5 nElo, the interval's midpoint, random-walks to the cap,
# which is the cost below. Rejected: `{-5, 5}` -- would ship a zero half the time
# (DEC-304's reason); `{0, 10}` and `--fast` -- S027's pair, the run that
# resolved nothing (DEC-063), the failure this step exists not to repeat;
# `{-5, 0}` -- a non-regression pair, for a change wanted for a reason other
# than strength, and tempo has no such reason.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic
# (`adocs/testing_strategy.md` section 1.1: D / (e1 - e0)^2 games at the
# midpoint, D = 1046535 at alpha = beta = 0.05; about 639770 / (e1 - e0)^2 on
# a bound): **41861 games** with the truth at the interval's midpoint, 19.84 h
# at the 2110 games an hour `.moltke.local.md` budgets 8+0.08 on
# `noob_3moves.epd` from (DEC-190); **25591 games** with it on a bound,
# 12.13 h. S135's run of the same pair and control played 13406 games in
# 6 h 17 m, 2133 an hour, to H1.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S283's, S135's and S296's rule, in
# place of fastchess.sh's 20000-round default. Four hours or more, so the run
# starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h), twice the
# cap's hours and the abort below, wall-clock only while the machine is awake.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), fastchess.sh's adjudication, the
# governor recorded and not set (DEC-195). If fastchess, the book, the
# adjudication or the machine has moved since the last fixed-rounds A/A, that
# A/A of 1000 games comes first (DEC-143); the coordinator checks on the day.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's PGN in
# `$OUT`), on a crash or a disconnect on either side (`SPRT-RUN-INVALID`,
# S212), if the machine loses mains power (POWER) or another load starts on it
# (MACHINE), if the banner is not what the coordinator's sentence below says
# it must print, or at 40 h of wall clock with neither bound. Nothing else
# stops it.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S283's lines with this
# run's shas, dates and out directory --
#
#   candidate  <CAND, short>  <its commit date>
#   reference  <REF, short>   <its commit date>
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s136_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# [COORDINATOR, on the day it pins: CAND's and REF's shas into `ref=` and
# `cand=` below, their dates into the banner lines above, REF's src/ re-read
# against 80520d7's (the guard below names it), the screen's reading.txt and
# its sha256 (g, s, m, the fitted pair), the corpus `locate` recorded (its
# sha256 CORPUS_SHA's), the candidate fit's sha256, the guard's re-targeting
# as landed (its red observed first, the pins re-drawn, the thresholds),
# plan.md's Open list re-read, the A/A rule checked, and this bracketed
# sentence replaced by what was found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted pair is accepted as gaining: H1's 5
#                   nElo preferred to H0's 0 at alpha = beta = 0.05. **It
#                   ships**, with the guard as re-derived; the closing commit
#                   makes the tempo comments true (src/evaluation.cpp,
#                   src/evaluation.hpp, tools/tuner_trace.hpp,
#                   tools/truncation_scan.cpp, DEV_MANUAL.md) and specs.md's
#                   "absent, evaluation" row loses tempo. The stopping run's
#                   Elo is upward-biased (DEC-063) and is not the effect size.
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the
#                   weights, the goldens it moved and the guard's
#                   re-targeting, back to tempo-zero's numbers -- and tempo is
#                   **recorded as zero** with its screen and match figures.
#                   No bisection: a family of one has no parts.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   reverted whole as H0 is, and recorded as **unresolved**,
#                   the second time after S027's -- not converted to zero
#                   (S136's accepts, DEC-092). The term stays at zero.
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S136_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
# regenerates plan.md's ledger table from it.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, and this script execs it and adds
# nothing after.

set -uo pipefail

fail()
{
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

# THE BOOKED SETTINGS, AND NOTHING ELSE FROM THE CALLER'S ENVIRONMENT, as
# S296_sprt.sh's same block reads them: pinned, refused if set to anything
# else, and passed on the exec line; ROUNDS, BARE_FEN, SRAND and AA refused if
# set at all; RELAY_PYTHON unset; OUT overridden.
ref="[COORDINATOR: REF's sha]"
cand="[COORDINATOR: CAND's sha]"
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"
sprt_rounds="20950"

# THE COORDINATOR'S SENTENCE AND SHAS. While the bracketed marker is anywhere
# in this file the pre-registration is unfinished and the run does not start.
# The marker is assembled in two halves so this line does not match itself.
# Checked before the environment, because an unpinned sha would read there as
# a mismatch and name the wrong fault.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); pin the pair and fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

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

cd /home/max/ws/chesso || fail "cd"

# THE PAIR: REF is CAND's parent, so the run measures S136's paste and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S136"
fi

# REF's engine is the one that ships tempo at zero: S136's phase 1 changed
# nothing under src/, so a documents-only descendant of 80520d7 passes and a
# tree with any other src/ change does not.
if ! git diff --quiet 80520d7 "$ref" -- src/; then
  fail "REF's src/ differs from 80520d7's -- REF is not the shipped engine or" \
       "a documents-only descendant of it"
fi

# The paste is an `--only tempo` fit: the tables cannot have moved, under
# src/ only src/evaluation.cpp may differ, and there only the two tempo
# definitions outside comments.
if ! git diff --quiet "$ref" "$cand" -- src/eval_tables.hpp; then
  fail "src/eval_tables.hpp differs between REF and CAND -- not a tempo-only paste"
fi
if git diff --quiet "$ref" "$cand" -- src/evaluation.cpp; then
  fail "src/evaluation.cpp is the same at CAND and REF -- not S136's paste"
fi
if [[ "$(git diff --name-only "$ref" "$cand" -- src)" != "src/evaluation.cpp" ]]; then
  fail "CAND changes more under src/ than src/evaluation.cpp"
fi
code_lines="$(git diff -U0 "$ref" "$cand" -- src/evaluation.cpp |
              grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' | cut -c2- |
              grep -vE '^[[:space:]]*(//.*)?$')"
if [[ -z "$code_lines" ]] ||
   grep -qvE '^constexpr int tempo_(mg|eg) = -?[0-9]+;$' <<< "$code_lines"; then
  fail "src/evaluation.cpp differs outside comments in more than the two" \
       "tempo definitions -- not a tempo-only paste"
fi

[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh"

# THE HARNESS: fastchess.sh must read both names, or it ignores them and plays
# its own `{0, 5}` to 20000 rounds.
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s136_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
