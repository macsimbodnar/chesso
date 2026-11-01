#!/usr/bin/env python3
"""Fixtures with known answers for each lexer."""
from comment_lexer import lex_cpp, lex_python, lex_shell, lex_cmake, strip_comments

cpp = r'''#include <x> // one
int a = 1'000'000; /* two */ char c = '"'; // three
const char* s = "// not a comment /* neither */";
const char* r = R"x(// not
/* comment */ )" still )x"; // four
char d = '\''; char e = '/'; // five
/* six
   spans */
// seven \
   continues
int b = a /2/ 3; // eight
const char* u = u8R"(//no)"; char q = '\\'; // nine
'''
cs, code = lex_cpp(cpp)
assert len(cs) == 9, [cpp[c.start_off : c.end_off] for c in cs]
assert [c.trailing for c in cs] == [True, True, True, True, True, False, False, True, True], [
    c.trailing for c in cs
]

py = '''#!/usr/bin/env python3
"""Module doc."""
# one
x = "# no"  # two
def f():
    """Doc
    more."""
    y = """# not a doc
    # nor comment"""  # three
    return y
class C:
    "class doc"
s = 'é'  # four
'''
cs, code = lex_python(py)
kinds = [c.kind for c in cs]
assert kinds.count("line") == 4 and kinds.count("docstring") == 3 and kinds.count("shebang") == 1, kinds
assert py[cs[1].start_off : cs[1].end_off] == '"""Module doc."""', py[cs[1].start_off : cs[1].end_off]
assert 8 in code and 9 in code and 6 not in code

sh = r"""#!/bin/bash
# one
echo "# no" # two
echo '# no' ; # three
x=$# ; y=${#arr[@]} # four
cat <<HD
# heredoc data
HD
cat <<-'A' # five
	# data
	A
echo a#b # six
z=$'\'#' # seven
(( 16#ff )) # eight
"""
cs, code = lex_shell(sh)
real = [c for c in cs if c.kind == "line"]
assert len(real) == 8, [sh[c.start_off : c.end_off] for c in cs]

cm = """# one
set(A "# no") # two
#[[ three
spans ]]
set(B [[ # no ]]) # four
"""
cs, code = lex_cmake(cm)
assert len(cs) == 4, [cm[c.start_off : c.end_off] for c in cs]

assert strip_comments("x.cpp", "int a; // z\n// w\nint b;\n") == "int a;\nint b;\n"
print("selftest OK")
