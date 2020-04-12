// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include <string>

struct arguments_t
{
  int samplerate;
  int thinning;
  std::string device;
  std::string uri;
  std::string rh_uri;
};

arguments_t parse_args(int argc, char* argv[]);
