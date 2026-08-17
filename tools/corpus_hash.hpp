#pragma once
// SHA-256 over a corpus file, so an emitted weight table can name the data it
// was fitted from rather than the path it happened to be read from. S077.
//
// `adocs/eval_tuning_strategy.md` section 10: "Version every weight vector
// alongside the git commit of the engine and the dataset hash that produced it.
// Untraceable weights are unreproducible results." A path is not a corpus --
// `.tuning/selfplay_v2.tsv` has already meant two different files on two
// machines, `selfplay_v1.tsv` did not survive DEC-049, and S082 and S083 write
// two more that a path cannot tell apart.
//
// **SHA-256, over the file's bytes exactly as they are on disk**, and it is
// written out here rather than taken from a library for one reason that is
// worth the hundred lines: the value the header carries is then checkable by a
// tool that knows nothing about this project.
//
//   sha256sum .tuning/selfplay_v2_dedup.tsv
//
// must print what the emitted table says. A 64-bit hash of our own invention
// would be reproducible only by the binary that produced it, which is a weaker
// claim than the one this step is for. DEC-066.
//
// The implementation is FIPS 180-4 section 6.2 written from the specification.
// `tests/test_corpus_hash.cpp` holds it against the published vectors, against
// the one-million-'a' streaming case, and against a chunking property -- the
// same bytes fed in different splits must give the same digest, which is what
// says the buffering is right rather than the arithmetic alone.
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace corpus_hash
{

// FIPS 180-4 section 4.2.2: the first 32 bits of the fractional parts of the
// cube roots of the first 64 primes.
constexpr uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};


inline uint32_t rotr(uint32_t value, int bits)
{ return (value >> bits) | (value << (32 - bits)); }


// One 64-byte block into the state, FIPS 180-4 section 6.2.2.
inline void compress(uint32_t* state, const uint8_t* block)
{
  uint32_t w[64];

  for (int i = 0; i < 16; ++i) {
    w[i] = (static_cast<uint32_t>(block[i * 4 + 0]) << 24) |
           (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
           (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
           static_cast<uint32_t>(block[i * 4 + 3]);
  }

  for (int i = 16; i < 64; ++i) {
    const uint32_t s0 =
        rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
    const uint32_t s1 =
        rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);

    w[i] = w[i - 16] + s0 + w[i - 7] + s1;
  }

  uint32_t a = state[0];
  uint32_t b = state[1];
  uint32_t c = state[2];
  uint32_t d = state[3];
  uint32_t e = state[4];
  uint32_t f = state[5];
  uint32_t g = state[6];
  uint32_t h = state[7];

  for (int i = 0; i < 64; ++i) {
    const uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
    const uint32_t choice = (e & f) ^ (~e & g);
    const uint32_t temp1 = h + s1 + choice + K[i] + w[i];
    const uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
    const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
    const uint32_t temp2 = s0 + majority;

    h = g;
    g = f;
    f = e;
    e = d + temp1;
    d = c;
    c = b;
    b = a;
    a = temp1 + temp2;
  }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  state[5] += f;
  state[6] += g;
  state[7] += h;
}


// Streaming, because the corpora this is for are hundreds of megabytes and
// nothing here needs them in memory at once.
struct sha256_t
{
  // FIPS 180-4 section 5.3.3: the first 32 bits of the fractional parts of the
  // square roots of the first 8 primes.
  uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  uint8_t buffer[64] = {};
  size_t buffered = 0;
  uint64_t length = 0;  // message bytes, for the padded length field

  void update(const uint8_t* data, size_t size)
  {
    length += size;

    // Fill a partial block first, so a caller's chunk boundaries cannot change
    // the digest. The chunking property in the test is this line.
    if (buffered > 0) {
      const size_t take = (size < 64 - buffered) ? size : 64 - buffered;

      memcpy(buffer + buffered, data, take);
      buffered += take;
      data += take;
      size -= take;

      if (buffered < 64) { return; }

      compress(state, buffer);
      buffered = 0;
    }

    while (size >= 64) {
      compress(state, data);
      data += 64;
      size -= 64;
    }

    memcpy(buffer, data, size);
    buffered = size;
  }

  void update(const std::string& text)
  { update(reinterpret_cast<const uint8_t*>(text.data()), text.size()); }

  // The padded tail, then the digest as lower-case hex. Consumes the state, so
  // an instance is finished once.
  std::string hex()
  {
    const uint64_t bits = length * 8;

    uint8_t padding[72] = {0x80};
    // A block ends with an 8-byte length, so the 0x80 and the zeros have to
    // carry the message to 56 mod 64.
    const size_t zeros = ((buffered % 64) < 56) ? (56 - (buffered % 64))
                                                : (120 - (buffered % 64));

    for (int i = 0; i < 8; ++i) {
      padding[zeros + i] = static_cast<uint8_t>(bits >> (56 - 8 * i));
    }

    update(padding, zeros + 8);

    char text[65];

    for (int i = 0; i < 8; ++i) {
      snprintf(text + i * 8, 9, "%08x", state[i]);
    }

    return std::string(text, 64);
  }
};


// The digest of a whole file, with the bytes and the rows it holds. A row is a
// line: every '\n' in the file, plus one more when the last line is not
// terminated -- which `tools/datagen` never writes, and which is counted rather
// than assumed.
//
// False when the file cannot be opened or read, so a fit refuses to stamp a
// corpus it could not measure.
inline bool hash_file(const std::string& path,
                      std::string* hex,
                      uint64_t* bytes,
                      uint64_t* rows)
{
  FILE* file = fopen(path.c_str(), "rb");

  if (file == nullptr) { return false; }

  sha256_t digest;
  std::vector<uint8_t> chunk(1 << 20);

  uint64_t total = 0;
  uint64_t lines = 0;
  uint8_t last = '\n';

  for (;;) {
    const size_t read = fread(chunk.data(), 1, chunk.size(), file);

    if (read == 0) { break; }

    digest.update(chunk.data(), read);
    total += read;

    for (size_t i = 0; i < read; ++i) {
      if (chunk[i] == '\n') { lines++; }
    }

    last = chunk[read - 1];
  }

  const bool failed = (ferror(file) != 0);
  fclose(file);

  if (failed) { return false; }

  if (total > 0 && last != '\n') { lines++; }

  *hex = digest.hex();
  *bytes = total;
  *rows = lines;
  return true;
}

}  // namespace corpus_hash
