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

#ifndef cmCTestGenericHandler_h
#define cmCTestGenericHandler_h


#include "cmStandardIncludes.h"

class cmCTest;
class cmMakefile;

/** \class cmCTestGenericHandler
 * \brief A superclass of all CTest Handlers
 *
 */
class cmCTestGenericHandler
{
public:
  /**
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { m_Verbose = val; }

  /**
   * Populate internals from CTest custom scripts
   */
  void PopulateCustomVectors(cmMakefile *) {}

  /**
   * Set the CTest instance
   */
  void SetCTestInstance(cmCTest* ctest) { m_CTest = ctest; }

  /**
   * Construct handler
   */
  cmCTestGenericHandler();

protected:
  bool m_Verbose;
  cmCTest *m_CTest;
};

#endif

