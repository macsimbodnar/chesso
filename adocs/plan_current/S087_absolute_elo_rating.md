id:         S087
goal:       an absolute rating for chesso on the CCRL Blitz scale, with an interval and a measured anchor sensitivity
accepts:    a gauntlet of chesso against at least three reference engines carrying CCRL Blitz ratings read from the list at run time and never hardcoded; a bracketing pre-run establishes that chesso scores below 90 % against the strongest reference and above 10 % against the weakest, and the set is widened before the rated run is booked if it does not; the rated run returns a 95 % interval of +/- 30 Elo or tighter on chesso's solved rating; the PGN is checked for time forfeits and the count is reported, a single forfeit invalidating the run; the rating is re-solved anchoring each reference engine in turn and the full spread across anchors is reported, a spread above 30 Elo reported as soft rather than hidden; one script re-runs gauntlet and solve end to end; a tracked manifest names each reference engine and the exact version installed in /usr/games; no third-party source or binary is added to this repository
touches:    a new run script beside fastchess.sh, a tracked reference manifest, adocs/data/ for the PGN and the results file, DEV_MANUAL.md
excludes:   replacing fastchess.sh as the per-change SPRT gate; any edit under src/ or tests/; compiling or installing the reference engines, which the owner does; installing the .NET SDK, which Leorik alone would need; assessing any position, move or game from the resulting PGN
decisions:  DEC-067, DEC-068
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

Checked at `ebb469f`, not assumed:

| thing | state |
|---|---|
| `ordo` | **installed**, `1.2.6`, `/usr/local/bin/ordo` |
| `fastchess` | `alpha 1.8.1 20260720-daa3ea2`, supports `-tournament gauntlet` and `-seeds N` |
| reference engines | **built and installed by the owner**, see the set below |
| reference sources | `/home/max/ws/Leorik` at `1.0`, `/home/max/ws/rustic` at `alpha-3.0.6`, `/home/max/ws/blunder` at `v5.0.0`, each a detached checkout of the tag |
| book | `books/8moves_v3.pgn`, 34700 openings |
| `g++` | `13.3`, the DEC-049 reference compiler |

So the tooling cost of this step is zero. What it costs is machine time.

## Three corrections to the source specification

The specification document is sound on method. These of its specifics are wrong
for this repository, and the step is written against the corrected versions.

1. **`run_test.sh` does not exist here.** The script whose conventions are to be
   followed is `fastchess.sh`.
2. **`https://github.com/nescitus/sungorus` returns 404**, and Sungorus is not
   in the installed set. It is not needed: the three engines below span 310 Elo
   of CCRL Blitz between them.
3. **`https://computerchess.org.uk/ccrl/404/` 302s** to
   `https://computerchess.org.uk/404/`. The fetch follows redirects.

A fourth correction has itself been overtaken: `dotnet` was absent when this
step was written and Leorik was excluded for it. The owner has since built and
installed Leorik 1.0, so Leorik is in the set. Recorded because DEC-067's
`Rejected` still carries the old reason.

## Where the reference binaries live

**In `/usr/games/`, beside `stockfish` and `fastchess`.** The owner compiles the
reference engines and installs them there. Nothing third-party, source or
binary, enters this repository.

What is tracked here is a manifest naming each reference engine and the exact
version installed — that is what makes a result attributable to a specific
opponent build, and it is all the tracking the binaries need. The CCRL rating
used as each anchor and the date it was read are recorded per run in the results
file instead, because they change between runs and the installed binary does
not.

Running another engine's binary as a tool creates no derivative work and is
encouraged. DEC-016.

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

## The reference set, as installed

The owner built and installed three engines. Each was checked by asking the
binary itself what it is, not by trusting the filename — `printf 'uci\nquit\n' |
<binary>`:

| binary | `id name` says | source checkout | CCRL Blitz |
|---|---|---|---|
| `/usr/games/Leorik-1.0` | `Leorik 1.0` | `/home/max/ws/Leorik` at tag `1.0` | **2102** +20/−20 |
| `/usr/games/blunder` | `Blunder 5.0.0` | `/home/max/ws/blunder` at tag `v5.0.0` | **2017** +20/−20 |
| `/usr/games/rustic` | **`engine 3.99.36`** — wrong binary, see below | `/home/max/ws/rustic` at tag `alpha-3.0.6` | Alpha 3.0.0 is **1792** +16/−16 |

Ratings read from `https://computerchess.org.uk/ccrl/404/rating_list_all.html`
on **2026-08-17**. The run script re-reads them; these are here to show the set
brackets.

**The set spans 1792 to 2102, 310 Elo.** Against a 2000-rated engine that is a
usable bracket in both directions. Against a much stronger one it is not, and
the bracketing run is what decides which case this is. The cheap upward
expansions, both from clones already on disk, are Blunder 7.1.0 at 2389 and
Leorik 2.0.2 at 2538.

### Two things block the rated run

**1. `/usr/games/rustic` is not Rustic Alpha 3.0.6.** It reports `id name engine
3.99.36` and is `md5` identical to `/home/max/ws/rustic/target/release/rustic`,
which is the workspace's development binary. The tag build is a *different
file*: at `alpha-3.0.6` the package is named `rustic-alpha`, so `cargo build
--release` writes `target/release/rustic-alpha`, and that binary reports
`id name Rustic Alpha 3.0.6`. Both exist in `target/release/` and the wrong one
was installed. One command fixes it:

```bash
sudo install -m 0755 /home/max/ws/rustic/target/release/rustic-alpha /usr/games/rustic-alpha-3.0.6
```

A development build has no published rating at all, so anchoring on it would
attach 1792 to a binary CCRL has never played. This is the exact failure the
manifest exists to catch, and it was caught before a game was played rather than
after.

**2. CCRL lists Rustic Alpha 3.0.0, not 3.0.6.** The list carries
`Rustic Alpha 3.0.0 64-bit 1792`, `Rustic Alpha 2 1719` and
`Rustic Alpha 1 1549`, and no 3.0.x entry between them. Anchoring the installed
3.0.6 at 3.0.0's rating imports six patch releases of unmeasured difference into
the low anchor. Either build `alpha-3.0.0` so the anchor names the binary that
earned it, or keep 3.0.6 and record the low anchor as approximate. Owner's call;
it does not block the bracketing run, which needs score fractions and no anchors
at all.

### Bracketing run 1: FAILED, the set is too weak

Run on 2026-08-18, 204 games, `10+0.2`, `Hash=64`, concurrency 12, book
`8moves_v3.pgn`, 7 m 30 s. Evidence: `adocs/data/S087_bracket1.pgn`,
`adocs/data/S087_bracket1_h2h.txt`.

**Zero time forfeits.** 194 games ended `[Termination "adjudication"]`, 10
`normal`, nothing else, and no line in the fastchess log matched a time loss or
disconnect. So concurrency 12 against foreign engines produced no forfeit at
this time control, which is the risk DEC-067 accepted and chose to detect. It is
one run at one time control and does not license skipping the check.

chesso's score against each reference, from `ordo -j`:

| reference | CCRL Blitz | games | chesso score |
|---|---|---|---|
| Leorik 1.0 | 2102 | 68 | **90.4 %** |
| Rustic Alpha 3.0.6 | (3.0.0 = 1792) | 68 | 94.9 % |
| Blunder 5.0.0 | 2017 | 68 | 95.6 % |

Overall 93.6 % over 204 games, +185 =12 -7.

**The gate is "meaningfully below 90 % against the strongest reference". 90.4 %
is not below 90 %, so the set does not bracket chesso and the rated run is not
bookable against it.** Every score sits in the tail of the logistic curve where
the estimate is dominated by the curve rather than by the games, which is the
condition the specification calls unreliable.

**No rating is claimed from this run and none can be.** That is the point of
running it: it cost 7 m 30 s to learn that the ~2000 prior is wrong and that a
night at `2+1` against this set would have bought a number in the tail. The
prior was explicitly not to be assumed, and it was not.

The set has to shift up. Both families are already cloned, so the cost is a
checkout and a build per rung. From the same list read on 2026-08-17:

| candidate | CCRL Blitz | family |
|---|---|---|
| Blunder 7.1.0 | 2389 | Go |
| Blunder 7.4.0 | 2521 | Go |
| Leorik 2.0.2 | 2538 | C# |
| Leorik 2.1 | 2568 | C# |
| Blunder 8.0.0 | 2651 | Go |
| Blunder 8.5.5 | 2664 | Go |
| Leorik 2.2 | 2689 | C# |
| Leorik 2.4 | 2829 | C# |
| Leorik 2.5 | 2917 | C# |

Rustic is out of the set at any tag: its strongest rated build is Alpha 3.0.0 at
1792. That retires both of this step's open Rustic questions — the wrong
installed binary and the 3.0.6-versus-3.0.0 anchor gap — without either being
answered, since neither engine is in the set any more.

### Engine options

None of the three references exposes `Threads` — all are single-threaded by
construction, and Rustic prints `Threads: 1 (unused, always 1)`. chesso exposes
`Threads` with `min 1 max 1`. So **`-each option.Threads=1` is wrong here** and
the option is simply not sent; the single-thread condition holds by construction
on all four.

`Hash` exists on all four, and the binding maximum is Blunder's **256 MB**:
Leorik 2047, Rustic 65535, chesso 4096. `option.Hash=64` is inside every one of
them.

chesso also exposes `Use Book`, default `false`. It must stay false — openings
come from the book file, and an engine playing its own book is not playing the
position it was dealt.

**Residual risk to name rather than hide:** three engines from three authors and
three languages is a better spread than the two-family set this step originally
planned, but it is still three. If chesso has a systematic property all three
share a weakness to, the estimate skews and no game count detects it. The
anchor-sensitivity sweep in the accepts is what would show the references
disagreeing internally; it is not proof they are jointly right. The figure gets
a fourth family before it is quoted anywhere outside this repository.

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
2. A tracked manifest naming each reference engine and the exact version
   installed in `/usr/games`. The binaries themselves are not in this
   repository.
3. A results file under `adocs/data/`: raw score per opponent, total games,
   forfeit count, the CCRL rating used for each anchor with the date it was
   read, chesso's solved rating under each anchor choice, the spread across
   anchors, and the interval — with its time control and its trinomial
   provenance stated beside it.

## Cost

Bracketing run ~13 min. Rated run ~1 h at `10+0.2` or ~8 h at `2+1`, the choice
made after the bracketing run. The builds are already done. Measurement capacity is the
binding constraint on the whole plan (`specs.md`), and this step spends it on an
instrument rather than on a strength change — deliberately, and the number it
produces is re-derivable from one script after every later milestone.
author:    Maksym Bodnar
