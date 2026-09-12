#!/usr/bin/env python3
"""S223's census: every FEN literal in tests/, src/ and tools/ against
python-chess's Board.status(), reporting the ones a one-king-a-side and
no-check-against-the-side-not-to-move load bound would refuse.

Run from the repository root with the python-chess interpreter:

    ~/.venv/chess/bin/python adocs/data/S223_fen_census.py > adocs/data/S223_fen_census.txt

The regex takes four to six FEN fields; a four-field literal is completed
with `0 1`, which is what `position fen` does too (S176). The vendored
tests/json/ tree is excluded. The output is the step's cost list: DEC-197 and
adocs/plan_todo/S223_position_fen_legality_boundary.md read it. Written by
the coordinator on 2026-09-12 while dispositioning
adocs/audit/2026-09-12_adversarial.md.
"""
import re, glob, sys
import chess
print("python-chess", chess.__version__)
pat = re.compile(r'((?:[1-8pnbrqkPNBRQK]+/){7}[1-8pnbrqkPNBRQK]+)\s+([wb])\s+(-|[KQkqA-Ha-h]{1,4})\s+(-|[a-h][36])(?:\s+(\d+)\s+(\d+))?')
files = [f for f in glob.glob('tests/**/*', recursive=True) if re.search(r'\.(cpp|hpp|json|py|epd|txt)$', f) and not f.startswith('tests/json/')]
files += glob.glob('src/*.cpp') + glob.glob('src/*.hpp') + glob.glob('tools/*.cpp') + glob.glob('tools/*.hpp') + glob.glob('tools/*.py')
seen = {}
for f in files:
    try:
        text = open(f, errors='replace').read()
    except Exception:
        continue
    for lineno, line in enumerate(text.splitlines(), 1):
        for m in pat.finditer(line):
            fen = ' '.join(x for x in m.groups() if x is not None)
            if m.group(5) is None:
                fen += ' 0 1'
            seen.setdefault(fen, []).append(f'{f}:{lineno}')
flags = chess.STATUS_NO_WHITE_KING | chess.STATUS_NO_BLACK_KING | chess.STATUS_TOO_MANY_KINGS | chess.STATUS_OPPOSITE_CHECK
bad = []
n_ok = 0
for fen, where in seen.items():
    try:
        b = chess.Board(fen)
        st = b.status()
    except Exception as e:
        bad.append((fen, where, f'unparseable: {e}')); continue
    hit = st & flags
    if hit:
        names = [n[7:] for n in ('STATUS_NO_WHITE_KING','STATUS_NO_BLACK_KING','STATUS_TOO_MANY_KINGS','STATUS_OPPOSITE_CHECK') if st & getattr(chess, n)]
        bad.append((fen, where, ','.join(names)))
    else:
        n_ok += 1
print(f"unique FENs: {len(seen)}  pass the king/opposite-check gate: {n_ok}  would be refused: {len(bad)}")
for fen, where, why in sorted(bad, key=lambda t: t[1][0]):
    print(f"  {why:32s} {fen:58s} {' '.join(where[:3])}{' ...' if len(where)>3 else ''}")
b = chess.Board("7k/8/8/8/8/8/8/K6R w - - 0 1")
print("audit FEN status:", int(b.status()), "OPPOSITE_CHECK" if b.status() & chess.STATUS_OPPOSITE_CHECK else "")
