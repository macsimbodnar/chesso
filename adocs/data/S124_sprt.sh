#!/usr/bin/env bash
#
# S124: THE ENDGAME FAMILY -- the screen's F1 and F2 scale factor cells (the decided fit for
# F2, F1's and F2's ten cells free over S122's corpus at K 1.3248, F3 dropped by the
# tablebase guard and its three cells at F) and P1's mop-up (SPSA on conversion, kept by
# DEC-358), against the tree that ships the family at its identity: one SPRT at the harness's
# 8+0.08, Hash 16, {0, 5} nElo (DEC-259 (b), DEC-350 (8)), bisected along the pre-registered
# partition on H0 or no verdict.
#
#   cd /home/max/ws/chesso-s124p2 && nohup adocs/data/S124_sprt.sh > /home/max/ws/chesso/.tuning/s124_sprt.console 2>&1 &
#
# Written 2026-11-11 in S124's phase 2b, before any of it ran, in adocs/data/S122_sprt.sh's
# working-tree form (DEC-334, DEC-254), re-pinned to S124. **Why the working tree: an owner's
# stop fired** (DEC-344 (9)): re-derived by its script on S124's candidate's grid
# (adocs/data/S124_sweep_s170.txt), `C_mate7_depth11`'s short-line ceiling rises 4 -> 10, one
# cell (stride 1, 2000000 nodes, 39 mate lines, 10 short), past DEC-299's +2. The ceiling in
# tests/test_mate_carry.cpp is not moved and the budgets S170_pick_budgets.sh answers are not
# applied (DEC-299's guard is not met); the case reads green at its landed budgets. In
# DEC-334's form the SPRT plays the staged tree and the owner rules on the ceiling only on H1.
# Both mate floors hold (`EXACT_FLOOR` 157 against 165 shipping and 156 weakened,
# `MATE_IN_THREE_FLOOR` 11 against 12 and 11). The run plays the uncommitted, staged tree of
# the worktree `/home/max/ws/chesso-s124p2` against its HEAD, fastchess.sh's dirty-tree mode
# (S256, DEC-254): the candidate is named `cand-<HEAD>+<hex>` and the run saves its diff as
# `$OUT/candidate.diff`. Nothing here was run. The coordinator pins REF and fills the
# bracketed lines below, and until then the script refuses to start (the self-check at the
# bottom).
#
# THE COMMITS AND THE TREE, EACH NAMED BY WHAT IS CHECKED OF IT. No guard names a commit
# for its place in the log, so a commit that touches neither src/ nor a build file trips
# none of them, wherever it falls.
#
#   screened  the commit the screen ran from, `c496403`, the engine every part was read
#             against. Checked: its family is at its identity (S124_screen.py
#             landing-check on that commit's src/evaluation.cpp, not on whatever is
#             checked out); and it is the commit the screen's .tuning/s124/screen/bin/stamp
#             names, in this worktree (gitignored), which the screen wrote on freezing its
#             tools and held every stage to (S124_screen.sh tools). P1 ran at `54c892d`,
#             a descendant whose src/ and build files are `screened`'s.
#   ref       what the candidate is played against, and the worktree's HEAD, `7214ce5` at
#             this script's writing (S338's close; the family at its identity since
#             S124's phase 1 landed). Checked: it descends from `screened`; its src/ and
#             build files are `screened`'s byte for byte, or for a later bisection part
#             that plus what has landed (THE PARTS below). A commit cannot hold its own
#             sha, so the pin is an uncommitted edit of the worktree's copy of this file,
#             and the run's candidate.diff holds it, as S101's and S122's did.
#   the tree  the paste: `ref` plus the staged diff, uncommitted. Checked: it is dirty;
#             nothing unstaged under src/, tests/, cmake/ or CMakeLists.txt, nothing
#             untracked under src/ or cmake/; cmake/ and CMakeLists.txt are `ref`'s; under
#             src/ only src/evaluation.cpp differs from `ref`, and outside whole-line
#             comments only in whole definitions of the arrays this run moves, each read
#             to its closing `};` across the lines clang-format wrapped it over and nothing
#             after it; within endgame_scale only the cells the part frees differ from
#             `ref`'s, F3's three at F always; every other weight array and define reads
#             `ref`'s value by name (.tuning/verify_fit.py's reader); and build/'s binary is
#             no older than any source it is built from.
#
# THE PAIR.
#
#   CAND  the working tree of `/home/max/ws/chesso-s124p2`, uncommitted: its HEAD (REF) plus
#         S124's staged paste. `.tuning/apply_fit.py` pasted the screen's decided fit for F2
#         (`.tuning/s124/screen/fits/f1_f2/full/r4/fit.hpp`, sha256 c56cd322...,
#         adocs/data/S124_screen_f1_f2_fit.hpp) with P1's two mop-up lines in place of the
#         fit's zeros (adocs/data/S124_p1_final.txt, sha256 63b6e1fb...), and
#         `.tuning/verify_fit.py` read 1190 of 1190 parameters the composed header's:
#         `endgame_scale` {45, 0, 9, 29, 81, 114, 86, 128, 123, 19, 128, 128, 128},
#         `mopup_mg` {96, 149, 0}, `mopup_eg` {15, 13, 6}; every golden the paste moved
#         re-derived by its own script (DEC-142) but the ceiling above, the first-mate
#         case's second position and the four the step file lists as findings.
#   REF   the worktree's HEAD, which the coordinator pins below.
#
# THE PARTS: HOW THE GUARDS READ A BISECTION PART (on H0 or no verdict, the order the screen's
# pre-registration wrote: A = mop-up, B = F1, C = F2). A part's run is this file over its own
# uncommitted tree, with `ref=` and `part=` re-pinned and the coordinator's sentence
# rewritten; `screened=` stays:
#
#   part   CAND may move                                   REF may carry past `screened`
#   ""     mopup_mg, mopup_eg; endgame_scale cells 0-9      nothing
#   A      mopup_mg, mopup_eg (P1's six, as here)          nothing
#   B      endgame_scale cells 0, 1, 9 (F1's decided fit,  mopup_mg, mopup_eg, if A landed
#          `.tuning/s124/screen/fits/f1/full`)
#   C      endgame_scale cells 0-9 (F2's decided fit,      mop-up's if A landed; cells 0, 1,
#          which frees F1's cells again, amendment 6)      9 if B landed
#
# F3's cells 10, 11 and 12 stay at F = 128 in every part. Mop-up's six are P1's in A and are
# not refit; F1's and F2's cells are the screen's decided fits, not refit either.
#
# WHAT THE VERDICT PRICES. The fitted cells and mop-up and their cost together: every
# stage-one evaluation pays the cell's gate and pick and mop-up's domain test, the shortcut's
# nodes included, the family being stage one. The cost is recorded apart, on identical
# trees, by adocs/data/S124_cost_masks.sh (the candidate, the factor alone, mop-up alone, the
# parent) and printed beside the verdict; it decides nothing, and there is no bar.
#
# WHAT THE VERDICT DOES NOT SEE. **The harness's two-sided resignation at 400 cp
# (fastchess.sh's adjudication, S212) ends most games before a lone-king ending is reached,
# so most of mop-up's conversions are hidden from this verdict**; P1's conversion test
# (adocs/data/S124_p1_*, DEC-358) is the reading of them, and this pair prices what is left:
# the factor's scaling everywhere and mop-up where a game reaches its domain unadjudicated.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's control for an
# evaluation family. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names it for a
# family's survivors, DEC-350 (8) for this one. Rejected: `{-5, 5}`, `--fast` and `{0, 10}`,
# one SPRT per part (S101_sprt.sh's reasons).
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic (`adocs/testing_strategy.md`
# section 1.1, D = 1046535 and 639770 over the bounds' width squared, 25): **41861 games**
# with the truth at the interval's midpoint, 19.84 h at the 2110 games an hour
# `.moltke.local.md` budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** on a
# bound, 12.13 h. S122's run of the same pair and control played 2916 games to H1.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour, S101's rule. Four hours
# or more, so the run starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h),
# wall-clock only while the machine is awake.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd` (DEC-189), concurrency
# 12 (DEC-050), fastchess.sh's adjudication, the governor recorded and not set (DEC-195). If
# fastchess, the book, the adjudication or the machine has moved since the last fixed-rounds
# A/A, that A/A of 1000 games comes first (DEC-143); the coordinator checks on the day.
#
# ABORT RULE, S101's. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's PGN in `$OUT`), on a
# crash or a disconnect on either side (`SPRT-RUN-INVALID`, S212), if the machine loses mains
# power (POWER) or another load starts on it (MACHINE), if the banner is not what the
# coordinator's sentence below says it must print, or at 40 h of wall clock with neither
# bound. Nothing else stops it.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S283's lines with this run's shas, dates
# and out directory --
#
#   candidate  <REF>  <its date>  + uncommitted changes
#   diff       /home/max/ws/chesso/.tuning/sprt_s124_<stamp>/candidate.diff  blob <12 hex>
#   reference  <REF>  <its date>
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s124_<stamp>
#
# with no `relay` or `A/A CALIBRATION` line, fastchess's engine names `cand-<REF, short>+<the
# same 12 hex>` and `ref-<REF, short>`, the candidate's `id name` carrying `-dirty`, and
# fastchess's first `Started game 1 of N` line reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171): S339's three, in
# plan_todo/ at `7214ce5` -- the mutation tool scores a ctest row that was skipped or not run
# as a kill; `validate` dies on a ValueError at a pair that is not an (old, new) pair of
# strings; the loader dies on a KeyError at a file that brings its own `m` and gives no
# pairs. None moves play. No audit finding reads open at `7214ce5`. And whatever plan.md's
# Open list says on the day.
#
# THE COORDINATOR'S SENTENCE, 2026-11-12, before a game is played. REF `7214ce5` (committed
# 2026-11-11 22:44, S338's close: tools and tests only since `screened`, `src/` and the build
# files byte for byte `screened`'s, bench 4707943); `screened` `c496403`, the commit
# `.tuning/s124/screen/bin/stamp` names. The tree: `/home/max/ws/chesso-s124p2`, REF plus the
# staged diff, `.tuning/coord/S124/S124_candidate_v2.diff`, sha256 0a869b19..., 38 files:
# phase 2b's delivery (`S124_candidate.diff`, ecca2b17...) with the step file's section
# "2026-11-12: before the SPRT" added, the paste of the screen's F1 and F2 cells and P1's
# mop-up, bench 4587846 in both builds and read again on the binary rebuilt 2026-11-12 03:10;
# this pin is an unstaged edit of this file beside it. The cost (`S124_cost_masks.sh`, five
# interleaved runs, instructions, node counts the parent's on all four builds; taken beside
# another session's soak test, spread under 1800 in 1e10): both +1.969 % on `bench` and
# +1.955 % on `bench 16`, the factor alone +1.112 %, mop-up alone +0.970 %
# (`.tuning/coord/S124/cost_masks.log`). The harness: fastchess alpha 1.8.1 20260720-daa3ea2,
# `noob_3moves.epd` sha256 2011193b..., `fastchess.sh` last changed `ba1cad6` (S319, comments
# only), the same workstation; **the kernel moved 7.2.6 -> 7.2.15, so an A/A was owed
# (DEC-143) and ran first**, 2026-11-12 02:40 to 03:09, `7214ce5` against itself: 1000 games,
# `SPRT-RUN-DONE full`, no `SPRT-RUN-INVALID`, 0 forfeits, pair-score variance 0.2559 +/-
# 0.0142 against the book's 0.2905 +/- 0.0184 (z -1.49), 2063 games an hour beside 2110; it
# passes the rule the step file wrote before it ran, and the hours above stand. Before the
# run, on the tree: both fast suites 54 of 61, the same seven red in each -- the gradient case
# (the second stop, below) and the six fixtures the step file lists under "Reds left", none
# repaired before the verdict since the bisection's parts would read different ones; format
# clean, the five prose modes 0; `STAGES="debug sanitize clang" tools/gate_extra.sh` red on
# those tests alone, no sanitizer report, the candidate compiling under clang; Debug
# self-play 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`; one fast check over the
# delivery, nothing blocking, two lows left for the repair. **The owner's stops as they stand
# (DEC-344 (9)): two fired and neither is ruled.** The ceiling above; and
# test_tuner_gradient's bound, 1.69336e-6 at parameter 1002 against 1e-6, DEC-316's script
# answering no pair and CLAMP_PUSH's no push, the case left red. On H1 the owner rules on
# both before anything lands. The Open list re-read at `739a537`, `achesso`'s head, three
# commits past REF and none of them in this run: S339 and S342 are closed there (tools
# only); S340's fix is landed there (`5048611`, `[[maybe_unused]]` on mopup_terms and
# scaled_taper so clang builds the identity, no functional change, in neither side of this
# pair); S341 is open, filler (DEC-171): test_tuner_gradient's factor case reads 4.70e-6
# under clang's library at the identity. None moves play. S102 onward are strength steps not
# begun in this tree. The governor reads `performance`, recorded and not set (DEC-195).
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted family is accepted as gaining: H1's 5 nElo preferred
#                   to H0's 0 at alpha = beta = 0.05. **The owner rules on the mate-carry
#                   ceiling first** (DEC-334's form, DEC-344 (9)): C_mate7_depth11's 4 -> 10.
#                   On the owner's word the tree lands with the DEC-142 items phase 2b
#                   re-derived, the ceiling and S170's budgets as the owner rules them, the
#                   step's specs.md rows and the record commit's block. F3's cells stay at F,
#                   refused by the guard, recorded with their figures. The stopping run's Elo
#                   is upward-biased (DEC-063) and is not the effect size.
#
#   H0           -> not shown to gain. **Nothing was committed, so the paste is simply not
#                   landed**: REF's family stays at its identity, with REF's goldens. The
#                   bisection follows, survivors only, in order, each its own uncommitted tree
#                   in this file's shape, each stopping for the coordinator: A = mop-up (P1's
#                   six), B = F1 (the screen's decided F1 fit), C = F2 (the screen's decided F2
#                   fit, F1's cells freed again), each `{0, 5}` against what has landed -- at
#                   worst three more runs after this one, each up to 19.86 h, 79.4 h with this
#                   run. A part's H1 lands after its verdict and before the next is pasted; its
#                   H0 or no verdict is that part's zero. Leads, none built: KBNvK's
#                   conversion (63 of 184 fresh starts), F3's cells (S126's).
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound: not landed,
#                   as H0 is, recorded as **unresolved**, and bisected as H0 is (DEC-259 (c)).
#
# ON ANY OUTCOME, before any commit: `EXACT_FLOOR` under 157, `MATE_IN_THREE_FLOOR` under 11,
# a mate-carry ceiling rising by more than 2, or a recurrence on test_tuner_gradient's bound
# stops for the owner (DEC-344 (9)); the ceiling fired at phase 2b (above), the rest did not.
#
# THE COMMIT THAT CLOSES THE VERDICT -- the record commit on H0 or no verdict, on H1 the one
# that lands the tree -- carries DEC-220's block -- `SPRT | cand <REF>+<hex> vs ref <REF>,
# 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`, `Games`, `Wall` and
# `Log` lines -- with the log trimmed into `adocs/data/S124_sprt.log` by
# `tools/trim_console.py` and the run's `candidate.diff` committed beside it as
# `adocs/data/S124_candidate.diff` (DEC-254); `tools/ledger.py` regenerates plan.md's ledger
# table from it.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or SPRT-RUN-FAILED on
# every exit path, and this script execs it and adds nothing after.

set -uo pipefail

fail()
{
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

# THE BOOKED SETTINGS, AND NOTHING ELSE FROM THE CALLER'S ENVIRONMENT.
ref="7214ce5"
worktree="/home/max/ws/chesso-s124p2"
screened="c496403"
part=""           # THE PARTS above: empty for the family, or A, B or C
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"
sprt_rounds="20950"

# THE COORDINATOR'S SENTENCE AND SHAS. While the bracketed marker is anywhere in this file
# the pre-registration is unfinished and the run does not start. The marker is assembled in
# two halves so this line does not match itself.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); pin REF and fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

for pair in "REF=$ref" "TC=$tc" "HASH=$hash" \
            "CONCURRENCY=$concurrency" "SPRT_BOUNDS=$sprt_bounds" \
            "SPRT_ROUNDS=$sprt_rounds"; do
  name="${pair%%=*}"
  want="${pair#*=}"
  if [[ -n "${!name+set}" && "${!name}" != "$want" ]]; then
    fail "$name is '${!name}' in the environment, and this run is booked at" \
         "$name=$want; unset it"
  fi
done
for name in CAND ROUNDS BARE_FEN SRAND AA; do
  if [[ -n "${!name+set}" ]]; then
    fail "$name is set in the environment ('${!name}'), and this run is" \
         "booked without it; unset it"
  fi
done

cd "$worktree" || fail "cd $worktree"

if [[ "$(git rev-parse HEAD)" != "$(git rev-parse "$ref")" ]]; then
  fail "$worktree's HEAD is $(git rev-parse --short HEAD), not REF" \
       "$(git rev-parse --short "$ref") -- the pair would measure more than S124"
fi

[[ -n "$(git status --porcelain)" ]] \
  || fail "$worktree is clean: there is no candidate change to play"

build_paths=(src CMakeLists.txt cmake)

# THE PARTS: the arrays this run may move and the endgame_scale cells it may free; what REF
# may carry past `screened`, and the cells it may carry.
mopup="mopup_mg|mopup_eg"
case "$part" in
  "") moves="$mopup|endgame_scale"; cells="0,1,2,3,4,5,6,7,8,9"; carries="";              carried_cells="" ;;
  A)  moves="$mopup";               cells="";                    carries="";              carried_cells="" ;;
  B)  moves="endgame_scale";        cells="0,1,9";               carries="$mopup";        carried_cells="" ;;
  C)  moves="endgame_scale";        cells="0,1,2,3,4,5,6,7,8,9"; carries="$mopup|endgame_scale"; carried_cells="0,1,9" ;;
  *)  fail "part is '$part', not empty, A, B or C" ;;
esac
[[ "$sprt_bounds" == "elo0=0 elo1=5 alpha=0.05 beta=0.05" ]] ||
  fail "part '$part' is booked at {0, 5}: re-pin sprt_bounds"

# Whether two revisions' src/ differ, outside whole-line comments, only in whole definitions of
# ARRAYS. Each file the two differ in is read whole at both revisions: whole-line comments are
# dropped, every `constexpr int NAME[...] = {...};` of a NAME in ARRAYS is read across the lines
# clang-format wrapped it over and replaced by a placeholder, and what is left must be the same
# text. A definition's first line need not change for this to see it: endgame_scale's values sit
# on its continuation line. A diff of comments alone leaves nothing to read, which fails: it moved
# no weight. NEW is a revision or `-` for the working tree.
#   only_definitions ARRAYS OLD NEW
only_definitions()
{
  local arrays="$1" old="$2" new="$3" file files
  if [[ "$new" == - ]]; then
    files="$(git diff --name-only "$old" -- src)"
  else
    files="$(git diff --name-only "$old" "$new" -- src)"
  fi
  [[ -n "$files" ]] || return 1
  while IFS= read -r file; do
    python3 - "${arrays:-NONE}" \
        <(git show "$old:$file" 2> /dev/null) \
        <(if [[ "$new" == - ]]; then cat "$file"; else git show "$new:$file" 2> /dev/null; fi) <<'EOF2' || return 1
import re, sys
arrays, a, b = sys.argv[1].split("|"), open(sys.argv[2]).read(), open(sys.argv[3]).read()
definition = re.compile(r"^constexpr int (%s)\[[A-Z_0-9]+\] = \{[^}]*\};$" % "|".join(map(re.escape, arrays)), re.M)
def reduce(text):
    text = "\n".join(l for l in text.split("\n") if not re.match(r"^\s*//", l))
    return definition.sub(lambda m: "<<%s>>" % m.group(1), text), definition.findall(text)
ra, na = reduce(a)
rb, nb = reduce(b)
sys.exit(0 if ra == rb and a != b and sorted(na) == sorted(nb) else 1)
EOF2
  done <<< "$files"
  # Something outside whole-line comments moved: the files differ in a definition.
  if [[ "$new" == - ]]; then
    git diff -U0 "$old" -- src | grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' |
      grep -qvE '^[-+][[:space:]]*(//.*)?$'
  else
    git diff -U0 "$old" "$new" -- src | grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' |
      grep -qvE '^[-+][[:space:]]*(//.*)?$'
  fi
}

# Whether endgame_scale differs between two sources only in CELLS, and F3's three are at F in
# the second; and whether every other weight array and define reads the same by name.
#   weights_differ_only OLD_REVISION NEW_REVISION_OR_- ARRAYS CELLS
weights_differ_only()
{
  local old="$1" new="$2" arrays="$3" cells="$4"
  python3 - "$arrays" "$cells" \
      <(git show "$old:src/eval_tables.hpp"; git show "$old:src/evaluation.cpp") \
      <(if [[ "$new" == - ]]; then cat src/eval_tables.hpp src/evaluation.cpp;
        else git show "$new:src/eval_tables.hpp"; git show "$new:src/evaluation.cpp"; fi) <<'EOF'
import re, sys
arrays, cells, a, b = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
allowed = set(arrays.split("|")) if arrays else set()
free = {int(c) for c in cells.split(",")} if cells else set()
def extract(text):
    strip = lambda s: re.sub(r"//[^\n]*", "", s)
    out = {}
    for name, value in re.findall(r"^#define (\w+)\s+(-?\d+)$", text, re.M):
        out[name] = [int(value)]
    for name in ("psqt_mg", "psqt_eg"):
        m = re.search(r"static constexpr int %s\[6\]\[64\] = \{(.*?)\n\};" % name, text, re.S)
        if m:
            out[name] = [int(v) for v in re.findall(r"-?\d+", strip(m.group(1)))]
    for name, body in re.findall(r"^(?:constexpr|const) int (\w+)\[\w+\] = \{([^}]*)\};", text, re.M):
        out[name] = [int(v) for v in re.findall(r"-?\d+", strip(body))]
    for name, value in re.findall(r"^(?:constexpr|const) int (\w+) = (-?\d+);", text, re.M):
        out[name] = [int(value)]
    return out
old, new = extract(open(a).read()), extract(open(b).read())
bad = []
for name in sorted(set(old) | set(new)):
    if old.get(name) == new.get(name):
        continue
    if name not in allowed:
        bad.append("%s moves" % name)
    elif name == "endgame_scale":
        moved = [c for c, (x, y) in enumerate(zip(old[name], new[name])) if x != y]
        if set(moved) - free:
            bad.append("endgame_scale moves cells %s, outside %s" % (sorted(set(moved) - free), sorted(free)))
if "endgame_scale" in new and new["endgame_scale"][10:] != [128, 128, 128]:
    bad.append("F3's cells are %s, not at F" % new["endgame_scale"][10:])
for line in bad:
    print("  " + line, file=sys.stderr)
sys.exit(1 if bad else 0)
EOF
}

# `screened`: the family at its identity, read at that commit.
python3 adocs/data/S124_screen.py landing-check <(git show "$screened:src/evaluation.cpp") ||
  fail "the commit named as the one the screen ran from, $screened, is not the family at its identity"

stamp_file="$worktree/.tuning/s124/screen/bin/stamp"
[[ -f "$stamp_file" ]] ||
  fail "no $stamp_file, so nothing records the commit the screen ran from"
stamped="$(git rev-parse --verify --quiet "$(cat "$stamp_file")^{commit}")" ||
  fail "$stamp_file holds '$(cat "$stamp_file")', which names no commit"
if [[ "$(git rev-parse --verify --quiet "$screened^{commit}")" != "$stamped" ]]; then
  fail "screened $screened is not $stamped, the commit $stamp_file names"
fi

# `ref`: it descends from `screened`, its engine `screened`'s; a later part's REF may also
# carry what has landed by then, and nothing else.
if ! git merge-base --is-ancestor "$screened" "$ref"; then
  fail "REF does not descend from the screened commit $screened"
fi
ref_changed="$(git diff --name-only "$screened" "$ref" -- "${build_paths[@]}")"
if [[ -n "$ref_changed" ]]; then
  [[ -n "$carries" ]] ||
    fail "REF's src/ or build files differ from the screened commit's -- REF is not the" \
         "family at its identity"
  while IFS= read -r file; do
    case "$file" in
      src/evaluation.cpp) ;;
      *) fail "REF changes $file beyond what part $part may have landed" ;;
    esac
  done <<< "$ref_changed"
  only_definitions "$carries" "$screened" "$ref" ||
    fail "REF's src/ differs from the screened commit's outside comments in more than what" \
         "part $part may have landed"
  weights_differ_only "$screened" "$ref" "$carries" "$carried_cells" ||
    fail "REF's weights differ from the screened commit's in more than part $part may have landed"
fi

# ONLY THE STAGED PASTE IS PLAYED.
git diff --quiet -- src tests cmake CMakeLists.txt \
  || fail "unstaged changes under src/, tests/, cmake/ or CMakeLists.txt:" \
          "$(git diff --name-only -- src tests cmake CMakeLists.txt |
             head -5 | tr '\n' ' ')"
untracked="$(git ls-files --others --exclude-standard -- src cmake | head -5)"
[[ -z "$untracked" ]] || fail "untracked files under src/ or cmake/: $untracked"

git diff --quiet "$ref" -- cmake CMakeLists.txt \
  || fail "cmake/ or CMakeLists.txt differs between the tree and REF:" \
          "$(git diff --name-only "$ref" -- cmake CMakeLists.txt |
             head -5 | tr '\n' ' ')"

# THE PASTE IS THE FAMILY'S WEIGHTS, in src/evaluation.cpp alone, comments aside.
changed="$(git diff --name-only "$ref" -- src)"
[[ -n "$changed" ]] || fail "the tree's src/ is REF's -- not S124's paste"
while IFS= read -r file; do
  case "$file" in
    src/evaluation.cpp) ;;
    *) fail "the tree changes $file under src/ beyond the paste" ;;
  esac
done <<< "$changed"

only_definitions "$moves" "$ref" - ||
  fail "the tree's src/ differs from REF's outside comments in more than whole definitions" \
       "of the arrays this run moves (${moves:-none})"

# Every weight array and define outside the arrays this part moves is REF's in the tree,
# compared by name; within endgame_scale only the part's cells, and F3's at F.
weights_differ_only "$ref" - "$moves" "$cells" ||
  fail "the tree moves a weight outside what part '$part' moves against REF"

# BUILD/ WAS BUILT FROM THIS TREE.
binary="$worktree/build/src/chesso"
cmake --build "$worktree/build" -j "$concurrency" --target chesso > /dev/null \
  || fail "cmake --build $worktree/build --target chesso failed"
[[ -x "$binary" ]] || fail "no engine at $binary after the build"
newer="$(find "$worktree/src" "$worktree/cmake" "$worktree/CMakeLists.txt" \
           -newer "$binary" -type f 2> /dev/null | head -5)"
[[ -z "$newer" ]] || fail "sources newer than $binary: $newer"

[[ -s books/noob_3moves.epd ]] \
  || fail "no books/noob_3moves.epd in $worktree; copy the main checkout's"

[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh"

for name in SPRT_BOUNDS SPRT_ROUNDS; do
  if ! grep -qE '\$\{?'"$name"'([^A-Za-z0-9_]|$)' ./fastchess.sh; then
    fail "./fastchess.sh does not read $name, so this run's bounds or cap would be" \
         "ignored -- see THE HARNESS in adocs/data/S283_sprt.sh"
  fi
done

shopt -s execfail
exec env -u CAND -u ROUNDS -u BARE_FEN -u SRAND -u AA -u RELAY_PYTHON \
     REF="$ref" \
     TC="$tc" \
     HASH="$hash" \
     CONCURRENCY="$concurrency" \
     SPRT_BOUNDS="$sprt_bounds" \
     SPRT_ROUNDS="$sprt_rounds" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s124_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
