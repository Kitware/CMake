/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAddCompileOptionsCommand_h
#define cmAddCompileOptionsCommand_h

#include "cmCommand.h"
#include "cmDocumentGeneratorExpressions.h"

class cmAddCompileOptionsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmAddCompileOptionsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const {return "add_compile_options";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Adds options to the compilation of source files.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  add_compile_options(<option> ...)\n"
      "Adds options to the compiler command line for sources in the "
      "current directory and below.  This command can be used to add any "
      "options, but alternative commands exist to add preprocessor "
      "definitions or include directories.  "
      "See documentation of the directory and target COMPILE_OPTIONS "
      "properties for details.  "
      "Arguments to add_compile_options may use \"generator "
      "expressions\" with the syntax \"$<...>\".  "
      CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS
      ;
    }

  cmTypeMacro(cmAddCompileOptionsCommand, cmCommand);
};

#endif
