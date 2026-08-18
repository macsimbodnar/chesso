#include "transposition_table.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "log.hpp"


void tt_resize(transposition_table_t* tt, size_t megabytes)
{
  assert(tt != nullptr);

  megabytes = std::clamp<size_t>(megabytes, TT_MIN_MB, TT_MAX_MB);

  free(tt->entries);
  tt->entries = nullptr;
  tt->entry_count = 0;
  tt->index_mask = 0;

  // Rounded down to a power of two so probing can mask instead of divide.
  size_t count = std::bit_floor((megabytes * 1024 * 1024) / sizeof(tt_entry_t));

  // A machine that cannot spare the requested size still has to play, so keep
  // halving rather than giving up on the table entirely.
  while (count > 0) {
    tt->entries = static_cast<tt_entry_t*>(calloc(count, sizeof(tt_entry_t)));

    if (tt->entries != nullptr) { break; }

    LOG_W << "TT allocation of " << count << " entries failed, halving"
          << END_W;
    count /= 2;
  }

  if (tt->entries == nullptr) {
    LOG_E << "Could not allocate a transposition table. Running without one."
          << END_E;
    return;
  }

  tt->entry_count = count;
  tt->index_mask = count - 1;
  tt->generation = 1;  // 1 makes it stable

  LOG_I << "TT sized to " << megabytes << "MB (" << count << " entries, "
        << ((count * sizeof(tt_entry_t)) / (1024 * 1024)) << "MB used)"
        << END_I;
}


void tt_free(transposition_table_t* tt)
{
  assert(tt != nullptr);

  free(tt->entries);
  tt->entries = nullptr;
  tt->entry_count = 0;
  tt->index_mask = 0;
}


void tt_reset(transposition_table_t* tt)
{
  assert(tt != nullptr);

  LOG_I << "Cleanup TT" << END_I;

  // Only the entries are cleared. Wiping the struct itself would drop the
  // pointer to the allocation and leak it.
  if (tt->entries != nullptr) {
    memset(tt->entries, 0, tt->entry_count * sizeof(tt_entry_t));
  }

  tt->generation = 1;  // 1 makes it stable
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

  if (tt->entries == nullptr) { return nullptr; }

  const uint64_t hash = board->hash;
  const tt_entry_t* res = &tt->entries[hash & tt->index_mask];

  if (res->key == hash) { return res; }

  return nullptr;
}


void tt_store_entry(transposition_table_t* tt,
                    const board_t* board,
                    int depth,
                    int score,
                    node_type_t type,
                    move_t best_move,
                    int eval)
{
  assert(tt != nullptr);
  assert(board != nullptr);

  if (tt->entries == nullptr) { return; }

  const uint64_t hash = board->hash;
  tt_entry_t* entry = &tt->entries[hash & tt->index_mask];

  // Depth-preferred inside one search, always replaceable across searches.
  if (entry->generation != tt->generation || depth >= entry->depth) {
    entry->key = hash;
    entry->score = score;
    entry->best_move = best_move;
    entry->depth = static_cast<int16_t>(depth);

    // Clamped rather than truncated, and asserted rather than trusted. Nothing
    // the evaluation can return comes near the bound (see tt_entry_t), so the
    // clamp is there to make a future term that does a wrong score instead of
    // a wrapped one.
    assert(eval == TT_EVAL_NONE || (eval > TT_EVAL_NONE && eval <= INT16_MAX));
    entry->eval =
        (eval == TT_EVAL_NONE)
            ? static_cast<int16_t>(TT_EVAL_NONE)
            : static_cast<int16_t>(std::clamp(eval, TT_EVAL_NONE + 1,
                                              static_cast<int>(INT16_MAX)));

    entry->type = static_cast<uint8_t>(type);
    entry->generation = tt->generation;
  }
}
