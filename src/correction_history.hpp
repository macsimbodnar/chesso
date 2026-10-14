#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "data_structures.hpp"
#include "search_params.hpp"

// STATIC EVALUATION CORRECTION HISTORY, S099: the read, the update and the
// clear of correction_table_t (src/data_structures.hpp), in one place so the
// search, the UCI layer, datagen and the tests cannot disagree about the slot,
// the fixed point or the clamp.
//
// The published form, implemented from the step file's description and the
// Chess Programming Wiki's Static Evaluation Correction History page, whose
// shape and nothing else is taken (DEC-221; the page's code blocks are other
// engines' source and none of their numbers is behind any constant here,
// DEC-105, DEC-134): an exponential moving average per entry,
//
//   entry = (entry * (SCALE - weight) + observation * weight) / SCALE
//
// with the weight rising with the remaining depth and capped, and the entry
// clamped to a maximum. The observation is the search's score less the raw
// static evaluation, in fixed point. Which nodes may teach the table, and
// where the read is added, is src/search.cpp's: negamax_at reads the entry once
// at its compute-or-read site and updates it beside its one store.

// The slot a key lands in: its low bits. The side to move is the other index,
// never folded into the key (correction_table_t says why).
template <size_t Entries>
inline size_t correction_slot(hash_t key)
{ return static_cast<size_t>(key & (Entries - 1)); }


// What the entry adds to a raw static evaluation, in evaluation units: the
// entry over the grain, truncated toward zero, and never more than
// CORRHIST_MAX either way. The update already holds the entry inside that
// bound; the read holds it again so that a CorrHistMax lowered over a warm
// table in the tune build cannot apply more than it says.
template <size_t Entries>
inline int correction_value(const correction_table_t<Entries>* table,
                            color_t side,
                            hash_t key)
{
  assert(table != nullptr);
  assert(side == WHITE || side == BLACK);

  const int value =
      table->entries[side][correction_slot<Entries>(key)] / CORRHIST_GRAIN;

  return std::clamp(value, -CORRHIST_MAX, CORRHIST_MAX);
}


// The weight an observation at this remaining depth is given, out of
// CORRHIST_SCALE: `(depth + 1)^2`, capped at CorrHistWeightCap and never above
// the scale itself, where the entry would overshoot the observation instead of
// moving toward it. The cap's declared range already ends at the scale; this
// holds the arithmetic whatever the range says.
inline int correction_weight(int depth)
{
  const int grown = (depth + 1) * (depth + 1);

  return std::min(std::min(grown, CORRHIST_WEIGHT_CAP), CORRHIST_SCALE);
}


// One observation: `diff` is the search's score less the raw static
// evaluation, both relative to `side`, the side to move, at a node of
// remaining depth `depth`. Whether the node may teach the table at all is the
// caller's question (src/search.cpp `correction_may_learn`).
//
// The observation is clamped to +/-CORRHIST_MAX before it is scaled, so one
// wild score moves an entry no further than a clamped one would, and every
// product below stays inside int32: |entry| and |observation| are at most
// CORRHIST_MAX * CORRHIST_GRAIN, each multiplied by at most CORRHIST_SCALE.
// Clamping before the multiply is also what keeps `diff * CORRHIST_GRAIN`
// from overflowing on a difference the gate let through.
template <size_t Entries>
inline void correction_update(correction_table_t<Entries>* table,
                              color_t side,
                              hash_t key,
                              int diff,
                              int depth)
{
  assert(table != nullptr);
  assert(side == WHITE || side == BLACK);

  const int bound = CORRHIST_MAX * CORRHIST_GRAIN;
  const int observation =
      std::clamp(diff, -CORRHIST_MAX, CORRHIST_MAX) * CORRHIST_GRAIN;
  const int weight = correction_weight(depth);

  int32_t& entry = table->entries[side][correction_slot<Entries>(key)];

  const int updated =
      (entry * (CORRHIST_SCALE - weight) + observation * weight) /
      CORRHIST_SCALE;

  entry = std::clamp(updated, -bound, bound);
}


// Every entry back to 0, the table's state at the start of a game. 0 is the
// off value of every slot: it adds nothing to the evaluation it is read for.
template <size_t Entries>
inline void correction_reset(correction_table_t<Entries>* table)
{
  assert(table != nullptr);

  memset(static_cast<void*>(table->entries), 0, sizeof(table->entries));
}
