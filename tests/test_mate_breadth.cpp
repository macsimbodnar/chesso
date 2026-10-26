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
// WHERE THE FLOOR COMES FROM. Measured at 3f7acd3 on the machine
// .moltke.local.md describes, depth 10, Hash at its default 16 (S250):
//
//                          exact   any   wrong sign
//   RfpMinPly 3 (ships)      146   149            0
//   RfpMinPly 2              146   150            0
//   RfpMinPly 1              143   145            0
//   RfpMinPly 0              143   145            0
//
// 145 sits strictly between the shipping value and the value the weakened
// guard produces, which is the property that matters: it goes red when the
// guard goes and not when the tree shifts under it. S145 placed 143 on this
// rule at 146 against 139; S156 re-measured 145 against 141 and kept 143.
// By S249 the weakened reading had risen to 143, the floor itself, and the
// gate built at the weakened default went green; S250 raised the floor to 145
// on the same rule. The gap has narrowed from 7 to 4 to 3, which is a reason
// to re-measure the floor when it next goes red, or stops going red, rather
// than to lower it reflexively.
//
// **Re-read at S116 on razoring's candidate**, the same script on a commit
// object carrying that tree: 149 exact at RfpMinPly 3, 145 at 2, 143 at 1 and
// 0, and the gate built at 1 red at 145. The shipping end rose 146 to 149 and
// the weakened end held, so the gap is 6 and the floor still separates; it is
// not moved. Razoring reads RfpMinPly as its own floor (DEC-248), which is why
// the reading at 2 fell from 146 to 145, the floor itself.
//
// Reproduce the table with adocs/data/S156_mined_floor_sweep.py. It needs a
// patched bound and says why: S142 narrowed RfpMinPly's minimum to 2 (S251 to
// 3), so the values that produce the red reading cannot be reached through
// setoption any more and the sweep builds a throwaway worktree to reach them.
// That is the point rather than a limitation - the front line is now a
// constant in src/search_params.hpp, and this test is what stands behind it.
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

// GOLDEN (DEC-142): 158, the fewest exact mate distances the engine may find
// over the 318 mined positions of the tracked TSV at depth 10. Its two ends:
// 160 with the guard shipping, 156 with it weakened (S135, below; 159 and
// 154 at S083, 163 and 160 at S293, 164 and 160 at S286).
// Re-derive: python3 adocs/data/S156_mined_floor_sweep.py --floor 158, which
// builds a throwaway worktree because the red end is no longer reachable by
// setoption; S250 read the gate built at RfpMinPly 1 red at 145.
// Moves legitimately on: a search change that costs or buys mate finding --
// re-derive when either end moves, and never read a replacement off the failing
// run.
// Margin: 2 below the shipping count and 2 above the weakened one since
// S135 (1 and 4 at S083, 0 below the shipping count at S293). The S156
// script's rule admits any floor strictly above what the weakened guard finds
// and at or below what ships -- 157 to 160 against S135's 156 and 160, 155 to
// 159 against S083's 154 and 159 -- and flags one outside that band as no
// longer separating. The floor this gate kept until S293 sat 1 below the
// shipping count, as it did again at S083: a single mate the shipping build
// loses to an unrelated tree change did not turn it red, and the weakened guard
// still did with 3 to spare. (At S261 the gap was 4 as well, 160 against 156;
// at S250 it was 3, 146 against 143, and the spare point went above the
// weakened end because that was the end that had moved, 139 to 141 to 143.)
// Property beside it: the `wrong_sign == 0` assertion in the same case -- a
// mate score for the side being mated is a defect at any total, so it is
// asserted at zero and carries no floor.
//
// **159 since S261, re-derived by the script (DEC-142, DEC-277).** On the
// lane's vector S156's sweep reads **160** exact at RfpMinPly 3, 163 at 2 and
// **156** at 1 and 0; the floor goes 1 below the shipping count, 3 above the
// weakened one, and `--floor 159` builds the gate at RfpMinPly 1 and sees it
// red at 156. The family reading that cleared the vector is DEC-277's F3 (the
// S261 step file): 160 against the incumbent's 150. **It also closes a finding
// on achesso's head:** the same sweep at `fd9e44c` and `870d210` read 150
// shipping and 145 weakened, so the floor of 145 no longer separated and the
// gate built at RfpMinPly 1 stayed green there -- found by S261 and closed by
// this re-derivation (DEC-278).
//
// **163 since S286, re-derived by the script (DEC-142).** S099's pawn
// correction, relanded, raises both ends: on S286's tree S156's sweep reads
// **164** exact at RfpMinPly 3, 165 at 2 and **160** at 1 and 0, so the floor
// of 159 no longer separates -- the gate built at RfpMinPly 1 reads 160, green
// at 159, the failure the script exists to catch. The floor rises 159 -> 163,
// 1 below the shipping count and 3 above the weakened one, and the gate built
// at RfpMinPly 1 with 163 compiled in is red at 160
// (`adocs/data/S286_mined_floor.log`). A rise tightens the test, so no
// decision is owed (DEC-295's guard asks that the floor hold or rise); it
// goes back to 159 with a revert of S286. S099 read the same ends on its own
// landing, whose code is this tree's line for line, and its H0 took the 163
// back out.
//
// **163 held at S293, re-read by the script (DEC-142, DEC-299).** Reverse
// futility's table guard moves the shipping end 164 -> 163 and leaves the
// weakened one at 160: the one row it loses, mined row 171 at depth 10, is
// DEC-158's price at another node (`adocs/data/S293_row171.txt`). 163 is
// inside the script's band, 161 to 163, and the gate built at RfpMinPly 1 is
// red at 160 (`adocs/data/S293_goldens.txt`), so it still separates. It now
// sits at the shipping count, not 1 below it: 162 would be a falling floor,
// which DEC-299's guard refuses, so the spare point below the shipping end
// is gone until a change raises that end again. A revert of S293 gives the
// spare point back with the 164.
//
// **158 since S083, by the owner (DEC-313 (1)), re-derived by the script
// (DEC-142).** S083's evaluation moves both ends down: on its tree S156's
// sweep reads **159** exact at RfpMinPly 3, 158 at 2 and **154** at 1 and 0,
// so the shipping build is red at 163 (`adocs/data/S083s_mined_floor.log`).
// A falling floor is outside DEC-299's guard, so it went to the owner, who
// lowered it to 158 on S083's H1 (+116 Elo against four of the 318 mates):
// 1 below the shipping count and 4 above the weakened one, the rule the floor
// was set by until S293. The gate built at RfpMinPly 1 with 158 compiled in
// is red at 154 (`adocs/data/S083s_mined_floor_158.log`). Since S083 this
// floor carries the RfpMinPly 1 separation that tests/test_engine.cpp's
// MATE_IN_THREE_FLOOR no longer does: 11 holds at both of that count's ends,
// 12 and 11 (DEC-313 (2)). A revert of S083 puts 163 back.
//
// **158 held at S135, re-read by the script (DEC-142, DEC-299).** S135's
// piece placement fit raises both ends: on its tree S156's sweep reads
// **160** exact at RfpMinPly 3, 156 at 2 and **156** at 1 and 0, and the gate
// built at RfpMinPly 1 with 158 compiled in is red at 156
// (`adocs/data/S135_goldens.txt`). 158 is inside the band, so it still
// separates and is not moved: the owner set it (DEC-313 (1)), and a floor
// that holds needs no decision. MATE_IN_THREE_FLOOR separates again there,
// 13 shipping against 8 weakened.
//
// **159 since S296, the coordinator's ruling (DEC-319 (5)), re-derived by the
// script (DEC-142).** S296's rule -- a node that skipped a move and found
// every searched move mated returns alpha -- leaves the shipping end at 160
// and raises the weakened one 156 -> 159: on its tree S156's sweep reads
// **160** exact at RfpMinPly 3, 161 at 2 and **159** at 1 and 0
// (`adocs/data/S296_goldens.txt`). The floor rises 158 -> 159, one below the
// shipping count, DEC-313 (1)'s rule; 160, the only value that would still
// separate, sits at the shipping count with no margin, which DEC-116 and
// DEC-313 (2) reject, and S293's at-shipping floor is the one DEC-313 (1)
// moved back. A rise, inside DEC-299's guard. **It no longer separates
// RfpMinPly 1**: the gate built there with 159 compiled in reads 159 and
// stays green on this count. That is DEC-313 (2)'s form with the roles
// swapped -- tests/test_engine.cpp's MATE_IN_THREE_FLOOR carries the
// separation again, 13 shipping against 8 weakened, the same weakened gate
// red there at 8 >= 11. A revert of S296 puts 158 back.
static constexpr int EXACT_FLOOR = 159;


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
