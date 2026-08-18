#!/usr/bin/env bash
set -euo pipefail

# S088's anchor sweep, solved by hand.
#
# rating.sh exits on RATING-RUN-INVALID *before* it reaches the sweep, and
# DEC-076 tolerates this run's one forfeit rather than voiding it. So the solve
# that rating.sh would have run is issued here instead, with the identical
# command, and this file is the reproducible record. DEC-076 decision 4.
#
#   adocs/data/S088_solve.sh <games.pgn> [outdir]
#
# Anchors are NOT hardcoded here: they are read from the CCRL Blitz list at run
# time by tools/ccrl_rating.py, exactly as rating.sh does, and a name that is not
# on the list stops the solve. S087, DEC-068.

pgn="${1:?usage: S088_solve.sh <games.pgn> [outdir]}"
out="${2:-$(dirname "$pgn")}"
repo="$(cd "$(dirname "$0")/../.." && pwd)"

[[ -r "$pgn" ]] || { echo "no pgn at $pgn" >&2; exit 1; }
mkdir -p "$out"
command -v ordo > /dev/null || { echo "ordo not on PATH" >&2; exit 1; }

# uci_name -> ccrl_name, from the manifest, so this cannot drift from what was
# played. Stash is the row where the two differ: `Stash v21.0` / `Stash 21.0`.
mapfile -t rows < <(awk -F'\t' '!/^#/ && NF >= 3 { print $1 "\t" $3 }' "$repo/references.tsv")
((${#rows[@]} >= 3)) || { echo "manifest has ${#rows[@]} engines" >&2; exit 1; }

ccrl_names=()
for row in "${rows[@]}"; do ccrl_names+=("${row#*$'\t'}"); done

anchors="$out/anchors.tsv"
"$repo/tools/ccrl_rating.py" --cache "$out/ccrl_list.html" "${ccrl_names[@]}" > "$anchors"

echo "=== anchors, read from the CCRL Blitz list ==="
cat "$anchors"

echo
echo "=== terminations, and the forfeit rate per engine (DEC-075, DEC-076) ==="
grep -h '^\[Termination' "$pgn" | sort | uniq -c | sort -rn
"$repo/tools/forfeit_report.py" "$pgn" --max-pct "${FORFEIT_MAX_PCT:-1.0}" || true

echo
echo "=== chesso's solved rating, once per anchor choice ==="
for row in "${rows[@]}"; do
  uci="${row%%$'\t'*}"; ccrl="${row#*$'\t'}"
  rating="$(awk -F'\t' -v n="$ccrl" '$1 == n { print $2 }' "$anchors")"
  [[ -n "$rating" ]] || { echo "no anchor rating for $ccrl" >&2; exit 1; }
  solved="$out/ordo_anchored_${uci// /_}.txt"
  ordo -q -p "$pgn" -o "$solved" -a "$rating" -A "$uci" -s 1000 -F 95 -n 12 -W -D
  line="$(grep -E '^\s*[0-9]+\s+chesso' "$solved" || true)"
  echo "anchored on $uci at $rating (CCRL $ccrl):  $line"
done

echo
echo "=== head to head ==="
ordo -q -p "$pgn" -o "$out/ordo_free.txt" -s 100 -F 95 -n 12 -W -D -j "$out/h2h.txt"
cat "$out/h2h.txt"

echo
echo "ordo intervals are trinomial. SPRT verdicts here run model=normalized and"
echo "report nElo. The two are never quoted against each other."
echo "S088-SOLVE-DONE"
