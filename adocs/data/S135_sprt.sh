#!/usr/bin/env bash
#
# S135: THE PIECE PLACEMENT FAMILY -- the screen's survivors fitted together
# with every other constant frozen, against the tree that holds them at zero,
# one SPRT at the harness's 8+0.08, Hash 16, {0, 5} nElo (DEC-259 (b),
# DEC-091), bisected on H0 or no verdict along the partition below (DEC-259
# (c), DEC-082).
#
#   nohup adocs/data/S135_sprt.sh > .tuning/s135_sprt.log 2>&1 &
#
# Written 2026-10-22 in S135's phase 1, blind (DEC-260: S083's corpus stages
# held the machine), in adocs/data/S110_sprt.sh's and S083_sprt.sh's shape,
# whose harness section -- and S283_sprt.sh's before them -- this file relies
# on and does not repeat. Nothing here was run: every figure below is read
# from a file or a git object and names it. The pair does not exist yet; the
# coordinator pins it in the bracketed sentence below, and until then the
# script refuses to start (the self-check at the bottom).
#
# Repointed 2026-10-25 at S135's rebase onto b7be983, S083's constants
# landed (01222f3, DEC-313): the corpus is S083's candidate's, as below; the
# bounds, control, cap, costs, abort rule and bisection do not depend on the
# tree and are as written.
#
# THE PAIR.
#
#   CAND  S135's candidate commit: `adocs/data/S135_screen.sh apply` pasted
#         the candidate fit -- the survivors of the screen fitted together,
#         `--only piece_placement` with the dropped terms in `--freeze`, on
#         S083's candidate corpus as `locate` recorded it (curve 0's top rung,
#         every game of S082's training side at 46 rows a game, 6226944 rows,
#         sha256 2e0e3de9..., S135_screen.sh's CORPUS_SHA), against S082's
#         fixed validation set -- over
#         src/evaluation.cpp, every golden the paste moved re-derived by its
#         own script (DEC-142), so the commit is green.
#   REF   CAND's parent: S135's zero-weight landing (the seven columns, the
#         terms, the tests; bench and `tools/search_bench.py` the parent's,
#         INV-6), or that landing rebased, whichever HEAD is when the paste
#         is made.
#
# The two differ by the placement weights and their goldens alone, which the
# guards below check: CAND^ is REF, src/eval_tables.hpp is the same at both,
# and src/evaluation.cpp is the only file under src/ that differs.
#
# WHAT THE VERDICT PRICES. Every surviving term's weights, fitted together on
# the corpus S083's shipped constants were fitted on (S083 read H1, DEC-313),
# and their speed cost: at fitted, non-zero weights the compiler
# keeps the term, so every game pays for it (DEC-047, DEC-091). The cost is
# also timed apart, adocs/data/S135_cost.sh, and its reading printed beside
# this verdict; it decides nothing. A losing term can ride inside a winning
# family; DEC-259 accepts that for power per machine-hour.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's
# control for every verdict but the correction family's, and this is an
# evaluation family. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names it
# for a family's survivors, and the step file's own section 6 and DEC-063 ask
# for a pair that straddles the expected effect -- the located parts are
# single digits each (+8.2 for a tapered bishop pair, +9.86 for a rook file
# retune, DEC-084, order of magnitude only), a sum of them is the family's
# size, and nElo here runs about 1.3 times Elo (DEC-259's context, S055), so a
# family of that size sits past elo1 = 5 and the pair resolves; a smaller one
# random-walks to the cap, which is the cost below. Rejected: `{-5, 5}` --
# would ship a zero half the time (DEC-304's reason); `{0, 10}` -- a true gain
# under about 5 nElo read as H0 more often, and the bisection then spends its
# parts' verdicts on terms the family's verdict should have kept; `--fast` --
# S027's `{0, 10}` at fast alpha and beta is the failure this step exists not
# to repeat (S100).
#
# WHAT THE PAIR COSTS (DEC-143): **41861 games** with the truth at the
# interval's midpoint, 19.84 h at the 2110 games an hour `.moltke.local.md`
# budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** with it
# on a bound, 12.13 h (plan.md's "What the formula says before a run"). The
# step file's "1.5 to 3 hours" is the run length for an effect well outside
# the interval, S105's 23.1 to 38.7 games a minute; it is not the worst case.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S283's, S110's and S083's rule, in
# place of fastchess.sh's 20000-round default, which a truth at the midpoint
# meets before it meets a bound. Four hours or more, so the run starts by
# DEC-155's night rule. Watcher ceiling 144000 s (40 h), twice the cap's hours
# and the abort below, wall-clock only while the machine is awake.
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
#   out        /home/max/ws/chesso/.tuning/sprt_s135_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# [COORDINATOR, on the day it pins: CAND's and REF's shas into `ref=` and
# `cand=` below, their dates into the banner lines above, the screen's
# survivors and its reading.txt's sha256, the corpus `locate` recorded (its
# sha256 CORPUS_SHA's), the candidate fit's sha256, S135_cost.sh's reading if it has run,
# plan.md's Open list re-read, the A/A rule checked, and this bracketed
# sentence replaced by what was found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the family's fitted weights are accepted as
#                   gaining: H1's 5 nElo preferred to H0's 0 at alpha = beta =
#                   0.05. **They ship**, all survivors together; the dropped
#                   terms stay at zero with their screen figures recorded. The
#                   stopping run's Elo is upward-biased (DEC-063) and is not
#                   the effect size. No bisection follows an H1.
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the
#                   weights and the goldens it moved -- and the family is
#                   **bisected, never zeroed by hand** (DEC-091, DEC-082): the
#                   S027 failure was a hand revert that left four features
#                   unmeasured.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   read as H0 is -- reverted whole and bisected (DEC-259 (c):
#                   "On H0 or no verdict the family is bisected").
#
# THE BISECTION, written now and the same as S135_screen.sh's header. The
# family splits into A, S027's three (bishop_pair, rook_open,
# rook_half_open), and B, DEC-258's two (bad_bishop, trapped); A into A1
# (bishop_pair) and A2 (rook_open, rook_half_open), A2 into its two; B into B1
# (bad_bishop) and B2 (trapped). Applied to the survivors: an empty half is
# dropped, a half holding all of its part's survivors is skipped to its own
# halves, a single term is a leaf; the screen's reading.txt prints the order.
# Each part: refitted alone (`S135_screen.sh part NAME`, every other placement
# weight at what ships then), pasted by `S135_screen.sh apply`, and measured
# by its own SPRT in this file's shape -- a copy named S135_sprt_<NAME>.sh,
# the same bounds, control, cap and abort, its pair pinned the same way,
# against the tree as it stands then. Order: A, then A's halves if A fails,
# then B, then B's halves if B fails. A part's H1 lands and stays, and the
# parts after it are fitted and measured on top of it; its H0 or no verdict
# reverts it and splits it; a leaf's H0 or no verdict is that term's zero,
# recorded as zero with its screen figures. The worst case is every part
# measured: with all five surviving, eight more SPRTs at most (A and B; A1 and
# A2 if A fails; A2a and A2b if A2 fails; B1 and B2 if B fails), each up to
# 19.9 h, about 159 h in all -- a budget the coordinator states, part by part,
# before each one starts.
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S135_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
# regenerates plan.md's ledger table from it. A part's commit carries its own
# block and log, `adocs/data/S135_sprt_<NAME>.log`.
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

# THE PAIR: REF is CAND's parent, so the run measures S135's paste and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S135"
fi

# The paste is an `--only piece_placement` fit: the tables cannot have moved,
# and under src/ only src/evaluation.cpp, where the placement weights live,
# may differ.
if ! git diff --quiet "$ref" "$cand" -- src/eval_tables.hpp; then
  fail "src/eval_tables.hpp differs between REF and CAND -- not a placement-only paste"
fi
if git diff --quiet "$ref" "$cand" -- src/evaluation.cpp; then
  fail "src/evaluation.cpp is the same at CAND and REF -- not S135's paste"
fi
if [[ "$(git diff --name-only "$ref" "$cand" -- src)" != "src/evaluation.cpp" ]]; then
  fail "CAND changes more under src/ than src/evaluation.cpp"
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s135_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
