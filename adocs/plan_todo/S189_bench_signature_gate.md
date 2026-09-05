id:         S189
goal:       a `bench` UCI command prints one node signature over a fixed position set, every commit touching `src/` carries it, and `tools/gate.sh` runs the gate and checks the built binary against the message
accepts:    `bench` is in the UCI dispatch table: `ucinewgame`, then a fixed set of at least eight positions -- the three of `tools/search_bench.py` plus positions that reach quiescence mates, promotions, en passant and castling -- each searched at a fixed depth, the per-position best moves printed above one final line `<nodes> nodes <nps> nps`; deterministic single-threaded: two runs in one process and two processes give the same total; `MANUAL.md` documents the command before `tests/test_uci_surface.cpp`'s golden list gains it (SURFACE rule); `tools/gate.sh` runs the TESTS rule command and then compares `build/src/chesso bench`'s total with the `Bench: <n>` line of the commit under test (HEAD by default, or a message file passed to it), exits non-zero on a mismatch or on a missing line in a commit that touched `src/`, and accepts `No functional change` only when the total equals the parent commit's; observed red on a commit whose message carries the previous signature after a functional change, then green; `DEV_MANUAL.md` "Test" and "Measure" say so and record the current signature; `tools/search_bench.py` keeps its timing role unchanged
touches:    src/chesso.cpp, MANUAL.md, tests/test_uci_surface.cpp, tools/gate.sh, DEV_MANUAL.md, adocs/specs.md
excludes:   any change to the search; changing what `tools/search_bench.py` measures; a remote CI service (considered and refused, `adocs/testing_strategy.md` section 5)
decisions:  DEC-139, DEC-140
closes:     2026-09-04_test_review-F07
blocks:
paused_by:
author:
done:

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
