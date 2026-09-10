# Testing strategy: how chesso decides that a change is an improvement

**Purpose.** Input document for the plan, written 2026-09-04 at `5cffb70`. It
surveys what the literature and the strongest open engines do to test a chess
engine, states where chesso's instruments stand against that, and lists what
to change, ranked by what it buys for what it costs. The measurements behind
the assessment are `adocs/audit/2026-09-04_test_review.md` and its evidence
directory `adocs/data/2026-09-04_test_review/`; this file does not repeat
them. Like `eval_tuning_strategy.md` it is an input: every recommendation is
the owner's to accept, refuse or reshape, and a step exists only once one is
written.

**Sources.** Every figure and every claim about another project carries the URL
it was read at, fetched on 2026-09-04, or the word *unverified*. Paraphrased,
not copied; the same rule as `adocs/data/2026-09-04_plan_review_literature_check.md`.
Nothing here is a chess judgement.

---

## 0. Summary

Chesso has five instruments and the audit measured four of them.

| instrument | what it decides | state |
|---|---|---|
| the fast suite, 27 targets, about 280 doctest cases | correctness of the board, the search rules, the table, the UCI surface, the tuner, the scripts | **31 of 32** injected bugs caught; 98.9 % line coverage of the search; not flaky over 35 runs. Two invariants live in assertions the gate compiles out; the null-move and reduction guards are covered by one golden count and nothing else |
| `tools/search_bench.py`, INV-6 | that a change meant to be neutral is neutral | run by hand, compared by eye, recorded nowhere the gate can read; blind to 12 of the 33 injected bugs, and the only instrument that saw a one-ply floor drift the mate gates missed |
| `fastchess.sh`, SPRT at 8+0.08, nElo bounds, pentanomial | that a change that alters play gains or does not lose | the statistics match fishtest's; the regime matches the engines the plan reads from; calibrated once, on the previous machine |
| `rating.sh`, the gauntlet | how strong the engine is on the CCRL scale | one run, 2559 +/- 25 soft, deferred to near 3000 by DEC-108 |
| the Debug binaries, sanitizers, deep perft | the invariants and memory safety | by hand, on request, in no gate |

Against the field the gaps are the same three the 2026-08-14 review named and
one it did not: no bench signature in the gate (every surveyed CI has one),
no automatic run of the assertions or sanitizers (Stockfish self-plays a
Debug binary in CI), no reproducibility test across `ucinewgame` (Stockfish's
found a table-ageing bug an SPRT could not), and -- new -- a suite whose
sensitivity to search changes comes mostly from golden numbers that every
legitimate search change will also move.

Fourteen recommendations follow in section 4, ranked. The first six are
machine-free and close the audit's findings; one needs 26 minutes of the
machine on mains; the rest are decisions about practice. Nothing here changes
a bound, a time control or a book: the SPRT regime is right and the cheapest
improvement to it is to write down what it costs.

---

## 1. Deciding strength: the statistics, and where chesso stands

### 1.1 What the field does

**The SPRT and its bounds.** The test is Wald's (Ann. Math. Statist. 16(2),
1945, https://projecteuclid.org/journals/annals-of-mathematical-statistics/volume-16/issue-2/Sequential-Tests-of-Statistical-Hypotheses/10.1214/aoms/1177731118.full):
accumulate the log-likelihood ratio and stop when it leaves
`[log(beta/(1-alpha)), log((1-beta)/alpha)]`, which at alpha = beta = 0.05 is
+/-2.944 -- the `(-2.94, 2.94)` every `fastchess.sh` run prints. fastchess
(`sprt.cpp`, https://github.com/Disservin/fastchess) and fishtest
(`stats/sprt.py`, https://github.com/official-stockfish/fishtest) compute the
same bounds.

**GSPRT, pentanomial, pairs.** Neither framework runs a plain SPRT: the
unknown parameters under H0 and H1 are replaced by their maximum-likelihood
estimates (Van den Bergh, "A practical introduction to the GSPRT",
https://www.cantate.be/Fishtest/GSPRT_approximation.pdf; fishtest's summary at
https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-Mathematics.html).
The outcome model is the five-outcome distribution of a *pair* -- the two
games on one opening with colours reversed, scored 0, 1/4, 1/2, 3/4, 1 --
because the return game's result is correlated with the first through the
opening's bias; the trinomial model "greatly underestimates variance" for
paired games (https://www.chessprogramming.org/Match_Statistics), and the
pentanomial one "leads to a substantial saving of testing resources"
(fishtest mathematics page above). The difference between the two variances is
itself an estimate of the book's bias (https://github.com/vdbergh/pentanomial).
**Chesso does this**: `-repeat`, `-report penta` (fastchess's default), and
`adocs/data/S105_pairs.py` and `S024_pair_stats.py` read pairs back out of a
PGN and were validated against fastchess's own printout (S024).

**Normalized Elo.** nElo is the score's t-statistic scaled by
C = 800/ln 10 = 347.44: `e_n = C * (mu - 1/2) / sigma`, where sigma is the
per-game score standard deviation, with the pair variance times sqrt 2 in the
pentanomial case (Van den Bergh, "Comments on normalized Elo",
https://cantate.be/Fishtest/normalized_elo_practical.pdf). For a balanced book
`e_n = e_logistic / sqrt(1 - d)` with d the draw ratio, so 5 nElo is 5.00 /
3.54 / 2.24 logistic Elo at 0 % / 50 % / 80 % draws. fishtest expresses its
bounds in nElo so that a test's expected length depends "only on the chosen
bounds, independent of the specific draw ratio or opening book used"
(fishtest mathematics page). **Chesso does this** (S157, DEC-063): the
project's own conversions, 3.54 to 3.99 logistic Elo per 5 nElo across six
runs, sit near the 3.7 to 3.8 the balanced-book formula gives for 42 to 45 %
draws; the residual is the unbalanced book's pair correlation, which that
formula ignores and the pentanomial statistic does not.

**What a pair of bounds costs.** The same paper gives the expected length of
a test whose true effect sits at the midpoint of the interval -- the worst
case -- as `D / (e1 - e0)^2` games with `D = C^2 * (ln 19)^2 = 1046535` at
alpha = beta = 0.05, and about `639770 / (e1 - e0)^2` when the truth sits on
one of the bounds. Van den Bergh's own gloss: an SPRT(0,5) in nElo "takes
about 42k games to complete (expected worst case) regardless of the book or
the draw ratio" (comment on fishtest#865, 2020-12-28,
https://github.com/official-stockfish/fishtest/issues/865). Applied to
chesso's pairs at the throughput the ledger records for this machine --
seven runs since S105, **2328 to 2346 games an hour, mean 2337**, read from
the stamps S182 tabulates:

| pair | expected games, truth at the midpoint | at 2337 games/h | truth on a bound | at 2337 games/h |
|---|---|---|---|---|
| `{0, 5}` and `{-5, 0}` | 41861 | 17.9 h | 25591 | 10.9 h |
| `{-5, 5}` | 10465 | 4.5 h | 6398 | 2.7 h |
| `{0, 10}` at alpha = beta = 0.10 | 5828 | 2.5 h | -- | -- |
| fishtest STC `{0, 2}` | 261634 | 111.9 h | 159942 | 68.4 h |

The ledger agrees with the table: the seven runs average 4 h 40 m, which is
about 10900 games, between the bound case and the midpoint case of the
`{0,5}` / `{-5,0}` pairs they mostly ran, and DEC-063's 6 h 36 m no-verdict
run sits inside the midpoint case. **The plan's 45 to 75 minutes per verdict
is a number these pairs produce only when the true effect sits far outside
the interval** -- the +40 to +120 class, which the ledger says one pending
step (S109) is in; S182 is re-deriving the cost line from the ledger and
should carry this table beside it, because the table prices a pair *before*
the run and the ledger only after.

**The point estimate is biased.** "The SPRT Elo estimates are only unbiased if
one takes *all* patches into account, both passed and non-passed ones"
(fishtest FAQ, https://github.com/official-stockfish/fishtest/wiki/Fishtest-faq).
fishtest reports a median-unbiased estimate from a Brownian model of the LLR
path (`stats/sprt.py`, formula (6.1) of
https://www.cantate.be/Fishtest/brownian_approximation.pdf); fastchess prints
the naive estimate. **Chesso already refuses to report it as the effect size**
(DEC-063, S068's pooled figure falling from +12.18 to +5.02, S157), which is
the right practice; what it does not do is the sum-of-passed-patches check
the FAQ warns about, and section 3.4 below is that.

**Bounds practice.** fishtest today: STC 10+0.1 Hash 16 at `[0, 2]`, LTC
60+0.6 Hash 64 at `[0.5, 2.5]`, simplifications `[-1.75, 0.25]` at both, all
nElo (`server/fishtest/templates/tests_run.html.j2` and `views.py` in the
fishtest repository; https://github.com/official-stockfish/fishtest/wiki/Creating-my-first-test).
The CPW table chesso took its pairs from -- top 200 `{0,5}` / `{-5,0}`,
everyone else `{0,10}` / `{-10,0}` -- is at
https://www.chessprogramming.org/Sequential_Probability_Ratio_Test. OpenBench
engines carry their own: Ethereal `[0, 3]`, Berserk STC `[0, 2]` and LTC
`[0, 2.5]` with simplifications at `[-2, 0]`, Alexandria `[0, 3]` and
`[-3, 1]` (`Engines/*.json`, https://github.com/AndyGrant/OpenBench). On
straddling: as games go to infinity, patches above the interval's midpoint
pass and below it fail (snicolet, fishtest#865); the lower bound exists "to
make elo neutral STC tests easier to pass and elo neutral LTC tests more
difficult" (Van den Bergh, same issue); and "Relaxing bounds is very dangerous
given the volume of tests on Fishtest" (Van den Bergh, Stockfish#2600,
https://github.com/official-stockfish/Stockfish/issues/2600). **Chesso's
DEC-063 is this rule, arrived at from its own runs.**

**Multiple testing and two stages.** "Generally, four or five tries is the
limit" for re-running a failed test (fishtest FAQ). Stockfish#2600 counted
"23 reds, 16 yellows, 1 green" among the last 40 gaining LTC tests, i.e. a
green STC rarely survives LTC, and the maintainers kept the stricter LTC on
purpose so that badly-scaling patches fail there. From the run-length
formula, an exactly-zero patch passes fishtest's STC pair 5 % of the time and
its LTC pair about 1.2 %; both stages about 0.06 %. **Chesso runs one stage.**
At its throughput a second stage at four times the control costs four times
the games-per-hour, which is why S151 scopes the longer-control confirmation
to pruning and reduction parameters only; that scoping is right and the
statistics above say what it forgoes: a single `{0,5}` stage passes a true
zero one run in twenty.

**Fixed games.** fishtest's regression tests are fixed-length -- 60000 games
at LTC against the last release, book `UHO_Lichess_4852_v1.epd` since
2026-02-04 (https://github.com/official-stockfish/Stockfish/wiki/Regression-Tests)
-- and are for tracking progress, not for verdicts; "1 in 20 tests will be an
outlier" (FAQ). For a parameter exploration the FAQ recommends a fixed 20000
games rather than an SPRT.

**A/A.** "The *first* test that you run should be a 'sanity check' of your
baseline engine vs itself" (https://dannyhammer.github.io/engine-testing-guide/strength-testing.html).
An SPRT between identical engines passes a `{0,u}` pair with probability
alpha exactly, so a *fixed-rounds* A/A is the calibration and an SPRT A/A is
not. **Chesso did this once** (S105, two runs of 1000 games, on the previous
machine) and has the `AA=1` switch (S160) with no run recorded on this one.

### 1.2 Where chesso stands

Right, and confirmed at source: pairs, pentanomial, nElo, the straddle rule,
the refusal to report the stopping estimate, the pre-registered reading of
every outcome, the attributable reference build and the snapshotted candidate
(DEC-020), the terminal markers and the four-exit watcher. Three things the
sources add:

1. The cost of a pair is a formula, not a hope. The table above belongs in
   `DEV_MANUAL.md`'s "Which bounds" section and in S182's cost line.
2. The one-stage design passes a true zero 5 % of the time at `{0,5}`. The
   plan has about 45 pending verdicts; at that rate two of them will be false
   gains, and the FAQ's "sum of passed patches overstates the total" is the
   mechanism by which the plan's Elo arithmetic (S183) drifts up. The check
   is a periodic fixed-games match against a pinned old reference (section
   3.4), not a second SPRT stage.
3. The harness has never been calibrated on this machine (audit F10).

---

## 2. The match regime: time control, hash, book, adjudication, hardware

### 2.1 What the field does

**Controls and hash.** fishtest STC 10+0.1 with Hash 16, LTC 60+0.6 with Hash
64 (fishtest templates, above). Every OpenBench engine preset tests STC at
8 to 32 MB and most at 8+0.08 (`Engines/*.json`, confirmed at source by
`adocs/data/2026-09-04_plan_review_literature_check.md` A33). The invariant
that carries across controls is table pressure, overwrites per entry, not
size -- DEC-088's arithmetic, which is why `fastchess.sh` runs 16 MB and
`rating.sh` 128. **Chesso matches this** (S105, DEC-083, DEC-088).

**Scaling.** "Optimal parameters are often time sensitive" and gains verified
at the very short control used for tuning "regress at STC or LTC"; tune "at
the TC that is most relevant" and verify by matches at the others (vondele,
nevergrad4sf README, https://github.com/vondele/nevergrad4sf). Stockfish#2600
counted "23 reds, 16 yellows, 1 green" among the last 40 gaining LTC tests
and named what scales non-linearly: "especially extensions in search,
countermove pruning and initiative in eval" (Vizvezdenec,
https://api.github.com/repos/official-stockfish/Stockfish/issues/2600/comments).
The wiki's rule is the general form: a pruning or reduction condition "may
only be effectively tested on time controls where this new condition is
triggered frequently enough" (https://www.chessprogramming.org/Engine_Testing).
The Elo a doubling of time buys also falls with the control -- Komodo data:
144 Elo from 10+0.1 to 20+0.2, 51 Elo from 1280+12.8 to 2560+25.6
(https://www.chessprogramming.org/Match_Statistics) -- which is the mechanism
behind S128's scale-compression question and behind the 1.43-against-2.10
speed figures. **Chesso's S151** is the answer for the parameters that were
tuned at 2+0.02, and its scoping to pruning and reduction parameters matches
the list Vizvezdenec gives.

**Books.** Pohl's UHO books are human openings filtered to an evaluation band;
his statement of the effect is about 57 % draws against about 90 % on a
balanced book, "Elo-spreading ... around 3.5x bigger", keep the draw rate
between 45 % and 60 %, below 45 % the spreading shrinks
(https://www.talkchess.com/forum3/viewtopic.php?t=79415;
https://www.sp-cc.de/uho_2024.htm). fishtest moved its regression tests from
`8moves_v3.pgn` to UHO on 2023-06-29 and to `UHO_Lichess_4852_v1.epd` on
2026-02-04 (Regression-Tests page above); the book itself is CC0 at
https://github.com/official-stockfish/books. The counter-view is on record
too: about a quarter of the UHO_Lichess positions are decisive even at a 2:1
node ratio, so the most biased pairs carry little information
(https://github.com/official-stockfish/Stockfish/discussions/5079). **Chesso
measured exactly this** at S105: the pair-score variance did not move
(ratio 1.022), pairs decided by the opening rose from 13.4 % to 19.8 %, and
the book is kept on the game-length gain alone. That measurement is the
strongest thing in this section and it is chesso's own.

**Adjudication.** fishtest's fastchess line runs `-resign movecount=3
score=600 -draw movenumber=34 movecount=8 score=20`, with `-repeat -games 2
-recover -report penta=true -check-mate-pvs` and no tablebase adjudication
(https://github.com/official-stockfish/fishtest/wiki/Running-Fastchess;
`worker/games.py`). No justification is written on the wiki. OpenBench's
defaults are `movecount=3 score=400` and `movenumber=40 movecount=8 score=10`
(`Engines/Ethereal.json`; https://github.com/AndyGrant/OpenBench/wiki/Configuring-New-Engines)
-- **exactly the flags `fastchess.sh` and `rating.sh` run**, so chesso's
adjudication is the OpenBench default and not a local choice. The criticism on
record is Stockfish#1904 (2018): shallow STC scores make adjudication favour
"certain eval characteristics"; no maintainer resolution followed
(https://github.com/official-stockfish/Stockfish/issues/1904). Both flags are
two-sided by construction in fastchess -- a resign needs both engines to agree
-- so an evaluation on a different scale cannot trigger one alone (`rating.sh`
header). The S162 census (DEC-107) is the project's own evidence that the
100-halfmove zone is reached in about 3 % of games and matters in none.

**Time losses.** fastchess forfeits only when `time_left < -timemargin` and
counts the forfeit as an ordinary loss (`app/src/game/timecontrol/timecontrol.cpp`,
`match.cpp` in https://github.com/Disservin/fastchess). OpenBench always passes
`timemargin=250` "to avoid time losses, in unusually unstable conditions"
(`Client/worker.py`; https://github.com/AndyGrant/OpenBench/wiki/SPRT-and-Fixed%E2%80%90Game-Workloads)
and reports time losses without voiding. fishtest marks a worker's task bad
when `crashes > 3` or more than 10 % of its games were lost on time
(`server/fishtest/util.py`, `rundb.py`), excludes it from the purged result and
otherwise voids nothing. **Chesso's `tools/forfeit_report.py` threshold of 1 %
per engine (DEC-075) is ten times stricter than fishtest's**, and every census
so far has read 0.

**Concurrency and hardware.** The frameworks' guidance is uniform and it is
the opposite of DEC-048. fishtest: run "the number of physical cores leaving
one core for the OS", use virtual cores only on dedicated machines, and on
heterogeneous hardware "restrict the Fishtest worker to a subset of identical
cores" -- for Apple silicon "not exceed the number of fast cores", since more
"would lead to inconsistent results" (Running-the-Worker,
https://github.com/official-stockfish/fishtest/wiki); the macOS page says
concurrency is cores "(not including Hyperthreaded cores!), leaving one core
for the OS"; the AWS page: "If there are time losses or crashes then you should
disable hyper-threading." The worker also refuses slow or overloaded machines
(`min_nps_required = 180000 / (1 + 3*tanh((concurrency-1)/8))`,
`worker/games.py`). OpenBench's worker defaults its thread count to the
physical cores and reports logical against physical "to differentiate
hyperthreads" (`Client/worker.py`). Why: "Engines using that core will get
higher nps and create noise" (syzygy, https://talkchess.com/viewtopic.php?t=65747);
at very high concurrency even a dedicated box loses games on time
(https://github.com/cutechess/cutechess/issues/630). Turbo and thermal
behaviour of laptops appears in developer threads
(https://talkchess.com/viewtopic.php?t=81851: boost targets power and
temperature, "In the summer, the CPU will probably hit the 95 thermal target")
and in no framework document. **DEC-048 took the other side knowingly** --
every core, efficiency cores included, more variance for more throughput --
and asked that the cost in games per verdict be recorded. What the ledger
shows is that the trade is at least stable: 2328 to 2346 games an hour across
seven runs spanning two weeks, 0 forfeits in every census. The variance half
has never been measured on this machine (audit F10; R7 below), and it is the
one number that would let the owner re-take DEC-048 on evidence rather than on
the frameworks' word.

**Nodes as time.** Stockfish's `nodestime` option (`options.add("nodestime",
Option(0, 0, 10000))`, https://raw.githubusercontent.com/official-stockfish/Stockfish/master/src/engine.cpp)
converts the clock into nodes inside time management -- "convert from time to
nodes, and use resulting values in time management formulas", with the
increment and the move overhead scaled the same way
(https://raw.githubusercontent.com/official-stockfish/Stockfish/master/src/timeman.cpp)
-- so the engine plays a real time control, soft and hard limits included, on
a clock that hardware cannot disturb. fishtest uses it for SPSA "removing
noise introduced by inconsistent hardware speed", 160+1.6 in node time
standing for 60+0.6, with the caveat that it is only valid "if the value
you're tuning is not susceptible to change significantly the nps"
(https://raw.githubusercontent.com/wiki/official-stockfish/fishtest/Creating-my-first-test.md);
the worker adds `timemargin=10000` when it is on. fastchess has no such
conversion of its own; it offers per-engine `nodes=N`, `plies=N` and `st=N`
(`man.md`; `fastchess -help` 1.8.1 on this machine), and OpenBench's
data-generation presets run `N=40000` fixed nodes, noting that at fixed nodes
"a properly written engine will produce identical results" so reversed pairs
are pointless in self-play. The bias is agreed by everyone who has written
about it: "Your faster speed does not help you in fixed node testing" (Hyatt),
fixed nodes only for changes with "little impact on the nodes and speed"
(Don Dailey), best for "screening", confirm with timed matches (michiguel;
all at https://talkchess.com/viewtopic.php?p=415982), and the wiki's rule that
fixed budgets suit changes that "don't alter the search tree itself, but only
affect performance" (Engine_Testing page above). A `nodestime`-style option is
the better screening instrument than fastchess's `nodes=`, because time
management is still exercised; R12 is the decision about whether chesso wants
one.

### 2.2 Where chesso stands

The regime is the surveyed engines' regime and it was moved there
deliberately, one variable at a time, with a calibration before the first
verdict; the adjudication turns out to be OpenBench's default to the number.
One thing the sources say that the project measured before reading it: the
wiki recommends balanced books "for weaker engines" and biased books "with
stronger engines to reduce draw rate"
(https://www.chessprogramming.org/Sequential_Probability_Ratio_Test), which is
S105's finding -- the unbalanced book bought nothing at the pair level at 2559
-- from the other side; the book stays on its game-length gain and the owner's
standing option to revisit it (specs.md, Open items) now has the field's
statement beside the project's measurement. The two open items are
measurement debts, not design errors: the variance and forfeit calibration on
this machine, and the cost table above written where the bounds are chosen.
Whether to re-take DEC-048 is a third, and it waits on the first.

---

## 3. Correctness: what the engines test, and how they know their tests work

### 3.1 What the field does

**The bench signature.** Stockfish's commit-message hook requires
`Bench: <nodes>` or "No functional change"
(https://github.com/official-stockfish/Stockfish/wiki/Developers); CI takes
the reference from the last 100 commits and runs `tests/signature.sh`, which
compares `bench`'s node total with it after every architecture build
(https://raw.githubusercontent.com/official-stockfish/Stockfish/master/tests/signature.sh;
`.github/workflows/tests.yml`). Every fishtest worker runs `bench 16 1 13
default depth` before the first game and refuses the run on a mismatch
(`worker/games.py`); OpenBench requires `bench` to print nodes and nps,
deterministic across machines, and its client raises `Wrong Bench` otherwise
(https://github.com/AndyGrant/OpenBench/wiki/Requirements-For-Public-Engines).
Berserk's CI greps `Bench:` from the commit and exits 1 on a mismatch
(`.github/workflows/berserk.yml`); Stash's `check_bench.sh` does the same and
then a 20-iteration reproducibility loop and six perft positions
(`ci_scripts/check_bench.sh`, https://github.com/mhouppin/stash-bot). What the
signature guards is the tree's identity: a wrong branch, a miscompile or an
unintended functional change is caught before a game is played. **Chesso has
the instrument** (`tools/search_bench.py`, three positions, node counts and
best moves) **and none of the automation** (audit F07). The mutation pass
measured what it is worth: 21 of 33 injected bugs moved the depth-9 counts,
12 did not, so it complements the suite rather than replacing it.

**Reproducibility.** `tests/reprosearch.sh` plays two short sequences twice
across `ucinewgame` at 20 node limits and requires every `nodes N` line to
appear an even number of times
(https://raw.githubusercontent.com/official-stockfish/Stockfish/master/tests/reprosearch.sh).
It found a real bug: consecutive `bench` calls alternated 2,964,957 and
3,089,159 nodes because a table generation counter was not reset on
`ucinewgame` (Stockfish#5376,
https://api.github.com/repos/official-stockfish/Stockfish/issues/5376/comments)
-- a defect an SPRT cannot see and a signature check sees only by luck.
**Chesso has no equivalent**; `test_search.cpp:245` repeats three searches on
a fresh table only, and the audit found a warm-table hazard in `set_position`
(F08) that this test shape would catch.

**Assertions and sanitizers in CI.** Stockfish builds `debug=yes` (asserts on)
and self-plays it for 4 rounds at `tc=4+0.04` under fastchess, failing on any
`Assertion` in the log (`.github/workflows/games.yml`); `instrumented.py`
runs the command set under UBSan, TSan and Valgrind (`sanitizers.yml`,
https://raw.githubusercontent.com/official-stockfish/Stockfish/master/tests/instrumented.py).
Stash builds ASan and UBSan binaries and runs `d`, `go nodes`, `go depth`,
`go movetime`, `go wtime`, `bench` and a scripted game under each
(`ci_scripts/sanitize.sh`). Its search asserts `alpha < beta`, the window
sides, depth bounds, `!(PvNode && cutNode)` and more, live in that self-play
(https://raw.githubusercontent.com/official-stockfish/Stockfish/master/src/search.cpp).
**Chesso's assertions are the same kind and are dead in every binary the gate
runs** (audit F01); the sanitizer run exists as a CMake option and a habit.

**Perft and mate suites.** Universal: Stockfish's `perft.sh` (seven
positions plus eleven Chess960), Berserk's 120-position `perft.sh`,
`TerjeKir/EngineTests`' 127-position `perftsuite.epd`
(https://github.com/TerjeKir/EngineTests). Mate: Stockfish runs vondele's
`matetrack` over 6554 positions in CI, failing on PVs too short, too long,
allowing a draw or not ending in mate (`matetrack.yml`,
https://github.com/vondele/matetrack); fastchess's `-check-mate-pvs` is the
per-game form (`man.md`). **Chesso is ahead of every surveyed engine but
Stockfish here**: 82 constructed and 318 mined positions scored as floors, four
replayed games, `-check-mate-pvs` on every match (S145, S156, S168, S170).

**Unit tests at all.** Rare. Arasan is the one surveyed engine with a real
unit file -- SEE, perft, movegen, hash, eval symmetry, repetition, check
status, notation (https://raw.githubusercontent.com/jdart1/arasan-chess/master/src/unit.cpp).
Ethereal and Stormphrax show no `tests/` or `.github` directory (inferred
from 404s, not from a statement). Leorik has EPD-driven console programs.
**Chesso's 280 cases are unusual in the field**, and the tuner-model identity,
the surface guard, the DEC-102 bound rules and the script tests have no
counterpart anywhere surveyed (the 2026-08-14 review said so; still true).

**Differential and property testing.** Recompute-and-compare for keys and
accumulators in debug builds is the standing advice
(https://www.chessprogramming.org/Incremental_Updates); a `board_integrity`
routine asserted at function entry and exit is the worked example
(https://www.chessprogramming.net/debugging-a-chess-move-generator/); Beowulf
recomputed the key at every node under `DEBUG_VERIFY`
(https://talkchess.com/viewtopic.php?t=29797). `perftree` diffs per-root-move
counts against Stockfish's `go perft`
(https://github.com/agausmann/perftree), which `tests/debug_perft_app.cpp`
already speaks. No published brute-force oracle for SEE was found; Arasan's
`testSee` and tsoj's 22 cases are hand-labelled
(https://www.talkchess.com/forum3/viewtopic.php?t=69052). **Chesso's
`see_ge`-against-`see` cross-check over 2696 positions at eight thresholds is
ahead of that, with the caveat the audit records: a shared error passes.**

**Fuzzing.** AFL over UCI stdin found NNUE accumulator crashes and a
15-pieces-on-one-rank FEN that crashed `go searchmoves`
(https://chess.resistant.tech/); Stockfish's answer since version 19 is to
exit with `CRITICAL ERROR` on a position not reachable from the start
(https://github.com/official-stockfish/Stockfish/wiki/Stockfish-FAQ). No
libFuzzer harness for a chess engine was found (unverified negative).
**Chesso's S161, S174 and S176 are this class of work done by audit**; a
harness would find the next one without an auditor.

**Test suites for strength.** Displaced: EPD suites are "useful to prove
whether things are broken after program changes" and solving them "does not
necessarily correlate with practical playing strength"
(https://www.chessprogramming.org/Engine_Testing); "testing positions has
fallen out of favor among top engine developers"
(https://www.chessprogramming.org/Test-Positions). STS's Elo formula was
never given error bars and its labels do not survive re-analysis with newer
engines (https://talkchess.com/viewtopic.php?t=80876). **Chesso's DEC-019 and
its nine hand-built tactical positions are the right amount of this.**

**Knowing whether the tests work: mutation testing.** The accepted measure of
a suite's fault-detection ability is its mutation score, and it correlates
with real-fault detection independently of coverage (Just et al., FSE 2014,
Test-of-Time award 2024,
https://2024.esec-fse.org/details/fse-2024-plenary-events/9/Are-mutants-a-valid-substitute-for-real-faults-in-software-testing-),
while coverage's correlation with effectiveness "drops when suite size is
controlled for" (Inozemtseva and Holmes, ICSE 2014, summarised at
https://blog.acolyer.org/2014/10/21/coverage-is-not-strongly-correlated-with-test-suite-effectiveness/).
Tools exist for C++ -- Mull embeds LLVM-IR mutants in one binary
(https://mull.readthedocs.io/en/latest/HowMullWorks.html), dextool mutate
works on the Clang AST (https://github.com/joakim-brannstrom/dextool) -- and
no chess engine was found using either (unverified negative). **Chesso now
has a number**: 31 of 32, from 33 hand-written mutants in about 50 minutes of
machine time, with no dependency added. S145's "observed red under a stated
mutation" was this discipline applied to one test; the audit applied it to
the suite.

### 3.2 Where chesso stands

Ahead on: unit coverage of the search rules, the mate instruments, the SEE
and tuner cross-checks, the surface guard, the scripts under test. Behind on
automation of exactly the things the field automates first: the signature,
the assertions, the sanitizers, the reproducibility check. The audit's F01,
F02, F07 and F08 are those four; F03 is the cost of having built the suite on
golden numbers and is the one problem the field's practice does not speak to,
because the field mostly has no unit suite to have the problem with.

### 3.3 Speed, proxies and tuning

**Speed.** The Stockfish wiki's "Useful data" page gives, "For small speedups
(<~5%)", `Elo_stc(x) = 2.10 x` at 10+0.1 and `Elo_ltc(x) = 1.43 x` at 60+0.6
per percent of nps, adds "Numbers will depend on the precise hardware", and
prices the SPRT's sensitivity at 0.24 % (STC) and 0.70 % (LTC) speedup for a
50 % pass chance (https://official-stockfish.github.io/docs/stockfish-wiki/Useful-data.html,
confirmed in the 2026-09-04 literature check, A31). That is where INV-6's
conversion and DEC-083's "not measured in games" rule come from, and both are
Stockfish figures at Stockfish's draw rates, named as a conversion and never
as a verdict. How the speedup itself is measured: trivial speedups "around
0.5%" go straight to a pull request with repeated `bench` runs, mean and
standard deviation, tools `perf stat`, pyshbench, FishBench
(Creating-my-first-test.md above); pyshbench prints base and test nps with
standard deviations, the speed-up and a p-value (live copy
https://github.com/hazzl/pyshbench; vondele's original is gone); a commit
carries "50 runs", the two means and "No functional change"
(https://github.com/official-stockfish/Stockfish/commit/805afcbf3d5db39c85b759232cfb99ab0a250311).
Noise sources named in the sources: single-thread bench and all-thread
speedtest disagreeing under AVX-512 downclocking
(https://github.com/official-stockfish/Stockfish/issues/5757); turbo targeting
power and temperature rather than frequency, thermal ceilings in summer
(https://talkchess.com/viewtopic.php?t=81851); `hyperfine`'s warmup, outlier
detection and shell-spawn correction (https://github.com/sharkdp/hyperfine).
**Chesso's practice matches**: interleaved `hyperfine` with the noise floor
read off the run and `bench_movegen`'s own two-halves resolution (DEV_MANUAL
"Measure"), the `ps aux` idleness check first. What it lacks is the
convention that the speedup number rides in the commit (R1's `Bench:` line
can carry the nps too).

**Proxies.** Reported depth "is not comparable in general" between programs
(https://www.chessprogramming.org/Depth); the effective branching factor is
"quite useless to compare between programs" once reductions and pruning are
in (https://www.chessprogramming.org/Branching_Factor); time-to-depth failed
as a predictor when lazy SMP gained strength while looking worse on it
(https://chessprogramming.org/Lazy_SMP; https://talkchess.com/viewtopic.php?t=61131),
and a hardware-benchmark thread calls it "a completely useless metric" with a
consensus that only match play compares reliably
(https://talkchess.com/viewtopic.php?t=84278). No source endorses
time-to-depth even for move-ordering changes. **Chesso's INV-6 identity is
stricter than anything found** -- identical node counts and best moves, not a
depth reached in a time -- and S021's record (aspiration windows +0.8 % nodes
on three positions, -7.5 % on 300, H1 in the match) is the project's own case
against reading a saving off a bench.

**SPSA.** Spall's alpha 0.602 and gamma 0.101; Kiiski's tuner measures each
iteration with one game pair and ends at `c_k` of 4 centipawns
(https://www.chessprogramming.org/SPSA; https://github.com/zamar/spsa).
fishtest fixes the game count at creation, uses `r_k = a_k / c_k^2` with no
paper reference (https://github.com/official-stockfish/fishtest/issues/535),
says "60+0.6 is best to get better scaling values", warns that "a very short
time control on fishtest is not working", and sends a tuned vector through the
normal STC-then-LTC SPRT (Creating-my-first-test.md; one of two reads of the
page returned that last sentence and the other did not -- read the page
before quoting it). **Chesso's S084/S085 follow this shape** and S151 is the
scaling caveat made a step.

**Texel tuning.** Osterlund's description: mean squared error between the
game result and the sigmoid of the quiescence score, K fitted once and
frozen, 64000 fast games and about 8.8 M positions, book and mate-score
positions excluded, about 400 parameters, "Correlation does not imply
causation", and the closing doubt whether "the method is good" or Texel's
evaluation "was particularly bad" (https://www.chessprogramming.org/Texel%27s_Tuning_Method).
Practitioners since: overfitting and L1 regularisation
(https://talkchess.com/forum3/viewtopic.php?t=65538); a lower error that
"loses but only by a few ELO over 10K games"
(https://www.talkchess.com/forum/viewtopic.php?t=64189&start=36); tuning
non-quiet positions with a static score is a bug, use a quiet set
(https://talkchess.com/viewtopic.php?t=83196); an all-zero evaluation scores
0.125 MSE at 50 % draws, so an error near that "is not good at all"
(https://talkchess.com/viewtopic.php?p=879780); "it is necessary to play the
games" (https://github.com/official-stockfish/nnue-pytorch/wiki/Basic-training-procedure-(train.py)).
**Chesso's S028/S065/S075/S076 chain does all of it** -- held-out games,
game-level split (S066), the lambda blend, K fitted on the outcome, the SPRT
after every fit -- and DEC-037's "-14.93 Elo from a hand-picked set that
looked right" is its own version of the closing doubt.

**CLOP and the rest.** Local quadratic models of the win rate, samples
"confidently inferior to the mean" discarded, W/D/L inputs
(https://www.remi-coulom.fr/CLOP/); infeasible at 400 parameters
(https://www.chessprogramming.org/CLOP); every tuner's output is verified by
games (https://www.chessprogramming.org/Automated_Tuning).

**Data for a future network.** Generators drop positions where the best move
is a capture or a promotion or the mover is in check ("smart fen skipping",
on by default in the current trainer), skip duplicates, cap the evaluation,
randomise the first plies and sample few positions per game
(https://github.com/nodchip/Stockfish/blob/master/docs/gensfen.md;
.../docs/learn.md; the nnue-pytorch wiki above and its Training-datasets
page); "Good datasets are chosen empirically" and "Better evaluations don't
always give better results". A hobby-scale account: a few random plies, fixed
5000 to 10000 nodes, skip check and capture positions, and "recommend 100M+"
positions after 15 M failed (https://talkchess.com/viewtopic.php?t=83944).
S082 and S083 already carry the resolved-position label and the held-out
decision; the filters above are what S082 should state it applies or refuses.

### 3.4 Absolute rating and drift

**The instruments.** Ordo fits a logistic scale (76 % expectancy is 202
points) by maximum likelihood, with fixed, loose or relative anchors, a pool
average default of 2300, simulated error bars, and the manual's warning that
its errors are "always against the reference (anchor)" and "incorrect to use
... to estimate relative values against other engines" without the `-e`
matrix (https://raw.githubusercontent.com/michiguel/Ordo/master/manual.tex).
BayesElo estimates a white advantage and a draw parameter from the data
(32.8 and 97.3 on 29610 games in its example), pulls ratings together through
its prior and needs more games than Elostat
(https://www.remi-coulom.fr/Bayesian-Elo/). CCRL's Blitz list is "Computed
... with Bayeselo", 2'+1" on an i7-4770k equivalent, hash 128 or 256 MB,
ponder off, four- to six-piece tablebases, any book limited to 12 moves and
the same for all engines, learning off
(https://www.computerchess.org.uk/ccrl/404/about.html; index page read by
`curl`, WebFetch gets 403). So `rating.sh` solves with a different estimator
than the list it anchors to, and the compaction BayesElo applies is on record
from a practitioner: "CCRL uses BayesElo, which tends to compact the Elo
numbers" (Kaufman, https://www.talkchess.com/forum3/viewtopic.php?f=2&t=73124).
DEC-077 named the control as the leading suspect for the 121.8-Elo anchor
spread; the estimator is a second suspect the sources add, and S152's
two-control run can separate them only if it also solves once with BayesElo.

**How much self-play sticks.** The plan discounts self-play Elo "at the ratio
the published per-release records support (~60 % sticks)" (S183's quote of
`plan.md`). **That ratio could not be verified at source today**: the page it
is attributed to returned 403 and no other source states it; Kaufman's
statement is that Komodo's gains against Stockfish were "close to the gains
in self-testing" (same thread), which is a 1:1 anecdote at the top of the
list. S183 should carry the ratio as *unverified* and let the ledger's own
published-to-measured figures do the discounting, which is what its accepts
already asks.

**Drift.** fishtest's regression tests are the field's instrument for the
question the plan actually has -- *do the passed patches add up?* -- a
fixed-length match against a pinned reference, repeated over time, read as a
trend: 60000 games at 60+0.6 against the last release, reported as Elo, nElo,
pentanomial and pairs ratio, every entry master against Stockfish 18 today
(https://official-stockfish.github.io/docs/stockfish-wiki/Regression-Tests.html).
For chesso that is 2000 games against the S105-era commit, about 52 minutes at
2337 games/h, with about a +/- 10 Elo interval at the S105 pair variance --
enough to see whether the sum of the kept verdicts (S183's table) is in the
engine, and cheap enough to repeat every ten verdicts. It is not the gauntlet
and does not replace S152; it answers the FAQ's bias warning with a
measurement (R14).

---

## 4. Recommendations, ranked

**Decided 2026-09-05, one item at a time (DEC-139).**

| item | decision | where it lives |
|---|---|---|
| R1 | step + commit rule | S189, DEC-140 |
| R2 | step + Debug self-play habit | S190, DEC-141 |
| R3 | step, before S109 | S191 |
| R4 | step + rule generalising DEC-116, `anchors.py` committed | S192, DEC-142 |
| R5 | one step | S193 |
| R6 | step with an environment-variable seed | S194 |
| R7 | **refused for this machine**: the workstation returns soon; the calibration is its first run | F10 `accepted`, DEC-139; parked in `status.md`; DEC-143 |
| R8 | step; the keys join S179 | S195, S179 amended |
| R9 | tool + TESTS-rule line | S196, DEC-141 |
| R10 | script + cadence rule | S197, DEC-141 |
| R11 | all three | S182 amended (a), S198 (b), DEC-143 (a, c) |
| R12 | deferred to S127's design | S127 amended |
| R13 | folded into S197 as a documented command | S197 |
| R14 | step, first point after the S109 block | S199 |

Each: what, the evidence, the cost, what it buys, how it is verified, and
whether it is a step or a decision. "Machine-free" means no match and no fit.
Costs are agent-hours of writing plus the gate.

### R1  A `bench` command, its signature in every `src/` commit, and a gate step that checks it   -- closes audit F07, F08

**What.** `bench` in the UCI dispatch: `ucinewgame`, then `go depth 9` (or a
fixed node budget) over the three `search_bench.py` positions and a few more,
printing one line `<nodes> nodes <nps> nps` and the per-position best moves.
The commit-message rule: a commit touching `src/` ends with `Bench: <nodes>`
or `No functional change`. A `tools/gate.sh` that runs the TESTS rule and
then compares the built binary's bench with the message. `search_bench.py`
keeps its timing role.
**Evidence.** Every surveyed CI does this first (section 3.1). The mutation
pass: 21 of 33 bugs moved the signature, including the one-ply floor drift the
mate gates missed. INV-6 is today "no test: a procedure".
**Cost.** Machine-free. Half a day: the command, its surface in `MANUAL.md`
and the golden guard (SURFACE rule), the gate script, `DEV_MANUAL.md`.
**Buys.** Automatic INV-6 for every neutral change; DEC-020-class
contamination visible at commit time; the field's convention, so an OpenBench
instance could run chesso later (it requires `bench`).
**Verified by.** The gate script fails on a commit whose message carries the
previous signature after a functional change (observed red, then green).
**Step.** One step; decision on the commit-message rule belongs in AGENTS.md
COMMITS and is the owner's.

### R2  The invariants in the gate: a Release test for INV-2 and INV-4, and a Debug self-play habit   -- closes F01

**What.** A fast test with the shape of `test_engine.cpp:56`: walk the corpus
at depth 3 to 4 and after every make and unmake compare `material`,
`psqt_mg`, `psqt_eg`, `phase` and `squares[]` against a full rebuild
(`eval_refresh`, `squares_match_bitboards` made available to tests). Plus the
Stockfish `games.yml` habit: a step that touches make/unmake, the generator
or the search runs a 4-round `fastchess` match of the Debug binary against
itself at `4+0.04` and greps the log for `Assertion`, and says so in its
stamp.
**Evidence.** F01; `CLAUDE.md`'s NNUE-hook hazard; the hash has this oracle
and it caught M23.
**Cost.** Machine-free for the test (hours). The Debug self-play is minutes
per step that needs it.
**Buys.** Two of six invariants enforced by something that runs.
**Verified by.** A mutant that drops one `add_piece` accumulator update goes
red in Release.
**Step.** One step. The habit is a line in the TESTS rule and a decision.

### R3  Direct tests for the pruning and reduction guards, and the S165 defender set registered   -- closes F02

**What.** One case per guard with a precondition that the guard's condition
holds at the node and an assertion on the node's result: no null move in
check; a pawn-only position searched through; a defender node from
`adocs/data/S165_defender_set.tsv` inside the mate band gets no null-move
cutoff; a checking move and a capture searched at `child_depth`; a reduced
move that beats alpha re-searched at full depth. The existing patterns are
`test_engine.cpp:2062` (fixture TSV, floors) and the tune build's
`search_lmr_reduction_probe`.
**Evidence.** Five guard-removal mutants caught by one golden count and
nothing else; 104 proved positions built for exactly this and read by nothing.
**Cost.** Machine-free, a day. The S109 block adds four more rules and each
should arrive with its case and its mutant (R9).
**Buys.** Coverage of the recurring bug class that survives a golden
re-derivation (F03).
**Verified by.** M01 to M04 and M07 each go red in a test other than
`test_mate_carry`.
**Step.** One step, before S109.

### R4  Golden hygiene: every golden named, scripted and paired with a property   -- closes F03

**What.** (a) Each golden value or floor in `tests/` carries a comment naming
it a golden and the script that re-derives it; `adocs/data/S154_floor_margin_sweep.py`
is the pattern and DEC-116 the rule for one of them. (b) `.tuning/anchors.py`
or its equivalent is committed, so the piece anchors are re-derivable from
the repository (`status.md` has parked this since 2026-08-23) -- S192 committed
it as `adocs/data/S192_anchors.py`. (c)
`tests/test_engine.cpp` "the iteration loop scales its soft limit by the
history it counted" asserts the scaling on a constructed stability history, not
on a search of a fixed position -- named by its `TEST_CASE` and not by a line
number, which is DEC-135 and which the line this replaced had already outlived. (d) The rule is
generalised: a floor is re-derived whenever either end of it moves, by its
script, with the margin stated -- a decision extending DEC-116 to every
golden.
**Evidence.** F03: 16 cases red on an eval sign flip, 8 on tree-shape changes
that were not defects, `test_mate_carry` red on 21 of 22 search mutants.
**Cost.** Machine-free, a day; the decision is one entry.
**Buys.** Every pending search and evaluation step pays a known, scripted
re-derivation instead of an argument about a floor; the pressure to weaken is
replaced by a procedure.
**Verified by.** The scripts run and reproduce today's numbers.
**Step and decision.** One step; the rule is a decision.

### R5  The small repairs: the fifty-move boundary, the vacuous assertions, the titles   -- closes F04, F05

**What.** A depth-1 fifty-move boundary pair (clock 99 root, clock 100 draw;
clock 98 root, not a draw); `ucinewgame` before `test_engine.cpp:901`'s timed
search; the `LOG_I` assertion dropped or replaced by an observable; a `count
> 0` precondition at `test_movegen.cpp:265`; `test_perft`'s asset check made
Release-safe and its columns made to fail; the `RUN_THREADS` dead code
removed; honest titles at `test_search.cpp:284` and `test_engine.cpp:1367`;
`test_helpers.hpp`'s comment and `test_generate_legal_moves` made true;
`tests/test_audit_go_infinite.cpp` registered once its window is a condition
rather than a sleep.
**Evidence.** F04 (a survivor), F05 (eleven items).
**Cost.** Machine-free, hours.
**Buys.** The green count means what it says.
**Verified by.** M19 goes red; each repaired case is shown red under its own
mutation before green.
**Step.** One step; the go-infinite registration closes
`2026-09-04_adversarial-F01`'s test half and may fold into that finding's
step.

### R6  The UCI book path under test, with a seed hook   -- closes F06

**What.** A seed for the book's `mt19937_64` (tune-build option or an
environment variable read at startup) and two fast cases on the embedded
book: `OwnBook true` then `go depth 1` from the start position answers a move
the library returns for the start key; `Best Book Move true` answers the
heaviest.
**Evidence.** F06: zero executions of `src/chesso.cpp:662-736` across the
fast label; the S172 and S175 defects were in this path.
**Cost.** Machine-free, hours.
**Step.** One step.

### R7  Calibrate the harness on this machine   -- closes F10

**What.** `AA=1 ./fastchess.sh` at fixed rounds, 1000 games, on mains
(POWER), read with `adocs/data/S105_pairs.py` and `tools/forfeit_report.py`:
pair-score variance, pentanomial, forfeits, games per hour; recorded beside
S105's numbers and beside the cost table of section 1.1 in `DEV_MANUAL.md`.
**Evidence.** F10; DEC-048's unanswered "how many more games".
**Cost.** About 26 minutes of the machine at 2337 games/h, detached, with
the four-exit watcher. Write-up an hour.
**Buys.** S182's cost line gets a measured variance for this machine; the
efficiency-core trade gets its number; a baseline to re-run after any harness
change (fastchess version, book, machine).
**Step.** One step; it owns a run, so the coordinator's.

### R8  A reproducibility test across `ucinewgame`, and `bench` resets before every position   -- closes F08

**What.** The `reprosearch.sh` shape as a fast test: two move sequences, each
searched at several node limits before and after `ucinewgame`, every node
count required to repeat; plus a case that two `go depth N` on the same FEN
in one process, separated by `ucinewgame`, agree. The `bench` of R1 sends
`ucinewgame` per position. The Zobrist keys move to the project's own integer
generator under the S179 seed discipline; `DEV_MANUAL.md` records the
warm-table rule.
**Evidence.** F08; Stockfish#5376.
**Cost.** Machine-free, hours. The key change is behaviour-neutral by
construction and proves it on the signature (node counts move, best moves and
perft do not -- state which before the change).
**Step.** One step; the key regeneration may join S179.

### R9  Mutation checking as a standing instrument

**What.** The evidence driver becomes `tools/mutation_check.py`, reading a
tracked mutant list; run after any change to `tests/` that adds or removes
coverage, and each new pruning or reduction rule ships with a mutant that its
test kills -- S145's "observed red under a stated mutation" generalised. A
full pass is about 50 minutes; a step runs its own mutants only.
**Evidence.** Section 3.1's literature; the audit's 31 of 32 is the baseline
to hold.
**Cost.** An hour to make the driver a tool; per step, minutes. No
dependency (mull and dextool were considered and are not needed for 33
hand-written mutants; the DEPS rule stands).
**Buys.** The only measure of the suite's teeth that does not depend on
reading it.
**Step and rule.** One small step; the per-rule mutant is a line in the
TESTS rule and a decision.

### R10  A second gate script for what the fast label cannot hold

**What.** `tools/gate_extra.sh`: build and run the six Debug binaries (INV-2,
INV-4; about 7 minutes), an ASan/UBSan build of the fast label and `bench`,
`ctest -L slow` (deep perft), `--prose` and `--citations`. Cadence: before a
step completes when it touched make/unmake, the generator or the search;
otherwise weekly, recorded in `status.md`'s Watching line.
**Evidence.** Section 3.1: Stockfish's `sanitizers.yml` and `games.yml`;
audit F01 and the 2026-08-14 review's "sanitizers absent from the gate".
**Cost.** Machine-free to write, about 20 minutes to run.
**Step and rule.** One step; the cadence is a decision.

### R11  SPRT practice: write the cost down, and reproduce the opening order

**What.** (a) The cost table of section 1.1 in `DEV_MANUAL.md` "Which bounds"
and in S182's line, and a rule that a run's pre-registration states the pair's
worst-case games and the abort rule beside the three outcomes (DEC-063
already asks for the outcomes). (b) `-srand <seed>` in `fastchess.sh`, printed
in the banner, so a run's opening sequence is reproducible and two runs on one
change share their openings; `-pgnout ... nodes=true timeleft=true` so a
census can read node counts and clock margins from the PGN (both are
fastchess 1.8.1 options, `fastchess -help`). (c) An A/A fixed-rounds run after
every change to the harness (R7 is the first).
**Cost.** Documents and two flags; no verdict's cost changes.
**Decision.** (a) and (c) are practice; (b) is a harness change and needs one
A/A to show the seed changes nothing about the distribution.

### R12  Node-time screening, for decision

**What.** Whether sweeps and `--fast` first looks may run on a clock that
hardware cannot disturb, with every verdict still taken at 8+0.08 in wall
time. Two forms. The cheap one is fastchess's per-engine `nodes=N`: fixed
nodes per move, deterministic, time management not exercised at all. The
better one is a `NodesTime` option in the tune build, Stockfish's `nodestime`
shape: the clock, the increment and the move overhead converted to nodes, so
the soft and hard limits and the S089/S132 scaling still run and only the
hardware is removed. Either form carries the stated bias -- a change that
spends time to save nodes looks better than it plays, and a speed change is
invisible -- so neither may decide a step.
**Evidence.** Section 2.1: fishtest's SPSA use of `nodestime` with its
caveat; Hyatt, Dailey and michiguel on fixed nodes as a screen; DEC-048's
efficiency-core variance is exactly what node time removes.
**Cost.** `nodes=` is a `--nodes` mode in `fastchess.sh`, an hour. `NodesTime`
is a small change to `compute_search_time_budget` and the timer, tune build
only, behaviour-neutral in the shipping build and proved so on the signature
(R1); half a day with its tests.
**Decision.** The owner's. The recommendation is `NodesTime` in the tune
build, allowed for sweeps and first looks and written into `DEV_MANUAL.md` as
never a verdict; S127's whole-parameter SPSA is the first run that would use
it.

### R13  Coverage as a periodic report, not a gate

**What.** The `llvm-cov` recipe from the audit as a documented command in
`DEV_MANUAL.md` "Test", run when a subsystem is added; the unexecuted list
compared with the previous one.
**Evidence.** Section 3.1: coverage does not measure effectiveness; it does
locate unreached code (F06 was found this way).
**Cost.** A paragraph. No step needed beyond the document.

### R14  A regression match against a pinned reference, for decision

**What.** Every ten landed verdicts, or at each block boundary, 2000 games at
8+0.08 against a pinned early-S105 commit, fixed rounds, read as Elo +/-
interval and pentanomial, plotted over time in `adocs/data/`. Not a verdict
and not the gauntlet.
**Evidence.** Section 1.1 and 3.4: the FAQ's bias, fishtest's regression
tests, DEC-108's deferral of the expensive instrument.
**Cost.** About 52 minutes of machine per point.
**Decision.** The owner's, against DEC-108: this is the cheap form of the
question DEC-108 postponed, and it can be refused on the same reasoning.

---

## 5. What was considered and not recommended

- **A remote CI service.** Every surveyed engine with automation uses GitHub
  Actions or GitLab CI. Here the machine is the binding constraint, the gate
  is a rule agents follow, and a local `tools/gate.sh` (R1) gives the same
  guarantee at commit time without a second machine. Reconsider if the
  workstation returns and a runner can live on it.
- **Mull or dextool.** Real tools, real cost (LLVM version pinning, build
  integration); 33 hand-written mutants measured what was needed in an hour
  (R9). DEPS rule.
- **Changing the bounds, the control, the hash or the book.** Nothing in the
  sources argues against the current regime for an engine at this strength,
  and S105 measured the one claim that did not hold. The cost is a property of
  the pairs (section 1.1) and is now written down.
- **Tactical EPD suites as a gate.** DEC-019 already; the field agrees.
- **Fuzzing harness.** Worth doing once for the parsers (libFuzzer on
  `load_FEN`, `algebraic_to_move`, the UCI tokenizer) -- an afternoon, no
  dependency beyond clang's built-in `-fsanitize=fuzzer` -- but the three
  audits have been doing this work by hand and finding one defect each; a
  low-priority step rather than a recommendation.

---

## 6. Source table

Fetched 2026-09-04 by four literature subagents whose notes this file
condenses; where two reads of one page disagreed (fishtest's current SPRT
presets) the repository template was taken over the wiki paraphrase, and the
page is named so it can be re-read.

| topic | source |
|---|---|
| Wald 1945 | https://projecteuclid.org/journals/annals-of-mathematical-statistics/volume-16/issue-2/Sequential-Tests-of-Statistical-Hypotheses/10.1214/aoms/1177731118.full |
| GSPRT | https://www.cantate.be/Fishtest/GSPRT_approximation.pdf |
| normalized Elo, run length | https://cantate.be/Fishtest/normalized_elo_practical.pdf ; https://www.cantate.be/Fishtest/sprta.pdf |
| stopping bias, median-unbiased estimate | https://www.cantate.be/Fishtest/brownian_approximation.pdf ; fishtest FAQ https://github.com/official-stockfish/fishtest/wiki/Fishtest-faq |
| fishtest mathematics, bounds, presets | https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-Mathematics.html ; https://github.com/official-stockfish/fishtest/wiki/Creating-my-first-test ; https://github.com/official-stockfish/fishtest (templates, `stats/sprt.py`, `util.py`) ; https://github.com/official-stockfish/fishtest/issues/865 |
| pentanomial vs trinomial | https://github.com/vdbergh/pentanomial ; https://www.chessprogramming.org/Match_Statistics ; https://www.talkchess.com/forum3/viewtopic.php?t=69407 |
| STC vs LTC | https://github.com/official-stockfish/Stockfish/issues/2600 |
| regression tests, books over time | https://github.com/official-stockfish/Stockfish/wiki/Regression-Tests |
| CPW bounds table | https://www.chessprogramming.org/Sequential_Probability_Ratio_Test |
| OpenBench bounds and requirements | https://github.com/AndyGrant/OpenBench (Engines/*.json, `OpenBench/stats.py`, `Client/bench.py`) ; https://github.com/AndyGrant/OpenBench/wiki/Requirements-For-Public-Engines ; https://github.com/AndyGrant/OpenBench/wiki/SPRT-and-Fixed%E2%80%90Game-Workloads |
| engine testing guide | https://dannyhammer.github.io/engine-testing-guide/sprt.html ; .../strength-testing.html |
| fastchess | https://github.com/Disservin/fastchess (`man.md`, `sprt.cpp`, `stats.hpp`, `elo_pentanomial.cpp`) ; `fastchess -help` 1.8.1 locally |
| UHO books | https://www.talkchess.com/forum3/viewtopic.php?t=79415 ; https://www.sp-cc.de/uho_2024.htm ; https://www.sp-cc.de/anti-draw-openings.htm ; https://github.com/official-stockfish/books ; https://github.com/official-stockfish/Stockfish/discussions/5079 |
| Stockfish tests and CI | https://raw.githubusercontent.com/official-stockfish/Stockfish/master/tests/{perft.sh,signature.sh,reprosearch.sh,instrumented.py,testing.py} ; `.github/workflows/{tests,sanitizers,games,matetrack}.yml` ; https://github.com/official-stockfish/Stockfish/wiki/Developers ; https://api.github.com/repos/official-stockfish/Stockfish/issues/5376/comments |
| other engines' CI | https://raw.githubusercontent.com/jhonnold/berserk/main/.github/workflows/berserk.yml ; https://raw.githubusercontent.com/mhouppin/stash-bot/master/.gitlab-ci.yml and `ci_scripts/` ; https://raw.githubusercontent.com/jdart1/arasan-chess/master/src/unit.cpp ; https://github.com/TerjeKir/EngineTests |
| matetrack | https://github.com/vondele/matetrack |
| perft, incremental updates, debugging | https://www.chessprogramming.org/Perft ; https://www.chessprogramming.org/Perft_Results ; https://www.chessprogramming.org/Incremental_Updates ; https://www.chessprogramming.net/debugging-a-chess-move-generator/ ; https://github.com/agausmann/perftree ; https://talkchess.com/viewtopic.php?t=29797 |
| SEE cases | https://www.talkchess.com/forum3/viewtopic.php?t=69052 |
| fuzzing | https://chess.resistant.tech/ ; https://github.com/official-stockfish/Stockfish/wiki/Stockfish-FAQ |
| EPD suites and strength | https://www.chessprogramming.org/Engine_Testing ; https://www.chessprogramming.org/Test-Positions ; https://talkchess.com/viewtopic.php?t=80876 ; https://www.talkchess.com/forum3/viewtopic.php?t=56653 |
| mutation testing and coverage | https://2024.esec-fse.org/details/fse-2024-plenary-events/9/Are-mutants-a-valid-substitute-for-real-faults-in-software-testing- ; https://blog.acolyer.org/2014/10/21/coverage-is-not-strongly-correlated-with-test-suite-effectiveness/ ; https://mull.readthedocs.io/en/latest/HowMullWorks.html ; https://github.com/joakim-brannstrom/dextool ; https://clang.llvm.org/docs/SourceBasedCodeCoverage.html |
| Elo per percent of speed | https://official-stockfish.github.io/docs/stockfish-wiki/Useful-data.html (A31 of the 2026-09-04 literature check) |
| fishtest worker guidance, speed floor, bad tasks | https://github.com/official-stockfish/fishtest/wiki (Running-the-Worker, Running-the-worker-on-macOS, Running-the-worker-in-the-Amazon-AWS-EC2-cloud, Running-Fastchess) ; `worker/games.py` ; `server/fishtest/util.py`, `rundb.py`, `views.py` |
| fishtest first-test guide, speedups, SPSA, nodestime use | https://raw.githubusercontent.com/wiki/official-stockfish/fishtest/Creating-my-first-test.md ; https://github.com/official-stockfish/fishtest/issues/535 |
| Stockfish `nodestime`, useful data, contributing | https://raw.githubusercontent.com/official-stockfish/Stockfish/master/src/engine.cpp ; .../src/timeman.cpp ; https://official-stockfish.github.io/docs/stockfish-wiki/Useful-data.html ; https://github.com/official-stockfish/Stockfish/blob/master/CONTRIBUTING.md ; https://github.com/official-stockfish/Stockfish/issues/1904 ; https://github.com/official-stockfish/Stockfish/issues/5757 ; https://github.com/official-stockfish/Stockfish/commit/805afcbf3d5db39c85b759232cfb99ab0a250311 |
| OpenBench worker and presets | https://raw.githubusercontent.com/AndyGrant/OpenBench/master/Client/worker.py ; .../Engines/Ethereal.json ; https://github.com/AndyGrant/OpenBench/wiki/Configuring-New-Engines |
| hardware noise, concurrency | https://talkchess.com/viewtopic.php?t=65747 ; https://talkchess.com/viewtopic.php?t=81851 ; https://github.com/cutechess/cutechess/issues/630 ; https://raw.githubusercontent.com/cutechess/cutechess/master/docs/cutechess-cli.6.txt |
| fixed nodes, proxies | https://talkchess.com/viewtopic.php?p=415982 ; https://www.chessprogramming.org/Depth ; https://www.chessprogramming.org/Branching_Factor ; https://chessprogramming.org/Lazy_SMP ; https://talkchess.com/viewtopic.php?t=61131 ; https://talkchess.com/viewtopic.php?t=84278 ; https://www.chessprogramming.org/Nodes_per_Second |
| speed measurement | https://github.com/hazzl/pyshbench ; https://github.com/sharkdp/hyperfine |
| tuning | https://www.chessprogramming.org/SPSA ; https://github.com/zamar/spsa ; https://github.com/vondele/nevergrad4sf ; https://www.chessprogramming.org/Texel%27s_Tuning_Method ; https://talkchess.com/forum3/viewtopic.php?t=65538 ; https://www.talkchess.com/forum/viewtopic.php?t=64189&start=36 ; https://talkchess.com/viewtopic.php?t=83196 ; https://talkchess.com/viewtopic.php?p=879780 ; https://www.remi-coulom.fr/CLOP/ ; https://www.chessprogramming.org/CLOP ; https://www.chessprogramming.org/Automated_Tuning |
| training data | https://github.com/nodchip/Stockfish/blob/master/docs/gensfen.md ; .../docs/learn.md ; https://github.com/official-stockfish/nnue-pytorch/wiki/Basic-training-procedure-(train.py) ; https://github.com/official-stockfish/nnue-pytorch/wiki/Training-datasets ; https://talkchess.com/viewtopic.php?t=83944 |
| rating | https://raw.githubusercontent.com/michiguel/Ordo/master/manual.tex ; https://www.remi-coulom.fr/Bayesian-Elo/ ; https://www.computerchess.org.uk/ccrl/404/about.html and .../4040/about.html (by `curl`) ; https://www.talkchess.com/forum3/viewtopic.php?f=2&t=73124 ; https://en.wikipedia.org/wiki/CCRL |
| UHO 2024 construction, uhotrack | https://www.sp-cc.de/uho_2024.htm ; https://github.com/robertnurnberg/uhotrack |
| unverified today (403 or 404) | the "~60 % sticks" self-play ratio page ; CCRL about pages via WebFetch ; the Stockfish wiki UCI page for `nodestime` (the option was read from the source instead) ; Grant's Ethereal tuning paper (binary) |
