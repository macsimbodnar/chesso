"""The node-fraction time manager, S132: the counting, the factor, the gate,
the floor on the product, and the accumulation the share is taken over.

Seven mutants over one step's rule, and the classes are the ones the published
record actually shipped at this technique rather than classes invented here.
Three separate engines wrote a follow-up commit for the counting half alone --
a root-move count that was wrong, aspiration widenings left out of the
accumulation, a snapshot that had drifted away from the move it was taken for
-- and none of the three is visible in a game, in a node count or in a bench
signature. **A clock bug moves no fixed-depth search by one node**, which is
what the file `time.py` says about its own four and what makes a direct case
the only thing that can catch any of these:

  the key collides      a bucket keyed on squares merges the four promotions
                        of one pawn push, so the best move's share is the
                        share of whatever it was merged with. Invisible until
                        the best move is a promotion, which is the one case
                        an engine fixed after shipping
  the snapshot is lost  every root move is charged with every node searched
                        before it as well as its own, so the shares are a
                        prefix sum and the last move owns the tree
  the factor inverted   more of the tree under the best move buys *more*
                        time, which is the rule upside down and still a
                        perfectly smooth function of the same input
  the gate dropped      a depth-1 distribution -- one move, one node -- is
                        believed, which is the noise the gate exists for
  the floor misplaced   the floor sits on S089's two scalers alone and the
                        product falls through it, which is the
                        multiplicative-stacking hazard the step file's
                        section 5 names: 30 % of 30 % is 9 % of the
                        allocation
  the accumulation      buckets cleared between iterations, or a denominator
                        taken from one iteration's counter -- both report a
                        plausible percentage of the wrong tree

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. A mutant's several pairs are applied
together as one bug. `expected` is "killed" unless a person has argued the
mutant is behaviourally equivalent to the engine, which no tool can decide.

**One mutant that is not here, declared and not inferred.** Moving the
snapshot from before `make_move` to after it is **equivalent**: `make_move`
counts no node, so the two readings of `explored_nodes` are the same number.
The bug the record fixed under that name was a snapshot that had drifted
several statements away from the move, which is `K02` here -- the killable
form of the same mistake.

Ids are never reused: a new mutant takes the next free number across every
file here, and this file opens the `K` prefix.
"""

C = "src/chesso.cpp"
D = "src/data_structures.hpp"
S = "src/search.cpp"

m("K01_bucket_keyed_on_squares", D, "time/nodes",
  'the bucket is keyed on the from and to squares instead of on the whole '
  'move encoding, so the four promotions of one pawn push share one bucket '
  'and a best move that is a promotion is credited with its siblings\' nodes. '
  'The sum identity cannot see it -- nothing is lost, only merged -- and what '
  'kills it is the bucket count against the root\'s own legal move count, on '
  'a position chosen because several of its moves differ only in what the '
  'pawn becomes',
  ('    if (state->root_move_keys[i] == move) {\n'
   '      state->root_move_nodes[i] += nodes;\n'
   '      return;\n'
   '    }',
   '    if (MOVE_FROM(state->root_move_keys[i]) == MOVE_FROM(move) &&\n'
   '        MOVE_TO(state->root_move_keys[i]) == MOVE_TO(move)) {\n'
   '      state->root_move_nodes[i] += nodes;\n'
   '      return;\n'
   '    }'),
  origin="S132")

m("K02_snapshot_lost", S, "time/nodes",
  'the node count before the move is taken as zero, so every root move is '
  'charged with the whole tree searched before it as well as its own subtree '
  '-- the shares become a prefix sum and the last move searched owns most of '
  'the root. This is the snapshot-placement bug three engines wrote a '
  'follow-up commit for, in its killable form',
  ('    const uint64_t nodes_before_move = (ply == 0) ? state->explored_nodes '
   ': 0;',
   '    const uint64_t nodes_before_move = 0;'),
  origin="S132")

m("K03_factor_inverted", C, "time/nodes",
  'the share is added where it is subtracted, so a best move that took most '
  'of the tree buys **more** time and a wide open position buys less: the '
  'rule exactly upside down, still monotone, still smooth, still inside every '
  'bound the settings declare. No node count and no bench signature moves',
  ('  return ((TM_NODE_BASE_PCT - bestmove_node_percent) * TM_NODE_SCALE_PCT) '
   '/ 100;',
   '  return ((TM_NODE_BASE_PCT + bestmove_node_percent) * TM_NODE_SCALE_PCT) '
   '/ 100;'),
  origin="S132")

m("K04_floor_off_the_product", C, "time/nodes",
  'the floor is left on S089\'s two scalers alone and the product of the '
  'three falls through it, which is the multiplicative-stacking hazard the '
  'step file names: a stability discount already near the floor times a '
  'factor at a share of 100 % spends a tenth of the allocation and the search '
  'stops after its second iteration',
  ('      const int combined_scale =\n'
   '          std::max((scale * node_factor) / 100, TM_SCALE_MIN_PERCENT);',
   '      const int combined_scale = (scale * node_factor) / 100;'),
  origin="S132")

m("K05_depth_gate_dropped", C, "time/nodes",
  'the depth gate goes and a depth-1 distribution decides the clock -- one '
  'move searched, one node, a share of 100 % -- which is the noise '
  'TmNodeMinDepth exists for and is worst exactly where the clock is '
  'shortest',
  ('      const int node_factor =\n'
   '          (conf.scale_time && current_depth >= TM_NODE_MIN_DEPTH)\n'
   '              ? search_time_node_factor_percent(last_bestmove_node_percent)'
   '\n'
   '              : 100;',
   '      const int node_factor =\n'
   '          conf.scale_time\n'
   '              ? search_time_node_factor_percent(last_bestmove_node_percent)'
   '\n'
   '              : 100;'),
  origin="S132")

m("K06_buckets_cleared_per_iteration", C, "time/nodes",
  'the buckets are cleared with the node counter at the top of every '
  'iteration, so the share is the last iteration\'s and not the search\'s. '
  'The published reading is the whole search -- "the total nodes searched" -- '
  'and an engine that had the other one shipped a fix for it. The percentage '
  'stays plausible on every position, which is why the case that catches this '
  'asserts an identity over the denominator instead',
  ('    // Reset the explored nodes in the previous iteration\n'
   '    state.explored_nodes = 0;',
   '    // Reset the explored nodes in the previous iteration\n'
   '    state.explored_nodes = 0;\n'
   '    state.root_move_count = 0;'),
  origin="S132")

m("K07_denominator_is_the_counter", C, "time/nodes",
  'the share is taken over the iteration\'s own node counter instead of over '
  'the buckets: the numerator accumulates across iterations and the '
  'denominator does not, so the share climbs past 100 % as the search '
  'deepens and the factor runs off the end of the range its settings declare',
  ('      const uint64_t root_nodes = root_nodes_total(&state);',
   '      const uint64_t root_nodes = state.explored_nodes;'),
  origin="S132")
