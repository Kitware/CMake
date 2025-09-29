/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmArgumentParser.h"

struct cmTestDiscoveryArgs : ArgumentParser::ParseResult
{
  ArgumentParser::NonEmpty<std::vector<std::string>> Command;
  ArgumentParser::NonEmpty<std::vector<std::string>> DiscoveryArgs;
  ArgumentParser::NonEmpty<std::string> DiscoveryMatch;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> DiscoveryProperties;
  ArgumentParser::NonEmpty<std::string> TestName;
  ArgumentParser::NonEmpty<std::vector<std::string>> TestArgs;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> TestProperties;
};

template <typename Args>
auto cmTestDiscoveryParser() -> cmArgumentParser<Args>
{
  static_assert(std::is_base_of<cmTestDiscoveryArgs, Args>::value, "");
  return cmArgumentParser<Args>{}
    .Bind("COMMAND"_s, &Args::Command)
    .Bind("DISCOVERY_ARGS"_s, &Args::DiscoveryArgs)
    .Bind("DISCOVERY_MATCH"_s, &Args::DiscoveryMatch)
    .Bind("DISCOVERY_PROPERTIES"_s, &Args::DiscoveryProperties)
    .Bind("TEST_NAME"_s, &Args::TestName)
    .Bind("TEST_ARGS"_s, &Args::TestArgs)
    .Bind("TEST_PROPERTIES"_s, &Args::TestProperties);
}
