id:         S194
goal:       the UCI book path is executed by the fast suite, with the weighted draw seeded through an environment variable
accepts:    `CHESSO_BOOK_SEED`, read once at startup, seeds the book's `mt19937_64` when set and leaves `std::random_device` in place when absent; two fast cases on the embedded book: with `OwnBook true`, `position startpos` and `go depth 1` the `bestmove` is one of the moves the library returns for the start key, over several seeds; with `Best Book Move true` it is the heaviest entry; the S175 repaired position (`rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8`) answers `bestmove d2f3` with no `info` line; the previously unexecuted block of `src/chesso.cpp` (the probe, the draw and `Best Book Move`) is shown executed, by the S197 coverage recipe or by an observable in the test; `MANUAL.md` documents the variable in its book section; no UCI option is added and the golden surface is unchanged; an unparsable `CHESSO_BOOK_SEED` is **refused, not ignored** -- one `info string` line at startup naming the variable and the value it could not read, in both builds, after which the seed falls back to `std::random_device` -- documented in `MANUAL.md`'s book section before `test_uci_surface` is touched (DEC-184); fast suite green in both builds
touches:    src/chesso.cpp, tests/test_engine.cpp, MANUAL.md, DEV_MANUAL.md, adocs/specs.md
excludes:   the book format, `src/openings.cpp`, `tools/make_book`
decisions:  DEC-139, DEC-184
closes:     2026-09-04_test_review-F06
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F06`. Coverage of the fast label shows the book probe,
the weight-proportional draw and `Best Book Move` in `src/chesso.cpp` with zero
executions; `tests/test_engine.cpp`'s book case tests `validate_book_move` on
a synthetic move and never reaches the probe, and `tests/test_openings.cpp`
tests the library. The S172 and S175 defects were in exactly this path, and
their regression guard is a command run by hand and recorded in a stamp. The
draw is seeded from `std::random_device` with no hook, which is why the path
has never been in a deterministic test.

## Cost

Machine-free, hours.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

With `OwnBook` on, `command_go` in `src/chesso.cpp` calls `search_book_move`
instead of searching: the library returns the entries under the position's
Polyglot key, `validate_book_move` drops any the generator does not produce,
then the heaviest entry is taken (`Best Book Move true`) or one is drawn in
proportion to weight, and `bestmove` is printed alone. Coverage found that block
never executed in the fast label (F06): "setoption carries a value containing
spaces" in `tests/test_engine.cpp` uses an unloadable path so the probe returns
early, and `tests/test_openings.cpp` tests `get_book_moves_for_key` below the
UCI layer. The draw reads `static std::mt19937_64 gen(rd())` at the top of
`src/chesso.cpp`, seeded from `std::random_device` with no hook, which is why no
deterministic test exists. This step adds one environment variable that seeds
`gen` and two fast cases over the embedded book. No configuration a match uses
changes.

### 2. The technique as published

Polyglot's format page (http://hgm.nubati.net/book_format.html): "A Polyglot
book is a series of 'entries' of 16 bytes" (key, move, weight, learn), "All
integers are stored highest byte first", "The entries are ordered according to
key. Lowest key first", and the selection rule the draw implements: "the
probability that a move is selected is its weight divided by the sum of the
weights of all the moves in the given position." The wiki's Opening Book page
(https://www.chessprogramming.org/Opening_Book) notes book moves "can be chosen
randomly, whereas searches are more or less deterministic" -- the property that
needs a seed before it can be tested. The oracle is python-chess's
`chess.polyglot` (https://python-chess.readthedocs.io/en/latest/polyglot.html):
`find_all` "Seeks a specific position and yields corresponding entries", `find`
returns "the (first) entry with the highest weight", `weighted_choice` selects
"distributed by the weights of the entries" -- the two rules chesso implements.

The test practice is the seeded-RNG replay: the random choice takes a seed
settable from outside and defaults to something non-reproducible, so a failure
replays. GoogleTest documents it for its shuffle
(https://google.github.io/googletest/advanced.html): a seed "calculated from the
current time" by default, printed, and set "using the `--gtest_random_seed=SEED`
flag (or set the `GTEST_RANDOM_SEED` environment variable)". The repository has
one instance already, `test_seed` in `tests/test_chesso.cpp` reading
`CHESSO_TEST_SEED`. Chesso's form keeps `std::random_device` as the fallback --
a harness must not get the same line every game -- and reads the variable at the
engine's startup, not the test binary's.

Two standard facts bound what the test may assert. [rand.predef]
(https://eel.is/c++draft/rand.predef) fixes `mt19937_64` and requires "The
10000th consecutive invocation of a default-constructed object of type
mt19937_64 produces the value 9981545732273789042", so one seed gives one raw
sequence on the workstation's libstdc++ and this machine's libc++. But
[rand.dist.general] (https://eel.is/c++draft/rand.dist.general): "The algorithms
for producing each of the specified distributions are implementation-defined",
so the `std::uniform_int_distribution` ticket can differ between the two. Hence
membership and same-process reproducibility only, never "seed 7 gives d2d4".
[rand.device] (https://eel.is/c++draft/rand.device) lets `random_device`
"employ a random number engine", so the fallback is tested for no value.

### 3. What chesso has today, and where the change plugs in

All in `src/chesso.cpp` unless said otherwise:

- `rd`, `gen`: file-scope statics, `gen(rd())` at static initialisation.
  `search_book_move` is `gen`'s only reader.
- `uci_init`: called once by `main.cpp`, and at the start of every
  `tests/test_engine.cpp` case. **The seed is read here**, after
  `try_load_opening_book()`: `std::getenv("CHESSO_BOOK_SEED")`; if non-null,
  `std::strtoull` base 10, accept only a non-empty string consumed to its end,
  then `gen.seed(static_cast<std::mt19937_64::result_type>(value))`; otherwise
  do nothing, so `rd()`'s seeding stands. A rejected value goes to `LOG_W`,
  Debug-only per `src/log.hpp` and `MANUAL.md`'s "a warning in the log". Needs
  `<cstdlib>`. Not in `gen`'s initialiser: that runs before doctest's `main`, so
  no `TEST_CASE` could set the variable first. Thread safety: `uci_init` runs
  before any search thread exists (created in `command_go` after the probe has
  returned), `gen` is only touched on the UCI thread, and POSIX's "The getenv()
  function need not be thread-safe"
  (https://pubs.opengroup.org/onlinepubs/9699919799/functions/getenv.html) is
  moot for one call before a second thread starts.
- `search_book_move`: probe, filter, heaviest loop (strict `>`, first maximum
  wins ties), ticket draw over `[0, total - 1]` walking the entries. Untouched.
- `command_go`: consults the book only when `!infinite && nodes == 0`; on a hit
  prints `bestmove` and returns before any `info` line (its `info score` reply
  is commented out) -- the accepts' observable.
- `opening_book_enabled`, `opening_book_best_move`, `opening_book_file`,
  `still_in_opening`: statics **not reset by `uci_init`**. `still_in_opening` is
  re-armed only by `command_ucinewgame`, `set_position` on a *changed* FEN, a
  `Book File` change and `try_load_opening_book`.
- `src/openings.hpp` `load_book_embedded`, `get_book_moves_for_key`: what the
  test calls for the library's own set. Excluded from change.
- `tests/test_helpers.hpp` `stdout_capture_t`: swaps `std::cout`'s buffer, sees
  `uci_reply`, never `LOG_*` (`std::clog`).

Order: the `uci_init` block; the two cases; the docs; the coverage run; the gate.

### 4. Constants and seeds

Nothing enters `src/search_params.hpp`. The seeds the test sets are arbitrary
labels, not DEC-105 seeds: use 1 to 13, one per start-key entry. The parse rule
(base-10 unsigned, whole string, else ignored) is chosen here for the smallest
surface; `MANUAL.md` states it verbatim.

### 5. Interactions and traps

- **State leaks between cases.** doctest runs the binary as one process in file
  order (`order-by` defaults to `file` in `tests/doctest/doctest/doctest.h`) and
  the option statics outlive `uci_shutdown`. Each new case: `ucinewgame` after
  `uci_init`, set `OwnBook` and `Best Book Move` explicitly, restore both to
  `false` at the end as the spaces case does for `Book File`, and
  `unsetenv("CHESSO_BOOK_SEED")`. `setenv`/`unsetenv` are POSIX (`<cstdlib>`);
  there is no Windows build.
- **Only `uci_init` reseeds.** Two draws from one seed need
  `uci_shutdown(); uci_init();` between them; `ucinewgame` does not reseed.
- **Capture around `go` only.** Construct `stdout_capture_t` just before
  `go depth 1` so "the only line is `bestmove`" is clean in both builds. A book
  hit starts no thread, so `uci_wait_for_search()` is a no-op there; the
  negative control needs it.
- **`Best Book Move` is deterministic without a seed**; its case sets no
  variable, or a reader will think the pin depends on one.
- **S193 R3 edits the neighbouring spaces case** (adds
  `REQUIRE(capture.contains("info score "))`) and lands first. Leave it alone;
  the negative control below reuses that observable for the *loaded* book with
  `OwnBook false`, which R3 does not cover.
- No engine source is read (DEC-016); the format page and python-chess are the
  references. Ordering bands, INV-4, fail-soft, improving, time management:
  untouched, no search code moves.

### 6. Tests

Shared precondition: after `uci_init()` and `ucinewgame`,
`book_t book; REQUIRE(load_book_embedded(&book));` then
`get_book_moves_for_key(&book, &uci_game()->board, moves, weights)` on startpos,
each move through `uci_move_to_algebraic` into a `std::set<std::string>`.
Goldens, named at their site with the re-deriving command (DEC-142):
`count == 13`, weights summing to `34700`, heaviest `e2e4` at `12956` --
`build/tools/make_book dump src/openings.bin --top 10` (key `463b96181691fc9c`:
`e2e4 12956`, `d2d4 12493`, `c2c4 4192`, `g1f3 4128`; the rest `g2g3 579`,
`b2b3 183`, `f2f4 107`, `b2b4 19`, `b1c3 15`, `e2e3 13`, `a2a3 7`, `d2d3 6`,
`c2c3 2` from
`~/.venv/chess/bin/python -c 'import chess, chess.polyglot as p; r=p.open_reader("src/openings.bin"); [print(e.move.uci(), e.weight) for e in r.find_all(chess.Board())]'`,
run 2026-09-05). The 13 is the precondition: an empty set makes membership
unfalsifiable.

```
TEST_CASE("OwnBook answers a book move for the start key, seeded")
  for seed in 1..13:
    setenv("CHESSO_BOOK_SEED", seed, 1); uci_init(); ucinewgame
    setoption OwnBook true; setoption Best Book Move false; position startpos
    { stdout_capture_t c; go depth 1; uci_wait_for_search();
      REQUIRE(c.lines().size() == 1); first = token after "bestmove " }
    REQUIRE(library_set.count(first) == 1)
    uci_shutdown(); uci_init(); ucinewgame; same options; position startpos
    { capture -> second }; REQUIRE(second == first)    // seed honoured
    uci_shutdown()
  uci_init(); ucinewgame; setoption OwnBook false; position startpos   // control
  { stdout_capture_t c; go depth 1; uci_wait_for_search();
    REQUIRE(c.contains("info score ")); REQUIRE(c.contains("bestmove ")) }
  unsetenv("CHESSO_BOOK_SEED"); uci_shutdown()

TEST_CASE("Best Book Move plays the heaviest entry, and the S175 position d2f3")
  uci_init(); ucinewgame; setoption OwnBook true; setoption Best Book Move true
  position startpos; { capture; go depth 1 }  REQUIRE(lines == {"bestmove e2e4"})
  position fen rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8
  { capture; go depth 1 }  REQUIRE(lines == {"bestmove d2f3"})     // no info line
  setoption Best Book Move false; same fen -> still {"bestmove d2f3"}
  setoption OwnBook false; uci_shutdown()
```

The S175 position has one entry under key `bff7aaf88aaf9fbb`, `d2f3` weight 1
(python-chess `find_all`, 2026-09-05; the key `tests/test_audit_polyglot_key.cpp`
pins), so both rules must answer `d2f3` with any seed, hence checked under both.
The `second == first` check makes the seed falsifiable: with the seed ignored,
two independent draws agree with probability 0.298 (sum of squared weights over
34700 squared), thirteen pairs about 1.5e-7. Mutants for
`tools/mutation_check.py` once S196 lands, in S196's file format: delete the
`gen.seed` call (killed by the pair check); flip `>` to `<` in the heaviest loop
(killed deterministically, startpos answers `c2c3`). DEC-141 owes no mutant --
no pruning rule -- these are extra. Debug self-play **not owed**: no
`make_move`, `unmake_move`, generator or search change.

Coverage: run the S197 recipe in `DEV_MANUAL.md` "Test" (clang only, `xcrun`
prefix on macOS) and quote in the stamp `show.txt`'s counts for
`search_book_move`'s draw and heaviest lines against zero in
`adocs/data/2026-09-04_test_review/coverage_unexecuted.txt`. If the recipe is
unavailable, the `bestmove`-only reply is the observable the accepts permits;
the stamp says which was used.

INV-6, before and after: `python3 tools/search_bench.py ./build/src/chesso 9`
and `... 12`, identical counts and best moves (`OwnBook` defaults false).

### 7. Measurement

Behaviour-neutral: no SPRT, no pre-registration. The added code runs once at
startup, so no interleaved `hyperfine` is owed; say so in the stamp. Record both
`search_bench.py` signatures. Commit trailer `No functional change` (DEC-140,
checked by S189's `tools/gate.sh`).

### 8. Completion checklist

- Gate, both builds: `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`
  (`-j` to the workstation's cores); then `tools/gate.sh` if S189 has landed.
- `MANUAL.md` "The book it ships with": `CHESSO_BOOK_SEED`, an unsigned base-10
  integer in the environment, read once at startup, seeds the weighted draw;
  unset or unparsable, `std::random_device` seeds it and the draw differs per
  process; `Best Book Move true` never consults it. `DEV_MANUAL.md`: the book
  section gets the variable and the two case titles, "Test" one line beside the
  coverage recipe. `adocs/specs.md` book paragraph gains the seeding sentence,
  through the coordinator (PLAN rule); `specs.md` is not in `touches:`.
- SURFACE: no option added, `expected_option_lines` in
  `tests/test_uci_surface.cpp` and the `uci` reply unchanged; run it, refresh
  nothing.
- Stamp: case titles, coverage counts or the observable used, both signatures,
  mutants run and their kills, docs touched, suite counts for both builds.
  `plan.md` and `status.md` via the coordinator.

### 9. Sources read

- http://hgm.nubati.net/book_format.html -- layout, byte order, ordering,
  proportional selection. Fetched.
- https://python-chess.readthedocs.io/en/latest/polyglot.html -- `find_all`,
  `find`, `choice`, `weighted_choice`. Fetched.
- https://www.chessprogramming.org/Opening_Book -- random book choice. Fetched.
- https://eel.is/c++draft/rand.predef, https://eel.is/c++draft/rand.dist.general,
  https://eel.is/c++draft/rand.device -- portable engine, implementation-defined
  distributions, `random_device` may be an engine. Fetched.
- https://pubs.opengroup.org/onlinepubs/9699919799/functions/getenv.html --
  null on absence, "need not be thread-safe". Fetched.
- https://google.github.io/googletest/advanced.html -- seed replay, flag and
  environment forms. Fetched.
- https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine,
  `.../random_device`, `.../uniform_int_distribution`,
  https://en.cppreference.com/w/cpp/utility/program/getenv -- HTTP 403 twice
  each, **unverified**; the eel.is and POSIX pages carry the same facts.
- Repository: `adocs/audit/2026-09-04_test_review.md` F06 and coverage table;
  `adocs/testing_strategy.md` R6; `adocs/plan_done/` S172, S175, S146;
  `adocs/plan_todo/` S193 R3, S197 recipe, S189, S196;
  `adocs/data/S175_book_conformance.py`; `src/chesso.cpp`, `src/openings.hpp`,
  `src/openings.cpp`, `src/log.hpp`, `src/main.cpp`, `src/uci.hpp`,
  `src/CMakeLists.txt`; `tests/test_engine.cpp`, `tests/test_helpers.hpp`,
  `tests/test_chesso.cpp`, `tests/test_openings.cpp`, `tests/test_uci_surface.cpp`,
  `tests/test_audit_polyglot_key.cpp`, `tests/CMakeLists.txt`; `MANUAL.md`,
  `DEV_MANUAL.md`, `adocs/specs.md`; `build/tools/make_book dump` and
  python-chess 1.11.2 (`~/.venv/chess`) over `src/openings.bin`, 2026-09-05.

### 10. Questions deferred to the owner

- `adocs/specs.md` is owed a sentence by the DOCS rule but is not in
  `touches:`: add it to the header, or accept the coordinator writing it.
- An unparsable `CHESSO_BOOK_SEED` is proposed to be ignored silently in
  Release (Debug logs it). An `info string` refusal instead is a
  protocol-visible startup line and needs `MANUAL.md` first.

**Answered 2026-09-11, DEC-184.** `adocs/specs.md` joins `touches:` and the
coordinator writes its sentence, which the PLAN rule already provides for. The
unparsable seed is refused with an `info string` line, `MANUAL.md` first -- the
S172 and S176 pattern: what the engine drops, it says so on the channel.
