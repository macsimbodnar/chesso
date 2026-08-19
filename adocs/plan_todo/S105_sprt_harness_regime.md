id:         S105
goal:       fastchess.sh plays the surveyed engines' testing regime -- 8+0.08, Hash 16, a UHO-class unbalanced book -- and a behaviour-neutral change is accepted on an interleaved timing instead of a match
accepts:    `fastchess.sh` runs `tc=8+0.08` and `option.Hash=16`, and reads a UHO-class unbalanced opening book, with the decisive-game rate at the new control measured and recorded against the 45-60 % draw band the book class is built for; a **time-forfeit check at the new control is run first and its count recorded** -- read from the PGN filtered to the run, because the fastchess log is WARN-only and comes back empty (the S089 lesson); the games-per-hour figure before and after is recorded so the throughput claim is a number and not a hope; the default SPRT bounds are stated in `fastchess.sh` and DEV_MANUAL.md -- gainers `elo0=0 elo1=5`, non-regressions `elo0=-5 elo1=0`, DEC-063's evidence cited; `rating.sh` staying at Hash 128+ is stated as deliberate with DEC-088 as the reason; DEV_MANUAL.md's "Play games" section states the new control, the new hash, the new book, and the rule that a behaviour-neutral change is not sent to a match; tests/test_fastchess_script.sh still green
touches:    fastchess.sh, books/, rating.sh, DEV_MANUAL.md, tests/test_fastchess_script.sh
excludes:   any change under src/; the rating gauntlet's own time control, which is S128
decisions:  DEC-083, DEC-088, DEC-048, DEC-050
closes:
blocks:
paused_by:
done:

## The three mismatches, measured

**Time control.** `fastchess.sh:25` is `tc="10+0.2"`. The engines this plan
reads figures from test at 8+0.08 and 10+0.1. At roughly 80 moves a game,
10+0.2 costs about 52 s a game against about 29 s -- **1.8 times the cost per
verdict**, spent for no extra resolution.

**Hash.** `fastchess.sh:126` is `option.Hash=16` and that number turns out to
be right for the SPRT -- for a reason DEC-083 got wrong and DEC-088 corrects.
What transfers across time controls is table **pressure**, not table size:
every OpenBench engine preset tests STC at 8 to 32 MB (Stash's preset is
exactly 8+0.08 with Hash=16), because at the list's 2'+1" a game writes on the
order of 660 M nodes against 5.6 to 11 M entries -- roughly 60 to 120
overwrites per entry -- and 16 MB at 8+0.08 reproduces that ratio where 128 MB
undershoots it about eightfold. So the SPRT keeps Hash=16, and `rating.sh`
keeps 128+ because the gauntlet's job is the list's absolute regime. The
16-vs-512 measurement DEC-083 records (36 % fewer nodes, 21 % lower nps at
`go movetime 2000`) stands as a fact about table size at 2 s a move; it was
the wrong invariant to match at 0.2 s a move.

**Book.** `books/8moves_v3.pgn` is balanced. Pohl's measurement on the UHO
book class: balanced openings ran ~91 % draws where the UHO bands run 40 to
57 %, and every surveyed engine's OpenBench preset defaults to a UHO book --
8moves_v3 is registered nowhere as a default. An unbalanced book raises the
decisive-game rate and shortens every run, which is measurement capacity and
not strength.

## What this buys

Roughly three times the verdicts per night, against a plan whose own budget
line says measurement capacity is the binding constraint. It costs the
comparability of verdict sizes across this commit, which was already lost at
the DEC-049 machine move and which per-change verdicts against a named commit
never had.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

Two published regimes dominate, and this step lands between them by design.
**Fishtest** (Stockfish's framework) tests STC at `10+0.1 Hash=16`, LTC at
`60+0.6 Hash=64`, book `UHO_Lichess_4852_v1.epd format=epd order=random
plies=16`, adjudication `-resign movecount=3 score=600 -draw movenumber=34
movecount=8 score=20`, `-report penta=true`, per its own "Running Fastchess"
page. **OpenBench** instances test STC at `8.0+0.08s Threads=1 Hash=16MB`
(DEC-088's survey; Danny Hammer's testing guide shows the same preset), and
the framework applies a **250 ms time-margin buffer** "to prevent time losses
on unstable hardware" (OpenBench wiki, SPRT workloads page).

**The book class.** Pohl's UHO — Unbalanced Human Openings — are 100 %
human-played lines, deduplicated, cut at 6 or 8 moves, both queens still on,
kept only when the end position's KomodoDragon 3.3 eval (10 s/position) falls
in a stated band. His measured draw rates with top engines: **balanced books
91.6 %**; UHO band `+0.85/+0.94` **56.9 %**, `+0.95/+1.04` **45.6 %**,
`+1.00/+1.09` **40.3 %**, `+1.05/+1.14` **35.6 %** — each band step ≈ −5 %.
His stated recommendations: start at `+0.85/+0.94` and move up only if too
drawish, and **never let draws fall below ~45 %**, because pairs where the
opening is simply won for White score 1:1 and carry no signal. His claim of
adoption: "UHO-openings are used in the development of all top engines …
since August 2021" (talkchess t=85026). UHO 2024 ships eight overlapping
bands, each as 6-move, 8-move and 8-move_big **pgn/epd files** (band
`+0.85/+0.94`: 6 696 / 19 303 / 37 622 lines) in `uho_2024.7z`.

**Licensing splits the candidates.** Pohl's own pages carry
"(C) 2024 Stefan Pohl (SPCC)" and **no usage-license text**. The
`official-stockfish/books` repository is **CC0-1.0**, one zip per book, and
hosts `UHO_Lichess_4852_v1.epd` (2 632 036 positions, human Lichess games,
2–16 plies deep, avg 13.3, no position under 23 pieces — a milder band than
Pohl's), `UHO_4060_v1..v4.epd` (~242 k positions), Pohl's older `UHO_XXL_*.pgn`
sets, and the `8moves_v3.pgn` chesso runs today. The installed fastchess's own
docs example uses `UHO_Lichess_4852_v1.epd` with the fishtest adjudication.

**Bounds practice.** CPW's SPRT page tabulates gainer bounds by strength:
Stockfish STC `[0, 2]`, top-30 `[0, 3]`, **top-200 `[0, 5]`, everyone else
`[0, 10]`**, alpha=beta=0.05; fishtest's simplification bound is
`[-1.75, 0.25]` nElo; Hammer's guide gives non-regression `[-5, 0]` (strong)
and `[-10, 0]` (weak/<2500). Fastchess `model=normalized` states bounds in
nElo, which is what makes a bound mean the same games-to-verdict across draw
regimes — the reason fishtest states bounds in nElo and CPW recommends
fastchess for its pentanomial statistics. Chesso's existing `{0,5}` full /
`{0,10}` fast sit exactly in the published bands for its strength.

### 2. Shape for chesso

Verified against the installed `fastchess alpha 1.8.1 20260720-daa3ea2`
(`-help`): `format=(epd|pgn)`, `-srand`, `-each timemargin=N` (default not
stated), `-report penta` default true, `-log level=` **default warn** (the
S089 empty-log fact, now settable to info/trace), `-pgnout` **append=true by
default** plus optional `timeleft=`/`latency=` tracking, `-show-latency`.

- `fastchess.sh:25` `tc="10+0.2"` → `8+0.08`. `fastchess.sh:126`
  `option.Hash=16` **stays** (DEC-088). `fastchess.sh:23` book path →
  the new book, `format=pgn` at `:125` follows the chosen file's format.
- `fastchess.sh:51-61` holds both bounds blocks; the accepts adds the stated
  default pair — gainers `elo0=0 elo1=5`, non-regressions `elo0=-5 elo1=0`
  (DEC-063) — simplest as a third mode flag beside `--fast`.
- `fastchess.sh:63-64` fixed `/tmp/fastchess_${tag}.{log,pgn}` names plus
  append-by-default PGN is why S089 had to filter: adopt rating.sh's stamped
  per-run outdir (`rating.sh:72-77`) so "filtered to the run" is the filename.
- Forfeit machinery to borrow, not rewrite: `rating.sh:196-207` termination
  census + `tools/forfeit_report.py --max-pct` (FORFEIT_MAX_PCT 1.0, DEC-075).
- `rating.sh:28` is `hash_mb=64` today — **below the 128+ the accepts states
  as deliberate**; raise to 128 with DEC-088/DEC-089 cited (list conditions:
  hash 128–256). `rating.sh:27` tc stays 10+0.2 — S128's question, excluded.
- `tests/test_fastchess_script.sh:112-114` greps for exactly one
  `^fastchess \` line — keep the invocation shape; `:49` seeds the sandbox
  with `books/8moves_v3.pgn` — update to the new book name.
- `books/` holds only `8moves_v3.pgn` (8.0 MB). `src/uci.hpp:12`
  `MOVE_OVERHEAD_MS 50` is a read-only fact here (src/ excluded).
- Games-per-hour baseline exists: DEV_MANUAL.md:591-596, ~23 games/min at
  12 cores and 10+0.2.

### 3. Implementation sketch

1. Per-run stamped pgn/log names in `fastchess.sh`; smoke test still green.
2. Acquire the book with provenance: prefer `UHO_Lichess_4852_v1.epd` from
   the CC0 `official-stockfish/books` repo (fetch script + pinned sha256,
   file gitignored like `.ref-builds/` — ~190 MB unzipped is not a committed
   file); record the license alongside. Pohl's UHO 2024 band file is the
   fallback if the owner prefers his band structure — download only, do not
   commit (license unstated).
3. Flip tc, wire the book, add the non-regression bounds mode, state defaults
   in the script header and DEV_MANUAL.
4. Calibration run, self-play, few hundred games: count
   `[Termination "time forfeit"]` from this run's PGN **first** (accepts);
   record decisive rate against the band; record games/hour vs the 23/min
   baseline.
5. If the decisive rate says the band is wrong (draws < 45 %: milder class,
   UHO_4060 or 4852; > 60 %: hotter Pohl band), swap the file and re-measure —
   the band choice is the fitted constant here.
6. `rating.sh` hash 64 → 128, comment citing DEC-088. DEV_MANUAL "Play games"
   rewrite: control, hash, book, bounds table, DEC-083's timing-not-match rule.

### 4. Constants and seeds

- `tc=8+0.08`, `Hash=16` — fixed by owner decisions DEC-083/DEC-088, not fitted.
- Book file/band — seed: start `UHO_Lichess_4852_v1.epd` (CC0, fishtest's
  choice) or Pohl band `+0.85/+0.94` (his stated start); **decided here by the
  measured decisive rate** (draws ≥ 45 %, Pohl's floor). DEC-084.
- Gainers `{0,5}`, non-regressions `{-5,0}`, `alpha=beta=0.05`,
  `model=normalized` — seeds from CPW top-200 row + Hammer guide + DEC-063's
  own evidence; already chesso's practice.
- `timemargin` — 0 for the forfeit check run (measure the truth first);
  published seed **250 ms** (OpenBench wiki) only if forfeits appear, recorded.
- Adjudication `fastchess.sh:49` — **unchanged this step** (one change at a
  time); fishtest's `movenumber=34 score=20 / score=600` noted as the
  published alternative seed for a later step.

### 5. Pitfalls

- **Append-by-default PGN + fixed filename** contaminates every count with
  earlier runs — the S089 lesson generalised; stamped filenames kill it.
- **The log is WARN-only by default** and an empty log proves nothing; the
  PGN is the source of truth for forfeits (S089; `-log level=info` exists).
- **Draws below ~45 % are a failure mode, not a win** (Pohl): 1:1 pairs carry
  no signal. Chesso at 2559 may sit below the top-engine draw bands on the
  same book — that is exactly what the calibration run measures.
- Pohl's draw bands were measured with ~3600+ engines under his adjudication;
  the 45–60 % comparison is coarse. Keep chesso's adjudication fixed so the
  book is the one change (INV-6 discipline).
- `-repeat` (paired colours, `fastchess.sh:130`) is what makes an unbalanced
  book sound; never drop it, and penta reporting (default true) is what
  models the pair correlation.
- The smoke test's injection anchor (`^fastchess \`) and sandbox book path
  break silently if the invocation is reshaped or the book renamed.
- Harness options are external to the binary: existing `.ref-builds/`
  worktrees need no rebuild; an old REF simply plays under the new regime.
- Nothing Pohl-hosted gets committed while his pages state no license
  (CLAUDE.md rule 1 adjacency: bundle nothing whose license is unstated);
  the CC0 repo is the clean path.
- 12-way SMT concurrency already has measured forfeit data: foreign engines
  0.45 % worst case (`rating.sh:33-41`), chesso self-play 0 in 500 (S089) and
  0 in 3000 (S094) — but those were at 10+0.2 with a 200 ms increment;
  8+0.08 thins the margin over `MOVE_OVERHEAD_MS 50` by 2.5×, which is why
  the accepts puts the forfeit count first.

### 6. Measurement

This step changes the instrument, not the engine: **no SPRT is owed and no
node-identity proof applies** (no `src/` change; `tools/search_bench.py`
counts are unchanged by construction — one confirming run is a one-liner, not
a deliverable). What it owes instead, all from the accepts: (1) time-forfeit
count at 8+0.08 from the run-filtered PGN, recorded even if zero;
(2) decisive-game rate on the adopted book vs the 45–60 % band; (3) games/hour
before and after against the 23 games/min baseline (expected ≈ ×1.8 from tc
alone, plus shorter games from decisive openings — record, don't assume);
(4) `tests/test_fastchess_script.sh` green. Expected verdict count: zero.

### 7. Interactions

First pending step of block 0 — every SPRT-owing step below (S093 onward,
~45–55 verdicts) consumes this regime, which is why it leads. S119 is
explicitly measured "at the S105 pressure setting" (DEC-088). S128 owns the
rating gauntlet's time control and is excluded here; S105 touches rating.sh
only for the hash. Verdicts before and after this step are not comparable in
size (DEC-083 consequences) — the step file already prices that. Nothing
pending needs to land first.

### 8. References

- https://official-stockfish.github.io/docs/fishtest-wiki/Running-Fastchess.html — fishtest's exact fastchess flags: book, adjudication, penta.
- https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html — STC/LTC + Hash presets, gainer {0,2} and simplification {-1.75,0.25} nElo bounds.
- https://www.sp-cc.de/uho_2024.htm — UHO 2024 construction, bands, files, draw rates, pgn/epd formats, download, copyright line.
- https://talkchess.com/viewtopic.php?t=83379 — Pohl's UHO 2024 release note: 91.6 % balanced-book draws, band rates, ≥45 % rule.
- https://talkchess.com/viewtopic.php?t=85026 — Pohl: UHO used by all top-engine development since August 2021.
- https://github.com/official-stockfish/books — CC0-1.0 license, one zip per book, UHO_Lichess_4852_v1.epd.zip hosted.
- https://raw.githubusercontent.com/official-stockfish/books/master/README.md — position counts and depths per book (4852: 2 632 036 positions, 2–16 plies).
- https://github.com/robertnurnberg/uhotrack — 4852 provenance: human Lichess games, avg 13.3 plies, ≥23 pieces.
- https://github.com/AndyGrant/OpenBench/wiki/SPRT-and-Fixed%E2%80%90Game-Workloads — the 250 ms time-margin buffer against time losses; Threads/Hash mandatory.
- https://dannyhammer.github.io/engine-testing-guide/sprt.html — 8+0.08 Hash=16 preset; bounds by strength incl. non-regression {-5,0}/{-10,0}; timemargin=250 example.
- https://www.chessprogramming.org/Sequential_Probability_Ratio_Test — bounds-by-strength table ({0,5} top-200, {0,10} otherwise); fastchess recommended for pentanomial stats.
- `fastchess -help` on the installed 1.8.1 binary — epd/pgn books, -srand, timemargin, penta default, log levels, pgnout append/timeleft/latency.
