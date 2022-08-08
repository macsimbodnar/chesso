#include "pixello.hpp"
#include <iostream>
#include <vector>

constexpr uint32_t w = 60;
constexpr uint32_t h = 40;

class pixel : public pixello {
public:
  pixel() : pixello({w, h, 600, 400, 100, 100, "Test pixello", 60}) {}

private:
  void log(const std::string &msg) override { std::cout << msg << std::endl; }

  void on_update() override {
    // clear({0xFF000000});

    for (uint32_t i = 0; i < 1000; i++) {
      const uint32_t x = (rand() % (w - 2)) + 1;
      const uint32_t y = (rand() % (h - 2)) + 1;

      pixel_t p;
      p.r = rand() % 255;
      p.g = rand() % 255;
      p.b = rand() % 255;
      p.a = 255;

      draw(x, y, p);
    }

    pixel_t p;
    p.a = 255;

    p.r = 255;
    p.g = 0;
    p.b = 0;
    draw(0, 0, p); // top left red

    p.r = 0;
    p.g = 255;
    p.b = 0;
    draw(w - 1, 0, p); // top tight green

    p.r = 0;
    p.g = 0;
    p.b = 255;
    draw(w - 1, h - 1, p); // bottom right blu

    p.r = 255;
    p.g = 0;
    p.b = 255;
    draw(0, h - 1, p); // bottom left purple
  }
};

int main() {
  pixel p;

  p.run();

  return 0;
}