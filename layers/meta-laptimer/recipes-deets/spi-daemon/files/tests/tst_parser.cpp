// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved

#include "parser.hpp"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"


TEST_CASE( "tune messages", "[parser]" ) {
  const auto result = parse(std::string_view("T0:5438"));
  REQUIRE(std::holds_alternative<tune_cmd_t>(result));
  const auto tune = std::get<tune_cmd_t>(result);
  REQUIRE(tune.node == 0);
  REQUIRE(tune.frequency == 5438);
}
