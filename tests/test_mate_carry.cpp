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
// WHY THE PRECONDITION IS ASSERTED FIRST. What a table holds at a given ply is
// fragile, and a change that moves the tree can make a case stop reporting a
// mate at all -- at which point "no short line" is true and means nothing. So
// every case must first be seen to report a mate, and the count of mate lines
// is asserted to be at least what was measured when the case was chosen.
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

// WHAT RETIRES THIS FILE'S FIXTURE, AND WHAT DOES NOT. The class here is
// transposition-table eviction, and which entries evict which is decided by the
// Zobrist keys. So the budgets in `adocs/data/S170_cases.tsv` are valid only
// for the key set they were chosen under, and a redraw retires them -- measured
// over four arbitrary seeds, every one of which left three or four of the six
// cases reporting no mate at all. That is what the vacuity assertion below
// catches.
//
// What it does *not* mean is that the cases themselves are lost. S203 redrew
// the keys and every one of the six came back by re-sweeping its node budget
// alone, no game and no mining run: A at 1000000 nodes instead of 300000, C at
// 1500000 instead of 1000000, D at 4000000 instead of 1000000. The rule is
// stated once in `adocs/data/S203_case_sweep.sh` and applied to every row --
// the cheapest budget at which the case reports at least its floor of mate
// lines with all of them complete -- because a budget chosen per case because
// it happened to be green would be fitting the fixture to the test.
//
// The budgets are a knife edge and the file says so rather than implying it:
// C reports 13 mate lines at 1500000 nodes and 0 at both 1000000 and 2000000.
// Expect to re-run that sweep after any change that moves the tree, not only
// after a key change. DEC-154, DEC-156.
//
// D carries one more thing. Between 1200000 and 3000000 nodes it reproduces a
// short line -- `mate -6` at ply 35 depth 11 with a 10-of-12-ply PV, at a depth
// that also publishes a complete 12/12 -- which is the class DEC-122 leaves
// short and visible and S202 owns. It is recorded there as a reproduction
// rather than hidden behind the 4000000 that clears it.

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
  // `F_mate6_inherited_no_line` is `no`: it is the reproduction of the 8 lines
  // S171's own census left, measured and not yet closed. Its `mate 6` is the
  // position's true distance -- stockfish gives `#+6` at depth 20 and 30 --
  // read off the table at depth 3 on 1224 nodes, too shallow to build the
  // 11 plies it needs, and the line that iteration did build continues in the
  // table into a chain proving `mate 8`. Both lookups in the walk are keyed on
  // the distance still owed, so both refuse it, and all-or-nothing leaves the
  // line short and visible, which is what it is for. No walk can close this
  // one: the 11-ply line the score names is not in the table to be found, and
  // building it would mean searching, which this path may not do. The same
  // three lines reproduce byte for byte on 457e355, so nothing in S171 caused
  // it.
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


// The mate lines each case reported when it was chosen, 2026-09-02. A floor
// and not an equality: a change that finds *more* mates is not a regression,
// and one that finds none has made the case vacuous, which is the thing this
// number exists to catch.
static size_t expected_mate_lines(const std::string& name)
{
  // Re-derived 2026-09-08 by adocs/data/S203_case_sweep.sh, which is the script
  // DEC-142 requires beside a golden. The rule, stated once and applied to
  // every row rather than tuned per case: a floor is half the mate lines the
  // case reports at its own budget, rounded down. Half and not the count
  // itself, because the count swings with the table -- C reports 13 lines at
  // 1500000 nodes and 0 at both 1000000 and 2000000 -- and a floor set at the
  // observation would break on drift the guard does not care about. What it has
  // to catch is a case that reports no mate at all, and any positive floor does
  // that.
  //
  // A row whose budget did not move keeps the floor it was measured with: B
  // reports 8 against its 7 and E reports 23 against its 9, so nothing about
  // them was re-chosen and re-deriving them would only churn the record. A, C
  // and D were re-swept and carry new floors; A and D rose, so no floor here
  // was lowered against a live guard. F is `guard no` and its floor was stale
  // at 6 against 4 reported -- corrected to 2 so the number means something if
  // the row is ever guarded.
  //
  //   case                       budget      reports  floor
  //   A_mate8_shallow            1000000       12       6
  //   B_mate6_shallow             100000        8       7   (unchanged)
  //   C_mate7_depth11            1500000       13       6
  //   D_mate_minus6_depth10      4000000        6       3
  //   E_mate_minus9              1500000       23       9   (unchanged)
  //   F_mate6_inherited_no_line   300000        4       2   (not guarded)
  if (name == "A_mate8_shallow") { return 6; }
  if (name == "B_mate6_shallow") { return 7; }
  if (name == "C_mate7_depth11") { return 6; }
  if (name == "D_mate_minus6_depth10") { return 3; }
  if (name == "E_mate_minus9") { return 9; }
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

  for (const case_t& game : cases) {
    if (!game.guard) { continue; }

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
    std::vector<std::string> failures;

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

        if (report.pv.size() < needed) {
          failures.push_back(where + ": pv is " +
                             std::to_string(report.pv.size()) + " plies, " +
                             std::to_string(needed) + " needed");
          continue;
        }

        std::string why;
        if (!line_ends_in_mate(game.fen, game.moves, report.ply, report.pv,
                               needed, why)) {
          failures.push_back(where + ": " + why);
        }
      }
    }

    CHECK_MESSAGE(mate_lines >= expected_mate_lines(game.name),
                  (game.name + " reported " + std::to_string(mate_lines) +
                   " mate lines, at least " +
                   std::to_string(expected_mate_lines(game.name)) +
                   " when the case was chosen -- the case has gone vacuous "
                   "and needs re-choosing, not deleting"));

    std::string report;
    for (const std::string& failure : failures) {
      report += "\n  " + failure;
    }

    CHECK_MESSAGE(failures.empty(),
                  (game.name + ": " + std::to_string(failures.size()) + " of " +
                   std::to_string(mate_lines) +
                   " mate lines do not reach their mate:" + report));
  }

  uci_shutdown();
}
