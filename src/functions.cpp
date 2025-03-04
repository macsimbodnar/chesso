#include "functions.hpp"
#include <cassert>
#include <map>
#include <string>
#include <vector>
#include "data_structures.hpp"
#include "exceptions.hpp"
#include "utils.hpp"


color_t us(const board_t* board)
{
  return board->game_state.active_color;
}


color_t opponent(const board_t* board)
{
  if (board->game_state.active_color == WHITE) { return BLACK; }

  return WHITE;
}


void init_board(const std::string& fen, board_t* board)
{
  assert(board != nullptr);

  // Cleanup
  board->board.fill(EMPTY);
  history_t empty_history = history_t();
  board->history.swap(empty_history);
  cleanup_game_state(&board->game_state);

  // Init random numbers
  init_zobrist(&board->zobrist_randoms);

  // Load FEN
  load_FEN(fen, board);
  board->initial_fen = fen;

  // Init Zobrist
  board->game_state.zobrist_key = init_zobrist_key(board);

  // TODO: Init phase_value
}


void reset(board_t* board)
{
  assert(board != nullptr);

  // Cleanup
  board->board.fill(EMPTY);
  history_t empty_history = history_t();
  board->history.swap(empty_history);
  cleanup_game_state(&board->game_state);


  // Load FEN
  load_FEN(board->initial_fen, board);

  // Init Zobrist
  board->game_state.zobrist_key = init_zobrist_key(board);
}


std::array<piece_t, BOARD_SIZE> get_chess_board(const board_t* board)
{
  return board->board;
}


position_t king_square(color_t color, const board_t* board)
{
  piece_t king_to_search = W_KING;
  if (color == BLACK) { king_to_search = B_KING; }

  uint8_t king_index = INVALID_BOARD_INDEX;
  for (uint8_t i = 0; i < BOARD_SIZE; ++i) {
    if (board->board[i] == king_to_search) {
      king_index = i;
      break;
    }
  }

  if (king_index == INVALID_BOARD_INDEX) {
    throw kin_not_on_board_exception(color_to_string(color) +
                                     " king is not ot the board.");
  }

  return index_to_position(king_index);
}


bool has_bishop_pair(color_t color, const board_t* board)
{
  piece_t bishop_to_search = B_BISHOP;
  if (color == WHITE) { bishop_to_search = W_BISHOP; }

  bool found[2] = {false, false};

  for (uint8_t index = 0; index < BOARD_SIZE; ++index) {
    if (board->board[index] == bishop_to_search) {
      color_t c = get_color_at_index(index);
      found[c] = true;
    }
  }


  return (found[0] && found[1]);
}
