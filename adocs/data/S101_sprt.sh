#!/usr/bin/env bash
#
# S101: THE THREAT FAMILY -- the survivors of S101's screen, fitted on S083's corpus
# with every other constant frozen, against the phase 1 landing's engine, which ships
# the family at zero: one SPRT at the harness's 8+0.08, Hash 16, {0, 5} nElo (DEC-259
# (b), DEC-333 (7)), bisected along DEC-333 (7)'s partition on H0 or no verdict.
#
#   nohup adocs/data/S101_sprt.sh > .tuning/s101_sprt.log 2>&1 &
#
# Written 2026-11-01 in S101's phase 1, before any of it ran, in adocs/data/S125_sprt.sh's
# shape with the guards that shape strengthened (the build files beside src/, a family
# line read to its end, every other weight compared by name), and S121_sprt.sh's harness
# section -- S283_sprt.sh's before it -- relied on and not repeated. Nothing here was
# run. The pair does not exist yet; the coordinator pins it in the bracketed lines below,
# and until then the script refuses to start (the self-check at the bottom).
#
# THE COMMITS, EACH NAMED BY WHAT IS CHECKED OF IT. No guard names a commit for its
# place in the log, so a commit that touches neither src/ nor a build file -- S307,
# tests only, which may land between runs (DEC-333 (11)) -- trips none of them, wherever
# it falls. S308 touches src/ and lands after this step's close, and these guards refuse
# it before then, as they should.
#
#   screened  the commit the screen ran from, the engine every gain was read against.
#             Checked: its fourteen family arrays are all zero (S101_screen.py
#             landing-check --commit), at that commit and not at whatever is checked out;
#             and it is the commit .tuning/s101/bin/stamp names, which the screen wrote on
#             freezing its tuner and held every stage to (S101_screen.sh tools). A missing
#             stamp or another commit is refused, so `screened` cannot be re-pinned past a
#             src/ commit that landed after the screen. That file stays until the last run.
#   ref       what the candidate is played against. Checked: it descends from `screened`;
#             its src/ and build files are `screened`'s byte for byte, or for a later
#             bisection part that plus what has landed (THE PARTS below).
#   cand      the paste. Checked: its parent is `ref`; its src/ and build files are
#             `ref`'s but for src/evaluation.cpp, which differs outside whole-line comments
#             only in whole one-line definitions of the arrays this run moves, each read
#             to its closing `};` and nothing after it (a definition clang-format wrapped
#             is refused, not read: join it before pinning); and every weight array and
#             define outside the family's fourteen reads `ref`'s value by name
#             (S101_screen.py only-family-pair).
#
# So `cand` and `ref` differ by the fitted family and nothing else in src/ or the build's
# flags. The files the engine's build reads are src/ (src/CMakeLists.txt with it), the
# top-level CMakeLists.txt and cmake/; tests/ and tools/ build no part of the chesso
# target fastchess.sh plays.
#
# THE PAIR.
#
#   CAND  S101's candidate commit: `adocs/data/S101_screen.sh apply` pasted the screen's
#         candidate -- the last survivor's full-corpus fit, `--only` the survivors'
#         groups with their ties on S083's corpus (6226944 rows, sha256 2e0e3de9...,
#         S101_screen.sh's CORPUS_SHA) against S082's fixed validation set -- over the
#         family's arrays in src/evaluation.cpp; every golden the paste moved re-derived
#         by its own script (DEC-142); so the commit is green.
#   REF   CAND's parent.
#
# THE PARTS: HOW THE GUARDS READ A BISECTION PART. A part's run is this file with `ref=`,
# `cand=` and `part=` re-pinned and the coordinator's sentence rewritten; `screened=` stays.
# Each part's arrays are a subset of the family, and `part=` names that subset, so a part
# reads the same guards with a narrower list and never a wider one:
#
#   part   CAND may move                               REF may carry past `screened`
#   ""     the fourteen (narrowed at the pin)          nothing
#   A      threat_pawn, threat_piece, threat_minor     nothing: the paste reverted
#   B      hanging_piece, hanging_pawn, threat_king    A's arrays
#   C      threat_push                                 A's and B's
#
# What REF carries is read by the same line test, from `screened` to REF, and is what the
# parts before it may have landed. [COORDINATOR, at the pin: `moves` below narrowed to the
# survivors' arrays, so a dropped group's array cannot move, and this sentence replaced by
# what the screen kept.]
#
# WHAT THE VERDICT PRICES. The fitted survivors and their cost together: every stage-two
# evaluation pays the groups whose weights are live, the unions they read, one OR a piece,
# and a popcount a cell (DEC-333 (1)). The lazy shortcut's nodes pay none of it, 39.0 % of
# all evaluation calls on `bench`, since the family is stage two. A forced-weight
# prototype of the four kinds read +5.3 to +7.7 % instructions on `bench`, the largest
# evaluation cost yet, an estimate from a scratch build and no timing; the cost is recorded
# apart, on identical trees, by adocs/data/S101_cost.sh (DEC-047's method), and printed
# beside the verdict; it decides nothing, and there is no bar (DEC-333 (10)).
#
# THE TRANSFER GAP, WHICH THIS RUN IS THE ONE THING TO SEE (DEC-333 (8)). The corpus is
# quiescence leaves: 8.0 % of lesser and 8.6 % of hanging piece threats there have their
# attacker on move, against 54.2 % and 56.2 % of the search's lazy calls on `bench`, which
# also hold four to five times as many a call. One weight serves both. A fit reads almost
# one case; the screen cannot see the other, and this match plays both. On H0 this is the
# first lead: the cells doubled by side to move, unbuilt.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's control for
# every verdict but the correction family's, and this is an evaluation family. `{0, 5}`
# nElo at alpha = beta = 0.05: DEC-259 (b) names it for a family's survivors. What is known
# of the size, deciding nothing (DEC-019, the step file's section 1): Stash v26's initiative
# term built on threatened pieces, wider than this, +10.13 +/- 6.50 at 10+0.1 and +7.77 +/-
# 5.23 at 60+0.6. Ethereal's release notes give titles and no figure. Rejected: `{-5, 5}` --
# would ship a zero half the time (DEC-304's reason); `--fast` and `{0, 10}` -- DEC-063; one
# SPRT per part -- DEC-259 measures the family once and a part alone only on H0.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic (`adocs/testing_strategy.md`
# section 1.1): **41861 games** with the truth at the interval's midpoint, 19.84 h at the
# 2110 games an hour `.moltke.local.md` budgets 8+0.08 on `noob_3moves.epd` from (DEC-190);
# **25591 games** on a bound, 12.13 h. S125's run of the same pair and control played 2508
# games in 1 h 12 m to H1, S121's 3160 in 1 h 30 m, S135's 13406 in 6 h 17 m.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour, S283's, S135's, S136's,
# S121's, S123's and S125's rule. Four hours or more, so the run starts by DEC-155's night
# rule. Watcher ceiling 144000 s (40 h), wall-clock only while the machine is awake.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd` (DEC-189), concurrency
# 12 (DEC-050), fastchess.sh's adjudication, the governor recorded and not set (DEC-195). If
# fastchess, the book, the adjudication or the machine has moved since the last fixed-rounds
# A/A, that A/A of 1000 games comes first (DEC-143); the coordinator checks on the day.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes **1.0 % on
# either side** (`tools/forfeit_report.py` over the run's PGN in `$OUT`), on a crash or a
# disconnect on either side (`SPRT-RUN-INVALID`, S212), if the machine loses mains power
# (POWER) or another load starts on it (MACHINE), if the banner is not what the
# coordinator's sentence below says it must print, or at 40 h of wall clock with neither
# bound. Nothing else stops it.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S283's lines with this run's shas, dates
# and out directory --
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
#   out        /home/max/ws/chesso/.tuning/sprt_s101_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first `Started game 1
# of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171): S307 and S308,
# neither moving play, unless plan.md's Open list says otherwise on the day.
# [COORDINATOR, on the day it pins: CAND's and REF's shas into `ref=` and `cand=` below, the
# commit the screen ran from, as .tuning/s101/bin/stamp names it, into `screened=`, their
# dates into the banner lines above, the screen's seven readings and their sha256 (each
# part's d, s, m and verdict), the survivors
# and the H0 partition the screen printed, the corpus `locate` recorded, the candidate fit's
# sha256 and its tie files', the clamp share `apply` read against its 1.0 %, the cost run's
# reading, plan.md's Open list re-read, the A/A rule checked, and this bracketed sentence
# replaced by what was found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted family is accepted as gaining: H1's 5 nElo
#                   preferred to H0's 0 at alpha = beta = 0.05. **It ships.** A part the
#                   screen dropped stays at zero, in no expression, recorded as a zero
#                   (S126's rule). The stopping run's Elo is upward-biased (DEC-063) and
#                   is not the effect size. S122 inherits the clamp's readings and the
#                   sets the survivors build; `LAZY_EVAL_MARGIN` is its to re-derive
#                   (DEC-324, DEC-333 (6)).
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the family back
#                   to zero, the goldens back -- and the family is bisected along DEC-333
#                   (7)'s partition, survivors only, in order: A = P1 to P3, attacked by a
#                   lesser or a like piece, | B = P4 to P6, undefended, | C = P7, the push;
#                   a part with no survivor does not exist. Each part its own `{0, 5}` SPRT
#                   in this file's shape -- at worst three more runs after this one, four in
#                   all, each up to 19.86 h, 79.4 h with this run. Each plays over what has
#                   landed by then, its groups refitted then (`adocs/data/S101_screen.sh part
#                   NAME`); a part's H1 lands before the next is fitted; a part's H0 or no
#                   verdict is that part's zero. The cells doubled by side to move are the
#                   first lead (DEC-333 (8)), unbuilt, and no part of this bisection.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound: reverted
#                   whole as H0 is, recorded as **unresolved**, and bisected as H0 is
#                   (DEC-259 (c)).
#
# ON ANY OUTCOME, before any commit: `MATE_IN_THREE_FLOOR` read under 11 stops for the owner
# (DEC-303, DEC-330); a candidate over 1.0 % on the clamp has stopped phase 2 before this
# run exists (DEC-333 (6)).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref <REF>, 8+0.08,
# Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`, `Games`, `Wall` and `Log`
# lines -- with the log trimmed into `adocs/data/S101_sprt.log` by `tools/trim_console.py`;
# `tools/ledger.py` regenerates plan.md's ledger table from it.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or SPRT-RUN-FAILED on
# every exit path, and this script execs it and adds nothing after.

set -uo pipefail

fail()
{
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

# THE BOOKED SETTINGS, AND NOTHING ELSE FROM THE CALLER'S ENVIRONMENT, as S136_sprt.sh's
# same block reads them.
ref="[COORDINATOR: REF's sha]"
cand="[COORDINATOR: CAND's sha]"
screened="[COORDINATOR: the sha the screen ran from]"
part=""           # THE PARTS above: empty for the family, or A, B or C
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"
sprt_rounds="20950"

# THE COORDINATOR'S SENTENCE AND SHAS. While the bracketed marker is anywhere in this file
# the pre-registration is unfinished and the run does not start. The marker is assembled in
# two halves so this line does not match itself.
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

# `cand`'s parent is `ref`, so the run measures S101's paste and nothing else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S101"
fi

# The files the engine's build reads: src/ (src/CMakeLists.txt with it), the top-level
# CMakeLists.txt and cmake/'s flag files.
build_paths=(src CMakeLists.txt cmake)

# THE PARTS: what this run may move, and what REF may carry past `screened`.
part_a="threat_pawn|threat_piece|threat_minor"
part_b="hanging_piece|hanging_pawn|threat_king"
part_c="threat_push"
case "$part" in
  "") moves="$part_a|$part_b|$part_c"
      carries="" ;;
  A)  moves="$part_a"
      carries="" ;;
  B)  moves="$part_b"
      carries="$part_a" ;;
  C)  moves="$part_c"
      carries="$part_a|$part_b" ;;
  *)  fail "part is '$part', not empty, A, B or C" ;;
esac

# Whether FROM and TO differ under src/, outside whole-line comments, only in whole one-line
# definitions of ARRAYS, each to its closing `};` and nothing after it: no continuation
# line, and no code appended to a definition.
only_definitions()
{
  local lines form
  lines="$(git diff -U0 "$1" "$2" -- src |
           grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' | cut -c2- |
           grep -vE '^[[:space:]]*(//.*)?$')"
  form='^constexpr int ('"$3"')_(mg|eg)\[[0-9]+\] = \{[[:space:]]*-?[0-9]+([[:space:]]*,[[:space:]]*-?[0-9]+)*[[:space:]]*,?[[:space:]]*\};[[:space:]]*$'
  ! grep -qvE "$form" <<< "$lines"
}

# `screened`: the family at zero, read at that commit. A tree whose arrays are not all zero
# is not the evaluation any gain was read against.
python3 adocs/data/S101_screen.py landing-check --tree . --commit "$screened" ||
  fail "the commit named as the one the screen ran from, $screened, is not the family at zero"

# And it is the commit the screen recorded. Every other guard compares `screened` with itself
# or with REF, so a src/ commit landing after the screen, `screened` re-pinned past it, would
# pass them all and play gains read on the old engine on the new one. The stamp holds a short
# sha, so the two are compared as the commits they name.
stamp_file=".tuning/s101/bin/stamp"
[[ -f "$stamp_file" ]] ||
  fail "no $stamp_file, so nothing records the commit the screen ran from"
stamped="$(git rev-parse --verify --quiet "$(cat "$stamp_file")^{commit}")" ||
  fail "$stamp_file holds '$(cat "$stamp_file")', which names no commit"
if [[ "$(git rev-parse --verify --quiet "$screened^{commit}")" != "$stamped" ]]; then
  fail "screened $screened is not $stamped, the commit $stamp_file names"
fi

# `ref`: it descends from `screened`, its engine `screened`'s; a later bisection part's REF
# may also carry what has landed by then, and nothing else. A commit touching neither
# src/ nor a build file between the two changes nothing here (S307, DEC-333 (11)).
if ! git merge-base --is-ancestor "$screened" "$ref"; then
  fail "REF does not descend from the screened commit $screened"
fi
ref_changed="$(git diff --name-only "$screened" "$ref" -- "${build_paths[@]}")"
if [[ -n "$ref_changed" ]]; then
  [[ -n "$carries" ]] ||
    fail "REF's src/ or build files differ from the screened commit's -- REF is not the" \
         "family at zero"
  [[ "$ref_changed" == src/evaluation.cpp ]] ||
    fail "REF changes '${ref_changed//$'\n'/ }' beyond what part $part may have landed"
  only_definitions "$screened" "$ref" "$carries" ||
    fail "REF's src/ differs from the screened commit's outside comments in more than whole" \
         "one-line definitions of what part $part may have landed"
fi

# `cand`: its src/ and build files are REF's but for src/evaluation.cpp.
changed="$(git diff --name-only "$ref" "$cand" -- "${build_paths[@]}")"
if [[ -z "$changed" ]]; then
  fail "CAND's src/ is REF's -- not S101's paste"
fi
while IFS= read -r file; do
  [[ "$file" == src/evaluation.cpp ]] ||
    fail "CAND changes $file beyond the paste of the family's weights"
done <<< "$changed"

# And there only whole one-line definitions of the arrays this run moves.
only_definitions "$ref" "$cand" "$moves" ||
  fail "CAND's src/ differs outside comments in more than whole one-line definitions of" \
       "the arrays this run moves ($moves)"

# Every weight array and define outside the family's fourteen is REF's at CAND, compared by
# name (S101_screen.py only-family-pair, `apply`'s only-family between two commits): the
# line test reads the diff, this reads the values.
python3 adocs/data/S101_screen.py only-family-pair "$ref" "$cand" --tree . ||
  fail "CAND moves a weight outside the family's fourteen arrays against REF"

[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh"

# THE HARNESS: fastchess.sh must read both names, or it ignores them and plays its own
# `{0, 5}` to 20000 rounds.
for name in SPRT_BOUNDS SPRT_ROUNDS; do
  if ! grep -qE '\$\{?'"$name"'([^A-Za-z0-9_]|$)' ./fastchess.sh; then
    fail "./fastchess.sh does not read $name, so this run's bounds or cap would be" \
         "ignored -- see THE HARNESS in adocs/data/S283_sprt.sh"
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s101_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
