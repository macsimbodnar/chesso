#!/usr/bin/env python3
"""Turn a samply profile into a text self-time report.

samply is built for the Firefox Profiler UI, which is no use in a terminal or
to an agent. Recording with --unstable-presymbolicate writes a .syms.json
sidecar next to the profile; this reads both and prints self time per function,
which is the number that says where to optimise.

    samply record --save-only --unstable-presymbolicate -o prof.json.gz -- ./binary
    tools/samply_report.py prof.json.gz

The binary must carry debug info (-g). On macOS also run
`dsymutil <binary>` first, otherwise every frame resolves to a bare address.
"""

import bisect
import collections
import gzip
import json
import sys


def load(profile_path):
    sidecar = profile_path
    for suffix in (".gz", ".json"):
        if sidecar.endswith(suffix):
            sidecar = sidecar[: -len(suffix)]
    sidecar += ".json.syms.json"

    opener = gzip.open if profile_path.endswith(".gz") else open
    with opener(profile_path) as handle:
        profile = json.load(handle)

    try:
        with open(sidecar) as handle:
            symbols = json.load(handle)
    except FileNotFoundError:
        sys.exit(
            f"missing {sidecar}\n"
            "re-record with --unstable-presymbolicate, or frames stay as addresses"
        )

    return profile, symbols


def symbol_tables(symbols):
    """debug_name -> (sorted rva list, symbol entries), for bisect lookup."""
    tables = {}
    for entry in symbols["data"]:
        table = sorted(entry["symbol_table"], key=lambda s: s["rva"])
        tables[entry["debug_name"]] = ([s["rva"] for s in table], table)
    return tables


def main():
    if len(sys.argv) < 2:
        sys.exit(f"usage: {sys.argv[0]} <profile.json.gz> [top-n]")

    top_n = int(sys.argv[2]) if len(sys.argv) > 2 else 15
    profile, symbols = load(sys.argv[1])

    names = symbols["string_table"]
    tables = symbol_tables(symbols)
    libs = profile["libs"]

    report = collections.Counter()

    for thread in profile["threads"]:
        frames = thread["frameTable"]
        stacks = thread["stackTable"]
        funcs = thread["funcTable"]
        resources = thread.get("resourceTable")
        strings = thread.get("stringArray") or profile.get("shared", {}).get(
            "stringArray"
        )

        def lib_of(frame):
            resource = funcs.get("resource", [None] * funcs["length"])[
                frames["func"][frame]
            ]
            if resources is None or resource is None or resource < 0:
                return None
            lib = resources["lib"][resource]
            return libs[lib]["debugName"] if lib is not None else None

        def resolve(frame):
            table = tables.get(lib_of(frame))
            if table is not None:
                rvas, entries = table
                address = frames["address"][frame]
                index = bisect.bisect_right(rvas, address) - 1
                if index >= 0:
                    entry = entries[index]
                    if address < entry["rva"] + entry["size"]:
                        return names[entry["symbol"]]
            if strings is None:
                return "UNKNOWN"
            return strings[funcs["name"][frames["func"][frame]]]

        for stack, weight in zip(thread["samples"]["stack"], thread["samples"]["weight"]):
            if stack is None:
                continue
            report[resolve(stacks["frame"][stack])] += weight

    total = sum(report.values())
    if total == 0:
        sys.exit("no samples in profile")

    print(f"{total} samples\n")
    for name, count in report.most_common(top_n):
        print(f"{100 * count / total:6.2f}%  {name}")


if __name__ == "__main__":
    main()
