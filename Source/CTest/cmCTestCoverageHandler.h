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

  /** Report coverage only for sources with these labels.  */
  void SetLabelFilter(std::set<cmStdString> const& labels);

private:
  bool ShouldIDoCoverage(const char* file, const char* srcDir,
    const char* binDir);
  bool StartCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);
  void EndCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);

  //! Handle coverage using GCC's GCov
  int HandleGCovCoverage(cmCTestCoverageHandlerContainer* cont);

  //! Handle coverage using Bullseye
  int HandleBullseyeCoverage(cmCTestCoverageHandlerContainer* cont);
  int RunBullseyeSourceSummary(cmCTestCoverageHandlerContainer* cont);
  int RunBullseyeCoverageBranch(cmCTestCoverageHandlerContainer* cont,
                                std::set<cmStdString>& coveredFileNames,
                                std::vector<std::string>& files,
                                std::vector<std::string>& filesFullPath);
  int RunBullseyeCommand(
    cmCTestCoverageHandlerContainer* cont,
    const char* cmd,
    const char* arg,
    std::string& outputFile);
  bool ParseBullsEyeCovsrcLine(
    std::string const& inputLine,
    std::string& sourceFile,
    int& functionsCalled,
    int& totalFunctions,
    int& percentFunction,
    int& branchCovered,
    int& totalBranches,
    int& percentBranch);
  bool GetNextInt(std::string const& inputLine,
                  std::string::size_type& pos,
                  int& value);
  //! Handle Python coverage using Python's Trace.py
  int HandleTracePyCoverage(cmCTestCoverageHandlerContainer* cont);

  // Find the source file based on the source and build tree. This is used for
  // Trace.py mode, since that one does not tell us where the source file is.
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

  // Map from source file to label ids.
  class LabelSet: public std::set<int> {};
  typedef std::map<cmStdString, LabelSet> LabelMapType;
  LabelMapType SourceLabels;

  // Map from label name to label id.
  typedef std::map<cmStdString, int> LabelIdMapType;
  LabelIdMapType LabelIdMap;
  std::vector<std::string> Labels;
  int GetLabelId(std::string const& label);

  // Load reading and writing methods.
  void LoadLabels();
  void LoadLabels(const char* fname);
  void WriteXMLLabels(std::ofstream& os, std::string const& source);

  // Label-based filtering.
  std::set<int> LabelFilter;
  bool IsFilteredOut(std::string const& source);
};

#endif
