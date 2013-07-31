/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmTargetIncludeDirectoriesCommand_h
#define cmTargetIncludeDirectoriesCommand_h

#include "cmTargetPropCommandBase.h"

//----------------------------------------------------------------------------
class cmTargetIncludeDirectoriesCommand : public cmTargetPropCommandBase
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmTargetIncludeDirectoriesCommand;
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
  virtual const char* GetName() const { return "target_include_directories";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return
      "Add include directories to a target.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  target_include_directories(<target> [SYSTEM] [BEFORE] "
      "<INTERFACE|PUBLIC|PRIVATE> [items1...]\n"
      "    [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])\n"
      "Specify include directories or targets to use when compiling a given "
      "target.  "
      "The named <target> must have been created by a command such as "
      "add_executable or add_library and must not be an IMPORTED target.\n"
      "If BEFORE is specified, the content will be prepended to the property "
      "instead of being appended.\n"
      "The INTERFACE, PUBLIC and PRIVATE keywords are required to specify "
      "the scope of the following arguments.  PRIVATE and PUBLIC items will "
      "populate the INCLUDE_DIRECTORIES property of <target>.  PUBLIC and "
      "INTERFACE items will populate the INTERFACE_INCLUDE_DIRECTORIES "
      "property of <target>.   "
      "The following arguments specify include directories.  Specified "
      "include directories may be absolute paths or relative paths.  "
      "Repeated calls for the same <target> append items in the order called."
      "If SYSTEM is specified, the compiler will be told the "
      "directories are meant as system include directories on some "
      "platforms (signalling this setting might achieve effects such as "
      "the compiler skipping warnings, or these fixed-install system files "
      "not being considered in dependency calculations - see compiler "
      "docs).  If SYSTEM is used together with PUBLIC or INTERFACE, the "
      "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES target property will be "
      "populated with the specified directories."
      "\n"
      "Arguments to target_include_directories may use \"generator "
      "expressions\" with the syntax \"$<...>\".  "
      CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS
      ;
    }

  cmTypeMacro(cmTargetIncludeDirectoriesCommand, cmTargetPropCommandBase);

private:
  virtual void HandleImportedTarget(const std::string &tgt);
  virtual void HandleMissingTarget(const std::string &name);

  virtual void HandleDirectContent(cmTarget *tgt,
                                   const std::vector<std::string> &content,
                                   bool prepend, bool system);
  virtual void HandleInterfaceContent(cmTarget *tgt,
                                   const std::vector<std::string> &content,
                                   bool prepend, bool system);

  virtual std::string Join(const std::vector<std::string> &content);
};

#endif
