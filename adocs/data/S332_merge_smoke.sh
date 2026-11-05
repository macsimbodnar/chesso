#!/usr/bin/env bash
# S332: `adocs/data/S122_corpus.sh merge` on two small runs of S331's datagen, and
# `adocs/data/S331_measure.py read` on what it writes. The pool merged after a stop
# (DEC-296) must carry one games file and one combined `repetition:` line that the
# reader accepts; before S332 the merge left part 1's games.tsv and the top-up's
# apart and the merged log no line to read.
#
#   adocs/data/S332_merge_smoke.sh runs     # the two runs, into DIR/pristine
#   adocs/data/S332_merge_smoke.sh merge    # a copy of them merged, then read
#   adocs/data/S332_merge_smoke.sh cases    # S331_measure.py merge on edited copies
#
# RUNS plays part 1 as S122_generate.sh's `corpus` frames it -- first line `S122
# corpus`, `datagen sha256`, `command`, datagen's stderr, `ended ..., exit N`,
# S122-CORPUS-FAILED -- `--games 60` under seed 990332000, stopped by SIGTERM to
# datagen alone STOP (15) seconds in, as a shutdown stops the corpus; then the
# top-up as `topup` frames it, `exit 0` and S122-TOPUP-DONE, the 60 games less the
# ones S082_pool_merge.py part1 keeps, under seed 990332100. Both at the corpus's
# settings, `--leaf root,qs,pv --nodes 100000 --threads 12 --quiet-limit 30000
# --games-out games.tsv`, by the worktree's build/tools/datagen (DATAGEN), nice 10.
# Under a minute on 12 threads. S122_generate.sh itself is not run: it refuses under
# 6 GB free, and the machine had 3. Refused if DIR/pristine exists; refused if part 1
# finished before the stop, since a pool that finished needs no merge. The seeds meet
# no run named in .tuning/s122/corpus/generate.log's `seeds` lines.
#
# MERGE copies DIR/pristine to DIR/merge, afresh, and runs S122_corpus.sh merge there
# with the overrides it has (TUNING, POOL, TOPUP, OUT, GAMES, CORPUS_SEED, TOPUP_SEED),
# then S331_measure.py read on the merged pool, and checks the pristine files
# unchanged. CASES runs S331_measure.py merge directly on copies of DIR/merge's
# inputs, then S122_corpus.sh on copies of DIR/pristine, each edited one way, and
# states what each must do; case C, power_loss, is part 1's games.tsv cut short as a
# power loss leaves it. Every stage's last line is S332-SMOKE-<STAGE>-DONE or -FAILED.
set -uo pipefail
{

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
DIR="${DIR:-$root/.tuning/s332/smoke}"
DATAGEN="${DATAGEN:-$root/build/tools/datagen}"
corpus_sh="$root/adocs/data/S122_corpus.sh"
reader="$root/adocs/data/S331_measure.py"
merger="$root/adocs/data/S082_pool_merge.py"
GAMES=60
CORPUS_SEED=990332000
TOPUP_SEED=990332100
STOP="${STOP:-15}"
NODES=100000
THREADS=12
QUIET=30000
LEAVES=root,qs,pv

say() { echo "$*"; }

# The marker even when the shell itself stops, on an unset variable under -u say: the
# cases' first run ended so, on a call missing a space, with no last line.
marker="S332-SMOKE-$(tr '[:lower:]' '[:upper:]' <<< "${stage:-none}")"
finished=0
trap '(( finished )) || echo "$marker-FAILED"' EXIT

# One run of DATAGEN in DIR framed into LOG as S122_generate.sh's play() frames it.
play() {
  local kind="$1" dir="$2" log="$3" games="$4" seed="$5" extra="$6" dpid rc
  (
    cd "$dir" || exit 1
    {
      if [[ "$kind" == corpus ]]; then echo "S122 corpus"; else echo "S122 corpus top-up"; fi
      echo "started   $(date -Is)"
      echo "engine    S332 smoke, $(git -C "$root" rev-parse --short HEAD)'s tree, build/tools/datagen"
      echo "datagen   sha256 $(sha256sum "$DATAGEN")"
      [[ -n "$extra" ]] && echo "$extra"
      echo "command   datagen --out pool.tsv --leaf $LEAVES --games $games --nodes $NODES --threads $THREADS --seed $seed --quiet-limit $QUIET --games-out games.tsv"
      nice -n 10 "$DATAGEN" --out pool.tsv --leaf "$LEAVES" --games "$games" --nodes "$NODES" \
          --threads "$THREADS" --seed "$seed" --quiet-limit "$QUIET" --games-out games.tsv &
      dpid=$!
      echo "$dpid" > "$dir/datagen.pid"
      if [[ "$kind" == corpus ]]; then
        ( sleep "$STOP"; kill -TERM "$dpid" 2>/dev/null ) &
      fi
      wait "$dpid"
      rc=$?
      if [[ "$kind" == corpus ]]; then
        echo "ended     $(date -Is), exit $rc"
        if (( rc == 0 )); then echo S122-CORPUS-DONE; else echo S122-CORPUS-FAILED; fi
      else
        echo "ended     $(date -Is)"
        echo "exit $rc"
        if (( rc == 0 )); then echo S122-TOPUP-DONE; else echo S122-TOPUP-FAILED; fi
      fi
    } > "$log" 2>&1
  )
}

runs() {
  local p="$DIR/pristine" kept
  [[ ! -e "$p" ]] || { say "refused: $p exists: the runs are never replayed"; return 1; }
  [[ -x "$DATAGEN" && "$("$DATAGEN" --help 2>&1)" == *--games-out* ]] ||
    { say "refused: $DATAGEN is not S331's datagen (no --games-out): build/ first"; return 1; }
  mkdir -p "$p/pool" "$p/topup" || return 1
  say "part 1    $GAMES games, seed $CORPUS_SEED, SIGTERM to datagen $STOP s in"
  play corpus "$p/pool" "$p/pool/generate.log" "$GAMES" "$CORPUS_SEED" ""
  grep -E '^(datagen:|ended|S122-)' "$p/pool/generate.log"
  [[ "$(tail -n 1 "$p/pool/generate.log")" == S122-CORPUS-FAILED ]] &&
    ! grep -q '^leaf root: ' "$p/pool/generate.log" ||
    { say "refused: part 1 finished before the stop; a larger GAMES or a shorter STOP"; return 1; }
  python3 "$merger" part1 "$p/pool/pool.tsv" "$p/pool/generate.log" > "$p/topup/part1.txt" ||
    { cat "$p/topup/part1.txt"; say "refused: S082_pool_merge.py part1 does not cut part 1"; return 1; }
  kept=$(sed -n 's/^part 1    cut: the \([0-9][0-9]*\) games before game .*/\1/p' "$p/topup/part1.txt")
  [[ "$kept" =~ ^[0-9]+$ ]] && (( kept < GAMES )) || { say "refused: no kept games in $p/topup/part1.txt"; return 1; }
  grep '^part 1    cut:' "$p/topup/part1.txt"
  say "top-up    $(( GAMES - kept )) games, seed $TOPUP_SEED"
  play topup "$p/topup" "$p/topup/topup.log" "$(( GAMES - kept ))" "$TOPUP_SEED" \
    "kept      $kept games of $p/pool by S082_pool_merge.py part1 ($p/topup/part1.txt); this run plays $GAMES - $kept"
  grep -E '^(datagen:|repetition:|exit|S122-)' "$p/topup/topup.log"
  [[ "$(tail -n 1 "$p/topup/topup.log")" == S122-TOPUP-DONE ]] || { say "the top-up failed"; return 1; }
  (cd "$p" && find pool topup -type f | sort | xargs sha256sum) > "$DIR/pristine.sha256" || return 1
  say "pristine  $(wc -l < "$DIR/pristine.sha256") files, sha256 in $DIR/pristine.sha256"
}

merge_stage() {
  local m="$DIR/merge" rc
  [[ -s "$DIR/pristine.sha256" ]] || { say "refused: no $DIR/pristine.sha256: run the runs stage first"; return 1; }
  (cd "$DIR/pristine" && sha256sum --quiet -c "$DIR/pristine.sha256") || { say "refused: the pristine runs changed"; return 1; }
  rm -rf "$m" && cp -a "$DIR/pristine" "$m" || return 1
  env TUNING="$m" POOL="$m/pool" TOPUP="$m/topup" OUT="$m/prepared" GAMES="$GAMES" \
      CORPUS_SEED="$CORPUS_SEED" TOPUP_SEED="$TOPUP_SEED" bash "$corpus_sh" merge > "$m/merge.console" 2>&1
  rc=$?
  say "merge     S122_corpus.sh merge, exit $rc, its last line $(tail -n 1 "$m/merge.console")"
  grep -E '^(merged |repetition: |part [12]    (games|repetition)|read      files|check: merged)' "$m/pool/generate.log" 2>/dev/null
  (( rc == 0 )) || { tail -n 5 "$m/merge.console"; return 1; }
  [[ -f "$m/pool/games.tsv" ]] || { say "refused: the merge wrote no $m/pool/games.tsv"; return 1; }
  say "games     $m/pool/games.tsv, $(wc -l < "$m/pool/games.tsv") lines"
  python3 "$reader" read "$m/pool" > "$m/reading.txt"
  rc=$?
  cat "$m/reading.txt"
  (( rc == 0 )) || { say "refused: S331_measure.py read does not accept the merged pool"; return 1; }
  (cd "$DIR/pristine" && sha256sum --quiet -c "$DIR/pristine.sha256") || { say "refused: the pristine runs changed"; return 1; }
  say "pristine  unchanged, every file against $DIR/pristine.sha256"
}

# S331_measure.py merge on a copy of the merge's inputs, one edit each: WANT is ok or
# a phrase its refusal must hold.
one_case() {
  local name="$1" want="$2" edit="$3" c="$DIR/cases/$1" offset out rc
  rm -rf "$c" && mkdir -p "$c" || return 1
  cp -a "$DIR/merge/pool/part1" "$c/part1" && cp -a "$DIR/merge/topup" "$c/topup" &&
    cp "$DIR/merge/pool/pool.tsv.root" "$c/merged.root" || return 1
  offset=$(sed -n "s/^offset    part 2's game ids plus \([0-9][0-9]*\),.*/\1/p" "$DIR/merge/pool/generate.log")
  ( cd "$c" && eval "$edit" ) || { say "case $name: the edit failed"; return 1; }
  out=$(python3 "$reader" merge "$offset" "$c/part1" "$c/topup" "$c/topup/topup.log" "$c/merged.root" "$c/games.tsv" 2>&1)
  rc=$?
  if [[ "$want" == ok ]]; then
    (( rc == 0 )) || { say "case $name: FAILED, refused: $out"; return 1; }
    say "case $name: accepted, $(grep -E '^merged games: ' <<< "$out" | sed 's/, sha256 [0-9a-f]*;/;/')"
    grep -E '^part 1    games: ' <<< "$out" | sed "s|$c/||"
  else
    (( rc != 0 )) && [[ "$out" == *"$want"* ]] || { say "case $name: FAILED, wanted a refusal naming '$want': $out"; return 1; }
    [[ ! -e "$c/games.tsv.merging" ]] || { say "case $name: FAILED, a refusal left $c/games.tsv.merging"; return 1; }
    say "case $name: refused, $(tail -n 1 <<< "$out" | sed "s|$c/||g")"
  fi
}

cases() {
  local failed=0 first last
  [[ -f "$DIR/merge/pool/games.tsv" ]] || { say "refused: no merged smoke pool: run the merge stage first"; return 1; }
  first=$(head -n 1 "$DIR/merge/pool/part1/games.tsv" | cut -f1)
  # The games line of the last game part 1's root keeps.
  last=$(awk -F'\t' -v off="$(sed -n "s/^offset    part 2's game ids plus \([0-9][0-9]*\),.*/\1/p" "$DIR/merge/pool/generate.log")" \
           '$5 < off { g = $5 } END { print g }' "$DIR/merge/pool/pool.tsv.root")
  one_case as_merged ok ':' || failed=1
  if cmp -s "$DIR/cases/as_merged/games.tsv.merging" "$DIR/merge/pool/games.tsv"; then
    say "case as_merged: its games file byte-equal to the merge's, $DIR/merge/pool/games.tsv"
  else
    say "case as_merged: FAILED, its games file is not the merge's"; failed=1
  fi
  one_case nul_tail ok 'printf "\0\0\0\0" >> part1/games.tsv' || failed=1
  one_case no_row_game_kept ok \
    "awk -F'\t' -v at=$last 'BEGIN { OFS = \"\t\" } { print } \$1 == at { print 99, \"0.5\", 8, -1, 8 }' part1/games.tsv > g && mv g part1/games.tsv" || failed=1
  one_case kept_game_missing "has no games line" "grep -v '^$first	' part1/games.tsv > g && mv g part1/games.tsv" || failed=1
  one_case partial_line "neither a line nor" 'printf "77\t1.0" >> part1/games.tsv' || failed=1
  one_case result_differs "not its games line's" \
    "awk -F'\t' 'BEGIN { OFS = \"\t\" } NR == 1 { \$2 = (\$2 == \"1.0\") ? \"0.0\" : \"1.0\" } { print }' part1/games.tsv > g && mv g part1/games.tsv" || failed=1
  one_case part2_short "games lines, its log books" "sed -i '\$d' topup/games.tsv" || failed=1
  one_case part2_line_off "part 2's files give" \
    "sed -i -E 's/^repetition: ([0-9]+) games/repetition: 9\\1 games/' topup/topup.log" || failed=1
  one_case id_at_offset "at or above the offset" \
    "awk -F'\t' -v at=$last 'BEGIN { OFS = \"\t\" } { print } \$1 == at { print 100000000, \"0.5\", 8, -1, 8 }' part1/games.tsv > g && mv g part1/games.tsv" || failed=1
  stage_case merge_no_row_game "S122-CORPUS-MERGE-DONE" merge \
    "awk -F'\t' -v at=$last 'BEGIN { OFS = \"\t\" } { print } \$1 == at { print 99, \"0.5\", 8, -1, 8 }' pool/games.tsv > g && mv g pool/games.tsv" || failed=1
  stage_case topup_no_games "no $DIR/cases/topup_no_games/topup/games.tsv" merge 'rm topup/games.tsv' || failed=1
  stage_case part1_no_games "no $DIR/cases/part1_no_games/pool/games.tsv" merge 'rm pool/games.tsv' || failed=1
  stage_case verify_games_changed "games.tsv's sha256 is not the one its merge wrote" verify \
    "cp -a '$DIR/merge/pool/.' pool/ && sed -i 's|$DIR/merge/pool/|$DIR/cases/verify_games_changed/pool/|g' pool/generate.log && sed -i '1s/\t/\t\t/' pool/games.tsv" || failed=1
  stage_case verify_as_merged "S122-CORPUS-VERIFY-DONE" verify \
    "cp -a '$DIR/merge/pool/.' pool/ && sed -i 's|$DIR/merge/pool/|$DIR/cases/verify_as_merged/pool/|g' pool/generate.log" || failed=1
  power_loss "$last" || failed=1
  return "$failed"
}

# Case C, the coordinator's fast check of 2026-11-05: a power loss keeps in each file
# what the OS had flushed, so part 1's games.tsv can end before games its views hold
# and the merger keeps. Here the lines of the dropped game and of LAST, the last kept
# one, are gone. The merge must pass with the views as merged without the loss, write
# what part 1's file has, give no combined `repetition:` line but one naming the kept
# games without a line, and `read` must refuse the pool's check as partial.
power_loss() {
  local last="$1" c="$DIR/cases/power_loss" dropped out rc v lines
  dropped=$(sed -n 's/^part 1    cut: the [0-9]* games before game \([0-9]*\) .*/\1/p' "$DIR/pristine/topup/part1.txt")
  rm -rf "$c" && cp -a "$DIR/pristine" "$c" || return 1
  grep -vE "^($dropped|$last)	" "$c/pool/games.tsv" > "$c/g" && mv "$c/g" "$c/pool/games.tsv" || return 1
  say "case power_loss: part 1's games.tsv without game $dropped's line, the dropped game's, and game $last's, the last kept"
  out=$(env TUNING="$c" POOL="$c/pool" TOPUP="$c/topup" OUT="$c/prepared" GAMES="$GAMES" \
          CORPUS_SEED="$CORPUS_SEED" TOPUP_SEED="$TOPUP_SEED" bash "$corpus_sh" merge 2>&1)
  rc=$?
  (( rc == 0 )) && [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-MERGE-DONE ]] ||
    { say "case power_loss: FAILED, the merge did not pass: $(grep -m 1 '^refused' <<< "$out" | sed "s|$c/||g")"; return 1; }
  for v in root qs pv; do
    cmp -s "$c/pool/pool.tsv.$v" "$DIR/merge/pool/pool.tsv.$v" ||
      { say "case power_loss: FAILED, pool.tsv.$v is not the views merged without the loss"; return 1; }
  done
  lines=$(wc -l < "$c/pool/games.tsv")
  (( lines == $(wc -l < "$DIR/merge/pool/games.tsv") - 1 )) ||
    { say "case power_loss: FAILED, games.tsv has $lines lines"; return 1; }
  ! grep -q '^repetition: ' "$c/pool/generate.log" && grep -q '^repetition incomplete: ' "$c/pool/generate.log" ||
    { say "case power_loss: FAILED, the merged log gives a combined repetition: line or no incomplete one"; return 1; }
  say "case power_loss: merge passed, the three views byte-equal to the merge's without the loss, games.tsv $lines lines"
  grep -E '^(merged games|repetition incomplete|read  )' "$c/pool/generate.log" | sed "s|$c/||g; s/, sha256 [0-9a-f]*;/;/"
  out=$(python3 "$reader" read "$c/pool" 2>&1)
  rc=$?
  (( rc != 0 )) && [[ "$out" == *partial* ]] ||
    { say "case power_loss: FAILED, read did not refuse the check as partial: $out"; return 1; }
  say "case power_loss: read refused, $(sed "s|$c/||g" <<< "$out")"
}

# S122_corpus.sh STAGE on a copy of the pristine runs, one edit each, refused naming
# WANT; for merge, nothing in the pool moved and no merged file written. A WANT that is
# a DONE marker is the stage passing, its last line.
stage_case() {
  local name="$1" want="$2" stage="$3" edit="$4" c="$DIR/cases/$1" before out rc
  rm -rf "$c" && cp -a "$DIR/pristine" "$c" || return 1
  ( cd "$c" && eval "$edit" ) || { say "case $name: the edit failed"; return 1; }
  before=$(cd "$c" && find pool topup | sort)
  out=$(env TUNING="$c" POOL="$c/pool" TOPUP="$c/topup" OUT="$c/prepared" GAMES="$GAMES" \
          CORPUS_SEED="$CORPUS_SEED" TOPUP_SEED="$TOPUP_SEED" bash "$corpus_sh" "$stage" 2>&1)
  rc=$?
  if [[ "$want" == *-DONE ]]; then
    (( rc == 0 )) && [[ "$(tail -n 1 <<< "$out")" == "$want" ]] ||
      { say "case $name: FAILED, wanted $stage to pass: $out"; return 1; }
    say "case $name: $stage passed, $(grep -E '^check: merged games' <<< "$out"), $want"
    return 0
  fi
  (( rc != 0 )) && [[ "$out" == *"$want"* ]] ||
    { say "case $name: FAILED, wanted $stage refused naming '$want': $out"; return 1; }
  if [[ "$stage" == merge && "$(cd "$c" && find pool topup | sort)" != "$before" ]]; then
    say "case $name: FAILED, the refused merge moved or wrote a file"; return 1
  fi
  [[ "$stage" != verify || ! -e "$c/prepared/verify.txt" ]] ||
    { say "case $name: FAILED, a refused verify left verify.txt"; return 1; }
  say "case $name: $stage refused, $(grep -m 1 -F "$want" <<< "$out" | sed "s|$c/||g")"
}

case "$stage" in
  runs) runs ;;
  merge) merge_stage ;;
  cases) cases ;;
  *) echo "usage: $0 runs | merge | cases" >&2; exit 2 ;;
esac
rc=$?
finished=1
if (( rc == 0 )); then echo "$marker-DONE"; exit 0; fi
echo "$marker-FAILED"
exit 1
}
