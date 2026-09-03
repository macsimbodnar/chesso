#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include "data_structures.hpp"


// The value of `Book File` that means the book compiled into the binary. UCI
// has no way to say "no path", and an empty string is what a GUI sends when the
// user clears the field, so both are accepted and mean the same thing.
#define BOOK_FILE_EMBEDDED "<embedded>"


struct book_t
{
  // The Polyglot bytes. For the built-in book this points into the binary's own
  // read-only data and `storage` is empty, so probing it copies nothing; for a
  // book read from disk it points into `storage`. Never dereferenced unless
  // `num_of_positions` is non-zero, which only a validated book reaches.
  const uint8_t* data = nullptr;
  size_t num_of_positions = 0;

  std::vector<uint8_t> storage;
};


uint64_t get_key(const board_t* board);
bool load_book_embedded(book_t* book_out);
// `reason` is filled in on failure with a sentence a user can act on -- the
// path did not open, the file is not a Polyglot book -- because the caller is a
// UCI option a person typed a path into and silence there is indistinguishable
// from success.
bool load_book_from_file(const std::string& file,
                         book_t* book,
                         std::string* reason = nullptr);

// Fills `found_moves` with every book move for the position and
// `found_weights` with each one's Polyglot weight, both up to MAX_MOVES, and
// returns how many. The weight is what decides which move is played and is
// returned rather than acted on here: this function knows the book, not the
// engine's options.
size_t get_book_moves_for_key(const book_t* book,
                              const board_t* board,
                              move_t found_moves[],
                              uint16_t found_weights[]);
