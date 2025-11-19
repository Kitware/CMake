/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/json/value.h>

class cmTarget;

class cmCxxModuleMetadata
{
public:
  struct PreprocessorDefineData
  {
    std::string Name;
    cm::optional<std::string> Value;
    bool Undef = false;
    cm::optional<Json::Value> Vendor;
  };

  struct LocalArgumentsData
  {
    std::vector<std::string> IncludeDirectories;
    std::vector<std::string> SystemIncludeDirectories;
    std::vector<PreprocessorDefineData> Definitions;
    cm::optional<Json::Value> Vendor;
  };

  struct ModuleData
  {
    std::string LogicalName;
    std::string SourcePath;
    bool IsInterface = true;
    bool IsStdLibrary = false;
    cm::optional<LocalArgumentsData> LocalArguments;
    cm::optional<Json::Value> Vendor;
  };

  int Version = 0;
  int Revision = 0;
  std::vector<ModuleData> Modules;
  std::string MetadataFilePath;

  std::unordered_map<std::string, Json::Value> Extensions;

  struct ParseResult;

  struct SaveResult
  {
    bool Ok = false;
    std::string Error;
    explicit operator bool() const { return Ok; }
  };

  static ParseResult LoadFromFile(std::string const& path);
  static Json::Value ToJsonValue(cmCxxModuleMetadata const& meta);
  static SaveResult SaveToFile(std::string const& path,
                               cmCxxModuleMetadata const& meta);
  static void PopulateTarget(cmTarget& target, cmCxxModuleMetadata const& meta,
                             std::vector<std::string> const& configs);
  static void PopulateTarget(cmTarget& target, cmCxxModuleMetadata const& meta,
                             cm::string_view config)
  {
    PopulateTarget(target, meta,
                   std::vector<std::string>{ std::string(config) });
  }
  static void PopulateTarget(cmTarget& target, cmCxxModuleMetadata const& meta)
  {
    PopulateTarget(target, meta, "NOCONFIG");
  }
};

struct cmCxxModuleMetadata::ParseResult
{
  cm::optional<cmCxxModuleMetadata> Meta;
  std::string Error;
  explicit operator bool() const { return static_cast<bool>(Meta); }
};
