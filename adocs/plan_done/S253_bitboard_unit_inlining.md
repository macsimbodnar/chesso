id:         S253
goal:       make_move's accumulator hooks inline whatever else grows in src/bitboard.cpp, so a generator change is timed on its own cost and not on gcc's inlining budget
accepts:    `-fopt-info-inline-missed` shows no `eval_add_piece` / `eval_remove_piece` call left out of line in `make_move` and `unmake_move_impl` after the change (eight do at `32633c4`); INV-6 node-identical (bench and tools/search_bench.py at 9 and 12); the interleaved hyperfine timing of S020's shape (A/A first, paired ratio and CI) decides keep-or-revert; then S020's increments (a) and (b) are re-attempted on top from `.tuning/coord/S020_files/inc2_attempt_final.diff` (S020's timing scripts beside it) and timed the same way, a zero recorded as zero
touches:    src/eval_tables.hpp (the hooks), src/bitboard.cpp / .hpp, possibly CMakeLists.txt (a unit-local parameter)
excludes:   any behaviour change; raising inline-unit-growth globally without measuring every unit
decisions:  DEC-049, DEC-083
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-05 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against 5ad8837: bench 4081329 and bench 12 1860699 with every info line and bestmove identical; search_bench 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at depth 12, c3d5 / e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical. No SPRT owed (DEC-083). The accumulator hooks are always_inline, with a `Side` template form make_move's helpers call (no colour branch); `-fopt-info-inline-missed` and the object file show 0 out-of-line hook calls in make_move and unmake_move_impl, against 8 in make_move at the parent. Plain always_inline measured -0.92 % and was not taken; the chosen form -0.24 % (CI -0.35 .. -0.13, 24 pairs; A/A +0.10 %, CI -0.03 .. +0.23), a zero kept because it removes the cliff. S020's increments on top: (a) alone -0.83 %, (a)+(b) +0.11 % (CI -0.01 .. +0.24), kept on a zero, node-identical, perft 4 % faster than the parent where S020 read 11 to 15 % slower; whole tree -0.20 % (CI -0.39 .. -0.00). Suites 41/41 in build and build-tune, format clean (DEC-146 override). Second tier (DEC-141): Debug self-play 8 games at 4+0.04, 0 Assertion, 0 disconnect; tools/gate_extra.sh 5 stages green in 1025 s (.tuning/gate_extra_2026-10-05_S253.log). Timings in adocs/data/S253_timing.txt.

## Why this exists (2026-10-04, the coordinator, from S020's report)

S020 found `src/bitboard.cpp` at gcc 13.3's `inline-unit-growth` limit: at
`32633c4` gcc already refuses 61 inlinings in that unit on the budget, eight
of `make_move`'s accumulator calls among them. Every shape of S020's
generator seam grew the unit, `make_move` lost more of its inlined hooks (14
or 16 out-of-line call sites against 8, plus two in `unmake_move_impl`), and
perft fell 11 to 15 %; the search measured -1.89 % and -3.52 %. Rebuilt with
`--param inline-unit-growth=200`, parent and candidate both inline every
hook and perft reads 1075 / 1092 ms against 1082 / 1083 ms -- the seam was
not the cost. S042, S032, S030 and S117 all edit that unit next, and each
would be timed against the same cliff.

Candidate fixes, chosen by measurement: `[[gnu::always_inline]]` on the
hooks; a unit-local `#pragma GCC optimize` or a per-source compile option;
splitting the generator out of the unit. The first also shows whether the
eight refusals at the parent already cost speed today.

## Report (2026-10-05, the implementer)

### Deviations, first

1. **The fix that lands is not one of the three named candidates as
   written.** Candidate (i), `[[gnu::always_inline]]` on the two hooks as
   they are, met the inlining clause and measured **-0.92 %** on the search
   (CI -1.06 .. -0.78, 24 pairs) and about 5 % slower perft. Its opposite,
   `[[gnu::noinline]]` (also deterministic), made perft 18 % slower. What
   lands is (i) plus a `Side` template form of each hook: make_move's
   helpers (`add_piece` / `remove_piece` / `move_piece`) already know the
   piece's colour at compile time, so the form they call has no colour
   branch and half the code. The plain hooks stay, always_inline, and
   dispatch on the colour to the template form, so every other caller
   (unmake, `eval_refresh`, other units) is unchanged at source level. A
   Debug assert in the template form holds the colour to the piece.
2. **(ii) and (iii) were not built.** S020 already measured (ii)'s shape,
   `--param inline-unit-growth=200`, as perft 1075 / 1092 ms against
   1038 .. 1046 -- slower, like (i), for the same reason (every hook inlined
   in its two-branch form). (iii), splitting the generator out of the unit,
   is the largest change and was not needed once the template form met the
   accepts.
3. **The baseline was eight out-of-line call sites in `make_move` and none in
   `unmake_move_impl`**, confirmed from the object file, not only from gcc's
   remarks: 4 `eval_add_piece` + 4 `eval_remove_piece` calls in `make_move`'s
   code, and 8 more in `move_to_algebraic` (an inlined copy of
   `unmake_move_impl<BLACK>`, so gcc's remarks name `unmake_move_impl` for
   them). gcc's remarks at `5ad8837`: 61 `inline-unit-growth` refusals in the
   unit, 16 of them hook calls.
4. **S020's increments (a) and (b) are kept on a zero**, under the brief's
   rule (keep if non-negative and node-identical). (a) alone measured
   **-0.83 %** and would be reverted on its own; (a)+(b) together measured
   **+0.11 %** (CI -0.01 .. +0.24), a zero. The whole tree against the parent
   reads **-0.20 %** (CI -0.39 .. -0.00). Reason to keep: the seam is
   node-identical, removes the repeated king scan and pin loop S020 named,
   and costs nothing measurable. Reason not to: it adds a generator API
   (`gen_masks_t`, three overloads, a Debug `masks_match`) for no measured
   speed. The two land as separate commits (below), so B can be dropped
   without touching A.

### The inlining, counted

`.tuning/coord/S253_files/inl.sh ROOT` compiles `ROOT/src/bitboard.cpp` with
the Release flags and `-fopt-info-inline-missed` and lists every refused hook
call (any reason) plus the unit's `inline-unit-growth` total;
`calls.sh OBJ` counts the hooks' call relocations per function in the object,
the ground truth.

| tree | hook refusals (remarks) | out-of-line hook calls in the object | unit-growth refusals | `make_move` bytes |
|---|---|---|---|---|
| `5ad8837` | 16 | make_move 8, move_to_algebraic 8 | 61 | 3173 |
| (i) always_inline, plain hooks | 0 | 0 | 13 | 4867 |
| noinline (control) | -- | make_move 16, unmake_move_impl 8 each side | 21 | 2903 |
| **chosen: `Side` form, always_inline** | **0** | **0** | 12 | 3214 |
| chosen + S020 (a)+(b) | **0** | **0** | 46 | -- |

The hooks no longer enter the budget's ranking, so the seam's growth (12 to
46 refusals elsewhere) no longer reaches them.

### INV-6

`.tuning/coord/S253_files/inv6.sh` (S020's, timing fields stripped, streams
diffed whole) against a Release build of `5ad8837` in `.ref-builds/s253_base`:
`chesso bench` **4081329**, `bench 12` **1860699**, every info line and
bestmove identical; `search_bench.py` depth 9 **32932 / 70095 / 25178**
`c3d5` / `e2a6` / `d7c8q`, depth 12 **67792 / 280873 / 137893** `c3d5` /
`d5e6` / `d7c8q` -- identical for (i), noinline, the chosen fix, (a), and
(a)+(b). The final binary is byte-identical (`cmp`) to the one timed.

### Timing

S020's shape and scripts (`timing.sh`, `pairs.py`, `workload.py`, copied to
`.tuning/coord/S253_files/` with paths repointed): 300 stratified positions at
`go depth 11`, 26512806 nodes a run, about 7.4 s; one hyperfine invocation per
pair, order alternating. Governor `powersave` (recorded, not set); load average
1.4 to 2.7 across the runs, the top process `rustdesk` at about 15 %. Every
reading is in `adocs/data/S253_timing.txt`.

| comparison | pairs | speed-up | 95 % CI | paired t |
|---|---|---|---|---|
| A/A, parent vs byte copy | 16 | +0.10 % | -0.03 .. +0.23 | -1.67 |
| (i) vs parent | 24 | -0.92 % | -1.06 .. -0.78 | 13.67 |
| **chosen fix vs parent** | 24 | **-0.24 %** | -0.35 .. -0.13 | 4.36 |
| (a) on the fix, vs the fix | 24 | -0.83 % | -0.95 .. -0.71 | 14.56 |
| **(a)+(b) on the fix, vs the fix** | 24 | **+0.11 %** | -0.01 .. +0.24 | -1.88 |
| final tree vs parent | 24 | -0.20 % | -0.39 .. -0.00 | 2.12 |

`bench_movegen` perft (best of 10, three alternations each, against the
parent's 1023 .. 1044 ms; resolution 0.2 % at `-r 20`): (i) 1085 .. 1106,
noinline 1213 .. 1278, chosen fix 998 .. 1018, chosen + (a) 985 .. 994. S020
read its seam shapes on the parent at 1152 .. 1197.

**Keep-or-revert.** The chosen fix reads -0.24 %: under CLAUDE.md's 3 % line,
at about the A/A floor's width, recorded as a zero with a slight negative
sign. Kept on the step's own reason: the cliff is gone -- S020's seam cost
-1.89 % / -3.52 % and 11 to 15 % perft through the hooks, and on top of this
fix the same seam reads +0.11 % and perft 4 % faster than the parent. (a)+(b)
kept on a zero, deviation 4.

### Suites and second tier

- `cmake --build build -j8 && ctest --test-dir build -L fast` **41/41**,
  `build-tune` **41/41**, `./clang-format.sh --check` clean under
  `CLANG_FORMAT_MAJOR=22` -- one chain, exit 0
  (`.tuning/coord/S253_files/gate.log`). The first chain failed the format
  check on three lines of the seam; clang-format-22 applied, binary
  unchanged (`cmp`).
- Debug self-play, DEV_MANUAL's command, 8 games at 4+0.04: **0
  `Assertion`** in log and stdout, **0 `disconnect`**
  (`.tuning/coord/S253_files/debug_selfplay.*`).
- `tools/gate_extra.sh`: **5 stages green in 1025 s** (prose, citations, debug 384 s, sanitize 584 s, perft 56 s)
  (`.tuning/gate_extra_2026-10-05_S253.log`).
- No test added: behaviour-neutral, INV-6 is the proof; no pruning,
  reduction or extension rule moved, so DEC-141's mutant clause does not
  apply.

`MANUAL.md` checked: no UCI surface moved -- no change. `DEV_MANUAL.md`
checked: no command, flag or default moved, and it says nothing about
inlining -- no change. `specs.md`: no behaviour changed, no sentence proposed.
### Portability, and the split into two commits (coordinator follow-up)

The forced inline is spelt through one macro, `CHESSO_ALWAYS_INLINE` in
`src/data_structures.hpp` beside the other defines: `[[gnu::always_inline]]`
under `__GNUC__` (gcc and clang), `[[msvc::forceinline]]` under `_MSC_VER`
(CMakeLists builds MSVC `/W4 /WX`, where the gnu spelling is an unknown
attribute), empty otherwise. All four hook definitions use it. The Release
binary is byte-identical (`cmp`) to the one timed, for both commits. The
comment on the hooks now says sixteen inlined copies, eight hook calls in
`make_move_impl` times two colour instantiations.

The work lands as two commits, each gated on its own:

- **A, the hooks with the macro**: `.tuning/coord/S253_files/commit_A.diff`
  (`src/data_structures.hpp`, `src/eval_tables.hpp`, the four `<Side>` calls
  in `src/bitboard.cpp`). Applied alone to a worktree of `5ad8837`
  (`.ref-builds/s253_A`, doctest copied in from the main tree because the
  submodule fetch hung): both builds 41/41, format clean, exit 0
  (`gate_A.log`); `chesso bench` **4081329**; 0 out-of-line hook calls;
  binary `cmp`-identical to the timed fix.
- **B, S020's seam on top**: `commit_B.diff`, relative to A
  (`src/bitboard.cpp`, `src/bitboard.hpp`, `src/search.cpp`). A then B
  reproduce the working tree byte for byte. The working tree: both builds
  41/41, format clean, exit 0 (`gate_AB.log`); `chesso bench` **4081329**;
  binary `cmp`-identical to the timed (a)+(b).
