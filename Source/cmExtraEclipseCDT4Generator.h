/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include "cmExternalMakefileProjectGenerator.h"

class cmLocalGenerator;
class cmMakefile;
class cmSourceGroup;
class cmXMLWriter;

/** \class cmExtraEclipseCDT4Generator
 * \brief Write Eclipse project files for Makefile based projects
 */
class cmExtraEclipseCDT4Generator : public cmExternalMakefileProjectGenerator
{
public:
  enum LinkType
  {
    VirtualFolder,
    LinkToFolder,
    LinkToFile
  };

  cmExtraEclipseCDT4Generator();

  static cmExternalMakefileProjectGeneratorFactory* GetFactory();

  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

  void Generate() override;

private:
  // create .project file in the source tree
  void CreateSourceProjectFile();

  // create .settings/org.eclipse.core.resources.prefs
  void CreateSettingsResourcePrefsFile();

  // create .project file
  void CreateProjectFile();

  // create .cproject file
  void CreateCProjectFile() const;

  // If built with cygwin cmake, convert posix to windows path.
  static std::string GetEclipsePath(std::string const& path);

  // Extract basename.
  static std::string GetPathBasename(std::string const& path);

  // Generate the project name as: <name>-<type>@<path>
  static std::string GenerateProjectName(std::string const& name,
                                         std::string const& type,
                                         std::string const& path);

  // Helper functions
  static void AppendStorageScanners(cmXMLWriter& xml,
                                    cmMakefile const& makefile);
  static void AppendTarget(cmXMLWriter& xml, std::string const& target,
                           std::string const& make,
                           std::string const& makeArguments,
                           std::string const& path, char const* prefix = "",
                           char const* makeTarget = nullptr);
  static void AppendScannerProfile(
    cmXMLWriter& xml, std::string const& profileID, bool openActionEnabled,
    std::string const& openActionFilePath, bool pParserEnabled,
    std::string const& scannerInfoProviderID,
    std::string const& runActionArguments, std::string const& runActionCommand,
    bool runActionUseDefault, bool sipParserEnabled);

  static void AppendLinkedResource(cmXMLWriter& xml, std::string const& name,
                                   std::string const& path, LinkType linkType);

  static void AppendIncludeDirectories(
    cmXMLWriter& xml, std::vector<std::string> const& includeDirs,
    std::set<std::string>& emittedDirs);

  static void AddEnvVar(std::ostream& out, char const* envVar,
                        cmLocalGenerator& lg);

  void WriteGroups(std::vector<cmSourceGroup> const& sourceGroups,
                   std::string& linkName, cmXMLWriter& xml);
  void CreateLinksToSubprojects(cmXMLWriter& xml, std::string const& baseDir);
  void CreateLinksForTargets(cmXMLWriter& xml);

  std::vector<std::string> SrcLinkedResources;
  std::set<std::string> Natures;
  std::string HomeDirectory;
  std::string HomeOutputDirectory;
  bool IsOutOfSourceBuild;
  bool GenerateSourceProject;
  bool GenerateLinkedResources;
  bool SupportsVirtualFolders;
  bool SupportsGmakeErrorParser;
  bool SupportsMachO64Parser;
  bool CEnabled;
  bool CXXEnabled;
};
