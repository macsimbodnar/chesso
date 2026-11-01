#!/usr/bin/env bash
# S101's screen: the threat family's seven parts, each screened in fixed order with
# the earlier survivors live, by held-out loss on S082's fixed set (DEC-259 (a),
# DEC-333 (5)): P1, a piece a pawn attacks; P2, a piece a lesser piece attacks; P3, a
# minor a minor attacks; P4, a piece attacked and undefended; P5, a pawn attacked and
# undefended; P6, a man only the king attacks, undefended; P7, a piece a safe pawn
# push would attack. The last survivor's full fit is the candidate S101's one family
# SPRT measures (adocs/data/S101_sprt.sh).
#
#   adocs/data/S101_screen.sh locate          # the corpus, its sha256
#   adocs/data/S101_screen.sh audit           # identity, support, the ties
#   adocs/data/S101_screen.sh landing-check   # HEAD's new arrays all zero
#   adocs/data/S101_screen.sh screen PART     # PART's six fits; resumable
#   adocs/data/S101_screen.sh read PART       # the rule over them
#   adocs/data/S101_screen.sh all             # locate, audit, P1 to P7 in order
#   adocs/data/S101_screen.sh part NAME       # a bisection part's fit, on H0
#   adocs/data/S101_screen.sh apply TREE [FIT] # the candidate (or a part) into TREE
#
# PART is P1 to P7, NAME A, B or C. Run from the main checkout with S101's phase 1
# committed (the family at zero, `No functional change`), so build/'s tuner and
# feature_audit carry the 1121-wide layout, the seven groups and the family's ties; a
# stage refuses a tuner that does not. No mask patch and no variant build: the seven
# groups coexist in src/ at zero weights, and a part is a free set, not a different
# count (DEC-333 (2)). Each stage prints S101-<STAGE>-DONE or S101-<STAGE>-FAILED as
# its last stdout line, the reasons above a FAILED, so each runs detached under a
# four-exit watcher (WATCHERS). OUT defaults to .tuning/s101 (DEC-235: a run's outputs
# stay with the run; the readings go to adocs/data/ and the step file).
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-11-01 in S101's phase 1, before any fit.
# The audit ran once, after these rules were written and before any fit (the step
# file's "The audit, run once at phase 1"); the screen's own `audit` stage runs it
# again and writes the ties. Nothing below the line changes once `screen P1` has
# started; a change is a dated section in the step file and a decision. DEC-333 is the
# coordinator's ruling it carries.
#
# THE AUDIT AT PHASE 1: tools/feature_audit `--free` the seven groups over the corpus,
# 2026-11-01 03:25:21 to 03:26:27 (66 s), this file last changed 03:25:20 and
# S101_screen.py 03:24:01: rank over the rationals exactly 1074 of 1121, nullity 47, no
# further dependency, no family column zero, 0 of 48 cell halves tied, the thinnest
# threat_king_eg[2], 12347 rows and 9149 games. AFTER THE AUDIT and before any fit this
# file was edited in this paragraph and in MACHINE HOURS' entries figure, and
# S101_screen.py gained `landing-check --commit` for S101_sprt.sh's guard; none of it
# moves the identity rule, the ties, the parts, the fits or the reading.
# 2026-11-01, also after the audit at 03:25 and before any fit, on the fast check's findings:
# fit_one runs start-check when a stage resumes over a log (THE TUNER IS HEAD'S says so
# now), and S101_screen.py read counts a missing full.hpp as THE READING's "a fit missing".
# They fix mechanism, so the rules hold on a resume; the screen's rules did not move, and
# feature_audit, audit-check and ties are untouched, so the audit's figures stand.
# ============================================================================
#
# THE CORPUS: S083's, the one S135's, S121's, S123's and S125's screens fitted on and
# the shipped constants come from. `locate` finds it by S135_screen.py's own `locate
# --source s083`, reused by argument, then holds it to CORPUS_SHA below: curve 0's top
# rung, every game of S082's training side at 46 rows a game, 6226944 rows from 137978
# games. The fixed validation set is S082's qs view, its three views' sha256 checked
# before every stage. Both are outcome-labelled.
#
# THE TRANSFER GAP, NAMED BEFORE ANY FIT (DEC-333 (8)): the screen cannot see it and the
# SPRT can. The corpus is quiescence leaves, so the side to move has no capture
# quiescence would still make. Counted on S083's corpus, FEN field only, and over
# `chesso bench`'s lazy evaluation calls (the step file's research pass, 4d3edf6), the
# share of threats whose attacker is on move, and how many there are a row or a call:
#
#   kind              corpus, on move, a row   search, on move, a call
#   lesser, 9 pairs   8.0 %, 0.114             54.2 %, 0.446
#   hanging N B R Q   8.6 %, 0.121             56.2 %, 0.578
#   hanging pawn      25.8 %, 0.236            50.9 %, 0.652
#   push              53.8 %, 0.377            51.2 %, 0.651
#
# So a fit reads almost only lesser and hanging threats against the side to move, and
# the engine applies the same weight both ways (one weight, DEC-333 (8)). A gain here
# says nothing about a threat by the side to move. Doubling the cells by side to move is
# the first lead on an H0, unbuilt; on-move cells are as thin as about 1250 rows.
#
# THE PARTS, from their descriptions (DEC-221; DEC-333 (2) to (4); src/evaluation.hpp
# states each set and cell). Fixed order: hanging after lesser so that their overlap
# (72 % of hanging queen rows, 55 % of rook, about 20 % of minor rows are also attacked
# by a lesser piece) is priced as an increment, the pawn apart because its 18 % of rows
# could carry weak piece cells past the margin, push last, being nearly disjoint (0.1 to
# 0.4 % of rows) with the costliest sets. A part frees its own new weights and the
# earlier survivors'; every other constant -- the tables, mobility, king safety, S123's
# and S125's families, the buckets, the flats, placement, tempo -- is frozen at what it
# ships:
#   P1  threat_pawn: 8. Enemy N, B, R, Q an own pawn attacks.
#   P2  threat_piece: 10. N>R, N>Q, B>R, B>Q, R>Q, per pair.
#   P3  threat_minor: 4. N>B, B>N. The two are one rank, so not "lesser"; B3 of the
#       2026-09-04 literature check and Ethereal's 11.03 title name it.
#   P4  hanging_piece: 8. Enemy N, B, R, Q attacked by anything but the king,
#       undefended by anything of its own, king and pawns included (CPW Hanging Piece,
#       Loose Piece).
#   P5  hanging_pawn: 2. The same for pawns, which CPW Loose Piece treats apart.
#   P6  threat_king: 8. Enemy P, N, B, R only the king attacks, undefended. A queen
#       cell cannot exist: beside the enemy king it gives check (0 of 6226944 rows).
#   P7  threat_push: 8. Enemy N, B, R, Q a safe push target would attack, not already
#       attacked by an own pawn. "Safe" is chesso's own word (DEC-333 (4)).
#   Every new weight starts the fit at zero (no seed: DEC-333 (5), DEC-084), so every
#   fit of every part starts at the landing's evaluation.
#
# THE IDENTITY CHECK, before any fit (S134's lesson). `audit` runs tools/feature_audit
# `--free threat_pawn,threat_piece,threat_minor,hanging_piece,hanging_pawn,threat_king,
# threat_push` over the corpus and passes only on S101_screen.py audit-check's rule:
#   - rank over the rationals exactly 1074 - Z of 1121, Z the mobility and family
#     columns (S123's, S125's and S101's) zero on every row: S125's 1026 and S101's 48,
#     no new null direction;
#   - the zero columns are those Z and the 32 dead pawn columns, no other;
#   - fifteen dependencies by construction, each holding on every row: S134's seven and
#     S121's eight mobility levels, the levels "held by the freeze" under this free set
#     (the tables and the mobility cells are frozen);
#   - no exact dependency beyond those, and the span line's "every null direction is
#     one Adam never moves, one the tuner holds, or a mobility level the run's frozen
#     columns hold". Anything else stops the screen for the coordinator before any fit.
#   One audit serves the seven parts: the counts do not change between parts, only the
#   free set.
#
# THE TIES (DEC-333 (5), DEC-331 (6), DEC-326 (5), DEC-084: every shipped entry a
# fitted value). From the same report's `support` lines for S101's family: a cell's
# half is supported when the rows that put a non-zero coefficient on it at a phase
# giving that half a taper weight come from at least TIE_GAMES = 1000 games -- S121's,
# S123's and S125's threshold and its argument unchanged (0.72 % of the corpus's 137978
# games; a choice, not a measurement). An unsupported half is tied to the nearest
# supported cell of the same array and half, the lower cell on a tie, one shared
# parameter (tools/tuner_groups.hpp parse_ties, `--tie`); an array and half with no
# supported cell -- hanging_pawn, one wide, among them -- stops the screen. The lists,
# one a part with each tied cell's rows and games, are written by `audit` before any
# fit, $OUT/audit/ties_P{1..7}.txt, and a fit reads the lists of the parts it frees,
# concatenated. A tied cell takes its anchor's value once the run's start figures are
# read at the shipped constants and before the first step.
#
# THE FIT, DEC-304's settings as the screens before it ran them: `tuner --data DATA
# --holdout .tuning/s082/pool.tsv.qs.validation --only GROUPS --tie TIES --k K --epochs
# 5000 --report 100 --patience 20 --lambda 0 --seed 1 --threads 12`, GROUPS the part's
# free set as an `--only` list. K is fitted once per sample -- the full corpus and each
# quarter -- by a zero-epoch run at the shipped constants and handed to every fit on
# that sample, so a paired difference compares two fitted ends on one scale. One frozen
# copy of build/'s tuner for the whole screen.
#
# THE NOISE: five samples of the corpus at a quarter of its rows, `corpus_split.py
# sample --in CORPUS --rows C/4 --per-game 0 --seed s`, s = 1 to 5, the three views
# named, drawn once and kept for every part, so each part's quarter fit and its
# predecessor's share their rows.
#
# THE READING, by S101_screen.py read, mechanically (DEC-326 (3), DEC-329 (10)):
#   d_x = best(pred, x) - best(part, x) for x the full corpus and each quarter; s the
#   standard deviation of the five quarter d; m = max(1.645 s, 2e-6); kept iff full
#   d > m. pred is the last survivor's fit, named by its parts' digits (`1`, `13`,
#   ...). With no survivor before the part, pred's evaluation is the part's own start,
#   so d is start - best, the same margin. Unreadable, nothing chosen: a fit missing or
#   refused, a log whose start: and best: lines disagree, s above 1e-3, or a full fit
#   past the packed sum's assert. Printed beside, deciding nothing: every fit's start,
#   best, the epoch of its best report and whether it stopped on patience or at the
#   5000-epoch cap; the full fit's family tables.
#
# THE MARGIN, AS DEC-315 STATES IT AND S135'S HEADER ARGUES IT, NOT RE-ARGUED: s prices
# training-corpus resampling alone; the fixed set's own noise and the best-report pick
# on it are common to a part and its predecessor and unpriced, accepted because the
# screen only removes a part and the one SPRT decides what ships. The floor of 2e-6 is
# two units of the tuner's sixth printed decimal.
#
# THE TUNER IS HEAD'S, CHECKED. `screen` and `all` first refuse a HEAD that is not the
# landing (S101_screen.py landing-check): S101's fourteen arrays not all zero. Every
# gain is read against the landing's evaluation, and a HEAD past it -- a candidate
# committed -- would move that baseline in silence. `part` is exempt: it fits over
# landed parts. The frozen tuner is then run for zero epochs on the corpus's first 2000
# rows and its header compared with HEAD's sources (S101_screen.py start-check): every
# array HEAD's. Every fit's header is checked the same way after it, and again whenever a
# stage resumes over it, the free groups' arrays excepted.
#
# THE HEADROOM. src/evaluation.cpp's assert on mobility's packed sum, threat_family_half()
# included (S101_screen.py headroom, each array by its reach). A full fit past it is not
# clipped and not pasted: the coordinator's (DEC-084).
#
# THE CANDIDATE: the last survivor's full-corpus fit itself, freeing every survivor
# (DEC-321's form: with cumulative parts no joint refit is owed). P7's reading names it,
# `CANDIDATE <fit> groups <list>`. No survivor means no candidate, no SPRT, each part's
# zero recorded with its figures, and the family stays at zero, in no expression -- a
# corpus finding (S126's rule).
#
# THE BISECTION, ON H0 OR NO VERDICT (DEC-259 (c), DEC-333 (7)): A = P1 to P3 | B = P4 to
# P6 | C = P7, survivors only, in that order; a part with no survivor does not exist.
# Each its own `{0, 5}` nElo SPRT at 8+0.08 in S101_sprt.sh's shape, each played over
# what has landed by then and refitted then: `part NAME` fits NAME's survivors' groups
# with their ties, from HEAD as it stands, the landed parts held in the sources. A
# part's H1 lands before the next is fitted. One family SPRT and at worst three part
# SPRTs: four.
#
# THE CLAMP, READ AT `apply` (DEC-333 (6), DEC-326 (9)): the family is stage two, summed
# into mobility's pair, under the clamp (DEC-333 (1)). On the pasted tree's own tools
# over the corpus: the share of rows on which the stage-two clamp binds (feature_audit's
# load line) and tools/eval_spread's mobility, threats and combined p99 and maximum. At
# the shipped constants the share reads 0.355 % (22083 of 6226944). If the candidate's
# share passes CLAMP_SHARE_BOUND = 1.0 %, `apply` prints STOP and phase 2 stops for the
# coordinator. LAZY_EVAL_MARGIN is S122's to re-derive (DEC-324), not this step's; S122
# inherits the readings. A flat hypothetical weight across all kinds passes the 1.0 %
# near 45 to 50 (the step file's research pass: a probe, no weight proposed).
#
# MACHINE HOURS, from S125's measured screen (adocs/data/S125_screen_reading.txt: eight
# parts, 48 fits with locate, audit, six K probes and five samples in 3 h 09 m, 00:35 to
# 03:44): 23.6 min a part, 7 of the 48 at the 5000-epoch cap. An epoch costs the rows'
# trace entries, which S101's family raises from 36.49 to 37.37 a row (+2.4 %, the
# trace lines of S125's audit and this step's, 227235623 and 232675289 entries over
# 6226944 rows): about 24.2 min a part, seven
# parts about 2 h 50 m typical, 5 h 40 m at the cap; watcher ceiling 12 h. Daytime under
# DEC-155: the expected run is under four hours. `locate` under a minute; `audit` about
# 1 min. `read` seconds. `part` one K probe and one full fit: about 17 min, ceiling 40
# min. `apply` about 8 min, with the clamp's two readings about 2 min more.
#
# DISK. The five quarter samples, about 124 MB each, 0.62 GB, kept until the screen is
# recorded, each reproducible from its seed and the corpus's sha256. A stage refuses to
# start under 4 GB free.
#
# OPEN FINDINGS (BUGS as scoped by DEC-171): S307 (a timer case's precondition, tests
# only) and S308 (nine false comments) are open filler while this screen runs, neither
# moving play. Neither lands during a screen: a commit mid-run voids it (DEC-333 (11);
# S307 may land between runs, S308 after this step's close).
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s101}"
S083_OUT="${S083_OUT:-$root/.tuning/s083}"
CORPUS_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
THREADS=12
FREE_GB_FLOOR=4
SEEDS="1 2 3 4 5"
EVERY_GROUP="threat_pawn,threat_piece,threat_minor,hanging_piece,hanging_pawn,threat_king,threat_push"

S082_OUT="${S082_OUT:-$root/.tuning/s082}"
HOLDOUT="$S082_OUT/pool.tsv.qs.validation"
VIEWS=("$S082_OUT/pool.tsv.qs.validation"
       "$S082_OUT/pool.tsv.root.validation"
       "$S082_OUT/pool.tsv.pv.validation")
VIEW_SHAS=(2b508067fb64693bb4b278a2455fb9d101eaa742975a4b53f4ff56e6fa16981e
           58bcbb315fd82bd332372d0b2f1df7d7370693568bd27fa9ed6f17b8e74652d6
           24c4f671131c4a6f3f945ef0f5bb0d8cf85969b41d2e846e745539ef2d53ffdd)
FIT="--epochs 5000 --report 100 --patience 20 --lambda 0 --seed 1
     --threads $THREADS"

split="$root/tools/corpus_split.py"
helper="$root/adocs/data/S101_screen.py"
s135_helper="$root/adocs/data/S135_screen.py"

say() { echo "$*"; }

binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

# The engine a stage ran: HEAD, and nothing uncommitted under src/ or tools/,
# because every check here reads HEAD's sources (S135_screen.py
# head_weights) and the tools must be the build of them.
engine_stamp() {
  local untracked
  untracked=$(git -C "$root" ls-files --others -- src tools |
                grep -v '/__pycache__/')
  if [[ -n "$untracked" ]]; then
    {
      echo "refused: untracked files under src/ or tools/, which no stamp names:"
      echo "$untracked"
    } >&2
    return 1
  fi
  git -C "$root" diff --quiet HEAD -- src tools || {
    echo "refused: src/ or tools/ differ from HEAD; the screen reads HEAD's constants and needs build/ built from them" >&2
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

# A frozen copy of a build binary, its sha256 recorded when it is copied and
# checked before every run. An existing copy is reused, never replaced.
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

# The tuner must be S101's: the family's groups and tie rows. The usage is
# captured before it is searched (S135_cost.sh carries_columns: pipefail and
# SIGPIPE).
check_tuner() {
  local usage
  usage=$("$1" --help 2>&1)
  [[ "$usage" == *--tie* && "$usage" == *threat_pawn* &&
     "$usage" == *threat_push* && "$usage" == *"(S101)"* ]] || {
    say "refused: $1 has no S101 groups or tie rows; build the tools from a tree with S101's phase 1 landed"
    return 1
  }
}

check_views() {
  local i
  for i in 0 1 2; do
    if [[ "$(sha256sum "${VIEWS[$i]}" | cut -d' ' -f1)" != "${VIEW_SHAS[$i]}" ]]; then
      say "the fixed validation set's ${VIEWS[$i]} is not the file S082 recorded"
      return 1
    fi
  done
  say "fixed set: the three views' sha256 are S082's"
}

# The corpus `locate` recorded, re-hashed: "PATH ROWS".
corpus_of() {
  local record="$OUT/corpus.txt" path sha rows
  [[ -f "$record" ]] || { say "no $record; run locate first"; return 1; }
  path=$(sed -n 's/^corpus //p' "$record")
  sha=$(sed -n 's/^sha256 //p' "$record")
  rows=$(sed -n 's/^rows //p' "$record")
  [[ -n "$path" && -n "$sha" && -n "$rows" ]] || { say "$record is incomplete"; return 1; }
  grep -qx "sha256 $CORPUS_SHA" "$record" ||
    { say "refused: $record names sha256 $sha, not $CORPUS_SHA, the corpus this screen pre-registers"; return 1; }
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
  rm -f "$OUT/corpus.txt.tmp"
  python3 "$s135_helper" locate --source s083 --s083-out "$S083_OUT" \
      --s082-out "$S082_OUT" --out "$OUT/corpus.txt.tmp" || return 1
  grep -qx "sha256 $CORPUS_SHA" "$OUT/corpus.txt.tmp" || {
    say "refused: the located corpus is not $CORPUS_SHA, the one S083's curve 0 recorded and this screen pre-registers"
    return 1
  }
  mv "$OUT/corpus.txt.tmp" "$OUT/corpus.txt"
}

# build/'s tuner and feature_audit, frozen under $1/bin with the HEAD they
# were built from; a directory frozen from another HEAD is refused, never
# rebuilt over.
tools() {
  local dir="$1" head had
  head=$(engine_stamp) || return 1
  mkdir -p "$dir/bin" || return 1
  if [[ -f "$dir/bin/stamp" ]]; then
    had=$(cat "$dir/bin/stamp")
    [[ "$had" == "$head" ]] ||
      { say "refused: $dir/bin was frozen at $had, not HEAD $head; take a fresh OUT"; return 1; }
    check_copy "$dir/bin/tuner" && check_copy "$dir/bin/feature_audit" || return 1
    return 0
  fi
  freeze_binary "$root/build/tools/tuner" "$dir/bin/tuner" > /dev/null || return 1
  freeze_binary "$root/build/tools/feature_audit" "$dir/bin/feature_audit" > /dev/null || return 1
  check_tuner "$dir/bin/tuner" || return 1
  echo "$head" > "$dir/bin/stamp"
  say "tools at $head: tuner $(cut -c1-16 "$dir/bin/tuner.sha256"), feature_audit $(cut -c1-16 "$dir/bin/feature_audit.sha256")"
}

# A zero-epoch run of the frozen tuner on the corpus's first 2000 rows, its
# header compared with HEAD's sources.
probe_tuner() {
  local copy="$1" corpus="$2" dir="$3"
  head -n 2000 "$corpus" > "$dir/probe.tsv" || return 1
  check_copy "$copy" || return 1
  "$copy" --data "$dir/probe.tsv" --out "$dir/probe.hpp" --epochs 0 \
      --only "$EVERY_GROUP" --threads "$THREADS" > "$dir/probe.log" 2>&1 ||
    { tail -5 "$dir/probe.log"; say "the probe run of $copy failed"; return 1; }
  python3 "$helper" start-check "$dir/probe.hpp" --tree "$root"
}

# The identity report and the ties, once, under $1.
audit_once() {
  local base="$1" info corpus rows dir
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$base/audit"
  if [[ -f "$dir/pass" ]] && grep -qs "corpus $CORPUS_SHA" "$dir/pass"; then
    say "audited before: $(cat "$dir/pass")"
    return 0
  fi
  mkdir -p "$dir" || return 1
  tools "$base" || return 1
  local bin="$base/bin"
  {
    echo "S101 audit, $(date -Iseconds)"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line audit "$bin/feature_audit"
    echo "head      $(cat "$bin/stamp")"
  } > "$dir/audit.log"
  check_copy "$bin/feature_audit" || return 1
  "$bin/feature_audit" --data "$corpus" --free "$EVERY_GROUP" > "$dir/report.txt" 2>&1 ||
    { tail -20 "$dir/report.txt"; say "feature_audit failed"; return 1; }
  grep -E '^identities: (rank|no exact|.*null dimensions|by construction.*S121|mobility level)' \
      "$dir/report.txt" | tee -a "$dir/audit.log"
  python3 "$helper" audit-check "$dir/report.txt" | tee -a "$dir/audit.log"
  (( PIPESTATUS[0] == 0 )) ||
    { say "identity: not the rule's -- the coordinator's, before any fit ($dir/report.txt)"; return 1; }
  python3 "$helper" ties "$dir/report.txt" --out "$dir" | tee -a "$dir/audit.log"
  (( PIPESTATUS[0] == 0 )) || { say "the ties refused"; return 1; }
  echo "pass $(date -Iseconds) corpus $CORPUS_SHA" > "$dir/pass"
}

# What PART frees and against what, from the readings before it
# (S101_screen.py plan), into the globals below.
p_groups="" p_fit="" p_ties="" p_pred=""
plan_of() {
  local text
  text=$(python3 "$helper" plan "$1" --out "$OUT") || { echo "$text"; return 1; }
  echo "$text"
  p_groups=$(sed -n 's/^groups //p' <<< "$text")
  p_fit=$(sed -n 's/^fit //p' <<< "$text")
  p_ties=$(sed -n 's/^ties //p' <<< "$text")
  p_pred=$(sed -n 's/^pred //p' <<< "$text")
}

# The tie list a fit reads: the named parts' lists concatenated, into $2.
ties_for() {
  local files="$1" out="$2" f
  : > "$out.tmp" || return 1
  for f in $files; do
    [[ -f "$OUT/audit/$f" ]] || { say "no $OUT/audit/$f; run audit first"; return 1; }
    cat "$OUT/audit/$f" >> "$out.tmp" || return 1
  done
  mv "$out.tmp" "$out"
}

# A K file as `--k` will read it: one positive decimal and nothing else.
# `tuner --k ""` and `--k junk` exit 0 -- atof() reads 0, and 0 means "fit K"
# -- so a probe that printed no K would hand a fit a K of its own in silence
# (S125's fast check, 2026-10-30).
k_check() {
  local value
  value=$(cat "$1" 2> /dev/null) || { say "refused: no K file $1"; return 1; }
  if [[ ! "$value" =~ ^[0-9]*\.?[0-9]+$ ]] ||
     ! awk -v k="$value" 'BEGIN { exit !(k > 0) }'; then
    say "refused: $1 holds '$value', not a positive K; remove it and re-run its probe"
    return 1
  fi
}

# The five quarter samples and the six K, drawn and fitted once.
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
  local x data tuner="$OUT/bin/tuner"
  for x in full q1 q2 q3 q4 q5; do
    if [[ -f "$OUT/k/$x.k" ]]; then
      k_check "$OUT/k/$x.k" || return 1
      continue
    fi
    data="$corpus"; [[ "$x" != full ]] && data="$dir/$x.tsv"
    check_copy "$tuner" || return 1
    "$tuner" --data "$data" --holdout "$HOLDOUT" --out "$OUT/k/$x.hpp" \
        --epochs 0 --only "$EVERY_GROUP" --threads "$THREADS" > "$OUT/k/$x.log" 2>&1 ||
      { tail -5 "$OUT/k/$x.log"; say "the K probe on $x failed"; return 1; }
    sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$OUT/k/$x.log" > "$OUT/k/$x.k"
    k_check "$OUT/k/$x.k" || { say "the K probe on $x printed no K"; return 1; }
    say "K $x $(cat "$OUT/k/$x.k")"
  done
}

# One fit; skipped when its log already holds a best: line, so a stage cut
# short resumes. The header is checked either way: a log with best: says only
# that the tuner got that far, since it prints best: before it writes the
# header, and a fit that failed this check left such a log behind.
fit_one() {
  local tuner="$1" data="$2" log="$3" k="$4" ties="$5" groups="$6"
  if [[ -f "$log" ]] && grep -q '^best: ' "$log"; then
    say "$(basename "$(dirname "$log")")/$(basename "$log" .log) fitted before"
    python3 "$helper" start-check "${log%.log}.hpp" --tree "$root" --free "$groups" || return 1
    return 0
  fi
  check_copy "$tuner" || return 1
  # shellcheck disable=SC2086
  "$tuner" --data "$data" --holdout "$HOLDOUT" --out "${log%.log}.hpp" \
      --k "$k" --tie "$ties" --only "$groups" $FIT > "$log" 2>&1 ||
    { say "fit $log failed; see it"; return 1; }
  say "$(basename "$(dirname "$log")")/$(basename "$log" .log): $(grep '^best: ' "$log")"
  python3 "$helper" start-check "${log%.log}.hpp" --tree "$root" --free "$groups" || return 1
}

# The six fits of one free set.
fit_set() {
  local name="$1" groups="$2" tiefiles="$3" info corpus rows x data
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  local dir="$OUT/fits/$name" tuner="$OUT/bin/tuner"
  mkdir -p "$dir" || return 1
  ties_for "$tiefiles" "$dir/ties.new" || return 1
  if [[ -f "$dir/ties.txt" ]]; then
    cmp -s "$dir/ties.new" "$dir/ties.txt" || { say "refused: $dir was fitted with other ties"; return 1; }
    rm -f "$dir/ties.new"
  else
    mv "$dir/ties.new" "$dir/ties.txt" || return 1
  fi
  echo "$groups" > "$dir/groups.txt"
  probe_tuner "$tuner" "$corpus" "$dir" || return 1
  for x in full q1 q2 q3 q4 q5; do
    data="$corpus"; [[ "$x" != full ]] && data="$OUT/samples/$x.tsv"
    fit_one "$tuner" "$data" "$dir/$x.log" "$(cat "$OUT/k/$x.k")" "$dir/ties.txt" "$groups" || return 1
  done
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
  plan_of "$part" || return 1
  [[ -f "$OUT/audit/pass" ]] || { say "refused: no audit pass; run audit first"; return 1; }
  tools "$OUT" || return 1
  samples || return 1
  mkdir -p "$OUT/fits" || return 1
  stamp=$(engine_stamp) || return 1
  {
    echo "S101 screen $part"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line tuner "$OUT/bin/tuner"
    binary_line helper "$helper"
    binary_line split "$split"
    echo "python    $(python3 --version 2>&1)"
    echo "plan      groups $p_groups fit $p_fit ties $p_ties pred ${p_pred:-(start)}"
  } >> "$OUT/fits/screen.log"
  if [[ -n "$p_pred" ]]; then
    # The predecessor is a part screened and kept before, its six fits
    # already on disk.
    local x
    for x in full q1 q2 q3 q4 q5; do
      grep -qs '^best: ' "$OUT/fits/$p_pred/$x.log" ||
        { say "refused: the predecessor $p_pred has no $x fit"; return 1; }
    done
  fi
  fit_set "$p_fit" "$p_groups" "$p_ties" | tee -a "$OUT/fits/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  echo "finished  $part $(date -Iseconds)" >> "$OUT/fits/screen.log"
  read_part "$part"
}

read_part() {
  python3 "$helper" read "$1" --out "$OUT" --tree "$root"
}

all() {
  local part
  python3 "$helper" landing-check --tree "$root" || return 1
  locate || return 1
  audit || { say "S101-AUDIT-FAILED"; return 1; }
  for part in P1 P2 P3 P4 P5 P6 P7; do
    say "== $part"
    if [[ -f "$OUT/read/$part.txt" ]] && grep -qE '^PART-OUTCOME (kept|dropped)$' "$OUT/read/$part.txt"; then
      say "$part read before: $(grep '^PART-OUTCOME' "$OUT/read/$part.txt")"
      continue
    fi
    screen "$part" || { say "S101-SCREEN-FAILED ($part)"; return 1; }
  done
  grep -E '^(survivors|candidate|bisection|CANDIDATE)' "$OUT/read/P7.txt"
}

# A bisection part on H0 or no verdict: its survivors' groups fitted over the
# tree as it stands then, the landed parts in its sources; one full fit, under
# $OUT/bisect/NAME, with its own audit (the tree has moved) and its own K. The
# survivors are the screen's, read from $OUT/read.
part() {
  local name="$1" spec groups fit tiefiles base info corpus rows f
  enough_disk || return 1
  check_views || return 1
  spec=$(python3 "$helper" part "$name" --out "$OUT") || { echo "$spec"; return 1; }
  echo "$spec"
  groups=$(sed -n 's/^groups //p' <<< "$spec")
  fit=$(sed -n 's/^fit //p' <<< "$spec")
  tiefiles=$(sed -n 's/^ties //p' <<< "$spec")
  base="$OUT/bisect/$name"
  mkdir -p "$base/fits/$fit" "$base/k" || return 1
  audit_once "$base" || return 1
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  if [[ ! -s "$base/k/full.k" ]]; then
    "$base/bin/tuner" --data "$corpus" --holdout "$HOLDOUT" \
        --out "$base/k/full.hpp" --epochs 0 --only "$groups" --threads "$THREADS" \
        > "$base/k/full.log" 2>&1 ||
      { say "the K probe failed"; return 1; }
    sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$base/k/full.log" > "$base/k/full.k"
  fi
  k_check "$base/k/full.k" || return 1
  : > "$base/fits/$fit/ties.txt" || return 1
  for f in $tiefiles; do
    cat "$base/audit/$f" >> "$base/fits/$fit/ties.txt" || return 1
  done
  probe_tuner "$base/bin/tuner" "$corpus" "$base/fits/$fit" || return 1
  fit_one "$base/bin/tuner" "$corpus" "$base/fits/$fit/full.log" \
      "$(cat "$base/k/full.k")" "$base/fits/$fit/ties.txt" "$groups" || return 1
  python3 "$helper" headroom "$base/fits/$fit/full.hpp" --tree "$root" || return 1
  say "part $name: $base/fits/$fit/full.hpp, sha256 $(sha256sum "$base/fits/$fit/full.hpp" | cut -d' ' -f1)"
}

# The candidate into TREE: the paste by .tuning/apply_fit.py, the check by
# .tuning/verify_fit.py and that only the family's arrays moved, the formatter, both
# builds, bench and both fast suites -- their reds recorded, never answered here --
# then the clamp's two readings on the pasted tree's own tools. TREE stays uncommitted;
# nothing here edits a test.
apply() {
  local tree="$1" line name fit dir info corpus rows
  if [[ -n "${2:-}" ]]; then
    fit="$2"
    name="part_$(basename "$(dirname "$(dirname "$(dirname "$fit")")")")"
  else
    line=$(grep '^CANDIDATE ' "$OUT/read/P7.txt" 2>/dev/null) ||
      { say "refused: $OUT/read/P7.txt names no candidate"; return 1; }
    [[ "$line" == "CANDIDATE none" ]] && { say "refused: the screen chose no candidate"; return 1; }
    name=$(awk '{print $2}' <<< "$line")
    fit="$OUT/fits/$name/full.hpp"
  fi
  [[ -f "$fit" ]] || { say "refused: no $fit"; return 1; }
  git -C "$tree" rev-parse --is-inside-work-tree > /dev/null 2>&1 || { say "$tree is not a git tree"; return 1; }
  git -C "$tree" diff --quiet HEAD || { say "refused: $tree has changes already"; return 1; }
  [[ "$(git -C "$tree" rev-parse HEAD)" == "$(git -C "$root" rev-parse HEAD)" ]] ||
    { say "refused: $tree is not at HEAD, the tree the fit started from"; return 1; }
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$OUT/apply_$name"
  mkdir -p "$dir" || return 1
  say "S101 apply: $fit into $tree at $(git -C "$tree" rev-parse --short HEAD)"
  sha256sum "$fit" | tee "$dir/fit.sha256"
  python3 "$helper" start-check "$fit" --tree "$tree" --free "$EVERY_GROUP" || return 1
  python3 "$helper" headroom "$fit" --tree "$tree" ||
    { say "refused before the paste: the pre-registered branch, the coordinator's"; return 1; }

  python3 "$root/.tuning/apply_fit.py" "$fit" --root "$tree" | tee "$dir/apply_fit.txt"
  (( PIPESTATUS[0] == 0 )) || { say "apply_fit refused"; return 1; }
  python3 "$root/.tuning/verify_fit.py" "$fit" --root "$tree" | tee "$dir/verify_fit.txt"
  (( PIPESTATUS[0] == 0 )) || { say "verify_fit found the paste incomplete"; return 1; }
  python3 "$helper" only-family "$tree" | tee "$dir/only_family.txt"
  (( PIPESTATUS[0] == 0 )) || return 1

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
    2) tail -20 "$dir/build.log"; say "the Release build failed (the packed sum's static_assert fires here)"; return 1 ;;
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

  "$tree/build/tools/feature_audit" --data "$corpus" --no-columns --no-identities \
      --no-occurrence --no-collinearity > "$dir/clamp_audit.txt" 2>&1 ||
    { tail -5 "$dir/clamp_audit.txt"; say "feature_audit failed on the pasted tree"; return 1; }
  "$tree/build/tools/eval_spread" --data "$corpus" > "$dir/spread.txt" 2>&1 ||
    { tail -5 "$dir/spread.txt"; say "eval_spread failed on the pasted tree"; return 1; }
  python3 "$helper" clamp "$dir/clamp_audit.txt" "$dir/spread.txt" | tee "$dir/clamp.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  if grep -q '^STOP: ' "$dir/clamp.txt"; then
    say "STOP: the clamp share passes its bound; phase 2 stops for the coordinator"
  fi
  say "the goldens the paste moved are the step's, each by its script; nothing here edited a test"
}

mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S101-FAILED"; exit 1; }

case "$stage" in
  locate)
    if locate; then echo "S101-LOCATE-DONE"; else echo "S101-LOCATE-FAILED"; exit 1; fi ;;
  audit)
    if [[ $# -eq 1 ]] && audit; then echo "S101-AUDIT-DONE"; else echo "S101-AUDIT-FAILED"; exit 1; fi ;;
  landing-check)
    if python3 "$helper" landing-check --tree "$root"; then echo "S101-LANDING-CHECK-DONE"; else echo "S101-LANDING-CHECK-FAILED"; exit 1; fi ;;
  screen)
    if [[ $# -eq 2 ]] && screen "$2"; then echo "S101-SCREEN-DONE"; else echo "S101-SCREEN-FAILED"; exit 1; fi ;;
  read)
    if [[ $# -eq 2 ]] && read_part "$2"; then echo "S101-READ-DONE"; else echo "S101-READ-FAILED"; exit 1; fi ;;
  all)
    if all; then echo "S101-ALL-DONE"; else echo "S101-ALL-FAILED"; exit 1; fi ;;
  part)
    if [[ $# -eq 2 ]] && part "$2"; then echo "S101-PART-DONE"; else echo "S101-PART-FAILED"; exit 1; fi ;;
  apply)
    if [[ $# -eq 2 || $# -eq 3 ]] && apply "$2" "${3:-}"; then echo "S101-APPLY-DONE"; else echo "S101-APPLY-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 locate | audit | landing-check | screen PART | read PART | all | part NAME | apply TREE [FIT]" >&2
    exit 2 ;;
esac
