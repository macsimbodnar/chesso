#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cstdint>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "openings.hpp"

// 2026-09-03_adversarial-F01. Written by the audit before any fix and observed
// red at 1d8cbac -- 7 of the 10 cases below fail. Not registered in
// tests/CMakeLists.txt; the fix registers it and observes it green.
//
// get_key() in src/openings.cpp decides whether the Polyglot key carries the
// en-passant component by looking for a pawn of the side to move on
// `en_passant + 7` and `en_passant + 9` (White to move) or `en_passant - 9` and
// `en_passant - 7` (Black to move). On the a8 = 0 index scheme those offsets
// wrap round the edge of the board when the en-passant square is on the a- or
// h-file: for `h6` White's +9 lands on a4, for `a6` White's +7 lands on h6, for
// `a3` Black's -9 lands on h5, for `h3` Black's -7 lands on a3. A same-side
// pawn standing on the wrapped square makes the key include a component the
// format says it must not carry -- the specification asks for "a pawn next to
// it belonging to the player to move", next to the pushed pawn on its own rank.
//
// The expected keys are python-chess 1.11.2's chess.polyglot.zobrist_hash(),
// an independent implementation of the same specification. The seven failing
// positions are the seven entries of src/openings.bin whose keys disagree with
// that implementation over all 555200 plies of books/8moves_v3.pgn; the three
// controls are the same shapes with the wrapped square empty or with a genuine
// capturer beside the pushed pawn, where the two implementations agree. The
// en-passant square is written into each FEN so that load_FEN()'s sanitizer
// (S161) keeps it: the victim pawn is on the board in every case.

static game_t game;


struct key_case_t
{
  const char* fen;
  uint64_t spec_key;
  const char* why;
};


TEST_CASE(
    "the Polyglot key follows the format on an edge-file en-passant square")
{
  initialize_game_const_data(&game);

  // clang-format off
  const std::vector<key_case_t> cases = {
    {"r2qk1nr/pp1nbpp1/2p1p3/3pPb1p/P2P4/2P2N2/1P2BPPP/RNBQ1RK1 b kq a3 0 8", 0x35FDF2C529E5841EULL, "a3 ep, black pawn on h5 (a3 - 9 wraps to h5)"},
    {"rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8", 0xBFF7AAF88AAF9FBBULL, "h6 ep, white pawn on a4 (h6 + 9 wraps to a4)"},
    {"rnbqkb1r/pp3pp1/2pp1n2/4p2p/P2PP3/2N2P2/1PP1N1PP/R1BQKB1R b KQkq a3 0 6", 0xCA4A060057CF5B4DULL, "a3 ep, black pawn on h5"},
    {"r1bqk2r/pp1nppb1/2pp1np1/7p/P2PP3/2N1B1N1/1PP2PPP/R2QKB1R w KQkq h6 0 8", 0xE425C4F582E5EBD3ULL, "h6 ep, white pawn on a4"},
    {"rnb1kbnr/2pq1pp1/1p2p3/p2pP2p/P2P1P2/2P5/1P4PP/RNBQKBNR w KQkq h6 0 7", 0xD8D6AE27ABDBBC36ULL, "h6 ep, white pawn on a4"},
    {"r1bqk2r/1ppnppb1/p2p1np1/7p/P1PPP3/2N3N1/1P3PPP/R1BQKB1R w KQkq h6 0 8", 0x5B83FC21D030703DULL, "h6 ep, white pawn on a4"},
    {"rnbqk2r/p3ppb1/2pp1np1/1p5p/P2PP2P/2N1BP2/1PPQ2P1/R3KBNR b KQkq a3 0 8", 0xAA2989371CD8F709ULL, "a3 ep, black pawn on h5"},
    // controls, green before and after the fix
    {"rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/3P1P2/2P5/PP1N2PP/R1BQKBNR w KQkq h6 0 8", 0x2CE8F2BE481BCF7CULL, "control: h6 ep, no white pawn on g5 or a4"},
    {"rnb1kb1r/2pqnpp1/1p2p3/p2pP1Pp/P2P4/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8", 0x9EF62758DF5C09EDULL, "control: h6 ep, white pawn on g5 captures"},
    {"r2qk1nr/pp1nbpp1/2p1p3/3pPb2/Pp1P4/2P2N2/1P2BPPP/RNBQ1RK1 b kq a3 0 8", 0xBBD23228D44C6412ULL, "control: a3 ep, black pawn on b4 captures"},
  };
  // clang-format on

  for (const key_case_t& c : cases) {
    REQUIRE_MESSAGE(load_FEN(c.fen, &game), c.fen);

    // Precondition: the sanitizer kept the en-passant square. Without it the
    // case would compare two keys that never look at the file at all.
    REQUIRE_MESSAGE(game.board.en_passant != INVALID_INDEX, c.fen);

    CHECK_MESSAGE(get_key(&game.board) == c.spec_key,
                  (std::string(c.fen) + " -- " + c.why));
  }
}
