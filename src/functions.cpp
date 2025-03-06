#include "functions.hpp"
#include <array>
#include <cassert>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "data_structures.hpp"
#include "exceptions.hpp"
#include "move_generator.hpp"
#include "utils.hpp"


color_t us(const board_t* board)
{
  return board->game_state.active_color;
}


color_t opponent(const board_t* board)
{
  return !board->game_state.active_color;
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
      color_t c = get_square_color(index);
      found[c] = true;
    }
  }


  return (found[0] && found[1]);
}


bool is_ambiguous_move(const move_t* move, const std::vector<move_t>* moves)
{
  for (const move_t& I : *moves) {
    if (move->piece == I.piece && move->to == I.to && move->from != I.from) {
      return true;
    }
  }

  return false;
}


std::string move_to_algebraic(const move_t* move,
                              const std::vector<move_t>* moves,
                              const board_t* board)
{
  assert(move != nullptr);
  assert(board != nullptr);
  assert(move->piece != INVALID);
  assert(move->piece != EMPTY);

  // Not using the piece_to_char function because the piece moved in always
  // upper case
  static const std::unordered_map<piece_t, char> piece_to_char_map = {
      {B_PAWN, 'P'},   {B_KNIGHT, 'N'}, {B_BISHOP, 'B'}, {B_ROOK, 'R'},
      {B_QUEEN, 'Q'},  {B_KING, 'K'},   {W_PAWN, 'P'},   {W_KNIGHT, 'N'},
      {W_BISHOP, 'B'}, {W_ROOK, 'R'},   {W_QUEEN, 'Q'},  {W_KING, 'K'},
      {INVALID, '*'},  {EMPTY, ' '}};

  static const std::array<char, 8> file_to_char_map = {'a', 'b', 'c', 'd',
                                                       'e', 'f', 'g', 'h'};

  std::string notation;

  // Handle castling
  if (move->castling_move) {
    if (move->to == 0x06 || move->to == 0x76)
      return "O-O";  // King-side castling
    if (move->to == 0x02 || move->to == 0x72)
      return "O-O-O";  // Queen-side castling
  }

  if (move->piece != W_PAWN && move->piece != B_PAWN) {
    notation += piece_to_char_map.at(move->piece);  // Non-pawn pieces

    // If ambiguous move the add the from file
    if (is_ambiguous_move(move, moves)) {
      notation += file_to_char_map[index_to_position(move->from).file];
    }
  }

  // Capture notation
  if (move->captured != INVALID && move->piece != W_PAWN &&
      move->piece != B_PAWN) {
    notation += 'x';
  }

  // Destination square
  notation += index_to_algebraic(move->to);

  // Pawn captures (ex: exd5)
  if ((move->piece == W_PAWN || move->piece == B_PAWN) &&
      move->captured != INVALID) {
    notation = index_to_algebraic(move->from)[0] + std::string("x") +
               index_to_algebraic(move->to);
  }

  // Pawn promotion
  if (move->promoted_to != TO_NONE) {
    notation += "=";
    switch (move->promoted_to) {
      case TO_QUEEN:
        notation += 'Q';
        break;
      case TO_ROOK:
        notation += 'R';
        break;
      case TO_BISHOP:
        notation += 'B';
        break;
      case TO_KNIGHT:
        notation += 'N';
        break;
      default:
        break;
    }
  }

  // Handle check
  const index_t opponent_king_index = get_king_index(opponent(board), board);

  board_t tmp_board = *board;
  bool move_happened = make_move(move, &tmp_board);
  assert(move_happened == true);

  // Generate moves for my color but after the current move is done
  const auto pseudo_legal_moves =
      generate_pseudo_legal_moves_from_index(move->to, &tmp_board);

  // Check if one of this moves put under check the opponent ing

  for (const auto& pseudo_move : pseudo_legal_moves) {
    if (pseudo_move.to == opponent_king_index) {
      // Append a '+' to the notation
      notation += '+';
      break;
    }
  }

  return notation;
}


bool make_move(const move_t* move, board_t* board)
{
  assert(move != nullptr);
  assert(board != nullptr);
  assert(move->captured != EMPTY);

  // Remove en-passant
  clear_ep_square(board);

  // Check move type
  if (move->captured != INVALID) {
    // In case of attack remove the piece from the board
    const piece_t removed = remove_piece(move->to, board);
    assert(removed == move->captured);
  }

  // Move the moving piece
  const piece_t moved_piece = move_piece(move->from, move->to, board);
  assert(moved_piece == move->piece);

  // TODO: handle promotion
  // TODO: handle en-passant set in case of double_pawn_move
  // TODO: handle en-passant capture
  // TODO: handle castling move
  // TODO: handle updating castling rights in case of rook or king move

  // Swap side
  swap_side(board);

  return true;
}
