/*
 * S172. The built-in opening book, linked in as the raw Polyglot bytes.
 *
 * S146. The bytes are this project's own, and `books/fetch_book.sh` plus
 * `tools/make_book` are the whole recipe:
 *
 *   ./books/fetch_book.sh 8moves_v3.pgn
 *   ./build/tools/make_book build books/8moves_v3.pgn --out src/openings.bin
 *
 * 172232 entries over 129613 positions, 2755712 bytes, sha256
 * 3b89a4ad9146e266ae9296778067aaedcb7f57f3cf0ff2086b9ae6df15b873dd. The input
 * is the committed CC0-1.0 `books/8moves_v3.pgn`, pinned in `fetch_book.sh` by
 * both digests; the moves are read by the engine's own `algebraic_to_move` and
 * keyed by its own `get_key`, so no third-party book, table or binary is in
 * the path. The book it replaced had no recorded origin and could not get one,
 * which is what S146 was, and DEC-131 is the ruling.
 *
 * It used to be `src/openings.book`, a 5220541-byte C header holding one
 * `#define BOOK "<hex>"` that every process decoded into a 2610256-byte heap
 * buffer at startup -- twice the tracked bytes, and an ASCII parse in front of
 * every one of the thousands of engine processes a match night starts. Here the
 * bytes are the file, the linker puts them in read-only data, and the probe
 * reads them where they lie. `openings.cpp` takes them through the two symbols
 * below; nothing is copied and nothing is decoded.
 *
 * CHESSO_BOOK_PATH is the absolute path to `src/openings.bin`, defined by
 * `src/CMakeLists.txt`. It is passed rather than written here so that `.incbin`
 * does not depend on the assembler's include-path search, which differs between
 * toolchains.
 *
 * The file is `.S` and not `.s` so that the C preprocessor runs over it, which
 * is what lets one file serve both object formats: Mach-O prefixes symbols with
 * an underscore and names its read-only section differently from ELF. There is
 * no MSVC arm -- MASM has no `.incbin` and this tree has never been built with
 * it -- so a Windows build would need the generated-array form instead. DEC-130.
 */

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#if defined(__APPLE__)
#define SYMBOL(name) _##name
        .section __TEXT,__const
#else
#define SYMBOL(name) name
        .section .rodata
#endif

/*
 * Eight, because openings.cpp reads a Polyglot key out of these bytes with an
 * eight-byte load. An `.incbin` lands wherever the previous section contents
 * left off unless it is told otherwise.
 */
        .balign 8

        .globl  SYMBOL(chesso_embedded_book_begin)
SYMBOL(chesso_embedded_book_begin):
        .incbin STRINGIFY(CHESSO_BOOK_PATH)
        .globl  SYMBOL(chesso_embedded_book_end)
SYMBOL(chesso_embedded_book_end):

#if !defined(__APPLE__)
/* Without this an object file assembled from source marks the stack executable
 * on ELF, and the linker propagates that to the whole binary. */
        .section .note.GNU-stack,"",%progbits
#endif
