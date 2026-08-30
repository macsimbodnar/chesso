id:         S146
goal:       the 5.2 MB opening book compiled into the shipped binary has a recorded origin and licence, or it is replaced by one that does
accepts:    `src/openings.book`'s origin, author and licence are established and written down where a reader will find them, or -- if they cannot be established -- the file is replaced by a book this project can account for and the replacement's provenance is recorded the way `books/fetch_book.sh` records the match books, with both digests and the licence named; whichever way it goes is a recorded decision, because this is the owner's call and not an agent's; the engine's book behaviour is unchanged or its change is measured, since `Use Book` defaults false and a book swap only alters play when it is switched on; `MANUAL.md` documents what the shipped book is; the `polyglot_randoms[781]` table (`src/openings.cpp:44`) gets the same owner ruling -- either format-defining constants are the published spec rather than a copied table, recorded as a decision and cited at the table, or the Polyglot path goes with the blob (2026-08-22_adversarial-F06)
touches:    src/openings.book, src/openings.cpp, MANUAL.md, adocs/decisions.md
excludes:   the match books under `books/`, which `books/fetch_book.sh` already pins with licences; making the engine read a book from disk instead of from the binary, which is a feature and would need its own step and verdict
decisions:  DEC-016
closes:     2026-08-22_adversarial-F06
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
