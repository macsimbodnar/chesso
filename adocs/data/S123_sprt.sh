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
# 2026-10-30, S123's phase 2, re-read against the candidate before it is
# pinned (the step file's "2026-10-30: the screen and the candidate"). The
# screen kept all three groups: `CANDIDATE ABC groups
# passed_advance,passed_kings,candidate_passers`, the fit
# .tuning/s123/fits/ABC/full.hpp (sha256 c7f340c6...), pasted by
# `S123_screen.sh apply` over the ten family arrays, bench 6016079 -> 5434393.
# The guards below hold as written, read on the worktree against its HEAD
# `1a231dd`: src/evaluation.cpp is the only file under src/, CMakeLists.txt
# and cmake/ that differs;
# outside whole-line comments its 20 changed lines are the ten family
# definitions, old and new, nothing outside the allowed forms; and every
# weight array and define outside the ten reads HEAD's by name
# (`S123_screen.py only-family`, the working-tree form of only-family-pair).
# REF `1a231dd` with `landing=` `03680ab` passes the landing check: their
# src/ is identical, and CAND's parent will be `1a231dd`. The bisection the
# screen printed is A | B | C, so the H0 paragraph's "at worst three more
# runs after this one, four in all" holds as written; each part is its group
# refitted alone over what has landed by then (`part NAME`, `--only` its
# group, the landed parts held in the sources), so B and C under H0 are not
# the screen's B and C, which were fitted with the earlier parts free. The
# verdict pays all's cost: every group is live at the candidate, and
# adocs/data/S123_cost.sh's `all` row reads +4.96 % (a +2.59 %, b +1.52 %, c
# +1.48 %, parent zero; adocs/data/S123_cost_reading.txt). There is no clamp
# reading: the family is stage one.
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
#         expression (adocs/data/S123_inv6.txt), or a descendant of it whose
#         src/ is the landing's byte for byte -- `1a231dd`, the cost script's
#         fix and the step file over `03680ab`, `No functional change`, is
#         one.
#
# The two differ by the fitted family arrays and nothing else in src/ or the
# build's flags, which the guards below check: CAND^ is REF; REF's src/, its
# top-level CMakeLists.txt and cmake/ are the phase 1 landing's byte for byte
# (src/CMakeLists.txt is under src/; the engine target reads no other build
# file); and CAND's are REF's but for src/evaluation.cpp, which differs
# outside whole-line comments only in whole one-line definitions of the ten
# family arrays (passed_free, passed_safe, passed_king_own, passed_king_enemy
# and candidate_passer, mg and eg; nothing after the closing `};`) or
# numbers-only lines, and in which every other weight array and define reads
# REF's value by name. The build-file and end-of-line clauses were added at
# phase 2's fast check (2026-10-30): before them a flag change in
# CMakeLists.txt or cmake/, or code appended after a family line's `};`,
# passed. A dropped group's arrays stay zero and are in
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
#   candidate  0553160  2026-10-30 06:14
#   reference  1a231dd  2026-10-30 03:29
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
# Pinned by the coordinator 2026-10-30 06:20: CAND `0553160` (committed
# 2026-10-30 06:14, the screen's ABC fit pasted over the ten family arrays,
# bench 5434393) and REF `1a231dd` (committed 2026-10-30 03:29, the cost
# script's fix, `src/` and the build files byte for byte the landing's,
# bench 6016079), CAND's parent; `landing=` is `03680ab`, the phase 1
# landing (committed 2026-10-30 02:31, the family at zero). The screen's three
# readings (`.tuning/s123/read/`, concatenated in
# `adocs/data/S123_screen_reading.txt`, sha256 04309775...): A g +0.000070
# s 0.000005 m 0.000009 kept (A.txt fa084279...); B d +0.000109 against A,
# s 0.000008 m 0.000013 kept (B.txt dd2d0d3d...); C d +0.000018 against AB,
# s 0.000006 m 0.000010 kept (C.txt 6ba292fd...); survivors A, B, C, the H0
# order A | B | C. The corpus `locate` recorded: S083's curve 0, 6226944
# rows, sha256 2e0e3de9... The candidate fit `.tuning/s123/fits/ABC/full.hpp`,
# sha256 c7f340c6...; the tie files, none tied, ties_A.txt a3b4ae44...,
# ties_B.txt d5f0ad16..., ties_C.txt d9a04245.... The cost
# (`adocs/data/S123_cost_reading.txt`, a2652990...): a +2.59 %, b +1.52 %,
# c +1.48 %, all +4.96 %, parent zero. The Open list at `0553160` re-read:
# no DEC-171 filler is open. `MATE_IN_THREE_FLOOR` holds at 11 at zero
# margin (DEC-330). The harness is S121's run's of 2026-10-29 -- fastchess
# alpha 1.8.1 20260720-daa3ea2, `noob_3moves.epd`, `fastchess.sh` last
# changed `769ce7a` (S284), the same workstation and kernel 7.2.6 -- so no
# A/A is owed (DEC-143). Before the run: the commit gate green at CAND (both
# fast suites 50 of 50, format and the prose checks clean), `gate_extra` 6
# stages green on CAND's tree, Debug self-play 8 games, 0 `Assertion`, 0
# `disconnect`, and a dry copy of this script printed DRY-RUN-WOULD-EXEC.
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
ref="1a231dd"
cand="0553160"
landing="03680ab"
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

# The files the engine's build reads: src/ (src/CMakeLists.txt with it), the
# top-level CMakeLists.txt and cmake/'s flag files. tests/ and tools/ build no
# part of the chesso target fastchess.sh plays.
build_paths=(src CMakeLists.txt cmake)

# REF's engine is the phase 1 landing's: the family at zero.
if ! git diff --quiet "$landing" "$ref" -- "${build_paths[@]}"; then
  fail "REF's src/ or build files differ from the phase 1 landing's -- REF is" \
       "not the family at zero or a descendant of it with the landing's src/"
fi

# CAND's src/ and build files are REF's but for src/evaluation.cpp.
changed="$(git diff --name-only "$ref" "$cand" -- "${build_paths[@]}")"
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
table='^constexpr int (passed_free|passed_safe|passed_king_own|passed_king_enemy|candidate_passer)_(mg|eg)\[[0-9]+\] = \{[[:space:]]*-?[0-9]+([[:space:]]*,[[:space:]]*-?[0-9]+)*[[:space:]]*,?[[:space:]]*\};[[:space:]]*$'
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
