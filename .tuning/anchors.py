#!/usr/bin/env python3
"""Recompute the suite's absolute evaluation anchors from a weight set, by hand.

A second implementation of evaluate() for the pinned positions, written from the
specification in src/evaluation.cpp rather than by calling the engine: an anchor
copied from the thing it anchors asserts nothing (S028).

The feature vectors below are hand-derived and do not depend on the weights, so
running this against the weights the engine ships must reproduce every value the
suite asserts today. That is the check that the derivation is right; the same run
against a fitted header then gives the new anchors.

Usage: anchors.py [emitted.hpp]     no argument means the shipped weights
"""
import re
import sys

ROOT = "/home/max/ws/chesso/"

# index 0 is a8, 63 is h1
def sq(name):
    return (8 - int(name[1])) * 8 + "abcdefgh".index(name[0])


PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING = range(6)

KS = {  # king_safety_feature_t's order
    "KNIGHT_ATT": 0,
    "BISHOP_ATT": 1,
    "ROOK_ATT": 2,
    "QUEEN_ATT": 3,
    "ZONE_ATTACKS": 4,
    "SHIELD_NEAR": 5,
    "SHIELD_FAR": 6,
    "OPEN_FILE": 7,
    "HALF_OPEN_FILE": 8,
}

# Every count below was derived by hand from the position and is White relative
# where a sign appears:
#
#  phase      sum of phase_value over both sides' pieces
#  psqt       (type, square, sign); Black's pieces read the mirrored square
#  material   (name, signed count)
#  passed     bucket -> signed count
#  structure  isolated, doubled, backward, White minus Black
#  placement  pair, rook open, rook half open, rook seventh, White minus Black
#  mobility   (type, attacked squares not occupied by own pieces, sign)
#  attackers  (type, zone-attack incidences, sign of the *defender*)
#  shelter    per colour: the shield and file counts of that king
CASES = [
    dict(
        title="pawn on e2",
        fen="4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",
        anchors={"evaluate": 135},   # test_evaluation.cpp:247
        stm="w",
        phase=0,
        material=[("PAWN", 1)],
        psqt=[(PAWN, sq("e2"), 1), (KING, sq("e1"), 1), (KING, sq("e1"), -1)],
        passed={0: 1},  # no black pawns at all, so e2 passes: 6 - (52 >> 3) = 0
        structure=(1, 0, 0),  # isolated: no white pawn on d or f
        placement=(0, 0, 0, 0),
        mobility=[],
        attackers=[],
        # White's own pawn closes the e file, so d and f are open for him; for
        # Black the same file holds an enemy pawn, so it is half open
        shelter={"w": {"SHIELD_NEAR": 1, "OPEN_FILE": 2},
                 "b": {"OPEN_FILE": 2, "HALF_OPEN_FILE": 1}},
    ),
    dict(
        title="knight on b1",
        fen="4k3/8/8/8/8/8/8/1N2K3 w - - 0 1",
        anchors={"evaluate": 244},   # test_evaluation.cpp:248
        stm="w",
        phase=1,
        material=[("KNIGHT", 1)],
        psqt=[(KNIGHT, sq("b1"), 1), (KING, sq("e1"), 1), (KING, sq("e1"), -1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, 0, 0, 0),
        mobility=[(KNIGHT - 1, 3, 1)],  # a3, c3, d2
        attackers=[],
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},  # no pawns
    ),
    dict(
        title="bishop on c1",
        fen="4k3/8/8/8/8/8/8/2B1K3 w - - 0 1",
        anchors={"evaluate": 325},   # test_evaluation.cpp:249
        stm="w",
        phase=1,
        material=[("BISHOP", 1)],
        psqt=[(BISHOP, sq("c1"), 1), (KING, sq("e1"), 1), (KING, sq("e1"), -1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, 0, 0, 0),  # one bishop is not a pair
        mobility=[(BISHOP - 1, 7, 1)],  # d2 e3 f4 g5 h6, b2 a3
        attackers=[],  # the long ray stops at h6, clear of Black's king zone
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
    dict(
        title="rook on d1",
        fen="4k3/8/8/8/8/8/8/3RK3 w - - 0 1",
        anchors={"evaluate": 563, "cheap": 567},   # test_evaluation.cpp:250, test_search.cpp:592-593
        stm="w",
        phase=2,
        material=[("ROOK", 1)],
        psqt=[(ROOK, sq("d1"), 1), (KING, sq("e1"), 1), (KING, sq("e1"), -1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, 1, 0, 0),  # no pawn of either colour on the d file
        mobility=[(ROOK - 1, 10, 1)],  # a1 b1 c1, d2..d8; e1 is own-occupied
        attackers=[(ROOK - 1, 2, -1)],  # d7 and d8 are in Black's king zone
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
    dict(
        title="queen on d1",
        fen="4k3/8/8/8/8/8/8/3QK3 w - - 0 1",
        anchors={"evaluate": 787},   # test_evaluation.cpp:251
        stm="w",
        phase=4,
        material=[("QUEEN", 1)],
        psqt=[(QUEEN, sq("d1"), 1), (KING, sq("e1"), 1), (KING, sq("e1"), -1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, 0, 0, 0),  # the open-file feature counts rooks only
        mobility=[(QUEEN - 1, 17, 1)],  # the rook's 10 plus e2 f3 g4 h5, c2 b3 a4
        attackers=[(QUEEN - 1, 2, -1)],
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
    dict(
        title="bare kings cancel",
        fen="4k3/8/8/8/8/8/8/4K3 w - - 0 1",
        anchors={"evaluate": 0},
        stm="w",
        phase=0,
        material=[],
        psqt=[(KING, sq("e1"), 1), (KING, sq("e1"), -1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, 0, 0, 0),
        mobility=[],
        attackers=[],
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
    # test_search, "a side in check may not stand pat". White Re8 Re1 Kg1,
    # Black Kg8 Qa4 pawns f7 g7 h7, Black to move.
    dict(
        title="black in check, Re8",
        fen="4R1k1/5ppp/8/8/q7/8/8/4R1K1 b - - 0 1",
        anchors={"evaluate": 198},   # test_search.cpp:653
        stm="b",
        phase=8,  # two white rooks and one black queen
        material=[("ROOK", 2), ("QUEEN", -1), ("PAWN", -3)],
        psqt=[
            (ROOK, sq("e8"), 1),
            (ROOK, sq("e1"), 1),
            (KING, sq("g1"), 1),
            (KING, sq("g1"), -1),  # Black's king on g8 mirrors onto g1
            (QUEEN, sq("a5"), -1),  # a4 mirrored
            (PAWN, sq("f2"), -1),  # f7 mirrored
            (PAWN, sq("g2"), -1),
            (PAWN, sq("h2"), -1),
        ],
        passed={0: -3},  # White has no pawns, so all three of Black's pass
        structure=(0, 0, 0),  # f g h are mutual neighbours; nothing is backward
        placement=(0, 2, 0, 0),  # both white rooks on the open e file
        mobility=[(ROOK - 1, 12, 1), (ROOK - 1, 11, 1), (QUEEN - 1, 21, -1)],
        attackers=[(ROOK - 1, 2, -1)],  # Re8 bears on f8 and g8
        shelter={"w": {"HALF_OPEN_FILE": 3},  # f g h each hold a black pawn only
                 "b": {"SHIELD_NEAR": 3}},  # f7 g7 h7 in front of the king
    ),
    # test_search, "the losing side takes an available repetition", after
    # Ra2-b2 Kh8-h7 Rb2-a2. White Ra2 Ka1, Black Kh7, Black to move.
    dict(
        title="black a rook down, Kh7",
        fen="8/7k/8/8/8/8/R7/K7 b - - 3 2",
        anchors={"evaluate": -569},  # test_search.cpp:1133
        stm="b",
        phase=2,
        material=[("ROOK", 1)],
        psqt=[(ROOK, sq("a2"), 1), (KING, sq("a1"), 1), (KING, sq("h2"), -1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, 1, 0, 0),  # the a file holds no pawn of either colour
        # a3..a8 is 6 and b2..h2 is 7; a1 is in the attack set and own-occupied
        mobility=[(ROOK - 1, 13, 1)],
        attackers=[],  # the a file and the second rank miss h7's zone entirely
        # a1 sees files a and b, h7 sees g and h: the edge clips both to two
        shelter={"w": {"OPEN_FILE": 2}, "b": {"OPEN_FILE": 2}},
    ),
]

# test_search, "a quiet evasion is a legal answer to a check": quiesce() on
# 4rk2/8/8/8/8/8/8/4K3 w - - 0 1, White to move and in check from Re8.
#
# Not an evaluation anchor and not derivable as one. In check, quiescence
# searches evasions rather than standing pat, so the number the test pins is a
# one-ply negamax and not a call to evaluate(). White has four king moves, none
# of them a capture; after each, Black is to move, is not in check, and has no
# capture either, so every leaf returns its own stand pat. The root is therefore
# max over the four of -evaluate(leaf), and each leaf is an ordinary anchor.
#
# The e file is clear from e7 down to e1 in all four, so Black's rook reaches
# whichever of e1, e2, e3 lies in White's king zone.
LEAVES = [
    dict(
        title="after Ke1-d1",
        fen="4rk2/8/8/8/8/8/8/3K4 b - - 1 1",
        anchors={},
        stm="b",
        phase=2,
        material=[("ROOK", -1)],
        psqt=[(ROOK, sq("e1"), -1), (KING, sq("f1"), -1), (KING, sq("d1"), 1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, -1, 0, 0),  # Black's rook, on the open e file
        # a8..d8 is 4 and e1..e7 is 7; f8 is in the attack set and own-occupied
        mobility=[(ROOK - 1, 11, -1)],
        attackers=[(ROOK - 1, 2, 1)],  # d1's zone holds e1 and e2
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
    dict(
        title="after Ke1-d2",
        fen="4rk2/8/8/8/8/8/3K4/8 b - - 1 1",
        anchors={},
        stm="b",
        phase=2,
        material=[("ROOK", -1)],
        psqt=[(ROOK, sq("e1"), -1), (KING, sq("f1"), -1), (KING, sq("d2"), 1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, -1, 0, 0),
        mobility=[(ROOK - 1, 11, -1)],
        attackers=[(ROOK - 1, 3, 1)],  # d2's zone holds e1, e2 and e3
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
    dict(
        title="after Ke1-f1",
        fen="4rk2/8/8/8/8/8/8/5K2 b - - 1 1",
        anchors={},
        stm="b",
        phase=2,
        material=[("ROOK", -1)],
        psqt=[(ROOK, sq("e1"), -1), (KING, sq("f1"), -1), (KING, sq("f1"), 1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, -1, 0, 0),
        mobility=[(ROOK - 1, 11, -1)],
        attackers=[(ROOK - 1, 2, 1)],  # f1's zone holds e1 and e2
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
    dict(
        title="after Ke1-f2",
        fen="4rk2/8/8/8/8/8/5K2/8 b - - 1 1",
        anchors={},
        stm="b",
        phase=2,
        material=[("ROOK", -1)],
        psqt=[(ROOK, sq("e1"), -1), (KING, sq("f1"), -1), (KING, sq("f2"), 1)],
        passed={},
        structure=(0, 0, 0),
        placement=(0, -1, 0, 0),
        mobility=[(ROOK - 1, 11, -1)],
        attackers=[(ROOK - 1, 3, 1)],  # f2's zone holds e1, e2 and e3
        shelter={"w": {"OPEN_FILE": 3}, "b": {"OPEN_FILE": 3}},
    ),
]

QUIESCE_IN_CHECK = -505  # test_search.cpp:696, on the shipped weights


def weights(source):
    if source is None:
        text = open(ROOT + "src/eval_tables.hpp").read() + open(
            ROOT + "src/evaluation.cpp"
        ).read()
    else:
        text = open(source).read()

    def strip(s):
        return re.sub(r"//[^\n]*", "", s)

    w = {}
    w.update(
        (n, int(v)) for n, v in re.findall(r"^#define (\w+)\s+(-?\d+)$", text, re.M)
    )
    for name in ("psqt_mg", "psqt_eg"):
        m = re.search(
            r"static constexpr int %s\[6\]\[64\] = \{(.*?)\n\};" % name, text, re.S
        )
        values = [int(v) for v in re.findall(r"-?\d+", strip(m.group(1)))]
        assert len(values) == 384, (name, len(values))
        w[name] = values
    for n, body in re.findall(r"^const int (\w+)\[\w+\] = \{([^}]*)\};", text, re.M):
        w[n] = [int(v) for v in re.findall(r"-?\d+", strip(body))]
    for n, v in re.findall(r"^const int (\w+) = (-?\d+);", text, re.M):
        w[n] = int(v)
    return w


def trunc_div(numerator, denominator):
    # C integer division truncates towards zero; Python's // floors.
    q = abs(numerator) // denominator
    return q if numerator >= 0 else -q


def score(case, w):
    MAX = 24
    phase = case["phase"]
    endgame = MAX - phase
    stm = 1 if case["stm"] == "w" else -1

    material = sum(w[name] * count for name, count in case["material"])

    psqt_mg = sum(sign * w["psqt_mg"][t * 64 + s] for t, s, sign in case["psqt"])
    psqt_eg = sum(sign * w["psqt_eg"][t * 64 + s] for t, s, sign in case["psqt"])

    pawn_mg = pawn_eg = 0
    for bucket, count in case["passed"].items():
        pawn_mg += count * w["passed_pawn_mg"][bucket]
        pawn_eg += count * w["passed_pawn_eg"][bucket]
    for f, count in enumerate(case["structure"]):
        pawn_mg += count * w["pawn_structure_mg"][f]
        pawn_eg += count * w["pawn_structure_eg"][f]
    for f, count in enumerate(case["placement"]):
        pawn_mg += count * w["piece_placement_mg"][f]
        pawn_eg += count * w["piece_placement_eg"][f]

    positional = trunc_div(
        (psqt_mg + pawn_mg) * phase + (psqt_eg + pawn_eg) * endgame, MAX
    )
    tempo = trunc_div(w["tempo_mg"] * phase + w["tempo_eg"] * endgame, MAX)
    cheap = stm * (material + positional) + tempo

    mob_mg = mob_eg = 0
    for t, count, sign in case["mobility"]:
        mob_mg += sign * count * w["mobility_mg"][t]
        mob_eg += sign * count * w["mobility_eg"][t]

    # An attack on a king zone is worth its weight to the king under fire, so it
    # enters the White-relative sum with the defender's sign.
    ks_mg = ks_eg = 0
    for t, incidences, sign in case["attackers"]:
        ks_mg += sign * (
            w["king_safety_mg"][KS["KNIGHT_ATT"] + t]
            + incidences * w["king_safety_mg"][KS["ZONE_ATTACKS"]]
        )
        ks_eg += sign * (
            w["king_safety_eg"][KS["KNIGHT_ATT"] + t]
            + incidences * w["king_safety_eg"][KS["ZONE_ATTACKS"]]
        )
    for colour, sign in (("w", 1), ("b", -1)):
        for name, count in case["shelter"][colour].items():
            ks_mg += sign * count * w["king_safety_mg"][KS[name]]
            ks_eg += sign * count * w["king_safety_eg"][KS[name]]

    mobility = trunc_div(mob_mg * phase + mob_eg * endgame, MAX)
    safety = trunc_div(ks_mg * phase + ks_eg * endgame, MAX)
    expensive = stm * max(-150, min(150, mobility + safety))  # LAZY_EVAL_MARGIN

    return dict(
        evaluate=cheap + expensive,
        cheap=cheap,
        positional=positional,
        tempo=tempo,
        mobility=mobility,
        safety=safety,
        expensive=expensive,
    )


def main():
    source = sys.argv[1] if len(sys.argv) > 1 else None
    w = weights(source)
    print("weights from %s" % (source or "src/eval_tables.hpp + src/evaluation.cpp"))
    bad = 0
    for case in CASES:
        parts = score(case, w)
        checks = []
        for name, shipped in case["anchors"].items():
            got = parts[name]
            if source is None:
                checks.append(
                    "%s %d vs %d %s" % (name, got, shipped, "OK" if got == shipped else "MISMATCH")
                )
                bad += got != shipped
            else:
                checks.append("%s %d (was %d)" % (name, got, shipped))
        print(
            "%-22s %-34s positional %5d tempo %4d mobility %4d safety %5d"
            % (
                case["title"],
                ", ".join(checks),
                parts["positional"],
                parts["tempo"],
                parts["mobility"],
                parts["safety"],
            )
        )
    print()
    best = None
    for leaf in LEAVES:
        leaf_score = score(leaf, w)["evaluate"]
        print(
            "  %-22s evaluate %5d  -> root %5d"
            % (leaf["title"], leaf_score, -leaf_score)
        )
        best = -leaf_score if best is None else max(best, -leaf_score)

    if source is None:
        checks = "%d vs %d %s" % (
            best,
            QUIESCE_IN_CHECK,
            "OK" if best == QUIESCE_IN_CHECK else "MISMATCH",
        )
        bad += best != QUIESCE_IN_CHECK
    else:
        checks = "%d (was %d)" % (best, QUIESCE_IN_CHECK)
    print("%-22s quiesce %s" % ("white in check, Re8", checks))

    if source is None:
        total = sum(len(c["anchors"]) for c in CASES) + 1
        print("\n%d of %d reproduced" % (total - bad, total))
        sys.exit(1 if bad else 0)


main()
