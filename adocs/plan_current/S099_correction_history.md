id:         S099
goal:       a static evaluation correction learned from the difference between the static score and what the search returned, keyed on the pawn structure
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); a table keyed on the pawn structure, updated with a running average of the difference between the search score and the static score at nodes where that difference is meaningful, and applied as a bounded correction to the static score; the correction is bounded so it can never turn a non-mate score into a mate score or cross a mate bound, asserted by a test; INV-5 still holds -- the corrected score is side-to-move relative and mirroring a position agrees rather than negates; the correction is cleared on ucinewgame; every constant in src/search_params.hpp with a stated range (S073); the fast suite green
touches:    src/search.cpp, a new table beside the transposition table, src/search_params.hpp, tests/test_search.cpp, tests/test_evaluation.cpp for the mirror property -- amended by the implementer 2026-10-13 (As built, deviation 7): part (a) src/bitboard.cpp, src/bitboard.hpp, src/data_structures.hpp, tests/test_engine.cpp, tests/test_invariants.cpp, tools/mutants/S099_pawn_key.py, adocs/specs.md, DEV_MANUAL.md; part (b) src/correction_history.hpp (the new table), src/data_structures.hpp, src/search.hpp, src/chesso.cpp, src/uci.hpp, tools/datagen.cpp, tests/test_engine.cpp, tests/test_search_params.cpp, tools/mutants/S099_correction_history.py, tools/mutants/S234_tt_estimate_margins.py, MANUAL.md, DEV_MANUAL.md, adocs/specs.md, adocs/data/S099_sprt.sh; phase 2 (DEC-270, DEC-271) adocs/data/S099_pawn_key_census.py and its tune-only instrument in src/search.cpp, src/search.hpp and src/chesso.cpp, and the goldens' re-derivation: tests/test_mate_breadth.cpp, tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv, adocs/data/S099_floor_sweep.log, adocs/data/S099_mined_floor.log, adocs/data/S099_sweep_s170.txt; phase 2 on 6fc7f23 adocs/data/README.md (S099's rows)
excludes:   any change to evaluate() itself; NNUE, which is parked at DEC-054; continuation-indexed and material-indexed correction tables, which are a later step if the pawn-keyed one measures positive
decisions:  DEC-071, DEC-222, DEC-258, DEC-270, DEC-271, DEC-272, DEC-282, DEC-283, DEC-284
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:

## Promoted 2026-10-08, DEC-258: a fixed slot, not an idle night

DEC-222 (1), the owner's ruling of 2026-09-19, was that this step "runs as the
probe ... on the next idle night". The reading rule keeps the machine always
busy, so in 23 SPRT runs after that ruling no night was idle and the probe
never ran. On 2026-10-08 the owner gave it a fixed slot: directly after
S261's search lane and before S082 regenerates the corpus, because the
correction changes the search that plays the corpus games. Phase (a), the
pawn key, is behaviour-neutral and may be written in a worktree during
S119's run and land between runs (DEC-260). The SPRT is phase (b), one
`{0, 5}` nElo verdict as the accepts says; quiescence stays untouched in v1,
as section 2 states -- the quiescence consumer is a separate step.

**On H1**, S110 and S111 run next, each its own verdict, and the two consumers
DEC-222 named -- the correction applied in quiescence, and the correction's
magnitude as a reduction and margin input -- get their own step files then,
each one verdict. **On H0 or no verdict**, S110 and S111 return to the reserve
and no consumer step is created.

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
corrected score must never reach the mate or decisive band; one engine carries
a commit named for exactly that bug.

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

**Update rule, published FORM** (CPW, attribution corrected below): an
exponential moving average per
entry, `entry = (entry*(SCALE - w) + scaled_diff*w) / SCALE`, clamped to
`+/-CORRHIST_MAX`, with the weight a capped function of depth;
`diff = search_score - static_eval`.

**Attribution corrected 2026-09-19, S232.** The 2026-08-19 pass wrote that
update rule and its weight `w = min(depth*depth + 2*depth + 1, 128)` as
"CPW". It is not the wiki's own derivation: the page introduces the block it
sits in with "A version as seen in Alexandria is shown below", so what the
wiki republishes there is another engine's source (re-fetched 2026-09-19).
The *shape* -- an EMA per entry, the weight rising with depth and capped --
is the published form and is what this step takes; the cap **128** is
Alexandria's constant, kept here as record and never a seed, which is
DEC-105's PeSTO case exactly (DEC-134). Section 4 seeds the cap from
chesso's own recorded median depth instead. The SF commit message states a *different*
shape in prose -- update proportional to `(best_value - static_eval)` times a
**linear** function of depth, per-update change capped at half the table
maximum -- which the page introduces as the history-gravity form, "as
introduced by Stockfish"; chesso takes the EMA shape and the two are a sweep
at the owning step, not a seed either way.

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
corrected value is **clamped inside the non-decisive band** (the wiki's
reproduced EMA code gives
`-MATE_FOUND+1 .. MATE_FOUND-1`; the SF form clamps inside the tablebase
range; Rogatia 2026-07-29 widens to "clear of the decisive range, not just the
mate range"). The correction touches the **static eval only**: never a TT
cutoff score, never a returned search score, never the score field of a store.
The SF introduction keeps a separate raw value (prose descriptions name it
`unadjustedStaticEval`) and **stores the raw eval in the TT**, correcting at
use -- the layering that stops a correction compounding through storage.

**The traced +11.4**: Lynx PR #1662 "Pawn correction history / corrhist",
merged 2025-04-15, **+11.35 +/-5.16 over 7502 games** at 8+0.08 1t 32MB, LLR
2.94 [0.00, 3.00] -- DEC-087's "+11.4 at ~2850", **and ~2850 is wrong by about
380 points**. S181 banded it: the merge falls between v1.9.1 (2025-04-05, not
on the CCRL list) and v1.10.0 (2025-06-29), the nearest rated release before it
being v1.9.0 (2025-03-11), so the band is **3224 to 3291** CCRL Blitz 1CPU on
the list computed 2026-09-05 (`adocs/data/S181_lynx_bands.md`). PR #1663 "no king in hash"
then removed the king from the key: **+3.27 +/-2.43 over 33028 games** on top,
and **+12.09 +/-5.37** measured alone. So the measured endpoint is a **pure
pawn key**, and chesso goes straight there. Why pawn-keyed first: the pawn
structure is the position feature most stable across a subtree, the origin
term was pawn-keyed, and SF's introduction chose it. **What is no longer a
reason is DEC-087's:** it kept this table in the main order as "the only
correction table with sub-3000 evidence", and the band above says that figure
was measured at 3224-3291 -- above the ~3100 that demoted the other two, so
that criterion separated nothing. **DEC-133 is the owner's answer**: S099 moves
to the head of the reserve as the family's probe, run on a spare night, with
S110 and S111 gated on its verdict. It returns to the main order if this
engine's own verdict is positive, which is the only band that ever mattered
(DEC-019). `2026-09-04_plan_review-F02`, corrected by S181 on 2026-09-11. Caissa's LTC figures exceeding
its STC figures (8.99 vs 3.66; 30.3 vs 20.1) say the technique scales up with
time control, so 8+0.08 likely understates it.

### 2. Shape for chesso

**Pawn key: absent, this step adds it.** Verified: `board_t` carries one `hash`
(`src/data_structures.hpp` `hash`); the three-function alphabet
`add_piece`/`remove_piece`/`move_piece` xors only `board->hash`
(`src/bitboard.cpp` `add_piece`, `src/bitboard.cpp` `remove_piece`,
`src/bitboard.cpp` `move_piece`); `unmake_move` restores the hash by copy from
the history entry (`src/bitboard.cpp` `unmake_move_impl`); `compute_full_hash`
(`src/bitboard.cpp` `compute_full_hash`) is the only from-scratch rebuild. No
pawn-only key exists anywhere. Plan under INV-4: `hash_t pawn_hash` in
`board_t`, xored in the three alphabet functions when the piece is
W_PAWN/B_PAWN, reusing the existing `piece_randoms` -- pawn placement only,
both colours in one key, **no side/castling/ep randoms** (side-to-move is the
table's index dimension instead, and #1663 says no kings). Promotion removes
the pawn from the key via the existing `remove_piece` call (`src/bitboard.cpp`
`make_move_impl`); en passant removes the victim (`src/bitboard.cpp`
`make_move_impl`). Restore on unmake by copy, like `hash`: one `pawn_hash`
field in `history_entry_t` (16 -> 24 bytes -- a timing question the DEC-083 run
answers; the alternative, open-coded xor-undo at unmake's four pawn-touching
sites, avoids the growth at the cost of more edit sites). FEN load and
`cleanup_board` (`src/bitboard.cpp` `cleanup_board`) get a
`compute_full_pawn_hash` sibling, and the make/unmake asserts alongside
`eval_accumulators_match` -- two in `src/bitboard.cpp` `make_move_impl` and one
in `src/bitboard.cpp` `unmake_move_impl` --
check key-equals-recomputation in debug builds. Never rebuilt in `evaluate()`
-- INV-4, and `excludes:` already bars touching `evaluate()` at all.

**Table**: beside the TT per `touches:` -- a static next to `tt`
(`src/chesso.cpp` `tt`), wired through `search_state_t` like `state.tt`
(`src/chesso.cpp` `iterative_deepening_search`), surviving across `go` within a
game, cleared in `command_ucinewgame` (`src/chesso.cpp` `command_ucinewgame`)
where `tt_reset` already runs. Shape as published FORM: `entry[stm][pawn_hash &
(N-1)]`, N a power of two, int32 entries. Indexing by side to move is CPW's
"Color and Hash", and it is what INV-5 requires: eval and search score are both
stm-relative at the node, so the diff is stm-relative and White's and Black's
errors for one structure must not share a slot.

**Where it applies, post-S108**: at S108's single compute-or-read site (top of
node in `src/search.cpp` `negamax`, after the in-check test and the TT-cutoff
return). `corrected = raw + correction(stm,
pawn_hash)`, computed once. Consumers see **corrected**: the stack slot
`static_evals[ply]` (so improving compares corrected values -- Lynx #1999
orders correction before improving, per S108 section 7), the RFP margin
(`src/search.cpp` `negamax` today), then S109 futility, S114 null-move scaling,
S116 razoring as they land. The TT eval field stores **raw** -- S108 section 7
reserves exactly this, and the SF layering above is the published reason. The
update diff uses **raw** in v1 (`best_so_far - raw_eval`): published prose does
not pin raw-vs-corrected for the diff, raw is the CPW definition and the
testable one; the SF-style residual variant is a recorded later tweak.
Quiescence is untouched in v1 -- stand-pat correction is a possible later
verdict, stated here so the scope is explicit.

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
(`src/search.cpp` `negamax`), where `type`, `best_move` and the raw eval are
all
in scope and `state->aborted` has already returned (`src/search.cpp`
`negamax`). Gate:
not in check, raw eval present (not TT_EVAL_NONE), best move absent-or-quiet
(`MOVE_CAPTURE`), bound consistency by `type` (TT_BETA needs `score >= raw`,
TT_ALPHA needs `score <= raw`, TT_PV always), `|best_so_far| < MATE_MIN` -- and
the diff uses `best_so_far`, **never** the ply-normalized `to_store`
(`src/search.cpp` `negamax`). Apply at the S108 site; corrected value clamped
inside
`+/-(MATE_MIN - 1)`. Unit tests: update bounded (drive huge diffs, entry stays
inside the clamp); sign (a positive diff raises the corrected eval for the same
stm+key, and leaves the other stm's slot untouched); no update in check / on a
capture best move / on a mate score / on a wrong-direction bound -- each with
the precondition established first, non-vacuously; correction applied at the
consumer (plant an entry, the RFP decision at `src/search.cpp` `negamax`
shifts) and
nowhere else (the TT eval field reads back raw with a planted correction live);
the accepts' mate-bound test; the INV-5 mirror test; cleared on ucinewgame.

### 4. Constants and seeds

**Reseeded 2026-09-19 by S232**, under DEC-105 as DEC-134 states it. The
2026-08-19 pass seeded this section from Stockfish commit-message prose and
from an Alexandria code block the wiki republishes; the 2026-09-19 study
review found it (finding F06 of that report) and DEC-222 (1) ordered the
rewrite before the probe night. Every seed below is one of three forms and
says which: **(a)** a value from a publication *about the technique*, with
its URL; **(b)** a derivation over chesso's own data or scale, run by this
step at its start; **(c)** the range midpoint or the off value, stated as
such. Each constant lands in the `CHESSO_SEARCH_PARAMS` X-macro with a stated
range (S073) and each is a **seed -- fitted or SPSA'd here** (S127). No
engine's shipped weight, cap, grain, clamp or size seeds anything, wherever
it is republished; those records stay in section 1 and, as anti-seeds, at the
end of this section.

**Units, once for this file.** The correction is added to `evaluate()`'s
output, so its magnitude is in chesso's own material scale and in nobody
else's internal units: `src/eval_tables.hpp` `PAWN` is **94** at this step's
HEAD, not 100. The header's own comment says the split between a piece's
value and its tables is degenerate, so the material term alone is the unit.
The arithmetic below is written out rather than reduced to a number, so the
seeds re-derive themselves when the scale is refit or the census moves --
re-read both at this step's HEAD before seeding anything.

- **Weight cap** **144** -- **(b), arithmetic over a constant chesso already
  ships.** The weight rises with depth and saturates, and the cap is where it
  saturates: at chesso's own median remaining depth, so a node at the typical
  depth gets full weight and a shallower one less. That median is already a
  named constant in this tree -- `src/data_structures.hpp`
  `CONT_HIST_REF_DEPTH`, **11**, which S222 put there as exactly this kind of
  unit and which S085's own table measured as the median depth its 2+0.02
  tuning control reaches. The shape stays `min((depth + 1)^2, cap)`, so the
  cap is `(CONT_HIST_REF_DEPTH + 1)^2` = `(11 + 1)^2` = **144**; read the
  constant at this step's HEAD rather than the 11 written here, so the seed
  follows it. **One caveat, and the procedure that settles it.** 11 is the
  median at the *tuning* control and the 8+0.08 verdict control reaches
  deeper, so a step that wants its own number re-measures the median at
  8+0.08 over the 300 positions of the S021 stratified pick and re-does the
  arithmetic -- a procedure this step may run at its start, not a value read
  anywhere. Range 1 to the EMA scale below: at the scale the entry is just
  the newest observation and the table has no memory, which is the off value
  of the averaging. The sweep decides, and whether the shape is the quadratic
  or the linear-in-depth one section 1 records is a sweep here too, not a
  seed either way.
- **EMA scale** **256** -- **(b), arithmetic over the cap above.** The update
  divides by the scale and the weight may never exceed it, so the scale is
  the smallest power of two at or above the cap: 144 -> **256**. It is not
  swept independently: the pair sets one quantity, the maximum EMA rate
  `cap / scale`, and that is what S127 moves.
- **Maximum applied correction** **47** -- **(c), the midpoint of a range
  declared by purpose, in chesso's own scale.** The range is 0 to one pawn: 0
  is off, and a correction larger than a pawn rewrites the material balance
  instead of correcting the static score. One pawn is `PAWN` = 94 above, so
  the midpoint is **47**. This is the only quantity on the apply path with
  Elo content in it and the one S127 sweeps; the two constants below are its
  fixed-point spelling and carry no freedom of their own.
- **Grain** **256** -- **(c), the midpoint of a range declared by purpose,
  stated as such.** Nothing derives this one and no publication states it: it
  is the entry's fixed-point resolution, and once the applied maximum above
  is fixed it carries no Elo of its own. The range is 64 to 1024 in powers of
  two, because the division is a shift -- at 64 the stored error is quantised
  to a sixty-fourth of a chesso material unit, at 1024 the worst-case product
  below grows fourfold to buy resolution the EMA cannot use. On a
  power-of-two axis the midpoint is the geometric one, `sqrt(64 * 1024)` =
  **256**, and that is the seed.
- **Entry clamp** **12032** -- **(b), arithmetic over the two seeds above:**
  applied maximum x grain = `47 * 256`. The entry holds the running error in
  fixed point and `entry / grain` is what is added, so clamp and grain move
  together and only the applied maximum is tunable; the arithmetic is written
  out so the clamp re-derives when either end moves. Worst case the update
  forms `clamp * scale` = 12032 x 256 = 3080192, inside int32 by a factor of
  about 700, so the representation does not bind the choice.
- **Table entry count per side to move** -- **(b), a census this step runs at
  its start**, with a **(c)** fallback. Once (a) of the sketch has landed the
  pawn key, instrument it under `CHESSO_TUNE` only, run `go depth 12` over
  the 300 positions of the S021 stratified pick and record the distinct pawn
  keys visited per position. Size the table at the next power of two above
  the 95th percentile, so a search rarely fills it and collisions stay rare,
  and record the percentile table in this step's stamp. If the census cannot
  run before the table is declared, take **(c)**: the exponent range is 2^10
  to 2^20 by purpose -- 2^10 is small enough that every search overfills it,
  2^20 is 4 MiB a side and past what a table read at every node should cost
  -- midpoint exponent 15, so **32768** entries a side.

**Anti-seeds -- records, not seeds.** DEC-019 lets a record say which
direction is worth trying; DEC-105 forbids any of these numbers starting a
sweep, which is why every bullet above rests on chesso's own arithmetic or on
a range declared by purpose. The records, each of them another engine's
constant however it is republished: Alexandria's weight cap **128**, and its
scale, grain, clamp and size symbols, in the code block the wiki reproduces;
Stockfish's application snippet `66 * cv / 512`, literal engine source quoted
on a wiki, which is DEC-105's PeSTO case; and that engine's commit-message
prose -- an applied correction of `entry / 32`, a maximum adjustment of about
**32** internal units implying an entry clamp near **1024** in its own scale,
a per-update change capped at half the table maximum, and "max correction of
roughly a third of a pawn", which is a unit statement in that engine's scale
and not in chesso's. Section 1's commit record adds PiChess's "clamp corrhist
correction to +/-150cp". **None of these is a seed, a fallback, or a sanity
check on the seeds above**, and the per-update cap does not even belong to
the shape chesso takes -- it is the history-gravity form's, not the EMA's.
The 2026-08-19 pass carried them in this section under the hedge "take the
shape, not the number"; the hedge is exactly what F06 found insufficient,
because the numbers were still on the page an implementer reads.

seeds re-derived 2026-09-19 under DEC-105 (DEC-134)

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
- - **Never correct a lazy bound.** The S108 site computes full `evaluate()`;
  quiescence's `evaluate_lazy()` bound (`src/search.cpp` `quiescence`) is
  window-relative and adding a correction to it is neither a bound nor a score.
  Out of scope in v1 anyway.

### 6. Measurement

- (a) owes node-identical search_bench at depths 9 and 12 plus one
  interleaved timing (DEC-083, 1.43 Elo/% named as a conversion). Expect a
  small negative: three xor sites and 8 bytes per history entry.
- (b) owes **one SPRT** at the S105 regime (8+0.08, Hash 16, UHO book),
  gainer bounds `elo0=0 elo1=5`: the Lynx expectation ~+11 sits above
  elo1, so the bounds do not straddle the effect (DEC-063) and the run
  resolves fast. **Read that expectation for what it is now**: it was measured
  at 3224-3291, about 300 points above where this engine will run it, and
  DEC-019's three transfer failures are the reason a figure from another band
  chooses the bounds and never the conclusion. Fast suite and the mate cases green first. Verdict recorded
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
  origin credit, EMA form, gating list, apply clamp, family, and the
  time-control sentence. Re-fetched 2026-09-19 (S232): the weight formula and
  the apply clamp are inside code blocks the page attributes to Alexandria
  and to Stockfish, so they are those engines' source republished and not the
  wiki's own -- section 1's corrected attribution and section 4's anti-seeds.
  The gating list and the time-control sentence *are* the page's own prose.
- https://api.github.com/search/commits?q=repo:Witek902/Caissa+correction --
  aff75594 pawn term +1.74; 6dd05529 material; 190abaa7 SF-style +20.1/+30.3;
  6e71545 continuation (credits Motor); 39a3b7f gating tweak; 54aef4a
  bad-capture-per-SEE update +2.63 (2026 refinement, not for v1).
- -
  https://github.com/Witek902/Caissa/commit/6dd05529dea17e7a675dc593e4e0ab99965a306e
  -- message only: average error per key, portion subtracted at eval.
- -
  https://github.com/official-stockfish/Stockfish/commit/b4d995d0d910044cf4ea2ad3ee30fd1d21070cd8
  -- the SF introduction message: mechanism, caps, /32 application, STC/LTC.
- https://github.com/official-stockfish/Stockfish/pull/4950 -- PR prose,
  author, date, Caissa credit.
- https://github.com/lynx-chess/Lynx/pull/1662 -- **+11.35 +/-5.16, 7502
  games, 8+0.08** -- the +11.4 the plan cites, banded at **3224-3291** by
  `adocs/data/S181_lynx_bands.md`.
- https://github.com/lynx-chess/Lynx/pull/1663 -- pure pawn key (+3.27 on
  top; +12.09 alone).
- -
  https://api.github.com/search/commits?q=repo:lynx-chess/Lynx+correction+history
  -- the two Lynx shas and dates.
- https://api.github.com/search/commits?q=corrhist+mate -- Amethyst, PiChess,
  Rogatia mate/decisive-band commits, messages only.
- https://github.com/mcthouacbb/Sirius/pull/176 -- minor-piece corrhist
  +7.43 +/-4.78 (the S110-family band, for the reserve file's context).
- https://github.com/zzzzz151/Starzix/pull/98 -- pieces corrhist, merged
  2024-08-05, no prose detail (read and found empty, recorded as such).


## As built (2026-10-13, Opus subagent)

**Rebased onto `22e8df7` on 2026-10-14, written blind -- the last section,
"Rebased onto `22e8df7`", supersedes every reading below that was taken on
`5ac17bd`.** The readings below are that tree's and are kept as its record.

**Phase 1, written blind.** Worktree `/home/max/ws/chesso-s099` at `fe2a28a`
(achesso's head), while S119's SPRT held every core. Under DEC-260 nothing was
compiled, tested or run then. Two diffs, kept apart:
`.tuning/coord/S099/a.diff` (HEAD to part (a)) and `.tuning/coord/S099/b.diff`
(part (a) to part (b)), each checked to apply exactly -- through a temporary
index, no ref moved -- to the tree the other leaves (`make_diffs.sh` and
`check_diffs.sh` beside them). Backups in the main checkout's
`.tuning/coord/S099_files/`.

**Phase 2, part (a) -- built, gated and measured; "Part (a): phase 2" below
has the readings.** Worktree `/home/max/ws/chesso-s099b` at `5ac17bd`, after
S119 (H1), S263, S259 and S276 landed; a.diff applied there with `git apply
--3way --index` without a conflict.

**DEC-271 changed the split: one candidate, one verdict.** (a)'s timing
measured a cost (1.4 to 2.1 % of speed, a branch on the moving piece the likely
cause), and (b)'s SPRT against (a) would never have priced it. So (a) does not
land alone: it was made branchless and lands with (b) as **one commit**, whose
SPRT reference is the commit that lands S261's lane -- the tree before the
key. Everything below that speaks of (a) landing on its own, or of (b)'s SPRT
against (a), is the history of how that changed; "Part (a): branchless" and
"Part (b): phase 2" are the tree that lands, and the commit text at the end is
the one commit's. DEC-270's rulings settle the five choices phase 1 left open
(the census before the SPRT, datagen wired, a 32+0.32 estimate on H0 only, no
bisection booked, two parameters and three definitions).

### Deviations, first

1. **The pawn key is undone by xor on unmake, not copied from the history
   entry.** Section 2 offered both and priced the copy at `history_entry_t` 16
   to 24 bytes; that record's own comment (beside `halfmove_clock` in board_t)
   is why it stays 16 -- a third more history per cache line on the walk
   `classify_repetition()` runs at every node. Four xor sites in
   `unmake_move_impl` instead: the promotion's pawn, the en-passant victim,
   the mover and the captured piece. `board_t` goes 216 to 224 bytes; the
   history does not grow.
2. **The table size is form (c), 32768 a side.** Section 4's census could not
   run before the table was declared -- this phase compiles nothing -- which
   is exactly the case its fallback names. Phase 2 may run the census and
   move the constant before the SPRT; that is the coordinator's call, item 6
   below.
3. **Two of the six constants are UCI parameters, not six.** Section 4 says
   each constant lands in the X-macro, and in the same breath that the scale
   is "not swept independently", the grain "carries no Elo of its own" and
   the clamp moves with the grain so "only the applied maximum is tunable".
   So `CorrHistMax` and `CorrHistWeightCap` are parameters with stated ranges
   (S073), pinned in the tune build's golden list; `CORRHIST_SCALE`,
   `CORRHIST_GRAIN` and the entry count are definitions beside the struct in
   the data-structures header, the shape CONT_HIST_BOUND took at S222 for the
   same gauge reason; the entry clamp is derived at use, `CorrHistMax *
   grain`, so it cannot drift from either.
4. **The bound is held twice, and the observation is clamped before it is
   scaled.** The read is clamped to +/-`CorrHistMax` as well as the entry at
   every update (a tune-build `CorrHistMax` lowered over a warm table cannot
   apply more than it says), and the corrected score to `+/-(MATE_MIN - 1)`
   as section 3 asks. The difference is clamped to +/-`CorrHistMax` before the
   multiply, which is what section 4's int32 arithmetic (`clamp * scale`)
   already assumes, and the weight is held at or under the scale. A
   consequence the step file did not state: with truncating integer
   division, an entry driven by a constant large difference settles one unit
   short of its clamp, so a saturated read is 46 and not 47 at the seeds.
5. **Both stores write the raw evaluation.** Section 3 names negamax's one
   store; ProbCut's store wrote `static_eval` too and would have written the
   corrected number. Both now take `raw_eval`.
6. **Where the table is cleared**: `ucinewgame`, `bench` before each position
   (both through `reset_for_new_game`), `uci_init` (a test binary starts many
   engines), and `test` exactly where it clears the transposition table, so a
   second `test` still reads the first one's total. Not `clean-tt`, whose name
   is the transposition table's, and not `position`.
7. **Outside `touches:`, now listed there.** Part (a) is the board's
   alphabet and its oracles. Part (b) adds the table's own header, the search
   state's pointer and the UCI layer's instance, two probes in the search
   header, the UCI accessor, and **`tools/datagen.cpp`**: each worker carries
   its own table, cleared per game. DEC-258 (4) places this step before S082
   regenerates the corpus "because the correction changes the search that
   plays the corpus games"; without the wiring the corpus would be played by
   the engine before S099. One hunk; the coordinator may take it out.
8. **The forced-mate re-run is a case of its own**, "the correction table
   hides no forced mate", and not an edit of "pruning does not hide a forced
   mate": every case that drives search() on its own state carries no table,
   reads the raw score and searches the tree before S099 node for node, so
   that case is unchanged and runs as it did.
9. **One existing mutant re-anchored**: S234's G04 anchored on the store's
   `static_eval`, which is now `raw_eval`; without the edit
   `tools/mutation_check.py` refuses the whole directory.
10. **CPU disclosed for S119's record.** No build, test, engine, tool or
    timing ran. What did run, each well under a second: `git` plumbing
    (`show`, `apply` into a scratch copy and against a temporary index,
    `hash-object`, `read-tree`, `update-index`, `write-tree`, `diff`) through
    three short scripts in the scratch directory; `sed`, `awk` and `grep` text
    edits; and, by a slip, one `python3 -` with empty input. One misdirected
    write -- a scratch copy landed in the main checkout's `.tuning/coord/`
    when a script ran from the wrong directory -- was removed at once;
    nothing else outside the worktree and the backup directory was written.

### Part (a): the pawn key -- behaviour-neutral, lands on its own

Files: `src/data_structures.hpp`, `src/bitboard.cpp`, `src/bitboard.hpp`,
`tests/test_invariants.cpp`, `tests/test_engine.cpp`,
`tools/mutants/S099_pawn_key.py`, `adocs/specs.md` (INV-2's wording and
row), `DEV_MANUAL.md` (the walk's paragraph).

- `board_t` gains `pawn_hash`, after `hash`: the existing piece-square keys
  of the pawns of both colours, nothing else -- no other piece, no side, no
  castling, no en passant (section 2; the Lynx #1663 record for no kings).
- `add_piece`, `remove_piece` and `move_piece` xor it when the piece is a
  pawn, so it is maintained by the alphabet and never rebuilt (INV-4's rule).
  Where the piece is a compile-time constant at an inlined call the test
  folds away.
- `unmake_move_impl` undoes it by xor at four sites (deviation 1).
- The oracle, `compute_full_pawn_hash()`, is declared in the board header
  beside the two INV-2/INV-4 oracles, not hidden like the main key's: it is
  asserted at make's entry and exit and unmake's exit in Debug, called by
  the tests in every build, and by `load_FEN` once per load.
  `cleanup_board` zeroes the key; a pawnless board keys to 0.
- Nothing reads the key, so the tree cannot move: INV-6 on node counts.

Tests:
- the corpus walk in `test_invariants` (INV-2 in the gate): after every
  make, the key equals its oracle **and** has moved on exactly the moves that
  move or take a pawn (en passant included) and on no other -- which is what
  catches an alphabet and an oracle that agree on the wrong thing. The
  `board_t` memcmp after unmake covers the restore. Floors `> 0` on both
  classes, not goldens; the existing en-passant and promotion floors carry
  the counted classes. The planted-drift case gains two plants for the key.
- `test_engine`'s hash walk compares the key too, and its load check;
  "the pawn key ignores the side to move and castling"; "a line of moves and
  its fen share a pawn key" (the S195 case's second sequence, two pawn
  captures, its legality already judged there).
- Mutants PK01 to PK04 (move_piece's xor dropped, unmake's mover and
  en-passant xors dropped, `load_FEN` not setting the key), each named at the
  case meant to kill it. All are node-identical in this tree, so only the
  walks can see them.

### Part (a): phase 2

On `5ac17bd`, i7-8700K, g++ 13.3, the native padded builds; the reference
binary is the main checkout's `build/src/chesso` at that commit, clean.

- **Compiler.** Release, tune and Debug build with 0 warnings and 0 errors
  at -Wall -Wextra -Werror; nothing the phase-1 text risked happened.
- **Format.** `./clang-format.sh --check` (CLANG_FORMAT_MAJOR=22) wanted
  three reflows, all mine and all whitespace: the move_piece xor, one
  `load_FEN` call in `test_engine`, one FAIL message in `test_invariants`.
  Applied with clang-format-22 itself. PK01's anchor followed the reflowed
  move_piece lines.
- **The gate**, AGENTS.md's TESTS chain at -j12: **45/45** in `build`,
  **45/45** in `build-tune`, format clean, exit 0
  (`.tuning/coord/S099/logs/gate_a.log`). `test_invariants` 1.06 s, and its
  census now reads 2132167 makes, 181 en passants, 221928 promotions, 206212
  capture promotions, **781437 pawn events and 1350730 moves touching no
  pawn** -- both of S099's classes populated.
- **INV-6, node-identical against `5ac17bd`.** `bench` **3872032** on both,
  the whole stream identical line for line but for the nps fields (121
  lines, eight replies `c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6`); `bench
  12` 1730486 on both, its stream identical the same way.
  `tools/search_bench.py` depth 9: 32829 / 69892 / 25174, `c3d5` / `e2a6` /
  `d7c8q`, both sides; depth 12: 68090 / 246712 / 114555, `c3d5` / `d5e6` /
  `d7c8q`, both sides. The Debug binary's `bench` prints 3872032 too, with
  the key's oracle asserted at every make and unmake.
- **The interleaved timing** (section 6, DEC-083, read by DEC-266) is the
  pawn key's only measurement: part (b)'s SPRT is taken against part (a). It
  prices `board_t` 216 to 224 bytes and the pawn branches -- one in each of
  `add_piece`, `remove_piece` and `move_piece` on make, three on unmake plus
  an unconditional xor in the promotion and en-passant blocks. Harness and
  rules are item T below; the coordinator ran it 2026-10-13 10:08:00 to
  10:35:33, `RUN-DONE` (`.tuning/coord/S099/timing/runs/`). **The reading:
  a measured cost, about 1.4 to 2.1 % of speed.**

  *Void checks pass.* Preflight identical on all five workloads; every
  block's node totals equal per side (bench 16 9631170, the 300 positions
  `300 25551090`, the bench_eval checksum, perft 41812668 verified). Load
  about 1.0 on twelve cores from start to end, which is the measurement's
  own core; bench_movegen's resolution 0.0 to 0.2 % at start and end.
  *Drift* (the change half's base cycles median against the A/A's):
  -0.08 % bench 16, -0.07 % the 300 positions, +0.14 % bench_eval, -0.13 %
  movegen, +0.21 % hyperfine's base median -- all well inside 1 %.

  | workload, instrument | A/A floor | change | CI |
  |---|---|---|---|
  | bench 16, cycles | 0.311 % | **+1.961 %** | +1.742 .. +2.181 |
  | bench 16, wall | 0.457 % | +2.017 % | +1.722 .. +2.314 |
  | bench 16, instructions | -- | +0.692 % | exact |
  | 300 positions, cycles | 0.471 % | **+1.438 %** | +1.264 .. +1.612 |
  | 300 positions, wall | 0.732 % | +1.424 % | +1.187 .. +1.663 |
  | 300 positions, instructions | -- | +0.657 % | exact |
  | 300 positions, hyperfine speed | 0.23 % | **-1.42 %** | -1.60 .. -1.25 |
  | movegen, cycles | 1.100 % | +2.183 % | +1.753 .. +2.614 |
  | movegen, perft ms | 1.838 % | +4.288 % | +3.586 .. +4.995 |
  | movegen, instructions | -- | +1.930 % | exact |
  | bench_eval, ns per call | 0.998 % | -0.139 % | -0.507 .. +0.230 |

  *Verdict.* Rule 3 holds: the cycles CI on bench 16 and on the 300
  positions, and hyperfine's wall CI, lie wholly beyond their floors, all
  slower. Rule 4 holds: instructions move the same way, +0.69 % and +0.66 %,
  deterministic, so no layout rebuild is owed -- the cost is the change's
  own. Cycles rise about twice as much as instructions, which points at the
  pawn tests' branches on a runtime piece (and, less, the larger board)
  rather than at the xors themselves. Movegen, the path the change sits on,
  shows it plainest: perft +4.3 %. bench_eval does not move (rule 6), as an
  evaluation part (a) never touches should not.

  *As a conversion, not a verdict:* the speed loss is 1.42 % (the 300
  positions, both instruments) to 2.07 % (bench 16; nps median 4543694
  to 4449482, -2.07 %). At **1.43 Elo per percent of nps**, the long-control
  figure INV-6 and DEC-083 cite (specs.md's INV-6 paragraph; it names 2.10 at
  short controls), that is **about -2.0 to -3.0 Elo**; at 2.10, -3.0 to -4.3.

  *What is unmeasured, plainly:* no SPRT prices this. Part (b)'s
  pre-registration takes REF as part (a)'s landing, so (b)'s verdict says
  what the correction gains on top of the key and nothing about what the key
  cost; the package's net is (b)'s reading less this conversion, and the
  conversion is not a match. Part (a) on its own buys nothing in play. So it
  is a regression the step has to answer before (a) lands, one of two ways --
  the coordinator's or the owner's to choose: **fix it** (a branchless form
  -- the pawn rows of `piece_randoms` copied into a twelve-row table zero
  for every other piece, xored unconditionally, so the alphabet loses its
  three tests and unmake its two -- re-timed by this same harness), or
  **price it** (part (b)'s SPRT taken against `f6647dc`, the tree before
  (a), so its verdict covers the key and the correction together; the
  pre-registration says so before a game).
- **Second tier (DEC-141).** Four rounds of Debug self-play at 4+0.04 with
  `-repeat`, DEV_MANUAL.md's command (`.tuning/coord/S099/selfplay_debug.sh`):
  **8 games**, 0 time forfeits, 0 disconnects, **0 `Assertion`** over a
  127698-line trace log carrying 1000 `bestmove` lines.
  `tools/gate_extra.sh`, all five stages, JOBS=12: prose ok, citations ok,
  **debug ok** (354 s: the six Debug binaries with the key's oracle asserted
  at every make and unmake), perft ok (51 s) -- and **sanitize FAILED, on a
  defect this part did not make**: `test_tuner_gradient` and
  `test_eval_trace` stop at UBSan's "store to null pointer of type 'struct
  record_t *'" on `eval_trace::sink = &record;` in `tools/tuner_trace.hpp`
  `trace_board` (S262's trace, S276's reader). The same stage on a clean
  `5ac17bd` worktree fails the same two tests on the same line, 43 of 45
  passing on both (`.tuning/coord/S099/logs/gate_extra_ref5ac/`). Because the
  stage's ctest failed, its sanitizer-bench comparison never ran; run by hand,
  the sanitizer build's `bench` prints **3872032**, the Release total, with
  no sanitizer report. Not reachable in play (DEC-171), and reported to the
  coordinator rather than fixed here: it is S262's code, outside this step.
- **Mutants**: `tools/mutation_check.py tools/mutants/S099_pawn_key.py`, on
  a fixture -- a commit object on no branch holding exactly part (a) on
  `5ac17bd`, its worktree removed after -- **4 of 4 killed**, each with the
  bench signature unchanged, as a key nothing reads must leave it. PK01 by
  both walks and the line-of-moves case; PK02 and PK04 by both walks'
  board compares (PK04 at an en-passant capture of the corpus walk); PK03 by
  the load checks, the line-of-moves case, the corpus walk and the
  planted-drift case. 605 s, `.tuning/coord/S099/logs/mutation/results.tsv`.
- **Prose checks** on this tree, with the step file in `plan_current/`:
  `tools/plan_prose_check.py` `--citations` (0 flagged over 32 files; this
  file 23 code citations, 0 flagged), `--touches` (0 flagged), `--params`
  (exit 0).

### Part (a): branchless (DEC-271)

The xor sites stop asking which piece moved. `zobrist_randoms_t` gains
`pawn_randoms[12][64]`: the two pawn rows of `piece_randoms` copied, the other
ten zero, filled by `init_zobrist()` after the 851 draws so their order -- and
every key -- is untouched. `add_piece`, `remove_piece` and `move_piece` xor
the pawn key from it for every piece; unmake's mover and captured-piece sites
do the same, and its promotion and en-passant sites, pawns by construction,
keep their unconditional xor. The oracle, `compute_full_pawn_hash()`, still
reads `piece_randoms` and the pawn bitboards, so it checks the derived table
as well as the alphabet; the Debug asserts and INV-4's rule are as they were.
6 KiB more per `game_t`.

- **INV-6 against `f6647dc`** (the main checkout's clean build, src/
  identical to `5ac17bd`): `bench` 3872032 with the stream identical line for
  line but nps (121 lines), `bench 12` 1730486 (105 lines);
  `tools/search_bench.py` depth 9 32829 / 69892 / 25174 and depth 12 68090 /
  246712 / 114555, best moves `c3d5 e2a6 d7c8q` and `c3d5 d5e6 d7c8q`, both
  sides.
- **Mutants, 5 of 5 killed** (`tools/mutation_check.py
  tools/mutants/S099_pawn_key.py` on a fixture commit object holding exactly
  branchless (a)): PK01 and PK02 re-anchored on the branchless lines, and
  **PK05 new** -- the derived table carrying the white knights' row -- killed
  by both walks. Each with the bench signature unchanged, as a key nothing
  reads must leave it. 753 s, `.tuning/coord/S099/logs/mutation_pk2/`.
- **Did it take the cost away?** Prepared, not run -- S277's agent was
  building on the machine: `.tuning/coord/S099/timing/run_s099a2.sh`, a
  counters comparison (not a DEC-266 verdict) of branchless (a) against the
  parent and against (a) with the branch, on bench 16 and on bench_movegen's
  perft, six interleaved pairs each, about 6 minutes; node counts on all three
  binaries verified first (dry run: bench 16 9631170 and perft 41812668
  verified on each). Launch from the timing directory: `nohup
  ./run_s099a2.sh > run_s099a2.log 2>&1 & echo $! > run_s099a2.pid`; read
  `runs2/*.txt`, instructions exact and cycles indicative. Whatever cost is
  left, the SPRT prices it (DEC-271).

### Part (b): the correction -- the SPRT's candidate

Files: `src/correction_history.hpp` (new), `src/data_structures.hpp`,
`src/search.cpp`, `src/search.hpp`, `src/search_params.hpp`,
`src/chesso.cpp`, `src/uci.hpp`, `tools/datagen.cpp`,
`tests/test_search.cpp`, `tests/test_evaluation.cpp`,
`tests/test_engine.cpp`, `tests/test_search_params.cpp`,
`tools/mutants/S099_correction_history.py`,
`tools/mutants/S234_tt_estimate_margins.py`, `MANUAL.md`, `DEV_MANUAL.md`,
`adocs/specs.md`, `adocs/data/S099_sprt.sh`.

Design, from section 2 and 3's description (DEC-221; no engine's code opened):
- **The table** is one struct parameterised by its size, int32 per (side to
  move, key slot), the slot the key's low bits; `pawn_correction_t` is its
  one instance, 256 KiB as written blind and 16 KiB since the census
  ("Part (b): phase 2"). The new header holds the read, the update and the
  clear, so the search, the UCI layer, datagen and the tests share one slot
  rule and one fixed point.
- **The read**, once, at S108's compute-or-read site: the node keeps its raw
  `evaluate()` as `raw_eval` and the consumers' number becomes `static_eval`
  = raw plus the slot over the grain (at most `CorrHistMax` either way), held
  inside `+/-(MATE_MIN - 1)`. The improving stack, the futility estimate,
  reverse futility and razoring read it; both stores and the update read the
  raw one. A state with no table reads the raw score -- every test that
  drives search() itself, and the tree before S099.
- **The update**, beside the one store and under its `excluded_move == 0`
  guard, on the gate section 3 lists: not in check and a raw score present;
  best move absent or not a capture (a quiet promotion counts as quiet, as it
  does for the history tables); the search's own score, never the
  ply-normalised one, outside the mate band; a lower bound only at or above
  the raw score, an upper bound only at or below it, an exact score always.
  The step is the published moving average, `(entry * (256 - w) +
  observation * w) / 256`, `w = min((depth + 1)^2, CorrHistWeightCap)` at the
  node's remaining depth, the entry clamped to `CorrHistMax * grain`.
- **Ownership**: the UCI layer's static instance, attached to every `go`'s
  search state beside the mate line, cleared as deviation 6 says; datagen's
  per worker (deviation 7). Quiescence is untouched, as section 2 says: its
  stand pat reads the raw evaluation, from the entry or `evaluate_lazy()`.

Seeds (DEC-134 forms, each argued at its site):

| constant | value | form |
|---|---|---|
| `CorrHistMax` | 47, range 0 to 94 | (c), midpoint of 0 to one pawn (PAWN, 94); 0 is off |
| `CorrHistWeightCap` | 144, range 1 to 256 | (b), `(CONT_HIST_REF_DEPTH + 1)^2` |
| `CORRHIST_SCALE` | 256 | (b), the power of two at or above the cap |
| `CORRHIST_GRAIN` | 256 | (c), geometric midpoint of 64 to 1024 |
| entry clamp | 12032 | (b), `CorrHistMax * grain`, derived at use |
| slots a side | 32768, **2048 since the census** | (c), midpoint exponent of 2^10 to 2^20 (deviation 2); (b) since phase 2, the census DEC-270 ordered |

Nothing in part (b) is seeded from a value S261's lane fits: S261 excludes
the axes S099 adds (S127 fits them), and the seeds rest on PAWN and
CONT_HIST_REF_DEPTH, which no search lane moves. What S261 does move is the
margins the corrected number feeds, fitted without a correction; section 7
has them re-fitted after this lands.

Tests, all unrun -- in `tests/test_search.cpp`'s new suite "search: pawn
correction history" unless named elsewhere:
- **The accepts' bound**: "a corrected score never reaches the mate band" --
  raw scores up to and past the band's edge under the largest entry either
  way and under entries no update can write, the result strictly inside the
  band, and away from the edge exactly the clamped read. Precondition: at a
  raw score ten under the edge the planted correction alone would cross it.
- "a search state with no table reads the raw score".
- **Sign and sides**: "a side's observation moves its own slot and no
  other".
- "the update is the capped moving average and stays clamped": one shallow
  step, one capped step, written out from the form; a thousand unbounded
  steps either sign stay inside the clamp and saturate.
- **The gate**: "the gate refuses each published case and nothing else",
  each refusal beside the twin it refuses, through the exported probe.
- **The site against the gate**: "the search teaches the table exactly where
  the gate does" -- every third test position at depth 1 on a PV node (whose
  children are quiescence, so the node is the table's only writer), three
  windows so all three bound types are stored, a plant of +/-10 units in the
  node's slot so a site that learned from or stored the corrected number
  disagrees with the reference; floors `> 0` for a moved slot and for the
  in-check, capture and bound-direction refusals.
- **At the consumer and nowhere else**: "a correction moves reverse futility
  and not the store" -- S234's drive position, a planted +/-47 moving the
  cutoff both ways, each with the no-table precondition first; the stack
  holds the corrected number and the entry the raw one.
- In the guard suite, beside S113's: "a probcut store keeps the raw static
  evaluation".
- **The mate cases re-run**: "the correction table hides no forced mate" --
  the two queen mates in two and S109's mate by a hanging quiet, depths 3
  to 6, with a cold table and with every slot planted at +47 and at -47.
  **If a planted row goes red, that is a finding for the owner, not a row
  to relax.** It is the case I am least sure of blind.
- **INV-5**, in `tests/test_evaluation.cpp`: "a mirrored position reads the
  same corrected score" over every test position and its mirror.
- **Cleared on `ucinewgame`**, in `tests/test_engine.cpp`: "ucinewgame clears
  the pawn correction table", with uci_init's clear and a learning search as
  preconditions. "node-limited searches repeat across ucinewgame" already
  goes red if the clear is lost.
- `tests/test_search_params.cpp`: two golden rows (73 to 75), and "the
  correction's ranges are the ones their purpose declares" -- the top of
  `CorrHistMax` is PAWN, the top of `CorrHistWeightCap` is the scale, and the
  update's worst sum at the range tops sits inside int32.

**The mutant the bound test kills is CH01**, the clamp to the mate band's edge
dropped (CH02, the read clamp, with it). `tools/mutants/S099_correction_history.py`
has fourteen, CH01 to CH14 -- the two clamps, the four gate clauses, both
stores writing the corrected number, the side read flipped and negated for
Black, the difference negated and taken from the corrected number, the weight
uncapped, `ucinewgame` keeping the table -- each named at the case meant to
kill it. CH06 (the in-check clause) is equivalent in the engine, since every
in-check node carries the sentinel the gate also refuses, and is killed by
the gate case; its header says why it is still `killed`.

Docs: `MANUAL.md` -- two option rows; `RfpTtEstimate`'s and `RazorMargin`'s
rows say the static evaluation is the corrected one and the entry the raw
one; `ucinewgame` and `bench` clear the table. `adocs/specs.md` -- the search
row's feature list and one paragraph for the correction, its three stale
"raw static evaluation" phrases, and INV-5's row. `DEV_MANUAL.md` -- datagen
plays with a table since S099. No new UCI option in the release build and no
`info` field, so `test_uci_surface` needs nothing; the tune build's two new
options are documented in MANUAL.md's table, which that surface test
requires.

### Part (b): phase 2 (DEC-271)

On `5ac17bd` with branchless (a), the s099b worktree. `b.diff` applied on top
with every hunk clean but `adocs/specs.md`'s, whose context (a)'s INV-2 edit
had moved; that hunk was re-applied by hand with the same text
(`.tuning/coord/S099/specs_b.sed`, and INV-5's row). Readings on the tree that
lands; the step's numbers before this section are phase 1's and (a)'s.

- **Compiler.** Release, tune and Debug: 0 warnings, 0 errors at -Wall
  -Wextra -Werror. **Format**: clang-format-22 wanted reflows in the new
  tests and `src/search.cpp`, whitespace only, applied with it.
- **The census, DEC-270 (1), and the size it chose.** A tune-build
  instrument in `src/search.cpp` (`pawn_key_census_t`, off unless
  `CHESSO_PAWN_KEY_CENSUS` names a file; the UCI layer's new-game reset
  writes one line per game, `src/chesso.cpp`), driven by
  `adocs/data/S099_pawn_key_census.py`: the 300 timing positions, one cold
  `go depth 12` each, the distinct pawn keys a correction was read for, per
  side to move. At 32768 slots, the larger side position by position: p50
  273, p90 1159, **p95 1808**, p99 3699, max 4863. **2048 slots a side**,
  the next power of two above the p95: `CORRHIST_PAWN_ENTRIES = size_t{1} <<
  11`, form (b), 16 KiB for both sides where 32768 was 256 KiB. Re-taken at
  2048, since the size moves the tree the census reads: p95 1684, 2048 again.
  Node counts, so the load moved none of it.
  *A finding about the rule, not acted on.* Section 4 says sizing at the
  95th percentile "keeps collisions rare". Under uniform hashing 1808 keys in
  2048 slots leave about 59 % of a p95 search's keys sharing a slot with
  another (1 - e^(-1807/2048)), and about 12 % at the median's 273: computed,
  not measured. DEC-270 ruled the census's rule and the size is S127's to
  sweep, so the size is the rule's answer and the sentence in section 4 is
  what is wrong.
- **What the tree does.** `bench` 3872032 -> **3539313** (-8.6 %), the eight
  replies unchanged (`c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6`); at 32768
  slots it read 3386911. `bench 12` 1730486 -> 1564376.
  `tools/search_bench.py` depth 9: 32829 / 69892 / 25174 -> 23699 / 69126 /
  24255; depth 12: 68090 / 246712 / 114555 -> 52513 / 293466 / 89877; best
  moves `c3d5 e2a6 d7c8q` and `c3d5 d5e6 d7c8q`, unchanged. The Debug
  binary's `bench` prints 3539313 too.
- **Off is off (DEC-215).** The tune build at `setoption name CorrHistMax
  value 0` prints the parent's `bench` stream, 3872032, its 121 lines
  identical but for nps.
- **The gate**, AGENTS.md's TESTS chain at -j12. The first run read 43/45 in
  `build` (`.tuning/coord/S099/logs/gate_ab.log`): `test_search` and
  `test_mate_carry`. `test_search`'s red was two faults in my own new case,
  "the search teaches the table exactly where the gate does", and none in
  the engine: it asked the node's stack slot for the corrected score before
  asking whether the node stored -- a node that returns on a draw by rule
  does so before it writes the slot, which then holds an earlier drive's
  number -- and it asked the entry for the raw score in check, where there is
  no static score and S108's rule keeps the evaluation an earlier quiescence
  stored. Both checks are now asked of a node that stored, out of check; the
  case's floors and its reference comparison are unchanged.
  `test_mate_carry`'s was the budgets and one ceiling, below. After: **45/45
  in `build`, 45/45 in `build-tune`**, format clean, exit 0
  (`gate_ab2.log`). "the correction table hides no forced mate", the case
  phase 1 was least sure of, is green with every slot planted at +47 and at
  -47.
- **Goldens, all four re-derived by their scripts (DEC-142)**, on a commit
  object on no branch holding the tree (`3bc2ed7`, src/ identical to the
  tree that lands; `.tuning/coord/S099/full_fixture_commit.sh`), never read
  off a run:
  - `test_engine`'s first-mate depths,
    `adocs/data/S188_repair_goldens.py first-mate`: **10 and 9, unchanged**,
    mate 5 from there on.
  - `MATE_IN_THREE_FLOOR`, `adocs/data/S154_floor_margin_sweep.py floor` and
    `red`: shipping 13, weakened (RfpMinPly 1) 8, where S250 read 12 and 7;
    the gate built at 1 red at 11. **The floor stays 11**: it still separates,
    S250's and S116's precedent. Comment updated;
    `adocs/data/S099_floor_sweep.log`.
  - `test_mate_breadth`'s `EXACT_FLOOR`, `adocs/data/S156_mined_floor_sweep.py
    --floor 145`: 149 exact at RfpMinPly 3 and 2, 144 at 1 and 0, the gate
    built at 1 red at 145. **The floor stays 145**, one above the weakened
    end. A paragraph added; `adocs/data/S099_mined_floor.log`.
  - `test_mate_carry`, `adocs/data/S203_case_sweep.sh`, the 108-cell grid
    (`adocs/data/S099_sweep_s170.txt`, six cases swept in parallel, one
    process each -- node-limited, so the parallelism moves no count). Rule
    stated before the grid was read: the cheapest budget at the row's own
    stride whose cell reports a mate line. All three old cells of C, D and E
    had gone silent -- the majority of four failed at three -- and move: **C
    1500000 -> 300000 (12 lines), D 2000000 -> 3000000 (4), E 100000 ->
    1000000 (15)**, none short; A, B and F unchanged. `--ceilings` over
    S202's grid and this one: 0, 5, 4, 0, 0, **2**. **F's ceiling rises 0 ->
    2** -- its own cell, 100000 at stride 2, reads 8 lines with 2 short, and
    300000 reads 2 of 2. Raising a ceiling relaxes a test, so **this needs a
    decision before it lands**, as S116's did (DEC-250). The tree carries
    the script's answer so the gate and the mutants run on the proposal;
    `tests/test_mate_carry.cpp` names it DEC-272 at the one paragraph that
    cites it (a placeholder until the owner ruled, 2026-10-13: F rises to 2
    with S099, on its H1 only). Had it been refused, the line would have gone
    back to 0 and `test_mate_carry` been red on F in both builds. `unreached.empty()`, the
    guarantee, holds on all six cases at their own budgets. The note in
    `adocs/data/S170_cases.tsv` and the grid list in the test's comment
    updated; the golden comment's list of six had been stale since S202 and
    now reads the shipped numbers.
- **Mutants, CH01 to CH14: 14 of 14 killed, each by the case its
  `Mutation:` line names** (`tools/mutation_check.py
  tools/mutants/S099_correction_history.py` on a fixture -- a commit object
  on no branch holding the whole tree, `e1d06ef`, its worktree removed after
  -- 1714 s, `.tuning/coord/S099/logs/mutation_ch/results.tsv`). The bench
  signature moved on eleven; it held on CH01 and CH02, the two clamps, which
  no bench position reaches (no raw score comes near the mate band, and the
  update keeps every entry inside the read's clamp), and on CH06, the
  in-check clause, equivalent in the engine as its header argues -- all three
  killed by their named cases. `test_mate_carry` killed nine of them as
  well. The first launch refused at the baseline, `test_clang_format_script`
  red because the detached run had not inherited DEC-146's
  `CLANG_FORMAT_MAJOR=22` -- the environment, not the tree; re-launched with
  it exported (`mutation_ch_refused_env/`). PK01 to PK05 were not re-run on
  the whole tree: they ran on branchless (a), 5 of 5 ("Part (a):
  branchless"), and (b) touches none of their lines.
- **Second tier on (a) and (b) together (DEC-141).** Four rounds of Debug
  self-play at 4+0.04 with `-repeat` (`.tuning/coord/S099/selfplay_debug.sh`,
  the Debug build of this tree, 0 warnings): **8 games**, all ended normally,
  0 time forfeits, 0 disconnects, **0 `Assertion`** over a 132027-line trace
  log carrying 1068 `bestmove` lines. `tools/gate_extra.sh`, all five stages,
  JOBS=12 (`.tuning/coord/S099/logs/gate_extra/`): prose ok, citations ok,
  **debug ok** (353 s), perft ok (59 s), and **sanitize FAILED on S278 and
  nothing else**: `test_tuner_gradient` and `test_eval_trace`, UBSan's
  "store to null pointer of type 'struct record_t *'" on `eval_trace::sink =
  &record;` in `tools/tuner_trace.hpp` `trace_board`, the same two tests and
  the same statement a clean
  `5ac17bd` fails on (part (a)'s reading), 43 of 45 passing; the file is
  untouched by this step. The stage's sanitizer-bench comparison never ran
  behind its red ctest; run by hand, the sanitizer build's `bench` prints
  **3539313**, the Release total, with no sanitizer report.
- **Prose checks**, on the final text: `tools/plan_prose_check.py
  --citations` (0 flagged over 32 files; this file 25 code citations, 0
  flagged), `--touches` (0 flagged), `--params` (exit 0).
- **Counters for branchless (a)**, prepared and not run (S277's agent was
  building): "Part (a): branchless" has the command. Whatever (a) still
  costs, the SPRT prices it with (b) (DEC-271).

### What I expect phase 2's compiler and tests to say

Compiler, per file, at -Wall -Wextra -Werror in both builds: no errors and no
warnings expected. The risks I can name: the range-for over an initializer
list in `compute_full_pawn_hash` (`<initializer_list>` reaches it through the
headers); template argument deduction from a non-const table pointer to the
read's const parameter (a qualification conversion, allowed); a missed
backslash in the X-macro block (padded by script, every line checked at 80).
`clang-format.sh --check` is likely to want reflowing in the new tests --
long titles, a braced move list -- which phase 2 fixes and does not argue.

Suites: part (a) -- green, node-identical; `test_invariants` a little slower
(one 16-pawn oracle per make over its two-million-make walk). Part (b) --
direct-search tests unchanged by construction; the new cases green on the
arithmetic above; **UCI-path goldens may move and are re-derived, never
re-read** (DEC-142): `test_engine`'s first-mate depths
(`adocs/data/S188_repair_goldens.py first-mate`), `MATE_IN_THREE_FLOOR`
(`adocs/data/S154_floor_margin_sweep.py floor`, and `red`),
`test_mate_breadth`'s 145 (`adocs/data/S156_mined_floor_sweep.py --floor
145`), `test_mate_carry`'s ceilings (`adocs/data/S203_case_sweep.sh
--ceilings`). The UCI path's tree moved, so all four are re-derived whether
or not they go red.

### Pending after DEC-271 -- what is left before the SPRT

1. **The decision on F's ceiling**, 0 -> 2 ("Part (b): phase 2", the
   goldens). **Settled: DEC-272**, the owner's, 2026-10-13 -- F rises as the
   script answers, landing with S099 only on its H1; the token in
   `tests/test_mate_carry.cpp` is DEC-272. Since the rebase the 2 is not
   carried: the script answers again on S099's new grid (the rebase section
   at the end).
2. **The counters comparison of branchless (a)**, the coordinator's to
   launch when the machine is quiet ("Part (a): branchless" has the
   command). It informs; it decides nothing (DEC-271: the SPRT prices it).
3. **Rebase onto the commit that lands S261's lane**: expect textual
   conflicts in `src/search_params.hpp`, `tests/test_search_params.cpp` and
   `MANUAL.md` where S261 moves values. Then re-take on that tree, in this
   order: the gate in both builds; `bench`, `bench 12` and
   `tools/search_bench.py` 9 and 12; the census (the size moves only if the
   rule's answer does); the four goldens by their scripts, `test_mate_carry`'s
   grid as a new file beside this one; Debug self-play; the commit's `Bench:`
   line; `adocs/data/S099_sprt.sh`'s "what the tree does" numbers.
4. **Pin the pair** in `adocs/data/S099_sprt.sh`: REF the commit that lands
   S261's lane, CAND S099 landed on it, both committed. Re-read the open
   DEC-171 findings (S278 today) and the harness A/A rule (DEC-143) on the
   day. Then the SPRT, a night (DEC-155).

### Pending, phase 2 -- as it stood before DEC-271 (history)

Part (a)'s share is done -- items 2 and 3, PK01 to PK04 of item 5, item 8,
and item 2's interleaved timing as item T -- and read in "Part (a): phase
2" above. **The timing read a measured cost, so before (a) lands the step
has to fix it or price it** (that section's last paragraph has the two
ways). What is left after that is part (b)'s, on the tree part (a) lands
on, with DEC-270's census in item 6 no longer optional.

**T. Part (a)'s timing, DEC-083 read by DEC-266 -- run by the coordinator
2026-10-13 10:08 to 10:35, `RUN-DONE`, read in "Part (a): phase 2".** S263's
harness copied (never edited in place) into the
s099b worktree's `.tuning/coord/S099/timing/` and repointed: `run_s099a.sh`,
`counters.sh`, `counter_pairs.py`, `timing.sh`, `pairs.py`, `workload.py`,
and `build_variant.sh` with `dummy.hpp` for rule 7. Parent: a clean shared
clone of `f6647dc` (src/ identical to `5ac17bd`) in `parent_src/`, built in
`parent_build/`; candidate: s099b's `build/`. Both Release, native, padded,
frozen in `bin/` with `perf_counters`. Both print `bench` 3872032, `bench
16` 9631170, bench_eval checksum -27425414745967 and a verified perft total
of 41812668 (checked while preparing). **New against S263: a paired
`tests/bench_movegen` workload** (generate + make_move + unmake_move, five
timed sweeps a run), read under the counters and by its own perft figure.
The script verifies every node count on both sides before it times anything
and stops `RUN-FAILED` on a mismatch (CLAUDE.md rule 1), refuses to
overwrite an earlier `runs/`, records the load at start and end, and prints
`RUN-DONE` or `RUN-FAILED` last on every exit path. About **30 minutes**:
S263's set took 18 and the movegen pairs add about 10.

**The first launch died in its first second** (2026-10-13 10:04:44,
`runs_dead_1004/`), on two harness bugs: under `pipefail`, `ps | sort |
head` in the machine record failed when `head` closed the pipe, and the
exit trap's `RUN-FAILED` went into `machine.txt`, the redirection in force,
so the log carried no marker. Fixed: no `pipefail` (every number the run
depends on is checked by value), the diagnostic pipes made immune to being
cut short, and both markers written to fd 3, the log saved before any
redirection. Checked in `harness_tests/`: a failure forced inside the
machine-record group (marker in the log, none in `machine.txt`), a forced
void in the preflight, a SIGTERM, and `DRY_RUN=1` on the real path up to the
first timed pair (preflight identical on all five workloads, `RUN-DONE (dry
run, nothing timed)`). `RUNS=<dir>` writes elsewhere; neither variable is
set for the real run.

Launch, detached:

    cd /home/max/ws/chesso-s099b/.tuning/coord/S099/timing
    nohup ./run_s099a.sh > run_s099a.log 2>&1 & echo $! > run_s099a.pid

Watcher (Monitor, persistent), four exits, ceiling 5400 s, three times the
run:

    bash -c '
      end=$(($(date +%s)+5400))
      until grep -qE "RUN-(DONE|FAILED)" "$1"; do
        kill -0 "$2" 2>/dev/null || exit 3
        [ "$(date +%s)" -ge "$end" ] && exit 124
        sleep 30
      done' _ /home/max/ws/chesso-s099b/.tuning/coord/S099/timing/run_s099a.log \
      "$(cat /home/max/ws/chesso-s099b/.tuning/coord/S099/timing/run_s099a.pid)"

Read, from `runs/`:

    cd /home/max/ws/chesso-s099b/.tuning/coord/S099/timing/runs
    cat machine.txt preflight.txt
    tail -3 movegen_start.txt movegen_end.txt
    for f in aa_c_bench16 aa_c_pos11 aa_c_beval aa_c_movegen aa_h \
             s099a_c_bench16 s099a_c_pos11 s099a_c_beval s099a_c_movegen s099a_h; do
      echo "== $f"; tail -7 $f.txt | cut -c1-220; done

How to read it (DEC-266, S263's rules with this step's workloads):

1. **Void checks first.** `preflight.txt` equal on both sides for all five
   workloads (the script enforces it); every counters block's node totals
   equal per side (bench 16 `9631170`, the 300 positions' `300 <n>` total,
   bench_eval checksums, `perft nodes 41812668 verified`). `machine.txt`'s
   load and top processes at start, its end line, and `bench_movegen -r 40`'s
   resolution at start and end (rule 4). **Drift:** the s099a half's base
   cycles median against the A/A's base median, per workload; past about
   1 %, the machine changed between halves -- re-take the whole run.
2. **The floor**, per workload and instrument: the A/A's largest CI bound
   in absolute value -- `aa_c_bench16` and `aa_c_pos11` cycles and wall_ms,
   `aa_c_movegen` cycles and `perft_ms`, `aa_c_beval` ns_per_call, `aa_h`'s
   speed-up CI.
3. **Counts only if** the s099a cycles CI on bench 16 **and** on the 300
   positions, and the hyperfine wall CI, all lie wholly beyond their floors
   in the same direction. Movegen and bench_eval are reported beside the
   verdict, never as it: movegen is the path the change sits on and will
   show its cost most plainly, the search workloads are what a game pays.
4. **Credited** if instructions move the same way. They are deterministic,
   so a cost read in cycles with instructions up is the change's, and needs
   no layout rebuilds. Expect instructions up: the branches and xors are new
   work.
5. **Anything else is zero**, and DEC-266's consequence applies: a cost
   under about 0.5 % reads zero here. A zero is recorded as zero; part (a)
   lands on its reason either way (rule 8) -- S099's table and S118's cache
   need the key.
6. **A cost beyond the floor** on both search workloads is converted at
   DEC-083's 1.43 Elo per percent of nps, named as a conversion and never as
   a verdict, and it goes in the landing commit's text.
7. **Only if cycles move against instructions** (cycles up beyond the floor
   with instructions flat or down) is it layout: DEC-266 (4)'s two
   perturbations on both sides, `build_variant.sh parent|cand NAME
   "-falign-functions=64"` and `... NAME2 "-include
   /home/max/ws/chesso-s099b/.tuning/coord/S099/timing/dummy.hpp"`, and the
   counters re-run on those binaries.

1. Rebase onto the tree S261's lane leaves; expect textual conflicts in
   `src/search_params.hpp`, `tests/test_search_params.cpp` and `MANUAL.md`
   where S261 moves values.
2. Part (a) alone: build both, the TESTS chain, `./clang-format.sh --check`;
   INV-6 -- `bench` and `tools/search_bench.py` at depths 9 and 12 identical
   to the parent; one interleaved timing read by DEC-266 (board_t 216 to 224
   bytes, three xor sites), converted at DEC-083's rate and named as a
   conversion.
3. Part (a)'s second tier (DEC-141, it touches `make_move`): four rounds of
   Debug self-play at 4+0.04 grepped for `Assertion` and `disconnect`, the
   commands in DEV_MANUAL.md's "The Debug self-play habit"; then
   `nohup tools/gate_extra.sh > .tuning/gate_extra_<date>.log 2>&1 &`, all
   five stages, watched to its marker (DEV_MANUAL.md "The extra gate").
   The suite chain both times is AGENTS.md's TESTS command:
   `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`.
4. Part (b): build both, the TESTS chain, format; the new `bench` total;
   `bench` in the tune build at `CorrHistMax` 0 equals the parent's stream
   (DEC-215); `tools/search_bench.py` both sides; the four goldens above.
5. Mutants: `python3 tools/mutation_check.py tools/mutants .ref-builds/mut
   --only <id>` per PK and CH id, on a linked worktree made as the tool's
   docstring says, every one killed by its named case; a survivor gets a
   case, never an `equivalent` without the argument. The bound test's own
   mutant is CH01.
6. Optional, the coordinator's call: section 4's census -- distinct pawn keys
   a `go depth 12` visits over S021's 300 positions, instrumented under
   CHESSO_TUNE only -- and move the size to the next power of two above the
   95th percentile before the SPRT if it says so.
7. Part (b)'s second tier: Debug self-play, `tools/gate_extra.sh`.
8. `tools/plan_prose_check.py --citations`, `--touches`, `--params`, one mode
   a call, on both trees.
9. Fill `adocs/data/S099_sprt.sh`'s PHASE2 tokens and pin REF and CAND.

### Proposed `done:` stamp

One verdict for the key and the correction together (DEC-271):

    SPRT <H1|H0|no verdict> at {0, 5} nElo, 8+0.08, Hash 16, CAND <sha>
    (the pawn key and the correction, one commit) vs REF <sha> (S261's
    landing, the tree before the key): <Elo>, <nElo>, LLR <l>, <n> games,
    <f> forfeits (adocs/data/S099_sprt.log); <kept | the commit reverted
    whole, S110 and S111 to the reserve>. The key branchless and
    node-identical on its own (INV-6); the table 2048 slots a side by
    DEC-270's census. Fast suite 45/45 in both builds, format clean; Debug
    self-play four rounds, 0 Assertion; gate_extra prose, citations, debug
    and perft ok, sanitize red only on S278's two tests, sanitizer bench
    equal to Release; mutants 19 of 19 killed, PK01-05 on the key and
    CH01-14 on the whole tree.
    Goldens re-derived by their scripts: first-mate depths unchanged,
    MATE_IN_THREE_FLOOR 11 and the mined floor 145 kept, test_mate_carry's
    budgets C, D and E moved and F's ceiling 0 -> 2 by DEC-272.
    Implemented from the step file's description and the publications it
    cites (DEC-221).

The stamp before DEC-271, kept as the record of the split it replaced:

    SPRT <H1|H0|no verdict> at {0, 5} nElo, 8+0.08, Hash 16, against <REF>
    (part (a)'s landing): <Elo>, <nElo>, LLR <l>, <n> games, <f> forfeits
    (adocs/data/S099_sprt.log). Part (a), the pawn key, node-identical (INV-6)
    at <timing>; part (b) <kept | reverted, S110 and S111 to the reserve>.
    Fast suite <n>/<n> in both builds, format clean; Debug self-play four
    rounds, 0 Assertion; gate_extra <result>; mutants PK01-04 and CH01-14
    killed. Goldens re-derived: <list>. Implemented from the step file's
    description and the publications it cites (DEC-221).

### Commit text, DEC-271: one commit for the key and the correction

The candidate the SPRT plays as CAND; the verdict's record commit carries
DEC-220's block. The numbers are this tree's on `5ac17bd` and are re-taken on
S261's landing before it is committed (pending item 3).

    Correct the static evaluation by a pawn-keyed history (S099)

    The static score errs the same way, again and again, in a given pawn
    structure. The search now keeps, per side to move and pawn key, a
    running average of how far its own score landed from the raw
    evaluation, and adds it, at most CorrHistMax (47) either way, to the
    static evaluation every main-search node reads. Taught beside the node's one
    store: out of check, no capture best move, no mate score, a bound
    only in its own direction. Both stores keep the raw evaluation, so
    nothing compounds, and the corrected score never reaches the mate
    band. Quiescence is untouched.

    The key is board_t::pawn_hash, the pawn piece-square keys alone, kept
    by add_piece, remove_piece and move_piece and undone by xor in unmake
    from a twelve-row table whose non-pawn rows are zero, so no site asks
    which piece moved: the first form's branch cost 1.4 to 2.1 % of speed
    (DEC-271). compute_full_pawn_hash() is its oracle, walked by the
    invariant tests (INV-2, INV-4). Alone it is node-identical to the
    parent (INV-6: bench 3872032 and bench 12 1730486 with identical
    streams, tools/search_bench.py at 9 and 12). It lands with the
    correction because on its own it buys nothing, so the SPRT prices
    both against the tree before the key.

    The table is 2048 slots a side, the next power of two above the 95th
    percentile of the distinct pawn keys a depth-12 search reads
    (DEC-270's census, adocs/data/S099_pawn_key_census.py). The UCI
    layer owns it and datagen keeps one per worker; ucinewgame clears it.
    A mirrored position reads the same corrected score (INV-5), and the
    tune build at CorrHistMax 0 prints the parent's bench stream
    (DEC-215).

    Goldens re-derived by their scripts (DEC-142): the first-mate depths
    unchanged; MATE_IN_THREE_FLOOR's ends 13 and 8 and the mined set's
    149 and 144, both floors kept; test_mate_carry's budgets for C, D and
    E re-swept and F's short-line ceiling 0 -> 2 (DEC-272).
    adocs/data/S099_sprt.sh pre-registers the {0, 5} SPRT, one verdict
    for the key and the correction together.

    Bench: 3539313

### Commit texts before DEC-271 (history: the split it replaced)

Part (a), `a.diff`:

    Maintain a pawn-structure key beside the hash (S099)

    S099's correction table is keyed on the pawn structure, and S118's pawn
    cache will be. board_t gains pawn_hash: the pawn piece-square keys
    alone, with no side, castling or en passant. add_piece, remove_piece
    and move_piece keep it, so it is never rebuilt (INV-4's rule), and
    unmake undoes it by xor because history_entry_t stays 16 bytes.
    compute_full_pawn_hash() is the oracle, asserted in make and unmake in
    Debug and walked by test_invariants and test_engine in every build,
    which also require the key to move on every pawn event and no other
    (INV-2).

    Nothing reads the key yet. Node-identical (INV-6) against 5ac17bd:
    bench 3872032 and bench 12 1730486 with identical streams, and
    tools/search_bench.py at depths 9 and 12 with identical counts and
    best moves. Debug self-play, four rounds at 4+0.04: 8 games, 0
    Assertion. tools/gate_extra.sh: prose, citations, debug and perft
    green; sanitize red on test_tuner_gradient and test_eval_trace, the
    same two and the same UBSan line on a clean 5ac17bd, with the
    sanitizer build's bench at 3872032. Mutants PK01 to PK04: 4 of 4
    killed.

    It costs speed, measured (DEC-266, against f6647dc): cycles +1.96 % on
    bench 16 (CI +1.74 .. +2.18, A/A floor 0.31) and +1.44 % on the 300
    positions (CI +1.26 .. +1.61, floor 0.47), hyperfine wall -1.42 %
    speed (CI -1.60 .. -1.25, floor 0.23), instructions +0.69 % and
    +0.66 %; perft +4.3 %. About -2.0 to -3.0 Elo at DEC-083's 1.43 per
    percent of nps -- a conversion, not a verdict, and no SPRT prices it.

    No functional change

Part (b), `b.diff`, the landing the SPRT plays as CAND; the verdict's record
commit carries DEC-220's block:

    Correct the static evaluation by a pawn-keyed history (S099)

    The static score errs the same way, again and again, in a given pawn
    structure. The search now keeps, per side to move and pawn key, a
    running average of how far its own score landed from the raw
    evaluation, and adds it, at most CorrHistMax (47) either way, to the
    static evaluation every main-search node reads. Taught beside the node's one store:
    out of check, no capture best move, no mate score, a bound only in its
    own direction. Both stores keep the raw evaluation, so nothing
    compounds, and the corrected score never reaches the mate band.

    The table is the UCI layer's and datagen's, cleared on ucinewgame.
    adocs/data/S099_sprt.sh pre-registers the {0, 5} SPRT against part (a).

    Bench: <pending>

### Rebased onto `22e8df7` (2026-10-14, Opus subagent; phase 1, written blind)

Worktree `/home/max/ws/chesso-s099c`, branch `s099c` at `22e8df7` -- S261's
vector `1b84f4b` with its SPRT pinned -- while that SPRT holds the machine
(DEC-260): nothing was compiled, tested or run. The input is s099b's whole
tree, `git diff -M HEAD` with its untracked files intent-added, saved as
`.tuning/coord/S099/s099_5ac17bd.diff`; against the earlier
`.tuning/coord/S099_files/final/s099.diff` it differs only in
`tests/test_mate_carry.cpp`, whose `DEC-272` was filled after that copy. It
went on with `git apply --3way`. The result is
`.tuning/coord/S099/s099_on_22e8df7.diff`. Phase 2 re-applies it onto
whichever of S261's closing commit (H1) or its revert (H0, no verdict) lands;
on a revert the same files collide and resolve to the pre-S261 side.

#### Deviations, first

1. **S099's three readings of the old tree leave the candidate**:
   `adocs/data/S099_sweep_s170.txt`, as the brief orders, and with it
   `adocs/data/S099_floor_sweep.log` and `adocs/data/S099_mined_floor.log`,
   the readings behind the two floors, for the same reason -- each was taken
   on a tree S261's vector has since moved. Phase 2 re-takes each under the
   same name. The old ones are kept in `.tuning/coord/S099/stale_5ac17bd/`
   and inside `s099_5ac17bd.diff`.
2. **One edit outside a conflict, to keep S261's intent.** "a probcut store
   keeps the raw static evaluation" now calls `require_no_singular_block()`
   after its abort check, as S261 made every ProbCut drive that calls the
   search directly do (DEC-277 (4)). The case already requires the cutoff,
   which returns above the singular block, so it narrows nothing; it keeps
   the drive family uniform.
3. **Old-tree goldens are removed, not kept beside S261's**: each site now
   holds S261's value and a PENDING phase 2 note, listed below.
4. The brief named `.tuning/coord/S099/s099.diff` as the diff to compare;
   it lives at `.tuning/coord/S099_files/final/s099.diff`, and the
   comparison was made there.
5. **CPU, disclosed for S261's run.** `git` plumbing only -- `apply
   --3way`, `merge-file` on `adocs/specs.md`'s search row split at its
   commas, `read-tree` and `apply --cached --check` against a temporary
   index (the check below that part (a) applies to `22e8df7`), `add`, `rm`
   -- plus `sed`, `awk`, `grep`, `cmp` and `bash -n` on the SPRT script,
   each well under a second. No build, test, engine, python or tool.

#### Conflicts, and how each was resolved

- `tests/test_mate_carry.cpp`, four hunks: S261's `--ceilings` command (the
  S202 and S261 grids), its table rows, F's ceiling 1 (DEC-282) and its "One
  rose at S261" paragraph are kept. S099's fix of the stale headline is kept,
  now reading S261's six, 0, 5, 4, 0, 0 and 1. S099's B and F rows and its
  "F's rose at S099" paragraph are dropped for a PENDING paragraph (DEC-272:
  F may rise to 2 with S099, on its H1 only; anything more stops for a
  ruling) and a PENDING line above F's `return` in `short_line_ceiling`.
- `tests/test_mate_breadth.cpp`, merged clean and corrected by hand: S099's
  "Re-read at S099" paragraph (149 and 144 against a floor of 145) described
  a floor that is now S261's 159; it is replaced by a PENDING paragraph after
  S261's, above `EXACT_FLOOR`.
- `tests/test_engine.cpp`, merged clean and corrected by hand: S099's rewrite
  of the `MATE_IN_THREE_FLOOR` golden text (13 and 8) sat beside S261's
  "Re-taken at S261" paragraph (12 and 6) and contradicted it; the parent's
  text is restored and a PENDING paragraph follows S261's. The first-mate
  rows stay S261's 9 and 9, with a PENDING note under S261's paragraph.
- `tests/test_search.cpp`, the includes: S099's `<limits>` and S279's
  `<memory>`, both, in order. S099's suite merged clean.
- `tests/test_search_params.cpp`, the count's narrative: S261's paragraph,
  then S099's (73 to 75). The golden list merged clean -- S261's values with
  S099's two rows -- and reads 75.
- `adocs/data/S170_cases.tsv`: C, D and E keep S261's budgets (2000000,
  2000000, 300000; A's 1000000 merged clean); S099's paragraph is replaced by
  a PENDING one.
- `adocs/specs.md`, the search row, one line both sides rewrote: merged
  segment by segment, S261's fitted values and wording with S099's
  correction paragraph and its three "raw" edits. The one segment both
  touched, razoring's, reads "a static evaluation `RazorMargin` (242, S261's
  fit of the seed 282)". S279's `go nodes` paragraph merged clean.
- `MANUAL.md`: the `RfpMinPly` row is S261's, `RfpTtEstimate`'s S099's, and
  `RazorMargin`'s S261's 242 and fit sentence with S099's two edits.
- Clean and re-read: `src/search.cpp` -- S279's check opens `quiescence`,
  which S099 leaves alone, and S261's comment edit in `negamax_at` is clear
  of S099's hunks; both stores still take the raw evaluation and the update
  still sits beside the one store. `src/search_params.hpp` (S099's block
  below `RazorDepth`), `DEV_MANUAL.md`, `src/data_structures.hpp` (a PENDING
  line above `CORRHIST_PAWN_ENTRIES`), and the files only S099 touches.
- Mutant anchors: since `5ac17bd` only S279's file joined `tools/mutants/`,
  and its two anchors occur once each in the merged `src/search.cpp`; no
  S099 anchor sits on a line this rebase edited.

#### Goldens PENDING phase 2 -- S261's values hold until the scripts answer

| site | holds now (S261's) | re-derived by |
|---|---|---|
| `tests/test_mate_carry.cpp` `short_line_ceiling` | 0, 5, 4, 0, 0, 1 | `adocs/data/S203_case_sweep.sh --ceilings` with S099's new grid added |
| `adocs/data/S170_cases.tsv` budgets | A 1000000, B 100000, C 2000000, D 2000000, E 300000, F 100000 | `adocs/data/S203_case_sweep.sh`, the cheapest cell at the row's stride with a mate line |
| `tests/test_mate_breadth.cpp` `EXACT_FLOOR` | 159 | `adocs/data/S156_mined_floor_sweep.py --floor 159` |
| `tests/test_engine.cpp` `MATE_IN_THREE_FLOOR` | 11 | `adocs/data/S154_floor_margin_sweep.py floor`, `red` |
| `tests/test_engine.cpp` `first_mate_depth` | 9 and 9 | `adocs/data/S188_repair_goldens.py first-mate` |
| `src/data_structures.hpp` `CORRHIST_PAWN_ENTRIES` | 2048 (a seed, not a test golden) | `adocs/data/S099_pawn_key_census.py` |

Not S099's to re-derive: the direct-search goldens S261 moved in
`tests/test_search.cpp` -- the S230 rows, E21 and B04, LQ02 and LQ03,
`NODE_TYPE_DEPTH`, `CAP_DRIVE_DEPTH`, the ProbCut drives. Those cases attach
no correction table and read the raw score, and that file drives no UCI path,
so S099 cannot move them.

#### DEC placeholders

This file's two `DEC-<n>` (the proposed stamp and the commit text) and its two
mentions of the placeholder now read DEC-272; `tests/test_mate_carry.cpp`
already carried DEC-272. DEC-270 and DEC-271 were in place. `decisions:`
gains DEC-272 and DEC-282. The proposed stamp and commit text above keep
`5ac17bd`'s numbers; phase 2 rewrites them.

#### `adocs/data/S099_sprt.sh`

REF and CAND stay `PIN_ME`. REF is S261's landing: its closing commit on H1,
the revert of `1b84f4b` on H0 or no verdict (the header and the refusal both
say so). The DEC-171 line, re-read against plan.md's Open list at `22e8df7`:
S280 open (the mutation recipe's `submodule update`, tooling); S278 and S279
closed. Bounds, outcomes, abort rule and cost unchanged. "What the tree
does", the census line and part (a)'s INV-6 are marked PENDING phase 2, and
the S261 paragraph names the rules below.

#### What part (b) takes from S261's values

- **No seed.** `CorrHistMax` rests on `PAWN` (94) and `CorrHistWeightCap` on
  `CONT_HIST_REF_DEPTH` (11); S261 moved neither file. Softly: 11 stands for
  the median remaining depth at the tuning control, which a vector that
  reduces more (`LmrBase` 52 -> 113) may have moved; the constant did not, so
  the seed stands as form (b) writes it, and S127 sweeps it.
- **No clamp.** The read is held to `CorrHistMax` and the sum inside the mate
  band's edge; no margin enters either.
- **Behaviour, not values: the rules the corrected number feeds.** Reverse
  futility (its margin unmoved); razoring (`RazorMargin` 282 -> 242); the
  move loop's futility on the estimate (`FutBase` 147 -> 136, `FutSlope`
  170 -> 182, `FutMaxLmrDepth` 8 -> 16); and through the improving flag, late
  move pruning's count (`LmpBase` 733 -> 935, `LmpMaxLmrDepth` 8 -> 3) and the
  not-improving reduction (unmoved). Not fed: ProbCut (beta plus its margin
  against a search score; its store is now raw), the singular margin (a table
  score), `LmrDeeperMargin` (search scores), `QsFutilityMargin` (quiescence,
  raw). All of these were fitted without a correction; the SPRT measures the
  pair as it stands and S127 refits (section 7).
- **Tests that read an S261 value.** "a probcut store keeps the raw static
  evaluation" drives `probcut_drive_t` at S261's ProbCut depths and margin
  (6, 3 and 87) and needs its cutoff with one unit planted; S261 re-derived
  that drive without a table, so with the plant it is unproven. "the
  correction table hides no forced mate" searches depths 3 to 6 on S261's
  vector, where `SeMinDepth` 6 and `ProbCutMinDepth` 6 now admit both blocks
  at depth 6, with every slot at +47 and at -47; unproven there. The reverse
  futility consumer case reads `RFP_MARGIN`, `RFP_MIN_PLY` and
  `RFP_MAX_DEPTH`, none moved. The site case's children are quiescence, whose
  `QsFutilityMargin` moved, but it compares the node against its own
  reference and asserts floors of one.

#### What I expect phase 2's compiler and tests to say

- **Compiler**, both builds and Debug: nothing new to compile but where two
  sides met -- disjoint hunks in `src/search.cpp` and one include line -- so
  0 warnings and 0 errors. **Format**: every edit here is a comment within
  80 columns or the include, in the order the style sorts; clean expected.
- **`test_mate_carry` likely red**: S261's budgets on S099's tree. On
  `5ac17bd` S099 silenced all three of C, D and E's old cells. Fixed by the
  re-sweep, never by hand.
- **`test_mate_breadth` at risk**: 159 sits one below S261's shipping 160. On
  the tree before S261 the correction cost the mined set one exact mate at
  each end -- S261 read 150 and 145 on `fd9e44c`, whose search is
  `5ac17bd`'s, and S099 read 149 and 144 with the correction. (S099's old
  text compared against S116's 149 and called the shipping end held; the
  parent was 150.) The same loss here leaves 159 at zero margin.
- `test_engine`'s first-mate depths and mate-in-three counts may move; the
  two unproven cases above are expected green. Everything else as on
  `5ac17bd`: green.

#### Phase 2, in order, with machine time

`.moltke.local.md` measures match throughput only (2110 games an hour at
8+0.08); the other times are this step's and earlier steps' recorded wall
times, and the two marked *est.* are estimates. The worktree has no build
directories yet.

1. Configure `build`, `build-tune` and `build-debug` in the worktree
   (DEV_MANUAL.md's `cmake -S . -B ...` lines) and build: *est.* 15 min.
2. `export CLANG_FORMAT_MAJOR=22`, then AGENTS.md's TESTS command, both
   builds and the format check: about 5 min. Red on `test_mate_carry` is
   item 5's input; any other red stops phase 2.
3. `bench` (the new total), `bench 12`, `tools/search_bench.py` at depths 9
   and 12 on both sides; the tune build at `setoption name CorrHistMax value
   0` prints S261's `bench` stream, 5636835 (DEC-215): about 5 min.
4. INV-6 for (a) alone: a fixture of `22e8df7` plus
   `.tuning/coord/S099/a_branchless_5ac17bd.diff` (it applies there without
   a conflict, checked against a temporary index; its board hunks are
   byte-identical to this tree's and every line it adds to the data
   structures is here); `bench` 5636835 with an identical stream
   and `search_bench` 9 and 12 identical to `22e8df7`: about 10 min.
5. The census, `python3 adocs/data/S099_pawn_key_census.py
   build-tune/src/chesso 12`: 10 to 20 min (DEC-270). If its p95 passes 2048,
   the size moves and items 2 to 4 are re-taken first.
6. `adocs/data/S203_case_sweep.sh` on a fixture of the tree into a new
   `adocs/data/S099_sweep_s170.txt` (14 min serial at S116, less split six
   ways); the budgets by the rule; then `--ceilings` over
   `adocs/data/S202_sweep_s170.txt`, `adocs/data/S261_sweep_s170.txt` and
   the new grid. F above 2, or any case above S261's six, stops for a
   ruling (DEC-272).
7. The floors: `adocs/data/S154_floor_margin_sweep.py floor` and `red`,
   `adocs/data/S156_mined_floor_sweep.py --floor 159`, re-written as
   `adocs/data/S099_floor_sweep.log` and `adocs/data/S099_mined_floor.log`;
   `adocs/data/S188_repair_goldens.py first-mate`: about 15 min. A floor that
   no longer sits one below its shipping end and above its weakened end
   stops for a ruling (the question in the report).
8. The gate again with every golden in, both builds: about 5 min.
9. Mutants on fixtures, `tools/mutation_check.py`, with S280's
   `protocol.file.allow=always` and `CLANG_FORMAT_MAJOR=22` exported: CH01 to
   CH14 on the whole tree (1714 s on `5ac17bd`) and PK01 to PK05 on (a)'s
   fixture (753 s): about 45 min.
10. Debug self-play, four rounds at 4+0.04, DEV_MANUAL.md's command,
    grepped for `Assertion`: *est.* 10 min.
11. `tools/gate_extra.sh`, all five stages: 834 to 1040 s at S278 and S116,
    about 17 min; sanitize is expected green now that S278 has landed.
12. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`:
    seconds.
13. Writing, no machine: the SPRT script's numbers; rows in
    `adocs/data/README.md` for S099's data files, which have none;
    DEV_MANUAL.md's golden table rows for the four goldens; MANUAL.md's
    `RfpMinPly` breadth numbers if they move; the commit's `Bench:` line.
14. **No timing is owed**: DEC-271 has the SPRT price (a)'s cost. The
    counters comparison of branchless (a), about 6 minutes, stays optional
    and informs only.

Items 1 to 12 are about **2.5 hours** of machine, under four hours, so they
start when S261's verdict frees the machine (DEC-155). Then the SPRT: about
**12.1 h** on a bound and **19.0 h** at the 40000-game cap (the midpoint's
41861 expected games pass the cap), at 2110 games an hour -- a night; plus a
1000-game A/A (about 28 min) first if the harness has moved since the last
one (DEC-143).

### Phase 2 on `6fc7f23` (2026-10-14, Opus subagent)

`s099c` fast-forwarded `22e8df7` -> `6fc7f23` (S261's H1 closing commit
`bacead1`, then S280) by `git stash`, `merge --ff-only`, `stash pop --index`,
with no conflict; backup `.tuning/coord/S099/s099_pre_ff.diff`. Items 1 to 12
of the rebase section's list ran on this machine with nothing else on it;
every script and log is in `.tuning/coord/S099/p2/`. Phase 2 stopped twice
on the carry ceilings -- DEC-284 answered the first, and the coordinator read
DEC-272 onto F for the second -- and the fast check over its first diff found
a gate defect, fixed below, after which every reading the gate moves was
re-taken. The readings below are the fixed tree's unless marked.

#### Deviations, first

1. **The update gate refused a fail-low whose best move was a capture**
   (the fast check's finding 1, confirmed in the code). `negamax_at` always
   names a best move -- it asserts one before its store -- and at a fail-low
   that is only the move whose upper bound came out highest, this engine's
   form of the published "no best move"; the old clause, `best_move != 0 &&
   MOVE_CAPTURE(best_move) != 0`, refused it all the same. **Red first**: two
   new rows of "the gate refuses each published case and nothing else" -- a
   fail-low capture below and at the raw score, taught -- failed on the old
   clause, 2 of 23 assertions (`p2/redfirst.log`). The clause is now
   `type != TT_ALPHA_NODE && MOVE_CAPTURE(best_move) != 0`, its comment says
   why, the case also holds a fail-high capture refused and a fail-low
   capture above the raw score refused by its direction, and the probe-only
   `best_move` 0 row is gone (the site never passes 0). The site case
   classifies refusals in the gate's order and counts taught fail-low
   captures in its census line. **CH15** restores the old clause; CH05 is
   re-anchored on the new one. specs.md and MANUAL.md say so where they
   describe the gate.
2. **The table is 4096 slots a side, not 2048** -- the census's own rule
   (DEC-270 (1)): on the fixed tree p95 2781 at 2048 and 2844 at 4096.
   `CORRHIST_PAWN_ENTRIES`, its comment, the two "16 KiB" comments
   (`src/chesso.cpp`, `tools/datagen.cpp`) and specs.md's slot count read
   4096 and 32 KiB.
3. **The breadth floor rose, 159 -> 163.** The correction raised both of the
   mined set's ends, to 164 and 160, so 159 no longer separates; 163 is 1
   below the shipping count. A rise tightens the test, so it is the script's
   re-derivation (DEC-142); DEC-283, which speaks of drops, moved nothing.
4. **The carry ceilings are 2, 5, 4, 0, 1, 2**: A 0 -> 2 and E 0 -> 1 by
   DEC-284, F 1 -> 2 by DEC-272 (the owner's ruling for F, superseding
   DEC-282's 1 on S099's H1), each standing only on S099's H1. Before the
   gate's fix the same grid answered 2, 5, 4, 0, 4, 1, which is what DEC-284
   was ruled on.
5. **The fast check's findings 2 to 4**: specs.md's `board_t` reads 224 B
   (measured: `sizeof(board_t)` 224 on this tree, 216 on main's); the
   `RfpTtEstimate` 0 sentences in specs.md and MANUAL.md say that since S099
   0 gives the tree before S234 only with `CorrHistMax` 0 as well; "every
   node" reads "every main-search node" in specs.md, MANUAL.md and this
   file's commit texts; the census and first-mate readings are committed as
   `adocs/data/S099_pawn_key_census.txt` and `adocs/data/S099_first_mate.txt`
   with README rows.
6. Fixtures are commit objects on no branch: the whole tree `febfcf2` (the
   floors, and `5eee8eb` with the floor at 163) and `dfc37c6` (the mutants)
   on `6fc7f23`, part (a) `72480fd` on `bacead1`. Linked worktrees left inside
   s099c: `.ref-builds/s099a`, `.ref-builds/mut099`, `.ref-builds/mut099b`.
   No ref moved.
7. The reference engine is S261's SPRT build of `1b84f4b`
   (`.ref-builds/1b84f4b` in the main checkout); its `src/` is `bacead1`'s
   and `6fc7f23`'s.
8. Budgets are applied by `p2/pick_budgets.sh` (S203's rule over the
   recorded grid), never typed. One slip on the way, caught and repaired: its
   first wrapper read a cached copy of the TSV and put back an old comment
   paragraph, which was rewritten and the wrapper fixed to read the live file.
9. Rows beyond the list: `adocs/data/README.md` gains S099's section;
   DEV_MANUAL.md's golden table carries S099's readings and its
   `golden_defaults` row, stale at 71 since S116, reads 75; MANUAL.md's
   `RfpMinPly` row carries the breadth numbers on this tree.

#### Readings, the fixed tree

- **1. Builds.** Release, tune and Debug configured and built: no warning,
  no error, 118 s.
- **2. The gate before re-derivation**: 45 of 46 in each build, red only
  `test_mate_carry` (3 of 6 cases reporting, the budgets gone silent); format
  wanted one reflow in the new test, applied with clang-format-22.
- **3. What the tree does** (`p2/bench/`). `bench` 5636835 -> **6042933**,
  replies `c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6` (the second was e2a6,
  the fifth a7a6); `bench 12` 1937969 -> 2339074, replies unchanged.
  `tools/search_bench.py` depth 9 43496 / 160453 / 30816 -> 62042 / 125888 /
  34107, depth 12 180917 / 509438 / 147594 -> 214973 / 595000 / 217107,
  best moves `c3d5 e2a6 d7c8q` and `c3d5 d5e6 d7c8q` on both. The tune build
  at `CorrHistMax` 0 prints S261's stream, 121 lines, none differing
  (DEC-215). (Before the fix: 7052777 at 4096, 5286481 at 2048.)
- **4. INV-6 for part (a) alone** (`p2/inv6/`), `bacead1` plus the
  branchless key, not re-taken since part (a) is untouched: `bench` 5636835
  and `bench 12` 1937969 with streams identical line for line (121 and 105),
  `search_bench` 9 and 12 node-identical, same best moves.
- **5. The census** (`adocs/data/S099_pawn_key_census.txt`), larger side per
  position: at 2048 p50 409, p90 1756, **p95 2781**, p99 5029, max 8368; at
  4096 p50 409, p90 1756, **p95 2844**, p99 5698, max 9009; 4096 both
  times. It depends on the gate -- what the table learns moves the tree the
  reads walk -- so it was re-taken.
- **6. The grid** (`adocs/data/S099_sweep_s170.txt`, 108 cells, 1461 mate
  lines and 18 short, 279 s split six ways). Budgets by the rule: C 2000000
  -> 1200000 (23 lines, 1 short), D 2000000 -> 500000 (1, 0), F 100000 ->
  300000 (3, 0); A 1000000 (6, 2 short), B 100000 (29) and E 300000 (4, 0)
  unchanged. `--ceilings` over S202's, S261's and this grid: **2, 5, 4, 0,
  1, 2**; this grid alone 2, 1, 1, 0, 1, 2. A's 2 is its 1000000 cell at
  stride 1, E's 1 its 4000000 cell, F's 2 its 1500000 cell at stride 2.
- **7. The floors**, each on a commit object of the tree:

  | golden | shipping | weakened | gate at `RfpMinPly` 1 | value |
  |---|---|---|---|---|
  | `MATE_IN_THREE_FLOOR` | 12 of 24 | 7 | red (7 >= 11 fails) | 11, unmoved |
  | `EXACT_FLOOR` at S261's 159 | 164 of 318 | 160 | would pass at 160: no longer separating | -- |
  | `EXACT_FLOOR` at 162 | 164 | 160 | red | -- |
  | `EXACT_FLOOR` at 163 | 164 | 160 | red | **163** |
  | first-mate depths | 9 and 9, mate 5 | -- | -- | unmoved |

  `adocs/data/S099_floor_sweep.log`, `adocs/data/S099_mined_floor.log`,
  `adocs/data/S099_first_mate.txt`. No floor went down, so DEC-283 moved
  nothing.
- **8. The gate with every golden in**: 46 of 46 in both builds, format clean, 258 s, on the final text (`p2/gate8.log`); the run before it, `gate7`, was 46 of 46 with one comment reflow owed, applied with clang-format-22.
- **9. Mutants** (`p2/mutation_ch2/`, DEV_MANUAL.md's recipe with S280's
  line): **CH01 to CH15, 15 of 15 killed** on the whole-tree fixture `dfc37c6`, 1916 s, each by the binary its `Mutation:` line names; **CH15**, the old capture clause, by the gate case's two new fail-low rows (and `test_mate_carry`). The bench signature held on CH01, CH02 and CH06 (the two clamps and the in-check clause), as before. PK01 to PK05 were not re-run: part (a) is untouched,
  and they read 5 of 5 killed on its fixture before the fix (`p2/mutation_pk/`).
- **10. Debug self-play**: four rounds at 4+0.04, the Debug build of the fixed tree: **8 games**, all normal terminations, 0 time forfeits, 0 disconnects, **0 `Assertion`** over a 179659-line trace log carrying 1219 `bestmove` lines.
- **11. `tools/gate_extra.sh`**: all five stages, JOBS=12, **ok in 1019 s** (`p2/gate_extra2/`): prose, citations, debug (347 s, the Debug binaries with the key's oracle asserted at every make and unmake), sanitize (618 s, 46 of 46, the sanitizer build's `bench` 6042933, the Release total) and perft (54 s).
- **12. Prose checks** on this final text, one mode a call: `--citations` 0 flagged over 29 files (this file 30 code citations, 0 flagged), `--touches` 0 flagged, `--params` exit 0, `--gate` agreeing.

#### Proposed `done:` stamp (supersedes the two above)

    2026-10-14 HH:MM -- one verdict for the key and the correction together
    (DEC-271). <SPRT line for the coordinator: H1 | H0 | no verdict at
    {0, 5} nElo, 8+0.08, Hash 16, CAND <sha> vs REF bacead1: Elo, nElo,
    LLR, games, forfeits (adocs/data/S099_sprt.log); kept | the commit
    reverted whole, S110, S111 and S281 to the reserve>. The key branchless
    and node-identical on its own against bacead1 (INV-6: bench 5636835,
    bench 12 1937969, search_bench 9 and 12); the table 4096 slots a side by
    DEC-270's census (p95 2844); the gate teaches a fail-low whatever its
    best move, the fast check's finding, red first; bench 5636835 ->
    6042933. Fast suite 46/46 in both builds, format clean; Debug
    self-play four rounds, 8 games, 0 Assertion, 0 disconnect; gate_extra all five stages green (1019 s), the sanitizer bench equal to Release; mutants
    CH01-15 15 of 15 killed on the whole tree, each by its named case, and PK01-05 5 of 5 on the key's fixture. Goldens re-derived by their scripts: carry ceilings A
    0 -> 2 and E 0 -> 1 (DEC-284), F 1 -> 2 (DEC-272), budgets C 1200000,
    D 500000 and F 300000; EXACT_FLOOR 159 -> 163, a rise;
    MATE_IN_THREE_FLOOR 11 and the first-mate depths 9 and 9 unchanged
    (DEC-283 moved nothing). Implemented from the step file's description
    and the publications it cites (DEC-221).

#### The candidate's commit text (supersedes the ones above)

    Correct the static evaluation by a pawn-keyed history (S099)

    The static score errs the same way, again and again, in a given pawn
    structure. The search now keeps, per side to move and pawn key, a
    running average of how far its own score landed from the raw
    evaluation, and adds it, at most CorrHistMax (47) either way, to the
    static evaluation every main-search node reads. Taught beside the
    node's one store: out of check, no capture best move except at a
    fail-low (whose best move is only its highest upper bound), no mate
    score, a bound only in its own direction. Both stores keep the raw
    evaluation, so nothing compounds, and the corrected score never
    reaches the mate band. Quiescence is untouched.

    The key is board_t::pawn_hash, the pawn piece-square keys alone, kept
    by add_piece, remove_piece and move_piece and undone by xor in unmake
    from a twelve-row table whose non-pawn rows are zero, so no site asks
    which piece moved: the first form's branch cost 1.4 to 2.1 % of speed
    (DEC-271). compute_full_pawn_hash() is its oracle, walked by the
    invariant tests (INV-2, INV-4). Alone it is node-identical to S261's
    landing bacead1 (INV-6: bench 5636835 and bench 12 1937969 with
    identical streams, tools/search_bench.py at 9 and 12). It lands with
    the correction because on its own it buys nothing, so the SPRT prices
    both against the tree before the key.

    The table is 4096 slots a side, the next power of two above the 95th
    percentile of the distinct pawn keys a depth-12 search reads (DEC-270's
    census on S261's vector: 2781 at 2048 slots, 2844 at 4096). The UCI
    layer owns it and datagen keeps one per worker; ucinewgame clears it.
    A mirrored position reads the same corrected score (INV-5), and the
    tune build at CorrHistMax 0 prints the parent's bench stream (DEC-215).

    Goldens re-derived by their scripts (DEC-142): test_mate_carry's
    ceilings A 0 -> 2 and E 0 -> 1 (DEC-284) and F 1 -> 2 (DEC-272), with
    S099's grid added to the command and S281 to study the short lines,
    and the S170 budgets C 1200000, D 500000 and F 300000 from the same
    grid; test_mate_breadth's EXACT_FLOOR 159 -> 163, a rise, the
    correction having raised both ends to 164 and 160 so that 159 no
    longer separated; MATE_IN_THREE_FLOOR 11 (12 shipping, 7 weakened) and
    the first-mate depths 9 and 9 unchanged, so DEC-283 moved nothing.
    adocs/data/S099_sprt.sh pre-registers the {0, 5} SPRT against
    bacead1, one verdict for the key and the correction together.

    Bench: 6042933
