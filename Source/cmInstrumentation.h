/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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
  // Read Queries
  cmInstrumentation(const std::string& binary_dir,
                    bool clear_generated = false);
  // Create Query
  cmInstrumentation(const std::string& binary_dir,
                    std::set<cmInstrumentationQuery::Query>& queries,
                    std::set<cmInstrumentationQuery::Hook>& hooks,
                    std::string& callback);
  int InstrumentCommand(
    std::string command_type, const std::vector<std::string>& command,
    const std::function<int()>& callback,
    cm::optional<std::map<std::string, std::string>> options = cm::nullopt,
    cm::optional<std::map<std::string, std::string>> arrayOptions =
      cm::nullopt,
    bool reloadQueriesAfterCommand = false);
  int InstrumentTest(const std::string& name, const std::string& command,
                     const std::vector<std::string>& args, int64_t result,
                     std::chrono::steady_clock::time_point steadyStart,
                     std::chrono::system_clock::time_point systemStart);
  void GetPreTestStats();
  void LoadQueries();
  bool HasQuery();
  bool HasQuery(cmInstrumentationQuery::Query);
  bool ReadJSONQueries(const std::string& directory);
  void ReadJSONQuery(const std::string& file);
  void WriteJSONQuery();
  int CollectTimingData(cmInstrumentationQuery::Hook hook);
  std::string errorMsg;

private:
  void WriteInstrumentationJson(Json::Value& index,
                                const std::string& directory,
                                const std::string& file_name);
  static void InsertStaticSystemInformation(Json::Value& index);
  static void GetDynamicSystemInformation(double& memory, double& load);
  static void InsertDynamicSystemInformation(Json::Value& index,
                                             const std::string& instant);
  static void InsertTimingData(
    Json::Value& root, std::chrono::steady_clock::time_point steadyStart,
    std::chrono::system_clock::time_point systemStart);
  void ClearGeneratedQueries();
  bool HasQueryFile(const std::string& file);
  static std::string GetCommandStr(const std::vector<std::string>& args);
  static std::string ComputeSuffixHash(std::string const& command_str);
  static std::string ComputeSuffixTime();
  std::string binaryDir;
  std::string timingDirv1;
  std::string userTimingDirv1;
  std::set<cmInstrumentationQuery::Query> queries;
  std::set<cmInstrumentationQuery::Hook> hooks;
  std::vector<std::string> callbacks;
  std::vector<std::string> queryFiles;
  Json::Value preTestStats;
  bool hasQuery = false;
};
