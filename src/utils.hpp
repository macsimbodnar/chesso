#pragma once
#include "data_structures.hpp"
#include <cstdint>


void init_zobrist(zobrist_randoms_t *zobrist);
uint64_t init_zobrist_key();
void init_game_state(game_state_t *game_state);