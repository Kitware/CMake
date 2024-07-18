/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <string>

#include <cm/string_view>

#include "cmExportFileGenerator.h"
#include "cmStateTypes.h"

class cmGeneratorTarget;

/** \class cmExportAndroidMKGenerator
 * \brief Generate CMake configuration files exporting targets from a build or
 * install tree.
 *
 * cmExportAndroidMKGenerator is the superclass for
 * cmExportBuildAndroidMKGenerator and cmExportInstallAndroidMKGenerator.
 * It contains common code generation routines for the two kinds of export
 * implementations.
 */
class cmExportAndroidMKGenerator : virtual public cmExportFileGenerator
{
public:
  cmExportAndroidMKGenerator();

  using cmExportFileGenerator::GenerateImportFile;

protected:
  enum GenerateType
  {
    BUILD,
    INSTALL
  };
  virtual GenerateType GetGenerateType() const = 0;

  using ImportPropertyMap = std::map<std::string, std::string>;

  cm::string_view GetImportPrefixWithSlash() const override;

  void GenerateInterfaceProperties(cmGeneratorTarget const* target,
                                   std::ostream& os,
                                   ImportPropertyMap const& properties);

  // Methods to implement export file code generation.
  bool GenerateImportFile(std::ostream& os) override;
  virtual void GenerateImportHeaderCode(std::ostream& os,
                                        std::string const& config = "") = 0;
  virtual void GenerateImportTargetCode(
    std::ostream& os, cmGeneratorTarget const* target,
    cmStateEnums::TargetType targetType) = 0;

  void GenerateImportTargetsConfig(std::ostream& /*os*/,
                                   std::string const& /*config*/,
                                   std::string const& /*suffix*/) override
  {
  }

  std::string GetCxxModuleFile(std::string const& /*name*/) const override
  {
    return {};
  }

  void GenerateCxxModuleConfigInformation(std::string const& /*name*/,
                                          std::ostream& /*os*/) const override
  {
  }
};
