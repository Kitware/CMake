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
      "  target_include_directories(<target> [BEFORE] "
      "<INTERFACE|PUBLIC|PRIVATE> [items1...]\n"
      "    [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])\n"
      "Specify include directories or targets to use when compiling a given "
      "target.  "
      "The named <target> must have been created by a command such as "
      "add_executable or add_library.\n"
      "If BEFORE is specified, the content will be prepended to the property "
      "instead of being appended.\n"
      "The INTERFACE, PUBLIC and PRIVATE keywords are required to specify "
      "the scope of the following arguments.  PRIVATE and PUBLIC items will "
      "populate the INCLUDE_DIRECTORIES property of <target>.  PUBLIC and "
      "INTERFACE items will populate the INTERFACE_INCLUDE_DIRECTORIES "
      "property of <target>.   "
      "The non-scope arguments specify either include directories or targets "
      "to use INTERFACE_INCLUDE_DIRECTORIES from.  Any specified include "
      "directories must be absolute paths, not relative paths.  "
      "Repeated calls for the same <target> append items in the order called."
      "\n"
      ;
    }

  cmTypeMacro(cmTargetIncludeDirectoriesCommand, cmCommand);

private:
  virtual void HandleImportedTargetInvalidScope(const std::string &tgt,
                                   const std::string &scope);
  virtual void HandleMissingTarget(const std::string &name);

  virtual bool HandleNonTargetArg(std::string &content,
                          const std::string &sep,
                          const std::string &entry,
                          const std::string &tgt);

  virtual void HandleDirectContent(cmTarget *tgt, const std::string &content,
                                   bool prepend);
};

#endif
