#!/usr/bin/env python3
"""S261: does one lane of all its axes resolve each axis as well as a lane of half?

    python3 adocs/data/S261_split_sim.py [--config tools/spsa_s261.json]
                                         [--out .tuning/s261_split_sim]

The pre-registration's split test, item (2) of
`adocs/plan_current/S261_search_block_spsa_lane.md`. It runs once, before the
lane launches, and its last line decides between the one lane and the two
lanes that file states. It is never run after a lane has started.

It plays no games. `tools/spsa_driver.py run --sim` replaces the match with a
noisy quadratic whose optimum is known (S084's gate), and this script runs it
at the lane's own shape -- the frozen config's iterations, pairs, schedule,
bounds and c_end -- under two objectives:

  all   every axis has an optimum four c_end from its start, 3 Elo deep
  half  the same, but every odd-indexed axis has weight 0, so it moves nothing
        and the run is a half-width lane carrying the other half as passengers

and reads, for the even-indexed axes weighted in both, the mean share of each
axis's 3 Elo the run's rounded endpoint recovered. A zero-weight axis puts
nothing into the result of a mini-match, so the only difference between the
two is the cross-talk the odd-indexed gradients add to every axis's estimate --
which is exactly what a split would remove. Nothing here counts the axes: the
config's 32 (DEC-269) give 32 against 16, and an axis the reachability check
removed before launch is simply absent.

Why 3 Elo an axis: S085's simulator (adocs/plan_done/S085_spsa_first_run.md,
"What the simulator said") resolved a 12-axis objective 36 Elo deep at this
budget and did little at 15 -- 3 and 1.25 Elo an axis. Four c_end is the
distance that table and DEV_MANUAL's c_end rule were measured at. The draw rate
is S231's lane's own, 18432 of 60000 games.

The rule, fixed before the number: SPLIT if the full-width mean is below 0.8
of the half-width mean; ONE LANE otherwise; and if the half-width mean is not
positive the test cannot tell the two apart, which reads ONE LANE and says so.
Five seeds each. Stdlib only.
"""

import argparse
import json
import os
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
DRIVER = ROOT / "tools" / "spsa_driver.py"

DEPTH_PER_AXIS = 3.0   # Elo; S085's simulator, 36 over 12 axes
DRAW_RATE = 18432 / 60000  # S231's lane
SEEDS = (1, 2, 3, 4, 5)
SPLIT_BELOW = 0.8


def optimum(p, i):
    """Four c_end from the start, inside the range: above it for an even index
    and below for an odd one where the range allows, else the farther end. The
    weight makes the axis DEPTH_PER_AXIS Elo deep with sigma = c_end."""
    dist = 4.0 * p["c_end"]
    up, down = p["start"] + dist, p["start"] - dist
    first, second = ((up, down) if i % 2 == 0 else (down, up))
    if p["min"] <= first <= p["max"]:
        o = first
    elif p["min"] <= second <= p["max"]:
        o = second
    elif p["max"] - p["start"] >= p["start"] - p["min"]:
        o = p["max"]
    else:
        o = p["min"]
    z = (o - p["start"]) / p["c_end"]
    return o, DEPTH_PER_AXIS / (z * z)


def spec(params, weighted):
    opt, weights, sigmas = [], [], []
    for i, p in enumerate(params):
        o, w = optimum(p, i)
        opt.append(o)
        weights.append(w if weighted(i) else 0.0)
        sigmas.append(p["c_end"])
    return {"optimum": opt, "weights": weights, "sigmas": sigmas,
            "draw_rate": DRAW_RATE}


def uci_value(p, x):
    """What the driver sends and what would ship: clamp, then round."""
    return int(round(min(p["max"], max(p["min"], x))))


def recovered(params, theta, sp, idx):
    """Mean share of each listed axis's available Elo the endpoint recovered;
    1 is the optimum, 0 the start, negative worse than the start."""
    shares = []
    for i in idx:
        p = params[i]
        o, s = sp["optimum"][i], sp["sigmas"][i]
        z0 = (p["start"] - o) / s
        z1 = (uci_value(p, theta[i]) - o) / s
        shares.append(1.0 - (z1 * z1) / (z0 * z0))
    return sum(shares) / len(shares)


def run_one(raw, sp, seed, run_dir):
    if os.path.exists(run_dir):
        sys.exit(f"{run_dir} exists; a split test is run once, into a fresh --out")
    os.makedirs(run_dir)
    cfg_path = os.path.join(run_dir, "config.json")
    spec_path = os.path.join(run_dir, "sim.json")
    with open(cfg_path, "w") as fh:
        json.dump(dict(raw, seed=seed), fh, indent=2)
    with open(spec_path, "w") as fh:
        json.dump(sp, fh, indent=2)
    out = os.path.join(run_dir, "run")
    proc = subprocess.run([sys.executable, str(DRIVER), "run", cfg_path,
                           "--sim", spec_path, "--out", out],
                          capture_output=True, text=True)
    last = proc.stdout.strip().splitlines()[-1:] if proc.stdout.strip() else []
    if proc.returncode != 0 or last != ["SPSA-DONE"]:
        sys.exit(f"the driver failed on {run_dir} (exit {proc.returncode}):\n"
                 f"{proc.stdout[-2000:]}\n{proc.stderr[-2000:]}")
    with open(os.path.join(out, "checkpoint.json")) as fh:
        return json.load(fh)["theta"]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--config", default=str(ROOT / "tools" / "spsa_s261.json"))
    ap.add_argument("--out", default=str(ROOT / ".tuning" / "s261_split_sim"))
    args = ap.parse_args()

    with open(args.config) as fh:
        raw = json.load(fh)
    params = raw["params"]
    common = [i for i in range(len(params)) if i % 2 == 0]
    modes = (("all", lambda i: True), ("half", lambda i: i % 2 == 0))

    means = {}
    for mode, weighted in modes:
        sp = spec(params, weighted)
        got = []
        for seed in SEEDS:
            theta = run_one(raw, sp, seed,
                            os.path.join(args.out, f"{mode}_seed{seed}"))
            share = recovered(params, theta, sp, common)
            got.append(share)
            print(f"{mode:4s} seed {seed}: {share:+.3f} of each common axis's "
                  f"{DEPTH_PER_AXIS:g} Elo recovered, mean over {len(common)} axes")
        means[mode] = sum(got) / len(got)
        print(f"{mode:4s} mean over {len(SEEDS)} seeds: {means[mode]:+.3f}")

    if means["half"] <= 0.0:
        print(f"the {len(common)}-axis lane recovers nothing at this depth, so "
              "the test cannot tell one lane from two")
        print("S261-SPLIT-SIM: ONE LANE (inconclusive)")
        return 0
    ratio = means["all"] / means["half"]
    verdict = "SPLIT" if ratio < SPLIT_BELOW else "ONE LANE"
    print(f"{len(params)} axes against {len(common)}: {ratio:.3f} of the "
          f"narrower lane's per-axis recovery; split below {SPLIT_BELOW}")
    print(f"S261-SPLIT-SIM: {verdict}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
