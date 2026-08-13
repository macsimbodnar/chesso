# Audit 2026-08-13 adversarial

Scope: the whole repository at `44877c4` (branch `achesso`, working tree clean
at the start of the run) — engine source, build, tools, scripts, tests.

Method: read the code first and the documents second, then re-measured every
claim that could be re-measured. What was actually run:

- `ctest --test-dir build -L fast` (8/8 green), the same binaries from
  `build-debug` (assertions on, INV-2 and INV-4 live), and a fresh
  `-DSANITIZER=ON` RelWithDebInfo build in a scratch directory (ASan + UBSan
  clean over the whole suite and over a 309-case FEN fuzz).
- Differential harnesses against `.tuning/selfplay_v1.tsv`, compiled in a
  scratch directory against `build/src/libchesso_engine.a`: `see_ge` against
  `see` over 1415600 comparisons, `evaluate()` colour symmetry over 300000
  positions, `tools/eval_model.hpp` against `evaluate()` over 200000, and
  `|evaluate() - evaluate_cheap()|` over 100000.
- Move generation against Stockfish `go perft 1` over 3000 sampled corpus
  positions, plus `generate_captures`/`generate_quiets` partition on the same
  set, plus five self-play games move-validated against Stockfish.
- The engine driven over UCI for time control, mate reporting, `info` output,
  malformed FENs and `stop`/`quit` behaviour, and `fastchess.sh` run with a
  stubbed `fastchess` on `PATH`.

Nothing in the repository was modified. Every scratch program lives outside the
tree; the only file this run writes is this report. No new test was added: each
finding below carries a reproduction that runs from a clean checkout, so none of
them needed a test to be demonstrable. Where a fix wants a red-first test the
suggested resolution names the test and the case.

What was checked and found clean, stated because a negative result is a result:
INV-1 (3000 positions, zero disagreement with Stockfish), INV-3 (same 3000,
zero), INV-5 (300000 positions, zero asymmetry), `see_ge` against `see`
(1415600 comparisons, zero mismatches), the lazy shortcut's bound (never
exceeded, because `evaluate_expensive()` clamps it by construction), the tuner
model's clamp (it clamps exactly where the engine does), mate score reporting,
and memory safety under ASan/UBSan.

No prior audit report exists under `adocs/audit/`, so there are no earlier
findings to give a verdict on.

Written before any fix. A report edited while fixing stops being evidence of
what was found.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-08-13_adversarial-F<nn>`. The example below writes that prefix as
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

### 2026-08-13_adversarial-F01  high  `fastchess.sh` aborts before playing a game, and reports success

Status: planned

**Evidence.** `fastchess.sh:108` still prints a variable the commit under audit
deleted:

```
$ grep -n perf_cores fastchess.sh
108:echo "tc $tc  concurrency $concurrency  ($perf_cores performance cores)"
```

`44877c4` renamed it (`-perf_cores="$(sysctl -n hw.perflevel0.physicalcpu ...)"`,
`+all_cores="$(sysctl -n hw.physicalcpu 2> /dev/null || nproc)"`) and left this
reference behind. The script runs under `set -euo pipefail` (line 2), so the
expansion is fatal. Run with a stub `fastchess` on `PATH` so no match can start,
and an already-built reference so nothing else is done:

```
$ PATH=<stubdir>:$PATH REF=7b4d9a4 ./fastchess.sh --fast
WARNING: about 76% of a core is already busy. ...
candidate  44877c4
reference  7b4d9a4
./fastchess.sh: line 108: perf_cores: unbound variable
```

`fastchess` is never invoked — the stub prints nothing. The exit status is
**0**, because the `trap 'rm -f "$snapshot"' EXIT` at line 80 runs `rm -f`
successfully and bash takes the trap's status as the script's:

```
$ bash -c '... ./fastchess.sh --fast > /tmp/ffout.txt 2>&1; echo "EXIT=$?"'
EXIT=0
```

Minimal confirmation of the masking, same bash: the identical script with the
trap removed exits 1, with the trap it exits 0.

**Impact.** The project's only SPRT harness does not run at `HEAD`. Every rule
in `CLAUDE.md` about deciding a change by SPRT, and INV-6's "a change that
alters play is retained only with an SPRT verdict", currently has no working
tool behind it. The failure mode is the worst kind: launched the documented way
(`REF=<sha> nohup ./fastchess.sh --fast > .tuning/sprt_x.log 2>&1 &`) it exits
in under a second with status 0 and a log holding three lines, which a watcher
polling for `^Elo:` cannot distinguish from a match that has not produced its
first verdict yet. `DEV_MANUAL.md` documents exactly that polling loop.

**Suggested resolution.** Delete or repair the `$perf_cores` reference — the
number the line wants to report is `$concurrency`, already printed. Separately,
make the failure visible: either drop the `EXIT` trap's ability to set the
status (`trap 'status=$?; rm -f "$snapshot"; exit $status' EXIT`) or move the
snapshot cleanup somewhere that cannot rewrite it. A guard that would have
caught this is a smoke run of the script with a stubbed `fastchess` and a
pre-built reference, asserting the stub was reached; nothing in `tests/`
exercises any shell script today.

### 2026-08-13_adversarial-F02  high  `go` with a 1 ms clock arms no timer and never answers

Status: planned

**Evidence.** `compute_search_time_ms()` (`src/chesso.cpp:393-405`) returns 0
for `remaining_ms == 1` at every increment and every `movestogo`, because
`budget = std::min(budget, remaining_ms - MOVE_OVERHEAD_MS)` drives it to −49
and the floor `std::max(budget, std::min(50, remaining_ms / 2))` is
`min(50, 0) = 0`:

```
remaining 1 increment 0 movestogo 20  -> budget 0
remaining 1 increment 3 movestogo 39  -> budget 0
remaining 2 increment 0 movestogo 20  -> budget 1
```

(scratch program calling the exported `compute_search_time_ms` directly).

`command_go` then arms nothing: the fallback at `src/chesso.cpp:1047` is guarded
by `else if (!depth_given && ...)` and is unreachable because `remaining_ms > 0`
took the branch above it, and `src/chesso.cpp:1056` is `if
(search_options.search_time_ms > 0) { stop_search_after_ms(...); }`. Depth is
`MAX_DEPTH`, `nodes` is 0, `infinite` is false, so nothing bounds the search.

Reproduction, one line, `readyok` sent 8 s after the `go`:

```
$ { printf 'uci\nposition startpos\ngo wtime 1 btime 300000\n'; sleep 8; \
    printf 'isready\n'; sleep 2; } | ./build/src/chesso | grep -nE "^(bestmove|readyok)"
24:readyok
26:bestmove e2e4 ponder e7e5

$ ... same with 'go wtime 2 btime 300000' ...
10:bestmove d2d4 ponder d7d5
11:readyok
```

At `wtime 1` the `bestmove` appears only when stdin closes and the process shuts
down; at `wtime 2` it arrives before the `isready`. Held open longer, the search
runs indefinitely — 30 s, depth 19, 137396943 nodes, no `bestmove`; an explicit
`stop` answers in 0.11 s.

**Impact.** A UCI protocol hang. Any GUI or match runner that reports a clock of
exactly 1 ms — reachable at flag fall in any timed match, which is every match
this project runs — gets no reply and a chesso process pinned at 100 % of a core
until it is killed. At the new default concurrency of 8 (DEC-048) that runaway
shares the machine with the remaining games of the same SPRT, so the damage is
not confined to the game that triggered it: it distorts the timed games running
beside it, which is the one thing `fastchess.sh`'s own load warning exists to
prevent.

The existing guard misses it by construction. `tests/test_engine.cpp:400` "time
budget never exceeds the clock" asserts `budget > 0`, which is exactly the
property that fails, but its smallest case is `remaining = 51`
(`tests/test_engine.cpp:409-418`). `fastchess --compliance` (`./test_uci.sh`)
passes; its smallest clock is `wtime 100`.

**Suggested resolution.** Give `compute_search_time_ms()` a positive floor that
does not collapse at `remaining_ms <= 2 * MOVE_OVERHEAD_MS`, or make
`command_go` treat a non-positive budget the same way it treats a clock of zero
and fall back to `FALLBACK_SEARCH_TIME_MS`. Red first: add `{1, 0, 20}` and
`{1, 100, 1}` to the case list at `tests/test_engine.cpp:409` and observe
`REQUIRE_MESSAGE(budget > 0, title)` fail before the fix.

### 2026-08-13_adversarial-F03  medium  `info ... nodes` is the last iteration's count, so `search_bench.py` compares the wrong number

Status: planned

**Evidence.** `iterative_deepening_search` zeroes the counter at the top of
every iteration (`src/chesso.cpp:571`, `state.explored_nodes = 0;`), accumulates
the total into `result.total_node_explored` (`src/chesso.cpp:593`) and never
prints it: the `info` line reports `search_result.explored_nodes`
(`src/chesso.cpp:637`). The count is therefore per iteration and not monotonic:

```
info score cp 80 time 4 depth 6 nodes 49034 pv c3d5 e7d8 c2c3 f8e8 h2h3 g4f3
info score cp 80 time 2 depth 7 nodes 45397 pv c3d5 e7d8 c2c3 f8e8 h2h3 g4f3 e2f3
sum of per-iteration nodes: 122617
```

`tools/search_bench.py:41-43` keeps the last `info` line's value, so it prints
that same 45397:

```
$ tools/search_bench.py ./build/src/chesso 7
  midgame      0.010s        45397 nodes      4332 knps  best c3d5
```

45397 of 122617 nodes actually searched, and a knps figure understated 2.7-fold
because it divides the last iteration's nodes by the whole search's wall time.

**Impact.** Two things. First, a UCI deviation: `nodes` is universally read as
the count for the current search, and a GUI watching chesso sees it fall between
depths. Second, and this is the one that matters here, `DEV_MANUAL.md` names
`tools/search_bench.py` as how INV-6 is discharged for a behaviour-neutral
change ("identical node counts and identical best moves"), and the quantity
being compared is the final iteration only. A change that alters the tree at
depths 1..n−1 but leaves the last iteration identical passes that check. The
figures recorded from it are also mislabelled — `DEV_MANUAL.md`'s "47438623
nodes either way over the three search_bench positions" is a sum of last
iterations, not a node count.

**Suggested resolution.** Report the running total in `info nodes` (and, if
wanted, `nps` from the elapsed search time rather than the iteration time),
which fixes the protocol and the tool at once; or, if the per-iteration figure
is wanted for the log, print both and have `search_bench.py` read the
cumulative one. Then restate the recorded node figures as what they now mean.
Whatever is chosen, one of the two documents that quote these numbers has to
move with it.

### 2026-08-13_adversarial-F04  medium  the tuner-model guard's 2 cp tolerance is exceeded on 1.7 % of the real corpus

Status: planned

**Evidence.** `tests/test_eval_model.cpp:313` asserts
`std::abs(model - engine_white) <= 2.0`, justified by a comment saying
`evaluate()` truncates "twice, once for the tables and once for mobility". It
truncates three times: `src/evaluation.cpp:638` (piece-square plus pawn terms),
and `src/evaluation.cpp:949` and `:951` — mobility and king safety are tapered
by two separate integer divisions before being summed. Three truncations of a
`/24` bound the disagreement at 3 × 23/24 = 2.875, not 2.

The test passes only because its corpus is 26 hand-picked FENs. Over the real
tuning corpus (scratch program, same construction as the test, same
`starting_params`):

```
$ ./diff_model /Users/max/ws/chesso/.tuning/selfplay_v1.tsv 200000
DRIFT 2.33333 engine 90  model 87.6667  6k1/6p1/p7/2R5/2P3n1/P1N3P1/1rP4r/2R3K1 w - - 0 29
DRIFT 2.25    engine 441 model 443.25   r1bqkb1r/1pp1pp1p/5n2/p2p4/3P3R/2N2N2/PP1PPPP1/R1BQKB2 b Qkq - 2 8
DRIFT 2.125   engine 487 model 489.125  1k5r/pp3p2/5P2/q1pr4/4R2p/3B3P/P1P2QP1/1R4K1 w - - 6 26
positions 200000 beyond 2cp slack 3446 worst 2.875
worst fen 2r1r1k1/4Q1p1/p1P1p1q1/3p3p/1P1PpP2/4P2P/PB4P1/R4RK1 w - - 5 29
```

3446 of 200000 positions — 1.7 % — violate the bound the test asserts, and the
worst case is exactly the 2.875 the arithmetic predicts.

**Impact.** The one guard that stops `tools/eval_model.hpp` drifting from
`evaluate()` states a bound that is false, so it is not measuring what its
comment says it is. Two consequences. Extending the test corpus toward real
play makes it fail for no reason connected to a defect, which trains a reader to
widen the tolerance rather than to read it. And at 2 cp against a legitimate
2.875 cp of truncation, the tolerance cannot separate rounding from a genuine
model error of up to about 2.8 cp per position — which, spread over 1.49 M
positions, is a systematic bias the fit would happily absorb.

Not a drift in the model itself: the clamp is in both (`tools/eval_model.hpp:976`
matches `src/evaluation.cpp:982`), colour symmetry holds, and the worst
observed difference is exactly the truncation bound. What is wrong is the guard.

**Suggested resolution.** Either raise the slack to 3 and correct the comment to
name the three divisions, or remove the third truncation by tapering mobility
and king safety as one summed pair — which also puts the bound back to 2 and is
one integer division cheaper on the hot path. Whichever is chosen, the corpus
this was measured on is gitignored, so a fix should pin two or three of the FENs
above into the test rather than rely on `.tuning/`.

### 2026-08-13_adversarial-F05  low  `LAZY_EVAL_MARGIN`'s stated precondition is false, and the re-decision it promised has not happened

Status: planned

**Evidence.** `src/evaluation.hpp:274-282`:

> 150 is above the largest correction observed over 149084 positions [...] That
> figure is still the whole correction only because king safety ships at zero
> weight; the margin is re-decided from measured data once it is fitted, and
> tools/eval_spread is what measures it.

King safety has not shipped at zero weight since S027:
`src/evaluation.cpp:731-734` holds `{23, 13, 4, 21, -16, 3, 2, -33, -11}` and
`{-8, -8, -2, -34, 8, -3, -8, -10, 10}`. The margin is still 150. The tool the
comment names says what that now costs:

```
$ ./build/tools/eval_spread --data .tuning/selfplay_v1.tsv --limit 50000
  150            66 0.132%      0 0.000%    182 0.364%
  200             3 0.006%      0 0.000%     24 0.048%
  250             0 0.000%      0 0.000%      9 0.018%
worst position per term
  mobility      201  r1b2qk1/3p2p1/p2p4/1p1P1B2/5p1p/1QB4P/PP3P1K/4R1R1 b - - 0 27
  king safety   134  r1b4r/1p1k1pp1/1Qn2q1p/p2N4/3p4/5B2/P2B1PPP/4RRK1 b - - 0 21
  combined      279  r1b2q1r/ppppk3/5n1p/2P3p1/8/1P5N/PBQ3BK/3R1R2 b - - 5 21
```

The combined correction exceeds the margin on 0.364 % of positions and reaches
279, so `evaluate()` discards up to 129 cp of the term it computes on roughly
one position in 275.

**Impact.** No unsoundness: the clamp is applied inside `evaluate_expensive()`,
so `evaluate()` *is* the clamped function and the shortcut's bound holds by
construction — the 100000-position check in this run found the gap never above
150. The cost is expressiveness, and it lands where the term was added for:
king safety alone reaches 134 in a position with a king on d7 under fire, and
the clamp is what stops mobility and king safety from both being paid there. It
is also a live disagreement between a source comment and the code beside it,
which is what `specs.md`'s precedence rule exists to catch.

**Suggested resolution.** Re-run `eval_spread` over the full corpus at the
current weights, pick a margin from that distribution, and rewrite the comment
to describe the weights that ship rather than the ones that did. Raising the
margin costs search time in `evaluate_lazy` and that trade is an SPRT question,
so this is a step and not an edit.

### 2026-08-13_adversarial-F06  low  `DEV_MANUAL.md`'s tuner section is stale against `tools/tuner.cpp` in four places

Status: planned

**Evidence.** Code first, document second.

- `DEV_MANUAL.md:346` — "`tuner` fits 825 numbers". `eval_model::PARAM_COUNT` is
  **827** (printed from a scratch program including `tools/eval_model.hpp`); the
  breakdown that follows at `DEV_MANUAL.md:347-349` sums to 825 and omits
  tempo's 2.
- `DEV_MANUAL.md:368-370` — "Groups are `all` (default), `material`, `psqt`,
  `mobility`, `king_safety`, `passed_pawns`, `pawn_structure`,
  `piece_placement`". `GROUP_LIST` at `tools/tuner.cpp:140-142` also carries
  `tempo`, and `free_mask` implements it at `tools/tuner.cpp:198`. A reader
  following the manual does not know `--only tempo` exists.
- `DEV_MANUAL.md:357-359` — the paste target lists "the mobility, king safety,
  passed pawn, pawn structure and piece placement weights"; the tempo weights
  (`src/evaluation.cpp:580-581`) are not mentioned, and the manual's own warning
  is that "a fit that is half applied looks like a fit that did not work".
- `DEV_MANUAL.md:373-378` — "That has now happened three times" with three
  examples. `tools/tuner.cpp:194-197` records a fourth (`pawn_structure` past
  the piece placement weights was the third; `piece_placement` past tempo the
  fourth).

**Impact.** Low and confined to whoever reads the manual before running a fit —
which, by DEC-041, is now an agent doing it unsupervised. The specific harm is
the paste-target omission: a fit applied from the manual's list leaves the tempo
weights behind, and the symptom is a term that measures zero for a reason
nobody looks for. AGENTS.md §7 makes this a rule violation rather than a
nicety: doc claims are claims about code, and both files were "checked" at
S027's completion.

**Suggested resolution.** Re-derive the four numbers from the code in one edit:
827, the tempo group in the list, the tempo weights in the paste target, four
occurrences of the swallowing bug.

### 2026-08-13_adversarial-F07  low  `free_mask()` has broken four times and has no test

Status: planned

**Evidence.** `tools/tuner.cpp:157-209`. Four consecutive comments in the same
function record the same defect —"`king_safety` ran past the passed pawn
weights", "`passed_pawns` past the pawn structure weights", "`pawn_structure`
past the piece placement weights", "the fourth term in a row to meet the same
defect in the same function" — and the fifth is already loaded: `tempo` ends at
`PARAM_COUNT` (`tools/tuner.cpp:198-200`), so the next group appended after it
is swallowed unless someone remembers to move that end.

```
$ grep -rn "free_mask\|GROUP_LIST" tests/
(no output)
```

Nothing in `tests/` links `tools/tuner.cpp` at all. `test_eval_model` is the
only test that reaches into `tools/`, and it tests the model, not the mask.

**Impact.** The failure is silent and specifically invisible to the workflow
that surrounds it: a swallowed group returns the new term's weights exactly as
it was handed them, which reads as "the fit found nothing" — and DEV_MANUAL
already teaches that a term fitting to almost nothing is normal, because
residual fits do that. The cost of one occurrence is a wasted fit plus the SPRT
that follows it, which S027 prices at three to four and a half hours.

**Suggested resolution.** A test over `free_mask` alone, no dataset required:
every group's range is non-empty, the groups are pairwise disjoint, and their
union is exactly `[0, PARAM_COUNT)`. That last clause is what fails the moment a
group is appended without re-ending the one before it. It needs `tuner.cpp`'s
`free_mask` split into a header or the test compiled against the translation
unit; the mask logic is 50 lines and has no dependencies.

### 2026-08-13_adversarial-F08  low  the en passant square is set after every double push, splitting the hash of identical positions

Status: planned

**Evidence.** `src/bitboard.cpp:824-833` sets `new_en_passant` from the double
push unconditionally, with no test for an enemy pawn that could capture. Two
move orders reaching the same position therefore hash differently:

```
order 1  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2
         hash 11800988595472028807
order 2  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq d3 0 2
         hash 2736264005390963272
placement equal: yes
hashes equal:    no
```

(1.d4 d5 2.c4 against 1.c4 d5 2.d4, played through `algebraic_to_move` and
`make_move`.) How often the square is set with no capture available, over the
first 200000 lines of `.tuning/selfplay_v1.tsv`:

```
positions 200000 with ep square 9550 of which a capture is actually available 392
```

4.8 % of positions carry an en passant square; on 4.6 % of all positions it is
one no pawn can use.

**Impact.** Efficiency, not correctness. Move generation is unaffected — the
comparison against Stockfish's legal moves over 3000 positions found zero
disagreements — and repetition detection cannot be hurt, because a position
carrying an en passant square always has `halfmove_clock == 0` and the lookback
in `is_position_repeated` is bounded by that clock. What is lost is
transpositions: 4.6 % of quiet positions get a key that the same position
reached another way cannot match. The FENs the engine emits also differ from the
convention Stockfish and most tooling use, which is what made this visible —
comparing `generate_FEN` against Stockfish's `d` output disagrees on the fourth
field on every double push.

**Suggested resolution.** Set the square only when an enemy pawn stands ready to
capture onto it, which is one `pawn_attacks` lookup against the enemy pawn
bitboard in `make_move`. It changes the hash of a large fraction of positions,
so perft counts stay identical while the search tree does not: behaviour-neutral
by INV-6's first test and not by its second, which means an SPRT.

### 2026-08-13_adversarial-F09  info  `CMAKE_TOOLCHAIN_FILE` points at a file that does not exist and is set too late to matter

Status: planned

**Evidence.** `CMakeLists.txt:7`:

```
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake")
```

```
$ ls toolchain.cmake
ls: toolchain.cmake: No such file or directory
$ git ls-files | grep -i toolchain
TOOLCHAIN.md
```

The line sits after `project()`, and CMake reads the toolchain file during the
first `project()` call, so it has no effect. A fresh configure in a scratch
directory succeeds and picks AppleClang 16.0.0 exactly as `build/` did.

**Impact.** None today — recorded because it reads as though the build were
pinned to a toolchain when it is not, and because a future move of that line
above `project()`, or a `cmake --toolchain` invocation, turns a dead line into a
configure failure.

**Suggested resolution.** Delete the line, or add the `toolchain.cmake` it
refers to and move the setting above `project()`. `TOOLCHAIN.md` documents tool
setup and does not mention a CMake toolchain file, which suggests the first.
