id:         S159
goal:       measure whether the second killer slot wants ageing rather than distinctness: the unguarded shift discards slot 1 on every repeat, so S149's -11 Elo may be the guard preserving a stale killer for a whole go
accepts:    the hypothesis below is stated as a hypothesis and then tested, not assumed -- S149 measured the guard, not the mechanism, and nothing here may be written as a finding until a run says it; at least one ageing scheme for the killer table is implemented and decided by SPRT, one change at a time, and the candidates worth trying are the two the mechanism suggests: clear `killer_moves` at the start of each iterative-deepening iteration rather than once per `go` (`src/chesso.cpp:674` builds `search_state_t state = {}` once), and shift slot 1 down on a repeat instead of copying slot 0 onto itself, which ages the second slot without ever letting a distinct move survive a repeat; the duplicate rate is re-counted on the S149 instrumentation for whatever ships, so the ordering table's state is documented by number rather than by argument; a verdict of zero is recorded as zero and `adocs/specs.md`'s ordering clause is updated to whatever the run says, including if that is "nothing moved"
touches:    src/search.cpp, src/chesso.cpp, tests/test_search.cpp, adocs/specs.md
excludes:   the killer slot count, which stays at two; the ordering band constants, which CLAUDE.md and `src/search_params.hpp:29` keep out of the tuned set; history malus, gravity and butterfly indexing, which are S093; re-litigating S149's distinctness guard, which was measured at -11.02 +/- 10.53 Elo over 2522 games and is closed -- a scheme that happens to make the slots distinct is in scope only if that is a side effect of ageing and not the thing being proposed
decisions:  DEC-019
closes:
blocks:
paused_by:
author:
done:

## Where this comes from

S149 implemented CPW's *Killer Heuristic* replacement rule -- guard the shift so
the two slots always hold different moves -- and it measured **-11.02 +/- 10.53
Elo, H0 accepted over 2522 games**. The guard was reverted. The number is not in
dispute; what it *means* is, and this step is the part S149 could not answer.

**The hypothesis, and it is a hypothesis: nothing here has been measured.** The
unguarded shift does two things at once and only one of them is the bug F01
named. It duplicates the slots, yes. It also **discards whatever slot 1 was
holding**, every single time a quiet repeats at a ply -- which is 66 % of
stores. So the unguarded store is, incidentally, an aggressive ageing mechanism
for the second slot. The guard removes the duplication *and* the ageing
together, and since killers persist across every iteration of one `go`, a
guarded slot 1 can hold a move that refuted something eight iterations ago and
keep being tried at 800000 for the rest of the search.

If that reading is right, the 11 Elo is the cost of the staleness rather than
the benefit of the duplication, and the two are separable. If it is wrong, an
ageing scheme measures zero and that is recorded as zero -- which is itself
worth knowing, because it would say the second killer slot carries very little
either way on this search and S093 can stop treating it as load-bearing.

**Ordering.** S093 rewrites this same block (history malus, gravity, butterfly
indexing). The constraint S149 carried carries over unchanged: this lands
before S093 or is folded into it deliberately, never after, or S093's verdict is
taken over whatever the killer table happens to be doing.
