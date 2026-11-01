// S101's research pass, 2026-11-01: (1) on a corpus, how many threat victims stand in
// their own king's zone, the overlap with king safety's zone attacks; (2) on the tuner
// fixture, which of the candidate cells its rows reach, each way. FEN field only, no
// label read. Its output is in adocs/data/S101_census.txt, read before S101's coverage
// rows landed.
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "S101_threat_core.hpp"
#include "test_eval_positions.hpp"
using namespace s101;
static const char* NAMES[25] = {"P>N","P>B","P>R","P>Q","N>R","N>Q","B>R","B>Q","R>Q","N>B","B>N",
  "hang P","hang N","hang B","hang R","hang Q","konly P","konly N","konly B","konly R",
  "push N","push B","push R","push Q","unused"};
static void cells(const bb_tables_t* t, const board_t* b, int out[2][24]) {
  sets_t s; build_sets(t, b, &s);
  for (int c = 0; c < 2; ++c) {
    const int e = 1 - c;
    for (int p = 0; p < PAIRS; ++p) out[c][p] = count_bits(s.by[c][PAIR_A[p]] & s.pieces[e][PAIR_V[p]]);
    const bb_t undefended = ~s.all[e];
    for (int v = 0; v <= 4; ++v) out[c][11 + v] = count_bits(undefended & s.nonking[c] & s.pieces[e][v]);
    for (int v = 0; v <= 3; ++v) out[c][16 + v] = count_bits(undefended & s.by[c][K] & ~s.nonking[c] & s.pieces[e][v]);
    const bb_t empty = ~b->occupancies[BOTH];
    const bb_t one = single_push(c, s.pieces[c][P], empty);
    const bb_t safe = safe_target(s, c, 2);
    const bb_t targets = (one & safe) | (double_push(c, one & ~s.by[e][P], empty) & safe);
    for (int v = N; v <= Q; ++v) out[c][19 + v] = count_bits(pawn_attacks_from(c, targets) & s.pieces[e][v] & ~s.by[c][P]);
  }
}
int main(int argc, char** argv) {
  auto game = std::make_unique<game_t>();
  initialize_game_const_data(game.get());
  const bb_tables_t* t = game_tables();
  // the corpus
  std::ifstream in(argv[1]);
  std::string line;
  uint64_t rows = 0, total[3] = {}, inzone[3] = {}, undefended_zone[2] = {};
  while (std::getline(in, line)) {
    if (!load_FEN(line.substr(0, line.find('\t')), game.get())) return 1;
    rows++;
    const board_t* b = &game->board;
    sets_t s; build_sets(t, b, &s);
    for (int c = 0; c < 2; ++c) {
      const int e = 1 - c;
      const bb_t zone = s.by[e][K] | s.pieces[e][K];
      for (int p = 0; p < 9; ++p) {
        const bb_t hit = s.by[c][PAIR_A[p]] & s.pieces[e][PAIR_V[p]];
        total[0] += count_bits(hit); inzone[0] += count_bits(hit & zone);
      }
      for (int v = P; v <= Q; ++v) {
        const bb_t hit = s.nonking[c] & ~s.all[e] & s.pieces[e][v];
        total[1] += count_bits(hit); inzone[1] += count_bits(hit & zone);
      }
      const bb_t empty = ~b->occupancies[BOTH];
      const bb_t one = single_push(c, s.pieces[c][P], empty);
      const bb_t safe = safe_target(s, c, 2);
      const bb_t targets = (one & safe) | (double_push(c, one & ~s.by[e][P], empty) & safe);
      const bb_t hit = pawn_attacks_from(c, targets) & ~s.by[c][P] & (s.pieces[e][N]|s.pieces[e][B]|s.pieces[e][R]|s.pieces[e][Q]);
      total[2] += count_bits(hit); inzone[2] += count_bits(hit & zone);
    }
  }
  const char* kinds[3] = {"lesser9", "hanging PNBRQ", "push"};
  printf("rows %llu\n", (unsigned long long)rows);
  for (int k = 0; k < 3; ++k) printf("%-14s victims %llu, in their own king's zone %.2f %%\n", kinds[k], (unsigned long long)total[k], 100.0 * inzone[k] / total[k]);
  // the fixture
  const auto& list = test_eval_positions::positions;
  int pos[24] = {}, neg[24] = {}, n = 0;
  for (const auto& fen : list) {
    if (!load_FEN(fen, game.get())) continue;
    n++;
    int out[2][24]; cells(t, &game->board, out);
    for (int i = 0; i < 24; ++i) { int d = out[0][i] - out[1][i]; if (d > 0) pos[i]++; if (d < 0) neg[i]++; }
  }
  printf("\nfixture rows %d: cell, rows with White ahead, rows with Black ahead\n", n);
  int open = 0;
  for (int i = 0; i < 24; ++i) { printf("  %-8s %3d %3d\n", NAMES[i], pos[i], neg[i]); if (!pos[i] || !neg[i]) open++; }
  printf("cells not reached both ways: %d of 24\n", open);
  return 0;
}
