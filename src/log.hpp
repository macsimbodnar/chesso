#pragma once
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>


// #include <fstream>
// static std::ofstream log_file("chesso.log", std::ios::app);

#ifndef NDEBUG

#define LOG_I std::clog  // Start log
#define END_I "\n"       // End log

#define LOG_S LOG_I << "\033[92m"  // Success green log
#define END_S "\033[37m" << END_I  // End success green log

#define LOG_W LOG_I << "\033[33m"  // Warning orange log
#define END_W "\033[37m" << END_I  // End warning orange log

#define LOG_E LOG_I << "\033[31m"  // Error red log
#define END_E "\033[37m" << END_I  // End Error red log

#else

// clang-format off
#define LOG_I if (false) std::clog
#define END_I ""

#define LOG_S if (false) std::clog
#define END_S ""

#define LOG_W if (false) std::clog
#define END_W ""

#define LOG_E if (false) std::clog
#define END_E ""
// clang-format on

#endif

inline std::string to_hexstr(uint64_t value)
{
  std::ostringstream oss;
  oss << "0x" << std::hex << std::setw(16) << std::setfill('0') << value;
  return oss.str();
}


class stopwatch_t
{
private:
  std::chrono::steady_clock::time_point begin;
  std::chrono::nanoseconds tot = std::chrono::nanoseconds::zero();
  bool running = false;

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

  inline void start()
  {
    assert(!running);
    running = true;
    begin = std::chrono::steady_clock::now();
  }

  inline void stop()
  {
    if (running) {
      const auto now = std::chrono::steady_clock::now();
      tot += now - begin;
      begin = now;
      running = false;
    }
  }

  inline std::string duration_str()
  {
    if (running) { stop(); }

    std::stringstream ss;
    const auto duration_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(tot);

    const auto minutes =
        std::chrono::duration_cast<std::chrono::minutes>(duration_ns);
    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration_ns - minutes);
    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            duration_ns - minutes - seconds);

    ss << std::to_string(minutes.count()) << " min " << seconds.count()
       << " sec " << milliseconds.count() << " msec"
       << "\n";

    start();
    return ss.str();
  }
};
