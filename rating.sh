#!/usr/bin/env bash
set -euo pipefail

# Plays chesso in a gauntlet against reference engines with published CCRL
# Blitz ratings and solves the result into an absolute rating with ordo.
#
#   ./rating.sh --bracket        short run, no anchors, checks the set brackets
#   ./rating.sh                  rated run, solves and sweeps the anchors
#   TC=2+1 ./rating.sh           pick the time control
#
# S087, DEC-067, DEC-068.
#
# WHAT THIS IS FOR, AND WHAT fastchess.sh IS FOR. fastchess.sh answers "is this
# commit stronger than that one" and is the per-change gate. It cannot answer
# "how strong is chesso", because both sides of it are chesso. This answers that
# and is not a gate: it is run after a milestone, not before a commit.
#
# The two numbers are not comparable. ordo's intervals are trinomial; every SPRT
# verdict here runs model=normalized and reports nElo. Never quote one against
# the other.

repo="$(cd "$(dirname "$0")" && pwd)"
manifest="${MANIFEST:-$repo/references.tsv}"
book="$repo/books/8moves_v3.pgn"
candidate="$repo/build/src/chesso"

tc="${TC:-10+0.2}"
hash_mb=64

# Every core the machine reports, DEC-050, exactly as fastchess.sh. CONCURRENCY
# overrides.
#
# DEC-067 accepted this default against foreign engines knowingly: oversubscription
# need not hit both sides equally, because the side with the thinner
# time-management margin forfeits first. DEC-073 then dropped it to 6 after three
# forfeits, and DEC-075 put it back -- the owner's call to saturate the machine,
# and the drop was measured to buy nothing. Stash v21.0 forfeited 3 of its 668
# games at concurrency 12 (0.45 %) and 1 of 668 at concurrency 6 (0.15 %), and the
# concurrency-6 forfeit was a 25360 ms hang rather than a margin overrun. Load was
# never the cause. Concurrency 6 also costs 2.1x: 6.2 of 12 threads busy and
# 10.6 games/min against 22.6, measured on this machine.
all_cores="$(nproc)"
concurrency="${CONCURRENCY:-$all_cores}"

# Adjudication, as fastchess.sh. Both are two-sided -- a resign needs both
# engines to agree for movecount moves -- so an engine whose evaluation is on a
# different scale cannot trigger one alone.
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"

# ROUNDS is rounds PER PAIRING, and each round is two games with the colours
# reversed. The total is rounds x pairings x 2, so it grows with the manifest
# and is not a constant to be read off here; the run prints the derived total
# before it plays. Both defaults were quoted against a three-engine set until
# S088 and were already wrong for the four DEC-069 installed.
#
# The rated default is 334 rounds = 668 games per pairing, and that number is
# measured rather than chosen. In a gauntlet only chesso plays everybody, so the
# graph is a star and chesso's rating under a given anchor is fixed by that one
# pairing alone -- adding a rung adds an independent estimate and does not
# narrow any existing one. S087 measured 334 games per pairing at +/-34, outside
# the +/-30 the procedure requires, and 668 at +/-24 to +/-28, inside it. So 668
# is the floor for a rated run and the old 167 could not have met the criterion
# at any set size.
if [[ "${1:-}" == "--bracket" ]]; then
  mode="bracket"
  rounds="${ROUNDS:-34}"     # 68 games per pairing, enough for a 10-90 % check
else
  mode="rated"
  rounds="${ROUNDS:-334}"    # 668 games per pairing, the S087-measured floor
fi

stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_rating_${mode}_${stamp}}"
mkdir -p "$outdir"
pgnfile="$outdir/games.pgn"
logfile="$outdir/fastchess.log"
report="$outdir/report.txt"

fail() { echo "RATING-RUN-FAILED: $*" >&2; exit 1; }

[[ -x "$candidate" ]] || fail "no candidate at $candidate, build it first"
[[ -r "$manifest" ]] || fail "no manifest at $manifest"
[[ -r "$book" ]] || fail "no book at $book"
command -v fastchess > /dev/null || fail "fastchess not on PATH"
command -v ordo > /dev/null || fail "ordo not on PATH"

# Ask each binary what it is, before playing a single game.
#
# The filename is not evidence. The first install of this set put the Rustic
# workspace development build at /usr/games/rustic -- it answers `id name engine
# 3.99.36`, has no published rating, and would have been anchored at the tag
# build's 1792. A gauntlet whose opponent is not the engine the anchor belongs
# to produces a number that looks fine and means nothing.
# `|| true` is not swallowing an error, it is the point. Not every engine exits
# on `quit` -- Blunder 8.5.5 does not -- so `timeout` returns 124 for a probe
# that in fact succeeded and printed the name. Under `set -euo pipefail` that
# 124 propagates out of the command substitution and kills the script with no
# message and a zero-byte log, which is exactly what it did before this line
# read the way it does now. The probe's real result is the string, and an empty
# string is checked for by the caller.
identify() {
  { printf 'uci\nquit\n' | timeout -k 1 5 "$1" 2> /dev/null \
    | sed -n 's/^id name //p' | head -1 | tr -d '\r'; } || true
}

names=()
bins=()
ccrl_names=()

while IFS=$'\t' read -r uci_name binary ccrl_name _source _tag _md5; do
  [[ -z "${uci_name:-}" || "${uci_name:0:1}" == "#" ]] && continue
  [[ -x "$binary" ]] || fail "manifest names $binary, which is not executable"
  got="$(identify "$binary")"
  [[ -n "$got" ]] || fail "$binary printed no 'id name' line within 5s of 'uci'"
  [[ "$got" == "$uci_name" ]] \
    || fail "$binary says 'id name $got', manifest says '$uci_name'"
  names+=("$uci_name")
  bins+=("$binary")
  ccrl_names+=("$ccrl_name")
done < "$manifest"

((${#names[@]} >= 3)) || fail "need at least 3 reference engines, manifest has ${#names[@]}"

# Snapshot the candidate before playing a single game. fastchess spawns the
# engine once per game, so pointing it at build/ lets a rebuild swap the engine
# mid-run and mix two versions into one result. fastchess.sh:71-84 has the
# incident this comes from.
snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-rated.XXXXXX")"
cp "$candidate" "$snapshot"
chmod +x "$snapshot"
trap 'status=$?; rm -f "$snapshot"; exit $status' EXIT

busy="$(ps -A -o %cpu= | awk '{ total += $1 } END { printf "%.0f", total }')"
if ((busy > 60)); then
  echo "WARNING: about ${busy}% of a core is already busy. A timed match on a"
  echo "         loaded machine measures the load as much as the engine."
  echo
fi

engine_args=(-engine "cmd=$snapshot" name=chesso)
for i in "${!names[@]}"; do
  engine_args+=(-engine "cmd=${bins[$i]}" "name=${names[$i]}")
done

{
  echo "mode        $mode"
  echo "candidate   $(git -C "$repo" rev-parse --short HEAD)$(git -C "$repo" diff --quiet || echo ' + uncommitted changes')"
  echo "opponents   ${names[*]}"
  echo "tc $tc  hash ${hash_mb}MB  concurrency $concurrency of $all_cores cores"
  echo "rounds $rounds  -> $((rounds * ${#names[@]} * 2)) games"
  echo "book        $(basename "$book")"
  echo "output      $outdir"
  echo
} | tee "$report"

# option.Threads is deliberately absent. None of the reference engines exposes
# it -- they are single-threaded by construction -- and chesso's is min 1 max 1.
# Sending an option an engine does not have is a way to lose a game to a
# protocol error rather than to strength. DEC-068.
fastchess \
  "${engine_args[@]}" \
  -tournament gauntlet -seeds 1 \
  -openings "file=$book" format=pgn order=random \
  -each "tc=$tc" "option.Hash=$hash_mb" \
  $adjudication \
  -rounds "$rounds" \
  -games 2 \
  -repeat \
  -concurrency "$concurrency" \
  -recover \
  -pgnout "file=$pgnfile" \
  -log "file=$logfile" \
  | tee -a "$report"

# Two different failures used to be one check, and DEC-075 split them.
#
# A CRASH, DISCONNECT, ILLEGAL MOVE OR STALL is a broken instrument. No rate of
# it is acceptable and it still voids the run at zero.
#
# A TIME FORFEIT is a real game result -- the loser ran out of clock and the
# points are genuinely the opponent's -- and is tolerated up to FORFEIT_MAX_PCT
# of an INDIVIDUAL ENGINE'S OWN GAMES. The denominator is not the run: 3
# forfeits in 3340 games is 0.09 % overall but 0.45 % of Stash's 668, so a
# whole-run threshold would tolerate sixteen of them landing on one engine while
# printing a comfortable number. DEC-075, DEC-076.
#
# The set of termination values present is still asserted rather than grepped
# for a string we guessed at, so a failure cannot hide behind a spelling nobody
# anticipated. `time forfeit` is now an expected value rather than a fatal one;
# anything outside the three still is not.
forfeit_max_pct="${FORFEIT_MAX_PCT:-1.0}"

{
  echo
  echo "=== terminations ==="
  grep -h '^\[Termination' "$pgnfile" | sort | uniq -c | sort -rn
} | tee -a "$report"

unexpected="$(grep -h '^\[Termination' "$pgnfile" \
  | grep -cvE '"(normal|adjudication|time forfeit)"' || true)"
broken="$(grep -ciE 'disconnect|stall|illegal move|crash' "$logfile" || true)"

# Not piped into tee: the exit status is the verdict and a pipeline would
# report tee's instead.
forfeit_status=0
"$repo/tools/forfeit_report.py" "$pgnfile" --max-pct "$forfeit_max_pct" \
  > "$outdir/forfeits.txt" 2>&1 || forfeit_status=$?

{
  echo "terminations outside normal/adjudication/time forfeit  $unexpected"
  echo "log lines matching disconnect, stall, illegal move or crash  $broken"
  echo
  echo "=== time forfeits per engine, against ${forfeit_max_pct} % of its own games ==="
  cat "$outdir/forfeits.txt"
} | tee -a "$report"

if ((unexpected > 0 || broken > 0 || forfeit_status != 0)); then
  {
    echo "RATING-RUN-INVALID:"
    ((unexpected > 0)) && echo "  $unexpected termination(s) outside normal, adjudication and time forfeit"
    ((broken > 0)) && echo "  $broken log line(s) matching disconnect, stall, illegal move or crash"
    ((forfeit_status == 1)) && echo "  an engine's time-forfeit rate is over ${forfeit_max_pct} % of its own games"
    ((forfeit_status == 2)) && echo "  the PGN could not be read or names no games"
    echo "A crash or a disconnect voids at zero. A forfeit rate is raised with"
    echo "FORFEIT_MAX_PCT only by a recorded decision -- DEC-075 set 1 %."
  } | tee -a "$report"
  echo "RATING-RUN-DONE $mode INVALID $outdir"
  exit 1
fi

if [[ "$mode" == "bracket" ]]; then
  {
    echo
    echo "=== bracketing: chesso's score against each reference ==="
    echo "Below 90 % against the strongest and above 10 % against the weakest"
    echo "means the set brackets chesso and a rated run is worth booking."
  } | tee -a "$report"
  ordo -q -p "$pgnfile" -o "$outdir/ordo_bracket.txt" -W -D -j "$outdir/h2h.txt"
  tee -a "$report" < "$outdir/h2h.txt"
  echo "RATING-RUN-DONE bracket OK $outdir"
  exit 0
fi

# Rated: anchors read from the list at run time, never hardcoded.
cache="$outdir/ccrl_list.html"
anchors="$outdir/anchors.tsv"
"$repo/tools/ccrl_rating.py" --cache "$cache" "${ccrl_names[@]}" > "$anchors" \
  || fail "an anchor is not on the CCRL list, see above"

{
  echo
  echo "=== anchors, read from the CCRL Blitz list today ==="
  cat "$anchors"
  echo
  echo "=== chesso's solved rating, once per anchor choice ==="
} | tee -a "$report"

for i in "${!names[@]}"; do
  ccrl="${ccrl_names[$i]}"
  rating="$(awk -F'\t' -v n="$ccrl" '$1 == n { print $2 }' "$anchors")"
  [[ -n "$rating" ]] || fail "no anchor rating for $ccrl"
  solved="$outdir/ordo_anchored_$i.txt"
  ordo -q -p "$pgnfile" -o "$solved" -a "$rating" -A "${names[$i]}" \
    -s 1000 -F 95 -n "$concurrency" -W -D
  line="$(grep -E '^\s*[0-9]+\s+chesso' "$solved" || true)"
  echo "anchored on ${names[$i]} at $rating (CCRL $ccrl):  $line" | tee -a "$report"
done

{
  echo
  echo "ordo intervals are trinomial. The SPRT verdicts in this repository run"
  echo "model=normalized and report nElo. The two are never quoted against each"
  echo "other."
} | tee -a "$report"

echo "RATING-RUN-DONE rated OK $outdir"
