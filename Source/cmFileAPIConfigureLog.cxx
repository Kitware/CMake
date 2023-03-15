/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPIConfigureLog.h"

#include <cm3p/json/value.h>

#include "cmFileAPI.h"
#include "cmStringAlgorithms.h"
#include "cmake.h"

namespace {

class ConfigureLog
{
  cmFileAPI& FileAPI;
  unsigned long Version;

  Json::Value DumpPath();
  Json::Value DumpEventKindNames();

public:
  ConfigureLog(cmFileAPI& fileAPI, unsigned long version);
  Json::Value Dump();
};

ConfigureLog::ConfigureLog(cmFileAPI& fileAPI, unsigned long version)
  : FileAPI(fileAPI)
  , Version(version)
{
  static_cast<void>(this->Version);
}

Json::Value ConfigureLog::Dump()
{
  Json::Value configureLog = Json::objectValue;
  configureLog["path"] = this->DumpPath();
  configureLog["eventKindNames"] = this->DumpEventKindNames();
  return configureLog;
}

Json::Value ConfigureLog::DumpPath()
{
  return cmStrCat(this->FileAPI.GetCMakeInstance()->GetHomeOutputDirectory(),
                  "/CMakeFiles/CMakeConfigureLog.yaml");
}

Json::Value ConfigureLog::DumpEventKindNames()
{
  // Report at most one version of each event kind.
  // If a new event kind is added, increment ConfigureLogV1Minor.
  // If a new version of an existing event kind is added, a new
  // major version of the configureLog object kind is needed.
  Json::Value eventKindNames = Json::arrayValue;
  if (this->Version == 1) {
    eventKindNames.append("message-v1");     // WriteMessageEvent
    eventKindNames.append("try_compile-v1"); // WriteTryCompileEvent
    eventKindNames.append("try_run-v1");     // WriteTryRunEvent
  }
  return eventKindNames;
}
}

Json::Value cmFileAPIConfigureLogDump(cmFileAPI& fileAPI,
                                      unsigned long version)
{
  ConfigureLog configureLog(fileAPI, version);
  return configureLog.Dump();
}
