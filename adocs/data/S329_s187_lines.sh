#!/usr/bin/env bash
# S329: each citation S187's stamp lists, and each of its generator's first six
# HAND_CHOICES entries, read at the commit that wrote its line number, at the
# baseline S187's mapping records (adocs/data/S187_symbols.tsv `rev`), and at
# 124e557, S187's start. Run from a chesso checkout:
#
#     bash adocs/data/S329_s187_lines.sh
#
# The writers were found by the `git log -S` at the end, over the step file as
# it sat in adocs/plan_todo/; S024's moved to plan_current/ and back between
# e424032 and a433433 with its Note unchanged, which is why those two appear in
# its line. Its output on 2026-11-03 is adocs/data/S329_s187_lines.txt.
set -euo pipefail
show() { git show "$1:$2" | awk -v a="$3" -v b="$4" 'NR>=a && NR<=b {printf "      %s %5d: %s\n", c, NR, $0}' c="$1" | sed 's/ *$//'; }
row() { # label file range writer baseline
  local label=$1 file=$2 a=$3 b=$4 writer=$5 base=$6
  echo "== $label  $file $a-$b"
  for c in "$writer" "$base" 124e557; do show "$c" "$file" "$a" "$b"; done
}
# the stamp's eight
row "S099 state construction"      src/chesso.cpp 674 675 7862f41 7862f41
row "S132 state construction"      src/chesso.cpp 674 674 fe25f46 7504c3d
row "S159 state construction"      src/chesso.cpp 674 674 fe25f46 192a5a3
row "S132 node counter"            src/data_structures.hpp 453 453 fe25f46 7504c3d
row "S132 per-iteration reset"     src/chesso.cpp 726 726 fe25f46 7504c3d
row "S115 test-only counter"       src/chesso.cpp 60 64 7862f41 192a5a3
row "S115 test-only counter"       src/chesso.cpp 88 89 7862f41 192a5a3
row "S024 countermove write"       src/search.cpp 517 517 7b54f18 192a5a3
# the generator's first six not above
row "S024 countermove array"       src/data_structures.hpp 436 436 7b54f18 192a5a3
row "S091 see_value (two rows)"    src/bitboard.cpp 1096 1096 c0954ec 7862f41
row "S109 see_value"               src/bitboard.cpp 1096 1096 c0954ec 7504c3d
echo "== where the named lines were, in each writer's tree"
echo "  7b54f18 counter_moves decl / write:"; git show 7b54f18:src/data_structures.hpp | grep -n 'counter_moves' | sed 's/^/    /'; git show 7b54f18:src/search.cpp | grep -n 'counter_moves\[' | sed 's/^/    /'
echo "  7b54f18 search_t / search_state_t / negamax:"; git show 7b54f18:src/data_structures.hpp | grep -n '^struct search' | sed 's/^/    /'; git show 7b54f18:src/search.cpp | grep -n '^int negamax' | sed 's/^/    /'
echo "  c167d05, the first writer of S024's two:"; show c167d05 src/data_structures.hpp 434 434; show c167d05 src/search.cpp 507 507
echo "  fe25f46 search_state_t / iterative_deepening_search:"; git show fe25f46:src/data_structures.hpp | grep -n '^struct search_state_t' | sed 's/^/    /'; git show fe25f46:src/chesso.cpp | grep -n '^uci_search_result_t iterative_deepening_search' | sed 's/^/    /'
echo "== the commits that wrote each line number, git log -S over the step file"
for pair in S024_continuation_history.md:src/search.cpp:517 S024_continuation_history.md:src/data_structures.hpp:436 \
            S115_aspiration_refinements.md:src/chesso.cpp:60-64 S115_aspiration_refinements.md:src/chesso.cpp:88-89 \
            S132_time_management_node_fraction.md:data_structures.hpp:453 S132_time_management_node_fraction.md:chesso.cpp:726 \
            S091_see_pruning_main_search.md::1096 S109_shallow_depth_pruning_block.md::1096; do
  f=${pair%%:*}; cite=${pair#*:}
  echo "  $f $cite: $(git log --format=%h -S"$cite" -- adocs/plan_todo/$f | paste -sd' ')"
done
echo S329-S187-LINES-DONE
