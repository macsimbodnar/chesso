#!/usr/bin/env bash
# S330: the dedup file DEC-183's recipe rebuilds, re-derived from the archived
# corpus with corpus_dedupe built at two trees. Run from a chesso checkout:
#
#     bash adocs/data/S330_dedup_digests.sh CORPUS SCRATCH DOCTEST BASE
#
# CORPUS is .tuning/selfplay_v2.tsv, read in place and refused unless it hashes
# ad8c5dfe...; SCRATCH a folder the build trees are made in, each one new;
# DOCTEST a checkout of the tests/doctest submodule, copied into the trees that
# take it, because `git archive` writes a submodule as an empty folder; BASE the
# tree-ish of the step's base. Three builds, each `git archive` of a tree into a
# new folder, Release, the target corpus_dedupe only:
#
#   a3e84e1         S042's parent, doctest supplied, ccache
#   base            BASE, doctest supplied, ccache
#   a3e84e1-recipe  a3e84e1 again, no doctest and no ccache: the recipe
#                   DEV_MANUAL.md and the corrected CORPUS_README.md give,
#                   command for command, but for its folder, which is here
#                   under SCRATCH and not /tmp, and its --out, stdout here
#
# Each binary reads CORPUS and writes the kept rows to stdout, hashed and
# counted as they stream; no copy of the corpus or of a pass's output is
# written. Its output on 2026-11-03 is adocs/data/S330_dedup_digests.txt.
set -Eeuo pipefail
trap 'echo "S330-DIGESTS-FAILED at line $LINENO"' ERR
[ $# -eq 4 ] || { echo "usage: $0 CORPUS SCRATCH DOCTEST BASE"; echo "S330-DIGESTS-FAILED: usage"; exit 2; }
corpus=$1 scratch=$2 doctest=$3 base=$4
corpus_sha=ad8c5dfe1a236af8cbf946a67692e5fbfe533c7d38b6c7468c8a8ec202d14e05

stream_digest() { # stdin: kept rows; prints their count and sha256
  python3 -I -c '
import hashlib, sys
h, n, f = hashlib.sha256(), 0, sys.stdin.buffer
while chunk := f.read(1 << 20):
    h.update(chunk)
    n += chunk.count(b"\n")
print(f"kept rows {n}, sha256 {h.hexdigest()}")'
}

build() { # label tree-ish doctest(yes|no) ccache(yes|no)
  local label=$1 tree=$2 with_doctest=$3 with_ccache=$4
  local dir=$scratch/s330_build_$label
  mkdir "$dir"
  git archive "$tree" | tar -x -C "$dir"
  if [ "$with_doctest" = yes ]; then cp -a "$doctest"/. "$dir/tests/doctest/"; rm -f "$dir/tests/doctest/.git"; fi
  local launcher=()
  if [ "$with_ccache" = yes ]; then launcher=(-DCMAKE_CXX_COMPILER_LAUNCHER=ccache); fi
  echo "== $label: $tree, tree $(git rev-parse "$tree^{tree}")"
  echo "   tests/doctest files: $(find "$dir/tests/doctest" -type f | wc -l); ccache: $with_ccache"
  echo "-- cmake -S <dir> -B <dir>/build -DCMAKE_BUILD_TYPE=Release${launcher[*]:+ ${launcher[*]}}"
  nice -n 10 cmake -S "$dir" -B "$dir/build" -DCMAKE_BUILD_TYPE=Release "${launcher[@]}" >"$dir/configure.log" 2>&1
  echo "-- cmake --build <dir>/build -j12 --target corpus_dedupe"
  nice -n 10 cmake --build "$dir/build" -j12 --target corpus_dedupe >"$dir/build.log" 2>&1
  echo "   binary sha256 $(sha256sum "$dir/build/tools/corpus_dedupe" | cut -c1-16)..."
  echo "-- <dir>/build/tools/corpus_dedupe --in CORPUS --out /dev/stdout | (count and sha256)"
  /usr/bin/time -f "wall %e s, max rss %M kB" -o "$dir/time.txt" \
    nice -n 10 "$dir/build/tools/corpus_dedupe" --in "$corpus" --out /dev/stdout 2>"$dir/stderr.txt" | stream_digest
  sed 's/^/   /' "$dir/stderr.txt" "$dir/time.txt"
  echo
}

echo "# S330, $(date -u +%Y-%m-%dT%H:%M:%SZ), $(uname -sm), $(g++ --version | head -1), $(cmake --version | head -1)"
echo "# load at start: $(cat /proc/loadavg)"
echo "== CORPUS: $(basename "$corpus"), $(wc -l <"$corpus") rows"
got=$(sha256sum "$corpus" | cut -d' ' -f1)
echo "   sha256 $got"
[ "$got" = "$corpus_sha" ] || { echo "corpus is not ad8c5dfe..."; echo "S330-DIGESTS-FAILED: corpus digest"; exit 1; }
echo
build a3e84e1 a3e84e1 yes yes
build base "$base" yes yes
build a3e84e1-recipe a3e84e1 no no
echo "# load at end: $(cat /proc/loadavg)"
echo S330-DIGESTS-DONE
