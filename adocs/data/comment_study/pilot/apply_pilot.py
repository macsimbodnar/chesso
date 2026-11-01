#!/usr/bin/env python3
"""Question 8: compress comments in scratch copies by the rules proposed in the
report, by line range over the HEAD blob, so no code line is ever retyped.

Writes pilot/search.cpp and pilot/test_mate_carry.cpp. Never touches a tracked file.
Rules applied: keep contract, invariant, hazard and non-derivable why; keep the
DEC-142 golden form and the DEC-134, DEC-221 source forms; cut what a past step did,
superseded numbers and per-step logs; one id at the end of a claim; a measurement
stays only as the one number that decides something.
"""
import subprocess
from pathlib import Path

REPO = "/home/max/ws/chesso"
COMMIT = "5954c12"
HERE = Path(__file__).parent


def blob(p):
    return subprocess.run(["git", "-C", REPO, "show", f"{COMMIT}:{p}"], capture_output=True, check=True).stdout.decode()


def apply(path, edits, out):
    lines = blob(path).split("\n")
    # bottom up, so earlier line numbers stay valid
    for (a, b), new in sorted(edits.items(), reverse=True):
        lines[a - 1 : b] = new.strip("\n").split("\n") if new.strip() else []
    (HERE / out).write_text("\n".join(lines))


SEARCH = {
    (1781, 1818): """
  // The static evaluation of this node, computed once and read by everything
  // below that wants one, because every consumer has to see the same number.
  //
  // In check there is no number to have: the side to move is forced and a
  // static score bounds nothing. The sentinel goes in the slot and
  // improving_at() falls back four plies rather than two.
  //
  // evaluate() and never evaluate_lazy(): the lazy shortcut returns a bound
  // that holds against one window, and this number is compared against another
  // ply's and stored in an entry that outlives every window.
  //
  // The entry's number is the same number: evaluate() is a function of the
  // position, the key covers every field it reads, and what is stored is the
  // score and never a lazy bound. A call skipped, not a value approximated: 0
  // of 3368027 read-backs disagreed with a fresh call (S108). INV-4 is untouched.
  //
  // **Raw, and what the table stores.** This goes into every store this node
  // makes and into the correction's own update, and nowhere else; the consumers
  // read `static_eval`. So a read-back is corrected once, here, and never twice.
""",
    (1842, 1845): """
  // Hindsight reductions here read H0, S237: `nElo -6.34 +/- 7.58` over 8072
  // games.
""",
    (1847, 1875): """
  // The table's score is what a search of this position came back with, and
  // where the entry's bound certifies a direction it is the better input to a
  // pruning margin than the standing-still score. S234.
  //
  // Only a certified direction: a TT_BETA_NODE's score may raise the input and
  // never lower it, a TT_ALPHA_NODE's may lower it and never raise it. The mate
  // band is excluded: a mate score is a distance and not a value. TT_PV_NODE
  // would certify both, and is deliberately not taken: widening it is a change
  // with its own verdict.
  //
  // **The stack and the stored evaluation keep a static score, never this
  // estimate.** `improving` must never compare a search score against a static
  // one, and the stored entry must hold what `evaluate()` said or a correction
  // compounds through the table. The stack holds `static_eval`, the entry
  // `raw_eval`.
  //
  // Two consumers: the futility margin in the move loop, not switched, and
  // reverse futility below, switched by `RfpTtEstimate` (0 is `static_eval`).
""",
    (1889, 1992): """
  // Reverse futility pruning, also called static null move pruning: if the
  // node's own price is so far above beta that the opponent cannot claw the
  // difference back in the plies left, the node fails high. That price is
  // `pruning_eval` at `RfpTtEstimate` 1. One evaluate() and an assumption of
  // RFP_MARGIN per ply, not a proof, which is why the guards are the null move's
  // and the depth bound is low.
  //
  //   in check      the side to move is forced, so the static score bounds
  //                 nothing
  //   PV node       these lines get reported and played, and this returns a
  //                 bound rather than a score
  //   beta near mate  a fail-high against a mate bound would claim a mate this
  //                 never proved
  //   ply < 3       see RFP_MIN_PLY: the top of the tree decides the move
  //   excluded move  no move was searched, so it could only say "not singular",
  //                 and under the multicut it would return a static margin as
  //                 the node's value. S097.
  //   a whole search found it mated  below
  //
  // Not guarded on game_phase: no pass is made, so zugzwang does not enter it.
  //
  // What no setting of it can do: respect a forced mate for the opponent. A
  // static score is never a mate score; its bound is the material and table
  // sums and not the lazy clamp, which bounds one stage. test_evaluation "the
  // static score never reaches the mate band" asserts it, because an argument
  // from the size of some constants is what a refit walks through. Only the
  // depth and ply bounds contain the hidden mate (S033); a mate deeper than
  // RFP_MIN_PLY can still be missed for an iteration, and no test covers that.
  //
  // The cast on ply is the tune build's: a parameter is a plain int there.
  //
  // A WHOLE SEARCH'S PROOF OUTRANKS THE MARGIN, S293. No reverse futility at a
  // node whose entry is an upper bound or exact score inside the mated band
  // from a node that searched every legal move (`no_move_skipped`): the
  // margin's claim, that the opponent cannot win the difference back, is that
  // search's opposite. From a skipped-move node it is not a proof, S292. The
  // premise is the null move's: "the value of the best (legal) move has to be
  // better than that obtained from the null move" (David-Tabibi and Netanyahu,
  // "Verified Null-Move Pruning", ICGA Journal 25(3), 2002, section 2; the
  // Chess Programming Wiki, "Reverse Futility Pruning"). It only removes a cut,
  // so a wrong bound costs a subtree and cuts nothing. No constant, so no seed
  // (DEC-134); from the two descriptions above, DEC-221.
""",
    (2003, 2012): """
    // S234. Read here and not re-derived, so this site and the futility site
    // below decide on the same number, and one local so `RfpTtEstimate` 0 cannot
    // be half taken (DEC-215). The block excludes a node in check, where the
    // estimate is the sentinel.
""",
    (2017, 2033): """
    // Fail soft, returning the bound actually argued for: the estimate minus
    // what the opponent was assumed able to win back. On the lower-bound branch
    // that is below something the entry already proved, and on the upper-bound
    // branch it lowers the bound, which a fail-soft return may always do. A
    // point between beta and that bound read zero, S235.
""",
    (2043, 2089): """
  // Razoring, S116: reverse futility's mirror on the other tail. Where the
  // static score stands a margin or more below alpha a few plies from the
  // leaves, quiescence is asked whether the captures can lift it; the verified
  // form returns quiescence's score **only if it is at or below alpha**, and a
  // node it lifts falls through and is searched as before.
  //
  //   in check      the node is forced and quiescence would answer an evasion
  //                 node with no real ply
  //   PV node       these lines get reported and played
  //   alpha near mate  a static score never approaches the band, so whole
  //                 subtrees under a mate-scored bound would drop to captures
  //   excluded_move S097. Quiescence would search the excluded move itself if
  //                 it is a capture. Inert at the shipped depths, not inside
  //                 the declared ranges: SeMinDepth 4 puts a verification at
  //                 depth 1
  //   depth         RazorDepth. 0 is off, proved: no node reaches here below 1
  //   ply floor     RfpMinPly, reverse futility's own floor, DEC-248. Without
  //                 it 9 of the 26 S145 mates in two were found an iteration
  //                 late
  //
  // **Quiescence's blind spot is this rule's blind spot.** It generates captures
  // and quiet queen promotions only, so a quiet mate by the side to move is
  // invisible and such a node is razored whatever the verification says. In a
  // mate in two at depth 3 the mating move sits at a depth-1 node at ply 2, which
  // the ply floor keeps out; "pruning does not hide a forced mate" holds the row.
  //
  // The static score and never the table-tightened estimate: the input is the
  // node's own `evaluate()` plus the correction, what the step measured. The
  // node is counted here and again by quiescence at the same ply. Fail soft, and
  // no second window. Implemented from the step file's description, DEC-221.
""",
}

GRIDS = """  //   adocs/data/S203_case_sweep.sh --ceilings
  //       adocs/data/S202_sweep_s170.txt adocs/data/S261_sweep_s170.txt
  //       adocs/data/S286_sweep_s170.txt adocs/data/S292_sweep_s170.txt
  //       adocs/data/S293_sweep_s170.txt adocs/data/S083s_sweep_s170.txt
  //       adocs/data/S135_sweep_s170.txt adocs/data/S296_sweep_s170.txt
  //       adocs/data/S121_sweep_s170.txt adocs/data/S123_sweep_s170.txt
  //       adocs/data/S125_sweep_s170.txt"""
GRIDS = "\n".join(l[2:] for l in GRIDS.split("\n"))

CARRY = {
    (14, 142): """
// S170. A mate score reported from a table entry an earlier search wrote, or
// from a proof this search made and then overwrote, still carries a line that
// reaches the mate.
//
// NOT tests/test_mate_pv.cpp. That file gives every position a cold table, and
// this defect cannot exist on one: the score is read back from an entry written
// by a search of an earlier move in the same game, and the entries that carried
// its line are gone by then. The only shape that reproduces it is a game
// replayed move by move through one process.
//
// adocs/data/S170_cases.tsv holds the games, each row with its own `go` budget
// and first ply: node budgets, so a case is a property of the tree and not of
// how busy the machine is.
//
// THREE ASSERTIONS, DEC-162:
//
//   a line at least as long as the distance it claims that is not checkmate at
//   exactly that distance. DEC-122's guarantee, so it is asserted at zero, at
//   any budget, pinned to nothing.
//
//   a line shorter than it claims. DEC-122 leaves it short and visible on a
//   failed walk, an expected residue that moves with the tree. A per-case
//   ceiling, the worst cell the recorded grid shows for that case.
//
//   a mate was reported at all, over the set: a strict majority of the guarded
//   cases. Without it "no short line" is true and means nothing. One case falling
//   silent is a cell moving; a majority falling silent is a key redraw or the
//   engine no longer finding these mates.
//
// THE CAUSES. A, B, C: a score inherited across searches (C reports `mate 7`
// warm and no mate at any depth cold). D: a mate proved inside this search whose
// mid-line entry was evicted before the walk (reports `mate -6` cold as well). E:
// the walk stalls on one missing slot with the certifying entry in the children
// of the position whose entry is gone, S171. F: was an aborted iteration's line
// beside the last completed `mate 6`; on today's tree this case no longer reaches
// that path, and tests/test_mate_pv.cpp's cold cell holds it
// (adocs/data/S294_mw02_reach.txt).
//
// THE BUDGETS ARE A KNIFE EDGE (C reported mates at one budget of nine, S204).
// Eviction
// is decided by the Zobrist keys, so a key redraw retires them, and re-sweeping
// the budget alone brings every case back, no game and no mining (S203). Re-run
// after any change that moves the tree (DEC-154, DEC-156). A re-sweep moves
// budgets and nothing else: no golden here is a per-case mate count, and a case
// whose cell has moved does not fail on its own.
//
//   adocs/data/S203_case_sweep.sh        records the grid, and states the rule:
//                                        the cheapest budget at the row's stride
//                                        whose cell reports a mate line
//   adocs/data/S170_pick_budgets.sh adocs/data/S170_cases.tsv
//       adocs/data/<step>_sweep_s170.txt > NEW.tsv
""",
    (168, 181): """
  // Whether this row is a case the guard asserts on. A row that is `no` is a
  // reproduction kept for the step that owns it and not a property held here.
""",
    (379, 670): """
// GOLDEN (DEC-142): the most short mating PVs a case may report, one per replayed
// game of adocs/data/S170_cases.tsv at its own budget and stride. Each is the
// largest short-line count any cell of the recorded grid shows for that case, at
// the stride its TSV row carries, over every recorded sweep in the command.
// Re-derive, never reading one off a failing run:
//
"""
    + GRIDS
    + """
//
// Moves legitimately on: a re-sweep of the budgets, which is a Zobrist redraw
// (DEC-154); a change to complete_mate_pv() or to the reporting layer's aborted
// path (DEC-251). A tree-moving step that re-sweeps adds its grid to the command;
// grids older than S202's read a bound entry's move first and stay out, and a
// revert of S202 puts them back. Margin: 0. Lowering one is progress on the class.
// Raising one is relaxing a test: DEC-299's guard (+2, both mate floors holding)
// or the owner's decision. Property beside it: "a mate score carried across
// searches keeps a line that reaches it", whose other two assertions carry no
// per-case number and do not move with the tree.
//
//   case                       ceiling  worst cell, at the case's own stride
//   A_mate8_shallow                  2  1000000 on the S286 grid; DEC-295
//   B_mate6_shallow                 13  1200000 on the S121 grid; DEC-327, DEC-328
//   C_mate7_depth11                  4  4000000 on the S202 grid
//   D_mate_minus6_depth10            1  4000000 on the S135 grid; DEC-299's guard
//   E_mate_minus9                    2  2000000 on the S135 grid; DEC-299's guard
//   F_mate6_inherited_no_line        3  300000 on the S083s grid, stride 2; DEC-307
//
// Why they move: a line the search never stored is a line the walk cannot certify,
// so any change to the tree changes the residue. `unreached.empty()` is the
// guarantee beside them and a rise must not touch it. A ceiling of 0 says the grid
// never showed one; a budget where none happens to appear says nothing.
""",
    (673, 676): "",
    (810, 813): """
    // A strict majority, more than half: guarding a sixth case must not loosen
    // the precondition. DEC-251.
""",
}

if __name__ == "__main__":
    apply("src/search.cpp", SEARCH, "search.cpp")
    apply("tests/test_mate_carry.cpp", CARRY, "test_mate_carry.cpp")
    print("written")
