/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCTestTestHandler.h"

class cmMakefile;
class cmXMLWriter;

/** \class cmCTestMemCheckHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestMemCheckHandler : public cmCTestTestHandler
{
  friend class cmCTestRunTest;

public:
  using Superclass = cmCTestTestHandler;

  void PopulateCustomVectors(cmMakefile* mf) override;

  cmCTestMemCheckHandler();

  void Initialize() override;

  int GetDefectCount() const;

protected:
  int PreProcessHandler() override;
  int PostProcessHandler() override;
  void GenerateTestCommand(std::vector<std::string>& args, int test) override;

private:
  enum
  { // Memory checkers
    UNKNOWN = 0,
    VALGRIND,
    PURIFY,
    DRMEMORY,
    BOUNDS_CHECKER,
    // checkers after here do not use the standard error list
    CUDA_SANITIZER,
    ADDRESS_SANITIZER,
    LEAK_SANITIZER,
    THREAD_SANITIZER,
    MEMORY_SANITIZER,
    UB_SANITIZER
  };

public:
  enum
  { // Memory faults
    ABR = 0,
    ABW,
    ABWL,
    COR,
    EXU,
    FFM,
    FIM,
    FMM,
    FMR,
    FMW,
    FUM,
    IPR,
    IPW,
    MAF,
    MLK,
    MPK,
    NPR,
    ODS,
    PAR,
    PLK,
    UMC,
    UMR,
    NO_MEMORY_FAULT
  };

private:
  enum
  { // Program statuses
    NOT_RUN = 0,
    TIMEOUT,
    SEGFAULT,
    ILLEGAL,
    INTERRUPT,
    NUMERICAL,
    OTHER_FAULT,
    FAILED,
    BAD_COMMAND,
    COMPLETED
  };
  std::string BoundsCheckerDPBDFile;
  std::string BoundsCheckerXMLFile;
  std::string MemoryTester;
  std::vector<std::string> MemoryTesterDynamicOptions;
  std::vector<std::string> MemoryTesterOptions;
  int MemoryTesterStyle;
  std::string MemoryTesterOutputFile;
  std::string MemoryTesterEnvironmentVariable;
  // these are used to store the types of errors that can show up
  std::vector<std::string> ResultStrings;
  std::vector<std::string> ResultStringsLong;
  std::vector<int> GlobalResults;
  bool LogWithPID; // does log file add pid
  int DefectCount;

  std::vector<int>::size_type FindOrAddWarning(const std::string& warning);
  // initialize the ResultStrings and ResultStringsLong for
  // this type of checker
  void InitializeResultsVectors();

  //! Initialize memory checking subsystem.
  bool InitializeMemoryChecking();

  /**
   * Generate CTest DynamicAnalysis.xml files
   */
  void GenerateCTestXML(cmXMLWriter& xml) override;

  std::vector<std::string> CustomPreMemCheck;
  std::vector<std::string> CustomPostMemCheck;

  //! Parse Valgrind/Purify/Bounds Checker result out of the output
  // string. After running, log holds the output and results hold the
  // different memory errors.
  bool ProcessMemCheckOutput(const std::string& str, std::string& log,
                             std::vector<int>& results);
  bool ProcessMemCheckValgrindOutput(const std::string& str, std::string& log,
                                     std::vector<int>& results);
  bool ProcessMemCheckDrMemoryOutput(const std::string& str, std::string& log,
                                     std::vector<int>& results);
  bool ProcessMemCheckPurifyOutput(const std::string& str, std::string& log,
                                   std::vector<int>& results);
  bool ProcessMemCheckCudaOutput(const std::string& str, std::string& log,
                                 std::vector<int>& results);
  bool ProcessMemCheckSanitizerOutput(const std::string& str, std::string& log,
                                      std::vector<int>& results);
  bool ProcessMemCheckBoundsCheckerOutput(const std::string& str,
                                          std::string& log,
                                          std::vector<int>& results);

  void PostProcessTest(cmCTestTestResult& res, int test);
  void PostProcessBoundsCheckerTest(cmCTestTestResult& res, int test);
  void PostProcessDrMemoryTest(cmCTestTestResult& res, int test);

  //! append MemoryTesterOutputFile to the test log
  void AppendMemTesterOutput(cmCTestTestHandler::cmCTestTestResult& res,
                             std::string const& filename);

  //! generate the output filename for the given test index
  void TestOutputFileNames(int test, std::vector<std::string>& files);
};
