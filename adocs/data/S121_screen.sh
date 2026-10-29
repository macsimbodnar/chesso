#!/usr/bin/env bash
# S121's screen: mobility as a fitted table per piece type (P0), then four
# parts, each a change to the count the table is read at -- enemy pawn
# attacks excluded (P1), the mobility area in place of "not an own piece" with
# own blocked pawns excluded (P2a), own low-rank pawns excluded as well (P2b),
# the pin rule (P3) -- screened in that fixed order with the earlier
# survivors live, by held-out loss on S082's fixed set (DEC-259 (a), DEC-326).
# The last survivor's full fit is the candidate S121's one SPRT measures
# (adocs/data/S121_sprt.sh).
#
#   adocs/data/S121_screen.sh locate                 # the corpus, its sha256
#   adocs/data/S121_screen.sh audit PART             # PART's tools, identity, ties
#   adocs/data/S121_screen.sh screen PART            # PART's fits; resumable
#   adocs/data/S121_screen.sh read PART              # the rule over them
#   adocs/data/S121_screen.sh all                    # locate, then each part in order
#   adocs/data/S121_screen.sh part NAME LANDED CURVE # a bisection part's fit, on H0
#   adocs/data/S121_screen.sh apply TREE [FIT]       # the candidate (or a part) into TREE
#
# PART is P0, P1, P2a, P2b or P3. Run from the main checkout with S121's phase 1
# landed (the table at identity, `No functional change`), so build/'s tuner
# and feature_audit carry the 955-wide layout and `--tie`; a stage refuses a
# tuner that does not. The parts are never in src/: a part's tools are built
# from a throwaway worktree of HEAD with adocs/data/S121_parts.patch applied
# and `-DS121_PARTS=<mask>`, under $OUT/variants. Each stage prints
# S121-<STAGE>-DONE or S121-<STAGE>-FAILED as its last stdout line, the
# reasons above a FAILED, so each runs detached under a four-exit watcher
# (WATCHERS). OUT defaults to .tuning/s121 (DEC-235: a run's outputs stay
# with the run; the readings go to adocs/data/ and the step file).
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-28 in S121's phase 1a, blind (DEC-260:
# S136's SPRT held the machine), before any of it ran. Nothing below the line
# changes once `screen P0` has started; a change is a dated section in the
# step file and a decision. DEC-326 is the coordinator's ruling it carries.
# ============================================================================
#
# THE CORPUS: S083's, the one S135's and S136's screens fitted on and the
# shipped constants come from (S083 read H1, DEC-313). `locate` finds it by
# S135_screen.py's own `locate --source s083`, reused by argument, then holds
# it to CORPUS_SHA below: curve 0's top rung, every game of S082's training
# side at 46 rows a game, 6226944 rows from 137978 games. The fixed
# validation set is S082's qs view, its three views' sha256 checked before
# every stage. Both are outcome-labelled, so S136's verdict, whichever way it
# went, leaves them as they are.
#
# THE PARTS, from their descriptions (DEC-221; DEC-326 (1), (2)). Fixed order,
# each screened with the earlier survivors live (the step file's pitfall 3):
#   P0  the curve: one cell per count a piece can have, knight 0..8, bishop
#       0..13, rook 0..14, queen 0..27, read at today's count, the squares
#       the piece attacks that hold no own piece. The table at identity --
#       cell[t][c] = c x the shipped linear weight, S083's own fit, DEC-134
#       form (b) -- is what phase 1 lands, so P0's start is the evaluation
#       that ships, bit for bit.
#   P1  a square an enemy pawn attacks is not counted. Mask bit 1.
#   P2a the mobility area replaces "not an own piece": an own piece's square
#       counts, unless it holds an own blocked pawn -- one whose push square
#       holds any piece. Mask bit 2. Today's count already drops every own
#       piece, so an own-pawn exclusion means something only once own pieces
#       count; the switch that lets them is inseparable from the first
#       own-pawn exclusion and is this part's price, read as the accepts' own
#       form (DEC-326 (1), amended the same day). The own king's square is not
#       excluded: the accepts names pawns only, and that exclusion stays
#       unmeasured.
#   P2b an own pawn on the side's own second or third rank is excluded as
#       well. Mask bit 8, always with bit 2: over "not an own piece" the
#       exclusion is a no-op, so low-rank pawns are never measured there. Its
#       predecessor is P2a when P2a survives; when P2a is dropped, P2b is
#       screened as the area with both own-pawn exclusions against the last
#       survivor.
#   P3  a piece pinned to its own king counts only the squares on the line
#       through the king and itself (DEC-258): a pinned knight none. Both
#       colours, src/bitboard.cpp pinned_pieces() in the patch. Mask bit 4.
#   The area and the pins change the mobility count alone; king safety reads
#   the raw attack set (S122's excludes). A part's variant is built with the
#   bits of the parts kept before it plus its own (P2b's own being 8 and 2).
#
# THE IDENTITY CHECK, per variant, before any of its fits (S134's lesson).
# `audit PART` runs tools/feature_audit `--free mobility`, built from the
# part's variant, over the corpus, and passes only on S121_screen.py
# audit-check's rule:
#   - rank over the rationals exactly 908 - Z of 955, Z the mobility columns
#     zero on every row (a cell half no row reaches), where the 831-wide
#     layout read 792 of 831: 124 columns more and 8 null directions more;
#   - the zero columns are those Z and the 32 dead pawn columns, no other;
#   - fifteen dependencies by construction, each holding on every row: S134's
#     seven (five material-against-table, two king tables) and S121's eight,
#     one per mobility type and half -- every piece adds exactly one cell of
#     its row, so a row's cells summed carry the same coefficient as the
#     piece's table squares summed, White's pieces of the type minus Black's;
#     adding c to every cell of a row and taking c off every square of the
#     half's table is the same linear score, told apart only where the clamp
#     binds. No fit here walks them: every fit is `--only mobility`, the
#     tables frozen, and the report says so per level ("held by the freeze
#     under --free mobility", eight lines). They are not gauges (a hold would
#     move the clamped rows); the tuner refuses a free set that frees both
#     sides of one, and S126's joint refit must hold or freeze them;
#   - no exact dependency beyond those, and the span line's "every null
#     direction is one Adam never moves, one the tuner holds, or a mobility
#     level the run's frozen columns hold". Anything else stops the screen
#     for the coordinator.
#
# THE TIES (DEC-326 (5), DEC-084: every shipped entry a fitted value). From
# the same report's `support` lines: a cell half is supported when the rows
# that put a non-zero coefficient on it at a phase giving that half a taper
# weight come from at least TIE_GAMES = 1000 games (S121_screen.py) --
# games, because the label is the game's, and a cell a handful of games reach
# is fitted to those games' results by an optimiser that steps every
# coordinate at its learning rate however few rows feed it. 1000 games is
# 0.72 % of the corpus's 137978; at it the standard error of a game-mean
# result is about 0.45 / sqrt(1000) = 0.014, near 13 cp at the sigmoid's
# centre (slope 0.0011 a centipawn at K 0.76), a choice and not a
# measurement. An unsupported half is tied to the nearest supported cell of
# its piece and half, the lower count on a tie, one shared parameter
# (tools/tuner_groups.hpp parse_ties, `--tie`); a piece and half with no
# supported cell stops the screen. The list, with each tied cell's rows and
# games, is written by `audit` before any fit, $OUT/audit/m<mask>/ties.txt,
# and copied into the reading. A tied cell takes its anchor's value once the
# run's start figures are read at the shipped constants and before the first
# step, so a run's start is still the shipped constants' and every gradient is
# taken on the tied parameterisation.
#
# THE FIT, DEC-304's settings as S135's and S136's screens ran them: `tuner
# --data DATA --holdout .tuning/s082/pool.tsv.qs.validation --only mobility
# --tie TIES --k K --epochs 5000 --report 100 --patience 20 --lambda 0 --seed
# 1 --threads 12`. 132 parameters free less the tied ones. K is fitted once
# per sample -- the full corpus and each quarter -- by a zero-epoch run of
# HEAD's tuner at the shipped constants, and handed to every fit on that
# sample, every part and form alike, so that a paired difference compares two
# fitted ends on one scale (a part fitting its own K at an identity start
# over another count would move the scale the losses are read on). One
# frozen copy of each variant's tuner.
#
# THE NOISE: five samples of the corpus at a quarter of its rows,
# `corpus_split.py sample --in CORPUS --rows C/4 --per-game 0 --seed s`, s = 1
# to 5, the three views named, drawn once and kept for every part, so each
# part's quarter fit and its predecessor's share their rows.
#
# THE READING, by S121_screen.py read, mechanically (DEC-326 (3)):
#   P0  DEC-315 as written: g = start - best on each fit, both on the fixed
#       set, start the shipped evaluation; s the sample standard deviation of
#       the five quarter gains; m = max(1.645 s, 2e-6); kept iff full g > m.
#   P1, P2a, P2b, P3  paired by sample: d_x = best(predecessor, x) - best(part, x)
#       for x the full corpus and each quarter; s the standard deviation of
#       the five quarter d; m = max(1.645 s, 2e-6); kept iff full d > m. The
#       predecessor is the last survivor's fit: P0's, or the last part kept.
#   If P0 is dropped, P1 to P3 (P2a and P2b included) are screened over the
#   linear model instead -- every cell c times its row's count-1 cell, `--tie`
#   with scale c (S121_screen.py linear), four weights a half -- against the
#   linear model refitted over today's count, `linear_m0`, which `screen P1`
#   fits first: fitted against fitted, never against the shipped linear
#   unfitted. A linear candidate ships lround(c x w) per cell, w the fitted
#   real weight, which is a table within half a centipawn of linear.
#   Unreadable, nothing chosen: a fit missing or refused, a log whose start:
#   and best: lines disagree, s above 1e-3, or a full fit whose tables pass
#   the overflow assert. Printed beside, deciding nothing: every fit's start,
#   best, the epoch of its best report and whether it stopped on patience or
#   at the 5000-epoch cap (pitfall 1 is answered by convergence: a fit at the
#   cap is reported, not re-run); the full fit's eight tables. No monotonicity
#   is imposed or checked (pitfall 2).
#
# THE MARGIN, AS DEC-315 STATES IT AND S135'S HEADER ARGUES IT, NOT RE-ARGUED:
# s prices training-corpus resampling alone; the fixed set's own noise and the
# best-report pick on it are common to a part and its predecessor and
# unpriced, accepted because the screen only removes a part and the one SPRT
# decides what ships. P0 frees up to 132 parameters, which makes the
# best-report pick's upward bias larger than S135's 2 to 4 and still
# unmeasured. The floor of 2e-6 is two units of the tuner's sixth printed
# decimal.
#
# THE TUNER IS HEAD'S, CHECKED. Each variant's frozen tuner is first run for
# zero epochs on the corpus's first 2000 rows and its header compared with
# HEAD's sources (S121_screen.py start-check): the parts change counts, not
# constants, so every variant starts from HEAD's. Every fit's header is
# checked the same way after it, the eight mobility tables excepted.
#
# THE HEADROOM. src/evaluation.cpp's overflow assert is 32 x the largest
# |entry| of the eight tables <= 32767 (S121_screen.py headroom). A full fit
# past it is not clipped and not pasted: the coordinator's (DEC-084).
#
# THE CANDIDATE: the last survivor's full-corpus fit itself, in its form and
# mask (DEC-321's form: with cumulative parts no joint refit is owed). P3's
# reading names it, `CANDIDATE <fit> mask <m>`. No survivor at all -- P0
# dropped and no part beating the linear predecessor -- means no candidate, no
# SPRT, each part's zero recorded with its figures, and the table stays at
# identity, which scores as the linear term did.
#
# THE BISECTION, ON H0 OR NO VERDICT (DEC-259 (c), DEC-326 (8)): A the curve
# | B1 the enemy-pawn exclusion | B2a the area in place of ~own with blocked
# pawns excluded | B2b low-rank pawns excluded as well | C the pin rule, in
# that order, each its own `{0, 5}` nElo SPRT in S121_sprt.sh's shape,
# survivors only: one family SPRT and up to five part SPRTs. A is P0's full
# fit against the phase 1 landing. B2b carries the area whether B2a landed or
# not. B1, B2a, B2b and C each play their count over what has landed by then: the table
# refitted over the new count when A has landed, and otherwise the linear
# model refitted over the new count, four weights a half -- `part NAME
# LANDED CURVE` builds, audits and fits it from HEAD as it stands then. A
# part's H1 lands before the next is fitted.
#
# THE CLAMP (DEC-324, DEC-326 (9)). Read at `apply`, on the pasted tree's own
# tools over the corpus: the share of rows on which the stage-two clamp binds
# (feature_audit's load line) and tools/eval_spread's mobility p99 and maximum.
# At the shipped constants the share read 0.338 % (selfplay_v2_dedup.tsv) to
# 0.462 % (DEC-324). If the candidate's share passes CLAMP_SHARE_BOUND = 1.0 %,
# more than twice the shipped share on this corpus, `apply` prints STOP and
# phase 2 stops for the coordinator. S122 inherits both readings.
#
# MACHINE HOURS, from S135's and S136's measured fits (.tuning/s135/screen,
# 30 fits in 1 h 09 m: a full fit of 6226944 rows 5.3 to 10.7 min at 2000 to
# 3600 epochs, a quarter 1.3 to 2.2 min; 0.0252 s an epoch per million rows at
# the 5000-epoch cap, 13.1 min full and 3.3 min a quarter). An epoch's cost is
# the rows' trace, which the free set does not shorten, so 132 free columns
# cost what 2 did an epoch; whether they need more epochs is not measured.
# Per part: six fits, 14 min typical, 29.5 min at the cap. A variant's tools
# about 3 min from a fresh worktree with ccache; `audit` about 2 min (S135's
# 40 s to 2 min); the six K probes about 4 min once; the samples about 2 min
# once. `all`, five parts: about 1 h 30 m typical, 2 h 55 m at the cap,
# 3 h 25 m with `linear_m0`'s six fits if P0 is dropped; ceiling 6 h 30 m.
# Daytime under DEC-155 when typical; at the cap it stays under four hours.
# `locate` under a minute. `read` seconds. `apply` about 8 min. A bisection
# `part` one variant, one audit, one full fit: about 20 min, ceiling 45 min.
#
# DISK. The five quarter samples, about 124 MB each, 0.62 GB, kept until the
# screen is recorded, each reproducible from its seed and the corpus's sha256;
# up to four variant worktrees with their builds, about 250 MB each. A stage
# refuses to start under 4 GB free.
#
# OPEN FINDINGS (BUGS as scoped by DEC-171): written at launch by the
# coordinator, in the step file, against plan.md's Open list on the day.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s121}"
S083_OUT="${S083_OUT:-$root/.tuning/s083}"
CORPUS_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
THREADS=12
FREE_GB_FLOOR=4
SEEDS="1 2 3 4 5"

S082_OUT="${S082_OUT:-$root/.tuning/s082}"
HOLDOUT="$S082_OUT/pool.tsv.qs.validation"
VIEWS=("$S082_OUT/pool.tsv.qs.validation"
       "$S082_OUT/pool.tsv.root.validation"
       "$S082_OUT/pool.tsv.pv.validation")
VIEW_SHAS=(2b508067fb64693bb4b278a2455fb9d101eaa742975a4b53f4ff56e6fa16981e
           58bcbb315fd82bd332372d0b2f1df7d7370693568bd27fa9ed6f17b8e74652d6
           24c4f671131c4a6f3f945ef0f5bb0d8cf85969b41d2e846e745539ef2d53ffdd)
FIT="--only mobility --epochs 5000 --report 100 --patience 20 --lambda 0
     --seed 1 --threads $THREADS"

patch="$root/adocs/data/S121_parts.patch"
split="$root/tools/corpus_split.py"
helper="$root/adocs/data/S121_screen.py"
s135_helper="$root/adocs/data/S135_screen.py"

say() { echo "$*"; }

binary_line() {
  printf '%-9s sha256 %s\n' "$1" "$(sha256sum "$2" | cut -d' ' -f1)  $2"
}

# The engine a stage ran, as S083_corpus.sh and S135_screen.sh name it.
engine_stamp() {
  local head untracked
  untracked=$(git -C "$root" ls-files --others -- src tools |
                grep -v '/__pycache__/')
  if [[ -n "$untracked" ]]; then
    {
      echo "refused: untracked files under src/ or tools/, which no stamp names:"
      echo "$untracked"
    } >&2
    return 1
  fi
  head=$(git -C "$root" rev-parse --short HEAD)
  if git -C "$root" diff --quiet HEAD -- src tools; then
    echo "$head"
    return
  fi
  git -C "$root" diff --no-ext-diff --no-color --full-index --binary HEAD \
      -- src tools > "$1"
  echo "$head+$(git hash-object --no-filters "$1" | cut -c1-12), the diff at $1"
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

# The tuner must be S121's: one built before it has no `--tie`. The usage is
# captured before it is searched (S135_cost.sh carries_columns: pipefail and
# SIGPIPE).
check_tuner() {
  local usage
  usage=$("$1" --help 2>&1)
  [[ "$usage" == *--tie* ]] || {
    say "refused: $1 has no --tie; build the tools from a tree with S121's phase 1 landed"
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

# A part's tools for parts mask $1 under $2 (a variants directory): mask 0 is
# build/'s own tuner and feature_audit, frozen; any other mask a throwaway
# worktree of HEAD with the parts patch, built with -DS121_PARTS=<mask>. The
# stamp names HEAD, the mask and the patch's sha256; a directory built from
# anything else is refused, never rebuilt over.
variant() {
  local mask="$1" dir="$2/m$1" tree stamp had
  stamp="$(git -C "$root" rev-parse HEAD) mask $mask patch $(sha256sum "$patch" | cut -c1-64)"
  mkdir -p "$dir/bin" || return 1
  if [[ -f "$dir/stamp" ]]; then
    had=$(cat "$dir/stamp")
    [[ "$had" == "$stamp" ]] ||
      { say "refused: $dir was built from '$had', not '$stamp'; take a fresh OUT"; return 1; }
    check_copy "$dir/bin/tuner" && check_copy "$dir/bin/feature_audit" || return 1
    return 0
  fi
  if (( mask == 0 )); then
    freeze_binary "$root/build/tools/tuner" "$dir/bin/tuner" > /dev/null || return 1
    freeze_binary "$root/build/tools/feature_audit" "$dir/bin/feature_audit" > /dev/null || return 1
  else
    tree="$dir/tree"
    [[ -e "$tree" ]] && { say "refused: $tree exists with no stamp; remove it or take a fresh OUT"; return 1; }
    git -C "$root" worktree add -q --detach "$tree" HEAD || return 1
    git -C "$tree" apply "$patch" || { say "adocs/data/S121_parts.patch does not apply to HEAD"; return 1; }
    git -C "$tree" config submodule.tests/doctest.url "$root/tests/doctest" &&
      git -C "$tree" -c protocol.file.allow=always submodule update --init \
        > "$dir/submodule.log" 2>&1 ||
      { cat "$dir/submodule.log"; say "the doctest submodule did not check out"; return 1; }
    cmake -S "$tree" -B "$tree/build" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "-DCMAKE_CXX_FLAGS=-DS121_PARTS=$mask" \
        > "$dir/cfg.log" 2>&1 || { tail -20 "$dir/cfg.log"; say "configure of mask $mask failed"; return 1; }
    cmake --build "$tree/build" --target tuner feature_audit -j"$THREADS" \
        > "$dir/build.log" 2>&1 || { tail -20 "$dir/build.log"; say "the mask $mask tools did not build"; return 1; }
    freeze_binary "$tree/build/tools/tuner" "$dir/bin/tuner" > /dev/null || return 1
    freeze_binary "$tree/build/tools/feature_audit" "$dir/bin/feature_audit" > /dev/null || return 1
  fi
  check_tuner "$dir/bin/tuner" || return 1
  echo "$stamp" > "$dir/stamp"
  say "variant m$mask: tuner $(cut -c1-16 "$dir/bin/tuner.sha256"), feature_audit $(cut -c1-16 "$dir/bin/feature_audit.sha256")"
}

# A zero-epoch run of tuner $1 on corpus $2's first 2000 rows, its header
# compared with HEAD's sources.
probe_tuner() {
  local copy="$1" corpus="$2" dir="$3"
  head -n 2000 "$corpus" > "$dir/probe.tsv" || return 1
  check_copy "$copy" || return 1
  "$copy" --data "$dir/probe.tsv" --out "$dir/probe.hpp" --epochs 0 \
      --only mobility --threads "$THREADS" > "$dir/probe.log" 2>&1 ||
    { tail -5 "$dir/probe.log"; say "the probe run of $copy failed"; return 1; }
  python3 "$helper" start-check "$dir/probe.hpp" --tree "$root"
}

# The identity report and the ties of mask $1's variant, under $2.
audit_mask() {
  local mask="$1" base="$2" info corpus rows dir
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  dir="$base/audit/m$mask"
  if [[ -f "$dir/pass" ]] && grep -qs "corpus $CORPUS_SHA" "$dir/pass"; then
    say "m$mask audited before: $(cat "$dir/pass")"
    return 0
  fi
  mkdir -p "$dir" || return 1
  variant "$mask" "$base/variants" || return 1
  local bin="$base/variants/m$mask/bin"
  {
    echo "S121 audit, mask $mask, $(date -Iseconds)"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line audit "$bin/feature_audit"
    cat "$base/variants/m$mask/stamp"
  } > "$dir/audit.log"
  check_copy "$bin/feature_audit" || return 1
  "$bin/feature_audit" --data "$corpus" --free mobility > "$dir/report.txt" 2>&1 ||
    { tail -20 "$dir/report.txt"; say "feature_audit failed on mask $mask"; return 1; }
  grep -E '^identities: (rank|no exact|.*null dimensions|by construction.*S121|mobility level)' \
      "$dir/report.txt" | tee -a "$dir/audit.log"
  python3 "$helper" audit-check "$dir/report.txt" | tee -a "$dir/audit.log"
  (( PIPESTATUS[0] == 0 )) ||
    { say "identity: not the rule's -- the coordinator's, before any fit ($dir/report.txt)"; return 1; }
  python3 "$helper" ties "$dir/report.txt" --out "$dir/ties.txt" | tee -a "$dir/audit.log"
  (( PIPESTATUS[0] == 0 )) || { say "the ties refused on mask $mask"; return 1; }
  echo "pass $(date -Iseconds) corpus $CORPUS_SHA" > "$dir/pass"
}

# What PART is fitted as, from the readings before it (S121_screen.py plan),
# into the globals below.
p_form="" p_mask="" p_fit="" p_ties="" p_pred="" p_pred_mask="" p_pred_ties=""
plan_of() {
  local text
  text=$(python3 "$helper" plan "$1" --out "$OUT") || { echo "$text"; return 1; }
  echo "$text"
  p_form=$(sed -n 's/^form //p' <<< "$text")
  p_mask=$(sed -n 's/^mask //p' <<< "$text")
  p_fit=$(sed -n 's/^fit //p' <<< "$text")
  p_ties=$(sed -n 's/^ties //p' <<< "$text")
  p_pred=$(sed -n 's/^pred //p' <<< "$text")
  p_pred_mask=$(sed -n 's/^pred_mask //p' <<< "$text")
  p_pred_ties=$(sed -n 's/^pred_ties //p' <<< "$text")
}

# The tie file a fit reads: a variant's support ties, or the linear model's.
ties_path() {
  if [[ "$1" == linear.txt ]]; then
    [[ -f "$OUT/linear.txt" ]] || python3 "$helper" linear --out "$OUT/linear.txt" > /dev/null || return 1
    echo "$OUT/linear.txt"
  else
    local mask="${1#ties_m}"
    mask="${mask%.txt}"
    [[ -f "$OUT/audit/m$mask/ties.txt" ]] || { echo "no ties for mask $mask; run audit first"; return 1; }
    echo "$OUT/audit/m$mask/ties.txt"
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
  local x data tuner="$OUT/variants/m0/bin/tuner"
  for x in full q1 q2 q3 q4 q5; do
    [[ -f "$OUT/k/$x.k" ]] && continue
    data="$corpus"; [[ "$x" != full ]] && data="$dir/$x.tsv"
    check_copy "$tuner" || return 1
    "$tuner" --data "$data" --holdout "$HOLDOUT" --out "$OUT/k/$x.hpp" \
        --epochs 0 --only mobility --threads "$THREADS" > "$OUT/k/$x.log" 2>&1 ||
      { tail -5 "$OUT/k/$x.log"; say "the K probe on $x failed"; return 1; }
    sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$OUT/k/$x.log" > "$OUT/k/$x.k"
    [[ -s "$OUT/k/$x.k" ]] || { say "the K probe on $x printed no K"; return 1; }
    say "K $x $(cat "$OUT/k/$x.k")"
  done
}

# One fit; skipped when its log already holds a best: line, so a stage cut
# short resumes.
fit_one() {
  local tuner="$1" data="$2" log="$3" k="$4" ties="$5"
  if [[ -f "$log" ]] && grep -q '^best: ' "$log"; then
    say "$(basename "$(dirname "$log")")/$(basename "$log" .log) fitted before"
    return 0
  fi
  check_copy "$tuner" || return 1
  # shellcheck disable=SC2086
  "$tuner" --data "$data" --holdout "$HOLDOUT" --out "${log%.log}.hpp" \
      --k "$k" --tie "$ties" $FIT > "$log" 2>&1 ||
    { say "fit $log failed; see it"; return 1; }
  say "$(basename "$(dirname "$log")")/$(basename "$log" .log): $(grep '^best: ' "$log")"
  python3 "$helper" start-check "${log%.log}.hpp" --tree "$root" --free || return 1
}

# The six fits of one form and mask.
fit_set() {
  local name="$1" mask="$2" tiefile="$3" info corpus rows x data ties
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  ties=$(ties_path "$tiefile") || { echo "$ties"; return 1; }
  local dir="$OUT/fits/$name" tuner="$OUT/variants/m$mask/bin/tuner"
  mkdir -p "$dir" || return 1
  [[ -f "$dir/ties.txt" ]] || cp "$ties" "$dir/ties.txt" || return 1
  cmp -s "$ties" "$dir/ties.txt" || { say "refused: $dir was fitted with other ties"; return 1; }
  probe_tuner "$tuner" "$corpus" "$dir" || return 1
  for x in full q1 q2 q3 q4 q5; do
    data="$corpus"; [[ "$x" != full ]] && data="$OUT/samples/$x.tsv"
    fit_one "$tuner" "$data" "$dir/$x.log" "$(cat "$OUT/k/$x.k")" "$dir/ties.txt" || return 1
  done
}

audit() {
  local part="$1"
  enough_disk || return 1
  plan_of "$part" || return 1
  audit_mask "$p_mask" "$OUT" || return 1
  if [[ -n "$p_pred" ]]; then
    audit_mask "$p_pred_mask" "$OUT" || return 1
  fi
}

screen() {
  local part="$1" stamp
  enough_disk || return 1
  check_views || return 1
  plan_of "$part" || return 1
  [[ -f "$OUT/audit/m$p_mask/pass" ]] || { say "refused: no audit pass for mask $p_mask; run audit $part first"; return 1; }
  samples || return 1
  mkdir -p "$OUT/fits" || return 1
  stamp=$(engine_stamp "$OUT/fits/engine_$part.diff") || return 1
  {
    echo "S121 screen $part"
    echo "started   $(date -Iseconds)"
    echo "engine    $stamp"
    sed 's/^/corpus    /' "$OUT/corpus.txt"
    binary_line patch "$patch"
    binary_line helper "$helper"
    binary_line split "$split"
    echo "python    $(python3 --version 2>&1)"
    echo "plan      form $p_form mask $p_mask fit $p_fit ties $p_ties pred ${p_pred:-(start)}"
  } >> "$OUT/fits/screen.log"
  if [[ "$p_pred" == linear_m0 ]]; then
    # P0 dropped and no part kept yet: the predecessor is the linear model
    # refitted over today's count, fitted here, against fitted (DEC-326 (3)).
    # fit_set skips a fit whose log already holds its best: line.
    fit_set "$p_pred" "$p_pred_mask" "$p_pred_ties" | tee -a "$OUT/fits/screen.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  elif [[ -n "$p_pred" ]]; then
    # Every other predecessor is a part screened and kept before, its six fits
    # already on disk.
    local x
    for x in full q1 q2 q3 q4 q5; do
      grep -qs '^best: ' "$OUT/fits/$p_pred/$x.log" ||
        { say "refused: the predecessor $p_pred has no $x fit"; return 1; }
    done
  fi
  fit_set "$p_fit" "$p_mask" "$p_ties" | tee -a "$OUT/fits/screen.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  echo "finished  $part $(date -Iseconds)" >> "$OUT/fits/screen.log"
  read_part "$part"
}

read_part() {
  python3 "$helper" read "$1" --out "$OUT"
}

all() {
  local part
  locate || return 1
  for part in P0 P1 P2a P2b P3; do
    say "== $part"
    if [[ -f "$OUT/read/$part.txt" ]] && grep -qE '^PART-OUTCOME (kept|dropped)$' "$OUT/read/$part.txt"; then
      say "$part read before: $(grep '^PART-OUTCOME' "$OUT/read/$part.txt")"
      continue
    fi
    audit "$part" || { say "S121-AUDIT-FAILED ($part)"; return 1; }
    screen "$part" || { say "S121-SCREEN-FAILED ($part)"; return 1; }
  done
  grep -E '^(survivors|candidate|bisection|CANDIDATE)' "$OUT/read/P3.txt"
}

# A bisection part on H0 or no verdict: its count over what has landed,
# fitted from HEAD as it stands then, the table when the curve landed and
# the linear model when it did not; one full fit, under $OUT/bisect/NAME.
part() {
  local name="$1" landed="$2" curve="$3" spec form mask fit base info corpus rows ties
  enough_disk || return 1
  check_views || return 1
  spec=$(python3 "$helper" part "$name" --landed "$landed" --curve "$curve") ||
    { echo "$spec"; return 1; }
  echo "$spec"
  form=$(sed -n 's/^form //p' <<< "$spec")
  mask=$(sed -n 's/^mask //p' <<< "$spec")
  fit=$(sed -n 's/^fit //p' <<< "$spec")
  base="$OUT/bisect/$name"
  mkdir -p "$base/fits/$fit" "$base/k" || return 1
  audit_mask "$mask" "$base" || return 1
  if [[ "$form" == table ]]; then
    ties="$base/audit/m$mask/ties.txt"
  else
    ties=$(ties_path linear.txt) || return 1
  fi
  info=$(corpus_of) || { echo "$info"; return 1; }
  read -r corpus rows <<< "$info"
  variant 0 "$base/variants" || return 1
  if [[ ! -s "$base/k/full.k" ]]; then
    "$base/variants/m0/bin/tuner" --data "$corpus" --holdout "$HOLDOUT" \
        --out "$base/k/full.hpp" --epochs 0 --only mobility --threads "$THREADS" \
        > "$base/k/full.log" 2>&1 ||
      { say "the K probe failed"; return 1; }
    sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$base/k/full.log" > "$base/k/full.k"
  fi
  cp "$ties" "$base/fits/$fit/ties.txt" || return 1
  probe_tuner "$base/variants/m$mask/bin/tuner" "$corpus" "$base/fits/$fit" || return 1
  fit_one "$base/variants/m$mask/bin/tuner" "$corpus" "$base/fits/$fit/full.log" \
      "$(cat "$base/k/full.k")" "$base/fits/$fit/ties.txt" || return 1
  python3 "$helper" headroom "$base/fits/$fit/full.hpp" || return 1
  echo "$mask" > "$base/fits/$fit/mask.txt"
  say "part $name: $base/fits/$fit/full.hpp, mask $mask, sha256 $(sha256sum "$base/fits/$fit/full.hpp" | cut -d' ' -f1)"
}

# The candidate into TREE: the parts patch and its mask when the candidate has
# one, the paste by .tuning/apply_fit.py, the check by S121_verify_fit.py and
# that only the mobility tables moved, the formatter, both builds, bench and
# both fast suites -- their reds recorded, never answered here -- then the
# clamp's two readings on the pasted tree's own tools. TREE stays
# uncommitted; nothing here edits a test.
apply() {
  local tree="$1" line name mask fit dir info corpus rows
  if [[ -n "${2:-}" ]]; then
    # A bisection part's fit, as `part` wrote it, with its mask beside it.
    fit="$2"
    [[ -f "$(dirname "$fit")/mask.txt" ]] || { say "refused: $fit has no mask.txt beside it"; return 1; }
    mask=$(cat "$(dirname "$fit")/mask.txt")
    name="part_$(basename "$(dirname "$(dirname "$(dirname "$fit")")")")"
  else
    line=$(grep '^CANDIDATE ' "$OUT/read/P3.txt" 2>/dev/null) ||
      { say "refused: $OUT/read/P3.txt names no candidate"; return 1; }
    [[ "$line" == "CANDIDATE none" ]] && { say "refused: the screen chose no candidate"; return 1; }
    name=$(awk '{print $2}' <<< "$line")
    mask=$(awk '{print $4}' <<< "$line")
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
  say "S121 apply: $fit (mask $mask) into $tree at $(git -C "$tree" rev-parse --short HEAD)"
  sha256sum "$fit" | tee "$dir/fit.sha256"
  python3 "$helper" start-check "$fit" --tree "$tree" --free || return 1
  python3 "$helper" headroom "$fit" || { say "refused before the paste: the pre-registered branch, the coordinator's"; return 1; }

  if (( mask != 0 )); then
    git -C "$tree" apply "$patch" || { say "the parts patch does not apply to $tree"; return 1; }
    sed -i "s/^#define S121_PARTS 0$/#define S121_PARTS $mask/" "$tree/src/evaluation.hpp"
    grep -qx "#define S121_PARTS $mask" "$tree/src/evaluation.hpp" ||
      { say "the mask did not land in $tree/src/evaluation.hpp"; return 1; }
    say "parts patch applied, S121_PARTS $mask by default (folded at the closing commit, INV-6 against this build)"
  fi

  python3 "$root/.tuning/apply_fit.py" "$fit" --root "$tree" | tee "$dir/apply_fit.txt"
  (( PIPESTATUS[0] == 0 )) || { say "apply_fit refused"; return 1; }
  python3 "$root/adocs/data/S121_verify_fit.py" "$fit" --root "$tree" | tee "$dir/verify_fit.txt"
  (( PIPESTATUS[0] == 0 )) || { say "verify_fit found the paste incomplete"; return 1; }
  python3 "$helper" only-mobility "$tree" | tee "$dir/only_mobility.txt"
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
    2) tail -20 "$dir/build.log"; say "the Release build failed (a static_assert on a packed weight fires here)"; return 1 ;;
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

mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S121-FAILED"; exit 1; }

case "$stage" in
  locate)
    if locate; then echo "S121-LOCATE-DONE"; else echo "S121-LOCATE-FAILED"; exit 1; fi ;;
  audit)
    if [[ $# -eq 2 ]] && audit "$2"; then echo "S121-AUDIT-DONE"; else echo "S121-AUDIT-FAILED"; exit 1; fi ;;
  screen)
    if [[ $# -eq 2 ]] && screen "$2"; then echo "S121-SCREEN-DONE"; else echo "S121-SCREEN-FAILED"; exit 1; fi ;;
  read)
    if [[ $# -eq 2 ]] && read_part "$2"; then echo "S121-READ-DONE"; else echo "S121-READ-FAILED"; exit 1; fi ;;
  all)
    if all; then echo "S121-ALL-DONE"; else echo "S121-ALL-FAILED"; exit 1; fi ;;
  part)
    if [[ $# -eq 4 ]] && part "$2" "$3" "$4"; then echo "S121-PART-DONE"; else echo "S121-PART-FAILED"; exit 1; fi ;;
  apply)
    if [[ $# -eq 2 || $# -eq 3 ]] && apply "$2" "${3:-}"; then echo "S121-APPLY-DONE"; else echo "S121-APPLY-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 locate | audit PART | screen PART | read PART | all | part NAME LANDED CURVE | apply TREE [FIT]" >&2
    exit 2 ;;
esac
