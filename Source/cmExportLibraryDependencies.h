/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportLibraryDependenciesCommand_h
#define cmExportLibraryDependenciesCommand_h

#include "cmCommand.h"

/** \class cmExportLibraryDependenciesCommand
 * \brief Add a test to the lists of tests to run.
 *
 * cmExportLibraryDependenciesCommand adds a test to the list of tests to run
 * 
 */
class cmExportLibraryDependenciesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmExportLibraryDependenciesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. 
   */
  virtual void FinalPass();
  virtual bool HasFinalPass() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "export_library_dependencies";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Deprecated.  Use INSTALL(EXPORT) or EXPORT command.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "This command generates an old-style library dependencies file.  "
      "Projects requiring CMake 2.6 or later should not use the command.  "
      "Use instead the install(EXPORT) command to help export targets "
      "from an installation tree and the export() command to export targets "
      "from a build tree.\n"
      "The old-style library dependencies file does not take into account "
      "per-configuration names of libraries or the LINK_INTERFACE_LIBRARIES "
      "target property.\n"
      "  export_library_dependencies(<file> [APPEND])\n"
      "Create a file named <file> that can be included into a CMake listfile "
      "with the INCLUDE command.  The file will contain a number of SET "
      "commands that will set all the variables needed for library dependency "
      "information.  This should be the last command in the top level "
      "CMakeLists.txt file of the project.  If the APPEND option is "
      "specified, the SET commands will be appended to the given file "
      "instead of replacing it.";
    }

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }
  
  cmTypeMacro(cmExportLibraryDependenciesCommand, cmCommand);

private:
  std::string Filename;
  bool Append;
  void ConstFinalPass() const;
};


#endif
