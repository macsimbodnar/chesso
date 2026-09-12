// Fixture for tools/plan_prose_check.py's own tests (S221). The title below
// is deliberately written as two adjacent C++ string literals -- the shape
// clang-format produces once a TEST_CASE-family title crosses the 80-column
// limit, as S042's and S024's titles both did. Never compiled or linked into
// any binary: tests/CMakeLists.txt does not list this file. It exists only
// for tests/test_plan_citations.py to cite, so the checker's title-stitching
// has a real split literal to resolve a phrase against.
#include <doctest.h>

struct s221_fixture_t
{};

TEST_CASE_FIXTURE(s221_fixture_t,
                  "first half of a "
                  "title continued")
{}
