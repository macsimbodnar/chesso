"""The transposition table as clusters, S119: replacement, the age, the probe.

Six mutants over the rules S119 wrote. Four are the replacement rule the step
file asks to be weighed -- depth and age together, the age read on the
counter's cycle, a store of a new position never refused -- and two are the
probe's written test and `hashfull`'s count:

  TC01  age ignored: an entry is worth its depth    tests/test_search.cpp,
                                                    "replacement weighs depth
                                                    and age together", the
                                                    first half
  TC02  depth ignored: an entry is worth its age    the same case, the second
                                                    half
  TC03  age read as a plain difference floored at   "age is a distance on the
        0, so the counter's wrap reads as current   counter's cycle"
        -- S271's finding, one counter wider
  TC04  a store refused when the way it would       "a full cluster never
        replace is current and deeper: the          refuses a new position", and
        direct-mapped table's depth preference      "an entry aliased to current
        put back over the cluster, which an entry   never refuses a new
        the counter has come round to exactly       position" on its own
        trips as readily as one written this
        search
  TC05  the probe matches a never-written entry     "a partial-key collision
        whose key 0 equals a hash's low 32 bits     reads the other position's
                                                    entry", its last check
  TC06  hashfull counts every written entry, not    "hashfull counts the
        this search's                               current search's entries"

TC01 to TC04 change which entries the table keeps, so each moves the search
tree and the bench signature. TC05 needs a hash whose low 32 bits are 0 to
show, and TC06 is reporting only, so neither need move it; the named case is
what sees them.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`. `expected` is "killed".

Prefix TC is new: `git log --all -p -- tools/mutants` shows AW, GC, LQ, MW,
NG, NT, PD, RZ and every single letter taken, and no TC.
"""

T = "src/transposition_table.cpp"
H = "src/transposition_table.hpp"

m("TC01_age_ignored", T, "table/replacement",
  'replacement weighs an entry by its depth alone, so an entry from many '
  'searches ago keeps its way against a shallower one of this search however '
  'old it is -- the depth-preferred table S119 replaced, without even the '
  'generation that let it go across searches',
  ("  return entry.depth -\n"
   "         TT_AGE_PLIES * static_cast<int>(tt_age(now, entry.generation));\n",
   "  return entry.depth -\n"
   "         0 * static_cast<int>(tt_age(now, entry.generation));\n"),
  origin="S119")

m("TC02_depth_ignored", T, "table/replacement",
  'replacement weighs an entry by its age alone, so the oldest way is always '
  'replaced -- a deep result from the last search gives way to make room '
  'while a quiescence entry of this one stays',
  ("  return entry.depth -\n"
   "         TT_AGE_PLIES * static_cast<int>(tt_age(now, entry.generation));\n",
   "  return 0 * entry.depth -\n"
   "         TT_AGE_PLIES * static_cast<int>(tt_age(now, entry.generation));\n"),
  origin="S119")

m("TC03_age_not_cyclic", H, "table/replacement",
  'age is read as the plain difference of the two counters and 0 when the '
  'entry\'s is larger, so once the 16-bit counter comes round every entry '
  'written before the wrap reads as current and is weighed at its full '
  'depth: S271\'s finding on the 8-bit counter, put back',
  ("{ return (now + TT_GENERATION_CYCLE - then) % TT_GENERATION_CYCLE; }\n",
   "{ return (now >= then) ? static_cast<unsigned>(now - then) : 0u; }\n"),
  origin="S119")

m("TC04_new_position_refused", T, "table/replacement",
  'a store of a position the cluster does not hold is refused when the way '
  'it would replace is this search\'s and deeper, the direct-mapped table\'s '
  'depth preference applied to the victim -- so a cluster of three deep '
  'current entries takes nothing for the rest of the search, and neither does '
  'one whose entries the 16-bit counter has come round to exactly: S271\'s '
  'harm, one cycle wider',
  ("  // Decided before the key is overwritten, because the decision reads it.\n",
   "  if (entry->generation == tt->generation && depth < entry->depth) {\n"
   "    return;\n"
   "  }\n"
   "\n"
   "  // Decided before the key is overwritten, because the decision reads it.\n"),
  origin="S119")

m("TC05_probe_reads_unwritten", T, "table/probe",
  'the probe drops its written test, so a never-written entry -- key 0, '
  'generation 0 -- answers every hash whose low 32 bits are 0 with a depth-0 '
  'entry of no type and an evaluation of 0, which quiescence takes as the '
  'static score',
  ("    if (entry.key == key && entry.generation != 0) { return &entry; }\n",
   "    if (entry.key == key) { return &entry; }\n"),
  origin="S119")

m("TC06_hashfull_counts_all", T, "uci/reporting",
  'hashfull counts every written entry rather than this search\'s, so it '
  'reads near 1000 from the first few searches of a game on and reports the '
  'table\'s history, not its use',
  ("      if (entry.generation == tt->generation) { current++; }\n",
   "      if (entry.generation != 0) { current++; }\n"),
  origin="S119")
