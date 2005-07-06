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


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

class cmGeneratedFileStream;

/** \class cmCTestCoverageHandler
 * \brief A class that handles coverage computaiton for ctest
 *
 */
class cmCTestCoverageHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestCoverageHandler, cmCTestGenericHandler);

  /*
   * The main entry point for this class
   */
  int ProcessHandler();
  
  cmCTestCoverageHandler();
  
  virtual void Initialize();

private:
  bool ShouldIDoCoverage(const char* file, const char* srcDir,
    const char* binDir);
  bool StartCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);
  void EndCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);

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
    cmCTestCoverage(const cmCTestCoverage& rhs) :
      m_AbsolutePath(rhs.m_AbsolutePath),
      m_FullPath(rhs.m_FullPath),
      m_Covered(rhs.m_Covered),
      m_Tested(rhs.m_Tested),
      m_UnTested(rhs.m_UnTested),
      m_Lines(rhs.m_Lines),
      m_Show(rhs.m_Show)
      {
      }
    cmCTestCoverage& operator=(const cmCTestCoverage& rhs)
      {
      m_AbsolutePath = rhs.m_AbsolutePath;
      m_FullPath = rhs.m_FullPath;
      m_Covered = rhs.m_Covered;
      m_Tested = rhs.m_Tested;
      m_UnTested = rhs.m_UnTested;
      m_Lines = rhs.m_Lines;
      m_Show = rhs.m_Show;
      return *this;
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
