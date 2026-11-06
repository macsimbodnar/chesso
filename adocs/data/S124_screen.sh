#!/usr/bin/env bash
# S124's screen: the endgame family's identity rule, P0's conversion test, then the scale
# factor's three parts in fixed order, each with the earlier survivors live, by held-out
# loss on S122's regenerated corpus's own held-out games (DEC-259 (a), DEC-350 (7), S122's
# form): F1, F2, F3. Every part read at K refit at its fit, iterated, with K at the
# identity beside it (DEC-352), and refused by the tablebase guard whatever its d.
#
#   adocs/data/S124_screen.sh locate      # S122's prepared corpus, its sha256s
#   adocs/data/S124_screen.sh tools       # build/'s tuner, feature_audit, margin_census,
#                                         # chesso and S124_k_refit frozen at HEAD
#   adocs/data/S124_screen.sh identity    # the identity rule, the landing, the support rule
#   adocs/data/S124_screen.sh k           # the quarter samples, K at the identity on each
#   adocs/data/S124_screen.sh guard       # the tablebase guard's rows and the landing on them
#   adocs/data/S124_screen.sh p0          # head's conversion test, P0
#   adocs/data/S124_screen.sh part F1     # a part's fits, its K iteration, its guard, its
#                                         # reading; F1, then F2, then F3; resumable
#   adocs/data/S124_screen.sh read F1     # the rule over a part's fits
#   adocs/data/S124_screen.sh all         # locate to F3, in order
#   adocs/data/S124_screen.sh smoke       # every stage end to end on a slice; no reading
#   adocs/data/S124_screen.sh guard-probe # after F3: an inert part passes the guard and a
#                                         # planted one is refused (DEC-357); the smoke runs it
#
# Run from the main checkout with phase 1 landed (the family at its identity) and build/
# built at a clean HEAD: every binary is frozen with the HEAD it was built from and every
# fit's header must name it. Each stage prints S124-<STAGE>-DONE or S124-<STAGE>-FAILED as
# its last stdout line, the reasons above a FAILED, so each runs detached under a four-exit
# watcher (WATCHERS). OUT defaults to .tuning/s124/screen (DEC-235); the corpus is read in
# place and never written.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-11-06 in S124's phase 2a, before any fit, from the
# step file's draft ("Pre-registered: the screen, a draft") with every corpus figure
# re-read on S122's regenerated corpus (adocs/data/S124_phase2_preconditions.txt) and
# each change from the draft a dated amendment in the step file (its section "2026-11-06:
# phase 2a"). Nothing below the line changes once `part F1` has started; a change is a
# dated section in the step file and a decision. DEC-350 and DEC-352 are the rulings it
# carries.
# ============================================================================
#
# THE CORPUS (DEC-349, DEC-350 (7), (9)): S122's regenerated prepared corpus,
# .tuning/s122/prepared/corpus.tsv, sha256 04c596de726b5a28...76509, 6307294 rows from
# 137991 training games, prepared from the pool .tuning/s122/corpus/ (its record's `pool`
# line, S333), the labels the games' results under the laws (S331's datagen). The held-out
# games are its own fixed set, 12000 games, its qs view the tuner's --holdout
# (pool.tsv.qs.validation, sha256 faef5cd4...936eb, 1384182 rows). `locate` holds both to
# the preparation's record.
#
# THE IDENTITY RULE, before any fit (restated 2026-11-06 for a local rank): `identity` runs
# tools/feature_audit --free endgame_scale over the corpus, its design linearised at the
# shipped point (S122's curve at each side's own x), and passes only on
# S124_screen.py identity-check: the trace's identity on every row; the report in its
# local form; the design check and the linearisation check under 1e-6; the zero columns
# the 32 dead pawn squares and nothing else; 16 dependencies by construction, S134's 7,
# S121's 8 and the mop-up corner pair, each holding, no curve duplicate and no exact
# dependency beyond them; rank exactly 1142 of 1190, nullity 48; the span 48 of 48; every
# cell's support line at rows > 0. Read 2026-11-06 on this corpus: rank exactly 1142,
# nullity 48, design check 9.09e-13, linearisation check 1.56e-10. And the landing at its
# identity: a zero-epoch header's 13 factors at 128 and mop-up's six at 0.
#
# THE SUPPORT RULE (DEC-326 (5)), from the same report's support lines, games whose rows'
# E1 at the shipped weights picks the cell: OCB_PURE P<=2 under 1000 games stops before F1
# (no supported neighbour in F1); OCB_PURE P>=5 under 1000 is tied to P=3-4 at scale 1,
# `endgame_scale 11 10`; any other cell under 1000 stops (amendment 9, 2026-11-06, no tie
# being pre-registered for it). Read 2026-11-06: the thirteen
# cells on 1131 to 37360 games, OCB_PURE P<=2 on 2438, P=3-4 on 2616, P>=5 on 1131: no stop
# and no tie, so F3 fits P>=5 free (the draft's tie was the void corpus's 840 and S083's
# 798).
#
# P0, A READING: HEAD'S CONVERSION TEST (DEC-350 (4); adocs/data/S124_p0.py). Starts:
# training-side root positions of mop-up's domain from the raw root pool
# (.tuning/s122/corpus/pool.tsv.root), one a game, its first root row in the class; the
# class by piece counts; the clock zeroed; kept where Syzygy reads WDL 2 for the side with
# men; one start a position; in one seeded shuffle a class, seed 20261106, the first 200
# of KRvK, KQvK, KRBvK, KRNvK and KBNvK and all of KBBvK's: 1126 starts (read 2026-11-06:
# 126 KBBvK). Self-play by the frozen chesso, a process a side, Hash 16, Threads 1, 100000
# nodes a move, under the laws, no adjudication; converted = mated within 50 moves. If
# every start converts, mop-up is recorded zero and P1 is not run; otherwise P1 waits on
# its instrument, the coordinator's (the step file's options note). P0 decides no part.
#
# P1, NOT HERE: mop-up's SPSA over its pairs, the corner as one parameter, on P0's
# objective, then the conversion test on fresh starts (`S124_p0.py select --skip 200`,
# KBBvK's all taken, so none fresh there), kept iff McNemar's one-sided z > 1.645 over head
# and no class converts fewer (`S124_p0.py mcnemar`). Its instrument is not built.
#
# THE PARTS (DEC-350 (7)), in order, earlier survivors live, mop-up and every older
# constant frozen:
#   F1  endgame_scale.f1: P=0 d<=0, P=0 d=1, OCB_PURE P<=2
#   F2  endgame_scale.f2: P=0 d>=2, P=1 d<=0, d=1, d>=2, P=2 d<=0, d=1, d>=2
#   F3  endgame_scale.f3: OCB_PURE P=3-4, P>=5, OCB_ROOK; the tie above if it binds
# Each part frees its own cells and every earlier survivor's, and is paired against its
# predecessor, the landed tree plus the survivors so far (DEC-326 (3), S122's form): for F1,
# and any part with no survivor before it, the landed tree. A dropped part's cells stay at
# F = 128.
#
# THE FIT (DEC-345 (1)): build/'s tuner, `--data DATA --holdout VIEW_QS --only GROUPS [--tie
# TIES] --k K --vector FIT.vector --epochs 20000 --report 100 --patience 20 --lambda 0
# --seed 1 --threads 12`, Adam at lr 1, the box [0, F] after every step (DEC-350 (5)). Six
# fit sets a part: the full corpus and five quarter samples, `corpus_split.py sample --rows
# C/4 --per-game 0 --seed s`, s = 1 to 5, the three views named, drawn once.
#
# K, DEC-352: every part read at K refit at its fit, iterated, with K at the identity
# beside it. On each sample: K_0, K at the identity, the tuner's zero-epoch `fitted K` at
# the landed constants; round r fits the part at K_(r-1) from the landed constants, and
# K_r is K refit at that fit's emitted vector by adocs/data/S124_k_refit.cpp (the tuner's
# fit_k(), its search, rows and threads, at the vector); the round is decided when K_r is
# within 1e-4 of K_(r-1), at most 6 rounds, else K-STOP for the coordinator. `k` holds
# S124_k_refit at the identity to the tuner's own K on each sample, within 1e-4, before any
# fit: the instrument agrees with the tuner where both can read. Read 2026-11-06: K at the
# identity 1.2171 on the full corpus (S124_k_refit; the record's 1.2302 is 7fe9f89's, before
# S122's fit landed).
#
# THE READING, S124_screen.py read, mechanically (DEC-326 (3), S122's form): on each sample
# x, d_x = L(pred, x) - L(part, x), L the fixed held-out set's loss of a fit at its own
# decided K: the landed tree's at K_0 (the round-1 fit's start:), an earlier survivor's
# its decided fit's best:, the part's its decided fit's best:. s the standard deviation
# of the five quarter d; m = max(1.645 s, 2e-6); kept iff the full d > m and the guard
# passes. Unreadable: a fit missing, a log whose start: and best: disagree, a header at
# another cap or built at another HEAD, s above 1e-3, no guard reading. Printed beside,
# deciding nothing: round 1's d, each side at K_0; every fit's rounds, K and stop.
#
# THE TABLEBASE GUARD (DEC-350 (7)), every part: the training corpus's rows of five men or
# fewer, labelled by Syzygy at each row's own clock under the fifty-move rule (WON 1, DRAWN
# and SPOILED 0.5, LOST 0, White relative; EDGE rows, at exactly 100, left out;
# adocs/data/S124_guard.py), read by tools/margin_census at the part's decided full fit's
# vector and its predecessor's, BOTH AT ONE K, THE PART'S DECIDED K (amended 2026-11-06,
# DEC-357: each at its own K measured K, an inert part refused), paired by game: refused
# when the part raises that loss by more than 1.645 errors by game, whatever its held-out
# d. The same comparison with both at K_0 is printed beside and decides nothing. Read
# 2026-11-06: 292936 rows of five men or fewer in 29986 games, 292418 labelled (518 EDGE,
# 0 unprobed): WON 46052, LOST 35108, DRAWN 197583, SPOILED 13675. In F1's cells P=0 d<=0
# 38157, P=0 d=1 72749, OCB_PURE P<=2 10959 (its P=0 and P=1 rows alone); in F2's P=0 d>=2
# 28593, P=1 d<=0 81983, d=1 9960, d>=2 3511, P=2 d<=0 3996, P=2 d=1 and d>=2 none; in F3's
# none, so F3 alone moves no guard row and is screened on the loss, which its stamp says.
#
# THE CANDIDATE: the last survivor's decided full fit, every survivor free, at its K. No
# survivor: no candidate, each part's zero recorded, the factor at F.
#
# THE COST: S124_phase1_cost.sh's sink at the candidate's masks; it decides nothing and the
# verdict carries it.
#
# THE SPRT: one {0, 5} nElo of the survivors (and mop-up if P1 keeps it), 8+0.08, Hash 16,
# books/noob_3moves.epd, concurrency 12, against the landed tree; 41861 games and 19.84 h at
# the midpoint, 25591 and 12.13 h on a bound; cap 20950 rounds, 41900 games, 19.86 h;
# S101's abort rule; an A/A of 1000 games first if the harness moved. The two-sided resign
# at 400 cp hides most of mop-up's conversions from the verdict, and this says so. On H0 or
# no verdict, in order, each {0, 5} against what has landed, survivors only: A = mop-up,
# B = F1, C = F2 and F3's survivors.
#
# MACHINE HOURS, inferred, not measured: identity 2 min; k about 10 min; guard about 5 min;
# p0 about 5 min (12 starts in 3 s in the smoke; a KBNvK start that fails plays all 100
# plies); a part's round of six fits about 40 min at patience (S122's 0.87 s a part-epoch,
# about 2500 epochs), 3 to 4 rounds expected: about 2 to 3 h a part, 6 to 9 h for three;
# at worst, 6 rounds every fit on the 20000 cap, about 30 h a part. Every part is over
# four hours with its rounds only at the worst: DEC-155's night, the coordinator's
# schedule; a watcher's ceiling 2 x the stage's figure.
#
# DISK: five quarter samples, about 120 MB each; the guard rows 17 MB. A stage refuses to
# start under 4 GB free.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
main="$(dirname "$(git -C "$root" rev-parse --path-format=absolute --git-common-dir)")"
OUT="${OUT:-$root/.tuning/s124/screen}"
PREPARED="${PREPARED:-$main/.tuning/s122/prepared/prepared.txt}"
POOL="${POOL:-$main/.tuning/s122/corpus}"
CORPUS_SHA="${CORPUS_SHA:-04c596de726b5a2886df4e7e41c08fdd958529c669ba98ee69befd6a54676509}"
PY="${PY:-$HOME/.venv/chess/bin/python}"
THREADS=12
SEEDS="1 2 3 4 5"
SAMPLES="full q1 q2 q3 q4 q5"
FREE_GB_FLOOR=4
ROUNDS=6
# Overridable for the smoke and nothing else.
FIT_EPOCHS="${FIT_EPOCHS:-20000}"
P0_PER_CLASS="${P0_PER_CLASS:-200}"
P0_SMOKE="${P0_SMOKE:-}"
SMOKE="${SMOKE:-0}"
export FIT_EPOCHS

# Set to OUT/bin's frozen copies by `tools`, which every stage that runs one calls first.
helper="" split="" guard_py="" p0_py=""
say() { echo "$*"; }

fit_args() {
  echo "--epochs $FIT_EPOCHS --report 100 --patience 20 --lambda 0 --seed 1 --threads $THREADS"
}

enough_disk() {
  local free_gb
  mkdir -p "$OUT"
  free_gb=$(df --output=avail -BG "$OUT" | tail -n 1 | tr -dc '0-9')
  (( free_gb >= FREE_GB_FLOOR )) ||
    { say "refused: ${free_gb} GB free under $OUT, under ${FREE_GB_FLOOR} GB"; return 1; }
}

# Every file the screen builds or runs, held clean at HEAD (the fast check's finding 4):
# the engine and the tools, the tests' headers S124_cell_check.cpp includes, the two
# programs compiled here and the Python run from the frozen copies under OUT/bin.
SCREEN_FILES=(src tools tests adocs/data/S124_screen.sh adocs/data/S124_screen.py
              adocs/data/S135_screen.py adocs/data/S124_guard.py adocs/data/S124_p0.py
              adocs/data/S124_k_refit.cpp adocs/data/S124_cell_check.cpp)

engine_stamp() {
  # The smoke runs on a working tree too, its binaries' own `-dirty` stamp; nothing it
  # writes is a reading.
  if [[ "$SMOKE" == 1 ]] && ! git -C "$root" diff --quiet HEAD -- "${SCREEN_FILES[@]}"; then
    echo "$(git -C "$root" rev-parse --short HEAD)-dirty"
    return 0
  fi
  [[ -z "$(git -C "$root" ls-files --others --exclude-standard -- "${SCREEN_FILES[@]}")" ]] ||
    { say "refused: untracked files among the screen's sources: ${SCREEN_FILES[*]}" >&2; return 1; }
  git -C "$root" diff --quiet HEAD -- "${SCREEN_FILES[@]}" ||
    { say "refused: a file the screen builds or runs differs from HEAD (${SCREEN_FILES[*]})" >&2; return 1; }
  git -C "$root" rev-parse --short HEAD
}

check_copy() {
  [[ "$(sha256sum "$1" | cut -d' ' -f1)" == "$(cat "$1.sha256" 2>/dev/null)" ]] ||
    { say "refused: $1 is not the copy frozen"; return 1; }
}

freeze() {
  local src="$1" copy="$2"
  if [[ ! -f "$copy" ]]; then
    [[ -x "$src" ]] || { say "no binary at $src"; return 1; }
    cp "$src" "$copy.tmp" && chmod +x "$copy.tmp" &&
      sha256sum "$copy.tmp" | cut -d' ' -f1 > "$copy.sha256" && mv "$copy.tmp" "$copy" || return 1
  fi
  check_copy "$copy"
}

# ------------------------------------------------------------------- locate

record_field() { sed -n "s/^$1 //p" "$PREPARED" | head -n 1; }
view_path() { awk -v v="$1" '$1 == "view" && $2 == v { print $3 }' "$PREPARED"; }
view_sha() { awk -v v="$1" '$1 == "view" && $2 == v { print $4 }' "$PREPARED"; }

check_views() {
  local v path
  [[ -f "$PREPARED" ]] || { say "no $PREPARED"; return 1; }
  grep -q '^S122-PREPARED$' "$PREPARED" || { say "refused: $PREPARED has no S122-PREPARED stamp"; return 1; }
  [[ "$(record_field pool)" == "$POOL" ]] ||
    { say "refused: $PREPARED names the pool '$(record_field pool)', not $POOL (S333)"; return 1; }
  for v in qs root pv; do
    path=$(view_path "$v")
    [[ -n "$path" && "$(sha256sum "$path" | cut -d' ' -f1)" == "$(view_sha "$v")" ]] ||
      { say "the held-out set's $v view is not the file the preparation recorded"; return 1; }
  done
  HOLDOUT=$(view_path qs)
  VIEWS=("$(view_path qs)" "$(view_path root)" "$(view_path pv)")
}

corpus_of() {
  local record="$OUT/corpus.txt" path
  [[ -f "$record" ]] || { say "no $record; run locate first"; return 1; }
  path=$(sed -n 's/^corpus //p' "$record")
  [[ "$(sed -n 's/^sha256 //p' "$record")" == "$CORPUS_SHA" &&
     "$(sha256sum "$path" | cut -d' ' -f1)" == "$CORPUS_SHA" ]] ||
    { say "refused: $path is not the corpus $CORPUS_SHA"; return 1; }
  CORPUS="$path"
  ROWS=$(sed -n 's/^rows //p' "$record")
}

locate() {
  enough_disk || return 1
  check_views || return 1
  local path
  path=$(record_field corpus)
  [[ "$(record_field sha256)" == "$CORPUS_SHA" && "$(sha256sum "$path" | cut -d' ' -f1)" == "$CORPUS_SHA" ]] ||
    { say "refused: $PREPARED's corpus is not $CORPUS_SHA, the pre-registration's"; return 1; }
  if [[ ! -f "$OUT/corpus.txt" ]]; then
    printf 'corpus %s\nsha256 %s\nrows %s\nholdout %s\nprepared %s\n' "$path" "$CORPUS_SHA" \
      "$(record_field rows)" "$(view_path qs)" "$(sha256sum "$PREPARED" | cut -d' ' -f1)" > "$OUT/corpus.txt"
  fi
  cat "$OUT/corpus.txt"
  corpus_of
}

# -------------------------------------------------------------------- tools

tools() {
  local head bin="$OUT/bin"
  head=$(engine_stamp) || return 1
  mkdir -p "$bin" || return 1
  if [[ -f "$bin/stamp" ]]; then
    [[ "$(cat "$bin/stamp")" == "$head" ]] ||
      { say "refused: $bin was frozen at $(cat "$bin/stamp"), not HEAD $head; take a fresh OUT"; return 1; }
  else
    [[ -f "$root/build/src/libchesso_engine_trace.a" ]] || { say "no build/; build it at HEAD"; return 1; }
    g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I "$root/src" -I "$root/tools" \
      "$root/adocs/data/S124_k_refit.cpp" "$root/build/src/libchesso_engine_trace.a" -lpthread \
      -o "$bin/S124_k_refit.built" || { say "S124_k_refit did not build"; return 1; }
    g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I "$root/src" -I "$root/tools" \
      -I "$root/tests" "$root/adocs/data/S124_cell_check.cpp" \
      "$root/build/src/libchesso_engine_trace.a" -lpthread -o "$bin/S124_cell_check.built" ||
      { say "S124_cell_check did not build"; return 1; }
    local b
    for b in tuner feature_audit margin_census; do
      freeze "$root/build/tools/$b" "$bin/$b" || return 1
    done
    freeze "$root/build/src/chesso" "$bin/chesso" || return 1
    freeze "$bin/S124_k_refit.built" "$bin/S124_k_refit" || return 1
    freeze "$bin/S124_cell_check.built" "$bin/S124_cell_check" || return 1
    # The helpers run from here, never from the tree (the fast check's finding 4).
    for b in S124_screen.py S135_screen.py S124_guard.py S124_p0.py; do
      freeze "$root/adocs/data/$b" "$bin/$b" || return 1
    done
    freeze "$root/tools/corpus_split.py" "$bin/corpus_split.py" || return 1
    local bench
    bench=$("$bin/chesso" bench 2>&1) ||
      { say "chesso bench failed; nothing frozen is stamped"; return 1; }
    [[ "$(tail -n 1 <<< "$bench")" =~ ^[0-9]+\ nodes\  ]] ||
      { say "chesso bench printed no node count: $(tail -n 1 <<< "$bench")"; return 1; }
    tail -n 1 <<< "$bench" > "$bin/bench.txt"
    echo "$head" > "$bin/stamp"
  fi
  local b
  for b in tuner feature_audit margin_census chesso S124_k_refit S124_cell_check \
           S124_screen.py S135_screen.py S124_guard.py S124_p0.py corpus_split.py; do
    check_copy "$bin/$b" || return 1
  done
  helper="$bin/S124_screen.py"; split="$bin/corpus_split.py"
  guard_py="$bin/S124_guard.py"; p0_py="$bin/S124_p0.py"
  STAMP=$(cat "$bin/stamp")
  say "tools at $STAMP, bench $(cat "$bin/bench.txt")"
}

# ----------------------------------------------------------------- identity

identity() {
  local dir="$OUT/identity" bin="$OUT/bin"
  check_views && corpus_of && tools > /dev/null || return 1
  mkdir -p "$dir" || return 1
  if [[ ! -f "$dir/report.txt" ]]; then
    "$bin/feature_audit" --data "$CORPUS" --free endgame_scale --no-columns \
        --no-collinearity > "$dir/report.txt" 2>&1 ||
      { tail -5 "$dir/report.txt"; say "feature_audit failed"; return 1; }
  fi
  head -n 2000 "$CORPUS" > "$dir/probe.tsv" || return 1
  "$bin/tuner" --data "$dir/probe.tsv" --out "$dir/probe.hpp" --vector "$dir/landed.vector" \
      --epochs 0 --only endgame_scale --threads "$THREADS" > "$dir/probe.log" 2>&1 ||
    { tail -5 "$dir/probe.log"; say "the probe run failed"; return 1; }
  python3 "$helper" landing-check "$dir/probe.hpp" || return 1
  if [[ "$SMOKE" == 1 ]]; then
    python3 "$helper" identity-check "$dir/report.txt" ||
      say "smoke: the identity rule read on a slice, not binding"
  else
    python3 "$helper" identity-check "$dir/report.txt" ||
      { say "identity: not the rule's -- the coordinator's, before any fit"; return 1; }
  fi
  python3 "$helper" ties "$dir/report.txt" --out "$dir" | tee "$dir/ties.txt"
  if (( PIPESTATUS[0] != 0 )); then
    [[ "$SMOKE" == 1 && -f "$dir/ties_f3.txt" ]] ||
      { say "the support rule stops: the coordinator's"; return 1; }
    say "smoke: the support rule read on a slice, not binding"
  fi
  echo "pass $(date -Iseconds) corpus $CORPUS_SHA head $STAMP" > "$dir/pass"
}

# ------------------------------------------------------------------------ K

k_of() { sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$1"; }
refit_of() { sed -n 's/^K \([0-9.]*\)$/\1/p' "$1"; }
data_of() { if [[ "$1" == full ]]; then echo "$CORPUS"; else echo "$OUT/samples/$1.tsv"; fi; }

k_stage() {
  local dir="$OUT/k" s x data
  check_views && corpus_of && tools > /dev/null || return 1
  [[ -f "$OUT/identity/pass" ]] || { say "refused: no identity pass"; return 1; }
  mkdir -p "$OUT/samples" "$dir" || return 1
  for s in $SEEDS; do
    if [[ ! -f "$OUT/samples/q$s.done" ]]; then
      rm -f "$OUT/samples/q$s.tsv"
      python3 "$split" sample --in "$CORPUS" --validation "${VIEWS[0]}" --validation "${VIEWS[1]}" \
          --validation "${VIEWS[2]}" --rows $(( ROWS / 4 )) --per-game 0 --seed "$s" \
          --out "$OUT/samples/q$s.tsv" | tee -a "$OUT/samples/samples.log"
      (( PIPESTATUS[0] == 0 )) || { say "sample q$s refused"; return 1; }
      sha256sum "$OUT/samples/q$s.tsv" | cut -d' ' -f1 > "$OUT/samples/q$s.done"
    fi
    [[ "$(sha256sum "$OUT/samples/q$s.tsv" | cut -d' ' -f1)" == "$(cat "$OUT/samples/q$s.done")" ]] ||
      { say "q$s is not the sample drawn"; return 1; }
  done
  for x in $SAMPLES; do
    data=$(data_of "$x")
    if [[ -z "$(k_of "$dir/$x.log" 2>/dev/null)" ]]; then
      "$OUT/bin/tuner" --data "$data" --holdout "$HOLDOUT" --out "$dir/$x.hpp" --epochs 0 \
          --only endgame_scale --threads "$THREADS" > "$dir/$x.log" 2>&1 ||
        { say "the K probe on $x failed"; return 1; }
    fi
    if [[ -z "$(refit_of "$dir/$x.refit" 2>/dev/null)" ]]; then
      "$OUT/bin/S124_k_refit" --data "$data" --threads "$THREADS" > "$dir/$x.refit" 2>&1 ||
        { say "S124_k_refit at the identity on $x failed"; return 1; }
    fi
    awk -v a="$(k_of "$dir/$x.log")" -v b="$(refit_of "$dir/$x.refit")" \
        'BEGIN { d = a - b; if (d < 0) d = -d; exit !(a > 0 && d <= 1e-4 + 1e-12) }' ||
      { say "refused: on $x the tuner's K $(k_of "$dir/$x.log") and S124_k_refit's $(refit_of "$dir/$x.refit") disagree"; return 1; }
    k_of "$dir/$x.log" > "$dir/$x.k"
    say "K $x at the identity $(cat "$dir/$x.k") (S124_k_refit $(refit_of "$dir/$x.refit"))"
  done
}

# -------------------------------------------------------------------- guard

guard_stage() {
  local dir="$OUT/guard"
  check_views && corpus_of && tools > /dev/null || return 1
  [[ -f "$OUT/k/full.k" ]] || { say "refused: no K at the identity; run k first"; return 1; }
  mkdir -p "$dir" || return 1
  if ! grep -qs '^S124-GUARD-OK$' "$dir/guard.txt"; then
    "$PY" "$guard_py" "$CORPUS" --out "$dir/guard.tsv" > "$dir/guard.txt" 2>&1 ||
      { tail -5 "$dir/guard.txt"; say "the guard's rows were not built"; return 1; }
    sha256sum "$dir/guard.tsv" | cut -d' ' -f1 > "$dir/guard.tsv.done"
  fi
  [[ "$(sha256sum "$dir/guard.tsv" | cut -d' ' -f1)" == "$(cat "$dir/guard.tsv.done")" ]] ||
    { say "$dir/guard.tsv is not the file built"; return 1; }
  # The cells each guard row picks, a reading; a failed census leaves no file behind
  # that a rerun would take as done (the fast check's finding 2).
  if ! grep -qs '^S124-CELLS-OK$' "$dir/cells.txt"; then
    "$OUT/bin/S124_cell_check" "$dir/guard.tsv" > "$dir/cells.txt.tmp" 2>&1 &&
      grep -q '^S124-CELLS-OK$' "$dir/cells.txt.tmp" ||
      { tail -5 "$dir/cells.txt.tmp"; rm -f "$dir/cells.txt.tmp"; say "the guard's cell census failed"; return 1; }
    mv "$dir/cells.txt.tmp" "$dir/cells.txt" || return 1
  fi
  grep -E '^(rows|verdicts|guard rows)' "$dir/guard.txt"
}

# ----------------------------------------------------------------------- P0

p0_stage() {
  local dir="$OUT/p0"
  check_views && tools > /dev/null || return 1
  mkdir -p "$dir" || return 1
  if ! grep -qs '^S124-P0-SELECT-OK$' "$dir/select.txt"; then
    "$PY" "$p0_py" select --pool "$POOL/pool.tsv.root" --heldout "$HOLDOUT" \
        --out "$dir/starts.tsv" --per-class "$P0_PER_CLASS" ${P0_SMOKE:+--smoke "$P0_SMOKE"} \
        > "$dir/select.txt" 2>&1 ||
      { tail -5 "$dir/select.txt"; say "the starts were not selected"; return 1; }
  fi
  if ! grep -qs '^S124-P0-PLAY-OK$' "$dir/play.txt"; then
    "$PY" "$p0_py" play --starts "$dir/starts.tsv" --engine "$OUT/bin/chesso" \
        --out "$dir/records.tsv" --nodes 100000 --workers "$THREADS" > "$dir/play.txt" 2>&1 ||
      { tail -5 "$dir/play.txt"; say "the conversion games failed"; return 1; }
  fi
  "$PY" "$p0_py" read "$dir/records.tsv" | tee "$dir/read.txt"
}

# --------------------------------------------------------------------- part

# One sample's fit set with DEC-352's K iteration, into DIR/r1, r2, ...; DIR/decided
# holds the round and its K.
fit_sample() {
  local dir="$1" data="$2" groups="$3" ties="$4" k="$5" r knew verdict tie=()
  [[ -n "$ties" ]] && tie=(--tie "$ties")
  if [[ -f "$dir/decided" ]]; then say "  $(basename "$dir") decided before: $(cat "$dir/decided")"; return 0; fi
  for (( r = 1; r <= ROUNDS; r++ )); do
    mkdir -p "$dir/r$r" || return 1
    if ! grep -qs '^best: ' "$dir/r$r/fit.log"; then
      check_copy "$OUT/bin/tuner" || return 1
      # shellcheck disable=SC2046
      "$OUT/bin/tuner" --data "$data" --holdout "$HOLDOUT" --out "$dir/r$r/fit.hpp" \
          --only "$groups" "${tie[@]}" --k "$k" --vector "$dir/r$r/fit.vector" $(fit_args) \
          > "$dir/r$r/fit.log" 2>&1 || { say "the fit $dir/r$r failed"; return 1; }
    fi
    if [[ -z "$(refit_of "$dir/r$r/k.refit" 2>/dev/null)" ]]; then
      "$OUT/bin/S124_k_refit" --data "$data" --vector "$dir/r$r/fit.vector" \
          --threads "$THREADS" > "$dir/r$r/k.refit" 2>&1 || { say "the K refit $dir/r$r failed"; return 1; }
    fi
    knew=$(refit_of "$dir/r$r/k.refit")
    verdict=$(python3 "$helper" converged "$k" "$knew")
    say "  $(basename "$dir") round $r at K $k: $(grep '^best: ' "$dir/r$r/fit.log"); K refit $knew, $verdict"
    if [[ "$verdict" == converged ]]; then
      echo "r$r K $k" > "$dir/decided"
      return 0
    fi
    k="$knew"
  done
  say "K-STOP: $dir has not reached its fixed point in $ROUNDS rounds: the coordinator's"
  return 1
}

part_stage() {
  local part="$1" text groups name pred tiefile="" x
  check_views && corpus_of && tools > /dev/null || return 1
  enough_disk || return 1
  [[ -f "$OUT/identity/pass" && -f "$OUT/k/full.k" && -f "$OUT/guard/guard.tsv.done" ]] ||
    { say "refused: run identity, k and guard first"; return 1; }
  text=$(python3 "$helper" plan "$part" --out "$OUT") || { echo "$text"; return 1; }
  echo "$text"
  groups=$(sed -n 's/^groups //p' <<< "$text"); name=$(sed -n 's/^name //p' <<< "$text")
  pred=$(sed -n 's/^pred //p' <<< "$text")
  [[ -n "$(sed -n 's/^ties //p' <<< "$text")" ]] && tiefile="$OUT/identity/ties_f3.txt"
  [[ -n "$tiefile" ]] && ! grep -qv '^#' "$tiefile" && tiefile=""
  {
    echo "S124 part $part, $(date -Iseconds), head $STAMP, epochs $FIT_EPOCHS"
    echo "plan      groups $groups name $name pred $pred ties ${tiefile:-(none)}"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
  } >> "$OUT/screen.log"
  for x in $SAMPLES; do
    fit_sample "$OUT/fits/$name/$x" "$(data_of "$x")" "$groups" "$tiefile" "$(cat "$OUT/k/$x.k")" |
      tee -a "$OUT/screen.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  done
  # The guard, DEC-357: the part's decided full fit against its predecessor's, both at
  # the part's decided K, by game; the same with both at K_0 beside it, deciding nothing.
  local gdir="$OUT/fits/$name" r k pvector=""
  read -r r _ k < <(sed 's/^r\([0-9]*\) K /\1 K /' "$gdir/full/decided")
  if [[ "$pred" != landed ]]; then
    local pr
    read -r pr _ _ < <(sed 's/^r\([0-9]*\) K /\1 K /' "$OUT/fits/$pred/full/decided")
    pvector="$OUT/fits/$pred/full/r$pr/fit.vector"
  fi
  guard_pair "$gdir/full/r$r/fit.vector" "$pvector" "$k" "$(cat "$OUT/k/full.k")" \
      "$gdir/guard" "$part ($name against $pred)" | tee "$gdir/guard.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  python3 "$helper" read "$part" --out "$OUT" --stamp "$STAMP"
}

# One guard census: VECTOR's integers (the build's own when empty) on the guard's rows at
# K, a game a line, into OUT.
guard_census() {
  local vector="$1" k="$2" out="$3" with=()
  [[ -n "$vector" ]] && with=(--vector "$vector")
  "$OUT/bin/margin_census" --data "$OUT/guard/guard.tsv" "${with[@]}" --k "$k" \
      --games "$out.games" --threads "$THREADS" > "$out.census" 2>&1 ||
    { tail -5 "$out.census"; say "margin_census on the guard failed: $out.census"; return 1; }
}

# DEC-357's guard over PART's vector against PRED's (empty: the landed tree), both at
# K_PART, and the reading beside at K_ZERO for both; the lines into stdout.
guard_pair() {
  local part="$1" pred="$2" kpart="$3" kzero="$4" base="$5" label="$6"
  guard_census "$part" "$kpart" "$base.part_kpart" &&
    guard_census "$pred" "$kpart" "$base.pred_kpart" &&
    guard_census "$part" "$kzero" "$base.part_k0" &&
    guard_census "$pred" "$kzero" "$base.pred_k0" || return 1
  python3 "$helper" guard-read "$base.pred_kpart.games" "$base.part_kpart.games" \
      --label "$label, both at the part's K $kpart" || return 1
  python3 "$helper" guard-read "$base.pred_k0.games" "$base.part_k0.games" --beside \
      --label "$label, both at K_0 $kzero" || return 1
}

# DEC-357's consequence, the smoke's: against F3's predecessor at F3's decided K, an
# inert part -- the predecessor with F3's own factors, whose cells no row of five men or
# fewer can stand in -- passes the guard, and a planted part that moves guard rows -- the
# predecessor with every factor at 4 F = 512, each cell row's E1 share quadrupled, which
# pushes the drawn rows' prediction off 0.5 -- is refused. Read on whatever OUT holds.
guard_probe_stage() {
  local text name pred r k pr pvector dir="$OUT/guard/probe"
  check_views && corpus_of && tools > /dev/null || return 1
  text=$(python3 "$helper" plan F3 --out "$OUT") || { echo "$text"; return 1; }
  name=$(sed -n 's/^name //p' <<< "$text"); pred=$(sed -n 's/^pred //p' <<< "$text")
  [[ -f "$OUT/fits/$name/full/decided" ]] || { say "refused: F3 is not fitted; run part F3 first"; return 1; }
  read -r r _ k < <(sed 's/^r\([0-9]*\) K /\1 K /' "$OUT/fits/$name/full/decided")
  if [[ "$pred" == landed ]]; then
    pvector="$OUT/identity/landed.vector"
  else
    read -r pr _ _ < <(sed 's/^r\([0-9]*\) K /\1 K /' "$OUT/fits/$pred/full/decided")
    pvector="$OUT/fits/$pred/full/r$pr/fit.vector"
  fi
  mkdir -p "$dir" || return 1
  python3 "$helper" vector-edit "$pvector" --cells 10,11,12 \
      --source "$OUT/fits/$name/full/r$r/fit.vector" --out "$dir/inert.vector" || return 1
  python3 "$helper" vector-edit "$pvector" --cells all --value 512 --out "$dir/planted.vector" || return 1
  guard_pair "$dir/inert.vector" "$pvector" "$k" "$(cat "$OUT/k/full.k")" "$dir/inert" \
      "inert probe (F3's factors alone on $pred)" | tee "$dir/inert.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  guard_pair "$dir/planted.vector" "$pvector" "$k" "$(cat "$OUT/k/full.k")" "$dir/planted" \
      "planted probe (every factor at 512 on $pred)" | tee "$dir/planted.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  grep -q '^GUARD pass ' "$dir/inert.txt" || { say "refused: the inert probe did not pass the guard"; return 1; }
  grep -q '^GUARD refused ' "$dir/planted.txt" || { say "refused: the planted probe passed the guard"; return 1; }
  say "guard probes: the inert part passes, the planted part is refused"
}

read_stage() {
  tools > /dev/null || return 1
  python3 "$helper" read "$1" --out "$OUT" --stamp "$STAMP"
}

all_stage() {
  local part
  locate && tools || return 1
  identity || { say "S124-IDENTITY-FAILED"; return 1; }
  k_stage || { say "S124-K-FAILED"; return 1; }
  guard_stage || { say "S124-GUARD-FAILED"; return 1; }
  p0_stage || { say "S124-P0-FAILED"; return 1; }
  for part in F1 F2 F3; do
    if grep -qsE '^PART-OUTCOME (kept|dropped)$' "$OUT/read/$part.txt"; then
      say "$part read before: $(grep '^PART-OUTCOME' "$OUT/read/$part.txt")"
      continue
    fi
    part_stage "$part" || { say "S124-PART-FAILED ($part)"; return 1; }
  done
  grep -E '^(survivors|CANDIDATE)' "$OUT/read/F3.txt"
}

# ------------------------------------------------------------------- smoke
# Every stage end to end on a slice: the first SMOKE_ROWS rows of the corpus and of the
# held-out view, written under the smoke's own OUT with a record naming them, 300 epochs,
# P0 at two starts a class. Proves the stages run; reads nothing, and the identity rule on
# a slice is printed and not binding.
smoke_stage() {
  local dir="${SMOKE_OUT:-$root/.tuning/s124/smoke}" rows="${SMOKE_ROWS:-40000}"
  check_views || return 1
  local corpus; corpus=$(record_field corpus)
  rm -rf "$dir" && mkdir -p "$dir/prepared" || return 1
  head -n "$rows" "$corpus" > "$dir/prepared/corpus.tsv"
  local v
  for v in qs root pv; do
    head -n $(( rows / 4 )) "$(view_path "$v")" > "$dir/prepared/view.$v"
  done
  {
    echo "S124 smoke prepared corpus, $(date -Iseconds), a slice of $corpus"
    echo "corpus $dir/prepared/corpus.tsv"
    echo "sha256 $(sha256sum "$dir/prepared/corpus.tsv" | cut -d' ' -f1)"
    echo "rows $rows"
    for v in qs root pv; do
      echo "view $v $dir/prepared/view.$v $(sha256sum "$dir/prepared/view.$v" | cut -d' ' -f1)"
    done
    echo "pool $POOL"
    echo "S122-PREPARED"
  } > "$dir/prepared/prepared.txt"
  local s
  for s in locate tools identity k guard p0 "part F1" "part F2" "part F3" guard-probe; do
    say "== smoke: $s"
    OUT="$dir/out" PREPARED="$dir/prepared/prepared.txt" SMOKE=1 FIT_EPOCHS=300 P0_SMOKE=2 \
      CORPUS_SHA="$(sha256sum "$dir/prepared/corpus.tsv" | cut -d' ' -f1)" \
      bash "$0" $s > "$dir/stage.log" 2>&1
    tail -n 4 "$dir/stage.log"
    grep -qE '^S124-[A-Z0-9-]+-DONE$' "$dir/stage.log" || { say "smoke: $s failed: $dir/stage.log"; return 1; }
  done
  say "smoke: every stage ran; nothing here is a reading"
}

mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S124-FAILED"; exit 1; }
HOLDOUT=""; VIEWS=(); CORPUS=""; ROWS=""; STAMP=""

case "$stage" in
  locate) if locate; then echo S124-LOCATE-DONE; else echo S124-LOCATE-FAILED; exit 1; fi ;;
  tools) if tools; then echo S124-TOOLS-DONE; else echo S124-TOOLS-FAILED; exit 1; fi ;;
  identity) if identity; then echo S124-IDENTITY-DONE; else echo S124-IDENTITY-FAILED; exit 1; fi ;;
  k) if k_stage; then echo S124-K-DONE; else echo S124-K-FAILED; exit 1; fi ;;
  guard) if guard_stage; then echo S124-GUARD-DONE; else echo S124-GUARD-FAILED; exit 1; fi ;;
  p0) if p0_stage; then echo S124-P0-DONE; else echo S124-P0-FAILED; exit 1; fi ;;
  part)
    if [[ "${2:-}" =~ ^F[123]$ ]] && part_stage "$2"; then echo S124-PART-DONE; else echo S124-PART-FAILED; exit 1; fi ;;
  read)
    if [[ "${2:-}" =~ ^F[123]$ ]] && read_stage "$2"; then echo S124-READ-DONE; else echo S124-READ-FAILED; exit 1; fi ;;
  all) if all_stage; then echo S124-ALL-DONE; else echo S124-ALL-FAILED; exit 1; fi ;;
  guard-probe) if guard_probe_stage; then echo S124-GUARD-PROBE-DONE; else echo S124-GUARD-PROBE-FAILED; exit 1; fi ;;
  smoke) if smoke_stage; then echo S124-SMOKE-DONE; else echo S124-SMOKE-FAILED; exit 1; fi ;;
  *)
    echo "usage: $0 locate | tools | identity | k | guard | p0 | part F1|F2|F3 | read F1|F2|F3 | all | smoke | guard-probe" >&2
    exit 2 ;;
esac
