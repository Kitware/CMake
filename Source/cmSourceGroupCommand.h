/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSourceGroupCommand_h
#define cmSourceGroupCommand_h

#include "cmCommand.h"

/** \class cmSourceGroupCommand
 * \brief Adds a cmSourceGroup to the cmMakefile.
 *
 * cmSourceGroupCommand is used to define cmSourceGroups which split up
 * source files in to named, organized groups in the generated makefiles.
 */
class cmSourceGroupCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSourceGroupCommand;
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
  virtual const char* GetName() const {return "source_group";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Define a grouping for sources in the makefile.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  source_group(name [REGULAR_EXPRESSION regex] "
      "[FILES src1 src2 ...])\n"
      "Defines a group into which sources will be placed in project files.  "
      "This is mainly used to setup file tabs in Visual Studio.  "
      "Any file whose name is listed or matches the regular expression will "
      "be placed in this group.  If a file matches multiple groups, the LAST "
      "group that explicitly lists the file will be favored, if any.  If no "
      "group explicitly lists the file, the LAST group whose regular "
      "expression matches the file will be favored.\n"
      "The name of the group may contain backslashes to specify subgroups:\n"
      "  source_group(outer\\\\inner ...)\n"
      "For backwards compatibility, this command is also supports the "
      "format:\n"
      "  source_group(name regex)";
    }
  
  cmTypeMacro(cmSourceGroupCommand, cmCommand);
};



#endif
