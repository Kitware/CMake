/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

class cmGeneratorTarget;
struct cmScanDepInfo;
class cmSourceFile;

namespace Json {
class Value;
}

struct cmDyndepGeneratorCallbacks
{
  std::function<std::string(cmSourceFile const* sf, std::string const& config)>
    ObjectFilePath;
  std::function<std::string(cmSourceFile const* sf, std::string const& config)>
    BmiFilePath;
};

struct cmDyndepMetadataCallbacks
{
  std::function<cm::optional<std::string>(std::string const& name)> ModuleFile;
};

struct cmCxxModuleExportInfo;
struct cmCxxModuleExportInfoDeleter
{
  void operator()(cmCxxModuleExportInfo* ei) const;
};

struct cmDyndepCollation
{
  static void AddCollationInformation(Json::Value& tdi,
                                      cmGeneratorTarget const* gt,
                                      std::string const& config,
                                      cmDyndepGeneratorCallbacks const& cb);

  static std::unique_ptr<cmCxxModuleExportInfo, cmCxxModuleExportInfoDeleter>
  ParseExportInfo(Json::Value const& tdi);
  static bool WriteDyndepMetadata(std::string const& lang,
                                  std::vector<cmScanDepInfo> const& objects,
                                  cmCxxModuleExportInfo const& export_info,
                                  cmDyndepMetadataCallbacks const& cb);
  static bool IsObjectPrivate(std::string const& object,
                              cmCxxModuleExportInfo const& export_info);

  static bool IsBmiOnly(cmCxxModuleExportInfo const& exportInfo,
                        std::string const& object);
};
