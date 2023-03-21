#include "gui.hpp"
#include "exceptions.hpp"

static constexpr char FEN_INIT_POS[] =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static constexpr char PICK_SOUND[] = "tick_2";
static constexpr char RELEASE_SOUND[] = "tick_4";

void gui_t::on_init(void*)
{
  fonts[20] = load_font("assets/font/PressStart2P.ttf", 20);
  fonts[panel_conf.font_size] =
      load_font("assets/font/PressStart2P.ttf", panel_conf.font_size);

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

  piece_textures[gui::piece_t::W_PAWN] = load_image("assets/Chess_plt60.png");
  piece_textures[gui::piece_t::W_KNIGHT] = load_image("assets/Chess_nlt60.png");
  piece_textures[gui::piece_t::W_BISHOP] = load_image("assets/Chess_blt60.png");
  piece_textures[gui::piece_t::W_ROOK] = load_image("assets/Chess_rlt60.png");
  piece_textures[gui::piece_t::W_QUEEN] = load_image("assets/Chess_qlt60.png");
  piece_textures[gui::piece_t::W_KING] = load_image("assets/Chess_klt60.png");
  piece_textures[gui::piece_t::B_PAWN] = load_image("assets/Chess_pdt60.png");
  piece_textures[gui::piece_t::B_KNIGHT] = load_image("assets/Chess_ndt60.png");
  piece_textures[gui::piece_t::B_BISHOP] = load_image("assets/Chess_bdt60.png");
  piece_textures[gui::piece_t::B_ROOK] = load_image("assets/Chess_rdt60.png");
  piece_textures[gui::piece_t::B_QUEEN] = load_image("assets/Chess_qdt60.png");
  piece_textures[gui::piece_t::B_KING] = load_image("assets/Chess_kdt60.png");

  textures["background"] = load_image("assets/background_l.jpg");
  textures["flip"] = load_image("assets/flip_icon.png");

  sound_fx["tick_1"] = load_sound("assets/sound/tick_1.wav");
  sound_fx["tick_2"] = load_sound("assets/sound/tick_2.wav");
  sound_fx["tick_3"] = load_sound("assets/sound/tick_3.wav");
  sound_fx["tick_4"] = load_sound("assets/sound/tick_4.wav");
  sound_fx["tick_5"] = load_sound("assets/sound/tick_5.wav");
  sound_fx["click"] = load_sound("assets/sound/click.wav");
  sound_fx["wow"] = load_sound("assets/sound/anime-wow-sound-effect.mp3");

  {  // Generate the files and ranks text textures
    const font_t coordinates_font = load_font(
        "assets/font/ubuntu_mono/UbuntuMono-Bold.ttf", board_conf.padding);

    for (char i = 'A'; i < 'I'; ++i) {
      files_and_ranks_textures[i] =
          create_text(std::string(1, i), 0x000000FF, coordinates_font);
    }

    for (char i = '1'; i < '9'; ++i) {
      files_and_ranks_textures[i] =
          create_text(std::string(1, i), 0x000000FF, coordinates_font);
    }
  }

  // Load default state
  state = gui::load_FEN(FEN_INIT_POS);

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

  play_sound(sound_fx["wow"]);
}


point_t gui_t::piece_pos_to_matrix_pos(const gui::position_t& pos)
{
  point_t res;
  res.x = board_conf.flipped ? 7 - pos.file : pos.file;
  res.y = board_conf.flipped ? pos.rank : 7 - pos.rank;

  return res;
}


point_t gui_t::piece_pos_to_screen_pixel_pos(const gui::position_t& pos)
{
  const point_t matrix_pos = piece_pos_to_matrix_pos(pos);

  const point_t result = {
      board_conf.rect.x + (matrix_pos.x * board_conf.square_size),
      board_conf.rect.y + (matrix_pos.y * board_conf.square_size)};

  return result;
}


gui::position_t gui_t::screen_pos_to_position(const point_t& pos)
{
  gui::position_t res;

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


void gui_t::start_animation(const gui::position_t& from,
                            const gui::position_t& to)
{
  if (from == to) {
    LOG_I << "Looks like from and to is the same: " << from << END_I;
    return;
  }

  // If the piece is empty we skip the animation
  const gui::piece_t piece = gui::get_piece(state, from);

  assert(piece != gui::piece_t::EMPTY);

  if (piece == gui::piece_t::EMPTY) {
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


void gui_t::draw_piece(const gui::piece_t p, const gui::position_t& pos)
{
  if (p == gui::piece_t::EMPTY) { return; }

  point_t board_coord = piece_pos_to_matrix_pos(pos);
  const rect_t rect = get_square_rect(board_coord.x, board_coord.y);
  const texture_t& piece_texture = piece_textures[p];
  draw_texture(piece_texture, rect);
}


void gui_t::draw_pieces()
{
  const auto& board = state.board;
  for (uint8_t i = 0; i < 8; ++i) {
    for (uint8_t j = 0; j < 8; ++j) {
      const auto& piece = board[i][j];

      if (piece != gui::piece_t::EMPTY) {
        const gui::position_t pos = {i, j};  // rank, files

        // Skip the selected piece
        if (held_piece.selected && held_piece.piece_board_position == pos) {
          continue;
        }

        // We also should skip the piece in animation
        if (animation.state == piece_animation_t::RUNNING &&
            animation.piece_from == pos) {
          continue;
        }

        draw_piece(piece, pos);
      }
    }
  }

  // Draw the selected piece
  if (held_piece.selected) {
    const mouse_t& mouse = mouse_state();
    const rect_t r = {mouse.x - held_piece.offset.x,
                      mouse.y - held_piece.offset.y, board_conf.square_size,
                      board_conf.square_size};

    const gui::piece_t p =
        gui::get_piece(state, held_piece.piece_board_position);

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
            ? (board.h + ((screen.h - board.h + board.y - t.h) / 2))
            : ((board.y - t.h) / 2);

    const int32_t file = flipped ? 7 - i : i;
    const int32_t x =
        board.x + ((square_size * file) + ((square_size - t.w) / 2));

    draw_texture(t, x, y);
  }
}


void gui_t::draw_background()
{
  draw_texture(textures["background"], {0, 0, screen.w, screen.h});
}


#include <bitset>
#include <iostream>

void gui_t::draw_panel()
{
  const font_t& font = fonts[panel_conf.font_size];

  draw_rect(panel_conf.rect, panel_conf.background_color);

  {  // Active color
    const std::string active_text =
        "Turn: " + ((state.active_color == gui::color_t::WHITE)
                        ? std::string("WHITE")
                        : std::string("BLACK"));

    const texture_t active_color =
        create_text(active_text, panel_conf.text_color, font);

    const rect_t active_color_pos = panel_conf.get_grid_rect(0, 0, 1);
    draw_texture(active_color, active_color_pos.x, active_color_pos.y);
  }

  {  // Castling rights
    const texture_t castling_rights_texture =
        create_text("Castling rights:", panel_conf.text_color, font);

    const texture_t wq =
        create_text("WQ",
                    ((state.castling_rights & gui::castling_rights_t::WQ)
                         ? panel_conf.text_color
                         : panel_conf.text_off_color),
                    font);
    const texture_t wk =
        create_text("WK",
                    ((state.castling_rights & gui::castling_rights_t::WK)
                         ? panel_conf.text_color
                         : panel_conf.text_off_color),
                    font);
    const texture_t bq =
        create_text("BQ",
                    ((state.castling_rights & gui::castling_rights_t::BQ)
                         ? panel_conf.text_color
                         : panel_conf.text_off_color),
                    font);
    const texture_t bk =
        create_text("BK",
                    ((state.castling_rights & gui::castling_rights_t::BK)
                         ? panel_conf.text_color
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

    if (state.is_en_passant) {
      const texture_t en_passant_target_square =
          create_text(gui::pos_to_algebraic(state.en_passant_target_square),
                      panel_conf.text_color, font);

      draw_texture(en_passant_target_square,
                   en_passant_pos.x + en_passant.w + panel_conf.text_padding,
                   en_passant_pos.y);
    }
  }

  {  // Half move clock
    const texture_t half_move_clock =
        create_text("Half move clock: " + STR(state.half_move_clock),
                    panel_conf.text_color, font);
    const rect_t half_move_clock_pos = panel_conf.get_grid_rect(3, 0, 1);
    draw_texture(half_move_clock, half_move_clock_pos.x, half_move_clock_pos.y);
  }

  {  // Full move clock
    const texture_t full_move_clock =
        create_text("Full move clock: " + STR(state.full_move_clock),
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


void gui_t::handle_events()
{
  // if (is_key_pressed(keycap_t::A) && !a) {
  //   start_animation({1, 2}, {1, 3});
  //   a = true;
  // }
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
    const std::string FEN = gui::state_to_FEN(state);
    set_to_clipboard(FEN);
  }

  // Check copy FEN to clipboard button
  if (is_mouse_in(buttons["reset"].rect) && mouse.left_button.click) {
    play_sound(sound_fx["click"]);
    state = gui::load_FEN(FEN_INIT_POS);
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
      state = gui::load_FEN(FEN);
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
    gui::set_piece(state, animation.piece_to, animation.piece);
    gui::set_piece(state, animation.piece_from, gui::piece_t::EMPTY);
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
  const gui::position_t mouse_board_pos = screen_pos_to_position(mouse_pos);
  const point_t mouse_tail = piece_pos_to_matrix_pos(mouse_board_pos);

  const gui::piece_t pointed_piece = gui::get_piece(state, mouse_board_pos);

  // Check if we should start holding the piece
  if (pointed_piece != gui::piece_t::EMPTY &&
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
    // if (mouse_tail != selected_piece) {
    // TODO: In this case we want to unselect the square
    // asm("nop");
    // }

    // Set the piece to the destination column when release
    if (mouse_board_pos != held_piece.piece_board_position) {
      const gui::piece_t piece =
          gui::get_piece(state, held_piece.piece_board_position);

      gui::set_piece(state, mouse_board_pos, piece);
      gui::set_piece(state, held_piece.piece_board_position,
                     gui::piece_t::EMPTY);

      // TODO: Move the piece with the engine
    }

    held_piece.selected = false;
    held_piece.offset.x = 0;
    held_piece.offset.y = 0;

    // Play sound
    play_sound(sound_fx[RELEASE_SOUND]);
  }

  // // Click on the square
  // if (mouse.left_button.click) {
  //   if (selected_square.selected &&
  //       selected_square.position == mouse_board_position) {
  //     // If click selected then unselect
  //     selected_square.selected = false;
  //     suggested_positions.clear();
  //   } else {
  //     selected_square.position = mouse_board_position;
  //     selected_square.selected = true;

  //     // Get the available moves
  //     const position_t selected_pos =
  //         coordinates_to_postion(mouse_tail_coord.x, mouse_tail_coord.y);
  //     suggested_positions = chess.get_valid_moves(selected_pos);
  //   }
  // }
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
  draw_coordinates();
  draw_pieces();

  draw_panel();
}