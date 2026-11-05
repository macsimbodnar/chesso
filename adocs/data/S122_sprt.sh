#!/usr/bin/env bash
#
# S122: THE KING SAFETY FAMILY -- DEC-353's candidate `1245`, the survivors P1 (safe checks),
# P2 (weak squares), P4 (the storm) and P5 (the curve) fitted on S122's corpus at
# LAZY_EVAL_MARGIN 275 with every other constant frozen, the margin moved 184 -> 275 with
# them, against the tree that ships the family at its seed and the margin at 184: one SPRT
# at the harness's 8+0.08, Hash 16, {0, 5} nElo (DEC-259 (b), DEC-344 (6)), bisected along
# DEC-353's partition on H0 or no verdict.
#
#   cd /home/max/ws/chesso-s122p2 && nohup adocs/data/S122_sprt.sh > /home/max/ws/chesso/.tuning/s122_sprt.console 2>&1 &
#
# Written 2026-11-05 in S122's phase 2, before any of it ran, in adocs/data/S101_sprt.sh's
# working-tree form (DEC-334, DEC-254), re-pinned to S122. Why the working tree, as first
# written: the tree was not green -- tests/test_tuner_gradient.cpp "the clamp's derivative is
# exact where it binds" read its CLAMP_PUSH red, its script's rule answering no push at the
# candidate, and adocs/data/S101_gradient_rows.cpp no longer reproduced DEC-316's pair there,
# its pick being candidate 5. The coordinator first read the two as DEC-344 (9)'s third
# recurrence on test_tuner_gradient's bound, for the owner (adocs/data/S122_clamp_reading.txt,
# adocs/data/S122_gradient_options.txt). **DEC-354, 2026-11-05, the coordinator's ruling,
# applied DEC-142 as written**: the pair re-derived by its script, candidate 5; CLAMP_PUSH then
# +50 by its own; the gradient case 8.61e-7 at 984 under its 1e-6, so the recurrence does not
# fire and nothing is with the owner. The tree is green; the candidate is played uncommitted
# all the same, so that what plays is the staged diff the fast check read and nothing landed
# beside it. Both mate floors hold (`EXACT_FLOOR` 157 against 166 shipping and 156 weakened,
# `MATE_IN_THREE_FLOOR` 11 against 12 and 11). The
# run plays the uncommitted, staged tree of the worktree `/home/max/ws/chesso-s122p2`
# against its HEAD, fastchess.sh's dirty-tree mode (S256, DEC-254): the candidate is named
# `cand-<HEAD>+<hex>` and the run saves its diff as `$OUT/candidate.diff`. Nothing here was
# run. The coordinator pins REF and fills the bracketed lines below, and until then the
# script refuses to start (the self-check at the bottom).
#
# THE COMMITS AND THE TREE, EACH NAMED BY WHAT IS CHECKED OF IT. No guard names a commit
# for its place in the log, so a commit that touches neither src/ nor a build file trips
# none of them, wherever it falls.
#
#   screened  the commit the screen ran from, `2bf3ad4`, the engine every gain was read
#             against. Checked: its family is at its seed (S122_screen.py landing-check
#             --commit), at that commit and not at whatever is checked out; and it is the
#             commit /home/max/ws/chesso/.tuning/s122/screen/bin/stamp names, which the
#             screen wrote on freezing its tools and held every stage to (S122_screen.sh
#             tools); the main checkout's file, gitignored and absent from the worktree.
#   ref       what the candidate is played against, and the worktree's HEAD, `65606a9`
#             at this script's writing (part 1 of S122's phase 2, `apply` ruled). Checked:
#             it descends from `screened`; its src/ and build files are `screened`'s byte
#             for byte, or for a later bisection part that plus what has landed (THE
#             PARTS below). A commit cannot hold its own sha, so the pin is an uncommitted
#             edit of the worktree's copy of this file, and the run's candidate.diff holds
#             it, as S101's and S136's did.
#   the tree  the paste: `ref` plus the staged diff, uncommitted. Checked: it is dirty;
#             nothing unstaged under src/, tests/, cmake/ or CMakeLists.txt, nothing
#             untracked under src/ or cmake/; cmake/ and CMakeLists.txt are `ref`'s; under
#             src/ only src/evaluation.cpp, src/evaluation.hpp and src/search_params.hpp
#             differ from `ref`, and outside whole-line comments only in whole definitions of
#             the arrays this run moves, each read to its closing `};` across the lines
#             clang-format wrapped it over and nothing after it, and in the one
#             LAZY_EVAL_MARGIN row of src/search_params.hpp where this run moves the margin;
#             every weight array and define outside the family's arrays reads `ref`'s value
#             by name (S122_screen.py only-family, without --allow-five: the linear five
#             are `ref`'s); and build/'s binary is no older than any source it is built from.
#
# THE PAIR.
#
#   CAND  the working tree of `/home/max/ws/chesso-s122p2`, uncommitted: its HEAD (REF) plus
#         S122's staged paste. `adocs/data/S122_screen.sh apply TREE --ruled DEC-353 FIT`
#         pasted DEC-353's candidate -- `.tuning/s122/screen/fits/1245_e20000/full.hpp`,
#         sha256 50bd33bd..., the screen's decided fit for P5, `--only` its groups with
#         their ties on S122's corpus (6307294 rows, sha256 04c596de...) against its fixed
#         validation set at margin 275 and 20000 epochs -- over the family's arrays in
#         src/evaluation.cpp, and moved the margin row 184 -> 275; every golden the paste
#         moved re-derived by its own script (DEC-142) but CLAMP_PUSH (above).
#   REF   the worktree's HEAD, which the coordinator pins below.
#
# THE PARTS: HOW THE GUARDS READ A BISECTION PART (DEC-353, on H0 or no verdict). A part's
# run is this file over its own uncommitted tree, with `ref=`, `part=` and, for M, the bounds
# re-pinned and the coordinator's sentence rewritten; `screened=` stays:
#
#   part   CAND may move                                  REF may carry past `screened`
#   ""     ks_checks, ks_weak, ks_storm, ks_curve; margin  nothing
#   M      the margin row alone, 184 -> 275, at {-5, 0}    nothing
#   A      ks_checks                                      the margin row, if M landed
#   B      ks_weak, ks_storm                              the margin row; A's arrays
#   C      ks_curve                                       the margin row; A's and B's arrays
#
# M is the margin alone at the shipped constants, `{-5, 0}`, the only part at other bounds:
# its answer says whether A to C refit at 275 (`S122_screen.sh part NAME --ruled DEC-353`)
# or at 184 (the same with `--margin 184`). In A to C the margin row is REF's, 275 if M
# landed, 184 if not, and never moves in the part's own tree.
#
# WHAT THE VERDICT PRICES. The fitted survivors, the margin and their cost together: every
# stage-two evaluation pays the counts the live groups read and the curve's twelve products
# a king. The lazy shortcut's nodes pay none of it, the family being stage two, and the
# wider margin lets the shortcut fire less often. The cost is recorded apart, on identical
# trees, by adocs/data/S122_cost.sh, the `cand` row at mask 27, P1 + P2 + P4 + P5, and
# printed beside the verdict; it decides nothing, and there is no bar.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's control for an
# evaluation family. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names it for a
# family's survivors, and DEC-344 (6) the `{-5, 0}` for the margin alone. Rejected:
# `{-5, 5}`, `--fast` and `{0, 10}`, one SPRT per part (S101_sprt.sh's reasons).
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic (`adocs/testing_strategy.md`
# section 1.1, D = 1046535 and 639770 over the bounds' width squared, 25): **41861 games**
# with the truth at the interval's midpoint, 19.84 h at the 2110 games an hour
# `.moltke.local.md` budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** on a
# bound, 12.13 h. S101's run of the same pair and control played 1636 games to H1.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour, S101's rule. Four hours
# or more, so the run starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h),
# wall-clock only while the machine is awake.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd` (DEC-189), concurrency
# 12 (DEC-050), fastchess.sh's adjudication, the governor recorded and not set (DEC-195). If
# fastchess, the book, the adjudication or the machine has moved since the last fixed-rounds
# A/A, that A/A of 1000 games comes first (DEC-143); the coordinator checks on the day.
#
# ABORT RULE, S101's. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's PGN in `$OUT`), on a
# crash or a disconnect on either side (`SPRT-RUN-INVALID`, S212), if the machine loses mains
# power (POWER) or another load starts on it (MACHINE), if the banner is not what the
# coordinator's sentence below says it must print, or at 40 h of wall clock with neither
# bound. Nothing else stops it.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S283's lines with this run's shas, dates
# and out directory --
#
#   candidate  <REF>  <its date>  + uncommitted changes
#   diff       /home/max/ws/chesso/.tuning/sprt_s122_<stamp>/candidate.diff  blob <12 hex>
#   reference  <REF>  <its date>
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s122_<stamp>
#
# with no `relay` or `A/A CALIBRATION` line, fastchess's engine names `cand-<REF, short>+<the
# same 12 hex>` and `ref-<REF, short>`, the candidate's `id name` carrying `-dirty`, and
# fastchess's first `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171): none of phase 2's --
# CLAMP_PUSH and DEC-316's pair, first left for the owner, were re-derived by DEC-354 -- and
# whatever plan.md's Open list says on the day.
# Pinned by the coordinator 2026-11-05 12:00: REF `65606a9` (committed 2026-11-05 06:32, S122
# phase 2 part 1, `apply --ruled`, documents and S122_screen.* only, `src/` and the build files
# byte for byte `screened`'s, bench 5482277); `screened` `2bf3ad4` (committed 2026-11-04 20:43),
# the commit `.tuning/s122/screen/bin/stamp` names. The tree: `/home/max/ws/chesso-s122p2`, REF
# plus the staged diff, `.tuning/coord/S122/S122_phase2.diff`, sha256 f93eded8..., 33 files, the
# paste of DEC-353's `1245` at margin 275 and DEC-354's fixture, bench 4707943 in both builds;
# this pin is an unstaged edit of this file beside it. The cost (`S122_cost.sh`, idle machine,
# CAND b62becb, PARENT 1b2539a): all +5.57 % on its A/A floor, p1 +2.07 %, p4 +1.63 %, p5
# +0.82 %, p2, p3 and the parent zero (`.tuning/s122/cost/reading.txt`, a5f93459...); the cand
# row at mask 27 +5.28 %, forced instructions +6.67 % (bench 16) and +6.96 % (pos11)
# (`.tuning/s122/cost_cand/reading.txt`, 051d5d1b...); nps on its binaries, five interleaved
# `bench` pairs, cand 3789756 against zero 3980783, -4.80 %. The Open list at `65606a9`
# re-read: S332 and S333, filler that moves no play; S124 onward are strength steps not begun
# in this tree. The harness is S101's of 2026-11-01 -- fastchess alpha 1.8.1 20260720-daa3ea2,
# `noob_3moves.epd` sha256 2011193b..., `fastchess.sh` last changed `ba1cad6` (S319, comments
# only), the same workstation and kernel 7.2.6 -- so no A/A is owed (DEC-143). Before the run,
# on the tree: both fast suites 57 of 57, format clean, the five prose modes 0; `tools/gate_extra.sh`
# 6 of 6 stages green, 1669 s (prose, citations, debug, sanitize 57 of 57 with no sanitizer
# report, perft, clang 57 of 57 in both); Debug self-play 8 games at 4+0.04, 0 `Assertion`, 0
# `disconnect`, 0 crashes; the fast check over the phase and the one over DEC-354's delta, every
# finding fixed; a dry copy of this script printed DRY-RUN-WOULD-EXEC.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted family at margin 275 is accepted as gaining: H1's
#                   5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It ships**: the tree
#                   lands, green since DEC-354, with the DEC-142
#                   items phase 2 re-derived, the step's specs.md rows, and the record
#                   commit's block. P3 and P6 stay where the screen left them (P3 at zero,
#                   P6's patch unapplied) and P7 is not taken (DEC-353), each recorded with
#                   its figures. The stopping run's Elo is upward-biased (DEC-063) and is not
#                   the effect size.
#
#   H0           -> not shown to gain. **Nothing was committed, so the paste is simply not
#                   landed**: REF's family stays at its seed and the margin at 184, with REF's
#                   goldens. DEC-353's bisection follows, survivors only, in order, each its
#                   own uncommitted tree in this file's shape, each stopping for the
#                   coordinator: M 275 alone at `{-5, 0}`, then A = P1, B = P2 + P4,
#                   C = P5, each `{0, 5}` against what has landed, each refit by
#                   `adocs/data/S122_screen.sh part NAME --ruled DEC-353`, at 275 or, on
#                   M's word, `--margin 184` -- at worst four more runs after this one, each
#                   up to 19.86 h, 99.3 h with this run. A part's H1 lands after its verdict
#                   and before the next is fitted; its H0 or no verdict is that part's zero.
#                   Leads, none built: the check cells doubled by side to move; shelter inside
#                   x; the linear five retired (DEC-353, S126's).
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound: not landed,
#                   as H0 is, recorded as **unresolved**, and bisected as H0 is (DEC-259 (c)).
#
# ON ANY OUTCOME, before any commit: `EXACT_FLOOR` under 157, `MATE_IN_THREE_FLOOR` under 11,
# a mate-carry ceiling rising by more than 2, or a third recurrence on test_tuner_gradient's
# bound stops for the owner (DEC-344 (9)); none fired at phase 2. The gradient's recurrence was
# first put to the owner and, by DEC-354, does not fire: the bound holds at 8.61e-7 (above).
#
# THE COMMIT THAT CLOSES THE VERDICT -- the record commit on H0 or no verdict, on H1 the one
# that lands the tree, green -- carries DEC-220's block -- `SPRT | cand <REF>+<hex> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`, `Games`,
# `Wall` and `Log` lines -- with the log trimmed into `adocs/data/S122_sprt.log` by
# `tools/trim_console.py` and the run's `candidate.diff` committed beside it as
# `adocs/data/S122_candidate.diff` (DEC-254); `tools/ledger.py` regenerates plan.md's ledger
# table from it.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or SPRT-RUN-FAILED on
# every exit path, and this script execs it and adds nothing after.

set -uo pipefail

fail()
{
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

# THE BOOKED SETTINGS, AND NOTHING ELSE FROM THE CALLER'S ENVIRONMENT.
ref="65606a9"
worktree="/home/max/ws/chesso-s122p2"
screened="2bf3ad4"
part=""           # THE PARTS above: empty for the family, or M, A, B or C
margin_to="275"   # the margin the paste moves the row to (part "" and M)
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"   # M: elo0=-5 elo1=0
sprt_rounds="20950"

# THE COORDINATOR'S SENTENCE AND SHAS. While the bracketed marker is anywhere in this file
# the pre-registration is unfinished and the run does not start. The marker is assembled in
# two halves so this line does not match itself.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); pin REF and fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

for pair in "REF=$ref" "TC=$tc" "HASH=$hash" \
            "CONCURRENCY=$concurrency" "SPRT_BOUNDS=$sprt_bounds" \
            "SPRT_ROUNDS=$sprt_rounds"; do
  name="${pair%%=*}"
  want="${pair#*=}"
  if [[ -n "${!name+set}" && "${!name}" != "$want" ]]; then
    fail "$name is '${!name}' in the environment, and this run is booked at" \
         "$name=$want; unset it"
  fi
done
for name in CAND ROUNDS BARE_FEN SRAND AA; do
  if [[ -n "${!name+set}" ]]; then
    fail "$name is set in the environment ('${!name}'), and this run is" \
         "booked without it; unset it"
  fi
done

cd "$worktree" || fail "cd $worktree"

if [[ "$(git rev-parse HEAD)" != "$(git rev-parse "$ref")" ]]; then
  fail "$worktree's HEAD is $(git rev-parse --short HEAD), not REF" \
       "$(git rev-parse --short "$ref") -- the pair would measure more than S122"
fi

[[ -n "$(git status --porcelain)" ]] \
  || fail "$worktree is clean: there is no candidate change to play"

build_paths=(src CMakeLists.txt cmake)

# THE PARTS: what this run may move, and what REF may carry past `screened`.
part_a="ks_checks"
part_b="ks_weak|ks_storm"
part_c="ks_curve"
margin_row='^[[:space:]]*X\(LAZY_EVAL_MARGIN,[[:space:]]+"LazyEvalMargin",[[:space:]]+[0-9]+,[[:space:]]+0,[[:space:]]+2000\)[[:space:]]*\\$'
case "$part" in
  "") moves="$part_a|$part_b|$part_c"; moves_margin=1; carries=""; carries_margin=0 ;;
  M)  moves="";                       moves_margin=1; carries=""; carries_margin=0 ;;
  A)  moves="$part_a";                moves_margin=0; carries=""; carries_margin=1 ;;
  B)  moves="$part_b";                moves_margin=0; carries="$part_a"; carries_margin=1 ;;
  C)  moves="$part_c";                moves_margin=0; carries="$part_a|$part_b"; carries_margin=1 ;;
  *)  fail "part is '$part', not empty, M, A, B or C" ;;
esac
[[ "$part" == M && "$sprt_bounds" != "elo0=-5 elo1=0 alpha=0.05 beta=0.05" ]] &&
  fail "part M is booked at {-5, 0}: re-pin sprt_bounds"
[[ "$part" != M && "$sprt_bounds" != "elo0=0 elo1=5 alpha=0.05 beta=0.05" ]] &&
  fail "part '$part' is booked at {0, 5}: re-pin sprt_bounds"

# Whether a diff differs under src/, outside whole-line comments, only in whole definitions
# of ARRAYS, each read to its closing `};` across the lines clang-format wrapped it over and
# nothing after it, and, with MARGIN 1, in the LAZY_EVAL_MARGIN row. A continuation line is
# joined to the line before it while that line has not closed its brace; a definition that
# never closes, or code appended after `};`, fails the form. A diff of comments alone leaves
# nothing to read, which fails: it moved no weight.
#   only_definitions ARRAYS MARGIN REVISION...
only_definitions()
{
  local arrays="$1" with_margin="$2" lines form joined
  shift 2
  lines="$(git diff -U0 "$@" -- src |
           grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' |
           grep -vE '^[-+][[:space:]]*(//.*)?$')"
  [[ -n "$lines" ]] || return 1
  joined="$(awk '
    { sign = substr($0, 1, 1); body = substr($0, 2) }
    open && sign == opened { sub(/^[[:space:]]+/, " ", body); held = held body
                             if (held ~ /\};[[:space:]]*$/) { print held; open = 0 }; next }
    open { print held; open = 0 }
    body ~ /^constexpr int [a-z_]+\[[0-9]+\] = \{/ && body !~ /\};[[:space:]]*$/ {
      held = body; opened = sign; open = 1; next }
    { print body }
    END { if (open) print held }' <<< "$lines")"
  form='^constexpr int ('"${arrays:-NONE}"')_(mg|eg)\[[0-9]+\] = \{[[:space:]]*-?[0-9]+([[:space:]]*,[[:space:]]*-?[0-9]+)*[[:space:]]*,?[[:space:]]*\};[[:space:]]*$'
  while IFS= read -r line; do
    [[ "$line" =~ $form ]] && continue
    (( with_margin )) && [[ "$line" =~ $margin_row ]] && continue
    return 1
  done <<< "$joined"
}

# `screened`: the family at its seed, read at that commit.
python3 adocs/data/S122_screen.py landing-check --tree . --commit "$screened" ||
  fail "the commit named as the one the screen ran from, $screened, is not the family at its seed"

stamp_file="/home/max/ws/chesso/.tuning/s122/screen/bin/stamp"
[[ -f "$stamp_file" ]] ||
  fail "no $stamp_file, so nothing records the commit the screen ran from"
stamped="$(git rev-parse --verify --quiet "$(cat "$stamp_file")^{commit}")" ||
  fail "$stamp_file holds '$(cat "$stamp_file")', which names no commit"
if [[ "$(git rev-parse --verify --quiet "$screened^{commit}")" != "$stamped" ]]; then
  fail "screened $screened is not $stamped, the commit $stamp_file names"
fi

# `ref`: it descends from `screened`, its engine `screened`'s; a later part's REF may also
# carry what has landed by then, and nothing else.
if ! git merge-base --is-ancestor "$screened" "$ref"; then
  fail "REF does not descend from the screened commit $screened"
fi
ref_changed="$(git diff --name-only "$screened" "$ref" -- "${build_paths[@]}")"
if [[ -n "$ref_changed" ]]; then
  [[ -n "$carries" || "$carries_margin" == 1 ]] ||
    fail "REF's src/ or build files differ from the screened commit's -- REF is not the" \
         "family at its seed"
  while IFS= read -r file; do
    case "$file" in
      src/evaluation.cpp|src/evaluation.hpp|src/search_params.hpp) ;;
      *) fail "REF changes $file beyond what part $part may have landed" ;;
    esac
  done <<< "$ref_changed"
  only_definitions "$carries" "$carries_margin" "$screened" "$ref" ||
    fail "REF's src/ differs from the screened commit's outside comments in more than what" \
         "part $part may have landed"
fi

# ONLY THE STAGED PASTE IS PLAYED.
git diff --quiet -- src tests cmake CMakeLists.txt \
  || fail "unstaged changes under src/, tests/, cmake/ or CMakeLists.txt:" \
          "$(git diff --name-only -- src tests cmake CMakeLists.txt |
             head -5 | tr '\n' ' ')"
untracked="$(git ls-files --others --exclude-standard -- src cmake | head -5)"
[[ -z "$untracked" ]] || fail "untracked files under src/ or cmake/: $untracked"

git diff --quiet "$ref" -- cmake CMakeLists.txt \
  || fail "cmake/ or CMakeLists.txt differs between the tree and REF:" \
          "$(git diff --name-only "$ref" -- cmake CMakeLists.txt |
             head -5 | tr '\n' ' ')"

# THE PASTE IS THE FAMILY'S WEIGHTS AND THE MARGIN, in three files at most, comments aside.
changed="$(git diff --name-only "$ref" -- src)"
[[ -n "$changed" ]] || fail "the tree's src/ is REF's -- not S122's paste"
while IFS= read -r file; do
  case "$file" in
    src/evaluation.cpp|src/evaluation.hpp|src/search_params.hpp) ;;
    *) fail "the tree changes $file under src/ beyond the paste" ;;
  esac
done <<< "$changed"

only_definitions "$moves" "$moves_margin" "$ref" ||
  fail "the tree's src/ differs from REF's outside comments in more than whole definitions" \
       "of the arrays this run moves (${moves:-none}) and the margin row"

# The margin row, read: the paste moves it to `margin_to`, a part A to C leaves it REF's.
here="$(sed -n -E 's/.*X\(LAZY_EVAL_MARGIN, +"LazyEvalMargin", +([0-9]+),.*/\1/p' src/search_params.hpp)"
there="$(git show "$ref:src/search_params.hpp" |
         sed -n -E 's/.*X\(LAZY_EVAL_MARGIN, +"LazyEvalMargin", +([0-9]+),.*/\1/p')"
if (( moves_margin )); then
  [[ "$here" == "$margin_to" && "$there" != "$margin_to" ]] ||
    fail "the margin row reads $here against REF's $there; part '$part' moves it to $margin_to"
else
  [[ "$here" == "$there" ]] || fail "part $part moves the margin row, $there -> $here"
fi

# Every weight array and define outside the family's arrays is REF's in the tree, compared
# by name; without --allow-five, so king safety's linear five are REF's too (DEC-353).
python3 adocs/data/S122_screen.py only-family . ||
  fail "the tree moves a weight outside the family's arrays against REF"

# BUILD/ WAS BUILT FROM THIS TREE.
binary="$worktree/build/src/chesso"
cmake --build "$worktree/build" -j "$concurrency" --target chesso > /dev/null \
  || fail "cmake --build $worktree/build --target chesso failed"
[[ -x "$binary" ]] || fail "no engine at $binary after the build"
newer="$(find "$worktree/src" "$worktree/cmake" "$worktree/CMakeLists.txt" \
           -newer "$binary" -type f 2> /dev/null | head -5)"
[[ -z "$newer" ]] || fail "sources newer than $binary: $newer"

[[ -s books/noob_3moves.epd ]] \
  || fail "no books/noob_3moves.epd in $worktree; copy the main checkout's"

[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh"

for name in SPRT_BOUNDS SPRT_ROUNDS; do
  if ! grep -qE '\$\{?'"$name"'([^A-Za-z0-9_]|$)' ./fastchess.sh; then
    fail "./fastchess.sh does not read $name, so this run's bounds or cap would be" \
         "ignored -- see THE HARNESS in adocs/data/S283_sprt.sh"
  fi
done

shopt -s execfail
exec env -u CAND -u ROUNDS -u BARE_FEN -u SRAND -u AA -u RELAY_PYTHON \
     REF="$ref" \
     TC="$tc" \
     HASH="$hash" \
     CONCURRENCY="$concurrency" \
     SPRT_BOUNDS="$sprt_bounds" \
     SPRT_ROUNDS="$sprt_rounds" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s122_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
