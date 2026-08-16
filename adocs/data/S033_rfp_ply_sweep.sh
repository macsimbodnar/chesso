#!/bin/bash
# Third sweep: keep the top plies of the tree unpruned. The rule already
# exempts the root; this asks what it costs to exempt the plies just under it,
# where the reported score is decided and where the mate cases live.
set -u

cd /home/max/ws/chesso || exit 1

out=/tmp/claude-1000/-home-max-ws-chesso/e6b3ca05-ee17-4b5d-bd45-f4a1a2377e53/scratchpad/rfp_ply_sweep.tsv
echo -e "min_ply\tmargin\tmax\tsuite\tnodes_total\tbest_moves\tfailed_cases" > "$out"

for minply in 1 2 3 4 5; do
  for margin in 75 100; do
    maxd=6
    flags="-DRFP_MIN_PLY=$minply -DRFP_MARGIN=$margin -DRFP_MAX_DEPTH=$maxd"
    cmake -S . -B build-sweep -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
          -DCMAKE_CXX_FLAGS="$flags" > /dev/null 2>&1
    cmake --build build-sweep -j12 --target chesso test_search > /dev/null 2>&1 || {
      echo -e "$minply\t$margin\t$maxd\tBUILD_FAIL\t-\t-\t-" >> "$out"
      echo "min_ply=$minply margin=$margin BUILD_FAIL"; continue; }

    log=$(cd tests && ../build-sweep/tests/test_search 2>/dev/null)
    if echo "$log" | grep -q "Status: SUCCESS"; then
      suite=green; failed="-"
    else
      suite=RED
      failed=$(echo "$log" | grep "^TEST CASE:" | tr '\n' ';' | cut -c1-160)
    fi

    bench=$(tools/search_bench.py ./build-sweep/src/chesso 9 2>/dev/null)
    nodes=$(echo "$bench" | grep -oE "[0-9]+ nodes" | awk '{s+=$1} END {print s}')
    best=$(echo "$bench" | grep -oE "best [a-z0-9]+" | awk '{printf "%s ", $2}')

    echo -e "$minply\t$margin\t$maxd\t$suite\t$nodes\t$best\t$failed" >> "$out"
    echo "min_ply=$minply margin=$margin $suite nodes=$nodes best=$best"
  done
done
