// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#pragma once

#include "datagram.h"
#include "timetracker.hpp"
#include "readerwriterqueue.h"
#include "parser.hpp"

#include <thread>
#include <list>
#include <functional>


class Hub {
public:

  using item_t = std::tuple<time_point_t, SPIDatagram>;

  Hub();
  void start();
  // invoked from the SPI thread
  bool push(const SPIDatagram& datagram);

  template<typename T>
  void register_listener(T& listener)
  {
    _listeners.push_back([&listener](const item_t& item) { listener.consume(item) ;});
  }

private:
  using output_queue_t = moodycamel::ReaderWriterQueue<item_t>;

  void send_loop();
  void recv_loop();
  void propagate(const item_t&);

  output_queue_t _queue;

  std::thread _writer_thread;

  std::list<std::function<void(const item_t&)>> _listeners;
};
