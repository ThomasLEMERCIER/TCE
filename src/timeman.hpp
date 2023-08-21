#pragma once

#include "definition.hpp"

#include <chrono>

using TimePoint = std::chrono::milliseconds::rep;

TimePoint get_time_ms();

struct SearchLimits {
  TimePoint remaining_times[COLOR_NB];
  TimePoint incs[COLOR_NB];
  int movestogo;
  int depth;
  int nodes;
  TimePoint movetime;
  bool infinite;

  bool use_time_management() const {
    return (!infinite) && (remaining_times[Color::WHITE] || remaining_times[Color::BLACK] || incs[Color::WHITE] || incs[Color::BLACK] || movetime);
  }
};

class TimeManager {
public:
  TimeManager() : start_time(0), stop_time(0) {}
  void init(const SearchLimits& limits, Color us, TimePoint st);

  bool is_time_up() const {
    return get_time_ms() >= stop_time;
  }

  TimePoint start_time;
  TimePoint stop_time;  
};

extern TimeManager tm;
