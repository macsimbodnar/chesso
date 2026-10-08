"""F04 probe: replay recorded games and search one side's positions at a fixed
depth, sending the position three ways, to count what a cleared table costs.

  base   position fen <book fen> moves <every move so far>   (fastchess, python-chess)
  bare   position fen <current fen>                          (a client that sends the board)
  irrev  position fen <fen after the last irreversible move> moves <moves since>

usage: S271_replay.py ENGINE PGN NGAMES DEPTH HASH MODE OUT_JSON

2026-10-08_performance-F04's investigation, read in S271_replay.md. The side
searched is the side to move at the book position; every second ply of the
first 120 is searched, after `ucinewgame` at the start of each game. Node
counts are deterministic, so runs may share the machine.
"""
import json
import subprocess
import sys

import chess
import chess.pgn

engine_path, pgn_path, n_games, depth, hash_mb, mode, out_path = sys.argv[1:8]
n_games, depth, hash_mb = int(n_games), int(depth), int(hash_mb)
MAX_PLIES = 120

proc = subprocess.Popen([engine_path], stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE, text=True, bufsize=1)


def send(line):
    proc.stdin.write(line + "\n")
    proc.stdin.flush()


def wait_for(prefix):
    lines = []
    while True:
        line = proc.stdout.readline()
        if not line:
            raise RuntimeError("engine closed its output")
        lines.append(line)
        if line.startswith(prefix):
            return lines


send("uci")
wait_for("uciok")
send(f"setoption name Hash value {hash_mb}")
send("isready")
wait_for("readyok")

records = []
with open(pgn_path) as pgn:
    for game_index in range(n_games):
        game = chess.pgn.read_game(pgn)
        if game is None:
            break
        board = game.board()
        book_fen = board.fen()
        moves = list(game.mainline_moves())[:MAX_PLIES]

        # Positions along the game, and for each ply the ply at which the last
        # irreversible move left the board (the base an `irrev` client sends).
        fens, irrev_base = [], []
        last_irrev = 0
        b = board.copy()
        for ply in range(len(moves) + 1):
            if ply > 0 and b.halfmove_clock == 0:
                last_irrev = ply
            fens.append(b.fen())
            irrev_base.append(last_irrev)
            if ply < len(moves):
                b.push(moves[ply])

        send("ucinewgame")
        send("isready")
        wait_for("readyok")

        uci = [m.uci() for m in moves]
        for ply in range(0, len(moves), 2):
            if mode == "base":
                cmd = f"position fen {book_fen}" + (
                    " moves " + " ".join(uci[:ply]) if ply else "")
            elif mode == "bare":
                cmd = f"position fen {fens[ply]}"
            elif mode == "irrev":
                j = irrev_base[ply]
                cmd = f"position fen {fens[j]}" + (
                    " moves " + " ".join(uci[j:ply]) if ply > j else "")
            else:
                raise SystemExit(f"unknown mode {mode}")

            send(cmd)
            send(f"go depth {depth}")
            out = wait_for("bestmove")
            nodes = 0
            for line in out:
                parts = line.split()
                if parts and parts[0] == "info" and "nodes" in parts:
                    nodes = max(nodes, int(parts[parts.index("nodes") + 1]))
            records.append({"game": game_index, "ply": ply, "nodes": nodes,
                            "best": out[-1].split()[1]})

send("quit")
proc.wait()

with open(out_path, "w") as f:
    json.dump(records, f)
print(f"{mode} hash={hash_mb} positions={len(records)} "
      f"nodes={sum(r['nodes'] for r in records)}")
