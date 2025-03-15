#include <iostream>

#include <cassert>
#include <cmath>
#include <map>
#include <optional>
#include <pixello.hpp>
#include <random>
#include "board.hpp"
#include "data_structures.hpp"
#include "move_generator.hpp"
#include "utils.hpp"


#define LOG_I std::cout  // Start log
#define END_I std::endl  // End log

#define LOG_S LOG_I << "\033[92m"  // Success green log
#define END_S "\033[37m" << END_I  // End success green log

#define LOG_W LOG_I << "\033[33m"  // Warning orange log
#define END_W "\033[37m" << END_I  // End warning orange log

#define LOG_E LOG_I << "\033[31m"  // Error red log
#define END_E "\033[37m" << END_I  // End Error red log

static constexpr char FONT_PATH[] = "assets/gui/font/PressStart2P.ttf";

static constexpr char PICK_SOUND[] = "tick_2";
static constexpr char RELEASE_SOUND[] = "tick_4";

#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

#define U32(x) static_cast<uint32_t>(x)
#define INT(x) static_cast<int>(x)

struct fen_panel_conf_t
{
  int text_padding = 5;
  int font_size = 10;
  rect_t rect;
  pixel_t bg_color = 0x000000FF;
  pixel_t text_color = 0xFFFFFFFF;

  fen_panel_conf_t(const int screen_w, const int screen_h)
  {
    const int h = font_size + (text_padding * 2);
    rect = {0, screen_h - h, screen_w, h};
  }
};


struct chessboard_conf_t
{
  constexpr static float paddings_percentage = 0.04f;
  constexpr static int black_boundary_size_px = 2;
  int padding;
  rect_t rect;
  int square_size;
  pixel_t black_color = {109, 64, 24, 255};
  pixel_t white_color = {232, 235, 239, 255};
  bool flipped = false;

  chessboard_conf_t(const int screen_w,
                    const int screen_h,
                    const fen_panel_conf_t& fen_panel_conf)
  {
    assert(screen_w != 0);
    assert(screen_h != 0);

    const int full_size =
        (screen_w > screen_h) ? (screen_h - fen_panel_conf.rect.h) : screen_w;
    padding = std::round(static_cast<float>(full_size) * paddings_percentage);

    const int board_size = full_size - (padding * 2);
    square_size = board_size / 8;

    // This way we remove the additional pixels in case the the `board_size` is
    // not divisible by 8
    rect = {padding, padding, square_size * 8, square_size * 8};

    LOG_I << "Board width: " << rect.w << END_I;
    LOG_I << "Board padding: " << padding << END_I;
    LOG_I << "Board square size: " << square_size << END_I;
    LOG_I << "Board width/size: " << rect.w / (float)square_size << END_I;
  }
};

struct control_panel_conf
{
  rect_t rect;
  pixel_t background_color = 0x000000FF;
  pixel_t text_color = 0xFFFFFFFF;
  pixel_t text_off_color = 0XFF0000FF;
  constexpr static int text_padding = 5;
  constexpr static int font_size = 11;
  bool is_FEN_input_selected = false;
  texture_t FEN_input_texture;
  rect_t FEN_rect;

  control_panel_conf(const int screen_w,
                     const int screen_h,
                     const fen_panel_conf_t& fen_panel_conf,
                     const chessboard_conf_t& board_conf)
  {
    if (screen_w > screen_h) {
      rect.x = board_conf.rect.x + board_conf.rect.w + board_conf.padding;
      rect.w = screen_w - rect.x;

      rect.y = 0;
      rect.h = screen_h - fen_panel_conf.rect.h;
    } else {
      rect.x = 0;
      rect.y = board_conf.rect.y + board_conf.rect.h + board_conf.padding;

      rect.w = screen_w;
      rect.h = screen_h - rect.y - fen_panel_conf.rect.h;
    }

    // Get fen rect
    const rect_t pos = get_grid_rect(6, 0, 1);
    FEN_rect = {pos.x, pos.y, pos.w, pos.h + 10};
  }

  inline rect_t get_grid_rect(const int line,
                              const int col,
                              const int tot_col) const
  {
    const int w = (rect.w / tot_col) - 10;
    const int h = font_size + text_padding;
    const int x = rect.x + (w * col) + text_padding;
    const int y = rect.y + (h * line) + text_padding;

    const rect_t result = {x, y, w, h};

    return result;
  }
};


struct held_piece_t
{
  point_t offset;
  bool selected = false;
  position_t piece_board_position;
};


struct piece_animation_t
{
  enum state_t
  {
    OFF,
    RUNNING,
    DONE,
  };

  state_t state = OFF;
  int duration_ms = 200;
  uint64_t start_tick;
  position_t piece_from;
  position_t piece_to;
  piece_t piece;

  point_t start_pos;
  point_t end_pos;
  point_t total_distance;
};


struct game_piece_t
{
  piece_t piece;
  position_t position;
};


struct game_move_t
{
  piece_t piece;
  position_t from;
  position_t to;
};

struct selected_square_t
{
  bool selected = false;
  position_t position;
};

class game_t
{
private:
  board_t board;

public:
  game_t()
  {
    // Initialize the board to default
    init_board(DEFAULT_POSITION, &board);
  }


  std::vector<game_piece_t> get_pieces() const
  {
    std::vector<game_piece_t> result;
    for (index_t index = 0; index < BOARD_SIZE; ++index) {
      const piece_t piece = board.board[index];

      if (piece != EMPTY && piece != INVALID) {
        result.push_back({piece, index_to_position(index)});
      }
    }

    return result;
  }


  piece_t get_piece_at_position(const position_t& pos) const
  {
    piece_t piece = EMPTY;

    const index_t index = position_to_index(pos.file, pos.rank);
    assert(index < INVALID_BOARD_INDEX);

    piece = board.board[index];

    return piece;
  }


  color_t get_active_color() const { return board.game_state.active_color; }


  bool is_castling_available(const castling_rights_t castling) const
  {
    return (board.game_state.castling & castling);
  }


  std::optional<position_t> get_en_passant() const
  {
    if (board.game_state.en_passant != INVALID_BOARD_INDEX) {
      return index_to_position(board.game_state.en_passant);
    }

    return std::nullopt;
  }


  int get_halfmove() const { return board.game_state.halfmove_clock; }
  int get_fullmove() const { return board.game_state.fullmove_counter; }
  std::string get_fen() const { return generate_FEN(&board); }
  void set_fen(const std::string& fen) { load_FEN(fen, &board); }
  void reset() { load_FEN(DEFAULT_POSITION, &board); }

  bool make_move(const game_move_t& move)
  {
    const auto& legal_moves = generate_legal_moves(&board);

    const index_t from = position_to_index(move.from.file, move.from.rank);
    const index_t to = position_to_index(move.to.file, move.to.rank);
    const piece_t piece = move.piece;

    if (piece == INVALID || piece == EMPTY) { return false; }

    for (const auto& legal_move : legal_moves) {
      if (legal_move.from == from && legal_move.to == to &&
          legal_move.piece == piece) {
        return ::make_move(&legal_move, &board);
      }
    }

    return false;
  }

  std::vector<game_move_t> get_available_moves()
  {
    std::vector<game_move_t> result;
    const auto moves = generate_legal_moves(&board);

    for (const auto& move : moves) {
      result.push_back({move.piece, index_to_position(move.from),
                        index_to_position(move.to)});
    }

    return result;
  }
};


class gui_t : public pixello
{
private:
  const rect_t screen;
  const bool is_screen_horizontal;
  fen_panel_conf_t fen_panel_conf;
  chessboard_conf_t board_conf;
  control_panel_conf panel_conf;

  std::map<std::string, texture_t> textures;
  std::map<piece_t, texture_t> piece_textures;
  std::map<std::string, sound_t> sound_fx;
  std::map<char, texture_t> files_and_ranks_textures;
  std::map<int, font_t> fonts;
  std::map<std::string, button_t> buttons;

  game_t game;
  held_piece_t held_piece;
  piece_animation_t animation;
  selected_square_t selected_square;
  std::vector<position_t> current_piece_available_moves;

  bool enter_key_pressed = false;
  bool ai_is_moving = false;

  std::random_device rd;
  std::mt19937 gen;

public:
  gui_t(const int w, const int h)
      : pixello(w, h, "Chesso", 60),
        screen{0, 0, w, h},
        is_screen_horizontal(w > h),
        fen_panel_conf(w, h),
        board_conf(w, h, fen_panel_conf),
        panel_conf(w, h, fen_panel_conf, board_conf)
  {
    // Initialise random number generator
    gen = std::mt19937(rd());  // Mersenne Twister PRNG

    // Set sprites scaling for better look
    set_render_scaling_quality(scaling_quality_t::LINEAR);
  }

private:  // OVERRIDE
  void on_init(void*) override;
  void on_update(void*) override;

  inline void log(const std::string& msg) override { LOG_E << msg << END_E; }

private:  // EVENTS
  void handle_events();

private:  // UPDATE
  void update_state();

  void update_mouse_in_chessboard();

private:  // DRAW
  void draw_background();

  void draw_chessboard();
  void draw_squares_of_interest();
  void draw_coordinates();
  void draw_pieces();
  void draw_piece(const game_piece_t& p);

  void draw_panel();
  void draw_fen_panel();


private:  // UTILS
  rect_t get_square_rect(const int x, const int y);
  point_t piece_pos_to_matrix_pos(const position_t& pos);
  point_t piece_pos_to_screen_pixel_pos(const position_t& pos);
  position_t screen_pos_to_position(const point_t& pos);
  void start_animation(const position_t& from,
                       const position_t& to,
                       const piece_t& piece);
  point_t get_next_animation_pos();
  void draw_static_board(const rect_t& rect,
                         const std::vector<game_piece_t>& pieces);

private:  // AI
  void ai_move();
};


void gui_t::on_init(void*)
{
  fonts[20] = load_font(FONT_PATH, 20);
  fonts[panel_conf.font_size] = load_font(FONT_PATH, panel_conf.font_size);
  fonts[fen_panel_conf.font_size] =
      load_font(FONT_PATH, fen_panel_conf.font_size);

  /**
   * pawn = "P"
   * knight = "N"
   * bishop = "B"
   * rook = "R"
   * queen = "Q"
   * and king = "K
   *
   * White ("PNBRQK")
   * Black ("pnbrqk")
   */

  piece_textures[W_PAWN] = load_image("assets/gui/Chess_plt60.png");
  piece_textures[W_KNIGHT] = load_image("assets/gui/Chess_nlt60.png");
  piece_textures[W_BISHOP] = load_image("assets/gui/Chess_blt60.png");
  piece_textures[W_ROOK] = load_image("assets/gui/Chess_rlt60.png");
  piece_textures[W_QUEEN] = load_image("assets/gui/Chess_qlt60.png");
  piece_textures[W_KING] = load_image("assets/gui/Chess_klt60.png");
  piece_textures[B_PAWN] = load_image("assets/gui/Chess_pdt60.png");
  piece_textures[B_KNIGHT] = load_image("assets/gui/Chess_ndt60.png");
  piece_textures[B_BISHOP] = load_image("assets/gui/Chess_bdt60.png");
  piece_textures[B_ROOK] = load_image("assets/gui/Chess_rdt60.png");
  piece_textures[B_QUEEN] = load_image("assets/gui/Chess_qdt60.png");
  piece_textures[B_KING] = load_image("assets/gui/Chess_kdt60.png");

  textures["background"] = load_image("assets/gui/background_l.jpg");
  textures["flip"] = load_image("assets/gui/flip_icon.png");

  sound_fx["tick_1"] = load_sound("assets/gui/sound/tick_1.wav");
  sound_fx["tick_2"] = load_sound("assets/gui/sound/tick_2.wav");
  sound_fx["tick_3"] = load_sound("assets/gui/sound/tick_3.wav");
  sound_fx["tick_4"] = load_sound("assets/gui/sound/tick_4.wav");
  sound_fx["tick_5"] = load_sound("assets/gui/sound/tick_5.wav");
  sound_fx["click"] = load_sound("assets/gui/sound/click.wav");
  sound_fx["wow"] = load_sound("assets/gui/sound/anime-wow-sound-effect.mp3");

  {  // Generate the files and ranks text textures
    const font_t coordinates_font = load_font(
        "assets/gui/font/ubuntu_mono/UbuntuMono-Bold.ttf", board_conf.padding);

    for (char i = 'A'; i < 'I'; ++i) {
      files_and_ranks_textures[i] =
          create_text(std::string(1, i), 0x000000FF, coordinates_font);
    }

    for (char i = '1'; i < '9'; ++i) {
      files_and_ranks_textures[i] =
          create_text(std::string(1, i), 0x000000FF, coordinates_font);
    }
  }

  // Buttons
  {  // Flip board button

    const int32_t button_h = 18;
    const int32_t button_w = 20;
    const int32_t x = panel_conf.rect.x + panel_conf.text_padding;
    const int32_t y = panel_conf.rect.y + panel_conf.rect.h - button_h -
                      panel_conf.text_padding;

    buttons["flip"] = create_button({x, y, button_w, button_h}, 0xFFFFFFFF,
                                    textures["flip"], 0xAAAAAAFF);
  }

  {  // Copy FEN to clipboard button

    const texture_t text =
        create_text("Copy FEN", 0x000000FF, fonts[panel_conf.font_size]);

    const rect_t p = panel_conf.get_grid_rect(8, 0, 2);

    buttons["to_clipboard"] = create_button(
        {p.x, p.y, text.w + 10, text.h + 10}, 0xFFFFFFFF, text, 0xAAAAAAFF);
  }

  {  // reset button
    const texture_t text =
        create_text("Reset", 0x000000FF, fonts[panel_conf.font_size]);

    const rect_t p = panel_conf.get_grid_rect(8, 1, 2);

    buttons["reset"] = create_button({p.x, p.y, text.w + 10, text.h + 10},
                                     0xFFFFFFFF, text, 0xAAAAAAFF);
  }

  // play_sound(sound_fx["wow"]);
}


point_t gui_t::piece_pos_to_matrix_pos(const position_t& pos)
{
  point_t res;
  res.x = board_conf.flipped ? 7 - pos.file : pos.file;
  res.y = board_conf.flipped ? pos.rank : 7 - pos.rank;

  return res;
}


point_t gui_t::piece_pos_to_screen_pixel_pos(const position_t& pos)
{
  const point_t matrix_pos = piece_pos_to_matrix_pos(pos);

  const point_t result = {
      board_conf.rect.x + (matrix_pos.x * board_conf.square_size),
      board_conf.rect.y + (matrix_pos.y * board_conf.square_size)};

  return result;
}


position_t gui_t::screen_pos_to_position(const point_t& pos)
{
  position_t res;

  int x = (pos.x - board_conf.rect.x) / board_conf.square_size;
  int y = (pos.y - board_conf.rect.y) / board_conf.square_size;

  // Compensate for possible non even square size
  if (x > 7) { x = 7; }
  if (y > 7) { y = 7; }

  res.rank = board_conf.flipped ? y : 7 - y;
  res.file = board_conf.flipped ? 7 - x : x;

  return res;
}


rect_t gui_t::get_square_rect(const int x, const int y)
{
  const int square_size = board_conf.square_size;
  const int x_offset = board_conf.rect.x;
  const int y_offset = board_conf.rect.y;

  const rect_t result = {(x * square_size) + x_offset,
                         (y * square_size) + y_offset, square_size,
                         square_size};

  return result;
}


void gui_t::ai_move()
{
  // Lock the actions
  ai_is_moving = true;

  // Get list of moves for the current color
  const auto moves = game.get_available_moves();

  if (moves.size() < 1) {
    LOG_I << "NO moves found" << END_I;
    return;
  }

  // Pick a random one
  std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
  const size_t random_index = dist(gen);
  const auto& move_to_make = moves[random_index];

  // Start the animation
  start_animation(move_to_make.from, move_to_make.to, move_to_make.piece);
}


void gui_t::start_animation(const position_t& from,
                            const position_t& to,
                            const piece_t& piece)
{
  if (from == to) {
    LOG_I << "Looks like from and to is the same: " << from << END_I;
    return;
  }

  assert(piece != piece_t::EMPTY);

  if (piece == piece_t::EMPTY) {
    LOG_W << "Starting an animation with an empty piece. WTF" << END_W;
    return;
  }

  animation.state = piece_animation_t::RUNNING;
  animation.start_tick = get_ticks();
  animation.piece_from = from;
  animation.piece_to = to;
  animation.piece = piece;

  // Get the animation values in pixels
  animation.start_pos = piece_pos_to_screen_pixel_pos(from);
  animation.end_pos = piece_pos_to_screen_pixel_pos(to);

  // Calculate the total distance to move in both X and Y directions
  animation.total_distance = {(animation.end_pos.x - animation.start_pos.x),
                              (animation.end_pos.y - animation.start_pos.y)};

  play_sound(sound_fx[PICK_SOUND]);
}


point_t gui_t::get_next_animation_pos()
{
  // The animation is moving on the straight line

  if (animation.state != piece_animation_t::RUNNING) {
    throw runtime_exception(
        "trying to get an animation postion when animation is not anymore "
        "running");
  }

  // Get the current time
  const uint64_t current_tick = get_ticks();

  // Calculate the elapsed time since the animation started
  const uint64_t elapsed_time = current_tick - animation.start_tick;

  // Calculate the percentage of the animation completed
  float percentage_complete =
      elapsed_time / static_cast<float>(animation.duration_ms);


  if (percentage_complete > 1.0f) { percentage_complete = 1.0f; }

  // Calculate the current position of the animation based on the percentage
  // complete
  const point_t result = {
      animation.start_pos.x +
          INT(animation.total_distance.x * percentage_complete),
      animation.start_pos.y +
          INT(animation.total_distance.y * percentage_complete),
  };

  // In case of animation done
  if (animation.end_pos.x == result.x && animation.end_pos.y == result.y) {
    animation.state = piece_animation_t::DONE;
    return animation.end_pos;
  }

  // Return the current position
  return result;
}


void gui_t::draw_chessboard()
{
  // Draw the black boundary box
  const rect_t black_box = {
      board_conf.rect.x - board_conf.black_boundary_size_px,
      board_conf.rect.y - board_conf.black_boundary_size_px,
      board_conf.rect.w + board_conf.black_boundary_size_px * 2,
      board_conf.rect.h + board_conf.black_boundary_size_px * 2,
  };

  draw_rect(black_box, 0x000000FF);

  // Draw the tails
  bool black = false;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const pixel_t& color =
          black ? board_conf.black_color : board_conf.white_color;

      const rect_t rect = get_square_rect(x, y);

      draw_rect(rect, color);
      draw_rect_outline(rect, 0x000000FF);

      black = !black;
    }

    black = !black;
  }
}


void gui_t::draw_squares_of_interest()
{
  // Draw selected square
  if (selected_square.selected) {
    // Draw the selected square
    const point_t board_coord =
        piece_pos_to_matrix_pos(selected_square.position);
    const rect_t rect = get_square_rect(board_coord.x, board_coord.y);

    draw_rect(rect, 0xFFEE8CFF);
    draw_rect_outline(rect, 0x000000FF);

    // TODO Draw the available moves
    for (const auto& square : current_piece_available_moves) {
      const point_t board_coord = piece_pos_to_matrix_pos(square);
      const rect_t rect = get_square_rect(board_coord.x, board_coord.y);

      // draw_circle(rect.x + (rect.w / 2), rect.y + (rect.h / 2), 10,
      // 0xFF0000FF);
      draw_rect(rect, 0x88E788FF);
      draw_rect_outline(rect, 0x000000FF);
    }
  }
}


void gui_t::draw_piece(const game_piece_t& p)
{
  if (p.piece == piece_t::EMPTY) { return; }

  point_t board_coord = piece_pos_to_matrix_pos(p.position);
  const rect_t rect = get_square_rect(board_coord.x, board_coord.y);
  const texture_t& piece_texture = piece_textures[p.piece];
  draw_texture(piece_texture, rect);
}


void gui_t::draw_pieces()
{
  const auto& pieces = game.get_pieces();

  for (const auto& piece : pieces) {
    // Skip the selected piece
    if (held_piece.selected &&
        held_piece.piece_board_position == piece.position) {
      continue;
    }

    // We also should skip the piece in animation
    if (animation.state == piece_animation_t::RUNNING &&
        animation.piece_from == piece.position) {
      continue;
    }

    draw_piece(piece);
  }

  // Draw the selected piece
  if (held_piece.selected) {
    const mouse_t& mouse = mouse_state();
    const rect_t r = {mouse.x - held_piece.offset.x,
                      mouse.y - held_piece.offset.y, board_conf.square_size,
                      board_conf.square_size};

    const piece_t p =
        game.get_piece_at_position(held_piece.piece_board_position);

    draw_texture(piece_textures[p], r);
  }

  // Draw the animated piece
  if (animation.state == piece_animation_t::RUNNING) {
    const point_t next_animation_pos = get_next_animation_pos();

    const rect_t r = {next_animation_pos.x, next_animation_pos.y,
                      board_conf.square_size, board_conf.square_size};

    draw_texture(piece_textures[animation.piece], r);
  }
}


void gui_t::draw_coordinates()
{
  const rect_t& board = board_conf.rect;
  const int square_size = board_conf.square_size;
  const float flipped = board_conf.flipped;

  // Draw ranks  1 - 8
  for (int i = 0; i < 8; ++i) {
    const char c = '1' + i;
    const texture_t& t = files_and_ranks_textures[c];

    const int32_t x = (board.x - t.w) / 2;
    const int32_t rank = flipped ? i : 7 - i;
    const int32_t y =
        board.y + ((square_size * rank) + ((square_size - t.h) / 2));

    draw_texture(t, x, y);
  }

  // Draw files  A - H
  for (int i = 0; i < 8; ++i) {
    const char c = 'A' + i;
    const texture_t& t = files_and_ranks_textures[c];

    const int32_t y =
        is_screen_horizontal
            ? (board.h +
               ((screen.h - fen_panel_conf.rect.h - board.h + board.y - t.h) /
                2))
            : ((board.y - t.h) / 2);

    const int32_t file = flipped ? 7 - i : i;
    const int32_t x =
        board.x + ((square_size * file) + ((square_size - t.w) / 2));

    draw_texture(t, x, y);
  }
}


void gui_t::draw_static_board(const rect_t& rect,
                              const std::vector<game_piece_t>& pieces)
{
  // Check if the size makes sense
  assert(rect.w == rect.h);
  assert(rect.w % 8 == 0);
  assert(rect.h % 8 == 0);

  const int square_size = rect.w / 8;
  const int x_offset = rect.x;
  const int y_offset = rect.y;

  {  // Draw tiles

    bool black = false;
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        const pixel_t& color =
            black ? board_conf.black_color : board_conf.white_color;


        const rect_t rect = {(x * square_size) + x_offset,
                             (y * square_size) + y_offset, square_size,
                             square_size};

        draw_rect(rect, color);
        // draw_rect_outline(rect, 0x000000FF);

        black = !black;
      }

      black = !black;
    }
  }

  {  // Draw pieces
    for (const auto& piece : pieces) {
      const point_t board_coord = piece_pos_to_matrix_pos(piece.position);

      const rect_t rect = {(board_coord.x * square_size) + x_offset,
                           (board_coord.y * square_size) + y_offset,
                           square_size, square_size};

      const texture_t& piece_texture = piece_textures[piece.piece];
      draw_texture(piece_texture, rect);
    }
  }
}


void gui_t::draw_background()
{
  draw_texture(textures["background"], {0, 0, screen.w, screen.h});
}


void gui_t::draw_panel()
{
  const font_t& font = fonts[panel_conf.font_size];

  draw_rect(panel_conf.rect, panel_conf.background_color);

  {  // Active color
    const std::string active_text =
        "Turn: " + ((game.get_active_color() == WHITE) ? std::string("WHITE")
                                                       : std::string("BLACK"));

    const texture_t active_color =
        create_text(active_text, panel_conf.text_color, font);

    const rect_t active_color_pos = panel_conf.get_grid_rect(0, 0, 1);
    draw_texture(active_color, active_color_pos.x, active_color_pos.y);
  }

  {  // Castling rights
    const texture_t castling_rights_texture =
        create_text("Castling rights:", panel_conf.text_color, font);

    const texture_t wq = create_text(
        "WQ",
        (game.is_castling_available(WQ) ? panel_conf.text_color
                                        : panel_conf.text_off_color),
        font);
    const texture_t wk = create_text(
        "WK",
        (game.is_castling_available(WK) ? panel_conf.text_color
                                        : panel_conf.text_off_color),
        font);
    const texture_t bq = create_text(
        "BQ",
        (game.is_castling_available(BQ) ? panel_conf.text_color
                                        : panel_conf.text_off_color),
        font);
    const texture_t bk = create_text(
        "BK",
        (game.is_castling_available(BK) ? panel_conf.text_color
                                        : panel_conf.text_off_color),
        font);

    const rect_t castling_pos = panel_conf.get_grid_rect(1, 0, 1);
    draw_texture(castling_rights_texture, castling_pos.x, castling_pos.y);
    draw_texture(
        wq,
        castling_rights_texture.w + castling_pos.x + panel_conf.text_padding,
        castling_pos.y);
    draw_texture(wk,
                 castling_rights_texture.w + castling_pos.x + wq.w +
                     (panel_conf.text_padding * 2),
                 castling_pos.y);
    draw_texture(bq,
                 castling_rights_texture.w + castling_pos.x + wq.w + wk.w +
                     (panel_conf.text_padding * 3),
                 castling_pos.y);
    draw_texture(bk,
                 castling_rights_texture.w + castling_pos.x + wq.w + wk.w +
                     bq.w + (panel_conf.text_padding * 4),
                 castling_pos.y);
  }

  {  // En passant
    const rect_t en_passant_pos = panel_conf.get_grid_rect(2, 0, 1);
    const texture_t en_passant =
        create_text("En passant: ", panel_conf.text_color, font);
    draw_texture(en_passant, en_passant_pos.x, en_passant_pos.y);

    if (game.get_en_passant().has_value()) {
      const position_t pos = game.get_en_passant().value();
      const texture_t en_passant_target_square = create_text(
          index_to_string_coordinates(position_to_index(pos.file, pos.rank)),
          panel_conf.text_color, font);

      draw_texture(en_passant_target_square,
                   en_passant_pos.x + en_passant.w + panel_conf.text_padding,
                   en_passant_pos.y);
    }
  }

  {  // Half move clock
    const texture_t half_move_clock =
        create_text("Half move clock: " + STR(game.get_halfmove()),
                    panel_conf.text_color, font);
    const rect_t half_move_clock_pos = panel_conf.get_grid_rect(3, 0, 1);
    draw_texture(half_move_clock, half_move_clock_pos.x, half_move_clock_pos.y);
  }

  {  // Full move clock
    const texture_t full_move_clock =
        create_text("Full move clock: " + STR(game.get_fullmove()),
                    panel_conf.text_color, font);
    const rect_t full_move_clock_pos = panel_conf.get_grid_rect(4, 0, 1);
    draw_texture(full_move_clock, full_move_clock_pos.x, full_move_clock_pos.y);
  }

  {  // FEN Input text

    const texture_t fen_texture =
        create_text("Insert FEN: ", panel_conf.text_color, font);
    const rect_t fen_pos = panel_conf.get_grid_rect(5, 0, 1);
    draw_texture(fen_texture, fen_pos.x, fen_pos.y);

    pixel_t rect_color = 0xAAAAAAFF;
    if (panel_conf.is_FEN_input_selected) { rect_color = 0xFFFFFFFF; }
    draw_rect(panel_conf.FEN_rect, rect_color);

    if (panel_conf.FEN_input_texture.is_valid()) {
      const point_t pos = {
          panel_conf.FEN_rect.x + 2,
          panel_conf.FEN_rect.y +
              (panel_conf.FEN_rect.h - panel_conf.FEN_input_texture.h) / 2};

      draw_texture(panel_conf.FEN_input_texture, pos.x, pos.y);
    }
  }

  {  // FPS

    const texture_t fps_texture =
        create_text("FPS: " + STR(FPS()), panel_conf.text_color, font);

    const int32_t x = panel_conf.rect.x + panel_conf.rect.w - fps_texture.w -
                      panel_conf.text_padding;
    const int32_t y = panel_conf.rect.y + panel_conf.rect.h - fps_texture.h -
                      panel_conf.text_padding;

    draw_texture(fps_texture, x, y);
  }

  {  // Flip button
    const button_t& button = buttons["flip"];
    draw_button_with_icon(button);
  }

  {  // Copy to flip button
    const button_t& button = buttons["to_clipboard"];
    draw_button(button);
  }

  {  // Copy to flip button
    const button_t& button = buttons["reset"];
    draw_button(button);
  }
}


void gui_t::draw_fen_panel()
{
  draw_rect(fen_panel_conf.rect, fen_panel_conf.bg_color);

  const std::string fen = game.get_fen();
  const font_t& font = fonts[fen_panel_conf.font_size];
  const texture_t t = create_text(fen, fen_panel_conf.text_color, font);
  const point_t p = {
      fen_panel_conf.rect.x + fen_panel_conf.text_padding,
      fen_panel_conf.rect.y + ((fen_panel_conf.rect.h - t.h) / 2)};

  draw_texture(t, p.x, p.y);
}


void gui_t::handle_events()
{
  if (is_key_pressed(keycap_t::M) && !ai_is_moving) { ai_move(); }
}


void gui_t::update_state()
{
  const mouse_t& mouse = mouse_state();

  // Check if quit the app
  if (is_key_pressed(keycap_t::ESC)) { stop(); }

  // Check if flip the board
  if (is_mouse_in(buttons["flip"].rect) && mouse.left_button.click) {
    play_sound(sound_fx["click"]);
    board_conf.flipped = !board_conf.flipped;
  }

  // Check copy FEN to clipboard button
  if (is_mouse_in(buttons["to_clipboard"].rect) && mouse.left_button.click) {
    play_sound(sound_fx["click"]);
    const std::string FEN = game.get_fen();
    set_to_clipboard(FEN);
  }

  // Reset game
  if (is_mouse_in(buttons["reset"].rect) && mouse.left_button.click) {
    play_sound(sound_fx["click"]);
    game.reset();
  }

  // Check input text
  if (is_mouse_in(panel_conf.FEN_rect) && mouse.left_button.click) {
    panel_conf.is_FEN_input_selected = true;
    start_text_input();
  } else if (mouse.left_button.click) {
    panel_conf.is_FEN_input_selected = false;
    stop_text_input();
  }

  // Update input text texture in case
  if (panel_conf.is_FEN_input_selected && should_render_text()) {
    const std::string& FEN = get_input_text();
    const font_t& font = fonts[panel_conf.font_size];
    panel_conf.FEN_input_texture = create_text(FEN, 0x000000FF, font);
  }

  // Reset the enter key press
  if (!is_key_pressed(keycap_t::ENTER)) { enter_key_pressed = false; }

  // Check if enter is pressed in the FEN field
  if (panel_conf.is_FEN_input_selected && is_key_pressed(keycap_t::ENTER) &&
      !enter_key_pressed) {
    const std::string& FEN = get_input_text();

    try {
      game.set_fen(FEN);
      panel_conf.is_FEN_input_selected = false;
      stop_text_input();

    } catch (const std::exception& e) {
      LOG_W << "Error loading FEN string. " + std::string(e.what()) << END_W;
      // Cleanup also the render texture
      const font_t& font = fonts[panel_conf.font_size];
      panel_conf.FEN_input_texture = create_text(" ", 0x000000FF, font);
    }

    clear_input_text_buffer();
    enter_key_pressed = true;
  }


  // Check animation
  if (animation.state == piece_animation_t::DONE) {
    // Stop animation
    animation.state = piece_animation_t::OFF;

    // Sound
    play_sound(sound_fx[RELEASE_SOUND]);

    // Set the piece
    const game_move_t move = {animation.piece, animation.piece_from,
                              animation.piece_to};

    game.make_move(move);

    // Unlock the ai move
    ai_is_moving = false;
  }
}


void gui_t::update_mouse_in_chessboard()
{
  {  // If we are not in the chessboard we return
    const rect_t board_safe_rect = {
        board_conf.rect.x + 1, board_conf.rect.y + 1, board_conf.rect.w - 1,
        board_conf.rect.h - 1};

    if (!is_mouse_in(board_safe_rect)) { return; }
  }

  const mouse_t& mouse = mouse_state();
  const point_t mouse_pos = {mouse.x, mouse.y};
  const position_t mouse_board_pos = screen_pos_to_position(mouse_pos);
  const point_t mouse_tail = piece_pos_to_matrix_pos(mouse_board_pos);

  const piece_t pointed_piece = game.get_piece_at_position(mouse_board_pos);

  // Check if we should start holding the piece
  if (pointed_piece != piece_t::EMPTY &&
      (mouse.left_button.state == button_key_t::DOWN && !held_piece.selected)) {
    // Calculate the texture offset
    held_piece.offset = {mouse.x - (board_conf.padding +
                                    (mouse_tail.x * board_conf.square_size)),
                         mouse.y - (board_conf.padding +
                                    (mouse_tail.y * board_conf.square_size))};

    held_piece.selected = true;
    held_piece.piece_board_position = mouse_board_pos;

    // Play the soft sound
    play_sound(sound_fx[PICK_SOUND]);
  }

  // Reset the selected state
  if (mouse.left_button.state == button_key_t::UP && held_piece.selected) {
    // Set the piece to the destination column when release
    if (mouse_board_pos != held_piece.piece_board_position) {
      const game_move_t move = {
          game.get_piece_at_position(held_piece.piece_board_position),
          held_piece.piece_board_position, mouse_board_pos};

      // Make the move. In case the move happened then clear the selected square
      if (game.make_move(move)) {
        // Unselect the selected square
        selected_square.selected = false;
      }
    }

    held_piece.selected = false;
    held_piece.offset.x = 0;
    held_piece.offset.y = 0;

    // Play sound
    play_sound(sound_fx[RELEASE_SOUND]);
  }

  // Click on the square
  if (mouse.left_button.click) {
    if (selected_square.selected &&
        selected_square.position == mouse_board_pos) {
      // If click selected then unselect
      selected_square.selected = false;

      // Clear the available moves as well
      current_piece_available_moves.clear();
    } else {
      selected_square.position = mouse_board_pos;
      selected_square.selected = true;

      // If click on piece get the suggested moves
      const piece_t piece =
          game.get_piece_at_position(selected_square.position);
      if (piece != EMPTY && piece != INVALID) {
        // Get moves
        const auto& all_moves = game.get_available_moves();
        current_piece_available_moves.clear();
        for (const auto& move : all_moves) {
          if (move.piece == piece && move.from == selected_square.position) {
            current_piece_available_moves.push_back(move.to);
          }
        }
      }
    }
  }
}


void gui_t::on_update(void*)
{
  // EVENTS
  handle_events();

  // UPDATE
  update_state();
  update_mouse_in_chessboard();

  // DRAW
  draw_background();
  draw_chessboard();
  draw_squares_of_interest();
  draw_coordinates();
  draw_pieces();

  draw_panel();
  draw_fen_panel();

  // Test draw mini board
  draw_static_board({panel_conf.rect.x, panel_conf.rect.y + 200, 160, 160},
                    game.get_pieces());
}


int main()
{
  int screen_h = 600;
  int screen_w = 1100;
  gui_t gui(screen_w, screen_h);

  if (!gui.run()) { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}
