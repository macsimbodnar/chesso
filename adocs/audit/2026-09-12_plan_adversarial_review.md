# Adversarial strategic plan audit — 2026-09-12

Commit requested: `72f0fad1c791829755455776aaad82bc52c0a1be`.

Scope: the live plan and staged working-tree changes, assessed against the
project goal of an original MIT-licensed CPU engine and against published
testing evidence from other chess-engine developers. The repository was read
only by this reviewer. I made no repository-file or Git-state changes and
started no builds, tests, benchmarks, matches, fits, or training runs. The
coordinator concurrently moved S151 into `plan_current`; that uncommitted
state is noted below.

## Verdict

Two medium findings are open. Neither is a strength judgement; both can make
future measurements or their provenance non-comparable. The direct Open list
and the step files otherwise agreed on the observed queue at the initial
snapshot; during the audit S151 was moved to `plan_current` in an uncommitted
working-tree change while `plan.md` still listed it Open. That transient start
state is not filed as a separate finding. The recently
accepted decisions to defer SMP/network work and to delay the absolute-rating
checkpoint are not repeated as findings: they are explicit owner decisions
(DEC-175, DEC-179, DEC-108).

## Findings

### 2026-09-12_plan_adversarial-F01 — medium — S151 is pre-registered against a retired book and stale calibration

Status: closed — S151 as run, 2026-09-13 (DEC-206): the pre-registration `adocs/data/S151_ltc.sh` pinned the current book `noob_3moves.epd`, re-derived the interval at the 0.2905 pair variance (+/- 11.6 logistic Elo) and the throughput from DEC-190's 2110 over four; the reference pair stayed the historical vector and its parent, stated as such. Previously: open.

Evidence:

```text
$ nl -ba adocs/plan_current/S151_pruning_verdicts_at_longer_control.md | sed -n '306,313p'
308  Lane: a match. Candidate `21b4a21`, reference `3488506`, `32+0.32`, hash per
309  question 2, `UHO_Lichess_4852_v1.epd`, `-repeat`, `-check-mate-pvs`, all 12
310  threads, `-srand` printed if S198 has landed. Throughput assumed **584 games
311  an hour** (section 4).

$ nl -ba fastchess.sh | sed -n '153,154p'
153  book="$repo/books/noob_3moves.epd"
154  book_format="epd"

$ nl -ba adocs/specs.md | sed -n '650,678p'
650  ... the book `books/noob_3moves.epd`, balanced, since 2026-09-12 (S219, DEC-189)
653  -- the unbalanced `UHO_Lichess_4852_v1.epd` before that, and every verdict
654  taken until then stays attributed to it.
674  **The DEC-143 A/A on it, 2026-09-12** ... **2110
675  games an hour** ... pair score variance **0.2905 +/- 0.0184** against ...
677  **0.2905 +/- 0.0184** against S198's 0.2430 +/- 0.0154

$ nl -ba .moltke.local.md | sed -n '74,88p'
74  **2110 games an hour ...** at `8+0.08` on the current book,
75  `noob_3moves.epd` ...
84  S198 measured **2277 games an hour** on the previous book
85  (`UHO_Lichess_4852_v1.epd` ...)
87  ... pair score variance 0.2430 +/- 0.0154 ...
```

The queue still describes S151 as a roughly 3.4-hour, 1000-pair daytime run
(`adocs/plan.md:1028`), but its own pre-registration names the old UHO book and
the old calibration. The current harness and specification explicitly make
`noob_3moves.epd` the regime for new verdicts. The A/A also found materially
different pair variance (0.2905 versus 0.2430), so S151's old `+/-10.5`
logistic-Elo interval (`S151:337-343`) is not a current-book acceptance
interval.

This matters because S151 is not merely historical analysis: its standing rule
says pruning/reduction numbers are re-taken at the longer control before being
banked (`S151:414-425`), and S199 carries that reading into every block
boundary (`S199:31-45`). A result on a retired, demonstrably different book
cannot validate the transfer rule used by the current SPRT regime. Published
Fishtest guidance likewise treats book choice as a source of selection bias and
requires long-control testing for scaling; see [Creating my first test](https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html)
and the [Fishtest FAQ](https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-FAQ.html).

Suggested resolution: amend S151 before it starts. Pin the current book (and
its digest), state whether the reference remains intentionally historical, and
re-derive the pair variance, interval, and throughput at the actual
`32+0.32` control. If the old UHO comparison is retained, label it a separate
historical experiment and state why it informs the noob regime; do not let it
serve as the promised block-boundary control without that decision.

### 2026-09-12_plan_adversarial-F02 — medium — S212's configure-time identity requirement contradicts the repository's provenance invariant

Status: closed, in two halves — S228, 2026-09-14: the generation step is proved and the wiring that runs it is pinned. `tests/test_build_info_freshness.sh` drives `cmake/build_info.cmake` as `cmake -P` over a throwaway git repository, in the fast suite as `test_build_info_freshness`, and asserts the stamp follows the tree from the script's own re-run — the clean sha with its arch and tune values, the dirty suffix and the commit that clears it, an untracked file that is not dirty, an unchanged tree that does not rewrite the header, and a plain directory that stamps "unknown". A sixth case reads the top-level and the `src/` CMake files as text instead of running them, because no run of the script can show that anything invokes it: the build-time custom target exists with `ALL` and passes all four values, no configure-time `execute_process()` or `configure_file()` there names the script or the generated header, and the engine library still depends on the target. Both halves observed red first — the first against a copy that captures the stamp once, the second against copies with `ALL` deleted from the target and the dependency deleted from the library. Previously: closed as to the mechanism — S212, DEC-204 (a), 2026-09-13: the stamp is generated at build time through `cmake/build_info.cmake` on every build, not at configure time; the suggested freshness test (configure once, flip the commit or dirty state, rebuild without reconfiguring, `id name` follows) is **planned — S228** (DEC-206). Previously: open.

Evidence:

```text
$ nl -ba adocs/plan_todo/S212_harness_hygiene.md | sed -n '1,4p'
3  accepts: ... `id name` ... carries the short sha from `git describe --always --dirty`
   ... `CHESSO_ARCH` ... stamped at configure time through a generated header ...

$ nl -ba cmake/build_info.cmake | sed -n '1,11p'
4  # Run as a script (`cmake -P`) by a custom target on every build, not at
5  # configure time. Configure time is the trap this avoids: `cmake -S . -B build`
6  # runs once and a sha captured there goes stale on the next commit ...

$ nl -ba DEV_MANUAL.md | sed -n '3532,3539p'
3532  **The commit is stamped at build time, not at configure time.** ...
3535  `cmake/build_info.cmake` runs on every build through the `chesso_build_info`
3536  target ...
3538  ... a rebuild after a commit picks it up without reconfiguring.

$ nl -ba tools/CMakeLists.txt | sed -n '30,39p'
30  add_custom_target(chesso_build_info ALL
31       COMMAND ${CMAKE_COMMAND} ...
35       BYPRODUCTS ${CHESSO_BUILD_INFO}
38  add_dependencies(tuner chesso_build_info)
```

The acceptance text is the exact failure mode that S077's existing build-info
design was written to prevent. If followed literally, configuring once and
then committing or changing tracked files leaves the generated identity stale
until a reconfigure. `fastchess.sh` would then either reject a valid binary
because its label does not match the current run label, or, if the stale label
is treated as authoritative, attach a run to the wrong source state. The
existing custom target currently feeds the tuner; S212 must extend that
build-time dependency to the engine identity path rather than replace it with
a configure-time value.

Suggested resolution: change S212's acceptance to say “generated at build time
through the existing `chesso_build_info` target,” with explicit dependencies
for the engine and both tune/non-tune configurations. Add a property test that
configures once, changes the commit/dirty state, rebuilds without reconfiguring,
and verifies that `id name` changes. This is especially important because the
step's identity check is intended to make every subsequent match auditable.

## Prior findings reassessed

- 2026-08-21 adversarial F03 (single-control verdicts) is still open in
  substance and is being addressed by S151; F01 above is the newly exposed
  book-regime break in that remedy.
- 2026-08-21 adversarial F04 (no absolute checkpoint) is explicitly deferred
  by DEC-108/DEC-179 and is not a new finding.
- 2026-09-10 adversarial F04–F07 map to S212; F02 above records a remaining
  acceptance contradiction not covered by the existing mapping.
- 2026-09-12 adversarial F01 maps to S223 and remains planned; no duplicate is
  filed here.

## Literature and current-engine context

This section is coordinator-added context, separate from the clean reviewer's
findings. It records what the external evidence implies for the stated goal;
it does not add a repository step or reopen an owner decision.

- [Stockfish 19](https://stockfishchess.org/blog/2026/stockfish-19/), released
  2026-09-05, reports up to 44 Elo over Stockfish 18 in its own tests. The
  release changes the NNUE representation, retires the secondary network, and
  uses quantization-aware training over hundreds of billions of positions
  rescored by Leela. These are current frontier practices, not transferable
  Elo or data-volume promises for Chesso. The external-teacher workflow would
  conflict with Chesso's own-data rule, so it is evidence to account for when
  S029 is eventually authorised rather than a recommendation to import data.
- [Stockfish's NNUE documentation](https://official-stockfish.github.io/docs/nnue-pytorch-wiki/docs/nnue.html)
  describes incremental low-latency CPU inference, integer quantization and
  training objectives that blend search evaluations with game outcomes. A
  future Chesso network needs an explicit inference-cost and playing-strength
  budget; training loss alone cannot establish engine strength. This is a
  methodological recommendation, not a measured Chesso result.
- [Viridithas's disclosed evaluation history](https://github.com/cosmobobak/viridithas#evaluation-development-historyoriginality-hcennue)
  reports successive self-play generations, rescoring by newer versions, and
  some human-game positions. It is a practical example of an iterative data
  loop, while its disclosed PeSTO ancestry means it is not evidence that every
  provenance constraint in Chesso is satisfied.
- [The Torch team's announcement](https://www.chess.com/news/view/torch-chess-engine)
  describes an original engine supported by pytorch-nnue, Cutechess and
  OpenBench, developed by experienced engine authors. This supports keeping
  engine implementation originality distinct from use of external testing
  tools. It does not establish Torch's current rating or data provenance for
  Chesso's purposes.
- [Fishtest's testing methodology](https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html#testing-methodology)
  normally follows a passing short-control functional test with a long-control
  test and performs periodic SMP checks. [Its FAQ](https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-FAQ.html#how-to-compare-opening-books)
  warns that book selection can bias patch estimates and recommends controlled
  fixed-game book comparisons rather than treating different patch runs as a
  causal book experiment. This reinforces F01 and limits what the plan's
  ledger can claim.
- [Baxter, Tridgell and Weaver's TDLeaf paper](https://arxiv.org/html/cs/9901001v1),
  and [Tan and Watkinson Medina's NNUE-dataset study](https://arxiv.org/html/2412.17948v1),
  support self-play/search-generated labels and explicit dataset filtering as
  research directions. The latter studies Xiangqi and uses external-engine
  games; neither supplies a universal corpus size or a ready-made label rule
  for Chesso. S082/S083 should therefore keep held-out error, label stability,
  provenance and CPU inference cost as separate acceptance dimensions.

The evidence supports the existing 3000-without-network milestone as a local
platform target, but it does not make that milestone a proxy for the current
world-leading CPU frontier. A later phase-transition decision should specify
hardware, threads, time controls, books, opponent versions and error bounds;
then compare a self-trained network and parallel search under those resources.
The owner decisions that defer NNUE and SMP remain respected here.

## Limitations

The user prohibited machine workloads, so no dynamic confirmation of the
staged S211 changes or of the harness was possible. `git diff --cached --check`
was clean, but compilation, fast tests, Debug self-play, and `gate_extra.sh`
were intentionally not run. Literature comparison used the official
Stockfish/Fishtest documentation linked above; it is evidence about testing
practice, not a claim that chesso should copy Stockfish's implementation.
