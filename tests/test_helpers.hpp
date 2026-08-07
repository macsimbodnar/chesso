#pragma once
#include <doctest.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <json.hpp>
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


// generate_moves() is pseudo-legal. This filters it down to the moves that
// actually survive make_move().
inline size_t legal_moves(game_t* game, move_t out[])
{
  move_t pseudo[MAX_MOVES];
  const size_t count = generate_moves(&game->tables, &game->board, pseudo);

  size_t legal = 0;

  for (size_t i = 0; i < count; ++i) {
    if (make_move(game, pseudo[i])) {
      unmake_move(game);
      out[legal++] = pseudo[i];
    }
  }

  return legal;
}


// Plays the legal move that goes from one square to the other. Promotions are
// not addressable this way and are not handled.
inline bool play_move(game_t* game, index_t from, index_t to)
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(&game->tables, &game->board, moves);

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
    castling += static_cast<char>(std::isupper(c) ? std::tolower(c)
                                                  : std::toupper(c));
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
class stdout_capture_t
{
public:
  stdout_capture_t() : old_buffer(std::cout.rdbuf(buffer.rdbuf())) {}

  ~stdout_capture_t() { std::cout.rdbuf(old_buffer); }

  stdout_capture_t(const stdout_capture_t&) = delete;
  stdout_capture_t& operator=(const stdout_capture_t&) = delete;

  std::string str() const { return buffer.str(); }

  bool contains(const std::string& needle) const
  {
    return buffer.str().find(needle) != std::string::npos;
  }

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

private:
  std::stringstream buffer;
  std::streambuf* old_buffer;
};
