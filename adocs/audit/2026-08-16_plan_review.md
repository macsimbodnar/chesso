# Audit 2026-08-16 plan_review

Scope: `adocs/plan.md`, the 22 pending step files in `adocs/plan_todo/`
(`plan_current/` holds only `.gitkeep`), `adocs/status.md`, and `adocs/specs.md`
and `adocs/decisions.md` where they constrain a pending step, at commit
`02bb6a5fb190329f2f1edd06f515cb0028e16e69` (branch `achesso`, tree clean apart
from this report and the pre-existing untracked `adocs/eval_tuning_strategy.md`).
A pending step's claims about the code are in scope and were checked against the
code at this commit. Out of scope: `adocs/plan_done/` except as evidence about a
pending step, and code review unconnected to a pending step.

Method: every `file:line` citation in every pending step file resolved against
HEAD by hand; every numeric claim in a pending step re-measured from the tool the
step names. What was run: `cmake --build build -j12`, `ctest --test-dir build -L
fast` (12/12, 22.19 s), `build/tools/eval_spread` over the tracked corpus, over
200000 rows of `.tuning/selfplay_v2.tsv` and over all 11003693 of it,
`tools/search_bench.py ./build/src/chesso 9`, a probe build with
`-DCMAKE_CXX_FLAGS=-DRFP_MARGIN=75`, `stockfish dev-20260810-5062aee5` on one
FEN, and a Python pass over `plan.md`'s list entries against the three plan
directories. The workflow checker's own source was read read-only at
`~/.claude/plugins/cache/moltke/moltke/0.11.0/bin/moltke.py`. Prior findings in
the four tracked reports under `adocs/audit/` were each re-measured from their
own reproduction; verdicts at the end.

Nothing in the repository was modified except this file. Scratch files were
written only under the session scratchpad outside the tree. No test was added:
every finding below is a defect in a plan document with a reproduction that runs
from a clean checkout, and the one test defect recorded (F06) is demonstrated by
`stockfish` refusing the position, so none of them needs a new test to be
demonstrable.

Written before any fix. A report edited while fixing stops being evidence of
what was found.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-08-16_plan_review-F<nn>`. The example below writes that prefix as
`<report>`: a fenced example carrying this report's real stem cannot be told
apart from a real finding that a fence has swallowed, which is INV-14 (S049).

```
### <report>-F01  high  short title

Status: open

Evidence: file and line, or the command and its output.
Impact: what breaks, for whom, under what conditions.
Suggested resolution: what would close it. Not applied here.
```

Every finding ends in one of two places: a plan step whose `closes:` field
names it, or a decision entry stating why it will not be acted on, which moves
it to `accepted`. A finding moves to `closed` only after the audit is re-run
and no longer reports it. Fixing without re-running leaves it `planned`.

## Findings

### 2026-08-16_plan_review-F01  high  S057 is obsolete in every particular: the corpus it says is missing exists, and both its figures and S039's understate the measured spread by four times

Status: planned — S080

**Evidence.** S057 exists to give S039 "a corpus that exists in this tree, with
its spread figures re-measured at HEAD"
(`adocs/plan_todo/S057_s039_corpus_that_exists.md:2`). Every load-bearing claim
in it is false at this commit.

*The corpus exists.* `S057:17-20` says the corpus "does not exist here:
`.tuning` is gitignored (`.gitignore:11`) and `DEV_MANUAL.md:512` says so. The
work moved machines at DEC-049 and the dataset did not come with it."

```
$ ls -la .tuning/selfplay_v2.tsv
-rw-rw-r-- 1 max max 715409623 Aug 14 08:13 .tuning/selfplay_v2.tsv

$ head -1 .tuning/selfplay_v2.tsv
rnbqkbn1/ppppp1p1/7r/7p/P3Pp2/N5P1/1PPP1P1P/R1BQKBNR w KQq - 1 5	1.0	150	24
```

S065 regenerated it (DEC-055) and `DEV_MANUAL.md:254-255` documents the exact
run S039 asks for, against that file:

```
build/tools/eval_spread --data .tuning/selfplay_v2.tsv            # 11.0 M positions, 18.6 s
build/tools/eval_spread --data .tuning/selfplay_v2.tsv --limit 200000
```

The FEN is the first tab-separated field, so no conversion is needed — the
conversion `S057:42-46` calls "not optional" is needed only for the 5582-row
tracked corpus S057 chose instead. `DEV_MANUAL.md:508-512`, the line S057 cites,
now says the opposite of what S057 draws from it: it scopes the non-existence to
`selfplay_v1.tsv` and distinguishes it from v2.

*S057's replacement figures are stale.* `S057:31-40` prints an `eval_spread` run
"Re-measured at HEAD". Re-run verbatim at this commit:

```
$ tail -n +2 adocs/data/S028_raw.tsv | cut -f9 > /tmp/.../s028_fens.tsv
$ build/tools/eval_spread --data /tmp/.../s028_fens.tsv
  positions  5582
  150            40 0.717%     17 0.305%     96 1.720%
  200             0 0.000%      4 0.072%     31 0.555%
  mobility      182  8/8/2K5/8/5q2/8/6k1/7q b - - 1 75
  king safety   222  4r1k1/5ppp/p1b5/2bR4/2P5/1P1Q4/4K3/1R4q1 w - - 0 36
  combined      326  r1b2Q2/1pkr3p/2nq4/1R6/p3B3/3P2P1/P4P1P/2R3K1 w - - 5 27
```

against S057's recorded `29 0.520%` / `2 0.036%` / worst combined `242`. The
combined tail is 3.3 times what S057 records. Cause: S057 was written on
2026-08-13 and S065 (`33aa3b4`, 2026-08-15) refitted 827 constants.

*The real corpus is worse still.* Over all 11003693 rows at HEAD:

```
$ build/tools/eval_spread --data .tuning/selfplay_v2.tsv
  positions  11003693
                 mobility  king safety     combined
  p99.9               176          164          233
  max                 267          363          489
  150         51006 0.464%  20565 0.187% 171424 1.558%
  200          1491 0.014%   2068 0.019%  32524 0.296%
  300             0 0.000%     17 0.000%    995 0.009%
  400             0 0.000%      0 0.000%     22 0.000%
```

*S039's own figures are stale by the same refit, and the shipped comment's
headline claim is now false.* `S039_lazy_margin_redecide.md:28-39` records
"0.364 %" past 150 and worst 279, "so `evaluate()` discards up to 129 cp".
Measured: 1.558 % and 489, so up to **339 cp** is discarded, on four times as
many positions. And `src/evaluation.hpp:274-278` says

> 150 is above the largest correction observed over 149084 positions of S028
> self-play, where the tapered mobility term ran p50 19, p95 60, p99 81,
> p99.9 107 and a maximum of 143 centipawns.

Mobility alone now reaches **267** and king safety alone **363**. S039's "What
is stale" section names only the king-safety-at-zero-weight sentence as false;
the sentence above it is false too and S039 does not say so.

**Impact.** S057 sits at list entry 57, immediately ahead of S039 at 58, and its
job is to be the corrected evidence S039 executes against. Executing S057 as
written points S039 at a 5582-row tracked file and copies in figures 3.3 times
too small, when an 11-million-row corpus documented in `DEV_MANUAL.md` is on
disk. S039's decision is where to put `LAZY_EVAL_MARGIN`; a margin chosen from a
0.5 % tail when the real tail is 1.6 %, and from a worst case of 242 when it is
489, is chosen from the wrong distribution. S057's `accepts:` does say "figures
re-measured at the commit S057 completes on", so a session that re-measures
recovers — but the body reads as already-done work and the whole "What is there"
section argues for the smaller corpus on a premise that no longer holds.

**Suggested resolution.** Rewrite S057 against this commit: name
`.tuning/selfplay_v2.tsv` with the `DEV_MANUAL.md:254` command, state that it is
gitignored and that a machine move loses it again (which is what the tracked
corpus is a fallback for, not a first choice), and replace both its figures and
S039's with the run above. Add to S039's `accepts:` that
`src/evaluation.hpp:274-278`'s mobility figures are re-measured, not just the
king-safety sentence — the comment's leading claim is the false one.

### 2026-08-16_plan_review-F02  medium  17 file:line citations across six pending step files no longer resolve, and 9 of them are outside the reach of S063, the step created to fix exactly this

Status: planned — S079

**Evidence.** Every `file:line` citation in the 22 pending step files was
resolved against HEAD. Seventeen miss. The 2026-08-13 re-run found three
(`2026-08-13_plan_review.2-F08`), S063 was created for them, and the set has
grown almost six-fold since — `6bd650e` (S033) moved `src/search.cpp` by 57
lines and `33aa3b4` (S065) moved `tests/test_eval_model.cpp` by about 20.

| step file | citation | what is at HEAD | what was meant |
|---|---|---|---|
| S024:16 | `src/search.cpp:507` counter-move write | `if (reduction < 0) { reduction = 0; }` | `:564` |
| S024:17 | `src/evaluation.cpp:1091-1094` | `:1091` is `killer_moves[1]` | `:1093-1096` |
| S030:21 | `src/evaluation.cpp:1092,1097` | both blank | `:1094`, `:1099` |
| S039:20 | `src/evaluation.cpp:731-734` | a collinearity comment | `:733-736` |
| S056:17 | `tests/test_eval_model.cpp:381-422` | mid-comment through the loop body | `:401-442` |
| S056:18 | `:412` `CHECK_MESSAGE(difference > 2.0` | blank | `:432` |
| S056:19 | `truncation_positions` at `:215-220` | inside `positions` | `:227-232` |
| S056:20 | `:420` `CHECK_MESSAGE(worst > 2.8` | `REQUIRE(load_FEN(fen, &game))` | `:440` |
| S056:32 | `:388-391` tempo precondition message | a comment about the 2.0 threshold | `:408-411` |
| S056:34 | `:366-373` "the old 2.0" comment | the tempo/counts comment | `:378-385` |
| S057:17 | `DEV_MANUAL.md:217` eval_spread input | a node-count paragraph | `:254` |
| S057:18 | `DEV_MANUAL.md:512` | mid-sentence, and about v1 only | `:508-512` |
| S057:22 | `DEV_MANUAL.md:400-401` `--games 20000 --nodes 100000 --seed 20260810` | `grep -c "^Finished game"` | **gone** — `grep -n "games 20000" DEV_MANUAL.md` returns nothing |
| S057:23 | `adocs/status.md:37` "95 minutes" | DEC-033 prose | `:57` |
| S058:27 | `src/search.cpp:503` history write | the LMR guard | `:560` |
| S058:3, :28 | `src/search.cpp:507` counter-move write | `if (reduction < 0)` | `:564` |
| S061:23 | `tests/test_evaluation.cpp:643-704` "bands are strictly ordered" | case starts `:652`, ends `:713` | `:652-713` |

Verified with, among others:

```
$ grep -n "MOVE_PIECE" src/search.cpp
src/search.cpp:560:            state->history_moves[MOVE_PIECE(moves[i])][MOVE_TO(moves[i])];
src/search.cpp:564:          state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)] =

$ grep -n "difference > 2.0\|worst > 2.8\|truncation_positions" tests/test_eval_model.cpp
227:static const std::vector<std::string> truncation_positions = {
418:    for (const std::string& fen : truncation_positions) {
432:      CHECK_MESSAGE(difference > 2.0,
440:    CHECK_MESSAGE(worst > 2.8, ("worst pinned difference " +
```

S063's `touches:` is `adocs/plan_todo/S024_...`, `S030_...`, `S039_...` and its
`excludes:` forbids editing anything else, so the nine misses in S056, S057, S058
and S061 cannot be fixed by it. Its `accepts:` asks that "every other file:line
citation in the pending step files still resolves, checked in the same pass" —
a check whose remedy its own `touches:` puts out of reach.

**The ordering makes one of these actively harmful.** S063 sits at list entry 49
and S058 at 62. S063 repairs S030's citations; S058 then rewrites S030's hazard
paragraph and its `accepts:` (`S058:3`) *requires* that paragraph to name "the
write at src/search.cpp:507". A session that satisfies S058 as written puts a
stale line number back into S030 after S063 has just removed one.

**Impact.** S030's whole neutrality argument is a list of pointers, and S058
exists because that list was wrong; a pointer that lands on unrelated code is
the reason a session rebuilds the site set by hand instead of trusting the file.
For S056 and S057 it is worse than cosmetic: their citations are the evidence
for what they instruct another step to write, and six of S056's six point into a
test file whose pinned corpus S065 replaced (see F03).

**Suggested resolution.** Widen S063 to every pending step file, or cite symbols
and test-case names rather than lines — `grep -n MOVE_PIECE src/`,
`CHECK_MESSAGE(difference > 2.0`, `TEST_CASE_FIXTURE(..., "bands are strictly
ordered")` are all stable where a line number is not. Whichever is chosen,
S058's `accepts:` must stop naming a literal line, since it is a step that
writes citations into another step.

### 2026-08-16_plan_review-F03  medium  S056's numeric evidence describes a pinned corpus S065 replaced, so the step that re-targets S055's thresholds carries thresholds measured against positions no longer in the test

Status: planned — S081

**Evidence.** S056 exists to re-target the four pinned thresholds S055 would
break. `S056:22-29`:

> Those four were pinned *because* they exceed 2.0 under today's three effective
> truncations. [...] the four merged disagreements are 1.8750, 1.3333, 1.2500
> and 1.1250

Those four values were measured on the pinned set as it stood on 2026-08-13.
S065 replaced the set outright. `tests/test_eval_model.cpp:222-232`:

```
// The subset of `positions` pinned for the truncation bound rather than for a
// feature count, in the order they are listed above. [...] S038, re-measured at S065.
static const std::vector<std::string> truncation_positions = {
    "8/5R2/2nk2K1/8/1r3P2/8/8/8 b - - 3 54",
    "r3r1k1/p5p1/1p1Pb2p/5P2/8/7P/P2N1B2/bN3RK1 w - - 0 24",
    "5rk1/5ppp/p1b4q/8/2QP2P1/5N1n/PP3P2/4RR1K w - - 2 26",
    "r1bq1rk1/1ppp1p1p/n4np1/pNP3N1/4p2P/4P3/PBPP1PP1/R2QKB1R b KQ - 5 9",
};
```

None of these is one of the four the 2026-08-13 audit measured
(`2r1r1k1/4Q1p1/...`, `6k1/6p1/...`, `r1bqkb1r/...`, `1k5r/pp3p2/...`), and the
distribution changed with them: `tests/test_eval_model.cpp:392` now records
"Every position pinned above sits at 69/24 = 2.875, all three losing their
maximum at once", where the old set ran 2.1250 to 2.8750.

```
$ git log --oneline -S'2r1r1k1/4Q1p1' -- tests/test_eval_model.cpp
33aa3b4 Land S065's fit with the queen re-anchored, DEC-059
b904d1b Complete S038: the model guard's slack matches the truncation arithmetic
```

The step's conclusion survives — `2 x 23/24 = 1.917 < 2.0`, so the merge still
falsifies `difference > 2.0` for every position — but the numbers it hands S055
to re-pin against do not, and neither do its six line citations (F02).

**Impact.** S056 sits at entry 60, S055 at 61. S056's output is the text S055's
completion gate is read from. A session that copies S056's four values into S055
writes thresholds derived from FENs the test no longer loads; the guard would
then either be pinned too loosely or fail on the first run, in the one test whose
purpose is to be a bound rather than a number someone widened
(`2026-08-13_adversarial-F04`, S038).

**Suggested resolution.** Re-measure the four merged disagreements on the
current `truncation_positions` and restate S056's paragraph against them, or
drop the four literals and have S056 require only that S055 re-measure and re-pin
in its own commit — which its `accepts:` already says ("each threshold
re-measured in S055's own commit"), leaving the body the only stale part.

### 2026-08-16_plan_review-F04  medium  S068 is the next step and its entire evidence base is two files in a per-session scratchpad, produced by a method that cannot be re-run at HEAD

Status: planned — S072

**Evidence.** `adocs/status.md:10` gives `Next: S068`, which the list confirms
(entry 45, first entry not in `plan_done/`). `S068_rfp_margin_retune.md:27`:

> All at ply bound 3, depth bound 6, from `.../scratchpad/rfp_ply_sweep.tsv`:

and `:35`:

> Margins 150, 200 and 300 were measured too and prune less, so they are the
> wrong direction unless 75 loses. `.../scratchpad/rfp_sweep.tsv` has them.

Neither path names a machine, a session or a directory that can be resolved.
Both resolve today only inside one session's scratchpad:

```
$ find /tmp/claude-1000 -name 'rfp*sweep*'
/tmp/claude-1000/-home-max-ws-chesso/e6b3ca05-.../scratchpad/rfp_sweep.tsv
/tmp/claude-1000/-home-max-ws-chesso/e6b3ca05-.../scratchpad/rfp_ply_sweep.tsv
/tmp/claude-1000/-home-max-ws-chesso/e6b3ca05-.../scratchpad/rfp_ply_sweep.sh
...
```

AGENTS.md §12: "Nothing that matters is allowed to exist only in an agent's own
memory, session transcript, or tool-local notes."

**The method cannot be re-run.** Both scripts vary the constant with a compiler
flag — `flags="-DRFP_MARGIN=$margin -DRFP_MIN_DEPTH=$mind -DRFP_MAX_DEPTH=$maxd"`
in `rfp_sweep.sh`. At HEAD the constants are unconditional `#define`s
(`src/search.cpp:38-47`, no `#ifndef` guard), and `6bd650e` is the only commit
that ever touched them, so this is what the sweep now does:

```
$ cmake -S . -B <scratch>/build-probe -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-DRFP_MARGIN=75" && cmake --build <scratch>/build-probe -j12 --target chesso
/home/max/ws/chesso/src/search.cpp:38: error: "RFP_MARGIN" redefined [-Werror]
cc1plus: all warnings being treated as errors
```

`rfp_sweep.sh` swallows that into `BUILD_FAIL` and continues, so a re-run
produces a table of failures rather than an error.

**What does reproduce.** The shipping row of S068's table is exact:

```
$ tools/search_bench.py ./build/src/chesso 9
  midgame      0.080s       292313 nodes  ...  best c3d5
  kiwipete     0.193s      1026739 nodes  ...  best e2a6
  tactical     0.019s       103001 nodes  ...  best d7c8q
```

292313 + 1026739 + 103001 = **1422053**, matching `S068:33`. The margin-75 row
(1216123) cannot be reproduced without editing `src/search.cpp:38` by hand, which
S068 does not say and the scripts it points at cannot do.

**Impact.** The next step's premise — "75 [...] visits 1216123 nodes against
1422053, 14.5 % fewer" — rests on data outside the repository and on a build
procedure that now errors. A session that starts S068 and tries to widen the
sweep (S068's own trap paragraph invites it: "Any margin below 75 needs the same
treatment") hits `-Werror` and has no tracked record of what the earlier sweep
actually varied. `.tuning/` losing `selfplay_v1.tsv` at DEC-049 is the same
failure and cost S065 a night; the scratchpad is more volatile than `.tuning/`,
not less.

**Suggested resolution.** Copy `rfp_ply_sweep.tsv` and `rfp_sweep.tsv` into
`adocs/data/` (they are 10 and 46 rows) and cite them from S068 by tracked path,
with the column meanings stated — the two files use `min` for `RFP_MIN_DEPTH`
and `min_ply` for `RFP_MIN_PLY`, which S068's prose does not distinguish and
which makes the two tables look contradictory at margin 75 (green at 1216123 in
one, RED at 2886952 in the other). Either guard the three `#define`s with
`#ifndef` so `-D` works, or have S068 state that the margin is changed by editing
`src/search.cpp:38`.

### 2026-08-16_plan_review-F05  low  status.md's parked list contradicts specs.md about the machine and is stale in its own worked example of the retention window

Status: planned — S069

**Evidence.** `adocs/status.md:53-59`:

> **Measurement capacity is the binding constraint on the whole plan.** At
> 10+0.2 with three usable cores an SPRT verdict costs about an hour, the
> opening book is only `8moves_v3.pgn`, and this machine runs `opendirectoryd`
> at half a core often enough to matter. An x86-64 Linux box fixes this and is
> needed for S032 and S029 regardless.

`adocs/specs.md:189-194`, same item, rewritten:

> an SPRT verdict costs three to four and a half hours at the DEC-048/DEC-050
> settings (all 12 threads of the DEC-049 machine) [...] (2026-08-13: rewritten
> for the DEC-049 move -- the old text priced an hour on three Apple cores with
> `opendirectoryd` overhead and called for an x86-64 box, which S032 and S029
> now have.)

`opendirectoryd` is a macOS daemon; the machine is Linux with 12 threads
(`S032_pext_sliding_attacks.md:14`, and `adocs/plan_todo/S068...:48` prices an
SPRT at "44 m 10 s at 12 cores for 1012 games"). Precedence is specs > plan >
status, so status.md is the wrong one, and it is the first document in the
reading order of AGENTS.md §1.

The same file's first parked item, which explains the retention window, has
gone stale in three places since it was written:

> **"Last done" above reads S053 and the newest completion is S065.** [...] the
> retention window leaves S053 at position 66, after the parked S029, while
> S038, S066, S067 and S065 sit at 38 to 41.

At HEAD: three completions landed after S065 (`fca9522` S033, `a124f07` S062,
`2e7f561` S064); S038, S066 and S067 have no list entry at all
(`grep -n 'S038\|S066\|S067' adocs/plan.md` returns only prose lines 51, 97,
135); and S053 is at position **67**, S029 at 66.

**Impact.** Low, and both are read rather than executed. But `status.md` is the
first file the reading order names, the machine paragraph is the one a session
consults before deciding whether it can afford an SPRT, and the retention
paragraph is the worked example a session uses to understand why "Last done"
disagrees with the newest completion. `2026-08-13_plan_review.2` recorded the
machine residue as out of scope; it is in scope now and has survived four
completions.

**Suggested resolution.** Bring the machine paragraph into line with
`specs.md:189-194` — 12 threads, three to four and a half hours, no
`opendirectoryd`, no x86-64 box wanted. Restate the retention example in terms of
positions rather than a list of ids that the window keeps dropping, or drop the
id list and point at `plan.md:130-138`, which S064 already made correct.

### 2026-08-16_plan_review-F06  low  2026-08-14_test_review-F02 is only half fixed: the illegal mate-in-zero FEN survives in tests/test_engine.cpp

Status: planned — S070

**Evidence.** `2026-08-14_test_review-F02` recorded `7k/5Q1K/8/8/8/8/8/8 b - - 0
1` as a position where the only attacker of the mated king is the enemy king, and
named two sites in `tests/test_search.cpp`. S067 fixed both — the FEN now
survives there only inside a comment (`tests/test_search.cpp:146`, "The mate half
used [...] until S067"). It is still live in a third file the finding did not
name:

```
$ grep -n "7k/5Q1K/8/8/8/8/8/8" tests/*.cpp
tests/test_engine.cpp:560:      uci_process_line("position fen 7k/5Q1K/8/8/8/8/8/8 b - - 0 1");
tests/test_search.cpp:146:  // The mate half used 7k/5Q1K/8/8/8/8/8/8 b until S067, where the *only*
```

`tests/test_engine.cpp:554-566` is `TEST_CASE("first_legal_move reports nothing
in a terminal position")`, asserting `REQUIRE_EQ(first_legal_move(), 0)`.

```
$ printf 'position fen 7k/5Q1K/8/8/8/8/8/8 b - - 0 1\ngo depth 5\nquit\n' | stockfish
Stockfish dev-20260810-5062aee5 by the Stockfish developers (see AUTHORS file)
info string CRITICAL ERROR: Command `position fen 7k/5Q1K/8/8/8/8/8/8 b - - 0 1`
failed. Reason: Unsupported position. King can be captured.
```

**Impact.** Low. The assertion holds and nothing computes a wrong answer, but
what it proves is that a side whose king is adjacent to the enemy king has no
legal moves — not that a legally terminal position reports none. It is the same
class the audit raised, in the same suite, and a test_review re-run would find it
again and would be right to.

**Suggested resolution.** Replace with a legal mate or stalemate, verified by a
tool, and assert legality as a precondition — the shape S067 used in
`tests/test_search.cpp`. Note that this is a test defect, not a plan defect; it
is recorded here because it changes the verdict on a prior finding from "fixed"
to "partially fixed".

### 2026-08-16_plan_review-F07  low  S021 is the next play-altering step after S068 and is the only search step with no mate clause, while DEC-060 records that reverse futility's exposure is a function of beta

Status: planned — S074

**Evidence, and what it is not.** S021 (list entry 46) narrows the root window.
Its whole gate is `S021_aspiration_windows.md:3`:

> accepts:    an SPRT with bounds matched to the expected effect size returns a
> verdict

Every neighbouring step that touches pruning carries a mate clause and S021 does
not: `S033_reverse_futility_pruning.md:3` ("a position with a forced mate inside
the pruned depth is in the fast suite and passes before the feature is called
done"), `S068:3` ("every mate case in the fast suite stays green at the setting
that ships, the material-leader case included"), `S026:16` (the body clause S060
is queued to move into the accepts).

DEC-060 records that the guard S033 predicted would protect the mate cases is
not what protects them, and that the failure is a property of the caller's
window:

> RFP ply=1 depth=2 alpha=-965 beta=-964 eval=-764 ret=-964
> [...] It fails high because `beta` is **-964**: the parent is a null-window
> scout hunting a mate score [...] `beta` is nowhere near the mate band, so the
> guard the step prescribed does nothing.

and its Consequences: "A mate that first becomes visible below ply 3 can still
be missed for an iteration and nothing in the suite covers that. Stated, not
fixed." The same sentence ships in `src/search.cpp:337`. What contains the
hazard today is `RFP_MIN_PLY 3` (`src/search.cpp:47`), set from a measured
cliff, not from an argument about windows.

**I could not reproduce a failure.** Aspiration windows are not implemented
(`grep -rniE 'aspiration' src/*.cpp src/*.hpp` returns nothing), so there is
nothing to run. What is demonstrable is the gate asymmetry above and DEC-060's
measured finding that RFP's mate exposure depends on the bound its parent passes
down — which is the quantity S021 changes. Whether narrowing the root window
moves any node's `beta` into the shape DEC-060 traced is unmeasured, and I am
recording it as unconfirmed rather than as a defect in the search.

**Impact.** If it is real, the symptom is a mate missed for an iteration, which
`CLAUDE.md` names as this engine's recurring bug and which DEC-060 says the
suite does not cover below ply 3. S021's SPRT would not distinguish it from
noise at the effect size S021 itself predicts (+9 ± 17).

**Suggested resolution.** Add the fast suite's mate cases to S021's accepts, as
S068 does — the two `MATE_IN_2_*` cases plus "pruning does not hide a mate
against the material leader" (`tests/test_search.cpp:1013`), green at the window
schedule that ships. It costs one `ctest` run and it is the only pending
play-altering step that does not already ask for it.

### 2026-08-16_plan_review-F08  info  adocs/eval_tuning_strategy.md is an untracked 529-line plan input inside adocs/, outside the file map and outside git

Status: accepted — DEC-062

**Evidence.**

```
$ git status --porcelain
?? adocs/audit/2026-08-16_plan_review.md
?? adocs/eval_tuning_strategy.md

$ wc -l adocs/eval_tuning_strategy.md
529 adocs/eval_tuning_strategy.md

$ head -4 adocs/eval_tuning_strategy.md
# Evaluation and Search Parameter Tuning: State of the Art

**Purpose.** Input document for a development plan. Describes the techniques with the highest
expected Elo return for a classical alpha-beta chess engine, ordered by return on effort, with
```

AGENTS.md §2's file map governs `adocs/` and has no row for it. It is untracked,
so it does not survive the loss `status.md:22-32` records for `.tuning/`. No
pending step references it (`grep -rn eval_tuning_strategy adocs/plan*.md
adocs/plan_todo/` returns nothing). Its executive summary is a table of published
Elo figures presented as "Typical gain" (+100 to +300, +400 to +700, +10 to +40),
the class of claim DEC-019 and `CLAUDE.md` foundation 2 point 7 admit as
direction only.

**Impact.** Informational at present, because nothing cites it. It becomes a
problem the moment something does: a plan input that is neither tracked nor in
the file map is exactly the "memory that lives outside the repository" AGENTS.md
§12 forbids, and its Elo table is the shape DEC-019 exists to stop being read as
a target.

**Suggested resolution.** Either track it under a file-map row — it reads like
research input to phase-one planning, which `decisions.md` or a step file would
carry — or delete it. If it is kept, mark the Elo column as reported figures
that decide what to try and never what to conclude, per DEC-019.

### 2026-08-16_plan_review-F09  info  S060's and S061's supporting evidence predates S033, which added the test S060 says does not exist

Status: planned — S078

**Evidence.** `S060_s026_mate_test_in_gate.md:19-24`:

> Nothing states that the existing cases — `tests/test_search.cpp:82` "mate in
> one" and `:124` "mate in two is found at the right distance" — sit inside the
> depth window futility and razoring prune, and no test name asserts it.

Both citations still resolve. But S033 landed on 2026-08-16 (`fca9522`) and
added two cases that do exactly that, one of them named for it:

```
$ grep -n 'TEST_CASE_FIXTURE' tests/test_search.cpp | grep -i 'prun'
974:  TEST_CASE_FIXTURE(search_fixture_t, "pruning does not hide a forced mate")
1013:  TEST_CASE_FIXTURE(search_fixture_t,
1014:                    "pruning does not hide a mate against the material leader")
```

`tests/test_search.cpp:999-1008` states the three properties that make the second
bite, including "the mate is inside the pruned depth", and asserts two of them as
preconditions (`:1022-1024`).

S061's citation of the band test is off by nine lines at both ends: it names
`tests/test_evaluation.cpp:643-704`; the case runs `:652-713`.

**Impact.** Neither step's `accepts:` is wrong — S026's gate really does not
carry the clause, and S023's really has no instrument. The bodies overstate the
gap: S026's technique now has a template to copy, which is what S060 should point
at rather than at the absence.

**Suggested resolution.** Point S060's body at
`tests/test_search.cpp:1013-1033` as the shape the two new cases should take,
and re-point S061's range. Both are inside S063's stated intent and outside its
`touches:` (F02).

### 2026-08-16_plan_review-F10  info  plan.md and testing.md cite bin/moltke.py, which is not in this repository

Status: planned — S071

**Evidence.** `adocs/plan.md:130-131` and `adocs/testing.md:13`:

> `bin/moltke.py:1698-1700` collects the completed entries by position in the
> file and drops all but the final `PLAN_DONE_KEPT` of them, 5 at `:1681`.

```
$ ls /home/max/ws/chesso/bin/
ls: cannot access '/home/max/ws/chesso/bin/': No such file or directory
```

The file is the installed plugin's, at
`~/.claude/plugins/cache/moltke/moltke/0.11.0/bin/moltke.py`, where the citation
is exact (`:1681` `PLAN_DONE_KEPT = 5`; `:1698-1700` the `entry_lines`/`drop`
pair). Two versions are installed, `0.1.0` and `0.11.0`.

**Impact.** Informational. The claim is true and was checked; the path is not
resolvable from the repository and the line numbers move with the plugin
version, which is not tracked here. S064 and S053 both landed prose built on it.

**Suggested resolution.** Name the version alongside the path, or cite
`PLAN_DONE_KEPT` and the pruning function by symbol.

## What was checked and found sound

Stated because a negative result is a result.

- **List and file correspondence.** 27 list entries, no duplicates, every entry
  names an existing step file, every pending step file appears exactly once.
  22 pending entries against 22 files in `plan_todo/`; `plan_current/` holds only
  `.gitkeep`. Exactly five completed entries are retained (S065, S033, S062,
  S064, S053), which is `PLAN_DONE_KEPT`. The first list entry not in
  `plan_done/` is **S068**, agreeing with `status.md:10`.
- **The corrective steps sit where plan.md says.** S060(47) before S026(48),
  S063(49) before S024(50), S061(51) before S023(52), S059(53) before S025(54),
  S057(57) before S039(58), S056(60) before S055(61), S058(62) before S030(63).
  All seven hold.
- **INV-6 gates.** Every pending step that alters play carries an SPRT clause
  (S021, S022 two, S023, S024 two, S025, S026 per technique, S029, S039
  conditional, S042, S055, S068); every step claiming neutrality carries the
  `search_bench` identity clause (S020, S030 with the SPRT branch, S031, S032).
  S046's and S051's fixes hold.
- **plan.md's prose no longer describes a completed step as pending.** Every id
  named in prose was checked against `plan_done/`: S035/S036/S037 past tense and
  "all three are done"; S044-S052 "went ahead of everything then pending";
  S054, S065, S066 past tense; S029 marked parked at DEC-054 and last in the
  pending order. `2026-08-13_plan_review.2-F07` does not reproduce here — the
  new instance of that class is in `status.md`, recorded as F05.
- **plan.md's description of the checker matches the checker.** Retention by
  list position, `PLAN_DONE_KEPT = 5`, and the ledger rule that a row leaves only
  when every `S<nnn>` it names was dropped in the same pass — all three verified
  against `moltke.py:1681,1698-1700`. `2026-08-13_plan_review.2-F09` does not
  reproduce.
- **Premises that still hold at HEAD.** S020: `is_check` is recomputed per node
  in `quiescence` (`src/search.cpp:148`), in `negamax` (`:307`) and again for the
  child by the parent (`:471`). S031: all four side-to-move xor sites are exactly
  where the file says (`src/bitboard.cpp:878-880`, `1059-1061`, `1460-1462`,
  `1537`), and the `side_randoms[W]^side_randoms[B]` bit-identity argument
  holds. S042: `src/bitboard.cpp:826-828` still sets the square from
  `move.double_push` alone. S055: `tools/eval_model.hpp:973-974` still tapers the
  two terms once, `tempo_mg == tempo_eg == 0` (`src/evaluation.cpp:582-583`) so
  the three-truncation bound stands, and `src/evaluation.cpp:951,953` are the two
  divisions to merge. S023/S061: `MVV_PAWN 100`, `MVV_KING 100000`,
  `ORDER_CAPTURE 1000000`, `ORDER_KILLER_0 900000` (`src/evaluation.cpp:26-36`).
  S024: `move_t counter_moves[12][64]` at `src/data_structures.hpp:434`, no
  continuation history anywhere. S032: BMI2 present, 12 threads.
  S068: `RFP_MARGIN 100`, `RFP_MAX_DEPTH 6`, `RFP_MIN_PLY 3`.
- **Nothing pending is already implemented.** `grep -rniE
  'aspiration|delta prun|razor|capture_history|continuation|improving'
  src/*.cpp src/*.hpp` returns nothing. Reverse futility is present and is S033,
  which is done.
- **Audit coverage.** Every finding in the four tracked reports maps onto a step
  with a matching `closes:` field or onto a decision. None is dangling.
- **Suite at HEAD.** `ctest --test-dir build -L fast`: 12/12 passed, 22.19 s.

## Verdicts on prior findings

Each re-measured from its own reproduction at
`02bb6a5fb190329f2f1edd06f515cb0028e16e69`.

### 2026-08-13_plan_review.2 (nine findings, all recorded `planned`)

This run is a plan_review re-run, so these are the verdicts that can move a
plan_review finding to `closed`.

- **F01 (S055's accepts asks for an unreachable suite state)** — still
  reproduces. `S055...md:3` is unchanged: "the tuner-model tolerance returns to 2
  and test_eval_model passes over the pinned corpus at that tolerance", and
  `tests/test_eval_model.cpp:432,440` still assert `difference > 2.0` and
  `worst > 2.8`, which `2 x 23/24 = 1.917` cannot satisfy. S056 pending at entry
  60, and S056's own evidence has gone stale — F03 above. **planned.**
- **F02 (S039's corpus does not exist)** — the original evidence no longer
  reproduces: `.tuning/selfplay_v2.tsv` exists, is 11003693 positions, and
  `DEV_MANUAL.md:254-255` documents the exact `eval_spread` run. The finding is
  superseded rather than closed: S057, the step created for it, now argues from
  premises that are false and hands S039 figures 3.3 times too small — F01
  above. **planned**, and S057 needs rewriting rather than executing.
- **F03 (S030's remedy is wrong for the two prev_move sites)** — still
  reproduces. `S030...md:19-24` is unchanged: "Every such site must be
  re-pointed at `squares[from]`", and `src/evaluation.cpp:1094` and
  `src/search.cpp:564` still key on `prev_move`. S058 pending, and S058's own
  citations are stale — F02 above. **planned.**
- **F04 (S025's gate admits one outcome)** — still reproduces.
  `S025...md:3` unchanged: "fixed-depth time is not worse [...]; only then an
  SPRT". S059 pending at entry 53. **planned.**
- **F05 (S026's gate is weaker than its own body)** — still reproduces.
  `S026...md:3` unchanged. S060 pending at entry 47; its body evidence is now
  partly outdated — F09 above. **planned.**
- **F06 (S023's disjoint-bands criterion has no instrument)** — still
  reproduces. `S023...md:3` unchanged, and `grep -rn "900100\|MVV_KING" tests/`
  still returns nothing. S061 pending at entry 51. **planned.**
- **F07 (plan.md prose calls completed steps pending)** — does not reproduce in
  `plan.md`; S062 cleared it and every prose id was re-checked (see above). A new
  instance of the same class has appeared in `adocs/status.md` and is recorded as
  F05 rather than as a survival of this finding. **closed.**
- **F08 (three step files cite lines that moved two lines at S037)** — still
  reproduces, and has grown from 3 misses to 17 across six files, 9 of them
  outside S063's `touches:`. F02 above. **planned.**
- **F09 (plan.md describes pruning as oldest-completion-first)** — does not
  reproduce. `plan.md:130-147` now states retention by list position with the
  `moltke.py` citation and the ledger rule, and `testing.md:9-18` carries the
  same. Both verified against the checker. **closed.**

### 2026-08-13_adversarial (nine findings, seven verified fixed, two open)

A different audit type; only an adversarial re-run changes their status.

- **F05 (`LAZY_EVAL_MARGIN` precondition false)** — still reproduces and is
  materially worse than when it was written. `src/evaluation.hpp:274-281` still
  says 150 "is above the largest correction observed" and "is still the whole
  correction only because king safety ships at zero weight". Measured over the
  full corpus at HEAD: mobility alone reaches 267, king safety alone 363,
  combined 489, and 1.558 % of 11003693 positions exceed 150. S039 pending.
  **planned.**
- **F08 (en passant square set unconditionally)** — still reproduces.
  `src/bitboard.cpp:826-828` sets `new_en_passant` from `move.double_push`
  alone. S042 pending at entry 59. **planned.**
- F01, F02, F03, F04, F06, F07, F09 — fixes remain in place; re-verified only
  where a pending step depends on them (F03's cumulative `info nodes` underpins
  every `search_bench` figure quoted above and is correct: 1422053 is the
  whole-search sum). **planned**, pending an adversarial re-run.

### 2026-08-14_test_review (seven findings; six `planned` under S067, one `accepted`)

A different audit type; only a test_review re-run changes their status. S067 is
in `plan_done/`.

- **F01 (a tactical case has one legal move)** — fix verified. The FEN survives
  only in explanatory comments at `tests/test_search.cpp:373,414`.
- **F02 (the mate-in-zero position is mated by a king)** — fix **partial**.
  Repaired in `tests/test_search.cpp`; the same illegal FEN is still live at
  `tests/test_engine.cpp:560`. F06 above.
- **F03 (no working ctest invocation for the debug asserts)** — fix verified.
  `tests/CMakeLists.txt:11-15` selects the timeout by build type, 60 release /
  600 debug.
- **F04 (a helper states the opposite of INV-1)** — fix verified.
  `tests/test_helpers.hpp:75-83` now states legal-only generation and cites the
  finding.
- **F05 (insufficient material skips every boundary case)** — fix verified.
  Bishop-versus-bishop on both square colours and bishop-versus-knight are
  pinned, with the disagreement recorded in the case text.
- **F06 (the ordering guard has an order of magnitude of headroom)** — fix
  verified. `tests/test_search.cpp:529` sets `node_limit = 440000`, down from
  1000000.
- **F07** — `accepted` at DEC-058, unchanged.

Summary: of the nine plan_review findings, two no longer reproduce (F07, F09)
and are closed; seven still reproduce and keep their pending steps, one of them
(F02) superseded by a changed world rather than by a fix. Of the nine adversarial
findings, two still reproduce with steps pending. Of the seven test_review
findings, five are verified fixed, one is partially fixed, one is accepted.
