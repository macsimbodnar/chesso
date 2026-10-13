#!/usr/bin/env bash
#
# S278: no object in the build tree reaches eval_trace::sink through a TLS
# wrapper.
#
# The sink is an extern thread_local defined in src/evaluation.cpp and written
# from tools/tuner_trace.hpp and the tests. Declared without constinit, every
# such file reached it through GCC's wrapper, which tests the weak init
# function _ZTHN10eval_trace4sinkE for null first. Under SANITIZER=ON that
# path put UBSan's null check in flags the linker rewrites away, and the
# sanitizer build stopped on a "store to null pointer" at a valid address. The
# doctest case in test_eval_trace.cpp shows that only in build-sanitize and
# only where GCC picks the registers that expose it. This check holds in every
# build: a reference to the init function means some file again cannot see
# that the sink is constant-initialised. Observed red with constinit taken off
# the declaration in src/eval_trace.hpp, green with it.
#
# Every object under the build directory is read, so a new trace consumer is
# covered without being named here. Before asserting the absence, the run
# establishes what would make it meaningful: nm works on these objects, one
# object defines the sink, and at least one other refers to it from outside --
# the files that would need the wrapper exist.
#
#   test_trace_sink_tls.sh <build directory>
#
# Mach-O names carry one more leading underscore, hence the `_?`.
set -uo pipefail

fail()
{
  echo "FAIL: $*" >&2
  exit 1
}

build="${1:-}"
[[ -n "$build" && -d "$build" ]] || fail "no build directory given, or '$build' is not one"
command -v nm > /dev/null || fail "nm is not on PATH; nothing below would prove anything"

defined=0
referring=0
wrapped=""
objects=0

while IFS= read -r -d '' obj; do
  objects=$((objects + 1))
  if ! syms="$(nm "$obj" 2>&1)"; then
    fail "nm failed on $obj: $syms"
  fi
  if grep -Eq '[[:space:]][BDS][[:space:]]_?_ZN10eval_trace4sinkE$' <<< "$syms"; then
    defined=$((defined + 1))
  fi
  if grep -Eq '[[:space:]]U[[:space:]]_?_ZN10eval_trace4sinkE$' <<< "$syms"; then
    referring=$((referring + 1))
  fi
  if grep -Eq '_?_ZTHN10eval_trace4sinkE$' <<< "$syms"; then
    wrapped="${wrapped}${obj}"$'\n'
  fi
done < <(find "$build" -name '*.o' -not -path '*/CompilerId*' -print0)

((objects > 0)) || fail "no object under $build; build every target first"
((defined >= 1)) || fail "no object under $build defines eval_trace::sink; the trace library is not built here, so the check below is vacuous"
((referring >= 1)) || fail "no object under $build refers to eval_trace::sink from outside evaluation.cpp; the check below is vacuous"

if [[ -n "$wrapped" ]]; then
  echo "these objects reach eval_trace::sink through its TLS init function" >&2
  echo "(_ZTHN10eval_trace4sinkE): declare the sink constinit in src/eval_trace.hpp" >&2
  printf '%s' "$wrapped" >&2
  fail "the sink is reached through a TLS wrapper"
fi

echo "ok: $objects objects, $defined defining the sink, $referring referring to it, none through a TLS wrapper"
exit 0
