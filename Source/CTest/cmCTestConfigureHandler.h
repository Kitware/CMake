/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCTestConfigureHandler_h
#define cmCTestConfigureHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

/** \class cmCTestConfigureHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestConfigureHandler : public cmCTestGenericHandler
{
public:

  /*
   * The main entry point for this class
   */
  int ProcessHandler();
  
  cmCTestConfigureHandler();
};

#endif
