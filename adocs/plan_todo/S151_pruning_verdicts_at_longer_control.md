id:         S151
goal:       a change that moves a pruning or reduction parameter has its verdict re-taken at a control at least four times longer before the number is banked, starting with S085's shipped vector
accepts:    S085's shipped vector is measured against `3488506` at `32+0.32` and `Hash=64` in a fixed 1000-pair match read as an estimate with its 95 % interval -- design (iii) of section 7, DEC-172 -- with the control and its cost stated before the run is committed to, and whatever it returns is recorded -- including a regression, which is the outcome the published record says to expect if it exists; the rule is written where the bounds rule already lives in `fastchess.sh` and `DEV_MANUAL.md`, in its block-boundary form: the longer-control reading is one fixed 1000-pair match at `32+0.32` and `Hash=64` taken beside S199's drift point at each block boundary, so a block's pruning and reduction verdicts are read at the longer control before their magnitudes are banked; the rule is scoped so it does **not** apply to all 45 to 55 pending verdicts, because that roughly doubles the plan's machine budget, and the scoping reason is stated
touches:    fastchess.sh, DEV_MANUAL.md, adocs/data/, adocs/decisions.md, adocs/specs.md
excludes:   a second SPSA run at the longer control, which is a tuning step and not a verification one; re-testing the earlier verdicts S021, S068, S076, S089 or S107, which is a separate decision about history; changing any default, which only the verdict may do and which would be its own step
decisions:  DEC-019, DEC-063, DEC-094
closes:     2026-08-21_adversarial-F03
blocks:
paused_by:
done:

## The evidence, and the one instance that matters most

vondele, `nevergrad4sf`: "the optimal parameters are often time sensitive, i.e.
can be verified to be a gain at the VSTC used for tuning, but regress at STC or
LTC". Stockfish issue #2600: of the last 40 LTC tests with gaining bounds, "23
reds, 16 yellows, 1 green". The fishtest wiki: a long TC "is best to get better
scaling values".

S085's vector moves every affected axis toward more pruning and more reduction --
`RfpMaxDepth` 6 to 15, `LmrDivisor` 225 to 182, `MaxQsearchDepth` 8 to 19,
`AspirationMinDepth` 5 to 2 -- tuned at `2+0.02`. DEC-094 already excluded the
nine `Tm*` axes from S085 on exactly this reasoning, so the failure mode is
known; that it applies to the axes that were tuned is not recorded.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

Every verdict this project has taken was taken at one control, `8+0.08`
(S105, DEC-083). S085 tuned twelve search parameters at `2+0.02`, four times
faster again, and its vector was verified once at `8+0.08`: **H1 accepted at
`elo0=0 elo1=5` in 2946 games and 1 h 15 m 42 s, `Elo 21.02 +/- 9.86`, `nElo
26.81 +/- 12.55`, DrawRatio 35.78 %, `Ptnml(0-2) [121, 289, 527, 363, 173]`**
(`adocs/plan_done/S085_spsa_first_run.md`, "The verification match"). The
published record says a vector tuned at a very short control is the one most
likely to lose at a longer one, and nothing here has ever looked. This step
does two things and no engine code changes:

1. **One run.** S085's shipped vector, the commit `21b4a21` ("Ship S085's tuned
   vector, eleven axes of twelve", 2026-08-21), against `3488506` ("Regenerate
   status after DEC-094 amended S085's goal", 2026-08-20), at a control at
   least four times `8+0.08`. Verified at HEAD: `git rev-parse --short
   21b4a21^` prints `3488506`, so the reference is exactly the parent of the
   vector commit; `git diff --stat 3488506 21b4a21 -- src/` touches only
   `src/search_params.hpp` and a comment in `src/evaluation.hpp`; and `git
   diff 21b4a21 43bf189 -- src/` (S085's completing commit) is empty, so
   `21b4a21` is the candidate and not `43bf189`. The ten defaults that moved,
   by symbol in `src/search_params.hpp`: `MAX_QSEARCH_DEPTH` 8 to 19,
   `RFP_MARGIN` 75 to 63, `RFP_MAX_DEPTH` 6 to 15, `NULL_MOVE_BASE` 2 to 3,
   `LMR_BASE` 75 to 52, `LMR_DIVISOR` 225 to 182, `LAZY_EVAL_MARGIN` 150 to
   184, `ASPIRATION_MIN_DEPTH` 5 to 2, `ASPIRATION_DELTA` 50 to 21,
   `ASPIRATION_MAX_DELTA` 400 to 437. `NULL_MOVE_DIVISOR` came back unchanged
   at 6 and `RFP_MIN_PLY` was held at 3. The run measures the ten jointly and
   cannot attribute anything to one axis.
2. **One rule**, written where the bounds rule lives (`fastchess.sh`'s header
   block "WHICH BOUNDS, AND WHY THE PAIR IS NOT A DETAIL" and `DEV_MANUAL.md`
   "Which bounds"), scoped so it binds a stated class of steps and not all 45
   to 55 pending verdicts, with a decision entry and a sentence in
   `adocs/specs.md`.

The pair for the run is **the owner's open question** (`adocs/status.md`
Parked, DEC-144 Rejected): section 7 prices three designs so it can be
answered; nothing below decides it. `fastchess.sh` cannot express any of the
three today (section 3), so the step's first work is two small harness
additions with their tests.

### 2. The technique as published

**Two-stage testing.** fishtest's standard progression is STC `10+0.1` with
`Hash=16`, then LTC `60+0.6` with `Hash=64`: "If your STC test passes, create a
new test with the Test Type LTC"
(https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html).
The LTC stage exists to reject what does not scale: Stockfish issue #2600
("STC-LTC correlation ?", Alayan-stk-2, 2020-03-28) counted "Out of the 40
last LTC tests with elo-gaining bounds, there has been 23 reds, 16 yellows, 1
green", and Vizvezdenec named what scales non-linearly -- "extensions in
search, countermove pruning and initiative in eval" -- as the reason the LTC
bounds stay strict; vdbergh in the same thread: "Relaxing bounds is very
dangerous given the volume of tests on Fishtest"
(https://github.com/official-stockfish/Stockfish/issues/2600 and its comments
at https://api.github.com/repos/official-stockfish/Stockfish/issues/2600/comments).

**Tuned parameters are the archetype.** vondele's `nevergrad4sf` README: "It
appears that the optimal parameters are often time sensitive, i.e. can be
verified to be a gain at the VSTC used for tuning, but regress at STC or LTC"
(https://github.com/vondele/nevergrad4sf). The fishtest SPSA page: "A short TC
is appropriate to get faster approximations or for checking the tune's
correctness and parameters. A long TC (60+0.6) is best to get better scaling
values" (Creating-my-first-test, above). The wiki's general form, on raising
the null-move `R` at depth > 7: the change "may only be effectively tested on
time controls where this new condition is triggered frequently enough"
(https://www.chessprogramming.org/Engine_Testing) -- a depth-bounded pruning
rule is exercised differently at every control, which is exactly what
`RFP_MAX_DEPTH` 15 and `MAX_QSEARCH_DEPTH` 19 are.

**What a longer control does to the numbers.** The Komodo doubling series on
the wiki: 144 Elo per doubling at the shortest control falling to 51 at the
longest, with draws rising from 49.20 % to 76.63 %
(https://www.chessprogramming.org/Match_Statistics). Stockfish's own data
says the compression is not universal -- "the common wisdom that increased TC
causes elo compression is not always true", SF7 to SF8 gaining about the same
at STC and LTC -- and prices speed at 2.10 Elo per percent at STC against 1.43
at LTC (https://official-stockfish.github.io/docs/stockfish-wiki/Useful-data.html).
Two consequences for reading this run: a gain measured in logistic Elo is
expected to read smaller at `32+0.32` even if it transfers perfectly, and
bounds set in nElo do not care -- "This normalization makes the expected
duration of a test dependent only on the chosen bounds, independent of the
specific draw ratio or opening book used"
(https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-Mathematics.html).
`fastchess.sh` passes `model=normalized`, so every bound below is nElo and the
logistic figure is read off the run's own printed `Elo`/`nElo` pair, as
`DEV_MANUAL.md` "Which bounds" says.

**Run length.** Van den Bergh: "A `SPRT(0,5)` with bounds expressed in
normalized Elo takes about `42k` games to complete (expected worst case)
regardless of the book or the draw ratio" (comment of 2020-12-28,
https://api.github.com/repos/official-stockfish/fishtest/issues/865/comments);
snicolet, same thread, 2021-03-27: as games go to infinity patches above the
interval's midpoint pass and below it fail. The closed form `D / (e1 - e0)^2`
with `D = C^2 (ln 19)^2 = 1046535` at the midpoint and about `639770 / (e1 -
e0)^2` on a bound, `C = 800 / ln 10 = 347.44`, is the one `adocs/testing_strategy.md`
section 1.1 transcribes from "Comments on normalized Elo"
(https://cantate.be/Fishtest/normalized_elo_practical.pdf -- **unverified at
source today**: the PDF fetched but could not be rendered to text; the
transcription is used, and the 42k figure above verifies it at one point).

**Chesso's form.** One stage today. This step adds a second stage for a scoped
class only, at a control ratio of 4 (fishtest's is 6; 4 is the accepts' floor
and what is priced -- `48+0.48` costs 1.5 times more per game again). Whether
the second stage is an SPRT, as fishtest's LTC is, or a fixed-rounds estimate,
as fishtest's regression tests are, is the owner's question in section 7.

### 3. What chesso has today, and where the change plugs in

**`fastchess.sh` at HEAD** hard-wires `tc="8+0.08"` and `option.Hash=16`, and
its candidate is the working tree: `candidate="$repo/build/src/chesso"`,
snapshotted into `snapshot` before the first game. The reference is a git ref
(`REF`, default `HEAD`), resolved to `ref_sha`, built once into
`.ref-builds/$ref_sha` (`ref_dir`, `reference`) as a detached worktree with
`-DCMAKE_BUILD_TYPE=Release` and ccache. The A/A guard refuses when `ref_sha ==
head_sha` and `diff_status == 0` unless `AA=1`. The banner prints
`candidate <sha> <date>[ + uncommitted changes]`, `reference <sha> <date>`,
`tc $tc hash 16 concurrency ...`. `-sprt $sprt` is always passed, so there is
no fixed-rounds mode; `--nonreg` is `elo0=-5 elo1=0` with `rounds=20000`.
Every exit prints `SPRT-RUN-DONE` or `SPRT-RUN-FAILED` (`fail()`, the EXIT
trap). So this run needs three things the script does not have: a **time
control override**, a **candidate that is a git ref** rather than the working
tree, and -- only if the owner picks design (iii) -- **fixed rounds**.

**Add to `fastchess.sh`**, in this order, each with a comment saying why:

- `tc="${TC:-8+0.08}"` and `hash="${HASH:-16}"`, the banner reading both, and
  `option.Hash=$hash` in the `-each` line. The default stays; a run that sets
  either says so in its pre-registration.
- `CAND=<ref>`: when set, resolve `cand_sha="$(git rev-parse --short "$CAND")"`,
  build it exactly as the reference is built -- factor the `if [[ ! -x
  "$reference" ]]` block into `build_ref <sha>` that prints the binary path and
  call it for both sides -- and use that binary as `candidate`. The banner's
  candidate line then carries `cand_sha` and `commit_date "$cand_sha"` with no
  dirty flag: the working tree is not being played. The A/A guard compares
  `cand_sha` against `ref_sha` in this mode (the `diff_status` clause does not
  apply). Name the engine `cand-$cand_sha` so the PGN says what it holds.
  Snapshot it anyway: one code path, and a `.ref-builds/` rebuild during a run
  is as fatal as a `build/` one.
- Do **not** add a fixed-rounds mode to `fastchess.sh`; S199's accepts puts
  its fixed-rounds match in a standalone `adocs/data/S199_drift.sh`, and
  design (iii) follows the same precedent (`adocs/data/S105_calibration.sh`
  is the worked example: no `-sprt`, `-rounds 500` per 1000 games, its own
  terminal marker).

**`tests/test_fastchess_script.sh`** (registered in `tests/CMakeLists.txt` as
`test_fastchess_script`, label `fast`, timeout 60) builds a sandbox repository
with two commits -- `HEAD` and `HEAD~1`, the older one's committer date forced
to 2020-01-02 -- pre-built stub binaries under `.ref-builds/` for both, and a
stub `fastchess` that records that it ran (`make_sandbox`, `run_sandbox`).
Section 6 adds three properties to its eight.

**Documents.** `DEV_MANUAL.md` "Play games" (the usage block), "Which bounds"
(the rule; S182 later adds DEC-143's cost table beside it -- S182 is Open entry
23, after this step, so this step writes prose and S182 writes the table),
"Detach the run, and arm a watcher that outlives the turn" (unchanged);
`adocs/specs.md` INV-6 ("A change is retained only against a measurement")
gains one sentence; the Open item beginning "Measurement capacity is the
binding constraint on the whole plan" gains the control line;
`adocs/decisions.md` gains the entry section 8 proposes.

### 4. Constants and seeds

No engine constant moves; `src/search_params.hpp` is not touched. The run's
own settings, each with its origin:

- **Control `32+0.32`**: the accepts' floor ("at least four times"), form (c)
  in the sense that it is the cheapest admissible point of a stated range;
  fishtest's ratio is 6 (form (a), URL in section 2) and is priced at 1.5x.
- **Hash**: DEC-088's invariant is overwrites per entry, not megabytes -- 16 MB
  at `8+0.08` reproduces the rating list's 60 to 120 overwrites per entry.
  Four times the clock is about four times the nodes per game, so `Hash=16`
  at `32+0.32` is four times the pressure of the regime every verdict was
  taken in, and `Hash=64` holds it (form (b), DEC-088's own arithmetic;
  fishtest's LTC runs 64 at 6x). Owner's question 2.
- **Games per hour at `32+0.32`, about 584**: `2337 / 4`, the figure DEC-144
  used. Derivation: S105's calibration measured 17.9 s a game and 98.0 plies
  at `8+0.08` on 12 threads (`DEV_MANUAL.md` "What a verdict costs,
  measured"); time management spends a fixed share of the clock plus half the
  increment, so seconds a ply scale with the control, and **assuming plies a
  game roughly constant** a game takes about 71.6 s and 12 threads give about
  600 an hour. Two things push it down: the draw rate rises with the control
  (the Komodo series, section 2) and the draw adjudication needs eight moves
  inside 10 cp after move 40, so games may run longer. The UHO book shortens
  games (S105 measured x1.20 from it) and is kept. The first hour's `Finished
  game` count is the check; the ceiling is set from the pricing, not from it.
- **Bounds pair, rounds cap, ceiling**: section 7, owner's question 1.

### 5. Interactions and traps

- **S148 sits before this step in the Open order (DEC-144) and moves
  `RFP_MAX_DEPTH`, one of the ten axes.** The two do not interfere: S151
  compares two frozen commits, so whatever S148 ships at HEAD changes nothing
  in S151's run, and S148's `8+0.08` verdict against HEAD is not touched by
  S151's result. What S151 cannot do is attribute: an H0 here says the ten
  moved jointly regress at the longer control, not which one. S151's rule
  binds S148 only if S151 lands first, which the order says it will not;
  whether S148's already-taken verdict then owes a retroactive re-take is the
  same decision the excludes defers for S021, S068, S076, S089 and S107, and
  is written as one in the stamp, not taken.
- **Both commits predate S167.** `21b4a21` and `3488506` carry the dead
  `MAX` constant that Apple clang rejects under `-Werror` (`672e35a`); on the
  Linux workstation with gcc they build, on the MacBook they do not. The run is
  a workstation run.
- **Both commits predate S147, S170 and S171**, so `-check-mate-pvs` will
  print `Incomplete mating PV` lines from **both** sides. They are not a
  finding against either engine; count them per side from the log and record
  the two counts, and do not stop the run on them.
- **`fastchess.sh`'s dirty-tree logic is about the working tree** and the
  working tree is not being played. With `CAND` set the snapshot, the dirty
  flag and the guard all key on `cand_sha`; a run that reports `+ uncommitted
  changes` beside a ref candidate is a bug in the change.
- **nElo bounds exclude fewer logistic Elo at a higher draw rate.** `{-5,0}`
  read 3.54 logistic at 44.75 % draws and 3.99 at 35 to 36 %; at `32+0.32`
  the draw rate is expected higher, so the excluded logistic regression is
  smaller than at `8+0.08`. Quote the bound in nElo and the ratio off the run.
- **The rounds cap can end a `{-5,0}` run before the midpoint case.**
  `--nonreg`'s `rounds=20000` is 40000 games, about 68.5 h at 584 an hour,
  against 41861 games (71.7 h) at the midpoint -- a run that reaches the cap
  ends as "no verdict" by construction, as S148's guide already prices for
  its own run. Say so in the pre-registration.
- **DEC-143 needs the workstation's A/A (S198, Open entry 6) before this
  verdict**; it has not been taken as of this writing (`adocs/status.md`
  Parked).
- **WATCHERS and PLAN.** A `{-5,0}` run can hold the machine for three days.
  Agent-only entries may run beside it (DEC-144's reading rule) and no `src/`
  change may. The ceiling is wall-clock while the machine is awake; the
  workstation does not hibernate, but the pid exit is what catches a killed
  run.
- **An SPRT's point estimate is biased upward (DEC-063)**; a fixed-rounds
  estimate is not. If the owner's question is "how much of the +21 survives",
  only design (iii) answers it as a number; if it is "does it regress", (i)
  or (ii) answers it as a verdict. The two are different questions and the
  accepts asks the second.
- **The rule's cost is set by its design, not by its scope alone.** Section 7
  prices it: 13 to 16 bound verdicts at a `{-5,0}` re-take each cost more
  than the whole plan; the same 13 at a fixed 1000 pairs cost about 44 h.

### 6. Tests

No search rule, no `make_move`, no generator: no guard test, no mutant, no
Debug self-play and no INV-6 node-count run are owed, and the stamp says so in
one line. The tests are the harness's.

**Three properties for `tests/test_fastchess_script.sh`**, appended after
property 8, the count in the closing message moved from 8 to 11. Each is
observed red first by running the test against `git show HEAD:fastchess.sh`
saved to a file (the script's argument exists for exactly this), then green
against the changed script.

    # 9. TC=<control> reaches the engines, and the banner says so.
    #    The stub records "$@" into fastchess_argv, so the assertion reads the
    #    argument fastchess received and not a banner echo.
    tc_dir="$(make_sandbox "$script_under_test")"
    TC=32+0.32 run_sandbox "$tc_dir" HEAD~1            # extend run_sandbox to pass TC/HASH/CAND through, unset otherwise
    grep -qx 'tc=32+0.32'   "$tc_dir/fastchess_argv"   || fail "TC was not passed to fastchess"
    grep -qE '^tc 32\+0\.32 ' "$tc_dir/out.txt"        || fail "the banner does not print the override"
    # 10. CAND=<ref> plays the ref's .ref-builds binary, not build/src/chesso.
    cand_dir="$(make_sandbox "$script_under_test")"     # clean tree; HEAD and HEAD~1 pre-built
    CAND=HEAD REF=HEAD~1 run_sandbox "$cand_dir"
    grep -qE "^candidate  $head_sha  $(date +%F)\$" "$cand_dir/out.txt"   || fail "candidate line is not the ref, dated, undecorated"
    grep -qE "^reference  $older_sha  2020-01-02\$"  "$cand_dir/out.txt"   || fail "reference line wrong"
    grep -q  "cmd=.*\.ref-builds/$head_sha/"        "$cand_dir/fastchess_argv" || fail "candidate binary is not the ref build"
    # 11. CAND resolving to REF on any tree is refused with the A/A sentence,
    #     and AA=1 lets it through -- the guard keys on cand_sha, not head_sha.
    same_dir="$(make_sandbox "$script_under_test")"
    echo change >> "$same_dir/tracked.txt"              # dirty tree must NOT rescue it
    CAND=HEAD~1 REF=HEAD~1 run_sandbox "$same_dir"
    grep -q 'both sides are the same build' "$same_dir/out.txt" || fail "same-ref CAND/REF was played"

Property 10's `cmd=` grep is the one that stops a mutant that resolves `CAND`
for the banner and still plays `build/src/chesso`, which is the mistake the
`REF` machinery was built to prevent on the other side (DEC-020).

The gate: `cmake --build build -j12 && ctest --test-dir build -L fast
--output-on-failure && cmake --build build-tune -j12 && ctest --test-dir
build-tune -L fast --output-on-failure && ./clang-format.sh --check` (`-j12`
on the workstation). Goldens: none touched.

### 7. Measurement

Lane: a match. Candidate `21b4a21`, reference `3488506`, `32+0.32`, hash per
question 2, `UHO_Lichess_4852_v1.epd`, `-repeat`, `-check-mate-pvs`, all 12
threads, `-srand` printed if S198 has landed. Throughput assumed **584 games
an hour** (section 4). Every figure below is from `D / (e1 - e0)^2`, `D =
1046535` at alpha = beta = 0.05 (`C^2 (ln 9)^2 = 582800` at 0.10), and
`639770 / (e1 - e0)^2` on a bound.

| design | asks | games, truth at midpoint | hours | games, truth on a bound | hours |
|---|---|---|---|---|---|
| (i) SPRT `{-5, 0}` | not a regression of 5 nElo | 41861 | **71.7** | 25591 | 43.8 |
| (ii) SPRT `{-10, 0}` | not a regression of 10 nElo | 10465 | 17.9 | 6398 | 11.0 |
| (ii') SPRT `{-5, 5}` | is the sign positive | 10465 | 17.9 | 6398 | 11.0 |
| (iii) fixed 1000 pairs | an estimate with its interval | 2000 | **3.4** | -- | -- |

(i) is the harness's own `--nonreg` mode and the resolution every other
verdict has: it answers the accepts' question -- does the vector regress at
four times the control -- at 5 nElo and alpha 0.05. Its 72 h is the case
where the truth sits at -2.5 nElo. If S085's gain transfers even in part the
run is far shorter -- the normal-approximation drift `N = ln 19 * C^2 / ((e1 -
e0) * (e - midpoint))`, an approximation and named as one, gives about 9500
games (16 h) at a true +5 nElo, 5700 (10 h) at +10, 4100 (7 h) at +15 and
3200 (5.4 h) at +20; the `8+0.08` verdict read +26.81 nElo biased upward. So
(i) is 5 to 16 h if the published warning does not bite here and 44 to 72 h
if it does. (ii) is a quarter of the cost and lets a 6 to 9 nElo regression
pass as "not a regression", which is the precision the CPW table assigns to
engines outside the top 200
(https://www.chessprogramming.org/Sequential_Probability_Ratio_Test: `[0, 5]`
/ `[-5, 0]` top 200, `[0, 10]` / `[-10, 0]` all others). (ii') is the same
cost and asks whether any of the gain survives, which is a different question
from the accepts'. (iii) reads the effect as a number: at S105's pair-score
variance of 0.2395 (`8+0.08`, UHO; 0.2343 on the old regime; both on
`adocs/data/S105_pairs.py`'s 0-to-2 pair scale) 1000 pairs give a per-game
score standard error of 0.0077, a 95 % half-width of 0.0152 in score, **about
+/- 10.5 logistic Elo** at 694.8 Elo per unit score near 50 %, and about +/-
15 nElo -- the run's own printed `Elo`/`nElo` intervals are the figures to
quote, the pair variance at the new control being itself an output. (iii)
separates "the +21 transfers" from "it is gone" and cannot separate -4 from 0;
DEC-143 already says an estimate is not a verdict, and `adocs/status.md` notes
it would change the accepts' "re-tested". Its +/-8 there is superseded by the
+/-10.5 derived here.

**The rule's standing cost follows from the same table.** Section 8's scope
binds 13 verdicts (16 with extensions). At (i) each re-take averages about
four times the ledger's 4 h 40 m mean per verdict, about 19 h -- 13 of them
is about 240 h, more than the whole plan; at (iii) 13 of them is about 44 h,
roughly +20 % on the plan's `plan.md` "What this costs" range. The accepts'
"does not roughly double the plan's machine budget" is met by the scope
**and** a fixed-rounds re-take, or by a scope narrower still.

**Pre-registration**, `adocs/data/S151_ltc.sh`, header written before the
first game in the `adocs/data/S165_sprt.sh` / `S148_sprt.sh` shape: both shas
with their dates and the `git rev-parse --short 21b4a21^` check; the ten
axes from and to; S085's `8+0.08` block verbatim beside it; control, hash,
book, seed; the pair with its worst-case games and hours and the rounds cap;
the abort rule -- stop only for a forfeit rate over 1.0 % on a side
(`python3 tools/forfeit_report.py <dir>/games.pgn --max-pct 1.0`), the two
sides' `Incomplete mating PV` counts being recorded and not a stop; and the
three readings. For (i) or (ii) the body is `cd /home/max/ws/chesso || { echo
"SPRT-RUN-FAILED: cd" >&2; exit 1; }` then `exec env CAND=21b4a21 REF=3488506
TC=32+0.32 HASH=<n> ./fastchess.sh --nonreg` (with the pair edited in the
block for (ii), and the edit recorded, as S021 and S076 did). For (iii) the
body is `adocs/data/S105_calibration.sh`'s `run_one` shape with `-rounds 1000
-repeat` and no `-sprt`, both binaries from `.ref-builds/`, its own
`S151-LTC-DONE` / `S151-LTC-FAILED` markers.

The readings, for (i) and (ii):

- **H1** -- the vector is not a regression of `|e0|` nElo at `32+0.32`
  (logistic figure from the run's pair). The `8+0.08` verdict stands, the
  magnitude is not established, and the rule is written with this as its
  first instance and a null finding.
- **H0** -- the vector regresses `|e0|` nElo or more at four times the
  control: the outcome the record says to expect. Recorded here, in `specs.md`
  and in DEC-019's ledger. **Nothing in the engine changes** (excludes): which
  axis, and whether to revert one, hold one or re-tune at the longer control,
  is a new step opened from the stamp. S148's own verdict on `RFP_MAX_DEPTH`
  is independent evidence and neither result overrides the other.
- **No verdict at the cap** -- recorded as no verdict with the final `LLR`,
  `Elo` and `nElo`; the rule is written regardless, since its wording does
  not depend on the outcome. No re-run at other bounds without a decision.

For (iii) the reading is the interval: its position relative to zero and to
the `8+0.08` +21.02, quoted with both intervals and never as a verdict.

Launch and watch, workstation:

    nohup adocs/data/S151_ltc.sh > .tuning/sprt_s151.log 2>&1 &
    echo $! > .tuning/sprt_s151.pid
    # the WATCHERS poll loop verbatim, RUN_LOG=.tuning/sprt_s151.log, RUN_PID
    # from the pid file, armed through Monitor with persistent: true. Ceiling
    # 2x the design's worst case: 518400 s for (i), 129600 for (ii), 25200 for (iii).
    # The marker pattern is SPRT-RUN-(DONE|FAILED) for (i)/(ii), S151-LTC-(DONE|FAILED) for (iii).

Records: `adocs/data/S151_ltc.sh`, its stdout as `adocs/data/S151_ltc.log`,
a row each in `adocs/data/README.md`; the games are not committed (the
`S068` run-1 reasoning: every per-game result is in the log), unless the
owner wants them, as `S021_sprt.pgn` was. The step file gets the verdict
block -- `LLR`, `Elo`, `nElo`, games, wall time, draw ratio, `Ptnml`, forfeits,
both `Incomplete mating PV` counts, measured games an hour against the 584
assumed.

### 8. Completion checklist

- The gate (section 6) green in both builds; `python3 tools/plan_prose_check.py
  --touches` and `--params` report nothing new.
- **No `Bench:` line and no `No functional change` line**: the commits touch no
  `src/` file (DEC-140 binds `src/` commits only).
- **`fastchess.sh` header**, one paragraph under "WHICH BOUNDS", proposed
  wording, the scope in its one sentence: *"A change whose verdict moves a
  pruning or reduction parameter -- a margin, depth bound, reduction
  coefficient or divisor in `src/search_params.hpp` that decides whether a
  node or move is searched at all or how much shallower (`RFP_*`,
  `NULL_MOVE_*`, `LMR_*`, `MAX_QSEARCH_DEPTH`, and every constant the pruning
  steps add) -- has its verdict re-taken at `TC=32+0.32` against the same
  reference before its magnitude enters `plan.md`'s cost or Elo arithmetic;
  the `8+0.08` verdict decides whether it ships, the longer control decides
  what is written about it, and a regression there is a finding that opens a
  decision, not an automatic revert. It does not apply to evaluation weights,
  ordering tables, `TM_*`, hash or table layout, or a change proved
  behaviour-neutral on node counts. S151, DEC-<nnn>."* Binds: S109, S098 (three
  verdicts), S114, S116, S113, S091, S095, S112, S022 (two), S127's
  verification SPRT, and S148 only if this lands first -- 13 verdicts; S097
  (two) and S188 are extensions, which the accepts' letter omits and
  Vizvezdenec's list names, owner's question 4. Does not bind: S159, S024,
  S023, S025 (ordering), S115 (window schedule; in S085's vector, but not a
  prune), S132 (`TM_*`, DEC-094's family, already measured at an increment
  control by its accepts), S131, S039, S117 to S120, S121 to S125, S101, S102,
  S133 to S136, S020, S030, S032, S042, S055.
- **`DEV_MANUAL.md`**: "Play games" usage block gains `TC=32+0.32`, `HASH=64`
  and `CAND=<ref>` lines with one clause each; "Which bounds" gains the rule
  paragraph and the priced designs table of section 7 (S182 adds the general
  cost table later and reads this beside it). "Detach the run" is unchanged.
- **`MANUAL.md`**: checked, no change -- no UCI surface moves.
- **`adocs/specs.md`**: INV-6 gains "A verdict that moves a pruning or
  reduction parameter is re-taken at a control at least four times `8+0.08`
  before its magnitude is banked (2026-xx-xx, S151)"; the Open item on
  measurement capacity gains the measured games an hour at `32+0.32`.
- **`adocs/decisions.md`**, next free id, proposed: *Tags: measurement, sprt,
  time-control, s085, s151. Context: every verdict at one control; S085 tuned
  at 2+0.02, verified at 8+0.08; the record (#2600, nevergrad4sf) says the
  transfer is poor for exactly this class. Decision (by the owner): the scope
  sentence above; the run's design chosen (i, ii or iii) with its cost; hash
  16 or 64 at the longer control; whether the earlier verdicts and S148 owe a
  retroactive re-take (excludes: no, unless decided). Why: one line -- the
  cost of the rule is its design times its scope, and both are now written.
  Rejected: binding every verdict (doubles the budget); no rule (the finding
  F03 stays open); a second SPSA at the longer control (S127's, and tuning
  not verification).*
- **Stamp**: shas and dates, design, control and hash, games, wall time,
  measured games an hour, the verdict block, the rule's landing sites, the
  three tests' reds observed, "no `src/` change, no Bench line", and the
  question S148 raises for the excludes.
- `plan.md` (out of Open, Done list), `status.md` (the Parked question closed)
  and the decision entry go through the coordinator (PLAN rule).

### 9. Sources read

- `adocs/plan_done/S085_spsa_first_run.md` -- the vector, the verification
  block, the `REF=HEAD~1` invocation and its numbers.
- `adocs/plan_done/S068_rfp_margin_retune.md` and DEC-063 -- the two-bounds
  lesson and the pooled-estimate correction.
- `adocs/plan_done/S105_sprt_harness_regime.md`, `adocs/data/S105_calibration.sh`,
  `adocs/data/S105_pairs.py` -- fixed-rounds A/A shape, 17.9 s a game, pair
  variance 0.2343 / 0.2395 on the 0-to-2 scale.
- DEC-019, DEC-083, DEC-088, DEC-094, DEC-143, DEC-144, DEC-145 in
  `adocs/decisions.md`; `adocs/audit/2026-08-21_adversarial.md` F03.
- `adocs/testing_strategy.md` sections 1.1, 2.1, 3.4, R11, R14 -- the formula,
  the scaling record, the fixed-rounds interval.
- `fastchess.sh`, `tests/test_fastchess_script.sh`, `tests/CMakeLists.txt`,
  `DEV_MANUAL.md` "Play games" to "One trap when reading a fastchess.sh result",
  `adocs/data/S130_sprt.sh`, `S108_sprt.sh`, S148's guide section 7.
- `git log`/`git diff` at HEAD for `3488506`, `21b4a21`, `43bf189`, `672e35a`.
- https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html
  and https://raw.githubusercontent.com/wiki/official-stockfish/fishtest/Creating-my-first-test.md
  -- STC/LTC controls and hash, the LTC-after-STC sentence, the SPSA scaling sentence.
- https://github.com/official-stockfish/Stockfish/issues/2600 and
  https://api.github.com/repos/official-stockfish/Stockfish/issues/2600/comments
  -- "23 reds, 16 yellows, 1 green"; Vizvezdenec's list; vdbergh on bounds.
- https://github.com/vondele/nevergrad4sf -- the time-sensitivity sentence.
- https://www.chessprogramming.org/Engine_Testing -- the "triggered frequently
  enough" sentence.
- https://www.chessprogramming.org/Match_Statistics -- the Komodo doubling series.
- https://www.chessprogramming.org/Sequential_Probability_Ratio_Test -- the bounds table.
- https://official-stockfish.github.io/docs/stockfish-wiki/Useful-data.html --
  compression not universal; 2.10 / 1.43 Elo per percent.
- https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-Mathematics.html
  -- duration independent of draw ratio.
- https://api.github.com/repos/official-stockfish/fishtest/issues/865/comments
  -- the 42k sentence (2020-12-28), the midpoint remark, the lower-bound purpose.
- https://cantate.be/Fishtest/normalized_elo_practical.pdf -- **unverified**
  (fetched, not renderable here); formula taken from `adocs/testing_strategy.md`.

### 10. Questions deferred to the owner

1. **The design of S085's re-test**: (i) `{-5, 0}`, 44 to 72 h if the record
   is right and 5 to 16 h if the gain transfers; (ii) `{-10, 0}`, 11 to 18 h,
   coarser; (iii) 1000 pairs, 3.4 h, an estimate at +/- 10.5 Elo that changes
   the accepts' "re-tested". Section 7 is the pricing; this guide recommends
   nothing.
2. **Hash at `32+0.32`**: 16 (one variable moved, four times the pressure) or
   64 (DEC-088's invariant held, fishtest's LTC practice, two variables
   moved).
3. **The rule's standing re-take design**, separately from S085's run: the
   same SPRT pair (about 240 h over the 13 bound verdicts) or a fixed 1000
   pairs (about 44 h). The accepts' budget sentence is met only by the second
   or by a narrower scope.
4. **Scope edge cases**: extensions (S097, S188), which the record names and
   the accepts' letter omits; S115's aspiration triple, in S085's vector but
   not a prune; whether S148's verdict, taken before this lands, owes a
   retroactive re-take (the excludes' reading says no unless decided).
5. **Where the rule lives**: `touches` names `fastchess.sh`, `DEV_MANUAL.md`,
   `specs.md` and `decisions.md` but not `AGENTS.md`; DEC-143's sentences went
   into the MEASUREMENT rule. Adding one there is a `/moltke:rules` change
   recorded as a decision.
6. **Control ratio**: exactly four (`32+0.32`, priced) or fishtest's six
   (`48+0.48`, 1.5x the hours in every row above).

## Amended 2026-09-11, DEC-172: the design is (iii), and the rule is the block-boundary form

The owner's question of section 10 is answered under the 2026-09-11
delegation. **Design (iii)**: a fixed 1000-pair match at `32+0.32`, `Hash=64`
(question 2: DEC-088's pressure invariant held, fishtest's LTC practice),
control ratio exactly four (question 6), about 3.4 hours at the 584 games an
hour section 4 assumes -- checked in the run's first hour -- and read as an
estimate with the run's own printed `Elo`/`nElo` intervals, never as a verdict.
The accepts' "re-tested" became "measured ... read as an estimate" above, which
is the change section 7 said (iii) would make. The standing rule (question 3)
is the fixed-rounds form at block boundaries, beside S199's drift point, and
not a re-take per verdict: 13 re-takes at `{-5, 0}` cost more than the whole
plan and the accepts' budget sentence is met only by this scope. Extensions
(S097, S188) are inside the reading's scope because the record names them
(question 4); S148's already-taken verdict owes no retroactive re-take, per
the excludes; the rule lives in `fastchess.sh`, `DEV_MANUAL.md`, `specs.md`
and the decision, not in `AGENTS.md` (question 5). Design (i) -- 44 to 72
hours of the binding constraint for a number that adds no strength -- is the
rejected option and DEC-172 says why. The harness additions of section 3
(`TC`, `HASH`, `CAND`) are still this step's first work; their defaults leave
the regime byte-identical, and S212's A/A, which follows this step in the
order, is the DEC-143 calibration that covers them.
