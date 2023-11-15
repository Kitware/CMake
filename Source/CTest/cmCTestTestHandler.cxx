/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestTestHandler.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef> // IWYU pragma: keep
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iterator>
#include <ratio>
#include <set>
#include <sstream>
#include <utility>

#ifndef _WIN32
#  include <csignal>
#endif

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"
#include <cmsys/Base64.h>
#include <cmsys/Directory.hxx>
#include <cmsys/RegularExpression.hxx>

#include "cm_utf8.h"

#include "cmCTest.h"
#include "cmCTestMultiProcessHandler.h"
#include "cmCTestResourceGroupsLexerHelper.h"
#include "cmCTestTestMeasurementXMLParser.h"
#include "cmDuration.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmValue.h"
#include "cmWorkingDirectory.h"
#include "cmXMLWriter.h"
#include "cmake.h"

namespace {

class cmCTestCommand
{
public:
  cmCTestCommand(cmCTestTestHandler* testHandler)
    : TestHandler(testHandler)
  {
  }

  virtual ~cmCTestCommand() = default;
  cmCTestCommand(const cmCTestCommand&) = default;
  cmCTestCommand& operator=(const cmCTestCommand&) = default;

  bool operator()(std::vector<cmListFileArgument> const& args,
                  cmExecutionStatus& status)
  {
    cmMakefile& mf = status.GetMakefile();
    std::vector<std::string> expandedArguments;
    if (!mf.ExpandArguments(args, expandedArguments)) {
      // There was an error expanding arguments.  It was already
      // reported, so we can skip this command without error.
      return true;
    }
    return this->InitialPass(expandedArguments, status);
  }

  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus& status) = 0;

  cmCTestTestHandler* TestHandler;
};

bool ReadSubdirectory(std::string fname, cmExecutionStatus& status)
{
  if (!cmSystemTools::FileExists(fname)) {
    // No subdirectory? So what...
    return true;
  }
  bool readit = false;
  {
    cmWorkingDirectory workdir(fname);
    if (workdir.Failed()) {
      status.SetError("Failed to change directory to " + fname + " : " +
                      std::strerror(workdir.GetLastResult()));
      return false;
    }
    const char* testFilename;
    if (cmSystemTools::FileExists("CTestTestfile.cmake")) {
      // does the CTestTestfile.cmake exist ?
      testFilename = "CTestTestfile.cmake";
    } else if (cmSystemTools::FileExists("DartTestfile.txt")) {
      // does the DartTestfile.txt exist ?
      testFilename = "DartTestfile.txt";
    } else {
      // No CTestTestfile? Who cares...
      return true;
    }
    fname += "/";
    fname += testFilename;
    readit = status.GetMakefile().ReadDependentFile(fname);
  }
  if (!readit) {
    status.SetError(cmStrCat("Could not find include file: ", fname));
    return false;
  }
  return true;
}

bool cmCTestSubdirCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  for (std::string const& arg : args) {
    std::string fname;

    if (cmSystemTools::FileIsFullPath(arg)) {
      fname = arg;
    } else {
      fname = cmStrCat(cwd, '/', arg);
    }

    if (!ReadSubdirectory(std::move(fname), status)) {
      return false;
    }
  }
  return true;
}

bool cmCTestAddSubdirectoryCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string fname =
    cmStrCat(cmSystemTools::GetCurrentWorkingDirectory(), '/', args[0]);

  return ReadSubdirectory(std::move(fname), status);
}

class cmCTestAddTestCommand : public cmCTestCommand
{
public:
  using cmCTestCommand::cmCTestCommand;

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& /*args*/,
                   cmExecutionStatus& /*unused*/) override;
};

bool cmCTestAddTestCommand::InitialPass(std::vector<std::string> const& args,
                                        cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  return this->TestHandler->AddTest(args);
}

class cmCTestSetTestsPropertiesCommand : public cmCTestCommand
{
public:
  using cmCTestCommand::cmCTestCommand;

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& /*args*/,
                   cmExecutionStatus& /*unused*/) override;
};

bool cmCTestSetTestsPropertiesCommand::InitialPass(
  std::vector<std::string> const& args, cmExecutionStatus& /*unused*/)
{
  return this->TestHandler->SetTestsProperties(args);
}

class cmCTestSetDirectoryPropertiesCommand : public cmCTestCommand
{
public:
  using cmCTestCommand::cmCTestCommand;

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& /*unused*/,
                   cmExecutionStatus& /*unused*/) override;
};

bool cmCTestSetDirectoryPropertiesCommand::InitialPass(
  std::vector<std::string> const& args, cmExecutionStatus&)
{
  return this->TestHandler->SetDirectoryProperties(args);
}

// get the next number in a string with numbers separated by ,
// pos is the start of the search and pos2 is the end of the search
// pos becomes pos2 after a call to GetNextNumber.
// -1 is returned at the end of the list.
inline int GetNextNumber(std::string const& in, int& val,
                         std::string::size_type& pos,
                         std::string::size_type& pos2)
{
  pos2 = in.find(',', pos);
  if (pos2 != std::string::npos) {
    if (pos2 - pos == 0) {
      val = -1;
    } else {
      val = atoi(in.substr(pos, pos2 - pos).c_str());
    }
    pos = pos2 + 1;
    return 1;
  }
  if (in.size() - pos == 0) {
    val = -1;
  } else {
    val = atoi(in.substr(pos, in.size() - pos).c_str());
  }
  return 0;
}

// get the next number in a string with numbers separated by ,
// pos is the start of the search and pos2 is the end of the search
// pos becomes pos2 after a call to GetNextNumber.
// -1 is returned at the end of the list.
inline int GetNextRealNumber(std::string const& in, double& val,
                             std::string::size_type& pos,
                             std::string::size_type& pos2)
{
  pos2 = in.find(',', pos);
  if (pos2 != std::string::npos) {
    if (pos2 - pos == 0) {
      val = -1;
    } else {
      val = atof(in.substr(pos, pos2 - pos).c_str());
    }
    pos = pos2 + 1;
    return 1;
  }
  if (in.size() - pos == 0) {
    val = -1;
  } else {
    val = atof(in.substr(pos, in.size() - pos).c_str());
  }
  return 0;
}

} // namespace

cmCTestTestHandler::cmCTestTestHandler()
{
  this->UseUnion = false;

  this->UseIncludeRegExpFlag = false;
  this->UseExcludeRegExpFlag = false;
  this->UseExcludeRegExpFirst = false;

  this->CustomMaximumPassedTestOutputSize = 1 * 1024;
  this->CustomMaximumFailedTestOutputSize = 300 * 1024;
  this->TestOutputTruncation = cmCTestTypes::TruncationMode::Tail;

  this->MemCheck = false;

  this->LogFile = nullptr;

  // Support for JUnit XML output.
  this->JUnitXMLFileName = "";

  // Regular expressions to scan test output for custom measurements.

  // Capture the whole section of test output from the first opening
  // <(CTest|Dart)Measurement*> tag to the last </(CTest|Dart)Measurement*>
  // closing tag.
  this->AllTestMeasurementsRegex.compile(
    "(<(CTest|Dart)Measurement.*/(CTest|Dart)Measurement[a-zA-Z]*>)");

  // Capture a single <(CTest|Dart)Measurement*> XML element.
  this->SingleTestMeasurementRegex.compile(
    "(<(CTest|Dart)Measurement[^<]*</(CTest|Dart)Measurement[a-zA-Z]*>)");

  // Capture content from <CTestDetails>...</CTestDetails>
  this->CustomCompletionStatusRegex.compile(
    "<CTestDetails>(.*)</CTestDetails>");

  // Capture content from <CTestLabel>...</CTestLabel>
  this->CustomLabelRegex.compile("<CTestLabel>(.*)</CTestLabel>");
}

void cmCTestTestHandler::Initialize()
{
  this->Superclass::Initialize();

  this->ElapsedTestingTime = cmDuration();

  this->TestResults.clear();

  this->CustomTestsIgnore.clear();
  this->StartTest.clear();
  this->EndTest.clear();

  this->CustomPreTest.clear();
  this->CustomPostTest.clear();
  this->CustomMaximumPassedTestOutputSize = 1 * 1024;
  this->CustomMaximumFailedTestOutputSize = 300 * 1024;
  this->TestOutputTruncation = cmCTestTypes::TruncationMode::Tail;

  this->TestsToRun.clear();

  this->UseIncludeRegExpFlag = false;
  this->UseExcludeRegExpFlag = false;
  this->UseExcludeRegExpFirst = false;
  this->IncludeLabelRegularExpressions.clear();
  this->ExcludeLabelRegularExpressions.clear();
  this->IncludeRegExp.clear();
  this->ExcludeRegExp.clear();
  this->ExcludeFixtureRegExp.clear();
  this->ExcludeFixtureSetupRegExp.clear();
  this->ExcludeFixtureCleanupRegExp.clear();

  this->TestsToRunString.clear();
  this->UseUnion = false;
  this->TestList.clear();
}

void cmCTestTestHandler::PopulateCustomVectors(cmMakefile* mf)
{
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_TEST",
                                    this->CustomPreTest);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_POST_TEST",
                                    this->CustomPostTest);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_TESTS_IGNORE",
                                    this->CustomTestsIgnore);
  this->CTest->PopulateCustomInteger(
    mf, "CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE",
    this->CustomMaximumPassedTestOutputSize);
  this->CTest->PopulateCustomInteger(
    mf, "CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE",
    this->CustomMaximumFailedTestOutputSize);

  cmValue dval = mf->GetDefinition("CTEST_CUSTOM_TEST_OUTPUT_TRUNCATION");
  if (dval) {
    if (!this->SetTestOutputTruncation(*dval)) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Invalid value for CTEST_CUSTOM_TEST_OUTPUT_TRUNCATION: "
                   << *dval << std::endl);
    }
  }
}

int cmCTestTestHandler::PreProcessHandler()
{
  if (!this->ExecuteCommands(this->CustomPreTest)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Problem executing pre-test command(s)." << std::endl);
    return 0;
  }
  return 1;
}

int cmCTestTestHandler::PostProcessHandler()
{
  if (!this->ExecuteCommands(this->CustomPostTest)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Problem executing post-test command(s)." << std::endl);
    return 0;
  }
  return 1;
}

int cmCTestTestHandler::ProcessHandler()
{
  if (!this->ProcessOptions()) {
    return -1;
  }

  this->TestResults.clear();

  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     (this->MemCheck ? "Memory check" : "Test")
                       << " project "
                       << cmSystemTools::GetCurrentWorkingDirectory()
                       << std::endl,
                     this->Quiet);
  if (!this->PreProcessHandler()) {
    return -1;
  }

  cmGeneratedFileStream mLogFile;
  this->StartLogFile((this->MemCheck ? "DynamicAnalysis" : "Test"), mLogFile);
  this->LogFile = &mLogFile;

  std::vector<std::string> passed;
  std::vector<std::string> failed;

  // start the real time clock
  auto clock_start = std::chrono::steady_clock::now();

  if (!this->ProcessDirectory(passed, failed)) {
    return -1;
  }

  auto clock_finish = std::chrono::steady_clock::now();

  bool noTestsFoundError = false;
  if (passed.size() + failed.size() == 0) {
    if (!this->CTest->GetShowOnly() && !this->CTest->ShouldPrintLabels() &&
        this->CTest->GetNoTestsMode() != cmCTest::NoTests::Ignore) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "No tests were found!!!" << std::endl);
      if (this->CTest->GetNoTestsMode() == cmCTest::NoTests::Error) {
        noTestsFoundError = true;
      }
    }
  } else {
    if (this->HandlerVerbose && !passed.empty() &&
        (this->UseIncludeRegExpFlag || this->UseExcludeRegExpFlag)) {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         std::endl
                           << "The following tests passed:" << std::endl,
                         this->Quiet);
      for (std::string const& j : passed) {
        cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                           "\t" << j << std::endl, this->Quiet);
      }
    }

    SetOfTests resultsSet(this->TestResults.begin(), this->TestResults.end());
    std::vector<cmCTestTestHandler::cmCTestTestResult> disabledTests;

    for (cmCTestTestResult const& ft : resultsSet) {
      if (cmHasLiteralPrefix(ft.CompletionStatus, "SKIP_") ||
          ft.CompletionStatus == "Disabled") {
        disabledTests.push_back(ft);
      }
    }

    cmDuration durationInSecs = clock_finish - clock_start;
    this->LogTestSummary(passed, failed, durationInSecs);

    this->LogDisabledTests(disabledTests);

    this->LogFailedTests(failed, resultsSet);
  }

  if (!this->GenerateXML()) {
    return 1;
  }

  if (!this->WriteJUnitXML()) {
    return 1;
  }

  if (!this->PostProcessHandler()) {
    this->LogFile = nullptr;
    return -1;
  }

  if (!failed.empty()) {
    this->LogFile = nullptr;
    return -1;
  }

  if (noTestsFoundError) {
    this->LogFile = nullptr;
    return -1;
  }

  this->LogFile = nullptr;
  return 0;
}

/* Given a multi-option value `parts`, compile those parts into
 * regular expressions in `expressions`. Skip empty values.
 * Returns true if there were any expressions.
 */
static bool BuildLabelRE(const std::vector<std::string>& parts,
                         std::vector<cmsys::RegularExpression>& expressions)
{
  expressions.clear();
  for (const auto& p : parts) {
    if (!p.empty()) {
      expressions.emplace_back(p);
    }
  }
  return !expressions.empty();
}

bool cmCTestTestHandler::ProcessOptions()
{
  // Update internal data structure from generic one
  this->SetTestsToRunInformation(this->GetOption("TestsToRunInformation"));
  this->SetUseUnion(cmIsOn(this->GetOption("UseUnion")));
  if (cmIsOn(this->GetOption("ScheduleRandom"))) {
    this->CTest->SetScheduleType("Random");
  }
  if (cmValue repeat = this->GetOption("Repeat")) {
    cmsys::RegularExpression repeatRegex(
      "^(UNTIL_FAIL|UNTIL_PASS|AFTER_TIMEOUT):([0-9]+)$");
    if (repeatRegex.find(*repeat)) {
      std::string const& count = repeatRegex.match(2);
      unsigned long n = 1;
      cmStrToULong(count, &n); // regex guarantees success
      this->RepeatCount = static_cast<int>(n);
      if (this->RepeatCount > 1) {
        std::string const& mode = repeatRegex.match(1);
        if (mode == "UNTIL_FAIL") {
          this->RepeatMode = cmCTest::Repeat::UntilFail;
        } else if (mode == "UNTIL_PASS") {
          this->RepeatMode = cmCTest::Repeat::UntilPass;
        } else if (mode == "AFTER_TIMEOUT") {
          this->RepeatMode = cmCTest::Repeat::AfterTimeout;
        }
      }
    } else {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Repeat option invalid value: " << *repeat << std::endl);
      return false;
    }
  }
  if (this->GetOption("ParallelLevel")) {
    this->CTest->SetParallelLevel(
      std::stoi(*this->GetOption("ParallelLevel")));
  }

  if (this->GetOption("StopOnFailure")) {
    this->CTest->SetStopOnFailure(true);
  }

  BuildLabelRE(this->GetMultiOption("LabelRegularExpression"),
               this->IncludeLabelRegularExpressions);
  BuildLabelRE(this->GetMultiOption("ExcludeLabelRegularExpression"),
               this->ExcludeLabelRegularExpressions);
  cmValue val = this->GetOption("IncludeRegularExpression");
  if (val) {
    this->UseIncludeRegExp();
    this->SetIncludeRegExp(*val);
  }
  val = this->GetOption("ExcludeRegularExpression");
  if (val) {
    this->UseExcludeRegExp();
    this->SetExcludeRegExp(*val);
  }
  val = this->GetOption("ExcludeFixtureRegularExpression");
  if (val) {
    this->ExcludeFixtureRegExp = *val;
  }
  val = this->GetOption("ExcludeFixtureSetupRegularExpression");
  if (val) {
    this->ExcludeFixtureSetupRegExp = *val;
  }
  val = this->GetOption("ExcludeFixtureCleanupRegularExpression");
  if (val) {
    this->ExcludeFixtureCleanupRegExp = *val;
  }
  val = this->GetOption("ResourceSpecFile");
  if (val) {
    this->ResourceSpecFile = *val;
  }
  this->SetRerunFailed(cmIsOn(this->GetOption("RerunFailed")));

  return true;
}

void cmCTestTestHandler::LogTestSummary(const std::vector<std::string>& passed,
                                        const std::vector<std::string>& failed,
                                        const cmDuration& durationInSecs)
{
  std::size_t total = passed.size() + failed.size();

  float percent =
    static_cast<float>(passed.size()) * 100.0f / static_cast<float>(total);
  if (!failed.empty() && percent > 99) {
    percent = 99;
  }

  std::string passColorCode;
  std::string failedColorCode;
  if (failed.empty()) {
    passColorCode = this->CTest->GetColorCode(cmCTest::Color::GREEN);
  } else {
    failedColorCode = this->CTest->GetColorCode(cmCTest::Color::RED);
  }
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             std::endl
               << passColorCode << std::lround(percent) << "% tests passed"
               << this->CTest->GetColorCode(cmCTest::Color::CLEAR_COLOR)
               << ", " << failedColorCode << failed.size() << " tests failed"
               << this->CTest->GetColorCode(cmCTest::Color::CLEAR_COLOR)
               << " out of " << total << std::endl);
  if ((!this->CTest->GetLabelsForSubprojects().empty() &&
       this->CTest->GetSubprojectSummary())) {
    this->PrintLabelOrSubprojectSummary(true);
  }
  if (this->CTest->GetLabelSummary()) {
    this->PrintLabelOrSubprojectSummary(false);
  }
  char realBuf[1024];
  snprintf(realBuf, sizeof(realBuf), "%6.2f sec", durationInSecs.count());
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     "\nTotal Test time (real) = " << realBuf << "\n",
                     this->Quiet);
}

void cmCTestTestHandler::LogDisabledTests(
  const std::vector<cmCTestTestResult>& disabledTests)
{
  if (!disabledTests.empty()) {
    cmGeneratedFileStream ofs;
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               std::endl
                 << "The following tests did not run:" << std::endl);
    this->StartLogFile("TestsDisabled", ofs);

    const char* disabled_reason;
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               this->CTest->GetColorCode(cmCTest::Color::BLUE));
    for (cmCTestTestResult const& dt : disabledTests) {
      ofs << dt.TestCount << ":" << dt.Name << std::endl;
      if (dt.CompletionStatus == "Disabled") {
        disabled_reason = "Disabled";
      } else {
        disabled_reason = "Skipped";
      }
      cmCTestLog(this->CTest, HANDLER_OUTPUT,
                 "\t" << std::setw(3) << dt.TestCount << " - " << dt.Name
                      << " (" << disabled_reason << ")" << std::endl);
    }
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               this->CTest->GetColorCode(cmCTest::Color::CLEAR_COLOR));
  }
}

void cmCTestTestHandler::LogFailedTests(const std::vector<std::string>& failed,
                                        const SetOfTests& resultsSet)
{
  if (!failed.empty()) {
    cmGeneratedFileStream ofs;
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               std::endl
                 << "The following tests FAILED:" << std::endl);
    this->StartLogFile("TestsFailed", ofs);

    for (cmCTestTestResult const& ft : resultsSet) {
      if (ft.Status != cmCTestTestHandler::COMPLETED &&
          !cmHasLiteralPrefix(ft.CompletionStatus, "SKIP_") &&
          ft.CompletionStatus != "Disabled") {
        ofs << ft.TestCount << ":" << ft.Name << std::endl;
        auto testColor = cmCTest::Color::RED;
        if (this->GetTestStatus(ft) == "Not Run") {
          testColor = cmCTest::Color::YELLOW;
        }
        cmCTestLog(
          this->CTest, HANDLER_OUTPUT,
          "\t" << this->CTest->GetColorCode(testColor) << std::setw(3)
               << ft.TestCount << " - " << ft.Name << " ("
               << this->GetTestStatus(ft) << ")"
               << this->CTest->GetColorCode(cmCTest::Color::CLEAR_COLOR)
               << std::endl);
      }
    }
  }
}

bool cmCTestTestHandler::GenerateXML()
{
  if (this->CTest->GetProduceXML()) {
    cmGeneratedFileStream xmlfile;
    if (!this->StartResultingXML(
          (this->MemCheck ? cmCTest::PartMemCheck : cmCTest::PartTest),
          (this->MemCheck ? "DynamicAnalysis" : "Test"), xmlfile)) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Cannot create "
                   << (this->MemCheck ? "memory check" : "testing")
                   << " XML file" << std::endl);
      this->LogFile = nullptr;
      return false;
    }
    cmXMLWriter xml(xmlfile);
    this->GenerateCTestXML(xml);
  }

  if (this->MemCheck) {
    cmGeneratedFileStream xmlfile;
    if (!this->StartResultingXML(cmCTest::PartTest, "DynamicAnalysis-Test",
                                 xmlfile)) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Cannot create testing XML file" << std::endl);
      this->LogFile = nullptr;
      return false;
    }
    cmXMLWriter xml(xmlfile);
    // Explicitly call this class' `GenerateCTestXML` method to make `Test.xml`
    // as well.
    this->cmCTestTestHandler::GenerateCTestXML(xml);
  }

  return true;
}

void cmCTestTestHandler::PrintLabelOrSubprojectSummary(bool doSubProject)
{
  // collect subproject labels
  std::vector<std::string> subprojects =
    this->CTest->GetLabelsForSubprojects();
  std::map<std::string, double> labelTimes;
  std::map<std::string, int> labelCounts;
  std::set<std::string> labels;
  std::string::size_type maxlen = 0;
  // initialize maps
  for (cmCTestTestProperties& p : this->TestList) {
    for (std::string const& l : p.Labels) {
      // first check to see if the current label is a subproject label
      bool isSubprojectLabel = false;
      auto subproject = std::find(subprojects.begin(), subprojects.end(), l);
      if (subproject != subprojects.end()) {
        isSubprojectLabel = true;
      }
      // if we are doing sub projects and this label is one, then use it
      // if we are not doing sub projects and the label is not one use it
      if (doSubProject == isSubprojectLabel) {
        if (l.size() > maxlen) {
          maxlen = l.size();
        }
        labels.insert(l);
        labelTimes[l] = 0;
        labelCounts[l] = 0;
      }
    }
  }
  // fill maps
  for (cmCTestTestResult& result : this->TestResults) {
    cmCTestTestProperties& p = *result.Properties;
    for (std::string const& l : p.Labels) {
      // only use labels found in labels
      if (cm::contains(labels, l)) {
        labelTimes[l] +=
          result.ExecutionTime.count() * result.Properties->Processors;
        ++labelCounts[l];
      }
    }
  }
  // if no labels are found return and print nothing
  if (labels.empty()) {
    return;
  }
  // now print times
  if (doSubProject) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "\nSubproject Time Summary:", this->Quiet);
  } else {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "\nLabel Time Summary:", this->Quiet);
  }
  for (std::string const& i : labels) {
    std::string label = i;
    label.resize(maxlen + 3, ' ');

    char buf[1024];
    snprintf(buf, sizeof(buf), "%6.2f sec*proc", labelTimes[i]);

    std::ostringstream labelCountStr;
    labelCountStr << "(" << labelCounts[i] << " test";
    if (labelCounts[i] > 1) {
      labelCountStr << "s";
    }
    labelCountStr << ")";
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "\n"
                         << label << " = " << buf << " "
                         << labelCountStr.str(),
                       this->Quiet);
    if (this->LogFile) {
      *this->LogFile << "\n" << i << " = " << buf << "\n";
    }
  }
  if (this->LogFile) {
    *this->LogFile << "\n";
  }
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "\n", this->Quiet);
}

/**
 * Check if the labels (from a test) match all the expressions.
 *
 * Each of the RE's must match at least one label
 * (e.g. all of the REs must match **some** label,
 * in order for the filter to apply to the test).
 */
static bool MatchLabelsAgainstFilterRE(
  const std::vector<std::string>& labels,
  const std::vector<cmsys::RegularExpression>& expressions)
{
  for (const auto& re : expressions) {
    // check to see if the label regular expression matches
    bool found = false; // assume it does not match
    cmsys::RegularExpressionMatch match;
    // loop over all labels and look for match
    for (std::string const& l : labels) {
      if (re.find(l.c_str(), match)) {
        found = true;
        break;
      }
    }
    // if no match was found, exclude the test
    if (!found) {
      return false;
    }
  }
  return true;
}

void cmCTestTestHandler::CheckLabelFilterInclude(cmCTestTestProperties& it)
{
  // if not using Labels to filter then return
  if (this->IncludeLabelRegularExpressions.empty()) {
    return;
  }
  // if there are no labels and we are filtering by labels
  // then exclude the test as it does not have the label
  if (it.Labels.empty()) {
    it.IsInBasedOnREOptions = false;
    return;
  }
  // if no match was found, exclude the test
  if (!MatchLabelsAgainstFilterRE(it.Labels,
                                  this->IncludeLabelRegularExpressions)) {
    it.IsInBasedOnREOptions = false;
  }
}

void cmCTestTestHandler::CheckLabelFilterExclude(cmCTestTestProperties& it)
{
  // if not using Labels to filter then return
  if (this->ExcludeLabelRegularExpressions.empty()) {
    return;
  }
  // if there are no labels and we are excluding by labels
  // then do nothing as a no label can not be a match
  if (it.Labels.empty()) {
    return;
  }
  // if match was found, exclude the test
  if (MatchLabelsAgainstFilterRE(it.Labels,
                                 this->ExcludeLabelRegularExpressions)) {
    it.IsInBasedOnREOptions = false;
  }
}

void cmCTestTestHandler::CheckLabelFilter(cmCTestTestProperties& it)
{
  this->CheckLabelFilterInclude(it);
  this->CheckLabelFilterExclude(it);
}

bool cmCTestTestHandler::ComputeTestList()
{
  this->TestList.clear(); // clear list of test
  if (!this->GetListOfTests()) {
    return false;
  }

  if (this->RerunFailed) {
    return this->ComputeTestListForRerunFailed();
  }

  cmCTestTestHandler::ListOfTests::size_type tmsize = this->TestList.size();
  // how many tests are in based on RegExp?
  int inREcnt = 0;
  for (cmCTestTestProperties& tp : this->TestList) {
    this->CheckLabelFilter(tp);
    if (tp.IsInBasedOnREOptions) {
      inREcnt++;
    }
  }
  // expand the test list based on the union flag
  if (this->UseUnion) {
    this->ExpandTestsToRunInformation(static_cast<int>(tmsize));
  } else {
    this->ExpandTestsToRunInformation(inREcnt);
  }
  // Now create a final list of tests to run
  int cnt = 0;
  inREcnt = 0;
  std::string last_directory;
  ListOfTests finalList;
  for (cmCTestTestProperties& tp : this->TestList) {
    cnt++;
    if (tp.IsInBasedOnREOptions) {
      inREcnt++;
    }

    if (this->UseUnion) {
      // if it is not in the list and not in the regexp then skip
      if ((!this->TestsToRun.empty() &&
           !cm::contains(this->TestsToRun, cnt)) &&
          !tp.IsInBasedOnREOptions) {
        continue;
      }
    } else {
      // is this test in the list of tests to run? If not then skip it
      if ((!this->TestsToRun.empty() &&
           !cm::contains(this->TestsToRun, inREcnt)) ||
          !tp.IsInBasedOnREOptions) {
        continue;
      }
    }
    tp.Index = cnt; // save the index into the test list for this test
    finalList.push_back(tp);
  }

  this->UpdateForFixtures(finalList);

  // Save the total number of tests before exclusions
  this->TotalNumberOfTests = this->TestList.size();
  // Set the TestList to the final list of all test
  this->TestList = finalList;

  this->UpdateMaxTestNameWidth();
  return true;
}

bool cmCTestTestHandler::ComputeTestListForRerunFailed()
{
  this->ExpandTestsToRunInformationForRerunFailed();

  ListOfTests finalList;
  int cnt = 0;
  for (cmCTestTestProperties& tp : this->TestList) {
    cnt++;

    // if this test is not in our list of tests to run, then skip it.
    if (!this->TestsToRun.empty() && !cm::contains(this->TestsToRun, cnt)) {
      continue;
    }

    tp.Index = cnt;
    finalList.push_back(tp);
  }

  this->UpdateForFixtures(finalList);

  // Save the total number of tests before exclusions
  this->TotalNumberOfTests = this->TestList.size();

  // Set the TestList to the list of failed tests to rerun
  this->TestList = finalList;

  this->UpdateMaxTestNameWidth();

  return true;
}

void cmCTestTestHandler::UpdateForFixtures(ListOfTests& tests) const
{
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Updating test list for fixtures" << std::endl,
                     this->Quiet);

  // Prepare regular expression evaluators
  std::string setupRegExp(this->ExcludeFixtureRegExp);
  std::string cleanupRegExp(this->ExcludeFixtureRegExp);
  if (!this->ExcludeFixtureSetupRegExp.empty()) {
    if (setupRegExp.empty()) {
      setupRegExp = this->ExcludeFixtureSetupRegExp;
    } else {
      setupRegExp.append("(" + setupRegExp + ")|(" +
                         this->ExcludeFixtureSetupRegExp + ")");
    }
  }
  if (!this->ExcludeFixtureCleanupRegExp.empty()) {
    if (cleanupRegExp.empty()) {
      cleanupRegExp = this->ExcludeFixtureCleanupRegExp;
    } else {
      cleanupRegExp.append("(" + cleanupRegExp + ")|(" +
                           this->ExcludeFixtureCleanupRegExp + ")");
    }
  }
  cmsys::RegularExpression excludeSetupRegex(setupRegExp);
  cmsys::RegularExpression excludeCleanupRegex(cleanupRegExp);

  // Prepare some maps to help us find setup and cleanup tests for
  // any given fixture
  using TestIterator = ListOfTests::const_iterator;
  using FixtureDependencies = std::multimap<std::string, TestIterator>;
  using FixtureDepsIterator = FixtureDependencies::const_iterator;
  FixtureDependencies fixtureSetups;
  FixtureDependencies fixtureCleanups;

  for (auto it = this->TestList.begin(); it != this->TestList.end(); ++it) {
    const cmCTestTestProperties& p = *it;

    for (std::string const& deps : p.FixturesSetup) {
      fixtureSetups.insert(std::make_pair(deps, it));
    }

    for (std::string const& deps : p.FixturesCleanup) {
      fixtureCleanups.insert(std::make_pair(deps, it));
    }
  }

  // Prepare fast lookup of tests already included in our list of tests
  std::set<std::string> addedTests;
  for (cmCTestTestProperties const& p : tests) {
    addedTests.insert(p.Name);
  }

  // These are lookups of fixture name to a list of indices into the final
  // tests array for tests which require that fixture and tests which are
  // setups for that fixture. They are needed at the end to populate
  // dependencies of the cleanup tests in our final list of tests.
  std::map<std::string, std::vector<size_t>> fixtureRequirements;
  std::map<std::string, std::vector<size_t>> setupFixturesAdded;

  // Use integer index for iteration because we append to
  // the tests vector as we go
  size_t fixtureTestsAdded = 0;
  std::set<std::string> addedFixtures;
  for (size_t i = 0; i < tests.size(); ++i) {
    // Skip disabled tests
    if (tests[i].Disabled) {
      continue;
    }

    // There are two things to do for each test:
    //   1. For every fixture required by this test, record that fixture as
    //      being required and create dependencies on that fixture's setup
    //      tests.
    //   2. Record all setup tests in the final test list so we can later make
    //      cleanup tests in the test list depend on their associated setup
    //      tests to enforce correct ordering.

    // 1. Handle fixture requirements
    //
    // Must copy the set of fixtures required because we may invalidate
    // the tests array by appending to it
    std::set<std::string> fixtures = tests[i].FixturesRequired;
    for (std::string const& requiredFixtureName : fixtures) {
      if (requiredFixtureName.empty()) {
        continue;
      }

      fixtureRequirements[requiredFixtureName].push_back(i);

      // Add dependencies to this test for all of the setup tests
      // associated with the required fixture. If any of those setup
      // tests fail, this test should not run. We make the fixture's
      // cleanup tests depend on this test case later.
      std::pair<FixtureDepsIterator, FixtureDepsIterator> setupRange =
        fixtureSetups.equal_range(requiredFixtureName);
      for (auto sIt = setupRange.first; sIt != setupRange.second; ++sIt) {
        const std::string& setupTestName = sIt->second->Name;
        tests[i].RequireSuccessDepends.insert(setupTestName);
        if (!cm::contains(tests[i].Depends, setupTestName)) {
          tests[i].Depends.push_back(setupTestName);
        }
      }

      // Append any fixture setup/cleanup tests to our test list if they
      // are not already in it (they could have been in the original
      // set of tests passed to us at the outset or have already been
      // added from a previously checked test). A fixture isn't required
      // to have setup/cleanup tests.
      if (!addedFixtures.insert(requiredFixtureName).second) {
        // Already seen this fixture, no need to check it again
        continue;
      }

      // Only add setup tests if this fixture has not been excluded
      if (setupRegExp.empty() ||
          !excludeSetupRegex.find(requiredFixtureName)) {
        std::pair<FixtureDepsIterator, FixtureDepsIterator> fixtureRange =
          fixtureSetups.equal_range(requiredFixtureName);
        for (auto it = fixtureRange.first; it != fixtureRange.second; ++it) {
          ListOfTests::const_iterator lotIt = it->second;
          const cmCTestTestProperties& p = *lotIt;

          if (!addedTests.insert(p.Name).second) {
            // Already have p in our test list
            continue;
          }

          // This is a test not yet in our list, so add it and
          // update its index to reflect where it was in the original
          // full list of all tests (needed to track individual tests
          // across ctest runs for re-run failed, etc.)
          tests.push_back(p);
          tests.back().Index =
            1 + static_cast<int>(std::distance(this->TestList.begin(), lotIt));
          ++fixtureTestsAdded;

          cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                             "Added setup test "
                               << p.Name << " required by fixture "
                               << requiredFixtureName << std::endl,
                             this->Quiet);
        }
      }

      // Only add cleanup tests if this fixture has not been excluded
      if (cleanupRegExp.empty() ||
          !excludeCleanupRegex.find(requiredFixtureName)) {
        std::pair<FixtureDepsIterator, FixtureDepsIterator> fixtureRange =
          fixtureCleanups.equal_range(requiredFixtureName);
        for (auto it = fixtureRange.first; it != fixtureRange.second; ++it) {
          ListOfTests::const_iterator lotIt = it->second;
          const cmCTestTestProperties& p = *lotIt;

          if (!addedTests.insert(p.Name).second) {
            // Already have p in our test list
            continue;
          }

          // This is a test not yet in our list, so add it and
          // update its index to reflect where it was in the original
          // full list of all tests (needed to track individual tests
          // across ctest runs for re-run failed, etc.)
          tests.push_back(p);
          tests.back().Index =
            1 + static_cast<int>(std::distance(this->TestList.begin(), lotIt));
          ++fixtureTestsAdded;

          cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                             "Added cleanup test "
                               << p.Name << " required by fixture "
                               << requiredFixtureName << std::endl,
                             this->Quiet);
        }
      }
    }

    // 2. Record all setup fixtures included in the final list of tests
    for (std::string const& setupFixtureName : tests[i].FixturesSetup) {
      if (setupFixtureName.empty()) {
        continue;
      }

      setupFixturesAdded[setupFixtureName].push_back(i);
    }
  }

  // Now that we have the final list of tests, we can update all cleanup
  // tests to depend on those tests which require that fixture and on any
  // setup tests for that fixture. The latter is required to handle the
  // pathological case where setup and cleanup tests are in the test set
  // but no other test has that fixture as a requirement.
  for (cmCTestTestProperties& p : tests) {
    const std::set<std::string>& cleanups = p.FixturesCleanup;
    for (std::string const& fixture : cleanups) {
      // This cleanup test could be part of the original test list that was
      // passed in. It is then possible that no other test requires the
      // fIt fixture, so we have to check for this.
      auto cIt = fixtureRequirements.find(fixture);
      if (cIt != fixtureRequirements.end()) {
        const std::vector<size_t>& indices = cIt->second;
        for (size_t index : indices) {
          const std::string& reqTestName = tests[index].Name;
          if (!cm::contains(p.Depends, reqTestName)) {
            p.Depends.push_back(reqTestName);
          }
        }
      }

      // Ensure fixture cleanup tests always run after their setup tests, even
      // if no other test cases require the fixture
      cIt = setupFixturesAdded.find(fixture);
      if (cIt != setupFixturesAdded.end()) {
        const std::vector<size_t>& indices = cIt->second;
        for (size_t index : indices) {
          const std::string& setupTestName = tests[index].Name;
          if (!cm::contains(p.Depends, setupTestName)) {
            p.Depends.push_back(setupTestName);
          }
        }
      }
    }
  }

  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Added " << fixtureTestsAdded
                              << " tests to meet fixture requirements"
                              << std::endl,
                     this->Quiet);
}

void cmCTestTestHandler::UpdateMaxTestNameWidth()
{
  std::string::size_type max = this->CTest->GetMaxTestNameWidth();
  for (cmCTestTestProperties& p : this->TestList) {
    if (max < p.Name.size()) {
      max = p.Name.size();
    }
  }
  if (static_cast<std::string::size_type>(
        this->CTest->GetMaxTestNameWidth()) != max) {
    this->CTest->SetMaxTestNameWidth(static_cast<int>(max));
  }
}

bool cmCTestTestHandler::GetValue(const char* tag, int& value,
                                  std::istream& fin)
{
  std::string line;
  bool ret = true;
  cmSystemTools::GetLineFromStream(fin, line);
  if (line == tag) {
    fin >> value;
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
  } else {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: " << tag << " found [" << line << "]"
                                            << std::endl);
    ret = false;
  }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag, double& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if (line == tag) {
    fin >> value;
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
  } else {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: " << tag << " found [" << line << "]"
                                            << std::endl);
    ret = false;
  }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag, bool& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if (line == tag) {
#ifdef __HAIKU__
    int tmp = 0;
    fin >> tmp;
    value = false;
    if (tmp) {
      value = true;
    }
#else
    fin >> value;
#endif
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
  } else {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: " << tag << " found [" << line << "]"
                                            << std::endl);
    ret = false;
  }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag, size_t& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if (line == tag) {
    fin >> value;
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
  } else {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: " << tag << " found [" << line << "]"
                                            << std::endl);
    ret = false;
  }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag, std::string& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if (line == tag) {
    ret = cmSystemTools::GetLineFromStream(fin, value);
  } else {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: " << tag << " found [" << line << "]"
                                            << std::endl);
    ret = false;
  }
  return ret;
}

bool cmCTestTestHandler::ProcessDirectory(std::vector<std::string>& passed,
                                          std::vector<std::string>& failed)
{
  if (!this->ComputeTestList()) {
    return false;
  }

  this->StartTest = this->CTest->CurrentTime();
  this->StartTestTime = std::chrono::system_clock::now();
  auto elapsed_time_start = std::chrono::steady_clock::now();

  auto parallel = cm::make_unique<cmCTestMultiProcessHandler>();
  parallel->SetCTest(this->CTest);
  parallel->SetParallelLevel(this->CTest->GetParallelLevel());
  parallel->SetTestHandler(this);
  if (this->RepeatMode != cmCTest::Repeat::Never) {
    parallel->SetRepeatMode(this->RepeatMode, this->RepeatCount);
  } else {
    parallel->SetRepeatMode(this->CTest->GetRepeatMode(),
                            this->CTest->GetRepeatCount());
  }
  parallel->SetQuiet(this->Quiet);
  if (this->TestLoad > 0) {
    parallel->SetTestLoad(this->TestLoad);
  } else {
    parallel->SetTestLoad(this->CTest->GetTestLoad());
  }

  *this->LogFile
    << "Start testing: " << this->CTest->CurrentTime() << std::endl
    << "----------------------------------------------------------"
    << std::endl;

  cmCTestMultiProcessHandler::TestMap tests;
  cmCTestMultiProcessHandler::PropertiesMap properties;

  bool randomSchedule = this->CTest->GetScheduleType() == "Random";
  if (randomSchedule) {
    srand(static_cast<unsigned>(time(nullptr)));
  }

  for (cmCTestTestProperties& p : this->TestList) {
    cmCTestMultiProcessHandler::TestSet depends;

    if (randomSchedule) {
      p.Cost = static_cast<float>(rand());
    }

    if (!p.Depends.empty()) {
      for (std::string const& i : p.Depends) {
        for (cmCTestTestProperties const& it2 : this->TestList) {
          if (it2.Name == i) {
            depends.insert(it2.Index);
            break; // break out of test loop as name can only match 1
          }
        }
      }
    }
    tests[p.Index].Depends = depends;
    properties[p.Index] = &p;
  }
  parallel->SetResourceSpecFile(this->ResourceSpecFile);
  parallel->SetTests(std::move(tests), std::move(properties));
  parallel->SetPassFailVectors(&passed, &failed);
  this->TestResults.clear();
  parallel->SetTestResults(&this->TestResults);
  parallel->CheckResourceAvailability();

  if (this->CTest->ShouldPrintLabels()) {
    parallel->PrintLabels();
  } else if (this->CTest->GetShowOnly()) {
    parallel->PrintTestList();
  } else {
    parallel->RunTests();
  }
  this->EndTest = this->CTest->CurrentTime();
  this->EndTestTime = std::chrono::system_clock::now();
  this->ElapsedTestingTime =
    std::chrono::steady_clock::now() - elapsed_time_start;
  *this->LogFile << "End testing: " << this->CTest->CurrentTime() << std::endl;

  return true;
}

void cmCTestTestHandler::GenerateTestCommand(
  std::vector<std::string>& /*unused*/, int /*unused*/)
{
}

void cmCTestTestHandler::GenerateCTestXML(cmXMLWriter& xml)
{
  if (!this->CTest->GetProduceXML()) {
    return;
  }

  this->CTest->StartXML(xml, this->AppendXML);
  this->CTest->GenerateSubprojectsOutput(xml);
  xml.StartElement("Testing");
  xml.Element("StartDateTime", this->StartTest);
  xml.Element("StartTestTime", this->StartTestTime);
  xml.StartElement("TestList");
  for (cmCTestTestResult const& result : this->TestResults) {
    std::string testPath = result.Path + "/" + result.Name;
    xml.Element("Test", this->CTest->GetShortPathToFile(testPath));
  }
  xml.EndElement(); // TestList
  for (cmCTestTestResult& result : this->TestResults) {
    this->WriteTestResultHeader(xml, result);
    xml.StartElement("Results");

    if (result.Status != cmCTestTestHandler::NOT_RUN) {
      if (result.Status != cmCTestTestHandler::COMPLETED ||
          result.ReturnValue) {
        xml.StartElement("NamedMeasurement");
        xml.Attribute("type", "text/string");
        xml.Attribute("name", "Exit Code");
        xml.Element("Value", this->GetTestStatus(result));
        xml.EndElement(); // NamedMeasurement

        xml.StartElement("NamedMeasurement");
        xml.Attribute("type", "text/string");
        xml.Attribute("name", "Exit Value");
        xml.Element("Value", result.ReturnValue);
        xml.EndElement(); // NamedMeasurement
      }
      this->RecordCustomTestMeasurements(xml, result.TestMeasurementsOutput);
      xml.StartElement("NamedMeasurement");
      xml.Attribute("type", "numeric/double");
      xml.Attribute("name", "Execution Time");
      xml.Element("Value", result.ExecutionTime.count());
      xml.EndElement(); // NamedMeasurement
      if (!result.Reason.empty()) {
        const char* reasonType = "Pass Reason";
        if (result.Status != cmCTestTestHandler::COMPLETED) {
          reasonType = "Fail Reason";
        }
        xml.StartElement("NamedMeasurement");
        xml.Attribute("type", "text/string");
        xml.Attribute("name", reasonType);
        xml.Element("Value", result.Reason);
        xml.EndElement(); // NamedMeasurement
      }
    }

    xml.StartElement("NamedMeasurement");
    xml.Attribute("type", "numeric/double");
    xml.Attribute("name", "Processors");
    xml.Element("Value", result.Properties->Processors);
    xml.EndElement(); // NamedMeasurement

    xml.StartElement("NamedMeasurement");
    xml.Attribute("type", "text/string");
    xml.Attribute("name", "Completion Status");
    if (result.CustomCompletionStatus.empty()) {
      xml.Element("Value", result.CompletionStatus);
    } else {
      xml.Element("Value", result.CustomCompletionStatus);
    }
    xml.EndElement(); // NamedMeasurement

    xml.StartElement("NamedMeasurement");
    xml.Attribute("type", "text/string");
    xml.Attribute("name", "Command Line");
    xml.Element("Value", result.FullCommandLine);
    xml.EndElement(); // NamedMeasurement

    xml.StartElement("NamedMeasurement");
    xml.Attribute("type", "text/string");
    xml.Attribute("name", "Environment");
    xml.Element("Value", result.Environment);
    xml.EndElement(); // NamedMeasurement
    for (auto const& measure : result.Properties->Measurements) {
      xml.StartElement("NamedMeasurement");
      xml.Attribute("type", "text/string");
      xml.Attribute("name", measure.first);
      xml.Element("Value", measure.second);
      xml.EndElement(); // NamedMeasurement
    }
    xml.StartElement("Measurement");
    xml.StartElement("Value");
    if (result.CompressOutput) {
      xml.Attribute("encoding", "base64");
      xml.Attribute("compression", "gzip");
    }
    xml.Content(result.Output);
    xml.EndElement(); // Value
    xml.EndElement(); // Measurement
    xml.EndElement(); // Results

    this->AttachFiles(xml, result);
    this->WriteTestResultFooter(xml, result);
  }

  xml.Element("EndDateTime", this->EndTest);
  xml.Element("EndTestTime", this->EndTestTime);
  xml.Element(
    "ElapsedMinutes",
    std::chrono::duration_cast<std::chrono::minutes>(this->ElapsedTestingTime)
      .count());
  xml.EndElement(); // Testing
  this->CTest->EndXML(xml);
}

void cmCTestTestHandler::WriteTestResultHeader(cmXMLWriter& xml,
                                               cmCTestTestResult const& result)
{
  xml.StartElement("Test");
  if (result.Status == cmCTestTestHandler::COMPLETED) {
    xml.Attribute("Status", "passed");
  } else if (result.Status == cmCTestTestHandler::NOT_RUN) {
    xml.Attribute("Status", "notrun");
  } else {
    xml.Attribute("Status", "failed");
  }
  std::string testPath = result.Path + "/" + result.Name;
  xml.Element("Name", result.Name);
  xml.Element("Path", this->CTest->GetShortPathToFile(result.Path));
  xml.Element("FullName", this->CTest->GetShortPathToFile(testPath));
  xml.Element("FullCommandLine", result.FullCommandLine);
}

void cmCTestTestHandler::WriteTestResultFooter(cmXMLWriter& xml,
                                               cmCTestTestResult const& result)
{
  if (!result.Properties->Labels.empty()) {
    xml.StartElement("Labels");
    std::vector<std::string> const& labels = result.Properties->Labels;
    for (std::string const& label : labels) {
      xml.Element("Label", label);
    }
    xml.EndElement(); // Labels
  }

  xml.EndElement(); // Test
}

void cmCTestTestHandler::AttachFiles(cmXMLWriter& xml,
                                     cmCTestTestResult& result)
{
  if (result.Status != cmCTestTestHandler::COMPLETED &&
      !result.Properties->AttachOnFail.empty()) {
    result.Properties->AttachedFiles.insert(
      result.Properties->AttachedFiles.end(),
      result.Properties->AttachOnFail.begin(),
      result.Properties->AttachOnFail.end());
  }
  for (std::string const& file : result.Properties->AttachedFiles) {
    this->AttachFile(xml, file, "");
  }
}

void cmCTestTestHandler::AttachFile(cmXMLWriter& xml, std::string const& file,
                                    std::string const& name)
{
  const std::string& base64 = this->CTest->Base64GzipEncodeFile(file);
  std::string const fname = cmSystemTools::GetFilenameName(file);
  xml.StartElement("NamedMeasurement");
  std::string measurement_name = name;
  if (measurement_name.empty()) {
    measurement_name = "Attached File";
  }
  xml.Attribute("name", measurement_name);
  xml.Attribute("encoding", "base64");
  xml.Attribute("compression", "tar/gzip");
  xml.Attribute("filename", fname);
  xml.Attribute("type", "file");
  xml.Element("Value", base64);
  xml.EndElement(); // NamedMeasurement
}

int cmCTestTestHandler::ExecuteCommands(std::vector<std::string>& vec)
{
  for (std::string const& it : vec) {
    int retVal = 0;
    cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                       "Run command: " << it << std::endl, this->Quiet);
    if (!cmSystemTools::RunSingleCommand(it, nullptr, nullptr, &retVal,
                                         nullptr, cmSystemTools::OUTPUT_MERGE
                                         /*this->Verbose*/) ||
        retVal != 0) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Problem running command: " << it << std::endl);
      return 0;
    }
  }
  return 1;
}

// Find the appropriate executable to run for a test
std::string cmCTestTestHandler::FindTheExecutable(const std::string& exe)
{
  std::string resConfig;
  std::vector<std::string> extraPaths;
  std::vector<std::string> failedPaths;
  if (exe == "NOT_AVAILABLE") {
    return exe;
  }
  return cmCTestTestHandler::FindExecutable(this->CTest, exe, resConfig,
                                            extraPaths, failedPaths);
}

// add additional configurations to the search path
void cmCTestTestHandler::AddConfigurations(
  cmCTest* ctest, std::vector<std::string>& attempted,
  std::vector<std::string>& attemptedConfigs, std::string filepath,
  std::string& filename)
{
  std::string tempPath;

  if (!filepath.empty() && filepath[filepath.size() - 1] != '/') {
    filepath += "/";
  }
  tempPath = filepath + filename;
  attempted.push_back(tempPath);
  attemptedConfigs.emplace_back();

  if (!ctest->GetConfigType().empty()) {
    tempPath = cmStrCat(filepath, ctest->GetConfigType(), '/', filename);
    attempted.push_back(tempPath);
    attemptedConfigs.push_back(ctest->GetConfigType());
    // If the file is an OSX bundle then the configtype
    // will be at the start of the path
    tempPath = cmStrCat(ctest->GetConfigType(), '/', filepath, filename);
    attempted.push_back(tempPath);
    attemptedConfigs.push_back(ctest->GetConfigType());
  } else {
    // no config specified - try some options...
    tempPath = cmStrCat(filepath, "Release/", filename);
    attempted.push_back(tempPath);
    attemptedConfigs.emplace_back("Release");
    tempPath = cmStrCat(filepath, "Debug/", filename);
    attempted.push_back(tempPath);
    attemptedConfigs.emplace_back("Debug");
    tempPath = cmStrCat(filepath, "MinSizeRel/", filename);
    attempted.push_back(tempPath);
    attemptedConfigs.emplace_back("MinSizeRel");
    tempPath = cmStrCat(filepath, "RelWithDebInfo/", filename);
    attempted.push_back(tempPath);
    attemptedConfigs.emplace_back("RelWithDebInfo");
    tempPath = cmStrCat(filepath, "Deployment/", filename);
    attempted.push_back(tempPath);
    attemptedConfigs.emplace_back("Deployment");
    tempPath = cmStrCat(filepath, "Development/", filename);
    attempted.push_back(tempPath);
    attemptedConfigs.emplace_back("Deployment");
  }
}

// Find the appropriate executable to run for a test
std::string cmCTestTestHandler::FindExecutable(
  cmCTest* ctest, const std::string& testCommand, std::string& resultingConfig,
  std::vector<std::string>& extraPaths, std::vector<std::string>& failed)
{
  // now run the compiled test if we can find it
  std::vector<std::string> attempted;
  std::vector<std::string> attemptedConfigs;
  std::string tempPath;
  std::string filepath = cmSystemTools::GetFilenamePath(testCommand);
  std::string filename = cmSystemTools::GetFilenameName(testCommand);

  cmCTestTestHandler::AddConfigurations(ctest, attempted, attemptedConfigs,
                                        filepath, filename);

  // even if a fullpath was specified also try it relative to the current
  // directory
  if (!filepath.empty() && filepath[0] == '/') {
    std::string localfilepath = filepath.substr(1, filepath.size() - 1);
    cmCTestTestHandler::AddConfigurations(ctest, attempted, attemptedConfigs,
                                          localfilepath, filename);
  }

  // if extraPaths are provided and we were not passed a full path, try them,
  // try any extra paths
  if (filepath.empty()) {
    for (std::string const& extraPath : extraPaths) {
      std::string filepathExtra = cmSystemTools::GetFilenamePath(extraPath);
      std::string filenameExtra = cmSystemTools::GetFilenameName(extraPath);
      cmCTestTestHandler::AddConfigurations(ctest, attempted, attemptedConfigs,
                                            filepathExtra, filenameExtra);
    }
  }

  // store the final location in fullPath
  std::string fullPath;

  // now look in the paths we specified above
  for (unsigned int ai = 0; ai < attempted.size() && fullPath.empty(); ++ai) {
    // first check without exe extension
    if (cmSystemTools::FileExists(attempted[ai], true)) {
      fullPath = cmSystemTools::CollapseFullPath(attempted[ai]);
      resultingConfig = attemptedConfigs[ai];
    }
    // then try with the exe extension
    else {
      failed.push_back(attempted[ai]);
      tempPath =
        cmStrCat(attempted[ai], cmSystemTools::GetExecutableExtension());
      if (cmSystemTools::FileExists(tempPath, true)) {
        fullPath = cmSystemTools::CollapseFullPath(tempPath);
        resultingConfig = attemptedConfigs[ai];
      } else {
        failed.push_back(tempPath);
      }
    }
  }

  // if everything else failed, check the users path, but only if a full path
  // wasn't specified
  if (fullPath.empty() && filepath.empty()) {
    std::string path = cmSystemTools::FindProgram(filename.c_str());
    if (!path.empty()) {
      resultingConfig.clear();
      return path;
    }
  }
  if (fullPath.empty()) {
    cmCTestLog(ctest, HANDLER_OUTPUT,
               "Could not find executable "
                 << testCommand << "\n"
                 << "Looked in the following places:\n");
    for (std::string const& f : failed) {
      cmCTestLog(ctest, HANDLER_OUTPUT, f << "\n");
    }
  }

  return fullPath;
}

bool cmCTestTestHandler::ParseResourceGroupsProperty(
  const std::string& val,
  std::vector<std::vector<cmCTestTestResourceRequirement>>& resourceGroups)
{
  cmCTestResourceGroupsLexerHelper lexer(resourceGroups);
  return lexer.ParseString(val);
}

bool cmCTestTestHandler::GetListOfTests()
{
  if (!this->IncludeRegExp.empty()) {
    this->IncludeTestsRegularExpression.compile(this->IncludeRegExp);
  }
  if (!this->ExcludeRegExp.empty()) {
    this->ExcludeTestsRegularExpression.compile(this->ExcludeRegExp);
  }
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Constructing a list of tests" << std::endl, this->Quiet);
  cmake cm(cmake::RoleScript, cmState::CTest);
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator gg(&cm);
  cmMakefile mf(&gg, cm.GetCurrentSnapshot());
  mf.AddDefinition("CTEST_CONFIGURATION_TYPE", this->CTest->GetConfigType());

  // Add handler for ADD_TEST
  cm.GetState()->AddBuiltinCommand("add_test", cmCTestAddTestCommand(this));

  // Add handler for SUBDIRS
  cm.GetState()->AddBuiltinCommand("subdirs", cmCTestSubdirCommand);

  // Add handler for ADD_SUBDIRECTORY
  cm.GetState()->AddBuiltinCommand("add_subdirectory",
                                   cmCTestAddSubdirectoryCommand);

  // Add handler for SET_TESTS_PROPERTIES
  cm.GetState()->AddBuiltinCommand("set_tests_properties",
                                   cmCTestSetTestsPropertiesCommand(this));

  // Add handler for SET_DIRECTORY_PROPERTIES
  cm.GetState()->RemoveBuiltinCommand("set_directory_properties");
  cm.GetState()->AddBuiltinCommand("set_directory_properties",
                                   cmCTestSetDirectoryPropertiesCommand(this));

  const char* testFilename;
  if (cmSystemTools::FileExists("CTestTestfile.cmake")) {
    // does the CTestTestfile.cmake exist ?
    testFilename = "CTestTestfile.cmake";
  } else if (cmSystemTools::FileExists("DartTestfile.txt")) {
    // does the DartTestfile.txt exist ?
    testFilename = "DartTestfile.txt";
  } else {
    return true;
  }

  if (!mf.ReadListFile(testFilename)) {
    return false;
  }
  if (cmSystemTools::GetErrorOccurredFlag()) {
    // SEND_ERROR or FATAL_ERROR in CTestTestfile or TEST_INCLUDE_FILES
    return false;
  }
  cmValue specFile = mf.GetDefinition("CTEST_RESOURCE_SPEC_FILE");
  if (this->ResourceSpecFile.empty() && specFile) {
    this->ResourceSpecFile = *specFile;
  }
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Done constructing a list of tests" << std::endl,
                     this->Quiet);
  return true;
}

void cmCTestTestHandler::UseIncludeRegExp()
{
  this->UseIncludeRegExpFlag = true;
}

void cmCTestTestHandler::UseExcludeRegExp()
{
  this->UseExcludeRegExpFlag = true;
  this->UseExcludeRegExpFirst = !this->UseIncludeRegExpFlag;
}

std::string cmCTestTestHandler::GetTestStatus(cmCTestTestResult const& result)
{
  static const char* statuses[] = { "Not Run",     "Timeout",   "SEGFAULT",
                                    "ILLEGAL",     "INTERRUPT", "NUMERICAL",
                                    "OTHER_FAULT", "Failed",    "BAD_COMMAND",
                                    "Completed" };
  int status = result.Status;
  if (status < cmCTestTestHandler::NOT_RUN ||
      status > cmCTestTestHandler::COMPLETED) {
    return "No Status";
  }
  if (status == cmCTestTestHandler::OTHER_FAULT) {
    return result.ExceptionStatus;
  }
  return statuses[status];
}

void cmCTestTestHandler::ExpandTestsToRunInformation(size_t numTests)
{
  if (this->TestsToRunString.empty()) {
    return;
  }

  int start;
  int end = -1;
  double stride = -1;
  std::string::size_type pos = 0;
  std::string::size_type pos2;
  // read start
  if (GetNextNumber(this->TestsToRunString, start, pos, pos2)) {
    // read end
    if (GetNextNumber(this->TestsToRunString, end, pos, pos2)) {
      // read stride
      if (GetNextRealNumber(this->TestsToRunString, stride, pos, pos2)) {
        int val = 0;
        // now read specific numbers
        while (GetNextNumber(this->TestsToRunString, val, pos, pos2)) {
          this->TestsToRun.push_back(val);
        }
        this->TestsToRun.push_back(val);
      }
    }
  }

  // if start is not specified then we assume we start at 1
  if (start == -1) {
    start = 1;
  }

  // if end isnot specified then we assume we end with the last test
  if (end == -1) {
    end = static_cast<int>(numTests);
  }

  // if the stride wasn't specified then it defaults to 1
  if (stride == -1) {
    stride = 1;
  }

  // if we have a range then add it
  if (end != -1 && start != -1 && stride > 0) {
    int i = 0;
    while (i * stride + start <= end) {
      this->TestsToRun.push_back(static_cast<int>(i * stride + start));
      ++i;
    }
  }

  // sort the array
  std::sort(this->TestsToRun.begin(), this->TestsToRun.end(),
            std::less<int>());
  // remove duplicates
  auto new_end = std::unique(this->TestsToRun.begin(), this->TestsToRun.end());
  this->TestsToRun.erase(new_end, this->TestsToRun.end());
}

void cmCTestTestHandler::ExpandTestsToRunInformationForRerunFailed()
{

  std::string dirName = this->CTest->GetBinaryDir() + "/Testing/Temporary";

  cmsys::Directory directory;
  if (!directory.Load(dirName)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Unable to read the contents of " << dirName << std::endl);
    return;
  }

  int numFiles =
    static_cast<int>(cmsys::Directory::GetNumberOfFilesInDirectory(dirName));
  std::string pattern = "LastTestsFailed";
  std::string logName;

  for (int i = 0; i < numFiles; ++i) {
    std::string fileName = directory.GetFile(i);
    // bcc crashes if we attempt a normal substring comparison,
    // hence the following workaround
    std::string fileNameSubstring = fileName.substr(0, pattern.length());
    if (fileNameSubstring != pattern) {
      continue;
    }
    if (logName.empty()) {
      logName = fileName;
    } else {
      // if multiple matching logs were found we use the most recently
      // modified one.
      int res;
      cmSystemTools::FileTimeCompare(logName, fileName, &res);
      if (res == -1) {
        logName = fileName;
      }
    }
  }

  std::string lastTestsFailedLog =
    this->CTest->GetBinaryDir() + "/Testing/Temporary/" + logName;

  if (!cmSystemTools::FileExists(lastTestsFailedLog)) {
    if (!this->CTest->GetShowOnly() && !this->CTest->ShouldPrintLabels()) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 lastTestsFailedLog << " does not exist!" << std::endl);
    }
    return;
  }

  // parse the list of tests to rerun from LastTestsFailed.log
  cmsys::ifstream ifs(lastTestsFailedLog.c_str());
  if (ifs) {
    std::string line;
    std::string::size_type pos;
    while (cmSystemTools::GetLineFromStream(ifs, line)) {
      pos = line.find(':', 0);
      if (pos == std::string::npos) {
        continue;
      }

      line.erase(pos);
      int val = atoi(line.c_str());
      this->TestsToRun.push_back(val);
    }
    ifs.close();
  } else if (!this->CTest->GetShowOnly() &&
             !this->CTest->ShouldPrintLabels()) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Problem reading file: "
                 << lastTestsFailedLog
                 << " while generating list of previously failed tests."
                 << std::endl);
  }
}

void cmCTestTestHandler::RecordCustomTestMeasurements(cmXMLWriter& xml,
                                                      std::string content)
{
  while (this->SingleTestMeasurementRegex.find(content)) {
    // Extract regex match from content and parse it as an XML element.
    auto measurement_str = this->SingleTestMeasurementRegex.match(1);
    auto parser = cmCTestTestMeasurementXMLParser();
    parser.Parse(measurement_str.c_str());

    if (parser.ElementName == "CTestMeasurement" ||
        parser.ElementName == "DartMeasurement") {
      xml.StartElement("NamedMeasurement");
      xml.Attribute("type", parser.MeasurementType);
      xml.Attribute("name", parser.MeasurementName);
      xml.Element("Value", parser.CharacterData);
      xml.EndElement();
    } else if (parser.ElementName == "CTestMeasurementFile" ||
               parser.ElementName == "DartMeasurementFile") {
      const std::string& filename = cmCTest::CleanString(parser.CharacterData);
      if (!cmSystemTools::FileExists(filename)) {
        xml.StartElement("NamedMeasurement");
        xml.Attribute("name", parser.MeasurementName);
        xml.Attribute("text", "text/string");
        xml.Element("Value", "File " + filename + " not found");
        xml.EndElement();
        cmCTestOptionalLog(
          this->CTest, HANDLER_OUTPUT,
          "File \"" << filename << "\" not found." << std::endl, this->Quiet);
      } else {
        long len = cmSystemTools::FileLength(filename);
        if (len == 0) {
          xml.StartElement("NamedMeasurement");
          xml.Attribute("name", parser.MeasurementName);
          xml.Attribute("type", "text/string");
          xml.Attribute("encoding", "none");
          xml.Element("Value", "Image " + filename + " is empty");
          xml.EndElement();
        } else {
          if (parser.MeasurementType == "file") {
            // Treat this measurement like an "ATTACHED_FILE" when the type
            // is explicitly "file" (not an image).
            this->AttachFile(xml, filename, parser.MeasurementName);
          } else {
            cmsys::ifstream ifs(filename.c_str(),
                                std::ios::in
#ifdef _WIN32
                                  | std::ios::binary
#endif
            );
            auto file_buffer = cm::make_unique<unsigned char[]>(len + 1);
            ifs.read(reinterpret_cast<char*>(file_buffer.get()), len);
            auto encoded_buffer = cm::make_unique<unsigned char[]>(
              static_cast<int>(static_cast<double>(len) * 1.5 + 5.0));

            size_t rlen = cmsysBase64_Encode(file_buffer.get(), len,
                                             encoded_buffer.get(), 1);

            xml.StartElement("NamedMeasurement");
            xml.Attribute("name", parser.MeasurementName);
            xml.Attribute("type", parser.MeasurementType);
            xml.Attribute("encoding", "base64");
            std::ostringstream ostr;
            for (size_t cc = 0; cc < rlen; cc++) {
              ostr << encoded_buffer[cc];
              if (cc % 60 == 0 && cc) {
                ostr << std::endl;
              }
            }
            xml.Element("Value", ostr.str());
            xml.EndElement(); // NamedMeasurement
          }
        }
      }
    }

    // Remove this element from content.
    cmSystemTools::ReplaceString(content, measurement_str.c_str(), "");
  }
}

void cmCTestTestHandler::SetIncludeRegExp(const std::string& arg)
{
  this->IncludeRegExp = arg;
}

void cmCTestTestHandler::SetExcludeRegExp(const std::string& arg)
{
  this->ExcludeRegExp = arg;
}

bool cmCTestTestHandler::SetTestOutputTruncation(const std::string& mode)
{
  if (mode == "tail") {
    this->TestOutputTruncation = cmCTestTypes::TruncationMode::Tail;
  } else if (mode == "middle") {
    this->TestOutputTruncation = cmCTestTypes::TruncationMode::Middle;
  } else if (mode == "head") {
    this->TestOutputTruncation = cmCTestTypes::TruncationMode::Head;
  } else {
    return false;
  }
  return true;
}

void cmCTestTestHandler::SetTestsToRunInformation(cmValue in)
{
  if (!in) {
    return;
  }
  this->TestsToRunString = *in;
  // if the argument is a file, then read it and use the contents as the
  // string
  if (cmSystemTools::FileExists(*in)) {
    cmsys::ifstream fin(in->c_str());
    unsigned long filelen = cmSystemTools::FileLength(*in);
    auto buff = cm::make_unique<char[]>(filelen + 1);
    fin.getline(buff.get(), filelen);
    buff[fin.gcount()] = 0;
    this->TestsToRunString = buff.get();
  }
}

void cmCTestTestHandler::CleanTestOutput(std::string& output, size_t length,
                                         cmCTestTypes::TruncationMode truncate)
{
  if (!length || length >= output.size() ||
      output.find("CTEST_FULL_OUTPUT") != std::string::npos) {
    return;
  }

  // Advance n bytes in string delimited by begin/end but do not break in the
  // middle of a multi-byte UTF-8 encoding.
  auto utf8_advance = [](char const* const begin, char const* const end,
                         size_t n) -> const char* {
    char const* const stop = begin + n;
    char const* current = begin;
    while (current < stop) {
      unsigned int ch;
      if (const char* next = cm_utf8_decode_character(current, end, &ch)) {
        if (next > stop) {
          break;
        }
        current = next;
      } else // Bad byte will be handled by cmXMLWriter.
      {
        ++current;
      }
    }
    return current;
  };

  // Truncation message.
  const std::string msg =
    "\n[This part of the test output was removed since it "
    "exceeds the threshold of " +
    std::to_string(length) + " bytes.]\n";

  char const* const begin = output.c_str();
  char const* const end = begin + output.size();

  // Erase head, middle or tail of output.
  if (truncate == cmCTestTypes::TruncationMode::Head) {
    char const* current = utf8_advance(begin, end, output.size() - length);
    output.erase(0, current - begin);
    output.insert(0, msg + "...");
  } else if (truncate == cmCTestTypes::TruncationMode::Middle) {
    char const* current = utf8_advance(begin, end, length / 2);
    output.erase(current - begin, output.size() - length);
    output.insert(current - begin, "..." + msg + "...");
  } else { // default or "tail"
    char const* current = utf8_advance(begin, end, length);
    output.erase(current - begin);
    output += ("..." + msg);
  }
}

void cmCTestTestHandler::cmCTestTestProperties::AppendError(
  cm::string_view err)
{
  if (this->Error) {
    *this->Error = cmStrCat(*this->Error, '\n', err);
  } else {
    this->Error = err;
  }
}

bool cmCTestTestHandler::SetTestsProperties(
  const std::vector<std::string>& args)
{
  std::vector<std::string>::const_iterator it;
  std::vector<std::string> tests;
  bool found = false;
  for (it = args.begin(); it != args.end(); ++it) {
    if (*it == "PROPERTIES") {
      found = true;
      break;
    }
    tests.push_back(*it);
  }
  if (!found) {
    return false;
  }
  ++it; // skip PROPERTIES
  for (; it != args.end(); ++it) {
    std::string const& key = *it;
    ++it;
    if (it == args.end()) {
      break;
    }
    std::string const& val = *it;
    for (std::string const& t : tests) {
      for (cmCTestTestProperties& rt : this->TestList) {
        if (t == rt.Name) {
          if (key == "_BACKTRACE_TRIPLES"_s) {
            // allow empty args in the triples
            cmList triples{ val, cmList::EmptyElements::Yes };

            // Ensure we have complete triples otherwise the data is corrupt.
            if (triples.size() % 3 == 0) {
              cmState state(cmState::Unknown);
              rt.Backtrace = cmListFileBacktrace();

              // the first entry represents the top of the trace so we need to
              // reconstruct the backtrace in reverse
              for (auto i = triples.size(); i >= 3; i -= 3) {
                cmListFileContext fc;
                fc.FilePath = triples[i - 3];
                long line = 0;
                if (!cmStrToLong(triples[i - 2], &line)) {
                  line = 0;
                }
                fc.Line = line;
                fc.Name = triples[i - 1];
                rt.Backtrace = rt.Backtrace.Push(fc);
              }
            }
          } else if (key == "WILL_FAIL"_s) {
            rt.WillFail = cmIsOn(val);
          } else if (key == "DISABLED"_s) {
            rt.Disabled = cmIsOn(val);
          } else if (key == "ATTACHED_FILES"_s) {
            cmExpandList(val, rt.AttachedFiles);
          } else if (key == "ATTACHED_FILES_ON_FAIL"_s) {
            cmExpandList(val, rt.AttachOnFail);
          } else if (key == "RESOURCE_LOCK"_s) {
            cmList lval{ val };

            rt.ProjectResources.insert(lval.begin(), lval.end());
          } else if (key == "FIXTURES_SETUP"_s) {
            cmList lval{ val };

            rt.FixturesSetup.insert(lval.begin(), lval.end());
          } else if (key == "FIXTURES_CLEANUP"_s) {
            cmList lval{ val };

            rt.FixturesCleanup.insert(lval.begin(), lval.end());
          } else if (key == "FIXTURES_REQUIRED"_s) {
            cmList lval{ val };

            rt.FixturesRequired.insert(lval.begin(), lval.end());
          } else if (key == "TIMEOUT"_s) {
            rt.Timeout = cmDuration(atof(val.c_str()));
          } else if (key == "TIMEOUT_SIGNAL_NAME"_s) {
#ifdef _WIN32
            rt.AppendError("TIMEOUT_SIGNAL_NAME is not supported on Windows.");
#else
            std::string const& signalName = val;
            Signal s;
            if (signalName == "SIGINT"_s) {
              s.Number = SIGINT;
            } else if (signalName == "SIGQUIT"_s) {
              s.Number = SIGQUIT;
            } else if (signalName == "SIGTERM"_s) {
              s.Number = SIGTERM;
            } else if (signalName == "SIGUSR1"_s) {
              s.Number = SIGUSR1;
            } else if (signalName == "SIGUSR2"_s) {
              s.Number = SIGUSR2;
            }
            if (s.Number) {
              s.Name = signalName;
              rt.TimeoutSignal = std::move(s);
            } else {
              rt.AppendError(cmStrCat("TIMEOUT_SIGNAL_NAME \"", signalName,
                                      "\" not supported on this platform."));
            }
#endif
          } else if (key == "TIMEOUT_SIGNAL_GRACE_PERIOD"_s) {
#ifdef _WIN32
            rt.AppendError(
              "TIMEOUT_SIGNAL_GRACE_PERIOD is not supported on Windows.");
#else
            std::string const& gracePeriod = val;
            static cmDuration minGracePeriod{ 0 };
            static cmDuration maxGracePeriod{ 60 };
            cmDuration gp = cmDuration(atof(gracePeriod.c_str()));
            if (gp <= minGracePeriod) {
              rt.AppendError(cmStrCat("TIMEOUT_SIGNAL_GRACE_PERIOD \"",
                                      gracePeriod, "\" is not greater than \"",
                                      minGracePeriod.count(), "\" seconds."));
            } else if (gp > maxGracePeriod) {
              rt.AppendError(cmStrCat("TIMEOUT_SIGNAL_GRACE_PERIOD \"",
                                      gracePeriod,
                                      "\" is not less than the maximum of \"",
                                      maxGracePeriod.count(), "\" seconds."));
            } else {
              rt.TimeoutGracePeriod = gp;
            }
#endif
          } else if (key == "COST"_s) {
            rt.Cost = static_cast<float>(atof(val.c_str()));
          } else if (key == "REQUIRED_FILES"_s) {
            cmExpandList(val, rt.RequiredFiles);
          } else if (key == "RUN_SERIAL"_s) {
            rt.RunSerial = cmIsOn(val);
          } else if (key == "FAIL_REGULAR_EXPRESSION"_s) {
            cmList lval{ val };
            for (std::string const& cr : lval) {
              rt.ErrorRegularExpressions.emplace_back(cr, cr);
            }
          } else if (key == "SKIP_REGULAR_EXPRESSION"_s) {
            cmList lval{ val };
            for (std::string const& cr : lval) {
              rt.SkipRegularExpressions.emplace_back(cr, cr);
            }
          } else if (key == "PROCESSORS"_s) {
            rt.Processors = atoi(val.c_str());
            if (rt.Processors < 1) {
              rt.Processors = 1;
            }
          } else if (key == "PROCESSOR_AFFINITY"_s) {
            rt.WantAffinity = cmIsOn(val);
          } else if (key == "RESOURCE_GROUPS"_s) {
            if (!ParseResourceGroupsProperty(val, rt.ResourceGroups)) {
              return false;
            }
          } else if (key == "GENERATED_RESOURCE_SPEC_FILE"_s) {
            rt.GeneratedResourceSpecFile = val;
          } else if (key == "SKIP_RETURN_CODE"_s) {
            rt.SkipReturnCode = atoi(val.c_str());
            if (rt.SkipReturnCode < 0 || rt.SkipReturnCode > 255) {
              rt.SkipReturnCode = -1;
            }
          } else if (key == "DEPENDS"_s) {
            cmExpandList(val, rt.Depends);
          } else if (key == "ENVIRONMENT"_s) {
            cmExpandList(val, rt.Environment);
          } else if (key == "ENVIRONMENT_MODIFICATION"_s) {
            cmExpandList(val, rt.EnvironmentModification);
          } else if (key == "LABELS"_s) {
            cmList Labels{ val };
            rt.Labels.insert(rt.Labels.end(), Labels.begin(), Labels.end());
            // sort the array
            std::sort(rt.Labels.begin(), rt.Labels.end());
            // remove duplicates
            auto new_end = std::unique(rt.Labels.begin(), rt.Labels.end());
            rt.Labels.erase(new_end, rt.Labels.end());
          } else if (key == "MEASUREMENT"_s) {
            size_t pos = val.find_first_of('=');
            if (pos != std::string::npos) {
              std::string mKey = val.substr(0, pos);
              std::string mVal = val.substr(pos + 1);
              rt.Measurements[mKey] = std::move(mVal);
            } else {
              rt.Measurements[val] = "1";
            }
          } else if (key == "PASS_REGULAR_EXPRESSION"_s) {
            cmList lval{ val };
            for (std::string const& cr : lval) {
              rt.RequiredRegularExpressions.emplace_back(cr, cr);
            }
          } else if (key == "WORKING_DIRECTORY"_s) {
            rt.Directory = val;
          } else if (key == "TIMEOUT_AFTER_MATCH"_s) {
            cmList propArgs{ val };
            if (propArgs.size() != 2) {
              cmCTestLog(this->CTest, WARNING,
                         "TIMEOUT_AFTER_MATCH expects two arguments, found "
                           << propArgs.size() << std::endl);
            } else {
              rt.AlternateTimeout = cmDuration(atof(propArgs[0].c_str()));
              cmList lval{ propArgs[1] };
              for (std::string const& cr : lval) {
                rt.TimeoutRegularExpressions.emplace_back(cr, cr);
              }
            }
          }
        }
      }
    }
  }
  return true;
}

bool cmCTestTestHandler::SetDirectoryProperties(
  const std::vector<std::string>& args)
{
  std::vector<std::string>::const_iterator it;
  std::vector<std::string> tests;
  bool found = false;
  for (it = args.begin(); it != args.end(); ++it) {
    if (*it == "PROPERTIES") {
      found = true;
      break;
    }
    tests.push_back(*it);
  }

  if (!found) {
    return false;
  }
  ++it; // skip PROPERTIES
  for (; it != args.end(); ++it) {
    std::string const& key = *it;
    ++it;
    if (it == args.end()) {
      break;
    }
    std::string const& val = *it;
    for (cmCTestTestProperties& rt : this->TestList) {
      std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
      if (cwd == rt.Directory) {
        if (key == "LABELS"_s) {
          cmList DirectoryLabels{ val };
          rt.Labels.insert(rt.Labels.end(), DirectoryLabels.begin(),
                           DirectoryLabels.end());

          // sort the array
          std::sort(rt.Labels.begin(), rt.Labels.end());
          // remove duplicates
          auto new_end = std::unique(rt.Labels.begin(), rt.Labels.end());
          rt.Labels.erase(new_end, rt.Labels.end());
        }
      }
    }
  }
  return true;
}

bool cmCTestTestHandler::AddTest(const std::vector<std::string>& args)
{
  const std::string& testname = args[0];
  cmCTestOptionalLog(this->CTest, DEBUG, "Add test: " << args[0] << std::endl,
                     this->Quiet);

  if (this->UseExcludeRegExpFlag && this->UseExcludeRegExpFirst &&
      this->ExcludeTestsRegularExpression.find(testname)) {
    return true;
  }
  if (this->MemCheck) {
    std::vector<std::string>::iterator it;
    bool found = false;
    for (it = this->CustomTestsIgnore.begin();
         it != this->CustomTestsIgnore.end(); ++it) {
      if (*it == testname) {
        found = true;
        break;
      }
    }
    if (found) {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "Ignore memcheck: " << *it << std::endl, this->Quiet);
      return true;
    }
  } else {
    std::vector<std::string>::iterator it;
    bool found = false;
    for (it = this->CustomTestsIgnore.begin();
         it != this->CustomTestsIgnore.end(); ++it) {
      if (*it == testname) {
        found = true;
        break;
      }
    }
    if (found) {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "Ignore test: " << *it << std::endl, this->Quiet);
      return true;
    }
  }

  cmCTestTestProperties test;
  test.Name = testname;
  test.Args = args;
  test.Directory = cmSystemTools::GetCurrentWorkingDirectory();
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "Set test directory: " << test.Directory << std::endl,
                     this->Quiet);

  if (this->UseIncludeRegExpFlag &&
      (!this->IncludeTestsRegularExpression.find(testname) ||
       (!this->UseExcludeRegExpFirst &&
        this->ExcludeTestsRegularExpression.find(testname)))) {
    test.IsInBasedOnREOptions = false;
  }
  this->TestList.push_back(test);
  return true;
}

bool cmCTestTestHandler::cmCTestTestResourceRequirement::operator==(
  const cmCTestTestResourceRequirement& other) const
{
  return this->ResourceType == other.ResourceType &&
    this->SlotsNeeded == other.SlotsNeeded &&
    this->UnitsNeeded == other.UnitsNeeded;
}

bool cmCTestTestHandler::cmCTestTestResourceRequirement::operator!=(
  const cmCTestTestResourceRequirement& other) const
{
  return !(*this == other);
}

void cmCTestTestHandler::SetJUnitXMLFileName(const std::string& filename)
{
  this->JUnitXMLFileName = filename;
}

bool cmCTestTestHandler::WriteJUnitXML()
{
  if (this->JUnitXMLFileName.empty()) {
    return true;
  }

  // Open new XML file for writing.
  cmGeneratedFileStream xmlfile;
  xmlfile.SetTempExt("tmp");
  xmlfile.Open(this->JUnitXMLFileName);
  if (!xmlfile) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Problem opening file: " << this->JUnitXMLFileName
                                        << std::endl);
    return false;
  }
  cmXMLWriter xml(xmlfile);

  // Iterate over the test results to get the number of tests that
  // passed, failed, etc.
  auto num_tests = 0;
  auto num_failed = 0;
  auto num_notrun = 0;
  auto num_disabled = 0;
  SetOfTests resultsSet(this->TestResults.begin(), this->TestResults.end());
  for (cmCTestTestResult const& result : resultsSet) {
    num_tests++;
    if (result.Status == cmCTestTestHandler::NOT_RUN) {
      if (result.CompletionStatus == "Disabled") {
        num_disabled++;
      } else {
        num_notrun++;
      }
    } else if (result.Status != cmCTestTestHandler::COMPLETED) {
      num_failed++;
    }
  }

  // Write <testsuite> element.
  xml.StartDocument();
  xml.StartElement("testsuite");

  xml.Attribute("name",
                cmCTest::SafeBuildIdField(
                  this->CTest->GetCTestConfiguration("BuildName")));
  xml.BreakAttributes();

  xml.Attribute("tests", num_tests);
  xml.Attribute("failures", num_failed);

  // CTest disabled => JUnit disabled
  xml.Attribute("disabled", num_disabled);

  // Otherwise, CTest notrun => JUnit skipped.
  // The distinction between JUnit disabled vs. skipped is that
  // skipped tests can have a message associated with them
  // (why the test was skipped).
  xml.Attribute("skipped", num_notrun);

  xml.Attribute("hostname", this->CTest->GetCTestConfiguration("Site"));
  xml.Attribute(
    "time",
    std::chrono::duration_cast<std::chrono::seconds>(this->ElapsedTestingTime)
      .count());
  const std::time_t start_test_time_t =
    std::chrono::system_clock::to_time_t(this->StartTestTime);
  cmTimestamp cmts;
  xml.Attribute("timestamp",
                cmts.CreateTimestampFromTimeT(start_test_time_t,
                                              "%Y-%m-%dT%H:%M:%S", false));

  // Write <testcase> elements.
  for (cmCTestTestResult const& result : resultsSet) {
    xml.StartElement("testcase");
    xml.Attribute("name", result.Name);
    xml.Attribute("classname", result.Name);
    xml.Attribute("time", result.ExecutionTime.count());

    std::string status;
    if (result.Status == cmCTestTestHandler::COMPLETED) {
      status = "run";
    } else if (result.Status == cmCTestTestHandler::NOT_RUN) {
      if (result.CompletionStatus == "Disabled") {
        status = "disabled";
      } else {
        status = "notrun";
      }
    } else {
      status = "fail";
    }
    xml.Attribute("status", status);

    if (status == "notrun") {
      xml.StartElement("skipped");
      xml.Attribute("message", result.CompletionStatus);
      xml.EndElement(); // </skipped>
    } else if (status == "fail") {
      xml.StartElement("failure");
      xml.Attribute("message", this->GetTestStatus(result));
      xml.EndElement(); // </failure>
    }

    xml.StartElement("properties");
    if ((result.Properties) && (!result.Properties->Labels.empty())) {
      xml.StartElement("property");
      xml.Attribute("name", "cmake_labels");
      // Pass the property as a cmake-formatted list, consumers will know
      // anyway that this information is coming from cmake, so it should
      // be ok to put it here as a cmake-list.
      xml.Attribute("value", cmList::to_string(result.Properties->Labels));
      // if we export more properties, this should be done the same way,
      // i.e. prefix the property name with "cmake_", and it it can be
      // a list, write it cmake-formatted.
      xml.EndElement(); // </property>
    }
    xml.EndElement(); // </properties>

    // Note: compressed test output is unconditionally disabled when
    // --output-junit is specified.
    xml.Element("system-out", result.Output);
    xml.EndElement(); // </testcase>
  }

  xml.EndElement(); // </testsuite>
  xml.EndDocument();

  return true;
}
