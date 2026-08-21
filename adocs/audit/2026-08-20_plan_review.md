# Audit 2026-08-20 — plan_review

Type: `plan_review`.
Commit as briefed: `3c72e16762dee4e200838d1b415ed0b4589b6cd4`.
Report id stem: `2026-08-20_plan_review`.

## Scope, and what this pass did **not** check

In scope and examined: `adocs/plan.md` and its ordering; the 50 step files in
`adocs/plan_todo/`; `adocs/plan_current/S085_spsa_first_run.md`;
`adocs/status.md`; `adocs/specs.md`; and the entries in `adocs/decisions.md`
that constrain a pending step (DEC-053, DEC-057, DEC-087, DEC-088, DEC-090 to
DEC-092). Pending steps' claims about the code were resolved against the source
at HEAD. `adocs/plan_done/` was read only as evidence about a pending step.
Prior reports under `adocs/audit/` were not treated as a brief; no finding below
is copied from one.

**Hard operational constraint: the machine was fully committed to a live SPSA
run (S085, pid 1458253) for the duration of this audit.** Nothing was built,
compiled, benchmarked, matched or timed. No `cmake`, no `ctest`, no
`fastchess.sh`, no `stockfish`, no corpus pass. Every command in this report is
`git`, `grep`, `sed`, `awk`, `find`, `wc` or a short `python3` document pass.

**Consequently every numeric claim in the plan and in the step files that needs
a run is recorded here as deferred, not as verified and not as sound.** The
classes, and the command that would settle each:

| deferred claim class | where | command that settles it |
|---|---|---|
| every published Elo figure and every measured Elo verdict (`+37.5`/`+28` S093, `+44.7`/`+34.0` S024, `+25.41` S117, `+11.4` S099, `+10.8`/`+12.1` S130, `+9.9`/`+9.7` S132, `~+88`/`~+65` S133, `+18.22 %` S104, `x1.67` S105, the 2559 anchor) | `plan.md`, most step files, `specs.md` | `./fastchess.sh --fast` / `REF=<sha> ./fastchess.sh`; `./rating.sh` for the anchor |
| `eval_spread` spread figures (`0.132 %`, `0.364 %`, worst `279`) | `S039` "What it costs now" | `cmake --build build && build/tools/eval_spread <corpus>` |
| truncation bound `2.875` and the four pinned FENs | `S055`, `S136`, DEC-053, DEC-092 | `build/tools/truncation_scan`, then `ctest -L fast -R test_eval_model` |
| `is_check` profile share (`12 %`, an Apple figure by its own admission) and the `888738` node count | `S020` | `perf record` on `build/tools/search_bench` + `tools/search_bench.py` |
| the S134 identity claims (`R² 1.000000`, 0 violations over 1264773 / 550880 rows) | `S134`, DEC-090 | `build/tools/feature_audit` over `.tuning/selfplay_v2_dedup.tsv` |
| "the fast suite green" in 18 accepts clauses | most step files | `ctest -L fast` |
| the S085 trajectory table (23.96 s/iteration, per-axis positions) | `S085` mid-run read | re-read `.tuning/spsa_S085.log` after `SPSA-DONE` |
| `search_bench` node identities quoted as baselines (`164123 / 670488 / 84351` etc.) | `specs.md`, `S117`, `S134` | `tools/search_bench.py` at the named depths |

One numeric claim **was** settled without running anything, arithmetically:
`PARAM_COUNT` is `MATERIAL_COUNT 5 + 2*SQUARE_COUNT 384 + 2*MOBILITY_COUNT 4 +
2*KING_SAFETY_COUNT 9 + 2*PASSED_PAWN_COUNT 6 + 2*PAWN_STRUCTURE_COUNT 3 +
2*PIECE_PLACEMENT_COUNT 4 + 2*TEMPO_COUNT 1 = **827**`, and S134's post-fold
`823` follows. S134's two numbers are correct.

## Repository state during the audit — read this before `--audit check`

Two things moved while this report was being written, neither by me:

1. **HEAD advanced from `3c72e16` to `1f935d1`** ("Record S085's 39 % trajectory
   read and the RfpMinPly bound defect"). `git diff --stat 3c72e16..HEAD` is
   `adocs/plan_current/S085_spsa_first_run.md | 69 +` and
   `adocs/status.md | 16 +` — documents only, no source. Every citation
   resolution below is therefore valid at both commits; only S085's own
   last-modified commit changed.
2. **`adocs/status.md` was modified in the working tree** by the session under
   audit, adding a "Three questions banked for the owner" parked item. I did not
   write it and did not revert it.

**I modified exactly one file: this report.** No regression test was added, and
that is a decision rather than an omission: every finding below reproduces from
a printed command, so none of them needs a test to be demonstrable. F01 would
benefit from a permanent checker, but a checker is the *fix* and belongs to
whoever plans the step; adding a red test now would also turn the marker's
`test_command` red underneath a live overnight measurement, which is a
consequence I was not asked to impose.

3. **`src/search_params.hpp` was modified in the working tree, uncommitted, while
   this report was being written** -- a 25-line rewrite of the `RFP_MIN_PLY`
   comment, correcting the root-exemption claim and the "the mate cases go red"
   claim from measurements taken in another session. Not mine. It bears on F08
   and is handled in an addendum there rather than by rewriting the finding,
   because F08's evidence is quoted at the briefed commit.

**Reconciliation, then: `git status --short` should show exactly
`?? adocs/audit/2026-08-20_plan_review.md` from me, and
` M adocs/status.md` plus ` M src/search_params.hpp` from the session under
audit.** The two small in-place edits to this report after its first write were
made with `python3` through `Bash` because the `Edit` tool is disabled in this
session; the path is the one this role is permitted to write.

This report also trips the plugin's `INV-10` post-write hook: every finding is
`open` and none is named by a step's `closes:` or by a `decisions.md` entry. That
is correct for this run rather than an oversight -- section 9 puts the report
before any fix and the planning after it, and creating those steps is not this
role's to do. The hook's output is the handover list.

## Findings

18 findings: 2 high, 6 medium, 10 low.

---

### 2026-08-20_plan_review-F01 — high — 70 of 147 `file:line` citations in pending step files are stale, and six of them now point at the wrong test

Status: planned
Planned in: S138, landed 2026-08-21

**Evidence.**

Census. Every `path:line` and `path:line-line` citation in the 51 pending step
files was compared against the file content at HEAD and at the commit that last
wrote the step file:

```
$ python3 - <<'PY'
import re, glob, os, subprocess
files = sorted(glob.glob('adocs/plan_todo/S*.md')) + sorted(glob.glob('adocs/plan_current/S*.md'))
pat = re.compile(r'((?:src|tools|tests|bin|include|scripts)/[A-Za-z0-9_./-]+?\.(?:cpp|hpp|h|py|sh|txt|md|json)):(\d+)(?:\s*-\s*(\d+))?')
def at(rev, path):
    r = subprocess.run(['git','show',f'{rev}:{path}'],capture_output=True,text=True)
    return r.stdout.splitlines() if r.returncode==0 else None
stale = ok = 0
for f in files:
    rev = subprocess.run(['git','log','-1','--format=%H','--',f],capture_output=True,text=True).stdout.strip()
    for i, line in enumerate(open(f).read().splitlines(), 1):
        for m in pat.finditer(line):
            p, a = m.group(1), int(m.group(2))
            b = int(m.group(3)) if m.group(3) else a
            old, new = at(rev, p), at('HEAD', p)
            if old is None or new is None: continue
            if "\n".join(old[a-1:b]) == "\n".join(new[a-1:b]): ok += 1
            else:
                stale += 1
                print(f"DRIFTED {os.path.basename(f)}:{i} {p}:{a}-{b}")
print(f"drifted={stale} unchanged={ok}")
PY
...
drifted=70 unchanged=77
```

Drift magnitude, measured by relocating the originally-cited first line at HEAD:
`src/data_structures.hpp` +2 (8 citations), `src/chesso.cpp` +1 and +37 (4),
`src/search.cpp` +10 and +16 (26), `src/evaluation.cpp` +57 (17),
`tests/test_eval_model.cpp` +101 (1), `tests/test_search.cpp` +71 and +613 (12),
1 unlocatable. The four commits that caused it all landed *after* the enrichment
pass that wrote the citations: `9202f5d` (S100, +57 in `evaluation.cpp`),
`ec4d1dd`/`23b556f` (S106/S107, +10 in `search.cpp`, +613 in `test_search.cpp`),
`ce9ed34` (S137). `S020` is the only file that dates its citations
(`adocs/plan_todo/S020_single_check_computation.md:26` — "Line numbers read at
`c0954ec`").

The concrete hazard, not the census. Five pending steps gate on the mate-safety
test by line number:

```
$ grep -n "test_search.cpp:1274\|test_search.cpp:1313\|test_search.cpp:1270-1291" adocs/plan_todo/*.md
S091:165, S097:205, S098:183, S098:255, S109:220, S114:95
$ git show c0954ec:tests/test_search.cpp | sed -n '1274p'
  TEST_CASE_FIXTURE(search_fixture_t, "pruning does not hide a forced mate")
$ sed -n '1266p;1274,1275p;1887p' tests/test_search.cpp
1266:  TEST_CASE("a mate bound is compared after the ply adjustment, not before")
1274:    // "At least a mate one ply from here". Read at ply 3 that is a mate at ply
1275:    // 4 of the search, worth MATE_MAX - 4.
1887:  TEST_CASE_FIXTURE(search_fixture_t, "pruning does not hide a forced mate")
```

At HEAD line 1274 is inside a *different* test — the `tt_entry_answers`
ply-adjustment case S106 added — and the named test is at 1887. S098's
`tests/test_search.cpp:1270-1291`, cited as "null move hid a mate in 2", is
entirely inside that other test. S091's `tests/test_search.cpp:1649`, cited as
"six hand-valued cases, a quiet into an attack", is inside `"no stored score
leaves the mate band"` (test begins at 1625). S112 and S131 both cite
`tests/test_search.cpp:554-568` and `:630-664` for a quiescence helper; both are
off by 71.

Same class, one level up: DEC-053 — which constrains S055 and S136 — cites
`src/evaluation.cpp:640`, `:668`, `:951`, `:953`, `:582-583` and `:647-670`, all
of them +57 out at HEAD.

Also unresolvable rather than drifted: `S085:180` names
`python3 bin/moltke.py --watch ...`; there is no `bin/` in this repository
(`ls -d /home/max/ws/chesso/bin` → No such file or directory). The primitive is
at `/home/max/.claude/plugins/cache/moltke/moltke/0.12.0/bin/moltke.py`. And
`S091`'s **`excludes:` field** cites `src/search.cpp:205` for "where `see_ge`
already declines losing captures"; the `see_ge` guard is at `src/search.cpp:332-333`
at HEAD and was at `:322-323` when the file was written, so that one was never
right.

**Impact.** CLAUDE.md names "pruning that hides a mate" as the recurring bug and
`"pruning does not hide a forced mate"` as the guard that caught it twice. Five
of the six pending steps that add pruning or reductions instruct their
implementer to extend that guard *at a line number that now lands in an
unrelated test*. An implementer who follows the citation extends the wrong case
and ships pruning whose mate-safety gate was never touched — and the symptom of
that, by this project's own record, is a strength regression rather than a red
test. The remaining 64 stale citations cost re-derivation time and make every
"verified at" claim in those files unverifiable as written. This is the third
recorded occurrence of the class (`status.md` records a line range into
`specs.md` stale three times running; S071 and S078 were steps against earlier
instances), so it recurs by construction and not by accident.

**Suggested resolution.** Two parts. (a) Re-anchor: replace every line-range
citation in a pending step file with a symbol, a test title or a `grep`
expression — which is already what S024's own `accepts:` demands of itself
("every `src/` citation in this file names a symbol or a test title rather than a
line range"), so the rule exists and is applied to one file out of fifty. (b) Add
a check that fails when a `path:line` citation in `plan_todo/` or
`plan_current/` no longer holds the text it held when the file was written, and
register it with the suite so a landing step cannot silently invalidate fifty
others.

---

### 2026-08-20_plan_review-F02 — high — S119's `accepts:` demands a verdict at Hash 128, which DEC-088 explicitly ordered removed and which the harness cannot produce

Status: planned
Planned in: S139, landed 2026-08-21
Applied: S119's accepts now reads "at the S105 harness setting, with the pressure
ratio stated", DEC-088's `Consequences:` clause verbatim, with the 60-120 overwrites
per entry named; the "regime S105 sets" attribution is gone. Traced to
`fastchess.sh:225` (`option.Hash=16`, no override) and `rating.sh:36` (`hash_mb=128`,
named in S119's body as the separate tool a rating-regime verdict would need).

**Evidence.**

```
$ sed -n '3p' adocs/plan_todo/S119_tt_cluster_layout.md
accepts:    an SPRT verdict at **Hash 128** -- the regime S105 sets and the one
the rating list runs, because a table change measured at 16 MB measures the wrong
table; ...
```

Every other document says the opposite:

```
$ sed -n '67p' adocs/plan.md
where 128 MB undershoots them eightfold. `fastchess.sh` keeps Hash=16;
$ grep -n "Hash" fastchess.sh
203:  -each tc="$tc" option.Hash=16 option.Threads=1 \
$ grep -n "hash_mb" rating.sh
36:hash_mb=128
$ awk '/^## DEC-088/{f=1} f&&/^## DEC-089/{exit} f' adocs/decisions.md | tail -4
Consequences: S119's SPRT clause changes from "at Hash 128" to "at the S105
              harness setting, with the pressure ratio stated"; verdicts stay
              comparable within the S105 regime as before.
```

`specs.md:177` agrees with the harness: "the rating list runs 128 to 256 MB while
`fastchess.sh` runs 16. S119, and S105 for the regime". `fastchess.sh` exposes
`CONCURRENCY`, `OUT` and `REF` as overrides and no hash override —
`grep -n 'Hash' fastchess.sh` returns exactly one line, hardcoded.

**Impact.** DEC-088 is a recorded owner decision whose `Consequences:` clause
names the edit to make; the edit was never made, and the step file now asserts
the retracted premise *and* attributes it to S105, which set the opposite. Three
ways this bites: the clause is unsatisfiable with the harness as it ships (the
step's `touches:` does not include `fastchess.sh`, so an implementer honouring
both fields cannot run the required match); a verdict taken at 128 MB is not
comparable with any other verdict in the S105 regime, breaking the one thing
DEC-088 says survives; and DEC-088's stated reason for 16 MB is precisely that
128 MB "would flatter every table-hungry change S119 is about to make", so the
stale clause biases the measurement in the direction of accepting the step.

**Suggested resolution.** Apply DEC-088's `Consequences:` clause verbatim to
`S119`'s `accepts:` — "at the S105 harness setting, with the pressure ratio
stated" — and delete the "the regime S105 sets" attribution. If a second verdict
at the rating regime is genuinely wanted, it is a `rating.sh` run and needs to be
named as one, with `rating.sh` in `touches:`.

---

### 2026-08-20_plan_review-F03 — medium — S136's accepts arithmetic (and DEC-092's) is one taper division out at the point the plan runs it

Status: planned
Planned in: S139, landed 2026-08-21
Applied: half. S136's accepts is now derived -- N divisions can round at its own HEAD,
unfreezing tempo makes it N+1 -- with the S055-landed case spelled out (N=2: bound
1.917 to 2.875, tolerance 2 to 3, threshold `> 1.0` to `> 2.0 = 48/24`, worst-pin
`> 1.9` to `> 2.8`), and a new section traces it to `adocs/plan.md:352,363`, S055's
accepts and `tests/test_eval_model.cpp:242-249,277,311-314,335,343`. DEC-092's
literals are owner text: S139 appended an explicitly unapproved `Proposed:` block
rather than amending them. S136's `goal:` and `adocs/plan.md:363` still carry the
pre-S055 count of three -- moving them needs `adocs/plan.md` in a `touches:` field,
which S139 did not have.

**Evidence.** The engine tapers through four integer divisions; three can round
today because tempo ships at zero. `tests/test_eval_model.cpp` pins that:

```
$ sed -n '248,252p;277p;312,314p;335,344p' tests/test_eval_model.cpp
      // (src/evaluation.cpp:582-583), so its division truncates 0 / 24 exactly
      // and contributes nothing, which puts the bound at 3 x 23/24 = 2.875 and
      // this tolerance at 3. ...
      CHECK(std::abs(model - engine_white) <= 3.0);
                  "tempo is fitted, so all four taperings can truncate: the "
                  "bound is now 4 x 23/24 = 3.833 and the tolerance in \"the "
      CHECK_MESSAGE(difference > 2.0, ...
                                std::to_string(worst) + ", short of 2.875"));
```

S055 removes one of those divisions and says so, and re-pins both numbers:

```
$ sed -n '2,3p' adocs/plan_todo/S055_taper_stage_two_once.md
goal:       taper mobility and king safety through one division instead of two,
tightening the model guard's bound to 2
accepts:    ... re-pinned to the post-merge bound of 2 x 23/24 = 1.917, and the
tempo precondition message's arithmetic becomes 3 x 23/24 = 2.875 with its
tolerance of 4 becoming 3 ...
```

S136 assumes S055 has not happened:

```
$ sed -n '3p' adocs/plan_todo/S136_unfreeze_tempo.md
accepts:    ... the bound moves from 3 x 23/24 = 2.875 to 4 x 23/24 = 3.833 the
moment tempo is non-zero, so `test_eval_model`'s tolerance goes 3 to 4 **from
that arithmetic** ... the non-vacuity clause moves with it -- ... which is
`> 2.875` once four can truncate, not the old `> 2.0`
```

The plan orders S055 first:

```
$ grep -n "^33\.\|^44\." adocs/plan.md
33. S055  taper mobility and king safety through one division instead of two, ...
44. S136  unfreeze tempo, re-derive the truncation guard it was holding at three divisions, ...
```

S136 never mentions S055 or the division count changing
(`grep -n "S055" adocs/plan_todo/S136_unfreeze_tempo.md` → no match), and DEC-092
carries the same pre-S055 arithmetic in both its `Decision:` and
`Consequences:` blocks while tagging `dec-053` — DEC-053 being the decision that
counts the divisions in the first place.

**Impact.** After S055 lands there are three divisions, two live. Unfreezing
tempo then makes it three live: bound `3 x 23/24 = 2.875`, tolerance 3,
non-vacuity threshold `> 1.917`. S136's accepts demands 3.833, tolerance 4 and
`> 2.875` — each one division too high. Taking S136's numbers would raise the
model-versus-engine tolerance a whole unit above what the arithmetic supports,
which is exactly the relaxation DEC-092 says must not happen ("this is a
re-derivation and not a relaxation"), and would set a non-vacuity threshold no
position can reach, silently disarming the guard that exists to prove the pinned
corpus exercises every division. S136's own `goal:` line already carries the
stale premise: "the truncation guard it was holding at **three** divisions".

**Suggested resolution.** Make S136's arithmetic derived rather than literal —
state it as "one more division than can round at this step's own HEAD" and have
the step re-read the count — or amend DEC-092 and S136 together with an explicit
"if S055 has landed" branch, the way S102's accepts already hedges ("the taper is
a single division **if S055 has landed**"). Either way one of the two files has to
name the other.

---

### 2026-08-20_plan_review-F04 — medium — S125's accepts requires S118, which the plan deliberately orders after it

Status: planned
Planned in: S139, landed 2026-08-21
Applied: the first of the two options. S125's accepts drops the S118 dependency and
states that the terms are measured recomputed per call, the per-call cost recorded as
the baseline S118 later reclaims; the plan order is unchanged. Decided on DEC-087 (h)
and (i), `adocs/plan.md:143-145` and `:251-253`, and S118's own body
(`S118_pawn_hash_table.md:15-19`, `:34-37`) -- all four order the terms before the
cache, and a cache in front of a cheap computation is the published 10 % slowdown
DEC-087 moved S118 to avoid.

**Evidence.**

```
$ sed -n '3p' adocs/plan_todo/S125_pawn_structure_completion.md
accepts:    ... the terms live behind the pawn hash from S118 so the added cost
is paid once per structure
$ grep -n "^48\.\|^49\." adocs/plan.md
48. S125  backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
49. S118  the pawn terms and the king shelter are computed once per pawn structure and cached, ...
```

The ordering is intentional, not a slip: `plan.md:245` ("the connected and
phalanx pawn work (S125, +25.4 class), **the pawn hash that makes them
affordable (S118)**") and DEC-087's re-scope ("S118 moves out of the speed block
to land after the pawn terms it caches are worth caching (a cheap pawn eval
cached measured a published slowdown)").

**Impact.** S125 cannot satisfy its own `accepts:` at its ordered position:
there is no pawn hash for its terms to live behind. The clause is not cosmetic —
it is the step's only cost control, and the alternative is that four new pawn
terms are computed at every `evaluate()` call, which is the INV-4 hazard
CLAUDE.md names ("every evaluation term must be accumulated, not recomputed";
S014 removed 25 % of nps that way). Whoever runs S125 will either block on a
step that is not next, or drop the clause and ship the recomputation.

**Suggested resolution.** One of: strike the S118 clause from S125's accepts and
state plainly that S125 pays per-call until S118 lands, with the per-call cost
measured inside S125's own verdict; or swap the two list entries and record the
reversal of DEC-087's ordering as a decision.

---

### 2026-08-20_plan_review-F05 — medium — `specs.md` still routes check extensions to S096, which DEC-087 retired

Status: planned
Planned in: S140

**Evidence.**

```
$ sed -n '318,320p' adocs/specs.md
  **Discharged 2026-08-18 by DEC-071** and re-ordered 2026-08-19 by DEC-081 to
  DEC-086. Check extensions are S096, singular extensions S097, the quiescence
  transposition probe and the static evaluation in the entry S094 (done),
$ grep -n "Retired:\*\* S096" adocs/plan.md
280:**Retired:** S096 check extensions -- Ethereal removed check extensions for
$ ls adocs/plan_*/S096* 2>&1
ls: cannot access 'adocs/plan_*/S096*': No such file or directory
$ sed -n '5p' adocs/plan_todo/S097_singular_extensions.md
excludes:   check extensions, which are S096; any extension not derived from the verification search
```

A full sweep of step ids in `specs.md` finds exactly one such reference: S096 at
line 319. S092 at line 327 is handled correctly ("S092 was retired into S108").

**Impact.** `specs.md` outranks `plan.md` by the stated precedence
(`specs > plan > status`), so on the one document a reader is told to trust,
check extensions are a live step with an id. S097's `excludes:` field — which is
operative, not prose — routes the work there. The effect is that check
extensions have no owner and no recorded retirement in the highest-precedence
document, while `S091:73` ("S096 retired") and `S097:130` both know they were
retired: three files, two answers.

**Suggested resolution.** Rewrite that sentence in `specs.md` to record S096 as
retired by DEC-087 with the reason, in the same shape as the S092 sentence three
lines below it, and change S097's `excludes:` to name the retirement rather than
the id.

---

### 2026-08-20_plan_review-F06 — medium — three pending steps' `touches:` omits the file the change has to land in

Status: planned
Planned in: S141; S085's own third of it fixed 2026-08-20

**Evidence.**

S039, whose entire goal is to change `LAZY_EVAL_MARGIN`, never names the file
that holds it:

```
$ sed -n '2,4p' adocs/plan_todo/S039_lazy_margin_redecide.md
goal:       re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
accepts:    ... the LAZY_EVAL_MARGIN comment in src/evaluation.hpp is re-measured ...
touches:    src/evaluation.hpp LAZY_EVAL_MARGIN and its comment
$ grep -c "search_params" adocs/plan_todo/S039_lazy_margin_redecide.md
0
$ grep -n "LAZY_EVAL_MARGIN" src/search_params.hpp src/evaluation.hpp
src/search_params.hpp:91:  X(LAZY_EVAL_MARGIN,  "LazyEvalMargin",  150,    0, 2000)
src/evaluation.hpp:3:#include "search_params.hpp"  // LAZY_EVAL_MARGIN
$ sed -n '296,298p' src/evaluation.hpp
// The value itself moved to src/search_params.hpp at S073, where it is one of
// the parameters the tune build exposes over UCI. What it means stays here,
```

S085, whose output is a vector of those same defaults:

```
$ sed -n '4p' adocs/plan_current/S085_spsa_first_run.md
touches:    src/search.cpp, src/evaluation.hpp, adocs/plan_done/ on completion
```

S120, whose body requires two edits in a file it does not name:

```
$ sed -n '4p;186p;201p' adocs/plan_todo/S120_eval_cache.md
touches:    src/evaluation.cpp, src/evaluation.hpp, src/data_structures.hpp
  LazyEvalMargin is a setoption (src/search_params.hpp:91): clear the cache on
- **ucinewgame clears it** beside tt_reset (src/chesso.cpp:1091); a 512 KiB
```

**Impact.** `touches:`/`excludes:` is the step's scope contract (AGENTS.md §4)
and the thing a reviewer checks a diff against. S039 as written can re-measure a
comment and never move the number — the step exists to move the number, and
`plan.md` calls it "the architectural prerequisite" for S122. S085 must write its
returned vector into `src/search_params.hpp`; its own body knows this
(`S085:315-316`, "since S073 the shipping defaults live in
`src/search_params.hpp` -- the post-run edit lands there") but the header field
still says otherwise, and the header is what gets read. S120 must edit
`src/chesso.cpp` for `ucinewgame`.

**Suggested resolution.** Add `src/search_params.hpp` to S039's and S085's
`touches:`, and `src/chesso.cpp` to S120's. S039's `accepts:` should name the
value and the comment as two separate obligations in two separate files.

---

### 2026-08-20_plan_review-F07 — medium — S109's accepts contains a clause that cannot bind one of the four rules it gates

Status: planned
Planned in: S139, landed 2026-08-21
Applied: S109's accepts now splits the clause. In check, PV, first move and near-mate
bounds bind all four rules; the gives-check exemption binds futility, history and
quiet SEE only, and explicitly does not bind LMP. Traced to `src/search.cpp:678`
(`is_check_move` computed after `make_move`) with `grep -rn` over `src/` returning
that line and its one consumer at `:710` and nothing else. Scope concern 2 is
rewritten to the narrower owner question it leaves: whether to buy LMP the exemption
with a post-make prune.

**Evidence.**

```
$ sed -n '3p' adocs/plan_todo/S109_shallow_depth_pruning_block.md
accepts:    ... no quiet is pruned while in check, at a PV node, on the first
move, **when the move gives check**, or when alpha or beta is near mate, and the
test asserts the precondition that would otherwise prune it; ...
```

The engine has no pre-make gives-check predicate. In `negamax` the staged move
loop generates quiets at `src/search.cpp:645-662` and the check flag is computed
only after the move is on the board:

```
$ sed -n '677,678p' src/search.cpp
    // is_check() is an attack scan - do not pay for it on captures.
    const bool is_check_move = is_capture ? false : is_check(game);
```

The step's own analysis reaches the same conclusion and says so:

```
$ sed -n '182,186p;359,366p' adocs/plan_todo/S109_shallow_depth_pruning_block.md
- **Does NOT exist**: a pre-make gives-check predicate. `is_check_move` is
  known only after make_move, so the per-move rules (futility, history, SEE)
  run **after** :662 ...
2. **The accepts' gives-check exemption cannot bind LMP as written.** ...
   structurally impossible for a skip-quiets flag honoured at the generation
   stage ... Read the clause as binding the three per-move rules; resolve
   the LMP reading with the owner at implementation -- recorded, not silent.
```

**Impact.** S109 is a one-commit, one-verdict step over four rules. Its accepts
demands a property that one of the four (late move pruning, by the same accepts
required to work through a `skip_quiets` flag the generator honours) cannot have,
and demands a test asserting the precondition. As written the step cannot
complete honestly: either the clause is quietly read down at completion — which
is exactly the "never relax a gate to get green" prohibition — or LMP is
restructured to a per-move `unmake_move + continue`, which the same section
argues away on measured grounds. The clause is flagged in the step body and in
`status.md`, but the `accepts:` field is the gate, and it still says all four.

**Suggested resolution.** Split the clause in the `accepts:` field itself: name
the gives-check exemption as binding futility, history pruning and quiet SEE,
and state for LMP either that no such exemption applies or that it is bought with
a post-make prune. It is a two-line edit to a field, and it needs to happen
before the step starts, not at its completion.

---

### 2026-08-20_plan_review-F08 — medium — `RfpMinPly`'s declared minimum admits values the same file records as red, and the running SPSA has spent a third of that axis there

Status: planned
Planned in: S142, paused behind S145 by DEC-095

**Evidence.**

```
$ sed -n '63,70p' src/search_params.hpp
  /* The top of the tree is searched properly. The root is exempt because its
     answer is the one that gets played; ply 1 and ply 2 are exempt because a
     static bound returned there is what the root compares against alpha, and
     a mate two moves away lives exactly that far down. Measured, not
     assumed: at ply 1 the mate cases in test_search go red, and buying the
     two plies back costs 1.7 % of the nodes the rule saves. Exempting a
     third costs 27 %. S033. */
  X(RFP_MIN_PLY,       "RfpMinPly",       3,      0, 63)
$ sed -n '35,38p' src/search_params.hpp
// The ranges are new metadata and none of them narrows a value that ships. Each
// bound is either arithmetic (a divisor cannot be zero) or the constant's own
// stated purpose (history must stay under a killer), never a guess at where the
// good values are -- that is what the tuner is for.
$ grep -n "RfpMinPly" tools/spsa_s085.json
    { "name": "RfpMinPly",          "start": 3,   "min": 0, "max": 63,    "c_end": 1 },
```

The live run has walked that axis to its floor: S085's own mid-run read at 39 %
records `RfpMinPly` at **0**, with **30.8 %** of iterations spent at the bound
(`adocs/plan_current/S085_spsa_first_run.md`, "Mid-run read at 39 %").

**Impact.** The declared minimum contradicts the comment two lines above it and
the file's own stated rule for how bounds are chosen. Values 0, 1 and 2 are
values no step can ship on a green suite, by the file's own recorded measurement,
so about a third of one axis's gradient budget in a 30000-pair overnight run has
been spent exploring inadmissible territory — and the frozen configuration means
it cannot be corrected without discarding the run. The post-run decision (reject
the vector, hold one axis at a floor nobody measured, or narrow the bound and owe
a rerun) is a cost this defect created.

Independently re-derived here; the same defect is recorded inside the step file
at `1f935d1`. Recorded as a finding because it is open, because it is the class
CLAUDE.md says jumps the queue, and so a fix step can name it.

**Addendum, uncommitted working-tree change, same day.** After this finding was
written, `src/search_params.hpp` was edited in the working tree by another
session, replacing the comment quoted above. The replacement narrows two of its
claims from measurement: the root is exempted by `!is_pv` at
`src/search.cpp:521` rather than by this parameter, so 0 and 1 are the same
engine; and "at ply 1 the mate cases in test_search go red" was 3 of 18 cases,
not all of them, with the tested floor being 2 rather than the argued 3. **None
of that closes this finding** -- the new text states in its own words that "0 and
1 cannot ship" while leaving `X(RFP_MIN_PLY, "RfpMinPly", 3, 0, 63)` unchanged,
so the declared minimum still admits values no step can ship and the live run has
still spent 30.8 % of that axis at one of them. What changes is the wording a
fix has to cite: quote the working-tree comment, not the one above, once it is
committed.

**Suggested resolution.** Raise `RFP_MIN_PLY`'s declared minimum to the floor
its purpose sentence implies, with the red-at-ply-1 measurement cited beside it,
and sweep the other 21 rows for the same class — F14 below is a second instance.
Then decide S085's post-run handling as a recorded decision, not a judgement
call.

---

### 2026-08-20_plan_review-F09 — low — S085's goal names twenty parameters; the live surface is 22 and the frozen run is 12

Status: planned
Planned in: S085, resolved by DEC-094 -- goal line amended to the frozen 12

**Evidence.**

```
$ sed -n '2p' adocs/plan_current/S085_spsa_first_run.md
goal:       the first SPSA run, over the twenty search parameters that exist today, ...
$ grep -cP '^\s*X\(' src/search_params.hpp
22
$ python3 -c "import json;print(len(json.load(open('tools/spsa_s085.json'))['params']))"
12
```

The step body flags both (`S085:130`, "The live surface is **22**, not the goal's
twenty"; `S085:309-311`), and `status.md`'s newest parked item banks it as an
owner question.

**Impact.** The goal line is what `plan.md`'s list entry 13 repeats and what
`status.md`'s "In progress" line prints, so the wrong count is the one three
documents show. On completion the `done:` stamp has to carry a deviation from the
goal in both directions at once: the surface is larger than stated and the run is
smaller.

**Suggested resolution.** Amend the goal line to the frozen scope ("over twelve
of the twenty-two exposed search parameters") in the same commit as the plan.md
entry, or record the deviation as a decision. Not both left implicit.

---

### 2026-08-20_plan_review-F10 — low — three `excludes:` fields route work to retired step ids

Status: planned
Planned in: S140

**Evidence.**

```
$ sed -n '5p' adocs/plan_todo/S098_reduction_refinement.md
excludes:   late move pruning, which is S090; the improving flag itself, which is S092 and is an input here
$ sed -n '5p' adocs/plan_todo/S097_singular_extensions.md
excludes:   check extensions, which are S096; any extension not derived from the verification search
$ sed -n '5p' adocs/plan_todo/S042_en_passant_only_when_capturable.md
excludes:   any other zobrist change, and S031's side-to-move key
$ grep -n "the retired S019, S026, S031" adocs/plan.md
297:creation order and never renumbered or reused -- including for the retired S019, S026,
```

S090 was retired into S109, S092 into S108, S096 outright (DEC-087), S031
outright. All other references to retired ids in pending files are explicit
"folded in from the retired Sxxx by DEC-086" provenance notes and are correct as
history.

**Impact.** `excludes:` is operative — it tells the implementer where the
excluded work lives. Three of them point at ids with no file and no successor
named. S098's is the live one: it calls the improving flag "S092 and an input
here", where S108 is the step that supplies it and is ordered two entries
earlier.

**Suggested resolution.** Re-point each to the live owner: S098 → "late move
pruning, which is S109; the improving flag, which S108 supplies and this step
consumes"; S097 and S042 → name the retirement instead of the id.

---

### 2026-08-20_plan_review-F11 — low — `plan.md`'s block 3 prose omits all three of S134, S135 and S136

Status: planned
Planned in: S140

**Evidence.**

```
$ grep -n "S134\|S135\|S136" adocs/plan.md
358:40. S134  delete rook-on-the-seventh and passer bucket 5 ...
361:43. S135  unfreeze the piece placement group and refit it ...
362:44. S136  unfreeze tempo, re-derive the truncation guard ...
$ sed -n '245p' adocs/plan.md
**Block 3, the corpus and the evaluation, S082 to S126.** Corpus first, because
```

The block-3 paragraph enumerates thirteen entries (S082, S083, S039, S121, S123,
S125, S118, S101, S122, S124, S102, S133, S126) and none of the three created by
`da8b0df` on 2026-08-20. The header names S082 as the block's start while the
list's block 3 opens at S134 (entry 40).

**Impact.** `plan.md`'s prose is the reader's map of the order and it is what
`plan.md` itself says the list numbers do not replace. Three of the block's
sixteen entries are invisible to it, including S134 — the only pending step that
declares `blocks:` (S135) and the one whose landing changes `PARAM_COUNT` for
every step below it. `plan.md`'s "What this costs" paragraph is also silent on
their verdicts.

**Suggested resolution.** Rewrite the block-3 paragraph to include the three
steps and correct the header range; check the verdict count in "What this costs"
against the amended block.

---

### 2026-08-20_plan_review-F12 — low — S115's accepts names a results file as the source of its 300 positions

Status: planned
Planned in: S138, landed 2026-08-21

**Evidence.**

```
$ sed -n '3p' adocs/plan_todo/S115_aspiration_refinements.md
accepts:    ... the node-count sweep is run over the 300 stratified positions of
adocs/data/S021_aspiration_sweep.tsv and not over three, ...
$ head -1 adocs/data/S021_aspiration_sweep.tsv; wc -l adocs/data/S021_aspiration_sweep.tsv
sample	min_depth	delta	max_delta	nodes	rel	moves_changed
64 adocs/data/S021_aspiration_sweep.tsv
```

That file is 63 result rows (21 schedules × 3 samples) and carries no FEN column.
The positions come from `adocs/data/S018_raw.tsv` through
`adocs/data/S021_aspiration_sweep.py`, which S115's own body gets right at lines
94-95. `PER_PHASE = 4` in that script over 25 distinct phase values in
`S018_raw.tsv` gives 100 positions per offset, three offsets, 300 —
`awk` over the phase column confirms 25 distinct values. The script's own
docstring is also stale on this ("Positions are 32 sampled from
adocs/data/S018_raw.tsv", line 17).

**Impact.** As written the accepts clause cannot be satisfied — the named file
holds no positions. The clause exists because S021 recorded that one sample
chooses the wrong setting, so it is load-bearing, not decorative.

**Suggested resolution.** Point the accepts at
`adocs/data/S021_aspiration_sweep.py` with the three offsets, matching the
step's own body, and correct the script's docstring while the step is open.

---

### 2026-08-20_plan_review-F13 — low — two accepts clauses gate on "the mate-in-quiescence case", a test title that does not exist

Status: planned
Planned in: S138, landed 2026-08-21

**Evidence.**

```
$ grep -n "mate-in-quiescence" adocs/plan_todo/*.md
S112:3, S112:189, S131:3, S131:137
$ grep -n "TEST_CASE" tests/test_search.cpp | grep -i "mate"
84 "mate in one" / 154 "a mated side reports mate in zero" / 171 "stalemate scores
zero, not mate" / 187 "a balanced position is not a mate" / 775 "mate is recognised
at depth zero" / 881 ... / 1266 ... / 1625 ... / 1887 "pruning does not hide a
forced mate" / 1948 "which material can still mate"
$ sed -n '137p' adocs/plan_todo/S131_quiescence_promotions.md
   so the fast-suite mate-in-quiescence case ("a side in check may not stand
$ grep -n 'TEST_CASE.*stand pat' tests/test_search.cpp
701:  TEST_CASE_FIXTURE(search_fixture_t, "a side in check may not stand pat")
```

No test carries the name the two accepts clauses use. S131's body resolves it to
`"a side in check may not stand pat"` (line 701); the nearest match to the phrase
itself is `"mate is recognised at depth zero"` (line 775), which is the case that
exercises mate detection with only quiescence running. S112's body does not
disambiguate.

**Impact.** Two steps' completion gates name a test by description. Whoever
completes them picks a case, and the two steps can pick different ones — which
is how a gate stops covering what it was written for.

**Suggested resolution. ** Replace the description with the exact test title in
both accepts clauses, quoted the way `S109` quotes `"pruning does not hide a
forced mate"`.

---

### 2026-08-20_plan_review-F14 — low — `ORDER_HISTORY_MAX`'s declared range breaks the band clearance S093's accepts reasons from

Status: planned
Planned in: S142

**Evidence.**

```
$ sed -n '41,45p' src/search_params.hpp
  /* Keeps an accumulated history score from ever outranking a killer. The
     upper bound is that sentence: a killer scores 900000, so a history score
     allowed past it would invert the ordering silently. */
  X(ORDER_HISTORY_MAX, "OrderHistoryMax", 600000, 0, 899999)
$ sed -n '33,37p' src/evaluation.cpp
#define ORDER_TT_MOVE 2000000
#define ORDER_CAPTURE 1000000
#define ORDER_KILLER_0 900000
#define ORDER_KILLER_1 800000
#define ORDER_COUNTER 700000
$ sed -n '3p' adocs/plan_todo/S093_history_malus_and_ageing.md
accepts:    ... the gravity keeps every score inside ORDER_HISTORY_MAX so the
move-ordering bands still clear each other by 100 points, with the band clearance
asserted ...
```

There are two killer bands and a countermove band, at 900000, 800000 and 700000.
A value of 899999 — inside the declared range and settable over UCI on the tune
build — outranks both the second killer slot and every countermove, so the
comment's "from ever outranking a killer" holds for one of the two killers only.

**Impact.** S093's accepts derives band safety from a bound that does not
provide it: "inside `ORDER_HISTORY_MAX`" implies the bands clear only at the
shipped default of 600000, not over the parameter's declared range. Since the
same accepts requires the clearance to be *asserted by a test*, the test will be
written against the reasoning rather than the range, and a later sweep or SPSA
run over `OrderHistoryMax` inverts a countermove against a history score with no
red test — the CLAUDE.md hazard, whose symptom is a strength regression and not a
wrong node count. `OrderHistoryMax` is not in S085's frozen twelve, so nothing is
exploring it today.

**Suggested resolution.** Either lower the declared maximum to 699999 so the
sentence holds against the lowest band, or restate the comment and S093's accepts
in terms of the band the bound actually protects, and assert the clearance
against `ORDER_COUNTER` rather than against a killer.

---

### 2026-08-20_plan_review-F15 — low — S117's accepts admits a truncation change with "no SPRT owed", which INV-6 and its own body forbid

Status: planned
Planned in: S139, landed 2026-08-21
Applied: the "or" branch is gone. S117's accepts makes the identity a gate rather
than a claim, requires the truncation behaviour unchanged, calls a moved bound a
packing bug rather than a re-pinned tolerance, and says a truncation change retained
deliberately owes an SPRT under INV-6 (`adocs/specs.md:80-87`). The "do S055 first or
fold it in" open question in the body is closed to "S055 first", which its own
Interactions section had already decided.

**Evidence.**

```
$ sed -n '3p' adocs/plan_todo/S117_swar_packed_eval_score.md
accepts:    identical scores from `bench_eval`'s checksum ... -- **behaviour-neutral,
so no SPRT is owed** (INV-6, DEC-083); ... the truncation behaviour is unchanged
**or the change is stated and test_eval_model's tolerance is re-pinned** rather
than relaxed; ...
$ sed -n '165,167p' adocs/plan_todo/S117_swar_packed_eval_score.md
did), spread recorded ... Nothing should break identity; if
node counts differ, the change has a bug -- stop and fix, never fall back to
an SPRT.
```

INV-6 (`specs.md:80`): a change claimed behaviour-neutral proves it with
identical node counts and best moves and is not sent to a match; a change that
alters play is retained only with an SPRT verdict.

**Impact.** The accepts offers a branch its own body rules out. A changed
truncation changes `evaluate()`, changes the tree, and is therefore
play-altering — which INV-6 says needs a verdict and the same accepts clause says
does not. An implementer following the field alone can ship a play-altering
change on a re-pinned tolerance and no match.

**Suggested resolution.** Delete the "or" branch: require the truncation
behaviour to be unchanged, and state that a change in it is a bug in the packing,
which is what the body already says.

---

### 2026-08-20_plan_review-F16 — low — `status.md`'s enrichment census disagrees with the filesystem it says derives it

Status: planned
Planned in: S140

**Evidence.**

```
$ grep -L 'Technical details (SOTA research' adocs/plan_todo/*.md | wc -l
28
$ grep -L 'Technical details (SOTA research' adocs/plan_todo/*.md | xargs -n1 basename | sed 's/_.*//' | sort | tr '\n' ' '
S023 S025 S029 S030 S032 S039 S042 S082 S083 S101 S102 S110 S111 S118 S119
S121 S122 S123 S124 S125 S126 S127 S128 S129 S133 S134 S135 S136
$ grep -A4 "Not yet enriched" adocs/status.md
    28 of 48. Not yet enriched: S119, S042, S032, S030, then blocks 3 and 4
    (S082, S083, S039, S121, S123, S125, S118, S101, S122, S124, S102, S133,
    S126, S127, S128, S129).
```

The stated list is 20 ids; the filesystem gives 23 once the five excluded by
design (S023, S025, S110, S111, S029) are removed. The three missing are S134,
S135 and S136, created by `da8b0df` after the item was written. There are 51
pending steps, so "28 of 48" is stale in both numbers (23 enriched of 51, or 23
of 46 excluding the reserve and the parked one).

**Impact.** Low on its own — the item names the derivation, so a reader can
recompute. Recorded because the same file, two items further down, warns at
length that "a worked example here reads as a measurement while being a memory",
and this is that failure in the item above it.

**Suggested resolution.** Drop the counts and the enumeration from the parked
item and keep only the `grep -L` recipe, which is correct and self-updating.

---

### 2026-08-20_plan_review-F17 — low — INV-7 is referenced by the repository's own history and defined nowhere

Status: planned
Planned in: S140

**Evidence.**

```
$ git log --oneline --all --grep="INV-7"
68a61d0 Restore S084's landed step file (INV-7)
$ grep -rn "INV-7" --include=*.md --include=*.cpp --include=*.hpp --include=*.py --include=*.sh . | grep -v '^./.git'
(no output)
$ grep -n "^- \*\*INV-" adocs/specs.md
64:- **INV-1 ... 68:INV-2 ... 71:INV-3 ... 74:INV-4 ... 77:INV-5 ... 80:INV-6
```

**Impact.** `specs.md` is the numbering authority for invariants (AGENTS.md §3)
and stops at INV-6. A landed commit claims to enforce an INV-7 that no document
states, so the property it was defending — from the message, that
`plan_done/` is never rewritten or trimmed — is asserted by history and testable
by nothing. It also has no `testing.md` row, which every invariant is supposed to
have.

**Suggested resolution.** Either add INV-7 to `specs.md` with a testable
statement and a `testing.md` row, or correct the record that the commit was
enforcing AGENTS.md §2/§10 rather than a numbered invariant.

---

### 2026-08-20_plan_review-F18 — low — S085's stated watcher command names a path that does not exist and a ceiling that is not 2x its own budget

Status: planned
Planned in: S085, watcher path corrected 2026-08-20

**Evidence.**

```
$ sed -n '180,181p' adocs/plan_current/S085_spsa_first_run.md
`python3 bin/moltke.py --watch <log> 'SPSA-(DONE|FAILED)' --ceiling 14h --pid <pid>`
(ceiling 2x the budget, AGENTS.md par.12, DEC-061).
$ ls -d /home/max/ws/chesso/bin
ls: cannot access '/home/max/ws/chesso/bin': No such file or directory
$ grep -n "8.15 h" adocs/plan_current/S085_spsa_first_run.md | head -1
475:games in 8.15 h** -- one night, and twice the accepts' floor for the same
$ python3 -c "print(2*8.15)"
16.3
$ cat .git/moltke_watch/*.json | grep -E '"ceiling"|"log"'
  "log": "/home/max/ws/chesso/.tuning/spsa_S085.log",
  "ceiling": "18h",
```

**Impact.** The command as written cannot run in this repository — the primitive
lives in the installed plugin — and its stated ceiling of 14 h is below the 2x
of the file's own 8.15 h budget that the sentence beside it claims. The watcher
actually armed used 18 h, so nothing is unbounded in practice; the defect is that
the step file records a non-conforming command as the conforming one, and it is
the record a later step copies. (The `bin/moltke.py` path is inherited from
AGENTS.md §12 and is not S085's invention.)

**Suggested resolution.** Correct the ceiling in the step file to the armed 18 h
and state the primitive's path the way it is actually invoked, or make the ceiling
derived ("2x the budget in the paragraph above") so it cannot fall out of step.

---

## Checked and clean

Stated so a re-run knows what was already settled, and so the report is not read
as a list of everything examined.

- **`plan.md` list ↔ filesystem correspondence is exact.** 51 pending step files
  (50 `plan_todo/` + 1 `plan_current/`), 51 pending list entries, 5 retained
  completed entries (S106, S107, S100, S084, S137 — the last five in list order,
  as the retention rule states), no duplicate ids, no list entry without a file,
  no pending file without an entry.
- **`status.md`'s four derived fields agree with the filesystem and with the
  generator.** "Last done: S137" is the last completed entry in list order;
  "In progress"/"Next" are S085; "Blocked: none" is correct because the
  generator derives it from `paused_by` on `plan_current/` only
  (`moltke.py:2317-2342`), and S134's `blocks: S135` does not enter it. The
  `Watching:` line matches `.git/moltke_watch/1787246665_1476278.json`.
- **S134's arithmetic holds.** `PARAM_COUNT` 827 and post-fold 823, computed from
  the counts in `tools/eval_model.hpp` (5 + 768 + 8 + 18 + 12 + 6 + 8 + 2).
- **S085's frozen configuration satisfies its own accepts on the two clauses that
  can be checked without running anything.** `1250 × 24 = 30000` paired games
  meets the "at least 30000 paired games" floor exactly; the run's `tc=2+0.02`
  and `books/UHO_4060_v3.epd` both differ from the harness's `8+0.08` and
  `UHO_Lichess_4852_v1.epd`, so the "a time control and an opening book the run
  did not use" clause is satisfiable.
- **S108's central code claim is true.** `tt_store_entry` does write
  `TT_EVAL_NONE` over a stored evaluation (`src/transposition_table.cpp:135-139`),
  so "the table entry no longer overwrites a stored evaluation with
  TT_EVAL_NONE" is a real, satisfiable obligation. `specs.md:178` records the
  same thing.
- **S095's code claim is true.** `tt_get_entry` returns `nullptr` on a key
  mismatch (`src/transposition_table.cpp:95-102`).
- **77 of the 147 line citations still resolve** to the text they were written
  against; the drift in F01 is concentrated in four files.
- **Dependency order holds everywhere except F04.** S112 before S022, S108 before
  S109 and S098, S093 before S098, S120 before S039 before S122, S118 before
  S122, S134 before S135, S082/S083 before S135/S126/S133, S055 before S117,
  S099 before S110, S023 before S025, S085 before S127 — all consistent with the
  list order.
