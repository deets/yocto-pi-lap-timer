// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#pragma once

#include "hub.hpp"

#include "readerwriterqueue.h"

#include <thread>
#include <chrono>
#include <tuple>

struct TXStatistics
{
  int64_t last_timestamp = -1;
  int64_t max_diff = 0;
  time_point_t start = std::chrono::steady_clock::now();
  time_point_t spi_last_timestamp;
  int64_t spi_max_diff;
};

class Transmitter {

public:
  Transmitter(const std::string& uri, SPIDatagram& configuration, int thinning);

  void start();
  void consume(const Hub::item_t& item);

private:
  using input_queue_t = moodycamel::ReaderWriterQueue<parse_result_t>;

  void recv_loop();
  void do_statistics(const SPIDatagram&);
  void do_spi_statistics(const time_point_t&);
  std::string get_statistics_line() const;

  int _thinning;
  int _socket;
  int _endpoint;
  SPIDatagram& _configuration;

  TXStatistics _stats;

  std::thread _reader_thread;
  input_queue_t _input_queue;

  uint64_t _total_counter = 0;
  uint64_t _sent_counter = 0;

};
