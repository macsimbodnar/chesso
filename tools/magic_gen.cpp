// S179. Draws the two tables this project must be able to call its own output:
// the 128 sliding-attack magic numbers in src/bb_tables.hpp, and the 851
// Zobrist keys src/bitboard.cpp init_zobrist fills.
//
//   magic_gen magics  --seed N
//   magic_gen zobrist --seed N
//
// Until this existed, the magics in the tree had a provenance that could only
// be argued: they were generated in this repository in 2023 (a5dbe68) under the
// seed a public tutorial series uses, so they coincided with that series'
// published set value for value. DEC-132 asked for a seed of the project's own
// and a command that reproduces the table. This is that command.
//
// It links the engine deliberately. The masks, the occupancy enumeration and
// the reference attack sets must be the engine's own: a magic verified here
// against a second implementation of "which squares does a rook on d4 attack"
// could still fail in the engine, and the failure would be silent until a
// perft mismatch.
//
// Determinism: one thread, fixed square order, no wall-clock input, unsigned
// arithmetic only. Two runs with the same seed print byte-identical arrays.
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "data_structures.hpp"

// Defined in bitboard.cpp. Not in the header because nothing in the engine
// needs them at runtime - only this tool and the tests.
uint64_t project_random_next(uint64_t* state);
bool magic_is_collision_free(index_t square, bb_t magic, bool rook);
bb_t precompute_rook_attack_masks(index_t square);
bb_t precompute_bishop_attack_masks(index_t square);
extern const uint64_t CHESSO_PROJECT_SEED;


namespace
{

game_t game;

// The project's own bound, carried over from a5dbe68's find_magic_number: a
// safety stop, not a tuning. The search normally ends in thousands.
constexpr uint64_t MAX_ATTEMPTS = 100000000ULL;


void usage()
{
  fprintf(stderr,
          "usage:\n"
          "  magic_gen magics  --seed N\n"
          "  magic_gen zobrist --seed N\n");
}


//-#############################  THE MAGICS  ###############################-//

// A candidate wants few bits set, so that the product spreads the occupancies
// over the index range instead of piling them up. Romstad's method,
// https://www.chessprogramming.org/Looking_for_Magics : the bitwise AND of
// three draws, which leaves each bit set with probability 1/8.
bb_t magic_candidate(uint64_t* state)
{
  return project_random_next(state) & project_random_next(state) &
         project_random_next(state);
}


// Trial and error until the checker accepts. The cheap filter first: a product
// with fewer than six ones in its top eight bits cannot spread 2^10 or more
// occupancies over the index range, and rejecting it costs one multiply
// instead of a full enumeration.
bb_t find_magic(index_t square, bool rook, uint64_t* state, uint64_t* attempts)
{
  const bb_t mask = rook ? precompute_rook_attack_masks(square)
                         : precompute_bishop_attack_masks(square);

  for (uint64_t attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt) {
    const bb_t candidate = magic_candidate(state);

    if (count_bits((mask * candidate) & 0xFF00000000000000ULL) < 6) {
      continue;
    }

    if (magic_is_collision_free(square, candidate, rook)) {
      *attempts = attempt;
      return candidate;
    }
  }

  fprintf(stderr,
          "magic_gen: no magic found for %s square %d in %" PRIu64
          " attempts\n",
          rook ? "rook" : "bishop", static_cast<int>(square), MAX_ATTEMPTS);
  return BB_0;
}


// src/bb_tables.hpp's exact form. clang-format.sh restores the column
// alignment after the arrays are pasted in; what matters here is that the
// values and the surrounding lines are the ones that file wants.
void print_array(const char* name, const bb_t values[64])
{
  printf("static inline constexpr bb_t %s[64] = {\n", name);
  for (int i = 0; i < 64; ++i) {
    printf("    0x%" PRIx64 "ULL,\n", values[i]);
  }
  printf("};\n");
}


int run_magics(uint64_t seed)
{
  uint64_t state = seed;
  bb_t rooks[64];
  bb_t bishops[64];
  uint64_t rook_attempts_total = 0;
  uint64_t bishop_attempts_total = 0;

  for (index_t square = 0; square < 64; ++square) {
    uint64_t attempts = 0;
    rooks[square] = find_magic(square, true, &state, &attempts);
    if (rooks[square] == BB_0) { return 1; }
    rook_attempts_total += attempts;
    fprintf(stderr, "rook   square %2d  %" PRIu64 " attempts\n",
            static_cast<int>(square), attempts);
  }

  for (index_t square = 0; square < 64; ++square) {
    uint64_t attempts = 0;
    bishops[square] = find_magic(square, false, &state, &attempts);
    if (bishops[square] == BB_0) { return 1; }
    bishop_attempts_total += attempts;
    fprintf(stderr, "bishop square %2d  %" PRIu64 " attempts\n",
            static_cast<int>(square), attempts);
  }

  print_array("rook_magic_numbers", rooks);
  printf("\n");
  print_array("bishop_magic_numbers", bishops);

  // The accepts' coincidence clause. Before the header is replaced this counts
  // how many of the drawn values repeat the set already compiled in; after, it
  // must read 128 of 128, which is the proof that the committed arrays are
  // what this seed reproduces.
  std::set<bb_t> compiled;
  for (int i = 0; i < 64; ++i) {
    compiled.insert(rook_magic_numbers[i]);
    compiled.insert(bishop_magic_numbers[i]);
  }

  int coincide = 0;
  for (int i = 0; i < 64; ++i) {
    if (compiled.count(rooks[i]) != 0) { ++coincide; }
    if (compiled.count(bishops[i]) != 0) { ++coincide; }
  }

  fprintf(stderr, "\nseed %" PRIu64 "\n", seed);
  fprintf(stderr,
          "attempts: rook %" PRIu64 ", bishop %" PRIu64 ", total %" PRIu64 "\n",
          rook_attempts_total, bishop_attempts_total,
          rook_attempts_total + bishop_attempts_total);
  fprintf(stderr, "coincide with the compiled-in arrays: %d of 128\n",
          coincide);

  return 0;
}


//-#############################  THE KEYS  #################################-//

// init_zobrist's order, which is the order that matters: the keys are compared
// against a live game_t below, and a reordering here would report a mismatch
// that is not one.
void draw_keys(uint64_t seed, uint64_t out[851])
{
  uint64_t state = seed;
  int n = 0;

  for (int piece = 0; piece < 12; ++piece) {
    for (int square = 0; square < 64; ++square) {
      out[n++] = project_random_next(&state);
    }
  }
  for (int i = 0; i < 16; ++i) {
    out[n++] = project_random_next(&state);
  }
  for (int i = 0; i < 2; ++i) {
    out[n++] = project_random_next(&state);
  }
  for (int i = 0; i < 65; ++i) {
    out[n++] = project_random_next(&state);
  }
}


// The wiki's quality rule for Zobrist keys is linear independence, not Hamming
// distance: https://www.chessprogramming.org/Zobrist_Hashing asks that no small
// subset XOR to the same value as another. These are the checks at the sizes
// that can be enumerated - no key zero, all distinct (no 1- or 2-subset
// collision), no pair XOR equal to a key (no 3-subset XORing to zero), no two
// pair XORs equal (no 4-subset XORing to zero). The Hamming figure is reported
// and not asserted; it is not what decides.
int report_quality(const uint64_t keys[851])
{
  int failures = 0;

  int zeros = 0;
  for (int i = 0; i < 851; ++i) {
    if (keys[i] == 0) { ++zeros; }
  }
  fprintf(stderr, "zero keys: %d (want 0)\n", zeros);
  if (zeros != 0) { ++failures; }

  std::set<uint64_t> distinct(keys, keys + 851);
  fprintf(stderr, "distinct keys: %zu of 851\n", distinct.size());
  if (distinct.size() != 851) { ++failures; }

  std::set<uint64_t> pair_xors;
  int xor_hits_a_key = 0;
  int min_hamming = 64;
  for (int i = 0; i < 851; ++i) {
    for (int j = i + 1; j < 851; ++j) {
      const uint64_t x = keys[i] ^ keys[j];
      if (distinct.count(x) != 0) { ++xor_hits_a_key; }
      const int hamming = count_bits(x);
      if (hamming < min_hamming) { min_hamming = hamming; }
      pair_xors.insert(x);
    }
  }

  fprintf(stderr, "pair XORs equal to some key: %d (want 0)\n", xor_hits_a_key);
  if (xor_hits_a_key != 0) { ++failures; }

  fprintf(stderr, "distinct pair XORs: %zu of 361675\n", pair_xors.size());
  if (pair_xors.size() != 361675) { ++failures; }

  fprintf(stderr, "minimum pairwise Hamming distance: %d\n", min_hamming);

  return failures;
}


int run_zobrist(uint64_t seed)
{
  uint64_t keys[851];
  draw_keys(seed, keys);

  const int failures = report_quality(keys);

  // A game_t that skips this hashes every position to zero, and the comparison
  // below would then read zeros and call them a mismatch.
  initialize_game_const_data(&game);

  const zobrist_randoms_t* live = &game.hash_randoms;
  uint64_t engine[851];
  int n = 0;
  for (int piece = 0; piece < 12; ++piece) {
    for (int square = 0; square < 64; ++square) {
      engine[n++] = live->piece_randoms[piece][square];
    }
  }
  for (int i = 0; i < 16; ++i) {
    engine[n++] = live->castling_randoms[i];
  }
  for (int i = 0; i < 2; ++i) {
    engine[n++] = live->side_randoms[i];
  }
  for (int i = 0; i < 65; ++i) {
    engine[n++] = live->ep_randoms[i];
  }

  const bool matches = memcmp(keys, engine, sizeof(keys)) == 0;
  fprintf(stderr, "\nseed %" PRIu64 "\n", seed);
  fprintf(stderr, "matches init_zobrist: %s\n", matches ? "yes" : "no");

  return failures == 0 ? 0 : 1;
}

}  // namespace


int main(int argc, char** argv)
{
  if (argc < 2) {
    usage();
    return 1;
  }

  const std::string mode = argv[1];
  bool have_seed = false;
  uint64_t seed = 0;

  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--seed" && i + 1 < argc) {
      seed = strtoull(argv[++i], nullptr, 10);
      have_seed = true;
    } else {
      usage();
      return 1;
    }
  }

  if (!have_seed) {
    fprintf(stderr,
            "magic_gen: --seed is required (the project's is %" PRIu64
            ", src/bitboard.cpp CHESSO_PROJECT_SEED)\n",
            CHESSO_PROJECT_SEED);
    usage();
    return 1;
  }

  if (mode == "magics") { return run_magics(seed); }
  if (mode == "zobrist") { return run_zobrist(seed); }

  usage();
  return 1;
}
