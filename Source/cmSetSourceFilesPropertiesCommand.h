/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

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
        "to change, and then provide the values you want to set next. Common boolean properties ABSTRACT, WRAP_EXCLUDE, GENERATED and COMPILE_FLAGS. The first three are boolean properties (use a 1 or 0, TRUE or FALSE) while the COMPILE_FLAGS accepts any string. You can make up your own properties as well.";
    }
  
  cmTypeMacro(cmSetSourceFilesPropertiesCommand, cmCommand);
};



#endif
