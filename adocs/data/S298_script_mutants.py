#!/usr/bin/env python3
"""S298: the cuts tests/test_gate_extra_script.sh must be observed red under.

The clang stage's half of what adocs/data/S197_script_mutants.py is for the
first five stages: shell-script mutants of `tools/gate_extra.sh`, not `src/`
ones, so they are outside `tools/mutation_check.py`'s registry and nothing is
built. Each cut removes one thing the clang stage claims to do, and names the
case of the test that is meant to kill it. S197's cuts are not repeated here;
run both files.

    python3 adocs/data/S298_script_mutants.py <outdir>
    for m in <outdir>/*.sh; do
        bash tests/test_gate_extra_script.sh "$m" > /dev/null 2>&1 \\
            || echo "killed $m"
    done

Run from the repository root. Every substitution must appear exactly once in
`tools/gate_extra.sh`, so a cut whose anchor has moved is a hard error rather
than a silently skipped mutant. Written blind on 2026-10-29 while an SPRT held
the machine; the kill count is the step file's, once phase b has run it.
"""

import os
import sys

MUTANTS = {
    # the stage is in the default list at all -- the script as it stood
    # before S298, in effect (case 1: five stages, no clang commands)
    "N1_not_a_default_stage": (
        'all_stages="prose citations debug sanitize perft clang"',
        'all_stages="prose citations debug sanitize perft"'),
    # the refusal of a cache whose compiler is not clang (case 12, both runs)
    "N2_no_cache_compiler_check": (
        '    && clang_cache_or_fail "$dir" \\\n',
        ''),
    # the C compiler is checked as well as the C++ one (case 12, C run)
    "N3_cxx_compiler_only": (
        '  for var in CMAKE_CXX_COMPILER CMAKE_C_COMPILER; do',
        '  for var in CMAKE_CXX_COMPILER; do'),
    # the Mac's default compiler is a candidate (case 13)
    "N4_pinned_name_only": (
        '  for pair in c++:cc clang++-22:clang-22 clang++:clang; do',
        '  for pair in clang++-22:clang-22 clang++:clang; do'),
    # a candidate is asked what it is, not trusted by name: the sandbox's c++
    # is g++, taken first (case 1, and the cache check then refuses it)
    "N5_trusted_by_name": (
        '    if ! is_clang "$cxx_path" || ! is_clang "$cc_path"; then',
        '    if false; then'),
    # no clang is red, not a skip (case 14)
    "N6_skip_without_clang": (
        '  find_clang || return $?',
        '  find_clang || return 0'),
    # every error reaches the log, not only the first (case 1)
    "N7_no_keep_going": (
        '    && cmake --build "$repo/$dir" -j"$jobs_n" -- -k 0 \\',
        '    && cmake --build "$repo/$dir" -j"$jobs_n" \\'),
    # libc++, the Mac's standard library (case 1)
    "N8_no_libcxx": (
        '      -DCMAKE_CXX_FLAGS=-stdlib=libc++ -DCMAKE_EXE_LINKER_FLAGS=-stdlib=libc++ \\\n',
        ''),
    # Release, where the unused-* warnings are on (case 1)
    "N9_debug_build": (
        '      -G Ninja -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE="$tune" \\',
        '      -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCHESSO_TUNE="$tune" \\'),
    # the second directory is the tune build (case 1)
    "N10_tune_half_not_tune": (
        '  clang_half build-clang-tune ON || status=$?',
        '  clang_half build-clang-tune OFF || status=$?'),
    # the second directory still runs after the first fails (case 2)
    "N11_first_half_stops": (
        '  clang_half build-clang OFF || status=$?',
        '  clang_half build-clang OFF || return $?'),
    # both fast suites run (case 1)
    "N12_no_fast_suite": (
        '    && cmake --build "$repo/$dir" -j"$jobs_n" -- -k 0 \\\n'
        '    && ctest --test-dir "$repo/$dir" -L fast --output-on-failure'
        ' --no-tests=error\n',
        '    && cmake --build "$repo/$dir" -j"$jobs_n" -- -k 0\n'),
    # a red half makes the stage red (case 2)
    "N13_status_dropped": (
        '  return $status\n',
        '  return 0\n'),
    # the cache strings that make the directory the Mac's build are verified,
    # not only the build type (case 15, all three runs)
    "N14_expect_build_type_only": (
        '  expect="CMAKE_GENERATOR:INTERNAL=Ninja CMAKE_BUILD_TYPE:STRING=Release"\n'
        '  expect="$expect CHESSO_TUNE:BOOL=$tune'
        ' CMAKE_CXX_FLAGS:STRING=-stdlib=libc++"\n'
        '  expect="$expect CMAKE_EXE_LINKER_FLAGS:STRING=-stdlib=libc++"\n',
        '  expect="CMAKE_BUILD_TYPE:STRING=Release"\n'),
    # both halves of a pair must be clang, not only the C++ one (case 16)
    "N15_cc_unchecked": (
        '    if ! is_clang "$cxx_path" || ! is_clang "$cc_path"; then',
        '    if ! is_clang "$cxx_path"; then'),
    # a machine without ninja is refused by name (case 17)
    "N16_no_ninja_check": (
        '  if ! command -v ninja > /dev/null; then\n'
        '    echo "no ninja on PATH: the clang stage configures with -G Ninja'
        ' for -k 0" >&2\n'
        '    return 1\n'
        '  fi\n',
        ''),
}


def main():
    if len(sys.argv) != 2:
        print("usage: S298_script_mutants.py <outdir>", file=sys.stderr)
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
