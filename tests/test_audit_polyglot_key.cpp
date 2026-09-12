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
//
// S042/DEC-187, 2026-09-12: load_FEN()'s sanitizer gained a further term, its
// own capturer test (en_passant_is_capturable, src/bitboard.cpp) -- an
// en-passant square now survives loading only when a pawn of the side to move
// actually stands where it attacks the target, matching the Polyglot
// specification's own reading (get_key()'s comment above, "irrelevant if the
// potential en passant capturing move is legal", i.e. pseudo-legal). Eight of
// the ten cases below have no such pawn -- the wraparound bug this file guards
// lives entirely in code that get_key() only reaches once board.en_passant is
// already set, so seven non-control cases and the first control ("no white
// pawn on g5 or a4") were built with a pawn on the *wrapped* square and none on
// the *real* one. python-chess 1.11.2's `has_pseudo_legal_en_passant()` says
// which is which; every `key_case_t` below carries that verdict as
// `capturable`, and the case's own precondition is asserted from it: en
// passant survives loading when true, is INVALID_INDEX when false. Either
// way `get_key()` must still equal `spec_key` -- re-derived with
// `chess.polyglot.zobrist_hash()` against each case's fen and unchanged from
// the values already here in every one of the ten (python-chess's own
// hash_ep_square() re-derives capturability from the pawns on the board, not
// from the FEN's fourth field, so it never trusted the over-permissive
// classical convention to begin with). Two new cases, "d6 capturable" and "d3
// capturable", are added on a non-edge file so the ep term of the key --
// on_left/on_right in get_key(), src/openings.cpp -- is still exercised by a
// case with board.en_passant actually set, now that eight of the original ten
// no longer reach that code path at all.
//
// `capturable` and `spec_key` are goldens (DEC-142): re-derive both with
// `~/.venv/chess/bin/python adocs/data/S042_polyglot_key_cases.py
// tests/test_audit_polyglot_key.cpp`, which parses this file's own `cases`
// table rather than carrying a second copy of it, so it stays current across
// an edit here without anyone having to remember to update it there too.

static game_t game;


struct key_case_t
{
  const char* fen;
  uint64_t spec_key;
  bool capturable;
  const char* why;
};


TEST_CASE(
    "the Polyglot key follows the format on an edge-file en-passant square")
{
  initialize_game_const_data(&game);

  // clang-format off
  const std::vector<key_case_t> cases = {
    // Non-control cases: a pawn on the wrapped square, none on the real one --
    // uncapturable, confirmed with python-chess 1.11.2
    // (`has_pseudo_legal_en_passant()` is False on every one of the seven).
    // spec_key re-derived with chess.polyglot.zobrist_hash(): unchanged.
    {"r2qk1nr/pp1nbpp1/2p1p3/3pPb1p/P2P4/2P2N2/1P2BPPP/RNBQ1RK1 b kq a3 0 8", 0x35FDF2C529E5841EULL, false, "a3 ep, black pawn on h5 (a3 - 9 wraps to h5)"},
    {"rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8", 0xBFF7AAF88AAF9FBBULL, false, "h6 ep, white pawn on a4 (h6 + 9 wraps to a4)"},
    {"rnbqkb1r/pp3pp1/2pp1n2/4p2p/P2PP3/2N2P2/1PP1N1PP/R1BQKB1R b KQkq a3 0 6", 0xCA4A060057CF5B4DULL, false, "a3 ep, black pawn on h5"},
    {"r1bqk2r/pp1nppb1/2pp1np1/7p/P2PP3/2N1B1N1/1PP2PPP/R2QKB1R w KQkq h6 0 8", 0xE425C4F582E5EBD3ULL, false, "h6 ep, white pawn on a4"},
    {"rnb1kbnr/2pq1pp1/1p2p3/p2pP2p/P2P1P2/2P5/1P4PP/RNBQKBNR w KQkq h6 0 7", 0xD8D6AE27ABDBBC36ULL, false, "h6 ep, white pawn on a4"},
    {"r1bqk2r/1ppnppb1/p2p1np1/7p/P1PPP3/2N3N1/1P3PPP/R1BQKB1R w KQkq h6 0 8", 0x5B83FC21D030703DULL, false, "h6 ep, white pawn on a4"},
    {"rnbqk2r/p3ppb1/2pp1np1/1p5p/P2PP2P/2N1BP2/1PPQ2P1/R3KBNR b KQkq a3 0 8", 0xAA2989371CD8F709ULL, false, "a3 ep, black pawn on h5"},
    // controls, green before and after the fix
    {"rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/3P1P2/2P5/PP1N2PP/R1BQKBNR w KQkq h6 0 8", 0x2CE8F2BE481BCF7CULL, false, "control: h6 ep, no white pawn on g5 or a4"},
    {"rnb1kb1r/2pqnpp1/1p2p3/p2pP1Pp/P2P4/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8", 0x9EF62758DF5C09EDULL, true, "control: h6 ep, white pawn on g5 captures"},
    {"r2qk1nr/pp1nbpp1/2p1p3/3pPb2/Pp1P4/2P2N2/1P2BPPP/RNBQ1RK1 b kq a3 0 8", 0xBBD23228D44C6412ULL, true, "control: a3 ep, black pawn on b4 captures"},
    // S042: added so the ep term of the key is still exercised on a non-edge
    // file now that eight of the ten cases above load with en_passant ==
    // INVALID_INDEX. Both capturable, confirmed with python-chess 1.11.2
    // (`has_pseudo_legal_en_passant()` is True, xfen keeps the square);
    // spec_key from chess.polyglot.zobrist_hash(). d6: white pawn on e5
    // stands beside black's fresh double push to d5 (1. e4 e6 2. e5 d5). d3:
    // black pawn on e4 stands beside white's fresh double push to d4, the
    // same shape with the sides reversed, so get_key()'s -8 branch is
    // exercised too, not only its +8 one.
    {"rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3", 0x0CC1835B41412927ULL, true, "d6 ep, white pawn on e5 captures (non-edge file)"},
    {"rnbqkbnr/pppp1ppp/8/8/3Pp3/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 4", 0x3AE952459A0066BFULL, true, "d3 ep, black pawn on e4 captures (non-edge file)"},
  };
  // clang-format on

  for (const key_case_t& c : cases) {
    REQUIRE_MESSAGE(load_FEN(c.fen, &game), c.fen);

    // Precondition, restated per case for S042: the sanitizer keeps the
    // en-passant square only when python-chess's own pseudo-legal-capture
    // test agrees a pawn of the side to move could take it, and clears it to
    // INVALID_INDEX otherwise. Either way get_key() below must still equal
    // spec_key -- a case that never carries the square is still a case where
    // the format says the key must not carry the ep term, and that is worth
    // asserting, not skipping.
    if (c.capturable) {
      REQUIRE_MESSAGE(game.board.en_passant != INVALID_INDEX, c.fen);
    } else {
      REQUIRE_MESSAGE(game.board.en_passant == INVALID_INDEX, c.fen);
    }

    CHECK_MESSAGE(get_key(&game.board) == c.spec_key,
                  (std::string(c.fen) + " -- " + c.why));
  }
}
