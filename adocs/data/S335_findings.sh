#!/usr/bin/env bash
# S335: S332's three low findings in the corpus merge and its reader (S332's done file,
# "Findings beyond the step"), each on a fixture seen red at S332's landing and green at
# the tree's scripts, or, for the first, the evidence for leaving the count as it is.
#
#   adocs/data/S335_findings.sh red       # the cases by 65463ae's S122_corpus.sh and S331_measure.py
#   BEFORE=d0cb418 adocs/data/S335_findings.sh red   # the same by S335's parent's
#   adocs/data/S335_findings.sh green     # the cases by the tree's
#   adocs/data/S335_findings.sh reading   # S331's 2400-game reading by the tree's reader, byte-equal
#
# DIR is adocs/data/S332_merge_smoke.sh's: the two small runs its `runs` stage played into
# DIR/pristine, checked against DIR/pristine.sha256 and never written. Each case works on
# a copy under DIR/s335/<stage>/<case>, red_<BEFORE> for red, made afresh on every run.
#
#   plain          S122_corpus.sh merge on a copy as played, the reference for the rest.
#   no_row_game    finding 1: a part 1 game that wrote no row, its games line before the
#                  cut, as S332's merge_no_row_game plants it. The step keeps the count
#                  (its step file says why) and this is the evidence, the same at both
#                  scripts: the merge passes; its three views are the plain merge's bytes,
#                  so every stage after it reads the same pool; its games file is the plain
#                  one and that game's line; and GAMES, on the merger's lines, is part 1's
#                  kept games with a row plus the top-up's played, the count the top-up
#                  was booked by (its log's `kept` line and its `--games`).
#   read_*, run_*  finding 2: S331_measure.py read on a copy with one field not a number,
#                  in the games file or a view, of the merged pool or of the top-up read
#                  as one run: exit 1 and one line, `refused: ...` naming FILE:LINE.
#   stop_before    finding 3: the merge stopped by SIGKILL, as a power loss stops it, at
#                  its call of S331_measure.py merge, the merger's .merging files written
#                  and none renamed. The next run must refuse naming a stop before the
#                  rename, moving and writing nothing; then the recovery it names, done
#                  here by hand, must give the plain merge's views and games file.
#   stop_after     the same stop at its call of S331_measure.py read, the merged files
#                  renamed and the log not: the refusal naming a stop after the rename, as
#                  before S335, and its recovery.
#
# The stop is a python3 first on the merge's PATH, the case's own, that runs the real
# one for every call but the one named; there it kills each process up its parent chain
# that runs the corpus script's merge, the merge and any subshell of it, and exits 137.
# It reads the chain from /proc, so the two stop cases run on Linux only, as the corpus
# script's own `stat -c` does.
#
# `red` writes BEFORE's two scripts, 65463ae's by default, beside the tree's as .S335_before_corpus.sh and
# .S335_before_measure.py, so their root is the tree's, and removes them on exit. That
# corpus script runs the tree's S331_measure.py as its reader, as every S122_corpus.sh
# does, so the merges here read with it; the read cases call the before copy direct. At
# 65463ae merge() and check_merged() are d0cb418's, S335's parent, line for line, and
# S331_measure.py is d0cb418's. `red` passes when exactly the cases RED names fail and the rest pass; `green`
# when every case passes. Each stage prints S335-FINDINGS-<STAGE>-DONE or -FAILED as its
# last line. Under a minute.
set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
DIR="${DIR:-$root/.tuning/s335/smoke}"
[[ "$DIR" == /* ]] || DIR="$PWD/$DIR"
BEFORE="${BEFORE:-65463ae}"
GAMES=60
CORPUS_SEED=990332000
TOPUP_SEED=990332100
CASES="plain no_row_game read_games_id read_games_result read_games_ply read_view_score read_view_game run_view_ply stop_before stop_after"
RED="read_games_id read_games_result read_games_ply read_view_score read_view_game run_view_ply stop_before"
real_python="$(command -v python3)"
before_sh="$root/adocs/data/.S335_before_corpus.sh"
before_py="$root/adocs/data/.S335_before_measure.py"

say() { echo "$*"; }

marker="S335-FINDINGS-$(tr '[:lower:]' '[:upper:]' <<< "${stage:-none}")"
finished=0
cleanup() {
  rm -f "$before_sh" "$before_py"
  (( finished )) || echo "$marker-FAILED"
}
trap cleanup EXIT

fresh() { rm -rf "$1" && mkdir -p "$(dirname "$1")" && cp -a "$DIR/pristine" "$1"; }

# Every file under C's pool and topup, by name and sha256: what a refused run must leave.
manifest() { (cd "$1" && find pool topup -type f -print0 | sort -z | xargs -0 sha256sum); }

# S122_corpus.sh merge on the copy C, as S332_merge_smoke.sh runs it; ENV first.
merge_on() {
  local c="$1"
  shift
  env "$@" TUNING="$c" POOL="$c/pool" TOPUP="$c/topup" OUT="$c/prepared" GAMES="$GAMES" \
      CORPUS_SEED="$CORPUS_SEED" TOPUP_SEED="$TOPUP_SEED" bash "$corpus_sh" merge
}

offset_of() { sed -n "s/^offset    part 2's game ids plus \([0-9][0-9]*\),.*/\1/p" "$1/generate.log" | tail -n 1; }

case_plain() {
  local c="$W/plain" out rc
  fresh "$c" || return 1
  out=$(merge_on "$c" 2>&1)
  rc=$?
  (( rc == 0 )) && [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-MERGE-DONE ]] ||
    { say "  the merge did not pass: $(grep -m 1 '^refused' <<< "$out")"; return 1; }
  say "  $(grep -E '^merged games: ' "$c/pool/generate.log" | sed "s|$c/||; s/, sha256 [0-9a-f]*;/;/")"
}

case_no_row_game() {
  local c="$W/no_row_game" p="$W/plain/pool" out rc last v kept topped line
  [[ -f "$p/games.tsv" ]] || { say "  no plain merge to compare with"; return 1; }
  fresh "$c" || return 1
  last=$(awk -F'\t' -v off="$(offset_of "$p")" '$5 < off { g = $5 } END { print g }' "$p/pool.tsv.root")
  awk -F'\t' -v at="$last" 'BEGIN { OFS = "\t" } { print } $1 == at { print 99, "0.5", 8, -1, 8 }' \
    "$c/pool/games.tsv" > "$c/g" && mv "$c/g" "$c/pool/games.tsv" || return 1
  out=$(merge_on "$c" 2>&1)
  rc=$?
  (( rc == 0 )) && [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-MERGE-DONE ]] ||
    { say "  the merge did not pass: $(grep -m 1 '^refused' <<< "$out")"; return 1; }
  for v in root qs pv; do
    cmp -s "$c/pool/pool.tsv.$v" "$p/pool.tsv.$v" || { say "  pool.tsv.$v is not the plain merge's"; return 1; }
  done
  say "  merge passed; pool.tsv.root, .qs and .pv byte-equal to the plain merge's"
  cmp -s <(grep -v $'^99\t' "$c/pool/games.tsv") "$p/games.tsv" &&
    (( $(wc -l < "$c/pool/games.tsv") == $(wc -l < "$p/games.tsv") + 1 )) ||
    { say "  games.tsv is not the plain merge's and game 99's line"; return 1; }
  say "  games.tsv $(wc -l < "$c/pool/games.tsv") lines: the plain merge's $(wc -l < "$p/games.tsv") and game 99's, placed after game $last's"
  # The top-up's booking: GAMES less the games part 1 keeps by its rows.
  kept=$(sed -n 's/^kept      \([0-9][0-9]*\) games of .* this run plays [0-9]* - [0-9]*$/\1/p' "$c/topup/topup.log")
  topped=$(sed -n 's/^command .* --games \([0-9][0-9]*\) .*/\1/p' "$c/topup/topup.log")
  [[ -n "$kept" && -n "$topped" ]] && (( kept + topped == GAMES )) ||
    { say "  the top-up's log books '$topped' games for '$kept' kept, not $GAMES"; return 1; }
  say "  top-up booked $topped games, $GAMES less part 1's $kept kept by its rows (topup.log's kept and command lines)"
  line=$(grep -E "^merged root: [0-9]+ rows from $GAMES games written to " "$c/pool/generate.log")
  [[ "$line" == *"; part 1's $kept games with a row before its cut and part 2's $topped games played, "* ]] ||
    { say "  the merger's root line is not $GAMES = $kept + $topped: $line"; return 1; }
  say "  $(sed -E "s|$c/||; s/, sha256 [0-9a-f]*;/;/" <<< "$line")"
  line=$(grep -E '^merged games: ' "$c/pool/generate.log")
  [[ "$line" == "merged games: $(( kept + 1 + topped )) games written to "*"; part 1's $kept games with a row before its cut and 1 without, and part 2's $topped games played" ]] ||
    { say "  the merged games line does not count game 99 apart: $line"; return 1; }
  say "  $(sed -E "s|$c/||; s/, sha256 [0-9a-f]*;/;/" <<< "$line")"
}

# S331_measure.py read on a copy of SRC with field F of line L of FILE set to x; LOG, if
# given, is the log read. Exit 1 and one line naming FILE:L: exactly, no traceback.
read_case() {
  local name="$1" src="$2" file="$3" l="$4" f="$5" log="${6:-}" c="$W/$1" out rc n
  rm -rf "$c" && mkdir -p "$c" && cp -a "$src/." "$c/" || return 1
  awk -F'\t' -v l="$l" -v f="$f" 'BEGIN { OFS = "\t" } NR == l { $f = "x" } { print }' \
    "$c/$file" > "$c/edit" && mv "$c/edit" "$c/$file" || return 1
  say "  $file line $l, field $f set to x"
  if [[ -n "$log" ]]; then
    out=$("$real_python" "$reader" read "$c" "$c/$log" 2>&1)
  else
    out=$("$real_python" "$reader" read "$c" 2>&1)
  fi
  rc=$?
  n=$(wc -l <<< "$out")
  if (( rc == 1 && n == 1 )) && [[ "$out" == "refused: "* && "$out" == *"$c/$file:$l: "* ]]; then
    say "  exit 1, one line: $(sed "s|$c/||g" <<< "$out")"
    return 0
  fi
  say "  exit $rc, $n lines, the last: $(tail -n 1 <<< "$out" | sed "s|$c/||g")"
  return 1
}

case_read_games_id() { read_case read_games_id "$W/plain/pool" games.tsv 1 1; }
case_read_games_result() { read_case read_games_result "$W/plain/pool" games.tsv 1 2; }
case_read_games_ply() { read_case read_games_ply "$W/plain/pool" games.tsv 2 3; }
case_read_view_score() { read_case read_view_score "$W/plain/pool" pool.tsv.qs 5 3; }
case_read_view_game() { read_case read_view_game "$W/plain/pool" pool.tsv.root 7 5; }
case_run_view_ply() { read_case run_view_ply "$DIR/pristine/topup" pool.tsv.pv 3 6 topup.log; }

# C/bin/python3: the real python3, but at S331_measure.py MODE the merge is stopped.
stopper() {
  mkdir -p "$1/bin" || return 1
  sed -e "s|@MODE@|$2|" -e "s|@PYTHON@|$real_python|" > "$1/bin/python3" <<'EOF'
#!/usr/bin/env bash
# S335_findings.sh's stop: SIGKILL to the corpus script's merge, as a power loss stops
# it, when it calls S331_measure.py @MODE@; every other call is the real python3's.
if [[ "${1##*/}" == S331_measure.py && "${2:-}" == @MODE@ ]]; then
  chain=()
  p=$PPID
  while (( p > 1 )); do
    chain+=("$p")
    s=$(< "/proc/$p/stat") || break
    s=${s##*) }
    s=${s#* }
    p=${s%% *}
  done
  hit=0
  for p in "${chain[@]}"; do
    if tr '\0' ' ' < "/proc/$p/cmdline" | grep -q 'corpus\.sh merge'; then
      kill -KILL "$p"
      hit=1
    elif (( hit )); then
      break
    fi
  done
  exit 137
fi
exec @PYTHON@ "$@"
EOF
  chmod +x "$1/bin/python3"
}

# The merge stopped at S331_measure.py MODE on a fresh copy C, then run again: WANT is
# the phrase its refusal must hold, SHUN one it must not; nothing may move. Then the
# recovery, REMOVE (globs under C/pool) removed by hand, and the merge run once more
# must pass with the plain merge's views and games file.
stop_case() {
  local c="$W/$1" mode="$2" state="$3" want="$4" shun="$5" remove="$6" p="$W/plain/pool"
  local out rc v before refusal g globs
  [[ -f "$p/games.tsv" ]] || { say "  no plain merge to compare with"; return 1; }
  fresh "$c" && stopper "$c" "$mode" || return 1
  out=$( { merge_on "$c" PATH="$c/bin:$PATH"; } 2>&1)
  rc=$?
  (( rc == 137 )) || { say "  the merge was not stopped: exit $rc, $(tail -n 1 <<< "$out")"; return 1; }
  [[ -e "$c/pool/generate.log.merging" && ! -e "$c/pool/generate.log" && -e "$c/pool/part1/originals.sha256" ]] ||
    { say "  not the state of a merge stopped mid-way: generate.log.merging, part1/originals.sha256 and no generate.log"; return 1; }
  for v in root qs pv; do
    if [[ "$state" == before ]]; then
      [[ -e "$c/pool/pool.tsv.$v.merging" && ! -e "$c/pool/pool.tsv.$v" ]] ||
        { say "  pool.tsv.$v: not the merger's .merging file alone"; return 1; }
    else
      [[ -e "$c/pool/pool.tsv.$v" && ! -e "$c/pool/pool.tsv.$v.merging" ]] ||
        { say "  pool.tsv.$v: not renamed"; return 1; }
    fi
  done
  if [[ "$state" == before ]]; then
    [[ ! -e "$c/pool/games.tsv" && ! -e "$c/pool/games.tsv.merging" ]] || { say "  a games file exists"; return 1; }
  else
    [[ -e "$c/pool/games.tsv" && ! -e "$c/pool/games.tsv.merging" ]] || { say "  games.tsv not renamed"; return 1; }
  fi
  say "  stopped, exit 137, at its call of S331_measure.py $mode: $(cd "$c/pool" && ls -d pool.tsv.* games.tsv* generate.log* 2>/dev/null | tr '\n' ' ')"
  before=$(manifest "$c")
  out=$(merge_on "$c" 2>&1)
  rc=$?
  refusal=$(grep -m 1 '^refused: an interrupted merge' <<< "$out")
  [[ "$(manifest "$c")" == "$before" ]] || { say "  the run after the stop moved or wrote a file"; return 1; }
  if (( rc == 0 )) || [[ "$(tail -n 1 <<< "$out")" != S122-CORPUS-MERGE-FAILED || -z "$refusal" ]]; then
    say "  the run after the stop: exit $rc, no interrupted-merge refusal: $(grep -m 1 '^refused' <<< "$out" | sed "s|$c/||g")"
    return 1
  fi
  say "  run again: $(sed "s|$c/||g" <<< "$refusal")"
  [[ "$refusal" == *"$want"* ]] || { say "  the refusal does not say '$want'"; return 1; }
  [[ "$refusal" != *"$shun"* ]] || { say "  the refusal says '$shun'"; return 1; }
  say "  nothing moved or written: pool/ and topup/ by name and sha256 as before the run"
  read -ra globs <<< "$remove"
  for g in "${globs[@]}"; do
    # shellcheck disable=SC2086
    rm -f "$c/pool/"$g || return 1
  done
  out=$(merge_on "$c" 2>&1)
  rc=$?
  (( rc == 0 )) && [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-MERGE-DONE ]] ||
    { say "  after the recovery the merge did not pass: $(grep -m 1 '^refused' <<< "$out" | sed "s|$c/||g")"; return 1; }
  for v in pool.tsv.root pool.tsv.qs pool.tsv.pv games.tsv; do
    cmp -s "$c/pool/$v" "$p/$v" || { say "  after the recovery $v is not the plain merge's"; return 1; }
  done
  say "  recovery, removing $remove by hand: the merge passes, its views and games.tsv byte-equal to the plain merge's"
}

case_stop_before() {
  stop_case stop_before merge before "stopped before renaming its merged files" "after renaming its merged files" "*.merging"
}
case_stop_after() {
  stop_case stop_after read after "after renaming its merged files" "before renaming its merged files" \
    "pool.tsv.root pool.tsv.qs pool.tsv.pv games.tsv generate.log.merging"
}

# Every case in CASES; RED_WANTED is the set that must fail, empty for green.
run_cases() {
  local red_wanted="$1" name ok=0 bad=0
  [[ -s "$DIR/pristine.sha256" ]] || { say "refused: no $DIR/pristine.sha256: run S332_merge_smoke.sh runs with this DIR first"; return 1; }
  (cd "$DIR/pristine" && sha256sum --quiet -c "$DIR/pristine.sha256") || { say "refused: $DIR/pristine changed"; return 1; }
  say "scripts   $corpus_sh, sha256 $(sha256sum "$corpus_sh" | cut -d' ' -f1)"
  say "scripts   $reader, sha256 $(sha256sum "$reader" | cut -d' ' -f1)"
  for name in $CASES; do
    say "case $name"
    if "case_$name"; then
      if [[ " $red_wanted " == *" $name "* ]]; then say "case $name: passes, not red"; bad=$(( bad + 1 )); else say "case $name: passes, as it must"; ok=$(( ok + 1 )); fi
    else
      if [[ " $red_wanted " == *" $name "* ]]; then say "case $name: red, as it must be"; ok=$(( ok + 1 )); else say "case $name: FAILED"; bad=$(( bad + 1 )); fi
    fi
  done
  say "cases     $(( ok + bad )) cases, $ok as they must be, $bad not"
  (cd "$DIR/pristine" && sha256sum --quiet -c "$DIR/pristine.sha256") || { say "refused: $DIR/pristine changed"; return 1; }
  (( bad == 0 ))
}

red() {
  W="$DIR/s335/red_$BEFORE"
  git -C "$root" show "$BEFORE:adocs/data/S122_corpus.sh" > "$before_sh" &&
    git -C "$root" show "$BEFORE:adocs/data/S331_measure.py" > "$before_py" || return 1
  corpus_sh="$before_sh"
  reader="$before_py"
  say "red       $BEFORE's scripts ($(git -C "$root" log -1 --format='%h %s' "$BEFORE")); the cases RED names must fail: $RED"
  run_cases "$RED"
}

green() {
  W="$DIR/s335/green"
  corpus_sh="$root/adocs/data/S122_corpus.sh"
  reader="$root/adocs/data/S331_measure.py"
  run_cases ""
}

# S331's reading of its 2400-game run, by the tree's reader, against the reading S331
# wrote and adocs/data/S331_reading.txt's copy from its `S331 reading of` line on; OUT is
# the path string S331 read, so the first line can be the same bytes.
reading() {
  local out="$root/.tuning/s331/measure" got
  got=$(mktemp) || return 1
  python3 "$root/adocs/data/S331_measure.py" read "$out" > "$got" || { cat "$got"; rm -f "$got"; return 1; }
  say "reading   $(sha256sum < "$got" | cut -d' ' -f1), the tree's reader on $out"
  say "reading   $(sha256sum < "$out/reading.txt" | cut -d' ' -f1), $out/reading.txt"
  say "reading   $(sed -n '/^S331 reading of/,$p' "$root/adocs/data/S331_reading.txt" | sha256sum | cut -d' ' -f1), adocs/data/S331_reading.txt from its 'S331 reading of' line"
  cmp "$got" "$out/reading.txt" && cmp "$got" <(sed -n '/^S331 reading of/,$p' "$root/adocs/data/S331_reading.txt")
  local rc=$?
  rm -f "$got"
  (( rc == 0 )) && say "reading   byte-equal to both"
}

case "$stage" in
  red) red ;;
  green) green ;;
  reading) reading ;;
  *) echo "usage: $0 red | green | reading" >&2; finished=1; exit 2 ;;
esac
rc=$?
finished=1
if (( rc == 0 )); then echo "$marker-DONE"; exit 0; fi
echo "$marker-FAILED"
exit 1
