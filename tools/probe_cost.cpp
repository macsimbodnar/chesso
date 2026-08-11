// S034 feasibility. What does a table probe cost against 1.36 ns of evaluate()?
//
// Two loops differing only by the table access, so the difference is the
// access and not the loop. Indices come from a small array sized to stay in
// L1, because in the real search the index comes from a hash already in a
// register and the point is to price the table read, not an index fetch.
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "data_structures.hpp"

namespace
{

constexpr size_t INDEX_COUNT = 8192;  // 32 KB of indices, L1 resident
constexpr int SWEEPS = 7;

double best_ms(const std::vector<double>& v)
{
  double b = v[0];
  for (double x : v) {
    if (x < b) { b = x; }
  }
  return b;
}

struct eval_cache_entry_t
{
  uint32_t key;
  int32_t score;
};

}  // namespace


int main()
{
  printf("sizeof(tt_entry_t)        = %zu bytes\n", sizeof(tt_entry_t));
  printf("sizeof(eval_cache_entry_t) = %zu bytes\n", sizeof(eval_cache_entry_t));

  // The table the engine actually allocates at the default Hash=16.
  const size_t tt_count = std::bit_floor((16u * 1024 * 1024) / sizeof(tt_entry_t));
  printf("tt at Hash=16             = %zu entries, %zu MB\n\n", tt_count,
         (tt_count * sizeof(tt_entry_t)) / (1024 * 1024));

  // A small direct-mapped eval cache, sized to stay in L2.
  const size_t small_count = 32768;  // 8 bytes each, 256 KB

  std::vector<tt_entry_t> tt(tt_count);
  std::vector<eval_cache_entry_t> small(small_count);

  for (size_t i = 0; i < tt_count; ++i) {
    tt[i].key = i * 0x9E3779B97F4A7C15ull;
    tt[i].score = static_cast<int32_t>(i);
  }
  for (size_t i = 0; i < small_count; ++i) {
    small[i].key = static_cast<uint32_t>(i);
    small[i].score = static_cast<int32_t>(i);
  }

  // Random-looking but fixed, so every loop below walks the same order.
  std::vector<uint32_t> idx_big(INDEX_COUNT);
  std::vector<uint32_t> idx_small(INDEX_COUNT);
  uint64_t state = 0x123456789ABCDEFull;

  for (size_t i = 0; i < INDEX_COUNT; ++i) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    idx_big[i] = static_cast<uint32_t>(state % tt_count);
    idx_small[i] = static_cast<uint32_t>((state >> 32) % small_count);
  }

  const int repeats = 20000;  // 163.8 M iterations per sweep

  auto time_it = [&](auto&& body) {
    body();  // warm-up
    std::vector<double> samples;
    for (int s = 0; s < SWEEPS; ++s) {
      const auto start = std::chrono::steady_clock::now();
      uint64_t sink = body();
      // Without this the compiler moves the entire loop past the second clock
      // read, because the result is not needed until after it, and every
      // measurement comes back as zero. It did, twice.
      asm volatile("" : : "r"(sink) : "memory");
      const auto end = std::chrono::steady_clock::now();
      samples.push_back(
          std::chrono::duration<double, std::milli>(end - start).count());
    }
    const double ms = best_ms(samples);
    return (ms * 1e6) / (static_cast<double>(repeats) * INDEX_COUNT);
  };

  // The `+ r` is what stops the compiler hoisting the whole inner loop out of
  // the repeat loop as a loop-invariant sum. It also keeps the accesses
  // independent of each other, which is the realistic case: in the search the
  // index comes from a hash already in a register and out-of-order execution
  // overlaps the misses.
  const size_t tt_mask = tt_count - 1;
  const size_t small_mask = small_count - 1;

  const double baseline = time_it([&]() -> uint64_t {
    uint64_t sink = 0;
    for (int r = 0; r < repeats; ++r) {
      for (size_t i = 0; i < INDEX_COUNT; ++i) {
        sink += (idx_big[i] + r) & tt_mask;
      }
    }
    return sink;
  });

  const double big = time_it([&]() -> uint64_t {
    uint64_t sink = 0;
    for (int r = 0; r < repeats; ++r) {
      for (size_t i = 0; i < INDEX_COUNT; ++i) {
        sink += tt[(idx_big[i] + r) & tt_mask].key;
      }
    }
    return sink;
  });

  const double smallp = time_it([&]() -> uint64_t {
    uint64_t sink = 0;
    for (int r = 0; r < repeats; ++r) {
      for (size_t i = 0; i < INDEX_COUNT; ++i) {
        sink += small[(idx_small[i] + r) & small_mask].key;
      }
    }
    return sink;
  });

  printf("index loop only                 %6.2f ns per iteration\n", baseline);
  printf("+ probe into the %zu-entry tt  %6.2f ns  (access %.2f ns)\n",
         tt_count, big, big - baseline);
  printf("+ probe into a 256 KB cache     %6.2f ns  (access %.2f ns)\n", smallp,
         smallp - baseline);
  printf("\nevaluate() today is 1.36 ns. With a recomputed mobility term it was "
         "15.93 ns.\n");

  return 0;
}
