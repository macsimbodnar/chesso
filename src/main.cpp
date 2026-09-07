#include <iostream>
#include <string>
#include "log.hpp"
#include "uci.hpp"


int main(int argc, char** argv)
{
  uci_init();

  // OpenBench runs a public engine as `./binary bench`: it must print a final
  // node count and a nodes-per-second count and then exit. The command itself
  // is the same one the UCI loop dispatches, so the stdin form
  // `printf 'bench\nquit\n' | chesso` prints the identical signature. S189.
  if (argc > 1 && std::string(argv[1]) == "bench") {
    std::string line = "bench";

    for (int i = 2; i < argc; i++) {
      line += " ";
      line += argv[i];
    }

    uci_process_line(line);
    uci_shutdown();

    return 0;
  }

  while (uci_is_running()) {
    std::string input;

    // On EOF getline leaves input empty and keeps failing. Without this the
    // loop would spin forever on a closed pipe.
    if (!std::getline(std::cin, input)) {
      LOG_I << "stdin closed" << END_I;
      break;
    }

    uci_process_line(input);
  }

  uci_shutdown();

  return 0;
}
