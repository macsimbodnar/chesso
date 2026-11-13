#!/usr/bin/env python3
"""DEC-362 (3) and (4) for S124's part A: the two segments of one resumed SPRT, read from its PGN.

    python3 -I adocs/data/S124_partA_segments.py PGN STATE_AT_STOP FINAL_STATE [CONSOLE...]

`adocs/data/S124_partA_segments.txt` is the reading, with the command that made it. PGN is the
run's games.pgn, both segments appended; STATE_AT_STOP is fastchess's config.json as saved at the
stop, FINAL_STATE the same file as the resumed run left it (both kept in the owner's archive,
chesso_S124_partA_sprt_2026-11-13.tar.xz).

A game's segment is its GameStartTime's date (the stop was 2026-11-12 11:59, the resume
2026-11-13 00:03). Its fastchess game number is 2 * Round - 1 with the candidate White,
2 * Round with it Black. A pair is the two games of one Round inside one segment; a pair is
counted only when both its games are in that segment, which is fastchess's own accounting
(its statistics move only when a pair completes). Checked here: segment 1's pentanomial
equals the state saved at the stop, and segment 1 plus segment 2 equals the final state.

Per segment: the candidate's score per game s, its standard error from the pairs
(pentanomial, sd of the pair score over sqrt(pairs)), Elo -400 log10(1/s - 1) with its
standard error by the delta method; the difference's z; and each side's nodes per second,
the sum of n= over the sum of the move times, moves with a time of 0 left out (the rule of
the family's quarters side check, S124's step file). Also the coordinator's variant of
segment 1 (the first copies of the pair written twice dropped). Counts games and scores; no
chess judgement.

THE DIP, a sensitivity reading added at the coordinator's request, 2026-11-13. In ten-minute
bins of GameStartTime (printed below for 09:00 to 10:59) three bins of segment 1, 09:50, 10:00
and 10:10, read both sides' nodes per second below every other bin of the run, so the window is
[2026-11-12 09:50:00, 10:20:00) by GameStartTime. Every complete pair with a game started inside
it is dropped and the rest read again: games, W/L/D, pentanomial, Elo, nElo and the SPRT's LLR
at {0, 5} nElo. Each side's nodes per second is printed inside the window and outside it, over
every game written. A sensitivity reading, not a second verdict: the pre-registered verdict is
the pooled run's (DEC-063).

THE LLR is fastchess's own, ported line for line from its source at the played build
(app/src/matchmaking/sprt/sprt.cpp, fastchess alpha 1.8.1 20260720-daa3ea2, unchanged in that
function to 74deac2): the pentanomial counts regularized (a zero read as 1e-3), WL and DD
pooled, the normalized model's maximum likelihood distributions at t0 and t1 = sqrt(2) nElo /
(800 / ln 10) found by Van den Bergh's iteration (at most 10 rounds, 1e-4) with theta solved by
the ITP method (k1 0.1, k2 2, n0 0.99, epsilon 1e-7), and LLR = N x the mean log ratio under
the empirical distribution. Given CONSOLE files it is checked first against every report
fastchess printed in them: each `Ptnml(0-2)` line's counts, read back through the port, must
give that report's `LLR:` to its two printed decimals.
"""
import json
import math
import re
import sys
from collections import Counter, defaultdict

COMMENT = re.compile(r"\{[^}]*?([0-9.]+)s, tl=[0-9.]+s, n=(\d+)\}")
POINTS = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": 0.5}


def read(path):
    raw = open(path, encoding="utf-8", errors="replace").read()
    rows = []
    for chunk in re.split(r"(?m)^(?=\[Event )", raw):
        if not chunk.strip():
            continue
        head = dict(re.findall(r'^\[(\w+) "([^"]*)"\]', chunk, re.M))
        cand_white = head["White"].startswith("cand-")
        rnd = int(head["Round"])
        number = 2 * rnd - 1 if cand_white else 2 * rnd
        white_points = POINTS[head["Result"]]
        body = chunk.split("\n\n", 1)[1] if "\n\n" in chunk else ""
        side = {"cand": [0.0, 0], "ref": [0.0, 0]}
        black_first = head.get("FEN", "").split()[1:2] == ["b"]
        for k, (t, n) in enumerate(COMMENT.findall(body)):
            white_move = (k % 2 == 0) != black_first
            who = "cand" if white_move == cand_white else "ref"
            t = float(t)
            if t > 0:
                side[who][0] += t
                side[who][1] += int(n)
        rows.append({
            "index": len(rows) + 1,
            "round": rnd,
            "number": number,
            "cand": white_points if cand_white else 1.0 - white_points,
            "start": head["GameStartTime"],
            "end": head.get("GameEndTime", ""),
            "termination": head.get("Termination", ""),
            "side": side,
        })
    return rows


def pairs_of(games):
    by_round = defaultdict(dict)
    for g in games:
        by_round[g["round"]][g["number"]] = g
    out = []
    halves = []
    for rnd, pair in sorted(by_round.items()):
        if len(pair) == 2:
            out.append(sum(g["cand"] for g in pair.values()))
        else:
            halves.extend(pair.values())
    return out, halves


def penta(pair_scores):
    counts = [0] * 5
    for p in pair_scores:
        counts[int(round(2 * p))] += 1
    return counts


def stats(pair_scores):
    n = len(pair_scores)
    per_game = [p / 2.0 for p in pair_scores]
    s = sum(per_game) / n
    var = sum((x - s) ** 2 for x in per_game) / n
    se_s = math.sqrt(var / n)
    elo = -400.0 * math.log10(1.0 / s - 1.0)
    se_elo = se_s * 400.0 / (math.log(10) * s * (1.0 - s))
    # nElo as fastchess's normalized model reads it for pentanomial pairs:
    # (s - 1/2) / (sqrt(2) sd_pair_per_game) * 800 / ln 10
    nelo = (s - 0.5) / (math.sqrt(2.0 * var)) * 800.0 / math.log(10)
    se_nelo = se_s / math.sqrt(2.0 * var) * 800.0 / math.log(10)
    return {"pairs": n, "s": s, "sd_pair": math.sqrt(var), "se_s": se_s,
            "elo": elo, "se_elo": se_elo, "nelo": nelo, "se_nelo": se_nelo}


def nps(games):
    out = {}
    for who in ("cand", "ref"):
        t = sum(g["side"][who][0] for g in games)
        n = sum(g["side"][who][1] for g in games)
        out[who] = n / t / 1000.0
    return out


def state_penta(path):
    st = list(json.load(open(path))["stats"].values())[0]
    return [st["penta_LL"], st["penta_LD"], st["penta_WL"] + st["penta_DD"],
            st["penta_WD"], st["penta_WW"]], (st["wins"], st["losses"], st["draws"])


def wld(games):
    w = sum(1 for g in games if g["cand"] == 1.0)
    l = sum(1 for g in games if g["cand"] == 0.0)
    return w, l, len(games) - w - l


def show(label, st, extra=""):
    print("%s: %d pairs, s %.5f (sd per pair %.4f), Elo %+.2f se %.2f (95 %% +/- %.2f), "
          "nElo %+.2f se %.2f (95 %% +/- %.2f)%s" % (
              label, st["pairs"], st["s"], st["sd_pair"], st["elo"], st["se_elo"],
              1.96 * st["se_elo"], st["nelo"], st["se_nelo"], 1.96 * st["se_nelo"], extra))


def compare(a, b, label):
    se = math.sqrt(a["se_elo"] ** 2 + b["se_elo"] ** 2)
    z = (a["elo"] - b["elo"]) / se
    print("%s: Elo difference %+.2f, se of the difference %.2f, z %+.2f, |z| %s 2.58" % (
        label, a["elo"] - b["elo"], se, z, "<=" if abs(z) <= 2.58 else ">"))
    se_s = math.sqrt(a["se_s"] ** 2 + b["se_s"] ** 2)
    print("%s: same in score per game, difference %+.5f, se %.5f, z %+.2f" % (
        label, a["s"] - b["s"], se_s, (a["s"] - b["s"]) / se_s))
    return z


def itp(f, a, b, f_a, f_b, k_1, k_2, n_0, epsilon):
    """fastchess's itp(), with IEEE arithmetic as C++ has it (an infinite f at the ends)."""
    if f_a > 0:
        a, b = b, a
        f_a, f_b = f_b, f_a
    n_half = math.ceil(math.log2(abs(b - a) / (2.0 * epsilon)))
    n_max = n_half + n_0
    i = 0
    while abs(b - a) > 2.0 * epsilon:
        x_half = (a + b) / 2.0
        r = epsilon * 2.0 ** (n_max - i) - (b - a) / 2.0
        delta = k_1 * (b - a) ** k_2
        x_f = (f_b * a - f_a * b) / (f_b - f_a)
        diff = x_half - x_f
        sigma = diff / abs(diff) if diff == diff and diff != 0.0 else float("nan")
        x_t = x_f + sigma * delta if delta <= abs(diff) else x_half
        x_itp = x_t if abs(x_t - x_half) <= r else x_half - sigma * r
        f_itp = f(x_itp)
        if f_itp == 0.0:
            a = b = x_itp
        elif math.copysign(1.0, f_itp) < 0:
            a, f_a = x_itp, f_itp
        else:
            b, f_b = x_itp, f_itp
        i += 1
    return (a + b) / 2.0


def fastchess_llr(ptnml, elo0=0.0, elo1=5.0):
    """fastchess's SPRT::getLLR over a pentanomial [LL, LD, WL+DD, WD, WW], model normalized."""
    counts = [c if c != 0 else 1e-3 for c in ptnml]
    total = sum(counts)
    probs = [c / total for c in counts]
    scores = [0.0, 0.25, 0.5, 0.75, 1.0]
    n = len(scores)

    def mle(mu_ref, t_star):
        p = [1.0 / n] * n
        for _ in range(10):
            mu = sum(a * q for a, q in zip(scores, p))
            var = sum(q * (a - mu) ** 2 for a, q in zip(scores, p))
            sigma = math.sqrt(var)
            phi = [a - mu_ref - 0.5 * t_star * sigma * (1.0 + ((a - mu) / sigma) ** 2) for a in scores]
            u, v = min(phi), max(phi)
            theta = itp(lambda x: sum(ph * f / (1.0 + x * f) for ph, f in zip(probs, phi)),
                        -1.0 / v, -1.0 / u, float("inf"), float("-inf"), 0.1, 2.0, 0.99, 1e-7)
            new = [ph / (1.0 + theta * f) for ph, f in zip(probs, phi)]
            max_diff = max(abs(x - y) for x, y in zip(new, p))
            p = new
            if max_diff < 1e-4:
                break
        return p

    scale = 800.0 / math.log(10)
    p0 = mle(0.5, math.sqrt(2.0) * elo0 / scale)
    p1 = mle(0.5, math.sqrt(2.0) * elo1 / scale)
    return total * sum(ph * (math.log(b) - math.log(a)) for ph, a, b in zip(probs, p0, p1))


def verify_llr(consoles):
    """Every report in the consoles: the port must give fastchess's printed LLR."""
    checked = bad = 0
    for path in consoles:
        ptnml = None
        for line in open(path, encoding="utf-8", errors="replace"):
            m = re.match(r"Ptnml\(0-2\): \[(\d+), (\d+), (\d+), (\d+), (\d+)\]", line)
            if m:
                ptnml = [int(x) for x in m.groups()]
                continue
            m = re.match(r"LLR: (-?[0-9.]+) ", line)
            if m and ptnml is not None:
                printed = m.group(1)
                ours = "%.2f" % fastchess_llr(ptnml)
                checked += 1
                if ours != printed and ours != printed.replace("-0.00", "0.00"):
                    bad += 1
                    if bad <= 5:
                        print("  LLR MISMATCH %s: ptnml %s printed %s, the port %s" % (path, ptnml, printed, ours))
                ptnml = None
    print("the LLR port against fastchess's own reports: %d checked, %d differ at two decimals" % (checked, bad))
    return bad == 0


def complete_pairs(games):
    """{round: [its two games]} for the rounds both of whose games are in `games`."""
    by_round = defaultdict(list)
    for g in games:
        by_round[g["round"]].append(g)
    return {r: v for r, v in by_round.items() if len(v) == 2}


def main(pgn, state_stop, state_final, *consoles):
    rows = read(pgn)
    print("games in the PGN: %d" % len(rows))
    seg = {1: [g for g in rows if g["start"].startswith("2026-11-12")],
           2: [g for g in rows if g["start"].startswith("2026-11-13")]}
    other = len(rows) - len(seg[1]) - len(seg[2])
    print("segment 1: %d games, file positions %d to %d, started %s to %s" % (
        len(seg[1]), seg[1][0]["index"], seg[1][-1]["index"],
        min(g["start"] for g in seg[1]), max(g["start"] for g in seg[1])))
    print("segment 2: %d games, file positions %d to %d, started %s to %s" % (
        len(seg[2]), seg[2][0]["index"], seg[2][-1]["index"],
        min(g["start"] for g in seg[2]), max(g["start"] for g in seg[2])))
    print("games in neither: %d" % other)
    print("segment 1's last end %s; segment 2's last end %s" % (
        max(g["end"] for g in seg[1]), max(g["end"] for g in seg[2])))

    seen = defaultdict(list)
    for g in rows:
        seen[g["number"]].append(g)
    dups = {k: v for k, v in seen.items() if len(v) > 1}
    print("game numbers written twice: %s" % ", ".join(
        "%d (file positions %s, segments %s)" % (
            k, "/".join(str(g["index"]) for g in v),
            "/".join("1" if g in seg[1] else "2" for g in v)) for k, v in sorted(dups.items())))
    numbers = set(seen)
    top = max(numbers)
    missing = sorted(set(range(1, top + 1)) - numbers)
    print("game numbers 1 to %d absent from the PGN: %s" % (top, missing))

    result = {}
    for k in (1, 2):
        ps, halves = pairs_of(seg[k])
        result[k] = (ps, halves)
        print("segment %d: %d complete pairs (%d games), half pairs not counted: %s" % (
            k, len(ps), 2 * len(ps), ", ".join(
                "game %d (round %d, file position %d, %s)" % (
                    g["number"], g["round"], g["index"], g["start"][11:19]) for g in halves) or "none"))
    sp_stop, wld_stop = state_penta(state_stop)
    sp_final, wld_final = state_penta(state_final)
    p1 = penta(result[1][0])
    p2 = penta(result[2][0])
    pooled = [a + b for a, b in zip(p1, p2)]
    counted1 = [g for g in seg[1] if g not in result[1][1]]
    counted2 = [g for g in seg[2] if g not in result[2][1]]
    w1, w2 = wld(counted1), wld(counted2)
    print("segment 1 ptnml %s, W L D %s; the state at the stop %s, W L D %s: %s" % (
        p1, w1, sp_stop, wld_stop, "equal" if p1 == sp_stop and w1 == wld_stop else "DIFFER"))
    seg2_state = [a - b for a, b in zip(sp_final, sp_stop)]
    seg2_wld = tuple(a - b for a, b in zip(wld_final, wld_stop))
    print("segment 2 ptnml %s, W L D %s; final state less the stop's %s, W L D %s: %s" % (
        p2, w2, seg2_state, seg2_wld, "equal" if p2 == seg2_state and w2 == seg2_wld else "DIFFER"))
    print("pooled ptnml %s; the final state %s: %s" % (
        pooled, sp_final, "equal" if pooled == sp_final else "DIFFER"))

    s1, s2 = stats(result[1][0]), stats(result[2][0])
    sp = stats(result[1][0] + result[2][0])
    show("segment 1", s1)
    show("segment 2", s2)
    show("pooled   ", sp)
    z = compare(s1, s2, "segments 1 vs 2")

    # The coordinator's variant: segment 1 with the first copies of the pair written twice
    # dropped, i.e. its complete pairs less the rounds segment 2 also completed.
    rounds2 = {g["round"] for g in seg[2]}
    by_round1 = defaultdict(list)
    for g in counted1:
        by_round1[g["round"]].append(g)
    dropped = sorted(r for r in by_round1 if r in rounds2)
    ps_variant = [sum(g["cand"] for g in by_round1[r]) for r in sorted(by_round1) if r not in rounds2]
    sv = stats(ps_variant)
    show("segment 1 less round(s) %s" % dropped, sv)
    compare(sv, s2, "variant vs segment 2")

    n1, n2 = nps(seg[1]), nps(seg[2])
    for who in ("cand", "ref"):
        move = 100.0 * (n2[who] - n1[who]) / n1[who]
        print("nps %s: segment 1 %.1f knps, segment 2 %.1f knps, %+.2f %% (%s 3 %%)" % (
            who, n1[who], n2[who], move, "within" if abs(move) <= 3.0 else "PAST"))
    print("terminations: %s" % dict(sorted(Counter(g["termination"] for g in rows).items())))
    print("verdict of (4)'s first two clauses: %s" % (
        "hold" if abs(z) <= 2.58 and all(abs(100.0 * (n2[w] - n1[w]) / n1[w]) <= 3.0
                                         for w in ("cand", "ref")) else "FAIL"))

    # THE DIP: a sensitivity reading (the docstring), not a second verdict.
    print()
    print("THE DIP, a sensitivity reading")
    if consoles and not verify_llr(consoles):
        print("the port does not reproduce fastchess; no LLR below is read")
        return
    print("the port on the pooled pentanomial %s: LLR %.4f (fastchess printed -2.96)" % (
        sp_final, fastchess_llr(sp_final)))
    bins = defaultdict(list)
    for g in rows:
        if "2026-11-12T09:00" <= g["start"][:16] < "2026-11-12T11:00":
            bins[g["start"][:15]].append(g)
    print("ten-minute bins of GameStartTime, 09:00 to 10:59 (every other bin of the run reads the"
          " candidate between 1888 and 1945 knps):")
    for k in sorted(bins):
        b = nps(bins[k])
        print("  %s0  %3d games  cand %.0f knps  ref %.0f knps" % (k[11:], len(bins[k]), b["cand"], b["ref"]))
    lo, hi = "2026-11-12T09:50:00", "2026-11-12T10:20:00"
    inside = [g for g in rows if lo <= g["start"][:19] < hi]
    outside = [g for g in rows if not lo <= g["start"][:19] < hi]
    print("the window [%s, %s) by GameStartTime: %d games started in it, all in segment 1" % (
        lo.replace("T", " "), hi[11:], len(inside)))
    ni, no = nps(inside), nps(outside)
    for who in ("cand", "ref"):
        print("nps %s: inside the window %.1f knps (%d games), outside it %.1f knps (%d games), %+.2f %%" % (
            who, ni[who], len(inside), no[who], len(outside), 100.0 * (ni[who] - no[who]) / no[who]))
    print("inside the window the candidate's rate is the reference's %+.2f %%; outside it %+.2f %%" % (
        100.0 * (ni["cand"] - ni["ref"]) / ni["ref"], 100.0 * (no["cand"] - no["ref"]) / no["ref"]))
    pairs1 = complete_pairs(counted1)
    pairs2 = complete_pairs(counted2)
    hit = sorted(r for r, v in pairs1.items() if any(lo <= g["start"][:19] < hi for g in v))
    kept1 = [v for r, v in sorted(pairs1.items()) if r not in hit]
    dropped_pairs = [pairs1[r] for r in hit]
    kept = kept1 + [v for _, v in sorted(pairs2.items())]
    print("pairs dropped: %d (rounds %d to %d), every one in segment 1; %d pairs kept of %d" % (
        len(hit), hit[0], hit[-1], len(kept), len(pairs1) + len(pairs2)))
    for label, sel in (("the dropped pairs alone", dropped_pairs), ("what is left", kept)):
        scores_ = [sum(g["cand"] for g in v) for v in sel]
        games_ = [g for v in sel for g in v]
        pt = penta(scores_)
        st = stats(scores_)
        print("%s: %d games, W L D %s, ptnml %s" % (label, len(games_), wld(games_), pt))
        print("  Elo %+.2f +/- %.2f, nElo %+.2f +/- %.2f (95 %%), LLR at {0, 5} nElo %.2f" % (
            st["elo"], 1.96 * st["se_elo"], st["nelo"], 1.96 * st["se_nelo"], fastchess_llr(pt)))
    compare(stats([sum(g["cand"] for g in v) for v in dropped_pairs]),
            stats([sum(g["cand"] for g in v) for v in kept]), "the dropped pairs vs what is left")
    # The LLR moves with the pairs' count as well as their mean: what is left at the pooled
    # run's own pentanomial proportions, scaled to the kept count, says how much of the move
    # is the count alone.
    scaled = [c * len(kept) / sum(sp_final) for c in sp_final]
    print("the pooled proportions scaled to %d pairs: LLR %.2f (the count alone, no change of mean)" % (
        len(kept), fastchess_llr(scaled)))


if __name__ == "__main__":
    main(*sys.argv[1:])
