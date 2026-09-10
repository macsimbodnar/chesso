id:         S209
goal:       `setoption` follows the protocol on names and check values, `Hash` refuses a value that is not an integer in full and says so in every build, and `clean-tt` joins the search before it clears the table
accepts:    `src/chesso.cpp` `command_setoption` compares the option name and a check option's value case-insensitively, so `hash`, `HASH`, `ownbook`, `best book move` and `True`/`TRUE` are honoured exactly as their canonical spellings are, and the `Book File` **value** is still taken verbatim because it is a path; `Hash` is parsed with `std::from_chars` requiring the whole token, so `0x40`, `64abc` and `12.5` are refused rather than read as 0, 64 and 12, and the refusal is one `info string refused [Hash] <value>, not an integer` line on the UCI channel in **both** builds -- the S137 channel, not `LOG_W`, which is `if (false)` under `NDEBUG`; a refused value leaves the table size as it was; `command_clean_TT` calls `stop_and_join_search()` first, the rule the file's own comment states for every path that mutates the table; each behaviour has a red-first case in `tests/test_engine.cpp` observed red on the unfixed tree -- for the race, the `build-sanitize` tree gains a TSan-driven case or, if TSan cannot be built into the suite cheaply, the reproduction (`go infinite` then 40 `clean-tt`) is run by hand under a TSan build and its report count recorded in the stamp before and after; `MANUAL.md`'s `Hash` row stops saying a non-numeric value is ignored with a log warning and says what happens, and the two lines above document the case rule; `adocs/specs.md`'s setoption paragraph is updated in the same commit; `test_uci_surface` is refreshed only after both documents (SURFACE); INV-6 discharged on identical `tools/search_bench.py` counts and best moves and an identical `bench` signature
touches:    src/chesso.cpp, tests/test_engine.cpp, tests/test_uci_surface.cpp (golden), MANUAL.md, adocs/specs.md
excludes:   the `go` tokens parsed with `std::stoll` in `command_go` (`wtime`, `btime`, `winc`, `binc`, `movestogo`, `depth`, `nodes`, `movetime`), which are the same class and are noted here for S210 to decide rather than changed twice; a readback of live option values on `uci`, which `specs.md` records as absent; any change to what `Hash` clamps to
decisions:  DEC-170
closes:     2026-09-10_adversarial-F11, 2026-09-10_adversarial-F12, 2026-09-10_adversarial-F13
blocks:
paused_by:
author:
done:

## Why this exists

Three medium findings of `2026-09-10_adversarial`, all on the UCI surface the
harnesses drive and all silent in the binary that ships.

**F12.** `UCI.txt`, the repository's own copy of the protocol: "The name and
value of the option in `<id>` should not be case sensitive". `command_setoption`
compares with `==`, so `hash`, `HASH` and `ownbook` are refused as unknown
options, and `True`/`TRUE` are ignored for a check option. In the tune build
the refusal prints; in the Release build it prints nothing, so a harness that
writes `hash` measures the default and never learns -- the failure class
DEC-093 and S137 exist for, one option along.

**F13.** `Hash` is read with `std::stoll`, which stops at the first character it
cannot use: `0x40` buys 1 MB (0, clamped to `TT_MIN_MB`) and `64abc` buys 64,
both silently. Twenty lines below, the tune build's search parameters are read
with `std::from_chars` requiring `ptr == last`, for exactly this reason and in
those words. DEC-088 pins the harness's hash deliberately; a typo that buys
1 MB measures an engine nobody configured. `MANUAL.md` says a non-numeric value
is ignored with a warning in the log, which is true for `abc` and false for
`0x40`.

**F11.** `command_clean_TT` calls `tt_reset` without `stop_and_join_search()`,
against the rule the file states at that function. A TSan build driving `go
infinite` plus 40 `clean-tt` produced 11 reports of `tt_get_entry` racing the
`memset`; the same harness with every other mid-search command produced 0.
`memset` clears low to high, so a probe can match a key not yet cleared and
read a zeroed score with an un-zeroed type. A custom command no GUI sends,
reachable from any script, documented in `MANUAL.md`.

## Shape

- Case-folding: fold the name once after `name` is read and before the chain
  of comparisons, and fold a check value before comparing to `true`/`false`.
  The `Book File` and search-parameter **values** are untouched.
- `Hash`: the `from_chars`-with-`ptr == last` form the search-parameter
  branch already uses, reporting through `uci_reply("info string refused
  ...")`, which is legal UCI in every build state. Keep the clamp.
- `clean-tt`: one line at the top of `command_clean_TT`.

## Cost

Agent work, about two hours with the tests and the two documents; no run.
Node-identical, no `Bench:` change, no SPRT. The `go`-token class is left for
S210 by name so it is not lost.
