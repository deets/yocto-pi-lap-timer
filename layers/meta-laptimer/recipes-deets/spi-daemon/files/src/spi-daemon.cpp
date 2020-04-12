// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "tx.hpp"
#include "realtime.h"
#include "arguments.hpp"
#include "rotorhazard.hpp"

#include <linux/spi/spidev.h>
#include <time.h>
#include <sys/ioctl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <vector>


#define CHECK(v, msg) if(v == -1) {                 \
  std::stringstream s; \
  s << msg << "\n"; \
  throw std::runtime_error(s.str()); \
  }

#define SPI_SPEED 3500000
// This is the only thing the pi really supports
#define SPI_BITS_PER_WORD 8

namespace {

const std::vector<uint32_t> raceband = {5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917};

class SPIConnection
{
public:
  SPIConnection(const std::string& device)
    : _spi_fd(-1)
  {
    const auto fd = open(device.c_str(), O_RDWR);
    CHECK(fd, "Can't open SPI device ");
    _spi_fd = fd;

    const int mode = 0;
    const auto bits = SPI_BITS_PER_WORD;
    const auto speed = SPI_SPEED;
    const uint8_t lsb = 0;
    CHECK(ioctl(fd, SPI_IOC_WR_MODE, &mode), "SPI_IOC_WR_MODE");
    CHECK(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits), "SPI_IOC_WR_BITS_PER_WORD");
    CHECK(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed), "SPI_IOC_WR_MAX_SPEED_HZ");
    CHECK(ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb), "SPI_IOC_WR_LSB_FIRST");
  }

  const std::vector<uint8_t>& xfer(const std::vector<uint8_t>& buffer)
  {
    // forces the right size for subsequent reading
    _result.resize(buffer.size());
    struct spi_ioc_transfer tr = {
      .tx_buf=reinterpret_cast<__u64>(buffer.data()),
      .rx_buf=reinterpret_cast<__u64>(_result.data()),
      .len=buffer.size(),
    };
    CHECK(ioctl(_spi_fd, SPI_IOC_MESSAGE(1), &tr), "xfer error");
    return _result;
  }

private:
  int _spi_fd;
  std::vector<uint8_t> _result;
};

} // end ns anonymous

int main(int argc, char *argv[])
{
  using namespace std::chrono_literals;
  using sck = std::chrono::steady_clock;

  const auto args = parse_args(argc, argv);

  prevent_page_locking();
  set_priority(90, SCHED_RR);

  SPIDatagram configuration;
  int i = 0;
  for(const auto& f : raceband)
  {
    configuration.payload[i++] = f;
  }
  SPIConnection connection(args.device);
  Transmitter tx(args.uri, configuration, args.thinning);
  RotorHazardControllor rhc(args.rh_uri, configuration, 1, sck::now());
  Hub hub;

  hub.register_listener(tx);
  hub.register_listener(rhc);

  hub.start();
  rhc.start();
  tx.start();

  const auto period = 1000000us / args.samplerate;
  configuration.control = args.samplerate;

  auto output_data = std::vector<uint8_t>(sizeof(SPIDatagram));

  auto sleep_until = sck::now();
  while(true)
  {
    SPIDatagram datagram;

    // ATTENTION: this setup currently only ever
    // allows a configuration change when we are
    // not speed-reading. Which is fine, as
    // we expect these to be exceptional circumstances
    // anyway.
    configuration.to_byte_array(output_data);
    do
    {
      const auto& result = connection.xfer(output_data);
      datagram.from_byte_array(result);
      if(datagram && !hub.push(datagram))
      {
        std::cerr << "queue overflow, aborting.\n";
        abort();
      }
    } while(datagram.more_data_available());
    sleep_until += period;
    std::this_thread::sleep_for(sleep_until - sck::now());
  }
  return 0;
}
