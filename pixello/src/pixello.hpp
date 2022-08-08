#pragma once

#include <inttypes.h>
#include <string>

// Forward declaration to avoid include sdl in this header
struct SDL_Window;
struct SDL_Renderer;

#define STR(_N_) std::to_string(_N_)

class pixello {
public:
  struct pixel_t {
    union {
      uint32_t n = 0xFF000000;
      struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
      };
    };
  };

private:
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  struct config_t {
    uint32_t pixels_w;
    uint32_t pixels_h;

    uint32_t window_w;
    uint32_t window_h;

    uint32_t window_x;
    uint32_t window_y;

    const std::string name;

    float target_fps;

    config_t(uint32_t pw, uint32_t ph, uint32_t ww, uint32_t wh, uint32_t wx,
             uint32_t wy, std ::string wname, float Hz)
        : pixels_w(pw), pixels_h(ph), window_w(ww), window_h(wh), window_x(wx),
          window_y(wy), name(std::move(wname)), target_fps(Hz) {}
  };

  config_t config;

protected:
  virtual void log(const std::string &msg) = 0;
  virtual void on_update() = 0;

public:
  pixello(config_t configuration);
  ~pixello();

  bool run();

  // Routines
  void draw(uint32_t x, uint32_t y, pixel_t p);
  void clear(pixel_t p);
};
