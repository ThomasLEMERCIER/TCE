#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "definition.hpp"

#include <chrono>

typedef std::chrono::milliseconds::rep TimePoint;

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
    return (!infinite) && (remaining_times[WHITE] || remaining_times[BLACK] || incs[WHITE] || incs[BLACK]);
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

#endif
