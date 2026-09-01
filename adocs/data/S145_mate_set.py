#!/usr/bin/env python3
"""S145. Construct and verify the mate-safety set the RFP floor is measured on.

    ~/.venv/chess/bin/python adocs/data/S145_mate_set.py generate
    ~/.venv/chess/bin/python adocs/data/S145_mate_set.py verify
    ~/.venv/chess/bin/python adocs/data/S145_mate_set.py emit-cpp

`generate` writes adocs/data/S145_mate_set.tsv, `verify` re-checks that file
from scratch, `emit-cpp` prints the table tests/test_engine.cpp holds.

WHY A CONSTRUCTED SET AND NOT A SAMPLED ONE. S145 measured it: of 191 positions
in a 6347-position sample where chesso says the side to move is mated within 6,
one has a non-negative score for the mated side and the median is -1093. The
hazard reverse futility walks into needs the mated side to be *ahead*, which
does not occur in play often enough to sample. There is no sample size that
fixes that, so the positions are built.

THE HAZARD, AND THEREFORE THE SHAPE. Reverse futility returns a static score
instead of searching when `static_score - RFP_MARGIN * depth >= beta`, at a
non-PV node that is not in check and at ply >= RFP_MIN_PLY. A static score is
never a mate score, so a node whose true value is "mated" can fail high on
material. For that to happen the node must be

  1. a forced loss for the side to move,
  2. materially ahead, so the static score is high,
  3. not in check, or the rule is already off,

and it must sit at a ply the floor does not exempt. Property 1 and 2 together
are what no game position offers. So each position here is a forced mate for a
small force against a large one, and the defender nodes on the mating line are
the nodes under test. A mate in m puts defender nodes at plies 1, 3, ...,
2m - 3, which is why the set spans mate distances 2 to 5 rather than 2 alone:
before this, every mate case in the suite was a mate in two and exercised ply 1
and nothing else.

CONSTRUCTION, generalising what S033 did by hand. A family fixes a frozen clump
for the defender -- material it owns and cannot move -- and the mobile pieces
are then placed over the free squares:

  * The defender's back rank carries pieces that block each other and are
    blocked from the front by the defender's own pawns.
  * Those pawns are blocked by attacker pawns on files that leave no pawn
    capture available to either side, which is why the pawn files are
    non-adjacent: a pawn on a7 facing a pawn on a6 captures on b6, so b6 is
    left empty and a6/c6/e6 is a frozen wall where a6/b6/c6 is not.
  * The attacker pawns also take squares away from the defender king, which is
    what makes the pocket a pocket.

Immobility is a speed property and not a correctness one: it keeps the branching
at defender nodes near one, which is what makes exhaustive proof affordable at
mate in five. Nothing here trusts it. The mate distance is proved by search.

ONE MOTIF, AND THAT IS WHAT THIS SCRIPT PRODUCES. S155. Everything above
constrains the shape so hard that the 48 positions come out as a single motif:
S155_motif_census.py counts two material signatures over the tracked TSV, one
the colour mirror of the other, a lone queen as the mating force in 48 of 48,
`lead` 760 in 48 of 48, and eight family labels that are one geometry under two
file shifts, a mirror and a colour swap. Broad in mate distance, narrow in
shape. So the gate this feeds cannot catch a rule that hides a back-rank mate,
a smothered mate or any knight mate, a king hunt, an open-line mate, a
promotion mate -- `S155_motif_census.py --moves` counts 1292 legal moves over
the roots and defender nodes and 0 pawn moves among them -- or a mate in a
position with a realistic material balance.
A second motif with a different mating piece is worth building and is S168; it
belongs beside these families here, not in place of them.

TWO ORACLES, and neither of them is chesso.

  * `stockfish` proposes. It is fast, it is run at a fixed depth over sampled
    placements, and its answer is a filter and never a verdict.
  * An AND/OR search over `python-chess` proves. It is iterative-deepening in
    the mate distance, so the first distance that succeeds is exact: every
    shorter distance was refuted by exhausting every attacker move against
    every defender reply. That is a proof and not a search result.

Where the material allows it a Syzygy WDL probe is a third check, but the frozen
clump puts every constructed position far past five pieces, so the probe only
ever fires on the seeds. Recorded here because the accepts names it: the reason
it contributes nothing is the piece count, not a missing tablebase.

DEC-016. Nothing is copied. There is no public-domain collection of forced mates
to copy from in any case -- S145 checked -- and every free-licensed one at scale
is selected by what another engine's search could solve, which the rule bars
whatever the licence says.
"""

import argparse
import os
import random
import sys

import chess
import chess.engine


REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TSV = os.path.join(REPO, "adocs", "data", "S145_mate_set.tsv")

STOCKFISH = "/usr/games/stockfish"
STOCKFISH_OPTIONS = {"Threads": 1, "Hash": 16}

# The budget `verify` gives stockfish, per position and in its own process. Ten
# times the filter's, because verification cannot borrow a warm hash: the filter
# runs thousands of positions through one process and its answers depend on that
# sequence, which is fine for a proposer and useless for a check. Measured over
# the 48 positions, one fresh process each: 41 of 48 corroborate at 150000
# nodes, 45 at 1000000, 46 at 4000000.
VERIFY_NODES = 4_000_000

# The filter budget, in nodes and not in plies. Measured: at `depth=14` a few
# placements in every hundred cost seconds, because the defender's paralysed
# army makes stockfish's own pruning ineffective and the iteration runs long. A
# node limit bounds the worst case instead of the typical one -- 300 placements
# in 21.4 s, worst 0.14 s, against a depth limit that did not finish 300 in 95 s
# -- and 150000 nodes reaches past depth 25 here, far beyond the nine plies a
# mate in five needs. Node-limited and single-threaded is also reproducible,
# which a time or depth limit on a loaded machine is not.
FILTER_NODES = 150_000

# The exhaustive proof only ever runs on what the filter proposed, so the cap is
# a guard against a pathological placement rather than a budget. A node here is
# one make/unmake in the AND/OR search. The worst accepted candidate measured
# 232099 nodes, so this is two orders of slack; at 40 million a single placement
# in the `shift2` family ran for more than ten minutes before it was cut.
PROOF_NODE_CAP = 4_000_000

# Standard material, for the sampler's own filter and for the TSV's record of
# how far ahead the mated side is. The engine's own evaluate() is the number
# that decides whether the static cutoff is reachable, and it is asserted in
# tests/test_engine.cpp rather than restated here: this script must not carry a
# second copy of the piece values.
MATERIAL = {
    chess.PAWN: 100,
    chess.KNIGHT: 320,
    chess.BISHOP: 330,
    chess.ROOK: 500,
    chess.QUEEN: 900,
    chess.KING: 0,
}

MIN_MATERIAL_LEAD = 400
MATE_MIN, MATE_MAX = 2, 5


# ---------------------------------------------------------------- the families

# One family is a frozen clump plus the mobile pieces placed over what is left.
# The base layout is stated once and the variants are derived, so a file shift,
# a mirror or a colour swap cannot disagree with it by transcription.
#
# The base, with the wall on the a-file side and the defender playing Black:
#
#   r b r b . . . .     a8 rook   blocked by a7 and b8
#   p . p . p . . .     b8 bishop blocked by a7 and c7
#   P . P . P . . .     c8 rook   blocked by b8, d8 and c7
#   . . . . . . . .     d8 bishop blocked by c7 and e7
#
# The attacker pawns on a6, c6 and e6 block the defender pawns and take b7, d7
# and f7 away from the defender king. Neither side has a pawn capture: the
# files are two apart, so every square a pawn attacks is empty and stays empty.
BASE_WALL = {
    "a8": "r", "b8": "b", "c8": "r", "d8": "b",
    "a7": "p", "c7": "p", "e7": "p",
    "a6": "P", "c6": "P", "e6": "P",
}

# Mobile material: the attacker's king and queen against the defender's king.
# A queen is the whole attacking force on purpose -- the mated side has to be
# ahead, and a rook pair or a queen and a rook narrow the lead for nothing.
BASE_MOBILE = ["K", "Q", "k"]


def _shift_files(layout, by):
    out = {}
    for square, piece in layout.items():
        file_index = chess.square_file(chess.parse_square(square)) + by
        if not 0 <= file_index <= 7:
            return None
        rank_index = chess.square_rank(chess.parse_square(square))
        out[chess.square_name(chess.square(file_index, rank_index))] = piece
    return out


def _flip_files(layout):
    out = {}
    for square, piece in layout.items():
        parsed = chess.parse_square(square)
        flipped = chess.square(7 - chess.square_file(parsed), chess.square_rank(parsed))
        out[chess.square_name(flipped)] = piece
    return out


def _swap_colours(layout):
    out = {}
    for square, piece in layout.items():
        parsed = chess.parse_square(square)
        mirrored = chess.square(chess.square_file(parsed), 7 - chess.square_rank(parsed))
        out[chess.square_name(mirrored)] = piece.swapcase()
    return out


def families():
    """Every family, derived from the one base layout.

    Two file shifts, then the horizontal flip of each, then the colour swap of
    all four. Eight families, and the colour swap is not decoration: it is the
    one check that a mate-safety result is not an artefact of which side the
    tables are written from.
    """
    out = []

    for shift in (0, 2):
        shifted = _shift_files(BASE_WALL, shift)
        if shifted is None:
            continue

        for flip in (False, True):
            wall = _flip_files(shifted) if flip else shifted

            for swap in (False, True):
                layout = _swap_colours(wall) if swap else wall
                mobile = [p.swapcase() for p in BASE_MOBILE] if swap else BASE_MOBILE
                name = "shift%d%s%s" % (shift, "_flip" if flip else "", "_black" if swap else "")
                out.append({"name": name, "wall": layout, "mobile": mobile,
                            "attacker": chess.BLACK if swap else chess.WHITE})

    return out


def build(family, squares):
    """The family's frozen clump plus its mobile pieces on `squares`."""
    board = chess.Board.empty()

    for square, piece in family["wall"].items():
        board.set_piece_at(chess.parse_square(square), chess.Piece.from_symbol(piece))

    for piece, square in zip(family["mobile"], squares):
        if board.piece_at(square) is not None:
            return None
        board.set_piece_at(square, chess.Piece.from_symbol(piece))

    board.turn = family["attacker"]
    board.clear_stack()

    return board


def material_lead(board, defender):
    """How far the mated side is ahead, in standard centipawns."""
    lead = 0

    for square, piece in board.piece_map().items():
        value = MATERIAL[piece.piece_type]
        lead += value if piece.color == defender else -value

    return lead


# ------------------------------------------------- the exhaustive proof, oracle


class BudgetExceeded(Exception):
    """The AND/OR search hit PROOF_NODE_CAP. Never seen on a filtered
    candidate; the cap exists so a pathological placement cannot hang the run
    instead of being reported."""


def _ordered(board):
    """Checks first, then captures. Ordering only moves the cost of the proof
    around -- an AND/OR search returns the same answer in any order -- and
    checks first is what makes a mating line cheap to find."""
    return sorted(board.legal_moves,
                  key=lambda m: (not board.gives_check(m), not board.is_capture(m)))


def _or_mate(board, plies, memo, budget):
    """The side to move forces mate within `plies` plies."""
    if plies <= 0:
        return False

    key = (board._transposition_key(), plies, True)
    hit = memo.get(key)
    if hit is not None:
        return hit

    found = False

    for move in _ordered(board):
        budget[0] -= 1
        if budget[0] <= 0:
            raise BudgetExceeded()

        board.push(move)
        try:
            found = board.is_checkmate() or (
                plies >= 3 and _and_mate(board, plies - 1, memo, budget))
        finally:
            board.pop()

        if found:
            break

    memo[key] = found
    return found


def _and_mate(board, plies, memo, budget):
    """The side to move is mated within `plies` plies whatever it plays.

    No legal move is the terminal case and it is decided by the check test, not
    by the move count: no moves and in check is mate, no moves and not in check
    is a stalemate and refutes the line. That distinction is the one the whole
    construction has to get right, because a mating net that stalemates is the
    commonest way a hand-built mate position is wrong.
    """
    moves = list(board.legal_moves)

    if not moves:
        return board.is_check()

    if plies <= 0:
        return False

    key = (board._transposition_key(), plies, False)
    hit = memo.get(key)
    if hit is not None:
        return hit

    mated = True

    for move in moves:
        budget[0] -= 1
        if budget[0] <= 0:
            raise BudgetExceeded()

        board.push(move)
        try:
            mated = _or_mate(board, plies - 1, memo, budget)
        finally:
            board.pop()

        if not mated:
            break

    memo[key] = mated
    return mated


def exact_mate_distance(board, max_moves=MATE_MAX):
    """The exact number of moves the side to move needs to force mate, or None.

    Iterative deepening in the distance is what makes the answer exact rather
    than an upper bound: distance m is only returned after every distance below
    it was refuted by exhausting the tree at that depth.
    """
    memo = {}
    budget = [PROOF_NODE_CAP]

    for moves in range(1, max_moves + 1):
        if _or_mate(board, 2 * moves - 1, memo, budget):
            return moves, PROOF_NODE_CAP - budget[0]

    return None, PROOF_NODE_CAP - budget[0]


def is_quiet(board, move):
    """Neither a check nor a capture.

    A checking move is refused because the node after it is in check, where
    reverse futility is already forbidden and the case would prove nothing. A
    capture is refused because this engine orders and prunes captures
    differently, so a set that mixes the two cannot attribute a failure.
    """
    return not board.gives_check(move) and not board.is_capture(move)


def mating_moves(board, moves_left, memo=None, budget=None, quiet_only=False):
    """Every move at this node that still forces mate in `moves_left`.

    `quiet_only` is not a filter applied to the result -- it skips the proof for
    the moves it rejects, which is where nearly all of this generator's time
    went. At mate in five the root has around thirty legal moves and proving one
    of them is itself a mate in four.
    """
    memo = {} if memo is None else memo
    budget = [PROOF_NODE_CAP] if budget is None else budget
    out = []

    for move in board.legal_moves:
        if quiet_only and not is_quiet(board, move):
            continue

        board.push(move)
        try:
            if board.is_checkmate():
                ok = moves_left == 1
            else:
                ok = moves_left >= 2 and _and_mate(
                    board, 2 * moves_left - 2, memo, budget)
        finally:
            board.pop()

        if ok:
            out.append(move)

    return out


# ------------------------------------------------------- the quiet requirement


def quiet_proof(board, moves_left, memo, budget):
    """Every attacker move on the proof tree is quiet, except the mate itself.

    Stronger than "the key move is quiet", and deliberately: the engine
    searches every defender reply, so a branch whose continuation has to be a
    check is a branch where the defender node is in check and reverse futility
    was never allowed to fire there. A position where that is true of some
    branch would test the guard on part of its tree and nothing on the rest.

    A checking or capturing attacker move is refused. A capture is refused for
    the same reason a check is: this engine orders and prunes captures
    differently, and a set that mixes the two cannot attribute a failure.
    """
    if moves_left == 1:
        return bool(mating_moves(board, 1, memo, budget))

    for move in mating_moves(board, moves_left, memo, budget, quiet_only=True):
        board.push(move)
        try:
            survives = all(
                _push_pop(board, reply,
                          lambda: quiet_proof(board, moves_left - 1, memo, budget))
                for reply in board.legal_moves)
        finally:
            board.pop()

        if survives:
            return True

    return False


def _push_pop(board, move, thunk):
    board.push(move)
    try:
        return thunk()
    finally:
        board.pop()


def quiet_key(board, moves_left):
    """The move the representative line takes at the root, in UCI."""
    quiet = mating_moves(board, moves_left, quiet_only=True)

    return quiet[0].uci() if quiet else None


def representative_line(board, moves_left):
    """One line through the quiet proof tree, defender nodes recorded.

    The attacker takes the first quiet mating move at each node and the
    defender the first legal reply, so the line is a function of the position
    and the move generator alone. Returns the defender node FENs in ply order:
    index 0 is the node at ply 1, index 1 the node at ply 3, and so on.
    """
    memo = {}
    budget = [PROOF_NODE_CAP]

    walk = board.copy()
    nodes = []

    for remaining in range(moves_left, 1, -1):
        quiet = mating_moves(walk, remaining, memo, budget, quiet_only=True)
        if not quiet:
            return None

        walk.push(quiet[0])
        nodes.append(walk.fen())

        replies = list(walk.legal_moves)
        if not replies:
            return None

        walk.push(replies[0])

    return nodes


# ------------------------------------------------------------- the sampler


# Sampling and not enumeration. The three mobile pieces over 54 free squares is
# 148824 placements per family and 1.2 million over the eight, and the useful
# ones are a thin slice of that. A seeded sample with a stated try budget gets
# the same set back on a re-run and costs minutes instead of hours.
SEED = 145

# The budget is what a family that *cannot* fill its buckets costs, not what a
# family that can costs. Family `shift0` fills all four distances in about 1200
# tries; at 60000 a family with no all-quiet mate in five at all ground through
# 12000 stockfish calls at 71 ms and spent fourteen minutes finding nothing.
# 6000 caps that at about a minute and a half, and a family that comes up short
# says so in its own report line instead of stalling the run.
TRIES_PER_FAMILY = 6_000
PER_FAMILY_PER_DISTANCE = 2

# The defender's branching at every AND node, bounded at the root. Without this
# the exhaustive proof at mate in five is unaffordable and the position is
# unlikely to be a forced mate anyway.
MAX_DEFENDER_MOVES = 3


def defender_mobility(board):
    """How many legal moves the side not to move would have."""
    probe = board.copy()
    probe.turn = not board.turn

    return len(list(probe.legal_moves))


def cheap_filter(board):
    """Everything that can be decided without a search."""
    if not board.is_valid():
        return False

    if board.is_check() or board.is_game_over():
        return False

    return 1 <= defender_mobility(board) <= MAX_DEFENDER_MOVES


def sample(family, index, engine, report):
    """Positions from one family, keyed by mate distance.

    The seed is the family's position in the list and not a hash of its name:
    Python salts string hashing per process, so a name-seeded run would draw a
    different sample every time and the file would not be reproducible.
    """
    rng = random.Random(SEED + index)
    free = [s for s in chess.SQUARES
            if chess.square_name(s) not in family["wall"]]

    accepted = {}
    filtered = 0
    proposed = 0

    for _ in range(TRIES_PER_FAMILY):
        if all(len(accepted.get(m, [])) >= PER_FAMILY_PER_DISTANCE
               for m in range(MATE_MIN, MATE_MAX + 1)):
            break

        board = build(family, rng.sample(free, len(family["mobile"])))
        if board is None or not cheap_filter(board):
            continue

        filtered += 1

        # Stockfish proposes. Its answer is never recorded as the distance.
        info = engine.analyse(board, chess.engine.Limit(nodes=FILTER_NODES))
        score = info["score"].relative
        if not score.is_mate():
            continue

        guess = score.mate()
        if guess is None or not MATE_MIN <= guess <= MATE_MAX:
            continue

        if len(accepted.get(guess, [])) >= PER_FAMILY_PER_DISTANCE:
            continue

        proposed += 1

        # And the proof decides.
        try:
            distance, nodes = exact_mate_distance(board)
        except BudgetExceeded:
            report("  %s: past the %d node cap, skipped: %s"
                   % (family["name"], PROOF_NODE_CAP, board.fen()))
            continue

        if distance is None:
            report("  %s: stockfish said mate %d, the proof found none: %s"
                   % (family["name"], guess, board.fen()))
            continue

        if distance != guess:
            report("  %s: stockfish said mate %d, the proof says %d: %s"
                   % (family["name"], guess, distance, board.fen()))

        if not MATE_MIN <= distance <= MATE_MAX:
            continue

        # The order is a cost decision and not a logical one: one path through
        # the tree is cheap and refuses most candidates, the whole-tree
        # requirement is expensive and only runs on what survives.
        try:
            nodes_on = representative_line(board, distance)
            if nodes_on is None or len(nodes_on) != distance - 1:
                continue

            memo, budget = {}, [PROOF_NODE_CAP]
            if not quiet_proof(board, distance, memo, budget):
                continue
        except BudgetExceeded:
            report("  %s: quiet proof past the %d node cap, skipped: %s"
                   % (family["name"], PROOF_NODE_CAP, board.fen()))
            continue

        key = quiet_key(board, distance)
        if key is None:
            continue

        lead = material_lead(board, not family["attacker"])
        if lead < MIN_MATERIAL_LEAD:
            continue

        accepted.setdefault(distance, []).append(
            {"fen": board.fen(), "distance": distance, "family": family["name"],
             "lead": lead, "proof_nodes": nodes, "defender_nodes": nodes_on,
             "key": key})

    report("  %s: %d placements past the cheap filter, %d proposed, %s accepted"
           % (family["name"], filtered, proposed,
              ", ".join("mate %d x%d" % (m, len(v))
                        for m, v in sorted(accepted.items())) or "none"))

    return accepted


# ------------------------------------------------------------------- the file


COLUMNS = ["fen", "distance", "family", "lead", "proof_nodes", "key",
           "defender_nodes"]


def write_tsv(rows, path=TSV):
    with open(path, "w") as out:
        out.write("# S145 mate-safety set. Regenerate with:\n")
        out.write("#   ~/.venv/chess/bin/python adocs/data/S145_mate_set.py generate\n")
        out.write("# Re-check an existing file, both oracles, from scratch:\n")
        out.write("#   ~/.venv/chess/bin/python adocs/data/S145_mate_set.py verify\n")
        out.write("# defender_nodes are the positions at plies 1, 3, ... on the\n")
        out.write("# quiet mating line, which are the nodes the guard has to search.\n")
        out.write("\t".join(COLUMNS) + "\n")

        for row in rows:
            out.write("\t".join([
                row["fen"], str(row["distance"]), row["family"],
                str(row["lead"]), str(row["proof_nodes"]), row["key"],
                "|".join(row["defender_nodes"])]) + "\n")


def read_tsv(path=TSV):
    rows = []

    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or not line.strip():
                continue
            field = line.rstrip("\n").split("\t")
            if field[0] == "fen":
                continue
            rows.append({"fen": field[0], "distance": int(field[1]),
                         "family": field[2], "lead": int(field[3]),
                         "proof_nodes": int(field[4]), "key": field[5],
                         "defender_nodes": field[6].split("|") if field[6] else []})

    return rows


# --------------------------------------------------------------- the commands


def cmd_generate(args):
    rows = []

    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        engine.configure(STOCKFISH_OPTIONS)

        for index, family in enumerate(families()):
            found = sample(family, index, engine, lambda m: print(m, flush=True))

            for distance in sorted(found):
                rows.extend(found[distance])

    rows.sort(key=lambda r: (r["distance"], r["fen"]))
    write_tsv(rows, args.out)

    print("\n%d positions written to %s" % (len(rows), args.out))
    for distance in range(MATE_MIN, MATE_MAX + 1):
        count = sum(1 for r in rows if r["distance"] == distance)
        print("  mate in %d: %d, defender nodes at plies %s"
              % (distance, count,
                 ", ".join(str(2 * i + 1) for i in range(distance - 1))))

    return 0 if len(rows) >= 20 else 1


def cmd_verify(args):
    rows = read_tsv(args.out)
    failures = 0
    corroborated = 0

    for row in rows:
        # One process per position. Node-limited stockfish is reproducible only
        # within an identical call sequence, so a shared process makes the answer
        # a function of what ran before it.
        with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
            engine.configure(STOCKFISH_OPTIONS)

            board = chess.Board(row["fen"])
            where = row["fen"]

            if not board.is_valid():
                print("FAIL not a legal position: %s" % where)
                failures += 1
                continue

            # Reachability, the same property tests/test_helpers.hpp checks
            # through a null move: the side not to move must not be in check.
            probe = board.copy()
            probe.turn = not board.turn
            if probe.is_check():
                print("FAIL the side not to move is in check: %s" % where)
                failures += 1

            try:
                distance, _ = exact_mate_distance(board)
            except BudgetExceeded:
                print("FAIL proof past the %d node cap: %s" % (PROOF_NODE_CAP, where))
                failures += 1
                continue

            if distance != row["distance"]:
                print("FAIL proof says mate %s, file says %d: %s"
                      % (distance, row["distance"], where))
                failures += 1

            # Stockfish CORROBORATES; it does not decide, and the asymmetry is
            # the whole point. The enumeration's claim is "mate in exactly m",
            # and it refuted every shorter distance by exhausting the tree. So
            # stockfish reporting no mate, or a longer one, bounds stockfish's
            # search and says nothing about the claim -- while stockfish
            # reporting a *shorter* mate would falsify the proof outright, and
            # that is the only direction treated as a failure.
            #
            # It is not a formality here. Two of the 48 do not corroborate at
            # 4000000 nodes: one reads +1879 with no mate at all and one reads
            # mate 6 against a proved 5. Both were re-proved independently and
            # every shorter distance re-refuted. The reason is visible in the
            # construction -- the defender's army is frozen, which is a position
            # class stockfish's network scores badly wrong (it calls the
            # attacking side better while it is 760 behind on material), so its
            # ordering is poor exactly here.
            score = engine.analyse(
                board, chess.engine.Limit(nodes=VERIFY_NODES))["score"].relative

            if score.is_mate() and (score.mate() or 0) > 0:
                if score.mate() < row["distance"]:
                    print("FAIL stockfish found mate %d where the proof refuted "
                          "every distance below %d: %s"
                          % (score.mate(), row["distance"], where))
                    failures += 1
                elif score.mate() == row["distance"]:
                    corroborated += 1

            memo, budget = {}, [PROOF_NODE_CAP]
            if not quiet_proof(board, row["distance"], memo, budget):
                print("FAIL no all-quiet mating strategy: %s" % where)
                failures += 1

            nodes = representative_line(board, row["distance"])
            if nodes != row["defender_nodes"]:
                print("FAIL the recorded defender nodes do not reproduce: %s" % where)
                failures += 1

            for node in row["defender_nodes"]:
                at = chess.Board(node)
                if at.is_check():
                    print("FAIL defender node is in check: %s" % node)
                    failures += 1

            lead = material_lead(board, not board.turn)
            if lead != row["lead"] or lead < MIN_MATERIAL_LEAD:
                print("FAIL material lead %d, file says %d: %s"
                      % (lead, row["lead"], where))
                failures += 1

    spread = sorted({r["distance"] for r in rows})
    counts = sorted({chess.Board(r["fen"]).occupied.bit_count() for r in rows})
    print("\n%d positions, %d checks failed, mate distances %s" %
          (len(rows), failures, spread))

    # The third oracle the accepts names, and why it contributes nothing here.
    # Reported as a piece count rather than asserted in prose: `~/syzygy/` holds
    # 3-4-5 man, and a position with the frozen clump in it cannot be probed at
    # any size on this machine.
    print("piece counts %s, so the Syzygy 3-4-5 tables cannot probe any of them"
          % counts)
    print("stockfish corroborates %d of %d at %d nodes; the rest are stated in "
          "this file's verify comment" % (corroborated, len(rows), VERIFY_NODES))

    if len(rows) < 20:
        print("FAIL fewer than 20 positions")
        failures += 1

    if spread != list(range(MATE_MIN, MATE_MAX + 1)):
        print("FAIL the set does not span mate %d to %d" % (MATE_MIN, MATE_MAX))
        failures += 1

    return 1 if failures else 0


def cmd_emit_cpp(args):
    rows = read_tsv(args.out)

    print("    // clang-format off")
    print("    // Generated by `S145_mate_set.py emit-cpp` from"
          " adocs/data/S145_mate_set.tsv.")
    print("    // Do not edit here: edit the script, regenerate, re-emit.")
    print("    const std::vector<mate_case_t> cases = {")

    for row in rows:
        nodes = row["defender_nodes"] + [None] * (4 - len(row["defender_nodes"]))
        print('      {"%s",\n       {%s},\n       %d, %d, "%s"},'
              % (row["fen"],
                 ",\n        ".join('"%s"' % n if n else "nullptr" for n in nodes),
                 row["distance"], row["lead"], row["family"]))

    print("    };")
    print("    // clang-format on")

    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["generate", "verify", "emit-cpp"])
    parser.add_argument("--out", default=TSV)
    args = parser.parse_args()

    if args.command == "generate":
        return cmd_generate(args)
    if args.command == "verify":
        return cmd_verify(args)

    return cmd_emit_cpp(args)


if __name__ == "__main__":
    sys.exit(main())
