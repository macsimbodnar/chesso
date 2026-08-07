#include <iostream>
#include <string>
#include "log.hpp"
#include "uci.hpp"


int main()
{
  uci_init();

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
