#!/usr/bin/env bash
#
# S083: THE CORPUS THAT SHIPS -- the evaluation constants fitted on the corpus
# S083's curve picks, against the constants that ship, one SPRT at the
# harness's 8+0.08, Hash 16, {0, 5} nElo: the only corpus SPRT S082 and S083
# owe (DEC-259).
#
#   nohup adocs/data/S083_sprt.sh > .tuning/s083_sprt.log 2>&1 &
#
# Written 2026-10-22 in S083's phase 1, blind (DEC-260: S111's SPRT held the
# machine), in adocs/data/S110_sprt.sh's shape, whose harness section -- and
# S283_sprt.sh's before it -- this file relies on and does not repeat. Nothing
# here was run: every figure below is read from a file or a git object and
# names it. The pair does not exist yet; the coordinator pins it in the
# bracketed sentence below, and until then the script refuses to start (the
# self-check at the bottom).
#
# THE PAIR.
#
#   CAND  S083's landing commit: `adocs/data/S083_corpus.sh apply` pasted the
#         candidate's fit -- the top rung of the last stage S083's reading
#         stopped at, every game of that stage's pool at 46 rows a game, leaf
#         qs (S082's outcome (a)), fitted with S082's compare settings on the
#         fixed validation set -- over src/eval_tables.hpp and
#         src/evaluation.cpp, and every golden the paste moved was re-derived by
#         its own script (DEC-142), so the commit is green.
#   REF   CAND's parent, the tree whose constants ship: S076's fit on S065's
#         deduplicated root corpus (`77d7450`), re-expressed bit-exactly by
#         S134's fold (DEC-090) and fitted by nothing since.
#
# The two differ by the paste and its goldens alone, which the guard below
# checks (CAND^ is REF).
#
# WHAT THE VERDICT PRICES, AND WHAT IT CANNOT ATTRIBUTE, stated before a game.
# The candidate differs from REF's constants in the corpus's leaf, its density,
# its size and its engine, and is a fresh 5000-epoch fit with its own K: the
# package DEC-259 names, "the corpus that ships", and nothing inside it is
# priced apart. Two things travel with it. The scale: the fit holds the
# material-and-table gauges, not the evaluation's overall level, and S082's
# 1 M-row qs fit already moved the defines (PAWN 94 -> 86, KNIGHT 327 -> 374,
# BISHOP 308 -> 351, ROOK 487 -> 512, QUEEN 716 -> 721,
# `.tuning/s082/fit_qs_46_s1_qs.hpp`), while the search's margins in
# evaluation units were fitted on the old scale (S261's lane); an H0 may be
# that and not the corpus, which S127's lane exists to re-fit. And the
# refit: S075's lambda-0 control priced 5000 more epochs on one corpus at
# 3e-06 of held-out error, so the epochs are not the reason, whatever this
# reads.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 kept the harness's control
# for every verdict but the correction family's (DEC-297), and this is an
# evaluation refit; DEC-202's longer-control reading leaves evaluation weights
# out by name (fastchess.sh's own comment). `{0, 5}` nElo at alpha = beta =
# 0.05: plan.md's "How a change is measured" -- an evaluation family's
# survivors take one `{0, 5}` SPRT (DEC-259 (b)) -- and the question the step
# asks, whether the new corpus's constants gain over the ones that ship.
# Rejected: `{-5, 5}`, S076's form for a refit of unknown sign, 10465 games
# worst case -- it would ship a zero half the time, and every later evaluation
# family fits on what this verdict ships; `{0, 10}` -- a resolution no
# measurement here asks for, and a true gain under about 5 nElo read as H0
# more often.
#
# WHAT THE PAIR COSTS (DEC-143): **41861 games** with the truth at the
# interval's midpoint, 19.84 h at the 2110 games an hour `.moltke.local.md`
# budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** with it
# on a bound, 12.13 h (plan.md's "What the formula says before a run").
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S283's and S110's rule, in place of
# fastchess.sh's 20000-round default, which a truth at the midpoint meets
# before it meets a bound. Four hours or more, so the run starts by DEC-155's
# night rule. Watcher ceiling 144000 s (40 h), twice the cap's hours and the
# abort below, wall-clock only while the machine is awake.
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
#   candidate  <CAND>  <its committer date>
#   reference  <REF>  <its committer date>
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s083_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# [COORDINATOR, on the day it pins: CAND's and REF's shas into `ref=` and
# `cand=` below, their dates into the banner lines above, the stage and rung
# the candidate came from and its fit's sha256, plan.md's Open list re-read,
# the A/A rule checked, and this bracketed sentence replaced by what was
# found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the new corpus's constants are accepted as
#                   gaining: H1's 5 nElo preferred to H0's 0 at alpha = beta =
#                   0.05. **They ship**, and the corpus becomes the one every
#                   later evaluation fit and screen trains on (S135 on, S126).
#                   The stopping run's Elo is upward-biased (DEC-063) and is
#                   not the effect size.
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the
#                   constants and the goldens it moved -- and recorded as
#                   zero; REF's constants stay. The corpus is kept on disk and
#                   archived (DEC-235). Which corpus the later families screen
#                   on is the coordinator's decision, recorded at S135's start
#                   with this verdict beside it: this run prices the refit of
#                   all 823 constants, not the corpus as a training set for one
#                   term with every other frozen.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   read as H0 is -- reverted whole, recorded as zero, with
#                   the games and the interval -- and no follow-up run
#                   (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S083_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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
# S283_sprt.sh's same block reads them: pinned, refused if set to anything
# else, and passed on the exec line; ROUNDS, BARE_FEN, SRAND and AA refused if
# set at all; RELAY_PYTHON unset; OUT overridden.
ref="[COORDINATOR: REF's sha]"
cand="[COORDINATOR: CAND's sha]"
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"
sprt_rounds="20950"

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

# THE COORDINATOR'S SENTENCE AND SHAS. While the bracketed marker is anywhere
# in this file the pre-registration is unfinished and the run does not start.
# The marker is assembled in two halves so this line does not match itself.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); pin the pair and fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

cd /home/max/ws/chesso || fail "cd"

# THE PAIR: REF is CAND's parent, so the run measures S083's paste and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S083"
fi

# The paste changed the evaluation's constants: CAND must touch both files the
# tuner's header names, or the pair is not the one this file books.
for file in src/eval_tables.hpp src/evaluation.cpp; do
  if git diff --quiet "$ref" "$cand" -- "$file"; then
    fail "$file is the same at CAND and REF -- not S083's paste"
  fi
done

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
     OUT="/home/max/ws/chesso/.tuning/sprt_s083_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
