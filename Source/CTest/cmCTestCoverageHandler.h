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

#ifndef cmCTestCoverageHandler_h
#define cmCTestCoverageHandler_h


#include "cmStandardIncludes.h"
#include "cmListFileCache.h"

class cmCTest;

/** \class cmCTestCoverageHandler
 * \brief A class that handles coverage computaiton for ctest
 *
 */
class cmCTestCoverageHandler
{
public:

  /*
   * The main entry point for this class
   */
  int CoverageDirectory(cmCTest *);
  
  /*
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { m_Verbose = val; }
  
  cmCTestCoverageHandler();
  
private:
  bool m_Verbose;
  cmCTest *m_CTest;
  bool ShouldIDoCoverage(const char* file, const char* srcDir,
    const char* binDir, bool verbose);
  bool StartLogFile(std::ofstream& ostr, int logFileCount);
  void StopLogFile(std::ofstream& ostr, int logFileCount);

  struct cmCTestCoverage
    {
    cmCTestCoverage()
      {
      m_AbsolutePath = "";
      m_FullPath = "";
      m_Covered = false;
      m_Tested = 0;
      m_UnTested = 0;
      m_Lines.clear();
      m_Show = false;
      }
    std::string      m_AbsolutePath;
    std::string      m_FullPath;
    bool             m_Covered;
    int              m_Tested;
    int              m_UnTested;
    std::vector<int> m_Lines;
    bool             m_Show;
    };

  typedef std::map<std::string, cmCTestCoverage> tm_CoverageMap;
};

#endif
