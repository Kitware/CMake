/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmCTestGenericHandler.h"

class cmGeneratedFileStream;
class cmMakefile;
class cmXMLWriter;

class cmCTestCoverageHandlerContainer
{
public:
  int Error;
  std::string SourceDir;
  std::string BinaryDir;
  using SingleFileCoverageVector = std::vector<int>;
  using TotalCoverageMap = std::map<std::string, SingleFileCoverageVector>;
  TotalCoverageMap TotalCoverage;
  std::ostream* OFS;
  bool Quiet;
};
/** \class cmCTestCoverageHandler
 * \brief A class that handles coverage computation for ctest
 *
 */
class cmCTestCoverageHandler : public cmCTestGenericHandler
{
public:
  using Superclass = cmCTestGenericHandler;

  /*
   * The main entry point for this class
   */
  int ProcessHandler() override;

  cmCTestCoverageHandler();

  void Initialize() override;

  /**
   * This method is called when reading CTest custom file
   */
  void PopulateCustomVectors(cmMakefile* mf) override;

  /** Report coverage only for sources with these labels.  */
  void SetLabelFilter(std::set<std::string> const& labels);

private:
  bool ShouldIDoCoverage(std::string const& file, std::string const& srcDir,
                         std::string const& binDir);
  void CleanCoverageLogFiles(std::ostream& log);
  bool StartCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);
  void EndCoverageLogFile(cmGeneratedFileStream& ostr, int logFileCount);

  void StartCoverageLogXML(cmXMLWriter& xml);
  void EndCoverageLogXML(cmXMLWriter& xml);

  //! Handle coverage using GCC's GCov
  int HandleGCovCoverage(cmCTestCoverageHandlerContainer* cont);
  void FindGCovFiles(std::vector<std::string>& files);

  //! Handle coverage using Intel's LCov
  int HandleLCovCoverage(cmCTestCoverageHandlerContainer* cont);
  bool FindLCovFiles(std::vector<std::string>& files);

  //! Handle coverage using xdebug php coverage
  int HandlePHPCoverage(cmCTestCoverageHandlerContainer* cont);

  //! Handle coverage for Python with coverage.py
  int HandleCoberturaCoverage(cmCTestCoverageHandlerContainer* cont);

  //! Handle coverage for mumps
  int HandleMumpsCoverage(cmCTestCoverageHandlerContainer* cont);

  //! Handle coverage for Jacoco
  int HandleJacocoCoverage(cmCTestCoverageHandlerContainer* cont);

  //! Handle coverage for Delphi (Pascal)
  int HandleDelphiCoverage(cmCTestCoverageHandlerContainer* cont);

  //! Handle coverage for Jacoco
  int HandleBlanketJSCoverage(cmCTestCoverageHandlerContainer* cont);

  //! Handle coverage using Bullseye
  int HandleBullseyeCoverage(cmCTestCoverageHandlerContainer* cont);
  int RunBullseyeSourceSummary(cmCTestCoverageHandlerContainer* cont);
  int RunBullseyeCoverageBranch(cmCTestCoverageHandlerContainer* cont,
                                std::set<std::string>& coveredFileNames,
                                std::vector<std::string>& files,
                                std::vector<std::string>& filesFullPath);

  int RunBullseyeCommand(cmCTestCoverageHandlerContainer* cont,
                         const char* cmd, const char* arg,
                         std::string& outputFile);
  bool ParseBullsEyeCovsrcLine(std::string const& inputLine,
                               std::string& sourceFile, int& functionsCalled,
                               int& totalFunctions, int& percentFunction,
                               int& branchCovered, int& totalBranches,
                               int& percentBranch);
  bool GetNextInt(std::string const& inputLine, std::string::size_type& pos,
                  int& value);
  //! Handle Python coverage using Python's Trace.py
  int HandleTracePyCoverage(cmCTestCoverageHandlerContainer* cont);

  // Find the source file based on the source and build tree. This is used for
  // Trace.py mode, since that one does not tell us where the source file is.
  std::string FindFile(cmCTestCoverageHandlerContainer* cont,
                       std::string const& fileName);

  std::set<std::string> FindUncoveredFiles(
    cmCTestCoverageHandlerContainer* cont);
  std::vector<std::string> CustomCoverageExclude;
  std::vector<cmsys::RegularExpression> CustomCoverageExcludeRegex;
  std::vector<std::string> ExtraCoverageGlobs;

  // Map from source file to label ids.
  class LabelSet : public std::set<int>
  {
  };
  using LabelMapType = std::map<std::string, LabelSet>;
  LabelMapType SourceLabels;
  LabelMapType TargetDirs;

  // Map from label name to label id.
  using LabelIdMapType = std::map<std::string, int>;
  LabelIdMapType LabelIdMap;
  std::vector<std::string> Labels;
  int GetLabelId(std::string const& label);

  // Label reading and writing methods.
  void LoadLabels();
  void LoadLabels(const char* dir);
  void WriteXMLLabels(cmXMLWriter& xml, std::string const& source);

  // Label-based filtering.
  std::set<int> LabelFilter;
  bool IntersectsFilter(LabelSet const& labels);
  bool IsFilteredOut(std::string const& source);
};
