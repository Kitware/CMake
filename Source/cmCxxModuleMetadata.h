/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
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
    // definition["name"]
    std::string Name;
    // definition["value"]
    cm::optional<std::string> Value;
    // definition["undef"]
    bool Undef = false;
  };

  struct LocalArgumentsData
  {
    // local-arguments["include-directories"]
    std::vector<std::string> IncludeDirectories;
    // local-arguments["system-include-directories"]
    std::vector<std::string> SystemIncludeDirectories;
    // local-arguments["definitions"]
    std::vector<PreprocessorDefineData> Definitions;

    // These are CMake vendor extensions
    // local-arguments["vendor"]["cmake"]["compile-options"]
    std::vector<std::string> CompileOptions;
    // local-arguments["vendor"]["cmake"]["compile-features"]
    std::vector<std::string> CompileFeatures;
  };

  struct ModuleData
  {
    // module["logical-name"]
    std::string LogicalName;
    // module["source-path"]
    std::string SourcePath;
    // module["is-interface"]
    bool IsInterface = true;
    // module["is-std-library"]
    bool IsStdLibrary = false;
    // module["local-arguments"]
    cm::optional<LocalArgumentsData> LocalArguments;
  };

  // root["version"]
  int Version = 1;
  // root["revision"]
  int Revision = 1;
  // root["modules"]
  std::vector<ModuleData> Modules;

  // The path to the manifest file, either absolute or relative to the
  // installation root
  std::string MetadataFilePath;

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
