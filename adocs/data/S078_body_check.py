#!/usr/bin/env python3
"""S078 acceptance check.

Non-vacuous by construction: every title and every source line the step files
are required to quote is first extracted from tests/, and the check refuses
with exit 2 if any of them is not really there. Only then are the step bodies
tested. A rename in tests/ turns this red on the precondition, not on the gate.

Usage: s078_check.py <S060 path> <S061 path>
"""
import re
import sys

SEARCH_SRC = "tests/test_search.cpp"
EVAL_SRC = "tests/test_evaluation.cpp"

TEMPLATE = "pruning does not hide a mate against the material leader"
BAND = "bands are strictly ordered"
GENERAL = ["mate in one", "mate in two is found at the right distance"]

# The three lines S060 must reproduce verbatim as the preconditions the
# template asserts, plus the depth loop it runs the mate over.
PRECONDITIONS = [
    "REQUIRE(load_FEN(after_key, &game));",
    "REQUIRE_FALSE(is_check(&game));",
    "REQUIRE(evaluate(&game.board) > 300);",
]
DEPTH_LOOP = "for (int depth = 3; depth <= 6; ++depth)"

# The sentence S060 must stop making. S033 filled the absence it argued from.
ABSENCE = re.compile(r"no test name asserts it|Nothing states that the existing cases")
# A line-range citation of the band case's file, which S061 must no longer carry.
RANGE_CITE = re.compile(r"test_evaluation\.cpp:\d+")


def titles(path):
    body = open(path, encoding="utf-8").read()
    return set(re.findall(r'TEST_CASE_FIXTURE\([a-z_]+_t,\s*\n?\s*"([^"]+)"', body))


def flat(text):
    """Collapse whitespace. The step files are hard-wrapped, so a quoted title
    or source line is routinely split across two lines; a line-wise match would
    miss it. S062's checker was line-based and missed two of five for exactly
    this reason."""
    return " ".join(text.split())


def main():
    s060_path, s061_path = sys.argv[1], sys.argv[2]

    # --- precondition: everything quoted below is real -----------------------
    search_titles = titles(SEARCH_SRC)
    eval_titles = titles(EVAL_SRC)
    search_body = open(SEARCH_SRC, encoding="utf-8").read()

    missing = [t for t in [TEMPLATE] + GENERAL if t not in search_titles]
    if BAND not in eval_titles:
        missing.append(BAND)
    missing += [ln for ln in PRECONDITIONS + [DEPTH_LOOP] if ln not in search_body]
    if missing:
        for m in missing:
            print(f"PRECONDITION not in tests/: {m}")
        return 2
    print(f"precondition: {len(PRECONDITIONS) + 1} source lines and "
          f"{len(GENERAL) + 2} titles all present in tests/")

    # --- the gate ------------------------------------------------------------
    s060_raw = open(s060_path, encoding="utf-8").read()
    s061_raw = open(s061_path, encoding="utf-8").read()
    s060, s061 = flat(s060_raw), flat(s061_raw)
    bad = []

    if ABSENCE.search(s060):
        bad.append("S060 still argues from an absence S033 filled")
    if TEMPLATE not in s060:
        bad.append(f"S060 does not name the template {TEMPLATE!r}")
    if SEARCH_SRC not in s060:
        bad.append(f"S060 does not name {SEARCH_SRC}")
    for ln in PRECONDITIONS:
        if ln not in s060:
            bad.append(f"S060 does not quote the precondition {ln!r}")
    if DEPTH_LOOP not in s060:
        bad.append(f"S060 does not quote the depth loop {DEPTH_LOOP!r}")

    if BAND not in s061:
        bad.append(f"S061 does not name the band case {BAND!r}")
    for hit in RANGE_CITE.findall(s061):
        bad.append(f"S061 still cites the band case by line range: {hit}")

    # --- neither accepts: changed -------------------------------------------
    for name, text, expect in [
        ("S060", s060_raw, "S026's accepts requires, once per technique,"),
        ("S061", s061_raw, "S023's accepts names the case that discharges"),
    ]:
        line = next(l for l in text.splitlines() if l.startswith("accepts:"))
        if expect not in line:
            bad.append(f"{name} accepts: no longer opens with its original clause")

    for b in bad:
        print("FLAG:", b)
    print(f"flagged: {len(bad)}")
    return 1 if bad else 0


sys.exit(main())
