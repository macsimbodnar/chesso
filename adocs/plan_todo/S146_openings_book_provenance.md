id:         S146
goal:       the 5.2 MB opening book compiled into the shipped binary has a recorded origin and licence, or it is replaced by one that does
accepts:    `src/openings.book`'s origin, author and licence are established and written down where a reader will find them, or -- if they cannot be established -- the file is replaced by a book this project can account for and the replacement's provenance is recorded the way `books/fetch_book.sh` records the match books, with both digests and the licence named; whichever way it goes is a recorded decision, because this is the owner's call and not an agent's; the engine's book behaviour is unchanged or its change is measured, since `Use Book` defaults false and a book swap only alters play when it is switched on; `MANUAL.md` documents what the shipped book is
touches:    src/openings.book, src/openings.cpp, MANUAL.md, adocs/decisions.md
excludes:   the match books under `books/`, which `books/fetch_book.sh` already pins with licences; book *format* work; making the engine read a book from disk instead of from the binary, which is a feature and would need its own step and verdict
decisions:  DEC-016
closes:
blocks:
paused_by:
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

## What makes this cheap

`Use Book` defaults to **false**, so the book does not affect a single SPRT
verdict taken so far and replacing it alters no measurement. The cost is
research and a decision, not machine time -- which is why it can sit in
`plan_todo/` without blocking anything, and why it should not sit there
indefinitely.

## Cost

No match, no verdict. Research, a decision, and a documentation change.
