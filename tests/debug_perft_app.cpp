/**
 * @file chesso_perft_generator.cpp
 * @author Max (macsimbodnar@gmail.com)
 * @brief
 * This program is created to be used with the perftree tool.
 * https://github.com/agausmann/perftree
 * @version 0.1
 * @date 2025-03-12
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <cassert>
#include <iostream>
#include <queue>
#include <string>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "utils.hpp"


std::vector<std::string> split(std::string s, std::string delimiter)
{
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}


struct mini_move_t
{
  index_t from;
  index_t to;
  promotion_t promotion;
};


mini_move_t algebraic_to_mini_move(const std::string& p)
{
  assert(p.length() >= 4);

  uint8_t from_file = p[0];
  uint8_t from_rank = p[1];
  uint8_t to_file = p[2];
  uint8_t to_rank = p[3];

  assert(from_file >= 'a');
  assert(from_file <= 'h');
  assert(from_rank >= '1');
  assert(from_rank <= '8');

  assert(to_file >= 'a');
  assert(to_file <= 'h');
  assert(to_rank >= '1');
  assert(to_rank <= '8');

  from_file = from_file - 'a';
  from_rank = from_rank - '1';

  to_file = to_file - 'a';
  to_rank = to_rank - '1';

  const index_t from = position_to_index(from_file, from_rank);
  const index_t to = position_to_index(to_file, to_rank);

  mini_move_t move;
  move.from = from;
  move.to = to;
  move.promotion = TO_NONE;

  if (p.length() == 5) {
    // Handle promotion
    switch (p[4]) {
      case 'Q':
      case 'q':
        move.promotion = TO_QUEEN;
        break;

      case 'N':
      case 'n':
        move.promotion = TO_KNIGHT;
        break;

      case 'R':
      case 'r':
        move.promotion = TO_ROOK;
        break;

      case 'B':
      case 'b':
        move.promotion = TO_BISHOP;
        break;

      default:
        assert(false);
        break;
    }
  }

  return move;
}


std::string promotion_to_string(move_t move)
{
  switch (MOVE_PROMOTED(move)) {
    case TO_QUEEN:
      return "q";
      break;
    case TO_KNIGHT:
      return "n";
      break;
    case TO_ROOK:
      return "r";
      break;
    case TO_BISHOP:
      return "b";
      break;

    default:
      return "";
      break;
  }

  assert(false);
  return "ERROR";
}


size_t debug_generate_legal_moves(game_t* game, move_t moves[])
{
  size_t count = 0;
  move_t all_moves[MAX_MOVES];
  const size_t all_moves_count =
      generate_moves(game_tables(), &game->board, all_moves);

  assert(all_moves_count < MAX_MOVES);

  for (size_t i = 0; i < all_moves_count; ++i) {
    if (is_move_legal(game, all_moves[i])) { moves[count++] = all_moves[i]; }
  }

  return count;
}


uint64_t perft(int depth, game_t* game)
{
  assert(game != nullptr);

  uint64_t nodes = 0;

  if (depth == 0) { return 1; }

  move_t moves[270];
  const size_t moves_count = generate_moves(game_tables(), &game->board, moves);
  assert(moves_count <= 270);
  // node_stats += get_moves_stats(moves);

  for (size_t i = 0; i < moves_count; ++i) {
    if (make_move(game, moves[i])) {
      nodes += perft(depth - 1, game);
      unmake_move(game);
    }
  }

  return nodes;
}


static game_t g_game;

// "$depth" "$fen" "$moves"
int main(int argc, char* argv[])
{
  initialize_game_const_data(&g_game);

  if (argc < 3) {
    std::cout << "No arguments. Use \"$depth\" \"$fen\" \"$moves\"\n";
    return 1;
  }

  const int depth = std::atoi(argv[1]);
  const std::string fen = std::string(argv[2]);

  std::queue<mini_move_t> moves_to_apply;

  if (argc > 3) {
    std::string moves_str = std::string(argv[3]);
    const auto tokens = split(moves_str, " ");

    for (const auto& token : tokens) {
      const mini_move_t move = algebraic_to_mini_move(token);
      moves_to_apply.push(move);
    }
  }

  load_FEN(fen, &g_game);

  move_t moves[270];
  size_t moves_count = debug_generate_legal_moves(&g_game, moves);

  // Navigate the moves
  while (!moves_to_apply.empty()) {
    const mini_move_t mini_move = moves_to_apply.front();
    moves_to_apply.pop();

    bool found = false;
    for (size_t i = 0; i < moves_count; ++i) {
      const move_t move = moves[i];
      const unpacked_move_t um(move);

      if (um.from == mini_move.from && um.to == mini_move.to &&
          mini_move.promotion == um.promoted_to) {
        found = true;

        bool move_happened = make_move(&g_game, move);
        (void)move_happened;
        assert(move_happened);
        moves_count = debug_generate_legal_moves(&g_game, moves);
        break;
      }
    }

    (void)found;
    assert(found);
  }

  uint64_t tot_nodes = 1;

  if (depth > 0) {
    if (depth > 1) {
      tot_nodes = 0;
      for (size_t i = 0; i < moves_count; ++i) {
        const move_t move = moves[i];

        bool move_happened = make_move(&g_game, move);
        (void)move_happened;
        assert(move_happened);

        uint64_t num_of_nodes = perft(depth - 1, &g_game);
        unmake_move(&g_game);
        tot_nodes += num_of_nodes;

        std::cout << index_to_str(MOVE_FROM(move))
                  << index_to_str(MOVE_TO(move)) << promotion_to_string(move)
                  << " " << num_of_nodes << std::endl;
      }
    } else {
      tot_nodes = moves_count;
      for (size_t i = 0; i < moves_count; ++i) {
        const move_t& move = moves[i];
        std::cout << index_to_str(MOVE_FROM(move))
                  << index_to_str(MOVE_TO(move)) << promotion_to_string(move)
                  << " " << 1 << std::endl;
      }
    }
  }


  std::cout << "\n" << tot_nodes << std::endl;

  return 0;
}
