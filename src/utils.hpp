#pragma once
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "log.hpp"


struct position_t
{
  uint8_t file;
  uint8_t rank;
};


struct coordinates_t {
  uint8_t x;
  uint8_t y;
};


inline void print_move(const uint8_t from_file,
                       const uint8_t from_rank,
                       const uint8_t to_file,
                       const uint8_t to_rank)
{
  LOG_I << std::string(1, 'a' + from_file) << from_rank + 1 << " -> "
        << std::string(1, 'a' + to_file) << to_rank + 1 << END_I;
}

inline void print_index(const uint8_t index)
{
  const uint8_t file = index & 7;
  const uint8_t rank = index >> 4;

  LOG_I << std::string(1, 'a' + file) << rank + 1 << END_I;
}

template <typename T>
inline std::string PTR2STR(T* ptr)
{
  const void* address = static_cast<const void*>(ptr);
  std::stringstream ss;
  ss << address;
  return ss.str();
}


inline std::vector<std::string> split_string(const std::string& str)
{
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> tokens(begin, end);

  return tokens;
}


inline bool is_uint(const std::string& str)
{
  for (const char c : str) {
    if (!isdigit(c)) { return false; }
  }

  return true;
}
