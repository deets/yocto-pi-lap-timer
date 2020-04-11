// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "hub.hpp"

#include "realtime.h"

namespace {


} // end ns anonymous

Hub::Hub()
  : _queue(QUEUE_SIZE)
{}

void Hub::start()
{
  _writer_thread = std::thread([this]() { this->send_loop(); });
}


void Hub::send_loop()
{
  using namespace std::chrono_literals;
  set_priority(70, SCHED_RR);

  while(true)
  {
    const auto qsize = _queue.size_approx();
    for(int i=0; i < qsize; ++i)
    {
      output_queue_t::value_type item;
      _queue.try_dequeue(item);
      const auto& [timestamp, datagram] = item;
      if(datagram)
      {
        propagate(item);
      }
    }
    std::this_thread::sleep_for(100ms);
  }
}

void Hub::propagate(const item_t& item)
{
  for(auto& listener : _listeners)
  {
    listener(item);
  }
}

bool Hub::push(const SPIDatagram& datagram)
{
  const auto res = _queue.try_enqueue({std::chrono::steady_clock::now(), datagram});
  return res;
}
