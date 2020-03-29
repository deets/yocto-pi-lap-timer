// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "tx.h"

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

#include <iostream>
#include <chrono>
#include <sstream>

namespace {

const int QUEUE_SIZE = 4000;

} // end ns anonymous

Transmitter::Transmitter(const std::string& uri)
  : _queue(QUEUE_SIZE)
{
  _socket = nn_socket(AF_SP, NN_PAIR);
  assert(_socket >= 0);
  _endpoint = nn_bind (_socket, uri.c_str());
  if(_endpoint == -1)
  {
    std::cerr << "nn_bind error: " << errno << "\n";
    switch(errno)
    {
    case EBADF:
      std::cerr << "errno: EBADF\n";
      break;
    case EMFILE:
      std::cerr << "errno: EMFILE\n";
      break;
    case EINVAL:
      std::cerr << "errno: EINVAL\n";
      break;
    case ENAMETOOLONG:
      std::cerr << "errno: ENAMETOOLONG\n";
      break;
    case EPROTONOSUPPORT:
      std::cerr << "errno: EPROTONOSUPPORT\n";
      break;
    case EADDRNOTAVAIL:
      std::cerr << "errno: EADDRNOTAVAIL\n";
      break;
    case ENODEV:
      std::cerr << "errno: ENODEV\n";
      break;
    case EADDRINUSE:
      std::cerr << "errno: EADDRINUSE\n";
      break;
    case ETERM:
      std::cerr << "errno: ETERM\n";
      break;
    }
    assert(false);
  }
}

void Transmitter::run()
{
  using namespace std::chrono_literals;
  while(true)
  {
    if(_queue.size_approx() > QUEUE_SIZE / 2)
    {
      std::stringstream s;
      for(int i=0; i < _queue.size_approx(); ++i)
      {
        SPIDatagram item;
        _queue.try_dequeue(item);
        if(item)
        {
          s << item << "\n";
        }
      }
      const auto content = s.str();
      const auto err = nn_send(_socket, content.data(), content.size(), NN_DONTWAIT);
      if(err == -1 && errno != EAGAIN)
      {
        std::cerr << "nn_send error: " << errno << "\n";
      }
    }
    std::this_thread::sleep_for(100ms);
  }
}

void Transmitter::start()
{
  _thread = std::thread([this]() { this->run(); });
}


void Transmitter::push(const SPIDatagram& datagram)
{
  _queue.enqueue(datagram);
}
