// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#pragma once

#include "datagram.h"
#include "parser.hpp"

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
  ts_t spi_last_timestamp;
  int64_t spi_max_diff;
};

class Transmitter {

public:
  Transmitter(const std::string& uri, SPIDatagram& configuration, int thinning);

  void start();
  bool push(const SPIDatagram&);

private:
  using output_queue_t = moodycamel::ReaderWriterQueue<std::tuple<ts_t, SPIDatagram>>;
  using input_queue_t = moodycamel::ReaderWriterQueue<parse_result_t>;
  void send_loop();
  void recv_loop();
  void do_statistics(const SPIDatagram&);
  void do_spi_statistics(const ts_t&);
  std::string get_statistics_line() const;

  int _thinning;
  int _socket;
  int _endpoint;
  SPIDatagram& _configuration;

  output_queue_t _queue;
  input_queue_t _input_queue;

  std::thread _writer_thread;
  std::thread _reader_thread;

  TXStatistics _stats;
};
