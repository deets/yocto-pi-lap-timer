// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#pragma once

#include "datagram.h"

#include "readerwriterqueue.h"

#include <thread>
#include <chrono>
#include <tuple>

using ts_t = decltype(std::chrono::steady_clock::now());

struct TXStatistics
{
  int64_t last_timestamp = -1;
  int64_t max_diff = 0;
  ts_t start = std::chrono::steady_clock::now();
};

class Transmitter {

public:
  Transmitter(const std::string& uri);
  void start();
  bool push(const SPIDatagram&);

private:
  using queue_t = moodycamel::ReaderWriterQueue<std::tuple<ts_t, SPIDatagram>>;
  void run();
  void do_statistics(const SPIDatagram&);
  void do_spi_statistics(const ts_t&);
  std::string get_statistics_line() const;

  int _socket;
  int _endpoint;

  queue_t _queue;
  std::thread _thread;

  TXStatistics _stats;
};
