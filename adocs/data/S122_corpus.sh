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
#   adocs/data/S122_corpus.sh smoke      # all six on a slice of under 20000 rows
#
# Each stage prints S122-CORPUS-<STAGE>-DONE or -FAILED as its last line. POOL
# defaults to .tuning/s122/corpus, OUT to .tuning/s122/prepared (DEC-235). Only
# `merge` writes into POOL: it moves part 1's generate.log and pool.tsv.* (and its
# games.tsv, since DEC-349) into
# POOL/part1/ beside their originals.sha256, writes the merged pool.tsv.* and
# generate.log by way of .merging files, and keeps a refused run's log as
# merge.failed.log. No other stage writes there.
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
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
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

say() { echo "$*"; }

binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

# A merged pool against the merge's own `merged <view>:` lines in LOG: each plays
# GAMES and names POOL's file, and the file holds the rows the line counts, ends on
# a newline and has the sha256 the merge wrote. One line a view; non-zero on any
# finding.
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

# verify.txt is written only when every check below has passed, as a .tmp renamed
# last, and a run's old one is removed first: `filter` reads its presence as verify's
# pass, so a refusal leaves none.
verify() {
  local v rows games bad merged=0 record="$OUT/verify.txt"
  rm -f "$record" "$record.tmp"
  [[ -f "$POOL/generate.log" ]] || { say "no $POOL/generate.log"; return 1; }
  grep -q '^S122-CORPUS-DONE$' "$POOL/generate.log" || {
    say "refused: $POOL/generate.log has no S122-CORPUS-DONE: the pool is still generating, or failed; a top-up is S082's merge (DEC-296), the coordinator's"
    return 1
  }
  if [[ "$(head -n 1 "$POOL/generate.log")" == "$MERGED_HEADER" ]]; then
    merged=1
    check_merged "$POOL/generate.log" ||
      { say "refused: the merged pool is not the one its merge wrote"; return 1; }
  fi
  {
    echo "S122 corpus verify, $(date -Iseconds)"
    if (( merged )); then
      # Both runs, from their verbatim logs.
      echo "$MERGED_HEADER"
      grep -E '^(    )?(engine|datagen|command)' "$POOL/generate.log"
      grep -E '^merged [a-z]+: ' "$POOL/generate.log"
    else
      grep -E '^(engine|datagen|command)' "$POOL/generate.log"
    fi
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

filter() {
  local v kept past
  [[ -f "$OUT/verify.txt" ]] || { say "refused: run verify first"; return 1; }
  for v in $VIEWS_ALL; do
    if [[ -f "$OUT/pool.tsv.$v.done" ]]; then say "$v filtered before"; continue; fi
    awk -F'\t' -v band="$BAND" -v keep="$OUT/pool.tsv.$v" -v past="$OUT/pool_past.tsv.$v" '
        { v = $3 + 0; if (v < band && v > -band) print > keep; else print > past }' \
        "$POOL/pool.tsv.$v" || { say "the filter failed on $v"; return 1; }
    kept=$(wc -l < "$OUT/pool.tsv.$v")
    past=$(wc -l < "$OUT/pool_past.tsv.$v" 2>/dev/null || echo 0)
    echo "filter $v: |column 3| < $BAND keeps $kept rows, $past past it" | tee -a "$OUT/filter.txt"
    sha256sum "$OUT/pool.tsv.$v" | cut -d' ' -f1 > "$OUT/pool.tsv.$v.done"
  done
}

split_stage() {
  local v
  for v in $VIEWS_ALL; do
    [[ -f "$OUT/pool.tsv.$v.done" ]] || { say "refused: run filter first"; return 1; }
  done
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
  sha256sum "$OUT/corpus.tsv" | cut -d' ' -f1 > "$OUT/corpus.tsv.done"
}

k_stage() {
  [[ -f "$OUT/corpus.tsv.done" ]] || { say "refused: run sample first"; return 1; }
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

# DEC-296's form, S082_recipe.sh's `merge` for this pool. The pool datagen was writing
# when it was stopped (part 1: POOL's generate.log and pool.tsv.*) and the top-up that
# completes it (part 2: TOPUP's topup.log and pool.tsv.*, the coordinator's run of the
# corpus's frozen datagen under TOPUP_SEED) become the pool every later stage reads,
# at the names it reads: POOL/pool.tsv.* and POOL/generate.log. Part 1's four files
# move once to POOL/part1/, by mv on one filesystem, their sha256 written first to
# POOL/part1/originals.sha256 and checked on every run; nothing here writes into them.
# Since DEC-349 part 1's games.tsv, where its run wrote one, moves with them; no
# merged games file is written, and the top-up's stays in TOPUP.
# datagen, run.sh and pid stay in POOL, where S122_screen.sh sample reads the datagen.
#
# Refused before anything moves unless the top-up's log has one `exit` line, `exit 0`,
# and ends S122-TOPUP-DONE; it holds datagen's `repetition:` line and part 1's log
# names the same datagen sha256 (DEC-349: no part by a datagen before S331); its
# seed is TOPUP_SEED and its seeds, games-wide and per
# worker, meet neither the corpus's [CORPUS_SEED, CORPUS_SEED + GAMES) nor the side
# sample's (DEC-345 (5)); the corpus's own log (first line `S122 corpus`) ends
# S122-CORPUS-FAILED; S082_pool_merge.py part1 cuts part 1 clean; and its check finds
# that part 2's rows give every figure part 2's log prints. Then its merge: the
# cut, part 2's game ids moved up by the smallest power of ten above part 1's, the
# files written under .merging names and read back. The merged files are then held to
# the merge's own lines (check_merged). The log is composed as generate.log.merging --
# MERGED_HEADER, both runs' logs verbatim and indented, the readings -- and renamed
# last, ending S122-CORPUS-DONE, so a merged generate.log that exists is a merge that
# passed.
#
# Re-runnable: a refusal after the move leaves part 1 in POOL/part1/ and no merged
# pool; the files this run made are removed and its log kept as POOL/merge.failed.log,
# so the next run starts where it did. A merged pool is never overwritten. A run
# interrupted after renaming the merged files and before renaming its log leaves
# pool.tsv.* and generate.log.merging with no generate.log: the next run names that
# state and refuses, deleting nothing; part 1 is intact in POOL/part1/, so the
# recovery is by hand, removing pool.tsv.* and generate.log.merging and re-running.
merge() {
  local part1="$POOL/part1" log="$POOL/generate.log" topup_log="$TOPUP/topup.log"
  local originals="generate.log pool.tsv.root pool.tsv.qs pool.tsv.pv"
  local from name v command tseed tgames tthreads span reading checked report rc p1sha tsha

  [[ -r "$merger" ]] || { say "no merger at $merger"; return 1; }
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
       compgen -G "$POOL/pool.tsv.*" > /dev/null; then
      say "refused: an interrupted merge: $POOL/pool.tsv.* and $log.merging exist and $log does not, so a run stopped after renaming its merged files and before renaming its log. Part 1 is intact in $part1 (checked against originals.sha256 just now). By hand: remove $POOL/pool.tsv.* and $log.merging, then re-run"
      return 1
    fi
    for v in $VIEWS_ALL; do
      [[ ! -e "$POOL/pool.tsv.$v" ]] ||
        { say "refused: $POOL/pool.tsv.$v exists: a merged pool is never overwritten"; return 1; }
    done
    [[ ! -e "$log" ]] || { say "refused: $log exists: a merged pool is never overwritten"; return 1; }
  else
    [[ ! -e "$part1" ]] ||
      { say "refused: $part1 exists without originals.sha256: a move was interrupted; resolve it by hand"; return 1; }
    for name in $originals; do
      [[ -f "$POOL/$name" ]] || { say "refused: no $POOL/$name to move"; return 1; }
    done
    # Part 1's games file (`--games-out`, DEC-349) is part 1's alone: no merged one is
    # written, so it goes with the files it describes.
    [[ -f "$POOL/games.tsv" ]] && originals+=" games.tsv"
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
  rm -f "$log.merging"

  {
    echo "$MERGED_HEADER"
    echo "merged    $(date -Iseconds)"
    binary_line merger "$merger"
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

  for v in $VIEWS_ALL; do
    mv "$POOL/pool.tsv.$v.merging" "$POOL/pool.tsv.$v" || return 1
  done
  report=$(check_merged "$log.merging")
  rc=$?
  echo "$report" | tee -a "$log.merging"
  if (( rc != 0 )); then
    # This run's own files: they did not exist when it started.
    for v in $VIEWS_ALL; do rm -f "$POOL/pool.tsv.$v"; done
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

# The six stages on a slice of the 12000-game sample's pool, under 20000 rows a view.
smoke() {
  local src="$TUNING/s122/smoke" slice games
  OUT="$TUNING/s122/phase1/corpus_smoke"
  rm -rf "$OUT"; mkdir -p "$OUT/pool" || return 1
  slice="$OUT/pool"
  # The first games whose root rows stay under 20000; every view cut to them.
  games=$(awk -F'\t' '{ if (!($5 in seen)) { if (NR > 19000) exit; seen[$5] = 1; print $5 } }' \
            "$src/pool.tsv.root" | sort -n | tr '\n' ' ')
  for v in $VIEWS_ALL; do
    [[ -f "$src/pool.tsv.$v" ]] || { say "no $src/pool.tsv.$v"; return 1; }
    awk -F'\t' -v keep=" $games" 'index(keep, " " $5 " ") > 0' "$src/pool.tsv.$v" > "$slice/pool.tsv.$v"
  done
  printf 'smoke slice of %s\nS122-CORPUS-DONE\n' "$src" > "$slice/generate.log"
  POOL="$slice"; VALIDATION_GAMES=20; GAMES=12000
  all_stages
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
  smoke) if smoke; then echo "S122-CORPUS-SMOKE-DONE"; else echo "S122-CORPUS-SMOKE-FAILED"; exit 1; fi ;;
  *) echo "usage: $0 seeds | merge | verify | filter | split | sample | k | record | all | smoke" >&2; exit 2 ;;
esac
