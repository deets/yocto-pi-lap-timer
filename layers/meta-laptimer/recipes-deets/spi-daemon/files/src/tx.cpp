// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "tx.hpp"

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

#include <iostream>
#include <chrono>
#include <sstream>

namespace {

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>; // not needed as of C++20


const auto INPUT_QUEUE_SIZE = 100;
const auto SPI_STATISTICS_INTERVAL = 4000; // divide by samplerate

} // end ns anonymous

Transmitter::Transmitter(const std::string& uri, SPIDatagram& configuration, int thinning)
  : _configuration(configuration)
  , _input_queue(INPUT_QUEUE_SIZE)
  , _thinning(thinning)
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

void Transmitter::consume(const Hub::item_t& item)
{
  using namespace std::chrono_literals;

  std::stringstream s;

  ++_total_counter;

  const auto& [timestamp, datagram] = item;
  if(_total_counter % _thinning == 0)
  {
    s << "D" << datagram << "\n";
    ++_sent_counter;
  }
  do_statistics(datagram);
  do_spi_statistics(timestamp);

  if((_sent_counter % (SPI_STATISTICS_INTERVAL / _thinning)) == 0)
  {
    s << "S" << get_statistics_line() << "\n";
  }
  const auto content = s.str();
  if(content.size())
  {
    const auto err = nn_send(_socket, content.data(), content.size(), NN_DONTWAIT);
    if(err == -1 && errno != EAGAIN)
    {
      std::cerr << "nn_send error: " << errno << "\n";
      }
  }

  for(auto i=0; i < _input_queue.size_approx(); ++i)
  {
    input_queue_t::value_type command;
    _input_queue.try_dequeue(command);
    std::visit(overloaded {
        [this](tune_cmd_t tune) {
          _configuration.payload[tune.node] = tune.frequency;
        },
        [](auto arg) { }
      }, command);
  }
}

void Transmitter::start()
{
  _reader_thread = std::thread([this]() { this->recv_loop(); });
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


void Transmitter::do_spi_statistics(const time_point_t& spi_ts)
{
  using namespace std::chrono_literals;

  _stats.spi_max_diff = (spi_ts - _stats.spi_last_timestamp) / 1us;
  _stats.spi_last_timestamp = spi_ts;
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
      if(!std::holds_alternative<no_cmd_t>(command))
      {
        const auto result = _input_queue.try_enqueue(command);
        // assert(result);
        // std::cout << "got command " << msg << "q size:" << _input_queue.size_approx() << "\n";
      }
    }
    else
    {
      std::cerr << "spurious recv!\n";
    }
  }
}
