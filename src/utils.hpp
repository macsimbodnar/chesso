#pragma once
#include <iterator>
#include <sstream>
#include <vector>

#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

#define U32(x) static_cast<uint32_t>(x)
#define INT(x) static_cast<int>(x)


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

