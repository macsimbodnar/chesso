#include "pixello.hpp"
#include "SDL.h"
#include "SDL_render.h"

#define CHECK(_RES_, _MSG_)                                                    \
  do {                                                                         \
    int _res = (_RES_);                                                        \
    if (_res) {                                                                \
      log(_MSG_);                                                              \
      return _res;                                                             \
    }                                                                          \
  } while (0)

pixello::pixello(config_t configuration) : config(std::move(configuration)) {}

bool pixello::run() {

  const float target_s_per_frame = 1 / config.target_fps;

  int res;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    log("SDL could not initialize! SDL_Error: %s\n" +
        std::string(SDL_GetError()));
    return false;
  }

  // Create window
  window =
      SDL_CreateWindow(config.name.c_str(), config.window_x, config.window_y,
                       config.window_w, config.window_h, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    log("Window could not be created! SDL_Error: %s\n" +
        std::string(SDL_GetError()));
    return false;
  }

  // SDL_RENDERER_PRESENTVSYNC for vsync
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL) {
    log("Renderer could not be created! SDL_Error: %s\n" +
        std::string(SDL_GetError()));
    return false;
  }

  res = SDL_RenderSetLogicalSize(renderer, config.pixels_w, config.pixels_h);
  CHECK(res, "Failed to set renderer logical size");

  SDL_Event event;
  bool running = true;
  uint32_t FPS_counter = 0;
  uint64_t FPS_last_check = SDL_GetPerformanceCounter();
  /*************************************************************
   *                          MAIN LOOP                        *
   *************************************************************/
  while (running) {
    const uint64_t start = SDL_GetPerformanceCounter();

    // POLL EVENTS
    while (SDL_PollEvent(&event)) {
      if ((SDL_QUIT == event.type) ||
          (SDL_KEYDOWN == event.type &&
           SDL_SCANCODE_ESCAPE == event.key.keysym.scancode)) {
        running = false;
        break;
      }
    }

    // USER UPDATE
    on_update();

    // DRAW
    SDL_RenderPresent(renderer);

    // PERFORMANCE
    const uint64_t end = SDL_GetPerformanceCounter();
    const float freq = static_cast<float>(SDL_GetPerformanceFrequency());
    const float elapsed_s = (end - start) / freq;
    const float sleep_for_s = target_s_per_frame - elapsed_s;
    const uint64_t sleep_for_ms = static_cast<uint64_t>(sleep_for_s * 1000.0f);

    // log("Sleep for: " + STR(sleep_for_ms) + "ms");

    if (sleep_for_s > 0) {
      SDL_Delay(sleep_for_ms);
    }
    //  else {
    //   log("Frame too slow");
    // }

    const float FPS_elapsed_s = (end - FPS_last_check) / freq;
    if (FPS_elapsed_s > 1.0f) {
      FPS_last_check = end;
      // log("FPS: " + STR(FPS_counter));
      FPS_counter = 0;
    } else {
      ++FPS_counter;
    }
  }

  return true;
}

pixello::~pixello() {
  // Destroy window
  if (window) {
    SDL_DestroyWindow(window);
  }

  // Quit SDL subsystems
  SDL_Quit();
}

// Routines
void pixello::draw(uint32_t x, uint32_t y, pixel_t p) {
  SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, p.a);
  SDL_RenderDrawPoint(renderer, x, y);
}

void pixello::clear(pixel_t p) {
  SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, p.a);
  SDL_RenderClear(renderer);
}