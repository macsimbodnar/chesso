id:         S189
goal:       a `bench` UCI command prints one node signature over a fixed position set, every commit touching `src/` carries it, and `tools/gate.sh` runs the gate and checks the built binary against the message
accepts:    `bench` is in the UCI dispatch table: `ucinewgame`, then a fixed set of at least eight positions -- the three of `tools/search_bench.py` plus positions that reach quiescence mates, promotions, en passant and castling -- each searched at a fixed depth, the per-position best moves printed above one final line `<nodes> nodes <nps> nps`; deterministic single-threaded: two runs in one process and two processes give the same total; `MANUAL.md` documents the command before `tests/test_uci_surface.cpp`'s golden list gains it (SURFACE rule); `tools/gate.sh` runs the TESTS rule command and then compares `build/src/chesso bench`'s total with the `Bench: <n>` line of the commit under test (HEAD by default, or a message file passed to it), exits non-zero on a mismatch or on a missing line in a commit that touched `src/`, and accepts `No functional change` only when the total equals the parent commit's; observed red on a commit whose message carries the previous signature after a functional change, then green; `DEV_MANUAL.md` "Test" and "Measure" say so and record the current signature; `tools/search_bench.py` keeps its timing role unchanged
touches:    src/chesso.cpp, src/main.cpp, MANUAL.md, tests/test_uci_surface.cpp, tests/test_gate_script.sh, tests/CMakeLists.txt, tools/gate.sh, DEV_MANUAL.md, adocs/specs.md
excludes:   any change to the search; changing what `tools/search_bench.py` measures; a remote CI service (considered and refused, `adocs/testing_strategy.md` section 5)
decisions:  DEC-139, DEC-140
closes:     2026-09-04_test_review-F07
blocks:
paused_by:
author:     agent, 2026-09-08
done:       2026-09-08. `bench` is in the dispatch table and prints
            **24880255 nodes** over eight fixed positions at `BENCH_DEPTH` 14,
            identical across three fresh processes and across the stdin and
            argv forms; `tools/gate.sh` runs the TESTS chain in both builds and
            then refuses a message whose number is not the binary's, observed
            red at `GATE-FAILED: message says Bench: 24880255, the binary
            benches 13064004` and green at `GATE-DONE 13064004` after the
            amend. INV-6 identical at depths 9 and 12, interleaved timing 1.00x.
            Section 10's four questions answered by the owner. One real bug
            found in `gate.sh` by its own test and fixed; one finding recorded
            for S192.

## Why this exists

`2026-09-04_test_review-F07`. INV-6 is discharged by a person running
`tools/search_bench.py` twice and comparing by eye; nothing records the number
where a gate can read it. Every CI surveyed for `adocs/testing_strategy.md`
section 3.1 checks a bench signature from the commit message before anything
else runs -- Stockfish's `tests/signature.sh`, Berserk, Stash, the fishtest
and OpenBench workers -- and the fault-injection pass measured what it is
worth here: 21 of 33 injected bugs moved the depth-9 counts, including a
one-ply reverse-futility floor drift that two of the three mate gates did not
see. DEC-140 is the commit-message rule; this step is the command and the
check.

## Shape

`bench` sends `ucinewgame` before every position (S195's warm-table rule), so
a repeated position is searched cold. The position set is fixed in the source
and named in `MANUAL.md`; changing it changes the signature and is itself a
`Bench:` line. `tools/gate.sh` is the TESTS rule command followed by the
comparison; it prints `GATE-DONE` or `GATE-FAILED` last so a detached run can
be watched (WATCHERS rule). The R1 entry of `adocs/testing_strategy.md`
section 4 is the recommendation this step implements.

## Cost

Machine-free. About half a day: the command, the surface documents and golden,
the script, the manual sections.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

INV-6 is discharged today by a person running `tools/search_bench.py` on two
binaries and comparing three counts by eye; nothing records the number where a
script can read it (`adocs/specs.md` INV-6 row: "no test: a procedure"). This
step adds a `bench` command that searches a fixed position set to a fixed depth
and prints one deterministic node total, makes every commit touching `src/`
carry it (DEC-140, already the COMMITS rule), and writes `tools/gate.sh`, which
runs the TESTS rule command and then checks the built binary's total against
the message. No search code changes. This step's completing commit is the first
that must carry a `Bench:` line.

### 2. The technique as published

The OpenBench and fishtest bench signature. OpenBench's requirements: engines
"will be executed from the command line with `./binary bench`"; this "must
report a final node count, and a final nodes-per second count, and then exit.
A simple, acceptable format is `4712710 nodes 1323423 nps`"; "every client
will bench your engine, and they must produce the same nodes result every time"
(https://github.com/AndyGrant/OpenBench/wiki/Requirements-For-Public-Engines).
Stockfish's developer page: non-functional changes "don't change the search
behaviour", functional ones lead "to a different search tree", and the commit
hook requires a `Bench` or a `No functional change` entry
(https://github.com/official-stockfish/Stockfish/wiki/Developers). Fishtest
takes the number from the binary's `bench` ("Nodes searched : 4190940") as the
"Test signature"
(https://github.com/official-stockfish/fishtest/wiki/Creating-my-first-test).
`adocs/testing_strategy.md` 3.1 has each surveyed CI checking the line before
a game is played, and its worth here: "21 of 33 injected bugs moved the
depth-9 counts, 12 did not".

Chesso's form: the OpenBench line exactly, per-position `bestmove` lines above
it, its own position set, a depth constant in the source, a local script rather
than a remote CI (`adocs/testing_strategy.md` 5, "A remote CI service"). The
command is reached over stdin: `printf 'bench\nquit\n' | build/src/chesso` is
safe because `bench` runs synchronously and `quit` is read after it returns,
unlike `go` (DEV_MANUAL.md "Wait for `bestmove` when you script it"). The argv
form OpenBench wants needs `src/main.cpp` `main`, outside `touches:` (section 10).

### 3. What chesso has today, and where the change plugs in

- `src/chesso.cpp` `commands` is the dispatch table; `command_help` prints
  `uci_command_names()`, so `bench` reaches `help` with no further edit.
- `src/chesso.cpp` `command_test` is the template: `stop_and_join_search()`,
  `begin_search_session()`, then per position `set_position(fen)`,
  `iterative_deepening_search(search_options)` with only `depth` set,
  `bestmove` via `best_move_to_string`, a sum of `total_node_explored`
  (`src/uci.hpp` `uci_search_result_t`). Never go through `command_go`: it
  consults `search_book_move()` first, so `OwnBook` would answer without
  searching, and it starts a thread.
- `src/chesso.cpp` `command_ucinewgame` is the per-position reset the Shape
  requires: `stop_and_join_search()`, `set_position(DEFAULT_POSITION)`,
  `tt_reset(&tt)`, `proven_mate_line.length = 0`, `still_in_opening = true`.
  `set_position` alone resets the table only when the FEN differs from
  `initial_position` (F08, S195) and never clears `proven_mate_line` -- S170's
  mate line carried across searches, cleared nowhere else -- so a bench that
  skipped this would carry the line proven on `MATE_IN_2_W_POS` into the next
  position. Factor the body into a helper both commands call; then
  `set_position(fen)`.
- The position set, a `static const std::array` beside `command_test`, in
  order: the three `tools/search_bench.py` `POSITIONS` (midgame, kiwipete,
  tactical), then `KILLER_POS`, `CMK_POS`, `FINE_70_POS`, `MATE_IN_2_W_POS`,
  `MATE_IN_2_B_POS` from `src/data_structures.hpp`: eight distinct FENs
  (kiwipete is `TRICKY_POS` verbatim). Verified 2026-09-05 with python-chess
  1.11.2 in `~/.venv/chess` (`legal_moves`, `is_en_passant`, `is_castling`,
  `promotion`): castling legal in kiwipete (`e1g1`, `e1c1`) and tactical
  (`e1g1`); promotions in tactical (4) and `KILLER_POS` (12); en passant `f5e6`
  legal in `KILLER_POS`. Stockfish `dev-20260803-762dd1da` at `go depth 20`
  via `chess.engine` (the safe invocation, TOOLCHAIN.md "The chess oracle")
  scores `MATE_IN_2_W_POS` `#+2` and `MATE_IN_2_B_POS` `#-2`. Re-run that
  script on the workstation and paste its output into the comment above the
  array. One fact to carry: python-chess says `KILLER_POS` `is_valid() ==
  False`, `TOO_MANY_WHITE_PIECES|TOO_MANY_WHITE_PAWNS` (nine white pawns);
  the engine loads it and `test` already searches it (section 10).
- Output: per position the `info` lines `iterative_deepening_search` already
  prints, then `bestmove <move>`; last, `<total> nodes <nps> nps` with `nps` =
  total nodes over summed search wall time floored at one microsecond, as the
  `info` line does. All via `uci_reply` (stdout); `LOG_I` is `std::clog`, off
  in Release, so the gate parses stdout only. `bench [depth]` takes an optional
  depth through `pop_int` (clamped 1..`MAX_DEPTH`) like `test <n>`; the
  signature is the bare form.
- Order of edits: helper and `command_bench`; `MANUAL.md` "Non-standard
  commands" row and output format; `"bench"` into `expected_commands` in
  `tests/test_uci_surface.cpp`, where "MANUAL.md documents every command and
  every option" stays red until the row exists -- observe that first (SURFACE
  rule); the new case (section 6); `tools/gate.sh`; `DEV_MANUAL.md`;
  `adocs/specs.md` INV-6.

### 4. Constants and seeds

- `BENCH_DEPTH`, a `constexpr int` in `src/chesso.cpp`, deliberately not a
  `CHESSO_SEARCH_PARAMS` row: a settable depth makes the signature session
  state. Form (b): on the idle workstation on mains (`ps aux | sort -rnk3 |
  head`) run `hyperfine -w 1 -r 5 "printf 'bench D\nquit\n' | ./build/src/chesso"`
  for D = 9..14 and take the largest D whose mean is at most 5 s; floor 9, the
  depth the fault-injection pass measured. Table into the stamp. Scale: on the
  2026-09-05 MacBook `test 9` (seven positions) is 1768972 nodes in 0.17 s and
  depth 12 on the three `search_bench.py` positions is 4437796 nodes; expect
  12 to 14.
- The set size and the regex `^[0-9]+ nodes [0-9]+ nps$` are conventions, not
  tuned values. Changing set or depth changes the signature and is itself a
  `Bench:` commit (DEC-140).

### 5. Interactions and traps

- Single-threaded by construction: `command_uci` advertises `option name
  Threads type spin default 1 min 1 max 1`; `command_setoption` ignores other
  values. The table size is not fixed by `bench`: `setoption name Hash` before
  it changes the count. Define the signature as the fresh-process number at
  `TT_DEFAULT_MB`, which the gate's pipe produces; say so in `MANUAL.md`.
- No time: `depth` only, `search_time_ms`, `nodes`, `infinite` zero, never
  `stop_search_after_ms`. `begin_search_session` after `stop_and_join_search`
  disarms a timer a previous `go movetime` left (S163); keep both, that order.
- Cross-machine: Zobrist keys come through `std::uniform_int_distribution
  <uint64_t>`, implementation-defined (F08). glibc and Apple libc++ agreed on
  2026-08-27 (`.moltke.local.md`, 121512 / 800769 / 62907 at depth 9); until
  S179 regenerates the keys a signature is per standard library, and a
  disagreement between the machines is investigated before it is called a
  behaviour change.
- Trailer: commits end with `Co-Authored-By:`, so DEC-140's "ends with" is read
  as "carries in its trailer block": grep the whole message for `^Bench:
  [0-9]+$` and `^No functional change$`, require exactly one when `src/` was
  touched; convention puts the line last before `Co-Authored-By`.
- Parent's total for `No functional change`: from this commit on every `src/`
  commit carries a verified line, so the nearest ancestor `Bench:` is the
  parent's total by induction (Stockfish CI "takes the reference from the last
  100 commits", `adocs/testing_strategy.md` 3.1). Default: walk `git log
  --format=%B -n 100 <ref>^` for the first match, none is red;
  `--build-parent` builds `<ref>^` in a worktree under `.ref-builds/` as
  `fastchess.sh` does and runs its bench. This step's commit has no ancestor
  line: it is a `Bench:` line, never an NFC.
- bash 3.2 (S167, S177, `.moltke.local.md`): `#!/bin/bash`, `set -euo
  pipefail`; no `mapfile`, `readarray`, `declare -A`, `${x,,}`, `&>>`, `nproc`,
  `timeout`; the EXIT trap reads `marked`/`completed` flags, not `$?`, as
  `fastchess.sh`'s trap and `fail()` do, armed before anything can fail. Test
  under `/bin/bash`. `ctest` piped into `tail` hides its status (DEV_MANUAL.md
  "Test"): run the TESTS chain as one `&&` command, read its status directly.
- Tree under test: in `--message FILE` mode refuse when `git diff --quiet`
  fails (unstaged edits), so the bench run is the one the commit produces.

### 6. Tests

- `tests/test_uci_surface.cpp`: `"bench"` joins `expected_commands`; "the
  command set is exactly the documented one" and "help prints the dispatch
  table and nothing else" then hold it against `uci_command_names()`.
- New case there, `stdout_capture_t` (`tests/test_helpers.hpp`), synchronous
  so no `uci_wait_for_search`:

  ```
  TEST_CASE("bench prints one final line and repeats its total")
    uci_init(); capture; uci_process_line("bench"); lines = capture.lines()
    REQUIRE exactly one line matches ^[0-9]+ nodes [0-9]+ nps$, and it is last
    REQUIRE lines starting "bestmove " >= 8
    REQUIRE total == sum of the `nodes` field of the last `info` line before
            each `bestmove`                        // the sum is the signature
    second capture; uci_process_line("bench"); REQUIRE same total, same
            bestmove lines                         // two runs, one process
    uci_shutdown()
  ```
  Two processes agreeing is checked by hand at completion (the pipe twice) and
  quoted in the stamp.
- `tests/test_gate_script.sh`, registered in `tests/CMakeLists.txt` like
  `test_fastchess_script` (both need `touches:`, section 10), sandboxed as
  `tests/test_rating_script.sh`: a throwaway git repository, a stub
  `build/src/chesso` printing canned `bestmove` lines and `12345 nodes 999 nps`,
  stub `cmake`, `ctest`, `./clang-format.sh` exiting 0, PATH holding only
  those. Cases: `Bench: 12345` with `src/` touched is `GATE-DONE`, exit 0;
  `Bench: 12344` is `GATE-FAILED` naming both numbers; no line with `src/`
  touched fails; no line, docs only, passes; `No functional change` over parent
  `Bench: 12345` passes, over `Bench: 11111` fails; both lines fails; a stub
  with no final line gives exactly one `GATE-FAILED`; `bash -n` passes; no
  `mapfile`, bare `nproc` or `timeout` in the script.
- Not owed: guard test, mutant, Debug self-play (no search, `make_move` or
  generator change). No golden added: the signature lives in commit messages
  and `DEV_MANUAL.md`, quoted with its sha.
- INV-6: `python3 tools/search_bench.py ./build/src/chesso 9` and `... 12`
  before and after read 121512 / 800769 / 62907 and 639228 / 3430710 / 367858,
  best moves `c3d5` / `e2a6` / `d7c8q` (`.moltke.local.md`; valid on both
  machines since S167's handover check).

### 7. Measurement

Behaviour-neutral lane: no SPRT, no bounds pair, no A/A. Proof is the INV-6
identity plus one interleaved timing, a formality since no search line moves:
`hyperfine -w 2 -r 10 "python3 tools/search_bench.py ./build/src/chesso 12"`
against the same on a `HEAD` worktree build, within noise (under 3 %,
CLAUDE.md). Both into the stamp with shas.

`tools/gate.sh`: anchor to the repository root from `$0`; arm `fail()` and the
trap first; parse `--message FILE`, `--build-parent`, else a ref defaulting to
`HEAD`; run the TESTS rule command verbatim; bench through the pipe, first
field of the last line matching the regex; `touched_src` from `git diff-tree
--no-commit-id --name-only -r <ref> -- src/` (message mode: `git diff --cached
--name-only -- src/`); message from `git log -1 --format=%B <ref>` or the file;
compare per section 5; `GATE-DONE <total>` or `GATE-FAILED: <reason>` last.

```bash
#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
marked=0; completed=0
fail() { marked=1; echo "GATE-FAILED: $*" >&2; exit 1; }
trap 'status=$?
      if ((marked == 0 && (status != 0 || completed == 0))); then
        echo "GATE-FAILED: exited $status" >&2; ((status != 0)) || status=1
      fi; exit $status' EXIT
cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure \
  && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure \
  && ./clang-format.sh --check || fail "TESTS rule command failed"
line_re='^[0-9]+ nodes [0-9]+ nps$'
total="$(printf 'bench\nquit\n' | build/src/chesso | grep -E "$line_re" | tail -1 | cut -d' ' -f1)"
[[ -n "$total" ]] || fail "bench printed no '<nodes> nodes <nps> nps' line"
# ... touched_src, message, the two grep -E lines, the parent walk ...
completed=1; echo "GATE-DONE $total"
```

Red then green (accepts): in a scratch worktree apply one mutant the review
measured as moving the counts -- `M01_nmp_in_check` in
`adocs/data/2026-09-04_test_review/mutants.py`, `nodes_changed yes` in
`results.tsv` -- commit it with the previous `Bench:` line, run `tools/gate.sh`,
see `GATE-FAILED` with both numbers; amend to the new total, see `GATE-DONE`;
remove worktree and branch. Quote both lines in the stamp.

### 8. Completion checklist

- Gate: `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`,
  then `tools/gate.sh --message <msgfile>` before committing and `tools/gate.sh`
  on `HEAD` after; both `GATE-DONE`.
- The completing commit's body ends with `Bench: <total>`, the total the new
  command prints on that tree -- the first commit under DEC-140.
- `MANUAL.md` "Non-standard commands": the `bench [depth]` row, output format,
  Hash and single-thread caveats. `DEV_MANUAL.md` "Test": `tools/gate.sh`,
  modes, markers; "Measure": the signature with its sha and the depth table;
  `search_bench.py` keeps the timing role. `adocs/specs.md` INV-6 row: "no
  test: a procedure" becomes the gate for the neutral half; the paragraph
  names `bench` and `tools/gate.sh`. `README.md` untouched.
- Stamp: the eight FENs' verified properties with the script output; depth
  table and `BENCH_DEPTH`; two-process agreement; INV-6 counts; the
  red-then-green lines; what the three documents changed. `plan.md` and
  `status.md` through the coordinator.

### 9. Sources read

- https://github.com/AndyGrant/OpenBench/wiki/Requirements-For-Public-Engines -- `./binary bench`, the `nodes ... nps` line, determinism across clients (fetched).
- https://github.com/AndyGrant/OpenBench -- README; points to the wiki only (fetched).
- https://github.com/official-stockfish/Stockfish/wiki/Developers -- functional vs non-functional, the `Bench` / `No functional change` hook (fetched; documentation, no code read).
- https://github.com/official-stockfish/fishtest/wiki/Creating-my-first-test -- the signature from `bench`, the "Test signature" field (fetched).
- Repository: `adocs/testing_strategy.md` 0, 3.1, R1, R8, 5; `adocs/audit/2026-09-04_test_review.md` F07, F08; DEC-023, DEC-118, DEC-139 to DEC-142, DEC-145; `adocs/plan_todo/S195_reproducibility_test.md`; `adocs/plan_done/S167_fastchess_bash32_portability.md`, `S177_rating_release_scripts_portable.md`; `src/chesso.cpp`, `src/uci.hpp`, `src/main.cpp`, `src/data_structures.hpp`, `src/transposition_table.cpp`; `tests/test_uci_surface.cpp`, `tests/test_helpers.hpp`, `tests/test_search.cpp`, `tests/test_fastchess_script.sh`, `tests/test_rating_script.sh`, `tests/CMakeLists.txt`; `tools/search_bench.py`; `fastchess.sh`; `MANUAL.md`, `DEV_MANUAL.md`, `TOOLCHAIN.md`, `.moltke.local.md`; `adocs/data/2026-09-04_test_review/results.tsv`. python-chess and Stockfish runs of 2026-09-05 as in section 3. No figure unverified.

### 10. Questions deferred to the owner

1. `touches:` omits `tests/CMakeLists.txt` and the new `tests/test_gate_script.sh` the parser test needs: extend it, or drop that test from the accepts.
2. OpenBench's `./binary bench` argv form needs `src/main.cpp`: add it to `touches:` now, or keep the stdin form and plan the argv form later.
3. `KILLER_POS` fails python-chess `is_valid()` (nine white pawns): keep it (the engine loads it, the properties are verified, OpenBench asks only determinism), or derive a valid variant by removing one uninvolved white pawn and re-verifying `f5e6` and the promotions with the same script.
4. `-j8` in the gate is the rule's verbatim text; a `JOBS` override defaulting to 8 for the workstation deviates from the TESTS wording -- allow it or not.

## What was done, 2026-09-08

### The owner's four answers (section 10)

1. **`touches:` extended**, `tests/CMakeLists.txt` and `tests/test_gate_script.sh`
   added; the gate script keeps its test.
2. **The argv form was added now**, so `src/main.cpp` joins `touches:`.
   `./chesso bench` and `printf 'bench\nquit\n' | chesso` print the same number.
3. **`KILLER_POS` is kept as it is.** It fails python-chess `is_valid()` on
   nine white pawns and it is the only position in the set carrying the en
   passant capture and twelve promotions; a signature needs determinism, not
   legality. Said so in `MANUAL.md` and in the comment above the array.
4. `-j8` kept verbatim, the TESTS rule's own text; no `JOBS` override.

### The position set, verified here

`~/.venv/chess` python-chess **1.11.2**, and stockfish
**dev-20260810-5062aee5** at `go depth 20` through `chess.engine` (the safe
invocation, TOOLCHAIN.md). Eight distinct FENs; every property the MacBook
recorded on 2026-09-05 reproduces:

| position | legal | en passant | castling | promotions | valid |
|---|---|---|---|---|---|
| midgame | 46 | — | — | 0 | yes |
| kiwipete | 48 | — | `e1g1`, `e1c1` | 0 | yes |
| tactical | 44 | — | `e1g1` | 4 | yes |
| KILLER_POS | 42 | `f5e6` | — | 12 | **no** |
| CMK_POS | 43 | — | — | 0 | yes |
| FINE_70_POS | 3 | — | — | 0 | yes |
| MATE_IN_2_W_POS | 29 | — | — | 0 | yes, `#+2` |
| MATE_IN_2_B_POS | 29 | — | — | 0 | yes, `#-2` |

The accepts asked for quiescence mates, promotions, en passant and castling;
all four are present. `KILLER_POS` reports
`TOO_MANY_WHITE_PAWNS|TOO_MANY_WHITE_PIECES`.

### `BENCH_DEPTH`, form (b)

Idle workstation on mains, governor `performance`, `hyperfine -w 1 -r 5`:

| depth | mean | nodes |
|---|---|---|
| 9 | 0.208 s ± 0.003 | 1587743 |
| 10 | 0.344 s ± 0.007 | 2580811 |
| 11 | 0.583 s ± 0.007 | 4437125 |
| 12 | 0.997 s ± 0.008 | 7408328 |
| 13 | 1.754 s ± 0.006 | 13064004 |
| **14** | **3.445 s ± 0.028** | **24880255** |

14 is the largest at most five seconds, the rule the step set itself, and
inside the guide's predicted 12–14 band.

### The signature: 24880255

Three fresh processes: `24880255` each time, at 7.29, 7.32 and 7.37 M nps — the
rate moves, the count does not. `./build/src/chesso bench` gives the same
number as the stdin form. Best moves `c3d5`, `e2a6`, `d7c8q`, `g7h8q`, `a7a6`,
`a1b2`, `e5e6`, `e5e6`; the first three are `search_bench.py`'s own, unchanged.

### INV-6, and the timing

`tools/search_bench.py` at depth 9 gives **121512 / 800769 / 62907** and at
depth 12 **639228 / 3430710 / 367858**, best moves `c3d5` / `e2a6` / `d7c8q` at
both — identical to `.moltke.local.md`'s recorded values, so no search line
moved. `hyperfine -w 2 -r 10` over `search_bench.py` at depth 12, this tree
against a `6688a01` worktree build: **662.2 ms ± 6.0 against 659.1 ms ± 6.4**,
a ratio of 1.00 ± 0.01. Under 3 %, and no search code was touched.

### Red then green

Not with a search mutant, and that is the finding. **Every node-moving change
tried was refused by the fast suite before the signature was ever compared**,
so the signature branch cannot be demonstrated with one:

- `M01_nmp_in_check` from `adocs/data/2026-09-04_test_review/mutants.py`, the
  mutant the guide named → `test_mate_carry` red.
- `ORDER_KILLER_0`/`ORDER_KILLER_1` swapped → `test_evaluation` red, which
  asserts the band ordering as an invariant. Correct behaviour.
- `MVV_KNIGHT` 300 → 310, a change no band assertion covers →
  `test_mate_carry` red again.

`test_mate_carry` is the common cause. Its anti-vacuity precondition —
`mate_lines >= expected_mate_lines(name)`, "at least 5 when the case was
chosen … needs re-choosing, not deleting" — is a search-tree-sensitive golden,
so any change that reorders the tree trips it. That is the test doing its job,
and it is also a golden in the DEC-142 sense with no script that re-derives it.
**Recorded for S192**, which owns naming and scripting the goldens.

The demonstration used the case `MANUAL.md` already names as a deliberate
signature change, `BENCH_DEPTH` 14 → 13, in a scratch worktree off this tree:

```
GATE-DONE 24880255                                            # baseline
GATE-FAILED: message says Bench: 24880255, the binary benches 13064004
GATE-DONE 13064004                                            # after the amend
GATE-DONE 13064004 (no functional change)   # a comment-only src/ commit after
```

The `No functional change` path read the parent's total off the ancestry, not
by building it.

### One bug in `tools/gate.sh`, found by its own test and fixed

Case 8 of `tests/test_gate_script.sh` — a binary that prints no signature line —
printed the trap's generic `GATE-FAILED: exited 1` instead of the specific
message written for it. `grep` exits 1 when nothing matches, `set -o pipefail`
fails the whole pipeline, and errexit killed the script one line above its own
check. `bench_of` is now called `|| true` at both call sites, and the comment
at the site says why. The test was written first and observed red.

### Tests

- `tests/test_uci_surface.cpp`, new case *"bench prints one final signature
  line and repeats its total"*: exactly one line matching
  `^[0-9]+ nodes [0-9]+ nps$` and it is last, at least eight `bestmove` lines,
  the total equal to the sum of each search's last `info nodes` field, and a
  second run in the same process giving the same total and the same moves.
  **Falsifiability observed, two mutants killed**: `total_nodes +=
  res.total_node_explored + 1` and a second signature-shaped line printed above
  the real one.
- `tests/test_gate_script.sh`, nine cases in a throwaway git repository whose
  PATH holds stubs for `cmake`, `ctest` and `clang-format.sh` and a
  `build/src/chesso` printing a canned line. git is real, because the
  touched-paths and message reading is the half a stub would hide. Registered
  `fast`, 0.24 s.
- **SURFACE order observed in both directions**: `bench` in the dispatch table
  and not in the golden gave *"Present but not in the golden list: [bench]"*;
  `bench` in the golden and not in `MANUAL.md` gave *"MANUAL.md does not
  document the command [bench]"*. Then the manual row, then green.
- Not owed and not done: guard test, mutant list entry, Debug self-play — no
  search, `make_move` or generator change.

### Cost, measured

The fast suite goes from 52 s over 27 tests to **61 s over 28**. All of the
increase is `test_uci_surface`, 0.03 s → 6.8 s, because its case searches the
bench set twice at depth 14. **In `Debug` that case is 157 s**, measured, which
lands on S197's Debug gate rather than on the per-commit one. Kept at the
shipping depth on purpose: a case that benched shallower would not be testing
the number that ships.

### Documents

- `MANUAL.md`: the `bench` row, and a new *"The node signature, `bench`"*
  section — both invocations, the output format, and the three caveats that
  keep the number comparable (`Hash` is not fixed by the command, the search is
  single-threaded, the Zobrist keys are per standard library until S179).
- `DEV_MANUAL.md` *"Test"*: a new *"The gate, `tools/gate.sh`"* subsection —
  the four invocations, the markers, what it enforces, and that
  `search_bench.py` keeps the per-position role. The suite timing and test
  count updated, and the `Debug` paragraph given the 157 s.
- `DEV_MANUAL.md` *"Measure"*: a new *"The node signature"* subsection with
  **24880255 at S189**, quoted with its step the way every other number on that
  page is, and the depth sweep.
- `adocs/specs.md`: the INV-6 bullet gains the paragraph saying the neutral half
  is enforced rather than performed, and the invariant table's INV-6 row stops
  saying *"no test: a procedure"* and names the gate and the two tests.
- `README.md` untouched, as the DOCS rule requires.

### One thing carried forward

`git worktree` does not bring the submodules (`tests/doctest`, `tests/json`,
`tests/pixello`), so a worktree cannot build the test targets without them —
`fastchess.sh` never met this because it builds only the `chesso` target, and
`gate.sh --build-parent` builds only `chesso` for the same reason. It cost a
gate run to find during the demonstration. Not a defect in anything this step
owns; written down so the next agent that gates inside a worktree knows.
