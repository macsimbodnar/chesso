#!/usr/bin/env bash
set -euo pipefail

# S104. Builds a distributable, profile-guided binary for one instruction set.
#
#   ./build_release.sh bmi2         the target that ships to a rating list
#   ./build_release.sh avx2         Zen1 and Zen2, where PEXT is microcoded
#   ./build_release.sh portable     x86-64-v2 fallback
#   ./build_release.sh all          all three, in that order
#
# Output is build-release-<arch>/src/chesso. cmake/arch.cmake is the table of
# what each name means and cmake/pgo.cmake is why the profile flags are scoped to
# the engine targets.
#
# `native` is refused here on purpose. It is the default for `build/`, which is
# the directory that gets measured on this machine, and it is exactly wrong in a
# binary anyone else runs: -march=native on the DEC-049 machine emits AVX2 and
# BMI2 for a Coffee Lake part, and the same file on anything older raises SIGILL
# rather than running slowly.
#
# Three passes over one build directory, and the middle one is the point. A PGO
# build is only as good as its workload, so this one is a fixed-depth search over
# many positions and not perft and not one position:
#
#   * perft measures generate, make and unmake. It never evaluates, never orders
#     a move and never probes the table, so a profile taken from it would tell
#     the compiler that three quarters of the hot code is cold.
#   * one position profiles that position's pawn structure and material. The
#     branch that matters in an endgame is not the branch that matters in a
#     middlegame, and a profile from one is a profile against the other.
#
# The workload is 16 positions from each of the 25 values of the engine's own
# game_phase(), 400 in all, drawn from adocs/data/S018_raw.tsv -- 13522 positions
# chesso actually reached in 210 of its own games. Stratifying by phase is what
# stops the middlegame answering for the endgame; it is the same sampling
# S021_aspiration_sweep.py uses and for the same reason.
#
# Serial, through one process, and that is a choice. Twelve concurrent
# instrumented processes would finish in a twelfth of the time and DEC-050 would
# be satisfied, but .gcda merge-on-exit would then depend on the scheduler and
# the profile would stop being a function of the source tree. One process also
# leaves the transposition table warm across positions, which is the regime a
# game's later moves search in.
#
# The profile is regenerated per build and never committed. .pgo/ is gitignored:
# it is a property of one workload run against one source tree, and a stale
# profile reads as provenance while being noise. -Wcoverage-mismatch is left as
# the error -Werror makes it, so a profile that does not match these sources
# stops the build rather than being corrected past.

depth="${PGO_DEPTH:-10}"
per_phase="${PGO_PER_PHASE:-16}"
root="$(cd "$(dirname "$0")" && pwd)"
raw="$root/adocs/data/S018_raw.tsv"

case "${1:-}" in
     bmi2|avx2|portable) arches=("$1") ;;
     all)                arches=(bmi2 avx2 portable) ;;
     native)
          echo "build_release.sh: native is not distributable. Use build/ for local" >&2
          echo "measurement; ship bmi2, avx2 or portable. cmake/arch.cmake says why." >&2
          exit 2
          ;;
     *)
          echo "usage: $0 {bmi2|avx2|portable|all}" >&2
          exit 2
          ;;
esac

[[ -f "$raw" ]] || { echo "build_release.sh: no $raw to draw the workload from" >&2; exit 1; }

# Columns are `game ply phase cost ref own mate san fen`; phase is 3 and fen is
# 9. Deterministic: first N rows of each phase in file order, no sampling.
workload_fens="$(mktemp)"
trap 'rm -f "$workload_fens"' EXIT
awk -F'\t' -v n="$per_phase" 'NR>1 && seen[$3]++ < n { print $9 }' "$raw" > "$workload_fens"
positions="$(wc -l < "$workload_fens")"

echo "== workload: $positions positions, $per_phase per phase, depth $depth"

for arch in "${arches[@]}"; do
     profile="$root/.pgo/$arch"
     out_dir="$root/build-release-$arch"

     echo
     echo "===== $arch ====="

     # Regenerated per build: a .gcda left from a previous source tree is the one
     # input that fails silently, and a stale build directory is how it survives.
     rm -rf "$profile" "$out_dir"
     mkdir -p "$profile"

     echo "-- pass 1/3  instrumented build"
     cmake -S "$root" -B "$out_dir" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCHESSO_ARCH="$arch" \
          -DCHESSO_PGO=generate \
          -DCHESSO_PGO_DIR="$profile" > /dev/null
     cmake --build "$out_dir" --target chesso -j"$(nproc)" > /dev/null

     echo "-- pass 2/3  workload"
     workload_start=$SECONDS
     python3 - "$out_dir/src/chesso" "$depth" "$workload_fens" <<'WORKLOAD'
# Reads until `bestmove` between positions, which is not optional. `go` runs on
# its own thread, so a blind `position`/`go` stream down a pipe starts one search
# per position, cancels all but the last and profiles a depth-1 tree. Measured
# while writing this: the 400-position workload "finished" in under a second.
# tools/search_bench.py is the shape this copies, and DEV_MANUAL.md records the
# trap under "Wait for `bestmove` when you script it".
import subprocess, sys

engine, depth, fen_file = sys.argv[1], int(sys.argv[2]), sys.argv[3]
fens = [line.strip() for line in open(fen_file) if line.strip()]

engine_proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, text=True, bufsize=1)

def send(command):
    engine_proc.stdin.write(command + "\n")
    engine_proc.stdin.flush()

send("uci")
while "uciok" not in engine_proc.stdout.readline():
    pass

total_nodes, last = 0, 0
for fen in fens:
    send("position fen " + fen)
    send(f"go depth {depth}")
    while True:
        line = engine_proc.stdout.readline()
        if not line:
            sys.exit("build_release.sh: engine died during the workload")
        if line.startswith("info") and " nodes " in line:
            last = int(line.split(" nodes ")[1].split()[0])
        if line.startswith("bestmove"):
            total_nodes += last
            break

send("quit")
engine_proc.wait(timeout=60)
print(f"   {len(fens)} positions, {total_nodes} nodes")
WORKLOAD
     echo "   depth $depth in $((SECONDS - workload_start))s"

     gcda="$(find "$profile" -name '*.gcda' | wc -l)"
     [[ "$gcda" -gt 0 ]] || { echo "build_release.sh: workload wrote no .gcda into $profile" >&2; exit 1; }
     echo "   $gcda profile files"

     # Pass 3 reconfigures the SAME directory rather than using a second one, and
     # that is load bearing. GCC names a .gcda by mangling the absolute path of
     # the object that wrote it and looks it up under the same name; a second
     # build directory gives a second object path, every lookup misses, and the
     # result is an ordinary -O3 binary wearing a PGO label. This script had that
     # bug and it measured as PGO being worth zero. -Wmissing-profile under
     # -Werror is what stops it recurring, so nothing here silences it.
     echo "-- pass 3/3  optimised build"
     cmake -S "$root" -B "$out_dir" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCHESSO_ARCH="$arch" \
          -DCHESSO_PGO=use \
          -DCHESSO_PGO_DIR="$profile" > /dev/null
     cmake --build "$out_dir" --target chesso -j"$(nproc)" > /dev/null

     popcnt="$(objdump -d "$out_dir/src/chesso" | grep -c popcnt || true)"
     echo "-- $out_dir/src/chesso"
     echo "   $popcnt popcnt instructions, $(du -h "$out_dir/src/chesso" | cut -f1)"
     [[ "$popcnt" -gt 0 ]] || { echo "build_release.sh: no popcnt in $arch, S104's whole point" >&2; exit 1; }
done
