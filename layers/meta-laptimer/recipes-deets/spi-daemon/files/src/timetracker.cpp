// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "timetracker.hpp"

const int64_t MAX_U32 = 4294967296;

PropellerTimeTracker::PropellerTimeTracker(time_point_t now)
  : _start(now)
  , _cpu_freq(96000000)
  , _timestamp_accu(0)
{

}


time_point_t PropellerTimeTracker::feed(uint32_t timestamp_unsigned)
{
  using namespace std::chrono_literals;

  int64_t timestamp = timestamp_unsigned;
  if(_last_timestamp)
  {
    int64_t diff = (timestamp + MAX_U32 - *_last_timestamp) % MAX_U32;
    _timestamp_accu += diff;
  }
  _last_timestamp = timestamp;
  const auto microseconds = static_cast<long long>(static_cast<double>(_timestamp_accu) / _cpu_freq);
  const auto us = (1us * microseconds);
  return _start + us;
  }
