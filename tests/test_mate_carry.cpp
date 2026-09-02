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

  // Whether this row is a case the guard asserts on. A row that is `no` is a
  // reproduction kept for the step that owns it and not a property held here:
  // `E_mate_minus9` reports `mate -9` where the engine's own cold search says
  // the position is mate in 7 and stockfish agrees at depth 30 and 36, so
  // there is no 18-ply line for it and refusing to publish one is correct.
  // Asserting on it would be asserting that a wrong score gets a line.
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

    if (field.size() < 6 || field[0] == "name") { continue; }

    cases.push_back({field[0], field[1], split_words(field[2]), field[3],
                     static_cast<size_t>(std::stoul(field[4])),
                     field[5] == "yes"});
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
  if (name == "A_mate8_shallow") { return 5; }
  if (name == "B_mate6_shallow") { return 7; }
  if (name == "C_mate7_depth11") { return 6; }
  if (name == "D_mate_minus6_depth10") { return 1; }

  FAIL("unknown case " << name);
  return 0;
}


TEST_CASE("a mate score carried across searches keeps a line that reaches it")
{
  uci_init();
  initialize_game_const_data(&replay);

  const std::vector<case_t> cases = read_cases();
  REQUIRE_MESSAGE(cases.size() == 5,
                  ("adocs/data/S170_cases.tsv is not the tracked set any "
                   "more: " +
                   std::to_string(cases.size()) + " rows, expected 5"));

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

    for (size_t ply = game.start; ply <= game.moves.size(); ++ply) {
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
