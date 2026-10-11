#!/usr/bin/env python3
"""The bare-FEN relay, tools/bare_fen_relay.py (S272).

    python3 tests/test_bare_fen_relay.py CHESS_PYTHON RELAY ENGINE SESSION

CHESS_PYTHON is a python with python-chess, which the relay imports. When the
python running this cannot import it, the file re-runs itself under
CHESS_PYTHON with `-I`, as fastchess.sh launches the relay; when neither can,
it says so and exits 77, which tests/CMakeLists.txt registers as a skip: a
machine without python-chess cannot run the relay at all, and that is a
missing tool, not a failed relay. RELAY is the relay script, ENGINE the chesso
binary, SESSION
tests/assets/bare_fen_relay_session.txt: a command log fastchess sent one
engine, recorded, followed by two games sent a position at every ply
(adocs/data/S272_relay_session.py writes it and says which).

Properties:

  1. the session covers what the board has to get right: both base forms, a
     line with no moves, castling on both wings, an en-passant capture, a
     board whose FEN carries an en-passant square, a promotion and an
     under-promotion, a halfmove clock above zero and fullmove numbers past 1
     -- asserted first, so the checks below are not vacuous
  2. through the relay, a stub engine receives every line of the session in
     order: each `position` line as `position fen <six fields>` with no
     `moves`, every other line byte for byte
  3. what the stub prints comes back byte for byte
  4. every FEN the relay sent is the board chesso itself reaches from the
     original line, as chesso's own `fen` command prints it -- pieces, side,
     castling, en passant and both counters -- and chesso loads it back to the
     same string: python-chess and the engine are two implementations, so no
     expected FEN is written down here
  5. the kept board (each line pushes only its new moves) sends what a board
     built from scratch for every line sends
  6. a line the relay cannot read passes unchanged with a note on stderr and
     in the file BARE_FEN_RELAY_NOTES names; a
     failure half way through a line's moves does not leave the kept board
     ahead of the next line; and a new base after a line with no moves is not
     taken for a continuation of it
  7. the relay exits when the engine does, with the engine's status, while
     its own input is still open
  8. a reply comes back while the relay's input is still open: `uci` is
     answered `uciok`, and `isready` after a `position` `readyok`, before
     anything is closed. fastchess waits for each reply before it writes the
     next line, so a relay that holds the engine's output until it exits
     passes 1 to 7 and hangs a match at its first `uci`

Stdlib and python-chess, which the relay needs; the stub engine is this file's
interpreter running a short script. Registered in the fast suite.
"""

import importlib.util
import os
import select
import subprocess
import sys
import tempfile
import time

SKIP = 77
failures = []


def check(condition, message):
    if not condition:
        failures.append(message)
    return condition


STUB = r'''
import os, sys
log = open(sys.argv[1], "ab", buffering=0)
said = open(sys.argv[2], "ab", buffering=0)
def say(text):
    data = text.encode()
    said.write(data)
    sys.stdout.buffer.write(data)
    sys.stdout.buffer.flush()
searching = False
count = 0
for raw in sys.stdin.buffer:
    log.write(raw)
    word = raw.split()[:1]
    if word == [b"uci"]:
        say("id name relay-stub\nuciok\n")
    elif word == [b"isready"]:
        say("readyok\n")
    elif word == [b"go"]:
        count += 1
        if b"infinite" in raw:
            searching = True
        else:
            say(f"info string go {count}\nbestmove 0000\n")
    elif word == [b"stop"] and searching:
        searching = False
        say(f"info string stop {count}\nbestmove 0000\n")
    elif word == [b"quit"]:
        sys.exit(int(os.environ.get("STUB_EXIT", "0")))
'''


def load_relay(path):
    # Imported for property 5 only; no bytecode is left beside the tool.
    sys.dont_write_bytecode = True
    spec = importlib.util.spec_from_file_location("bare_fen_relay", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def run_relay(relay, stub, workdir, data, env_extra=None):
    log = os.path.join(workdir, "received")
    said = os.path.join(workdir, "said")
    for path in (log, said):
        open(path, "wb").close()
    env = dict(os.environ)
    env.pop("BARE_FEN_RELAY_NOTES", None)
    env.update(env_extra or {})
    try:
        done = subprocess.run([sys.executable, "-I", relay, sys.executable, stub, log, said],
                              input=data, capture_output=True, timeout=20, env=env)
    except subprocess.TimeoutExpired:
        # A relay that outlives its engine and its input; property 7 names it.
        done = subprocess.CompletedProcess([], None, b"", b"timed out after 20 s")
    return done, open(log, "rb").read(), open(said, "rb").read()


def engine_fens(engine, position_lines):
    """chesso's own `fen` after each line, one per line."""
    script = "".join(line + "\nfen\n" for line in position_lines) + "quit\n"
    out = subprocess.run([engine], input=script.encode(), capture_output=True,
                         timeout=60).stdout.decode().splitlines()
    return [line for line in out if line.count("/") == 7]


def coverage(position_lines):
    found = set()
    for line in position_lines:
        tokens = line.split()
        if tokens[1] == "startpos":
            found.add("startpos")
            board = chess.Board()
        else:
            found.add("fen")
            end = tokens.index("moves") if "moves" in tokens else len(tokens)
            board = chess.Board(" ".join(tokens[2:end]))
        if "moves" not in tokens:
            found.add("no moves")
            continue
        for uci in tokens[tokens.index("moves") + 1:]:
            move = chess.Move.from_uci(uci)
            if board.is_kingside_castling(move):
                found.add("kingside castling")
            if board.is_queenside_castling(move):
                found.add("queenside castling")
            if board.is_en_passant(move):
                found.add("en passant capture")
            if move.promotion:
                found.add("promotion" if move.promotion == chess.QUEEN else "under-promotion")
            board.push(move)
        if board.fen().split()[3] != "-":
            found.add("en-passant square")
        if board.halfmove_clock > 0:
            found.add("halfmove clock")
        if board.fullmove_number > 1:
            found.add("fullmove number")
    return found


def main(argv):
    if len(argv) != 5:
        print(__doc__.split("\n\n")[1], file=sys.stderr)
        return 2
    relay, engine, session = argv[2:]
    raw_lines = [line for line in open(session, "rb").read().splitlines(keepends=True)
                 if not line.startswith(b"#")]
    data = b"".join(raw_lines)
    originals = [line.decode().strip() for line in raw_lines if line.startswith(b"position")]

    # 1. The precondition.
    wanted = {"startpos", "fen", "no moves", "kingside castling", "queenside castling",
              "en passant capture", "en-passant square", "promotion", "under-promotion",
              "halfmove clock", "fullmove number"}
    missing = wanted - coverage(originals)
    check(not missing, f"1: the session does not cover {sorted(missing)}")

    with tempfile.TemporaryDirectory() as workdir:
        stub = os.path.join(workdir, "stub.py")
        with open(stub, "w") as handle:
            handle.write(STUB)

        done, received, said = run_relay(relay, stub, workdir, data)
        check(done.returncode == 0, f"2: the relay exited {done.returncode}: {done.stderr[-400:]!r}")
        check(done.stderr == b"", f"2: the relay wrote to stderr on a clean session: {done.stderr[:400]!r}")

        # 2. What the engine received.
        got = received.splitlines(keepends=True)
        check(len(got) == len(raw_lines),
              f"2: the stub received {len(got)} lines for the session's {len(raw_lines)}")
        sent_fens = []
        for number, (want, have) in enumerate(zip(raw_lines, got), 1):
            if want.startswith(b"position"):
                fields = have.decode().split()
                ok = (len(fields) == 8 and fields[:2] == ["position", "fen"]
                      and have.endswith(b"\n") and b"moves" not in have)
                if not check(ok, f"2: session line {number} reached the engine as {have!r}"):
                    break
                sent_fens.append(" ".join(fields[2:]))
            elif not check(have == want, f"2: session line {number} {want!r} reached the engine as {have!r}"):
                break

        # 3. What came back.
        check(said and done.stdout == said,
              f"3: the relay printed {len(done.stdout)} bytes for the stub's {len(said)}")
        replies, gos = said.count(b"bestmove"), data.count(b"\ngo ")
        check(replies == gos, f"3: {replies} bestmove replies for {gos} go lines")

        # 4. The board, against the engine's own.
        if check(len(sent_fens) == len(originals),
                 f"4: {len(sent_fens)} FENs sent for {len(originals)} position lines"):
            from_moves = engine_fens(engine, originals)
            loaded = engine_fens(engine, ["position fen " + fen for fen in sent_fens])
            check(len(from_moves) == len(originals) and len(loaded) == len(originals),
                  f"4: chesso answered {len(from_moves)} and {len(loaded)} fen lines for {len(originals)}")
            for line, sent, ours, back in zip(originals, sent_fens, from_moves, loaded):
                if not check(sent == ours, f"4: the relay sent [{sent}], chesso reaches [{ours}] from [{line[:90]}...]"):
                    break
                if not check(back == sent, f"4: chesso loads [{sent}] as [{back}]"):
                    break

        # 5. The kept board against a fresh one per line.
        module = load_relay(relay)
        for line, sent in zip(originals, sent_fens):
            fresh = module.Rewriter().fen_for(line)
            if not check(fresh == sent, f"5: a fresh board reads [{fresh}], the kept one sent [{sent}]"):
                break

        # 6. Lines the relay cannot read, and a failure half way through.
        fen_line = next(line for line in originals if line.split()[1] == "fen" and "moves" in line)
        odd = [b"position kiwipete\n",
               b"position startpos moves e2e4 e7e5\n",
               b"position startpos moves e2e4 e7e5 g1f3 zzzz\n",
               b"position startpos moves e2e4 e7e5 g1f3 b8c6\n",
               b"position fen not/a/fen w - - 0 1\n",
               b"position startpos\n",
               fen_line.encode() + b"\n",
               b"quit\n"]
        notes = os.path.join(workdir, "notes")
        done, received, _ = run_relay(relay, stub, workdir, b"".join(odd),
                                      {"BARE_FEN_RELAY_NOTES": notes})
        got = received.splitlines(keepends=True)
        check(len(got) == len(odd), f"6: the stub received {len(got)} lines for {len(odd)}")
        if len(got) == len(odd):
            for index in (0, 2, 4):
                check(got[index] == odd[index], f"6: {odd[index]!r} reached the engine as {got[index]!r}")
            # Line 3 follows a failure half way through line 2's moves, and
            # line 6 changes the base after a line with no moves, which the
            # kept board must not take for a continuation.
            for index in (1, 3, 6):
                sent = got[index].decode().split(maxsplit=2)
                reached = engine_fens(engine, [odd[index].decode().strip()])
                check(sent[:2] == ["position", "fen"] and [sent[2].strip()] == reached,
                      f"6: {odd[index][:60]!r} reached the engine as {got[index]!r}, chesso reaches {reached}")
        check(done.stderr.count(b"bare_fen_relay: passed unchanged") == 3,
              f"6: expected three notes on stderr, got {done.stderr!r}")
        noted = open(notes, "rb").read() if os.path.exists(notes) else b""
        check(noted.count(b"bare_fen_relay: passed unchanged") == 3 and noted == done.stderr,
              f"6: the notes file holds {noted!r}, stderr {done.stderr!r}")

        # 7. The engine leaves first, its status kept; the relay's input stays open.
        for status in ("0", "3"):
            proc = subprocess.Popen([sys.executable, "-I", relay, sys.executable, stub,
                                     os.path.join(workdir, "received"), os.path.join(workdir, "said")],
                                    stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                    env=dict(os.environ, STUB_EXIT=status))
            proc.stdin.write(b"uci\nquit\n")
            proc.stdin.flush()
            start = time.monotonic()
            try:
                code = proc.wait(timeout=10)
            except subprocess.TimeoutExpired:
                proc.kill()
                code = None
            check(code == int(status),
                  f"7: the stub exited {status} and the relay {'is still running' if code is None else f'exited {code}'}"
                  f" after {time.monotonic() - start:.1f} s with its input open")
            proc.stdin.close()
            proc.stdout.close()

        # 8. Replies while the input is open, each awaited before the next
        # line is written, as fastchess does.
        proc = subprocess.Popen([sys.executable, "-I", relay, sys.executable, stub,
                                 os.path.join(workdir, "received"), os.path.join(workdir, "said")],
                                stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        pending = b""

        def answered(send, want):
            nonlocal pending
            proc.stdin.write(send)
            proc.stdin.flush()
            deadline = time.monotonic() + 5
            while want not in pending:
                left = deadline - time.monotonic()
                if left <= 0 or not select.select([proc.stdout], [], [], left)[0]:
                    return False
                chunk = os.read(proc.stdout.fileno(), 65536)
                if not chunk:
                    return False
                pending += chunk
            pending = pending[pending.index(want) + len(want):]
            return True

        for send, want in ((b"uci\n", b"uciok\n"),
                           (b"position startpos moves e2e4 e7e5\nisready\n", b"readyok\n"),
                           (b"go wtime 1000 btime 1000\n", b"bestmove 0000\n")):
            if not check(answered(send, want),
                         f"8: {send!r} was not answered {want!r} within 5 s with the relay's input open"):
                break
        proc.stdin.close()
        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
        proc.stdout.close()

    for message in failures:
        print("FAIL:", message, file=sys.stderr)
    if failures:
        return 1
    print(f"test_bare_fen_relay: 8 properties hold over {len(raw_lines)} session lines, "
          f"{len(originals)} positions checked against chesso's own fen")
    return 0


def chess_or_skip(argv):
    """Returns once python-chess imports here.

    Re-runs this file under CHESS_PYTHON when only it has python-chess, and
    exits SKIP with the reason when neither does.
    """
    try:
        import chess  # noqa: F401 -- the probe; main's import is the use
        return
    except ImportError:
        pass
    python = argv[1] if len(argv) > 1 else ""
    try:
        ok = subprocess.run([python, "-I", "-c", "import chess"], capture_output=True,
                            timeout=60).returncode == 0
    except (OSError, subprocess.TimeoutExpired):
        ok = False
    if not ok:
        print(f"test_bare_fen_relay: SKIPPED -- [{python}] cannot import python-chess, "
              "which the relay needs; configure with -DCHESSO_CHESS_PYTHON=<a python that has it>")
        sys.exit(SKIP)
    os.execv(python, [python, "-I", os.path.abspath(__file__)] + argv[1:])


if __name__ == "__main__":
    chess_or_skip(sys.argv)
    import chess
    sys.exit(main(sys.argv))
