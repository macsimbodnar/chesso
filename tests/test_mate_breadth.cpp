#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "test_helpers.hpp"
#include "uci.hpp"

// S156. The mined breadth set, asserted as a count with a floor.
//
// adocs/data/S145_mined_set.tsv is 318 positions taken one per game from the
// final position of chesso's own SPSA games and labelled by stockfish at a node
// limit - the label cannot come from the engine under test. S145 built it and
// scored it once by hand; 2026-08-21_adversarial-F08 found that nothing read
// it afterwards, and this file is that finding closed.
//
// WHY A COUNT AND NEVER PER POSITION. A search is allowed to miss any
// particular deep mate, so a suite that forbids it is a suite that gets
// switched off. Of seventeen engines S145 surveyed, two wrote exact
// mate-distance tests, watched their own pruning break them, and disabled the
// tests rather than the pruning - one marking them "requires no pruning", the
// other deleting the CI step as flaky. A floor under a count is a claim a
// search can keep: the engine finds at least this many of these, and a change
// that finds fewer has cost something.
//
// This is the breadth half of the mate gate and not the guard half. The
// constructed set in tests/test_engine.cpp is built so that the reverse
// futility hazard is present in every position - the mated side materially
// ahead, its static score a margin clear of beta - and every position there is
// asserted. Nothing in the mined set was built for anything: the hazard occurs
// in it only as often as it occurs in play, which S145 measured at roughly
// never (of 191 sampled positions where the side to move is mated within six,
// one has a non-negative score for the mated side, median -1093). So this file
// cannot replace that one and does not try to. What it covers is the other
// direction: a change that costs mate finding generally, anywhere in the tree,
// on positions nobody chose.
//
// WHERE THE FLOOR COMES FROM. Measured at HEAD on the machine
// .moltke.local.md describes, depth 10, Hash at its default 16:
//
//                          exact   any   wrong sign
//   RfpMinPly 3 (ships)      145   147            0
//   RfpMinPly 2              145   147            0
//   RfpMinPly 1              141   143            0
//   RfpMinPly 0              141   143            0
//
// 143 sits strictly between the shipping value and the value the weakened
// guard produces, which is the property that matters: it goes red when the
// guard goes and not when the tree shifts under it. S145 placed the same floor
// on the same rule at 146 against 139; the tree has moved since - S142, S149
// and S165 all alter play - and the gap has narrowed from 7 to 4, which is a
// reason to re-measure the floor when it next goes red rather than to lower it
// reflexively.
//
// Reproduce the table with adocs/data/S156_mined_floor_sweep.py. It needs a
// patched bound and says why: S142 narrowed RfpMinPly's minimum to 2, so the
// values that produce the red reading cannot be reached through setoption any
// more and the sweep builds a throwaway worktree to reach them. That is the
// point rather than a limitation - the front line is now a constant in
// src/search_params.hpp, and this test is what stands behind it.
//
// WRONG SIGN IS NOT A COUNT. A mate score for the side being mated is a defect
// at any total, so it is asserted at zero and not against a floor.

#ifndef CHESSO_SOURCE_DIR
#error "CHESSO_SOURCE_DIR must be defined so the test can read the mined set"
#endif


// Measured at depth 10 because that is where the floor was placed and where
// the separation above was read. Depth is not free here: the same 318
// positions cost about 3 s at depth 8 and about 18 s at depth 10 on this
// machine, which is why this is its own binary rather than another case inside
// test_engine.
static constexpr int SCORE_DEPTH = 10;

// GOLDEN (DEC-142): 143, the fewest exact mate distances the engine may find
// over the 318 mined positions of the tracked TSV at depth 10. Its two ends are
// tabled above: 145 with the guard shipping, 141 with it weakened.
// Re-derive: python3 adocs/data/S156_mined_floor_sweep.py, which builds a
// throwaway worktree because the red end is no longer reachable by setoption.
// Moves legitimately on: a search change that costs or buys mate finding --
// re-derive when either end moves, and never read a replacement off the failing
// run.
// Margin: 2 below the shipping count, 2 above the weakened one. The gap was 7
// when S145 placed the same floor at 146 and is 4 now, so a red here is a
// reason to re-measure the table rather than to lower the number.
// Property beside it: the `wrong_sign == 0` assertion in the same case -- a
// mate score for the side being mated is a defect at any total, so it is
// asserted at zero and carries no floor.
static constexpr int EXACT_FLOOR = 143;


struct row_t
{
  std::string fen;
  int distance;
};


// The tracked TSV, comments and header skipped. Read rather than restated: a
// list of 318 FENs pasted into a test is a list whose derivation is lost, and
// adocs/data/S145_mined_set.py is how this one is regenerated.
static std::vector<row_t> read_mined_set()
{
  const std::string path =
      std::string(CHESSO_SOURCE_DIR) + "/adocs/data/S145_mined_set.tsv";

  std::ifstream file(path);
  REQUIRE_MESSAGE(file.good(), ("Cannot open " + path));

  std::vector<row_t> rows;
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') { continue; }

    const size_t first = line.find('\t');
    if (first == std::string::npos) { continue; }

    const std::string fen = line.substr(0, first);
    if (fen == "fen") { continue; }

    const size_t second = line.find('\t', first + 1);
    const std::string distance = line.substr(
        first + 1,
        second == std::string::npos ? std::string::npos : second - first - 1);

    rows.push_back({fen, std::stoi(distance)});
  }

  return rows;
}


struct verdict_t
{
  bool is_mate;
  int value;  // signed mate distance, from the side to move
};


// One iterative deepening search, and the last score the engine reported
// before bestmove - which is the answer a GUI reads and the same one
// python-chess hands back to adocs/data/S145_mined_set.py.
//
// Driven through [go] rather than by calling the search directly, for the
// reason tests/test_engine.cpp's deepen() records: iterative_deepening_search()
// does not clear stop_search_signal, so a direct call after any earlier case
// aborts at its first iteration.
static verdict_t search_position(const std::string& fen, int depth)
{
  {
    // Every position is its own game. The table is cleared between them so
    // that a count is a property of the set and not of the order it is read
    // in.
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

  verdict_t verdict{false, 0};

  for (const std::string& line : lines) {
    std::istringstream stream(line);
    std::vector<std::string> token{std::istream_iterator<std::string>(stream),
                                   std::istream_iterator<std::string>()};

    if (token.size() < 4 || token[0] != "info" || token[1] != "score") {
      continue;
    }

    verdict = {token[2] == "mate", std::stoi(token[3])};
  }

  return verdict;
}


TEST_SUITE("mate breadth: the mined set")
{
  TEST_CASE("the engine finds at least the floor of the mined mates")
  {
    uci_init();

    const std::vector<row_t> rows = read_mined_set();
    REQUIRE_MESSAGE(rows.size() == 318,
                    "the tracked mined set is 318 positions; regenerating it "
                    "with a different --games moves the floor with it");

    int found_any = 0;
    int found_exact = 0;
    int wrong_sign = 0;
    std::string offenders;

    for (const row_t& row : rows) {
      const verdict_t verdict = search_position(row.fen, SCORE_DEPTH);

      if (!verdict.is_mate) { continue; }

      if (verdict.value < 0) {
        wrong_sign++;
        offenders += " [" + row.fen + " reported mate " +
                     std::to_string(verdict.value) + ", labelled mate " +
                     std::to_string(row.distance) + "]";
        continue;
      }

      found_any++;
      if (verdict.value == row.distance) { found_exact++; }
    }

    // Printed rather than only carried into a failure message. The gate runs
    // with --output-on-failure, so this is silent in a green run and is there
    // when the binary is run by hand: the count drifting toward the floor is
    // the warning that precedes the floor going red.
    MESSAGE("mined set at depth ", SCORE_DEPTH, ": ", found_exact, " exact, ",
            found_any, " right sign, ", wrong_sign, " wrong sign");

    // The engine claiming a mate for the side that is being mated. Never a
    // count, never tolerated at any total.
    REQUIRE_MESSAGE(wrong_sign == 0,
                    ("a mate score with the wrong sign:" + offenders));

    REQUIRE_MESSAGE(
        found_exact >= EXACT_FLOOR,
        ("the mined set reads " + std::to_string(found_exact) +
         " exact at depth " + std::to_string(SCORE_DEPTH) + ", under the " +
         std::to_string(EXACT_FLOOR) +
         " this engine held when the floor was placed. Mate finding has cost "
         "something. adocs/data/S156_mined_floor_sweep.py re-measures the "
         "separation; the floor moves only with a reason written down"));

    uci_shutdown();
  }
}
