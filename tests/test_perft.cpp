#include <atomic>
#include <cassert>
#include <chrono>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <optional>
#include <ostream>
#include <thread>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "json.hpp"
#include "unordered_map"


using json = nlohmann::json;

// #define RUN_THREADS
#define MAXIMUM_DEPTH 20


// clang-format off
const static std::vector<std::string> test_files = {
  // "assets/perft_json/debug_perft.json",
  "assets/perft_json/talkchess_perft.json",
  "assets/perft_json/perft.json"
};
// clang-format on

static game_t game;


// ANSI escape codes for colors
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"

struct expected_stats_t
{
  std::optional<uint64_t> nodes;
  std::optional<uint64_t> captures;
  std::optional<uint32_t> en_passants;
  std::optional<uint32_t> castles;
  std::optional<uint32_t> promotions;
  std::optional<uint64_t> checks;
  std::optional<uint64_t> discovery_checks;
  std::optional<uint64_t> double_checks;
  std::optional<uint32_t> checkmates;
};

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

  stats_t()
      : nodes(0),
        captures(0),
        en_passants(0),
        castles(0),
        promotions(0),
        checks(0),
        discovery_checks(0),
        double_checks(0),
        checkmates(0)
  {}


  stats_t& operator+=(const stats_t& other)
  {
    this->nodes += other.nodes;
    this->captures += other.captures;
    this->en_passants += other.en_passants;
    this->castles += other.castles;
    this->promotions += other.promotions;
    this->checks += other.checks;
    this->discovery_checks += other.discovery_checks;
    this->double_checks += other.double_checks;
    this->checkmates += other.checkmates;
    return *this;
  }
};


class spin_lock
{
  std::atomic_flag locked;

public:
  void lock()
  {
    while (locked.test_and_set(std::memory_order_acquire)) {
      std::this_thread::yield();
    }
  }
  void unlock() { locked.clear(std::memory_order_release); }
};


/**
 *  Transposition table element
 *
 * I use the Transposition Table in perft to check if the hash key has no bugs
 */
struct tt_elem_t
{
  uint64_t key = 0;
  stats_t stats;
  int depth;
};

static spin_lock tt_spin_lock;
static tt_elem_t tt[TT_SIZE] = {};


inline void cleanup_tt()
{
  // std::cout << "Cleanup TT\n";
  memset(&tt, 0, sizeof(tt));
}


inline const stats_t* get_from_tt(const board_t* board, int depth)
{
  const stats_t* res = nullptr;

  const tt_elem_t* entry = &tt[board->zobrist_key % TT_SIZE];

  tt_spin_lock.lock();
  if (entry->key == board->zobrist_key && entry->depth == depth) {
    res = &entry->stats;
  }
  tt_spin_lock.unlock();

  return res;
}


inline void store_to_tt(const board_t* board, int depth, const stats_t* stats)
{
  tt_elem_t* elem = &tt[board->zobrist_key % TT_SIZE];

  tt_spin_lock.lock();
  elem->depth = depth;
  elem->stats = *stats;
  elem->key = board->zobrist_key;
  tt_spin_lock.unlock();
}


std::string print_stats_headline()
{
  const int width = 20;
  std::stringstream ss;

  // clang-format off
  ss << std::left
     << std::setw(30) << "|nodes"
     << std::setw(30) << "|captures"
     << std::setw(width) << "|en_passant"
     << std::setw(width) << "|castles"
     << std::setw(width) << "|promotions";
    //  << std::setw(width) << "|checks"
    //  << std::setw(width) << "|discovery_checks"
    //  << std::setw(width) << "|double_checks"
    //  << std::setw(width) << "|checkmates";
  // clang-format on
  return ss.str();
}

std::string print_stats_headline_second_line()
{
  const int width = 20;
  std::stringstream ss;

  // clang-format off
  ss << std::left
     << std::setw(30) << "|expected      real"
     << std::setw(30) << "|expected      real"
     << std::setw(width) << "|expected real"
     << std::setw(width) << "|expected real"
     << std::setw(width) << "|expected real";
    //  << std::setw(width) << "|expected real"
    //  << std::setw(width) << "|expected real"
    //  << std::setw(width) << "|expected real"
    //  << std::setw(width) << "|expected real";
  // clang-format on
  return ss.str();
}


inline std::ostream& operator<<(std::ostream& os, const stats_t& pos)
{
  const int width = 30;

  // clang-format off
  os << std::left
     << std::setw(width) << pos.nodes
     << std::setw(width) << pos.captures
     << std::setw(width) << pos.en_passants
     << std::setw(width) << pos.castles
     << std::setw(width) << pos.promotions
     << std::setw(width) << pos.checks
     << std::setw(width) << pos.checkmates;
  // clang-format on
  return os;
}


expected_stats_t load_expected_stats(const json& stats_dict)
{
  expected_stats_t result;

  // clang-format off
  result.nodes = stats_dict["nodes"];
  
  if (!stats_dict["captures"].is_null()) {
    result.captures = stats_dict["captures"].get<uint64_t>();
  }
  if (!stats_dict["en_passant"].is_null()) {
    result.en_passants = stats_dict["en_passant"].get<uint64_t>();
  }
  if (!stats_dict["castles"].is_null()) {
    result.castles = stats_dict["castles"].get<uint64_t>();
  }
  if (!stats_dict["promotions"].is_null()) {
    result.promotions = stats_dict["promotions"].get<uint64_t>();
  }
  if (!stats_dict["checks"].is_null()) {
    result.checks = stats_dict["checks"].get<uint64_t>();
  }
  if (!stats_dict["checkmates"].is_null()) {
    result.checkmates = stats_dict["checkmates"].get<uint64_t>();
  }
  // clang-format on

  return result;
}


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


stats_t get_move_stats(const move_t& move)
{
  stats_t stats;
  stats.nodes += 1;
  stats.captures += (MOVE_CAPTURE(move) ? 1 : 0);
  stats.en_passants += (MOVE_EN_PASSANT(move) ? 1 : 0);
  stats.castles += (MOVE_CASTLING(move) ? 1 : 0);
  stats.promotions += (MOVE_PROMOTED(move) > 0 ? 1 : 0);
  stats.checks = 0;
  stats.discovery_checks = 0;
  stats.double_checks = 0;
  stats.checkmates = 0;

  return stats;
}


stats_t get_moves_stats(const move_t moves[], size_t moves_count)
{
  stats_t result;
  for (size_t i = 0; i < moves_count; ++i) {
    result += get_move_stats(moves[i]);
  }

  return result;
}


stats_t perft(int depth, game_t* game)
{
  assert(game != nullptr);

  stats_t node_stats;

  if (depth == 0) {
    node_stats.nodes = 1;
    return node_stats;
  }

  // Check if this position is in tt table
  // const stats_t* stats_in_tt = get_from_tt(board, depth);
  // if (stats_in_tt != nullptr) { return *stats_in_tt; }

  move_t moves[270];
  const size_t moves_count = generate_moves(&game->tables, &game->board, moves);

  for (size_t i = 0; i < moves_count; ++i) {
    if (make_move(game, moves[i])) {
      if (depth == 1) {
        node_stats += get_move_stats(moves[i]);
      } else {
        node_stats.nodes += 1;
      }

      node_stats.nodes -= 1;
      node_stats += perft(depth - 1, game);

      unmake_move(game);
    }
  }

  // store_to_tt(board, depth, &node_stats);
  return node_stats;
}


std::string print_stats(const expected_stats_t& expected, const stats_t real)
{
  std::stringstream ss;
  const int width = 10;
  // clang-format off
  ss << std::left;

  if (expected.nodes.has_value()) { 
    ss << ((expected.nodes.value() == real.nodes) ? GREEN : RED) << std::setw(15) << std::string("|").append(std::to_string(expected.nodes.value())) << std::setw(15) << real.nodes << RESET;
  } else {
    ss << std::setw(15) << "| - " << std::setw(15) << real.nodes;
  }

  if (expected.captures.has_value()) { 
    ss << ((expected.captures.value() == real.captures) ? GREEN : RED) << std::setw(15) << std::string("|").append(std::to_string(expected.captures.value())) << std::setw(15) << real.captures << RESET;
  } else {
    ss << std::setw(15) << "| - " << std::setw(15) << real.captures;
  }

  if (expected.en_passants.has_value()) { 
    ss << ((expected.en_passants.value() == real.en_passants) ? GREEN : RED) << std::setw(width) << std::string("|").append(std::to_string(expected.en_passants.value())) << std::setw(width) << real.en_passants << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.en_passants;
  }

  if (expected.castles.has_value()) { 
    ss << ((expected.castles.value() == real.castles) ? GREEN : RED) << std::setw(width) << std::string("|").append(std::to_string(expected.castles.value())) << std::setw(width) << real.castles << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.castles;
  }

  if (expected.promotions.has_value()) { 
    ss << ((expected.promotions.value() == real.promotions) ? GREEN : RED) << std::setw(width) << std::string("|").append(std::to_string(expected.promotions.value())) << std::setw(width) << real.promotions << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.promotions;
  }

    //  << ((expected.checks == real.checks) ? GREEN : RED) << std::setw(width) << "|" + std::to_string(expected.checks) << std::setw(width) << real.checks << RESET
    //  << ((expected.discovery_checks == real.discovery_checks) ? GREEN : RED) << std::setw(width) << "|" + std::to_string(expected.discovery_checks) << std::setw(width) << real.discovery_checks << RESET
    //  << ((expected.double_checks == real.double_checks) ? GREEN : RED) << std::setw(width) << "|" + std::to_string(expected.double_checks) << std::setw(width) << real.double_checks << RESET
    //  << ((expected.checkmates == real.checkmates) ? GREEN : RED) << std::setw(width) << "|" + std::to_string(expected.checkmates) << std::setw(width) << real.checkmates << RESET;
  // clang-format on
  return ss.str();
}


int main()
{
  initialize_const_data(&game.tables);

  bool passed = true;

  for (const auto& test_file : test_files) {
    const json test_cases = load_json(test_file);

    for (const auto& test_case : test_cases) {
      const std::string fen = test_case["start_fen"];
      const bool enabled = test_case["enable"];
      const int depth_limit_candidate = test_case["depth_limit"];
      const std::string comments = test_case["comments"];

      const int depth_limit = (depth_limit_candidate > MAXIMUM_DEPTH)
                                  ? MAXIMUM_DEPTH
                                  : depth_limit_candidate;

      std::cout << YELLOW << "Starting position: " << fen << RESET
                << "\nEnabled " << enabled << "\nDepth limit " << depth_limit
                << "\nDepth hard limit " << MAXIMUM_DEPTH
                << "\nComments: " << comments << "\n"
                << std::endl;

      // If this test is disabled skip it
      if (!enabled) { continue; }

      const int width = 10;

      // Reset the Transposition Table for new position
      cleanup_tt();

      // clang-format off
      std::cout << std::left
                << std::setw(width - 2) << "|depth"
                << std::setw(width / 2) << "|min"
                << std::setw(width / 2) << "sec"
                << std::setw(width / 2) << "msec"
                << print_stats_headline() << std::setw(24) << "\n"
                << print_stats_headline_second_line()
                << std::endl;
      // clang-format on


      for (const auto& layer : test_case["depth_layers"]) {
        const int depth = layer["depth"];
        const expected_stats_t expected_stats = load_expected_stats(layer);

        if (depth > depth_limit) { continue; }

        auto start_time = std::chrono::high_resolution_clock::now();
        load_FEN(fen, &game.board, &game.history);
        stats_t stats;
        stats.nodes = 1;

        if (depth > 0) {
          stats = stats_t();

          move_t moves[270];
          const size_t moves_count =
              generate_moves(&game.tables, &game.board, moves);

          if (depth > 1) {
#ifdef RUN_THREADS
            std::vector<std::future<stats_t>> results;

            for (size_t i = 0; i < moves_count; ++i) {
              results.push_back(std::async(std::launch::async, [moves, i,
                                                                depth]() {
                const move_t tmp_move = moves[i];
                board_t tmp_board = g_board;

                global_state_t* globals = new global_state_t;
                *globals = g_globals;

                make_move(&tmp_move, &tmp_board, globals);
                const stats_t result = perft(depth - 1, &tmp_board, globals);
                unmake_move(&tmp_board, globals);

                delete globals;

                return result;
              }));
            }

            for (auto& res : results) {
              stats += res.get();
            }
#else
            for (size_t i = 0; i < moves_count; ++i) {
              if (make_move(&game, moves[i])) {
                const auto res = perft(depth - 1, &game);
                unmake_move(&game);
                stats += res;
              }
            }
#endif

          } else {
            stats += get_moves_stats(moves, moves_count);
          }
        }

        const auto end_time = std::chrono::high_resolution_clock::now();

        const int64_t difference = expected_stats.nodes.value() - stats.nodes;

        if (difference != 0) { passed = false; }

        const auto duration_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                                 start_time);

        // Convert to minutes, seconds, and milliseconds
        const auto minutes =
            std::chrono::duration_cast<std::chrono::minutes>(duration_ns);
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            duration_ns - minutes);
        const auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                duration_ns - minutes - seconds);


        std::cout << std::fixed << std::setprecision(10);
        // clang-format off
      std::cout << std::left
                << std::setw(width - 2) << std::string("|").append(std::to_string(depth))
                << std::setw(width / 2) << std::string("|").append(std::to_string(minutes.count()))
                << std::setw(width / 2) << seconds.count()
                << std::setw(width / 2) << milliseconds.count()
                << print_stats(expected_stats, stats)
                << std::endl;

        // clang-format on
      }

      std::cout << "----------------------------------------------"
                << std::endl;
    }
  }

  return (passed ? 0 : 1);
}
