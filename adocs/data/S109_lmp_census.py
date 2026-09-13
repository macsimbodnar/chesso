#!/usr/bin/env python3
"""S109. Where the late move pruning constants come from, over chesso's own tree.

DEC-105 and DEC-134: a threshold that originates as another engine's tuned
output is never a seed here, wherever it is republished. So `LmpBase` and
`LmpDepthCoeff` are derived rather than borrowed, by asking this engine's own
search a question it can answer about itself -- **how far down the move order
the quiet move that actually caused a beta cutoff sits** -- and setting the
count past which the answer is almost always already behind us.

    # the tree without the block, which is the tree the census is about
    cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
    cmake --build build-tune -j12
    ~/.venv/chess/bin/python adocs/data/S109_lmp_census.py run \\
        build-tune/src/chesso adocs/data/S109_lmp_census.tsv
    ~/.venv/chess/bin/python adocs/data/S109_lmp_census.py fit \\
        adocs/data/S109_lmp_census.tsv

`run` drives the tune build over the 300 positions at `go depth 10` with all
four of the block's caps set to 0 -- their exact off values (src/search_params.hpp
records why `<` and not `<=`) -- so what is measured is the search as it was
before this step, and the fit is not reading back its own effect. The engine
writes the census to the file named by `CHESSO_LMP_CENSUS` when it exits.

POSITIONS. The 300-position stratified pick of `adocs/data/S018_raw.tsv` that
`src/search_params.hpp` already quotes for the aspiration sweep: 4 positions per
`game_phase()` value that has at least 4 rows, at three offsets, so the
middlegame does not answer for the endgame. The pick is
`adocs/data/S021_aspiration_sweep.py`'s and is repeated here rather than
imported, because that file is a sweep driver that runs on import.

THE RULE THE FIT SETS, AND THE AXIS IT IS READ ON. Per **remaining depth** the
95th percentile of the cutoff index is taken: the count past which 19 of 20
quiet cutoffs at nodes that deep have already happened, which is what "late
enough to give up on the rest" means. The percentile is this step's choice; the
numbers are the census's.

The engine's own gate is the reduction-adjusted depth
`lmr_depth = max(0, depth - lmr_reduction(depth, move_number))`, so the fitted
line has to live on that axis -- and **the census may not be bucketed by it**.
`lmr_depth` is a decreasing function of the move number, so conditioning a
cutoff-index distribution on it conditions on the very quantity being measured:
read that way the first pass of this script returned a threshold that *fell*
with depth, `12.67 - 3.50 * lmr_depth`, which is the coupling and not the tree.
Recorded here because the wrong table is the plausible-looking one.

What is fitted instead is the pair the rule actually has to reproduce. For each
remaining depth `d` the census gives the count `m*(d)` the rule should first
fire at; the rule tests that move at `x(d) = lmr_depth(d, m*(d))`, read from the
reduction table the same binary dumped. `LmpBase` and `LmpDepthCoeff` are the
least-squares line through `(x(d), m*(d))` over the shallow depths, in
hundredths of a move -- shallow because that is where the rule spends almost all
of its firings, and a line fitted over the whole table is dragged by rows that
never bind.

`LmpMaxLmrDepth` is the largest `lmr_depth` at which the fitted threshold still
sits below the **median number of quiet moves** a node generates. Above that the
threshold is past the end of the move list and the rule cannot fire at all -- a
rule that never binds is not a rule, and setting a cap there would be pretending
to have measured something.
"""
import collections
import os
import subprocess
import sys

RAW = "adocs/data/S018_raw.tsv"
PER_PHASE = 4
OFFSETS = (0, 1, 2)
DEPTH = 10
PERCENTILE = 95
FIT_DEPTHS = (1, 2, 3)

# The four caps at their off values, so `run` measures the pre-S109 tree.
OFF = (
    "LmpMaxLmrDepth",
    "FutMaxLmrDepth",
    "HistPruneMaxLmrDepth",
    "SeeQuietMaxLmrDepth",
)


def positions():
    """S021's stratified pick, at three offsets."""
    by_phase = collections.defaultdict(list)
    with open(RAW) as handle:
        header = handle.readline().rstrip("\n").split("\t")
        phase_at, fen_at = header.index("phase"), header.index("fen")
        for line in handle:
            field = line.rstrip("\n").split("\t")
            by_phase[int(field[phase_at])].append(field[fen_at])

    picked = []
    for offset in OFFSETS:
        for phase in sorted(by_phase):
            rows = by_phase[phase]
            if len(rows) < PER_PHASE:
                continue
            step = len(rows) // PER_PHASE
            picked += [rows[(i * step + offset) % len(rows)]
                       for i in range(PER_PHASE)]
    return picked


def run(engine, out_path):
    """One process for the whole set, driven a command at a time.

    Never a bare pipe of every line at once. `quit` arriving while a search is
    running stops it: the same trap TOOLCHAIN.md records for the Stockfish
    oracle, and it was observed here first -- 300 positions answered in 0.4 s,
    every one of them at depth 1, and a census of zero cutoffs. Each `go` is
    waited out to its own `bestmove` before the next line is written.

    One process and not one per position because the engine dumps the census
    from its exit handler, so a process per position would overwrite the file
    300 times and leave the last one.
    """
    fens = positions()
    print(f"# {len(fens)} positions, depth {DEPTH}, engine {engine}",
          flush=True)

    environment = dict(os.environ)
    environment["CHESSO_LMP_CENSUS"] = os.path.abspath(out_path)

    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1,
                            env=environment)

    def send(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()

    def wait_for(prefix):
        while True:
            line = proc.stdout.readline()
            if not line:
                sys.exit("the engine died")
            if line.startswith(prefix):
                return line.strip()

    send("uci")
    wait_for("uciok")
    for name in OFF:
        send(f"setoption name {name} value 0")
    send("isready")
    wait_for("readyok")

    for index, fen in enumerate(fens):
        send("ucinewgame")
        send("position fen " + fen)
        send(f"go depth {DEPTH}")
        wait_for("bestmove")
        if (index + 1) % 50 == 0:
            print(f"# {index + 1} of {len(fens)}", flush=True)

    send("quit")
    if proc.wait() != 0:
        sys.exit(f"engine exited {proc.returncode}")
    print(f"# wrote {out_path}")


def sweep(engine, depth=DEPTH):
    """What each rule costs the tree, at a fixed depth, against all four off.

    Not Elo and not a claim to be one (DEC-019): node counts at a fixed depth
    say which candidate is worth an SPRT and nothing about the outcome. What
    the `moves` column is for is the other half -- a rule that saves nodes and
    changes the answer on many positions is pruning moves that mattered, which
    is the S021 instrument.
    """
    fens = positions()
    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()

    def wait_for(prefix):
        while True:
            line = proc.stdout.readline()
            if not line:
                sys.exit("the engine died")
            if line.startswith(prefix):
                return line.strip()

    def measure(caps):
        for name in OFF:
            send(f"setoption name {name} value {caps.get(name, 0)}")
        total, moves, nodes = 0, [], 0
        for fen in fens:
            send("ucinewgame")
            send("position fen " + fen)
            send(f"go depth {depth}")
            while True:
                line = proc.stdout.readline()
                if not line:
                    sys.exit("the engine died")
                if line.startswith("info score"):
                    field = line.split()
                    nodes = int(field[field.index("nodes") + 1])
                if line.startswith("bestmove"):
                    moves.append(line.split()[1])
                    total += nodes
                    break
        return total, moves

    send("uci")
    wait_for("uciok")

    ON = 8
    rows = [("all off", {})]
    for name in OFF:
        rows.append((name.replace("MaxLmrDepth", ""), {name: ON}))
    rows.append(("all on", {name: ON for name in OFF}))

    print(f"# {len(fens)} positions, depth {depth}, engine {engine}")
    print("rule\tnodes\trel\tmoves_changed")
    base_nodes, base_moves = None, None
    for label, caps in rows:
        total, moves = measure(caps)
        if base_nodes is None:
            base_nodes, base_moves = total, moves
        changed = sum(1 for a, b in zip(moves, base_moves) if a != b)
        print(f"{label}\t{total}\t{total / base_nodes:.4f}\t{changed}",
              flush=True)

    send("quit")
    proc.wait()


def read(path):
    cutoff = collections.defaultdict(collections.Counter)
    quiets = collections.defaultdict(collections.Counter)
    reduction = {}
    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or line.startswith("kind"):
                continue
            kind, depth, key, count = line.split()
            depth, key, count = int(depth), int(key), int(count)
            if kind == "cutoff":
                cutoff[depth][key] += count
            elif kind == "quiets":
                quiets[depth][key] += count
            elif kind == "reduction":
                reduction[(depth, key)] = count
    return cutoff, quiets, reduction


def percentile(counter, pct):
    """The smallest key at or below which `pct` per cent of the mass sits."""
    total = sum(counter.values())
    if total == 0:
        return None
    seen = 0
    for key in sorted(counter):
        seen += counter[key]
        if 100 * seen >= pct * total:
            return key
    return max(counter)


def median(counter):
    return percentile(counter, 50)


def fit(path):
    cutoff, quiets, reduction = read(path)

    def lmr_depth_of(depth, move):
        left = depth - reduction.get((min(depth, 63), min(move, 63)), 0)
        return max(left, 0)

    print(f"census {path}")
    print(f"  quiet beta cutoffs   {sum(sum(r.values()) for r in cutoff.values())}")
    print(f"  nodes generating quiets {sum(sum(r.values()) for r in quiets.values())}")
    print()
    print(f"depth  cutoffs      p50  p{PERCENTILE}  p99   max   "
          f"lmr_depth(depth, p{PERCENTILE})  median quiets")
    table = {}
    for depth in sorted(cutoff):
        if depth < 1:
            continue
        row = cutoff[depth]
        p = percentile(row, PERCENTILE)
        table[depth] = p
        print(f"{depth:>5}  {sum(row.values()):>9}  "
              f"{percentile(row, 50):>4} {p:>4} {percentile(row, 99):>4}  "
              f"{max(row):>4}   {lmr_depth_of(depth, p):>20}  "
              f"{median(quiets.get(depth, collections.Counter())) or 0:>13}")

    # The pair the rule has to reproduce: fire first at the census's count,
    # tested at the lmr depth that count lands on.
    rows = [(lmr_depth_of(d, table[d]), table[d])
            for d in FIT_DEPTHS if d in table]
    if len(rows) < 2:
        sys.exit("not enough rows to fit")

    n = len(rows)
    sx = sum(x for x, _ in rows)
    sy = sum(y for _, y in rows)
    sxx = sum(x * x for x, _ in rows)
    sxy = sum(x * y for x, y in rows)
    denominator = n * sxx - sx * sx
    if denominator == 0:
        sys.exit(f"the fit rows share one lmr depth: {rows}")
    slope = (n * sxy - sx * sy) / denominator
    base = (sy - slope * sx) / n

    print()
    print(f"fit rows (lmr_depth, count) over remaining depths {FIT_DEPTHS}: "
          f"{rows}")
    print(f"least squares, unconstrained: "
          f"threshold = {base:.4f} + {slope:.4f} * lmr_depth")

    # THE FINDING, and it is the reason this block exists rather than one
    # `print`. The slope comes out **negative** -- on this bracket, on every
    # other bracket tried (1..8), and at p99 as well as p95. It is not the
    # lmr_depth coupling of the first pass: the same fall is there on the
    # remaining-depth axis in the table above, 8 8 6 5 6 6 4 5 3 3. In this
    # tree a deeper node cuts off *earlier* in the move order, because its
    # table move and killers are better, and the census cannot support a count
    # that grows with depth.
    #
    # LmpDepthCoeff's declared range is non-negative by stated purpose: a
    # negative coefficient prunes harder the deeper the node, which inverts the
    # mechanism rather than tuning it and points the rule straight at the
    # hazard this repository keeps meeting. So the fit is taken subject to that
    # bound, which puts the coefficient on its floor and the base at the mean
    # of the rows.
    if slope < 0:
        slope = 0.0
        base = sy / n
        print("  the slope is negative, and the declared range is not: "
              "refitted at LmpDepthCoeff = 0")
    print(f"least squares, in range:      "
          f"threshold = {base:.4f} + {slope:.4f} * lmr_depth")
    print(f"  LmpBase        {round(base * 100)}   (hundredths of a move)")
    print(f"  LmpDepthCoeff  {round(slope * 100)}")

    # One median over every node that generated quiets: the move list does not
    # get shorter with depth (the table above shows it flat), so a per-depth
    # median would only add noise to a cap that is about the move list's length.
    all_quiets = collections.Counter()
    for row in quiets.values():
        all_quiets.update(row)
    med = median(all_quiets)

    print()
    print(f"median quiet moves generated at a node: {med}")
    print("cap: the largest lmr_depth whose fitted threshold is still below it")
    print("lmr_depth  threshold  binds")
    binding = []
    for lmr_depth in range(0, 17):
        threshold = base + slope * lmr_depth
        binds = threshold < med
        if binds:
            binding.append(lmr_depth)
        print(f"{lmr_depth:>9}  {threshold:>9.2f}  {'yes' if binds else 'no'}")

    print()
    if len(binding) == 17:
        # The criterion only excludes depths where the threshold is past the
        # end of the move list, and a flat threshold below the median is never
        # past it. So it discriminates nothing here and does not get to set the
        # cap by accident: the cap falls back to DEC-105 form (c), the declared
        # range's midpoint, which is what the other three rules' caps are.
        print("  the criterion does not discriminate: the threshold binds at "
              "every lmr depth in range,")
        print("  so the cap is not derived and takes the declared midpoint "
              "instead (DEC-105 form (c))")
        print("  LmpMaxLmrDepth 8   (midpoint of 0..16; the rule reads "
              "`lmr_depth < cap`)")
    else:
        print(f"  LmpMaxLmrDepth {min(max(binding) + 1, 16)}   (the rule reads "
              "`lmr_depth < cap`; the declared range tops at 16)")


def main(argv):
    if len(argv) < 2:
        sys.exit(__doc__)
    if argv[1] == "sweep":
        sweep(argv[2])
    elif argv[1] == "run":
        run(argv[2], argv[3])
    elif argv[1] == "fit":
        fit(argv[2])
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main(sys.argv)
