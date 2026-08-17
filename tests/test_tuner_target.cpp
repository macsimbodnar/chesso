#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cmath>
#include <string>
#include <vector>
#include "tuner_target.hpp"

// The label the tuner fits against. Until S075 it was the game result and
// nothing else; it is now
//
//   target = lambda * sigma(K * score) + (1 - lambda) * result
//
// and `--lambda 0` has to be the old fit **bit for bit**, not approximately it,
// because that is the only claim that lets every figure S065 measured still
// stand. The equality is proved end to end against the pre-S075 binary and
// recorded in the step file; what this file holds is the arithmetic that makes
// it true, plus the strictness that stops an unreadable corpus fitting against
// a silent zero.
//
// What this file holds:
//
//   1. lambda 0 returns the game result exactly, for every row of a fixture
//      whose scores span the corpus's real range
//   2. the same rows at lambda 0.5 do **not** return the game result -- the
//      non-vacuity guard for case 1, which a `blend` ignoring its score or its
//      lambda would otherwise pass
//   3. lambda 1 returns exactly the sigmoid of the score, the fixed point the
//      header warns about, and the game result is then not read at all
//   4. the blend is a convex combination: inside [0, 1], monotone in the score,
//      and at lambda 0.5 the midpoint of the two labels it blends
//   5. the sigmoid is the Elo convention rather than a number that was copied:
//      0 is exactly 0.5, it is symmetric, and 400 centipawns at K 1 is a factor
//      of ten in the odds
//   6. a lambda outside [0, 1] is refused, NaN included
//   7. the score column is parsed strictly: a float, trailing text, an empty
//      field or a value past int16 is refused rather than defaulted to 0, which
//      is a legal score meaning "equal"
//
// Case 2 is what stops the file passing over a function that returns `result`
// whatever it is handed.

namespace
{

constexpr double K = 0.7624;  // S065's fitted K over selfplay_v2.tsv

// Scores from the range datagen actually writes -- measured over all 11003693
// rows of `.tuning/selfplay_v2.tsv`, -999 .. 999 -- against every label a game
// can end in.
const std::vector<int> SCORES = {-999, -450, -37, 0, 12, 288, 999};
const std::vector<double> RESULTS = {0.0, 0.5, 1.0};

}  // namespace


TEST_CASE("lambda 0 is the game result, exactly")
{
  size_t rows = 0;

  for (const int score : SCORES) {
    for (const double result : RESULTS) {
      // Exact equality is the assertion, not a tolerance: `0.0 * sigma` is
      // exactly 0.0 and `1.0 * result` is exactly `result`, which is what makes
      // the pre-S075 fit reproducible rather than merely close. An epsilon here
      // would pass over an implementation that perturbed every label by a
      // rounding error and moved 827 constants with it.
      CHECK(tuner_target::blend(0.0, score, result, K) == result);
      ++rows;
    }
  }

  REQUIRE(rows == SCORES.size() * RESULTS.size());
}


TEST_CASE("lambda 0.5 is not the game result -- case 1 is not vacuous")
{
  size_t moved = 0;

  for (const int score : SCORES) {
    for (const double result : RESULTS) {
      const double blended = tuner_target::blend(0.5, score, result, K);

      // The one row where the blend legitimately equals the result is a drawn
      // game at score 0, where both labels are 0.5. Everything else must move,
      // or `blend` is ignoring an argument.
      if (score == 0 && result == 0.5) {
        CHECK(blended == result);
        continue;
      }

      CHECK(blended != result);
      ++moved;
    }
  }

  // A precondition, not a result: without it a fixture of drawn games at score
  // 0 would report success over nothing.
  REQUIRE(moved == SCORES.size() * RESULTS.size() - 1);
}


TEST_CASE("lambda 1 is the search score alone")
{
  for (const int score : SCORES) {
    const double expected = tuner_target::sigmoid(score, K);

    // The same score against all three outcomes: at lambda 1 the game result is
    // multiplied by exactly 0.0, so a won game and a lost one carry the same
    // label. This is the fixed point tuner_target.hpp warns about, asserted
    // here so that nobody has to take the warning on trust.
    for (const double result : RESULTS) {
      CHECK(tuner_target::blend(1.0, score, result, K) == expected);
    }
  }
}


TEST_CASE("the blend is a convex combination of its two labels")
{
  for (const double lambda : {0.0, 0.25, 0.5, 0.7, 1.0}) {
    for (const double result : RESULTS) {
      double previous = -1.0;

      for (const int score : SCORES) {
        const double blended = tuner_target::blend(lambda, score, result, K);

        CHECK(blended >= 0.0);
        CHECK(blended <= 1.0);

        // Monotone in the score for every lambda above 0, and flat at 0. Both
        // are covered by "never decreasing".
        CHECK(blended >= previous);
        previous = blended;
      }
    }
  }

  // The midpoint property, which pins the weighting rather than only its
  // endpoints: a lost game the search called +288 sits exactly halfway between
  // 0 and the search's own opinion.
  const double score_label = tuner_target::sigmoid(288, K);

  CHECK(tuner_target::blend(0.5, 288, 0.0, K) ==
        doctest::Approx(0.5 * score_label));
  CHECK(tuner_target::blend(0.5, 288, 1.0, K) ==
        doctest::Approx(0.5 * score_label + 0.5));
}


TEST_CASE("the sigmoid is the Elo convention")
{
  CHECK(tuner_target::sigmoid(0.0, K) == 0.5);
  CHECK(tuner_target::sigmoid(0.0, 1.0) == 0.5);

  for (const int score : SCORES) {
    CHECK(tuner_target::sigmoid(-score, K) ==
          doctest::Approx(1.0 - tuner_target::sigmoid(score, K)));
  }

  // 400 centipawns is a factor of ten in the odds at K 1, which is what
  // LN10_OVER_400 means. Asserted from the definition so the constant is
  // derived here rather than transcribed: p / (1 - p) == 10.
  const double p = tuner_target::sigmoid(400.0, 1.0);

  CHECK(p / (1.0 - p) == doctest::Approx(10.0));
}


TEST_CASE("a lambda outside [0, 1] is refused")
{
  CHECK(tuner_target::valid_lambda(0.0));
  CHECK(tuner_target::valid_lambda(0.7));
  CHECK(tuner_target::valid_lambda(1.0));

  CHECK_FALSE(tuner_target::valid_lambda(-0.001));
  CHECK_FALSE(tuner_target::valid_lambda(1.001));

  // NaN is the one a range test written as a negation would let through, and
  // `atof` produces it from a typo. It fails both comparisons, which is why
  // valid_lambda() is written as two of them.
  CHECK_FALSE(tuner_target::valid_lambda(std::nan("")));
}


TEST_CASE("the score column is parsed strictly")
{
  int score = 12345;

  CHECK(tuner_target::parse_score("150", &score));
  CHECK(score == 150);

  CHECK(tuner_target::parse_score("-999", &score));
  CHECK(score == -999);

  CHECK(tuner_target::parse_score("0", &score));
  CHECK(score == 0);

  CHECK(tuner_target::parse_score("32767", &score));
  CHECK(score == 32767);

  CHECK(tuner_target::parse_score("-32768", &score));
  CHECK(score == -32768);

  // strtol skips leading whitespace, which is harmless on a tab-split field and
  // is pinned rather than left unknown.
  CHECK(tuner_target::parse_score(" 7", &score));
  CHECK(score == 7);

  // Everything below would be 0 through `atoi` -- a legal score meaning "the
  // search called this position equal". At any non-zero lambda that is a label
  // saying every position is drawn, on a corpus nothing reported as broken.
  CHECK_FALSE(tuner_target::parse_score("", &score));
  CHECK_FALSE(tuner_target::parse_score("abc", &score));
  CHECK_FALSE(tuner_target::parse_score("12x", &score));
  CHECK_FALSE(tuner_target::parse_score("1.5", &score));
  CHECK_FALSE(tuner_target::parse_score("-", &score));
  CHECK_FALSE(tuner_target::parse_score("40000", &score));
  CHECK_FALSE(tuner_target::parse_score("-40000", &score));

  // The out parameter is untouched by a refusal, so a caller that ignores the
  // return value does not silently pick up a partial parse.
  CHECK(score == 7);
}
