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

#ifndef cmCTestUpdateHandler_h
#define cmCTestUpdateHandler_h


#include "cmStandardIncludes.h"
#include "cmListFileCache.h"

class cmCTest;

/** \class cmCTestUpdateHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestUpdateHandler
{
public:

  /*
   * The main entry point for this class
   */
  int UpdateDirectory(cmCTest *);
  
  /*
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { m_Verbose = val; }
  
  cmCTestUpdateHandler();
  
private:
  bool m_Verbose;
  cmCTest *m_CTest;

  // Some structures needed for cvs update
  struct StringPair : 
    public std::pair<std::string, std::string>{};
  struct UpdateFiles : public std::vector<StringPair>{};
  struct AuthorsToUpdatesMap : 
    public std::map<std::string, UpdateFiles>{};
};

#endif
