# Training chesso's own network: the article, the literature, and a proposed plan — 2026-09-27

Preparatory work only. The owner's rulings stand: the mark is 3000 on the
CCRL Blitz scale without a network (DEC-071), S029 stays parked (DEC-054),
and it is un-parked by a decision taken when S152 reads the mark (DEC-179).
This document changes no step and no decision; it is what that decision can
be taken from. Companion: `2026-09-27_article_finetuning_comparison.md`.

---

## 0. Provenance and scope

Read on 2026-09-27: the Medium article named in the companion document
(browser pane; Medium refuses fetch tools), that project's README, and the
published sources listed in section 10 — Stockfish's training documentation
and wiki, its data-generator documentation, two Stockfish commit messages and
one release post, the `bullet` trainer's documentation and licence, the
Chess Programming Wiki, one arXiv paper, one TalkChess thread, and the
release notes and READMEs of Leorik, Viridithas, Triumviratus and Enyo.
**No source file, no table, no network file and no data set of any project
was opened or downloaded.** Where a document carried code, the code was not
reproduced; where it carried a tuned constant, the constant is not used as a
seed (DEC-105, DEC-134). Every technique below is named and described in
prose, which is the form DEC-221 requires an implementer to work from.

Nothing here is a chess judgement. Every Elo figure quoted from another
project is a reported figure and decides what to try, never what to
conclude (DEC-019).

---

## 1. The answer, short

1. **The article's training technique is a 2020-shaped recipe on data
   chesso may not touch.** A plain HalfKA net (49152 inputs a perspective,
   256 wide, 32-32-1) distilled from 25.95 M Stockfish-labelled positions
   out of the Lichess evaluation database, then a planned but unreported
   "reinforcement" stage. Its own findings — severe overfitting from king
   squares the data never visits — are what the record solved years ago with
   king buckets and mirrored features, and no game or Elo is reported.
2. **The leading edge is two things.** At the top, Stockfish: threat-input
   feature sets (SFNNv10 to SFNNv16, 2025 to 2026), hundreds of billions of
   positions rescored by a Leela network, quantization-aware training, up to
   44 Elo for the latest release. In the open-source middle, the
   from-scratch loop:
   self-play at about 5000 nodes a move from randomised openings, a
   `(768 -> N) x 2 -> 1` net trained by `bullet`, regenerate with the new
   net, repeat; wider first layer, then king input buckets with horizontal
   mirroring, then material output buckets. Leorik walked that road from a
   hand-crafted 2917 to 3276 with its first net on 622 M own positions, and
   to 3496 on more than 6 billion.
3. **Under chesso's rules the top route is closed and the middle route is
   open.** No Leela data, no Stockfish-labelled data, no foreign weights,
   ever (DEC-002, DEC-016). Ideas, architectures and objectives are free to
   implement from their descriptions (DEC-014, DEC-221). Leorik and
   Viridithas are the precedents whose data provenance matches chesso's
   rule: own labels only, iterated. (Human-game positions carrying chesso's
   own labels: the owner's reading on 2026-09-28 is that they are fine;
   section 3.6 has the community record and section 9 the pending DEC.)
4. **The proposal** (section 6): an own binary corpus format and a datagen
   mode shared with S082/S083 now; after the mark, inference behind a
   compile flag with the accumulator held per ply, an own PyTorch trainer,
   generation 0 from the 3000-level hand-crafted engine, the owner runs the
   training, every net an SPRT, then one architecture step per verdict
   along the record's lineage.
5. **Two facts about this machine decide the schedule more than any
   technique.** The root file system has **8.5 GB free of 449 GB (99 %
   used)** on 2026-09-27; a billion-position corpus is about 32 GB in a
   compact record. And the network's data, training and verdicts compete
   with the plan's remaining 36 to 46 verdicts and its SPSA cadence for the
   same nights. Storage is a decision the owner takes before any of this
   starts.

---

## 2. The article's training technique, as read

- **Architecture.** "Dual-Perspective HalfKA": 12 piece types x 64 squares x
  64 own-king squares = 49152 binary inputs a perspective; accumulator 256
  wide, clipped ReLU in [0, 1]; then "512 -> 32", "32 -> 32", "32 -> 1"
  (its memory-layout table says 64 x 512 and 32 x 64 for the same layers).
  Weights int16 for the first layer, int8 after; the first layer's 24 MB is
  said to sit in L3 and the rest, 17.28 KB, in L1.
- **Data.** The Lichess evaluation database (394669566 positions in the
  article; 409710113 on the dataset card at the read), positions from human
  games "labeled by Stockfish". Filtered of checks, captures and imminent
  mates (a 35 % reduction), mirrored horizontally to double the set,
  deduplicated keeping the deeper label, filtered by minimum depth per
  phase (26 / 28 / 32 by piece count), then **stratified and resampled** to
  a target matrix over six score bands x three phases — down-sampling most
  cells to 0.08 x and up-sampling `late_micro_advantage` 6.00 x. Result:
  25952256 training positions and 524288 validation positions.
- **Training.** 30 epochs of 3168 steps at 8192 a batch; own script
  (`train_pipeline.py`); loss reported as 35.3 cp within +/- 350 cp and 46.8
  cp overall. Framework, optimiser and learning rate not stated.
- **The second stage, described and not run.** "Reinforcement Learning" with
  the target `0.8 * deep search score (30 ply) + 0.2 * game result`, argued
  as a guard against "deep horizon gambles".
- **Findings the author reports.** (1) Human games resign early, so the
  endgame is a "Dead Zone" that had to be up-sampled 6 x. (2) The king spends
  over 90 % of its time on the first rank, so more than half the king-square
  slices of the first layer are starved and the model "exhibited severe
  overfitting". The closing paragraph names Stockfish's king bucketing as the
  remedy.

---

## 3. The state of the art, by topic

### 3.1 Feature sets and architecture lineage

- **Origin.** Yu Nasu, 2018, for shogi: a sparse first layer indexed by the
  own king's square and each other piece's square and type, updated
  incrementally on make and unmake; Nodchip's port to Stockfish in 2020
  (CPW *NNUE*, *Stockfish NNUE*). English translation of the paper at
  https://github.com/asdfjkl/nnue.
- **Stockfish 12, 2020-09-02.** HalfKP, `256x2-32-32-1`; CPW gives 41024
  inputs a half and 10502144 first-layer weights, the training
  documentation gives 40960 features (the two count the empty slot
  differently); "at least 80 Elo" at "approximately halved search speed".
- **Stockfish 14, 2021-07-02.** HalfKAv2: kings in the feature set, 45056
  inputs a side, a vertical flip, piece-square-table outputs taken straight
  from the feature transformer, and eight small output sub-networks
  (`512x2 -> 16 -> 32 -> 1`) selected by material — the origin of output
  buckets.
- **HalfKAv2_hm** (the training documentation's current default feature set
  name): horizontal mirroring so the king is always on one side of the
  board, and king buckets that group the 64 king squares so each slice sees
  enough data — exactly the fix the article's overfitting finding calls
  for. First layers of 1024 and wider follow (`1024x2 -> 8 -> 32 -> 1` is the
  documentation's own example).
- **Threat inputs, 2025 to 2026.** SFNNv10 introduced "Threat Inputs" (the
  commit message's words; that they encode one piece attacking or defending
  another is a secondary description, **unverified** in a primary source
  this pass); SFNNv13 (commit `a6d055d`, 2026-02-13)
  doubled the second layer from 16 to 32 because the threat inputs had
  shrunk the first layer; SFNNv16 in Stockfish 19 (2026-09-05) removed
  "redundant threat features" and added "new pawn-pair features", and
  retired the small secondary network. A third-party feature count of 82672
  for SFNNv13 appeared in a search snippet and is **unverified**.
- **The community shape.** `bullet`'s documentation: start at "basic 768
  inputs", and "usually more beneficial to just increase the size of the
  first hidden layer (up to at least 1024)" before adding layers; the
  perspective network is two accumulators, one per side, combined by the
  side to move. Leorik's line: 3.0 `(768 -> 256) x 2 -> 1`; 3.1 640 wide with
  SCReLU (squared clipped ReLU); 3.2 `(768 x 5 -> 640) x 2 -> 1 x 8`,
  horizontally mirrored, 5 king input buckets, 8 material output buckets.

### 3.2 The training objective

From the Stockfish training documentation (`docs/nnue.md`) and its wiki:

- The network's output is mapped to win-draw-loss space by a sigmoid of the
  centipawn score over a scaling constant of about 400, and the loss is a
  mean squared error or a cross-entropy in that space; the documentation
  reports good results with a loss exponent of 2.6 rather than 2.
- The target is a blend, `lambda * sigmoid(search score) + (1 - lambda) *
  game result`, applied before or after the loss; the sample training
  command runs at `lambda 1.0`, batch size 16384, "about 400 epochs for the
  nets to mature", "very competitive even after only 100 epochs",
  `random-fen-skipping 3`; training starts from scratch or resumes from a
  saved net.
- Virtual (factorised) features are trained alongside the real ones and
  "coalesced into the real features weights after training is finished".
- Stockfish 19 adds quantization-aware training: the quantised inference
  is modelled during training instead of after it.

`adocs/eval_tuning_strategy.md` section 2.7 already states the blend for the
hand-crafted fit, and S075 measured it there: monotone harm at every
non-zero lambda on a corpus whose scores were a 100000-node search of an
earlier evaluator. **That result does not transfer to a network** — a
different model, a different label depth, a different fixed-point hazard —
and it is re-measured, not assumed, with DEC-064's two rules (K on the
outcome; held-out game-result error selects).

### 3.3 Data generation practice

- **Stockfish's own generator** (`gensfen` documentation): self-play at a
  fixed depth (default 3, with `depth2` as the upper bound and an optional
  node limit), up to 5 random moves between plies 1 and 24 (optionally
  chosen from a multi-PV search within a score window), rows written from
  ply 16 to 400, games stopped past an absolute score of 3000, an
  `ensure_quiet` flag that records quiescence leaves only, output in a
  compact binary pack. Its wiki lists the corpora that trained its nets:
  a depth-9 set of 16 billion positions, fixed-5000-node sets on the UHO
  and DFRC books, and Leela-derived sets; the recipe was "first train with
  datasets generated with Stockfish (depth 9, nodes 5000), and then retrain
  using various Lc0-derived datasets". Stockfish 19 trained on "hundreds of
  billions" of positions "consistently rescored using a strong Leela net".
- **The from-scratch loop, as its practitioners report it** (TalkChess
  t=84811, February 2025): "Set a hard node limit for your search e.g 5k
  nodes"; "Start with 7 or 8 random moves"; adjudicate on a large score held
  for several moves; "Write your own program for datagen that uses direct
  method calls instead of UCI"; "around 4k positions / second per machine";
  "The first 5-10 iterations only took a few hours each and gave massive ELO
  improvements", "The next 10 iterations were much much slower, taking days
  to weeks between networks"; reuse earlier data once it is good (Sapling).
  Another engine "caught up with the HCE level of play after 7-8 iterations
  with net size 768x32" from random weights, with the "first 10-12 moves
  random" and positions "filtered (so that there is no capture best move)"
  (Sovaz1997). Leorik's author: "a tiny network with only 8 neurons in the
  hidden layer" was "already en par with HCE"; "a few million labeled
  positions" suffice for small nets and "One billion seems about enough to
  train pretty good 256 hidden layer nets" (Lithander).
- **Leorik** (MIT): "Network trained on selfplay games using Bullet"; 3.0 on
  "622M labeled positions extracted from selfplay games"; 3.1 on 5.2 B; 3.2
  on "over 6 billion". CCRL Blitz: 2.5 hand-crafted 2917 (DEC-071); 3.0
  3276; 3.1 3430; 3.2 3496 — release deltas, each release carrying more
  than the net.
- **Viridithas** (engine AGPL, nets CC0): "All neural networks currently
  used in the development of Viridithas are trained exclusively on its own
  self-play games, and no network has ever been trained on the output of an
  engine other than Viridithas." Its first net was trained on games of
  three earlier versions "rescored with a development version ... at low
  depth"; from the 13th generation it also uses Lichess Elite positions
  rescored by itself; trainer `marlinflow` then `bullet`. (Its hand-crafted
  ancestry used PeSTO values, which chesso's rule refuses; the data loop is
  the precedent, not the tables.)
- **Quiet-position filtering as a published study.** Tan and Watkinson
  Medina, *Study of the Proper NNUE Dataset* (arXiv 2412.17948, 2024,
  Xiangqi): drop positions in check, drop positions where the static
  evaluation differs from the quiescence score by more than 60 cp or from a
  shallow search by more than 70 cp; composition targets (half the rows
  inside +/- 100 cp, 40 % with material imbalance); a `(1620 x 2 -> 256)`
  net; reported "+100 Elo" over the hand-crafted baseline from 44000
  grandmaster games plus 30000 self-play games. Not chess, external-engine
  games, and the authors say the rule "is not mathematically proven".
- **The article's data preparation against this.** Filtering checks,
  captures and mates and deduplicating are standard; depth gates belong to a
  corpus labelled elsewhere at unknown depth; **score-stratified resampling
  is not in any fetched source**, and it moves the label distribution the
  objective is defined on.

### 3.4 Quantisation and inference

From the Stockfish documentation: the feature transformer's weights and
biases are int16 scaled by 127; later layers' weights int8 scaled by 64 with
int32 biases, so a weight's magnitude is capped at 127/64 = 1.984375; the
clipped ReLU clamps to 0..127; the output is divided by the scale (a shift
when it is a power of two). Of 768 piece-square inputs "there can only be 32
non-zero inputs in any given legal chess position" — input sparsity about
0.1 % — which is why the first layer is an accumulator: subtract the column
of a removed feature, add the column of an added one, and refresh whole when
the king moves (under any king-indexed feature set). Beyond 256 accumulator
lanes "AVX2 doesn't have enough registers" for one pass, so the update runs
in chunks. The target is "million(s) of evaluations per second per thread".
Leorik 3.0 ships "16-bit integers with AVX2 intrinsics". Two practices named
in the community record without a fetched primary source this pass:
accumulator caches keyed by king bucket and colour so a king move restores
from the last state seen in that bucket instead of refreshing from zero,
and lazy updates that defer the accumulator to the first evaluation that
needs it. Both are **unverified as to origin** here and are implemented from
description if wanted.

### 3.5 What "from scratch" means in that community, and the trap in it

Triumviratus (2026) ships networks "trained from scratch" whose stage 1 is
"Stockfish-generated 5000-node positions (~136 GB)" and stage 2 Leela
self-play (T79/T80), on a datacentre GPU, landing 84 then 40 Elo below the
best Stockfish net. Enyo's nets are "trained from scratch" with "Stockfish
used only as a labeling oracle where the dataset requires it" on ODbL
binpacks. Coda trains on "the full LC0 dataset". In that usage *from
scratch* means the **weights** start random; the **data** is another
engine's search or evaluation. Under DEC-002 and DEC-016 that data is closed
to chesso whatever its licence — the Lichess evaluation set the article
uses is CC0 and still excluded, because its labels are Stockfish's search.
The only lineages whose provenance matches chesso's rule are Leorik's and
Viridithas's: own self-play, own labels, iterated.

### 3.6 Human-game data: what the community record says

Read 2026-09-28 at the owner's request, after the owner said human data
from Lichess is fine in their view. Threads and READMEs only; no data
downloaded.

- **The originality argument is about engine-generated data, never about
  human games.** TCEC's 2020 NNUE guideline, as quoted on TalkChess by
  Daniel Shawul (2020-10-10): "All NNUE training data should be generated by
  the unique engine's own search and/or eval code" (the wording recurs
  across threads; the primary TCEC page was not fetched). Alayan, same
  thread: "If 10 engines copy-paste the same NNUE architecture and copy SF
  nets because 'numbers are not the property of anybody', the whole
  competition is a farce." jorose (2024-06-30): Leela data is acceptable,
  but competitive events prefer original data. The 2026 thread on
  Stockfish's sources argues whether training on Lc0 games is fair in a
  match against Lc0 (phhnguyen, 2026-03-07). In none of the fetched threads
  is a position taken from a human game called unoriginal or derivative.
- **Human games are used in two forms, and in both the labels are the
  engine's own.** Viridithas states "no network has ever been trained on
  the output of an engine other than Viridithas" and, from its 13th
  generation, uses "positions from the Lichess Elite dataset rescored by
  Viridithas"; its hand-crafted evaluation was Texel-tuned on the same set.
  dangi12012 (2023-01-14): take "1Billion distinct fens out of real games"
  from Lichess, then "Selfplay these games with the strongest engine
  currently". jstanback (2023-01-12) samples positions from Wasp's games
  "against itself and other engines" and scores them "using my most recent
  Wasp ... with a 35K node fixed depth search". So the community reads
  "own data" as "own labels", not as "own starting positions".
- **Human game results as labels are not the practice for networks.** Every
  fetched recipe labels with the engine's own search score, alone or
  blended with the result of the *engine's* game (lithander's loop,
  2024-02-15; the Stockfish documentation's lambda). The one 2020 remark
  that "today most people are using lichess data, I think it is 'real' game
  outcome" (xr_a_y) sits beside Daniel Shawul's "either the score embedded
  in lichess games (again Stockfish is the mentor) or label them themselves
  with the strongest engine" — the embedded scores are the Lichess
  *evaluations*, which are Stockfish's and closed here.
- **The distribution argument cuts both ways.** For: human games reach
  openings and middlegame structures that random-ply self-play does not,
  which is why they serve as starting positions. Against: the article's own
  finding — resignations leave the endgame a "Dead Zone" it had to
  up-sample 6 x — and the self-play default of the whole record (AndrewGrant
  "My strongest nets are with 40k nodes per move"; mar "fixed nodes, 5-10k"
  from "a couple of random plies", "100M+"; chrisw "d9 recommended, a few
  hundred million"; Leorik "completely trained from scratch using only
  selfplay games"; Berserk 9 "~8B FENs from Berserk 8.5.1 self play games";
  casanchess "millions of self-play and random positions").
- **Licence.** The Lichess open database and its evaluations are CC0:
  "download, modify and redistribute them, without asking for permission".
  The Lichess Elite Database (nikonoel) filters the same games to 2400+
  against 2200+ (2500+ against 2300+ from December 2021), bullet excluded,
  monthly; the Hugging Face mirror counts 26.3 million games.
- **What follows for chesso** (section 9, item 3). Human-game positions are
  admissible under the copying rules in both forms as long as every label is
  chesso's; the amendment is to DEC-002's "self-play only" sentence, not to
  DEC-016. The Lichess evaluations database stays excluded.

---

## 4. chesso's rules applied to each technique

| technique or resource | status under the rules | where it is decided |
|---|---|---|
| Stockfish-labelled Lichess positions (the article's data) | **forbidden**: labels are another engine's search | DEC-002, DEC-016 |
| Leela / Lc0 game data, rescored or not | **forbidden** | DEC-002, DEC-016 |
| any published network's weights, as seed or as reference | **forbidden** | DEC-016, DEC-105 |
| human-game positions (Lichess database, CC0) with chesso's own labels — as starting positions for self-play, or scored by chesso's search | clean of engine output; **DEC-002 says self-play only, and the owner's reading on 2026-09-28 is that human data is fine**; the community record (section 3.6) raises no originality objection; the amending DEC is the owner's to record, draft at the end of section 9 | owner, DEC pending |
| self-play at fixed nodes, random openings, own labels | **allowed and required** | DEC-002, S029 |
| perspective net, king buckets, mirroring, output buckets, SCReLU, threat inputs, pairwise multiplication | **allowed** as ideas implemented from description | DEC-014, DEC-221 |
| eval scale 400, quantisation scales 127 / 64, lambda, loss exponent 2.6, batch 16384 | published hyperparameters of one project's training; **not seeds** — chesso sweeps its own on held-out error, the way S075 did (DEC-134 forms b and c) | DEC-105, DEC-134 |
| `bullet` (MIT) or `nnue-pytorch` (GPL) as the trainer | not copying an engine, but a dependency and an originality question; `nnue-pytorch` is GPL and refused on the licence line; `bullet` needs the DEPS ask | owner, DEPS rule |
| PyTorch, CUDA on the RTX 5070 Ti | a framework, and "a GPU may serve training, never play" (`specs.md`) | DEPS ask for the venv install |
| running the training | **the owner's**, or a one-run delegation as at S028 | DEC-015, DEC-041, DEC-034 |
| Stockfish or a tablebase as an analysis tool on results | allowed (a binary as a tool) — never as a label source | DEC-016 |
| the trainer's own code and the corpus format | written here; no format of another project is read as code | DEC-016, DEC-104 |

---

## 5. What chesso already has toward a network

- **The hook.** `make_move` and `unmake_move` in `src/bitboard.cpp` route
  every piece change through `add_piece`, `remove_piece` and `move_piece`,
  which call `eval_add_piece` and `eval_remove_piece`; today those maintain
  four scalar accumulators (material, two piece-square sums, phase) and
  `eval_refresh` rebuilds them; `eval_accumulators_match` is the INV-4 oracle
  and `test_invariants` runs it over 2.1 M make/unmake calls. A network
  accumulator is a wider value fed by the same alphabet. S029 already says
  it is held per ply in the search state and popped, never reverse-updated.
- **The data pipeline.** `tools/datagen` (fixed nodes, random plies,
  adjudication, filters with printed costs, `fen result score phase`),
  `tools/corpus_dedupe`, the provenance stamp (S077), the archive rule
  (DEC-183), and two open steps that reshape the corpus anyway: S082 (record
  the quiet leaf; density) and S083 (size against node budget under a nights
  budget).
- **The measurement instrument.** `fastchess.sh` at 8+0.08 with stated
  bounds and a pre-registration rule (DEC-143), the 32+0.32 block-boundary
  estimate (DEC-202), `rating.sh` on the CCRL scale, the second tier for
  anything touching `make_move` or the search (DEC-141), scripted goldens
  (DEC-142), the commit `Bench:` line.
- **The parking lot.** S029's shape — `(768 -> N) x 2 -> 1`, N of 256 to
  512, integer SIMD inference, a separate trainer — and S133, whose
  king-bucketed tables rehearse the bucket-crossing rebuild and the
  accumulator discipline a network needs (its file says so).
- **The hardware.** i7-8700K, 6 cores / 12 threads, AVX2 without AVX-512 or
  VNNI (the shipping `bmi2` target is x86-64-v3); 15 GB RAM; RTX 5070 Ti
  with 16 GB, driver 580.173.02; PyTorch **not installed** in
  `~/.venv/chess` (checked 2026-09-27); root file system 8.5 GB free.

---

## 6. The proposed plan

### 6.1 Principles, all inherited

Own data only; own trainer; every net an SPRT; one change per verdict; a
verdict of zero recorded as zero; every constant fitted or swept here;
every artefact carries its lineage (commit, data digests, trainer commit,
hyperparameters, parent net); the agent builds and runs everything except
the training run itself.

### 6.2 Now, before the mark: machine-free and inside DEC-054

Nothing below is network code, so nothing below reopens S029.

- **P0. Decisions** (section 9): storage; the trainer dependency; the data
  policy; the training-run delegation. Recorded as DECs when the owner
  answers; nothing starts without the storage one.
- **P1. One corpus record for both fits.** An own compact binary row —
  occupancy bitboard, packed piece nibbles, side to move, castling and
  en-passant, the search score, the result, the ply and a flags byte; about
  32 bytes — written by `tools/datagen` and read by `tools/tuner`, with the
  TSV kept as a debug print. It is designed here from the board's own
  representation; the record's existing packs are not read. Fold into
  S082's brief (it rewrites the row anyway) or a small tools-only step; a
  converter for the archived S065 corpus proves the reader.
- **P2. Datagen options the network needs, measured by S083.** Record every
  position from a minimum ply rather than a sample (the density question
  S082 already owns); an optional bounded random-move injection inside the
  game, the row after a random move not recorded; the coverage matrix
  printed. All three are S083's curve first and a verdict second, in the
  companion document's section 6.
- **P3. The throughput number.** S083 states the 5000-node rate in rows an
  hour on this machine. Every budget in section 7 is provisional until it
  exists.

### 6.3 After the mark: the steps, in order, ids allocated at un-parking

- **N1. Inference behind a compile flag.** `CHESSO_NNUE` off by default
  until a net ships; the net embedded in the binary at build time (no new
  UCI option, so `test_uci_surface` is untouched); a perspective net
  `(768 -> N) x 2 -> 1` with clipped ReLU, int16 first layer, int16 or int8
  output layer, an AVX2 path and a portable scalar path that must agree
  bit for bit; the accumulator pair held per ply in `search_state_t` and
  popped; king moves refresh (no buckets yet). Accepts: with the flag off,
  node-identical to the parent (INV-6); with a fixed random net,
  `accumulator == refresh` over the INV-4 corpus and under the Debug
  assertion; quantised inference equals the float reference within a
  stated tolerance on a pinned position set; the second tier green
  (DEC-141). No verdict owed: nothing plays until N5.
- **N2. The trainer.** Own program in Python on PyTorch: a memory-mapped
  loader over P1's format, shuffled by chunk; sparse first layer per
  perspective with shared weights; options for width, activation (clipped
  or squared clipped), lambda, loss exponent, learning-rate schedule, batch
  size, epochs, weight clipping to the quantisable range; a held-out split
  **by game** (S066's lesson; the record's random-skip is not a split);
  checkpoints; export to chesso's own net file with a provenance header
  (data digests, generating engine commit, trainer commit, hyperparameters,
  parent net hash); a float-versus-quantised check the engine test in N1
  reads. Disclosure as at S028: one smoke run on a small sample to show the
  loss descends, stated in the step file, nothing from it shipped. The
  exact run — data, hyperparameters, expected wall time — stated for the
  owner.
- **N3. Generation 0 data.** The hand-crafted engine at the mark generates
  at the S083 node budget from random openings, every row after the minimum
  ply, filtered as S082 lands them; budget in nights stated first; digests
  and recipe archived (DEC-183). Once the DEC of section 9, item 3 is
  recorded, a tagged share of the starting positions may come from human
  games, and a tagged set of human-game positions may be scored by chesso's
  own search; no row ever carries a foreign score. Target for net 1: the
  record says a few
  million rows train a small net and about a billion a good 256-wide one;
  the first target is what one to three nights produce at the measured
  rate, and the held-out curve says whether more helps.
- **N4. Net 1, trained by the owner** from N2's stated run; width 256; lambda
  and exponent chosen by a held-out sweep with one candidate to the match.
- **N5. The first verdict.** Net 1 against the hand-crafted incumbent at
  8+0.08, `{0, 5}` nElo, pre-registered with its worst-case games and abort
  rule (DEC-143), the nps cost recorded beside it. Three outcomes and all
  three are recorded: H1 ships the net; H0 or no verdict keeps the
  hand-crafted evaluation and goes to N6 with the net as the data
  generator only if it is at least not worse by the estimate. The record's
  one from-random report needed 7 to 8 iterations to reach hand-crafted
  parity; Leorik's first net from hand-crafted data landed +359 on the list.
  Neither transfers; the match decides.
- **N6. The loop.** Generate with the strongest engine so far, retrain from
  scratch on the union of the good generations (reuse once data is good, as
  the record advises), one SPRT per net, a generation ledger file with data
  digests, net hash, parent engine commit and verdict. Stop condition: two
  consecutive zeros at the same architecture, which is when N7 starts.
- **N7. Architecture steps, one verdict each, in the lineage's order:**
  wider first layer (512, then 1024); squared clipped ReLU; king input
  buckets with horizontal mirroring by king file (the article's own
  finding, the record's fix); material output buckets; then the frontier —
  threat inputs (piece attacks or defends piece) and pairwise
  multiplication of accumulator halves — implemented from description.
  Each step ships only on H1, keeps the previous net on anything else, and
  states its nps cost.
- **N8. Speed.** An accumulator cache keyed by king bucket and colour, and
  lazy updates, once buckets exist; behaviour-neutral by node count and
  best moves (INV-6), accepted on an interleaved timing (DEC-083).
- **N9. Search retune.** The evaluation's scale and noise change under a
  net, so the pruning margins and reductions do too: one SPSA lane over the
  margins after the first shipped net (DEC-222's cadence), verified by an
  independent SPRT; the hand-crafted `evaluate()` stays compiled as the
  fallback and as S133's platform.
- **N10. Rating checkpoint.** `rating.sh` after the first shipped net and
  after N7's last H1, so the factor between self-play and the list is
  re-read with a network on the board.
- **N11. Provenance statement.** The MANUAL.md originality and provenance
  statement DEC-105 proposed and nobody wrote, extended with the network's
  lineage: what generated the data, what trained it, what none of it ever
  touched; the ledger of N6 published beside the net.

### 6.4 What the trainer does not do

It does not read any other project's data format, weights or code. It does
not take a tuned constant from anyone: scales, lambda, the exponent, the
schedule are swept on chesso's held-out error and recorded. It does not
decide anything: held-out error ranks, the SPRT decides (DEC-019, DEC-064).

---

## 7. Budget, from measured numbers

- **Generation rate.** S065 measured 120000 games at 100000 nodes a move on
  12 threads in about 8 hours, 11.0 M rows: about 15000 games and 1.4 M rows
  an hour at about 92 rows a game. At 5000 nodes a move the search is a
  twentieth of that; if the rest of a game is cheap, 10 to 20 times more
  games an hour, so **roughly 15 to 25 M rows an hour, 100 to 200 M a
  night** — the same order as the record's "4k positions / second per
  machine" (14 M an hour). **Unmeasured until S083.** A billion rows is then
  five to ten nights; the first net needs far fewer.
- **Storage.** 32 bytes a row: 100 M rows 3.2 GB, 1 B rows 32 GB; the current
  TSV is 64.15 bytes a row (S065). `.tuning/` holds 4.9 GB today and the
  root file system 8.5 GB free. **Nothing in N3 starts on this disk as it
  stands.**
- **Training.** A `(768 -> 256) x 2 -> 1` net costs about 17000
  multiply-adds a position; a billion positions an epoch is seconds of GPU
  arithmetic and tens of gigabytes of I/O, so the loader and the disk set
  the epoch time — minutes each with 15 GB of RAM streaming from disk, a
  night for a net at 40 epochs over a billion rows, hours for the first
  nets on 100 M. A Blackwell part needs a CUDA 12.8-class PyTorch build;
  verified at install.
- **Verdicts.** One per net and one per architecture step at 8+0.08, 2110
  games an hour: the ledger's resolved runs span one to fifteen hours, and
  `{0, 5}` worst cases are stated at pre-registration (DEC-143); a 32+0.32
  confirmation is about 3 h 45 m per 2000 games (DEC-202); a rating run
  about 5 hours.
- **Rough total for generations 0 to 3 and the first three architecture
  steps**: 10 to 20 datagen nights, 5 to 10 training nights on the GPU,
  10 to 20 verdict nights — a season of the machine, on top of the plan's
  own remaining 36 to 46 verdicts, its SPSA cadence and S082/S083's nights.
  The record agrees on the shape: the first iterations are hours and cheap,
  the later ones "days to weeks between networks".

---

## 8. Risks, named

- **Disk** (section 7). Decided before anything else.
- **The machine is one machine.** Datagen, training data I/O and a match
  cannot share it without lowering `CONCURRENCY` (MACHINE rule); the GPU
  training can run beside a match only if its data loader does not starve
  the cores, which is measured, not assumed.
- **The run is the owner's** (DEC-015). Each generation waits on a run
  nobody else may start; the S028 precedent (DEC-034) is a one-run
  delegation if the owner prefers.
- **King-square starvation** (the article's finding) hits any king-indexed
  feature set on small data; chesso starts at 768 inputs, as `bullet`'s
  documentation advises, and adds buckets in N7 only when a curve says so.
- **The blend fixed point.** A net trained on scores its own generator
  produced at the same depth can learn its own search (strategy section 1);
  lambda is swept per generation and the label depth stated beside it.
- **Speed.** Stockfish 12 halved its speed for its net; chesso's evaluation
  is cheap by construction (INV-4) and 5.95 Mnps single-thread at
  2026-08-18. The verdict includes the slowdown; a net that predicts better
  and plays worse is recorded as such.
- **Pruning that hides a mate is the recurring bug** (`CLAUDE.md`); a new
  evaluation changes every margin's meaning, so the mate suite and the
  second tier run before N5, not after.
- **Published figures do not transfer** (DEC-019): +359 for Leorik's first
  net and "+100 Elo" in the Xiangqi study are directions, not forecasts.

---

## 9. Decisions this asks of the owner

Numbered so they can be answered one by one and recorded as DECs.

1. **Storage.** Where a corpus of tens of gigabytes lives: an external or
   second disk, space reclaimed on `/`, or the Synckeeper archive path with
   the working set local. Blocking for N3 and for S083's larger runs.
2. **The trainer.** Own PyTorch program, `bullet` (MIT), or another route.
   **Deferred by the owner on 2026-09-28**: the choice is not clear to them
   and is discussed when the plan itself is prepared; the report's
   recommendation (own PyTorch: the originality claim, and shapes small
   enough that a sparse first layer on this GPU is not the bottleneck)
   stands as an input only, and no decision is recorded. Whatever is
   chosen, its toolchain goes into the venv, which is the DEPS ask.
3. **Data policy.** The labels stay chesso's own (DEC-016). **The owner's
   reading on 2026-09-28: human-game positions are fine.** The community
   record was checked the same day (section 3.6): the originality debate is
   about engine-generated data, never about human games, and the accepted
   uses are positions rescored by the engine's own search or starting
   positions for its own self-play; the Lichess *evaluations* database stays
   excluded because its labels are Stockfish's. The DEC that amends
   DEC-002's "self-play only" is the owner's to record, through the
   coordinator; a draft follows the list.
4. **The training run.** The owner runs it (DEC-015), or delegates each run
   as at S028 (DEC-034). Either is workable; the plan above assumes the
   first.
5. **Timing.** Un-parking stays at S152's reading (DEC-179). P1 and P2 fold
   into S082/S083 now if the owner agrees they are corpus work and not
   network work.

### Draft for item 3, to be recorded by the coordinator when the owner confirms

Not recorded anywhere yet; the id is allocated at recording time.

```
## DEC-nnn  2026-09-28  Human-game positions may enter a corpus with chesso's own labels; DEC-002's "self-play only" narrows to the labels
Tags:         nnue, data, corpus, provenance, dec-002, dec-016
Amends:       DEC-002, whose "NNUE training data will come from self-play only"
              was written before the question of human-game positions came up
Context:      The 2026-09-27 preparation report asked whether positions from
              human games (the Lichess open database, CC0) could join a corpus
              with labels produced by chesso's own search. The community record
              read on 2026-09-28 (report section 3.6): the originality debate
              concerns engine-generated data -- TCEC's 2020 guideline that all
              training data be "generated by the unique engine's own search
              and/or eval code", the 2026 fairness argument over Stockfish
              training on Lc0 games -- and human games appear only as positions
              rescored by the engine (Viridithas, whose "no other engine's
              output, ever" statement coexists with Lichess Elite positions
              rescored by itself) or as starting positions for self-play. No
              fetched source objects to human-game positions.
Decision:     By the owner, 2026-09-28. Positions from human games under a
              licence that permits it may enter a corpus in two forms: as
              starting positions for chesso's own self-play, or as positions
              scored by chesso's own search. The label is chesso's in both
              forms; a human game's result is not a label. Any position that
              carries another engine's score -- the Lichess evaluations
              database among them -- stays excluded (DEC-016). Every corpus
              states the share of rows from each source, and the network's
              provenance statement names it.
Rejected:     Human game results as labels: they measure the humans' play,
              and early resignations leave the endgame empty. The Lichess
              evaluations database: Stockfish's labels. Keeping "self-play
              only": it excluded a source with clean provenance for no
              originality gain.
Consequences: DEC-002 carries an Amended line. S029's excludes line and the
              report's rules table read "another engine's evaluation or
              search", not "self-play only". `tools/datagen` gains a mode that
              starts games from a file of positions, and the corpus record
              carries a source tag (report section 6, P1 and P2).
```

---

## 10. References

Fetched 2026-09-27 unless stated. Documentation, release notes, commit
messages, forum posts and papers only; no source file, table or data.

- The article: https://medium.com/@alan0408yuan/from-texel-tuning-to-nnue-the-steep-learning-curve-of-building-a-3200-elo-chess-ai-d842393b5955 (browser pane); its README https://raw.githubusercontent.com/alanyuan08/Chess-App/main/README.md; its dataset https://huggingface.co/datasets/Lichess/chess-position-evaluations (409710113 positions, Stockfish-evaluated, CC0, updated 2026-09-27).
- Nasu 2018, English translation: https://github.com/asdfjkl/nnue (located by search; not fetched).
- CPW *NNUE*: https://www.chessprogramming.org/NNUE; CPW *Stockfish NNUE*: https://www.chessprogramming.org/Stockfish_NNUE — SF12 and SF14 figures, "at least 80 Elo", "approximately halved search speed".
- Stockfish `nnue-pytorch` `docs/nnue.md` (raw): https://raw.githubusercontent.com/official-stockfish/nnue-pytorch/master/docs/nnue.md — feature sets, quantisation scales, sparsity, refresh on king move, loss forms, lambda, exponent 2.6, factorised features.
- `nnue-pytorch` wiki, *Training datasets*: https://github.com/official-stockfish/nnue-pytorch/wiki/Training-datasets; *Basic training procedure*: https://github.com/official-stockfish/nnue-pytorch/wiki/Basic-training-procedure-(train.py); its licence https://raw.githubusercontent.com/official-stockfish/nnue-pytorch/master/LICENSE (GNU General Public License version 3).
- nodchip `docs/gensfen.md` (raw): https://raw.githubusercontent.com/nodchip/Stockfish/master/docs/gensfen.md.
- Stockfish commit `a6d055d` (SFNNv13), message via the GitHub API: https://api.github.com/repos/official-stockfish/Stockfish/commits/a6d055d7e27ab3e29a42e8b94215102824760057 — sscg13, 2026-02-13.
- Stockfish 19 release post: https://stockfishchess.org/blog/2026/stockfish-19/ — 2026-09-05, SFNNv16, QAT, "hundreds of billions", "up to 44 points".
- `bullet`: https://raw.githubusercontent.com/jw1912/bullet/main/README.md, `docs/0-contents.md`, `docs/1-basics.md`, `docs/3-data.md`, `LICENSE` (MIT, Jamie Whiting 2023).
- TalkChess *Generating original training data*: https://talkchess.com/viewtopic.php?t=84811 — Sapling 2025-02-07, Sovaz1997 2025-02-08, Lithander 2025-02-10.
- Leorik: https://raw.githubusercontent.com/lithander/Leorik/master/README.md; release 3.0 https://github.com/lithander/Leorik/releases/tag/3.0 (2023-02-05).
- Viridithas README (raw): https://raw.githubusercontent.com/cosmobobak/viridithas/master/README.md.
- Triumviratus releases: https://github.com/Tors3/Triumviratus/releases; Enyo: https://raw.githubusercontent.com/ilAYAli/nnue/master/README.md; Coda: https://github.com/adamtwiss/coda (search result only, not fetched).
- Tan, D., Watkinson Medina, N. (2024), *Study of the Proper NNUE Dataset*: https://arxiv.org/abs/2412.17948, https://arxiv.org/html/2412.17948v1.
- Section 3.6, fetched 2026-09-28: TalkChess *Games for Training NNUE?* https://talkchess.com/viewtopic.php?t=81330 and its page 2 https://talkchess.com/forum3/viewtopic.php?f=7&t=81330&start=10 (Steve Maughan, chrisw, AndrewGrant, jstanback, dangi12012, 2023-01); *Best practices for NNUE training data generation?* https://talkchess.com/viewtopic.php?t=83944 (mar, smatovic, jorose, 2024-06); *How do NNUEs self train?* https://talkchess.com/viewtopic.php?t=83343 (lithander, eboatwright, jdart, 2024-02); *How to get started with NNUE* https://talkchess.com/viewtopic.php?t=83170; *Dangerous turn*, page 2, https://talkchess.com/forum3/viewtopic.php?t=75358&start=10 (Daniel Shawul, xr_a_y, Alayan, 2020-10-10; the TCEC guideline as quoted there, primary page not fetched); *TCEC has no rules regarding engine uniqueness or data training* https://talkchess.com/forum3/viewtopic.php?t=75369; *Stockfish: what are recent game sources for NNUE?* https://talkchess.com/viewtopic.php?t=85975 (2026-02 to 2026-03); *Devlog of Leorik* pages 43 and 48, https://talkchess.com/viewtopic.php?t=79049&start=420 and https://www.talkchess.com/forum/viewtopic.php?p=977847; Lichess open database https://database.lichess.org/ (CC0); Lichess Elite Database https://database.nikonoel.fr/ and its Hugging Face mirror https://huggingface.co/datasets/nuriyev/lichess-elite; Berserk releases https://github.com/jhonnold/berserk/releases (9: "~8B FENs from Berserk 8.5.1 self play games"); Minic README https://raw.githubusercontent.com/tryingsomestuff/Minic/master/README.md; casanchess README https://raw.githubusercontent.com/casanche/casanchess/master/README.md.
- Local: `adocs/plan_todo/S029_nnue.md`, `S082_*`, `S083_*`, `S133_*`; `adocs/eval_tuning_strategy.md` sections 1, 2.7, 3; `adocs/audit/2026-09-12_plan_adversarial_review.md` "Literature and current-engine context"; `adocs/decisions.md` DEC-002, DEC-014, DEC-015, DEC-016, DEC-019, DEC-034, DEC-041, DEC-054, DEC-064, DEC-071, DEC-083, DEC-104, DEC-105, DEC-134, DEC-141, DEC-143, DEC-179, DEC-183, DEC-202, DEC-221, DEC-222; `.moltke.local.md`; `src/bitboard.cpp` (`add_piece`, `remove_piece`, `move_piece`), `src/eval_tables.hpp` (`eval_add_piece`, `eval_remove_piece`, `eval_refresh`), `tools/datagen.cpp`.
