/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include <cm/string_view>

#include "cmExportFileGenerator.h"
#include "cmFindPackageStack.h"
#include "cmStateTypes.h"

namespace Json {
class Value;
}

class cmGeneratorTarget;
class cmPackageInfoArguments;

/** \class cmExportPackageInfoGenerator
 * \brief Generate Common Package Specification package information files
 * exporting targets from a build or install tree.
 *
 * cmExportPackageInfoGenerator is the superclass for
 * cmExportBuildPackageInfoGenerator and cmExportInstallPackageInfoGenerator.
 * It contains common code generation routines for the two kinds of export
 * implementations.
 */
class cmExportPackageInfoGenerator : virtual public cmExportFileGenerator
{
public:
  cmExportPackageInfoGenerator(cmPackageInfoArguments arguments);

  using cmExportFileGenerator::GenerateImportFile;

protected:
  std::string const& GetPackageName() const { return this->PackageName; }

  void WritePackageInfo(Json::Value const& packageInfo,
                        std::ostream& os) const;

  // Methods to implement export file code generation.
  bool GenerateImportFile(std::ostream& os) override;

  bool CheckPackage() const
  {
    return this->CheckVersion() && this->CheckDefaultTargets();
  }

  Json::Value GeneratePackageInfo() const;
  Json::Value* GenerateImportTarget(Json::Value& components,
                                    cmGeneratorTarget const* target,
                                    cmStateEnums::TargetType targetType) const;

  void GeneratePackageRequires(Json::Value& package) const;

  using ImportPropertyMap = std::map<std::string, std::string>;
  bool GenerateInterfaceProperties(Json::Value& component,
                                   cmGeneratorTarget const* target,
                                   ImportPropertyMap const& properties) const;
  Json::Value GenerateInterfaceConfigProperties(
    std::string const& suffix, ImportPropertyMap const& properties) const;

  cm::string_view GetImportPrefixWithSlash() const override;

  std::string GetCxxModuleFile(std::string const& /*name*/) const override
  {
    // CPS does not have a general CxxModuleFile, we use the config-specific
    // manifests directly
    return {};
  }

  void GenerateCxxModuleConfigInformation(std::string const& /*name*/,
                                          std::ostream& /*os*/) const override
  {
    // We embed this directly in the CPS json
  }

  std::string GenerateCxxModules(Json::Value& component,
                                 cmGeneratorTarget* target,
                                 std::string const& packagePath,
                                 std::string const& config);

  bool NoteLinkedTarget(cmGeneratorTarget const* target,
                        std::string const& linkedName,
                        cmGeneratorTarget const* linkedTarget) override;

private:
  bool CheckVersion() const;
  bool CheckDefaultTargets() const;

  void GenerateInterfaceLinkProperties(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    ImportPropertyMap const& properties) const;

  void GenerateInterfaceCompileFeatures(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    ImportPropertyMap const& properties) const;

  void GenerateInterfaceCompileDefines(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    ImportPropertyMap const& properties) const;

  void GenerateInterfaceListProperty(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    std::string const& outName, cm::string_view inName,
    ImportPropertyMap const& properties) const;

  void GenerateProperty(bool& result, Json::Value& component,
                        cmGeneratorTarget const* target,
                        std::string const& outName, std::string const& inName,
                        ImportPropertyMap const& properties) const;

  std::string const PackageName;
  std::string const PackageVersion;
  std::string const PackageVersionCompat;
  std::string const PackageVersionSchema;
  std::string const PackageDescription;
  std::string const PackageWebsite;
  std::string const PackageLicense;
  std::string const DefaultLicense;

  std::vector<std::string> DefaultTargets;
  std::vector<std::string> DefaultConfigurations;

  std::map<std::string, std::string> LinkTargets;
  std::map<std::string, cmPackageInformation> Requirements;
};
