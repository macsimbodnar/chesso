#!/usr/bin/env bash
#
# S130: quiescence takes the table score as its stand pat where the stored
# bound allows it. Candidate is the working tree at `293a45b` plus the
# substitution in `src/search.cpp` and its tests in `tests/test_search.cpp`.
# Reference is `293a45b`, S093's commit.
#
# RUN 2. **Run 1 was killed at 103 games on 2026-08-22, deliberately**, about
# nine minutes in; its log is `.tuning/sprt_s130.log` and its games are in
# /tmp/chesso_sprt_s130_20260822_074255. No verdict was reached and none is
# claimed. It was stopped because the change it was measuring was unsound: the
# read side consumed a table bound and the final store then wrote the result
# back as **exact**, laundering `value >= s` into `value == s` over the entry
# that certified it. AGENTS.md 0 -- a known defect contaminates every
# measurement taken after it. The defect, its three red tests and the fix are
# DEC-102 and the step file. **This run measures the substitution and the
# bound-type correction together**, because neither is sound without the other
# and the correction is a no-op without the substitution.
#
# WHAT IT CHANGES. Quiescence already probes the table (S094) and already reads
# the entry's static `eval` field in place of recomputing one (S094, layer b).
# Where the entry did not answer the node outright, its *score* is still a
# bound on this position, and this step lets that bound move the stand pat in
# the one direction the entry's type certifies:
#
#   TT_BETA_NODE  (lower bound, value >= s)  may raise it, never lower it
#   TT_ALPHA_NODE (upper bound, value <= s)  may lower it, never raise it
#   TT_PV_NODE    (exact, value == s)        replaces it
#
# And the node then stores only what it can still claim. The stand pat carries
# its bound kind beside its number: exact when it is the static score, and
# otherwise the type of the entry it came from. The final store degrades
# accordingly -- a value that is still the raised stand pat is a lower bound, a
# value that is still the lowered stand pat is an upper bound, and a value a
# searched move produced is exact again, except where the floor was *lowered*,
# since the static score it displaced may beat that move too. With no entry
# every branch reduces to the pre-S130 code exactly. DEC-102.
#
# The stored *score* and the entry's `eval` field are unchanged: `stored_eval` is fixed
# from the static number before the substitution, so the entry's `eval` field
# keeps its S094 semantics -- the position's static score and never an improved
# stand pat, which exists only where a bound held against *this* node's window
# and the entry outlives the window. A mate-band score is never substituted at
# all, for the same reason a bound is never stored: the stand pat is handed to
# the parent and stored by three separate paths below, and a mate distance
# entering there is a mate no search found. The S145 48-position mate suite
# reads identically at `293a45b`, at run 1's candidate and at this one --
# `mate in 4: 0 of 8, mate in 5: 0 of 8`, 866668 assertions all three times.
#
# THE PUBLISHED PRIOR IS POSITIVE AND MULTIPLY SOURCED, AND THE NEAREST-SHAPED
# RECORD IS ZERO. Both are stated here before a game is played. Weiss cca90ea7
# (2020-09-08, "Use score from TT instead of static eval if possible", #336)
# measured +10.78 +/- 6.91 at 10+0.1, +12.09 +/- 6.83 at 60+0.6 and +21.44 +/-
# 9.52 at 20+0.2 on 8 threads -- but its commit message scopes the change to
# "pruning heuristics" and never to quiescence, so part of that gain plausibly
# belongs to the main-search sites, which here are S108/S109 and not this step.
# Ethereal 25e56feb (2019-01-13) measured +11.04/+3.12/+2.55 for probe-and-use
# landed as one patch, which prices S094's ground together with this. Lynx PR
# #1319 (2025-01) is the exact increment this step is -- score over eval field,
# quiescence only, on top of an existing probe and an existing eval read -- and
# it measured -1.70 +/- 3.02 over 22236 games, then -0.01 +/- 1.85 over 60152
# STC and +1.07 +/- 2.45 at LTC, and merged at about zero. DEC-019: the figures
# decide what to try, never what to conclude.
#
# THE MECHANISM IS SMALL AND IT WAS COUNTED, NOT ARGUED. Over the three
# `tools/search_bench.py` positions at depth 12, one engine process and one
# table for all three, quiescence reached the stand-pat site 1818978 times.
# 20432 of those (1.12 %) had an entry at all -- S094 measured 0.79 % on
# kiwipete alone and the probe has warmed since. The substitution fired on
# 4161 of them as a raise (0.228 % of sites) and 151 as a cap (0.008 %). The
# exact arm fired 0 times, as predicted: `tt_entry_answers()` returns an exact
# entry whatever the window is, so one never reaches the stand pat. 0 entries
# were rejected for carrying a mate score on this workload, which is why the
# band exclusion has a unit test and not a counter.
#
# THE STORE SIDE IS NOT SMALL IN RELATIVE TERMS, and it is the part run 1 was
# getting wrong. Over the same workload the final store writes PV 175, ALPHA
# 318250, BETA 386 -- and all 386 of those BETA stores are entries run 1 would
# have written **exact**. So 386 of 561 exact stores at that site, **69 %**,
# were laundered bounds.
#
# NODE COUNTS MOVE, BARELY. `tools/search_bench.py` at depth 12 reads
# 639228 / 3430710 / 367858 against `293a45b`'s 639205 / 3425236 / 367858 --
# +23, +5474 (+0.16 %), 0. Best move unchanged at all three. kiwipete moves
# further than run 1's -37 did and in the expected direction: fewer exact
# entries means fewer nodes answered outright, so slightly more search.
# INV-6 is not available: the change alters play by
# construction, so a node identity would be a bug and not a discharge. The
# numbers are here as the mechanism beside the verdict, not as evidence of one.
#
# BOUNDS.
#
#   elo0=0 elo1=5 alpha=0.05 beta=0.05
#
# fastchess.sh's default for a change claimed to gain, and the coordinator's
# decision rather than this script's. Same reasoning as S093 verdict 1: the
# published prior is positive and multiply sourced, and this pair is the one
# whose H1 produces a keepable claim.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
# All three readings below are about **the substitution together with the
# bound-type correction**, which is what this candidate is. They cannot be
# attributed separately and no attempt will be made to.
#
#   H1 accepted -> the substitution gains 5 Elo or more. Keep it, and record
#                  the point estimate as biased upward by the early stop
#                  (DEC-063): what the run establishes is the pre-registered
#                  claim and not the magnitude.
#   H0 accepted -> it does not. Recorded as zero with the number, in the step
#                  file and in `specs.md`, and DEC-019 gains an entry -- the
#                  Weiss prior of +10.8 does not survive it. The change may
#                  still be kept with the reason stated, as S005, S006 and
#                  S015 were; that is a separate decision and it is the
#                  owner's.
#   No verdict   -> recorded as no verdict, which is a real result. **The
#                  threshold is named now rather than after the fact: if the
#                  LLR is still inside the bounds at about 8000 games, that is
#                  itself evidence the effect is small.** S093 verdict 2 ran an
#                  H0 to the wall against this exact pair in 6 h 35 m over
#                  15398 games -- a null resolves slowly here by construction,
#                  and `elo0=0 elo1=5` cannot separate a small gain from zero,
#                  which is DEC-063's whole hazard. Re-running at two-sided
#                  bounds is a **recorded decision and not an automatic
#                  action** (DEC-063, S021's precedent). The fire rate above is
#                  the standing candidate explanation: 0.23 % of stand-pat
#                  sites is a thin mechanism to buy 5 Elo with.
#
# THE CORNER RUN 1 CALLED WATCHED AND OUT OF SCOPE IS FIXED, NOT WATCHED. It
# was the reason run 1 was killed; see DEC-102 and the run-2 note at the top.
#
# Everything except -sprt mirrors fastchess.sh's live engine block: tc 8+0.08,
# Hash 16, Threads 1, the UHO book, -repeat, -recover, the adjudication pair,
# and -check-mate-pvs (S145). AGENTS.md 12 and DEC-061: a terminal marker on
# every exit path, success and failure both.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

marked=0
fail()
{
  marked=1
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

book="books/UHO_Lichess_4852_v1.epd"
book_format="epd"
candidate="build/src/chesso"
tc="8+0.08"
sprt="elo0=0 elo1=5 alpha=0.05 beta=0.05"
rounds=20000
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"
mate_pv_check="-check-mate-pvs"
ref_sha="293a45b"

all_cores="$(sysctl -n hw.physicalcpu 2> /dev/null || nproc)"
concurrency="${CONCURRENCY:-$all_cores}"

stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_sprt_s130_v2_${stamp}}"
mkdir -p "$outdir" || fail "cannot create $outdir"
logfile="$outdir/fastchess.log"
pgnfile="$outdir/games.pgn"

# fastchess appends, so a second run into the same directory would mix two
# matches into one census. Refuse instead.
[[ ! -e "$pgnfile" ]] \
  || fail "$pgnfile already exists; fastchess appends and this run's census would mix two matches"

[[ -x "$candidate" ]] || fail "no candidate at $candidate, build it first"
[[ -r "$book" ]] || fail "no book at $book, fetch it with books/fetch_book.sh"

# Snapshot the candidate before the first game. fastchess spawns the binary
# once per game, so pointing it at build/ lets a rebuild swap the engine under
# the match -- which has already happened once and reported +301 Elo that meant
# nothing. DEC-020.
snapshot="$(mktemp "${TMPDIR:-/tmp}/chesso-candidate-s130v2.XXXXXX")" \
  || fail "mktemp for the candidate snapshot"
cp "$candidate" "$snapshot" || fail "cannot snapshot $candidate"
chmod +x "$snapshot"

# The trap preserves the status it was entered with: left to itself it would
# end in a successful `rm -f` and report an abort as a clean exit.
trap 'status=$?; rm -f "$snapshot"
      if ((status != 0 && marked == 0)); then
        echo "SPRT-RUN-FAILED: exited $status" >&2
      fi
      exit $status' EXIT

# The reference, from the ref and in its own worktree, exactly as fastchess.sh
# builds it. The block is here so the run is reproducible from the script alone.
ref_dir="$(git rev-parse --show-toplevel)/.ref-builds/$ref_sha"
reference="$ref_dir/build/src/chesso"

if [[ ! -x "$reference" ]]; then
  echo "Building reference $ref_sha ..."
  git worktree add --detach "$ref_dir" "$ref_sha" > /dev/null \
    || fail "git worktree add for $ref_sha"
  cmake -S "$ref_dir" -B "$ref_dir/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > /dev/null \
    || fail "cmake configure for $ref_sha"
  cmake --build "$ref_dir/build" --target chesso \
    -j"$(sysctl -n hw.logicalcpu 2> /dev/null || nproc)" > /dev/null \
    || fail "cmake build for $ref_sha"
fi

busy="$(ps -A -o %cpu= | awk '{ total += $1 } END { printf "%.0f", total }')"
if ((busy > 60)); then
  echo "WARNING: about ${busy}% of a core is already busy. A timed match on a"
  echo "         loaded machine measures the load as much as the engine."
  echo
fi

echo "step       S130 run 2, quiescence stand pat from the table score"
echo "candidate  $(git rev-parse --short HEAD)$(git diff --quiet || echo ' + uncommitted changes')"
echo "reference  $ref_sha"
echo "tc $tc  hash 16  concurrency $concurrency of $all_cores cores"
echo "book       $(basename "$book")"
echo "bounds     $sprt"
echo "out        $outdir"
echo

start=$(date +%s)

fastchess \
  -engine cmd="$snapshot" name=candidate-s130-qs-tt-stand-pat \
  -engine cmd="$reference" name="ref-$ref_sha" \
  -openings file="$book" format="$book_format" order=random \
  -each tc="$tc" option.Hash=16 option.Threads=1 \
  -sprt $sprt model=normalized \
  $adjudication \
  $mate_pv_check \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -recover \
  -pgnout file="$pgnfile" \
  -log file="$logfile"
status=$?

end=$(date +%s)

# The census is read from the PGN and not from the log: -log is WARN-and-above
# and comes back empty, and an empty log is indistinguishable from one that was
# never written (S089).
count()
{
  grep -hc "$1" "$pgnfile" 2> /dev/null || true
}

games="$(count '^\[Result ')"
draws="$(count '^\[Result "1/2-1/2"\]')"
forfeits="$(count '^\[Termination "time forfeit"\]')"
games="${games:-0}"
draws="${draws:-0}"
forfeits="${forfeits:-0}"

echo
echo "elapsed_seconds=$(( end - start ))"
echo "=== $games games in $pgnfile ==="
grep -h '^\[Termination' "$pgnfile" 2> /dev/null | sort | uniq -c | sort -rn || true
if ((games > 0)); then
  awk -v g="$games" -v d="$draws" -v f="$forfeits" 'BEGIN {
    printf "draws     %d of %d, %.1f %%\n", d, g, 100 * d / g
    printf "decisive  %d of %d, %.1f %%\n", g - d, g, 100 * (g - d) / g
    printf "forfeits  %d of %d, %.2f %%\n", f, g, 100 * f / g
  }'
fi

if ((status != 0)); then fail "fastchess exited $status"; fi

echo "SPRT-RUN-DONE s130v2 $outdir"
