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
#ifndef cmConfigureFileCommand_h
#define cmConfigureFileCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmConfigureFileCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmConfigureFileCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CONFIGURE_FILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Copy a file to another location and modify its contents.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  CONFIGURE_FILE(InputFile OutputFile\n"
        "                 [COPYONLY] [ESCAPE_QUOTES]\n"
        "                 [IMMEDIATE] [@ONLY])\n"
        "The Input and Ouput files have to have full paths.  "
        "This command replaces any variables in the input file referenced as "
        "${VAR} or @VAR@ with their values as determined by CMake.  If a "
        "variable is not defined, it will be replaced with nothing.  "
        "If COPYONLY is specified, then then no variable expansion will take "
        "place.  If ESCAPE_QUOTES is specified in then any substitued quotes "
        "will be C-style escaped.  "
        "If IMMEDIATE is specified, then the file will be configured with "
        "the current values of CMake variables instead of waiting until the "
        "end of CMakeLists processing.  If @ONLY is specified, only variables "
        "of the form @VAR@ will be replaces and ${VAR} will be ignored.  "
        "This is useful for configuring tcl scripts that use ${VAR}.";
    }

  virtual void FinalPass();
private:
  void ConfigureFile();
  
  std::string m_InputFile;
  std::string m_OuputFile;
  bool m_CopyOnly;
  bool m_EscapeQuotes;
  bool m_Immediate;
  bool m_AtOnly;
};



#endif
