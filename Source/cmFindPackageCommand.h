/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFindPackageCommand_h
#define cmFindPackageCommand_h

#include "cmFindCommon.h"

class cmFindPackageFileList;

/** \class cmFindPackageCommand
 * \brief Load settings from an external project.
 *
 * cmFindPackageCommand
 */
class cmFindPackageCommand : public cmFindCommon
{
public:
  cmFindPackageCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmFindPackageCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "find_package";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Load settings for an external project.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation();

  cmTypeMacro(cmFindPackageCommand, cmFindCommon);
protected:
  virtual void GenerateDocumentation();
private:
  void AppendSuccessInformation();
  void AppendToProperty(const char* propertyName);
  void SetModuleVariables(const std::string& components);
  bool FindModule(bool& found);
  void AddFindDefinition(const char* var, const char* val);
  void RestoreFindDefinitions();
  bool HandlePackageMode();
  bool FindConfig();
  bool FindPrefixedConfig();
  bool FindFrameworkConfig();
  bool FindAppBundleConfig();
  enum PolicyScopeRule { NoPolicyScope, DoPolicyScope };
  bool ReadListFile(const char* f, PolicyScopeRule psr);
  void StoreVersionFound();

  void ComputePrefixes();
  void AddPrefixesCMakeEnvironment();
  void AddPrefixesCMakeVariable();
  void AddPrefixesSystemEnvironment();
  void AddPrefixesUserRegistry();
  void AddPrefixesSystemRegistry();
  void AddPrefixesBuilds();
  void AddPrefixesCMakeSystemVariable();
  void AddPrefixesUserGuess();
  void AddPrefixesUserHints();
  void ComputeFinalPrefixes();
  void LoadPackageRegistryDir(std::string const& dir);
  void LoadPackageRegistryWinUser();
  void LoadPackageRegistryWinSystem();
  void LoadPackageRegistryWin(bool user, unsigned int view);
  bool CheckPackageRegistryEntry(std::istream& is);
  bool SearchDirectory(std::string const& dir);
  bool CheckDirectory(std::string const& dir);
  bool FindConfigFile(std::string const& dir, std::string& file);
  bool CheckVersion(std::string const& config_file);
  bool CheckVersionFile(std::string const& version_file,
                        std::string& result_version);
  bool SearchPrefix(std::string const& prefix);
  bool SearchFrameworkPrefix(std::string const& prefix_in);
  bool SearchAppBundlePrefix(std::string const& prefix_in);

  friend class cmFindPackageFileList;

  struct OriginalDef { bool exists; std::string value; };
  std::map<cmStdString, OriginalDef> OriginalDefs;

  std::string CommandDocumentation;
  cmStdString Name;
  cmStdString Variable;
  cmStdString Version;
  unsigned int VersionMajor;
  unsigned int VersionMinor;
  unsigned int VersionPatch;
  unsigned int VersionTweak;
  unsigned int VersionCount;
  bool VersionExact;
  cmStdString FileFound;
  cmStdString VersionFound;
  unsigned int VersionFoundMajor;
  unsigned int VersionFoundMinor;
  unsigned int VersionFoundPatch;
  unsigned int VersionFoundTweak;
  unsigned int VersionFoundCount;
  bool Quiet;
  bool Required;
  bool Compatibility_1_6;
  bool NoModule;
  bool NoUserRegistry;
  bool NoSystemRegistry;
  bool NoBuilds;
  bool DebugMode;
  bool UseLib64Paths;
  bool PolicyScope;
  std::string LibraryArchitecture;
  std::vector<std::string> Names;
  std::vector<std::string> Configs;
  std::set<std::string> IgnoredPaths;

  struct ConfigFileInfo { std::string filename; std::string version; };
  std::vector<ConfigFileInfo> ConsideredConfigs;
};

#endif
