# Audit 2026-08-14 test_review

Scope: the test suite at `15b1188` (branch `achesso`, working tree clean at the
start of the run) — everything under `tests/`, the perft and move-suite assets,
the two shell-script tests, `.moltke.json`'s gate, and `tools/search_bench.py`
as the instrument INV-6 rests on. Engine source read only where a test's claim
had to be traced to it.

Two questions were asked, both from the owner: are the tests correct, and are
they in line with the published literature and what other open-source engines
test.

**This is not a clean-context adversarial run under AGENTS.md §10.** It ran in
the session that had already read `status.md` and the plan, at the owner's
request, so it is a review with the blue team's knowledge and should be read as
one. A red-team pass over the same files would be a separate spawn.

Method: every oracle value in the suite that has an independent source was
re-derived from that source, rather than re-read. What was actually run:

- `ctest --test-dir build -L fast` — 12 of 12, 18.06 s.
- `build/tests/test_perft` — exit 0, 21 positions, 56 layers, 8436660159 leaf
  nodes, zero red cells.
- Every node count in `tests/assets/perft_json/perft.json` and
  `talkchess_perft.json` re-derived with `stockfish dev-20260810-5062aee5`
  `go perft`, depths 1 to 8: 56 of 56 identical.
- Those files' `captures` / `en_passant` / `castles` / `promotions` columns
  diffed against the Chess Programming Wiki tables for the start position,
  Kiwipete and Position 3: every published cell matches, including the start
  position at depth 9.
- `perftsuite.epd` — the 127-position suite Ethereal and the other OpenBench
  engines run, which is **not** in this repository — driven through
  `build/tests/debug_perft_app` at the deepest depth under 20 M nodes per
  position: 127 of 127 pass.
- The 9 positions of `"the winning move is found"` re-checked with `stockfish`
  `go depth 20`, MultiPV 2, and their legal-move counts taken from
  `python-chess`.
- Every FEN written inline in `tests/*.cpp` — 135 distinct — put through
  `python-chess` `Board.status()`.
- `cmake --build build-debug && ctest --test-dir build-debug -L fast`, and then
  the two binaries that failed there run directly.

Nothing in the repository was modified by the run itself. Scratch programs and
downloads live outside the tree.

What was checked and found clean, stated because a negative result is a result:
the perft oracle data (56 counts, every published breakdown column), move
generation against a suite six times larger than the one in the repo (127 of
127), the tactical assertions except F01 (8 of 9 have the asserted move as
Stockfish's unique best), the deliberate illegal positions elsewhere in the
suite (8 of the 10 illegal FENs are intentional and commented), and the
invariant asserts themselves — `build-debug/tests/test_movegen` is 14 of 14 over
90569574 assertions in 165.33 s and `test_search` is 44 of 44 over 455237
assertions in 215.91 s, both SUCCESS, so INV-2 and INV-4 hold today.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-08-14_test_review-F<nn>`.

Every finding ends in one of two places: a plan step whose `closes:` field
names it, or a decision entry stating why it will not be acted on, which moves
it to `accepted`. A finding moves to `closed` only after the audit is re-run
and no longer reports it. Fixing without re-running leaves it `planned`.

## Findings

### 2026-08-14_test_review-F01  high  a tactical case has exactly one legal move

Status: open

Evidence: `tests/test_search.cpp:348`, inside `"the winning move is found"`,
which asserts a specific `from`, `to` and promotion at depths 4, 5 and 6.

```
{"2k5/8/8/8/8/8/1q6/K1R5 w - - 0 1",       a1, b2, TO_NONE,  "the king takes the loose queen"},
```

`python-chess`: `legal_moves` = 1, the single move being `a1b2`.
`stockfish` refuses the position outright —
`info string CRITICAL ERROR: Command 'position fen 2k5/8/8/8/8/8/1q6/K1R5 w - - 0 1'
failed. Reason: Unsupported position. King can be captured.` White is in check
from `Qb2` while `Rc1` checks the black king on `c8`; `Board.status()` is
`OPPOSITE_CHECK`.

Impact: the case cannot fail for the reason it exists. Any engine that returns
a legal move passes it at all three depths. It duplicates the coverage of
`"search results are well formed everywhere"` and adds nothing to the suite it
sits in. The sibling case one line above,
`"the king takes the loose rook"` (`:347`), is real: 3 legal moves, Stockfish
`cp 621` for `e1e2` against `cp -3` for the runner-up.

Suggested resolution: replace the position with a legal one in which the
capture is the unique best among several legal moves, and add a precondition to
the loop asserting that every case has more than one legal move, so the class
of defect cannot return. Not applied here.

### 2026-08-14_test_review-F02  high  the mate-in-zero position is mated by a king

Status: open

Evidence: `tests/test_search.cpp:144` (`"a mated side reports mate in zero"`)
and `tests/test_search.cpp:610` (`"mate is recognised at depth zero"`), both
using `7k/5Q1K/8/8/8/8/8/8 b - - 0 1`.

`python-chess` over that FEN: `status()` is `OPPOSITE_CHECK`, `legal_moves` is
0, and the attackers of the side-to-move king are `['h7:K']` — **the only
attacker is the white king**. Two kings on adjacent squares cannot occur in a
legal game.

The case one below it, `"stalemate scores zero, not mate"`
(`7k/5Q2/6K1/8/8/8/8/8 b - - 0 1`), is `status()` = `VALID`. The two are a
matched pair separating mate from stalemate, and the mate half is the one that
is wrong.

Impact: both assertions hold today, but what they prove is that a side with no
legal moves whose king is adjacent to the enemy king scores as mate in zero.
They do not prove that a *legal* mate does. A regression in check detection that
only affected real checking pieces would leave both green.

Suggested resolution: replace with a legal mate, verified by a tool rather than
by reading, and assert the legality as a precondition of the case. Not applied
here.

### 2026-08-14_test_review-F03  medium  the invariant asserts have no gate, and no working ctest invocation

Status: open

Evidence: `.moltke.json`'s `test_command` is
`cmake --build build -j12 && ctest --test-dir build -L fast --output-on-failure && ./clang-format.sh --check`.
`build/CMakeCache.txt` has `CMAKE_BUILD_TYPE:STRING=Release` and
`CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG`, so every `assert()` is compiled
out of the binaries the gate runs.

`adocs/testing.md` records INV-4's covering test as
`assert(eval_accumulators_match(...))` at three sites in `src/bitboard.cpp`,
**debug build only**, and INV-2's second half as the debug `squares[]` assert.

The obvious way to run them fails: `ctest --test-dir build-debug -L fast` is
10 of 12, with `test_movegen` and `test_search` both reported as **Timeout**.
`tests/CMakeLists.txt:22` sets `TIMEOUT 60` on every doctest target; run
directly, outside ctest, the two take **165.33 s** and **215.91 s** and both
pass (14 of 14 / 44 of 44, SUCCESS).

Impact: two of the six invariants are enforced only by a human remembering to
run two binaries by hand, and the natural command for doing so reports a
failure that is a timeout rather than an assertion. `DEV_MANUAL.md:75-76`
documents the gate as "necessary and not sufficient" and `:82` documents the
direct-binary route, so this is a known shape — but a `TIMEOUT` that the build
it is written for cannot meet is a defect in the test registration, not a
policy.

Suggested resolution: make the timeout depend on the build type so the debug
suite can complete under ctest. Whether the debug run joins an automatic gate is
a separate decision. Not applied here.

### 2026-08-14_test_review-F04  medium  a test helper states the opposite of INV-1

Status: open

Evidence: `tests/test_helpers.hpp:75-76`:

> `generate_moves()` is pseudo-legal. This filters it down to the moves that
> actually survive `make_move()`.

`adocs/specs.md` INV-1 says generation is legal-only. `make_move_impl` has
exactly one `return false`, `src/bitboard.cpp:754`, the history-stack overflow
guard, so `legal_moves()` (`tests/test_helpers.hpp:77`) filters nothing.
`test_movegen`'s `check_split` asserts `REQUIRE(make_move(g, all[i]))` over a
three-ply tree from every test FEN and passes, which is the property stated
positively.

`legal_moves()` is called from 20 sites across six test files.
`tests/debug_perft_app.cpp:139` has the same redundancy through
`is_move_legal()`.

Impact: nothing computes a wrong answer. A reader is told the opposite of an
invariant at 20 call sites, and a regression to pseudo-legal generation would
be silently absorbed by the helper instead of failing.

Suggested resolution: state what is true, and make the helper assert legality
rather than filter for it, so the 20 call sites become INV-1 guards. Not applied
here.

### 2026-08-14_test_review-F05  low  the insufficient-material test skips every boundary case

Status: open

Evidence: `tests/test_search.cpp:901`, `"which material can still mate"`, 8
cases: KvK, KNvK, KNvKN, KNNvK, KBvK, KRvK, KQvK, KPvK.

`is_insufficient_material()` (`src/bitboard.cpp:1386`) reduces to
`count_bits(minors) <= 1` once pawns, rooks and queens are excluded. The test
has **no bishop-against-bishop case at all**, and no bishop-against-knight
case.

Impact: KBvKB with both bishops on the same colour is the case implementations
differ on and the one a rule-following draw adjudicator calls dead; chesso
answers `false` for it, as it does for KBvKN. Neither answer is pinned, so
either can change without a test noticing, and `src/search.cpp:264` returns
`DRAW_SCORE` off this function at every node above the root.

Suggested resolution: add cases pinning today's answer in both directions, with
the disagreement with the rule-following reading recorded in the case rather
than silently resolved. Changing the answer is a strength question and an SPRT,
not this. Not applied here.

### 2026-08-14_test_review-F06  low  the move-ordering guard has an order of magnitude of headroom

Status: open

Evidence: `tests/test_search.cpp:450`, `"ordering keeps the tree small"`, sets
`state.node_limit = 1000000` and asserts `REQUIRE_FALSE(state.aborted)` after
`search(5, ...)` on `TRICKY_POS`. The engine's whole iterative deepening to
depth 5 on that position costs 108459 nodes
(`position tricky` / `go depth 5`, last `info` line).

Impact: move ordering would have to get roughly ten times worse before the case
fires. Its own comment says so — "the budget leaves a wide margin" — so this is
a known weak guard rather than a hidden one.

Suggested resolution: measure `search(5, ...)`'s own count and set the budget
from it with a stated margin. Not applied here.

### 2026-08-14_test_review-F07  low  the built-in `test` command prints expectations it never checks

Status: accepted — DEC-058

Evidence: `src/chesso.cpp:1179-1198`. The seven entries carry their expected
answers inside the title string —
`{TRICKY_POS, "TRICKY_POS         bestmove e2a6 ponder b4c3"}` — and
`total_nodes` is printed as `"Total explored nodes: "`. Nothing compares
either, and the command is not registered with `ctest`.

The expectations themselves are right: `MATE_IN_2_W_POS` and `MATE_IN_2_B_POS`
both give `e5e6` as Stockfish's `mate 2` against `mate 3` for the runner-up.

Impact: what looks like the engine's own regression suite is a printout. The
node total it prints is the one quantity that would make it a bench signature
and nothing records or compares it.

Suggested resolution: out of scope for a test repair — the command is UCI
surface covered by `test_uci_surface` and `MANUAL.md`, and turning it into a
checked bench is its own step with its own surface change. Recorded so it is not
found again as if it were new.

## Against the literature and other engines

Not findings. The second half of the question the owner asked, recorded here so
the comparison is not re-derived later.

Sources read: Stockfish's `tests/` directory (`perft.sh`, `signature.sh`,
`reprosearch.sh`, `instrumented.py`, `testing.py`), `TerjeKir/EngineTests`
(`perftsuite.epd` at 127 positions, `eval_symmetry.py`, `mate.py` over
`mate_in_1..8.epd`, `speedup.py`), the OpenBench overview, and the Chess
Programming Wiki's Perft and Perft Results pages.

| what | who runs it | chesso |
|---|---|---|
| perft against a published suite | universal; `perftsuite.epd` is 127 positions | has it and passes all 127 — but only 21 positions are in the repo, 46 layers in the fast suite and 56 in the slow one |
| perft through a transposition table, as a hash sanity check | the technique the CPW Perft page names | `tests/test_perft.cpp` does exactly this |
| divide | CPW; `perftree` | `tests/debug_perft_app.cpp`, perftree-compatible |
| evaluation mirror symmetry | `TerjeKir/EngineTests/eval_symmetry.py` | `test_evaluation` "colour symmetry over every test position", 2696 positions, INV-5 |
| search determinism | Stockfish `tests/reprosearch.sh` | `test_search` "the same search twice gives the same answer" — 3 positions, one depth, fresh table only. `reprosearch.sh` also varies the node limit over 20 values and crosses `ucinewgame` |
| **bench node signature** | Stockfish `tests/signature.sh`; OpenBench requires `bench` in every commit message | **absent.** `tools/search_bench.py` is the same instrument — 3 positions, fixed depth, prints nodes and best move — but no reference number is recorded and nothing fails. `adocs/testing.md` records INV-6 as "compared by hand per change" |
| **sanitizer and valgrind runs** | Stockfish `tests/instrumented.py`, over bench and perft | **absent from the gate.** ASan and UBSan were run by hand at the 2026-08-13 audit; the debug build carries the invariant asserts and F03 is why it is not automatic |
| **continuous integration** | every engine on OpenBench | **absent.** No `.github/`. Everything above rests on a local gate plus a person |
| **mate suites** | `TerjeKir/EngineTests` ships `mate_in_1..8.epd`, 6270 positions | mate in one (3 positions), mate in two (2), mate in zero (1, and F02 says it is illegal). `CLAUDE.md` names pruning that hides a mate as the recurring bug in this engine |
| tactical EPD suites (WAC, Bratko-Kopec, ERET, STS) | historically standard, largely displaced by SPRT | 9 hand-built positions, and this is **deliberate**: DEC-019 and INV-6 say published figures and suite scores decide what to try, never what to conclude |
| SPRT against a named commit | OpenBench, fishtest | `fastchess.sh`, with `test_fastchess_script.sh` guarding the script itself |
| SEE unit tests | uncommon in-repo | ahead of the field: 6 pinned exchange values, plus `see_ge` checked against `see` over every legal move of all 2696 corpus positions at 8 thresholds |
| UCI surface golden test | uncommon | `test_uci_surface`, holding `MANUAL.md` to the dispatch table and the option lines. DEC-024, DEC-028 |
| Chess960 perft, Syzygy probing | Stockfish and most modern engines | not applicable — neither is implemented |

The gaps are automation, not ideas. Every absent row is something the
comparison projects get from CI and this one gets from a person following
`DEV_MANUAL.md`. Where chesso is ahead: the SEE cross-check, the surface guard,
the tuner-model-versus-engine identity (`test_eval_model`), the tuner group
partition and split tests, and the shell-script tests for `fastchess.sh` and
`clang-format.sh` — neither Stockfish's `tests/` nor `TerjeKir/EngineTests` has
anything corresponding to those last two, which is the extent of the sampling
behind the claim and not a survey.
