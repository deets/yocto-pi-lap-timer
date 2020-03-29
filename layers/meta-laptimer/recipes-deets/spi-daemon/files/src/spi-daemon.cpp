// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "tx.h"

#include <linux/spi/spidev.h>
#include <time.h>
#include <sys/ioctl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <vector>

#include <sys/mman.h> // Needed for mlockall()
#include <malloc.h>

#define CHECK(v, msg) if(v == -1) {                 \
  std::stringstream s; \
  s << msg << "\n"; \
  throw std::runtime_error(s.str()); \
  }

#define SPI_SPEED 3500000
// This is the only thing the pi really supports
#define SPI_BITS_PER_WORD 8

namespace {

const int SAMPLERATE = 2000;

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

void prevent_page_locking()
{
  // Now lock all current and future pages from preventing of being paged
  if (mlockall(MCL_CURRENT | MCL_FUTURE ))
  {
    perror("mlockall failed:");
  }
}

void set_priority(int prio, int sched)
{
  struct sched_param param;
  // Set realtime priority for this thread
  param.sched_priority = prio;
  if (sched_setscheduler(0, sched, &param) < 0)
    perror("sched_setscheduler");
}

} // end ns anonymous

int main(int argc, char *argv[])
{
  using namespace std::chrono_literals;

  prevent_page_locking();
  set_priority(90, SCHED_RR);

  SPIConnection connection(argv[1]);
  Transmitter tx(argv[2]);
  tx.start();

  // our buffers are 9 longs
  const auto input_data = std::vector<uint8_t>{
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe,
    0x7f, 0xff, 0x00, 0xfe
  };

  while(true)
  {
    SPIDatagram datagram;
    do
    {
      const auto& result = connection.xfer(input_data);
      datagram.from_byte_array(result);
      tx.push(datagram);
    } while(datagram.control > 8);
    std::this_thread::sleep_for(1s / SAMPLERATE);
  }
  return 0;
}
