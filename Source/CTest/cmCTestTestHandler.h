/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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

class cmMakefile;
class cmXMLWriter;

struct cmCTestTestOptions
{
  bool RerunFailed = false;
  bool ScheduleRandom = false;
  bool StopOnFailure = false;
  bool UseUnion = false;

  int OutputSizePassed = 1 * 1024;
  int OutputSizeFailed = 300 * 1024;
  cmCTestTypes::TruncationMode OutputTruncation =
    cmCTestTypes::TruncationMode::Tail;

  std::string TestsToRunInformation;
  std::string IncludeRegularExpression;
  std::string ExcludeRegularExpression;

  std::vector<std::string> LabelRegularExpression;
  std::vector<std::string> ExcludeLabelRegularExpression;

  std::string ExcludeFixtureRegularExpression;
  std::string ExcludeFixtureSetupRegularExpression;
  std::string ExcludeFixtureCleanupRegularExpression;

  std::string TestListFile;
  std::string ExcludeTestListFile;
  std::string ResourceSpecFile;
  std::string JUnitXMLFileName;
};

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
   * This method is called when reading CTest custom file
   */
  void PopulateCustomVectors(cmMakefile* mf) override;

  //! Control the use of the regular expressions, call these methods to turn
  /// them on
  void UseIncludeRegExp();
  void UseExcludeRegExp();

  void SetMaxIndex(int n) { this->MaxIndex = n; }
  int GetMaxIndex() { return this->MaxIndex; }

  //! pass the -I argument down
  void SetTestsToRunInformation(std::string const& in);

  cmCTestTestHandler(cmCTest* ctest);

  /*
   * Add the test to the list of tests to be executed
   */
  bool AddTest(std::vector<std::string> const& args);

  /*
   * Set tests properties
   */
  bool SetTestsProperties(std::vector<std::string> const& args);

  /**
   * Set directory properties
   */
  bool SetDirectoryProperties(std::vector<std::string> const& args);

  struct cmCTestTestResourceRequirement
  {
    std::string ResourceType;
    int SlotsNeeded;
    int UnitsNeeded;

    bool operator==(cmCTestTestResourceRequirement const& other) const;
    bool operator!=(cmCTestTestResourceRequirement const& other) const;
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
    std::map<std::string, std::string> CustomProperties;
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
    std::set<std::string> ProjectResources; // RESOURCE_LOCK
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
    std::string InstrumentationFile;
    int TestCount = 0;
    cmCTestTestProperties* Properties = nullptr;
  };

  struct cmCTestTestResultLess
  {
    bool operator()(cmCTestTestResult const& lhs,
                    cmCTestTestResult const& rhs) const
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
                                    std::string const& testCommand,
                                    std::string& resultingConfig,
                                    std::vector<std::string>& extraPaths,
                                    std::vector<std::string>& failed);

  static bool ParseResourceGroupsProperty(
    std::string const& val,
    std::vector<std::vector<cmCTestTestResourceRequirement>>& resourceGroups);

  using ListOfTests = std::vector<cmCTestTestProperties>;

  // Support for writing test results in JUnit XML format.
  void SetJUnitXMLFileName(std::string const& id);

protected:
  using SetOfTests =
    std::set<cmCTestTestHandler::cmCTestTestResult, cmCTestTestResultLess>;

  // compute a final test list
  virtual int PreProcessHandler();
  virtual int PostProcessHandler();
  virtual void GenerateTestCommand(std::vector<std::string>& args, int test);
  int ExecuteCommands(std::vector<std::string>& vec);

  bool ProcessOptions();
  void LogTestSummary(std::vector<std::string> const& passed,
                      std::vector<std::string> const& failed,
                      cmDuration const& durationInSecs);
  void LogDisabledTests(std::vector<cmCTestTestResult> const& disabledTests);
  void LogFailedTests(std::vector<std::string> const& failed,
                      SetOfTests const& resultsSet);
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

  cmCTestTestOptions TestOptions;

  cmDuration ElapsedTestingTime;

  using TestResultsVector = std::vector<cmCTestTestResult>;
  TestResultsVector TestResults;

  std::vector<std::string> CustomTestsIgnore;
  std::string StartTest;
  std::string EndTest;
  std::chrono::system_clock::time_point StartTestTime;
  std::chrono::system_clock::time_point EndTestTime;
  bool MemCheck = false;
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

  bool GetValue(char const* tag, std::string& value, std::istream& fin);
  bool GetValue(char const* tag, int& value, std::istream& fin);
  bool GetValue(char const* tag, size_t& value, std::istream& fin);
  bool GetValue(char const* tag, bool& value, std::istream& fin);
  bool GetValue(char const* tag, double& value, std::istream& fin);
  /**
   * Find the executable for a test
   */
  std::string FindTheExecutable(std::string const& exe);

  std::string GetTestStatus(cmCTestTestResult const&);
  void ExpandTestsToRunInformation(size_t numPossibleTests);
  void ExpandTestsToRunInformationForRerunFailed();
  cm::optional<std::set<std::string>> ReadTestListFile(
    std::string const& testListFileName) const;

  std::vector<std::string> CustomPreTest;
  std::vector<std::string> CustomPostTest;

  std::vector<int> TestsToRun;

  bool UseIncludeRegExpFlag = false;
  bool UseExcludeRegExpFlag = false;
  bool UseExcludeRegExpFirst = false;
  std::vector<cmsys::RegularExpression> IncludeLabelRegularExpressions;
  std::vector<cmsys::RegularExpression> ExcludeLabelRegularExpressions;
  cmsys::RegularExpression IncludeTestsRegularExpression;
  cmsys::RegularExpression ExcludeTestsRegularExpression;
  cm::optional<std::set<std::string>> TestsToRunByName;
  cm::optional<std::set<std::string>> TestsToExcludeByName;
  cm::optional<std::string> ParallelLevel;
  cm::optional<std::string> Repeat;

  void RecordCustomTestMeasurements(cmXMLWriter& xml, std::string content);
  void CheckLabelFilter(cmCTestTestProperties& it);
  void CheckLabelFilterExclude(cmCTestTestProperties& it);
  void CheckLabelFilterInclude(cmCTestTestProperties& it);

  std::string TestsToRunString;
  ListOfTests TestList;
  size_t TotalNumberOfTests;
  cmsys::RegularExpression AllTestMeasurementsRegex;
  cmsys::RegularExpression SingleTestMeasurementRegex;
  cmsys::RegularExpression CustomCompletionStatusRegex;
  cmsys::RegularExpression CustomLabelRegex;

  std::ostream* LogFile = nullptr;

  cmCTest::Repeat RepeatMode = cmCTest::Repeat::Never;
  int RepeatCount = 1;

  friend class cmCTestTestCommand;
};
