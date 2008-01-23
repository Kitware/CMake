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
#ifndef cmCTestHandlerCommand_h
#define cmCTestHandlerCommand_h

#include "cmCTestCommand.h"

class cmCTestGenericHandler;

/** \class cmCTestHandler
 * \brief Run a ctest script
 *
 * cmCTestHandlerCommand defineds the command to test the project.
 */
class cmCTestHandlerCommand : public cmCTestCommand
{
public:
  cmCTestHandlerCommand();

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  cmTypeMacro(cmCTestHandlerCommand, cmCTestCommand);

  enum
    {
    ct_NONE,
    ct_RETURN_VALUE,
    ct_BUILD,
    ct_SOURCE,
    ct_SUBMIT_INDEX,
    ct_LAST
    };

protected:
  virtual cmCTestGenericHandler* InitializeHandler() = 0;
  bool ProcessArguments(std::vector<std::string> const& args,
    int last, const char** strings, std::vector<const char*>& values);

  std::string ReturnVariable;
  std::vector<const char*> Arguments;
  std::vector<const char*> Values;
  size_t Last;
};

#endif
