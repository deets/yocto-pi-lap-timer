#pragma once

#include "FastRunningMedian.hpp"

#include <vector>
#include <cstdint>
#include <chrono>

using time_point_t = std::chrono::steady_clock::time_point;
using duration_t = std::chrono::steady_clock::duration;

typedef uint8_t rssi_t;
using namespace std::chrono_literals;

#define MAX_RSSI 0xFF

const duration_t MAX_DURATION = 0xFFFF * 1ms;

#define SmoothingSamples 255
#define SmoothingTimestampSize 128 // half median window, rounded up
#define isPeakValid(x) ((x).rssi != 0)
#define isNadirValid(x) ((x).rssi != MAX_RSSI)
#define endTime(x) ((x).firstTime + (x).duration)

struct Settings
{
  uint16_t vtxFreq = 5800;
  // lap pass begins when RSSI is at or above this level
  rssi_t enterAtLevel = 96;
  // lap pass ends when RSSI goes below this level
  rssi_t exitAtLevel = 80;
};

struct Extremum
{
  rssi_t rssi;
  time_point_t firstTime;
  duration_t duration;
};

struct State
{
  bool crossing = false; // True when the quad is going through the gate
  rssi_t rssi = 0; // Smoothed rssi value
  rssi_t lastRssi = 0;
  time_point_t rssiTimestamp {}; // timestamp of the smoothed value

  Extremum passPeak = {0, {}, {}}; // peak seen during current pass - only valid if pass.rssi != 0
  rssi_t passRssiNadir = MAX_RSSI; // lowest smoothed rssi seen since end of last pass

  rssi_t nodeRssiPeak = 0; // peak smoothed rssi seen since the node frequency was set
  rssi_t nodeRssiNadir = MAX_RSSI; // lowest smoothed rssi seen since the node frequency was set

  bool activatedFlag = false; // Set true after initial WRITE_FREQUENCY command received

  time_point_t lastloopMicros{};
};

struct History
{
  Extremum peak = {0, {}, {}};
  bool hasPendingPeak = false;
  Extremum peakSend = {0, {}, {}}; // only valid if peakSend.rssi != 0

  Extremum nadir = {MAX_RSSI, {}, {}};
  bool hasPendingNadir = false;
  Extremum nadirSend = {MAX_RSSI, {}, {}}; // only valid if nadirSend.rssi != MAX_RSSI

  int8_t rssiChange = 0; // >0 for raising, <0 for falling
};

struct LastPass
{
  rssi_t rssiPeak = 0;
  time_point_t timestamp{};
  rssi_t rssiNadir = MAX_RSSI;
  uint8_t lap = 0;
};


class RotorHazardNode {
public:

  RotorHazardNode(time_point_t);

  bool rssiStateValid();
  /**
   * Restarts rssi peak tracking for node.
   */
  void rssiStateReset();
  bool rssiProcess(rssi_t rssi, time_point_t millis);
  void rssiEndCrossing();

  std::vector<uint8_t> readLapStats(time_point_t now);

private:
  void initExtremum(Extremum *e);
  void bufferHistoricPeak(bool force);
  void bufferHistoricNadir(bool force);

  time_point_t SmoothingTimestamps[SmoothingTimestampSize];
  uint8_t SmoothingTimestampsIndex = 0;
  FastRunningMedian<rssi_t, SmoothingSamples, 0> rssiMedian;

  struct Settings settings;
  struct State state;
  struct History history;
  struct LastPass lastPass;

};
