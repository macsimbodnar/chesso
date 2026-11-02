#!/usr/bin/env bash
# S122 phase 1: the cost-only patch at each mask, evaluation.cpp alone recompiled and
# linked with the landing's other objects (build/); bench must be the landing's at every
# mask and the .text sections distinct. Run from a worktree whose build/ is the landing's:
# W is that worktree, D a scratch directory.
set -u
W=/home/max/ws/chesso-s122
D=/home/max/ws/chesso/.tuning/s122/phase1/costmasks
rm -rf $D; mkdir -p $D/src
cp -r $W/src/. $D/src/
( cd $D && patch -s -p1 < $W/adocs/data/S122_cost_only.patch ) || { echo "patch failed"; exit 1; }
for mask in 1 2 4 8 16 31; do
  nice /usr/bin/c++ -I$D/src -I$W/build/generated -O3 -DNDEBUG -DS122_COST=$mask -std=gnu++20 -march=native \
      -Wa,-mbranches-within-32B-boundaries -Wall -Wextra -Werror -o $D/eval_$mask.o -c $D/src/evaluation.cpp || { echo "compile $mask failed"; exit 1; }
  cp $W/build/src/libchesso_engine.a $D/lib_$mask.a
  ar d $D/lib_$mask.a evaluation.cpp.o && cp $D/eval_$mask.o $D/evaluation.cpp.o && ( cd $D && ar r lib_$mask.a evaluation.cpp.o ) || { echo "ar $mask failed"; exit 1; }
  /usr/bin/c++ -O3 -DNDEBUG $W/build/src/CMakeFiles/chesso.dir/main.cpp.o -o $D/chesso_$mask $D/lib_$mask.a || { echo "link $mask failed"; exit 1; }
  echo "mask $mask: $(echo bench | $D/chesso_$mask | tail -1 | sed -E 's/ [0-9]+ nps//'), .text sha $(objcopy -O binary --only-section=.text $D/chesso_$mask /dev/stdout | sha256sum | cut -c1-16)"
done
echo "zero: $(echo bench | $W/build/src/chesso | tail -1 | sed -E 's/ [0-9]+ nps//'), .text sha $(objcopy -O binary --only-section=.text $W/build/src/chesso /dev/stdout | sha256sum | cut -c1-16)"
echo COST-MASKS-DONE
