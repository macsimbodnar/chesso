#!/usr/bin/env bash
# S122's screen: the margin's fixed point, then the king safety family's parts in
# fixed order at that margin, each with the earlier survivors live, by held-out loss
# on the new corpus's fixed set (DEC-259 (a), DEC-331's form, DEC-344 (3) and (4)):
# P0, the margin alone; P1, safe checks; P2, weak squares; P3, defenders by square;
# P4, the storm; P5, the curve; P6, its wide zone; P7, the linear five retired. The
# last survivor's full fit is the candidate S122's one family SPRT measures. Then
# DEC-344 (1)'s two side readings.
#
#   adocs/data/S122_screen.sh locate          # the prepared corpus, its sha256
#   adocs/data/S122_screen.sh audit           # identity, support, the ties
#   adocs/data/S122_screen.sh landing-check   # HEAD's family at its seed
#   adocs/data/S122_screen.sh margin          # DEC-344 (4)'s fixed point; M
#   adocs/data/S122_screen.sh p0              # the margin alone, shipped constants
#   adocs/data/S122_screen.sh screen PART     # PART's six fits; resumable
#   adocs/data/S122_screen.sh read PART       # the rule over them
#   adocs/data/S122_screen.sh read PART --cap 80000 # the rule over DEC-345's re-run
#   adocs/data/S122_screen.sh all             # locate, audit, margin, p0, P1 to P7
#   adocs/data/S122_screen.sh sample          # side (b)'s sample, after the corpus
#   adocs/data/S122_screen.sh side            # the two side readings
#   adocs/data/S122_screen.sh part NAME       # a bisection part's fit, on H0, at M
#   adocs/data/S122_screen.sh part NAME --margin 184 # the same refit at 184
#   adocs/data/S122_screen.sh apply TREE [FIT] # the candidate (or a part) into TREE
#
# PART is P1 to P7, NAME A, B or C. Run from the main checkout with S122's phase 1
# committed (the family at its seed, `No functional change`) and both build/ and
# build-tune/ built at HEAD: the fits run on build-tune's tuner, the one `--margin`
# can set the clamp of, so that a fit's clamp and its load's identity are at the
# screen's margin (DEC-344 (4)); feature_audit and margin_census are build/'s. Each
# stage prints S122-<STAGE>-DONE or S122-<STAGE>-FAILED as its last stdout line, the
# reasons above a FAILED, so each runs detached under a four-exit watcher (WATCHERS).
# OUT defaults to .tuning/s122/screen (DEC-235).
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-11-02 in S122's phase 1, before any fit and
# before the corpus finished generating. Nothing below the line changes once
# `margin` has started; a change is a dated section in the step file and a
# decision. DEC-344 is the coordinator's ruling it carries.
# ============================================================================
#
# THE CORPUS: head's own games, DEC-344 (1), prepared by adocs/data/S122_corpus.sh
# from .tuning/s122/corpus/: every game of the training side at 46 qs rows a game,
# the recipe's filter at 1000 on column 3, S066's split by game, the fixed set of
# 12000 games by S082's rule. `locate` reads the preparation's record and holds every
# later stage to its sha256s; the corpus and the set's three views are not known
# before the preparation runs, so they are pinned by `locate` from the record and not
# here, and a record whose prepare stamp is missing is refused.
#
# THE PARTS, from their descriptions (DEC-221; src/evaluation.hpp states each count):
#   P0  the margin M alone at the shipped constants: no fit. Its reading is the fixed
#       set's error at the shipped margin less at M, one K, the full corpus's at M;
#       every later part is paired against fits started from the shipped constants at
#       M, whose start: is P0 at M.
#   P1  ks_checks: 8. Safe checks by type N B R Q, empty targets.
#   P2  ks_weak: 2. Zone squares A's men attack that no man of K's but the king does.
#   P3  ks_defenders: 2. Zone squares K's knights and bishops attack.
#   P4  ks_storm: 14. Seven cells, distance 1 blocked mapped to distance 1.
#   P5  the curve, T(x) = max(0, x)^2 / 2048 per side and half: ks_curve.core (the
#       constant and the five counts the linear term reads) and the curve terms of the
#       P1 to P3 survivors (ks_curve.checks, .weak, .defenders): at most 24. c starts
#       at its seed, 45; the linear cells stay live.
#   P6  only if P5 is kept: P5's free set on a variant tree,
#       adocs/data/S122_p6_wide_zone.patch, whose curve reads its attackers and
#       incidences over the zone and its forward rank (the research pass's mask, its
#       attackers_plus and incidences_plus); the linear five keep the zone. Read
#       against P5's fit. The zone and its forward rank are up to 12 squares, so the
#       incidences input reaches 15 x 12 = 180, not 135: the patch moves the engine's
#       bound and `headroom --wide` reads it (S122's fast check, finding 8).
#   P7  only if P5 is kept: the shipped linear five, king_safety's attackers N B R Q
#       and incidences, at zero (S122_screen.py zero-five) on a variant tree, P6's
#       patch in it too if P6 was kept, every survivor refit. Kept iff d >= -m.
#   A dropped part's weights stay zero, and c is never dropped. At most 50 new
#   parameters.
#
# THE IDENTITY CHECK, before any fit. `audit` runs tools/feature_audit `--free` every
# S122 group over the corpus and passes only on S122_screen.py audit-check's rule:
# rank over the rationals exactly 1100 of 1171, nullity 71; the zero columns the 32
# dead pawn squares and the curve constant's two, nothing else; 37 dependencies by
# construction, each holding on every row -- S134's 7, S121's 8 and the curve's 22
# duplicates of linear cells at its seed -- and no exact dependency beyond them; the
# span line's verdict naming the curve duplicates. Anything else stops the screen for
# the coordinator before any fit.
#
# THE TIES (DEC-331 (6), DEC-326 (5)): from the report's support lines, a linear
# cell's half is supported from TIE_GAMES = 1000 games; an unsupported one is tied to
# the nearest supported cell of its array and half. The curve has no tie row: an
# input half under 1000 games stops the screen. The research pass read every input's
# half on at least 37097 of S083's games; the new corpus is read, not assumed.
#
# THE MARGIN, DEC-344 (4). One fit of every part -- ks_checks, ks_weak, ks_defenders,
# ks_storm, ks_curve -- under 184 on the full corpus, its K fitted at 184, then
# tools/margin_census at the fit's own vector: it passes at 184 when at most 0.5 % of
# rows bind and the graded set's bound rows hold at most 10 % of the graded set's
# gradient mass at the clamped score. Otherwise M is the smallest multiple of 25
# above 184 that passes, the fit is redone from the same start under M (build-tune's
# tuner, `--margin M`, its K at M) and read again: the fixed point when M passes and is
# the pick, at most three rounds, then MARGIN-STOP for the coordinator. The screen and
# the candidate run at M. Readings, deciding nothing: every round's eight margins'
# rows and mass, unclamped mass beside, the x > 0 shares.
#
# THE FIT, DEC-304's settings as the screens before it: `tuner --data DATA --holdout
# FIXED_SET_QS --margin M --only GROUPS --tie TIES --k K --vector FIT.vector
# --epochs FIT_EPOCHS --report 100 --patience 20 --lambda 0 --seed 1 --threads 12`,
# at the tuner's own step, Adam at lr 1 (DEC-344 (2): lr 1 at d 2048 is S262's
# working band). The epoch cap is FIT_EPOCHS = 20000 for every fit -- the margin's,
# the screen's, the side readings' and a bisection part's -- at --report 100
# --patience 20 (DEC-345; phase 1's planted recovery: the curve's slow direction).
# Every fit set's directory names its cap (fits/NAME_eCAP), the cap a fit ran at is
# read from its header's `// run N epochs max` line, and a pair or a reuse across
# caps is refused; M.txt names the cap it was found at. A part pairs with its
# predecessor's decided fit and runs at that fit's cap. K is fitted once per sample
# at M by a zero-epoch run at the shipped constants. One frozen copy of build-tune's
# tuner for the landing's fits, one a variant tree.
#
# THE ESCALATION, DEC-345. A cap can only bias the curve towards a zero, so P5 or P6
# kept, or P7 passing, at a capped fit stays so. P5 or P6 dropped, or P7 failing
# non-inferiority, where either fit of its pair stopped on the cap: `read` prints
# ESCALATE and PART-OUTCOME escalate, `screen` re-runs both sides of that pair at
# 80000 epochs (the predecessor refitted with its own free set and tree), and
# `read --cap 80000` decides on the 80000 pair. Both pairs are recorded. A fit of the
# 80000 pair stopping on its cap prints STOP and the screen stops for the
# coordinator.
#
# THE NOISE: five samples at a quarter of the corpus's rows, `corpus_split.py sample
# --rows C/4 --per-game 0 --seed s`, s = 1 to 5, the three views named, drawn once.
#
# THE READING, by S122_screen.py read, mechanically (DEC-326 (3), DEC-329 (10)): d_x =
# best(pred, x) - best(part, x) for x the full corpus and each quarter; s the standard
# deviation of the five quarter d; m = max(1.645 s, 2e-6); kept iff full d > m, P7's
# iff d >= -m. With no survivor before a part, d is the part's start less its best:
# P0 at M. Unreadable: a fit missing or refused, a log whose start: and best: lines
# disagree, a fit at another margin or cap, s above 1e-3, a full fit past either
# assert, a curve part's decided fit with no census at its end. No re-seed rule
# (DEC-345): every group fit starts at the seed, where every side's x is the
# constant and positive. Printed beside: every fit's start, best, epoch and stop; the
# full fit's family arrays; a curve part's x > 0 share at its decided fit's end, by
# its tree's own census, a reading with no rule.
#
# THE CANDIDATE: the last survivor's full fit, freeing every survivor, on the tree its
# parts name. No survivor: no candidate, each part's zero recorded, the family at its
# seed; and if M moved, the margin alone is read only as DEC-344 (6)'s first part.
#
# THE SIDE READINGS, DEC-344 (1), `side`, after P7 is read:
#   (a) the candidate's free set fitted on S083's corpus (the 6226944 rows every screen
#       since S135 fitted, sha256 2e0e3de9..., less its rows on a position the new
#       fixed set holds) and read on the new fixed set at M, beside the candidate's own
#       best there: the corpus change's price, for S126. Deciding nothing.
#   (b) P0, every part's decided full fit and the candidate read on the 12000-game
#       sample, .tuning/s122/sample/qs_inside.tsv (DEC-345 (5); the first sample,
#       .tuning/s122/smoke/qs_inside.tsv, is refused, below), less its rows on a
#       position the corpus or the fixed set holds, at the sample's own K (shipped
#       constants, M):
#       P0 the shipped constants at 184 against them at M, beside d0; each part's
#       decided fit against the fit it was decided against (its start: P0 at M), by
#       the census of the tree it was fitted on (P6's counts are its own build's).
#       Paired by game, a game a run of one id at a rising ply, so the sample's two
#       pools, whose ids both start at 0, stay apart; each reading prints the games
#       and rows it paired (S122's fast check, finding 1). STOP, the coordinator's
#       before any SPRT, when a part's sample d is of the other sign from its screen d
#       and beyond 1.96 errors by game. `sample-prep` refuses a sample more than 1 %
#       of whose games sit over half on an excluded position, or more than 0.1 % of
#       whose games (12 of 12000) repeat another's rows (DEC-345 (5)): independent
#       workers' 8 random plies transpose into one game about 4.7e-9 a pair, 0.34
#       expected in 12000 games, and those stay in, counted; a worker seed two pools
#       share repeats about 1000. datagen seeds worker w from seed + w, so the
#       first sample, two pools at seeds 2026110201 and 2026110202 beside the
#       corpus's 2026110203, was mostly the corpus's own games and 2750 of them
#       twice. DEC-345 (5): side (b) reads a fresh sample, `sample`'s, seed
#       2026260203, 12000 games, generated after the corpus's preparation and before
#       the screen, the guard its last act, so that a refusal is read before the
#       night is spent.
#
# THE CLAMP, READ AT `apply` (DEC-333 (6)): the candidate's census at M, the share of
# rows bound against 1.0 %; past it phase 2 stops for the coordinator.
#
# THE BISECTION, ON H0 OR NO VERDICT (DEC-344 (6)), in order, each stopping for the
# coordinator: M alone at the shipped constants, {-5, 0}, if M is not 184; then A = P1,
# B = P2 to P4, C = P5 to P7, survivors only, each {0, 5} against what has landed,
# each refit by `part NAME` at M, or by `part NAME --margin 184` when M alone's answer
# says the rest refit at 184; no other margin. The margin names the part's directory
# (part_NAME_mM_eCAP) and its K, its headers say it, and nothing fitted at one margin
# is reused, pasted or read as another's: `apply` pastes a part at its own margin and
# K, moving src/search_params.hpp's row only when the tree's differs. A part on a P6
# or P7 tree builds its variant at the HEAD it runs at, its own directory.
#
# MACHINE HOURS, inferred, not measured: S101's screen ran a part (six fits, 2.25
# full-corpus fits) at 0.58 s an epoch at 37.37 trace entries a row over 6226944
# rows (24.2 min at patience, 48.6 min at the 5000 cap); the family adds about 13
# entries a row (x 1.35, its cost unmeasured) and the corpus is about 6.9 M rows
# (x 1.11): about 0.87 s a part-epoch. A linear part at S101's patience, about 2500
# epochs: about 36 min. A part at the 20000 cap: about 4 h 50 m; the margin's round,
# one full fit at the cap: about 2 h 10 m; P0 minutes. Typical, one margin round,
# P1 to P4 at patience and P5 to P7 on the cap with no escalation: about 19 h. Each
# escalation adds both sides at 80000, about 19 h 30 m each, a side fitted before
# reused: up to 39 h. Worst, three margin rounds, every part on the cap and every
# escalation the rule allows: about 110 h. Every figure is over four hours: DEC-155's
# night, the coordinator's schedule; a watcher's ceiling is 2 x the stage's figure.
# `side` about 2 h (one fit on S083's corpus at the cap, the sample's censuses
# minutes); `part` up to 2 h 10 m; `apply` about 15 min.
#
# DISK. The five quarter samples, about 140 MB each, the variant worktrees with their
# build-tune about 300 MB each. A stage refuses to start under 4 GB free.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s122/screen}"
PREPARED="${PREPARED:-$root/.tuning/s122/prepared/prepared.txt}"
S083_RECORD="${S083_RECORD:-$root/.tuning/s101/corpus.txt}"
S083_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
# DEC-345 (5): side (b)'s sample, generated by `sample` from a seed range the
# corpus's does not meet; SAMPLE is its inside file unless a smoke names another.
SAMPLE_DIR="${SAMPLE_DIR:-$root/.tuning/s122/sample}"
CORPUS_POOL="${CORPUS_POOL:-$root/.tuning/s122/corpus}"
# A merged pool's log starts so (adocs/data/S122_corpus.sh's MERGED_HEADER, DEC-296).
CORPUS_MERGED_HEADER="S122 corpus, merged from two datagen runs (DEC-296); not one run's log"
SAMPLE_SEED=2026260203
SAMPLE_GAMES="${SAMPLE_GAMES:-12000}"  # overridable for a smoke and nothing else
SAMPLE="${SAMPLE:-$SAMPLE_DIR/qs_inside.tsv}"
THREADS=12
FREE_GB_FLOOR=4
SEEDS="1 2 3 4 5"
EVERY_GROUP="ks_checks,ks_weak,ks_defenders,ks_storm,ks_curve"
SHIPPED_MARGIN=184
# DEC-345: every fit's cap is 20000, the curve's slow direction's; a curve pair
# dropped on a fit that stopped on its cap is re-run at 80000. Each fit set's
# directory names its cap and each header says it, and nothing pairs or reuses
# across caps. Both are overridable for a smoke and for nothing else.
FIT_EPOCHS="${FIT_EPOCHS:-20000}"
ESCALATE_EPOCHS="${ESCALATE_EPOCHS:-80000}"
export FIT_EPOCHS ESCALATE_EPOCHS
fit_args() {
  echo "--epochs $1 --report 100 --patience 20 --lambda 0 --seed 1 --threads $THREADS"
}
# A header's cap, its `// run N epochs max` line, and its margin, `// margin N,`.
cap_of_header() { sed -n 's#^// run *\([0-9]*\) epochs max.*#\1#p' "$1" | head -n 1; }
margin_of_header() { sed -n 's#^// margin *\([0-9]*\),.*#\1#p' "$1" | head -n 1; }

split="$root/tools/corpus_split.py"
helper="$root/adocs/data/S122_screen.py"
capacity_helper="$root/adocs/data/S083_corpus.py"
p6_patch="$root/adocs/data/S122_p6_wide_zone.patch"

say() { echo "$*"; }

binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

engine_stamp() {
  local untracked
  untracked=$(git -C "$root" ls-files --others -- src tools |
                grep -v '/__pycache__/')
  if [[ -n "$untracked" ]]; then
    { echo "refused: untracked files under src/ or tools/:"; echo "$untracked"; } >&2
    return 1
  fi
  git -C "$root" diff --quiet HEAD -- src tools || {
    echo "refused: src/ or tools/ differ from HEAD; the screen reads HEAD's constants and needs the builds built from them" >&2
    return 1
  }
  git -C "$root" rev-parse --short HEAD
}

enough_disk() {
  local free_gb
  mkdir -p "$OUT"
  free_gb=$(df --output=avail -BG "$OUT" | tail -n 1 | tr -dc '0-9')
  if (( free_gb < FREE_GB_FLOOR )); then
    say "refused: ${free_gb} GB free under $OUT, under the ${FREE_GB_FLOOR} GB a stage needs"
    return 1
  fi
}

freeze_binary() {
  local src="$1" copy="$2"
  mkdir -p "$(dirname "$copy")" || return 1
  if [[ ! -f "$copy" ]]; then
    [[ -x "$src" ]] || { echo "no binary at $src"; return 1; }
    cp "$src" "$copy.tmp" && chmod +x "$copy.tmp" || return 1
    sha256sum "$copy.tmp" | cut -d' ' -f1 > "$copy.sha256.tmp" || return 1
    mv "$copy.tmp" "$copy" && mv "$copy.sha256.tmp" "$copy.sha256" || return 1
  fi
  [[ -f "$copy.sha256" ]] || { echo "$copy has no recorded sha256"; return 1; }
  check_copy "$copy" || return 1
  cat "$copy.sha256"
}

check_copy() {
  if [[ "$(sha256sum "$1" | cut -d' ' -f1)" != "$(cat "$1.sha256")" ]]; then
    echo "refused: $1 no longer has the sha256 recorded when it was copied"
    return 1
  fi
}

# The tuner must be S122's tune build: the family's groups and --margin taking a
# value, which the default build refuses.
check_tuner() {
  local usage
  usage=$("$1" --help 2>&1)
  [[ "$usage" == *ks_curve.core* && "$usage" == *--margin* &&
     "$usage" == *--vector* ]] || {
    say "refused: $1 has no S122 groups, --margin or --vector"
    return 1
  }
  "$1" --data /dev/null --margin "$SHIPPED_MARGIN" 2>&1 | grep -q 'needs the tune build' && {
    say "refused: $1 is not the tune build; --margin needs build-tune/tools/tuner"
    return 1
  }
  return 0
}

# The prepared corpus's record, as S122_corpus.sh wrote it: corpus, sha256, rows,
# and the fixed set's three views with theirs.
record_field() { sed -n "s/^$1 //p" "$PREPARED" | head -n 1; }

view_path() { awk -v v="$1" '$1 == "view" && $2 == v { print $3 }' "$PREPARED"; }
view_sha() { awk -v v="$1" '$1 == "view" && $2 == v { print $4 }' "$PREPARED"; }

check_views() {
  local v path sha
  [[ -f "$PREPARED" ]] || { say "no $PREPARED; run adocs/data/S122_corpus.sh all first"; return 1; }
  grep -q '^S122-PREPARED$' "$PREPARED" || { say "refused: $PREPARED has no S122-PREPARED stamp"; return 1; }
  for v in qs root pv; do
    path=$(view_path "$v"); sha=$(view_sha "$v")
    [[ -n "$path" && -n "$sha" ]] || { say "$PREPARED names no $v view"; return 1; }
    [[ "$(sha256sum "$path" | cut -d' ' -f1)" == "$sha" ]] ||
      { say "the fixed set's $path is not the file the preparation recorded"; return 1; }
  done
  HOLDOUT=$(view_path qs)
  VIEWS=("$(view_path qs)" "$(view_path root)" "$(view_path pv)")
  say "fixed set: the three views' sha256 are the preparation's"
}

corpus_of() {
  local record="$OUT/corpus.txt" path sha rows
  [[ -f "$record" ]] || { say "no $record; run locate first"; return 1; }
  path=$(sed -n 's/^corpus //p' "$record")
  sha=$(sed -n 's/^sha256 //p' "$record")
  rows=$(sed -n 's/^rows //p' "$record")
  [[ -n "$path" && -n "$sha" && -n "$rows" ]] || { say "$record is incomplete"; return 1; }
  [[ "$sha" == "$(record_field sha256)" ]] ||
    { say "refused: $record names $sha, the preparation $(record_field sha256)"; return 1; }
  [[ "$(sha256sum "$path" | cut -d' ' -f1)" == "$sha" ]] ||
    { say "$path is not the corpus $record names"; return 1; }
  echo "$path $rows"
}

locate() {
  enough_disk || return 1
  check_views || return 1
  if [[ -f "$OUT/corpus.txt" ]]; then
    say "$OUT/corpus.txt exists; a located corpus is never replaced"
    cat "$OUT/corpus.txt"
    local info
    info=$(corpus_of) || { echo "$info"; return 1; }
    return 0
  fi
  local path sha rows
  path=$(record_field corpus); sha=$(record_field sha256); rows=$(record_field rows)
  [[ -n "$path" && -n "$sha" && -n "$rows" ]] || { say "$PREPARED is incomplete"; return 1; }
  [[ "$(sha256sum "$path" | cut -d' ' -f1)" == "$sha" ]] ||
    { say "$path is not the corpus $PREPARED names"; return 1; }
  printf 'corpus %s\nsha256 %s\nrows %s\nprepared %s\n' "$path" "$sha" "$rows" \
    "$(sha256sum "$PREPARED" | cut -d' ' -f1)" > "$OUT/corpus.txt"
  cat "$OUT/corpus.txt"
}

# build-tune's tuner and build/'s feature_audit and margin_census, frozen under
# $1/bin with the HEAD they were built from.
tools() {
  local dir="$1" head had
  head=$(engine_stamp) || return 1
  mkdir -p "$dir/bin" || return 1
  if [[ -f "$dir/bin/stamp" ]]; then
    had=$(cat "$dir/bin/stamp")
    [[ "$had" == "$head" ]] ||
      { say "refused: $dir/bin was frozen at $had, not HEAD $head; take a fresh OUT"; return 1; }
    check_copy "$dir/bin/tuner" && check_copy "$dir/bin/feature_audit" &&
      check_copy "$dir/bin/margin_census" || return 1
    return 0
  fi
  freeze_binary "$root/build-tune/tools/tuner" "$dir/bin/tuner" > /dev/null || return 1
  freeze_binary "$root/build/tools/feature_audit" "$dir/bin/feature_audit" > /dev/null || return 1
  freeze_binary "$root/build/tools/margin_census" "$dir/bin/margin_census" > /dev/null || return 1
  check_tuner "$dir/bin/tuner" || return 1
  echo "$head" > "$dir/bin/stamp"
  say "tools at $head: tuner $(cut -c1-16 "$dir/bin/tuner.sha256"), feature_audit $(cut -c1-16 "$dir/bin/feature_audit.sha256"), margin_census $(cut -c1-16 "$dir/bin/margin_census.sha256")"
}

# A variant tree for P6 or P7 (or both): a worktree at HEAD under
# $OUT/variant_NAME_HEAD, the patch and the zeroing applied, its build-tune tuner
# frozen beside it. Its trace is its own evaluate()'s, which its tuner's load checks
# on every row. The directory belongs to the HEAD it was built at, so a bisection
# part after a landing builds its own, and one built at another HEAD is never read.
variant_dir() { echo "$OUT/variant_${1}_$2"; }

variant() {
  local name="$1" dir head
  head=$(engine_stamp) || return 1
  dir=$(variant_dir "$name" "$head")
  if [[ -f "$dir/bin/stamp" ]]; then
    [[ "$(cat "$dir/bin/stamp")" == "$head" ]] ||
      { say "refused: $dir was built at $(cat "$dir/bin/stamp"), not $head"; return 1; }
    check_copy "$dir/bin/tuner" || return 1
    echo "$dir/bin/tuner"
    return 0
  fi
  if [[ ! -d "$dir/tree" ]]; then
    git -C "$root" worktree add --detach "$dir/tree" HEAD > "$dir.worktree.log" 2>&1 ||
      { say "the worktree for $name failed: $dir.worktree.log"; return 1; }
  fi
  if [[ "$name" == *p6* ]]; then
    git -C "$dir/tree" apply --check "$p6_patch" 2> /dev/null &&
      git -C "$dir/tree" apply "$p6_patch" || { say "S122_p6_wide_zone.patch does not apply"; return 1; }
  fi
  if [[ "$name" == *p7* ]]; then
    python3 "$helper" zero-five "$dir/tree" || return 1
  fi
  if [[ -z "$(ls -A "$dir/tree/tests/doctest" 2>/dev/null)" ]]; then
    git -C "$dir/tree" config submodule.tests/doctest.url "$root/tests/doctest" &&
      git -C "$dir/tree" -c protocol.file.allow=always submodule update --init \
        > "$dir.submodule.log" 2>&1 || { say "the doctest submodule did not check out"; return 1; }
  fi
  ( cd "$dir/tree" &&
    cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCHESSO_TUNE=ON > /dev/null &&
    cmake --build build-tune -j"$THREADS" --target tuner margin_census ) > "$dir.build.log" 2>&1 ||
    { tail -20 "$dir.build.log"; say "the $name variant's tools did not build"; return 1; }
  freeze_binary "$dir/tree/build-tune/tools/tuner" "$dir/bin/tuner" > /dev/null || return 1
  freeze_binary "$dir/tree/build-tune/tools/margin_census" "$dir/bin/margin_census" > /dev/null || return 1
  echo "$head" > "$dir/bin/stamp"
  echo "$dir/bin/tuner"
}

tuner_for() {
  local tree="$1"
  if [[ "$tree" == landing ]]; then echo "$OUT/bin/tuner"; return 0; fi
  variant "${tree//+/_}" | tail -n 1
}

# The census whose trace is the tree's own: a variant's counts (P6's wide zone)
# are its build's, so its x > 0 is read by its own margin_census.
census_for() {
  local tree="$1" head
  if [[ "$tree" == landing ]]; then echo "$OUT/bin/margin_census"; return 0; fi
  head=$(engine_stamp) || return 1
  echo "$(variant_dir "${tree//+/_}" "$head")/bin/margin_census"
}

probe_tuner() {
  local copy="$1" corpus="$2" dir="$3" five="$4"
  head -n 2000 "$corpus" > "$dir/probe.tsv" || return 1
  check_copy "$copy" || return 1
  "$copy" --data "$dir/probe.tsv" --out "$dir/probe.hpp" --epochs 0 \
      --margin "$M" --only "$EVERY_GROUP" --threads "$THREADS" > "$dir/probe.log" 2>&1 ||
    { tail -5 "$dir/probe.log"; say "the probe run of $copy failed"; return 1; }
  python3 "$helper" start-check "$dir/probe.hpp" --tree "$root" $five
}

audit_once() {
  local base="$1" info corpus rows dir
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$base/audit"
  if [[ -f "$dir/pass" ]] && grep -qs "corpus $(record_field sha256)" "$dir/pass"; then
    say "audited before: $(cat "$dir/pass")"
    return 0
  fi
  mkdir -p "$dir" || return 1
  tools "$base" || return 1
  local bin="$base/bin"
  {
    echo "S122 audit, $(date -Iseconds)"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line audit "$bin/feature_audit"
    echo "head      $(cat "$bin/stamp")"
  } > "$dir/audit.log"
  check_copy "$bin/feature_audit" || return 1
  "$bin/feature_audit" --data "$corpus" --free "$EVERY_GROUP" > "$dir/report.txt" 2>&1 ||
    { tail -20 "$dir/report.txt"; say "feature_audit failed"; return 1; }
  grep -E '^identities: (rank|no exact|.*null dimensions|the curve|curve duplicate)' \
      "$dir/report.txt" | tee -a "$dir/audit.log"
  python3 "$helper" audit-check "$dir/report.txt" | tee -a "$dir/audit.log"
  (( PIPESTATUS[0] == 0 )) ||
    { say "identity: not the rule's -- the coordinator's, before any fit ($dir/report.txt)"; return 1; }
  python3 "$helper" ties "$dir/report.txt" --out "$dir" | tee -a "$dir/audit.log"
  (( PIPESTATUS[0] == 0 )) || { say "the ties refused"; return 1; }
  echo "pass $(date -Iseconds) corpus $(record_field sha256)" > "$dir/pass"
}

k_check() {
  local value
  value=$(cat "$1" 2> /dev/null) || { say "refused: no K file $1"; return 1; }
  if [[ ! "$value" =~ ^[0-9]*\.?[0-9]+$ ]] ||
     ! awk -v k="$value" 'BEGIN { exit !(k > 0) }'; then
    say "refused: $1 holds '$value', not a positive K; remove it and re-run its probe"
    return 1
  fi
}

# K at margin $2 on data $1 into $3, a zero-epoch run at the shipped constants.
k_probe() {
  local data="$1" margin="$2" file="$3" tuner="$OUT/bin/tuner"
  if [[ -f "$file" ]]; then k_check "$file"; return; fi
  check_copy "$tuner" || return 1
  "$tuner" --data "$data" --holdout "$HOLDOUT" --out "${file%.k}.hpp" --margin "$margin" \
      --epochs 0 --only "$EVERY_GROUP" --threads "$THREADS" > "${file%.k}.log" 2>&1 ||
    { tail -5 "${file%.k}.log"; say "the K probe on $data failed"; return 1; }
  sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "${file%.k}.log" > "$file"
  k_check "$file" || { say "the K probe on $data printed no K"; return 1; }
}

# The screen's margin, M, once `margin` has written it.
margin_of() {
  local cap
  [[ -f "$OUT/margin/M.txt" ]] || { say "refused: no $OUT/margin/M.txt; run margin first"; return 1; }
  M=$(awk '$1 == "MARGIN" { print $2 }' "$OUT/margin/M.txt")
  cap=$(awk '$1 == "MARGIN" && $3 == "epochs" { print $4 }' "$OUT/margin/M.txt")
  [[ "$M" =~ ^[0-9]+$ ]] || { say "refused: $OUT/margin/M.txt names no margin"; return 1; }
  [[ "$cap" == "$FIT_EPOCHS" ]] ||
    { say "refused: the margin was found at ${cap:-an unnamed} epochs, not FIT_EPOCHS $FIT_EPOCHS"; return 1; }
}

# DEC-344 (4): round 1 at 184; round r at the pick of round r - 1; the fixed point
# or the stop after three rounds.
margin() {
  local info corpus rows round=1 m=$SHIPPED_MARGIN verdict dir
  enough_disk || return 1
  check_views || return 1
  [[ -f "$OUT/audit/pass" ]] || { say "refused: no audit pass; run audit first"; return 1; }
  tools "$OUT" || return 1
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  mkdir -p "$OUT/margin" || return 1
  if [[ -f "$OUT/margin/M.txt" ]]; then
    margin_of || return 1
    say "margin read before: $(cat "$OUT/margin/M.txt")"
    return 0
  fi
  : > "$OUT/margin/rounds.txt"
  cat "$OUT/audit/ties_P"{1,2,3,4}.txt > "$OUT/margin/ties.txt" || return 1
  while :; do
    dir="$OUT/margin/r${round}_m${m}_e$FIT_EPOCHS"
    mkdir -p "$dir" || return 1
    k_probe "$corpus" "$m" "$dir/k.k" || return 1
    if ! grep -qs '^best: ' "$dir/full.log"; then
      check_copy "$OUT/bin/tuner" || return 1
      # shellcheck disable=SC2046
      "$OUT/bin/tuner" --data "$corpus" --holdout "$HOLDOUT" --out "$dir/full.hpp" \
          --margin "$m" --only "$EVERY_GROUP" --tie "$OUT/margin/ties.txt" \
          --k "$(cat "$dir/k.k")" --vector "$dir/full.vector" $(fit_args "$FIT_EPOCHS") \
          > "$dir/full.log" 2>&1 ||
        { say "the margin fit at $m failed: $dir/full.log"; return 1; }
    fi
    [[ "$(cap_of_header "$dir/full.hpp")" == "$FIT_EPOCHS" ]] ||
      { say "refused: $dir/full.hpp did not run at $FIT_EPOCHS epochs"; return 1; }
    python3 "$helper" start-check "$dir/full.hpp" --tree "$root" --free "$EVERY_GROUP" || return 1
    "$OUT/bin/margin_census" --data "$corpus" --vector "$dir/full.vector" --also "$m" \
        --threads "$THREADS" > "$dir/census.txt" 2>&1 ||
      { tail -5 "$dir/census.txt"; say "margin_census failed"; return 1; }
    grep -E '^ +[0-9]+ |^pick|x > 0' "$dir/census.txt"
    verdict=$(python3 "$helper" margin-read "$dir/census.txt" --round "$round" --margin "$m" \
                --out "$OUT/margin/rounds.txt" | tail -n 1) || return 1
    say "$verdict"
    case "$verdict" in
      MARGIN\ *) echo "$verdict epochs $FIT_EPOCHS" > "$OUT/margin/M.txt"; return 0 ;;
      NEXT\ *) m=${verdict#NEXT }; round=$(( round + 1 )) ;;
      *) say "$verdict -- the coordinator's"; return 1 ;;
    esac
  done
}

p0() {
  local info corpus rows dir="$OUT/p0"
  check_views || return 1
  margin_of || return 1
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  mkdir -p "$dir" "$OUT/k" || return 1
  k_probe "$corpus" "$M" "$OUT/k/full.k" || return 1
  local m
  for m in "$SHIPPED_MARGIN" "$M"; do
    [[ -f "$dir/at_$m.log" ]] && continue
    "$OUT/bin/tuner" --data "$corpus" --holdout "$HOLDOUT" --out "$dir/at_$m.hpp" \
        --margin "$m" --k "$(cat "$OUT/k/full.k")" --epochs 0 --only "$EVERY_GROUP" \
        --threads "$THREADS" > "$dir/at_$m.log" 2>&1 || { say "the P0 run at $m failed"; return 1; }
  done
  python3 "$helper" p0-read "$dir/at_$SHIPPED_MARGIN.log" "$dir/at_$M.log" | tee "$dir/p0.txt"
}

p_groups="" p_fit="" p_ties="" p_pred="" p_tree="" p_skip="" p_cap="" p_dir="" p_escdir=""
p_predcap="" p_preddir="" p_escpreddir="" p_predgroups="" p_predties="" p_predtree=""
plan_of() {
  local text
  text=$(python3 "$helper" plan "$1" --out "$OUT") || { echo "$text"; return 1; }
  echo "$text"
  field() { sed -n "s/^$1 //p" <<< "$text"; }
  p_skip=$(field skip); p_groups=$(field groups); p_fit=$(field fit)
  p_ties=$(field ties); p_pred=$(field pred); p_tree=$(field tree)
  p_cap=$(field cap); p_dir=$(field dir); p_escdir=$(field escdir)
  p_predcap=$(field predcap); p_preddir=$(field preddir); p_escpreddir=$(field escpreddir)
  p_predgroups=$(field predgroups); p_predties=$(field predties); p_predtree=$(field predtree)
}

ties_for() {
  local files="$1" out="$2" f
  : > "$out.tmp" || return 1
  for f in $files; do
    [[ -f "$OUT/audit/$f" ]] || { say "no $OUT/audit/$f; run audit first"; return 1; }
    cat "$OUT/audit/$f" >> "$out.tmp" || return 1
  done
  mv "$out.tmp" "$out"
}

samples() {
  local info corpus rows quarter s dir="$OUT/samples"
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  mkdir -p "$dir" "$OUT/k" || return 1
  quarter=$(( rows / 4 ))
  for s in $SEEDS; do
    if [[ ! -f "$dir/q$s.done" ]]; then
      rm -f "$dir/q$s.tsv"
      python3 "$split" sample --in "$corpus" \
          --validation "${VIEWS[0]}" --validation "${VIEWS[1]}" \
          --validation "${VIEWS[2]}" --rows "$quarter" --per-game 0 \
          --seed "$s" --out "$dir/q$s.tsv" | tee -a "$dir/samples.log"
      (( PIPESTATUS[0] == 0 )) || { say "sample q$s refused"; return 1; }
      sha256sum "$dir/q$s.tsv" | cut -d' ' -f1 > "$dir/q$s.done"
    fi
    [[ "$(sha256sum "$dir/q$s.tsv" | cut -d' ' -f1)" == "$(cat "$dir/q$s.done")" ]] ||
      { say "$dir/q$s.tsv is not the sample drawn"; return 1; }
  done
  local x data
  for x in full q1 q2 q3 q4 q5; do
    data="$corpus"; [[ "$x" != full ]] && data="$dir/$x.tsv"
    k_probe "$data" "$M" "$OUT/k/$x.k" || return 1
    say "K $x $(cat "$OUT/k/$x.k") at margin $M"
  done
}

fit_one() {
  local tuner="$1" data="$2" log="$3" k="$4" ties="$5" groups="$6" five="$7" cap="$8"
  if [[ -f "$log" ]] && grep -q '^best: ' "$log"; then
    [[ "$(cap_of_header "${log%.log}.hpp")" == "$cap" ]] ||
      { say "refused: ${log%.log}.hpp was fitted at another cap than $cap"; return 1; }
    [[ "$(margin_of_header "${log%.log}.hpp")" == "$M" ]] ||
      { say "refused: ${log%.log}.hpp was fitted at another margin than $M"; return 1; }
    say "$(basename "$(dirname "$log")")/$(basename "$log" .log) fitted before"
    python3 "$helper" start-check "${log%.log}.hpp" --tree "$root" --free "$groups" $five || return 1
    return 0
  fi
  check_copy "$tuner" || return 1
  # shellcheck disable=SC2046
  "$tuner" --data "$data" --holdout "$HOLDOUT" --out "${log%.log}.hpp" --margin "$M" \
      --k "$k" --tie "$ties" --only "$groups" --vector "${log%.log}.vector" \
      $(fit_args "$cap") > "$log" 2>&1 ||
    { say "fit $log failed; see it"; return 1; }
  say "$(basename "$(dirname "$log")")/$(basename "$log" .log): $(grep '^best: ' "$log")"
  python3 "$helper" start-check "${log%.log}.hpp" --tree "$root" --free "$groups" $five || return 1
}

fit_set() {
  local dir="$1" groups="$2" tiefiles="$3" tree="$4" cap="$5" info corpus rows x data tuner five=""
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  tuner=$(tuner_for "$tree") || { echo "$tuner"; return 1; }
  [[ "$tree" == *p7* ]] && five="--allow-five"
  mkdir -p "$dir" || return 1
  ties_for "$tiefiles" "$dir/ties.new" || return 1
  if [[ -f "$dir/ties.txt" ]]; then
    cmp -s "$dir/ties.new" "$dir/ties.txt" || { say "refused: $dir was fitted with other ties"; return 1; }
    rm -f "$dir/ties.new"
  else
    mv "$dir/ties.new" "$dir/ties.txt" || return 1
  fi
  echo "$groups" > "$dir/groups.txt"
  echo "$tree" > "$dir/tree.txt"
  echo "$cap" > "$dir/cap.txt"
  probe_tuner "$tuner" "$corpus" "$dir" "$five" || return 1
  for x in full q1 q2 q3 q4 q5; do
    data="$corpus"; [[ "$x" != full ]] && data="$OUT/samples/$x.tsv"
    fit_one "$tuner" "$data" "$dir/$x.log" "$(cat "$OUT/k/$x.k")" "$dir/ties.txt" "$groups" "$five" "$cap" || return 1
  done
  # The curve's x > 0 share at a group fit's end, by the tree's own census: a
  # reading and no rule (DEC-345), every group fit starting at the seed, where
  # every king's x is the constant and positive.
  if [[ "$groups" == *ks_curve* && ! -f "$dir/census_end.txt" ]]; then
    local census
    census=$(census_for "$tree") || return 1
    check_copy "$census" || return 1
    "$census" --data "$corpus" --vector "$dir/full.vector" --also "$M" \
        --threads "$THREADS" > "$dir/census_end.txt" 2>&1 || { say "the end census failed"; return 1; }
  fi
}

audit() {
  enough_disk || return 1
  check_views || return 1
  audit_once "$OUT"
}

screen() {
  local part="$1" stamp
  python3 "$helper" landing-check --tree "$root" || return 1
  enough_disk || return 1
  check_views || return 1
  margin_of || return 1
  plan_of "$part" || return 1
  if [[ -n "$p_skip" ]]; then read_part "$part"; return; fi
  [[ -f "$OUT/audit/pass" ]] || { say "refused: no audit pass; run audit first"; return 1; }
  tools "$OUT" || return 1
  samples || return 1
  mkdir -p "$OUT/fits" || return 1
  stamp=$(engine_stamp) || return 1
  {
    echo "S122 screen $part"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    echo "margin    $M"
    echo "epochs    $FIT_EPOCHS"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line tuner "$OUT/bin/tuner"
    binary_line helper "$helper"
    binary_line split "$split"
    echo "python    $(python3 --version 2>&1)"
    echo "plan      groups $p_groups fit $p_fit tree $p_tree ties $p_ties cap $p_cap pred ${p_pred:-(start)} ${p_predcap:+at $p_predcap}"
  } >> "$OUT/fits/screen.log"
  if [[ -n "$p_pred" ]]; then
    local x
    for x in full q1 q2 q3 q4 q5; do
      grep -qs '^best: ' "$p_preddir/$x.log" ||
        { say "refused: the predecessor $p_pred has no $x fit in $p_preddir"; return 1; }
    done
  fi
  fit_set "$p_dir" "$p_groups" "$p_ties" "$p_tree" "$p_cap" | tee -a "$OUT/fits/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  echo "finished  $part at $p_cap $(date -Iseconds)" >> "$OUT/fits/screen.log"
  read_part "$part" || return 1
  grep -q '^PART-OUTCOME escalate$' "$OUT/read/$part.txt" || return 0
  # DEC-345: both sides of the pair again at 80000, and the decision on that pair.
  echo "escalate  $part to $ESCALATE_EPOCHS $(date -Iseconds)" >> "$OUT/fits/screen.log"
  if [[ -n "$p_pred" ]]; then
    fit_set "$p_escpreddir" "$p_predgroups" "$p_predties" "$p_predtree" "$ESCALATE_EPOCHS" |
      tee -a "$OUT/fits/screen.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  fi
  fit_set "$p_escdir" "$p_groups" "$p_ties" "$p_tree" "$ESCALATE_EPOCHS" | tee -a "$OUT/fits/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  echo "finished  $part at $ESCALATE_EPOCHS $(date -Iseconds)" >> "$OUT/fits/screen.log"
  read_part "$part" --cap "$ESCALATE_EPOCHS"
}

read_part() {
  local part="$1"
  shift
  python3 "$helper" read "$part" --out "$OUT" --tree "$root" "$@"
}

all() {
  local part
  python3 "$helper" landing-check --tree "$root" || return 1
  locate || return 1
  audit || { say "S122-AUDIT-FAILED"; return 1; }
  margin || { say "S122-MARGIN-FAILED"; return 1; }
  p0 || { say "S122-P0-FAILED"; return 1; }
  for part in P1 P2 P3 P4 P5 P6 P7; do
    say "== $part"
    if [[ -f "$OUT/read/$part.txt" ]] &&
       grep -qE '^PART-OUTCOME (kept|dropped|skipped)$' "$OUT/read/$part.txt"; then
      say "$part read before: $(grep '^PART-OUTCOME' "$OUT/read/$part.txt")"
      continue
    fi
    screen "$part" || { say "S122-SCREEN-FAILED ($part)"; return 1; }
  done
  grep -E '^(survivors|candidate|bisection|CANDIDATE)' "$OUT/read/P7.txt"
}

# DEC-345 (5): the sample side (b) reads. The corpus's own frozen datagen, its
# sha256 the corpus log's, `--leaf qs --games 12000 --nodes 100000 --threads 12
# --seed 2026260203 --quiet-limit 30000`, split at |column 3| < 1000 into the
# inside file exactly as S122_census.sh's stage_sample split the first sample.
# Refused unless the corpus log ends S122-CORPUS-DONE, so it never contends with
# the corpus, and refused if [seed, seed + games) meets any run's of the corpus's
# pool, each read from its run's command line: a single run's log names it on its
# command line, and a merged pool's (S122_corpus.sh merge, DEC-296, its log's first
# line CORPUS_MERGED_HEADER) holds each of its two runs' logs verbatim, indented four
# spaces, each with its command line and its datagen's sha256, which must be the
# frozen datagen's for every run. datagen seeds worker w from seed + w (tools/datagen.cpp,
# play_games()), so a run's streams are [seed, seed + threads): the games range is
# the coordinator's rule and the stricter of the two, and both are checked. Its last
# act is sample-prep's guard against the prepared corpus and the fixed set, so a
# sample the guard refuses is known before the screen starts and not after it.
side_sample() {
  local log="$CORPUS_POOL/generate.log" command cseed cgames cthreads want rc corpus
  local dg="$CORPUS_POOL/datagen" dir="$SAMPLE_DIR" commands shas sha expect run=0 ranges=""
  check_views || return 1
  corpus=$(record_field corpus)
  [[ -n "$corpus" && "$(sha256sum "$corpus" | cut -d' ' -f1)" == "$(record_field sha256)" ]] ||
    { say "refused: $PREPARED names no corpus whose sha256 it records"; return 1; }
  [[ -f "$log" ]] || { say "refused: no $log"; return 1; }
  [[ "$(tail -n 1 "$log")" == S122-CORPUS-DONE ]] ||
    { say "refused: $log does not end S122-CORPUS-DONE: the corpus still holds the machine, or failed"; return 1; }
  # The runs the pool is made of: one a single run's log (its first command line,
  # as before the merge), two a merged pool's, from their verbatim logs.
  if [[ "$(head -n 1 "$log")" == "$CORPUS_MERGED_HEADER" ]]; then
    commands=$(sed -n 's/^    command  *//p' "$log")
    shas=$(sed -n 's/^    datagen  *sha256 \([0-9a-f]\{64\}\) .*/\1/p' "$log")
    expect=2
  else
    commands=$(sed -n 's/^command *//p' "$log" | head -n 1)
    shas=$(sed -n 's/^datagen *sha256 \([0-9a-f]\{64\}\) .*/\1/p' "$log" | head -n 1)
    expect=1
  fi
  [[ "$(grep -c . <<< "$commands")" == "$expect" ]] ||
    { say "refused: $log names $(grep -c . <<< "$commands") runs' command lines, not $expect"; return 1; }
  [[ "$(grep -c . <<< "$shas")" == "$expect" ]] ||
    { say "refused: $log records $(grep -c . <<< "$shas") runs' datagen sha256, not $expect"; return 1; }
  while read -r command; do
    run=$(( run + 1 ))
    command=" $command "
    cseed=$(sed -n 's/.* --seed \([0-9][0-9]*\) .*/\1/p' <<< "$command")
    cgames=$(sed -n 's/.* --games \([0-9][0-9]*\) .*/\1/p' <<< "$command")
    cthreads=$(sed -n 's/.* --threads \([0-9][0-9]*\) .*/\1/p' <<< "$command")
    [[ -n "$cseed" && -n "$cgames" && -n "$cthreads" ]] ||
      { say "refused: $log's command line of run $run names no --seed, --games and --threads"; return 1; }
    if (( SAMPLE_SEED < cseed + cgames && cseed < SAMPLE_SEED + SAMPLE_GAMES )); then
      say "refused: the sample's seeds [$SAMPLE_SEED, $(( SAMPLE_SEED + SAMPLE_GAMES ))) meet run $run's of the corpus, [$cseed, $(( cseed + cgames )))"
      return 1
    fi
    if (( SAMPLE_SEED < cseed + cthreads && cseed < SAMPLE_SEED + THREADS )); then
      say "refused: the sample's worker seeds [$SAMPLE_SEED, $(( SAMPLE_SEED + THREADS ))) meet run $run's of the corpus, [$cseed, $(( cseed + cthreads )))"
      return 1
    fi
    ranges+="corpus    seeds [$cseed, $(( cseed + cgames ))), $cthreads threads, from $log"$'\n'
  done <<< "$commands"
  want=$(head -n 1 <<< "$shas")
  while read -r sha; do
    [[ "$sha" == "$want" ]] || { say "refused: $log's runs record two datagen sha256, $want and $sha"; return 1; }
  done <<< "$shas"
  [[ -x "$dg" && "$(sha256sum "$dg" | cut -d' ' -f1)" == "$want" ]] ||
    { say "refused: $dg is not the datagen $log records, $want"; return 1; }
  mkdir -p "$dir" || return 1
  if [[ -f "$dir/qs_inside.tsv.done" ]]; then
    [[ "$(sha256sum "$dir/qs_inside.tsv" | cut -d' ' -f1)" == "$(cat "$dir/qs_inside.tsv.done")" ]] ||
      { say "refused: $dir/qs_inside.tsv is not the file the sample stage wrote"; return 1; }
    say "sampled before"
    cat "$dir/sample.txt"
  else
    sample_run "$dir" "$dg" "$want" "$ranges" || return 1
  fi
  if ! grep -qs '^guard: pass' "$dir/guard.txt"; then
    python3 "$helper" sample-prep "$dir/qs_inside.tsv" --exclude "$corpus" --exclude "${VIEWS[0]}" \
        --exclude "${VIEWS[1]}" --exclude "${VIEWS[2]}" --out "$dir/guard.tsv" 2>&1 | tee "$dir/guard.txt"
    (( PIPESTATUS[0] == 0 )) || { say "refused by sample-prep's guard: the coordinator's, before the screen"; return 1; }
    rm -f "$dir/guard.tsv"
    echo "guard: pass against $corpus and the fixed set's three views" | tee -a "$dir/guard.txt"
  fi
}

# RANGES: one `corpus    seeds [a, b), T threads, from LOG` line a run of the corpus.
sample_run() {
  local dir="$1" dg="$2" want="$3" ranges="$4" rc
  if [[ -e "$dir/generate.log" ]]; then
    grep -q '^S122-SAMPLE-RUN-DONE$' "$dir/generate.log" ||
      { say "refused: $dir/generate.log is a run that did not finish; the coordinator's, never resumed"; return 1; }
  else
    {
      echo "S122 sample, DEC-345 (5)"
      echo "started   $(date -Is)"
      echo "datagen   sha256 $want  $dg"
      printf '%s' "$ranges"
      echo "command   datagen --out pool.tsv --leaf qs --games $SAMPLE_GAMES --nodes 100000 --threads $THREADS --seed $SAMPLE_SEED --quiet-limit 30000"
      ( cd "$dir" && "$dg" --out pool.tsv --leaf qs --games "$SAMPLE_GAMES" --nodes 100000 \
          --threads "$THREADS" --seed "$SAMPLE_SEED" --quiet-limit 30000 )
      rc=$?
      echo "ended     $(date -Is), exit $rc"
      (( rc == 0 )) && echo "S122-SAMPLE-RUN-DONE"
    } > "$dir/generate.log" 2>&1
    grep -q '^S122-SAMPLE-RUN-DONE$' "$dir/generate.log" ||
      { tail -5 "$dir/generate.log"; say "datagen failed: $dir/generate.log"; return 1; }
  fi
  rm -f "$dir/qs_inside.tsv.tmp" "$dir/qs_past.tsv.tmp"
  touch "$dir/qs_inside.tsv.tmp" "$dir/qs_past.tsv.tmp" || return 1
  awk -F'\t' -v d="$dir" '
    { v = $3; if (v < 0) v = -v; print > (d "/qs_" ((v < 1000) ? "inside" : "past") ".tsv.tmp") }' \
      "$dir/pool.tsv" || { say "the split failed"; return 1; }
  mv "$dir/qs_inside.tsv.tmp" "$dir/qs_inside.tsv" && mv "$dir/qs_past.tsv.tmp" "$dir/qs_past.tsv" || return 1
  {
    echo "S122 sample $(date -Iseconds): $SAMPLE_GAMES games, seed $SAMPLE_SEED, datagen sha256 $want"
    echo "pool    $(wc -l < "$dir/pool.tsv") rows, $(awk -F'\t' '{ seen[$5] = 1 } END { n = 0; for (g in seen) n++; print n }' "$dir/pool.tsv") games, sha256 $(sha256sum "$dir/pool.tsv" | cut -d' ' -f1)"
    echo "inside  $(wc -l < "$dir/qs_inside.tsv") rows under |column 3| 1000, sha256 $(sha256sum "$dir/qs_inside.tsv" | cut -d' ' -f1)"
    echo "past    $(wc -l < "$dir/qs_past.tsv") rows"
  } | tee "$dir/sample.txt"
  sha256sum "$dir/qs_inside.tsv" | cut -d' ' -f1 > "$dir/qs_inside.tsv.done"
}

# DEC-344 (1)'s two side readings, after the screen.
side() {
  local line name groups tree cdir info corpus rows dir="$OUT/side"
  check_views || return 1
  margin_of || return 1
  tools "$OUT" || return 1
  line=$(grep '^CANDIDATE ' "$OUT/read/P7.txt" 2>/dev/null) ||
    { say "refused: $OUT/read/P7.txt names no candidate; read P7 first"; return 1; }
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  mkdir -p "$dir" || return 1

  # (b) the sample, first: it needs no fit of its own.
  [[ -f "$SAMPLE" ]] || { say "no sample at $SAMPLE"; return 1; }
  # Every games file below is the prepared sample's; another SAMPLE is another OUT.
  local source
  source="$(sha256sum "$SAMPLE" | cut -d' ' -f1)  $SAMPLE"
  if [[ -f "$dir/sample.tsv" ]]; then
    [[ "$(cat "$dir/sample_source.txt" 2>/dev/null)" == "$source" ]] ||
      { say "refused: $dir/sample.tsv is not prepared from $SAMPLE as it is now"; return 1; }
  else
    python3 "$helper" sample-prep "$SAMPLE" --exclude "$corpus" --exclude "${VIEWS[0]}" \
        --exclude "${VIEWS[1]}" --exclude "${VIEWS[2]}" --out "$dir/sample.tsv" | tee "$dir/sample_prep.txt"
    (( PIPESTATUS[0] == 0 )) || return 1
    echo "$source" > "$dir/sample_source.txt"
  fi
  if [[ ! -s "$dir/sample.k" ]]; then
    "$OUT/bin/tuner" --data "$dir/sample.tsv" --out "$dir/sample_k.hpp" --margin "$M" \
        --epochs 0 --only "$EVERY_GROUP" --threads "$THREADS" > "$dir/sample_k.log" 2>&1 ||
      { say "the sample's K probe failed"; return 1; }
    sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$dir/sample_k.log" > "$dir/sample.k"
  fi
  k_check "$dir/sample.k" || return 1
  local k; k=$(cat "$dir/sample.k")

  # One census of the sample a vector, by the census of the tree the vector was
  # fitted on (P6's counts are its own build's), the games written for pairing.
  games_for() {  # DIR -> its games file, written once; every message to stderr
    local fdir="$1" ftree census out built
    out="$dir/games_$(basename "$fdir").txt"
    if [[ ! -f "$out" ]]; then
      ftree=$(cat "$fdir/tree.txt" 2>/dev/null) || { say "refused: $fdir has no tree.txt" >&2; return 1; }
      built=$(tuner_for "$ftree") || { echo "$built" >&2; return 1; }
      census=$(census_for "$ftree") || return 1
      check_copy "$census" >&2 || return 1
      "$census" --data "$dir/sample.tsv" --vector "$fdir/full.vector" --k "$k" \
          --loss-margin "$M" --games "$out" --threads "$THREADS" \
          > "$dir/census_$(basename "$fdir").txt" 2>&1 ||
        { say "the sample census of $fdir failed" >&2; return 1; }
      echo "$(basename "$fdir") tree $ftree census $census sha256 $(cat "$census.sha256")" >> "$dir/censuses.txt"
    fi
    echo "$out"
  }

  # P0: the shipped constants at 184 against them at M, the landing's census.
  local m
  for m in "$SHIPPED_MARGIN" "$M"; do
    [[ -f "$dir/games_P0_m$m.txt" ]] ||
      "$OUT/bin/margin_census" --data "$dir/sample.tsv" --k "$k" --loss-margin "$m" \
          --games "$dir/games_P0_m$m.txt" --threads "$THREADS" > "$dir/census_P0_m$m.txt" 2>&1 ||
        { say "the sample census at the shipped constants failed"; return 1; }
  done
  local d0
  d0=$(sed -n 's/.*d0 = \([-+0-9.]*\) .*/\1/p' "$OUT/p0/p0.txt" | head -n 1)
  [[ -n "$d0" ]] || { say "refused: no P0 reading in $OUT/p0/p0.txt; run p0 first"; return 1; }
  : > "$dir/side_b.txt"
  python3 "$helper" side-read "$dir/games_P0_m$SHIPPED_MARGIN.txt" "$dir/games_P0_m$M.txt" \
      --label "P0 (the shipped constants, $SHIPPED_MARGIN against $M)" --full-d "$d0" |
    tee -a "$dir/side_b.txt"
  (( PIPESTATUS[0] == 0 )) || return 1

  local part decided fdir pdir full_d fgames pgames
  for part in P1 P2 P3 P4 P5 P6 P7; do
    grep -qs '^PART-OUTCOME skipped$' "$OUT/read/$part.txt" && continue
    decided=$(grep '^DECIDED ' "$OUT/read/$part.txt" 2>/dev/null) ||
      { say "refused: $OUT/read/$part.txt is not a decided reading"; return 1; }
    fdir=$(awk '{print $3}' <<< "$decided")
    pdir=$(awk '{print $5}' <<< "$decided")
    full_d=$(awk '{print $7}' <<< "$decided")
    fgames=$(games_for "$fdir") || return 1
    if [[ "$pdir" == start ]]; then
      pgames="$dir/games_P0_m$M.txt"
    else
      pgames=$(games_for "$pdir") || return 1
    fi
    python3 "$helper" side-read "$pgames" "$fgames" \
        --label "$part ($(basename "$fdir") against $( [[ "$pdir" == start ]] && echo P0 || basename "$pdir"))" \
        --full-d "$full_d" | tee -a "$dir/side_b.txt"
    (( PIPESTATUS[0] == 0 )) || return 1
  done

  # (a) the candidate's free set on S083's corpus, read on the new fixed set.
  name=$(awk '{print $2}' <<< "$line")
  if [[ "$name" != none ]]; then
    groups=$(sed -n 's/.* groups \([^ ]*\) tree.*/\1/p' <<< "$line")
    tree=$(sed -n 's/.* tree \([^ ]*\) margin.*/\1/p' <<< "$line")
    cdir=$(sed -n 's/.* dir \([^ ]*\)$/\1/p' <<< "$line")
    local s083 tuner capacity five="" adir="$dir/s083_e$FIT_EPOCHS"
    [[ "$tree" == *p7* ]] && five="--allow-five"
    s083=$(sed -n 's/^corpus //p' "$S083_RECORD")
    [[ "$(sha256sum "$s083" | cut -d' ' -f1)" == "$S083_SHA" ]] ||
      { say "refused: $s083 is not S083's corpus, $S083_SHA"; return 1; }
    if [[ ! -f "$dir/s083.tsv" ]]; then
      capacity=$(python3 "$capacity_helper" capacity --in "$s083" --validation "${VIEWS[0]}" \
                   --validation "${VIEWS[1]}" --validation "${VIEWS[2]}" --per-game 0 |
                 sed -n 's/^capacity: per-game 0, \([0-9]*\) rows.*/\1/p')
      [[ -n "$capacity" ]] || { say "S083's capacity was not read"; return 1; }
      python3 "$split" sample --in "$s083" --validation "${VIEWS[0]}" --validation "${VIEWS[1]}" \
          --validation "${VIEWS[2]}" --rows "$capacity" --per-game 0 --seed 1 \
          --out "$dir/s083.tsv" > "$dir/s083_sample.txt" 2>&1 || { say "S083's resample failed"; return 1; }
    fi
    tuner=$(tuner_for "$tree") || { echo "$tuner"; return 1; }
    k_probe "$dir/s083.tsv" "$M" "$dir/s083.k" || return 1
    mkdir -p "$adir" || return 1
    cat "$cdir/ties.txt" > "$adir/ties.txt" || return 1
    fit_one "$tuner" "$dir/s083.tsv" "$adir/full.log" "$(cat "$dir/s083.k")" "$adir/ties.txt" \
        "$groups" "$five" "$FIT_EPOCHS" || return 1
    {
      echo "side (a), deciding nothing: the candidate's free set on S083's corpus, read on the new fixed set at $M"
      echo "  S083   $(grep '^best: ' "$adir/full.log")"
      echo "  new    $(grep '^best: ' "$cdir/full.log")"
    } | tee "$dir/side_a.txt"
  fi
  if grep -q '^STOP' "$dir/side_b.txt"; then
    say "STOP: a side reading disagrees beyond its error; the coordinator's before any SPRT"
    return 1
  fi
}

part() {
  local name="$1" spec groups fit tiefiles tree wide base fdir info corpus rows f tuner five="" w=""
  local asked="" kf
  [[ "${2:-}" == --margin ]] && asked="${3:-}"
  enough_disk || return 1
  check_views || return 1
  margin_of || return 1
  # DEC-344 (6): the parts run at M, or refit at 184 when M alone's {-5, 0} says
  # so; no other margin. The part's directory, K and headers name the one it ran at.
  if [[ -n "$asked" ]]; then
    [[ "$asked" == "$M" || "$asked" == "$SHIPPED_MARGIN" ]] ||
      { say "refused: --margin $asked is neither the screen's M $M nor $SHIPPED_MARGIN"; return 1; }
    M="$asked"
  fi
  spec=$(python3 "$helper" part "$name" --out "$OUT" --margin "$M") || { echo "$spec"; return 1; }
  echo "$spec"
  groups=$(sed -n 's/^groups //p' <<< "$spec")
  fit=$(sed -n 's/^fit //p' <<< "$spec")
  tiefiles=$(sed -n 's/^ties //p' <<< "$spec")
  tree=$(sed -n 's/^tree //p' <<< "$spec")
  wide=$(sed -n 's/^wide //p' <<< "$spec")
  [[ "$groups" == "(none: a tree only)" ]] && groups="$EVERY_GROUP"
  [[ "$tree" == *p7* ]] && five="--allow-five"
  [[ "$wide" == yes ]] && w="--wide"
  base="$OUT/bisect/$name"
  fdir="$base/fits/$fit"
  mkdir -p "$fdir" "$base/k" || return 1
  # The tree beside the fit, so `apply TREE FIT` pastes it on the tree it was
  # fitted on and refuses any other (S122's fast check, finding 5).
  if [[ -f "$fdir/tree.txt" ]]; then
    [[ "$(cat "$fdir/tree.txt")" == "$tree" ]] ||
      { say "refused: $fdir was fitted on tree $(cat "$fdir/tree.txt"), not $tree"; return 1; }
  fi
  if [[ -f "$fdir/margin.txt" ]]; then
    [[ "$(cat "$fdir/margin.txt")" == "$M" ]] ||
      { say "refused: $fdir was fitted at margin $(cat "$fdir/margin.txt"), not $M"; return 1; }
  fi
  audit_once "$base" || return 1
  # The tools frozen at what has landed; an audit read before froze none this run.
  tools "$base" || return 1
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  tuner="$base/bin/tuner"
  [[ "$tree" != landing ]] && { tuner=$(tuner_for "$tree") || { echo "$tuner"; return 1; }; }
  kf="$base/k/full_m$M"
  if [[ ! -s "$kf.k" ]]; then
    "$tuner" --data "$corpus" --holdout "$HOLDOUT" --margin "$M" \
        --out "$kf.hpp" --epochs 0 --only "$groups" --threads "$THREADS" \
        > "$kf.log" 2>&1 || { say "the K probe failed"; return 1; }
    sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$kf.log" > "$kf.k"
  fi
  k_check "$kf.k" || return 1
  : > "$fdir/ties.new" || return 1
  for f in $tiefiles; do
    cat "$base/audit/$f" >> "$fdir/ties.new" || return 1
  done
  if [[ -f "$fdir/ties.txt" ]]; then
    cmp -s "$fdir/ties.new" "$fdir/ties.txt" || { say "refused: $fdir was fitted with other ties"; return 1; }
    rm -f "$fdir/ties.new"
  else
    mv "$fdir/ties.new" "$fdir/ties.txt" || return 1
  fi
  echo "$groups" > "$fdir/groups.txt"
  echo "$tree" > "$fdir/tree.txt"
  echo "$FIT_EPOCHS" > "$fdir/cap.txt"
  echo "$M" > "$fdir/margin.txt"
  probe_tuner "$tuner" "$corpus" "$fdir" "$five" || return 1
  fit_one "$tuner" "$corpus" "$fdir/full.log" "$(cat "$kf.k")" "$fdir/ties.txt" \
      "$groups" "$five" "$FIT_EPOCHS" || return 1
  [[ "$(margin_of_header "$fdir/full.hpp")" == "$M" ]] ||
    { say "refused: $fdir/full.hpp's header does not say margin $M"; return 1; }
  # A cap biases a fit towards its start, the seed; the part is pasted as fitted
  # and its SPRT reads it (DEC-345), so a stop on the cap is a reading here.
  grep -q '^no improvement in' "$fdir/full.log" ||
    say "part $name: the fit stopped on its $FIT_EPOCHS-epoch cap, a reading"
  python3 "$helper" headroom "$fdir/full.hpp" --tree "$root" $w || return 1
  say "part $name: $fdir/full.hpp, tree $tree, margin $M, sha256 $(sha256sum "$fdir/full.hpp" | cut -d' ' -f1)"
}

# The candidate into TREE: the variant's source change if its parts name one, the
# paste, the checks, the margin into src/search_params.hpp if it moved, the
# formatter, both builds, bench and both fast suites -- their reds recorded, never
# answered here -- then the clamp's reading and a fresh truncation_scan.
apply() {
  local tree="$1" line name fit dir info corpus rows ctree five="" kfile
  margin_of || return 1
  local want spec
  if [[ -n "${2:-}" ]]; then
    fit="$2"
    # A bisection part's fit, OUT/bisect/NAME/fits/part_NAME_mM_eCAP/full.hpp: the
    # tree the part names now, which the fit's own tree.txt must say, pasted at
    # the margin it was fitted at, which its margin.txt and header must both say.
    local pname pm
    pname=$(basename "$(dirname "$(dirname "$(dirname "$fit")")")")
    [[ "$(dirname "$(dirname "$(dirname "$(dirname "$fit")")")")" == "$OUT/bisect" ]] ||
      { say "refused: $fit is not a bisection part's fit under $OUT/bisect"; return 1; }
    pm=$(cat "$(dirname "$fit")/margin.txt" 2>/dev/null) ||
      { say "refused: $(dirname "$fit") has no margin.txt"; return 1; }
    [[ "$pm" == "$M" || "$pm" == "$SHIPPED_MARGIN" ]] ||
      { say "refused: $fit names margin $pm, neither M $M nor $SHIPPED_MARGIN"; return 1; }
    M="$pm"
    name="part_${pname}_m$M"
    kfile="$OUT/bisect/$pname/k/full_m$M.k"
    spec=$(python3 "$helper" part "$pname" --out "$OUT" --margin "$M") || { echo "$spec"; return 1; }
    want=$(sed -n 's/^tree //p' <<< "$spec")
    [[ "$(sed -n 's/^fit //p' <<< "$spec")" == "$(basename "$(dirname "$fit")")" ]] ||
      { say "refused: $fit is not the fit part $pname names at margin $M"; return 1; }
  else
    line=$(grep '^CANDIDATE ' "$OUT/read/P7.txt" 2>/dev/null) ||
      { say "refused: $OUT/read/P7.txt names no candidate"; return 1; }
    [[ "$line" == "CANDIDATE none" ]] && { say "refused: the screen chose no candidate"; return 1; }
    name=$(awk '{print $2}' <<< "$line")
    want=$(sed -n 's/.* tree \([^ ]*\) margin.*/\1/p' <<< "$line")
    fit="$(sed -n 's/.* dir \([^ ]*\)$/\1/p' <<< "$line")/full.hpp"
    kfile="$OUT/k/full.k"
  fi
  [[ -f "$fit" ]] || { say "refused: no $fit"; return 1; }
  [[ "$(margin_of_header "$fit")" == "$M" ]] ||
    { say "refused: $fit's header says margin $(margin_of_header "$fit"), and it is pasted at $M"; return 1; }
  [[ -s "$kfile" ]] || { say "refused: no K at margin $M, $kfile"; return 1; }
  # The tree the fit ran on, written beside it; none or another is refused,
  # never read as the landing (S122's fast check, finding 5).
  ctree=$(cat "$(dirname "$fit")/tree.txt" 2>/dev/null) ||
    { say "refused: $(dirname "$fit") has no tree.txt"; return 1; }
  [[ "$ctree" == "$want" ]] ||
    { say "refused: $fit was fitted on tree $ctree, and its reading names $want"; return 1; }
  git -C "$tree" rev-parse --is-inside-work-tree > /dev/null 2>&1 || { say "$tree is not a git tree"; return 1; }
  git -C "$tree" diff --quiet HEAD || { say "refused: $tree has changes already"; return 1; }
  [[ "$(git -C "$tree" rev-parse HEAD)" == "$(git -C "$root" rev-parse HEAD)" ]] ||
    { say "refused: $tree is not at HEAD, the tree the fit started from"; return 1; }
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/apply_$name"
  mkdir -p "$dir" || return 1
  say "S122 apply: $fit into $tree at $(git -C "$tree" rev-parse --short HEAD), tree $ctree, margin $M"
  sha256sum "$fit" | tee "$dir/fit.sha256"
  [[ "$ctree" == *p7* ]] && five="--allow-five"
  python3 "$helper" start-check "$fit" --tree "$tree" --free "$EVERY_GROUP" $five || return 1
  local w=""
  [[ "$ctree" == *p6* ]] && w="--wide"
  python3 "$helper" headroom "$fit" --tree "$tree" $w ||
    { say "refused before the paste: the pre-registered branch, the coordinator's"; return 1; }
  if [[ "$ctree" == *p6* ]]; then
    git -C "$tree" apply "$p6_patch" || { say "the P6 patch did not apply to $tree"; return 1; }
  fi
  python3 "$root/.tuning/apply_fit.py" "$fit" --root "$tree" | tee "$dir/apply_fit.txt"
  (( PIPESTATUS[0] == 0 )) || { say "apply_fit refused"; return 1; }
  python3 "$root/.tuning/verify_fit.py" "$fit" --root "$tree" | tee "$dir/verify_fit.txt"
  (( PIPESTATUS[0] == 0 )) || { say "verify_fit found the paste incomplete"; return 1; }
  python3 "$helper" only-family "$tree" $five | tee "$dir/only_family.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  # The tree's margin as it stands -- 184, or M if M alone landed -- moved to the
  # one the fit ran at when they differ.
  local here
  here=$(sed -n -E 's/.*X\(LAZY_EVAL_MARGIN, +"LazyEvalMargin", +([0-9]+),.*/\1/p' \
           "$tree/src/search_params.hpp" | head -n 1)
  [[ -n "$here" ]] || { say "no LAZY_EVAL_MARGIN row in $tree/src/search_params.hpp"; return 1; }
  if [[ "$M" != "$here" ]]; then
    sed -i -E "s/(X\(LAZY_EVAL_MARGIN, +\"LazyEvalMargin\", +)$here,/\1$M,/" \
        "$tree/src/search_params.hpp"
    grep -qE "X\(LAZY_EVAL_MARGIN, +\"LazyEvalMargin\", +$M," "$tree/src/search_params.hpp" ||
      { say "the margin did not move to $M in $tree/src/search_params.hpp"; return 1; }
    say "LAZY_EVAL_MARGIN $here -> $M in $tree/src/search_params.hpp"
  fi

  export CLANG_FORMAT_MAJOR=22  # this machine's override, DEC-146
  ( cd "$tree" && ./clang-format.sh ) > "$dir/format_apply.log" 2>&1 ||
    { cat "$dir/format_apply.log"; say "clang-format.sh failed in $tree"; return 1; }
  if [[ -z "$(ls -A "$tree/tests/doctest" 2>/dev/null)" ]]; then
    git -C "$tree" config submodule.tests/doctest.url "$root/tests/doctest" &&
      git -C "$tree" -c protocol.file.allow=always submodule update --init \
        > "$dir/submodule.log" 2>&1 || { say "the doctest submodule did not check out"; return 1; }
  fi
  (
    cd "$tree" || exit 1
    [[ -d build ]] || cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > /dev/null || exit 1
    [[ -d build-tune ]] || cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCHESSO_TUNE=ON > /dev/null || exit 1
    cmake --build build -j"$THREADS" > "$dir/build.log" 2>&1 || exit 2
    cmake --build build-tune -j"$THREADS" > "$dir/build_tune.log" 2>&1 || exit 3
  )
  case $? in
    0) ;;
    2) tail -20 "$dir/build.log"; say "the Release build failed (an assert fires here)"; return 1 ;;
    3) tail -20 "$dir/build_tune.log"; say "the tune build failed"; return 1 ;;
    *) say "configure failed in $tree"; return 1 ;;
  esac
  "$tree/build/src/chesso" bench > "$dir/bench.txt" 2>&1 || { say "bench failed"; return 1; }
  say "bench: $(tail -n 1 "$dir/bench.txt")"
  ( cd "$tree" && ctest --test-dir build -L fast --output-on-failure ) > "$dir/fast.log" 2>&1
  say "fast suite, build: $(grep -E 'tests passed|tests failed' "$dir/fast.log" | tail -n 1)"
  ( cd "$tree" && ctest --test-dir build-tune -L fast --output-on-failure ) > "$dir/fast_tune.log" 2>&1
  say "fast suite, build-tune: $(grep -E 'tests passed|tests failed' "$dir/fast_tune.log" | tail -n 1)"
  grep -E '^\s*[0-9]+ - .*\(Failed\)' "$dir/fast.log" "$dir/fast_tune.log" | sed 's/^/red: /'

  # The pasted tree's own census at its own margin, and a fresh truncation_scan
  # (DEC-344 (4), DEC-169), each a reading for phase 2's goldens.
  "$tree/build/tools/margin_census" --data "$corpus" --k "$(cat "$kfile")" \
      --threads "$THREADS" > "$dir/census.txt" 2>&1 ||
    { tail -5 "$dir/census.txt"; say "margin_census failed on the pasted tree"; return 1; }
  python3 "$helper" clamp "$dir/census.txt" --margin "$M" | tee "$dir/clamp.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  "$tree/build/tools/truncation_scan" --data "$corpus" --min 2.8 > "$dir/truncation_scan.txt" \
      2> "$dir/truncation_scan.summary" ||
    { cat "$dir/truncation_scan.summary"; say "truncation_scan refused: the identity check failed on the pasted tree"; return 1; }
  say "truncation_scan: $(cat "$dir/truncation_scan.summary")"
  if grep -q '^STOP: ' "$dir/clamp.txt"; then
    say "STOP: the clamp share passes its bound; phase 2 stops for the coordinator"
  fi
  say "the goldens the paste moved are phase 2's, each by its script; nothing here edited a test"
}

mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S122-FAILED"; exit 1; }
HOLDOUT=""
VIEWS=()
M=""

case "$stage" in
  locate)
    if locate; then echo "S122-LOCATE-DONE"; else echo "S122-LOCATE-FAILED"; exit 1; fi ;;
  audit)
    if [[ $# -eq 1 ]] && audit; then echo "S122-AUDIT-DONE"; else echo "S122-AUDIT-FAILED"; exit 1; fi ;;
  landing-check)
    if python3 "$helper" landing-check --tree "$root"; then echo "S122-LANDING-CHECK-DONE"; else echo "S122-LANDING-CHECK-FAILED"; exit 1; fi ;;
  margin)
    if margin; then echo "S122-MARGIN-DONE"; else echo "S122-MARGIN-FAILED"; exit 1; fi ;;
  p0)
    if p0; then echo "S122-P0-DONE"; else echo "S122-P0-FAILED"; exit 1; fi ;;
  screen)
    if [[ $# -eq 2 ]] && check_views && margin_of && screen "$2"; then echo "S122-SCREEN-DONE"; else echo "S122-SCREEN-FAILED"; exit 1; fi ;;
  read)
    if [[ $# -eq 2 || ( $# -eq 4 && "$3" == --cap ) ]] && read_part "${@:2}"; then echo "S122-READ-DONE"; else echo "S122-READ-FAILED"; exit 1; fi ;;
  all)
    if all; then echo "S122-ALL-DONE"; else echo "S122-ALL-FAILED"; exit 1; fi ;;
  side)
    if side; then echo "S122-SIDE-DONE"; else echo "S122-SIDE-FAILED"; exit 1; fi ;;
  sample)
    if [[ $# -eq 1 ]] && side_sample; then echo "S122-SAMPLE-DONE"; else echo "S122-SAMPLE-FAILED"; exit 1; fi ;;
  part)
    if [[ $# -eq 2 || ( $# -eq 4 && "$3" == --margin ) ]] && part "${@:2}"; then echo "S122-PART-DONE"; else echo "S122-PART-FAILED"; exit 1; fi ;;
  apply)
    if [[ $# -eq 2 || $# -eq 3 ]] && check_views && apply "$2" "${3:-}"; then echo "S122-APPLY-DONE"; else echo "S122-APPLY-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 locate | audit | landing-check | margin | p0 | screen PART | read PART [--cap 80000] | all | sample | side | part NAME [--margin 184] | apply TREE [FIT]" >&2
    exit 2 ;;
esac
