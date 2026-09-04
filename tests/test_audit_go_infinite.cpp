#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include "test_helpers.hpp"
#include "uci.hpp"

// 2026-09-04_adversarial-F01. Written by the audit before any fix and observed
// red at 36b9f36. Not registered in tests/CMakeLists.txt; a fix registers it
// and observes it green.
//
// UCI: "infinite -- search until the stop command. Do not exit the search
// without being told so in this mode!" iterative_deepening_search() runs
// `for (current_depth = 1; current_depth <= conf.depth; ...)` with conf.depth
// = MAX_DEPTH (126) and nothing else holds a `go infinite` open, so a root
// whose tree collapses -- a dead draw by insufficient material, a stalemate or
// a checkmate -- reaches depth 126 in under a millisecond and `bestmove` is
// printed before any `stop` arrives. tests/test_engine.cpp's "an infinite
// search answers only once stop arrives" holds the start position, where depth
// 126 is unreachable, so it cannot see this.
//
// Three roots, each of which the search finishes instantly: king and knight
// against king (is_insufficient_material() draws every node above the root),
// a stalemated root and a checkmated root. In each the assertion is that no
// `bestmove` has been printed 200 ms after `go infinite`, and that one is
// printed once `stop` is sent -- the second half being the precondition that
// the search is alive and answering at all.

static std::string bestmove_line(const std::vector<std::string>& lines)
{
  for (const std::string& line : lines) {
    if (line.rfind("bestmove", 0) == 0) { return line; }
  }

  return "";
}


TEST_SUITE("audit: go infinite")
{
  TEST_CASE("go infinite does not answer before stop, whatever the position")
  {
    const std::vector<std::string> roots = {
        // King and knight against king: insufficient material, every node
        // above the root scores a draw and the tree is one ply wide.
        "4k3/8/8/8/8/8/8/4KN2 w - - 0 1",
        // Stalemate, Black to move: no legal move at all.
        "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",
        // Checkmate, Black to move.
        "7k/6Q1/6K1/8/8/8/8/8 b - - 0 1",
    };

    for (const std::string& fen : roots) {
      uci_init();

      std::string before_stop;
      std::string after_stop;

      {
        stdout_capture_t capture;
        uci_process_line("position fen " + fen);
        uci_process_line("go infinite");

        // Long enough for a collapsed tree to run to MAX_DEPTH many times
        // over: the three roots above do it in well under a millisecond.
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        before_stop = bestmove_line(capture.lines());

        uci_process_line("stop");
        uci_wait_for_search();
        after_stop = bestmove_line(capture.lines());
      }

      // The precondition: the search does answer once told to stop.
      REQUIRE_MESSAGE(!after_stop.empty(),
                      ("no bestmove after stop for " + fen));

      // The claim: nothing was answered before it.
      CHECK_MESSAGE(before_stop.empty(),
                    ("go infinite answered [" + before_stop +
                     "] before stop for " + fen));

      uci_shutdown();
    }
  }
}
