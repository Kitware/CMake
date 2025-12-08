/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <set>
#include <string>
#include <vector>

#include "cmJSONState.h"

class cmInstrumentationQuery
{

public:
  enum Option
  {
    StaticSystemInformation,
    DynamicSystemInformation,
    CDashSubmit,
    CDashVerbose,
    Trace
  };
  static std::vector<std::string> const OptionString;

  enum Hook
  {
    PostGenerate,
    PreBuild,
    PostBuild,
    PreCMakeBuild,
    PostCMakeBuild,
    PostCTest,
    PostCMakeInstall,
    PostCMakeWorkflow,
    PrepareForCDash,
    Manual
  };
  static std::vector<std::string> const HookString;

  struct QueryJSONRoot
  {
    std::vector<cmInstrumentationQuery::Option> options;
    std::vector<cmInstrumentationQuery::Hook> hooks;
    std::vector<std::string> callbacks;
    int version;
  };

  cmInstrumentationQuery() = default;
  bool ReadJSON(std::string const& file, std::string& errorMessage,
                std::set<Option>& options, std::set<Hook>& hooks,
                std::vector<std::string>& callbacks);
  QueryJSONRoot queryRoot;
  cmJSONState parseState;
};
