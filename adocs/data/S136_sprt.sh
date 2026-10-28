#!/usr/bin/env bash
#
# S136: TEMPO -- the side-to-move bonus fitted alone with every other constant
# frozen, `--only tempo --lambda 0` on S083's corpus, kept by DEC-315's screen,
# against the tree that ships it at zero: one SPRT at the harness's 8+0.08,
# Hash 16, {0, 5} nElo (DEC-259 (b), DEC-092). A family of one: no bisection.
#
#   cd /home/max/ws/chesso-s136b &&
#     nohup adocs/data/S136_sprt.sh > /home/max/ws/chesso/.tuning/s136_sprt.console 2>&1 &
#
# Written 2026-10-27 in S136's phase 1, at 80520d7, in adocs/data/S135_sprt.sh's
# and S296_sprt.sh's shape, whose harness section -- S283_sprt.sh's before
# them -- this file relies on and does not repeat. **Switched to WORKING-TREE
# MODE in phase 2c by DEC-323**, in adocs/data/S083_sprt.sh's form (DEC-307
# (3), DEC-254): the paste leaves `EXACT_FLOOR` red (159 against the 156 it
# reads at both ends, `adocs/data/S136_floor.txt`), the one ruling the owner
# makes, and only on H1 (DEC-323), so the candidate is not committed and every
# commit stays green. The run plays the uncommitted tree of the worktree
# `/home/max/ws/chesso-s136b` against its HEAD, fastchess.sh's dirty-tree mode
# (S256, DEC-254): the candidate is named `cand-<HEAD>+<hex>` and the run saves
# its diff as `$OUT/candidate.diff`. Nothing here was run: every figure below
# is read from a file or a git object and names it. The coordinator pins REF
# and fills the bracketed sentence below on the day, and until then the script
# refuses to start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  the working tree of `/home/max/ws/chesso-s136b`, uncommitted: its
#         HEAD (REF) plus S136's paste -- `adocs/data/S136_screen.sh apply`
#         pasted the screen's full-corpus fit, `.tuning/s136/screen/full_tempo.hpp`
#         -- `--only tempo --lambda 0` on S083's candidate corpus as `locate`
#         recorded it (curve 0's top rung, 6226944 rows, sha256 2e0e3de9...,
#         S136_screen.sh's CORPUS_SHA), against S082's fixed validation set --
#         over src/evaluation.cpp `tempo_mg` and `tempo_eg`, 10 and 1; the
#         truncation guard re-targeted by DEC-322 (tests/test_eval_trace.cpp,
#         its red observed first); every golden inside DEC-299's guard
#         re-derived by its own script and applied (DEC-142,
#         `adocs/data/S136_goldens.txt`); `EXACT_FLOOR` left red (DEC-323); the
#         comments the paste made false made true, under src/ in whole-line
#         comments of src/evaluation.cpp alone. Only the staged diff is
#         played: the guards below refuse unstaged changes under src/,
#         tests/, cmake/ or the root CMakeLists.txt, untracked files under
#         src/ or cmake/, and any change to cmake/ or CMakeLists.txt against
#         REF -- the build's inputs outside src/ (CMakeLists.txt includes
#         cmake/arch.cmake and cmake/pgo.cmake). Built in that worktree's
#         build/. Bench 5069022 -> 6007564.
#   REF   the worktree's HEAD, which the coordinator pins below: a commit
#         whose src/ is `80520d7`'s, the tree that ships tempo at zero --
#         S136's phase 1 changed nothing under src/, so the phase 1 landing or
#         a documents-only descendant of it. The worktree was made at
#         `ba65b86`, the pre-registration's commit, and every reading of the
#         paste was taken on its src/.
#
# The two differ by the two tempo weights and what they move, which the
# guards below check: the worktree's HEAD is REF, REF's src/ is byte for byte
# `80520d7`'s, the tree is dirty, src/eval_tables.hpp is the same in the tree
# and at REF, src/evaluation.cpp is the only file under src/ that differs, and
# outside its whole-line comments the only lines that differ there are the two
# `constexpr int tempo_mg` / `tempo_eg` definitions; build/'s binary is no
# older than any source it is built from.
#
# WHAT THE VERDICT PRICES. The fitted pair and its cost together: at zero the
# compiler deletes the term, at a non-zero pair every evaluation pays one
# taper -- seven instructions, `evaluate_cheap()` 164 at zero against 171 at
# forced weights, as src/evaluation.cpp `evaluate_cheap`'s comment records
# S027's disassembly (2026-08-13, on the MacBook's compiler; not re-read on
# g++ 13.3) -- and that cost is inside the verdict. **No speed-cost run is owed**: DEC-047's reason for
# timing a term apart is a count the compiler drops at zero weight and keeps
# at any other, so a zero-weight landing measures nothing; tempo is a
# constant, not a count, there is no feature to compute and nothing to time
# beyond the taper the verdict carries.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 keeps the harness's
# control for every verdict but the correction family's, and this is an
# evaluation term. `{0, 5}` nElo at alpha = beta = 0.05: DEC-259 (b) names it
# for a family's survivors, tempo being one of its families of one; the step's
# accepts asks for "bounds that resolve single digits", read as this pair (the
# step file's 2026-10-27 section, the coordinator's ruling). What is known of
# the size, deciding nothing (DEC-019, DEC-084): S027's run reached neither
# bound at `{0, 10}` `--fast` over 3000 games, -0.69 +/- 9.64 Elo; S100 put the
# label-side signal near 3.6 cp of tempo weight on the old corpus and S135's
# audit near -8.7 cp on S083's, the opposite sign, both confounded; the one
# traced published figure prices adding the term from nothing at another
# engine's band (+12.99 +/- 7.35 at 10+0.1, +6.67 +/- 4.67 at 40+0.4). A true
# effect near 2.5 nElo, the interval's midpoint, random-walks to the cap,
# which is the cost below. Rejected: `{-5, 5}` -- would ship a zero half the time
# (DEC-304's reason); `{0, 10}` and `--fast` -- S027's pair, the run that
# resolved nothing (DEC-063), the failure this step exists not to repeat;
# `{-5, 0}` -- a non-regression pair, for a change wanted for a reason other
# than strength, and tempo has no such reason.
#
# WHAT THE PAIR COSTS (DEC-143), the nElo formula's arithmetic
# (`adocs/testing_strategy.md` section 1.1: D / (e1 - e0)^2 games at the
# midpoint, D = 1046535 at alpha = beta = 0.05; about 639770 / (e1 - e0)^2 on
# a bound): **41861 games** with the truth at the interval's midpoint, 19.84 h
# at the 2110 games an hour `.moltke.local.md` budgets 8+0.08 on
# `noob_3moves.epd` from (DEC-190); **25591 games** with it on a bound,
# 12.13 h. S135's run of the same pair and control played 13406 games in
# 6 h 17 m, 2133 an hour, to H1.
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S283's, S135's and S296's rule, in
# place of fastchess.sh's 20000-round default. Four hours or more, so the run
# starts by DEC-155's night rule. Watcher ceiling 144000 s (40 h), twice the
# cap's hours and the abort below, wall-clock only while the machine is awake.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), fastchess.sh's adjudication, the
# governor recorded and not set (DEC-195). If fastchess, the book, the
# adjudication or the machine has moved since the last fixed-rounds A/A, that
# A/A of 1000 games comes first (DEC-143); the coordinator checks on the day.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's PGN in
# `$OUT`), on a crash or a disconnect on either side (`SPRT-RUN-INVALID`,
# S212), if the machine loses mains power (POWER) or another load starts on it
# (MACHINE), if the banner is not what the coordinator's sentence below says
# it must print, or at 40 h of wall clock with neither bound. Nothing else
# stops it.
#
# THE BANNER, READ BEFORE THE FIRST GAME FINISHES: S283's lines with this
# run's shas, dates and out directory --
#
#   candidate  ba65b86  2026-10-27  + uncommitted changes
#   diff       /home/max/ws/chesso/.tuning/sprt_s136_<stamp>/candidate.diff  blob <12 hex>
#   reference  ba65b86  2026-10-27
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s136_<stamp>
#
# with no `relay` or `A/A CALIBRATION` line, fastchess's engine names
# `cand-<REF, short>+<the same 12 hex>` and `ref-<REF, short>`, the candidate's
# `id name` carrying `-dirty`, and fastchess's first `Started game 1 of N` line
# reading N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# Pinned by the coordinator 2026-10-27 13:15. REF `ba65b86` (2026-10-27
# 11:17, the pre-registration commit), the worktree's HEAD; its src/ is
# `80520d7`'s, identical (`git diff --quiet 80520d7 ba65b86 -- src`). The
# staged diff, `adocs/data/S136_sprt.sh` excluded on both sides, is byte for
# byte the one the fast check read (`cmp` SAME, after `sha256sum -c` of
# `s136_phase2.diff`'s sidecar, f7b7cd1b...), and nothing is unstaged. The
# screen's `reading.txt` (sha256 f681a503...): baseline held-out 0.071732, K
# 0.7171, 6226944 rows; tempo gain +0.000031, s 0.000001, m 0.000002, kept;
# the fitted pair mg 10 / eg 1, spread 9, g 3. The corpus `locate` recorded is
# sha256 2e0e3de9..., CORPUS_SHA's. The candidate fit
# `.tuning/s136/screen/full_tempo.hpp` is sha256 73f17707.... The guard as
# landed: its red observed first (`CHECK_MESSAGE(tempo_unfitted)` and the
# phase 5 b pin at 48/24), the bound 90/24 = 3.75, both worst clauses past
# 3.675, each pin past 69/24 + 1e-6, the four pins re-drawn by S083's pick over
# S082's views at 3.75 (phases 5 b, 5 w, 13 b, 13 w), the views reaching 90/24
# (DEC-322 (3) does not arise); fast-checked twice, the second check's three
# fixes applied. `plan.md`'s Open list re-read: S136 is first. The A/A rule:
# `fastchess.sh` is unchanged since S296's run (`b477459`..`ba65b86`), the
# same fastchess binary, the same `noob_3moves.epd` (sha256 2011193b...),
# the same machine, so no A/A is owed. No BUGS-scoped finding is open. The run
# starts at 13:15, in the day: no other work needs the machine meanwhile
# (DEC-155's night rule is for a run that would displace one).
# Re-launched by the coordinator 2026-10-28 00:40, nothing else changed but
# this sentence: the first launch (13:15, out `sprt_s136_20261027_131508`) was
# stopped by SIGINT at 16:17 for the owner's shutdown, after 6520 games and no
# bound, and is void, never pooled or resumed (`adocs/status.md` 2026-10-27
# 16:20). Same kernel (7.2.6), fastchess, book and harness, so no A/A is owed;
# the worktree equalled `.tuning/coord/S136/s136b_pause.diff` (sha256
# 1378ae95...) before this edit. A night run, as DEC-155 prefers.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the fitted pair is accepted as gaining: H1's 5
#                   nElo preferred to H0's 0 at alpha = beta = 0.05. **Nothing
#                   is committed until the owner has ruled on `EXACT_FLOOR`
#                   (DEC-323)**, read on this tree: 156 shipping and 156
#                   weakened against 159, where the tree before the paste ships
#                   160 and 159 weakened; of the 10 mined rows lost at depth
#                   10, 8 come back one or two plies later and 2 are DEC-087
#                   (e)'s and DEC-158's price as DEC-319 (2) reads them, none a
#                   rule no decision prices (`adocs/data/S136_lost_mates.txt`);
#                   the recommendation put to the owner is 159 -> 155, one
#                   below the shipping count (DEC-313 (1)'s rule), recorded as
#                   not separating, `MATE_IN_THREE_FLOOR` carrying the
#                   separation (13 against 10). Every commit is green, so the
#                   tree lands only once the ruling leaves no red; it **ships**
#                   with the guard as re-derived, the closing commit makes
#                   src/evaluation.hpp's tempo comments true, and specs.md's
#                   "absent, evaluation" row loses tempo. The stopping run's
#                   Elo is upward-biased (DEC-063) and is not the effect size.
#
#   H0           -> not shown to gain. **Nothing of the paste is committed**:
#                   the worktree's diff -- the weights, the goldens it moved,
#                   the guard's re-targeting and the comments -- is the record
#                   (`candidate.diff`), REF's tempo-zero tree stays, the
#                   `EXACT_FLOOR` question vanishes, and tempo is **recorded as
#                   zero** with its screen and match figures. No bisection: a
#                   family of one has no parts.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   not committed, as H0 is, and recorded as **unresolved**,
#                   the second time after S027's -- not converted to zero
#                   (S136's accepts, DEC-092). The term stays at zero.
#
# THE COMMIT THAT CLOSES THE VERDICT -- the record commit on H0 or no verdict,
# and on H1 whichever commit the coordinator closes it with after the ruling,
# green -- carries DEC-220's block -- `SPRT | cand <REF>+<hex> vs ref <REF>,
# 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`, `LLR`, `Games`,
# `Wall` and `Log` lines -- with the log trimmed into `adocs/data/S136_sprt.log`
# by `tools/trim_console.py` and the run's `candidate.diff` committed beside it
# as `adocs/data/S136_candidate.diff` (DEC-254: `tools/gate.sh` and
# `tools/ledger.py` find it by its blob id, the `<hex>`); `tools/ledger.py`
# regenerates plan.md's ledger table from it.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, and this script execs it and adds
# nothing after.

set -uo pipefail

fail()
{
  echo "SPRT-RUN-FAILED: $*" >&2
  exit 1
}

# THE BOOKED SETTINGS, AND NOTHING ELSE FROM THE CALLER'S ENVIRONMENT, as
# S083_sprt.sh's same block reads them: pinned, refused if set to anything
# else, and passed on the exec line; CAND, ROUNDS, BARE_FEN, SRAND and AA
# refused if set at all -- CAND above all, which would turn the run into a
# commit's and leave the tree unplayed; RELAY_PYTHON unset; OUT overridden.
ref="ba65b86db8c8626e8906e2b70ee7ca1dc8339725"
worktree="/home/max/ws/chesso-s136b"
# The src/ REF must carry: S136's phase 1 start, the tree that ships tempo at
# zero, which every reading of the paste was taken on.
readings_src="80520d7"
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"
sprt_rounds="20950"

# THE COORDINATOR'S SENTENCE AND REF. While the bracketed marker is anywhere
# in this file the pre-registration is unfinished and the run does not start.
# The marker is assembled in two halves so this line does not match itself.
# Checked before the environment, because an unpinned sha would read there as
# a mismatch and name the wrong fault.
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

# THE PAIR: the worktree's HEAD is REF, so the reference is the tree the
# candidate's diff is taken against, and the run measures S136's paste and
# nothing else.
if [[ "$(git rev-parse HEAD)" != "$(git rev-parse "$ref")" ]]; then
  fail "$worktree's HEAD is $(git rev-parse --short HEAD), not REF" \
       "$(git rev-parse --short "$ref") -- the pair would measure more than S136"
fi

# REF's engine is the one that ships tempo at zero: S136's phase 1 changed
# nothing under src/, so a documents-only descendant of 80520d7 passes and a
# tree with any other src/ change does not -- it would put the paste on
# another search, and every reading of it was taken on this one.
if ! git diff --quiet "$readings_src" "$ref" -- src/; then
  fail "REF's src/ differs from $readings_src's -- REF is not the shipped" \
       "engine or a documents-only descendant of it"
fi

# THE TREE IS DIRTY: the candidate is the uncommitted change, so a clean tree
# would be an A/A. `git status --porcelain` counts staged, unstaged and
# untracked alike, as fastchess.sh's own test does through a scratch index
# (which names the candidate); this one only refuses early.
[[ -n "$(git status --porcelain)" ]] \
  || fail "$worktree is clean: there is no candidate change to play"

# ONLY THE STAGED PASTE IS PLAYED. fastchess.sh builds and diffs the working
# tree, staged, unstaged and untracked alike; the paste is the staged diff the
# fast check read, so an unstaged edit under src/, tests/, cmake/ or the root
# CMakeLists.txt -- the build's inputs outside src/: CMakeLists.txt includes
# cmake/arch.cmake and cmake/pgo.cmake -- or an untracked file under src/ or
# cmake/, would put something nobody read into the candidate or its recorded
# diff.
git diff --quiet -- src tests cmake CMakeLists.txt \
  || fail "unstaged changes under src/, tests/, cmake/ or CMakeLists.txt:" \
          "$(git diff --name-only -- src tests cmake CMakeLists.txt |
             head -5 | tr '\n' ' ')"
untracked="$(git ls-files --others --exclude-standard -- src cmake | head -5)"
[[ -z "$untracked" ]] || fail "untracked files under src/ or cmake/: $untracked"

# THE BUILD IS REF'S: the paste moves no build input, so cmake/ and the root
# CMakeLists.txt are REF's in the tree. Any change there would build the
# candidate with other flags than the reference.
git diff --quiet "$ref" -- cmake CMakeLists.txt \
  || fail "cmake/ or CMakeLists.txt differs between the tree and REF:" \
          "$(git diff --name-only "$ref" -- cmake CMakeLists.txt |
             head -5 | tr '\n' ' ')"

# The paste is an `--only tempo` fit: the tables cannot have moved, under
# src/ only src/evaluation.cpp may differ from REF, and there only the two
# tempo definitions outside whole-line comments. Against REF's commit, the
# tree as staged (the check above has refused anything unstaged).
if ! git diff --quiet "$ref" -- src/eval_tables.hpp; then
  fail "src/eval_tables.hpp differs between the tree and REF -- not a" \
       "tempo-only paste"
fi
if git diff --quiet "$ref" -- src/evaluation.cpp; then
  fail "src/evaluation.cpp is the same in the tree and at REF -- not S136's paste"
fi
if [[ "$(git diff --name-only "$ref" -- src)" != "src/evaluation.cpp" ]]; then
  fail "the tree changes more under src/ than src/evaluation.cpp"
fi
code_lines="$(git diff -U0 "$ref" -- src/evaluation.cpp |
              grep -E '^[-+]' | grep -vE '^(\+\+\+|---) ' | cut -c2- |
              grep -vE '^[[:space:]]*(//.*)?$')"
if [[ -z "$code_lines" ]] ||
   grep -qvE '^constexpr int tempo_(mg|eg) = -?[0-9]+;$' <<< "$code_lines"; then
  fail "src/evaluation.cpp differs outside whole-line comments in more than" \
       "the two tempo definitions -- not a tempo-only paste"
fi

# BUILD/ WAS BUILT FROM THIS TREE. fastchess.sh plays build/src/chesso as it
# finds it. Checked two ways, S083_sprt.sh's: `cmake --build` of the engine
# target first, so a stale binary is rebuilt from the tree before the snapshot
# rather than trusted; then no file of the engine's sources -- src/, cmake/ or
# a CMakeLists.txt -- may be newer than the binary, which a build that failed
# part way, or a tree edited after it, would leave behind.
binary="$worktree/build/src/chesso"
cmake --build "$worktree/build" -j "$concurrency" --target chesso > /dev/null \
  || fail "cmake --build $worktree/build --target chesso failed"
[[ -x "$binary" ]] || fail "no engine at $binary after the build"
newer="$(find "$worktree/src" "$worktree/cmake" "$worktree/CMakeLists.txt" \
           -newer "$binary" -type f 2> /dev/null | head -5)"
[[ -z "$newer" ]] || fail "sources newer than $binary: $newer"

# THE BOOK: books/*.epd is gitignored, so a fresh worktree has none (S111's
# first launch refused on it, 2026-10-21).
[[ -s books/noob_3moves.epd ]] \
  || fail "no books/noob_3moves.epd in $worktree; copy the main checkout's"

[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh"

# THE HARNESS: fastchess.sh must read both names, or it ignores them and plays
# its own `{0, 5}` to 20000 rounds.
for name in SPRT_BOUNDS SPRT_ROUNDS; do
  if ! grep -qE '\$\{?'"$name"'([^A-Za-z0-9_]|$)' ./fastchess.sh; then
    fail "./fastchess.sh does not read $name, so this run's bounds or cap" \
         "would be ignored -- see THE HARNESS in adocs/data/S283_sprt.sh"
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s136_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
