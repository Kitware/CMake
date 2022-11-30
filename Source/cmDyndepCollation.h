/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

#include "cmFileSet.h"

class cmGeneratorTarget;
class cmSourceFile;

namespace Json {
class Value;
}

struct cmDyndepGeneratorCallbacks
{
  std::function<std::string(cmSourceFile const* sf, std::string const& config)>
    ObjectFilePath;
};

struct CxxModuleFileSet
{
  std::string Name;
  std::string RelativeDirectory;
  std::string SourcePath;
  std::string Type;
  cmFileSetVisibility Visibility;
  cm::optional<std::string> Destination;
};

struct CxxModuleBmiInstall
{
  std::string Component;
  std::string Destination;
  bool ExcludeFromAll;
  bool Optional;
  std::string Permissions;
  std::string MessageLevel;
  std::string ScriptLocation;
};

struct CxxModuleExport
{
  std::string Name;
  std::string Destination;
  std::string Prefix;
  std::string CxxModuleInfoDir;
  std::string Namespace;
  bool Install;
};

struct cmCxxModuleExportInfo
{
  std::map<std::string, CxxModuleFileSet> ObjectToFileSet;
  cm::optional<CxxModuleBmiInstall> BmiInstallation;
  std::vector<CxxModuleExport> Exports;
  std::string Config;
};

struct cmDyndepCollation
{
  static void AddCollationInformation(Json::Value& tdi,
                                      cmGeneratorTarget const* gt,
                                      std::string const& config,
                                      cmDyndepGeneratorCallbacks const& cb);

  static std::unique_ptr<cmCxxModuleExportInfo> ParseExportInfo(
    Json::Value const& tdi);
};
