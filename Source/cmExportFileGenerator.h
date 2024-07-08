/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmGeneratorExpression.h"
#include "cmStateTypes.h"
#include "cmVersion.h"
#include "cmVersionConfig.h"

class cmExportSet;
class cmFileSet;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmTargetExport;

#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)

#define DEVEL_CMAKE_VERSION(major, minor)                                     \
  (CMake_VERSION_ENCODE(major, minor, 0) >                                    \
       CMake_VERSION_ENCODE(CMake_VERSION_MAJOR, CMake_VERSION_MINOR, 0)      \
     ? STRINGIFY(CMake_VERSION_MAJOR) "." STRINGIFY(                          \
         CMake_VERSION_MINOR) "." STRINGIFY(CMake_VERSION_PATCH)              \
     : #major "." #minor ".0")

/** \class cmExportFileGenerator
 * \brief Generate a file exporting targets from a build or install tree.
 *
 * cmExportFileGenerator is the superclass for
 * cmExportBuildFileGenerator and cmExportInstallFileGenerator.  It
 * contains common code generation routines for the two kinds of
 * export implementations.
 */
class cmExportFileGenerator
{
public:
  cmExportFileGenerator();
  virtual ~cmExportFileGenerator() = default;

  /** Set the full path to the export file to generate.  */
  void SetExportFile(char const* mainFile);
  std::string const& GetMainExportFileName() const;

  /** Set the namespace in which to place exported target names.  */
  void SetNamespace(std::string const& ns) { this->Namespace = ns; }
  std::string GetNamespace() const { return this->Namespace; }

  void SetExportOld(bool exportOld) { this->ExportOld = exportOld; }

  /** Add a configuration to be exported.  */
  void AddConfiguration(std::string const& config);

  /** Actually generate the export file.  Returns whether there was an
      error.  */
  bool GenerateImportFile();

  void SetExportPackageDependencies(bool exportPackageDependencies)
  {
    this->ExportPackageDependencies = exportPackageDependencies;
  }

protected:
  using ImportPropertyMap = std::map<std::string, std::string>;

  // Generate per-configuration target information to the given output
  // stream.
  void GenerateImportConfig(std::ostream& os, std::string const& config);

  // Methods to implement export file code generation.
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
    std::ostream& os, cmGeneratorTarget* target,
    ImportPropertyMap const& properties,
    std::set<std::string> const& importedLocations,
    std::string const& importedXcFrameworkLocation);
  virtual void GenerateImportedFileCheckLoop(std::ostream& os);
  virtual void GenerateMissingTargetsCheckCode(std::ostream& os);
  virtual void GenerateFindDependencyCalls(std::ostream& os);

  virtual void GenerateExpectedTargetsCode(std::ostream& os,
                                           std::string const& expectedTargets);

  // Collect properties with detailed information about targets beyond
  // their location on disk.
  void SetImportDetailProperties(std::string const& config,
                                 std::string const& suffix,
                                 cmGeneratorTarget* target,
                                 ImportPropertyMap& properties);

  enum class ImportLinkPropertyTargetNames
  {
    Yes,
    No,
  };
  template <typename T>
  void SetImportLinkProperty(std::string const& suffix,
                             cmGeneratorTarget const* target,
                             std::string const& propName,
                             std::vector<T> const& entries,
                             ImportPropertyMap& properties,
                             ImportLinkPropertyTargetNames targetNames);

  /** Each subclass knows how to generate its kind of export file.  */
  virtual bool GenerateMainFile(std::ostream& os) = 0;

  /** Each subclass knows where the target files are located.  */
  virtual void GenerateImportTargetsConfig(std::ostream& os,
                                           std::string const& config,
                                           std::string const& suffix) = 0;

  /** Each subclass knows how to deal with a target that is  missing from an
   *  export set.  */
  virtual void HandleMissingTarget(std::string& link_libs,
                                   cmGeneratorTarget const* depender,
                                   cmGeneratorTarget* dependee) = 0;
  void PopulateInterfaceProperty(std::string const&,
                                 cmGeneratorTarget const* target,
                                 cmGeneratorExpression::PreprocessContext,
                                 ImportPropertyMap& properties);
  bool PopulateInterfaceLinkLibrariesProperty(
    cmGeneratorTarget const* target, cmGeneratorExpression::PreprocessContext,
    ImportPropertyMap& properties);
  void PopulateInterfaceProperty(std::string const& propName,
                                 cmGeneratorTarget const* target,
                                 ImportPropertyMap& properties);
  void PopulateCompatibleInterfaceProperties(cmGeneratorTarget const* target,
                                             ImportPropertyMap& properties);
  void PopulateCustomTransitiveInterfaceProperties(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
  virtual void GenerateInterfaceProperties(
    cmGeneratorTarget const* target, std::ostream& os,
    ImportPropertyMap const& properties);
  void PopulateIncludeDirectoriesInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties, cmTargetExport const& te,
    std::string& includesDestinationDirs);
  void PopulateSourcesInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
  void PopulateLinkDirectoriesInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
  void PopulateLinkDependsInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);

  void SetImportLinkInterface(
    std::string const& config, std::string const& suffix,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    cmGeneratorTarget const* target, ImportPropertyMap& properties);

  enum FreeTargetsReplace
  {
    ReplaceFreeTargets,
    NoReplaceFreeTargets
  };

  void ResolveTargetsInGeneratorExpressions(
    std::string& input, cmGeneratorTarget const* target,
    FreeTargetsReplace replace = NoReplaceFreeTargets);

  bool PopulateCxxModuleExportProperties(
    cmGeneratorTarget const* gte, ImportPropertyMap& properties,
    cmGeneratorExpression::PreprocessContext ctx,
    std::string const& includesDestinationDirs, std::string& errorMessage);
  bool PopulateExportProperties(cmGeneratorTarget const* gte,
                                ImportPropertyMap& properties,
                                std::string& errorMessage);

  void GenerateTargetFileSets(cmGeneratorTarget* gte, std::ostream& os,
                              cmTargetExport* te = nullptr);

  void GenerateCxxModuleInformation(std::string const& name, std::ostream& os);

  virtual std::string GetFileSetDirectories(cmGeneratorTarget* gte,
                                            cmFileSet* fileSet,
                                            cmTargetExport* te) = 0;
  virtual std::string GetFileSetFiles(cmGeneratorTarget* gte,
                                      cmFileSet* fileSet,
                                      cmTargetExport* te) = 0;

  virtual cmExportSet* GetExportSet() const { return nullptr; }

  std::string GetCxxModuleFile(std::string const& name) const;

  void SetRequiredCMakeVersion(unsigned int major, unsigned int minor,
                               unsigned int patch);

  // The namespace in which the exports are placed in the generated file.
  std::string Namespace;

  bool ExportOld;

  // The set of configurations to export.
  std::vector<std::string> Configurations;

  // The file to generate.
  std::string MainImportFile;
  std::string FileDir;
  std::string FileBase;
  std::string FileExt;
  bool AppendMode;

  // The set of targets included in the export.
  std::set<cmGeneratorTarget*> ExportedTargets;

  std::vector<std::string> MissingTargets;

  std::set<cmGeneratorTarget const*> ExternalTargets;

  unsigned int RequiredCMakeVersionMajor = 2;
  unsigned int RequiredCMakeVersionMinor = 8;
  unsigned int RequiredCMakeVersionPatch = 3;

  bool ExportPackageDependencies = false;

private:
  void PopulateInterfaceProperty(std::string const&, std::string const&,
                                 cmGeneratorTarget const* target,
                                 cmGeneratorExpression::PreprocessContext,
                                 ImportPropertyMap& properties);

  bool AddTargetNamespace(std::string& input, cmGeneratorTarget const* target,
                          cmLocalGenerator const* lg);

  void ResolveTargetsInGeneratorExpression(std::string& input,
                                           cmGeneratorTarget const* target,
                                           cmLocalGenerator const* lg);

  virtual void ReplaceInstallPrefix(std::string& input);

  virtual std::string InstallNameDir(cmGeneratorTarget const* target,
                                     std::string const& config) = 0;

  virtual std::string GetCxxModulesDirectory() const = 0;
  virtual void GenerateCxxModuleConfigInformation(std::string const&,
                                                  std::ostream& os) const = 0;
};
