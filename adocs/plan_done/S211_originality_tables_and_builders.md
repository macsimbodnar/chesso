id:         S211
goal:       the last three tables and five mask builders inherited from a GPL-3.0 tutorial engine are replaced by the project's own derivations, the tutorial author's handle leaves the shipped source, and the twelve pieces of third-party artwork carry their licence -- so the code-level originality exposure the audit measured is zero
accepts:    `bishop_relevant_bits_count`, `rook_relevant_bits_count` and `castling_rights` in `src/bb_tables.hpp` are no longer literal tables: each is `constexpr`-derived from the engine's own geometry (the relevant-bit counts as `count_bits` of the attack masks the file already builds, the castling mask from the `WK`/`WQ`/`BK`/`BQ` bit assignment and the four rook and two king squares) or emitted by `tools/magic_gen` with a provenance comment the way S179 handled the magics, and `grep -c ' $' src/bb_tables.hpp` returns 0; `precompute_pawn_attacks`, `precompute_knight_attacks`, `precompute_king_attacks`, `precompute_bishop_attack_masks`, `precompute_rook_attack_masks`, `precompute_bishop_attacks`, `precompute_rook_attacks` and `set_occupancy` in `src/bitboard.cpp` are rewritten from the geometry -- one `(rank, file)` offset table for the leapers, a step list for the rays -- with no shared shift order, variable naming or comment lineage with the source the audit compared them against; `CMK_POS` is renamed for what the position is and `TRICKY_POS` for what the wiki calls it (Kiwipete), with the `position` shortcuts `cmk` and `tricky` in `src/chesso.cpp` `command_position` and `MANUAL.md`'s shortcut table changed together and `test_uci_surface` refreshed only after `MANUAL.md` (SURFACE); `tests/assets/gui/THIRD_PARTY.md` names Cburnett's Wikimedia Commons chess set, elects the BSD option of its quad licence and carries the notice, and `background_l.jpg`, `flip_icon.png` and `ChessPiecesArray.png` are either sourced there or deleted with `tests/debug_gui.cpp` adjusted; the false sentence in `books/fetch_book.sh` ("this repository bundles nothing whose licence is unstated") is made true by the above and reworded to say what the repository does bundle and under what; `CLAUDE.md`'s "hand-written and untuned" sentence about the piece-square tables is corrected to what `src/eval_tables.hpp` says they are -- fitted to chesso's own self-play; behaviour-neutral by construction and proved so: `bench` signature identical, `tools/search_bench.py` identical at depths 9 and 12, perft green over the nine columns, and `tests/test_movegen`'s attack-table cases green; the commit message carries `No functional change`
touches:    src/bb_tables.hpp, src/bitboard.cpp, src/data_structures.hpp, src/chesso.cpp, tools/magic_gen.cpp, tests/, tests/assets/gui/, books/fetch_book.sh, CLAUDE.md, MANUAL.md, adocs/specs.md
excludes:   the magic numbers and the Zobrist keys, already the project's own (S179, S203); `polyglot_randoms[781]`, format-defining specification by DEC-121; the FEN strings themselves, which are facts; any change to the attack tables' layout or to what `generate_moves` returns
decisions:  DEC-170
closes:     2026-09-10_adversarial-F01, 2026-09-10_adversarial-F02, 2026-09-10_adversarial-F03
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-12 19:28, machine idle
done:       2026-09-12 19:59. **Every value the generator produces is unchanged and every line that produced it is new.** `src/bb_tables.hpp` opens with twenty lines of board geometry -- `square_row`, `square_col`, `square_at`, `square_on_board`, a `step_t` of (dr, dc) and the two four-element step lists -- and `ray_attacks` walks one direction to the first blocker while `ray_relevant` keeps a square only when one more step stays on the board, which is the mask's definition rather than an edge subtraction. `bishop_relevant_bits_count` and `rook_relevant_bits_count` are now `relevant_bits_table(false)` and `relevant_bits_table(true)`, `std::popcount` of those masks at compile time, and `castling_rights` is `castling_rights_table()`, all four rights everywhere with `WK`/`WQ` cleared on `e1`/`h1`/`a1` and `BK`/`BQ` on `e8`/`h8`/`a8`. All 192 values were compared against the three tables that were deleted: **identical at every index**, checked by a throwaway program compiled against the header. `grep -c ' $' src/bb_tables.hpp` returns **0**, against 16 before, and both `// clang-format off` markers went with the tables. In `src/bitboard.cpp` the eight builders are rewritten: `KNIGHT_OFFSETS` and `KING_OFFSETS` as (dr, dc) lists walked by one `leaper_attacks` template, the pawn as one row toward its promotion rank and one column either way, the four slider entry points forwarding to the header's geometry so there is one definition of what a bishop reaches, and `set_occupancy` written as "the index-th subset of a mask's squares" with bit k pairing the k-th square. No shift, no `tr`/`tf`/`r`/`f`, no comment survives from the source the audit compared against, and that source was not opened. `TRICKY_POS` became `KIWIPETE_POS` and `CMK_POS` became `BLOCKED_CENTRE_POS`, named for placement checked with python-chess on 2026-09-12 -- the square ahead of each of d3, d4, e4 and e5 is occupied -- with the `position` shortcuts `tricky` -> `kiwipete` and `cmk` -> `blocked`, `MANUAL.md`'s table and its new explanatory sentence written before `test_uci_surface`'s golden token list was refreshed (SURFACE), and the handle gone from `src/search_params.hpp`'s S085 measurement line as well. **New tests, because the accepts asked for attack-table cases and `tests/test_movegen.cpp` had none**: seven in a `movegen: attack tables` suite, 348040 assertions, each a predicate over the answer rather than a second way of computing it, so none is a golden (DEC-142) -- a leaper reaches every legal offset and nothing else, a pawn attacks the two squares diagonally ahead, a blocker changes a slider's attacks exactly on its relevant mask, a full board leaves a slider its neighbours which is the king's move, `set_occupancy` enumerates every subset exactly once, the magic lookup agrees with a walked ray on all 64 squares by all 2^n occupancies, and every move that ends a castling right ends it through `make_move` over all six squares. Both bite: a planted wrong knight offset reddened one case, a planted `ray_relevant` that kept the last square of each ray reddened two. **Neutrality.** `bench` total **27322394**, the parent's to the node; `tools/search_bench.py` identical at depth 9 (midgame 121515 c3d5, kiwipete 801408 e2a6, tactical 72895 d7c8q) and depth 12 (638719 c3d5, 3514653 e2a6, 341715 d7c8q), node counts and best moves both; `test_perft` green in 55.6 s over all nine columns (nodes, captures, en passants, castles, promotions, checks, discovery checks, double checks, checkmates); `magic_gen magics --seed 20260904` still reports `coincide with the compiled-in arrays: 128 of 128`. The commit carries `No functional change`. **Debug self-play (DEC-141)**: 8 games at 4+0.04, 17 s, **0 `Assertion`, 0 `disconnect`**, no forfeits -- every game ended in a mate. **Gate**: 37/37 fast in `build` (85.0 s) and 37/37 in `build-tune` (85.0 s), `./clang-format.sh --check` clean under `CLANG_FORMAT_MAJOR=22`. **Assets**: `tests/assets/gui/THIRD_PARTY.md` inventories the whole directory -- Cburnett's twelve pieces under the BSD option of their quad licence with the notice quoted from the Commons file page (`{{self|GFDL|migration=relicense|BSD|GPL}}`, fetched 2026-09-12), both fonts with the licence files already beside them, and the seven sound files named as the one gap nobody can source. `background_l.jpg`, `flip_icon.png` and `ChessPiecesArray.png` were **deleted**: `git log --follow` reaches one 2025 commit each and their metadata names a tool and a date but never a rights holder. The third was read by nothing; the first two are replaced in `tests/debug_gui.cpp` by a flat background colour and a text "Flip" button, and the GUI was built with `-DCHESSO_BUILD_GUI=ON` and run for five seconds on a real display to prove it still works. `books/fetch_book.sh`'s false sentence is replaced by the list it should always have been. **Deviations, three.** `CLAUDE.md` is coordinator-owned so its "hand-written and untuned" sentence is proposed in the report, not edited here. `adocs/specs.md` needs no change: it names none of the renamed symbols and describes only the magics. `tools/magic_gen.cpp` was in `touches` and needed nothing -- deriving the tables removed the reason to emit them. **Second tier, DEC-141 clause 3, run by the coordinator before completion:** `tools/gate_extra.sh` on this tree, `GATE-EXTRA-DONE 5 stages 882 s` -- prose, citations, Debug 281 s, sanitize 546 s, deep perft 55 s -- log `.tuning/gate_extra_2026-09-12_S211.log`. The fast check re-derived all 192 table values, the eight builders over 64 squares and both sliders over every subset of their relevant masks against the pre-image: 0 mismatches; its one-line `DEV_MANUAL.md` correction (`magics` prints to stdout, nothing writes the header) was applied by the coordinator; the pawn builder's unreachable third colour value stays behind its precondition assert.

## Why this exists

`2026-09-10_adversarial` Part A: the first foundation, *nothing is copied*,
checked against the source the `bitboard` branch was learnt from.

**F01, high.** `src/bb_tables.hpp` carries three tables byte-identical to a
GPL-3.0 engine's, and the whitespace proves it: the same seven of eight rows
end in a trailing space in both files for both relevant-bit tables, and
`castling_rights` keeps the source's 4-space indentation inside a file
indented 2, preserved by `// clang-format off`. `git blame` puts all three on
the `bitboard` branch in May 2025, fifteen months before `achesso` branched;
no agent put them there. **But the same commit is the one S179 already acted
on**: at `9924d3b` the magics matched the same engine's 64 of 64 and 64 of 64,
and S179 regenerated them under the project's seed because inheritance is not
a defence (DEC-132). It replaced one artefact from a commit that contains four
and left three, twenty lines apart.

**F02, medium, with the audit's own caveat.** Five mask builders are
line-for-line transliterations -- the same eight knight shifts in the same
order with the same guards, the ray loops with the same `tr`/`tf`/`r`/`f`
names -- and the eight knight shifts are also the standard idiom on the wiki's
*Knight Pattern* page, so the finding is weak alone and strong twenty lines
from F01. `CMK_POS` is the source author's handle in this project's shipped
header, unexplained anywhere in the repository.

**F03, medium.** Twelve PNGs under `tests/assets/gui/` are Cburnett's
Wikimedia Commons set (98.2 % silhouette match, Inkscape `tEXtSoftware`),
quad-licensed GFDL / CC-BY-SA / BSD / GPL, every option requiring the notice
be retained, and the directory carries only the font's OFL. Three more assets
have no stated origin. `books/fetch_book.sh` asserts the repository bundles
nothing whose licence is unstated.

The legal exposure is thin -- the values are facts and the expression is de
minimis -- and the project's exposure is not legal: it is the accusation that
the work was copied, and these are the cheapest proofs anyone looking would
find. The owner's whole reason for the no-copy rule is that accusation
(DEC-104, DEC-105). So the fix is worth a step even at zero Elo, and it is
scheduled in the first daytime after the audit rather than at the end.

## What Part A found clean, so it is not re-done here

The magics reproduce from `magic_gen magics --seed 20260904` as an ordered
list, 0 of 128 shared with the source. The Zobrist keys are drawn at run time.
The piece-square tables have 0 of 64 exact matches with PeSTO and the wiki's
simplified function on every non-pawn table. The book's provenance is
reproducible end to end. Search, evaluation, UCI, the legal generator, `see_ge`
and `attackers_to` have 0.00 % 12-token shingle overlap with the source. 38 %
of `src/` is unchanged from the branch point, concentrated where F01 and F02
sit.

## Shape

- **Tables.** `bishop_relevant_bits_count[sq]` is `count_bits(
  precompute_bishop_attack_masks(sq))` and the rook one likewise; both are
  computable at compile time once the mask builders are `constexpr`, or
  emitted by `tools/magic_gen` with a comment in the S179 form. `castling_rights`
  is the mask of rights that survive a move touching square `sq`: all four
  everywhere except `a1`/`e1`/`h1` and `a8`/`e8`/`h8`, derived from the four
  `WK`/`WQ`/`BK`/`BQ` bits in code rather than typed.
- **Builders.** Leapers from an offset table `{(dr, df)}` with the on-board
  test done in rank-and-file arithmetic; rays as four (bishop) or four (rook)
  `(dr, df)` steps walked to the edge or the first blocker. `set_occupancy`
  from the wiki's definition -- the i-th subset of a mask's bits -- in the
  project's own words.
- **Names.** `CMK_POS` becomes a name for the position (a Ruy Lopez
  middlegame; name it by content, not by author) and the `cmk` shortcut goes
  with it; `TRICKY_POS` becomes `KIWIPETE_POS` and `tricky` becomes
  `kiwipete`. Every `bench` and `test` list that names them follows.
- **Assets.** `THIRD_PARTY.md` in the directory; the BSD election; the three
  unsourced files deleted unless their origin is found, with
  `tests/debug_gui.cpp` adjusted.

## Measurement

Behaviour-neutral by construction and proved on node counts: the attack tables
are values and a rewrite that produces the same values changes nothing the
generator returns. The `bench` signature and `tools/search_bench.py` are the
gate; `test_perft` over nine columns is the second. No timing is owed -- the
builders run once at start-up.

## Cost

Agent work, two to three hours; no run. DEC-140: the commit touches `src/`,
its total is the parent's, and the message says `No functional change`.
