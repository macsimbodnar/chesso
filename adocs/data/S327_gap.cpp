// S327's fast check: why the base's text comparison counted 503 where the base
// drops only 492 more rows than a3e84e1. Groups rows by the engine's key, as
// corpus_dedupe does, and splits each key's rows by their raw four-field text,
// as the base's --verify did. No build compiles it. From the repository root,
// after `cmake --build build`:
//
//   c++ -Itools -Isrc -O3 -DNDEBUG -std=gnu++20 -march=native
//       -o S327_gap adocs/data/S327_gap.cpp build/src/libchesso_engine.a
//
// as one line, then:
//
//   ./S327_gap .tuning/selfplay_v2.tsv
//
// Its output on that corpus is in adocs/data/S327_verify.txt.
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include "corpus_dedupe.hpp"

static bool text_fields(const std::string& fen, std::string* out)
{
  std::istringstream fields(fen);
  std::string field[4];

  for (size_t i = 0; i < 4; ++i) {
    if (!(fields >> field[i])) { return false; }
  }

  *out = field[0] + " " + field[1] + " " + field[2] + " " + field[3];
  return true;
}

static std::string first_three(const std::string& text)
{ return text.substr(0, text.rfind(' ')); }

int main(int argc, char** argv)
{
  if (argc != 2) { return 2; }

  std::ifstream in(argv[1]);
  game_t game;
  initialize_game_const_data(&game);

  std::unordered_map<uint64_t, std::string> witness;
  std::unordered_map<std::string, uint64_t> later;  // key|text -> rows

  std::string line, fen, text;
  uint64_t rows = 0, disagreeing = 0, ep_only = 0;

  while (std::getline(in, line)) {
    if (line.empty()) { continue; }

    ++rows;

    uint64_t key = 0;

    if (!corpus_dedupe::row_fen(line, &fen) ||
        !corpus_dedupe::position_key(fen, &game, &key) ||
        !text_fields(fen, &text)) {
      fprintf(stderr, "unreadable row %llu\n", (unsigned long long)rows);
      return 1;
    }

    auto found = witness.find(key);

    if (found == witness.end()) {
      witness.emplace(key, text);
      continue;
    }

    if (found->second == text) { continue; }

    ++disagreeing;

    if (first_three(found->second) == first_three(text)) { ++ep_only; }

    ++later[std::to_string(key) + "|" + text];
  }

  uint64_t already_duplicates = 0;

  for (const auto& entry : later) {
    already_duplicates += entry.second - 1;
  }

  printf("rows %llu\n", (unsigned long long)rows);
  printf("distinct keys %llu\n", (unsigned long long)witness.size());
  printf("rows whose text differs from their key's witness %llu\n",
         (unsigned long long)disagreeing);
  printf("  of them differing only in the en passant field %llu\n",
         (unsigned long long)ep_only);
  printf("distinct (key, text) groups after the witness's %llu\n",
         (unsigned long long)later.size());
  printf("rows already duplicates inside such a group %llu\n",
         (unsigned long long)already_duplicates);
  printf("distinct keys + those groups %llu\n",
         (unsigned long long)(witness.size() + later.size()));
  return 0;
}
