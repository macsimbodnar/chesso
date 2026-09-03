id:         S173
goal:       `make_book build` replaces a book atomically, so a failed write leaves the previous book where it was instead of destroying it
accepts:    `build` writes to a temporary file beside the destination and renames it into place only after the stream is verified good, so a short write, a full volume or a kill mid-write leaves any pre-existing file at `--out` byte-for-byte unchanged; the temporary is removed on every failure path and nothing is left beside the destination; `rename` within the same directory is used rather than a copy, so the replacement is atomic on the filesystems this project runs on; reproduced red before the change on a 1 MB HFS ram disk with a pre-existing file at `--out` -- the file is destroyed today -- and green after; the shipped book's digest is unchanged by the change, `3b89a4ad9146e266ae9296778067aaedcb7f57f3cf0ff2086b9ae6df15b873dd`; `DEV_MANUAL.md`'s "loadable does not mean complete" paragraph is amended to say what the tool now guarantees
touches:    tools/make_book.cpp, DEV_MANUAL.md
excludes:   the engine's own book loading, which reads and never writes; `dump`, which opens read-only; making any other tool in `tools/` write atomically, which is the same idea and a separate decision
decisions:  DEC-131
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found by S146's fast check over its own diff, 2026-09-03, and it is the *fix*
the reviewer proposed rather than the problem it reported.

**The reported problem was not real and that is worth stating first**, so nobody
re-derives it. The finding was that S146's new failure path calls
`std::remove(options.out.c_str())` and so "deletes any file at the output path,
including pre-existing files not created by this run". It does delete one. It
destroys nothing, because `std::ofstream output(options.out, std::ios::binary)`
(`tools/make_book.cpp:422`) opens `"wb"` and truncates the file the instant it
succeeds -- verified directly: a bare open with no write and no `remove` took a
nine-byte file to zero bytes. By the time the guard runs, the previous contents
are already gone. Removing the remnant is strictly better than leaving it,
because of the invariant below.

**What is real is that the whole operation is not atomic.** The tool destroys
the old book before it knows it can write the new one, and that is the thing
worth fixing:

- entries are sixteen bytes and sorted by key, so **any prefix of a valid book
  is a valid book**. `make_book dump` and the engine's loader both call a
  truncated file `loadable`; neither check can notice truncation. S146 measured
  it -- 901120 of 2755712 bytes written, 56320 entries, verdict `loadable`,
  engine loaded it.
- so the failure mode is not "the write failed and I noticed". It is "the write
  failed, the old book is gone, and what is on disk passes every validator in
  the tree". S146's guard turns that into a clean error and an absent file,
  which is safe but still loses the old book.

Write beside the destination, `rename` on success. The old book survives every
failure, and the replacement is never observed half-done by anything reading
concurrently.

## Cost

Small: the guard S146 added is already the place the temporary is cleaned up
from, and the shipped book's digest must not move, which is the check that the
change is behaviour-preserving on the success path.

## Not urgent

`--out` is pointed at `src/openings.bin` about once, and that file is in git.
The class of bug is the reason to fix it, not the exposure.
