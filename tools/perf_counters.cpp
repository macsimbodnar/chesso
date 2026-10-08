// S268. Runs a command and prints the user-space instructions and cycles it
// retired, from the kernel's own hardware counters through perf_event_open(2).
//
// It exists because `perf` is not installed on the workstation and installing
// it needs the owner, while the counters themselves need nothing:
// `kernel.perf_event_paranoid` is 2 there, which still lets a process count
// another process of its own in user space. A wall-clock timing of a speed
// change sits inside the machine's noise more often than not; an instruction
// count repeats to the last digits, and cycles carry what the instruction count
// cannot see (a load that misses). Linux only: the system call has no macOS
// equivalent, so tools/CMakeLists.txt does not build this there.
//
//   perf_counters [-o FILE] [--] COMMAND [ARGS...]
//
// The counters follow the whole process tree -- every thread and child the
// command starts -- from its exec to its exit, so the startup of this tool is
// not counted. They go to stderr, or appended to FILE, as one line:
//
//   perf_counters instructions <n> cycles <n> wall_ms <ms> scaled <0|1>
//
// `scaled` is 1 when the kernel had to multiplex a counter and the figure is an
// estimate; on the fixed counters these two use it should never be. The exit
// status is the command's own, or 128 plus the signal that ended it.
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{

struct reading_t
{
  uint64_t value;
  uint64_t time_enabled;
  uint64_t time_running;
};


int open_counter(pid_t pid, uint64_t config)
{
  perf_event_attr attr;
  std::memset(&attr, 0, sizeof(attr));

  attr.size = sizeof(attr);
  attr.type = PERF_TYPE_HARDWARE;
  attr.config = config;
  attr.read_format =
      PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
  // Off until the child execs, so the fork and the pipe wait below are not
  // counted; user space only, which is what paranoid level 2 permits.
  attr.disabled = 1;
  attr.enable_on_exec = 1;
  attr.inherit = 1;
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;

  return static_cast<int>(
      syscall(SYS_perf_event_open, &attr, pid, -1, -1, PERF_FLAG_FD_CLOEXEC));
}


bool read_counter(int fd, reading_t* out)
{ return read(fd, out, sizeof(*out)) == static_cast<ssize_t>(sizeof(*out)); }


// Multiplexed counters are scaled up to the time the event was enabled, as
// `perf stat` does, and flagged.
uint64_t scaled_value(const reading_t& r, bool* scaled)
{
  if (r.time_running == 0) { return 0; }
  if (r.time_running == r.time_enabled) { return r.value; }

  *scaled = true;
  return static_cast<uint64_t>(static_cast<double>(r.value) *
                               static_cast<double>(r.time_enabled) /
                               static_cast<double>(r.time_running));
}


int usage()
{
  std::fprintf(stderr,
               "usage: perf_counters [-o FILE] [--] COMMAND [ARGS...]\n");
  return 2;
}

}  // namespace


int main(int argc, char** argv)
{
  const char* out_path = nullptr;
  int first = 1;

  while (first < argc && argv[first][0] == '-') {
    if (std::strcmp(argv[first], "--") == 0) {
      first++;
      break;
    }

    if (std::strcmp(argv[first], "-o") == 0 && first + 1 < argc) {
      out_path = argv[first + 1];
      first += 2;
      continue;
    }

    return usage();
  }

  if (first >= argc) { return usage(); }

  // The child waits on this pipe until both counters are attached to it, so
  // nothing it executes can run uncounted.
  int gate[2];
  if (pipe(gate) != 0) {
    std::perror("perf_counters: pipe");
    return 125;
  }

  const pid_t child = fork();
  if (child < 0) {
    std::perror("perf_counters: fork");
    return 125;
  }

  if (child == 0) {
    close(gate[1]);
    char go = 0;
    if (read(gate[0], &go, 1) != 1) { _exit(125); }
    close(gate[0]);

    execvp(argv[first], argv + first);
    std::fprintf(stderr, "perf_counters: cannot run %s: %s\n", argv[first],
                 std::strerror(errno));
    _exit(127);
  }

  close(gate[0]);

  const int fd_instructions = open_counter(child, PERF_COUNT_HW_INSTRUCTIONS);
  const int fd_cycles = open_counter(child, PERF_COUNT_HW_CPU_CYCLES);

  if (fd_instructions < 0 || fd_cycles < 0) {
    std::fprintf(stderr,
                 "perf_counters: perf_event_open failed: %s "
                 "(kernel.perf_event_paranoid above 2?)\n",
                 std::strerror(errno));
    kill(child, SIGKILL);
    waitpid(child, nullptr, 0);
    return 125;
  }

  const auto start = std::chrono::steady_clock::now();
  const char go = 1;
  if (write(gate[1], &go, 1) != 1) {
    std::perror("perf_counters: write");
    kill(child, SIGKILL);
    waitpid(child, nullptr, 0);
    return 125;
  }
  close(gate[1]);

  int status = 0;
  while (waitpid(child, &status, 0) < 0) {
    if (errno != EINTR) {
      std::perror("perf_counters: waitpid");
      return 125;
    }
  }

  const auto stop = std::chrono::steady_clock::now();

  reading_t instructions{};
  reading_t cycles{};
  if (!read_counter(fd_instructions, &instructions) ||
      !read_counter(fd_cycles, &cycles)) {
    std::perror("perf_counters: read");
    return 125;
  }

  bool scaled = false;
  const uint64_t n_instructions = scaled_value(instructions, &scaled);
  const uint64_t n_cycles = scaled_value(cycles, &scaled);
  const double wall_ms =
      std::chrono::duration<double, std::milli>(stop - start).count();

  FILE* out = stderr;
  if (out_path != nullptr) {
    out = std::fopen(out_path, "a");
    if (out == nullptr) {
      std::perror("perf_counters: open output");
      return 125;
    }
  }

  std::fprintf(out,
               "perf_counters instructions %" PRIu64 " cycles %" PRIu64
               " wall_ms %.3f scaled %d\n",
               n_instructions, n_cycles, wall_ms, scaled ? 1 : 0);

  if (out != stderr) { std::fclose(out); }

  if (WIFEXITED(status)) { return WEXITSTATUS(status); }
  if (WIFSIGNALED(status)) { return 128 + WTERMSIG(status); }
  return 125;
}
