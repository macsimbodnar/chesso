id:         S211
goal:       the last three tables and five mask builders inherited from a GPL-3.0 tutorial engine are replaced by the project's own derivations, the tutorial author's handle leaves the shipped source, and the twelve pieces of third-party artwork carry their licence -- so the code-level originality exposure the audit measured is zero
accepts:    `bishop_relevant_bits_count`, `rook_relevant_bits_count` and `castling_rights` in `src/bb_tables.hpp` are no longer literal tables: each is `constexpr`-derived from the engine's own geometry (the relevant-bit counts as `count_bits` of the attack masks the file already builds, the castling mask from the `WK`/`WQ`/`BK`/`BQ` bit assignment and the four rook and two king squares) or emitted by `tools/magic_gen` with a provenance comment the way S179 handled the magics, and `grep -c ' $' src/bb_tables.hpp` returns 0; `precompute_pawn_attacks`, `precompute_knight_attacks`, `precompute_king_attacks`, `precompute_bishop_attack_masks`, `precompute_rook_attack_masks`, `precompute_bishop_attacks`, `precompute_rook_attacks` and `set_occupancy` in `src/bitboard.cpp` are rewritten from the geometry -- one `(rank, file)` offset table for the leapers, a step list for the rays -- with no shared shift order, variable naming or comment lineage with the source the audit compared them against; `CMK_POS` is renamed for what the position is and `TRICKY_POS` for what the wiki calls it (Kiwipete), with the `position` shortcuts `cmk` and `tricky` in `src/chesso.cpp` `command_position` and `MANUAL.md`'s shortcut table changed together and `test_uci_surface` refreshed only after `MANUAL.md` (SURFACE); `tests/assets/gui/THIRD_PARTY.md` names Cburnett's Wikimedia Commons chess set, elects the BSD option of its quad licence and carries the notice, and `background_l.jpg`, `flip_icon.png` and `ChessPiecesArray.png` are either sourced there or deleted with `tests/debug_gui.cpp` adjusted; the false sentence in `books/fetch_book.sh` ("this repository bundles nothing whose licence is unstated") is made true by the above and reworded to say what the repository does bundle and under what; `CLAUDE.md`'s "hand-written and untuned" sentence about the piece-square tables is corrected to what `src/eval_tables.hpp` says they are -- fitted to chesso's own self-play; behaviour-neutral by construction and proved so: `bench` signature identical, `tools/search_bench.py` identical at depths 9 and 12, perft green over the nine columns, and `tests/test_movegen`'s attack-table cases green; the commit message carries `No functional change`
touches:    src/bb_tables.hpp, src/bitboard.cpp, src/data_structures.hpp, src/chesso.cpp, tools/magic_gen.cpp, tests/, tests/assets/gui/, books/fetch_book.sh, CLAUDE.md, MANUAL.md, adocs/specs.md
excludes:   the magic numbers and the Zobrist keys, already the project's own (S179, S203); `polyglot_randoms[781]`, format-defining specification by DEC-121; the FEN strings themselves, which are facts; any change to the attack tables' layout or to what `generate_moves` returns
decisions:  DEC-170
closes:     2026-09-10_adversarial-F01, 2026-09-10_adversarial-F02, 2026-09-10_adversarial-F03
blocks:
paused_by:
author:
done:

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
