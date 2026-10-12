#pragma once
// The tuner's step: the optimiser, then the gauges held. S262 moved it out of
// tools/tuner_model.hpp, where S275 had put Adam and the hold, when the tuner
// stopped reading that file's model; the arithmetic of both is unchanged.
//
// Two optimisers, chosen by `--optimiser`:
//
//   adam     Kingma and Ba, "Adam: A Method for Stochastic Optimization"
//            (2014), with the paper's beta1 0.9, beta2 0.999 and epsilon
//            1e-8. The step size is the tuner's own --lr, 1.0 by default
//            since S028, not the paper's 0.001: a weight here is in
//            centipawns, and the paper's default would take thousands of
//            epochs to move one.
//   adagrad  Duchi, Hazan and Singer, "Adaptive Subgradient Methods for
//            Online Learning and Stochastic Optimization" (2011), the
//            diagonal form: each coordinate's step is the learning rate over
//            the root of its summed squared gradients. delta 1e-8, the same
//            guard Adam's epsilon is.
//
// Adam stays the default. S262's pre-registered comparison over the dedup
// corpus put AdaGrad at its best step 1.0e-5 under Adam's held-out error at the
// printed precision (0.118436 against 0.118446), and the rule was that AdaGrad
// takes over only below Adam by more than 1.0e-5. It also took 34 minutes to
// Adam's 9. adocs/data/S262_fits.txt has every run.
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "tuner_groups.hpp"

namespace tuner_optimiser
{

enum kind_t
{
  ADAM,
  ADAGRAD
};

inline bool parse_kind(const std::string& name, kind_t* out)
{
  if (name == "adam") {
    *out = ADAM;
    return true;
  }

  if (name == "adagrad") {
    *out = ADAGRAD;
    return true;
  }

  return false;
}

inline const char* kind_name(kind_t kind)
{ return (kind == ADAM) ? "adam" : "adagrad"; }

// AdaGrad's step size when --lr names none: the best of S262's pre-registered
// sweep over the dedup corpus at the tuner's defaults (DEC-134 form b),
// held-out 0.118448, 0.118446, 0.118445 and 0.118436 at 1, 3, 10 and 30. 30 is
// the top of the range swept, so a larger step was not tried.
constexpr double ADAGRAD_DEFAULT_LR = 30.0;


// One optimiser's state over a vector of `size` parameters. `moment` is
// Adam's first moment and unused by AdaGrad; `velocity` is Adam's second
// moment and AdaGrad's running sum of squares.
struct state_t
{
  kind_t kind = ADAM;
  std::vector<double> moment;
  std::vector<double> velocity;
  int epoch = 0;

  explicit state_t(size_t size = tuner_layout::PARAM_COUNT, kind_t which = ADAM)
      : kind(which), moment(size, 0.0), velocity(size, 0.0)
  {}
};


// One Adam step over the parameters `mask` leaves free, and nothing else: the
// plain update, with no gauge held. Moved from tools/tuner.cpp at S275 and to
// here at S262, its arithmetic unchanged.
inline void adam_update(const std::vector<double>& grad,
                        const std::vector<uint8_t>& mask,
                        double lr,
                        state_t* state,
                        double* params)
{
  const double beta1 = 0.9;
  const double beta2 = 0.999;
  const double epsilon = 1e-8;

  state->epoch++;

  const double correction1 = 1.0 - std::pow(beta1, state->epoch);
  const double correction2 = 1.0 - std::pow(beta2, state->epoch);

  for (size_t i = 0; i < grad.size(); ++i) {
    // A frozen parameter is left out of the update entirely rather than having
    // its gradient zeroed, because a zero gradient does not mean a zero step:
    // Adam divides a decaying moment by a decaying velocity and keeps moving on
    // what it accumulated before. Skipping the whole iteration leaves moment
    // and velocity at zero too, so the parameter comes out bit-identical to
    // what it went in as and the emitted file is one change.
    if (!mask[i]) { continue; }

    state->moment[i] = beta1 * state->moment[i] + (1.0 - beta1) * grad[i];
    state->velocity[i] =
        beta2 * state->velocity[i] + (1.0 - beta2) * grad[i] * grad[i];

    const double m = state->moment[i] / correction1;
    const double v = state->velocity[i] / correction2;

    params[i] -= lr * m / (std::sqrt(v) + epsilon);
  }
}


// One AdaGrad step, under the same mask rule: a frozen parameter is skipped,
// so its sum of squares stays zero and it comes out bit-identical. A
// coordinate whose gradient has always been zero steps 0 / (0 + delta) = 0,
// which is what keeps the 32 pawn columns no row reaches where they are, as
// it does under Adam (S275).
inline void adagrad_update(const std::vector<double>& grad,
                           const std::vector<uint8_t>& mask,
                           double lr,
                           state_t* state,
                           double* params)
{
  const double delta = 1e-8;

  state->epoch++;

  for (size_t i = 0; i < grad.size(); ++i) {
    if (!mask[i]) { continue; }

    state->velocity[i] += grad[i] * grad[i];
    params[i] -= lr * grad[i] / (std::sqrt(state->velocity[i]) + delta);
  }
}


inline void update(const std::vector<double>& grad,
                   const std::vector<uint8_t>& mask,
                   double lr,
                   state_t* state,
                   double* params)
{
  if (state->kind == ADAM) {
    adam_update(grad, mask, lr, state, params);
  } else {
    adagrad_update(grad, mask, lr, state, params);
  }
}


// The step the tuner takes: the optimiser, then every gauge the mask frees put
// back at the level `start` had. The hold moves only along null directions, so
// the evaluation of every row -- and with it every gradient and every reported
// error after this step -- is what the optimiser alone would have produced, to
// the last bits of rounding. S275.
inline void descend(const std::vector<double>& grad,
                    const std::vector<uint8_t>& mask,
                    const double* start,
                    double lr,
                    state_t* state,
                    double* params)
{
  update(grad, mask, lr, state, params);
  tuner_groups::hold_gauges(tuner_groups::held_gauges(mask), start, params);
}

}  // namespace tuner_optimiser
