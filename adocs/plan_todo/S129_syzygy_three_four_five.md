id:         S129
goal:       three, four and five man tablebase probing, written from the format description
accepts:    written from the published format description and **not** derived from any existing prober, which is a large part of what this step costs (DEC-016, DEC-084); win-draw-loss probed in the search under a piece-count and depth bound, distance-to-zero at the root; **the five-valued result is honoured** -- cursed wins and blessed losses are not collapsed into win and loss, which is a real bug that throws away won games; SyzygyPath is a UCI option, because the rating list supplies the files rather than the engine; the SPRT that measures it is run with **adjudication off**, since adjudication is what makes tablebases look worthless in a normal harness run; a verdict of zero is recorded as zero
touches:    src/, MANUAL.md
excludes:   six and seven man, which are 149 GB and 17 TB
decisions:  DEC-016, DEC-084
closes:
blocks:
paused_by:
done:

## Last, and optional, and the reason is the licence rather than the Elo

The rating list does allow four, five and six man tablebases. The published
gain for a hand-crafted engine of roughly this class is about **13 Elo**, and
one report of the standard route calls it "two thousand lines of foreign code
that I don't understand for a mere 5 Elo" (**unverified**: the 2026-09-04
literature check found no source for that quotation).

**And the 13 is a six-men figure, which this step is not.** Corrected by S185
on 2026-09-11 (2026-09-04_plan_review-F06, row A29). Both endpoints the plan
quotes for this group are six-men: CPW *Syzygy Bases*
(https://www.chessprogramming.org/Syzygy_Bases) gives Stockfish 10dev
classical, 10+0.1, all WDL in RAM, **+13** at six men -- and +2.7 for
Stockfish 15 with a network -- and the "about 25" is Topple at six men
(https://www.talkchess.com/forum3/viewtopic.php?t=70110, konsolas, 2019-03-05).
**This step is three to five men.** The only three-man figure found anywhere is
in that same thread: Minic with three-man tablebases, **0 Elo**. So the ratio
argued below is the optimistic reading of a figure measured on a set this step
excludes, and nothing in the record says what three to five men is worth to an
engine of this class.

Chesso cannot take that route in any case. The universal reference prober is
exactly the kind of thing DEC-016 forbids, so this is a from-scratch
implementation of a compressed table format for a gain that is **13 Elo at six
men and unmeasured at five**. That ratio is the worst on the plan, which
is why it sits at the end and is marked optional rather than dropped: it is
real Elo, it is allowed, and it is the last thing to reach for. If the
three-to-five figure ever comes back near Minic's zero, dropping the step is
the honest outcome and it is a decision, not a tidy-up.
