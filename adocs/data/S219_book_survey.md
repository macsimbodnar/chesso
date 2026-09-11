# S219 -- the open-licence opening books, surveyed 2026-09-11

Written by the coordinator from a research agent's report of 2026-09-11, for
DEC-182 and S219. Every claim carries the URL it was read at; a claim the
agent could not confirm is in the last section as `unverified`. No engine
source was read (DEC-016). Nothing here decides the book: the pick is
measured by S219, and this file says only what the candidates are and what
licence each is under.

## The one rule that survived contact

**Every book whose licence could be verified comes from one place**:
`official-stockfish/books`, licence CC0-1.0
(<https://raw.githubusercontent.com/official-stockfish/books/master/LICENSE>).
Both books this harness has ever played were fetched from it
(`books/fetch_book.sh`). Stefan Pohl's own pages state "(C) 2024 Stefan Pohl"
and no usage licence (<https://www.sp-cc.de/uho_2024.htm>,
<https://www.sp-cc.de/downloads--links.htm>), and the OpenBench book
repository carries no licence file
(<https://github.com/AndyGrant/openbench-books>). Under this project's rule --
nothing bundled whose licence is unstated -- both stay out, whatever their
quality. The Pohl-derived books redistributed **by** the CC0 repository are
fine to fetch from there, which is what the harness already does.

## Candidates, all CC0, all in `official-stockfish/books`

Download pattern: `https://github.com/official-stockfish/books/raw/master/<name>.zip`.
Counts from <https://raw.githubusercontent.com/official-stockfish/books/master/books.json>;
the file list from <https://api.github.com/repos/official-stockfish/books/contents/>.

| book | type | positions | who tests with it | note |
|---|---|---|---|---|
| `UHO_Lichess_4852_v1.epd` | unbalanced -- Lichess 2023 human positions at 2 to 16 plies, filtered by Stockfish to a 48-52 % draw rate | 2632036 | fishtest's default at STC and LTC since 2023-10; OpenBench | **what the harness plays now** |
| `UHO_4060_v4.epd` | unbalanced -- Pohl's UHO lines rescored by Stockfish, 40-60 % win rate kept | 241670 | fishtest's progression-test book 2025-04 to 2026-02 | candidate, unbalanced |
| `popularpos_lichess_v3.epd` | balanced-leaning -- the most frequent positions of Lichess games above 1800, crossed with fishtest LTC | 200000 | a fishtest option | **candidate, balanced** |
| `noob_3moves.epd` | balanced, shallow (6 plies) | 150932 | fishtest's default 2020-01 to 2021-08 | candidate, balanced |
| `UHO_XXL_+0.90_+1.19.pgn` | unbalanced, Pohl, KomodoDragon-evaluated | 223070 | fishtest's default 2021-08 to 2023-10 | superseded by the two UHO rows above |
| `8moves_v3.pgn` | balanced | 34700 | fishtest's default 2013 to 2020; OpenBench | **in `books/` already**, `rating.sh` plays it; too small for a long SPRT (below) |
| `Drawkiller_balanced_big.epd` | balanced evaluation, kings on opposite corners | 15962 | a 2021 book test only | too small |
| `noob_4moves.epd`, `2moves_v2.pgn`, `2moves_v1.epd` | balanced, very shallow | 1837365 / 12092 / 40457 | old fishtest defaults | not carried forward |

Construction records: `UHO_Lichess_4852_v1`
<https://github.com/official-stockfish/books/commit/426eca422c202ef381a1380ada1873a7f185c4b9>
("15B lichess games ... Jan-Sept 2023", "draw rate in the range 48-52 %");
`UHO_4060`
<https://github.com/official-stockfish/books/commit/994e158338acf4942fa5f7edc7093491c8339d88>;
`popularpos`
<https://github.com/official-stockfish/books/commit/4bef1f8baef306ed0410e9391c941cf2d6ed2872>.
fishtest warns that a book under about 100 k exits repeats openings inside a
long run (<https://github.com/official-stockfish/fishtest/issues/2298>), which
rules `8moves_v3` and `Drawkiller_balanced_big` out as the harness book and
leaves four.

## What fishtest does, and why

- Default `test_book = "UHO_Lichess_4852_v1.epd"` at STC and LTC
  (<https://raw.githubusercontent.com/official-stockfish/fishtest/master/server/fishtest/views.py>),
  adopted in PR #1837 on 2023-10-22 for "a favorable draw rate of about
  50 %" with the same Elo detection as the book before it
  (<https://github.com/official-stockfish/fishtest/pull/1837>); the
  maintainers' stated rule: "the only property that matters for a book is
  the draw ratio", optimum about 50 %.
- The fastchess line fishtest uses:
  `-openings file=UHO_Lichess_4852_v1.epd format=epd order=random plies=16`
  (<https://official-stockfish.github.io/docs/fishtest-wiki/Running-Fastchess.html>).
- **How fishtest compares books**, from its FAQ
  (<https://github.com/official-stockfish/fishtest/wiki/Fishtest-faq>): one
  binary against itself at about 30 % time odds, a fixed number of games per
  book, read as normalized Elo -- never an SPRT and never master against an
  old version. The book that resolves the known handicap with the highest
  normalized Elo is the more sensitive one. **This is the method S219
  uses.**

## Balanced against unbalanced at 2500 to 3000

- Stockfish's 2021 book measurements at LTC, normalized Elo for a fixed
  handicap, draw rate in brackets
  (<https://github.com/official-stockfish/Stockfish/issues/3323>):
  `Drawkiller_balanced_big` 128.6 (0.63), `UHO_XXL_+0.80_+1.09` 119.0
  (0.55), `UHO_XXL_+0.90_+1.19` 118.0 (0.49), `2moves_v1` 110.5 (0.76),
  `noob_3moves` 96.9 (0.83), `8moves_v3` 87.6 (0.83). The books drawing 83 %
  were worst and a **balanced** book at 63 % draws was best: the draw ratio
  drives sensitivity, not balance as such.
- Pohl's own target is 45 to 60 % draws; below 45 % "a lot of 1:1 pairs ...
  shrink the Elo spreading" (<https://www.sp-cc.de/uho_2024.htm>,
  <https://talkchess.com/viewtopic.php?t=83379>).
- The wiki's strength-dependent advice: balanced books such as `8moves_v3`
  for weaker engines, biased books such as `UHO_Lichess_4852_v1` and `Pohl`
  for stronger ones
  (<https://www.chessprogramming.org/Sequential_Probability_Ratio_Test>).
- **What that says about chesso.** S105 measured 40.3 % draws on the
  *balanced* book and fewer on the unbalanced one, with 1:1 pairs rising
  from 41.6 % to 46.8 % -- exactly Pohl's warning. The engine sits under
  every quoted draw floor on every book it has played, so the published
  guidance points **toward** balance, not away from it, and the owner's
  leaning agrees. Whether that is worth more than the x1.20 in games an hour
  the unbalanced book buys is the number S219 measures.

## A book of the project's own, later

The Lichess database is CC0 -- "You can download, modify and redistribute
them, without asking for permission" (<https://database.lichess.org/>) -- and
both `UHO_Lichess_4852_v1` and `popularpos_lichess` were compiled from it, so
a book compiled here from that source is viable, with `zstd`, `pgn-extract`
and python-chess as the tooling. **One question is open before that is
built**: both Stockfish books were *selected by Stockfish's evaluation*.
Running another engine's binary as a tool creates no derivative work
(DEC-016) and a harness book is neither engine code nor training data, but a
book whose exits were chosen by another engine's evaluation is a new case
and gets a decision before it is compiled, not after. Not part of S219.

## Unverified

- Pohl's consent to the CC0 redistribution of the UHO and Drawkiller books
  in `official-stockfish/books`: not documented anywhere the agent found. The
  repository's licence is what this project relies on.
- Any readme or licence inside Pohl's `.7z` downloads: not downloaded.
- The authorship of the `noob` books (attributed to noobpwnftw): the commit
  is by another author; unconfirmed.
- OpenBench's default book: none named in its documentation.
- The 2024 objection that about a quarter of `UHO_Lichess` exits are too
  imbalanced (<https://github.com/official-stockfish/Stockfish/discussions/5079>):
  one user's numbers, not reproduced; the maintainers saw no need to change.
- The FAQ's "unbalanced books are definitely better than balanced books"
  carries no linked data, and the 2021 measurements above partly contradict it.
- CCRL and TCEC publish no book under a licence this project could use
  (<https://ccrl.chessdom.com/ccrl/4040/about.html>).
