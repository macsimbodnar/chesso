// S122 phase 2: the killer slots of tests/test_search.cpp "a search fills the
// ordering tables", that case's search line for line over a depth range, each
// depth's first-slot, second-slot and duplicated counts printed. The case's
// depth is the lowest depth >= 8 at which the shipped library duplicates a
// second slot and S149's guard on the shift (the second slot written only when
// the new killer differs from the first) duplicates none. Build against a
// library and run, from the repository root:
//
//   c++ -I src -O3 -DNDEBUG -std=gnu++20 -march=native -o /tmp/killers \
//       adocs/data/S122_killer_slots.cpp build/src/libchesso_engine.a
//   /tmp/killers 6 12
//
// For the guard, apply it to src/search.cpp's shift in a throwaway tree,
// rebuild the library and link against that. adocs/data/S122_goldens.txt has
// S122's readings. Read-only over the engine.
#include <cstdio>
#include <cstdlib>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
game_t game;
transposition_table_t tt;
int main(int argc, char** argv)
{
  const int lo = atoi(argv[1]), hi = atoi(argv[2]);
  initialize_game_const_data(&game);
  tt_resize(&tt, 4);
  for (int depth = lo; depth <= hi; ++depth) {
    load_FEN(KIWIPETE_POS, &game);
    static std::atomic_bool never_stop = false;
    tt_reset(&tt);
    tt_new_search(&tt);
    search_state_t* state = new search_state_t();
    state->tt = &tt;
    state->stop = &never_stop;
    const search_t result = search(depth, &game, state);
    size_t k0 = 0, k1 = 0, dup = 0;
    for (size_t ply = 0; ply < MAX_PLY; ++ply) {
      if (state->killer_moves[0][ply] != 0) { k0++; }
      if (state->killer_moves[1][ply] != 0) {
        k1++;
        if (state->killer_moves[1][ply] == state->killer_moves[0][ply]) { dup++; }
      }
    }
    printf("depth %2d best %d killers_0 %zu killers_1 %zu duplicated %zu\n", depth,
           result.best_move != 0, k0, k1, dup);
    fflush(stdout);
    delete state;
  }
  return 0;
}
