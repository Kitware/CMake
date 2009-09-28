/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMarkAsAdvancedCommand_h
#define cmMarkAsAdvancedCommand_h

#include "cmCommand.h"

/** \class cmMarkAsAdvancedCommand
 * \brief MarkAsAdvanced a CMAKE variable
 *
 * cmMarkAsAdvancedCommand sets a variable to a value with expansion.  
 */
class cmMarkAsAdvancedCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmMarkAsAdvancedCommand;
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
  virtual const char* GetName() {return "mark_as_advanced";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Mark cmake cached variables as advanced.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  mark_as_advanced([CLEAR|FORCE] VAR VAR2 VAR...)\n"
      "Mark the named cached variables as advanced.  An advanced variable "
      "will not be displayed in any of the cmake GUIs unless the show "
      "advanced option is on.  "
      "If CLEAR is the first argument advanced variables are changed back "
      "to unadvanced.  "
      "If FORCE is the first argument, then the variable is made advanced.  "
      "If neither FORCE nor CLEAR is specified, new values will be marked as "
      "advanced, but if the variable already has an advanced/non-advanced "
      "state, it will not be changed.\n"
      "It does nothing in script mode.";
    }

  /**
   * This determines if the command is invoked when in script mode.
   * mark_as_advanced() will have no effect in script mode, but this will
   * make many of the modules usable in cmake/ctest scripts, (among them
   * FindUnixMake.cmake used by the CTEST_BUILD command.
  */
  virtual bool IsScriptable() { return true; }

  cmTypeMacro(cmMarkAsAdvancedCommand, cmCommand);
};



#endif
