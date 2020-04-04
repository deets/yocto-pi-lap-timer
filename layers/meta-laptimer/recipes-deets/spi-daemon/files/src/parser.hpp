// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include <variant>
#include <string_view>

struct no_cmd_t {};

struct tune_cmd_t
{
  int node;
  int frequency;
};


using parse_result_t = std::variant<no_cmd_t, tune_cmd_t>;

parse_result_t parse(const std::string_view&);
