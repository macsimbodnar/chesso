#!/usr/bin/env bash
#
# S083: THE CORPUS THAT SHIPS -- the evaluation constants fitted on the corpus
# S083's curve picks, against the constants that ship, one SPRT at the
# harness's 8+0.08, Hash 16, {0, 5} nElo: the only corpus SPRT S082 and S083
# owe (DEC-259).
#
#   cd /home/max/ws/chesso-s083s &&
#     nohup adocs/data/S083_sprt.sh > /home/max/ws/chesso/.tuning/s083_sprt.log 2>&1 &
#
# Written 2026-10-22 in S083's phase 1, blind (DEC-260: S111's SPRT held the
# machine), in adocs/data/S110_sprt.sh's shape, whose harness section -- and
# S283_sprt.sh's before it -- this file relies on and does not repeat; switched
# to WORKING-TREE MODE at S083's re-landing by DEC-307 (3), in
# adocs/data/S111_sprt.sh's form (DEC-303's), and moved to the re-landing on
# S292's and S293's guards after S289's guard read H0 (DEC-309; the first
# re-landing, on cd50178's tree, is committed as a record only, its readings
# adocs/data/S083r_*): the re-landing is not committed
# while the five rulings the owner makes at H1 stand (DEC-307 (4), DEC-308) --
# `EXACT_FLOOR`, `MATE_IN_THREE_FLOOR`'s separation, the ply-2 case's
# first-iteration clause, the multicut mined row of "pruning does not hide a
# forced mate" (S291's attribution, DEC-311 (1)) and test_tuner_gradient's
# gauge-walk floor -- so the run plays
# the uncommitted tree of the worktree `/home/max/ws/chesso-s083s` against its HEAD, fastchess.sh's
# dirty-tree mode (S256, DEC-254): the candidate is named `cand-<HEAD>+<hex>`
# and the run saves its diff as `$OUT/candidate.diff`. Nothing here was run:
# every figure below is read from a file or a git object and names it. The
# coordinator pins REF and fills the bracketed sentence below on the day, and
# until then the script refuses to start (the self-check at the bottom).
#
# THE PAIR.
#
#   CAND  the working tree of `/home/max/ws/chesso-s083s`, uncommitted: its
#         HEAD (REF) plus S083's re-landing -- `adocs/data/S083_corpus.sh
#         apply` pasted the candidate's fit, the top rung of the stage S083's
#         reading stopped at (stage 0, `fit_6226944_s1.hpp`, sha256
#         f7ccdb7f..., every game of S082's training side at 46 rows a game,
#         leaf qs, S082's outcome (a), fitted with S082's compare settings on the
#         fixed validation set), its queen re-anchored first by
#         `.tuning/reanchor.py <fit> <out> queen -273` (DEC-059's form, DEC-307
#         (1): `QUEEN` 710 -> 983 and -273 on all 128 queen squares, sha256
#         4cf27569...), over src/eval_tables.hpp and src/evaluation.cpp;
#         `CorrHistMax`'s declared top moved from 94 to 84 with `PAWN` (DEC-307
#         (2), a tune-build bound); every golden inside DEC-299's guard
#         re-derived by its own script and applied (DEC-142, DEC-307 (3)); the
#         five rulings left as they read -- `EXACT_FLOOR` red,
#         `MATE_IN_THREE_FLOOR` green and not separating, the ply-2
#         clause, the multicut row (no separating candidate on cd50178's
#         tree, reverse futility's price by S291's attribution, not re-mined)
#         and the gauge-walk floor (no input to re-pick). Every golden was
#         read again on this tree, adocs/data/S083s_*. Only the staged diff is
#         played: the guards below refuse unstaged changes under src/ or
#         tests/ and untracked files under src/. Built in that worktree's
#         build/. Bench 6042933 -> 3945487.
#   REF   the worktree's HEAD, which the coordinator pins below: a commit
#         whose src/ is `ad0cfc7`'s -- S292's narrowed null-move guard
#         (`e27d3e5`, DEC-310) and S293's reverse futility whole-search guard
#         (`ad0cfc7`, DEC-312) on the constants that ship, S076's fit on
#         S065's deduplicated root corpus (`77d7450`), re-expressed
#         bit-exactly by S134's fold (DEC-090) and fitted by nothing since;
#         bench 6042933. The worktree was made at `e972bd4`, the commit of the
#         first re-landing's record, whose src/ is ad0cfc7's. Every reading of
#         this re-landing was taken on that src/, so the guard below refuses a
#         REF whose src/ differs from it: any later engine change puts the
#         paste on another search, and the readings are taken again first
#         (DEC-307's consequence) and this check moves with them.
#
# The two differ by S083's re-landing alone, which the guards below check: the
# worktree's HEAD is REF, the tree is dirty, src/eval_tables.hpp and
# src/evaluation.cpp differ from REF's, and build/'s binary is no older than
# any source it is built from.
#
# WHAT THE VERDICT PRICES, AND WHAT IT CANNOT ATTRIBUTE, stated before a game.
# The candidate differs from REF's constants in the corpus's leaf, its density,
# its size and its engine, and is a fresh 5000-epoch fit with its own K: the
# package DEC-259 names, "the corpus that ships", and nothing inside it is
# priced apart. Two things travel with it. The scale: the fit holds the
# material-and-table gauges, not the evaluation's overall level, and S082's
# 1 M-row qs fit already moved the defines (PAWN 94 -> 86, KNIGHT 327 -> 374,
# BISHOP 308 -> 351, ROOK 487 -> 512, QUEEN 716 -> 721,
# `.tuning/s082/fit_qs_46_s1_qs.hpp`), while the search's margins in
# evaluation units were fitted on the old scale (S261's lane); an H0 may be
# that and not the corpus, which S127's lane exists to re-fit. And the
# refit: S075's lambda-0 control priced 5000 more epochs on one corpus at
# 3e-06 of held-out error, so the epochs are not the reason, whatever this
# reads.
#
# WHY THESE BOUNDS AND THIS CONTROL. 8+0.08: DEC-286 kept the harness's control
# for every verdict but the correction family's (DEC-297), and this is an
# evaluation refit; DEC-202's longer-control reading leaves evaluation weights
# out by name (fastchess.sh's own comment). `{0, 5}` nElo at alpha = beta =
# 0.05: plan.md's "How a change is measured" -- an evaluation family's
# survivors take one `{0, 5}` SPRT (DEC-259 (b)) -- and the question the step
# asks, whether the new corpus's constants gain over the ones that ship.
# Rejected: `{-5, 5}`, S076's form for a refit of unknown sign, 10465 games
# worst case -- it would ship a zero half the time, and every later evaluation
# family fits on what this verdict ships; `{0, 10}` -- a resolution no
# measurement here asks for, and a true gain under about 5 nElo read as H0
# more often.
#
# WHAT THE PAIR COSTS (DEC-143): **41861 games** with the truth at the
# interval's midpoint, 19.84 h at the 2110 games an hour `.moltke.local.md`
# budgets 8+0.08 on `noob_3moves.epd` from (DEC-190); **25591 games** with it
# on a bound, 12.13 h (plan.md's "What the formula says before a run").
#
# THE CAP. **20950 rounds, 41900 games**, 19.86 h at 2110 an hour: the midpoint
# figure rounded up to a whole hundred, S283's and S110's rule, in place of
# fastchess.sh's 20000-round default, which a truth at the midpoint meets
# before it meets a bound. Four hours or more, so the run starts by DEC-155's
# night rule. Watcher ceiling 144000 s (40 h), twice the cap's hours and the
# abort below, wall-clock only while the machine is awake.
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
#   candidate  e972bd4  2026-10-23  + uncommitted changes
#   diff       /home/max/ws/chesso/.tuning/sprt_s083_<stamp>/candidate.diff  blob <12 hex>
#   reference  e972bd4  2026-10-23
#   config     arch native  tune off
#   padding    cand on  ref on
#   tc 8+0.08  hash 16  concurrency 12 of 12 cores
#   book       noob_3moves.epd
#   seed       <the run's stamp, digits only>
#   bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05
#   cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
#   out        /home/max/ws/chesso/.tuning/sprt_s083_<stamp>
#
# with no `relay` or `A/A CALIBRATION` line, fastchess's engine names
# `cand-e972bd4+<the same 12 hex>` and `ref-e972bd4`, the candidate's `id name`
# carrying `-dirty`, and fastchess's first `Started game 1 of N` line reading
# N = 41900.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171):
# Pinned by the coordinator 2026-10-23 21:30: REF `e972bd4` (committed 2026-10-23
# 20:47, S083's cd50178-tree record on top of S293's H1 and ledger), the
# worktree's HEAD, its src/ `ad0cfc7`'s: S292's and S293's guards on the shipped
# weights, bench 6042933. The candidate is the staged re-landing, compared byte
# for byte with the diff the fast check read
# (`.tuning/coord/S083/s083_reland2.diff`, sha256 9c48cb3a...), bench 3945487,
# nothing unstaged or untracked beside it but this pin. The Open list at
# `e972bd4` re-read: one DEC-171 filler is open, **S290** (the S170 budget
# picker lives outside the tree), tooling only, reachable in no game, so it
# cannot move this run. The harness is S293's run's of 2026-10-23 -- fastchess
# alpha 1.8.1 20260720-daa3ea2, `noob_3moves.epd`, `fastchess.sh` last changed
# `769ce7a` (S284), the same workstation and kernel 7.2.6 -- so no A/A is owed
# (DEC-143). Before the run: both fast suites 46 of 50, the four reds the
# rulings name, values as read; format and the prose checks clean; the dry copy
# printed DRY-RUN-WOULD-EXEC. The run starts at once, the night's run (DEC-155).
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> at 8+0.08 the new corpus's constants are accepted as
#                   gaining: H1's 5 nElo preferred to H0's 0 at alpha = beta =
#                   0.05. **Nothing is committed until the owner has ruled
#                   (DEC-307 (4), DEC-308)** on the five rulings this tree
#                   carries, each read on this tree (adocs/data/S083s_*):
#                   `EXACT_FLOOR` (159 shipping and 154 weakened against
#                   163, where the tree before the paste ships 163),
#                   `MATE_IN_THREE_FLOOR`'s separation (12 shipping, 11
#                   weakened, against 11), the ply-2 case's first-iteration
#                   clause (`mate -2` first at 6 against 4), the multicut
#                   mined row (no mate at depth 10; reverse futility at the
#                   mated side's node, DEC-158's price by S291's
#                   attribution, DEC-311 (1); not re-mined) and
#                   test_tuner_gradient's gauge-walk floor (`psqt_eg[king]`
#                   0.0587 under AdaGrad against 0.1). Every commit is green, so the landing is
#                   committed only once the rulings leave none of the four
#                   red; then the corpus becomes the one every later
#                   evaluation fit and screen trains on (S135 on, S126). The
#                   stopping run's Elo is upward-biased (DEC-063) and is not
#                   the effect size.
#
#   H0           -> not shown to gain. **Nothing is committed** and S083
#                   completes as zero, the played diff kept as the record;
#                   REF's constants stay. The corpus is kept on disk and
#                   archived (DEC-235). Which corpus the later families screen
#                   on is the coordinator's decision, recorded at S135's start
#                   with this verdict beside it: this run prices the refit of
#                   all 823 constants, not the corpus as a training set for one
#                   term with every other frozen.
#
#   No verdict   -> the 41900-game cap, or the 40 h abort, with neither bound:
#                   read as H0 is -- not committed, recorded as zero, with the
#                   games and the interval -- and no follow-up run (DEC-063).
#
# THE COMMIT THAT CLOSES THE VERDICT -- the record commit on H0 or no
# verdict, and on H1 whichever commit the coordinator closes it with after
# the rulings, green -- carries DEC-220's block -- `SPRT | cand e972bd4+<hex> vs
# ref e972bd4, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo` and the `Elo`,
# `LLR`, `Games`, `Wall` and `Log` lines -- with the log trimmed into
# `adocs/data/S083_sprt.log` by `tools/trim_console.py` and the run's
# `candidate.diff` committed beside it as `adocs/data/S083_candidate.diff`
# (DEC-254: `tools/gate.sh` and `tools/ledger.py` find it by its blob id, the
# `<hex>`); `tools/ledger.py` regenerates plan.md's ledger table from it.
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
# S283_sprt.sh's same block reads them: pinned, refused if set to anything
# else, and passed on the exec line; CAND, ROUNDS, BARE_FEN, SRAND and AA
# refused if set at all -- CAND above all, which would turn the run into a
# commit's and leave the tree unplayed; RELAY_PYTHON unset; OUT overridden.
ref="e972bd44b07bcd7c47300002b90370e388754204"
worktree="/home/max/ws/chesso-s083s"
# The src/ every reading of the re-landing was taken on: S293's guard commit,
# whose src/ e972bd4 carries unchanged (S292's guard beneath it).
readings_src="ad0cfc7"
tc="8+0.08"
hash="16"
concurrency="12"
sprt_bounds="elo0=0 elo1=5 alpha=0.05 beta=0.05"
sprt_rounds="20950"

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

# THE COORDINATOR'S SENTENCE AND REF. While the bracketed marker is anywhere in
# this file the pre-registration is unfinished and the run does not start. The
# marker is assembled in two halves so this line does not match itself.
self="${BASH_SOURCE[0]}"
marker='[COORD'"INATOR"
grep -qF -- "$marker" "$self"
case $? in
  0) fail "$self still holds the coordinator's unfilled marker ($marker" \
          "...]); pin REF and fill the sentence before the run" ;;
  1) ;;
  *) fail "could not read $self to check for the coordinator's marker" ;;
esac

cd "$worktree" || fail "cd $worktree"

# THE PAIR: the worktree's HEAD is REF, so the reference is the tree the
# candidate's diff is taken against, and the run measures S083's re-landing
# and nothing else.
if [[ "$(git rev-parse HEAD)" != "$(git rev-parse "$ref")" ]]; then
  fail "$worktree's HEAD is $(git rev-parse --short HEAD), not REF" \
       "$(git rev-parse --short "$ref") -- the pair would measure more than S083"
fi

# REF'S ENGINE IS THE ONE THE READINGS WERE TAKEN ON: src/ as at ad0cfc7,
# S292's and S293's guards. A REF whose src/ differs -- any later engine
# change -- puts the paste on another search, and every golden and floor in
# S083's re-landing (adocs/data/S083s_*) was read on this one.
if ! git diff --quiet "$readings_src" "$ref" -- src; then
  fail "REF's src/ differs from $readings_src's, where S083's re-landing was" \
       "read; take the readings again on REF first (DEC-307's consequence)"
fi

# THE TREE IS DIRTY: the candidate is the uncommitted change, so a clean tree
# would be an A/A. `git status --porcelain` counts staged, unstaged and
# untracked alike, as fastchess.sh's own test does through a scratch index
# (which names the candidate); this one only refuses early.
[[ -n "$(git status --porcelain)" ]] \
  || fail "$worktree is clean: there is no candidate change to play"

# ONLY THE STAGED LANDING IS PLAYED. fastchess.sh builds and diffs the working
# tree, staged, unstaged and untracked alike; the landing is the staged diff
# the fast check read, so an unstaged edit under src/ or tests/, or an
# untracked file under src/, would put something nobody read into the
# candidate or its recorded diff.
git diff --quiet -- src tests \
  || fail "unstaged changes under src/ or tests/:" \
          "$(git diff --name-only -- src tests | head -5 | tr '\n' ' ')"
untracked="$(git ls-files --others --exclude-standard -- src | head -5)"
[[ -z "$untracked" ]] || fail "untracked files under src/: $untracked"

# The paste changed the evaluation's constants: the tree must differ from REF
# in both files the tuner's header names, or the candidate is not the one this
# file books. Against REF's commit, staged and unstaged alike.
for file in src/eval_tables.hpp src/evaluation.cpp; do
  if git diff --quiet "$ref" -- "$file"; then
    fail "$file is the same in the tree and at REF -- not S083's paste"
  fi
done

# BUILD/ WAS BUILT FROM THIS TREE. fastchess.sh plays build/src/chesso as it
# finds it. Checked two ways: `cmake --build` of the engine target first, so a
# stale binary is rebuilt from the tree before the snapshot rather than
# trusted; then no file of the engine's sources -- src/, cmake/ or a
# CMakeLists.txt -- may be newer than the binary, which a build that failed
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
     OUT="/home/max/ws/chesso/.tuning/sprt_s083_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
