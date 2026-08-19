id:         S107
goal:       a quiet move that gives check becomes eligible for the killer, history and countermove tables it is excluded from today
accepts:    an SPRT verdict, recorded whatever it is; the `is_check_move` term is removed from the killer, history and countermove condition in `negamax`'s fail-high block, and the step states what `is_check_move` is still needed for after the change -- if the answer is "the late move reduction guard alone", say so, because that is what S020 then has to preserve; the fast suite green
touches:    src/search.cpp negamax
excludes:   computing the in-check state once per node, which is S020; any reduction change, which is S098
decisions:
closes:
blocks:
paused_by:
done:

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
