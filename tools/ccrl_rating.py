#!/usr/bin/env python3
"""Read CCRL Blitz ratings for named engines, from the list rather than from us.

    tools/ccrl_rating.py --cache /tmp/ccrl.html "Leorik 1.0" "Blunder 5.0.0"

Prints one TSV row per engine: name, rating, +err, -err, list date. Exits
non-zero if any requested engine is not on the list, because a missing anchor
must stop a rated run rather than silently drop a reference (S087, DEC-068).

The name matched is the CCRL entry name without its " 64-bit" suffix, compared
exactly. "Leorik 1.0" must not match "Leorik 1.0.1".
"""

import argparse
import datetime
import html
import os
import re
import sys
import urllib.request

LIST_URL = "https://computerchess.org.uk/ccrl/404/rating_list_all.html"

# A rating cell is a bare 3-4 digit number. Engines below 1000 exist on the
# list, so the lower bound is deliberately loose and the upper one is not a
# guess about strength: it only has to exclude the game counts in later cells.
RATING = re.compile(r"^[0-9]{3,4}$")
ERR = re.compile(r"^[+\-−][0-9]+$")


def fetch(cache):
    """Return the list HTML, downloading it unless a cache file already has it."""
    if cache and os.path.exists(cache) and os.path.getsize(cache) > 0:
        with open(cache, encoding="utf-8", errors="replace") as f:
            return f.read(), datetime.date.fromtimestamp(os.path.getmtime(cache))
    req = urllib.request.Request(LIST_URL, headers={"User-Agent": "chesso-S087"})
    with urllib.request.urlopen(req, timeout=120) as r:
        body = r.read().decode("utf-8", errors="replace")
    if cache:
        with open(cache, "w", encoding="utf-8") as f:
            f.write(body)
    return body, datetime.date.today()


def rows(body):
    """Yield each table row as a list of cell strings, tags stripped."""
    for raw in re.split(r"<tr[^>]*>", body):
        text = html.unescape(re.sub(r"<[^>]+>", "\x00", raw))
        cells = [c.strip() for c in text.split("\x00") if c.strip()]
        if cells:
            yield cells


def find(body, wanted):
    """Map each wanted engine name to (rating, plus, minus), or None."""
    found = {}
    for cells in rows(body):
        for i, cell in enumerate(cells):
            name = cell[: -len(" 64-bit")] if cell.endswith(" 64-bit") else cell
            if name not in wanted or name in found:
                continue
            rest = cells[i + 1 :]
            if not rest or not RATING.match(rest[0]):
                continue
            errs = [c for c in rest[1:3] if ERR.match(c)]
            plus = errs[0] if errs else "?"
            minus = errs[1] if len(errs) > 1 else "?"
            found[name] = (rest[0], plus, minus)
    return found


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("engines", nargs="+")
    ap.add_argument("--cache", help="reuse this file instead of downloading")
    args = ap.parse_args()

    body, date = fetch(args.cache)
    found = find(body, set(args.engines))

    for name in args.engines:
        if name in found:
            rating, plus, minus = found[name]
            print(f"{name}\t{rating}\t{plus}\t{minus}\t{date}")

    missing = [n for n in args.engines if n not in found]
    if missing:
        print(f"not on the CCRL list: {', '.join(missing)}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
