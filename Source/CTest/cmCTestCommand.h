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
#ifndef cmCTestCommand_h
#define cmCTestCommand_h

#include "cmCommand.h"

class cmCTest;
class cmCTestScriptHandler;

/** \class cmCTestCommand
 * \brief A superclass for all commands added to the CTestScriptHandler
 *
 * cmCTestCommand is the superclass for all commands that will be added to
 * the ctest script handlers parser.
 * 
 */
class cmCTestCommand : public cmCommand
{
public:

  cmCTestCommand() {m_CTest = 0; m_CTestScriptHandler = 0;}
  
  cmCTest *m_CTest;
  cmCTestScriptHandler *m_CTestScriptHandler;
  
  cmTypeMacro(cmCTestCommand, cmCommand);

};


#endif
