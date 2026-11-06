// S124, DEC-350 (5). The rows tests/test_eval_positions.hpp gains so that
// every parameter of S124's endgame family is moved by the fixture, found by
// the rule below and never by hand. Written 2026-11-04 in S124's phase 1, in
// adocs/data/S101_coverage_rows.cpp's shape and with S121's rule; it runs on
// the tree the family lands in, and again on any later tree whose counts or
// constants move what it reads.
//
// WHAT IT ANSWERS. Two cases read every non-table column of the fixture, and
// at S124's landing they read red on the family's 19 parameters, which the 106
// rows before this block never reach:
//
//   tests/test_tuner_gradient.cpp "gradient() is the derivative of the error
//     it descends": every parameter from MOB_MG_BASE to PARAM_COUNT with
//     |analytic gradient| > 1e-12 over the list alone, the case's labels;
//   tests/test_eval_trace.cpp "the positions move every term both ways": every
//     feature outside the piece-square tables non-zero both ways over
//     all_positions() (the list, each row's colour-mirror, all_test_fens()).
//
// Neither case changes. The answer is rows, S100's, DEC-316's, S121's, S123's,
// S125's and S101's precedent.
//
// IT RE-DERIVES ITS OWN BLOCK (DEC-142). The list it starts from is the
// fixture's first BASE_ROWS = 106 rows, every row before S124's block, and not
// the landed list: the count is asserted against the block's marker comment in
// tests/test_eval_positions.hpp ("S124's endgame family, added at S124's"),
// read from S124_POSITIONS_HPP (default the source tree's, under
// CHESSO_SOURCE_DIR). Every row of the landed block is held to `eligible`
// again, each pair as the position and its mirror; and the picks made from the
// base must be the landed block, pair for pair and in order, or the program
// exits 1.
//
// THE RULE, S121's (DEC-326 (6)) over S124's parameters.
//   needs      mop-up's: a (feature, half) is open when no row of the list has
//              a non-zero coefficient on it at a phase that gives the half a
//              taper weight, phase > 0 for the middlegame half, < 24 for the
//              endgame one; 3 features, 6 needs. The factor's: a cell is open
//              when no row of the list picks it -- its E1 at the shipped
//              weights non-zero, its sign's candidate the cell -- at a phase
//              below 24, where the cell's gradient E1 w_eg / F is not zero; 13
//              needs. 19 needs, one per parameter.
//   candidates all_test_fens() in that function's sorted order; then, only for
//              the needs still open, the rows of the file named by
//              S124_COVERAGE_DATA (the coordinator names S083's corpus), in
//              file order, the first 64 eligible rows per open need kept; then,
//              only for what is still open, S101's constructive fallback:
//              CONSTRUCT_DRAWS placements drawn by std::mt19937 seeded
//              20261104, its raw output, of two kings, 0 to 8 pawns a side on
//              their legal rows and 0 to 2 pieces a kind a side, White to move.
//              A candidate the list already holds, either way round, is
//              skipped. A stage stops reading once every open need has its 64,
//              since no later row could then be kept.
//   eligible   S121's, unchanged: load_FEN accepts it and its colour-mirror;
//              its phase is 1 to 23, so a row that closes a feature closes
//              both halves; it and its mirror each sit more than a centipawn
//              inside the lazy clamp at the shipped constants; each scores
//              within SCORE_LIMIT = 800 cp of level there; and each loads
//              with either side to move.
//   pick       greedy, stage by stage: the eligible candidate closing the most
//              open needs, the earliest on a tie, appended as the list's
//              convention has it -- the position, then its colour-mirror --
//              until no need is open. A mirror picks its position's cell, P
//              and d being the same for the mirrored strong side. A need no
//              candidate closes is printed UNCLOSED and the program exits 1:
//              that stops phase 1 for a ruling, never a row made up to close
//              it.
//   checks     over the list with the picks appended: the gradient case's
//              non-vacuity, every parameter from MOB_MG_BASE to PARAM_COUNT
//              above 1e-12 at the shipped constants with the case's cycling
//              labels and K = 1; and the trace case's, every feature outside
//              the tables non-zero both ways over the extended
//              all_positions(). Either failing exits 1. Nothing about the
//              finite-difference check enters the rule (DEC-316's separation).
//
// WHAT IT DOES NOT CHECK, AND PHASE 1 DOES BY ITS OWN READING. New rows move
// what every precondition over the fixture reads: the clamp derivative's push
// and the gauge walk, the fixture clear of the clamp, the gradient case's worst
// relative error at h 0.01, and the planted recoveries of
// tests/test_tuner_gradient.cpp; the S124 step file records each.
//
// Built from the repository root after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -DCHESSO_SOURCE_DIR=\"$PWD\"
//       -I src -I tools -I tests -isystem tests/doctest/doctest -isystem
//       tests/third_party/nlohmann adocs/data/S124_coverage_rows.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S124_coverage_rows
//
//   S124_COVERAGE_DATA=<corpus> build/S124_coverage_rows
//
// It prints the open needs, each pick with the needs it closes, the checks,
// and the picks as C++ lines for tests/test_eval_positions.hpp.
#ifndef CHESSO_SOURCE_DIR
#error "build with -DCHESSO_SOURCE_DIR=\"<repository root>\", as the header says"
#endif
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "search_params.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
#include "tuner_layout.hpp"
#include "tuner_trace.hpp"

namespace
{

using tuner_layout::MOB_MG_BASE;
using tuner_layout::PARAM_COUNT;
using tuner_trace::dataset_t;

constexpr size_t PER_NEED = 64;
constexpr int SCORE_LIMIT = 800;  // centipawns, White relative, either way
constexpr size_t BASE_ROWS = 106;  // the fixture before S124's block
constexpr size_t CONSTRUCT_DRAWS = 400000;
constexpr uint32_t CONSTRUCT_SEED = 20261104U;
const char* const BLOCK_MARKER = "S124's endgame family, added at S124's";
// Where this block ends: a later block's marker, adocs/data/S124_gradient_rows.cpp's
// pair's if it lands (it did not at phase 1), or the list's end.
const char* const NEXT_MARKER = "DEC-316's pair at S124's landing";

game_t game;

// Mop-up's features in the trace's numbering, TERM_MOPUP's.
uint32_t first_feature()
{ return tuner_trace::feature_table().base[eval_trace::TERM_MOPUP]; }
uint32_t end_feature()
{ return tuner_trace::feature_table().base[eval_trace::TERM_MOPUP + 1]; }
size_t width() { return end_feature() - first_feature(); }

// The needs: mop-up's mg halves, its eg halves, then the thirteen cells.
size_t cell_need(size_t cell) { return 2 * width() + cell; }

// What one position offers: its phase, whether it and its mirror are
// eligible, the mop-up features its trace moves and the cell it picks.
struct offer_t
{
  std::string fen;
  std::string mirror;
  int phase = -1;
  bool eligible = false;
  std::vector<uint32_t> features;  // non-zero mop-up features, from 0
  uint8_t cell = eval_trace::NO_CELL;
};

// The cell a trace's E1 picks at `params`.
uint8_t picked(const tuner_trace::row_trace_t& trace, const double* params)
{
  dataset_t one;
  tuner_trace::append_row(trace, 0.5f, 0, 0, &one);
  return tuner_trace::smooth(tuner_trace::engine_model(), one, 0, params)
      .cell;
}

// S121_coverage_rows.cpp's, as S101's has it.
std::string canonical_castling(const std::string& fen)
{
  std::vector<std::string> parts = split_string(fen);
  if (parts.size() != 6 || parts[2] == "-") { return fen; }
  std::string rights;
  for (const char c : std::string("KQkq")) {
    if (parts[2].find(c) != std::string::npos) { rights += c; }
  }
  parts[2] = rights;
  std::string out;
  for (size_t i = 0; i < parts.size(); ++i) {
    out += (i ? " " : "") + parts[i];
  }
  return out;
}

// The tapered mobility-plus-king-safety sum at the shipped constants, as
// tests/test_tuner_gradient.cpp's stage_two() takes it.
double stage_two(const tuner_trace::row_trace_t& trace, const double* p)
{
  const std::vector<tuner_trace::feature_t>& features =
      tuner_trace::feature_table().features;
  const double mg_weight = trace.phase / static_cast<double>(GAME_PHASE_MAX);
  const double eg_weight = 1.0 - mg_weight;
  double sum = 0.0;

  for (const tuner_trace::entry_t& e : trace.entries) {
    const tuner_trace::feature_t& f = features[e.feature];
    if (!tuner_trace::clamped_site(f.site)) { continue; }
    sum += e.coef * (p[f.mg] * mg_weight + p[f.eg] * eg_weight);
  }

  // S122's curve, inside the clamp as the case's stage_two() has it since S122's
  // phase 1: each king's x per half, max(0, x)^2 / 2048 taken off its side.
  // Written on d83d6c0 this copy stopped above, which read the same while the
  // curve sat at its seed; S124's rebase onto S122's landing adds it, as S122's
  // phase 2 did for the other tools' copies (DEC-354).
  double x[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  for (const tuner_trace::side_entry_t& e : trace.side_entries) {
    if (e.group != 0) { continue; }
    x[e.side][0] += e.coef * p[tuner_layout::KSC_MG_BASE + e.input];
    x[e.side][1] += e.coef * p[tuner_layout::KSC_EG_BASE + e.input];
  }
  for (int side = 0; side < 2; ++side) {
    const double own = (side == 0) ? -1.0 : 1.0;
    const double m = std::max(x[side][0], 0.0);
    const double n = std::max(x[side][1], 0.0);
    sum += own * (m * m * mg_weight + n * n * eg_weight) / KS_CURVE_DIVISOR;
  }

  return sum;
}

std::string with_side_to_move(const std::string& fen, char side)
{
  std::string flipped = fen;
  flipped[fen.find(' ') + 1] = side;
  return flipped;
}

bool loads_either_side(const std::string& fen)
{
  return load_FEN(with_side_to_move(fen, 'w'), &game) &&
         load_FEN(with_side_to_move(fen, 'b'), &game);
}

bool trace_fen(const std::string& fen, tuner_trace::row_trace_t* trace)
{
  if (!load_FEN(fen, &game)) { return false; }
  std::string error;
  return tuner_trace::trace_board(&game.board, trace, &error);
}

offer_t offer_of(const std::string& fen, const double* params)
{
  offer_t out;
  out.fen = fen;
  out.mirror = canonical_castling(mirror_fen(fen));

  tuner_trace::row_trace_t trace;
  tuner_trace::row_trace_t mirrored;
  if (!trace_fen(fen, &trace) || !trace_fen(out.mirror, &mirrored)) {
    return out;
  }

  out.phase = trace.phase;
  out.cell = (trace.phase < GAME_PHASE_MAX) ? picked(trace, params)
                                            : eval_trace::NO_CELL;

  for (const tuner_trace::entry_t& e : trace.entries) {
    if (e.feature >= first_feature() && e.feature < end_feature() &&
        e.coef != 0) {
      out.features.push_back(e.feature - first_feature());
    }
  }

  out.eligible = out.phase >= 1 && out.phase <= GAME_PHASE_MAX - 1 &&
                 std::fabs(stage_two(trace, params)) < LAZY_EVAL_MARGIN - 1.0 &&
                 std::fabs(stage_two(mirrored, params)) < LAZY_EVAL_MARGIN - 1.0 &&
                 std::abs(trace.engine_white) <= SCORE_LIMIT &&
                 std::abs(mirrored.engine_white) <= SCORE_LIMIT &&
                 loads_either_side(fen) && loads_either_side(out.mirror);
  return out;
}

// The needs an offer's position holds: both halves of every mop-up feature
// it moves, its phase being 1 to 23, and the cell it picks.
std::vector<size_t> needs_of(const offer_t& offer)
{
  std::vector<size_t> out;
  for (const uint32_t f : offer.features) {
    out.push_back(f);
    out.push_back(width() + f);
  }
  if (offer.cell != eval_trace::NO_CELL) { out.push_back(cell_need(offer.cell)); }
  return out;
}

// The needs one offer closes among the open ones.
size_t closes(const offer_t& offer, const std::vector<uint8_t>& open)
{
  size_t n = 0;
  for (const size_t need : needs_of(offer)) {
    n += open[need];
  }
  return n;
}

// The parameter a need is, by name: tools/feature_audit's names.
std::string need_name(size_t need)
{
  if (need >= 2 * width()) {
    return "endgame_scale[" + std::to_string(need - 2 * width()) + "]";
  }
  const bool mg = need < width();
  return std::string(mg ? "mopup_mg[" : "mopup_eg[") +
         std::to_string(mg ? need : need - width()) + "]";
}

// Greedy over `pool`: the offer closing the most open needs, the earliest on
// a tie, until none closes any. Appends each pick and its mirror to `rows`.
void pick(const std::vector<offer_t>& pool,
          std::vector<uint8_t>* open,
          std::vector<std::string>* rows,
          std::vector<std::string>* lines)
{
  std::vector<uint8_t> used(pool.size(), 0);

  for (;;) {
    size_t best = pool.size();
    size_t best_n = 0;
    for (size_t i = 0; i < pool.size(); ++i) {
      if (used[i]) { continue; }
      const size_t n = closes(pool[i], *open);
      if (n > best_n) {
        best_n = n;
        best = i;
      }
    }
    if (best == pool.size()) { return; }

    used[best] = 1;
    std::string closed;
    for (const size_t need : needs_of(pool[best])) {
      if ((*open)[need]) {
        closed += " " + need_name(need);
        (*open)[need] = 0;
      }
    }

    printf("pick, phase %d, closes %zu:%s\n  %s\n", pool[best].phase, best_n,
           closed.c_str(), pool[best].fen.c_str());
    rows->push_back(pool[best].fen);
    rows->push_back(pool[best].mirror);
    lines->push_back("    // S124 coverage, closes" + closed);
    lines->push_back("    \"" + pool[best].fen + "\",");
    lines->push_back("    \"" + pool[best].mirror + "\",");
  }
}

// S121_coverage_rows.cpp's write_rows(), as it is there: the gradient case's
// fixture at phase_shift 0, labels cycling 1, 0.5, 0 and scores stepping by 41.
bool write_rows(const std::string& path, const std::vector<std::string>& fens)
{
  std::ofstream out(path);
  if (!out) { return false; }

  static const double results[3] = {1.0, 0.5, 0.0};
  int score = -280;

  for (size_t i = 0; i < fens.size(); ++i) {
    if (!load_FEN(fens[i], &game)) { return false; }
    out << fens[i] << '\t' << results[i % 3] << '\t' << score << '\t'
        << game_phase(&game.board) << '\n';
    score += 41;
    if (score > 300) { score -= 600; }
  }

  return out.good();
}

// The constructive fallback's draws, in draw order: two kings, 0 to 8 pawns a side
// and 0 to 2 pieces a kind a side, squares chosen by the generator's raw output
// modulo what is left, White to move. Placements load_FEN refuses are dropped here;
// the rest are offers like any other and pass `eligible` or not.
std::vector<std::string> constructed_draws()
{
  std::mt19937 rng(CONSTRUCT_SEED);
  std::vector<std::string> out;

  for (size_t draw = 0; draw < CONSTRUCT_DRAWS; ++draw) {
    std::vector<int> free_squares(64);
    for (int s = 0; s < 64; ++s) {
      free_squares[static_cast<size_t>(s)] = s;
    }
    char board[64] = {};

    const auto take = [&](bool pawn) {
      for (;;) {
        const size_t i = rng() % free_squares.size();
        const int s = free_squares[i];
        if (pawn && (s / 8 == 0 || s / 8 == 7)) { continue; }
        free_squares.erase(free_squares.begin() + static_cast<long>(i));
        return s;
      }
    };

    board[take(false)] = 'K';
    board[take(false)] = 'k';
    const int white_pawns = static_cast<int>(rng() % 9);
    const int black_pawns = static_cast<int>(rng() % 9);
    for (int i = 0; i < white_pawns; ++i) {
      board[take(true)] = 'P';
    }
    for (int i = 0; i < black_pawns; ++i) {
      board[take(true)] = 'p';
    }
    for (const char piece : {'N', 'B', 'R', 'Q', 'n', 'b', 'r', 'q'}) {
      const int wanted = static_cast<int>(rng() % 3);
      for (int i = 0; i < wanted; ++i) {
        board[take(false)] = piece;
      }
    }

    std::string fen;
    for (int row = 0; row < 8; ++row) {
      int empty = 0;
      for (int file = 0; file < 8; ++file) {
        const char c = board[row * 8 + file];
        if (c == '\0') {
          empty++;
          continue;
        }
        if (empty > 0) { fen += static_cast<char>('0' + empty); }
        empty = 0;
        fen += c;
      }
      if (empty > 0) { fen += static_cast<char>('0' + empty); }
      if (row < 7) { fen += '/'; }
    }
    fen += " w - - 0 1";

    if (load_FEN(fen, &game)) { out.push_back(fen); }
  }

  return out;
}

}  // namespace


TEST_CASE("S124: the fixture rows that move every endgame family parameter")
{
  initialize_game_const_data(&game);

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());


  const size_t needs = 2 * width() + tuner_layout::ENDGAME_SCALE_COUNT;

  // The base, asserted against the block's marker comment: the FEN lines
  // before it are BASE_ROWS, and the file's FEN lines are the list.
  const std::vector<std::string>& landed = test_eval_positions::positions;
  size_t block_end = 0;  // one past S124's block: the next marker, or the end
  {
    const char* path = std::getenv("S124_POSITIONS_HPP");
    std::ifstream header(path ? path
                              : CHESSO_SOURCE_DIR "/tests/test_eval_positions.hpp");
    REQUIRE_MESSAGE(header.good(), "tests/test_eval_positions.hpp");
    std::string line;
    size_t fens = 0;
    size_t before_marker = 0;
    bool marker = false;
    while (std::getline(header, line)) {
      if (line.find(BLOCK_MARKER) != std::string::npos) {
        REQUIRE_MESSAGE(!marker, "the block's marker twice");
        marker = true;
        before_marker = fens;
      }
      if (line.find(NEXT_MARKER) != std::string::npos) {
        REQUIRE_MESSAGE(block_end == 0, "the next rows' marker twice");
        block_end = fens;
      }
      const size_t open_quote = line.find_first_not_of(' ');
      if (open_quote != std::string::npos && line[open_quote] == '"' &&
          line.size() >= 2 && line.compare(line.size() - 2, 2, "\",") == 0) {
        fens++;
      }
    }
    if (!marker) { before_marker = fens; }
    if (block_end == 0) { block_end = fens; }
    printf("tests/test_eval_positions.hpp: %zu rows, %zu before S124's block "
           "(marker %s), the block ends at row %zu\n",
           fens, before_marker, marker ? "found" : "absent", block_end);
    REQUIRE(fens == landed.size());
    REQUIRE(before_marker == BASE_ROWS);
  }

  // Every landed row of the block held to `eligible` again, pair by pair.
  size_t ineligible = 0;
  for (size_t i = BASE_ROWS; i + 1 < block_end; i += 2) {
    const offer_t offer = offer_of(landed[i], params.data());
    if (!offer.eligible || offer.mirror != landed[i + 1]) {
      ineligible++;
      printf("INELIGIBLE landed pair %zu: %s\n", (i - BASE_ROWS) / 2 + 1,
             landed[i].c_str());
    }
  }
  REQUIRE((block_end - BASE_ROWS) % 2 == 0);
  printf("landed block: %zu pairs, %zu not eligible\n",
         (block_end - BASE_ROWS) / 2, ineligible);

  std::vector<std::string> rows(landed.begin(), landed.begin() + BASE_ROWS);
  std::vector<uint8_t> open(needs, 1);

  for (const std::string& fen : rows) {
    tuner_trace::row_trace_t trace;
    REQUIRE_MESSAGE(trace_fen(fen, &trace), fen);
    for (const tuner_trace::entry_t& e : trace.entries) {
      if (e.feature < first_feature() || e.feature >= end_feature() ||
          e.coef == 0) {
        continue;
      }
      const size_t f = e.feature - first_feature();
      if (trace.phase > 0) { open[f] = 0; }
      if (trace.phase < GAME_PHASE_MAX) { open[width() + f] = 0; }
    }
    if (trace.phase < GAME_PHASE_MAX) {
      const uint8_t cell = picked(trace, params.data());
      if (cell != eval_trace::NO_CELL) { open[cell_need(cell)] = 0; }
    }
  }

  const auto count_open = [&]() {
    size_t n = 0;
    for (const uint8_t o : open) {
      n += o;
    }
    return n;
  };

  printf("the list, %zu rows: %zu of %zu needs open\n", rows.size(),
         count_open(), needs);
  for (size_t need = 0; need < needs; ++need) {
    if (open[need]) { printf("  open %s\n", need_name(need).c_str()); }
  }

  const auto known = [&](const std::string& fen, const std::string& mirror) {
    for (const std::string& row : rows) {
      if (row == fen || row == mirror) { return true; }
    }
    return false;
  };

  std::vector<std::string> lines;

  // Stage one: the JSON suites' positions.
  {
    std::vector<offer_t> pool;
    for (const std::string& fen : all_test_fens()) {
      offer_t offer = offer_of(fen, params.data());
      if (!offer.eligible || known(offer.fen, offer.mirror)) { continue; }
      if (closes(offer, open) > 0) { pool.push_back(offer); }
    }
    printf("all_test_fens(): %zu eligible candidates close an open need\n",
           pool.size());
    pick(pool, &open, &rows, &lines);
  }

  // Stages two and three: the corpus, then the constructed draws, each for
  // what the stages before it left open, the first PER_NEED eligible
  // candidates per open need kept.
  // Each reads its source in order and stops once every open need has its
  // PER_NEED candidates, after which no later row could be kept; a row is
  // traced alone first, and its mirror and eligibility read only when it
  // holds a need still wanted. Neither changes what is kept: the rule's.
  const auto from_source = [&](const std::function<bool(std::string*)>& next,
                               const char* what) {
    std::vector<size_t> kept(needs, 0);
    std::vector<offer_t> pool_here;
    size_t read = 0;
    std::string fen;

    const auto satisfied = [&]() {
      for (size_t need = 0; need < needs; ++need) {
        if (open[need] && kept[need] < PER_NEED) { return false; }
      }
      return true;
    };

    while (!satisfied() && next(&fen)) {
      read++;

      offer_t quick;
      tuner_trace::row_trace_t trace;
      if (!trace_fen(fen, &trace)) { continue; }
      quick.phase = trace.phase;
      quick.cell = (trace.phase < GAME_PHASE_MAX) ? picked(trace, params.data())
                                                  : eval_trace::NO_CELL;
      for (const tuner_trace::entry_t& e : trace.entries) {
        if (e.feature >= first_feature() && e.feature < end_feature() &&
            e.coef != 0) {
          quick.features.push_back(e.feature - first_feature());
        }
      }
      bool wanted = false;
      for (const size_t need : needs_of(quick)) {
        if (open[need] && kept[need] < PER_NEED) { wanted = true; }
      }
      if (!wanted) { continue; }

      offer_t offer = offer_of(fen, params.data());
      if (!offer.eligible || closes(offer, open) == 0) { continue; }
      if (known(offer.fen, offer.mirror)) { continue; }

      for (const size_t need : needs_of(offer)) {
        if (open[need]) { kept[need]++; }
      }
      pool_here.push_back(offer);
    }

    printf("%s: %zu read, %zu candidates kept\n", what, read,
           pool_here.size());
    pick(pool_here, &open, &rows, &lines);
  };

  const char* data = std::getenv("S124_COVERAGE_DATA");
  if (count_open() > 0 && data != nullptr) {
    std::ifstream file(data);
    REQUIRE_MESSAGE(file.good(), data);
    from_source(
        [&](std::string* out) {
          std::string line;
          if (!std::getline(file, line)) { return false; }
          *out = line.substr(0, line.find('\t'));
          return true;
        },
        data);
  }

  if (count_open() > 0) {
    const std::vector<std::string> draws = constructed_draws();
    size_t at = 0;
    from_source(
        [&](std::string* out) {
          if (at >= draws.size()) { return false; }
          *out = draws[at++];
          return true;
        },
        "the constructive fallback");
  }

  bool ok = true;
  if (count_open() > 0) {
    ok = false;
    for (size_t need = 0; need < needs; ++need) {
      if (open[need]) {
        printf("UNCLOSED %s: no eligible candidate moves it; phase 1 stops "
               "for an owner ruling\n",
               need_name(need).c_str());
      }
    }
  }

  // Check one: the gradient case's non-vacuity over the extended list, its
  // span, MOB_MG_BASE to PARAM_COUNT.
  const std::string path =
      (std::filesystem::temp_directory_path() / "chesso_S124_coverage.tsv")
          .string();
  dataset_t fixture;
  tuner_trace::load_report_t report;
  std::string error;
  REQUIRE(write_rows(path, rows));
  REQUIRE(tuner_trace::load(path, &fixture, &report, &error, 1));
  REQUIRE(report.mismatches == 0);
  std::filesystem::remove(path);

  std::vector<uint32_t> index(fixture.size());
  std::vector<double> target(fixture.size());
  for (size_t i = 0; i < fixture.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
    target[i] = fixture.result[i];
  }
  std::vector<double> analytic;
  tuner_trace::gradient(fixture, params.data(), 1.0, index, 0, fixture.size(),
                        1, target, &analytic);

  size_t flat = 0;
  double smallest = INFINITY;
  double smallest_family = INFINITY;
  for (size_t p = MOB_MG_BASE; p < PARAM_COUNT; ++p) {
    // S122's curve's constant: exactly zero at its seed by construction
    // (tests/test_tuner_gradient.cpp says why), asserted there apart.
    if (p == tuner_layout::KSC_MG_BASE + KSC_CONSTANT ||
        p == tuner_layout::KSC_EG_BASE + KSC_CONSTANT) {
      continue;
    }
    smallest = std::min(smallest, std::fabs(analytic[p]));
    if (p >= tuner_layout::MOP_MG_BASE) {
      smallest_family = std::min(smallest_family, std::fabs(analytic[p]));
    }
    if (!(std::fabs(analytic[p]) > 1e-12)) {
      flat++;
      printf("FLAT parameter %zu: |gradient| %.3e\n", p, std::fabs(analytic[p]));
    }
  }
  printf("check gradient: %zu rows, %zu parameters at or under 1e-12, the "
         "smallest |gradient| from MOB_MG_BASE on %.3e, over S124's family "
         "%.3e\n",
         fixture.size(), flat, smallest, smallest_family);
  if (flat > 0) { ok = false; }

  // Check two: the trace case's both-ways coverage over all_positions(),
  // every term but the tables.
  const tuner_trace::feature_table_t& table = tuner_trace::feature_table();
  std::vector<int> positive(table.features.size(), 0);
  std::vector<int> negative(table.features.size(), 0);
  std::vector<std::string> everything;
  for (const std::string& fen : rows) {
    everything.push_back(fen);
    everything.push_back(mirror_fen(fen));
  }
  for (const std::string& fen : all_test_fens()) {
    everything.push_back(fen);
  }
  for (const std::string& fen : everything) {
    tuner_trace::row_trace_t trace;
    REQUIRE_MESSAGE(trace_fen(fen, &trace), fen);
    for (const tuner_trace::entry_t& e : trace.entries) {
      if (e.coef > 0) { positive[e.feature]++; }
      if (e.coef < 0) { negative[e.feature]++; }
    }
  }
  // Every term but the tables.
  size_t one_way = 0;
  for (int t = 0; t < eval_trace::TERM_COUNT; ++t) {
    if (t == eval_trace::TERM_PSQT) { continue; }
    for (uint32_t f = table.base[t]; f < table.base[t + 1]; ++f) {
      if (positive[f] == 0 || negative[f] == 0) {
        one_way++;
        printf("ONE-WAY term %d index %u: %d positive, %d negative\n", t,
               f - table.base[t], positive[f], negative[f]);
      }
    }
  }
  printf("check both ways: %zu positions, %zu features not moved both ways\n",
         everything.size(), one_way);
  if (one_way > 0) { ok = false; }

  // The picks against the landed block: the same pairs in the same order.
  const std::vector<std::string> block(landed.begin() + BASE_ROWS,
                                       landed.begin() + block_end);
  const std::vector<std::string> picks(rows.begin() + BASE_ROWS, rows.end());
  const bool reproduces = (block == picks);
  printf("reproduces the landed block: %s (%zu picked rows, %zu landed)\n",
         reproduces ? "yes, pair for pair and in order" : "NO", picks.size(),
         block.size());
  if (!block.empty() && !reproduces) { ok = false; }
  if (ineligible > 0) { ok = false; }

  printf("\n%zu rows the rule picks, for tests/test_eval_positions.hpp:\n",
         rows.size() - BASE_ROWS);
  for (const std::string& l : lines) {
    printf("%s\n", l.c_str());
  }

  CHECK(ok);
}
