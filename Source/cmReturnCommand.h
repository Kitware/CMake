/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmReturnCommand_h
#define cmReturnCommand_h

#include "cmCommand.h"

/** \class cmReturnCommand
 * \brief Return from a directory or function
 *
 * cmReturnCommand returns from a directory or function
 */
class cmReturnCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmReturnCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "return";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Return from a file, directory or function.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  return()\n"
      "Returns from a file, directory or function. When this command is "
      "encountered in an included file (via include() or find_package()), "
      "it causes processing of the current file to stop and control is "
      "returned to the including file. If it is encountered in a file which "
      "is not included by another file, e.g. a CMakeLists.txt, control is "
      "returned to the parent directory if there is one. "
      "If return is called in a function, control is returned to the caller "
      "of the function. Note that a macro "
      "is not a function and does not handle return like a function does.";
    }
  
  cmTypeMacro(cmReturnCommand, cmCommand);
};



#endif
