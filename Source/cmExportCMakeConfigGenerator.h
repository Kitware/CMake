/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>

#include <cm/string_view>

#include "cmExportFileGenerator.h"
#include "cmGeneratorExpression.h"
#include "cmStateTypes.h"

class cmFileSet;
class cmGeneratorTarget;
class cmTargetExport;

/** \class cmExportCMakeConfigGenerator
 * \brief Generate CMake configuration files exporting targets from a build or
 * install tree.
 *
 * cmExportCMakeConfigGenerator is the superclass for
 * cmExportBuildCMakeConfigGenerator and cmExportInstallCMakeConfigGenerator.
 * It contains common code generation routines for the two kinds of export
 * implementations.
 */
class cmExportCMakeConfigGenerator : virtual public cmExportFileGenerator
{
public:
  cmExportCMakeConfigGenerator();

  void SetExportOld(bool exportOld) { this->ExportOld = exportOld; }

  void SetExportPackageDependencies(bool exportPackageDependencies)
  {
    this->ExportPackageDependencies = exportPackageDependencies;
  }

  using cmExportFileGenerator::GenerateImportFile;

protected:
  using ImportPropertyMap = std::map<std::string, std::string>;

  // Methods to implement export file code generation.
  bool GenerateImportFile(std::ostream& os) override;
  virtual void GeneratePolicyHeaderCode(std::ostream& os);
  virtual void GeneratePolicyFooterCode(std::ostream& os);
  virtual void GenerateImportHeaderCode(std::ostream& os,
                                        std::string const& config = "");
  virtual void GenerateImportFooterCode(std::ostream& os);
  void GenerateImportVersionCode(std::ostream& os);
  virtual void GenerateImportTargetCode(std::ostream& os,
                                        cmGeneratorTarget const* target,
                                        cmStateEnums::TargetType targetType);
  virtual void GenerateImportPropertyCode(
    std::ostream& os, std::string const& config, std::string const& suffix,
    cmGeneratorTarget const* target, ImportPropertyMap const& properties,
    std::string const& importedXcFrameworkLocation);
  virtual void GenerateImportedFileChecksCode(
    std::ostream& os, cmGeneratorTarget const* target,
    ImportPropertyMap const& properties,
    std::set<std::string> const& importedLocations,
    std::string const& importedXcFrameworkLocation);
  virtual void GenerateImportedFileCheckLoop(std::ostream& os);
  virtual void GenerateMissingTargetsCheckCode(std::ostream& os);
  virtual void GenerateFindDependencyCalls(std::ostream& os);

  virtual void GenerateExpectedTargetsCode(std::ostream& os,
                                           std::string const& expectedTargets);

  cm::string_view GetImportPrefixWithSlash() const override;

  virtual void GenerateInterfaceProperties(
    cmGeneratorTarget const* target, std::ostream& os,
    ImportPropertyMap const& properties);

  void SetImportLinkInterface(
    std::string const& config, std::string const& suffix,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    cmGeneratorTarget const* target, ImportPropertyMap& properties);

  void GenerateTargetFileSets(cmGeneratorTarget* gte, std::ostream& os,
                              cmTargetExport const* te = nullptr);

  std::string GetCxxModuleFile(std::string const& name) const override;

  void GenerateCxxModuleInformation(std::string const& name, std::ostream& os);

  virtual std::string GetFileSetDirectories(cmGeneratorTarget* gte,
                                            cmFileSet* fileSet,
                                            cmTargetExport const* te) = 0;
  virtual std::string GetFileSetFiles(cmGeneratorTarget* gte,
                                      cmFileSet* fileSet,
                                      cmTargetExport const* te) = 0;

  void SetRequiredCMakeVersion(unsigned int major, unsigned int minor,
                               unsigned int patch);

  bool ExportOld = false;
  bool ExportPackageDependencies = false;

  unsigned int RequiredCMakeVersionMajor = 2;
  unsigned int RequiredCMakeVersionMinor = 8;
  unsigned int RequiredCMakeVersionPatch = 3;
};
