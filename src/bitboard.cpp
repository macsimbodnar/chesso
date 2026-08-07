#include "bitboard.hpp"
#include <bit>
#include <limits>
#include <random>
#include <unordered_map>
#include "bb_tables.hpp"
#include "data_structures.hpp"
#include "log.hpp"
#include "utils.hpp"


/******************************************************************************
 *                               MUST RUN FAST
 * NOTE: count_bits, get_lsb_index and the get_*_attacks lookups are defined
 * inline in bitboard.hpp so that every translation unit can inline them.
 ******************************************************************************/
bool is_attacked(const bb_tables_t* tables,
                 const board_t* board,
                 index_t index,
                 color_t color)
{
  assert(board != nullptr);
  assert(index < 64);

  // The six bitboards of a side are contiguous (W_PAWN..W_KING then
  // B_PAWN..B_KING), so one base pointer replaces every per-piece ternary.
  const bb_t* pieces = &board->bitboards[(color == WHITE) ? W_PAWN : B_PAWN];

  // Leapers first: plain table lookups, no magic multiply, and they reject
  // the common case cheaply.
  if (tables->pawn_attacks[!color][index] & pieces[0]) { return true; }
  if (tables->knight_attacks[index] & pieces[1]) { return true; }
  if (tables->king_attacks[index] & pieces[5]) { return true; }

  // Sliders: the queen shares the bishop and rook rays, so testing it against
  // the same two attack sets costs nothing extra. Computing queen attacks
  // separately would repeat both magic lookups for no gain.
  const bb_t occupancy = board->occupancies[BOTH];
  const bb_t queens = pieces[4];

  if (get_bishop_attacks(tables, index, occupancy) & (pieces[2] | queens)) {
    return true;
  }

  if (get_rook_attacks(tables, index, occupancy) & (pieces[3] | queens)) {
    return true;
  }

  return false;
}


size_t generate_moves(const bb_tables_t* tables,
                      const board_t* board,
                      move_t moves[])
{
  assert(tables != nullptr);
  assert(board != nullptr);
  assert(moves != nullptr);

  const color_t color = board->active_color;
  const color_t opponent = !color;

  const bb_t all_occupancy = board->occupancies[BOTH];
  const bb_t opp_occupancy = board->occupancies[opponent];
  const bb_t free_squares = ~board->occupancies[color];

  // The six bitboards of a side are contiguous (W_PAWN..W_KING, then
  // B_PAWN..B_KING), so the side to move indexes them off a single base and we
  // never walk the opponent's six.
  const piece_t first_piece = (color == WHITE) ? W_PAWN : B_PAWN;
  const bb_t* my_bitboards = &board->bitboards[first_piece];

  // Turning the en-passant square into a mask once lifts both the validity
  // test and the shift out of the per-pawn loop.
  const bb_t en_passant_mask =
      (board->en_passant != INVALID_INDEX) ? (BB_1 << board->en_passant) : BB_0;

  size_t move_count = 0;

  //-############################  PAWNS  ##################################-//
  {
    const piece_t piece = first_piece;  // W_PAWN or B_PAWN

    // A pawn can never legally stand on the first or last rank. A malformed
    // FEN can still put one there, and the push below would then compute an
    // off-board target square, so mask those pawns out once instead of
    // bounds-checking every push.
    bb_t bboard = my_bitboards[0] & 0x00FFFFFFFFFFFF00ULL;

    // Ranks the pawn pushes toward, expressed as source squares
    const index_t promotion_rank_start = (color == WHITE) ? a7 : a2;
    const index_t double_push_rank_start = (color == WHITE) ? a2 : a7;

    while (bboard) {
      const index_t from = get_lsb_index(bboard);
      bboard &= bboard - 1;  // Clear the lowest set bit

      const index_t to =
          static_cast<index_t>((color == WHITE) ? (from - 8) : (from + 8));

      const bool is_promoting =
          (from >= promotion_rank_start && from <= promotion_rank_start + 7);

      // Quiet pushes
      if (!GET_BIT(all_occupancy, to)) {
        if (is_promoting) {
          moves[move_count++] = NEW_MOVE(from, to, piece, TO_QUEEN, 0, 0, 0, 0);
          moves[move_count++] = NEW_MOVE(from, to, piece, TO_ROOK, 0, 0, 0, 0);
          moves[move_count++] =
              NEW_MOVE(from, to, piece, TO_BISHOP, 0, 0, 0, 0);
          moves[move_count++] =
              NEW_MOVE(from, to, piece, TO_KNIGHT, 0, 0, 0, 0);
        } else {
          moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);

          if (from >= double_push_rank_start &&
              from <= double_push_rank_start + 7) {
            const index_t double_to =
                static_cast<index_t>((color == WHITE) ? (to - 8) : (to + 8));

            if (!GET_BIT(all_occupancy, double_to)) {
              moves[move_count++] =
                  NEW_MOVE(from, double_to, piece, 0, 0, 1, 0, 0);
            }
          }
        }
      }

      // Captures
      const bb_t pawn_attacks = tables->pawn_attacks[color][from];
      bb_t attacks = pawn_attacks & opp_occupancy;

      while (attacks) {
        const index_t target = get_lsb_index(attacks);
        attacks &= attacks - 1;

        if (is_promoting) {
          moves[move_count++] =
              NEW_MOVE(from, target, piece, TO_QUEEN, 1, 0, 0, 0);
          moves[move_count++] =
              NEW_MOVE(from, target, piece, TO_ROOK, 1, 0, 0, 0);
          moves[move_count++] =
              NEW_MOVE(from, target, piece, TO_BISHOP, 1, 0, 0, 0);
          moves[move_count++] =
              NEW_MOVE(from, target, piece, TO_KNIGHT, 1, 0, 0, 0);
        } else {
          moves[move_count++] = NEW_MOVE(from, target, piece, 0, 1, 0, 0, 0);
        }
      }

      // En-passant
      if (pawn_attacks & en_passant_mask) {
        moves[move_count++] =
            NEW_MOVE(from, board->en_passant, piece, 0, 1, 0, 1, 0);
      }
    }
  }

  //-#####################  KNIGHTS TO QUEENS  #############################-//
  // Knights, bishops, rooks, queens and the king all share the same shape:
  // take the attack set, drop our own pieces, then tag each target as a
  // capture or a quiet move. The capture flag is derived with a shift rather
  // than a branch, which the target loop would mispredict constantly.
  const auto emit_moves = [&](int piece_offset, auto&& attacks_of) {
    const piece_t piece = static_cast<piece_t>(first_piece + piece_offset);
    bb_t bboard = my_bitboards[piece_offset];

    while (bboard) {
      const index_t from = get_lsb_index(bboard);
      bboard &= bboard - 1;  // Clear the lowest set bit

      bb_t attacks = attacks_of(from) & free_squares;

      while (attacks) {
        const index_t to = get_lsb_index(attacks);
        attacks &= attacks - 1;

        const move_t capture =
            static_cast<move_t>((opp_occupancy >> to) & BB_1);

        moves[move_count++] = NEW_MOVE(from, to, piece, 0, capture, 0, 0, 0);
      }
    }
  };

  emit_moves(1, [&](index_t sq) { return tables->knight_attacks[sq]; });
  emit_moves(2, [&](index_t sq) {
    return get_bishop_attacks(tables, sq, all_occupancy);
  });
  emit_moves(3, [&](index_t sq) {
    return get_rook_attacks(tables, sq, all_occupancy);
  });
  emit_moves(4, [&](index_t sq) {
    return get_queen_attacks(tables, sq, all_occupancy);
  });

  //-##########################  CASTLING  #################################-//
  // Emitted before the king's normal moves to keep the ordering the previous
  // implementation produced.
  {
    const castling_t king_side = (color == WHITE) ? WK : BK;
    const castling_t queen_side = (color == WHITE) ? WQ : BQ;

    if (board->castling & (king_side | queen_side)) {
      const index_t e_square = (color == WHITE) ? e1 : e8;
      const index_t f_square = (color == WHITE) ? f1 : f8;
      const index_t g_square = (color == WHITE) ? g1 : g8;
      const index_t d_square = (color == WHITE) ? d1 : d8;
      const index_t c_square = (color == WHITE) ? c1 : c8;
      const index_t b_square = (color == WHITE) ? b1 : b8;

      // Both sides need the king's square to be safe; test it once.
      const bool king_square_attacked =
          is_attacked(tables, board, e_square, opponent);

      if (!king_square_attacked) {
        const piece_t king = static_cast<piece_t>(first_piece + 5);

        if ((board->castling & king_side) &&
            !GET_BIT(all_occupancy, f_square) &&
            !GET_BIT(all_occupancy, g_square) &&
            !is_attacked(tables, board, f_square, opponent) &&
            !is_attacked(tables, board, g_square, opponent)) {
          moves[move_count++] =
              NEW_MOVE(e_square, g_square, king, 0, 0, 0, 0, 1);
        }

        if ((board->castling & queen_side) &&
            !GET_BIT(all_occupancy, d_square) &&
            !GET_BIT(all_occupancy, c_square) &&
            !GET_BIT(all_occupancy, b_square) &&
            !is_attacked(tables, board, d_square, opponent) &&
            !is_attacked(tables, board, c_square, opponent)) {
          moves[move_count++] =
              NEW_MOVE(e_square, c_square, king, 0, 0, 0, 0, 1);
        }
      }
    }
  }

  emit_moves(5, [&](index_t sq) { return tables->king_attacks[sq]; });

  assert(move_count < MAX_MOVES);
  return move_count;
}


bool move_belongs_to_side_to_move(const board_t* board, move_t move)
{
  assert(board != nullptr);

  const unsigned piece = MOVE_PIECE(move);

  if (piece > B_KING) { return false; }

  const bool is_white_piece = (piece < B_PAWN);

  if (is_white_piece != (board->active_color == WHITE)) { return false; }

  return GET_BIT(board->bitboards[piece], MOVE_FROM(move)) != BB_0;
}


bool make_move(game_t* game, move_t encoded_move)
{
  assert(game != nullptr);
  assert(move_belongs_to_side_to_move(&game->board, encoded_move));

  const bb_tables_t* tables = &game->tables;
  const zobrist_randoms_t* randoms = &game->hash_randoms;
  board_t* board = &game->board;
  history_t* history = &game->history;

  const hash_t old_hash = board->hash;

  // Both stacks are fixed size. Refusing the move leaves the board untouched
  // and consistent; writing past the end would corrupt whatever follows. The
  // search can never reach this - it is bounded by MAX_PLY - so this only
  // guards an absurdly long [position ... moves ...] line.
  if (history->size + 1 >= HISTORY_MAX_SIZE ||
      game->repetitions.size + 1 >= REPETITION_MAX_SIZE) {
    LOG_E << "Move stack is full, refusing the move" << END_E;
    return false;
  }

  // Store the history
  history_entry_t* history_entry = &history->entries[history->size++];
  memcpy(&history_entry->board, board, sizeof(board_t));
  history_entry->repetition_size = game->repetitions.size;

  unpacked_move_t move(encoded_move);

  const color_t us = board->active_color;
  const color_t them = !us;
  const bb_t from_bb = BB_1 << move.from;
  const bb_t to_bb = BB_1 << move.to;

  // Occupancies are updated incrementally below. Rebuilding them from the
  // twelve piece bitboards at the end of the move costs far more than the
  // handful of xors each case needs.
  board->bitboards[move.piece] ^= from_bb | to_bb;
  board->occupancies[us] ^= from_bb | to_bb;

  board->hash ^= randoms->piece_randoms[move.piece][move.from];
  board->hash ^= randoms->piece_randoms[move.piece][move.to];

  // An en-passant move carries the capture flag but the captured pawn does not
  // sit on the target square, so the occupancy test below skips it and the
  // en-passant block further down removes it.
  if (move.capture && (board->occupancies[them] & to_bb)) {
    // Loop over the bitboards opposite to the current side to move
    const piece_t start_piece = (us == WHITE) ? B_PAWN : W_PAWN;
    const piece_t end_piece = (us == WHITE) ? B_KING : W_KING;

    for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
      if (board->bitboards[bb_piece] & to_bb) {
        board->bitboards[bb_piece] ^= to_bb;
        board->hash ^= randoms->piece_randoms[bb_piece][move.to];
        break;
      }
    }

    board->occupancies[them] ^= to_bb;
  }

  static const piece_t w_promotion_map[] = {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK,
                                            W_QUEEN};
  static const piece_t b_promotion_map[] = {B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK,
                                            B_QUEEN};

  if (move.promoted_to) {
    // The pawn leaves and the promoted piece arrives on the same square, so
    // the occupancies do not change here.
    const piece_t pawn = (us == WHITE) ? W_PAWN : B_PAWN;
    const piece_t promoted_to = (us == WHITE)
                                    ? w_promotion_map[move.promoted_to]
                                    : b_promotion_map[move.promoted_to];

    board->bitboards[pawn] ^= to_bb;
    board->hash ^= randoms->piece_randoms[pawn][move.to];

    board->bitboards[promoted_to] ^= to_bb;
    board->hash ^= randoms->piece_randoms[promoted_to][move.to];
  }

  if (move.en_passant) {
    const piece_t captured_pawn = (us == WHITE) ? B_PAWN : W_PAWN;
    const index_t captured_square =
        (us == WHITE) ? (move.to + 8) : (move.to - 8);
    const bb_t captured_bb = BB_1 << captured_square;

    board->bitboards[captured_pawn] ^= captured_bb;
    board->occupancies[them] ^= captured_bb;
    board->hash ^= randoms->piece_randoms[captured_pawn][captured_square];
  }

  // Update the en-passant square.
  //
  // ep_randoms has an entry for INVALID_INDEX and it must be folded in and out
  // like any other square. Skipping it when there is no en-passant square (as
  // this used to) leaves the incremental hash offset by a constant from
  // compute_full_hash() and from set_en_passant(), both of which apply it
  // unconditionally, so the two would disagree about the key for the same
  // position.
  board->hash ^= randoms->ep_randoms[board->en_passant];
  const index_t push =
      static_cast<index_t>((us == WHITE) ? (move.to + 8) : (move.to - 8));
  board->en_passant = move.double_push ? push : static_cast<index_t>(INVALID_INDEX);

  board->hash ^= randoms->ep_randoms[board->en_passant];

  if (move.castling) {
    piece_t rook = EMPTY;
    index_t rook_from = INVALID_INDEX;
    index_t rook_to = INVALID_INDEX;

    switch (move.to) {
      case (g1):  // White castles king side
        rook = W_ROOK;
        rook_from = h1;
        rook_to = f1;
        break;

      case (c1):  // White castles queen side
        rook = W_ROOK;
        rook_from = a1;
        rook_to = d1;
        break;

      case (g8):  // Black castles king side
        rook = B_ROOK;
        rook_from = h8;
        rook_to = f8;
        break;

      case (c8):  // Black castles queen side
        rook = B_ROOK;
        rook_from = a8;
        rook_to = d8;
        break;

      default:
        // A castling flag on any other target square is malformed input; the
        // king move has already been applied, leave the rooks alone.
        break;
    }

    if (rook != EMPTY) {
      const bb_t rook_bb = (BB_1 << rook_from) | (BB_1 << rook_to);
      board->bitboards[rook] ^= rook_bb;
      board->occupancies[us] ^= rook_bb;
      board->hash ^= randoms->piece_randoms[rook][rook_from];
      board->hash ^= randoms->piece_randoms[rook][rook_to];
    }
  }

  // Handle half move clock
  if (move.capture || move.piece == W_PAWN || move.piece == B_PAWN) {
    board->halfmove_clock = 0;
  } else {
    board->halfmove_clock += 1;
  }

  // Update castling rights
  board->hash ^= randoms->castling_randoms[board->castling];
  board->castling &= castling_rights[move.from];
  board->castling &= castling_rights[move.to];
  board->hash ^= randoms->castling_randoms[board->castling];

  // The per-side occupancies were maintained incrementally above
  board->occupancies[BOTH] =
      board->occupancies[WHITE] | board->occupancies[BLACK];

  assert(board->occupancies[WHITE] ==
         (board->bitboards[W_PAWN] | board->bitboards[W_KNIGHT] |
          board->bitboards[W_BISHOP] | board->bitboards[W_ROOK] |
          board->bitboards[W_QUEEN] | board->bitboards[W_KING]));
  assert(board->occupancies[BLACK] ==
         (board->bitboards[B_PAWN] | board->bitboards[B_KNIGHT] |
          board->bitboards[B_BISHOP] | board->bitboards[B_ROOK] |
          board->bitboards[B_QUEEN] | board->bitboards[B_KING]));

  // change side
  board->hash ^= randoms->side_randoms[us];
  board->active_color = them;
  board->hash ^= randoms->side_randoms[them];

  // After black turn update the full move counter as well
  if (us == BLACK) { board->fullmove_counter++; }

  // Store repetitions. Capacity was checked before the history push above.
  game->repetitions.entries[game->repetitions.size++] = old_hash;

  // Check legality: the side that just moved must not have left its king en
  // prise.
  const bb_t our_king = board->bitboards[(us == WHITE) ? W_KING : B_KING];

  if (our_king && is_attacked(tables, board, get_lsb_index(our_king), them)) {
    // Restore board
    history->size--;
    memcpy(board, &history->entries[history->size].board, sizeof(board_t));
    game->repetitions.size = history->entries[history->size].repetition_size;

    return false;
  }

  return true;
}


void unmake_move(game_t* game)
{
  assert(game != nullptr);

  // Guard the *empty* end of the stack: `size` is unsigned, so decrementing it
  // at zero wraps around and reads far out of bounds.
  if (game->history.size > 0) {
    game->history.size--;
    const history_entry_t* entry = &game->history.entries[game->history.size];
    memcpy(&game->board, &entry->board, sizeof(board_t));
    game->repetitions.size = entry->repetition_size;
  }
}


bool is_position_repeated(const repetition_t* rep, const board_t* board)
{
  assert(rep != nullptr);
  assert(board != nullptr);

  // A position can only recur while no irreversible move (capture, pawn move)
  // has been played, so the halfmove clock bounds how far back it is worth
  // looking. Anything older cannot match except through a hash collision.
  //
  // Stepping by two skips the plies where the other side is to move; those can
  // never equal the current hash because the side to move is part of it.
  const size_t limit =
      (board->halfmove_clock < rep->size) ? board->halfmove_clock : rep->size;

  for (size_t back = 2; back <= limit; back += 2) {
    if (rep->entries[rep->size - back] == board->hash) { return true; }
  }

  return false;
}


bool is_check(const game_t* game)
{
  assert(game != nullptr);

  const bb_t king = (game->board.active_color == WHITE)
                        ? game->board.bitboards[W_KING]
                        : game->board.bitboards[B_KING];

  // A side with no king cannot be in check. This is reachable: EMPTY_POS has
  // no kings at all, and an illegal FEN where the side to move is already
  // giving check lets the search capture the enemy king. get_lsb_index()
  // returns 64 for an empty board, which is out of bounds for every attack
  // table is_attacked() touches. make_move() already guards its own king test
  // the same way.
  if (king == BB_0) { return false; }

  const index_t index = get_lsb_index(king);

  return is_attacked(&game->tables, &game->board, index,
                     !game->board.active_color);
}


void swap_side(game_t* game)
{
  assert(game != nullptr);

  game->board.hash ^= game->hash_randoms.side_randoms[game->board.active_color];
  game->board.active_color = !game->board.active_color;
  game->board.hash ^= game->hash_randoms.side_randoms[game->board.active_color];
}


void set_en_passant(game_t* game, index_t en_passant_index)
{
  assert(game != nullptr);

  game->board.hash ^= game->hash_randoms.ep_randoms[game->board.en_passant];
  game->board.en_passant = en_passant_index;
  game->board.hash ^= game->hash_randoms.ep_randoms[game->board.en_passant];
}


bool is_capturing_king(const board_t* board, move_t move)
{
  assert(board != nullptr);
  assert(move != 0);
  const bb_t kings_board = board->bitboards[W_KING] | board->bitboards[B_KING];
  const index_t to = MOVE_TO(move);
  const bb_t attack_mask = BB_1 << to;
  const bool capture = MOVE_CAPTURE(move);
  const bb_t attack = kings_board & attack_mask;
  const bool result = capture && attack;
  return result;
}


/******************************************************************************
 *                               UTIL FUNCTIONS
 * NOTE: Does not need to be optimized
 ******************************************************************************/
void cleanup_board(game_t* game)
{
  assert(game != nullptr);

  memset(&game->board, 0, sizeof(board_t));

  game->board.active_color = WHITE;
  game->board.castling = WQ | WK | BQ | BK;
  game->board.halfmove_clock = 0;
  game->board.en_passant = INVALID_INDEX;
  game->board.fullmove_counter = 1;
  game->board.hash = 0ULL;

  game->history.size = 0;
  game->repetitions.size = 0;
}


hash_t compute_full_hash(game_t* game)
{
  assert(game != nullptr);

  hash_t key = 0;

  // Xor pieces on the board
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const index_t index = position_to_index(file, rank);
      const piece_t piece = get_piece(&game->board, index);

      if (piece != EMPTY) {
        key ^= game->hash_randoms.piece_randoms[piece][index];
      }
    }
  }

  // Xor side to move
  key ^= game->hash_randoms.side_randoms[game->board.active_color];

  // Xor castling
  key ^= game->hash_randoms.castling_randoms[game->board.castling];

  // Xor en-passant
  key ^= game->hash_randoms.ep_randoms[game->board.en_passant];

  return key;
}


bool load_FEN(const std::string& FEN, game_t* game)
{
  assert(game != nullptr);
  board_t* board = &game->board;

  cleanup_board(game);

  // Start parsing
  auto sections = split_string(FEN);

  if (sections.size() != 6) {
    LOG_E << "Bad FEN string: " << FEN << END_E;
    return false;
  }

  /*****************************************************************************
   * 0. Piece placement tables
   *
   * pawn = "P"
   * knight = "N"
   * bishop = "B"
   * rook = "R"
   * queen = "Q"
   * and king = "K
   *
   * White ("PNBRQK")
   * Black ("pnbrqk")
   ****************************************************************************/

  uint8_t file = 0;
  uint8_t rank = 7;

  for (const char c : sections[0]) {
    switch (c) {
      case '/':
        // Both counters are unsigned and index straight into a 64 bit board,
        // so a rank that does not add up to 8, or a ninth rank, has to be
        // rejected here rather than shifting past the end of the bitboard.
        if (file != 8 || rank == 0) {
          LOG_E << "Malformed rank in FEN string. FEN: " << FEN << END_E;
          return false;
        }

        file = 0;
        --rank;
        break;

      case '1':
        file += 1;
        break;
      case '2':
        file += 2;
        break;
      case '3':
        file += 3;
        break;
      case '4':
        file += 4;
        break;
      case '5':
        file += 5;
        break;
      case '6':
        file += 6;
        break;
      case '7':
        file += 7;
        break;
      case '8':
        file += 8;
        break;

      case 'P':
      case 'N':
      case 'B':
      case 'R':
      case 'Q':
      case 'K':
      case 'p':
      case 'n':
      case 'b':
      case 'r':
      case 'q':
      case 'k': {
        if (file >= 8) {
          LOG_E << "Too many pieces on a rank in FEN string. FEN: " << FEN
                << END_E;
          return false;
        }

        const index_t index = position_to_index(file, rank);
        const piece_t piece = char_to_piece(c);
        SET_BIT(board->bitboards[piece], index);
        ++file;
      } break;

      default:
        // We get a non valid string
        LOG_E << "Invalid char in FEN string [" << STR(c) << "]. FEN: " << FEN
              << END_E;

        return false;
        break;
    }

    if (file > 8) {
      LOG_E << "Rank overflows past the h file in FEN string. FEN: " << FEN
            << END_E;
      return false;
    }
  }

  if (rank != 0 || file != 8) {
    LOG_E << "FEN string does not describe all 8 ranks. FEN: " << FEN << END_E;
    return false;
  }

  /***************************************************************************
   * 1. Active color
   **************************************************************************/
  if (sections[1].size() != 1) {
    LOG_E << "invalid active color section. FEN: " << FEN << END_E;
    return false;
  }

  char color = sections[1][0];
  switch (color) {
    case 'w':
      board->active_color = WHITE;
      break;
    case 'b':
      board->active_color = BLACK;
      break;
    default:
      LOG_E << "Invalid color char in FEN string [" << std::string(1, color)
            << "]. FEN: " << FEN << END_E;
      return false;
      break;
  }

  /***************************************************************************
   * 2. Castling availability
   *
   * "-" No castling available
   * "K" if White can castle kingside
   * "Q" if White can castle queenside
   * "k" if Black can castle kingside
   * "q" if Black can castle queenside
   **************************************************************************/
  if (sections[2].size() < 1 || sections[2].size() > 4) {
    LOG_E << "Invalid castling availability section size. FEN: " << FEN
          << END_E;
    return false;
  }

  board->castling = 0x00;
  for (const char c : sections[2]) {
    switch (c) {
      case '-':
        board->castling = 0x00;
        if (sections[2].size() != 1) {
          LOG_E
              << "Invalid castling availability section size. No castling "
                 "available char is set but the section size is too big. FEN: "
              << FEN << END_E;
          return false;
        }
        break;
      case 'K':
        board->castling |= WK;
        break;
      case 'Q':
        board->castling |= WQ;
        break;
      case 'k':
        board->castling |= BK;
        break;
      case 'q':
        board->castling |= BQ;
        break;

      default:
        LOG_E << "Invalid castling availability character ["
              << std::string(1, c) << "]. FEN: " + FEN << END_E;

        return false;
    }
  }

  /***************************************************************************
   * 3. En passant target square
   *
   * "-" None
   **************************************************************************/
  if (sections[3].size() < 1 || sections[3].size() > 2) {
    LOG_E << "Invalid en passant section size. FEN: " << FEN << END_E;
    return false;
  }

  if (sections[3].size() == 1 && sections[3][0] != '-') {
    LOG_E << "Invalid en passant section char [ "
          << std::string(1, sections[3][0]) << "]. FEN: " << FEN << END_E;
    return false;
  }

  if (sections[3].size() == 2) {
    // isalpha/isdigit are far too permissive here: str_to_index() subtracts
    // 'a' and '1' and feeds the result to position_to_index(), so anything
    // outside the board wraps the unsigned index and reads out of bounds.
    if (sections[3][0] < 'a' || sections[3][0] > 'h' || sections[3][1] < '1' ||
        sections[3][1] > '8') {
      LOG_E << "Invalid en passant section. Wrong algebraic notation ["
            << sections[3] << "]. FEN: " << FEN << END_E;
      return false;
    }
  }

  if (sections[3] == "-") {
    board->en_passant = INVALID_INDEX;
  } else {
    board->en_passant = str_to_index(sections[3]);
  }

  /***************************************************************************
   * 4. Halfmove clock
   *
   * The number of halfmoves since the last capture or pawn advance, used for
   * the fifty-move rule.
   **************************************************************************/
  const std::string& half_move = sections[4];
  if (half_move.size() < 1) {
    LOG_E << "Invalid Halfmove clock section size. FEN: " << FEN << END_E;
    return false;
  }

  if (!is_uint(half_move)) {
    LOG_E << "Invalid Halfmove clock section is not a number. FEN: " << FEN
          << END_E;
    return false;
  }

  try {
    const unsigned long clock = std::stoul(half_move);

    // The field is a uint8_t. Truncating a larger value would silently reset
    // the fifty move counter and shrink the repetition search window.
    if (clock > std::numeric_limits<uint8_t>::max()) {
      LOG_E << "Halfmove clock out of range [" << half_move
            << "]. FEN: " << FEN << END_E;
      return false;
    }

    board->halfmove_clock = static_cast<uint8_t>(clock);
  } catch (std::exception& e) {
    LOG_E << "Can't convert Halfmove clock to integer.What: "
          << std::string(e.what()) << " FEN: " << FEN << END_E;
    return false;
  }

  /***************************************************************************
   * 5. Fullmove number
   *
   * The number of the full moves. It starts at 1 and is incremented after
   * Black's move.
   **************************************************************************/
  const std::string& full_move = sections[5];
  if (full_move.size() < 1) {
    LOG_E << "Invalid Fullmove number section size. FEN: " << FEN << END_E;
    return false;
  }

  if (!is_uint(full_move)) {
    LOG_E << "Invalid Fullmove number section is not a number. FEN: " << FEN
          << END_E;
    return false;
  }

  try {
    board->fullmove_counter = static_cast<int>(std::stoul(full_move));
  } catch (std::exception& e) {
    LOG_E << "Can't convert Fullmove number to integer. What: "
          << std::string(e.what()) << " FEN: " << FEN << END_E;
    return false;
  }

  if (board->fullmove_counter < 1) {
    LOG_E << "Fullmove number can't be less then 1 but it is "
          << std::string(STR(board->fullmove_counter)) << " FEN: " << FEN
          << END_E;
    return false;
  }

  // Populate occupancies
  for (int piece = W_PAWN; piece <= W_KING; ++piece) {
    board->occupancies[WHITE] |= board->bitboards[piece];
    board->occupancies[BLACK] |= board->bitboards[piece + B_PAWN];
  }

  board->occupancies[BOTH] |= board->occupancies[WHITE];
  board->occupancies[BOTH] |= board->occupancies[BLACK];

  board->hash = compute_full_hash(game);
  return true;
}


std::string generate_FEN(const board_t* board)
{
  assert(board != nullptr);

  std::stringstream ss;

  // Step 1: Board representation
  int empty_count = 0;

  for (int rank = 7; rank >= 0; --rank) {
    for (int file = 0; file < 8; ++file) {
      const auto index = position_to_index(file, rank);
      const auto piece = get_piece(board, index);

      if (piece == piece_t::EMPTY) {
        ++empty_count;
      } else {
        if (empty_count > 0) {
          ss << empty_count;
          empty_count = 0;
        }

        switch (piece) {
          case piece_t::B_KING:
            ss << 'k';
            break;
          case piece_t::B_QUEEN:
            ss << 'q';
            break;
          case piece_t::B_ROOK:
            ss << 'r';
            break;
          case piece_t::B_BISHOP:
            ss << 'b';
            break;
          case piece_t::B_KNIGHT:
            ss << 'n';
            break;
          case piece_t::B_PAWN:
            ss << 'p';
            break;
          case piece_t::W_KING:
            ss << 'K';
            break;
          case piece_t::W_QUEEN:
            ss << 'Q';
            break;
          case piece_t::W_ROOK:
            ss << 'R';
            break;
          case piece_t::W_BISHOP:
            ss << 'B';
            break;
          case piece_t::W_KNIGHT:
            ss << 'N';
            break;
          case piece_t::W_PAWN:
            ss << 'P';
            break;

          default:
            break;
        }
      }
    }

    if (empty_count > 0) {
      ss << empty_count;
      empty_count = 0;
    }

    if (rank > 0) { ss << '/'; }
  }

  // Step 2: Active color
  ss << (board->active_color == color_t::WHITE ? " w " : " b ");

  // Step 3: Castling rights
  bool has_castling_rights = false;

  if (board->castling & WK) {
    ss << 'K';
    has_castling_rights = true;
  }

  if (board->castling & WQ) {
    ss << 'Q';
    has_castling_rights = true;
  }

  if (board->castling & BK) {
    ss << 'k';
    has_castling_rights = true;
  }

  if (board->castling & BQ) {
    ss << 'q';
    has_castling_rights = true;
  }

  if (!has_castling_rights) { ss << '-'; }

  ss << ' ';

  // Step 4: En passant target square
  if (board->en_passant != INVALID_INDEX) {
    ss << index_to_str(board->en_passant);
  } else {
    ss << '-';
  }

  ss << ' ';

  // Step 5: Halfmove clock
  ss << int(board->halfmove_clock) << ' ';

  // Step 6: Fullmove number
  ss << int(board->fullmove_counter);

  return ss.str();
}


/**
 * @brief Returns the indexes of the ambiguous moves.
 */
size_t get_ambiguous_move(const unpacked_move_t* move,
                          const move_t moves[],
                          size_t moves_size,
                          index_t result[])
{
  assert(moves != nullptr);
  assert(result != nullptr);

  size_t result_count = 0;
  for (size_t i = 0; i < moves_size; ++i) {
    const unpacked_move_t I(moves[i]);

    // If same piece, same destination and different source
    if (move->piece == I.piece && move->to == I.to && move->from != I.from) {
      result[result_count] = i;
      ++result_count;
    }
  }

  return result_count;
}


bool is_move_legal(game_t* game, move_t move)
{
  if (!move_belongs_to_side_to_move(&game->board, move)) { return false; }

  if (make_move(game, move)) {
    unmake_move(game);
    return true;
  }

  return false;
}


size_t count_legal_moves(game_t* game, move_t moves[], size_t count)
{
  size_t result = 0;

  for (size_t i = 0; i < count; ++i) {
    if (is_move_legal(game, moves[i])) { result++; }
  }

  return result;
}


std::string move_to_algebraic(game_t* game,
                              move_t encoded_move,
                              const move_t moves[],
                              size_t moves_size)
{
  assert(game != nullptr);
  assert(moves != nullptr);
  assert(moves_size > 0);

  board_t* board = &game->board;

  // Not using the piece_to_char function because the piece moved in
  // always upper case
  static const std::unordered_map<piece_t, char> piece_to_char_map = {
      {B_PAWN, 'P'},   {B_KNIGHT, 'N'}, {B_BISHOP, 'B'}, {B_ROOK, 'R'},
      {B_QUEEN, 'Q'},  {B_KING, 'K'},   {W_PAWN, 'P'},   {W_KNIGHT, 'N'},
      {W_BISHOP, 'B'}, {W_ROOK, 'R'},   {W_QUEEN, 'Q'},  {W_KING, 'K'},
      {EMPTY, ' '}};

  static const char file_to_char_map[8] = {'a', 'b', 'c', 'd',
                                           'e', 'f', 'g', 'h'};
  static const char rank_to_char_map[8] = {'1', '2', '3', '4',
                                           '5', '6', '7', '8'};

  std::string notation;
  const unpacked_move_t move(encoded_move);

  // Handle castling
  if (move.castling) {
    if (move.to == g1 || move.to == g8) {
      return "O-O";  // King-side castling
    }

    if (move.to == c1 || move.to == c8) {
      return "O-O-O";  // Queen-side castling
    }
  }

  if (move.piece != W_PAWN && move.piece != B_PAWN) {
    notation += piece_to_char_map.at(move.piece);  // Non-pawn pieces

    // If ambiguous move the add the from file
    index_t ambiguous_moves[MAX_MOVES];
    const size_t ambiguous_moves_count =
        get_ambiguous_move(&move, moves, moves_size, ambiguous_moves);

    if (ambiguous_moves_count > 0) {
      const position_t move_from_pos = index_to_position(move.from);
      bool is_file_unique = true;
      bool is_rank_unique = true;

      // Check if file or rank are unique for the move.from
      for (size_t i = 0; i < ambiguous_moves_count; ++i) {
        const index_t index = ambiguous_moves[i];
        const position_t i_pos = index_to_position(MOVE_FROM(moves[index]));

        if (move_from_pos.file == i_pos.file) { is_file_unique = false; }

        if (move_from_pos.rank == i_pos.rank) { is_rank_unique = false; }

        // Exit from the loop in case both are non unique. No make sense
        // to search for more
        if (!is_file_unique && !is_rank_unique) { break; }
      }

      if (is_file_unique) {
        // Check if file unique
        notation += file_to_char_map[move_from_pos.file];
      } else if (is_rank_unique) {
        // Check if rank unique
        notation += rank_to_char_map[move_from_pos.rank];
      } else {
        // In case none is unique use both
        notation += file_to_char_map[move_from_pos.file];
        notation += rank_to_char_map[move_from_pos.rank];
      }
    }
  }

  // Capture notation
  if (move.capture && move.piece != W_PAWN && move.piece != B_PAWN) {
    notation += 'x';
  }

  // Destination square
  notation += index_to_str(move.to);

  // Pawn captures (ex: exd5)
  if ((move.piece == W_PAWN || move.piece == B_PAWN) && move.capture) {
    notation =
        index_to_str(move.from)[0] + std::string("x") + index_to_str(move.to);
  }

  // Pawn promotion
  if (move.promoted_to > 0) {
    notation += "=";
    switch (move.promoted_to) {
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

  // Handle check and mate
  const bool move_happened = make_move(game, encoded_move);
  assert(move_happened);

  if (move_happened) {
    move_t loc_moves[MAX_MOVES];
    const size_t loc_moves_count =
        generate_moves(&game->tables, &game->board, loc_moves);

    const size_t legal_moves_count =
        count_legal_moves(game, loc_moves, loc_moves_count);

    const piece_t king_to_select =
        (board->active_color == WHITE) ? W_KING : B_KING;

    const color_t opponent = (board->active_color == WHITE) ? BLACK : WHITE;

    const index_t king_index = get_lsb_index(board->bitboards[king_to_select]);

    const bool is_check =
        is_attacked(&game->tables, &game->board, king_index, opponent);

    if (is_check && legal_moves_count == 0) {
      // Check mate
      notation += '#';
    } else if (is_check) {
      // Append a '+' to the notation
      notation += '+';
    }

    unmake_move(game);
  }

  return notation;
}


move_t algebraic_to_move(std::string notation, game_t* game)
{
  assert(game != nullptr);
  board_t* board = &game->board;

  static const std::unordered_map<char, uint8_t> char_to_file_map = {
      {'a', 0}, {'b', 1}, {'c', 2}, {'d', 3},
      {'e', 4}, {'f', 5}, {'g', 6}, {'h', 7}};
  static const std::unordered_map<char, uint8_t> char_to_rank_map = {
      {'1', 0}, {'2', 1}, {'3', 2}, {'4', 3},
      {'5', 4}, {'6', 5}, {'7', 6}, {'8', 7}};

  const std::string original_notation = notation;

  unpacked_move_t result(BB_0);

  const color_t color = board->active_color;

  // Make a working copy of the move string.
  bool is_capture = false;

  // TODO: Use this
  // bool is_check = false;
  // bool is_mate = false;
  // if (notation.back() == '+') { is_check = true; }
  // if (notation.back() == '#') { is_mate = true; }

  // Remove any trailing check ('+') or checkmate ('#') symbols.
  while (!notation.empty() &&
         (notation.back() == '+' || notation.back() == '#')) {
    notation.pop_back();
  }

  // Parse castling
  if (notation == "O-O-O") {
    result.castling = true;
    switch (color) {
      case BLACK:
        result.from = e8;
        result.to = c8;
        result.piece = B_KING;
        break;
      case WHITE:
        result.from = e1;
        result.to = c1;
        result.piece = W_KING;
        break;
      default:
        assert(false);
        break;
    }

    return NEW_MOVE(result.from, result.to, result.piece, result.promoted_to,
                    result.capture, result.double_push, result.en_passant,
                    result.castling);
  }

  if (notation == "O-O") {
    result.castling = true;

    switch (color) {
      case BLACK:
        result.from = e8;
        result.to = g8;
        result.piece = B_KING;
        break;
      case WHITE:
        result.from = e1;
        result.to = g1;
        result.piece = W_KING;
        break;
      default:
        assert(false);
        break;
    }

    return NEW_MOVE(result.from, result.to, result.piece, result.promoted_to,
                    result.capture, result.double_push, result.en_passant,
                    result.castling);
  }


  // Parse non-castling moves
  size_t pos = 0;
  piece_t moving_piece;

  // If the move begins with a piece letter (K, Q, R, B, N), then use it.
  if (pos < notation.size() && std::isupper(notation[pos])) {
    char piece_char = notation[pos];
    switch (piece_char) {
      case 'K':
        moving_piece = (color == WHITE) ? W_KING : B_KING;
        break;
      case 'Q':
        moving_piece = (color == WHITE) ? W_QUEEN : B_QUEEN;
        break;
      case 'R':
        moving_piece = (color == WHITE) ? W_ROOK : B_ROOK;
        break;
      case 'B':
        moving_piece = (color == WHITE) ? W_BISHOP : B_BISHOP;
        break;
      case 'N':
        moving_piece = (color == WHITE) ? W_KNIGHT : B_KNIGHT;
        break;
      default:
        moving_piece = EMPTY;
        break;
    }
    ++pos;
  } else {
    // If no piece letter then it's a pawn move.
    moving_piece = (color == WHITE) ? W_PAWN : B_PAWN;
  }
  result.piece = moving_piece;

  // We now extract any disambiguation info.
  // This may be a file letter, a rank digit, or both.
  std::optional<char> disambiguous_file;
  std::optional<char> disambiguous_rank;

  // Look ahead for an 'x' (capture marker) or destination square.
  // We will also later remove any 'x' from the string.
  size_t temp_pos = pos;
  while (temp_pos < notation.size() && notation[temp_pos] != 'x' &&
         !(notation[temp_pos] >= 'a' && notation[temp_pos] <= 'h' &&
           (temp_pos + 1 < notation.size() && notation[temp_pos + 1] >= '1' &&
            notation[temp_pos + 1] <= '8'))) {
    // Assume any character here is part of disambiguation.
    char d = notation[temp_pos];
    if (d >= 'a' && d <= 'h')
      disambiguous_file = d;
    else if (d >= '1' && d <= '8')
      disambiguous_rank = d;
    ++temp_pos;
  }

  // Remove capture marker(s) from the string.
  std::string cleaned;
  for (char ch : notation.substr(pos)) {
    if (ch != 'x') {
      cleaned.push_back(ch);
    } else {
      is_capture = true;
    }
  }

  // Look for promotion: if there is an '=' then the following char is the
  // promotion piece.
  promotion_t promo = TO_NONE;
  size_t promo_pos = cleaned.find('=');
  if (promo_pos != std::string::npos && promo_pos + 1 < cleaned.size()) {
    char promo_char = cleaned[promo_pos + 1];
    switch (promo_char) {
      case 'Q':
        promo = TO_QUEEN;
        break;
      case 'R':
        promo = TO_ROOK;
        break;
      case 'B':
        promo = TO_BISHOP;
        break;
      case 'N':
        promo = TO_KNIGHT;
        break;
      default:
        promo = TO_NONE;
        break;
    }
    cleaned = cleaned.substr(0, promo_pos);
  }
  result.promoted_to = promo;

  // The destination square is the last two characters of the cleaned string.
  if (cleaned.size() < 2) {
    // Error: not enough characters to form a square.

    LOG_E << "Wrong formatting. Invalid Algebraic notation: "
          << original_notation << END_E;
    assert(false);
  }

  std::string dest_square = cleaned.substr(cleaned.size() - 2, 2);
  index_t to_index = str_to_index(dest_square);
  result.to = to_index;

  if (result.to >= INVALID_INDEX) {
    LOG_E << "Invalid destination square. Invalid Algebraic notation: "
          << original_notation << END_E;

    assert(false);
  }

  if (is_capture) {
    // Attempt to use the destination as capture piece
    result.capture = true;
  }

  // Any remaining characters between our initial pos and the destination
  // have been interpreted as disambiguation.
  // (In many SAN moves the disambiguation is omitted if unneeded.)
  // Here we already extracted potential disambiguation earlier.

  // Generate legal moves and search the compatible one
  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_moves(&game->tables, &game->board, moves);

  bool found = false;
  for (size_t i = 0; i < moves_count; ++i) {
    if (!is_move_legal(game, moves[i])) { continue; }

    const unpacked_move_t legal_move(moves[i]);

    if (legal_move.to == result.to && legal_move.piece == result.piece &&
        legal_move.promoted_to == result.promoted_to &&
        legal_move.capture == result.capture &&
        legal_move.castling == result.castling) {
      // Check for disambiguous
      const position_t legal_from_pos = index_to_position(legal_move.from);

      if (disambiguous_file.has_value()) {
        const uint8_t file = char_to_file_map.at(disambiguous_file.value());

        if (file != legal_from_pos.file) { continue; }
      }

      if (disambiguous_rank.has_value()) {
        const uint8_t rank = char_to_rank_map.at(disambiguous_rank.value());

        if (rank != legal_from_pos.rank) { continue; }
      }

      // We found the move
      found = true;
      result = legal_move;
      break;
    }
  }

  if (!found) {
    LOG_E << "No legal move found. Invalid Algebraic notation: "
          << original_notation << END_E;

    assert(false);
  }

  return NEW_MOVE(result.from, result.to, result.piece, result.promoted_to,
                  result.capture, result.double_push, result.en_passant,
                  result.castling);
}


/**
 * This function fixes the weirdo castling move that can be found in Polyglot
 * book format and some times the UCI can send that as well! (Looking at you
 * Cutechess!)
 *
 * We just need the move
 * white short      e1h1 -> e1g1
 * white long       e1a1 -> e1c1
 * black short      e8h8 -> e8g8
 * black long       e8a8 -> e8c8
 */
move_t fix_weirdo_castling(const board_t* board, move_t encoded_move)
{
  assert(board != nullptr);
  unpacked_move_t move(encoded_move);
  const piece_t p = get_piece(board, move.from);

  if (move.from == e1 && move.to == h1 && p == W_KING) {
    // white short
    move.to = g1;
    move.castling = true;
    move.piece = W_KING;

    return move.pack();
  } else if (move.from == e1 && move.to == a1 && p == W_KING) {
    // white long
    move.to = c1;
    move.castling = true;
    move.piece = W_KING;

    return move.pack();
  } else if (move.from == e8 && move.to == h8 && p == B_KING) {
    // black short
    move.to = g8;
    move.castling = true;
    move.piece = B_KING;

    return move.pack();
  } else if (move.from == e8 && move.to == a8 && p == B_KING) {
    // black short
    move.to = c8;
    move.castling = true;
    move.piece = B_KING;

    return move.pack();
  }

  return encoded_move;
}


/**
 * This function fixes the weirdo castling move that can be found in Polyglot
 * book format and some times the UCI can send that as well! (Looking at you
 * Cutechess!)
 *
 * We just need the move
 * white short      e1h1 -> e1g1
 * white long       e1a1 -> e1c1
 * black short      e8h8 -> e8g8
 * black long       e8a8 -> e8c8
 */
void fix_weirdo_castling(const board_t* board, unpacked_move_t* move)
{
  assert(board != nullptr);
  assert(move != nullptr);
  const piece_t p = get_piece(board, move->from);

  if (move->from == e1 && move->to == h1 && p == W_KING) {
    // white short
    move->to = g1;
    move->castling = true;
    move->piece = W_KING;
  } else if (move->from == e1 && move->to == a1 && p == W_KING) {
    // white long
    move->to = c1;
    move->castling = true;
    move->piece = W_KING;
  } else if (move->from == e8 && move->to == h8 && p == B_KING) {
    // black short
    move->to = g8;
    move->castling = true;
    move->piece = B_KING;
  } else if (move->from == e8 && move->to == a8 && p == B_KING) {
    // black short
    move->to = c8;
    move->castling = true;
    move->piece = B_KING;
  }
}


bool is_pv_legal(game_t* game, const pv_t* pv)
{
  assert(game != nullptr);
  assert(pv != nullptr);

  bool is_pv_ok = true;
  size_t make_move_counter = 0;

  // Empty pv is illegal
  if (pv->length < 1) {
    LOG_W << "Empty PV" << END_W;
    return false;
  }

  for (size_t i = 0; i < pv->length; ++i) {
    const move_t move_to_test = pv->table[i];

    move_t moves[MAX_MOVES];
    const size_t moves_count =
        generate_moves(&game->tables, &game->board, moves);

    if (moves_count < 1) {
      is_pv_ok = false;
      break;
    }

    bool found = false;
    for (size_t move_index = 0; move_index < moves_count; ++move_index) {
      if (move_to_test == moves[move_index]) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG_W << "PV with illegal move: " << print_move(move_to_test) << END_W;
      is_pv_ok = false;
      break;
    }

    const bool legal = make_move(game, move_to_test);
    assert(legal);
    (void)legal;
    make_move_counter++;
  }

  for (size_t i = 0; i < make_move_counter; ++i) {
    unmake_move(game);
  }

  return is_pv_ok;
}


/******************************************************************************
 *                      INITIALIZATION FUNCTIONS
 * NOTE: Does not need to be optimized, they are called once at the start
 ******************************************************************************/
void init_zobrist(zobrist_randoms_t* zobrist)
{
  assert(zobrist != nullptr);

  // Fixed seed.
  std::mt19937_64 gen(0x9E3779B97F4A7C15ULL);
  std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

  for (auto& piece_array : zobrist->piece_randoms) {
    for (uint64_t& random : piece_array) {
      random = dist(gen);
    }
  }

  for (uint64_t& random : zobrist->castling_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->side_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->ep_randoms) {
    random = dist(gen);
  }

  zobrist->initialized = true;
}


bb_t set_occupancy(uint64_t index, int mask_bit_count, bb_t attack_mask)
{
  bb_t occupancy = BB_0;

  for (int count = 0; count < mask_bit_count; ++count) {
    const uint64_t square = get_lsb_index(attack_mask);
    assert(square < 64);

    POP_BIT(attack_mask, square);

    // Check if on board
    if (index & (BB_1 << count)) { occupancy |= BB_1 << square; }
  }

  return occupancy;
}


bb_t precompute_pawn_attacks(color_t color, index_t square)
{
  assert(square < 64);

  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  switch (color) {
    case WHITE:
      if ((board >> 7) & (~file_masks[0])) { attacks |= (board >> 7); }
      if ((board >> 9) & (~file_masks[7])) { attacks |= (board >> 9); }
      break;

    case BLACK:
      if ((board << 7) & (~file_masks[7])) { attacks |= (board << 7); }
      if ((board << 9) & (~file_masks[0])) { attacks |= (board << 9); }
      break;

    default:
      assert(false);
  }

  return attacks;
}


bb_t precompute_knight_attacks(index_t square)
{
  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  if ((board >> 17) & (~file_masks[7])) { attacks |= (board >> 17); }
  if ((board >> 15) & (~file_masks[0])) { attacks |= (board >> 15); }
  if ((board >> 10) & (~(file_masks[6] | file_masks[7]))) {
    attacks |= (board >> 10);
  }
  if ((board >> 6) & (~(file_masks[0] | file_masks[1]))) {
    attacks |= (board >> 6);
  }

  if ((board << 17) & (~file_masks[0])) { attacks |= (board << 17); }
  if ((board << 15) & (~file_masks[7])) { attacks |= (board << 15); }
  if ((board << 10) & (~(file_masks[0] | file_masks[1]))) {
    attacks |= (board << 10);
  }
  if ((board << 6) & (~(file_masks[6] | file_masks[7]))) {
    attacks |= (board << 6);
  }

  return attacks;
}


bb_t precompute_king_attacks(index_t square)
{
  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  if (board >> 8) { attacks |= (board >> 8); }
  if ((board >> 9) & (~file_masks[7])) { attacks |= (board >> 9); }
  if ((board >> 7) & (~file_masks[0])) { attacks |= (board >> 7); }
  if ((board >> 1) & (~file_masks[7])) { attacks |= (board >> 1); }

  if (board << 8) { attacks |= (board << 8); }
  if ((board << 9) & (~file_masks[0])) { attacks |= (board << 9); }
  if ((board << 7) & (~file_masks[7])) { attacks |= (board << 7); }
  if ((board << 1) & (~file_masks[0])) { attacks |= (board << 1); }

  return attacks;
}


bb_t precompute_bishop_attack_masks(index_t square)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r < 7 && f < 7; ++r, ++f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf + 1; r > 0 && f < 7; --r, ++f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr + 1, f = tf - 1; r < 7 && f > 0; ++r, --f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf - 1; r > 0 && f > 0; --r, --f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }

  return attacks;
}


bb_t precompute_rook_attack_masks(index_t square)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r < 7; ++r) {
    attacks |= (BB_1 << ((r * 8) + tf));
  }
  for (r = tr - 1; r > 0; --r) {
    attacks |= (BB_1 << ((r * 8) + tf));
  }
  for (f = tf + 1; f < 7; ++f) {
    attacks |= (BB_1 << ((tr * 8) + f));
  }
  for (f = tf - 1; f > 0; --f) {
    attacks |= (BB_1 << ((tr * 8) + f));
  }

  return attacks;
}


bb_t precompute_bishop_attacks(index_t square, bb_t blocks)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; ++r, ++f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; --r, ++f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; ++r, --f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; --r, --f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


bb_t precompute_rook_attacks(index_t square, bb_t blocks)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r <= 7; ++r) {
    const bb_t candidate = (BB_1 << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1; r >= 0; --r) {
    const bb_t candidate = (BB_1 << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf + 1; f <= 7; ++f) {
    const bb_t candidate = (BB_1 << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf - 1; f >= 0; --f) {
    const bb_t candidate = (BB_1 << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


void initialize_game_const_data(game_t* game)
{
  assert(game != nullptr);

  // initialize Hash randoms values
  init_zobrist(&game->hash_randoms);

  // Initialize bitboard tables
  bb_tables_t* tables = &game->tables;
  memset(tables, 0, sizeof(bb_tables_t));

  for (index_t square = 0; square < 64; ++square) {
    {
      // Init pawn attacks
      tables->pawn_attacks[WHITE][square] =
          precompute_pawn_attacks(WHITE, square);
      tables->pawn_attacks[BLACK][square] =
          precompute_pawn_attacks(BLACK, square);

      // Init knight attacks
      tables->knight_attacks[square] = precompute_knight_attacks(square);

      // Init king attacks
      tables->king_attacks[square] = precompute_king_attacks(square);
    }

    {  // Bishop
      tables->bishop_masks[square] = precompute_bishop_attack_masks(square);

      const bb_t bishop_attack_mask = tables->bishop_masks[square];

      const uint8_t bishop_num_relevant_bits =
          bishop_relevant_bits_count[square];

      const uint64_t bishop_occupancy_indicies =
          (BB_1 << bishop_num_relevant_bits);

      for (uint64_t index = 0; index < bishop_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, bishop_num_relevant_bits, bishop_attack_mask);

        const uint64_t magic_index =
            (occupancy * bishop_magic_numbers[square]) >>
            (64 - bishop_num_relevant_bits);

        tables->bishop_attacks[square][magic_index] =
            precompute_bishop_attacks(square, occupancy);
      }
    }

    {  // Rook
      tables->rook_masks[square] = precompute_rook_attack_masks(square);

      const bb_t rook_attack_mask = tables->rook_masks[square];
      const uint8_t rook_num_relevant_bits = rook_relevant_bits_count[square];
      const uint64_t rook_occupancy_indicies = (BB_1 << rook_num_relevant_bits);

      for (uint64_t index = 0; index < rook_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, rook_num_relevant_bits, rook_attack_mask);

        const uint64_t magic_index = (occupancy * rook_magic_numbers[square]) >>
                                     (64 - rook_num_relevant_bits);

        tables->rook_attacks[square][magic_index] =
            precompute_rook_attacks(square, occupancy);
      }
    }
  }
  // LOG_I << "Bitboard const tables initialized " << END_I;
}


piece_t get_piece(const board_t* board, index_t square)
{
  for (int piece = W_PAWN; piece <= B_KING; ++piece) {
    if (GET_BIT(board->bitboards[piece], square)) {
      return static_cast<piece_t>(piece);
    }
  }

  return EMPTY;
}
