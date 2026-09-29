#pragma once
#include <doctest.h>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "utils.hpp"

// Shared fixtures for the doctest binaries. Header only on purpose: every test
// executable already links chesso_engine, and a second library target for a
// hundred lines of glue is not worth the build graph.

// The JSON move suites, relative to the test working directory.
// clang-format off
inline const std::vector<std::string> json_test_files = {
  "assets/test_jsons/castling.json",
  "assets/test_jsons/checkmates.json",
  "assets/test_jsons/famous.json",
  "assets/test_jsons/pawns.json",
  "assets/test_jsons/promotions.json",
  "assets/test_jsons/stalemates.json",
  "assets/test_jsons/standard.json",
  "assets/test_jsons/taxing.json",
};
// clang-format on


inline nlohmann::json load_json(const std::string& filename)
{
  std::ifstream file(filename);
  REQUIRE_MESSAGE(file.good(), ("Cannot open " + filename));

  nlohmann::json data;
  file >> data;

  return data;
}


// Every start position and every resulting position from the JSON suites, as
// a flat deduplicated FEN list. Built once on first use.
inline const std::vector<std::string>& all_test_fens()
{
  static const std::vector<std::string> fens = [] {
    std::vector<std::string> result;

    for (const auto& file : json_test_files) {
      const nlohmann::json cases = load_json(file);

      for (const nlohmann::json& test_case : cases["testCases"]) {
        result.push_back(test_case["start"]["fen"].get<std::string>());

        for (const nlohmann::json& expected : test_case["expected"]) {
          result.push_back(expected["fen"].get<std::string>());
        }
      }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
  }();

  return fens;
}


// A position no legal game can reach. The side **not** to move must not be in
// check: if it were, the previous move left its own king attacked, or was made
// while it already was. Two kings on adjacent squares fail it in both
// directions, and so does a position where the side to move can capture the
// enemy king.
//
// Taken from the engine's own is_check() through a null move rather than from a
// rule restated here, which is the same construction "a null move undoes itself
// exactly" already relies on. CLAUDE.md: chess judgement comes from a tool.
//
// This is a precondition, not a property under test. Cases that use an illegal
// position on purpose - a missing king, a capturable king - do not call it.
// S067, 2026-08-14_test_review-F01 and -F02.
inline bool position_is_reachable(game_t* game)
{
  make_null_move(game);
  const bool other_side_in_check = is_check(game);
  unmake_null_move(game);

  return !other_side_in_check;
}


// generate_moves() is legal-only (INV-1), so there is nothing here to filter:
// every move it produces survives make_move(), and make_move() has exactly one
// way to refuse - the history stack overflow guard at src/bitboard.cpp:754,
// which no test position can reach.
//
// The comment above this said the opposite until S067, and the body read as a
// filter at all of its call sites. It asserts the property instead: a
// regression to pseudo-legal generation fails here rather than being absorbed
// silently everywhere this is called. 2026-08-14_test_review-F04.
//
// Until S193 it only asserted that make_move() accepts the move, which refuses
// nothing but a full history stack, so the claim above was not checked
// anywhere. It reads the mover's king through the engine's own is_check(),
// which is what legality is: a generator that emitted a move leaving its own
// king attacked fails here now. S193, 2026-09-04_test_review-F05.
inline size_t legal_moves(game_t* game, move_t out[])
{
  move_t pseudo[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, pseudo);

  for (size_t i = 0; i < count; ++i) {
    REQUIRE_MESSAGE(
        make_move(game, pseudo[i]),
        ("generate_moves() produced a move make_move() refuses: " +
         print_move(pseudo[i]) + " in " + generate_FEN(&game->board)));
    REQUIRE_MESSAGE(
        position_is_reachable(game),
        ("generate_moves() produced a move that leaves its own king "
         "attacked: " +
         print_move(pseudo[i]) + " in " + generate_FEN(&game->board)));
    unmake_move(game);
    out[i] = pseudo[i];
  }

  return count;
}


// Plays the legal move that goes from one square to the other. Promotions are
// not addressable this way and are not handled.
inline bool play_move(game_t* game, index_t from, index_t to)
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (MOVE_FROM(moves[i]) != from || MOVE_TO(moves[i]) != to) { continue; }
    if (make_move(game, moves[i])) { return true; }
  }

  return false;
}


inline bool move_is_legal(game_t* game, move_t move)
{
  move_t moves[MAX_MOVES];
  const size_t count = legal_moves(game, moves);

  for (size_t i = 0; i < count; ++i) {
    if (moves[i] == move) { return true; }
  }

  return false;
}


// Flips the board vertically and swaps the colour of every piece, castling
// right and the side to move. The mirror of a position is worth exactly the
// negative of the original to any correct evaluation, which makes this the one
// eval assertion that survives every future change to evaluate().
inline std::string mirror_fen(const std::string& fen)
{
  const std::vector<std::string> parts = split_string(fen);
  REQUIRE(parts.size() == 6);

  // Board: reverse the rank order, swap the case of every piece letter.
  std::vector<std::string> ranks;
  std::string current;

  for (const char c : parts[0]) {
    if (c == '/') {
      ranks.push_back(current);
      current.clear();
    } else {
      current += static_cast<char>(
          std::isupper(c) ? std::tolower(c)
                          : (std::islower(c) ? std::toupper(c) : c));
    }
  }
  ranks.push_back(current);

  std::string board;
  for (size_t i = ranks.size(); i > 0; --i) {
    board += ranks[i - 1];
    if (i > 1) { board += '/'; }
  }

  const std::string color = (parts[1] == "w") ? "b" : "w";

  std::string castling;
  for (const char c : parts[2]) {
    castling +=
        static_cast<char>(std::isupper(c) ? std::tolower(c) : std::toupper(c));
  }
  if (parts[2] == "-") { castling = "-"; }

  // The en-passant square mirrors across the middle of the board: rank 3
  // becomes rank 6 and the other way round. The file is unchanged.
  std::string en_passant = parts[3];
  if (en_passant != "-") {
    en_passant[1] = static_cast<char>('0' + (9 - (en_passant[1] - '0')));
  }

  return board + " " + color + " " + castling + " " + en_passant + " " +
         parts[4] + " " + parts[5];
}


// RAII capture of everything written to std::cout, so the UCI replies can be
// asserted on without a subprocess.
//
// doctest's console reporter writes to std::cout as well, so what it reports
// while a capture is alive goes into the capture's buffer and leaves the run
// with it. capture_report_listener_t below repeats it, and reads from here
// whether a capture is alive. S242.
class stdout_capture_t
{
public:
  stdout_capture_t() : old_buffer(std::cout.rdbuf(buffer.rdbuf())) { ++alive; }

  ~stdout_capture_t()
  {
    std::cout.rdbuf(old_buffer);
    --alive;
  }

  stdout_capture_t(const stdout_capture_t&) = delete;
  stdout_capture_t& operator=(const stdout_capture_t&) = delete;

  std::string str() const { return buffer.str(); }

  bool contains(const std::string& needle) const
  { return buffer.str().find(needle) != std::string::npos; }

  std::vector<std::string> lines() const
  {
    std::vector<std::string> result;
    std::istringstream stream(buffer.str());
    std::string line;

    while (std::getline(stream, line)) {
      if (!line.empty()) { result.push_back(line); }
    }

    return result;
  }

  // Atomic because doctest lets any thread assert, and the listener asks from
  // the thread that asserted.
  static bool any_alive() { return alive.load() > 0; }

private:
  inline static std::atomic<int> alive{0};

  std::stringstream buffer;
  std::streambuf* old_buffer;
};


// A failure logged inside a stdout_capture_t scope used to vanish: the console
// reporter printed it into the capture's buffer, the capture was destroyed, and
// the run ended on "1 failed" with no word of which assertion. S131's mutation
// baseline met exactly that twice in test_engine beside a match on every core,
// and S242 reproduced it once in 500 runs of the one case it came from.
//
// This listener repeats on std::cerr, which no capture swaps, what the console
// prints while a capture is alive, and prints nothing otherwise, so no report
// appears twice. Registered from this header rather than from a main() of each
// binary's own, so a binary has it by including the header that gives it the
// capture. The wording follows the console's closely enough that `ERROR:`
// finds both, not byte for byte. It uses the reporter interface doctest
// documents (tests/doctest/doc/markdown/reporters.md) and nothing internal.
class capture_report_listener_t : public doctest::IReporter
{
public:
  explicit capture_report_listener_t(const doctest::ContextOptions& in)
      : options(in)
  {}

  void report_query(const doctest::QueryData&) override {}
  void test_run_start() override {}
  void test_run_end(const doctest::TestRunStats&) override {}
  void test_case_end(const doctest::CurrentTestCaseStats&) override {}
  void test_case_skipped(const doctest::TestCaseData&) override {}

  void test_case_start(const doctest::TestCaseData& in) override
  {
    const std::lock_guard<std::mutex> lock(mutex);
    test_case = &in;
    subcases.clear();
    header_written = false;
  }

  void test_case_reenter(const doctest::TestCaseData&) override
  {
    const std::lock_guard<std::mutex> lock(mutex);
    subcases.clear();
    header_written = false;
  }

  void subcase_start(const doctest::SubcaseSignature& in) override
  {
    const std::lock_guard<std::mutex> lock(mutex);
    subcases.push_back(in.m_name.c_str());
    header_written = false;
  }

  void subcase_end() override
  {
    const std::lock_guard<std::mutex> lock(mutex);
    if (!subcases.empty()) { subcases.pop_back(); }
    header_written = false;
  }

  // An exception unwinds the test body, and every capture in it, before
  // doctest reports it, so what arrives here with a capture alive is a crash:
  // a signal, and the capture's buffer is never printed.
  void test_case_exception(const doctest::TestCaseException& in) override
  {
    if (!swallowed()) { return; }

    std::ostringstream text;
    text << location(test_case->m_file.c_str(),
                     static_cast<int>(test_case->m_line))
         << " ERROR: test case " << (in.is_crash ? "CRASHED: " : "THREW: ")
         << in.error_string << "\n\n";
    write(text.str());
  }

  void log_assert(const doctest::AssertData& in) override
  {
    // Every assertion reaches every reporter; the console prints a passing one
    // only under --success, and so does this.
    if (!in.m_failed && !options.success) { return; }
    if (!swallowed()) { return; }

    namespace at = doctest::assertType;
    const bool about_throwing =
        (in.m_at & (at::is_throws | at::is_throws_as | at::is_throws_with |
                    at::is_nothrow)) != 0;

    std::ostringstream text;
    text << location(in.m_file, in.m_line) << ' '
         << (in.m_failed ? doctest::failureString(in.m_at) : "SUCCESS") << ": "
         << doctest::assertString(in.m_at) << "( " << in.m_expr << " ) ";

    if (about_throwing) {
      text << (in.m_threw ? "threw " : "did NOT throw") << in.m_exception
           << '\n';
    } else if (in.m_threw) {
      text << "THREW exception: " << in.m_exception << '\n';
    } else {
      text << (in.m_failed ? "is NOT correct!" : "is correct!") << '\n';

      if (in.m_decomp.size() > 0) {
        text << "  values: " << doctest::assertString(in.m_at) << "( "
             << in.m_decomp << " )\n";
      }
    }

    write_with_contexts(text);
  }

  // MESSAGE, WARN, FAIL_CHECK and FAIL all arrive here, and the console prints
  // every one of them.
  void log_message(const doctest::MessageData& in) override
  {
    if (!swallowed()) { return; }

    const bool is_warn = (in.m_severity & doctest::assertType::is_warn) != 0;

    std::ostringstream text;
    text << location(in.m_file, in.m_line) << ' '
         << (is_warn ? "MESSAGE" : doctest::failureString(in.m_severity))
         << ": " << in.m_string << '\n';

    write_with_contexts(text);
  }

private:
  // The console writes to *options.cout, which is std::cout unless --out names
  // a file or --quiet discards it, and std::cout is the only stream a capture
  // swaps.
  bool swallowed() const
  {
    return options.cout == &std::cout && stdout_capture_t::any_alive() &&
           test_case != nullptr && !test_case->m_no_output;
  }

  std::string location(const char* file, int line) const
  {
    std::ostringstream text;
    text << doctest::skipPathFromFilename(file)
         << (options.gnu_file_line ? ":" : "(")
         << (options.no_line_numbers ? 0 : line)
         << (options.gnu_file_line ? ":" : "):");
    return text.str();
  }

  // The INFO and CAPTURE scopes active at the assertion, which is also where
  // a CHECK_MESSAGE keeps its message.
  void write_with_contexts(std::ostringstream& text)
  {
    const int count = get_num_active_contexts();
    const doctest::IContextScope* const* contexts = get_active_contexts();

    for (int i = 0; i < count; ++i) {
      text << (i == 0 ? "  logged: " : "          ");
      contexts[i]->stringify(&text);
      text << '\n';
    }

    text << '\n';
    write(text.str());
  }

  void write(const std::string& report)
  {
    const std::lock_guard<std::mutex> lock(mutex);

    // std::cout's own buffer is the capture's now, so what the run printed to
    // stdout before the capture is still in C's stdout buffer; flushed first,
    // it stays ahead of this in a log that takes both streams.
    std::fflush(stdout);

    if (!header_written) {
      std::cerr << std::string(79, '=') << '\n'
                << "doctest reported this inside a stdout_capture_t scope, "
                   "which kept it off stdout\n"
                << location(test_case->m_file.c_str(),
                            static_cast<int>(test_case->m_line))
                << '\n';

      if (test_case->m_test_suite != nullptr &&
          test_case->m_test_suite[0] != '\0') {
        std::cerr << "TEST SUITE: " << test_case->m_test_suite << '\n';
      }

      std::cerr << "TEST CASE:  " << test_case->m_name << '\n';

      for (const std::string& subcase : subcases) {
        std::cerr << "  " << subcase << '\n';
      }

      std::cerr << '\n';
      header_written = true;
    }

    std::cerr << report << std::flush;
  }

  const doctest::ContextOptions& options;
  const doctest::TestCaseData* test_case = nullptr;
  std::vector<std::string> subcases;
  bool header_written = false;
  std::mutex mutex;
};

DOCTEST_REGISTER_LISTENER("stdout_capture_report",
                          1,
                          capture_report_listener_t);
