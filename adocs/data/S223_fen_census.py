#!/usr/bin/env python3
"""S223's census: every FEN literal in tests/, src/ and tools/ against
python-chess's Board.status(), reporting the ones a one-king-a-side and
no-check-against-the-side-not-to-move load bound would refuse -- and, from
S223's completing commit on, whether the engine itself agrees with the oracle
on every one of them.

Run from the repository root with the python-chess interpreter:

    ~/.venv/chess/bin/python adocs/data/S223_fen_census.py > adocs/data/S223_fen_census.txt

The regex takes four to six FEN fields; a four-field literal is completed
with `0 1`, which is what `position fen` does too (S176). The vendored
tests/json/ tree is excluded. The output is the step's cost list: DEC-197 and
adocs/plan_done/S223_position_fen_legality_boundary.md read it. Written by
the coordinator on 2026-09-12 while dispositioning
adocs/audit/2026-09-12_adversarial.md.

The second pass drives `build/src/chesso` over UCI, one `position fen` and one
`fen` per literal, and reads the `info string refused` line the loader prints.
It is the agreement check S223's accepts asks for, and it is a script rather
than a paragraph so that it can be re-run whenever either end moves (DEC-142).
What it compares is deliberately narrow: **whether the engine refuses at all**
against **whether the oracle reports one of the four king flags**, over the
positions S208's two classes and the parser do not refuse first. Anything else
would be comparing two different rulebooks -- python-chess also reports
BAD_CASTLING_RIGHTS, INVALID_EP_SQUARE, TOO_MANY_CHECKERS and the rest, none of
which this engine refuses and none of which it should (S161 repairs the first
two, and S223's excludes names the others).

    --engine <path>   which binary to drive (default build/src/chesso)
    --no-engine       the python-chess pass only, as the script was on
                      2026-09-12
"""
import re, glob, sys, subprocess
import chess

ENGINE = 'build/src/chesso'
RUN_ENGINE = True
argv = sys.argv[1:]
while argv:
    if argv[0] == '--engine':
        ENGINE = argv[1]
        argv = argv[2:]
    elif argv[0] == '--no-engine':
        RUN_ENGINE = False
        argv = argv[1:]
    else:
        sys.exit(f"unknown argument {argv[0]}")

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
king_flagged = set()
parseable = set()
for fen, where in seen.items():
    try:
        b = chess.Board(fen)
        st = b.status()
    except Exception as e:
        bad.append((fen, where, f'unparseable: {e}')); continue
    parseable.add(fen)
    hit = st & flags
    if hit:
        names = [n[7:] for n in ('STATUS_NO_WHITE_KING','STATUS_NO_BLACK_KING','STATUS_TOO_MANY_KINGS','STATUS_OPPOSITE_CHECK') if st & getattr(chess, n)]
        bad.append((fen, where, ','.join(names)))
        king_flagged.add(fen)
    else:
        n_ok += 1
print(f"unique FENs: {len(seen)}  pass the king/opposite-check gate: {n_ok}  would be refused: {len(bad)}")
for fen, where, why in sorted(bad, key=lambda t: t[1][0]):
    print(f"  {why:32s} {fen:58s} {' '.join(where[:3])}{' ...' if len(where)>3 else ''}")
b = chess.Board("7k/8/8/8/8/8/8/K6R w - - 0 1")
print("audit FEN status:", int(b.status()), "OPPOSITE_CHECK" if b.status() & chess.STATUS_OPPOSITE_CHECK else "")

if not RUN_ENGINE:
    sys.exit(0)


# One process for the whole corpus. `position fen <fen>` prints a refusal line
# or nothing; `fen` then prints one line whatever happened, which is the
# sentinel that says the reply for this FEN is complete.
def engine_verdicts(fens):
    proc = subprocess.Popen([ENGINE], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1)
    proc.stdin.write("uci\n")
    proc.stdin.flush()
    while "uciok" not in proc.stdout.readline():
        pass

    out = {}
    for fen in fens:
        proc.stdin.write(f"position fen {fen}\nfen\n")
        proc.stdin.flush()
        reason = None
        while True:
            line = proc.stdout.readline()
            if not line:
                sys.exit("engine closed its stdout")
            line = line.strip()
            if line.startswith("info string refused"):
                reason = line.split(", ", 1)[1] if ", " in line else line
                continue
            break  # the `fen` echo, refused or not
        out[fen] = reason

    proc.stdin.write("quit\n")
    proc.stdin.flush()
    proc.wait(timeout=10)
    return out


# The reasons S208 owns. A FEN one of them answers for says nothing about the
# king rules, so it is excluded from the comparison rather than counted as an
# agreement -- the ordering that keeps them first is pinned by
# tests/test_audit_fen_semantics.cpp, case "S208's two reasons still fire
# first".
EARLIER = ("more than 16 pieces", "a pawn on rank 1 or rank 8")

# What load_position() prints when the loader filled no reason in: a syntax or
# field failure, which happens before any placement rule and is not one. Three
# literals in the corpus are here on purpose -- two out-of-range clocks and a
# fullmove counter of 0, all of them S210's and S161's refusals.
PARSER = "does not load"

verdicts = engine_verdicts(sorted(parseable))
compared = 0
earlier = 0
parser = 0
disagree = []
for fen in sorted(parseable):
    reason = verdicts[fen]
    if reason is not None and reason.startswith(EARLIER):
        earlier += 1
        continue
    if reason == PARSER:
        parser += 1
        continue
    compared += 1
    engine_refuses = reason is not None
    oracle_refuses = fen in king_flagged
    if engine_refuses != oracle_refuses:
        disagree.append((fen, reason, oracle_refuses))

print()
print(f"engine: {ENGINE}")
print(f"compared: {compared}  refused earlier by S208's classes: {earlier}  "
      f"refused by the parser or a field bound: {parser}  "
      f"disagreements: {len(disagree)}")
for fen, reason, oracle_refuses in disagree:
    print(f"  engine={reason or 'accepted':40s} oracle_king_flag={oracle_refuses} {fen}")
sys.exit(1 if disagree else 0)
