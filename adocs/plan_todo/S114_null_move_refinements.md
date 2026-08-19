id:         S114
goal:       the null move reduction scales with how far the static score is above beta, and a verification search guards the deep case
accepts:    an SPRT verdict, recorded whatever it is; **the static-score term is capped** -- uncapped, a position twenty pawns ahead reduces to depth 0 and re-creates the mate-hiding bug this engine has already shipped twice; the verification search carries a minimum-ply guard so it cannot itself null-move at the same ply; the existing zugzwang guard on game_phase is kept, not replaced; the mate cases in the fast suite pass, and the cap is observed to be load-bearing by removing it and watching one go red
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   the reduction formula's base and divisor, which S127 fits
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:
