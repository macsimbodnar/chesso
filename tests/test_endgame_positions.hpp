#pragma once
// S124's endgame population (DEC-350 (6)): seeded random placements of the
// material signatures S124's cells and mop-up's domain are defined over, so
// the engine's counts, the trace's factor and the tuner's product are each
// read on every cell and every mop-up count, for each colour. A random
// middlegame placement reaches almost none of them, which is why S101's
// population is not enough here.
//
// The signatures are counts of men and nothing else, written from the
// definitions in src/evaluation.hpp: each cell's P and d from both sides'
// point of view, both bishop colourings, the gate's two sides, mop-up's
// domain and its exclusions. No placement is judged; a placement load_FEN()
// refuses -- a king in contact, the side not to move in check -- is skipped.
// Each signature is placed as written and with the colours swapped, and every
// kept placement is kept with White and with Black to move.
#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"

namespace test_endgame_positions
{

// White's men, then Black's, beside the kings: P N B R Q.
struct signature_t
{
  const char* white;
  const char* black;
};

inline const signature_t signatures[] = {
    // PAWNS, P 0 to 2 against d below 0 to past 2, either side strong.
    {"R", "B"},
    {"R", "R"},
    {"Q", "B"},
    {"Q", "R"},
    {"RB", "R"},
    {"RN", "B"},
    {"BB", "N"},
    {"RP", "B"},
    {"RP", "R"},
    {"RP", "N"},
    {"QP", "R"},
    {"RBP", "R"},
    {"RPP", "B"},
    {"RPP", "R"},
    {"QPP", "R"},
    {"RBPP", "R"},
    {"BPP", "N"},
    {"NP", "B"},
    {"P", "P"},
    {"PP", "P"},
    {"RPP", "RP"},
    {"RPPP", "RPP"},
    {"QRPPP", "QRPPP"},
    // OCB, with the colours random: PURE by P, ROOK, and the other.
    {"BP", "B"},
    {"BPP", "BP"},
    {"BPPP", "B"},
    {"BPPPP", "BPP"},
    {"BPPPPP", "BP"},
    {"BPPPPPP", "BPPPP"},
    {"RBP", "RB"},
    {"RBPPP", "RBPP"},
    {"RBPPPPP", "RB"},
    {"NBP", "B"},
    {"RBP", "NB"},
    {"QBPP", "QB"},
    {"RRBP", "RB"},
    // LONE, mop-up's domain and its exclusions.
    {"R", ""},
    {"Q", ""},
    {"BN", ""},
    {"BB", ""},
    {"RB", ""},
    {"RN", ""},
    {"QR", ""},
    {"NN", ""},
    {"B", ""},
    {"N", ""},
    {"P", ""},
    {"RP", ""},
    {"", ""},
};


inline char piece_char(char c, bool white)
{ return white ? c : static_cast<char>(c - 'A' + 'a'); }


// S341. The shuffle placements() draws its squares with: Fisher and Yates', in
// Durstenfeld's form, over the generator's raw output. [rand.predef] fixes what
// std::mt19937 returns from a seed; std::shuffle's algorithm is each standard
// library's own, and until S341 placements() called it: from this seed
// libstdc++ drew 5719 rows at 40 and libc++ 5696, other positions, so DEC-356's
// pair, picked on libstdc++'s, did not hold under clang. The index is the raw
// value modulo the span left; its bias, under 64 in 2^32, is a fixture's.
inline void shuffle_squares(std::vector<int>* order, std::mt19937* rng)
{
  for (size_t i = order->size() - 1; i > 0; --i) {
    const size_t j = static_cast<size_t>((*rng)() % (i + 1));
    std::swap((*order)[i], (*order)[j]);
  }
}


// `per_signature` placements a signature and colouring, each kept with both
// sides to move where load_FEN() takes it: the generator, population() below
// without DEC-356's rows. adocs/data/S124_factor_gradient_rows.cpp reads it
// as the rule's base at 40 and as its candidates at 80.
inline std::vector<std::string> placements(int per_signature = 40)
{
  std::vector<std::string> out;
  std::mt19937 rng(20261104U);
  game_t game;
  initialize_game_const_data(&game);

  for (const signature_t& sig : signatures) {
    for (int swap = 0; swap < 2; ++swap) {
      const std::string white = swap ? sig.black : sig.white;
      const std::string black = swap ? sig.white : sig.black;

      for (int trial = 0; trial < per_signature; ++trial) {
        std::vector<int> order(64);
        for (int square = 0; square < 64; ++square) {
          order[static_cast<size_t>(square)] = square;
        }
        shuffle_squares(&order, &rng);

        char squares[64] = {};
        size_t next = 0;
        squares[order[next++]] = 'K';
        squares[order[next++]] = 'k';

        for (int side = 0; side < 2; ++side) {
          for (const char man : (side == 0) ? white : black) {
            while (next < 64) {
              const int square = order[next++];
              const int row = square / 8;
              if (man == 'P' && (row == 0 || row == 7)) { continue; }
              squares[square] = piece_char(man, side == 0);
              break;
            }
          }
        }

        std::string placement;
        for (int row = 0; row < 8; ++row) {
          int empty = 0;
          for (int file = 0; file < 8; ++file) {
            const char c = squares[row * 8 + file];
            if (c == '\0') {
              empty++;
              continue;
            }
            if (empty > 0) { placement += static_cast<char>('0' + empty); }
            empty = 0;
            placement += c;
          }
          if (empty > 0) { placement += static_cast<char>('0' + empty); }
          if (row < 7) { placement += '/'; }
        }

        for (const char* side : {" w - - 0 1", " b - - 0 1"}) {
          const std::string fen = placement + side;
          if (load_FEN(fen, &game)) { out.push_back(fen); }
        }
      }
    }
  }

  return out;
}


// DEC-356 (2026-11-06), DEC-316's answer for tests/test_tuner_gradient.cpp's
// "the endgame factor's gradient is the derivative, chain rule in": at S122's
// constants the case read 3.4757e-6 at parameter 726, psqt_eg[king][b6],
// against its 1e-6, the derivative right and the weight nearly stationary over
// the generator's rows (|g| 1.98e-9, 1.1e-5 of the case's largest). One pair
// appended after the generator's rows, the position and its colour-mirror,
// took 726's |g| x65, and the case read 6.27485e-7 at 739 over 5717 rows: the
// pair was 8/8/8/3r4/8/1k6/8/K3R3 w and its mirror, picked on the rows
// libstdc++'s std::shuffle drew.
// S341 (2026-11-13, DEC-359): on shuffle_squares()'s rows, the one population
// under both libraries, that pair read 8.99624e-6 at 646, psqt_eg[queen][b8]
// (646's |g| 2.54e-10 over the generator's rows), and the rule, run
// unchanged, picks two pairs:
// the first for 646, after which the case read 1.3848e-6 at 679,
// psqt_eg[queen][c4], and the second for 679. With both the case reads
// 8.30801e-7 at 435, psqt_eg[pawn][g3], over 5721 rows, a factor of 1.20,
// under GCC with libstdc++ and clang with libc++ alike.
// GOLDEN (DEC-142): the pairs, picked by the rule written before it first ran
// (adocs/data/S124_factor_gradient_options.txt, "Option A, the rule"): the
// first eligible pair of placements(80), in order, that puts |dE/d TARGET| at
// three times the base's, again over the extended rows while the case's worst
// is past 1e-6, at most three pairs. Re-derive with
// adocs/data/S124_factor_gradient_rows.cpp (its header has the build line),
// which exits non-zero if its pick is not these rows, whenever the weights, the
// generator or the case's setting move.
inline const char* const factor_gradient_rows[] = {
    "1Q6/7k/7b/8/8/8/K7/8 w - - 0 1",
    "8/k7/8/8/8/7B/7K/1q6 b - - 0 1",
    "8/6k1/8/1b3K2/2Q5/8/8/8 w - - 0 1",
    "8/8/8/2q5/1B3k2/8/6K1/8 b - - 0 1",
};


// S124's population: the generator's rows at 40 a signature and colouring,
// then DEC-356's rows, one pair until S341 and two since (DEC-359).
inline std::vector<std::string> population()
{
  std::vector<std::string> out = placements();
  for (const char* fen : factor_gradient_rows) {
    out.push_back(fen);
  }
  return out;
}

}  // namespace test_endgame_positions
