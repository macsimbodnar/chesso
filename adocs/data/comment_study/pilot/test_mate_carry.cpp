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
//   cases. Without it "no short line" is true and means nothing. One case
//   falling silent is a cell moving; a majority falling silent is a key redraw
//   or the engine no longer finding these mates.
//
// THE CAUSES. A, B, C: a score inherited across searches (C reports `mate 7`
// warm and no mate at any depth cold). D: a mate proved inside this search
// whose mid-line entry was evicted before the walk (reports `mate -6` cold as
// well). E: the walk stalls on one missing slot with the certifying entry in
// the children of the position whose entry is gone, S171. F: was an aborted
// iteration's line beside the last completed `mate 6`; on today's tree this
// case no longer reaches that path, and tests/test_mate_pv.cpp's cold cell
// holds it (adocs/data/S294_mw02_reach.txt).
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
//                                        the cheapest budget at the row's
//                                        stride whose cell reports a mate line
//   adocs/data/S170_pick_budgets.sh adocs/data/S170_cases.tsv
//       adocs/data/<step>_sweep_s170.txt > NEW.tsv

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


// GOLDEN (DEC-142): the most short mating PVs a case may report, one per
// replayed game of adocs/data/S170_cases.tsv at its own budget and stride. Each
// is the largest short-line count any cell of the recorded grid shows for that
// case, at the stride its TSV row carries, over every recorded sweep in the
// command. Re-derive, never reading one off a failing run:
//
//   adocs/data/S203_case_sweep.sh --ceilings
//       adocs/data/S202_sweep_s170.txt adocs/data/S261_sweep_s170.txt
//       adocs/data/S286_sweep_s170.txt adocs/data/S292_sweep_s170.txt
//       adocs/data/S293_sweep_s170.txt adocs/data/S083s_sweep_s170.txt
//       adocs/data/S135_sweep_s170.txt adocs/data/S296_sweep_s170.txt
//       adocs/data/S121_sweep_s170.txt adocs/data/S123_sweep_s170.txt
//       adocs/data/S125_sweep_s170.txt
//
// Moves legitimately on: a re-sweep of the budgets, which is a Zobrist redraw
// (DEC-154); a change to complete_mate_pv() or to the reporting layer's aborted
// path (DEC-251). A tree-moving step that re-sweeps adds its grid to the
// command; grids older than S202's read a bound entry's move first and stay
// out, and a revert of S202 puts them back. Margin: 0. Lowering one is progress
// on the class. Raising one is relaxing a test: DEC-299's guard (+2, both mate
// floors holding) or the owner's decision. Property beside it: "a mate score
// carried across searches keeps a line that reaches it", whose other two
// assertions carry no per-case number and do not move with the tree.
//
//   case                       ceiling  worst cell, at the case's own stride
//   A_mate8_shallow                  2  1000000 on the S286 grid; DEC-295
//   B_mate6_shallow                 13  1200000 on the S121 grid; DEC-327,
//   DEC-328 C_mate7_depth11                  4  4000000 on the S202 grid
//   D_mate_minus6_depth10            1  4000000 on the S135 grid; DEC-299's
//   guard E_mate_minus9                    2  2000000 on the S135 grid;
//   DEC-299's guard F_mate6_inherited_no_line        3  300000 on the S083s
//   grid, stride 2; DEC-307
//
// Why they move: a line the search never stored is a line the walk cannot
// certify, so any change to the tree changes the residue. `unreached.empty()`
// is the guarantee beside them and a rise must not touch it. A ceiling of 0
// says the grid never showed one; a budget where none happens to appear says
// nothing.
static size_t short_line_ceiling(const std::string& name)
{
  if (name == "A_mate8_shallow") { return 2; }
  if (name == "B_mate6_shallow") { return 13; }
  if (name == "C_mate7_depth11") { return 4; }
  if (name == "D_mate_minus6_depth10") { return 1; }
  if (name == "E_mate_minus9") { return 2; }
  if (name == "F_mate6_inherited_no_line") { return 3; }

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
  // A strict majority, more than half: guarding a sixth case must not loosen
  // the precondition. DEC-251.
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
