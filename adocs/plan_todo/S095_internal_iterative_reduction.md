id:         S095
goal:       reduce a node whose table entry carries no move instead of searching it at full depth
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the depth threshold and the reduction amount are constants in src/search_params.hpp with stated ranges (S073); the reduction applies only where the entry genuinely has no move, with a test asserting the precondition -- a node whose entry does have a move must not be reduced, and the test fails if the precondition is absent; a position with a forced mate inside the reduced depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   internal iterative deepening, the older and more expensive form, unless the step measures both and says which it kept
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## What it replaces

The older technique is internal iterative deepening: search the node to a
shallower depth first, purely to get a move to order with. The reduction is the
modern form -- if there is no table move, the node is cheaper than its depth
claims and is searched one or two plies shallower instead. Both are documented;
this step implements the reduction and measures it, and the deepening form is
out of scope unless the measurement says otherwise.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol if drifted. Every GitHub read
below was a commit message, PR body or release note, never a diff or source
file (DEC-016, DEC-084). rebel13.nl (ProDeo's own writeup) answered HTTP 526
in this pass and is cited from nothing; the talkchess thread is the primary.

### 1. State of the art

**Lineage.** IID (CPW: Scott 1969; Anantharaman 1991 for Deep Thought — kept
there "despite it being pretty much a washout on average"): no hash move at a
PV node -> run a reduced search of this node purely to get one, then search at
full depth. The modern form keeps the signal and drops the purchase: **no hash
move -> cut the node's whole depth**, "in the hope that the node must not be
very important as there was no hash move present" (CPW IIR). Ed Schroder
introduced it in Rebel — talkchess 2020-08-13, "An alternative to IID": IID
out, "a full ply reduction without research", **+17 after 5000 games**, about
two plies more depth at fixed time (12.67 -> 14.49); an independent tester
confirmed +13/2.5k, +14/7.5k.

**Why reduction won, in the record.** Weiss added classic IID in 2019 at ~0
STC / +4.11 LTC (#113) and replaced it with the Rebel form two days after
Ed's post for **+8.02 +/-4.95 STC / +5.61 +/-3.76 LTC** (#316, 2020-08-15).
Stockfish deleted IID as a *simplification* the week after — non-regression
bounds — "employ a depth reduction if the position is not in TT and on the
PV" (e64b957, 2020-08-21). Ethereal had removed IID outright at 11.16
(2019-01) with nothing in its place. The deepening's shallow search
duplicates what the outer iterative-deepening loop and the table already do.

**Conditions, precisely.** Two published triggers: **entry-absent**
("position is not in TT", SF's first form) and **move-absent**
(`ttMove == 0`, which also fires on an entry whose move field is empty).
Lynx measured the widening entry->move at **+11.63 +/-5.25** (#1516) — the
union is the stronger published condition. Node types: Rebel used all node
types (CPW); Lynx ships that and measured every restriction negative in its
stack (PV-only **-45.38/-46.88**, #2024/#2026; pv||cut **-19.75/-21.08**,
#2027/#2220); Weiss went PV-only for +1.67 STC at ~3300 (#569); SF began
PV-only, Chaly extended to expected cut nodes in 2021 (CPW prose — commit
untraced publicly this pass); Ethereal built it in reverse order, cutnodes
first (**+4.39/+2.88**, 3071b40, 2023-04), PV later (+1.37/+2.08, 9243eae).
Direction is stack-dependent — DEC-019. Threshold: Lynx introduced at **min
depth 4** (#507), 4->5 +0.43 over 109k games, 5->6 0.08 — flat above 4; CPW
offers "depth > 5, say"; SF simplified its depth condition away at ~3800
(3747a19). Amount: **1 ply** at every traced introduction; 2 plies measured
**-12.38** (#1361) and **-46.23** (#2028) at Lynx.

**Traced introductions.** Rebel +17/5000. Weiss #316 +8.02/+5.61 (~3000
era). Berserk #89 "Replace IID with Ed's IIR" **+6.74 +/-4.67** at 8+0.08
(7ba082b, 2021-05). Lynx #507 **+9.5 +/-6.8** LTC, v1.1.0 (2023-12) — Lynx
1.0.1 = 2432 CCRL, the one introduction below chesso's band. Placement:
before the forward-pruning block — Weiss #450 **+2.87/+2.37**, Berserk #140
**+2.17** ("less depth to be hit by more pruning mechanisms"); after-pruning
re-tried at +0.44 ~0 (Lynx #2195, ~3100+). Root: Lynx allows it, **+1.39
+/-1.05** over 170k games (#2025). Refinements at 3400+, noted not targeted:
reduce more when the entry's depth lags the search depth (Ethereal 0850fdd
+2.86/+3.31; SF cc992e5), cutnode-with-upper-bound-move (SF db147fe), later
pulled back for "poor scaling at longer time controls" (SF 55cb235, 8b32e48,
40e0486). Stash: no IID/IIR commit traced at all.

### 2. Shape for chesso

- - The condition: tt_get_entry (src/search.cpp:663) returns nullptr on a miss
  (transposition_table.cpp:96-103), and src/search.cpp:667 already computes
  `tt_move = (tt_entry != nullptr) ? tt_entry->best_move : 0`. **The probe
  exposes both variants today** — `tt_entry == nullptr` is entry-absent,
  non-null with `best_move == 0` is entry-present-move-empty — and `tt_move ==
  0` is their union, the published winning condition, one variable already in
  scope. Verified who writes moveless entries: negamax never (assert
  src/search.cpp:1069, store src/search.cpp:1099-1101); **quiescence does**,
  since S094 (src/search.cpp:432-434 stand-pat beta, src/search.cpp:556-557
  mated, src/search.cpp:576-578 fail-low), all at TT_DEPTH_QS. So "entry with
  no move" concretely means "only quiescence has resolved this position" — the
  unimportance signal the technique prices. The goal's "table entry carries no
  move" is read as the union: a missing entry carries no move either.
- - The site: node level, after the TT-cutoff block (src/search.cpp:675-681)
  and the quiescence drop (src/search.cpp:684), before the RFP block
  (src/search.cpp:765) — before all forward pruning, the placement with the two
  positive records. Cut `depth` once; everything downstream reads the reduced
  depth by construction: the RFP gate (src/search.cpp:766), the null-move
  formula (src/search.cpp:814), S109's move-loop gates, S098's table row via
  lmr_reduction(depth, ...) (src/search.cpp:962), child_depth
  (src/search.cpp:943), and the store (src/search.cpp:1100), which then records
  the depth the node was actually searched to.
- - No ply guard: the root re-stores its entry with a move every iteration, so
  from iteration 2 it has a TT move — and Lynx's allow-root record is +1.39. No
  in-check condition either (the published simple form has none; is_in_check
  src/search.cpp:687 sits below the site anyway). Quiescence is untouched — no
  depth to cut (qply cap only), and no published implementation reduces there.
  No IID remnant exists in the tree (grepped; specs.md "absent, search" lists
  IIR as absent) — the excludes line is scope, not a deletion.

### 3. Implementation sketch

1. The change is the three lines plan.md priced:
   `if (depth >= IIR_MIN_DEPTH && tt_move == 0) depth = std::max(depth -
   IIR_REDUCTION, 1);` at the site above. The explicit floor at 1 is the
   depth<=0 guard: no reduction may create a fall-through to quiescence (the
   repo's recurring bug class — null move's depth-0 mate miss). At the seeds
   the floor never binds; it exists so no SPSA value can open that path.
2. Tests, red first. The harness is the exposed state by construction:
   search_fen (tests/test_search.cpp:37) resets the table and makes one
   fixed-depth search() call, so every node starts with tt_move == 0.
   - The accepts' precondition pair: cold-table search at fixed depth >=
     IIR_MIN_DEPTH, node count recorded; then pre-store an entry with a move
     for the root position (a prior shallower search through search_fen_with
     suffices) and assert the with-move count equals the feature-off count —
     the "entry with a move must not be reduced" side, failing if the
     precondition is absent.
   - The accepts' mate case: a forced mate inside the reduced depth added
     beside "pruning does not hide a forced mate",
     tests/test_search.cpp:2808 and "pruning does not hide a mate against
     the material leader", tests/test_search.cpp:2847, observed red with the
     guard removed (threshold to 0 locally, observed, reverted — the S033
     protocol; cold fixed-depth search is where it bites).
   - Both existing mate suites re-run; fast suite green. Node counts move by
     construction — INV-6 takes the SPRT path; search_bench depths 9/12 in
     the stamp.

### 4. Constants and seeds

Both in src/search_params.hpp (the src/search_params.hpp:50 X-macro) with
stated ranges; each is a **seed — must be fitted/SPSA'd here** (S127, DEC-084).

- `IIR_MIN_DEPTH` **4** — Lynx #507's introduction value; 4->5 +0.43 and
  5->6 0.08 say flat above it; CPW's "depth > 5, say"; SF deleted the
  condition at ~3800. Range 2..63: 2 keeps the floor trivially true, 63 is
  the off value.
- `IIR_REDUCTION` **1** — every traced introduction. Range 0..3, 0 = off.
  **Anti-seed: 2, measured -12.38 and -46.23 at Lynx** (#1361, #2028).
- No cutnode term: chesso gains a cut_node input only if S098's verdict 2
  shipped, and the cutnode variants are 3300+ records in both directions —
  S127-era material, not this step's.

### 5. Pitfalls

- **Stacking with S098/S091 is in series, not additive.** IIR cuts `depth`
  once at node level before the move loop; S098's reduction and S091's extra
  ply are per-move cuts on child_depth inside it. The per-move machinery
  reads the already-reduced depth (table row, S109 gates, clamps) and needs
  no edit. What is forbidden is a second per-move no-TT-move term inside
  LMR: Lynx measured that duplicate at **-8.08** with IIR present (#2253);
  S098's file carries the same warning from its side.
- - **Re-visits do not re-reduce forever here.** The reduced visit stores its
  entry at the reduced depth *with a move* (src/search.cpp:1069); the next
  visit at the same nominal depth finds the move — no cutoff, stored depth one
  short (src/search.cpp:228) — and is not reduced. The loop terminates because
  every negamax store carries a move. Residual re-fire paths — a slot lost to a
  collision (key mismatch reads as no-entry), a quiescence store recapturing
  the slot across a `go` boundary (in-generation it cannot:
  transposition_table.cpp:158 replaces only at depth >= entry->depth, and
  TT_DEPTH_QS loses to any main depth) — cost 1 ply once per visit, bounded.
  The published mitigation for the saturated-table case is the depth threshold
  (Ed's stated concern on talkchess), and S105's Hash 16 regime is deliberately
  high-pressure, so the threshold is load-bearing here, not decorative.
- **S097 needs a TT move by definition — no overlap.** Singular extension
  verifies a node whose entry has a move and sufficient depth/bound; IIR
  fires only where tt_move == 0 — mutually exclusive at a node. The one real
  interplay, IIR firing *inside* the verification search's subtree, is
  S097's edit, records both ways: Berserk +3.64 for disabling IIR during SE
  (#91), Lynx +0.09 ~0 for the same (#1734).
- **The mate hazard is one iteration wide, and the floor is pinned.**
  Reduce-1 above the threshold keeps depth >= 1 always — never into
  quiescence. What remains is a mate at a reduced node's horizon arriving
  one iteration later; iterative deepening plus the stored move heal it in
  play, and the cold-table fixed-depth mate tests are where a wrong floor or
  threshold shows.
- - **Store the reduced depth.** Mutating `depth` before the loop makes
  src/search.cpp:1100 store it correctly; cutting a copy and storing the
  original would claim depth the node never searched and poison deeper cutoffs.

### 6. Measurement

One SPRT at the S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`,
against the commit before it; fast suite and both mate cases green first, the
new tests observed red first with printouts recorded. Expectation (DEC-019,
direction only): introductions at or near the band measured +6.7..+9.5
(Berserk at 8+0.08, Lynx LTC at ~2450, Weiss STC). A zero is recorded as zero
and dropping three lines is the default outcome at zero (S005/S006/S015).

### 7. Interactions

- **S098 (before)**: composes in series, node-level before per-move; no
  per-move duplicate term — its file and section 5 both say so.
- **S097 (after)**: TT-move dependency makes the node conditions disjoint;
  verification-subtree behaviour is S097's decision, records in section 5.
- **S119 (later, note only)**: buckets and aged replacement change how often
  a probe finds an entry and whether its move survived, so the IIR fire rate
  moves with the table layout; S127 re-tunes the two constants after it.

### 8. References

- - https://www.chessprogramming.org/Internal_Iterative_Reductions — lineage
  (Schroder, Rebel 2020), all-node-types origin, SF PV-only then Chaly cutnodes
  2021, "depth > 5, say", the entry-depth refinement idea.
- - https://www.chessprogramming.org/Internal_Iterative_Deepening — Scott 1969,
  Anantharaman 1991, classical PV-only conditions, reduction amounts, Deep
  Thought's "washout on average".
- - https://talkchess.com/viewtopic.php?t=74769 — Ed Schroder, "An alternative
  to IID", 2020-08-13: +17/5000, full ply without research, ~2 plies depth
  gain, TT-saturation concern; silentshark +13/+14.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+IIR+in:title+type:pr
  — #507 +9.5 (min depth 4); #1516 !ttHit -> !ttHit||!ttMove +11.63; #2025 root
  +1.39; #2035 depth 4->5 +0.43; #2236 5->6 0.08; #1361/-12.38 and #2028/-46.23
  reduce-2; #2024/#2026 PV-only -45/-47; #2027/#2220 pv||cut -20/-21; #1237
  pv&&cut -6.73; #2195 after-pruning +0.44; #1734 SE +0.09; #1524..#1534
  ttDepth offsets ~0.
- - https://api.github.com/repos/lynx-chess/Lynx/releases/tags/v1.1.0 —
  2023-12-14, "Add Internal Iterative Reduction (IIR) (#507)".
- - https://api.github.com/search/issues?q=repo:TerjeKir/weiss+IIR+type:pr and
  +IID — #113 IID 2019 (~0/+4.11); #316 "Rebel IID" 2020-08-15
  +8.02/+5.61/+3.83; #450 before-pruning +2.87/+2.37; #569 PV-only +1.67 STC,
  LTC unresolved.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22internal+iterative%22
  — e64b957 2020-08-21 removes IID, "depth reduction if the position is not in
  TT and on the PV", non-regression; 8dea070 before-probcut 2023.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22IIR%22
  — 6d0d430, db147fe, 55cb235 ("poor scaling"), 8b32e48, 40e0486, 9cc15b3,
  3747a19 (depth condition simplified away), cc992e5 (entry depth >= search
  depth, reduce more).
- - https://api.github.com/search/commits?q=repo:jhonnold/berserk+IIR — 7ba082b
  #89 "Replace IID with Ed's IIR" +6.74 at 8+0.08; b81aa1d #91 disable during
  SE +3.64; 9c1b743 #140 before pruning +2.17.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+IIR —
  3071b40 "Introduce cutnodes, and perform IIR on them" +4.39/+2.88 (2023-04);
  0850fdd tt-depth lag +2.86/+3.31; 9243eae PV +1.37/+2.08.
- - https://api.github.com/repos/AndyGrant/Ethereal/releases — 11.16 (2019-01)
  "Remove Internal Iterative Deepening, apply Singular Extensions
  aggressively".
- -
  https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+%22iterative%22
  — negative result: no IID/IIR commit traced.
- - https://rebel13.nl/prodeo/prodeo-3.0.html — unreachable this pass (HTTP
  526); not used.
