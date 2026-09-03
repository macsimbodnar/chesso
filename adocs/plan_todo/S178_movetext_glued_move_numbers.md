id:         S178
goal:       `movetext_to_san()` splits a move number glued to its move, so PGN import format (`1.e4 e5 2.Nf3`, `1...e5`) parses instead of cutting the game short at ply 0
accepts:    a token of digits, one or more dots, then a move (`1.e4`, `12...Nf6`, `3.O-O`) yields the move alone; a token of digits and dots only is still dropped as a move number and a bare number is still dropped; a fixture PGN in `tests/test_make_book_tools.sh` written in the glued form builds byte-identically to the same game written as `1. e4 e5 2. Nf3 Nc6 *`, observed red first (today: `game 1 cut short: cannot parse '1.e4' at ply 0`, exit 1, no file) and green after; the shipped `src/openings.bin` rebuilt from `books/8moves_v3.pgn` is byte-identical to the committed file, since that PGN has no glued token (`grep -c -E '(^|[[:space:]])[0-9]+\.[a-hNBRQKO]'` is 0); `tools/search_bench.py` node counts and best moves identical (INV-6, no search path); `DEV_MANUAL.md`'s make_book section says the import form is accepted
touches:    tools/make_book.cpp, tests/test_make_book_tools.sh, DEV_MANUAL.md
excludes:   `pgn_to_positions`, which reads whitespace-separated SAN and documents "no move numbers"; any other PGN import-format leniency (e.g. `e8Q` without `=`, `0-0` with zeros, `P` prefixes) -- each is its own small decision; the parser `algebraic_to_move()` itself
decisions:
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found while doing S174, 2026-09-04, by running the fixed tools on the PGN
import form. `movetext_to_san()` (`tools/make_book.cpp:72-140`) drops a token
that is digits followed only by dots, and passes everything else to the parser,
so `1.e4` reaches `algebraic_to_move()` whole. Before S174 the parser
fabricated a move from it and the game was built from a rewritten board; since
S174 it returns 0 and the game is cut short at ply 0, the build is refused and
the message names the token:

```
game 1 cut short: cannot parse '1.e4' at ply 0
1 game(s) cut short -- book not written. ...
exit=1
```

Correct, and useless for the half of the world's PGN files that write the
import form -- the standard's export format is `1. e4`, its import format
allows `1.e4`, and tools emit both. The shipped PGN is all export form (the
grep above is 0), so nothing shipped is affected and this does not jump the
queue: the tool refuses honestly today, it does not lie. Filed as its own step
rather than folded into S174 because S174's goal is that the tool fails closed,
and this is about what it accepts.
