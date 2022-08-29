#pragma once
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace
{

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


}  // namespace