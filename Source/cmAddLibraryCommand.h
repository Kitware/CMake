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
#ifndef cmLibrarysCommand_h
#define cmLibrarysCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmLibrarysCommand
 * \brief Defines a list of executables to build.
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmAddLibraryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddLibraryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ADD_LIBRARY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add an library to the project that uses the specified source files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "ADD_LIBRARY(libname [SHARED | STATIC | MODULE] source1 source2 ... sourceN)\n"
      "Adds a library target.  SHARED, STATIC or MODULE keywords are used\n"
      "to set the library type.  If the keywork MODULE appears, the library\n"
      "type is set to MH_BUNDLE on systems which use dyld. Systems without\n"
      "dyld MODULE is treated like SHARED. If no keywords appear as the second\n"
      "argument, the type defaults to the current value of BUILD_SHARED_LIBS.\n"
      "If this variable is not set, the type defaults to STATIC.";
    }
  
  cmTypeMacro(cmAddLibraryCommand, cmCommand);

private:
  std::string m_LibName;
};


#endif
