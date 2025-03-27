/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmGeneratorExpression.h"

class cmExportSet;
class cmGeneratorTarget;
class cmLocalGenerator;

#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)

#define DEVEL_CMAKE_VERSION(major, minor)                                     \
  (CMake_VERSION_ENCODE(major, minor, 0) >                                    \
       CMake_VERSION_ENCODE(CMake_VERSION_MAJOR, CMake_VERSION_MINOR, 0)      \
     ? STRINGIFY(CMake_VERSION_MAJOR) "." STRINGIFY(                          \
         CMake_VERSION_MINOR) "." STRINGIFY(CMake_VERSION_PATCH)              \
     : #major "." #minor ".0")

/** \class cmExportFileGenerator
 * \brief Generate files exporting targets from a build or install tree.
 *
 * cmExportFileGenerator is the interface class for generating export files.
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

  /** Add a configuration to be exported.  */
  void AddConfiguration(std::string const& config);

  /** Create and actually generate the export file.  Returns whether there was
      an error.  */
  bool GenerateImportFile();

protected:
  using ImportPropertyMap = std::map<std::string, std::string>;

  // Collect properties with detailed information about targets beyond
  // their location on disk.
  void SetImportDetailProperties(std::string const& config,
                                 std::string const& suffix,
                                 cmGeneratorTarget const* target,
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

  /** Generate the export file to the given output stream.  Returns whether
      there was an error.  */
  virtual bool GenerateImportFile(std::ostream& os) = 0;

  /** Each subclass knows how to generate its kind of export file.  */
  virtual bool GenerateMainFile(std::ostream& os) = 0;

  /** Generate per-configuration target information to the given output
      stream.  */
  virtual void GenerateImportConfig(std::ostream& os,
                                    std::string const& config);

  /** Each subclass knows where the target files are located.  */
  virtual void GenerateImportTargetsConfig(std::ostream& os,
                                           std::string const& config,
                                           std::string const& suffix) = 0;

  /** Record a target referenced by an exported target. */
  virtual bool NoteLinkedTarget(cmGeneratorTarget const* target,
                                std::string const& linkedName,
                                cmGeneratorTarget const* linkedTarget);

  /** Each subclass knows how to deal with a target that is  missing from an
   *  export set.  */
  virtual void HandleMissingTarget(std::string& link_libs,
                                   cmGeneratorTarget const* depender,
                                   cmGeneratorTarget* dependee) = 0;

  /** Complain when a duplicate target is encountered.  */
  virtual void ComplainAboutDuplicateTarget(
    std::string const& targetName) const = 0;

  virtual cm::string_view GetImportPrefixWithSlash() const = 0;

  void AddImportPrefix(std::string& exportDirs) const;

  void PopulateInterfaceProperty(std::string const& propName,
                                 cmGeneratorTarget const* target,
                                 ImportPropertyMap& properties) const;
  void PopulateInterfaceProperty(std::string const& propName,
                                 cmGeneratorTarget const* target,
                                 cmGeneratorExpression::PreprocessContext,
                                 ImportPropertyMap& properties);
  bool PopulateInterfaceLinkLibrariesProperty(
    cmGeneratorTarget const* target, cmGeneratorExpression::PreprocessContext,
    ImportPropertyMap& properties);

  bool PopulateInterfaceProperties(
    cmGeneratorTarget const* target,
    std::string const& includesDestinationDirs,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);

  virtual void ReportError(std::string const& errorMessage) const = 0;

  using ExportInfo = std::pair<std::vector<std::string>, std::string>;

  /** Find the set of export files and the unique namespace (if any) for a
   *  target. */
  virtual ExportInfo FindExportInfo(cmGeneratorTarget const* target) const = 0;

  enum FreeTargetsReplace
  {
    ReplaceFreeTargets,
    NoReplaceFreeTargets
  };

  void ResolveTargetsInGeneratorExpressions(
    std::string& input, cmGeneratorTarget const* target,
    FreeTargetsReplace replace = NoReplaceFreeTargets);

  virtual cmExportSet* GetExportSet() const { return nullptr; }

  virtual void ReplaceInstallPrefix(std::string& input) const;

  virtual std::string InstallNameDir(cmGeneratorTarget const* target,
                                     std::string const& config) = 0;

  /** Get the temporary location of the config-agnostic C++ module file.  */
  virtual std::string GetCxxModuleFile(std::string const& name) const = 0;

  virtual std::string GetCxxModulesDirectory() const = 0;
  virtual void GenerateCxxModuleConfigInformation(std::string const&,
                                                  std::ostream& os) const = 0;

  bool AddTargetNamespace(std::string& input, cmGeneratorTarget const* target,
                          cmLocalGenerator const* lg);

  // The namespace in which the exports are placed in the generated file.
  std::string Namespace;

  // The set of configurations to export.
  std::vector<std::string> Configurations;

  // The file to generate.
  std::string MainImportFile;
  std::string FileDir;
  std::string FileBase;
  std::string FileExt;
  bool AppendMode = false;

  // The set of targets included in the export.
  std::set<cmGeneratorTarget const*> ExportedTargets;

  std::vector<std::string> MissingTargets;

  std::set<cmGeneratorTarget const*> ExternalTargets;

private:
  void PopulateInterfaceProperty(std::string const& propName,
                                 std::string const& outputName,
                                 cmGeneratorTarget const* target,
                                 cmGeneratorExpression::PreprocessContext,
                                 ImportPropertyMap& properties);

  void PopulateCompatibleInterfaceProperties(
    cmGeneratorTarget const* target, ImportPropertyMap& properties) const;
  void PopulateCustomTransitiveInterfaceProperties(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
  bool PopulateCxxModuleExportProperties(
    cmGeneratorTarget const* gte, ImportPropertyMap& properties,
    cmGeneratorExpression::PreprocessContext ctx,
    std::string const& includesDestinationDirs, std::string& errorMessage);
  bool PopulateExportProperties(cmGeneratorTarget const* gte,
                                ImportPropertyMap& properties,
                                std::string& errorMessage) const;

  void ResolveTargetsInGeneratorExpression(std::string& input,
                                           cmGeneratorTarget const* target,
                                           cmLocalGenerator const* lg);
};

extern template void cmExportFileGenerator::SetImportLinkProperty<std::string>(
  std::string const&, cmGeneratorTarget const*, std::string const&,
  std::vector<std::string> const&, ImportPropertyMap& properties,
  ImportLinkPropertyTargetNames);

extern template void cmExportFileGenerator::SetImportLinkProperty<cmLinkItem>(
  std::string const&, cmGeneratorTarget const*, std::string const&,
  std::vector<cmLinkItem> const&, ImportPropertyMap& properties,
  ImportLinkPropertyTargetNames);
