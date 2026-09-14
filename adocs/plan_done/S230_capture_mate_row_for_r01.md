id:         S230
goal:       the "pruning does not hide a forced mate" table regains a mined position whose line runs through a losing capture and separates S091's R01 mutant (the extra reduction ignoring gives-check) at a measured depth, after S222's ordering moved the row that did
accepts:    one new row in `tests/test_search.cpp` "pruning does not hide a forced mate", a position mined from the repository's own sets (the S145 sets or the S219 A/A corpus) by the rule the table states -- the shipped build reports the mate at the row's depth and the R01 mutant does not -- verified with Stockfish through python-chess (the oracle's line recorded), observed red under `tools/mutants/S091_capture_see.py`'s R01 by hand and reverted; the depth read as a measurement over depths 3 to 12 the way DEC-209's clause 4 describes; the other rows' labels unchanged; no `src/`; `No functional change`
touches:    tests/test_search.cpp, adocs/data/, DEV_MANUAL.md
excludes:   any change to S091's rules or to the mutant; re-picking an existing row's depth
decisions:  DEC-209, DEC-141, DEC-142
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-14 16:16, filler while S091 waits for its SPRT night and S222 for its SPSA night
done:       2026-09-14. **The row was mined, not picked, and the yield says how
thin R01's kill is in the tree S222 left: two positions out of 281 separate it
at any depth from 3 to 12, and both by one depth.** `adocs/data/S230_mine_r01_row.py`
is the whole procedure in four stages -- `pool` built **39987 FENs** from this
project's own games alone (the two committed S145 mate sets plus the last 40
plies of every game of `adocs/data/S219_aa_calibration.pgn`, deduped, positions
already over dropped); `cmd_label` ran one stockfish at 50000 nodes over them
as a shortlist; `cmd_line` re-asked each mate in a **fresh** process at depth
20 and kept the ones whose principal variation carries a capture that gives
check for the mating side; `cmd_depths` compiled a throwaway driver against
`build/src/libchesso_engine.a` -- `search_fen()` of `tests/test_search.cpp`
line for line -- and swept depths 3 to 12. **The instrument is `search_fen()`
and not `go depth N`, deliberately and against the brief**: the case calls
`search()` once at a fixed depth from a cold table, the UCI reply is iterative
deepening over a table that carries between depths, and the table's own comment
already records that the shipped engine answers these positions in centipawns
over UCI at the depths they are read at -- mining on the UCI reply would have
picked rows the case then fails. The driver was validated before it was
believed: on the three existing rows it reproduced `d7 d9 d10 d11 d12`,
`d7 d9 d10 d11 d12` and `d9 d10 d11 d12` with distances 5, 5 and 4, which is
what the table says to the depth. **Counts.** The label pass was stopped at
**32000 of the 39987** once the row was decided and a 167-position widening had
already returned nothing -- the brief's budget, and the marginal yield from the
game tails was measured at roughly **one mate per thousand positions against
two in three for the S145 sets** (267 of the sets' 400, 30 over the 31600 game
positions scanned) -- the same figure `adocs/data/README.md` now carries. It
found **297 mates** in distance 2 to 6; all
297 went through the line filter and **117 survived** it. **281 distinct
positions were swept** over depths 3 to 12 on the shipped build and again under
R01 applied by hand, and they add up as **114 + 167**: 114 of the 117 survivors
-- the other three are the rows already in the table, swept on the shipped
build only -- plus the 167 labelled mates the line filter dropped, kept in the
pass because a position without such a capture on its oracle line can still
separate a mutant and the pass had to be able to say that it did not. **2
separated**:
`8/2N1Pkp1/1p1P3p/2p2p2/5p2/1P3P2/6PP/b4K2 w - - 3 41` at depth 12 alone, and
the row that shipped at depth 11 out of four. The second was taken because a
shipped profile of four consecutive depths is a row and a profile of one depth
is the fragility DEC-209 punished. Oracle time about 50 minutes.

**The row.** `1r3r1k/2p1n1pp/8/p2n1p2/2BPp3/Q1B1P2q/1P3P1P/2R1R1K1 b - - 1 22`,
depth 11, mate in 5, label **"C02 and R01"** -- and it is the only row of the
table not from the S145 sets: python-chess located it at ply 37 of game 64 of
`adocs/data/S219_aa_calibration.pgn`, `candidate` against `ref-5047070`,
adjudicated 0-1, this engine playing itself. Stockfish through python-chess in
a fresh process at depth 20: **`#+5` in 16769 nodes, pv
`f8f6 a3d6 f6d6 g1h1 d6g6 c4f1 h3f3 f1g2 f3g2`** = `22...Rf6 23. Qd6 Rxd6
24. Kh1 Rg6 25. Bf1 Qf3+ 26. Bg2 Qxg2#`; python-chess reads the root
`is_valid True`, `is_check False`, **45 legal moves, 5 captures, 0
promotions**. `Qxg2#` is the capture on the line and it is the mate.
**What is stated rather than glossed:** the engine's own `see_ge` *clears*
both captures the oracle plays -- `Rxd6` and `Qxg2#` both answer
`see_ge(move, 0) = 1` -- exactly as it did for the R01 row S222 removed. What
R01 reaches here is not the line, it is the position: the root's own `Qxh2+`
is a capture that gives check which `see_ge` writes off, and over the first two
plies -- 1367 nodes not in check -- **1487 captures give check and 1478 of them
lose material by that same `see_ge`**, measured with the engine's own function
through a throwaway probe. The separation is the measurement and the comment
claims nothing more.

**Both depth lists, DEC-209 clause 4.** Shipped `d9 d10 d11 d12`; under R01
`d9 d10 d12`; under C02 no mate at 11. **The label was observed one mutant at a
time** by the apply/build/sweep/revert loop of `.tuning/coord/S230_mutant_sweep.sh`,
each of the six mutants of `tools/mutants/S091_capture_see.py` patched into the
working tree, `chesso_engine` rebuilt and the position run at depth 11: C02 and
R01 report no mate, C05, C06, C07 and R02 report `#+5`, so the label names
exactly those two. The other three rows' depths, distances and labels are
untouched and were re-derived unchanged.

**Red, verbatim**, with the row in place and R01 patched in by hand,
`ctest --test-dir build -R "^test_search$" --output-on-failure`:

    TEST CASE:  pruning does not hide a forced mate
    FATAL ERROR: REQUIRE( result.mate_found ) is NOT correct!
      values: REQUIRE( false )
      logged: capture mate, 1r3r1k/2p1n1pp/8/p2n1p2/2BPp3/Q1B1P2q/1P3P1P/2R1R1K1 b - - 1 22, depth 11, red under C02 and R01

doctest's source-location prefix on the second line is elided, as S091's own
stamp elides it: it is a `path:line` and DEC-135 keeps those out of `adocs/`.

**Green, verbatim**, after `git checkout`-equivalent restore of the byte-for-byte
original and a rebuild: `100% tests passed, 0 tests failed out of 1`.

**Suite.** `cmake --build build -j12 && ctest --test-dir build -L fast
--output-on-failure` on the completed tree: **100% tests passed, 0 tests failed
out of 39**, total 121.33 s; `./clang-format.sh --check` exit 0 under
`CLANG_FORMAT_MAJOR=22`. The case itself is 48 assertions in 0.63 s, the new
row's depth-11 search 0.54 s of that. **`src/` is byte-identical to `80d1894`** -- `git diff --stat -- src/`
empty -- and `./build/src/chesso bench` totals **5950740 nodes**, the parent's.
`No functional change`.

**Docs.** `adocs/data/README.md` gains four rows: the script, the candidates
tsv, `S230_r01_sweep.tsv` and `S230_table_fens.txt`. **The last two exist
because the fast check found the evidence and the re-derivation both short of
runnable.** `S230_r01_sweep.tsv` is the step's actual claim -- all 281
positions with both depth profiles and a `separated_at` column applying
DEC-209 clause 4 -- where the candidates tsv carried only the shipped sweep; a
separate file rather than an `r01_depths` column, because 167 of the 281 are
not candidates and have no row there. `S230_table_fens.txt` holds the table's
four FENs in table order, so the GOLDEN block and the manual can print the
command that runs rather than a sentence describing it; run once as written it
answers `d7 d9 d10 d11 d12`, `d7 d9 d10 d11 d12`, `d9 d10 d11 d12` and
`d9 d10 d11 d12` at distances 5, 5, 4 and 5.
**`DEV_MANUAL.md` gained a row too, and it is outside the `touches` this step
started with** (the field was extended rather than the edit left unrecorded):
its DEC-142 golden table had no entry for `capture_mates`, because until this
step the depths and labels had no script to re-derive them with -- S222 ran the
derivation by hand. `cmd_depths` is that script, so the golden is now named at
its site in `tests/test_search.cpp` with the command beside it and listed in
the manual with the rest. `MANUAL.md` needs nothing: the change is a test and
touches no UCI surface. One older sentence was corrected rather than left
false -- "Run under R01, the whole fast suite fails there and nowhere else"
became "Run under R01 at S222, ...", because that is now a statement about
S222's tree and not this one.

## Why this exists

S222 (2026-09-14) moved the depth at which one S091 mate row separated its
mutants, and the table's own rule -- the depth is where the shipped build
reports the mate and the mutant does not -- removed the row rather than
re-pick it. R01's kill survives in S091's direct guard "a capture that gives
check is not reduced"; the mate table, the recurring-bug guard of this engine,
lost its second witness. Filler behind S229 (DEC-171): no reach in play.

## Cost

Agent work, an hour or two of mining and oracle checks; no run.
