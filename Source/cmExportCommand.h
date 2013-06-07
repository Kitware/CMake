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
#include "cmDocumentLocationUndefined.h"

class cmExportBuildFileGenerator;

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
  virtual cmCommand* Clone()
    {
    return new cmExportCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "export";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return
      "Export targets from the build tree for use by outside projects.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  export(TARGETS [target1 [target2 [...]]] [NAMESPACE <namespace>]\n"
      "         [APPEND] FILE <filename>)\n"
      "Create a file <filename> that may be included by outside projects to "
      "import targets from the current project's build tree.  "
      "This is useful during cross-compiling to build utility executables "
      "that can run on the host platform in one project and then import "
      "them into another project being compiled for the target platform.  "
      "If the NAMESPACE option is given the <namespace> string will be "
      "prepended to all target names written to the file.  "
      "If the APPEND option is given the generated code will be appended "
      "to the file instead of overwriting it.  "
      "If a library target is included in the export but "
      "a target to which it links is not included the behavior is "
      "unspecified."
      "\n"
      "The file created by this command is specific to the build tree and "
      "should never be installed.  "
      "See the install(EXPORT) command to export targets from an "
      "installation tree."
      CM_LOCATION_UNDEFINED_BEHAVIOR("passing it to this command")
      "\n"
      "  export(PACKAGE <name>)\n"
      "Store the current build directory in the CMake user package registry "
      "for package <name>.  "
      "The find_package command may consider the directory while searching "
      "for package <name>.  "
      "This helps dependent projects find and use a package from the "
      "current project's build tree without help from the user.  "
      "Note that the entry in the package registry that this command "
      "creates works only in conjunction with a package configuration "
      "file (<name>Config.cmake) that works with the build tree."
      ;
    }

  cmTypeMacro(cmExportCommand, cmCommand);

private:
  cmCommandArgumentGroup ArgumentGroup;
  cmCAStringVector Targets;
  cmCAEnabler Append;
  cmCAString Namespace;
  cmCAString Filename;

  friend class cmExportBuildFileGenerator;
  std::string ErrorMessage;

  bool HandlePackage(std::vector<std::string> const& args);
  void StorePackageRegistryWin(std::string const& package,
                               const char* content, const char* hash);
  void StorePackageRegistryDir(std::string const& package,
                               const char* content, const char* hash);
  void ReportRegistryError(std::string const& msg, std::string const& key,
                           long err);
};


#endif
