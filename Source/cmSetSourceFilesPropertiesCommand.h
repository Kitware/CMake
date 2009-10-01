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
  virtual const char* GetName() { return "set_source_files_properties";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Source files can have properties that affect how they are built.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  set_source_files_properties(file1 file2 ...\n"
        "                              PROPERTIES prop1 value1\n"
        "                              prop2 value2 ...)\n"
        "Set properties on a file. The syntax for the command is to list all "
        "the files you want "
        "to change, and then provide the values you want to set next.  You "
        "can make up your own properties as well.  "
        "The following are used by CMake.  "
        "The ABSTRACT flag (boolean) is used by some class wrapping "
        "commands. "
        "If WRAP_EXCLUDE (boolean) is true then many wrapping commands "
        "will ignore this file. If GENERATED (boolean) is true then it "
        "is not an error if this source file does not exist when it is "
        "added to a target.  Obviously, "
        "it must be created (presumably by a custom command) before the "
        "target is built.  "
        "If the HEADER_FILE_ONLY (boolean) property is true then the "
        "file is not compiled.  This is useful if you want to add extra "
        "non build files to an IDE. "
        "OBJECT_DEPENDS (string) adds dependencies to the object file.  "
        "COMPILE_FLAGS (string) is passed to the compiler as additional "
        "command line arguments when the source file is compiled.  "
        "LANGUAGE (string) CXX|C will change the default compiler used "
        "to compile the source file. The languages used need to be enabled " 
        "in the PROJECT command. "
        "If SYMBOLIC (boolean) is set to true the build system will be "
        "informed that the source file is not actually created on disk but "
        "instead used as a symbolic name for a build rule.";
      
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
