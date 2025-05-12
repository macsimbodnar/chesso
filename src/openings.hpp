#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include "data_structures.hpp"


struct book_t
{
  std::vector<uint8_t> book;
  size_t num_of_positions;
};


uint64_t get_key(const board_t* board);
bool load_book_embedded(book_t* book_out);
bool load_book_from_file(const std::string& file, book_t* book);
size_t get_book_moves_for_key(const book_t* book,
                              const board_t* board,
                              move_t found_moves[]);