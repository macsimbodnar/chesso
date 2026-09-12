#!/usr/bin/env python3
"""Re-derive tests/test_audit_polyglot_key.cpp's cases with python-chess.

S042/DEC-187, 2026-09-12. `load_FEN`'s sanitizer gained a capturer test
(en_passant_is_capturable, src/bitboard.cpp): an en-passant square now
survives loading only when a pawn of the side to move actually attacks the
target, matching X-FEN and the Polyglot specification's own reading. Eight of
the ten pre-existing cases in that file have no such pawn (they were built to
isolate a *different*, earlier bug -- get_key()'s wraparound arithmetic on an
edge-file square, S175/2026-09-03_adversarial-F01), so their precondition
changed from "en_passant is set" to "en_passant is INVALID_INDEX". Two new
cases were added on a non-edge file so the ep term of the key is still
exercised by a case that keeps the square. `spec_key` itself does not move for
any of the twelve: python-chess's hash_ep_square() (chess/polyglot.py) derives
capturability itself from the pawns on the board, not from the FEN's fourth
field, so it never trusted the over-permissive classical convention this step
replaces.

This script does not carry its own copy of the twelve cases -- that would be a
second place for them to drift out of sync with the C++ file. It parses
`cases` straight out of tests/test_audit_polyglot_key.cpp with a regex over
the four braced fields (fen, spec_key, capturable, why) and checks each fen's
`has_pseudo_legal_en_passant()` against `capturable` and its
`chess.polyglot.zobrist_hash()` against `spec_key`. Run it again whenever
either end moves: a case edited in the C++ file, or a python-chess upgrade.

Exit 0 only when every case's capturable flag and spec_key both agree.

Usage:
  ~/.venv/chess/bin/python adocs/data/S042_polyglot_key_cases.py \
      tests/test_audit_polyglot_key.cpp

python-chess 1.11.2 in ~/.venv/chess (TOOLCHAIN.md); the system python has no
python-chess, which is why this is not a ctest.
"""

import re
import sys

import chess
import chess.polyglot

CASE_RE = re.compile(
    r'\{"([^"]+)",\s*(0x[0-9A-Fa-f]+)ULL,\s*(true|false),\s*"([^"]+)"\}'
)


def parse_cases(path):
    with open(path, encoding="utf-8") as handle:
        text = handle.read()
    cases = [
        (fen, int(spec_key, 16), capturable == "true", why)
        for fen, spec_key, capturable, why in CASE_RE.findall(text)
    ]
    if not cases:
        sys.exit(f"{path}: no key_case_t entries matched -- regex out of date?")
    return cases


def main():
    if len(sys.argv) != 2:
        sys.exit(f"usage: {sys.argv[0]} tests/test_audit_polyglot_key.cpp")

    cases = parse_cases(sys.argv[1])
    mismatches = 0

    for fen, spec_key, capturable, why in cases:
        board = chess.Board(fen)
        actual_capturable = board.has_pseudo_legal_en_passant()
        actual_key = chess.polyglot.zobrist_hash(board)

        cap_ok = actual_capturable == capturable
        key_ok = actual_key == spec_key
        status = "ok" if cap_ok and key_ok else "MISMATCH"
        if not (cap_ok and key_ok):
            mismatches += 1

        print(f"{status:8} capturable={actual_capturable!s:5} "
              f"(expected {capturable!s:5}) key=0x{actual_key:016X} "
              f"(expected 0x{spec_key:016X})  -- {why}")

    print(f"\n{len(cases)} cases, {mismatches} mismatches")
    return 0 if mismatches == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
