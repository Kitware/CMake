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
#ifndef cmSourceFilesFlagsCommand_h
#define cmSourceFilesFlagsCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmSourceFilesFlagsCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmSourceFilesFlagsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SOURCE_FILES_FLAGS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set compile flags for a specific list of files.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "SOURCE_FILES_FLAGS(flags file1 file2 ..)";
    }
  
  cmTypeMacro(cmSourceFilesFlagsCommand, cmCommand);
};



#endif
