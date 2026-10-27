#!/usr/bin/env bash
# S136's screen: tempo, a family of one (DEC-259), fitted alone with every
# other constant frozen, kept only past the margin stated here, then pasted
# and measured by one SPRT (adocs/data/S136_sprt.sh). DEC-092, DEC-315's rule
# over one term.
#
#   adocs/data/S136_screen.sh locate          # the corpus, its sha256
#   adocs/data/S136_screen.sh forecast        # the guard's forecast on it
#   adocs/data/S136_screen.sh screen          # the six fits; resumable
#   adocs/data/S136_screen.sh read            # the rule, over the fits on disk
#   adocs/data/S136_screen.sh apply FIT TREE  # the fit into TREE, built, gated
#
# Run from the main checkout with S136's phase 1 landed (it changes nothing
# under src/ or tools/, so build/'s tuner is HEAD's; the probe below checks
# that rather than assuming it). Each stage prints S136-<STAGE>-DONE or
# S136-<STAGE>-FAILED as its last stdout line, the reasons above a FAILED, so
# each runs detached under a four-exit watcher (WATCHERS). OUT defaults to
# .tuning/s136 (DEC-235: a run's outputs stay with the run; the readings go to
# adocs/data/ and the step file).
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-27 in S136's phase 1, at 80520d7, before
# any of it ran. Nothing below the line changes once `screen` has started; a
# change is a dated section in the step file and a decision.
# ============================================================================
#
# THE CORPUS: S083's, the one S135's screen and its shipped constants were
# fitted on (S083 read H1, DEC-313). `locate` finds it mechanically by
# S135_screen.py's own `locate --source s083`, reused by argument: the last
# $S083_OUT/curve/sN/reading.txt that printed READ-OUTCOME stop, its
# `candidate fit_<top>_s1.hpp`, and cand_<top>_s1.tsv beside it, hashed
# against that stage's samples.log, re-drawn from its pool if removed -- then
# against CORPUS_SHA below, curve 0's top rung, every game of S082's training
# side at 46 rows a game, 6226944 rows, either mismatch refusing. The fixed
# validation set is S082's qs view, its three views' sha256 checked before
# every stage.
#
# NO IDENTITY STAGE. S135's `audit` ran tools/feature_audit over this corpus
# at the 831-parameter layout (.tuning/s135/audit/report.txt): rank exactly 792
# of 831, the 39 null directions S134 named -- 32 pawn columns on ranks 1 and
# 8, five material-against-table directions, the two king tables -- none of
# them tempo, and no exact dependency beyond them. Tempo's column is
# identified on this corpus and nothing since has changed a column.
#
# THE FIT, DEC-304's settings as S135's screen used them, `--only tempo`:
# `tuner --data CORPUS --holdout .tuning/s082/pool.tsv.qs.validation --only
# tempo --epochs 5000 --report 100 --patience 20 --lambda 0 --seed 1
# --threads 12`. Two parameters free, tempo_mg and tempo_eg; every other
# constant at what ships -- S135's fit included -- and K fitted from each
# corpus's own rows. `--lambda 0` is stated, not defaulted: the label is the
# game's result alone, because a score-blend label cannot teach a term the
# current evaluator scores at zero (DEC-092). One frozen copy of the tuner
# for the whole screen.
#
# THE NOISE: five samples of the corpus at a quarter of its rows,
# `corpus_split.py sample --in CORPUS --rows C/4 --per-game 0 --seed s`, s = 1
# to 5, the three views named; tempo fitted on each by the same command.
#
# THE READING, by S136_screen.py read, mechanically -- DEC-315's rule, over
# one term:
#   1. A fit's gain g is its own start figure minus its `best:` validation
#      figure, both on the fixed set: the shipped constants with the run's K,
#      and the minimum over the run's reports.
#   2. s is the sample standard deviation (n - 1) of the five quarter gains;
#      m = max(1.645 s, 2e-6).
#   3. Tempo is kept when the full-corpus gain g > m, and dropped otherwise.
#   4. Unreadable, nothing chosen: a fit missing or refused, a log whose
#      start: and best: lines disagree about the start, or s above 1e-3.
#   Printed beside, deciding nothing: the six fitted pairs; S027's --only fit
#   (mg 10, eg 0), S065's unfrozen one (39, 21) and the label-side size --
#   near 3.6 cp on S100's corpus, near -8.7 cp on this one by S135's audit's
#   gap of -0.017848, both confounded -- none of them a seed or a target
#   (DEC-105);
#   and the truncation guard's arithmetic at the full fit's pair with the
#   red it forecasts (S136_screen.py guard).
#
# THE MARGIN, as DEC-315 states it and S135's header argues it, not re-argued
# here: s prices training-corpus resampling alone; the fixed set's own
# sampling noise and the best-report pick on it are common to the six fits,
# unpriced, and bias g upward -- accepted because the screen only removes a
# term and the one SPRT decides what ships. The fit frees 2 parameters
# against N = 1015332 validation rows from 12000 games. The floor of 2e-6 is
# two units of the tuner's sixth printed decimal.
#
# THE TUNER IS HEAD'S, CHECKED. The frozen copy is first run for zero epochs
# on the corpus's first 2000 rows and its header compared, value by value,
# with HEAD's sources (S136_screen.py start-check); any difference refuses.
# Every fit's header is checked the same way after, tempo_mg and tempo_eg
# excepted. No stage-one headroom check: tempo is tapered on its own and is
# in no packed sum stage_one_bound covers.
#
# THE OUTCOMES.
#   SCREEN-OUTCOME none       -> tempo is recorded as zero with its figures
#                                (g, s, m, the fitted pairs); no paste, no
#                                match, the guard unchanged. The weights stay
#                                0 and the compiler deletes the term.
#   SCREEN-OUTCOME candidate  -> the candidate is the full-corpus fit itself,
#                                $OUT/screen/full_tempo.hpp: a family of one
#                                has no survivors to refit together, and the
#                                joint fit's command would be this one, same
#                                seed. `apply` pastes it; S136_sprt.sh is the
#                                one `{0, 5}` nElo SPRT. There is no bisection.
#   SCREEN-OUTCOME unreadable -> nothing chosen; the coordinator's.
#
# THE GUARD, AT THE PASTE (the step file's guard plan has the derivation).
# `apply` pastes, builds and gates by S083_corpus.sh's own `apply`, whose
# reds are the goldens the paste moved; then it checks that only tempo_mg and
# tempo_eg moved, prints S136_screen.py guard's forecast for the fitted pair
# beside the two truncation cases run on the pasted build -- the red
# forecast against the red observed, before any test line moves -- and,
# where tempo's division rounds at the fitted pair, the re-derivation's
# readings: the pasted build's `truncation_scan --min 2.875` over the corpus
# and, concatenated, over S082's three training views (TRAIN_VIEWS), then
# adocs/data/S083_truncation_pick.py and S136_screen.py pick over each, then
# S136_screen.py short over the views. DEC-322, the coordinator's of
# 2026-10-27: the pins come from the views, by S083's pick, by S136's where
# that refuses (at four divisions S083's refuses on the corpus for 43 of the
# 46 non-zero spreads and on the views for 1,
# adocs/data/S136_guard_forecast.txt). DEC-322 (3) is NOT ruled: where the
# views fall short of the arithmetic reach (spreads 1 and 23 modulo 24),
# `short` prints a STOP line, the stage repeats it before its marker, and
# phase 2 stops before the re-targeting for the owner. A forecast, scan or
# S136 pick that exits non-zero fails the stage; S083's pick refusing and the
# truncation cases going red are readings.
#
# MACHINE HOURS, from S135's screen as measured (.tuning/s135/screen, its
# headers' mtimes, 2026-10-25 21:18 to 22:27, 30 fits in 1 h 09 m): a full
# fit of 6226944 rows took 5.3 to 10.7 min at 2000 to 3600 epochs, a quarter
# fit 1.3 to 2.2 min at 2000 to 3500 (quarter_s4_trapped the longest, 2 m
# 09 s), each quarter sample drawn in about 20 s. An epoch's cost is the
# rows' trace, which the free set does not shorten (S135's header), so
# tempo's fits cost what a placement term's did. The screen, one full fit and
# five quarters with their samples: about 14 min typical (2000-epoch fits),
# 23 min if every fit runs as long as S135's longest, 31 min at the
# 5000-epoch cap, ceiling 1 h. `locate`
# under a minute (hashing 0.49 GB). `forecast` about 2 min (18.4 s measured
# on the corpus at 80520d7, 31 to 34 s on each training view). `read`
# seconds. `apply` 6 min (S135's 5 m 36 s) plus about 2 min for the scans.
# All daytime work under DEC-155.
#
# DISK. The five quarter samples, about 124 MB each, 0.62 GB, removable once
# `read` is recorded, each reproducible from its seed and the corpus's
# sha256; the forecast's rows 8.6 MB for the corpus and 48 MB for the views.
# A stage refuses to start under 4 GB free.
#
# OPEN FINDINGS (BUGS as scoped by DEC-171): written at launch by the
# coordinator, in the step file, against plan.md's Open list on the day.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s136}"
S083_OUT="${S083_OUT:-$root/.tuning/s083}"
CORPUS_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
THREADS=12
FREE_GB_FLOOR=4
SEEDS="1 2 3 4 5"

S082_OUT="${S082_OUT:-$root/.tuning/s082}"
HOLDOUT="$S082_OUT/pool.tsv.qs.validation"
VIEWS=("$S082_OUT/pool.tsv.qs.validation"
       "$S082_OUT/pool.tsv.root.validation"
       "$S082_OUT/pool.tsv.pv.validation")
VIEW_SHAS=(2b508067fb64693bb4b278a2455fb9d101eaa742975a4b53f4ff56e6fa16981e
           58bcbb315fd82bd332372d0b2f1df7d7370693568bd27fa9ed6f17b8e74652d6
           24c4f671131c4a6f3f945ef0f5bb0d8cf85969b41d2e846e745539ef2d53ffdd)
# S082's three training views, a second set of positions for the guard's
# forecast and scan (a residual belongs to the weights, not to where a
# position came from); the corpus is a sample of the first.
TRAIN_VIEWS=("$S082_OUT/pool.tsv.qs.train"
             "$S082_OUT/pool.tsv.root.train"
             "$S082_OUT/pool.tsv.pv.train")
FIT="--only tempo --epochs 5000 --report 100 --patience 20 --lambda 0
     --seed 1 --threads $THREADS"

# The four FENs tests/test_eval_trace.cpp pins at S136's HEAD, which
# S136_screen.py PINS records with their residuals.
PIN_FENS=("2R5/8/3r3p/6p1/1k2B1P1/4K2P/5P2/8 b - - 16 52"
          "6k1/7p/p2b2p1/5n2/8/2Q5/PP3P1P/2KN3q w - - 1 33"
          "2q3k1/1b1R2p1/2p1r3/p2pNQ1p/Pb1P2pP/4P1P1/1P4K1/2R5 b - - 5 35"
          "rn5k/ppp3bp/3qbp2/3p2pB/8/1PN2Q1P/P1PP1PP1/R4RK1 w - - 4 16")

tuner="${TUNER:-$root/build/tools/tuner}"
trace_lib="${TRACE_LIB:-$root/build/src/libchesso_engine_trace.a}"
split="$root/tools/corpus_split.py"
helper="$root/adocs/data/S136_screen.py"
s135_helper="$root/adocs/data/S135_screen.py"

say() { echo "$*"; }

binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

# The engine a stage ran, as S083_corpus.sh and S135_screen.sh name it:
# HEAD's sha, or HEAD's sha plus the first 12 hex of the blob id of the diff
# under src/ and tools/, saved at $1. Refused when an untracked file sits
# under src/ or tools/.
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
# checked before every run, so a rebuild of build/ cannot put another tuner
# under a recorded sha. An existing copy is reused, never replaced. Prints
# the sha.
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
  grep -qx "sha256 $CORPUS_SHA" "$record" ||
    { say "refused: $record names sha256 $sha, not $CORPUS_SHA, the corpus this screen pre-registers"; return 1; }
  [[ "$(sha256sum "$path" | cut -d' ' -f1)" == "$sha" ]] ||
    { say "$path is not the corpus $record names"; return 1; }
  echo "$path $rows"
}

locate() {
  enough_disk || return 1
  check_views || return 1
  if [[ -f "$OUT/corpus.txt" ]]; then
    say "$OUT/corpus.txt exists; a located corpus is never replaced"
    cat "$OUT/corpus.txt"
    grep -qx "sha256 $CORPUS_SHA" "$OUT/corpus.txt" ||
      { say "refused: $OUT/corpus.txt does not name $CORPUS_SHA, the corpus this screen pre-registers"; return 1; }
    local info
    info=$(corpus_of) || { echo "$info"; return 1; }
    return 0
  fi
  rm -f "$OUT/corpus.txt.tmp"
  python3 "$s135_helper" locate --source s083 --s083-out "$S083_OUT" \
      --s082-out "$S082_OUT" --out "$OUT/corpus.txt.tmp" || return 1
  grep -qx "sha256 $CORPUS_SHA" "$OUT/corpus.txt.tmp" || {
    say "refused: the located corpus is not $CORPUS_SHA, the one S083's curve 0 recorded and this screen pre-registers"
    return 1
  }
  mv "$OUT/corpus.txt.tmp" "$OUT/corpus.txt"
}

# adocs/data/S136_guard_forecast.cpp built against build/'s trace library and
# run on the corpus and on the four pins; the pins' reading must be the one
# S136_screen.py PINS records, or the forecast is of other weights.
forecast() {
  enough_disk || return 1
  local info corpus rows dir stamp
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/forecast"
  mkdir -p "$dir/bin" || return 1
  [[ -f "$trace_lib" ]] || { say "no trace library at $trace_lib; build build/ first"; return 1; }
  stamp=$(engine_stamp "$dir/engine.diff") || return 1
  g++ -std=c++20 -O2 -Wall -Wextra -Werror -DCHESSO_TRACE -I"$root/src" \
      -I"$root/tools" "$root/adocs/data/S136_guard_forecast.cpp" "$trace_lib" \
      -lpthread -o "$dir/bin/guard_forecast" > "$dir/build.log" 2>&1 ||
    { cat "$dir/build.log"; say "S136_guard_forecast.cpp did not build"; return 1; }
  {
    echo "S136 forecast"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line forecast "$dir/bin/guard_forecast"
    binary_line library "$trace_lib"
  } > "$dir/forecast.log"
  printf '%s\n' "${PIN_FENS[@]}" > "$dir/pins.fen"
  "$dir/bin/guard_forecast" --data "$dir/pins.fen" --min 0 > "$dir/pins.tsv" \
      2>> "$dir/forecast.log" || { say "the forecast failed on the pins"; return 1; }
  "$dir/bin/guard_forecast" --data "$corpus" > "$dir/rows.tsv" \
      2>> "$dir/forecast.log" || { tail -3 "$dir/forecast.log"; say "the forecast failed on the corpus"; return 1; }
  binary_line rows "$dir/rows.tsv" >> "$dir/forecast.log"
  local view
  rm -f "$dir/views_rows.tsv"
  for view in "${TRAIN_VIEWS[@]}"; do
    binary_line view "$view" >> "$dir/forecast.log"
    "$dir/bin/guard_forecast" --data "$view" >> "$dir/views_rows.tsv" \
        2>> "$dir/forecast.log" || { tail -3 "$dir/forecast.log"; say "the forecast failed on $view"; return 1; }
  done
  binary_line views "$dir/views_rows.tsv" >> "$dir/forecast.log"
  python3 "$helper" forecast "$dir/rows.tsv" --pins "$dir/pins.tsv" > "$dir/table.txt" ||
    { cat "$dir/table.txt"; return 1; }
  python3 "$helper" forecast "$dir/views_rows.tsv" --pins "$dir/pins.tsv" \
      > "$dir/views_table.txt" || { cat "$dir/views_table.txt"; return 1; }
  echo "finished  $(date -Iseconds)" >> "$dir/forecast.log"
  cat "$dir/forecast.log" "$dir/table.txt" "$dir/views_table.txt"
}

# One fit; skipped when its log already holds a best: line, so a stage cut
# short resumes. Runs the stage's frozen tuner, re-hashed first.
fit_tuner=""
fit_one() {
  local data="$1" log="$2"
  if [[ -f "$log" ]] && grep -q '^best: ' "$log"; then
    say "$(basename "$log" .log) fitted before"
    return 0
  fi
  check_copy "$fit_tuner" || return 1
  # shellcheck disable=SC2086
  "$fit_tuner" --data "$data" --holdout "$HOLDOUT" --out "${log%.log}.hpp" \
      $FIT > "$log" 2>&1 || { say "fit $(basename "$log") failed; see $log"; return 1; }
  say "$(basename "$log" .log): $(grep '^best: ' "$log")"
}

screen() {
  enough_disk || return 1
  check_views || return 1
  local info corpus rows dir sha stamp quarter s sample
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/screen"
  mkdir -p "$dir" || return 1

  fit_tuner="$dir/bin/tuner"
  sha=$(freeze_binary "$tuner" "$fit_tuner") || { echo "$sha"; return 1; }
  probe_tuner "$fit_tuner" "$corpus" "$dir" | tee -a "$dir/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  stamp=$(engine_stamp "$dir/engine.diff") || return 1
  {
    echo "S136 screen"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line tuner "$fit_tuner"
    binary_line split "$split"
    binary_line helper "$helper"
    binary_line s135 "$s135_helper"
    echo "python    $(python3 --version 2>&1)"
  } >> "$dir/screen.log"

  fit_one "$corpus" "$dir/full_tempo.log" | tee -a "$dir/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  python3 "$helper" start-check "$dir/full_tempo.hpp" --tree "$root" --free |
    tee -a "$dir/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1

  quarter=$(( rows / 4 ))
  for s in $SEEDS; do
    sample="$dir/quarter_s$s.tsv"
    if ! grep -qs '^best: ' "$dir/quarter_s${s}_tempo.log"; then
      rm -f "$sample"
      python3 "$split" sample --in "$corpus" \
          --validation "${VIEWS[0]}" --validation "${VIEWS[1]}" \
          --validation "${VIEWS[2]}" --rows "$quarter" --per-game 0 \
          --seed "$s" --out "$sample" | tee -a "$dir/samples.log"
      (( PIPESTATUS[0] == 0 )) || { say "sample s$s refused"; return 1; }
    fi
    fit_one "$sample" "$dir/quarter_s${s}_tempo.log" | tee -a "$dir/screen.log"
    (( PIPESTATUS[0] == 0 )) || return 1
    python3 "$helper" start-check "$dir/quarter_s${s}_tempo.hpp" --tree "$root" \
        --free | tee -a "$dir/screen.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  done

  echo "finished  $(date -Iseconds)" >> "$dir/screen.log"
  read_screen
}

read_screen() {
  local dir="$OUT/screen" status rows=() f
  for f in rows views_rows; do
    [[ -f "$OUT/forecast/$f.tsv" ]] && rows+=(--rows "$OUT/forecast/$f.tsv")
  done
  python3 "$helper" read "$dir" --tree "$root" "${rows[@]}" > "$dir/reading.txt" 2>&1
  status=$?
  cat "$dir/reading.txt"
  return "$status"
}

# The guard's readings at the paste, each appended to $4/guard.txt and the
# stage log: the forecast, the two truncation cases as the pasted build runs
# them, and -- where tempo's division rounds -- the scans, both picks over
# each and the reach check on S082's views (DEC-322). A forecast, scan or
# S136 pick that exits non-zero fails the stage; S083's pick refusing and the
# truncation cases going red are readings, not failures.
guard_readings() {
  local fit="$1" tree="$2" corpus="$3" dir="$4" log="$4.log" out mg eg view scan
  out="$dir/guard.txt"
  : > "$out"
  note() { echo "$*" | tee -a "$out" "$log"; }
  run() { "$@" 2>&1 | tee -a "$out" "$log"; return "${PIPESTATUS[0]}"; }

  note "== the red forecast for the fitted pair, before any test line moves"
  run python3 "$helper" guard --header "$fit" --rows "$OUT/forecast/rows.tsv" \
      --rows "$OUT/forecast/views_rows.tsv" ||
    { say "the guard forecast failed"; return 1; }

  note "== the red observed: the two truncation cases on the pasted build"
  # From the test directory, as ctest runs them: the shared position list
  # reads assets/test_jsons/ relative to it.
  (
    cd "$tree/build/tests" || exit 1
    ./test_eval_trace -tc="the pinned positions reach the truncation bound"
    ./test_eval_trace \
        -tc="the smooth score is within the truncation bound of the exact one"
    exit 0
  ) 2>&1 | tee -a "$out" "$log"

  mg=$(sed -n 's/^constexpr int tempo_mg = \(-\{0,1\}[0-9]*\);$/\1/p' "$fit")
  eg=$(sed -n 's/^constexpr int tempo_eg = \(-\{0,1\}[0-9]*\);$/\1/p' "$fit")
  [[ -n "$mg" && -n "$eg" ]] || { say "no tempo pair in $fit"; return 1; }
  if (( (mg - eg) % 24 == 0 )); then
    note "== tempo's division is exact at $mg / $eg: three divisions round, the four pins stand, no re-draw"
    return 0
  fi

  note "== the re-derivation's readings: $tree/build/tools/truncation_scan --min 2.875"
  "$tree/build/tools/truncation_scan" --data "$corpus" --min 2.875 \
      > "$dir/scan.txt" 2> "$dir/scan.err" ||
    { cat "$dir/scan.err"; say "truncation_scan failed on the corpus"; return 1; }
  run cat "$dir/scan.err"
  rm -f "$dir/views_scan.txt" "$dir/views_scan.err"
  for view in "${TRAIN_VIEWS[@]}"; do
    "$tree/build/tools/truncation_scan" --data "$view" --min 2.875 \
        >> "$dir/views_scan.txt" 2>> "$dir/views_scan.err" ||
      { cat "$dir/views_scan.err"; say "truncation_scan failed on $view"; return 1; }
  done
  run cat "$dir/views_scan.err"
  for scan in scan views_scan; do
    note "-- $scan: adocs/data/S083_truncation_pick.py (S083's rule, at the maximum)"
    run python3 "$root/adocs/data/S083_truncation_pick.py" "$dir/$scan.txt" ||
      note "   (S083's rule refuses here)"
    note "-- $scan: S136_screen.py pick (DEC-322 (2), where S083's refuses)"
    run python3 "$helper" pick "$dir/$scan.txt" --header "$fit" ||
      { say "S136_screen.py pick failed on $scan"; return 1; }
  done
  note "-- views_scan: the reach (DEC-322 (3) is unruled)"
  run python3 "$helper" short "$dir/views_scan.txt" --header "$fit" ||
    { say "the reach check failed"; return 1; }
}

# The fit into TREE by S083_corpus.sh's own `apply` -- the paste, the
# constant-by-constant check, both builds, bench and both fast suites, in a
# directory of this run's -- then the check that only tempo moved, the
# guard's forecast against the two truncation cases as the pasted build runs
# them, and the re-derivation's readings. TREE stays uncommitted; nothing here
# edits a test.
apply() {
  local fit="$1" tree="$2" dir info corpus rows
  [[ "$(cd "$(dirname "$fit")" && pwd)/$(basename "$fit")" == "$OUT/screen/full_tempo.hpp" ]] ||
    { say "refused: apply takes the screen's candidate, $OUT/screen/full_tempo.hpp"; return 1; }
  grep -qx 'SCREEN-OUTCOME candidate tempo' "$OUT/screen/reading.txt" 2>/dev/null ||
    { say "refused: $OUT/screen/reading.txt printed no candidate"; return 1; }
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/apply_full"
  python3 "$helper" start-check "$fit" --tree "$tree" --free || return 1
  OUT="$dir" bash "$root/adocs/data/S083_corpus.sh" apply "$fit" "$tree" |
    tee "$dir.log"
  grep -qx 'S083-APPLY-DONE' "$dir.log" || { say "the paste or its gate failed"; return 1; }
  python3 "$helper" only-tempo "$tree" | tee -a "$dir.log"
  (( PIPESTATUS[0] == 0 )) || return 1

  guard_readings "$fit" "$tree" "$corpus" "$dir" || return 1
  if grep -q '^STOP: ' "$dir/guard.txt"; then
    say "STOP: S082's views fall short of the arithmetic reach at the fitted pair; phase 2 stops before the re-targeting and the owner rules (DEC-322 (3))"
  fi
  say "the guard re-targeting is the step's, by the plan in its step file; nothing here edited a test"
}

mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S136-FAILED"; exit 1; }

case "$stage" in
  locate)
    if locate; then echo "S136-LOCATE-DONE"; else echo "S136-LOCATE-FAILED"; exit 1; fi ;;
  forecast)
    if forecast; then echo "S136-FORECAST-DONE"; else echo "S136-FORECAST-FAILED"; exit 1; fi ;;
  screen)
    if screen; then echo "S136-SCREEN-DONE"; else echo "S136-SCREEN-FAILED"; exit 1; fi ;;
  read)
    if read_screen; then echo "S136-READ-DONE"; else echo "S136-READ-FAILED"; exit 1; fi ;;
  apply)
    if [[ $# -eq 3 ]] && apply "$2" "$3"; then echo "S136-APPLY-DONE"; else echo "S136-APPLY-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 locate | forecast | screen | read | apply FIT TREE" >&2
    exit 2 ;;
esac
