// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "tx.h"
#include "realtime.h"
#include "parser.hpp"

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

#include <iostream>
#include <chrono>
#include <sstream>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>; // not needed as of C++20


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

void Transmitter::send_loop()
{
  using namespace std::chrono_literals;
  set_priority(70, SCHED_RR);

  while(true)
  {
    while(_queue.size_approx() > 100)
    {
      std::stringstream s;
      for(int i=0; i < 100; ++i)
      {
        queue_t::value_type item;
        _queue.try_dequeue(item);
        const auto [timestamp, datagram] = item;
        if(datagram)
        {
          s << "D" << datagram << "\n";
          do_statistics(datagram);
        }
        do_spi_statistics(timestamp);
      }
      s << "S" << get_statistics_line() << "\n";

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

void Transmitter::recv_loop()
{
  std::array<char, 2048> buffer;


  while(true)
  {
    const auto nbytes = nn_recv(_socket, buffer.data(), buffer.size(), 0);
    if(nbytes < 0)
    {
    }
    else if(nbytes > 0)
    {
      std::string_view msg(buffer.data(), nbytes);
      const auto command = parse(msg);
      std::visit(overloaded {
          [](tune_cmd_t tune) { std::cout << "tune: " << tune.node << "->" << tune.frequency << "\n"; },
          [](auto arg) { }
        }, command);
    }
    else
    {
      std::cerr << "spurious recv!\n";
    }
  }
}

void Transmitter::start()
{
  _writer_thread = std::thread([this]() { this->send_loop(); });
  _reader_thread = std::thread([this]() { this->recv_loop(); });
}


bool Transmitter::push(const SPIDatagram& datagram)
{
  return _queue.try_enqueue({std::chrono::steady_clock::now(), datagram});
}

void Transmitter::do_statistics(const SPIDatagram& item)
{
  using namespace std::chrono_literals;

  int64_t new_timestamp = item.payload[1];

  if(_stats.last_timestamp != -1)
  {
    _stats.max_diff = std::max(
      _stats.max_diff,
      (new_timestamp + 4294967295 - _stats.last_timestamp) % 4294967296
      );
  }
  _stats.last_timestamp = new_timestamp;
  if(std::chrono::steady_clock::now() -_stats.start > 10s)
  {
    _stats.max_diff = _stats.spi_max_diff = 0;
    _stats.start = std::chrono::steady_clock::now();
  }
}

std::string Transmitter::get_statistics_line() const
{
  std::stringstream s;
  s << ":" << _stats.max_diff << ":" << _stats.spi_max_diff;
  return s.str();
}


void Transmitter::do_spi_statistics(const ts_t& spi_ts)
{
  using namespace std::chrono_literals;

  _stats.spi_max_diff = (spi_ts - _stats.spi_last_timestamp) / 1us;
  _stats.spi_last_timestamp = spi_ts;
}
