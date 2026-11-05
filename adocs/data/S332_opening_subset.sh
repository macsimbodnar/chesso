#!/usr/bin/env bash
# S332: the streaming reader against HEAD's on games whose opening ended on a recurrence.
# S331's 2400-game run has none, so its byte-equal re-run never takes the reader's
# `kind == "opening"` branch; S122's pool has 25 (the coordinator's fast check,
# 2026-11-05, which closed the gap by hand and asked for it recorded).
#
#   POOL=/home/max/ws/chesso/.tuning/s122/corpus adocs/data/S332_opening_subset.sh
#
# POOL (default .tuning/s122/corpus) is read in place, games.tsv and pool.tsv.root only.
# The subset is every game whose opening ended on a recurrence (games.tsv's fourth column
# equal to its third) and every EVERY-th (500th) other game in file order, with all their
# root rows, written under SCRATCH (.tuning/s332/opening_subset, a few MB) and deleted at
# the end, pass or fail. Its log is made for the subset: a settings line booking its
# games, a `leaf root:` line counting its rows, and a `repetition:` line computed by awk
# from the subset's own files, a third reckoning beside the two readers'. Then, on (a) the
# subset in file order and (b) its games.tsv and root rows each shuffled (Python's
# random.Random(332)), HEAD's reader -- `git show bcc41f3:adocs/data/S331_measure.py` --
# and the worktree's read the same directory, and their outputs are compared by cmp. The
# subset's settings line is never printed: S122_corpus.sh seeds reads every text file
# under adocs/data for one, and it names no run. Last line S332-OPENING-SUBSET-DONE or
# -FAILED. Seconds: one awk pass over the pool's root view.
set -uo pipefail
{

root="$(cd "$(dirname "$0")/../.." && pwd)"
POOL="${POOL:-$root/.tuning/s122/corpus}"
SCRATCH="${SCRATCH:-$root/.tuning/s332/opening_subset}"
EVERY=500
HEAD_COMMIT=bcc41f3
reader="$root/adocs/data/S331_measure.py"
marker=S332-OPENING-SUBSET
finished=0
made=0
trap '(( made )) && rm -rf -- "$SCRATCH"; (( finished )) || echo "$marker-FAILED"' EXIT
fail() { echo "$*"; exit 1; }

[[ -r "$POOL/games.tsv" && -r "$POOL/pool.tsv.root" && -r "$POOL/generate.log" ]] ||
  fail "refused: no games.tsv, pool.tsv.root and generate.log in $POOL"
[[ ! -e "$SCRATCH" ]] || fail "refused: $SCRATCH exists; it is this script's and is removed at its end"
mkdir -p "$SCRATCH/a" "$SCRATCH/b" && made=1 || fail "cannot create $SCRATCH"
git -C "$root" show "$HEAD_COMMIT:adocs/data/S331_measure.py" > "$SCRATCH/head_reader.py" ||
  fail "cannot read $HEAD_COMMIT's reader"

echo "pool      $POOL, read in place: games.tsv sha256 $(sha256sum "$POOL/games.tsv" | cut -d' ' -f1)"
echo "readers   HEAD's, $HEAD_COMMIT:adocs/data/S331_measure.py, sha256 $(sha256sum "$SCRATCH/head_reader.py" | cut -d' ' -f1)"
echo "          the worktree's, sha256 $(sha256sum "$reader" | cut -d' ' -f1)"

# The subset's games, in the pool's order.
awk -F'\t' -v every="$EVERY" '$4 == $3 { print; next } { if (++n % every == 0) print }' \
  "$POOL/games.tsv" > "$SCRATCH/a/games.tsv" || fail "the games selection failed"
awk -F'\t' 'NR == FNR { keep[$1] = 1; next } ($5 in keep)' \
  "$SCRATCH/a/games.tsv" "$POOL/pool.tsv.root" > "$SCRATCH/a/pool.tsv.root" || fail "the rows selection failed"

# The repetition: figures from the subset's files, by awk: a game recurred after its
# opening when column 4 >= 0 and differs from column 3, an opening when they are equal.
read -r games openings recurred won lost < <(awk -F'\t' '
  { g++ } $4 == $3 { o++ } $4 >= 0 && $4 != $3 { r++; if ($2 == "1.0") w++; if ($2 == "0.0") l++ }
  END { print g, o + 0, r + 0, w + 0, l + 0 }' "$SCRATCH/a/games.tsv")
read -r rows in_recurred before < <(awk -F'\t' '
  NR == FNR { if ($4 >= 0 && $4 != $3) at[$1] = $4; next }
  { n++ } ($5 in at) { r++; if ($6 < at[$5]) b++ }
  END { print n, r + 0, b + 0 }' "$SCRATCH/a/games.tsv" "$SCRATCH/a/pool.tsv.root")
line="repetition: $recurred games reached a position that had occurred before, which drew a game until S331; $(( won + lost )) of them ended other than drawn, $won won by White and $lost lost; $in_recurred positions passed in them, a root row each, $before before the first recurrence; $openings openings ended on one, discarded until S331"
{
  echo "S332 opening subset of $POOL: not a datagen run's log"
  sed -n 's/^datagen: [0-9]* games, \(.*\), leaf .*$/datagen: '"$games"' games, \1, leaf root/p' "$POOL/generate.log"
  echo "leaf root: $rows rows from $games games written to pool.tsv.root, the subset's"
  echo "$line"
} > "$SCRATCH/a/generate.log"
[[ "$(grep -c '^datagen: ' "$SCRATCH/a/generate.log")" == 1 ]] || fail "no settings line made from $POOL/generate.log"
echo "subset    $games games, every one whose opening ended on a recurrence ($openings) and every ${EVERY}th other; $rows root rows"
echo "awk       $line"

python3 - "$SCRATCH/a" "$SCRATCH/b" <<'EOF' || fail "the shuffle failed"
import random, shutil, sys
a, b = sys.argv[1:]
rng = random.Random(332)
for name in ("games.tsv", "pool.tsv.root"):
    lines = open(f"{a}/{name}").readlines()
    rng.shuffle(lines)
    open(f"{b}/{name}", "w").writelines(lines)
shutil.copy(f"{a}/generate.log", f"{b}/generate.log")
EOF

failed=0
for set in a b; do
  python3 "$SCRATCH/head_reader.py" read "$SCRATCH/$set" > "$SCRATCH/$set.head.txt" 2>&1; rh=$?
  python3 "$reader" read "$SCRATCH/$set" > "$SCRATCH/$set.new.txt" 2>&1; rn=$?
  name=$([[ "$set" == a ]] && echo "in file order" || echo "games.tsv and root rows shuffled")
  if (( rh == 0 && rn == 0 )) && cmp -s "$SCRATCH/$set.head.txt" "$SCRATCH/$set.new.txt"; then
    echo "($set)       $name: both exit 0, byte-equal, sha256 $(sha256sum < "$SCRATCH/$set.new.txt" | cut -d' ' -f1)"
  else
    echo "($set)       $name: HEAD's exit $rh, the worktree's exit $rn, not byte-equal"
    diff "$SCRATCH/$set.head.txt" "$SCRATCH/$set.new.txt" | head -20
    failed=1
  fi
done
cmp -s "$SCRATCH/a.new.txt" "$SCRATCH/b.new.txt" && same="the same bytes" || same="not the same bytes"
echo "(a), (b)  $same: the shuffle reorders the games and so the sums behind the clustered errors"
echo "reading   (a), the worktree's, verbatim, its directory removed at the end:"
cat "$SCRATCH/a.new.txt"
(( failed == 0 )) || fail "the readers disagree"
finished=1
echo "$marker-DONE"
exit 0
}
