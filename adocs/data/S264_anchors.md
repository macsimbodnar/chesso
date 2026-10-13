# S264: two rating anchors above 2850, Weiss 1.1 and Stash v30.0

**Status: done. The reading and the choice (phase 1, 2026-10-12/13, no CPU used
while S119's SPRT held the machine, DEC-260), the builds, md5s and probes
(phase 2, 2026-10-13) and the smoke run (2026-10-13, launched by the
coordinator) all passed.**

## The list, read 2026-10-12

| | |
|---|---|
| fetched | 2026-10-12 23:57 CEST, `curl -sSL` |
| URL | `https://computerchess.org.uk/ccrl/404/rating_list_all.html`, which now answers **302** to `https://computerchess.org.uk/404/rating_list_all.html` |
| list computed | "October 10, 2026", the page's own line |
| page | 2390776 bytes, md5 `8143a9ccb098a61bc8333544cd4c7e8d`, kept under `.tuning/coord/S264/ccrl/` and not committed |
| rows | 3005 entries, 2625 of them 1CPU (no `<n>CPU` suffix) |
| in the band | 466 1CPU entries from 2840 to 3210, 442 of them from 2850 to 3200: `S264_ccrl_blitz_band_2026-10-12.tsv`, rating, interval, games, list name |

Extracted without python, because a match held the machine:

```bash
grep 'class="name"' rating_list_all.html \
  | sed -e 's#</td>#\t#g' -e 's/<[^>]*>//g' -e 's/&nbsp;//g' -e 's/&#8722;/-/g' -e 's/&amp;/\&/g' \
  | awk -F'\t' '{gsub(/^ +| +$/,"",$2); print $2"\t"$3"\t"$4"\t"$5"\t"$6"\t"$7"\t"$8"\t"$9}' > list.tsv
awk -F'\t' -v OFS='\t' '$2>=2840 && $2<=3210 && $1 !~ /[0-9]CPU$/ {print $2, $3, $4, $8, $1}' list.tsv \
  | sort -t$'\t' -k1,1n -k5,5
```

The five incumbents on the same read: Blunder 7.1.0 2389 +/-18 (1049
games), Leorik 2.1 2568 +/-18 (990), Blunder 8.5.5 2663 +/-11 (2534), Stash
21.0 2714 +/-14 (1493), Leorik 2.4 2830 +/-11 (2437). S240 read 2388, 2568,
2662, 2713 and 2830 on 2026-09-27.

## The two chosen

| | Weiss 1.1 | Stash v30.0 |
|---|---|---|
| CCRL Blitz, 2026-10-12 | **2987 +18/-18, 893 games** (`Weiss 1.1 64-bit`) | **3153 +17/-17, 953 games** (`Stash 30.0 64-bit`) |
| author, language | Terje Kirstihagen, C | Morgan Houppin, C |
| licence | GPL-3.0 (`COPYING.txt`) | GPL-3.0 (`LICENSE`) |
| source | `https://github.com/TerjeKir/weiss.git`, tag `v1.1`, `5c3bd88785ad888bb181c32f15c123400ef53caa` (2020-09-02) | `https://gitlab.com/mhouppin/stash-bot.git`, tag `v30.0`, `2c588cdb1fef59affaafe16a1b983156635cdf78` (2021-04-30) |
| clone | `/home/max/ws/engines/weiss-1.1`, `--depth 1 --branch v1.1` | `/home/max/ws/engines/stash-30.0`, `--depth 1 --branch v30.0` |
| build | `make pext` in `src/`: gcc, `-O3 -flto -march=native -DUSE_PEXT -mbmi2`, no PGO, the author's `release` target's codegen without `-static` | `ARCH=x86-64-bmi2 sh utils/unix_build.sh`: two-pass PGO, exactly as S088 built v21.0; the author's own release binaries are PGO too (`utils/release_build.sh`) |
| binary | `/home/max/ws/engines/weiss-1.1.bin` | `/home/max/ws/engines/stash-30.0.bin` |
| md5 | `a7868b74724741e357ae6c3ea23f3ecf`; **reproduces**: a second `make pext` gave the same hash, so it is a rebuild check on this CPU and gcc 13 only (`-march=native`) | `36cc28332b5bad10c05589d474c165cb`; **identity only**: a second PGO build gave `f27240eb956d0f09f8e833342a9183ef`, with the same bench node count, 13720671 |
| build log | 0 errors; 3 `-Wsign-compare` warnings, all in the bundled Syzygy prober | 0 errors, `-Werror` held through both passes; one `lto-wrapper` note |
| `id name` | `Weiss 1.1`, as `src/uci.h:25` `NAME` says; probed | `Stash v30.0`, as `src/sources/uci.c:196` says; probed |
| `ccrl_name` | `Weiss 1.1` | `Stash 30.0` -- differs from `uci_name`, as v21.0's does |
| options, from source | Hash 2..65536 default 32, Threads 1..2048 default 1, SyzygyPath empty, NoobBook check default false | Threads 1..256 default 1, Hash 1..33554432 default 16, Move Overhead 0..30000 default 100, MultiPV, UCI_Chess960, Clear Hash |
| evaluation | hand-crafted (`evaluate.c`, `psqt.c`), no network file | hand-crafted (`evaluate.c`, `pawns.c`, `endgame.c`, a KPK bitbase computed at start), no network file |
| time margin | a fixed 5 ms a move, and never more than 80 % of the clock (`src/time.c`) | `Move Overhead` 100 ms |

**Why Weiss 1.1.** A family with five 1CPU entries across the band (0.10
2847, 1.0 2896, 1.1 2987, 1.2 3056, 1.3 3136), so a later rung comes from the
same build path. A fourth author and a fourth family, so it votes
independently of all three incumbents (DEC-072's argument). C, built with gcc
and make alone; no network file; a tag per release. Its rung sits at the mark
itself: S152 reads 3000, and an anchor at 2987 puts that reading inside the
set instead of past it, at the score where the logistic curve carries the
most information.

**Why Stash v30.0.** The family the engine-testing guide recommends for a
gauntlet and S088 already uses, with six entries from 2933 to 3208. It tops
the set at the step's 3150. Being Stash 21.0's own line nine versions on, the
pair is 439 Elo apart on the list from one author, so the measured gap between
the two Stash rungs reads the scale (DEC-077's compression, ratio 0.853)
without a family difference inside it. The build path is the one S088
proved.

The set becomes seven rungs and four families -- 2389, 2568, 2663, 2714,
2830, 2987, 3153 -- spanning 764 Elo, with 157 and 166 between the new rungs
and the old top.

## What the source reading found

**Stash's startup deadlock is closed in v30.0, by construction.** S088 found
v21.0 parks its engine thread on an unconditional `pthread_cond_wait` while
`main()` goes straight to reading commands, so a `go` already in the pipe at
`exec` loses the broadcast (5/5 hangs from a shell pipe). In v30.0
`EngineMode` starts at `THINKING` (`src/sources/main.c:37`), and `main()`
calls `wait_search_end()` (`main.c:75`), which waits under the mutex for
`WAITING`; the engine thread sets `WAITING` and broadcasts while holding the
mutex and releases it only inside `pthread_cond_wait` (`engine.c:357-365`), so
no command is read before the thread is parked. v21.0 initialised the mode to
`WAITING` and had no such wait (`stash-21.0/src/sources/misc/globals.c:42`).
Measured in phase 2, below: v21.0 hangs 10/10 and v30.0 answers 20/20 under
the same probe.

**Weiss 1.1 can open a socket, and does not by default.** `NoobBook` queries
an online opening book (`noobprobe/noobprobe.c`, `socket(AF_INET, ...)`) only
when the option is true (`search.c:562`); it defaults to false and
`rating.sh` sends only `Hash`. Its search thread is created per `go`
(`uci.c:75`) and its wait loop re-checks a predicate (`threads.c:64-69`), so
the Stash v21.0 race has no counterpart. Phase 2's `strace` shows no `socket`
or `connect`.

**Both expose `Threads`, default 1.** `rating.sh` sends no `Threads`, so both
play on one thread like every other rung. `rating.sh`'s comment "None of the
reference engines exposes it" is now untrue for these two; the behaviour it
protects is unchanged.

**Hash.** `rating.sh` sends 128 MB; Weiss's maximum is 65536 and Stash
v30.0's 33554432, both far above it. `DEV_MANUAL.md` named "Blunder at 256 MB"
as the binding maximum; both Blunders in the set print `max 32000`
(`engine/uci.go` at v7.1.0 and v8.5.5), so that line described the first
set's Blunder 5.0.0 (`max 256` at `/home/max/ws/blunder`, `v5.0.0`) and is
corrected. Phase 2's option dump puts the binding maximum at Leorik's 2047
(2.1 and 2.4).

## Rejected

| candidate | CCRL Blitz | why not |
|---|---|---|
| Stash 25.0, 27.0, 28.0, 29.0, 31.0 | 2933, 3049, 3083, 3128, 3208 | the same family; one more Stash rung is enough, and 30.0 sits at the target top. **29.0 is the fallback** if 30.0 fails phase 2 |
| Weiss 0.10, 1.0 | 2847, 2896 | within 70 Elo of Leorik 2.4; no rung toward 3000. **Weiss 1.2 (3056 +/-15, 1195 games) is the fallback** if 1.1 fails; 1.3 (3136) would sit beside Stash 30.0 |
| Leorik 2.5, 3.0 | 2918, 3265 | a third Leorik rung adds no independent vote; 3.0 is above the band. The release-binary path is proven (DEC-069), so 2.5 is a further fallback |
| Blunder | -- | no entry above 8.5.5's 2663 |
| CuckooChess 1.13 | 3071 +/-12 | Java; no JVM on this machine (`command -v java javac` empty); a JVM is a dependency (DEPS) |
| Gaviota 1.0 | 2869 +/-9 | no engine source found (only its tablebase code is published, MIT), so binary only; and 39 above the current top, no help toward 3000 |
| Crafty 25.6, 25.3 | 2970, 2975 +/-11 | speaks the xboard protocol (CECP); `fastchess` drives UCI, so it needs an adapter, a dependency. Its licence is its own, not OSI |
| DiscoCheck 5.2.1 | 2930 +/-9 | one entry, no family across the band; the author's repository (`github.com/lucasart/chess`) no longer answers `git ls-remote`. Its successor Demolito tags only `20211004` (3245, above the band); the in-band Demolito builds are untagged |
| Zurichess Neuchatel | 2919 +/-9 | one entry; Go, Blunder's language (DEC-072's own reason); `github.com/brtzsnr/zurichess` does not answer and the original host was Bitbucket |
| Zahak 7.1 | 2972 and 2900 | the list carries `Zahak 7.1 64-bit` and a 32-bit `Zahak 7.1`; `tools/ccrl_rating.py` strips " 64-bit" and would take whichever row came first, silently |
| Texel 1.07 | 3130 +/-11, 2697 games | C++, GPL-3, buildable: the strongest alternative for the top rung on interval, rejected for Stash's coverage of the band and its proven build. **The alternate if a fifth family is wanted** |
| Combusken, Counter | 2972-3177, 2899-3152 | Go is present (1.22.2) but is Blunder's language; Combusken 1.4.0 (3177 +/-11) is an alternate |
| Rust families: Inanis, Velvet, Princhess, Frozenight, Viridithas, Simbelmyne | 2851-3189 | cargo 1.95 is present, but a cargo build downloads its crates, more untrusted code than two gcc builds; Inanis 1.6.0 (3091 +/-11, 2554 games) is the best of them |
| Lozza, tomitankChess; Bagatur, chess22k; Avalanche; Amoeba; ChessBrainVB | various | JavaScript, Java, Zig, D and Visual Basic: no toolchain here |
| Francesca, Cheese, Seawall, PeSTO, HIARCS, Deep Junior, Rybka, Fritz, Zappa and the like | various | closed source or commercial: nothing to build |
| Ethereal | 3333 and up | its lowest entry is above the band |

Toolchains present, checked with `command -v` only: gcc 13 (14 also
installed), make, cmake, go 1.22.2, rustc and cargo 1.95.0. Absent: clang,
dotnet, java, node, zig.

## Phase 2, 2026-10-13: builds and probes

The machine idle (load 0.35), both clones checked clean at their recorded
commits, and each tag resolving to that commit, before anything was built.
Logs under the worktree's `.tuning/coord/S264/`; the commands are in the step
file.

**Identity, all seven rows, as `rating.sh`'s `identify()` asks.** Every
binary answers its manifest `uci_name`, and the five incumbents' md5s equal the
manifest's, so S240's installed set is unchanged.

**Options.** No incumbent exposes `Threads`; Weiss 1.1 and Stash v30.0 do,
default 1. Hash maxima: Blunder 7.1.0 and 8.5.5 32000, Leorik 2.1 and 2.4
2047, Stash v21.0 131072, Weiss 1.1 65536, Stash v30.0 33554432. Weiss's
`NoobBook` prints `default false`; Stash v30.0's `Move Overhead` 100 (v21.0's
is 20).

**Libraries, files, network.** `ldd`: libc, libm, the loader and the vDSO, for
both. An `strace -f` of a timed search with `Hash 128` opened nothing but
libc, libm and locale files and made no `socket` or `connect` call, for both;
each returned a `bestmove`.

**The shell-pipe probe, and the form it had to take.** Commands in the pipe
before the engine has started (`uci`, `position startpos`, `go depth 10`, then
`quit` two seconds later), a `bestmove` line counted per trial, 10 s timeout:

| binary | form | trials | answered | hung or crashed |
|---|---|---|---|---|
| Stash v21.0 | with `isready`, unpinned (the form drafted in phase 1) | 5 | 5 | **0** |
| Stash v21.0 | without `isready`, unpinned | 10 | 8 | 2 |
| Stash v21.0 | with `isready`, pinned to one core | 10 | 2 | 8 |
| **Stash v21.0** | **without `isready`, pinned to one core** | 10 | 0 | **10** |
| **Stash v30.0** | **without `isready`, pinned to one core** | 20 | **20** | 0 |
| Stash v30.0 | with `isready`, pinned | 20 | 20 | 0 |
| Weiss 1.1 | without `isready`, pinned | 20 | 0 | **20, segfault** |
| Weiss 1.1 | with `isready`, pinned | 20 | 20 | 0 |

**The drafted form did not establish the precondition**: v21.0 answered 5/5
on this idle 12-thread machine, against S088's 5/5 hangs, because the race is
between the engine thread's start and the main thread reading the `go`.
Pinning the process to one core (`taskset -c 3`) leaves the new thread no
core to start on before the `go` is read, and v21.0 then hangs 10/10. Under
that form **v30.0 answers 20/20**: the deadlock is closed, as the source
reading said.

**Weiss 1.1 segfaults on a `go` sent before any `isready`** (exit 139, 3 of 3
re-checked). Its hash table is allocated by `isready` (`UCIIsReady` calls
`InitTT`), and `ucinewgame` does not touch an untouched table, so the first
`go` searches with a null table. With `isready` first it answers 20/20. A
match does not reach it: fastchess's `UciEngine::ucinewgame()` sends `isready`
and waits for `readyok` (read in the fastchess source; the smoke run is the
test on the installed 1.8.1), and `rating.sh`'s `identify()` sends no `go`.
Recorded in `references.tsv` and `DEV_MANUAL.md`.

**The CCRL names resolve.** `tools/ccrl_rating.py` on all seven names against
the live list, 2026-10-13: exit 0, Blunder 7.1.0 2389, Leorik 2.1 2568, Blunder
8.5.5 2663, Stash 21.0 2714, Leorik 2.4 2830, Weiss 1.1 2987, Stash 30.0 3153,
unchanged from the 2026-10-12 read (the list was computed 2026-10-10). Python's
`urllib` follows the 302 to `/404/rating_list_all.html`, so the tool reads the
list as it stands.

**The candidate for the smoke run**, the worktree's own Release build at
`cead820`: `bench` 3872032 nodes.

## The smoke run, 2026-10-13

`rating.sh --bracket` from the worktree, launched by the coordinator exactly as
the step file writes it: `MANIFEST` of Leorik 2.4, Weiss 1.1 and Stash v30.0
(the third rung only because `rating.sh` needs three), `ROUNDS=100`, every
other setting `rating.sh`'s own -- 10+0.2, Hash 128, concurrency 12 of 12,
`8moves_v3.pgn`, two-sided adjudication. Candidate `cead820` plus this step's
uncommitted documents, `bench` 3872032.

| check | result |
|---|---|
| marker | `RATING-RUN-DONE bracket OK` |
| identity | passed for all three before the first game |
| games | 600, 200 per pairing; 516 `adjudication`, 84 `normal` |
| terminations outside normal, adjudication, time forfeit | 0 |
| log lines matching disconnect, stall, illegal move or crash (`rating.sh`'s own grep) | 0 |
| `illegal`, `segmentation`, `crash` or `disconnect` in `fastchess.log` | 0 |
| time forfeits | 0 of 200 for Weiss 1.1, Stash v30.0 and Leorik 2.4 each; 0 of 600 for chesso |
| console trimmed | `trim_console.py` dropped 6738 "PV continues" blocks (26952 lines), all Leorik 2.4's; `report.txt` 101457 bytes |
| wall | 1663 s from the start stamp (08:43:59) to `report.txt`'s last write (09:11:41.95, its mtime; the planned end stamp was not written), fastchess's own total 27 m 42 s agreeing: **1299 games an hour**, 3.7 % under S240 run 2's 1349; it includes `rating.sh`'s start, which a 600-game run carries proportionally more than a 4676-game one |

Weiss 1.1 played 200 games under fastchess 1.8.1 without a crash, so its
`isready` defect is not reached by a match, as the fastchess source said.

**No rating is read from this run (DEC-108), and none is in the tree.**
`h2h.txt`, `ordo_bracket.txt` and the report's bracketing block were not read.
`S264_smoke_report.txt` is the report cut before `=== bracketing`
(`sed '/^=== bracketing/,$d'`) and then stripped of the 30 standings tables
fastchess prints into its console as it plays, by shape and without reading
them: `adocs/data/S264_strip_standings.awk` (mawk-safe: no interval braces) drops every
block from a dashed rule followed by a line beginning `Rank Name` through the
next dashed rule, 30 blocks and 210 lines (1460 to 1250). Afterwards the file
holds no dashed rule, no `Rank Name`, no `Elo`, `nElo`, `Ptnml`, `LOS` or
`LLR`, and no pentanomial row; all 600 `Started game` and 600 `Finished game`
lines, the trim counts, the terminations and the forfeit census remain.
`S264_smoke_forfeits.txt` is the run's forfeit census. The run directory stays
under the worktree's `.tuning/coord/S264/smoke/` and is not archived.

## S152's cost with seven anchors

S240's valid run 2 measured `rating.sh`'s throughput on this machine: 3340
games in 2 h 28 m 31 s (fastchess's own total) at 10+0.2, concurrency 12, so
**1349 games an hour** and **29 m 42 s a pairing** (668 games). Run 1, 2 h 30 m
45 s, was void and shared the machine for its first hour, so it is not the
reference. A gauntlet costs `668 x P / 1349` hours at P pairings: 3.47 h for
seven, against 2.48 h for five. S152 plays two gauntlets; the second control
scales the time by `k`, so the pair costs `3.47 x (1 + k)` hours. The smoke
run read 1299 games an hour, 3.7 % under, which at seven pairings is 3 h 36 m.
The figures are in S152's file.
