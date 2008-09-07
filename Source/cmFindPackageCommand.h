/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
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
private:
  void AppendSuccessInformation();
  void AppendToProperty(const char* propertyName);
  void SetModuleVariables(const std::string& components);
  bool FindModule(bool& found);
  bool HandlePackageMode();
  void FindConfig();
  bool FindPrefixedConfig();
  bool FindFrameworkConfig();
  bool FindAppBundleConfig();
  bool ReadListFile(const char* f);
  void StoreVersionFound();

  void ComputePrefixes();
  void AddPrefixesCMakeEnvironment();
  void AddPrefixesCMakeVariable();
  void AddPrefixesSystemEnvironment();
  void AddPrefixesBuilds();
  void AddPrefixesCMakeSystemVariable();
  void AddPrefixesUserGuess();
  void AddPrefixesUserHints();
  void ComputeFinalPrefixes();
  bool SearchDirectory(std::string const& dir);
  bool CheckDirectory(std::string const& dir);
  bool FindConfigFile(std::string const& dir, std::string& file);
  bool FindConfigFileToLoad(std::string const& dir, std::string& file);
  bool CheckVersion(std::string const& config_file);
  bool CheckVersionFile(std::string const& version_file);
  bool SearchPrefix(std::string const& prefix);
  bool SearchFrameworkPrefix(std::string const& prefix_in);
  bool SearchAppBundlePrefix(std::string const& prefix_in);

  friend class cmFindPackageFileList;

  std::string CommandDocumentation;
  cmStdString Name;
  cmStdString Variable;
  cmStdString Version;
  unsigned int VersionMajor;
  unsigned int VersionMinor;
  unsigned int VersionPatch;
  unsigned int VersionCount;
  bool VersionExact;
  cmStdString FileFound;
  cmStdString VersionFound;
  unsigned int VersionFoundMajor;
  unsigned int VersionFoundMinor;
  unsigned int VersionFoundPatch;
  unsigned int VersionFoundCount;
  bool Quiet;
  bool Required;
  bool Compatibility_1_6;
  bool NoModule;
  bool NoBuilds;
  bool DebugMode;
  bool UseLib64Paths;
  std::vector<std::string> Names;
  std::vector<std::string> Configs;
};

#endif
