#pragma once
#include <cstdint>
#include <iostream>

class piece
{
private:
  char _p = 0;
  uint8_t _file = 0;
  uint8_t _rank = 0;

public:
  piece(const uint8_t file, const uint8_t rank, const char c)
      : _file(file), _rank(rank), _p(c)
  {}

  // ~piece() { std::cout << "destroyed " << _p << std::endl; }

  inline uint8_t file() const { return _file; }
  inline uint8_t rank() const { return _rank; }
  inline char c() const { return _p; }

  inline void set_file(const uint8_t f) { _file = f; }
  inline void set_rank(const uint8_t r) { _rank = r; }
};