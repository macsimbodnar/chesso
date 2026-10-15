#pragma once
#include "data_structures.hpp"

// The depth a quiescence entry is stored at. Negative on purpose: negamax
// never asks the table for a depth below zero -- it recurses with depth - 1
// and only while depth >= 1, and every reduction is clamped so the child keeps
// at least one ply -- so an entry written here can never satisfy a main-search
// probe, while quiescence, which asks for this depth, accepts every entry in
// the table. The asymmetry is the whole point: a quiescence score has seen
// captures and nothing else and must not answer for a node that was going to
// search quiet moves. S094.
inline constexpr int TT_DEPTH_QS = -1;

// The generation counter's cycle: 1 to 65535, since tt_new_search() skips the
// 0 that marks a slot never written. S119.
inline constexpr unsigned TT_GENERATION_CYCLE = 65535;

// How many searches ago generation `then` was current, when `now` is. A
// distance on the counter's cycle, never a comparison of the two numbers: the
// counter comes round, and an entry written at 65535 is one search old when the
// counter reads 1, not 65534 searches in the future. What the cycle cannot
// tell apart is an entry untouched for exactly a multiple of 65535 searches,
// which reads as 0 -- current -- for that one search and 1 again at the next;
// replacement is the only thing that reads an age, so that is one way of one
// cluster kept a search longer, and a store of another position still lands
// (tt_store_entry()). S271 found the same aliasing at 255 on the 8-bit counter,
// where a direct-mapped slot refused every shallower store for the search.
inline unsigned tt_age(uint16_t now, uint16_t then)
{ return (now + TT_GENERATION_CYCLE - then) % TT_GENERATION_CYCLE; }

// The top half of the 128-bit product a * b, from 32-bit halves, for a
// compiler without a 128-bit type. tt_cluster_index() uses it only there; the
// suite checks it against the 128-bit product wherever that exists.
inline uint64_t tt_mul_hi64_halves(uint64_t a, uint64_t b)
{
  const uint64_t a_lo = a & 0xffffffffULL;
  const uint64_t a_hi = a >> 32;
  const uint64_t b_lo = b & 0xffffffffULL;
  const uint64_t b_hi = b >> 32;

  const uint64_t lo_lo = a_lo * b_lo;
  const uint64_t hi_lo = a_hi * b_lo;
  const uint64_t lo_hi = a_lo * b_hi;

  const uint64_t cross =
      (lo_lo >> 32) + (hi_lo & 0xffffffffULL) + (lo_hi & 0xffffffffULL);

  return a_hi * b_hi + (hi_lo >> 32) + (lo_hi >> 32) + (cross >> 32);
}

// The cluster `hash` lives in: its high bits scaled onto [0, cluster_count) by
// one 64 x 64 multiply, keeping the top half of the product. Any count works,
// so a Hash that is not a power of two is used whole. The low 32 bits, which
// this barely reads, are what tt_entry_t::key checks. S119.
inline size_t tt_cluster_index(const transposition_table_t* tt, uint64_t hash)
{
#if defined(__SIZEOF_INT128__)
  return static_cast<size_t>(
      (static_cast<unsigned __int128>(hash) * tt->cluster_count) >> 64);
#else
  return static_cast<size_t>(tt_mul_hi64_halves(hash, tt->cluster_count));
#endif
}

// The part of `hash` an entry keeps and a probe compares.
inline uint32_t tt_key_of(uint64_t hash)
{ return static_cast<uint32_t>(hash); }

// Allocates (or reallocates) the table. Must be called before any probe or
// store; everything else tolerates an unallocated table by doing nothing.
void tt_resize(transposition_table_t* tt, size_t megabytes);

void tt_free(transposition_table_t* tt);

void tt_reset(transposition_table_t* tt);

void tt_new_search(transposition_table_t* tt);

const tt_entry_t* tt_get_entry(const transposition_table_t* tt,
                               const board_t* board);

// UCI's `hashfull`: permille of the entries the current search has written,
// read off the first clusters of the table (1002 entries, or the whole table
// where it is smaller), which a uniform hash fills at the rate of the rest.
// An entry an earlier search wrote is not counted -- the table is always full
// of those after the first few searches, so counting them would read 1000 and
// say nothing. Reporting only. S119.
int tt_hashfull(const transposition_table_t* tt);

// Asks the memory system for the cluster `hash` indexes, without waiting for
// it. The search calls this the moment a child's key is final -- right after
// make_move() or make_null_move() returns -- so the line is on its way while
// the child runs its draw and repetition tests, and the probe at the top of the
// child finds it in cache more often. A hint and nothing else: it reads and
// writes no entry, so no node count can move (INV-6). One line, because a
// cluster is one aligned line. make_move() itself stays ignorant of the table,
// so perft and the generator pay nothing for it and the NNUE hook's alphabet
// is untouched. S119.
inline void tt_prefetch(const transposition_table_t* tt, uint64_t hash)
{
#if defined(__GNUC__) || defined(__clang__)
  if (tt->clusters == nullptr) { return; }

  __builtin_prefetch(&tt->clusters[tt_cluster_index(tt, hash)]);
#else
  (void)tt;
  (void)hash;
#endif
}

// What the eval field becomes on a store, given what the entry already holds.
// Split out of tt_store_entry() because the rule it carries is one line and one
// hazard: an incoming TT_EVAL_NONE keeps the number that is there, and it may
// only do that when the entry is this same position and was ever written --
// handing a colliding position's evaluation to the next reader is worse than
// having none, and a fresh slot's key 0 and eval 0 are not an evaluation of the
// position whose hash is 0. The collision cannot be produced from a FEN on
// demand, so the decision is tested directly rather than through a store. S108.
int16_t tt_eval_to_store(const tt_entry_t* entry, uint64_t hash, int eval);


// best_move may be 0. Quiescence stores a node it stood pat on, and standing
// pat is not a move; a probe reads the zero back as "no move to try first",
// which is what it already does for a slot that was never written. S094.
//
// `eval` is the position's static evaluation, defaulted for the callers that
// never computed one. It has to be the score itself and never a bound -- until
// S039 the lazy shortcut returned one in its place -- because a bound is only
// true on the side of the window it was taken at, and an entry outlives that
// window. S094.
//
// Passing TT_EVAL_NONE does not erase an evaluation this same position already
// has on the slot; tt_eval_to_store() is that rule. S108.
void tt_store_entry(transposition_table_t* tt,
                    const board_t* board,
                    int depth,
                    int score,
                    node_type_t type,
                    move_t best_move,
                    int eval = TT_EVAL_NONE);
