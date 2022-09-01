#pragma once
#include <array>
#include <cstdint>
#include <vector>

static const std::array<uint8_t, 2> offsets_p = {0x10, 0x20};


class piece_t
{
private:
  char _p = 0;
  uint8_t _index = 0;
  uint8_t _file = 0;
  uint8_t _rank = 0;

public:
  piece_t(const uint8_t file, const uint8_t rank, const char c)
      : _file(file), _rank(rank), _p(c), _index((rank << 4) + file)
  {}

  inline uint8_t file() const { return _file; }
  inline uint8_t rank() const { return _rank; }
  inline uint8_t index() const { return _index; }
  inline char c() const { return _p; }

  inline void set_file(const uint8_t f) { _file = f; }
  inline void set_rank(const uint8_t r) { _rank = r; }
  inline void set_index(const uint8_t i) { _index = i; }

  inline std::vector<uint8_t> get_all_moves()
  {
    std::vector<uint8_t> result;

    switch (_p) {
      case 'p':
        result.reserve(offsets_p.size());
        for (const auto I : offsets_p) {
          result.push_back(_index - I);
        }
        break;

      case 'P':
        result.reserve(offsets_p.size());
        for (const auto I : offsets_p) {
          result.push_back(_index + I);
        }
        break;

      default:
        break;
    }
    return result;
  }
};