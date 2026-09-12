id:         S110
goal:       a second correction table keyed on the non-pawn structure, split by colour
accepts:    an SPRT verdict, recorded whatever it is; the key is maintained incrementally in add_piece, remove_piece and move_piece and is never recomputed in evaluate() -- INV-4, and the 25 % of nps S014 removed is what a recomputation puts back; the corrected score can never cross into the mate or decisive band, and a test asserts it; the table is a constant-sized array with its dimensions and its clamp stated in src/search_params.hpp
touches:    src/search.cpp, src/bitboard.cpp piece primitives, src/data_structures.hpp, src/search_params.hpp
excludes:   the pawn table, which is S099 and lands first; continuation correction, which is S111
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## Reserve, 2026-08-19, DEC-087

Demoted behind the 3000 push by the second review: the non-pawn and
continuation tables measure +3 to +8 and only above ~3100 in the surveyed
record -- **the band is unverified** and the 2026-09-04 literature check says
so in as many words: one fetched point lands inside it, Sirius pull request
#176, minor-piece and king correction history, **+7.43 +/- 4.78** at 8+0.08
(https://github.com/mcthouacbb/Sirius/pull/176), and *nothing* fetched
supports "only above ~3100"; Starzix pull request #98, "Pieces correction
history", publishes no Elo figure at all
(https://github.com/zzzzz151/Starzix/pull/98), and CPW's page states none
(https://www.chessprogramming.org/Static_Evaluation_Correction_History, which
does say gains grow with the time control). **Searched again 2026-09-13**,
after S186's fast check: the same page, an open web search for the band and
for the rating claim, and the engine changelogs that search turns up -- no
source for "+3 to +8" and none for "only above ~3100"; the one ledger that
prices these tables separately (below) carries no rating band at all. The
band stays unverified and the order does not rest on it. The pawn table (S099, which stays
in the main order) has its figure sourced: **+11.35 +/- 5.16** at 8+0.08 over
7502 games, bounds [0, 3], Lynx pull request #1662, merged 2025-04-15
(https://github.com/lynx-chess/Lynx/pull/1662) -- the "+11.4" this file quoted
rounded. **The strength it was measured at is not sub-3000**: S181 banded that
merge at **3224 to 3291** CCRL Blitz 1CPU -- between v1.9.1, which the list
does not carry, and v1.10.0, with v1.9.0 the nearest rated release before it,
on the list computed 2026-09-05 (`adocs/data/S181_lynx_bands.md`). So the one
figure DEC-087 (b) called sub-3000 was measured **above** the ~3100 that
demoted this step and S111, and the criterion that separated them separates
nothing. DEC-133 is the owner's answer -- S099 to the head of the reserve as
the family's probe, this step and S111 gated on its verdict -- and DEC-176
records that the re-banding changes the record and not the order. The long-control doubling noted below is one more
reason this family reads better after S152, which absorbed S128's question
(DEC-108), moves the measurement nearer the list's control.

## Order, and why it is three steps

The published record measures these separately and they are **not** inert
apart, so DEC-082 does not apply and they do not become one step. Reported, all
short then long time control: pawn +11.29 / +12.40 and +4.87 / +11.70,
non-pawn +6.98 / +12.28 and +2.80 / +6.84, continuation +2.58 / +5.46 and
+2.75 / +5.46.

**Every one of those twelve numbers is unverified** and has been since it was
written down: the 2026-09-04 literature check looked for their source and found
none (row A22), and a third search on 2026-09-13, after S186's fast check,
found none either -- an open web search for the numerals themselves against
"correction history", the CPW page below, and the engine changelogs it leads
to. They are kept because the *shape* they carry -- separate effects, long
control about twice short -- is what the step's argument uses. Do not price a
run on them.

**The shape has a source even though the twelve numerals do not** (2026-09-13,
after the fast check). Tcheran's `CHANGELOG.md`
(https://raw.githubusercontent.com/tcheran-chess/tcheran/master/CHANGELOG.md,
changelog entries only, DEC-016) prices four correction-history tables as four
separate patches in one engine, release 10.0: "Add pawn eval correction
history (21.76 +- 8.62)", "Add major and minor piece eval correction history
(18.86 +- 7.99)", "Add non-pawn eval correction history (12.69 +- 6.30)", "Add
threat eval correction history (9.30 +- 5.31)". Four tables, four positive
verdicts, each measured on top of the ones before it -- which is exactly the
"not inert apart" claim this section makes, now with a URL behind it, and the
non-pawn entry is the class this step is. **The changelog states one figure per
line and no time control for these entries**, so nothing here reads on the
short-versus-long doubling; that half of the shape still rests on CPW's
statement that the family's gains grow with the time control and on S099's
sourced +11.35. One engine's ledger is direction, not a prediction for this
one (DEC-019).

Two things follow. **The long-control figure is consistently about twice the
short one**, so `--fast` at 8+0.08 will under-measure this family by half --
say so in the verdict rather than reading the point estimate as the effect.
And the whole family is reported to be worth *more* for a hand-crafted
evaluation than for a network, because the residual there is to correct is
larger. That is the one place on this plan where DEC-054's no-NNUE constraint
is an advantage rather than a cost.
