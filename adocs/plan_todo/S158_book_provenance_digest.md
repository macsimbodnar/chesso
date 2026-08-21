id:         S158
goal:       S146 carries the shipped book's digest and entry count so its origin can be searched for rather than guessed
accepts:    `adocs/plan_todo/S146_book_provenance.md` carries the decoded book's digest and entry count, so its origin can be searched for rather than guessed: 2610256 bytes, 163141 sixteen-byte Polyglot entries, sha256 `47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5`, first entry key `00000883b144421f` move `0dae` weight 45; the command that reproduces the digest from `src/openings.book` is recorded with it; the statement that `Use Book` defaults false (`src/chesso.cpp:930`) is recorded too, because it is what makes this a licence exposure and not a measurement contaminant
touches:    adocs/plan_todo/S146_book_provenance.md
excludes:   establishing the book's actual origin or licence, replacing it, and any decision about it, all of which are S146's and the owner's; the match books under `books/`, which `books/fetch_book.sh` already pins
decisions:  DEC-016
closes:     2026-08-21_adversarial-F10
blocks:
paused_by:
done:

## Why this is a step and not just a note

S146 cannot close without an identifier to search on, and the digest costs
nothing to compute. Splitting it out keeps S146's own decision -- establish,
replace, or record -- untouched by an agent.
