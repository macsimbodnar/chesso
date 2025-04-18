#pragma once
#include <chrono>
#include <iostream>


// #include <fstream>
// static std::ofstream log_file("chesso_engine.log", std::ios::app);


#define LOG_I std::clog  // Start log
#define END_I std::endl  // End log

#define LOG_S LOG_I << "\033[92m"  // Success green log
#define END_S "\033[37m" << END_I  // End success green log

#define LOG_W LOG_I << "\033[33m"  // Warning orange log
#define END_W "\033[37m" << END_I  // End warning orange log

#define LOG_E LOG_I << "\033[31m"  // Error red log
#define END_E "\033[37m" << END_I  // End Error red log


class stopwatch_t
{
private:
  std::chrono::steady_clock::time_point begin;
  std::chrono::nanoseconds tot = std::chrono::nanoseconds::zero();
  bool running = false;

  inline void start()
  {
    assert(!running);
    running = true;

    // Keep this as last instruction
    begin = std::chrono::steady_clock::now();
  }

  inline void stop()
  {
    // keep `now` as first instruction
    const auto now = std::chrono::steady_clock::now();

    assert(running);
    if (running) {
      tot += now - begin;
      begin = now;
      running = false;
    }
  }

  inline void reset()
  {
    running = false;
    tot = std::chrono::nanoseconds::zero();
  }

public:
  explicit stopwatch_t()
  {
    reset();
    start();
  }

  ~stopwatch_t()
  {
    if (running) {
      stop();


      const auto duration_ns =
          std::chrono::duration_cast<std::chrono::nanoseconds>(tot);

      const auto minutes =
          std::chrono::duration_cast<std::chrono::minutes>(duration_ns);
      const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
          duration_ns - minutes);
      const auto milliseconds =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              duration_ns - minutes - seconds);

      LOG_I << "Exec time: " << std::to_string(minutes.count()) << " min "
            << seconds.count() << " sec " << milliseconds.count() << " msec"
            << END_I;
    }
  }
};
