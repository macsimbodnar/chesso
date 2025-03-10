#include <cassert>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <thread>
#include "data_structures.hpp"
#include "functions.hpp"
#include "json.hpp"
#include "move_generator.hpp"
#include "unordered_map"

using json = nlohmann::json;


struct stats_t
{
  uint64_t nodes;
  uint64_t captures;
  uint32_t en_passants;
  uint32_t castles;
  uint32_t promotions;
  uint64_t checks;
  uint64_t discovery_checks;
  uint64_t double_checks;
  uint32_t checkmates;
};


json load_json(const std::string& filename)
{
  std::ifstream file(filename);
  assert(file);

  json json_data;
  try {
    file >> json_data;
  } catch (const json::parse_error& e) {
    assert(false);
  }

  file.close();

  return json_data;
}


uint64_t perft(int depth, const board_t* board)
{
  size_t nodes = 0;

  if (depth == 0) { return 1; }

  const auto& moves = generate_legal_moves(board);

  for (const auto& move : moves) {
    board_t tmp_board = *board;
    make_move(&move, &tmp_board);
    nodes += perft(depth - 1, &tmp_board);
  }

  return nodes;
}


int main()
{
  const json test_cases = load_json("assets/perft_json/perft.json");

  for (const auto& test_case : test_cases) {
    const std::string fen = test_case["start_fen"];
    const bool enabled = test_case["enable"];
    const int depth_limit = test_case["depth_limit"];

    std::cout << "Starting position: " << fen << "\nEnabled " << enabled
              << "\nDepth limit " << depth_limit << "\n"
              << std::endl;

    // If this test is disabled skip it
    if (!enabled) { continue; }

    for (const auto& layer : test_case["depth_layers"]) {
      const int depth = layer["depth"];
      const uint64_t expected_num_of_nodes = layer["nodes"];

      if (depth > depth_limit) { continue; }

      board_t board;
      init_board(fen, &board);
      uint64_t number_of_nodes = 1;

      if (depth > 0) {
        number_of_nodes = 0;
        const auto& moves = generate_legal_moves(&board);

        if (depth > 1) {
          std::vector<std::future<uint64_t>> results;
          for (const auto& move : moves) {
            results.push_back(std::async(std::launch::async, [&]() {
              board_t tmp_board = board;
              move_t tmp_move = move;
              make_move(&tmp_move, &tmp_board);
              return perft(depth - 1, &tmp_board);
            }));
          }

          for (auto& res : results) {
            number_of_nodes += res.get();
          }
        } else {
          number_of_nodes += moves.size();
        }
      }

      bool passed = (expected_num_of_nodes == number_of_nodes);

      const int width = 10;
      std::cout << std::left << std::setw(width) << depth << " | "
                << std::setw(width) << number_of_nodes
                << "  expected: " << std::setw(width) << expected_num_of_nodes
                << "  passed: " << std::setw(width) << passed << std::endl;
    }

    std::cout << "----------------------------------------------" << std::endl;
  }

  return 0;
}
