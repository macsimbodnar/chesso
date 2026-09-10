# Audit 2026-09-10 adversarial

Commit audited: `d318113f405399d14d0915adc00a913cb3c9f40f` (`achesso`, "Say what
F08 being closed covers, and what it does not (S195)", 2026-09-10), clean
working tree. Run started 2026-09-10 and finished after midnight on
2026-09-11; the report keeps the start date, which is also the audited
commit's, because the finding ids carry it.

Type: `adversarial`, whole project, asked as **"review the engine implementation
so far in respect to the project goals and restrictions, using literature, open
source engines and online available material."** So the two founding rules of
`CLAUDE.md` are the axes, not a preamble to them: *nothing is copied* and
*nothing is believed without a measurement*. Scope: `src/`, `tests/`, `tools/`,
the shell harnesses, `adocs/` and the root documents, read against the published
record and against other engines' sources where a licence question required it.

Method. Five reviewers on separate axes -- originality and licence forensics,
search, board/movegen/TT/UCI, evaluation and its tuner, and the measurement
apparatus -- each read-only, each required to cite `file:line` or a command and
its output, each forbidden to judge a chess position from its own reasoning
(`CLAUDE.md`). **Every finding below marked VERIFIED was reproduced a second
time by the coordinator, from the reproduction and not from the reviewer's
prose**; three did not survive that step and are recorded at the end of Part F.
Oracles: `/usr/games/stockfish` and python-chess 1.11.2 driven through
`chess.engine` (never a bare pipe -- see Part F). Runs: `cmake --build build
-j12`; `ctest -L fast` in `build` (33/33, 91.1 s) and `build-tune` (33/33,
92.1 s) under `CLANG_FORMAT_MAJOR=22`; `ctest -L slow` (`test_perft`, 1/1,
73.0 s); `build-sanitize` rebuilt and driven; `bench`; `tools/search_bench.py`
at depths 9 and 12; `build/tools/magic_gen`. No match, SPRT, SPSA or tuner run
was started. Nothing in the repository was modified except this file.

Finding ids read `2026-09-10_adversarial-F<nn>`. Status is `open` on all of
them; none was fixed here.

## Verdict

**Five high findings. Three are defects the engine ships today; two are
breaches of the project's two founding rules.**

The engine ships a crash and two out-of-bounds accesses reachable from its own
`position fen` surface, and it scores a class of position that occurs in
ordinary play as a dead draw when no draw exists -- the last one pinned as
correct by a test in the fast suite. `src/bb_tables.hpp` still carries three
tables copied from a GPL-3.0 engine, provable by trailing whitespace, from the
same commit whose magic numbers S179 already replaced for exactly this reason.
And the SPRT harness and the rating harness both run **one-sided** resign
adjudication while `rating.sh` states in a comment that they do not -- 76 % to
84 % of every game in this project's ledger ends on an engine's own reported
score rather than on play.

**Against that: the record is accurate and the discipline is real.** Every
load-bearing number in `adocs/specs.md` that can be re-derived was re-derived
here and matched exactly -- `bench` 26851183, both `search_bench` baselines to
the node with their best moves, all 128 magics reproducing from the project's
own seed as an ordered list, both gated builds green, perft green over nine
columns. The lazy-evaluation shortcut is sound by construction rather than by
sampling, which is rarer than it sounds. The SPSA implementation is correct
against Spall. Negative results are recorded as negative. The five reviewers
between them found no defect at all in move generation, make/unmake, the Zobrist
hashing or the transposition table's semantics, under fuzzing that took every
legal move at every node of 120 games and compared `board_t` byte for byte.

The gap this audit is really about is elsewhere, and it is Part D: **no measured
Elo has been added since 2026-08-22**, and the next six steps in `plan.md`'s
order contain no engine feature.

---

# Part A -- the first foundation: nothing is copied

### 2026-09-10_adversarial-F01  high  Three tables in `src/bb_tables.hpp` are copies from a GPL-3.0 engine, and the whitespace proves it

Status: open. VERIFIED.

Evidence. `github.com/maksimKorzh/bbc` ("Bit Board Chess (BBC) ... by Code Monkey
King") is **GPL-3.0** -- confirmed from the GitHub API, `license.spdx_id`. This
is the engine of the `@chessprogramming591` video series that `CLAUDE.md:20`
names as the source of the `bitboard` branch this one is founded on.

`src/bb_tables.hpp:31-39` `castling_rights[64]` against BBC `bbc.c:1851-1860`:

```
chesso src/bb_tables.hpp:32     "     7, 15, 15, 15,  3, 15, 15, 11,"
bbc.c:1852                      "     7, 15, 15, 15,  3, 15, 15, 11,"
chesso src/bb_tables.hpp:39     "    13, 15, 15, 15, 12, 15, 15, 14"
bbc.c:1859                      "    13, 15, 15, 15, 12, 15, 15, 14"
```

Byte for byte, including the 4-space base indentation -- **which is a formatting
island: every other array in that file is indented 2** -- and including the
extra leading space before `7` and the alignment space before `3`. It sits
inside `// clang-format off` (`src/bb_tables.hpp:5`), which is what preserved it.

`bishop_relevant_bits_count[64]` (`:8-17`) and `rook_relevant_bits_count[64]`
(`:20-29`) against BBC `bbc.c:981` and `:993`. The values are geometric facts
and prove nothing on their own. The whitespace does. Under `cat -A`, in both
files, in **both** tables, rows 1 through 7 end in a trailing space and row 8
does not:

```
bbc.c:982      "    6, 5, 5, 5, 5, 5, 5, 6, $"      <- trailing space
bbc.c:989      "    6, 5, 5, 5, 5, 5, 5, 6$"        <- row 8, none
chesso :9      "  6, 5, 5, 5, 5, 5, 5, 6, $"        <- trailing space
chesso :16     "  6, 5, 5, 5, 5, 5, 5, 6$"          <- row 8, none
```

`grep -c ' $' src/bb_tables.hpp` returns **16** -- the fourteen data rows of the
two tables plus their two comment lines, and nothing else in the file. The
indentation was reformatted 4 to 2 and the comments were capitalised; retyping
does not reproduce another file's trailing whitespace on the same seven rows of
two tables.

Attribution, and it matters. `git blame` puts all three at `9924d3b`
(2025-05-08), `c5cf6508` (2025-05-10, subject **"Implemented CMK make move"** --
CMK is Code Monkey King) and `c57e9d7c` (2025-05-11), on the `bitboard` branch,
by the repository owner, **fifteen months before `achesso` branched**
(`git merge-base HEAD bitboard` = `ca3602e`, 2026-08-02). No agent put them
there. They are inside the inheritance `CLAUDE.md` declares.

**But that same commit `9924d3b` is the one S179 already acted on.** Checked
here: at `9924d3b`, `rook_magic_numbers` and `bishop_magic_numbers` matched
BBC's published arrays **64 of 64 and 64 of 64, position for position**. S179
regenerated them under the project's own seed precisely because inheritance is
not a defence (DEC-132). It replaced one artifact from a commit that contains
four and left three, in the same file, twenty lines apart.

Impact. A direct breach of the COPYING rule ("Never copy tables") and of
`CLAUDE.md:85`, "the owner wants no GPL question anywhere in this codebase".
The legal exposure is thin -- the values are facts and the expression is de
minimis -- but the project's exposure is not legal, it is the accusation that
the work was copied, and this is the cheapest possible proof of copying anyone
looking would find. The three tables are also all trivially derivable: the
relevant-bit counts are `count_bits()` of the attack masks the file already
builds, and `castling_rights` follows mechanically from the WK/WQ/BK/BQ bit
assignment.

Suggested resolution. Derive all three `constexpr` from the engine's own mask
functions, or emit them through `tools/magic_gen` the way S179 handled the
magics, with a provenance comment. The whitespace question disappears with the
constants.

### 2026-09-10_adversarial-F02  medium  Five mask builders in `src/bitboard.cpp` are line-for-line transliterations of the same source, and `CMK_POS` names its author in the shipped header

Status: open. VERIFIED, with a stated caveat.

Evidence. `src/bitboard.cpp` `precompute_knight_attacks` against BBC
`mask_knight_attacks`: the same eight shifts in the same order
(`>>17, >>15, >>10, >>6, <<17, <<15, <<10, <<6`), each with the same file guard
in the same position, `not_h_file` rewritten as `~file_masks[7]` and
`not_hg_file` as `~(file_masks[6] | file_masks[7])`, `bitboard` renamed `board`,
comments dropped. The same shape holds for the pawn and king builders, the
bishop/rook ray loops (same `tr`/`tf`/`r`/`f` names, BBC's `// init target rank &
files` becoming `// Target rank` / `// Target file`) and `set_occupancy`.

**Caveat, stated because it is the honest one:** the eight knight shifts are the
standard bitboard idiom and appear in essentially this form on the Chess
Programming Wiki's *Knight Pattern* page. This finding would be weak on its own.
It is not on its own -- it sits twenty lines from F01's proven copy, in a file
whose magics were proven copies, and a reader who has seen F01 will read these
six functions the same way.

`src/data_structures.hpp:14-18` carries BBC's five debug FENs
(`bbc.c:99-103`) as the same set in the same order, uppercased:
`empty_board`/`start_position`/`tricky_position`/`killer_position`/`cmk_position`
becoming `EMPTY_POS`/`DEFAULT_POSITION`/`TRICKY_POS`/`KILLER_POS`/**`CMK_POS`**.
FENs are facts and carry no licence weight. `CMK_POS` is another author's handle
in this project's shipped source, unexplained anywhere in the repository, and it
is the single line an accusation would screenshot.

Suggested resolution. Rewrite the five builders from the geometry -- one
`(dr, df)` offset table for knight/king/pawn, a step list for the rays. Under
100 lines and behaviour-neutral, so INV-6 discharges it on node counts without a
match. Rename `CMK_POS` for what the position is and `TRICKY_POS` to Kiwipete,
which is what the wiki calls it.

### 2026-09-10_adversarial-F03  medium  Twelve pieces of third-party artwork are redistributed with no licence or attribution, and `books/fetch_book.sh` asserts that this cannot happen

Status: open. VERIFIED.

Evidence. `git ls-files tests/assets/gui/` tracks `Chess_{b,k,n,p,q,r}{d,l}t60.png`
-- Wikimedia Commons' Cburnett naming scheme -- plus `background_l.jpg`,
`flip_icon.png` and `ChessPiecesArray.png`. The originality reviewer matched the
repo's `Chess_bdt60.png` against Commons' 60 px render of `Chess_bdt45.svg` at
98.2 % silhouette agreement (different rasteriser: the repo PNGs carry
`tEXtSoftware www.inkscape.org`). Commons File:Chess_bdt45.svg is by **Cburnett**,
quad-licensed GFDL 1.2+ / CC-BY-SA 3.0 / BSD / GPL -- every one of the four
requires the notice be retained.

The only two `LICENSE.txt` in that directory are both the SIL OFL for the Press
Start 2P font (`head -3` on each: *"Copyright (c) 2011, Cody "CodeMan38"
Boisclair"*). Added `2025-03-09` by the owner, pre-`achesso`.

`books/fetch_book.sh:21-22` states: *"this repository bundles nothing whose
licence is unstated (CLAUDE.md's first foundation, DEC-016)."* That sentence is
false in the same repository that contains it.

Impact. Fixable at zero cost -- the BSD option makes the art MIT-compatible --
but a project whose headline claim is licence hygiene is currently
non-compliant with all four licences it was offered.

Suggested resolution. `tests/assets/gui/THIRD_PARTY.md` naming Cburnett,
electing BSD, with the notice. Delete or replace the three assets whose origin
nobody can state. Correct `fetch_book.sh:21-22`.

### What Part A checked and found CLEAN

- **The magics are the project's own, exactly as S179 claims.**
  `./build/tools/magic_gen magics --seed 20260904` reproduces all 128 constants
  in `src/bb_tables.hpp` **as an ordered list**, verified by parsing both.
  0 of 128 shared with BBC's published set. This is the model the rest of the
  tree should follow.
- **The Zobrist keys** are drawn at run time from splitmix64 under
  `CHESSO_PROJECT_SEED`; nothing is embedded. `tests/test_engine.cpp:200-264`
  really does replay all 851 draws and assert every live key against them, plus
  distinctness and the pair-XOR conditions -- the specs claim is enforced, not
  performed.
- **The piece-square tables have no relationship to any published set.** Against
  PeSTO/Rofchade and CPW's Simplified Evaluation Function: **0 of 64 exact
  matches** on every non-pawn table; the pawn table's 16 are the structurally
  forced zeros on ranks 1 and 8. Correlations are the +0.3 to +0.8 any two sane
  tables share. `CLAUDE.md:88` calls them "hand-written and untuned" and that is
  now stale in the *stronger* direction -- they are fitted to chesso's own
  self-play (`src/eval_tables.hpp:37-42`, `specs.md:476`) -- but a first-read
  document being wrong about the artifact it names is worth one sentence.
- **The opening book's provenance is reproducible end to end**, from a CC0
  source to the shipped bytes, and is the strongest provenance artifact here.
- **Training and tuning data** are chesso self-play labelled by game outcome. No
  external engine appears in `datagen.cpp`, `tuner.cpp` or `tuner_target.hpp`.
- **Vendored dependencies** are submodules, all MIT-compatible.
- **Search, evaluation, UCI, the legal generator, `see_ge`, `attackers_to`**:
  0.00 % 12-token shingle overlap with `bbc.c`. The rewrite above the table
  layer is real and thorough.
- **DEC-105's remediation is genuine**: S180's reseeding of the ten
  prose-quoted constants was spot-checked at HEAD and holds.

**Measured scale of the inheritance, for the record.** 38 % of today's `src/`
(4415 of 11587 lines) is unchanged from the pre-`achesso` branch point, by
`git blame` against `git rev-list ca3602e`. Concentrated exactly where F01 and
F02 are: `bb_tables.hpp` 73 %, `openings.cpp` 68 %, `bitboard.cpp` 52 %,
`data_structures.hpp` 58 %, `utils.cpp` and `log.hpp` 100 %. `search.cpp` is
10 %, `evaluation.cpp` 1 %, `eval_tables.hpp` and `search_params.hpp` 0 %.

---

# Part B -- the second foundation: nothing is believed without a measurement

### 2026-09-10_adversarial-F04  high  Resign adjudication is one-sided in both harnesses, and `rating.sh` states the opposite in the comment that justifies it

Status: open. VERIFIED, and quantified further here.

Evidence. `fastchess.sh:178` and `rating.sh:80` both pass
`-resign movecount=3 score=400`, and neither passes `twosided`. The installed
harness, `fastchess alpha 1.8.1 20260720-daa3ea2` -- the exact build
`.moltke.local.md` records as the one S105, S087 and S145 measured with -- says
in its own `--help`:

> `twosided` - if true, enables two-sided resignation. **Defaults to false.**

`rating.sh:77-79` says:

> `# Adjudication, as fastchess.sh. Both are two-sided -- a resign needs both`
> `# engines to agree for movecount moves -- so an engine whose evaluation is on a`
> `# different scale cannot trigger one alone.`

The comment is inverted, and the property it claims is precisely the one the
rating run depends on: chesso plays Blunder, Leorik and Stash, three
independently scaled evaluations.

How much of the ledger this decides, counted from the tracked PGNs:

| run | games | adjudicated | played out |
|---|---|---|---|
| `S198_calibration.pgn` (A/A) | 1000 | **760 (76.0 %)** | 240 |
| `S088_rated_c6.pgn` (the 2559) | 3340 | **2807 (84.0 %)** | 532 |

**The exposure was then measured rather than argued**, by reading each game's
last score for both sides out of the PGN and asking whether the winner's own
score also reached +400:

| run | decisive adjudications | both sides agreed | **one-sided** |
|---|---|---|---|
| A/A, identical binaries | 676 | 665 | **11 (1.6 %)** |
| S088 rating, foreign evals | 2627 | 2113 | **514 (19.6 %)** |

So in self-play with an unchanged eval scale the exposure is small -- 1.6 % is
the floor, and it is why this has not silently corrupted the search ledger. It
is not small where the comment claims protection. In the run that produced
2559, **chesso conceded alone 311 times and its opponents 203**, and the
direction is opponent-specific:

| anchor | its CCRL | chesso solves to | chesso alone | opponent alone |
|---|---|---|---|---|
| Leorik 2.1 | 2568 | **2476.7** | 107 | 10 |
| Blunder 7.1.0 | 2389 | 2533.8 | 48 | 8 |
| Blunder 8.5.5 | 2664 | 2586.0 | 156 | 4 |
| Stash v21.0 | 2713 | 2597.6 | 0 | 117 |
| Leorik 2.4 | 2829 | 2598.5 | 0 | 64 |

Net one-sided concessions per game against an anchor correlate with that
anchor's solved rating at **r = -0.505, Spearman -0.600** -- the sign the
mechanism predicts: the more often chesso concedes alone, the lower that anchor
rates it. **n = 5, so this is a hypothesis and not a cause**, and the report's
own compression effect is stronger (r = +0.662 against the anchor's CCRL
rating). But DEC-077 attributes the whole 121.8 Elo anchor spread to the time
control and records that attacking it is nobody's step. This is a second
candidate, it has never been considered, and it costs one re-scoring of a PGN
already on disk to test.

`fastchess`'s own documented example uses `score=600`; fishtest uses 600. This
project truncates games earlier than the practice it copied.

Suggested resolution. Decide `twosided=true` or record the exposure, with a
step. Either way delete the false comment now: it is the sentence a future
reader will rely on. First measurement is free -- re-score
`S088_rated_c6.pgn` per side, which is what the table above already is.

### 2026-09-10_adversarial-F05  medium  The engine's `id name` carries no version, so the identity check the project enforces on every opponent it cannot enforce on itself

Status: open. VERIFIED.

Evidence. `src/chesso.cpp:1074` is `uci_reply("id name Chesso");` -- a literal,
with no version, no commit, no build configuration. `printf 'uci\nquit\n' |
./build/src/chesso` answers `id name Chesso`.

`fastchess.sh:405-406` labels the two sides `name=candidate` and
`name="ref-$ref_sha"`, from the script's own variables. Nothing in the loop asks
either binary who it is, so every archived PGN's engine names are an assertion
of the script rather than a property of what played.

Against `references.tsv:6-9`: *"uci_name must be exactly what the binary answers
to `uci` with `id name`. rating.sh asks every engine before it plays a game and
refuses the run on a mismatch. That check exists because the first install ...
put the Rustic workspace development build (`engine 3.99.36`) at
/usr/games/rustic instead of the alpha-3.0.6 tag build, which is a different
file."* DEC-068, DEC-069 and DEC-072 apply the same rule to Leorik, Blunder and
Stash. `grep -n -i "id name" adocs/decisions.md` returns those opponent rows and
nothing about chesso's own.

Impact. DEC-020 -- the run that reported +301 Elo and meant nothing -- is this
class. The banner's sha pair is a good guard and does not close it: the banner
describes what the script *intended* to build, and the PGN inherits that
intention rather than recording what ran.

Suggested resolution. Stamp `git describe`/short sha and the arch into `id name`
at configure time, and have `fastchess.sh` assert each side's `id name` against
the sha it labelled it with. About twenty lines, and it closes the loop the
project already enforces on everyone else.

### 2026-09-10_adversarial-F06  medium  `.ref-builds/` binaries are reused forever with nothing checking the worktree, the build's freshness, or its configuration

Status: open. VERIFIED (the mechanism; no contaminated verdict found).

Evidence. `fastchess.sh:356` is `if [[ ! -x "$reference" ]]; then` -- the
worktree is created and built only when the binary is absent. Nothing
afterwards checks that the worktree is still at `$ref_sha`, that the build is
current with it, or that it was configured like the candidate. Measured over the
30 directories on disk: `.ref-builds/2b54a4f` is **dirty** (` M DEV_MANUAL.md`),
and 11 of the 30 have no `CHESSO_ARCH` in their `CMakeCache.txt` because they
predate S104 -- `objdump | grep -c popcnt` returns **0** on those binaries and
159 on every later one and on `build/src/chesso`.

`fastchess.sh:362-364` configures the reference with bare defaults; the
candidate is whatever `build/` was last configured as. They agree today by luck
-- `cmake/arch.cmake:32` defaults `CHESSO_ARCH` to `native`, so both get
`-march=native` -- not by check, and `CHESSO_TUNE=ON` is by DEC-118's own words
*"deliberately different code"*. The banner prints tc, hash, concurrency, book
and seed, and never the configuration.

**No recorded verdict is contaminated.** Every cached binary's mtime is within a
day of its commit date and the popcnt split falls exactly on S104's date
(2026-08-19), so the stale-reference path has not fired. `DEV_MANUAL.md:2069`
nonetheless teaches `REF=7b4d9a4`, which today serves a 2026-08-13 binary with
no hardware popcount against a native candidate -- S104 measured that difference
at +12.62 % nps for the arch flag alone.

Suggested resolution. Rebuild the reference unconditionally -- ccache makes it
nearly free -- or record the reference's arch/tune/pgo and refuse when they
differ from `build/CMakeCache.txt`. Print the candidate's configuration on the
banner.

### 2026-09-10_adversarial-F07  medium  `adocs/specs.md` says `./rating.sh` re-derives 2559; it cannot, and the changed variant of that run was voided by the project

Status: open. VERIFIED.

Evidence. `adocs/specs.md:32-33`: *"`adocs/data/rating_2026-08-18_S088_ccrl_blitz.md`
is the record and **`./rating.sh` re-derives it**."*

| | the S088 run | `rating.sh` at HEAD |
|---|---|---|
| hash | **64 MB** (`rating_2026-08-18_S088_ccrl_blitz.md:67`) | `hash_mb=128` (`rating.sh:55`) |
| concurrency | **6** (`:70`, *"Concurrency 6 is this figure only"*) | `${CONCURRENCY:-$all_cores}` = 12 here (`rating.sh:75`) |
| openings | 334, unseeded | 334, unseeded, a different draw (`rating.sh` passes no `-srand`) |

The report itself records that the five-engine concurrency-12 run was voided as
`S088_rated_c12_INVALID.pgn` and was *"the outlier of the three, by 5 to 7
points on Blunder 8.5.5 alone"* (`:154`).

Impact. The one reproduction path the project offers for its headline strength
figure runs a different experiment, with nothing saying so, in the file whose
job is to be the current truth.

Suggested resolution. Either gate the S088 regime behind a flag, or say what
`rating.sh` does now and that 2559 is not re-derivable at HEAD.

### On the +/-25, stated plainly

`specs.md` calls 2559 "95 % +/-25, soft" and names the softness. The **+/-25 is
one anchor's `ordo` solve**, and four documents quote it as the interval on the
number. Terms it does not contain: the anchor-choice dispersion (SD **52.9**
over the five solves, range 121.8), the anchors' own CCRL error (+/-11 to +/-18),
the 334-openings-each-played-ten-times sample (`rating.sh:120` sets
`rounds=334`, and fastchess truncates the book to `rounds`; verified in the PGN
-- 3340 games, **334 distinct first-16-ply signatures, every one played exactly
10 times**, while `books/8moves_v3.pgn` holds 34700), the time control, and
F04's adjudication. The measurement reviewer also notes the CCRL list is
BayesElo and `ordo` is logistic, which is a second candidate for the compression
and points the wrong way for it.

**The defensible statement is that the interval is no narrower than +/-60 from
anchor choice and statistics alone.** This is not a criticism of the number's
honesty -- `specs.md:44-53` refuses to sum the kept SPRT estimates into the
headline and says so at length, which is the right instinct. It is the one
figure left in the file that reads more precisely than it is.

---

# Part C -- correctness defects the engine ships

### 2026-09-10_adversarial-F08  high  A two-fold repetition whose first occurrence is in the pre-root game history is scored as a dead draw, and a fast-suite test pins that as correct

Status: open. VERIFIED with two oracles.

Evidence. `src/bitboard.cpp:1412-1437` `is_position_repeated` scans back over
the whole history window and returns `true` on the **first** hash match. Nothing
in it or in its caller (`src/search.cpp:630`) distinguishes a match inside the
search tree from one before the root.

Driven through python-chess, which waits for `bestmove` -- **not** a bare pipe;
see Part F -- same board both times, `go depth 10`:

```
with history 'g1f3 g8f6 f3g1'   depth=10  score=0     nodes=4249    pv=['f6g8']
identical board, no history     depth=10  score=-900  nodes=325965  pv=['f6d5','g1f3','f7f6']
```

Oracles. python-chess 1.11.2 on the position after `f6g8`: `is_repetition(2)=True`,
`is_repetition(3)=False`, `can_claim_threefold_repetition()=False`,
`is_game_over(claim_draw=True)=False`. **No draw exists.** Stockfish at depth 18,
given the identical board and move history: **-687**, playing `Nd5`.

The node count falls by a factor of 77 because the search prunes the tree behind
a draw that is not there.

**And the suite asserts it.** `tests/test_search.cpp:3033`, *"the losing side
takes an available repetition"*, plays `Ra2b2 Kh8h7 Rb2a2` from
`7k/8/8/8/8/8/R7/K7 w - - 0 1` and requires a draw score; its comment at
`:3029-3031` states the mechanism as intended -- *"be seen through the moves
played before the search started"*. python-chess on that exact position after
`Kh7h8`: `is_repetition(3)=False`, `can_claim_threefold_repetition()=False`,
`outcome(claim_draw=True)=None`. **No draw exists there either.**
`grep -in 'repetit' adocs/decisions.md` finds no entry recording the convention.

Impact. Ordinary play, unlike F09 and F10. The engine publishes `score cp 0` for
positions it is losing by several pawns, and chooses moves on that score.
`tools/analyse_game.py` -- the tool `CLAUDE.md` mandates *because* agent chess
judgement is banned -- reads those scores. The refinement that fixes it is
standard: distinguish a match at or after the root from one strictly before it
(CPW *Repetitions*; Stockfish PR #925, which measured the same class and
whose own example is a losing move preferred over a drawing one).

Suggested resolution. Plumb the root's `history.size` into `search_state_t` and
require the match to be at or after it for a one-match draw, two matches
otherwise. **The test must be re-stated in the same change**, and under the
project's TESTS rule -- *"Never relax a test, never delete one to get green ...
If a test looks wrong, stop and tell me"* -- that is the owner's call, not an
agent's. This finding is the reason to make it.

### 2026-09-10_adversarial-F09  high  `generate_moves()` overruns its 270-entry stack buffer, so one `position fen` line aborts the shipping binary

Status: open. VERIFIED.

Evidence, against `build/src/chesso`, the Release binary that ships and that
every SPRT measures. The FEN loads cleanly first -- the `fen` command echoes it
back verbatim, so this is not a rejection path:

```
$ printf 'position fen QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1\nfen\nquit\n' | ./build/src/chesso
QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1

$ printf 'position fen QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1\ngo depth 4\nquit\n' | ./build/src/chesso
*** stack smashing detected ***: terminated
exit code: 134            # SIGABRT, core dumped
```

`build-sanitize/src/chesso` names it: `stack-buffer-overflow ... WRITE of size 4`
in `generate_moves_body<WHITE,false,GEN_QUIETS>` at `src/bitboard.cpp:375`, from
`generate_quiets` at `:571`, from `negamax_at` at `src/search.cpp:933`, into
`'moves' (line 876)`.

Mechanism. `src/data_structures.hpp:40-41`: `// The maximum number of legal moves
that is possible to generate` / `#define MAX_MOVES 270`. True of legal positions
(the known maximum is 218). `load_FEN`'s S161 sanitiser repairs only castling
rights and the en-passant square and says so: *"Position legality at large --
pawn counts, two kings, the side not to move in check -- is deliberately not
checked: only the two classes that corrupt."* Piece counts are a third class
that corrupts. The only guard is `assert(move_count < MAX_MOVES)`
(`src/bitboard.cpp:463`) -- **dead under `NDEBUG`, and both gated builds are
Release**. Every caller declares the buffer on the stack, and
`src/search.cpp:876` appends `generate_captures` and `generate_quiets` into the
*same* 270-entry array.

The reviewer's direct maximiser over placements `load_FEN` accepts reaches
**277** moves from two independent seeds. Constrained to <=16 pieces a side the
maximum found was 224, so the trigger needs an impossible piece count, not
merely an illegal position.

Impact. A crash, from the shipping UCI surface, in the shipping binary. Not
reachable from legal play, and no GUI sends such a FEN -- but `position fen` is
also how corpora, `tools/pgn_to_positions`, `datagen` and any fuzzer reach the
engine, and a near-miss corrupts the adjacent `scores[]`/`quiets_tried[]`
instead of aborting. `src/openings.cpp:666` already uses the right shape
(`for (...; moves_count < MAX_MOVES; ++i)`); the generator does not.

Suggested resolution. Bound the generator at run time, or extend the S161
sanitiser to refuse a side piece count above 16, and state the chosen bound in
`specs.md`. Raising `MAX_MOVES` alone is not enough -- 277 is a search result,
not a proof.

### 2026-09-10_adversarial-F10  high  A pawn on the first or eighth rank indexes the passed-pawn table out of bounds, read and write, from `position fen`

Status: open. VERIFIED.

Evidence. `src/evaluation.cpp:511` computes `bucket = 6 - (get_lsb_index(white_passed) >> 3)`
and `:524` computes `bucket = (get_lsb_index(black_passed) >> 3) - 1`. Under the
project's own sanitizer build, both colours:

```
$ printf 'uci\nposition fen P7/8/8/8/8/8/8/K6k w - - 0 1\ngo depth 4\nquit\n' | ./build-sanitize/src/chesso
src/evaluation.cpp:516:36: runtime error: index 6 out of bounds for type 'int [6]'

$ ... position fen K6k/8/8/8/8/8/8/p7 b - - 0 1 ...
src/evaluation.cpp:529:36: runtime error: index 6 out of bounds for type 'int [6]'
```

The Release binary runs it and answers: `info score cp 418 ... bestmove a1b2`.
python-chess on that FEN: `status 32` (`STATUS_PAWNS_ON_BACKRANK`),
`is_valid() False`.

**The code names the hazard and guards it with a dead assert.**
`src/evaluation.cpp:507-510`: *"What keeps it in range is that a pawn cannot
stand on rank 1 or rank 8; the debug build checks that instead of trusting it,
because the penalty for being wrong is a read off the end of a six-entry
table."* `assert(bucket >= 0 && bucket < 6)` at `:514` and `:527`, compiled out
in Release, and nothing at the load boundary enforces the precondition the
comment relies on.

The collecting instantiation is worse than a read: `:519` and `:535` do
`passed_out[colour][bucket]++` into the caller's `int[2][6]`, so
`passed_pawn_counts()` writes one `int` past the end of the caller's array.

**The gate structurally cannot catch it.** `tools/gate_extra.sh` stage 4 runs
the sanitizer build's `ctest -L fast` and its `bench`; neither loads a
back-rank-pawn FEN. A 20 000-FEN fuzz of syntactically valid placements through
`evaluate` and the count accessors under ASan+UBSan produced **exactly one**
distinct report site -- this one -- with 6138 of the 20 000 carrying a back-rank
pawn.

`grep -rn -i "back-rank pawn\|pawns on the first"` over `adocs/audit/`,
`adocs/decisions.md` and `adocs/plan_todo/` returns nothing: not previously
recorded. S161's `excludes:` names *"pawn counts, doubled kings"* but not this.

Suggested resolution. Refuse or repair at the load boundary, as S161 did for the
other two semantic classes, with a red-first test. Clamping the bucket inside
`evaluate_pawns` would hide an invalid board rather than refuse it.

### 2026-09-10_adversarial-F11  medium  `clean-tt` clears the transposition table without joining the search, and ThreadSanitizer reports the race

Status: open. Reported by the board/UCI reviewer under TSan; the code path
verified here, the TSan run not re-executed.

Evidence. `src/chesso.cpp:1894-1900` `command_clean_TT` calls `tt_reset`
directly. `src/chesso.cpp:122-124` states the rule it breaks: *"Every path that
mutates the board or the transposition table, or that ends the process, must
call this first"* -- `stop_and_join_search()`. A TSan build driving
`go infinite` + 40 x `clean-tt` produced 11 reports
(`tt_get_entry` at `src/transposition_table.cpp:101` racing `memset` in
`tt_reset` at `:72`); the same harness with `isready`, `setoption name Hash`,
`position`, `ucinewgame`, `fen`, `pb`, `bench` and `test` in its place produced
**0** each.

Impact. `memset` clears low-to-high, so a concurrent probe can match a key that
has not been cleared yet and read a zeroed score with an un-zeroed type -- a
node returning 0 where a real score was stored. A custom command a GUI never
sends, which is why it is medium; it is documented in `MANUAL.md` and reachable
from any script.

Suggested resolution. One line: `stop_and_join_search();` at the top of
`command_clean_TT`.

### 2026-09-10_adversarial-F12  medium  `setoption` is case-sensitive, against the spec, and the shipping binary says nothing when it refuses

Status: open. VERIFIED.

Evidence. The repository's own copy of the spec, `UCI.txt:90`: *"The name and
value of the option in `<id>` should not be case sensitive and can inlude
spaces."* Asked of the tune build, which has a refusal channel:

```
Hash               -> (accepted)
hash               -> info string refused [hash], unknown option
HASH               -> info string refused [HASH], unknown option
OwnBook            -> (accepted)
ownbook            -> info string refused [ownbook], unknown option
Best Book Move     -> (accepted)
best book move     -> info string refused [best book move], unknown option
```

`src/chesso.cpp:1175-1223` compares with `==`. The reviewer measured the same
for check values -- `true` honoured, `True` and `TRUE` ignored.

**And the Release binary is silent**: the same `setoption name hash value 64`
against `build/src/chesso` prints **0** lines containing "refused", because
`LOG_W` is `if (false)` under `NDEBUG`. A harness that writes `hash` or `True`
measures the default and never learns -- the failure class DEC-093/S137 exists
for, one option along.

Suggested resolution. Case-fold the option name and the check values before
comparing. The `Book File` *value* must stay verbatim: it is a path.

### 2026-09-10_adversarial-F13  medium  `Hash` is parsed with `std::stoll`, so `0x40` silently buys 1 MB, in the same function that already does this correctly

Status: open. VERIFIED by reading both parse sites; the reviewer's node-count
probe is the behavioural evidence.

Evidence. `src/chesso.cpp:1208-1212`:

```cpp
if (option_name == "Hash") {
  try {
    const long long megabytes = std::stoll(option_value);
```

`std::stoll` stops at the first character it cannot use: `"0x40"` -> `0` ->
clamped to `TT_MIN_MB`; `"64abc"` -> 64. The reviewer's depth-13 node counts
confirm both (`0x40` identical to `Hash 1`, `64abc` identical to `Hash 64`).

Twenty-odd lines below, `src/chesso.cpp:1256-1270` parses the tune build's
search parameters with `std::from_chars` **and requires `parsed.ptr == last`**,
reporting `info string refused [...] value ..., not an integer` otherwise --
because, in its own words, *"`0x50` set 0, `120.9` set 120 and `12x` set 12,
each of them silently -- the same failure this step exists to remove."* `Hash`
was never given that treatment. `MANUAL.md:52` states *"A non-numeric value is
ignored with a warning in the log"*, which is true for `abc` and false for
`0x40` and `64abc`.

Impact. DEC-088 pins the SPRT harness's hash deliberately; a typo that silently
buys 1 MB instead of 64 measures an engine nobody configured.

Suggested resolution. Use the `from_chars`-with-`ptr == last` form, report
through the S137 `info string` channel which is legal UCI in every build state,
and correct the `MANUAL.md` row.

---

# Part D -- the goal, and the distance to it

This part is the axis the request named and it contains no defect. It is what
the measurements say about whether phase one is on course.

### 2026-09-10_adversarial-F14  medium  No measured Elo has been added in nineteen days, and the next six steps in the plan's order contain no engine feature

Status: open. VERIFIED.

Evidence. The SPRT ledger, built by grepping every `plan_done/` stamp for a
verdict: **the last kept positive verdict is S093, 2026-08-22, +10.73 +/- 6.70.**
In the nineteen days since, across 30-plus completed steps and roughly 50
commits, the only Elo figures produced are S171 `+3.24 +/- 8.91` (a null on a
reporting-only change) and S148 `-5.66 +/- 4.33` -- a challenger rejected after
14808 games and 6 h 19 m, correctly, to confirm the incumbent. Everything else
that landed is tests, tooling, documentation, mate-PV reporting, book loading,
the magic and Zobrist reseeding, and gate infrastructure.

`plan.md`'s Open list, positions 1 to 6: S194 (a test), S151 (a measurement
policy that re-runs verdicts at 4x control), S181, S185, S182, S183 -- the last
four touch only `plan.md`, `decisions.md` and `adocs/data/`. **The first step
that adds strength is #7, S024.** Across the whole list, 8 of 57 open steps
touch no `src/` at all and 6 of those 8 are edits to the plan document itself.

Distance to the goal: **+441 Elo** from the 2559 anchor.

Impact, stated as the trade it is. None of that work is waste -- the bench-signature
gate, the mutation checker, the extra gate and the INV-4 enforcement in Release
are real risk reduction, and S148 and S159 are the project's own rule working
exactly as designed. But the ratio has inverted, and the plan's own instruments
say so: S182 exists because the cost line reads 45-to-75 minutes a verdict
against a measured mean of 4 h 40 m, and S183 exists because the headline
feasibility claim -- *"The midpoint clears 3000"*, `plan.md`, resting on a
~60 % transfer of published figures -- has never been checked against this
project's own transfer ratio. Where both numbers exist that ratio is far below
0.6: S093 reported +37.5 / +28 and measured +10.73; three published figures
were taken at face value and measured 0, 0 and *slower*. S183 is the step that
would settle it and it is sixth in a queue that has not moved in nineteen days.

Suggested resolution. None proposed -- this is the owner's call on ordering, not
a defect. Recorded because the request asked about the goal, and because S182
and S183 are the two steps that would turn it from an impression into a number.

### 2026-09-10_adversarial-F15  medium  There is no parallel search anywhere in the code or the plan, and the stated end goal requires one

Status: open. VERIFIED.

Evidence. `src/chesso.cpp:1081` advertises
`option name Threads type spin default 1 min 1 max 1`, and `:1220` logs and
ignores any other value -- honest, and better than an option that lies. But
`grep -i` for SMP, Lazy SMP, multi-thread or parallel search over `plan.md`,
`specs.md` and all 57 files in `plan_todo/` returns **no step, no reserve entry
and no decision**; the only hits are incidental mentions inside S097 and S120
about other engines.

`CLAUDE.md` and `specs.md:8` state the end goal as *"the strongest CPU chess
engine in the world"*. Phase one's target is the CCRL Blitz **1CPU** list
(DEC-089), which single-threaded play satisfies -- so nothing here is wrong
today. The gap is that the 1CPU framing makes a structural requirement of the
end goal invisible rather than deferred: there is no record deciding when, or
whether, SMP enters.

Suggested resolution. One decision entry placing parallel search in phase two
with a reason, or a reserve step. Either makes it a choice on the record.

### 2026-09-10_adversarial-F16  medium  The whole non-material part of the evaluation is clamped to +/-184 centipawns, and what that costs has never been measured

Status: open. VERIFIED.

Evidence. `src/evaluation.cpp:1042-1048` `evaluate_expensive()` clamps
mobility-plus-king-safety to `+/-LAZY_EVAL_MARGIN` **before** the side-to-move
sign, so `evaluate()` *is* the clamped function and the lazy shortcut's bound
holds identically. That is a genuine soundness proof and it is rare -- see Part
F. Its price is that everything outside material and the piece-square tables can
never exceed 1.84 pawns.

S039's own recorded numbers: the combined correction reaches **279** and exceeds
the margin on **0.364 %** of corpus positions, so up to 129 cp of an
already-computed term is discarded on roughly one position in 275. The
evaluation reviewer re-measured at the shipping weights and the spread has grown
since that figure was written: `eval_spread` over 1.5 M rows now reads p99
**153**, max **401**, against the `src/evaluation.cpp:659-662` comment's "p99
128 and max 330".

The path is planned -- S039 sizes the margin, S122 removes the clamp -- and
S039's file already records that it is *"a prerequisite for S122 rather than a
micro-tune: the clamp it sizes is what caps king safety."* Two things are worth
saying anyway. **What the clamp costs today is unmeasured**: no SPRT or fit
prices it. And S039 sizes the margin *"at the weights that ship at this step's
own HEAD"*, while `plan.md`'s Open order puts S121 (mobility curves), S123,
S125 and S101 -- four steps that change the very sum being clamped -- between
S039 and S122.

Suggested resolution. None beyond ordering: consider whether S039's measurement
survives the four steps scheduled after it, or belongs beside S122.

---

# Part E -- lower-severity findings

Each verified by the reviewer that raised it; not independently re-run here
unless marked.

**F17 low `halfmove_clock` is a `uint8_t` and wraps at 256.** `src/data_structures.hpp:311`;
incremented at `src/bitboard.cpp:855` and `:1070`. `position startpos moves`
with 256 reversible plies gives clock 0; 300 gives 44. Both consumers then fail:
the `>= 100` draw test at `src/search.cpp:656` stops firing, and
`is_position_repeated`'s window `min(halfmove_clock, history.size)` collapses to
0, so the whole subtree has neither draw rule. `load_FEN` explicitly *refuses* a
clock above 255 with the comment *"Truncating a larger value would silently
reset the fifty move counter and shrink the repetition search window"* -- the
exact harm the increment then produces. Out of reach of any adjudicated match;
silent when it happens. Fix: `uint16_t`, or saturate at 255 with the reason.

**F18 low a long `position` line makes the engine answer `bestmove 0000` in a
position with 22 legal moves.** At 4999 plies the root's own `make_move` refuses
(`src/bitboard.cpp:759`), every move is skipped, and `first_legal_move()`
cannot rescue it because it calls `make_move` too. Clean under ASan. The comment
at `src/bitboard.cpp:757` -- *"The search can never reach this - it is bounded
by MAX_PLY"* -- is wrong: the search starts from whatever size the `position`
line left. `make_null_move` has only an assert, dead in Release, unreachable
today solely because NMP forbids two passes in a row, which is written down
nowhere.

**F19 low `go infinite depth N` returns without a `stop`.** `UCI.txt:171`: *"Do
not exit the search without being told so in this mode!"* `command_go`
pre-seeds `depth = MAX_DEPTH` (`src/chesso.cpp:1486`) and `infinite` does not
clear it. A second trigger for `2026-09-04_adversarial-F01`, which is still open
and whose description does not cover this one.

**F20 low `movestogo 0` is clamped to 1**, so the engine spends ~46 % of its
clock on one move (measured: 4.64 s of a 10 s clock, identical to `movestogo 1`).
`src/chesso.cpp:1523` clamps rather than rejects, while `src/uci.hpp:47-50` and
S089 are explicit that 0 means sudden death and *"is not a stand-in for some
number of moves"*. Needs a non-conforming GUI.

**F21 low the first iteration cannot be stopped.** `src/chesso.cpp:777-778` sets
`state.stop = &never_stop;` and swaps in the real signal only after the first
`search()` returns, so depth 1 ignores both `stop` and the hard timer. Measured
exposure in ordinary play is small -- depth-1 wall time over 400 positions from
`S018_raw.tsv`: median 0.56 ms, p99 1.04, max 1.34. On a pathological but
`STATUS_VALID` board the reviewer measured 254 ms used against a 100 ms clock
(a 50 ms hard limit) -- a forfeit. Latent, not a current match risk.

**F22 low quiescence never tests insufficient material.** `is_insufficient_material`
has one call site, `src/search.cpp:668`, inside `negamax_at`. A capture made
inside quiescence that leaves KvK, KNvK or KBvK is scored statically (-103,
+190, +235 measured) instead of `DRAW_SCORE`.

**F23 low same-coloured bishops are not drawn, and the stated reason is wrong.**
`src/bitboard.cpp:1393-1409` counts minors and never asks about square colour;
`evaluate()` returns +598 for `8/8/8/4k3/8/8/8/2B1K1B1 w` (both bishops dark),
and the search reports +828 at depth 14. python-chess `is_insufficient_material`
returns True. The comment at `:1389-1392` justifies the omission as *"drawn in
practice but not by the laws"* -- FIDE 5.2.2 and 6.9 say otherwise for this
family, though the comment's argument does hold for KNvKN. The behavioural half
is recorded (`2026-08-14_test_review-F05`, pinned by S067); the false
justification is not.

**F24 low the tuner's gradient ignores the model's clamp on a stale
justification.** `tools/tuner_model.hpp:391-396` defends it with *"the term's
maximum over 149084 real positions was 143 against a bound of 150"* -- a
mobility-only figure at the old margin, while `tools/eval_model.hpp:966-975`
already says the opposite. Measured today at the shipping weights over 400 000
rows: 0.33 % of rows clamped, carrying 0.44 % of the stage-two gradient
magnitude. `tests/test_tuner_gradient.cpp:357` and `:580-585` choose a fixture
where no row is clamped, so the finite-difference check has **zero coverage** of
the region where the gradient is knowingly not the derivative.

**F25 low `tools/eval_spread`'s candidate margins are `{150, 200, 250, 300,
400}`** (`tools/eval_spread.cpp:32-35`) and do not include the 184 that has
shipped since S085 -- in the tool whose entire job is S039's re-decision.

**F26 low stale zero-weight comments in `src/evaluation.hpp`.** Five blocks say
*"Zero until the tuner fits them"*; the dependent claims are the defect.
`:253-256` -- *"only because the weights ship at zero. While they do, comparing
the engine's score against the tuner's model compares 0 against 0 ... The counts
themselves are the only thing that can be checked today"* -- guards
`king_safety`, whose weights are `{17,20,19,34,-27,26,14,-40,-25}` at
`src/evaluation.cpp:790`. Same shape at `:60`/`:71-73` for `passed_pawn`
(`evaluation.cpp:80-81`, non-zero). S039's own file records this exact staleness
class in this exact file for a different sentence; the file was never swept.

**F27 low the "static score cannot approach the mate band" argument names the
wrong bound.** `src/search.cpp:775-779` and `src/search_params.hpp:119-122`
attribute it to the `evaluate_expensive()` clamp, which bounds only the
expensive stage; `evaluate_cheap` is bounded by nothing. The conclusion is true
-- measured max `|evaluate()|` 14391 over 80 000 pathological placements, table
bound 16991, against `MATE_MIN` 48000 -- but the reason given does not
establish it, and no test asserts `|evaluate()| < MATE_MIN`.

**F28 low `tools/analyse_game.py` can silently return 0.00.** `:41-57`
initialises `score, best = 0, "-"` and returns them if no `info` line carrying
both a score and a pv arrives; it also accepts `lowerbound`/`upperbound` lines.
This is the tool `CLAUDE.md` mandates *because* agent chess judgement is banned,
so a silent 0.00 is the DEC-023 failure mode arriving through the instrument
that exists to prevent it.

**F29 low `spsa_driver.py check` proves reachability for one axis.** It
validates every parameter's presence and bounds, then probes that a value
actually reaches the search with a hardcoded `RfpMargin` node comparison -- one
of the twelve axes S085 tuned. An axis wired to a variable nothing reads would
random-walk and be indistinguishable from a tuned one.

**F30 low S199's drift reading rule has no term for DEC-083 conversions.** It
reads a point as *"inside the previous point's interval plus the verdicts landed
since"*. Block 2 is eight speed steps that land no verdict at all and are
discharged by timing conversion -- `plan.md` says *"Six steps stop owing a match
at all under DEC-083"* -- so a drift point taken after block 2 reads high
against a rule with no slot for them.

**F31 low `fastchess.sh` censuses crashes and disconnects but never voids on
them**, where `rating.sh:250-256` greps for the same and prints
`RATING-RUN-INVALID`. The stated justification covers time forfeits only (*"Both
sides here are chesso"*), which does not extend to a crash, since only one side
carries the new code. Never observed: 17682 tracked games show only
`adjudication`, `normal` and 4 `time forfeit`. Given F09, worth closing.

**F32 low the busy-machine guard carries no information.** `fastchess.sh:370-377`
sums `ps -A -o %cpu=`, which is each process's *lifetime* average, and warns
above 60. Measured here: loadavg 0.79, `ps` sum **255** -- 3.2x over-reported, so
it fires on every run. `CLAUDE.md` founding rule 4 recommends the same statistic.

**F33 low dead API and stale comments.** `swap_side` and `set_en_passant`
(`src/bitboard.hpp:152-153`) have zero callers anywhere -- and `set_en_passant`
is a hash-mutating public entry point that bypasses the S161 sanitiser.
`is_uint` (`src/utils.cpp:12`) and `trim_whitespace` (`src/chesso.cpp:215-217`)
pass a possibly-negative `char` to `isdigit`/`isspace`, UB outside `unsigned
char`. `src/data_structures.hpp:437` still says *"20 bytes of content in 24"*;
it is 22 since the `eval` field landed (`sizeof` is 24, which is the
load-bearing half and is right).

**F34 low the tuner has no regularisation and no decision recording that**, over
a parameterisation whose own source documents two exact linear degeneracies at
R2 = 1.000000 (`src/evaluation.cpp:228-238` and `:59-73`). S134 deletes the
degenerate columns, which is a valid alternative answer; the absence is
currently an omission rather than a choice on the record.

**F35 low a fit is not reproducible from its own provenance stamp.**
`tools/tuner.cpp:249-282` emits the commit, corpus digest, K, seed, lr and
split, but not `--epochs`, `--report`, `--patience` or `--threads` -- and
`--report`/`--patience` select *which vector is emitted* (`:713`, `:735`).

**F36 low the tuner trusts the corpus's `phase` column** (`tools/tuner_model.hpp:158`,
range-checked only) instead of recomputing it with `eval_model::phase_of()`,
which sits in the same header. Verified equal today over 456 304 rows, 0
disagreements. Latent: S134 and S117 both move `phase_value`, after which every
existing corpus tapers on the wrong phase and the range check still passes.

**F37 low `eval_tuning_strategy.md` diverges from the code in two unrecorded
places**: it specifies mini-batch Adam at batch 16k-64k and the code is
full-batch (`tools/tuner.cpp:434`), and its Phase A requires an in-engine eval
trace and UCI exposure of every tunable, neither of which exists -- the trace
lives in `tools/eval_model.hpp` as a second implementation. The document's other
divergences (leaf labels, non-linear king safety) are recorded and planned.

---

# Part F -- checked and clean

**Every re-derivable number in `specs.md` reproduced exactly at HEAD.**
`bench` **26851183**. `search_bench` depth 9 **121530 / 801481 / 72924** and
depth 12 **636677 / 3520847 / 494098**, best moves `c3d5` / `e2a6` / `d7c8q` at
both. `ctest -L fast` 33/33 in `build` and 33/33 in `build-tune`. `ctest -L slow`
1/1, `test_perft` green over the nine columns S205 added. The extra gate's last
run is stamped in `status.md` at `4795ef4`, today, and `4795ef4` is an ancestor
of HEAD. On a project this heavily documented, the documents being *true* is the
finding that took the most work to establish and it held everywhere it was
checked.

**Move generation, make/unmake and hashing: no defect found, under real
pressure.** 500 random positions against python-chess -- legal-move sets and
perft(3) identical, 0 mismatches. A systematic en-passant rank-pin sweep, 280
valid positions, **0 mismatches** -- the classic bug is not present. A castling
sweep with attackers placed on every square, 775 valid positions, 0 mismatches.
A fuzz over 120 games taking **every** legal move at every node, comparing
`board_t` byte for byte after `unmake_move` plus hash-against-full-recompute,
`squares_match_bitboards`, `eval_accumulators_match` and all three occupancies:
**0 failures**, promotion, promotion-with-capture, en passant and castling
included. INV-3 holds at every node of the same fuzz. `make_null_move` /
`unmake_null_move` round-trip byte for byte.

**The transposition table's semantics are right.** Full 64-bit key compare;
`tt_entry_answers` rejects on `depth <`, returns exact unconditionally, lower
bound only at `>= beta`, upper only at `<= alpha`; the TT cutoff is guarded
`!is_pv && ply > 0`; `TT_DEPTH_QS = -1` genuinely cannot answer a main-search
probe. **A stored move can never be played illegally** -- it is used only by
`score_move` for ordering, so a collision matches nothing the generator emitted,
and `complete_mate_pv` re-validates against the generator
(`src/search.cpp:1465-1474`). Mate scores round-trip through
`normalize_score`/`de_normalize_score` with a deliberate, documented asymmetry
at `+/-MATE_MAX` and a `static_assert` holding the band open.

**The lazy-evaluation shortcut is sound by construction, not by sampling.**
`evaluate_expensive()` clamps before the sign and `evaluate() = cheap +
expensive`, so `cheap -/+ MARGIN` are true bounds identically, and
`evaluate_lazy` returns exactly those. The audit brief asked whether the margin
is proved or merely fitted; it is **proved**. F16 is its price, not a hole in it.

**PVS, null move and LMR are correctly built.** No double null, in-check guard,
both mate-band edges, `depth-1-R >= 1`, and a mate out of a null search
downgraded to beta. LMR does reduced null-window, then full-depth null-window on
`> alpha`, then full-window on `alpha < score < beta`, with the reduction
clamped to leave a ply. Every aborting path returns without a TT store. 300
positions at `go depth 9` and 200 at `go movetime 30`: **0 illegal PV moves, 0
`bestmove != pv[0]`**.

**Protocol robustness is good.** `isready` answered in 0.000 s *during* a live
search -- the spec's hard requirement, and one many engines fail. Empty lines,
non-ASCII, control bytes, a 200 000-character line, 5000 `wtime` tokens, bare
`setoption`, `stop` before `go`, `go` while searching, EOF at every point:
survives all of it, exits 0, no stray output, no hang. `go movetime {1,2,5,20}`
worst overshoot 0.9 ms; `go infinite` + `stop` latency 0.4 ms; `go nodes N`
exact. TSan clean on eight mid-search command paths -- F11 is the one exception.

**The SPRT itself is correctly specified and correctly read.** Pentanomial by
default and the normalized model's arithmetic checked against the source;
`-repeat` gives one opening per round with colours reversed and the result is
read pairwise; openings never repeat *within* an SPRT run (verified: S198's 1000
games are 500 distinct FENs); `-srand` is validated, printed and carried into
every `[Event]` header. Draw adjudication *is* effectively two-sided. The A/A
control exists, was run, and was read -- 500 pairs, variance 0.2430 +/- 0.0154
against S105's 0.2395, z = +0.16, 0 forfeits in 1000 games at concurrency 12.
The SPSA implementation matches Spall's `a_k/c_k` exactly, with Rademacher
perturbation, clamping before `setoption`, and a config digest guarding resume.
S085's independent verification happened as its `accepts` required, at a
different book *and* a different control. DEC-143's pre-registration is real:
`S148_sprt.sh` states bounds, worst-case games from the nElo formula, the abort
rule and all three readings before the first game.

**Negative results are recorded as negative** -- S148 H0 at 14808 games, S149 H0,
S130 no-verdict at 16784 games recorded as zero and kept with the reason. That
is the rule working.

### Three claims that did not survive verification, recorded so they are not repeated

1. **An arch asymmetry between candidate and reference.** `cmake/arch.cmake:32`
   defaults `CHESSO_ARCH` to `native`, so a reference built by `fastchess.sh`'s
   bare `cmake` call gets the same flags as `build/`. The hypothesis was wrong;
   what survives is F06, the *unchecked* reuse.
2. **That S199's pinned reference predates S104.** S104 landed 2026-08-19 and
   S105 on 2026-08-20, so the pinned commit is after it. What survives is F30,
   a different gap in the same reading rule.
3. **A depth-1 early exit in the reproduction of F08.** That was an artifact of
   `printf ... | chesso`, which is exactly the trap `TOOLCHAIN.md:206-218`
   documents for chesso as well as for Stockfish -- `command_quit` calls
   `stop_and_join_search()`, so a piped `go` is killed before it searches. The
   coordinator walked into it while checking a reviewer's work, on the same day
   the repository's own warning was being cited elsewhere in this audit. F08 was
   re-taken through `chess.engine` and stands. **The warning works only if it is
   read before the pipe is typed, not after the answer looks odd.**

---

## What would close the most, cheapest

Not a plan -- the order is the owner's -- but the audit's own reading of its
findings.

1. **F08**, because it is the only high finding that fires in ordinary play, and
   because it needs the owner: the fix requires re-stating a test the TESTS rule
   forbids an agent to touch.
2. **F09 and F10 together**, one step: a third class in the S161 sanitiser
   (piece counts, back-rank pawns) plus a runtime bound in the generator. Both
   are `position fen` reachable, both are dead-assert-guarded in Release, and
   the BUGS rule arms on them.
3. **F01, F02 and F03 together**, one step: derive the three tables, rewrite
   five mask builders from the geometry, rename `CMK_POS`, and add
   `THIRD_PARTY.md`. Around 200 lines, behaviour-neutral, discharged on node
   counts. It removes the whole code-level originality exposure and it is the
   answer to an accusation that is far more convincing written before the
   accusation than after.
4. **F04's comment**, one line, today: whatever is decided about `twosided`, the
   sentence claiming a protection that does not exist is the one a future reader
   will rely on.
