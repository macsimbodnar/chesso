#!/usr/bin/env bash
# S082's recipe comparison: which position a corpus row names, and how many
# rows a game gives, chosen offline by held-out loss on the fixed validation
# set this step builds (DEC-259). No SPRT of its own: S083's one SPRT measures
# the corpus that ships, at the size and node budget S083 chooses.
#
#   adocs/data/S082_recipe.sh smoke      # 600 games: the abort rule's reading
#   adocs/data/S082_recipe.sh generate   # the pool: the night's part
#   adocs/data/S082_recipe.sh compare    # the set, the corpora, the fits
#   adocs/data/S082_recipe.sh verify [PREFIX LOG GAMES]
#                                        # a pool against its own log; by
#                                        # default $OUT/pool.tsv, generate.log
#   adocs/data/S082_recipe.sh merge      # the pool a power cut stopped, and
#                                        # its top-up, joined (DEC-296)
#
# Each prints its own terminal marker as its last action -- S082-SMOKE-,
# S082-GENERATE-, S082-COMPARE-, S082-VERIFY- or S082-MERGE- then DONE or
# FAILED -- so any of them can run detached under a watcher with all four
# exits (WATCHERS).
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-13, phase 1, before any of it ran
# (DEC-260: the machine was S261's). Nothing below is changed after the pool
# exists; a change is a new dated section in the step file.
# ============================================================================
#
# HELD FIXED, because S083 owns them. The node budget, 100000 a move -- S065's,
# the corpus the shipping weights descend from and one of S083's two
# candidates. The corpus size, 1000000 training rows for every candidate,
# exactly: tools/corpus_split.py sample refuses rather than return fewer. The
# fit: S076's regime, `--freeze tempo,piece_placement --epochs 5000 --report
# 100 --patience 20 --lambda 0 --seed 1 --threads 12`, K fitted from each
# candidate's own rows. The engine: whatever HEAD is when `generate` starts,
# stated in the log -- S261, S099, S023 and S039 land before this runs.
#
# THE FIXED VALIDATION SET, DEC-275. 12000 games -- the held-out game count of
# the split every fit since S066 was read on, 10 % of S065's 120000 -- chosen by
# tools/corpus_split.py split under seed 20261013 from the pool's games before
# anything is sampled; every row of those games, played at 100000 nodes. Its
# primary view is the quiescence leaf, pool.tsv.qs.validation: the objective
# CPW's Texel's Tuning Method defines is the quiescence score of a position
# against the game result, and evaluate() at a position's quiescence leaf is
# that score. The root view of the same games is fitted beside it in every cell
# and reported, never decisive; the pv view is written and kept out of every
# corpus, and read by nothing here.
#
# THE CANDIDATES, eight cells and the control:
#
#   leaf      root (the control at every row: the incumbent recipe, S065's
#             --allow-tactical 1 under another name), qs, pv
#   per game  0 -- every row, the incumbent's density, 91.70 a game in S065;
#             46 -- the midpoint of the range the step file declares, 1 to 92
#             (DEC-105 form (c), section 4); 8 -- the lowest density one
#             night's pool affords at a million rows: 1000000 / 8 = 125000
#             training games
#
#   The control, (root, every row), is sampled five times, under seeds 1 to 5:
#   five corpora of one recipe, whose spread is the noise between two corpora
#   at this size. The eight other cells are sampled once, under seed 1.
#
# Every corpus is sampled against all three views of the set, so none holds a
# position any view holds, and fitted twice: `--holdout` on the qs view
# (primary) and on the root view (reported).
#
# THE READING, mechanically, by adocs/data/S082_recipe_read.py:
#
#   1. L(c) is the `best:` line's validation figure of c's primary fit -- the
#      held-out error against the game result at the best reported epoch.
#   2. The control's L is the mean of its five fits, and s their sample
#      standard deviation (n - 1). The margin is
#
#          m = 2.49 * sqrt(2) * s, about 3.52 s
#
#      -- sqrt(2) s the spread of a difference between two single fits, and
#      2.49 the 95 % point of the largest of eight independent standard
#      normals (Phi(x)^8 = 0.95), since the best is picked from eight cells.
#      Conservative against the control, a mean of five whose difference with
#      one fit spreads as sqrt(1.2) s. No floor under m: s is measured at this
#      scale, and S075's 4e-05 was checkpoint jitter inside one 11 M-row fit,
#      another quantity. s from five draws is itself loose -- a 90 % interval
#      on the true spread runs from about 0.65 s to 2.37 s (chi-square, 4
#      degrees of freedom) -- which is why the rule only ever picks a cell
#      that clears m, and an SPRT (S083's) still decides what ships.
#   3. The best cell is the lowest L of the eight. If the control is within m
#      of it -- L(control) - L(best) <= m -- no recipe beat the incumbent: the
#      step records a zero and S083 scales the incumbent recipe.
#   4. Otherwise the chosen cell is, among the eight within m of the best, the
#      one with the most rows a game (S083 pays per game, not per row), and at
#      equal density qs before pv before root (qs is reproducible from the FEN
#      alone).
#   5. The root view, K, rows a game and each leaf's refusal shares from the
#      pool's own log are printed beside and decide nothing (DEC-275). A leaf
#      that fits worse is a finding and not a failure (step file section 6).
#
# THE THREE OUTCOMES. (a) A leaf cell is chosen: S083 generates at its leaf and
# density. (b) The control stands: S083 generates root rows -- the retired
# clause stays retired, since S065's corpus already admitted those rows. (c)
# The run cannot be read -- a fit refused, a sample refused, s above 1e-3 (ten
# times anything recorded): nothing is chosen, the step stays open, and the
# coordinator writes what went wrong before anything re-runs.
#
# ABORT RULE. `generate` does not start, or is stopped and nothing is read, if
# the 600-game smoke run at these settings shows, on either walked leaf, more
# than 1 % of the positions that passed the filter refused as truncated,
# aborted or uncertified -- the refusals that mean the walk is not measuring
# what it claims -- or a games-an-hour figure that puts the pool past 18 h.
# Terminal refusals are a policy outcome and are printed, never gated on: a
# line can meet a mate or a repetition the search draws with a correct walk,
# and endgame-heavy self-play meets them often. `smoke` prints the reading
# (S082_recipe_read.py --smoke).
#
# MACHINE HOURS, from the one datagen throughput measured on this workstation
# (S065: 120000 games at 100000 nodes in 7 h 08 m, 4.669 games/s, 2026-08-14
# -- .moltke.local.md records none) and the one fit time (S076: 10795695 rows,
# 1378 s): the pool is 150000 games, 12000 to the set and 138000 to train on,
# about 9 h at S065's rate plus the walks -- a night, DEC-155. `compare` is
# about 1 h 15 m: the split and thirteen samples in minutes, twenty-six fits at
# a million rows of about two minutes each. `smoke` is the abort rule's
# reading, about 3 minutes (S065's 600-game smoke took 144 s).
#
# LAUNCH, written 2026-10-16 by phase 2 on `816d70c`, after the smoke and
# before the pool. The smoke on this tree (S286's engine, the correction in
# every worker, plus S082) read 600 games in 192 s, 11250 games an hour, and
# its re-run after the fast check's two fixes 193 s, 11191 an hour, the same
# rows; so the pool's 150000 are about 13.4 h -- past one night; S065's full
# run was 12 % faster than its own smoke, so about 12 h is the likely low end,
# and the abort rule's 18 h is clear. With S082 committed, build `build/`
# (`cmake --build build -j12`), then from the main tree:
#
#   cd /home/max/ws/chesso && mkdir -p .tuning/s082 &&
#   OUT=/home/max/ws/chesso/.tuning/s082 THREADS=12 nohup \
#       adocs/data/S082_recipe.sh generate \
#       > /home/max/ws/chesso/.tuning/s082/generate.console 2>&1 &
#
# It writes $OUT/pool.tsv.root, .qs and .pv and $OUT/generate.log: the
# stamp, the sha256 of the datagen binary that runs, the command, datagen's
# own lines, the elapsed time, check_pool()'s findings, each file's rows and
# sha256. It refuses before datagen starts when $OUT/pool.tsv.qs exists or
# an untracked file sits under src/ or tools/. On a tree with src/ or tools/
# uncommitted the stamp names the diff by blob id and saves it as
# $OUT/generate.engine.diff. DONE needs datagen's exit 0 -- since the fast
# check it exits 1 on a failed write or flush and on fewer games than booked
# -- and check_pool() clean: every leaf's log line plays the 150000 games and
# names its file, the file ends on a newline, holds the rows the line counts,
# and every row has seven fields. Its last stdout line, so
# generate.console's, is S082-GENERATE-DONE or S082-GENERATE-FAILED, the
# reasons above it; a watcher's ceiling is 27 h, twice the estimate.
# `verify` re-runs the same check on the pool at any time, and `compare` runs
# it before it reads anything. Disk: the smoke wrote 11.5 MB per 600 games
# over the three files, so the pool is about 2.9 GB, `compare`'s split as much
# again and its thirteen samples about 1 GB, against 21 GB free on 2026-10-16.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171),
# added 2026-10-14 at the step's rebase onto `001c82e`, re-read against
# plan.md's Open list there: **none**. S278, S279 and S280 closed before it;
# S281, conditional on S099's H1, is a study of the short mate lines S099's
# tree adds (DEC-284), a reporting residue (DEC-122) and not a defect. The
# coordinator re-reads the list on the day `generate` starts. "Land before
# this runs" above means decided: on S099's H0 its revert is in HEAD instead.
# Re-read 2026-10-15 at `ced5d77`, the step's third base: **none**. S283 is an
# SPRT and S039 a committed candidate awaiting its own; S281 is void on S099's
# H0; S282 and S285, both filler, are closed. S099's and S023's reverts on
# their H0 are in HEAD, and S039's verdict, which follows S283's, is read
# before this runs.
# Re-read 2026-10-16 at `816d70c`, the base phase 2 runs on: **S287**, open,
# filler under DEC-171 -- eight documents and comments still saying S039
# retires or re-decides the lazy shortcut and the clamp, which S039's H0 kept
# (`74d0adf`). Prose and comments only: it moves no evaluation, search, score
# or row, so the pool is taken with it open and reads nothing it touches.
# Every step "land before this runs" names is decided: S099 relanded by S286
# on S283's H1, the correction in every datagen worker (DEC-294); S023 and
# S039 reverted on their H0. S281 stays void (plan.md does not list it).
# ============================================================================
#
# 2026-10-20, DEC-296, below the pre-registration and changing none of it: a
# power cut stopped `generate` 11 h 22 m in, before datagen's closing lines.
# The pool is the 122748 games complete in all three files before the last one
# they reach, and 27252 games of a top-up from the same datagen binary at the
# same settings under seed 2026102000, joined by the `merge` stage. Every
# cell, seed and reading rule above stands. The step file's section
# "2026-10-20: the pool cut by a power loss, and its top-up (DEC-296)" has the
# facts, the checks and what part 1's lost log no longer gives.

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s082}"
THREADS="${THREADS:-12}"

GAMES=150000
NODES=100000
SEED=20261013
TOPUP_SEED=2026102000  # DEC-296: the top-up that completes the pool
VALIDATION_GAMES=12000
ROWS=1000000
LEAVES="root qs pv"
DENSITIES="0 46 8"
CONTROL_SEEDS="1 2 3 4 5"
FIT="--freeze tempo,piece_placement --epochs 5000 --report 100 --patience 20
     --lambda 0 --seed 1 --threads $THREADS"

datagen="$root/build/tools/datagen"
tuner="$root/build/tools/tuner"
split="$root/tools/corpus_split.py"
reader="$root/adocs/data/S082_recipe_read.py"
merger="$root/adocs/data/S082_pool_merge.py"

# The first line of a merged pool's log (DEC-296). check_pool() reads such a
# log by the merge's own lines, never by datagen's.
MERGED_HEADER="S082 pool, merged from two datagen runs (DEC-296); not one run's log"

# The engine a stage ran: HEAD's sha, or on a tree with src/ or tools/ changed
# against HEAD, HEAD's sha plus the first 12 hex of the git blob id of that
# diff, saved at $1 -- fastchess.sh's name for a dirty candidate (S256). Until
# 2026-10-16 a dirty tree printed only "-dirty", which names no engine; the
# pool may be generated before S082 is committed.
#
# Refused, with nothing printed to stdout, when an untracked file sits under
# src/ or tools/ -- ignored ones included, since src/'s GLOB compiles any
# .cpp there and no diff against HEAD names it; Python's __pycache__ is the
# one exception. The stamp names the tree; binary_line() names the binary.
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

# What actually ran: the sha256 of a binary or script a stage executes, with
# its path. The stamp says which tree; this says which build of it, since a
# binary left from another tree runs as readily as a fresh one.
binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

# A pool against the log of the run that wrote it, before any stage calls it
# complete or reads it. datagen exits 1 on a failed write or a game short; this
# also catches what happens to the files after the run -- a kill, a full disk
# at the last flush, a file touched since. Per leaf: the log's line plays the
# booked games and names this file, the file ends on a newline, holds as many
# rows as the line counts, and every row has datagen's seven fields. One line
# per finding or per leaf checked, on stdout; non-zero on any finding.
#
# A merged pool's log (DEC-296, MERGED_HEADER its first line) is read by the
# merge's `merged <leaf>:` lines, in the same shape -- rows, the games both
# parts give, the file -- and each file's sha256 must also be the one the
# merge wrote. Its datagen lines are part 2's alone and are not read here.
check_pool() {
  local prefix="$1" log="$2" games="$3" failed=0 leaf kind=leaf

  [[ -r "$log" ]] || { echo "check: no log at $log"; return 1; }
  [[ "$(head -n 1 "$log")" == "$MERGED_HEADER" ]] && kind=merged

  for leaf in $LEAVES; do
    local file="$prefix.$leaf" line rows played counted fields

    line=$(grep -E "^$kind $leaf: [0-9]+ rows from [0-9]+ games written to " \
             "$log" | tail -n 1)

    if [[ -z "$line" ]]; then
      echo "check: $log has no '$kind $leaf:' line"
      failed=1
      continue
    fi

    rows=$(sed -E "s/^$kind [a-z]+: ([0-9]+) rows from .*/\1/" <<< "$line")
    played=$(sed -E "s/^$kind [a-z]+: [0-9]+ rows from ([0-9]+) games .*/\1/" \
               <<< "$line")

    if [[ "$played" != "$games" ]]; then
      echo "check: leaf $leaf played $played games of $games booked"
      failed=1
    fi

    if [[ "$line" != *"written to $file, "* ]]; then
      echo "check: the log's leaf $leaf line does not name $file"
      failed=1
    fi

    if [[ ! -f "$file" || ! -s "$file" ]]; then
      echo "check: $file is missing, empty or not a regular file"
      failed=1
      continue
    fi

    if [[ "$kind" == merged &&
          "$line" != *" sha256 $(sha256sum "$file" | cut -d' ' -f1);"* ]]; then
      echo "check: $file's sha256 is not the one its merge wrote"
      failed=1
    fi

    if [[ "$(tail -c 1 "$file" | wc -l)" -ne 1 ]]; then
      echo "check: $file does not end on a newline, so its last row is cut"
      failed=1
    fi

    counted=$(wc -l < "$file")

    if [[ "$counted" -ne "$rows" ]]; then
      echo "check: $file holds $counted rows, its log line $rows"
      failed=1
    fi

    fields=$(awk -F'\t' 'NF != 7 { n++ } END { print n + 0 }' "$file")

    if [[ "$fields" -ne 0 ]]; then
      echo "check: $file has $fields rows without seven fields"
      failed=1
    fi

    echo "check: $kind $leaf, $counted rows of $rows counted, $played games of" \
         "$games, $fields rows without seven fields"
  done

  return "$failed"
}

# The abort rule's reading: the pool's own settings over 600 games, into a
# directory of its own, datagen's closing lines, the games an hour, and the
# rule applied by the reader -- truncated, aborted and uncertified against 1 %
# per walked leaf, terminal printed and not gated. Fails, so the night does
# not start, when the rule says abort.
smoke() {
  [[ -x "$datagen" ]] || { echo "no datagen at $datagen" >&2; return 1; }
  mkdir -p "$OUT/smoke"

  local stamp
  stamp=$(engine_stamp "$OUT/smoke/engine.diff") || return 1
  echo "engine    $stamp"
  binary_line datagen "$datagen"

  local started
  started=$(date +%s)

  "$datagen" --out "$OUT/smoke/pool.tsv" --leaf root,qs,pv --games 600 \
      --nodes "$NODES" --threads "$THREADS" --seed "$SEED" \
      2> "$OUT/smoke/datagen.log"
  local status=$?

  local elapsed=$(( $(date +%s) - started ))

  if (( status != 0 )); then
    cat "$OUT/smoke/datagen.log"
    echo "datagen exited $status"
    return 1
  fi

  check_pool "$OUT/smoke/pool.tsv" "$OUT/smoke/datagen.log" 600 || return 1

  (( elapsed > 0 )) || elapsed=1
  local hours=$(( GAMES * elapsed / 600 / 3600 ))
  cat "$OUT/smoke/datagen.log"
  echo "elapsed $elapsed s, $(( 600 * 3600 / elapsed )) games an hour; the pool's $GAMES at that rate: $hours h"

  python3 "$reader" --smoke "$OUT/smoke/datagen.log" || return 1

  if (( hours >= 18 )); then
    echo "abort: the pool would take $hours h, past the 18 h the rule allows"
    return 1
  fi
}

generate() {
  [[ -x "$datagen" ]] || { echo "no datagen at $datagen" >&2; return 1; }
  mkdir -p "$OUT"
  [[ ! -e "$OUT/pool.tsv.qs" ]] || { echo "$OUT/pool.tsv.qs exists" >&2; return 1; }

  local stamp
  stamp=$(engine_stamp "$OUT/generate.engine.diff") || return 1

  {
    echo "S082 pool"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    binary_line datagen "$datagen"
    echo "command   datagen --out $OUT/pool.tsv --leaf root,qs,pv --games $GAMES --nodes $NODES --threads $THREADS --seed $SEED"
  } > "$OUT/generate.log"

  local started
  started=$(date +%s)

  "$datagen" --out "$OUT/pool.tsv" --leaf root,qs,pv --games "$GAMES" \
      --nodes "$NODES" --threads "$THREADS" --seed "$SEED" \
      2>> "$OUT/generate.log"
  local status=$?

  echo "elapsed   $(( $(date +%s) - started )) s" >> "$OUT/generate.log"

  # Checked whatever datagen said, so a failed run's log carries how far the
  # files got; DONE needs both.
  local report checked
  report=$(check_pool "$OUT/pool.tsv" "$OUT/generate.log" "$GAMES")
  checked=$?

  {
    echo "$report"
    (( status == 0 )) || echo "check: datagen exited $status"
    for leaf in $LEAVES; do
      if [[ -f "$OUT/pool.tsv.$leaf" ]]; then
        echo "$leaf      $(wc -l < "$OUT/pool.tsv.$leaf") rows, sha256 $(sha256sum "$OUT/pool.tsv.$leaf" | cut -d' ' -f1)"
      fi
    done
    echo "finished  $(date -Iseconds)"
  } >> "$OUT/generate.log"

  echo "$report"

  if (( status != 0 )); then
    echo "datagen exited $status: its own FAILED line is in $OUT/generate.log"
    return 1
  fi

  (( checked == 0 )) || return 1
}

# A pool checked against its log on demand -- after the night, before compare,
# or on a copy. Arguments default to the pool `generate` writes.
verify() {
  check_pool "${1:-$OUT/pool.tsv}" "${2:-$OUT/generate.log}" "${3:-$GAMES}"
}

compare() {
  [[ -x "$tuner" ]] || { echo "no tuner at $tuner" >&2; return 1; }
  for leaf in $LEAVES; do
    [[ -r "$OUT/pool.tsv.$leaf" ]] || { echo "no pool at $OUT/pool.tsv.$leaf" >&2; return 1; }
  done

  local log="$OUT/compare.log" stamp
  stamp=$(engine_stamp "$OUT/compare.engine.diff") || return 1

  {
    echo "S082 compare"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    binary_line tuner "$tuner"
    binary_line split "$split"
    binary_line reader "$reader"
    echo "python    $(python3 --version 2>&1)"
  } > "$log"

  # The pool is read only as its own log describes it.
  check_pool "$OUT/pool.tsv" "$OUT/generate.log" "$GAMES" >> "$log" ||
    { echo "the pool fails its checks; see $log"; return 1; }

  python3 "$split" split --seed "$SEED" --validation-games "$VALIDATION_GAMES" \
      "$OUT/pool.tsv.root" "$OUT/pool.tsv.qs" "$OUT/pool.tsv.pv" >> "$log" 2>&1 || return 1

  local views=(--validation "$OUT/pool.tsv.qs.validation"
               --validation "$OUT/pool.tsv.root.validation"
               --validation "$OUT/pool.tsv.pv.validation")

  # The eight cells under seed 1, then the control under seeds 1 to 5.
  local cells=()
  for leaf in $LEAVES; do
    for density in $DENSITIES; do
      [[ "$leaf $density" == "root 0" ]] && continue
      cells+=("$leaf $density 1")
    done
  done
  for seed in $CONTROL_SEEDS; do
    cells+=("root 0 $seed")
  done

  for cell in "${cells[@]}"; do
    read -r leaf density seed <<< "$cell"
    local name="${leaf}_${density}_s${seed}"

    python3 "$split" sample --in "$OUT/pool.tsv.$leaf.train" "${views[@]}" \
        --rows "$ROWS" --per-game "$density" --seed "$seed" \
        --out "$OUT/cand_$name.tsv" >> "$log" 2>&1 || return 1

    for view in qs root; do
      # shellcheck disable=SC2086
      "$tuner" --data "$OUT/cand_$name.tsv" \
          --holdout "$OUT/pool.tsv.$view.validation" \
          --out "$OUT/fit_${name}_$view.hpp" $FIT \
          > "$OUT/fit_${name}_$view.log" 2>&1 || return 1
    done
  done

  python3 "$reader" "$OUT" | tee -a "$log" || return 1
  echo "finished  $(date -Iseconds)" >> "$log"
}

# DEC-296. The pool `generate` was writing when a power cut stopped it (part
# 1) and the top-up that completes it (part 2: $OUT/topup, run by hand under
# TOPUP_SEED, with its own log) become the pool `compare` reads, at the names
# it reads: $OUT/pool.tsv.* and $OUT/generate.log. Moving part 1's files and
# log aside to $OUT/part1/ is the smaller change than teaching compare, verify
# and the reader another prefix: nothing downstream changes but
# check_pool()'s merged branch, and compare.log names the paths its
# pre-registration names. They move once, by mv on one filesystem, with their
# sha256 written first to $OUT/part1/originals.sha256 and checked on every
# run; nothing here writes to them. Part 2 passes check_pool() against its own
# log before anything is written. The merger cuts part 1, checks that part
# 2's rows give its log's figures, shifts part 2's game ids by one offset,
# writes the merged files under .merging names and reads them back; the
# merged log is composed beside them, all four are renamed, and check_pool()
# reads the result by MERGED_HEADER.
#
# Re-runnable: a failed run leaves part 1 in $OUT/part1/ and no merged pool --
# the files it made are removed and its log kept as $OUT/merge.failed.log --
# so the next run starts where it did. A merged pool is never overwritten.
merge() {
  local part1="$OUT/part1" topup="$OUT/topup" log="$OUT/generate.log"
  local topup_log="$OUT/topup/topup.log"
  local originals="generate.log pool.tsv.root pool.tsv.qs pool.tsv.pv"
  local leaf name booked part2_check checked merged report

  [[ -r "$merger" ]] || { echo "no merger at $merger"; return 1; }
  [[ -r "$topup_log" ]] || { echo "no top-up log at $topup_log"; return 1; }
  if ! grep -qE '^exit +0$' "$topup_log"; then
    echo "$topup_log has no 'exit 0' line: the top-up has not finished, or failed"
    return 1
  fi

  # Part 1 aside, once; afterwards only checked.
  if [[ ! -e "$part1/originals.sha256" ]]; then
    if [[ -e "$part1" ]]; then
      echo "$part1 exists without originals.sha256: a move was interrupted; resolve it by hand"
      return 1
    fi
    if [[ "$(head -n 1 "$log" 2>/dev/null)" != "S082 pool" ]]; then
      echo "$log is not generate's own log: no part 1 to move"
      return 1
    fi
    for name in $originals; do
      [[ -f "$OUT/$name" ]] || { echo "no $OUT/$name to move"; return 1; }
    done
    mkdir "$part1" || return 1
    if [[ "$(stat -c %d "$OUT")" != "$(stat -c %d "$part1")" ]]; then
      rmdir "$part1"
      echo "$part1 is on another filesystem than $OUT: mv would copy"
      return 1
    fi
    # shellcheck disable=SC2086
    if ! (cd "$OUT" && sha256sum $originals) > "$part1/originals.sha256.tmp"; then
      rm -f "$part1/originals.sha256.tmp"
      rmdir "$part1"
      return 1
    fi
    for name in $originals; do
      mv "$OUT/$name" "$part1/$name" || return 1
    done
    mv "$part1/originals.sha256.tmp" "$part1/originals.sha256" || return 1
    echo "moved     part 1's $originals to $part1/, their sha256 written first to $part1/originals.sha256"
  fi

  if ! (cd "$part1" && sha256sum --quiet -c originals.sha256); then
    echo "check: part 1's files in $part1 are not the ones whose sha256 was written before the move"
    return 1
  fi

  for leaf in $LEAVES; do
    if [[ -e "$OUT/pool.tsv.$leaf" ]]; then
      echo "$OUT/pool.tsv.$leaf exists: a merged pool is never overwritten"
      return 1
    fi
    rm -f "$OUT/pool.tsv.$leaf.merging"
  done
  if [[ -e "$log" ]]; then
    echo "$log exists: a merged pool is never overwritten"
    return 1
  fi
  rm -f "$log.merging"

  # Part 2 against its own log, at the games its own command booked.
  booked=$(sed -nE 's/^datagen: ([0-9]+) games, .*/\1/p' "$topup_log" | head -n 1)
  [[ -n "$booked" ]] || { echo "$topup_log has no 'datagen:' line"; return 1; }
  part2_check=$(check_pool "$topup/pool.tsv" "$topup_log" "$booked")
  checked=$?
  echo "$part2_check"
  if (( checked != 0 )); then
    echo "part 2 fails its checks against its own log"
    return 1
  fi

  {
    echo "$MERGED_HEADER"
    echo "merged    $(date -Iseconds)"
    binary_line merger "$merger"
    binary_line recipe "$root/adocs/data/S082_recipe.sh"
    echo "python    $(python3 --version 2>&1)"
    echo "part 1    $part1/generate.log, the first run's own log, moved there unchanged; verbatim:"
    sed 's/^/    /' "$part1/generate.log"
    echo "part 1    stopped by a power cut before datagen's closing lines; its files end on a NUL tail"
    echo "part 1    sha256 written before the move to $part1/originals.sha256, checked on this run:"
    sed 's/^/    /' "$part1/originals.sha256"
    echo "part 2    $topup_log, the top-up's own log; verbatim:"
    sed 's/^/    /' "$topup_log"
    echo "part 2    checked against its own log, $booked games booked:"
    sed 's/^/    /' <<< "$part2_check"
  } > "$log.merging"

  python3 "$merger" merge --games "$GAMES" --seed "$SEED" \
      --topup-seed "$TOPUP_SEED" --nodes "$NODES" \
      "$part1/pool.tsv" "$part1/generate.log" "$topup/pool.tsv" "$topup_log" \
      "$OUT/pool.tsv" | tee -a "$log.merging"
  merged=${PIPESTATUS[0]}

  if (( merged != 0 )); then
    mv "$log.merging" "$OUT/merge.failed.log"
    echo "the merger refused; its log is $OUT/merge.failed.log"
    return 1
  fi

  for leaf in $LEAVES; do
    mv "$OUT/pool.tsv.$leaf.merging" "$OUT/pool.tsv.$leaf" || return 1
  done
  mv "$log.merging" "$log" || return 1

  report=$(check_pool "$OUT/pool.tsv" "$log" "$GAMES")
  checked=$?
  { echo "$report"; echo "finished  $(date -Iseconds)"; } >> "$log"
  echo "$report"

  if (( checked != 0 )); then
    # This run's own files: they did not exist when it started.
    for leaf in $LEAVES; do
      rm -f "$OUT/pool.tsv.$leaf"
    done
    mv "$log" "$OUT/merge.failed.log"
    echo "the merged pool fails its checks: removed, its log kept as $OUT/merge.failed.log"
    return 1
  fi

  # The pool needed this stage because a power cut lost its page cache, so
  # DONE waits until the merged files and the log are on disk.
  sync
}

case "$stage" in
  smoke)
    if smoke; then echo "S082-SMOKE-DONE"; else echo "S082-SMOKE-FAILED"; exit 1; fi
    ;;
  generate)
    if generate; then echo "S082-GENERATE-DONE"; else echo "S082-GENERATE-FAILED"; exit 1; fi
    ;;
  compare)
    if compare; then echo "S082-COMPARE-DONE"; else echo "S082-COMPARE-FAILED"; exit 1; fi
    ;;
  verify)
    if verify "${@:2}"; then echo "S082-VERIFY-DONE"; else echo "S082-VERIFY-FAILED"; exit 1; fi
    ;;
  merge)
    if merge; then echo "S082-MERGE-DONE"; else echo "S082-MERGE-FAILED"; exit 1; fi
    ;;
  *)
    echo "usage: $0 smoke|generate|compare|verify [PREFIX LOG GAMES]|merge" >&2
    exit 2
    ;;
esac
