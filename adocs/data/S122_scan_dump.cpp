// S122's scans: writes one scan_rec_t a row of a corpus. FEN, result, score and game columns.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -I adocs/data -I src
//       adocs/data/S122_scan_dump.cpp build/src/libchesso_engine.a -lpthread -o scan_dump
//   scan_dump <corpus.tsv> <out.bin>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include "S122_census_core.hpp"
#include "S122_scan_rec.hpp"

using namespace s122;

int main(int argc, char** argv)
{
  if (argc != 3) {
    fprintf(stderr, "usage: scan_dump <corpus.tsv> <out.bin>\n");
    return 1;
  }

  auto game = std::make_unique<game_t>();
  initialize_game_const_data(game.get());
  const bb_tables_t* t = game_tables();

  std::ifstream in(argv[1]);
  FILE* out = fopen(argv[2], "wb");
  if (!in || out == nullptr) {
    fprintf(stderr, "cannot open %s or %s\n", argv[1], argv[2]);
    return 1;
  }

  std::string line;
  uint64_t rows = 0, mismatches = 0;

  while (std::getline(in, line)) {
    const size_t t1 = line.find('\t'), t2 = line.find('\t', t1 + 1);
    const size_t t3 = line.find('\t', t2 + 1), t4 = line.find('\t', t3 + 1);
    const std::string fen = line.substr(0, t1);

    if (!load_FEN(fen, game.get())) {
      fprintf(stderr, "refused: %s\n", fen.c_str());
      return 1;
    }

    const board_t* b = &game->board;
    const bool white_stm = b->active_color == WHITE;

    scan_rec_t r{};
    r.result = static_cast<float>(strtod(line.c_str() + t1 + 1, nullptr));
    r.score = static_cast<int16_t>(std::atoi(line.c_str() + t2 + 1));
    r.game = std::atoi(line.c_str() + t4 + 1);
    r.stm_white = white_stm;
    r.phase = static_cast<uint8_t>(game_phase(b));

    s101::sets_t s;
    s101::build_sets(t, b, &s);
    ks_counts_t c[2];
    ks_counts(t, b, s, WHITE, &c[WHITE]);
    ks_counts(t, b, s, BLACK, &c[BLACK]);

    int engine[2][KS_FEATURE_COUNT];
    king_safety_features(b, engine);
    for (int side = 0; side < 2; ++side) {
      for (int type = 0; type < 4; ++type) {
        if (engine[side][KS_KNIGHT_ATTACKERS + type] != c[side].attackers[type]) { mismatches++; }
      }
      if (engine[side][KS_ZONE_ATTACKS] != c[side].incidences) { mismatches++; }
    }

    int mob = 0, safety = 0, threats = 0;
    evaluate_expensive_terms(b, &mob, &safety, &threats);
    const int two = mob + safety + threats;
    r.two_white = static_cast<int16_t>(white_stm ? two : -two);
    const int eval = evaluate(b);
    r.white_eval = static_cast<int16_t>(white_stm ? eval : -eval);

    for (int k = 0; k < 2; ++k) {
      const ks_counts_t& d = c[k];
      r.u[k] = static_cast<uint8_t>(ks_units(d));
      r.att[k] = static_cast<uint8_t>(d.attacker_total());
      r.inc[k] = static_cast<uint8_t>(d.incidences);
      r.chk[k] = static_cast<uint8_t>(d.checks_total(2));
      r.chke[k] = static_cast<uint8_t>(d.checks_total(2) - d.check_capture);
      r.weak[k] = static_cast<uint8_t>(d.weak[0]);
      r.defsq[k] = static_cast<uint8_t>(d.def_sq);
      r.an[k] = static_cast<uint8_t>(d.attackers[0]);
      r.ab[k] = static_cast<uint8_t>(d.attackers[1]);
      r.ar[k] = static_cast<uint8_t>(d.attackers[2]);
      r.aq[k] = static_cast<uint8_t>(d.attackers[3]);
      r.cn[k] = static_cast<uint8_t>(d.checks[2][0]);
      r.cb[k] = static_cast<uint8_t>(d.checks[2][1]);
      r.cr[k] = static_cast<uint8_t>(d.checks[2][2]);
      r.cq[k] = static_cast<uint8_t>(d.checks[2][3]);
      if (d.attacker_total() >= 3 && d.attackers[3] > 0 && d.checks_total(2) > 0) {
        r.graded |= static_cast<uint8_t>(k == WHITE ? 1 : 2);
      }
    }

    if (fwrite(&r, sizeof r, 1, out) != 1) {
      fprintf(stderr, "write failed\n");
      return 1;
    }
    rows++;
  }

  if (fclose(out) != 0) {
    fprintf(stderr, "close failed\n");
    return 1;
  }
  printf("rows %llu engine-count mismatches %llu record %zu bytes\n", (unsigned long long)rows,
         (unsigned long long)mismatches, sizeof(scan_rec_t));
  return mismatches == 0 ? 0 : 2;
}
