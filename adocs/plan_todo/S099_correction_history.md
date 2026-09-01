id:         S099
goal:       a static evaluation correction learned from the difference between the static score and what the search returned, keyed on the pawn structure
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); a table keyed on the pawn structure, updated with a running average of the difference between the search score and the static score at nodes where that difference is meaningful, and applied as a bounded correction to the static score; the correction is bounded so it can never turn a non-mate score into a mate score or cross a mate bound, asserted by a test; INV-5 still holds -- the corrected score is side-to-move relative and mirroring a position agrees rather than negates; the correction is cleared on ucinewgame; every constant in src/search_params.hpp with a stated range (S073); the fast suite green
touches:    src/search.cpp, a new table beside the transposition table, src/search_params.hpp, tests/test_search.cpp, tests/test_evaluation.cpp for the mirror property
excludes:   any change to evaluate() itself; NNUE, which is parked at DEC-054; continuation-indexed and material-indexed correction tables, which are a later step if the pawn-keyed one measures positive
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## What it is and is not

It is not a second evaluation. It is a per-key running correction on the static
score, learned inside one search from the only stronger estimator available --
what the search itself returned for that position. It is the last item in the
DEC-071 search block because it needs the static evaluation to be cheap to
obtain (S094) and stable (the block before it).

## Hazard

Anything that adjusts a score near a mate bound can manufacture or destroy a
mate score. The bound is asserted by a test, and the "pruning does not hide a
forced mate" case is re-run.


## Scoped to the pawn table, 2026-08-19

The family is three steps -- this one, S110 non-pawn, S111 continuation --
because the surveyed record measures them separately and they are **not** inert
apart, so DEC-082 does not apply. This one is first because it is the one every
engine added first and the one with the largest reported figure.

Two constraints that are this project's rather than the literature's. The
pawn-structure key must be maintained in `add_piece`, `remove_piece` and
`move_piece` and never rebuilt in `evaluate()` -- that is INV-4, and rebuilding
it is how the 25 % of nodes per second S014 removed comes back. And the
corrected score must never reach the mate or decisive band; one engine carries a
commit named for exactly that bug.

It reads the static evaluation S108 supplies and everything downstream of it --
reverse futility, null move, the S109 block, razoring -- shifts when it lands,
so the margins are re-fitted after it rather than before.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `cf89e22`; re-locate by symbol if drifted. Every GitHub read
was a commit message or PR conversation, never a diff (DEC-016). No dedicated
talkchess origin thread was traced; the public record is CPW plus engine
commits and PRs.

### 1. State of the art

CPW carries the technique as **Static Evaluation Correction History**
(corrhist). Origin: **Caissa**, Witek902, October 2023 -- traced to two commit
messages: aff75594 (2023-10-24) "Pawn structure's eval correction term",
**+1.74 +/-1.68 over 76718 games**, and 6dd05529 (2023-10-26) "Dynamic
material score correction" (+3.66/+8.99), which computes the average error
`static eval - search score` per key and subtracts a portion at evaluation.
The spread: **Stockfish b4d995d** (PR #4950, Vizvezdenec, merged 2023-12-31)
"Introduce static evaluation correction history", pawn-structure keyed,
credited to Caissa; passed STC <0.00,2.00> over 128672 games and LTC
<0.50,2.50> over 97422. A year later Caissa re-imported the refined update
(190abaa7, 2024-10-06, "SF-style correction history") for **+20.10 +/-6.82 /
+30.30 +/-12.17** -- the update rule's form is worth more than the original
term was, in the engine that invented it.

**Update rule, published FORM** (CPW): an exponential moving average per
entry, `entry = (entry*(SCALE - w) + scaled_diff*w) / SCALE`, clamped to
`+/-CORRHIST_MAX`, with weight `w = min(depth*depth + 2*depth + 1, 128)`;
`diff = search_score - static_eval`. The SF commit message states the same
shape in prose: update proportional to `(best_value - static_eval)` times a
linear function of depth, per-update change capped at half the table maximum.

**Update gating, published** (CPW, verified against the SF message): update
only when **not in check**; only when the **best move is absent or quiet**
(the SF message states the narrower "exclude fail-highs whose move is a
capture"); **bound-consistent** -- a lower bound (fail-high) only when
`score >= static_eval`, an upper bound only when `score <= static_eval`,
exact scores always. **Mate scores are excluded from the update**: not on the
CPW page, but on the commit record in prose -- Amethyst 2025-03-17 "Don't add
mate scores to corrhist tables", PiChess 2026-06-08 "clamp corrhist correction
to +/-150cp + skip mate-score training". This is the commit family the Hazard
section anticipated.

**Application, published**: `corrected = raw_eval + entry / GRAIN`, and the
corrected value is **clamped inside the non-decisive band** (CPW gives
`-MATE_FOUND+1 .. MATE_FOUND-1`; the SF form clamps inside the tablebase
range; Rogatia 2026-07-29 widens to "clear of the decisive range, not just the
mate range"). The correction touches the **static eval only**: never a TT
cutoff score, never a returned search score, never the score field of a store.
The SF introduction keeps a separate raw value (prose descriptions name it
`unadjustedStaticEval`) and **stores the raw eval in the TT**, correcting at
use -- the layering that stops a correction compounding through storage.

**The traced +11.4**: Lynx PR #1662 "Pawn correction history / corrhist",
merged 2025-04-15, **+11.35 +/-5.16 over 7502 games** at 8+0.08 1t 32MB, LLR
2.94 [0.00, 3.00] -- DEC-087's "+11.4 at ~2850". PR #1663 "no king in hash"
then removed the king from the key: **+3.27 +/-2.43 over 33028 games** on top,
and **+12.09 +/-5.37** measured alone. So the measured endpoint is a **pure
pawn key**, and chesso goes straight there. Why pawn-keyed first: the pawn
structure is the position feature most stable across a subtree, the origin
term was pawn-keyed, SF's introduction chose it, and DEC-087 found it is the
only correction table with sub-3000 evidence (non-pawn/continuation measure
+3 to +8 above ~3100 -- S110/S111, reserve). Caissa's LTC figures exceeding
its STC figures (8.99 vs 3.66; 30.3 vs 20.1) say the technique scales up with
time control, so 8+0.08 likely understates it.

### 2. Shape for chesso

**Pawn key: absent, this step adds it.** Verified: `board_t` carries one
`hash` (src/data_structures.hpp:298); the three-function alphabet
`add_piece`/`remove_piece`/`move_piece` xors only `board->hash`
(src/bitboard.cpp:685, :702, :724-725); `unmake_move` restores the hash by
copy from the history entry (:992); `compute_full_hash` (:1518) is the only
from-scratch rebuild. No pawn-only key exists anywhere. Plan under INV-4:
`hash_t pawn_hash` in `board_t`, xored in the three alphabet functions when
the piece is W_PAWN/B_PAWN, reusing the existing `piece_randoms` -- pawn
placement only, both colours in one key, **no side/castling/ep randoms**
(side-to-move is the table's index dimension instead, and #1663 says no
kings). Promotion removes the pawn from the key via the existing
`remove_piece` call (src/bitboard.cpp:803); en passant removes the victim
(:812). Restore on unmake by copy, like `hash`: one `pawn_hash` field in
`history_entry_t` (16 -> 24 bytes -- a timing question the DEC-083 run
answers; the alternative, open-coded xor-undo at unmake's four pawn-touching
sites, avoids the growth at the cost of more edit sites). FEN load and
`cleanup_board` (:1494) get a `compute_full_pawn_hash` sibling, and the
make/unmake asserts alongside `eval_accumulators_match` (:742, :875, :997)
check key-equals-recomputation in debug builds. Never rebuilt in `evaluate()`
-- INV-4, and `excludes:` already bars touching `evaluate()` at all.

**Table**: beside the TT per `touches:` -- a static next to `tt`
(src/chesso.cpp:48), wired through `search_state_t` like `state.tt`
(:646-647), surviving across `go` within a game, cleared in
`command_ucinewgame` (:1084-1097) where `tt_reset` already runs. Shape as
published FORM: `entry[stm][pawn_hash & (N-1)]`, N a power of two, int32
entries. Indexing by side to move is CPW's "Color and Hash", and it is what
INV-5 requires: eval and search score are both stm-relative at the node, so
the diff is stm-relative and White's and Black's errors for one structure must
not share a slot.

**Where it applies, post-S108**: at S108's single compute-or-read site (top of
node, after the in-check test at src/search.cpp:687 and the TT-cutoff return
at :460-463). `corrected = raw + correction(stm, pawn_hash)`, computed once.
Consumers see **corrected**: the stack slot `static_evals[ply]` (so improving
compares corrected values -- Lynx #1999 orders correction before improving,
per S108 section 7), the RFP margin (:529-536 today), then S109 futility,
S114 null-move scaling, S116 razoring as they land. The TT eval field stores
**raw** -- S108 section 7 reserves exactly this, and the SF layering above is
the published reason. The update diff uses **raw** in v1 (`best_so_far -
raw_eval`): published prose does not pin raw-vs-corrected for the diff, raw is
the CPW definition and the testable one; the SF-style residual variant is a
recorded later tweak. Quiescence is untouched in v1 -- stand-pat correction is
a possible later verdict, stated here so the scope is explicit.

### 3. Implementation sketch

(a) **Pawn key only -- behaviour-neutral, proven per DEC-083.** The key is
written and read by nothing, so node counts and best moves cannot move; the
claim is sound because the only observable effects are struct sizes (board_t
216 -> 224 B, history_entry_t 16 -> 24 B), which are timing, not shape.
search_bench at depths 9 and 12 node-identical, one interleaved timing
recorded. Tests, red-first: pawn_hash after unmake equals before make (INV-2
pattern) across a promotion, an en-passant capture and a pawn capture; FEN
round-trip equals `compute_full_pawn_hash`; a knight move leaves the key
untouched.

(b) **Table + update + apply -- the SPRT.** Update beside negamax's one store
(:813-815), where `type`, `best_move` and the raw eval are all in scope and
`state->aborted` has already returned (:729). Gate: not in check, raw eval
present (not TT_EVAL_NONE), best move absent-or-quiet (`MOVE_CAPTURE`), bound
consistency by `type` (TT_BETA needs `score >= raw`, TT_ALPHA needs
`score <= raw`, TT_PV always), `|best_so_far| < MATE_MIN` -- and the diff uses
`best_so_far`, **never** the ply-normalized `to_store` (:813). Apply at the
S108 site; corrected value clamped inside `+/-(MATE_MIN - 1)`. Unit tests:
update bounded (drive huge diffs, entry stays inside the clamp); sign (a
positive diff raises the corrected eval for the same stm+key, and leaves the
other stm's slot untouched); no update in check / on a capture best move / on
a mate score / on a wrong-direction bound -- each with the precondition
established first, non-vacuously; correction applied at the consumer (plant an
entry, the RFP decision at :536 shifts) and nowhere else (the TT eval field
reads back raw with a planted correction live); the accepts' mate-bound test;
the INV-5 mirror test; cleared on ucinewgame.

### 4. Constants and seeds

- Update weight `w = min(depth*depth + 2*depth + 1, 128)` -- CPW. **Seed --
  must be fitted/SPSA'd here.**
- Per-update cap = half the entry clamp; applied correction = `entry / 32`
  with the max adjustment ~32 internal units (implying an entry clamp near
  1024 in SF's scale) -- SF commit message prose. **Seed -- must be fitted
  here**, and SF internal units are not centipawns: take "max correction of
  roughly a third of a pawn" as the shape, not the number.
- EMA `SCALE`, `GRAIN`, entry clamp, table entry count: **no publishable
  seed** -- CPW names them without values and no PR states them in prose. Own
  choices: SCALE 256 (>= max weight), GRAIN 256, clamp sized so the applied
  maximum lands in the 32-64 cp class, 16384 entries per stm side, all
  power-of-two, all in src/search_params.hpp with stated ranges (S073), all
  swept.
- CPW also republishes SF's literal `66 * cv / 512` application snippet.
  That is verbatim engine source quoted on a wiki: under DEC-084's intent it
  is not taken as a seed. The generic form above is what is taken.

### 5. Pitfalls

- **Double correction.** The invariant: the correction applies **exactly
  once, at read, to the static eval only**. Raw goes into the TT eval field
  and into the update diff; corrected goes to the stack and the margins. If
  corrected were stored, every store-read cycle would re-correct -- the drift
  the published raw-in-TT layering exists to prevent. A test plants a
  correction and asserts the stored eval field still reads back raw.
- **Mate contamination, both directions.** Update-side: skip `|score| >=
  MATE_MIN` (Amethyst's commit is the published record of learning this the
  hard way). Apply-side: clamp corrected inside the band (CPW; Rogatia widens
  it to all decisive scores). Re-run the "pruning does not hide a forced
  mate" cases -- the recurring bug of this codebase.
- **In-check nodes.** S108 stores the TT_EVAL_NONE sentinel and skips the
  eval in check -- so no read and no update happens there by construction;
  the test still drives an in-check node and asserts the table is untouched,
  precondition first.
- **Sign and normalization.** Everything is stm-relative (INV-5): index by
  `active_color`, never negate on read; and the update reads `best_so_far`,
  not the normalized `to_store` -- `normalize_score` shifts the mate band by
  ply and the mate gate must see the search value.
- **S100 ordering.** A systematic evaluation bias -- five terms fitted to
  exactly zero -- is precisely what a correction table would partially mask.
  S100's diagnosis runs first in plan order; keep it that way, or the
  diagnostic reads a symptom this step has papered over.
- **Never correct a lazy bound.** The S108 site computes full `evaluate()`;
  quiescence's `evaluate_lazy()` bound (:247) is window-relative and adding a
  correction to it is neither a bound nor a score. Out of scope in v1 anyway.

### 6. Measurement

- (a) owes node-identical search_bench at depths 9 and 12 plus one
  interleaved timing (DEC-083, 1.43 Elo/% named as a conversion). Expect a
  small negative: three xor sites and 8 bytes per history entry.
- (b) owes **one SPRT** at the S105 regime (8+0.08, Hash 16, UHO book),
  gainer bounds `elo0=0 elo1=5`: the Lynx-band expectation ~+11 sits above
  elo1, so the bounds do not straddle the effect (DEC-063) and the run
  resolves fast. Fast suite and the mate cases green first. Verdict recorded
  whatever it is (INV-6); a fail is bisected update-gating-first, because the
  published failure modes are gating bugs.

### 7. Interactions

- **S108 (input).** Supplies the per-node raw eval, the stack and the TT eval
  field this step corrects and preserves; plan order guarantees it lands
  first. Keep S108's rule: the TT eval field stays raw.
- **S109 / S114 / S116 (consumers).** Their margins read the corrected value
  through the same variable. They are fitted before this lands and re-fitted
  after (S127) -- the file's own note, unchanged.
- **S110 / S111 (reserve, DEC-087).** Write the table as one parameterised
  struct (key space, weight, clamp, grain) instantiated once for the pawn
  table, so a non-pawn or continuation instance is a declaration, not new
  plumbing. Do not build them now.
- **S118 (pawn eval cache).** Needs exactly the pawn key this step maintains
  -- built once here, reused there. The two tables stay separate structures.
- **S126 (refit).** The correction learns what the eval lacks; after the
  refit there is less to learn and the table's measured value should shrink.
  That is expected, not a regression.

### 8. References

- https://www.chessprogramming.org/Static_Evaluation_Correction_History --
  origin credit, EMA form, weight formula, gating list, apply clamp, family.
- https://api.github.com/search/commits?q=repo:Witek902/Caissa+correction --
  aff75594 pawn term +1.74; 6dd05529 material; 190abaa7 SF-style +20.1/+30.3;
  6e71545 continuation (credits Motor); 39a3b7f gating tweak; 54aef4a
  bad-capture-per-SEE update +2.63 (2026 refinement, not for v1).
- https://github.com/Witek902/Caissa/commit/6dd05529dea17e7a675dc593e4e0ab99965a306e
  -- message only: average error per key, portion subtracted at eval.
- https://github.com/official-stockfish/Stockfish/commit/b4d995d0d910044cf4ea2ad3ee30fd1d21070cd8
  -- the SF introduction message: mechanism, caps, /32 application, STC/LTC.
- https://github.com/official-stockfish/Stockfish/pull/4950 -- PR prose,
  author, date, Caissa credit.
- https://github.com/lynx-chess/Lynx/pull/1662 -- **+11.35 +/-5.16, 7502
  games, 8+0.08** -- the +11.4 the plan cites.
- https://github.com/lynx-chess/Lynx/pull/1663 -- pure pawn key (+3.27 on
  top; +12.09 alone).
- https://api.github.com/search/commits?q=repo:lynx-chess/Lynx+correction+history
  -- the two Lynx shas and dates.
- https://api.github.com/search/commits?q=corrhist+mate -- Amethyst, PiChess,
  Rogatia mate/decisive-band commits, messages only.
- https://github.com/mcthouacbb/Sirius/pull/176 -- minor-piece corrhist
  +7.43 +/-4.78 (the S110-family band, for the reserve file's context).
- https://github.com/zzzzz151/Starzix/pull/98 -- pieces corrhist, merged
  2024-08-05, no prose detail (read and found empty, recorded as such).
