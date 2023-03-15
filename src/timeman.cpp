#include "timeman.hpp"

#include <chrono>

TimeManager tm;

TimePoint get_time_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void TimeManager::init(const SearchLimits& limits, Color us, TimePoint st) {
  start_time = st;

  TimePoint time_to_search;
  int moves_to_go = (limits.movestogo ? limits.movestogo : 40);

  if (limits.movetime > 0)    time_to_search = limits.movetime;
  else                        time_to_search = limits.times[us] / moves_to_go + limits.incs[us] - 50;

  stop_time = start_time + time_to_search;
}
