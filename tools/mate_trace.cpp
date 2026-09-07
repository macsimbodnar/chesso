// What the transposition table holds along a reported line, after a game has
// been replayed into it. S171.
//
//   build/tools/mate_trace --fen '<fen>' --moves '<game>' --start 40
//       --warm 'nodes 1500000' --final 'depth 11' --line '<reported pv>'
//
// The defect this exists for cannot be read off a cold search: a mate score
// the engine reports in a game comes back out of entries earlier searches in
// the same game wrote, and the only thing that says which entry answered is
// the table itself, in the state that search left it. So the game is replayed
// here move by move through the real UCI layer -- one process, one table --
// and when the final search is done the line it reported is walked over a
// board of its own, with the entry for every position on it printed: the score
// as stored, the score as a reader at that distance from the root sees it, the
// depth, the node type and the generation that wrote it.
//
// Reading the table and not the search: nothing here searches a node or stores
// an entry, so the picture printed is the one the reported line was produced
// from.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "transposition_table.hpp"
#include "uci.hpp"
#include "utils.hpp"

namespace
{

// search.cpp's, where they are file-local macros. Copied rather than shared
// because this is a diagnostic that must keep reading a table the engine is
// free to change: if the band moves and this file is not updated, the mate
// column here goes wrong and no engine behaviour does.
constexpr int MATE_MAX = 49000;
constexpr int MATE_MIN = 48000;


std::vector<std::string> split_words(const std::string& text)
{
  std::istringstream stream(text);
  std::vector<std::string> out;
  std::string word;

  while (stream >> word) {
    out.push_back(word);
  }

  return out;
}


// de_normalize_score() from search.cpp: what a reader `ply` plies from the
// root sees in an entry whose score counts from the position it was stored at.
int de_normalize(int score, int ply)
{
  if (score > MATE_MIN && score <= MATE_MAX) { return score - ply; }
  if (score < -MATE_MIN && score >= -MATE_MAX) { return score + ply; }
  return score;
}


// The mate distance in moves a score carries, or 0 when it carries none.
int mate_in_moves(int score)
{
  if (score > MATE_MIN) { return (MATE_MAX - score + 1) / 2; }
  if (score < -MATE_MIN) { return -((MATE_MAX + score + 1) / 2); }
  return 0;
}


const char* type_name(uint8_t type)
{
  switch (type) {
    case TT_EMPTY_NODE:
      return "empty";
    case TT_PV_NODE:
      return "exact";
    case TT_ALPHA_NODE:
      return "upper";
    case TT_BETA_NODE:
      return "lower";
    default:
      return "?";
  }
}


// The move `text` names in this position, or 0 when it names none. Looked up
// in the engine's own generator so a token naming no legal move is caught here
// rather than played.
move_t legal_move_named(const game_t* game, const std::string& text)
{
  const std::optional<uci_move_t> parsed = algebraic_to_uci_move(text);

  if (!parsed.has_value()) { return 0; }

  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (MOVE_FROM(moves[i]) == parsed->from &&
        MOVE_TO(moves[i]) == parsed->to &&
        MOVE_PROMOTED(moves[i]) == parsed->promotion) {
      return moves[i];
    }
  }

  return 0;
}


// The stored move, re-validated against this position's generator. A stored
// move is a move_t from another position on a key that matched, and S147's
// extend_mate_pv() re-validates for the same reason: a 64-bit key still
// collides.
move_t legal_move_matching(const game_t* game, move_t stored)
{
  if (stored == 0) { return 0; }

  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (moves[i] == stored) { return moves[i]; }
  }

  return 0;
}


// Every legal move here and what the table holds for the position it reaches.
// A stalled walk asks this: the hole is one ply deep exactly when a child of
// the stalled node carries the distance the parent's missing entry would have
// named, and that is the difference between a hole a lookahead can fill and
// one that needs a search.
void print_children(game_t* game, size_t ply)
{
  const transposition_table_t* tt = uci_tt();

  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  printf("  %zu legal moves here; what the table holds one ply on:\n", count);

  for (size_t i = 0; i < count; ++i) {
    if (!make_move(game, moves[i])) { continue; }

    const tt_entry_t* entry = tt_get_entry(tt, &game->board);

    if (entry == nullptr) {
      printf("    %-16s miss\n", print_move(moves[i]).c_str());
    } else {
      const int at_root = de_normalize(entry->score, static_cast<int>(ply) + 1);

      printf("    %-16s %10d  mate %3d  depth %3d  %-5s gen %u\n",
             print_move(moves[i]).c_str(), at_root, mate_in_moves(at_root),
             entry->depth, type_name(entry->type), entry->generation);
    }

    unmake_move(game);
  }
}


void run_search(const std::string& position, const std::string& go)
{
  uci_process_line(position);
  uci_process_line("go " + go);
  uci_wait_for_search();
}


// One row per position on `line`, root included, and then -- when `needed` is
// longer than the line -- one row per position the table's own best move leads
// to, which is the walk complete_mate_pv() makes. `game` is left where the walk
// ended.
void walk(game_t* game, const std::vector<std::string>& line, size_t needed)
{
  const transposition_table_t* tt = uci_tt();

  printf("\n%4s %-6s %-8s %10s %10s %6s %6s %5s %5s %s\n", "ply", "move",
         "entry", "stored", "at root", "mate", "depth", "type", "gen", "best");

  const size_t last = std::max(line.size(), needed);

  for (size_t i = 0; i <= last; ++i) {
    const tt_entry_t* entry = tt_get_entry(tt, &game->board);
    std::string move = "-";

    if (i > 0) { move = (i <= line.size()) ? line[i - 1] : "tt"; }

    if (entry == nullptr) {
      printf("%4zu %-6s %-8s\n", i, move.c_str(), "miss");
    } else {
      const int at_root = de_normalize(entry->score, static_cast<int>(i));

      printf(
          "%4zu %-6s %-8s %10d %10d %6d %6d %5s %5u %s\n", i, move.c_str(),
          "hit", entry->score, at_root, mate_in_moves(at_root), entry->depth,
          type_name(entry->type), entry->generation,
          entry->best_move == 0 ? "-" : print_move(entry->best_move).c_str());
    }

    if (i == last) { break; }

    // Past the reported line the next move is the one the table names, which
    // is where complete_mate_pv() gets its continuation. A slot that misses or
    // carries no move is where that walk stalls, and the row above says so.
    move_t played = 0;

    if (i < line.size()) {
      played = legal_move_named(game, line[i]);
    } else if (entry != nullptr && entry->best_move != 0) {
      played = legal_move_matching(game, entry->best_move);
    }

    if (played == 0 || !make_move(game, played)) {
      printf("  walk stops at ply %zu: no move to play\n", i);
      print_children(game, i);
      break;
    }

    if (i >= line.size()) {
      printf("       followed %s\n", print_move(played).c_str());
    }
  }
}


[[noreturn]] void usage()
{
  fprintf(stderr,
          "usage: mate_trace --fen FEN [--moves 'e2e4 ...'] [--start N]\n"
          "                  [--warm 'nodes 1500000'] [--final 'depth 11']\n"
          "                  [--line 'h1g1 ...'] [--needed N] [--hash MB]\n");
  exit(2);
}

}  // namespace


int main(int argc, char** argv)
{
  std::string fen;
  std::string moves_text;
  std::string line_text;
  std::string warm = "nodes 1500000";
  size_t needed = 0;
  std::string final_go;
  size_t start = 0;
  int hash_mb = 16;

  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    const bool has_value = (i + 1 < argc);

    if (strcmp(arg, "--fen") == 0 && has_value) {
      fen = argv[++i];
    } else if (strcmp(arg, "--moves") == 0 && has_value) {
      moves_text = argv[++i];
    } else if (strcmp(arg, "--line") == 0 && has_value) {
      line_text = argv[++i];
    } else if (strcmp(arg, "--warm") == 0 && has_value) {
      warm = argv[++i];
    } else if (strcmp(arg, "--final") == 0 && has_value) {
      final_go = argv[++i];
    } else if (strcmp(arg, "--needed") == 0 && has_value) {
      needed = strtoul(argv[++i], nullptr, 10);
    } else if (strcmp(arg, "--start") == 0 && has_value) {
      start = strtoul(argv[++i], nullptr, 10);
    } else if (strcmp(arg, "--hash") == 0 && has_value) {
      hash_mb = atoi(argv[++i]);
    } else {
      usage();
    }
  }

  if (fen.empty()) { usage(); }
  if (final_go.empty()) { final_go = warm; }

  const std::vector<std::string> moves = split_words(moves_text);
  const std::vector<std::string> line = split_words(line_text);

  uci_init();
  uci_process_line("setoption name Hash value " + std::to_string(hash_mb));
  uci_process_line("ucinewgame");

  // Every ply from `start` on is searched, because what the table holds at the
  // last one is made by the searches before it. Dropping the earlier plies is
  // dropping the warming, not the moves: the position is always the full move
  // list.
  for (size_t i = start; i < moves.size(); ++i) {
    std::string position = "position fen " + fen;

    if (i > 0) {
      position += " moves";
      for (size_t m = 0; m < i; ++m) {
        position += " " + moves[m];
      }
    }

    run_search(position, warm);
  }

  std::string position = "position fen " + fen;
  if (!moves.empty()) {
    position += " moves";
    for (const std::string& move : moves) {
      position += " " + move;
    }
  }

  printf("=== final search: go %s\n", final_go.c_str());
  run_search(position, final_go);

  if (!line.empty() || needed > 0) {
    game_t board;
    initialize_game_const_data(&board);

    if (!load_FEN(fen, &board)) {
      fprintf(stderr, "cannot load the FEN\n");
      uci_shutdown();
      return 1;
    }

    for (const std::string& move : moves) {
      const move_t played = legal_move_named(&board, move);

      if (played == 0 || !make_move(&board, played)) {
        fprintf(stderr, "game move %s is not legal\n", move.c_str());
        uci_shutdown();
        return 1;
      }
    }

    walk(&board, line, needed);
  }

  uci_shutdown();
  return 0;
}
