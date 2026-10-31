"""S118's pawn table, DEC-332: every way it can answer for the wrong position.

The first group is the accepts' own clause, that a collision is a recomputation
and never a score: the verifier cut down, one colour at a time, to a stored
piece of the key, and to nothing. The rest are a table read wrongly or not at
all, and the terms that must stay out of it.

  PT01  the verifier ignores Black's pawns   "a forced collision recomputes and
                                             never answers for another
                                             structure", the pair sharing White's
  PT02  the verifier ignores White's pawns   the same, the pair sharing Black's
  PT03  the verifier is the key's top bits   the same: the planted key, which
        (the design DEC-332 (2) rejected)    nothing else can arrange
  PT04  no verifier: every read is a hit     every case that compares a score
  PT05  a hit recomputes as well             "a hit answers from the entry": the
                                             score is right and the table buys
                                             nothing, which agreement cannot see
  PT06  the slot from the whole key          the cases' own slot reads: no wrong
                                             score, and every king move a miss
  PT07  the mask off by one                  the same: two slots in the table
  PT08  the terms outside the entry stored   "the same pawns with other pieces
        in it on a miss                      read their own terms"
  PT09  the file bytes swapped               the pure form shares the entry, so
                                             the counts' own cases and the mirror
  PT10  the passers one colour's             the same
  PT11  a rank missing from the file sets    the same, the rook file counts
  PT12  the trace build reads the table      test_eval_trace "the search's form
                                             records what the pure form records,
                                             twice"
  PT13  the lazy form's window turned round  "the lazy form reads the same
                                             through the table"
  PT14  quiescence calls the pure form       "a quiescence node reads the pawn
                                             table"
  PT15  a main node calls the pure form      "a main search node reads the pawn
                                             table"

Each of PT05, PT06, PT14 and PT15 leaves every score and node count where it was
and costs only speed, so `chesso bench` cannot see them and the suite has to.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`. `expected` is "killed" unless a
person has argued the mutant behaviourally equivalent, which no tool decides.

Ids are never reused: PT is this file's own prefix, and no file in tools/mutants/
has used it, on any branch (`git log --all -p -- tools/mutants`).
"""

E = "src/evaluation.cpp"
S = "src/search.cpp"

VERIFIER = ("  if (entry->white_pawns != white_pawns || "
            "entry->black_pawns != black_pawns)\n")
FILL = ("    pawn_entry_fill(white_pawns, black_pawns, entry);\n"
        "    PAWN_TABLE_COUNT(misses);\n")

m("PT01_verifier_ignores_black", E, "eval/pawn table",
  "a slot holding the same White pawns over other Black pawns answers",
  (VERIFIER, "  if (entry->white_pawns != white_pawns)\n"),
  origin="S118")

m("PT02_verifier_ignores_white", E, "eval/pawn table",
  "a slot holding the same Black pawns over other White pawns answers",
  (VERIFIER, "  if (entry->black_pawns != black_pawns)\n"),
  origin="S118")

m("PT03_verifier_is_the_key", E, "eval/pawn table",
  "the entry verified by a stored piece of the key, which is not exact: two "
  "structures under one key share an entry",
  (VERIFIER,
   "  if (entry->spare != static_cast<uint16_t>(board->pawn_hash >> 48))\n"),
  (FILL,
   FILL + "    entry->spare = static_cast<uint16_t>(board->pawn_hash >> 48);\n"),
  origin="S118")

m("PT04_no_verifier", E, "eval/pawn table",
  "every read is a hit, on whatever the slot holds",
  (VERIFIER, "  if (false && entry->black_pawns != black_pawns + white_pawns)\n"),
  origin="S118")

m("PT05_hit_recomputes", E, "eval/pawn table",
  "a hit runs the pass as well: every score right and nothing saved",
  ("    assert(pawn_entry_is_fresh(entry));\n",
   "    pawn_entry_fill(white_pawns, black_pawns, entry);\n"),
  origin="S118")

m("PT06_slot_from_the_whole_key", E, "eval/pawn table",
  "the slot is the position's key's: right scores, and a miss per position "
  "instead of per pawn structure",
  ("      &table->entries[board->pawn_hash & (PAWN_TABLE_ENTRIES - 1)];\n",
   "      &table->entries[board->hash & (PAWN_TABLE_ENTRIES - 1)];\n"),
  origin="S118")

m("PT07_mask_off_by_one", E, "eval/pawn table",
  "the index masks with the size and not the size less one: two slots, and "
  "one of them past the end",
  ("      &table->entries[board->pawn_hash & (PAWN_TABLE_ENTRIES - 1)];\n",
   "      &table->entries[board->pawn_hash & PAWN_TABLE_ENTRIES];\n"),
  origin="S118")

m("PT08_outside_terms_stored", E, "eval/pawn table",
  "piece placement and S123's A and B folded into the entry when it is filled, "
  "so a hit reads the pieces of whoever filled it",
  (FILL,
   FILL + "    {\n      score_t stored = entry->score;\n"
   "      outside_pawn_terms<false>(board, entry, &stored, nullptr, nullptr);\n"
   "      entry->score = stored;\n    }\n"),
  ("static inline int cheap_score(const board_t* board, const pawn_entry_t* entry)\n",
   "static inline int cheap_score(const board_t* board, const pawn_entry_t* entry,"
   " bool stored = false)\n"),
  ("  outside_pawn_terms<false>(board, entry, &pawns, nullptr, nullptr);\n",
   "  if (!stored) {\n"
   "    outside_pawn_terms<false>(board, entry, &pawns, nullptr, nullptr);\n"
   "  }\n"),
  ("    return cheap_score(board, pawn_entry_of(board, table));\n",
   "    return cheap_score(board, pawn_entry_of(board, table), true);\n"),
  origin="S118")

m("PT09_file_bytes_swapped", E, "eval/pawn table",
  "each side's rook file counts read the other side's files",
  ("  entry->white_files = static_cast<uint8_t>(white_pawn_files);\n",
   "  entry->white_files = static_cast<uint8_t>(black_pawn_files);\n"),
  ("  entry->black_files = static_cast<uint8_t>(black_pawn_files);\n",
   "  entry->black_files = static_cast<uint8_t>(white_pawn_files);\n"),
  origin="S118")

m("PT10_passers_one_colour", E, "eval/pawn table",
  "the entry keeps White's passers only, so Black's have no A or B",
  ("  entry->passers = white_passers | black_passers;\n",
   "  entry->passers = white_passers;\n"),
  origin="S118")

m("PT11_file_set_short_a_rank", E, "eval/pawn table",
  "the byte expands to seven ranks of the whole-file set, so a rook on the "
  "missing one reads every file as open",
  ("  constexpr bb_t every_rank = 0x0101010101010101ULL;\n",
   "  constexpr bb_t every_rank = 0x0001010101010101ULL;\n"),
  origin="S118")

m("PT12_trace_reads_the_table", E, "eval/pawn table",
  "the trace build reads the table like the search: a hit records no pawn "
  "coefficient, so a row traced through it fits half its terms",
  ("  if constexpr (trace_build) {\n    return evaluate_cheap(board);\n",
   "  if constexpr (false) {\n    return evaluate_cheap(board);\n"),
  origin="S118")

m("PT13_lazy_window_turned_round", E, "eval/pawn table",
  "the lazy form through the table takes its shortcuts on the wrong edges",
  ("  return lazy_from_cheap(board, cheap_from_table(board, table), alpha, beta,\n"
   "                         exact);\n",
   "  return lazy_from_cheap(board, cheap_from_table(board, table), beta, alpha,\n"
   "                         exact);\n"),
  origin="S118")

m("PT14_quiescence_site_pure", S, "eval/pawn table",
  "quiescence, where the evaluation is called most, never reads the table: "
  "every score right and nothing saved",
  ("          : evaluate_lazy(&game->board, alpha, beta, &static_eval_is_exact,\n"
   "                          &state->pawn_table);\n",
   "          : evaluate_lazy(&game->board, alpha, beta, &static_eval_is_exact);\n"),
  origin="S118")

m("PT15_main_site_pure", S, "eval/pawn table",
  "a main search node never reads the table: every score right and nothing "
  "saved",
  ("                 : evaluate(&game->board, &state->pawn_table));\n",
   "                 : evaluate(&game->board));\n"),
  origin="S118")
