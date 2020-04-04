// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved

#include "parser.hpp"

#include <regex>


namespace {

auto tune_rex = std::regex("T([0123456789]+):([0123456789]+)", std::regex::extended);

} // end ns anonymous


parse_result_t parse(const std::string_view& msg)
{
  parse_result_t res = no_cmd_t{};

  std::cmatch m;
  if(std::regex_match(msg.begin(), msg.end(), m, tune_rex))
  {
    auto tc = tune_cmd_t { -1, -1 };
    tc.node =  std::stoi(m[1]);
    tc.frequency =  std::stoi(m[2]);
    res = tc;
  }
  return res;
}
