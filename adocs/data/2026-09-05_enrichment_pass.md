# Enrichment pass of 2026-09-05 — report to the owner

**What happened.** The owner left for the day with two instructions: reorder
the plan for the goal under the project's rules, assuming every step runs on
the Linux workstation, and then enrich every pending step file for the agent
that will implement it, one agent per file, one at a time, in the new order.
Mid-pass the owner added: after the current file, stop and record how far the
pass reached. Both instructions were carried out on the MacBook. No match, no
fit and no timing was started; the engine did not change. The tree was green
at `66cbc54` before the first edit (fast suite 27/27 in both builds) and every
commit since is documents only.

**Commits.** `2445d23` is the reorder (DEC-144, DEC-145). `ad4e6fa` to
`04abd23` are twenty enrichment commits, one per file, each verified with
`git status` (only the step file), `tools/plan_prose_check.py --touches`
(0 flagged) and a grep for line-number citations into code (0 added).

**Where the pass stopped.** Twenty of 74 pending files carry
`## Implementation guide (2026-09-05)`: S178, S173, S171, S189, S179, S198,
S180, S184, S148, S187, S190, S159, S193, S191, S196, S197, S192, S195, S194,
S151 — Open entries 1 to 20. **The next file is S181** (Open entry 21), then
S185, S182, S183, and block 1 from S024. The recipe that names what is left:

    grep -L 'Implementation guide (2026-09-05)' adocs/plan_todo/*.md

The brief every agent followed is tracked as
`adocs/data/2026-09-05_enrichment_brief.md`; a later session hands the same
brief and the file path to one general-purpose agent per file, sequentially,
and commits after each. Each agent cost 150 k to 310 k tokens and 10 to 22
minutes; three were killed by the session limit and relaunched from a clean
tree.

## The reorder, in one paragraph

The DEC-112 machine-scope lane is deleted from `plan.md` and the Open list
re-sorted (DEC-144): two tool bugs; S171's census, the first run on the
workstation (DEC-128); S189 and S179 as agent-only work while it plays; S198's
harness flags with the calibrating A/A (DEC-143); an instrument lane of the
sixteen review steps interleaved with S148, S159 and S151 so that one agent
with one machine always has a next entry; then the four blocks unchanged in
their 2026-08-19 order as DEC-133 corrected it; S152 closing the main order;
reserve and parked last. `plan.md` "What the 2026-09-05 reorder changed" has
the reading rule for one machine. One question was deferred there: S151's
bounds pair (below).

## Per-file summary

| step | lines | sources fetched | unverified | seeds replaced | note |
|---|---|---|---|---|---|
| S178 | 246 | 2 (+1 blocked 403) | 0 | none | PGN standard sections quoted at source |
| S173 | 321 | 9 | 1 | none | finding: SIGXFSZ kills the tool before S146's guard |
| S171 | 298 | 6 | 1 | none | census run guide; fastchess.sh not edited while it plays |
| S189 | 258 | 4 | 0 | none | finding: `set_position` never clears `proven_mate_line` |
| S179 | 464 | 9 | 2 | none | splitmix64; two commits, magics then keys |
| S198 | 320 | 9 | 2 | none | three accepts assumptions do not hold at HEAD |
| S180 | 407 | 11 | 2 | none (inventory of 26 seeds written) | 15 seeds the review missed |
| S184 | 330 | 5 | 0 | none | `--params` already in the fast suite; S127 stale |
| S148 | 429 | 10 | 1 | none | challenger derived from a HEAD sweep, not the stale table |
| S187 | 293 | 0 (in-repo) | 0 | none | census 564 citations, not 549 |
| S190 | 283 | 3 | 0 | none | accepts' depth-3 walk measured at ~34 s vs 5 s budget |
| S159 | 406 | 6 | 0 | none | candidate B is behaviour-neutral, cannot be SPRT'd |
| S193 | 163 | 4 | 0 | none | `ucinewgame` cannot clear the stop flag at HEAD |
| S191 | 457 | 3 | 0 | none | every guard observable without touching `src/` |
| S196 | 316 | 3 | 1 | none | 34 mutants (M06 split), two results rows missing |
| S197 | 378 | 9 | 3 | none | finding: SANITIZER block lets UBSan recover, exit 0 |
| S192 | 320 | 2 | 0 | none | `anchors.py` is tracked, not gitignored; floor is 11 not 8 |
| S195 | 346 | 4 | 1 | none | all 40 node-limited pairs identical at HEAD today |
| S194 | 249 | 8 (+4 403s) | 0 | none | RNG lives in `src/chesso.cpp`; seed read in `uci_init` |
| S151 | 499 | 12 | 1 | none | three run designs priced; six owner choices |

No seed was replaced because none of the twenty files is one of DEC-134's
seven; S180's inventory names the replacement for every seed in those seven,
which the agents for S095, S097, S098, S109, S113, S114 and S132 will apply
when the pass reaches them.

## Findings that look like bugs or stale clauses

None of these was fixed; the BUGS rule says a found bug is fixed before
anything else starts, and the owner decides which of these are bugs.

**In code or scripts, worth a decision before the workstation runs.**

1. `tools/make_book.cpp` `build` under a file-size limit is killed by
   `SIGXFSZ` (exit 153) before S146's stream guard runs and leaves a
   204800-byte book that `dump` calls `loadable` — a failure mode beyond what
   S173 describes. Prototype of `mkstemp` + `write` + `fsync` + `rename` with
   `SIGXFSZ` ignored passes both failure modes. (S173 agent, reproduced today.)
2. `CMakeLists.txt` `SANITIZER` block: UBSan recovers by default under clang
   and gcc, so an undefined-behaviour report exits 0 and a `-DSANITIZER=ON`
   run cannot fail on UB today. Fix `-fno-sanitize-recover=undefined`, red
   first. (S197 agent; the file is in S197's touches.)
3. `adocs/data/S105_pairs.py` `report()` defaults `engine='chesso-a'`;
   `fastchess.sh` names sides `candidate` and `ref-<sha>`, so reading S198's
   A/A with it as the accepts states gives a silently inflated pair variance
   (1:1 pairs read as 0.0). (S198 agent.)
4. `fastchess.sh` has no fixed-rounds mode without `-sprt`, so S198's
   1000-game A/A cannot be launched as the accepts states; `AA=1` also plays
   two separately compiled builds (`build/` vs `.ref-builds/`). (S198 agent.)
5. `command_ucinewgame` calls `stop_and_join_search()`, which sets the stop
   flag; only `command_go` and `command_test` clear it. S193's accepts names
   `ucinewgame` as the remedy for the unbounded-search case, which it cannot
   be. (S193 agent; confirmed by S195's agent.)
6. `set_position` never clears `proven_mate_line`; only `command_ucinewgame`
   does, so a multi-position command that skips `ucinewgame` carries S170's
   mate line across positions. Reporting only, cannot move node counts; the
   `test` command has the exposure today and `bench` must run the full reset.
   (S189 agent; confirmed by S195's.)
7. `.tuning/anchors.py` is tracked (since `c56ab41`, `.gitignore` has
   `!.tuning/*.py`) but hard-codes `ROOT = "/home/max/ws/chesso/"` and does
   not run here; patched in a scratch copy it prints `10 of 10 reproduced`.
   `status.md`'s Parked note calling it gitignored is stale. (S192 agent.)
8. `tests/test_chesso.cpp` `test_generate_legal_moves` carries an `assert`
   that compiles out of Release (F01 class); `tests/test_corpus_hash.cpp` has
   three fixed temp-file names the review's F09 did not list. (S193 agent.)
9. `tests/test_engine.cpp` "setoption carries a value containing spaces": its
   `REQUIRE(!capture.contains(...))` is vacuous in both builds — the capture
   reads `std::cout`, the log goes to `std::clog`. Already S193's R3. (S194.)
10. `tools/plan_prose_check.py` `carries()` strips `()` so
    `CHESSO_SEARCH_PARAMS(X)` becomes `CHESSO_SEARCH_PARAMS(X`; S187's new
    token grammar must strip a parenthesised tail. (S187 agent.)
11. `set_en_passant` in `src/bitboard.cpp` has no caller anywhere; S042 should
    decide on the dead API. (S184 agent.)

**Stale clauses in pending files.**

12. S190's accepts: "every test FEN to depth 3 ... under 5 s" is
    unsatisfiable by about seven times (54 M makes, about 34 s with the
    compare after every make and unmake, measured today in Release on the M1).
    Proposed: depth 2 over the corpus plus the hash oracle's own depths on its
    five positions, 2.1 M makes, 1.4 s.
13. S159's second candidate ("shift slot 1 down on a repeat") is
    behaviour-neutral on this `score_move`, which tests slot 0 first; it is
    proved by INV-6 and cannot be "decided by SPRT". A substitute is the
    owner's.
14. S148's S145 table is the `14748c9` reading; S154's re-take at `fc5526e`
    moved every count and the 82-row set has never been swept for the
    ceiling. Its "19 ... 13" sentence quotes Stockfish's shipped constants
    for an excluded feature (DEC-134 wording).
15. S184 assumes `--params` is not in the fast suite; it is (`test_plan_params`,
    S150). S127's `MaxQsearchDepth` sentence is stale and S127 is not in
    S184's touches, so the extended gate is red until it is added. Two of
    F09's five items were already closed by `2445d23`; one new orphan sentence
    in `plan.md` ("The batch does not change the machine-scope lane below it")
    took their place and is in S184's scope.
16. S180's inventory found 15 seeded constants the review's F01 did not list,
    and five stale claims in the seven files: S114 says `NULL_MOVE_BASE` 2
    ships (3 since S085); S132 seeds `TM_NODE_MIN_DEPTH` 5 "beside
    `ASPIRATION_MIN_DEPTH`" (that ships 2); S098 and S109 cite `HISTORY_MAX`
    (the symbol is `QUIET_HISTORY_MAX`); S097 claims a CPW singular-margin
    range not on the page; S109's futility margins were converted at
    `see_value`'s pawn, 147/170 in `piece_value` terms.
17. S192: the mate-in-three floor is 11 (S168), not 8; `DEV_MANUAL.md` "Mate
    safety" still says "a floor of 8". S196's accepts says 33 mutants; the
    file holds 34 (M06 split a/b); `results.tsv` lacks rows for M08 and M12.
18. S187: the census at HEAD is 564 code citations, not 549; 59 drifted.
19. S171's accepts clause "MANUAL.md loses the residual" is already satisfied
    at `136b03f`. `DEV_MANUAL.md` names fastchess `20260720-daa3ea2`; this
    MacBook has `alpha 1.8.1 20260715-0b06677`; S105's stamp records no
    version (DEC-143 makes it a harness variable).
20. Header hygiene: S148 carries `author:` in `plan_todo/` and lacks `done:`
    (S184's F10); several files still cite `src/...:line` in their bodies and
    header fields (S187).

## Questions deferred to the owner

Each file's section 10 carries its own list; the ones that change an
`accepts:` or need a decision are gathered here by step.

- **Reorder (DEC-144).** S151's design: (i) SPRT `{-5, 0}` at `32+0.32`,
  41861 games / 71.7 h worst case (5 to 16 h if S085's gain transfers);
  (ii) `{-10, 0}` or `{-5, 5}`, 10465 / 17.9 h; (iii) 1000 pairs, 3.4 h,
  about +/- 10.5 Elo, which changes the accepts' "re-tested". Also S151's
  hash at the longer control (16 or 64), the standing re-take design for the
  13 verdicts the rule binds (240 h at (i), 44 h at (iii)), whether extensions
  and S115's aspiration triple are in scope, whether S148 owes a retroactive
  re-take, and four times (`32+0.32`) or fishtest's six (`48+0.48`).
- **S178.** PGN 8.2.2.1 whitespace forms (`1 . e4`, `1. ... e5`): extend or
  file separately; the cut-short message names the move part, not the glued
  token; a half-sentence in `specs.md`.
- **S173.** "Nothing left beside the destination" cannot hold for `kill -9`;
  a fixed `<out>.tmp` name or a reading; touches omits the test files;
  file-mode policy after replacement.
- **S171.** If the `--fast` SPRT bound ends the census before 3000 games
  (about 4244 expected to a bound for identical play), accept as it stands
  (recommended) or top up.
- **S189.** Touches omits `tests/CMakeLists.txt` and a gate-script test;
  OpenBench's `./binary bench` argv form needs `src/main.cpp`; `KILLER_POS`
  fails python-chess `is_valid()` (nine white pawns); may `tools/gate.sh`
  take a `JOBS` override. DEC-140's "ends with `Bench:`" collides with the
  `Co-Authored-By:` trailer; the guide reads it as "in the trailer block".
- **S179.** The seed value (`20260904` proposed); the accepts' "every best
  move unchanged" after the key change is not guaranteed by a new table slot
  pattern; 851 keys, not "781-plus"; a declared header for the PRNG.
- **S198.** `ROUNDS=` override versus a standalone runner; "shows the
  distribution unchanged" is only a band check against S105; `-event
  "... srand=$seed"` to put the seed in the PGN; a forfeit rate over 1 %
  points at `MOVE_OVERHEAD_MS`, a new step; the workstation's fastchess
  version.
- **S180.** `NULL_MOVE_EVAL_CAP`: midpoint 8 or a measured mate-suite bound;
  the stamp's "F01 table walked row by row" does not cover the 15 extra rows;
  S114's stale `NULL_MOVE_BASE` is in neither S180's nor S184's accepts.
- **S184.** Add S127 to touches; S042's rule pseudo-legal (X-FEN) versus
  legal for the Stockfish comparison; clear or keep S171's `author:`; S118's
  `bench_eval` figure dated or re-measured; should `--prose` learn F09's
  sentence shapes.
- **S148.** `{-5, 0}` versus `{-5, 5}`, and whether a null keeps 15 or ships
  the challenger; one verdict or a pre-registered second at 6; the accepts'
  "re-run `S145_rfp_sweep.log`" conflicts with the data directory's
  append-only rule; reword the Stockfish-constant sentence; a mate-in-four or
  five floor at a count of 1 has no margin.
- **S187.** Touches lacks `adocs/data/` and `tests/`; whether an absent
  document phrase gates or notes; the header says 549; how the fast-label
  decision is recorded. Recommended: the generator at
  `adocs/data/S187_symbolise.py`, the S144/S169 precedent.
- **S190.** The depth and budget above; `memcmp` before/after in this test or
  the hash oracle; a walk-depth override for `gate_extra.sh`.
- **S159.** Drop candidate B or substitute (the per-node two-ply reset needs
  reopening DEC-138's no-step record; candidate A plus S149's guard reads as
  re-litigating S149); `{-5, 5}` versus `{0, 5}`; revert (recommended) or
  keep on a null.
- **S193.** Reword the `ucinewgame` clause; the five unenumerated F05 items
  and `test_corpus_hash`'s temp names in scope; `legal_moves()` asserted via
  `position_is_reachable` (recommended) or reworded.
- **S191.** The accepts requires red under M04 but names no case that can see
  it; six guards have no mutant in the append-only list, so
  `adocs/data/S191_mutants.py` folded by S196; RFP inside the mate band is
  constructible at the negative edge only; "takes no null-move cutoff"
  asserted as "makes no null move"; a probe alternative would put
  `src/search.cpp` in touches; the 104-row scoring against `test_search`'s
  600 s Debug ceiling.
- **S196.** `tests/` in touches for the self-test; does `gate_extra` run the
  full pass weekly; the full-pass table under `adocs/data/`; the fold if S191
  or S193 has not landed.
- **S197.** Assert the sanitizer `bench` total equals Release's (recommended
  yes); the three mate binaries in the Debug stage (recommended no); S196's
  50-minute pass weekly (recommended opt-in); `tools/coverage_unexecuted.py`
  outside touches; the SANITIZER flags; the weekly note's place in `status.md`.
- **S192.** `test_mate_carry` floors at margin 0; M06a becomes a
  single-detector once the soft-limit case stops asserting on the tree, and
  S191 omits the RFP ply floor; the drop half of the soft-limit rule needs a
  `src/` hook; two named constants for the nine 563/567 sites; retiring the
  stale Parked note.
- **S195.** The "adds it otherwise" clause has no `bench` if S195 precedes
  S189 (it does not in the new order); confirm the "last info line plus
  bestmove, pairwise" reading; no case-1 negative control; `MANUAL.md` has no
  `ucinewgame` sentence and is outside touches.
- **S194.** `specs.md` owes a sentence but is not in touches; an unparsable
  `CHESSO_BOOK_SEED` ignored silently in Release or refused with an `info
  string` (needs `MANUAL.md` first).
- **Standing, unchanged by today.** The three lows of the 2026-09-04 audit
  re-run (`go infinite` prints `bestmove` unasked; a bad token in `position
  ... moves` is skipped silently; the aborted-iteration best move assumes its
  table entry survives) still wait on one decision or steps; S193's R12 leaves
  `test_audit_go_infinite` unregistered until then.

## What the agents ran on this machine

Read-only builds and probes only: the fast suite once at `66cbc54`; scratch
walkers linked against `build/src/libchesso_engine.a` (S190, S191); the built
engine and `make_book` driven for node counts, keys and boundary positions
(S189, S193, S194, S195); python-chess and Stockfish at depth 20 as the
DEC-023 oracles; five two-round fastchess smoke runs to verify `-srand` and
`-pgnout` flag behaviour (S198); a 1 MB ram disk created and detached
(S173). Nothing was left in the tree; `git status` was clean before every
commit.
