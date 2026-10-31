#!/usr/bin/env bash
#
# S125: THE PAWN STRUCTURE FAMILY -- the survivors of S125's screen (P2, isolated
# pawns by file; P5, phalanx pawns; P6, supported pawns; P7, weak unopposed
# pawns) with P0's three flats refitted beside them, fitted on S083's corpus with
# every other constant frozen, against the phase 1 landing's engine, which ships
# the family at zero: one SPRT at the harness's 8+0.08, Hash 16, {0, 5} nElo
# (DEC-259 (b)), bisected along DEC-331 (7)'s partition on H0 or no verdict. The
# screen dropped P1, the isolated and backward increment, P3, doubled pawns by
# file, and P4, backward pawns not isolated by rank: their arrays stay at zero,
# in no expression, and this run does not measure them.
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
# 2026-10-31, S125's phase 2, re-read against the candidate before it is pinned
# (the step file's "2026-10-31: the screen and the candidate"). The screen kept
# P2, P5, P6 and P7 and dropped P1 (d -0.000004 against m 0.000003), P3
# (-0.000002 against 0.000011) and P4 (+0.000005 against 0.000008): `CANDIDATE
# 02567 groups pawn_structure,isolated_files,phalanx,supported,weak_unopposed`,
# the fit .tuning/s125/fits/02567/full.hpp (sha256 176755da...), pasted by
# `S125_screen.sh apply` over ten arrays, bench 5434393 -> 5707449. The guards
# below hold, read on the worktree against its HEAD `f08d9db`, and were narrowed
# and shown on a scratch pin: src/evaluation.cpp is the only file under src/,
# CMakeLists.txt and cmake/ that differs; outside whole-line comments its 20
# changed lines are the ten definitions, old and new; and every weight outside
# them reads HEAD's by name. The bisection the screen printed is A = P0,P2 | B =
# P0,P5,P6 | C = P0,P7, three parts, so the H0 row's "at worst three more runs
# after this one, four in all" holds as written. The verdict pays the set the
# candidate computes, mask 114, adocs/data/S125_cost.sh's row `cand`, which the
# coordinator times before this run; `all`'s +4.20 % also forces P1, P3 and P4
# and is not its row (one group alone: p2 +1.18 %, p5 +1.36 %, p6 +1.05 %, p7
# zero; adocs/data/S125_cost_reading.txt). There is no clamp reading: the family
# is stage one.
#
# THE LANDING, AND WHY `landing=` IS NOT THE LANDING'S COMMIT. S125's phase 1
# landed the family at zero in `b5f133c`. The screen ran from `f08d9db`, which is
# `b5f133c` with S299 to S302 on top, none of them touching src/ or the top-level
# CMakeLists.txt; the sole build-file difference between the two is S302's
# cmake/pgo.cmake, comments and a branch only a `CHESSO_PGO=use` build takes,
# which fastchess.sh's build is not. `landing=` names `f08d9db`, the tree every
# gain was read against (DEC-331 (14)), and REF is held to it. What makes the
# sentence above true is checked at run time, not assumed: `phase1` is an
# ancestor of `landing`, and the only path under src/, CMakeLists.txt and cmake/
# that differs between them is exactly cmake/pgo.cmake.
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
#   REF   CAND's parent: a descendant of `landing` whose src/ and build files
#         are `landing`'s byte for byte, the family at zero and in no expression
#         (adocs/data/S125_inv6.txt). A later bisection part's REF may also
#         carry what has landed by then, and nothing else (THE PARTS below).
#
# The two differ by the fitted family and flats and nothing else in src/ or the
# build's flags, which the guards below check:
#   - CAND^ is REF, and REF descends from `landing`;
#   - `landing` is the phase 1 landing's engine but for cmake/pgo.cmake (THE
#     LANDING above);
#   - REF's src/, its top-level CMakeLists.txt and cmake/ are `landing`'s byte
#     for byte (src/CMakeLists.txt is under src/; the engine target reads no
#     other build file);
#   - CAND's are REF's but for src/evaluation.cpp, which differs outside
#     whole-line comments only in whole one-line definitions of the ten arrays
#     this run moves -- pawn_structure (P0's flats, DEC-331 (5)), isolated_file,
#     phalanx, supported and weak_unopposed, mg and eg -- each read to its
#     closing `};` and nothing after it. A definition clang-format wrapped onto
#     a second line is refused, not read: join it before pinning. The three
#     dropped groups' six arrays are not in that list, so they cannot move: REF
#     holds them at zero, CAND holds what REF holds, and a group at zero is in
#     no expression (the liveness flags in src/evaluation.cpp);
#   - and every weight array and define outside the family's sixteen reads REF's
#     value by name (S125_screen.py only-family-pair).
#
# THE PARTS: HOW THE GUARDS READ A BISECTION PART. A part's run is this file with
# `ref=`, `cand=` and `part=` re-pinned and the coordinator's sentence rewritten;
# `landing=` and `phase1=` stay. Each part's arrays are a subset of the ten, and
# `part=` names that subset, so a part reads the same guards with a narrower list
# and never a wider one:
#
#   part   CAND may move                         REF may carry past `landing`
#   ""     the ten, the family                   nothing
#   A      pawn_structure, isolated_file         nothing: the paste reverted
#   B      pawn_structure, phalanx, supported    pawn_structure, isolated_file
#   C      pawn_structure, weak_unopposed        those and phalanx, supported
#
# What REF carries is read by the same line test, from `landing` to REF, and is
# what the parts before it may have landed: B's REF holds A's arrays if A read
# H1, C's holds A's and B's as they read. The flats are in every part because
# every part refits them over what has landed (the H0 row). The family's run and
# part A take no REF past `landing`, as this guard always read.
#
# WHAT THE VERDICT PRICES. The fitted survivors and their cost together:
# every evaluation pays the four groups whose weights are live -- a few set
# operations over the pawn fills, a popcount for each count, a loop over the
# pawns of each table -- at every node, the shortcut's nodes included, since
# the family is stage one (DEC-331 (8)); the flats' refit costs nothing and, at
# CAND, the dropped groups cost nothing, being in no expression. That holds for
# CAND and not after the close: the close's fold (DEC-331 (3), (15)) makes
# isolated_backward live, so the engine then also computes P1 and the
# `& ~isolated`, which this verdict does not price. The fold adds P1's forced
# row, +0.37 % instructions, read zero with its cycles and wall CIs reaching
# inside the A/A floor, and S118's baseline is read with both rows, `cand` and
# p1. The
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
#   candidate  cf8661b  2026-10-31 07:21
#   reference  9cbeb68  2026-10-31 05:55
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
# Pinned by the coordinator 2026-10-31 07:55: CAND `cf8661b` (committed
# 2026-10-31 07:21, the screen's 02567 fit pasted over ten arrays, bench
# 5707449) and REF `9cbeb68` (committed 2026-10-31 05:55, S303's landing, tools
# and documents only, `src/` and the build files byte for byte `f08d9db`'s,
# bench 5434393), CAND's parent; `landing=` is `f08d9db` (committed 2026-10-30
# 13:10), the commit the screen ran from, `b5f133c` and S299 to S302 with the
# family at zero. The screen's eight readings (`.tuning/s125/read/`,
# concatenated in `adocs/data/S125_screen_reading.txt`, sha256 67a1c678...): P0
# g +0.000007 s 0.000003 m 0.000005 recorded (P0.txt 8e5b72e7...); P1 d
# -0.000004 against P0, s 0.000002 m 0.000003 dropped (f644fc59...); P2 d
# +0.000012 against P0, s 0.000003 m 0.000004 kept (01611811...); P3 d -0.000002
# against P2, s 0.000006 m 0.000011 dropped (5defa9bc...); P4 d +0.000005 against
# P2, s 0.000005 m 0.000008 dropped (ac349f44...); P5 d +0.000036 against P2, s
# 0.000002 m 0.000003 kept (806c7a04...); P6 d +0.000031 against P5, s 0.000003 m
# 0.000005 kept (8afe0105...); P7 d +0.000017 against P6, s 0.000003 m 0.000005
# kept (9c34d684...). Survivors P2, P5, P6, P7, the H0 partition A = P0,P2 | B =
# P0,P5,P6 | C = P0,P7. The corpus `locate` recorded: S083's curve 0, 6226944
# rows, sha256 2e0e3de9... The candidate fit `.tuning/s125/fits/02567/full.hpp`,
# sha256 176755da...; the tie files, phalanx[5] mg and eg tied to their sixth,
# ties_P0.txt 2558330d..., ties_P1.txt 614d3b97..., ties_P2.txt 712df8f3...,
# ties_P3.txt 88521c8f..., ties_P4.txt 64a1dd30..., ties_P5.txt 9e1f4c7d...,
# ties_P6.txt 0307403a..., ties_P7.txt 705b1840.... The cost
# (`adocs/data/S125_cost_reading.txt`, d8c55c2b...): cand, the candidate's set,
# +3.50 % on its own A/A floor; p2 +1.18 %, p5 +1.36 %, p6 +1.05 %, p7 zero, all
# +4.20 % with the dropped groups forced. The Open list at `cf8661b` re-read: S304
# is the one open DEC-171 filler, tools only, `spsa_driver.py` and
# `depth_vs_eval.py`, neither run by this match. Both mate floors hold,
# `EXACT_FLOOR` 159 against 168 and 158, `MATE_IN_THREE_FLOOR` 11 against 12 and
# 10. The harness is S123's run's of 2026-10-30 -- fastchess alpha 1.8.1
# 20260720-daa3ea2, `noob_3moves.epd`, `fastchess.sh` last changed `769ce7a`
# (S284), the same workstation and kernel 7.2.6 -- so no A/A is owed (DEC-143).
# Before the run: the commit gate green at CAND (both fast suites 50 of 50, format
# and the prose checks clean), `gate_extra` 6 stages green on CAND's tree, Debug
# self-play 8 games, 0 `Assertion`, 0 `disconnect`, and a dry copy of this script
# printed DRY-RUN-WOULD-EXEC.
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
#                   true. P1 was dropped, so the increment is zero and IB's own
#                   weight at the fold is backward's flat alone, {-6} and {0} at
#                   this fit: the one dropped group the fold takes out of zero,
#                   by DEC-331 (3)'s arithmetic and at no change of score. The
#                   fold is owed on every verdict (DEC-331 (15)).
#                   P3 and P4 stay at zero, in no expression, recorded as zeros
#                   (S126's rule). The stopping run's Elo is upward-biased
#                   (DEC-063) and is not the effect size.
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the
#                   family back to zero, the flats back to S083's, the goldens
#                   back -- and the family is bisected along DEC-331 (7)'s
#                   partition as ruled at phase 1's fast check, survivors
#                   only, in order: A = P1 to P4, the reshaped existing terms,
#                   B = P5 and P6, connected pawns, C = P7, weak unopposed
#                   pawns; a part with no survivor does not exist. The screen
#                   printed A = P0,P2 | B = P0,P5,P6 | C = P0,P7, so A is
#                   `--only pawn_structure,isolated_files`, B
#                   `pawn_structure,phalanx,supported` with P5's two ties, and C
#                   `pawn_structure,weak_unopposed`. P0's flats
#                   never land alone: every part that exists frees them with
#                   its groups, as every screen part did, so they enter with
#                   the first and each later part refits them over what has
#                   landed. So B and C under H0 are not the screen's fits, which
#                   carried the parts before them free; a part's fit is refit
#                   over what has landed and read by THE PARTS' guards above.
#                   Each part its own `{0, 5}` SPRT in this file's
#                   shape -- at worst three more runs after this one, four in
#                   all, each up to 19.86 h, 79.4 h with this run. Each plays
#                   over what has landed by then, its groups refitted then
#                   (`adocs/data/S125_screen.sh part NAME`); a part's H1 lands
#                   before the next is fitted; a part's H0 or no verdict is
#                   that part's zero. **The fold of DEC-331 (3) is owed at S125's
#                   close on this outcome too, as on H1** (DEC-331 (15): the
#                   accepts' clause is unconditional): neutral (INV-6), IB taking
#                   backward's flat as it stands on the tree at the close, the
#                   increment being zero. On a family fully H0 that is the
#                   parent's flats, {-11} and {-2}; with a part landed, that
#                   part's refit of them.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   reverted whole as H0 is, recorded as **unresolved**, and
#                   bisected as H0 is (DEC-259 (c)). **The fold is owed here
#                   too**, as on H0, over the flats the tree carries at the
#                   close: the parent's, {-11} and {-2}, if no part lands
#                   (DEC-331 (15)).
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
ref="9cbeb68"
cand="cf8661b"
landing="f08d9db"
phase1="b5f133c"  # the commit that landed the family at zero
part=""           # THE PARTS above: empty for the family, or A, B or C
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

# THE PARTS: what this run may move, and what REF may carry past `landing`.
case "$part" in
  "") moves="pawn_structure|isolated_file|phalanx|supported|weak_unopposed"
      carries="" ;;
  A)  moves="pawn_structure|isolated_file"
      carries="" ;;
  B)  moves="pawn_structure|phalanx|supported"
      carries="pawn_structure|isolated_file" ;;
  C)  moves="pawn_structure|weak_unopposed"
      carries="pawn_structure|isolated_file|phalanx|supported" ;;
  *)  fail "part is '$part', not empty, A, B or C" ;;
esac

# Whether FROM and TO differ under src/, outside whole-line comments, only in
# whole one-line definitions of ARRAYS, each to its closing `};` and nothing
# after it: no continuation line, and no code appended to a definition.
only_definitions()
{
  local lines form
  lines="$(git diff -U0 "$1" "$2" -- src |
           grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' | cut -c2- |
           grep -vE '^[[:space:]]*(//.*)?$')"
  form='^constexpr int ('"$3"')_(mg|eg)\[([0-9]+|PS_FEATURE_COUNT)\] = \{[[:space:]]*-?[0-9]+([[:space:]]*,[[:space:]]*-?[0-9]+)*[[:space:]]*,?[[:space:]]*\};[[:space:]]*$'
  ! grep -qvE "$form" <<< "$lines"
}

# `landing` is the commit the screen ran from (THE LANDING above): the phase 1
# landing's engine, S302's cmake/pgo.cmake apart, and nothing else apart.
if ! git merge-base --is-ancestor "$phase1" "$landing"; then
  fail "the phase 1 landing $phase1 is not an ancestor of landing $landing"
fi
landing_diff="$(git diff --name-only "$phase1" "$landing" -- "${build_paths[@]}")"
if [[ "$landing_diff" != cmake/pgo.cmake ]]; then
  fail "landing's src/ and build files differ from $phase1's in" \
       "'${landing_diff//$'\n'/ }', and cmake/pgo.cmake is the one difference" \
       "it may hold"
fi

# REF's engine is `landing`'s: the family at zero, the flats S083's; a later
# bisection part's REF may also carry what has landed by then, and nothing else.
if ! git merge-base --is-ancestor "$landing" "$ref"; then
  fail "REF does not descend from landing $landing"
fi
ref_changed="$(git diff --name-only "$landing" "$ref" -- "${build_paths[@]}")"
if [[ -n "$ref_changed" ]]; then
  [[ -n "$carries" ]] ||
    fail "REF's src/ or build files differ from landing's -- REF is not the" \
         "family at zero or a descendant of it with landing's src/"
  [[ "$ref_changed" == src/evaluation.cpp ]] ||
    fail "REF changes '${ref_changed//$'\n'/ }' beyond what part $part may" \
         "have landed"
  only_definitions "$landing" "$ref" "$carries" ||
    fail "REF's src/ differs from landing's outside comments in more than whole" \
         "one-line definitions of what part $part may have landed"
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

# And there only whole one-line definitions of the arrays this run moves. A
# dropped group's arrays are not among them, so they read REF's zero at CAND.
only_definitions "$ref" "$cand" "$moves" ||
  fail "CAND's src/ differs outside comments in more than whole one-line" \
       "definitions of the arrays this run moves ($moves)"

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
