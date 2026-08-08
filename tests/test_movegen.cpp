#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "test_helpers.hpp"
#include "utils.hpp"


static game_t game;


struct movegen_fixture_t
{
  movegen_fixture_t() { initialize_game_const_data(&game); }
};


TEST_SUITE("movegen: move encoding")
{
  // The move_t bit layout is documented in a comment and used by a dozen
  // macros. Nothing else checks that the fields do not overlap.
  TEST_CASE("every field survives a round trip")
  {
    const std::vector<promotion_t> promotions = {TO_NONE, TO_KNIGHT, TO_BISHOP,
                                                 TO_ROOK, TO_QUEEN};

    for (int from = 0; from < 64; ++from) {
      for (int to = 0; to < 64; ++to) {
        for (int piece = W_PAWN; piece <= B_KING; ++piece) {
          for (const promotion_t promotion : promotions) {
            for (int flags = 0; flags < 16; ++flags) {
              const int capture = (flags >> 0) & 1;
              const int double_push = (flags >> 1) & 1;
              const int en_passant = (flags >> 2) & 1;
              const int castling = (flags >> 3) & 1;

              const move_t move = NEW_MOVE(from, to, piece, promotion, capture,
                                           double_push, en_passant, castling);

              REQUIRE_EQ(MOVE_FROM(move), from);
              REQUIRE_EQ(MOVE_TO(move), to);
              REQUIRE_EQ(MOVE_PIECE(move), piece);
              REQUIRE_EQ(MOVE_PROMOTED(move), promotion);
              REQUIRE_EQ(MOVE_CAPTURE(move) != 0, capture != 0);
              REQUIRE_EQ(MOVE_DOUBLE_PUSH(move) != 0, double_push != 0);
              REQUIRE_EQ(MOVE_EN_PASSANT(move) != 0, en_passant != 0);
              REQUIRE_EQ(MOVE_CASTLING(move) != 0, castling != 0);
            }
          }
        }
      }
    }
  }

  // score_move() indexes piece_values_abs[] with MOVE_PROMOTED(), which only
  // works because the two enums line up.
  TEST_CASE("promotion_t maps onto the white piece values")
  {
    REQUIRE_EQ(static_cast<int>(TO_KNIGHT), static_cast<int>(W_KNIGHT));
    REQUIRE_EQ(static_cast<int>(TO_BISHOP), static_cast<int>(W_BISHOP));
    REQUIRE_EQ(static_cast<int>(TO_ROOK), static_cast<int>(W_ROOK));
    REQUIRE_EQ(static_cast<int>(TO_QUEEN), static_cast<int>(W_QUEEN));
  }
}


TEST_SUITE("movegen: generation")
{
  TEST_CASE_FIXTURE(movegen_fixture_t, "never exceeds MAX_MOVES")
  {
    for (const std::string& fen : all_test_fens()) {
      REQUIRE(load_FEN(fen, &game));

      move_t moves[MAX_MOVES];
      const size_t count = generate_moves(&game.tables, &game.board, moves);

      REQUIRE_MESSAGE(count < MAX_MOVES, ("FEN: " + fen));
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "castling")
  {
    struct case_t
    {
      std::string fen;
      std::string title;
      bool king_side;
      bool queen_side;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "both sides free",            true,  true},
      {"r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", "both sides free, black",     true,  true},
      {"r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1",    "no rights",                  false, false},
      {"r3k2r/8/8/8/8/8/8/R3K1NR w KQkq - 0 1","king side blocked by knight", false, true},
      {"r3k2r/8/8/8/8/8/8/RN2K2R w KQkq - 0 1","queen side blocked on b1",   true,  false},
      {"r3k2r/8/8/8/8/4r3/8/R3K2R w KQkq - 0 1","king in check",             false, false},
      {"r3k2r/8/8/8/8/5r2/8/R3K2R w KQkq - 0 1","f1 attacked",               false, true},
      {"r3k2r/8/8/8/8/3r4/8/R3K2R w KQkq - 0 1","d1 attacked",               true,  false},
      // b1 attacked is legal to castle through: only e1, d1 and c1 matter.
      {"r3k2r/8/8/8/8/1r6/8/R3K2R w KQkq - 0 1","b1 attacked is still legal",true,  true},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE(load_FEN(test.fen, &game));

      const bool white = (game.board.active_color == WHITE);
      const index_t king_from = white ? e1 : e8;
      const index_t king_to = white ? g1 : g8;
      const index_t queen_to = white ? c1 : c8;
      const piece_t king = white ? W_KING : B_KING;

      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      bool found_king_side = false;
      bool found_queen_side = false;

      for (size_t i = 0; i < count; ++i) {
        if (!MOVE_CASTLING(moves[i])) { continue; }

        REQUIRE_EQ(MOVE_PIECE(moves[i]), king);
        REQUIRE_EQ(MOVE_FROM(moves[i]), king_from);

        if (MOVE_TO(moves[i]) == king_to) { found_king_side = true; }
        if (MOVE_TO(moves[i]) == queen_to) { found_queen_side = true; }
      }

      REQUIRE_MESSAGE(found_king_side == test.king_side, test.title);
      REQUIRE_MESSAGE(found_queen_side == test.queen_side, test.title);
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "castling rights are lost")
  {
    // Capturing the h1 rook must clear White's king side right. The bishop
    // sits on g2 so that h1 is on its diagonal.
    REQUIRE(load_FEN("r3k2r/8/8/8/8/8/6b1/R3K2R b KQkq - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    bool played = false;
    for (size_t i = 0; i < count; ++i) {
      if (MOVE_FROM(moves[i]) != g2 || MOVE_TO(moves[i]) != h1) { continue; }

      REQUIRE(make_move(&game, moves[i]));
      played = true;
      break;
    }

    REQUIRE(played);
    REQUIRE_EQ(game.board.castling & WK, 0);
    REQUIRE_NE(game.board.castling & WQ, 0);
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "en passant")
  {
    SUBCASE("available right after the double push")
    {
      REQUIRE(load_FEN("8/8/8/3pP3/8/8/8/4K2k w - d6 0 1", &game));

      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      bool found = false;
      for (size_t i = 0; i < count; ++i) {
        if (MOVE_EN_PASSANT(moves[i])) {
          REQUIRE_EQ(MOVE_FROM(moves[i]), e5);
          REQUIRE_EQ(MOVE_TO(moves[i]), d6);
          REQUIRE(MOVE_CAPTURE(moves[i]));
          found = true;
        }
      }

      REQUIRE(found);
    }

    SUBCASE("gone once the square is not set")
    {
      REQUIRE(load_FEN("8/8/8/3pP3/8/8/8/4K2k w - - 0 1", &game));

      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      // Without this the assertion below is satisfied by an empty move list.
      REQUIRE(count > 0);

      for (size_t i = 0; i < count; ++i) {
        REQUIRE_FALSE(MOVE_EN_PASSANT(moves[i]));
      }
    }

    SUBCASE("illegal when it exposes the king along the rank")
    {
      // The classic case: taking en passant removes two pawns from the fifth
      // rank at once and the black rook on h5 then hits the white king.
      REQUIRE(load_FEN("8/8/8/K1pP3r/8/8/8/7k w - c6 0 1", &game));

      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      REQUIRE(count > 0);

      for (size_t i = 0; i < count; ++i) {
        REQUIRE_FALSE(MOVE_EN_PASSANT(moves[i]));
      }
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "promotion emits all four pieces")
  {
    SUBCASE("quiet push")
    {
      REQUIRE(load_FEN("8/4P3/8/8/8/8/8/4K2k w - - 0 1", &game));

      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      int promotions = 0;
      bool seen[5] = {};

      for (size_t i = 0; i < count; ++i) {
        if (MOVE_PROMOTED(moves[i]) == TO_NONE) { continue; }

        REQUIRE_EQ(MOVE_TO(moves[i]), e8);
        seen[MOVE_PROMOTED(moves[i])] = true;
        promotions++;
      }

      REQUIRE_EQ(promotions, 4);
      REQUIRE(seen[TO_QUEEN]);
      REQUIRE(seen[TO_ROOK]);
      REQUIRE(seen[TO_BISHOP]);
      REQUIRE(seen[TO_KNIGHT]);
    }

    SUBCASE("capture promotion, black")
    {
      // The white king is on g1, not f1: on f1 it would be attacked by the e2
      // pawn while Black is to move, which is not a legal position.
      REQUIRE(load_FEN("4k3/8/8/8/8/8/4p3/3R2K1 b - - 0 1", &game));

      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      int capture_promotions = 0;

      for (size_t i = 0; i < count; ++i) {
        if (MOVE_PROMOTED(moves[i]) == TO_NONE) { continue; }
        if (!MOVE_CAPTURE(moves[i])) { continue; }

        REQUIRE_EQ(MOVE_TO(moves[i]), d1);
        capture_promotions++;
      }

      REQUIRE_EQ(capture_promotions, 4);
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "a pinned piece cannot leave the ray")
  {
    // The knight on e4 is pinned by the rook on e8 against the king on e1.
    REQUIRE(load_FEN("4r3/8/8/8/4N3/8/8/4K3 w - - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    for (size_t i = 0; i < count; ++i) {
      REQUIRE_NE(MOVE_PIECE(moves[i]), W_KNIGHT);
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "only evasions when in check")
  {
    // The rook on e2 checks along the e file. The king can take it, or step
    // off the file to d1 or f1. d2 and f2 stay on the rook's rank and e2 is
    // the only capture, so the legal reply set is exactly three moves.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    REQUIRE_EQ(count, 3);

    bool seen[64] = {};

    for (size_t i = 0; i < count; ++i) {
      REQUIRE_EQ(MOVE_PIECE(moves[i]), W_KING);
      REQUIRE_EQ(MOVE_FROM(moves[i]), e1);
      seen[MOVE_TO(moves[i])] = true;
    }

    REQUIRE(seen[e2]);
    REQUIRE(seen[d1]);
    REQUIRE(seen[f1]);
  }
}


TEST_SUITE("movegen: perft")
{
  // test_perft carries the deep runs and takes minutes, so it is labelled slow
  // and nobody runs it by hand. This is a second, independent walk of the same
  // expectations, cut off at whatever depth stays cheap, and it checks the
  // columns the deep run prints but never compares: captures, en passant,
  // castles and promotions. A generator that produces the right number of
  // moves for the wrong reasons fails here.

  struct perft_counts_t
  {
    uint64_t nodes = 0;
    uint64_t captures = 0;
    uint64_t en_passant = 0;
    uint64_t castles = 0;
    uint64_t promotions = 0;
  };

  // The counters are the ones on the leaf moves, which is the convention every
  // published perft table uses.
  static void perft(game_t * g, int depth, perft_counts_t* out)
  {
    if (depth == 0) {
      out->nodes++;
      return;
    }

    move_t moves[MAX_MOVES];
    const size_t count = generate_moves(&g->tables, &g->board, moves);

    for (size_t i = 0; i < count; ++i) {
      if (!make_move(g, moves[i])) { continue; }

      if (depth == 1) {
        out->nodes++;
        if (MOVE_CAPTURE(moves[i])) { out->captures++; }
        if (MOVE_EN_PASSANT(moves[i])) { out->en_passant++; }
        if (MOVE_CASTLING(moves[i])) { out->castles++; }
        if (MOVE_PROMOTED(moves[i]) != TO_NONE) { out->promotions++; }
      } else {
        perft(g, depth - 1, out);
      }

      unmake_move(g);
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "shallow perft matches every column")
  {
    // Anything past this costs more than the fast suite is allowed to.
    const uint64_t node_ceiling = 5000000;

    size_t layers_checked = 0;
    size_t columns_checked = 0;

    for (const std::string& file : {"assets/perft_json/perft.json",
                                    "assets/perft_json/talkchess_perft.json"}) {
      const nlohmann::json positions = load_json(file);

      for (const nlohmann::json& position : positions) {
        const std::string fen = position["start_fen"].get<std::string>();

        for (const nlohmann::json& layer : position["depth_layers"]) {
          const int depth = layer["depth"].get<int>();
          const uint64_t nodes = layer["nodes"].get<uint64_t>();

          if (nodes > node_ceiling) { continue; }

          REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

          perft_counts_t counts;
          perft(&game, depth, &counts);

          const std::string title =
              file + "\nFEN: " + fen + "\ndepth " + std::to_string(depth);

          REQUIRE_MESSAGE(counts.nodes == nodes, (title + " nodes"));

          // Only some of the published tables carry the extra columns; the
          // rest are null and there is nothing to compare against.
          const auto compare = [&](const char* column, uint64_t actual) {
            if (layer[column].is_null()) { return; }

            REQUIRE_MESSAGE(
                actual == layer[column].get<uint64_t>(),
                (title + " " + column + ": got " + std::to_string(actual) +
                 " expected " + layer[column].dump()));
            columns_checked++;
          };

          compare("captures", counts.captures);
          compare("en_passant", counts.en_passant);
          compare("castles", counts.castles);
          compare("promotions", counts.promotions);

          // The board has to come back untouched, or every later layer is
          // measuring something else.
          REQUIRE_MESSAGE(generate_FEN(&game.board) == fen, (title + " board"));

          layers_checked++;
        }
      }
    }

    // Guards against a schema change quietly turning this into a no-op.
    REQUIRE(layers_checked > 40);
    REQUIRE(columns_checked > 80);
  }
}


TEST_SUITE("movegen: FEN validation")
{
  // Regressions for the out-of-bounds reads and undefined shifts that a
  // malformed FEN used to trigger. Every one of these was caught by
  // AddressSanitizer before the bounds checks went in.
  TEST_CASE_FIXTURE(movegen_fixture_t, "malformed FENs are rejected")
  {
    // clang-format off
    const std::vector<std::pair<std::string, std::string>> bad = {
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a9 0 1",  "en passant rank 9"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq z3 0 1",  "en passant file z"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNRR w KQkq - 0 1",  "nine pieces on a rank"},
      {"rnbqkbnr/pppppppp/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "nine ranks"},
      {"rnbqkbn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",    "short rank"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1",            "seven ranks"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 300 1", "halfmove clock 300"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0",   "fullmove 0"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1",   "bad side to move"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkqX - 0 1",  "bad castling section"},
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0",     "missing section"},
    };
    // clang-format on

    for (const auto& [fen, title] : bad) {
      REQUIRE_MESSAGE(load_FEN(fen, &game) == false, title);
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "well formed FENs still load")
  {
    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));
      REQUIRE_EQ(generate_FEN(&game.board), fen);
    }
  }

  TEST_CASE_FIXTURE(movegen_fixture_t, "halfmove clock at the uint8 boundary")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 255 200", &game));
    REQUIRE_EQ(game.board.halfmove_clock, 255);

    REQUIRE_FALSE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 256 200", &game));
  }
}
