#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <fstream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "test_helpers.hpp"
#include "uci.hpp"

// S170. A mate score reported from a table entry an earlier search wrote, or
// from a proof this search made and then overwrote, still carries a line that
// reaches the mate.
//
// WHY THIS IS NOT tests/test_mate_pv.cpp. That file gives every position its
// own `ucinewgame` and one search, which is a cold table by construction, and
// the defect measured here cannot exist on a cold table: the score is read
// back from an entry written by a search of an *earlier move in the same
// game*, and the entries that carried its line are gone by the time it is read
// back. A 16 MB table holds about half a million entries and one search here
// visits more than that. So the only shape that reproduces it is a game
// replayed move by move through one process, which is what this file does.
//
// WHAT THE CASES ARE. adocs/data/S170_cases.tsv is the four games that
// produced the ten `Incomplete mating PV` lines S147's clean 3000-game run
// left, taken from the `Position;` and `Moves;` lines fastchess printed beside
// each warning. Each row carries its own `go` budget and its own first ply,
// swept for the cheapest reproduction: node budgets rather than movetime, so
// the case is a property of the tree and not of how busy the machine is.
//
// WHAT THIS FILE ASSERTS, AND WHY IT IS THREE THINGS AND NOT ONE. S204,
// DEC-162. Until then it held one failure list and one per-case floor, and both
// had to be true at one node budget, so the budget was chosen where they were.
// The count that retired that shape: over the nine budgets of the sweep grid,
// `C_mate7_depth11` reports a mate line in **one cell** at HEAD and its
// configured budget is that cell. A green that is one cell in nine is a
// coincidence, not a property, and every change that moved the tree re-pinned
// it.
//
// The two things the failure list merged are not the same kind of thing:
//
//   a line at least as long as the distance it claims that is not checkmate at
//   exactly that distance. DEC-122's own guarantee -- the walk extends only
//   when it reaches the mate -- so this is a published lie and it is asserted
//   at zero, at any budget, pinned to nothing. Measured 0 in 428 mate lines
//   across the whole grid by `adocs/data/S204_class_census.py`.
//
//   a line shorter than the distance it claims. DEC-122 leaves it short and
//   visible on a failed walk, so it is an expected residue, S202 owns closing
//   it, and it moves with the tree -- 11 of the stride-1 cells show one at
//   HEAD. It carries a per-case ceiling instead of a zero, and the ceiling is
//   the worst cell the recorded grid shows for that case.
//
// And the precondition -- that a case was seen to report a mate at all, without
// which "no short line" is true and means nothing -- is now asserted over the
// **set** rather than per case: a majority of the guarded cases must report
// one. One case falling silent is a cell moving; three of six falling silent
// is the engine no longer finding these mates, which is what the fixture is
// for. (Five cases were guarded until S202 guarded F, DEC-251. The majority is
// a strict one, more than half: 3 of 5 then, 4 of 6 now. The old form,
// `(guarded + 1) / 2`, gave 3 of 5 and would have given 3 of 6.)
//
// The three causes the cases cover, measured 2026-09-02 and recorded in the
// step file: a score inherited across searches (A, B, C -- C reports `mate 7`
// warm and no mate at any depth cold), and a mate proved inside this search
// whose mid-line entry was evicted before the line could be walked (D --
// reports `mate -6` cold as well).
//
// E is the fourth, added by S171: the walk stalls eight plies from the mate on
// one missing slot, with the entry that certifies the continuation sitting in
// the children of the position whose entry is gone.
//
// F is the fifth, guarded from S202 (DEC-251). At its own cell it is S170's
// third cause: the last completed iteration's `mate 6` printed beside an
// aborted iteration's line, which the table proves a mate 7 and no walk can
// complete to 11 plies. iterative_deepening_search() now prints the last
// completed line there, which starts with the same move and reaches the mate,
// and F's ceiling is 0. On fe5d3b3 this case reports 1 short line in 8 at its
// cell; mutant MW02 of tools/mutants/S202_mate_walk.py is that tree's aborted
// path and this case kills it.
//
// What S202 also changed under every case here: complete_mate_pv() reads a
// move certified at the distance still owed -- the entry's own when exact
// there, else a certified child -- before a bound entry's move. Over the grid
// that closed the walks that followed bound entries to a checkmate too early
// or to the claimed length without one (DEC-150's class where the table does
// certify a line); tests/test_search.cpp holds that rule directly.

// WHAT RETIRES THIS FILE'S FIXTURE, AND WHAT DOES NOT. The class here is
// transposition-table eviction, and which entries evict which is decided by the
// Zobrist keys. So the budgets in `adocs/data/S170_cases.tsv` are valid only
// for the key set they were chosen under, and a redraw retires them -- measured
// over four arbitrary seeds, every one of which left three or four of the six
// cases reporting no mate at all. That is what the majority assertion below
// catches -- a redraw silences most of the set, where a change that moves the
// tree silences one case and leaves the rest reporting.
//
// What it does *not* mean is that the cases themselves are lost. S203 redrew
// the keys and every one of the six came back by re-sweeping its node budget
// alone, no game and no mining run: A at 1000000 nodes instead of 300000, C at
// 1500000 instead of 1000000, D at 4000000 instead of 1000000. The rule is
// stated once in `adocs/data/S203_case_sweep.sh` and applied to every row --
// as DEC-162 left it, the cheapest budget at the row's own stride whose cell
// reports a mate line, chosen on the mate count alone; DEC-156's first form
// also wanted a floor of lines, all complete, and the floors are gone --
// because a budget chosen per case because it happened to be green would be
// fitting the fixture to the test. S245 re-swept every row in that form.
//
// The budgets are a knife edge and the file says so rather than implying it:
// C reported 13 mate lines at 1500000 nodes and 0 at both 1000000 and 2000000
// when S203 swept it, and on S245's grid (adocs/data/S245_sweep.txt) it
// reports 16 at 1000000 and 0 in every other stride-1 cell, 1500000 and
// 2000000 included.
// Re-run that sweep after any change that moves the tree, not only after a key
// change -- DEC-154, DEC-156. What DEC-162 changed is what depends on the
// answer: no golden here is a per-case mate count any more, so a re-sweep moves
// budgets and nothing else, and the assertions below are written so that a case
// whose cell has moved does not fail on its own.
//
// D carries one more thing. Between 1200000 and 3000000 nodes it reproduced a
// short line when S203 swept it -- `mate -6` at ply 35 depth 11 with a
// 10-of-12-ply PV, at a depth that also published a complete 12/12 -- which
// is the class DEC-122 leaves short and visible and S202 owns. It is recorded
// there as a reproduction rather than hidden behind the 4000000 that clears
// it. On S245's grid D reports no mate line in that range and 4, none short,
// at 4000000.

#ifndef CHESSO_SOURCE_DIR
#error "CHESSO_SOURCE_DIR must be defined so the test can read the case file"
#endif


// The board a reported line is replayed on. Separate from the game the UCI
// layer searches with, so replaying a line cannot disturb the engine state the
// next position is searched from.
static game_t replay;


struct case_t
{
  std::string name;
  std::string fen;
  std::vector<std::string> moves;
  std::string go;
  size_t start;

  // Plies between one search and the next. 1 searches every ply; 2 is one
  // side's own turns, which is what a game gives an engine's table -- it
  // never searches a position its opponent moved from. S171.
  size_t stride;

  // Whether this row is a case the guard asserts on. A row that is `no` is a
  // reproduction kept for the step that owns it and not a property held here.
  // `E_mate_minus9` was `no` until S171, on the reading that its `mate -9`
  // claimed a distance the position does not hold; S171 measured that reading
  // wrong -- an 18-ply mating line from that root exists and the engine
  // publishes it once the walk can reach it -- so the row is guarded. DEC-127.
  //
  // `F_mate6_inherited_no_line` was `no` until S202: the reproduction of the
  // 8 lines S171's own census left (DEC-150), kept for the step that owned
  // the class. Under the keys and budget it has now, its one short line is not
  // that census's mechanism but S170's third cause -- an aborted iteration's
  // line beside the last completed `mate 6` -- and S202 closes it in the
  // reporting layer, so the row is guarded with a ceiling of 0. DEC-251.
  bool guard;
};


static std::vector<std::string> split_words(const std::string& text)
{
  std::istringstream stream(text);
  return {std::istream_iterator<std::string>(stream),
          std::istream_iterator<std::string>()};
}


static std::vector<case_t> read_cases()
{
  const std::string path =
      std::string(CHESSO_SOURCE_DIR) + "/adocs/data/S170_cases.tsv";

  std::ifstream file(path);
  REQUIRE_MESSAGE(file.good(), ("Cannot open " + path));

  std::vector<case_t> cases;
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') { continue; }

    std::vector<std::string> field;
    std::istringstream stream(line);
    std::string cell;

    while (std::getline(stream, cell, '\t')) {
      field.push_back(cell);
    }

    if (field.size() < 7 || field[0] == "name") { continue; }

    const size_t start = static_cast<size_t>(std::stoul(field[4]));
    const size_t stride = static_cast<size_t>(std::stoul(field[5]));
    const std::vector<std::string> moves = split_words(field[2]);

    REQUIRE_MESSAGE(stride >= 1, (field[0] + ": stride must be at least 1"));

    // Both are size_t, so this is what keeps the subtraction below from
    // wrapping to a huge value that the modulo could then accept.
    REQUIRE_MESSAGE(start <= moves.size(),
                    (field[0] + ": start " + field[4] + " is past ply " +
                     std::to_string(moves.size())));

    // A schedule that steps over the last ply never searches the position the
    // case is about, and the row would pass by never looking. S171.
    REQUIRE_MESSAGE(
        (moves.size() - start) % stride == 0,
        (field[0] + ": start " + field[4] + " and stride " + field[5] +
         " step over the last ply, " + std::to_string(moves.size())));

    cases.push_back({field[0], field[1], moves, field[3], start, stride,
                     field[6] == "yes"});
  }

  return cases;
}


struct mate_report_t
{
  size_t ply;
  int depth;
  int mate_in;
  std::vector<std::string> pv;
};


// The plies a mate at this distance takes. The side delivering it moves last,
// so its mate in N is 2N - 1 plies and one it receives is 2|N|.
static size_t plies_for(int mate_in)
{
  return (mate_in > 0) ? static_cast<size_t>(2 * mate_in - 1)
                       : static_cast<size_t>(-2 * mate_in);
}


// Every `info` line of one search that carried a mate score.
static std::vector<mate_report_t> search_mate_lines(const std::string& command,
                                                    const std::string& go,
                                                    size_t ply)
{
  {
    stdout_capture_t capture;
    uci_process_line(command);
  }

  std::vector<std::string> lines;
  {
    stdout_capture_t capture;
    uci_process_line("go " + go);
    uci_wait_for_search();
    lines = capture.lines();
  }

  std::vector<mate_report_t> result;

  for (const std::string& line : lines) {
    const std::vector<std::string> token = split_words(line);

    if (token.size() < 4 || token[0] != "info" || token[1] != "score") {
      continue;
    }
    if (token[2] != "mate") { continue; }

    mate_report_t entry{ply, 0, std::stoi(token[3]), {}};

    for (size_t i = 4; i < token.size(); ++i) {
      if (token[i] == "depth" && i + 1 < token.size()) {
        entry.depth = std::stoi(token[i + 1]);
      }

      if (token[i] == "pv") {
        entry.pv.assign(token.begin() + static_cast<long>(i) + 1, token.end());
        break;
      }
    }

    result.push_back(entry);
  }

  return result;
}


// Plays `prefix` and then the first `count` moves of the reported line, and
// answers whether the position they reach is checkmate. Every move is looked
// up in the engine's own generator, so a token naming no legal move is a
// failure and not a silent skip.
static bool line_ends_in_mate(const std::string& fen,
                              const std::vector<std::string>& prefix,
                              size_t prefix_count,
                              const std::vector<std::string>& pv,
                              size_t count,
                              std::string& why)
{
  REQUIRE(load_FEN(fen, &replay));

  std::vector<std::string> played(
      prefix.begin(), prefix.begin() + static_cast<long>(prefix_count));
  played.insert(played.end(), pv.begin(),
                pv.begin() + static_cast<long>(count));

  for (size_t i = 0; i < played.size(); ++i) {
    const std::optional<uci_move_t> parsed = algebraic_to_uci_move(played[i]);

    if (!parsed.has_value()) {
      why = "unparseable move " + played[i];
      return false;
    }

    move_t moves[MAX_MOVES];
    const size_t generated =
        generate_moves(game_tables(), &replay.board, moves);

    move_t chosen = 0;
    for (size_t m = 0; m < generated; ++m) {
      if (MOVE_FROM(moves[m]) == parsed->from &&
          MOVE_TO(moves[m]) == parsed->to &&
          MOVE_PROMOTED(moves[m]) == parsed->promotion) {
        chosen = moves[m];
        break;
      }
    }

    if (chosen == 0 || !make_move(&replay, chosen)) {
      why = "illegal move " + played[i] + " at ply " + std::to_string(i);
      return false;
    }
  }

  move_t replies[MAX_MOVES];
  const size_t reply_count =
      generate_moves(game_tables(), &replay.board, replies);

  // generate_moves() is legal-only (INV-1), so an empty list while in check is
  // checkmate by definition and no per-move legality pass is wanted here.
  if (reply_count > 0) {
    why = "the position after " + std::to_string(count) +
          " reported plies has legal replies";
    return false;
  }

  if (!is_check(&replay)) {
    why = "the position after " + std::to_string(count) +
          " reported plies is stalemate";
    return false;
  }

  return true;
}


// GOLDEN (DEC-142): the most short mating PVs a case may report -- 2, 5, 4, 1,
// 4 and 2, one per replayed game of adocs/data/S170_cases.tsv at its own
// budget and stride. (The line read "5, 11, 0, 1, 8 and 2" until S095 and had
// been stale since S109 moved three of them; it is the function below that
// ships, and these six are now the same numbers. C's 0 became 2 while S236's
// history term was in the tree and came back when it left, DEC-231; S116's
// razoring moved A 5 -> 6 and C 0 -> 5, DEC-250; S202's walk took all six to
// 0, 5, 4, 0, 0, 0, DEC-251, and S261's vector moved F 0 -> 1, DEC-282. This
// line kept S116's six from S202 on, stale: S023 corrected it and its revert
// put the stale six back; S039 corrected it again, and its revert keeps the
// correction. S286's relanded correction moved A 0 -> 2, E 0 -> 1 and F 1 -> 2,
// DEC-295; S110's non-pawn tables D 0 -> 1, DEC-299, and E 1 -> 4,
// provisionally, DEC-301.)
// Re-derive: adocs/data/S203_case_sweep.sh --ceilings
// adocs/data/S202_sweep_s170.txt adocs/data/S261_sweep_s170.txt
// adocs/data/S286_sweep_s170.txt adocs/data/S110_sweep_s170.txt
// (the rule, the grid list and the worst cells are tabled below).
// Moves legitimately on: a
// re-sweep of the budgets, which is a Zobrist redraw (DEC-154); a change to
// complete_mate_pv() or to the reporting layer's aborted path, which is S202's
// ground (DEC-251). Margin: 0 -- each ceiling is the worst cell of the
// recorded grid, so a case that gets worse goes red on the first extra short
// line. Widening it is a decision, not a re-derivation. Property beside it: "a
// mate score carried across searches keeps a line that reaches it", whose two
// other assertions -- a full-length line ends in checkmate, and a majority of
// the guarded cases reports a mate line at all -- carry no per-case number and
// do not move with the tree.
//
// S204, DEC-162: this replaces the per-case floor on mate lines, which was a
// single cell of a sparse grid and was re-pinned by every change that moved the
// tree.
//
// A short line is DEC-122's expected residue -- the walk found no line it
// could certify, so the line stays exactly as the search produced it. S202
// closed the part of the class the table does certify (DEC-251) and the rest
// is what these ceilings bound.
//
// The rule, stated once and applied to every row: the ceiling is the largest
// short-line count any cell of the recorded grid shows for that case, at the
// stride its TSV row carries, over every recorded sweep in the command.
// Re-derive it with
//
//   adocs/data/S203_case_sweep.sh --ceilings
//       adocs/data/S202_sweep_s170.txt adocs/data/S261_sweep_s170.txt
//       adocs/data/S286_sweep_s170.txt adocs/data/S110_sweep_s170.txt
//
// which is the script DEC-142 requires beside a golden. Never read one off a
// failing run.
//
// WHICH GRIDS ARE IN THE COMMAND, AND WHY ONE. S202. The command held the five
// grids S204, S109, S095 and S116 recorded. Every one of them was taken with
// complete_mate_pv() reading a bound entry's move before a certified one, so
// each counts short lines the walk this file now drives would have closed, and
// four of them were taken on engines whose trees are not today's at all. The
// grid a ceiling is read from has to be today's engine, walk included, so
// those five leave the command -- they stay tracked as the record of their
// trees. S116's grid is today's search to the node (`bench` 4081329, no step
// since moved a node) and so the same 108 cells as S202's, read with the old
// walk; S202's grid supersedes it rather than adding a sample to it. A
// tree-moving step that re-sweeps adds its grid to the command, which is how
// the list grows back to several samples (DEC-162's intent). **S261's is the
// first so added** (`adocs/data/S261_sweep_s170.txt`, the lane's vector on
// `870d210`, 108 cells, 1485 mate lines and 16 short): it moves no ceiling but
// F's, below (DEC-282). **S286's is the next**
// (`adocs/data/S286_sweep_s170.txt`, S099's pawn correction relanded on
// `8560a36`, 108 cells, 1461 mate lines and 18 short): it moves A, E and F,
// below (DEC-295). **S110's is the next** (`adocs/data/S110_sweep_s170.txt`,
// the non-pawn correction tables averaged with the pawn table's, DEC-300, 108
// cells, 1506 mate lines and 26 short): it moves D and E, below (DEC-299,
// DEC-301).
//
//   case                       ceiling  worst cell, at the case's own stride
//   A_mate8_shallow                  2  1000000 on the S286 grid, 2 of 6;
//                                         DEC-295, in S286's commit only
//   B_mate6_shallow                  5  1500000 on the S202 grid, 5 of 32
//   C_mate7_depth11                  4  4000000 on the S202 grid, 4 of 13
//   D_mate_minus6_depth10            1  3000000 on the S110 grid, 1 of 4;
//                                         DEC-299, in S110's commit only
//   E_mate_minus9                    4  4000000 on the S110 grid, 4 of 15;
//                                         DEC-301, provisional, the owner
//                                         to rule before an H1 ships
//   F_mate6_inherited_no_line        2  1500000 on the S286 grid at stride 2,
//                                         2 of 4; DEC-295, in S286's commit
//                                         only (DEC-282's 1 on S261's grid
//                                         before)
//
// Before S202 they were 6, 15, 5, 2, 11 and 5 over the five older grids.
//
// C's ceiling was 0 over the four earlier grids, earned rather than chosen,
// which is the difference this file keeps: a ceiling of 0 says the grid has
// never shown one, and a budget where none happens to appear says nothing. It
// was 2 for as long as S236's history term was in the tree -- one grid showed
// two, and the rule is the worst cell and not the usual one -- and the revert
// was the grid leaving the command above, not a number being lowered to match
// a run. S116's grid shows five, and it is 5 now (DEC-250).
//
// A step that lowers a ceiling is recording progress on S202. A step that
// raises one is relaxing a test and needs a decision.
//
// **One rose at S261 and that is DEC-282's decision.** The lane's vector moves
// twenty-one search parameters, and its grid (`adocs/data/S261_sweep_s170.txt`)
// shows F's 300000 cell at stride 2 reporting 7 mate lines, 1 short, where
// S202's grid showed none at that stride, so the script over both grids
// answers F 0 -> 1 and nothing else moves. It lands in the vector's commit and
// stands only on S261's H1 -- an H0 or a no verdict reverts it with the vector
// -- the DEC-272, DEC-250, DEC-241 and DEC-225 form. F's own cell, 100000 at
// stride 2, reports 2 mate lines and none short. What rose is DEC-122's
// expected residue, not the guarantee: `unreached` is empty on all six cases.
//
// **Three rose at S286 and that is DEC-295's guard, the owner's.** S099's
// pawn correction, relanded, moves the static score every pruning margin
// reads, so the tree a replay builds is a different tree, and a line the
// search never stored is a line the walk cannot certify. Its grid
// (`adocs/data/S286_sweep_s170.txt`) shows A's 1000000 cell at stride 1
// reporting 6 mate lines, 2 short, E's 4000000 cell 23 with 1 short, and F's
// 1500000 cell at stride 2 4 with 2 short, so the script over the three grids
// answers A 0 -> 2, E 0 -> 1 and F 1 -> 2 and nothing else moves. DEC-295
// applies them as the script answers because both mate floors held or rose on
// that tree (`EXACT_FLOOR` 159 -> 163, `MATE_IN_THREE_FLOOR` 11) and no
// ceiling rose by more than 2; they stand in S286's commit only and leave with
// any revert of it. The budgets moved by the same grid and rule: C 2000000 ->
// 1200000 (23 lines, 1 short), D 2000000 -> 500000 (1 line) and F 100000 ->
// 300000 (3 lines). S099's own grid, on S261's tree, read the same 108 cells
// from the same src/ (DEC-284 and DEC-272 lapsed with its H0).
//
// **Two rose at S110: D by DEC-299's guard, E past it by DEC-301.** The
// non-pawn tables move the static score again, and their grid
// (`adocs/data/S110_sweep_s170.txt`) shows D's 3000000 cell at stride 1
// reporting 4 mate lines, 1 short, and E's 4000000 cell 15 with 4 short, so
// the script over the four grids answers D 0 -> 1 and E 1 -> 4 and nothing
// else moves; the grid alone answers 0, 4, 0, 1, 4, 0. Both mate floors held
// on that tree (`EXACT_FLOOR` 163, `MATE_IN_THREE_FLOOR` 11), so D's +1 is
// inside DEC-299 -- DEC-295's guard word for word, for S110 -- and stands in
// S110's commit only. E's +3 is outside it: **DEC-301 carries E at 4
// provisionally in S110's commit, and the owner rules on it before an H1
// ships**; it leaves with any revert. The budgets moved by the same grid and
// rule: C 1200000 -> 500000 (14 lines), D 500000 -> 1500000 (1 line), E
// 300000 -> 1000000 (5 lines) and F 300000 -> 100000 at its stride 2 (6
// lines), none short there -- C, D and F by DEC-299, E by DEC-301.
//
// **Three of them rose at S109 and that is the decision the block owes.** The
// shallow-depth pruning block takes 74 % of the tree, and a line the search
// never stored is a line the walk cannot certify, so short lines are what a
// block of that size produces more of: D from 1 to 2, E from 8 to 9 and F from
// 2 to 5, each read off this step's own grid by the script above and none off
// the failing run that found them. What did **not** move is the guarantee
// beside them -- `unreached.empty()`, the clause that says a line published at
// its claimed length ends in checkmate -- which is 0 across the whole of this
// grid as it was across S204's. So what rose is the residue DEC-122 already
// calls expected, not the promise. S202 still owns closing the class.
//
// **Two of them rose at S095 and that is the decision the step owes
// (DEC-225).** That step adds a ply of reduction at every node whose table
// entry carries no move, which in a replay from a cold table is most of them,
// and a line the search never stored is a line the walk cannot certify -- the
// same mechanism S109's paragraph above describes, one rule further on. E goes
// 9 to 11 and B 11 to 15, both read off this step's own grid
// (`adocs/data/S095_sweep_block.txt`, 108 cells) by the script above and
// **never off the failing run that found them**; the other four are unmoved
// and the rule's answer over all four grids is 5, 15, 0, 2, 11, 5.
//
// **One of them rose at S236 v2 and came back down when that step ended
// (DEC-231).** That step scaled a late quiet's reduction by a fraction of a ply
// of the move's own history, and a line the search never stored is a line the
// walk cannot certify: C went 0 to 2, read off that candidate's own grid
// (`adocs/data/S236_v2_sweep.txt`, 108 cells, still tracked) by the script
// above and never off the failing run that found it. Its second verdict read a
// zero, the term left, and the ceiling came back to 0 by dropping that grid
// from the `--ceilings` command -- which is what "re-derive, never re-read"
// looks like in both directions. The other five never moved.
//
// **Two of them rose at S116 and that is the decision the step owes
// (DEC-250).** Razoring at depth one answers a node far below alpha with a
// quiescence search when that confirms the fail-low, and a line the search
// never stored is a line the walk cannot certify -- the mechanism the
// paragraphs above describe, one rule further on. A goes 5 to 6 and C 0 to 5,
// both read off that landing's own grid (`adocs/data/S116_sweep_s170.txt`, 108
// cells, the one its budgets were re-derived from) by the script above and
// **never off the failing run that found them**; the other four are unmoved
// and the rule's answer over the five grids is 6, 15, 5, 2, 11, 5. C's worst
// cell is the one it now drives, 1500000 at stride 1, 8 mate lines with 5
// short. A's is 3000000, 18 with 6 short, while A drives 500000, 4 lines with
// 0 short: like B's at S095, that loosens an assertion that was green at its
// own cell, because the rule is the worst cell of the grid at the row's
// stride. What rose is DEC-122's expected residue, not the guarantee; S202
// still owns closing the class.
//
// The number that matters for the reading is not the count alone. At the
// cell E drove until S245 -- stride 1, 1500000 nodes -- S095's tree reported
// **22 mate lines with 11 short** where S109's grid reported 13 with 9: the
// count rises past the ceiling and the share of short lines **falls, 69 % to
// 50 %**. That reading is S095's tree's and does not carry: on S245's grid
// the same cell reports 10 with 0 short, and E drove 500000 from S245, 2
// lines with 1 short; since S114's re-sweep it drives 300000, where all 10
// of its lines were short on that tree, inside its ceiling of 11, and on the
// tree S114's removal leaves the same cell reports 2 with 0 short. B's worst
// cell is 15 of 52 at 1200000 against 11 of its own at S204's 1000000.
//
// What did not move is again the guarantee beside them: `unreached.empty()`
// holds on all six cases at their own budgets and strides in both builds, so
// no line published at its claimed length fails to end in mate. That clause is
// the case's and not the grid's -- `adocs/data/S170_replay.py` reports short
// lines and does not walk a full-length one to its mate -- so it is asserted
// where the case asserts it and is not claimed of the 108 cells.
//
// **All six came down at S202 and that is the progress the paragraph above
// the table promised (DEC-251).** The walk reads a move certified at the owed
// distance before a bound entry's, and an aborted line it cannot complete
// gives way to the last completed line of the same move. Neither moves a node.
// The S202 grid (`adocs/data/S202_sweep_s170.txt`, 108 cells, the S116 grid's
// trees exactly) reports 19 short lines where the same cells reported 33,
// and every mate count is unchanged, so the budgets do not move. The ceilings
// go 6, 15, 5, 2, 11, 5 -> 0, 5, 4, 0, 0, 0, read off that grid by the script
// above and never off a run. This history is kept because a revert of either
// fix puts back the old walk, and with it the old grids' command.
static size_t short_line_ceiling(const std::string& name)
{
  // A's 2 and F's 2 by DEC-295, in S286's commit only; D's 1 by DEC-299, in
  // S110's; E's 4 by DEC-301, provisional in S110's commit until the owner
  // rules, which happens before an H1 ships.
  if (name == "A_mate8_shallow") { return 2; }
  if (name == "B_mate6_shallow") { return 5; }
  if (name == "C_mate7_depth11") { return 4; }
  if (name == "D_mate_minus6_depth10") { return 1; }
  if (name == "E_mate_minus9") { return 4; }
  if (name == "F_mate6_inherited_no_line") { return 2; }

  FAIL("unknown case " << name);
  return 0;
}


TEST_CASE("a mate score carried across searches keeps a line that reaches it")
{
  uci_init();
  initialize_game_const_data(&replay);

  const std::vector<case_t> cases = read_cases();
  REQUIRE_MESSAGE(cases.size() == 6,
                  ("adocs/data/S170_cases.tsv is not the tracked set any "
                   "more: " +
                   std::to_string(cases.size()) + " rows, expected 6"));

  size_t guarded = 0;
  size_t reporting = 0;
  std::string quiet;

  for (const case_t& game : cases) {
    if (!game.guard) { continue; }

    ++guarded;

    {
      // One `ucinewgame` per game and none between the plies: the table the
      // next search reads is the one the previous plies left, which is the
      // whole point of the file.
      stdout_capture_t capture;
      uci_process_line("uci");
      uci_process_line("setoption name Hash value 16");
      uci_process_line("ucinewgame");
    }

    size_t mate_lines = 0;
    size_t short_lines = 0;
    std::vector<std::string> unreached;

    for (size_t ply = game.start; ply <= game.moves.size();
         ply += game.stride) {
      std::string command = "position fen " + game.fen;

      if (ply > 0) {
        command += " moves";
        for (size_t i = 0; i < ply; ++i) {
          command += " " + game.moves[i];
        }
      }

      const std::vector<mate_report_t> reports =
          search_mate_lines(command, game.go, ply);

      for (const mate_report_t& report : reports) {
        ++mate_lines;

        const size_t needed = plies_for(report.mate_in);
        const std::string where = game.name + " ply " +
                                  std::to_string(report.ply) + " depth " +
                                  std::to_string(report.depth) + " mate " +
                                  std::to_string(report.mate_in);

        // Shorter than the distance claimed: the walk stalled and DEC-122
        // leaves the line as the search produced it. Counted against the
        // ceiling, not a failure on its own.
        if (report.pv.size() < needed) {
          ++short_lines;
          continue;
        }

        // Long enough to be walked to the end, so it must end where the score
        // says. This one is not budget-dependent and has no tolerance.
        std::string why;
        if (!line_ends_in_mate(game.fen, game.moves, report.ply, report.pv,
                               needed, why)) {
          unreached.push_back(where + ": " + why);
        }
      }
    }

    if (mate_lines > 0) {
      ++reporting;
    } else {
      quiet += (quiet.empty() ? "" : ", ") + game.name;
    }

    std::string report;
    for (const std::string& failure : unreached) {
      report += "\n  " + failure;
    }

    // DEC-122's guarantee, asserted at every budget and pinned to none: a line
    // the search published at the length its score claims reaches the mate, or
    // it is a line the engine cannot stand behind. 0 in 428 mate lines across
    // the whole sweep grid when this was written (S204).
    CHECK_MESSAGE(unreached.empty(),
                  (game.name + ": " + std::to_string(unreached.size()) +
                   " of " + std::to_string(mate_lines) +
                   " mate lines run their claimed distance and do not end in "
                   "checkmate -- DEC-122 says the walk extends only to a mate, "
                   "so this is a published line the engine cannot back:" +
                   report));

    // S202's residue, bounded rather than assumed away. A ceiling that is
    // exceeded is either a regression in the walk or the case having moved to
    // a cell the recorded grid does not cover -- re-run
    // adocs/data/S203_case_sweep.sh before deciding which, and never raise the
    // ceiling to match a run.
    CHECK_MESSAGE(short_lines <= short_line_ceiling(game.name),
                  (game.name + " reported " + std::to_string(short_lines) +
                   " short mating PVs of " + std::to_string(mate_lines) +
                   " mate lines, ceiling " +
                   std::to_string(short_line_ceiling(game.name)) +
                   " -- the worst cell the recorded grid shows for this case "
                   "(DEC-162)"));
  }

  // The precondition, over the set instead of per case. A case whose budget no
  // longer lands on a cell that reports a mate is a cell moving in a sparse
  // grid -- C reports one at one budget of nine -- and asserting on it per case
  // re-pinned the golden at every tree-moving change, which is the defect S204
  // exists for. A majority falling silent is not that: it is a Zobrist redraw
  // (three or four of six went quiet on every seed tried) or the engine no
  // longer finding these mates, and either one makes the assertions above
  // vacuous. DEC-161, DEC-162.
  //
  // A strict majority, more than half. Until S202 this read
  // `(guarded + 1) / 2`, which is the same 3 of 5 for an odd set and half --
  // 3 of 6 -- once F made the set even; guarding a sixth case must not loosen
  // the precondition, so it is 4 of 6 now. S202, DEC-251.
  const size_t majority = guarded / 2 + 1;

  CHECK_MESSAGE(
      reporting >= majority,
      (std::to_string(reporting) + " of " + std::to_string(guarded) +
       " guarded cases reported a mate line, " + std::to_string(majority) +
       " needed -- the set has gone vacuous and the budgets need "
       "re-sweeping with adocs/data/S203_case_sweep.sh, not the "
       "assertions relaxing. Silent: " +
       quiet));

  uci_shutdown();
}
