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
#ifndef cmSetSourceFilesPropertiesCommand_h
#define cmSetSourceFilesPropertiesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmSetSourceFilesPropertiesCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmSetSourceFilesPropertiesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SET_SOURCE_FILES_PROPERTIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set attributes for a specific list of files.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "SET_SOURCE_FILES_PROPERTIES(file1 file2 .. filen PROPERTIES prop1 value1 prop2 value2 ... prop2 valuen)"
        "Set properties on a file. The syntax for the command is to list all the files you want "
        "to change, and then provide the values you want to set next.  You can make up your own properties as well.  "
        "The following are used by CMake.  "
        "The ABSTRACT flag (boolean) appears to have some effect on the VTK wrapper commands.  "
        "If WRAP_EXCLUDE (boolean) is true then the wrapping commands (FLTKWrapUI, QTWrapCC, QTWrapUI, VTKMakeInstantiator, VTKWrapJava, VTKWrapPython, and VTKWrapTcl) will ignore this file.  "
        "If GENERATED (boolean) is true then it is not an error if this source file does not exist when it is added to a target.  Obviously, it must be created (presumably by a custom command) before the target is built.  "
        "If the HEADER_FILE_ONLY (boolean) property is true then dependency information is not created for that file (this is set automatically, based on the file's name's extension and is probably only used by Makefiles).  "
        "OBJECT_DEPENDS (string) adds dependencies to the object file.  "
        "COMPILE_FLAGS (string) is passed to the compiler as additional command line arguments when the source file is compiled.  ";
      
    }
  
  cmTypeMacro(cmSetSourceFilesPropertiesCommand, cmCommand);
};



#endif
