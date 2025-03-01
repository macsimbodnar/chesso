#include "utils.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <random>


void init_zobrist(zobrist_randoms_t* zobrist)
{
  assert(zobrist != nullptr);

  // TODO(max): Move random initialization outside
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

  for (uint64_t& random : zobrist->piece_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->castling_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->side_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->ep_randoms) {
    random = dist(gen);
  }
}


uint64_t init_zobrist_key()
{
  uint64_t key = 0;
  // TODO: implement
  return key;
}


void init_game_state(game_state_t* gs)
{
  assert(gs != nullptr);

  gs->active_color = WHITE;
  gs->castling = WQ | WK | BQ | BK;
  gs->half_move_clock = 0;
  gs->en_passant = std::optional<uint8_t>(std::nullopt);
  gs->full_move_number = 0;
  gs->zobrist_key = 0;
  gs->phase_value = 0;
  gs->next_move = move_t();
}