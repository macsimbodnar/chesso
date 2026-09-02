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

// S147. Every `info` line that claims a mate carries a line long enough to
// reach it, and the position that line ends on is checkmate.
//
// WHAT IS BEING GUARDED. `state->pv_table` is filled from the main search, so
// it holds at most one move per main-search ply. A mate found inside
// quiescence puts a correct mate score at a node whose line continues below
// the deepest main-search ply, and the reported line stops where the table
// stops. Measured in S145 over 400 positions: 31 `info` lines carried a mate
// score and 2 of them were shorter than the distance they claimed, 0 had the
// wrong distance. The score was right and the line was short, which is why
// this is a reporting test and not a mate-finding one.
//
// WHY EVERY ITERATION AND NOT THE LAST. The truncation is a shallow-iteration
// effect: a line is exactly as long as the iteration is deep, so it is short
// only while the iteration is shallower than the mate. Reading the last `info`
// line -- which is what tests/test_mate_breadth.cpp does, and what a GUI acting
// on the result does -- sees the repaired line and never the broken one. Both
// of S145's two short lines came from an iteration that was not the last.
//
// WHY BOTH SETS. adocs/data/S145_mate_set.tsv is 82 constructed mates in two
// and three, where a mate is reported at almost every iteration and the
// truncation is dense. adocs/data/S145_mined_set.tsv is 318 positions from
// chesso's own games, where mates are sparse and the lines are whatever play
// produced. The first is the concentrated case and the second is the one
// nobody chose.
//
// This is the same property `fastchess -check-mate-pvs` asserts on every SPRT
// since S145, over the positions two engines actually meet. That check is the
// wider net and this one is the reproducible one: a red here names the FEN and
// the depth, and it fires in the completion gate rather than after a night of
// games.

#ifndef CHESSO_SOURCE_DIR
#error "CHESSO_SOURCE_DIR must be defined so the test can read the mate sets"
#endif


// Deep enough that both sets report mates -- the constructed set is mates in
// two and three, and the mined set is labelled at six or less -- and shallow
// enough to stay in the fast gate: 400 searches cost about 4 s in Release
// here, against test_mate_breadth's 18 s at depth 10. Depth buys mate *finding*
// and this file asserts nothing about how many mates are found, so it buys
// nothing here beyond the iterations it adds.
static constexpr int SEARCH_DEPTH = 8;


// The board the reported line is replayed on. Separate from the game the UCI
// layer searches with, so that replaying a line cannot disturb the engine
// state the next position is searched from.
static game_t replay;


// Column one of a tracked TSV, comments and header skipped. Both sets have the
// same first two columns and this file needs only the first.
static std::vector<std::string> read_fens(const std::string& name)
{
  const std::string path =
      std::string(CHESSO_SOURCE_DIR) + "/adocs/data/" + name;

  std::ifstream file(path);
  REQUIRE_MESSAGE(file.good(), ("Cannot open " + path));

  std::vector<std::string> fens;
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') { continue; }

    const size_t first = line.find('\t');
    if (first == std::string::npos) { continue; }

    const std::string fen = line.substr(0, first);
    if (fen == "fen") { continue; }

    fens.push_back(fen);
  }

  return fens;
}


struct mate_line_t
{
  int depth;
  int mate_in;
  std::vector<std::string> pv;
};


// Every `info` line of one iterative deepening search that carried a mate
// score, in the order they were printed.
//
// Driven through [go] rather than by calling the search directly, for the
// reason tests/test_engine.cpp's deepen() records: iterative_deepening_search()
// does not clear stop_search_signal, so a direct call after any earlier case
// aborts at its first iteration.
static std::vector<mate_line_t> mate_lines(const std::string& fen, int depth)
{
  {
    // Every position is its own game, so that what a line contains is a
    // property of the position and not of what an earlier one left in the
    // table.
    stdout_capture_t capture;
    uci_process_line("ucinewgame");
    uci_process_line("position fen " + fen);
  }

  std::vector<std::string> lines;
  {
    stdout_capture_t capture;
    uci_process_line("go depth " + std::to_string(depth));
    uci_wait_for_search();
    lines = capture.lines();
  }

  std::vector<mate_line_t> result;

  for (const std::string& line : lines) {
    std::istringstream stream(line);
    const std::vector<std::string> token{
        std::istream_iterator<std::string>(stream),
        std::istream_iterator<std::string>()};

    if (token.size() < 4 || token[0] != "info" || token[1] != "score") {
      continue;
    }
    if (token[2] != "mate") { continue; }

    mate_line_t entry{0, std::stoi(token[3]), {}};

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


// The number of plies a mate at this distance takes. A mate the side to move
// delivers is 2N - 1 plies -- it moves last -- and one it receives is 2|N|.
static size_t plies_for(int mate_in)
{
  return (mate_in > 0) ? static_cast<size_t>(2 * mate_in - 1)
                       : static_cast<size_t>(-2 * mate_in);
}


// Plays the first `count` moves of the reported line onto `replay` and answers
// whether the position they reach is checkmate. Every move is looked up in the
// engine's own generator, so a token that names no legal move is a failure and
// not a silent skip.
static bool line_ends_in_mate(const std::string& fen,
                              const std::vector<std::string>& pv,
                              size_t count,
                              std::string& why)
{
  REQUIRE(load_FEN(fen, &replay));

  for (size_t i = 0; i < count; ++i) {
    const std::optional<uci_move_t> parsed = algebraic_to_uci_move(pv[i]);

    if (!parsed.has_value()) {
      why = "unparseable move " + pv[i];
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
      why = "illegal move " + pv[i] + " at ply " + std::to_string(i);
      return false;
    }
  }

  move_t replies[MAX_MOVES];
  const size_t count_replies =
      generate_moves(game_tables(), &replay.board, replies);

  // generate_moves() is legal-only (INV-1), so an empty list while in check is
  // checkmate by definition and no per-move legality pass is wanted here.
  if (count_replies > 0) {
    why = "the position after " + std::to_string(count) +
          " plies has legal replies";
    return false;
  }

  if (!is_check(&replay)) {
    why = "the position after " + std::to_string(count) + " plies is stalemate";
    return false;
  }

  return true;
}


// One set, every position, every mate line. Failures are collected rather than
// asserted one at a time: the count is what says whether a change made this
// worse or only moved it, and a REQUIRE on the first offender hides the rest.
static void check_set(const std::string& name, size_t expected_rows)
{
  const std::vector<std::string> fens = read_fens(name);
  REQUIRE_MESSAGE(fens.size() == expected_rows,
                  (name + " is not the tracked set any more: " +
                   std::to_string(fens.size()) + " rows, expected " +
                   std::to_string(expected_rows)));

  int mate_lines_seen = 0;
  int offending = 0;
  std::string offenders;

  for (const std::string& fen : fens) {
    for (const mate_line_t& line : mate_lines(fen, SEARCH_DEPTH)) {
      mate_lines_seen++;

      const size_t needed = plies_for(line.mate_in);
      std::string why;

      if (line.pv.size() < needed) {
        why = "the line is " + std::to_string(line.pv.size()) +
              " plies and the distance needs " + std::to_string(needed);
      } else if (!line_ends_in_mate(fen, line.pv, needed, why)) {
        // why is set by line_ends_in_mate()
      } else {
        continue;
      }

      offending++;
      offenders += "\n  " + fen + " at depth " + std::to_string(line.depth) +
                   " reported mate " + std::to_string(line.mate_in) + ": " +
                   why;
    }
  }

  MESSAGE(name, " at depth ", SEARCH_DEPTH, ": ", mate_lines_seen,
          " mate lines, ", offending, " incomplete");

  REQUIRE_MESSAGE(
      mate_lines_seen > 0,
      (name + " produced no mate score at all at depth " +
       std::to_string(SEARCH_DEPTH) + ", so this file asserted nothing"));

  REQUIRE_MESSAGE(offending == 0,
                  ("a mate score was reported with a line that does not reach "
                   "the mate it claims:" +
                   offenders));
}


TEST_SUITE("mate PV completeness")
{
  TEST_CASE("the constructed set reports no mate it cannot show")
  {
    uci_init();
    initialize_game_const_data(&replay);

    check_set("S145_mate_set.tsv", 82);

    uci_shutdown();
  }

  TEST_CASE("the mined set reports no mate it cannot show")
  {
    uci_init();
    initialize_game_const_data(&replay);

    check_set("S145_mined_set.tsv", 318);

    uci_shutdown();
  }
}
