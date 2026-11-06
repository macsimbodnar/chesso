// S124's phase 1, 2026-11-04: the cell census as a check (DEC-350 (6)). Over
// a corpus, read in place, every row's engine counts -- both candidate cells
// and mop-up's three counts per side, endgame_family_counts(), the functions
// stage one calls -- against tests/test_walked_counts.hpp's walk, which
// shares no code with the engine, and the trace's record of the same cells.
// Beside the check, the census itself: rows per cell picked by E1's sign at
// the shipped weights, and mop-up's domain, read off the engine's counts once
// they are shown equal to the walk's. The FEN column alone is read; no label,
// no score, no position judged.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I src -I tools
//       -I tests adocs/data/S124_cell_check.cpp
//       build/src/libchesso_engine_trace.a -lpthread -o S124_cell_check
//   S124_cell_check <corpus.tsv> [...]
//
// One block a file; a file with any disagreement prints its first five and
// the program exits 1. The last line is S124-CELLS-OK or S124-CELLS-FAILED.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "test_walked_counts.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

namespace
{

const char* const cell_name[ENDGAME_CELL_COUNT + 1] = {
    "P=0 d<=0",       "P=0 d=1",        "P=0 d>=2",      "P=1 d<=0",
    "P=1 d=1",        "P=1 d>=2",       "P=2 d<=0",      "P=2 d=1",
    "P=2 d>=2",       "OCB_PURE P<=2",  "OCB_PURE P=3-4", "OCB_PURE P>=5",
    "OCB_ROOK",       "none"};

}  // namespace


int main(int argc, char** argv)
{
  if (argc < 2) {
    fprintf(stderr, "S124_cell_check <corpus.tsv> [...]\n");
    return 2;
  }

  game_t game;
  initialize_game_const_data(&game);

  std::vector<double> shipped(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(shipped.data());

  bool ok = true;

  for (int a = 1; a < argc; ++a) {
    std::ifstream in(argv[a]);
    if (!in) {
      fprintf(stderr, "cannot read %s\n", argv[a]);
      return 2;
    }

    uint64_t rows = 0;
    uint64_t walk_differs = 0;
    uint64_t trace_differs = 0;
    uint64_t picked[ENDGAME_CELL_COUNT + 1] = {};
    uint64_t candidate[2][ENDGAME_CELL_COUNT + 1] = {};
    uint64_t mopup_rows = 0;
    uint64_t kbn_rows = 0;
    uint64_t mopup_nonzero[2][MOPUP_FEATURE_COUNT] = {};
    std::string line;

    while (std::getline(in, line)) {
      if (line.empty()) { continue; }

      const std::string fen = line.substr(0, line.find('\t'));
      if (!load_FEN(fen, &game)) {
        fprintf(stderr, "unloadable FEN: %s\n", fen.c_str());
        return 2;
      }

      endgame_family_t engine;
      endgame_family_counts(&game.board, &engine);

      walked_counts::endgame_family_walk_t walk;
      if (!walked_counts::endgame_family_features(fen.substr(0, fen.find(' ')),
                                                  &walk)) {
        fprintf(stderr, "the walk refuses: %s\n", fen.c_str());
        return 2;
      }

      bool same = true;
      for (int side = 0; side < 2; ++side) {
        same = same && engine.cell[side] == walk.cell[side];
        for (int i = 0; i < MOPUP_FEATURE_COUNT; ++i) {
          same = same && engine.mopup[side][i] == walk.mopup[side][i];
        }
      }
      if (!same) {
        if (walk_differs < 5) {
          printf("WALK DIFFERS: engine %d %d, walk %d %d: %s\n", engine.cell[0],
                 engine.cell[1], walk.cell[0], walk.cell[1], fen.c_str());
        }
        walk_differs++;
      }

      tuner_trace::row_trace_t trace;
      std::string why;
      if (!tuner_trace::trace_board(&game.board, &trace, &why)) {
        fprintf(stderr, "untraceable (%s): %s\n", why.c_str(), fen.c_str());
        return 2;
      }

      for (int side = 0; side < 2; ++side) {
        const int recorded = (trace.cells[side] == eval_trace::NO_CELL)
                                 ? ENDGAME_CELL_NONE
                                 : trace.cells[side];
        if (recorded != engine.cell[side]) {
          if (trace_differs < 5) {
            printf("TRACE DIFFERS: side %d, trace %d, engine %d: %s\n", side,
                   recorded, engine.cell[side], fen.c_str());
          }
          trace_differs++;
        }
        candidate[side][engine.cell[side]]++;
      }

      tuner_trace::dataset_t one;
      tuner_trace::append_row(trace, 0.5f, 0, 0, &one);
      const tuner_trace::smooth_t s = tuner_trace::smooth(
          tuner_trace::engine_model(), one, 0, shipped.data());
      picked[(s.cell == eval_trace::NO_CELL) ? ENDGAME_CELL_NONE : s.cell]++;

      bool in_domain = false;
      for (int side = 0; side < 2; ++side) {
        for (int i = 0; i < MOPUP_FEATURE_COUNT; ++i) {
          if (engine.mopup[side][i] != 0) {
            mopup_nonzero[side][i]++;
            in_domain = true;
          }
        }
      }
      // In the domain the kings' count is 14 less a distance of at most 14;
      // a row with every count zero there is two kings 14 apart and the lone
      // one on a centre square, which no board holds, so a non-zero count is
      // the domain.
      if (in_domain) {
        mopup_rows++;
        if (count_bits(game.board.bitboards[W_BISHOP] |
                       game.board.bitboards[B_BISHOP]) == 1 &&
            count_bits(game.board.bitboards[W_KNIGHT] |
                       game.board.bitboards[B_KNIGHT]) == 1 &&
            count_bits(game.board.occupancies[BOTH]) == 4) {
          kbn_rows++;
        }
      }

      rows++;
    }

    printf("\n%s\n", argv[a]);
    printf("rows %llu; the walk differs on %llu, the trace's cells on %llu\n",
           static_cast<unsigned long long>(rows),
           static_cast<unsigned long long>(walk_differs),
           static_cast<unsigned long long>(trace_differs));
    printf("%-16s %10s %8s %12s %12s\n", "cell", "picked", "%", "White's",
           "Black's");
    for (int c = 0; c <= ENDGAME_CELL_COUNT; ++c) {
      printf("%-16s %10llu %8.3f %12llu %12llu\n", cell_name[c],
             static_cast<unsigned long long>(picked[c]),
             100.0 * static_cast<double>(picked[c]) /
                 static_cast<double>(rows ? rows : 1),
             static_cast<unsigned long long>(candidate[0][c]),
             static_cast<unsigned long long>(candidate[1][c]));
    }
    printf("mop-up's domain %llu rows (%.3f %%), of them KBN %llu\n",
           static_cast<unsigned long long>(mopup_rows),
           100.0 * static_cast<double>(mopup_rows) /
               static_cast<double>(rows ? rows : 1),
           static_cast<unsigned long long>(kbn_rows));
    printf("mop-up counts non-zero, White's then Black's: centre %llu %llu, "
           "kings %llu %llu, corner %llu %llu\n",
           static_cast<unsigned long long>(mopup_nonzero[0][0]),
           static_cast<unsigned long long>(mopup_nonzero[1][0]),
           static_cast<unsigned long long>(mopup_nonzero[0][1]),
           static_cast<unsigned long long>(mopup_nonzero[1][1]),
           static_cast<unsigned long long>(mopup_nonzero[0][2]),
           static_cast<unsigned long long>(mopup_nonzero[1][2]));

    if (walk_differs > 0 || trace_differs > 0 || rows == 0) { ok = false; }
  }

  printf("%s\n", ok ? "S124-CELLS-OK" : "S124-CELLS-FAILED");
  return ok ? 0 : 1;
}
