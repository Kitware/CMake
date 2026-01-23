/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cm/memory>
#include <cm/optional>

#include <cm3p/json/value.h>
#include <stddef.h>

#ifndef CMAKE_BOOTSTRAP
#  include <cmsys/SystemInformation.hxx>
#endif
#include <stdint.h>

#include "cmFileLock.h"
#include "cmInstrumentationQuery.h"
#include "cmStateTypes.h"

class cmGlobalGenerator;

class cmInstrumentation
{
public:
  enum class LoadQueriesAfter
  {
    Yes,
    No
  };
  cmInstrumentation(std::string const& binary_dir,
                    LoadQueriesAfter loadQueries = LoadQueriesAfter::Yes);
  void LoadQueries();
  void CheckCDashVariable();
  int InstrumentCommand(
    std::string command_type, std::vector<std::string> const& command,
    std::function<int()> const& callback,
    cm::optional<std::map<std::string, std::string>> options = cm::nullopt,
    cm::optional<std::map<std::string, std::string>> arrayOptions =
      cm::nullopt,
    LoadQueriesAfter reloadQueriesAfterCommand = LoadQueriesAfter::No);
  std::string InstrumentTest(std::string const& name,
                             std::string const& command,
                             std::vector<std::string> const& args,
                             int64_t result,
                             std::chrono::steady_clock::time_point steadyStart,
                             std::chrono::system_clock::time_point systemStart,
                             std::string config);
  void GetPreTestStats();
  bool HasQuery() const;
  bool HasOption(cmInstrumentationQuery::Option option) const;
  bool HasHook(cmInstrumentationQuery::Hook hook) const;
  bool ReadJSONQueries(std::string const& directory);
  void ReadJSONQuery(std::string const& file);
  void WriteJSONQuery(std::set<cmInstrumentationQuery::Option> const& options,
                      std::set<cmInstrumentationQuery::Hook> const& hooks,
                      std::vector<std::vector<std::string>> const& callback);
  void AddCustomContent(std::string const& name, Json::Value const& contents);
  void WriteCMakeContent(std::unique_ptr<cmGlobalGenerator> const& gg);
  Json::Value DumpTargets(std::unique_ptr<cmGlobalGenerator> const& gg);
  void ClearGeneratedQueries();
  int CollectTimingData(cmInstrumentationQuery::Hook hook);
  int SpawnBuildDaemon();
  bool LockBuildDaemon();
  int CollectTimingAfterBuild(int ppid);
  void AddHook(cmInstrumentationQuery::Hook hook);
  void AddOption(cmInstrumentationQuery::Option option);
  bool HasErrors() const;
  std::string const& GetCDashDir() const;
  std::string const& GetDataDir() const;
  enum LatestOrOldest
  {
    Latest,
    Oldest
  };
  std::string GetFileByTimestamp(LatestOrOldest latestOrOldest,
                                 std::string const& dataSubdir,
                                 std::string const& exclude = "");

private:
  Json::Value ReadJsonSnippet(std::string const& file_name);
  void WriteInstrumentationJson(Json::Value& index,
                                std::string const& directory,
                                std::string const& file_name);
  void InsertStaticSystemInformation(Json::Value& index);
  void GetDynamicSystemInformation(double& memory, double& load);
  void InsertDynamicSystemInformation(Json::Value& index,
                                      std::string const& instant);
  void InsertTimingData(Json::Value& root,
                        std::chrono::steady_clock::time_point steadyStart,
                        std::chrono::system_clock::time_point systemStart);
  bool HasQueryFile(std::string const& file);
  static std::string GetCommandStr(std::vector<std::string> const& args);
  static std::string ComputeSuffixHash(std::string const& command_str);
  static std::string ComputeSuffixTime(
    cm::optional<std::chrono::system_clock::time_point> time = cm::nullopt);
  static bool IsInstrumentableTargetType(cmStateEnums::TargetType type);
  void PrepareDataForCDash(std::string const& data_dir,
                           std::string const& index_path);
  void RemoveOldFiles(std::string const& dataSubdir);
  void WriteTraceFile(Json::Value const& index, std::string const& trace_name);
  Json::Value BuildTraceEvent(std::vector<uint64_t>& workers,
                              Json::Value const& snippetData);
  size_t AssignTargetToTraceThread(std::vector<uint64_t>& workers,
                                   uint64_t timeStart, uint64_t duration);
  std::string binaryDir;
  std::string timingDirv1;
  std::string userTimingDirv1;
  std::string cdashDir;
  std::string dataDir;
  std::set<cmInstrumentationQuery::Option> options;
  std::set<cmInstrumentationQuery::Hook> hooks;
  std::vector<std::string> callbacks;
  std::vector<std::string> queryFiles;
  static std::map<std::string, std::string> cdashSnippetsMap;
  Json::Value preTestStats;
  std::string errorMsg;
  bool hasQuery = false;
  bool ranSystemChecks = false;
  bool ranOSCheck = false;
  Json::Value customContent = Json::objectValue;
  Json::Value configureSnippetData;
  std::string configureSnippetName;
#ifndef CMAKE_BOOTSTRAP
  std::unique_ptr<cmsys::SystemInformation> systemInformation;
  cmsys::SystemInformation& GetSystemInformation();
#endif
  int writtenJsonQueries = 0;
  cmFileLock lock;
};
