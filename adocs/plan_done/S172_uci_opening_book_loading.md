id:         S172
goal:       an opening book is loadable over UCI on Stockfish's option surface, the built-in book stays built in as a raw Polyglot binary instead of a hex string, and a tool builds and inspects the book instead of nothing building it
accepts:    the engine advertises `OwnBook` (check, default false), `Book File` (string, default `<embedded>`) and `Best Book Move` (check, default false) and no longer advertises `Use Book`; `Book File` set to a path loads that Polyglot book from disk through `load_book_from_file()`, which has its first caller and its first test; `setoption` carries a value containing spaces, so a path with a space in it arrives whole; a book that is not a valid Polyglot file is refused at load with a logged reason and leaves the engine bookless rather than probing garbage; `Best Book Move` false selects among the position's entries in proportion to their Polyglot weight and true selects the highest-weight entry, replacing today's uniform-random pick; the built-in book is `src/openings.bin`, 2610256 bytes, sha256 `47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5`, the same 163141 entries the hex header decodes to today and not a different book, pulled into the binary by `.incbin` and probed in place with no startup decode and no copy; `build/tools/make_book` builds a Polyglot book from a PGN and dumps or verifies an existing one; the default configuration plays identically, discharged on `tools/search_bench.py` node counts and best moves per INV-6; `MANUAL.md`, `adocs/specs.md`, `DEV_MANUAL.md` and the golden `tests/test_uci_surface.cpp` all carry the new surface, in that order
touches:    src/openings.hpp, src/openings.cpp, src/openings.bin, src/openings_embedded.S, src/chesso.cpp, src/CMakeLists.txt, CMakeLists.txt, tools/make_book.cpp, tools/CMakeLists.txt, tests/test_openings.cpp, tests/test_uci_surface.cpp, tests/CMakeLists.txt, MANUAL.md, DEV_MANUAL.md, adocs/specs.md, adocs/decisions.md
excludes:   the built-in book's origin and licence, which is S146's and stays open -- this step changes the container and not the contents, and the digest above is what holds it to that; replacing the shipped book with one built from `books/8moves_v3.pgn`, which `make_book` makes possible and which is S146's ruling to take, not this step's; turning the book on by default, which DEC-085 forbids for a rated run and which no measurement here wants; the `polyglot_randoms[781]` table, settled at DEC-121; book learning, retired at DEC-085
decisions:  DEC-085, DEC-121
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-03
done:       2026-09-03. **All four parts landed and the digest holds the step to its own excludes.** The engine advertises `OwnBook` (check, default false), `Book File` (string, default `<embedded>`) and `Best Book Move` (check, default false); `Use Book` is gone, aliased nowhere. The built-in book is `src/openings.bin` -- **2610256 bytes, 163141 entries, sha256 `47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5`, exactly S158's recomputed figures for what the hex header decoded to**, so the container changed and the contents did not and S146 is untouched. It reaches the binary through `.incbin` from `src/openings_embedded.S`; `nm` reports the two symbols and `chesso_embedded_book_end - chesso_embedded_book_begin` is `0x27d450`, 2610256. `build/tools/make_book` builds a book from a PGN and dumps or verifies one. DEC-129 and DEC-130 are recorded.

            **Three defects were fixed, and each is a thing that had never run rather than a thing that ran wrong.** (1) `load_book_from_file()` had existed since the `bitboard` branch with **no caller anywhere in the tree**; it has one now and its first tests. (2) `setoption` read **one token** as the value, so `Book File value /nonexistent dir/with spaces/book.bin` reached the loader as `/nonexistent`. **Observed red**: `tests/test_engine.cpp`'s "setoption carries a value containing spaces" fails against the old parser -- the log line shows the split, `Args: ["name", "Book", "File", "value", "/nonexistent", "dir/with", "spaces/book.bin"]` -- and passes after. (3) `get_book_moves_for_key()` scanned **all 163141 entries per probe**; it binary-searches now, which the format's key ordering makes exact and which the loader now verifies. `tests/test_openings.cpp` asserts the search and the old scan agree over five positions including one no line reaches.

            **A book that will not load leaves the engine bookless**, never falling back to the built-in one, and says so as `info string book [<path>] not loaded: <why>. Playing without a book` -- on the UCI channel because `LOG_E` is `if (false) std::clog` under `NDEBUG` and the shipping binary is Release. All four refusals were driven end to end against the real binary: a missing path, a 100-byte truncation, a book with entry 50's key zeroed, and -- loading correctly -- a path containing a space, 172232 entries through `/tmp/.../My Books/with space.bin`.

            **Selection is weight-proportional and `Best Book Move` takes the heaviest**, where it drew uniformly and never read the `weight` field. Play-altering in principle and **no SPRT owed**: `OwnBook` defaults false and S158 established that no measurement this project has taken ever played a book move. **INV-6 discharged on the default configuration**: 121512 / 800769 / 62907 at depth 9 and 639228 / 3430710 / 367858 at depth 12, `c3d5` / `e2a6` / `d7c8q` at both, identical to the figures on record.

            **The tool was proved against the committed CC0 PGN, not against a fixture.** `make_book build books/8moves_v3.pgn --max-ply 16` reads **34700 games** -- the count `DEV_MANUAL.md` records for that file -- with **0 games cut short** and **555200 plies, exactly 34700 x 16**, so every SAN token in eight million bytes of PGN parsed through the engine's own `algebraic_to_move`. It writes 172232 entries over 129613 positions, the engine loads them, and `position startpos / go depth 1` answers `bestmove d2d4` from the book. `make_book dump src/openings.bin` reads 163141 entries over **154916 distinct positions**, which is the figure S146 measured independently, and its first entry matches S158's record byte for byte.

            **The embedding's saving is measured, not asserted: startup 4.7 ms +/- 0.4 to 2.8 ms +/- 0.1**, `hyperfine -N --warmup 50 -m 500` on `printf 'uci\nquit\n' | chesso` against Release builds of `32982a2` and the candidate, 574 and 976 runs, **x1.67 +/- 0.15**. Not a strength claim -- a time control does not charge for process start -- and it is the whole reason the embedding changed. The machine was **not** idle for it (Spotlight indexing PDFs through five `CGPDFService` workers, load average about 15), which is why the shell was taken out of the loop with `-N`; the sigmas are tight and the two means do not overlap.

            **One defect I introduced and caught before committing, recorded because the next reader will meet the same list**: the tune build's unknown-option guard carried the literal `"Use Book"`, so a renamed option would have been answered `info string refused [OwnBook], unknown option` in `build-tune` only. `tests/test_uci_surface.cpp` drives every advertised name through that guard and would have failed; it is fixed and the tune build's surface test passes 188 assertions.

            **One thing I broke and restored, recorded because DEC-052 exists for exactly it.** Mid-step I ran `rm -rf build && cmake -S . -B build` without arguments, which dropped `CMAKE_BUILD_TYPE=Release` and `ccache` out of the directory DEV_MANUAL.md calls "the one that gets measured": the tree then built unoptimized with asserts on, the per-test ctest timeout moved from **60 s to 600 s**, and `test_movegen` ran for **eight minutes** where the whole gate is 93.6 s. Nothing false was recorded from it -- the node counts INV-6 is discharged on are properties of the algorithm and matched the Release-built figures on record exactly, and the startup timing was taken against two directories configured `Release` by hand -- but every gate run before the restore was measuring a build nobody ships. `build` is reconfigured per DEV_MANUAL.md's own line and the gate below is the run on it. DEC-052 says nothing may reconfigure `build` behind your back; an agent is a "nothing" for that purpose.

            **A gate run failed and it was the machine, established rather than assumed.** `test_mate_breadth` reported `(Timeout)` in `build-tune` on a run taken while Spotlight was still indexing (load average 8 to 12 on eight cores). Re-timed on the quiet machine the same binary is **15.80 s** against a 120 s Release ceiling, and 15.46 s in `build` -- the tune build is not the cause either. Both suites were then re-run clean. The rule the episode is worth stating for: a suite that only ever passes on an idle machine has a ceiling that is really a load threshold, and this one is six times its Release figure, so the failure was contention and not a margin that has quietly closed.

            Completion gate, both on the quiet machine and both green: `build` **24/24 in 56.17 s**, `build-tune` **24/24 in 52.34 s**, `./clang-format.sh --check` exit 0. Docs: `MANUAL.md` carries the three option rows, the new `uci` reply and the corrected "outside this" paragraph; `adocs/specs.md` carries the book paragraph and moves the release surface from three lines to five; `DEV_MANUAL.md` gains "The engine's own opening book" with the digest and both `make_book` modes; `tests/test_uci_surface.cpp` refreshed **after** both, per the SURFACE rule. `README.md` untouched, as always.

## Why now

The owner asked for it directly on 2026-09-03, and it is the step's whole
justification: the engine can be given a book by a GUI or a harness the way
every other engine can, and the book it ships with is produced by something in
this repository instead of arriving as an opaque blob.

Three of the four pieces are already half-built and none of them work:

- **`load_book_from_file()` exists at `src/openings.cpp` and has no caller
  anywhere.** It has never run in this repository. It is dead code that looks
  like a feature.
- **`Use Book` is not a UCI option name.** The protocol's own name for this is
  `OwnBook`, and every GUI that offers a book checkbox looks for that name or
  for nothing.
- **The `setoption` value parser reads one token.** `option_value =
  args.front()` at `src/chesso.cpp`, so `setoption name Book File value
  /Users/max/My Books/x.bin` would set the value to `/Users/max/My`. This is
  invisible today because `Hash`, `Threads` and `Use Book` all take
  single-token values, and it becomes a defect the moment a string option
  exists.

## The option surface, and what "Stockfish semantics" means here

The owner's instruction was to follow Stockfish. Modern Stockfish has no book
at all -- it was removed in 2016 -- so the reference is the surface Stockfish
carried while it had one, which is also what the UCI specification describes:

| option | type | default | effect |
|---|---|---|---|
| `OwnBook` | check | `false` | the engine plays from its own book and the GUI does not play one for it |
| `Book File` | string | `<embedded>` | which book. `<embedded>` or an empty value means the one compiled in; anything else is a path to a Polyglot `.bin` |
| `Best Book Move` | check | `false` | `true` plays the highest-weight entry; `false` plays one at random in proportion to weight |

`Use Book` is removed rather than aliased. It is not in any harness script in
this repository -- `grep` over `*.sh`, `*.py` and `*.json` finds it only in
prose -- so nothing breaks that a rename would have saved, and carrying two
names for one setting is the kind of thing that is still there in five years.

**`Book File` on an unloadable path leaves the engine bookless**, logged, and
does not silently fall back to the embedded book. A harness that asked for a
specific book and got a different one is measuring something it did not
configure, which is worse than measuring an engine with no book.

**Weighted selection is a behaviour change and it is deliberate.**
`search_random_move_in_book()` picks uniformly among the matching entries today
and ignores the `weight` field entirely, so a line the book gives one game of
weight is played as often as one it gives two hundred. That is not the Polyglot
format's semantics and it is not Stockfish's. It alters play only when
`OwnBook` is on, which is off by default and off in every measurement this
project has taken (S158 established that no SPRT here has ever played a book
move), so it owes no SPRT -- but it is stated here so that the change is
recorded and not discovered.

## The embedding

`src/openings.book` is a 5220541-byte C header holding one
`#define BOOK "<hex>"`, decoded at every startup by `hex_string_to_vector()`
into a 2610256-byte `std::vector`. So the repository stores twice the bytes,
and every process that starts -- including every one of the thousands a
fastchess night starts -- parses 5.2 MB of ASCII to rebuild a constant.

It becomes `src/openings.bin`, the raw Polyglot file, pulled in by a small
`.S` file:

```
        .balign 8
        .globl  SYM(chesso_book_begin)
SYM(chesso_book_begin):
        .incbin "openings.bin"
        .globl  SYM(chesso_book_end)
SYM(chesso_book_end):
```

Half the tracked bytes, no startup work at all, and the probe reads the bytes
where the loader put them in `rodata` instead of a heap copy.

**Three things about this that will bite and are named so they do not:**

1. **The `.balign 8` is load-bearing.** `get_book_moves_for_key()` does
   `reinterpret_cast<const polyglot_entry_t*>(...)` over the bytes, which is
   fine over a `std::vector`'s over-aligned buffer and is not fine over an
   arbitrary label in `rodata`. `alignof(polyglot_entry_t)` is 8.
2. **Mach-O is not ELF.** Apple's assembler wants a leading underscore on the
   symbol and `.section __TEXT,__const` where ELF wants `.rodata`. The file is
   `.S` and not `.s` so the preprocessor runs and one `#if defined(__APPLE__)`
   covers both. This machine is the macOS one (`.moltke.local.md`), so the ELF
   half is the arm that will be written blind and needs saying so in the
   `done:` stamp.
3. **`src/CMakeLists.txt` globs `*.cpp`.** The `.S` is added by name, and the
   top-level `CMakeLists.txt` needs `enable_language(ASM)`. `CHESSO_ARCH`'s
   flags are directory-wide C++ flags and do not reach an assembler source,
   which is correct -- there is no code in it to target.

The alternative considered and rejected was a generated
`constexpr uint8_t[2610256]` header: portable to MSVC, no assembler, and a
16 MB source file that has to be compiled. The owner's stated preference is the
less complex and more performant option over the elegant one, and MSVC has
never built this tree.

## The tool

`tools/make_book` in two modes.

**Build.** `make_book build <pgn> --out <bin> [--max-ply N] [--min-games N]`.
Reads a PGN, plays each game through the engine's own parser and `make_move`,
keys every position with the engine's own `get_key()`, and accumulates a weight
per (position, move) from the game results. Writes a Polyglot `.bin` sorted by
key, which is the format's own requirement.

It keys with `get_key()` and parses with the engine's parser deliberately: a
book built against a second implementation of "the same position" is a book the
engine cannot probe, and that class of bug is silent -- it looks like a book
with no entries.

`tools/pgn_to_positions.cpp` is **not** reusable for the reading half. It takes
SAN tokens on stdin and knows nothing about tag pairs, results, comments, NAGs
or variations, so `make_book` needs a real PGN reader. That is the bulk of the
work in this step.

**Dump and verify.** `make_book dump <bin>` reports entry count, distinct
positions, maximum ply reached, the heaviest lines, and -- the one that matters
-- whether the file is a valid Polyglot book at all: size divisible by 16 and
keys non-decreasing. The engine's loader runs the same two checks, so a book
that `dump` accepts is a book the engine will load, and `Book File` pointed at
a JPEG is refused instead of probed.

## What the probe does with a sorted book

Sortedness is the format's guarantee and the loader now verifies it, so
`get_book_moves_for_key()` stops scanning all 163141 entries per probe and
binary-searches instead. Same moves found, same order among equal keys, so it
is behaviour-neutral by construction and covered by the existing
`tests/test_openings.cpp` case rather than owed anything new. It is in scope
because the validation it depends on is required by the file-loading feature
anyway, and doing the validation without using it would be leaving the result
on the floor.

## Verification

- **`ctest -L fast` green in both `build` and `build-tune`**, plus
  `./clang-format.sh --check`, per the TESTS rule.
- **The embedded book is the same book.** `shasum -a 256 src/openings.bin`
  reports `47a817350459843da2a20e1d5cba28462d9df30bdb99c93097bd3cb66ce78fb5`
  and the file is 2610256 bytes -- S158's recomputed figures for what the hex
  header decodes to. This is the one check that keeps the step out of S146's
  territory.
- **The default configuration is unchanged.** `tools/search_bench.py` at
  depth 9 and depth 12 against `HEAD`, expecting 121512 / 800769 / 62907 and
  639228 / 3430710 / 367858 with `c3d5` / `e2a6` / `d7c8q`. INV-6, so no SPRT.
- **A book loaded from a file is probed.** New test: `make_book` builds a small
  book from a fixture PGN, `load_book_from_file()` loads it, and a position in
  it returns a legal move. This is the first execution of that function in the
  project's history.
- **A bad book is refused.** New test, red first: truncated file, size not a
  multiple of 16, and keys out of order each leave the book unloaded.
- **The surface moved everywhere.** `tests/test_uci_surface.cpp` is refreshed
  only after `MANUAL.md` and `adocs/specs.md` describe the change, per the
  SURFACE rule.
- **The startup saving is measured, not asserted.** `hyperfine` over
  `printf 'uci\nquit\n' | chesso` before and after. It is not a strength claim
  and no Elo is attached to it; it is what the embedding change was for.

## Decisions this owes

Two, proposed by the agent and taken by the owner, written when they are taken:

- **DEC-129** -- the book's UCI surface is Stockfish's, `Use Book` is removed
  rather than aliased, an unloadable `Book File` leaves the engine bookless,
  and book move selection becomes weight-proportional.
- **DEC-130** -- the built-in book is embedded as a raw binary through
  `.incbin` rather than as a hex string or a generated array, at the cost of
  MSVC portability the tree has never had.
