// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved

#include "readerwriterqueue.h"

#include <linux/spi/spidev.h>
#include <time.h>
#include <sys/ioctl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <cstdint>
#include <vector>
#include <cstring>
#include <iomanip>

using namespace moodycamel;

#define CHECK(v, msg) if(v == -1) {                 \
  std::stringstream s; \
  s << msg << "\n"; \
  throw std::runtime_error(s.str()); \
  }

#define SPI_SPEED 3500000
#define SPI_BITS_PER_WORD 8

namespace {

struct SPIDatagram
{
  uint32_t control;
  uint32_t payload[8];

  void from_byte_array(const auto& byte_array)
  {
    uint32_t value;
    const auto* p = byte_array.data();
    std::memcpy(&value, p, sizeof(uint32_t));
    control = __bswap_32(value);
    for(int i=0; i < 8; ++i)
    {
      // on first iteration, already skip the control
      p += sizeof(uint32_t);
      std::memcpy(&value, p, sizeof(uint32_t));
      payload[i] = __bswap_32(value);
    }
  }

  operator bool() const
  {
    return control && 0xff;
  }

};


std::ostream& operator << ( std::ostream& os, SPIDatagram const& value ) {
  os << std::hex << std::internal << std::setfill('0');
  os << std::setw(8);
  os << value.control;
  for(int i=0; i < sizeof(value.payload) / sizeof(uint32_t); ++i)
  {
    os << ":";
    os << std::setw(8);
    if(value)
    {
      os << value.payload[i];
    }
    else
    {
      os << 0;
    }
  }
  os << std::dec;
  return os;
}



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
  SPIConnection connection(argv[1]);

  struct timespec  period = { 0, 100000000 };

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
    const auto& result = connection.xfer(input_data);
    datagram.from_byte_array(result);
    std::cout << datagram << "\n";
    nanosleep(&period, nullptr);
  }
  return 0;
}
