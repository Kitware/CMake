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
#ifndef cmCallVisualStudioMacro_h
#define cmCallVisualStudioMacro_h

#include "cmStandardIncludes.h"

/** \class cmCallVisualStudioMacro
 * \brief Control class for communicating with CMake's Visual Studio macros
 *
 * Find running instances of Visual Studio by full path solution name.
 * Call a Visual Studio IDE macro in any of those instances.
 */
class cmCallVisualStudioMacro
{
public:
  ///! Call the named macro in instances of Visual Studio with the
  ///! given solution file open. Pass "ALL" for slnFile to call the
  ///! macro in each Visual Studio instance.
  static int CallMacro(const std::string& slnFile,
                       const std::string& macro,
                       const std::string& args,
                       const bool logErrorsAsMessages);

  ///! Count the number of running instances of Visual Studio with the
  ///! given solution file open. Pass "ALL" for slnFile to count all
  ///! running Visual Studio instances.
  static int GetNumberOfRunningVisualStudioInstances(
    const std::string& slnFile);

protected:

private:
};

#endif
