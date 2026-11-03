// S124's research pass, 2026-11-03: where each candidate endgame case fires on a corpus,
// how much of the endgame half it reaches, and (with --k, a training corpus only) what the
// labels read beside the shipped evaluation for a flat scale factor per case. Counts on the
// board; no fit, no timing, no chess judgement. A case is a count of men, pawns and bishop
// colours, never a reading of who is winning. Its output is read into
// adocs/data/S124_census.txt by adocs/data/S124_census.sh.
//
//   g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I src -I tools
//       adocs/data/S124_census.cpp build/src/libchesso_engine_trace.a -lpthread -o census
//   census <corpus.tsv> [--k K] [--dump FILE] [--pool [--exclude GAMES]] [--limit N]
//
// Without --k only the FEN, phase, game and ply columns are read. --k reads the result
// column too, and is passed for a training corpus only: S082's fixed views keep their labels
// unread by anything that chooses a form. --pool reads a raw datagen pool: every row is
// counted by men and lone king, and column 3, datagen's search score, is read for the
// recipe's filter at 1000 and nothing else; the trace runs only on rows of seven men or
// fewer, and no label table is printed. --dump writes the rows of five men or fewer, for the
// tablebase probe (adocs/data/S124_tb_probe.py); their result only under --k, and in a pool
// only for a game not in --exclude's list (the held-out games' ids, one a line, read from
// the views' game column alone). A dump line: fen, result or '-', game, ply, case, strong,
// E1, signature, phase, the smooth score at f = 1 (White relative), the label cell, the
// proposal's cell or -1.
//
// Every definition is chesso's own and is said to be (DEC-221):
//
//   E1        stage one's endgame half with the material in it: board->material plus the
//             endgame sum of the positional site (the piece-square tables and the pawn
//             families), White relative, from the trace at the shipped weights
//   E2        stage two's endgame half: mobility, threats, king safety and its curve,
//             before the clamp
//   strong    the side E1 favours; none when E1 is 0
//   npm[s]    a side's men in phase units, knight 1, bishop 1, rook 2, queen 4
//             (src/eval_tables.hpp phase_value), so no case boundary moves with a refit
//   d         npm[strong] - npm[weak]
//   P         the strong side's pawns
//   case, first that holds:
//     DEAD    is_insufficient_material(): the search returns DRAW_SCORE there
//     LONE    one side has its king alone: mop-up's domain
//     OCB     each side exactly one bishop, on squares of opposite colour; split PURE (no
//             knight, rook or queen), ROOK (one rook each and nothing else) and OTHER
//     PAWNS   anything else with a strong side, the pawn-count factor's domain, cells P
//             by d
//     NONE    E1 is 0
//
// A scale factor f multiplies an endgame half. The two readings, smooth (every taper
// exact, the clamp kept), White relative:
//   A  s + (f - 1) * E1 * w_eg                  stage one's half only
//   B  A, and stage two's endgame half times f before the clamp
// w_eg = (24 - phase) / 24. f runs 0 to 1.25 in steps of 1/32; a flat f per cell, over the
// labels, in sample: a reading and never a fit.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
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
constexpr int GRID = 41;  // f = i / 32, i in [0, 40]

double grid_f(int i) { return i / 32.0; }

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

enum case_t
{
  C_DEAD,
  C_LONE,
  C_OCB_PURE,
  C_OCB_ROOK,
  C_OCB_OTHER,
  C_PAWNS,
  C_NONE,
  C_COUNT
};

const char* case_name[C_COUNT] = {"DEAD",      "LONE",  "OCB_PURE", "OCB_ROOK",
                                  "OCB_OTHER", "PAWNS", "NONE"};

constexpr int P_CELLS = 9;  // P 0..8
constexpr int D_CELLS = 5;  // d <= -1, 0, 1, 2, >= 3
const char* d_name[D_CELLS] = {"d<=-1", "d=0", "d=1", "d=2", "d>=3"};

int d_cell(int d) { return std::clamp(d + 1, 0, D_CELLS - 1); }

// The label cells: the cases, the PAWNS grid, OCB_PURE by P.
constexpr int L_CASE = 0;
constexpr int L_GRID = C_COUNT;
constexpr int L_OCB = L_GRID + P_CELLS * D_CELLS;
constexpr int L_PROP = L_OCB + P_CELLS;

// The pass's proposal: the scale factor's thirteen cells, PAWNS for P 0 to 2 by d <= 0, 1,
// >= 2, OCB_PURE by P <= 2, 3 to 4, >= 5, and OCB_ROOK; mop-up's domain, a lone king against
// a side with no pawn and men worth two phase units or more, knights alone excepted, and
// KBN within it; and the rest of LONE.
enum prop_t
{
  PR_OCB_LOW = 9,
  PR_OCB_MID,
  PR_OCB_HIGH,
  PR_OCB_ROOK,
  PR_MOPUP,
  PR_MOPUP_KBN,
  PR_LONE_OTHER,
  PR_COUNT
};
const char* prop_name[PR_COUNT] = {"P=0 d<=0", "P=0 d=1", "P=0 d>=2", "P=1 d<=0", "P=1 d=1",
                                   "P=1 d>=2", "P=2 d<=0", "P=2 d=1", "P=2 d>=2", "OCB_PURE P<=2",
                                   "OCB_PURE P=3-4", "OCB_PURE P>=5", "OCB_ROOK", "MOPUP",
                                   "MOPUP of it KBN", "LONE other"};
constexpr int L_COUNT = L_PROP + PR_COUNT;

struct cls_t
{
  int pawns[2] = {0, 0};
  int knights[2] = {0, 0};
  int bishops[2] = {0, 0};
  int rooks[2] = {0, 0};
  int queens[2] = {0, 0};
  int npm[2] = {0, 0};
  int men = 0;
  bool lone[2] = {false, false};
  bool dead = false;
  bool ocb = false;
  int strong = -1;
  case_t c = C_NONE;
};

bb_t light_squares()
{
  bb_t out = 0;
  for (int sq = 0; sq < 64; ++sq) {
    if (((sq / 8) + (sq % 8)) % 2 == 0) { out |= (bb_t)1 << sq; }
  }
  return out;
}

const bb_t LIGHT = light_squares();

// The proposal's cell of a classified row, -1 for none; KBN is reported apart by the caller.
int prop_cell(const cls_t& c)
{
  if (c.c == C_LONE) {
    const int a = c.lone[0] ? 1 : 0;  // the side with men
    const bool knights_alone = c.knights[a] == c.npm[a];
    return (c.pawns[a] == 0 && c.npm[a] >= 2 && !knights_alone) ? PR_MOPUP : PR_LONE_OTHER;
  }
  if (c.strong < 0) { return -1; }
  const int p = c.pawns[c.strong];
  if (c.c == C_OCB_PURE) { return (p <= 2) ? PR_OCB_LOW : ((p <= 4) ? PR_OCB_MID : PR_OCB_HIGH); }
  if (c.c == C_OCB_ROOK) { return PR_OCB_ROOK; }
  if (c.c != C_PAWNS || p > 2) { return -1; }
  const int d = c.npm[c.strong] - c.npm[1 - c.strong];
  return p * 3 + ((d <= 0) ? 0 : ((d == 1) ? 1 : 2));
}

bool is_kbn(const cls_t& c)
{
  const int a = c.lone[0] ? 1 : 0;
  return c.c == C_LONE && c.pawns[a] == 0 && c.bishops[a] == 1 && c.knights[a] == 1 &&
         c.rooks[a] == 0 && c.queens[a] == 0;
}

void classify_material(const board_t* b, cls_t* c)
{
  for (int s = 0; s < 2; ++s) {
    const int o = (s == 0) ? 0 : B_PAWN;
    c->pawns[s] = count_bits(b->bitboards[o + W_PAWN]);
    c->knights[s] = count_bits(b->bitboards[o + W_KNIGHT]);
    c->bishops[s] = count_bits(b->bitboards[o + W_BISHOP]);
    c->rooks[s] = count_bits(b->bitboards[o + W_ROOK]);
    c->queens[s] = count_bits(b->bitboards[o + W_QUEEN]);
    c->npm[s] = c->knights[s] + c->bishops[s] + 2 * c->rooks[s] + 4 * c->queens[s];
    c->lone[s] = (c->pawns[s] + c->knights[s] + c->bishops[s] + c->rooks[s] +
                  c->queens[s]) == 0;
    c->men += 1 + c->pawns[s] + c->knights[s] + c->bishops[s] + c->rooks[s] +
              c->queens[s];
  }

  c->dead = is_insufficient_material(b);

  if (c->bishops[0] == 1 && c->bishops[1] == 1) {
    const bool w_light = (b->bitboards[W_BISHOP] & LIGHT) != 0;
    const bool b_light = (b->bitboards[B_BISHOP] & LIGHT) != 0;
    c->ocb = (w_light != b_light);
  }
}

void assign_case(cls_t* c)
{
  if (c->dead) {
    c->c = C_DEAD;
  } else if (c->lone[0] || c->lone[1]) {
    c->c = C_LONE;
  } else if (c->ocb) {
    const bool no_others = c->knights[0] + c->knights[1] + c->rooks[0] + c->rooks[1] +
                               c->queens[0] + c->queens[1] ==
                           0;
    const bool rook_each = c->rooks[0] == 1 && c->rooks[1] == 1 &&
                           c->knights[0] + c->knights[1] + c->queens[0] +
                                   c->queens[1] ==
                               0;
    c->c = no_others ? C_OCB_PURE : (rook_each ? C_OCB_ROOK : C_OCB_OTHER);
  } else if (c->strong >= 0) {
    c->c = C_PAWNS;
  } else {
    c->c = C_NONE;
  }
}

// A side's men as letters, queens first, pawns as a count.
std::string side_sig(const cls_t& c, int s)
{
  std::string out = "K";
  out += std::string(c.queens[s], 'Q');
  out += std::string(c.rooks[s], 'R');
  out += std::string(c.bishops[s], 'B');
  out += std::string(c.knights[s], 'N');
  if (c.pawns[s] > 0) { out += "P" + std::to_string(c.pawns[s]); }
  return out;
}

// The side with more men in phase units first, then more pawns, then White.
std::string signature(const cls_t& c)
{
  int first = 0;
  if (c.npm[1] > c.npm[0] || (c.npm[1] == c.npm[0] && c.pawns[1] > c.pawns[0])) {
    first = 1;
  }
  std::string sig = side_sig(c, first) + "v" + side_sig(c, 1 - first);
  if (c.bishops[0] == 1 && c.bishops[1] == 1) { sig += c.ocb ? " (ocb)" : " (scb)"; }
  return sig;
}

// The trace's halves at the shipped weights, smooth: material, each site's middlegame and
// endgame sums, the clamped groups' transforms added to king safety's halves as smooth()
// adds them.
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
    const double hm = transform(group.transform, group.divisor, xm[0]) -
                      transform(group.transform, group.divisor, xm[1]);
    const double he = transform(group.transform, group.divisor, xe[0]) -
                      transform(group.transform, group.divisor, xe[1]);
    if (!group.clamped) {
      fprintf(stderr, "an unclamped group: the census has no site for it\n");
      exit(1);
    }
    h.mg[SITE_KING_SAFETY] += group.sign * hm;
    h.eg[SITE_KING_SAFETY] += group.sign * he;
  }

  return h;
}

// The smooth score under reading A or B at factor f, White relative.
double scored(const halves_t& h, int phase, double f, bool b_reading)
{
  using namespace tuner_trace;
  const double mgw = phase / 24.0;
  const double egw = 1.0 - mgw;
  const double margin = LAZY_EVAL_MARGIN;
  const double e1 = h.material + h.eg[SITE_POSITIONAL];
  const double f2 = b_reading ? f : 1.0;
  double two = (h.mg[SITE_MOBILITY] + h.mg[SITE_KING_SAFETY]) * mgw +
               f2 * (h.eg[SITE_MOBILITY] + h.eg[SITE_KING_SAFETY]) * egw;
  two = std::clamp(two, -margin, margin);
  return h.material + h.mg[SITE_POSITIONAL] * mgw + h.eg[SITE_POSITIONAL] * egw +
         (f - 1.0) * e1 * egw + h.mg[SITE_TEMPO] * mgw + h.eg[SITE_TEMPO] * egw + two;
}

// A cell's label reading, clustered by game.
struct label_cell_t
{
  uint64_t rows = 0;
  uint64_t games = 0;
  double nn = 0;  // sum over games of n_g^2
  double err1 = 0;
  double gap = 0, gap_g2 = 0, gap_ng = 0;  // label - prediction, strong side's view
  double label = 0, pred = 0;
  double s[2][GRID] = {}, ss[2][GRID] = {}, sn[2][GRID] = {};
  // the open game
  int n_g = 0;
  double d_g[2][GRID] = {};
  double gap_open = 0;
};

struct count_cell_t
{
  uint64_t rows = 0;
  double eg_mass = 0;  // sum of w_eg
  double e1_abs = 0;   // sum of |E1| * w_eg
  double e2_abs = 0;   // sum of |E2| * w_eg
  uint64_t sign_split = 0;  // E1 and E1 + E2 of opposite sign
  games_t games;
};

struct options_t
{
  std::string path;
  double k = 0.0;
  bool labels = false;
  bool pool = false;
  std::string dump;
  std::string exclude;
  uint64_t limit = 0;
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
  return out->size() >= 6;
}

// Men on a FEN's board, from its first field.
int fen_men(const std::string& fen)
{
  int men = 0;
  for (char ch : fen) {
    if (ch == ' ') { break; }
    if (std::isalpha(static_cast<unsigned char>(ch))) { men++; }
  }
  return men;
}

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;

  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--k" && i + 1 < argc) {
      opts.k = atof(argv[++i]);
      opts.labels = true;
    } else if (a == "--dump" && i + 1 < argc) {
      opts.dump = argv[++i];
    } else if (a == "--pool") {
      opts.pool = true;
    } else if (a == "--exclude" && i + 1 < argc) {
      opts.exclude = argv[++i];
    } else if (a == "--limit" && i + 1 < argc) {
      opts.limit = strtoull(argv[++i], nullptr, 10);
    } else if (opts.path.empty()) {
      opts.path = a;
    } else {
      fprintf(stderr, "unknown argument %s\n", a.c_str());
      return 2;
    }
  }

  if (opts.path.empty() || (opts.pool && opts.labels && opts.exclude.empty()) ||
      (!opts.exclude.empty() && !opts.pool)) {
    fprintf(stderr,
            "census <corpus.tsv> [--k K] [--dump FILE] [--pool [--exclude GAMES]] [--limit N]\n"
            "  a pool's labels need --exclude, the held-out games' ids\n");
    return 2;
  }

  std::vector<bool> excluded(MAX_GAME, false);
  if (!opts.exclude.empty()) {
    std::ifstream ex(opts.exclude);
    if (!ex) {
      fprintf(stderr, "cannot read %s\n", opts.exclude.c_str());
      return 1;
    }
    std::string id;
    uint64_t n = 0;
    while (std::getline(ex, id)) {
      if (id.empty()) { continue; }
      const long long g = atoll(id.c_str());
      if (g < 0 || g >= MAX_GAME) {
        fprintf(stderr, "game id %s past the census's table\n", id.c_str());
        return 1;
      }
      excluded[g] = true;
      n++;
    }
    fprintf(stderr, "excluded games: %llu\n", (unsigned long long)n);
  }

  std::ifstream in(opts.path);
  if (!in) {
    fprintf(stderr, "cannot read %s\n", opts.path.c_str());
    return 1;
  }

  FILE* dump = nullptr;
  if (!opts.dump.empty()) {
    dump = fopen(opts.dump.c_str(), "w");
    if (dump == nullptr) {
      fprintf(stderr, "cannot write %s\n", opts.dump.c_str());
      return 1;
    }
  }

  game_t game;
  initialize_game_const_data(&game);

  std::vector<double> params(tuner_layout::PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());
  const std::vector<int64_t> weights = tuner_trace::engine_weights();

  uint64_t rows = 0, traced = 0, mismatches = 0, smooth_off = 0;
  games_t all_games;
  count_cell_t cases[C_COUNT];
  std::vector<count_cell_t> grid(P_CELLS * D_CELLS);
  std::vector<count_cell_t> ocb_by_p(P_CELLS);
  std::vector<count_cell_t> prop(PR_COUNT);
  std::map<std::string, std::pair<uint64_t, games_t*>> lone_sigs;
  std::map<std::string, uint64_t> low_sigs;  // phase <= 6
  std::map<std::string, games_t*> low_sig_games;
  uint64_t men_hist[33] = {};
  uint64_t pool_past = 0, pool_lone = 0, pool_lone_past = 0, pool_le5 = 0,
           pool_le5_past = 0;
  std::vector<label_cell_t> lab(opts.labels ? L_COUNT : 0);
  std::vector<int> touched;
  int64_t open_game = -1;

  const auto close_game = [&]() {
    for (int cell : touched) {
      label_cell_t& L = lab[cell];
      L.games++;
      L.nn += double(L.n_g) * L.n_g;
      for (int o = 0; o < 2; ++o) {
        for (int i = 0; i < GRID; ++i) {
          L.s[o][i] += L.d_g[o][i];
          L.ss[o][i] += L.d_g[o][i] * L.d_g[o][i];
          L.sn[o][i] += L.n_g * L.d_g[o][i];
          L.d_g[o][i] = 0;
        }
      }
      L.gap_g2 += L.gap_open * L.gap_open;
      L.gap_ng += L.n_g * L.gap_open;
      L.gap_open = 0;
      L.n_g = 0;
    }
    touched.clear();
  };

  std::string line;
  std::vector<std::string> fields;
  tuner_trace::row_trace_t trace;
  std::string why;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }
    if (opts.limit != 0 && rows >= opts.limit) { break; }
    if (!split(line, &fields)) {
      fprintf(stderr, "a row with fewer than six fields: %s\n", line.c_str());
      return 1;
    }
    rows++;

    const std::string& fen = fields[0];
    const int64_t game_id = strtoll(fields[4].c_str(), nullptr, 10);
    all_games.note(game_id);

    const int men = fen_men(fen);
    men_hist[std::min(men, 32)]++;

    bool past = false;
    if (opts.pool) {
      past = std::abs(atoi(fields[2].c_str())) >= 1000;
      if (past) { pool_past++; }
      if (men > 7) { continue; }
    }

    if (!load_FEN(fen, &game, &why)) {
      fprintf(stderr, "the engine refuses %s\n", fen.c_str());
      return 1;
    }

    if (!tuner_trace::trace_board(&game.board, &trace, &why)) {
      fprintf(stderr, "trace refused (%s): %s\n", why.c_str(), fen.c_str());
      return 1;
    }
    traced++;

    // The identity: the trace's integer score is evaluate()'s.
    {
      tuner_trace::dataset_t one;
      tuner_trace::append_row(trace, 0.5f, 0, 0, &one);
      if (tuner_trace::exact_score(one, 0, weights.data()) != trace.engine_white) {
        mismatches++;
      }
      const double smooth = tuner_trace::score(one, 0, params.data());
      const halves_t h0 = halves_of(trace, params);
      if (std::abs(scored(h0, trace.phase, 1.0, false) - smooth) > 1e-6) { smooth_off++; }
    }

    const halves_t h = halves_of(trace, params);
    const int phase = trace.phase;
    const double egw = (24.0 - phase) / 24.0;
    const double e1 = h.material + h.eg[tuner_trace::SITE_POSITIONAL];
    const double e2 = h.eg[tuner_trace::SITE_MOBILITY] + h.eg[tuner_trace::SITE_KING_SAFETY];

    cls_t c;
    classify_material(&game.board, &c);
    c.strong = (e1 > 0) ? 0 : ((e1 < 0) ? 1 : -1);
    assign_case(&c);

    if (opts.pool) {
      const bool lone = c.lone[0] || c.lone[1];
      if (lone) {
        pool_lone++;
        if (past) { pool_lone_past++; }
      }
      if (men <= 5) {
        pool_le5++;
        if (past) { pool_le5_past++; }
      }
    }

    const auto count = [&](count_cell_t& cell) {
      cell.rows++;
      cell.eg_mass += egw;
      cell.e1_abs += std::abs(e1) * egw;
      cell.e2_abs += std::abs(e2) * egw;
      if ((e1 > 0 && e1 + e2 < 0) || (e1 < 0 && e1 + e2 > 0)) { cell.sign_split++; }
      cell.games.note(game_id);
    };

    count(cases[c.c]);

    const int weak = (c.strong >= 0) ? 1 - c.strong : -1;
    const int p_strong = (c.strong >= 0) ? std::min(c.pawns[c.strong], 8) : 0;
    const int d = (c.strong >= 0) ? c.npm[c.strong] - c.npm[weak] : 0;
    int grid_cell = -1;

    if (c.c == C_PAWNS) {
      grid_cell = p_strong * D_CELLS + d_cell(d);
      count(grid[grid_cell]);
    }
    if (c.c == C_OCB_PURE && c.strong >= 0) { count(ocb_by_p[p_strong]); }
    const int pc = prop_cell(c);
    if (pc >= 0) { count(prop[pc]); }
    if (is_kbn(c)) { count(prop[PR_MOPUP_KBN]); }

    const std::string sig = signature(c);
    if (c.c == C_LONE) {
      auto& slot = lone_sigs[sig];
      if (slot.second == nullptr) { slot.second = new games_t(); }
      slot.first++;
      slot.second->note(game_id);
    }
    if (phase <= 6 && !opts.pool) {
      low_sigs[sig]++;
      games_t*& gs = low_sig_games[sig];
      if (gs == nullptr) { gs = new games_t(); }
      gs->note(game_id);
    }

    if (dump != nullptr && men <= 5) {
      const bool label_ok = opts.labels && !excluded[game_id];
      // The label cell, as the table below numbers it: the PAWNS grid or OCB_PURE by P,
      // else the case.
      const int label_cell =
          (grid_cell >= 0) ? L_GRID + grid_cell
                           : ((c.c == C_OCB_PURE && c.strong >= 0) ? L_OCB + p_strong
                                                                   : L_CASE + c.c);
      fprintf(dump, "%s\t%s\t%lld\t%s\t%s\t%d\t%.1f\t%s\t%d\t%.3f\t%d\t%d\n", fen.c_str(),
              label_ok ? fields[1].c_str() : "-", (long long)game_id,
              fields[5].c_str(), case_name[c.c], c.strong, e1, sig.c_str(), phase,
              scored(h, phase, 1.0, false), label_cell, pc);
    }

    if (!opts.labels || opts.pool) { continue; }

    // The labels: the training corpus only.
    if (game_id != open_game) {
      close_game();
      open_game = game_id;
    }

    const double result = atof(fields[1].c_str());
    const double orient = (c.strong == 1) ? -1.0 : 1.0;
    const double s0 = trace.engine_white;
    const double pred0 = tuner_target::sigmoid(s0, opts.k);

    double err[2][GRID];
    for (int o = 0; o < 2; ++o) {
      for (int i = 0; i < GRID; ++i) {
        const double s = scored(h, phase, grid_f(i), o == 1);
        const double dd = result - tuner_target::sigmoid(s, opts.k);
        err[o][i] = dd * dd;
      }
    }

    const auto add = [&](int cell) {
      label_cell_t& L = lab[cell];
      if (L.n_g == 0) { touched.push_back(cell); }
      L.n_g++;
      L.rows++;
      L.err1 += err[0][32];
      const double strong_label = (orient > 0) ? result : 1.0 - result;
      const double strong_pred = (orient > 0) ? pred0 : 1.0 - pred0;
      L.label += strong_label;
      L.pred += strong_pred;
      L.gap += strong_label - strong_pred;
      L.gap_open += strong_label - strong_pred;
      for (int o = 0; o < 2; ++o) {
        for (int i = 0; i < GRID; ++i) {
          L.d_g[o][i] += err[o][i] - err[o][32];
        }
      }
    };

    add(L_CASE + c.c);
    if (grid_cell >= 0) { add(L_GRID + grid_cell); }
    if (c.c == C_OCB_PURE && c.strong >= 0) { add(L_OCB + p_strong); }
    if (pc >= 0) { add(L_PROP + pc); }
    if (is_kbn(c)) { add(L_PROP + PR_MOPUP_KBN); }
  }

  if (opts.labels && !opts.pool) { close_game(); }
  if (dump != nullptr) { fclose(dump); }

  printf("S124 census of %s\n", opts.path.c_str());
  printf("rows %llu games %llu traced %llu identity_mismatches %llu smooth_mismatches %llu\n",
         (unsigned long long)rows, (unsigned long long)all_games.count,
         (unsigned long long)traced, (unsigned long long)mismatches,
         (unsigned long long)smooth_off);
  if (opts.labels && !opts.pool) { printf("labels: result column read at K %.4f\n", opts.k); }
  if (opts.labels && opts.pool) {
    printf("labels: result column written to the dump for games outside %s, no table\n",
           opts.exclude.c_str());
  }
  if (opts.pool) {
    printf("pool: past the filter (|column 3| >= 1000) %llu; lone-king rows %llu, of them past %llu; "
           "rows of five men or fewer %llu, of them past %llu\n",
           (unsigned long long)pool_past, (unsigned long long)pool_lone,
           (unsigned long long)pool_lone_past, (unsigned long long)pool_le5,
           (unsigned long long)pool_le5_past);
  }

  printf("\nmen on the board (kings included): rows\n");
  for (int m = 2; m <= 32; ++m) {
    if (men_hist[m] != 0) { printf("  men %2d %llu\n", m, (unsigned long long)men_hist[m]); }
  }

  const auto print_cell = [&](const char* name, const count_cell_t& cell) {
    printf("  %-22s rows %9llu  %7.3f %%  games %7llu  eg_mass %10.1f  |E1|w %7.1f  |E2|w %6.1f  split %llu\n",
           name, (unsigned long long)cell.rows, 100.0 * cell.rows / std::max<uint64_t>(rows, 1),
           (unsigned long long)cell.games.count, cell.eg_mass,
           cell.rows ? cell.e1_abs / cell.rows : 0.0, cell.rows ? cell.e2_abs / cell.rows : 0.0,
           (unsigned long long)cell.sign_split);
  };

  printf("\ncases, first that holds (of %s rows):\n", opts.pool ? "traced" : "all");
  for (int k = 0; k < C_COUNT; ++k) { print_cell(case_name[k], cases[k]); }

  printf("\nPAWNS by P (the strong side's pawns) and d (its men's phase units less the other's):\n");
  for (int p = 0; p < P_CELLS; ++p) {
    for (int dc = 0; dc < D_CELLS; ++dc) {
      char name[32];
      snprintf(name, sizeof(name), "P=%d %s", p, d_name[dc]);
      if (grid[p * D_CELLS + dc].rows != 0) { print_cell(name, grid[p * D_CELLS + dc]); }
    }
  }

  printf("\nOCB_PURE by P:\n");
  for (int p = 0; p < P_CELLS; ++p) {
    char name[32];
    snprintf(name, sizeof(name), "P=%d", p);
    if (ocb_by_p[p].rows != 0) { print_cell(name, ocb_by_p[p]); }
  }

  printf("\nthe proposal's cells:\n");
  for (int k = 0; k < PR_COUNT; ++k) { print_cell(prop_name[k], prop[k]); }

  printf("\nLONE by signature:\n");
  {
    std::vector<std::pair<uint64_t, std::string>> v;
    for (auto& kv : lone_sigs) { v.push_back({kv.second.first, kv.first}); }
    std::sort(v.rbegin(), v.rend());
    for (auto& e : v) {
      printf("  %-24s rows %8llu games %7llu\n", e.second.c_str(), (unsigned long long)e.first,
             (unsigned long long)lone_sigs[e.second].second->count);
    }
  }

  if (!opts.pool) {
    printf("\nsignatures at phase <= 6, top 60 by rows:\n");
    std::vector<std::pair<uint64_t, std::string>> v;
    for (auto& kv : low_sigs) { v.push_back({kv.second, kv.first}); }
    std::sort(v.rbegin(), v.rend());
    for (size_t i = 0; i < v.size() && i < 60; ++i) {
      printf("  %-28s rows %8llu games %7llu\n", v[i].second.c_str(),
             (unsigned long long)v[i].first,
             (unsigned long long)low_sig_games[v[i].second]->count);
    }
    printf("  signatures at phase <= 6: %zu\n", v.size());
  }

  if (opts.labels && !opts.pool) {
    printf("\nlabels, in sample, a flat f per cell, dMSE against f = 1 in 1e-6 with whole-game errors;\n"
           "gap = mean label less mean prediction, the strong side's view, in points of 1e-3:\n");
    printf("  %-22s %9s %7s %8s %8s %12s  %6s %16s  %6s %16s\n", "cell", "rows", "games",
           "label", "pred", "gap", "f_A", "dMSE_A", "f_B", "dMSE_B");
    for (int cell = 0; cell < L_COUNT; ++cell) {
      const label_cell_t& L = lab[cell];
      if (L.rows == 0) { continue; }
      char name[40];
      if (cell < L_GRID) {
        snprintf(name, sizeof(name), "%s", case_name[cell]);
      } else if (cell < L_OCB) {
        const int g = cell - L_GRID;
        snprintf(name, sizeof(name), "PAWNS P=%d %s", g / D_CELLS, d_name[g % D_CELLS]);
      } else {
        if (cell < L_PROP) {
          snprintf(name, sizeof(name), "OCB_PURE P=%d", cell - L_OCB);
        } else {
          snprintf(name, sizeof(name), "prop %s", prop_name[cell - L_PROP]);
        }
      }
      const double n = L.rows;
      const double gap = L.gap / n;
      const double mu_gap = L.gap / n;
      const double var_gap = L.gap_g2 - 2 * mu_gap * L.gap_ng + mu_gap * mu_gap * L.nn;
      const double se_gap = std::sqrt(std::max(var_gap, 0.0)) / n;
      int best[2] = {32, 32};
      for (int o = 0; o < 2; ++o) {
        for (int i = 0; i < GRID; ++i) {
          if (L.s[o][i] < L.s[o][best[o]]) { best[o] = i; }
        }
      }
      double dm[2], se[2];
      for (int o = 0; o < 2; ++o) {
        const int i = best[o];
        const double mu = L.s[o][i] / n;
        const double var = L.ss[o][i] - 2 * mu * L.sn[o][i] + mu * mu * L.nn;
        dm[o] = 1e6 * L.s[o][i] / n;
        se[o] = 1e6 * std::sqrt(std::max(var, 0.0)) / n;
      }
      printf("  %-22s %9llu %7llu %8.4f %8.4f %6.1f +/- %4.1f  %6.3f %8.2f +/- %5.2f  %6.3f %8.2f +/- %5.2f\n",
             name, (unsigned long long)L.rows, (unsigned long long)L.games, L.label / n,
             L.pred / n, 1e3 * gap, 1e3 * se_gap, grid_f(best[0]), dm[0], se[0],
             grid_f(best[1]), dm[1], se[1]);
    }

    printf("\nthe same cells' dMSE over the whole corpus's rows, 1e-6 (a cell's sum / all rows):\n");
    for (int cell = 0; cell < L_COUNT; ++cell) {
      const label_cell_t& L = lab[cell];
      if (L.rows == 0) { continue; }
      int best = 32;
      for (int i = 0; i < GRID; ++i) {
        if (L.s[0][i] < L.s[0][best]) { best = i; }
      }
      printf("  cell %2d rows %9llu  A at f %.3f: %.3f  at f 0.5: %.3f\n", cell,
             (unsigned long long)L.rows, grid_f(best), 1e6 * L.s[0][best] / rows,
             1e6 * L.s[0][16] / rows);
    }
  }

  printf("S124-CENSUS-OK\n");
  return 0;
}
