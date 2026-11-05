// S266's research pass, 2026-11-05: where the complexity term's features fire on a corpus, by
// phase, how many rows an adjustment of the endgame half could take to zero, how much of the term
// the existing design and S124's thirteen cells already span, and (with --k, a training corpus
// only) what the labels read beside the shipped evaluation for the term, for S124's cells and for
// the two together. Counts on the board and label readings; no fit of the engine, no timing, no
// chess judgement: every feature is a count of pawns, files and men. Read into
// adocs/data/S266_census.txt by adocs/data/S266_census.sh.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I src -I tools
//       adocs/data/S266_census.cpp build/src/libchesso_engine_trace.a -lpthread -o census
//   census <corpus.tsv> [--k K [--refit-k]] [--r2 STRIDE] [--c c0,cP,cF,cE] [--limit N]
//          [--threads T]
//
// Without --k only the FEN and game columns are read. --k reads the result column too and is
// passed for a training corpus only: the held-out views keep their labels unread by anything that
// chooses a form. --refit-k reads every label reading a second time at K refitted at S124's cells:
// F12 fitted at K, K refitted at that fit by golden section on the in-sample MSE, repeated until K
// moves by under 1e-4. The tuner fits K at its start constants, so with S124's cells at their fit
// this is the K S266's screen reads at. --r2 STRIDE takes every STRIDE-th row into the
// collinearity sample. --c counts
// the rows a given C would take to zero, LONE rows left out as the proposal leaves them (the
// training fit's, read on a view: E1 and the features are the engine's, not labels).
//
// Built with -DS266_PLANT the bitboard form counts file d on neither flank, the queenside a to c
// and the kingside e to h; the walk below must then differ, which is what says it can fail
// (adocs/data/S266_census.sh plant).
//
// Every definition is chesso's own and is said to be (DEC-221). Grant 2020, section 1.5, names the
// features ("remaining pawns, whether there are pawns on both flanks, and whether we are in a pawn
// endgame") and the form, E'_eg = E_eg + sign(E_eg) max(-|E_eg|, C); it defines none of them:
//
//   P       the pawns on the board, both colours (popcount of the two pawn bitboards)
//   flank   a pawn of either colour on files a to d and one of either colour on files e to h
//   pe      no knight, bishop, rook or queen on the board: board->phase is 0, kings and pawns
//   C       c0 + cP P + cF flank + cE pe, one endgame weight each, no middlegame half
//   E1      stage one's endgame half with the material in it, White relative: board->material
//           plus the positional site's endgame sum, from the trace at the shipped weights -- the
//           E S124's factor multiplies (DEC-350 (1))
//   E2      stage two's endgame half before the clamp: mobility, threats, king safety and its curve
//   adj     adj(E, C) = E + sign(E) max(-|E|, C): never E's opposite sign, 0 at E = 0, and 0
//           wherever C <= -|E|
//   cell    S124's thirteen, as S124's phase 1 builds them (src/evaluation.cpp endgame_cell() in
//           the chesso-s124p1 worktree, rewritten here from its definition), picked by E1's sign
//
// The label readings, smooth (every taper exact, the clamp kept), White relative, w_eg = (24 -
// phase) / 24, s0 the shipped smooth score:
//   C4      s0 + w_eg (adj(E1, C) - E1), the proposal: the form on E1
//   W4      s0 + w_eg (adj(E1 + E2, C) - E1 - E2): the paper's whole endgame half, the change put
//           outside the clamp (a reading of the form, not a design: E2 is behind the shortcut)
//   R4      C4 with C replaced by min(C, 0): the goal's "toward zero" taken literally
//   F12     s0 + w_eg (r_c - 1) E1: S124's factor as a real ratio in its box [0, 1], twelve free,
//           OCB_PURE P >= 5 tied to P = 3-4 at scale 1 (DEC-350 (2))
//   F12+C4  s0 + w_eg (r_c adj(E1, C) - E1): the paper's order, complexity then the factor
//           (Grant 2020, section 1.3's xi E_eg after section 1.5's adjustment)
//   F12+C4' s0 + w_eg (adj(r_c E1, C) - E1): the other order
//   G1, G6  s0 + w_eg (r - 1) E1 with one unboxed r for every row, or one a phase band: a
//           recalibration of the endgame half with no cell and no feature, the control that says
//           how much of a reading the shipped constants' scale would give to any term on E1
//   F12G6   S124's cells where one is picked, the phase band's unboxed ratio on every other row
//           below phase 24 but a lone king's
//   X | Y   X's complexity weights fitted with Y's ratios held at Y's fit of the same rows
//   C4x, R4x  C4 and R4 with no adjustment on a row where either king stands alone, mop-up's
//           domain, which S124 fits on conversion and not on these labels (DEC-350 (4))
//   W4x, C4x'  C4x on E1 + E2, and C4x in the other order, each over F12's cells held: Q2 and Q3 in
//           the screen's setting
// Each is fitted to the labels by damped Gauss-Newton from the identity (C = 0, r = 1) and, for a
// model with ratios and complexity, also from the ratios' own fit (best_fit()), in sample
// and across halves (the even-numbered games' fit read on the odd ones and the odd ones' on the
// even, summed), dMSE in 1e-6 against s0 with whole-game errors. A reading and never a fit of the
// engine: the engine's other weights stay at the shipped constants.
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "tuner_layout.hpp"
#include "tuner_target.hpp"
#include "tuner_trace.hpp"

namespace
{

constexpr int MAX_GAME = 1 << 21;
constexpr int PHASE_BINS = 7;
// The first six are S102's bands; the seventh is phase 24 alone, where w_eg is 0 and no
// adjustment of the endgame half can move the score.
int phase_bin(int phase)
{
  if (phase >= 24) { return 6; }
  return (phase <= 4) ? 0 : std::min((phase - 1) / 4, 5);
}
const char* phase_name[PHASE_BINS] = {"0-4", "5-8", "9-12", "13-16", "17-20", "21-23", "24"};

constexpr bb_t FILE_A = 0x0101010101010101ULL;
// Square 0 is a8, so a square's file is its index & 7, a = 0.
#ifdef S266_PLANT
constexpr bb_t QUEENSIDE = FILE_A | (FILE_A << 1) | (FILE_A << 2);
#else
constexpr bb_t QUEENSIDE = FILE_A | (FILE_A << 1) | (FILE_A << 2) | (FILE_A << 3);
#endif
constexpr bb_t KINGSIDE = ~(FILE_A | (FILE_A << 1) | (FILE_A << 2) | (FILE_A << 3));

enum input_t
{
  IN_CONST,
  IN_P,
  IN_FLANK,
  IN_PE,
  IN_COUNT
};
const char* input_name[IN_COUNT] = {"c0", "cP", "cF", "cE"};

// S124's cells, numbered as its phase 1 numbers them.
constexpr int CELLS = 13;
constexpr int NO_CELL = 255;
const char* cell_name[CELLS] = {"P=0 d<=0", "P=0 d=1", "P=0 d>=2", "P=1 d<=0", "P=1 d=1",
                                "P=1 d>=2", "P=2 d<=0", "P=2 d=1", "P=2 d>=2", "OCB_PURE P<=2",
                                "OCB_PURE P=3-4", "OCB_PURE P>=5", "OCB_ROOK"};
constexpr int OCB_MID = 10, OCB_HIGH = 11;

bb_t light_squares()
{
  bb_t out = 0;
  for (int sq = 0; sq < 64; ++sq) {
    if (((sq / 8) + (sq % 8)) % 2 == 0) { out |= (bb_t)1 << sq; }
  }
  return out;
}
const bb_t LIGHT = light_squares();

int units(const bb_t* b, int o)
{
  return count_bits(b[o + W_KNIGHT]) + count_bits(b[o + W_BISHOP]) +
         2 * count_bits(b[o + W_ROOK]) + 4 * count_bits(b[o + W_QUEEN]);
}

// The cell if `strong` is the strong side, NO_CELL for none: LONE (either king alone) none; then
// one bishop a side on opposite colours, PURE by the strong side's pawns <= 2, 3-4, >= 5 with no
// knight, rook or queen, ROOK with one rook a side and no knight or queen, any other none; then
// the strong side's pawns 0 to 2 by d, its men in phase units less the other's, <= 0, 1, >= 2.
int endgame_cell(const board_t* board, int strong)
{
  const bb_t* b = board->bitboards;
  const int own = (strong == WHITE) ? 0 : B_PAWN;
  const int other = (strong == WHITE) ? B_PAWN : 0;
  const int pawns = count_bits(b[own + W_PAWN]);
  if (board->occupancies[WHITE] == b[W_KING] || board->occupancies[BLACK] == b[B_KING]) {
    return NO_CELL;
  }
  const bool bishop_each = count_bits(b[W_BISHOP]) == 1 && count_bits(b[B_BISHOP]) == 1;
  const bb_t nq = b[W_KNIGHT] | b[B_KNIGHT] | b[W_QUEEN] | b[B_QUEEN];
  if (bishop_each && ((b[W_BISHOP] & LIGHT) != 0) != ((b[B_BISHOP] & LIGHT) != 0)) {
    if (nq != 0) { return NO_CELL; }
    const bb_t rooks = b[W_ROOK] | b[B_ROOK];
    if (rooks == 0) { return (pawns <= 2) ? 9 : ((pawns <= 4) ? 10 : 11); }
    return (count_bits(b[W_ROOK]) == 1 && count_bits(b[B_ROOK]) == 1) ? 12 : NO_CELL;
  }
  if (pawns > 2) { return NO_CELL; }
  const int d = units(b, own) - units(b, other);
  return 3 * pawns + ((d <= 0) ? 0 : ((d == 1) ? 1 : 2));
}

struct features_t
{
  int p = 0;
  bool flank = false;
  bool pe = false;
};

features_t features(const board_t* board)
{
  const bb_t* b = board->bitboards;
  const bb_t pawns = b[W_PAWN] | b[B_PAWN];
  features_t f;
  f.p = count_bits(pawns);
  f.flank = (pawns & QUEENSIDE) != 0 && (pawns & KINGSIDE) != 0;
  f.pe = board->phase == 0;
  return f;
}

// The same three read square by square from board->squares, the placement and not the bitboards
// or the accumulated phase.
features_t walked(const board_t* board)
{
  features_t f;
  bool queenside = false, kingside = false;
  int pieces = 0;
  for (int sq = 0; sq < 64; ++sq) {
    const int piece = board->squares[sq];
    if (piece == EMPTY) { continue; }
    if (piece == W_PAWN || piece == B_PAWN) {
      f.p++;
      if ((sq & 7) <= 3) {
        queenside = true;
      } else {
        kingside = true;
      }
    } else if (piece != W_KING && piece != B_KING) {
      pieces++;
    }
  }
  f.flank = queenside && kingside;
  f.pe = pieces == 0;
  return f;
}

struct games_t
{
  std::vector<bool> seen;
  uint64_t count = 0;
  games_t() : seen(MAX_GAME, false) {}
  void note(int64_t game)
  {
    if (game < 0 || game >= MAX_GAME) {
      fprintf(stderr, "game id %lld past the census's table\n", (long long)game);
      exit(1);
    }
    if (!seen[game]) {
      seen[game] = true;
      count++;
    }
  }
};

bool split(const std::string& line, std::vector<std::string>* out)
{
  out->clear();
  size_t start = 0;
  while (true) {
    const size_t tab = line.find('\t', start);
    out->push_back(line.substr(start, tab - start));
    if (tab == std::string::npos) { break; }
    start = tab + 1;
  }
  return out->size() >= 5;
}

// The trace's halves at the shipped weights, smooth, as S124's census reads them: the clamped
// groups' transforms added to king safety's halves.
struct halves_t
{
  double material = 0.0;
  double mg[tuner_trace::SITE_COUNT] = {};
  double eg[tuner_trace::SITE_COUNT] = {};
};

halves_t halves_of(const tuner_trace::row_trace_t& t, const std::vector<double>& p)
{
  using namespace tuner_trace;
  halves_t h;
  const std::vector<feature_t>& features = feature_table().features;
  for (const entry_t& e : t.entries) {
    const feature_t& f = features[e.feature];
    if (f.site == SITE_MATERIAL) {
      h.material += e.coef * p[f.mg];
      continue;
    }
    h.mg[f.site] += e.coef * p[f.mg];
    h.eg[f.site] += e.coef * p[f.eg];
  }
  const model_t& model = engine_model();
  for (size_t g = 0; g < model.groups.size(); ++g) {
    const group_t& group = model.groups[g];
    double xm[2] = {0, 0}, xe[2] = {0, 0};
    for (const side_entry_t& e : t.side_entries) {
      if (e.group != g) { continue; }
      xm[e.side] += e.coef * p[group.inputs[e.input].first];
      xe[e.side] += e.coef * p[group.inputs[e.input].second];
    }
    if (!group.clamped) {
      fprintf(stderr, "an unclamped group: the census has no site for it\n");
      exit(1);
    }
    h.mg[SITE_KING_SAFETY] += group.sign * (transform(group.transform, group.divisor, xm[0]) -
                                            transform(group.transform, group.divisor, xm[1]));
    h.eg[SITE_KING_SAFETY] += group.sign * (transform(group.transform, group.divisor, xe[0]) -
                                            transform(group.transform, group.divisor, xe[1]));
  }
  return h;
}

double sgn(double x) { return (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0); }

double adj(double e, double c) { return e + sgn(e) * std::max(-std::fabs(e), c); }

// --------------------------------------------------------------- the stored rows
struct row_t
{
  float s0;      // the shipped smooth score, White relative
  int32_t e1;    // E1 at the shipped weights (integers, so exact)
  float e2;      // stage two's endgame half before the clamp
  uint8_t phase;
  uint8_t p;
  uint8_t flank;
  uint8_t pe;
  uint8_t cell[2];  // the candidate if White is strong, if Black is
  uint8_t label;    // result times 2
  uint8_t lone;     // either king alone: mop-up's side of S124, no cell
  int32_t game;
};

double x_of(const row_t& r, int i)
{
  switch (i) {
    case IN_CONST: return 1.0;
    case IN_P: return r.p;
    case IN_FLANK: return r.flank;
    case IN_PE: return r.pe;
  }
  return 0.0;
}

int picked(const row_t& r)
{
  if (r.e1 > 0) { return r.cell[0]; }
  if (r.e1 < 0) { return r.cell[1]; }
  return NO_CELL;
}

// A model of the readings above. Parameters: the free cells' ratios first (r - 1, so 0 is the
// identity), then the complexity inputs' weights.
enum ratio_t
{
  RATIO_NONE,
  RATIO_CELLS,   // S124's cells, twelve free
  RATIO_GLOBAL,  // one ratio on every row's E1: a recalibration of the endgame half, no cell
  RATIO_PHASE,   // one ratio per phase band below 24: the same by phase
  RATIO_HYBRID   // S124's cells where a cell is picked, the phase band's ratio on every other row
};

struct spec_t
{
  const char* name;
  bool cells = false;  // a ratio of `ratio`'s kind
  ratio_t ratio = RATIO_CELLS;
  std::vector<int> inputs;  // complexity inputs, empty for none
  bool whole = false;       // E1 + E2 in place of E1
  bool restrict_neg = false;
  bool other_order = false;  // adj(r E1, C) in place of r adj(E1, C)
  bool frozen_cells = false;  // the cells held where the start put them, C alone free
  bool skip_lone = false;     // no adjustment where a king stands alone: mop-up's domain
};

// The free cells: every cell but OCB_HIGH, which shares OCB_MID's ratio.
constexpr int FREE_CELLS = 12;
int cell_param(int cell) { return (cell == OCB_HIGH) ? OCB_MID - 1 : ((cell > OCB_HIGH) ? cell - 1 : cell); }

size_t ratios_of(const spec_t& s)
{
  if (!s.cells) { return 0; }
  switch (s.ratio) {
    case RATIO_CELLS: return FREE_CELLS;
    case RATIO_GLOBAL: return 1;
    case RATIO_PHASE: return 6;
    case RATIO_HYBRID: return FREE_CELLS + 6;
    default: return 0;
  }
}

size_t width_of(const spec_t& s) { return ratios_of(s) + s.inputs.size(); }

// The ratio parameter a row reads, -1 for none.
int ratio_param(const spec_t& s, const row_t& r)
{
  if (!s.cells || r.e1 == 0) { return -1; }
  if (s.ratio == RATIO_GLOBAL) { return 0; }
  if (s.ratio == RATIO_PHASE) { return (r.phase >= 24) ? -1 : phase_bin(r.phase); }
  const int cell = picked(r);
  if (s.ratio == RATIO_HYBRID && cell == NO_CELL) {
    return (r.phase >= 24 || r.lone) ? -1 : FREE_CELLS + phase_bin(r.phase);
  }
  return (cell == NO_CELL) ? -1 : cell_param(cell);
}

// Delta (the change to the endgame half, before w_eg) and its derivative in every parameter.
double delta(const spec_t& s, const std::vector<double>& th, const row_t& r, double* d)
{
  const size_t w = width_of(s);
  for (size_t i = 0; i < w; ++i) { d[i] = 0.0; }
  const size_t base = ratios_of(s);
  double c = 0.0;
  for (size_t i = 0; i < s.inputs.size(); ++i) { c += th[base + i] * x_of(r, s.inputs[i]); }
  // Under skip_lone a lone king's row takes no adjustment: C reads 0 there and moves nothing.
  const bool cx_here = !(s.skip_lone && r.lone);
  if (!cx_here) { c = 0.0; }
  // The left derivative at 0 under min(C, 0), so a fit from 0 can leave it downwards.
  double dc = 1.0;
  if (s.restrict_neg) {
    if (c > 0.0) {
      c = 0.0;
      dc = 0.0;
    }
  }
  const double e = s.whole ? double(r.e1) + r.e2 : double(r.e1);
  const int cp = ratio_param(s, r);
  const double ratio = (cp >= 0) ? 1.0 + th[cp] : 1.0;
  if (!s.other_order) {
    const double a = s.inputs.empty() ? e : adj(e, c);
    const bool live = cx_here && std::fabs(e) > 0.0 && c > -std::fabs(e);
    if (cp >= 0) { d[cp] = a; }
    if (live) {
      for (size_t i = 0; i < s.inputs.size(); ++i) {
        d[base + i] = ratio * sgn(e) * dc * x_of(r, s.inputs[i]);
      }
    }
    return ratio * a - e;
  }
  const double g = ratio * e;
  const double a = adj(g, c);
  const bool live = cx_here && std::fabs(g) > 0.0 && c > -std::fabs(g);
  if (cp >= 0) { d[cp] = live ? e : 0.0; }
  if (live) {
    for (size_t i = 0; i < s.inputs.size(); ++i) {
      d[base + i] = sgn(g) * dc * x_of(r, s.inputs[i]);
    }
  }
  return a - e;
}

struct reader_t
{
  const std::vector<row_t>* rows;
  double k;  // K
  unsigned threads;
};

double predict(const reader_t& rd, const row_t& r, double dlt)
{
  const double weg = (24.0 - r.phase) / 24.0;
  return tuner_target::sigmoid(r.s0 + weg * dlt, rd.k);
}

// Which rows a pass reads: 0 all, 1 even-numbered games, 2 odd.
bool in_half(const row_t& r, int half)
{
  return half == 0 || (half == 1 && r.game % 2 == 0) || (half == 2 && r.game % 2 == 1);
}

// One Gauss-Newton system over a half: loss, J^T J, J^T r.
double system(const reader_t& rd, const spec_t& s, const std::vector<double>& th, int half,
              std::vector<double>* jtj, std::vector<double>* jtr)
{
  const size_t w = width_of(s);
  const std::vector<row_t>& rows = *rd.rows;
  std::vector<std::vector<double>> pj(rd.threads, std::vector<double>(w * w, 0.0));
  std::vector<std::vector<double>> pr(rd.threads, std::vector<double>(w, 0.0));
  std::vector<double> pl(rd.threads, 0.0);
  std::vector<std::thread> ws;
  const double kk = rd.k * tuner_target::LN10_OVER_400;
  for (unsigned t = 0; t < rd.threads; ++t) {
    ws.emplace_back([&, t]() {
      std::vector<double> d(w);
      std::vector<double>& J = pj[t];
      std::vector<double>& R = pr[t];
      double loss = 0.0;
      for (size_t i = t; i < rows.size(); i += rd.threads) {
        const row_t& r = rows[i];
        if (!in_half(r, half)) { continue; }
        const double dl = delta(s, th, r, d.data());
        const double weg = (24.0 - r.phase) / 24.0;
        const double sig = tuner_target::sigmoid(r.s0 + weg * dl, rd.k);
        const double res = r.label * 0.5 - sig;
        loss += res * res;
        const double scale = kk * sig * (1.0 - sig) * weg;
        for (size_t a = 0; a < w; ++a) {
          if (d[a] == 0.0) { continue; }
          const double ja = scale * d[a];
          R[a] += ja * res;
          for (size_t b = 0; b < w; ++b) { J[a * w + b] += ja * scale * d[b]; }
        }
      }
      pl[t] = loss;
    });
  }
  for (auto& x : ws) { x.join(); }
  jtj->assign(w * w, 0.0);
  jtr->assign(w, 0.0);
  double loss = 0.0;
  for (unsigned t = 0; t < rd.threads; ++t) {
    for (size_t a = 0; a < w * w; ++a) { (*jtj)[a] += pj[t][a]; }
    for (size_t a = 0; a < w; ++a) { (*jtr)[a] += pr[t][a]; }
    loss += pl[t];
  }
  return loss;
}

bool solve(std::vector<double> m, std::vector<double> v, size_t w, std::vector<double>* out)
{
  // Gaussian elimination with partial pivoting.
  for (size_t c = 0; c < w; ++c) {
    size_t piv = c;
    for (size_t r = c + 1; r < w; ++r) {
      if (std::fabs(m[r * w + c]) > std::fabs(m[piv * w + c])) { piv = r; }
    }
    if (std::fabs(m[piv * w + c]) < 1e-300) { return false; }
    if (piv != c) {
      for (size_t k = 0; k < w; ++k) { std::swap(m[c * w + k], m[piv * w + k]); }
      std::swap(v[c], v[piv]);
    }
    for (size_t r = c + 1; r < w; ++r) {
      const double f = m[r * w + c] / m[c * w + c];
      for (size_t k = c; k < w; ++k) { m[r * w + k] -= f * m[c * w + k]; }
      v[r] -= f * v[c];
    }
  }
  out->assign(w, 0.0);
  for (size_t c = w; c-- > 0;) {
    double sum = v[c];
    for (size_t k = c + 1; k < w; ++k) { sum -= m[c * w + k] * (*out)[k]; }
    (*out)[c] = sum / m[c * w + c];
  }
  return true;
}

struct fit_t
{
  std::vector<double> th;
  double loss = 0.0;
  int iterations = 0;
};

// S124's box: a cell's factor in [0, F], so r in [0, 1] and the parameter r - 1 in [-1, 0]
// (tools/tuner_groups.hpp box_scales() in S124's phase 1). The global and per-band ratios of the
// recalibration readings are not boxed.
bool boxed(const spec_t& s, size_t a)
{
  return s.cells && (s.ratio == RATIO_CELLS || s.ratio == RATIO_HYBRID) && a < size_t(FREE_CELLS);
}

// Levenberg-Marquardt from `start`, the step accepted only where the loss falls; under
// frozen_cells only the complexity weights move. A boxed parameter at its bound whose descent
// points out of the box sits the iteration out, and every trial is clipped to the box.
fit_t fit(const reader_t& rd, const spec_t& s, int half, const std::vector<double>& start)
{
  const size_t w = width_of(s);
  fit_t f;
  f.th = start;
  for (size_t a = 0; a < w; ++a) {
    if (boxed(s, a)) { f.th[a] = std::clamp(f.th[a], -1.0, 0.0); }
  }
  std::vector<double> jtj, jtr, step, trial;
  double loss = system(rd, s, f.th, half, &jtj, &jtr);
  double lambda = 1e-4;
  for (int it = 0; it < 100; ++it) {
    f.iterations = it + 1;
    std::vector<size_t> free;
    for (size_t a = 0; a < w; ++a) {
      if (s.frozen_cells && a < ratios_of(s)) { continue; }
      if (boxed(s, a) && ((f.th[a] >= 0.0 && jtr[a] > 0.0) || (f.th[a] <= -1.0 && jtr[a] < 0.0))) {
        continue;
      }
      free.push_back(a);
    }
    const size_t q = free.size();
    if (q == 0) { break; }
    bool moved = false;
    for (int tries = 0; tries < 14; ++tries) {
      std::vector<double> m(q * q), v(q);
      for (size_t a = 0; a < q; ++a) {
        for (size_t b = 0; b < q; ++b) { m[a * q + b] = jtj[free[a] * w + free[b]]; }
        m[a * q + a] += lambda * std::max(jtj[free[a] * w + free[a]], 1e-12);
        v[a] = jtr[free[a]];
      }
      if (!solve(m, v, q, &step)) {
        lambda *= 10.0;
        continue;
      }
      trial = f.th;
      for (size_t a = 0; a < q; ++a) {
        trial[free[a]] += step[a];
        if (boxed(s, free[a])) { trial[free[a]] = std::clamp(trial[free[a]], -1.0, 0.0); }
      }
      std::vector<double> nj, nr;
      const double nl = system(rd, s, trial, half, &nj, &nr);
      if (nl < loss) {
        const double gain = loss - nl;
        f.th = trial;
        jtj = nj;
        jtr = nr;
        lambda = std::max(lambda / 10.0, 1e-9);
        moved = true;
        const bool done = gain < 1e-10 * loss;
        loss = nl;
        if (done) { it = 1000; }
        break;
      }
      lambda *= 10.0;
    }
    if (!moved) { break; }
  }
  f.loss = loss;
  return f;
}

// The best of several starts: the loss is not convex in a product of a ratio and a kink.
fit_t best_fit(const reader_t& rd, const spec_t& s, int half,
               const std::vector<std::vector<double>>& starts)
{
  fit_t best;
  bool any = false;
  for (const std::vector<double>& st : starts) {
    const fit_t f = fit(rd, s, half, st);
    if (!any || f.loss < best.loss) {
      best = f;
      any = true;
    }
  }
  return best;
}

// Per-game sums of the loss difference against s0, at `th` for the rows of `half` (0: all), or
// across halves at th_even for odd games and th_odd for even ones.
struct per_game_t
{
  std::vector<double> d;   // by game id
  std::vector<int> n;      // rows by game id
  double total = 0.0;
  uint64_t rows = 0;
};

void accumulate(const reader_t& rd, const spec_t& s, const std::vector<double>* th_even_rows,
                const std::vector<double>* th_odd_rows, per_game_t* out)
{
  const std::vector<row_t>& rows = *rd.rows;
  out->d.assign(MAX_GAME, 0.0);
  out->n.assign(MAX_GAME, 0);
  out->total = 0.0;
  out->rows = 0;
  std::vector<double> d(width_of(s));
  for (const row_t& r : rows) {
    const std::vector<double>& th = (r.game % 2 == 0) ? *th_even_rows : *th_odd_rows;
    const double dl = delta(s, th, r, d.data());
    const double y = r.label * 0.5;
    const double p1 = predict(rd, r, dl);
    const double p0 = tuner_target::sigmoid(r.s0, rd.k);
    const double diff = (y - p1) * (y - p1) - (y - p0) * (y - p0);
    out->d[r.game] += diff;
    out->n[r.game]++;
    out->total += diff;
    out->rows++;
  }
}

// The mean per row and its whole-game error, in 1e-6.
void clustered(const std::vector<double>& d, const std::vector<int>& n, uint64_t rows, double* mean,
               double* err)
{
  double total = 0.0;
  for (size_t g = 0; g < d.size(); ++g) { total += d[g]; }
  const double m = total / rows;
  double v = 0.0;
  for (size_t g = 0; g < d.size(); ++g) {
    if (n[g] == 0) { continue; }
    const double x = d[g] - m * n[g];
    v += x * x;
  }
  *mean = 1e6 * m;
  *err = 1e6 * std::sqrt(v) / rows;
}

std::string params_text(const spec_t& s, const std::vector<double>& th)
{
  std::string out;
  char buf[96];
  if (s.cells) {
    out += (s.ratio == RATIO_CELLS || s.ratio == RATIO_HYBRID)
               ? "r:"
               : ((s.ratio == RATIO_GLOBAL) ? "r all:" : "r by band:");
    if (s.ratio == RATIO_CELLS || s.ratio == RATIO_HYBRID) {
      for (int c = 0; c < CELLS; ++c) {
        snprintf(buf, sizeof buf, " %.3f", 1.0 + th[cell_param(c)]);
        out += buf;
      }
      if (s.ratio == RATIO_HYBRID) {
        out += "; by band:";
        for (int b = 0; b < 6; ++b) {
          snprintf(buf, sizeof buf, " %.3f", 1.0 + th[FREE_CELLS + b]);
          out += buf;
        }
      }
    } else {
      for (size_t i = 0; i < ratios_of(s); ++i) {
        snprintf(buf, sizeof buf, " %.3f", 1.0 + th[i]);
        out += buf;
      }
    }
    out += ";";
  }
  const size_t base = ratios_of(s);
  for (size_t i = 0; i < s.inputs.size(); ++i) {
    snprintf(buf, sizeof buf, " %s %.2f", input_name[s.inputs[i]], th[base + i]);
    out += buf;
  }
  return out;
}

std::vector<double> rounded(const spec_t& s, std::vector<double> th)
{
  // Cells to F = 128ths, complexity weights to integer centipawns.
  const size_t base = ratios_of(s);
  for (size_t i = 0; i < base; ++i) { th[i] = std::round((1.0 + th[i]) * 128.0) / 128.0 - 1.0; }
  for (size_t i = base; i < th.size(); ++i) { th[i] = std::round(th[i]); }
  return th;
}

// ------------------------------------------------------------------ the census
struct options_t
{
  std::string path;
  double k = 0.0;
  bool labels = false;
  size_t r2_stride = 0;
  uint64_t limit = 0;
  unsigned threads = 2;
  bool given_c = false;
  double c[IN_COUNT] = {0, 0, 0, 0};
  bool refit_k = false;  // read every label reading again at K refitted at S124's cells
};

// Ordinary least squares on a dense design with an intercept, by normal equations (S102's census's
// form): a pivot under 1e-9 of its diagonal is a column the earlier ones span and is dropped.
struct factor_t
{
  size_t width = 0;
  std::vector<double> l;
  std::vector<char> dead;
};

factor_t factor_gram(std::vector<double> gram, size_t width)
{
  factor_t f;
  f.width = width;
  f.dead.assign(width, 0);
  std::vector<double> diag(width);
  for (size_t a = 0; a < width; ++a) { diag[a] = gram[a * width + a]; }
  for (size_t a = 0; a < width; ++a) {
    for (size_t b = 0; b <= a; ++b) {
      if (f.dead[b]) {
        gram[a * width + b] = 0.0;
        continue;
      }
      double sum = gram[a * width + b];
      for (size_t c = 0; c < b; ++c) { sum -= gram[a * width + c] * gram[b * width + c]; }
      if (a == b) {
        if (diag[a] <= 0.0 || sum <= 1e-9 * diag[a]) {
          f.dead[a] = 1;
          gram[a * width + a] = 0.0;
        } else {
          gram[a * width + a] = std::sqrt(sum);
        }
      } else {
        gram[a * width + b] = sum / gram[b * width + b];
      }
    }
  }
  f.l = std::move(gram);
  return f;
}

double r_squared(const factor_t& f, const std::vector<double>& rhs, double yy, double ysum,
                 double n)
{
  const size_t w = f.width;
  std::vector<double> z(w, 0.0);
  double explained = 0.0;
  for (size_t a = 0; a < w; ++a) {
    if (f.dead[a]) { continue; }
    double sum = rhs[a];
    for (size_t c = 0; c < a; ++c) { sum -= f.l[a * w + c] * z[c]; }
    z[a] = sum / f.l[a * w + a];
    explained += z[a] * z[a];
  }
  const double tss = yy - ysum * ysum / n;
  if (tss <= 0.0) { return -1.0; }
  return 1.0 - (yy - explained) / tss;
}

size_t live_columns(const factor_t& f)
{
  size_t live = 0;
  for (char d : f.dead) { live += d ? 0 : 1; }
  return live;
}

constexpr int NEAR_BINS = 6;
const double near_edge[NEAR_BINS] = {0, 10, 25, 50, 100, 200};
const int grid_c[5] = {-10, -25, -50, -100, -200};

struct band_t
{
  uint64_t rows = 0;
  uint64_t p_hist[17] = {};
  uint64_t p_sum = 0;
  uint64_t flank = 0;
  uint64_t pe = 0;
  uint64_t near[NEAR_BINS] = {};  // |E1| <= edge
  uint64_t split = 0;             // E1 and E1 + E2 of opposite sign, or one of them 0 alone
  uint64_t celled = 0;            // a cell picked
  uint64_t bind_grid[5] = {};     // 0 < |E1| <= -C
  uint64_t bind_c = 0, widen_c = 0, damp_c = 0;  // at --c
};

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--k" && i + 1 < argc) {
      opts.k = atof(argv[++i]);
      opts.labels = true;
    } else if (a == "--r2" && i + 1 < argc) {
      opts.r2_stride = strtoull(argv[++i], nullptr, 10);
    } else if (a == "--limit" && i + 1 < argc) {
      opts.limit = strtoull(argv[++i], nullptr, 10);
    } else if (a == "--threads" && i + 1 < argc) {
      opts.threads = static_cast<unsigned>(atoi(argv[++i]));
    } else if (a == "--refit-k") {
      opts.refit_k = true;
    } else if (a == "--c" && i + 1 < argc) {
      opts.given_c = true;
      if (sscanf(argv[++i], "%lf,%lf,%lf,%lf", &opts.c[0], &opts.c[1], &opts.c[2], &opts.c[3]) != 4) {
        fprintf(stderr, "--c takes c0,cP,cF,cE\n");
        return 2;
      }
    } else if (opts.path.empty()) {
      opts.path = a;
    } else {
      fprintf(stderr, "unknown argument %s\n", a.c_str());
      return 2;
    }
  }
  if (opts.path.empty()) {
    fprintf(stderr, "census <corpus.tsv> [--k K] [--r2 STRIDE] [--c c0,cP,cF,cE] [--limit N]\n");
    return 2;
  }
  std::ifstream in(opts.path);
  if (!in) {
    fprintf(stderr, "cannot read %s\n", opts.path.c_str());
    return 1;
  }

  game_t game;
  initialize_game_const_data(&game);
  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();
  const tuner_trace::feature_table_t& ft = tuner_trace::feature_table();

  uint64_t rows = 0, walk_differ = 0, mismatches = 0;
  uint64_t e1_zero = 0, pe_cell[2] = {0, 0}, pe_rows_nocell = 0;
  band_t band[PHASE_BINS];
  games_t all_games, flank_games, pe_games, near_games, celled_games, bind_games[5];
  uint64_t cell_rows[CELLS] = {};
  uint64_t cell_pe[CELLS] = {}, cell_flank[CELLS] = {};
  double cell_p[CELLS] = {};

  std::vector<row_t> stored;
  std::vector<std::vector<tuner_trace::entry_t>> r2_entries;
  std::vector<row_t> r2_rows;

  std::string line, why;
  std::vector<std::string> fields;
  tuner_trace::row_trace_t trace;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }
    if (opts.limit != 0 && rows >= opts.limit) { break; }
    if (!split(line, &fields)) {
      fprintf(stderr, "a row with fewer than five fields: %s\n", line.c_str());
      return 1;
    }
    const int64_t game_id = strtoll(fields[4].c_str(), nullptr, 10);
    all_games.note(game_id);
    if (!load_FEN(fields[0], &game, &why)) {
      fprintf(stderr, "the engine refuses %s\n", fields[0].c_str());
      return 1;
    }
    const board_t* board = &game.board;
    const int phase = game_phase(board);

    const features_t f = features(board);
    const features_t w = walked(board);
    if (f.p != w.p || f.flank != w.flank || f.pe != w.pe) {
      walk_differ++;
      if (walk_differ <= 5) { fprintf(stderr, "walk differs: %s\n", fields[0].c_str()); }
    }

    if (!tuner_trace::trace_board(board, &trace, &why)) {
      fprintf(stderr, "trace refused (%s): %s\n", why.c_str(), fields[0].c_str());
      return 1;
    }
    tuner_trace::dataset_t one;
    tuner_trace::append_row(trace, 0.5f, 0, 0, &one);
    if (tuner_trace::exact_score(one, 0, weights.data()) != trace.engine_white) { mismatches++; }
    const halves_t h = halves_of(trace, params);
    const double e1 = h.material + h.eg[tuner_trace::SITE_POSITIONAL];
    const double e2 = h.eg[tuner_trace::SITE_MOBILITY] + h.eg[tuner_trace::SITE_KING_SAFETY];

    row_t r;
    r.s0 = static_cast<float>(tuner_trace::score(one, 0, params.data()));
    r.e1 = static_cast<int32_t>(std::lround(e1));
    r.e2 = static_cast<float>(e2);
    r.phase = static_cast<uint8_t>(phase);
    r.p = static_cast<uint8_t>(f.p);
    r.flank = f.flank;
    r.pe = f.pe;
    const int cw = endgame_cell(board, WHITE), cb = endgame_cell(board, BLACK);
    r.cell[0] = static_cast<uint8_t>(cw);
    r.cell[1] = static_cast<uint8_t>(cb);
    r.label = opts.labels ? static_cast<uint8_t>(std::lround(2.0 * atof(fields[1].c_str()))) : 0;
    r.lone = board->occupancies[WHITE] == board->bitboards[W_KING] ||
             board->occupancies[BLACK] == board->bitboards[B_KING];
    r.game = static_cast<int32_t>(game_id);
    if (std::fabs(e1 - r.e1) > 1e-9) {
      fprintf(stderr, "E1 is not an integer at the shipped weights: %s\n", fields[0].c_str());
      return 1;
    }

    rows++;
    band_t& bd = band[phase_bin(phase)];
    bd.rows++;
    bd.p_hist[std::min(f.p, 16)]++;
    bd.p_sum += f.p;
    if (f.flank) {
      bd.flank++;
      flank_games.note(game_id);
    }
    if (f.pe) {
      bd.pe++;
      pe_games.note(game_id);
    }
    const double a1 = std::fabs(e1);
    for (int i = 0; i < NEAR_BINS; ++i) { bd.near[i] += a1 <= near_edge[i]; }
    if (a1 <= 25 && phase < 24) { near_games.note(game_id); }
    if (r.e1 == 0) { e1_zero++; }
    if (sgn(e1) != sgn(e1 + e2)) { bd.split++; }
    const int cell = picked(r);
    if (cell != NO_CELL) {
      bd.celled++;
      celled_games.note(game_id);
      cell_rows[cell]++;
      cell_pe[cell] += f.pe;
      cell_flank[cell] += f.flank;
      cell_p[cell] += f.p;
    }
    if (f.pe) {
      if (cell == NO_CELL) {
        pe_rows_nocell++;
      } else {
        pe_cell[cell <= 8 ? 0 : 1]++;
      }
    }
    for (int i = 0; i < 5; ++i) {
      if (a1 > 0 && a1 <= -grid_c[i] && phase < 24) {
        bd.bind_grid[i]++;
        bind_games[i].note(game_id);
      }
    }
    if (opts.given_c && phase < 24 && r.e1 != 0 && !r.lone) {
      const double c = opts.c[0] + opts.c[1] * f.p + opts.c[2] * f.flank + opts.c[3] * f.pe;
      if (c <= -a1) {
        bd.bind_c++;
      } else if (c > 0) {
        bd.widen_c++;
      } else if (c < 0) {
        bd.damp_c++;
      }
    }

    if (opts.labels) { stored.push_back(r); }
    if (opts.r2_stride > 0 && (rows - 1) % opts.r2_stride == 0) {
      r2_entries.push_back(trace.entries);
      r2_rows.push_back(r);
    }
  }

  printf("S266 census of %s\n", opts.path.c_str());
  printf("rows %llu games %llu\n", (unsigned long long)rows, (unsigned long long)all_games.count);
  printf("walk: %llu rows differ from the bitboard form (P, flank, pe; every row)\n",
         (unsigned long long)walk_differ);
  printf("identity: %llu traced rows differ from evaluate()\n", (unsigned long long)mismatches);
  printf("E1 = 0 on %llu rows (%.3f%%)\n", (unsigned long long)e1_zero, 100.0 * e1_zero / rows);

  auto pct = [&](uint64_t a, uint64_t b) { return 100.0 * a / std::max<uint64_t>(b, 1); };
  printf("\nby phase band: rows %%, then within the band\n");
  printf("%-6s %8s %7s %7s %7s %8s %8s %8s %8s %8s %8s %8s\n", "phase", "rows%", "meanP", "flank%",
         "pe%", "E1=0%", "|E1|<=10", "<=25", "<=50", "<=100", "<=200", "split%");
  band_t tot;
  for (int b = 0; b < PHASE_BINS; ++b) {
    const band_t& x = band[b];
    printf("%-6s %8.3f %7.2f %7.2f %7.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f\n",
           phase_name[b], pct(x.rows, rows), double(x.p_sum) / std::max<uint64_t>(x.rows, 1),
           pct(x.flank, x.rows), pct(x.pe, x.rows), pct(x.near[0], x.rows), pct(x.near[1], x.rows),
           pct(x.near[2], x.rows), pct(x.near[3], x.rows), pct(x.near[4], x.rows),
           pct(x.near[5], x.rows), pct(x.split, x.rows));
    tot.rows += x.rows;
    tot.p_sum += x.p_sum;
    tot.flank += x.flank;
    tot.pe += x.pe;
    tot.split += x.split;
    tot.celled += x.celled;
    for (int i = 0; i < NEAR_BINS; ++i) { tot.near[i] += x.near[i]; }
    for (int i = 0; i < 17; ++i) { tot.p_hist[i] += x.p_hist[i]; }
    for (int i = 0; i < 5; ++i) { tot.bind_grid[i] += x.bind_grid[i]; }
    tot.bind_c += x.bind_c;
    tot.widen_c += x.widen_c;
    tot.damp_c += x.damp_c;
  }
  printf("%-6s %8.3f %7.2f %7.2f %7.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f\n", "all", 100.0,
         double(tot.p_sum) / rows, pct(tot.flank, rows), pct(tot.pe, rows), pct(tot.near[0], rows),
         pct(tot.near[1], rows), pct(tot.near[2], rows), pct(tot.near[3], rows),
         pct(tot.near[4], rows), pct(tot.near[5], rows), pct(tot.split, rows));
  printf("games: flank %llu, pe %llu, |E1| <= 25 below phase 24 %llu, a cell picked %llu\n",
         (unsigned long long)flank_games.count, (unsigned long long)pe_games.count,
         (unsigned long long)near_games.count, (unsigned long long)celled_games.count);
  printf("pe rows: %llu (%.3f%%), in a PAWNS cell %llu, in an OCB cell %llu, in no cell %llu\n",
         (unsigned long long)tot.pe, pct(tot.pe, rows), (unsigned long long)pe_cell[0],
         (unsigned long long)pe_cell[1], (unsigned long long)pe_rows_nocell);

  printf("\nP, the pawns on the board, rows %% by band:\n%-6s", "P");
  for (int b = 0; b < PHASE_BINS; ++b) { printf(" %7s", phase_name[b]); }
  printf(" %7s\n", "all");
  for (int p = 0; p <= 16; ++p) {
    printf("%-6d", p);
    for (int b = 0; b < PHASE_BINS; ++b) { printf(" %7.3f", pct(band[b].p_hist[p], band[b].rows)); }
    printf(" %7.3f\n", pct(tot.p_hist[p], rows));
  }

  printf("\nrows a C would take to zero: 0 < |E1| <= -C, phase under 24, rows %% by band, then all "
         "and games\n%-6s", "C");
  for (int b = 0; b < PHASE_BINS - 1; ++b) { printf(" %7s", phase_name[b]); }
  printf(" %8s %8s\n", "all%", "games");
  for (int i = 0; i < 5; ++i) {
    printf("%-6d", grid_c[i]);
    for (int b = 0; b < PHASE_BINS - 1; ++b) { printf(" %7.3f", pct(band[b].bind_grid[i], band[b].rows)); }
    printf(" %8.3f %8llu\n", pct(tot.bind_grid[i], rows), (unsigned long long)bind_games[i].count);
  }
  if (opts.given_c) {
    printf("\nat the given C (%.2f %.2f %.2f %.2f), rows with E1 != 0 below phase 24 and no king "
           "alone: bound to 0, "
           "widened (C > 0), damped (C < 0, not bound), rows %% by band\n%-8s", opts.c[0], opts.c[1],
           opts.c[2], opts.c[3], "");
    for (int b = 0; b < PHASE_BINS - 1; ++b) { printf(" %7s", phase_name[b]); }
    printf(" %8s\n", "all%");
    const char* what[3] = {"bound", "widened", "damped"};
    for (int k = 0; k < 3; ++k) {
      printf("%-8s", what[k]);
      for (int b = 0; b < PHASE_BINS - 1; ++b) {
        const uint64_t v = (k == 0) ? band[b].bind_c : ((k == 1) ? band[b].widen_c : band[b].damp_c);
        printf(" %7.3f", pct(v, band[b].rows));
      }
      const uint64_t v = (k == 0) ? tot.bind_c : ((k == 1) ? tot.widen_c : tot.damp_c);
      printf(" %8.3f\n", pct(v, rows));
    }
  }

  printf("\nS124's cells, picked by E1's sign: rows %%, and within the cell pe %%, flank %%, mean P\n");
  for (int c = 0; c < CELLS; ++c) {
    printf("%-15s %8.3f %7.2f %7.2f %6.2f\n", cell_name[c], pct(cell_rows[c], rows),
           pct(cell_pe[c], cell_rows[c]), pct(cell_flank[c], cell_rows[c]),
           cell_p[c] / std::max<uint64_t>(cell_rows[c], 1));
  }
  printf("any cell %.3f%% of rows\n", pct(tot.celled, rows));

  // ------------------------------------------------------------ collinearity
  if (opts.r2_stride > 0) {
    const size_t n = r2_rows.size();
    printf("\ncollinearity: %zu rows, every %zu-th. Each target column regressed on a design with an "
           "intercept; R^2, 1 is spanned.\n", n, opts.r2_stride);
    printf("  the complexity columns at the seed C = 0, reading A: sign(E1) x_i w_eg, the score's "
           "derivative in each weight; the existing design: every parameter's tapered column "
           "(material untapered, the rest c w_mg and c w_eg); S124's cells at F: E1 w_eg on the "
           "rows whose E1 picks the cell\n");
    const size_t P = tuner_layout::PARAM_COUNT;
    // Columns: 0 intercept, 1..P existing, P+1..P+13 cells, P+14..P+17 complexity.
    const size_t W = P + 1 + CELLS + IN_COUNT;
    std::vector<double> gram(W * W, 0.0);
    std::vector<std::pair<uint32_t, double>> xs;
    for (size_t r = 0; r < n; ++r) {
      const row_t& rr = r2_rows[r];
      const double wm = rr.phase / 24.0, we = 1.0 - wm;
      xs.clear();
      xs.push_back({0, 1.0});
      for (const auto& e : r2_entries[r]) {
        const tuner_trace::feature_t& fe = ft.features[e.feature];
        if (fe.site == tuner_trace::SITE_MATERIAL) {
          xs.push_back({fe.mg + 1, double(e.coef)});
        } else {
          xs.push_back({fe.mg + 1, e.coef * wm});
          if (fe.eg != tuner_trace::NO_PARAM) { xs.push_back({fe.eg + 1, e.coef * we}); }
        }
      }
      const int cell = picked(rr);
      if (cell != NO_CELL && we > 0) {
        xs.push_back({static_cast<uint32_t>(P + 1 + cell), rr.e1 * we});
      }
      const double s = sgn(rr.e1);
      for (int i = 0; i < IN_COUNT; ++i) {
        const double v = s * x_of(rr, i) * we;
        if (v != 0.0) { xs.push_back({static_cast<uint32_t>(P + 1 + CELLS + i), v}); }
      }
      for (const auto& [a, va] : xs) {
        for (const auto& [b, vb] : xs) { gram[a * W + b] += va * vb; }
      }
    }
    // R^2 of target column t on the columns `keep` lists.
    auto r2_on = [&](size_t t, const std::vector<size_t>& keep) {
      const size_t m = keep.size();
      std::vector<double> g(m * m), rhs(m);
      for (size_t a = 0; a < m; ++a) {
        for (size_t b = 0; b < m; ++b) { g[a * m + b] = gram[keep[a] * W + keep[b]]; }
        rhs[a] = gram[keep[a] * W + t];
      }
      const factor_t f = factor_gram(g, m);
      return std::make_pair(r_squared(f, rhs, gram[t * W + t], gram[0 * W + t], double(n)),
                            live_columns(f));
    };
    std::vector<size_t> existing;
    for (size_t a = 0; a <= P; ++a) { existing.push_back(a); }
    std::vector<size_t> with_cells = existing;
    for (int c = 0; c < CELLS; ++c) { with_cells.push_back(P + 1 + c); }
    const auto base = r2_on(P + 1 + CELLS, existing);
    printf("  live columns: existing %zu of %zu, intercept included\n", base.second, P + 1);
    printf("%-6s %12s %16s %22s\n", "column", "R2 existing", "R2 + S124 cells",
           "R2 + cells + the other three");
    for (int i = 0; i < IN_COUNT; ++i) {
      const size_t t = P + 1 + CELLS + i;
      std::vector<size_t> all = with_cells;
      for (int j = 0; j < IN_COUNT; ++j) {
        if (j != i) { all.push_back(P + 1 + CELLS + j); }
      }
      printf("%-6s %12.6f %16.6f %22.6f\n", input_name[i], r2_on(t, existing).first,
             r2_on(t, with_cells).first, r2_on(t, all).first);
    }
    printf("  S124's cells on the existing design plus the four complexity columns:\n");
    std::vector<size_t> with_cx = existing;
    for (int j = 0; j < IN_COUNT; ++j) { with_cx.push_back(P + 1 + CELLS + j); }
    for (int c = 0; c < CELLS; ++c) {
      const size_t t = P + 1 + c;
      if (gram[t * W + t] == 0.0) {
        printf("  %-15s no row\n", cell_name[c]);
        continue;
      }
      printf("  %-15s existing %.6f, + complexity %.6f\n", cell_name[c], r2_on(t, existing).first,
             r2_on(t, with_cx).first);
    }
    std::vector<size_t> everything = with_cells;
    for (int j = 0; j < IN_COUNT; ++j) { everything.push_back(P + 1 + CELLS + j); }
    std::vector<double> g(everything.size() * everything.size());
    for (size_t a = 0; a < everything.size(); ++a) {
      for (size_t b = 0; b < everything.size(); ++b) {
        g[a * everything.size() + b] = gram[everything[a] * W + everything[b]];
      }
    }
    printf("  live columns, existing + cells + complexity: %zu of %zu\n",
           live_columns(factor_gram(g, everything.size())), everything.size());
  }

  // ---------------------------------------------------------- label readings
  // Every reading at one K: the corpus's own, and with --refit-k again at K refitted at S124's cells.
  auto readings = [&](const double k_read) {
    reader_t rd{&stored, k_read, std::max(1u, opts.threads)};
    const size_t n = stored.size();
    double mse0 = 0.0;
    for (const row_t& r : stored) {
      const double p = tuner_target::sigmoid(r.s0, k_read);
      mse0 += (r.label * 0.5 - p) * (r.label * 0.5 - p);
    }
    mse0 /= n;
    printf("\nlabel readings: %zu rows, K %.4f, in-sample MSE at the shipped constants %.6f\n", n,
           k_read, mse0);
    printf("  each model fitted by damped Gauss-Newton (starts below) on the squared error, "
           "every engine weight shipped; dMSE in 1e-6 against s0 with whole-game errors: in sample, "
           "at the fit rounded (cells to 128ths, weights to integers), and across halves (the even "
           "games' fit read on the odd and the odd games' on the even, summed over all rows). A "
           "reading and never a fit of the engine.\n");

    const std::vector<int> c4 = {IN_CONST, IN_P, IN_FLANK, IN_PE};
    std::vector<spec_t> specs;
    {
      spec_t s;
      s.name = "C4";
      s.inputs = c4;
      specs.push_back(s);
      s.name = "C1 c0 alone";
      s.inputs = {IN_CONST};
      specs.push_back(s);
      s.name = "C4 less cP";
      s.inputs = {IN_CONST, IN_FLANK, IN_PE};
      specs.push_back(s);
      s.name = "C4 less cF";
      s.inputs = {IN_CONST, IN_P, IN_PE};
      specs.push_back(s);
      s.name = "C4 less cE";
      s.inputs = {IN_CONST, IN_P, IN_FLANK};
      specs.push_back(s);
      s.name = "W4 whole half";
      s.inputs = c4;
      s.whole = true;
      specs.push_back(s);
      s.whole = false;
      s.name = "R4 min(C, 0)";
      s.restrict_neg = true;
      specs.push_back(s);
      s.restrict_neg = false;
      spec_t f;
      f.name = "F12";
      f.cells = true;
      specs.push_back(f);
      f.name = "F12+C4";
      f.inputs = c4;
      specs.push_back(f);
      f.name = "F12+C4'";
      f.other_order = true;
      specs.push_back(f);
      f.other_order = false;
      f.name = "F12+R4";
      f.restrict_neg = true;
      specs.push_back(f);
      f.restrict_neg = false;
      f.frozen_cells = true;
      f.name = "C4 | F12";
      specs.push_back(f);
      f.name = "R4 | F12";
      f.restrict_neg = true;
      specs.push_back(f);
      spec_t g;
      g.cells = true;
      g.ratio = RATIO_GLOBAL;
      g.name = "G1";
      specs.push_back(g);
      g.ratio = RATIO_PHASE;
      g.name = "G6";
      specs.push_back(g);
      g.inputs = c4;
      g.frozen_cells = true;
      g.name = "C4 | G6";
      specs.push_back(g);
      g.restrict_neg = true;
      g.name = "R4 | G6";
      specs.push_back(g);
      // The proposal's candidates: LONE left to mop-up, over S124's cells, and over S124's cells
      // with every other row's endgame half recalibrated by its phase band.
      spec_t h;
      h.cells = true;
      h.ratio = RATIO_HYBRID;
      h.name = "F12G6";
      specs.push_back(h);
      spec_t x;
      x.cells = true;
      x.inputs = c4;
      x.frozen_cells = true;
      x.skip_lone = true;
      x.name = "C4x | F12";
      specs.push_back(x);
      x.restrict_neg = true;
      x.name = "R4x | F12";
      specs.push_back(x);
      x.ratio = RATIO_HYBRID;
      x.restrict_neg = false;
      x.name = "C4x | F12G6";
      specs.push_back(x);
      x.restrict_neg = true;
      x.name = "R4x | F12G6";
      specs.push_back(x);
      // Q2 and Q3 in the screen's setting: the whole half, and the other order, over the cells.
      x.ratio = RATIO_CELLS;
      x.restrict_neg = false;
      x.whole = true;
      x.name = "W4x | F12";
      specs.push_back(x);
      x.whole = false;
      x.other_order = true;
      x.name = "C4x' | F12";
      specs.push_back(x);
    }
    enum { M_C4, M_C1, M_LP, M_LF, M_LE, M_W4, M_R4, M_F12, M_FC, M_FC2, M_FR, M_C_F, M_R_F, M_G1,
           M_G6, M_C_G, M_R_G, M_FG, M_CX_F, M_RX_F, M_CX_FG, M_RX_FG, M_WX_F, M_CX2_F };

    // fits[m][h]: h 0 all rows, 1 even games, 2 odd games. Every model starts from the identity;
    // a model with cells and complexity also from F12's fit of the same rows with C at 0, and from
    // that with C at the complexity-only fit's, the best loss kept; a frozen-cells model holds
    // F12's cells of the same rows and starts C at 0 and at the complexity-only fit's.
    std::vector<std::array<fit_t, 3>> fits(specs.size());
    std::vector<per_game_t> xh(specs.size());
    for (size_t m = 0; m < specs.size(); ++m) {
      const spec_t& s = specs[m];
      for (int h = 0; h < 3; ++h) {
        std::vector<std::vector<double>> starts;
        const std::vector<double> identity(width_of(s), 0.0);
        if (!(s.cells && !s.inputs.empty())) {
          starts.push_back(identity);
        } else {
          const int source = (s.ratio == RATIO_PHASE)    ? M_G6
                             : (s.ratio == RATIO_GLOBAL) ? M_G1
                             : (s.ratio == RATIO_HYBRID) ? M_FG
                                                         : M_F12;
          const std::vector<double>& cells = fits[source][h].th;
          const std::vector<double>& cx = fits[s.restrict_neg ? M_R4 : M_C4][h].th;
          std::vector<double> a(cells), b(cells);
          for (size_t i = 0; i < s.inputs.size(); ++i) {
            a.push_back(0.0);
            b.push_back(cx[i]);
          }
          if (!s.frozen_cells) { starts.push_back(identity); }
          starts.push_back(a);
          starts.push_back(b);
        }
        fits[m][h] = best_fit(rd, s, h, starts);
      }
      const fit_t& full = fits[m][0];
      const fit_t& ev = fits[m][1];
      const fit_t& od = fits[m][2];
      per_game_t in_sample, at_round;
      accumulate(rd, s, &full.th, &full.th, &in_sample);
      const std::vector<double> rt = rounded(s, full.th);
      accumulate(rd, s, &rt, &rt, &at_round);
      // Even games read at the odd games' fit and odd games at the even games'.
      accumulate(rd, s, &od.th, &ev.th, &xh[m]);
      double a, ae, b, be, c, ce;
      clustered(in_sample.d, in_sample.n, n, &a, &ae);
      clustered(at_round.d, at_round.n, n, &b, &be);
      clustered(xh[m].d, xh[m].n, n, &c, &ce);
      printf("%-14s in sample %9.2f +/- %6.2f  rounded %9.2f  across halves %9.2f +/- %6.2f  "
             "(%d, %d, %d iterations)\n", s.name, a, ae, b, c, ce, full.iterations,
             ev.iterations, od.iterations);
      printf("%-14s   fit:%s\n", "", params_text(s, full.th).c_str());
      printf("%-14s   even:%s\n", "", params_text(s, ev.th).c_str());
      printf("%-14s   odd:%s\n", "", params_text(s, od.th).c_str());
      fflush(stdout);
    }

    // Paired increments across halves.
    auto inc = [&](size_t big, size_t small) {
      std::vector<double> d(MAX_GAME);
      for (size_t g = 0; g < d.size(); ++g) { d[g] = xh[big].d[g] - xh[small].d[g]; }
      double m, e;
      clustered(d, xh[big].n, n, &m, &e);
      printf("  %-14s over %-14s %9.2f +/- %6.2f\n", specs[big].name, specs[small].name, m, e);
    };
    printf("\npaired increments across halves, 1e-6, whole-game errors:\n");
    inc(M_C4, M_C1);
    inc(M_C4, M_LP);
    inc(M_C4, M_LF);
    inc(M_C4, M_LE);
    inc(M_W4, M_C4);
    inc(M_R4, M_C4);
    inc(M_FC, M_F12);
    inc(M_FC2, M_F12);
    inc(M_FC, M_FC2);
    inc(M_FR, M_F12);
    inc(M_C_F, M_F12);
    inc(M_R_F, M_F12);
    inc(M_R_F, M_C_F);
    inc(M_FC, M_C4);
    inc(M_FR, M_R4);
    inc(M_F12, M_C4);
    inc(M_F12, M_R4);
    inc(M_G6, M_G1);
    inc(M_C4, M_G6);
    inc(M_C_G, M_G6);
    inc(M_R_G, M_G6);
    inc(M_F12, M_G6);
    inc(M_CX_F, M_F12);
    inc(M_RX_F, M_F12);
    inc(M_RX_F, M_CX_F);
    inc(M_CX_F, M_C_F);
    inc(M_FG, M_F12);
    inc(M_CX_FG, M_FG);
    inc(M_RX_FG, M_FG);
    inc(M_RX_FG, M_CX_FG);
    inc(M_R_F, M_RX_F);
    inc(M_WX_F, M_CX_F);
    inc(M_CX2_F, M_CX_F);

    // Where each reading's in-sample gain comes from: its dMSE summed over a region's rows, in
    // 1e-6 of all rows so the regions add to the total, whole-game errors, and the region's
    // label and prediction from the strong side's view (E1's sign) before and after.
    {
      enum { G_LONE, G_PAWNS, G_OCB, G_PE, G_LOW, G_MID, G_HIGH, G_24, G_COUNT };
      const char* gname[G_COUNT] = {"LONE (no cell)", "PAWNS cells 0-8", "OCB cells", "pe, no cell",
                                    "no cell, phase 0-8", "no cell, 9-16", "no cell, 17-23",
                                    "phase 24"};
      auto region = [](const row_t& r) {
        if (r.phase >= 24) { return int(G_24); }
        if (r.lone) { return int(G_LONE); }
        const int c = picked(r);
        if (c != NO_CELL) { return (c <= 8) ? int(G_PAWNS) : int(G_OCB); }
        if (r.pe) { return int(G_PE); }
        return (r.phase <= 8) ? int(G_LOW) : ((r.phase <= 16) ? int(G_MID) : int(G_HIGH));
      };
      std::vector<uint64_t> rows_in(G_COUNT, 0);
      for (const row_t& r : stored) { rows_in[region(r)]++; }
      for (size_t m : {size_t(M_C4), size_t(M_R4), size_t(M_F12), size_t(M_C_F), size_t(M_R_F),
                       size_t(M_G6), size_t(M_C_G), size_t(M_FG), size_t(M_CX_F), size_t(M_RX_F),
                       size_t(M_CX_FG), size_t(M_RX_FG)}) {
        const spec_t& s = specs[m];
        std::vector<std::vector<double>> dg(G_COUNT, std::vector<double>());
        std::vector<std::vector<int>> ng(G_COUNT);
        std::vector<double> lab(G_COUNT, 0), p0(G_COUNT, 0), p1(G_COUNT, 0);
        for (int g = 0; g < G_COUNT; ++g) {
          dg[g].assign(MAX_GAME, 0.0);
          ng[g].assign(MAX_GAME, 0);
        }
        std::vector<double> d(width_of(s));
        for (const row_t& r : stored) {
          const int g = region(r);
          const double dl = delta(s, fits[m][0].th, r, d.data());
          const double y = r.label * 0.5;
          const double q1 = predict(rd, r, dl);
          const double q0 = tuner_target::sigmoid(r.s0, k_read);
          dg[g][r.game] += (y - q1) * (y - q1) - (y - q0) * (y - q0);
          ng[g][r.game]++;
          const bool black = r.e1 < 0;
          lab[g] += black ? 1.0 - y : y;
          p0[g] += black ? 1.0 - q0 : q0;
          p1[g] += black ? 1.0 - q1 : q1;
        }
        printf("\nwhere %s's in-sample gain sits: dMSE over all rows, label, prediction before and "
               "after, strong side's view\n", s.name);
        for (int g = 0; g < G_COUNT; ++g) {
          double mean, err;
          clustered(dg[g], ng[g], n, &mean, &err);
          const double k = std::max<uint64_t>(rows_in[g], 1);
          printf("  %-20s rows %8llu (%6.3f%%)  %9.2f +/- %6.2f  label %.3f  before %.3f  after "
                 "%.3f\n", gname[g], (unsigned long long)rows_in[g], pct(rows_in[g], n), mean, err,
                 lab[g] / k, p0[g] / k, p1[g] / k);
        }
      }
    }

    // Where the fitted C acts, at the full fits of C4, R4, C4 | F12 and R4 | F12. The jump: where
    // C > 0, E1's sign change carries the endgame half from -C to +C, 2 C w_eg on the score.
    for (size_t m : {size_t(M_C4), size_t(M_R4), size_t(M_C_F), size_t(M_R_F), size_t(M_C_G),
                     size_t(M_CX_F), size_t(M_RX_F), size_t(M_CX_FG)}) {
      const spec_t& s = specs[m];
      const size_t base = ratios_of(s);
      uint64_t act[PHASE_BINS] = {}, bound[PHASE_BINS] = {}, widen[PHASE_BINS] = {},
               damp[PHASE_BINS] = {};
      double cmin = 1e9, cmax = -1e9;
      std::vector<float> jumps;
      uint64_t near_rows = 0;
      for (const row_t& r : stored) {
        if (r.phase >= 24 || r.e1 == 0 || (s.skip_lone && r.lone)) { continue; }
        double c = 0.0;
        for (size_t i = 0; i < s.inputs.size(); ++i) { c += fits[m][0].th[base + i] * x_of(r, s.inputs[i]); }
        if (s.restrict_neg) { c = std::min(c, 0.0); }
        cmin = std::min(cmin, c);
        cmax = std::max(cmax, c);
        const int b = phase_bin(r.phase);
        act[b]++;
        if (c <= -std::fabs(double(r.e1))) {
          bound[b]++;
        } else if (c > 0) {
          widen[b]++;
        } else if (c < 0) {
          damp[b]++;
        }
        if (std::abs(r.e1) <= 25) {
          near_rows++;
          if (c > 0) { jumps.push_back(static_cast<float>(2.0 * c * (24.0 - r.phase) / 24.0)); }
        }
      }
      printf("\nat %s's fit, rows with E1 != 0 below phase 24, C from %.2f to %.2f: bound to 0, "
             "widened, damped, rows %% of the band\n", s.name, cmin, cmax);
      uint64_t tb = 0, tw = 0, td = 0;
      for (int b = 0; b < PHASE_BINS - 1; ++b) {
        printf("  %-6s rows %8llu  bound %7.3f  widened %7.3f  damped %7.3f\n", phase_name[b],
               (unsigned long long)act[b], pct(bound[b], band[b].rows), pct(widen[b], band[b].rows),
               pct(damp[b], band[b].rows));
        tb += bound[b];
        tw += widen[b];
        td += damp[b];
      }
      printf("  all    bound %llu (%.3f%%), widened %llu (%.3f%%), damped %llu (%.3f%%)\n",
             (unsigned long long)tb, pct(tb, n), (unsigned long long)tw, pct(tw, n),
             (unsigned long long)td, pct(td, n));
      std::sort(jumps.begin(), jumps.end());
      if (!jumps.empty()) {
        printf("  0 < |E1| <= 25 below phase 24: %llu rows, %zu with C > 0, whose sign change of "
               "E1 jumps the score 2 C w_eg: p50 %.1f, p90 %.1f, max %.1f cp\n",
               (unsigned long long)near_rows, jumps.size(), jumps[jumps.size() / 2],
               jumps[jumps.size() * 9 / 10], jumps.back());
      } else {
        printf("  0 < |E1| <= 25 below phase 24: %llu rows, none with C > 0: no jump\n",
               (unsigned long long)near_rows);
      }
    }
  };

  if (opts.labels) {
    readings(opts.k);
    if (opts.refit_k) {
      // K refitted at S124's cells, to a fixed point: F12 fitted at K, K refitted at that fit by
      // golden section on the in-sample MSE, until K moves by under 1e-4. The tuner fits K at its
      // start constants; with S124's cells at their fit that is this K.
      const size_t n = stored.size();
      const unsigned threads = std::max(1u, opts.threads);
      auto mse_at = [&](const std::vector<double>& dl, double k) {
        std::vector<double> part(threads, 0.0);
        std::vector<std::thread> ws;
        for (unsigned t = 0; t < threads; ++t) {
          ws.emplace_back([&, t]() {
            double sum = 0.0;
            for (size_t i = t; i < n; i += threads) {
              const row_t& r = stored[i];
              const double e = r.label * 0.5 -
                               tuner_target::sigmoid(r.s0 + (24.0 - r.phase) / 24.0 * dl[i], k);
              sum += e * e;
            }
            part[t] = sum;
          });
        }
        for (auto& w : ws) { w.join(); }
        double total = 0.0;
        for (double v : part) { total += v; }
        return total / n;
      };
      auto best_k = [&](const std::vector<double>& dl) {
        double lo = 0.5, hi = 3.0;
        for (int it = 0; it < 60; ++it) {
          const double m1 = lo + (hi - lo) * 0.381966, m2 = lo + (hi - lo) * 0.618034;
          if (mse_at(dl, m1) < mse_at(dl, m2)) {
            hi = m2;
          } else {
            lo = m1;
          }
        }
        return 0.5 * (lo + hi);
      };
      spec_t f12;
      f12.name = "F12";
      f12.cells = true;
      std::vector<double> dl(n, 0.0), d(width_of(f12));
      const double k_identity = best_k(dl);
      printf("\nK refit: at the shipped constants the in-sample optimum is %.4f (the corpus's %.4f)\n",
             k_identity, opts.k);
      double k = opts.k;
      std::vector<double> th(width_of(f12), 0.0);
      int round = 0;
      for (; round < 30; ++round) {
        reader_t rk{&stored, k, threads};
        th = fit(rk, f12, 0, th).th;
        for (size_t i = 0; i < n; ++i) { dl[i] = delta(f12, th, stored[i], d.data()); }
        const double next = best_k(dl);
        printf("  round %d: F12 fitted at K %.4f, K refitted at it %.4f, MSE %.8f\n", round + 1, k,
               next, mse_at(dl, next));
        const bool done = std::fabs(next - k) < 1e-4;
        k = next;
        if (done) { break; }
      }
      printf("K refit converged after %d rounds at %.4f\n", round + 1, k);
      readings(k);
    }
  }

  printf("S266-CENSUS-OK\n");
  return 0;
}
