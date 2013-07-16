/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmTargetCompileOptionsCommand_h
#define cmTargetCompileOptionsCommand_h

#include "cmTargetPropCommandBase.h"

class cmTargetCompileOptionsCommand : public cmTargetPropCommandBase
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmTargetCompileOptionsCommand;
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
  virtual const char* GetName() const { return "target_compile_options";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return
      "Add compile options to a target.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  target_compile_options(<target> [BEFORE] "
      "<INTERFACE|PUBLIC|PRIVATE> [items1...]\n"
      "    [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])\n"
      "Specify compile options to use when compiling a given target.  "
      "The named <target> must have been created by a command such as "
      "add_executable or add_library and must not be an IMPORTED target.  "
      "If BEFORE is specified, the content will be prepended to the property "
      "instead of being appended.\n"
      "The INTERFACE, PUBLIC and PRIVATE keywords are required to specify "
      "the scope of the following arguments.  PRIVATE and PUBLIC items will "
      "populate the COMPILE_OPTIONS property of <target>.  PUBLIC and "
      "INTERFACE items will populate the INTERFACE_COMPILE_OPTIONS "
      "property of <target>.   "
      "The following arguments specify compile opitions.  "
      "Repeated calls for the same <target> append items in the order called."
      "\n"
      "Arguments to target_compile_options may use \"generator "
      "expressions\" with the syntax \"$<...>\".  "
      CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS
      ;
    }

  cmTypeMacro(cmTargetCompileOptionsCommand, cmTargetPropCommandBase);

private:
  virtual void HandleImportedTarget(const std::string &tgt);
  virtual void HandleMissingTarget(const std::string &name);

  virtual void HandleDirectContent(cmTarget *tgt,
                                   const std::vector<std::string> &content,
                                   bool prepend, bool system);
  virtual std::string Join(const std::vector<std::string> &content);
};

#endif
