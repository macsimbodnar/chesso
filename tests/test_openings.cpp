#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "openings.hpp"


static game_t game;


TEST_SUITE("Test openings")
{
  TEST_CASE("Initialize")
  { initialize_game_const_data(&game); }

  TEST_CASE("Test key generation")
  {
    // clang-format off
    std::unordered_map<std::string, uint64_t> test_cases = {
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5060803636482931868ULL},
      {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 9384546495678726550ULL},
      {"rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", 528813709611831216ULL},
      {"rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2", 7363297126586722772ULL},
      {"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", 2496273314520498040ULL},
      {"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3", 7289745035295343297ULL},
      {"rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4", 71445182323015129ULL},
      {"rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3", 4359805404264691255ULL},
      {"rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4", 6647202560273257824ULL}
    };
    // clang-format on

    for (const auto& test_case : test_cases) {
      const std::string& pos = test_case.first;
      const uint64_t expected_key = test_case.second;

      load_FEN(pos, &game);

      const uint64_t key = get_key(&game.board);
      REQUIRE_EQ(key, expected_key);
    }
  }

  TEST_CASE("Test get moves")
  {
    load_FEN(DEFAULT_POSITION, &game);
    book_t book;
    load_book_embedded(&book);

    move_t moves[MAX_MOVES];
    uint16_t weights[MAX_MOVES];
    const size_t moves_cout =
        get_book_moves_for_key(&book, &game.board, moves, weights);
    REQUIRE(moves_cout > 0);

    REQUIRE(make_move(&game, moves[0]));
  }

  // S172. Everything below is the file-loading half of the book, which had none
  // of this before: load_book_from_file() had existed since the bitboard branch
  // with no caller anywhere in the tree and had therefore never run.

  TEST_CASE("Test weights come back with the moves")
  {
    load_FEN(DEFAULT_POSITION, &game);
    book_t book;
    REQUIRE(load_book_embedded(&book));

    move_t moves[MAX_MOVES];
    uint16_t weights[MAX_MOVES];
    const size_t count =
        get_book_moves_for_key(&book, &game.board, moves, weights);
    REQUIRE(count > 0);

    // The opening position is the most played position there is, so every entry
    // for it carries a weight. A book whose weights all read zero would still
    // pass a move-count check and would silently turn the weighted draw into a
    // uniform one.
    uint64_t total = 0;
    for (size_t i = 0; i < count; ++i) {
      total += weights[i];
    }

    REQUIRE(total > 0);
  }

  // The binary search replaced a scan over every entry in the book. It is only
  // equal to the scan while the keys are sorted, which is why the loader
  // checks, and this is the check that the two agree on the shipped book.
  TEST_CASE("Test the search finds what a scan finds")
  {
    book_t book;
    REQUIRE(load_book_embedded(&book));

    const std::vector<std::string> positions = {
        DEFAULT_POSITION,
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
        "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
        // A position no book line reaches, so the empty answer is covered too.
        "8/8/8/4k3/8/4K3/8/8 w - - 0 1",
    };

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      move_t moves[MAX_MOVES];
      uint16_t weights[MAX_MOVES];
      const size_t found =
          get_book_moves_for_key(&book, &game.board, moves, weights);

      // The scan the probe used to be, rebuilt here over the same bytes.
      const uint64_t key = get_key(&game.board);
      size_t scanned = 0;

      for (size_t i = 0; i < book.num_of_positions; ++i) {
        uint64_t entry_key = 0;
        memcpy(&entry_key, book.data + (i * 16), sizeof(entry_key));

        // The file is big endian; the comparison only has to be consistent.
        uint64_t host_key = 0;
        for (size_t b = 0; b < 8; ++b) {
          host_key = (host_key << 8) | book.data[(i * 16) + b];
        }

        if (host_key == key) { ++scanned; }
      }

      REQUIRE_EQ(found, scanned);
    }
  }

  TEST_CASE("Test a book is loaded from a file")
  {
    const std::string path =
        (std::filesystem::temp_directory_path() / "chesso_s172_good.bin")
            .string();

    // Two entries for the opening position, e2e4 heavier than d2d4, written by
    // hand so the test does not depend on tools/make_book.
    const uint8_t entries[32] = {
        0x46, 0x3b, 0x96, 0x18, 0x16, 0x91, 0xfc, 0x9c,  // the startpos key
        0x03, 0x1c, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00,  // e2e4, weight 100
        0x46, 0x3b, 0x96, 0x18, 0x16, 0x91, 0xfc, 0x9c,
        0x02, 0x1b, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00,  // d2d4, weight 10
    };

    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(entries), sizeof(entries));
    out.close();

    book_t book;
    std::string reason;
    REQUIRE(load_book_from_file(path, &book, &reason));
    REQUIRE_EQ(book.num_of_positions, 2);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    move_t moves[MAX_MOVES];
    uint16_t weights[MAX_MOVES];
    const size_t count =
        get_book_moves_for_key(&book, &game.board, moves, weights);

    REQUIRE_EQ(count, 2);
    REQUIRE_EQ(weights[0], 100);
    REQUIRE_EQ(weights[1], 10);
    REQUIRE(make_move(&game, moves[0]));

    std::filesystem::remove(path);
  }

  // A path a user typed is pointed at a PGN, at a JPEG and at a half-downloaded
  // book. A probe over arbitrary bytes does not fail -- it returns arbitrary
  // moves that the engine reports as its best move without ever searching them
  // -- so each of these has to be refused at load.
  TEST_CASE("Test a book that is not a book is refused")
  {
    const std::string path =
        (std::filesystem::temp_directory_path() / "chesso_s172_bad.bin")
            .string();

    book_t book;
    std::string reason;

    SUBCASE("a path that does not open")
    {
      REQUIRE(!load_book_from_file(path + ".missing", &book, &reason));
      REQUIRE(reason.find("could not be opened") != std::string::npos);
    }

    SUBCASE("an empty file")
    {
      std::ofstream out(path, std::ios::binary);
      out.close();

      REQUIRE(!load_book_from_file(path, &book, &reason));
      REQUIRE(reason.find("empty") != std::string::npos);
    }

    SUBCASE("a size that is not a whole number of entries")
    {
      const uint8_t bytes[20] = {};
      std::ofstream out(path, std::ios::binary);
      out.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
      out.close();

      REQUIRE(!load_book_from_file(path, &book, &reason));
      REQUIRE(reason.find("not a whole number") != std::string::npos);
    }

    SUBCASE("keys that are not sorted")
    {
      const uint8_t bytes[32] = {
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x1c, 0x00,
          0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x01, 0x03, 0x1c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      };

      std::ofstream out(path, std::ios::binary);
      out.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
      out.close();

      REQUIRE(!load_book_from_file(path, &book, &reason));
      REQUIRE(reason.find("not sorted") != std::string::npos);
    }

    // A refused book leaves nothing behind for the probe to walk into.
    REQUIRE_EQ(book.num_of_positions, 0);
    REQUIRE(book.data == nullptr);

    std::filesystem::remove(path);
  }
}
