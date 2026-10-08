"""Does an engine keep its table across a new base FEN? Search A, then the
board two plies later sent as a bare FEN, with and without ucinewgame between."""
import subprocess, sys
import chess
eng = sys.argv[1]
A = "r1bq1rk1/pp2bppp/2n2n2/3p4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 10"

def run(clear_between):
    p = subprocess.Popen([eng], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
    def send(s): p.stdin.write(s + "\n"); p.stdin.flush()
    def until(pre):
        out = []
        while True:
            l = p.stdout.readline()
            out.append(l)
            if l.startswith(pre): return out
    send("uci"); until("uciok")
    send("setoption name Threads value 1"); send("setoption name Hash value 64")
    send("ucinewgame"); send("isready"); until("readyok")
    send(f"position fen {A}"); send("go depth 16"); out = until("bestmove")
    pv = None
    for l in out:
        if " pv " in l: pv = l.split(" pv ")[1].split()
    b = chess.Board(A)
    for m in pv[:2]: b.push_uci(m)
    if clear_between:
        send("ucinewgame"); send("isready"); until("readyok")
    send(f"position fen {b.fen()}"); send("go depth 16"); out = until("bestmove")
    nodes = max(int(l.split()[l.split().index("nodes")+1]) for l in out if l.startswith("info") and " nodes " in l)
    send("quit"); p.wait()
    return nodes

kept, cleared = run(False), run(True)
print(f"{eng.split('/')[-1]}: second search nodes, bare FEN after A: {kept}; with ucinewgame between: {cleared}")
