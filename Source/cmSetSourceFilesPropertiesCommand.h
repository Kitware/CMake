/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSetSourceFilesPropertiesCommand_h
#define cmSetSourceFilesPropertiesCommand_h

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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "set_source_files_properties";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Source files can have properties that affect how they are built.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
      return
        "  set_source_files_properties([file1 [file2 [...]]]\n"
        "                              PROPERTIES prop1 value1\n"
        "                              [prop2 value2 [...]])\n"
        "Set properties associated with source files using a key/value "
        "paired list.  "
        "See properties documentation for those known to CMake.  "
        "Unrecognized properties are ignored.  "
        "Source file properties are visible only to targets "
        "added in the same directory (CMakeLists.txt).";
    }
  
  cmTypeMacro(cmSetSourceFilesPropertiesCommand, cmCommand);

  static bool RunCommand(cmMakefile *mf,
                         std::vector<std::string>::const_iterator filebeg,
                         std::vector<std::string>::const_iterator fileend,
                         std::vector<std::string>::const_iterator propbeg,
                         std::vector<std::string>::const_iterator propend,
                         std::string &errors);
};



#endif
