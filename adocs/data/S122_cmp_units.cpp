// S122's research pass: does the census's ks_units() read what the scored prototype's own
// loop counts? Linked against the identity-plants build of chesso_engine, which exports the
// loop's two counts. Neither check in the tools can see a wrong count, so this is the only
// read of one against the other before phase 1's walker (DEC-344 (5)).
//
//   S122_census.sh plants builds and runs it: cmp_units <corpus.tsv> [limit]
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include "S122_census_core.hpp"

using namespace s122;

int main(int argc, char** argv)
{
  auto game = std::make_unique<game_t>();
  initialize_game_const_data(game.get());
  const bb_tables_t* t = game_tables();
  std::ifstream in(argv[1]);
  const unsigned long long limit = argc > 2 ? strtoull(argv[2], nullptr, 10) : 0;
  std::string line;
  unsigned long long rows = 0, different = 0;

  while (std::getline(in, line) && (limit == 0 || rows < limit)) {
    if (!load_FEN(line.substr(0, line.find('\t')), game.get())) { return 1; }
    const board_t* b = &game->board;
    s101::sets_t s;
    s101::build_sets(t, b, &s);
    ks_counts_t c[2];
    ks_counts(t, b, s, WHITE, &c[WHITE]);
    ks_counts(t, b, s, BLACK, &c[BLACK]);
    int mob = 0, safety = 0, threats = 0;
    evaluate_expensive_terms(b, &mob, &safety, &threats);
    rows++;
    for (int k = 0; k < 2; ++k) {
      if (s122_debug_units[k] != ks_units(c[k])) { different++; }
    }
  }
  printf("rows %llu, sides where the census and the prototype's loop count different units: %llu\n", rows, different);
  return different == 0 ? 0 : 2;
}
