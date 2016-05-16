/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2007 Miguel A. Figueroa-Villanueva

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExtraEclipseCDT4Generator_h
#define cmExtraEclipseCDT4Generator_h

#include "cmExternalMakefileProjectGenerator.h"

class cmMakefile;
class cmXMLWriter;
class cmSourceGroup;

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

  static cmExternalMakefileProjectGenerator* New()
  {
    return new cmExtraEclipseCDT4Generator;
  }

  virtual std::string GetName() const
  {
    return cmExtraEclipseCDT4Generator::GetActualName();
  }

  static std::string GetActualName() { return "Eclipse CDT4"; }

  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const std::string& fullName) const;
  virtual void EnableLanguage(std::vector<std::string> const& languages,
                              cmMakefile*, bool optional);

  virtual void Generate();

private:
  // create .project file in the source tree
  void CreateSourceProjectFile();

  // create .project file
  void CreateProjectFile();

  // create .cproject file
  void CreateCProjectFile() const;

  // If built with cygwin cmake, convert posix to windows path.
  static std::string GetEclipsePath(const std::string& path);

  // Extract basename.
  static std::string GetPathBasename(const std::string& path);

  // Generate the project name as: <name>-<type>@<path>
  static std::string GenerateProjectName(const std::string& name,
                                         const std::string& type,
                                         const std::string& path);

  // Helper functions
  static void AppendStorageScanners(cmXMLWriter& xml,
                                    const cmMakefile& makefile);
  static void AppendTarget(cmXMLWriter& xml, const std::string& target,
                           const std::string& make,
                           const std::string& makeArguments,
                           const std::string& path, const char* prefix = "",
                           const char* makeTarget = NULL);
  static void AppendScannerProfile(
    cmXMLWriter& xml, const std::string& profileID, bool openActionEnabled,
    const std::string& openActionFilePath, bool pParserEnabled,
    const std::string& scannerInfoProviderID,
    const std::string& runActionArguments, const std::string& runActionCommand,
    bool runActionUseDefault, bool sipParserEnabled);

  static void AppendLinkedResource(cmXMLWriter& xml, const std::string& name,
                                   const std::string& path, LinkType linkType);

  static void AppendIncludeDirectories(
    cmXMLWriter& xml, const std::vector<std::string>& includeDirs,
    std::set<std::string>& emittedDirs);

  static void AddEnvVar(std::ostream& out, const char* envVar,
                        cmLocalGenerator* lg);

  void WriteGroups(std::vector<cmSourceGroup> const& sourceGroups,
                   std::string& linkName, cmXMLWriter& xml);
  void CreateLinksToSubprojects(cmXMLWriter& xml, const std::string& baseDir);
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

#endif
