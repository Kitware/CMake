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

#include <cmsys/RegularExpression.hxx>

class cmGeneratedFileStream;
class cmCTestCoverageHandlerContainer;

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

  /**
   * This method is called when reading CTest custom file
   */
  void PopulateCustomVectors(cmMakefile *mf);

private:
  bool ShouldIDoCoverage(const char* file, const char* srcDir,
    const char* binDir);
  bool StartCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);
  void EndCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);

  int HandleGCovCoverage(cmCTestCoverageHandlerContainer* cont);
  int HandleTracePyCoverage(cmCTestCoverageHandlerContainer* cont);

  std::string FindFile(cmCTestCoverageHandlerContainer* cont,
    std::string fileName);

  struct cmCTestCoverage
    {
    cmCTestCoverage()
      {
      this->AbsolutePath = "";
      this->FullPath = "";
      this->Covered = false;
      this->Tested = 0;
      this->UnTested = 0;
      this->Lines.clear();
      this->Show = false;
      }
    cmCTestCoverage(const cmCTestCoverage& rhs) :
      AbsolutePath(rhs.AbsolutePath),
      FullPath(rhs.FullPath),
      Covered(rhs.Covered),
      Tested(rhs.Tested),
      UnTested(rhs.UnTested),
      Lines(rhs.Lines),
      Show(rhs.Show)
      {
      }
    cmCTestCoverage& operator=(const cmCTestCoverage& rhs)
      {
      this->AbsolutePath = rhs.AbsolutePath;
      this->FullPath = rhs.FullPath;
      this->Covered = rhs.Covered;
      this->Tested = rhs.Tested;
      this->UnTested = rhs.UnTested;
      this->Lines = rhs.Lines;
      this->Show = rhs.Show;
      return *this;
      }
    std::string      AbsolutePath;
    std::string      FullPath;
    bool             Covered;
    int              Tested;
    int              UnTested;
    std::vector<int> Lines;
    bool             Show;
    };

  std::vector<cmStdString> CustomCoverageExclude;
  std::vector<cmsys::RegularExpression> CustomCoverageExcludeRegex;

  typedef std::map<std::string, cmCTestCoverage> CoverageMap;
};

#endif
