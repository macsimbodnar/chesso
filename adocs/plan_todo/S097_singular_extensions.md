id:         S097
goal:       extend the one move a verification search says is singular, and take the multicut the same search offers
accepts:    an SPRT verdict per change, measured separately -- the extension and the multicut are two changes off one verification search; the verification search excludes the table move, runs at a reduced depth against a window below the table score, and is skipped at the root and where the entry is too shallow or its bound is wrong, each condition asserted by a test that fails if the precondition is absent; the margins and the reduced depth are constants in src/search_params.hpp with stated ranges (S073); a position with a forced mate inside the multicut's pruned depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   check extensions, which are S096; any extension not derived from the verification search
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## The most expensive item in the block

It searches the node twice at some nodes and it is the one technique here whose
cost shows up as a nodes-per-second loss before any Elo appears. It also
depends on the table entry carrying a usable depth and bound, so it sits after
S094. It is kept in the plan because it is the one search feature that is on
every engine in the 3000-plus hand-crafted band and absent here.

Reported figures put it at 30 to 60 Elo. DEC-019: that decides that it is
tried, and the SPRT decides what is kept.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol if drifted. Every GitHub read
below was a PR body, commit message, release note or README rating table,
never a diff or source file (DEC-016, DEC-084). CCRL's own pages refused this
pass (ccrl.chessdom.com DNS-dead, computerchess.org.uk 403), so rating bands
lean on repo-recorded numbers and the engines' own records, marked where soft.

### 1. State of the art

**Origin, acknowledged.** Anantharaman, Campbell and Hsu, "Singular
Extensions: Adding Selectivity to Brute-Force Searching" — AAAI Spring
Symposium 1988, ICCA Journal 11(4), Artificial Intelligence 43(1) 1990 —
from ChipTest/Deep Thought: extend the move that is much better than every
alternative, detected by searching the alternatives against a null window
lowered by a margin. Follow-ups: Anantharaman's two 1991 ICCA papers; Hsu,
*Behind Deep Blue* (2002). The modern restriction CPW dates to Stockfish 1.6
(2009): only the **TT move** is ever tested — "restricted to moves found in
the TT with a lower bound flag set", later relaxed to exact bounds too.

**The verification-search form (published prose).** At a non-root node with
no exclusion already active: a TT move whose entry is deep enough —
`entry depth >= depth - 3` is the traced form (edwardyu's 2011 talkchess
writeup of the SF-inspired implementation: "depth - nBetaDepth <= 3") — with
a lower-or-exact bound, node depth at or above a threshold (SF prose: "depth
threshold 6 unless this is a PvNode... to 8", 8e823459; Cardoso 2018 quotes
"depth>8*ONE_PLY"), and a TT score that is not terminal (Weiss #756 prose:
allow SE "as long as the score from TT isn't a terminal score"). Then
`singularBeta = ttScore - margin(depth)` — the margin **scales with depth**
(SF 23cbb221, "Depth dependant singular extension margin"; flat margins
10..100 are CPW's recorded experimentation space) — and the node is
re-searched at about half depth (CPW: "depth/2" up to "depth-N"), zero window
`(singularBeta-1, singularBeta)`, **with the TT move excluded**. Every
alternative failing low means the TT move is singular: search it one ply
deeper. Extension amount 1 at every traced introduction.

**Multicut fold-in.** The same verification failing HIGH means an alternative
also clears a bar just under the TT score — Björnsson/Marsland's multi-cut
signal (CG 1998; Information Sciences 122, 2000; TCS 252, 2001: M candidate
moves, C cutoffs, reduced depth R) collapsed onto the search SE already paid
for. Ethereal d397a7b5 (2019-12): "When a singular search shows a move
besides the tt move fails high we assume it's safe to prune. ELO | 5.74
+- 4.14". Lynx: returning the verification's own fail-soft score measured
**+6.21 +/-3.46** LTC (#1751 "multicut with singularScore", merged) where
returning singularBeta itself failed (#1750, -0.84 STC, unmerged) — return
the score, not the bound — with a mate-range guard on the returned value
(#1761, merged). Stash's first SE attempt carried multicut and the retry
dropped it: "Second try to Singular Extensions, no Multi-Cut this time"
(ad8a87c6, 2020-12) — dropped at roughly the band chesso is heading for.

**Negative extension.** Verification says not-singular and the TT score
already sits at or above beta (or at/below alpha — engines differ): reduce
the TT move instead. SF prose traces the churn (e1f12aa 2022 intro,
39da50e/98dafda/a48573e 2023, c43425b 2024 simplifies one away); Ethereal
f71ac476 "+4.66" (2023, ~3400); Stash #171 +2.66/+2.05 (2024); Weiss #655
+2.66/+3.14 (2023), #781 +3.25/+3.55 (2026); Lynx #1743 **+11.99 +/-5.03**
LTC at its introduction release.

**Introduction records, banded honestly.**
- Weiss #354 (2020-09-26): **+11.52 +/-6.84** at 10+0.1, **+24.21 +/-9.83**
  at 60+0.6; shipped in v1.2 (2020-10-17 release note "Singular Extension"),
  the release DEC-087 records at 3055 CCRL Blitz — introduced just below it.
- Berserk #42 (2021-04-07): **+19.82 +/-9.34** at 10+0.1, five weeks after
  the author estimated v2.0.0 at ~2450 (CCRL board prose) — the one clearly
  sub-3000 introduction; two days later e06b444 "Disable TT and NMP on
  singular search".
- Stash ad8a87c6/2db36e4f (2020-12 / 2021-02): lands across the v27 3000
  crossing DEC-087 records; later Stash: min depth 8→7 **+7.87/+4.97**
  (dfa889e7, 2022), "do more" +2.50/+6.27 (2023).
- Ethereal aa36bc09 (2018-04, ~10.x, low-3100s): three stacked patches —
  basic SE +6.23, don't stack check extension on it +3.46, threshold 8→10
  **+12.68**.
- Lynx #1731 (2025-06): **+20.53 +/-8.48** LTC, branch named
  "se-04-7-no-tt-cutoffs"; v1.10.0 shipped SE plus double extension (#1742
  +18.33, margin 15), negative extension (#1743 +11.99), multicut (#1751
  +6.21) and a `ply < 3 * depth` cap (#1768) in one release.
- The old record: Komodo's Dailey, "one of the really big elo gains"; Hyatt
  measured no gain in Crafty (talkchess t=38104) — DEC-019's oldest case.

**The honest split.** The extension: introductions cluster from ~2500 to the
3000-3100 boundary — in-band for this plan, and plan.md's premise ("the one
feature every 3000-plus engine has") is what the record shows. Multicut:
+5.7/+6.2 at 3300-class engines, dropped by Stash at ~3000 — thin at the
band, hence the second verdict, and a zero is a live outcome. 3100+
refinements, not this step's: negative extensions, double/triple extensions
(Berserk #102 +2.90, #542 triple +6.73; Weiss #656 +11.52/+11.18; SF
16566a8f couples TT-capture LMR to it), the quiet-limit cost control
(Ethereal 464fa339 **+10.82**), ttPv singularBeta bonus (Lynx #2331 +4.95 —
needs a PV flag bit tt_entry_t does not carry, S098's ttPv wall).

**Band caution, recorded not resolved.** Lynx's own README rates v1.8.0 at
**3144** and v1.10.0 at **3293** CCRL Blitz where this repo's S098 trace and
DEC-087 place that era high-2800s / "~2850". If the README is right, Lynx's
SE block is 3150-3300 evidence, not sub-3000 — carried into the split above —
and DEC-087's Lynx-based banding (S099's "+11.4 at ~2850" included) reads
~300 high. Flagged for the owner; not this step's premise either way.

### 2. Shape for chesso

- **negamax is one function and has no excluded-move plumbing** — verified:
  signature `(alpha0, beta, depth, ply, game, state, prev_move, is_pv)`
  (src/search.hpp:47-54, src/search.cpp:411-418); no per-ply search stack,
  only per-ply arrays in search_state_t (src/data_structures.hpp:441-460).
  Published shape is a per-ply excludedMove (SF prose d6bdcec5 "(ss+1)->
  excludedMove ... reset right after singular search is finished"); chesso's
  natural form is one more parameter, `move_t excluded_move`, travelling
  exactly as prev_move does. negamax is test-callable since S103.
- **No extensions of any kind exist** (specs.md "absent, search"; S096
  retired by DEC-087), so this is the tree's first depth increase. Plumbing:
  `child_depth = depth - 1` (:676); the singular move alone searches at
  `child_depth + 1`. MAX_PLY walls carry it: negamax returns evaluate() at
  `ply + 1 >= MAX_PLY` (:424), quiescence stand-pat at :260; MAX_PLY 128,
  MAX_DEPTH 126 (data_structures.hpp:42-43). An always-extending path
  terminates on the ply wall by construction; the published cap that keeps
  the wall theoretical is Lynx's `ply < 3 * depth` (#1768).
- **TT entry fields suffice** — verified at src/data_structures.hpp:388-414:
  `depth` int16_t, `type` uint8_t (TT_BETA_NODE = lower, TT_PV_NODE = exact,
  :360-378), `score` int32_t, `best_move`; probe :445, `tt_move` copied out
  :449. No change to the 24-byte layout is needed for V1/V2.
- **The score read is a new de-normalize site.** entry->score is stored
  normalized (:813); SE bypasses tt_entry_answers (it wants the value, not a
  cutoff), so it calls `de_normalize_score(entry->score, ply)` (:139-144)
  itself, then requires `|tt_score| < MATE_MIN` before deriving
  singularBeta — S106's round-trip lesson applied at the new reader.
- **TT while excluding, published practice:** at the excluded node take no
  TT cutoff (Berserk e06b444 "Disable TT and NMP on singular search"; Lynx's
  merged branch literally named "no-tt-cutoffs") and **write no store** (SF
  ebe021f6, "Don't update TT at excluded move ply") — gate :457-463 and
  :813-815 on `excluded_move == 0`. The subtree below probes and stores
  normally in every traced writeup. The alternative — hashing the exclusion
  into the key so the verification owns a separate entry — is known practice
  whose open prose this pass could not trace: **unknown, not taken**
  (DEC-084 caution). Weiss #600 (SMP probe rule) and #644 (skip TB) are out
  of scope here.
- **Also suppressed at the excluded node:** NMP (same Berserk prose — a
  null-move bound would answer the verification with no alternative
  searched; gate it on excluded_move directly, never by abusing the
  `prev_move != 0` gate at :569, since prev_move must keep flowing for
  countermoves/S024) and SE itself (`excluded_move == 0` in the conditions —
  no recursive exclusion, the published rule). Whether RFP (:511) needs
  suppressing too has no traced prose: a static fail-high there answers
  "not singular" without any move searched — decide with a test, record
  which.
- **The move loop under exclusion:** skip `moves[i] == excluded_move` before
  make_move and before legal_moves_counter++ (:656-663). score_move still
  ranks the excluded move first — one wasted pick, harmless. If no legal
  alternative exists, :779-781 returns mate/draw: the mate side reads as
  fail-low (singular — correct, it is the only legal move); the stalemate
  DRAW_SCORE side reads as fail-high when singularBeta <= 0 — no traced
  prose, decide and pin with a test.

### 3. Implementation sketch

Two verdicts off one verification search, as the accepts prices: the
extension first, the multicut second, each its own SPRT.

**V1 — verification search + extension:**
1. Thread `excluded_move` (default 0) through negamax; the three gates above
   (no TT cutoff, no store, no NMP, no SE while excluding).
2. Between the NMP block and move generation (a null-pruned node then never
   pays for verification): when `ply > 0 && excluded_move == 0 &&
   depth >= SE_MIN_DEPTH && tt_move != 0 && tt_entry != nullptr &&
   tt_entry->depth >= depth - SE_TT_DEPTH_MARGIN && (type is BETA or PV) &&
   |tt_score| < MATE_MIN && ply < SE_PLY_FACTOR * depth`:
   `singular_beta = tt_score - se_margin(depth)`;
   `vscore = negamax(singular_beta - 1, singular_beta, (depth - 1) / 2, ply,
   game, state, prev_move, false)` with excluded_move = tt_move; check
   `state->aborted` before using vscore (:729's pattern).
   `vscore < singular_beta` → `se_extension = 1`.
3. In the loop: the searched depth for `moves[i] == tt_move` becomes
   `child_depth + se_extension`; the LMR clamp (:697) and S098-V3's deeper
   cap follow the extended child depth — the edit S098 §5 assigns here.
4. Tests, red first: **exclusion unit test through the negamax seam** — a
   tool-built mate-in-1 with exactly one mating move (python-chess
   enumeration + Stockfish confirmation, S033 protocol, DEC-023): with
   excluded_move = the mating move negamax must not return a mate score;
   with excluded_move = another legal move and with 0 it must — excludes
   exactly the given move. **Condition tests per the accepts**, each by
   planting entries with tt_store_entry (public) and failing if the
   precondition is absent: root never verifies; an entry shallower than
   `depth - SE_TT_DEPTH_MARGIN` never does; a TT_ALPHA_NODE bound never
   does — observed via node counts moving only when the condition holds.
   Both mate cases re-run -- "pruning does not hide a forced mate",
   tests/test_search.cpp:1887 and "pruning does not hide a mate against the
   material leader", tests/test_search.cpp:1926; fast suite. SPRT.

**V2 — multicut:**
1. `vscore >= singular_beta && vscore >= beta && |vscore| < MATE_MIN &&
   !is_pv` → return vscore. Fail-soft score, not singularBeta (Lynx #1751
   vs #1750); mate guard is #1761's; the !is_pv gate is the house pattern
   for bound-returning prunes (RFP :511, NMP :569) — direct prose untraced,
   stated as a choice in the commit.
2. The accepts' mate case: a forced mate inside the multicut's pruned depth
   added beside "pruning does not hide a forced mate",
   tests/test_search.cpp:1887, observed red with the mate-range guard
   removed.
3. SPRT; a random walk near +3 is terminated and recorded as zero (DEC-063),
   and dropping the multicut while keeping the extension is the default at
   zero (S005/S006/S015 precedent).

### 4. Constants and seeds

All in the src/search_params.hpp X-macro (:41) with stated ranges; every
number is a **seed — must be fitted/SPSA'd here** (S127, DEC-084).

- `SE_MIN_DEPTH` **8**, range 4..16 (SF prose 8e823459; Ethereal 8→10
  +12.68 up, Stash 8→7 +7.87 down, Weiss #639/#641 lower still at ~3300 —
  direction is engine-dependent, sweep).
- `SE_TT_DEPTH_MARGIN` **3**, range 0..8 (edwardyu 2011 prose,
  "depth - nBetaDepth <= 3").
- `SE_MARGIN_PER_DEPTH`: form `margin = SE_MARGIN_PER_DEPTH * depth` scaled
  to land a few tens of cp at depth 8 — **no publishable coefficient**
  (engine values are source; CPW's flat 10..100 experimentation range is the
  open record). Off value: range top (never singular).
- Verification depth: `(depth - 1) / 2` as the shipped form; CPW publishes
  "depth/2 to depth-N" as the space — if parameterised, `SE_VDEPTH_SUB`
  with the halving stated, else record the fixed form in the commit.
- `SE_PLY_FACTOR` **3**, range 2..8 (Lynx #1768 title prose).
- Multicut adds no constant beyond the guards; a return margin is S127-era.
- Anti-seeds, measured negative elsewhere: returning singularBeta from
  multicut (Lynx #1750 failed where #1751 passed); stacking a check
  extension on a singular one (SF 30c58320 prose; moot here — none exist,
  DEC-087); extending more than 1 without the double-extension guards
  (Lynx caps doubles at 6 per line, #1777).

### 5. Pitfalls

- **Search explosion is the published hazard, not a hidden mate** — the
  extension only adds depth. Caps: +1 once per node, no recursive exclusion,
  `ply < SE_PLY_FACTOR * depth`, the MAX_PLY walls (:424, :260). §6's
  fixed-node depth check is the instrument that catches a blowup before an
  SPRT spends a night on it.
- **TT pollution from the verification:** its result is computed with the
  best move removed — stored, it poisons every later probe of the position.
  The no-store gate (:813-815) is SF-prose-backed (ebe021f6); the no-cutoff
  gate keeps the entry from answering its own verification (the entry's
  lower bound >= singularBeta would multicut every time, vacuously).
- **Mate scores.** De-normalize before deriving anything (S106's lesson;
  tt_entry_answers does it at :170 for cutoffs, this is a second reader);
  `|tt_score| < MATE_MIN` gates entry; singularBeta itself must stay out of
  the mate band (Lynx #2559/#2560 — the merged guard measured ~0 but exists
  to stop false mate reports); multicut never returns a mate-range value
  (#1761). MATE_MIN/MATE_MAX are :16-17.
- **IIR (S095, lands before) is disjoint by construction** — IIR fires on
  `tt_move == 0`, SE requires `tt_move != 0`; inside the verification node
  the probe still finds the entry, so IIR stays off there unless the
  implementation masks tt_move under exclusion — do not. Subtree records
  both ways: Berserk #91 +3.64 disabling IIR during SE, Lynx #1734 +0.09
  ~0 — no edit owed by default; S127 revisits.
- **S109's pruning stays live inside the verification** — pruning the quiet
  tail biases toward "singular", and the published record treats that as a
  feature: Ethereal deliberately limits quiets tried to disprove singularity
  (+10.82). No suppression owed; the SPRT prices the bias.
- **A collided or stale tt_move** not in the generated list: the exclusion
  matches nothing, the verification degenerates to a half-depth re-search
  and the extension never fires — harmless but wasted; gating on the move
  appearing in the list is one comparison if the waste shows in profiles.
- **The abort path:** vscore from an aborted verification is garbage —
  check state->aborted immediately (the :729 pattern) and take no decision
  from it.
- **The repo gate:** both mate suites re-run per verdict (CLAUDE.md: any new
  pruning gets the mate treatment before it is called done — multicut is
  pruning); tune-build strength numbers forbidden (S073).

### 6. Measurement

Two SPRTs at the S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`,
V1 then V2, each against the commit before it; fast suite plus both mate
cases green first, new tests observed red first with printouts recorded.
Node counts move by construction — INV-6 takes the SPRT path both times;
search_bench depths 9/12 node counts recorded per verdict in the stamp.
**Node-explosion check recorded per verdict:** depth reached at fixed
`go nodes 1000000` over the three search_bench positions, before and after —
SE trades nodes-per-depth for time-to-mate-class-information; a falling
fixed-node depth with no SPRT gain is the explosion signature. Expectations
(DEC-019, direction only): V1 +10..+20 (Weiss +11.5, Berserk +19.8 at
10+0.1; Lynx +20.5 LTC); V2 0..+6 (Ethereal +5.7, Lynx +6.2 LTC, Stash
dropped it at ~3000). A stalled V2 near +3 straddles the {0,5} bounds
(DEC-063): terminate, record zero, drop by default.

### 7. Interactions

- **S095 (before):** node conditions disjoint (tt_move == 0 vs != 0); IIR
  inside the verification is off by construction; records both ways carried
  in §5; no edit owed.
- **S098 (before):** the LMR clamp and the V3 deeper-cap follow the extended
  child depth — the edit S098 §5 assigns to this step; SE needs nothing from
  its cut_node plumbing for V1/V2.
- **S099 (before):** correction history runs inside the verification
  unchanged — Lynx #1844 "Avoid correcting on SE verification" measured
  +0.49 ~0; nothing owed now.
- **S109 (before):** its prunes run inside the verification subtree and
  against `singular_beta - 1` at the excluded node; kept live (§5).
- **S113 (after):** ProbCut also runs reduced verification-like searches but
  asks the opposite question — a raised bar (`beta + margin`) over captures
  to prune the node, where SE asks a lowered bar (`ttScore - margin`) over
  the alternatives to classify one move; multicut is where the two answers
  meet. Distinct machinery; no sharing owed.
- **S127:** every SE constant refit; the deferred refinements priced there
  or as a follow-up step — negative extensions, double/triple extensions,
  the quiet-limit, ttPv bonus (blocked on an entry flag bit).
- **S130 (before):** unaffected — the verification's no-store keeps the
  stand-pat TT reads it added clean.

### Scope concerns

1. **The Lynx band discrepancy** (§1): Lynx's README claims 3144/3225/3293
   CCRL Blitz for v1.8.0/v1.9.0/v1.10.0 where DEC-087 and S098's trace used
   high-2800s/"~2850" for that era. S097's case survives either reading
   (Weiss, Berserk, Stash carry the band evidence), but DEC-087's
   Lynx-anchored banding — S099's headline number included — may sit ~300
   low. Owner's to re-read; recorded here because this pass found it.
2. **Negative and double extensions measured +11.99 and +18.33 LTC at
   Lynx's introduction release** — large enough that a follow-up step (not
   a smuggled third change; the accepts prices exactly two verdicts) is
   worth creating if V1 passes. Routed to S127-era or a new step id.

### 8. References

- https://www.chessprogramming.org/Singular_Extensions — origin papers, Stockfish 1.6 lower-bound restriction, exact-bound relaxation, margin/depth experimentation space.
- https://www.chessprogramming.org/Multi-Cut — Björnsson/Marsland papers, M/C/R parameters, the SE fold-in as modern practice.
- Anantharaman, Campbell, Hsu 1988/1990; Anantharaman ICCA 14(1)/14(2) 1991; Hsu, Behind Deep Blue 2002 — cited via CPW's reference list, not fetched.
- https://api.github.com/search/issues?q=repo:TerjeKir/weiss+singular+type:pr — #354 intro +11.52/+24.21; #639/#641 lower depths; #655 negative +2.66/+3.14; #656 double +11.52/+11.18; #600 SMP TT probing; #644 skip TB; #756 terminal-score condition; #781 negative-for-cutnodes +3.25/+3.55.
- https://api.github.com/repos/TerjeKir/weiss/pulls/354 — "Code inspired by SF, Eth and many other engines"; the two SPRT blocks.
- https://api.github.com/repos/TerjeKir/weiss/releases — v1.2 2020-10-17 "Singular Extension"; v1.1 2020-09-02.
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+singular+type:pr — #1731 intro +20.53 LTC ("se-04-7-no-tt-cutoffs"); #2331 ttPv bonus +4.95; #2559/#2560 singularBeta mate guards.
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+SE+in:title+type:pr — #1737 no check-ext in verification +1.14; #1734 IIR-during-SE +0.09 unmerged; #1844 no correcting on verification +0.49; #2423 double negext on cutnode +1.73.
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+multicut+type:pr — #1751 singularScore +6.21 merged; #1750 singularBeta -0.84 unmerged; #1752 reduce-instead +0.78 unmerged; #1761 mate guard; #2450 fail-firm -0.75 unmerged.
- https://api.github.com/repos/lynx-chess/Lynx/releases/tags/v1.10.0 — the SE block shipped whole: #1731/#1742/#1743/#1751/#1761/#1768/#1777.
- https://api.github.com/repos/lynx-chess/Lynx/issues/1742 and /1743 — double ext +18.33 (margin 15), negative ext +11.99.
- https://raw.githubusercontent.com/lynx-chess/Lynx/main/README.md — the version/rating table behind the band caution (1.8.0=3144, 1.10.0=3293).
- https://api.github.com/search/commits?q=repo:jhonnold/berserk+singular — b0eea29 #42 intro; e06b444 #45 "Disable TT and NMP on singular search"; 60bcdd6 #102 double +2.90; 0e42746 #542 triple +6.73; 8cae7ac #424 negative reductions +2.13; 1b95cfe #198 eval reuse +1.22.
- https://api.github.com/repos/jhonnold/berserk/pulls/42 — +19.82 +/-9.34 at 10+0.1, 2021-04-07.
- https://kirill-kryukov.com/chess/discussion-board/viewtopic.php?t=12771 — Berserk v1.2.0 ~2000, v2.0.0 ~2450 author estimates (2021-02/03): the introduction band anchor.
- https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+singular — ad8a87c6 "no Multi-Cut this time" 2020-12; 2db36e4f retry 2021-02; dfa889e7 depth 8→7 +7.87/+4.97; c9238d02/df1e1c9b more SE +2.50/+6.27; 07b3c88e negative +2.66/+2.05; 343ed1c5 skip known win/loss +5.73; c34419cc quiet 3-ply 2025.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+singular — aa36bc09 2018-04 intro (+6.23/+3.46/+12.68); d397a7b5 multicut +5.74; 464fa339 quiet limit +10.82; f71ac476 negative +4.66.
- https://api.github.com/repos/AndyGrant/Ethereal/releases — 11.16 "apply Singular Extensions aggressively"; 11.38 quiet limit; 12.08 "singular moves mistaken as MultiCut moves" fix; 13.76 singularity() simplification.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+excluded — ebe021f6 2017 "Don't update TT at excluded move ply"; d6bdcec5 excludedMove reset prose; 5bec768d/25c22ffe 2009 tte/ttMove condition prose.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+singular — 23cbb221 depth-dependent margin; 8e823459 threshold 6/8 prose; 30c58320 check-ext exclusivity; 16566a8f capture coupling; 4d0981fe mate-position revert; b34a690c/b1f52293 singular result reused by MCP.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22negative+extension%22 — e1f12aa 2022 intro; 39da50e/98dafda/a48573e 2023; c43425b 2024 simplify-away; c5aef2b 2026.
- https://talkchess.com/forum3/viewtopic.php?t=38104 — edwardyu 2011: skip hashmove, sing_beta = value - margin, depth>=6, entry within 3 of depth; Dailey (Komodo "really big"), Hyatt (Crafty none).
- https://www.talkchess.com/forum3/viewtopic.php?t=68290 — once-per-line/explosion discussion; Cardoso: "depth>8*ONE_PLY", node-count cost.
- Unreachable this pass: ccrl.chessdom.com (DNS), computerchess.org.uk (403) — Weiss 1.1 / Berserk 3.x-4.x blitz numbers untraced, said so above.
