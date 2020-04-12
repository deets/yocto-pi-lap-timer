
#include "rotorhazard.hpp"
#include "nanomsg-helper.hpp"
#include "realtime.h"

#include <nanomsg/nn.h>

#include <algorithm>

struct Settings settings;
struct State state;
struct History history;
struct LastPass lastPass;

#define ioBufferWriteRssi(buf, rssi) (ioBufferWrite8(buf, rssi))

namespace {

template<typename T>
void ioBufferWrite8(T& buf, uint8_t data)
{
  buf.push_back(data);
}

template<typename T>
void ioBufferWrite16(T& buf, uint16_t data)
{
  buf.push_back(data >> 8);
  buf.push_back(data);
}

template<typename T>
void ioBufferWriteExtremum(T& buf, Extremum *e, time_point_t now)
{
    ioBufferWriteRssi(buf, e->rssi);
    ioBufferWrite16(buf, uint16_t((now - e->firstTime) / 1ms));
    ioBufferWrite16(buf, uint16_t((now - e->firstTime - e->duration) / 1ms));
}

} // end ns anonymous

RotorHazardNode::RotorHazardNode(time_point_t last_loop_micros)
{
    rssiMedian.init();
    state.lastloopMicros = last_loop_micros;
}

bool RotorHazardNode::rssiStateValid()
{
    return state.nodeRssiNadir <= state.rssi && state.rssi <= state.nodeRssiPeak;
}

void RotorHazardNode::rssiStateReset()
{
    state.crossing = false;
    state.passPeak.rssi = 0;
    state.passRssiNadir = MAX_RSSI;
    state.nodeRssiPeak = 0;
    state.nodeRssiNadir = MAX_RSSI;
    history.hasPendingPeak = false;
    history.peakSend.rssi = 0;
    history.hasPendingNadir = false;
    history.nadirSend.rssi = MAX_RSSI;
}

void RotorHazardNode::bufferHistoricPeak(bool force)
{
    if (history.hasPendingPeak)
    {
      if (!isPeakValid(history.peakSend))
      {
        // no current peak to send so just overwrite
        history.peakSend = history.peak;
        history.hasPendingPeak = false;
      }
      else if (force)
      {
        // must do something
        if (history.peak.rssi > history.peakSend.rssi)
        {
          // prefer higher peak
          history.peakSend = history.peak;
        }
        else if (history.peak.rssi == history.peakSend.rssi)
        {
          // merge
          history.peakSend.duration = endTime(history.peak) - history.peakSend.firstTime;
        }
        history.hasPendingPeak = false;
      }
    }
}

void RotorHazardNode::bufferHistoricNadir(bool force)
{
    if (history.hasPendingNadir)
    {
      if (!isNadirValid(history.nadirSend))
      {
        // no current nadir to send so just overwrite
        history.nadirSend = history.nadir;
        history.hasPendingNadir = false;
      }
      else if (force)
      {
        // must do something
        if (history.nadir.rssi < history.nadirSend.rssi)
        {
          // prefer lower nadir
          history.nadirSend = history.nadir;
        }
        else if (history.nadir.rssi == history.nadirSend.rssi)
        {
          // merge
          history.nadirSend.duration = endTime(history.nadir) - history.nadirSend.firstTime;
        }
        history.hasPendingNadir = false;
      }
    }
}

void RotorHazardNode::initExtremum(Extremum *e)
{
    e->rssi = state.rssi;
    e->firstTime = state.rssiTimestamp;
    e->duration = {};
}

bool RotorHazardNode::rssiProcess(rssi_t rssi, time_point_t millis)
{
    rssiMedian.addValue(rssi);

    SmoothingTimestamps[SmoothingTimestampsIndex] = millis;
    SmoothingTimestampsIndex++;
    if (SmoothingTimestampsIndex >= SmoothingTimestampSize)
    {
        SmoothingTimestampsIndex = 0;
    }

    if (rssiMedian.isFilled() && state.activatedFlag)
    {  //don't start operations until after first WRITE_FREQUENCY command is received

        state.lastRssi = state.rssi;
        state.rssi = rssiMedian.getMedian();  // retrieve the median
        state.rssiTimestamp = SmoothingTimestamps[SmoothingTimestampsIndex];

        /*** update history ***/

        int rssiChange = state.rssi - state.lastRssi;
        if (rssiChange > 0)
        {  // RSSI is rising
            // must buffer latest peak to prevent losing it (overwriting any unsent peak)
            bufferHistoricPeak(true);

            initExtremum(&(history.peak));

            // if RSSI was falling or unchanged, but it's rising now, we found a nadir
            // copy the values to be sent in the next loop
            if (history.rssiChange <= 0)
            {  // was falling or unchanged
                // declare a new nadir
                history.hasPendingNadir = true;
            }

        }
        else if (rssiChange < 0)
        {  // RSSI is falling
            // must buffer latest nadir to prevent losing it (overwriting any unsent nadir)
            bufferHistoricNadir(true);

            // whenever history is falling, record the time and value as a nadir
            initExtremum(&(history.nadir));

            // if RSSI was rising or unchanged, but it's falling now, we found a peak
            // copy the values to be sent in the next loop
            if (history.rssiChange >= 0)
            {  // was rising or unchanged
                // declare a new peak
                history.hasPendingPeak = true;
            }

        }
        else
        {  // RSSI is equal
            if (state.rssi == history.peak.rssi)
            {  // is peak
              history.peak.duration = std::clamp(state.rssiTimestamp - history.peak.firstTime, {},
                        MAX_DURATION);
                if (history.peak.duration == MAX_DURATION)
                {
                    bufferHistoricPeak(true);
                    initExtremum(&(history.peak));
                }
            }
            else if (state.rssi == history.nadir.rssi)
            {  // is nadir
              history.nadir.duration = std::clamp(state.rssiTimestamp - history.nadir.firstTime, {},
                        MAX_DURATION);
                if (history.nadir.duration == MAX_DURATION)
                {
                    bufferHistoricNadir(true);
                    initExtremum(&(history.nadir));
                }
            }
        }

        // clamp to prevent overflow
        history.rssiChange = std::clamp(rssiChange, -127, 127);

        // try to buffer latest peak/nadir (don't overwrite any unsent peak/nadir)
        bufferHistoricPeak(false);
        bufferHistoricNadir(false);

        /*** node lifetime RSSI max/min ***/

        if (state.rssi > state.nodeRssiPeak)
        {
            state.nodeRssiPeak = state.rssi;
        }

        if (state.rssi < state.nodeRssiNadir)
        {
            state.nodeRssiNadir = state.rssi;
        }

        /*** crossing transition ***/

        if ((!state.crossing) && state.rssi >= settings.enterAtLevel)
        {
            state.crossing = true;  // quad is going through the gate (lap pass starting)
        }
        else if (state.crossing && state.rssi < settings.exitAtLevel)
        {
            // quad has left the gate
            rssiEndCrossing();
        }

        /*** pass processing **/

        if (state.crossing)
        {  //lap pass is in progress
            // Find the peak rssi and the time it occured during a crossing event
            if (state.rssi > state.passPeak.rssi)
            {
                // this is first time this peak RSSI value was seen, so save value and timestamp
                initExtremum(&(state.passPeak));
            }
            else if (state.rssi == state.passPeak.rssi)
            {
                // if at max peak for more than one iteration then track duration
                // so middle-timestamp value can be returned
              state.passPeak.duration = std::clamp(state.rssiTimestamp - state.passPeak.firstTime,
                                                   {}, MAX_DURATION);
            }
        }
        else
        {
            // track lowest rssi seen since end of last pass
          state.passRssiNadir = std::min<volatile rssi_t>(state.rssi, state.passRssiNadir);
        }
    }

    return state.crossing;
}

// Function called when crossing ends (by RSSI or I2C command)
void RotorHazardNode::rssiEndCrossing()
{
    // save values for lap pass
  lastPass.rssiPeak = state.passPeak.rssi;
  // lap timestamp is between first and last peak RSSI
  lastPass.timestamp = state.passPeak.firstTime + state.passPeak.duration / 2;
  lastPass.rssiNadir = state.passRssiNadir;
  lastPass.lap = lastPass.lap + 1;
  // reset lap-pass variables
  state.crossing = false;
  state.passPeak.rssi = 0;
  state.passRssiNadir = MAX_RSSI;
}

void RotorHazardNode::readLapStats(time_point_t now, std::vector<uint8_t>& buffer)
{
  ioBufferWrite8(buffer, lastPass.lap);
  ioBufferWrite16(buffer, uint16_t((now - lastPass.timestamp) / 1ms));  // ms since lap
  ioBufferWriteRssi(buffer, state.rssi);
  ioBufferWriteRssi(buffer, state.nodeRssiPeak);
  ioBufferWriteRssi(buffer, lastPass.rssiPeak);  // RSSI peak for last lap pass
  ioBufferWrite16(buffer, uint16_t(1000));
  uint8_t flags = state.crossing ? (uint8_t)1 : (uint8_t)0;  // 'crossing' status
  if (isPeakValid(history.peakSend)
      && (!isNadirValid(history.nadirSend)
          || (history.peakSend.firstTime < history.nadirSend.firstTime)))
  {
    flags |= 0x02;
  }
  ioBufferWrite8(buffer, flags);
  ioBufferWriteRssi(buffer, lastPass.rssiNadir);  // lowest rssi since end of last pass
  ioBufferWriteRssi(buffer, state.nodeRssiNadir);

  if (isPeakValid(history.peakSend)
      && (!isNadirValid(history.nadirSend)
          || (history.peakSend.firstTime < history.nadirSend.firstTime)))
  {
    // send peak and reset
    ioBufferWriteExtremum(buffer, &(history.peakSend), now);
    history.peakSend.rssi = 0;
  }
  else if (isNadirValid(history.nadirSend)
           && (!isPeakValid(history.peakSend)
               || (history.nadirSend.firstTime < history.peakSend.firstTime)))
  {
    // send nadir and reset
    ioBufferWriteExtremum(buffer, &(history.nadirSend), now);
    history.nadirSend.rssi = MAX_RSSI;
  }
  else
  {
    ioBufferWriteRssi(buffer, 0);
    ioBufferWrite16(buffer, 0);
    ioBufferWrite16(buffer, 0);
  }
}


//////////////////////////////////////////////////////////////////////

RotorHazardControllor::RotorHazardControllor(const std::string& uri, SPIDatagram& configuration, int node_number, time_point_t start_time)
  : _configuration(configuration)
{
  for(int i=0; i < node_number; ++i)
  {
    _nodes.emplace_back(start_time);
  }
  const auto [socket, endpoint] = open_nanomsg_pair_server(uri);
  _socket = socket;
  _endpoint = endpoint;
}


void RotorHazardControllor::consume(const Hub::item_t& item)
{
  const auto& [timestamp, datagram] = item;
  for(int i=0; i < _nodes.size(); ++i)
  {
    // our payload is
    // timestamp, node 0, node 1, ...
    // and we sample with 10 bit, meaning we have
    // to shift the value by two bits.
    _nodes[i].rssiProcess(datagram.payload[1 + i] << 2, timestamp);
  }
}


void RotorHazardControllor::start()
{
  _thread = std::thread([this]() { this->loop(); });
}


void RotorHazardControllor::loop()
{
  using namespace std::chrono_literals;
  using sck = std::chrono::steady_clock;

  set_priority(70, SCHED_RR);

  while(true)
  {
    const auto now = sck::now();
    _buffer.clear();
    _buffer.push_back('L');
    for(auto& node : _nodes)
    {
      node.readLapStats(now, _buffer);
    }
    const auto err = nn_send(_socket, _buffer.data(), _buffer.size(), NN_DONTWAIT);
    // this is from the RH code
    std::this_thread::sleep_for(100ms);
  }
}
