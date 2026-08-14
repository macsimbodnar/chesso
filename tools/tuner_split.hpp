#pragma once
// Where the tuner cuts its validation split, and why the cut is between games.
// S066.
//
// `tools/datagen.cpp` writes a game's rows inside one `lock_guard` on
// `output_lock`, so they are contiguous, and every row of a game carries the
// same label -- the game's result. The tuner used to shuffle a **row** index
// and hold out the last tenth of it, which puts nearly every game on both sides
// of the cut: measured over the 11003693-row `selfplay_v2.tsv`, 119360 of
// 119999 games, **99.47 %**. A held-out row whose game is in the training set
// measures memorisation, not generalisation.
//
// The split is cut between games instead. The cut needs to know where one game
// ends and the next begins, and the corpus carries no game id, so the boundary
// is reconstructed from the FEN: the ply a position stands at,
// `2 * (fullmove - 1) + (black to move)`, **strictly increases** with every
// move played, so within one game it can only go up. A row whose ply does not
// advance on its predecessor cannot belong to the same game.
//
// That predicate is one-sided on purpose, and the direction is what makes the
// result exact:
//
// - It **never splits a game**, because it only fires where a within-game
//   invariant is violated. So no game can land on both sides of the cut. Zero
//   straddling games is a property of the construction, not a measurement that
//   came out well.
// - It **can miss a boundary**, when the next game's first recorded position
//   stands at a later ply than the previous game's last. Two games then merge
//   into one block -- and a merged block still lands wholly on one side, so the
//   cost is a coarser split and not a contaminated one. Measured on
//   `selfplay_v2.tsv`: 119998 blocks against the 120000 games datagen reported,
//   so **at most 2 missed boundaries in 120000**, 0.0017 %. (At most, because a
//   game whose every position was filtered out writes no rows at all and is
//   indistinguishable from a merge.)
//
// The ply is used rather than the FEN's move number alone because the move
// number misses far more: 119852 blocks on the same corpus, at most 148 missed
// boundaries, 0.123 %. A game ending with Black to move and the next starting
// at the same move number with White to move is no descent in the move number
// and is a ply that went backwards. Two stronger predicates were measured and
// buy almost nothing on top of the ply: adding "a piece appeared, so a capture
// ran backwards" finds 119998, exactly what the ply alone finds, and adding "a
// castling right came back, or the label changed" finds 119999, one more.
//
// The held-out fraction is no longer exact, because a block is indivisible. The
// last block taken can overshoot `--validation` by up to its own length; on
// `selfplay_v2.tsv` at seed 1 the realised fraction is 10.0002 % against the
// 10 % asked for, an overshoot of 19 rows.
//
// Indivisibility also costs a clamp at the other end: the **last block is never
// held out**, so a corpus of one game holds nothing out rather than holding
// everything out and leaving the gradient dividing by zero. That case is new --
// a row-level split could always cut one row off a game -- and it is reachable,
// because a two-row corpus is how S040 and S041 measured the `--only` groups.
//
// With one row per game this reproduces the old row-level split exactly -- same
// permutation, same rows held out, same train count -- for every `--validation`
// below 1.0, which is what pins the change to the grouping and nothing else.
// `tests/test_tuner_split.cpp` asserts it. At 1.0 the clamp above binds and the
// old code divided by zero, so there is nothing there worth being equal to.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace tuner_split
{

// The half-move a position stands at, counted from the start of the game. What
// makes it usable as a boundary detector is that make_move() only ever advances
// it: `src/bitboard.cpp:883` increments the full-move counter after Black
// moves, and the side to move alternates, so this is strictly increasing over a
// game's positions whatever the search does in between.
inline uint32_t ply_index(uint32_t fullmove, bool black_to_move)
{ return 2 * (fullmove - 1) + (black_to_move ? 1 : 0); }


// The ply of one `fen result score phase` row. False when the row does not
// carry a usable FEN move number, which is the one thing this reconstruction
// needs and the four-column format does not name.
//
// The whole row is re-split here rather than taking the pieces load() already
// parsed, so that the tuner and the test walk the same parser over the same
// bytes. It costs one pass over a line whose placement is about to be expanded
// into six feature vectors.
inline bool row_ply(const std::string& line, uint32_t* ply)
{
  const size_t tab = line.find('\t');
  const std::string fen =
      line.substr(0, (tab == std::string::npos) ? line.size() : tab);

  std::istringstream fields(fen);
  std::string field[6];

  for (size_t i = 0; i < 6; ++i) {
    if (!(fields >> field[i])) { return false; }
  }

  if (field[1] != "w" && field[1] != "b") { return false; }

  const int fullmove = atoi(field[5].c_str());

  if (fullmove < 1) { return false; }

  *ply = ply_index(static_cast<uint32_t>(fullmove), field[1] == "b");
  return true;
}


// The first row of each block, in file order. A block is one game, or two
// adjacent games the ply could not tell apart -- see the header comment for why
// the second case is safe.
inline void game_starts(const std::vector<uint32_t>& ply,
                        std::vector<uint32_t>* starts)
{
  starts->clear();

  if (ply.empty()) { return; }

  starts->push_back(0);

  for (size_t i = 1; i < ply.size(); ++i) {
    if (ply[i] <= ply[i - 1]) { starts->push_back(static_cast<uint32_t>(i)); }
  }
}


// Shuffles the blocks, holds out whole blocks until `validation` of the rows
// are held out, and writes the training rows first and the held-out rows after
// them. Returns the number of training rows, which is where the caller cuts.
//
// Blocks are taken from the **end** of the shuffled order, and the rows keep
// their file order inside a block. Neither choice matters to the fit -- the
// gradient is full-batch, so the order within a range is a summation order and
// nothing else -- and together they are what makes the one-row-per-game case
// reproduce the old row-level split bit for bit.
inline size_t split(const std::vector<uint32_t>& starts,
                    size_t rows,
                    double validation,
                    uint64_t seed,
                    std::vector<uint32_t>* index)
{
  index->clear();
  index->reserve(rows);

  if (starts.empty()) { return 0; }

  const auto block_end = [&](size_t block) {
    return (block + 1 < starts.size()) ? static_cast<size_t>(starts[block + 1])
                                       : rows;
  };

  std::vector<uint32_t> order(starts.size());
  for (size_t i = 0; i < order.size(); ++i) {
    order[i] = static_cast<uint32_t>(i);
  }

  std::mt19937_64 rng(seed);
  std::shuffle(order.begin(), order.end(), rng);

  const size_t target =
      static_cast<size_t>(static_cast<double>(rows) * validation);

  size_t held = 0;
  size_t first_held = order.size();

  // `> 1`, not `> 0`: the last block is never held out. A block is indivisible,
  // so a corpus of one game cannot give up part of itself, and holding all of
  // it out leaves gradient() dividing by a count of zero -- measured on a
  // two-row corpus at `--validation 0.5`, `0 train, 2 validation (100.0000%)`
  // and then `epoch 1  train 0.000000  validation -nan`. The row-level split
  // this replaced could always cut a row off a game, so this failure mode is
  // new and is clamped here rather than left to the caller. The realised
  // fraction is what main() prints, and a corpus that cannot hold anything out
  // says so.
  while (first_held > 1 && held < target) {
    --first_held;
    held += block_end(order[first_held]) - starts[order[first_held]];
  }

  const auto emit = [&](size_t block) {
    for (size_t row = starts[block]; row < block_end(block); ++row) {
      index->push_back(static_cast<uint32_t>(row));
    }
  };

  for (size_t k = 0; k < first_held; ++k) {
    emit(order[k]);
  }

  const size_t train_count = index->size();

  for (size_t k = first_held; k < order.size(); ++k) {
    emit(order[k]);
  }

  return train_count;
}

}  // namespace tuner_split
