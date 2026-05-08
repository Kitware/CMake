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
    CaptureOutput,
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

  struct Version
  {
    int Major = 0;
    int Minor = 0;
  };

  struct Callback
  {
    std::string Command;
    Version DataVersion;
  };

  struct QueryJSONRoot
  {
    std::vector<cmInstrumentationQuery::Option> options;
    std::vector<cmInstrumentationQuery::Hook> hooks;
    std::vector<std::string> callbacks;
    Version version;
  };

  cmInstrumentationQuery() = default;
  bool ReadJSON(std::string const& file, std::string& errorMessage,
                std::set<Option>& options, std::set<Hook>& hooks,
                std::vector<Callback>& callbacks);
  QueryJSONRoot queryRoot;
  cmJSONState parseState;
  static Version LatestDataVersion();
  static bool ValidDataVersion(Version version);
};
