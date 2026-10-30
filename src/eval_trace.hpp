#pragma once
// The trace build, S262: evaluate() records the coefficient of every weight it
// applies, so the tuner fits the evaluation's own arithmetic rather than a
// second, hand-written copy of it. Grant's "evaluation wrapping" (Evaluation &
// Tuning in Chess Engines, 2020): every place a weight is multiplied in also
// says, in a trace build, what it was multiplied by.
//
// **A compile-time switch, and compiled out of every engine binary.**
// CHESSO_TRACE is defined on one target only, the `chesso_engine_trace`
// library in src/CMakeLists.txt, which the tuner and the trace tests link.
// `chesso_engine` and the `chesso` binary built from it never see it, in the
// release, tune, debug or any other build: without the define every
// EVAL_TRACE_* below expands to `((void)0)`, its arguments are never evaluated,
// and src/evaluation.cpp compiles to the object code it compiled to before the
// calls were written (INV-6; the S262 step file has the disassembly
// comparison).
//
// What is recorded is the coefficient, not the product: for a weight pair
// (mg, eg) applied as `count * score`, the trace holds `count` against the
// pair's (term, index). Which taper the pair's sum then goes through, and the
// clamp over stage two, are not recorded -- they are fixed by where the term
// sits in evaluation.cpp, and tools/tuner_trace.hpp states them per term. The
// identity that a trace reconstructs evaluate() exactly, integer tapers and
// clamp included, is what holds the two together; the tuner checks it on every
// row it loads.
//
// Linear terms record White's coefficient minus Black's, the one number their
// score depends on. A term whose weights pass through a per-side transform --
// a king-safety curve, say, where the side's sum is squared before the two are
// subtracted -- needs each side's coefficients separately, because the
// transform of a difference is not the difference of the transforms. That is
// EVAL_TRACE_SIDE. No term in evaluate() takes it today; the format carries
// it, and tests/test_tuner_gradient.cpp exercises the path with a synthetic
// group.
#include <cstddef>
#include <cstdint>

#ifdef CHESSO_TRACE

namespace eval_trace
{

// The weight arrays a coefficient multiplies, one per mg/eg pair of arrays and
// one for the untapered piece values. `index` is the array index: the piece
// type 0..4 for material, `type * 64 + square` for the tables with the square
// mirrored for Black exactly as eval_add_piece() mirrors it, the enum index for
// the rest, 0 for tempo, and the pawn's own rank, 0 for its second, for
// S123's five passed pawn arrays. Those five come after tempo so that every
// term before them keeps the feature numbers it had, and S125's seven come
// after them for the same reason, each indexed by its weights' cell
// (src/evaluation.hpp): 0 for the two counts, the file less one for the two
// tables by file, the own rank less the table's first for the three by rank.
enum term_t : uint8_t
{
  TERM_MATERIAL,
  TERM_PSQT,
  TERM_MOBILITY,
  TERM_KING_SAFETY,
  TERM_PASSED_PAWN,
  TERM_PAWN_STRUCTURE,
  TERM_PIECE_PLACEMENT,
  TERM_TEMPO,
  TERM_PASSED_FREE,
  TERM_PASSED_SAFE,
  TERM_PASSED_KING_OWN,
  TERM_PASSED_KING_ENEMY,
  TERM_CANDIDATE_PASSER,
  TERM_ISOLATED_BACKWARD,
  TERM_ISOLATED_FILE,
  TERM_DOUBLED_FILE,
  TERM_BACKWARD_RANK,
  TERM_PHALANX,
  TERM_SUPPORTED,
  TERM_WEAK_UNOPPOSED,
  TERM_COUNT
};

// Whose coefficient it is. BOTH is White's minus Black's, the form every
// linear term records; WHITE and BLACK are one side's own, for a per-side
// transform.
enum side_t : uint8_t
{
  SIDE_WHITE = 0,
  SIDE_BLACK = 1,
  SIDE_BOTH = 2
};

struct entry_t
{
  term_t term;
  side_t side;
  uint16_t index;
  int32_t coef;
};

// One evaluate() call's record. Entries are appended as the weights are
// applied, so one (term, index) can appear many times -- once per piece for
// mobility, for instance -- and the reader sums them. 32 pieces is the most
// load_FEN accepts and no term records more than two entries a piece, so the
// capacity is several times what a legal board can write; `overflow` is set
// rather than writing past it, and the reader refuses such a record.
struct record_t
{
  static constexpr size_t CAPACITY = 1024;

  int phase = -1;
  size_t count = 0;
  bool overflow = false;
  entry_t entries[CAPACITY];

  void clear()
  {
    phase = -1;
    count = 0;
    overflow = false;
  }
};

// Where a trace build writes, or nullptr for nowhere. Thread local so a
// reader can extract on several threads at once; set around one evaluate()
// call and cleared after it, so the accessors the tests call record nothing.
//
// constinit, here and on the definition in evaluation.cpp, for the other
// translation units that write it (tools/tuner_trace.hpp, the tests). Without
// it they cannot know the sink needs no dynamic initialisation, so GCC reaches
// it through a TLS wrapper that first tests a weak init function for null,
// and SANITIZER=ON null-checks the address the wrapper returns. S278 found GCC
// 13.3 reading that check off the flags of `add sink@gottpoff(%rip), %reg`,
// which ld's initial-exec to local-exec rewrite turns into a `lea` that sets
// no flags: the jump read the weak function's zero, and UBSan stopped on a
// "store to null pointer" whose address was not null.
extern constinit thread_local record_t* sink;

inline void add(term_t term, side_t side, int index, int coef)
{
  record_t* record = sink;

  if (record == nullptr) { return; }

  if (record->count >= record_t::CAPACITY) {
    record->overflow = true;
    return;
  }

  record->entries[record->count++] = {term, side, static_cast<uint16_t>(index),
                                      coef};
}

inline void set_phase(int phase)
{
  if (sink != nullptr) { sink->phase = phase; }
}

}  // namespace eval_trace

#define EVAL_TRACE(term, index, coef)                                     \
  ::eval_trace::add(::eval_trace::term, ::eval_trace::SIDE_BOTH, (index), \
                    (coef))
#define EVAL_TRACE_SIDE(term, side, index, coef) \
  ::eval_trace::add(::eval_trace::term, (side), (index), (coef))
#define EVAL_TRACE_PHASE(phase) ::eval_trace::set_phase(phase)

#else

#define EVAL_TRACE(term, index, coef) ((void)0)
#define EVAL_TRACE_SIDE(term, side, index, coef) ((void)0)
#define EVAL_TRACE_PHASE(phase) ((void)0)

#endif
