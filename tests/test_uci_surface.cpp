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

// The golden surface test required by AGENTS.md section 7 and DEC-024. Chesso's
// whole product surface is the UCI protocol, so `surface_guard` is `cli` and
// this file is what holds MANUAL.md to account: adding, renaming or removing a
// command or an option fails here until MANUAL.md is updated in the same
// commit.
//
// The commands and the option lines are read out of the running engine rather
// than restated from the source. uci_command_names() is the dispatch table that
// uci_process_line() dispatches through, and the option lines are the reply a
// GUI actually reads, so a surface addition cannot pass unnoticed.
//
// The [go] and [position] argument tokens are the exception. They are an
// if-else chain inside their handlers with nothing to enumerate at runtime, so
// the lists here are maintained by hand: a rename or a removal fails, a
// brand-new token does not. DEC-028 records that limit and why it was taken
// instead of rewriting the two parsers into dispatch tables.

#ifndef CHESSO_SOURCE_DIR
#error "CHESSO_SOURCE_DIR must be defined so the test can read MANUAL.md"
#endif


// clang-format off
static const std::vector<std::string> expected_commands = {
  // standard UCI
  "uci", "debug", "isready", "setoption", "register", "ucinewgame",
  "position", "go", "stop", "ponderhit", "quit",
  // non-standard convenience commands
  "pb", "fen", "help", "test", "clean-tt",
};

// Verbatim, including every default and every range. A changed Hash range is a
// changed surface and has to reach MANUAL.md like anything else.
static const std::vector<std::string> expected_option_lines = {
  "option name Use Book type check default false",
  "option name Hash type spin default 16 min 1 max 4096",
  "option name Threads type spin default 1 min 1 max 1",
};

static const std::vector<std::string> expected_option_names = {
  "Use Book", "Hash", "Threads",
};

static const std::vector<std::string> expected_go_tokens = {
  "depth", "movetime", "nodes", "wtime", "btime", "winc", "binc", "movestogo",
  "infinite",
  // accepted and ignored, which is still surface a GUI can send
  "mate", "searchmoves", "ponder",
};

static const std::vector<std::string> expected_position_tokens = {
  "startpos", "fen", "moves",
  // non-standard shortcuts
  "empty", "mate2w", "mate2b", "3frep", "tricky", "killer", "cmk", "fine70",
};
// clang-format on


static std::vector<std::string> sorted(std::vector<std::string> values)
{
  std::sort(values.begin(), values.end());
  return values;
}


// Reports both directions of a mismatch, so a failure names the command that
// was added and the one that went away instead of only saying they differ.
static std::string set_difference_report(const std::vector<std::string>& actual,
                                         const std::vector<std::string>& golden)
{
  const std::vector<std::string> a = sorted(actual);
  const std::vector<std::string> g = sorted(golden);

  std::vector<std::string> unexpected;
  std::vector<std::string> missing;

  std::set_difference(a.begin(), a.end(), g.begin(), g.end(),
                      std::back_inserter(unexpected));
  std::set_difference(g.begin(), g.end(), a.begin(), a.end(),
                      std::back_inserter(missing));

  if (unexpected.empty() && missing.empty()) { return ""; }

  std::string report = "UCI surface changed.";

  if (!unexpected.empty()) {
    report += " Present but not in the golden list:";
    for (const std::string& value : unexpected) {
      report += " [" + value + "]";
    }
  }

  if (!missing.empty()) {
    report += " In the golden list but gone from the engine:";
    for (const std::string& value : missing) {
      report += " [" + value + "]";
    }
  }

  report += ". Update MANUAL.md and this test together.";

  return report;
}


static std::string read_manual()
{
  const std::string path = std::string(CHESSO_SOURCE_DIR) + "/MANUAL.md";
  std::ifstream file(path);
  REQUIRE_MESSAGE(file.good(), ("Cannot open " + path));

  std::stringstream contents;
  contents << file.rdbuf();

  return contents.str();
}


// MANUAL.md writes every name of the surface in backticks. The [go] arguments
// that are accepted and ignored are written with their command, so both
// spellings count as documented.
static bool manual_documents(const std::string& manual,
                             const std::string& command,
                             const std::string& name)
{
  if (manual.find("`" + name + "`") != std::string::npos) { return true; }
  if (command.empty()) { return false; }

  return manual.find("`" + command + " " + name + "`") != std::string::npos;
}


// Everything [help] prints between its two banner lines.
static std::vector<std::string> commands_from_help()
{
  stdout_capture_t capture;
  uci_process_line("help");

  std::vector<std::string> names;
  bool inside = false;

  for (const std::string& line : capture.lines()) {
    if (line == "--- Help: available commands ---") {
      inside = true;
      continue;
    }

    if (line == "--------------------------------") {
      inside = false;
      continue;
    }

    if (inside) { names.push_back(line); }
  }

  return names;
}


static std::vector<std::string> option_lines_from_uci()
{
  stdout_capture_t capture;
  uci_process_line("uci");

  std::vector<std::string> options;

  for (const std::string& line : capture.lines()) {
    if (line.rfind("option ", 0) == 0) { options.push_back(line); }
  }

  return options;
}


// The FEN [fen] prints for the position the engine is holding.
static std::string current_fen()
{
  stdout_capture_t capture;
  uci_process_line("fen");

  const std::vector<std::string> lines = capture.lines();
  REQUIRE(lines.size() == 1);

  return lines.back();
}


TEST_SUITE("uci surface")
{
  TEST_CASE("the command set is exactly the documented one")
  {
    uci_init();

    const std::vector<std::string> actual = uci_command_names();

    // An empty enumeration would satisfy the comparison below by accident. The
    // count is deliberately not asserted here: a mismatch has to reach the
    // report, which names what moved, instead of stopping at a number.
    REQUIRE(!actual.empty());

    const std::string report = set_difference_report(actual, expected_commands);
    CHECK_MESSAGE(report.empty(), report);

    uci_shutdown();
  }


  TEST_CASE("help prints the dispatch table and nothing else")
  {
    uci_init();

    const std::vector<std::string> listed = commands_from_help();

    // [help] is user-facing surface in its own right. If it ever stops
    // reporting the table it walks, the list above stops being evidence of
    // anything.
    REQUIRE(!listed.empty());
    CHECK(listed == uci_command_names());

    uci_shutdown();
  }


  TEST_CASE("an unknown command is answered with silence")
  {
    uci_init();

    // The precondition for the assertion below: a name that is a command does
    // reach stdout, so an empty capture means the input was refused rather
    // than that nothing ever prints here.
    {
      stdout_capture_t capture;
      uci_process_line("isready");
      REQUIRE(!capture.str().empty());
    }

    {
      stdout_capture_t capture;
      uci_process_line("nosuchcommand");
      CHECK(capture.str().empty());
    }

    uci_shutdown();
  }


  TEST_CASE("the option declarations are exactly the documented ones")
  {
    uci_init();

    const std::vector<std::string> actual = option_lines_from_uci();

    REQUIRE(!actual.empty());

    const std::string report =
        set_difference_report(actual, expected_option_lines);
    CHECK_MESSAGE(report.empty(), report);

    uci_shutdown();
  }


  TEST_CASE("the uci reply carries the identification a GUI needs")
  {
    uci_init();

    stdout_capture_t capture;
    uci_process_line("uci");

    CHECK(capture.contains("id name Chesso"));
    CHECK(capture.contains("id author MazerFaker"));
    CHECK(capture.contains("uciok"));

    uci_shutdown();
  }


  TEST_CASE("MANUAL.md documents every command and every option")
  {
    const std::string manual = read_manual();

    // A MANUAL.md that failed to load would document nothing, and every name
    // below would be reported missing for the wrong reason.
    REQUIRE(manual.find("# Chesso") != std::string::npos);

    for (const std::string& name : expected_commands) {
      CHECK_MESSAGE(manual_documents(manual, "", name),
                    ("MANUAL.md does not document the command [" + name + "]"));
    }

    for (const std::string& name : expected_option_names) {
      CHECK_MESSAGE(manual_documents(manual, "", name),
                    ("MANUAL.md does not document the option [" + name + "]"));
    }
  }


  TEST_CASE("MANUAL.md documents every go and position argument")
  {
    const std::string manual = read_manual();

    REQUIRE(manual.find("# Chesso") != std::string::npos);

    for (const std::string& name : expected_go_tokens) {
      CHECK_MESSAGE(manual_documents(manual, "go", name),
                    ("MANUAL.md does not document [go " + name + "]"));
    }

    for (const std::string& name : expected_position_tokens) {
      CHECK_MESSAGE(manual_documents(manual, "position", name),
                    ("MANUAL.md does not document [position " + name + "]"));
    }
  }


  TEST_CASE("the ignored go arguments leave the rest of the line working")
  {
    uci_init();

    // [go depth] and the time controls are covered end to end in test_engine.
    // What only exists as surface is the promise that mate, searchmoves and
    // ponder are swallowed rather than rejected, and that the limit sitting
    // behind them is still read.
    stdout_capture_t capture;
    uci_process_line("position startpos");
    uci_process_line("go mate 2 searchmoves e2e4 ponder depth 1");
    uci_wait_for_search();

    CHECK(capture.contains("bestmove"));

    uci_shutdown();
  }


  TEST_CASE("every position argument still reaches the board")
  {
    uci_init();

    const std::string start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    {
      uci_process_line("position startpos");
      REQUIRE(current_fen() == start_fen);
    }

    // Each shortcut has to land on a position of its own. A token that stopped
    // being parsed would leave the board where the preceding command put it,
    // and two shortcuts would then report the same FEN.
    std::vector<std::string> reached;

    for (const std::string& token : expected_position_tokens) {
      if (token == "fen" || token == "moves") { continue; }

      uci_process_line("position startpos");
      uci_process_line("position " + token);

      const std::string fen = current_fen();

      if (token != "startpos") {
        CHECK_MESSAGE(fen != start_fen,
                      ("[position " + token + "] left the start position"));
      }

      reached.push_back(fen);
    }

    std::vector<std::string> distinct = sorted(reached);
    distinct.erase(std::unique(distinct.begin(), distinct.end()),
                   distinct.end());

    CHECK(distinct.size() == reached.size());

    {
      uci_process_line("position empty");
      uci_process_line("position fen " + start_fen);

      CHECK(current_fen() == start_fen);
    }

    {
      uci_process_line("position startpos moves e2e4");

      CHECK(current_fen() != start_fen);
    }

    uci_shutdown();
  }
}
