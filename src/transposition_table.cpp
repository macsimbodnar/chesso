#include "transposition_table.hpp"
#include <cassert>
#include "log.hpp"


void tt_reset(transposition_table_t* tt)
{
  LOG_I << "Cleanup TT" << END_I;
  memset(tt, 0, sizeof(transposition_table_t));

  tt->generation = 1; // 1 makes it stable
}


void tt_new_search(transposition_table_t* tt)
{
  assert(tt != nullptr);

  tt->generation++;

  // 0 is reserved for slots that were never written.
  if (tt->generation == 0) { tt->generation = 1; }
}


const tt_entry_t* tt_get_entry(const transposition_table_t* tt,
                               const board_t* board)
{
  assert(tt != nullptr);
  assert(board != nullptr);

  const uint64_t hash = board->hash;
  const tt_entry_t* res = &tt->entries[hash % TT_SIZE];

  assert(res != nullptr);

  if (res->key == hash) { return res; }

  return nullptr;
}


void tt_store_entry(transposition_table_t* tt,
                    const board_t* board,
                    int depth,
                    int score,
                    node_type_t type,
                    move_t best_move)
{
  assert(tt != nullptr);
  assert(board != nullptr);
  assert(best_move != 0);

  const uint64_t hash = board->hash;
  tt_entry_t* entry = &tt->entries[hash % TT_SIZE];

  // Depth-preferred inside one search, always replaceable across searches.
  if (entry->generation != tt->generation || depth >= entry->depth) {
    entry->key = hash;
    entry->score = score;
    entry->best_move = best_move;
    entry->depth = static_cast<int16_t>(depth);
    entry->type = static_cast<uint8_t>(type);
    entry->generation = tt->generation;
  }
}
