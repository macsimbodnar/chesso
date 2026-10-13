id:         S278
goal:       the sanitizer build's `test_tuner_gradient` and `test_eval_trace` run clean: UBSan's "store to null pointer of type 'struct record_t *'" at `tools/tuner_trace.hpp` `trace_board`'s `eval_trace::sink = &record;` is explained and fixed, and `tools/gate_extra.sh`'s sanitize stage is green again
accepts:    (1) the failure reproduced first on `f6647dc` with a minimized failing case, observed, not assumed; (2) the cause stated -- the thread_local `eval_trace::sink` defined in `src/evaluation.cpp` under `CHESSO_TRACE` and written from the tools' translation units -- and whether any non-sanitizer build is affected (the trace reconstructed `evaluate()` on all 10795695 rows in S262, S263 and S276, so the Release path works; say why the sanitizer build differs); (3) the fix changes no `chesso` binary: `evaluation.cpp`'s object in the plain build byte-identical, bench 3872032; (4) `tools/gate_extra.sh` all five stages green, its sanitize stage's bench check included
touches:    src/eval_trace.hpp, src/evaluation.cpp (the trace-only definition, under `CHESSO_TRACE`), tools/tuner_trace.hpp, cmake/ or tools/CMakeLists.txt if the trace library's sanitizer flags are the cause, tests/ for the minimized case
excludes:   any change to `evaluate()`'s arithmetic or to the shipped binary; the tuner's fit path
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S278_brief.md), 2026-10-13
done:       2026-10-13 -- the sanitizer build's trace tests run clean; gate_extra all five stages green (834 s), and its sanitize stage again on the final tree: 46 of 46, both benches 3872032. The UBSan "store to null pointer" on trace_board()'s sink write was a false report at a valid address. Outside evaluation.cpp, the plain `extern thread_local` went through GCC's TLS wrapper, the null check sat in the flags of `add sink@gottpoff(%rip), %reg`, and ld's initial-exec to local-exec rewrite made that add a flagless lea. The je read the wrapper's weak-init test, ZF=1 (gdb: rax == &sink, not null). Only -fsanitize=null reads those flags: 10 such sites in the parent's sanitizer objects (6 test_tuner_gradient, 2 test_eval_trace, 2 feature_audit), 0 in every Release and tune object, 0 unresolved anywhere (adocs/data/S278_tls_flag_scan.py, which walks past padding). The trace's 0 of 10795695 rows stands. Fix: constinit on the sink's declaration and definition, which removes the wrapper; 0 sites and no wrapper reference in any tree after. Guards: the minimized case (one trace_of() of the start position), red on the parent's sanitizer build and green after; and test_trace_sink_tls, an nm check over every object, red on the parent's sources in all three trees and with constinit off the declaration alone, green with the fix. evaluation.cpp's Release and tune objects byte-identical to the parent's, engine and trace library both; bench 3872032; the sanitizer trace library's object moves by register choice from constinit itself, not shipped. Fast suites 46/46 both, format clean, prose checks clean. DEV_MANUAL gains the false-report trap; MANUAL checked, no change. No second tier owed beyond gate_extra (no make_move, generator, search or pruning change). Fast check: the fix sound; four items fixed before commit (the evidence moved into the step's directory, the scanner walks past padding, a deterministic nm guard in every build, the DEV_MANUAL wording).

## Why this exists (2026-10-13, the coordinator, from S099 (a)'s second tier)

S099 (a)'s `gate_extra.sh` run had its sanitize stage red: `test_tuner_gradient`
and `test_eval_trace` stop at UBSan "store to null pointer of type 'struct
record_t *'" on `eval_trace::sink = &record;` in `tools/tuner_trace.hpp`
(`trace_board`). The same two tests fail on the same line on a clean `5ac17bd`
worktree, 43 of 45 passing on both, so S099 did not cause it. The sink is
`thread_local` (`src/eval_trace.hpp`, defined in `src/evaluation.cpp`). The
last green `gate_extra` was S119's, on `298788b`, before S262 added the trace;
S262, S275 and S276 are tooling steps that ran no second tier, which is how it
went unseen. The sanitizer build's bench, run by hand, prints 3872032 with no
report.

## Lane

Agent work, tooling; minutes of machine. Filler under DEC-171: not reachable
in play, on the UCI surface, or by a reported score, move or line. Named as an
open finding in the pre-registration of every run taken while it is open.


## As built (2026-10-13)

Evidence is in `.tuning/coord/S278/` (gitignored; backed up in the main
tree's `.tuning/coord/S278_files/`). `phases.sh` rebuilt `build`, `build-tune`
and `build-sanitize` with ccache off, three times: `parent` (the two `src/`
files at `27eb114`), `header` (the fix with `constinit` taken off the
declaration only, `build` alone) and `fixed`. Each phase wrote
`<phase>_sources.txt`, `<phase>_objects.txt` (object hashes, wrapper
references, the scan, the guard test, bench), `<phase>_min_case.txt` and
`<phase>_sanitize_*.txt`. `parent_gdb.txt` is the register reading.
`session1/` holds the first pass's logs: the first reproduction, the
`--no-relax` run, both `gate_extra` runs and the TESTS gate runs.

### Deviations

- **Reproduced on `27eb114`, achesso's head, not on `f6647dc`.** The two
  commits between them change documents only: `git diff f6647dc 27eb114 --
  src tools tests CMakeLists.txt cmake` is empty.
- **The cause is the toolchain, not the trace library's flags or its
  linking.** `touches:` allowed for a CMake cause. There was none, and nothing
  under `cmake/` or `tools/` changed. `tools/tuner_trace.hpp` is unchanged too:
  the fix sits on the sink's declaration and definition.
- **The minimized case goes through `trace_of()`, not through a bare write.** A
  first case wrote `eval_trace::sink` directly in a fresh test case, through
  `trace_board()` inlined into it, and it *passed* before the fix: there GCC
  computed the address with `add %reg, %reg`, whose flags are its own. Observed
  and dropped. Whether a write site shows the defect depends on GCC's register
  choice at that site (below).
- **A second guard, `test_trace_sink_tls`, deterministic in every build**
  (the coordinator's fast check asked for it): the doctest case can bite only
  in `build-sanitize` and only with this GCC's register choice.
- **No standalone two-file reproduction.** Five variants built with the
  sanitizer build's exact flags (`-O2 -g -march=native -fsanitize=address
  -fsanitize=undefined -fno-sanitize-recover=undefined
  -fno-omit-frame-pointer`), with register pressure forced up by `-ffixed-`,
  all kept the register form and passed. The evidence is the project's own
  objects and binary instead.
- **`build-sanitize`'s trace-library `evaluation.cpp.o` changes with the fix**
  (below); every Release and tune object is byte-identical.
- **Files outside `touches:`**: `DEV_MANUAL.md` (one paragraph in the
  `gate_extra` section, beside the compile-failure trap of 2026-09-13),
  `adocs/data/S278_tls_flag_scan.py` (new) and its row in
  `adocs/data/README.md`.

### The minimized failing case, observed before the fix

`tests/test_eval_trace.cpp`, "one position traced from outside
evaluation.cpp": one `trace_of(DEFAULT_POSITION)`, then the row is non-empty and
the sink is cleared. Built in `build-sanitize` (RelWithDebInfo, `SANITIZER=ON`)
on the parent's sources and run alone, `-tc="one position traced from outside
evaluation.cpp"`, with `UBSAN_OPTIONS=print_stacktrace=1`: **exit 1**,
`runtime error: store to null pointer of type 'struct record_t *'` on
`trace_board()`'s `eval_trace::sink = &record;`, frames `trace_board` <-
`trace_of` <- the new case (`session1/min_before.txt`, again in
`parent_min_case.txt`). The whole binaries: `test_eval_trace` and
`test_tuner_gradient` stop on the same statement (`parent_sanitize_*.txt`).
With the fix the case exits 0, `test_eval_trace` passes 1196416 assertions and
`test_tuner_gradient` 22205, with no runtime error in either
(`fixed_min_case.txt`, `fixed_sanitize_*.txt`).

### The cause, with the evidence

The sink's declaration in `src/eval_trace.hpp` was `extern thread_local
record_t* sink;`. A translation unit that sees only that line cannot know the
sink needs no dynamic initialisation. So GCC reaches it through the TLS
wrapper: test the weak TLS init function `_ZTHN10eval_trace4sinkE` for null,
call it if present, then compute the address. On the parent's sources five
objects in each tree refer to that init function: `test_eval_trace.cpp.o`,
`test_tuner_gradient.cpp.o`, `tuner.cpp.o`, `feature_audit.cpp.o` and
`truncation_scan.cpp.o` (`parent_objects.txt`). Nothing defines it, so its GOT
slot is 0. `-fsanitize=null` checks the reference the wrapper yields, and the
defect is where GCC took that check from: in `trace_of()`, with
`trace_board()` inlined, the object reads (GCC 13.3.0, Ubuntu
13.3.0-6ubuntu2~24.04.1; symbols abbreviated):

```
cmpq   $0x0,0x0(%rip)      R_X86_64_GOTPCREL  _ZTHN10eval_trace4sinkE   ZF=1: no init function
movb   $0x0,%fs:0x0        R_X86_64_TPOFF32   trace_board::record guard
je     <skip the call>
call   _ZTHN10eval_trace4sinkE
mov    %r13,%rax                                                    r13 = the thread pointer
add    0x0(%rip),%rax      R_X86_64_GOTTPOFF  eval_trace::sink      rax = &sink, ZF iff null
je     <__ubsan_handle_type_mismatch_v1_abort>
mov    0x0(%rip),%rcx      R_X86_64_GOTTPOFF  eval_trace::sink
lea    0x0(%r13),%rax      R_X86_64_TPOFF32   trace_board::record
mov    %rax,%fs:(%rcx)                                              the store itself
```

The null check is the flags of the `add`. When it links an executable, ld (GNU
ld 2.42) rewrites initial-exec TLS to local-exec, and it rewrote that `add`
into `lea -0x8(%rax),%rax`, which sets no flags. The `je` then reads ZF from
the `cmpq` three instructions earlier. With no init function that ZF is 1, so
the jump to the UBSan handler is taken on every call.

- **gdb at that `je`** (`parent_gdb.txt`; the linked binary's listing is in
  the file, the jump at `0x288cc0`, first stop in the minimized case):
  `rax = 0x7ffff7f6c9f8`, and `p &eval_trace::sink` prints `0x7ffff7f6c9f8`;
  `r13 = $fs_base = 0x7ffff7f6ca00`; `eflags = [ PF ZF IF ]`. The address was
  valid and the flag was stale.
- **`-Wl,--no-relax` does not change it** (`session1/run_norelax.txt`): the
  same objects relinked with it carry the same `lea` and stop with the same
  report. ld applies the TLS transition whatever `--no-relax` says.
- **The sites, counted.** `adocs/data/S278_tls_flag_scan.py` walks forward
  from every `add ...@gottpoff` past moves, `lea`, nops and padding prefixes
  to the first instruction that reads or writes the flags or transfers
  control. A reader is a site, and anything it cannot classify is counted as
  unresolved, never as safe. `cmake/arch.cmake` passes
  `-Wa,-mbranches-within-32B-boundaries`, so the assembler can put a `nopw`
  between the add and its `je`. That is why the scan must walk: S278's first
  scanner read only the next instruction and missed one such site. On the
  parent's sources `build-sanitize` holds **10 sites, 0 unresolved, in 61
  objects**, all on the sink and all `je`: 6 in `test_tuner_gradient.cpp.o`, 2
  in `test_eval_trace.cpp.o`, 2 in `feature_audit.cpp.o` (`parent_objects.txt`;
  `feature_audit` runs in no test, so nothing reported those two). The other
  sink accesses in those objects used `add %reg, %reg` and were sound. That is
  why the bare write in the dropped first case passed.

**Why only the sanitizer build.** The flag reader is the null check, and only
`-fsanitize=null` (in `undefined`) emits one. The plain builds compute the same
address through the same rewritten `lea` and store through it, with nothing
after it reading flags. The scan reads **0 sites, 0 unresolved** in all 61
objects of `build` and of `build-tune`, on the parent's sources and on the
fix's. That fits S262, S263 and S276 reading 0 of 10795695 rows different from
`evaluate()`. No fit, audit or reading took a wrong value from this. It is a
false report from a check, and the trace was right.

### The fix

`constinit` on the declaration (`extern constinit thread_local record_t*
sink;`, with the reason at the site) and on the definition in
`src/evaluation.cpp` (C++20 [dcl.constinit]: the initialising declaration
carries it too, and the compiler then enforces the constant initialisation the
declaration promises). Every translation unit now knows the sink is constant
initialised. So there is no TLS wrapper and no `_ZTH` reference, and the access
is direct, which UBSan does not null-check. With the fix, in all three trees: 0
objects refer to `_ZTHN10eval_trace4sinkE`, and the scan reads 0 sites, 0
unresolved, in 61 objects (`fixed_objects.txt`).

**The deterministic guard, `test_trace_sink_tls`**
(`tests/test_trace_sink_tls.sh`, fast label). It runs `nm` over every object
under the build directory and fails when any refers to
`_ZTHN10eval_trace4sinkE`. Before asserting that
absence it establishes the precondition: one object defines the sink and at
least one other refers to it from outside, so the files that would need the
wrapper exist and the absence means something. Every object is read, so a new
trace consumer is covered without being named. Observed:

- **red** on the parent's sources in `build`, `build-tune` and
  `build-sanitize`, naming the five objects (`parent_objects.txt`);
- **red** with `constinit` taken off the declaration alone, the definition
  keeping it, in `build` (`header_objects.txt`). That is the regression a
  later edit to the header would make;
- **green** with the fix in all three: `ok: 61 objects, 1 defining the sink, 5
  referring to it, none through a TLS wrapper` (`fixed_objects.txt`).

Not taken: accessor functions in `evaluation.cpp`, which add an out-of-line
call and change every write in the tests; `-fno-extern-tls-init` on the trace
targets, a GCC-only flag in CMake over every extern TLS variable, for one
variable that `constinit` describes exactly.

### INV-6 and the shipped binary

`evaluation.cpp.o`, rebuilt with ccache off, parent's sources against the fix
(`parent_objects.txt`, `fixed_objects.txt`):

| tree | `chesso_engine` | `chesso_engine_trace` |
|---|---|---|
| `build` (Release) | `e5646441c5d9...` both, **byte-identical** | `dcabb363d31e...` both, byte-identical |
| `build-tune` | `90ea88fc53b3...` both, byte-identical | `eb3eea30a8c7...` both, byte-identical |
| `build-sanitize` | `0a42daa14a10...` both, byte-identical | `c59c830ece59...` against `36bee2fe5e45...` |

- Both edits to `chesso_engine`'s sources sit inside `#ifdef CHESSO_TRACE`.
  The shipped object is the parent's, and so is every trace object in the two
  gated builds.
- The one object that moves is the sanitizer build's trace library, which no
  engine binary links. A compile controlled for the source path shows the move
  is `constinit` itself, not the header's added comment lines. The fix with
  the comment removed compiles to the same instructions as the fix with it;
  the parent at two path lengths compiles identically. The difference is
  register choice in `evaluate_cheap()` and `evaluate_expensive()`, with the
  same six TLS accesses and the same 463 sanitizer calls, and its trace still
  reconstructs `evaluate()` on every position of `test_eval_trace`.
- `bench` **3872032** on the parent's sources and on the fix
  (`parent_objects.txt`, `fixed_objects.txt`).

### Gates

- TESTS gate, `CLANG_FORMAT_MAJOR=22` exported, on the final tree: `build`
  fast 46 of 46 and `build-tune` fast 46 of 46, `test_trace_sink_tls` among
  them; `./clang-format.sh --check` clean; exit 0 (`tests_gate3.log`).
- `STAGES=sanitize tools/gate_extra.sh` on the final tree,
  **`GATE-EXTRA-DONE 1 stages 414 s`**: 46 of 46 under ASan and UBSan
  (`test_tuner_gradient`, `test_eval_trace` and `test_trace_sink_tls` among
  them), no sanitizer report in the log, `sanitizer bench: 3872032 nodes`,
  `release bench: 3872032 nodes`, `INV-6 across builds: both 3872032 nodes`
  (`gate_extra_sanitize/`).
- All five stages, run before the fast check's additions. The fix and the
  minimized case have not changed since. The additions are a test script and
  its registration, the scanner and two documents, and none of them is read
  by the debug, perft, prose or citations stage, apart from the prose checks
  below. **`GATE-EXTRA-DONE 5 stages 834 s`** (`session1/gate_extra_run2/`):
  prose ok (0 s); citations ok (1 s); debug ok (344 s, 6 of 6); sanitize ok
  (435 s, 45 of 45, both benches 3872032); perft ok (54 s, 1 of 1).
- The run before that was red on this step's own record, not on the code
  (`session1/gate_extra_run1/`): sanitize 44 of 45, with
  `test_plan_citation_freshness` flagging two line-form citations (DEC-135) in
  this As built's quotation of the UBSan report. Both trace tests passed in
  it. The quotation now names the symbol.
- The first fast-label run, without the export, had `test_clang_format_script`
  red on "clang-format 23 not found" (DEC-146's machine override,
  `session1/fast_build.log`). It is green with the export. The format check
  then flagged the new case's `TEST_CASE_FIXTURE` line, and `clang-format-22
  -i` wrapped it.
- `tools/plan_prose_check.py`, one mode per call, on the final tree:
  `--prose` 0 sentences flagged; `--citations` 0 flagged over 32 files;
  `--touches` 0 flagged over 32 files.
- No second tier is owed by DEC-141 clause 1 or 2: no `make_move`, generator,
  search or pruning change. Clause 3's `gate_extra` is the runs above.
- DEV_MANUAL updated (above, and it names the new test). MANUAL.md checked:
  no UCI or behaviour change, no edit.

### For the coordinator

- `status.md`: S278 closes the open DEC-171 finding that every
  pre-registration names. `gate_extra` is green on the S278 tree, the first
  full green since S119's `298788b`.

### Proposed `done:` stamp

2026-10-13 -- the sanitizer build's trace tests run clean; gate_extra all five
stages green (834 s), and its sanitize stage again on the final tree: 46 of 46,
both benches 3872032. The UBSan "store to null pointer" on trace_board()'s sink
write was a false report at a valid address. Outside evaluation.cpp, the plain
`extern thread_local` went through GCC's TLS wrapper, the null check sat in the
flags of `add sink@gottpoff(%rip), %reg`, and ld's initial-exec to local-exec
rewrite made that add a flagless lea. The je read the wrapper's weak-init test,
ZF=1 (gdb: rax == &sink, not null). Only -fsanitize=null reads those flags: 10
such sites in the parent's sanitizer objects (6 test_tuner_gradient, 2
test_eval_trace, 2 feature_audit), 0 in every Release and tune object, 0
unresolved anywhere (adocs/data/S278_tls_flag_scan.py, which walks past
padding). The trace's 0 of 10795695 rows stands. Fix: constinit on the sink's
declaration and definition, which removes the wrapper; 0 sites and no wrapper
reference in any tree after. Guards: the minimized case (one trace_of() of the
start position), red on the parent's sanitizer build and green after; and
test_trace_sink_tls, an nm check over every object, red on the parent's sources
in all three trees and with constinit off the declaration alone, green with the
fix. evaluation.cpp's Release and tune objects byte-identical to the parent's,
engine and trace library both; bench 3872032; the sanitizer trace library's
object moves by register choice from constinit itself, not shipped. Fast suites
46/46 both, format clean, prose checks clean. DEV_MANUAL gains the false-report
trap; MANUAL checked, no change. No second tier owed beyond gate_extra (no
make_move, generator, search or pruning change).

### Commit text

```
Declare the trace sink constinit: sanitizer stage green (S278)

The sanitizer build's test_eval_trace and test_tuner_gradient stopped
at UBSan "store to null pointer" on trace_board()'s write of
eval_trace::sink, at an address that was not null. A plain extern
thread_local is reached from other files through GCC's TLS wrapper.
UBSan null-checks what the wrapper yields, and gcc 13.3 took that
check from the flags of `add sink@gottpoff(%rip), %reg`. ld rewrites
that add into a lea, which sets no flags, so the jump read the zero of
the wrapper's weak init-function test. constinit on the declaration and
the definition removes the wrapper and the check with it.

No plain build was affected: no Release or tune object reads flags off
such an add, and evaluation.cpp's Release and tune objects are
byte-identical to the parent's. Two guards: a minimized case in
test_eval_trace, red on the parent's sanitizer build, and
test_trace_sink_tls, which fails in any build when an object reaches
the sink through its TLS init function. tools/gate_extra.sh passes all
five stages again. Closes the open DEC-171 finding.

No functional change

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01R5qjDk7nnTM95M81VFjWNB
```
