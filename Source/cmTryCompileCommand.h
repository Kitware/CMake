/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmTryCompileCommand_h
#define cmTryCompileCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmTryCompileCommand
 * \brief Specifies where to install some files
 *
 * cmTryCompileCommand is used to test if soucre code can be compiled
 */
class cmTryCompileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmTryCompileCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "TRY_COMPILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Try compiling some code";
    }
  

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "TRY_COMPILE(RESULT_VAR bindir srcdir projectName <CMAKE_FLAGS <Flags>>)\n"
      "Try compiling a program. Return the success or failure in RESULT_VAR "
      "If <target name> is specified then build just that target "
      "otherwise the all or ALL_BUILD target is built.\n"
      "TRY_COMPILE(RESULT_VAR bindir srcfile <CMAKE_FLAGS <Flags>>)\n"
      "Try compiling a srcfile. Return the success or failure in RESULT_VAR.";
    }
  
  cmTypeMacro(cmTryCompileCommand, cmCommand);

};


#endif
