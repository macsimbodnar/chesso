#!/usr/bin/env bash
#
# S125: THE PAWN STRUCTURE FAMILY -- the survivors of S125's screen (P1, the
# isolated and backward increment; P2, isolated pawns by file; P3, doubled
# pawns by file; P4, backward pawns not isolated by rank; P5, phalanx pawns;
# P6, supported pawns; P7, weak unopposed pawns) with P0's three flats refitted
# beside them, fitted on S083's corpus with every other constant frozen,
# against the phase 1 landing that ships the family at zero: one SPRT at the
# harness's 8+0.08, Hash 16, {0, 5} nElo (DEC-259 (b)), bisected along DEC-331
# (7)'s partition on H0 or no verdict.
#
#   nohup adocs/data/S125_sprt.sh > .tuning/s125_sprt.log 2>&1 &
#
# Written 2026-10-30 in S125's phase 1, before any of it ran, in
# adocs/data/S123_sprt.sh's shape with the guards S123's phase 2 fast check
# strengthened (the build files beside src/, a family line read to its end,
# every other weight compared by name), and S121_sprt.sh's harness section --
# S283_sprt.sh's before it -- relied on and not repeated. Nothing here was
# run. The pair does not exist yet; the coordinator pins it in the bracketed
# lines below, and until then the script refuses to start (the self-check at
# the bottom).
#
# THE PAIR.
#
#   CAND  S125's candidate commit: `adocs/data/S125_screen.sh apply` pasted the
#         screen's candidate -- the last survivor's full-corpus fit, `--only`
#         P0's flats and the survivors' groups with their ties on S083's
#         corpus (6226944 rows, sha256 2e0e3de9..., S125_screen.sh's
#         CORPUS_SHA) against S082's fixed validation set -- over the family's
#         arrays and the three flats in src/evaluation.cpp; every golden the
#         paste moved re-derived by its own script (DEC-142); so the commit is
#         green.
#   REF   CAND's parent: S125's phase 1 landing, the family at zero and in no
#         expression (adocs/data/S125_inv6.txt), or a descendant of it whose
#         src/ and build files are the landing's byte for byte.
#
# The two differ by the fitted family and flats and nothing else in src/ or the
# build's flags, which the guards below check:
#   - CAND^ is REF;
#   - REF's src/, its top-level CMakeLists.txt and cmake/ are the phase 1
#     landing's byte for byte (src/CMakeLists.txt is under src/; the engine
#     target reads no other build file);
#   - CAND's are REF's but for src/evaluation.cpp, which differs outside
#     whole-line comments only in whole one-line definitions of the family's
#     sixteen arrays -- isolated_backward, isolated_file, doubled_file,
#     backward_rank, phalanx, supported, weak_unopposed and pawn_structure (P0's
#     flats are the family's too, DEC-331 (5)), mg and eg -- each read to its
#     closing `};` and nothing after it. A definition clang-format wrapped onto
#     a second line is refused, not read: join it before pinning;
#   - and every weight array and define outside those sixteen reads REF's
#     value by name (S125_screen.py only-family-pair). A dropped group's
#     arrays stay zero and are in no expression at CAND either.
#
# WHAT THE VERDICT PRICES. The fitted survivors and their cost together:
# every evaluation pays the groups whose weights are live -- a few set
# operations over the pawn fills, a popcount for each count, a loop over the
# pawns of each table -- at every node, the shortcut's nodes included, since
# the family is stage one (DEC-331 (8)); the flats' refit costs nothing. The
# cost is recorded apart, on identical trees, by adocs/data/S125_cost.sh
# (DEC-047's method), and printed beside the verdict; it decides nothing.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's
# control for every verdict but the correction family's, and this is an
# evaluation family. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names it
# for a family's survivors, and every bisection part so far used it. What is
# known of the size, deciding nothing (DEC-019, the step file's section 1):
# Stash v31's connected pawns, phalanx and defender, +25.38 +/- 10.40 at
# 8+0.08; Ethereal's isolated and stacked pawns by file +6.60 +/- 4.44, the
# double count +4.01 +/- 3.15, backward by rank +3.77 +/- 3.01 (a bundle with a
# pawn table retune), every one of them smaller at 60+0.6. No located source
# prices a weak unopposed pawn. Rejected: `{-5, 5}` -- would ship a zero half
# the time (DEC-304's reason); `--fast` and `{0, 10}` -- DEC-063; one SPRT per
# group -- DEC-259 measures the family once and a part alone only on H0.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic
# (`adocs/testing_strategy.md` section 1.1): **41861 games** with the truth at
# the interval's midpoint, 19.84 h at the 2110 games an hour `.moltke.local.md`
# budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** on a
# bound, 12.13 h. S123's run of the same pair and control played 1166 games in
# 33 m to H1, S121's 3160 in 1 h 30 m, S135's 13406 in 6 h 17 m.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour, S283's,
# S135's, S136's, S121's and S123's rule. Four hours or more, so the run
# starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h), wall-clock
# only while the machine is awake.
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
#   out        /home/max/ws/chesso/.tuning/sprt_s125_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# [COORDINATOR, on the day it pins: CAND's and REF's shas into `ref=` and
# `cand=` below, the phase 1 landing's sha into `landing=`, their dates into
# the banner lines above, the screen's eight readings and their sha256 (P0's g
# and each part's d, s, m and verdict), the survivors and the H0 partition the
# screen printed, the corpus `locate` recorded, the candidate fit's sha256 and
# its tie files', the cost run's reading, plan.md's Open list re-read, the A/A
# rule checked, and this bracketed sentence replaced by what was found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted family is accepted as gaining: H1's
#                   5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It
#                   ships.** The closing commit folds the increment (DEC-331
#                   (3)): backward reads `backward & ~isolated` and IB takes
#                   its own weight, backward's plus the increment, INV-6
#                   against CAND (the scores identical, and no isolated pawn
#                   counted backward, the accepts); with it, the hand cases and
#                   comments that assert the double count are rewritten with
#                   the definition, in the same commit as specs.md's wording
#                   (DEC-331 (13)), and the comments over the weights made
#                   true. A dropped group stays at zero, in no expression,
#                   recorded as a zero (S126's rule). The stopping run's Elo is
#                   upward-biased (DEC-063) and is not the effect size.
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the
#                   family back to zero, the flats back to S083's, the goldens
#                   back -- and the family is bisected along DEC-331 (7)'s
#                   partition as ruled at phase 1's fast check, survivors
#                   only, in order: A = P1 to P4, the reshaped existing terms,
#                   B = P5 and P6, connected pawns, C = P7, weak unopposed
#                   pawns; a part with no survivor does not exist. P0's flats
#                   never land alone: every part that exists frees them with
#                   its groups, as every screen part did, so they enter with
#                   the first and each later part refits them over what has
#                   landed. Each part its own `{0, 5}` SPRT in this file's
#                   shape -- at worst three more runs after this one, four in
#                   all, each up to 19.86 h, 79.4 h with this run. Each plays
#                   over what has landed by then, its groups refitted then
#                   (`adocs/data/S125_screen.sh part NAME`); a part's H1 lands
#                   before the next is fitted; a part's H0 or no verdict is
#                   that part's zero.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   reverted whole as H0 is, recorded as **unresolved**, and
#                   bisected as H0 is (DEC-259 (c)).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S125_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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

# THE PAIR: REF is CAND's parent, so the run measures S125's paste and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S125"
fi

# The files the engine's build reads: src/ (src/CMakeLists.txt with it), the
# top-level CMakeLists.txt and cmake/'s flag files. tests/ and tools/ build no
# part of the chesso target fastchess.sh plays.
build_paths=(src CMakeLists.txt cmake)

# REF's engine is the phase 1 landing's: the family at zero, the flats S083's.
if ! git diff --quiet "$landing" "$ref" -- "${build_paths[@]}"; then
  fail "REF's src/ or build files differ from the phase 1 landing's -- REF is" \
       "not the family at zero or a descendant of it with the landing's src/"
fi

# CAND's src/ and build files are REF's but for src/evaluation.cpp.
changed="$(git diff --name-only "$ref" "$cand" -- "${build_paths[@]}")"
if [[ -z "$changed" ]]; then
  fail "CAND's src/ is REF's -- not S125's paste"
fi
while IFS= read -r file; do
  [[ "$file" == src/evaluation.cpp ]] ||
    fail "CAND changes $file beyond the paste of the family's weights"
done <<< "$changed"

# And there only whole one-line definitions of the family's sixteen arrays,
# outside whole-line comments, each to its closing `};` and nothing after it:
# no continuation line, and no code appended to a definition.
code_lines="$(git diff -U0 "$ref" "$cand" -- src |
              grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' | cut -c2- |
              grep -vE '^[[:space:]]*(//.*)?$')"
table='^constexpr int (isolated_backward|isolated_file|doubled_file|backward_rank|phalanx|supported|weak_unopposed|pawn_structure)_(mg|eg)\[([0-9]+|PS_FEATURE_COUNT)\] = \{[[:space:]]*-?[0-9]+([[:space:]]*,[[:space:]]*-?[0-9]+)*[[:space:]]*,?[[:space:]]*\};[[:space:]]*$'
if grep -qvE "$table" <<< "$code_lines"; then
  fail "CAND's src/ differs outside comments in more than whole one-line" \
       "definitions of the family's weights"
fi

# Every weight array and define outside the family's sixteen is REF's at CAND,
# compared by name (S125_screen.py only-family-pair, `apply`'s only-family
# between two commits): the line test reads the diff, this reads the values.
python3 adocs/data/S125_screen.py only-family-pair "$ref" "$cand" --tree . ||
  fail "CAND moves a weight outside the family's sixteen arrays against REF"

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
     OUT="/home/max/ws/chesso/.tuning/sprt_s125_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
