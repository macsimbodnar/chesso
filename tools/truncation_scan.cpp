// Finds the positions that make evaluate()'s taper truncate hardest. S076.
//
//   build/tools/truncation_scan --data .tuning/selfplay_v2_dedup.tsv --min 2.8
//
// `tests/test_eval_model.cpp` pins four positions whose float model and integer
// engine scores disagree by 69/24 = 2.875, the arithmetic maximum of three
// truncating divisions, so that the tolerance in "the model reproduces
// evaluate() on every phase" is asserted against a corpus that actually reaches
// it. Its own comment states the rule this tool serves: **a residual belongs to
// the weights, not to the position**, so the list is re-measured whenever the
// constants are refitted rather than carried forward. DEC-057.
//
// It had been re-measured twice by hand, at S038 and at S065, each time from a
// script in a session scratchpad that no longer exists -- the shape
// `2026-08-16_plan_review-F04` found. S076 refits again, so the third
// re-measurement is a tracked tool.
//
// The model here is `model_score_white()` from that test file, without doctest:
// the same `eval_model` calls in the same order, against
// `eval_model::starting_params`, which is what the engine ships.
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "eval_model.hpp"
#include "evaluation.hpp"

namespace
{

struct options_t
{
  std::string data;
  double min = 2.8;
  uint64_t limit = 0;   // 0 scans everything
  uint64_t report = 0;  // 0 prints every hit
};


// The FEN of a `fen result score phase` row, or the whole line when it carries
// no tab -- so a plain list of FENs scans too.
std::string row_fen(const std::string& line)
{
  const size_t tab = line.find('\t');
  return (tab == std::string::npos) ? line : line.substr(0, tab);
}


std::string side_to_move_of(const std::string& fen)
{
  std::istringstream fields(fen);
  std::string placement, side;

  fields >> placement >> side;
  return side;
}


// tests/test_eval_model.cpp:46-76, with the REQUIREs turned into a false
// return. Any disagreement between the two is a bug in this file, not a
// measurement.
bool model_score_white(const std::string& fen,
                       const double* params,
                       double* score,
                       int* phase_out)
{
  std::vector<uint16_t> pieces;
  const std::string placement = fen.substr(0, fen.find(' '));

  if (!eval_model::parse_placement(placement, &pieces)) { return false; }

  const int phase = eval_model::phase_of(pieces.data(), pieces.size());

  int mobility[4] = {0, 0, 0, 0};
  if (!eval_model::mobility_features(placement, mobility)) { return false; }

  int king_safety[eval_model::KING_SAFETY_COUNT] = {};
  if (!eval_model::king_safety_features(placement, king_safety)) {
    return false;
  }

  int passed_pawn[eval_model::PASSED_PAWN_COUNT] = {};
  if (!eval_model::passed_pawn_features(placement, passed_pawn)) {
    return false;
  }

  int pawn_structure[eval_model::PAWN_STRUCTURE_COUNT] = {};
  if (!eval_model::pawn_structure_features(placement, pawn_structure)) {
    return false;
  }

  int piece_placement[eval_model::PIECE_PLACEMENT_COUNT] = {};
  if (!eval_model::piece_placement_features(placement, piece_placement)) {
    return false;
  }

  int tempo = 0;
  if (!eval_model::tempo_feature(side_to_move_of(fen), &tempo)) {
    return false;
  }

  *phase_out = phase;
  *score = eval_model::evaluate(pieces.data(), pieces.size(), phase, mobility,
                                params, king_safety, passed_pawn,
                                pawn_structure, piece_placement, tempo);
  return true;
}


void usage()
{
  fprintf(stderr,
          "truncation_scan --data FILE [options]\n"
          "  --data FILE   a `fen result score phase` corpus, or one FEN per\n"
          "                line\n"
          "  --min VALUE   print positions disagreeing by more than this\n"
          "                (default 2.8; the maximum is 69/24 = 2.875)\n"
          "  --limit N     stop after N rows (default 0, everything)\n"
          "  --report N    print at most N hits (default 0, all of them)\n");
}

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "--help") {
      usage();
      return 0;
    }

    if (i + 1 >= argc) {
      usage();
      return 1;
    }

    const std::string value = argv[++i];

    if (arg == "--data") {
      opts.data = value;
    } else if (arg == "--min") {
      opts.min = atof(value.c_str());
    } else if (arg == "--limit") {
      opts.limit = strtoull(value.c_str(), nullptr, 10);
    } else if (arg == "--report") {
      opts.report = strtoull(value.c_str(), nullptr, 10);
    } else {
      usage();
      return 1;
    }
  }

  if (opts.data.empty()) {
    usage();
    return 1;
  }

  std::ifstream in(opts.data);

  if (!in) {
    fprintf(stderr, "cannot read %s\n", opts.data.c_str());
    return 1;
  }

  game_t game;
  initialize_game_const_data(&game);

  std::vector<double> params(eval_model::PARAM_COUNT, 0.0);
  eval_model::starting_params(params.data());

  std::string line;
  uint64_t scanned = 0;
  uint64_t past_two = 0;
  uint64_t past_min = 0;
  uint64_t printed = 0;
  double worst = 0.0;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }
    if (opts.limit > 0 && scanned >= opts.limit) { break; }

    const std::string fen = row_fen(line);

    double model = 0.0;
    int phase = 0;

    if (!model_score_white(fen, params.data(), &model, &phase)) {
      fprintf(stderr, "unusable FEN: %s\n", fen.c_str());
      return 1;
    }

    if (!load_FEN(fen, &game)) {
      fprintf(stderr, "unloadable FEN: %s\n", fen.c_str());
      return 1;
    }

    const int engine_relative = evaluate(&game.board);
    const int engine_white =
        (game.board.active_color == WHITE) ? engine_relative : -engine_relative;

    const double difference =
        std::abs(model - static_cast<double>(engine_white));

    scanned++;
    worst = std::max(worst, difference);

    if (difference > 2.0) { past_two++; }

    if (difference > opts.min) {
      past_min++;

      if (opts.report == 0 || printed < opts.report) {
        printed++;
        printf("%.6f\tphase %2d\t%s\t%s\n", difference, phase,
               side_to_move_of(fen).c_str(), fen.c_str());
      }
    }
  }

  fprintf(stderr,
          "%" PRIu64 " rows scanned, %" PRIu64 " past 2.0, %" PRIu64
          " past %.4f, worst %.6f\n",
          scanned, past_two, past_min, opts.min, worst);

  return 0;
}
