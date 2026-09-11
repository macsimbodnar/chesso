# Status

Convenience view, rewritten by hand at the end of any turn that changed plan
state. The filesystem beats this file: on disagreement, `plan_current/` wins.
Nothing generates it since moltke v1 (DEC-109), so a stale line here is a
missed edit and not a tool's opinion.

Updated: 2026-09-11, by hand.

- **S215's fast check found two real things, both prose, both fixed in the
  commit below and neither a step.**

  **(1) The mechanism blamed for M39's 13 % bench move was wrong**, in the step
  stamp, in `specs.md`, in `status.md` twice and in M39's own description
  string. `- 1` was described as restoring the pre-S207 root-recurrence draw.
  It does that only to a root that has a history behind it. Every `bench` root
  is a FEN, `load_FEN` calls `cleanup_board` which zeroes `history.size`, and
  `size` is `size_t`, so on a bench root `size - 1` is **`SIZE_MAX`** and *no*
  entry is ever in-tree: M39 removes every in-tree draw rather than adding one
  class back. **Measured, not argued: an explicit `SIZE_MAX` benches 26117924,
  M39's total to the node, against 30046849 shipped.** The direction agrees --
  fewer draws detected, fewer nodes, the same sign S207 measured. The step knew
  about the underflow (it is why the case builds its history from a FEN one ply
  earlier) and did not connect it to bench, where every root is size 0. The
  13 % and both kills stand; only the explanation was wrong.

  **(2) `specs.md` contradicted the case it describes**, saying the depth-4
  search runs "with nothing behind the root" where the root has exactly one
  entry behind it and must have, for (1)'s reason. `plan.md` had it right.

  **The stamp in `adocs/plan_done/` is not edited** -- `plan_done/` is history
  (AGENTS.md) -- so the correction lives here, in `specs.md` and at the mutant.
  Everything else came back clean: the three searches re-derived (**-909 / 0 /
  0**, static **-929**), the perpetual re-verified through python-chess and
  Stockfish, chesso's own PV checked to confirm search 2's `0` is the
  repetition and not another draw, the anchor found exactly once in
  `src/search.cpp`, both mutants' `old` strings exact, 47 `m(` entries against
  `DEV_MANUAL.md`'s 47, and the `done:` stamp present in the commit.

- **S215 is complete, 2026-09-11 afternoon: the line S207's rule hangs its
  boundary on is read by the suite at last, and both of its neighbours die on
  it.**

  S207's rule has two halves and only one was pinned. `classify_repetition()`'s
  comparison is held by M35 and M36; the value it compares against --
  `search()`'s `state->root_history_size = game->history.size` -- was read by
  **nothing**, which the Tier-1 fast check over `23f926d` found and measured
  rather than suspected: mutated to `history.size - 1` the whole fast suite
  stayed green while `bench` read **26117924** against the tree's **30046849**,
  a tree **13 %** different. **What moves the bench is the underflow, not the
  root-recurrence class** -- S215's own fast check caught the stamp saying
  otherwise, and an A/B settled it: every `bench` root is a FEN and `load_FEN`
  zeroes the history, so `size - 1` is `SIZE_MAX` there and *no* entry is ever
  in-tree; an explicit `SIZE_MAX` benches **26117924**, M39's total to the node.
  The root-recurrence reading -- the root position recurring once inside its own
  tree scoring `DRAW_SCORE` again, part of the F08 shape and the class whose
  removal moved `bench` 1.34 % at S207 -- is what `- 1` does to a root that has a
  history behind it, which is ordinary play and is what the case asserts.

  **One board, three searches, all of them through `search()`.** The board is a
  forced perpetual, `6k1/r4pp1/6p1/8/7Q/8/6K1/q7 w - - 10 40`: White a rook and
  three pawns down, **exactly one legal reply to every check**, **no capture
  anywhere in the cycle**, so the winning side never has to cooperate and
  nothing but the boundary decides the line's score. Stockfish at depth 22 says
  **0** on `Qd8+ Kh7 Qh4+ Kg8 Qd8+`; python-chess says `Status.VALID` and counts
  the single reply. (1) **Depth 4** from a root with one entry behind it that is
  not the root position: the cycle returns to the root's *own* entry, not
  strictly inside the tree, no draw, the material -- **-909**, against a
  static **-929**. (2) The same
  board and history at **depth 5**: the ply-1 position returns at ply 5,
  strictly above the root's entry, a draw -- **0**, and the only one available,
  which is the `accepts`' draw that can come from `index > root_history_size`
  alone. (3) The same board with the cycle played *before* the root: two
  occurrences at or below the root, a draw wherever both lie -- **0** at the
  depth that answered the material. (1) and (3) are asserted the same position
  by board hash with `history.size` **1 against 4**, which is S207's rule from
  the other side: same board, other history, other score.

  **All four wrong boundaries observed red before the case was called done**,
  each on the assertion written for it: `- 1` and `0` fail (1) at
  `REQUIRE_LT( 0, -300 )`, `+ 1` and the deleted assignment -- the `SIZE_MAX`
  default -- fail (2) at `REQUIRE_EQ( -841, 0 )`. The history behind (1) and (2)
  is built from a FEN one ply earlier for a reason: at `history.size == 0` the
  `- 1` underflows to `SIZE_MAX` and survives. Under each mutant the **only**
  failing assertion across 33 test binaries is this case's, which is the gap
  restated from the other end.

  **M39 and M40 killed, 2 of 2, from a worktree at `cfe2406`** -- the commit
  carrying the case and the mutants -- baseline green at 33 tests and
  `bench 30046849`, each killed by its own assertion and by `test_search`
  alone. The first run of the pair was taken at `177171e`, the same tree before
  it was amended to carry the trailer every other commit here has, and read the
  same two kills.

  **INV-6 discharged**: `search_bench` depth 9 **121530 / 801481 / 72924**,
  `c3d5` / `e2a6` / `d7c8q`, `bench` **30046849**, the parent's total --
  identical because **no `src/` file was touched**, which is also why the commit
  carries neither a `Bench:` line nor `No functional change` (`tools/gate.sh`:
  a commit touching no `src/` file needs neither). No SPRT: a test and two
  mutants cannot alter play. DEC-141's second tier does not bind and
  `gate_extra` last ran 2026-09-10, inside its cadence. `ctest -L fast` 33/33 in
  **both** builds, format clean under `CLANG_FORMAT_MAJOR=22`.

  **Docs**: `specs.md`'s repetition paragraph names the third case and the
  mutants; `DEV_MANUAL.md`'s mutation-cost line said **41** where the tree holds
  **47** and now names S207's, S208's and this step's additions, the measured
  3948 s staying with the 40 rows it was taken on; `MANUAL.md` checked,
  unchanged, nothing on the UCI surface moved.

- **S209 is complete, 2026-09-11 midday: the UCI option surface follows the
  protocol on case, `Hash` says so when it refuses, and `clean-tt` no longer
  clears the table under a running search.**

  `command_setoption` folds the option name once, before every comparison, and
  folds a check option's value before comparing it to `true`/`false`;
  `option_name` is left as it arrived so a refusal quotes back what was sent,
  and the two values that are **not** folded are `Book File`'s, a path, and a
  spin value, a number. `Hash` is parsed with `std::from_chars` requiring the
  whole token and answers `info string refused [Hash] <value>, not an integer`
  or `..., out of range` on the UCI channel in **both** builds, leaving the
  table the size it had; `-5` still clamps to the minimum and still prints
  nothing, because what `Hash` clamps to is outside this step.
  `command_clean_TT` calls `stop_and_join_search()` first.

  **Every red observed on `013600d` before a line was written.**
  `setoption name hash value 64` and `name HASH value 1` both left the table at
  **524288 entries** -- the name was not an option at all -- and
  `setoption name ownbook value True` then `go depth 1` printed
  `info score cp 72 ... pv e2e4`, a real search, where the canonical spelling
  answers `bestmove e2e4` alone. **`0x40` bought 32768 entries (1 MB, the
  clamped 0) and `64abc` bought 2097152 (64 MB)**, both in silence, as did
  `+64`, `12.5`, `abc`, an empty value and both twenty-digit tokens. `clean-tt`
  during `go infinite` returned with **no `bestmove` on stdout**. In the tune
  build, `rfpmargin`, `RFPMARGIN` and `RfPmArGiN` were each answered
  `unknown option` and left `RfpMargin` at 63.

  **The race is measured, and the first measurement lied.** ASan and TSan
  cannot be linked into one binary and `build-sanitize` is the ASan tree, so
  this is the `accepts`' second branch: a fourth, hand-built TSan tree,
  `go infinite` then 40 `clean-tt`. **38, 41 and 36 reports on `013600d`
  against 0, 0, 0 on the candidate**, and the A/B taken inside the *same* build
  tree -- the one line added to the worktree that had just produced 36 takes it
  to 0, 0. `tt_reset`'s `memset` against `tt_store_entry` and `tt_get_entry` in
  the search thread, which is F11's claim. **The very first run reported 0 on
  the defective tree**: `FATAL: ThreadSanitizer: unexpected memory mapping`,
  this kernel's ASLR against TSan's shadow map, and a sanitizer that never
  started prints the same number as a fixed engine. `setarch -R` and a
  `grep -q FATAL:` guard are now in `TOOLCHAIN.md` with the numbers, because
  that zero would have closed the finding.

  **Five red-first cases, all five observed red**, three in
  `tests/test_engine.cpp`, one in `tests/test_search_params.cpp`, and the
  fold's precondition in `tests/test_uci_surface.cpp` -- "no two advertised
  option names collide when folded", 529 assertions over the 33 names the tune
  build advertises, green before and after. **DEC-178 records the three choices
  the `accepts` left open**: the fold reaches the search parameter names too,
  so S137's golden `Rfpmargin` assertion is **re-stated and not relaxed**
  (`RfpMargn` is the unknown option now); `Hash` answers two shapes rather than
  one, because "not an integer" about `99999999999999999999` is false; and the
  suite guards the **join** -- a deterministic observable in both builds --
  rather than the race.

  **INV-6 discharged**: `search_bench` depth 9 **121530 / 801481 / 72924**,
  best moves `c3d5` / `e2a6` / `d7c8q`, and `bench` **30046849**, the parent's
  total, so the commit says `No functional change`. No SPRT: nothing on a
  search path moved. DEC-141's second tier does not bind -- `make_move`,
  `unmake_move`, the generator and the search are untouched and no pruning rule
  was added -- and the extra gate last ran green on 2026-09-10, inside its
  weekly cadence. `ctest -L fast` 33/33 in both builds, format clean under
  `CLANG_FORMAT_MAJOR=22`. Closes `2026-09-10_adversarial-F11`, `-F12` and
  `-F13`.

  **The fast check over `5c11b72` found one real thing and it was trivial**, so
  it is fixed in `52f4c37` and earned no step: two comments in `src/` and
  `tests/` carried the *audit's* figure of 11 races where every document in the
  same commit carried this step's measured 38 / 41 / 36, and the audit says of
  its own number "the TSan run not re-executed" -- so the number a reader met
  first, in `src/`, was the unreproduced one. The same commit moves the
  `Status:` lines of F08 to F13 from planned to closed, which is the one edit an
  audit report takes. Everything else came back clean, including the fold's
  coverage (every name comparison folds, `Book File`'s value and the spin values
  verbatim, `search_param_set` on the canonical spelling), the reentrancy of the
  new join, and every added case's ability to fail. Two inputs that used to work
  and no longer do, checked against the project's own harnesses rather than
  argued: `+64` and `64 extra` -- `fastchess.sh` sends `option.Hash=16`,
  `rating.sh` `128`, `tools/spsa_driver.py` coerces with `int(...)`, so nothing
  here sends either.

- **S208 is complete and the fast check over S207 earned a new step, 2026-09-11
  morning.**

  **S208**: `load_FEN()` refuses **more than 16 pieces of one colour** and **a
  pawn on rank 1 or rank 8**, each naming its own reason through a new
  defaulted `std::string* reason` out-parameter that `set_position()` prints in
  S176's shape. Both reds observed on `23f926d` before a line was written --
  **`*** stack smashing detected ***`, exit 134, core dumped** in the Release
  binary for the 27-piece placement, and **`index 6 out of bounds for type
  'int [6]'`** at `src/evaluation.cpp:516` and `:529` under `build-sanitize` --
  and after the fix all three FENs are refused with **0** sanitizer reports.
  Four red-first ctest cases in S161's file, **all four observed red** by
  disabling the refusals (16 passed, 4 failed), two of them pinning the reason
  string's content so a refusal for the wrong reason is not green; four
  controls that must stay green and do.

  **The discovery is a decision, DEC-177.** The accepts asked for the refusal
  **and** an identical `bench` signature, and those contradict each other:
  `KILLER_POS` is one of the eight bench positions, carries **17 white
  pieces**, and `src/chesso.cpp`'s own comment kept it because "**the engine
  loads it**" -- which S208 makes false. Measured, not reasoned: with the
  refusal in and the constant untouched, bench reads **30746008** because
  `set_position` leaves the previous board and the set searches a duplicate.
  There is no resolution in which the constant stays unloadable. So the bound
  stays at 16 a side and **`KILLER_POS` becomes legal by deleting its h3
  pawn** -- python-chess says `Status.VALID`, and the **twelve promotions** and
  the **f5e6** en-passant capture it was kept for are intact, legal moves
  42 -> **48**. **`bench` therefore moves 26491479 -> 30046849 and the commit
  carries a `Bench:` line**, while `tools/search_bench.py` stays
  node-identical at `121530 / 801481 / 72924`: **no legal position's tree
  changed**, which is the half of INV-6 that speaks here. `MAX_MOVES` now
  states what defends it -- 218 published, 224 the audit maximiser's best under
  the 16-a-side rule and named as a search result rather than a proof, 270 the
  buffer -- and DEC-177 records that the honest rule is a bound on the move
  count and why that form was not taken. Gate **33/33 in both builds**, format
  clean; `test_uci_surface` needed no refresh and that was checked rather than
  assumed.

  **The Tier-1 fast check over S207's diff found one real gap and it is now
  S215.** `src/search.cpp` `search()` sets
  `state->root_history_size = game->history.size`, and **nothing in the suite
  reads that line**: mutated to `history.size - 1` the whole fast suite stays
  green while `bench` moves **26117924 against 30046849**, a tree 13 %
  different. What `- 1` does to a root with a history behind it is restore the
  pre-S207 behaviour for the root-recurrence class -- part of the F08 shape and
  the class whose removal moved bench 1.34 % at S207 -- but that is **not** what
  moved the bench: corrected by S215's fast check, see the S215 bullet. The three existing cases all miss it for reasons
  written into S215: two reach `search()` but land on the two-match or the
  pre-root path either way, and the boundary control sets the field by hand and
  calls `negamax()`. **Verified by measurement before it was written down**,
  not taken from the reviewer's prose. It is a test gap and not a defect, so
  the BUGS rule does not arm (DEC-171); S215 sits second in Open, behind S209.

  **The fast check over S208 then found three more, all confirmed at the file
  or by running the mutation rather than taken from its prose.** Two were mine
  and are fixed in this commit; one is **S216**.
  (1) **`MANUAL.md`'s `position killer` table still listed the pre-DEC-177
  FEN**, so the one statement that table makes -- "the FEN each one loads" --
  was false in the very commit that changed it. Fixed.
  (2) **The back-rank refusal had one untested corner.** Only a8 and a1 were
  pinned; mutating `square < 8` to `square < 7` loads a pawn on **h8** and the
  whole fast suite stays green at **33/33**, because no FEN anywhere in the
  tree has a pawn there. Both cases now sweep all four corners in both
  colours, and the two mutants S208 shipped without -- **M37** (that
  off-by-one) and **M38** (`> 16` becoming `> 17`) -- are in
  `tools/mutants/board.py` and observed killed, 1 of 20 cases each. The "pawns
  on ranks 2 and 7" case is restated as the control it actually is: a2 is index
  48, eight squares from the boundary, so it never probed anything.
  (3) **S216**: `adocs/data/S159_census_positions.txt`'s `promo-mess` row is
  the pre-S208 `KILLER_POS` and is now refused, and
  `adocs/data/S159_census_run.py` scrapes only `info ... nodes` -- so the
  refusal line is skipped and the row **above** it is reported under
  `promo-mess`. DEC-160 reads that census as the evidence that refuted S149's
  ageing reading before a match was spent, so this is DEC-142's "either end
  moved" trigger. The instrument has to fail loudly first; whether the census
  is re-derived is the judgement S216 takes.
  (4) And a **pre-existing** one, added to S213 rather than fixed here: three
  labels in the `test` command's table disagree with what the engine prints --
  `TRICKY_POS` and `FINE_70_POS` in their ponder move, **`CMK_POS` in its
  bestmove too**. Checked by running `test`. The fast check's fourth claim, a
  duplicated `MATE_IN_2_B_POS` label, is **not** a defect: the engine really
  does print the same line for both, those two positions being colour swaps
  rather than rank mirrors.

- **S207 is complete: H1 accepted, and the night's four document steps with
  it, 2026-09-11.** `--nonreg` `{-5, 0}` nElo: **LLR 2.96, H1 accepted, Elo
  +3.32 +/- 5.00, nElo +4.47 +/- 6.72, LOS 90.36 %, PairsRatio 1.05, Ptnml
  [434, 1025, 2135, 1079, 456] over 10258 games in 4 h 26 m 56 s**, candidate
  against `ae4eed4`, seed `20260911012459`. The pre-registered H1 reading is
  applied unchanged: **not a regression of 5 nElo or more, kept, no magnitude
  claimed** -- the point estimate is biased upward by the early stop (DEC-063).
  **The abort rule was checked independently**: `tools/forfeit_report.py`
  reports 0 forfeits of 10259 on each side against a 1.0 % rule, and
  `fastchess.sh` does not call that script, so its census and this one are two
  counts and not one. **The class is reachable in ordinary play and the run
  says how often**: 1690 of 10259 games, **16.47 %**, ended in a three-fold
  repetition draw, split 853 / 837 by colour -- even, so that is the rule's
  domain and not an attribution of the Elo. **DEC-136 fired for the first
  time**: the ledger is nine runs now, **mean 4 h 49 m, median 5 h 27 m,
  101366 games at 2334.6 an hour**, the fast class widening from 1 h 49 m to
  **2 h 28 m** because S207 sat outside the interval but close to the near
  bound and took 10258 games where the other three fast runs took 2522 to 6412,
  and the projection moving to **289 to 356 machine-hours**. S207 is also the
  ledger's one throughput outlier at 2305.7 against a 2328-to-2346 band, and
  the reason is known rather than guessed: the document lane ran on the same
  machine for its first forty minutes, costing about **1.2 %** -- three minutes
  on a four-and-a-half-hour run, which is what DEC-172's lane costs and it is
  worth it. Gate re-run on the idle machine after the verdict, **33/33 in both
  builds**, format clean; `Bench: 26491479`, verified against the very binary
  the match played by `sha256sum`.

  **The two mutants, re-validated through the tool after the commit** at
  `23f926d`, from a linked worktree as `tools/mutation_check.py` requires:
  **M35 killed, 1 of 33 failed, bench moved, 210 s** -- caught by *the root's
  own occurrence is the boundary* -- and **M36 killed, 1 of 33, bench moved,
  179 s** -- caught by that case **and** by *one occurrence before the root is
  not a draw*. Both signatures moved, so neither can be argued equivalent.
  **A process note, since the stamp cannot be edited**: S207's `done:` stamp
  says the two were "re-validated by `tools/mutation_check.py` from a worktree
  at this commit", and that clause was written **before** the run rather than
  after it -- the hand-applied observation it also records was genuine and
  pre-commit, but the tool run was a prediction at the time. It is confirmed
  here, and the lesson is the ordinary one: the tool needs the commit to exist,
  so the stamp should have said what it *would* run and this file should carry
  the result, which is what it now does.

- **How S207 and the night's four document steps were built, 2026-09-11 night.**

  **In progress, S207** (`plan_current/`): a repetition is scored as a draw only
  where the search itself walked into it, or on a third occurrence anywhere.
  `src/bitboard.cpp` gains `classify_repetition()`, which compares the matching
  history index against `search_state_t::root_history_size` --
  `history.size` as `search()` was entered, set there unconditionally because
  that is the only way into ply 0, so nothing had to be plumbed through the UCI
  layer -- and `negamax_at` scores `DRAW_SCORE` on its `DRAW` class alone.
  `is_position_repeated()` keeps its two-fold contract as a wrapper, which is
  what leaves `test_engine.cpp`'s two direct cases and `datagen`'s game-level
  adjudication untouched. **The F08 reproduction now answers the same way with
  the history and without it** -- `-1229 / 412680 / e7e5` both, against the
  reference's `score 0, nodes 5387, pv f6g8` with it, a false draw behind a
  tree **77 times smaller**; oracles re-taken, python-chess
  `is_repetition(3) False` and `can_claim_threefold_repetition() False`,
  stockfish depth 18 **-703**. `search_bench` depth 9 is **node-identical**
  (121530 / 801481 / 72924) while `bench` moves **26851183 -> 26491479**, and
  the pair is exact rather than contradictory: from a bare FEN the only class
  that can move is the root position recurring once inside the tree. Gate
  **33/33 in both builds**, format clean, Debug self-play **8 games, 0
  `Assertion`, 0 disconnect** (DEC-141), the `-569` golden re-derived,
  `--prose` / `--citations` / `--touches` / `--params` all clean. Two mutants
  added and both killed by hand -- **M35** (`>` to `>=`, the root's own entry)
  and **M36** (the pre-S207 rule) -- `mutation_check.py` re-validates them from
  a worktree after the completing commit. Three fast-suite cases moved: two
  **re-stated, not relaxed** under DEC-173, each now reaching its position a
  third time, and one of them ("a repetition is answered before the table is")
  was a **discovery** the accepts did not name. **The SPRT ran** from
  `adocs/data/S207_sprt.sh`, launched detached 01:24, out
  `/tmp/chesso_sprt_nonreg_20260911_012459`, log `.tuning/sprt_s207.log`, and
  it was **priced at 41861 games / 18.4 h worst case at the midpoint and 25591
  / 11.2 h on a bound (DEC-143) against a 40000-game cap**. It took 10258 games
  in 4 h 26 m 56 s, so the pre-registered third outcome -- no verdict at the
  cap, recorded as zero and kept -- never came near. The watcher fired on the
  `SPRT-RUN-DONE` marker; the verdict is in the bullet above.

  **Done, S185**: every figure the plan and thirteen pending files argue from
  now carries its source or the word unverified where it sits. The Ethereal
  ledger (commit `e755a814`) and the Stash `CHANGELOG.md` entries the block
  order rests on are cited where they are used, and on DEC-087 as a second
  `Amended:` line. **Three of the four numbers behind the zero-weight paragraph
  did not survive being looked up**: +12.99 is Weiss #241's *tempo*, not rook
  on the seventh, and +16.7, +4.2 and +9.2 have no source at all -- marked
  unverified with the nearest fetched figures beside them. "Single digits for
  most evaluation terms" is deleted as a claim, that ledger pricing search
  steps only. S129's 13 and 25 are restated as **six-men** figures against
  Minic's **0** at three men, with dropping the step named as an honest
  outcome. S109's "~0 alone" is re-pointed to pull request **#2401**. About
  thirty figures stay quoted as unverified with what was searched, handed to
  **S186** as a sixteen-row work list. Nothing reordered. Documents only, no
  `src/`, no run.

  **Done, S181**: every Lynx figure the steps and DEC-087 cite is banded, in
  `adocs/data/S181_lynx_bands.md` -- twenty-one pull requests to their
  `merged_at`, the two releases they fall between and each release's CCRL Blitz
  1CPU rating, from the list **computed 2026-09-05** and read 2026-09-11 with
  `curl` (`WebFetch` gets 403 there). The list drifts 0 to 4 points a
  fortnight, which is why the read date sits beside every figure, and **four
  releases the argument needs are not on it at all**, so an unrated bracket
  runs from the nearest rated release before the merge. **Three band words were
  low by 180 to 380 points and one claim was wrong in kind**: S099's "+11.4 at
  ~2850" is **3224-3291**, *above* the ~3100 that demoted S110 and S111, so
  DEC-087 (b)'s criterion separated nothing; **S023's "Lynx failed four SPRTs
  at ~2600" is withdrawn** -- unsourced, and Lynx *merged* capture history as
  #634 into v1.3.0 at a banded **2653** -- leaving Weiss #428's **-4.17** STC
  at an engine CCRL rates 3055 as the demotion's evidence; S098's cutnode,
  !improving and PV-min-moves leave the "Sub-3000 evidence" heading at
  **3119-3138**, TT-capture and deeper/shallower at **3138-3224**; S109's Lynx
  LMP band is **2420-2430**, 180 points *below* what was written, which
  strengthens that step. **S097's band caution of 2026-08-19 is closed in the
  README's favour** -- it said the repository's banding might sit ~300 low and
  it did. **DEC-176 rules that the record moves and no step's position does**,
  taken by the agent under the owner's delegation and naming itself as such:
  S023 stays in the reserve on the Weiss figure, S098 stays one step with three
  verdicts in that order, S099/S110/S111 stay where DEC-133 put them; what
  changes is what those files may claim. Documents and API reads only.

  **Done, S182**: the plan prices a verdict from its own ledger of eight runs
  since S105 and the answer is **287 to 354 machine-hours against a struck 75
  to 110**. Every row read from its owning step's completion stamp: **mean
  4 h 52 m, median 5 h 53 m**, 91108 games in 38.97 hours, **2337.9 games an
  hour with every run between 2328 and 2346** -- S148 on the workstation at
  2341 against seven MacBook runs, so the wall-time spread is game count and
  nothing else. Two classes, split by where the truth sat relative to the
  bounds: **fast 1 h 49 m** (S149, S107, S093 v1), **slow 6 h 42 m** (S108,
  S148, S093 v2, S130, S165), three of those five on a null or a small
  negative. The pending list is almost all slow class -- one block-class
  effect, S109, and two steps with a sourced figure above +20 -- so
  `3 x 1 h 49 m + 42 to 52 x 6 h 42 m`, with the flat mean's 219 to 268 hours
  named as the **floor**. DEC-143's worst-case table sits beside it, **18.4 h
  at the midpoint and 11.2 h on a bound at 2277** for the `{-5,0}` / `{0,5}`
  pair every strength verdict uses, and the slow mean is *below* the
  on-a-bound case because five of eight hit a bound. **DEC-136's rule is
  written twice**, in the section and beside the status-rewrite rule: a step
  that lands a verdict re-derives the section in its own completing commit.
  `DEV_MANUAL.md` "Which bounds" carries the same table. **S159 and S203 were
  checked as candidate rows and neither is one** -- S159's scheme was reverted
  unrun and S203's 3000 games were a fixed-rounds drift reading. Documents
  only.

  **Done, S183 -- and this is the one the owner has to read.** The Elo
  arithmetic is checkable for the first time and **it does not reach 3000, not
  at the midpoint and not at the high end**. `adocs/data/S183_elo_inputs.md`
  records every pending step's published figure, its source or the word
  unverified, the block it sums into and the per-block sums, with the selection
  rule and the discount rule both fixed **in writing before any sum was
  computed**. The record behind the discount: **eight** published-to-measured
  transfers, five with a figure on both ends, of which **two are exactly zero,
  one is the wrong sign, one is 0.10 and one is 0.33** -- mean 0.061, median
  0.00. The headline discount is **0.38**, the largest ever measured here, as
  the most generous reading that is still a measurement. **The plan's own +390
  to +680 lands at 2707 to 2817; the reconstruction from recorded inputs at
  2658.** 2817 is **183 short**, and adding the anchor's whole 121.8 Elo of
  internal disagreement in the favourable direction reaches 2939 and is still
  short. Block 1 reproduces almost exactly (+184.6 against the quoted +180 to
  +280), so the original arithmetic was the same shape -- but evaluation reads
  +54.3 against +90 to +160, speed +22.7 against +40 to +90, and **tuning's +50
  to +90 has no published input behind it at all**, its only evidence being
  S028's one-time +188.74 from hand-picked to fitted. "The midpoint clears
  3000" is **deleted**, struck and dated at the site, and the paragraph
  **names DEC-071 as the decision it puts to the owner**: whether the goal is
  met by this list, by a longer one, or only with the network premise DEC-054
  parked. **No step's order and no step's content changed on the result** --
  the step's `excludes:` forbids it, and this is a question for the owner and
  not a ruling the agent should take. A convergence worth noting: the
  2026-08-23 estimate reached 2600 to 2650 by summing what had already been
  kept; this reconstruction reaches 2658 by summing what is still owed. Two
  arithmetics, opposite directions, 8 to 58 Elo apart. Re-derived when S024 and
  S109 land -- 46 % of the raw sum between them. Documents only.

  **Next**: S208 and S209, Open entries 3 and 4, node-identical and owning no
  run -- the load-boundary crash and the `setoption` protocol defects, both
  reachable on the shipping UCI surface. Then S024's first verdict as the next
  night's run, which is also one of the two inputs S183's arithmetic is
  re-derived on. **The one thing waiting on the owner is S183's result**, which
  is a DEC-071 question and not an agent's ruling.

- **The 2026-09-10 audit is digested and the plan is re-sorted for the 3000
  mark, 2026-09-11 (DEC-170 to DEC-175).** Thirty-seven findings, five high:
  eight steps, **S207 to S214**, six decisions, six pending files amended (five
  for findings, S151 for its design), two `specs.md` sentences corrected, the report's `Status:` lines moved to
  `planned`/`accepted`. The owner's instruction that shaped it: Elo first,
  bugs first only where they are reachable in play or can move an evaluation,
  a UCI answer or a line -- DEC-171 writes that scope into BUGS. **Every
  ruling was taken by the agent under the owner's delegation of 2026-09-11**
  (engine-related questions are the agent's; the owner is asked only when the
  workstation is at risk, the ethic would change, or the goal is not served)
  and each names itself as such, so the owner's veto on reading is the check:
  DEC-173 changes the repetition convention and re-states a test; DEC-174 makes
  resign adjudication two-sided; DEC-172 takes S151's cheap design and moves
  S039 beside S122; DEC-175 puts parallel search in phase two.
  **Tonight's run is S207**, Open entry 1: the F08 repetition fix, one
  `--nonreg` SPRT priced at 4 to 18 hours, launched detached with a watcher
  through `Monitor`. While it plays the coordinator takes the filler behind it
  in order -- S185, S181, S182, S183, documents only -- and when the verdict
  lands it completes S207 and continues down the list without waiting for the
  owner (DEC-172's clause): S208 and S209 in the morning, node-identical and
  owning no run, then S024's first verdict as the next night's run. `plan.md`'s cost and Elo paragraphs
  stand as written until S182 and S183 rewrite them. **That digest commit**
  changed no `src/`, carried no `Bench:` line and started no run; the bullet
  above is what happened after it.

- **S195 is complete: three cases hold what an SPRT cannot see, and two of the
  guide's four predictions about them were wrong.** `tests/test_engine.cpp`
  gains *node-limited searches repeat across ucinewgame* -- two sequences at ten
  node limits, each run twice with `ucinewgame` between, **40 searches and every
  pair identical** on the last `info` line's `nodes` and on the `bestmove` line,
  ten and nine distinct counts against a floor of five -- *a repeated go depth
  is cold only across ucinewgame* -- **77612 cold, 77612 cold again, 8460 warm**
  at depth 8, the inequality holding at every depth 5 to 10 -- and *bench
  searches its last position cold*, `bench 9`'s eighth position 128048 against a
  cold standalone 128048. **Four mutants, every one built and observed, all
  reverted**: `tt_reset` out of `reset_for_new_game` reddens case 1 at the first
  limit, `113 then 232`; **`set_position`'s reset gate never firing leaves all
  three green**, where the guide predicted case 2 red -- S189's
  `reset_for_new_game` calls `tt_reset` directly as well, so `ucinewgame` clears
  the table whatever `set_position` does; deleting that gate so every `position`
  resets, the F08 hazard silently repaired, reddens case 2 at `warm repeat 77612
  equals cold 77612`; cutting both paths reddens all three. **The bench clause
  needed no code and case 3 cannot prove what it was written to prove** -- S189
  landed first, so `command_bench` already resets per position, and commenting
  that call out leaves `bench 9`'s output **byte-identical**, every `nodes`,
  `score` and `pv` field and the signature total `1600876`, because the eight
  FENs are pairwise distinct and `set_position` already resets on a differing
  FEN. The loop's reset guards a bench list that ever *repeats* a position and
  no test can build that from outside, so case 3 ships as the end-to-end
  coldness property with the blind spot named at its site. Section 4's constants
  were re-derived here and **the MacBook's did not transfer** -- 77612 against
  its 77617 -- which is S203's key redraw landing in between, and is why nothing
  in these cases is compared against a recorded number. Goldens: none (DEC-142).
  `DEV_MANUAL.md` "Measure" gains the warm-table paragraph; `MANUAL.md` checked
  and unchanged, the owner deciding on 2026-09-10 that neither candidate
  sentence is wanted. No `src/` change, no `Bench:` line, no SPRT. Cost Release
  0.343 / 0.021 / 0.205 s, Debug 7.01 / 0.47 / 5.19 s inside a 65.26 s
  `test_engine` against that build's 600 s ceiling. Gate green in both builds,
  33/33 and 33/33 under `CLANG_FORMAT_MAJOR=22` (DEC-146), format clean. Closes
  `2026-09-04_test_review-F08` with S203 and S189: all three items of its
  suggested resolution are done, and the LMR `std::log` line in its evidence
  asked for none and stays recorded, not repaired.

- **S206 is complete: `truncation_scan`'s drift is two commits and neither is a
  defect.** Both readings reproduce at their own shas -- 135399 / 99 / 30 at
  `77d7450`, 138331 / 105 / 33 at HEAD, same corpus, same command, and the
  corpus file has not been rewritten since 34 minutes before `77d7450` itself.
  **`21b4a21`, S085's SPSA vector, is the whole of 99 -> 105 and 30 -> 33**: it
  raised `LAZY_EVAL_MARGIN` from 150 to 184, and `eval_model::evaluate` and
  `evaluate()` both clamp the tapered mobility-plus-king-safety sum at that
  margin, so **wherever the clamp binds the two agree exactly and the taper's
  truncation residual is not there to be measured**. Raising the margin
  unclamps a band of positions and it reappears, which is why the count went
  *up*. **`883c255`, S104's `CHESSO_ARCH=native`, moved the 2.0 column alone by
  -991**: `-march=native` contracts the model's
  `mobility[t] * params[...] + sum` into an FMA and the double moves by an ulp,
  visible only at a threshold rows sit exactly on -- a residual is a multiple of
  1/24 and **2.0 = 48/24**, where 2.8 is not. **Both proved by counterfactual,
  not by argument**: HEAD with the margin edited back to 150 reads
  **134408 / 99 / 30**, its parent's reading to the row; `883c255` built with
  `-ffp-contract=off` reads **135399** again. **The candidate the step was
  opened on moves nothing** -- `ecd735e` (S161, `load_FEN`) is dated after
  `21b4a21`, which already prints what HEAD prints. The weights were excluded by
  measurement and not by the anchors alone: every non-zero entry of the model's
  starting vector is identical at the two shas, and so is every extracted model
  feature on a row that entered the set. Classification **(a) twice**, so the BUGS rule
  does not arm. **DEC-169** adds the second trigger -- the four pinned positions
  are re-derived after a refit *and* after any move of `LAZY_EVAL_MARGIN`, which
  is what S039 exists to do -- and the `GOLDEN (DEC-142)` note in
  `tests/test_eval_model.cpp` says so at its site. **The evidence script's own
  first run was vacuous on two rows**: it passed the word `HEAD` to
  `git -C "$WT" checkout`, which resolves in the worktree, so both HEAD rows
  re-measured their predecessor and both still printed `as recorded`. The sha is
  resolved in the repository now and the margin counterfactual greps for the 184
  it replaces. No `src/` change, no `Bench:` line, no SPRT. Gate green in both
  builds, 33/33 and 33/33 with `CLANG_FORMAT_MAJOR=22` (DEC-146), format clean.

- **S205 is complete: `test_perft` gates nine columns, not five, and the four
  it never compared do not mean what they look like.** `perft()` counts
  `checks`, `discovery_checks`, `double_checks` and `checkmates` at the depth-1
  leaf, `load_expected_stats` parses the two it never parsed, `columns_match`
  decides on all four. The census, `adocs/data/S205_check_columns.py`,
  reproduces the 42/30/30/42 the step was opened on and corrects it where it
  matters: **the glob count includes the six layers of the commented-out
  `debug_perft.json` and seven layers past their `depth_limit`**, so what a run
  compares is **58 layers, 33 with checks and checkmates, 21 with discovery and
  double** -- 29 distinct layers, **84 expected values in tracked assets that no
  run had ever read**. **The obvious reading of the four columns is wrong**: a
  check, two checkers, a checker that is not the mover matched 22 of the 29 and
  put the run red on seven layers. Classifying every affordable layer under each
  candidate reading left exactly one that matched all 29 -- `checks` includes
  mates, `checkmates` is every mate, and the two classification columns describe
  the **non-mating** checks only, are exclusive of each other, and count a
  castle's rook as having moved. **Kiwipete at depth 5 decides both exclusions
  and nothing else does**: 8 of its 2645 two-checker leaves are mate (the
  asset's 2637) and 12 of its 19895 discovered checks are castles (the asset's
  19883). **The first conclusion was that the asset was wrong, and the tool
  killed it** -- the 2645 paths were dumped and replayed through python-chess,
  a different board and legality test and check detector, **2645 of 2645 legal
  and ending in a two-checker position**. Engine and asset are both right and
  were counting different things; nothing under `tests/assets/` was touched,
  which is what the step's `excludes:` is for. Cost **+27 %**: 53.35/52.67/52.69
  s before, 67.47/67.23/67.39 s after, **68.90 s through `ctest -L slow`**
  against the 53 s the step quoted -- so the extra gate's `perft` stage is now
  about 68 s, not the 54 s S197 recorded. It is cheap because `generate_moves()`
  is legal-only, so a mate is an empty move list needing no make/unmake, and
  because the whole classification is paid at check leaves alone. Each of the
  four was **observed red** under a scratch asset one off at Kiwipete depth 4 --
  one red cell each, in the perturbed column and no other -- then restored to
  exit 0. `--max-nodes 5000000` recounts 23 layers up to Kiwipete depth 4 with
  python-chess as the board and every column agrees. No `src/` change, no
  `Bench:` line, no SPRT. Gate green in both builds, 33/33 and 33/33, format
  clean, slow label green.

- **S192's Tier-1 fast check found five real defects and a proved coverage gap;
  all six are closed in the commit after it** -- `plan_done/` is history and was
  not edited. (1) `test_invariants.cpp`'s five census floors were self-declared
  "Goldens (DEC-142)" with a script and a margin but **not in the marker's
  shape**, so the `grep` listing `DEV_MANUAL.md` introduces as "list every site"
  missed all five; S190 landed after F03 was written, so the pass never saw
  them. Exactly the miss the stamp records for `test_uci_surface.cpp`. (2) A
  **thirteenth golden**: `REQUIRE_EQ(lazy, 383)` sits beside the symbolic
  assertion in "a stand pat that is itself a bound is still capped", is
  `QUIET_ROOK_EVAL_CHEAP - LAZY_EVAL_MARGIN`, and moves with a refit **and**
  with any SPSA run that touches the margin -- so the new block's "a refit edits
  two lines" was wrong by one. (3) The node band's Margin line quoted the
  original 4x and a fifth, ten lines under the re-taken count at which the real
  headroom is 2.4x and a ninth -- the wrong number for judging a future red.
  (4) "holds at any weights" was false of the property beside the truncation
  positions: the same file asserts that a fitted tempo takes the bound to 3.833
  and the tolerance to 4. (5) `adocs/testing_strategy.md` cited
  `test_engine.cpp:1005` for the case this step moved to 1192, which is the
  `file:line` form DEC-135 bans; that file is outside the prose check's set, so
  nothing caught it. `adocs/data/S192_node_budget.py`'s source slice ran to end
  of file and would have reported **200000**, a later case's budget, if the case
  ever lost its own line -- observed, and it is bounded at the next `TEST_CASE`
  now.
- **The gap was measured, not argued, and it was real.** With the old case's
  `REQUIRE(scaled.drop > 0)` gone, nothing in the suite failed if the iteration
  loop handed `search_time_scale_percent` a constant zero fall: the identity the
  replacement asserts holds trivially, because the loop records the same zero it
  passed. **`M34_score_drop_always_zero` survived the whole fast label at
  `0abe648`**, and `bench same` -- the node signature is blind to it, so only a
  test could ever catch it. The first form of the mutant was **stillborn**,
  `variable 'previous_score' set but not used` under -Werror, and carries a
  `(void)` of both orphaned symbols now, which is the class `tools/mutants/search.py`
  already documents. What kills it is a subcase asserting **over a set rather
  than over one position's tree**: four positions driven at depth 8, at least
  one must report a fall. All four do -- 22, 5, 15 and 31 cp -- so a search
  change has to stop every one of them falling before it reddens. Same shape as
  `test_mate_carry`'s majority (DEC-162). Red first, then green: 1 of 33 on
  `REQUIRE( falling > 0 )`. The registry is 41 mutants now.
- **S192 is done, 2026-09-10: twelve goldens in `tests/` are named, scripted and
  paired with a property, and the case that fired on eight search mutants for no
  defect is a construction now.** `grep -rn 'GOLDEN (DEC-142)' tests/` lists
  thirteen sites over eight files -- twelve goldens and `MATE_DEPTH_SLACK`
  marked **not** one, because it is a budget the mate reading is taken under and
  widening it changes what the floor means. Two sites the step's inventory did
  not have: `test_uci_surface.cpp`'s option-line count, already a golden with
  its own re-derivation since S193 but not in the marker's shape, and
  `test_eval_model.cpp`'s four truncation positions, which were the inventory's
  one open row and are resolved by naming the tool that re-derives them.
  Closes `2026-09-04_test_review-F03`.
- **`.tuning/anchors.py` is `adocs/data/S192_anchors.py` and it runs here.**
  Root from `__file__` instead of the hard-coded `/home/max/ws/chesso/`, every
  `file:line` citation replaced by the `TEST_CASE` title the anchor belongs to
  (DEC-135), contract unchanged -- **`10 of 10 reproduced`, exit 0** at
  `4732d8f`, logged. The Parked item that called the fit scripts gitignored is
  retired: all five have been tracked since `c56ab41` and this one has left
  `.tuning/` altogether. What stays parked is the corpus.
- **The old soft-limit case was observed red under M06a before it was deleted.**
  `REQUIRE( scaled.drop == 0 )`, one case of 56 in `test_engine`, and
  `kills.txt` carries eight such rows over the 2026-09-04 mutants -- every one
  on `scaled.drop`, none of them a time-management defect. DEC-168's form A
  replaces it: the suite's tool-verified mate in one at depths 2, 5 and 8, where
  the loop counts **stability exactly `depth - 1` and a fall of exactly 0** on
  any machine and after any search change. The fall half keeps one subcase with
  **no precondition on the number** -- it reports `stability 1, fall 22 cp,
  scale 107%` and asserts only the identity against
  `search_time_scale_percent`.
- **The four-mutant re-run: 4 of 4 killed, 431 s**, against a worktree carrying
  this step's `tests/`. **M06a is caught by S191's "a null-move fail-high
  against a mate returns the bound"**, so section 10's question 2 is answered by
  measurement and no ply-floor guard case is owed -- the ply floor is covered by
  construction, not by a golden and not by `test_mate_carry` as the step
  expected. The red lands on that case's *precondition*, because the mating node
  sits at exactly `RFP_MIN_PLY` and one ply lower lets reverse futility answer
  with a static score; whoever wants the louder signal has S191's pattern. M09
  likewise on an S191 case. M29 on fifteen cases in three binaries, M30 on its
  two predicted properties -- and **both lost `test_engine`**, which was the
  replaced case. That is the trade, made deliberately.
- **The node band was re-derived and it is the one number that had drifted.**
  `adocs/data/S192_node_budget.py` reads the count off a `MESSAGE` the case
  gains: **179851 nodes**, against the 109575 the band was placed on in
  2026-08. The tree has grown 64 % under a band that did not move, so the budget
  is 2.4x the count rather than 4x. Still inside the middle half, which is the
  condition on leaving 440000 and 20000 alone, and `excludes:` forbade moving
  them anyway.
- **`truncation_scan` no longer reproduces its recorded counts, and that is
  S206.** Run to verify the inventory's last open row, it reads **10795695 rows,
  138331 past 2.0, 105 past 2.8, 33 at 2.875** against `DEV_MANUAL.md`'s
  135399 / 99 / 30 on the same corpus file. Deterministic over two runs, all
  four pinned positions still in the set, the suite green, and the weights
  proved unmoved -- the anchors script reproduces all ten at the S076 values. It
  is not diagnosed and is not called a bug until it is: the manual carries the
  measured numbers and says so, and S206 bisects it. The instrument re-chooses
  those four positions at every refit (DEC-057), so it wants explaining before
  S126.
- **`DEV_MANUAL.md`'s mate-in-three floor said 8 and the test has said 11 since
  2026-09-01.** S168 enlarged the set to 82 the same day S154 re-derived the 8,
  which moved both ends to 12 and 10. Corrected here, a month late, which is the
  class of drift the new "Goldens: named, scripted, re-derived" section exists
  to stop.

- **S197 is done, 2026-09-10: the second tier of the gate is a script with one
  marker, and its first green run says the tree is clean under instrumentation.**
  `tools/gate_extra.sh`, five stages cheapest first, **GATE-EXTRA-DONE 5 stages
  768 s** -- `prose` 0 s, `citations` 1 s, `debug` 258 s over the six binaries
  that drive `make_move` (INV-2, INV-4), `sanitize` 455 s over the whole `fast`
  label plus `bench`, `perft` 54 s (INV-1). **Zero ASan, UBSan and LeakSanitizer
  reports** across 33 binaries under instrumentation; the option had not been run
  since the 2026-08-13 audit, so this is the first statement about the tree in a
  month and it is a clean one. DEC-167's cross-build assertion green: sanitizer
  `26851183 nodes 1385612 nps` against Release `26851183 nodes 7651211 nps`, the
  instrumentation costing **5.5x on nps** where ASan's own documentation says 2x
  for itself alone. DEC-141 clause 3 now has a tool to name.
- **The flag that makes the option usable as a stage had never been there, and
  its red was observed.** Without `-fno-sanitize-recover=undefined` a planted
  signed overflow printed `runtime error: signed integer overflow: 2147483647 +
  1`, ran the whole command to completion and **exited 0** -- a stage on that
  build is green on a log carrying undefined behaviour. With it, exit 1 at the
  first check. **The guide's literal plant produces no report at all under
  g++ 13.3 at -O2**: gcc folds `int x = INT_MAX; x += 1; (void) x;`, and a
  `volatile` seed is still not enough because the result is dead -- it takes a
  store back. Anyone re-running section 6 (a) needs the third form.
- **The smoke test found a defect in the script before a single stage ever
  ran.** `repo="$(cd "$(dirname "$0")/.." && pwd)"` -- the form `rating.sh` uses
  -- expands to nothing on a `PATH` without `dirname`, so the root became `/`
  and all five stages ran against the filesystem root while the script printed
  `gate_extra: repo /` and said nothing was wrong. It is `rating.sh`'s bare
  `$(nproc)` again (S177), caught on the day the script was written rather than
  by an audit months later. The root is `${0%/*}` now, a parameter expansion,
  and is **checked rather than trusted**: a root with no
  `tools/plan_prose_check.py` is a named refusal before any stage.
- **Seven sandbox cases at 0.46 s in the fast label, 12 of 12 cuts killed over
  three passes** -- and three of the seven exist only because the earlier passes
  left something alive. Pass 1, 8 cuts, killed 7: `M5_no_bench_compare` survived
  because the test asserted only that the bench comparison had *run*, both stubs
  printing the same total either way. Pass 2, 12 cuts, killed 10:
  `M11_debug_is_release` and `M12_no_root_check` survived because nothing read
  the Debug directory's build type and nothing ever gave the root check a wrong
  root to refuse. Cases 4, 5 and 6 close all three. The generator is
  `adocs/data/S197_script_mutants.py` and it refuses unless every anchor resolves
  exactly once.
- **Run 1 failed, it was not the sanitizers, and the precondition is now written
  where the launch line is.** `GATE-EXTRA-FAILED: sanitize` after 514 s, caused
  by `test_clang_format_script` -- one of the 33 binaries stage 4 runs under the
  sanitizer -- failing to resolve its pinned major, which is **DEC-146** and
  wants `CLANG_FORMAT_MAJOR=22` exported. A detached run inherits no interactive
  shell's environment and the `nohup` line did not carry it. The script behaved
  correctly and the marker named the stage that failed; what is wrong is that
  the marker alone blames the sanitizers for something that is not theirs, after
  the build has been paid for. **The extra gate presumes the automatic gate is
  green**, and that sentence is now in the script header and in `DEV_MANUAL.md`.
  A pre-flight refusal was considered and declined, with the reason in the stamp
  so nobody re-derives it.
- **A watcher cried wolf and the reason is worth keeping.** The first watcher on
  run 2 reported "died with no marker" while the run was in stage 3, because the
  pid came from `pgrep -f gate_extra.sh` -- which also matches the wrapper shell
  whose command line contains the launch string, and that shell exits seconds
  later. `pid=$!` in the launching shell is the only form that names the run.
  `DEV_MANUAL.md` says so now. DEC-061.
- **S197's Tier-1 fast check found four real defects and all four are closed**,
  in the commit after it -- `plan_done/` is history and was not edited. Each has
  a case observed red against the committed script first, and the mutant file
  went from twelve cuts to sixteen, the test from seven cases to ten,
  **16 of 16 killed**. (1) `configure_if_needed` returned early on any
  `CMakeCache.txt`, so a `build-sanitize` left without `-DSANITIZER=ON` would
  build an uninstrumented tree, run the whole fast label with no sanitizer,
  match the Release bench total trivially and report **`sanitize ok`** -- the
  most expensive stage returning a green that means nothing, and DEC-052 records
  VS Code's CMake Tools writing into a directory of this tree uninvited, so it is
  live. It configures and then verifies the cache now. (2) **A `fail()` reached
  from inside a stage printed no marker anywhere a watcher polls**: the driver
  redirects each stage's whole output to that stage's log, a function is not a
  subshell, and `exit` left stderr pointing at the log while `marked=1` silenced
  the trap. Measured before the fix -- terminal log empty, marker in the stage
  log. Markers go to `exec 9>&2` now. No stage calls `fail` today; this keeps the
  header's promise true for whoever adds stage 6. (3) Only the sanitizer's bench
  line was pattern-checked, so a stale `build/` whose last line is
  `bestmove e2e4` made the Release "total" the word `bestmove` and the stage
  **accused the tree of a memory bug and escalated it to the BUGS rule**. Both
  sides go through one validator now. (4) A prose claim that was false:
  `--citations` has been in the fast label as `test_plan_citation_freshness`
  since S187 (DEC-159), so **stage 2 catches nothing the automatic gate cannot**
  -- only `--prose` is out. The script header and `DEV_MANUAL.md`'s table said
  otherwise. The stage stays, because the `accepts:` named both modes, and its
  row now says what it is.
- **`M13_trust_any_cache` survived the first attempt to kill it**, and the reason
  is the shape of the sandbox: every case there starts with no `CMakeCache.txt`,
  so the early-return branch was never reached and the mutant was behaviourally
  identical. Case 9 plants a wrong cache *before* the run and asserts the
  configure still happens. A cut that only bites on pre-existing state needs a
  case that creates that state.
- **Re-run after the fixes: `GATE-EXTRA-DONE 5 stages 774 s`**, against 768 s
  before them -- `prose` 0 s, `citations` 0 s, `debug` 260 s, `sanitize` 460 s,
  `perft` 54 s. Always configuring instead of returning early costs about six
  seconds. Zero ASan, UBSan and LeakSanitizer reports again, and
  `INV-6 across builds: both 26851183 nodes`.
- **DEC-167 records the owner's five section-10 answers**, 2026-09-10: assert the
  cross-build bench totals in stage 4 (so `adocs/specs.md`'s INV-6 row gains the
  clause and the file joined `touches:`); six Debug binaries, the three mate ones
  stay out on cost; **no** `tools/coverage_unexecuted.py`, here or as a step of
  its own, so the `llvm-cov` recipe is documented and the comparison stays by
  eye; the two `SANITIZER` flags go in; and the cadence gets its own `status.md`
  bullet rather than a clause inside `Watching:`. Question 3 was already answered
  by DEC-166. **No coverage run was made** -- the recipe is documented, not
  exercised, so nothing here claims a fresh coverage number.

- **S196 is done, 2026-09-10: the fault-injection driver is `tools/mutation_check.py`
  over 40 tracked mutants, and the fast suite's kill rate is a measured number
  -- 39 of 39, 100 %.** 3948 s wall on this workstation, worktree at `44440b4`,
  baseline green at 31 tests with `bench 26851183`. M26 is the one declared
  equivalent; nothing survived, nothing was stillborn, nothing unmeasured. The
  table is `adocs/data/S196_full_pass.tsv` and it is what the next pass diffs
  against. **M19 is dead**: the fifty-move survivor of the 2026-09-04 review is
  killed by `test_search` alone, which is S193 proved by an instrument.
- **The run found a bug in the rule the guide wrote for it, and it cost two of
  the forty. DEC-165.** "Any `(Timeout)` row is `unmeasured`" was written
  against a real trap -- a busy machine hits a 60 s ceiling and a naive parser
  reads the non-zero exit as detection. But `M22` and `M31` hang
  `test_uci_surface`, **9.26 s on the unmutated worktree in the same run**,
  while four and two other binaries fail on assertions. The hang is the
  mutant's, so the prescribed re-run on a quiet machine reproduces it exactly
  and the kill is hidden for good. Narrowed to "only when the ceiling is the
  whole evidence", with two further readings of the same evidence: `killed` is
  decided before `unmeasured`, so a mutant that leaves the engine unable to
  print a bench line still reports its kill, and a non-zero `ctest` that ran
  nothing reads `unmeasured` rather than a kill. The first pass was abandoned at
  17 of 40 and re-run whole rather than patched with two rows from a second run.
- **19 of 40 are single-binary kills and not one of them is `test_mate_carry`
  alone** -- 15 `test_search`, 3 `test_engine`, 1 `test_chesso`. That is F02 and
  F03 closed and counted: the review found five guards whose only catcher was
  that golden. `test_mate_carry` is red on 11 of 40 now against 21 of 22 search
  mutants in the review, which is S204's narrowing from the other side. The
  bench signature is still blind to **10 of 40**, so a still bench argues
  nothing.
- **The tool's own gate is 21 cases in 1.33 s**, over a throwaway git repository
  with `cmake`, `ctest` and the engine stubbed on PATH -- it never builds the
  engine. Every verdict branch and every refusal in `validate()` has a case, and
  **each was observed red under a cut to the guard it names**. All 40 mutants
  compare byte for byte against `adocs/data/2026-09-04_test_review/mutants.py`
  and `adocs/data/S191_mutants.py`, both unchanged.
- **DEC-166 answers section 10.** The full pass stays on demand and out of
  S197's weekly script -- so **S197 needs no amendment to its `accepts:`**, and
  its own section 10 question 3 is answered there -- and the table is evidence
  under `adocs/data/` rather than living only in a stamp. The self-test is
  registered, which is why `tests/` joined `touches:`.
- **S196's Tier-1 fast check found three real defects in the tool and all three
  are fixed**, in the commit after it -- "Close the three defects S196's fast
  check found". Each has a case, each observed red against the committed tool
  first. (1) `apply_mutant` sat outside the `try` that reverts,
  so a mutant whose pairs interact -- pair 0 making pair 1's anchor ambiguous --
  fails while applying with pair 0 already on disk, and every later run refuses
  at the clean-src check until somebody reverts by hand. (2) **`SIGTERM` skipped
  every `finally`**, which is what happened when the pre-fix pass was stopped
  today: the worktree was left with `src/search.cpp` mutated and was cleaned by
  hand without the defect being recognised. It is caught and raised now. (3)
  ctest's trailing summary has no `Start` line to close it, so a `FAIL:` there
  was reported as the last binary's own assertion -- visible only when that
  binary has no output of its own, which a ceiling hit is. **The 39 of 39 pass
  is unaffected**: no kill line in it carries summary text, and none of the
  three can move a verdict. Self-test 21 cases to **24, 1.61 s**.
- **What the next step should know.** A `--only M<nn>` row costs 168 s including
  the baseline, which is what DEC-141 clause 2 charges a completing step. Do not
  run a pass beside a match. The worktree recipe needs a line `fastchess.sh`'s
  does not -- `git -C .ref-builds/mut submodule update --init tests/doctest
  tests/json` -- or the run refuses at "the unmutated worktree does not build",
  and `CLANG_FORMAT_MAJOR=22` must be exported here or the baseline suite is red
  and the run refuses before the first mutant (DEC-146).

- **S191 is done, 2026-09-09: every guard on null move pruning, reverse
  futility and late move reduction now has a case of its own, and each was
  observed red under the mutant that removes it.** Thirteen cases in
  `tests/test_search.cpp`'s new `search: pruning and reduction guards` suite.
  The five removals `2026-09-04_test_review-F02` found were caught by
  `test_mate_carry`'s per-game floor **and by nothing else**, and three of the
  five left the depth-9 bench identical, so INV-6 would have passed them too.
  The 104 proved defender nodes of `adocs/data/S165_defender_set.tsv` are a
  registered fixture -- until now nothing in `tests/` read that file at all.
- **The observable is `search_node_probe_t`, and making it free took a second
  design.** The owner chose section 10's counter route over the
  transposition-table one, so `src/` joined `touches:`. The first version
  resolved `state->probe` at every interior node and tested it once per move:
  **1.49 % fewer nodes per second, sd 0.66 % over 13 interleaved paired
  `chesso bench` runs** -- resolved, not this machine's noise, and moving the
  field beside the hot ones did not recover it. `negamax` is now
  `negamax_at<bool PROBING>`, `<false>` everywhere the engine searches and
  `<true>` only at the node `negamax_probed()` drives; the recursion is always
  `<false>`, exact because a probe names one ply and every child is at another.
  Re-measured over **33 pairs: 0.14 % +/- 0.24 % at 95 %**. INV-6 identical at
  depths 9 and 12, `bench 26851183` on both sides, so the commit carries
  `No functional change`.
- **One case needed a beta the guide did not name.** M04's mating node sits
  three plies below the driven one, which is exactly `RfpMinPly`, and it
  inherits the drive's beta: at the guide's 100 reverse futility fired there on
  a static score a queen up and the precondition read
  `REQUIRE( 742 >= 48000 )`. Beta is 40000, still inside the mate band. It is
  the reverse-futility comment's own "a mate deeper than ply 3 can still be
  missed for an iteration", met head on.
- **Six mutants had no home** -- the 2026-09-04 file is append-only -- so
  `adocs/data/S191_mutants.py` holds them in the same shape, and S196 folds
  both. All **39 anchors across the two files** still resolve uniquely against
  the templated source; the red pass was run twice, once before the templating
  and once after. Debug self-play: 8 of 8 games, **0 `Assertion`** in a log
  carrying 78601 engine-stderr lines (a first attempt greped a 0-byte log and
  was thrown away). `test_search` **2.16 s Release, 67.08 s Debug**, so it
  needs no binary of its own.
- **`tools/gate_extra.sh` and `tools/mutation_check.py` do not exist yet** --
  S197 and S196, now Open entries 2 and 1 -- so DEC-141's other two clauses had
  no tool to run and the mutation pass above is the hand form S196 will fold in.

- **S193 is done, 2026-09-09: the one injected bug that survived the whole fast
  suite is dead, and seventeen assertions that could not fail can now.** The
  survivor was the fifty-move boundary. The only direct case searched a root
  already at clock 100 -- which the root exemption makes unreachable, every node
  below it being at 101 or more -- so mutant M19, `>= 100` becoming `>= 101`,
  passed **all 27 binaries**. The new pair puts the root a halfmove lower and
  reads **0 at clock 99 against 929 at clock 98**, red under M19 at exactly
  those two numbers while the old case stayed green in the same run. Its
  preconditions come from the engine's own generator: 21 replies, every one
  quiet, all landing on clock exactly 100, none mate, none insufficient
  material -- and **the guide's own precondition was wrong**, asking for no
  check on any child when several of the queen's moves give check; what the
  block excepts is mate, spelled the way `negamax` spells it.
- **`test_perft` stopped exiting 0 on a run that checked nothing.** Its two
  `assert`s were compiled out of the Release build the gate runs, and only
  `nodes` reached the pass flag. Observed before and after: **a missing asset
  0 -> 2** (and the old run printed nothing at all), **an unparseable one
  0 -> 2**, **a wrong `captures` column 0 -> 1**. The dead `RUN_THREADS`
  branches went with it -- they named `g_board`, `g_globals` and a `make_move`
  signature that has not existed for years, and could not have compiled.
- **Three numbers say what the vacuity was worth.** The case that claims to
  bound a 200 ms search measured **0 ms** and now measures 239, 235, 238, 232
  and 239. `./test_chesso -tc="Basic test"` answered **16 moves against 20**
  before the tables moved into a fixture. And the lazy-margin bound, dropped as
  the `std::clamp` restated, left a measurement behind that proves it: **the
  widest correction over 2696 positions is 184, which is `LAZY_EVAL_MARGIN`
  exactly**, so the assertion had been reading a saturated value.
- **Six `src/` mutants, six reds, every one reverted and the tree confirmed
  clean.** M19 for the boundary; the embedded book loading after a file load
  fails (R3); the repetition window ignoring the clock (R13); pins ignored in
  `generate_moves_body` (R7, which names the offending move and beside which the
  pre-existing `make_move` assertion stayed green -- the vacuity); the book
  move's from-rank flipped (R17). R4's guard was shown to bite on a tool-checked
  mated position instead, no mutant existing for a missing precondition.
  **R12 stays unregistered**: `2026-09-04_adversarial-F01` still reads
  `Status: open`, which is the accepts' own condition. **No `src/` change, no
  Bench line, no SPRT, no Debug self-play owed.** DEC-163 amended the accepts
  before the step started -- the R2 clause named `ucinewgame`, which sets the
  stop flag rather than clearing it -- and widened it to R13 to R17. Closes
  `2026-09-04_test_review-F04`, `-F05` and `-F09`.
- **S204 is done, 2026-09-09: `test_mate_carry` stops firing on any change that
  moves the tree, and starts firing on three separate things that matter.**
  DEC-162. The count that retired the old shape, from the two grids the step was
  created with: over the nine stride-1 budgets `C_mate7_depth11` reports a mate
  line in **one cell** at `c982f9d` and its configured budget is that cell,
  `B_mate6_shallow`'s is 100000, the lowest cell and the edge it switches on --
  and **every case's union over the nine is non-zero on both sides**, 13 against
  48 for C, so no case lost its mate and only the cell holding it moved. One
  failure list and one per-case floor became three assertions, budgets unmoved:
  a line as long as the distance it claims **ends in checkmate**, at zero and
  pinned to nothing; a shorter line is S202's residue against a **per-case
  ceiling** (A 5, B 11, C 0, D 1, E 8) that
  `adocs/data/S203_case_sweep.sh --ceilings` re-derives from the recorded grids;
  and a **majority of the guarded cases, 3 of 5**, reports a mate line at all.
- **The class split is the measurement DEC-156 and DEC-161 did not have.**
  `adocs/data/S204_class_census.py` over both sides: **881 mate lines, 64 short,
  0 that run their claimed distance and fail to be checkmate**, and its counts
  reproduce `adocs/data/S204_sweep_head.txt` and
  `adocs/data/S204_sweep_killer_iter_clear.txt` in **all 90 cells**, which is
  what says the driver agrees with `adocs/data/S203_case_sweep.sh` instead of
  measuring something else. So one of the two things the old list merged is
  budget-independent over 881 lines and only the other ever needed a budget
  chosen for it.
- **Green on the tree-moving change, red on a mutant once per assertion, every
  failure observed.** S159's candidate A -- the four reds that started this --
  passes with 61 assertions, and the reconstruction was checked cell-identical
  to the recorded grid before it was trusted. No checkmate detection anywhere
  takes the majority to `0 >= 3`; DEC-122's own rejected option, extending to
  the claimed length instead of to the mate, takes the mate-reaching assertion
  red at a cell where the walk stalls, with the same budget green unmutated; the
  walk publishing nothing takes four ceilings red. **Three plausible search
  mutants stayed green and that is the guard working** -- the table refusing
  mate scores, null move reducing to depth 0 inside the mate window, quiescence
  losing checkmate detection: each moves the tree and each leaves the mates
  being found, two of them in larger numbers than HEAD. `tools/mutation_check.py`
  is S196 and does not exist yet, so the mutants were applied by hand and
  reverted; `git diff -- src/` is empty.
- **The Tier-1 check over S204's diff found three things and all three were
  real; two are fixed and one could not be.** The `--ceilings` mode dropped a
  case with no row at its own stride instead of erroring -- a golden-deriving
  script that ships a short table quietly, reachable whenever a stride moves or
  a sweep file is partial -- now fails and names the case, observed on a grid
  with `C_mate7_depth11` removed. `short_line_ceiling`'s comment cited
  `F_mate6_inherited_no_line`'s worst cell as 2000000, which is a **stride-1**
  cell for a row whose stride is 2; its own worst cell is 1200000. The ceiling
  of 2 was right either way, because the script selects the stride and only the
  prose did not. And **DEC-162's cost arithmetic was wrong**: "13.5x that" is
  not a ratio of the fixture -- 13.5 M is the sum of the nine budgets, the
  multiplier for one *search* of a case at 1 M and 135x for B at 100000. The
  union is 1863 M against 188 M, **9.9x, and the 4.3 minutes the rejection
  rests on was right**. DEC-162 is corrected in place and says so;
  `plan_done/S204_*` carries the wrong figure and is not edited, because
  `plan_done/` is history.
- **DEC-156 is amended and not upheld.** Its re-sweep prescription stands for the
  *budgets*, which nothing here moved, and no longer applies to floors because
  there are none. `adocs/plan_todo/S192_golden_hygiene.md` row 6 moved with it:
  it cited `expected_mate_lines` and turned the citation gate red the moment the
  symbol went, which is that gate working. The fixture costs **23.5 s** against
  26.3 s.
- **S159 is done, 2026-09-09, and it cost no machine time: the hypothesis was
  refuted by a census before a game was played.** S149 measured CPW's killer
  distinctness guard at -11.02 +/- 10.53 Elo and reverted it; S159's reading was
  that the guard removed an *ageing* mechanism as a side effect and that the 11
  Elo was the staleness it then preserved. Counted over **18166063 nodes** on 11
  recorded positions, on HEAD and on HEAD with the guard re-applied to an
  instrumented copy: **the stale share of distinct second-killer offers is
  1.96 % against 1.91 %, flat.** The guard did not preserve proportionally
  staler killers -- it roughly doubled how often a distinct second killer is
  offered at all, **45.4 % of nodes to 88.8 %**, and the stale count rose only
  with the offer count. On HEAD the unguarded shift already *is* the ageing
  mechanism, discarding slot 1 on **72.0 % of stores**, which left candidate A
  -- the table cleared once per iteration -- **0.89 % of nodes** to act on. It
  was built, tested, mutated and **reverted unrun**. DEC-160.
- **The `{-5, 5}` SPRT was pre-registered, priced and declined.** 10465 games
  and 4 h 36 m worst case at the measured 2277 games/h. The census is written
  into the run script's header rather than read back over the games, which is
  the point: DEC-019 is not weakened, because what was measured cheaply is the
  *size of the mechanism* and not its Elo, and a census that had come out large
  would have bought the run instead of replacing it.
- **Candidate B was neutral exactly as the step file predicted** -- node- and
  best-move-identical at depths 9 and 12 (INV-6), 121530 / 801481 / 72924 and
  636677 / 3520847 / 494098 -- and its only observable is the duplication fence
  in `tests/test_search.cpp` going red for a change that alters no game. The
  fence was not rewritten for it.
- **S149's instrumentation driver is not reproducible, and that is how it was
  found.** Its 11 positions are named in `plan_done/S149_*` and in the
  2026-08-21 audit and their FENs appear in neither, so nobody can re-derive
  66.0 % / 44.4 %. S159's set, driver, instrumentation patch and outputs are all
  under `adocs/data/S159_*`; the duplicate rates reproduce on it at 72.0 % of
  stores and 44.0 % of nodes. A census that cannot be re-run is an anecdote.
- **S190 is done, 2026-09-09: INV-2 and INV-4 are enforced by the gate.**
  `tests/test_invariants` walks every test FEN two plies deep plus the five
  positions of `test_engine`'s hash oracle at their own depths -- **2132167
  `make_move` calls** -- and after every make and unmake rebuilds the four
  accumulators with `eval_refresh`, rebuilds `squares[]` from the bitboards,
  and compares the whole `board_t` against its pre-make copy. **1.816 s +/-
  0.018 s** in Release against a 5 s budget, 12.7 s in Debug. Both gated
  builds are Release, where every `assert` in `src/` is dead, so
  `eval_accumulators_match` and `squares_match_bitboards` moved out of
  `#ifndef NDEBUG` and lost `static`; the three assert sites are untouched and
  behaviour does not change -- identical node counts and best moves against
  `dc878de` at depths 9 and 12, `bench 26851183` on both sides, so
  `No functional change`. Closes `2026-09-04_test_review-F01`.
- **The `accepts:`' "every test FEN to depth 3" was unsatisfiable** -- 53975914
  makes, about 34 s with the compares, against "Release wall time under 5 s" --
  and the owner re-decided it on 2026-09-09 to depth 2 over the corpus plus the
  five at their own depths, section 10 question 1's own proposal. Question 2's
  `memcmp` is in; question 3's environment override is left to S197 behind the
  single `CORPUS_DEPTH` constant.
- **Six mutants, all killed, each naming the field it broke**, and two of the
  guide's predictions came out wrong. The `squares[]` mutant did **not** redden
  perft -- `test_movegen` *"shallow perft matches every column"* stayed green,
  because `squares[]` is not what the generator reads -- and the `phase` mutant
  reddened three other binaries rather than none. **No mutant is caught by the
  new test alone**: `test_engine`'s existing `memcmp` compares the whole
  `board_t` after every unmake, and that catches all six -- which is the half
  F01 already had. What this step adds is the *after make* half, and five of
  the six mutants fail there, at `test_invariants.cpp:123`. **The phase mutant
  is the one that does not**, failing instead at the after-unmake line 131, and
  the reason is worth keeping: it drops the `phase` decrement from
  `eval_remove_piece`, the first drift it reaches is a promotion, and the piece
  `make_move` removes there is a **pawn**, whose `phase_value` is zero. Nothing
  moves until `unmake_move` removes the queen. So the mutant set as it stands
  does not exhibit a drift that only the after-make half can see; S196 inherits
  that as a gap to close, not as a proven equivalence.
- **The Debug self-play line is traced to observed output, and the obvious form
  of it lies.** `-log file=...` defaults to WARN and does not capture engine
  stderr, which is where an `assert` writes: a Debug binary carrying a planted
  accumulator mutant aborted in every game and `grep -c Assertion` read **0** on
  that log, against **2** on the same run at `level=trace engine=true`.
  `fastchess.sh` uses the default form. `DEV_MANUAL.md` now carries the line
  with the level and the reason; the clean run was 8 games in 19 s, 0
  `Assertion`, 0 `disconnect`, 0 crashes.
- **The section 7 timing formality resolved nothing and is recorded as
  resolving nothing.** `hyperfine -w 2 -r 10` over `search_bench ... 12` read
  the working tree 1.11x faster than the reference and 1.08x slower with the
  order swapped, while an **A/A of one binary against itself read 1.07x**. The
  probe's noise floor at this shape is 7 to 11 % on a 0.8 s run dominated by
  process startup; both A/B readings sit inside it and no difference is
  claimed. INV-6's identical node counts are the discharge.
- **Census floors are goldens and were re-derived, not read back** (DEC-142):
  `adocs/data/S190_walk_census.py` counts the same tree with python-chess and
  agreed with the engine's walk exactly -- 2132167 makes, 15023 castlings, 181
  en passants, 221928 promotions, 206212 capture promotions -- so the asserted
  floors are half of each: 1000000 / 7000 / 90 / 100000 / 100000.
- **S187 is done, 2026-09-09: every citation from a pending step file into
  code names a symbol, and the checker refuses a line number.** **572
  converted over 27 files**; `tools/plan_prose_check.py --citations` reads
  **0 flagged over 65 files**, from a census of **558 code citations, 14
  document, 0 bare and 52 DRIFT** banked at `2b198f6` in
  `adocs/data/S187_citations_before.txt` before the first conversion. The
  mapping is `adocs/data/S187_symbols.tsv` and the generator
  `adocs/data/S187_symbolise.py` -- S144's and S169's shape, because the
  evidence for a conversion is the mapping and never a green run (DEC-119).
  Methods: 393 WALK, 47 AUTO, 50 TITLE, 37 CHOSEN by hand, 20 HAND read and
  rewritten, 15 SPAN over a pair with the path repeated per DEC-120, 10 PHRASE
  at file scope. Closes
  `2026-09-04_plan_review-F07`.
- **Six citations were wrong at the moment they were written, and that is the
  class the old checker could not see** -- its baseline was the commit that
  wrote them. `src/chesso.cpp` line 674 was cited by **S099, S132 and S159**
  for where the per-`go` `search_state_t` is built, and at all three baselines
  it had fallen into the book-move helper; S132's node counter named the
  transposition table declared under it and its per-iteration reset the same
  helper; S115's two citations for the test-only aspiration counter named a
  thread block and a typedef; S024's countermove write named an `unmake_move`
  line. Each is in the generator's `HAND_CHOICES` with the sentence that
  decided it. The four F07 cases come out right: S109's `is_check_move` and
  its reduction guard are both `src/search.cpp` `negamax`, S055's taper
  divisions split between `evaluate_cheap` and
  `evaluate_mobility_and_king_safety`, S024's countermove read is
  `src/evaluation.cpp` `score_move`, S119's is `rating.sh` `hash_mb`.
- **`--citations` is in the fast suite now, DEC-159**, as
  `test_plan_citation_freshness`: **0.45 s** over the 66 pending files, median
  of five, against **6.3 s** under the retired DRIFT class, which ran
  `git show` once per baseline-and-path pair. The S141 reason for keeping it
  out -- "any source commit shifts lines under fifty step files at once" -- no
  longer reaches it, because nothing it reads is a line. What turns it red is
  a renamed or deleted symbol with a pending step still citing it, and that
  commit is exactly when a manual check is not run. `tests/CMakeLists.txt`,
  `DEV_MANUAL.md` and `adocs/plan.md`'s writing rule all say so.
  `tests/test_plan_citations.py` is the separate planted-case gate on the
  checker itself, 18 cases over temporary files, so a recogniser that stopped
  firing cannot hide behind a green real set.
- **Three things the next writer should know.** `BOUNDS`, `ANCHOR` and `DRIFT`
  are retired; `LINE`, `MISSING` and `BARE` replace them. **`LINE` is found by
  a scan over the file's text, not through the path token**, because a path
  preceded by a slash is invisible to the token -- S020 and S117 each write
  `path:line/path:line`, and four citations were hiding there. And the
  conversion costs prose: a step that cited eight distinct lines inside
  `negamax` now cites `negamax` eight times, so **fifty paragraphs were
  rewritten** to name the enclosing function once and the sites by what they
  do. Write it that way from the start; the checker fires only on a backticked
  identifier or a quoted phrase directly after the path, so backtick the
  symbol.
- **S148 is done, 2026-09-09: the reverse futility ceiling stays at 15, and
  the deep mates it loses are its measured price.** `{-5, 0}` nElo at 8+0.08,
  **H0 accepted at LLR -2.95** -- `Elo -5.66 +/- 4.33, nElo -7.31 +/- 5.60`
  over **14808 games in 6 h 19 m 35 s, 0 time forfeits**, LOS 0.52 %,
  `adocs/data/S148_sprt.log`. The candidate was one integer, `RfpMaxDepth`
  15 -> 4, and it is reverted. The reading was written into
  `adocs/data/S148_sprt.sh` before the first game and applied unchanged: a
  ceiling low enough to find the deep mates costs more than five nElo of
  ordinary play, so the mates the engine does not find -- **1 of 16 at four,
  0 of 16 at five** -- are what the pruning costs. DEC-158, and the five
  places 15 is written now carry the price beside it: `specs.md`, both
  `MANUAL.md` sites, `DEV_MANUAL.md`'s mate table (re-taken over 82 rows, it
  still read 48 and S154's counts), `src/search_params.hpp`'s comment and the
  mate suite's own. **A verdict of "keep the incumbent" is a result and the
  step completes on it**, which is the S005 / S006 / S015 precedent read in
  the other direction.
- **Neither deep class was promoted to an asserted floor, and the reason is
  not that they are zero.** At the shipping value mate in four is **1 of 16**,
  which satisfies the accepts' "non-zero" literally -- but a floor of 1 has no
  margin between its ends, which is what DEC-116 rejected for the mate in
  three, and the owner decided on 2026-09-08 that a class at exactly 1 stays
  recorded. Both stay in the `MESSAGE`; the comment above
  `MATE_IN_THREE_FLOOR` now says which, why, and what the refused ceiling
  would have made them.
- **Two facts from the run are worth more than the verdict.** The
  `Incomplete mating PV` class was read from both sides -- **7 lines from the
  candidate against 13 from the reference** -- so the class is not this
  ceiling's and the lower ceiling produced *fewer*; **S202 inherits that**.
  And `test_mate_carry` went red on the candidate and was established as
  budget calibration **before** 18 hours of machine were committed:
  `adocs/data/S203_case_sweep.sh` shows the short lines appearing and
  vanishing with the node budget in both directions -- A short 2 at 1000000
  and 0 at 1200000, 1500000, 2000000, 3000000 and 4000000 -- which is
  DEC-122's eviction class and S203's own knife-edge warning, not a defect
  this change introduced.
- **One thing S187 should know before it starts.** The `src/search_params.hpp`
  comment grew by eight lines here, which shifted every line-number citation
  below it and took `--citations` from 52 flags to 59. The seven are restored
  mechanically -- S098:120, S109:193, S114:87 and :117, S120:89 and :192,
  S132:87, each **+8** -- so the check reads **52 over 66 files** again, the
  same 52 the parent read over 67. It is red on both sides and has been since
  before this step; that is F07 and S187 owns it. One of the seven is a worked
  example of why the unit is wrong: **S109:193 names `LMR_DIVISOR` and points
  at the reverse futility comment**, and the checker cannot see that, because
  all it verifies is that the cited line still holds the text it held.
- **The sweep is the reusable half.** The finer grid was measured before the
  run and the challenger decided by the rule the file pre-registered: `adocs/data/S148_rfp_ceiling_sweep.py --mined` at `192a5a3`,
  every ceiling from 0 to 15 over the 82 constructed rows and the 318 mined
  ones, `RfpMinPly` held at 3 and read from the binary. **C1 = 4**, because the
  mate in five class is the binding one and it is a cliff -- 11 / 9 / 6 / 4 / 1
  of 16 at ceilings 0 to 4 and 0 from 5 up. The whole constructed set moves
  **39 to 52 of 82** between the shipping 15 and 4, the mined set 145 to 159 of
  318, and `short` and `sign` are 0 at all sixteen settings. Two readings the
  coarse grids could not give: the **plateau starts at 10**, not at 15, and the
  mate in four class is 1 of 16 at the shipping value rather than S145's
  "0 of 8", the set having grown.
- **The owner answered all five deferred questions on 2026-09-08, every one as
  the file recommended**, and the answers are in the step file: `{-5, 0}`
  `--nonreg`; **C1 alone**, no C2 = 6 fallback; **keep 15** on a null or a
  rejection; the re-run lands in `S148_rfp_ceiling_sweep.log` because S145's is
  evidence; the excluded dynamic-cutoff sentence reworded off Stockfish's two
  constants (DEC-134). The fifth binds: a class whose count is exactly 1 stays
  in the `MESSAGE`, and at C1 the mate in five count **is** 1, so only the mate
  in four class is promoted when the value ships.
- **The candidate was not behaviour-neutral, which is why INV-6 could not
  discharge it and the games had to.**
  `search_bench.py` depth 9 **124511 / 805638 / 111393** against HEAD's
  121530 / 801481 / 72924, depth 12 **716171 / 3707680 / 560079** against
  636677 / 3520847 / 494098, `bench` **28339749** against 26851183, best moves
  `c3d5` / `e2a6` / `d7c8q` unchanged throughout. Debug self-play **0
  Assertion over 8 games**, 0 disconnect, crash or illegal. Gate green at HEAD
  in both builds, 28/28, before the integer moved.
- **S184 is done, 2026-09-08: the pending documents state the engine as it
  ships, and the parameter checker now reads them.** Every F05 value restored
  from `src/search_params.hpp` at HEAD -- S115's aspiration sweep designed on
  2 / 21 / 437 with the "keep 5" instruction gone, S082's quiescence cap on 19,
  the lazy clamp on 184 in S120, S122 and `plan.md`, S118's cost paragraph
  dated to before S104 with `specs.md`'s 53.90 ns quoted beside it. S042's
  `touches:` names all four sites that build the en-passant key and its body
  states the one rule applied at each. Five F09 sentences and two F10 headers.
  Documents only: `git diff --stat -- src/` empty, 28/28 in both builds.
  Closes `2026-09-04_plan_review-F05`, `-F08`, `-F09`, `-F10`.
- **Three things from S184 are worth knowing before the next step.**
  `tools/plan_prose_check.py --params` now reads the **68 pending step files**
  as well as the four documents, and matches the **C++ symbol** -- with both
  backticks mandatory, because optional they make S114's formula line read as a
  claim about its divisor. So a pending file stating a stale parameter is a red
  fast suite from now on, and the cost is 1.27 s a run against 0.37 s.
  **S171's three F09/F10 items are void, not deferred**: that step completed
  into `plan_done/` on 2026-09-07/08 and `plan_done/` is never edited -- its
  `done:` stamp sits at line 551 rather than in the header, which no longer
  matters. And **an absent `author:` is the corpus convention** for an
  unstarted step, not a defect: 47 of 48 `plan_todo/` headers omit it, which
  is why S024 got one and then had it reverted. The header audit that
  established this found one fresh instance of F10's real class, S202's missing
  `done:`, now added.
- **S180 is done, 2026-09-08: no pending step tells an implementer to start a
  sweep from another engine's number.** Every seed in the `### 4. Constants and
  seeds` section of S095, S097, S098, S109, S113, S114 and S132 is now one of
  three declared forms -- **(a)** a literature value with its URL, **(b)** a
  derivation over chesso's own data or scale written as the procedure its
  owning step runs, **(c)** a declared range's midpoint or off value stated as
  such. F01's ten rows walked plus about a dozen the inventory found.
  Documents only: no `src/`, nothing measured, `GATE-DONE 26851183` unchanged
  from the parent, fast suite 28/28 in both builds. Closes
  `2026-09-04_plan_review-F01`.
- **Three replacements are worth knowing before their steps start.**
  `PROBCUT_MARGIN` is no longer four engines' margins averaged: it is Buro's
  own regression over chesso's positions at the paper's `t` = 1.0, and the
  procedure with its exclusions is written into S113 for S113 to run. **The
  futility margins were wrong in a way nobody had noticed**: the 2026-08-19
  pass read the wiki's "minor" and "rook" as 300 and 500 -- `see_value`'s
  scale -- for a margin compared against `evaluate()`, so `FUT_BASE` /
  `FUT_SLOPE` become **147 / 170** from `piece_value`, not 100 / 200. Every
  rewritten section now carries a units paragraph naming which of the two
  pawns, 94 or 100, its numbers are in. And the clause seeding an LMR
  re-sweep from three engines' curve coefficients republished on the wiki is
  **deleted** -- DEC-105's own PeSTO case -- leaving S085's own 52 / 182.
- **DEC-157 answers the step's three deferred questions, and one of them is a
  standing rule.** `NULL_MOVE_EVAL_CAP` seeds from the range midpoint 8 with
  the measured bound (P4) named beside it, because a step file may state an
  alternative but S180 measures nothing; the stamp lists F01's rows first and
  the inventory's second; and the two stale shipping values sitting inside the
  rewritten sections -- S114's `NULL_MOVE_BASE` "2 ships today" against the 3
  S085 shipped, S132's `TM_NODE_MIN_DEPTH` 5 against the compiled 2 -- were
  fixed here rather than handed to S184, whose file now says so.
- **S203 is done, 2026-09-08: the Zobrist keys are the project generator's, and
  the mating-PV fixture was rebuilt by a rule rather than by a match.**
  `init_zobrist` draws its 851 keys from `project_random_next` seeded with
  `CHESSO_PROJECT_SEED`; `magic_gen zobrist --seed 20260904` reports **`matches
  init_zobrist: yes`** where it reported `no`, quality clean (0 zero keys,
  851/851 distinct, 0 pair XORs equal to a key, 361675/361675 distinct pair
  XORs, minimum Hamming 14). The fast suite asserts all of it. **New baselines:**
  depth 9 **121530 / 801481 / 72924**, depth 12 **636677 / 3520847 / 494098**,
  `bench` **26851183**, **best moves unchanged** -- `specs.md` under INV-6 and
  the handover check below carry them. Perft verified, `test_perft` green, Debug
  self-play 8 games 0 assertions. Closes `2026-09-04_test_review-F08`.
- **The mining run produced nothing, and that is the finding, DEC-156.**
  `ROUNDS=1500 ./fastchess.sh`, 3000 games at 8+0.08, **1 h 18 m 10 s, 0 time
  forfeits** -- and **3** `Incomplete mating PV` warnings over 2 roots, **all
  from the old-keys reference, none from the candidate**. That is not evidence
  the redraw fixed the class: 3 lines landing all on one side has probability
  0.125 under an equal-rate null. What it says is that the class has become far
  rarer than when the set was built -- S147 read 10 lines over 4 games in 3000 --
  so a mining campaign for six fresh cases was not going to converge. The run's
  Elo was not read and the banner says why: fixed rounds, no bounds, not a
  verdict.
- **All six cases came back by re-sweeping their node budget, no match needed.**
  One rule, stated before it was applied and applied to every row: the cheapest
  budget at which the case reports at least its floor of mate lines with all of
  them complete. A `300000 -> 1000000`, C `1000000 -> 1500000`,
  D `1000000 -> 4000000`; B, E, F untouched. Floors re-derived, A 5 -> 6 and
  D 1 -> 3, **no live floor lowered**; F's stale 6 against 4 reported corrected
  to 2. `adocs/data/S203_case_sweep.sh` is the script, which is what DEC-142
  wants beside a golden. **The grid is a knife edge and both files now say so:**
  C reports 13 mate lines at 1500000 nodes and **0 at both 1000000 and
  2000000**, so re-run the sweep after anything that moves the tree, not only
  after a key change.
- **Two reproductions changed hands.** `D_mate_minus6_depth10` between 1200000
  and 3000000 nodes publishes `mate -6` at ply 35 depth 11 with a 10-of-12-ply
  PV, at a depth that also publishes a complete 12/12 -- DEC-122's class, and
  **S202** now carries it as a seconds-long reproduction where its only other
  route was a 3000-game match. In the same move, **S202's existing F
  reproduction is retired**: it printed 6 mate lines and 3 short for two months
  and prints 4 and 0 under the redrawn keys. The old numbers are kept beside the
  new ones, because they are quoted elsewhere. The fast suite costs about 8 s
  more, D's new budget being the price.
- **S179 is done, 2026-09-08: the 128 magic numbers are this project's own
  output and share 0 values with the tutorial set.** `tools/magic_gen` is the
  command and **20260904** the seed -- DEC-132's date, the owner's choice.
  It finds all 128 in **1.56 s** over 9270054 candidates, and the coincidence
  check reads **0 of 128** against the arrays it replaced -- an independent
  regex over both headers, 0 shared values and 0 at the same index -- then
  **128 of 128** from the tool rebuilt against the new one, which is the proof
  that what is committed is what the seed reproduces, not an argument that it
  is. Node-identical by construction and proved rather than asserted:
  `bench_movegen` verifies its perft counts before printing, `test_perft`
  green, `search_bench.py` gives 121512 / 800769 / 62907 and
  639228 / 3430710 / 367858 with `c3d5` / `e2a6` / `d7c8q`, and `gate.sh`
  **GATE-DONE 24880255 (no functional change)**. Interleaved `hyperfine`
  **1.01x** (5.267 +/- 0.016 s against 5.217 +/- 0.075 s), Debug self-play
  **0 assertions**. `magic_is_collision_free` lives in the engine so the
  predicate the generator accepts a candidate with is the one the suite runs
  over all 128, with a zero magic as the precondition proving it can fail.
- **The step was split and the reason is worth more than the split, DEC-154.**
  The Zobrist half was written, measured and then withdrawn: it turned
  `test_mate_carry` red, and not by introducing a defect. `adocs/data/S170_cases.tsv`
  is six eviction reproductions **mined from S147's 3000-game run**, and which
  entries evict which is decided by the keys -- so over four arbitrary seeds
  (20260904, 12345, 999983, 777777777) **every one** left 3, 3, 4 and 3 of the
  six cases reporting no mate at all. There is no seed to pick, and picking one
  would be fitting the seed to the test. Under 20260904 one case also published
  `mate 9` on a 5-ply PV, which is the class DEC-122 leaves short and visible
  and **S202** owns -- not a wrong score, with perft, `test_perft`, the
  hash-versus-recompute test and Debug self-play all green and the 851 keys
  distinct with no three- or four-subset XORing to zero. The measured figures
  are carried into **S203**'s file rather than lost: depth 9
  121530 / 801481 / 72924, depth 12 636677 / 3520847 / 494098, `bench`
  26851183, **best moves unchanged**. `2026-09-04_test_review-F08` goes to S203
  with the half that closes it. The fixture's key-set dependency is now named
  at both its sites, which is DEC-142's rule reaching a fixture that is not a
  number.
- **S198 is done, 2026-09-08: the harness seeds its openings, records nodes and
  clocks, and the workstation is calibrated.** `fastchess.sh` derives `-srand`
  from the run stamp, prints it in the banner and carries it in each PGN's
  `[Event]` header -- fastchess records it on no stream, in no log and in no
  header of its own -- with `SRAND=<n>` to replay a sequence and `ROUNDS=<n>`
  for the fixed-rounds mode nothing had, which drops `-sprt` and prints
  `bounds none -- fixed <n> rounds ... NOT a verdict`. `-pgnout` gains
  `nodes=true timeleft=true`. Three test cases, red first with 5 assertions
  against HEAD's script; eleven properties now.
- **DEC-143's calibration is taken and it stands.** 1000 games at `bbbf8f6`
  between **byte-identical binaries** (sha256 `b047f22d...`), 26 m 21 s,
  **0 time forfeits on either side**, pair score variance **0.2430 +/- 0.0154**
  against S105's after-run 0.2395 +/- 0.0152: **`z = +0.16`, inside the
  pre-registered band**, ratio 1.014, nothing re-run. **2277 games an hour**,
  and the difference from S105's 2322 is game length rather than machine speed
  -- seconds a ply 0.1831 → 0.1791 while plies a game went 98.0 → 102.1. One
  nElo is **0.698 logistic Elo** here, read off the run's own pair. The run is
  also the mode's own argument: with the truth zero by construction it printed
  `Elo: 7.99 +/- 15.03, LOS 85.16 %`, which is why a fixed-rounds run is never
  quoted as a verdict. Records `adocs/data/S198_*`; DEC-153 has the owner's
  three answers. **The next verdict may be taken.**
- **S189 is done, 2026-09-08: the engine has a node signature and a script that
  checks it.** `bench` searches eight fixed positions at `BENCH_DEPTH` 14 and
  prints **`24880255 nodes <nps> nps`** -- identical across three fresh
  processes and across the stdin and argv forms -- and `tools/gate.sh` runs the
  TESTS chain in both builds and then refuses a message whose `Bench:` line is
  not the binary's number. **DEC-140 binds from this commit on**: every commit
  touching `src/` carries `Bench: <n>` or `No functional change`. INV-6 stops
  being a procedure a person performs by eye; it is identical at depths 9 and
  12 and the interleaved timing is 1.00x, so nothing in the search moved.
  DEC-152 records the owner's four answers -- `KILLER_POS` stays in the set
  though it is illegal by piece count, depth 14 by the step's own rule, the
  OpenBench argv form ships now, and `gate.sh` gets a test.
- **The red-then-green could not be taken with a search mutant, and that is
  worth more than the demonstration was.** Three node-moving changes were tried
  -- `M01_nmp_in_check` from the test review's own mutant list, the two killer
  ordering ranks swapped, `MVV_KNIGHT` 300 → 310 -- and the fast suite refused
  all three before the signature was ever compared. The common cause is
  `test_mate_carry`: its anti-vacuity precondition, `mate_lines >=
  expected_mate_lines(name)`, is a search-tree-sensitive golden with no script
  that re-derives it, so any change that reorders the tree trips it. That is
  the test doing its job and it is also exactly the DEC-142 class. **Recorded
  for S192**, which owns naming and scripting the goldens. The demonstration
  used `BENCH_DEPTH` 14 → 13 instead, the case `MANUAL.md` names as a
  deliberate signature change: `GATE-FAILED: message says Bench: 24880255, the
  binary benches 13064004`, then `GATE-DONE 13064004` after the amend.
- **The S189 fast check found a second instance of the first bug, fixed at
  `HEAD`.** The `No functional change` parent walk had no `|| true` either, so
  an ancestry carrying no `Bench:` line anywhere -- a history from before
  DEC-140, which is precisely what `--build-parent` exists for -- died at
  `GATE-FAILED: exited 1` one line above the message telling the reader to use
  it. The one path that most needed to say what to do next was the one that
  lost it. `tests/test_gate_script.sh` gains case 10, observed red on the
  unfixed script and green after; cases 5 and 6 could never have caught it
  because their sandbox has carried a `Bench:` line since its root commit.
- **One bug found and fixed on the way, by the gate's own test.** Case 8 of
  `tests/test_gate_script.sh` -- a binary printing no signature line -- got the
  trap's generic `GATE-FAILED: exited 1` instead of the specific message
  written for it: `grep` exits 1 on no match, `set -o pipefail` fails the
  pipeline, and errexit killed the script one line above its own check.
  `bench_of` is now called `|| true` at both sites.
- **What it cost the suite.** 52 s over 27 tests → **61 s over 28**, all of it
  `test_uci_surface` going 0.03 s → 6.8 s because its new case searches the
  bench set twice at the shipping depth. **In `Debug` that one case is 157 s**,
  measured, which S197's Debug gate inherits. Kept at the shipping depth on
  purpose (DEC-152).
- **Carried forward, cheap to know:** `git worktree` does not bring the
  submodules, so a worktree cannot build the test targets without them.
  `fastchess.sh` and `gate.sh --build-parent` never meet this because both
  build only the `chesso` target. It cost one gate run to find.
- **S171 is done, 2026-09-07/08: the census ran on this machine and its
  residual is measured rather than closed (DEC-150).** The run DEC-128
  postponed here -- `REF=457e355 ./fastchess.sh --fast`, 3000 games at 8+0.08,
  **1 h 17 m 49 s, 0 forfeits**, `Elo +3.24 +/- 8.91` between INV-6 identical
  builds, out `/tmp/chesso_sprt_fast_20260907_224259`. The count it exists for:
  candidate **8** `Incomplete mating PV` lines from **1** search,
  `ref-457e355` **0** from **0**. The `accepts` said 0, and 0 turned out to be
  unreachable inside the step's own `excludes`, so the owner amended it to the
  measured pair.
- **The residual is neither a regression nor a wrong score, and no walk can
  close it.** `stockfish` gives `#+6` for the position at depth 20 and 30, so
  the `mate 6` is right; it is read off the table at depth 3 on 1224 nodes, too
  shallow to build the 11 plies it owes, and the line that iteration built
  continues in the table into a chain proving `mate 8`, so both distance-keyed
  lookups in `complete_mate_pv()` refuse and DEC-122 leaves the line short and
  visible. The same three lines reproduce byte for byte on `457e355`; the
  8-against-0 split is which side met the position. Building the missing line
  means searching, which that path may not do, and every other route is named
  in S171's `excludes`. **S202** owns the class and may alter play under its own
  SPRT. The standing figure in `adocs/specs.md`, `DEV_MANUAL.md` instrument 3
  and now `MANUAL.md` is **8 from 1 search in 3000 games** on this machine, with
  the MacBook's 5 kept attributed to the MacBook.
- **One hypothesis was implemented and reverted on the way, and it is worth the
  line.** That the walk stalled on a position whose single legal move carried an
  upper bound, which `certified_mate_move()` declines by construction. It was
  read off a walk of the match's line over a *differently warmed* table, the
  guard case stayed red under it, and it was backed out: `src/search.cpp` is
  untouched by this step and stands at `136b03f`. A dump of the table taken at a
  different moment than the failure is not evidence about the failure.
- **The instruments were probing a table no game produces, DEC-151.**
  `adocs/data/S170_replay.py` and `tools/mate_trace.cpp` both replayed **every**
  ply through one process; a game gives one engine only the positions it moves
  from. The census case was read back at stride 1 three times and came up clean
  each time, and reproduced at stride 2 on the first attempt -- at
  `nodes 1000000` from ply 56 or 64, *every* mate line the stride-2 replay sees
  is short. Both tools take a stride now, `adocs/data/S170_cases.tsv` has a
  `stride` column, and the guard test refuses a `start`/`stride` pair that steps
  over the last ply. Rows A to E stay stride 1 and still report 0 short lines.
  The class hidden behind stride 1 is unmeasured and S202 inherits it.
- **The POWER rule's literal test does not survive a wireless mouse.**
  `/sys/class/power_supply/` on this machine holds exactly one entry,
  `hidpp_battery_0`, type `Battery` -- a Logitech peripheral. S171's guide said
  "no `Battery` line" satisfies the rule; on a mains-only desktop that reads as
  a refusal. Taken as satisfied and recorded in the stamp. Whoever writes the
  next power check should test for a supply whose `scope` is `System`, not for
  the absence of the word.

- **Back on the workstation, 2026-09-07, and its gate was red on arrival --
  both halves fixed or decided, nothing in the engine changed.** No code moved
  between the MacBook and here; a machine changed and two compiler-and-toolchain
  differences came with it, which is what S167 was in the other direction.
  **(1)** `tools/mate_trace.cpp`'s usage comment wrapped with a trailing
  backslash, which g++ 13.3 -- the reference compiler here, DEC-049 -- calls
  `-Wcomment` and `-Werror` makes fatal; clang does not warn, so it survived
  S171 (`136b03f`) and stopped `build` and `build-tune` at the same object.
  Fixed at `d54f690`, comment only. **(2)** clang-format 23 (DEC-110's pin) is
  not installable from what this machine carries -- the apt line is
  `llvm-toolchain-noble-22` -- so the script resolved Ubuntu's 18.1.3, refused
  it and exited 1, and `test_clang_format_script` failed six assertions in both
  builds: the failure DEC-110's Consequences predicted for this machine, word
  for word. **The owner decided the override, DEC-146**: the pin stays 23,
  this machine exports `CLANG_FORMAT_MAJOR=22`, and `.moltke.local.md` carries
  it. The two majors format this tree identically -- measured, the check prints
  nothing and exits 0. **After both: fast suite 27/27 in both builds, gate
  exit 0.** Also re-checked on arrival: INV-6 reproduces to the node
  (121512 / 800769 / 62907 at depth 9, 639228 / 3430710 / 367858 at depth 12,
  `c3d5` / `e2a6` / `d7c8q`), `src/openings.bin` still digests to
  `77f47f1b...db06b58`, the Open list's 74 entries match `plan_todo/`'s 74
  files id for id with none duplicated and none also in `plan_done/` (72 after
  the day's three completions, re-checked), and
  `plan_prose_check.py --touches` flags 0. **Before the first timed run the
  machine still needs**: the `performance` governor (it boots `powersave`),
  `kernel.perf_event_paranoid=1` for samply, and an idle desktop -- gthumb and
  firefox were between them holding about two cores while this was written.
  **Two facts the workstation settles**: `fastchess` here is
  `alpha 1.8.1 20260720-daa3ea2`, the version S105, S087 and S145 measured
  with, which answers S198's open question about it; and `.ref-builds/` holds
  21 August worktrees at 2.2 GB on a root filesystem at 95 %, prunable with
  `git worktree remove` at the cost of one rebuild if a ref is re-used.
- Last done: **S189, 2026-09-08 -- the `bench` node signature and
  `tools/gate.sh`.** `24880255` at `BENCH_DEPTH` 14 over eight fixed positions;
  the gate runs the TESTS chain in both builds and then checks the binary
  against the commit message. DEC-140 binds from its completing commit on and
  DEC-152 records the four owner answers. Before it: **S200, 2026-09-07 --
  `movetext_to_san()` reads the whitespace
  form of a move number indication, and a cut-short message quotes the token as
  the PGN wrote it.** PGN 8.2.2.1 lets white space sit between the digits and
  the dots, so `1 . e4`, `1 .e4` and `1. ... e5` hand the splitter a token of
  dots with no digits in front of it; S178's block required digits, so the dots
  reached `algebraic_to_move()` whole and the build was refused at ply 0.
  Dropping that one clause -- `dot > 0` -- reads all three forms. The message
  half is the reader's problem and not the engine's: `cannot parse 'Qxf7'`
  matches every line of a PGN that plays the move somewhere, where `2.Qxf7`
  matches the one line that holds it, so `movetext_to_san()` now returns
  `san_token_t {move, as_written}` and `build()` reads the second field for both
  `report_cut_short()` messages. **Red first at `ffe207f`**: two new fixture
  properties gave 13 `FAIL:` lines -- `cannot parse '.' at ply 0`, `'.e4' at ply
  0`, `'...' at ply 1`, and `'Qxf7' at ply 2` failing both the as-written
  assertion and its bare-form negation. Green after. **Numbered 12 and 13, not
  the 9 and 10 the step file named**: S173 landed between the writing and the
  doing and took 9, 10 and 11. **The remainder is still not re-classified**, as
  the step's traps require: `.2` cuts short as `.2` and `1.2` as `1.2`, while a
  bare `2` is still dropped -- silently dropping broken input is the fault class
  S174 closed. **python-chess agrees**: `adocs/data/S175_book_conformance.py`
  reads all three new fixtures natively at `missing 0 extra 0
  weight_mismatch 0`. **Nothing in the engine moved**: no `src/` file in the
  diff, so no `Bench:` line is owed (DEC-140) and `tools/gate.sh` does not exist
  yet; INV-6 reproduces to the node (121512 / 800769 / 62907 and
  639228 / 3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`), and `src/openings.bin`
  rebuilt from `books/8moves_v3.pgn` is byte-identical at `77f47f1b...db06b58`
  -- 34700 games, 0 cut short, 172232 entries. Gate 27/27 in both builds, format
  clean under `CLANG_FORMAT_MAJOR=22`. **Documents**: `DEV_MANUAL.md`'s
  move-number paragraph says the whitespace form is read and what happens to the
  remainder, its cut-short sentence says the token is quoted as written, and
  `adocs/specs.md`'s book clause is widened from S178's wording -- the
  coordinator's own call, this step held the machine; `MANUAL.md` checked, no
  UCI surface touched. **`1 .e4` builds correctly only because S201 fixed the
  parser first**; the other import-format leniencies S178 listed are untouched.
- Previously: **S173, 2026-09-07 -- `make_book build` replaces a book
  atomically.** The bytes go to `<out>.tmp` beside the destination and are
  renamed over `--out` only after the write, `fsync` and `close` all succeed;
  every failure the process lives through unlinks the temporary and exits 1
  with `book not written` and the `errno` string. Before, `std::ofstream
  output(options.out, ...)` truncated the previous book the instant it opened,
  so a write that then failed destroyed it -- and because any prefix of a
  sorted 16-byte-entry book is a valid book, what was left passed `dump` and
  the engine's loader alike. **Two reds on this machine, both new here.** The
  fixture's three new properties (9, 10, 11) gave two `FAIL:` lines --
  destination 0 bytes, refusal message absent -- and by hand `ulimit -f 0`
  exited **153**, SIGXFSZ killing the tool before any guard could run. The
  partial-write form, which S146 had only on macOS, reproduces here too:
  pre-change under `ulimit -f 200` the shipped PGN left **204800 bytes** at
  `--out` and `dump` read it (`heaviest 1: 0844931a6ef4b9a0 g1f3 weight
  2527`). **Green after**: fixture `ok`; by hand exit 1, `wrote 204800 of
  2755712 bytes to '<out>.tmp': File too large -- book not written`, the
  destination still `previous book` at 13 bytes, no temporary. So `SIGXFSZ` is
  now ignored in `main` -- the limit arrives as `EFBIG` from `write` instead of
  as a kill -- and `tools/make_book.cpp` carries the first POSIX headers in the
  tree (`<fcntl.h>`, `<unistd.h>`: nothing standard gives `fsync`).
  **Success path unchanged, measured**: the shipped book rebuilt through the
  new write path is 2755712 bytes, `cmp`-identical to `src/openings.bin` at
  `77f47f1b...db06b58`, 1.15 s. **No `src/` file in the diff**, so no `Bench:`
  line is owed and INV-6 is not owed -- `make_book` is its own executable.
  Gate 27/27 in both builds, format clean under `CLANG_FORMAT_MAJOR=22`,
  `plan_prose_check.py` exit 0 with 0 touches flagged. **The owner chose the
  fixed temporary name over `mkstemp`, DEC-149**: `<out>.tmp`, so at most one
  is ever left behind -- by a kill the process cannot survive -- and the next
  build to the same `--out` truncates it, which lets the `accepts` hold as
  written; the cost is that two concurrent builds to one destination would
  clobber each other, which nothing in the tree does. That choice also answers
  the step's file-mode question by itself: `O_WRONLY|O_CREAT|O_TRUNC` at 0666
  is exactly what the replaced `std::ofstream` asked for, and `rename(2)`
  carries the mode onto the destination. `DEV_MANUAL.md`'s "`loadable` does not
  mean complete" paragraph now states the guarantee and carries the
  `ulimit -f` reproduction as the root-free form of S146's ram disk;
  `MANUAL.md` and `adocs/specs.md` checked, neither moves.
- Before that: **S178, 2026-09-07 -- `movetext_to_san()` reads a move number
  indication glued to the move it introduces.** `1.e4`, `2...Nc6`, `4.O-O`: PGN
  import format, 8.2.2.1. Before, the whole token reached `algebraic_to_move()`,
  got 0 from S174's fail-closed parser, and the build was refused at ply 0 --
  honest and useless for the half of the world's PGN files written that way.
  The splitter now drops a bare number first, then, for digits followed by at
  least one dot, either drops the indication alone or erases it and keeps the
  move. **Red first at `3200a1a`**: two new properties in
  `tests/test_make_book_tools.sh` gave 8 `FAIL:` lines, all on `cannot parse
  '1.e4' at ply 0`; the export-form control was green from the start, so the
  expected entry counts came from its own output. Green after.
  **python-chess agrees**: `adocs/data/S175_book_conformance.py` re-derives both
  glued fixtures at `missing 0 extra 0 weight_mismatch 0`. **Nothing in the
  engine moved**: no `src/` file in the diff, so no `Bench:` line is owed
  (DEC-140), INV-6 reproduces to the node (121512 / 800769 / 62907 and
  639228 / 3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`), and `src/openings.bin`
  rebuilt through the fixed tool is byte-identical at `77f47f1b...db06b58`
  because `books/8moves_v3.pgn` has no glued token. Gate 27/27 in both builds,
  format clean under `CLANG_FORMAT_MAJOR=22`. `DEV_MANUAL.md` and
  `adocs/specs.md` say so; `MANUAL.md` checked, no UCI surface touched.
  **Two owner questions are open and neither is a bug**: whether `1 . e4`
  (whitespace between the digits and the dots, also legal by 8.2.2.1) is worth
  the same few lines or its own step -- today the leading `.` reaches the parser
  and cuts the game short by name -- and whether the cut-short message should
  carry the token as written (`1.e4`) rather than the move part (`e4`).
- **S178's two deferred questions answered by the owner, 2026-09-07, DEC-147;
  both become S200, done the same day.** The whitespace form of a move number
  indication (`1 . e4`, `1 .e4`, `1. ... e5`, which PGN 8.2.2.1 allows) is
  folded into a follow-up rather than left refused, and the cut-short message
  will quote the token as the PGN wrote it -- the owner's condition on the
  second was "if not too complex", and it was sized before the step was
  written: `movetext_to_san()` returns a two-field struct, its one caller in
  the tree reads the second field for the message, about fifteen lines. One
  step and not two, because both edits are in that function and the loop that
  reads it, both are covered by the same fixture file, and neither alters play.
  It sits after S173 to group the two `make_book` tool steps; move it if the
  order should differ.
- **S201, 2026-09-07: the parser defect found while sizing S200, called a bug
  by the owner and fixed before anything else started (BUGS rule); DEC-148.**
  `algebraic_to_move()` reads the piece letter at position 0 only
  (`src/bitboard.cpp`, the `std::isupper(notation[pos])` test): a leading
  character that is not an uppercase piece letter falls to the pawn branch, and
  the disambiguation loop then swallows the real piece letter. **`.Nf3` comes
  back as `f2f3`, a pawn push, where the token means the knight move `g1f3`.**
  python-chess refuses `.Nf3`, `.e4` and `..e4` outright (checked). It is
  reachable: `1 .Nf3` is legal PGN import format, so `make_book` and
  `pgn_to_positions` build silently from a wrong board -- the shape S174 closed
  for a token the parser *cannot* read, still open for one it reads as
  something else. `Zf3` is refused and is what the suite tests; no test covers
  a non-uppercase leading character. **Bounded**: `algebraic_to_move()` is
  called only by the two tools and the tests, never on the engine's UCI move
  path, which is long algebraic; `books/8moves_v3.pgn` has no such token, so
  `src/openings.bin` is unaffected. Interior junk is lenient but *correct* --
  `N.f3` gives `g1f3` -- so only the leading character produces a wrong move.
  **The fix, narrow by the owner's choice**: after the suffix strip and the two
  castling returns, an empty token or one whose first character is not in
  `[a-hKQRBN]` returns 0. The wider gate -- every character in
  `[a-h1-8KQRBNx=]` -- was rejected: the wrong-move class is entirely a
  leading-character effect, and the interior forms it would also close give the
  correct move today. **Red first**: five `CHECK`s, `.Nf3` coming back as
  `2933`. **Verified over the corpus, not argued**: the book rebuilt through
  the changed parser is byte-identical at `77f47f1b...db06b58` -- 34700 games,
  172232 entries, 0 cut short -- so roughly 278000 real SAN tokens parse as
  before. INV-6 identical, gate 27/27 in both builds. No `Bench:` line is owed:
  DEC-140 bound from S189's completing commit and S189 was open. (S189 has
  since landed, 2026-09-08; every `src/` commit after it carries the line.)
  **One thing found in passing and not fixed, for S193**: the new
  `REQUIRE(algebraic_to_move("Nf3", &game) != 0)` is the first positive
  assertion in that TEST_CASE. Every existing subcase asserts `== 0`, so with
  `game_tables()` uninitialised -- which is what running the binary with a
  `-tc=` filter and no earlier case does -- they all pass vacuously. Under
  `ctest` the case is sound; the vacuity is the class S193 was written for.
- In progress: **nothing.** `adocs/plan_current/` is empty; S195 completed into
  `plan_done/` on 2026-09-10, S206, S205 and S192 the same day. **The next step
  is Open entry 1, S207** -- the F08 repetition rule, the night run of
  2026-09-11 (DEC-172). **The enrichment pass of DEC-145 is stopped at the
  owner's word after twenty of the then 74 files -- S178, since done, through
  S151.** Resume by handing `adocs/data/2026-09-05_enrichment_brief.md` and
  one step path to one agent per file, in Open order, one commit per file; what
  is left is named by
  `grep -L 'Implementation guide (2026-09-05)' adocs/plan_todo/*.md`. The
  session's report, with twenty findings and the owner questions from every
  file, is `adocs/data/2026-09-05_enrichment_pass.md`. The eight steps of
  2026-09-11 carry their guidance in their own files and are outside that
  pass.
- **Reordered 2026-09-05 for the workstation, DEC-144.** The owner, leaving for
  the day, asked that the plan be re-sorted for the goal under the rules with
  every step assumed to run on the Linux workstation, and that decisions be
  taken. The DEC-112 machine-scope lane is gone from `plan.md` (its text is in
  the file's history at `66cbc54`). The Open list now reads: two tool bugs
  (S178, S173); the census (S171, DEC-128's first run there); S189 and S179 as
  agent-only work while it plays; S198's harness flags and the calibrating A/A
  (DEC-143) before any verdict; then an instrument lane of the sixteen
  document and test steps interleaved with S148, S159 and S151 so the machine
  has a job while documents are written; then blocks 1 to 4 exactly as the
  2026-08-19 review ordered them and DEC-133 corrected them, S152 closing the
  main order, the reserve and the parked network last. **One question is
  deferred to the owner:** S151's re-test of S085's vector at a control at
  least four times 8+0.08 prices a `{-5, 0}` pair near 72 hours worst case by
  DEC-143's formula; whether that pair, a cheaper pair or a fixed-rounds
  reading is wanted decides when it runs -- it sits at Open entry 17 until
  then. Nothing in the engine changed, no run was started, the gate was green
  at `66cbc54` before the edit (27/27 in both builds) and the four prose
  checks pass after it.
- **The enrichment pass, DEC-145.** Also on the owner's instruction: every
  pending step file gets an `## Implementation guide (2026-09-05)` section for
  the agent that will implement it -- the technique as published, chesso's
  form, the symbols it touches at HEAD, seeds in DEC-105 form only, the tests
  DEC-141/142 require, the pair priced per DEC-143, the completion checklist,
  the repository's own traps, every source with its URL or `unverified`, and
  deferred owner questions. Engine-originated seeds in the seven DEC-134 files
  are replaced in place, so S180 becomes a verification; block 3's files are
  brought to S186's accepts, so S186 becomes one too. What the pass has not
  reached: `grep -L 'Implementation guide (2026-09-05)' adocs/plan_todo/*.md`.
  `adocs/data/2026-09-05_enrichment_pass.md` lists what each agent flagged.
  **Findings the owner should read before the workstation runs anything**, none
  fixed today (BUGS rule: the owner says which are bugs): `make_book` is killed
  by `SIGXFSZ` before S146's stream guard and leaves a truncated `loadable`
  book; the `SANITIZER` CMake block lets UBSan recover so a UB report exits 0;
  `adocs/data/S105_pairs.py` reads a side name `fastchess.sh` never prints, so
  S198's A/A read as its accepts says would inflate the variance silently, and
  `fastchess.sh` has no fixed-rounds mode without `-sprt`, so that A/A cannot be
  launched as written; `command_ucinewgame` sets the stop flag and only `go`
  and `test` clear it; `set_position` never clears `proven_mate_line`. Stale
  clauses in pending files: S190's depth-3 walk is about seven times over its
  budget, S159's second candidate is behaviour-neutral and cannot be SPRT'd,
  S148's S145 table predates S154's re-take, S184 assumes `--params` is outside
  the fast suite, S180's inventory found fifteen seeds the review missed.
- **Plan review 2026-09-04 (`adocs/audit/2026-09-04_plan_review.md`), digested
  the same day: no high, 5 medium, 5 low; every finding has a home and every
  home is a document step.** A cold reviewer read the plan against the
  published record, the code at HEAD and the project's rules; an independent
  pass fetched 33 of the plan's figures at source and is tracked as
  `adocs/data/2026-09-04_plan_review_literature_check.md` (DEC-137). Fast suite
  27/27 in both builds, both node-count baselines reproduced, 45 of 46 prior
  plan-review findings closed on re-measurement. The owner decided item by
  item: **F01** seven search steps seed constants from other engines'
  commit-message prose -- stick to DEC-105, reseed, **S180** (DEC-134); **F02**
  S099's "+11.4 at ~2850" was measured at Lynx 3226-3293, so S099 goes to the
  **reserve head** as the family's probe, S110/S111 gated on it, Lynx figures
  re-banded by **S181** (DEC-133); **F03** the cost line's 45-75 minutes per
  verdict against a ledger mean of 4 h 40 m -- **S182**; **F04** the Elo
  arithmetic with no recorded inputs -- **S183** (DEC-136); **F05, F08, F09,
  F10** stale parameter values (S115 would revert a verified SPSA axis), S042's
  narrow touches, five stale sentences, two missing `done:` fields --
  **S184**; **F06** block 3's unsourced ledgers, both located by the literature
  pass -- **S185**, and the DEC-097 enrichment becomes **S186** before block 3
  (DEC-137); **F07** 59 drifted citations three days after S169 -- citations
  into code become symbols, **S187** (DEC-135). From the literature pass, not
  the report: Ethereal removed only its *pre-move-loop* check extension, so
  the in-loop form is reopened as **S188** after S097, S096's id staying
  retired; S133's +65/+88 are release-bundle deltas, kept on adoption breadth
  (both DEC-133). Techniques the surveyed engines carry and the plan gives no
  step are recorded in DEC-138. Nine steps, 63 Open entries; `plan.md` carries
  a section on the review and inline corrections at the DEC-087 rulings; the
  report's findings read `planned`. Nothing in the engine changed, no run was
  started.
- **Test review 2026-09-04 (`adocs/audit/2026-09-04_test_review.md`), digested
  2026-09-05: no high, 3 medium, 7 low; eleven steps, four rule amendments, one
  finding accepted (DEC-139 to DEC-143).** The owner asked for research on how
  a chess engine's tests and measurements are done and an assessment of ours.
  The suite was **measured**, not read: 33 hand-written engine bugs injected
  one at a time into a scratch worktree of `5cffb70`, **31 of 32
  non-equivalent caught**, the survivor a fifty-move draw claimed at 101
  instead of 100 (F04); 98.9 % line and 92 % branch coverage of
  `src/search.cpp`, 100 % of the evaluation, the UCI book path never executed
  (F06); 35 full runs of the fast label, 0 unexplained failures, 41 to 64 s.
  The mediums: INV-2 and INV-4 live in `assert`s both gated builds compile
  out (F01, **S190**); the null-move and reduction guards have no direct test
  and five guard-removal bugs were caught by `test_mate_carry`'s count floor
  and nothing else (F02, **S191**, before S109); the suite's sensitivity
  comes largely from goldens every legitimate search change will redden (F03,
  **S192**, DEC-142). The lows: F04/F05/F09 **S193**; F06 **S194**; F07 the
  bench signature **S189** with DEC-140's `Bench:` commit line and
  `tools/gate.sh`; F08 **S195**, the Zobrist keys folded into **S179**;
  **F10 accepted** -- no harness calibration on this MacBook, the workstation
  is back soon and its A/A is the first run taken there (DEC-143, doubling
  as **S198**'s). From the survey (`adocs/testing_strategy.md`, four
  source-verified literature passes): **S196** the mutation driver as a tool
  and a mutant per new search rule, **S197** `tools/gate_extra.sh` with the
  coverage recipe, **S199** a fixed-rounds drift match against a pinned
  reference after each block; a `NodesTime` clock deferred to S127. The
  bounds now have a price before a run: a `{0,5}` pair is 41861 expected
  games at the midpoint, **17.9 h at the 2337 games an hour the seven
  ledger runs average here** (2328 to 2346), carried into S182 and DEC-143.
  The plan's "~60 % sticks" self-play ratio could not be verified at any
  source; S183 carries it as unverified. Evidence under
  `adocs/data/2026-09-04_test_review/`. 74 Open entries. Nothing in the
  engine changed, no run was started, the machine was on battery throughout.
- **Audit re-run 2026-09-04 (`adocs/audit/2026-09-04_adversarial.md`), the
  closing run for the 2026-09-03 batch: no high, no medium, three low.** All
  four 2026-09-03 findings are **closed on re-measurement from their own
  reproductions**, not from the stamps -- the Polyglot key (registered test
  green, python-chess re-derivation 0/0/0, `OwnBook` answers `d2f3` on one of the
  seven positions), the SAN parser (annotated PGN builds, `Qxf7` refused with
  exit 1 and no file), the scripts (`rating.sh --bracket` prints
  `RATING-RUN-FAILED: ordo not on PATH`) and `position fen` (short forms load, a
  bad FEN after `startpos moves e2e4` keeps the e2e4 board). Rules: nothing.
  Elo behind the literature: nothing outside the plan. Prior findings recounted:
  37 code-audit findings, 30 closed, 4 accepted, 3 planned (S039, S042, S151).
  **The three lows are `open` and wait on the owner** (DEC-035 says a run with
  no high and no medium is where the loop stops, and the lows are discharged by
  one decision or become steps): **F01** `go infinite` prints `bestmove` unasked
  on a root whose tree collapses (KNvK, stalemate, checkmate) because the
  infinite search is the depth loop to `MAX_DEPTH` 126 -- a protocol break, red
  test written and unregistered at `tests/test_audit_go_infinite.cpp`, 3 of 6
  red; **F02** a bad token in `position ... moves` is skipped and the rest
  applied, silently in Release, documented in `MANUAL.md` as intended -- the
  S176 rule applied to the FEN half of the command and not to the moves half;
  **F03** the aborted-iteration best-move logic assumes the root's table entry
  survives the iteration, which the replacement rule does not guarantee --
  mechanism shown, not reproduced, about 6e-5 per iteration by estimate.
- Last done: **S177, 2026-09-04 -- `rating.sh` and `build_release.sh` run on
  macOS as `fastchess.sh` does, and `rating.sh` prints its terminal marker on
  every exit.** On this machine `./rating.sh --bracket` died at
  `all_cores="$(nproc)"` with exit 127 and **no marker**, 90 lines before its
  trap was armed -- the WATCHERS-rule hole S167 closed for `fastchess.sh`. Now
  the marker trap and `fail()` come first, with a `marked` flag rather than `$?`
  because bash 3.2's EXIT trap reads `$? == 0` on a `set -u` abort; cores are
  counted through `sysctl -n hw.physicalcpu` with `nproc` as the fallback and a
  named refusal when neither exists; the GNU `timeout` `identify()` needs is
  resolved once as `timeout` or `gtimeout` and refused by name before any engine
  is probed; and the `command -v x || fail` checks are `if`s, since bash 3.2
  exits before `fail` runs in the old form. `build_release.sh` takes the core
  count the same way (`hw.logicalcpu` for build jobs). **Red first**:
  `tests/test_rating_script.sh`, a sandbox whose PATH holds only the utilities
  it links in -- no `nproc`, `timeout`, `gtimeout`, `sysctl` or `ordo` -- failed
  all five of its properties on the old scripts and passes after; on this
  machine the real script now answers `RATING-RUN-FAILED: ordo not on PATH`,
  exit 1, one marker, snapshot cleaned. Fast suite 27/27 in both builds.
  `DEV_MANUAL.md` (concurrency rows, rating section) and `TOOLCHAIN.md` (new
  "GNU coreutils on macOS" section) say so; `.moltke.local.md` now lists which
  scripts run here. No engine binary changed.
- Before S177: **S176, 2026-09-04 -- `position fen` takes four to six fields, and
  a FEN that does not load changes nothing.** Two defects in `command_position`.
  Fewer than six fields was dropped silently with the board left wherever it
  was, and a four-field FEN followed by `moves` read `moves` as its fifth
  field, failed to load, and applied nothing. A failed load then reloaded
  `initial_position` -- the last *FEN*, not the last *position* -- so the moves
  applied since were gone: after `startpos moves e2e4` one malformed FEN put the
  engine on the start position and applied the moves that followed to it. Now
  the clocks default to `0 1`, reading stops at `moves`, and fewer than four
  fields or a FEN `load_FEN()` rejects is refused with one `info string` line
  (`refused [position fen] <fen>, fewer than four fields` / `, does not load`)
  that ends the whole command; `set_position()` saves the whole `game_t` before
  the load and restores it on failure, about 100 KB and not on a search path.
  **Red first**: three new `test_engine` cases, 4 assertions failed on the
  unfixed tree, green after; the golden surface gained the two refusal shapes
  only after `MANUAL.md` and `specs.md` described them (SURFACE). INV-6
  identical (121512 / 800769 / 62907; 639228 / 3430710 / 367858, same best
  moves). Fast suite 26/26 in both builds. `DEV_MANUAL.md` needed nothing.
- Before S176: **S175, 2026-09-04 -- the Polyglot key follows the format on an
  edge-file en-passant square, and the shipped book is rebuilt to it.**
  `get_key()` looked for the capturing pawn at `en_passant + 7/+9` (White) and
  `-9/-7` (Black); with a8 at index 0 those wrap round the board edge on the a-
  and h-files, so `h6 + 9` read a4 and `a3 - 9` read h5, and a same-side pawn
  there switched the en-passant component on against the format. Now the two
  squares are taken by file with bounds, on the rank the mover's pawns capture
  from. **Red first through ctest**: the audit's `test_audit_polyglot_key`,
  registered, failed 7 of 10 on the unfixed tree and passes 10 of 10 after; the
  nine format example keys in `test_openings` still pass. **The book is
  rebuilt**: `src/openings.bin` is `make_book build books/8moves_v3.pgn` again
  through the gated tool, 0 games cut short, the same 2755712 bytes, 172232
  entries over 129613 positions, **sha256
  `77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58`**, swapped
  into `specs.md`, `MANUAL.md`, `DEV_MANUAL.md`, `src/openings_embedded.S` and
  S173's accepts. **Checked from outside, which the digest never could**:
  `adocs/data/S175_book_conformance.py` re-derives every entry from the PGN with
  python-chess's independent key and reports `missing 0 extra 0
  weight_mismatch 0` against the new file, where the 2026-09-03 file gave
  `missing 7 extra 7` on exactly the audit's seven positions. INV-6 identical
  (121512 / 800769 / 62907; 639228 / 3430710 / 367858; same best moves) -- the
  default configuration never probes the book. End to end: with `OwnBook` on,
  the position `rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w
  KQkq h6 0 8` -- one of the seven -- answers `bestmove d2f3` from the book with
  no `info` line. Fast suite 26/26 in both builds.
- Before S175: **S174, 2026-09-04 -- the SAN parser fails closed and `make_book`
  gates on it.** `algebraic_to_move()`'s three failure paths were `assert(false)`
  with no return, so in Release a token it could not read became a fabricated
  move that `make_move()` applied and the board was rewritten, not left illegal.
  Now each returns 0 in every build, the destination square is range-checked
  before `str_to_index()` (which asserts, and otherwise wraps), and PGN suffix
  annotations (`!`, `?` and their pairs) are stripped in the same loop as
  `+`/`#` -- in the parser, not in `make_book`'s tokenizer as the step first
  said, because `pgn_to_positions` needs it too. `make_book build` refuses to
  write when any game was cut short, names the game and the token on stderr,
  and `--allow-cut-short` is the deliberate form of the old report; a trailing
  flag with no value is an error now instead of silently skipped. **Red
  observed first**: 23 assertions over 2 new `test_chesso` cases and 9 of the
  fixture test's checks failed on the unfixed tree, green after; fast suite
  25/25 in both builds. **INV-6 identical**: 121512 / 800769 / 62907 and
  639228 / 3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`. The shipped book is
  untouched -- digest `3b89a4ad...15b873dd` -- and rebuilding it through the
  gated tool is byte-identical with 0 games cut short, so S146's `0 cut short`
  is a gate's word now. `DEV_MANUAL.md` and `adocs/specs.md` say so;
  `MANUAL.md` needed nothing, no UCI surface moved.
- **Discovered while doing S174, filed as S178, not folded in.** Run on the PGN
  import form (`1.e4 e5 2.Nf3`, no space), the fixed tools refuse at ply 0 and
  name the token -- honest, where before S174 the parser fabricated a move from
  `1.e4` and built from a rewritten board. `movetext_to_san()` should split the
  glued move number; `books/8moves_v3.pgn` has no such token (grep 0), so the
  shipped book is unaffected and the step sits after the audit batch.
- **Audit 2026-09-03, digested 2026-09-04
  (`adocs/audit/2026-09-03_adversarial.md`).** Adversarial, whole engine, cold
  context. **No high, 2 medium, 2 low**; fast suite 24/24 green and both
  `search_bench` baselines reproduce to the node; the project's own rules turned
  up nothing; every technique found behind the literature already has a step
  (S114, S098, S109, S091, S095, S097, S099, S131, S112, S022, S119, S039, S121,
  S122). Both mediums are in the code S172 and S146 shipped and were found by
  re-deriving the shipped book independently with python-chess, not by reading
  the stamps. **F01** `get_key()` wraps round the board edge for an en-passant
  square on the a/h file -- 7 of 172232 shipped entries carry the wrong key, and
  a third-party book is silently abandoned at any such position. **F02**
  `algebraic_to_move()` fabricates a move instead of returning 0 in Release, so
  `make_book` and `pgn_to_positions` (the DEC-023 board tool) run on silently
  on a rewritten board -- `1. e4!? e5` builds a `loadable` book with move `a8a7`
  from the start position, exit 0. The shipped book is nonetheless verified
  correct move for move and weight for weight; only the 7 keys differ. **F03**
  `rating.sh` and `build_release.sh` die on `nproc` before any marker is armed.
  **F04** `position fen` drops a 4/5-field FEN silently, and a bad FEN resets to
  the last FEN without its moves. **All four are steps, S174 to S177, at the
  head of the Open list** (BUGS rule); none owes a match. Of 33 prior code-audit
  findings, 27 are closed on re-measurement, 3 accepted, 3 planned in pending
  steps; two earlier reports still read `planned`/`open` on items now closed
  (2026-08-22 F01, F03), which a re-run records. The unregistered red test
  `tests/test_audit_polyglot_key.cpp` is committed with the report and S175
  registers it. `2026-08-13_plan_review.2-F06` had no trace in S023, whose
  `accepts` already carries its resolution; `closes:` is set now.
- **The magic-number question is answered: DEC-132, 2026-09-04.** The 128
  constants in `src/bb_tables.hpp` were generated in-repo (`a5dbe68`) with the
  tutorial's xorshift seed and coincide with that series' published set. The
  owner keeps them for now -- own generator output, inherited under DEC-013,
  origin in git -- and **S179** recovers or rewrites the generator, commits it,
  and regenerates both arrays under a project seed, proved unchanged on
  `bench_movegen`'s perft verification, `test_perft` and `search_bench` node
  counts. The table cites DEC-132 until then. S179 sits second in the Open
  list, after S178.
- Before S174: **S146, 2026-09-03 -- the book the engine ships with is built by
  this project, from a source it can account for.** The owner took the second of
  DEC-126's three standing options: the unaccounted blob is **deleted**, and
  `src/openings.bin` is now `build/tools/make_book build books/8moves_v3.pgn
  --out src/openings.bin` at the tool's defaults -- **2755712 bytes, 172232
  entries over 129613 positions, sha256 `3b89a4ad9146e266ae9296778067aaedcb7f57
  f3cf0ff2086b9ae6df15b873dd`**. DEC-131 is the ruling. (That digest is history
  since S175 rebuilt the file to the format's keys: `77f47f1b...db06b58`.)
- **What made it possible was S172, not new evidence about the blob.** DEC-126's
  refutation still stands -- `books/8moves_v3.pgn` shares only 11703 of the old
  book's 154916 positions, 7.6 % -- so this is a **different book, not a
  re-attribution**. What changed in a day is that S172 built `tools/make_book`;
  until 2026-09-03 nothing in this repository could produce a Polyglot book, so
  "build a replacement" had always named a tool that did not exist. The old
  163141-entry file is recoverable from `62d07d4` and from nowhere else.
- **The build is reproducible, which the old file could never be.** The writer
  sorts by key then by weight, so the digest is a function of the PGN and two
  runs are byte-identical (`cmp`, verified). Defaults were used and both are
  right rather than convenient: `--max-ply 16` is the full depth of every line
  in that PGN (**34700 games, 0 cut short, 555200 plies, exactly 34700 x 16**)
  and `--min-games 1` drops nothing. The input is CC0-1.0 and pinned by both
  digests in `books/fetch_book.sh`; the SAN is read by the engine's own
  `algebraic_to_move` and keyed by its own `get_key`, so no other engine's code,
  table or output is anywhere in the path.
- **No SPRT is owed, on S172's reasoning and not a shortcut.** `OwnBook` defaults
  false and S158 established that no measurement this project has taken ever
  played a book move. **INV-6 discharged on the default configuration**:
  121512 / 800769 / 62907 at depth 9 and 639228 / 3430710 / 367858 at depth 12,
  `c3d5` / `e2a6` / `d7c8q` at both. Weight now means something it never did --
  every game in that PGN is recorded `1/2-1/2`, so an entry's weight is the
  count of the 34700 lines that played the move; `Best Book Move` true gives
  `e2e4` at weight 12956.
- **One defect found and fixed in scope, worse than the report of it.** The fast
  check over S172's diff flagged that `make_book build` never checked its output
  stream. Reproduced on a 1 MB HFS ram disk rather than assumed: the tool wrote
  **901120 of 2755712 bytes, printed `bytes 2755712 -> <path>` and exited 0**.
  The part the report missed is why it was invisible -- entries are sixteen
  bytes and sorted, so **any prefix of a valid book is a valid book**, and
  `make_book dump` called the truncated file **`loadable`** with 56320 entries
  while the engine loaded it. Both of the loader's checks are structurally
  incapable of noticing truncation; only a digest is. Fixed, red observed before
  and green after; not a ctest, because the tool has no test target and macOS
  has no `/dev/full`, so the reproduction is in the step file and the hazard is
  in `DEV_MANUAL.md`.
- **The stale citations went with the blob.** `src/openings.book` had not existed
  since `62d07d4` and S146's own reproduction commands still `sed`-ed a
  `#define BOOK` out of it. Re-anchored across `DEV_MANUAL.md`, `adocs/specs.md`,
  `MANUAL.md` (new section "The book it ships with"), `src/openings_embedded.S`,
  `src/openings.cpp` and `tests/test_openings.cpp`. `polyglot_randoms[781]`
  needed nothing -- settled at DEC-121 on 2026-09-02, untouched.
- **S146's own fast check produced one finding, which was wrong, whose fix is
  right.** It reported that the new failure path's `std::remove` destroys a
  pre-existing file at `--out`. It does not: `std::ofstream(path, binary)` opens
  `"wb"` and truncates, so the contents are gone at the open -- verified, a bare
  open with no write and no remove took a nine-byte file to zero. What is real
  is that the whole replacement is not atomic: the tool destroys the old book
  before it knows it can write the new one. **S173 is that fix** -- write beside
  the destination, rename on success -- filed in `plan_todo/` and 49th in the
  Open list, not urgent because `--out` points at `src/openings.bin` about once
  and that file is in git. The other four answers checked out: `close()` then
  `good()` is the right order, `std::remove` binds the `<cstdio>` overload
  unambiguously, and the entry count and sha256 in the comments match the file.
- **The fast check over S172 is done and it took two passes.** The first returned
  "No issues" over 631 new lines of PGN parsing and a rewritten `setoption`; that
  was not accepted, and a second pass answering six named questions with quoted
  code returned the short-write finding above. Recorded because the lesson is
  cheap: a clean review and a shallow one look identical until the questions are
  specific.
- **S171's postponement, kept for the record.** It was postponed here from the
  MacBook by DEC-128 after two attempts died on that machine -- `pmset -g ac`
  said `No adapter attached`, which the POWER rule forbids (DEC-109), and on
  mains `fastchess.sh`'s own load guard reported `about 387% of a core is
  already busy` with Spotlight indexing PDFs. The fix itself was committed and
  green there at `136b03f`: `certified_mate_move()` completes a mate line across
  a table slot the walk has lost, `tests/test_mate_carry.cpp` guards it, INV-6
  discharged. It keeps that evidence -- the five stride-1 cases still report 0
  short lines -- and what the census found is a different cause it never
  claimed. The run itself is the entry at the top of this file.
- Before S146: **S172, 2026-09-03 -- an opening book is loadable over UCI, on
  the option names the protocol actually uses.** `OwnBook` (check, default
  false), `Book File` (string, default `<embedded>`) and `Best Book Move`
  (check, default false) replace `Use Book`, which is removed. A named book that
  will not load leaves the engine **bookless** -- never falling back to the
  built-in one -- and says so as `info string book [<path>] not loaded: <why>`
  on the UCI channel. Selection follows the Polyglot `weight`, which the engine
  had never read. Three things that had never run at all were fixed:
  `load_book_from_file()` had **no caller anywhere** since the `bitboard`
  branch; `setoption` read **one token** as its value, so a path with a space
  arrived truncated; and the probe **scanned every entry** where the format's
  key ordering makes a binary search exact. The container also changed --
  `src/openings.bin` pulled in by `.incbin` instead of a 5.2 MB hex header
  decoded at every startup, **4.7 ms +/- 0.4 to 2.8 ms +/- 0.1, x1.67 +/- 0.15**
  over 574 and 976 `hyperfine -N` runs. DEC-129, DEC-130.
- Before S172: S170 -- **a mate score the search did not itself prove is
  reported with a line that reaches it.** Three causes, one fix each, all
  reporting-only and all under DEC-122; `tests/test_mate_carry.cpp` is the guard
  and `adocs/data/S170_replay.py` with `S170_cases.tsv` the reproduction. Its
  own `--fast` run took 12 lines over 3 searches down to 5 over 1, and the 5
  became S171 above.
- Before it: S147 (**a mate line the search proved reaches its mate**, 54 short
  of 706 before and 0 after; DEC-122, DEC-123), S144 (**a citation in a plan
  document carries its own path**; DEC-120), S169 (**the 97 stale citations are
  re-anchored**; DEC-119), S143 (**the completion gate builds and tests
  `build-tune` beside `build`**; DEC-118).
- **S146's first half was DEC-121 and it still stands**: `polyglot_randoms[781]`
  is format-defining specification, kept, cited at the array, with all 781
  constants verified element by element against the live format description.
  The second half is closed above at DEC-131 and the blob is gone; DEC-126's
  measurement of why the PGN was never the blob's origin -- **11703 positions of
  the book's 154916, 7.6 %** -- is what keeps the replacement honest as a
  replacement rather than an attribution.
- **What the episode did settle, and it is committed.** `books/8moves_v3.pgn`
  is committed, played by `rating.sh`, and had no recorded origin anywhere. It
  is byte-identical to the file in `official-stockfish/books`, which is
  **CC0-1.0**, and is now pinned in `books/fetch_book.sh` by both digests --
  zip `7e1e9dd1...`, unpacked `5835239f...` -- so one file answers where every
  book here came from, and `./books/fetch_book.sh 8moves_v3.pgn` verifies the
  tracked copy in place instead of downloading. `UHO_Lichess_4852_v1.epd`
  needed nothing: that script already carried its digests and the licence
  reasoning. `DEV_MANUAL.md` says the same.
- **`python-chess` was missing on this machine and is installed now**, at the
  owner's decision of 2026-09-01: 1.11.2 on python 3.9.6 in `~/.venv/chess`,
  the path `TOOLCHAIN.md` and every S145 script already name. It had not
  survived the move from the Linux workstation, so
  `adocs/data/S145_rfp_sweep.py` and `S145_mate_set.py` could not run here at
  all and nothing said so. `.moltke.local.md` records it now.
- Last done: **S148, 2026-09-09 -- the reverse futility ceiling re-decided by
  SPRT and kept at 15.** One integer measured and reverted; documents, one
  `src/` comment and one test comment carry the price. `No functional change`.
  (Older `Last done:` lines sit below this one; this is the live pointer.)
- Last done: **S193, 2026-09-09 -- the fast suite's vacuous assertions made
  falsifiable and the fifty-move boundary pinned.** Tests and documents only,
  no `src/`, no Bench line. (This is the live pointer; the S184 line below is
  the previous one.)
- Last done: **S191, 2026-09-09 -- every null-move, reverse-futility and
  reduction guard has a direct case, each red under its own mutant, and the
  S165 defender set is a registered fixture.** `src/search.cpp`,
  `src/search.hpp` and `src/data_structures.hpp` changed and the engine's tree
  did not: `No functional change`, INV-6 identical at 9 and 12.
- Last done: **S184, 2026-09-08 -- the pending documents at HEAD values and
  `--params` extended over them.** Documents and one checker change, no `src/`.
  (Two earlier `Last done:` lines sit above this one, S189's and S177's, from
  earlier sessions: they are that log's chronology and this is the live
  pointer. Nothing here reconciles them -- a flat list carrying three of the
  same field is a hygiene finding and not S184's scope.)
- Next: **S216**, Open entry 1 -- the killer-slot census that measures a
  position the engine now refuses, the second of the two Tier-1 fast-check
  findings and the last entry before the run-owning work; node-identical and it
  owns no run. **The next run-owning entry is S024**, entry 2, the largest
  ordering gain surveyed -- one SPRT, slow class by S182's ledger, so a night
  run -- with **S214** as its filler. Behind it **S211**, then **S151**,
  entry 5: DEC-172 takes design (iii), a fixed 1000-pair estimate at
  `32+0.32`, `Hash=64`, about 3.4 h and a daytime run.

- **S193's fast check found one real thing and it is S205, not a mid-step
  fix.** `tests/test_perft.cpp` parses four more columns than it compares --
  `checks`, `discovery_checks`, `double_checks`, `checkmates` -- and
  `get_move_stats` sets all four to 0, so `perft()` does not count them at all.
  The reviewer read this as latent on the belief that every asset layer leaves
  them null; counted, **42 of 71 layers carry a `checks` value and 42 a
  `checkmates` value**, 30 each for the other two, so these are real expected
  numbers in tracked assets that no run has ever compared. It is a step and not
  a one-liner because counting checks means `is_check()` at every perft node
  and the slow label runs 53 s -- the measurement is what decides between
  counting them and deleting the dead fields. **Done: the measurement said
  count them, +27 %, and it also said the reviewer's 42/30/30/42 counts layers
  a run never reads -- 33/21/21/33 is the number, and the four columns do not
  mean what they look like.** See the S205 bullet at the top. The rest of the
  check came back clean, including the two removals S193 claimed were the clamp
  and the no-op filter restated, both verified against `src/`.

- Extra gate: last **GATE-EXTRA-DONE 2026-09-10 `4795ef4` 12:54**, on the tree
  `4795ef4` committed -- the re-run after the fast check's four fixes, 774 s
  against the 768 s of the first green run on `04effb3`. The sha is
  filled in by the commit after the one it names, because a commit cannot
  contain its own hash and an amend that tries moves it again. DEC-141 clause 3 is the cadence -- before a
  step that touched `make_move`, `unmake_move`, the generator or the search
  completes, and otherwise weekly -- and this bullet is where a missed week
  shows (DEC-167). Export `CLANG_FORMAT_MAJOR=22` in the launching shell first
  or stage 4 goes red on the formatter (DEC-146).
- Blocked: **nothing.**
- Watching: **nothing. No run is armed.** S215 armed two, both on
  `MUTATION-RUN-(DONE|FAILED)` over a polled log with a 30-minute ceiling and a
  liveness check on the run's pid, and both exited on `MUTATION-RUN-DONE` --
  329 s for the pair at `177171e`, and the re-run at `cfe2406` -- each
  acknowledged and read in the turn that armed it. Before them, S148's SPRT
  finished at 01:57 on 2026-09-09 and its watcher exited on `SPRT-RUN-DONE`,
  acknowledged and read in the same turn; the verdict is recorded above and in DEC-158. Its PGN,
  73 MB at `/tmp/chesso_sprt_nonreg_20260908_193719/games.pgn`, is **not**
  committed and `/tmp` will take it; the log is, and carries every game's
  result. Before it, S198's A/A finished at 02:39 on 2026-09-08.

- Parked:
  - **Three findings filed by S195, none of them planned.** (1) `MANUAL.md` has
    no sentence saying what `ucinewgame` resets; the owner decided on
    2026-09-10 that S195 would not add one, so the surface stays undocumented
    on that point. (2) `MANUAL.md`'s `nodes` wording, "counting every
    iteration", is not the whole truth: the last `info` line is printed only
    when the iteration had a result, so a final iteration that aborts before it
    has a PV leaves the reported count **under** the budget -- measured
    2026-09-10, `go nodes 5000` reports 2917. Same decision, not added. (3)
    **`command_bench`'s per-position `reset_for_new_game()` is unobservable and
    no test can reach it.** Commenting it out leaves `bench 9`'s output
    byte-identical on every `nodes`, `score` and `pv` field and on the
    signature total, because the eight FENs are pairwise distinct and
    `set_position` already resets on a differing FEN. It is a guard against a
    bench list that ever repeats a position, and that list cannot be
    constructed from outside the binary; the only thing that would make it
    testable is surface `command_bench` does not have. Parked, not planned: a
    step is created by a decision and none has been taken on any of the three.
  - ~~**Calibrate the harness on the workstation.**~~ **Taken 2026-09-08 as
    S198's A/A and retired**: 1000 fixed rounds, 0 forfeits, pair variance
    0.2430 +/- 0.0154 inside S105's band at `z = +0.16`, 2277 games an hour.
    The next machine change owes the next one under DEC-143.
  - **S151's pair, the owner question from the 2026-09-05 reorder, is answered
    by DEC-172 on 2026-09-11 under the owner's delegation**: design (iii), a
    fixed 1000-pair match at `32+0.32` and `Hash=64` read as an estimate, about
    3.4 h, and the longer-control rule in its block-boundary form. Its accepts
    is amended in its file. Kept here one turn so the owner sees the question
    closed; prune at the next rewrite.
  - **The three lows of the 2026-09-04 audit re-run still wait on the owner**
    (`go infinite` printing `bestmove` unasked; a bad token in `position ...
    moves` skipped silently; the aborted-iteration best move assuming its table
    entry survives) -- one decision or steps, unchanged by the reorder.
  - **HANDOVER TO THE MACBOOK, 2026-08-23. Discharged 2026-08-27 -- kept for
    what it explains, not as a thing to do.**

    **The check passed to the node.** `search_bench` at depth 9 gives
    121512 / 800769 / 62907 and at depth 12 gives 639228 / 3430710 / 367858,
    best moves `c3d5` / `e2a6` / `d7c8q` at both, exactly as predicted below.
    `build_lmr_table()`'s `log` was the stated portability exposure and Apple's
    `log` does not move the tree, so every deterministic figure this repository
    records is valid here as written and nothing was regenerated. Throughput in
    games per hour was still unmeasured when this was written and is not any
    more: **2700 games/h on 8 threads at 8+0.08**, from S024's run 3, which is
    the one thing those aborted runs did buy. `.moltke.local.md` is written.
    The commits were pushed. What the move did break was two things the
    handover did not predict, both found by running the check: the tree did not
    compile under Apple clang's `-Werror`, and `fastchess.sh` could not complete
    a run under bash 3.2 -- **S167** is both, and until it landed no SPRT could
    have been taken here at all.

    The original text follows, because its reasoning about which figures carry
    and which do not is the standing rule, not a one-off.

    **S203 moved these numbers on 2026-09-08 and the check still works, with
    the new set.** Redrawing the Zobrist keys changes which positions share a
    transposition-table slot, so the counts move once by design; the values
    below are what the engine gave until that step, and from it on they are
    **121530 / 801481 / 72924** at depth 9 and **636677 / 3520847 / 494098** at
    depth 12, best moves unchanged. `adocs/specs.md` under INV-6 carries the
    current pair and is the one to read. The original text follows unedited,
    because what it says about which figures carry across a machine and which do
    not is the standing rule.

    **Start here, and it takes ten seconds.** Build, then run
    `python3 tools/search_bench.py ./build/src/chesso 9` and the same at 12.
    Expect **121512 / 800769 / 62907** at depth 9 and
    **639228 / 3430710 / 367858** at depth 12, best moves `c3d5` / `e2a6` /
    `d7c8q` at both, exactly. If they match, every deterministic figure this
    repository records is valid on the MacBook **as written**, and nothing needs
    regenerating. If they do not match, stop and read the next paragraph before
    trusting any node count in any document.

    **Why that check and not a re-baseline.** The numbers here fall into two
    classes and only one of them is machine-bound. *Deterministic*: node counts,
    best moves, perft, the reach counts, the counter tallies. At a fixed depth
    with `Threads=1` these are properties of the algorithm and not of the
    machine, so they carry across unchanged -- which is why one comparison
    confirms the whole class instead of re-measuring it. *Time-based*: nps
    ratios, games per hour, Elo verdicts, the CCRL estimate. Those cannot be
    carried and **do not need regenerating either**: DEC-049 already decides it
    -- comparability is a property of a (machine, compiler) pair, and the
    TOOLCHAIN.md warning "bites only when a figure is carried across machines".
    Every such figure stays attributed to the machine that produced it and
    remains valid as that. No re-run makes S108 more honest than it is.

    **The one thing that could break the deterministic class, stated so the
    check has a purpose.** `build_lmr_table()` at `src/search.cpp:102` is the
    engine's only floating-point path: `(LMR_BASE / 100.0) + log(depth) *
    log(move_number) / (LMR_DIVISOR / 100.0)`, truncated to `uint8_t`. glibc's
    `log` and Apple's are not bit-identical, and a 1-ULP difference where that
    expression sits just under an integer flips one reduction by a ply, which
    moves the tree and every node count with it. That is the entire portability
    exposure and the search_bench comparison is what detects it. If it fires,
    the fix is not to re-measure the documents: it is a decision about whether
    the table is built from a portable integer approximation instead, and it
    would be the first decision taken on the MacBook.

    **The only figure the MacBook actually lacks is throughput**, games per hour
    at `8+0.08`, needed for scheduling and nothing else. It arrives free from
    the first hour of the next real SPRT -- do not spend a dedicated calibration
    run on it. This machine did 2340 games/h on 12 threads for comparison.

    **`.moltke.local.md` does not travel and the MacBook has none.** It is
    machine-local and uncommitted by design, so write a fresh one there before
    measuring anything: the tool paths, the core count, the concurrency default
    and the compiler are all different. `TOOLCHAIN.md` is macOS throughout and
    is *correct* there while being stale here, which is the reverse of the
    situation its own Open note describes -- so on the MacBook TOOLCHAIN.md
    becomes the reliable document and `.moltke.local.md`'s Linux notes are what
    is gone.

    **What does not survive the move at all.** `.tuning/` is 1.4 GB and
    gitignored: `selfplay_v2.tsv` is 683 MB and regenerable at the cost of a
    night, and the five fit scripts are **32 KB in total** -- `anchors.py` 16 K,
    `apply_fit.py`, `verify_fit.py`, `reanchor.py`, `diff_fit.py` 4 K each.
    `anchors.py` is still the only executable record of how ten pinned test
    values are derived, including the quiescence composite no evaluation model
    can produce. **Answered on 2026-09-10:** all five have been tracked since
    `c56ab41` and S192 moved that one out of `.tuning/` altogether, to
    `adocs/data/S192_anchors.py`. The corpus is what still does not survive a
    move.
    `.ref-builds/` is 2.2 GB of gitignored worktrees and is worth nothing --
    `fastchess.sh` rebuilds them on demand.

    **The commits must be pushed before the machine goes down.** The GIT rule
    is that the agent never pushes. At the time of writing `achesso` is
    ahead of `origin/achesso` by three: `bbbd9f4` (DEC-108), `3b39e5a` (S108's
    hoist) and S108's completing commit. Unpushed, the MacBook sees a branch
    that ends at `77ecb6f` and none of S108 exists.

    **Where the work stands.** **S108 is complete.** H1 accepted at `elo0=-5 elo1=0` in 12774 games and 5 h 26 m against `bbbd9f4`, `Elo 2.28 +/- 4.40`, `LLR 2.98`, 0 forfeits -- a regression of 5 nElo or more excluded (**3.64 logistic Elo** at this run's own `nElo 3.13` against `Elo 2.28`; the bounds are normalized Elo, S157), no gain claimed. The engine now computes a static evaluation at every non-check main-search node and the table entry carries it, which is the input S109 reads. The next step in plan.md order is
    **S024**, continuation history -- not S109. It is play-altering and owes an
    SPRT, so it is the step that wants the throughput figure above, and it is
    the one to start with on the MacBook once the search_bench check above has
    passed. **S109 sits behind it** and carries the layer S108 deferred -- a
    direction-certified table score as a pruning margin's input, written up in
    its step file under `## Inherited from S108` -- so that decision is waiting
    where the step that takes it will be read, not here.
  - **Nothing checks that the two copies of the completion gate agree.** S143
    put the command in `AGENTS.md`'s TESTS rule and in DEV_MANUAL.md's Test
    section and made them identical; DEC-118 says they are changed together,
    which is an assumption and not a guard. `tools/plan_prose_check.py
    --params` is the precedent for turning exactly this class of prose drift
    into a `fast` test -- it already reads documents and compares them to the
    code. Parked, not planned: a step is created by a decision and none has
    been taken on this.
  - **The gitignored-evidence exposure was real and S024's share of it is
    discharged, DEC-111.** This item used to say `.tuning/` held the only copy
    of S024's aborted run and that it would die with the machine. It would
    have. All of it now sits on branch `s024_mac_attempt` under
    `adocs/data/S024_mac_attempt/`, gzipped, with digests and a README: both
    PGNs, the three run logs, the fastchess log, the resume config and the
    resume script. The exposure itself is not discharged -- `.tuning/` is still
    gitignored and still holds the corpus and the fit scripts the item below
    describes, which are the larger stakes. What changed is that the pattern
    has a worked example now: a branch nobody merges is a cheap place to put
    evidence that must outlive a machine.
  - **A fourth question, from S085's run: what should `RFP_MIN_PLY`'s declared
    minimum be?** Measured 2026-08-20: the tested floor is **2**, not the 3 the
    comment argues for, and **0 and 1 are the same engine** because `!is_pv`
    exempts the root, not this parameter. 3 of 18 mate cases fail at 0 and 1;
    all pass at 2. Narrowing to 2 is measurement-backed; narrowing to 3 matches
    the stated purpose but rests on an argument no test exercises. The wrong
    claims in `src/search_params.hpp` are corrected; the bound is left at 0
    because changing it is a decision. `RFP_MAX_DEPTH` has the same shape and no
    red test -- its comment says "the last few plies" and its max of 63 permits
    every depth. Full measurement in S085's step file.
  - **Three questions banked for the owner while working overnight, 2026-08-20.**
    Asked here rather than blocking the run. (1) **Resume the SOTA enrichment
    pass?** 20 pending steps are still unenriched and it was stopped at S120 by
    your instruction earlier the same day; it is machine-free and parallelises
    one agent per step, but it is not a plan step, so it did not fit the
    instruction that plan steps run in sequence. (2) **S085's goal line says
    "the twenty search parameters that exist today" and the frozen run is 12** --
    your decision at freeze time, with the nine `Tm*` and `OrderHistoryMax`
    excluded for reasons recorded in the step file. Amend the goal line, or let
    the `done:` stamp carry the deviation? The step's own Scope concern already
    flags that the live surface is 22 rather than the goal's twenty. (3) **The
    fourth plan_review is running documents-and-citations only**, because its
    method re-measures every numeric claim from the tool the step names and the
    machine is committed to S085; each unverifiable number is being recorded as
    deferred with the command that would settle it. Schedule the re-measurement
    pass as its own run, or fold each deferred number into the step that owns it?
  - **The SOTA enrichment pass over the 3000-Elo steps stopped at S120, by the
    owner's instruction, 2026-08-20.** One research agent per pending step
    appends a `## Technical details (SOTA research, 2026-08-19)` section —
    published form, traced records, file:line grounding, seeds per DEC-084,
    measurement plan per DEC-083/S105. Done: all of block 0 (commit 0edfd26)
    and block 1 (commit c0954ec), plus S020, S055, S117, S120 of block 2.
    **The remainder is derived and is no longer enumerated here.** The
    enumeration this item used to carry — "28 of 48" and twenty ids — was
    already three steps stale the day after it was written, because both ends
    move: every completion shrinks the denominator and every new step grows it.
    The recipe is the answer and it is self-updating:
    `grep -L 'Technical details (SOTA research' adocs/plan_todo/*.md` names
    what is left, `grep -l` the same pattern names what is done, and neither
    reaches a step sitting in `plan_current/`. **Snapshot, 2026-08-21, stated
    as a snapshot and not as a census: 22 enriched, 44 not, over the 66 files
    in `plan_todo/`.** Of the 44, five are excluded by design (reserve
    S023/S025/S110/S111 and parked S029) and nineteen are steps created after
    the pass stopped — S134 to S136 and S141 onward — so what the pass itself
    left behind is the other twenty. Several sections flag scope concerns for
    owner decision — the largest: S097's section notes Lynx's README
    self-rating (3144/3293) against DEC-087's ~2850 banding of Lynx-derived
    records. S109's no-prune-when-giving-check clause was the other one and is
    resolved: S139 split it, and what is left is the owner question the field
    now states, whether to buy late move pruning the exemption with a post-make
    prune.
  - **The 2026-08-16 plan_review findings are planned, not closed, and closing
    them needs a re-run.** Ten findings, one high and three medium: nine went to
    steps (S069 to S072, S074, S078 to S081) and F08 to DEC-062. A finding moves
    to `closed` only when a re-run no longer reports it, so a fourth plan_review
    is owed once those steps land — and the severity profile says the loop has
    not reached its stopping condition, which is a re-run with no high and no
    medium. F05 was this file's own two parked items and S069 has rewritten
    them; it is `planned` until the re-run, like the rest.
  - **The corpus is gitignored and does not survive a machine move; the fit
    scripts do -- corrected 2026-09-05, and the anchors half is closed by S192,
    2026-09-10.** `.tuning/` holds `selfplay_v2.tsv` (715 MB, 11003693
    positions) and `selfplay_v2_dedup.tsv` (706 MB), both gitignored, and both
    are on this workstation though neither was on the MacBook. The five scripts
    S065 leaned on — `apply_fit.py`, `verify_fit.py`, `anchors.py`,
    `reanchor.py`, `diff_fit.py` — **are tracked** since `c56ab41`
    (`.gitignore` carries `!.tuning/*.py`), which this item wrongly called
    gitignored until S192's enrichment agent checked. **The one that did not
    run is fixed:** `anchors.py` hard-coded `ROOT = "/home/max/ws/chesso/"` and
    is now `adocs/data/S192_anchors.py`, root from `__file__`, printing
    `10 of 10 reproduced` at HEAD (`adocs/data/S192_anchors.log`). The other
    four stay in `.tuning/`. What remains parked is the corpus itself, which is
    the same class of loss `2026-08-13_plan_review.2-F02` recorded when
    `selfplay_v1.tsv` did not survive DEC-049, and which cost S065 a night of
    regeneration. Found while completing S065. Parked, not planned: a step is
    created by a decision and none has been taken on this.
  - **DEC-033's ordering conclusion is superseded by DEC-081; its measurement
    is not.** 160 expensive moves re-asked at 16 times the search removed 24.1 %
    of the error and left 95 of 160 unchanged, and that is still true. What it
    priced was *nodes*, and sixteen times the nodes is about four plies in this
    tree: measured 2026-08-19, chesso reaches depth 12 on 1448572 nodes where
    Stockfish reaches depth 15 to 16 on 158837. So the evaluation was the
    binding constraint in 2026-08 — S028 at +188.74 and S065 at +21.10 are that
    cashed — and the tree shape is the constraint now. The search block leads
    the evaluation block. S019 is still retired.
  - **S015's quiescence SEE pruning has never been re-measured.** It returned
    0 Elo when `see()` cost 12.1 % more than it does now, so the same feature is
    a cheaper trade today than when it was judged. Folded into S022 rather than
    given its own step.
  - **S013's LMR SPRT was never formally concluded.** Killed at 96 % LLR,
    +129.2 +/- 33.8 over 183 games. The trend was unambiguous and the machine was
    needed elsewhere. Nothing depends on closing it; recorded so nobody reads
    "passed" into a run that was stopped.
  - **Measurement capacity is still the binding constraint, and S105 bought
    less than it was priced at.** Landed 2026-08-20: `tc=8+0.08`, `Hash=16`
    (DEC-088, not DEC-083's 128), `UHO_Lichess_4852_v1.epd`, all 12 threads of
    the DEC-049 machine. **Measured x1.67, 23.1 to 38.7 games a minute**,
    against the x3 DEC-083 predicted — x1.41 from the control and x1.20 from
    shorter games. **The unbalanced book bought nothing at the pair level**
    (variance ratio 1.022) and Pohl's 45 % draw floor is unreachable at this
    strength: the *balanced* book measured 40.3 % draws. It is kept on the
    x1.20 and on DEC-083, and whether to keep it is the owner's to revisit —
    the argument is in `adocs/data/S105_calibration_pairs.txt` and
    `adocs/plan_done/S105_sprt_harness_regime.md`. Find the source paragraph in
    `specs.md` under "Open items" by the phrase "Measurement capacity is the
    binding constraint" — this item carried a line range into that file three
    times and it was stale all three times, which is why it is a phrase now.
    A verdict that stops early is cheaper — S033's took 44 m 10 s for 1012
    games — and one that goes the distance costs the full window. Nothing else
    is measured while a match runs, and data generation and fits compete for the
    same machine.
  - **The bounds decide whether a night buys a verdict at all, DEC-063.** S068
    measured one constant twice with the same binaries: `elo0=0 elo1=5` ran
    6 h 36 m over 9036 games and returned nothing, `elo0=-5 elo1=5` returned
    H1 in 1 h 41 m over 2312. Both at 1371 games/h, so the cost of a verdict is
    set by the hypothesis pair and not only by the hardware. Not parked as work:
    it is a rule, and the pending steps it constrains are S021, S023, S024 and
    S026.
  - **Steps S001 to S016 were retro-stamped at moltke adoption**, not completed
    under the workflow. Their measurements are transcribed from the commits and
    from the two plan documents they replace (DEC-027). Treat their `done:`
    stamps as provenance, not as evidence that any completion gate ran.
  - **The `info` line is UCI surface that the golden guard does not cover.**
    `test_uci_surface` holds the command set, the option lines and the `go` and
    `position` tokens against `MANUAL.md`; the search output has never been in
    it, which is how S037 could add an `nps` field and change what `nodes` and
    `time` mean without any test noticing. `MANUAL.md` now documents the fields,
    so a golden check has something to hold them against. Found while doing
    S037. Parked, not planned: a step is created by a decision and none has been
    taken on this.
  - **The tuner's last `--only` group runs to `PARAM_COUNT`, and that is a blind
    spot S041's test cannot cover.** `tempo` ends at `PARAM_COUNT`
    (`tools/tuner_groups.hpp`), so a parameter block appended after it and given
    no group of its own is covered by `tempo` and all three partition properties
    still hold. `test_tuner_groups` catches it through a precondition instead —
    `TEMPO_EG_BASE + TEMPO_COUNT == PARAM_COUNT`, the bases chain against
    `eval_model.hpp`'s independent width sum — which fires but names the wrong
    thing. The structural fix is a group table the last entry cannot outrun, and
    it would delete the four historical comments that are the only record of the
    defect. Found while doing S041, whose `excludes:` puts it out of reach.
    Parked, not planned: a step is created by a decision and none has been taken
    on this.
  - **NNUE is deferred and S029 is parked, DEC-054.** The owner's decision:
    strength comes from search and from the hand-crafted evaluation instead.
    Parked is not retired — `plan_todo/S029_nnue.md` is kept whole, its id is
    not reused, and its plan.md entry moved to the end of the pending order
    marked parked, so nothing derives it as the next step. Resuming it is a
    decision, not a drift. The standard search machinery that used to be parked
    beside it is no longer: **DEC-071 is the decision that was missing**, and
    the 2026-08-19 review (DEC-081 to DEC-086) is what ordered it — a search
    block that leads, an evaluation block that follows it, and one step, S109,
    where four pruning rules that are inert apart are measured together.
