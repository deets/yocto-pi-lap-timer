// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved

#include "arguments.hpp"

#include "cxxopts.hpp"

arguments_t parse_args(int argc, char* argv[])
{
  cxxopts::Options options(argv[0], "The SPI laptimer daemon");
  options.add_options()
  ("s,samplerate", "samplerate/second", cxxopts::value<int>()->default_value("2000"))
  ("t,thinning", "skip T samples for network transfer", cxxopts::value<int>()->default_value("10"))
  ("d,device", "SPI device to use", cxxopts::value<std::string>()->default_value("/dev/spidev0.0"))
  ("u,uri", "URI to bind to with nanomsg", cxxopts::value<std::string>()->default_value("tcp://0.0.0.0:5000"))
  ("h,help", "Print usage")
  ;
  auto result = options.parse(argc, argv);
  if (result.count("help"))
  {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  arguments_t res = {
    result["samplerate"].as<int>(),
    result["thinning"].as<int>(),
    result["device"].as<std::string>(),
    result["uri"].as<std::string>()
  };

  return res;
}
