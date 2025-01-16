/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <set>
#include <string>
#include <vector>

#include "cmJSONState.h"

class cmInstrumentationQuery
{

public:
  enum Query
  {
    StaticSystemInformation,
    DynamicSystemInformation
  };
  static const std::vector<std::string> QueryString;

  enum Hook
  {
    PostGenerate,
    PreBuild,
    PostBuild,
    PreCMakeBuild,
    PostCMakeBuild,
    PostTest,
    PostInstall,
    Manual
  };
  static const std::vector<std::string> HookString;

  struct QueryJSONRoot
  {
    std::vector<cmInstrumentationQuery::Query> queries;
    std::vector<cmInstrumentationQuery::Hook> hooks;
    std::vector<std::string> callbacks;
    int version;
  };

  cmInstrumentationQuery() = default;
  bool ReadJSON(const std::string& file, std::string& errorMessage,
                std::set<Query>& queries, std::set<Hook>& hooks,
                std::vector<std::string>& callbacks);
  QueryJSONRoot queryRoot;
  cmJSONState parseState;
};
