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
#ifndef cmMacroCommand_h
#define cmMacroCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmFunctionBlocker.h"

/** \class cmMacroFunctionBlocker
 * \brief subclass of function blocker
 *
 * 
 */
class cmMacroFunctionBlocker : public cmFunctionBlocker
{
public:
  cmMacroFunctionBlocker() {m_Executing = false;}
  virtual ~cmMacroFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction&, cmMakefile &mf);
  virtual bool ShouldRemove(const cmListFileFunction&, cmMakefile &mf);
  virtual void ScopeEnded(cmMakefile &mf);
  
  std::vector<std::string> m_Args;
  std::vector<cmListFileFunction> m_Functions;
  bool m_Executing;
};

/** \class cmMacroCommand
 * \brief starts an if block
 *
 * cmMacroCommand starts an if block
 */
class cmMacroCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmMacroCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "MACRO";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "start defining a Macro.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "MACRO(name arg1 arg2 arg3 ...) Starts to define a macro named name that takes arguments named arg1 arg2 arg3...  When the macro is invoked the actual arguments passed replace the formal arguments. ";
    }
  
  cmTypeMacro(cmMacroCommand, cmCommand);
};


#endif
