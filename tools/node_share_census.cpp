// What share of the root's own nodes the best move takes, per position. S132.
//
//   build/tools/node_share_census --depth 12 < fens.txt
//   build/tools/node_share_census --depth 12 --hash 16 --fens positions.txt
//
// One row per FEN on stdout, tab separated:
//
//   fen  depth  best  total_nodes  root_nodes  best_nodes  share_pct
//
// `total_nodes` is what the engine reports to a GUI for the whole `go`, read
// off its own `info` line; `root_nodes` is the sum of the per-root-move
// buckets, which is that number less one per search() call; `best_nodes` is
// `share_pct * root_nodes / 100`, the winner's bucket re-derived from the
// published percentage and not read independently, so the column carries no
// information of its own. `share_pct` is the engine's own integer percentage
// -- the number the time manager reads -- and not a float recomputed here: a
// census that computed its own would be calibrating one implementation
// against another.
//
// It links the engine and drives the real UCI layer rather than talking to a
// pipe, for the reason tools/mate_trace.cpp does: the buckets live inside
// search_state_t, that struct is built and destroyed inside one `go`, and no
// process outside this one can see them. The engine's own output is captured
// rather than printed, so the rows below are the whole of stdout and the
// best move and node total are the ones a GUI would have been given.
//
// The engine is left as a game leaves it between moves: one table, a
// `ucinewgame` before each position so nothing an earlier one stored decides
// anything, and the same iterative deepening a clock would run -- at a fixed
// depth, so a row is a property of the tree and not of the machine.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "data_structures.hpp"
#include "uci.hpp"

namespace
{

// Runs one command with the engine's UCI channel captured, and hands back
// what it said.
std::string quietly(const std::string& command, bool wait)
{
  std::ostringstream sink;
  std::streambuf* const saved = std::cout.rdbuf(sink.rdbuf());

  uci_process_line(command);

  if (wait) { uci_wait_for_search(); }

  std::cout.rdbuf(saved);

  return sink.str();
}


// The last `nodes <n>` field of the captured `info` lines, which is the whole
// search's count (S037) and is what a GUI divides by the time.
uint64_t nodes_reported(const std::string& output)
{
  uint64_t nodes = 0;
  std::istringstream lines(output);
  std::string line;

  while (std::getline(lines, line)) {
    std::istringstream words(line);
    std::string word;

    while (words >> word) {
      if (word == "nodes" && (words >> word)) {
        nodes = strtoull(word.c_str(), nullptr, 10);
      }
    }
  }

  return nodes;
}


std::string best_move_reported(const std::string& output)
{
  std::istringstream lines(output);
  std::string line;

  while (std::getline(lines, line)) {
    if (line.rfind("bestmove ", 0) != 0) { continue; }

    std::istringstream words(line);
    std::string word;
    std::string move;

    words >> word >> move;

    return move;
  }

  return "none";
}

}  // namespace


int main(int argc, char** argv)
{
  int depth = 12;
  int hash_mb = 16;
  const char* fens_path = nullptr;

  for (int i = 1; i < argc; ++i) {
    const bool has_value = (i + 1 < argc);

    if (strcmp(argv[i], "--depth") == 0 && has_value) {
      depth = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--hash") == 0 && has_value) {
      hash_mb = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--fens") == 0 && has_value) {
      fens_path = argv[++i];
    } else {
      fprintf(stderr,
              "usage: %s [--depth N] [--hash MB] [--fens FILE]\n"
              "FENs are read from FILE, or from stdin, one per line.\n",
              argv[0]);
      return 2;
    }
  }

  if (depth < 1) {
    fprintf(stderr, "--depth must be at least 1\n");
    return 2;
  }

  std::vector<std::string> fens;
  {
    std::ifstream file;
    std::istream* in = &std::cin;

    if (fens_path != nullptr) {
      file.open(fens_path);

      if (!file) {
        fprintf(stderr, "cannot read %s\n", fens_path);
        return 2;
      }

      in = &file;
    }

    std::string line;

    while (std::getline(*in, line)) {
      if (!line.empty() && line.back() == '\r') { line.pop_back(); }
      if (line.empty() || line[0] == '#') { continue; }

      fens.push_back(line);
    }
  }

  if (fens.empty()) {
    fprintf(stderr, "no FEN to read\n");
    return 2;
  }

  {
    // uci_init() allocates the table and arms the book, and says so on the
    // channel; the rows below are the whole of stdout, so it is captured like
    // everything else.
    std::ostringstream sink;
    std::streambuf* const saved = std::cout.rdbuf(sink.rdbuf());

    uci_init();

    std::cout.rdbuf(saved);
  }

  quietly("setoption name Hash value " + std::to_string(hash_mb), false);

  printf("fen\tdepth\tbest\ttotal_nodes\troot_nodes\tbest_nodes\tshare_pct\n");

  int refused = 0;

  for (const std::string& fen : fens) {
    quietly("ucinewgame", false);

    const std::string loading = quietly("position fen " + fen, false);

    if (loading.find("info string refused") != std::string::npos) {
      fprintf(stderr, "refused: %s\n", fen.c_str());
      refused++;
      continue;
    }

    const std::string played =
        quietly("go depth " + std::to_string(depth), true);

    const uint64_t root_nodes = uci_last_root_nodes_total();
    const int share = uci_last_bestmove_node_percent();

    // The winner's own bucket, recovered from the two numbers the engine
    // publishes: the share is an integer percentage, so this is its numerator
    // only to the rounding the time manager itself does.
    const uint64_t best_nodes =
        (static_cast<uint64_t>(share) * root_nodes) / 100;

    printf("%s\t%d\t%s\t%llu\t%llu\t%llu\t%d\n", fen.c_str(), depth,
           best_move_reported(played).c_str(),
           static_cast<unsigned long long>(nodes_reported(played)),
           static_cast<unsigned long long>(root_nodes),
           static_cast<unsigned long long>(best_nodes), share);
    fflush(stdout);
  }

  uci_shutdown();

  if (refused > 0) { fprintf(stderr, "%d position(s) refused\n", refused); }

  return (refused > 0) ? 1 : 0;
}
