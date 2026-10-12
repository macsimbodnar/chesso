#include "transposition_table.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "log.hpp"

#if defined(__linux__)
#include <sys/mman.h>
#endif


namespace
{

// The transparent huge page on x86-64 Linux, and on arm64 Linux with 4 KiB
// base pages. Only an alignment: madvise() accepts any range, and the kernel
// can back with a huge page exactly the 2 MiB-aligned blocks inside it, so a
// table that starts on this boundary loses none of its blocks to the edges.
constexpr size_t TT_HUGE_PAGE = size_t(2) * 1024 * 1024;

// The line a probe reads, and a cluster's size and alignment. The huge-page
// boundary is a multiple of it, and the fallback aligns to it by hand.
constexpr size_t TT_LINE = 64;

static_assert(TT_LINE == sizeof(tt_cluster_t) &&
                  TT_LINE == alignof(tt_cluster_t),
              "the allocation aligns to what a cluster needs");
static_assert(TT_HUGE_PAGE % TT_LINE == 0,
              "the huge-page alignment is a line alignment too");

// What one search of age costs an entry, in plies, when replacement weighs
// it against depth (tt_store_entry()). Derived from how the engine plays
// rather than taken from anywhere (DEC-134 form (b)): one search per own move,
// so the root of the next search is two plies below this one's, and a node
// both reach is two plies nearer that root -- searched about two plies deeper
// there, if the iteration depth holds from one move to the next. An entry's
// depth is therefore worth two plies less to each later search. S119.
constexpr int TT_AGE_PLIES = 2;


// Zeroed memory for `bytes` of table, recorded on `tt` so tt_release() can
// return it. S119.
//
// On Linux, an anonymous mapping, aligned to the huge page and advised
// MADV_HUGEPAGE. The kernel zeroes it page by page on first touch -- what
// calloc() does for a block this size anyway -- so a large Hash that no
// search fills commits no memory, as before. This machine runs transparent
// huge pages in `madvise` mode (/sys/kernel/mm/transparent_hugepage/enabled),
// where nothing is huge unless it is asked for. The advice is only that: a
// kernel without THP refuses it or ignores it, and the table then sits on
// ordinary pages and works exactly as it did. That is the whole failure path.
// It never aborts, and a refused mapping falls through to calloc() below.
//
// Elsewhere (macOS has no such advice), calloc() with a line of slack, aligned
// by hand.
void* tt_allocate(transposition_table_t* tt, size_t bytes)
{
#if defined(__linux__) && defined(MADV_HUGEPAGE)
  const size_t length = bytes + TT_HUGE_PAGE;

  void* mapped = mmap(nullptr, length, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (mapped != MAP_FAILED) {
    const uintptr_t at =
        (reinterpret_cast<uintptr_t>(mapped) + TT_HUGE_PAGE - 1) &
        ~static_cast<uintptr_t>(TT_HUGE_PAGE - 1);

    if (madvise(reinterpret_cast<void*>(at), bytes, MADV_HUGEPAGE) != 0) {
      LOG_W << "TT huge pages refused, using ordinary pages" << END_W;
    }

    tt->allocation = mapped;
    tt->allocation_bytes = length;
    tt->allocation_mapped = true;

    return reinterpret_cast<void*>(at);
  }

  LOG_W << "TT mapping of " << length << " bytes failed, trying calloc"
        << END_W;
#endif

  void* block = calloc(1, bytes + TT_LINE);

  if (block == nullptr) { return nullptr; }

  tt->allocation = block;
  tt->allocation_bytes = bytes + TT_LINE;
  tt->allocation_mapped = false;

  const uintptr_t at = (reinterpret_cast<uintptr_t>(block) + TT_LINE - 1) &
                       ~static_cast<uintptr_t>(TT_LINE - 1);

  return reinterpret_cast<void*>(at);
}


void tt_release(transposition_table_t* tt)
{
#if defined(__linux__) && defined(MADV_HUGEPAGE)
  if (tt->allocation_mapped) {
    munmap(tt->allocation, tt->allocation_bytes);
  } else {
    free(tt->allocation);
  }
#else
  free(tt->allocation);
#endif

  tt->allocation = nullptr;
  tt->allocation_bytes = 0;
  tt->allocation_mapped = false;
  tt->clusters = nullptr;
  tt->cluster_count = 0;
  tt->entry_count = 0;
}


// What replacement weighs an entry at: its depth, less what its age costs.
int tt_worth(const tt_entry_t& entry, uint16_t now)
{
  return entry.depth -
         TT_AGE_PLIES * static_cast<int>(tt_age(now, entry.generation));
}

}  // namespace


void tt_resize(transposition_table_t* tt, size_t megabytes)
{
  assert(tt != nullptr);

  megabytes = std::clamp<size_t>(megabytes, TT_MIN_MB, TT_MAX_MB);

  tt_release(tt);

  // Every megabyte asked for, as whole lines. A megabyte is 16384 of them, so
  // nothing is rounded at all; until S119 the entry count was floored to a
  // power of two, and Hash 16 used 12 MiB of 24-byte entries.
  size_t count = (megabytes * 1024 * 1024) / sizeof(tt_cluster_t);

  // A machine that cannot spare the requested size still has to play, so keep
  // halving rather than giving up on the table entirely.
  while (count > 0) {
    tt->clusters = static_cast<tt_cluster_t*>(
        tt_allocate(tt, count * sizeof(tt_cluster_t)));

    if (tt->clusters != nullptr) { break; }

    LOG_W << "TT allocation of " << count << " clusters failed, halving"
          << END_W;
    count /= 2;
  }

  if (tt->clusters == nullptr) {
    LOG_E << "Could not allocate a transposition table. Running without one."
          << END_E;
    return;
  }

  tt->cluster_count = count;
  tt->entry_count = count * TT_CLUSTER_ENTRIES;
  tt->generation = 1;  // 1 makes it stable

  LOG_I << "TT sized to " << megabytes << "MB (" << count << " clusters, "
        << tt->entry_count << " entries, "
        << ((count * sizeof(tt_cluster_t)) / (1024 * 1024)) << "MB used)"
        << END_I;
}


void tt_free(transposition_table_t* tt)
{
  assert(tt != nullptr);

  tt_release(tt);
}


void tt_reset(transposition_table_t* tt)
{
  assert(tt != nullptr);

  LOG_I << "Cleanup TT" << END_I;

  // Only the entries are cleared. Wiping the struct itself would drop the
  // pointer to the allocation and leak it.
  if (tt->clusters != nullptr) {
    memset(static_cast<void*>(tt->clusters), 0,
           tt->cluster_count * sizeof(tt_cluster_t));
  }

  tt->generation = 1;  // 1 makes it stable
}


void tt_new_search(transposition_table_t* tt)
{
  assert(tt != nullptr);

  tt->generation++;

  // 0 is reserved for entries that were never written, so the counter runs
  // 1 to 65535 and comes round; tt_age() reads distances on that cycle.
  if (tt->generation == 0) { tt->generation = 1; }
}


const tt_entry_t* tt_get_entry(const transposition_table_t* tt,
                               const board_t* board)
{
  assert(tt != nullptr);
  assert(board != nullptr);

  if (tt->clusters == nullptr) { return nullptr; }

  const uint64_t hash = board->hash;
  const tt_cluster_t* cluster = &tt->clusters[tt_cluster_index(tt, hash)];
  const uint32_t key = tt_key_of(hash);

  // THE COLLISION TRADE, S119 and DEC-258. A match is the 32 key bits and the
  // cluster index, about 50 of the hash's 64 bits at Hash 16, where until S119
  // it was all 64. A position that is not in the table matches some other
  // position's entry with probability 3 / 2^32 per probe, 7e-10, so a reader
  // can be handed another position's score, depth, bound, move or evaluation.
  // What guards each reader:
  //
  //   the move    -- never played or trusted on its own. Ordering, the
  //                  singular extension's excluded move and the reduction
  //                  heuristics compare it against the generated list, and the
  //                  mate walk refuses a table move the generator does not
  //                  produce (complete_mate_pv()). A move from another
  //                  position is a no-op there, never an illegal move made.
  //   a mate line -- complete_mate_pv() publishes a line only when it ends in
  //                  checkmate at exactly the claimed distance, so a colliding
  //                  entry's exact mate score and move cannot extend a line
  //                  that does not deliver it.
  //   the score, depth, bound and evaluation -- not authenticated. A colliding
  //                  entry can cut a non-PV node, refine a stand pat or a
  //                  pruning margin, or stand in for evaluate(), and a mate
  //                  score read that way is a claim no search made here. The
  //                  rate is what bounds it, and the step file states it.
  //
  // `generation != 0` keeps a never-written entry, key 0, from matching a hash
  // whose low 32 bits are 0.
  for (const tt_entry_t& entry : cluster->entries) {
    if (entry.key == key && entry.generation != 0) { return &entry; }
  }

  return nullptr;
}


int tt_hashfull(const transposition_table_t* tt)
{
  assert(tt != nullptr);

  if (tt->clusters == nullptr) { return 0; }

  // 334 clusters are 1002 entries: the smallest sample of whole clusters that
  // is at least the thousand a permille is out of.
  const size_t sampled = std::min<size_t>(tt->cluster_count, 334);
  size_t current = 0;

  for (size_t i = 0; i < sampled; ++i) {
    for (const tt_entry_t& entry : tt->clusters[i].entries) {
      if (entry.generation == tt->generation) { current++; }
    }
  }

  return static_cast<int>((current * 1000) / (sampled * TT_CLUSTER_ENTRIES));
}


int16_t tt_eval_to_store(const tt_entry_t* entry, uint64_t hash, int eval)
{
  assert(entry != nullptr);

  // Clamped rather than truncated, and asserted rather than trusted. Nothing
  // the evaluation can return comes near the bound (see tt_entry_t), so the
  // clamp is there to make a future term that does a wrong score instead of a
  // wrapped one.
  assert(eval == TT_EVAL_NONE || (eval > TT_EVAL_NONE && eval <= INT16_MAX));

  if (eval == TT_EVAL_NONE) {
    // Nothing to record, so nothing is destroyed. The main search stores the
    // sentinel at every node it was in check at and TT_DEPTH_QS sits below
    // every main depth, so without this a check node wipes the number
    // quiescence recorded for the same position -- which is what the field did
    // from S094 until S108.
    //
    // Only for this same position, and generation is what says the entry was
    // ever written: a fresh entry has key 0 and eval 0, and 0 is an ordinary
    // evaluation. Preserving across a key change would hand another
    // position's number to the next reader, which is worse than having none.
    // "Same" is the partial key's sameness (tt_get_entry() states the trade).
    const bool same_position =
        (entry->generation != 0 && entry->key == tt_key_of(hash));

    return same_position ? entry->eval : static_cast<int16_t>(TT_EVAL_NONE);
  }

  return static_cast<int16_t>(
      std::clamp(eval, TT_EVAL_NONE + 1, static_cast<int>(INT16_MAX)));
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

  if (tt->clusters == nullptr) { return; }

  const uint64_t hash = board->hash;
  tt_cluster_t* cluster = &tt->clusters[tt_cluster_index(tt, hash)];
  const uint32_t key = tt_key_of(hash);

  // This position's own entry, where the cluster has one. Then the rule it
  // always had: depth-preferred inside one search, always replaceable across
  // searches -- so a re-search at the same depth carries the newer bound, and
  // a shallower store from the same search is refused.
  tt_entry_t* entry = nullptr;

  for (tt_entry_t& way : cluster->entries) {
    if (way.generation != 0 && way.key == key) {
      entry = &way;
      break;
    }
  }

  if (entry != nullptr) {
    if (entry->generation == tt->generation && depth < entry->depth) { return; }
  } else {
    // ANOTHER POSITION'S ENTRY GIVES WAY, and one always does: a store of a
    // position the cluster does not hold is never refused. A way never written
    // is taken first. Otherwise the way worth least, depth less TT_AGE_PLIES
    // per search of age (tt_worth()), so depth and age are weighed together:
    // a deep entry from long ago gives way before a shallow one of this
    // search when its depth, discounted, is the smaller. The first such way on
    // a tie. Until S119 the slot was one entry, refused every shallower store
    // within a search, and an entry the 8-bit counter had come round to read
    // as current and refused them for that whole search (S271).
    entry = &cluster->entries[0];
    int least = tt_worth(*entry, tt->generation);

    for (tt_entry_t& way : cluster->entries) {
      if (way.generation == 0) {
        entry = &way;
        break;
      }

      const int worth = tt_worth(way, tt->generation);

      if (worth < least) {
        entry = &way;
        least = worth;
      }
    }
  }

  // Decided before the key is overwritten, because the decision reads it.
  const int16_t decided_eval = tt_eval_to_store(entry, hash, eval);

  entry->key = key;
  entry->score = score;
  entry->best_move = best_move;
  entry->depth = static_cast<int16_t>(depth);

  entry->eval = decided_eval;

  entry->type = static_cast<uint8_t>(type);
  entry->generation = tt->generation;
}
