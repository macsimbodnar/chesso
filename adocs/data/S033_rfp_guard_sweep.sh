#!/bin/bash
# Second sweep: can a guard rescue the strong setting (margin 100, depth 1..6)
# that saves 55 % of nodes but hides the mate?
set -u

cd /home/max/ws/chesso || exit 1

out=/tmp/claude-1000/-home-max-ws-chesso/e6b3ca05-ee17-4b5d-bd45-f4a1a2377e53/scratchpad/rfp_guard_sweep.tsv
echo -e "guard\tmargin\tmin\tmax\tsuite\tnodes_total\tfailed_cases" > "$out"

# guard "off" is RFP compiled out through its own depth bound: same tree as
# HEAD, and the baseline every node count below is read against.
for guard in off 0 1 2 3 4; do
  for margin in 100 150; do
    mind=1; maxd=6
    if [[ "$guard" == "off" ]]; then
      maxd=0
      [[ "$margin" == "150" ]] && continue
    fi
    gflag=$guard
    [[ "$guard" == "off" ]] && gflag=0
    flags="-DRFP_GUARD=$gflag -DRFP_MARGIN=$margin -DRFP_MIN_DEPTH=$mind -DRFP_MAX_DEPTH=$maxd"
    cmake -S . -B build-sweep -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
          -DCMAKE_CXX_FLAGS="$flags" > /dev/null 2>&1
    cmake --build build-sweep -j12 --target chesso test_search > /dev/null 2>&1 || {
      echo -e "$guard\t$margin\t$mind\t$maxd\tBUILD_FAIL\t-\t-" >> "$out"
      echo "guard=$guard margin=$margin BUILD_FAIL"; continue; }

    log=$(cd tests && ../build-sweep/tests/test_search 2>/dev/null)
    if echo "$log" | grep -q "Status: SUCCESS"; then
      suite=green; failed="-"
    else
      suite=RED
      failed=$(echo "$log" | grep "^TEST CASE:" | tr '\n' ';' | cut -c1-160)
    fi

    nodes=$(tools/search_bench.py ./build-sweep/src/chesso 9 2>/dev/null \
            | grep -oE "[0-9]+ nodes" | awk '{s+=$1} END {print s}')

    echo -e "$guard\t$margin\t$mind\t$maxd\t$suite\t$nodes\t$failed" >> "$out"
    echo "guard=$guard margin=$margin $suite nodes=$nodes"
  done
done
