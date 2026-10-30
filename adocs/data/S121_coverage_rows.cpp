// S121, DEC-326 (6). The rows tests/test_eval_positions.hpp gains so that
// every cell of S121's mobility table is moved by the fixture, found by the
// rule below and never by hand. Written 2026-10-28 in S121's phase 1a, before
// it ran; it runs in phase 1b on the tree the table lands in, and again on
// any later tree whose counts move (a part of the screen that lands, a refit
// that moves a precondition).
//
// WHAT IT ANSWERS. Two cases read every non-table column of the fixture and
// would read S121's 132 mobility parameters red where the forty-two rows never
// put a piece on a cell -- a queen on its 27th square, a rook on its 14th, a
// knight with no square at all:
//
//   tests/test_eval_trace.cpp "the positions move every term both ways": every
//     feature outside the piece-square tables non-zero both ways over
//     all_positions() (the list, each row's colour-mirror, all_test_fens());
//   tests/test_tuner_gradient.cpp "gradient() is the derivative of the error
//     it descends": every parameter from MOB_MG_BASE to PARAM_COUNT with
//     |analytic gradient| > 1e-12 over the list alone, the case's labels.
//
// Neither case changes. The answer is rows, S100's and DEC-316's precedent.
//
// IT RE-DERIVES ITS OWN BLOCK (DEC-142). The list it starts from is the
// fixture's first BASE_ROWS = 42 rows, the rows before S121's block, and not
// the landed list: the count is asserted against the block's marker comment
// in tests/test_eval_positions.hpp ("S121's mobility table, added at S121's
// phase 1b"), read from S121_POSITIONS_HPP (default
// ../../tests/test_eval_positions.hpp, from build/tests). Every row of the
// landed block is held to `eligible` again, each pair as the position and its
// mirror; and the picks made from the base must be the landed block, pair for
// pair and in order, or the program exits 1. The checks below then read the
// base with the picks, which is the landed list. Since 2026-10-29, after the
// phase 1b fast check: until then it started from the landed 80 rows, found
// no open need, and never re-checked the rows it had added.
//
// THE RULE.
//   needs      a (cell, half) is open when no row of the list has a non-zero
//              coefficient on the cell at a phase that gives the half a
//              taper weight: phase > 0 for the middlegame half, < 24 for
//              the endgame one.
//   candidates all_test_fens() in that function's sorted order; then, only
//              for the needs still open, the rows of the file named by
//              S121_COVERAGE_DATA (the coordinator names S083's corpus), in
//              file order, the first 64 eligible rows per open need kept;
//              then, only for what is still open, the positions of the file
//              named by S121_COVERAGE_CONSTRUCTED, written by
//              adocs/data/S121_construct_rows.py for the cells this program
//              printed UNCLOSED without it (the constructive fallback, a
//              deterministic search over legal positions with the cell's
//              count, each `is_valid()` by python-chess), in file order and
//              kept the same way. A candidate the list already holds, either
//              way round, is skipped.
//   eligible   load_FEN accepts it and its colour-mirror; its phase is 1 to
//              23, so a row that closes a cell closes both of its halves;
//              and it and its mirror each sit more than a centipawn inside
//              the lazy clamp at the shipped constants, which "the fixture
//              sits clear of the clamp at the shipped constants" asserts of
//              every row of the list; and, since 2026-10-29, it and its
//              mirror each score within SCORE_LIMIT = 800 cp of level at the
//              shipped constants. Amended after the first run, before any
//              pick was kept: it closed every need, and its own gradient
//              check then read four parameters at or under 1e-12 -- knight
//              count 1 and bishop count 11, both halves, at 2.8e-19 and
//              1.2e-14 -- because the pairs that closed them were lopsided
//              (a full White army against a bare king, nine White bishops),
//              where the case's sigmoid at K = 1 is flat and a row moves no
//              gradient. At 800 cp its slope is still about 0.0099 of its
//              peak's 0.25 per unit of K * score * ln 10 / 400. And, the
//              second amendment of 2026-10-29, after the second run's picks
//              were appended and before they were kept: it and its mirror
//              each load with either side to move, the FEN's second field
//              flipped and nothing else, which tests/test_eval_model.cpp "the
//              move is worth the same to either side" requires of every row
//              of the list (REQUIRE on load_FEN); the second run's rook-13
//              pick, 1k3R2/8/8/8/8/8/8/4K3 b, has Black in check and refused
//              to load with White to move.
//   pick       greedy, all_test_fens()'s candidates first: the eligible
//              candidate closing the most open needs, the earliest on a tie,
//              appended as the list's convention has it -- the position, then
//              its colour-mirror -- until no need is open; then the corpus's
//              candidates the same way, then the constructed ones. Every
//              candidate, constructed or not, is held to `eligible` by this
//              program's own trace of it, so a count the generator got wrong
//              closes nothing. A need no candidate closes is printed UNCLOSED
//              and the program exits 1: run the generator for those cells
//              and this again with S121_COVERAGE_CONSTRUCTED; a need still
//              unclosed then stops phase 1b for an owner ruling, never a row
//              made up to close it.
//   checks     over the list with the picks appended: the gradient case's
//              non-vacuity, every parameter from MOB_MG_BASE to PARAM_COUNT
//              above 1e-12 at the shipped constants with the case's cycling
//              labels and K = 1; and the trace case's, every feature outside
//              the tables non-zero both ways over the extended all_positions().
//              Either failing exits 1. Nothing about the finite-difference
//              check enters the rule, so the picks are not chosen by the
//              gradient case passing (DEC-316's same separation).
//
// WHAT IT DOES NOT CHECK, AND PHASE 1B DOES BY ITS OWN SCRIPT. New rows move
// what every precondition over the fixture reads: the clamp binding both ways
// in "the trace reconstructs evaluate() exactly" (positions, not the fixture),
// the clamp derivative's push and the gauge walk's floor
// (adocs/data/S083_tuner_preconditions.cpp, `both`), the gradient case's
// worst relative error at h 0.01 (DEC-316: a near-stationary parameter is
// answered by rows, S135_gradient_rows.cpp's rule retargeted), and the planted
// recoveries of tests/test_tuner_gradient.cpp, which read the fixture too.
//
// Built from the repository root after `cmake --build build`, in one line:
//
//   g++ -O2 -std=gnu++20 -march=native -DCHESSO_TRACE -I src -I tools
//       -I tests -isystem tests/doctest/doctest -isystem
//       tests/third_party/nlohmann adocs/data/S121_coverage_rows.cpp
//       build/src/libchesso_engine_trace.a -lpthread
//       -o build/S121_coverage_rows
//
// and run from build/tests, where the JSON suites' relative paths resolve:
//
//   (cd build/tests && S121_COVERAGE_DATA=<corpus> ../S121_coverage_rows)
//
// It prints the open needs, each pick with the needs it closes, the two
// checks, and the picks as C++ lines for tests/test_eval_positions.hpp.
//
// SCOPED TO ITS OWN BLOCK AND SPAN, 2026-10-30 (S123's phase 1, DEC-329 (8)).
// S123 appended its passed pawn family after tempo and its own block of rows
// after this one, picked by adocs/data/S123_coverage_rows.cpp. Read on that
// tree as written, this program would hold S123's 58 parameters to its
// gradient check over its own 80 rows, S123's terms to its both-ways check,
// and S123's rows to its "landed block". So: the gradient check reads the
// mobility span its rule was written for, [MOB_MG_BASE, KS_MG_BASE); the
// both-ways check reads the terms that existed when it was written, every
// term before S123's TERM_PASSED_FREE but the tables; and the landed block
// ends at S123's block marker when the header has one. Its rule, its picks
// and its block are unchanged, and it must still reproduce the block.
// S123's program checks S123's span, terms and block.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

constexpr size_t NEEDS = 2 * static_cast<size_t>(MOBILITY_CELLS);
constexpr size_t PER_NEED = 64;
constexpr int SCORE_LIMIT = 800;  // centipawns, White relative, either way
constexpr size_t BASE_ROWS = 42;  // the fixture before S121's block
const char* const BLOCK_MARKER = "S121's mobility table, added at S121's";
// Where this block ends, since S123: the next block's marker.
const char* const NEXT_MARKER = "S123's passed pawn family, added at S123's";

game_t game;

// What one position offers: its phase, whether it and its mirror are
// eligible, and the cells its trace moves.
struct offer_t
{
  std::string fen;
  std::string mirror;
  int phase = -1;
  bool eligible = false;
  std::vector<int> cells;  // non-zero mobility cells
};

// mirror_fen() swaps the castling letters' case in place, so "KQk" comes back
// as "kqK"; the parser reads either, and the list writes White's rights first.
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
// tests/test_tuner_gradient.cpp's stage_two() takes it, for one traced row.
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

  return sum;
}

// The FEN with `side` to move and nothing else changed:
// tests/test_eval_model.cpp's with_side_to_move().
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
  const uint32_t first =
      tuner_trace::feature_table().base[eval_trace::TERM_MOBILITY];

  for (const tuner_trace::entry_t& e : trace.entries) {
    if (e.feature >= first &&
        e.feature < first + static_cast<uint32_t>(MOBILITY_CELLS) &&
        e.coef != 0) {
      out.cells.push_back(static_cast<int>(e.feature - first));
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

// The needs one offer closes among the open ones: both halves of every cell
// it moves, its phase being 1 to 23.
size_t closes(const offer_t& offer, const std::vector<uint8_t>& open)
{
  size_t n = 0;
  for (const int cell : offer.cells) {
    n += open[static_cast<size_t>(cell)];
    n += open[static_cast<size_t>(MOBILITY_CELLS + cell)];
  }
  return n;
}

std::string need_name(size_t need)
{
  static const char* const pieces[4] = {"knight", "bishop", "rook", "queen"};
  const bool mg = need < static_cast<size_t>(MOBILITY_CELLS);
  const int cell = static_cast<int>(mg ? need : need - MOBILITY_CELLS);
  int type = 3;
  while (cell < MOBILITY_OFFSET[type]) {
    type--;
  }
  return std::string("mobility_") + pieces[type] + (mg ? "_mg[" : "_eg[") +
         std::to_string(cell - MOBILITY_OFFSET[type]) + "]";
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
    for (const int cell : pool[best].cells) {
      for (const size_t need : {static_cast<size_t>(cell),
                                static_cast<size_t>(MOBILITY_CELLS + cell)}) {
        if ((*open)[need]) {
          closed += " " + need_name(need);
          (*open)[need] = 0;
        }
      }
    }

    printf("pick, phase %d, closes %zu:%s\n  %s\n", pool[best].phase, best_n,
           closed.c_str(), pool[best].fen.c_str());
    rows->push_back(pool[best].fen);
    rows->push_back(pool[best].mirror);
    lines->push_back("    // S121 coverage, closes" + closed);
    lines->push_back("    \"" + pool[best].fen + "\",");
    lines->push_back("    \"" + pool[best].mirror + "\",");
  }
}

// tests/test_tuner_gradient.cpp's write_fixture() at phase_shift 0, over any
// list: the labels cycle 1, 0.5, 0 by index and the scores step by 41.
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

}  // namespace


TEST_CASE("S121: the fixture rows that move every mobility cell")
{
  initialize_game_const_data(&game);

  std::vector<double> params(PARAM_COUNT, 0.0);
  tuner_layout::starting_params(params.data());

  // The base, asserted against the block's marker comment: the FEN lines
  // before it are BASE_ROWS, and the file's FEN lines are the list.
  const std::vector<std::string>& landed = test_eval_positions::positions;
  size_t block_end = 0;  // one past S121's block: S123's marker, or the end
  {
    const char* path = std::getenv("S121_POSITIONS_HPP");
    std::ifstream header(path ? path : "../../tests/test_eval_positions.hpp");
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
        REQUIRE_MESSAGE(block_end == 0, "S123's marker twice");
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
    printf("tests/test_eval_positions.hpp: %zu rows, %zu before S121's block "
           "(marker %s), the block ends at row %zu\n",
           fens, before_marker, marker ? "found" : "absent", block_end);
    REQUIRE(fens == landed.size());
    REQUIRE(before_marker == BASE_ROWS);
    REQUIRE(block_end >= BASE_ROWS);
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
  std::vector<uint8_t> open(NEEDS, 1);

  for (const std::string& fen : rows) {
    tuner_trace::row_trace_t trace;
    REQUIRE_MESSAGE(trace_fen(fen, &trace), fen);
    const uint32_t first =
        tuner_trace::feature_table().base[eval_trace::TERM_MOBILITY];
    for (const tuner_trace::entry_t& e : trace.entries) {
      if (e.feature < first ||
          e.feature >= first + static_cast<uint32_t>(MOBILITY_CELLS) ||
          e.coef == 0) {
        continue;
      }
      const size_t cell = e.feature - first;
      if (trace.phase > 0) { open[cell] = 0; }
      if (trace.phase < GAME_PHASE_MAX) { open[MOBILITY_CELLS + cell] = 0; }
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
         count_open(), NEEDS);
  for (size_t need = 0; need < NEEDS; ++need) {
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
  std::vector<offer_t> pool;
  for (const std::string& fen : all_test_fens()) {
    offer_t offer = offer_of(fen, params.data());
    if (!offer.eligible || known(offer.fen, offer.mirror)) { continue; }
    if (closes(offer, open) > 0) { pool.push_back(offer); }
  }
  printf("all_test_fens(): %zu eligible candidates close an open need\n",
         pool.size());
  pick(pool, &open, &rows, &lines);

  // Stages two and three: the corpus, then the constructed positions, each
  // for what the stages before it left open.
  const auto from_file = [&](const char* path) {
    std::ifstream file(path);
    REQUIRE_MESSAGE(file.good(), path);
    std::vector<size_t> kept(NEEDS, 0);
    std::vector<offer_t> pool_here;
    std::string line;
    size_t read = 0;

    while (std::getline(file, line)) {
      read++;
      const std::string fen = line.substr(0, line.find('\t'));
      offer_t offer = offer_of(fen, params.data());
      if (!offer.eligible || closes(offer, open) == 0) { continue; }
      if (known(offer.fen, offer.mirror)) { continue; }

      bool wanted = false;
      for (const int cell : offer.cells) {
        for (const size_t need : {static_cast<size_t>(cell),
                                  static_cast<size_t>(MOBILITY_CELLS + cell)}) {
          if (open[need] && kept[need] < PER_NEED) { wanted = true; }
        }
      }
      if (!wanted) { continue; }

      for (const int cell : offer.cells) {
        for (const size_t need : {static_cast<size_t>(cell),
                                  static_cast<size_t>(MOBILITY_CELLS + cell)}) {
          if (open[need]) { kept[need]++; }
        }
      }
      pool_here.push_back(offer);
    }

    printf("%s: %zu rows read, %zu candidates kept\n", path, read,
           pool_here.size());
    pick(pool_here, &open, &rows, &lines);
  };

  const char* data = std::getenv("S121_COVERAGE_DATA");
  if (count_open() > 0 && data != nullptr) { from_file(data); }

  const char* constructed = std::getenv("S121_COVERAGE_CONSTRUCTED");
  if (count_open() > 0 && constructed != nullptr) { from_file(constructed); }

  bool ok = true;
  if (count_open() > 0) {
    ok = false;
    for (size_t need = 0; need < NEEDS; ++need) {
      if (open[need]) {
        printf("UNCLOSED %s: no eligible candidate moves it; next, "
               "adocs/data/S121_construct_rows.py for it\n",
               need_name(need).c_str());
      }
    }
  }

  // Check one: the gradient case's non-vacuity over the extended list.
  const std::string path =
      (std::filesystem::temp_directory_path() / "chesso_S121_coverage.tsv")
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

  // The mobility span, since S123 (the header says why).
  size_t flat = 0;
  double smallest = INFINITY;
  for (size_t p = MOB_MG_BASE; p < tuner_layout::KS_MG_BASE; ++p) {
    smallest = std::min(smallest, std::fabs(analytic[p]));
    if (!(std::fabs(analytic[p]) > 1e-12)) {
      flat++;
      printf("FLAT parameter %zu: |gradient| %.3e\n", p, std::fabs(analytic[p]));
    }
  }
  printf("check gradient: %zu rows, %zu parameters at or under 1e-12, the "
         "smallest |gradient| over the mobility span %.3e\n",
         fixture.size(), flat, smallest);
  if (flat > 0) { ok = false; }

  // Check two: the trace case's both-ways coverage over all_positions().
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
  // The terms before S123's, since S123 (the header says why).
  size_t one_way = 0;
  for (int t = 0; t < eval_trace::TERM_PASSED_FREE; ++t) {
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
  const std::vector<std::string> block(
      landed.begin() + BASE_ROWS,
      landed.begin() + static_cast<std::ptrdiff_t>(block_end));
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
