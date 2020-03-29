// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#pragma once
#include <cstdint>
#include <cstring>
#include <iomanip>

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


std::ostream& operator << ( std::ostream& os, SPIDatagram const& value );
