id:         S264
goal:       the rating reference set brackets about 2750 to 3150 on the CCRL Blitz 1CPU scale, so S152's run can read a rating near 3000 without extrapolating past its top anchor
accepts:    (1) at least two engine families with 1CPU entries between about 2850 and 3200 join S088's five anchors; each candidate's CCRL Blitz rating, interval and game count read on the day from the list and recorded with the read date; licence noted; (2) each binary built or downloaded by the agent (DEC-069), its md5 and version string recorded in `references.tsv`; an engine that needs a network file is acceptable as an anchor -- running another engine's binary is a tool use (DEC-016) -- and the file's md5 is recorded with it; (3) one smoke match per new anchor under `rating.sh`'s own settings on the workstation: 0 crashes, 0 illegal moves, forfeits within DEC-075's 1 %, the console trimmed by `tools/trim_console.py`; (4) the cost of S152's two gauntlets with the larger manifest is recomputed (`rating.sh` plays 334 rounds, 668 games, per pairing) and stated in S152's file; (5) no rating is read from the smoke matches (DEC-108)
touches:    references.tsv, rating.sh only if an anchor needs an option, DEV_MANUAL.md, adocs/data/ for the anchors record, adocs/plan_todo/S152_rating_checkpoint_after_search_block.md for the cost line
excludes:   changing `rating.sh`'s regime; the rating run itself, which is S152
decisions:  DEC-258, DEC-016, DEC-069, DEC-072, DEC-075, DEC-077, DEC-108, DEC-234
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-13 -- the reference set brackets about 2750 to 3150: Weiss 1.1 (CCRL Blitz 2987 +/-18, 893 games) and Stash v30.0 (3153 +/-17, 953 games), read 2026-10-12 from the list computed 2026-10-10, join S088's five as the sixth and seventh rungs and the fourth family; seven rungs 2389 to 3153. Both built here from tagged GPL-3 source under /home/max/ws/engines (DEC-069): Weiss 1.1 `make pext`, md5 a7868b74724741e357ae6c3ea23f3ecf, reproduced by a second build (a rebuild check on this CPU and compiler only, `-march=native`); Stash v30.0 PGO as S088, md5 36cc28332b5bad10c05589d474c165cb, identity only (a second build differs, bench nodes equal). `uci` answers `Weiss 1.1` and `Stash v30.0`; no network file, no socket under strace; the five incumbents' md5s unchanged. Stash v30.0's startup deadlock closed: pinned to one core with the commands in the pipe, v21.0 hung 10/10 and v30.0 answered 20/20. Weiss 1.1 segfaults on a `go` before any `isready` (20/20), which fastchess does not send; recorded. tools/ccrl_rating.py resolves all seven names through the list's new 302. Smoke run under rating.sh's settings, 600 games against Leorik 2.4 and the two new anchors, 200 each: RATING-RUN-DONE bracket OK, 0 crashes, disconnects or illegal moves, 0 unexpected terminations, 0 forfeits for every engine (inside DEC-075's 1 %), report trimmed by trim_console.py and committed cut before its bracketing block and stripped of fastchess's 30 standings tables by shape; no rating read or committed (DEC-108); 1299 games an hour against S240 run 2's 1349. S152's cost line written from S240 run 2's 1349 games an hour: 4676 games, about 3 h 28 m at 10+0.2 (3 h 36 m at the smoke's 1299), and `3.47 x (1 + k)` hours for both controls. DEV_MANUAL's rating section updated (the set, hash 128, the Hash maxima with Leorik's 2047 binding, Weiss's isready crash, where the references live). Fallbacks if needed: Stash 29.0, Weiss 1.2. No src/ change. Fast check: five items fixed before commit (fastchess standings tables stripped from the committed report, unread; the trim check reads its own section; throughput from the report mtime; G from S240 run 2; S152 arithmetic and the Weiss md5 wording).

## Why

S240's top anchor, Leorik 2.4 at 2830, still outscored chesso, and its
record says the next read wants an anchor above about 2850. A 3000 reading
solved from anchors that all sit below 2850 is an extrapolation, and DEC-077
already measured scale compression across this set.

## Candidates

Direction only; ratings are re-read on the day. A family with documented
entries across the band is preferred (the engine-testing guide recommends
Stash as a gauntlet family: S088 already uses Stash v21.0). Other families
in the 2900 to 3150 band on the list read 2026-10-07: CuckooChess,
Gaviota, Crafty, DiscoCheck, Zurichess.

## Lane

Agent work; the smoke matches hold the workstation for minutes and run
between verdicts. Ported from the 2026-10-07 proposal's P04.

## As built

Two phases, because S119's SPRT held every core from 2026-10-12 23:13
(DEC-260). **Phase 1, written 2026-10-12/13, used no CPU**: the list read,
the choice, two `git clone`s, the drafted rows, records and commands.
**Phase 2, 2026-10-13, after S119's verdict**: the builds, the md5s, the
probes, the CCRL names and the candidate build. **The smoke run, 2026-10-13**,
launched by the coordinator from the command below because it holds the
machine for matches: passed.

### Deviations

1. **One smoke run carries both new anchors, not one run each.** `rating.sh`
   refuses a manifest of fewer than three engines, so each smoke needs
   companions; one run with Weiss 1.1, Stash v30.0 and Leorik 2.4 (the
   existing top rung, the required third) gives each new anchor its own
   pairing against chesso under `rating.sh`'s settings.
2. **`ROUNDS=100`, 200 games a pairing, not `--bracket`'s default 34.** At 68
   games a single forfeit is 1.5 % and voids under DEC-075's 1 %; at Stash
   v21.0's measured rate (0.45 % at concurrency 12) a 68-game smoke would void
   about one time in four on chance alone. At 200 games two forfeits pass and
   three void. About 27 minutes for 600 games.
3. **`DEV_MANUAL.md` corrections beyond the two new rows**, all in statements
   about the anchor set that this step had to re-read: the run's Hash is 128,
   not 64; "the binding maximum is Blunder at 256 MB" was the first set's
   Blunder 5.0.0 -- both Blunders in the set print `max 32000`; the rated
   run's cost line still said five hours at concurrency 6 (DEC-075 reversed
   that); and the `/usr/games` section placed the current references there,
   where only the first set lives. Each is listed so the coordinator can drop
   any it judges out of scope.
4. **The band read is 2840 to 3210**, a little wider than "about 2850 to
   3200": 466 entries, 442 strictly inside.
5. **Not changed, for the coordinator** (outside `touches:`): `rating.sh`'s
   comment "None of the reference engines exposes it" (`Threads`) is untrue
   for both new anchors, which default to one thread, so behaviour is
   unchanged; `tools/ccrl_rating.py`'s `LIST_URL` now answers a 302 to
   `https://computerchess.org.uk/404/rating_list_all.html`, which `urllib`
   follows, so the tool works -- phase 2 confirmed it; `DEV_MANUAL.md`'s "Last
   result: chesso ~= 2559" in the rating section is stale since S240.
6. **The shell-pipe probe changed form, because the drafted one did not
   establish its precondition.** Unpinned and with `isready`, Stash v21.0
   answered 5/5 on this idle machine, against S088's 5/5 hangs, so a 20/20
   from v30.0 in that form would have proved nothing. Pinned to one core
   (`taskset -c 3`) and without `isready`, v21.0 hangs 10/10; v30.0 answers
   20/20 in the same form. Every form tried is in the data record.
7. **Found, not planned: Weiss 1.1 segfaults on a `go` before any `isready`**
   (20/20; 0/20 with `isready` first) -- its hash table is allocated by
   `isready`. fastchess sends `isready` after every `ucinewgame` and waits for
   `readyok` (its source), and `rating.sh`'s identity probe sends no `go`, so a
   run should not reach it -- and the smoke run played Weiss 200 games under
   the installed fastchess 1.8.1 without a crash. Recorded in
   `references.tsv` and `DEV_MANUAL.md`.
8. **`adocs/data/README.md`'s S264 section sits after S088's**, not at the end
   of the file, because achesso's `d2b0fbe` appended there since this
   worktree's base `cead820`; the two hunks do not touch.
9. **Stash's md5 was measured to be non-reproducible, not assumed**: a second
   PGO build gave a different hash and the same bench node count.
10. **The committed smoke report has fastchess's standings tables removed.**
    fastchess prints a running table (Elo, nElo, score, pentanomial) into its
    console as it plays; 30 of them sat in the report above the bracketing
    cut. `adocs/data/S264_strip_standings.awk` drops each by its shape alone
    -- a dashed rule followed by a line beginning `Rank Name`, through the
    next dashed rule -- 30 blocks, 210 lines, no value read. Afterwards the
    file has no dashed rule, `Rank Name`, `Elo`, `nElo`, `Ptnml`, `LOS`,
    `LLR` or pentanomial row; all 600 started and 600 finished game lines,
    the trim counts, terminations and forfeits remain, and
    `sed '/^=== bracketing/,$d' report.txt | awk -f S264_strip_standings.awk`
    reproduces the committed file byte for byte. The awk avoids interval
    braces because this machine's awk is mawk 1.3.4, which ignored `-{10,}`
    and dropped nothing on the first pass.
11. **Throughput is sourced from S240's valid run 2, not run 1**: run 1 was
    void and shared the machine for its first hour. Run 2's fastchess total,
    2 h 28 m 31 s, gives 1349 games an hour. The smoke run's end stamp was
    never written (the coordinator's watcher replaced the planned one), so
    its end is `report.txt`'s mtime: 1663 s, 1299 games an hour.

### The list, read 2026-10-12

CCRL Blitz, `rating_list_all.html`, fetched 23:57 CEST with `curl -L`, "Computed
on October 10, 2026", md5 `8143a9ccb098a61bc8333544cd4c7e8d`, 3005 entries of
which 2625 are 1CPU. Every 1CPU entry from 2840 to 3210 -- rating, interval,
games, list name -- is `adocs/data/S264_ccrl_blitz_band_2026-10-12.tsv`, with
the extraction command in `adocs/data/S264_anchors.md`. Incumbents on the same
read: 2389, 2568, 2663, 2714, 2830.

### Chosen

| anchor | CCRL Blitz 2026-10-12 | family | licence | source, tag, commit | build |
|---|---|---|---|---|---|
| **Weiss 1.1** | 2987 +/-18, 893 games | new, the fourth: Terje Kirstihagen, C | GPL-3.0 | github.com/TerjeKir/weiss, `v1.1`, `5c3bd887` | `make pext`, gcc, no PGO |
| **Stash v30.0** | 3153 +/-17, 953 games | Stash, already in the set: Morgan Houppin, C | GPL-3.0 | gitlab.com/mhouppin/stash-bot, `v30.0`, `2c588cdb` | `utils/unix_build.sh` at `ARCH=x86-64-bmi2`, two-pass PGO, as S088 |

Weiss 1.1: a family with five entries across the band (2847 to 3136), so
later rungs share its build path; a fourth author votes independently of all
three incumbents (DEC-072's argument); C with gcc and make only, a
hand-written evaluation, no network file; and its rung sits at the mark, where
S152's reading is interpolated instead of extrapolated. Stash v30.0: the
family the step names, six entries from 2933 to 3208; it tops the set at the
step's 3150; and as Stash 21.0's own line 439 list-Elo above it, the two Stash
rungs read the scale inside one author's engines, which is DEC-077's
compression question without a family difference in it.

The set becomes 2389, 2568, 2663, 2714, 2830, 2987, 3153: seven rungs, four
families, 764 Elo.

Found by reading the sources at their tags (exact places in the data record):
**Stash v30.0 closes v21.0's startup deadlock** -- its engine mode starts at
`THINKING` and `main()` waits under the mutex for the engine thread to park
before it reads a command; **Weiss 1.1's `NoobBook` opens a socket to an
online book only when set true**, and it defaults to false; **both expose
`Threads`, default 1**, which `rating.sh` never sends. Neither needs a network
file. Clones at `/home/max/ws/engines/weiss-1.1` and
`/home/max/ws/engines/stash-30.0`; in phase 1 nothing executed, nothing
downloaded but the two source trees and the list. Phase 2 measured each of
those readings; the results are below.

### Rejected

The full table with reasons is in `adocs/data/S264_anchors.md`. In short:
other Stash and Weiss rungs (one more of each is enough; **Stash 29.0 at 3128
and Weiss 1.2 at 3056 are the fallbacks** if the smoke run fails either); Leorik 2.5
and 3.0 (a third Leorik rung adds no vote; 3.0 is above the band); Blunder (no
entry above 2663); CuckooChess 1.13 (Java, no JVM here: a dependency);
Gaviota 1.0 (no engine source found, 2869 adds little); Crafty 25.x (xboard
protocol only, fastchess drives UCI: an adapter is a dependency); DiscoCheck
5.2.1 (one entry, repository gone); Zurichess Neuchatel (one entry, Go like
Blunder, repository gone); Zahak 7.1 (two list rows share the name once
`ccrl_rating.py` strips " 64-bit"); Texel 1.07 (3130 +/-11, the best
alternate top rung, a fifth family if one is wanted); Go and Rust families
(toolchains present, but Go is Blunder's language and a cargo build downloads
crates); JavaScript, Java, Zig, D and Visual Basic engines (no toolchain);
closed-source entries (nothing to build); Ethereal (lowest entry 3333).

**No toolchain, dependency or licence question for the owner.** Both build
with gcc 13 and make, already here. Both are GPL-3.0; running a binary as a
measurement instrument is a tool use (DEC-016) and nothing of theirs enters
the tree but name, version, md5 and licence.

### The manifest rows

In `references.tsv`, with a header paragraph after Stash 21.0's (the build of
each, Weiss's `isready` crash, the deadlock measurement, `Threads`):

    Weiss 1.1	/home/max/ws/engines/weiss-1.1.bin	Weiss 1.1	/home/max/ws/engines/weiss-1.1	v1.1	a7868b74724741e357ae6c3ea23f3ecf
    Stash v30.0	/home/max/ws/engines/stash-30.0.bin	Stash 30.0	/home/max/ws/engines/stash-30.0	v30.0	36cc28332b5bad10c05589d474c165cb

Nothing in the manifest is pending.

### Phase 2 results, steps 1 to 8 (2026-10-13)

Load 0.35 before the first build; both clones clean at their recorded
commits and each tag resolving to it. Logs under the worktree's
`.tuning/coord/S264/`.

| step | result |
|---|---|
| 1. Weiss 1.1 | `make pext` exit 0, 3 `-Wsign-compare` warnings in the bundled Syzygy prober; md5 `a7868b74724741e357ae6c3ea23f3ecf`, **reproduced** by a second build: a rebuild check on this CPU and compiler only (`-march=native`) |
| 2. Stash v30.0 | `unix_build.sh` exit 0, `-Werror` held through both PGO passes, profile bench 13720671 nodes; md5 `36cc28332b5bad10c05589d474c165cb`; a second build gave `f27240eb956d0f09f8e833342a9183ef` with the same bench nodes: **identity only**, as v21.0 |
| 3. identity | all seven rows answer their `uci_name` (`Weiss 1.1`, `Stash v30.0` among them); the five incumbents' md5s equal the manifest's |
| 4. options | `Threads` only on the two new anchors, default 1; Hash maxima Blunder 32000, Leorik 2047 (binding), Stash v21.0 131072, Weiss 65536, Stash v30.0 33554432; `NoobBook` default false |
| 5. `ldd`, `strace` | libc, libm, loader, vDSO only; a timed search at `Hash 128` opened only libc, libm and locale files, made no `socket` or `connect` call, and returned a `bestmove`, for both |
| 6. shell-pipe probe | pinned to one core, no `isready`: **Stash v21.0 hung 10/10, Stash v30.0 answered 20/20**; Weiss 1.1 segfaulted 20/20 in that form and answered 20/20 with `isready` (deviations 6 and 7) |
| 7. CCRL names | `tools/ccrl_rating.py`, live list, exit 0: 2389, 2568, 2663, 2714, 2830, **2987**, **3153**, as read 2026-10-12; `urllib` follows the 302 |
| 8. candidate | worktree Release build at `cead820`, `bench` 3872032 |
| 9. smoke run | `RATING-RUN-DONE bracket OK`; 600 games, 516 adjudicated and 84 normal; 0 unexpected terminations, 0 crash, disconnect, stall or illegal-move log lines; **0 forfeits** for every engine (Weiss 1.1 and Stash v30.0 0 of 200 each); 6738 Leorik 2.4 "PV continues" blocks trimmed, report 0.1 MB; 1663 s from the start stamp to `report.txt`'s mtime (fastchess's total 27 m 42 s agrees), **1299 games an hour** against S240 run 2's 1349 |

`tools/plan_prose_check.py` in the worktree: exit 0, this file and S152's 0
flagged.

### The commands, as run (1 to 8) and for the coordinator (9)

Steps 1 to 8 as run on 2026-10-13 (step 6 written as one function; the runs
inlined the same strings). Step 6 is the form that established its
precondition (deviation 6); the drafted form and the others tried are in
`pipe_stash21.txt` and `pipe_stash21_forms.txt` beside the logs.

```bash
S=/home/max/ws/chesso-s264/.tuning/coord/S264; E=/home/max/ws/engines
cat /proc/loadavg                                   # 0.35
git -C $E/weiss-1.1 rev-parse 'v1.1^{commit}' HEAD   # both 5c3bd887..., clean
git -C $E/stash-30.0 rev-parse 'v30.0^{commit}' HEAD # both 2c588cdb..., clean

# 1. Weiss 1.1: build, install, rebuild to a second name, compare
cd $E/weiss-1.1/src && make pext > $S/build_weiss_1.log 2>&1 && cp weiss $E/weiss-1.1.bin
make pext EXE=weiss-rebuild > $S/build_weiss_2.log 2>&1
md5sum $E/weiss-1.1.bin weiss-rebuild | tee $S/md5_weiss.txt   # equal: a7868b74...

# 2. Stash v30.0: PGO as S088 built v21.0, then a second build to test the md5
cd $E/stash-30.0 && ARCH=x86-64-bmi2 sh utils/unix_build.sh > $S/build_stash.log 2>&1 \
  && cp src/stash-bot $E/stash-30.0.bin && md5sum $E/stash-30.0.bin | tee $S/md5_stash.txt
ARCH=x86-64-bmi2 sh utils/unix_build.sh > $S/build_stash_2.log 2>&1
md5sum src/stash-bot $E/stash-30.0.bin | tee $S/md5_stash_rebuild.txt   # differ

# 3. identity, as rating.sh's identify() asks it, all seven rows, with md5s
cd /home/max/ws/chesso-s264
grep -v '^#' references.tsv | while IFS=$'\t' read -r u b c s t m; do
  got=$(printf 'uci\nquit\n' | timeout -k 1 5 "$b" 2>/dev/null | sed -n 's/^id name //p' | head -1 | tr -d '\r')
  h=$(md5sum < "$b" | cut -d' ' -f1)
  echo "$u | $got | $([ "$got" = "$u" ] && echo OK || echo MISMATCH) | $h / $m"
done | tee $S/identity.txt                          # 7 OK

# 4. every engine's options
grep -v '^#' references.tsv | cut -f2 | while read -r b; do
  echo "== $b"; printf 'uci\nquit\n' | timeout -k 1 5 "$b" 2>/dev/null | grep '^option'
done > $S/options.txt

# 5. libraries, files and network during a timed search
ldd $E/weiss-1.1.bin $E/stash-30.0.bin > $S/ldd.txt
for x in weiss-1.1 stash-30.0; do
  strace -f -e trace=openat,socket,connect -o $S/strace_$x.txt bash -c "{ printf 'uci\nsetoption name Hash value 128\nisready\nucinewgame\nposition startpos\ngo wtime 10000 btime 10000 winc 200 binc 200\n'; sleep 3; printf 'quit\n'; } | timeout -k 1 10 $E/$x.bin" > $S/go_$x.txt
  echo "$x socket/connect $(grep -cE 'socket\(|connect\(' $S/strace_$x.txt) bestmove $(grep -c '^bestmove' $S/go_$x.txt)"
done                                                # 0 and 1, each

# 6. the shell-pipe probe, pinned to one core so the engine thread cannot start
#    before the `go` is read; a count of 0 bestmove lines is a hang or a crash
probe() { for i in $(seq "$2"); do { printf "$3"; sleep 2; printf 'quit\n'; } \
  | timeout -k 1 10 taskset -c 3 "$1" 2>/dev/null | grep -c '^bestmove'; done | sort | uniq -c; }
nois='uci\nposition startpos\ngo depth 10\n'; withis='uci\nisready\nposition startpos\ngo depth 10\n'
probe $E/stash-21.0.bin 10 "$nois"     # "10 0": hangs 10/10, the precondition
probe $E/stash-30.0.bin 20 "$nois"     # "20 1"
probe $E/stash-30.0.bin 20 "$withis"   # "20 1"
probe $E/weiss-1.1.bin 20 "$nois"      # "20 0": segfaults, exit 139
probe $E/weiss-1.1.bin 20 "$withis"    # "20 1"
# the exit code of the Weiss failure, three re-checks (these ran with `sleep 1`)
for i in 1 2 3; do { printf "$nois"; sleep 1; printf 'quit\n'; } \
  | timeout -k 1 10 taskset -c 3 $E/weiss-1.1.bin > /dev/null 2>&1; echo -n "${PIPESTATUS[1]} "; done   # 139 139 139

# 7. the seven CCRL names resolve on the live list (through the 302)
tools/ccrl_rating.py "Blunder 7.1.0" "Leorik 2.1" "Blunder 8.5.5" "Stash 21.0" "Leorik 2.4" \
  "Weiss 1.1" "Stash 30.0" | tee $S/ccrl_names.tsv  # exit 0, seven rows

# 8. the candidate rating.sh plays: this worktree's own Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > $S/cmake.log 2>&1 \
  && cmake --build build -j12 --target chesso >> $S/cmake.log 2>&1
build/src/chesso bench 2>&1 | tail -1               # 3872032 nodes
```

**Step 9, the smoke run: launched by the coordinator, 2026-10-13, exactly as
written here.** `rating.sh`'s own
settings (10+0.2, Hash 128, every core, `8moves_v3.pgn`, two-sided
adjudication), its own identity check before the first game, both new anchors
and Leorik 2.4 as the third rung it requires, 100 rounds: **600 games, 200 per
pairing, about 27 minutes** at S240 run 2's 1349 games an hour. The candidate is the
worktree's `build/src/chesso` (`cead820`, `bench` 3872032); the smoke reads no
strength, so the candidate's version does not matter -- the main tree's
`rating.sh` with the same `MANIFEST` and `OUT` plays the main tree's build
instead, if the coordinator prefers it.

```bash
S=/home/max/ws/chesso-s264/.tuning/coord/S264
cd /home/max/ws/chesso-s264
grep -v '^#' references.tsv | grep -E '^(Leorik 2\.4|Weiss 1\.1|Stash v30\.0)	' > $S/smoke_manifest.tsv
wc -l < $S/smoke_manifest.tsv                       # 3
date +%s > $S/smoke_start
nohup env MANIFEST=$S/smoke_manifest.tsv OUT=$S/smoke ROUNDS=100 ./rating.sh --bracket > $S/smoke.log 2>&1 &
echo $! > $S/smoke.pid                              # rating.sh's own pid ($!, never pgrep)

# the watcher, armed through Monitor with persistent: true; four exits:
# marker (DONE covers OK and INVALID), FAILED marker, process death, 60 min ceiling (2x)
bash -c 'end=$(($(date +%s)+3600))
  until grep -qE "RATING-RUN-(DONE|FAILED)" "$1"; do
    kill -0 "$2" 2>/dev/null || exit 3
    [ "$(date +%s)" -ge "$end" ] && exit 124
    sleep 30
  done' _ $S/smoke.log "$(cat $S/smoke.pid)"
date +%s > $S/smoke_end    # not written in the run: its end is report.txt's mtime

# the checks
tail -1 $S/smoke.log                                 # RATING-RUN-DONE bracket OK <outdir>
grep -E 'terminations outside|log lines matching' $S/smoke/report.txt   # both 0
cat $S/smoke/forfeits.txt                            # Weiss 1.1, Stash v30.0: each <= 2 of 200
grep -ciE 'illegal|segmentation|crash' $S/smoke/fastchess.log   # 0
grep -c '^\[Termination' $S/smoke/games.pgn          # 600
sed -n '/^=== fastchess console lines omitted/,/^=== terminations/p' $S/smoke/report.txt   # trim counts: 6738 blocks
echo "games/h $(( 600 * 3600 / ($(stat -c %Y $S/smoke/report.txt) - $(cat $S/smoke_start)) ))"  # 1299 against 1349
```

Not read and not quoted (DEC-108, accepts (5)): `h2h.txt`, `ordo_bracket.txt`
and the bracketing block at the end of the report. `adocs/data/` takes the
report from its first line to `=== bracketing`, exclusive
(`sed '/^=== bracketing/,$d'`), stripped of fastchess's standings tables by
`adocs/data/S264_strip_standings.awk` (deviation 10), as
`S264_smoke_report.txt`, with `S264_smoke_forfeits.txt`, and rows in the data
README's S264 section;
the run directory stays under `.tuning/` -- a smoke is not a run worth
archiving unless the coordinator says otherwise. A crash, an illegal move or
a forfeit rate over 1 % on either new anchor fails the step's (3): the
fallback is Stash 29.0 or Weiss 1.2, through the same commands.

The reading: `RATING-RUN-DONE bracket OK`; 600 `[Termination` lines, 516
adjudication and 84 normal; 0 forfeits for every engine; 0
`illegal|segmentation|crash|disconnect` lines in `fastchess.log`; both of
`rating.sh`'s own counters 0; the trim section counts 6738 Leorik 2.4 blocks,
26952 lines. 1663 s from the start stamp (08:43:59) to `report.txt`'s mtime
(09:11:41.95), fastchess's own total 27 m 42 s: 1299 games an hour.
`adocs/data/S264_smoke_report.txt` (77961 bytes) is the report cut before
`=== bracketing` and stripped of fastchess's standings tables (deviation 10);
`S264_smoke_forfeits.txt` is copied whole. Nothing from the bracketing block,
`h2h.txt` or `ordo_bracket.txt` was read.
The suite and format check run per TESTS on the landed tree.

### S152's cost

Written into S152's file. `rating.sh` plays 334 rounds, 668 games, per
pairing; `hours = 668 x P / G`. S240's valid run 2 measured G = 1349 games an
hour at 10+0.2 (3340 games in 2 h 28 m 31 s, fastchess's total, concurrency
12, hash 128; run 1 was void), so 29 m 42 s a pairing and, at seven pairings,
**4676 games in about 3 h 28 m** against 2 h 29 m for five; the smoke run's
1299 gives 3 h 36 m. The second control is unmeasured: throughput scales about
with the control's length (self-play 2110 games an hour at 8+0.08, 534 to 545
at 32+0.32, a factor 3.87 to 3.95), so the pair costs about `3.47 x (1 + k)`
hours for a control `k` times 10+0.2 -- 10.4 h at `k = 2`, 18.6 h at 60+0.6,
31.8 h at 120+1.

### Proposed `done:` stamp

    done:       <date> -- the reference set brackets about 2750 to 3150: Weiss 1.1 (CCRL Blitz 2987 +/-18, 893 games) and Stash v30.0 (3153 +/-17, 953 games), read 2026-10-12 from the list computed 2026-10-10, join S088's five as the sixth and seventh rungs and the fourth family; seven rungs 2389 to 3153. Both built here from tagged GPL-3 source under /home/max/ws/engines (DEC-069): Weiss 1.1 `make pext`, md5 a7868b74724741e357ae6c3ea23f3ecf, reproduced by a second build (a rebuild check on this CPU and compiler only, `-march=native`); Stash v30.0 PGO as S088, md5 36cc28332b5bad10c05589d474c165cb, identity only (a second build differs, bench nodes equal). `uci` answers `Weiss 1.1` and `Stash v30.0`; no network file, no socket under strace; the five incumbents' md5s unchanged. Stash v30.0's startup deadlock closed: pinned to one core with the commands in the pipe, v21.0 hung 10/10 and v30.0 answered 20/20. Weiss 1.1 segfaults on a `go` before any `isready` (20/20), which fastchess does not send; recorded. tools/ccrl_rating.py resolves all seven names through the list's new 302. Smoke run under rating.sh's settings, 600 games against Leorik 2.4 and the two new anchors, 200 each: RATING-RUN-DONE bracket OK, 0 crashes, disconnects or illegal moves, 0 unexpected terminations, 0 forfeits for every engine (inside DEC-075's 1 %), report trimmed by trim_console.py and committed cut before its bracketing block and stripped of fastchess's 30 standings tables by shape; no rating read or committed (DEC-108); 1299 games an hour against S240 run 2's 1349. S152's cost line written from S240 run 2's 1349 games an hour: 4676 games, about 3 h 28 m at 10+0.2 (3 h 36 m at the smoke's 1299), and `3.47 x (1 + k)` hours for both controls. DEV_MANUAL's rating section updated (the set, hash 128, the Hash maxima with Leorik's 2047 binding, Weiss's isready crash, where the references live). Fallbacks if needed: Stash 29.0, Weiss 1.2. No src/ change.

### Commit text

    Add Weiss 1.1 and Stash v30.0 as rating anchors above 2850 (S264)

    S240's top anchor, Leorik 2.4 at 2830, still outscored chesso, so S152's
    reading near 3000 would be solved past the top of the reference set, and
    DEC-077 measured compression across that set. Weiss 1.1 (CCRL Blitz 2987,
    list read 2026-10-12) is a fourth family with a rung at the mark; Stash
    v30.0 (3153) tops the set and is Stash 21.0's own line, so the two Stash
    rungs read the scale inside one author's engines.

    Both are built here from tagged GPL-3 source under /home/max/ws/engines
    (DEC-069); only name, version, md5 and licence enter the tree (DEC-016).
    Weiss 1.1's md5 reproduces across builds; Stash v30.0's is PGO and an
    identity record. Stash v30.0 closes v21.0's startup deadlock: pinned to
    one core with the commands in the pipe, v21.0 hung 10/10 and v30.0
    answered 20/20. Weiss 1.1 segfaults on a go before any isready, which
    fastchess does not send; recorded in the manifest and DEV_MANUAL.

    The smoke run under rating.sh's settings played 600 games, 200 per new
    anchor: 0 crashes, 0 illegal moves, 0 time forfeits. No rating is read
    from it (DEC-108). S152's cost is recomputed for seven
    pairings: 4676 games, about 3 h 28 m at 10+0.2.

No `src/` change, so no `Bench:` line; the session's attribution trailers
follow.
