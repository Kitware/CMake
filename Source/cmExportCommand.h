/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportCommand_h
#define cmExportCommand_h

#include "cmCommand.h"

class cmExportBuildFileGenerator;
class cmExportSet;

/** \class cmExportLibraryDependenciesCommand
 * \brief Add a test to the lists of tests to run.
 *
 * cmExportLibraryDependenciesCommand adds a test to the list of tests to run
 *
 */
class cmExportCommand : public cmCommand
{
public:
  cmExportCommand();
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmExportCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "export"; }

  cmTypeMacro(cmExportCommand, cmCommand);

private:
  cmCommandArgumentGroup ArgumentGroup;
  cmCAStringVector Targets;
  cmCAEnabler Append;
  cmCAString ExportSetName;
  cmCAString Namespace;
  cmCAString Filename;
  cmCAEnabler ExportOld;

  cmExportSet* ExportSet;

  friend class cmExportBuildFileGenerator;
  std::string ErrorMessage;

  bool HandlePackage(std::vector<std::string> const& args);
  void StorePackageRegistryWin(std::string const& package, const char* content,
                               const char* hash);
  void StorePackageRegistryDir(std::string const& package, const char* content,
                               const char* hash);
  void ReportRegistryError(std::string const& msg, std::string const& key,
                           long err);
};

#endif
