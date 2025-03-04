#pragma once

#include <inttypes.h>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

/*******************************************************************************
 * VALUES
 ******************************************************************************/
enum class keycap_t
{
  ESC,
  Q,
  W,
  E,
  R,
  T,
  Y,
  U,
  I,
  O,
  P,
  A,
  S,
  D,
  F,
  G,
  H,
  J,
  K,
  L,
  Z,
  X,
  C,
  V,
  B,
  N,
  M,
  N1,
  N2,
  N3,
  N4,
  N5,
  N6,
  N7,
  N8,
  N9,
  N0,
  LEFT,
  RIGHT,
  UP,
  DOWN,
  ENTER,
  SPACE,
  LSHIFT
};

/*******************************************************************************
 * EXCEPTIONS
 ******************************************************************************/
class pixello_exception : public std::runtime_error
{
public:
  pixello_exception(std::string msg) : std::runtime_error(std::move(msg)) {}
};

class init_exception : public pixello_exception
{
public:
  init_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};

class load_exceptions : public pixello_exception
{
public:
  load_exceptions(std::string msg) : pixello_exception(std::move(msg)) {}
};

class runtime_exception : public pixello_exception
{
public:
  runtime_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};

class input_exception : public pixello_exception
{
public:
  input_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};


// Forward declaration to avoid include sdl in this header
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct _TTF_Font;
struct _Mix_Music;
struct Mix_Chunk;

/*******************************************************************************
 * MACRO
 ******************************************************************************/
#define STR(_N_) std::to_string(_N_)


/*******************************************************************************
 * STRUCTS
 ******************************************************************************/
struct point_t
{
  int x;
  int y;
};

inline bool operator==(const point_t& lhs, const point_t& rhs)
{
  bool result = (lhs.x == rhs.x && lhs.y == rhs.y);
  return result;
}

inline bool operator!=(const point_t& lhs, const point_t& rhs)
{
  bool result = (lhs.x != rhs.x || lhs.y != rhs.y);
  return result;
}


struct rect_t
{
  int x, y;
  int w, h;
};

inline bool operator==(const rect_t& lhs, const rect_t& rhs)
{
  bool result =
      (lhs.x == rhs.x && lhs.y == rhs.y && lhs.w == rhs.w && lhs.h == rhs.h);
  return result;
}

inline bool operator!=(const rect_t& lhs, const rect_t& rhs)
{
  bool result =
      (lhs.x != rhs.x || lhs.y != rhs.y || lhs.w != rhs.w || lhs.h != rhs.h);
  return result;
}


struct pixel_t
{
  union
  {
    uint32_t n;  // 0xRRGGGBBBAAA
    struct
    {
      uint8_t a;
      uint8_t b;
      uint8_t g;
      uint8_t r;
    };
  };

  pixel_t() : n(0x000000FF) {}
  pixel_t(uint32_t color) : n(color) {}
  pixel_t(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
      : a(alpha), b(blue), g(green), r(red)
  {}
};

inline bool operator==(const pixel_t& lhs, const pixel_t& rhs)
{
  bool result = (lhs.n == rhs.n);
  return result;
}

inline bool operator!=(const pixel_t& lhs, const pixel_t& rhs)
{
  bool result = (lhs.n != rhs.n);
  return result;
}


struct sdl_texture_wrapper_t
{
  SDL_Texture* ptr = NULL;

  sdl_texture_wrapper_t() = delete;

  sdl_texture_wrapper_t(SDL_Texture* p) : ptr(p) {}
  ~sdl_texture_wrapper_t();
};

struct texture_t
{
  int32_t w = 0;
  int32_t h = 0;

  std::shared_ptr<sdl_texture_wrapper_t> _ptr;

  texture_t() {}
  inline SDL_Texture* pointer() const { return _ptr.get()->ptr; }
};

struct sdl_sound_wrapper_t
{
  _Mix_Music* music_ptr = NULL;
  Mix_Chunk* chunk_ptr = NULL;

  sdl_sound_wrapper_t() = delete;
  sdl_sound_wrapper_t(_Mix_Music* p) : music_ptr(p), chunk_ptr(NULL) {}
  sdl_sound_wrapper_t(Mix_Chunk* p) : music_ptr(NULL), chunk_ptr(p) {}
  ~sdl_sound_wrapper_t();
};

struct sound_t
{
  std::shared_ptr<sdl_sound_wrapper_t> _ptr;

  sound_t() {}
  inline Mix_Chunk* pointer() const { return _ptr.get()->chunk_ptr; }
};

struct music_t
{
  std::shared_ptr<sdl_sound_wrapper_t> _ptr;

  music_t() {}
  inline _Mix_Music* pointer() const { return _ptr.get()->music_ptr; }
};

struct config_t
{
  int32_t pixel_size;

  uint32_t window_w;
  uint32_t window_h;

  const std::string name;

  float target_fps;
  float target_s_per_frame;

  int32_t width_in_pixels;
  int32_t height_in_pixels;

  std::string font_path;
  int32_t font_size;

  pixel_t background_color;

  config_t(uint32_t ps,
           uint32_t ww,
           uint32_t wh,
           std ::string wname,
           float Hz,
           std::string font,
           int32_t font_s)
      : pixel_size(ps),
        window_w(ww),
        window_h(wh),
        name(std::move(wname)),
        target_fps(Hz),
        target_s_per_frame(1 / target_fps),
        width_in_pixels(window_w / pixel_size),
        height_in_pixels(window_h / pixel_size),
        font_path(std::move(font)),
        font_size(font_s)
  {
    if (pixel_size < 1) {
      pixel_size = 1;
      background_color = 0x000000FF;
    }
  }
};

struct button_key_t
{
  enum state_t
  {
    DOWN,
    UP
  };

  state_t state = UP;
  bool click = false;
  bool double_click = false;
};

struct mouse_t
{
  int x = 0;
  int y = 0;
  int relative_x;
  int relative_y;
  bool did_mouse_moved = false;

  bool left_button_pressed = false;
  bool central_button_pressed = false;
  bool right_button_pressed = false;

  button_key_t left_button;
  button_key_t central_button;
  button_key_t right_button;
};


struct key_input_t
{
  bool pressed;

  key_input_t() : pressed(false) {}
};


struct button_t
{
  rect_t rect;
  rect_t with_viewport;
  pixel_t color;
  texture_t text_texture;
  rect_t text_rect;
  std::function<void()> on_click;
  bool hover;
};


struct simple_timer
{
public:
  simple_timer();

  // The various clock actions
  void start();
  void stop();
  void pause();
  void unpause();
  void restart();

  // get timer's time
  uint64_t get_ticks() const;

  // Checks the status of the timer
  inline bool is_started() const { return started; }
  inline bool is_paused() const { return paused && started; }

private:
  // The clock time when the timer started in ms
  uint64_t start_ticks;

  // The ticks stored when the timer was paused
  uint64_t paused_ticks;

  // The timer status
  bool paused;
  bool started;
};

/*******************************************************************************
 * PIXELLO CLASS
 ******************************************************************************/
class pixello
{
private:
  uint32_t _FPS = 0;
  uint64_t dt;  // time form the last frame
  mouse_t _mouse_state;
  bool _running = true;

  SDL_Window* _window = NULL;
  SDL_Renderer* _renderer = NULL;
  _TTF_Font* _font = NULL;

  config_t _config;

  void* _external_data = nullptr;

  void init();

protected:
  // Have to Override this
  virtual void on_update(void* external_data) = 0;
  virtual void on_init(void* external_data) = 0;

  // You can override this if you want
  virtual void log(const std::string& msg);

public:
  pixello(uint32_t ww,
          uint32_t wh,
          std ::string wname,
          float Hz,
          std::string font,
          int32_t font_s,
          void* external_data = nullptr)
      : _config({1, ww, wh, wname, Hz, font, font_s}),
        _external_data(external_data)
  {}

  pixello(uint32_t pixel_size,
          uint32_t ww,
          uint32_t wh,
          std ::string wname,
          float Hz,
          std::string font,
          int32_t font_s,
          void* external_data = nullptr)
      : _config({pixel_size, ww, wh, wname, Hz, font, font_s}),
        _external_data(external_data)
  {}

  ~pixello();

  bool run();

  // Routines
  void draw_pixel(const int32_t x, const int32_t y, const pixel_t& p) const;
  void draw_rect(const rect_t& rect, const pixel_t& p) const;
  void draw_line(const point_t& a, const point_t& b, const pixel_t& p) const;
  void draw_dot(const point_t& a, const pixel_t& p) const;
  void draw_rect_outline(const rect_t& rect, const pixel_t& p) const;

  void draw_texture(const texture_t& t, const int32_t x, const int32_t y) const;
  void draw_texture(const texture_t& t, const rect_t& rect) const;
  void draw_texture(const texture_t& t,
                    const rect_t& rect,
                    const rect_t& clip) const;

  void draw_circle(const int32_t x,
                   const int32_t y,
                   const int32_t r,
                   const pixel_t& color) const;

  void play_music(const music_t& music) const;
  void play_sound(const sound_t& sound) const;
  float set_sound_volume(const sound_t& sound, const float volume) const;
  void pause_sounds() const;
  void resume_sounds() const;
  void pause_music() const;
  void resume_music() const;
  void stop_music() const;
  void set_master_volume(const float value) const;
  void set_music_volume(const float value) const;
  void set_sound_volume(const float value) const;


  texture_t load_image(const std::string& img_path) const;
  texture_t create_text(const std::string& text, const pixel_t& color) const;
  sound_t load_sound(const std::string& sound_path) const;
  music_t load_music(const std::string& music_path) const;


  inline int32_t width_in_pixels() const { return _config.width_in_pixels; }
  inline int32_t height_in_pixels() const { return _config.height_in_pixels; }
  inline int32_t width() const { return _config.window_w; }
  inline int32_t height() const { return _config.window_h; }
  inline mouse_t mouse_state() const { return _mouse_state; }
  bool is_key_pressed(const keycap_t k) const;

  inline void mouse_reset_clicks()
  {
    _mouse_state.left_button.click = false;
    _mouse_state.left_button.double_click = false;
    _mouse_state.right_button.click = false;
    _mouse_state.right_button.double_click = false;
    _mouse_state.central_button.click = false;
    _mouse_state.central_button.double_click = false;
  }


  inline uint32_t FPS() const { return _FPS; }
  inline uint64_t delta_time() const { return dt; }
  inline void stop() { _running = false; }

  void set_current_viewport(const rect_t& rect,
                            const pixel_t& color = {0x555555FF});

  bool is_mouse_in(const rect_t& rect) const;
  void show_mouse(const bool show) const;
  void mouse_set_FPS_mode(const bool enable) const;

  button_t create_button(const rect_t& viewport,
                         const rect_t& button_rect,
                         const pixel_t button_color,
                         const std::string& button_text,
                         const std::function<void()> on_click) const;

  void draw_button(const button_t& b) const;
  void on_click_button(button_t& b) const;
};
