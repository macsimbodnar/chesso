#!/usr/bin/env python3
"""Drop fastchess's "PV continues after" blocks from a console stream.

    fastchess ... | tee console.txt | tools/trim_console.py [--summary] | tee report.txt

Copies stdin to stdout line by line, flushed per line so a watcher reading the
report sees a game finish when it finishes, except the four-line block
fastchess prints each time an engine's PV runs past a game-ending rule:

    Warning; PV continues after threefold repetition - move <m> from <engine>
    Info; <the engine's info line>
    Position; <fen or startpos>
    Moves; <every move of the game so far>

The rule is "threefold repetition" or "fifty-move rule". The block is dropped
whole and counted per rule and engine; with --summary the counts are printed
once the input ends, so the report keeps the number and loses the bulk. Every
other line passes, and so do the same three context lines when they follow any
other warning: "Incomplete mating PV" carries them too, and S238's pairs
reading counts that one per side.

WHY. S240's two rating reports were 59 MB each, 3340 games apiece, and 98.9 %
of the bytes were 55065 of these blocks -- 50827 after a threefold repetition,
4238 after the fifty-move rule -- each repeating the game's whole move list.
They came from the anchors' PVs, not chesso's (Leorik 2.4 31790, Leorik 2.1
23161, Blunder 8.5.5 113, chesso 1), and nothing read them. GitHub warned at
50 MB and refuses at 100. The count is what a reader may want; the move lists
are the PGN's, which stays with the run. DEC-235, S241.

Bytes in, bytes out: the stream is not decoded, so an engine name in any
encoding survives unchanged and the output is the input minus the blocks.
"""

import sys

PREFIX = b"Warning; PV continues after "
CONTEXT = (b"Info;", b"Position;", b"Moves;")
MOVE = b" - move "
FROM = b" from "


def rule_and_engine(warning):
    """The rule and the engine a "PV continues after" warning names.

    Either is b'unknown' when the line does not carry it.
    """
    rest = warning[len(PREFIX):].rstrip(b"\r\n")
    at = rest.rfind(FROM)
    engine = rest[at + len(FROM):].strip() if at >= 0 else b""
    head = rest[:at] if at >= 0 else rest
    cut = head.find(MOVE)
    rule = (head[:cut] if cut >= 0 else head).strip()
    return rule or b"unknown", engine or b"unknown"


def trim(lines, out, counts):
    """Copy `lines` to `out` minus the blocks; tally them in `counts`.

    Returns the number of lines dropped. `pending` is how many context lines
    the last dropped warning still owes; any other line clears it, so a block
    interleaved with another game's output loses at most its own lines.
    """
    dropped = 0
    pending = 0
    for line in lines:
        if line.startswith(PREFIX):
            key = rule_and_engine(line)
            counts[key] = counts.get(key, 0) + 1
            pending = 3
            dropped += 1
            continue
        if pending and line.startswith(CONTEXT):
            pending -= 1
            dropped += 1
            continue
        pending = 0
        out.write(line)
        out.flush()
    return dropped


def summary(counts, dropped):
    lines = [b"", b"=== fastchess console lines omitted from this report ==="]
    lines.append(b'"PV continues after <rule>" blocks -- the warning and its Info, '
                 b"Position and Moves lines -- per rule and engine:")
    if not counts:
        lines.append(b"  none")
    rule_width = max((len(rule) for rule, _ in counts), default=0)
    engine_width = max((len(engine) for _, engine in counts), default=0)
    ordered = sorted(counts.items(), key=lambda item: (-item[1], item[0]))
    for (rule, engine), n in ordered:
        lines.append(b"  " + rule.ljust(rule_width) + b"  " + engine.ljust(engine_width)
                     + b"  " + str(n).encode())
    blocks = sum(counts.values())
    lines.append(b"  total: %d blocks, %d lines" % (blocks, dropped))
    return b"\n".join(lines) + b"\n"


def main(argv):
    want_summary = False
    for arg in argv[1:]:
        if arg == "--summary":
            want_summary = True
        else:
            sys.stderr.write("trim_console.py: unknown argument %r\n" % arg)
            return 2
    counts = {}
    out = sys.stdout.buffer
    try:
        dropped = trim(sys.stdin.buffer, out, counts)
        if want_summary:
            out.write(summary(counts, dropped))
            out.flush()
    except BrokenPipeError:
        # The reader went away; there is nothing left to say and no one to say
        # it to. The pipeline's status is the reader's.
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
