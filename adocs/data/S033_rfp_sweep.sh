#!/bin/bash
# Sweep the reverse futility constants: for each setting, does the fast search
# suite stay green, and how many nodes does the search bench save.
# Every build goes to build-sweep so build/ keeps the shipping configuration.
set -u

cd /home/max/ws/chesso || exit 1

out=/tmp/claude-1000/-home-max-ws-chesso/e6b3ca05-ee17-4b5d-bd45-f4a1a2377e53/scratchpad/rfp_sweep.tsv
echo -e "margin\tmin\tmax\tsuite\tnodes_total\tfailed_cases" > "$out"

for margin in 75 100 150 200 300; do
  for mind in 1 2 3; do
    for maxd in 4 6 8; do
      flags="-DRFP_MARGIN=$margin -DRFP_MIN_DEPTH=$mind -DRFP_MAX_DEPTH=$maxd"
      cmake -S . -B build-sweep -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DCMAKE_CXX_FLAGS="$flags" > /dev/null 2>&1
      cmake --build build-sweep -j12 --target chesso test_search > /dev/null 2>&1 || {
        echo -e "$margin\t$mind\t$maxd\tBUILD_FAIL\t-\t-" >> "$out"; continue; }

      log=$(cd tests && ../build-sweep/tests/test_search 2>/dev/null)
      if echo "$log" | grep -q "Status: SUCCESS"; then
        suite=green
        failed="-"
      else
        suite=RED
        failed=$(echo "$log" | grep "^TEST CASE:" | tr '\n' ';' | cut -c1-120)
      fi

      nodes=$(tools/search_bench.py ./build-sweep/src/chesso 9 2>/dev/null \
              | grep -oE "[0-9]+ nodes" | awk '{s+=$1} END {print s}')

      echo -e "$margin\t$mind\t$maxd\t$suite\t$nodes\t$failed" >> "$out"
      echo "margin=$margin min=$mind max=$maxd $suite nodes=$nodes"
    done
  done
done
