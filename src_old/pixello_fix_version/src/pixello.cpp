#include "pixello.hpp"
#include <SDL_mixer.h>
#include <iostream>
#include "SDL2_gfxPrimitives.h"
#include "SDL_image.h"
#include "SDL_render.h"
#include "SDL_ttf.h"

// TODO: Check all return code from SDL and return an error in case of failure

/*******************************************************************************
 * CONSTANT VALUES
 ******************************************************************************/
// clang-format off
constexpr SDL_Scancode KEYCAP_MAP[] = {
  SDL_SCANCODE_ESCAPE,
  SDL_SCANCODE_Q,
  SDL_SCANCODE_W,
  SDL_SCANCODE_E,
  SDL_SCANCODE_R,
  SDL_SCANCODE_T,
  SDL_SCANCODE_Y,
  SDL_SCANCODE_U,
  SDL_SCANCODE_I,
  SDL_SCANCODE_O,
  SDL_SCANCODE_P,
  SDL_SCANCODE_A,
  SDL_SCANCODE_S,
  SDL_SCANCODE_D,
  SDL_SCANCODE_F,
  SDL_SCANCODE_G,
  SDL_SCANCODE_H,
  SDL_SCANCODE_J,
  SDL_SCANCODE_K,
  SDL_SCANCODE_L,
  SDL_SCANCODE_Z,
  SDL_SCANCODE_X,
  SDL_SCANCODE_C,
  SDL_SCANCODE_V,
  SDL_SCANCODE_B,
  SDL_SCANCODE_N,
  SDL_SCANCODE_M,
  SDL_SCANCODE_KP_1,
  SDL_SCANCODE_KP_2,
  SDL_SCANCODE_KP_3,
  SDL_SCANCODE_KP_4,
  SDL_SCANCODE_KP_5,
  SDL_SCANCODE_KP_6,
  SDL_SCANCODE_KP_7,
  SDL_SCANCODE_KP_8,
  SDL_SCANCODE_KP_9,
  SDL_SCANCODE_KP_0,
  SDL_SCANCODE_LEFT,
  SDL_SCANCODE_RIGHT,
  SDL_SCANCODE_UP,
  SDL_SCANCODE_DOWN,
  SDL_SCANCODE_RETURN,
  SDL_SCANCODE_SPACE,
  SDL_SCANCODE_LSHIFT
};
// clang-format on


/*******************************************************************************
 * STRUCTS
 ******************************************************************************/

sdl_texture_wrapper_t::~sdl_texture_wrapper_t()
{
  if (ptr) {
    SDL_DestroyTexture(ptr);
    ptr = NULL;
  }
}

sdl_sound_wrapper_t::~sdl_sound_wrapper_t()
{
  if (music_ptr) {
    Mix_FreeMusic(music_ptr);
    music_ptr = NULL;
  }

  if (chunk_ptr) {
    Mix_FreeChunk(chunk_ptr);
    chunk_ptr = NULL;
  }
}

simple_timer::simple_timer()
{
  start_ticks = 0;
  paused_ticks = 0;

  paused = false;
  started = false;
}

void simple_timer::start()
{
  started = true;
  paused = false;

  // Get the current clock time
  start_ticks = SDL_GetTicks64();
  paused_ticks = 0;
}

void simple_timer::stop()
{
  started = false;
  paused = false;
  start_ticks = 0;
  paused_ticks = 0;
}

void simple_timer::pause()
{
  // If the timer is running and isn't already paused
  if (started && !paused) {
    paused = true;

    // Calculate the paused ticks
    paused_ticks = SDL_GetTicks64() - start_ticks;
    start_ticks = 0;
  }
}

void simple_timer::restart()
{
  if (started) {
    stop();
    start();
  }
}

void simple_timer::unpause()
{
  // If the timer is running and paused
  if (started && paused) {
    paused = false;

    // Reset the starting ticks
    start_ticks = SDL_GetTicks64() - paused_ticks;
    paused_ticks = 0;
  }
}

uint64_t simple_timer::get_ticks() const
{
  uint64_t time = 0;

  if (started) {
    if (paused) {
      // Return the number of ticks when the timer was paused
      time = paused_ticks;
    } else {
      // Return the current time minus the start time
      time = SDL_GetTicks64() - start_ticks;
    }
  }

  return time;
}

/*******************************************************************************
 * PIXELLO CLASS
 ******************************************************************************/

pixello::~pixello()
{
  if (_font) { TTF_CloseFont(_font); }
  if (_renderer) { SDL_DestroyRenderer(_renderer); }
  if (_window) { SDL_DestroyWindow(_window); }

  Mix_Quit();
  IMG_Quit();
  SDL_Quit();
}

void pixello::log(const std::string& msg)
{
  std::cout << msg << std::endl;
}

void pixello::init()
{
  _running = true;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    throw init_exception("SDL could not initialize! SDL_Error: " +
                         std::string(SDL_GetError()));
  }

  // Set texture filtering to linear
  if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == false) {
    throw init_exception("Failed to setup the renderer: " +
                         std::string(SDL_GetError()));
  }

  // Init SDL IMG
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    throw init_exception("SDL_image could not initialize! SDL_image Error: " +
                         std::string(IMG_GetError()));
  }

  // Init SDL Mixer
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    throw init_exception("SDL_mixer could not initialize! SDL_mixer Error: " +
                         std::string(Mix_GetError()));
  }

  // Initialize SDL_ttf
  if (TTF_Init() == -1) {
    throw init_exception("SDL_ttf could not initialize! SDL_ttf Error: " +
                         std::string(TTF_GetError()));
  }

  // Create window
  _window = SDL_CreateWindow(_config.name.c_str(), SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, _config.window_w,
                             _config.window_h, SDL_WINDOW_SHOWN);

  if (_window == NULL) {
    throw init_exception("Window could not be created! SDL_Error: " +
                         std::string(SDL_GetError()));
  }

  // Get the window renderer
  _renderer = SDL_CreateRenderer(
      _window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (_renderer == NULL) {
    throw init_exception("Failed to create a window renderer! SDL_Error: " +
                         std::string(SDL_GetError()));
  }

  // Load the font id required
  if (_config.font_path.empty() == false) {
    _font = TTF_OpenFont(_config.font_path.c_str(), _config.font_size);

    if (_font == NULL) {
      throw init_exception("Failed to load the font! SDL_Error: " +
                           std::string(TTF_GetError()));
    }
  }

  // Initialize renderer color
  SDL_SetRenderDrawColor(_renderer, 0xFF, 0xFF, 0xFF, 0xFF);

  // Enable Blend mode
  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

  // CALL THE USER INIT
  on_init(_external_data);
}


bool pixello::run()
{
  try {
    init();

    SDL_Event event;
    uint32_t FPS_counter = 0;
    uint64_t FPS_last_check = SDL_GetPerformanceCounter();
    uint64_t start = SDL_GetPerformanceCounter();
    /*************************************************************
     *                          MAIN LOOP                        *
     *************************************************************/
    while (_running) {
      const uint64_t now = SDL_GetPerformanceCounter();
      dt = now - start;
      start = now;

      // Reset did mouse moved flag
      _mouse_state.did_mouse_moved = false;
      _mouse_state.relative_x = 0;
      _mouse_state.relative_y = 0;
      _mouse_state.central_button_pressed = false;
      _mouse_state.left_button_pressed = false;
      _mouse_state.right_button_pressed = false;

      // POLL EVENTS
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            _running = false;
            break;

          // MOUSE SECTION
          case SDL_MOUSEMOTION: {
            _mouse_state.x = event.motion.x;
            _mouse_state.y = event.motion.y;
            _mouse_state.relative_x = event.motion.xrel;
            _mouse_state.relative_y = event.motion.yrel;
            _mouse_state.did_mouse_moved = true;
          } break;

          case SDL_MOUSEBUTTONDOWN:
            switch (event.button.button) {
              case SDL_BUTTON_LEFT:
                _mouse_state.left_button_pressed = true;
                _mouse_state.left_button.state = button_key_t::DOWN;
                _mouse_state.left_button.click =
                    event.button.clicks == 1 ? true : false;
                _mouse_state.left_button.double_click =
                    event.button.clicks == 2 ? true : false;
                break;
              case SDL_BUTTON_RIGHT:
                _mouse_state.right_button_pressed = true;
                _mouse_state.right_button.state = button_key_t::DOWN;
                _mouse_state.right_button.click =
                    event.button.clicks == 1 ? true : false;
                _mouse_state.right_button.double_click =
                    event.button.clicks == 2 ? true : false;
                break;
              case SDL_BUTTON_MIDDLE:
                _mouse_state.central_button_pressed = true;
                _mouse_state.central_button.state = button_key_t::DOWN;
                _mouse_state.central_button.click =
                    event.button.clicks == 1 ? true : false;
                _mouse_state.central_button.double_click =
                    event.button.clicks == 2 ? true : false;
                break;
            }
            break;

          case SDL_MOUSEBUTTONUP:
            switch (event.button.button) {
              case SDL_BUTTON_LEFT:
                _mouse_state.left_button.state = button_key_t::UP;
                break;
              case SDL_BUTTON_RIGHT:
                _mouse_state.right_button.state = button_key_t::UP;
                break;
              case SDL_BUTTON_MIDDLE:
                _mouse_state.central_button.state = button_key_t::UP;
                break;
            }
            break;
        }
      }

      // Clear
      const pixel_t& bg_color = _config.background_color;
      SDL_SetRenderDrawColor(_renderer, bg_color.r, bg_color.g, bg_color.b,
                             bg_color.a);
      SDL_RenderClear(_renderer);

      // Reset the viewport to entire window
      SDL_RenderSetViewport(_renderer, NULL);

      // Set the alpha channel blend mode
      SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

      // USER UPDATE
      on_update(_external_data);

      // Reset mouse click state
      mouse_reset_clicks();

      // DRAW
      SDL_RenderPresent(_renderer);

      // PERFORMANCE
      const uint64_t end = SDL_GetPerformanceCounter();
      const float freq = static_cast<float>(SDL_GetPerformanceFrequency());
      const float elapsed_s = (end - start) / freq;
      const float sleep_for_s = _config.target_s_per_frame - elapsed_s;
      const uint64_t sleep_for_ms =
          static_cast<uint64_t>(sleep_for_s * 1000.0f);

      // log("Sleep for: " + STR(sleep_for_ms) + "ms");

      if (sleep_for_s > 0) { SDL_Delay(sleep_for_ms); }
      //  else {
      //   log("Frame too slow");
      // }

      const float FPS_elapsed_s = (end - FPS_last_check) / freq;
      if (FPS_elapsed_s > 1.0f) {
        FPS_last_check = end;
        // log("FPS: " + STR(FPS_counter));
        _FPS = FPS_counter;
        FPS_counter = 0;
      } else {
        ++FPS_counter;
      }
    }
  } catch (pixello_exception& e) {
    log("Exception: " + std::string(e.what()));
    return false;
  }

  return true;
}


void pixello::draw_pixel(const int32_t x,
                         const int32_t y,
                         const pixel_t& p) const
{
  SDL_Rect rect = {x * _config.pixel_size, y * _config.pixel_size,
                   _config.pixel_size, _config.pixel_size};

  SDL_SetRenderDrawColor(_renderer, p.r, p.g, p.b, p.a);
  SDL_RenderFillRect(_renderer, &rect);
}


void pixello::draw_rect(const rect_t& rect, const pixel_t& p) const
{
  SDL_SetRenderDrawColor(_renderer, p.r, p.g, p.b, p.a);
  SDL_RenderFillRect(_renderer, (SDL_Rect*)&rect);
}


void pixello::draw_rect_outline(const rect_t& rect, const pixel_t& p) const
{
  SDL_SetRenderDrawColor(_renderer, p.r, p.g, p.b, p.a);
  SDL_RenderDrawRect(_renderer, (SDL_Rect*)&rect);
}


void pixello::draw_line(const point_t& a,
                        const point_t& b,
                        const pixel_t& p) const
{
  SDL_SetRenderDrawColor(_renderer, p.r, p.g, p.b, p.a);
  SDL_RenderDrawLine(_renderer, a.x, a.y, b.x, b.y);
}


void pixello::draw_dot(const point_t& a, const pixel_t& p) const
{
  SDL_SetRenderDrawColor(_renderer, p.r, p.g, p.b, p.a);
  SDL_RenderDrawPoint(_renderer, a.x, a.y);
}


void pixello::draw_texture(const texture_t& t, const rect_t& rect) const
{
  SDL_RenderCopy(_renderer, t.pointer(), NULL, (SDL_Rect*)&rect);
}


void pixello::draw_texture(const texture_t& t,
                           const rect_t& rect,
                           const rect_t& clip) const
{
  SDL_RenderCopy(_renderer, t.pointer(), (SDL_Rect*)&clip, (SDL_Rect*)&rect);
}


void pixello::draw_texture(const texture_t& t,
                           const int32_t x,
                           const int32_t y) const
{
  const rect_t rect = {x, y, t.w, t.h};
  draw_texture(t, rect);
}


void pixello::play_sound(const sound_t& sound) const
{
  (void)Mix_PlayChannel(-1, sound.pointer(), 0);
}

float pixello::set_sound_volume(const sound_t& sound, const float volume) const
{
  if (volume >= 1.0f || volume <= .0f) {
    throw input_exception("Invalid volume value: " + STR(volume));
  }

  const int V = static_cast<int>(MIX_MAX_VOLUME * volume);
  const int old_volume = Mix_VolumeChunk(sound.pointer(), V);

  if (old_volume < 0 || old_volume > MIX_MAX_VOLUME) {
    throw runtime_exception("Failed to configure the volume for the chunk");
  }

  // Normalizing the value
  const float result = old_volume / static_cast<float>(MIX_MAX_VOLUME);
  return result;
}

void pixello::set_master_volume(const float value) const
{
  if (value >= 1.0f || value <= .0f) {
    throw input_exception("Invalid volume value: " + STR(value));
  }

  // De-normalizing the value from range [0,1] to range [0, 128]
  const int volume = static_cast<int>(MIX_MAX_VOLUME * value);

  (void)Mix_VolumeMusic(volume);
  (void)Mix_Volume(-1, volume);
}


void pixello::set_music_volume(const float value) const
{
  if (value >= 1.0f || value <= .0f) {
    throw input_exception("Invalid volume value: " + STR(value));
  }

  // De-normalizing the value from range [0,1] to range [0, 128]
  const int volume = static_cast<int>(MIX_MAX_VOLUME * value);

  (void)Mix_VolumeMusic(volume);
}


void pixello::set_sound_volume(const float value) const
{
  if (value >= 1.0f || value <= .0f) {
    throw input_exception("Invalid volume value: " + STR(value));
  }

  // De-normalizing the value from range [0,1] to range [0, 128]
  const int volume = static_cast<int>(MIX_MAX_VOLUME * value);

  (void)Mix_Volume(-1, volume);
}


void pixello::set_current_viewport(const rect_t& rect, const pixel_t& c)
{
  // Set viewport
  SDL_RenderSetViewport(_renderer, (SDL_Rect*)&rect);

  // Set background color for view port
  SDL_Rect rect2 = {0, 0, rect.w, rect.h};
  SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
  SDL_RenderFillRect(_renderer, &rect2);
}


void pixello::play_music(const music_t& music) const
{
  const int result = Mix_PlayMusic(music.pointer(), -1);

  if (result == -1) {
    throw runtime_exception("Failed to play music: " +
                            std::string(Mix_GetError()));
  }
}


void pixello::pause_sounds() const
{
  Mix_Pause(-1);
}


void pixello::resume_sounds() const
{
  Mix_Resume(-1);
}


void pixello::pause_music() const
{
  Mix_PauseMusic();
}


void pixello::resume_music() const
{
  Mix_ResumeMusic();
}


void pixello::stop_music() const
{
  (void)Mix_HaltMusic();
}


texture_t pixello::load_image(const std::string& img_path) const
{
  SDL_Texture* tmp_ptr = IMG_LoadTexture(_renderer, img_path.c_str());

  if (!tmp_ptr) {
    throw load_exceptions("Unable to load image to texture: " + img_path +
                          "! SDL Error: " + std::string(IMG_GetError()));
  }

  texture_t t;
  t._ptr = std::make_shared<sdl_texture_wrapper_t>(tmp_ptr);

  SDL_QueryTexture(t._ptr.get()->ptr, NULL, NULL, &t.w, &t.h);

  return t;
}


texture_t pixello::create_text(const std::string& text,
                               const pixel_t& color) const
{
  const SDL_Color c = {color.r, color.g, color.b, color.a};

  // Render text surface
  SDL_Surface* txt_surface = TTF_RenderText_Solid(_font, text.c_str(), c);

  if (txt_surface == NULL) {
    throw load_exceptions("Failed to generate the text surface: " + text +
                          " Error: " + std::string(TTF_GetError()));
  }

  // Create texture from surface pixels
  auto tmp = SDL_CreateTextureFromSurface(_renderer, txt_surface);

  if (!tmp) {
    // Free the surface in any case
    SDL_FreeSurface(txt_surface);

    throw load_exceptions("Failed to generate the text texture: " + text +
                          " Error: " + std::string(TTF_GetError()));
  }

  texture_t t;
  t._ptr = std::make_shared<sdl_texture_wrapper_t>(tmp);

  // Get image dimensions
  t.w = txt_surface->w;
  t.h = txt_surface->h;

  // Get rid of old surface
  SDL_FreeSurface(txt_surface);

  return t;
}


sound_t pixello::load_sound(const std::string& sound_path) const
{
  auto tmp = Mix_LoadWAV(sound_path.c_str());

  if (!tmp) {
    throw load_exceptions("Failed to load sound! " + sound_path +
                          ". SDL_mixer Error: " + std::string(Mix_GetError()));
  }

  sound_t sound;
  sound._ptr = std::make_shared<sdl_sound_wrapper_t>(tmp);

  return sound;
}


music_t pixello::load_music(const std::string& music_path) const
{
  auto tmp = Mix_LoadMUS(music_path.c_str());

  if (!tmp) {
    throw load_exceptions("Failed to load music! " + music_path +
                          ". SDL_mixer Error: " + std::string(Mix_GetError()));
  }

  music_t music;
  music._ptr = std::make_shared<sdl_sound_wrapper_t>(tmp);

  return music;
}


bool pixello::is_mouse_in(const rect_t& rect) const
{
  return ((_mouse_state.x >= rect.x) && (_mouse_state.x < (rect.x + rect.w)) &&
          (_mouse_state.y >= rect.y) && (_mouse_state.y < (rect.y + rect.h)))
             ? true
             : false;
}


void pixello::draw_circle(const int32_t x,
                          const int32_t y,
                          const int32_t r,
                          const pixel_t& color) const
{
  filledCircleRGBA(_renderer, x, y, r, color.r, color.g, color.b, color.a);
}


button_t pixello::create_button(const rect_t& viewport,
                                const rect_t& button_rect,
                                const pixel_t button_color,
                                const std::string& button_text,
                                const std::function<void()> on_click) const
{
  button_t button;
  button.hover = false;
  button.color = button_color;
  button.rect = button_rect;
  button.on_click = on_click;
  button.text_texture = create_text(button_text, 0x000000FF);

  button.with_viewport.w = button_rect.w;
  button.with_viewport.h = button_rect.h;
  button.with_viewport.x = button_rect.x + viewport.x;
  button.with_viewport.y = button_rect.y + viewport.y;

  button.text_rect.w = button.text_texture.w;
  button.text_rect.h = button.text_texture.h;
  button.text_rect.x =
      button_rect.x + ((button_rect.w - button.text_texture.w) / 2);
  button.text_rect.y =
      button_rect.y + ((button_rect.h - button.text_texture.h) / 2);

  return button;
}


void pixello::draw_button(const button_t& b) const
{
  draw_rect(b.rect, b.color);
  if (b.hover) { draw_rect(b.rect, 0xAAAAAA33); }

  draw_texture(b.text_texture, b.text_rect);
}


void pixello::on_click_button(button_t& b) const
{
  const auto mouse = mouse_state();

  if (is_mouse_in(b.with_viewport)) {
    b.hover = true;
    if (mouse.left_button.click) { b.on_click(); }
  } else {
    b.hover = false;
  }
}


void pixello::show_mouse(const bool show) const
{
  const int t = show ? SDL_ENABLE : SDL_DISABLE;
  SDL_ShowCursor(t);
}


void pixello::mouse_set_FPS_mode(const bool enable) const
{
  const SDL_bool b = enable ? SDL_TRUE : SDL_FALSE;
  SDL_SetRelativeMouseMode(b);
}


bool pixello::is_key_pressed(const keycap_t k) const
{
  const int index = static_cast<int>(k);
  const SDL_Scancode code = KEYCAP_MAP[index];
  const Uint8* current_key_states = SDL_GetKeyboardState(NULL);

  const bool result = static_cast<bool>(current_key_states[code]);

  return result;
}