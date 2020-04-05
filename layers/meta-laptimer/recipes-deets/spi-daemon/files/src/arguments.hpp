// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include <string>

struct arguments_t
{
  int samplerate;
  std::string device;
  std::string uri;
};

arguments_t parse_args(int argc, char* argv[]);
