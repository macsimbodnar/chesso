id:         S087
goal:       an absolute rating for chesso on the CCRL Blitz scale, with an interval and a measured anchor sensitivity
accepts:    a gauntlet of chesso against at least three reference engines carrying CCRL Blitz ratings read from the list at run time and never hardcoded; a bracketing pre-run establishes that chesso scores below 90 % against the strongest reference and above 10 % against the weakest, and the set is widened before the rated run is booked if it does not; the rated run returns a 95 % interval of +/- 30 Elo or tighter on chesso's solved rating; the PGN is checked for time forfeits and the count is reported, a single forfeit invalidating the run; the rating is re-solved anchoring each reference engine in turn and the full spread across anchors is reported, a spread above 30 Elo reported as soft rather than hidden; one script re-runs gauntlet and solve end to end; a tracked manifest names each reference engine's source, commit or tag, build command, binary sha256, and the CCRL rating with the date it was read; no third-party source or binary is added to this repository
touches:    a new run script beside fastchess.sh, a tracked reference manifest, adocs/data/ for the PGN and the results file, DEV_MANUAL.md
excludes:   replacing fastchess.sh as the per-change SPRT gate; any edit under src/ or tests/; installing anything into /usr/games; installing the .NET SDK, which Leorik alone would need; assessing any position, move or game from the resulting PGN
decisions:  DEC-067
closes:
blocks:
paused_by:
done:

## Why this exists

Every strength figure this project has recorded is a delta against an earlier
chesso. `fastchess.sh:123-124` plays the working tree against a `.ref-builds/`
worktree of a reference sha and runs an SPRT on it, which answers "is B stronger
than A". Nothing in the tree answers "how strong". The stated goal is the
strongest open-source engine in the world and there is no instrument that
measures the distance to it.

Self-play cannot supply one at any game count. It measures progress inside one
version lineage and is inflated relative to a public scale.

This step is an instrument, not a strength change. It buys zero Elo. The plan
has put instruments first before, and for the same reason — S035 repaired the
SPRT harness and S037 the reporting, and `plan.md` records that "the rest of
this plan is measured with them". This one is the first that can be read against
somebody else's engine.

## What is already on this machine

Checked at `3e80a33`, not assumed:

| thing | state |
|---|---|
| `ordo` | **installed**, `1.2.6`, `/usr/local/bin/ordo` |
| `fastchess` | `alpha 1.8.1 20260720-daa3ea2`, supports `-tournament gauntlet` and `-seeds N` |
| `cargo` | `1.95.0` — Rustic builds |
| `go` | `go1.22.2 linux/amd64` — Blunder builds |
| `g++` | `13.3`, the DEC-049 reference compiler |
| `dotnet` | **absent** — Leorik does not build here without a new dependency |
| Rustic source | already cloned at `/home/max/ws/rustic`, tags `alpha-1` to `alpha-3.0.6` |
| book | `books/8moves_v3.pgn`, 34700 openings |

So the tooling cost of this step is close to zero. What it costs is machine
time and the reference builds.

## Four corrections to the source specification

The specification document is sound on method. These four of its specifics are
wrong for this repository or this machine, and the step is written against the
corrected versions.

1. **`run_test.sh` does not exist here.** The script whose conventions are to be
   followed is `fastchess.sh`.
2. **`https://github.com/nescitus/sungorus` returns 404.** Sungorus is not at
   the URL given and was not located before this step was written; the GitHub
   search API rate-limited during the attempt. Resolving it is execution work,
   and the reference set below does not depend on it.
3. **`dotnet` is absent, so Leorik is out of the starting set.** `CLAUDE.md`
   forbids the agent adding a dependency on its own. Leorik stays available as
   the upward expansion if bracketing fails high — as an owner decision, not a
   drift.
4. **`https://computerchess.org.uk/ccrl/404/` 302s** to
   `https://computerchess.org.uk/404/`. The fetch follows redirects.

## The licensing boundary, DEC-067

The specification's deliverable 2 asks for a `references/` directory in this
repository holding the reference engine binaries. **It is not built that way
here.** Rustic is GPL-3, and a GPL binary committed into this tree is exactly
the question `CLAUDE.md`'s first foundation exists to keep out of this codebase
and out of any future network.

Instead: sources are cloned and built under `/home/max/ws/engines/<name>/`,
outside the repository. What is tracked is a manifest with the same provenance
content and no licensing question — source URL, tag or commit, build command,
binary `sha256`, CCRL rating, and the date that rating was read.

Running another engine's binary as a tool creates no derivative work and is
encouraged. DEC-016. Compiling it outside the tree and recording what was
compiled is the same act.

## Concurrency: 12 stands, and the forfeit check is why

The specification asks for concurrency at physical cores minus one. The house
rule is every core the machine reports — DEC-048, DEC-050, `fastchess.sh:45-46`,
12 here.

The conflict is real and is not only about variance. DEC-050's justification is
that oversubscription hits both sides about equally, so it inflates variance
rather than biasing the result. That holds for chesso against chesso. Against a
foreign engine whose time management differs it does not obviously hold: the
side with the thinner safety margin forfeits more often, and a forfeit is a
whole point rather than noise.

**Settled at 12 by the owner, for throughput, with detection instead of
avoidance.** The run script counts time forfeits in the PGN and reports the
count; one forfeit invalidates the run. If the bracketing run shows any, the
rated run drops to `CONCURRENCY=6` and DEC-067 is superseded rather than
reinterpreted.

## Run shape: bracket cheap, then decide

The bracketing check is the specification's own idea and it is what makes the
expensive run safe to book. Two runs:

**Run one, bracketing.** `10+0.2` — the repository's own time control — about
200 games total, roughly 34 rounds per pairing at `-games 2 -repeat`. It answers
two questions and no others: does the reference set bracket chesso between 10 %
and 90 %, and does anything forfeit on time. Roughly 13 minutes at concurrency
12. Its rating output is not quoted anywhere.

**Run two, rated.** About 1000 games for the +/- 30 Elo the accepts requires.
Its time control is chosen *after* run one, with a real score in hand, and is
the owner's call:

| TC | ~1000 games at concurrency 12 |
|---|---|
| `10+0.2` | ~1 h. Same scale as every chesso measurement on record. Result labelled approximate against CCRL Blitz |
| `2+1` | ~8 h. Matches CCRL Blitz list 404, so the anchor import is honest. A night of the binding constraint |

Both estimates are before adjudication, which cuts them.

The reason for not simply booking `2+1` first: a bracketing failure spends that
night producing a number the specification itself calls unreliable, because the
score sits in the tail of the logistic curve where the estimate is dominated by
it.

## The reference set

The specification's starting set is Rustic Alpha 2, Sungorus 1.4 and Blunder
4.0. With Sungorus unlocated and Leorik unbuildable here, the set is built from
**two repositories that are both reachable and both have their toolchain
present**, plus Sungorus if it is found:

| engine | source | tag | language | notes |
|---|---|---|---|---|
| Rustic Alpha 1 | `https://codeberg.org/mvanthoor/rustic.git` | `alpha-1` | Rust | low anchor; already cloned |
| Rustic Alpha 2 | same | `alpha-2` | Rust | |
| Blunder 3.0.0 | `https://github.com/algerbrex/blunder` | `v3.0.0` | Go | tag confirmed present |
| Blunder 4.0.0 | same | `v4.0.0` | Go | tag confirmed present |
| Sungorus 1.4 | **unresolved** | — | C++ | include if located |

Rustic's home is Codeberg, not the GitHub URL the specification gives; the
GitHub path resolves but the local clone at `/home/max/ws/rustic` already points
at Codeberg.

**Residual risk to name rather than hide:** a set drawn from two engine families
is narrower than one drawn from four. If chesso has some systematic property
that both Rustic and Blunder share a weakness to, the estimate skews and no
game count detects it. Two distinct authors, two distinct languages and two
distinct evaluation designs is the mitigation; the anchor-sensitivity sweep in
the accepts is what would show the references disagreeing internally. The
figure gets one more reference family before it is quoted anywhere outside this
repository.

Which CCRL entry each build corresponds to is a manifest field, and the name in
the manifest must be the name on the list — a tag built is not automatically the
version rated.

## Builds are supervised

The owner runs the builds; the agent writes the commands, verifies the result
and writes the manifest. Nothing here needs `sudo` and nothing is installed.

```bash
mkdir -p /home/max/ws/engines

# Rustic (GPL-3) -- two tags from one clone, built into separate binaries
git clone https://codeberg.org/mvanthoor/rustic.git /home/max/ws/engines/rustic
cd /home/max/ws/engines/rustic
git checkout alpha-1 && cargo build --release && cp target/release/rustic ../rustic-alpha-1
git checkout alpha-2 && cargo build --release && cp target/release/rustic ../rustic-alpha-2

# Blunder (MIT)
git clone https://github.com/algerbrex/blunder /home/max/ws/engines/blunder
cd /home/max/ws/engines/blunder
git checkout v3.0.0 && go build -o ../blunder-3.0.0 ./...
git checkout v4.0.0 && go build -o ../blunder-4.0.0 ./...
```

The binary paths and the `go build` package path are what the specification
calls "compile from source where practical" — release downloads may be built
for another microarchitecture, which on a timed match is a strength difference
nobody asked for.

Agent verification per binary, before it enters the manifest: it answers `uci`
with `uciok`, it accepts `setoption name Hash value 16` and `setoption name
Threads value 1` or documents that it does not, and it plays one game against
itself without a forfeit.

## Solving with ordo

`ordo 1.2.6`, switches confirmed against `ordo --help` on this machine:

```
ordo -p <pgn> -o <out> -a <rating> -A "<engine name>" -s 1000 -F 95 -n 12 -W -D -j <h2h>
```

`-a` with `-A` fixes one player's rating and floats the rest, `-s` runs the
simulations the interval comes from, `-F 95` is the confidence, `-W` and `-D`
auto-adjust the white advantage and the draw rate, `-j` writes the head-to-head
table the per-opponent scores come from.

The anchor sweep is that same command run once per reference engine, each time
anchoring a different one at its CCRL rating, and the reported figure is
chesso's solved rating from each. The spread across those solves is the
sensitivity number the accepts asks for.

`ordo` intervals are trinomial. Every SPRT verdict in this repository runs
`model=normalized` and carries an nElo figure. **The two are never quoted
against each other**, and the results file says so where the interval is
printed.

## Deliverables

1. `rating.sh` at the repository root, beside `fastchess.sh` and following it:
   snapshots the candidate binary before a single game is played (the mid-run
   rebuild that `fastchess.sh:71-84` documents applies identically here), warns
   on a busy machine, prints what it is measuring and what it is measuring
   against, takes `CONCURRENCY` and a mode flag, and ends by counting forfeits
   and running the anchor sweep.
2. A tracked manifest of the reference engines, as above. The binaries it
   describes are not in this repository.
3. A results file under `adocs/data/`: raw score per opponent, total games,
   forfeit count, chesso's solved rating under each anchor choice, the spread
   across anchors, and the interval — with its time control and its trinomial
   provenance stated beside it.

## Cost

Bracketing run ~13 min. Rated run ~1 h at `10+0.2` or ~8 h at `2+1`, the choice
made after the bracketing run. Builds are minutes. Measurement capacity is the
binding constraint on the whole plan (`specs.md`), and this step spends it on an
instrument rather than on a strength change — deliberately, and the number it
produces is re-derivable from one script after every later milestone.
