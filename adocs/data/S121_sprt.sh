#!/usr/bin/env bash
#
# S121: MOBILITY -- the table per piece type fitted by S121's screen over the
# count its surviving parts define, against the phase 1 landing that ships the
# table at identity: one SPRT at the harness's 8+0.08, Hash 16, {0, 5} nElo
# (DEC-259 (b)), bisected along DEC-326 (8)'s partition on H0 or no verdict.
#
#   nohup adocs/data/S121_sprt.sh > .tuning/s121_sprt.log 2>&1 &
#
# Written 2026-10-28 in S121's phase 1a, blind, in adocs/data/S136_sprt.sh's
# and S135_sprt.sh's shape, whose harness section -- S283_sprt.sh's before
# them -- this file relies on and does not repeat. Nothing here was run. The
# pair does not exist yet; the coordinator pins it in the bracketed lines
# below, and until then the script refuses to start (the self-check at the
# bottom).
#
# THE PAIR.
#
#   CAND  S121's candidate commit: `adocs/data/S121_screen.sh apply` pasted
#         the screen's candidate -- the last survivor's full-corpus fit,
#         `--only mobility` with its ties on S083's corpus (6226944 rows,
#         sha256 2e0e3de9..., S121_screen.sh's CORPUS_SHA) against S082's
#         fixed validation set -- over the eight mobility tables in
#         src/evaluation.cpp, and, when a part survived, applied
#         adocs/data/S121_parts.patch with `#define S121_PARTS <mask>` in
#         src/evaluation.hpp; every golden the paste moved re-derived by its
#         own script (DEC-142); so the commit is green.
#   REF   CAND's parent: S121's phase 1 landing, the table at identity, which
#         scores every position as the linear term did, or a documents-only
#         descendant of it.
#
# The two differ by the fitted tables and, with a mask, the parts' code,
# which the guards below check: CAND^ is REF; REF's src/ is the phase 1
# landing's byte for byte; src/eval_tables.hpp is the same at both; and CAND's
# src/ is REF's with the parts patch applied when the mask is not 0, differing
# from that outside comments only in the eight table definitions and the one
# S121_PARTS line.
#
# WHAT THE VERDICT PRICES. The fitted table, its parts' count and their cost
# together: every evaluation that reaches stage two pays the area and the pins
# the mask names. The cost is recorded apart, on identical trees, by
# adocs/data/S121_cost.sh (DEC-047's method), and printed beside the verdict;
# it decides nothing.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's
# control for every verdict but the correction family's, and this is an
# evaluation family. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names
# it for a family's survivors. What is known of the size, deciding nothing
# (DEC-019): Ethereal 7.78's release with the mobility area and per-count
# tables, +22.2 to +42.5 for a bundle; Stash v27's zone excluding rammed and
# low-rank pawns, +19.95 +/- 9.63 at 10+0.1; Stash v30's pins, +6.15 (the
# step file's section 1 and its DEC-258 amendment). Rejected: `{-5, 5}` --
# would ship a zero half the time (DEC-304's reason); `--fast` and `{0, 10}`
# -- DEC-063.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic
# (`adocs/testing_strategy.md` section 1.1): **41861 games** with the truth at
# the interval's midpoint, 19.84 h at the 2110 games an hour `.moltke.local.md`
# budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** on a
# bound, 12.13 h. S135's run of the same pair and control played 13406 games
# in 6 h 17 m, 2133 an hour, to H1.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour, S283's,
# S135's and S136's rule. Four hours or more, so the run starts by DEC-155's
# night rule. Watcher ceiling 144000 s (40 h), wall-clock only while the
# machine is awake.
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
#   out        /home/max/ws/chesso/.tuning/sprt_s121_<stamp>
#
# with no `diff`, `relay` or `A/A CALIBRATION` line, and fastchess's first
# `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# [COORDINATOR, on the day it pins: CAND's and REF's shas into `ref=` and
# `cand=` below, the phase 1 landing's sha into `landing=`, the candidate's
# parts mask into `mask=`, their dates into the banner lines above, the
# screen's five readings and their sha256 (each part's gain, s, m and
# verdict), the corpus `locate` recorded, the candidate fit's sha256 and its
# tie file's, the clamp's two readings (share against the 1.0 % bound, the
# mobility p99 and maximum), the cost run's reading, plan.md's Open list
# re-read, the A/A rule checked, and this bracketed sentence replaced by what
# was found.]
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted family is accepted as gaining: H1's
#                   5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It
#                   ships.** The closing commit folds S121_PARTS -- a
#                   surviving part's code made unconditional, the dropped
#                   parts' deleted, the macro and its asserts gone, the walk
#                   in tests/test_walked_counts.hpp and the counts in
#                   adocs/data/S192_anchors.py folded the same way -- and
#                   proves the fold behaviour-neutral against CAND (INV-6:
#                   bench and `tools/search_bench.py` node counts and best
#                   moves at depths 9 and 12 identical); it makes the
#                   comments true and specs.md's evaluation rows. The stopping
#                   run's Elo is upward-biased (DEC-063) and is not the effect
#                   size.
#
#   H0           -> not shown to gain. **The paste is reverted whole** -- the
#                   tables back to identity, the parts patch off, the goldens
#                   back -- and the family is bisected along DEC-326 (8)'s
#                   partition, survivors only, in order: A the curve (P0's
#                   full fit against the phase 1 landing), B1 the enemy-pawn
#                   exclusion, B2a the area in place of ~own with blocked
#                   pawns excluded, B2b low-rank pawns excluded as well
#                   (always inside the area), C the pin rule, each its own
#                   `{0, 5}` SPRT in this file's shape -- at worst five more
#                   runs after this one, six in all, each up to 19.86 h. B1,
#                   B2a, B2b and C play their count over what has landed by
#                   then: the
#                   table refitted when A has landed, the linear model refitted
#                   over the new count when it has not
#                   (`adocs/data/S121_screen.sh part NAME LANDED CURVE`). A
#                   part's H1 lands before the next is fitted; a part's H0 or
#                   no verdict is that part's zero.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   reverted whole as H0 is, recorded as **unresolved**, and
#                   bisected as H0 is (DEC-259 (c)).
#
# THE CLOSING COMMIT carries DEC-220's block -- `SPRT | cand <CAND> vs ref
# <REF>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`,
# `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S121_sprt.log` by `tools/trim_console.py`; `tools/ledger.py`
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
mask="[COORDINATOR: the candidate's parts mask, 0 to 15, bit 8 only with 2]"
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

[[ "$mask" =~ ^([0-9]|1[0-5])$ ]] && { (( (mask & 8) == 0 || (mask & 2) != 0 )); } ||
  fail "mask '$mask' is not a parts mask, 0 to 15 with bit 8 only beside 2"

cd /home/max/ws/chesso || fail "cd"

# THE PAIR: REF is CAND's parent, so the run measures S121's paste and nothing
# else.
if [[ "$(git rev-parse "$cand^")" != "$(git rev-parse "$ref")" ]]; then
  fail "REF is not CAND's parent -- the pair would measure more than S121"
fi

# REF's engine is the phase 1 landing's: the table at identity.
if ! git diff --quiet "$landing" "$ref" -- src/; then
  fail "REF's src/ differs from the phase 1 landing's -- REF is not the" \
       "table at identity or a documents-only descendant of it"
fi

if ! git diff --quiet "$ref" "$cand" -- src/eval_tables.hpp; then
  fail "src/eval_tables.hpp differs between REF and CAND -- not a" \
       "mobility-only paste"
fi

# What CAND's src/ must be, but for the tables and the mask line: REF's, with
# the parts patch applied when the mask is not 0, written from a scratch
# index so neither the working tree nor its index is touched.
expected="$ref"
if [[ "$mask" != 0 ]]; then
  scratch=$(mktemp -d) || fail "mktemp"
  GIT_INDEX_FILE="$scratch/index" git read-tree "$ref" &&
    git show "$cand:adocs/data/S121_parts.patch" > "$scratch/parts.patch" &&
    GIT_INDEX_FILE="$scratch/index" git apply --cached "$scratch/parts.patch" &&
    expected=$(GIT_INDEX_FILE="$scratch/index" git write-tree)
  status=$?
  rm -rf "$scratch"
  (( status == 0 )) || fail "the parts patch does not apply to REF"
fi

changed="$(git diff --name-only "$expected" "$cand" -- src)"
allowed="src/evaluation.cpp"
[[ "$mask" != 0 ]] && allowed=$'src/evaluation.cpp\nsrc/evaluation.hpp'
if [[ -z "$changed" ]]; then
  fail "CAND's src/ is REF's (with the patch) -- not S121's paste"
fi
while IFS= read -r file; do
  grep -qxF -- "$file" <<< "$allowed" ||
    fail "CAND changes $file beyond the paste and the parts patch"
done <<< "$changed"

code_lines="$(git diff -U0 "$expected" "$cand" -- src |
              grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' | cut -c2- |
              grep -vE '^[[:space:]]*(//.*)?$')"
table='^constexpr int mobility_(knight|bishop|rook|queen)_(mg|eg)\[[0-9]+\] = \{'
values='^[[:space:]]*-?[0-9]+(,[[:space:]]*-?[0-9]+)*,?[[:space:]]*(\};)?$'
parts_line='^#define S121_PARTS ([0-9]|1[0-5])$'
if grep -qvE "($table)|($values)|($parts_line)" <<< "$code_lines"; then
  fail "CAND's src/ differs outside comments in more than the mobility" \
       "tables and the S121_PARTS line"
fi
if [[ "$mask" != 0 ]] &&
   ! git show "$cand:src/evaluation.hpp" | grep -qx "#define S121_PARTS $mask"; then
  fail "CAND's src/evaluation.hpp does not set S121_PARTS to $mask"
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s121_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
