// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#pragma once
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>

#define DATAGRAM_SIZE 9

const int QUEUE_SIZE = 400000;

#pragma pack(push,4)
struct SPIDatagram
{
  uint32_t control;
  uint32_t payload[DATAGRAM_SIZE-1];

  // sets our values with endian swap
  // from an array of received bytes.
  void from_byte_array(const auto& byte_array)
  {
    uint32_t value;
    const auto* p = byte_array.data();
    std::memcpy(&value, p, sizeof(uint32_t));
    control = __bswap_32(value);
    for(int i=0; i < DATAGRAM_SIZE - 1; ++i)
    {
      // on first iteration, already skip the control
      p += sizeof(uint32_t);
      std::memcpy(&value, p, sizeof(uint32_t));
      payload[i] = __bswap_32(value);
    }
  }

  // populates a passed array with
  // enidaness-swapped values from ourselves
  void to_byte_array(auto& byte_array) const
  {
    uint32_t value = __bswap_32(control);
    auto p = byte_array.data();
    std::memcpy(p, &value, sizeof(uint32_t));
    for(int i=0; i < DATAGRAM_SIZE - 1; ++i)
    {
      // on first iteration, already skip the control
      p += sizeof(uint32_t);
      value = __bswap_32(payload[i]);
      std::memcpy(p,  &value, sizeof(uint32_t));
    }
  }


  operator bool() const
  {
    return control && 0xff;
  }
};

#pragma pack(pop)

static_assert(sizeof(SPIDatagram) == DATAGRAM_SIZE * 4);

std::ostream& operator << ( std::ostream& os, SPIDatagram const& value );
