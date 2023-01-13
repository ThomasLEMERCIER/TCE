#include "utils.hpp"

#include <sysinfoapi.h>

int get_time_ms() {
  return GetTickCount();
}
