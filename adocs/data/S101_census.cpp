// S101's research pass, 2026-11-01: how often each candidate threat kind occurs in a
// corpus, split by whether the attacker's side is on move, how the kinds overlap, and
// how far the stage-two sum sits from the clamp with each kind added at a grid of flat
// hypothetical weights. Reads the FEN field alone: the label, the score and every fit
// are never touched. Its output is adocs/data/S101_census.txt.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -I adocs/data -I src
//       adocs/data/S101_census.cpp build/src/libchesso_engine.a -lpthread -o census
//   census <corpus.tsv> [limit]
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "S101_threat_core.hpp"

using namespace s101;

struct cell_t
{
  uint64_t stm = 0;      // total count, attacker's side on move
  uint64_t other = 0;    // total count, attacker's side not on move
  uint64_t nonzero = 0;  // rows whose White-minus-Black coefficient is not zero
  int worst = 0;
};

static std::map<std::string, cell_t> cells;

static void note(const std::string& name, int white, int black, bool white_stm)
{
  cell_t& c = cells[name];
  (white_stm ? c.stm : c.other) += white;
  (white_stm ? c.other : c.stm) += black;
  const int d = white - black;
  if (d != 0) { c.nonzero++; }
  c.worst = std::max(c.worst, std::abs(d));
}

int main(int argc, char** argv)
{
  const std::string path = argv[1];
  const size_t limit = argc > 2 ? strtoull(argv[2], nullptr, 10) : 0;
  auto game = std::make_unique<game_t>();
  initialize_game_const_data(game.get());
  const bb_tables_t* t = game_tables();

  const int margin = LAZY_EVAL_MARGIN;
  const std::vector<int> weights = {5, 10, 15, 20, 30, 40, 60, 80};
  enum
  {
    G_LESSER,
    G_HANGING,
    G_KING,
    G_PUSH,
    G_ALL,
    G_COUNT
  };
  const char* group_name[G_COUNT] = {"lesser9", "hanging5", "kingonly5", "push",
                                     "all"};
  std::vector<std::vector<uint64_t>> clamp_rows(
      G_COUNT, std::vector<uint64_t>(weights.size(), 0));
  std::vector<std::vector<uint64_t>> sign_flip(
      G_COUNT, std::vector<uint64_t>(weights.size(), 0));
  std::vector<int> abs_hist[G_COUNT];
  std::vector<int> two_hist(4096, 0);

  uint64_t rows = 0, clamped = 0, entries = 0;
  uint64_t near[6] = {0, 0, 0, 0, 0, 0};
  const int near_of[6] = {0, 10, 25, 50, 75, 100};
  std::ifstream in(path);
  std::string line;

  for (auto& h : abs_hist) { h.assign(256, 0); }

  while (std::getline(in, line)) {
    if (limit && rows >= limit) { break; }
    const std::string fen = line.substr(0, line.find('\t'));
    if (!load_FEN(fen, game.get())) {
      fprintf(stderr, "refused: %s\n", fen.c_str());
      return 1;
    }
    rows++;
    const board_t* b = &game->board;
    const bool white_stm = b->active_color == WHITE;
    sets_t s;
    build_sets(t, b, &s);

    int mob = 0, safety = 0, threats = 0;
    evaluate_expensive_terms(b, &mob, &safety, &threats);
    const int stm_two = mob + safety + threats;
    const int two = white_stm ? stm_two : -stm_two;
    two_hist[std::min(std::abs(two), 4095)]++;
    if (std::abs(two) > margin) { clamped++; }
    for (int k = 0; k < 6; ++k) {
      if (std::abs(two) > margin - near_of[k]) { near[k]++; }
    }

    int d[G_COUNT] = {0, 0, 0, 0, 0};
    int nz = 0;

    // The pairs: a victim counted once a pair.
    for (int p = 0; p < PAIRS; ++p) {
      const int w = count_bits(s.by[WHITE][PAIR_A[p]] &
                               s.pieces[BLACK][PAIR_V[p]]);
      const int bl = count_bits(s.by[BLACK][PAIR_A[p]] &
                                s.pieces[WHITE][PAIR_V[p]]);
      note(std::string("pair ") + PAIR_NAME[p], w, bl, white_stm);
      if (p < 9) { d[G_LESSER] += w - bl; }
      if (w != bl) { nz++; }
    }

    // Victims with at least one strictly lesser attacker: what a least-attacker
    // form would count, against the pairs' sum.
    const char* v_name[6] = {"P", "N", "B", "R", "Q", "K"};
    for (int v = N; v <= Q; ++v) {
      int w = 0, bl = 0;
      for (int c = 0; c < 2; ++c) {
        bb_t a = s.by[c][P];
        if (v >= R) { a |= s.by[c][N] | s.by[c][B]; }
        if (v == Q) { a |= s.by[c][R]; }
        (c == WHITE ? w : bl) = count_bits(a & s.pieces[1 - c][v]);
      }
      note(std::string("least ") + v_name[v], w, bl, white_stm);
    }

    // Hanging, by the victim's type, attacked by anything but the king and
    // defended by nothing of its own (its king and pawns included).
    for (int v = P; v <= Q; ++v) {
      int w = 0, bl = 0, wo = 0, blo = 0, wl = 0, bll = 0, wn = 0, bln = 0;
      for (int c = 0; c < 2; ++c) {
        const int e = 1 - c;
        const bb_t undefended = s.pieces[e][v] & ~s.all[e];
        const int h = count_bits(undefended & s.nonking[c]);
        const int ka = count_bits(undefended & s.by[c][K]);
        const int ko = count_bits(undefended & s.by[c][K] & ~s.nonking[c]);
        bb_t a = 0;
        if (v >= N) { a |= s.by[c][P]; }
        if (v >= R) { a |= s.by[c][N] | s.by[c][B]; }
        if (v == Q) { a |= s.by[c][R]; }
        const int hl = count_bits(undefended & a);
        (c == WHITE ? w : bl) = h;
        (c == WHITE ? wo : blo) = ko;
        (c == WHITE ? wl : bll) = hl;
        (c == WHITE ? wn : bln) = ka;
      }
      note(std::string("hanging ") + v_name[v], w, bl, white_stm);
      note(std::string("kinganyy ") + v_name[v], wn, bln, white_stm);
      note(std::string("kingonly ") + v_name[v], wo, blo, white_stm);
      note(std::string("hanging&lesser ") + v_name[v], wl, bll, white_stm);
      d[G_HANGING] += w - bl;
      d[G_KING] += wo - blo;
      if (w != bl) { nz++; }
      if (wo != blo) { nz++; }
    }

    // Pawn push threats, three readings of safe, single only or single and
    // double, victims not attacked by an own pawn already.
    for (int reading = 0; reading < 3; ++reading) {
      for (int both = 0; both < 2; ++both) {
        for (int v = N; v <= Q; ++v) {
          int w = 0, bl = 0, wh = 0, blh = 0;
          for (int c = 0; c < 2; ++c) {
            const int e = 1 - c;
            const bb_t empty = ~b->occupancies[BOTH];
            const bb_t safe = safe_target(s, c, reading);
            // The double push passes the single's square: an enemy pawn
            // attacking that square has the en passant capture.
            const bb_t one = single_push(c, s.pieces[c][P], empty);
            bb_t targets = one & safe;
            if (both) {
              targets |= double_push(c, one & ~s.by[e][P], empty) & safe;
            }
            const bb_t hit = pawn_attacks_from(c, targets) & s.pieces[e][v] &
                             ~s.by[c][P];
            (c == WHITE ? w : bl) = count_bits(hit);
            (c == WHITE ? wh : blh) =
                count_bits(hit & (s.nonking[c] & ~s.all[e]));
          }
          char name[64];
          snprintf(name, sizeof name, "push safe%d %s %s", reading,
                   both ? "both" : "single", v_name[v]);
          note(name, w, bl, white_stm);
          snprintf(name, sizeof name, "push&hanging safe%d %s %s", reading,
                   both ? "both" : "single", v_name[v]);
          note(name, wh, blh, white_stm);
          if (reading == 2 && both == 1) {
            d[G_PUSH] += w - bl;
            if (w != bl) { nz++; }
          }
        }
      }
    }

    entries += nz;
    d[G_ALL] = d[G_LESSER] + d[G_HANGING] + d[G_KING] + d[G_PUSH];

    for (int g = 0; g < G_COUNT; ++g) {
      abs_hist[g][std::min(std::abs(d[g]), 255)]++;
      for (size_t i = 0; i < weights.size(); ++i) {
        const int sum = two + weights[i] * d[g];
        if (std::abs(sum) > margin) { clamp_rows[g][i]++; }
      }
    }
  }

  printf("rows %llu clamped_now %llu (%.3f %%) margin %d\n",
         (unsigned long long)rows, (unsigned long long)clamped,
         100.0 * clamped / rows, margin);
  for (int k = 0; k < 6; ++k) {
    printf("  |stage two| > margin-%d: %llu (%.3f %%)\n", near_of[k],
           (unsigned long long)near[k], 100.0 * near[k] / rows);
  }
  uint64_t seen = 0;
  for (int q : {50, 90, 99, 999}) {
    seen = 0;
    for (int i = 0; i < 4096; ++i) {
      seen += two_hist[i];
      if (seen * 1000 >= rows * (q == 999 ? 999 : q * 10)) {
        printf("  |stage two| p%s %d\n", q == 999 ? "99.9" : std::to_string(q).c_str(),
               i);
        break;
      }
    }
  }
  printf("\nmean non-zero threat cells a row (25 cells): %.3f\n",
         double(entries) / rows);

  printf("\nkind rows_nonzero rows%% mean_per_row_stm mean_per_row_other "
         "share_stm worst|d|\n");
  for (const auto& [name, c] : cells) {
    const double total = double(c.stm + c.other);
    printf("%-34s %9llu %6.2f %8.4f %8.4f %5.1f %3d\n", name.c_str(),
           (unsigned long long)c.nonzero, 100.0 * c.nonzero / rows,
           double(c.stm) / rows, double(c.other) / rows,
           total > 0 ? 100.0 * c.stm / total : 0.0, c.worst);
  }

  printf("\nclamp share with the group added at flat hypothetical weight w "
         "(a reach probe, no weight proposed):\n%-10s", "group");
  for (int w : weights) { printf(" w=%-6d", w); }
  printf("\n");
  for (int g = 0; g < G_COUNT; ++g) {
    printf("%-10s", group_name[g]);
    for (size_t i = 0; i < weights.size(); ++i) {
      printf(" %7.3f", 100.0 * clamp_rows[g][i] / rows);
    }
    printf("\n");
  }

  printf("\n|White-Black count| per row, cumulative: group p50 p90 p99 p99.9 max\n");
  for (int g = 0; g < G_COUNT; ++g) {
    uint64_t sum = 0;
    int p50 = -1, p90 = -1, p99 = -1, p999 = -1, mx = 0;
    for (int i = 0; i < 256; ++i) {
      if (abs_hist[g][i]) { mx = i; }
      sum += abs_hist[g][i];
      if (p50 < 0 && sum * 100 >= rows * 50) { p50 = i; }
      if (p90 < 0 && sum * 100 >= rows * 90) { p90 = i; }
      if (p99 < 0 && sum * 100 >= rows * 99) { p99 = i; }
      if (p999 < 0 && sum * 1000 >= rows * 999) { p999 = i; }
    }
    printf("%-10s %d %d %d %d %d\n", group_name[g], p50, p90, p99, p999, mx);
  }
  return 0;
}
