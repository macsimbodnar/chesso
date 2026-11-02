// S122's research pass, 2026-11-02: the king-danger counts on a corpus, per input and
// per attack grade, how far a flat accumulator reaches, what a forced flat weight does
// to the clamp, and (with --k, training corpus only) what the labels say beside the
// shipped evaluation. Counts on the board; no fit, no timing, no chess judgement, and
// "attacking" means only the count thresholds stated below. Its output is
// adocs/data/S122_census.txt.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -I adocs/data -I src
//       adocs/data/S122_census.cpp build/src/libchesso_engine.a -lpthread -o census
//   census <corpus.tsv> [--k K] [--limit N]
//
// Without --k the FEN and the game column alone are read. --k reads the result column
// too, and is passed for S083's training corpus only: S082's fixed set keeps its labels
// unread by anything that chooses a form.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "S122_census_core.hpp"

using namespace s122;

namespace
{

constexpr int MAX_GAME = 1 << 21;

struct games_t
{
  std::vector<bool> seen;
  uint64_t count = 0;
  games_t() : seen(MAX_GAME, false) {}
  void note(int game)
  {
    if (game < 0 || game >= MAX_GAME) {
      fprintf(stderr, "game id %d past the census's table\n", game);
      exit(1);
    }
    if (!seen[game]) {
      seen[game] = true;
      count++;
    }
  }
};

// One input of the accumulator: rows and sides with a non-zero count, split by whether
// the attacker is on move, and the rows that reach each half of the taper.
struct input_t
{
  const char* name;
  uint64_t sides = 0;
  uint64_t sides_on_move = 0;
  uint64_t rows = 0;
  uint64_t mg_rows = 0;
  uint64_t eg_rows = 0;
  uint64_t sum = 0;
  uint64_t sum_on_move = 0;
  int largest = 0;
  games_t mg_games;
  games_t eg_games;
  explicit input_t(const char* n) : name(n) {}
};

// A grade, a yes or no on one side's counts. A row is in the grade when either side's
// king is.
enum grade_t
{
  G_A1,      // at least one attacker
  G_A2,      // at least two
  G_A3,      // at least three
  G_A4,      // at least four
  G_A2Q,     // at least two, a queen among them
  G_A3Q,     // at least three, a queen among them
  G_C1,      // a reading 2 safe check of any type
  G_A2C,     // at least two attackers and a safe check
  G_A2QC,    // at least two, a queen among them, and a safe check
  G_A3QC,    // at least three, a queen, a safe check
  G_A3QCW,   // at least three, a queen, a safe check, two weak squares
  G_COUNT
};

const char* const GRADE_NAME[G_COUNT] = {
    ">=1 attacker", ">=2", ">=3", ">=4", ">=2 with Q", ">=3 with Q",
    "safe check", ">=2 + check", ">=2 Q + check", ">=3 Q + check",
    ">=3 Q + check + 2 weak"};

bool in_grade(const ks_counts_t& c, int g)
{
  const int a = c.attacker_total();
  const bool q = c.attackers[3] > 0;
  const bool chk = c.checks_total(2) > 0;
  switch (g) {
    case G_A1: return a >= 1;
    case G_A2: return a >= 2;
    case G_A3: return a >= 3;
    case G_A4: return a >= 4;
    case G_A2Q: return a >= 2 && q;
    case G_A3Q: return a >= 3 && q;
    case G_C1: return chk;
    case G_A2C: return a >= 2 && chk;
    case G_A2QC: return a >= 2 && q && chk;
    case G_A3QC: return a >= 3 && q && chk;
    case G_A3QCW: return a >= 3 && q && chk && c.weak[0] >= 2;
  }
  return false;
}

struct grade_stat_t
{
  uint64_t rows = 0;
  uint64_t sides = 0;
  uint64_t sides_on_move = 0;
  uint64_t by_phase[3] = {0, 0, 0};  // phase >= 16, 8 to 15, under 8
  uint64_t by_eval[3] = {0, 0, 0};   // |evaluate| under 150, under 400, past
  double mass = 0.0;                 // sigma * (1 - sigma), the gradient's weight
  games_t games;
};

uint64_t percentile_of(const std::vector<uint64_t>& hist, uint64_t total, double p)
{
  uint64_t seen = 0;
  for (size_t i = 0; i < hist.size(); ++i) {
    seen += hist[i];
    if (static_cast<double>(seen) >= p * static_cast<double>(total)) { return i; }
  }
  return hist.size() - 1;
}

void print_hist(const char* name, const std::vector<uint64_t>& hist)
{
  uint64_t total = 0;
  size_t top = 0;
  for (size_t i = 0; i < hist.size(); ++i) {
    total += hist[i];
    if (hist[i]) { top = i; }
  }
  printf("%-24s p50 %3llu p90 %3llu p99 %3llu p99.9 %3llu p99.99 %3llu max %3zu\n", name,
         (unsigned long long)percentile_of(hist, total, 0.5),
         (unsigned long long)percentile_of(hist, total, 0.9),
         (unsigned long long)percentile_of(hist, total, 0.99),
         (unsigned long long)percentile_of(hist, total, 0.999),
         (unsigned long long)percentile_of(hist, total, 0.9999), top);
}

double logit_cp(double p, double k)
{
  p = std::min(std::max(p, 1e-6), 1 - 1e-6);
  return std::log(p / (1 - p)) / (k * 2.302585092994046 / 400.0);
}

}  // namespace

int main(int argc, char** argv)
{
  if (argc < 2) {
    fprintf(stderr, "usage: census <corpus.tsv> [--k K] [--limit N]\n");
    return 1;
  }

  const std::string path = argv[1];
  double k = 0.0;
  uint64_t limit = 0;
  for (int i = 2; i + 1 < argc; i += 2) {
    const std::string arg = argv[i];
    if (arg == "--k") {
      k = std::strtod(argv[i + 1], nullptr);
    } else if (arg == "--limit") {
      limit = std::strtoull(argv[i + 1], nullptr, 10);
    } else {
      fprintf(stderr, "unknown option %s\n", arg.c_str());
      return 1;
    }
  }

  auto game = std::make_unique<game_t>();
  initialize_game_const_data(game.get());
  const bb_tables_t* t = game_tables();

  std::vector<std::unique_ptr<input_t>> inputs;
  const auto add_input = [&](const char* name) {
    inputs.push_back(std::make_unique<input_t>(name));
    return inputs.size() - 1;
  };

  add_input("attackers N");
  add_input("attackers B");
  add_input("attackers R");
  add_input("attackers Q");
  add_input("incidences");
  add_input("pawn zone squares");
  add_input("check reach N");
  add_input("check reach B");
  add_input("check reach R");
  add_input("check reach Q");
  add_input("check strict N");
  add_input("check strict B");
  add_input("check strict R");
  add_input("check strict Q");
  add_input("check safe N");
  add_input("check safe B");
  add_input("check safe R");
  add_input("check safe Q");
  add_input("check no-pawn N");
  add_input("check no-pawn B");
  add_input("check no-pawn R");
  add_input("check no-pawn Q");
  add_input("contact check Q");
  add_input("contact check R");
  add_input("weak zone, king only");
  add_input("weak zone, king or Q");
  add_input("zone undefended");
  add_input("defender N");
  add_input("defender B");
  add_input("defended zone squares");
  add_input("shield near");
  add_input("shield far");
  add_input("open file");
  add_input("half-open file");
  add_input("storm d1 free");
  add_input("storm d1 blocked");
  add_input("storm d2 free");
  add_input("storm d2 blocked");
  add_input("storm d3 free");
  add_input("storm d3 blocked");
  add_input("storm d4 free");
  add_input("storm d4 blocked");
  add_input("weak zone, holding a man");
  add_input("safe check on a man");
  add_input("check empty N");
  add_input("check empty B");
  add_input("check empty R");
  add_input("check empty Q");

  std::vector<grade_stat_t> grades(G_COUNT);

  std::vector<uint64_t> h_attackers(32, 0), h_inc(128, 0), h_inc_plus(128, 0),
      h_units(256, 0), h_units_diff(256, 0), h_checks(64, 0), h_weak(16, 0),
      h_def(16, 0), h_attackers_plus(32, 0);

  // Joint table: attackers 0,1,2,3,4+ by safe checks 0,1,2,3+ by weak 0,1,2,3+.
  uint64_t joint[5][4][4] = {};
  uint64_t joint_mid[5][4][4] = {};  // phase at least 8

  // Forced flat weight w on every unit, T = x^2 / DIVISOR on each side, White minus
  // Black, against the shipped stage two. DIVISOR is a gauge; only w^2 / DIVISOR reads.
  const double DIVISOR = 4096.0;
  const std::vector<int> probe_w = {8, 16, 24, 32, 48, 64, 96, 128};
  const std::vector<int> margins = {184, 200, 250, 300, 400, 500, 600, 800};
  std::vector<std::vector<uint64_t>> clamp_mg(
      probe_w.size(), std::vector<uint64_t>(margins.size(), 0));
  std::vector<std::vector<uint64_t>> clamp_both(
      probe_w.size(), std::vector<uint64_t>(margins.size(), 0));
  std::vector<std::vector<uint64_t>> t_hist(probe_w.size(),
                                            std::vector<uint64_t>(4096, 0));

  // The same probe, within a grade: how many of the rows the term is for would the clamp
  // leave blind? Either king in the grade, mg half alone, at w 32, 48 and 64.
  const std::vector<int> grade_probe = {G_A3, G_A3Q, G_A3QC};
  const std::vector<int> grade_w = {32, 48, 64};
  uint64_t grade_rows[3] = {0, 0, 0};
  uint64_t grade_clamp[3][3][8] = {};

  std::vector<uint64_t> two_hist(4096, 0);

  // With --k: the labels beside the shipped score, by the units' White minus Black
  // difference oriented to the side with the larger attack.
  struct residual_t
  {
    uint64_t n = 0;
    double result = 0, predicted = 0;
    uint64_t n_level = 0;
    double result_level = 0, predicted_level = 0;
  };
  const std::vector<int> bin_of = {0, 1, 3, 5, 7, 9, 11, 14, 18, 23, 30, 1000};
  std::vector<residual_t> residual(bin_of.size());
  std::vector<residual_t> residual_wide(bin_of.size());

  // The same labels, one count at a time: the White-minus-Black difference of a single
  // input, oriented to the side with more of it. A one-way reading, every other input
  // left to fall as it does: it says where unexplained value sits and never what an
  // input is worth.
  enum
  {
    W_ATTACKERS,
    W_INCIDENCES,
    W_CHECK_N,
    W_CHECK_B,
    W_CHECK_R,
    W_CHECK_Q,
    W_CHECK_ALL,
    W_WEAK,
    W_DEFENDERS,
    W_DEF_SQUARES,
    W_STORM_NEAR,
    W_COUNT
  };
  const char* const way_name[W_COUNT] = {"attackers", "incidences", "safe checks N",
                                         "safe checks B", "safe checks R", "safe checks Q",
                                         "safe checks", "weak zone", "own minors covering",
                                         "zone squares they cover", "storm d1+d2"};
  constexpr int WAY_BINS = 6;
  residual_t way[W_COUNT][WAY_BINS];
  residual_t way_mover[W_COUNT][2][WAY_BINS];  // [0] the side with more is on move

  // Two-way: the king with the larger units count, by its own shield pawns on the
  // near rank, within a band of the units' difference. Does the shelter multiply?
  residual_t shelter_way[3][4];

  uint64_t rows = 0, clamped = 0, mismatches = 0, stm_white = 0, in_check = 0;
  double total_mass = 0.0;

  std::ifstream in(path);
  if (!in) {
    fprintf(stderr, "cannot read %s\n", path.c_str());
    return 1;
  }

  std::string line;
  while (std::getline(in, line)) {
    if (limit && rows >= limit) { break; }

    // fen result score phase game ply distance
    const size_t tab1 = line.find('\t');
    const std::string fen = line.substr(0, tab1);
    const size_t tab2 = line.find('\t', tab1 + 1);
    const size_t tab3 = line.find('\t', tab2 + 1);
    const size_t tab4 = line.find('\t', tab3 + 1);
    const size_t tab5 = line.find('\t', tab4 + 1);
    const int game_id = std::atoi(line.c_str() + tab4 + 1);
    (void)tab5;

    if (!load_FEN(fen, game.get())) {
      fprintf(stderr, "refused: %s\n", fen.c_str());
      return 1;
    }

    rows++;

    const board_t* b = &game->board;
    const bool white_stm = b->active_color == WHITE;
    if (white_stm) { stm_white++; }
    if (side_to_move_checkers(t, b) != BB_0) { in_check++; }
    const int phase = game_phase(b);
    const double mg_w = phase / 24.0;
    const double eg_w = (24 - phase) / 24.0;

    s101::sets_t s;
    s101::build_sets(t, b, &s);

    ks_counts_t c[2];
    ks_counts(t, b, s, WHITE, &c[WHITE]);
    ks_counts(t, b, s, BLACK, &c[BLACK]);

    // Held to the engine's own count, so the census reads what stage two reads.
    int engine[2][KS_FEATURE_COUNT];
    king_safety_features(b, engine);
    for (int side = 0; side < 2; ++side) {
      for (int type = 0; type < 4; ++type) {
        if (engine[side][KS_KNIGHT_ATTACKERS + type] != c[side].attackers[type]) {
          mismatches++;
        }
      }
      if (engine[side][KS_ZONE_ATTACKS] != c[side].incidences) { mismatches++; }
    }

    int mob = 0, safety = 0, threats = 0;
    evaluate_expensive_terms(b, &mob, &safety, &threats);
    const int stm_two = mob + safety + threats;
    const int two_white = white_stm ? stm_two : -stm_two;
    two_hist[std::min(std::abs(two_white), 4095)]++;
    if (std::abs(two_white) > LAZY_EVAL_MARGIN) { clamped++; }

    const int stm_eval = evaluate(b);
    const int white_eval = white_stm ? stm_eval : -stm_eval;
    const double sig =
        (k > 0.0)
            ? 1.0 / (1.0 + std::exp(-k * white_eval * 2.302585092994046 / 400.0))
            : 0.5;
    const double mass = sig * (1.0 - sig);
    total_mass += mass;

    // The inputs, per side. The attacker's side is on move when K's side is not.
    bool row_seen_input[64] = {};
    for (int side = 0; side < 2; ++side) {
      const ks_counts_t& d = c[side];
      const bool attacker_on_move = (side == WHITE) != white_stm;
      int values[64];
      int n = 0;
      for (int type = 0; type < 4; ++type) { values[n++] = d.attackers[type]; }
      values[n++] = d.incidences;
      values[n++] = d.pawn_inc;
      for (int r = 0; r < READINGS; ++r) {
        for (int type = 0; type < 4; ++type) { values[n++] = d.checks[r][type]; }
      }
      values[n++] = d.contact_q;
      values[n++] = d.contact_r;
      values[n++] = d.weak[0];
      values[n++] = d.weak[1];
      values[n++] = d.weak[2];
      values[n++] = d.def[0];
      values[n++] = d.def[1];
      values[n++] = d.def_sq;
      values[n++] = engine[side][KS_SHIELD_NEAR];
      values[n++] = engine[side][KS_SHIELD_FAR];
      values[n++] = engine[side][KS_OPEN_FILE];
      values[n++] = engine[side][KS_HALF_OPEN_FILE];
      for (int di = 0; di < 4; ++di) {
        for (int bl = 0; bl < 2; ++bl) { values[n++] = d.storm[di][bl]; }
      }
      values[n++] = d.weak_man;
      values[n++] = d.check_capture;
      for (int type = 0; type < 4; ++type) { values[n++] = d.checks_e[type]; }

      for (int i = 0; i < n; ++i) {
        input_t& in_ = *inputs[i];
        const int v = values[i];
        if (v == 0) { continue; }
        in_.sides++;
        in_.sum += v;
        in_.largest = std::max(in_.largest, v);
        if (attacker_on_move) {
          in_.sides_on_move++;
          in_.sum_on_move += v;
        }
        if (!row_seen_input[i]) {
          row_seen_input[i] = true;
          in_.rows++;
        }
        if (phase > 0) {
          in_.mg_rows++;
          in_.mg_games.note(game_id);
        }
        if (phase < 24) {
          in_.eg_rows++;
          in_.eg_games.note(game_id);
        }
      }

      h_attackers[std::min(d.attacker_total(), 31)]++;
      h_inc[std::min(d.incidences, 127)]++;
      int plus = 0;
      for (int type = 0; type < 4; ++type) { plus += d.attackers_plus[type]; }
      h_attackers_plus[std::min(plus, 31)]++;
      h_inc_plus[std::min(d.incidences_plus, 127)]++;
      h_units[std::min(ks_units(d), 255)]++;
      h_checks[std::min(d.checks_total(2), 63)]++;
      h_weak[std::min(d.weak[0], 15)]++;
      h_def[std::min(d.def[0] + d.def[1], 15)]++;

      const int ai = std::min(d.attacker_total(), 4);
      const int ci = std::min(d.checks_total(2), 3);
      const int wi = std::min(d.weak[0], 3);
      joint[ai][ci][wi]++;
      if (phase >= 8) { joint_mid[ai][ci][wi]++; }
    }

    for (int g = 0; g < G_COUNT; ++g) {
      const bool w = in_grade(c[WHITE], g);
      const bool bl = in_grade(c[BLACK], g);
      if (!w && !bl) { continue; }
      grade_stat_t& st = grades[g];
      st.rows++;
      st.games.note(game_id);
      st.mass += mass;
      st.by_phase[phase >= 16 ? 0 : (phase >= 8 ? 1 : 2)]++;
      const int ae = std::abs(white_eval);
      st.by_eval[ae < 150 ? 0 : (ae < 400 ? 1 : 2)]++;
      if (w) {
        st.sides++;
        if (!white_stm) { st.sides_on_move++; }
      }
      if (bl) {
        st.sides++;
        if (white_stm) { st.sides_on_move++; }
      }
    }

    const int uw = ks_units(c[WHITE]);
    const int ub = ks_units(c[BLACK]);
    h_units_diff[std::min(std::abs(uw - ub), 255)]++;

    for (size_t w = 0; w < probe_w.size(); ++w) {
      const double xw = probe_w[w] * static_cast<double>(uw);
      const double xb = probe_w[w] * static_cast<double>(ub);
      const double delta = (xb * xb - xw * xw) / DIVISOR;  // White relative
      const double mg_only = delta * mg_w;
      const double both = delta * (mg_w + eg_w);
      t_hist[w][std::min(static_cast<int>(std::abs(mg_only)), 4095)]++;
      for (size_t m = 0; m < margins.size(); ++m) {
        if (std::abs(two_white + mg_only) > margins[m]) { clamp_mg[w][m]++; }
        if (std::abs(two_white + both) > margins[m]) { clamp_both[w][m]++; }
      }
    }

    for (size_t g = 0; g < grade_probe.size(); ++g) {
      if (!in_grade(c[WHITE], grade_probe[g]) && !in_grade(c[BLACK], grade_probe[g])) {
        continue;
      }
      grade_rows[g]++;
      for (size_t w = 0; w < grade_w.size(); ++w) {
        const double xw = grade_w[w] * static_cast<double>(uw);
        const double xb = grade_w[w] * static_cast<double>(ub);
        const double sum = two_white + (xb * xb - xw * xw) / DIVISOR * mg_w;
        for (size_t m = 0; m < margins.size(); ++m) {
          if (std::abs(sum) > margins[m]) { grade_clamp[g][w][m]++; }
        }
      }
    }

    if (k > 0.0) {
      const double result = std::strtod(line.c_str() + tab1 + 1, nullptr);
      // Oriented to the side attacking more: White's score if Black's king has the
      // larger count.
      const int du = ub - uw;
      const double r = (du >= 0) ? result : 1.0 - result;
      const double p = (du >= 0) ? sig : 1.0 - sig;
      const int ad = std::abs(du);
      size_t bin = 0;
      while (bin + 1 < bin_of.size() && ad >= bin_of[bin + 1]) { bin++; }
      residual[bin].n++;
      residual[bin].result += r;
      residual[bin].predicted += p;
      if (std::abs(white_eval) < 300) {
        residual[bin].n_level++;
        residual[bin].result_level += r;
        residual[bin].predicted_level += p;
      }

      const auto wide_units = [](const ks_counts_t& d) {
        return d.attackers_plus[0] + d.attackers_plus[1] + d.attackers_plus[2] +
               d.attackers_plus[3] + d.incidences_plus + d.checks_total(2) + d.weak[0];
      };
      const int dw = wide_units(c[BLACK]) - wide_units(c[WHITE]);
      const double r_w = (dw >= 0) ? result : 1.0 - result;
      const double p_w = (dw >= 0) ? sig : 1.0 - sig;
      size_t bin_w = 0;
      while (bin_w + 1 < bin_of.size() && std::abs(dw) >= bin_of[bin_w + 1]) { bin_w++; }
      residual_wide[bin_w].n++;
      residual_wide[bin_w].result += r_w;
      residual_wide[bin_w].predicted += p_w;
      if (std::abs(white_eval) < 300) {
        residual_wide[bin_w].n_level++;
        residual_wide[bin_w].result_level += r_w;
        residual_wide[bin_w].predicted_level += p_w;
      }

      const auto way_of = [](const ks_counts_t& d, int w) {
        switch (w) {
          case W_ATTACKERS: return d.attacker_total();
          case W_INCIDENCES: return d.incidences;
          case W_CHECK_N: return d.checks[2][0];
          case W_CHECK_B: return d.checks[2][1];
          case W_CHECK_R: return d.checks[2][2];
          case W_CHECK_Q: return d.checks[2][3];
          case W_CHECK_ALL: return d.checks_total(2);
          case W_WEAK: return d.weak[0];
          case W_DEFENDERS: return d.def[0] + d.def[1];
          case W_DEF_SQUARES: return d.def_sq;
          default: return d.storm[0][0] + d.storm[1][0] + d.storm[1][1];
        }
      };
      if (ad >= 5 && std::abs(white_eval) < 300) {
        const int under = (du >= 0) ? BLACK : WHITE;
        const int shield = std::min(engine[under][KS_SHIELD_NEAR], 3);
        residual_t& cell = shelter_way[ad >= 14 ? 2 : (ad >= 9 ? 1 : 0)][shield];
        cell.n_level++;
        cell.result_level += r;
        cell.predicted_level += p;
      }

      for (int w = 0; w < W_COUNT; ++w) {
        const int dv = way_of(c[BLACK], w) - way_of(c[WHITE], w);
        const double rv = (dv >= 0) ? result : 1.0 - result;
        const double pv = (dv >= 0) ? sig : 1.0 - sig;
        residual_t& cell = way[w][std::min(std::abs(dv), WAY_BINS - 1)];
        cell.n++;
        cell.result += rv;
        cell.predicted += pv;
        if (std::abs(white_eval) < 300) {
          cell.n_level++;
          cell.result_level += rv;
          cell.predicted_level += pv;
          if (dv != 0) {
            const bool more_on_move = (dv > 0) == white_stm;
            residual_t& m = way_mover[w][more_on_move ? 0 : 1][std::min(std::abs(dv), WAY_BINS - 1)];
            m.n_level++;
            m.result_level += rv;
            m.predicted_level += pv;
          }
        }
      }
    }
  }

  printf("rows %llu  white on move %.2f %%  in check %llu  engine-count mismatches %llu\n",
         (unsigned long long)rows, 100.0 * stm_white / rows, (unsigned long long)in_check,
         (unsigned long long)mismatches);
  printf("clamp at the shipped %d: %llu rows, %.3f %%\n", LAZY_EVAL_MARGIN,
         (unsigned long long)clamped, 100.0 * clamped / rows);
  print_hist("|stage two|", two_hist);
  printf("\nper side-row (two a row):\n");
  print_hist("attackers", h_attackers);
  print_hist("attackers, zone+rank", h_attackers_plus);
  print_hist("incidences", h_inc);
  print_hist("incidences, zone+rank", h_inc_plus);
  print_hist("safe checks", h_checks);
  print_hist("weak, king only", h_weak);
  print_hist("own minors covering", h_def);
  print_hist("flat units", h_units);
  print_hist("|units W - B|, a row", h_units_diff);

  printf("\nattackers on one king, share of side-rows:");
  {
    uint64_t total = 0;
    for (const uint64_t v : h_attackers) { total += v; }
    for (int i = 0; i < 8; ++i) {
      printf(" %d:%.3f", i, 100.0 * h_attackers[i] / total);
    }
    printf("\n");
  }

  printf("\ninput                     sides nonzero  mean/side on-move%% rows%%  largest  mg rows/games           eg rows/games\n");
  for (const auto& p : inputs) {
    const input_t& i = *p;
    printf("%-24s %10llu   %7.4f  %6.2f  %6.3f %3d  %10llu %7llu  %10llu %7llu\n", i.name,
           (unsigned long long)i.sides, double(i.sum) / (2.0 * rows),
           i.sum ? 100.0 * i.sum_on_move / i.sum : 0.0, 100.0 * i.rows / rows, i.largest,
           (unsigned long long)i.mg_rows, (unsigned long long)i.mg_games.count,
           (unsigned long long)i.eg_rows, (unsigned long long)i.eg_games.count);
  }

  printf("\ngrade (either king)        rows     rows%%    games   sides on-move%%  phase>=16 8-15 <8    |eval|<150 <400 past   grad mass%%\n");
  for (int g = 0; g < G_COUNT; ++g) {
    const grade_stat_t& st = grades[g];
    printf("%-24s %9llu %7.3f %8llu %6.1f   %9llu %8llu %8llu  %9llu %8llu %8llu  %6.3f\n",
           GRADE_NAME[g], (unsigned long long)st.rows, 100.0 * st.rows / rows,
           (unsigned long long)st.games.count,
           st.sides ? 100.0 * st.sides_on_move / st.sides : 0.0,
           (unsigned long long)st.by_phase[0], (unsigned long long)st.by_phase[1],
           (unsigned long long)st.by_phase[2], (unsigned long long)st.by_eval[0],
           (unsigned long long)st.by_eval[1], (unsigned long long)st.by_eval[2],
           total_mass > 0 ? 100.0 * st.mass / total_mass : 0.0);
  }

  printf("\nside-rows per cell, phase >= 8 in brackets: attackers (row) by safe checks 0,1,2,3+ (col), one block per weak 0,1,2,3+\n");
  for (int w = 0; w < 4; ++w) {
    printf("weak %d\n", w);
    for (int a = 0; a < 5; ++a) {
      printf("  att %d%s", a, a == 4 ? "+" : " ");
      for (int ci = 0; ci < 4; ++ci) {
        printf(" %9llu (%9llu)", (unsigned long long)joint[a][ci][w],
               (unsigned long long)joint_mid[a][ci][w]);
      }
      printf("\n");
    }
  }

  printf("\nforced flat weight w a unit, T = (w u)^2 / 4096, a probe and no seed. |T_w - T_b| * phase/24:\n");
  for (size_t w = 0; w < probe_w.size(); ++w) {
    char name[32];
    snprintf(name, sizeof name, "w=%d, mg half alone", probe_w[w]);
    print_hist(name, t_hist[w]);
  }
  printf("\nclamp share %%, stage two + mg half alone; columns the margins:");
  for (const int m : margins) { printf(" %7d", m); }
  printf("\n");
  for (size_t w = 0; w < probe_w.size(); ++w) {
    printf("w=%-4d                                                       ", probe_w[w]);
    for (size_t m = 0; m < margins.size(); ++m) {
      printf(" %7.3f", 100.0 * clamp_mg[w][m] / rows);
    }
    printf("\n");
  }
  printf("clamp share %%, stage two + both halves alike:\n");
  for (size_t w = 0; w < probe_w.size(); ++w) {
    printf("w=%-4d                                                       ", probe_w[w]);
    for (size_t m = 0; m < margins.size(); ++m) {
      printf(" %7.3f", 100.0 * clamp_both[w][m] / rows);
    }
    printf("\n");
  }

  printf("\nblind rows within a grade: clamp share %% among the grade's rows, mg half alone; columns the margins above\n");
  for (size_t g = 0; g < grade_probe.size(); ++g) {
    for (size_t w = 0; w < grade_w.size(); ++w) {
      printf("%-24s %8llu rows, w=%-3d", GRADE_NAME[grade_probe[g]],
             (unsigned long long)grade_rows[g], grade_w[w]);
      for (size_t m = 0; m < margins.size(); ++m) {
        printf(" %7.3f", grade_rows[g] ? 100.0 * grade_clamp[g][w][m] / grade_rows[g] : 0.0);
      }
      printf("\n");
    }
  }

  if (k > 0.0) {
    printf("\nthe labels beside the shipped score, K %.4f, rows by |units W - B|, oriented to the side attacking more.\n", k);
    printf("gap = the result's logit less the mean prediction's, in centipawns of the sigmoid; a reading and never a fit\n");
    printf("bin  rows       result  predicted  gap-cp    rows|eval|<300  result  predicted  gap-cp\n");
    for (size_t b = 0; b < bin_of.size(); ++b) {
      const residual_t& r = residual[b];
      if (r.n == 0) { continue; }
      const double res = r.result / r.n, pre = r.predicted / r.n;
      const double res_l = r.n_level ? r.result_level / r.n_level : 0.5;
      const double pre_l = r.n_level ? r.predicted_level / r.n_level : 0.5;
      printf("%3d+ %9llu  %.4f  %.4f   %7.1f   %9llu   %.4f  %.4f   %7.1f\n", bin_of[b],
             (unsigned long long)r.n, res, pre, logit_cp(res, k) - logit_cp(pre, k),
             (unsigned long long)r.n_level, res_l, pre_l,
             logit_cp(res_l, k) - logit_cp(pre_l, k));
    }

    printf("\nthe same over the wider zone's counts (the zone plus its forward rank)\n");
    for (size_t b = 0; b < bin_of.size(); ++b) {
      const residual_t& r = residual_wide[b];
      if (r.n == 0) { continue; }
      const double res = r.result / r.n, pre = r.predicted / r.n;
      const double res_l = r.n_level ? r.result_level / r.n_level : 0.5;
      const double pre_l = r.n_level ? r.predicted_level / r.n_level : 0.5;
      printf("%3d+ %9llu  %.4f  %.4f   %7.1f   %9llu   %.4f  %.4f   %7.1f\n", bin_of[b],
             (unsigned long long)r.n, res, pre, logit_cp(res, k) - logit_cp(pre, k),
             (unsigned long long)r.n_level, res_l, pre_l,
             logit_cp(res_l, k) - logit_cp(pre_l, k));
    }

    printf("\none count at a time, |eval| < 300 only, by |difference| 0,1,2,3,4,5+: rows, then the gap in cp\n");
    for (int w = 0; w < W_COUNT; ++w) {
      printf("%-22s", way_name[w]);
      for (int d = 0; d < WAY_BINS; ++d) {
        const residual_t& r = way[w][d];
        if (r.n_level == 0) {
          printf(" %9s %6s", "-", "-");
          continue;
        }
        const double res_l = r.result_level / r.n_level;
        const double pre_l = r.predicted_level / r.n_level;
        printf(" %9llu %6.1f", (unsigned long long)r.n_level,
               logit_cp(res_l, k) - logit_cp(pre_l, k));
      }
      printf("\n");
    }

    printf("\nunits' difference 5-8, 9-13, 14+ (row) by the attacked king's near shield pawns 0,1,2,3 (col), |eval| < 300: rows, gap\n");
    for (int band = 0; band < 3; ++band) {
      printf("  band %d", band);
      for (int s = 0; s < 4; ++s) {
        const residual_t& r = shelter_way[band][s];
        if (r.n_level == 0) {
          printf(" %9s %6s", "-", "-");
          continue;
        }
        const double res_l = r.result_level / r.n_level;
        const double pre_l = r.predicted_level / r.n_level;
        printf(" %9llu %6.1f", (unsigned long long)r.n_level,
               logit_cp(res_l, k) - logit_cp(pre_l, k));
      }
      printf("\n");
    }

    printf("\nthe same split by who is on move: the side with more of the count, then the other. |difference| 1,2,3,4,5+\n");
    for (int w = 0; w < W_COUNT; ++w) {
      for (int who = 0; who < 2; ++who) {
        printf("%-22s %-10s", way_name[w], who == 0 ? "on move" : "not on move");
        for (int d = 1; d < WAY_BINS; ++d) {
          const residual_t& r = way_mover[w][who][d];
          if (r.n_level == 0) {
            printf(" %9s %6s", "-", "-");
            continue;
          }
          const double res_l = r.result_level / r.n_level;
          const double pre_l = r.predicted_level / r.n_level;
          printf(" %9llu %6.1f", (unsigned long long)r.n_level,
                 logit_cp(res_l, k) - logit_cp(pre_l, k));
        }
        printf("\n");
      }
    }
  }

  return mismatches == 0 ? 0 : 2;
}
