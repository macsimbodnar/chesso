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
//
// AND ONE NODE-LIMITED CELL, S294. The two sets search to a fixed depth, so
// every iteration they print completed. The last case below ends its search
// inside an iteration instead, which is the one place the line printed and the
// score printed come from different iterations: it guards S202's aborted-line
// fallback (DEC-251) on a cold table.

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

  // S294. When a search ends inside an iteration that has a line of its own,
  // iterative_deepening_search() prints that line beside the last completed
  // iteration's score and depth, completed against that score by the walk;
  // where the walk refuses, the last completed line of the same first move is
  // printed instead (S202, DEC-251). Mutant MW02 of
  // tools/mutants/S202_mate_walk.py takes that fallback out. Its guard was
  // tests/test_mate_carry.cpp's F cell, and on the tree S135 left no cell of
  // the S170 grid reaches the fallback any more -- of 62 aborted iterations
  // beside a carried mate there, none falls back
  // (adocs/data/S294_mw02_reach.txt) -- so MW02 survived the whole fast
  // suite. This cell reaches it on a cold table.
  //
  // GOLDEN (DEC-142): the cell -- the position, line 162 of
  // adocs/data/S145_mined_set.tsv (game 4593), at `go nodes 150000` on a cold
  // table at Hash 16 -- the middle of the run of budgets, 100000 to 200000,
  // where the shipped build prints a line that reaches its mate and the build
  // with MW02 applied prints the aborted line short.
  // Re-derive: adocs/data/S294_fallback_cell.py, the four stages in its
  // header (sweep both builds over the mined set, then `pick`).
  // Margin: 50000 nodes either side, the run's half-width. Moves legitimately
  // on: any change that moves the tree, and a change to complete_mate_pv() or
  // to the reporting layer's aborted path, S202's ground. Re-derived by the
  // script when either end moves, never re-read from a failing run.
  //
  // The preconditions below are what the fallback needs that a test can see
  // from outside the engine: the search ended in an iteration with a line, the
  // completed iteration before it scored a mate, and the two lines start with
  // the same move. The fallback's output is visible too: it prints the
  // completed line verbatim (iterative_deepening_search()'s aborted path), so
  // a last line that differs from the completed one proves it did not run --
  // the case's last check. The aborted line itself is not printed, so that the
  // walk refuses it is read from outside only as the script's separation on
  // the MW02 build; a last line equal to the completed one is consistent with
  // the fallback and does not prove it.
  TEST_CASE(
      "an aborted iteration's line printed beside a completed mate "
      "reaches it")
  {
    uci_init();
    initialize_game_const_data(&replay);

    const std::string fen = "6k1/3R4/2R1p1Bp/4P3/6q1/p7/P3KP2/8 w - - 5 39";

    {
      stdout_capture_t capture;
      uci_process_line("setoption name Hash value 16");
      uci_process_line("ucinewgame");
      uci_process_line("position fen " + fen);
    }

    std::vector<std::string> lines;
    {
      stdout_capture_t capture;
      uci_process_line("go nodes 150000");
      uci_wait_for_search();
      lines = capture.lines();
    }

    struct info_t
    {
      int depth;
      bool mate;
      int mate_in;
      std::vector<std::string> pv;
    };

    std::vector<info_t> infos;
    std::vector<std::string> bestmove;

    for (const std::string& line : lines) {
      std::istringstream stream(line);
      const std::vector<std::string> token{
          std::istream_iterator<std::string>(stream),
          std::istream_iterator<std::string>()};

      if (!token.empty() && token[0] == "bestmove") { bestmove = token; }

      if (token.size() < 4 || token[0] != "info" || token[1] != "score") {
        continue;
      }

      info_t entry{0, token[2] == "mate", std::stoi(token[3]), {}};

      for (size_t i = 4; i < token.size(); ++i) {
        if (token[i] == "depth" && i + 1 < token.size()) {
          entry.depth = std::stoi(token[i + 1]);
        }

        if (token[i] == "pv") {
          entry.pv.assign(token.begin() + static_cast<long>(i) + 1,
                          token.end());
          break;
        }
      }

      infos.push_back(entry);
    }

    // The search ended inside an iteration, and that iteration has a line:
    // the only state in which the aborted path runs at all.
    const std::vector<aspiration_search_t>& searches =
        uci_last_aspiration_searches();
    REQUIRE(!searches.empty());

    const aspiration_search_t& aborted = searches.back();
    REQUIRE_MESSAGE(aborted.aborted,
                    "the cell no longer ends inside an iteration; re-derive "
                    "it with adocs/data/S294_fallback_cell.py");
    REQUIRE(aborted.pv_move != 0);

    // The completed iteration before it scored a mate, so the score printed
    // beside the aborted line is a distance the line has to reach.
    const aspiration_search_t* completed = nullptr;
    for (const aspiration_search_t& search : searches) {
      if (!search.aborted) { completed = &search; }
    }
    REQUIRE(completed != nullptr);
    REQUIRE(completed->mate_found);
    REQUIRE(completed->iteration + 1 == aborted.iteration);

    // The two lines start with the same move, the fallback's own condition.
    REQUIRE(aborted.pv_move == completed->pv_move);

    // The last two `info` lines are the completed iteration's and the aborted
    // one's: the loop prints the aborted line under the completed depth and
    // score.
    REQUIRE(infos.size() >= 2);
    const info_t& last = infos.back();
    const info_t& before = infos[infos.size() - 2];
    REQUIRE(before.depth == completed->iteration);
    REQUIRE(last.depth == completed->iteration);
    REQUIRE(before.mate);
    REQUIRE(last.mate);
    REQUIRE(last.mate_in == before.mate_in);

    // The property: the line printed last reaches the mate its score claims,
    // and the move to play and the move to ponder are that line's.
    const size_t needed = plies_for(last.mate_in);
    std::string why;

    CHECK_MESSAGE(last.pv.size() >= needed,
                  ("the last line is " + std::to_string(last.pv.size()) +
                   " plies beside mate " + std::to_string(last.mate_in) +
                   ", which needs " + std::to_string(needed)));

    if (last.pv.size() >= needed) {
      CHECK_MESSAGE(line_ends_in_mate(fen, last.pv, needed, why), why);
    }

    REQUIRE(bestmove.size() == 4);
    REQUIRE(last.pv.size() >= 2);
    CHECK(bestmove[1] == last.pv[0]);
    CHECK(bestmove[2] == "ponder");
    CHECK(bestmove[3] == last.pv[1]);

    // The fallback ran: it prints the completed line verbatim, so anything
    // else is a line it did not produce. With the property above green, that
    // is the walk now completing the aborted line itself, and the case has
    // stopped guarding the fallback without failing on it -- re-derive the
    // cell rather than accept the green.
    CHECK_MESSAGE(last.pv == before.pv,
                  "the last line is not the completed iteration's, so the "
                  "aborted-line fallback did not run at this cell; re-derive "
                  "it with adocs/data/S294_fallback_cell.py");

    uci_shutdown();
  }
}
