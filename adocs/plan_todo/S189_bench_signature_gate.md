id:         S189
goal:       a `bench` UCI command prints one node signature over a fixed position set, every commit touching `src/` carries it, and `tools/gate.sh` runs the gate and checks the built binary against the message
accepts:    `bench` is in the UCI dispatch table: `ucinewgame`, then a fixed set of at least eight positions -- the three of `tools/search_bench.py` plus positions that reach quiescence mates, promotions, en passant and castling -- each searched at a fixed depth, the per-position best moves printed above one final line `<nodes> nodes <nps> nps`; deterministic single-threaded: two runs in one process and two processes give the same total; `MANUAL.md` documents the command before `tests/test_uci_surface.cpp`'s golden list gains it (SURFACE rule); `tools/gate.sh` runs the TESTS rule command and then compares `build/src/chesso bench`'s total with the `Bench: <n>` line of the commit under test (HEAD by default, or a message file passed to it), exits non-zero on a mismatch or on a missing line in a commit that touched `src/`, and accepts `No functional change` only when the total equals the parent commit's; observed red on a commit whose message carries the previous signature after a functional change, then green; `DEV_MANUAL.md` "Test" and "Measure" say so and record the current signature; `tools/search_bench.py` keeps its timing role unchanged
touches:    src/chesso.cpp, MANUAL.md, tests/test_uci_surface.cpp, tools/gate.sh, DEV_MANUAL.md, adocs/specs.md
excludes:   any change to the search; changing what `tools/search_bench.py` measures; a remote CI service (considered and refused, `adocs/testing_strategy.md` section 5)
decisions:  DEC-139, DEC-140
closes:     2026-09-04_test_review-F07
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F07`. INV-6 is discharged by a person running
`tools/search_bench.py` twice and comparing by eye; nothing records the number
where a gate can read it. Every CI surveyed for `adocs/testing_strategy.md`
section 3.1 checks a bench signature from the commit message before anything
else runs -- Stockfish's `tests/signature.sh`, Berserk, Stash, the fishtest
and OpenBench workers -- and the fault-injection pass measured what it is
worth here: 21 of 33 injected bugs moved the depth-9 counts, including a
one-ply reverse-futility floor drift that two of the three mate gates did not
see. DEC-140 is the commit-message rule; this step is the command and the
check.

## Shape

`bench` sends `ucinewgame` before every position (S195's warm-table rule), so
a repeated position is searched cold. The position set is fixed in the source
and named in `MANUAL.md`; changing it changes the signature and is itself a
`Bench:` line. `tools/gate.sh` is the TESTS rule command followed by the
comparison; it prints `GATE-DONE` or `GATE-FAILED` last so a detached run can
be watched (WATCHERS rule). The R1 entry of `adocs/testing_strategy.md`
section 4 is the recommendation this step implements.

## Cost

Machine-free. About half a day: the command, the surface documents and golden,
the script, the manual sections.
