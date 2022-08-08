#include "pixello.hpp"
#include <iostream>


class pixel : public pixello {
public:
  pixel() : pixello({8, 8, 800, 800, 100, 100, "Test pixello", 60}) {}

private:
  void log(const std::string &msg) override { std::cout << msg << std::endl; }

  void on_update() override {
    // clear({0xFF000000});

    pixel_t p;
    p.a = 255;

    bool black = false;
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {

        if (black) {
          p.r = 89;
          p.g = 44;
          p.b = 4;
        } else {
          p.r = 232;
          p.g = 235;
          p.b = 239;
        }

        draw(x, y, p); // top left red
        black = !black;
      }

      black = !black;
    }
  }
};

int main() {

  pixel p;

  p.run();

  return 0;
}