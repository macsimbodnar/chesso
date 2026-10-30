#!/usr/bin/env bash
#
# S123: THE PASSED PAWN FAMILY -- the survivors of S123's screen (A, the free
# and safe increments; B, both kings' distance slopes; C, candidate passers),
# fitted on S083's corpus with every old constant frozen, against the phase 1
# landing that ships the family at zero: one SPRT at the harness's 8+0.08,
# Hash 16, {0, 5} nElo (DEC-259 (b)), bisected along DEC-329 (5)'s partition
# on H0 or no verdict.
#
#   nohup adocs/data/S123_sprt.sh > .tuning/s123_sprt.log 2>&1 &
#
# Written 2026-10-30 in S123's phase 1, before any of it ran, in
# adocs/data/S121_sprt.sh's shape, whose harness section -- S283_sprt.sh's
# before it -- this file relies on and does not repeat. Nothing here was run.
# The pair does not exist yet; the coordinator pins it in the bracketed lines
# below, and until then the script refuses to start (the self-check at the
# bottom).
#
# THE PAIR.
#
#   CAND  S123's candidate commit: `adocs/data/S123_screen.sh apply` pasted the
#         screen's candidate -- the last survivor's full-corpus fit, `--only`
#         the survivors' groups with their ties on S083's corpus (6226944
#         rows, sha256 2e0e3de9..., S123_screen.sh's CORPUS_SHA) against
#         S082's fixed validation set -- over the family's arrays in
#         src/evaluation.cpp; every golden the paste moved re-derived by its
#         own script (DEC-142); so the commit is green.
#   REF   CAND's parent: S123's phase 1 landing, the family at zero and in no
#         expression (adocs/data/S123_inv6.txt), or a documents-only
#         descendant of it.
#
# The two differ by the fitted family arrays and nothing else in src/, which
# the guards below check: CAND^ is REF; REF's src/ is the phase 1 landing's
# byte for byte; and CAND's src/ is REF's but for src/evaluation.cpp, which
# differs outside whole-line comments only in lines of the ten family
# definitions' shape (passed_free, passed_safe, passed_king_own,
# passed_king_enemy and candidate_passer, mg and eg), and in which every other
# weight array and define reads REF's value by name. A dropped group's arrays stay zero and are in
# no expression at CAND either.
#
# WHAT THE VERDICT PRICES. The fitted survivors and their cost together:
# every evaluation pays the groups whose weights are live -- A's is_attacked()
# per free passer, B's distances, C's sets -- at every node, the shortcut's
# nodes included, since the family is stage one (DEC-329 (1)). The cost is
# recorded apart, on identical trees, by adocs/data/S123_cost.sh (DEC-047's
# method), and printed beside the verdict; it decides nothing.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's
# control for every verdict but the correction family's, and this is an
# evaluation family. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names it
# for a family's survivors, and every bisection part so far used it. What is
# known of the size, deciding nothing (DEC-019, the step file's section 1):
# Stash v32's king proximity in the passed-pawn evaluation +22.27 +/- 9.86 at
# 8+0.08; Ethereal's proximity to both kings +6.13 +/- 4.38 at 10+0.1;
# candidate passers +5.03 (Ethereal) and +6.87 (Stash v30). No located source
# prices a rank x can-advance x safe-advance table. Rejected: `{-5, 5}` --
# would ship a zero half the time (DEC-304's reason); `--fast` and `{0, 10}`
# -- DEC-063; a separate `{0, 3}` for C's "+5 class" -- DEC-259 measures the
# family once and C alone only on H0.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic
# (`adocs/testing_strategy.md` section 1.1): **41861 games** with the truth at
# the interval's midpoint, 19.84 h at the 2110 games an hour `.moltke.local.md`
# budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** on a
# bound, 12.13 h. S121's run of the same pair and control played 3160 games in
# 1 h 30 m to H1, S135's 13406 in 6 h 17 m.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour, S283's,
# S135's, S136's and S121's rule. Four hours or more, so the run starts by
# DEC-155's night rule. Watcher ceiling 144000 s (40 h), wall-clock only while
# the machine is awake.
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
#   out        /home/max/ws/chesso/.tuning/sprt_s123_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# [COORDINATOR, on the day it pins: CAND's and REF's shas into `ref=` and
# `cand=` below, the phase 1 landing's sha into `landing=`, their dates into
# the banner lines above, the screen's three readings and their sha256 (each
# part's gain, s, m and verdict), the survivors and the H0 order the screen
# printed, the corpus `locate` recorded, the candidate fit's sha256 and its
# tie files', the cost run's reading, plan.md's Open list re-read, the A/A
# rule checked, and this bracketed sentence replaced by what was found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted family is accepted as gaining: H1's
#                   5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It
#                   ships.** The closing commit makes the comments over the
#                   family's weights and evaluation.hpp's definitions true,
#                   and specs.md's evaluation rows; a dropped group stays at
#                   zero, in no expression, recorded as a zero (S126's rule).
#                   The stopping run's Elo is upward-biased (DEC-063) and is
#                   not the effect size.
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the
#                   family back to zero, the goldens back -- and the family is
#                   bisected along DEC-329 (5)'s partition, survivors only, in
#                   order: A the free and safe increments, B both kings'
#                   distance slopes, C candidate passers, each its own `{0, 5}`
#                   SPRT in this file's shape -- at worst three more runs after
#                   this one, four in all, each up to 19.86 h, 79.4 h with this
#                   run. Each plays over what has landed by then, its group
#                   refitted alone then (`adocs/data/S123_screen.sh part
#                   NAME`); a part's H1 lands before the next is fitted; a
#                   part's H0 or no verdict is that part's zero.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   reverted whole as H0 is, recorded as **unresolved**, and
#                   bisected as H0 is (DEC-259 (c)).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S123_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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
# S136_sprt.sh's same block reads them.
ref="[COORDINATOR: REF's sha]"
cand="[COORDINATOR: CAND's sha]"
landing="[COORDINATOR: the phase 1 landing's sha]"
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"
sprt_rounds="20950"

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

# THE PAIR: REF is CAND's parent, so the run measures S123's paste and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S123"
fi

# REF's engine is the phase 1 landing's: the family at zero.
if ! git diff --quiet "$landing" "$ref" -- src/; then
  fail "REF's src/ differs from the phase 1 landing's -- REF is not the" \
       "family at zero or a documents-only descendant of it"
fi

# CAND's src/ is REF's but for src/evaluation.cpp.
changed="$(git diff --name-only "$ref" "$cand" -- src)"
if [[ -z "$changed" ]]; then
  fail "CAND's src/ is REF's -- not S123's paste"
fi
while IFS= read -r file; do
  [[ "$file" == src/evaluation.cpp ]] ||
    fail "CAND changes $file beyond the paste of the family's weights"
done <<< "$changed"

# And there only the family's ten definitions, outside whole-line comments.
code_lines="$(git diff -U0 "$ref" "$cand" -- src |
              grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' | cut -c2- |
              grep -vE '^[[:space:]]*(//.*)?$')"
table='^constexpr int (passed_free|passed_safe|passed_king_own|passed_king_enemy|candidate_passer)_(mg|eg)\[[0-9]+\] = \{'
values='^[[:space:]]*-?[0-9]+(,[[:space:]]*-?[0-9]+)*,?[[:space:]]*(\};)?$'
if grep -qvE "($table)|($values)" <<< "$code_lines"; then
  fail "CAND's src/ differs outside comments in more than the family's" \
       "weight definitions"
fi

# The line test accepts a numbers-only line anywhere in the file -- a
# continuation line of a mobility table among them -- so every weight array
# and define is also compared by name: each one outside the family's ten is
# REF's at CAND (S123_screen.py only-family-pair, `apply`'s only-family
# between two commits).
python3 adocs/data/S123_screen.py only-family-pair "$ref" "$cand" --tree . ||
  fail "CAND moves a weight outside the family's ten arrays against REF"

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
     OUT="/home/max/ws/chesso/.tuning/sprt_s123_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
