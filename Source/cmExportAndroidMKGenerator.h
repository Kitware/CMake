/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmExportBuildFileGenerator.h"
#include "cmStateTypes.h"

class cmGeneratorTarget;

/** \class cmExportAndroidMKGenerator
 * \brief Generate a file exporting targets from a build tree.
 *
 * cmExportAndroidMKGenerator generates a file exporting targets from
 * a build tree.  This exports the targets to the Android ndk build tool
 * makefile format for prebuilt libraries.
 *
 * This is used to implement the export() command.
 */
class cmExportAndroidMKGenerator : public cmExportBuildFileGenerator
{
public:
  cmExportAndroidMKGenerator();
  // this is so cmExportInstallAndroidMKGenerator can share this
  // function as they are almost the same
  enum GenerateType
  {
    BUILD,
    INSTALL
  };
  static void GenerateInterfaceProperties(cmGeneratorTarget const* target,
                                          std::ostream& os,
                                          ImportPropertyMap const& properties,
                                          GenerateType type,
                                          std::string const& config);

protected:
  // Implement virtual methods from the superclass.
  void GeneratePolicyHeaderCode(std::ostream&) override {}
  void GeneratePolicyFooterCode(std::ostream&) override {}
  void GenerateImportHeaderCode(std::ostream& os,
                                std::string const& config = "") override;
  void GenerateImportFooterCode(std::ostream& os) override;
  void GenerateImportTargetCode(
    std::ostream& os, cmGeneratorTarget const* target,
    cmStateEnums::TargetType /*targetType*/) override;
  void GenerateExpectedTargetsCode(
    std::ostream& os, std::string const& expectedTargets) override;
  void GenerateImportPropertyCode(
    std::ostream& os, std::string const& config, std::string const& suffix,
    cmGeneratorTarget const* target, ImportPropertyMap const& properties,
    std::string const& importedXcFrameworkLocation) override;
  void GenerateMissingTargetsCheckCode(std::ostream& os) override;
  void GenerateFindDependencyCalls(std::ostream&) override {}
  void GenerateInterfaceProperties(
    cmGeneratorTarget const* target, std::ostream& os,
    ImportPropertyMap const& properties) override;
};
