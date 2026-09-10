"""Time-management mutants: the two limits and what bounds them.

These are the mutants the node signature cannot see at all -- a clock
bug does not move a fixed-depth search by one node -- so whatever catches
them is a test that ran a clock.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver. `old` must occur exactly once in `file` --
an ambiguous anchor mutates a site nobody chose, and the tool refuses the whole
run before it writes anything. A mutant's several pairs are applied together as
one bug. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.
"""

C = "src/chesso.cpp"

m("M32_hard_limit_no_overhead", C, "time",
  'hard limit ignores MOVE_OVERHEAD_MS',
  ('hard_ms = std::min(hard_ms, static_cast<int64_t>(remaining_ms) -\n                                  static_cast<int64_t>(MOVE_OVERHEAD_MS));',
   'hard_ms = std::min(hard_ms, static_cast<int64_t>(remaining_ms));'),
  origin="2026-09-04_test_review")

m("M33_soft_limit_unbounded", C, "time",
  'soft limit not capped by the hard limit',
  ('soft_ms = std::max<int64_t>(std::min(soft_ms, hard_ms), 1);',
   'soft_ms = std::max<int64_t>(soft_ms, 1);'),
  origin="2026-09-04_test_review")

# S192's fast check asked for this one, and the first answer was a survivor.
# The replaced soft-limit case no longer carries `REQUIRE(scaled.drop > 0)` --
# that precondition fired on eight search mutants that were not
# time-management defects, which is why it went (DEC-168) -- and the identity
# the replacement asserts holds trivially on a loop that passes a constant zero
# fall, because the same zero is what it records. Run against `0abe648` this
# mutant **survived the whole fast label**: a proved gap, not an argued one.
# What kills it is the subcase written for it, "a fall reaches the time manager
# on at least one position", which asserts over a set of four rather than
# pinning one position's tree.
#
# It carries a `(void)` of each symbol it orphans, for the reason search.py's
# header gives: the Release build this tool runs is -Werror, and without them
# `previous_score` and `previous_score_ready` are set but never read, so the
# mutant is stillborn instead of measured. Observed: the first form failed to
# compile at src/chesso.cpp:823, `variable 'previous_score' set but not used`.
m("M34_score_drop_always_zero", C, "time",
  'the loop passes a constant zero fall to the time manager',
  ('      const int score_drop =\n          previous_score_ready\n              ? std::max(0, previous_score - search_result.score)\n              : 0;',
   '      const int score_drop = 0;\n      (void) previous_score;\n      (void) previous_score_ready;'),
  origin="S192 fast check")
