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
#ifndef cmWrapExcludeFilesCommand_h
#define cmWrapExcludeFilesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmWrapExcludeFilesCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmWrapExcludeFilesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "WRAP_EXCLUDE_FILES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "A list of classes, to exclude from wrapping.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "WRAP_EXCLUDE_FILES(file1 file2 ..)";
    }
  
  cmTypeMacro(cmWrapExcludeFilesCommand, cmCommand);
};



#endif
