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

#include <cm/optional>

#include <cm3p/json/value.h>
#include <stdint.h>

#include "cmInstrumentationQuery.h"

class cmInstrumentation
{
public:
  cmInstrumentation(std::string const& binary_dir);
  int InstrumentCommand(
    std::string command_type, std::vector<std::string> const& command,
    std::function<int()> const& callback,
    cm::optional<std::map<std::string, std::string>> options = cm::nullopt,
    cm::optional<std::map<std::string, std::string>> arrayOptions =
      cm::nullopt,
    bool reloadQueriesAfterCommand = false);
  std::string InstrumentTest(std::string const& name,
                             std::string const& command,
                             std::vector<std::string> const& args,
                             int64_t result,
                             std::chrono::steady_clock::time_point steadyStart,
                             std::chrono::system_clock::time_point systemStart,
                             std::string config);
  void GetPreTestStats();
  void LoadQueries();
  bool HasQuery() const;
  bool HasQuery(cmInstrumentationQuery::Query) const;
  bool HasHook(cmInstrumentationQuery::Hook) const;
  bool HasPreOrPostBuildHook() const;
  bool ReadJSONQueries(std::string const& directory);
  void ReadJSONQuery(std::string const& file);
  void WriteJSONQuery(std::set<cmInstrumentationQuery::Query> const& queries,
                      std::set<cmInstrumentationQuery::Hook> const& hooks,
                      std::vector<std::vector<std::string>> const& callback);
  void ClearGeneratedQueries();
  int CollectTimingData(cmInstrumentationQuery::Hook hook);
  int SpawnBuildDaemon();
  int CollectTimingAfterBuild(int ppid);
  void AddHook(cmInstrumentationQuery::Hook hook);
  void AddQuery(cmInstrumentationQuery::Query query);
  std::string errorMsg;
  std::string const& GetCDashDir();

private:
  void WriteInstrumentationJson(Json::Value& index,
                                std::string const& directory,
                                std::string const& file_name);
  static void InsertStaticSystemInformation(Json::Value& index);
  static void GetDynamicSystemInformation(double& memory, double& load);
  static void InsertDynamicSystemInformation(Json::Value& index,
                                             std::string const& instant);
  static void InsertTimingData(
    Json::Value& root, std::chrono::steady_clock::time_point steadyStart,
    std::chrono::system_clock::time_point systemStart);
  bool HasQueryFile(std::string const& file);
  static std::string GetCommandStr(std::vector<std::string> const& args);
  static std::string ComputeSuffixHash(std::string const& command_str);
  static std::string ComputeSuffixTime();
  void PrepareDataForCDash(std::string const& data_dir,
                           std::string const& index_path);
  std::string binaryDir;
  std::string timingDirv1;
  std::string userTimingDirv1;
  std::string cdashDir;
  std::set<cmInstrumentationQuery::Query> queries;
  std::set<cmInstrumentationQuery::Hook> hooks;
  std::vector<std::string> callbacks;
  std::vector<std::string> queryFiles;
  std::map<std::string, std::string> cdashSnippetsMap;
  Json::Value preTestStats;
  bool hasQuery = false;
};
