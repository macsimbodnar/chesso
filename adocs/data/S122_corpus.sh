#!/usr/bin/env bash
# S122's corpus, prepared from head's own games (DEC-344 (1)): the 150000-game pool
# .tuning/s122/corpus/ generated with `--leaf root,qs,pv --quiet-limit 30000`, cut by
# S083's recipe into the training corpus and the fixed held-out set
# adocs/data/S122_screen.sh fits and reads.
#
#   adocs/data/S122_corpus.sh seeds      # DEC-349's seeds against every logged run
#   adocs/data/S122_corpus.sh merge      # a stopped pool and its top-up joined (DEC-296)
#   adocs/data/S122_corpus.sh verify     # the pool against its own log
#   adocs/data/S122_corpus.sh filter     # the recipe's band, offline
#   adocs/data/S122_corpus.sh split      # the fixed set, 12000 games
#   adocs/data/S122_corpus.sh sample     # every training game at 46 qs rows
#   adocs/data/S122_corpus.sh k          # K on the new rows
#   adocs/data/S122_corpus.sh record     # prepared.txt, the screen's input
#   adocs/data/S122_corpus.sh all        # the six in order; resumable
#   adocs/data/S122_corpus.sh adopt      # a preparation before S333 stamped with its pool
#   adocs/data/S122_corpus.sh smoke      # all six on a slice of under 20000 rows, and the refusals
#
# Each stage prints S122-CORPUS-<STAGE>-DONE or -FAILED as its last line. POOL
# defaults to .tuning/s122/corpus, OUT to .tuning/s122/prepared (DEC-235). Only
# `merge` writes into POOL: it moves part 1's generate.log and pool.tsv.* (and its
# games.tsv, since DEC-349) into
# POOL/part1/ beside their originals.sha256, writes the merged pool.tsv.*, games.tsv
# (S332) and generate.log by way of .merging files, and keeps a refused run's log as
# merge.failed.log. No other stage writes there.
#
# Since S333 a preparation names the pool it was made from: verify.txt and each .done
# marker in OUT carry a `pool` line, POOL/generate.log's sha256 when the file was
# written, and filter, split, sample, k and record refuse OUT when any of them names
# another pool or none (check_prepared), and each refuses when an earlier stage's record
# is absent (needs); verify refuses such an OUT before it removes anything. `adopt`
# stamps a preparation made before S333 after the checks above adopt(), which name
# what they do not prove; ADOPT_DRY=1 checks and writes nothing.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-11-02 in S122's phase 1, while the pool was
# still generating, before any of it ran on the pool. Nothing below changes once
# `filter` has run on it; a change is a dated section in the step file.
# Amended 2026-11-04 (DEC-349): the pool is generated again; "AMENDED", below.
# ============================================================================
#
# S083'S RECIPE, AND EACH DEPARTURE FROM IT, STATED:
#   - The pool: datagen at `ba1cad6` (bench 5482277), `--games 150000 --nodes 100000
#     --threads 12 --seed 2026110203 --leaf root,qs,pv --quiet-limit 30000`, every
#     other setting datagen's default, as S082's. DEPARTURE (DEC-344 (1)): the quiet
#     limit is 30000 and not 1000, so that the rows past the recipe's band are kept and
#     can be read. datagen records a position only when |its search score| < the limit
#     (tools/datagen.cpp, inside_band), and column 3 of every view's row is that
#     search score, White relative (sample_leaf()), so `filter` keeping |column 3| <
#     1000 is the recipe's band exactly, row for row; the rows past it go to
#     pool_past.tsv.* and into no corpus.
#   - A top-up: if the pool did not finish -- a power cut, a crash -- S082's merge is
#     the precedent (DEC-296, adocs/data/S082_pool_merge.py), and it is the
#     coordinator's: `verify` refuses a pool whose log lacks S122-CORPUS-DONE.
#     2026-11-03, a dated section in the step file ("the corpus cut for a shutdown,
#     and its top-up"), before `filter` ran: the pool was stopped at 102412 games and
#     `merge` below joins it with a 47588-game top-up under seed 2026300203.
#   - The fixed held-out set: S082's rule, 12000 games chosen by
#     tools/corpus_split.py split before anything is sampled, every row of those
#     games, the qs view primary. DEPARTURE: drawn from this pool and not S082's, under
#     seed 20261102, because the fit is on head's games and DEC-344 (1) reads the
#     family on head's games; S082's set stays S082's, read only by DEC-344 (1)'s side
#     reading (a).
#   - The training corpus: every game of the training side at 46 qs rows a game
#     (S083's top rung), `corpus_split.py sample --per-game 46 --seed 1`, the three
#     views of the set named so no corpus row stands on a set position, the rows the
#     capacity counts (S083_corpus.py capacity). No dedupe, S082's setting.
#   - K: fitted on the new rows at the shipped constants, a zero-epoch tuner run
#     against the set, and recorded; adocs/data/S122_screen.sh fits it again per sample
#     at the screen's margin.
#   - The split is S066's by game, which `sample` and the tuner's --holdout keep.
#
# THE SMOKE: `smoke` runs the six stages on the first games of
# .tuning/s122/smoke/pool.tsv.* (the 12000-game sample's pool, head's engine) whose
# root rows stay under 20000, 20 games held out, under OUT=.tuning/s122/phase1/
# corpus_smoke: the pipeline's shape, no reading taken. The main checkout's .tuning is read through TUNING, so a
# worktree runs it with TUNING=/home/max/ws/chesso/.tuning.
# (So until S333, 2026-11-06. That sample is a pool before S331, which `verify` now
# refuses; the slice is S331's measured run's since, AMENDED 2026-11-06, below.)
#
# MACHINE TIME, inferred from S082's and S083's stages on pools of this size: verify
# and filter about 10 min (three 13 M-row views read twice), split about 10 min, sample
# about 5 min, K one zero-epoch load, about 3 min at 12 threads. About 30 min; it
# shares the machine with nothing heavier than reads.
#
# AMENDED 2026-11-04, DEC-349 -- before the new pool exists, so before `filter` ran
# on it; the step file's section "2026-11-04: the corpus generated again (DEC-349)".
# S331 found that datagen, from S207 (DEC-173) to S331, ended a game as a draw at any
# two-fold repetition while the search played on: on 2400 games at these settings
# 22.79 % of games and 19.74 % of root rows carried a draw the laws would not give
# (adocs/data/S331_reading.txt). The pool generated under this pre-registration, its
# top-up, its preparation and the side sample were written so. DEC-349 voids them,
# moved aside to .tuning/s122/void_twofold/, and generates the corpus again. What
# changed, and nothing else:
#   - datagen: S331's, a game ending on a repetition only at a third occurrence,
#     built at a HEAD that holds S331 and frozen in POOL by adocs/data/S122_generate.sh
#     freeze, its sha256 in the log as before. That script is the run.
#   - the seeds: CORPUS_SEED 2026500349, was 2026110203; TOPUP_SEED 2026800349 for a
#     top-up if the run is stopped, was 2026300203; the side sample's 2026700349, was
#     2026260203. `seeds` holds each range, games-wide and per worker, apart from
#     every run a log names and from each other.
#   - `--games-out games.tsv` on the command line, recording only: one line a game in
#     a file of its own (tools/datagen.cpp), so the log's `repetition:` line can be
#     checked against the files. The rows are written as before.
#   - `merge` refuses a part whose datagen predates S331: the top-up's log must hold
#     datagen's closing `repetition:` line, and part 1's datagen, stopped before its
#     closing lines, must be the top-up's by sha256. It moves part 1's games.tsv with
#     part 1's other files.
# The size, 150000 games, the settings, S083's recipe and its departures, the fixed
# set's split seed 20261102, 46 qs rows a game and K stand as DEC-344 and DEC-345 have
# them.
#
# AMENDED 2026-11-06, S333 -- the guards, not the recipe, written after the regenerated
# pool was prepared; that preparation's data files stay as they are, and `adopt` adds
# lines to its verify.txt and markers only. The pool DEC-349 voided was kept out of
# the new preparation only by moving its directories aside (S122's step file, "What
# still guards only by the move"). Since S333:
#   - `verify` refuses a one-run pool whose log holds no `repetition:` line, a pool
#     written before S331, as `merge` refuses such a top-up;
#   - verify.txt and every .done marker name POOL's generate.log by sha256, and
#     verify, filter, split, sample, k and record refuse a preparation that names
#     another pool, or none, and each stage after verify one that lacks an earlier
#     stage's record; verify refuses before it removes anything; `adopt` stamps a
#     preparation made before S333, its checks and what they leave unproved below;
#   - `smoke` slices S331's measured run, .tuning/s331/measure (2400 games at these
#     settings by S331's datagen), not the research pass's sample, and then plays
#     each refusal on a planted copy.
# Nothing in the recipe, its seeds, its sizes or K changes.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
# This script, which `smoke` runs its cases by unless SMOKE_AGAINST names another.
self="$(cd "$(dirname "$0")" && pwd)/$(basename "$0")"
# The main checkout's .tuning, which a worktree does not have (DEC-235).
TUNING="${TUNING:-$root/.tuning}"
POOL="${POOL:-$TUNING/s122/corpus}"
OUT="${OUT:-$TUNING/s122/prepared}"
SPLIT_SEED=20261102
VALIDATION_GAMES="${VALIDATION_GAMES:-12000}"
GAMES="${GAMES:-150000}"
PER_GAME=46
BAND=1000
THREADS=12
VIEWS_ALL="root qs pv"

split="$root/tools/corpus_split.py"
capacity_helper="$root/adocs/data/S083_corpus.py"
tuner="${TUNER:-$root/build/tools/tuner}"
merger="$root/adocs/data/S082_pool_merge.py"
# The merged pool's games file and its `repetition:` line, and the read of both (S332).
reader="$root/adocs/data/S331_measure.py"

# The merge's runs (DEC-296's form). CORPUS_SEED and TOPUP_SEED are overridable for
# a smoke and nothing else; the merger refuses a log whose seed is not the one given.
# DEC-349's fresh seeds, the void pool's having been 2026110203 and 2026300203;
# adocs/data/S122_generate.sh reads these lines, so the runs play the numbers checked
# here.
CORPUS_SEED="${CORPUS_SEED:-2026500349}"
TOPUP_SEED="${TOPUP_SEED:-2026800349}"
NODES=100000
TOPUP="${TOPUP:-$TUNING/s122/topup}"
# adocs/data/S122_screen.sh's side sample, DEC-345 (5): a top-up's seeds meet neither
# it nor the corpus's. DEC-349's fresh seed, the void sample's having been 2026260203;
# `seeds` refuses one S122_screen.sh does not share.
SAMPLE_SEED=2026700349
SAMPLE_GAMES=12000
SAMPLE_DIR="${SAMPLE_DIR:-$TUNING/s122/sample}"
# The first line of a merged pool's log. Such a log holds each run's own log
# verbatim, indented four spaces; S122_screen.sh sample reads its runs from it.
MERGED_HEADER="S122 corpus, merged from two datagen runs (DEC-296); not one run's log"
# The stages' markers in OUT, each its file's sha256 and, since S333, its pool's.
view_markers="pool.tsv.root.done pool.tsv.qs.done pool.tsv.pv.done"
markers="$view_markers corpus.tsv.done"
# The recipe's band, |column 3| < BAND, to `keep` and the rest to `past`: filter's
# program, which adopt runs again into sha256sum.
band_filter='{ v = $3 + 0; if (v < band && v > -band) print > keep; else print > past }'
# Set by check_pool_log.
pool_merged=0

say() { echo "$*"; }

binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

# A merged pool against the merge's own `merged <view>:` lines in LOG: each plays
# GAMES and names POOL's file, and the file holds the rows the line counts, ends on
# a newline and has the sha256 the merge wrote. One line a view; non-zero on any
# finding. Since S332 POOL/games.tsv too, against the `merged games:` line: a merged
# pool without one, merged before S332, is a finding.
check_merged() {
  local log="$1" v line rows played sha same file counted failed=0
  for v in $VIEWS_ALL; do
    file="$POOL/pool.tsv.$v"
    line=$(grep -E "^merged $v: [0-9]+ rows from [0-9]+ games written to " "$log" | tail -n 1)
    [[ -n "$line" ]] || { say "check: $log has no 'merged $v:' line"; failed=1; continue; }
    rows=$(sed -E 's/^merged [a-z]+: ([0-9]+) rows .*/\1/' <<< "$line")
    played=$(sed -E 's/^merged [a-z]+: [0-9]+ rows from ([0-9]+) games .*/\1/' <<< "$line")
    sha=$(sed -nE 's/.*, sha256 ([0-9a-f]{64});.*/\1/p' <<< "$line")
    [[ "$played" == "$GAMES" ]] || { say "check: merged $v plays $played games of $GAMES"; failed=1; }
    [[ "$line" == *"written to $file, "* ]] || { say "check: the merge's $v line does not name $file"; failed=1; }
    [[ -f "$file" && -s "$file" ]] || { say "check: $file is missing or empty"; failed=1; continue; }
    if [[ -n "$sha" && "$(sha256sum "$file" | cut -d' ' -f1)" == "$sha" ]]; then
      same="the merge's"
    else
      same="not the merge's"
      say "check: $file's sha256 is not the one its merge wrote"
      failed=1
    fi
    [[ "$(tail -c 1 "$file" | wc -l)" -eq 1 ]] || { say "check: $file does not end on a newline"; failed=1; }
    counted=$(wc -l < "$file")
    [[ "$counted" -eq "$rows" ]] || { say "check: $file holds $counted rows, the merge's line $rows"; failed=1; }
    say "check: merged $v, $counted rows of $rows, $played games of $GAMES, sha256 $same"
  done
  file="$POOL/games.tsv"
  line=$(grep -E "^merged games: [0-9]+ games written to " "$log" | tail -n 1)
  if [[ -z "$line" ]]; then
    say "check: $log has no 'merged games:' line: merged before S332, the pool has no one games file"
    return 1
  fi
  rows=$(sed -E 's/^merged games: ([0-9]+) games .*/\1/' <<< "$line")
  sha=$(sed -nE 's/.*, sha256 ([0-9a-f]{64});.*/\1/p' <<< "$line")
  [[ "$line" == *"written to $file, "* ]] || { say "check: the merge's games line does not name $file"; failed=1; }
  [[ -f "$file" && -s "$file" ]] || { say "check: $file is missing or empty"; return 1; }
  if [[ -n "$sha" && "$(sha256sum "$file" | cut -d' ' -f1)" == "$sha" ]]; then
    same="the merge's"
  else
    same="not the merge's"
    say "check: $file's sha256 is not the one its merge wrote"
    failed=1
  fi
  [[ "$(tail -c 1 "$file" | wc -l)" -eq 1 ]] || { say "check: $file does not end on a newline"; failed=1; }
  counted=$(wc -l < "$file")
  [[ "$counted" -eq "$rows" ]] || { say "check: $file holds $counted games, the merge's line $rows"; failed=1; }
  say "check: merged games, $counted lines of $rows, sha256 $same"
  return "$failed"
}

# DEC-349's seeds against every datagen run a log names: S331_measure.sh's
# check_seeds, read over every text file under 20 MB and not only *.log. A run is
# named by datagen's own settings line, `datagen: G games, N nodes per move, T
# threads, seed S, ...`, anywhere under SEED_LOGS, by default TUNING and adocs/data.
# Each of this pool's three runs -- the corpus, CORPUS_SEED and GAMES; a top-up,
# TOPUP_SEED and at most GAMES, its games unknown before a stop; the side sample,
# SAMPLE_SEED and SAMPLE_GAMES -- meets none of them, nor the other two, games-wide,
# [seed, seed + games), nor per worker, [seed, seed + threads): datagen seeds worker w
# from seed + w (play_games()). The three runs' own directories, POOL, TOPUP and
# SAMPLE_DIR, are left out of the search, so the check reads the same after their
# runs have started; the void runs' logs, in .tuning/s122/void_twofold/, are in it,
# more ranges to avoid. Refused when no run is read at all. Writes nothing.
seeds() {
  local dirs logged n failed=0 ours name seed games threads met oname oseed ogames othreads screen_seed
  read -ra dirs <<< "${SEED_LOGS:-$TUNING $root/adocs/data}"
  logged=$(find "${dirs[@]}" \( -path "$POOL" -o -path "$TOPUP" -o -path "$SAMPLE_DIR" \) -prune \
             -o -type f -size -20M ! -name '*.tsv*' -print0 2>/dev/null |
           xargs -0 -r grep -IhoE 'datagen: [0-9]+ games, [0-9]+ nodes per move, [0-9]+ threads, seed [0-9]+' |
           sed -E 's/^datagen: ([0-9]+) games, [0-9]+ nodes per move, ([0-9]+) threads, seed ([0-9]+)$/\3 \1 \2/' |
           sort -u -k1,1n -k2,2n -k3,3n)
  n=$(grep -c . <<< "$logged")
  (( n > 0 )) || { say "refused: no datagen settings line under ${dirs[*]}, so no range was checked"; return 1; }
  say "seeds     $n runs named by datagen's settings line under ${dirs[*]}, $POOL, $TOPUP and $SAMPLE_DIR left out:"
  while read -r oseed ogames othreads; do
    say "seeds       seed $oseed, $ogames games, $othreads threads: [$oseed, $(( oseed + ogames ))), workers [$oseed, $(( oseed + othreads )))"
  done <<< "$logged"
  ours="corpus $CORPUS_SEED $GAMES $THREADS"$'\n'"top-up $TOPUP_SEED $GAMES $THREADS"$'\n'"sample $SAMPLE_SEED $SAMPLE_GAMES $THREADS"
  while read -r name seed games threads; do
    met=""
    while read -r oseed ogames othreads; do
      (( seed < oseed + ogames && oseed < seed + games )) && met+=" [$oseed, $(( oseed + ogames )))"
      (( seed < oseed + othreads && oseed < seed + threads )) && met+=" workers [$oseed, $(( oseed + othreads )))"
    done <<< "$logged"
    while read -r oname oseed ogames othreads; do
      [[ "$oname" == "$name" ]] && continue
      (( seed < oseed + ogames && oseed < seed + games )) && met+=" the $oname's [$oseed, $(( oseed + ogames )))"
      (( seed < oseed + othreads && oseed < seed + threads )) && met+=" the $oname's workers"
    done <<< "$ours"
    if [[ -n "$met" ]]; then
      say "seeds     refused: the $name's [$seed, $(( seed + games ))) or its workers [$seed, $(( seed + threads ))) meet$met"
      failed=1
    else
      say "seeds     $name [$seed, $(( seed + games ))), workers [$seed, $(( seed + threads ))): meets none of the $n, nor the other two"
    fi
  done <<< "$ours"
  screen_seed=$(sed -n 's/^SAMPLE_SEED=\([0-9][0-9]*\)$/\1/p' "$root/adocs/data/S122_screen.sh")
  if [[ "$screen_seed" != "$SAMPLE_SEED" ]]; then
    say "seeds     refused: adocs/data/S122_screen.sh's SAMPLE_SEED is '$screen_seed', this script's $SAMPLE_SEED"
    failed=1
  fi
  return "$failed"
}

# verify's checks of POOL's log, which `adopt` makes too: the pool finished; a merged
# pool's files are the ones its merge wrote; and, since S333, a one-run pool's log holds
# datagen's closing `repetition:` line, which only S331's datagen prints, as `merge`
# asks of a top-up's (DEC-349). A merged pool's parts were held to that at its merge.
check_pool_log() {
  pool_merged=0
  [[ -f "$POOL/generate.log" ]] || { say "no $POOL/generate.log"; return 1; }
  grep -q '^S122-CORPUS-DONE$' "$POOL/generate.log" || {
    say "refused: $POOL/generate.log has no S122-CORPUS-DONE: the pool is still generating, or failed; a top-up is S082's merge (DEC-296), the coordinator's"
    return 1
  }
  if [[ "$(head -n 1 "$POOL/generate.log")" == "$MERGED_HEADER" ]]; then
    pool_merged=1
    check_merged "$POOL/generate.log" ||
      { say "refused: the merged pool is not the one its merge wrote"; return 1; }
  else
    grep -q '^repetition: ' "$POOL/generate.log" ||
      { say "refused: $POOL/generate.log has no 'repetition:' line: its datagen predates S331, whose two-fold draws DEC-349 voids"; return 1; }
  fi
}

# The pool's own lines verify.txt records: a run's engine, datagen and command lines,
# or for a merged pool both runs', from their verbatim logs, and the merge's.
pool_lines() {
  if (( pool_merged )); then
    echo "$MERGED_HEADER"
    grep -E '^(    )?(engine|datagen|command)' "$POOL/generate.log"
    grep -E '^merged [a-z]+: ' "$POOL/generate.log"
  else
    grep -E '^(engine|datagen|command)' "$POOL/generate.log"
  fi
}

# The pool FILE names on its `pool` line, by its log's sha256; empty when it names none.
named_pool() { sed -n 's/^pool  *sha256 \([0-9a-f]\{64\}\)  .*/\1/p' "$1" | head -n 1; }

# S333: OUT is POOL's preparation. verify.txt and every marker present must name, on
# their `pool` line, the pool POOL/generate.log is now; none is skipped, so a stage
# refuses a directory holding any file of another pool's preparation. One line a file
# found; non-zero on any.
check_prepared() {
  local now name named failed=0
  [[ -f "$POOL/generate.log" ]] || { say "refused: no $POOL/generate.log, so no pool to hold $OUT to"; return 1; }
  now=$(sha256sum "$POOL/generate.log" | cut -d' ' -f1)
  for name in verify.txt $markers; do
    [[ -f "$OUT/$name" ]] || continue
    named=$(named_pool "$OUT/$name")
    if [[ -z "$named" ]]; then
      say "refused: $OUT/$name names no pool: written before S333; \`adopt\` stamps a preparation whose files are this pool's"
      failed=1
    elif [[ "$named" != "$now" ]]; then
      say "refused: $OUT/$name was prepared from the pool whose generate.log has sha256 $named, and $POOL/generate.log's is $now: another pool's preparation"
      failed=1
    fi
  done
  return "$failed"
}

# S333: the records a stage reads, each written by the stage before it -- verify.txt by
# verify, the view markers by filter, split.txt by split, corpus.tsv.done by sample,
# k.txt by k. A stage refuses when any it names is absent, then holds OUT to POOL
# (check_prepared), so markers removed by hand do not let a stage run on another pool's
# files. Only verify, the first, runs on an empty OUT.
needs() {
  local name by
  for name in "$@"; do
    [[ -f "$OUT/$name" ]] && continue
    case "$name" in
      verify.txt) by=verify ;;
      pool.tsv.*.done) by=filter ;;
      split.txt) by=split ;;
      corpus.tsv.done) by=sample ;;
      *) by=k ;;
    esac
    say "refused: no $OUT/$name: run $by first"
    return 1
  done
  check_prepared
}

# verify.txt is written only when every check below has passed, as a .tmp renamed
# last, and a run's old one is removed first: `filter` reads its presence as verify's
# pass, so a refusal leaves none. Since S333 it names the pool it read, and OUT is held
# to POOL before anything is removed, so verify never replaces the verify.txt of
# another pool's preparation, or of one made before S333 that `adopt` must read.
verify() {
  local v rows games bad record="$OUT/verify.txt"
  check_pool_log || return 1
  check_prepared || return 1
  rm -f "$record" "$record.tmp"
  {
    echo "S122 corpus verify, $(date -Iseconds)"
    binary_line pool "$POOL/generate.log"
    pool_lines
  } > "$record.tmp"
  for v in $VIEWS_ALL; do
    [[ -f "$POOL/pool.tsv.$v" ]] || { say "no $POOL/pool.tsv.$v"; return 1; }
    read -r rows games bad < <(awk -F'\t' '{ if (NF != 7) bad++; seen[$5] = 1 }
        END { n = 0; for (g in seen) n++; print NR, n, bad + 0 }' "$POOL/pool.tsv.$v")
    echo "view $v: $rows rows, $games games, $bad rows without seven fields, sha256 $(sha256sum "$POOL/pool.tsv.$v" | cut -d' ' -f1)" | tee -a "$record.tmp"
    (( rows > 0 )) || { say "refused: $v holds no row"; return 1; }
    (( bad == 0 )) || { say "refused: $v has rows without seven fields"; return 1; }
    (( games <= GAMES )) || { say "refused: $v names $games games, more than the $GAMES played"; return 1; }
  done
  mv "$record.tmp" "$record"
}

# OUT/NAME.done for OUT/NAME: the file's sha256, then, since S333, the pool's line.
write_marker() {
  { sha256sum "$OUT/$1" | cut -d' ' -f1; binary_line pool "$POOL/generate.log"; } > "$OUT/$1.done.tmp" &&
    mv "$OUT/$1.done.tmp" "$OUT/$1.done"
}

filter() {
  local v kept past
  needs verify.txt || return 1
  for v in $VIEWS_ALL; do
    if [[ -f "$OUT/pool.tsv.$v.done" ]]; then say "$v filtered before"; continue; fi
    awk -F'\t' -v band="$BAND" -v keep="$OUT/pool.tsv.$v" -v past="$OUT/pool_past.tsv.$v" "$band_filter" \
        "$POOL/pool.tsv.$v" || { say "the filter failed on $v"; return 1; }
    kept=$(wc -l < "$OUT/pool.tsv.$v")
    past=$(wc -l < "$OUT/pool_past.tsv.$v" 2>/dev/null || echo 0)
    echo "filter $v: |column 3| < $BAND keeps $kept rows, $past past it" | tee -a "$OUT/filter.txt"
    write_marker "pool.tsv.$v" || return 1
  done
}

split_stage() {
  needs verify.txt $view_markers || return 1
  if [[ -f "$OUT/split.txt" ]] && [[ -f "$OUT/pool.tsv.qs.validation" ]]; then
    say "split before"; cat "$OUT/split.txt"; return 0
  fi
  python3 "$split" split --seed "$SPLIT_SEED" --validation-games "$VALIDATION_GAMES" \
      "$OUT/pool.tsv.root" "$OUT/pool.tsv.qs" "$OUT/pool.tsv.pv" | tee "$OUT/split.txt"
  (( PIPESTATUS[0] == 0 )) || { rm -f "$OUT/split.txt"; say "the split refused"; return 1; }
}

sample() {
  local rows
  [[ -f "$OUT/pool.tsv.qs.train" ]] || { say "refused: run split first"; return 1; }
  needs verify.txt $view_markers split.txt || return 1
  if [[ -f "$OUT/corpus.tsv.done" ]]; then say "sampled before"; return 0; fi
  python3 "$capacity_helper" capacity --in "$OUT/pool.tsv.qs.train" \
      --validation "$OUT/pool.tsv.qs.validation" --validation "$OUT/pool.tsv.root.validation" \
      --validation "$OUT/pool.tsv.pv.validation" --per-game "$PER_GAME" | tee "$OUT/capacity.txt"
  (( PIPESTATUS[0] == 0 )) || { say "the capacity refused"; return 1; }
  rows=$(sed -n "s/^capacity: per-game $PER_GAME, \([0-9]*\) rows.*/\1/p" "$OUT/capacity.txt")
  [[ -n "$rows" ]] || { say "no capacity read"; return 1; }
  rm -f "$OUT/corpus.tsv"
  python3 "$split" sample --in "$OUT/pool.tsv.qs.train" \
      --validation "$OUT/pool.tsv.qs.validation" --validation "$OUT/pool.tsv.root.validation" \
      --validation "$OUT/pool.tsv.pv.validation" --rows "$rows" --per-game "$PER_GAME" \
      --seed 1 --out "$OUT/corpus.tsv" | tee "$OUT/sample.txt"
  (( PIPESTATUS[0] == 0 )) || { say "the sample refused"; return 1; }
  write_marker corpus.tsv
}

k_stage() {
  needs verify.txt $view_markers split.txt corpus.tsv.done || return 1
  if [[ -s "$OUT/k.txt" ]]; then say "K before: $(cat "$OUT/k.txt")"; return 0; fi
  [[ -x "$tuner" ]] || { say "no tuner at $tuner"; return 1; }
  # A free set the tuner accepts, S122's groups: K is fitted at the shipped
  # constants before any step, whatever the set (S101_screen.sh's K probes).
  "$tuner" --data "$OUT/corpus.tsv" --holdout "$OUT/pool.tsv.qs.validation" \
      --out "$OUT/k.hpp" --epochs 0 --only ks_checks,ks_weak,ks_defenders,ks_storm,ks_curve \
      --threads "$THREADS" > "$OUT/k.log" 2>&1 ||
    { tail -5 "$OUT/k.log"; say "the K run failed"; return 1; }
  sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$OUT/k.log" > "$OUT/k.txt"
  [[ -s "$OUT/k.txt" ]] || { say "the K run printed no K"; return 1; }
  grep -E '^trace:' "$OUT/k.log"
  say "K $(cat "$OUT/k.txt") at the shipped constants and margin"
}

record() {
  local v
  needs verify.txt $view_markers split.txt corpus.tsv.done k.txt || return 1
  [[ -s "$OUT/k.txt" ]] || { say "refused: run k first"; return 1; }
  {
    echo "S122 prepared corpus, $(date -Iseconds)"
    echo "corpus $OUT/corpus.tsv"
    echo "sha256 $(sha256sum "$OUT/corpus.tsv" | cut -d' ' -f1)"
    echo "rows $(wc -l < "$OUT/corpus.tsv")"
    echo "games $(awk -F'\t' '{ seen[$5] = 1 } END { n = 0; for (g in seen) n++; print n }' "$OUT/corpus.tsv")"
    for v in qs root pv; do
      echo "view $v $OUT/pool.tsv.$v.validation $(sha256sum "$OUT/pool.tsv.$v.validation" | cut -d' ' -f1)"
    done
    echo "k $(cat "$OUT/k.txt")"
    echo "pool $POOL"
    echo "recipe filter |column 3| < $BAND, split seed $SPLIT_SEED, $VALIDATION_GAMES games held out, $PER_GAME qs rows a game, sample seed 1"
    echo "S122-PREPARED"
  } > "$OUT/prepared.txt"
  cat "$OUT/prepared.txt"
}

# adopt's checks of corpus.tsv.done's preparation beyond its own file (finding 3 of S333's
# fast check): the records it rests on are present and agree with the files now.
adopt_corpus() {
  local p v recorded ok=1 prep="$OUT/prepared.txt"
  for p in $view_markers split.txt prepared.txt k.txt; do
    [[ -f "$OUT/$p" ]] || { say "refused: no $OUT/$p, so corpus.tsv's preparation cannot be read"; ok=0; }
  done
  (( ok )) || return 1
  if [[ "$(sed -n 's/^pool //p' "$prep")" != "$POOL" ]] ||
     [[ "$(sed -n 's/^sha256 //p' "$prep")" != "$(head -n 1 "$OUT/corpus.tsv.done")" ]]; then
    say "refused: $prep does not name $POOL and corpus.tsv's sha256"
    ok=0
  fi
  for v in $VIEWS_ALL; do
    recorded=$(awk -v v="$v" '$1 == "view" && $2 == v { print $4 }' "$prep")
    if [[ -z "$recorded" || "$recorded" != "$(sha256sum "$OUT/pool.tsv.$v.validation" 2>/dev/null | cut -d' ' -f1)" ]]; then
      say "refused: $prep's $v view sha256 '${recorded:-none}' is not $OUT/pool.tsv.$v.validation's"
      ok=0
    elif [[ "$OUT/pool.tsv.$v.validation" -ot "$POOL/generate.log" ]]; then
      say "refused: $OUT/pool.tsv.$v.validation is older than $POOL/generate.log, so it was not made from this pool"
      ok=0
    fi
  done
  if [[ "$(sed -n 's/^k //p' "$prep")" != "$(cat "$OUT/k.txt")" ]]; then
    say "refused: $prep's k '$(sed -n 's/^k //p' "$prep")' is not $OUT/k.txt's '$(cat "$OUT/k.txt")'"
    ok=0
  fi
  (( ok ))
}

# S333: a preparation made before S333, its verify.txt and markers naming no pool,
# stamped with POOL's, so the stages read it again. Nothing is written until every
# check has passed:
#   - POOL's log passes verify's checks of it (check_pool_log);
#   - verify.txt's pool lines are POOL's log's, and the sha256 it records of each view
#     is POOL's view's now;
#   - each marker's first line is its file's sha256, and the file is no older than
#     POOL/generate.log;
#   - each filter marker's file is the band filter of POOL's view now: the filter run
#     again into sha256sum, written nowhere;
#   - for corpus.tsv.done: the view markers, split.txt, prepared.txt and k.txt are
#     present; prepared.txt names POOL and corpus.tsv's sha256; the sha256 of each held-out
#     view it records is that .validation file's now, and the file is no older than
#     POOL/generate.log; its `k` is k.txt's.
# Not checked: the training views and split.txt's contents, and that the split and the
# sample were drawn from these files, since recomputing them would be the preparation
# run again. No stage after `sample` reads the training views; the screen reads
# corpus.tsv, the held-out views and K, through prepared.txt.
# Then each file named takes a `pool` line, as verify and the stages write it, and an
# `adopted` line, so a stamp is told from a stage's; by way of a .tmp renamed. A file
# already naming POOL's pool is left; one naming another is refused, never restamped.
adopt() {
  local now v name file named recorded failed=0 stamp="" when
  check_pool_log || { say "refused: $POOL fails verify's checks of its log, so nothing is adopted by it"; return 1; }
  now=$(sha256sum "$POOL/generate.log" | cut -d' ' -f1)
  [[ -f "$OUT/verify.txt" ]] || { say "refused: no $OUT/verify.txt: no preparation to adopt"; return 1; }
  for name in verify.txt $markers; do
    [[ -f "$OUT/$name" ]] || continue
    named=$(named_pool "$OUT/$name")
    if [[ -z "$named" ]]; then
      stamp+=" $name"
    elif [[ "$named" == "$now" ]]; then
      say "adopt     $name names this pool already"
    else
      say "refused: $OUT/$name names the pool whose generate.log has sha256 $named, not $POOL's $now: never restamped"
      failed=1
    fi
  done
  (( failed == 0 )) || return 1
  [[ -n "$stamp" ]] || { say "adopt     nothing to stamp"; return 0; }

  if [[ " $stamp " == *" verify.txt "* ]]; then
    if diff <(sed 1d "$OUT/verify.txt" | grep -vE '^(pool|view|adopted) ') <(pool_lines) > /dev/null; then
      say "adopt     verify.txt's pool lines are $POOL/generate.log's"
    else
      say "refused: $OUT/verify.txt's pool lines are not $POOL/generate.log's"
      failed=1
    fi
    for v in $VIEWS_ALL; do
      recorded=$(sed -n "s/^view $v: .*, sha256 \([0-9a-f]\{64\}\)$/\1/p" "$OUT/verify.txt")
      if [[ -n "$recorded" && "$recorded" == "$(sha256sum "$POOL/pool.tsv.$v" | cut -d' ' -f1)" ]]; then
        say "adopt     verify.txt's $v view is $POOL/pool.tsv.$v, by sha256"
      else
        say "refused: $OUT/verify.txt's $v view sha256 '${recorded:-none}' is not $POOL/pool.tsv.$v's"
        failed=1
      fi
    done
  fi
  for name in $stamp; do
    [[ "$name" != verify.txt ]] || continue
    file="$OUT/${name%.done}"
    if [[ ! -f "$file" || "$(head -n 1 "$OUT/$name")" != "$(sha256sum "$file" | cut -d' ' -f1)" ]]; then
      say "refused: $file is not the file its marker $name records"
      failed=1
      continue
    fi
    if [[ "$file" -ot "$POOL/generate.log" ]]; then
      say "refused: $file is older than $POOL/generate.log, so it was not made from this pool"
      failed=1
      continue
    fi
    case "$name" in
      pool.tsv.*.done)
        v=${name#pool.tsv.}
        v=${v%.done}
        recorded=$(awk -F'\t' -v band="$BAND" -v keep=/dev/stdout -v past=/dev/null "$band_filter" \
                     "$POOL/pool.tsv.$v" | sha256sum | cut -d' ' -f1)
        if [[ "$recorded" == "$(head -n 1 "$OUT/$name")" ]]; then
          say "adopt     $file is the band filter of $POOL/pool.tsv.$v, recomputed"
        else
          say "refused: $file is not the band filter of $POOL/pool.tsv.$v, recomputed: sha256 $recorded"
          failed=1
        fi
        ;;
      corpus.tsv.done)
        if ! adopt_corpus; then
          failed=1
        else
          say "adopt     $file is its marker's and prepared.txt's, as are the three held-out views and K, each made after the pool's log; the split and the sample are not recomputed"
        fi
        ;;
    esac
  done
  (( failed == 0 )) || return 1
  if [[ "${ADOPT_DRY:-0}" == 1 ]]; then
    say "adopt     ADOPT_DRY=1: every check passed; would stamp$stamp; nothing written"
    return 0
  fi
  when=$(date -Iseconds)
  for name in $stamp; do
    {
      if [[ "$name" == verify.txt ]]; then cat "$OUT/$name"; else head -n 1 "$OUT/$name"; fi
      binary_line pool "$POOL/generate.log"
      echo "adopted   $when by S122_corpus.sh adopt (S333), after its checks"
    } > "$OUT/$name.tmp" && mv "$OUT/$name.tmp" "$OUT/$name" || return 1
    say "adopt     stamped $OUT/$name"
  done
}

# DEC-296's form, S082_recipe.sh's `merge` for this pool. The pool datagen was writing
# when it was stopped (part 1: POOL's generate.log and pool.tsv.*) and the top-up that
# completes it (part 2: TOPUP's topup.log and pool.tsv.*, the coordinator's run of the
# corpus's frozen datagen under TOPUP_SEED) become the pool every later stage reads,
# at the names it reads: POOL/pool.tsv.* and POOL/generate.log. Part 1's four files
# move once to POOL/part1/, by mv on one filesystem, their sha256 written first to
# POOL/part1/originals.sha256 and checked on every run; nothing here writes into them.
# Since DEC-349 part 1's games.tsv moves with them. Since S332 the merged pool has one
# games file, POOL/games.tsv: part 1's games lines cut where its rows were, then the
# top-up's with its ids moved as its rows' are, written by S331_measure.py merge, which
# also gives the merged log one combined `repetition:` line, part 1's figures recomputed
# from its kept lines and rows beside part 2's from its log. The top-up's own games.tsv
# stays in TOPUP. datagen, run.sh and pid stay in POOL, where S122_screen.sh sample
# reads the datagen.
#
# Refused before anything moves unless the top-up's log has one `exit` line, `exit 0`,
# and ends S122-TOPUP-DONE; it holds datagen's `repetition:` line and part 1's log
# names the same datagen sha256 (DEC-349: no part by a datagen before S331); both
# parts have a games.tsv (S332); its
# seed is TOPUP_SEED and its seeds, games-wide and per
# worker, meet neither the corpus's [CORPUS_SEED, CORPUS_SEED + GAMES) nor the side
# sample's (DEC-345 (5)); the corpus's own log (first line `S122 corpus`) ends
# S122-CORPUS-FAILED; S082_pool_merge.py part1 cuts part 1 clean; and its check finds
# that part 2's rows give every figure part 2's log prints. Then its merge: the
# cut, part 2's game ids moved up by the smallest power of ten above part 1's, the
# files written under .merging names and read back; then the games file, from the
# merged root view. The merged files are then held to the merge's own lines
# (check_merged), and S331_measure.py read reads the merged pool against the log being
# composed, its `repetition:` line recomputed from the merged files (S332). A power loss,
# DEC-296's own case, can leave part 1's games.tsv ending before games the merger keeps:
# the merge then passes with the views as ever and the games file holding what part 1's
# has, and the log gives a `repetition incomplete:` line in place of the combined one,
# so the check is partial and `read` is not run. The log is
# composed as generate.log.merging -- MERGED_HEADER, both runs' logs verbatim and
# indented, the readings -- and renamed last, ending S122-CORPUS-DONE, so a merged
# generate.log that exists is a merge that passed.
#
# Re-runnable: a refusal after the move leaves part 1 in POOL/part1/ and no merged
# pool; the files this run made are removed and its log kept as POOL/merge.failed.log,
# so the next run starts where it did. A merged pool is never overwritten. A run
# interrupted after renaming the merged files and before renaming its log leaves
# pool.tsv.*, games.tsv and generate.log.merging with no generate.log: the next run
# names that state and refuses, deleting nothing; part 1 is intact in POOL/part1/, so
# the recovery is by hand, removing pool.tsv.*, games.tsv and generate.log.merging and
# re-running.
merge() {
  local part1="$POOL/part1" log="$POOL/generate.log" topup_log="$TOPUP/topup.log"
  local originals="generate.log pool.tsv.root pool.tsv.qs pool.tsv.pv"
  local from name v command tseed tgames tthreads span reading checked report rc p1sha tsha offset

  [[ -r "$merger" ]] || { say "no merger at $merger"; return 1; }
  [[ -r "$reader" ]] || { say "no reader at $reader"; return 1; }
  [[ -r "$topup_log" ]] || { say "refused: no top-up log at $topup_log"; return 1; }
  if [[ "$(grep -cE '^exit +[0-9]+$' "$topup_log")" != 1 ]] || ! grep -qxE 'exit +0' "$topup_log"; then
    say "refused: $topup_log has no one 'exit 0' line: the top-up has not finished, or failed"
    return 1
  fi
  [[ "$(tail -n 1 "$topup_log")" == S122-TOPUP-DONE ]] ||
    { say "refused: $topup_log does not end S122-TOPUP-DONE"; return 1; }
  # DEC-349: no part by a datagen before S331, which drew a game at any two-fold. The
  # top-up finished, so its log holds datagen's closing `repetition:` line, which only
  # S331's datagen prints; part 1, stopped before datagen's closing lines, is held to
  # the top-up's datagen by sha256 below, once it is found.
  grep -q '^repetition: ' "$topup_log" ||
    { say "refused: $topup_log has no 'repetition:' line: its datagen predates S331, whose two-fold draws DEC-349 voids"; return 1; }
  # S332: the merged pool's games file is made of both parts' (`--games-out`).
  [[ -f "$TOPUP/games.tsv" ]] ||
    { say "refused: no $TOPUP/games.tsv: the top-up ran without --games-out, and the merged pool would have no games file"; return 1; }

  command=" $(sed -n 's/^command *//p' "$topup_log" | head -n 1) "
  tseed=$(sed -n 's/.* --seed \([0-9][0-9]*\) .*/\1/p' <<< "$command")
  tgames=$(sed -n 's/.* --games \([0-9][0-9]*\) .*/\1/p' <<< "$command")
  tthreads=$(sed -n 's/.* --threads \([0-9][0-9]*\) .*/\1/p' <<< "$command")
  [[ -n "$tseed" && -n "$tgames" && -n "$tthreads" ]] ||
    { say "refused: $topup_log's command line names no --seed, --games and --threads"; return 1; }
  [[ "$tseed" == "$TOPUP_SEED" ]] ||
    { say "refused: the top-up's seed is $tseed, not $TOPUP_SEED"; return 1; }
  # A run's seeds are its workers' [seed, seed + threads) and the games-wide range,
  # the coordinator's stricter rule: the span covers both.
  span=$(( tgames > tthreads ? tgames : tthreads ))
  if (( tseed < CORPUS_SEED + GAMES && CORPUS_SEED < tseed + span )); then
    say "refused: the top-up's seeds [$tseed, $(( tseed + span ))) meet the corpus's [$CORPUS_SEED, $(( CORPUS_SEED + GAMES )))"
    return 1
  fi
  if (( tseed < SAMPLE_SEED + SAMPLE_GAMES && SAMPLE_SEED < tseed + span )); then
    say "refused: the top-up's seeds [$tseed, $(( tseed + span ))) meet the side sample's [$SAMPLE_SEED, $(( SAMPLE_SEED + SAMPLE_GAMES )))"
    return 1
  fi

  # Part 1 where it is: still in POOL, or moved before by a run that then stopped.
  if [[ -e "$part1/originals.sha256" ]]; then
    from="$part1"
    (cd "$part1" && sha256sum --quiet -c originals.sha256) ||
      { say "refused: part 1's files in $part1 are not the ones whose sha256 was written before the move"; return 1; }
    if [[ ! -e "$log" && -e "$log.merging" ]] &&
       { compgen -G "$POOL/pool.tsv.*" > /dev/null || [[ -e "$POOL/games.tsv" ]]; }; then
      say "refused: an interrupted merge: $POOL/pool.tsv.* or $POOL/games.tsv, and $log.merging, exist and $log does not, so a run stopped after renaming its merged files and before renaming its log. Part 1 is intact in $part1 (checked against originals.sha256 just now). By hand: remove $POOL/pool.tsv.*, $POOL/games.tsv and $log.merging, then re-run"
      return 1
    fi
    for v in $VIEWS_ALL; do
      [[ ! -e "$POOL/pool.tsv.$v" ]] ||
        { say "refused: $POOL/pool.tsv.$v exists: a merged pool is never overwritten"; return 1; }
    done
    [[ ! -e "$POOL/games.tsv" ]] || { say "refused: $POOL/games.tsv exists: a merged pool is never overwritten"; return 1; }
    [[ ! -e "$log" ]] || { say "refused: $log exists: a merged pool is never overwritten"; return 1; }
    grep -qE '^[0-9a-f]{64}  games\.tsv$' "$part1/originals.sha256" ||
      { say "refused: $part1/originals.sha256 names no games.tsv: part 1 was moved without its games file, so the merged pool would have none (S332)"; return 1; }
  else
    [[ ! -e "$part1" ]] ||
      { say "refused: $part1 exists without originals.sha256: a move was interrupted; resolve it by hand"; return 1; }
    for name in $originals; do
      [[ -f "$POOL/$name" ]] || { say "refused: no $POOL/$name to move"; return 1; }
    done
    # Part 1's games file (`--games-out`, DEC-349) goes with the files it describes; the
    # merged one is cut from it (S332).
    [[ -f "$POOL/games.tsv" ]] ||
      { say "refused: no $POOL/games.tsv: part 1 ran without --games-out, and the merged pool would have no games file"; return 1; }
    originals+=" games.tsv"
    [[ "$(head -n 1 "$log")" == "S122 corpus" ]] ||
      { say "refused: $log is not the corpus's own log: no part 1 to move"; return 1; }
    [[ "$(tail -n 1 "$log")" == S122-CORPUS-FAILED ]] ||
      { say "refused: $log does not end S122-CORPUS-FAILED: a pool that finished needs no top-up"; return 1; }
    from="$POOL"
  fi
  p1sha=$(sed -n 's/^datagen *sha256 \([0-9a-f]\{64\}\) .*/\1/p' "$from/generate.log" | head -n 1)
  tsha=$(sed -n 's/^datagen *sha256 \([0-9a-f]\{64\}\) .*/\1/p' "$topup_log" | head -n 1)
  [[ -n "$p1sha" && "$p1sha" == "$tsha" ]] ||
    { say "refused: part 1's datagen sha256 '${p1sha:-none}' is not the top-up's '${tsha:-none}', the one its 'repetition:' line shows to be S331's (DEC-349)"; return 1; }

  # Read only, before anything moves: the cut, and part 2 against its own log.
  reading=$(python3 "$merger" part1 "$from/pool.tsv" "$from/generate.log")
  rc=$?
  echo "$reading"
  (( rc == 0 )) || { say "refused: S082_pool_merge.py part1 does not cut part 1"; return 1; }
  checked=$(python3 "$merger" check "$TOPUP/pool.tsv" "$topup_log")
  rc=$?
  echo "$checked"
  (( rc == 0 )) || { say "refused: part 2's rows do not give what its log prints"; return 1; }

  if [[ "$from" == "$POOL" ]]; then
    mkdir "$part1" || return 1
    if [[ "$(stat -c %d "$POOL")" != "$(stat -c %d "$part1")" ]]; then
      rmdir "$part1"
      say "refused: $part1 is on another filesystem than $POOL: mv would copy"
      return 1
    fi
    # shellcheck disable=SC2086
    if ! (cd "$POOL" && sha256sum $originals) > "$part1/originals.sha256.tmp"; then
      rm -f "$part1/originals.sha256.tmp"
      rmdir "$part1"
      return 1
    fi
    for name in $originals; do
      mv "$POOL/$name" "$part1/$name" || return 1
    done
    mv "$part1/originals.sha256.tmp" "$part1/originals.sha256" || return 1
    say "moved     part 1's $originals to $part1/, their sha256 written first to $part1/originals.sha256"
    (cd "$part1" && sha256sum --quiet -c originals.sha256) ||
      { say "refused: part 1's files in $part1 are not the ones whose sha256 was written before the move"; return 1; }
  fi
  for v in $VIEWS_ALL; do rm -f "$POOL/pool.tsv.$v.merging"; done
  rm -f "$POOL/games.tsv.merging" "$log.merging"

  {
    echo "$MERGED_HEADER"
    echo "merged    $(date -Iseconds)"
    binary_line merger "$merger"
    binary_line reader "$reader"
    binary_line stage "$root/adocs/data/S122_corpus.sh"
    echo "python    $(python3 --version 2>&1)"
    echo "part 1    $part1/generate.log, the corpus's own log, moved there unchanged; verbatim:"
    sed 's/^/    /' "$part1/generate.log"
    echo "part 1    stopped before datagen's closing lines"
    echo "part 1    sha256 written before the move to $part1/originals.sha256, checked on this run:"
    sed 's/^/    /' "$part1/originals.sha256"
    echo "part 1    S082_pool_merge.py part1, read only:"
    sed 's/^/    /' <<< "$reading"
    echo "part 2    $topup_log, the top-up's own log; verbatim:"
    sed 's/^/    /' "$topup_log"
    echo "part 2    S082_pool_merge.py check, against its own log:"
    sed 's/^/    /' <<< "$checked"
  } > "$log.merging"

  python3 "$merger" merge --games "$GAMES" --seed "$CORPUS_SEED" --topup-seed "$TOPUP_SEED" \
      --nodes "$NODES" "$part1/pool.tsv" "$part1/generate.log" "$TOPUP/pool.tsv" "$topup_log" \
      "$POOL/pool.tsv" | tee -a "$log.merging"
  rc=${PIPESTATUS[0]}
  if (( rc != 0 )); then
    mv "$log.merging" "$POOL/merge.failed.log"
    say "refused: the merger refused; its log is $POOL/merge.failed.log"
    return 1
  fi

  # The games file, from the merged root view and both parts' games files, and the one
  # `repetition:` line; the offset is the merger's own line's.
  offset=$(sed -n "s/^offset    part 2's game ids plus \([0-9][0-9]*\),.*/\1/p" "$log.merging" | tail -n 1)
  python3 "$reader" merge "${offset:-none}" "$part1" "$TOPUP" "$topup_log" \
      "$POOL/pool.tsv.root.merging" "$POOL/games.tsv" | tee -a "$log.merging"
  rc=${PIPESTATUS[0]}
  if (( rc != 0 )); then
    # The merger's files of this run, and the games file it may have left.
    for v in $VIEWS_ALL; do rm -f "$POOL/pool.tsv.$v.merging"; done
    rm -f "$POOL/games.tsv.merging"
    mv "$log.merging" "$POOL/merge.failed.log"
    say "refused: S331_measure.py merge refused the games files; its log is $POOL/merge.failed.log"
    return 1
  fi

  for v in $VIEWS_ALL; do
    mv "$POOL/pool.tsv.$v.merging" "$POOL/pool.tsv.$v" || return 1
  done
  mv "$POOL/games.tsv.merging" "$POOL/games.tsv" || return 1
  report=$(check_merged "$log.merging")
  rc=$?
  echo "$report" | tee -a "$log.merging"
  if (( rc == 0 )) && grep -q '^repetition incomplete: ' "$log.merging"; then
    # Part 1's games file cut short by a power loss: the pool stands, its check is partial.
    echo "read      not run: part 1's games file ends before kept games (the line above), so no combined repetition: line exists to read; the views and the games file are held to the merge's lines above" |
      tee -a "$log.merging"
  elif (( rc == 0 )); then
    echo "read      S331_measure.py read $POOL against this log, its views and games file and the repetition: line above:" |
      tee -a "$log.merging"
    report=$(python3 "$reader" read "$POOL" "$log.merging")
    rc=$?
    sed -E 's/^/read      /; s/ +$//' <<< "$report" | tee -a "$log.merging"
  fi
  if (( rc != 0 )); then
    # This run's own files: they did not exist when it started.
    for v in $VIEWS_ALL; do rm -f "$POOL/pool.tsv.$v"; done
    rm -f "$POOL/games.tsv"
    mv "$log.merging" "$POOL/merge.failed.log"
    say "refused: the merged pool fails its checks: removed, its log kept as $POOL/merge.failed.log"
    return 1
  fi
  { echo "finished  $(date -Iseconds)"; echo "S122-CORPUS-DONE"; } >> "$log.merging"
  # The marker is believed only once the pool and its log are on disk.
  sync
  mv "$log.merging" "$log" || return 1
  sync
}

all_stages() {
  verify && filter && split_stage && sample && k_stage && record
}

# A slice of SRC into DIR: the games whose first root row is in rows (LO, HI] of
# SRC/pool.tsv.root, every row of them in every view, and a log of SRC's own lines.
smoke_slice() {
  local src="$1" lo="$2" hi="$3" dir="$4" games v
  mkdir -p "$dir" || return 1
  games=$(awk -F'\t' -v lo="$lo" -v hi="$hi" \
            '{ if (!($5 in seen)) { if (NR > hi) exit; seen[$5] = 1; if (NR > lo) print $5 } }' \
            "$src/pool.tsv.root" | sort -n | tr '\n' ' ')
  [[ -n "${games// /}" ]] || { say "no game starts in rows ($lo, $hi] of $src/pool.tsv.root"; return 1; }
  for v in $VIEWS_ALL; do
    [[ -f "$src/pool.tsv.$v" ]] || { say "no $src/pool.tsv.$v"; return 1; }
    awk -F'\t' -v keep=" $games" 'index(keep, " " $5 " ") > 0' "$src/pool.tsv.$v" > "$dir/pool.tsv.$v"
  done
  {
    echo "S122 corpus smoke: the games of $src whose first root row is in rows ($lo, $hi], every row of them; the lines below are $src/generate.log's, for its whole run"
    grep -E '^(engine|datagen|command|repetition)' "$src/generate.log"
    echo "S122-CORPUS-DONE"
  } > "$dir/generate.log"
}

# Every file under DIR by sha256.
smoke_manifest() { (cd "$1" && find . -type f -print0 | sort -z | xargs -0 -r sha256sum); }

# The cases' setups, each given the case's directory C, writing C/prepared and, for a
# pool of the case's own, C/pool. They are this script's functions, whatever
# SMOKE_AGAINST is.
smoke_copy() { find "$OUT" -maxdepth 1 -type f -exec cp -p -t "$1/prepared" {} +; }
# verify.txt naming the swap and the markers A: the markers' refusal alone. A plant,
# since verify no longer writes over another pool's preparation.
smoke_verify_swap() {
  smoke_copy "$1" && sed -i "s|^pool  *sha256 .*|$(binary_line pool "$smoke_b/generate.log")|" "$1/prepared/verify.txt" &&
    [[ "$(named_pool "$1/prepared/verify.txt")" == "$(sha256sum "$smoke_b/generate.log" | cut -d' ' -f1)" ]]
}
# Every marker and verify.txt removed by hand, the split and the sample left.
smoke_unmarked() { smoke_copy "$1" && (cd "$1/prepared" && rm verify.txt $markers); }
smoke_verify_only() { cp -p "$OUT/verify.txt" "$1/prepared/"; }
# A preparation as the stages wrote it before S333: markers of one line, no pool line.
smoke_old_form() {
  local name
  smoke_copy "$1" || return 1
  for name in $markers; do
    head -n 1 "$1/prepared/$name" > "$1/prepared/$name.tmp" && mv "$1/prepared/$name.tmp" "$1/prepared/$name" || return 1
  done
  grep -v '^pool ' "$1/prepared/verify.txt" > "$1/prepared/verify.txt.tmp" &&
    mv "$1/prepared/verify.txt.tmp" "$1/prepared/verify.txt"
}
smoke_old_tampered() { smoke_old_form "$1" && echo >> "$1/prepared/pool.tsv.qs"; }
# The file and its marker agree, and the file is not the pool's filter.
smoke_old_forged() {
  smoke_old_form "$1" && sed -i '$d' "$1/prepared/pool.tsv.root" &&
    sha256sum "$1/prepared/pool.tsv.root" | cut -d' ' -f1 > "$1/prepared/pool.tsv.root.done"
}
smoke_old_aged() { smoke_old_form "$1" && touch -d '2000-01-01T00:00:00' "$1/prepared/corpus.tsv"; }
smoke_old_adopted() { smoke_old_form "$1" && ( POOL="$smoke_a"; OUT="$1/prepared"; adopt ); }
smoke_old_prepared_sha() {
  smoke_old_form "$1" && sed -i 's/^sha256 .*/sha256 0000000000000000000000000000000000000000000000000000000000000000/' "$1/prepared/prepared.txt"
}
smoke_old_validation() { smoke_old_form "$1" && echo >> "$1/prepared/pool.tsv.qs.validation"; }
smoke_old_no_prepared() { smoke_old_form "$1" && rm "$1/prepared/prepared.txt"; }
smoke_old_k() { smoke_old_form "$1" && echo 9.9999 > "$1/prepared/k.txt"; }
# C/pool: A's views, its log edited by EDIT (a sed program) and given A's log's mtime, so
# only the edit tells the two apart.
smoke_own_pool() {
  mkdir "$1/pool" && cp -p "$smoke_a"/pool.tsv.* "$1/pool/" &&
    sed "$2" "$smoke_a/generate.log" > "$1/pool/generate.log" &&
    ! cmp -s "$smoke_a/generate.log" "$1/pool/generate.log" &&
    touch -r "$smoke_a/generate.log" "$1/pool/generate.log"
}
smoke_old_pool_lines() { smoke_old_form "$1" && smoke_own_pool "$1" 's/^command .*/& --edited/'; }
smoke_old_pre_s331() { smoke_old_form "$1" && smoke_own_pool "$1" '/^repetition: /d'; }
# A one-run pool written before S331: the slice's log without its `repetition:` line.
smoke_pre_s331() {
  mkdir "$1/pool" && cp -p "$smoke_a"/pool.tsv.* "$1/pool/" &&
    grep -q '^repetition: ' "$smoke_a/generate.log" &&
    grep -v '^repetition: ' "$smoke_a/generate.log" > "$1/pool/generate.log"
}

# One case, OUT/cases/NAME: SETUP builds it, then STAGE runs as a child of
# smoke_against on POOL (A the slice, B the swap, own the case's) with EXTRA in its
# environment. `refused` asks for -FAILED and prepared/ unchanged, `passes` for -DONE,
# `unchanged` for both; and each REGEX printed. Counted in the caller's n and bad.
smoke_case() {
  local name="$1" expect="$2" pool="$3" setup="$4" stage="$5" extra="$6" c="$OUT/cases/$1"
  local out last before after re ok=1
  shift 6
  n=$(( n + 1 ))
  case "$pool" in
    A) pool="$smoke_a" ;;
    B) pool="$smoke_b" ;;
    own) pool="$c/pool" ;;
  esac
  rm -rf "$c"; mkdir -p "$c/prepared" || { bad=$(( bad + 1 )); return 1; }
  if ! ( "$setup" "$c" ) > "$c/setup.txt" 2>&1; then
    say "smoke case $name: FAILED, its setup $setup failed: $(tail -n 1 "$c/setup.txt")"
    bad=$(( bad + 1 ))
    return 1
  fi
  before=$(smoke_manifest "$c/prepared" "$stage")
  # shellcheck disable=SC2086
  out=$(env $extra POOL="$pool" OUT="$c/prepared" TUNING="$TUNING" GAMES="$GAMES" \
          VALIDATION_GAMES="$VALIDATION_GAMES" bash "$smoke_against" "$stage" 2>&1)
  printf '%s\n' "$out" > "$c/console.txt"
  last=$(tail -n 1 <<< "$out")
  after=$(smoke_manifest "$c/prepared" "$stage")
  case "$expect" in
    refused) [[ "$last" == "S122-CORPUS-${stage^^}-FAILED" && "$before" == "$after" ]] || ok=0 ;;
    passes) [[ "$last" == "S122-CORPUS-${stage^^}-DONE" ]] || ok=0 ;;
    unchanged) [[ "$last" == "S122-CORPUS-${stage^^}-DONE" && "$before" == "$after" ]] || ok=0 ;;
  esac
  for re in "$@"; do grep -qE -- "$re" <<< "$out" || ok=0; done
  if (( ok )); then
    say "smoke case $name: $expect, as it must be: $(grep -m 1 -E -- "$1" <<< "$out")"
  else
    say "smoke case $name: FAILED: expected $expect, the stage ended '$last'$([[ "$before" == "$after" ]] || echo ', and prepared/ changed')"
    bad=$(( bad + 1 ))
    return 1
  fi
}

# The six stages on a slice of S331's measured run (S333; until then the research
# pass's sample, a pool before S331, which verify now refuses), under 20000 rows a
# view; then S333's refusals, each on its own copy under OUT/cases, run as a child of
# SMOKE_AGAINST, by default this script: given the script before S333, each refusal, and
# adopt itself, is missing, the red. The two controls, control_all and adopted_reads, pass
# under either.
smoke() {
  local src="$TUNING/s331/measure" played n=0 bad=0
  local marker_swap='pool\.tsv\.(root|qs|pv)\.done was prepared from the pool'
  local corpus_swap='corpus\.tsv\.done was prepared from the pool'
  smoke_against="${SMOKE_AGAINST:-$self}"
  OUT="$TUNING/s122/phase1/corpus_smoke"
  rm -rf "$OUT"; mkdir -p "$OUT" || return 1
  [[ -f "$src/generate.log" ]] || { say "no $src/generate.log"; return 1; }
  played=$(sed -n 's/^datagen: \([0-9][0-9]*\) games, .*/\1/p' "$src/generate.log" | head -n 1)
  [[ -n "$played" ]] || { say "no datagen settings line in $src/generate.log"; return 1; }
  smoke_a="$OUT/pool"
  smoke_b="$OUT/swap"
  smoke_slice "$src" 0 19000 "$smoke_a" || return 1
  smoke_slice "$src" 19000 38000 "$smoke_b" || return 1
  POOL="$smoke_a"; VALIDATION_GAMES=20; GAMES="$played"
  all_stages || return 1
  say "smoke     the six stages passed on $smoke_a; the swap is $smoke_b; the cases are run by $smoke_against"
  smoke_case control_all passes A smoke_copy all "" 'filtered before' 'split before' 'sampled before' 'K before'
  smoke_case verify_no_repetition refused own smoke_pre_s331 verify "" "has no 'repetition:' line: its datagen predates S331"
  smoke_case all_swapped refused B smoke_copy all "" "$marker_swap"
  smoke_case filter_swapped refused B smoke_verify_swap filter "" "$marker_swap"
  smoke_case split_swapped refused B smoke_verify_swap split "" "$marker_swap"
  smoke_case sample_swapped refused B smoke_verify_swap sample "" "$corpus_swap"
  smoke_case k_swapped refused B smoke_verify_swap k "" "$corpus_swap"
  smoke_case record_swapped refused B smoke_verify_swap record "" "$corpus_swap"
  smoke_case filter_unverified refused B smoke_verify_only filter "" 'verify\.txt was prepared from the pool'
  smoke_case old_marker refused A smoke_old_form filter "" 'pool\.tsv\.root\.done names no pool: written before S333'
  smoke_case adopt_dry unchanged A smoke_old_form adopt ADOPT_DRY=1 'every check passed; would stamp verify\.txt'
  smoke_case adopt passes A smoke_old_form adopt "" 'stamped .*/corpus\.tsv\.done' 'is the band filter of .*/pool\.tsv\.pv, recomputed'
  smoke_case adopted_reads passes A smoke_old_adopted k "" 'K before'
  smoke_case adopt_swapped refused B smoke_old_form adopt "" "verify\.txt's root view sha256 .* is not"
  smoke_case adopt_tampered refused A smoke_old_tampered adopt "" 'pool\.tsv\.qs is not the file its marker'
  smoke_case adopt_forged refused A smoke_old_forged adopt "" 'pool\.tsv\.root is not the band filter'
  smoke_case adopt_aged refused A smoke_old_aged adopt "" 'corpus\.tsv is older than'
  smoke_case adopt_restamp refused B smoke_copy adopt "" 'names the pool whose generate\.log has sha256 .*never restamped'
  smoke_case adopt_pool_lines refused own smoke_old_pool_lines adopt "" "verify\.txt's pool lines are not .*generate\.log's"
  smoke_case adopt_prepared_sha refused A smoke_old_prepared_sha adopt "" "prepared\.txt does not name .* and corpus\.tsv's sha256"
  smoke_case adopt_pre_s331 refused own smoke_old_pre_s331 adopt "" "has no 'repetition:' line: its datagen predates S331"
  smoke_case adopt_validation refused A smoke_old_validation adopt "" "prepared\.txt's qs view sha256 .* is not .*/pool\.tsv\.qs\.validation's"
  smoke_case adopt_no_prepared refused A smoke_old_no_prepared adopt "" "no .*/prepared\.txt, so corpus\.tsv's preparation cannot be read"
  smoke_case adopt_k refused A smoke_old_k adopt "" "prepared\.txt's k '[0-9.]+' is not .*/k\.txt's '9\.9999'"
  smoke_case sample_unmarked refused B smoke_unmarked sample "" 'no .*/verify\.txt: run verify first'
  say "smoke     $n cases, $(( n - bad )) as they must be, $bad not"
  (( bad == 0 ))
}

# `seeds` writes nothing, and runs before the void outputs are moved and after.
[[ "$stage" == seeds ]] || mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S122-CORPUS-FAILED"; exit 1; }

case "$stage" in
  seeds) if seeds; then echo "S122-CORPUS-SEEDS-DONE"; else echo "S122-CORPUS-SEEDS-FAILED"; exit 1; fi ;;
  merge) if merge; then echo "S122-CORPUS-MERGE-DONE"; else echo "S122-CORPUS-MERGE-FAILED"; exit 1; fi ;;
  verify) if verify; then echo "S122-CORPUS-VERIFY-DONE"; else echo "S122-CORPUS-VERIFY-FAILED"; exit 1; fi ;;
  filter) if filter; then echo "S122-CORPUS-FILTER-DONE"; else echo "S122-CORPUS-FILTER-FAILED"; exit 1; fi ;;
  split) if split_stage; then echo "S122-CORPUS-SPLIT-DONE"; else echo "S122-CORPUS-SPLIT-FAILED"; exit 1; fi ;;
  sample) if sample; then echo "S122-CORPUS-SAMPLE-DONE"; else echo "S122-CORPUS-SAMPLE-FAILED"; exit 1; fi ;;
  k) if k_stage; then echo "S122-CORPUS-K-DONE"; else echo "S122-CORPUS-K-FAILED"; exit 1; fi ;;
  record) if record; then echo "S122-CORPUS-RECORD-DONE"; else echo "S122-CORPUS-RECORD-FAILED"; exit 1; fi ;;
  all) if all_stages; then echo "S122-CORPUS-ALL-DONE"; else echo "S122-CORPUS-ALL-FAILED"; exit 1; fi ;;
  adopt) if adopt; then echo "S122-CORPUS-ADOPT-DONE"; else echo "S122-CORPUS-ADOPT-FAILED"; exit 1; fi ;;
  smoke) if smoke; then echo "S122-CORPUS-SMOKE-DONE"; else echo "S122-CORPUS-SMOKE-FAILED"; exit 1; fi ;;
  *) echo "usage: $0 seeds | merge | verify | filter | split | sample | k | record | all | adopt | smoke" >&2; exit 2 ;;
esac
