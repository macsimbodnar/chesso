id:         S224
goal:       the seven sound files under `tests/assets/gui/sound/` whose origin nobody can state are deleted with their loader, so every file the repository bundles carries a licence `THIRD_PARTY.md` can quote (DEC-201)
accepts:    `tests/assets/gui/sound/` is gone (`click.wav`, `tick_1.wav` to `tick_5.wav`, `anime-wow-sound-effect.mp3`); `tests/debug_gui.cpp` no longer declares `sound_t`, `sound_fx`, `load_sound` or `play_sound`, and no call site remains; the GUI builds with `-DCHESSO_BUILD_GUI=ON` and runs against the display for five seconds without a missing-asset message; `tests/assets/gui/THIRD_PARTY.md` loses its "remaining gap" section and states that every file in the directory is listed with its licence; `books/fetch_book.sh`'s sentence about what the repository bundles no longer names the sounds as an exception; `grep -rn 'sound' tests/assets/gui/THIRD_PARTY.md books/fetch_book.sh tests/debug_gui.cpp` returns nothing; the fast suite is green in both builds and the commit touches no `src/`
touches:    tests/assets/gui/sound/, tests/assets/gui/THIRD_PARTY.md, tests/debug_gui.cpp, books/fetch_book.sh
excludes:   the fonts and the pieces, already licensed by S211; any other GUI change; anything under `src/`
decisions:  DEC-201
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-12 23:58 as filler before S151's match
done:

## Why this exists

S211 licensed the pieces, sourced or deleted the images, and inventoried the
directory; the seven sounds were the one thing left whose origin nobody can
state -- committed 2025-03-09 in `766465f` with no author or copyright
anywhere. DEC-201 deletes them rather than disclose a gap that has no rights
holder to disclose. Filler behind S151: tests only, no run, no `src/`.

## Cost

Agent work, under an hour. The GUI build is the proof; the fast suite does not
build the GUI, so the stamp states the GUI build and the five-second run.
