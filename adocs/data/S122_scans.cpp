// S122's scans, DEC-344's Context and (4), from the fast check's readings (.tuning/coord/
// S122/fastcheck/an1..an7): flat weights over labels, no fit, no match, no timing. A delta is
// the change in mean (result - sigma(K score))^2 against head at the margin 184, in 1e-6,
// negative better. A +/- is a jackknife over 40 groups of whole games. T is
// (w u)^2 / 4096 per side, Black's king's less White's, so White relative, on the mg half
// alone (T x phase / 24), real valued and untruncated, added to stage two before the clamp.
// The w here is that table's unit, which is not the w of a T with no taper.
//
// The score is the engine's own: white_eval = cheap + clamp(two, +/-184), so
//   score(T, M) = white_eval - clamp(two, +/-184) + clamp(two + T, +/-M).
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -I adocs/data adocs/data/S122_scans.cpp -lpthread -o scans
//   scans <dump.bin> --k K [--run-a N] [--quick] [--cross c,n,b,r,q,inc]
//
// --run-a: the first N rows came from another datagen run, whose game numbers repeat. --quick:
// no grid and no margin table. --cross: read that P1 grid point here as well.
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include "S122_scan_rec.hpp"

namespace
{

constexpr double LN = 2.302585092994046 / 400.0;
constexpr int GROUPS = 40;
constexpr unsigned THREADS = 12;

double K = 0.7119;
const scan_rec_t* R = nullptr;
size_t N = 0;
size_t RUN_A = 0;

inline double sig(double s, double k) { return 1.0 / (1.0 + std::exp(-k * s * LN)); }
inline double sig(double s) { return sig(s, K); }
double logit_cp(double p)
{
  p = std::min(std::max(p, 1e-6), 1 - 1e-6);
  return std::log(p / (1 - p)) / (K * LN);
}
inline int group_of(size_t i)
{
  const uint64_t g = static_cast<uint64_t>(R[i].game) * 2654435761u + (i < RUN_A ? 7 : 13);
  return static_cast<int>((g >> 7) % GROUPS);
}
inline double score_of(const scan_rec_t& x, double T, double M)
{
  const double two = x.two_white;
  return x.white_eval - std::clamp(two, -184.0, 184.0) + std::clamp(two + T, -M, M);
}

struct reading_t
{
  double est;
  double se;
};

// The mean of per-row values and its jackknife over whole games.
reading_t jack(const std::vector<double>& sum, const std::vector<double>& cnt)
{
  double S = 0, C = 0;
  for (int g = 0; g < GROUPS; ++g) {
    S += sum[g];
    C += cnt[g];
  }
  double m = 0;
  std::vector<double> leave(GROUPS);
  for (int g = 0; g < GROUPS; ++g) {
    leave[g] = (S - sum[g]) / (C - cnt[g]);
    m += leave[g];
  }
  m /= GROUPS;
  double v = 0;
  for (int g = 0; g < GROUPS; ++g) { v += (leave[g] - m) * (leave[g] - m); }
  return {S / C, std::sqrt(v * (GROUPS - 1.0) / GROUPS)};
}

// (Ta at Ma) against (Tb at Mb), per row, in 1e-6.
template <typename Ta, typename Tb>
reading_t delta(double Ma, Ta ta, double Mb, Tb tb)
{
  std::vector<std::vector<double>> sum(THREADS, std::vector<double>(GROUPS, 0.0));
  std::vector<std::vector<double>> cnt(THREADS, std::vector<double>(GROUPS, 0.0));
  std::vector<std::thread> workers;
  for (unsigned t = 0; t < THREADS; ++t) {
    workers.emplace_back([&, t] {
      for (size_t i = t; i < N; i += THREADS) {
        const scan_rec_t& x = R[i];
        const double ea = x.result - sig(score_of(x, ta(x), Ma));
        const double eb = x.result - sig(score_of(x, tb(x), Mb));
        const int g = group_of(i);
        sum[t][g] += ea * ea - eb * eb;
        cnt[t][g] += 1.0;
      }
    });
  }
  for (auto& w : workers) { w.join(); }
  std::vector<double> s(GROUPS, 0.0), c(GROUPS, 0.0);
  for (unsigned t = 0; t < THREADS; ++t) {
    for (int g = 0; g < GROUPS; ++g) {
      s[g] += sum[t][g];
      c[g] += cnt[t][g];
    }
  }
  const reading_t r = jack(s, c);
  return {r.est * 1e6, r.se * 1e6};
}

inline double no_t(const scan_rec_t&) { return 0.0; }

template <typename Ta>
reading_t against_head(double M, Ta ta) { return delta(M, ta, 184.0, no_t); }

double mg(const scan_rec_t& x) { return x.phase / 24.0; }
double A_of(double w) { return w * w / 4096.0; }

auto t_units(double w)
{
  const double A = A_of(w);
  return [A](const scan_rec_t& x) {
    return A * (static_cast<double>(x.u[1]) * x.u[1] - static_cast<double>(x.u[0]) * x.u[0]) * mg(x);
  };
}
auto t_p1(double w)
{
  const double A = A_of(w);
  return [A](const scan_rec_t& x) {
    const double b = static_cast<double>(x.att[1]) + x.inc[1];
    const double a = static_cast<double>(x.att[0]) + x.inc[0];
    return A * (b * b - a * a) * mg(x);
  };
}
// a a safe check, reading 2 men included, b a weak square, the mg half alone.
auto t_linear(double a, double b)
{
  return [a, b](const scan_rec_t& x) {
    return (a * (static_cast<double>(x.chk[1]) - x.chk[0]) +
            b * (static_cast<double>(x.weak[1]) - x.weak[0])) * mg(x);
  };
}
auto t_linear_empty(double a)
{
  return [a](const scan_rec_t& x) {
    return a * (static_cast<double>(x.chke[1]) - x.chke[0]) * mg(x);
  };
}
auto t_p1_over_linear(double w, double a, double b)
{
  auto p = t_p1(w);
  auto l = t_linear(a, b);
  return [p, l](const scan_rec_t& x) { return p(x) + l(x); };
}
struct p1_point_t
{
  double c, n, b, r, q, inc;
};
auto t_p1_typed(p1_point_t p)
{
  return [p](const scan_rec_t& x) {
    const double b = p.c + p.n * x.an[1] + p.b * x.ab[1] + p.r * x.ar[1] + p.q * x.aq[1] + p.inc * x.inc[1];
    const double w = p.c + p.n * x.an[0] + p.b * x.ab[0] + p.r * x.ar[0] + p.q * x.aq[0] + p.inc * x.inc[0];
    return (b * b - w * w) / 4096.0 * mg(x);
  };
}

// A cell of the label gap, per group, so that its +/- is clustered as the deltas' are.
struct gap_t
{
  double n[GROUPS] = {}, r[GROUPS] = {}, p[GROUPS] = {};
  void add(int g, double rr, double pp)
  {
    n[g] += 1;
    r[g] += rr;
    p[g] += pp;
  }
  double rows() const
  {
    double s = 0;
    for (double v : n) { s += v; }
    return s;
  }
  reading_t read() const
  {
    double N_ = 0, R_ = 0, P_ = 0;
    for (int g = 0; g < GROUPS; ++g) {
      N_ += n[g];
      R_ += r[g];
      P_ += p[g];
    }
    if (N_ == 0) { return {0, 0}; }
    const double est = logit_cp(R_ / N_) - logit_cp(P_ / N_);
    double m = 0;
    double leave[GROUPS];
    for (int g = 0; g < GROUPS; ++g) {
      const double nn = N_ - n[g];
      leave[g] = logit_cp((R_ - r[g]) / nn) - logit_cp((P_ - p[g]) / nn);
      m += leave[g];
    }
    m /= GROUPS;
    double v = 0;
    for (int g = 0; g < GROUPS; ++g) { v += (leave[g] - m) * (leave[g] - m); }
    return {est, std::sqrt(v * (GROUPS - 1.0) / GROUPS)};
  }
};

const std::vector<int> BINS = {0, 1, 3, 5, 7, 9, 11, 14, 18, 23, 30, 1000};
const std::vector<double> MID = {0, 1.5, 3.5, 5.5, 7.5, 9.5, 12, 15.5, 20, 0, 0, 0};

int bin_of(int ad)
{
  int b = 0;
  while (b + 1 < static_cast<int>(BINS.size()) && ad >= BINS[b + 1]) { b++; }
  return b;
}

}  // namespace

int main(int argc, char** argv)
{
  if (argc < 4) {
    fprintf(stderr, "usage: scans <dump.bin> --k K [--run-a N] [--quick] [--cross c,n,b,r,q,inc]\n");
    return 1;
  }
  bool quick = false;
  bool has_cross = false;
  p1_point_t cross{};
  for (int i = 2; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--k") {
      K = atof(argv[++i]);
    } else if (a == "--run-a") {
      RUN_A = strtoull(argv[++i], nullptr, 10);
    } else if (a == "--quick") {
      quick = true;
    } else if (a == "--cross") {
      has_cross = sscanf(argv[++i], "%lf,%lf,%lf,%lf,%lf,%lf", &cross.c, &cross.n, &cross.b, &cross.r, &cross.q, &cross.inc) == 6;
      if (!has_cross) {
        fprintf(stderr, "unreadable --cross\n");
        return 1;
      }
    } else {
      fprintf(stderr, "unknown option %s\n", a.c_str());
      return 1;
    }
  }

  const int fd = open(argv[1], O_RDONLY);
  struct stat st;
  if (fd < 0 || fstat(fd, &st) != 0) {
    fprintf(stderr, "cannot read %s\n", argv[1]);
    return 1;
  }
  N = st.st_size / sizeof(scan_rec_t);
  R = static_cast<const scan_rec_t*>(mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0));

  // The K the labels and head's evaluate() ask for, golden section, beside the K the reading uses.
  double lo = 0.3, hi = 1.3;
  for (int it = 0; it < 25; ++it) {
    const double a = lo + (hi - lo) * 0.382, b = lo + (hi - lo) * 0.618;
    std::vector<double> ea(THREADS, 0.0), eb(THREADS, 0.0);
    std::vector<std::thread> workers;
    for (unsigned t = 0; t < THREADS; ++t) {
      workers.emplace_back([&, t] {
        for (size_t i = t; i < N; i += THREADS) {
          const double s = R[i].white_eval, r = R[i].result;
          double x = r - sig(s, a);
          ea[t] += x * x;
          x = r - sig(s, b);
          eb[t] += x * x;
        }
      });
    }
    for (auto& w : workers) { w.join(); }
    double A = 0, B = 0;
    for (unsigned t = 0; t < THREADS; ++t) {
      A += ea[t];
      B += eb[t];
    }
    if (A < B) { hi = b; } else { lo = a; }
  }
  double base = 0;
  for (size_t i = 0; i < N; ++i) {
    const double e = R[i].result - sig(R[i].white_eval);
    base += e * e;
  }
  std::vector<uint64_t> group_rows(GROUPS, 0);
  for (size_t i = 0; i < N; ++i) { group_rows[group_of(i)]++; }
  printf("rows %zu  K read at %.4f  K the labels ask at head's evaluate %.4f  base MSE %.6f  groups %d, %llu to %llu rows\n", N, K, (lo + hi) / 2, base / N,
         GROUPS, (unsigned long long)*std::min_element(group_rows.begin(), group_rows.end()),
         (unsigned long long)*std::max_element(group_rows.begin(), group_rows.end()));

  // ---- the gap by unit difference, |eval| < 300
  {
    std::vector<gap_t> cell(BINS.size());
    std::vector<std::vector<gap_t>> by_phase(BINS.size(), std::vector<gap_t>(3));
    std::vector<double> d(BINS.size(), 0), q(BINS.size(), 0), qf(BINS.size(), 0), umin(BINS.size(), 0);
    for (size_t i = 0; i < N; ++i) {
      const scan_rec_t& x = R[i];
      if (std::abs(static_cast<int>(x.white_eval)) >= 300) { continue; }
      const int du = static_cast<int>(x.u[1]) - x.u[0];
      const int b = bin_of(std::abs(du));
      const int A = du >= 0 ? 1 : 0, D = 1 - A;
      const double s = sig(x.white_eval);
      const double rr = du >= 0 ? x.result : 1 - x.result;
      const double pp = du >= 0 ? s : 1 - s;
      const int g = group_of(i);
      cell[b].add(g, rr, pp);
      by_phase[b][x.phase >= 16 ? 0 : (x.phase >= 8 ? 1 : 2)].add(g, rr, pp);
      const double ua = x.u[A], ud = x.u[D];
      d[b] += ua - ud;
      umin[b] += ud;
      q[b] += ua * ua - ud * ud;
      qf[b] += (ua * ua - ud * ud) * mg(x);
    }
    printf("\nthe gap by |units W - B|, |eval| < 300, oriented to the side attacking more:\n");
    printf("bin  rows       gap +/-    mean D  E[q]/D^2  w(mid-bin, D^2, no taper)  w(per side, no taper)  w(per side, mg half: the margin tables' w)\n");
    for (size_t b = 1; b < BINS.size(); ++b) {
      const double n = cell[b].rows();
      if (n == 0) { continue; }
      const reading_t r = cell[b].read();
      const double g = std::max(r.est, 0.0), md = d[b] / n;
      printf("%3d+ %8.0f %7.1f %6.1f  %6.2f  %6.2f  %9.1f %22.1f %24.1f\n", BINS[b], n, r.est, r.se, md,
             (q[b] / n) / std::max(md * md, 1e-9), MID[b] > 0 ? std::sqrt(g * 4096) / MID[b] : 0.0,
             q[b] > 0 ? std::sqrt(g * 4096 / (q[b] / n)) : 0.0, qf[b] > 0 ? std::sqrt(g * 4096 / (qf[b] / n)) : 0.0);
    }
    printf("\nthe same by phase: rows gap +/- at phase >= 16, 8-15, < 8\n");
    for (size_t b = 1; b < 9; ++b) {
      printf("%3d+", BINS[b]);
      for (int c = 0; c < 3; ++c) {
        const reading_t r = by_phase[b][c].read();
        printf("  %8.0f %7.1f %6.1f", by_phase[b][c].rows(), r.est, r.se);
      }
      printf("\n");
    }
  }

  // ---- held equal
  {
    std::vector<gap_t> eq(16), chk_eq(16), any(16);
    for (size_t i = 0; i < N; ++i) {
      const scan_rec_t& x = R[i];
      if (std::abs(static_cast<int>(x.white_eval)) >= 300) { continue; }
      const int d = static_cast<int>(x.att[1]) + x.inc[1] - x.att[0] - x.inc[0];
      const double s = sig(x.white_eval);
      const double rr = d >= 0 ? x.result : 1 - x.result;
      const double pp = d >= 0 ? s : 1 - s;
      const int a = std::min(std::abs(d), 15);
      const int g = group_of(i);
      any[a].add(g, rr, pp);
      if (x.chk[0] == x.chk[1]) {
        chk_eq[a].add(g, rr, pp);
        if (x.weak[0] == x.weak[1]) { eq[a].add(g, rr, pp); }
      }
    }
    const auto line = [](const char* name, const std::vector<gap_t>& c) {
      printf("%-30s", name);
      for (int a = 1; a < 13; ++a) {
        if (c[a].rows() < 30) { continue; }
        const reading_t r = c[a].read();
        printf("  %d: %.0f %.1f +/-%.1f", a, c[a].rows(), r.est, r.se);
      }
      printf("\n");
    };
    printf("\nthe gap by |attackers + incidences, W - B|, |eval| < 300: difference: rows gap\n");
    line("checks and weak held equal", eq);
    line("checks held equal", chk_eq);
    line("nothing held", any);

    std::vector<gap_t> c_eq(8), c_one(8), ce_eq(8), ce_one(8);
    for (size_t i = 0; i < N; ++i) {
      const scan_rec_t& x = R[i];
      if (std::abs(static_cast<int>(x.white_eval)) >= 300) { continue; }
      const double s = sig(x.white_eval);
      const int g = group_of(i);
      const int ai = static_cast<int>(x.att[1]) + x.inc[1] - x.att[0] - x.inc[0];
      const int w = static_cast<int>(x.weak[1]) - x.weak[0];
      const int d = static_cast<int>(x.chk[1]) - x.chk[0];
      const int de = static_cast<int>(x.chke[1]) - x.chke[0];
      const auto put = [&](int dd, std::vector<gap_t>& one, std::vector<gap_t>& held) {
        const double rr = dd >= 0 ? x.result : 1 - x.result;
        const double pp = dd >= 0 ? s : 1 - s;
        const int a = std::min(std::abs(dd), 7);
        one[a].add(g, rr, pp);
        if (ai == 0 && w == 0) { held[a].add(g, rr, pp); }
      };
      put(d, c_one, c_eq);
      put(de, ce_one, ce_eq);
    }
    const auto line2 = [](const char* name, const std::vector<gap_t>& c) {
      printf("%-44s", name);
      for (int a = 1; a < 8; ++a) {
        if (c[a].rows() < 30) { continue; }
        const reading_t r = c[a].read();
        printf("  %d: %.0f %.1f +/-%.1f", a, c[a].rows(), r.est, r.se);
      }
      printf("\n");
    };
    printf("\nthe gap by |safe checks, W - B|, |eval| < 300: difference: rows gap\n");
    line2("one way, men included", c_one);
    line2("one way, empty targets", ce_one);
    line2("attackers + incidences and weak equal", c_eq);
    line2("the same, empty targets", ce_eq);
  }

  // ---- DEC-344's Context table
  printf("\nDEC-344's table, delta MSE 1e-6 against head at M 184, +/- by whole games:\n");
  {
    const reading_t l20 = against_head(184, t_linear(20, 0));
    printf("one linear safe-check cell, a = 20, men included, mg half   %8.1f +/- %.1f\n", l20.est, l20.se);
    const reading_t e20 = against_head(184, t_linear_empty(20));
    printf("the same, empty targets only, P1's own count (DEC-344 (3))  %8.1f +/- %.1f\n", e20.est, e20.se);
    printf("one cell by a, men included:");
    for (double a : {10., 15., 20., 25., 30., 40.}) {
      const reading_t r = against_head(184, t_linear(a, 0));
      printf("  a%.0f %.1f +/-%.1f", a, r.est, r.se);
    }
    printf("\none cell by a, empty targets:");
    for (double a : {10., 15., 20., 25., 30., 40.}) {
      const reading_t r = against_head(184, t_linear_empty(a));
      printf("  a%.0f %.1f +/-%.1f", a, r.est, r.se);
    }
    printf("\n\nT over all units, by flat w (best on this grid marked):\n");
    double best = 1e9;
    int best_w = 0;
    reading_t best_r{};
    const std::vector<int> ws = {16, 24, 28, 32, 36, 40, 48, 56, 64, 72};
    std::vector<reading_t> at184, at250;
    for (int w : ws) {
      at184.push_back(against_head(184, t_units(w)));
      at250.push_back(against_head(250, t_units(w)));
      if (at184.back().est < best) {
        best = at184.back().est;
        best_w = w;
        best_r = at184.back();
      }
    }
    for (size_t i = 0; i < ws.size(); ++i) {
      printf("  w%-3d M184 %8.1f +/- %5.1f   M250 %8.1f +/- %5.1f\n", ws[i], at184[i].est, at184[i].se, at250[i].est, at250[i].se);
    }
    printf("T over all units, best flat w: %.1f +/- %.1f at w %d\n", best_r.est, best_r.se, best_w);
    printf("\nT over attackers and incidences alone, by flat w:\n");
    for (int w : {16, 24, 32, 48, 56, 64}) {
      const reading_t r = against_head(184, t_p1(w));
      printf("  w%-3d M184 %8.1f +/- %5.1f\n", w, r.est, r.se);
    }
    printf("\nthe margin alone, T = 0:\n");
    for (double M : {150., 200., 225., 250., 275., 300., 400.}) {
      const reading_t r = against_head(M, no_t);
      printf("  M%-4.0f %8.1f +/- %5.1f\n", M, r.est, r.se);
    }
  }

  // ---- the curve over the linear cells
  printf("\nP1's curve over linear checks and weak squares, mg half, M 184. A cell is delta against head; d is against w 0 in its column, paired by game:\n");
  {
    const std::vector<std::pair<double, double>> cols = {{20, 0}, {20, 4}, {25, 6}, {30, 4}, {30, 8}, {40, 4}};
    printf("       ");
    for (const auto& c : cols) { printf("   a%.0f b%.0f, d +/-       ", c.first, c.second); }
    printf("\n");
    for (double w : {0., 16., 24., 32., 40., 48., 56., 64.}) {
      printf(" w%-4.0f", w);
      for (const auto& c : cols) {
        const reading_t v = against_head(184, t_p1_over_linear(w, c.first, c.second));
        const reading_t dd = delta(184, t_p1_over_linear(w, c.first, c.second), 184, t_linear(c.first, c.second));
        printf("  %7.1f %6.1f %4.1f  ", v.est, dd.est, dd.se);
      }
      printf("\n");
    }
    // DEC-344's Context quotes two of those cells, one a dataset. Both, on this dataset.
    for (const auto& cell : std::vector<std::array<double, 3>>{{30, 4, 32}, {20, 4, 24}}) {
      const reading_t dd = delta(184, t_p1_over_linear(cell[2], cell[0], cell[1]), 184, t_linear(cell[0], cell[1]));
      printf("P1's curve at a %.0f b %.0f w %.0f, d against w 0: %.1f +/- %.1f\n", cell[0], cell[1], cell[2], dd.est, dd.se);
    }
    // Each dataset's own best linear cell on the grid, and the curve over it.
    double best = 1e9, ba = 0, bb = 0;
    for (double a = 0; a <= 50; a += 5) {
      for (double b = 0; b <= 10; b += 2) {
        const reading_t r = against_head(184, t_linear(a, b));
        if (r.est < best) {
          best = r.est;
          ba = a;
          bb = b;
        }
      }
    }
    printf("this dataset's best linear cell on the grid: a %.0f b %.0f, %.1f; the curve over it, d against w 0:", ba, bb, best);
    for (double w : {8., 16., 24., 32., 40., 48.}) {
      const reading_t dd = delta(184, t_p1_over_linear(w, ba, bb), 184, t_linear(ba, bb));
      printf("  w%.0f %.1f +/-%.1f", w, dd.est, dd.se);
    }
    printf("\n");
  }

  // ---- P1's own grid
  if (!quick) {
    double best = 1e9;
    p1_point_t bp{};
    for (double c : {0., 64., 256.}) {
      for (double wn : {0., 32., 64., 128.}) {
        for (double wb : {0., 32., 64., 128.}) {
          for (double wr : {0., 32., 64., 128.}) {
            for (double wq : {0., 64., 128., 192., 256.}) {
              for (double wi : {0., 16., 32., 48., 64.}) {
                const p1_point_t p{c, wn, wb, wr, wq, wi};
                auto tp = t_p1_typed(p);
                const double d = delta(184, tp, 184, no_t).est;
                if (d < best) {
                  best = d;
                  bp = p;
                }
              }
            }
          }
        }
      }
    }
    const reading_t r = against_head(184, t_p1_typed(bp));
    printf("\nP1 alone, best on its grid, M 184, mg half: %.1f +/- %.1f at c %.0f N %.0f B %.0f R %.0f Q %.0f inc %.0f\n", r.est, r.se, bp.c, bp.n, bp.b, bp.r, bp.q, bp.inc);
    printf("grid-best-point %.0f,%.0f,%.0f,%.0f,%.0f,%.0f\n", bp.c, bp.n, bp.b, bp.r, bp.q, bp.inc);
  }
  if (has_cross) {
    const reading_t r = against_head(184, t_p1_typed(cross));
    printf("\nanother dataset's best P1 grid point (c %.0f N %.0f B %.0f R %.0f Q %.0f inc %.0f) read here, M 184: %.1f +/- %.1f\n", cross.c, cross.n, cross.b, cross.r, cross.q, cross.inc, r.est, r.se);
  }

  // ---- seed gradients, DEC-344's seed: 2c/d = 90/2048 a count, both halves
  {
    const char* name[] = {"attN", "attB", "attR", "attQ", "inc", "weak", "defsq", "chkN", "chkB", "chkR", "chkQ"};
    constexpr int INPUTS = 11;
    std::vector<std::vector<double>> gm(INPUTS, std::vector<double>(GROUPS, 0.0)), ge(INPUTS, std::vector<double>(GROUPS, 0.0));
    std::vector<double> cnt(GROUPS, 0.0);
    const double slope = 90.0 / 2048.0;
    for (size_t i = 0; i < N; ++i) {
      const scan_rec_t& x = R[i];
      const double s = sig(x.white_eval);
      const double outer = (s - x.result) * s * (1 - s) * K * LN * 2;
      const double f = mg(x);
      const int d[INPUTS] = {x.an[1] - x.an[0], x.ab[1] - x.ab[0], x.ar[1] - x.ar[0], x.aq[1] - x.aq[0],
                             x.inc[1] - x.inc[0], x.weak[1] - x.weak[0], x.defsq[1] - x.defsq[0],
                             x.cn[1] - x.cn[0], x.cb[1] - x.cb[0], x.cr[1] - x.cr[0], x.cq[1] - x.cq[0]};
      const int g = group_of(i);
      cnt[g] += 1;
      for (int k = 0; k < INPUTS; ++k) {
        gm[k][g] += outer * d[k] * slope * f;
        ge[k][g] += outer * d[k] * slope * (1 - f);
      }
    }
    printf("\nseed gradients at DEC-344's seed, c 45, d 2048, in 1e-7, +/- by whole games; descent moves a weight against the sign, so negative says danger:\n");
    for (int k = 0; k < INPUTS; ++k) {
      const reading_t a = jack(gm[k], cnt), b = jack(ge[k], cnt);
      printf(" %-6s mg %7.3f +/- %5.3f   eg %7.3f +/- %5.3f\n", name[k], a.est * 1e7, a.se * 1e7, b.est * 1e7, b.se * 1e7);
    }
  }

  // ---- the margin table: DEC-344 (4)
  if (!quick) {
    const std::vector<double> Ms = {184, 200, 225, 250, 275, 300, 325, 350, 400};
    double graded = 0, graded_hi = 0;
    for (size_t i = 0; i < N; ++i) {
      if (R[i].graded != 0) {
        graded++;
        if (R[i].phase >= 16) { graded_hi++; }
      }
    }
    printf("\ngraded rows (>= 3 attackers, a queen, a safe check, either king): %.0f, %.3f %% of rows, %.1f %% at phase >= 16\n", graded, 100.0 * graded / N, 100.0 * graded_hi / graded);
    printf("the margin table, flat w, mg half alone, c 0. Mass is sigma(1 - sigma), the gradient's weight less the row's own residual, at the unclamped score; the second mass row at the score the clamp leaves.\n");
    printf("%-22s", "share bound %, at M:");
    for (double M : Ms) { printf(" %6.0f", M); }
    printf("\n");
    for (double w : {0., 24., 28., 32., 36., 40., 48., 56., 64.}) {
      std::vector<double> bound(Ms.size(), 0), gr(Ms.size(), 0), gm_(Ms.size(), 0), gc(Ms.size(), 0);
      double mass = 0;
      std::vector<double> mass_c(Ms.size(), 0);
      auto tu = t_units(w);
      for (size_t i = 0; i < N; ++i) {
        const scan_rec_t& x = R[i];
        const double T = tu(x);
        const double sum = x.two_white + T;
        const bool g = x.graded != 0;
        const double cheap = x.white_eval - std::clamp(static_cast<double>(x.two_white), -184.0, 184.0);
        double s = sig(cheap + sum);
        const double m = s * (1 - s);
        if (g) { mass += m; }
        for (size_t k = 0; k < Ms.size(); ++k) {
          const bool over = std::fabs(sum) > Ms[k];
          if (over) { bound[k]++; }
          if (g) {
            s = sig(cheap + std::clamp(sum, -Ms[k], Ms[k]));
            const double mc = s * (1 - s);
            mass_c[k] += mc;
            if (over) {
              gr[k]++;
              gm_[k] += m;
              gc[k] += mc;
            }
          }
        }
      }
      printf("w %-3.0f all rows %%       ", w);
      for (size_t k = 0; k < Ms.size(); ++k) { printf(" %6.3f", 100 * bound[k] / N); }
      printf("\n      graded rows %%    ");
      for (size_t k = 0; k < Ms.size(); ++k) { printf(" %6.2f", 100 * gr[k] / graded); }
      printf("\n      graded mass %%    ");
      for (size_t k = 0; k < Ms.size(); ++k) { printf(" %6.2f", 100 * gm_[k] / mass); }
      printf("\n      mass, clamped %%  ");
      for (size_t k = 0; k < Ms.size(); ++k) { printf(" %6.2f", 100 * gc[k] / mass_c[k]); }
      printf("\n");
    }
  }
  return 0;
}
