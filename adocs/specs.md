# Specs

What the software must do. Precedence: specs beat plan beat status. Code that
disagrees with this file is a bug or an unrecorded decision.

## What is being built

**The end goal: the strongest CPU chess engine in the world, built by
AI-driven development — open source under the MIT licence, and it stays MIT
(DEC-104).** Nothing is copy-pasted from another open-source project. The
techniques are bleeding edge: taken from the literature and research, invented
on the spot, or adapted — never copy-pasted — from other open-source engines
where their licence consents. The evaluation end-state is an own NNUE, or
whatever supersedes it, trained by this project on its own data — parked while
the 3000 mark is pursued without a network (DEC-054, DEC-071). Every change is
decided by rigorous testing — SPRT and the other modern methods — with
specialized tools taken where they exist and built ad hoc where they do not,
for training, fine-tuning and testing. "CPU engine" names the arena: it plays
on CPUs, and a GPU may serve training, never play.

Built on the `achesso` branch — *agentic chesso* — to find out what AI-driven
development can produce. It is founded on the owner's own bitboard engine, its
test framework and its fastchess SPRT scripts, and on nothing else. DEC-013.

Phase one is to reach the level the published literature describes, by reading
documented technique and implementing it here. Phase two is to experiment.
`adocs/plan.md` is phase one. DEC-014.

**Measured strength, 2026-08-18: chesso is approximately 2559 on the CCRL Blitz
scale, 95 % ±25, and the figure is soft.** 3340 games against **five** rated
engines from **three families**, solved with `ordo` anchored on each in turn.
`adocs/data/rating_2026-08-18_S088_ccrl_blitz.md` is the record and `./rating.sh`
re-derives it. S088, DEC-072 and DEC-075 to DEC-077.

Soft because the reference set disagrees internally by **121.8 Elo**, four times
the 30 the procedure allows; approximate because the games were played at
`10+0.2` rather than the list's "equivalent to 2'+1" on an i7-4770K". S087
measured 2570 over four engines and two families (still recorded in
`rating_2026-08-18_ccrl_blitz.md`), and **`src/` is byte-identical between the
two runs** — the instrument changed, the engine did not.

**Adding a third family did not close the spread; it identified it.** With
Leorik 2.1 set aside the solved rating rises monotonically with the anchor's own
rating and flattens at the top, so the disagreement is scale compression rather
than one misrated engine: across a CCRL span of 440 Elo the measured differences
span 375.3, a ratio of 0.853. No reference set fixes that. The leading candidate
is the time control, and attacking it is nobody's step yet. DEC-077.

**The target is at least 3000 on that scale, and it is pursued without a
network.** DEC-071, 2026-08-18, the owner's decision. It is a goal and not an
invariant: nothing fails a test for being below it. Reachability was checked
against the same list, single-CPU entries read 2026-08-18 -- Stockfish 11 at
3565, Komodo 14.1 at 3482, Xiphos 0.6 at 3356, Ethereal 11.75 at 3346 and six
more above 3130, every one a hand-crafted evaluation one version below that
engine's first network -- and against the Leorik 2.x line on this machine, which
carries no network file at any tag and reaches 2917. DEC-054 stands: S029 is
parked and NNUE is not reopened. `./rating.sh` is re-run **when substantial work has been
done to the engine**, which is the owner's judgement and not a threshold an
agent derives (DEC-074, superseding DEC-071's "any landed step an SPRT credits
with 20 Elo or more"). Per-change decisions stay with the SPRT. The gauntlet
still has to be re-read sometimes, because every other figure here is self-play
against an earlier chesso and the factor between the two scales is not yet
measured; it costs about 5 hours a time since DEC-073.

## Prime directive

Chesso never plays or accepts an illegal move and never corrupts its own board
state; a reproduced correctness defect outranks every strength, speed and
feature item on the plan.

## Invariants

Numbered, testable properties. Referenced by number from code comments, test
names and commit messages. Each one has a row in `testing.md`.

- **INV-1 Move generation is legal-only and exact.** Perft counts match an
  independent oracle at every tested position and depth. A generator that loses
  moves is faster and wrong, so node counts are checked before any timing is
  printed.
- **INV-2 `make_move` and `unmake_move` are exact inverses.** Bitboards,
  `squares[64]`, hash, castling rights, en passant and the evaluation
  accumulators after an unmake equal their values before the matching make.
- **INV-3 `generate_captures` and `generate_quiets` partition
  `generate_moves` exactly.** Same multiset, no overlap, no quiet move in the
  capture list.
- **INV-4 The incremental evaluation accumulators equal a full recomputation.**
  `material`, `psqt_mg`, `psqt_eg` and `phase` are maintained by `make_move`;
  they must agree with rebuilding them from the bitboards, at every node.
- **INV-5 `evaluate()` is side-to-move relative.** Positive means the side to
  move is better. Callers apply no sign, and mirroring a position mirrors the
  side to move, so the score agrees rather than negates.
- **INV-6 A change is retained only against a measurement.** A change claimed
  behaviour-neutral proves it with identical node counts and identical best
  moves from `tools/search_bench.py`, and **is not sent to a match**: an SPRT
  cannot resolve a speed-up below about 0.24 % at short time control and spends
  a night saying so, so the strength claim is the interleaved timing converted
  at the published 1.43 Elo per percent of nps at long time control and 2.10 at
  short, named as a conversion and never as a verdict (2026-08-19, DEC-083). A
  change that alters play is retained only with an SPRT verdict against a named
  commit, and a verdict of zero is recorded as zero. (2026-08-13, S037: the
  count the tool reads is the whole search's, cumulative over every iteration,
  so the identity covers the whole tree.
  Before S037 `info nodes` was the current iteration's own count and the tool
  compared the final iteration alone — about half the search — so a change that
  altered depths 1..n-1 could pass. Every node figure recorded from the tool
  before that date is a sum of last iterations.)

**The numbering ends at INV-6 and there is no INV-7.** One commit cites one --
`68a61d0`, *"Restore S084's landed step file (INV-7)"*, 2026-08-20 -- and it is
the only record of the number anywhere outside git. What it enforced is a
workflow rule rather than a property of the engine: its body is *"plan_done/ is
append by move only"*, which is `AGENTS.md` section 2 (*"`adocs/plan_done/` is
never rewritten or trimmed, and this is enforced: it is the project history"*)
and section 10's hard prohibition on writing there. Read the commit as citing
those two. The number is not allocated to it, because an invariant here is a
testable property of the software with a row in `testing.md`, and a rule about
which directories an agent may write to is neither. (2026-08-21, S140, closing
`2026-08-20_plan_review-F17`.)

## Behaviour

Chesso is a UCI engine. The protocol surface is the product surface, which is
why `surface_guard` is `cli`; `MANUAL.md` documents it and S017 makes it
checkable.

There is one build that is not the product. `-DCHESSO_TUNE=ON` turns the
parameters in `src/search_params.hpp` from constants the compiler folds into
variables settable over UCI, and adds one spin option per parameter. **No
strength number is ever taken on it**: a constant that folds is not the same
code as a variable that must be loaded, and the difference is a timing rather
than a node count. The release build's option surface is the three lines it has
always had, the two builds' defaults are held equal member by member by
`test_search_params`, and `tools/search_bench.py` reports the same counts on
both with no `setoption` sent. (2026-08-16, S073. The parameter count is
deliberately not written here: it moves with every step that adds one, and
`test_search_params` is where it is pinned.) **The declared ranges are pinned
there too, since S142**: only the tune build reads a bound, so until then a
bound could move -- or fail to move when its reason did -- with the whole suite
staying green, and two of them had (2026-08-20\_plan\_review-F08 and -F14).

**A `setoption` naming a search parameter the tune build cannot honour answers
with one `info string` line**, naming the parameter and its range for a value
outside it or for a value that is not an integer in full, and naming the name for
an option the build does not have. A legal value prints nothing, and the release
build prints nothing in any of the three cases. `Use Book`, `Hash` and `Threads`
are outside this: their handlers are the release build's, so a bad value for one
of them stays log-only in both builds. Before S137 all three were silent -- the
refusal went to a macro that compiles to nothing under `NDEBUG` and `build-tune`
is a Release build -- so a tuner could spend a night playing games against a
compiled default and read it as success. There is still no readback: `uci`
re-prints each parameter's compiled default, not its live value.
(2026-08-20, S137, DEC-093.)

**The binary that ships is not the binary that gets measured here, and both are
new since 2026-08-19.** `-DCHESSO_ARCH=` has four values: **`bmi2`
(`-march=x86-64-v3`) is the one that ships to a rating list**, `avx2` is the same
level with `-mno-bmi2` for Zen1 and Zen2 where PEXT is microcoded, `portable` is
`-march=x86-64-v2`, and `native` is the default because `build/` is the directory
that gets measured. `build_release.sh` refuses `native` by name: a binary built
for one machine raises SIGILL on an older one. Until S104 the release build
carried **no** architecture flag at all, so `std::popcount` compiled to a software
SWAR popcount — `objdump -d build/src/chesso | grep -c popcnt` returned **0** at
`20d058a` and returns **159** now, **125** in a profile-guided release target.
`build_release.sh` adds profile-guided optimisation on top, trained on a
fixed-depth search over 400 positions stratified by the engine's own
`game_phase()`, and the profile is regenerated per build and never committed.

**Behaviour-neutral, so INV-6 is discharged on node counts and no SPRT is owed
(DEC-083).** All four binaries return `164123 / 670488 / 84351` at depth 9 and
`1162576 / 5167100 / 683367` at depth 12, best moves `c3d5 / e2a6 / d7c8q`,
identical to `20d058a` to the node. **The shipping binary is +18.22 % faster**,
95 % CI +16.19 to +20.28, ratio 0.8459 geometric mean over one interleaved run of
10 rotating triples, paired t −21.90 — **+26.1 Elo at the published 1.43 per
percent, which is a conversion and not a verdict.** The architecture flag is
+12.62 % of that and the profile the remaining +4.98 %. **Measured on 400
positions the profile was not trained on**, because the same run over the training
positions reads +18.65 % and timing a profile-guided binary on its own workload is
not a measurement of it; the 0.43-point gap between the two is how small the
overfit is.

**Every timing and nps figure recorded before this step was taken on a binary
with no architecture flag and is not comparable with one taken after it**, the
same way DEC-049's machine move invalidated everything before it. Node counts are
unaffected and stay comparable. The new baselines on the native `build/`:
`bench_eval` **53.90 ns a call, 18.6 M calls per second**, against 83.35 and
86.41 ns on 2026-08-19 before the flag; `bench_movegen` **814.3 ms** for 12 M
perft nodes, resolution 0.2 %. (2026-08-19, S104.)

Engine state as of 2026-08-09, at commit `b6ef5c4`:

| area | state |
|---|---|
| move generation | legal-only, templated `<Color, Constrained, Type>`, ~90 Mnps perft |
| `generate_captures` / `generate_quiets` | partition `generate_moves` exactly (INV-3) |
| board | `board_t` 216 B: bitboards, `squares[64]`, evaluation accumulators |
| position input | `load_FEN` validates syntax **and the two semantic classes that corrupt** since 2026-08-22, S161. A castling right whose king or rook is not on its square, and an en-passant square that is on the wrong rank for the side to move, has no enemy pawn on the victim square, or has an occupied target square, are **cleared and the position is accepted** -- not rejected, because GUIs, books and conversion tools send stale fields and the match harnesses sanitize rather than refuse. Both clearings happen after the fields are parsed and before `compute_full_hash()`, so a sanitized position hashes as the position it actually is. This is prime-directive work and not a strength item: the generator emits castles from the hard-coded `e1`/`e8`, `castling_rook()` answers from the king's target square rather than from the board, and every piece update is an xor -- so an unsupportable field did not produce an illegal move, it **created pieces**, silently in the Release build that ships and is measured. Perft (INV-1) could never see it, every perft FEN being semantically valid. Legality at large -- pawn counts, two kings, the side not to move in check -- is deliberately still unchecked. Behaviour-neutral on every valid FEN and discharged on node counts: 121512 / 800769 / 62907 and `c3d5` / `e2a6` / `d7c8q` at depth 9, identical to `0a274c9` (INV-6). `2026-08-22_adversarial-F01` |
| make/unmake | 16-byte history record, undo by xor, no board copy |
| evaluation | material, tapered piece-square tables, mobility, king safety, passed pawns and pawn structure. The first two are maintained incrementally (INV-4). Mobility and king safety are recomputed behind a lazy shortcut that skips them when the cheap score is a margin clear of the window, and their sum is clamped to that margin so the bound holds by construction (2026-08-11, S034; 2026-08-12, S027). The three pawn terms are recomputed in the cheap stage instead, sharing four bitboard fills, because the clamp would truncate them on the positions they exist for (2026-08-12, S027). **All 817 shipped constants fitted to chesso's own self-play outcomes**, each term fitted with every other constant frozen so its SPRT measured one change. The corpus they are fitted to is **deduplicated by the engine's own zobrist key**, one row per distinct position: 207998 of 11003693 rows, 1.8903 %, were repeats carrying repeated weight, and the refit without them measured `Elo: 26.68 +/- 16.40`, H1 accepted over 1044 games against `elo0=-5 elo1=5` -- the pre-registered claim being not a regression of 5 or more with the sign positive at `LOS: 99.93 %`, since an early stop biases the point estimate upward (2026-08-17, S076; DEC-065) |
| search | alpha-beta, transposition table, quiescence, PVS, aspiration windows, null move pruning, late move reduction, reverse futility pruning, staged generation, killers, history, countermoves, insufficient-material draws. **A node whose halfmove clock has reached 100 scores a draw only once checkmate has been ruled out since 2026-08-22, S162**: FIDE 5.1.1 ends the game the instant checkmate is delivered and 9.6.2 states the same exception for the 75-move rule in so many words, so the clock test runs `is_check()` and one move generation before returning `DRAW_SCORE`, and a mated node falls through to the two sites that already know the distance -- `negamax`'s own no-legal-move return and quiescence's -- rather than computing a second copy of that arithmetic. Neither call is paid at a node whose clock has not reached 100, which is every node any benchmark visits. What it cost before: on `7k/6pp/8/8/8/7n/6P1/R6K w - - 99 60` at depth 1, where the clock reaches 100 on the mating move, the engine reported `cp 448` and played `g2h3`, discarding a mate in one it had already generated -- stockfish scores the same position `Mate(+1)` and python-chess reports the child mate at a clock of exactly 100. The insufficient-material test immediately below cannot intercept a mated node: over all **1700560** legal positions it calls a draw -- two kings with at most one knight or bishop, either side to move -- exactly **0** are checkmate, enumerated rather than argued. Bench node counts and best moves are identical to `ecd735e` -- 639228 / 3430710 / 367858 at depth 12, `c3d5` / `e2a6` / `d7c8q` -- but that is not the whole of INV-6 here, because it only says no bench FEN approaches the boundary. **What carries it is a census over the games, and the insurance SPRT was killed on it (DEC-107)**: over 3356 games at 8+0.08, 102 peaked at a halfmove clock of 80 or more, so the branch does execute in about 3 % of games and 55 reached clock 100 on the board -- and the positions that were checkmate at a clock of 100 or more numbered **0**, at 90 or more also 0. The run had reached `LLR 0.09` in 3304 games, 2.9 % of the way to a bound, so its only reachable ending was the null its own accepts had pre-declared. `adocs/data/S162_clock_census.py` is the count and `adocs/plan_done/S162_fifty_move_mate_precedence.md` is the ledger. Reverse futility returns a static lower bound instead of searching and therefore cannot see a mate; it is **skipped at plies 0, 1 and 2 and applies from ply 3 down, with a remaining depth of 15 or less**, and five guards that were meant to make it mate-safe measured red or inert (2026-08-16, S033). Both bounds were SPSA-tuned in S085, the depth one from 6, and at 15 the assumption is made at every depth this engine reaches -- median 11 at the tuning control -- so the depth bound has stopped being a guard and `RFP_MIN_PLY` is the one that carries it. **S145 measured what that costs and what the floor is worth**, on a constructed set of **48 forced mates** spanning mate distances two to five so the guarded defender nodes land at plies 1, 3, 5 and 7, each proved by exhaustive AND/OR enumeration and corroborated by stockfish, neither of them chesso (`adocs/data/S145_mate_set.py`). The reading is almost entirely a function of the mate distance: at the shipping defaults the engine reports **16 of 16 mates in two with no delay, 8 of 16 mates in three, and 0 of 8 at both four and five**, and no setting of either bound produces a false mate -- 0 wrong signs and 0 distances shorter than the proved minimum over twelve settings. The floor separates cleanly: `RfpMinPly` 2 and 3 are identical at 24 of 48 with every mate in two immediate, while 1 and 0 -- the same engine, the root being exempted by `!is_pv` -- drop to 19 of 48 and delay a mate in two by up to seven iterations. The deep classes are recovered by the *ceiling* and not by the floor, 4 of 8 and 3 of 8 at `RfpMaxDepth` 0 against 0 and 0 at 10 and above, so S085's move of that bound from 6 to 15 is the trade to re-measure and S148 is where it is. Its margin is **63 centipawns per remaining ply**, cut from the 75 S068 measured and the 100 S033 shipped unfitted: 14.5 % fewer nodes at depth 9 and, pooled over 11348 games, a small positive effect of about 3 to 5 Elo and not a regression of 5 or more (2026-08-17, S068; the bounds lesson is DEC-063). Aspiration windows search the root of each iteration from depth 5 in a band **50 centipawns** either side of the previous iteration's score, doubling the failing side alone and going to the full window past 400; the schedule was chosen on node counts over 300 positions in three samples, where it costs 0.9248 of what the feature switched off costs and nothing else swept is below 0.9439, and one sample alone would have chosen a worse one. **H1 accepted over 824 games** against `elo0=-5 elo1=5`, `Elo: 35.12 +/- 19.06` -- the point estimate is biased upward by the early stop, so what the run establishes is the pre-registered claim, not a regression of 5 Elo or more, with the sign positive at `LOS: 99.99 %` (2026-08-17, S021) |
| exchange evaluation | `see()` exact, `see_ge()` fast; quiescence declines losing captures |
| time management | the clock becomes an allocation for this move -- `remaining / movestogo` with a `movestogo`, and **5 % of what is left plus half the increment** without one -- and the allocation becomes two limits: a soft one at 60 % that decides whether to begin another iteration, and a hard one at 300 % that a timer is armed at and that stops the search inside an iteration. The hard limit is clamped to `remaining - MOVE_OVERHEAD_MS` and floored by the S036 rule, and nothing downstream raises it; the soft limit is clamped to it. The soft limit is then scaled after each completed iteration: **4 % off per consecutive iteration with an unchanged best move, up to 8**, and up to **+50 % for a score that has fallen 100 centipawns or more since the previous one**, floored at 30 %. A time the GUI named with `go movetime` is not scaled. **`DEFAULT_MOVES_TO_GO` is gone**: a sudden-death control no longer divides the clock by a move count nobody sent. The nine constants are in `src/search_params.hpp` (S073) and the base allocation is deliberately the number the old formula produced at `movestogo 20`, so the SPRT measures the split and the scaling rather than a re-tuned base. **SPRT H1 accepted** at `elo0=0 elo1=10 alpha=0.10 beta=0.10`, 500 games in 21 m 59 s against `1069cc6`, `Ptnml(0-2) [13, 44, 94, 63, 36]`, and **0 time forfeits in the 500** -- checked from the PGN filtered to this run, because the fastchess log is WARN-only and was empty. The point estimate `+45.42 +/- 23.47` is **not** the effect size: an SPRT stops early exactly when the observed effect has run favourable. Recorded verdict: not a regression, sign positive (2026-08-18, S089; DEC-071) |
| absent, evaluation | bishop pair, rook on an open file, rook on a half-open file, rook on the seventh, tempo — all five present in the code, all five shipped at zero weight. At zero the compiler deletes them, so they cost nothing; the tuner still carries their 10 parameters, making 827 fitted and 817 shipped. **The reason is not "they measured zero", and S100 is what corrected that wording** (2026-08-20). Four of the five — the pair and the three rook features — share **one bundled SPRT**, −5.48 +/− 11.46 over 2284 games at `--fast` bounds `elo0=0 elo1=10`, and were then zeroed **by hand**; published figures for the parts are +8.2 for the pair and +9.86 for a rook file retune, which those bounds cannot resolve (DEC-063, DEC-084). Tempo **reached neither bound** — −0.69 +/− 9.64, LLR −1.46 against −2.20 over 3000 games — which is unresolved and was recorded as unresolved at S027. And every fit since S065 carries `--freeze tempo,piece_placement` (DEC-057), so all ten parameters have been **held** at zero rather than fitted to it. What S100 did exclude, for all five: a wrong feature (10795695 rows re-extracted, 0 disagreements), a wrong gradient (worst 4.9e-8 against finite differences over all 827 parameters), a pipeline that cannot recover a known vector (18 of 22 identified parameters exact, train error 0.000000), and a coverage desert (every column non-zero on 5.10 % to 31.38 % of rows). Two of the six symptoms it diagnosed are **exact algebraic degeneracies and their split from the tables is not identified at all**: rook-on-the-seventh against `psqt[rook][8..15]` and passer bucket 5 against `psqt[pawn][8..15]`, both R² 1.000000 with 0 violations in 1264773 and 550880 non-zero rows, which is why `passed_pawn_mg[5]` reads −17 here, −1 in S065's frozen fit and +22 in its unfrozen one while every other bucket moves by at most 4. Mobility arrived at S034, king safety, passed pawns and pawn structure at S027 |
| absent, search | forward futility, razoring, late move pruning, history pruning, SEE pruning outside quiescence, extensions of any kind, ProbCut, internal iterative reduction, delta pruning, quiescence per-move futility, capture history, continuation history, correction history, and the `improving` flag. Reverse futility left this row at S033 (2026-08-16) and aspiration windows at S021 (2026-08-17). **Measured 2026-08-19, this is what the row costs:** after `e4 e5 Nf3 Nc6 Bb5 a6` at `go movetime 250` chesso reaches **depth 12 on 1448572 nodes** where Stockfish reaches **depth 15 to 16 on 158837 nodes** — nine times the nodes, four plies shallower. DEC-081 is the decision that follows from it |
| absent, evaluation model | mobility is **linear**, one weight per piece type over a raw count with no exclusions, and king safety is **linear** in attacker counts — the weakest published forms of both. The fit returns knight mobility at -1 middlegame and 0 endgame, rook at 0 endgame and queen at -6, which is what a correct fit gives back when the model cannot hold the curve. Also absent: a per-count mobility table, safe checks, weak squares in the king zone, a pawn hash, an evaluation cache, threats, outposts, space, candidate and blocked passers, passer-to-king distance, phalanx and supported pawns, and endgame scaling factors. S121, S122, S123, S124, S101, S125, S102 and S118 |
| absent, machinery | the transposition table is **direct-mapped**, one 24-byte entry per slot with no bucket, no prefetch and no page hint; 24 bytes divides neither 32 nor 64, so entries straddle cache lines. Measured 2026-08-19, `go movetime 2000` from the start position: 16 MB against 512 MB is **36 % fewer nodes and 21 % lower nps**, and the rating list runs 128 to 256 MB while `fastchess.sh` runs 16. S119, and S105 for the regime |
| absent, machinery | ~~quiescence never probes or stores the transposition table~~ **it does both since 2026-08-18, S094**: an entry it writes is stored at `TT_DEPTH_QS = -1`, below every depth the main search can ask for, so it answers quiescence and nothing else, while quiescence accepts every entry in the table. `tt_entry_answers()` is the one place the depth rule lives and both callers go through it. The store carries no move where the node stood pat. **The SPRT that decides it returned zero**: 3000 games in 2 h 07 m against `bd45afe`, **no bound reached**, `LLR -1.32`, `Elo -0.23 +/- 9.32`, `nElo -0.31 +/- 12.43`, 49.97 %, 0 time forfeits. Recorded as zero and kept, DEC-079 -- the commit also carries the mate-score fix below, which is not a zero. Two things came out of it that were not in the plan: `de_normalize_score()` excluded `+/-MATE_MAX` and quiescence is the only writer that reaches it, so a mate with no legal reply read back three plies short until the bound was made inclusive; and `tt_store_entry()` no longer asserts a non-zero move, the main search asserting its own instead. ~~no static evaluation in a table entry~~ **the entry carries one since 2026-08-18, S094**: `int16_t eval`, `TT_EVAL_NONE` where the node that wrote it never had one. **The hazard the step names does not exist here and that is measured, not argued**: the field lands in padding the key's alignment was already reserving, so `sizeof(tt_entry_t)` is **24 before and after**, and `tt_resize()` floors the entry count to a power of two, which absorbs any entry size from 17 to 32 bytes -- 4 MB buys 131072 entries at 20, 24 or 32 bytes alike. So the layout change is behaviour-neutral and INV-6 is discharged on node counts rather than owed an SPRT: **164123 / 670488 / 84351 nodes, `c3d5` / `e2a6` / `d7c8q`**, identical to the commit before it. What is stored is the score and never the bound `evaluate_lazy()` returns in its place, which is why that function now reports which of the two it returned -- a bound is true on one side of one window and an entry outlives the window. Quiescence stores it where the shortcut did not fire; the main search stores it at the reverse-futility nodes, the only place it computes one, and no call was added anywhere to fill the field in (INV-4). **Quiescence reads it back in place of the recomputation, and that measured zero too**: **H0 accepted** in 1954 games and 1 h 23 m against `7d2da9d`, `LLR -2.21`, `Elo -6.40 +/- 11.42`, `nElo -8.64 +/- 15.40`, 49.08 %, 0 time forfeits -- H0 being "not a +10 improvement", which a true zero satisfies. INV-4 still holds -- the number came from the accumulators `make_move` maintains and nothing stopped maintaining them; what is skipped is a second rebuild of the same score from them. **The reach is small and measured**: over a depth 12 search on the kiwipete position, 2530591 quiescence nodes past the probe found any entry at all on **19901 of them, 0.79 %**, carried an evaluation on 11882 of those, and the number differed from a fresh `evaluate_lazy()` on about 0.05 % of nodes -- so the probe hit rate, not the field, is what limits it. ~~The reverse-futility site is not a consumer yet and is now S103~~ **it reads the field since 2026-08-19, S103**, which is where the value turned out to be: reverse futility takes the stored number where the probe it already paid for found one and calls `evaluate()` only otherwise. **Behaviour-neutral and discharged on node counts, no SPRT owed**: 164123 / 670488 / 84351 at depth 9 and 1162576 / 5167100 / 683367 at depth 12, `c3d5` / `e2a6` / `d7c8q` at both, identical to `45ec005`. **The hit rate is a property of how warm the table is and was counted, not argued**: 3368027 of 14589403 calls at the site, **23.09 %**, over 300 positions at depth 10 through one engine process, and **0 disagreements** with a fresh call; the three `search_bench` positions at depth 12, one process and one table for all three, read 173440 of 1617617, **10.72 %**, 0 disagreements. **The speed-up is +2.47 %**, 95 % CI +1.35 to +3.59 %, geometric mean ratio 0.9759 over one interleaved run of 24 alternating pairs, paired t -4.36. It is smaller than a single before-and-after comparison on this machine can resolve -- `bench_movegen` reported resolution 0.1 % but 2.6 % spread that session and `bench_eval` 3.0 % -- and the pairing is what resolves it. 291 ms of skipped `evaluate()` at 86.41 ns a call against an 11.55 s base predicts 2.52 %, which is the clock's number reached from the call count instead. **The bound conditions and the mate-score round trip were swept in 2026-08-20, S106, and nothing was found**: every store and probe in both searches was walked against the source, and the eight sites are correct -- the exactness test in both searches is against the bound the node started with, `de_normalize_score()` runs before the bound comparison so a mate bound is compared re-based, and quiescence's stand-pat lower bound is true because `evaluate_expensive()` is clamped to `+/-LAZY_EVAL_MARGIN`. What the step leaves behind is nine tests, each observed red under a stated mutation -- swapped bound conditions, comparison before adjustment, each of the four ply terms dropped, the draw tests moved below the probe -- and a `static_assert` that the mate band exceeds `MAX_PLY`, which the round trip has always depended on and nothing held. Two things are recorded as accepted rather than fixed: quiescence probes with no draw checks (GHI, the published state of the practice), and `negamax` can overwrite a stored `eval` with `TT_EVAL_NONE`, which S108 removes. **Quiescence stands pat on the table *score* and not only on the stored static evaluation since 2026-08-22, S130** -- the layer above S094's, and the one the published record prices best. Where the probe already paid for did not answer the node outright, its score is still a bound on this position, and it moves the stand pat in exactly the one direction its type certifies: a `TT_BETA_NODE` may only raise it, a `TT_ALPHA_NODE` may only lower it, an exact score replaces it -- the last arm unreachable today, since `tt_entry_answers()` returns an exact entry whatever the window is, and written anyway because the rule is stated by bound type and not by which arms the probe leaves over. **A mate-band score is never substituted**, tested on the raw score, which is ply-independent because the band is 1000 wide against a `MAX_PLY` of 128; the stand pat is handed to the parent and stored by three separate paths, and a mate distance entering there is a mate no search found. The `eval` field keeps S094's semantics exactly: `stored_eval` is fixed from the static number before the substituted stand pat exists, so an improved stand pat cannot reach it. **And the node stores only what it can still claim, which S130 as first written did not** -- the final store marked the entry exact whenever `best_value > alpha0`, which is correct while the stand pat is the static score and false once it is a bound, so `value >= s` was written back as `value == s` at the same key and depth, over the entry that certified it. The upper-bound case was unconditional: an upper-bound entry only reaches the stand pat when its score is above alpha, so the capped value is always above `alpha0`. Instrumented over three positions at depth 12, **386 of the 561 exact stores at that site were laundered bounds, 69 %**. The stand pat now carries its bound kind beside its number and the store degrades: still the raised stand pat means a lower bound, still the lowered one means an upper bound, and a searched move that took over the maximum restores exactness -- except where the floor was *lowered*, since the static score it displaced may beat that move too, which is the one asymmetry and it has its own test. With no entry every branch reduces to the pre-S130 code exactly, and a raise nothing beats now re-stores the same `TT_BETA_NODE` it read instead of upgrading its own evidence, so the substitution is fixpoint-stable. DEC-102. **The reach is thin and it was counted before the match was booked**: over the three `search_bench` positions at depth 12 in one process, quiescence reached the stand-pat site 1821940 times, found any entry on **20432 of them, 1.12 %** -- up from S094's 0.79 % as the probe warmed -- and substituted on **4161 raises, 0.228 % of sites**, plus 151 caps at 0.008 %; the exact arm fired 0 times and 0 entries were rejected for a mate score, which is why the band exclusion is held by a unit test and not by a counter. **The SPRT returned no verdict and that is the recorded outcome**: 16784 games in 7 h 12 m against `293a45b` at `elo0=0 elo1=5`, `Elo 1.14 +/- 4.04`, `nElo 1.48 +/- 5.26`, `LOS 70.95 %`, `LLR -0.71` inside `(-2.94, 2.94)`, 50.16 %, 0 time forfeits in 16788. Not a slow approach to a bound but an oscillation around zero: over 839 samples the LLR stayed inside `[-1.71, +0.69]`, never past 58 % of the way to H0 or 23 % to H1. **Kept and recorded as zero, DEC-103**, on a positive point estimate, the S005/S006/S015 precedent, a thin mechanism that S119 and S120 both widen, and -- the load-bearing reason -- the bound-type rule being durable infrastructure that S112, S022 and S116 each need before they substitute into the same stand pat. **The published figures did not transfer, for the third time this session**: Weiss cca90ea7 measured +10.78/+12.09/+21.44 and Ethereal 25e56feb +11.04/+3.12/+2.55, while Lynx PR #1319 -- the only exactly-shaped record, qsearch only on top of an existing eval-field read -- merged at about zero over 60152 games, and it is Lynx's number this reproduced. DEC-019. `adocs/data/S130_sprt.sh` is the run, its three readings pre-registered before the first game including the no-verdict clause with 8000 games on it, and `adocs/data/S130_sprt.log` is what it printed; an earlier run was killed at 103 games because it was measuring the unsound store, `adocs/data/S130_sprt_run1.log`. no `improving` flag; quiescence is capped at `MaxQsearchDepth` plies, 19 as shipped since S085 retuned it from 8. **Quiet history is butterfly-indexed with a malus and gravity ageing since 2026-08-22, S093.** It was `[piece][to]`, a `depth*depth` bonus to the move that cut off, no penalty for anything else, and a `std::min` saturation at 600000 standing in for ageing -- so it was a monotone "moves that ever worked" counter and not a signed preference, and two same-type pieces going to the same square shared one cell while two different pieces going the same way did not. It is now `int16_t quiet_history[2][64][64]`, `[side to move][from][to]`, 16 KB, updated by the one published line `entry += clamp(b) - entry*|clamp(b)|/QuietHistoryMax` with `+b` to the quiet that caused the cutoff and `-b` to every quiet the node searched before it. Three things follow from that algebra and each is asserted rather than argued: the interval `[-QuietHistoryMax, +QuietHistoryMax]` is closed under the update, so the ordering band is bounded by the update itself and the saturation is deleted rather than kept beside it; every update shrinks the old value by `(1 - |b|/MAX)`, so ageing is by construction and a periodic halving on top would age twice; and an unexpected cutoff moves an entry by the whole bonus where an expected one moves it by nothing. The malus span is built at the call site and appended to *after* the cutoff test, so the move that cut off is structurally absent from it -- Lynx shipped the inverse off-by-one and removing it measured +12.89 on its own -- and it mirrors the bonus gate exactly, `!is_capture` and nothing else. Its other published bug, charging the malus to moves whose `make_move` failed, cannot occur here: `generate_moves()` emits legal moves only. The band is a CLAUDE.md one-way door and the malus is what made its floor real, so `tests/test_evaluation.cpp` now pins both edges of the closed interval against the countermove band above and against the emptiness below, and `tests/test_search.cpp` drives an entry to each asymptote through the update rather than assigning it there. The three seeded bonus/malus coefficients are shipped equal and split only as tuner axes for S127; the published split (Weiss +5.78 LTC, Lynx `x^2+x+c`) is not claimed here. Play-altering by construction, so INV-6 is not available: `search_bench` at depth 13 reads 944870 / 5202441 / 533229 against `6e0afa0`'s 863774 / 7248224 / 529142, **+9.4 % / -28.2 % / +0.8 %, -22.7 % in total**, best move unchanged at all three, and node throughput down about 2 to 3 %. **H1 accepted** against `elo0=0 elo1=5` in 6412 games and 2 h 44 m against `6e0afa0`, `LLR 2.95`, `Elo 10.73 +/- 6.70`, `nElo 13.63 +/- 8.50`, `LOS 99.92 %`, 51.54 %, 0 time forfeits in 6413 -- so the mechanism gains 5 Elo or more, which is the pre-registered claim, and the magnitude is not established because the early stop biases the point estimate upward (DEC-063). **The published figures did not transfer and that is worth the sentence:** Weiss measured this at +37.49 +- 12.19 over 1228 games and Lynx at +28.0 +- 10.3, and neither interval overlaps the +10.73 +- 6.70 measured here over 6412. The direction transferred, the magnitude did not -- DEC-019 in the form that is easy to miss, because the change is a real gain and the number is still wrong. **History is still zeroed on every `go`, and since 2026-08-22 that is measured rather than inherited.** Carrying it through a game was S093 verdict 2, the second half of the same published mechanism and the thing every engine surveyed does: Lynx PR #637 measured +12.5 +/- 6.6 over 6419 games at LOS 100 % for exactly it, and PR #457 measured every alternative to keeping worse. Built here in full -- the table hoisted into a `quiet_history_t` the UCI layer owned, cleared on `ucinewgame` and on a root that does not descend from the last one searched -- and **H0 accepted in 6 h 35 m over 15398 games against `40f5b56`: `Elo -1.65 +/- 4.22`, `nElo -2.14 +/- 5.49`, `LOS 22.22 %`, `LLR -2.96`, 49.76 %, 0 forfeits in 15399.** A well-measured null and not an ambiguous one: the interval excludes the +5 the bounds were set to find and a loss of 6 or more alike. Reverted in full, DEC-101, and the reason it is reverted rather than kept as a zero is that it is the second transfer failure in one step -- verdict 1 measured a third of its prior and this measured the wrong sign -- and that the hoist demonstrably cost a silent `nullptr` dereference in `tools/datagen.cpp` for it. The design is not lost: `adocs/plan_done/S093_history_malus_and_ageing.md` keeps the descendancy rule, the three-stage red-first evidence and both verdicts, so reviving it is a re-measurement and not a rebuild. `adocs/data/S093_sprt_v1.sh` is the run and its three readings were pre-registered in the script header before the first game. **Eligibility for all three ordering tables is one predicate since 2026-08-20, S107: a non-capture that caused a cutoff, checks included** -- the countermove slot alone carries a second condition that was always there and is not an eligibility rule, `prev_move != 0`, since the root and the child of a null move have no previous move to index by. The gate also carried `!is_check_move`, so a quiet move that gives check entered no killer slot, no history cell and no countermove slot -- the one class of quiet this engine's own late move reduction refuses to reduce on the grounds that the line is forcing was the class its ordering memory refused to remember, and no comment or decision argued for it. Nothing in the published record does either: CPW's Killer, History and Countermove pages all gate on a non-capture at a cutoff and none carries a check condition. `is_check_move` now has exactly one consumer, the late move reduction guard, which is what S020 has to preserve. **H1 accepted** against `elo0=-5 elo1=0` in 3812 games and 1 h 37 m 52 s, `LLR 2.95`, `Elo 12.67 +/- 8.65`, `nElo 16.18 +/- 11.03`, `LOS 99.80 %`, 51.82 %, 0 time forfeits in 3813 -- so a regression of 5 Elo or more is excluded and the sign is positive, while the magnitude is not established, the early stop biasing the point estimate upward (DEC-063, and S068 is the case where a pooled estimate fell from +12.18 to +5.02 under that correction). Play-altering by construction and measured so: `search_bench` at depth 11 reads 558693 / 2402718 / 325059 against `ec4d1dd`'s 563497 / 2420695 / 287100, -0.85 % / -0.74 % / **+13.2 %**, best move unchanged at all three. Eligibility says which moves may enter the tables. **The two killer slots routinely hold the same move, and since 2026-08-21, S149, that is a measured choice rather than an oversight** -- the store shifts slot 0 into slot 1 unguarded, so a quiet that fails high twice at one ply copies slot 0 onto itself. Killers survive every iteration of one `go`, so the repeat is the common case and not an edge: **351422 of 532133 stores, 66.0 %**, leaving both slots equal on **5115505 of 11531069 negamax nodes, 44.4 %**, over 11 positions at depths 12 to 22 and about 19 M nodes. `score_move` tests slot 0 first, so on those nodes no distinct move reaches the `ORDER_KILLER_1` band at all and the second refutation falls to the countermove band or into history. **CPW's *Killer Heuristic* replacement rule says the slots ought to hold different moves; implemented here it measured negative and was rejected.** The guard is two lines and it removed the duplication completely -- the same instrumentation reads **0 of 11146351 nodes** -- for negamax nodes -3.34 % and total nodes -2.87 % over the same 11 positions. It then lost: **H0 accepted** against `elo0=-5 elo1=5` in 2522 games and 1 h 05 m against `ac4c588`, `LLR -2.97`, `Elo -11.02 +/- 10.53`, `nElo -14.21 +/- 13.56`, `LOS 2.00 %`, 48.41 %, 0 time forfeits in 2524 -- so the published rule costs about 11 Elo on this search and DEC-019 has a fourth entry (`adocs/data/S149_sprt.sh` is the run and its bounds were pre-registered with all three readings). **The hypothesis for why, untested and recorded as a hypothesis:** the unguarded shift discards whatever slot 1 held every time a quiet repeats, so it is also an ageing mechanism for the second slot, and the guard preserves a stale killer for the whole of one `go`. That points at a different change and it is S159, not this one. Found by 2026-08-21_adversarial-F01, whose second half stands regardless of the verdict: the test written for the slot counted slots that were **non-zero**, which a slot 0 copied onto itself satisfies. It counts **duplicated** slots now, asserting the behaviour that shipped and carrying the number that decided it, so an agent who re-guards the store goes red and finds the measurement instead of repeating the night. |

Both absent rows hold strength. **The evaluation row holds more of it**, which
is measured rather than assumed — see below and DEC-033. The machinery row is
what the audit of the search found while measuring that and has no steps yet.

### Where the centipawns actually go

Measured, not assumed. 13522 moves by chesso over 210 games against sgambetto at
10+0.2, every position scored by Stockfish `dev-20260803-762dd1da` at 3000000
nodes. S018, DEC-032. Phase is the engine's own `game_phase()`, so it names the
quantity the tapered evaluation tapers on.

| phase | moves | cp/move | share of 407740 cp | own score minus reference, mean |
|---|---|---|---|---|
| opening 22-24 | 1934 | 39.9 | 18.9 % | +39.2 |
| early middlegame 14-21 | 3323 | **44.1** | **36.0 %** | +77.4 |
| late middlegame 7-13 | 3493 | 28.4 | 24.3 % | **+100.2** |
| endgame 1-6 | 4570 | 18.3 | 20.5 % | +41.8 |
| pawn endgame 0 | 202 | 6.8 | 0.3 % | +29.5 |

Half the loss sits in moves costing 100 to 400 cp. Blunders above 400 cp are
0.5 % of moves and 9.3 % of the loss; moves under 25 cp are 73 % of moves and
7.7 % of the loss.

Two things followed. **The early middlegame is where the centipawns go**, not
the endgame, which is the cheapest phase per move outside pawn endgames. **The
evaluation is optimistic in every phase**, worst in the late middlegame. The
ranking is stable after removing mate-touching moves and after removing clamped
reference scores. It has been measured against one opponent only, and DEC-019
is the reason that matters.

#### The same profile after the constants were fitted

Added 2026-08-11 with DEC-035. S028 replaced every constant in the evaluation
and was worth +188.74 Elo, so the table above describes an engine that no longer
exists. Re-run identically -- same opponent, time control, adjudication,
reference and 3000000-node limit -- over 98 games and 5582 moves. Evidence in
`adocs/data/S028_raw.tsv`.

| phase | moves | cp/move | share of 154039 cp | own score minus reference, mean | median |
|---|---|---|---|---|---|
| opening 22-24 | 1098 | 31.6 | 22.5 % | -1.5 | 6 |
| early middlegame 14-21 | 1697 | **35.2** | **38.8 %** | -24.3 | 0 |
| late middlegame 7-13 | 1289 | 24.8 | 20.8 % | -40.3 | -4 |
| endgame 1-6 | 1448 | 18.6 | 17.5 % | -3.5 | -1 |
| pawn endgame 0 | 50 | 13.2 | 0.4 % | **+204.3** | 227 |

**The ranking is unchanged and the optimism is gone.** Per-move cost fell in
every phase except the endgame, which is flat at 18.6 against 18.3: the tuning
bought most where most of the loss already was, and moved the endgame least.
The bias reversed sign in four phases of five, but the medians say that is a
tail effect rather than a uniform shift. The exception is the pawn endgame,
which is the one place the old warning survives and the one place it got worse
-- on 22 moves, which decides nothing by itself.

### Search error or evaluation error

Added 2026-08-10 with DEC-033. The profile above says how much was given away
and where; it does not say why. 160 moves that cost 100 cp or more while the
game was still undecided were re-asked of chesso at 4000000 nodes, about its
budget per move at 10+0.2, and at 64000000 nodes, and both answers were costed
by the same reference. `tools/depth_vs_eval.py`, evidence in
`adocs/data/DEC033_depth_vs_eval.tsv`.

| | cp/move |
|---|---|
| as played in the game | 204.7 |
| at 4000000 nodes | 170.1 |
| at 64000000 nodes | 129.1 |

Sixteen times the search removes **24.1 %** of the error, 10.3 cp per doubling,
and **95 of the 160 moves are unchanged**. Where the move does change, cost
falls from 186.0 to 84.9. Quiet moves carry 82.8 % of all loss in the profile
and 1031 of its 1235 errors of 100 cp or more.

**Chesso is evaluation-limited, not depth-limited, on the errors that decide
games.** That is what put S028 next and reordered everything after it. The
figure bounds the search block too: about 10 cp per effective doubling is what
those steps are playing for.

Re-run on the fitted evaluation, 2026-08-11, DEC-035, same criteria and sample
size, evidence in `adocs/data/S028_depth_vs_eval.tsv`: as played 182.1, at 4M
139.7, at 64M 98.0. Sixteen times the search now removes **29.8 %** and **78 of
160 moves are unchanged**. The balance shifted toward the search and did not
turn over -- seventy per cent of the error still survives a sixteen-fold search
-- and the price is the same, **10.4 cp per effective doubling** against 10.3.
The order stands.

## Non-goals

- **Nothing is copied.** No source from another engine, no tables from another
  engine, no NNUE training data derived from another engine's evaluation or
  search. Ideas and published articles are used freely; that is the plan.
  Running another engine's binary as a tool creates no derivative work and is
  encouraged. DEC-016. Inspiration from another open-source engine is taken
  only where its licence consents, adapted and never copy-pasted. DEC-104.
  Another engine's constants are never seeds, and republication does not
  change their origin — a tuned table on the wiki is still an engine's
  table. DEC-084, DEC-105.
- **The agent does not run the NNUE network training.** It builds the trainer,
  prepares the data and states what the run should be; the owner executes that
  one run. Fits, measurements and evaluation tuning are the agent's to run.
  DEC-015 as amended by DEC-041. (2026-08-13: narrowed from "training or table
  tuning" — DEC-041 superseded DEC-015 for evaluation tuning and every kind of
  measurement, and S028's fit was agent-run.)
- **Published Elo figures are not targets.** They have failed to transfer three
  times here. They decide what to try, never what to conclude. DEC-019.
- **Phase-two experiments are not started early.** An idea measured against a
  weak engine produces a number that does not transfer. DEC-014.
- **`README.md` is not an agent-writable file.** DEC-017.
- **`master` and `bitboard` are lineage, not maintained here.** DEC-013.

## Open items

- The S015 quiescence SEE pruning measured 0 Elo when `see()` cost 12.1 % more
  than it does now. The rerun has not happened; it is folded into S022.
- The S013 LMR SPRT was killed at 96 % LLR, +129.2 +/- 33.8 over 183 games. It
  was never formally concluded.
- Measurement capacity is the binding constraint on the whole plan. The
  harness runs the surveyed engines' regime since 2026-08-20 (S105, DEC-083,
  DEC-088): `tc=8+0.08`, `Hash=16`, the unbalanced `UHO_Lichess_4852_v1.epd`,
  all 12 threads of the DEC-049 machine. **Calibrated at 38.7 games a minute
  against the old regime's 23.1, measured over two A/A runs of 1000 games each
  in the same hour** -- so **x1.67, not the x3 DEC-083 priced**. It decomposes
  as x1.41 from the control (0.2579 to 0.1831 seconds a ply) and x1.20 from
  shorter games. **The unbalanced book buys the game length and nothing else**:
  the pair score variance is unchanged within its error bar (0.2343 +/- 0.0148
  against 0.2395 +/- 0.0152, ratio 1.022) while 1:1 pairs rose 41.6 % to 46.8 %
  and pairs decided by the opening 13.4 % to 19.8 %. **Pohl's >= 45 % draw
  floor is unreachable at this strength** and was already breached before the
  change: chesso self-plays the *balanced* book at 40.3 % draws where Pohl
  measured 91.6 % between engines 600 points stronger. The book is kept on the
  x1.20 and on DEC-083; its stated reason does not hold here.
  `adocs/data/S105_calibration*` is the evidence. **0 time forfeits in 1000
  games at the faster control**, checked from the PGN before anything else was
  read -- both `fastchess.log` files were 0 bytes, the WARN-only default again.
  (2026-08-13: rewritten for the DEC-049 move -- the old text priced an hour on
  three Apple cores with `opendirectoryd` overhead and called for an x86-64
  box, which S032 and S029 now have.)
- Phase two has no steps and should not get any until the engine is strong
  enough for an experiment to mean something. The transition gets a decision
  entry when it happens.
- ~~The "absent, machinery" row above has no plan steps behind it.~~
  **Discharged 2026-08-18 by DEC-071** and re-ordered 2026-08-19 by DEC-081 to
  DEC-086. **Check extensions have no step and are not owed one**: S096 was
  retired outright by DEC-087 (a), because Ethereal removed check extensions for
  +4.1/+4.5 and Stormphrax removed them as a simplification, because the late
  move reduction here already refuses to reduce a checking move
  (`src/search.cpp:710`, `!is_in_check && !is_check_move`), and because S097
  covers the forcing-line concern. The id is not reused. Singular extensions are
  S097, the quiescence
  transposition probe and the static evaluation in the entry S094 (done),
  history malus, gravity and butterfly indexing S093, correction history S099,
  S110 and S111; SEE pruning of captures in the main search is S091, internal
  iterative reduction S095 and reduction refinement S098. Time management is
  S089 (done). **Late move pruning, futility, history pruning and quiet SEE
  pruning are one step, S109**, because the parts are inert apart — DEC-082.
  **The `improving` flag has no step of its own**: it needs a static evaluation
  at every node, which the main search does not compute, so S092 was retired
  into S108, which supplies it. DEC-081 is the reordering: the search block now
  leads the evaluation block.

- **The lazy-evaluation clamp is the ceiling on how much the evaluation is
  allowed to say, and no step addressed that until 2026-08-19.**
  `evaluate_expensive()` clamps mobility **plus** king safety to
  `+/-LAZY_EVAL_MARGIN`, 184 centipawns for the two together since S085, and `evaluate()`
  is `evaluate_cheap() + evaluate_expensive()` — so the clamp binds the real
  score, not only the shortcut's. A king-safety term strong enough to price a
  mating attack cannot exist under it. Measured 2026-08-19, `go movetime 2000`
  from the start position on the tune build: `LazyEvalMargin` at 0, 150 and 2000
  gives **6.55, 5.46 and 4.82 Mnps**, so paying the full evaluation everywhere
  costs 11.7 %. S039 re-decides the margin, S120 caches the score behind it, and
  S122 is the rebuild that needs both.

