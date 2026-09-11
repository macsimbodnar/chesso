id:         S209
goal:       `setoption` follows the protocol on names and check values, `Hash` refuses a value that is not an integer in full and says so in every build, and `clean-tt` joins the search before it clears the table
accepts:    `src/chesso.cpp` `command_setoption` compares the option name and a check option's value case-insensitively, so `hash`, `HASH`, `ownbook`, `best book move` and `True`/`TRUE` are honoured exactly as their canonical spellings are, and the `Book File` **value** is still taken verbatim because it is a path; `Hash` is parsed with `std::from_chars` requiring the whole token, so `0x40`, `64abc` and `12.5` are refused rather than read as 0, 64 and 12, and the refusal is one `info string refused [Hash] <value>, not an integer` line on the UCI channel in **both** builds -- the S137 channel, not `LOG_W`, which is `if (false)` under `NDEBUG`; a refused value leaves the table size as it was; `command_clean_TT` calls `stop_and_join_search()` first, the rule the file's own comment states for every path that mutates the table; each behaviour has a red-first case in `tests/test_engine.cpp` observed red on the unfixed tree -- for the race, the `build-sanitize` tree gains a TSan-driven case or, if TSan cannot be built into the suite cheaply, the reproduction (`go infinite` then 40 `clean-tt`) is run by hand under a TSan build and its report count recorded in the stamp before and after; `MANUAL.md`'s `Hash` row stops saying a non-numeric value is ignored with a log warning and says what happens, and the two lines above document the case rule; `adocs/specs.md`'s setoption paragraph is updated in the same commit; `test_uci_surface` is refreshed only after both documents (SURFACE); INV-6 discharged on identical `tools/search_bench.py` counts and best moves and an identical `bench` signature
touches:    src/chesso.cpp, tests/test_engine.cpp, tests/test_uci_surface.cpp (golden), MANUAL.md, adocs/specs.md
excludes:   the `go` tokens parsed with `std::stoll` in `command_go` (`wtime`, `btime`, `winc`, `binc`, `movestogo`, `depth`, `nodes`, `movetime`), which are the same class and are noted here for S210 to decide rather than changed twice; a readback of live option values on `uci`, which `specs.md` records as absent; any change to what `Hash` clamps to
decisions:  DEC-170, DEC-178
closes:     2026-09-10_adversarial-F11, 2026-09-10_adversarial-F12, 2026-09-10_adversarial-F13
blocks:
paused_by:
author:     Claude Opus 5, coordinator, 2026-09-11
done:       2026-09-11. **`setoption` follows the protocol on case, `Hash` says so when it refuses, and `clean-tt` no longer clears the table under a running search.** `command_setoption` folds the option name once, before every comparison, and folds a check option's value before comparing it to `true`/`false`; `option_name` itself is left as it arrived so a refusal quotes back what was sent, `Book File`'s value stays verbatim because it is a path, and a spin value stays verbatim because it is a number. **`Hash` is parsed with `std::from_chars` requiring `ptr == last`** and answers two shapes on the UCI channel in **both** builds -- `info string refused [Hash] <value>, not an integer` for a token it cannot read in full and `info string refused [Hash] <value>, out of range` for a well-formed integer no `long long` can hold -- leaving the table the size it had; what `Hash` clamps to is untouched, so `value -5` still buys the minimum and still prints nothing. `command_clean_TT` calls `stop_and_join_search()` first, the rule `src/chesso.cpp` `stop_and_join_search` states at its own definition.
            **Every red observed on `013600d` before a line was written.** F12: `setoption name hash value 64` and `name HASH value 1` both left the table at **524288 entries** -- the option was not merely mis-parsed, it was not an option -- and `setoption name ownbook value True` then `go depth 1` printed `info score cp 72 ... pv e2e4`, a real search, where the canonical `OwnBook value true` answers `bestmove e2e4` with no `info` line at all: the book observable S193 established. F13: **`0x40` bought 32768 entries (1 MB, the clamped 0) and `64abc` bought 2097152 (64 MB)**, both in silence; `+64` bought 64 MB; `12.5`, `abc`, an empty value and both twenty-digit tokens were silent too. `12.5` is the one the entry count cannot see -- 12 MB rounds to the same power of two as 16 -- so the message assertion is what catches it, which is why both are asserted per token. F11: `clean-tt` during `go infinite` returned with **no `bestmove` on stdout**; the reply arrived only after the cleanup `stop`. In the tune build, `rfpmargin`, `RFPMARGIN` and `RfPmArGiN` were each answered `info string refused [<as sent>], unknown option` and left `RfpMargin` at its default 63.
            **The race is measured, and the first measurement lied.** ASan and TSan cannot be linked into one binary and `build-sanitize` is the ASan tree, so the `accepts`' second branch is what this took: a fourth, hand-configured TSan build, `go infinite` then 40 `clean-tt`. **`013600d`: 38, 41 and 36 reports** over three runs -- `tt_reset`'s `memset`, reached through `command_clean_TT` on the main thread, against `tt_store_entry` and `tt_get_entry` in the search thread, which is exactly F11's claim. **Candidate: 0, 0, 0.** And the A/B taken in the *same* build tree rather than across two: adding the one line to the `013600d` worktree that had just produced 36 reports takes it to **0, 0**. The first run of all reported **0 on the defective tree** because the sanitizer never started -- `FATAL: ThreadSanitizer: unexpected memory mapping`, this kernel's ASLR against TSan's shadow map -- which is indistinguishable from a fixed engine if the count is read alone. `setarch -R` and a `grep -q FATAL:` guard are now in `TOOLCHAIN.md` beside the build line, with the numbers, because that zero would have closed the finding.
            **Five red-first cases, all five observed red and now green.** `tests/test_engine.cpp` "setoption folds the option name and a check value" (both directions on the spin option, and the check option through the book, with the book-off precondition asserted so the silence means a book move), "setoption Hash refuses a value that is not an integer in full" (six malformed tokens and two out-of-range ones, each asserted on the line *and* on the table, with a legal value's resize and its silence as the precondition, and `-5`'s clamp as the control that must not become a refusal), "clean-tt joins the search before it clears the table" (a local scan rather than `bestmove_of`, because a missing line is the red case and a `REQUIRE` inside that helper would skip the cleanup and leave an infinite search running through the rest of the suite); `tests/test_search_params.cpp` "a mis-cased parameter name is still the parameter" (three casings, each checked on the value *and* on the silence, with `RfpMargn` as the half the fold must not swallow); `tests/test_uci_surface.cpp` "no two advertised option names collide when folded", the precondition the fold needs -- 529 assertions over the 33 names the tune build advertises, green before and after.
            **Three things the `accepts` did not settle, decided and recorded as DEC-178.** The fold reaches the **search parameter names** too, because the Shape says to fold before the chain and two rules on one surface is worse than one; so `test_uci_surface`'s S137 assertion is **re-stated, not relaxed** -- `Rfpmargin` is the parameter now, `RfpMargn` is the unknown option, and DEC-093's half that matters survives. `Hash` answers **two** shapes rather than the one the `accepts` names, because "not an integer" about `99999999999999999999` is false. And the suite guards the **join** rather than the race, the race being measured by hand as above.
            **INV-6 discharged.** `tools/search_bench.py` at depth 9: **121530 / 801481 / 72924** nodes, best moves **`c3d5` / `e2a6` / `d7c8q`** -- identical to the baseline S207 and S208 recorded. `bench` **30046849**, the parent's total, so the commit carries **`No functional change`** and not a `Bench:` line. No SPRT: nothing on any search path moved.
            **DEC-141's second tier does not bind** -- `make_move`, `unmake_move`, the generator and the search are untouched, and no pruning, reduction or extension rule was added -- and `tools/gate_extra.sh` last ran green on 2026-09-10, inside its weekly cadence. Gate: `ctest -L fast` **33/33 in `build` and 33/33 in `build-tune`**, format clean under `CLANG_FORMAT_MAJOR=22` (DEC-146).
            **Docs.** `MANUAL.md`: the `Hash` row stops saying a non-numeric value is ignored with a log warning and says what happens; a new paragraph under Options states the case rule and what is *not* folded; the tune-build refusal section gains the name-case sentence and the two `Hash` shapes as a fenced block, which is what `test_uci_surface`'s template list holds it to; the `clean-tt` row says it joins first; the known-bugs `Hash` entry names the one case that is now reported. `adocs/specs.md`: the `setoption` paragraph carries the two `Hash` lines, and two new paragraphs state the case rule (with the collision precondition and its test) and the table-mutation rule with `clean-tt` as the path that broke it. `TOOLCHAIN.md`: the TSan section above. `DEV_MANUAL.md` checked -- its bench signature line reads `At S208: 30046849`, which is still the signature, and nothing else it says about `setoption` or `clean-tt` moved. `README.md` untouched, human-owned. `test_uci_surface` refreshed **after** both documents (SURFACE).
            **Excludes honoured:** the `go` tokens `command_go` reads with `std::stoll` are untouched and S210's `accepts` already owns that decision by name; no readback was added; `Hash`'s clamp is unchanged. **Beyond `touches:`**, noted rather than hidden: `tests/test_search_params.cpp` gains the mis-cased-name case (the fold reaches the parameter chain, which that file is the home of), `TOOLCHAIN.md` gains the TSan section, and `adocs/decisions.md` gains DEC-178 -- all three consequences of decisions the `accepts` left open. Closes `2026-09-10_adversarial-F11`, `2026-09-10_adversarial-F12` and `2026-09-10_adversarial-F13`.

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
