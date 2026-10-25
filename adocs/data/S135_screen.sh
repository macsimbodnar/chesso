#!/usr/bin/env bash
# S135's screen: each of the piece placement group's five terms fitted alone
# with every other constant frozen, kept only past a margin stated here, then
# the survivors fitted together -- the candidate S135's one SPRT measures
# (adocs/data/S135_sprt.sh). DEC-259 (a); DEC-091's bundle, DEC-258's two
# added features.
#
#   adocs/data/S135_screen.sh locate                 # the corpus, its sha256
#   adocs/data/S135_screen.sh audit                  # the identity report on it
#   adocs/data/S135_screen.sh screen                 # the 30 term fits; resumable
#   adocs/data/S135_screen.sh read                   # the rule, over the fits on disk
#   adocs/data/S135_screen.sh candidate              # the survivors together
#   adocs/data/S135_screen.sh part NAME              # a bisection part's fit, on H0
#   adocs/data/S135_screen.sh apply FIT TREE         # the fit into TREE, built and gated
#
# Run from the main checkout with S135 landed (weights zero), so build/'s
# tuner carries the seven placement columns and the `piece_placement.<term>`
# names; the screen refuses a tuner that does not. Each stage prints
# S135-<STAGE>-DONE or S135-<STAGE>-FAILED as its last stdout line, the reasons
# above a FAILED, so each runs detached under a four-exit watcher (WATCHERS).
# OUT defaults to .tuning/s135 (DEC-235: a run's outputs stay with the run;
# the readings go to adocs/data/ and the step file).
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-22 in S135's phase 1, blind (DEC-260:
# S083's corpus stages held the machine), before any of it ran. Nothing below
# the line changes once `screen` has started; a change is a dated section in
# the step file and a decision.
# ============================================================================
#
# THE CORPUS, AND HOW THIS FINDS IT. Settled at S135's rebase, 2026-10-25:
# S083's SPRT read H1 (adocs/data/S083_sprt.log, landed 01222f3, DEC-313), so
# its constants ship and the corpus is the one its candidate was fitted on.
# Curve 0 stopped (READ-OUTCOME stop in .tuning/s083/curve/s0/reading.txt), its
# candidate fit_6226944_s1.hpp, drawn on cand_6226944_s1.tsv beside it: every
# game of S082's training side at 46 rows a game, 6226944 rows from 137978
# games, sha256 2e0e3de9... as curve 0's samples.log recorded it, pinned below
# as CORPUS_SHA. `locate` finds it mechanically (S135_screen.py locate
# --source s083): the last $S083_OUT/curve/sN/reading.txt that printed
# READ-OUTCOME stop, its `candidate fit_<top>_s1.hpp`, and cand_<top>_s1.tsv
# beside it, hashed against that stage's samples.log and then against
# CORPUS_SHA, either mismatch refusing; a file S083 has since removed is drawn
# again from the pool its curve.log names, same seed, rows and density, and
# must come out at that sha256. SOURCE, which phase 1 left for the coordinator
# to state on S083's verdict, is s083 by that verdict and nothing else is
# accepted. The fixed validation set is S082's qs view, its three views'
# sha256 checked before every stage.
#
# THE IDENTITY CHECK FIRST (S134's lesson: a column that is a sum of table
# columns is no quantity, and its fit is a direction, not a weight). `audit`
# runs tools/feature_audit over the located corpus and passes only when it
# prints `rank over the rationals exactly 792 of 831`, `no exact dependency
# beyond the ones listed above` and `every null direction is one Adam never
# moves or one the tuner holds` -- S134's 39 null directions and no more, the
# four new columns adding rank 8. Anything else stops the screen for the
# coordinator: a new exact dependency is a finding before any fit. The
# collinearity R^2 of the four new columns is printed beside, deciding
# nothing.
#
# THE TERMS. tools/tuner_groups.hpp PLACEMENT_TERMS, five of them over seven
# columns: bishop_pair, rook_open, rook_half_open (S027's, frozen at zero
# since S065, DEC-057), bad_bishop (the bad bishop count and its rammed share,
# two columns, one term) and trapped (the trapped minor and trapped rook
# counts, two columns, one term). A two-column term is screened whole because
# its columns were defined together from one description and the bisection
# below splits at terms; the step file says why each pair is two columns.
#
# THE FIT, S082's compare's and DEC-304's except the free set: `tuner --data
# CORPUS --holdout .tuning/s082/pool.tsv.qs.validation --only
# piece_placement.<term> --epochs 5000 --report 100 --patience 20 --lambda 0
# --seed 1 --threads 12`, K fitted from each corpus's own rows, every fit
# starting from the constants that ship -- which hold every placement weight
# at zero. One frozen copy of the tuner for the whole screen.
#
# THE NOISE. Five samples of the corpus at a quarter of its rows, Q = C / 4,
# `corpus_split.py sample --in CORPUS --rows Q --per-game 0 --seed s`, s = 1 to
# 5, the three views named: a quarter of the corpus's games, every row of each,
# a different quarter per seed. Each term is fitted on each, the same command.
#
# THE READING, by S135_screen.py read, mechanically:
#   1. A fit's gain g is its `best:` line's validation figure subtracted from
#      its own start figure -- the shipped constants on the same fixed rows
#      with the same K, one figure for every fit of the screen (rule 2), so
#      the start adds no training-resampling noise to g. `best:` is the
#      minimum over the fit's reports on that same fixed set, so g >= 0 and
#      is selected on the set that scores it (THE MARGIN says what that
#      leaves unpriced).
#   2. The five full-corpus fits must print one start figure and one K, the
#      frozen baseline; if they do not, the screen is unreadable.
#   3. s_t is the sample standard deviation (n - 1) of term t's five quarter
#      gains; m_t = max(1.645 * s_t, 2e-6).
#   4. Term t is kept when its full-corpus gain g_t > m_t, and dropped
#      otherwise, recorded with its figures; a dropped term plays no match.
#   5. Unreadable, nothing chosen: a fit missing or refused, rule 2 broken, or
#      some s_t above 1e-3.
#   Printed beside, deciding nothing: each term's fitted weights alone, the
#   step file's two sanity bands (bishop pair 47; rook open 0 to 47, midpoint
#   24; never a target), the survivors' bisection order.
#
# THE MARGIN, WHAT IT PRICES AND WHAT IT DOES NOT (corrected 2026-10-25 by the
# phase 1b fast check; the rule is unchanged). 1.645 is DEC-304's one-sided
# 95 % point. **What s measures: training-corpus resampling alone.** All six
# fits of a term -- the full corpus and the five quarters -- are scored on one
# fixed validation set and start from one figure, so s is the spread of the
# gain as the training rows change and nothing else. DEC-304's sqrt(2) priced
# a difference of two fits on two resampled corpora; here g is one resampled
# fit against a start figure that does not move with the training rows, so no
# sqrt(2) is owed for that noise. s is taken at a quarter of the rows, the
# size at which five draws are affordable; if the resampling spread falls as
# one over the square root of the rows it is about twice the full corpus's --
# an assumption, not a measurement. **What is not priced**: the fixed set's
# own sampling noise, and the selection of `best:` as the minimum over up to
# 50 reports on that same set. Both are common to all six fits of a term, so
# they are not in s, and the selection biases g upward. No claim is made that
# the margin errs in either direction on balance. **Why that is accepted**:
# the screen only removes terms and never ships one -- the one SPRT decides
# what ships (DEC-259) -- and each fit frees few parameters, 2 for
# bishop_pair, rook_open and rook_half_open and 4 for bad_bishop and trapped
# (mg and eg per column; the tuner's own `N free`), against a validation set
# of N = 1015332 rows from 12000 games (S082's fixed qs view, sha256
# 2b508067..., S082's step file and the tuner's `held out:` line), so the
# bias of picking the best report along a two- or four-parameter path is
# taken as small; it is not measured. The floor of 2e-6 is two
# units of the sixth decimal the tuner prints: each figure carries 5e-7 of
# rounding, a gain 1e-6, and a zero spread from five identical prints must not
# pass a gain the printout cannot resolve. s from five draws is loose -- a
# 90 % interval of 0.65 s to 2.37 s, chi-square on 4 degrees of freedom -- as
# S082 and DEC-304 said of theirs.
#
# THE TUNER IS HEAD'S, CHECKED, NOT ASSUMED. A tuner starts from the constants
# compiled into it, so one built before HEAD -- before S083's paste lands, or
# before a bisection part does -- would fit from other constants and emit a
# landed part's columns at their old values. Each frozen copy is first run for
# zero epochs on the corpus's first 2000 rows and its emitted header compared,
# value by value, with HEAD's sources (S135_screen.py start-check); any
# difference refuses the stage. Every fit's header is checked the same way
# after it, its own free columns excepted.
#
# THE STAGE-ONE BOUND, BEFORE ANY PASTE. src/evaluation.cpp's stage_one_bound
# static_assert fails a build whose weights could overflow a packed half. Its
# arithmetic is repeated by S135_screen.py headroom for every fit: `read`
# prints each term's, `candidate` and `part` refuse a fit that passes 32767 in
# either half, and `apply` refuses before it pastes. At the tables S135 was
# written on (41e4140) the bound was 19350 in the middlegame and 10714 in the
# endgame; on S083's tables, read at the rebase by `headroom` and by a
# static_assert on each half, it is 12565 and 12165, 20202 and 20602 of room.
# If a fit legitimately passes it, that is not clipped and not pasted: the
# step pauses for the coordinator, the first answer being a tighter valid
# bound (behaviour-neutral, the static_assert's input alone), the second the
# owner's -- the unpacked sum for stage one, a speed change to time -- and the
# weights are never cut by hand to fit (DEC-084: no constant ships unfitted).
#
# THE CANDIDATE: the survivors fitted together on the full corpus, `--only
# piece_placement --freeze piece_placement.<each dropped term>` (no --freeze
# when all five survive), the same settings. It is what S135_sprt.sh measures.
# A candidate fit that does not improve on the shipped constants is
# unreadable, not a verdict. No survivor: no candidate, no SPRT, the five
# zeros recorded with their figures, the seven columns kept at zero (the
# compiler deletes them, DEC-047).
#
# THE BISECTION, ON H0 OR NO VERDICT (DEC-259 (c), DEC-082, DEC-091), written
# now. The family splits into A, S027's three (bishop_pair, rook_open,
# rook_half_open), and B, DEC-258's two (bad_bishop, trapped); A into A1
# (bishop_pair) and A2 (rook_open, rook_half_open), A2 into its two; B into B1
# (bad_bishop) and B2 (trapped). Applied to the survivors: an empty half is
# dropped, a half holding all of its part's survivors is skipped to its own
# halves, and a single term is a leaf. Each part is refitted alone, `part
# NAME`, every other placement term frozen at what ships -- zero, or a part
# already landed -- and measured by its own SPRT in S135_sprt.sh's shape
# against the tree as it stands then; parts run in the order A, A's halves, B,
# B's halves. A part's H1 lands; its H0 or no verdict splits it again; a
# leaf's is the term's zero. `read` prints the order for the survivors it
# finds.
#
# MACHINE HOURS, from measured throughput only (re-read at the rebase). A fit
# runs to its best report plus 20 reports of patience, 2000 epochs, capped at
# 5000; S083's eight curve-0 fits ran 2500 to 5000 epochs, median 4300, at
# 0.0236 to 0.0252 s an epoch per million rows with their loads -- the top
# rung's 692 s over 4400 epochs on 6226944 rows the slowest (sample written
# 10:56:37, header 11:08:09, 2026-10-22, .tuning/s083/curve/s0). Taken at
# 0.0252, the same for a fit freeing two columns as for 815: an epoch's cost
# is the rows' trace, which the free set does not shorten. A full fit is then
# 5.5 / 11.2 / 13.1 min at 2100 (best at the first report) / 4300 / 5000
# epochs, a quarter fit 1.4 / 2.8 / 3.3 min. The screen, five full fits and
# 25 quarters with five samples of seconds each: 1 h 02 m / 2 h 07 m /
# 2 h 27 m, ceiling 5 h. The candidate and each part one full fit, ceiling 30
# min. `locate` under a minute (hashing 0.49 GB; a re-draw about 30 s per 0.92
# GB of pool). `audit` read at the rebase: the step file's figure; ceiling 15
# min. Under DEC-155's four hours: the screen is daytime work.
#
# DISK. The five quarter samples, about 124 MB each, 0.62 GB, removable once
# `read` is recorded, each reproducible from its seed and the corpus's
# sha256. A stage refuses to start under 4 GB free.
#
# OPEN FINDINGS (BUGS as scoped by DEC-171): written at launch by the
# coordinator, in the step file, against plan.md's Open list on the day.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s135}"
S083_OUT="${S083_OUT:-$root/.tuning/s083}"
SOURCE="${SOURCE:-s083}"
CORPUS_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
THREADS=12
FREE_GB_FLOOR=4
SEEDS="1 2 3 4 5"
TERMS="bishop_pair rook_open rook_half_open bad_bishop trapped"

S082_OUT="${S082_OUT:-$root/.tuning/s082}"
HOLDOUT="$S082_OUT/pool.tsv.qs.validation"
VIEWS=("$S082_OUT/pool.tsv.qs.validation"
       "$S082_OUT/pool.tsv.root.validation"
       "$S082_OUT/pool.tsv.pv.validation")
VIEW_SHAS=(2b508067fb64693bb4b278a2455fb9d101eaa742975a4b53f4ff56e6fa16981e
           58bcbb315fd82bd332372d0b2f1df7d7370693568bd27fa9ed6f17b8e74652d6
           24c4f671131c4a6f3f945ef0f5bb0d8cf85969b41d2e846e745539ef2d53ffdd)
FIT="--epochs 5000 --report 100 --patience 20 --lambda 0 --seed 1
     --threads $THREADS"

tuner="${TUNER:-$root/build/tools/tuner}"
audit_tool="${FEATURE_AUDIT:-$root/build/tools/feature_audit}"
split="$root/tools/corpus_split.py"
helper="$root/adocs/data/S135_screen.py"

say() { echo "$*"; }

binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

# The engine a stage ran, as S083_corpus.sh names it: HEAD's sha, or HEAD's
# sha plus the first 12 hex of the blob id of the diff under src/ and tools/,
# saved at $1. Refused when an untracked file sits under src/ or tools/.
engine_stamp() {
  local head untracked
  untracked=$(git -C "$root" ls-files --others -- src tools |
                grep -v '/__pycache__/')
  if [[ -n "$untracked" ]]; then
    {
      echo "refused: untracked files under src/ or tools/, which no stamp names:"
      echo "$untracked"
    } >&2
    return 1
  fi
  head=$(git -C "$root" rev-parse --short HEAD)
  if git -C "$root" diff --quiet HEAD -- src tools; then
    echo "$head"
    return
  fi
  git -C "$root" diff --no-ext-diff --no-color --full-index --binary HEAD \
      -- src tools > "$1"
  echo "$head+$(git hash-object --no-filters "$1" | cut -c1-12), the diff at $1"
}

enough_disk() {
  local free_gb
  mkdir -p "$OUT"
  free_gb=$(df --output=avail -BG "$OUT" | tail -n 1 | tr -dc '0-9')
  if (( free_gb < FREE_GB_FLOOR )); then
    say "refused: ${free_gb} GB free under $OUT, under the ${FREE_GB_FLOOR} GB a stage needs"
    return 1
  fi
}

# A frozen copy of a build binary, its sha256 recorded when it is copied and
# checked before every run, so a rebuild of build/ -- tools/gate.sh rebuilds
# it on any commit -- cannot put another tuner under a recorded sha. An
# existing copy is reused, never replaced. Prints the sha.
freeze_binary() {
  local src="$1" copy="$2"
  mkdir -p "$(dirname "$copy")" || return 1
  if [[ ! -f "$copy" ]]; then
    [[ -x "$src" ]] || { echo "no binary at $src"; return 1; }
    cp "$src" "$copy.tmp" && chmod +x "$copy.tmp" || return 1
    sha256sum "$copy.tmp" | cut -d' ' -f1 > "$copy.sha256.tmp" || return 1
    mv "$copy.tmp" "$copy" && mv "$copy.sha256.tmp" "$copy.sha256" || return 1
  fi
  [[ -f "$copy.sha256" ]] || { echo "$copy has no recorded sha256"; return 1; }
  check_copy "$copy" || return 1
  cat "$copy.sha256"
}

check_copy() {
  if [[ "$(sha256sum "$1" | cut -d' ' -f1)" != "$(cat "$1.sha256")" ]]; then
    echo "refused: $1 no longer has the sha256 recorded when it was copied"
    return 1
  fi
}

# The tuner must be S135's: a tuner built before it knows no term and is three
# columns short. Its usage names the terms. The usage is captured before it is
# searched: under pipefail `--help | grep -q` reads 141 whenever grep exits on
# its match before the tuner has written the rest, so the answer would depend
# on the race (S135_cost.sh carries_columns, 2026-10-25).
check_tuner() {
  local usage
  usage=$("$1" --help 2>&1)
  [[ "$usage" == *piece_placement.bad_bishop* ]] || {
    say "refused: $1 does not know the piece placement terms; build the tuner from a tree with S135 landed"
    return 1
  }
}

# A zero-epoch run of the frozen tuner $1 on the corpus $2's first 2000 rows,
# its header compared with HEAD's sources: the tuner starts from HEAD's
# constants, or the stage refuses.
probe_tuner() {
  local copy="$1" corpus="$2" dir="$3"
  head -n 2000 "$corpus" > "$dir/probe.tsv" || return 1
  check_copy "$copy" || return 1
  "$copy" --data "$dir/probe.tsv" --out "$dir/probe.hpp" --epochs 0 \
      --threads "$THREADS" > "$dir/probe.log" 2>&1 ||
    { tail -5 "$dir/probe.log"; say "the probe run of $copy failed"; return 1; }
  python3 "$helper" start-check "$dir/probe.hpp" --tree "$root"
}

check_views() {
  local i
  for i in 0 1 2; do
    if [[ "$(sha256sum "${VIEWS[$i]}" | cut -d' ' -f1)" != "${VIEW_SHAS[$i]}" ]]; then
      say "the fixed validation set's ${VIEWS[$i]} is not the file S082 recorded"
      return 1
    fi
  done
  say "fixed set: the three views' sha256 are S082's"
}

# The corpus `locate` recorded, re-hashed: "PATH ROWS".
corpus_of() {
  local record="$OUT/corpus.txt" path sha rows
  [[ -f "$record" ]] || { say "no $record; run locate first"; return 1; }
  path=$(sed -n 's/^corpus //p' "$record")
  sha=$(sed -n 's/^sha256 //p' "$record")
  rows=$(sed -n 's/^rows //p' "$record")
  [[ -n "$path" && -n "$sha" && -n "$rows" ]] || { say "$record is incomplete"; return 1; }
  [[ "$(sha256sum "$path" | cut -d' ' -f1)" == "$sha" ]] ||
    { say "$path is not the corpus $record names"; return 1; }
  echo "$path $rows"
}

locate() {
  [[ "$SOURCE" == s083 ]] || {
    say "refused: SOURCE is '$SOURCE'; S083 read H1 (DEC-313), so the corpus is its candidate's and only s083 is accepted"
    return 1
  }
  enough_disk || return 1
  check_views || return 1
  if [[ -f "$OUT/corpus.txt" ]]; then
    say "$OUT/corpus.txt exists; a located corpus is never replaced"
    cat "$OUT/corpus.txt"
    corpus_of > /dev/null || return 1
    grep -qx "source $SOURCE" "$OUT/corpus.txt" ||
      { say "refused: it was located under another SOURCE"; return 1; }
    return 0
  fi
  rm -f "$OUT/corpus.txt.tmp"
  python3 "$helper" locate --source "$SOURCE" --s083-out "$S083_OUT" \
      --s082-out "$S082_OUT" --out "$OUT/corpus.txt.tmp" || return 1
  grep -qx "sha256 $CORPUS_SHA" "$OUT/corpus.txt.tmp" || {
    say "refused: the located corpus is not $CORPUS_SHA, the one S083's curve 0 recorded and this screen pre-registers"
    return 1
  }
  mv "$OUT/corpus.txt.tmp" "$OUT/corpus.txt"
}

# tools/feature_audit over the located corpus, a frozen copy; a pass is
# written only on S134's 39 null directions and rank 8 more.
audit() {
  enough_disk || return 1
  local info corpus rows dir sha
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/audit"
  mkdir -p "$dir" || return 1
  rm -f "$dir/pass"
  sha=$(freeze_binary "$audit_tool" "$dir/bin/feature_audit") || { echo "$sha"; return 1; }
  {
    echo "S135 audit, $(date -Iseconds)"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line audit "$dir/bin/feature_audit"
  } > "$dir/audit.log"
  check_copy "$dir/bin/feature_audit" || return 1
  "$dir/bin/feature_audit" --data "$corpus" > "$dir/report.txt" 2>&1 ||
    { tail -20 "$dir/report.txt"; say "feature_audit failed"; return 1; }
  grep -E '^identities: (rank|no exact|.*null dimensions)' "$dir/report.txt" | tee -a "$dir/audit.log"
  grep -E 'R\^2' "$dir/report.txt" | grep -E 'bad bishop|bishop rammed|trapped' | tee -a "$dir/audit.log"
  if grep -q '^identities: rank over the rationals exactly 792 of 831, nullity 39' "$dir/report.txt" &&
     grep -q '^identities: no exact dependency beyond the ones listed above' "$dir/report.txt" &&
     grep -q 'every null direction is one Adam never moves or one the tuner holds' "$dir/report.txt"; then
    echo "pass $(date -Iseconds) corpus $(sed -n 's/^sha256 //p' "$OUT/corpus.txt")" > "$dir/pass"
    say "identity: S134's 39 null directions and no more; the four columns are identified"
  else
    say "identity: not S134's 39 and rank 792 -- the coordinator's, before any fit ($dir/report.txt)"
    return 1
  fi
}

# One fit; skipped when its log already holds a best: line, so a stage cut
# short resumes. Runs the stage's frozen tuner, re-hashed first.
fit_tuner=""
fit_one() {
  local data="$1" log="$2"
  shift 2
  if [[ -f "$log" ]] && grep -q '^best: ' "$log"; then
    say "$(basename "$log" .log) fitted before"
    return 0
  fi
  check_copy "$fit_tuner" || return 1
  # shellcheck disable=SC2086
  "$fit_tuner" --data "$data" --holdout "$HOLDOUT" --out "${log%.log}.hpp" \
      "$@" $FIT > "$log" 2>&1 || { say "fit $(basename "$log") failed; see $log"; return 1; }
  say "$(basename "$log" .log): $(grep '^best: ' "$log")"
}

screen() {
  enough_disk || return 1
  check_views || return 1
  if ! grep -qs "corpus $(sed -n 's/^sha256 //p' "$OUT/corpus.txt" 2>/dev/null)" "$OUT/audit/pass"; then
    say "refused: no audit pass for the located corpus; run audit first"
    return 1
  fi
  local info corpus rows dir sha stamp quarter
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/screen"
  mkdir -p "$dir" || return 1

  check_tuner "$tuner" || return 1
  fit_tuner="$dir/bin/tuner"
  sha=$(freeze_binary "$tuner" "$fit_tuner") || { echo "$sha"; return 1; }
  check_tuner "$fit_tuner" || return 1
  probe_tuner "$fit_tuner" "$corpus" "$dir" | tee -a "$dir/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  stamp=$(engine_stamp "$dir/engine.diff") || return 1
  {
    echo "S135 screen"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    cat "$OUT/corpus.txt" | sed 's/^/corpus    /'
    binary_line tuner "$fit_tuner"
    binary_line split "$split"
    binary_line helper "$helper"
    echo "python    $(python3 --version 2>&1)"
  } >> "$dir/screen.log"

  # The five full-corpus fits, then the five quarters, each fitted per term.
  local t s
  for t in $TERMS; do
    fit_one "$corpus" "$dir/full_$t.log" --only "piece_placement.$t" |
      tee -a "$dir/screen.log"
    (( PIPESTATUS[0] == 0 )) || return 1
    python3 "$helper" start-check "$dir/full_$t.hpp" --tree "$root" --free "$t" |
      tee -a "$dir/screen.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  done

  quarter=$(( rows / 4 ))
  for s in $SEEDS; do
    local sample="$dir/quarter_s$s.tsv" all_done=1
    for t in $TERMS; do
      grep -qs '^best: ' "$dir/quarter_s${s}_$t.log" || all_done=0
    done
    if (( all_done == 0 )); then
      rm -f "$sample"
      python3 "$split" sample --in "$corpus" \
          --validation "${VIEWS[0]}" --validation "${VIEWS[1]}" \
          --validation "${VIEWS[2]}" --rows "$quarter" --per-game 0 \
          --seed "$s" --out "$sample" | tee -a "$dir/samples.log"
      (( PIPESTATUS[0] == 0 )) || { say "sample s$s refused"; return 1; }
    fi
    for t in $TERMS; do
      fit_one "$sample" "$dir/quarter_s${s}_$t.log" --only "piece_placement.$t" |
        tee -a "$dir/screen.log"
      (( PIPESTATUS[0] == 0 )) || return 1
      python3 "$helper" start-check "$dir/quarter_s${s}_$t.hpp" --tree "$root" \
          --free "$t" | tee -a "$dir/screen.log"
      (( PIPESTATUS[0] == 0 )) || return 1
    done
  done

  echo "finished  $(date -Iseconds)" >> "$dir/screen.log"
  read_screen
}

read_screen() {
  local dir="$OUT/screen" status
  python3 "$helper" read "$dir" --tree "$root" > "$dir/reading.txt" 2>&1
  status=$?
  cat "$dir/reading.txt"
  return "$status"
}

# The survivors the reading printed, or a refusal.
survivors_of() {
  local reading="$OUT/screen/reading.txt"
  [[ -f "$reading" ]] || { say "no $reading; run screen and read first"; return 1; }
  sed -n 's/^SCREEN-OUTCOME candidate //p' "$reading" | grep . || {
    say "refused: $reading printed no candidate ($(grep '^SCREEN-OUTCOME' "$reading"))"
    return 1
  }
}

# One joint fit: the survivors or a part ($3, comma separated), the rest of
# the group frozen ($2).
joint() {
  local tag="$1" freeze="$2" free="$3" info corpus rows dir sha stamp
  enough_disk || return 1
  check_views || return 1
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/$tag"
  mkdir -p "$dir" || return 1
  check_tuner "$tuner" || return 1
  fit_tuner="$dir/bin/tuner"
  sha=$(freeze_binary "$tuner" "$fit_tuner") || { echo "$sha"; return 1; }
  probe_tuner "$fit_tuner" "$corpus" "$dir" | tee -a "$dir/$tag.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  echo "$free" > "$dir/free.txt"
  stamp=$(engine_stamp "$dir/engine.diff") || return 1
  {
    echo "S135 $tag"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line tuner "$fit_tuner"
    echo "freeze    ${freeze:-(nothing)}"
  } >> "$dir/$tag.log"
  if [[ -n "$freeze" ]]; then
    fit_one "$corpus" "$dir/fit.log" --only piece_placement --freeze "$freeze" |
      tee -a "$dir/$tag.log"
  else
    fit_one "$corpus" "$dir/fit.log" --only piece_placement | tee -a "$dir/$tag.log"
  fi
  (( PIPESTATUS[0] == 0 )) || return 1
  if grep -q 'held-out error did not improve' "$dir/fit.log"; then
    say "the $tag fit did not improve on the shipped constants: unreadable, not a verdict"
    return 1
  fi
  python3 "$helper" start-check "$dir/fit.hpp" --tree "$root" --free "$free" |
    tee -a "$dir/$tag.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  python3 "$helper" headroom "$dir/fit.hpp" --tree "$root" | tee -a "$dir/$tag.log"
  (( PIPESTATUS[0] == 0 )) || { say "the $tag fit passes the stage-one bound: the pre-registered branch, the coordinator's"; return 1; }
  echo "finished  $(date -Iseconds)" >> "$dir/$tag.log"
  say "$tag: $dir/fit.hpp, sha256 $(sha256sum "$dir/fit.hpp" | cut -d' ' -f1)"
}

candidate() {
  local survivors freeze
  survivors=$(survivors_of) || { echo "$survivors"; return 1; }
  freeze=$(sed -n 's/^freeze //p' "$OUT/screen/reading.txt")
  [[ "$freeze" == "(nothing)" ]] && freeze=""
  say "survivors $survivors; freeze ${freeze:-(nothing)}"
  joint candidate "$freeze" "$survivors"
}

part() {
  local name="$1" survivors spec freeze
  survivors=$(survivors_of) || { echo "$survivors"; return 1; }
  spec=$(python3 "$helper" part "$name" --survivors "$survivors") ||
    { echo "$spec"; return 1; }
  echo "$spec"
  freeze=$(sed -n 's/^freeze //p' <<< "$spec")
  joint "part_$name" "$freeze" "$(sed -n 's/^terms //p' <<< "$spec")"
}

# The fit into TREE by S083_corpus.sh's own `apply` -- the paste, the
# constant-by-constant check, both builds, bench and both fast suites, in a
# directory of this run's -- and then the check that only the placement
# weights moved.
apply() {
  local fit="$1" tree="$2" tag free
  tag=$(basename "$(dirname "$fit")")
  [[ -f "$(dirname "$fit")/free.txt" ]] ||
    { say "refused: $fit has no free.txt beside it; apply takes a candidate or part fit"; return 1; }
  free=$(cat "$(dirname "$fit")/free.txt")
  python3 "$helper" start-check "$fit" --tree "$tree" --free "$free" || return 1
  python3 "$helper" headroom "$fit" --tree "$tree" ||
    { say "refused before the paste: the pre-registered branch, the coordinator's"; return 1; }
  OUT="$OUT/apply_$tag" bash "$root/adocs/data/S083_corpus.sh" apply "$fit" "$tree" |
    tee "$OUT/apply_$tag.log"
  grep -qx 'S083-APPLY-DONE' "$OUT/apply_$tag.log" || { say "the paste or its gate failed"; return 1; }
  python3 "$helper" only-placement "$tree" --free "$free"
}

mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S135-FAILED"; exit 1; }

case "$stage" in
  locate)
    if locate; then echo "S135-LOCATE-DONE"; else echo "S135-LOCATE-FAILED"; exit 1; fi ;;
  audit)
    if audit; then echo "S135-AUDIT-DONE"; else echo "S135-AUDIT-FAILED"; exit 1; fi ;;
  screen)
    if screen; then echo "S135-SCREEN-DONE"; else echo "S135-SCREEN-FAILED"; exit 1; fi ;;
  read)
    if read_screen; then echo "S135-READ-DONE"; else echo "S135-READ-FAILED"; exit 1; fi ;;
  candidate)
    if candidate; then echo "S135-CANDIDATE-DONE"; else echo "S135-CANDIDATE-FAILED"; exit 1; fi ;;
  part)
    if [[ $# -eq 2 ]] && part "$2"; then echo "S135-PART-DONE"; else echo "S135-PART-FAILED"; exit 1; fi ;;
  apply)
    if [[ $# -eq 3 ]] && apply "$2" "$3"; then echo "S135-APPLY-DONE"; else echo "S135-APPLY-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 locate | audit | screen | read | candidate | part NAME | apply FIT TREE" >&2
    exit 2 ;;
esac
