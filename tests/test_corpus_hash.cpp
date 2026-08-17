#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include "corpus_hash.hpp"

// S077. An emitted weight table names the corpus it was fitted from by content
// hash, because a path is not a corpus: `.tuning/selfplay_v2.tsv` has meant two
// different files on two machines and S082 and S083 write two more.
//
// The hash is SHA-256 written from FIPS 180-4 rather than taken from a library,
// so that the value in the header is checkable by `sha256sum`, which knows
// nothing about this project. That only helps if the implementation is right,
// which is what this file is for.
//
// What it holds:
//
//   1. the three published vectors of FIPS 180-4 -- the empty message, "abc",
//      and the 448-bit two-block message
//   2. one million 'a', the published long-message vector: 15625 blocks, which
//      is the only case here that exercises the block loop at scale
//   3. the chunking property: the same bytes fed in every split from 1 to 200
//      bytes give the same digest, which is what says the buffering is right
//      rather than the arithmetic alone
//   4. a padding sweep across the block boundary: every length from 0 to 200
//      differs from its neighbours, so no length silently hashes as another
//   5. hash_file() over a written fixture agrees with the streaming digest of
//      the same bytes, and reports the byte and row counts
//   6. a row count is lines, terminated or not
//   7. a file that cannot be opened is refused rather than hashed as empty --
//      the failure that would stamp a table with the digest of nothing
//
// Case 1's empty-message vector is the non-vacuity guard for case 7:
// `hash_file` on a missing path must not return that digest, and this file
// states it rather than leaving the two to be compared by eye.

namespace
{

// FIPS 180-4 appendix B, and the long-message vector from the same source.
const std::string EMPTY_DIGEST =
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
const std::string ABC_DIGEST =
    "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
const std::string TWO_BLOCK_MESSAGE =
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
const std::string TWO_BLOCK_DIGEST =
    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
const std::string MILLION_A_DIGEST =
    "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0";


std::string digest_of(const std::string& text)
{
  corpus_hash::sha256_t hash;
  hash.update(text);
  return hash.hex();
}


std::string fixture_path(const std::string& name)
{ return (std::filesystem::temp_directory_path() / name).string(); }


void write_file(const std::string& path, const std::string& content)
{
  std::ofstream out(path, std::ios::binary);
  REQUIRE(out.good());
  out << content;
  out.close();
}

}  // namespace


TEST_CASE("the published vectors")
{
  CHECK(digest_of("") == EMPTY_DIGEST);
  CHECK(digest_of("abc") == ABC_DIGEST);
  CHECK(digest_of(TWO_BLOCK_MESSAGE) == TWO_BLOCK_DIGEST);

  // 56 bytes: one byte past the point where the length field no longer fits in
  // the first block, so this vector is the two-block padding path.
  CHECK(TWO_BLOCK_MESSAGE.size() == 56);
}


TEST_CASE("one million 'a'")
{
  corpus_hash::sha256_t hash;
  const std::string block(1000, 'a');

  for (int i = 0; i < 1000; ++i) {
    hash.update(block);
  }

  CHECK(hash.hex() == MILLION_A_DIGEST);
}


TEST_CASE("the digest does not depend on how the bytes were split")
{
  // Long enough to span several blocks and not a multiple of 64, so every
  // chunk size below lands mid-block somewhere.
  std::string message;
  for (int i = 0; i < 500; ++i) {
    message += static_cast<char>('a' + (i % 26));
  }

  const std::string whole = digest_of(message);

  // Non-vacuous: a digest that ignored its input would pass the loop below and
  // fail this.
  CHECK(whole != EMPTY_DIGEST);

  for (size_t chunk = 1; chunk <= 200; ++chunk) {
    corpus_hash::sha256_t hash;

    for (size_t at = 0; at < message.size(); at += chunk) {
      hash.update(message.substr(at, chunk));
    }

    CAPTURE(chunk);
    CHECK(hash.hex() == whole);
  }
}


TEST_CASE("every length across the block boundary hashes differently")
{
  std::string previous;

  for (size_t length = 0; length <= 200; ++length) {
    const std::string message(length, 'x');
    const std::string digest = digest_of(message);

    CAPTURE(length);
    CHECK(digest != previous);
    CHECK(digest.size() == 64);

    previous = digest;
  }
}


TEST_CASE("hash_file agrees with the streaming digest, and counts rows")
{
  const std::string path = fixture_path("chesso_test_corpus_hash.tsv");
  const std::string content = "one\t1.0\t31\t24\ntwo\t0.0\t-58\t24\n";

  write_file(path, content);

  std::string hex;
  uint64_t bytes = 0;
  uint64_t rows = 0;

  REQUIRE(corpus_hash::hash_file(path, &hex, &bytes, &rows));

  CHECK(hex == digest_of(content));
  CHECK(bytes == content.size());
  CHECK(rows == 2);

  std::filesystem::remove(path);
}


TEST_CASE("an unterminated last line is still a row")
{
  const std::string path = fixture_path("chesso_test_corpus_hash_partial.tsv");

  write_file(path, "one\ntwo");

  std::string hex;
  uint64_t bytes = 0;
  uint64_t rows = 0;

  REQUIRE(corpus_hash::hash_file(path, &hex, &bytes, &rows));

  CHECK(bytes == 7);
  CHECK(rows == 2);

  // And an empty file is zero rows rather than one.
  write_file(path, "");
  REQUIRE(corpus_hash::hash_file(path, &hex, &bytes, &rows));
  CHECK(bytes == 0);
  CHECK(rows == 0);
  CHECK(hex == EMPTY_DIGEST);

  std::filesystem::remove(path);
}


TEST_CASE("a file that cannot be opened is refused, not hashed as empty")
{
  const std::string path = fixture_path("chesso_test_corpus_hash_absent.tsv");
  std::filesystem::remove(path);

  std::string hex = "untouched";
  uint64_t bytes = 12345;
  uint64_t rows = 678;

  CHECK_FALSE(corpus_hash::hash_file(path, &hex, &bytes, &rows));

  // The out parameters are left alone, so a caller that ignores the return
  // value cannot pick up the digest of nothing and stamp it onto a table.
  CHECK(hex == "untouched");
  CHECK(hex != EMPTY_DIGEST);
  CHECK(bytes == 12345);
  CHECK(rows == 678);
}
