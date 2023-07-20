/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestTypes.h" // IWYU pragma: keep
#include "cmDuration.h"
#include "cmListFileCache.h"
#include "cmValue.h"

class cmMakefile;
class cmXMLWriter;

/** \class cmCTestTestHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestTestHandler : public cmCTestGenericHandler
{
  friend class cmCTest;
  friend class cmCTestRunTest;
  friend class cmCTestMultiProcessHandler;

public:
  using Superclass = cmCTestGenericHandler;

  /**
   * The main entry point for this class
   */
  int ProcessHandler() override;

  /**
   * When both -R and -I are used should the resulting test list be the
   * intersection or the union of the lists. By default it is the
   * intersection.
   */
  void SetUseUnion(bool val) { this->UseUnion = val; }

  /**
   * Set whether or not CTest should only execute the tests that failed
   * on the previous run.  By default this is false.
   */
  void SetRerunFailed(bool val) { this->RerunFailed = val; }

  /**
   * This method is called when reading CTest custom file
   */
  void PopulateCustomVectors(cmMakefile* mf) override;

  //! Control the use of the regular expresisons, call these methods to turn
  /// them on
  void UseIncludeRegExp();
  void UseExcludeRegExp();
  void SetIncludeRegExp(const std::string&);
  void SetExcludeRegExp(const std::string&);

  void SetMaxIndex(int n) { this->MaxIndex = n; }
  int GetMaxIndex() { return this->MaxIndex; }

  void SetTestOutputSizePassed(int n)
  {
    this->CustomMaximumPassedTestOutputSize = n;
  }
  void SetTestOutputSizeFailed(int n)
  {
    this->CustomMaximumFailedTestOutputSize = n;
  }

  //! Set test output truncation mode. Return false if unknown mode.
  bool SetTestOutputTruncation(const std::string& mode);

  //! pass the -I argument down
  void SetTestsToRunInformation(cmValue);

  cmCTestTestHandler();

  /*
   * Add the test to the list of tests to be executed
   */
  bool AddTest(const std::vector<std::string>& args);

  /*
   * Set tests properties
   */
  bool SetTestsProperties(const std::vector<std::string>& args);

  /**
   * Set directory properties
   */
  bool SetDirectoryProperties(const std::vector<std::string>& args);

  void Initialize() override;

  struct cmCTestTestResourceRequirement
  {
    std::string ResourceType;
    int SlotsNeeded;
    int UnitsNeeded;

    bool operator==(const cmCTestTestResourceRequirement& other) const;
    bool operator!=(const cmCTestTestResourceRequirement& other) const;
  };

  struct Signal
  {
    int Number = 0;
    std::string Name;
  };

  struct cmCTestTestProperties
  {
    void AppendError(cm::string_view err);
    cm::optional<std::string> Error;
    std::string Name;
    std::string Directory;
    std::vector<std::string> Args;
    std::vector<std::string> RequiredFiles;
    std::vector<std::string> Depends;
    std::vector<std::string> AttachedFiles;
    std::vector<std::string> AttachOnFail;
    std::vector<std::pair<cmsys::RegularExpression, std::string>>
      ErrorRegularExpressions;
    std::vector<std::pair<cmsys::RegularExpression, std::string>>
      RequiredRegularExpressions;
    std::vector<std::pair<cmsys::RegularExpression, std::string>>
      SkipRegularExpressions;
    std::vector<std::pair<cmsys::RegularExpression, std::string>>
      TimeoutRegularExpressions;
    std::map<std::string, std::string> Measurements;
    bool IsInBasedOnREOptions = true;
    bool WillFail = false;
    bool Disabled = false;
    float Cost = 0;
    int PreviousRuns = 0;
    bool RunSerial = false;
    cm::optional<cmDuration> Timeout;
    cm::optional<Signal> TimeoutSignal;
    cm::optional<cmDuration> TimeoutGracePeriod;
    cmDuration AlternateTimeout;
    int Index = 0;
    // Requested number of process slots
    int Processors = 1;
    bool WantAffinity = false;
    std::vector<size_t> Affinity;
    // return code of test which will mark test as "not run"
    int SkipReturnCode = -1;
    std::vector<std::string> Environment;
    std::vector<std::string> EnvironmentModification;
    std::vector<std::string> Labels;
    std::set<std::string> LockedResources;
    std::set<std::string> FixturesSetup;
    std::set<std::string> FixturesCleanup;
    std::set<std::string> FixturesRequired;
    std::set<std::string> RequireSuccessDepends;
    std::vector<std::vector<cmCTestTestResourceRequirement>> ResourceGroups;
    std::string GeneratedResourceSpecFile;
    // Private test generator properties used to track backtraces
    cmListFileBacktrace Backtrace;
  };

  struct cmCTestTestResult
  {
    std::string Name;
    std::string Path;
    std::string Reason;
    std::string FullCommandLine;
    std::string Environment;
    cmDuration ExecutionTime = cmDuration::zero();
    std::int64_t ReturnValue = 0;
    int Status = NOT_RUN;
    std::string ExceptionStatus;
    bool CompressOutput;
    std::string CompletionStatus;
    std::string CustomCompletionStatus;
    std::string Output;
    std::string TestMeasurementsOutput;
    int TestCount = 0;
    cmCTestTestProperties* Properties = nullptr;
  };

  struct cmCTestTestResultLess
  {
    bool operator()(const cmCTestTestResult& lhs,
                    const cmCTestTestResult& rhs) const
    {
      return lhs.TestCount < rhs.TestCount;
    }
  };

  // add configurations to a search path for an executable
  static void AddConfigurations(cmCTest* ctest,
                                std::vector<std::string>& attempted,
                                std::vector<std::string>& attemptedConfigs,
                                std::string filepath, std::string& filename);

  // full signature static method to find an executable
  static std::string FindExecutable(cmCTest* ctest,
                                    const std::string& testCommand,
                                    std::string& resultingConfig,
                                    std::vector<std::string>& extraPaths,
                                    std::vector<std::string>& failed);

  static bool ParseResourceGroupsProperty(
    const std::string& val,
    std::vector<std::vector<cmCTestTestResourceRequirement>>& resourceGroups);

  using ListOfTests = std::vector<cmCTestTestProperties>;

  // Support for writing test results in JUnit XML format.
  void SetJUnitXMLFileName(const std::string& id);

protected:
  using SetOfTests =
    std::set<cmCTestTestHandler::cmCTestTestResult, cmCTestTestResultLess>;

  // compute a final test list
  virtual int PreProcessHandler();
  virtual int PostProcessHandler();
  virtual void GenerateTestCommand(std::vector<std::string>& args, int test);
  int ExecuteCommands(std::vector<std::string>& vec);

  bool ProcessOptions();
  void LogTestSummary(const std::vector<std::string>& passed,
                      const std::vector<std::string>& failed,
                      const cmDuration& durationInSecs);
  void LogDisabledTests(const std::vector<cmCTestTestResult>& disabledTests);
  void LogFailedTests(const std::vector<std::string>& failed,
                      const SetOfTests& resultsSet);
  bool GenerateXML();

  void WriteTestResultHeader(cmXMLWriter& xml,
                             cmCTestTestResult const& result);
  void WriteTestResultFooter(cmXMLWriter& xml,
                             cmCTestTestResult const& result);
  // Write attached test files into the xml
  void AttachFiles(cmXMLWriter& xml, cmCTestTestResult& result);
  void AttachFile(cmXMLWriter& xml, std::string const& file,
                  std::string const& name);

  //! Clean test output to specified length and truncation mode
  void CleanTestOutput(std::string& output, size_t length,
                       cmCTestTypes::TruncationMode truncate);

  cmDuration ElapsedTestingTime;

  using TestResultsVector = std::vector<cmCTestTestResult>;
  TestResultsVector TestResults;

  std::vector<std::string> CustomTestsIgnore;
  std::string StartTest;
  std::string EndTest;
  std::chrono::system_clock::time_point StartTestTime;
  std::chrono::system_clock::time_point EndTestTime;
  bool MemCheck;
  int CustomMaximumPassedTestOutputSize;
  int CustomMaximumFailedTestOutputSize;
  cmCTestTypes::TruncationMode TestOutputTruncation;
  int MaxIndex;

public:
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

private:
  /**
   * Write test results in CTest's Test.xml format
   */
  virtual void GenerateCTestXML(cmXMLWriter& xml);

  /**
   * Write test results in JUnit XML format
   */
  bool WriteJUnitXML();

  void PrintLabelOrSubprojectSummary(bool isSubProject);

  /**
   * Run the tests for a directory and any subdirectories
   */
  bool ProcessDirectory(std::vector<std::string>& passed,
                        std::vector<std::string>& failed);

  /**
   * Get the list of tests in directory and subdirectories.
   */
  bool GetListOfTests();
  // compute the lists of tests that will actually run
  // based on union regex and -I stuff
  bool ComputeTestList();

  // compute the lists of tests that will actually run
  // based on LastTestFailed.log
  bool ComputeTestListForRerunFailed();

  // add required setup/cleanup tests not already in the
  // list of tests to be run and update dependencies between
  // tests to account for fixture setup/cleanup
  void UpdateForFixtures(ListOfTests& tests) const;

  void UpdateMaxTestNameWidth();

  bool GetValue(const char* tag, std::string& value, std::istream& fin);
  bool GetValue(const char* tag, int& value, std::istream& fin);
  bool GetValue(const char* tag, size_t& value, std::istream& fin);
  bool GetValue(const char* tag, bool& value, std::istream& fin);
  bool GetValue(const char* tag, double& value, std::istream& fin);
  /**
   * Find the executable for a test
   */
  std::string FindTheExecutable(const std::string& exe);

  std::string GetTestStatus(cmCTestTestResult const&);
  void ExpandTestsToRunInformation(size_t numPossibleTests);
  void ExpandTestsToRunInformationForRerunFailed();

  std::vector<std::string> CustomPreTest;
  std::vector<std::string> CustomPostTest;

  std::vector<int> TestsToRun;

  bool UseIncludeRegExpFlag;
  bool UseExcludeRegExpFlag;
  bool UseExcludeRegExpFirst;
  std::string IncludeRegExp;
  std::string ExcludeRegExp;
  std::string ExcludeFixtureRegExp;
  std::string ExcludeFixtureSetupRegExp;
  std::string ExcludeFixtureCleanupRegExp;
  std::vector<cmsys::RegularExpression> IncludeLabelRegularExpressions;
  std::vector<cmsys::RegularExpression> ExcludeLabelRegularExpressions;
  cmsys::RegularExpression IncludeTestsRegularExpression;
  cmsys::RegularExpression ExcludeTestsRegularExpression;

  std::string ResourceSpecFile;

  void RecordCustomTestMeasurements(cmXMLWriter& xml, std::string content);
  void CheckLabelFilter(cmCTestTestProperties& it);
  void CheckLabelFilterExclude(cmCTestTestProperties& it);
  void CheckLabelFilterInclude(cmCTestTestProperties& it);

  std::string TestsToRunString;
  bool UseUnion;
  ListOfTests TestList;
  size_t TotalNumberOfTests;
  cmsys::RegularExpression AllTestMeasurementsRegex;
  cmsys::RegularExpression SingleTestMeasurementRegex;
  cmsys::RegularExpression CustomCompletionStatusRegex;
  cmsys::RegularExpression CustomLabelRegex;

  std::ostream* LogFile;

  cmCTest::Repeat RepeatMode = cmCTest::Repeat::Never;
  int RepeatCount = 1;
  bool RerunFailed;

  std::string JUnitXMLFileName;
};
