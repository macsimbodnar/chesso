# Literature check of `adocs/plan.md` — 2026-09-04

Method. Every figure below was fetched during this task on 2026-09-04: CCRL via
`curl` with a browser user-agent (WebFetch got HTTP 403 on computerchess.org.uk),
GitHub PR/commit pages via WebFetch, commit messages and directory listings via
the GitHub REST API, raw source files via raw.githubusercontent.com, CPW pages
via WebFetch. WebFetch returns a model's paraphrase of the page; numbers were
cross-checked against a second route (API text, `curl`+`grep`) wherever one
existed. No code and no table was copied from any engine — technique names and
numbers only. Where the plan's own step files trace a figure to a URL, that URL
was fetched; where they say the URL was untraced, that is stated. "NOT FOUND"
lists what was searched.

Nothing below is a chess judgement; it is a comparison of numbers and names.

---

## A. The plan's cited figures

| # | Claim (plan wording) | Where stated | Source fetched | What the source says | Verdict |
|---|---|---|---|---|---|
| A1 | Leorik 2.5 at 2917 CCRL Blitz | `plan.md:44` | https://computerchess.org.uk/ccrl/404/rating_list_all.html (list dated August 28, 2026; 2'089'684 games, 2'933 programs) | Row "Leorik 2.5 64-bit 2917 +16 −16, 1116 games". (Leorik's README states 2921 for 2.5 — an older snapshot.) | CONFIRMED |
| A2 | Leorik 2.5 has "less evaluation than chesso ships" | `plan.md:44` | https://github.com/lithander/Leorik/releases/tag/2.5 ; raw `Leorik.Core/Evaluation/Evaluation.cs` and `Features.cs` at tag 2.5 ; chesso `specs.md:388` | Leorik 2.5: piece-square values as linear functions of both kings' positions and phase (18 parameters), mobility (added 2.2), pawn structure (isolated, passed, protected, connected, backward), draw recognition; **no king-safety term**. Chesso: material, tapered PSQT, mobility (linear), king safety (linear), passed pawns, pawn structure ("three pawn terms"). | DIFFERS (in part). Fewer term families at Leorik (no king safety) supports the sentence; but Leorik's PSQT is king-relative and it has five pawn features to chesso's three. The source does not settle "less" either way. |
| A3 | Leorik's largest jump, +436, was four search features and no evaluation change | `plan.md:45` | CCRL list (A1): Leorik 1.0 64-bit **2102**, Leorik 2.0.2 64-bit **2538** ; https://github.com/lithander/Leorik/releases/tag/2.0 | 2538 − 2102 = 436. Release 2.0 lists four additions — null move pruning, futility pruning, history sorting for quiet moves, late quiet moves at reduced depth — states the evaluation "is still minimal, using the same PSTs as version 1.0 and nothing else", claims "at least 400 Elo stronger", and also credits "a significant increase of nodes searched per second". | CONFIRMED (note the nps increase the release also credits) |
| A4 | Ethereal's removal ledger: history −759, LMR −249, quiet-pruning family −175, reverse futility −32 | `plan.md:46-48`; DEC-087; `S098:31-32` says the primary URL was "untraced publicly" | https://api.github.com/repos/AndyGrant/Ethereal/commits/e755a8140fba — "Add elo estimates to search steps (#113)", 2020-01-22 (= Ethereal 11.82 per the V12.00 release notes) | All tests at 12.0+0.12s, Threads=1, Hash=8MB, "elo loss from the removal of the search step": History **−759.05 ±57.40** (2000 games, W1 L1951 D48); Late Move Reduction **−248.59 ±10.48**; Quiet Move Pruning **−175.08 ±7.24**; Beta Pruning **−31.95 ±3.21**; also Null Move −93.08, Late Move Pruning −76.88, Extensions −59.87, SEE Pruning −41.54, ProbCut −9.08, Counter-move Pruning −8.10, Futility −3.13, Futility-no-history −2.42, Follow-up-move Pruning −1.60. | CONFIRMED — and the URL the plan's files call untraced is this commit |
| A5 | "…and single digits for most evaluation terms" | `plan.md:48` | same commit as A4 | The ledger prices **search steps only**; no evaluation term appears in it. The single-digit entries are search items (ProbCut −9.08, counter-move pruning −8.10, futility −3.13/−2.42, follow-up −1.60). | DIFFERS — no evaluation term is priced in that ledger; the clause has no source there. Searched: the commit, Ethereal V12.00 release notes, two web searches for an Ethereal evaluation-removal table — none found. |
| A6 | Ethereal removed check extensions for +4.1/+4.5 | `plan.md:123`; DEC-087(a); `specs.md:541` | GitHub commit search → commit 3f4ef5376d3d, 2018-06-25, "Simplify and remove the **pre-moveloop** check extensions" ; raw `src/search.c` (master) ; https://www.chessprogramming.org/Ethereal ; V12.00 release notes | Commit: **+4.14 ±4.15** at 10.0+0.1s Hash 8 (11825 games) and **+4.54 ±4.13** at 30.0+0.3s Hash 32 (10800 games), bounds [−3, 1]. But Ethereal's current `search.c` still contains check extensions (in the move loop), CPW lists check extensions for Ethereal, and the release notes read "10.19: Remove some check extensions that occurred before the move loop". | DIFFERS — numbers right, scope wrong: only the pre-move-loop check extensions were removed; Ethereal kept check extensions in the move loop. |
| A7 | Stormphrax removed check extensions | `plan.md:123`; DEC-087(a) "as a simplification" | https://api.github.com/search/commits?q=repo:Ciekce/Stormphrax+check+extension | Commit 2024-03-04 "remove check extensions (#67)", message body "Bench: 3663587" — no Elo figure. | CONFIRMED (removed; no published Elo in the message) |
| A8 | Weiss 1.2 at 3055 CCRL Blitz | `plan.md:150`; DEC-087 | CCRL list (A1) | "Weiss 1.2 64-bit 3055 +15 −15, 1195 games" | CONFIRMED |
| A9 | …with a 301-line evaluation | `plan.md:151` | https://raw.githubusercontent.com/TerjeKir/weiss/v1.2/src/evaluate.c (`wc -l`) | 301 lines (evaluate.c alone; `psqt.c` is a separate file). | CONFIRMED |
| A10 | …no capture, continuation or correction history | `plan.md:151` | v1.2 `search.c`, `movepicker.c` (raw); https://github.com/TerjeKir/weiss/pull/477 ; https://github.com/TerjeKir/weiss/pull/428 ; CPW Static_Evaluation_Correction_History | v1.2 orders by TT move, killers, MVV-LVA with SEE split, and one butterfly `history[side][from][to]`; no capture/continuation/correction identifiers. Counter-move history arrived in PR #477 (2021-07-03) and capture history in PR #428 (2021-06-04), both after 1.2 (2020-10-17); correction history first appeared in Caissa, October 2023. | CONFIRMED |
| A11 | …and the full pruning stack | `plan.md:152` | v1.2 `search.c` (raw) | Contains razoring, reverse futility, null move, ProbCut, internal iterative reduction (Rebel form), late move pruning, singular extension, LMR from a log-log table, quiescence futility, SEE-classified bad captures. | CONFIRMED (and it had ProbCut and singular extensions at 3055, which the plan schedules later) |
| A12 | Quiescence stand-pat from the TT score, +10.8/+12.1 at Weiss | `plan.md:134`; DEC-087(c) | https://github.com/TerjeKir/weiss/pull/336 "Use score from TT instead of static eval if possible", 2020-09-08 | **+10.78 ±6.91** (10+0.1, Hash 32, 4966 games), **+12.09 ±6.83** (60+0.6, Hash 128, 4052 games), +21.44 ±9.52 (20+0.2, 8 threads). PR text scopes the change to pruning heuristics. | CONFIRMED (S130 has since measured it at ~0 here, recorded in `plan_done/S130`) |
| A13 | Capture history STC-negative at Weiss | `plan.md:128`; DEC-087(b) | https://github.com/TerjeKir/weiss/pull/428 "Capture History", 2021-06-04, merged | STC **−4.17 ±4.83** (10+0.1, 9088 games), LTC **+3.66 ±3.29** (60+0.6, 15936 games); author: "hurts STC, but LTC shows decent gain". | CONFIRMED |
| A14 | Stash crossed 3000 at v27 | `plan.md:153` | CCRL list (A1) | Stash 25.0 = 2932; **Stash 27.0 = 3049 ±17** (no v26 listed); 28.0 = 3083. | CONFIRMED |
| A15 | …and reached 3424 with no network | `plan.md:153` | CCRL list (A1); https://raw.githubusercontent.com/mhouppin/stash-bot/master/CHANGELOG.md ; raw `src/sources/evaluate.c` (master, VERSION "v37.26"); https://www.chessprogramming.org/Stash | "Stash 37.0 64-bit **3424** +11 −11, 2199 games". CHANGELOG v26–v35: hand-crafted terms tuned with AdaGrad/Adam, no NNUE anywhere; master `evaluate.c` is hand-crafted (material, PSQT, mobility area, passers, king safety, threats, outposts, scaling, pawn hash). CHANGELOG stops at v35 (2023-10-16); the repo has no releases page. | CONFIRMED (the v37 "no network" rests on master's `evaluate.c`, the changelog ending at v35) |
| A16 | Lynx measured late move pruning and futility at +4.7 each when added alone | `plan.md:87`; DEC-087 | https://github.com/lynx-chess/Lynx/pull/512 ; https://github.com/lynx-chess/Lynx/pull/733 | #512 "Add basic LMP", merged 2023-11-24: **+4.7 ±3.9** at 8+0.08, 18875 games (other variants −23.8, −0.3, +5.8, −3.1). #733 "Futility pruning", merged 2024-05-14: **+4.72 ±3.97** STC (18034 games) and +2.07 ±2.85 LTC (40+0.4, 32512 games). | CONFIRMED |
| A17 | Stockfish's removal test: move-count pruning ~0 alone, the block ~204 | `plan.md:86`; DEC-082 | https://github.com/official-stockfish/Stockfish/pull/2401 and its `.diff` (vondele, opened 2019-11-09, merged 2020-01-10; 10+0.1, 20000 games per test, ±3 Elo) | Comment updates: "Step 14. Pruning at shallow depth ~170 → **~204** Elo"; "Move count based pruning **~0** Elo" (added); countermoves-based pruning ~20→~22; futility (parent) ~2→~7; negative-SEE pruning ~10→~17; SEE pruning check ~20→~26; Step 8 futility (child) ~30→~49; razoring ~2→~0; LMR ~208 (added). | CONFIRMED |
| A18 | Berserk 4.3 king-relative PSQT ~+65 | `plan.md:137`; DEC-087(d) | https://github.com/jhonnold/berserk/releases/tag/4.3.0 (2021-07-03) | "about 65 Elo stronger than Berserk 4.2.0" is the author's estimate for the **whole release** (self-play +157.76 ±3.86 at 8+0.08 Hash 16 and +130.11 ±4.79 at 32+0.32, halved). The release bundles "PSQTs indexed based on same side as enemy king", space, piece imbalance tables, expanded king area, phased move generation, history pruning, null threat in LMR, ordering, TT bucket size 4, and bug fixes. | DIFFERS — +65 is the release, not the tables (S133's own text says "in a bundle"; `plan.md:137` attributes it to the tables) |
| A19 | Leorik 2.5 ~+88 for the same | `plan.md:137`; DEC-087(d) | CCRL list (A1): Leorik 2.4 = **2829**, 2.5 = **2917**; https://github.com/lithander/Leorik/releases/tag/2.5 | 2917 − 2829 = 88. Release 2.5 claims ~100 Elo and bundles MIT release, .NET 8, PEXT move generation, "advanced piece-square values" (18-parameter linear functions of both kings' squares and phase, AVX2), and threads. | DIFFERS — +88 is the 2.4→2.5 CCRL delta for a four-change release, not the PSQT alone; and Leorik's form is linear in king squares, not a bucketed table |
| A20 | Ethereal node-fraction time management +9.9/+9.7 | `plan.md:136`; DEC-087(c) | https://github.com/AndyGrant/Ethereal/commit/60f4d5c5 "Rewrite the Time Managment using old ideas and a new one" | Adds a TM factor from the best move's share of nodes, credits Koivisto: **+9.85 ±6.19** (10+0.1, Hash 8), **+9.66 ±6.02** (60+0.6, Hash 64), +20.90 ±9.67 (40/10), +8.33 ±5.52 (40/40). | CONFIRMED (the numbers price a TM rewrite whose new element is this factor) |
| A21 | Pawn correction history +11.4 at ~2850 | `plan.md:131`; DEC-087(b) | https://github.com/lynx-chess/Lynx/pull/1662 (merged 2025-04-15); CCRL list (A1); Lynx README | PR: **+11.35 ±5.16** at 8+0.08 Hash 32, 7502 games, bounds [0, 3]. Band: CCRL Blitz has Lynx 1.9.0 (README release date 2025-03-11) at **3226** and 1.10.0 (2025-06-29) at **3293**; Lynx README gives 1.9.0 = 3159 on CCRL 40/15. | DIFFERS — the number is right; "~2850" is not: Lynx was ~3160–3290 on CCRL when the PR merged. No source for 2850 was found (checked CCRL Blitz rows for every Lynx version and the README's 40/15 table). |
| A22 | Correction-history extensions +3 to +8, only above ~3100 | `plan.md:130`; DEC-087(b); `S110:14-16` | `S110:22-27` (lists pawn +11.29/+12.40 & +4.87/+11.70, non-pawn +6.98/+12.28 & +2.80/+6.84, continuation +2.58/+5.46 & +2.75/+5.46 with **no URL**); https://github.com/mcthouacbb/Sirius/pull/176 ; https://github.com/zzzzz151/Starzix/pull/98 ; CPW Static_Evaluation_Correction_History | Sirius #176 (2024-09-07) minor-piece+king corrhist **+7.43 ±4.78** at 8+0.08. Starzix #98 (2024-08-05) "Pieces correction history": no Elo in the PR. CPW: no Elo figures; says gains grow with time control. | NOT FOUND for the band claim — one fetched point (+7.43) lies inside +3..+8, but the S110 figures have no cited source and nothing fetched supports "only above ~3100". |
| A23 | Continuation history one-ply table +44.7/+34.0 | `plan.md:295`; S024 | https://github.com/TerjeKir/weiss/pull/477 "Counter move history", 2021-07-03 | **+44.68 ±14.02** (8+0.08, Hash 32, 1040 games), **+33.95 ±10.83** (40+0.4, Hash 128, 1304 games). | CONFIRMED (one-ply = counter-move history) |
| A24 | History malus and gravity +37.5/+28 | `plan.md:290`; S093 | https://github.com/TerjeKir/weiss/pull/296 "Quiet history malus", 2020-05-30 ; https://github.com/lynx-chess/Lynx/pull/610 (2024-01-14) | Weiss **+37.49 ±12.19** at 60+0.6 Hash 128, 1228 games (LTC only). Lynx **+28.0 ±10.3**, 2744 games; later quiets-only refinement +5.4 ±4.4. | CONFIRMED |
| A25 | History persistence across `go` +12.5 | `plan.md:290-291`; S093 | https://github.com/lynx-chess/Lynx/pull/637 (2024-02-03) | **+12.5 ±6.6**, 6419 games, LOS 100 %. | CONFIRMED (chesso measured −1.65 ±4.22 and reverted, DEC-101) |
| A26 | SWAR packed eval score +25.41 | `plan.md:309`; S117 | https://github.com/jhonnold/berserk/pull/65 "Evaluation uses single score eval", 2021-04-25 | **+25.41 ±11.28** at 8+0.08 Hash 8, 1808 games, bounds [−4, 1]; the PR is 17 commits including tuning iterations and bug fixes; bench 6589641 → 7019944. | DIFFERS — number right, attribution loose: it prices a 17-commit bundle with tuning, not the packing alone (S117's text already says so) |
| A27 | Stash ledger: mobility +20 class, passers with king distance +22.3, connected/phalanx +25.4, threats +10 | `plan.md:328-331`; DEC-087(i) | https://raw.githubusercontent.com/mhouppin/stash-bot/master/CHANGELOG.md | v27 mobility zone excluding rammed and low-rank pawns **+19.95 ±9.63** (10+0.1); v32 king proximity in passed-pawn eval **+22.27 ±9.86** (8+0.08); v31 connected pawns (phalanx and defender) **+25.38 ±10.40** (8+0.08) / +18.57 ±8.45 (40+0.4); v26 dynamic initiative from pieces threatened by lower-valued ones **+10.13 ±6.50** (10+0.1) / +7.77 ±5.23 (60+0.6). Also: v31 king-safety rewrite +4.24 then +9.69 (~+13 total), v32 king square out of mobility zone +10.86. | CONFIRMED (the "+10 threats" is Stash's *initiative* term, built on threatened pieces) |
| A28 | Published fits: bishop pair +16.7, rook open file +4.2, half-open +9.2, rook on seventh +12.99 | `plan.md:180-182` (five terms, four numbers) | `plan_done/S100:43-45, 221-227` (maps +16.7 bishop pair, +9.2 rook and queen on the seventh, +4.2 rook on open file, +12.99 tempo; says the +16.7/+4.2/+9.2 URLs "were not re-located"); https://github.com/TerjeKir/weiss/pull/241 ; https://github.com/TerjeKir/weiss/pull/231 ; https://github.com/lynx-chess/Lynx/pull/390 ; Weiss PR #95 via API | Weiss #241 "Tempo" (2020-04-11): **+12.99 ±7.35** (10+0.1) / +6.67 ±4.67 (40+0.4) — so +12.99 is **tempo**, not rook on seventh. Weiss #231 "Tune Rook+Queen" (rook/queen PSQT plus open and semi-open file bonuses, 2020-04-06): **+9.86 ±5.74** / +8.40 ±5.42. Lynx #390 tapered bishop pair (2023-09-10): **+8.2 ±6.1** / +9.5 ±6.7. Weiss #95 "Tune Eval 1" (CLOP of bishop pair and KLVuln, 2019-11-26): +16.10 ±8.74 / +12.24 ±7.21. | DIFFERS / NOT FOUND — +12.99 is a tempo figure; the traced rook-file retune is +9.86 not +9.2; no source gives +16.7, +4.2 or +9.2 exactly, and none gives a half-open-file figure on its own. Searched: Weiss PRs for "bishop pair", "seventh", "open file"; Berserk PRs for "bishop pair"; Stash changelog (only "removed knight pair and rook pair bonuses"). |
| A29 | Syzygy worth 13 to 25 Elo | `plan.md:339-340`; S129 | https://www.chessprogramming.org/Syzygy_Bases ; https://www.talkchess.com/forum3/viewtopic.php?t=70110 | CPW: Stockfish 10dev (classical), 10+0.1, all WDL in RAM, 6-men: **+13 Elo**; Stockfish 15 (NNUE) 6-men: +2.7. Talkchess (konsolas, 2019-03-05): Topple "about 25 elo from syzygy 6 piece tablebases"; Minic with 3-man: 0 Elo. | CONFIRMED — both endpoints are **6-men** figures; S129 is 3–5 men, for which the only fetched figure is Minic's 0 at 3-man |
| A30 | An 8-thread search is worth about +180 at LTC | `plan.md:110`; DEC-089 | https://official-stockfish.github.io/docs/stockfish-wiki/Useful-data.html | 8 threads vs 1 at LTC 60+0.6, 8moves_v3: 476-3-521 [0.737], **Elo 178.6 ±14.0**. | CONFIRMED |
| A31 | 1.43 Elo per % nps at LTC, 2.10 at STC | `specs.md:117`; DEC-083; S104/S117/S120 | same page | "For small speedups (<~5%)": Elo_stc(x) = **2.10 x** (10+0.1), Elo_ltc(x) = **1.43 x** (60+0.6). Same page: hash at LTC (SF15.1) 16 MB +0.70 ±12.8 vs 64 MB, 8 MB −10.7 ±11.0. | CONFIRMED (linear only below ~5 %) |
| A32 | CCRL Blitz 1CPU: Stockfish 11 3565, Komodo 14.1 3482, Xiphos 0.6 3356, Ethereal 11.75 3346 | `specs.md:68-69`; DEC-071/085/089 | CCRL list (A1), August 28, 2026 | Stockfish 11 64-bit **3565 ±15** (4CPU 3619); Komodo 14.1 64-bit **3482 ±15** (8CPU 3560); Xiphos 0.6 64-bit **3356 ±7** (4CPU 3432, 8CPU 3465); Ethereal 11.75 64-bit **3346 ±35** (4CPU 3435). Unsuffixed rows are the 1CPU entries. | CONFIRMED — all four unchanged today |
| A33 | OpenBench presets test STC at 8 to 32 MB hash, 8+0.08 | `plan.md:62, 66-67`; DEC-088; `S105:76-77` | https://raw.githubusercontent.com/AndyGrant/OpenBench/master/Engines/*.json (20 engines listed in `Config/config.json`) | STC presets: Hash=8 for 16 engines, Hash=16 for Stash and Stockfish, Hash=32 for Seer and Weiss → range **8 to 32** ✓. Time control: **8.0+0.08** for 14 (4ku, Bit-Genie, BlackMarlin, Demolito, Drofa, Equisetum, FabChess, Halogen, Koivisto, Laser, Seer, Stash, Winter, Zahak); **10.0+0.1** for 6 (Berserk, Ethereal, Igel, RubiChess, Stockfish, Weiss). Stash: 8.0+0.08 Hash=16, as S105 says. | CONFIRMED for hash; DIFFERS for "the engines this plan reads from test at 8+0.08" — Ethereal, Berserk, Weiss and Stockfish test at 10+0.1; Stash and Lynx (per its PRs) at 8+0.08 |

Not checked: `plan.md:63`'s "every surveyed engine defaults to a UHO book" — no book field was read from the preset files.

---

## B. What the strongest engines and the literature have, against the plan's steps

Inventory sources fetched: Stockfish `src/search.cpp` (master); Ethereal `src/search.c` and `src/evaluate.c` (master); Berserk `src/search.c` (main) and `src/eval.c` at tag 4.3.0; Weiss `src/search.c` and `src/evaluate.c` (master); Stash `src/sources/search.c` and `evaluate.c` (master, v37.26); Leorik `Leorik.Search/IterativeSearch.cs` (master) and release notes; CPW pages Search, Evaluation, plus the engine pages Ethereal/Berserk/Weiss/Stash/Leorik. Chesso's current state is `adocs/specs.md:387-396` and `MANUAL.md`. Step ids are from the `## Open` list (54 entries) and the `goal:` lines of `plan_todo/`.

"No step" means no Open entry's goal covers it; where the plan records the omission deliberately, that is said. "CPW number" is given only where a fetched page states one.

### B1. Search, pruning, extensions

| Technique | Who has it (fetched) | Chesso today | Plan step | Verdict |
|---|---|---|---|---|
| Aspiration windows | SF, Ethereal, Berserk, Weiss, Stash, Leorik | present (S021, S085) | S115 (fail-soft, halving, fail-high root reduction) | step |
| TT cutoff / hash move | all six | present | — | present |
| Mate distance pruning | SF, Ethereal, Berserk, Weiss, Stash, Leorik | absent | none — `plan.md:265-267` records it as deliberately given no step | **no step** (deliberate). CPW: "will not add much"; no number |
| Razoring | SF (~0 Elo in 2019 removal test), Berserk, Stash, Leorik; Weiss 1.2 had it; Ethereal 11.81 removed it | absent | S116 (depth one) | step. CPW: SF 2022 reintroduction ≈ 1 Elo |
| Reverse futility / beta pruning | all six | present (S033) | S148 re-decides the depth ceiling | step |
| Null move pruning | all six | present | S114 (eval-scaled R, base R) | step |
| NMP verification search | SF, Berserk, Stash | absent | dropped from S114 by DEC-087(e) | **no step** (deliberate) |
| ProbCut | SF, Ethereal (−9.08 on removal), Berserk, Weiss, Stash; Lynx never had it | absent | S113 | step |
| Internal iterative reduction | SF, Ethereal, Berserk, Weiss, Stash | absent | S095 | step |
| Internal iterative deepening | CPW; none of the six current sources | absent | none | **no step**. CPW: "pretty much a washout on average"; no number |
| Singular extensions | SF, Ethereal, Berserk, Weiss, Stash | absent ("extensions of any kind") | S097 | step |
| Multi-cut | SF, Ethereal, Weiss, Stash | absent | S097 | step |
| Negative extensions | SF, Berserk, Weiss, Stash | absent | named in S097 as a later refinement, "not this step's" (`S097:107, 325`) | **no step owns it** |
| Double / triple extensions | SF ("TT-move extensions"), Weiss | absent | same as above (`S097:107`) | **no step owns it** |
| TT-move / ttPv-based extension or reduction terms | SF, Weiss, Stash, Lynx | absent (entry has no PV flag bit, `S098:140`) | S098 mentions the "ttPv wall"; no goal names it | **no step** |
| Check extensions | SF (conditional), Ethereal (in-loop), Stash, Weiss (CPW); removed by Stormphrax | LMR exempts checking moves | S096 retired, DEC-087(a) | retired (deliberate) |
| Late move pruning | all six (Ethereal −76.88 on removal) | absent | S109 | step |
| Futility pruning (parent, quiet moves) | all six | absent | S109 | step |
| History / continuation pruning | SF, Ethereal ("continuation pruning"), Berserk, Weiss, Stash | absent | S109 | step |
| SEE pruning in the main search | all six (Ethereal −41.54 on removal) | quiescence only | S091 (captures) and S109 (quiets) | step |
| Late move reductions | all six (Ethereal −248.59, SF ~208) | present | S098 | step |
| LMR scaled by history, node type, re-search result | SF, Berserk, Weiss, Stash, Leorik (history, SEE) | depth and move number only | S098 | step |
| Cut-node / all-node reduction scaling | SF, Berserk, Weiss, Stash | absent | S098 | step |
| Improving flag | SF, Berserk, Weiss, Stash, Leorik | defined, no consumer (S108) | S109 supplies consumers | step |
| Opponent worsening | SF | absent | none | **no step** |
| Quiescence delta pruning | SF, Ethereal, Berserk | absent (`specs.md:393`) | S022 decides delta vs per-move futility | step |
| Quiescence per-move futility | Weiss, SF ("quiescence futility") | absent | S112 | step |
| Quiescence stand-pat from TT | Weiss, Ethereal, Stash (v26) | present (S130) | done | present |
| Quiescence non-capture promotions | — (engine TODO) | filtered out | S131 | step |
| Killer heuristic | all six | present | S159 (slot ageing) | step |
| Killer reset two plies down | recorded by the 2026-09-03 audit | absent | none — `plan.md:265-267`, deliberate | **no step** (deliberate) |
| Countermove heuristic | Berserk, Stash, Weiss | present | — | present |
| Quiet history with malus/gravity | all six | present (S093) | done | present |
| Continuation history (1-, 2-ply) | all six | absent | S024 | step |
| Capture history | SF, Ethereal, Berserk, Weiss, Stash | absent | S023 reserve, S025 reserve | reserve |
| Pawn history (quiet move × pawn structure) | SF, Weiss | absent | none | **no step** |
| Correction history — pawn | SF, Berserk, Weiss, Stash, Leorik 3.1+ | absent | S099 | step |
| Correction history — non-pawn / material | SF, Stash | absent | S110 reserve | reserve |
| Correction history — minor / major piece | Stash; CPW lists both | absent | none | **no step** |
| Correction history — continuation | SF (PR #5617), CPW | absent | S111 reserve | reserve |
| Correction history — threats / last-move | CPW variants | absent | none | **no step** |
| Staged move generation | Ethereal, Stash, Berserk 4.3 | present | — | present |
| Syzygy probing | SF, Ethereal, Berserk, Weiss, Stormphrax | absent | S129 (3–5 men, last, optional) | step |
| Lazy SMP / threads | all six | single-threaded (`MANUAL.md:282`) | none — phase two, DEC-089 | **no step** (deliberate). SF wiki: 8 threads +178.6 ±14.0 at LTC |
| MultiPV | SF, Stash, Stormphrax, Weiss 2.0 | absent (no option in `MANUAL.md`) | none | **no step** (not a strength item; SF wiki: MultiPV 2 costs −97.2) |
| Pondering | SF, Weiss, Stash | `go ponder` ignored (`MANUAL.md:283`) | none | **no step** |
| Chess960 | Weiss 2.0, Stormphrax, Leorik 3.2 | absent | none | **no step** (not a strength item) |
| Contempt / draw contempt | Ethereal 11.93, Stormphrax | absent | none | **no step** |
| Rule-50 / GHI TT workaround | SF | quiescence probes without draw checks, recorded as accepted (S106) | none | **no step** |

### B2. Move ordering (beyond the history tables above)

| Technique | Who has it | Chesso today | Plan step | Verdict |
|---|---|---|---|---|
| MVV-LVA, SEE good/bad capture split | all | present | — | present |
| Retry losing captures after quiets | Weiss 1.2, Stash | not ordered that way | S025 reserve | reserve |
| 16-bit move encoding | Weiss (#544 encodes the piece instead) | 32-bit with piece | S030 | step |

### B3. Evaluation (hand-crafted, as shipped by Ethereal 12.x, Berserk 4.3, Weiss, Stash 37)

| Term | Who has it | Chesso today | Plan step | Verdict |
|---|---|---|---|---|
| Material, tapered PSQT | all | present | — | present |
| King-relative PSQT | Berserk 4.3 (enemy-king-side mirror), Leorik 2.5 (linear in both kings), Lynx ("king-bucketed PSQT") | absent | S133 | step |
| Material imbalance tables | Berserk 4.3 ("Imbalance"), CPW Material page | absent | none | **no step**. CPW: no number |
| Bishop pair | Ethereal, Berserk, Weiss, Stash | present at zero weight | S135 | step |
| Rook on open / semi-open file | Ethereal, Berserk, Weiss, Stash | present at zero weight | S135 | step |
| Rook on seventh | Ethereal | present at zero weight, degenerate with PSQT | S134 folds it into the tables | step |
| Tempo / initiative | all four | present at zero weight | S136 | step |
| Mobility over a mobility area, per-count curve | Ethereal, Berserk, Weiss, Stash ("Mobility Area") | linear over raw count | S121 | step |
| Isolated / doubled (stacked) / backward pawns | all four | "three pawn terms exist" | S125 (backward) | step |
| Connected / phalanx / supported (defended) pawns | all four | absent | S125 | step |
| Candidate passers | Ethereal, Berserk | absent | S123 | step |
| Passed pawns by rank, king distance, safe push, blocked, square rule | all four | rank buckets only | S123 | step |
| King safety: attacker weights, weak squares, safe checks, king zone | all four | linear attacker count, clamped | S122 | step |
| Pawn shelter / storm | Ethereal, Berserk, Stash, Weiss | absent | S118 (shelter cached), S122 | step |
| King defenders / knight defense | Ethereal, Berserk | absent | none | **no step** |
| Threats: minor attacked by pawn/minor/major, hanging pieces, pawn-push threats, overloaded pieces | Ethereal, Berserk, Weiss, Stash | absent | S101 ("a piece attacked by a lesser piece") | partial — hanging, pawn-push and overload threats are outside S101's goal |
| Outposts (and reachable outposts), space | Ethereal, Berserk, Stash | absent | S102 | step |
| Trapped pieces (bishop, rook) | Berserk 4.3, CPW Evaluation of Pieces | absent | none | **no step**. CPW: no number |
| Minor behind pawn, bishop long diagonal, bad bishop / rammed pawns | Ethereal, Berserk | absent | none | **no step** |
| Closedness adjustments (knight/rook value by pawn structure) | Ethereal | absent | none | **no step** |
| Complexity (winning-chance scaling by pawns, flanks) | Ethereal | absent | none | **no step** |
| Queen relative pin | Ethereal | absent | none | **no step** |
| Endgame scaling (opposite bishops, pawnless, material) | all four | absent | S124 | step |
| Endgame specializations / KPK bitbase / material-draw recognition | Stash (`endgame.c`, `kpk_bitbase.c`), Berserk (`endgame.c`, `kpk.h`), Leorik (KvKNN) | insufficient-material draws only | none beyond S124's scaling | **no step** |
| Pawn (pawn-king) hash | Ethereal, Berserk, Weiss, Stash, Leorik 2.1 | absent | S118 | step |
| Evaluation cache | CPW | absent | S120 | step |
| Lazy evaluation | Berserk 4.3, chesso | present, clamped | S039 | step |
| Packed mg/eg score (SWAR) | Ethereal (9.45), Berserk (#65), Stormphrax | two integers | S117 | step |
| NNUE | SF, Ethereal 13+, Berserk 6+, Leorik 3+, Stormphrax | absent | S029 parked (DEC-054) | parked |

### B4. Machinery and time management

| Technique | Who has it | Chesso today | Plan step | Verdict |
|---|---|---|---|---|
| TT buckets / clusters, ageing, prefetch, huge pages | Ethereal 11.92/11.99, Berserk 4.3 (bucket 4), Leorik (two buckets, ageing), Stash, SF | direct-mapped 24-byte entries, no prefetch, no page hint | S119 | step |
| PEXT sliding attacks | Leorik 2.5, SF (bmi2 builds) | magics | S032 | step |
| En passant square only when capturable | — | always set | S042 | step |
| In-check computed once per node | — | per call site | S020 | step |
| TM: best-move stability | SF, Stash, Berserk (#324) | present (S089) | — | present |
| TM: falling evaluation | SF, Stash | present (S089) | — | present |
| TM: best-move node fraction | SF ("effort"), Ethereal, Stormphrax, Lynx, Weiss (#752), Berserk (#324), Koivisto | absent | S132 | step |
| TM: move-type factors (large-SEE capture, checking move, only move) | Stash v26 | absent | none | **no step** |

### B5. The "no step" list, collected

Deliberately recorded in the plan as no-step: mate distance pruning, killer reset two plies down (`plan.md:265-267`); null-move verification search (DEC-087 e); check extensions (S096 retired, DEC-087 a); threads (DEC-089, phase two).

Not recorded anywhere in the plan or a step goal: negative extensions and double/triple extensions (named in S097 only as future refinements), TT-move/ttPv extension and reduction terms, opponent-worsening, internal iterative deepening, pawn history, minor/major/threats/last-move correction histories, MultiPV, pondering, Chess960, contempt, rule-50 TT handling; material imbalance, trapped pieces, minor-behind-pawn / long-diagonal / bad-bishop terms, closedness, complexity, queen relative pin, king defenders, hanging-piece and pawn-push threats, KPK bitbase and specialized endgames, move-type time-management factors.

CPW states no Elo number for any item in the second list (pages fetched: Mate_Distance_Pruning, Internal_Iterative_Deepening, Check_Extensions, Killer_Heuristic, Countermove_Heuristic, Material, Evaluation_of_Pieces, Pawn_Structure, King_Safety, Time_Management, Lazy_SMP). The one number found for a no-step item is threads: Stockfish wiki, 8 threads vs 1 at LTC +178.6 ±14.0.

---

## C. CPW definitions of the techniques the plan's steps describe

| Technique (step) | URL | One-line definition as the page states it |
|---|---|---|
| Internal iterative reductions (S095) | https://www.chessprogramming.org/Internal_Iterative_Reductions | Reduce the depth at a node that has no hash move, instead of running a reduced pre-search for one; Ed Schröder, Rebel 2020; later Stockfish (2021), Ethereal. No Elo stated. |
| ProbCut (S113) | https://www.chessprogramming.org/ProbCut | Buro 1994: a shallow search's result is used to predict, with a stated probability, that the deep search would fall outside the window, and the subtree is cut; Multi-ProbCut adds several depth pairs and phase-specific parameters; noted as working in Stockfish (Linscott). No Elo stated. |
| Singular extensions (S097) | https://www.chessprogramming.org/Singular_Extensions | At expected PV and cut nodes, extend the one move that a reduced null-window search with a lowered beta shows to be much better than every alternative; modern engines test the TT move with a lower-bound or exact entry. No Elo stated. |
| Multi-cut (S097) | https://www.chessprogramming.org/Multi-Cut | Björnsson 1998: reduced searches of several moves at an expected cut node; if enough of them fail high, prune the node. No Elo stated. |
| Correction history (S099, S110, S111) | https://www.chessprogramming.org/Static_Evaluation_Correction_History | Record the difference between static evaluation and search score in a table indexed by a board feature, and add it to future static evaluations with the same feature; Caissa, October 2023; variants pawn, material, threats, minor, major, non-pawn, last-move, continuation; gains grow with time control. No Elo stated. |
| Continuation history (S024) | https://www.chessprogramming.org/History_Heuristic | An n-ply continuation history is a history score indexed by the move played n plies ago and the current move, generalising counter-move history (Stockfish 7); 1- and 2-ply most common, up to 6-ply. No number stated. |
| Capture history (S023) | same page | A history table indexed by moved piece, target square and captured piece type, with bonus on cutoff and malus otherwise, used instead of or beside MVV-LVA. No number stated. |
| Late move pruning (S109) | https://www.chessprogramming.org/Futility_Pruning (Move Count Based Pruning section) | A variation of extended futility pruning combining Fruit's history leaf pruning and LMR: late quiet moves are skipped by move count at shallow depth. No Elo stated. |
| Futility pruning (S109, S112) | same page | Discard moves that cannot raise alpha, judged by static evaluation plus a margin at frontier nodes; extended futility (Heinz) applies at pre-frontier nodes with a larger margin. No Elo stated. |
| SEE pruning (S091, S109) | https://www.chessprogramming.org/Static_Exchange_Evaluation | SEE computes the material outcome of the capture sequence on one square; the page describes pruning captures in quiescence and moves in the main search under a depth-dependent threshold (linear for captures, quadratic for quiets) and ordering captures as good or bad. No Elo stated. |
| Razoring (S116) | https://www.chessprogramming.org/Razoring | Birmingham & Kent 1977: at shallow depth, when static evaluation plus a margin is at or below alpha, prune or drop straight into quiescence. Stockfish's 2022 reintroduction is credited with about 1 Elo. |
| Delta pruning (S022) | https://www.chessprogramming.org/Delta_Pruning | In quiescence, before making a capture, skip it if the captured piece's value plus a safety margin (typically ~200 centipawns) cannot raise alpha. No Elo stated. |
| Aspiration windows (S115) | https://www.chessprogramming.org/Aspiration_Windows | Search the root with alpha-beta bounds around a guessed score; on a fail, widen only the failing bound, exponentially in modern engines. No Elo stated. |
| Null move pruning: verification, eval-scaled R (S114) | https://www.chessprogramming.org/Null_Move_Pruning | Pass the move and search reduced; a beta cutoff prunes. Verified null move (Tabibi/Netanyahu) re-searches to handle zugzwang — Hyatt found it "does not help at all in Crafty"; adaptive R varies with depth, with the evaluation-minus-beta difference and with endgame features. No Elo stated. |
| Reverse futility pruning (S148) | https://www.chessprogramming.org/Reverse_Futility_Pruning | Static null move pruning: at shallow depth, return when static evaluation minus a margin is at or above beta, relying on the null move observation. No Elo stated. |
| King-relative PSQT (S133) | https://www.chessprogramming.org/Piece-Square_Tables — **not described there** (one 2013 forum title only). Engine writeups: https://github.com/jhonnold/berserk/releases/tag/4.3.0 ; https://github.com/lithander/Leorik/releases/tag/2.5 | Berserk 4.3: piece-square tables indexed by the side of the board the enemy king stands on. Leorik 2.5: piece-square values computed as linear functions of both kings' positions and the game phase (18 parameters). | 
| Mobility area (S121) | https://www.chessprogramming.org/Mobility | Mobility is the number of moves or reachable squares; "safe mobility" counts only squares not attacked by enemy pawns (best for knights). The term "mobility area" and curve bonuses are **not** on the page; Stash's `evaluate.c` names a "Mobility Area" and its changelog v27 prices a zone excluding rammed and low-rank pawns at +19.95. |
| Pawn hash (S118) | https://www.chessprogramming.org/Pawn_Hash_Table | A hash table caching pawn-structure evaluation keyed by the pawn hash; a few thousand entries give hit rates above 95–99 %. No speed or Elo figure. |
| Syzygy (S129) | https://www.chessprogramming.org/Syzygy_Bases | Compact endgame tablebases with WDL and DTZ files and heavy compression; Stockfish 10dev classical 6-men +13 Elo at 10+0.1; Stockfish 15 NNUE +2.7. |
| PEXT bitboards (S032) | https://www.chessprogramming.org/BMI2 | PEXT/PDEP map the relevant occupancy bits to a dense index ("fancy PEXT bitboards") as an alternative to magic multiplication; de Man's PEXT/PDEP hybrid quarters the table size. No speed comparison stated. |
| 16-bit move encoding (S030) | https://www.chessprogramming.org/Encoding_Moves | Six bits each for from and to squares plus a four-bit nibble for move kind and promotion piece; moving and captured pieces are read from the board rather than stored. |
| SWAR / packed evaluation score (S117) | CPW Tapered_Eval, Score, SIMD_and_SWAR_Techniques — **none describes packing mg and eg into one integer** as a technique (forum links only). Engine writeups: Berserk PR #65; Ethereal V12.00 notes line "9.45: Merge MG and EG scores into one value, like SF" | Two 16-bit halves carried in one 32-bit integer so one addition updates both phases. **NOT FOUND on CPW** as a definition. |
| Evaluation hash / cache (S120) | https://www.chessprogramming.org/Evaluation_Hash_Table | A Zobrist-keyed table caching expensive positional evaluation, separate from the TT with its own replacement policy, giving "a considerable amount of additional hits". No figure. |
| TT buckets / clusters (S119) | https://www.chessprogramming.org/Transposition_Table | Bucket systems: several entries per bucket sized to a cache line, the shallowest entry replaced; ageing by storing the halfmove clock modulo a power of two and preferring stale entries for replacement. Huge pages appear only as a See-also link. |
| Prefetch (S119) | CPW `Prefetch` and `Cache` pages: **HTTP 404**. https://www.chessprogramming.org/Memory (Cache section) links "Cache prefetching" and "The prefetch instruction" without a definition. Engine writeup: Ethereal V12.00 notes "11.92: Transposition Table prefetching". | **NOT FOUND** as a CPW page. |
| Huge pages (S119) | https://www.chessprogramming.org/Memory (Huge Pages section) | Windows "large pages" versus Linux "huge pages"/huge TLB pages, with links; no TT-specific discussion. Ethereal 11.99 added "Transparent Huge Pages kernel advice". |
| Time management by node fraction (S132) | https://www.chessprogramming.org/Time_Management | Lists "the ratio of the size of the subtree under the best move versus the size of the whole search tree" as a consideration, with no method and no number; best-move changes and score trend also listed. Engine writeup: Ethereal commit 60f4d5c5 (+9.85/+9.66, credits Koivisto). |

Extra definitions fetched for Part B (no plan step): Mate_Distance_Pruning — cut lines where no shorter mate is possible, "will not add much"; Internal_Iterative_Deepening — reduced-depth pre-search to obtain a first move, "washout on average"; Check_Extensions — extend when giving or evading check, some programmers omit it; Killer_Heuristic — order moves that cut off in a sibling node first (no reset rule described); Countermove_Heuristic — Uiterwijk 1992, a natural reply to the previous move; Lazy_SMP — threads search the same root with varied depth/ordering, "scales surprisingly well up to 8 cores and beyond" (no number).
