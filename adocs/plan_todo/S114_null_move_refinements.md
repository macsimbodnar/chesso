id:         S114
goal:       the null move reduction scales with how far the static score is above beta, and the base reduction is re-decided
accepts:    an SPRT verdict, recorded whatever it is; **the static-score term is capped** -- uncapped, a position twenty pawns ahead reduces to depth 0 and re-creates the mate-hiding bug this engine has already shipped twice; the existing zugzwang guard on game_phase is kept, not replaced; the mate cases in the fast suite pass, and the cap is observed to be load-bearing by removing it and watching one go red; the base and divisor seeds are stated with their source and everything ships from our own sweep and S127 (DEC-084)
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   a null-move verification search -- dropped by DEC-087, no evidence below 3000 and the game_phase guard already covers zugzwang; the reduction formula's final values, which S127 fits
decisions:  DEC-071, DEC-084, DEC-087
closes:
blocks:
paused_by:
done:

## Re-scoped 2026-08-19 by the second review, DEC-087

Two halves, one kept and one dropped. **Kept:** the eval-scaled reduction --
`R = base + depth/divisor + min((eval - beta)/scale, cap)` is the surveyed
form, and a deeper base R alone measured **+12.3** at Berserk, with the
eval-term cap worth a further +3.3/+2.3 at Ethereal. The static evaluation
input exists since S108. **Dropped:** the verification search. No engine in
this band shows a gain for it -- it is a Stockfish-depth device -- and this
engine's zugzwang exposure is already guarded by `game_phase(&game->board) > 0`,
which stays.
