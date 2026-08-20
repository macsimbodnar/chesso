id:         S107
goal:       a quiet move that gives check becomes eligible for the killer, history and countermove tables it is excluded from today
accepts:    an SPRT verdict, recorded whatever it is; the `is_check_move` term is removed from the killer, history and countermove condition in `negamax`'s fail-high block, and the step states what `is_check_move` is still needed for after the change -- if the answer is "the late move reduction guard alone", say so, because that is what S020 then has to preserve; the fast suite green
touches:    src/search.cpp negamax
excludes:   computing the in-check state once per node, which is S020; any reduction change, which is S098
decisions:
closes:
blocks:
paused_by:
done:      2026-08-20. One line: the fail-high gate `!is_capture && !is_check_move` became `!is_capture`, so a quiet move that gives check now enters the killer, history and countermove tables it was excluded from all three of. No comment or decision had ever argued for the exclusion and the published record does not either -- CPW's Killer, History and Countermove pages each gate on a non-capture at a cutoff and none carries a check condition. The accepts' required statement: after this step `is_check_move` is needed for the late move reduction guard alone (src/search.cpp:710), and that is what S020 has to preserve; `grep -rn is_check_move src/ tests/` returns the definition and that guard, nothing else. The comment above the definition was rewritten in the same commit -- it claimed the flag was "only ever needed to decide whether a quiet move may become a killer", false in both halves after the change, and leaving it would invite S020 to delete a flag late move reduction still needs. H1 accepted against elo0=-5 elo1=0 in 3812 games and 1 h 37 m 52 s against ec4d1dd: LLR 2.95 (100.1%) (-2.94, 2.94), Elo 12.67 +/- 8.65, nElo 16.18 +/- 11.03, LOS 99.80 %, 51.82 %, Ptnml(0-2) [157, 406, 711, 405, 227], PairsRatio 1.12, DrawRatio 37.30 %, 0 time forfeits in 3813, 31.2 % draws over the run's own PGN. What that establishes is the pre-registered claim -- not a regression of 5 Elo or more -- with the sign positive; it is not a gain of 12.67, because an SPRT stops early exactly when the observed effect has run favourable and the point estimate is biased upward (DEC-063, and S068 is the case where a pooled estimate fell from +12.18 to +5.02 under that correction). Crediting a magnitude needs a gainer pair, the accepts does not ask for one, and this machine's time is the binding constraint. Play-altering and measured so, INV-6 not available: search_bench at depth 11 reads 558693 / 2402718 / 325059 against ec4d1dd's 563497 / 2420695 / 287100 -- -0.85 %, -0.74 %, +13.2 % -- with the best move unchanged at all three (c3d5 / e2a6 / d7c8q). The tactical position going the other way is the shape an ordering change has rather than a pruning one: admitting a class of move to the killer slots displaces whatever was in them. One test, red first and non-vacuous. Driven at a node rather than through search(), because a countermove needs a non-zero prev_move and search() passes 0 at the root, so a root-only test reaches two of the three tables and never the third: negamax(48499, 48500, 1, 1, ..., f8g8, false) on 6rk/6pp/7N/8/8/8/8/6K1 w - - 0 1. Only a mate score clears that beta, so the move that fails high is the mate rather than whichever move the ordering tried first, and S093 and S098 can move the order freely without touching the case; the bound also sits above MATE_MIN, the guard reverse futility and null move pruning both carry, so neither may answer for the node. Position from tools, not from the board (DEC-023): python-chess gave the whole mating set as [('Nf7#', 'h6f7', False)] over 9 legal moves, stockfish dev-20260810 on the position after h6f7 answered `Checkers: f7` / `info depth 0 score mate 0` / `bestmove (none)`, and python-chess confirmed f8g8 legal in 5r1k/6pp/7N/8/8/8/8/6K1 b - - 0 1 and landing on the FEN. The case asserts what the move is -- not a capture, not a promotion, gives check when made -- then presence in all three tables and never a magnitude or the update formula, so S093's bonus, indexing and ageing rewrite does not have to rewrite it. Red at HEAD with the fail-high precondition passing first, so the red was the table and not a node that never cut off: `:556 REQUIRE( killer != 0 )` -- `values: REQUIRE( 0 != 0 )`, 13 of 14 assertions passing. Green after the deletion, 20 of 20. Noted and not fixed, and now one class wider than it was: a non-capture promotion passes the gate and can occupy a killer slot, checking promotions included since this change. Harmless as things stand -- score_move ranks promotions in the capture band at evaluation.cpp:1097, before killers are consulted at :1099 -- and CPW's quiet definition excludes promotions, so it is a wrinkle to close deliberately rather than in passing. The band arithmetic is untouched by construction: no value moved. ctest -L fast 16/16 in 12.62 s release and 213.89 s debug; clang-format clean; moltke --validate clean; plan_prose_check 0 flagged. README.md is owner-written, no change needed; MANUAL.md unchanged -- no UCI flag, default or output moved, and its `killer` entry is a bench FEN shortcut unrelated to the tables; DEV_MANUAL.md unchanged -- no build, test or bench invocation moved. specs.md's "absent, machinery" row now states the single eligibility predicate and carries the verdict. plan.md pruned S103 to keep five done entries, which took the four testing.md rows naming S103 alone with it.

## What is there, and why it looks wrong

`src/search.cpp`, the fail-high block:

    if (!is_capture && !is_check_move) {
      ... killer, history, countermove ...
    }

A quiet move that gives check is excluded from **all three** ordering tables.
Those are exactly the moves most likely to be the refutation at a sibling node,
and the engine refuses to remember any of them. No comment in the file argues
for the exclusion and no decision records it; it reads like an accident of the
condition being written once for two purposes.

Small, one line, and it alters play -- so it owes a verdict like anything else,
and a verdict of zero is recorded as zero. It goes early because it changes the
tables every later history step reads, and measuring those on a table with a
hole in it measures the hole.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

Published eligibility for all three tables is one predicate: a non-capture
that failed high. CPW Killer Heuristic: on a fail-high "a quiet move that
caused a cutoff is stored", two or three distinct slots per ply. CPW History
Heuristic: the update is gated on the move being a non-capture at a cutoff,
bonus a multiple of depth or depth*depth, indexed `[from][to]` (butterfly) or
`[piece][to]`. CPW Countermove Heuristic: a non-capture causing a cutoff,
indexed by the previous move's `[from][to]` or `[piece][to]`. None of the
three pages carries any check condition. CPW Move Ordering's canonical
sequence -- PV/hash move, winning captures, equal captures, killers ("non
capture"), history-sorted non-captures, losing captures -- has no checking
class at all, and no practitioner scheme in the talkchess "Move ordering -
what's best?" thread excludes or special-cases checks in the tables either.

The one place the record even permits a check carve-out is the definition
page: CPW Quiet Moves defines quiets as "all moves which do not alter
Material, thus no captures nor promotions" and adds that one "can also
exclude moves that present imminent threats, such as check" -- an optional
clause for pruning-type uses that no table page takes up. That confirms this
file's diagnosis from the record: one "quiet" predicate written for two
purposes. The reduction purpose keeps the check term (CPW Late Move
Reductions lists "moves which give check" and "moves while in check" among
the non-reduced classes); the table-eligibility purpose never had it.

No per-patch Elo record exists for "admit checking quiets to the tables",
because no surveyed engine had the exclusion to remove. The conceptual cost
of keeping it: the class of quiets this engine's own LMR refuses to reduce
because "those lines are forcing" (src/search.cpp:683-686) is the class its
ordering memory refuses to remember, so every sibling re-finds a checking
refutation from raw generation order -- and once S109 lands, a permanent
history of 0 reads as maximally unpromising to history pruning.

### 2. Shape for chesso

- src/search.cpp:662 -- `is_check_move`, computed per legal quiet as
  `is_capture ? false : is_check(game)`, the child's in-check state.
- src/search.cpp:744 -- the fail-high gate `if (!is_capture &&
  !is_check_move)` in front of **all three** updates: killers 745-746,
  history 749-752 (depth*depth, saturated at ORDER_HISTORY_MAX), countermove
  754-757 (written only when `prev_move != 0`; the root and the child of a
  null move carry 0).
- src/search.cpp:694 -- the only other consumer: the LMR guard.

Exhaustive: `grep -rn is_check_move src/ tests/` returns exactly those three
lines. The tables' only reader is score_move, src/evaluation.cpp:1099-1108.
Quiescence is untouched: it orders by capture_score alone (src/search.cpp:327)
and never reads or writes any of the three tables. Continuation history (S024)
and malus/gravity/persistence (S093) do not exist yet; data_structures.hpp:447-457
confirms killers, history_moves[12][64] and counter_moves[12][64] are the set.

Minimal change: delete `&& !is_check_move` at src/search.cpp:744. In the same
commit, rewrite the comment at 660-661 -- "only ever needed to decide whether
a quiet move may become a killer" becomes false. The accepts' required
statement, answered: **after S107, `is_check_move` is needed for the late
move reduction guard at src/search.cpp:694 alone, and that is what S020 has
to preserve** -- not dead weight, since published LMR practice keeps the
exemption. Leave line 662 itself alone: making the computation conditional is
S020's scope, excluded here.

### 3. Implementation sketch

One deletion plus one test, red first and non-vacuous (AGENTS.md section 6):

1. Position selection is a tool job (DEC-023): a FEN whose refutation is a
   non-capture checking move, verified with `stockfish` on the FEN
   (`go depth 20`, bestmove a quiet check) or taken from the mate fixtures;
   record the tool output in the step notes.
2. Precondition evidence at HEAD: a scratch build with the gate inverted
   confirms the chosen position and depth actually write that move into the
   tables when eligible. Recorded, not committed.
3. The committed test (beside "search: move ordering state",
   tests/test_search.cpp:453-502): fixed-depth search from the FEN with an
   owned `search_state_t`; assert the move appears in some killer slot, its
   `history_moves[MOVE_PIECE][MOVE_TO]` cell is non-zero, and it appears as a
   stored counter_moves value. **Presence only, never magnitudes or the
   update formula** -- S093 replaces bonus, indexing and ageing later and
   must not have to rewrite this test.
4. Observe red at HEAD and record the output; apply the deletion; green;
   fast suite green.

### 4. Constants and seeds

None. No band value or parameter moves.

### 5. Pitfalls

- Band arithmetic is unchanged by construction: a checking quiet lands in the
  existing bands (killers 900000/800000, counter 700000, history saturated at
  ORDER_HISTORY_MAX = 600000, search_params.hpp:45; bands at
  src/evaluation.cpp:33-37). The CLAUDE.md 100-point clearance hazard is not
  touched because no value moves -- do not "improve" a band while here.
- Do not assert a countermove at the root: `prev_move` is 0 there and after a
  null move (search.cpp:576-577, 754).
- Quiescence needs no assertion -- it never consults the tables (section 2).
- Pre-existing wrinkle, out of scope: a non-capture, non-checking promotion
  passes the 744 gate today and can occupy a killer slot, though score_move
  scores promotions in the capture band before killers are consulted
  (evaluation.cpp:1097 before :1099). CPW's quiet definition excludes
  promotions. Note it, do not fix it here.
- The 660-661 comment rewrite is part of the change: a stale "only needed for
  killers" comment invites S020 to delete a flag LMR still needs.

### 6. Measurement

Play-altering: a checking quiet entering a slot reorders siblings, so
search_bench counts are expected to differ and this cannot be discharged as
behaviour-neutral (INV-6). SPRT under the S105 regime -- 8+0.08, Hash=16, UHO
book; S105 precedes this step in plan order. Bounds: the accepts keeps the
removal whatever the verdict says ("recorded whatever it is"), which is fix
semantics, so the S105 non-regression pair `elo0=-5 elo1=0` fits; DEC-063
applies, and no published effect-size seed exists for this patch, so a pair
straddling small effects is the safe choice. A gainer run at `elo0=0 elo1=5`
is legitimate if a gain is to be credited, but an H0 there does not un-ship
the fix. A zero is recorded as zero.

### 7. Interactions

- S093 (malus, gravity, butterfly, persistence): inherits check-inclusive
  eligibility, so its malus will also apply to checking quiets that were
  tried and failed -- the published shape. S107's test asserts presence only
  so S093 never has to touch it.
- S024 (continuation history): its update lands in the same fail-high block;
  after S107 the gate there is `!is_capture` alone, so continuation history
  is born check-inclusive by default. S107 leaves exactly one eligibility
  predicate at that site.
- S109 (history pruning): consults these scores; landing S107 first means its
  thresholds are fitted against a table without the hole. S109's own
  gives-check *pruning* exemption is the other purpose the old single
  condition conflated -- the two stay separate conditions.
- S020 / S098: the LMR guard is the sole surviving consumer of
  `is_check_move`; S020 must preserve that information when it restructures
  the in-check computation, and S098 owns any change to the exemption itself.

### 8. References

- https://www.chessprogramming.org/Killer_Heuristic -- eligibility "a quiet move that caused a cutoff", 2-3 distinct slots per ply; no check condition anywhere.
- https://www.chessprogramming.org/History_Heuristic -- non-capture-at-cutoff update, depth*depth-class bonus, butterfly vs [piece][to], gravity noted; no check exclusion.
- https://www.chessprogramming.org/Countermove_Heuristic -- non-capture at cutoff, indexed by the previous move's [from][to] or [piece][to]; no check exclusion.
- https://www.chessprogramming.org/Quiet_Moves -- "no captures nor promotions", plus the optional "can also exclude ... check" clause no table page takes up.
- https://www.chessprogramming.org/Move_Ordering -- the canonical ordering sequence; checking moves are not a class in it.
- https://www.chessprogramming.org/Late_Move_Reductions -- "moves which give check" / "while in check" among non-reduced classes; what the surviving consumer implements.
- https://www.talkchess.com/forum3/viewtopic.php?t=69968 -- practitioner ordering schemes (hash, SEE-sorted captures, killers, history, losing captures); none excludes checks from the tables.


## What was done (2026-08-20)

One deletion in `src/search.cpp`. The fail-high gate

    if (!is_capture && !is_check_move) {

became

    if (!is_capture) {

and the comment above `is_check_move` was rewritten in the same commit: it said
the flag was "only ever needed to decide whether a quiet move may become a
killer", which the change makes false in both halves -- it is not needed for
that and it is needed elsewhere.

**The accepts' required statement.** After this step, `is_check_move` is needed
for the **late move reduction guard alone** (`src/search.cpp:710`, the
`!is_check_move` term beside `!is_in_check` and `!MOVE_PROMOTED`). That is what
S020 has to preserve when it restructures the in-check computation: the flag is
not dead weight, published LMR practice keeps the exemption, and deleting it
along with its old comment would silently start reducing forcing lines.
`grep -rn is_check_move src/ tests/` now returns two lines, the definition and
that guard.

## The test, and the red it was observed under

`tests/test_search.cpp`, suite `search: move ordering state`, case "a quiet move
that gives check enters the ordering tables".

Driven at a node rather than through `search()`, because a countermove needs a
non-zero `prev_move` and `search()` passes 0 at the root -- so a root-only test
could assert two of the three tables and never the third.

`negamax(48499, 48500, 1, 1, &game, &state, prev_move, false)` on
`6rk/6pp/7N/8/8/8/8/6K1 w - - 0 1`, with `prev_move` = black's `f8g8`, the move
that reaches the position.

Position selection was a tool job, not a reading of the board (DEC-023):

- python-chess over the legal moves reported the **whole** mating set as
  `[('Nf7#', 'h6f7', False)]` -- one mate in one, and it is not a capture. 9
  legal moves, `is_valid()` true.
- `/usr/games/stockfish` (dev-20260810) on the position after `h6f7`:
  `Checkers: f7`, `info depth 0 score mate 0`, `bestmove (none)`.
- python-chess also confirms `f8g8` is legal in
  `5r1k/6pp/7N/8/8/8/8/6K1 b - - 0 1` and lands on the FEN above.

Why the window is what it is, and why the case does not depend on move order:
`beta = 48500` can only be cleared by a mate score, so the move that fails high
is *the mate* rather than whichever move the ordering happened to try first --
S093, S098 and every later ordering step can move the order freely without
touching this case. The bound also sits above `search.cpp`'s `MATE_MIN`, which
is the guard reverse futility and null move pruning both carry, so neither may
answer for the node. At `depth 1` the mate is reached through quiescence's own
in-check mate return, and `ply 1` keeps the node off the root so the countermove
site is live.

The case asserts what the move **is** rather than a literal it equals -- not a
capture, not a promotion, and gives check when made -- then presence in all
three tables. **Presence only, never magnitude or formula**: S093 replaces the
bonus, the indexing and the ageing and must not have to rewrite this.

**Red at HEAD**, and non-vacuous: the fail-high precondition
`REQUIRE(score >= BETA)` passed and the table assertion is what failed.

    tests/test_search.cpp:556: FATAL ERROR: REQUIRE( killer != 0 ) is NOT correct!
      values: REQUIRE( 0 != 0 )
    [doctest] assertions: 14 | 13 passed | 1 failed |

Green after the deletion: `assertions: 20 | 20 passed | 0 failed`.

## Measurement

Play-altering, as expected, so INV-6 cannot discharge it -- `search_bench` at
depth 11, interleaved against `ec4d1dd`:

| position | ec4d1dd | S107 | |
|---|---|---|---|
| midgame | 563497 | 558693 | -0.85 % |
| kiwipete | 2420695 | 2402718 | -0.74 % |
| tactical | 287100 | 325059 | **+13.2 %** |

Best moves identical at all three (`c3d5` / `e2a6` / `d7c8q`), so the tree moved
without the answer moving on these three. The tactical position going the other
way is the shape to expect from an ordering change rather than a pruning one:
admitting a class of move to the killer slots displaces whatever was in them.

SPRT under the S105 regime, 8+0.08, Hash 16, UHO book, concurrency 12,
`--nonreg` (`elo0=-5 elo1=0`, alpha=beta=0.05) per section 6 -- the removal is
kept whatever the verdict says, which is fix semantics. Candidate `ec4d1dd` plus
this diff against reference `ec4d1dd`; both builds Release, `native`, PGO off.

**H1 accepted.** 3812 games in 1 h 37 m 52 s against `ec4d1dd`,
`LLR: 2.95 (100.1%) (-2.94, 2.94) [-5.00, 0.00]`, `Elo: 12.67 +/- 8.65`,
`nElo: 16.18 +/- 11.03`, `LOS: 99.80 %`, 51.82 %, `Ptnml(0-2): [157, 406, 711,
405, 227]`, `PairsRatio: 1.12`, DrawRatio 37.30 %, **0 time forfeits in 3813**,
31.2 % draws over the run's own PGN.

**What that establishes, and what it does not.** The pair was `elo0=-5 elo1=0`,
so H1 is the pre-registered claim "not a regression of 5 Elo or more", and the
sign is positive at `LOS 99.80 %`. It is **not** a measured gain of 12.67: an
SPRT stops early exactly when the observed effect has run favourable, so the
point estimate is biased upward and is never reported as the effect size
(DEV_MANUAL, and S068 is the case where a pooled estimate fell from +12.18 to
+5.02 under that correction). Crediting a magnitude would need a separate
`elo0=0 elo1=5` run; the accepts does not ask for one, the removal ships either
way, and this machine's time is the binding constraint on the plan. So what
goes in the record is the sign and the bound, not the number.
