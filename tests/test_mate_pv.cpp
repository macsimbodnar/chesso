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
#include "search.hpp"
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
// AND A NODE-LIMITED CELL, S294'S. The two sets search to a fixed depth, so
// every iteration they print completed. One case below ends its search inside
// an iteration instead, which is the one place the line printed and the score
// printed come from different iterations, on a cold table: it guards S202's
// aborted-line fallback for a line the walk leaves short (DEC-251). S295's
// cell, the same path for a line already past the claimed distance and not
// mate there (DEC-317), is retired since S296, which removed the short mate
// it rested on. The last two cases drive the same line choices on
// constructed lines, for the branches no search measured reaches: the aborted
// path's, and the cut search() makes on a completed line.

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


// S295's constructed lines on S294's cell, for the two cases at the end of the
// suite: the position, the mate the completed iteration claimed there, and
// four lines on it. Why each is what it is, and that none is a chess
// judgement, is said beside the first of those cases.
struct constructed_t
{
  const std::string fen = "6rk/5rqp/p7/2P1p3/P3Q3/7n/1P4P1/R2R3K b - - 1 32";
  const int mate_in = 4;
  const size_t needed = plies_for(4);

  const std::vector<std::string> completed = {"h3f2", "h1h2", "g7h6", "e4h4",
                                              "h6h4", "h2g1", "h4h1"};
  const std::vector<std::string> long_same = {
      "h3f2", "h1g1", "f2e4", "d1d2", "e4d2", "g2g3", "g7g3", "g1h1", "g3h4"};
  const std::vector<std::string> other = {"a6a5", "a1a2", "f7a7", "a2a1",
                                          "a7a6", "a1a2", "a6a7", "a2a1"};
  const std::vector<std::string> short_same = {"h3f2", "h1h2", "g7h6"};

  static std::vector<std::string> prefix(const std::vector<std::string>& line,
                                         size_t count)
  { return {line.begin(), line.begin() + static_cast<long>(count)}; }

  // By the engine's generator: the completed line is checkmate at the
  // distance; neither long line nor `other` cut to it is; `short_same` is the
  // completed line's start and short of it; `other` starts with another move.
  void require_preconditions() const
  {
    std::string why;
    REQUIRE(completed.size() == needed);
    REQUIRE_MESSAGE(line_ends_in_mate(fen, completed, needed, why), why);
    REQUIRE(long_same.size() > needed);
    REQUIRE_FALSE(line_ends_in_mate(fen, long_same, needed, why));
    REQUIRE(other.size() > needed);
    REQUIRE_FALSE(line_ends_in_mate(fen, other, needed, why));
    REQUIRE(other[0] != completed[0]);
    REQUIRE(short_same.size() < needed);
    REQUIRE(short_same == prefix(completed, short_same.size()));
  }
};


// A line of UCI moves as the engine's moves, each looked up in the generator
// on the board it is played from. Uses `replay`, which it leaves after them.
static pv_t constructed_pv(const std::string& fen,
                           const std::vector<std::string>& uci)
{
  REQUIRE(load_FEN(fen, &replay));
  pv_t pv = {};

  for (const std::string& token : uci) {
    const std::optional<uci_move_t> parsed = algebraic_to_uci_move(token);
    REQUIRE(parsed.has_value());

    move_t moves[MAX_MOVES];
    const size_t count = generate_moves(game_tables(), &replay.board, moves);

    move_t chosen = 0;
    for (size_t m = 0; m < count; ++m) {
      if (MOVE_FROM(moves[m]) == parsed->from &&
          MOVE_TO(moves[m]) == parsed->to &&
          MOVE_PROMOTED(moves[m]) == parsed->promotion) {
        chosen = moves[m];
      }
    }

    REQUIRE_MESSAGE(chosen != 0, ("illegal move " + token));
    REQUIRE(make_move(&replay, chosen));
    pv.table[pv.length++] = chosen;
  }

  return pv;
}


static std::vector<std::string> constructed_uci(const pv_t& pv)
{
  std::vector<std::string> out;

  for (size_t i = 0; i < pv.length; ++i) {
    const uci_move_t move = {MOVE_FROM(pv.table[i]), MOVE_TO(pv.table[i]),
                             MOVE_PROMOTED(pv.table[i])};
    out.push_back(uci_move_to_algebraic(&move));
  }

  return out;
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
  // GOLDEN (DEC-142): the cell -- the position, line 114 of
  // adocs/data/S145_mined_set.tsv (game 4543), at `go nodes 250000` on a cold
  // table at Hash 16 -- the lower middle of the run of budgets, 150000 to
  // 360000, where the shipped build prints a line that reaches its mate and
  // the build with MW02 applied prints the aborted line short. Since S125,
  // whose pawn structure family left line 101's run at 30000..60000 and opened
  // this longer one, so the rule takes this position (4 of 318 positions
  // separate; adocs/data/S125_goldens.txt). S123's passed pawn family had
  // moved line 101's run back from S121's 40000..70000 (3 of 318;
  // adocs/data/S123_goldens.txt), the budget 50000 -> 40000 at line 101 (game
  // 3499); S121's mobility table had moved it from
  // S296's 30000..60000 (2 of 318; adocs/data/S121_goldens.txt), the budget
  // 40000 -> 50000; since S296 the position; S294 picked line 162 at
  // `go nodes 150000`, whose run S296's tree shortened to 180000..200000
  // (adocs/data/S296_goldens.txt).
  // Re-derive: adocs/data/S294_fallback_cell.py, the four stages in its
  // header (sweep both builds over the mined set, then `pick`).
  // Margin: 100000 nodes below and 110000 above. Moves legitimately
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

    const std::string fen = "8/5Q1p/p5pk/3P4/8/q1N4P/2K2P2/4R3 w - - 1 41";

    {
      stdout_capture_t capture;
      uci_process_line("setoption name Hash value 16");
      uci_process_line("ucinewgame");
      uci_process_line("position fen " + fen);
    }

    std::vector<std::string> lines;
    {
      stdout_capture_t capture;
      uci_process_line("go nodes 250000");
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

  // S295. The other side of the same path. The walk extends a short line only
  // to a checkmate and leaves one already as long as the mate needs as it is,
  // so an aborted line that runs to a *longer* mate came out of it unchanged,
  // S202's fallback -- keyed on a line still too short -- did not run, and the
  // line was printed beside the shorter completed score, `ponder` its second
  // move: 9 plies beside `mate 4`, checkmate only at ply 9
  // (adocs/data/S294_aborted_long_line.txt, S294's finding, DEC-317). Since
  // S295 a line at or past the distance that is not checkmate there gives way
  // to the completed line of the same move, or is cut below the distance.
  // Mutant MW03 of tools/mutants/S202_mate_walk.py takes that out.
  //
  // **Its node-limited cell is retired at S296.** It was line 168 of
  // adocs/data/S145_mined_set.tsv (game 5850) at `go nodes 140000`, picked by
  // adocs/data/S295_long_line.py, and it existed only because the completed
  // depth-8 iteration read `mate 4` there, one move short of the position's
  // mate in 5 (adocs/data/S295_trace.txt). S296's rule removes that short
  // mate: the iteration reads `mate 5` with a 9-ply line, the aborted line is
  // no longer past the distance, and the script's pick over the whole mined
  // grid on S296's tree finds no cell that separates (0 of 318,
  // adocs/data/S296_goldens.txt). The path itself stays guarded by the
  // constructed-lines case below, which kills MW03 and MW04 without a search.

  // S295. The same line choices driven directly, for the branches no search
  // reaches: an aborted line of another first move than the completed one, at
  // or past the claimed distance and not checkmate there, which has no
  // completed line of the same choice to give way to and is cut to one ply
  // under the distance; and a completed line the last completed iteration
  // offers that does not reach its own mate, which cannot stand in. Over the
  // mined grid, the S170 grid and a wider scan of 408 positions at Hash 1 and
  // 16 no cell prints a wrong line of another first move
  // (adocs/data/S295_census.txt), so a node-limited cell cannot guard either.
  // uci_aborted_mate_line_probe() is the function iterative_deepening_search()
  // calls after the walk, and the case hands it S294's cell's position and
  // lines constructed on it, the last completed line among them. Mutant MW04
  // of tools/mutants/S202_mate_walk.py takes the cut out, MW05 the completed
  // line's own check, MW02 and MW03 the two halves of the fallback.
  //
  // The lines are not chess judgements. `completed` and `long_same` are the
  // engine's own lines at that cell (adocs/data/S294_aborted_long_line.txt);
  // `other` is the first legal move other than h3f2 and then the first legal
  // move in UCI order at every ply, which python-chess finds legal throughout
  // and checkmate nowhere; `short_same` is `completed`'s first three plies.
  // Whether each is checkmate at its length is asserted by the engine's own
  // generator, before any choice is read.
  TEST_CASE("the aborted path's line choice, on constructed lines")
  {
    uci_init();
    initialize_game_const_data(&replay);

    const constructed_t c;
    c.require_preconditions();

    // The choice, on the cell's position, which it must leave as it found it.
    const auto choose = [&](const std::vector<std::string>& line,
                            const std::vector<std::string>& last_line) {
      pv_t chosen = constructed_pv(c.fen, line);
      const pv_t last = constructed_pv(c.fen, last_line);

      REQUIRE(load_FEN(c.fen, &replay));
      const hash_t before = replay.board.hash;
      uci_aborted_mate_line_probe(&replay, &chosen, &last, c.mate_in);
      CHECK(replay.board.hash == before);

      return constructed_uci(chosen);
    };

    // The cut: a wrong line of another first move, past the distance and at
    // exactly it, is left one ply under the distance, its own first move kept.
    CHECK(choose(c.other, c.completed) == c.prefix(c.other, c.needed - 1));
    CHECK(choose(c.prefix(c.other, c.needed), c.completed) ==
          c.prefix(c.other, c.needed - 1));

    // The fallback, both halves: a long line and a short line of the
    // completed line's first move give way to it (S295's half, S202's).
    CHECK(choose(c.long_same, c.completed) == c.completed);
    CHECK(choose(c.short_same, c.completed) == c.completed);

    // The completed line stands in only if it reaches its own mate: offered a
    // short one of the same first move instead, the long line is cut below
    // the distance, h3f2 h1g1 f2e4 d1d2 e4d2 g2g3, and not replaced.
    CHECK(choose(c.long_same, c.short_same) ==
          c.prefix(c.long_same, c.needed - 1));

    // A line that reaches its mate stands, and a short line of another move
    // stays short (DEC-122).
    CHECK(choose(c.completed, c.completed) == c.completed);
    CHECK(choose(c.prefix(c.other, 3), c.completed) == c.prefix(c.other, 3));

    uci_shutdown();
  }

  // S295, at the coordinator's fast check. The same rule for a completed
  // iteration: search() hands its reported line to cut_unreached_mate_pv()
  // after the walk, which leaves a line already as long as the mate needs as
  // it is, mate or not, so a completed line at or past the distance that is
  // not checkmate there is cut to one ply under it. No completed line of the
  // census grids is one (adocs/data/S295_census.txt), so the function search()
  // calls is driven here on the constructed lines above. Mutant MW06 of
  // tools/mutants/S202_mate_walk.py takes the cut out. That search() makes the
  // call at all is read by no case: no search measured reaches it.
  TEST_CASE(
      "a mate line at or past its distance and not mate there is cut, "
      "on constructed lines")
  {
    uci_init();
    initialize_game_const_data(&replay);

    const constructed_t c;
    c.require_preconditions();

    const auto cut = [&](const std::vector<std::string>& line, bool expected,
                         int mate_in) {
      pv_t pv = constructed_pv(c.fen, line);

      REQUIRE(load_FEN(c.fen, &replay));
      const hash_t before = replay.board.hash;
      CHECK(cut_unreached_mate_pv(&replay, &pv, mate_in) == expected);
      CHECK(replay.board.hash == before);

      return constructed_uci(pv);
    };

    // Past the distance and at exactly it, not mate there: cut one ply under.
    CHECK(cut(c.long_same, true, c.mate_in) ==
          c.prefix(c.long_same, c.needed - 1));
    CHECK(cut(c.other, true, c.mate_in) == c.prefix(c.other, c.needed - 1));
    CHECK(cut(c.prefix(c.other, c.needed), true, c.mate_in) ==
          c.prefix(c.other, c.needed - 1));

    // Checkmate at the distance, or short of it: left as it is.
    CHECK(cut(c.completed, false, c.mate_in) == c.completed);
    CHECK(cut(c.short_same, false, c.mate_in) == c.short_same);

    // Beside `mate 1` a refused one-move line has no ply under the distance:
    // the move played is all it has, so it stays, and the function still
    // answers that it was refused. And `mated 1`, two plies, is cut to its
    // first. `other`'s first move and first two plies are checkmate nowhere
    // (asserted here).
    std::string why;
    REQUIRE_FALSE(line_ends_in_mate(c.fen, c.other, 1, why));
    REQUIRE_FALSE(line_ends_in_mate(c.fen, c.other, 2, why));
    CHECK(cut(c.prefix(c.other, 1), true, 1) == c.prefix(c.other, 1));
    CHECK(cut(c.prefix(c.other, 2), true, -1) == c.prefix(c.other, 1));

    // `mate 0`, a root already mated, has no line and none is made: the one
    // distance with no ply under it. search() hands it exactly this.
    {
      pv_t none = {};
      REQUIRE(load_FEN(c.fen, &replay));
      CHECK_FALSE(cut_unreached_mate_pv(&replay, &none, 0));
      CHECK(none.length == 0);
    }

    uci_shutdown();
  }
}
