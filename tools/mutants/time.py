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
