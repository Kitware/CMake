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
#ifndef cmExportLibraryDependenciesCommand_h
#define cmExportLibraryDependenciesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmExportLibraryDependenciesCommand
 * \brief Add a test to the lists of tests to run.
 *
 * cmExportLibraryDependenciesCommand adds a test to the list of tests to run .
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
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. 
   */
  virtual void FinalPass();

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "EXPORT_LIBRARY_DEPENDENCIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Write out the dependency information for all targets of a project.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  EXPORT_LIBRARY_DEPENDENCIES(FILE [APPEND])\n"
      "Create a file that can be included into a CMake listfile with the "
      "INCLUDE command.  The file will contain a number of SET commands "
      "that will set all the variables needed for library dependency "
      "information.  This should be the last command in the top level "
      "CMakeLists.txt file of the project.  If the APPEND option is "
      "specified, the SET commands will be appended to the given file "
      "instead of replacing it.";
    }
  
  cmTypeMacro(cmExportLibraryDependenciesCommand, cmCommand);

private:
  std::vector<std::string> m_Args;
};


#endif
