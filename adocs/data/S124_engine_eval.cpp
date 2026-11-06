// S124's phase 1, the fast check's finding: every identity check of the factor
// runs on `chesso_engine_trace`, and the shipped `chesso` links `chesso_engine`.
// This prints `evaluate()` for every row of a corpus, one integer a line, and is
// built twice against one build's two libraries -- once linking
// libchesso_engine.a, once libchesso_engine_trace.a -- so a run of each over the
// same corpus, compared line for line, holds the engine's evaluation to the trace
// build's, which tools/truncation_scan holds to exact_score(). On a build with
// every cell forced off F (-DS124_FORCED), that is the forced identity read on
// the library the engine ships. adocs/data/S124_phase1_plants.sh runs it.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -I src adocs/data/S124_engine_eval.cpp
//       <build>/src/libchesso_engine.a -lpthread -o engine_eval
//   engine_eval <corpus.tsv>
//
// The FEN column alone is read. Exits 1 on a FEN the engine refuses.
#include <cstdio>
#include <fstream>
#include <string>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"

int main(int argc, char** argv)
{
  if (argc != 2) {
    fprintf(stderr, "engine_eval <corpus.tsv>\n");
    return 2;
  }

  std::ifstream in(argv[1]);
  if (!in) {
    fprintf(stderr, "cannot read %s\n", argv[1]);
    return 2;
  }

  game_t game;
  initialize_game_const_data(&game);

  std::string line;
  while (std::getline(in, line)) {
    if (line.empty()) { continue; }

    const std::string fen = line.substr(0, line.find('\t'));
    if (!load_FEN(fen, &game)) {
      fprintf(stderr, "unloadable FEN: %s\n", fen.c_str());
      return 1;
    }

    printf("%d\n", evaluate(&game.board));
  }

  return 0;
}
