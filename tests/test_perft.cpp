#include <cassert>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <ostream>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "json.hpp"
#include "unordered_map"


using json = nlohmann::json;

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

static tt_elem_t tt[TT_SIZE] = {};


inline void cleanup_tt()
{
  // std::cout << "Cleanup TT\n";
  memset(&tt, 0, sizeof(tt));
}


inline const stats_t* get_from_tt(const board_t* board, int depth)
{
  const stats_t* res = nullptr;

  const tt_elem_t* entry = &tt[board->hash % TT_SIZE];

  if (entry->key == board->hash && entry->depth == depth) {
    res = &entry->stats;
  }

  return res;
}


inline void store_to_tt(const board_t* board, int depth, const stats_t* stats)
{
  tt_elem_t* elem = &tt[board->hash % TT_SIZE];

  elem->depth = depth;
  elem->stats = *stats;
  elem->key = board->hash;
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
     << std::setw(width) << "|promotions"
     << std::setw(30) << "|checks"
     << std::setw(width) << "|discovery"
     << std::setw(width) << "|double"
     << std::setw(width) << "|checkmates";
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
     << std::setw(width) << "|expected real"
     << std::setw(30) << "|expected      real"
     << std::setw(width) << "|expected real"
     << std::setw(width) << "|expected real"
     << std::setw(width) << "|expected real";
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
  if (!stats_dict["discovery_checks"].is_null()) {
    result.discovery_checks = stats_dict["discovery_checks"].get<uint64_t>();
  }
  if (!stats_dict["double_checks"].is_null()) {
    result.double_checks = stats_dict["double_checks"].get<uint64_t>();
  }
  if (!stats_dict["checkmates"].is_null()) {
    result.checkmates = stats_dict["checkmates"].get<uint64_t>();
  }
  // clang-format on

  return result;
}


// Both refusals used to be `assert`, which the Release build compiles out: a
// missing or unparseable asset iterated zero cases and the binary exited 0, so
// the gate reported a pass for a run that checked nothing. Every check that
// must hold in the gate exits explicitly. S193, 2026-09-04_test_review-F05.
json load_json(const std::string& filename)
{
  std::ifstream file(filename);
  if (!file) {
    std::cerr << RED << "test_perft: cannot open asset: " << filename << RESET
              << "\nRun from a directory holding assets/perft_json/, such as "
                 "build/tests."
              << std::endl;
    std::exit(2);
  }

  json json_data;
  try {
    file >> json_data;
  } catch (const json::parse_error& e) {
    std::cerr << RED << "test_perft: cannot parse asset: " << filename << ": "
              << e.what() << RESET << std::endl;
    std::exit(2);
  }

  file.close();

  return json_data;
}


// The pieces of the side that just moved bearing on the side to move's king,
// computed on the position `move` has already been made into. is_attacked()
// answers "any", and three of the four columns need the set itself: how many
// checkers there are, and whether one of them is a piece that did not move.
// It is the same six lookups is_attacked() does, without its early exits, and
// the king is left out because a king cannot give check. S205.
static bb_t checkers_of(const game_t* game)
{
  const board_t* board = &game->board;
  const color_t us = board->active_color;

  const bb_t king = board->bitboards[(us == WHITE) ? W_KING : B_KING];

  // Same guard as is_check(): a side with no king is not in check, and
  // get_lsb_index() would return 64 into every attack table. Reachable through
  // a malformed FEN, not through any asset here.
  if (king == BB_0) { return BB_0; }

  const index_t index = get_lsb_index(king);
  const bb_tables_t* tables = game_tables();
  const bb_t occupancy = board->occupancies[BOTH];

  // Contiguous per-side bitboards, as is_attacked() reads them: index 0 is the
  // pawn of the side that just moved, 4 its queen.
  const bb_t* pieces = &board->bitboards[(us == WHITE) ? B_PAWN : W_PAWN];
  const bb_t queens = pieces[4];

  return (tables->pawn_attacks[us][index] & pieces[0]) |
         (tables->knight_attacks[index] & pieces[1]) |
         (get_bishop_attacks(tables, index, occupancy) & (pieces[2] | queens)) |
         (get_rook_attacks(tables, index, occupancy) & (pieces[3] | queens));
}


// The squares this move put a piece on. Normally just the target; a castle
// moves two pieces, and a rook that checks from f1 or d1 has moved, so it is
// not a discovered check. The rook's destination is the square the king
// stepped over, which is the midpoint of the king's own two squares for both
// sides and both wings.
static bb_t moved_to_mask(const move_t& move)
{
  bb_t mask = BB_1 << MOVE_TO(move);

  if (MOVE_CASTLING(move)) {
    mask |= BB_1 << ((MOVE_FROM(move) + MOVE_TO(move)) / 2);
  }

  return mask;
}


// `game` is the position after `move` was made, which is where all four check
// columns are decided. Before S205 the four were set to 0 unconditionally
// while 33 of the 58 asset layers a run reads carried real expected values for
// two of them, so the columns were parsed and never compared.
stats_t get_move_stats(const move_t& move, const game_t* game)
{
  stats_t stats;
  stats.nodes += 1;
  stats.captures += (MOVE_CAPTURE(move) ? 1 : 0);
  stats.en_passants += (MOVE_EN_PASSANT(move) ? 1 : 0);
  stats.castles += (MOVE_CASTLING(move) ? 1 : 0);
  stats.promotions += (MOVE_PROMOTED(move) > 0 ? 1 : 0);

  const bb_t checkers = checkers_of(game);

  if (checkers != BB_0) {
    stats.checks = 1;

    // generate_moves() emits legal moves only, so mate is an empty list and
    // needs no make_move to confirm it. Paid at check nodes alone, which is
    // why counting all four columns costs a quarter of the run and not a
    // multiple of it.
    move_t replies[270];
    const bool mate =
        (generate_moves(game_tables(), &game->board, replies) == 0);

    // The convention the assets are written in, established in S205 by
    // classifying every layer that carries a value under each reading and
    // taking the one that matched all 29 with no exception: `checks` is every
    // check leaf including mates, `checkmates` is every mate, and the two
    // classification columns describe the non-mating checks only and are
    // exclusive of each other -- a double check that is also discovered is
    // counted once, as double. The one layer that decides each exclusion is
    // Kiwipete at depth 5: 8 of its 2645 two-checker leaves are mate (2637
    // expected), and 12 of its 19895 discovered checks are castles (19883
    // expected).
    if (mate) {
      stats.checkmates = 1;
    } else if (count_bits(checkers) > 1) {
      stats.double_checks = 1;
    } else if ((checkers & ~moved_to_mask(move)) != BB_0) {
      stats.discovery_checks = 1;
    }
  }

  return stats;
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
  const stats_t* stats_in_tt = get_from_tt(&game->board, depth);
  if (stats_in_tt != nullptr) { return *stats_in_tt; }

  move_t moves[270];
  const size_t moves_count = generate_moves(game_tables(), &game->board, moves);

  for (size_t i = 0; i < moves_count; ++i) {
    if (make_move(game, moves[i])) {
      if (depth == 1) {
        node_stats += get_move_stats(moves[i], game);
      } else {
        node_stats.nodes += 1;
      }

      node_stats.nodes -= 1;
      node_stats += perft(depth - 1, game);

      unmake_move(game);
    }
  }

  store_to_tt(&game->board, depth, &node_stats);
  return node_stats;
}


// One definition of "this column agrees", read by the colours below and by
// main()'s pass flag, so a red cell and a zero exit status cannot disagree.
// Only `nodes` used to reach the flag; the other four were printed in red and
// the run still passed. A column the asset leaves null makes no claim and
// decides nothing. S193.
template <typename E, typename R>
static bool column_matches(const std::optional<E>& expected, R real)
{
  return !expected.has_value() ||
         static_cast<uint64_t>(expected.value()) == static_cast<uint64_t>(real);
}


static bool columns_match(const expected_stats_t& expected, const stats_t& real)
{
  return column_matches(expected.nodes, real.nodes) &&
         column_matches(expected.captures, real.captures) &&
         column_matches(expected.en_passants, real.en_passants) &&
         column_matches(expected.castles, real.castles) &&
         column_matches(expected.promotions, real.promotions) &&
         column_matches(expected.checks, real.checks) &&
         column_matches(expected.discovery_checks, real.discovery_checks) &&
         column_matches(expected.double_checks, real.double_checks) &&
         column_matches(expected.checkmates, real.checkmates);
}


std::string print_stats(const expected_stats_t& expected, const stats_t real)
{
  std::stringstream ss;
  const int width = 10;
  // clang-format off
  ss << std::left;

  if (expected.nodes.has_value()) { 
    ss << (column_matches(expected.nodes, real.nodes) ? GREEN : RED) << std::setw(15) << std::string("|").append(STR(expected.nodes.value())) << std::setw(15) << real.nodes << RESET;
  } else {
    ss << std::setw(15) << "| - " << std::setw(15) << real.nodes;
  }

  if (expected.captures.has_value()) { 
    ss << (column_matches(expected.captures, real.captures) ? GREEN : RED) << std::setw(15) << std::string("|").append(STR(expected.captures.value())) << std::setw(15) << real.captures << RESET;
  } else {
    ss << std::setw(15) << "| - " << std::setw(15) << real.captures;
  }

  if (expected.en_passants.has_value()) { 
    ss << (column_matches(expected.en_passants, real.en_passants) ? GREEN : RED) << std::setw(width) << std::string("|").append(STR(expected.en_passants.value())) << std::setw(width) << real.en_passants << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.en_passants;
  }

  if (expected.castles.has_value()) { 
    ss << (column_matches(expected.castles, real.castles) ? GREEN : RED) << std::setw(width) << std::string("|").append(STR(expected.castles.value())) << std::setw(width) << real.castles << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.castles;
  }

  if (expected.promotions.has_value()) { 
    ss << (column_matches(expected.promotions, real.promotions) ? GREEN : RED) << std::setw(width) << std::string("|").append(STR(expected.promotions.value())) << std::setw(width) << real.promotions << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.promotions;
  }

  if (expected.checks.has_value()) { 
    ss << (column_matches(expected.checks, real.checks) ? GREEN : RED) << std::setw(15) << std::string("|").append(STR(expected.checks.value())) << std::setw(15) << real.checks << RESET;
  } else {
    ss << std::setw(15) << "| - " << std::setw(15) << real.checks;
  }

  if (expected.discovery_checks.has_value()) { 
    ss << (column_matches(expected.discovery_checks, real.discovery_checks) ? GREEN : RED) << std::setw(width) << std::string("|").append(STR(expected.discovery_checks.value())) << std::setw(width) << real.discovery_checks << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.discovery_checks;
  }

  if (expected.double_checks.has_value()) { 
    ss << (column_matches(expected.double_checks, real.double_checks) ? GREEN : RED) << std::setw(width) << std::string("|").append(STR(expected.double_checks.value())) << std::setw(width) << real.double_checks << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.double_checks;
  }

  if (expected.checkmates.has_value()) { 
    ss << (column_matches(expected.checkmates, real.checkmates) ? GREEN : RED) << std::setw(width) << std::string("|").append(STR(expected.checkmates.value())) << std::setw(width) << real.checkmates << RESET;
  } else {
    ss << std::setw(width) << "| - " << std::setw(width) << real.checkmates;
  }
  // clang-format on
  return ss.str();
}


int main()
{
  initialize_game_const_data(&game);

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
        load_FEN(fen, &game);
        stats_t stats;
        stats.nodes = 1;

        if (depth > 0) {
          stats = stats_t();

          move_t moves[270];
          const size_t moves_count =
              generate_moves(game_tables(), &game.board, moves);

          if (depth > 1) {
            for (size_t i = 0; i < moves_count; ++i) {
              if (make_move(&game, moves[i])) {
                const auto res = perft(depth - 1, &game);
                unmake_move(&game);
                stats += res;
              }
            }
          } else {
            // In case depth 1 we try for legal moves and count stats
            for (size_t i = 0; i < moves_count; ++i) {
              if (make_move(&game, moves[i])) {
                stats += get_move_stats(moves[i], &game);
                unmake_move(&game);
              }
            }
          }
        }

        const auto end_time = std::chrono::high_resolution_clock::now();

        if (!columns_match(expected_stats, stats)) { passed = false; }

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
                << std::setw(width - 2) << std::string("|").append(STR(depth))
                << std::setw(width / 2) << std::string("|").append(STR(minutes.count()))
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
