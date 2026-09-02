id:         S146
goal:       the 5.2 MB opening book compiled into the shipped binary has a recorded origin and licence, or it is replaced by one that does
accepts:    `src/openings.book`'s origin, author and licence are established and written down where a reader will find them, or -- if they cannot be established -- the file is replaced by a book this project can account for and the replacement's provenance is recorded the way `books/fetch_book.sh` records the match books, with both digests and the licence named; whichever way it goes is a recorded decision, because this is the owner's call and not an agent's; the engine's book behaviour is unchanged or its change is measured, since `Use Book` defaults false and a book swap only alters play when it is switched on; `MANUAL.md` documents what the shipped book is; the `polyglot_randoms[781]` table (`src/openings.cpp:44`) gets the same owner ruling -- either format-defining constants are the published spec rather than a copied table, recorded as a decision and cited at the table, or the Polyglot path goes with the blob (2026-08-22_adversarial-F06)
touches:    src/openings.book, src/openings.cpp, MANUAL.md, adocs/decisions.md
excludes:   the match books under `books/`, which `books/fetch_book.sh` already pins with licences; making the engine read a book from disk instead of from the binary, which is a feature and would need its own step and verdict
decisions:  DEC-016
closes:     2026-08-22_adversarial-F06
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-02
done:

## What was found

`src/openings.book` is **5220541 bytes, tracked, and compiled into the shipped
binary**: it is a C header -- `#pragma once`, then one `#define BOOK "…hex…"` --
included at `src/openings.cpp:5`. It decodes to roughly 163000 sixteen-byte
Polyglot entries. It entered on the `bitboard` branch and was inherited here.

**No document in the repository records where it came from.** `grep` over
`README.md`, `MANUAL.md`, `DEV_MANUAL.md`, `CLAUDE.md` and `TOOLCHAIN.md` finds
nothing; the only mention anywhere is one line in
`adocs/audit/2026-08-13_plan_review.md` noting the file exists. Found while
researching position-set licences for S145.

## Why this is not housekeeping

`CLAUDE.md`'s first foundation is that nothing is copied, and it gives a second
reason beside the licence one: "the owner wants no GPL question anywhere in this
codebase or in a future network". This is a five-megabyte third-party-shaped blob
**linked into the binary that ships**, and circulating Polyglot books are largely
of unknown or GPL-adjacent origin. It is the one artefact in the tree with no
story, and the project's rule is stricter than the law precisely so that this
case does not arise.

The same research pass priced the licence landscape for position data and found
that the honest answer for almost every circulating collection is "no licence
file, and a README that asserts a status it has no standing to assert". A book is
the same class of artefact.

## Amended 2026-08-22

The audit found the step's own excludes left a second copied artifact out of
scope: `excludes` said "book *format* work", and the `polyglot_randoms[781]`
constant table at `src/openings.cpp:44` — verbatim, necessarily, GPL-origin
via the PolyGlot adapter — is format work. So the 5.2 MB blob had a pending
provenance step and the 781-constant table beside it had none
(2026-08-22_adversarial-F06). The exclude is dropped and the table joins the
accepts: same artifact family, same owner ruling, one step. There is a
defensible reading — format-defining constants are the published spec itself,
reproduced by the wiki, unavoidable for interoperability — but the rule says
tables are copied never, not rarely, so the reading is the owner's to record,
not an agent's to assume.

## What makes this cheap

`Use Book` defaults to **false**, so the book does not affect a single SPRT
verdict taken so far and replacing it alters no measurement. The cost is
research and a decision, not machine time -- which is why it can sit in
`plan_todo/` without blocking anything, and why it should not sit there
indefinitely.

## Cost

No match, no verdict. Research, a decision, and a documentation change.

## Identifiers, so the origin can be searched for rather than guessed (S158, 2026-08-30)

Computed from `src/openings.book` as it is tracked today, not carried from
another document. The blob is a C header holding one hex string; every figure
below is of the **decoded** book, which is what a published book would be
compared against.

| what | value |
|---|---|
| decoded size | **2610256 bytes** |
| entries | **163141**, sixteen bytes each, no remainder |
| sha256 of the decoded book | `47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5` |
| first entry | key `00000883b144421f`, move `0dae`, weight 45, learn 0 |
| sha256 of `src/openings.book` as tracked | `6fd66d8d86cce8e0c639f047d0d6d9cd42a06ca9ba3f91d01601c6ad7f84e026` |
| source file size | 5220541 bytes (5220512 hex characters plus the 29-byte `#pragma once` / `#define BOOK` wrapper) |
| introduced by | `349f8cf`, 2025-05-12, "Add openings book", on `bitboard`. The message is one line and says nothing about origin |

**Reproducing the digest.** There is no `sha256sum` on the macOS machine
(`.moltke.local.md`), so the pipeline uses `shasum -a 256`; the python form
needs neither and also prints the counts:

```sh
sed -n 's/.*#define BOOK "\([0-9a-f]*\)".*/\1/p' src/openings.book \
  | tr -d '\n' | xxd -r -p | shasum -a 256

python3 -c 'import re,hashlib;h=re.search(r"\"([0-9a-f]*)\"",open("src/openings.book").read(),re.S).group(1);print(len(h)//2,"bytes",len(h)//32,"entries",hashlib.sha256(bytes.fromhex(h)).hexdigest())'
```

**The entry count is confirmed at runtime and not only by arithmetic.**
`load_book_embedded` (`src/openings.cpp:358`) sets `num_of_positions` to
`book.size() / sizeof(polyglot_entry_t)`, and the debug build prints it:
`printf 'uci\nposition startpos\ngo depth 1\nquit\n' | ./build-debug/src/chesso`
answers **"Opening book loaded correctly! 163141 entries."**. The shipping
Release build cannot be asked -- `src/log.hpp:29` compiles `LOG_I` to
`if (false) std::clog` under `NDEBUG` -- so the observation needs the debug
binary.

**Why this is a licence exposure and not a measurement contaminant.**
`Use Book` is advertised as `default false` at `src/chesso.cpp:957`, and the
state it advertises is real: `opening_book_enabled` is initialised `false` at
`src/chesso.cpp:31` and moves only in the `setoption` handler at
`src/chesso.cpp:1044-1052`. The book is loaded regardless -- and so is linked
into every shipped binary -- but `search_random_move_in_book`
(`src/chesso.cpp:606`) requires `opening_book_enabled`, so no SPRT taken so far
has played a book move. Replacing the blob therefore costs no verdict and
invalidates none.

163141 entries and that decoded digest are what to search a candidate book on:
a published book either matches the digest exactly or it is not this file, and
the entry count alone narrows the field before any download.

## The search for the blob's origin, and what it returned (2026-09-02)

Recorded because a negative result is the finding here, and because the next
person to look should not repeat it.

**Inside the repository the trail ends at the first commit.** `--follow` over
the file reaches `628d827`, 2025-04-16, "Add book openings", where it is
`src/book.hpp` and already a `#define BOOK "…hex…"` header. S158 named
`349f8cf` (2025-05-12) as the introducing commit; that is the `bitboard`
branch's copy, and `a809f0f` renamed `src/book.hpp` to `src/openings.book`
along the way. The decoded sha256 at `628d827` is
`47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5` -- the same
digest S158 recorded for HEAD, so the bytes have never changed. The commit
message is one line and the diff carries no URL, no attribution and no licence
text; the only external reference anywhere in it is the format description
(`src/openings.cpp:15-18`), which describes the format and not this book.

**Outside it, four searches, no match.**

| what was searched | result |
|---|---|
| the decoded sha256, on the open web | nothing; no index carries it |
| GitHub code search for the hex prefix `00000883b144421f0dae002d`, and for `"#define BOOK"` | no chess result |
| `michaeldv/donna_opening_books` by file size (gm2001 486656, komodo 9250016, rodent 2805680) | none is 2610256 |
| `ChrisWhittington/polyglot-books`, `sakya/corechess`, `ulthiel/polyglot` trees | no `.bin` of that size |

**The blob's own fingerprint, computed here, for whoever searches next.**
163141 entries over **154916 distinct positions**, of which **148321 carry
exactly one move** -- a narrow book, not a multi-choice one. The start position
offers three moves, `d2d4`, `g1f3` and `e2e4`, at weight 54 each. Weights run
1 to 305, sum 8980569, mean 55.05, and their histogram is bell-shaped around
50 rather than flat or frequency-like -- so the weight is not a game count.
Every `learn` field is zero. The entries are sorted by key, as the format
requires.

**What it costs while it stays.** `build/src/chesso` is 5529736 bytes and
`strings -n 64` finds one 5220512-byte literal in it: the hex text of the book
is **94.4 %** of the shipped binary, and it is the hex text rather than the
2610256-byte book, because `load_book_embedded` (`src/openings.cpp:358`)
decodes at startup rather than at compile time.

## The table half is settled: DEC-121 (2026-09-02)

The owner's ruling on `polyglot_randoms[781]`: **format-defining specification,
kept, cited at the table**. All 781 constants were extracted from the live
format description at `https://hgm.nubati.net/book_format.html` (page sha256
`bd95784721dbc0bfd4d734e87281b87f91e917886dc3b347f38f6e6bda5eb31b`) and
compared element by element against the array -- equal at every index, in
order. That page's own note on copyright is what makes the ruling available:
the algorithm "may be freely implemented by all GUIs, adapters and engines,
including closed source ones", "Polyglot itself is GPL but the GPL only covers
actual code and not algorithms", and "a table of random numbers cannot be
covered by copyright".

The citation and that note now sit above the array in `src/openings.cpp`, so
the ruling is where the constants are and not only in `decisions.md` -- which
is what 2026-08-22_adversarial-F06 asked for. The cited URL was corrected from
`http` to `https` in the same edit; the site redirects.

DEC-121 states the exception narrowly on purpose: constants that **define an
interchange format**, published in that format's own specification, where a
different value produces a non-interoperable result. It does not reach a tuned
table, whatever republishes it -- DEC-084 as amended by DEC-105 is untouched.

## The blob half is with the owner (2026-09-02)

Asked and answered on 2026-09-02: of the four options put -- name the source,
delete the book and the `Use Book` option, build a replacement from a
permissively-licensed source, or keep it with provenance recorded as unknown --
the owner took **the first: he names the source**. The name and its licence are
his to supply; until they are here, the step cannot write them into `MANUAL.md`
and cannot close.

## The owner's first naming, and why it does not fit (2026-09-02)

The owner named `books/8moves_v3.pgn`, from `https://github.com/official-stockfish/books`, as the source of the header blob. **It is not**, and that is measured rather than argued.

| check, over `books/8moves_v3.pgn` as committed | the PGN | `src/openings.book` |
|---|---|---|
| distinct first moves | 13 (`e4` 12956, `d4` 12493, `c4` 4192, `Nf3` 4128, `g3` 579, `b3` 183, `f4` 107, `b4` 19, `Nc3` 15, `e3` 13, `a3` 7, `d3` 6, `c3` 2) | **3**: `d2d4`, `g1f3`, `e2e4`, weight 54 each |
| positions of 300 of its games present in the book, ply 2 | all, by construction | 261 of 300 |
| same at ply 15 | all | 25 of 300 |
| the game's own move present in the book, ply 0 / ply 15 | all | 261 / 300, then 9 / 300 |

A book compiled from that PGN would contain every one of its positions to ply 16 and the move played from each. Coverage instead decays to a quarter, which is the overlap any general opening book has with any collection of master openings. `official-stockfish/books` also ships no Polyglot `.bin` at all -- every entry in it is `.pgn` or `.epd` -- so nothing there is a candidate for a 16-byte-entry binary book in the first place.

Reproduce with (`~/.venv/chess/bin/python`, `.moltke.local.md`): decode the hex, key the 16-byte entries by their big-endian `uint64`, and compare against `chess.polyglot.zobrist_hash` along each game. The Polyglot move field is `to_file | to_row<<3 | from_file<<6 | from_row<<9`; decoding it row-first silently drops the match rate to 35 of 4800 and looks like a refutation of everything, which is a trap worth naming.

**What the naming did settle: the two match books under `books/`.** Both come from `official-stockfish/books`, which is **CC0-1.0** -- `LICENSE` is the CC0 1.0 Universal text and the GitHub licence API reports `spdx_id: CC0-1.0`.

- `books/8moves_v3.pgn`, committed here, is byte-identical to the upstream file: zip sha256 `7e1e9dd118b4bb97d8a8b5b8a790c86e21f8509d59a27d2883767d94477be02e`, unpacked sha256 `5835239f88cc2c7511b177c32392a69f3ede21819cf0616f80a7f907cd21d17e`, which is the digest of the tracked copy. It had no recorded origin anywhere in the repository before this.
- `books/UHO_Lichess_4852_v1.epd` needed no research: `books/fetch_book.sh` already pins both its digests and states the licence reasoning -- upstream is Stefan Pohl (SPCC), whose own pages carry a copyright line and no usage licence, so the CC0 redistribution is the source and his site is not.

Those two are `excludes` for this step, so recording `8moves_v3.pgn`'s digest and licence beside the fetched books is a separate change. The blob remains what blocks S146.

## Parked 2026-09-02, DEC-126

The owner does not remember where the blob came from, has checked the history
himself and found nothing, and thinks the file may be on his other computer.
The step goes back to `adocs/plan_todo/` and its Open entry is tagged
`parked, DEC-126`, beside S029's. It resumes when that machine can be searched
or when one of the three standing options is taken instead: build a replacement
from a nameable source, delete the book and the `Use Book` option, or keep the
blob with provenance recorded as unknown.

Everything the episode did settle is committed and does not need redoing:
`polyglot_randoms[781]` under DEC-121, and `books/8moves_v3.pgn` pinned in
`books/fetch_book.sh` by both digests against the CC0-1.0 upstream. What
remains is one question -- the origin of `src/openings.book` -- and the
fingerprint above is what answers it.
