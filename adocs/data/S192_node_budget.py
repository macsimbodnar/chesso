#!/usr/bin/env python3
"""Re-derive the node band of tests/test_search.cpp "ordering keeps the tree small".

S192, DEC-142. The case searches TRICKY_POS to depth 5 from a cold table and
asserts the cost is inside a band: below a budget of 440000 and above a floor of
20000. Neither number is a measurement of anything on its own -- both are ratios
of the count the case actually costs, 4x above and a fifth below, chosen so the
case fires on ordering that has stopped working and not on a tree that moved.
109575 was the count when the band was placed on 2026-08-14.

So the re-derivation is: run the case, read the count it prints, multiply. The
count is read off a MESSAGE rather than recomputed here, because an in-process
search(5, ...) on a cold table with no aspiration is not the same search as a
UCI [go depth 5] of the same FEN and only the binary can do the first one.

    python3 adocs/data/S192_node_budget.py [path/to/test_search]

Prints the count, the two bounds the ratios give, and where the count sits in
the band the source currently carries. It asserts nothing and changes nothing:
what to do about a count that has left the middle half of the band is the
step's decision, not this script's.
"""
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BINARY = os.path.join(REPO, "build", "tests", "test_search")
CASE = "ordering keeps the tree small"
SOURCE = os.path.join(REPO, "tests", "test_search.cpp")

# Both are the ratios the case has carried since 2026-08-14: 440000 / 109575 is
# 4.02 and 20000 / 109575 is 1 / 5.5. Rounded to the two the comment states.
BUDGET_RATIO = 4
FLOOR_DIVISOR = 5


def read_count(binary):
    """The count the case reports, from its own MESSAGE under --success."""
    out = subprocess.run(
        [binary, "--test-case=" + CASE, "--success"],
        cwd=os.path.dirname(binary),
        capture_output=True,
        text=True,
    )
    if out.returncode != 0:
        sys.exit(
            "the case did not pass, so its count is not a number to derive from:\n"
            + out.stdout[-2000:]
            + out.stderr[-2000:]
        )

    found = re.search(r"ordering node count: (\d+)", out.stdout)
    if not found:
        sys.exit(
            "no [ordering node count:] MESSAGE in the output. The case is what "
            "prints it and --success is what shows it; if the case no longer "
            "prints it, this script is stale and not the test."
        )
    return int(found.group(1))


def read_shipping():
    """The two numbers the source asserts today, so the run can be compared."""
    text = open(SOURCE).read()
    case = text[text.index('"' + CASE + '"'):]
    budget = re.search(r"state\.node_limit = (\d+);", case)
    floor = re.search(r"result\.explored_nodes > (\d+)", case)
    if not budget or not floor:
        sys.exit(
            "the case no longer spells its budget as [state.node_limit = N;] "
            "or its floor as [result.explored_nodes > N]; read them by hand "
            "rather than trusting this script"
        )
    return int(budget.group(1)), int(floor.group(1))


def main():
    binary = sys.argv[1] if len(sys.argv) > 1 else BINARY
    if not os.path.exists(binary):
        sys.exit("no test binary at " + binary + " -- build it first")

    count = read_count(binary)
    budget = count * BUDGET_RATIO
    floor = count // FLOOR_DIVISOR
    shipping_budget, shipping_floor = read_shipping()

    print("binary        %s" % binary)
    print("count         %d nodes, depth 5 on TRICKY_POS, cold table" % count)
    print("budget        %d  (%dx the count)" % (budget, BUDGET_RATIO))
    print("floor         %d  (the count over %d)" % (floor, FLOOR_DIVISOR))
    print()
    print("shipping      budget %d, floor %d" % (shipping_budget, shipping_floor))
    print(
        "the count is  %.2fx the shipping floor and %.2f of the shipping budget"
        % (count / shipping_floor, count / shipping_budget)
    )

    # The middle half of the band the source carries. Outside it the count has
    # drifted far enough that the band is no longer centred on the search, which
    # is when DEC-142 says the golden is re-derived rather than left.
    low = shipping_floor + (shipping_budget - shipping_floor) // 4
    high = shipping_budget - (shipping_budget - shipping_floor) // 4
    where = "inside" if low <= count <= high else "OUTSIDE"
    print(
        "middle half   [%d, %d]: the count is %s it" % (low, high, where)
    )


main()
