id:         S051
goal:       S020 and S030 gates accept a measured zero and name the instrument
accepts:    both steps' accepts read outcome-neutral — a measurement with its noise floor recorded, the keep-or-revert call made from it, zero recorded as zero; S020 names an instrument that actually reports what the gate asks (e.g. hyperfine over interleaved fixed-depth runs); neither accepts requires a positive gain to complete
touches:    adocs/plan_todo/S020_single_check_computation.md, adocs/plan_todo/S030_move_encoding_16_bit.md
excludes:   measuring anything now
decisions:  DEC-019
closes:     2026-08-13_plan_review-F08
blocks:
paused_by:
done:

## What is there

`S020` accepts "measured gain larger than the benchmark's own reported
resolution"; `S030` accepts the same phrase. The project's record is that
predicted gains routinely measure zero — DEC-019 counts 0, 0 and *slower*,
and S005, S006 and S015 recorded zeros and made the keep-or-revert call with
the reason stated. A gate that requires the gain cannot be met by the honest
outcome and rewards finding a number over measuring one. For S020 no
instrument fits the words: `bench_movegen` reports its own resolution but
does not run the search; `search_bench.py` times the search but reports no
resolution.

Full evidence: 2026-08-13_plan_review-F08.
