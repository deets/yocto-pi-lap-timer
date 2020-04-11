// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include <cstdint>
#include <chrono>
#include <optional>

using time_point_t = std::chrono::steady_clock::time_point;

class PropellerTimeTracker
{
public:
  PropellerTimeTracker(time_point_t);

  time_point_t feed(uint32_t timestamp);

private:
  time_point_t _start;
  double _cpu_freq;
  int64_t _timestamp_accu;
  std::optional<int64_t> _last_timestamp;
};
