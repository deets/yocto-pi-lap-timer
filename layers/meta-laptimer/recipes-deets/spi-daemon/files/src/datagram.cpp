// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "datagram.h"

std::ostream& operator << ( std::ostream& os, SPIDatagram const& value )
{
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
