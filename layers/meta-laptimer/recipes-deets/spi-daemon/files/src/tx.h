// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#pragma once

#include "datagram.h"

#include "readerwriterqueue.h"

#include <thread>

class Transmitter {

public:
  Transmitter(const std::string& uri);
  void start();
  void push(const SPIDatagram&);

private:
  void run();

  int _socket;
  int _endpoint;

  moodycamel::ReaderWriterQueue<SPIDatagram> _queue;
  std::thread _thread;
};
