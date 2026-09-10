#!/usr/bin/env python3
"""S197: the twelve cuts tests/test_gate_extra_script.sh was observed red under.

These are shell-script mutants, not `src/` ones, so they are not in the
`tools/mutants/` registry `tools/mutation_check.py` drives -- nothing here
touches the engine and no suite is built. They are kept because a smoke test
whose cases were never seen failing is a green run and not a guard: two of the
seven cases exist only because a first version of the test let M5 and M12
through.

    python3 adocs/data/S197_script_mutants.py <outdir>
    for m in <outdir>/*.sh; do
        bash tests/test_gate_extra_script.sh "$m" > /dev/null 2>&1 \
            || echo "killed $m"
    done

Run from the repository root. Every substitution must appear exactly once in
`tools/gate_extra.sh`, so a cut whose anchor has moved is a hard error rather
than a silently skipped mutant. 12 of 12 killed on 2026-09-10 at the tree
that completed S197, and 16 of 16 at the tree that closed the four defects its
fast check then found -- M13 to M16 are those four.
"""

import os
import sys

MUTANTS = {
    # the marker, which is what a watcher terminates on
    "M1_no_done_marker": (
        'echo "GATE-EXTRA-DONE $index stages $total s $outdir"',
        'echo "all stages ok"'),
    "M2_no_failed_marker": (
        '  echo "GATE-EXTRA-FAILED: $failed $outdir" >&9\n  exit 1',
        '  exit 1'),
    "M7_fail_unmarked": (
        'fail()\n{\n  marked=1',
        'fail()\n{\n  marked=0'),
    # every stage runs even after one fails
    "M3_stop_on_first_fail": (
        '    failed="${failed}${failed:+ }${name}"',
        '    failed="${failed}${failed:+ }${name}"\n    break'),
    # each stage is the thing it claims to be
    "M4_unanchored_regex": (
        "-R '^(test_chesso|test_openings|test_movegen|test_evaluation"
        "|test_search|test_engine)$'",
        "-R 'test_chesso|test_openings|test_movegen|test_evaluation"
        "|test_search|test_engine'"),
    "M10_no_sanitizer_option": (
        '-DCMAKE_BUILD_TYPE=RelWithDebInfo -DSANITIZER=ON',
        '-DCMAKE_BUILD_TYPE=RelWithDebInfo'),
    "M11_debug_is_release": (
        'configure_and_verify build-debug "CMAKE_BUILD_TYPE:STRING=Debug" \\\n'
        '      -DCMAKE_BUILD_TYPE=Debug',
        'configure_and_verify build-debug "CMAKE_BUILD_TYPE:STRING=Release" \\\n'
        '      -DCMAKE_BUILD_TYPE=Release'),
    # DEC-167's assertion, and the parse that has to happen before it
    "M5_no_bench_compare": (
        '  if [[ "$san_total" != "$rel_total" ]]; then',
        '  if false; then'),
    "M9_no_signature_check": (
        '  if [[ ! "$line" =~ ^[0-9]+\\ nodes\\ [0-9]+\\ nps$ ]]; then',
        '  if false; then'),
    # the refusals that come before any stage
    "M6_no_cmake_check": (
        'if ! command -v cmake > /dev/null; then fail "cmake not on PATH"; fi',
        ':'),
    "M8_dirname_again": (
        'here="$0"\ncase "$here" in\n  */*) here="${here%/*}" ;;\n'
        '  *)   here="." ;;\nesac\nrepo="$(cd "$here/.." && pwd)"',
        'repo="$(cd "$(dirname "$0")/.." && pwd)"'),
    # the four the fast check found, S197's follow-up
    "M13_trust_any_cache": (
        '  cmake -S "$repo" -B "$repo/$dir" "$@" $launcher || return $?\n'
        '\n'
        '  local cache="$repo/$dir/CMakeCache.txt"',
        '  if [[ -f "$repo/$dir/CMakeCache.txt" ]]; then return 0; fi\n'
        '  cmake -S "$repo" -B "$repo/$dir" "$@" $launcher || return $?\n'
        '\n'
        '  local cache="$repo/$dir/CMakeCache.txt"'),
    "M14_no_cache_verify": (
        '    if ! grep -q -- "$want" "$cache"; then',
        '    if false; then'),
    "M15_release_line_unchecked": (
        '  signature_or_fail Release "$rel_line" || return $?',
        '  :'),
    "M16_marker_to_stderr": (
        '  echo "GATE-EXTRA-FAILED: $*" >&9',
        '  echo "GATE-EXTRA-FAILED: $*" >&2'),
    "M12_no_root_check": (
        '[[ -r "$repo/tools/plan_prose_check.py" ]] \\\n'
        '  || fail "$repo does not look like the chesso tree:'
        ' no tools/plan_prose_check.py"',
        ':'),
}


def main():
    if len(sys.argv) != 2:
        print("usage: S197_script_mutants.py <outdir>", file=sys.stderr)
        return 2
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    src = open("tools/gate_extra.sh").read()
    for name, (old, new) in sorted(MUTANTS.items()):
        found = src.count(old)
        if found != 1:
            print("anchor for %s appears %d times, not once" % (name, found),
                  file=sys.stderr)
            return 1
        path = os.path.join(outdir, name + ".sh")
        with open(path, "w") as handle:
            handle.write(src.replace(old, new))
    print("wrote %d mutants to %s" % (len(MUTANTS), outdir))
    return 0


if __name__ == "__main__":
    sys.exit(main())
